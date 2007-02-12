/*-----------------------------------------------------------------------------
 * Hardware-dependent code for usb_jtag
 *-----------------------------------------------------------------------------
 * Copyright (C) 2007 Kolja Waschk, ixo.de
 *-----------------------------------------------------------------------------
 * This code is part of usbjtag. usbjtag is free software; you can redistribute
 * it and/or modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of the License,
 * or (at your option) any later version. usbjtag is distributed in the hope
 * that it will be useful, but WITHOUT ANY WARRANTY; without even the implied
 * warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.  You should have received a
 * copy of the GNU General Public License along with this program in the file
 * COPYING; if not, write to the Free Software Foundation, Inc., 51 Franklin
 * St, Fifth Floor, Boston, MA  02110-1301  USA
 *-----------------------------------------------------------------------------
 */

#ifndef _HARDWARE_H
#define _HARDWARE_H 1

// #define HAVE_PS_MODE 1
// #define HAVE_AS_MODE 1
// #define HAVE_OE_LED  1

//-----------------------------------------------------------------------------

#include <fx2regs.h>

/* JTAG TCK, AS/PS DCLK */

sbit at 0xA2          TCK; /* Port C.0 */
#define bmTCKOE       bmBIT2
#define SetTCK(x)     do{TCK=(x);}while(0)

/* JTAG TDI, AS ASDI, PS DATA0 */

sbit at 0xA0          TDI; /* Port C.2 */
#define bmTDIOE       bmBIT0
#define SetTDI(x)     do{TDI=(x);}while(0)

/* JTAG TMS, AS/PS nCONFIG */

sbit at 0xA3          TMS; /* Port C.3 */
#define bmTMSOE       bmBIT3
#define SetTMS(x)     do{TMS=(x);}while(0)

/* JTAG TDO, AS/PS CONF_DONE */

sbit at 0xA1          TDO; /* Port C.1 */
#define bmTDOOE       bmBIT1
#define GetTDO(x)     TDO

//-----------------------------------------------------------------------------

#if defined(HAVE_PS_MODE) || defined(HAVE_AS_MODE)

  /* AS DATAOUT, PS nSTATUS */

  sbit at 0xA6        ASDO; /* Port C.6 */
  #define bmASDOOE    bmBIT6
  #define GetASDO(x)  ASDO

#else

  #define bmASDOOE    0
  #define GetASDO(x)  0

#endif

//-----------------------------------------------------------------------------

#if defined(HAVE_AS_MODE)

  /* AS Mode nCS */

  sbit at 0xA4        NCS; /* Port C.4 */
  #define bmNCSOE     bmBIT4
  #define SetNCS(x)   do{NCS=(x);}while(0)
  #define GetNCS(x)   NCS

  /* AS Mode nCE */

  sbit at 0xA5        NCE; /* Port C.5 */
  #define bmNCEOE     bmBIT5
  #define SetNCE(x)   do{NCE=(x);}while(0)

  extern unsigned char ShiftInOut_AS(unsigned char x);

#else

  #define bmNCSOE     0
  #define SetNCS(x)   while(0){}
  #define GetNCS(x)   1
  #define bmNCEOE     0
  #define SetNCE(x)   while(0){}

  #define ShiftInOut_AS(x) x

#endif

//-----------------------------------------------------------------------------

#ifdef HAVE_OE_LED

  sbit at 0xA7        OELED; /* Port C.7 */
  #define bmOELEDOE   bmBIT7
  #define SetOELED(x) do{OELED=(x);}while(0)

#else

  #define bmOELEDOE   0
  #define SetOELED(x) while(0){}

#endif

//-----------------------------------------------------------------------------

#define bmPROGOUTOE (bmTCKOE|bmTDIOE|bmTMSOE|bmNCEOE|bmNCSOE|bmOELEDOE)
#define bmPROGINOE  (bmTDOOE|bmASDOOE)

#define ProgIO_Init()    HW_Init()
#define ProgIO_Enable()  do{OEC=(OEC&~bmPROGINOE)|bmPROGOUTOE;}while(0)
#define ProgIO_Disable() do{OEC=OEC&~(bmPROGINOE|bmPROGOUTOE);}while(0)
#define ProgIO_Deinit()  while(0){}

extern void HW_Init(void);
extern void ShiftOut(unsigned char x);
extern unsigned char ShiftInOut(unsigned char x);

#endif /* _HARDWARE_H */

