#include "c4sdcc.h"

void EZUSB_Renum(void) _naked
{
  _asm
    DPS = 0x86
    mov   dptr,#USBCS
    mov   a,#0b00001010        ; set DISCON and RENUM
    movx  @dptr,a              ; do it
;
; Hold disconnect low for 12 milliseconds (12000 microseconds).
; 10 cycles * 166.6 ns per cycle is 1.66 microseconds per loop.
; 12000 microseconds / 1.66 = 7229.  [assumes 24 MHz clock]

; fx2bug - need to add support 12MHz, 24MHz, and 48MHz

    mov  a,#0               ; Clear dps so that we´re using dph and dpl!  
    mov  dps,a              ; 
    mov  dptr,#(0xFFFF - 6024)    ; long pulse for operating
    mov  r4,#5

time12msec:     
        inc     dptr                 ; 3 cycles
    mov     a,dpl                ; 2 cycles
        orl     a,dph                ; 2 cycles
        jnz     time12msec           ; 3 cycles
     djnz  r4,time12msec
;
    mov      dptr,#USBCS
    mov      a,#0b00000010       ; discon LO, renum HI
    movx  @dptr,a
  _endasm;
}
