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
extern bit Ep_Status1;					// Contains status bytes for EP 0-2
extern bit Ep_StatusOUT2;
extern bit Ep_StatusIN2;

extern Ttxbuffer TXBuffer[ TXBUFSIZE ];	// Ring buffer for TX and RX
extern Trxbuffer RXBuffer[ RXBUFSIZE ];
extern Ttxbuffer * TXWTPtr;
extern Trxbuffer * RXRDPtr;

//-----------------------------------------------------------------------------
// Global Variables
//-----------------------------------------------------------------------------

BYTE cs_Line_State = 0;					// serial line state
bit cs_Line_State_Update = FALSE;		// update line state

#define CS_TEMPBUF_SIZE		sizeof(TLine_Coding)	// size of temporary buffer
										// must be greater than sizeof(TLine_Coding) (= 7 bytes)
BYTE idata cs_temp_buffer[ CS_TEMPBUF_SIZE ];	// temporary buffer for
												//  CS_Get_Encapsulated_Command
												//  CS_Send_Encapsulated_Command
												//  CS_Set_Line_Coding and CS_Get_Line_Coding

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
	if ( Setup.wIndex.i == 0 )		// interface index must be 0 - Comm Class IF
	{
		switch(Setup.bRequest)		// dispatch according to the request
		{
			case SEND_ENCAPSULATED_COMMAND:		CS_Send_Encapsulated_Command();	break;
			case GET_ENCAPSULATED_RESPONSE:		CS_Get_Encapsulated_Command();	break;
			case SET_LINE_CODING:				CS_Set_Line_Coding();			break;
			case GET_LINE_CODING:				CS_Get_Line_Coding();			break;
			case SET_CONTROL_LINE_STATE: 		CS_Set_ControlLine_State();		break;
			case SEND_BREAK: 					CS_Send_Break();				break;
			default:															break;
		}
	}
}

//-----------------------------------------------------------------------------
// Support Subroutines
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// Send_Encapsulated_Command
//-----------------------------------------------------------------------------
//
//	Nothing to do other than unloading the data sent in the data stage.
//
//-----------------------------------------------------------------------------

void CS_Send_Encapsulated_Command(void)
{
	if (   (Setup.bmRequestType == OUT_CL_INTERFACE)
		&& (Setup.wValue.i == 0 )
		&& (Setup.wLength.i <= CS_TEMPBUF_SIZE) )	// less than the buffer size
	{
		Ep_Status0 = EP_RX;				// Put endpoint in recieve mode
		DataPtr = (BYTE *)cs_temp_buffer;
		DataSize = Setup.wLength.i;		// Read out the command sent
		setup_handled = TRUE;
	}
}

//-----------------------------------------------------------------------------
// Get_Encapsulated_Command
//-----------------------------------------------------------------------------
//
//	Return a zero-length packet
//
//-----------------------------------------------------------------------------

void CS_Get_Encapsulated_Command(void)
{
	if (   (Setup.bmRequestType == IN_CL_INTERFACE)
		&& (Setup.wValue.i == 0) )
	{
		Ep_Status0 = EP_TX;				// Put endpoint in transmit mode
		DataPtr = (BYTE *)cs_temp_buffer;
		DataSize = 0;					// Send ZLP
		setup_handled = TRUE;
	}
}

//-----------------------------------------------------------------------------
// Set_Line_Coding
//-----------------------------------------------------------------------------
//
//	Unload the line coding structure (7 bytes) sent in the data stage.
//	Apply this setting to the UART
//	Flush the communication buffer
//
//	Line Coding Structure (7 bytes)
//	0-3 dwDTERate    Data terminal rate (baudrate), in bits per second (LSB first)
//	4   bCharFormat  Stop bits: 0 - 1 Stop bit, 1 - 1.5 Stop bits, 2 - 2 Stop bits
//	5   bParityType  Parity:    0 - None, 1 - Odd, 2 - Even, 3 - Mark, 4 - Space
//	6   bDataBits    Data bits: 5, 6, 7, 8, 16
//
//-----------------------------------------------------------------------------

void CS_Set_Line_Coding(void)
{
	if (   (Setup.bmRequestType == OUT_CL_INTERFACE)
		&& (Setup.wValue.i == 0)
		&& (Setup.wLength.i == sizeof(TLine_Coding)) )
	{
		Ep_Status0 = EP_RX;						// Put endpoint in recieve mode
		DataPtr = (BYTE *)cs_temp_buffer;
		DataSize = sizeof(TLine_Coding);		// Read out the command sent
		setup_handled = TRUE;
	}
}

// called at the end of data stage
void CS_Set_Line_Coding_Complete(void)
{

#if defined BIG_ENDIAN
						// swap baudrate field LSB first --> MSB first
	swap_endian_long( (ULONG idata *)cs_temp_buffer );

#endif // end of BIG_ENDIAN

	Set_Line_Coding( (TLine_Coding idata *)cs_temp_buffer );
}

//-----------------------------------------------------------------------------
// Get_Line_Coding
//-----------------------------------------------------------------------------
//
//	Return the line coding structure
//
//-----------------------------------------------------------------------------

void CS_Get_Line_Coding(void)
{
	if (   (Setup.bmRequestType == IN_CL_INTERFACE)
		&& (Setup.wValue.i == 0)
		&& (Setup.wLength.i == sizeof(TLine_Coding)) )
	{

#if defined BIG_ENDIAN
		BYTE cnt;
		BYTE idata * dst;
		BYTE idata * src;

		dst = cs_temp_buffer;			// copy line coding structure
		src = (BYTE idata *)&uart_line_coding;
		for (cnt = sizeof( TLine_Coding ); cnt > 0; cnt--)
			*dst++ = *src++;

		swap_endian_long( (ULONG idata *)cs_temp_buffer );

		DataPtr = (BYTE *)cs_temp_buffer;
#else
		DataPtr = (BYTE *)(&uart_line_coding);	// send it directly
#endif // end of BIG_ENDIAN

		Ep_Status0 = EP_TX;					// Put endpoint in transmit mode
		DataSize = sizeof(TLine_Coding);	// Send Line coding
		setup_handled = TRUE;
	}
}

//-----------------------------------------------------------------------------
// Set_ControlLine_State
//-----------------------------------------------------------------------------
//
//	Set/reset RTS/DTR according to wValue
//	wValue
//	 bit 1  RTS
//	 bit 0  DTR
//
//-----------------------------------------------------------------------------

void CS_Set_ControlLine_State(void)
{
	if (   (Setup.bmRequestType == OUT_CL_INTERFACE)
		&& (Setup.wLength.i == 0) )
	{
		Set_Line_State( Setup.wValue.c[LSB] & (CDC_RTS | CDC_DTR ) );
		setup_handled = TRUE;
	}
}

//-----------------------------------------------------------------------------
// Send_Break
//-----------------------------------------------------------------------------
//
//	Send break from UART TX port, for wValue (msec) duration.
//	wValue
//	 0xFFFF: continuous break
//	 0x0000: stop break
//
//-----------------------------------------------------------------------------

void CS_Send_Break(void)
{
	if (   (Setup.bmRequestType == OUT_CL_INTERFACE)
		&& (Setup.wLength.i == 0) )
	{
		Send_Break( Setup.wValue.i );
		setup_handled = TRUE;
	}
}

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
	if((Setup.bmRequestType & DRD_MASK) == DRD_IN)
	{
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
	};

	setup_handled = TRUE;
}

//-----------------------------------------------------------------------------
// Handle_In1
//-----------------------------------------------------------------------------
//
// Interrupt IN EP - Notification element
//	Return SerialState notification response (10 bytes) to host 
//	whenever the status are changed
//
//	response
//    0:  0xA1   bmRequestType
//    1:  0x20   bNotification (SERIAL_STATE)
//    2:  0x00   wValue
//    3:  0x00
//    4:  0x00   wIndex (Interface #, LSB first)
//    5:  0x00
//    6:  0x02   wLength (Data length = 2 bytes, LSB first)
//    7:  0x00
//    8:  xx     UART State Bitmap (16bits, LSB first)
//    9:  xx
//
//  UART State Bitmap
//    15-8: reserved
//    7:  (no spec extra) CTS
//    6:  bOverRun    overrun error
//    5:  bParity     parity error
//    4:  bFraming    framing error
//    3:  bRingSignal RI
//    2:  bBreak      break reception
//    1:  bTxCarrier  DSR
//    0:  bRxCarrier  DCD
//
//-----------------------------------------------------------------------------

#define CS_SERIAL_STATE_NUM		0x08

BYTE code serialStateResponse[ CS_SERIAL_STATE_NUM ] = 
{
	0xA1, 0x20, 0x00, 0x00, 0x00, 0x00, 0x02, 0x00
};

void Handle_In1( void )
{
	BYTE ControlReg;

	if (Ep_Status1 == EP_IDLE)
	{
		POLL_WRITE_BYTE(INDEX, 1);						// Set index to endpoint 1 registers
		POLL_READ_BYTE(EINCSR1, ControlReg); 			// Read contol register for EP

		if ( !(ControlReg & rbInINPRDY)					// FIFO is empty?
			&& cs_Line_State_Update )					// and must update?
		{
			cs_Line_State_Update = FALSE;
														// Put Serial State Response on Fifo
//			FIFO_Write_code(FIFO_EP1, CS_SERIAL_STATE_NUM, serialStateResponse);
			Fifo_Write(FIFO_EP1, CS_SERIAL_STATE_NUM, (BYTE *)serialStateResponse);
			USB0DAT = cs_Line_State;					// Put UART state bitmap (LSB)
			while(USB0ADR & 0x80);
			USB0DAT = 0x00;								//						 (MSB)
			while(USB0ADR & 0x80);
			POLL_WRITE_BYTE(EINCSR1, rbInINPRDY);
														// Set In Packet ready bit, indicating 
														// fresh data on Fifo 1
		}
	}
}

//-----------------------------------------------------------------------------
// SDCC suport
//-----------------------------------------------------------------------------
#if defined SDCC
#pragma restore
#endif // SDCC

//-----------------------------------------------------------------------------
// Handle_In2
//-----------------------------------------------------------------------------
//	Handle IN EP2
//	- Endpoint HALT - STALL
//	- RX data stream
//
//	This routine is called from mainloop
//-----------------------------------------------------------------------------

void Handle_In2( void )
{
	BYTE ControlReg;
	BYTE send_cnt, first_cnt, second_cnt;
	UINT pos;
	bit turnover;

	if (Ep_StatusIN2 == EP_IDLE)						// If endpoint is currently halted, 
	{
		EIE1 &= ~0x02;									// disable USB interrupt

		POLL_WRITE_BYTE(INDEX, 2);						// Set index to endpoint 2 registers
		POLL_READ_BYTE(EINCSR1, ControlReg);			// Read contol register for EP

														// RX buffer (UART --> host), USB side
		if ( !(ControlReg & rbInINPRDY) && RXcount )	// FIFO is empty and must send?
		{
			if ( RXcount > EP2_PACKET_SIZE )			// determine the bytes to send in this transaction
				send_cnt = EP2_PACKET_SIZE;
			else
				send_cnt = (BYTE)RXcount;

			pos = RXBUFSIZE - (RXRDPtr - RXBuffer);		// consider on wrap around of ring buffer
			if ( send_cnt < pos )
			{
				first_cnt = send_cnt;
				second_cnt = 0;
				turnover = FALSE;
			}
			else
			{
				first_cnt = (BYTE)pos;
				second_cnt = send_cnt - first_cnt;
				turnover = TRUE;
			}

			FIFO_WRITE_FUNC( FIFO_EP2, first_cnt, RXRDPtr );	// send bytes to FIFO
			if ( second_cnt != 0 )
				FIFO_WRITE_FUNC( FIFO_EP2, second_cnt, RXBuffer );
			POLL_WRITE_BYTE(EINCSR1, rbInINPRDY);

			if ( turnover )
				RXRDPtr = RXBuffer + second_cnt;
			else
				RXRDPtr += send_cnt;

			RXcount -= send_cnt;
			RXReady = (RXcount != RXBUFSIZE);
		}

		EIE1 |= 0x02;									// enable USB interrupt
	}
}

//-----------------------------------------------------------------------------
// Handle_Out2
//-----------------------------------------------------------------------------
//
// Return Value : None
// Parameters	: None
//
// Take the received packet from the host off the fifo and put it into
// the Out_Packet array
//
//-----------------------------------------------------------------------------

void Handle_Out2( void )
{
	BYTE ControlReg;
	BYTE FIFO_Cnt, first_cnt, second_cnt;
	UINT pos;
	bit turnover;

	if (Ep_StatusOUT2 == EP_IDLE)						// If endpoint is ready
	{
		EIE1 &= ~0x02;									// disable USB interrupt

		POLL_WRITE_BYTE(INDEX, 2);						// Set index to endpoint 2 registers
		POLL_READ_BYTE(EOUTCSR1, ControlReg);
														// TX buffer (host --> UART), USB side
		if ( ControlReg & rbOutOPRDY )					// FIFO has data?
		{
			POLL_READ_BYTE(EOUTCNTL, FIFO_Cnt);			// read out number of bytes on the FIFO

			if ( (TXBUFSIZE - TXcount) >= FIFO_Cnt )	// if any room on the buffer to read out the FIFO
			{
				pos = TXBUFSIZE - (TXWTPtr - TXBuffer);	// consider on wrap around of ring buffer
				if ( FIFO_Cnt < pos )
				{
					first_cnt = FIFO_Cnt;
					second_cnt = 0;
					turnover = FALSE;
				}
				else
				{
					first_cnt = (BYTE)pos;
					second_cnt = FIFO_Cnt - first_cnt;
					turnover = TRUE;
				}

				FIFO_READ_FUNC( FIFO_EP2, first_cnt, TXWTPtr );	// read bytes from FIFO
				if ( second_cnt != 0 )
					FIFO_READ_FUNC( FIFO_EP2, second_cnt, TXBuffer );
				POLL_WRITE_BYTE(EOUTCSR1, 0);			// Clear Out Packet ready bit

				if ( turnover )
					TXWTPtr = TXBuffer + second_cnt;
				else
					TXWTPtr += FIFO_Cnt;

				TXcount += FIFO_Cnt;
				TXReady = (TXcount != 0);
			}
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
