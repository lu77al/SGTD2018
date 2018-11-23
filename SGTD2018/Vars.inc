.def	SSREG	= r2

.def	AFlags	= r3
	.equ	afWait	= 0

.def	BFlags	= r4
	.equ	bfRTCRead    = 0
	.equ	bfRTCWrite   = 1


#define	ZeroReg	YH

.equ	XSEG = $100
; --- Массивы ($100..$1FF)
.equ	RTCBuf	  = $70
.equ	RXBuf     = $80
.equ	TXBuf     = $A0

 ; EditVal, Val

.equ	YBUF = $60
; --- Переменные ($60..$9F)
.equ	yMainCntL	= $00
.equ	yMainCntM	= $01
.equ	yMainCntH	= $02
.equ	yRXPointer	= $03
.equ	yTXPointer	= $04
.equ	yTXEnd		= $05
.equ	yRXTimer	= $06
.equ	yRXTXCHS	= $07
.equ	yMyAddr		= $08
.equ	yNextADC	= $09
.equ	yADCSaveAddr	= $0A
.equ	yADCChCnt	= $0B

.equ	ySecond		= $10
.equ	yMinute		= $11
.equ	yHour		= $12
.equ	yRTCstage	= $13
.equ	yRTCWaitStatus	= $14

.equ	yDB0		= $20
.equ	yDB1		= $21
.equ	yDB2		= $22
.equ	yDB3		= $23
.equ	yDB4		= $24
.equ	yDB5		= $25
.equ	yDB6		= $26
.equ	yDB7		= $27

.equ	yStatus0	= $28















