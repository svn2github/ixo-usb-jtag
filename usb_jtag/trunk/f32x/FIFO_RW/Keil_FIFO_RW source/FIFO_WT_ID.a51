; FIFO_RW.a51 generated from FIFO_RW.c
;
; void FIFO_Write_idata( BYTE fifo_adr, BYTE n, BYTE idata * ptr );

$NOMOD51

$include (c8051f320.inc)	; Include register definition file.

?PR?_FIFO_Write_idata?FIFO_WT_ID	SEGMENT CODE
	PUBLIC	_FIFO_Write_idata
	RSEG	?PR?_FIFO_Write_idata?FIFO_WT_ID
_FIFO_Write_idata:

;---- Variable fifo_adr assigned to Register 'R7'
;---- Variable n assigned to Register 'R5'
;---- Variable 'ptr' assigned to Register R3 -> R0

	MOV		A,R5				; if (n == 0)
	JZ		wtFi_ret			;    return;

	MOV		A,R3				; R0 = ptr;
	MOV		R0,A
	MOV		A,R7				; Set address (mask out bits7-6)
	ANL		A,#03FH
	MOV		USB0ADR,A
wtFi_loop1:
	MOV		USB0DAT,@R0			; USB0DAT = *ptr++
	INC		R0
wtFi_loop2:
	MOV		A,USB0ADR			; Wait for BUSY->'0' (data ready)
	JB		ACC.7,wtFi_loop2
	DJNZ	R5,wtFi_loop1		; loop n times

wtFi_ret:
	RET
; END OF _FIFO_Write_idata

	END
