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

//-----------------------------------------------------------------------------

#include <fx2regs.h>

#define SetTCK(x)     do{if(x) IOE|=0x08; else IOE&=~0x08; }while(0)
#define SetTMS(x)     do{if(x) IOE|=0x10; else IOE&=~0x10; }while(0)
#define SetTDI(x)     do{if(x) IOE|=0x40; else IOE&=~0x40; }while(0)
#define GetTDO()      ((IOE>>5)&1)

/* XPCU has neither AS nor PS mode pins */

#define GetASDO(x)    0
#define SetNCS(x)     while(0){}
#define GetNCS(x)     1
#define SetNCE(x)     while(0){}

#define HAVE_OE_LED 1
/* +0=green led, +1=red led */
sbit at 0x80+1        OELED;
#define SetOELED(x)   do{OELED=(x);}while(0)

//-----------------------------------------------------------------------------

#define ProgIO_Init()    HW_Init()
#define ProgIO_Enable()  do{OEA=0x03; IOA=0x01; OEE=0x58; }while(0)
#define ProgIO_Disable() do{OEA=0x03; IOA=0x01; OEE=0x00; }while(0)
#define ProgIO_Deinit()  while(0){}

extern void HW_Init(void);
extern void ShiftOut(unsigned char x);
extern unsigned char ShiftInOut(unsigned char x);

#endif /* _HARDWARE_H */

