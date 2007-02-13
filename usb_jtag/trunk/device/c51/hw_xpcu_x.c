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

#include "delay.h"
#include "syncdelay.h"
#include "hw_xpcu_x.h"


#include "isr.h"
#include "timer.h"
#include "delay.h"
#include "fx2regs.h"
#include "fx2utils.h"
#include "usb_common.h"
#include "usb_descriptors.h"
#include "usb_requests.h"
#include "syncdelay.h"
#include "eeprom.h"

static unsigned char curios;

const unsigned char wavedata[64] =
{
  /* Single Write:
     s0: BITS=D0     NEXT/SGLCRC DATA WAIT 4
     s1: BITS=       DATA WAIT 4
     s2: BITS=D1|D0  DATA WAIT 4
     s3: BITS=D1     DATA WAIT 3
     s4: BITS=D1     DATA DP IF(RDY0) THEN 5 ELSE 2
     s5: BITS=D1|D0  DATA WAIT 4
     s6: BITS=D1     DATA WAIT 3
     s7: BITS=D1     DATA FIN */

  4, 4, 4, 3, 0x2A, 4, 3, 7,
  6, 2, 2, 2, 3,    2, 2, 2,
  1, 0, 3, 2, 2,    3, 2, 2,
  0, 0, 0, 0, 0,    0, 0, 0x3F,

  /* Single Read:
     s0: BITS=D0     WAIT 4
     s1: BITS=       WAIT 4
     s2: BITS=D1|D0  WAIT 4
     s3: BITS=D1     WAIT 4
     s4: BITS=D1|D0  WAIT 3
     s5: BITS=D1|D0  DP IF(RDY0) THEN 6 ELSE 3
     s6: BITS=D1     DATA WAIT 4
     s7: BITS=D1     FIN */

  4, 4, 4, 4, 3, 0x33, 4, 7,
  0, 0, 0, 0, 0, 1,    2, 0,
  1, 0, 3, 2, 3, 3,    2, 2,
  0, 0, 0, 0, 0, 0,    0, 0x3F
};

void HW_Init(void)
{
  unsigned char i;

  /* The following code depends on your actual circuit design.
     Make required changes _before_ you try the code! */

  // set the CPU clock to 48MHz, enable clock output to FPGA
  CPUCS = bmCLKOE | bmCLKSPD1;

  // Use internal 48 MHz, enable output, GPIF Master mode
  IFCONFIG = bmIFCLKSRC | bm3048MHZ | bmIFCLKOE | bmIFGPIF;

  PORTACFG = 0x00; OEA = 0xFB; IOA = 0x20;
  PORTCCFG = 0x00; OEC = 0xFF; IOC = 0x10;
  PORTECFG = 0x00; OEE = 0xFC; IOE = 0xC0;
 
  GPIFABORT    = 0xFF;

  GPIFREADYCFG = 0xA0;
  GPIFCTLCFG   = 0x00;
  GPIFIDLECS   = 0x00;
  GPIFIDLECTL  = 0x00;
  GPIFWFSELECT = 0x01;

  // Copy waveform data
  AUTOPTRSETUP = 0x07;
  APTR1H = MSB( &wavedata );
  APTR1L = LSB( &wavedata );
  AUTOPTRH2 = 0xE4;
  AUTOPTRL2 = 0x00;
  for ( i = 0; i < 64; i++ ) EXTAUTODAT2 = EXTAUTODAT1;

  SYNCDELAY;
  GPIFADRH      = 0x00;
  SYNCDELAY;
  GPIFADRL      = 0x00;

  FLOWSTATE     = 0x00;
  FLOWLOGIC     = 0x00;
  FLOWEQ0CTL    = 0x00;
  FLOWEQ1CTL    = 0x00;
  FLOWHOLDOFF   = 0x00;
  FLOWSTB       = 0x00;
  FLOWSTBEDGE   = 0x00;
  FLOWSTBHPERIOD = 0x00;

  curios = 0;
}

unsigned char GetTDO()
{
  unsigned char x;

  IOC = 0x41;

  while(!(GPIFTRIG & 0x80));
  x = XGPIFSGLDATLX;

  while(!(GPIFTRIG & 0x80));
  x = XGPIFSGLDATLNOX;

  if(IOA & 0x20) IOA |= 0x40; else IOA &= ~0x40;

  /* Activity on 2 and 0x10, but that's not the real thing it seems */

  x = (x&1)?1:0;

  /* LED visualization */
  if(x) IOA=(IOA&~3)|1; else IOA=(IOA&~3)|2;

  return x;
}


void SetPins(unsigned char x)
{
  IOC = 0x81;

  while(!(GPIFTRIG & 0x80));
  XGPIFSGLDATLX = x;

  curios = x;
}

/* 
 * 0x01/0x10: TDI
 * 0x02/0x20: TMS
 * 0x04/0x40: TCK
 * 0x08/0x80: no activity
 */

void SetTDI(unsigned char x)
{
  if(x) SetPins(curios | 0x10);
   else SetPins(curios & ~0x10);
}

void SetTMS(unsigned char x)
{
  if(x) SetPins(curios | 0x20);
   else SetPins(curios & ~0x20);
}

void SetTCK(unsigned char x)
{
  if(x) SetPins(curios | 0x40);
   else SetPins(curios & ~0x40);
}

static unsigned char n = 0;

void ShiftOut(unsigned char c)
{
  unsigned char lc=c;

  SetTDI(lc&1); SetTCK(1); lc>>=1; SetTCK(0);
  SetTDI(lc&1); SetTCK(1); lc>>=1; SetTCK(0);
  SetTDI(lc&1); SetTCK(1); lc>>=1; SetTCK(0);
  SetTDI(lc&1); SetTCK(1); lc>>=1; SetTCK(0);

  SetTDI(lc&1); SetTCK(1); lc>>=1; SetTCK(0);
  SetTDI(lc&1); SetTCK(1); lc>>=1; SetTCK(0);
  SetTDI(lc&1); SetTCK(1); lc>>=1; SetTCK(0);
  SetTDI(lc&1); SetTCK(1); lc>>=1; SetTCK(0);
}

unsigned char ShiftInOut(unsigned char c)
{
  unsigned char carry;
  unsigned char lc=c;

  carry=GetTDO()?0x80:0; SetTDI(lc&1); SetTCK(1); lc=carry|(lc>>1); SetTCK(0);
  carry=GetTDO()?0x80:0; SetTDI(lc&1); SetTCK(1); lc=carry|(lc>>1); SetTCK(0);
  carry=GetTDO()?0x80:0; SetTDI(lc&1); SetTCK(1); lc=carry|(lc>>1); SetTCK(0);
  carry=GetTDO()?0x80:0; SetTDI(lc&1); SetTCK(1); lc=carry|(lc>>1); SetTCK(0);
  carry=GetTDO()?0x80:0; SetTDI(lc&1); SetTCK(1); lc=carry|(lc>>1); SetTCK(0);
  carry=GetTDO()?0x80:0; SetTDI(lc&1); SetTCK(1); lc=carry|(lc>>1); SetTCK(0);
  carry=GetTDO()?0x80:0; SetTDI(lc&1); SetTCK(1); lc=carry|(lc>>1); SetTCK(0);
  carry=GetTDO()?0x80:0; SetTDI(lc&1); SetTCK(1); lc=carry|(lc>>1); SetTCK(0);
 
  return lc;
}


