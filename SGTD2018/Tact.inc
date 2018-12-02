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

;-- MainCnt 24bit
	ldd	r23,Y+yMainCntL
	ldd	r24,Y+yMainCntM
	ldd	r25,Y+yMainCntH
	subi_w	r24,r23,-1
	sbci	r25,$FF
	std	Y+yMainCntL,r23
	std	Y+yMainCntM,r24
	std	Y+yMainCntH,r25

	rjmp	PC+1	; Delay for Rotor HS stabilize 
	rjmp	PC+1	; to stabilize power of rot
	rjmp	PC+1
	rjmp	PC+1





	ROTOR_HS_POWER_ON
	LIMIT_HS_POWER_ON
	rjmp	PC+1
	rjmp	PC+1
	rjmp	PC+1
	rjmp	PC+1
	rjmp	PC+1
	rjmp	PC+1
	rjmp	PC+1
	rjmp	PC+1
	rjmp	PC+1
	rjmp	PC+1
	ROTOR_HS_POWER_OFF
	LIMIT_HS_POWER_OFF


/*	ldi	r16,0b111
	and	r16,r23
	cpi	r16,1
	brlo	tt_1
	ROTOR_HS_POWER_OFF
	rjmp	tt_2
tt_1:	ROTOR_HS_POWER_ON
tt_2:*/

;--- Br0 ---
	cpi	r23,0b01100111
	brne	skBr0

	rjmp	RetTact
skBr0:


;--- Br ---
	cpi	r23,0b10100111; 15 關
	brne	sk_BR6


	rjmp	RetTact
sk_BR6:


;--- Br2 ---
	andi	r23,0b01111111
	cpi	r23,0b00010111	; 30 關
	brne	sk_BR2

	rjmp	RetTact
sk_BR2:


;--- Br3 ---
	andi	r23,0b00111111	; 61 關
	cpi	r23,0b00010011
	brne	sk_BR3

	rjmp	RetTact
sk_BR3:

;--------- 扱闀膱 ------- 122 關
	andi	r23,0b00011111 
	cpi	r23,0b00000001
	brne	skBut

	rjmp	RetTact
skBut:
	rjmp	RetTact
