;-- Init ports state --
	InitPorts

;-- Стек и указатели --
	outi	SPH,high(RAMEND)
	outi	SPL,low(RAMEND)
	ldi_w	YH,YL,$60
	ldi_w	XH,XL,$100

;-- Очиститка памяти с $100 по $1ff
	ldi_w	ZH,ZL,$60
ini_1:	st	Z+,YH
	cpi	ZH,2
	brlo	ini_1

	ldi_w	YH,YL,YSEG
	ldi_w	XH,XL,XSEG

	clr	AFlags
	clr	BFlags

	rcall	InitTact

	rcall	InitUart

	rcall	InitPWM

	rcall	PrepareButtonsF

	stsi	iInitCnt,$FF
	stdi	ySimButMask,$FF


	stdi	yDelayCntH,3
	set
	bld	AFlags,afWaitButRel

	rcall	InitADC

	stdi	yHour,12
	rcall	TimeToTargetPos
	ldd	r16,Y+yTargetPosL
	ldd	r17,Y+yTargetPosM
	ldd	r18,Y+yTargetPosH
	std	Y+yBakTargetPosL,r16
	std	Y+yBakTargetPosM,r17
	std	Y+yBakTargetPosH,r18
	std	Y+yPanPosL,r16
	std	Y+yPanPosM,r17
	std	Y+yPanPosH,r18

	rcall	ReadParams

	sei


