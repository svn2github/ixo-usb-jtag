/*
   Utility to upload HEX file to FX2/FX target using libusb

   Runs with Cygwin, GCC, libusb-win32 from http://libusb-win32.sourceforge.net

   The ezusb_* code here is from Stephan Meyer (ste_meyer, web.de), as posted in
     http://osdir.com/ml/lib.libusb.devel.windows/2005-02/msg00001.html or
     http://sourceforge.net/mailarchive/message.php?msg_id=667335292%40web.de
*/

#include <inttypes.h>
#include <stdio.h>
#include <usb.h>
#include <stdio.h>


#if 0
#define EZUSB_CS_ADDRESS 0x7F92 /* FX */
#else
#define EZUSB_CS_ADDRESS 0xE600 /* FX2 */
#endif

#define MAX_HEX_RECORD_LENGTH 16

typedef struct
{
  uint32_t length;
  uint32_t address;
  uint32_t type;
  uint8_t data[MAX_HEX_RECORD_LENGTH];
} hex_record;


static int ezusb_read_hex_record(hex_record *record, FILE *file); 
static int ezusb_load(usb_dev_handle *dev, uint32_t address, uint32_t length, uint8_t *data);

static int ezusb_read_hex_record(hex_record *record, FILE *file)
{
  char c;
  uint32_t i, length, read, tmp, checksum;

  if(feof(file))
  {
    return 0;
  }

  c = getc(file);

  if(c != ':')
  {
    return 0;
  }

  read = fscanf(file, "%2X%4X%2X", &record->length, &record->address, &record->type);

  if(read != 3)
  {
    return 0;
  }

  checksum = record->length + (record->address >> 8) + record->address + record->type;

  length = record->length;

  if(length > MAX_HEX_RECORD_LENGTH)
  {
    return 0;
  }

  for(i = 0; i < length; i++)
  {
    read = fscanf(file, "%2X", &tmp);

    if(read != 1)
    {
      return 0;
    }

    record->data[i] = (uint8_t)tmp;
    checksum += tmp;
  }

  read = fscanf(file, "%2X\n", &tmp);

  if((read != 1) || (((uint8_t)(checksum + tmp)) != 0x00))
  {
    return 0;
  }

  return 1;
}

static int ezusb_load(usb_dev_handle *dev, uint32_t address, uint32_t length, uint8_t *data)
{
  if(usb_control_msg(dev, 0x40, 0xA0, address, 0, data, length, 5000) <= 0)
  {
    return 0;
  }

  return 1;
}


int ezusb_load_firmware(usb_dev_handle *dev, const char *hex_file)
{
  uint8_t ezusb_cs;
  FILE *firmware;
  hex_record record;

  firmware = fopen(hex_file, "r");

  if(!firmware)
  {
    return 0;
  }

  ezusb_cs = 1;

  if(!ezusb_load(dev, EZUSB_CS_ADDRESS, 1, &ezusb_cs))
  {
    return 0;
  }

  while(ezusb_read_hex_record(&record, firmware))
  {
    if(record.type != 0)
    {
      break;
    }

    printf("Load L=%X @=%X\n", record.length, record.address);

    if(!ezusb_load(dev, record.address, record.length, record.data))
    {
      return 0;
    }
  }

  ezusb_cs = 0;

  ezusb_load(dev, EZUSB_CS_ADDRESS, 1, &ezusb_cs);

  fclose(firmware);

  return 1;
}

#define MY_VID 0x0D06
#define MY_PID 0x0202

usb_dev_handle *open_dev(void)
{
  struct usb_bus *bus;
  struct usb_device *dev;

  for(bus = usb_get_busses(); bus; bus = bus->next) 
  {
    for(dev = bus->devices; dev; dev = dev->next) 
    {
      if(dev->descriptor.idVendor == 0x0D06) // telos
      {
        return usb_open(dev);
      };
      if(dev->descriptor.idVendor == 0x09FB) // Altera
      {
        return usb_open(dev);
      };
    }
  }
  return NULL;
}

int main(int argc, char *argv[])
{
  usb_dev_handle *dev = NULL; /* the device handle */

  if(argc<2)
  {
    fprintf(stderr, "error: no hex file specified\n");
    return -1;
  };

  usb_init(); /* initialize the library */
  usb_find_busses(); /* find all busses */
  usb_find_devices(); /* find all connected devices */

  if(!(dev = open_dev()))
  {
    printf("error: device not found!\n");
    return -1;
  }

  if(!ezusb_load_firmware(dev, argv[1]))
  {
    printf("error: ezusb_load_firmware failed\n");
    return -1;
  }

  usb_release_interface(dev, 0);
  usb_close(dev);

  return 0;
}
