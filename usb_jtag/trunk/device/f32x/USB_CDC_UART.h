//-----------------------------------------------------------------------------
// USB_CDC_UART.h
//-----------------------------------------------------------------------------

#ifndef USB_CDC_UART_H
#define USB_CDC_UART_H

//-----------------------------------------------------------------------------
// Constant
//-----------------------------------------------------------------------------

// Line coding structure for Get/Set_Line_Coding
//	0-3 dwDTERate    Data terminal rate (baudrate), in bits per second
//	4   bCharFormat  Stop bits: 0 - 1 Stop bit, 1 - 1.5 Stop bits, 2 - 2 Stop bits
//	5   bParityType  Parity:    0 - None, 1 - Odd, 2 - Even, 3 - Mark, 4 - Space
//	6   bDataBits    Data bits: 5, 6, 7, 8, 16
typedef struct {
	ULONG baudrate;
	BYTE stopbit;
	BYTE parity;
	BYTE databit;
} TLine_Coding;

// bitmap for Set_Line_State()
#define CDC_DTR			0x01
#define CDC_RTS			0x02

// bitmap for Update_Line_State
#define CDC_DCD			0x01
#define CDC_DSR			0x02
#define CDC_BREAK		0x04	// break reception
#define CDC_RI			0x08
#define CDC_FRAME		0x10	// frame error
#define CDC_PARITY		0x20	// parity error
#define CDC_OVRRUN		0x40	// overrun error
#define CDC_CTS			0x80

// size of TX and RX ring buffer (must be greater than EP2_PACKET_SIZE)
#define TXBUFSIZE		0x100
#define RXBUFSIZE		0x100

// Memmory space allocation for TX and RX buffer
//#define TXBUF_IDATA
//#define TXBUF_PDATA
#define TXBUF_XDATA

//#define RXBUF_IDATA
//#define RXBUF_PDATA
#define RXBUF_XDATA

#if defined TXBUF_IDATA
	typedef BYTE idata Ttxbuffer;
	#define FIFO_READ_FUNC	FIFO_Read_idata
#elif defined TXBUF_PDATA
	typedef BYTE pdata Ttxbuffer;
	#define FIFO_READ_FUNC	FIFO_Read_pdata
#elif defined TXBUF_XDATA
	typedef BYTE xdata Ttxbuffer;
	#define FIFO_READ_FUNC	FIFO_Read_xdata
#endif // TXBUF_IDATA

#if defined RXBUF_IDATA
	typedef BYTE idata Trxbuffer;
	#define FIFO_WRITE_FUNC	FIFO_Write_idata
#elif defined RXBUF_PDATA
	typedef BYTE pdata Trxbuffer;
	#define FIFO_WRITE_FUNC	FIFO_Write_pdata
#elif defined RXBUF_XDATA
	typedef BYTE xdata Trxbuffer;
	#define FIFO_WRITE_FUNC	FIFO_Write_xdata
#endif // RXBUF_IDATA

//-----------------------------------------------------------------------------
// Externs
//-----------------------------------------------------------------------------

extern volatile bit uart_DTR;				// Line status output
extern volatile bit uart_RTS;
extern volatile TLine_Coding idata uart_line_coding;

// TX and RX buffer variables
extern UINT TXcount;						// number of data to transmit from UART
extern UINT RXcount;						// number of data to be recieved by host
extern bit TXReady;							// TX buffer has data to transmit
extern bit RXReady;							// RX buffer has room

//-----------------------------------------------------------------------------
// Prototypes
//-----------------------------------------------------------------------------

bit Set_Line_Coding( TLine_Coding idata * setting );
void Set_Line_State( BYTE st );
void Update_Line_State( BYTE st );
void Send_Break( UINT dur );

void Flush_COMbuffers( void );
BYTE COMGetByte( void );
void COMPutByte( BYTE dt );

#endif	// USB_CDC_UART_H

//-----------------------------------------------------------------------------
// End Of File
//-----------------------------------------------------------------------------
