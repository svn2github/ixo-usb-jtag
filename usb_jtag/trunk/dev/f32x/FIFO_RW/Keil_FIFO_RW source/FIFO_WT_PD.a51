; FIFO_RW.a51 generated from FIFO_RW.c
;
; void FIFO_Write_pdata( BYTE fifo_adr, BYTE n, BYTE pdata * ptr );

$NOMOD51

$include (c8051f320.inc)	; Include register definition file.

?PR?_FIFO_Write_pdata?FIFO_WT_PD	SEGMENT CODE
	PUBLIC	_FIFO_Write_pdata
	RSEG	?PR?_FIFO_Write_pdata?FIFO_WT_PD
_FIFO_Write_pdata:

;---- Variable fifo_adr assigned to Register R7
;---- Variable n assigned to Register R5
;---- Variable ptr assigned to Register R3 -> R0

	MOV		A,R5				; if (n == 0)
	JZ		wtFp_ret			;    return;

	MOV		A,R3				; R0 = ptr;
	MOV		R0,A
	MOV		A,R7				; Set address (mask out bits7-6)
	ANL		A,#03FH
	MOV		USB0ADR,A
wtFp_loop1:
	MOVX	A,@R0				; USB0DAT = *ptr++
	MOV		USB0DAT,A
	INC		R0
wtFp_loop2:
	MOV		A,USB0ADR			; Wait for BUSY->'0' (data ready)
	JB		ACC.7,wtFp_loop2
	DJNZ	R5,wtFp_loop1		; loop n times

wtFp_ret:
	RET
; END OF _FIFO_Write_pdata

	END
