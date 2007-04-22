#include "c4sdcc.h"
#include <stdio.h>

void EZUSB_Resume(void)
{
    if(EZUSB_EXTWAKEUP())
    {
        printf("Signalling remote wakeup.\n");
        USBCS |= bmSIGRESUME;
        EZUSB_Delay(20);
        USBCS &= ~bmSIGRESUME;
    }
}
