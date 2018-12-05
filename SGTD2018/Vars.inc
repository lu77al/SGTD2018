.def	SSREG	= r2

.def	AFlags	= r3
	.equ	afActive   = 0
	.equ	afReverse  = 1
	.equ	afFastStop = 2

.def	BFlags	= r4
	.equ	bfRTCRead  = 0
	.equ	bfRTCWrite = 1

.def	KS_Dead    = r5
.def	KS_High    = r6


#define	ZeroReg	YH

.equ	XSEG = $100
; --- Arrays ($100..$1FF)
;.equ	PeriodBuf = $00
;.equ	KS_Buf  = $00
;.equ	TimeBuf = $03
;.equ	ADCBuf	= $20
;.equ	InputBuf= $30
.equ	RXBuf   = $50
.equ	TXBuf   = $70

 ; EditVal, Val

.equ	YSEG = $60
; --- Fast vars ($60..$9F)
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

.equ	yPanPosL	= $10
.equ	yPanPosM	= $11
.equ	yPanPosH	= $12
.equ	yRotorHallsPort	= $13
.equ	yRotorState	= $14
.equ	yJamPath	= $15
.equ	yCurPeriod	= $16
.equ	yLastPeriod	= $17
.equ	yDummyPeriod	= $18
.equ	yPWML		= $19
.equ	yPWMH		= $1A
.equ	yNeedPeriod	= $1B
.equ	yStopPosL	= $1C
.equ	yStopPosM	= $1D
.equ	yStopPosH	= $1E
.equ	yAccelCnt	= $1F
.equ	yTargetPosL	= $20
.equ	yTargetPosM	= $21
.equ	yTargetPosH	= $22
.equ	yAfterFailCntL	= $23
.equ	yAfterFailCntH	= $24
.equ	yAfterFailPeriod= $25
.equ	yHour		= $26
.equ	yMinute		= $27
.equ	ySecond		= $28
.equ	ySecondCntL	= $29
.equ	ySecondCntH	= $2A



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















