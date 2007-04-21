
void EZUSB_Susp(void) _naked
{
  _asm
    USBCS = 0x7FD6 ; XDATA
    PCON  = 0x87   ; DATA
    mov   dptr,#USBCS  ; Clear the Wake Source bit in
    movx  a,@dptr      ; the USBCS register
    orl  a,#0x80
    movx  @dptr,a
    orl  PCON,#1       ; Place the processor in idle
    nop                ; Insert some meaningless instruction
    nop                ; fetches to insure that the processor
    nop                ; suspends and resumes before RET
    nop
    nop
  _endasm;
}

