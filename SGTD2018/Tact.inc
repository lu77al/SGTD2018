InitTact:
	outi	TCCR2,0b010	; CK/8
;	outi	OCR2,$40
	ret

.macro	tskTact		;----- 砫膷 3906.25 Hz -----
	in	r16,TIFR
	sbrc	r16,TOV2
	rjmp	Tact
.endm

Tact: ;	ROTOR_HS_POWER_ON
	outi	TIFR,1<<TOV2

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

	sbic	ROTOR_HS_POWER
	sbrc	AFLags,afActive
	rjmp	tct_2
	rcall	GetHallState
tct_2:
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
	cpi	r23,0b00011111	; 15.2587890625 關
	brne	skBr0

	ldi_w	r20,r19,END_POS_GAP
	rcall	CorrectPosAtEdge	; Don't allow reach 0 and FULL_CIRCLE

	rcall	CheckTargetPos

	rjmp	RetTact
skBr0:

;--- Br ---
	cpi	r23,0b00111111	; 15.2587890625 關
	brne	sk_BR1

	andi	r24,0b11
	brne	br1_1
	rcall	DelayCountdown
br1_1:
	ldd	r24,Y+yMainCntM
	cpi	r24,$FF
	brne	br1_2
	ldd	r25,Y+yMainCntH
	andi	r25,0b11
	brne	br1_2
	lds	r16,iNoRequestsCnt
	subi	r16,1
	brcc	br1_3
	stsi	iHCResetCnt,60
	ldi	r16,128
br1_3:	sts	iNoRequestsCnt,r16	
br1_2:
	lds	r16,iHCResetCnt
	subi	r16,1
	brcs	br1_4
	sts	iHCResetCnt,r16
	HC_POWER_OFF
	rjmp	br1_5
br1_4:	HC_POWER_ON
br1_5:

	rjmp	RetTact
sk_BR1:


;--- Br ---
	cpi	r23,0b01011111	; 15.2587890625 關
	brne	sk_BR2

	rcall	ATSetupDelayCountDown

	rcall	CheckJam

	rcall	CheckSaveParam

	rcall	DebugCmdStart

	rjmp	RetTact
sk_BR2:


;--- Br ---
	cpi	r23,0b01111111	; 15.2587890625 關
	brne	sk_BR3

	lds	r16,iInitCnt
	subi	r16,1
	brcs	br3_2
	sts	iInitCnt,r16
	cpi	r16,$FF-30
	brsh	br3_3
	sts	iInitCnt,r16
	andi	r16,0b11
	brne	br3_3
	rcall	StartRTCRead
	rjmp	br3_3
br3_2:
	andi	r24,0b11111
	brne	br3_3
	rcall	CheckForRTCWrite
br3_3:

	rcall	TimeGo

	rcall	PrepareButtonsF

	rjmp	RetTact
sk_BR3:

;--- Br ---
	cpi	r23,0b10011111	; 15.2587890625 關
	brne	sk_BR4

	rcall	TimeToTargetPos

	rcall	ProcessButtons

	std	Y+yAFlags,AFlags

	lds	r16,iLedLightCnt
	subi	r16,1
	brcs	i_6
	sts	iLedLightCnt,r16
	rjmp	i_2
i_6:
	sbrc	BFLags,bfATSetup
	rjmp	i_2

	ldd	r16,Y+yButState
	andi	r16,(1<<btEastLimit) + (1<<btWestLimit)
	breq	i_7
	ldd	r24,Y+yMainCntM
	sbrs	r24,0
	rjmp	i_2
	rjmp	i_3
i_7:
	sbrc	AFlags,afActive
	rjmp	i_3	
	ldd	r24,Y+yMainCntM
	ldd	r16,Y+yDelayCntL
	ldd	r17,Y+yDelayCntH
	cp	r16,ZeroReg
	cpc	r17,ZeroReg
	brne	i_1
	andi	r24,0b11111
	cpi	r24,0b00010
	breq	i_2
	ldd	r18,Y+yButState
	andi	r18,(1<<btEastLimit) + (1<<btWestLimit)
	breq	i_3
	cpi	r24,0b00100
	breq	i_2
	sbrs	r18,btWestLimit
	rjmp	i_3
	cpi	r24,0b00110
	breq	i_2
	rjmp	i_3
i_1:
	cpi	r17,3
	brsh	i_5
	andi	r24,0b1111
	breq	i_2
	rjmp	i_3
i_5:	andi	r24,0b111
	breq	i_2
	rjmp	i_3
	

i_3:	LED_OFF
	rjmp	i_4
i_2:	LED_ON
i_4:

	rjmp	RetTact
sk_BR4:

;--- Br ---
	andi	r23,0b01111111	; 30.517578125 關
	cpi	r23,0b00000001
	brne	sk_BR5_0

	LIMIT_HS_POWER_ON

	rjmp	RetTact
sk_BR5_0:

;--- Br ---
	cpi	r23,0b00000010	; 30.517578125 關
	brne	sk_BR5

	rcall	ReadBLS

	lds	r16,iInitCnt
	cpi	r16,$FF-10
	brlo	br5_1
	std	Y+yButPress,ZeroReg
	std	Y+yButRelease,ZeroReg
	rjmp	br5_2
br5_1:
	rcall	TestLimitPressed

	rcall	StopOnLimits
br5_2:
	ROTOR_HS_POWER_ON

	rjmp	RetTact
sk_BR5:

;--- Br ---
	cpi	r23,0b00000011	; 30.517578125 關
	brne	sk_BR7

	ldd	r16,Y+yDriveHallActCnt
	sbrc	AFlags,afActive
	ldi	r16,120
	subi	r16,1
	brcs	br7_1
	std	Y+yDriveHallActCnt,r16
	rjmp	br7_2
br7_1:	ROTOR_HS_POWER_OFF
br7_2:

	rjmp	RetTact
sk_BR7:

;--- Br ---
	andi	r23,0b00011111	; 122.0703125 關
	cpi	r23,0b00000100
	brne	sk_BR6

	rcall	ReadADC

	rjmp	RetTact
sk_BR6:

	rjmp	RetTact
