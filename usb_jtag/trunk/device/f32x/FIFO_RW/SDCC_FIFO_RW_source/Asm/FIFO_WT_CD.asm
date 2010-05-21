;--------------------------------------------------------
; File Created by SDCC : FreeWare ANSI-C Compiler
; Version 2.5.5 #1236 (Apr  6 2006)
; This file generated Tue Jul 18 04:07:46 2006
;--------------------------------------------------------
	.module FIFO_WT_CD
	.optsdcc -mmcs51 --model-small
	
;--------------------------------------------------------
; Public variables in this module
;--------------------------------------------------------
	.globl _FIFO_Write_code_PARM_3
	.globl _FIFO_Write_code_PARM_2
	.globl _FIFO_Write_code

	.globl _USB0DAT
	.globl _USB0ADR
;--------------------------------------------------------
; special function registers
;--------------------------------------------------------
	.area RSEG    (DATA)
G$USB0ADR$0$0 == 0x0096
_USB0ADR	=	0x0096
G$USB0DAT$0$0 == 0x0097
_USB0DAT	=	0x0097
;--------------------------------------------------------
; overlayable register banks
;--------------------------------------------------------
	.area REG_BANK_0	(REL,OVR,DATA)
	.ds 8
;--------------------------------------------------------
; internal ram data
;--------------------------------------------------------
	.area DSEG    (DATA)
;--------------------------------------------------------
; overlayable items in internal ram 
;--------------------------------------------------------
	.area	OSEG    (OVR,DATA)
LFIFO_Write_code$n$1$1==.
_FIFO_Write_code_PARM_2::
	.ds 1
LFIFO_Write_code$ptr$1$1==.
_FIFO_Write_code_PARM_3::
	.ds 2
;--------------------------------------------------------
; indirectly addressable internal ram data
;--------------------------------------------------------
	.area ISEG    (DATA)
;--------------------------------------------------------
; bit data
;--------------------------------------------------------
	.area BSEG    (BIT)
;--------------------------------------------------------
; paged external ram data
;--------------------------------------------------------
	.area PSEG    (PAG,XDATA)
;--------------------------------------------------------
; external ram data
;--------------------------------------------------------
	.area XSEG    (XDATA)
;--------------------------------------------------------
; external initialized ram data
;--------------------------------------------------------
	.area XISEG   (XDATA)
	.area HOME    (CODE)
	.area GSINIT0 (CODE)
	.area GSINIT1 (CODE)
	.area GSINIT2 (CODE)
	.area GSINIT3 (CODE)
	.area GSINIT4 (CODE)
	.area GSINIT5 (CODE)
	.area GSINIT  (CODE)
	.area GSFINAL (CODE)
	.area CSEG    (CODE)
;--------------------------------------------------------
; global & static initialisations
;--------------------------------------------------------
	.area HOME    (CODE)
	.area GSINIT  (CODE)
	.area GSFINAL (CODE)
	.area GSINIT  (CODE)
;--------------------------------------------------------
; Home
;--------------------------------------------------------
	.area HOME    (CODE)
	.area CSEG    (CODE)
;--------------------------------------------------------
; code
;--------------------------------------------------------
	.area CSEG    (CODE)
;------------------------------------------------------------
;Allocation info for local variables in function 'FIFO_Write_code'
;------------------------------------------------------------
;n                         Allocated with name '_FIFO_Write_code_PARM_2'
;ptr                       Allocated with name '_FIFO_Write_code_PARM_3'
;fifo_adr                  Allocated to registers r2 
;------------------------------------------------------------
	G$FIFO_Write_code$0$0 ==.
	C$FIFO_WT_CD.c$32$0$0 ==.
;C:/working/SDCC_FIFO_RW/FIFO_WT_CD.c:32: void FIFO_Write_code( BYTE fifo_adr, BYTE n, BYTE code * ptr  )
;	-----------------------------------------
;	 function FIFO_Write_code
;	-----------------------------------------

_FIFO_Write_code:
	mov	a,_FIFO_Write_code_PARM_2
	jz	00109$
	mov	a,#0x3F
	anl	a,dpl
	mov	_USB0ADR,a
	mov	r2,_FIFO_Write_code_PARM_2
	mov	dpl,_FIFO_Write_code_PARM_3
	mov	dph,(_FIFO_Write_code_PARM_3 + 1)
00104$:
	clr	a
	movc	a,@a+dptr
	mov	_USB0DAT,a
	inc	dptr
00101$:
	mov	a,_USB0ADR
	jb	acc.7,00101$
	djnz	r2,00104$
00109$:
	ret

	.area CSEG    (CODE)
	.area CONST   (CODE)
	.area XINIT   (CODE)
