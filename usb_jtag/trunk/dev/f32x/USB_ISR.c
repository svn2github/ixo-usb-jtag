//-----------------------------------------------------------------------------
// USB_ISR.c
//-----------------------------------------------------------------------------
// Copyright 2005 Silicon Laboratories, Inc.
// http://www.silabs.com
//
// Program Description:
//
// Source file for USB firmware. Includes top level ISR with Setup,
// and Endpoint data handlers.	Also includes routine for USB suspend,
// reset, and procedural stall.
//
//
// How To Test:		See Readme.txt
//
//
// FID:				32X000023
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
//	  -Modified file to fit new formatting guidelines
//	  -Changed file name from USB_ISR.c
//	  -Removed extraneous code that was commented out
//	  -Added USB Suspend code
//	 
// Release 1.0
//	  -Initial Revision (DM)
//	  -08 NOV 2002
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
// Global Externs
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// Global Variables
//-----------------------------------------------------------------------------

BYTE USB_State;							// Holds the current USB State
										// def. in USB_Main.h

Tsetup_buffer Setup;					// Buffer for current device request
bit   setup_handled;					// flag that indicates setup stage is handled or not
UINT  DataSize;							// Size of data to return
BYTE* DataPtr;							// Pointer to data to return
bit   send_eq_requested;				// flag that indicates that the data to send on TX
										// equals to the requested by Setup wLength

// Holds the status for each endpoint
BYTE Ep_Status0   = EP_IDLE;
bit Ep_StatusIN   = EP_HALT;
bit Ep_StatusOUT  = EP_HALT;

//-----------------------------------------------------------------------------
// Interrupt Service Routines
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// Usb_ISR
//-----------------------------------------------------------------------------
//
// Called after any USB type interrupt, this handler determines which type
// of interrupt occurred, and calls the specific routine to handle it.
//
//-----------------------------------------------------------------------------
void Usb_ISR(void) interrupt 8			// Top-level USB ISR
{
	BYTE bCommon, bIn, bOut;
	POLL_READ_BYTE(CMINT, bCommon);		// Read all interrupt registers
	POLL_READ_BYTE(IN1INT, bIn);		// this read also clears the register
	POLL_READ_BYTE(OUT1INT, bOut);
	{
/*
		if (bCommon & rbRSUINT)			// Handle Resume interrupt
		{
			Usb_Resume();
		}
*/
		if (bCommon & rbRSTINT)			// Handle Reset interrupt
		{
			Usb_Reset();
		}
		if (bCommon & rbSOF)			// SOF interrupt
		{
			Handle_EP_HALT();
		}
		if (bIn & rbEP0)				// Handle Setup packet received
		{								// or packet transmitted if Endpoint 0
			Handle_Setup();				// is transmit mode
		}
		if (bCommon & rbSUSINT)			// Handle Suspend interrupt
		{
			Usb_Suspend();
		}
	}
}

//-----------------------------------------------------------------------------
// Support Routines for ISR
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// SDCC suport
//-----------------------------------------------------------------------------
#if defined SDCC
#pragma nooverlay
#endif // SDCC

//-----------------------------------------------------------------------------
// Usb_Reset
//-----------------------------------------------------------------------------
//
// Return Value : None
// Parameters	: None
//
// - Set state to default
// - Clear Usb Inhibit bit
//
//-----------------------------------------------------------------------------

void Usb_Reset(void)
{
	POLL_WRITE_BYTE(IN1IE,	0x01);		// Disable all IN and OUT EP interrupt except Ep0
	POLL_WRITE_BYTE(OUT1IE, 0x00);
	POLL_WRITE_BYTE(CMIE,	0x0D);		// Enable SOF, Reset, Suspend interrupts

	POLL_WRITE_BYTE(POWER, 0x01);		// Clear usb inhibit bit to enable USB
										// suspend detection

	USB_State = DEV_DEFAULT;			// Set device state to default

	Ep_Status0 = EP_IDLE;				// Set default Endpoint Status
	Ep_StatusIN  = EP_HALT;
	Ep_StatusOUT = EP_HALT;
}

//-----------------------------------------------------------------------------
// Handle_Setup
//-----------------------------------------------------------------------------
//
// Return Value : None
// Parameters	: None
//
// - Decode Incoming Setup requests
// - Load data packets on fifo while in transmit mode
//
//-----------------------------------------------------------------------------

void Handle_Setup(void)
{
	BYTE ControlReg, TempReg, dataCount;	// Temporary storage for EP control
										// register

	POLL_WRITE_BYTE(INDEX, 0);			// Set Index to Endpoint Zero
	POLL_READ_BYTE(E0CSR, ControlReg);	// Read control register

	if (ControlReg & rbSTSTL)			// If last packet was a sent stall, reset
	{									// STSTL bit and return EP0 to idle state
		POLL_WRITE_BYTE(E0CSR, 0);
		Ep_Status0 = EP_IDLE;
		return;
	}

	if (ControlReg & rbSUEND)			// SUEND bit is asserted after status stage by SIE
	{									// or when SIE receives early SETUP
		POLL_WRITE_BYTE(E0CSR, rbSSUEND);	// Serviced Setup End bit and return to EP_IDLE
		Ep_Status0 = EP_IDLE;
	}

	if (Ep_Status0 == EP_IDLE)			// If Endpoint 0 is in idle mode
	{
		if (ControlReg & rbOPRDY)		// Make sure that EP 0 has an Out Packet ready from host
		{								// although if EP0 is idle, this should always be the case
			Fifo_Read(FIFO_EP0, 8, (BYTE *)&Setup);
										// Get Setup Packet off of Fifo
#if defined BIG_ENDIAN
										// As the USB custom, multi-byte number is LSB first - little endian
			Setup.wValue.i  = ((UINT)Setup.wValue.c[1]  << 8) | Setup.wValue.c[0];
			Setup.wIndex.i  = ((UINT)Setup.wIndex.c[1]  << 8) | Setup.wIndex.c[0];
			Setup.wLength.i = ((UINT)Setup.wLength.c[1] << 8) | Setup.wLength.c[0];
#endif // end of BIG_ENDIAN

			UART_PutString("SETUP:");
			UART_PutHex( Setup.bmRequestType );
			UART_PutHex( Setup.bRequest );
			UART_PutString("\r\n");

			setup_handled = FALSE;
			switch ( Setup.bmRequestType & DRT_MASK )	// Device Request Type
			{
				case DRT_STD:							// Standard device request
					Standard_Device_Request();
					break;
				case DRT_CLASS:
					Class_Request();
					break;
				case DRT_VENDOR:
					Vendor_Request();
					break;
				default:
					break;
			}

			POLL_WRITE_BYTE(INDEX, 0);					// Assure the indexed registers are accessed correctly
			if ( setup_handled )
			{
				POLL_WRITE_BYTE(E0CSR, rbSOPRDY);		// Tell to SIE that the setup is handled
			} else {
														// Return STALL to the host
				POLL_WRITE_BYTE(E0CSR, rbSDSTL);		// Set the send stall bit
				Ep_Status0 = EP_STALL;					// Put the endpoint in stall status
			}

			send_eq_requested = (DataSize == Setup.wLength.i);	// get this flag before DataSize 
																// is reduced in TX cycle
		}
	} // end of EP_IDLE

	if ( Ep_Status0 == EP_TX )			// See if the endpoint has data to transmit to host
	{
		if ( !(ControlReg & rbINPRDY) ) // Make sure you don't overwrite last packet
		{			
			POLL_READ_BYTE( E0CSR, ControlReg );
										// Read control register

			if ( (!(ControlReg & rbSUEND)) || (!(ControlReg & rbOPRDY)) )
										// Check to see if Setup End or Out Packet received, if so
										// do not put any new data on FIFO
			{
				TempReg = rbINPRDY;		// Add In Packet ready flag to E0CSR bitmask
										// Break Data into multiple packets if larger than Max Packet
				if (DataSize >= EP0_PACKET_SIZE)
				{									// The data size to send in this cycle is
													// just EP0_PACKET_SIZE
					Fifo_Write( FIFO_EP0, EP0_PACKET_SIZE, (BYTE *)DataPtr );
					DataPtr += EP0_PACKET_SIZE;			// Advance data pointer
					DataSize -= EP0_PACKET_SIZE;		// Decrement data size
					if ( send_eq_requested && (DataSize == 0) )	// In this case, no need to send ZLP,
					{											// finish TX immediately
						TempReg |= rbDATAEND;
						Ep_Status0 = EP_IDLE;	
					}
				} else {							// The data size to send in this cycle is
													// smaller than EP0_PACKET_SIZE or zero (ZLP)
					Fifo_Write( FIFO_EP0, (BYTE)DataSize, (BYTE *)DataPtr );
					TempReg |= rbDATAEND;
					Ep_Status0 = EP_IDLE;
				}
				POLL_WRITE_BYTE(E0CSR, TempReg);		// Write mask to E0CSR
			}
		}
	} // end of EP_TX

	if (Ep_Status0 == EP_RX)			// See if endpoint should receive
	{
		POLL_READ_BYTE( E0CSR, ControlReg );	// Read control register
		if ( ControlReg & rbOPRDY )				// Verify packet was received
		{
			ControlReg = rbSOPRDY;

			POLL_READ_BYTE( E0CNT, dataCount );	// get data bytes on the FIFO
			Fifo_Read( FIFO_EP0, dataCount, (BYTE*)DataPtr );
											// Empty the FIFO
												// FIFO must be read out just the size it has,
												// otherwize the FIFO pointer on the SIE goes out of synch.
			DataPtr += dataCount;			// Advance the pointer
			if ( DataSize > dataCount )		// Update the scheduled number to be received
				DataSize -= dataCount;
			else {							// Meet the end of the data stage
				DataSize = 0;
				ControlReg |= rbDATAEND;	// Signal end of data stage
				Ep_Status0 = EP_IDLE;		// Set Endpoint to IDLE
			}

			POLL_WRITE_BYTE ( E0CSR, ControlReg );
		}
	} // end of EP_RX

}

//-----------------------------------------------------------------------------
// Usb_Suspend
//-----------------------------------------------------------------------------
//
// Return Value : None
// Parameters	: None
//
// Enter suspend mode after suspend signalling is present on the bus
//
//-----------------------------------------------------------------------------

void Usb_Suspend(void)
{			
	// Put the device in a low power configuration
#ifndef C8051F326_H
	P0MDIN	= 0x00;						// Port 0 configured as analog input
	P1MDIN	= 0x00;						// Port 1 configured as analog input
	P2MDIN	= 0x00;						// Port 2 configured as analog input
	P3MDIN	= 0x00;						// Port 3 configured as analog input
#endif

//	ADC0CN &= ~0x80;					// Disable ADC0
//	REF0CN	= 0x00;						// Disable voltage reference

	OSCICN |= 0x20;						// Put oscillator to suspend

	// When the device receives a non-idle USB event, it will resume execution
	// on the instruction that follows OSCICN |= 0x20.		

	// Re-enable everything that was disabled when going into Suspend
#if defined C8051F320_H
	P0MDIN	= 0xFF;						// Port 0 configured as diital input
	P1MDIN	= 0x7F;						// Port 1 pin 7 set as analog input
	P2MDIN	= 0xFF;						// Port 2 configured as diital input
	P3MDIN	= 0xFF;						// Port 3 configured as diital input
#endif // C8051F320_H

#if defined C8051F340_H
	P0MDIN	= 0xFF;						// Port 0 configured as diital input
	P1MDIN	= 0xFF;						// Port 1 configured as diital input
	P2MDIN  = ~(0x20);					// Port 2 pin 5 set as analog input
	P3MDIN	= 0xFF;						// Port 3 configured as diital input
#endif // C8051F340_H


//	REF0CN	= 0x0E;						// Enable voltage reference VREF
//	ADC0CN |= 0x80;						// Re-enable ADC

}

//-----------------------------------------------------------------------------
// Usb_Resume
//-----------------------------------------------------------------------------
//
// Return Value : None
// Parameters	: None
//
// Resume normal USB operation
//
//-----------------------------------------------------------------------------
/*
void Usb_Resume(void)
{
	volatile int k;

	k++;

	// Add code for resume
}
*/

//-----------------------------------------------------------------------------
// Handle_EP_HALT
//-----------------------------------------------------------------------------
//
// Return Value : None
// Parameters	: None
//
// Handle endpoint HALT status, except EP0
//
//-----------------------------------------------------------------------------

void Handle_EP_HALT(void)
{
	BYTE ControlReg;

	// IN EP1
	POLL_WRITE_BYTE(INDEX, 1);				// Set index to endpoint 1 registers
	POLL_READ_BYTE(EINCSR1, ControlReg);	// Read contol register for EP
	if (Ep_StatusIN == EP_HALT)			// If endpoint is currently halted, send a stall
	{
		POLL_WRITE_BYTE(EINCSR1, rbInSDSTL);
	}
	else
	{
		if (ControlReg & rbInSTSTL)			// Clear sent stall if last packet returned a stall
		{
			POLL_WRITE_BYTE(EINCSR1, 0);
		}
	}

	// OUT EP
#if !defined(C8051F326_H)
	POLL_WRITE_BYTE(INDEX, 2);				// Set index to endpoint 2 registers
#endif
	POLL_READ_BYTE(EOUTCSR1, ControlReg);	// Read contol register for EP
	if (Ep_StatusOUT == EP_HALT)			// If endpoint is halted, send a stall
	{
		POLL_WRITE_BYTE(EOUTCSR1, rbOutSDSTL | rbOutOPRDY);	// preserve OPRDY bit
	}
	else
	{
		if (ControlReg & rbOutSTSTL)	// Clear sent stall bit if last packet was a stall
		{
			POLL_WRITE_BYTE(EOUTCSR1, rbOutOPRDY);			// preserve OPRDY bit
		}
	}

}

//-----------------------------------------------------------------------------
// Fifo_Read
//-----------------------------------------------------------------------------
//
// Return Value : None
// Parameters	:
//					1) BYTE addr : target address
//					2) BYTE uNumBytes : number of bytes to unload
//					3) BYTE * pData : read data destination
//
// Read from the selected endpoint FIFO
//
//-----------------------------------------------------------------------------

void Fifo_Read(BYTE addr, BYTE uNumBytes, BYTE * pData)
{
	if (uNumBytes)							// Check if >0 bytes requested,
	{
		if ( --uNumBytes == 0 )
		{
			USB0ADR = addr | 0x80;			// Set address and initiate read
			while(USB0ADR & 0x80);			// Wait for BUSY->'0' (data ready)
			*pData = USB0DAT;				// Copy data byte
			return;
		}
		else
		{
			USB0ADR = addr | 0xC0;			// Set address
											// Set auto-read and initiate
											// first read

			// Unload <NumBytes> from the selected FIFO
			do
			{
				while(USB0ADR & 0x80);		// Wait for BUSY->'0' (data ready)
				//*pData++ = USB0DAT;		// Copy data byte
				*pData = USB0DAT;			// splitting it result better asm code (Keil)
				pData++;
			} while ( --uNumBytes != 0 );

			while(USB0ADR & 0x80);			// Wait for BUSY->'0' (data ready)
			USB0ADR = 0;					// Clear auto-read
			*pData = USB0DAT;				// Copy data byte
		}
	}
}

//-----------------------------------------------------------------------------
// Fifo_Write
//-----------------------------------------------------------------------------
//
// Return Value : None
// Parameters	:
//					1) BYTE addr : target address
//					2) BYTE uNumBytes : number of bytes to unload
//					3) BYTE * pData : location of source data
//
// Write to the selected endpoint FIFO
//
//-----------------------------------------------------------------------------
void Fifo_Write(BYTE addr, BYTE uNumBytes, BYTE * pData)
{
	// If >0 bytes requested,
	if (uNumBytes)
	{
		while(USB0ADR & 0x80);				// Wait for BUSY->'0'
											// (register available)
		USB0ADR = (addr);					// Set address (mask out bits7-6)

		// Write <NumBytes> to the selected FIFO
		do
		{
//			USB0DAT = *pData++;				// splitting it results shorter code size
			USB0DAT = *pData;
			pData++;
			while(USB0ADR & 0x80);			// Wait for BUSY->'0' (data ready)
		} while ( --uNumBytes != 0 );
	}
}

//-----------------------------------------------------------------------------
// POLL_READ_BYTE, POLL_WRITE_BYTE
//-----------------------------------------------------------------------------
// When the macros are not defined, provide them as functions
//-----------------------------------------------------------------------------

#if defined( POLL_READ_BYTE_DEF )

BYTE POLL_READ_BYTE_FUNC( BYTE addr )
{
	while( USB0ADR & 0x80 );
	USB0ADR = (0x80 | addr);
	while( USB0ADR & 0x80 );
	return USB0DAT;
}

#endif

#if !defined( POLL_WRITE_BYTE )

void POLL_WRITE_BYTE( BYTE addr, BYTE dt )
{
	while(USB0ADR & 0x80);
	USB0ADR = addr;
	USB0DAT = dt;
}

#endif

//-----------------------------------------------------------------------------
// End Of File
//-----------------------------------------------------------------------------
