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
	sbrs	AFlags,afWestDir
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
	ldd	r16,Y+yRotorHallsPort
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
	sbrc	AFlags,afWestDir
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
	sbrs	AFlags,afWestDir
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
	breq	tr_1
	std	Y+yLastPeriod,r25
	std	Y+yCurPeriod,ZeroReg
tr_1:
	ldd	r16,Y+yCurPeriod
	inc	r16
	breq	tr_4			; Max period = 255 -> 3906.25 / 255 / 36 = 0.426 Hz (2.35s/r)
	std	Y+yCurPeriod,r16
tr_4:
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

/*
; 120rpm speed in old driver
StabSpeed: 
	ldd	r16,Y+yCurPeriod
	ldd	r17,Y+yLastPeriod
	cpi	r16,r17
	brsh	ss_1
	mov	r16,r17
ss_1:	ldd	r17,Y+yTargetPeriod
	sub	r16,r17
	brcs	ss_2
	cpi	r16,20
	brlo	ss_

	add	r18,
	

	

	ret
*/
