.def	SSREG	= r2

.def	AFlags	= r3
	.equ	afActive     = 0
	.equ	afReverse    = 1
	.equ	afFastStop   = 2
	.equ	afFailStop   = 3
	.equ	afStopping   = 4
	.equ	afSetupMove  = 5
	.equ	afWaitButRel = 6
	.equ	afSaveParams = 7

.def	BFlags	= r4
	.equ	bfRTCRead  = 0
	.equ	bfRTCWrite = 1

.def	KS_Dead    = r5
.def	KS_High    = r6

#define	ZeroReg	YH

.equ	XSEG = $100
; --- Arrays ($100..$1FF)
.equ	RTCBuf  = $00
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
.equ	yAFlags		= $09
.equ	yRTCDebug	= $0A
.equ	yADCChCnt	= $0B
.equ	yButState	= $0C
.equ	yButPress	= $0D
.equ	yButRelease	= $0E
.equ	yButStateCnt	= $0F

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
.equ	yDelayCntL	= $23
.equ	yDelayCntH	= $24
.equ	yNextDelay	= $25
.equ	yHour		= $26
.equ	yMinute		= $27
.equ	ySecond		= $28
.equ	ySecondCntL	= $29
.equ	ySecondCntH	= $2A
.equ	yStopLength	= $2B
.equ	yUreg		= $2C
.equ	yIreg		= $2D
.equ	yRTCStage	= $2E
.equ	yRTCWaitStatus	= $2F
.equ	yLimSetupStage  = $30

.equ	iHalfSpanL	= $A0
.equ	iHalfSpanH	= $A1
.equ	iTimeOffsetL	= $A2	; /2
.equ	iTimeOffsetH	= $A3
.equ	iMorningL	= $A4
.equ	iMorningH	= $A5
.equ	iEveningL	= $A6
.equ	iEveningH	= $A7
.equ	iParkingL	= $A8
.equ	iParkingH	= $A9
.equ	iInitCnt	= $AA

.equ	iButtons_F	= $AB
.equ	iEastLim_F	= $AC
.equ	iWestLim_F	= $AD
.equ	iEastBut_F	= $AE
.equ	iWestBut_F	= $AF
.equ	iIzeroL		= $B0
.equ	iIzeroH		= $B1
.equ	iIadcL		= $B2
.equ	iIadcH		= $B3
.equ	iCurrent	= $B4
.equ	iUadcL		= $B5
.equ	iUadcH		= $B6
.equ	iUH		= $B7
.equ	iUL		= $B8
.equ	iSleepCnt	= $B9
