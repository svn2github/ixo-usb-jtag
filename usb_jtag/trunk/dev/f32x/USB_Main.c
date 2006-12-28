//-----------------------------------------------------------------------------
// USB_Main.c
//-----------------------------------------------------------------------------
// Copyright 2005 Silicon Laboratories, Inc.
// http://www.silabs.com
//
// Program Description:
//
// This application note covers the implementation of a simple USB application 
// using the interrupt transfer type. This includes support for device
// enumeration, control and interrupt transactions, and definitions of 
// descriptor data. The purpose of this software is to give a simple working 
// example of an interrupt transfer application; it does not include
// support for multiple configurations or other transfer types.
//
// How To Test:		See Readme.txt
//
//
// FID:				32X000024
// Target:			C8051F32x
// Tool chain:		Keil C51 7.50 / Keil EVAL C51
//					Silicon Laboratories IDE version 2.6
// Command Line:	 See Readme.txt
// Project Name:	 F32x_USB_Interrupt
//
//
// Release 1.3
//		-All changes by GP
//		-22 NOV 2005
//		-Changed revision number to match project revision
//		No content changes to this file
//		-Modified file to fit new formatting guidelines
//		-Changed file name from USB_MAIN.c

// Release 1.1
//		-All changes by DM
//		-22 NOV 2002
//		-Added support for switches and sample USB interrupt application.
//
// Release 1.0
//		-Initial Revision (JS)
//		-22 FEB 2002
//

//-----------------------------------------------------------------------------
// Includes
//-----------------------------------------------------------------------------

#include "USB_CDC_Type.h"
#include "USB_Register.h"
#include "USB_Main.h"
#include "USB_Descriptor.h"
#include "USB_Standard_Requests.h"
#include "USB_CDC_UART.h"

//-----------------------------------------------------------------------------
// 16-bit SFR Definitions for 'F32x
//-----------------------------------------------------------------------------

#if defined KEIL

#ifndef C8051F326_H
sfr16 TMR2RL	= 0xca;					// Timer2 reload value
sfr16 TMR2		= 0xcc;					// Timer2 counter
#endif

#endif // KEIL

//-----------------------------------------------------------------------------
// Globals
//-----------------------------------------------------------------------------

#define USE_ADC 0

#if defined KEIL

sbit Led1 = P2^2;						// LED='1' means ON
sbit Led2 = P2^3;

#endif // KEIL

#if defined SDCC

#define Led1   P2_2						// LED='1' means ON
#define Led2   P2_3

#endif // SDCC

#define Sw1 0x01						// These are the port2 bits for Sw1
#define Sw2 0x02						// and Sw2 on the development board

BYTE Switch1State = 0;					// Indicate status of switch
BYTE Switch2State = 0;					// starting at 0 == off

BYTE Toggle1 = 0;						// Variable to make sure each button
BYTE Toggle2 = 0;						// press and release toggles switch

volatile bit switch_changed = FALSE;

#if USE_ADC
BYTE Potentiometer = 0x00;				// Last read potentiometer value
BYTE Temperature   = 0x00;				// Last read temperature sensor value

idata BYTE Out_Packet[64];				// Last packet received from host
idata BYTE In_Packet[64];				// Next packet to sent to host

code const BYTE TEMP_ADD = 112;			// This constant is added to Temperature
#endif

//-----------------------------------------------------------------------------
// Local Prototypes
//-----------------------------------------------------------------------------

// Initialization Routines
void Sysclk_Init(void);					// Initialize the system clock
void Port_Init(void);					// Configure ports
void UART_Init(void);					// Configure UART(s)
void Usb0_Init(void);					// Configure USB for Full speed
void Timer_Init(void);					// Timer 2 for use by ADC and swtiches
#if USE_ADC
void Adc_Init(void);					// Configure ADC for continuous
										// conversion low-power mode
#endif

// Other Routines
void Delay(void);						// About 80 us/1 ms on Full Speed
void UART_PutNibble(BYTE x);
void UART_PutString(char *s);
void UART_PutChar(char c);
void UART_PutHex(BYTE x);

//-----------------------------------------------------------------------------
// ISR prototypes for SDCC
//-----------------------------------------------------------------------------

#if defined SDCC

#ifndef C8051F326_H
extern void Timer2_ISR(void) interrupt 5;					// Checks if switches are pressed
#endif
#if USE_ADC
extern void Adc_ConvComplete_ISR(void) interrupt 10;		// Upon Conversion, switch ADC MUX
#endif
extern void Usb_ISR(void) interrupt 8;						// Determines type of USB interrupt

#endif // SDCC

//-----------------------------------------------------------------------------
// startup routine
//-----------------------------------------------------------------------------

#if defined SDCC

unsigned char _sdcc_external_startup ( void )
{
#ifndef C8051F326_H
   PCA0MD &= ~0x40;                    // Disable Watchdog timer temporarily
#endif
   return 0;
}

#endif // SDCC

//-----------------------------------------------------------------------------
// UART_PutChar(char c)
//-----------------------------------------------------------------------------

void UART_PutChar(char c)
{
	TI0 = 0;
	SBUF0 = c;
	while(!TI0);
}

void UART_PutNibble(BYTE x)
{
	UART_PutChar(x+(x<10)?'0':('A'-'0'));
}

void UART_PutHex(BYTE x)
{
	UART_PutNibble(x>>4);
	UART_PutNibble(x&15);
}

void UART_PutString(char *s)
{
	char *sp = s;
	while(*sp) UART_PutChar(*(sp++));
}

//-----------------------------------------------------------------------------
// Main Routine
//-----------------------------------------------------------------------------
void main(void)
{
//	BYTE Count;
	BYTE dt;
	BYTE line_state;

#if defined KEIL

	PCA0MD &= ~0x40;					// Disable Watchdog timer temporarily

#endif // KEIL

	Sysclk_Init();						// Initialize oscillator
	Port_Init();						// Initialize crossbar and GPIO
	UART_Init();						// Initialize serial port(s)
	Usb0_Init();						// Initialize USB0
	Timer_Init();						// Initialize timer2
#if USE_ADC
	Adc_Init();							// Initialize ADC
#endif
	Flush_COMbuffers();					// Initialize COM ring buffer
	Led1 = FALSE;
	Led2 = FALSE;

#if USE_ADC
	// Set initial values of In_Packet and Out_Packet to zero
	// Initialized here so that WDT doesn't kick in first
	for (Count = 0; Count < 64; Count++)
	{
		Out_Packet[Count] = 0;
		In_Packet[Count] = 0;
	}

	EIE1	|= 0x0A;					// Enable conversion complete interrupt and US0
#else
	EIE1	|= 0x02;					// Enable USB0 interrupt
#endif
	IE		|= 0xA0;					// Enable Timer2 and Global Interrupt enable

	UART_PutString("\r\nRESET\r\n");

	while (1)
	{
		Handle_In2();					// Poll IN/OUT EP2 and handle transaction
		Handle_Out2();

		if ( switch_changed )			// Reflect the switch status to DSR and CTS
		{
			switch_changed = FALSE;

			line_state = 0;
			if ( Switch1State ) line_state |= CDC_DSR;
			if ( Switch2State ) line_state |= CDC_CTS;
			Update_Line_State( line_state );
		}

		while (TXReady && RXReady)		// loop back TX to RX
		{
			dt = COMGetByte();
			COMPutByte( dt );
		}

		Led1 = uart_DTR;				// output line status
		Led2 = uart_RTS;
	}
}

//-----------------------------------------------------------------------------
// Initialization Subroutines
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// Sysclk_Init
//-----------------------------------------------------------------------------
//
// Return Value : None
// Parameters	: None
//
// Initialize the system clock and USB clock
//
//-----------------------------------------------------------------------------
void Sysclk_Init(void)
{

	OSCICN |= 0x83;						// Configure internal oscillator for
										// its maximum frequency and enable
										// missing clock detector
	CLKMUL	= 0x00;						// Select internal oscillator as
										// input to clock multiplier
	CLKMUL |= 0x80;						// Enable clock multiplier
	Delay();							// Delay for clock multiplier to begin
	CLKMUL |= 0xC0;						// Initialize the clock multiplier
	Delay();							// Delay for clock multiplier to begin
	while(!(CLKMUL & 0x20));			// Wait for multiplier to lock

#if (defined C8051F320_H) || (defined C8051F326_H)
	CLKSEL	= SYS_4X_DIV_2 | USB_4X_CLOCK;	// Select system clock and USB clock
#elif (defined C8051F340_H)
	CLKSEL	= SYS_4X | USB_4X_CLOCK;	// Select system clock and USB clock
#endif // C8051F340_H

}

//-----------------------------------------------------------------------------
// PORT_Init
//-----------------------------------------------------------------------------
//
// Return Value : None
// Parameters	: None
//
// This function configures the crossbar and GPIO ports.
//
// F320DB, F326DB
//  P1.7		analog					Potentiometer
//  P2.2		digital	push-pull		LED
//  P2.3		digital	push-pull		LED
//
// F340DB
//  P2.2		digital push-pull		LED
//  P2.3		digital push-pull		LED
//  P2.5		analog					Potentiometer
//-----------------------------------------------------------------------------
void Port_Init(void)
{

#if defined C8051F320_H
	P1MDIN	 = 0x7F;					// Port 1 pin 7 set as analog input
	P0MDOUT |= 0x0F;					// Port 0 pins 0-3 set push-pull
	P1MDOUT |= 0x0F;					// Port 1 pins 0-3 set push-pull
	P2MDOUT |= 0x0C;					// P2.2 and P2.3 set to push-pull
	P1SKIP	 = 0x80;					// Port 1 pin 7 skipped by crossbar
	XBR0	 = 0x00;
	XBR1	 = 0x40;					// Enable Crossbar
#endif // C8051F320_H

#if defined C8051F326_H
	GPIOCN = 0xC0;						// disable pull-ups, enable input path
	P0MDOUT |= 0x10;					// enable P0.4 (TxD) as a push-pull output
	P0 |= 0x10;							// TxD is AND-wired with P0.4
	//P1MDOUT							// F326 doesn't have P1
	P2MDOUT = 0;						// no push-pull output enabled on P2
	//XBR0								// F326 doesn't have a crossbar
	//XBR1								// F326 doesn't have a crossbar
#endif // C8051F326_H

#if defined C8051F340_H
	P2MDIN   = ~(0x20);					// Port 2 pin 5 set as analog input
	P0MDOUT |= 0x0F;					// Port 0 pins 0-3 set high impedence
	P1MDOUT |= 0x0F;					// Port 1 pins 0-3 set high impedence
	P2MDOUT |= 0x0C;					// Port 2 pins 0,1 set high empedence
	P2SKIP   = 0x20;					// Port 2 pin 5 skipped by crossbar
	XBR0     = 0x00;
	XBR1     = 0x40;					// Enable Crossbar
#endif // C8051F340_H

}

//-----------------------------------------------------------------------------
// UART_Init
//-----------------------------------------------------------------------------
//
// Return Value : None
// Parameters	: None
// 
// UART: 115200 Baud 8N1; see also Port_Init for I/O configuration
//-----------------------------------------------------------------------------

void UART_Init(void)
{
	SMOD0	= 0x0C;						// 8 Bits, no parity, 1 stop bit
	SBRLH0	= 0xFF;	SBRLL0 = 0x98;		// 115200 Baud (from SYSCLK 48/2 MHz)
	SBCON0	= 0x00;						// Disable BRGEN
	SBCON0	= 0x43;						// Enable BRGEN (0x40), Prescaler = 1 (0x3)
	SCON0	= 0x10;						// Enable receiver, clear interrupts
}

//-----------------------------------------------------------------------------
// Usb0_Init
//-----------------------------------------------------------------------------
//
// Return Value : None
// Parameters	: None
// 
// - Initialize USB0
// - Enable USB0 interrupts
// - Enable USB0 transceiver
// - Enable USB0 with suspend detection
//-----------------------------------------------------------------------------
void Usb0_Init(void)
{
	POLL_WRITE_BYTE(POWER,	0x08);		// Force Asynchronous USB Reset
										// USB Interrupt enable flags are reset by USB bus reset
										// 
	POLL_WRITE_BYTE(IN1IE,	0x01);		// Enable EP 0 interrupt, disable EP1-3 IN interrupt
	POLL_WRITE_BYTE(OUT1IE,	0x00);		// Disable EP 1-3 OUT interrupts
	POLL_WRITE_BYTE(CMIE,	0x05);		// Enable Reset and Suspend interrupts

	USB0XCN = 0xE0;						// Enable transceiver; select full speed
	POLL_WRITE_BYTE(CLKREC, 0x80);		// Enable clock recovery, single-step mode
										// disabled
										// Enable USB0 by clearing the USB 
										// Inhibit bit
	POLL_WRITE_BYTE(POWER,	0x01);		// and enable suspend detection
}

//-----------------------------------------------------------------------------
// Timer_Init
//-----------------------------------------------------------------------------
//
// Return Value : None
// Parameters	: None
// 
// USB_JTAG: Make Timer reload at approx. 100 Hz for 0x3061 keepalive packets
// Timer 2 reload, used to check if switch pressed on overflow and
// used for ADC continuous conversion
//-----------------------------------------------------------------------------

void Timer_Init(void)
{
#ifndef C8051F326_H
	TMR2CN	 = 0x00;					// Stop Timer2; Clear TF2;

	CKCON	&= ~0xF0;					// Timer2 clocked based on T2XCLK;
	TMR2RL	 = 0;						// Initialize reload value
	TMR2	 = 0xffff;					// Set to reload immediately

	TR2		 = 1;						// Start Timer2
#endif
}

#if USE_ADC
//-----------------------------------------------------------------------------
// Adc_Init
//-----------------------------------------------------------------------------
//
// Return Value : None
// Parameters	: None
// 
// Configures ADC for single ended continuous conversion or Timer2
//-----------------------------------------------------------------------------

void Adc_Init(void)
{
	REF0CN	= 0x0E;						// Enable voltage reference VREF
	AMX0P	= 0x1E;						// Positive input starts as temp sensor
	AMX0N	= 0x1F;						// Single ended mode(neginput = gnd)

	ADC0CF	= 0xF8;						// SAR Period 0x1F, Right adjusted

	ADC0CN	= 0xC2;						// Continuous converion on timer 2 
										// overflow; low power tracking mode on
}
#endif

#ifndef C8051F326_H
//-----------------------------------------------------------------------------
// Timer2_ISR
//-----------------------------------------------------------------------------
//
// Called when timer 2 overflows, check to see if switch is pressed,
// then watch for release.
//
//-----------------------------------------------------------------------------

void Timer2_ISR(void) interrupt 5
{
	TF2H = 0;							// Clear Timer2 interrupt flag

	if (!(P2 & Sw1))					// Check for switch #1 pressed
	{
		if (Toggle1 == 0)				// Toggle is used to debounce switch
		{								// so that one press and release will
			Switch1State = ~Switch1State; // toggle the state of the switch sent
			Toggle1 = 1;				// to the host
			switch_changed = TRUE;
		}
	}
	else Toggle1 = 0;					// Reset toggle variable

	if (!(P2 & Sw2))					// Same as above, but Switch2
	{
		if (Toggle2 == 0)
		{
			Switch2State = ~Switch2State;
			Toggle2 = 1;
			switch_changed = TRUE;
		}
	}
	else Toggle2 = 0;
}
#endif

#if USE_ADC
//-----------------------------------------------------------------------------
// Adc_ConvComplete_ISR
//-----------------------------------------------------------------------------
//
// Called after a conversion of the ADC has finished
// Updates the appropriate variable for potentiometer or temperature sensor
// Switches the ADC multiplexor value to switch between the potentiometer 
// and temp sensor
//
//-----------------------------------------------------------------------------

void Adc_ConvComplete_ISR(void) interrupt 10
{
	AD0INT = 0;

	if (AMX0P == 0x1E)					// This switches the AMUX between
	{									// the temperature sensor and the
		Temperature	= ADC0L;			// potentiometer pin after conversion
		Temperature	+= TEMP_ADD;		// Add offset to Temperature
		AMX0P			= 0x07;			// switch to potentiometer
		ADC0CF		= 0xFC;				// Place ADC0 in left-adjusted mode
	}
	else
	{
		Potentiometer = ADC0H;
		AMX0P			= 0x1E;			// switch to temperature sensor
		ADC0CF		= 0xF8;				// place ADC0 in right-adjusted mode
	}
}
#endif

//-----------------------------------------------------------------------------
// Delay
//-----------------------------------------------------------------------------
//
// Used for a small pause, approximately 80 us in Full Speed,
// and 1 ms when clock is configured for Low Speed
//
//-----------------------------------------------------------------------------

void Delay(void)
{
	int x;
	for(x = 0;x < 500;x)
		x++;
}

//-----------------------------------------------------------------------------
// End Of File
//-----------------------------------------------------------------------------
