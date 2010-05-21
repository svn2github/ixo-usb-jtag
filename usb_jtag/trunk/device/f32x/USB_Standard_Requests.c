//-----------------------------------------------------------------------------
// USB_Standard_Requests.c
//-----------------------------------------------------------------------------
// Copyright 2005 Silicon Laboratories, Inc.
// http://www.silabs.com
//
// Program Description:
//
// This source file contains the subroutines used to handle incoming 
// setup packets. These are called by Handle_Setup in USB_ISR.c and used for 
// USB chapter 9 compliance.
//

// How To Test:		See Readme.txt
//
//
// FID:				32X000027
// Target:			C8051F32x
// Tool chain:		Keil C51 7.50 / Keil EVAL C51
//					Silicon Laboratories IDE version 2.6
// Command Line:	 See Readme.txt
// Project Name:	 F32x_USB_Interrupt
//
//
// Release 1.3
//	  -All changes by GP
//	  -22 NOV 2005
//	  -Changed revision number to match project revision
//		No content changes to this file
//	  -Modified file to fit new formatting guidelines
//	  -Changed file name from USB_STD_REQ.c
//
// Release 1.0
//	  -Initial Revision (JS)
//	  -22 NOV 2002
//

//-----------------------------------------------------------------------------
// Includes
//-----------------------------------------------------------------------------

#include "USB_CDC_Type.h"
#include "USB_Register.h"
#include "USB_Descriptor.h"
#include "USB_Standard_Requests.h"

extern void UART_PutNibble(BYTE x);
extern void UART_PutString(char *s);
extern void UART_PutChar(char c);
extern void UART_PutHex(BYTE x);


//-----------------------------------------------------------------------------
// Externs
//-----------------------------------------------------------------------------

// These are created in USB_DESCRIPTOR.h

extern Tdevice_descriptor code DeviceDesc;
extern Tconfiguration_desc_set code ConfigDescSet;
extern BYTE code * code StringDescTable[];

extern Tsetup_buffer Setup;				// Buffer for current device request
extern bit   setup_handled;				// flag that indicates setup stage is handled or not
extern UINT  DataSize;
extern BYTE* DataPtr;

extern BYTE Ep_Status0;					// Contains status bytes for EP 0-2
extern bit Ep_StatusIN;
extern bit Ep_StatusOUT;

extern BYTE USB_State;					// Determines current usb device state

extern bit cs_Line_State_Update;		// update line state

//-----------------------------------------------------------------------------
// Global Variables
//-----------------------------------------------------------------------------

// These are response packets used for communication with host
code BYTE ONES_PACKET[2] = {0x01, 0x00};		
code BYTE ZERO_PACKET[2] = {0x00, 0x00};		

//-----------------------------------------------------------------------------
// Function prototypes
//-----------------------------------------------------------------------------

void Get_Status(void);
void Clear_Feature(void);
void Set_Feature(void);
void Set_Address(void);
void Get_Descriptor(void);
void Get_Configuration(void);
void Set_Configuration(void);
void Get_Interface(void);
void Set_Interface(void);

//-----------------------------------------------------------------------------
// SDCC suport
//-----------------------------------------------------------------------------
#if defined SDCC
#pragma nooverlay
#endif // SDCC

//-----------------------------------------------------------------------------
// Standard device request handler
//-----------------------------------------------------------------------------

void Standard_Device_Request( void )
{
	switch(Setup.bRequest)
	{
		case GET_STATUS:		Get_Status();			break;
		case CLEAR_FEATURE:		Clear_Feature();		break;
		case SET_FEATURE:		Set_Feature();			break;
		case SET_ADDRESS:		Set_Address();			break;
		case GET_DESCRIPTOR:	Get_Descriptor();		break;
		case GET_CONFIGURATION: Get_Configuration();	break;
		case SET_CONFIGURATION: Set_Configuration();	break;
		case GET_INTERFACE:		Get_Interface();		break;
		case SET_INTERFACE:		Set_Interface();		break;
		default:										break;
	}
}

//-----------------------------------------------------------------------------
// Support Subroutines
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// Get_Status
//-----------------------------------------------------------------------------
//
// Return Value : None
// Parameters	: None
//
// This routine returns a two byte status packet to the host
//
//-----------------------------------------------------------------------------

void Get_Status(void)
{
	bit aStatus;

	// Valid when wValue equals to zero, and wLength (data length) equals to 2 
	if ( (Setup.wValue.i == 0) && (Setup.wLength.i == 2) )
	{
		// Determine if recipient was device, interface, or EP
		switch( Setup.bmRequestType )					
		{
			case IN_DEVICE:					// If recipient was device
				if ( Setup.wIndex.i == 0 ) // Valid when wIndex equals to zero
				{
					// send 0x00, indicating bus power and no remote wake-up supported
					DataPtr = (BYTE*)&ZERO_PACKET;
					setup_handled = TRUE;
				}
				break;

			case IN_INTERFACE:				// See if recipient was interface						
				// Only valid if device is configured and existing interface index
				UART_PutString("INI "); UART_PutHex(Setup.wIndex.i); UART_PutString("\r\n");
				if ( (USB_State == DEV_CONFIGURED) && (Setup.wIndex.i < DSC_NUM_INTERFACE) )												
				{
					// Status packet always returns 0x00
					DataPtr = (BYTE*)&ZERO_PACKET;		
					setup_handled = TRUE;
				}
				break;

			case IN_ENDPOINT:				// See if recipient was an endpoint							
				// Make sure device is configured and index msb = 0x00
				UART_PutString("INE "); UART_PutHex(Setup.wIndex.c[MSB]); UART_PutString("\r\n");
				if ((USB_State == DEV_CONFIGURED) && (Setup.wIndex.c[MSB] == 0) )
				{
					aStatus = EP_IDLE;
					switch ( Setup.wIndex.c[LSB] )
					{
						case IN_EP1:	UART_PutString("E1I\r\n"); aStatus = Ep_StatusIN;		setup_handled = TRUE;	break;		
						case IN_EP2:	UART_PutString("E2I\r\n"); aStatus = Ep_StatusIN;		setup_handled = TRUE;	break;		
						case OUT_EP1:	UART_PutString("E1O\r\n"); aStatus = Ep_StatusOUT;		setup_handled = TRUE;	break;
						case OUT_EP2:	UART_PutString("E2O\r\n"); aStatus = Ep_StatusOUT;		setup_handled = TRUE;	break;
						default:															break;
					}
					if (aStatus == EP_HALT)
					{
						// If endpoint is halted, return 0x01,0x00
						DataPtr = (BYTE*)&ONES_PACKET;
					} else {
						// Otherwise return 0x00,0x00 to indicate endpoint active
						DataPtr = (BYTE*)&ZERO_PACKET;
					}
				}
				break;

			default:
				break;
		}
	}

	if ( setup_handled )
	{
		// Set serviced Setup Packet, Endpoint 0 intransmit mode, 
		Ep_Status0 = EP_TX;						
		DataSize = 2;						
	}
}

//-----------------------------------------------------------------------------
// Clear_Feature
//-----------------------------------------------------------------------------
//
// Return Value : None
// Parameters	: None
//
// This routine can clear Halt Endpoint features on endpoint 1 and 2.
//
//-----------------------------------------------------------------------------

void Clear_Feature()
{
	if (   (USB_State == DEV_CONFIGURED)			// Make sure device is configured
		&& (Setup.wLength.i == 0) )					// and data length set to zero.
	{
		switch( Setup.bmRequestType )					
		{
			case OUT_DEVICE:						// for device, only remote wakeup is valid
				if (   (Setup.wValue.i == DEVICE_REMOTE_WAKEUP)
					&& (Setup.wIndex.i == 0) )
				{
					// clear remote wakeup condition here
					setup_handled = TRUE;
				}
				break;

			case OUT_ENDPOINT:						// for endpoint, only endpoint halt is valid
				if (   (Setup.wValue.i == ENDPOINT_HALT)
					&& (Setup.wIndex.c[MSB] == 0) )
				{
					switch ( Setup.wIndex.c[LSB] )
					{
						case IN_EP1:
							UART_PutString("CF_IN_EP1\r\n");
							POLL_WRITE_BYTE (INDEX, 1);			// Clear feature endpoint 1 halt
							POLL_WRITE_BYTE (EINCSR1, rbInCLRDT);	// clear data toggle, SDSTL, STSTL
							Ep_StatusIN = EP_IDLE;				// Set endpoint 1 status back to idle
							setup_handled = TRUE;
							break;

						case OUT_EP1:
							UART_PutString("CF_OUT_EP1\r\n");
							POLL_WRITE_BYTE (INDEX, 1);			// Clear feature endpoint 2 halt
							POLL_WRITE_BYTE (EOUTCSR1, rbOutCLRDT); // clear data toggle, SDSTL, STSTL
							Ep_StatusOUT = EP_IDLE;				// Set endpoint 2 status back to idle
							setup_handled = TRUE;
							break;

#if !defined(C8051F326_H)
						case OUT_EP2:
							UART_PutString("CF_OUT_EP2\r\n");
							POLL_WRITE_BYTE (INDEX, 2);			// Clear feature endpoint 2 halt
							POLL_WRITE_BYTE (EOUTCSR1, rbOutCLRDT); // clear data toggle, SDSTL, STSTL
							Ep_StatusOUT = EP_IDLE;				// Set endpoint 2 status back to idle
							setup_handled = TRUE;
							break;

						case IN_EP2:
							UART_PutString("CF_IN_EP2\r\n");
							POLL_WRITE_BYTE (INDEX, 2);			// Clear feature endpoint 2 halt
							POLL_WRITE_BYTE (EINCSR1, rbInCLRDT);	// clear data toggle, SDSTL, STSTL
							Ep_StatusIN2 = EP_IDLE;				// Set endpoint 2 status back to idle
							setup_handled = TRUE;
							break;
#endif

						default:
							break;
					}
				}
				break;

			default:
				break;
		}
	}
}

//-----------------------------------------------------------------------------
// Set_Feature
//-----------------------------------------------------------------------------
//
// Return Value : None
// Parameters	: None
//
// This routine will set the EP Halt feature for endpoints 1 and 2
//
//-----------------------------------------------------------------------------

void Set_Feature(void)
{
	if (   (USB_State == DEV_CONFIGURED)			// Make sure device is configured
		&& (Setup.wLength.i == 0) )					// and data length set to zero.
	{
		switch( Setup.bmRequestType )					
		{
			case OUT_DEVICE:						// for device, only remote wakeup is valid
				if (   (Setup.wValue.i == DEVICE_REMOTE_WAKEUP)
					&& (Setup.wIndex.i == 0) )
				{
					// clear remote wakeup condition here
					setup_handled = TRUE;
				}
				break;

			case OUT_ENDPOINT:						// for endpoint, only endpoint halt is valid
				if (   (Setup.wValue.i == ENDPOINT_HALT)
					&& (Setup.wIndex.c[MSB] == 0) )
				{
					switch ( Setup.wIndex.c[LSB] )
					{
						case IN_EP1:
							UART_PutString("SF_IN_EP1\r\n");
							Ep_StatusIN = EP_HALT;				// Set endpoint 1 status to halt
							setup_handled = TRUE;
							break;

						case IN_EP2:
							UART_PutString("SF_IN_EP2\r\n");
							Ep_StatusIN = EP_HALT;				// Set endpoint 2 status to halt
							setup_handled = TRUE;
							break;

						case OUT_EP1:
							UART_PutString("SF_OUT_EP1\r\n");
							Ep_StatusOUT = EP_HALT;			// Set endpoint 2 status to halt
							setup_handled = TRUE;
							break;

						case OUT_EP2:
							UART_PutString("SF_OUT_EP2\r\n");
							Ep_StatusOUT = EP_HALT;			// Set endpoint 2 status to halt
							setup_handled = TRUE;
							break;

						default:
							break;
					}
				}
				break;

			default:
				break;
		}
	}
}

//-----------------------------------------------------------------------------
// Set_Address
//-----------------------------------------------------------------------------
//
// Return Value : None
// Parameters	: None
//
// Set new function address
//
//-----------------------------------------------------------------------------

void Set_Address(void)
{
	if (   (Setup.bmRequestType == OUT_DEVICE)		// Request must be directed to device
		&& (Setup.wIndex.i == 0)					// with index and length set to zero.
		&& (Setup.wLength.i == 0)					// wValue holds the address, up to 0x7F
		&& (Setup.wValue.c[MSB] == 0) && ((Setup.wValue.c[LSB] & 0x80) == 0) )
	{
		if (Setup.wValue.c[LSB] != 0)
		{
			POLL_WRITE_BYTE(FADDR, Setup.wValue.c[LSB]);	// write new address to FADDR
															// SIE applies this address after status stage
			USB_State = DEV_ADDRESS;		// Indicate that device state is now address
		}
		else
		{
			USB_State = DEV_DEFAULT;		// If new address was 0x00, return device to default state
		}
		setup_handled = TRUE;
	}
}

//-----------------------------------------------------------------------------
// Get_Descriptor
//-----------------------------------------------------------------------------
//
// Return Value : None
// Parameters	: None
//
// This routine sets the data pointer and size to correct 
// descriptor and sets the endpoint status to transmit
//
//-----------------------------------------------------------------------------

void Get_Descriptor(void)
{
	if ( (Setup.bmRequestType == IN_DEVICE) ) // Request must be directed to device
	{
		switch(Setup.wValue.c[MSB])				// Determine which type of descriptor
		{										// was requested, and set data ptr and
			case DST_DEVICE:					// size accordingly
				if ( (Setup.wValue.c[LSB] == 0) && (Setup.wIndex.i == 0) )
				{
					DataPtr = (BYTE*)&DeviceDesc;
					DataSize = sizeof( Tdevice_descriptor );
					setup_handled = TRUE;
				}
				break;

			case DST_CONFIG:					// wValue.LSB holds config index
				if ( (Setup.wValue.c[LSB] == 0) && (Setup.wIndex.i == 0) )
				{
					DataPtr = (BYTE*)&ConfigDescSet;
					DataSize = sizeof( Tconfiguration_desc_set );
					setup_handled = TRUE;
				}
				break;

			case DST_STRING:					// wValue.LSB holds string index
												// wIndex holds language ID
				if ( Setup.wValue.c[LSB] < DSC_NUM_STRING )
				{
					DataPtr = StringDescTable[Setup.wValue.c[LSB]];
					DataSize = *DataPtr;
					setup_handled = TRUE;
				}
				break;

			default:
				break;
		}
	}

	if ( setup_handled )
	{
		Ep_Status0 = EP_TX;							// Put endpoint in transmit mode
		if ( DataSize > Setup.wLength.i )
			DataSize = Setup.wLength.i;				// Send only requested amount of data
	}
}

//-----------------------------------------------------------------------------
// Get_Configuration
//-----------------------------------------------------------------------------
//
// Return Value : None
// Parameters	: None
//
// This routine returns current configuration value
//
//-----------------------------------------------------------------------------

void Get_Configuration(void)
{
	if (   (Setup.bmRequestType == IN_DEVICE) // This request must be directed to the device
		&& (Setup.wValue.i == 0)				// with value word set to zero
		&& (Setup.wIndex.i == 0)				// and index set to zero
		&& (Setup.wLength.i == 1) )				// and setup length set to one
	{
		if (USB_State == DEV_CONFIGURED)		// If the device is configured, then return value 0x01
		{										// since this software only supports one configuration
			DataPtr = (BYTE*)&ONES_PACKET;
			setup_handled = TRUE;
		}
		if (USB_State == DEV_ADDRESS)			// If the device is in address state, it is not
		{										// configured, so return 0x00
			DataPtr = (BYTE*)&ZERO_PACKET;
			setup_handled = TRUE;
		}
	}
	
	if ( setup_handled )
	{
		// Set serviced Setup Packet, Endpoint 0 intransmit mode
		Ep_Status0 = EP_TX;						
		DataSize = 1;						
	}
}

//-----------------------------------------------------------------------------
// Set_Configuration
//-----------------------------------------------------------------------------
//
// Return Value : None
// Parameters	: None
//
// This routine allows host to change current device configuration value
//
//-----------------------------------------------------------------------------

void Set_Configuration(void)
{
	if (   (Setup.bmRequestType == OUT_DEVICE)	// This request must be directed to the device
		&& (Setup.wIndex.i == 0)				// and index set to zero
		&& (Setup.wLength.i == 0) )				// and data length set to one
	{
		if (Setup.wValue.c[LSB] > 0)			// Any positive configuration request
		{										// results in configuration being set to 1
			USB_State = DEV_CONFIGURED;

			// The endpoint regsiters, E0CSR, EINCSRL/H and EOUTCSRL/H, are cleared by bus reset.
			// Also data toggles for the EPs are also cleared by bus reset.
			// Set_Configuration is a good place to set these registers,
			// when any alternate interface is present.
			// When the device has any alternate interface, initialize these registers in
			// Set_Interface according to selected interface

			UART_PutString("Set_Config OK\r\n");
#if defined(C8051F326_H)
			POLL_WRITE_BYTE(INDEX, 1);				// Select EP1
			POLL_WRITE_BYTE(EINCSR2, 0);			// no double buffer (64 bytes max, otherwise only 32)
			POLL_WRITE_BYTE(EINCSR1, rbInCLRDT);	// clear data toggle, SDSTL, STSTL
			POLL_WRITE_BYTE(EOUTCSR2, rbOutDBOEN);	// double buffer (2x64 bytes)
			POLL_WRITE_BYTE(EOUTCSR1, rbOutCLRDT);	// clear data toggle
#else
			POLL_WRITE_BYTE(INDEX, 1);				// Select EP1
			POLL_WRITE_BYTE(EINCSR2, rbInDBIEN | rbInSPLIT);	// split, double buffer
			POLL_WRITE_BYTE(EINCSR1, rbInCLRDT);	// clear data toggle, SDSTL, STSTL

			POLL_WRITE_BYTE(INDEX, 2);				// Select EP2
			POLL_WRITE_BYTE(EOUTCSR2, rbOutDBOEN);	// double buffer
			POLL_WRITE_BYTE(EOUTCSR1, rbOutCLRDT);	// clear data toggle
#endif
			Ep_StatusIN = EP_IDLE;
			Ep_StatusOUT = EP_IDLE;
		}
		else
		{
			UART_PutString("Set_Config NAK\r\n");
			USB_State = DEV_ADDRESS;				// Unconfigures device by setting state to
			Ep_StatusIN = EP_HALT;					// address, and changing endpoint 
			Ep_StatusOUT = EP_HALT;					// status to halt
		}

		setup_handled = TRUE;
	}
}

//-----------------------------------------------------------------------------
// Get_Interface
//-----------------------------------------------------------------------------
//
// Return Value : None
// Parameters	: None
//
// This routine returns 0x00, since no alternate interface is supported by 
// this firmware
//
//-----------------------------------------------------------------------------

void Get_Interface(void)
{
	if (   (USB_State == DEV_CONFIGURED)			// If device is configured
		&& (Setup.bmRequestType == IN_INTERFACE)	// and recipient is an interface
		&& (Setup.wValue.i == 0)					// and wValue equals to 0
		&& (Setup.wIndex.i < DSC_NUM_INTERFACE)		// and valid interface index
		&& (Setup.wLength.i == 1) )					// and data length equals to one
	{
		DataPtr = (BYTE*)&ZERO_PACKET;			// return 0x00 to host
		Ep_Status0 = EP_TX;
		DataSize = 1;
		setup_handled = TRUE;
	}
}

//-----------------------------------------------------------------------------
// Set_Interface
//-----------------------------------------------------------------------------
//
// Return Value : None
// Parameters	: None
//
// This routine allows host to change the interface to alternate one
// The request is only valid when wValue (alternate IF index) is 0,
// because this implementation has no alternate IF
//
//-----------------------------------------------------------------------------

void Set_Interface(void)
{
	// Make sure request is directed at interface and all other packet values 
	// are set to zero
	if (   (Setup.bmRequestType == OUT_INTERFACE)
		&& (Setup.wValue.i == 0 )		// wValue holds alternate interface index
		&& (Setup.wIndex.i < DSC_NUM_INTERFACE)
		&& (Setup.wLength.i == 0) )
	{
		// Indicate setup packet has been serviced
		setup_handled = TRUE;
	}
}

//-----------------------------------------------------------------------------
// End Of File
//-----------------------------------------------------------------------------
