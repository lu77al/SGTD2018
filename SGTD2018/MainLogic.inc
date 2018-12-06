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
	and	r18,r16	; Pressed = PrevState^ & State
	or	r19,r18
	std	Y+yButState,r16
	std	Y+yButPress,r19
	std	Y+yButRelease,r20
	ret

UpdatePosOnLimits:
	ldd	r25,Y+yButPress
	mov	r16,r25
	andi	r16,$FF - (1<<btEastLimit) - (1<<btWestLimit)
	std	Y+yButPress,r16
	sbrs	r25,btEastLimit	; East limit trigger
	rjmp	ph_1
	sbrc	AFlags,afSetup
	rjmp	EastLimitSetup
	ldi_w	r17,r16,HALF_CIRCLE - HALL_OFFSET
	ldi	r18,0
	lds	r19,iHalfSpanL
	lds	r20,iHalfSpanH
	sub	r16,r19
	sbc	r17,r20
	rjmp	ph_2
ph_1:	sbrs	r25,btWestLimit	; West limit trigger
	rjmp	ph_3
	sbrc	AFlags,afSetup
	rjmp	WestLimitSetup
	ldi_w	r17,r16,HALF_CIRCLE + HALL_OFFSET
	ldi	r18,0
	lds	r19,iHalfSpanL
	lds	r20,iHalfSpanH
	add	r16,r19
	adc	r17,r20
	adc	r18,ZeroReg
ph_2:	std	Y+yPanPosL,r16
	std	Y+yPanPosM,r17
	std	Y+yPanPosH,r18
ph_3:	ret

StopOnLimits:
	sbrc	AFlags,afStopping
	ret
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
	ldd	r25,Y+yAfterFailPeriod
	lsl	r25
	inc	r25
	std	Y+yAfterFailPeriod,r25
	std	Y+yAfterFailCntH,r25
	std	Y+yAfterFailCntL,ZeroReg
soh_1:	ret

WestLimitSetup:
	sbrc	AFLags,afReverse
	ret

EastLimitSetup:
	sbrs	AFLags,afReverse
	ret

	ret

UpdateOffset:
	ldd	r16,Y+yHour
	ldi	r17,60
	mul	r17,r16
	movw	r17:r16,r1:r0
	ldd	r18,Y+yMinute
	add	r16,r18
	adc	r17,ZeroReg ; Minutes from 00:00
	ldi_w	r20,r19,12567
	rcall	M24x16	    ; r25:r24:r23 Converted to tics
	ldd	r16,Y+yPanPosL
	ldd	r17,Y+yPanPosM
	ldd	r18,Y+yPanPosH
	sub	r16,r23
	sbc	r17,r24
	sbc	r18,r25
	asr16	r18,r17
	ror	r16
	asr16	r18,r17
	ror	r16
	sts	iTimeOffsetL,r16
	sts	iTimeOffsetH,r17
	ret
