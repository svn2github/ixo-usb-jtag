-----------------------------------------------------------------------------

usb_jtag - Variations on the implementation of a USB JTAG adapter.

Copyright (C) 2005-2007 Kolja Waschk, ixo.de

-----------------------------------------------------------------------------

usb_jtag is free software; you can redistribute it and/or modify it under the
terms of the GNU General Public License as published by the Free Software
Foundation; either version 2 of the License, or (at your option) any later
version.

usb_jtag is distributed in the hope that it will be useful, but WITHOUT ANY
WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR
A PARTICULAR PURPOSE.  See the GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program as the file COPYING.txt; if not, write to the Free Software 
Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301  USA

-----------------------------------------------------------------------------

NEWS:
- May 2010: This project has been moved to SourceForge:
  
             http://ixo-jtag.sourceforge.net/ 
  
  I (Hauke D.) have taken over the maintenance of this project for now. I've
  mostly just imported various things into the SVN repository, applied a
  few small patches, made a few minor updates, and did some general cleanup.
  Please see the above link for information on getting help. Also, patches are
  always welcome :)
  
  IMPORTANT: I can currently only comment on the status of a few of the things
  here:
  - the device/c51/ directory has been tested on a Nexys2 and Nexys(1)
  - the device/cpld/ directory should be stable
  - the device/fpga4u/ directory is in sync with version 4.2.0 of the FPGA4U
    firmware (see the README in that directory for more information)
  All the other stuff might be in various states of decay - if you are using
  any of these things in production, I'd appreciate a note so that I can update
  this notice.

-----------------------------------------------------------------------------

IMPORTANT: The code and information herein is NOT sufficient to build a 
device that can be used exactly like an "Altera USB Blaster". I have only
included code here that is purely my own work or taken from Cypress library
for the FX2 firmware. I do not include any copy of Altera code or data, such as
the content of the EEPROM that configures the USB controller of their product
(containing USB vendor and product ID and some more data). But you need at least
the correct IDs in the EEPROM to make it compatible with their software. 

Please do not ask me about such data. The code presented here is interesting
enough even if used without Altera software. If you include support for this
device in your host software, please let me know, so I can mention it in this
README. And please be so kind to make your software compatible enough so it
could make use of an original Altera device as well!

Quartus, SignalTap and USB-Blaster are trademarks of Altera Corporation.
ChipScope is a trademark of Xilinx Inc.

-----------------------------------------------------------------------------

FILES: Beside this README, this repository contains the following files:

 - COPYING.txt: The text of the GNU Public License (GPL).
 - device/c51: Firmware for usb_jtag based on Cypress FX2, for SDCC compiler
 - device/fpga4u: branch of the Cypress FX2 firmware for the "FPGA4U" boards,
   please see device/fpga4u/README.txt and http://fpga4u.epfl.ch/wiki/FX2 for
   more information!
 - device/fx2.old: Alternative, now old and unsupported FX2 firmware, for Keil
   compiler
 - device/cpld: CPLD logic for usb_jtag with FT245BM + CPLD
 - device/f32x: (currently unfinished) attempt to get the firmware running on
   SiLabs F32x USB controller
 - host/eecksum: Code to compute a valid checksum for FT245BM EEPROM emulation
 - host/devtest: Small utility to do some basic tests with usb_jtag adapter
 - host/openocd: Experimental usb_jtag/USB-Blaster driver for OpenOCD
 - host/fxpush: Utility to upload HEX file to FX2/FX target using libusb

Where appropriate, the directories contain a file "README.txt" with further
information about purpose, usage and history.

-----------------------------------------------------------------------------

HISTORY: In my company, in 2006 we received first samples of a new hardware
design as a base for upcoming products; two major components of this design
being a Cypress EZ-USB FX2 USB controller and an Altera Cyclone EP1C12 FPGA.
The FX2 implements the USB interface of the product; its first task after
booting is to configure the FPGA. The FPGA configuration data is sent from the
USB host to the FX2 and then via JTAG to the FPGA.

I had the idea to make the FX2 code less specific, i.e. turn it into a
general-purpose JTAG interface and move all the code specific to FPGA
configuration away from the FX2, up to the host PC. While developing a stripped
down FX2 firmware, it occured to me that its function in the end would become
similar to that of the USB JTAG adapter developed by Altera; just direct FPGA
signal control (bit banging) and fast parallel/serial conversion. The Altera
device obviously can't do more, because it consists of just an USB-Parallel
chip (FTDI FT245BM) and a small CPLD that can't do much more than
parallel/serial conversion for single bytes.  At that time, I took a look at
the Altera communication mechanisms and decided to write my code so it would
behave similar to their device.

-----------------------------------------------------------------------------

THANKS: After I released the first CPLD and FX2 code, Antti Lukats (xilant.com)
was so kind to reserve an ID for my project, 0x16C0 / 0x06AD.  Thanks Antti!

Jean from fpga4fun.com was the first who tried the FX2 code "outside my lab"
and kept trying until he succeeded. Thanks for trying and reporting, Jean!

There is an ongoing discussion at http://www.edaboard.com/ftopic114946.html
with valuable discussion and info from the past. Whenever possible, I check
there for new postings and try to answer questions. Thanks to all members
who contributed (and do so in future) there!

-----------------------------------------------------------------------------

COMMON ISSUES: I received a number of mails from others who used my logic with
varying success. Following are some general tips.

If it works "partially" for you, "sometimes" or "it programs the device, but
the device doesn't work afterwards", your setup maybe isn't suitable for clear
transmission of signals at several MHz. It may still be a critical timing in
the logic, but I'm using it even slightly "overclocked" at 25 MHz and haven't
had any errors since months. So I doubt it's the logic. Try reducing the clock
to the CPLD from 24 MHz to, say, 6 MHz.

When using AS mode with a FPGA connected in parallel with a configuration
device (or similar setup), don't forget to connect nCE and nCS.

Whenever I experienced problems, it was due to wrong connections, bad
pullup/pulldown resistors on the target JTAG interface, or mismatch between
logic levels (3.3/5 V) of CPLD vs. target JTAG interface.

The cable between CPLD and target should not exceed 10 cm in length.

If you want to debug the CPLD logic, you could load jtag_logic into a larger
FPGA and embed SignalTap or ChipScope to watch what's happening. 

-----------------------------------------------------------------------------

Kolja Waschk, January 2007

