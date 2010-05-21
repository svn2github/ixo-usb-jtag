#!/usr/bin/perl

# EPCS12 configuration has 2322888 bits
# EPCS12 IDCODE is 0x020830DD (0000.0010.0000.1000.0011.0000.1101.1101)

use strict;
use integer;

my $verbose = 0;
my $recorded_shifts = 1024;

######################### JTAG TAP SIMULATION ###############################

my $sampled_tdi = 0;
my $sampled_tms = 0;

my $state = 'reset';
my $jtag_dr_in   = '';
my $jtag_dr_out  = '';
my $jtag_ir_in   = '';
my $jtag_ir_out  = '';
my $shift_count  = 0;
my $previous_tck = 0;

my %state_change =
(
  'reset'      => [ 'idle',       'reset' ],
  'idle'       => [ 'idle',       'select_dr' ],
  'select_dr'  => [ 'capture_dr', 'select_ir' ],
  'select_ir'  => [ 'capture_ir', 'reset' ],
  'capture_dr' => [ 'shift_dr',   'exit_1_dr' ],
  'capture_ir' => [ 'shift_ir',   'exit_1_ir' ],
  'shift_dr'   => [ 'shift_dr',   'exit_1_dr' ],
  'shift_ir'   => [ 'shift_ir',   'exit_1_ir' ],
  'exit_1_dr'  => [ 'pause_dr',   'update_dr' ],
  'exit_1_ir'  => [ 'pause_ir',   'update_ir' ],
  'pause_dr'   => [ 'pause_dr',   'exit_2_dr' ],
  'pause_ir'   => [ 'pause_ir',   'exit_2_ir' ],
  'exit_2_dr'  => [ 'shift_dr',   'update_dr' ],
  'exit_2_ir'  => [ 'shift_ir',   'update_ir' ],
  'update_dr'  => [ 'idle',       'select_dr' ],
  'update_ir'  => [ 'idle',       'select_dr' ]
);

sub interpretation($$)
{
  my ($bits, $is_ir) = @_;
  my $byte = 0;
  my $bytes = '';
  my $bitcount = 0;
  my $instruction = '';
  my $all_bits_one = 1;

  for(my $i=length($bits)-1; $i>=0; $i--)
  {
    my $last_bit = substr($bits, $i, 1);
    if($last_bit != '1') { $all_bits_one = 0 };
    $byte = ($last_bit ? 128:0) | ($byte >> 1);
    $bitcount ++;
    if($bitcount >= 8) 
    {
      $bytes = sprintf('%02x',$byte).$bytes;
      $bitcount = 0;
      $byte = 0; 
    };
  };
  if($bitcount)
  {
    for(;$bitcount<8;$bitcount++) { $byte >>= 1 };
    $bytes = sprintf('%02x',$byte).$bytes;
  };

  if($is_ir)
  {
    my $icode = hex(substr($bytes,-4,4));
    if   ($icode == 0x000) { $instruction = 'EXTEST' }
    elsif($icode == 0x001) { $instruction = 'PULSE_NCONFIG' }
    elsif($icode == 0x002) { $instruction = 'PROGRAM' }
    elsif($icode == 0x003) { $instruction = 'STARTUP' }
    elsif($icode == 0x004) { $instruction = 'CHECK_STATUS' }
    elsif($icode == 0x005) { $instruction = 'SAMPLE/PRELOAD' }
    elsif($icode == 0x006) { $instruction = 'IDCODE' }
    elsif($icode == 0x007) { $instruction = 'USERCODE' }
    elsif($icode == 0x00A) { $instruction = 'CLAMP' }
    elsif($icode == 0x00B) { $instruction = 'HIGHZ' }
    elsif($icode == 0x00C) { $instruction = 'VIRTUAL_DR'  }
    elsif($icode == 0x00D) { $instruction = 'CONFIG_IO' }
    elsif($icode == 0x00E) { $instruction = 'VIRTUAL_IR'  }
    elsif($icode == 0x00F) { $instruction = 'EXTEST' } # some devices
    elsif($all_bits_one)   { $instruction = 'BYPASS' }
    else                   { $instruction = '???' };
    return $bits.' = 0x'.$bytes.' ('.$instruction.')';
  };
  return $bits.' = 0x'.$bytes.$instruction;
};

sub update_jtag_state($$$$$)
{
  my($tdi, $tck, $tms, $tdo, $time) = @_;

  if(!$previous_tck && $tck) # Rising edge of TCK
  { 
    # Sample TDI and TMS (TDO is sampled at falling edge)
    $sampled_tms = $tms;
    $sampled_tdi = $tdi;
    if($verbose) { printf "TCK raised, TMS=%d, TDI=%d, TDO=%d, state=%s\n", $tms, $tdi, $tdo, $state; };
  }
  elsif($previous_tck && !$tck) # Falling edge of TCK
  {
    if($state eq 'shift_dr')
    {  
      if($shift_count++ < $recorded_shifts)
      { 
        $jtag_dr_in = $sampled_tdi.$jtag_dr_in; 
        $jtag_dr_out = $tdo.$jtag_dr_out;
      }
    }
    elsif($state eq 'shift_ir')
    {
      if($shift_count++ < $recorded_shifts)
      { 
        $jtag_ir_in = $sampled_tdi.$jtag_ir_in; 
        $jtag_ir_out = $tdo.$jtag_ir_out;
      }
    }
    elsif($state eq 'capture_dr')
    {
      $jtag_dr_in = '';
      $jtag_dr_out = '';
      $shift_count=0;
    }
    elsif($state eq 'capture_ir')
    { 
      $jtag_ir_in = '';
      $jtag_ir_out = '';
      $shift_count=0; 
    }
    elsif($state eq 'update_dr')
    { 
      printf "## %s\n", $time;
      printf "  DR IN  %8u %s\n", $shift_count, interpretation($jtag_dr_in,0);
      printf "  DR OUT %8u %s\n", $shift_count, interpretation($jtag_dr_out,0);
    }
    elsif($state eq 'update_ir')
    { 
      printf "## %s\n", $time;
      printf "  IR IN  %8u %s\n", $shift_count, interpretation($jtag_ir_in,1);
      printf "  IR OUT %8u %s\n", $shift_count, interpretation($jtag_ir_out,0);
    };

    # Update state
    my $new_state = $state_change{$state}->[$sampled_tms];
    if($verbose) { if($new_state ne $state) { printf ">> $new_state\n" } };
    $state = $new_state;
    
    if($verbose) { printf "TCK fell, TMS=%d, TDI=%d, TDO=%d, state->%s\n", $tms, $tdi, $tdo, $state; };
  }
  else
  {
    if($verbose) { printf "TCK stable, TMS=%d, TDI=%d, TDO=%d, state->%s\n", $tms, $tdi, $tdo, $state; };
  };

  $previous_tck = $tck;
};

######################### USB-BLASTER TRAFFIC PARSER ########################

my $last_tdo = 0; # from device
my $last_tms = 0; # to device

my $print_pointer = -1;
my @activity = ();


sub dump_activity()
{
  while($print_pointer < $#activity)
  {
    $print_pointer++;
    my($time, $type, $rw, $to_device, $from_device) = @{$activity[$print_pointer]};
    if($type eq 'bit')
    {
      my $tck = (($to_device & (1<<0)) != 0) ? 1 : 0;
      my $tms = (($to_device & (1<<1)) != 0) ? 1 : 0;
      my $tdi = (($to_device & (1<<4)) != 0) ? 1 : 0;
      if($rw eq 'rw') { $last_tdo = (($from_device & (1<<0)) != 0) ? 1 : 0; };
      if($verbose) { printf("Output bits %02X\n", $to_device); };
      update_jtag_state($tdi, $tck, $tms, $last_tdo, $time);
      $last_tms = $tms; 
    }
    else
    {
      if($verbose) { printf("Output byte %02X\n", $to_device); };
      for(my $i=0; $i<8; $i++)
      {
        my $tdi = $to_device & 1;
        $to_device >>= 1;
        if($rw eq 'rw') { $last_tdo = (($from_device & (1<<0)) != 0) ? 1 : 0; }
        $from_device >>= 1;
        update_jtag_state($tdi, 1, $last_tms, $last_tdo, $time);
        update_jtag_state($tdi, 0, $last_tms, $last_tdo, $time);
      };
    }
  }
};

######################### DATA SET PARSER #########################

my $read_write = 'w';
my $clock_n_bytes = 0;
my $read_pointer = -1;
my $logfile_format = 0;
my $klog_time = '';
my $klog_data = '';
my $klog_direction = '';
my $klog_line = 0;

sub parse_long_log_line($$$)
{
  my ($time,$direction,$data) = @_;

  # print "PARSE: $time $direction $data\n";

  if($direction eq 'read' && $data =~ /data\s*(.*)\s*$/)
  {
    my @data = split(/\s+/,$1);
    if($verbose) { printf "##$time\n==================== Read:  %s\n", join(',',@data); };

    while($#data >= 0)
    {
      my @read = splice(@data,0,64); # Process only chunks of 64 bytes at once.
      @read = splice(@read,2);       # Discard two header bytes from FTDI chip.

      while($read_pointer < $#activity && $#read >= 0)
      {
        $read_pointer++;
        if($activity[$read_pointer]->[2] eq 'rw')
        {
          $activity[$read_pointer]->[4] = hex(shift @read);
        }
      }
    }
  }
  elsif($direction eq 'write' && $data =~ /data:\s*(.*)\s*$/)
  {
    my @data = split(/\s+/,$1);

    dump_activity();

    if($#activity>=0)
    {
      while($read_pointer < $#activity && $activity[$read_pointer+1]->[2] eq 'w')
      { 
        $read_pointer++;
      };
      if($read_pointer < $#activity)
      {
        # There's still a chance that klog output was broken, especially when doing a large number
        # of byte transfers at once. Try to recover.
        printf "WARNING! input line %d: read_pointer is %d but activity is %d.. Resetting.\n", $klog_line, $read_pointer, $#activity;
        $read_pointer = $#activity;
      }
    };

    if($verbose) { printf "##$time\n==================== Wrote: %s\n", join(',',@data); };

    my $n = $#data + 1;

    for(my $i=0; $i<$n;)
    {
      if($clock_n_bytes > 0)
      {
        my $m = $n - $i;
        if($clock_n_bytes < $m) { $m = $clock_n_bytes };
        $clock_n_bytes -= $m; 
        $i += $m;
        while($m--) { push(@activity, [ $time, 'byte', $read_write, hex(shift @data) ]) }
      }
      else
      {
        my $d = hex(shift @data);
        $read_write = (($d & (1<<6)) != 0) ? 'rw' : 'w';

        if(($d & (1<<7)) != 0)
        {
          $clock_n_bytes = $d & 0x3F;
        }
        else
        {
          push(@activity, [ $time, 'bit', $read_write, $d, 0 ]);
        };
        $i++;
      }
    }
  }
}

######################### SYSLOG AND USBMON PARSER ################
  
while(<>)
{
  $klog_line ++;

  if($logfile_format == 0 || $logfile_format == 1)
  {
#Sep  2 10:01:28 n4 kernel: [  892.163043] usb 2-6: usbdev_ioctl: BULK
#Sep  2 10:01:28 n4 kernel: [  892.163047] usb 2-6: bulk write: len=0x17E timeout=2000
#Sep  2 10:01:28 n4 kernel: [  892.163049] usb 2-6: bulk write: data: 00 00 00 00 00 00 00 ...
#Sep  2 10:01:28 n4 kernel: 00 0c bd 0f 0f 0f f3 f0 f0 f0
#Sep  2 10:01:28 n4 kernel: [  892.163551] usb 2-6: usbdev_ioctl: BULK
#Sep  2 10:01:28 n4 kernel: [  892.163554] usb 2-6: bulk read: len=0x40 timeout=0100
#Sep  2 10:01:28 n4 kernel: [  892.163663] usb 2-6: bulk read: data 31 60
#Sep  2 10:01:28 n4 kernel: [  892.163668] usb 2-6: usbdev_ioctl: BULK
#Sep  2 10:01:28 n4 kernel: [  892.163671] usb 2-6: bulk write: len=0x01 timeout=2000
#Sep  2 10:01:28 n4 kernel: [  892.163674] usb 2-6: bulk write: data: 7e
#Sep  2 10:01:28 n4 kernel: [  892.164005] usb 2-6: usbdev_ioctl: BULK
#Sep  2 10:01:28 n4 kernel: [  892.164008] usb 2-6: bulk read: len=0x40 timeout=2000
#Sep  2 10:01:28 n4 kernel: [  892.163878] usb 2-6: bulk read: data 31 60 03

    if(/(.*kernel:)*\s*\[\s*([0-9.]+)\]\s*usb\s*.*\s*bulk\s+(read|write):\s*(.*?)\s*$/)
    {
      $logfile_format = 1;
      if($klog_data ne '') { parse_long_log_line($klog_time, $klog_direction, $klog_data); };
      ($klog_time,$klog_direction,$klog_data) = ($2,$3,$4);
    }
    elsif(/(.*kernel:)*\s*(( [0-9a-fA-F]{2})+)\s*$/ && $klog_data ne '')
    {
      $klog_data .= $2;
    }
    else
    {
      if($klog_data ne '') { parse_long_log_line($klog_time, $klog_direction, $klog_data); };
      $klog_data = '';
    };
  };

  if($logfile_format == 0 || $logfile_format == 2)
  {
# 000004: Bulk or Interrupt Transfer (UP), 10.04.2006 00:23:35.2182416 +0.0
# 000012: Control Transfer (UP), 10.04.2006 00:23:35.2382704 +0.0100144

    if(/^(\d+): (.*) \((UP|DOWN)\), (.*?)[\r\n\s]*$/)
    {
      $logfile_format = 2;
      if($klog_data ne '') { parse_long_log_line($klog_time, $klog_direction, $klog_data); };
      $klog_direction = $2;
      $klog_time = $4;
      $klog_data = '';
    }
    elsif(/^(Get|Send) 0x[0-9a-fA-F]+ bytes (from|to) the device/)
    {
      my $dir = $1;
      if($klog_direction =~ /^Bulk/)
      {
        if($dir eq 'Get') 
        {
          $klog_direction = 'read';
          $klog_data = 'data';
        }
        else
        {
          $klog_direction = 'write';
          $klog_data = 'data:';
        }
      }
      else
      {
        $klog_data = '';
      };
    }
    elsif(/((\s[0-9a-fA-F]{2}|   ){16}) / && $klog_data ne '')
    {
      $klog_data .= $1;
    }
    else
    {
      # print "UNMATCHED: $L";
    };
  }
}

if($klog_data ne '') { parse_long_log_line($klog_time, $klog_direction, $klog_data); };
dump_activity();


# :vim:nosmartindent:
#
