#include <stdio.h>
#include <stdlib.h>
#include <ftdi.h>
#include <stdint.h>

struct ftdi_context fc;

#define ALTERA_VID 0x09FB
#define BLASTER_PID 0x6001
#define USBJTAG_VID 0x16C0
#define USBJTAG_PID 0x06AD

void dev_deinit(void)
{ 
  ftdi_deinit(&fc);
}

void dev_close(void)
{
  ftdi_usb_close(&fc);
}

void dev_reset(void)
{
  ftdi_usb_reset(&fc);
}

int dev_error (char *s)
{
  fprintf (stderr, "%s: %s\n", s, ftdi_get_error_string(&fc));
  return -1;
}

int eeprom_test(void)
{
  uint8_t eeprom[128];

  if(ftdi_read_eeprom(&fc, eeprom) < 0)
  {
    (void)dev_error("ftdi_read_eeprom failed");
    return;
  };

  printf("=== EEPROM test ===\n");
  printf("  VID 0x%04X\n", eeprom[2] + (eeprom[3]<<8));
  printf("  PID 0x%04X\n", eeprom[4] + (eeprom[5]<<8));
  printf("  HW Version 0x%04X\n", eeprom[6] + (eeprom[7]<<8));
  printf("  USB Flags 0x%02X\n", eeprom[8]);
  printf("  Power 0x%02X\n", eeprom[9]);
  printf("  FTDI Flags 0x%02X\n", eeprom[10]);

  return 0;
}

int jtag_test(void)
{
  printf("=== JTAG test ===\n  not implemented yet\n");
  return 0;
}

int asmi_test(void)
{
  printf("=== ASMI test ===\n  not implemented yet\n");
  return 0;
}

int main(int argc, char *argv[])
{
  ftdi_init(&fc);

  atexit(dev_deinit);

  printf("=== Search ixo.de adapter ===\n");

  if(ftdi_usb_open(&fc, USBJTAG_VID, USBJTAG_PID) < 0)
  {
    (void)dev_error("ftdi_usb_open failed");
  
    printf("=== Search Altera adapter ===\n");
    if(ftdi_usb_open(&fc, ALTERA_VID, BLASTER_PID) < 0)
    {
      return dev_error("ftdi_usb_open failed");
    };
  };

  atexit(dev_close);

  if(eeprom_test() < 0) return -1;

  if(ftdi_enable_bitbang(&fc, 0xFF) < 0)
  {
    return dev_error("Can't enable bitbang"); 
  };

  atexit(dev_reset);

  if(jtag_test() < 0) return -1;
  if(asmi_test() < 0) return -1;

  return 0;
}
