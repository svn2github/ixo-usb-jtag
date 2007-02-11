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

#include "hw_xpcu_i.h"

void ShiftOut(unsigned char c)
{
  SetTDI(c&1); SetTCK(1); c>>=1; SetTCK(0);
  SetTDI(c&1); SetTCK(1); c>>=1; SetTCK(0);
  SetTDI(c&1); SetTCK(1); c>>=1; SetTCK(0);
  SetTDI(c&1); SetTCK(1); c>>=1; SetTCK(0);
                                 
  SetTDI(c&1); SetTCK(1); c>>=1; SetTCK(0);
  SetTDI(c&1); SetTCK(1); c>>=1; SetTCK(0);
  SetTDI(c&1); SetTCK(1); c>>=1; SetTCK(0);
  SetTDI(c&1); SetTCK(1); c>>=1; SetTCK(0);
}

void ShiftInOut(unsigned char c)
{
  bit carry;

  carry=GetTDO()?0x80:0; SetTDI(c&1); SetTCK(1); c=carry|(c>>1); SetTCK(0);
  carry=GetTDO()?0x80:0; SetTDI(c&1); SetTCK(1); c=carry|(c>>1); SetTCK(0);
  carry=GetTDO()?0x80:0; SetTDI(c&1); SetTCK(1); c=carry|(c>>1); SetTCK(0);
  carry=GetTDO()?0x80:0; SetTDI(c&1); SetTCK(1); c=carry|(c>>1); SetTCK(0);

  carry=GetTDO()?0x80:0; SetTDI(c&1); SetTCK(1); c=carry|(c>>1); SetTCK(0);
  carry=GetTDO()?0x80:0; SetTDI(c&1); SetTCK(1); c=carry|(c>>1); SetTCK(0);
  carry=GetTDO()?0x80:0; SetTDI(c&1); SetTCK(1); c=carry|(c>>1); SetTCK(0);
  carry=GetTDO()?0x80:0; SetTDI(c&1); SetTCK(1); c=carry|(c>>1); SetTCK(0);
}


