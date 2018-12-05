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
	ldi_w	r17,r16,3*60
	sts	iMorningL,r16
	sts	iMorningH,r17
	ldi_w	r17,r16,21*60
	sts	iEveningL,r16
	sts	iEveningH,r17
	ldi_w	r17,r16,12*60
	sts	iParkingL,r16
	sts	iParkingH,r17
	ldi_w	r17,r16,17673
	sts	iHalfSpanL,r16
	sts	iHalfSpanH,r17
	stdi	yHour,12
	rcall	TimeToTargetPos
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
