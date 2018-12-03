.equ	PWMPeriod = 1600	; f = 8e6 / 1600 = 5000Hz
.equ	MaxPWM = PWMPeriod/8*6
.equ	MinPWM = 24
.equ	PWMInitState = MinPWM

/*** Interrupt timer1 compare A (PWM -> high) ***/
SetKeysHigh: 
	out	PORTB,KS_Dead
	rjmp	PC+1
	rjmp	PC+1
	rjmp	PC+1
	out	PORTB,KS_High
	reti

/*** Interrupt timer1 compare B (PWM -> low) ***/
SetKeysLow:
	ROTOR_HS_POWER_ON	; Power hall sensors
	out	PORTB,KS_Dead	; Keys -> dead_state
	push	ZH
	push	ZL
	in	SSReg,SREG
	rjmp	PC+1
	ldi	ZL,0b010101
	out	PORTB,ZL	; Keys -> low_state
	ldi	ZH,0
	rjmp	PC+1
	rjmp	PC+1
	in	ZL,PINC		; Hall sensors state
	ROTOR_HS_POWER_OFF	; Power off hall sensors
	std	Y+yRotorHallsPort,ZL
	sbrc	AFlags,afReverse
	com	ZL		; Apply direction
	andi	ZL,0b1110
	subi	ZL,-DriveKeyTable*2 ; Pointer to table of Keys' states
	lpm	KS_High,Z+
	lpm	KS_Dead,Z
	pop	ZL
	pop	ZH
	out	SREG,SSReg
	reti

/*** KS_Buf (KS - keys state) *******
 * 0 - DeadTime port state
 * 1 - High PWM port state
 * 2 - Low PWM port state
 ************************************/

/*** Init timer1 based PWM frequency ***/
InitPWM:
	ldi_w	r17,r16,PWMPeriod
	out	OCR1AH,r17
	out	OCR1AL,r16	; PWM frequency
	out	TCCR1A,ZeroReg
	out	TCCR1B,ZeroReg	; Reset timer1
	in	r16,TIMSK	
	ori	r16,(1<<OCIE1B) + (1<<OCIE1A)
	out	TIMSK,r16	  ; Enable interrupts
	ret

/*** Activate timer1 setup PWM start values ***/
EnablePWM:
	ldi_w	r17,r16,PWMInitState ; Minimum phase
	out	OCR1BH,r17
	out	OCR1BL,r16
	clr	KS_High		; Zero state registers
	clr	KS_Dead
	out	PORTB,ZeroReg
	outi	TIFR,(1<<OCF1B) + (1<<OCF1A) ; Clear interrupt flags
	outi	TCCR1B,0b00001001 ; Start timer on CK frequency with CTC at OCR1A
	ret

/*** Disactivate timer1 release port ***/
DisablePWM:
	outi	TCCR1B,0b00001000 ; Stop timer
	outi	TIFR,(1<<OCF1B) + (1<<OCF1A) ; Clear interrupt flags
	out	PORTB,ZeroReg
	rjmp	PC+1
	rjmp	PC+1
	rjmp	PC+1
	outi	PORTB,0b010101	; Short drive
	ret


.equ	JAM_SMALL_PATH = 10

/*** Track rotor position and speed (step period) ***/
TrackRotor:
	ldd	r16,Y+yCurPeriod
	inc	r16
	breq	tr_4
	std	Y+yCurPeriod,r16
tr_4:	ldd	r16,Y+yRotorHallsPort
	andi	r16,0b1110	; Actual state of rotor sensors
	ldd	ZL,Y+yRotorState ; Recorded state
	cp	r16,ZL
	breq	tr_1	; Hall state wasn't changed
	ldi	r25,0	; Step Flag
	std	Y+yRotorState,r16
	ldi	ZH,0
	subi_w	ZH,ZL,-2*RotorMovement	; Pointer to raw
	ldd	r18,Y+yPanPosL	; load current panel position
	ldd	r19,Y+yPanPosM
	ldd	r20,Y+yPanPosH
	ldd	r21,Y+yJamPath	; path for jam detection
	lpm	r17,Z+
	cp	r16,r17	; is it step to west?
	brne	tr_2
	sbrs	AFlags,afReverse
	ldd	r25,Y+yCurPeriod
	subi_w	r19,r18,-1 ; increase position
	sbci	r20,$FF
	cpi	r21,128+JAM_SMALL_PATH
	brsh	tr_3
	inc	r21
	rjmp	tr_3
tr_2:	lpm	r17,Z+
	cp	r16,r17	; is it step to east?
	brne	tr_1
	sbrc	AFlags,afReverse
	ldd	r25,Y+yCurPeriod
	subi_w	r19,r18,1  ; decrease position
	sbci	r20,0
	cpi	r21,128-JAM_SMALL_PATH
	brlo	tr_3
	dec	r21
tr_3:	std	Y+yPanPosL,r18	; save current panel position
	std	Y+yPanPosM,r19
	std	Y+yPanPosH,r20
	std	Y+yJamPath,r21
	cpi	r25,0
	brne	tr_5
tr_1:	ldd	r16,Y+yDummyPeriod
	inc	r16
	std	Y+yDummyPeriod,r16
	breq	tr_6
	ret
tr_6:	ldi	r25,$FF
	rjmp	tr_7
tr_5:	std	Y+yCurPeriod,ZeroReg
	std	Y+yDummyPeriod,ZeroReg
tr_7:	std	Y+yLastPeriod,r25


; *** Stabilize ***

	ldd	r16,Y+yPanPosL
	ldd	r17,Y+yPanPosM
	ldi_w	ZH,ZL,20*36
	sub	ZL,r16
	sbc	ZH,r17
	brcc	tb_1
	rjmp	DisablePWM
tb_1:	lsr16	ZH,ZL
	cpi	ZL,85
	cpc	ZH,ZeroReg
	brlo	tb_2
	ldi_w	ZH,ZL,85
tb_2:	subi_w	ZH,ZL,-2*BreakTable
	lpm	r16,Z
	std	Y+yTargetPeriod,r16



	ldd	r25,Y+yLastPeriod

	ldd	r16,Y+yTargetPeriod
	sub	r25,r16
	brcs	st_1
; Add power
	cpi	r25,100
	brlo	st_2
	ldi	r25,100
	rjmp	st_2
st_1:	cpi	r25,-100
	brsh	st_2
	ldi	r25,-100
st_2:
	ldd	r16,Y+yPWML
	ldd	r17,Y+yPWMH
	sbrc	r25,7
	rjmp	st_3
	add	r16,r25
	adc	r17,ZeroReg
	ldi_w	r19,r18,9600
	cp	r18,r16
	cpc	r19,r17
	brsh	st_4
	movw	r17:r16,r19:r18
	rjmp	st_4
st_3:	neg	r25
	lsl	r25
	ldi	r18,0
	lsl16	r18,r25
	sub	r16,r25
	sbc	r17,r18
	brcc	st_4
	ldi_w	r17,r16,0
st_4:	std	Y+yPWML,r16
	std	Y+yPWMH,r17
	lsr16	r17,r16
	lsr16	r17,r16
	lsr16	r17,r16
	subi_w	r17,r16,-24
	out	OCR1BH,r17
	out	OCR1BL,r16
	ret


RotorMovement:	; Expected next states of rotor sensors ("+" - to west, "-" - to east)
		;  +    -	current
	.db	0b1110,0b1110	; 000 - ubnormal
	.db	0b0110,0b1010	; 001
	.db	0b1100,0b0110	; 010
	.db	0b0100,0b0010	; 011
	.db	0b1010,0b1100	; 100
	.db	0b0010,0b1000	; 101
	.db	0b1000,0b0100	; 110
	.db	0b0000,0b0000	; 111 - ubnormal


BreakTable:
	.db	200,189,179,171,164,157,151,146,141,136	; 0
	.db	132,128,124,121,118,115,112,110,107,105	; 10
	.db	103,101, 99, 97, 95, 93, 92, 90, 89, 88	; 20
	.db	 86, 85, 84, 83, 82, 80, 79, 78, 78, 77	; 30
	.db	 76, 75, 74, 73, 73, 72, 71, 70, 70, 69	; 40
	.db	 68, 68, 67, 67, 66, 66, 65, 65, 64, 64	; 50
	.db	 63, 63, 62, 62, 61, 61, 61, 60, 60, 59	; 60
	.db	 59, 59, 58, 58, 58, 57, 57, 57, 56, 56	; 70
	.db	 56, 55, 55, 55, 55, 54			; 80..85
