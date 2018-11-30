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
	out	PORTB,KS_Dead
	lds	KS_Dead,XSEG+KS_Buf+0
	lds	KS_High,XSEG+KS_Buf+1
	lds	SSReg, XSEG+KS_Buf+2
	out	PORTB,SSREG
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
	ret

/*** Activate timer1 setup PWM start values ***/
EnablePWM:
	ldi_w	r17,r16,PWMInitState ; Minimum phase
	out	OCR1BH,r17
	out	OCR1BL,r16
	clr	KS_High		; Zero state registers
	clr	KS_Dead
	out	PORTB,ZeroReg
	ldi	XL,KS_Buf
	st	X+,KS_Dead	; ... and state buffer
	st	X+,KS_High
	st	X+,ZeroReg
	outi	TIFR,(1<<OCF1B) + (1<<OCF1A) ; Clear interrupt flags
	in	r16,TIMSK	
	ori	r16,(1<<OCIE1B) + (1<<OCIE1A)
	out	TIMSK,r16	  ; Enable interrupts
	outi	TCCR1B,0b00001001 ; Start timer on CK frequency with CTC at OCR1A
	ret

/*** Disactivate timer1 release port ***/
DisablePWM:
	outi	TCCR1B,0b00001000 ; Stop timer
	outi	TIFR,(1<<OCF1B) + (1<<OCF1A) ; Clear interrupt flags
	out	PORTB,ZeroReg
	ret

/*** Start sinchronous meandr on all channels ***/
StartKeysTest:
	rcall	EnablePWM
	ldi_w	r17,r16,PWMPeriod/2
	out	OCR1BH,r17
	out	OCR1BL,r16
	ldi	r16,0b101010	; High
	ldi	r17,0b010101	; Low
	ldi	XL,KS_Buf
	cli
	st	X+,ZeroReg	; Dead state
	st	X+,r16		; High
	st	X+,r17		; Low
	sei
	ret

/*** Track rotor position and speed ***/
TrackRotor:
	in	r16,PORTC
	lsr	r16
	andi	r16,0b111	; Actual state of rotor sensors
	ldd	ZL,Y+yRotorState ; Recorded state
	cp	r16,ZL
	brne	tr_1
	ret		; Nothing changed !!! add logic
tr_4:			; Incorrect step !!!  add logic
	ret

tr_1:	std	Y+yRotorState,r16
	lsl	ZL
	ldi	ZH,0
	subi_w	ZH,ZL,-2*RotorMovement	; Pointer to raw
	ldd	r18,Y+yPanPosL	; load current panel position
	ldd	r19,Y+yPanPosM
	ldd	r20,Y+yPanPosH
	lpm	r17,Z+
	cp	r16,r17	; is it step to west?
	brne	tr_2
	subi_w	r19,r18,-1 ; increase position
	sbci	r20,$FF
	rjmp	tr_3
tr_2:	lpm	r17,Z+
	cp	r16,r17	; is it step to east?
	brne	tr_4
	subi_w	r19,r18,1  ; decrease position
	sbci	r20,0
tr_3:	std	Y+yPanPosL,r18	; save current panel position
	std	Y+yPanPosM,r19
	std	Y+yPanPosH,r20
	ret

RotorMovement:	; Expected next states of rotor sensors ("+" - to west, "-" - to east)
		;  +    -	current
	.db	0b111,0b111	; 000 - ubnormal
	.db	0b011,0b101	; 001
	.db	0b110,0b011	; 010
	.db	0b010,0b001	; 011
	.db	0b101,0b110	; 100
	.db	0b001,0b100	; 101
	.db	0b100,0b010	; 110
	.db	0b000,0b000	; 111 - ubnormal
