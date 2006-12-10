CC=sdcc
CFLAGS=-mmcs51 --model-small --debug --use-stdout -V

MAIN=USB_Main

default: $(MAIN).hex

.PHONY: default flash clean USB_ALT.flashed
.SUFFIXES: .rel
.c.rel:
	$(CC) -c $(CFLAGS) -o $@ $<

$(MAIN).hex: $(MAIN).rel USB_Descriptor.rel USB_ISR.rel USB_Standard_Requests.rel USB_Class_Requests.rel USB_CDC_UART.rel SDCC_FIFO_RW.lib
	$(CC) $(CFLAGS) -o$@ $+

$(MAIN).rel: $(MAIN).c
USB_Descriptor.rel: USB_Descriptor.c
USB_ISR.rel: USB_ISR.c
USB_CDC_UART.rel: USB_CDC_UART.c
USB_Class_Requests.rel: USB_Class_Requests.c
USB_Standard_Requests.rel: USB_Standard_Requests.c

.PHONY: reflashed
reflashed: $(MAIN.hex)
	ec2writeflash --port=USB --hex $<
	touch flashed

flashed: $(MAIN).hex
	ec2writeflash --port=USB --hex $<
	touch $@

clean:
	rm -f $(MAIN) $(MAIN).cdb $(MAIN).lnk *.hex *.rel *.lst *.mem *.map *.asm *.adb *.sym *.rst

