//-----------------------------------------------------------------------------
// USB_CDC_UART.c
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// Includes
//-----------------------------------------------------------------------------

#include "USB_CDC_Type.h"
#include "USB_CDC_UART.h"

//-----------------------------------------------------------------------------
// Constant
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// Externs
//-----------------------------------------------------------------------------

extern BYTE cs_Line_State;					// serial line state
extern bit cs_Line_State_Update;			// update line state

//-----------------------------------------------------------------------------
// Global Variables
//-----------------------------------------------------------------------------

volatile TLine_Coding idata uart_line_coding = {		// line coding structure and its default value
	115200,		// baudrate
	0,			// stop bit:	1
	0,			// parity:		none
	8			// data bits:	8
};

volatile bit uart_DTR = FALSE;				// Line status output
volatile bit uart_RTS = FALSE;

Ttxbuffer TXBuffer[ TXBUFSIZE ];			// Ring buffer for TX and RX
Trxbuffer RXBuffer[ RXBUFSIZE ];
Ttxbuffer * TXRDPtr;						// pointers for ring buffer
Ttxbuffer * TXWTPtr;
Trxbuffer * RXRDPtr;
Trxbuffer * RXWTPtr;
UINT TXcount;								// number of data to transmit from UART
UINT RXcount;								// number of data to be recieved by host
bit TXReady;								// TX buffer has data to transmit
bit RXReady;								// RX buffer has room

//-----------------------------------------------------------------------------
// UART Subroutines
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// Set_Line_Coding
//-----------------------------------------------------------------------------
//
//	Check the line coding setting (baudrate, databits, parity and stop bit)
//	If it is acceptable, copy it to line_coding structure,
//	and set it to the UART, flush the communication buffer
//	Otherwise, return FALSE
//
//	called by USB0 interrupt
//
// Line coding structure
//	0-3 dwDTERate    Data terminal rate (baudrate), in bits per second
//	4   bCharFormat  Stop bits: 0 - 1 Stop bit, 1 - 1.5 Stop bits, 2 - 2 Stop bits
//	5   bParityType  Parity:    0 - None, 1 - Odd, 2 - Even, 3 - Mark, 4 - Space
//	6   bDataBits    Data bits: 5, 6, 7, 8, 16
//-----------------------------------------------------------------------------

//---------------------------
// SDCC suport
//---------------------------
#if defined SDCC
#pragma save
#pragma nooverlay
#endif // SDCC

bit Set_Line_Coding( TLine_Coding idata * setting )
{
	BYTE cnt;
	BYTE idata * src;
	BYTE idata * dst;

	// Check the setting is acceptable or not

	// Copy setting
	src = (BYTE idata *)setting;
	dst = (BYTE idata *)&uart_line_coding;
	for ( cnt = sizeof(TLine_Coding); cnt > 0; cnt-- )
		*dst++ = *src++;

	// Apply setting to UART

	// Flush COM buffers
	Flush_COMbuffers();
	
	return TRUE;
}

//---------------------------
// SDCC suport
//---------------------------
#if defined SDCC
#pragma restore
#endif // SDCC

//-----------------------------------------------------------------------------
// Set_Line_State
//-----------------------------------------------------------------------------
//	Set/reset RTS/DTR of UART
//		paramter
//			bit 1  RTS
//			bit 0  DTR
//
//	called by USB0 interrupt
//-----------------------------------------------------------------------------

//---------------------------
// SDCC suport
//---------------------------
#if defined SDCC
#pragma save
#pragma nooverlay
#endif // SDCC

void Set_Line_State( BYTE st )
{
	uart_DTR = ((st & CDC_DTR) != 0);
	uart_RTS = ((st & CDC_RTS) != 0);
}

//---------------------------
// SDCC suport
//---------------------------
#if defined SDCC
#pragma restore
#endif // SDCC

//-----------------------------------------------------------------------------
// Update_Line_State
//-----------------------------------------------------------------------------
//  Send line state update to host
//
//  UART State Bitmap
//    7:  (no spec extra) CTS
//    6:  bOverRun    overrun error
//    5:  bParity     parity error
//    4:  bFraming    framing error
//    3:  bRingSignal RI
//    2:  bBreak      break reception
//    1:  bTxCarrier  DSR
//    0:  bRxCarrier  DCD
//-----------------------------------------------------------------------------

void Update_Line_State( BYTE st )
{
	EIE1 &= ~0x02;					// disable USB interrupt
	cs_Line_State = st;
	cs_Line_State_Update = TRUE;
	EIE1 |= ~0x02;					// enable USB interrupt
}

//-----------------------------------------------------------------------------
// Send_Break
//-----------------------------------------------------------------------------
//	Send break from UART TX port, for specified duration.
//		paramter
//		dur: duration to output break (msec)
//			0x0000: stop break
//			0xFFFF: send break continuously
//
//	called by USB0 interrupt
//-----------------------------------------------------------------------------

//---------------------------
// SDCC suport
//---------------------------
#if defined SDCC
#pragma save
#pragma nooverlay
#endif // SDCC

void Send_Break( UINT dur )
{
	// To do: Send break for 'dur' msec
}

//---------------------------
// SDCC suport
//---------------------------
#if defined SDCC
#pragma restore
#endif // SDCC

//-----------------------------------------------------------------------------
// Flush_COMbuffers, COMGetByte, COMPutByte
//-----------------------------------------------------------------------------
// Ring buffers for TX (host --> UART) and RX (UART --> host) over bulk EP
//
//	Flush_COMbuffers:	initialize COM ring buffer
//	COMGetByte:			read a byte from COM TX buffer
//	COMPutByte:			write a byte to COM RX buffer
//-----------------------------------------------------------------------------

//---------------------------
// SDCC suport
//---------------------------
#if defined SDCC
#pragma save
#pragma nooverlay
#endif // SDCC

void Flush_COMbuffers( void )
{
	TXRDPtr = TXBuffer;
	TXWTPtr = TXBuffer;
	RXRDPtr = RXBuffer;
	RXWTPtr = RXBuffer;
	TXcount = 0;
	RXcount = 0;
	TXReady = FALSE;
	RXReady = TRUE;
}

//---------------------------
// SDCC suport
//---------------------------
#if defined SDCC
#pragma restore
#endif // SDCC


// TX buffer (host --> UART), UART side
BYTE COMGetByte( void )
{
	BYTE dt;

	dt = *TXRDPtr;
	if ( TXRDPtr == (TXBuffer + (TXBUFSIZE - 1)) )	// at the end of the buffer
		TXRDPtr = TXBuffer;					// go to the head of the buffer
	else
		TXRDPtr++;

	TXcount--;
	TXReady = (TXcount != 0);

	return dt;
}

// RX buffer (UART --> host), UART side
void COMPutByte( BYTE dt )
{
	*RXWTPtr = dt;
	if ( RXWTPtr == (RXBuffer + (RXBUFSIZE - 1)) )	// at the end of the buffer
		RXWTPtr = RXBuffer;					// go to the head of the buffer
	else
		RXWTPtr++;

	RXcount++;
	RXReady = (RXcount != RXBUFSIZE);
}
	
//-----------------------------------------------------------------------------
// End Of File
//-----------------------------------------------------------------------------
