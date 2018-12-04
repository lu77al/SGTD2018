/*
StartSimplePWM:
	clt
	bld	AFlags,afReverse
	rcall	EnablePWM
;	ldi_w	r17,r16,PWMPeriod/4 ; Minimum phase
;	out	OCR1BH,r17
;	out	OCR1BL,r16
	stdi	yTargetPeriod,54
	ret
*/

StartTest:
	ldi	r16,$80
	std	Y+yPanPosH,r16
	std	Y+yPanPosM,ZeroReg
	std	Y+yPanPosL,ZeroReg
	std	Y+yTargetPosH,r16
	std	Y+yTargetPosM,ZeroReg
	std	Y+yTargetPosL,ZeroReg
	ret


/* Stop procedure variants
 - regular: (speed profile) + immediate stop
 - fast: set new target pos + double acceleration profile
 */
