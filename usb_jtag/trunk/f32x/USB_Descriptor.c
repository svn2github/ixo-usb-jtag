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
	0x02,				// bDeviceClass (Communication Class)
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
	0x0F				// bMaxPower
  },
  {					// Interface(0) - Communication Class
	sizeof(Tinterface_descriptor),	// bLength
	DSC_TYPE_INTERFACE,	// bDescriptorType
	0x00,				// bInterfaceNumber
	0x00,				// bAlternateSetting
	0x01,				// bNumEndpoints
	0x02,				// bInterfaceClass (Communication Class)
	0x02,				// bInterfaceSubClass (Abstract Control Model)
	0x01,				// bInterfaceProcotol (V.25ter, Common AT commands)
	0x00				// iInterface
  },
  {					// Header Functional Descriptor
	sizeof(Theader_func_descriptor),	// bLength
	DSC_TYPE_CS_INTERFACE,		// bDescriptorType (CS_INTERFACE)
	DSC_SUBTYPE_CS_HEADER_FUNC,	// bDescriptorSubtype (Header Functional)
	LE(0x0110)					// bcdCDC (CDC spec release number, 1.1)
  },
  {					// Call Management Functional Descriptor
	sizeof(Tcall_man_func_descriptor),	// bLength
	DSC_TYPE_CS_INTERFACE,		// bDescriptorType (CS_INTERFACE)
	DSC_SUBTYPE_CS_CALL_MAN,	// bDescriptorSubtype (Call Management)
	0x01,						// bmCapabilities (only over Communication Class IF / handles itself)
	0x01						// bDataInterface (Interface number of Data Class interface)
  },
  {					// Abstract Control Management Functional Descriptor
	sizeof(Tabst_control_mana_descriptor),	// bLength
	DSC_TYPE_CS_INTERFACE,		// bDescriptorType (CS_INTERFACE)
	DSC_SUBTYPE_CS_ABST_CNTRL,	// bDescriptorSubtype (Abstract Control Management)
	0x06						// bmCapabilities (Supports Send_Break, Set_Line_Coding, Set_Control_Line_State,
								//					Get_Line_Coding, and the notification Serial_State)
  },
  {					// Union Functional Descriptor
	sizeof(Tunion_func_descriptor),	// bLength
	DSC_TYPE_CS_INTERFACE,		// bDescriptorType (CS_INTERFACE)
	DSC_SUBTYPE_CS_UNION_FUNC,	// bDescriptorSubtype (Union Functional)
	0x00,						// bMasterInterface (Interface number master interface in the union)
	0x01						// bSlaveInterface0 (Interface number slave interface in the union)
  },
  {					// Endpoint1
	sizeof(Tendpoint_descriptor),	// bLength
	DSC_TYPE_ENDPOINT,	// bDescriptorType
	IN_EP1,				// bEndpointAddress
	DSC_EP_INTERRUPT,	// bmAttributes
	LE( EP1_PACKET_SIZE ),	// MaxPacketSize
	1					// bInterval
  },
  {					// Interface(1) - Data Interface Class
	sizeof(Tinterface_descriptor),	// bLength
	DSC_TYPE_INTERFACE, // bDescriptorType
	0x01,				// bInterfaceNumber
	0x00,				// bAlternateSetting
	0x02,				// bNumEndpoints
	0x0A,				// bInterfaceClass (Data Interface Class)
	0x00,				// bInterfaceSubClass
	0x00,				// bInterfaceProcotol (No class specific protocol required)
	0x00				// iInterface
  },
  {					// Endpoint IN 2 descriptor
	sizeof(Tendpoint_descriptor),	// bLength
	DSC_TYPE_ENDPOINT,	// bDescriptorType
	IN_EP2,				// bEndpointAddress
	DSC_EP_BULK,		// bmAttributes
	LE( EP2_PACKET_SIZE ), // MaxPacketSize
	0					// bInterval
  },
  {					// Endpoint OUT 2 descriptor
	sizeof(Tendpoint_descriptor),	// bLength
	DSC_TYPE_ENDPOINT,	// bDescriptorType
	OUT_EP2,			// bEndpointAddress
	DSC_EP_BULK,		// bmAttributes
	LE( EP2_PACKET_SIZE ), // MaxPacketSize
	0					// bInterval
  }
}; //end of Configuration

#define STR0LEN 4

BYTE code String0Desc[STR0LEN] =
{
	STR0LEN, DSC_TYPE_STRING, 0x09, 0x04
}; //end of String0Desc

#define STR1LEN sizeof("SILICON LABORATORIES INC.")*2

BYTE code String1Desc[STR1LEN] =
{
	STR1LEN, DSC_TYPE_STRING,
	'S', 0,
	'I', 0,
	'L', 0,
	'I', 0,
	'C', 0,
	'O', 0,
	'N', 0,
	' ', 0,
	'L', 0,
	'A', 0,
	'B', 0,
	'O', 0,
	'R', 0,
	'A', 0,
	'T', 0,
	'O', 0,
	'R', 0,
	'I', 0,
	'E', 0,
	'S', 0,
	' ', 0,
	'I', 0,
	'N', 0,
	'C', 0,
	'.', 0
}; //end of String1Desc

#define STR2LEN sizeof("C8051F320 Development Board")*2

BYTE code String2Desc[STR2LEN] =
{
	STR2LEN, DSC_TYPE_STRING,
	'C', 0,
	'8', 0,
	'0', 0,
	'5', 0,
	'1', 0,
	'F', 0,
	'3', 0,
	'2', 0,
	'0', 0,
	' ', 0,
	'D', 0,
	'e', 0,
	'v', 0,
	'e', 0,
	'l', 0,
	'o', 0,
	'p', 0,
	'm', 0,
	'e', 0,
	'n', 0,
	't', 0,
	' ', 0,
	'B', 0,
	'o', 0,
	'a', 0,
	'r', 0,
	'd', 0
}; //end of String2Desc

// serial number string

#define STR3LEN sizeof("0001")*2

BYTE code String3Desc[STR3LEN] =
{
	STR3LEN, DSC_TYPE_STRING,
	'0', 0,
	'0', 0,
	'0', 0,
	'1', 0
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
