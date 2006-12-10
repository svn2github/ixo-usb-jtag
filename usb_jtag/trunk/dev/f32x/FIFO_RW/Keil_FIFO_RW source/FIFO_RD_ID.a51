; FIFO_Read.a51 generated from FIFO_RW.c
;
; void FIFO_Read_idata( BYTE fifo_adr, BYTE n, BYTE idata * ptr );

$NOMOD51

$include (c8051f320.inc)	; Include register definition file.

?PR?_FIFO_Read_idata?FIFO_RD_ID		SEGMENT CODE
	PUBLIC	_FIFO_Read_idata
	RSEG	?PR?_FIFO_Read_idata?FIFO_RD_ID
_FIFO_Read_idata:

;---- Variable 'fifo_adr' assigned to Register 'R7'
;---- Variable 'n' assigned to Register 'R5'
;---- Variable 'ptr' assigned to Register R3 -> R0

	MOV		A,R5				; if (n == 0)
	JZ		rdFi_ret			;    return;

	MOV		A,R3				; R0 = ptr;
	MOV		R0,A
	DJNZ	R5,rdFi_multi		; if ( --n == 0 )
	MOV		A,R7				; USB0ADR = fifo_adr | 0x80;
	ORL  	A,#080H
	MOV		USB0ADR,A			; Set FIFO address and initiate first read
rdFi_one:
	MOV		A,USB0ADR			; Wait for data ready
	JB		ACC.7,rdFi_one
	MOV		@R0,USB0DAT			; *ptr = USB0DAT;
rdFi_ret:
	RET

rdFi_multi:						; else
	MOV		A,R7				; USB0ADR = fifo_adr | 0xC0;
	ORL		A,#0C0H				; Set auto-read and initiate first read
	MOV		USB0ADR,A			; set FIFO address
rdFi_loop:
	MOV		A,USB0ADR			; Wait for BUSY->'0' (data ready)
	JB		ACC.7,rdFi_loop
	MOV		@R0,USB0DAT			; *ptr++ = USB0DAT;
	INC		R0
	DJNZ	R5,rdFi_loop		; loop n-1 times
rdFi_last:
	MOV		A,USB0ADR			; Wait for data ready
	JB		ACC.7,rdFi_last
	CLR		A					; clear auto-read flag
	MOV		USB0ADR,A
	MOV		@R0,USB0DAT			; *ptr = USB0DAT;
	RET
; END OF _FIFO_Read_idata

END
