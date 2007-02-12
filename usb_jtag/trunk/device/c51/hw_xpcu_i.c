/*-----------------------------------------------------------------------------
 *
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

#include "hw_xpcu_i.h"
#include "fx2regs.h"
#include "syncdelay.h"

void HW_Init(void)
{
  /* The following code depends on your actual circuit design.
     Make required changes _before_ you try the code! */

  // set the CPU clock to 48MHz, enable clock output to FPGA
  CPUCS = bmCLKOE | bmCLKSPD1;

  // Use internal 48 MHz, enable output, use "Port" mode for all pins
  IFCONFIG = bmIFCLKSRC | bm3048MHZ | bmIFCLKOE;

  GPIFABORT = 0xFF;

  PORTACFG = 0x00; OEA = 0x03; IOA=0x01;
  PORTCCFG = 0x00; OEC = 0x00; IOC=0x00;
  PORTECFG = 0x00; OEE = 0x00; IOE=0x00;
}

void ShiftOut(unsigned char c)
{
  unsigned char lc=c;

  if(lc&1) IOE|=0x40; else IOE&=~0x40; IOE|=0x08; lc>>=1; IOE&=~0x08;
  if(lc&1) IOE|=0x40; else IOE&=~0x40; IOE|=0x08; lc>>=1; IOE&=~0x08;
  if(lc&1) IOE|=0x40; else IOE&=~0x40; IOE|=0x08; lc>>=1; IOE&=~0x08;
  if(lc&1) IOE|=0x40; else IOE&=~0x40; IOE|=0x08; lc>>=1; IOE&=~0x08;

  if(lc&1) IOE|=0x40; else IOE&=~0x40; IOE|=0x08; lc>>=1; IOE&=~0x08;
  if(lc&1) IOE|=0x40; else IOE&=~0x40; IOE|=0x08; lc>>=1; IOE&=~0x08;
  if(lc&1) IOE|=0x40; else IOE&=~0x40; IOE|=0x08; lc>>=1; IOE&=~0x08;
  if(lc&1) IOE|=0x40; else IOE&=~0x40; IOE|=0x08; lc>>=1; IOE&=~0x08;
}

unsigned char ShiftInOut(unsigned char c)
{
  unsigned char carry;
  unsigned char lc=c;

  carry = (IOE&0x20)<<2; if(lc&1) IOE|=0x40; else IOE&=~0x40; IOE|=0x08; lc=carry|(lc>>1); IOE&=~0x08;
  carry = (IOE&0x20)<<2; if(lc&1) IOE|=0x40; else IOE&=~0x40; IOE|=0x08; lc=carry|(lc>>1); IOE&=~0x08;
  carry = (IOE&0x20)<<2; if(lc&1) IOE|=0x40; else IOE&=~0x40; IOE|=0x08; lc=carry|(lc>>1); IOE&=~0x08;
  carry = (IOE&0x20)<<2; if(lc&1) IOE|=0x40; else IOE&=~0x40; IOE|=0x08; lc=carry|(lc>>1); IOE&=~0x08;

  carry = (IOE&0x20)<<2; if(lc&1) IOE|=0x40; else IOE&=~0x40; IOE|=0x08; lc=carry|(lc>>1); IOE&=~0x08;
  carry = (IOE&0x20)<<2; if(lc&1) IOE|=0x40; else IOE&=~0x40; IOE|=0x08; lc=carry|(lc>>1); IOE&=~0x08;
  carry = (IOE&0x20)<<2; if(lc&1) IOE|=0x40; else IOE&=~0x40; IOE|=0x08; lc=carry|(lc>>1); IOE&=~0x08;
  carry = (IOE&0x20)<<2; if(lc&1) IOE|=0x40; else IOE&=~0x40; IOE|=0x08; lc=carry|(lc>>1); IOE&=~0x08;

  return lc;
}


