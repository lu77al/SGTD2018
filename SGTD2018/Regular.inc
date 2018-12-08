PrepareButtonsF:
	ldi	r17,$F0
	lds	r16,iEastLim_F
	cpi	r16,0
	breq	pbf_10
	cpi	r16,1
	breq	pbf_11
	ori	r17,1<<btEastLimit
	rjmp	pbf_10
pbf_11:	andi	r17,$FF-(1<<(btEastLimit+4))
pbf_10:
	lds	r16,iWestLim_F
	cpi	r16,0
	breq	pbf_20
	cpi	r16,1
	breq	pbf_21
	ori	r17,1<<btWestLimit
	rjmp	pbf_20
pbf_21:	andi	r17,$FF-(1<<(btWestLimit+4))
pbf_20:
	lds	r16,iEastBut_F
	cpi	r16,0
	breq	pbf_30
	cpi	r16,1
	breq	pbf_31
	ori	r17,1<<btEastButton
	rjmp	pbf_30
pbf_31:	andi	r17,$FF-(1<<(btEastButton+4))
pbf_30:
	lds	r16,iWestBut_F
	cpi	r16,0
	breq	pbf_40
	cpi	r16,1
	breq	pbf_41
	ori	r17,1<<btWestButton
	rjmp	pbf_40
pbf_41:	andi	r17,$FF-(1<<(btWestButton+4))
pbf_40:
	com	r17
	sts	iButtons_F,r17
	ret

ReadBLS: ; Read buttons and limit sensors
	LIMIT_HS_POWER_ON
	rjmp	PC+1
	rjmp	PC+1
	rjmp	PC+1
	rjmp	PC+1
	rjmp	PC+1
	rjmp	PC+1
	rjmp	PC+1
	lds	r21,iButtons_F
	ldd	r17,Y+yButState ; Prev_State
	ldd	r18,Y+yButState
	com	r18		; Prev_State^
	ldd	r19,Y+yButPress	   ; Press flags
	ldd	r20,Y+yButRelease  ; Release flags
	in	r16,PIND
	LIMIT_HS_POWER_OFF
	bst	r16,7
	bld	r16,6
	lsl	r16
	swap	r16	; State^
	and	r16,r21
	swap	r21
	or	r16,r21
	and	r17,r16	; Released = Prev_State & State^
	or	r20,r17
	com	r16
	ldd	r17,Y+yButState ; Test for buttons change
	eor	r17,r16
	andi	r17,(1<<btEastButton) + (1<<btWestButton)
	breq	rbls_1
	std	Y+yButStateCnt,ZeroReg
rbls_1:	and	r18,r16	; Pressed = PrevState^ & State
	or	r19,r18
	std	Y+yButState,r16
	std	Y+yButPress,r19
	std	Y+yButRelease,r20
	ret


TestLimitPressed:
	ldd	r25,Y+yButPress
	mov	r16,r25
	andi	r16,$FF - (1<<btEastLimit) - (1<<btWestLimit)
	std	Y+yButPress,r16
	sbrs	r25,btEastLimit	; East limit trigger
	rjmp	ph_1
	sbrc	AFlags,afSetupMove
	rcall	EastLimitSetup
	set
	rjmp	ph_2
ph_1:	sbrs	r25,btWestLimit	; West limit trigger
	ret
	sbrc	AFlags,afSetupMove
	rcall	WestLimitSetup
	clt
ph_2:	rcall	SetPosOnLimit
	rjmp	UpdateTimeOffset


SetPosOnLimit:
	ldi_w	r17,r16,HALF_CIRCLE
	ldi	r18,0
	lds	r19,iHalfSpanL
	lds	r20,iHalfSpanH
	brcc	spl_1
	sub	r16,r19
	sbc	r17,r20
	sbc	r18,ZeroReg
	rjmp	spl_2
spl_1:	add	r16,r19
	adc	r17,r20
	adc	r18,ZeroReg
spl_2:	std	Y+yPanPosL,r16
	std	Y+yPanPosM,r17
	std	Y+yPanPosH,r18
	ret

StopOnLimits:
	sbrc	AFlags,afActive	; Skip not active and already stopping
	sbrc	AFlags,afStopping
soh_1:	ret
	ldd	r16,Y+yButState
	ldi	r17,(1<<btWestLimit)
	sbrc	AFlags,afReverse
	ldi	r17,(1<<btEastLimit)
	and	r17,r16
	breq	soh_1
	set
	bld	AFlags,afFastStop
	bld	AFlags,afFailStop
	rcall	StopDrive
	rjmp	CalculateNextDelay

CalculateNextDelay:
	ldd	r16,Y+yNextDelay
	sbrc	AFlags,afSetupMove
	ldi	r16,0
	lsl	r16
	inc	r16
	std	Y+yNextDelay,r16
	std	Y+yDelayCntH,r16
	std	Y+yDelayCntL,ZeroReg
	ret


InitADC:
	outi	ADCSR,0b10000110	; Enable ADC, CK/64
	outi	SFIOR,0
	outi	ADMUX,0b11100110
	sbi	ADCSR,ADSC
	ret

ReadADC:
	sbic	ADCSR,ADSC
	ret
	in	r16,ADCL ; Result
	in	r17,ADCH
	in	r18,ADMUX
	cpi	r18,0b11100110	; Current
	brne	adc_1
	lds	r18,iSleepCnt	; SleepCnt
	sbrc	AFlags,afActive
	ldi	r18,20
	subi	r18,1
	brcs	adc_2
	sts	iSleepCnt,r18	; Work current
	lds	r18,iIzeroL
	lds	r19,iIzeroH
	sub	r18,r16
	sbc	r19,r17		; IadcZero - Iadc
	brcc	adc_3
	ldi_w	r19,r18,0
adc_3:	sts	iIadcL,r18
	sts	iIadcH,r19	
	ldi_w	r17,r16,255	; Calculate I (0.1A)
	rcall	K16x16
	sts	iCurrent,r16
	rjmp	adc_5
adc_2:	sts	iIzeroL,r16	; Zero current
	sts	iIzeroH,r17
	sts	iIadcL,ZeroReg
	sts	iIadcH,ZeroReg
	sts	iCurrent,ZeroReg
adc_5:	rcall	PrepareIreg
	ldi	r18,0b11100111
	rjmp	adc_4
adc_1:				; Voltage
	sts	iUadcL,r16
	sts	iUadcH,r17
	K16x16m	813
	sts	iUL,r16
	sts	iUH,r17
	rcall	PrepareUreg
	ldi	r18,0b11100110
adc_4:	out	ADMUX,r18
	sbi	ADCSR,ADSC
	ret

PrepareUreg:
	lds	r16,iUadcL
	lds	r17,iUadcH
	lsr16	r17,r16
	lsr16	r17,r16
	lsr16	r17,r16
	lsr16	r17,r16
	lsr16	r17,r16	; / 32 (2048)
	ldi_w	r19,r18,1612 ; 64 V
	sub	r18,r16
	sbc	r19,r17
	ldi_w	r17,r16,-100
	cp	r18,r16
	cpc	r19,r17
	brge	purg_1
	ldi	r18,-100
	rjmp	purg_2
purg_1:	cpi	r18,100
	cpc	r19,ZeroReg
	brlt	purg_2
	ldi	r18,100
purg_2:	std	Y+yUreg,r18
	ret

PrepareIreg:
	lds	r16,iIadcL
	lds	r17,iIadcH
	lsr16	r17,r16
	lsr16	r17,r16
	lsr16	r17,r16
	lsr16	r17,r16
	lsr16	r17,r16	; / 32 (2048)
	ldi_w	r19,r18,640 ; 8 A
	sub	r18,r16
	sbc	r19,r17
	ldi_w	r17,r16,-100
	cp	r18,r16
	cpc	r19,r17
	brge	pirg_1
	ldi	r18,-100
	rjmp	pirg_2
pirg_1:	cpi	r18,100
	cpc	r19,ZeroReg
	brlt	pirg_2
	ldi	r18,100
pirg_2:	std	Y+yIreg,r18
	ret

