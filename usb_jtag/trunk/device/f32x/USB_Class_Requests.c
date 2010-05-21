//-----------------------------------------------------------------------------
// USB_Class_Requests.c
//-----------------------------------------------------------------------------
//
//	Handler for class specific request and Vendor request

//-----------------------------------------------------------------------------
// Includes
//-----------------------------------------------------------------------------

#include "USB_CDC_Type.h"
#include "USB_Register.h"
#include "USB_Descriptor.h"
#include "USB_Standard_Requests.h"
#include "USB_CDC_UART.h"
#include "FIFO_RW.h"

#include "EEPROM.c"

//-----------------------------------------------------------------------------
// Constant
//-----------------------------------------------------------------------------

// CDC ACM vendor specifc requests
#define VR_FLOW_CONTROL		0x01	// flow control

//-----------------------------------------------------------------------------
// Externs
//-----------------------------------------------------------------------------

extern Tsetup_buffer Setup;				// Buffer for current device request
extern bit   setup_handled;				// flag that indicates setup stage is handled or not
extern UINT  DataSize;
extern BYTE* DataPtr;

extern BYTE USB_State;					// Holds the current USB State
										// def. in USB_Main.h
extern BYTE Ep_Status0;
extern bit Ep_StatusIN;
extern bit Ep_StatusOUT;

extern Ttxbuffer TXBuffer[ TXBUFSIZE ];	// Ring buffer for TX and RX
extern Trxbuffer RXBuffer[ RXBUFSIZE ];
extern Ttxbuffer * TXWTPtr;
extern Trxbuffer * RXRDPtr;

extern void UART_PutNibble(BYTE x);
extern void UART_PutString(char *s);
extern void UART_PutChar(char c);
extern void UART_PutHex(BYTE x);

//-----------------------------------------------------------------------------
// Global Variables
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// Function prototypes
//-----------------------------------------------------------------------------

void CS_Send_Encapsulated_Command(void);
void CS_Get_Encapsulated_Command(void);
void CS_Set_Line_Coding(void);
void CS_Get_Line_Coding(void);
void CS_Set_ControlLine_State(void);
void CS_Send_Break(void);

#if defined BIG_ENDIAN

void swap_endian_long( ULONG idata *lptr );

#endif // end of BIG_ENDIAN

//-----------------------------------------------------------------------------
// SDCC suport
//-----------------------------------------------------------------------------
#if defined SDCC
#pragma save
#pragma nooverlay
#endif // SDCC

//-----------------------------------------------------------------------------
// class specific request handler
//-----------------------------------------------------------------------------

void Class_Request( void )
{
}

//-----------------------------------------------------------------------------
// Support Subroutines
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// Vendor request handler
//-----------------------------------------------------------------------------
//  Flow control
//    Vendor OUT requrest
//    0x41    bmRequestType (OUT, Vendor, Interface)
//    0x01    bRequest      (Flow control request)
//    xxxx    wValue        (Flow control code)
//    0x0000  wIndex        (Interface #)
//    0x0000  wLength
//
//    Vendor IN requrest
//    0xC1    bmRequestType (IN, Vendor, Interface)
//    0x01    bRequest      (Flow control request)
//    0x0000  wValue
//    0x0000  wIndex        (Interface #)
//    0x0002  wLength
//
//    Flow control code
//    0x01  RTS-CTS
//    0x02  DTR-DSR
//-----------------------------------------------------------------------------

BYTE idata VRQ_Response[2];

void Vendor_Request( void )
{
	UART_PutString("VR");
	UART_PutHex(Setup.bRequest);
	UART_PutString(", Val");
	UART_PutHex(Setup.wValue.c[MSB]);
	UART_PutHex(Setup.wValue.c[LSB]);
	UART_PutString(", Idx");
	UART_PutHex(Setup.wIndex.c[MSB]);
	UART_PutHex(Setup.wIndex.c[LSB]);
	UART_PutString(", Len");
	UART_PutHex(Setup.wLength.c[MSB]);
	UART_PutHex(Setup.wLength.c[LSB]);

	if((Setup.bmRequestType & DRD_MASK) == DRD_IN)
	{
		UART_PutString(" (IN)\r\n");

		switch(Setup.bRequest)
		{
			case 0x90:  // READ_EEPROM
			{
				DataPtr = PROM + ((Setup.wIndex.c[LSB] << 1) & 0x7F);
				DataSize = 2;
				break;
			};

			default:
			{
				VRQ_Response[0] = 0x36;
				VRQ_Response[1] = 0x83;
				DataPtr = VRQ_Response;
				DataSize = 2;
				break;
			};
		};

		if(DataSize > 0)
		{
			Ep_Status0 = EP_TX;
		};
	}
	else
	{
		UART_PutString(" (OUT)\r\n");
	};

	setup_handled = TRUE;
}

//-----------------------------------------------------------------------------
// SDCC suport
//-----------------------------------------------------------------------------
#if defined SDCC
#pragma restore
#endif // SDCC

//-----------------------------------------------------------------------------
// Handle_In
//-----------------------------------------------------------------------------
//	Handle IN EP
//	- Endpoint HALT - STALL
//	- RX data stream
//
//	This routine is called from mainloop
//-----------------------------------------------------------------------------

int remtosend = 4;
BYTE idata buf[4] = { 0x01, 0x02, 0x03, 0x04 };

void Handle_In( void )
{
	BYTE ControlReg;

	if (Ep_StatusIN == EP_IDLE)						// If endpoint is currently halted, 
	{
		EIE1 &= ~0x02;									// disable USB interrupt

		POLL_WRITE_BYTE(INDEX, 1);						// Set index to endpoint 1 registers
		POLL_READ_BYTE(EINCSR1, ControlReg);			// Read contol register for EP

		if( !(ControlReg & rbInINPRDY) && remtosend > 0)
		{
			FIFO_Write_idata( FIFO_EP1, 2, buf);
			FIFO_Write_idata( FIFO_EP1, 4, buf);
			remtosend = 0;
			POLL_WRITE_BYTE(EINCSR1, rbInINPRDY);
		};

		EIE1 |= 0x02;									// enable USB interrupt
	}
}

//-----------------------------------------------------------------------------
// Handle_Out
//-----------------------------------------------------------------------------
//
// Return Value : None
// Parameters	: None
//
// Take the received packet from the host off the fifo and put it into
// the Out_Packet array
//
//-----------------------------------------------------------------------------

#if defined(C8051F326_H)
 // #define OUT_EP_INDEX 1
 // #define OUT_FIFO_ADDR FIFO_EP1
 #define OUT_EP_INDEX 2
 #define OUT_FIFO_ADDR FIFO_EP1
#else
 #define OUT_EP_INDEX 2
 #define OUT_FIFO_ADDR FIFO_EP2
#endif

void Handle_Out( void )
{
	BYTE ControlReg;

	if (Ep_StatusOUT == EP_IDLE)						// If endpoint is ready
	{
		EIE1 &= ~0x02;									// disable USB interrupt

		POLL_WRITE_BYTE(INDEX, OUT_EP_INDEX);			// Set index to endpoint 1/2 registers
		POLL_READ_BYTE(EOUTCSR1, ControlReg);
														// TX buffer (host --> UART), USB side
		if ( ControlReg & rbOutOPRDY )					// FIFO has data?
		{
			BYTE n;
			UART_PutString("OUT\r\n");
			POLL_READ_BYTE(EOUTCNTL, n);				// read out number of bytes on the FIFO

			if(n != 0)
			{
				BYTE d;
				if(--n == 0)
				{
					USB0ADR = OUT_FIFO_ADDR | 0x80;		// Set address and initiate first read
				}
				else
				{
					USB0ADR = OUT_FIFO_ADDR | 0xC0;		// Set address, auto-read and initiate first read
					do
					{
						POLL_READ_BYTE(USB0DAT, d);		// Read a byte from FIFO
						UART_PutHex(d);					// and display it	
						if((n&7)==0) UART_PutString("\r\n");
					}
					while ( --n != 0 );
				}
				while(USB0ADR & 0x80);					// Wait until byte is ready
				USB0ADR = 0x00;							// Clear auto-read
				d = USB0DAT;							// Read last byte from FIFO
				UART_PutHex(d);							// and display it	
				UART_PutString("\r\n");
			}
			POLL_WRITE_BYTE(EOUTCSR1, 0);				// Clear Out Packet ready bit
		}

		EIE1 |= 0x02;									// enable USB interrupt
	}
}

//-----------------------------------------------------------------------------
// swap_endian_long
//-----------------------------------------------------------------------------
// swap endian for long varialbe
//-----------------------------------------------------------------------------

#if defined BIG_ENDIAN

void swap_endian_long( ULONG idata *lptr )
{
/*
	BYTE tmp;
	BYTE idata * ptr = (BYTE idata *)lptr;

	tmp = ptr[0];
	ptr[0] = ptr[3];
	ptr[3] = tmp;
	tmp = lptr[1];
	ptr[1] = ptr[2];
	ptr[2] = tmp;
*/
	BYTE tmp[4];
	BYTE idata * ptr = (BYTE idata *)lptr;

	tmp[3] = ptr[0];
	tmp[2] = ptr[1];
	tmp[1] = ptr[2];
	tmp[0] = ptr[3];

	ptr[0] = tmp[0];
	ptr[1] = tmp[1];
	ptr[2] = tmp[2];
	ptr[3] = tmp[3];
}

#endif // end of BIG_ENDIAN

//-----------------------------------------------------------------------------
// End Of File
//-----------------------------------------------------------------------------
