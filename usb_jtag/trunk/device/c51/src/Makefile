
CC=sdcc
CFLAGS+=-mmcs51 --no-xinit-opt -I../include

AS=asx8051
ASFLAGS+=-plosgff

LDFLAGS=--code-loc 0x0000 --code-size 0x1800
LDFLAGS+=--xram-loc 0x1800 --xram-size 0x0800
LDFLAGS+=-Wl '-b USBDESCSEG = 0xE000'
LDFLAGS+=-L ../lib

%.rel : %.a51
	$(AS) $(ASFLAGS) $<

%.rel : %.c
	$(CC) -c $(CFLAGS) $(CPPFLAGS) $< -o $@

std.ihx: vectors.rel usrp_main.rel usrp_common.rel dscr.rel eeprom.rel _startup.rel
	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ $+ libfx2.lib

boot: std.ihx
	-test -e /dev/usb_jtag    && /sbin/fxload -D /dev/usb_jtag    -I std.ihx -t fx2
	-test -e /dev/tracii_xl2  && /sbin/fxload -D /dev/tracii_xl2  -I std.ihx -t fx2
	-test -e /dev/xilinx_xpcu && /sbin/fxload -D /dev/xilinx_xpcu -I std.ihx -t fx2

clean:
	rm -f *.lst *.asm *.lib *.sym *.rel *.mem *.map *.rst *.lnk *.ihx


