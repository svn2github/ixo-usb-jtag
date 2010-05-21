; FIFO_RW.a51 generated from FIFO_RW.c
;
; void FIFO_Write_code( BYTE fifo_adr, BYTE n, BYTE code * ptr );

$NOMOD51

$include (c8051f320.inc)		; Include register definition file.

?PR?_FIFO_Write_code?FIFO_WT_CD		SEGMENT CODE 
	PUBLIC	_FIFO_Write_code
	RSEG	?PR?_FIFO_Write_code?FIFO_WT_CD
_FIFO_Write_code:

;---- Variable fifo_adr assigned to Register 'R7'
;---- Variable n assigned to Register 'R5'
;---- Variable 'ptr' assigned to Register R2:R3 -> 'DPTR'

	MOV		A,R5				; if (n == 0)
	JZ		wtFc_ret			;    return;

	MOV		DPL,R3				; DPTR = ptr;
	MOV		DPH,R2
	MOV		A,R7				; Set address (mask out bits7-6)
	ANL		A,#03FH
	MOV		USB0ADR,A
wtFc_loop1:
	CLR		A
	MOVC	A,@A+DPTR			; USB0DAT = *ptr++
	MOV		USB0DAT,A
	INC		DPTR
wtFc_loop2:
	MOV		A,USB0ADR			; Wait for BUSY->'0' (data ready)
	JB		ACC.7,wtFc_loop2
	DJNZ	R5,wtFc_loop1		; loop n times

wtFc_ret:
	RET
; END OF _FIFO_Write_code

	END
