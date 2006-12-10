; FIFO_Read.a51 generated from FIFO_RW.c
;
; void FIFO_Read_pdata( BYTE fifo_adr, BYTE n, BYTE pdata * ptr );

$NOMOD51

$include (c8051f320.inc)	; Include register definition file.

?PR?_FIFO_Read_pdata?FIFO_RD_PD		SEGMENT CODE
	PUBLIC	_FIFO_Read_pdata
	RSEG	?PR?_FIFO_Read_pdata?FIFO_RD_PD
_FIFO_Read_pdata:

;---- Variable 'fifo_adr' assigned to Register 'R7'
;---- Variable 'n' assigned to Register 'R5'
;---- Variable 'ptr' assigned to Register R3 -> R0

	MOV		A,R5				; if (n == 0)
	JZ		rdFp_ret			;	return;

	MOV		A,R3				; R0 = ptr;
	MOV		R0,A
	DJNZ	R5,rdFp_multi		; if ( --n == 0 )
	MOV		A,R7				; USB0ADR = fifo_adr | 0x80;
	ORL		A,#080H
	MOV		USB0ADR,A			; Set FIFO address and initiate first read
rdFp_one:
	MOV		A,USB0ADR			; Wait for data ready
	JB		ACC.7,rdFp_one
	MOV		A,USB0DAT			; *ptr = USB0DAT;                           
	MOVX	@R0,A
rdFp_ret:
	RET

rdFp_multi:						; else
	MOV		A,R7				; USB0ADR = fifo_adr | 0xC0;
	ORL		A,#0C0H				; Set auto-read and initiate first read
	MOV		USB0ADR,A			; set FIFO address
rdFp_loop:
	MOV		A,USB0ADR			; Wait for BUSY->'0' (data ready)
	JB		ACC.7,rdFp_loop
	MOV		A,USB0DAT			; *ptr++ = USB0DAT;
	MOVX	@R0,A
	INC		R0
	DJNZ	R5,rdFp_loop		; loop n-1 times
rdFp_last:
	MOV		A,USB0ADR			; Wait for data ready
	JB		ACC.7,rdFp_last
	CLR		A					; clear auto-read flag
	MOV		USB0ADR,A
	MOV		A,USB0DAT			; *ptr = USB0DAT;
	MOVX	@R0,A
	RET
; END OF _FIFO_Read_pdata

END
