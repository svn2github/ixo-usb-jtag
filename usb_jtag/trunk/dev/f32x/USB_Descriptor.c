//-----------------------------------------------------------------------------
// USB_Descriptor.c
//-----------------------------------------------------------------------------
// Copyright 2005 Silicon Laboratories, Inc.
// http://www.silabs.com
//
// Program Description:
//
// Source file for USB firmware. Includes descriptor data.
//
//
// How To Test:		See Readme.txt
//
//
// FID:				32X000021
// Target:			C8051F32x
// Tool chain:		Keil C51 7.50 / Keil EVAL C51
//					Silicon Laboratories IDE version 2.6
// Command Line:	See Readme.txt
// Project Name:	F32x_USB_Interrupt
//
//
// Release 1.3
//	  -All changes by GP
//	  -22 NOV 2005
//	  -Changed revision number to match project revision
//		No content changes to this file
//	  -Modified file to fit new formatting guidelines
//	  -Changed file name from USB_DESCRIPTOR.c
//
// Release 1.0
//	  -Initial Revision (DM)
//	  -22 NOV 2002
//

//-----------------------------------------------------------------------------
// Includes
//-----------------------------------------------------------------------------

#include "USB_CDC_Type.h"
#include "USB_Descriptor.h"

//-----------------------------------------------------------------------------
// Macro for two byte field
//-----------------------------------------------------------------------------

#if defined BIG_ENDIAN
#define LE(x)	((((x)&0x00FF)<<8)|(((x)&0xFF00)>>8))	// convert to little endian
#else
#define LE(x)	(x)										// no conversion
#endif

//-----------------------------------------------------------------------------
// Descriptor Declarations
//-----------------------------------------------------------------------------

// Device descriptor
Tdevice_descriptor code DeviceDesc =
{
	sizeof(Tdevice_descriptor),	// bLength
	DSC_TYPE_DEVICE,	// bDescriptorType
	LE( VER_USB ),		// bcdUSB
	0x00,				// bDeviceClass (Communication Class)
	0x00,				// bDeviceSubClass
	0x00,				// bDeviceProtocol
	EP0_PACKET_SIZE,	// bMaxPacketSize0
	LE( VID ),			// idVendor
	LE( PID ),			// idProduct
	LE( DEV_REV ),		// bcdDevice
	0x01,				// iManufacturer
	0x02,				// iProduct
	0x03,				// iSerialNumber
	0x01				// bNumConfigurations
}; //end of DeviceDesc

// Configuration
Tconfiguration_desc_set code ConfigDescSet =
{
  {					// Configuration descriptor
	sizeof(Tconfiguration_descriptor),	// bLength
	DSC_TYPE_CONFIG,	// bDescriptorType
	LE( sizeof(Tconfiguration_desc_set) ),// bTotalLength
	DSC_NUM_INTERFACE,	// bNumInterfaces
	0x01,				// bConfigurationValue
	0x00,				// iConfiguration
	0x80,				// bmAttributes
	0x4B				// bMaxPower 2*75
  },
  {					// Interface(0) 
	sizeof(Tinterface_descriptor),	// bLength
	DSC_TYPE_INTERFACE,	// bDescriptorType
	0x00,				// bInterfaceNumber
	0x00,				// bAlternateSetting
	0x02,				// bNumEndpoints
	0xFF,				// bInterfaceClass (Communication Class)
	0xFF,				// bInterfaceSubClass (Abstract Control Model)
	0xFF,				// bInterfaceProcotol (V.25ter, Common AT commands)
	0x02				// iInterface
  },
  {					// Endpoint IN
	sizeof(Tendpoint_descriptor),	// bLength
	DSC_TYPE_ENDPOINT,	// bDescriptorType
	IN_EP1,				// bEndpointAddress
	DSC_EP_BULK,		// bmAttributes
	LE( EP1_PACKET_SIZE ),	// MaxPacketSize
	0					// bInterval
  },
  {					// Endpoint OUT
	sizeof(Tendpoint_descriptor),	// bLength
	DSC_TYPE_ENDPOINT,	// bDescriptorType
	OUT_EP1,			// bEndpointAddress
	DSC_EP_BULK,		// bmAttributes
	LE( EP1_PACKET_SIZE ), // MaxPacketSize
	0					// bInterval
  }

}; //end of Configuration

#define STR0LEN 4

BYTE code String0Desc[STR0LEN] =
{
	STR0LEN, DSC_TYPE_STRING, 0x09, 0x04
}; //end of String0Desc

#define STR1LEN sizeof("ixo.de")*2

BYTE code String1Desc[STR1LEN] =
{
	STR1LEN, DSC_TYPE_STRING,
	'i', 0,
	'x', 0,
	'o', 0,
	'.', 0,
	'd', 0,
	'e', 0,
}; //end of String1Desc

#define STR2LEN sizeof("USB-JTAG-IF")*2

BYTE code String2Desc[STR2LEN] =
{
	STR2LEN, DSC_TYPE_STRING,
	'U', 0,
	'S', 0,
	'B', 0,
	'-', 0,
	'J', 0,
	'T', 0,
	'A', 0,
	'G', 0,
	'-', 0,
	'I', 0,
	'F', 0,
}; //end of String2Desc

// serial number string

#define STR3LEN sizeof("00000000")*2

BYTE code String3Desc[STR3LEN] =
{
	STR3LEN, DSC_TYPE_STRING,
	'0', 0,
	'0', 0,
	'0', 0,
	'0', 0,
	'0', 0,
	'0', 0,
	'0', 0,
	'0', 0
}; //end of String3Desc

BYTE code * code StringDescTable[ DSC_NUM_STRING ] =
{
	String0Desc,
	String1Desc,
	String2Desc,
	String3Desc
};

//-----------------------------------------------------------------------------
// End Of File
//-----------------------------------------------------------------------------
