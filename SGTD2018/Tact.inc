InitTact:
	outi	TCCR2,0b010	; CK/8
;	outi	OCR2,$40
	ret

.macro	tskTact		;----- 砫膷 3906.25 Hz -----
	in	r16,TIFR
	sbrc	r16,TOV2
	rjmp	Tact
.endm

Tact:	outi	TIFR,1<<TOV2

;**** RXTimer ****
	ldd	r16,Y+yRXTimer
	inc	r16
	breq	rxtm_1
	std	Y+yRXTimer,r16
	cpi	r16,UARTByteDelay
	brne	rxtm_1
	stdi	yRXPointer,RXBuf-1	
rxtm_1:
;**** RXTimer ****

	rcall	TrackRotor

;-- MainCnt 24bit
	ldd	r23,Y+yMainCntL
	ldd	r24,Y+yMainCntM
	ldd	r25,Y+yMainCntH
	subi_w	r24,r23,-1
	sbci	r25,$FF
	std	Y+yMainCntL,r23
	std	Y+yMainCntM,r24
	std	Y+yMainCntH,r25

;--- Br0 ---
	cpi	r23,0b00001111	; 15.2587890625 關
	brne	skBr0

	rcall	CheckFixPanPos	; Don't allow reach 0 and FULL_CIRCLE

	rcall	CheckTargetPos

	rjmp	RetTact
skBr0:

;--- Br ---
	cpi	r23,0b00011111	; 15.2587890625 關
	brne	sk_BR1

	andi	r24,0b11
	brne	br1_1
	rcall	CountdownAfterFail
br1_1:
	rjmp	RetTact
sk_BR1:


;--- Br ---
	cpi	r23,0b00101111	; 15.2587890625 關
	brne	sk_BR2

	rcall	CheckJam

	rjmp	RetTact
sk_BR2:


;--- Br ---
	cpi	r23,0b00111111	; 15.2587890625 關
	brne	sk_BR3

	rcall	TimeGo

	rcall	PrepareButtonsF

	rjmp	RetTact
sk_BR3:

;--- Br ---
	cpi	r23,0b01001111	; 15.2587890625 關
	brne	sk_BR4

	rcall	TimeToTargetPos

	rcall	ProcessButtons

	std	Y+yAFlags,AFlags

	rjmp	RetTact
sk_BR4:

;--- Br ---
	andi	r23,0b00011111	; 122.0703125 關
	cpi	r23,0b00000111
	brne	sk_BR5

	rcall	ReadBLS

	rcall	TestLimitPressed

	rcall	StopOnLimits	

	rjmp	RetTact
sk_BR5:

	rjmp	RetTact
