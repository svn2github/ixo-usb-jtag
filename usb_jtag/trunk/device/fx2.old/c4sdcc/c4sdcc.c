#include "c4sdcc.h"

#if 0

?ASlink-Warning-Undefined Global '_EZUSB_Discon_PARM_1' referenced by module 'fw'
?ASlink-Warning-Undefined Global '_EZUSB_Discon' referenced by module 'fw'
?ASlink-Warning-Undefined Global '_EZUSB_Susp' referenced by module 'fw'
?ASlink-Warning-Undefined Global '_EZUSB_Delay' referenced by module 'ftdemu'
?ASlink-Warning-Undefined Global '_EZUSB_Resume' referenced by module 'fw'

#endif

STRINGDSCR xdata * EZUSB_GetStringDscr(BYTE StrIdx)
{
  STRINGDSCR xdata *      dscr;

  dscr = (STRINGDSCR xdata *) pStringDscr;

  while(dscr->type == STRING_DSCR)
  {
    if(!StrIdx--) return(dscr);
    dscr = (STRINGDSCR xdata *)((WORD)dscr + dscr->length);
  }

  return 0;
}

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

void EZUSB_Resume(void)
{
  if(USBCS & bmRWAKEUP)
  {
    USBCS |= bmSIGRESUME;
    EZUSB_Delay(20);
    USBCS &= ~bmSIGRESUME;
  }
}

void EZUSB_Discon(BOOL renum)
{

   USBCS |=bmDISCON;       // added for FX first silicon keeper problem
   WRITEDELAY();           // allow for register write recovery time
   USBCS &=~bmDISCOE;      // Tristate the disconnect pin
   WRITEDELAY();           // allow for register write recovery time
   USBCS |=bmDISCON;       // Set this high to disconnect USB
   if(renum)               // If renumerate
     USBCS |=bmRENUM;      // then set renumerate bit
    
   EZUSB_Delay(1500);      // Wait 1500 ms  (SHK -- If you don't wait this long, 
                           // some hubs and hosts will not see the disconnect.)

   if (CPUCS & CPUCS_48MHZ)
      EZUSB_Delay(1500);   // Wait 1500 ms  (SHK -- If you don't wait this long,
                           // some hubs and hosts will not see the disconnect.)

   USBIRQ = 0xff;          // Clear any pending USB interrupt requests.  They're for our old life.
   EPINIRQ = 0xff;         // clear old activity
   EPOUTIRQ = 0xff;
   EZUSB_IRQ_CLEAR();

   USBCS &=~bmDISCON;      // reconnect USB
   SYNCDELAY();            // allow for register write recovery time
   USBCS |=bmDISCOE;       // Release Tristated disconnect pin
}

