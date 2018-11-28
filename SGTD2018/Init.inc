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


	sei


