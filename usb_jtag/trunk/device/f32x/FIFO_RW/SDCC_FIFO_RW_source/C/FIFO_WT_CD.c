// FIFO_WT_CD.c
//
// void FIFO_Write_code( BYTE fifo_adr, BYTE n, BYTE code * ptr  )
//
//--------------------------------------------------------------------

#include <c8051F320.h>

typedef unsigned char BYTE;

void FIFO_Write_code( BYTE fifo_adr, BYTE n, BYTE code * ptr  )
{
	if ( n != 0 ) {
		USB0ADR = fifo_adr & 0x3F;		// Set address (mask out bits7-6)

		do {
			USB0DAT = *ptr++;			// Push to FIFO
			while(USB0ADR & 0x80);		// Wait for BUSY->'0' (data ready)
		} while ( --n != 0 );
	}
}
