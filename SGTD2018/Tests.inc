StartSimplePWM:
	rcall	EnablePWM
	ldi_w	r17,r16,PWMPeriod/8 ; Minimum phase
	out	OCR1BH,r17
	out	OCR1BL,r16
	ret
