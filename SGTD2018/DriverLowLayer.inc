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
