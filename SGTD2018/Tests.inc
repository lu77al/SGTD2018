StartSimplePWM:
	clt
	bld	AFlags,afReverse
	rcall	EnablePWM
;	ldi_w	r17,r16,PWMPeriod/4 ; Minimum phase
;	out	OCR1BH,r17
;	out	OCR1BL,r16
	stdi	yTargetPeriod,54
	ret
