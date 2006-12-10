; FIFO_RW.a51 generated from FIFO_RW.c
;
; void FIFO_Write_xdata( BYTE fifo_adr, BYTE n, BYTE xdata * ptr );

$NOMOD51

$include (c8051f320.inc)	; Include register definition file.

?PR?_FIFO_Write_xdata?FIFO_WT_XD	SEGMENT CODE
	PUBLIC	_FIFO_Write_xdata
	RSEG	?PR?_FIFO_Write_xdata?FIFO_WT_XD
_FIFO_Write_xdata:

;---- Variable fifo_adr assigned to Register R7
;---- Variable n assigned to Register R5
;---- Variable ptr assigned to Register R2:R3 -> DPTR

	MOV		A,R5				; if (n == 0)
	JZ		wtFx_ret			;    return;

	MOV		DPL,R3				; DPTR = ptr;
	MOV		DPH,R2
	MOV		A,R7				; Set address (mask out bits7-6)
	ANL		A,#03FH
	MOV		USB0ADR,A
wtFx_loop1:
	MOVX	A,@DPTR				; USB0DAT = *ptr++
	MOV		USB0DAT,A
	INC		DPTR
wtFx_loop2:
	MOV		A,USB0ADR			; Wait for BUSY->'0' (data ready)
	JB		ACC.7,wtFx_loop2
	DJNZ	R5,wtFx_loop1		; loop n times

wtFx_ret:
	RET
; END OF _FIFO_Write_xdata

	END
