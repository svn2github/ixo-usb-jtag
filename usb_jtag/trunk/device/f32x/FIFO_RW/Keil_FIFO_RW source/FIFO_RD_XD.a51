; FIFO_Read.a51 generated from FIFO_RW.c
;
; void FIFO_Read_xdata( BYTE fifo_adr, BYTE n, BYTE xdata * ptr );

$NOMOD51

$include (c8051f320.inc)	; Include register definition file.

?PR?_FIFO_Read_xdata?FIFO_RD_XD		SEGMENT CODE
	PUBLIC	_FIFO_Read_xdata
	RSEG	?PR?_FIFO_Read_xdata?FIFO_RD_XD
_FIFO_Read_xdata:

;---- Variable 'fifo_adr' assigned to Register 'R7'
;---- Variable 'n' assigned to Register 'R5'
;---- Variable 'ptr' assigned to Register R2:R3 -> 'DPTR'

	MOV		A,R5				; if (n == 0)
	JZ		rdFx_ret			;    return;

	MOV		DPL,R3				; DPTR = ptr;
	MOV		DPH,R2
	DJNZ	R5,rdFx_multi		; if ( --n == 0 )
	MOV		A,R7				; USB0ADR = fifo_adr | 0x80;
	ORL		A,#080H
	MOV		USB0ADR,A			; Set FIFO address and initiate first read
rdFx_one:
	MOV		A,USB0ADR			; Wait for data ready
	JB		ACC.7,rdFx_one
	MOV		A,USB0DAT			; ptr++ = USB0DAT;                           
	MOVX	@DPTR,A
rdFx_ret:
	RET

rdFx_multi:						; else
	MOV		A,R7				; USB0ADR = fifo_adr | 0xC0;
	ORL		A,#0C0H				; Set auto-read and initiate first read
	MOV		USB0ADR,A			; set FIFO address
rdFx_loop:
	MOV		A,USB0ADR			; Wait for BUSY->'0' (data ready)
	JB		ACC.7,rdFx_loop
	MOV		A,USB0DAT			; *ptr++ = USB0DAT;
	MOVX	@DPTR,A
	INC		DPTR
	DJNZ	R5,rdFx_loop		; loop n-1 times
rdFx_last:
	MOV		A,USB0ADR			; Wait for data ready
	JB		ACC.7,rdFx_last
	CLR		A					; clear auto-read flag
	MOV		USB0ADR,A
	MOV		A,USB0DAT			; *ptr = USB0DAT;
	MOVX	@DPTR,A
	RET
; END OF _FIFO_Read_xdata

END
