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
	stdi	yHour,12
	rcall	SetTargetPos
	ldd	r16,Y+yTargetPosL
	ldd	r17,Y+yTargetPosM
	ldd	r18,Y+yTargetPosH
	std	Y+yPanPosL,r16
	std	Y+yPanPosM,r17
	std	Y+yPanPosH,r18
	ret



/* Stop procedure variants
 - regular: (speed profile) + immediate stop
 - fast: set new target pos + double acceleration profile
 */
