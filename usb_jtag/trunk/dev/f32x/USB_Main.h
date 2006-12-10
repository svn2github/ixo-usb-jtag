//-----------------------------------------------------------------------------
// USB_Main.h
//-----------------------------------------------------------------------------
// Copyright 2005 Silicon Laboratories, Inc.
// http://www.silabs.com
//
// Program Description:
//
// Main header file for USB firmware. Includes function prototypes,
// standard constants, and configuration constants.
//
//
// How To Test:		See Readme.txt
//
//
// FID:				32X000025
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
//	  -Changed file name from USB_MAIN.h
//
// Release 1.1
//	  -All changes by DM
//	  -22 NOV 2002
//	  -Updated function prototypes and added constants to USB_MAIN.h
//		with sample interrupt firmware.
//
// Release 1.0
//	  -Initial Revision (JS)
//	  -05 APR 2002
//
#ifndef USB_MAIN_H
#define USB_MAIN_H

//-----------------------------------------------------------------------------
// Global Constants
//-----------------------------------------------------------------------------

// #define SYSCLK					12000000	// SYSCLK frequency in Hz

#if defined C8051F320_H

	// USB clock selections (SFR CLKSEL)
	#define USB_4X_CLOCK			0x00		// Select 4x clock multiplier,
												// for USB Full Speed
	#define USB_INT_OSC_DIV_2		0x10		// See Oscillators in Datasheet
	#define USB_EXT_OSC				0x20
	#define USB_EXT_OSC_DIV_2		0x30
	#define USB_EXT_OSC_DIV_3		0x40
	#define USB_EXT_OSC_DIV_4		0x50

	// System clock selections (SFR CLKSEL)
	#define SYS_INT_OSC				0x00		// Select to use internal osc.
	#define SYS_EXT_OSC				0x01		// Select to use external osc.
	#define SYS_4X_DIV_2			0x02

#endif // C8051F320_H

#if defined C8051F326_H

	// USB clock selections (SFR CLKSEL)
	#define USB_4X_CLOCK			0x00		// Select 4x clock multiplier,
												// for USB Full Speed
	#define USB_INT_OSC_DIV_2		0x10		// See Oscillators in Datasheet
	#define USB_EXT_OSC				0x20
	#define USB_CLOCK_OFF			0x30

	// System clock selections (SFR CLKSEL)
	#define SYS_INT_OSC				0x00		// Select to use internal osc.
	#define SYS_EXT_OSC				0x01		// Select to use external osc.
	#define SYS_4X_DIV_2			0x02
	#define SYS_LOW_FREQ_OSC		0x03		// Low frequency OSC

#endif // C8051F326_H

#if defined C8051F340_H

	// USB clock selections (SFR CLKSEL)
	#define USB_4X_CLOCK			0x00		// Select 4x clock multiplier,
												// for USB Full Speed
	#define USB_INT_OSC_DIV_2		0x10		// See Oscillators in Datasheet
	#define USB_EXT_OSC				0x20
	#define USB_EXT_OSC_DIV_2		0x30
	#define USB_EXT_OSC_DIV_3		0x40
	#define USB_EXT_OSC_DIV_4		0x50

	// System clock selections (SFR CLKSEL)
	#define SYS_INT_OSC				0x00		// Select to use internal osc.
	#define SYS_EXT_OSC				0x01		// Select to use external osc.
	#define SYS_4X_DIV_2			0x02
	#define SYS_4X					0x03

#endif // C8051F340_H

#endif		/* USB_MAIN_H */

//-----------------------------------------------------------------------------
// End Of File
//-----------------------------------------------------------------------------
