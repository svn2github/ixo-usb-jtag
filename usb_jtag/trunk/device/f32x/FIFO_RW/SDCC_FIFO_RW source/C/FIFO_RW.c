// FIFO_RW.c
//   FIFO read / write
//   Template C code to generate an asm source
//

#include <c8051F320.h>

typedef unsigned char BYTE;

typedef BYTE idata buf_t;
//typedef BYTE pdata buf_t;
//typedef BYTE xdata buf_t;
//typedef BYTE code buf_t;


void FIFO_Read( BYTE fifo_adr, BYTE n, buf_t * ptr )
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
//			*ptr++ = USB0DAT;			// Copy data byte
			*ptr = USB0DAT;				// Separating dereference and increment results better Asm code
			ptr++;                           
		} while ( --n != 0 );

		while(USB0ADR & 0x80);			// Wait for data ready
		USB0ADR = 0;					// Clear auto-read
		*ptr = USB0DAT;					// Read last data
	}
}

void FIFO_Write( BYTE fifo_adr, BYTE n, buf_t * ptr  )
{
	if ( n != 0 ) {
		USB0ADR = fifo_adr & 0x3F;		// Set address (mask out bits7-6)

		do {
			USB0DAT = *ptr++;			// Push to FIFO
			while(USB0ADR & 0x80);		// Wait for BUSY->'0' (data ready)
		} while ( --n != 0 );
	}
}
