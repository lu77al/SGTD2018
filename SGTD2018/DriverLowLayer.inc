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
	ldi_w	r17,r16,PWM_PERIOD
	out	OCR1AH,r17
	out	OCR1AL,r16	; PWM frequency
	out	TCCR1A,ZeroReg
	out	TCCR1B,ZeroReg	; Reset timer1
	in	r16,TIMSK	
	ori	r16,(1<<OCIE1B) + (1<<OCIE1A)
	out	TIMSK,r16	  ; Enable interrupts
	ret

/*** Activate timer1 setup PWM start values ***/
DriveOn:
	set
	bld	AFlags,afActive
	std	Y+yPWML,ZeroReg
	std	Y+yPWMH,ZeroReg
	std	Y+yAccelCnt,ZeroReg
	ldi	r16,$80+JAM_INIT_PATH
	sbrc	AFlags,afReverse
	ldi	r16,$80-JAM_INIT_PATH
	std	Y+yJamPath,r16

	ldi_w	r17,r16,PWM_MIN_PULSE ; Minimum pulse
	out	OCR1BH,r17
	out	OCR1BL,r16
	clr	KS_High		; Zero state registers
	clr	KS_Dead
	out	PORTB,ZeroReg
	outi	TIFR,(1<<OCF1B) + (1<<OCF1A) ; Clear interrupt flags
	outi	TCCR1B,0b00001001 ; Start timer on CK frequency with CTC at OCR1A
	ret

/*** Disactivate timer1 release port ***/
DriveOff:
	outi	TCCR1B,0b00001000 ; Stop timer
	outi	TIFR,(1<<OCF1B) + (1<<OCF1A) ; Clear interrupt flags
	out	PORTB,ZeroReg
	rjmp	PC+1
	rjmp	PC+1
	rjmp	PC+1
	clt
	bld	AFlags,afActive
	outi	PORTB,0b010101	; Short drive
	std	Y+yAfterFailCntL,ZeroReg
	std	Y+yAfterFailCntH,ZeroReg
	std	Y+yAfterFailPeriod,ZeroReg
	ret


GetHallState:
	ROTOR_HS_POWER_ON
	rjmp	PC+1
	rjmp	PC+1
	rjmp	PC+1
	rjmp	PC+1
	rjmp	PC+1
	rjmp	PC+1
	rjmp	PC+1
	rjmp	PC+1
	rjmp	PC+1
	in	r16,PINC
	ROTOR_HS_POWER_OFF	; Power off hall sensors
	std	Y+yRotorHallsPort,r16
	ret

/*** Track rotor position and speed (step period) ***/
TrackRotor:
	ldd	r16,Y+yCurPeriod
	inc	r16
	breq	tr_4
	std	Y+yCurPeriod,r16
tr_4:	sbrs	AFlags,afActive	; If PWM isn't active -> get halls port state
	rcall	GetHallState
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
	sbrs	AFlags,afReverse
	ldd	r25,Y+yCurPeriod
	subi_w	r19,r18,-1 ; increase position
	sbci	r20,$FF
	cpi	r21,$80+JAM_PATH
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
	cpi	r21,$80-JAM_PATH
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

	sbrs	AFlags,afActive
	ret

; *** Set speed ***
	sbrs	AFlags,afReverse	; Calculate rest of target path
	rjmp	ss_1
	ldd	r16,Y+yPanPosL		;  - reverse (to East): Real-Target
	ldd	r17,Y+yPanPosM
	ldd	r18,Y+yPanPosH
	ldd	r19,Y+yStopPosL
	ldd	r20,Y+yStopPosM
	ldd	r21,Y+yStopPosH
	rjmp	ss_2
ss_1:	ldd	r19,Y+yPanPosL		; - direct (to West): Target-Real
	ldd	r20,Y+yPanPosM
	ldd	r21,Y+yPanPosH
	ldd	r16,Y+yStopPosL
	ldd	r17,Y+yStopPosM
	ldd	r18,Y+yStopPosH
ss_2:
	sub	r16,r19
	sbc	r17,r20
	sbc	r18,r21
	brcc	ss_3		; if RestPath<0
	rcall	DriveOff	;    -> Stop drive
	ret
ss_3:
	or	r17,r18	; Too far from target position -> cruising speed
	brne	ss_4
	sbrs	AFlags,afFastStop
	rjmp	ss_5
	lsl	r16	; Double deacceleration
	brcs	ss_4
ss_5:	cpi	r16,120	; Too far from target position
	brlo	ss_6
ss_4:	ldi	r16,120		; -> cruising speed 
ss_6:	ldd	r17,Y+yAccelCnt
	inc	r17	; Increase start path counter
	cpi	r17,120
	brlo	ss_7
	ldi	r17,119	; limit start path
ss_7:	std	Y+yAccelCnt,r17
	cp	r16,r17	; Select lower value (start path, break path)
	brlo	ss_8
	mov	r16,r17
ss_8:	ldi_w	ZH,ZL,2*BreakTable ; Get value from the table
	add	ZL,r16
	adc	ZH,ZeroReg
	lpm	r16,Z
	std	Y+yNeedPeriod,r16


; *** Stabilize ***
	ldd	r20,Y+yLastPeriod

	ldd	r16,Y+yNeedPeriod
	sub	r20,r16
	brcs	st_1
; Add power
	cpi	r20,60
	brlo	st_2
	ldi	r20,60
	rjmp	st_2
st_1:	cpi	r20,-60
	brsh	st_2
	ldi	r20,-60
st_2:
	ldd	r16,Y+yPWML
	ldd	r17,Y+yPWMH
	sbrc	r20,7
	rjmp	st_3
	add	r16,r20
	adc	r17,ZeroReg
	ldi_w	r19,r18,2400
	cp	r18,r16
	cpc	r19,r17
	brsh	st_4
	movw	r17:r16,r19:r18
	rjmp	st_4
st_3:	neg	r20
	sub	r16,r20
	sbc	r17,ZeroReg
	brcc	st_4
	ldi_w	r17,r16,0
st_4:	std	Y+yPWML,r16
	std	Y+yPWMH,r17
	lsr16	r17,r16
	subi_w	r17,r16,-PWM_MIN_PULSE
	out	OCR1BH,r17
	out	OCR1BL,r16
	ret

CheckFixPanPos:
	ldd	r16,Y+yPanPosL
	ldd	r17,Y+yPanPosM
	ldd	r18,Y+yPanPosH
	cpi	r16,20
	cpc	r17,ZeroReg
	cpc	r18,ZeroReg
	brge	cfp_1
	ldi_w	r17,r16,20
	ldi	r18,0
	rjmp	cfp_2
cfp_1:	ldi_w	r21,r20,FULL_CIRCLE-20
	ldi	r22,BYTE3(FULL_CIRCLE-20)
	cp	r20,r16
	cpc	r21,r17
	cpc	r22,r18
	brlo	cfp_3
	ret
cfp_3:	movw	r17:r16,r21:r20
	mov	r18,r22
cfp_2:	std	Y+yPanPosL,r16
	std	Y+yPanPosM,r17
	std	Y+yPanPosH,r18
	ret

CheckTargetPos:
	set
	ldd	r19,Y+yTargetPosL
	ldd	r20,Y+yTargetPosM
	ldd	r21,Y+yTargetPosH
	ldd	r16,Y+yPanPosL
	ldd	r17,Y+yPanPosM
	ldd	r18,Y+yPanPosH
	sub	r16,r19
	sbc	r17,r20
	sbc	r18,r21
	brcc	ctp_1
	clt
	com	r16
	com	r17
	com	r18
	subi_w	r17,r16,-1
	sbci	r18,-1
ctp_1:	
	cpi_w	r17,r16,MAX_POS_DIFF,r25
	cpc	r18,ZeroReg
	brsh	ctp_2
	ret
ctp_2:
	bld	AFlags,afReverse
	std	Y+yStopPosL,r19
	std	Y+yStopPosM,r20
	std	Y+yStopPosH,r21
	rcall	DriveOn
	ret

CheckJam:
	ldd	r16,Y+yJamPath
	sbrc	AFlags,afReverse
	rjmp	chj_1
	dec	r16
	cpi	r16,$81
	brlo	chj_2
chj_3:	std	Y+yJamPath,r16
	ret
chj_1:	inc	r16
	cpi	r16,$80
	brlo	chj_3
chj_2:	rjmp	StopFail

StopFail:
	ldd	r25,Y+yAfterFailPeriod
	rcall	DriveOff
	lsl	r25
	inc	r25
	std	Y+yAfterFailPeriod,r25
	std	Y+yAfterFailCntH,r25
	std	Y+yAfterFailCntL,ZeroReg
	ret

CountdownAfterFail:
	ldd	r16,Y+yAfterFailCntL
	ldd	r17,Y+yAfterFailCntH
	subi_w	r17,r16,1
	brcs	caf_1
	std	Y+yAfterFailCntL,r16
	std	Y+yAfterFailCntH,r17
caf_1:	ret


RotorMovement:	; Expected next states of rotor sensors ("+" - to west, "-" - to east)
		;  +    -	current
	.db	0b0001,0b0001	; 000 - ubnormal
	.db	0b0110,0b1010	; 001
	.db	0b1100,0b0110	; 010
	.db	0b0100,0b0010	; 011
	.db	0b1010,0b1100	; 100
	.db	0b0010,0b1000	; 101
	.db	0b1000,0b0100	; 110
	.db	0b0001,0b0001	; 111 - ubnormal


BreakTable: ; Periods for each step (from TargetPos to free move)
	.db	200,192,185,179,173,168,163,159,155,151 ; 0
	.db	147,143,140,137,134,131,128,126,124,121	; 10
	.db	119,117,115,113,111,110,108,106,105,103	; 20
	.db	102,100, 99, 98, 97, 95, 94, 93, 92, 91	; 30
	.db	 90, 89, 88, 87, 86, 85, 85, 84, 83, 82	; 40
	.db	 82, 81, 80, 79, 79, 78, 77, 77, 76, 76	; 50
	.db	 75, 75, 74, 73, 73, 72, 72, 71, 71, 70	; 60
	.db	 70, 70, 69, 69, 68, 68, 67, 67, 67, 66	; 70
	.db	 66, 66, 65, 65, 64, 64, 64, 63, 63, 63	; 80
	.db	 63, 62, 62, 62, 61, 61, 61, 61, 60, 60	; 90
	.db	 60, 59, 59, 59, 59, 58, 58, 58, 58, 58	; 100
	.db	 57, 57, 57, 57, 56, 56, 56, 56, 56, 54	; 110..119
 
