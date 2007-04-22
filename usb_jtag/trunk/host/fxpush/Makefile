LIBUSB=/cygdrive/c/Programme/LibUSB-Win32

boot: fxpush.exe
	./fxpush.exe ../../../ftdemu/fx2/ftdemu.hex 


fxpush.exe: fxpush.c
	gcc -o fxpush.exe fxpush.c -I${LIBUSB}/include -L${LIBUSB}/lib/gcc -lusb
