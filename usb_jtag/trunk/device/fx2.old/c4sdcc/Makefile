#-----------------------------------------------------------------------------
# Makefile for FX2 library code
#-----------------------------------------------------------------------------
# Copyright (C) 2007 Kolja Waschk, ixo.de
#-----------------------------------------------------------------------------
# This code is part of usbjtag. usbjtag is free software; you can redistribute
# it and/or modify it under the terms of the GNU General Public License as
# published by the Free Software Foundation; either version 2 of the License,
# or (at your option) any later version. usbjtag is distributed in the hope
# that it will be useful, but WITHOUT ANY WARRANTY; without even the implied
# warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.  You should have received a
# copy of the GNU General Public License along with this program in the file
# COPYING; if not, write to the Free Software Foundation, Inc., 51 Franklin
# St, Fifth Floor, Boston, MA  02110-1301  USA
#-----------------------------------------------------------------------------

CC=sdcc
CFLAGS+=-mmcs51 --no-xinit-opt -I.
CPPFLAGS+=
OBJS=stall.rel delay.rel discon.rel get_strd.rel susp.rel \
     renum.rel resume.rel get_cnfg.rel get_dscr.rel get_infc.rel i2c.rel i2c_rw.rel

AR=sdcclib

%.rel : %.c
	$(CC) $(CFLAGS) $(CPPFLAGS) -c $< -o $*.rel

(%.rel) : %.c
	$(CC) $(CFLAGS) $(CPPFLAGS) -c $< -o $*.rel
	$(AR) -a $@ $*.rel
	rm $*.rel

%.rel : %.asx
	$(AS) $(ASFLAGS) $<

(%.rel) : %.asx
	$(AS) $(ASFLAGS) $<
	$(AR) -a $@ $*.rel
	rm $*.rel

c4sdcc.lib: c4sdcc.lib($(OBJS))



stall.rel: stall.c
delay.rel: delay.c
discon.rel: discon.c
get_strd.rel: get_strd.c
susp.rel: susp.c
renum.rel: renum.c
resume.rel: resume.c
get_cnfg.rel: get_cnfg.c
get_dscr.rel: get_dscr.c
get_infc.rel: get_infc.c
i2c.rel: i2c.c
i2c_rw.rel: i2c_rw.c

startup.rel: startup.asx
usbjmptb.rel: usbjmptb.asx

clean:
	rm -f *.lst *.asm *.lib *.sym *.rel *.lib *.rst


