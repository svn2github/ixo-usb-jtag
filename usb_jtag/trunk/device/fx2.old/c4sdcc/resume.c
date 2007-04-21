#include "c4sdcc.h"

void EZUSB_Resume(void)
{
	if( ((WAKEUPCS & bmWUEN)&&(WAKEUPCS & bmWU)) ||   // TPM: Check status AND Enable
		((WAKEUPCS & bmWU2EN)&&(WAKEUPCS & bmWU2)) )
	{
		USBCS |= bmSIGRESUME;
		EZUSB_Delay(20);
		USBCS &= ~bmSIGRESUME;
	}
}
