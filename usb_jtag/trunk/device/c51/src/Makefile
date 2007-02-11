
CC=sdcc
CFLAGS+=-mmcs51 --no-xinit-opt -I../include

AS=asx8051
ASFLAGS+=-plosgff

LIBDIR=../lib
LIB=libfx2.lib

LDFLAGS=--code-loc 0x0000 --code-size 0x1800
LDFLAGS+=--xram-loc 0x1800 --xram-size 0x0800
LDFLAGS+=-Wl '-b USBDESCSEG = 0xE000'
LDFLAGS+=-L ${LIBDIR}

%.rel : %.a51
	$(AS) $(ASFLAGS) $<

%.rel : %.c
	$(CC) -c $(CFLAGS) $(CPPFLAGS) $< -o $@

default: std.ihx

std.ihx: vectors.rel usbjtag.rel dscr.rel eeprom.rel hardware.rel _startup.rel ${LIBDIR}/${LIB}
	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ $+ ${LIB}

${LIBDIR}/${LIB}:
	make -C ${LIBDIR}


.PHONY: boot
boot: std.ihx
	-test -e /dev/usb_jtag    && /sbin/fxload -D /dev/usb_jtag    -I std.ihx -t fx2
	-test -e /dev/tracii_xl2  && /sbin/fxload -D /dev/tracii_xl2  -I std.ihx -t fx2
	-test -e /dev/xilinx_xpcu && /sbin/fxload -D /dev/xilinx_xpcu -I std.ihx -t fx2

REF=/srv/altera/blaster.hex
.PHONY: ref
ref: 
	-test -e /dev/usb_jtag    && /sbin/fxload -D /dev/usb_jtag    -I ${REF} -t fx2
	-test -e /dev/tracii_xl2  && /sbin/fxload -D /dev/tracii_xl2  -I ${REF} -t fx2
	-test -e /dev/xilinx_xpcu && /sbin/fxload -D /dev/xilinx_xpcu -I ${REF} -t fx2


dscr.rel: dscr.a51 product.inc
eeprom.rel: eeprom.a51 product.inc
usbjtag.rel: usbjtag.c hardware.h
hardware.rel: hardware.c hardware.h

clean:
	rm -f *.lst *.asm *.lib *.sym *.rel *.mem *.map *.rst *.lnk *.ihx


