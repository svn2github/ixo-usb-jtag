// FIFO_RD_XD.c
//
// void FIFO_Read_xdata( BYTE fifo_adr, BYTE n, BYTE xdata * ptr )
//
//--------------------------------------------------------------------

#include <c8051F320.h>

typedef unsigned char BYTE;

void FIFO_Read_xdata( BYTE fifo_adr, BYTE n, BYTE xdata * ptr )
{
	if ( n != 0 ) {
		if ( --n == 0 ) {
			USB0ADR = fifo_adr | 0x80;	// Set address and initiate first read
			while( USB0ADR & 0x80 );	// Wait for data ready
			*ptr = USB0DAT;				// read out FIFO data
			return;
		}

		USB0ADR = fifo_adr | 0xC0;		// Set address, auto-read and initiate first read
		do {
			while(USB0ADR & 0x80);		// Wait for BUSY->'0' (data ready)
			*ptr++ = USB0DAT;			// Copy data byte
		} while ( --n != 0 );

		while(USB0ADR & 0x80);			// Wait for data ready
		USB0ADR = 0;					// Clear auto-read
		*ptr = USB0DAT;					// Read last data
	}
}
