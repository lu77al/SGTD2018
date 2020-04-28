TimeGo:	; RTC H:M:S
	ldd	r16,Y+ySecondCntL
	ldd	r17,Y+ySecondCntH
	subi_w	r17,r16,SECOND_FRACTION
	brcc	tg_1
	subi_w	r17,r16,-FULL_SECOND
	ldd	r18,Y+ySecond
	wdr
	inc	r18
	cpi	r18,60
	brlo	tg_2
	ldi	r18,0
	ldd	r19,Y+yMinute
	inc	r19
	cpi	r19,60
	brlo	tg_3
	ldi	r19,0
	ldd	r20,Y+yHour
	inc	r20
	cpi	r20,24
	brlo	tg_4
	ldi	r20,0
tg_4:	std	Y+yHour,r20
tg_3:	std	Y+yMinute,r19
tg_2:	std	Y+ySecond,r18
tg_1:	std	Y+ySecondCntH,r17
	std	Y+ySecondCntL,r16
	ret


TimeToTargetPos:
	ldd	r16,Y+yHour
	ldi	r17,60
	mul	r17,r16
	movw	r17:r16,r1:r0
	ldd	r18,Y+yMinute
	add	r16,r18
	adc	r17,ZeroReg ; Minutes from 00:00
	rcall	CheckForParking
	brts	tt_1
	ldi_w	r19,r18,2*MAX_POS_DIFF_MIN
	movw	r25:r24,r17:r16
	rcall	Div
	sub	r24,r14
	sbc	r25,r15
	subi_w	r25,r24,-MAX_POS_DIFF_MIN ; Minutes from 00:00 with step DIFF*2
	movw	r17:r16,r25:r24
tt_1:	mov	r18,r17
	mov	r17,r16
	ldi	r16,0
	ldi_w	r20,r19,12567
	rcall	M24x16		; r25:r24:r23 Converted to tics
	clt
	rcall	OffsetAndFitRange
	rcall	FitLimits
	std	Y+yTargetPosL,r23
	std	Y+yTargetPosM,r24
	std	Y+yTargetPosH,r25
	ret

CheckForParking: ; r17:r16 - input minutes from 00:00 / SREG.T and r17:r16 - output
	clt
	lds	r18,iMorningL
	lds	r19,iMorningH
	lds	r20,iEveningL
	lds	r21,iEveningH
	cp	r18,r20
	cpc	r19,r21
	breq	cp_1		; M = E -> normal
	brsh	cp_2
	;-- M < E --
	cp	r18,r16
	cpc	r19,r17
	brsh	cp_3		; M >= T -> parking
	cp	r16,r20
	cpc	r17,r21
	brsh	cp_3		; T >= E -> parking
	ret 
cp_2:	;-- M > E 
	cp	r16,r20
	cpc	r17,r21
	brlo	cp_1		; T < E -> normal
	cp	r18,r16
	cpc	r19,r17
	brsh	cp_3		; M >= T -> parking
cp_1:	ret
cp_3:	lds	r16,iParkingL
	lds	r17,iParkingH	
	set
	ret


OffsetAndFitRange: ; r25:r24:r23 - input/output position
	lds	r16,iTimeOffsetL
	lds	r17,iTimeOffsetM
	lds	r18,iTimeOffsetH
	brtc	fr_6
	sub	r23,r16		; Apply offset
	sbc	r24,r17
	sbc	r25,r18
	rjmp	fr_2
fr_6:	add	r23,r16		; Apply offset
	adc	r24,r17
	adc	r25,r18
fr_2:	sbrs	r25,7		; Fit low range
	rjmp	fr_1
	subi_w	r24,r23,-FULL_CIRCLE
	sbci	r25,BYTE3(-FULL_CIRCLE)
	rjmp	fr_2
fr_1:	ldi_w	r17,r16,FULL_CIRCLE   ; Fit high range
	ldi	r18,BYTE3(FULL_CIRCLE)
fr_4:	cp	r16,r23
	cpc	r17,r24
	cpc	r18,r25
	brsh	fr_3
	sub	r23,r16
	sbc	r24,r17
	sbc	r25,r18
	rjmp	fr_4
fr_3:	ret

FitLimits:
	ldi_w	r17,r16,HALF_CIRCLE + HALL_OFFSET ; Fit low limit
	ldi	r18,0
	lds	r19,iHalfSpanL
	lds	r20,iHalfSpanH
	sub	r16,r19
	sbc	r17,r20
	cp	r23,r16
	cpc	r24,r17
	cpc	r25,r18
	brlo	flim_1
	ldi_w	r17,r16,HALF_CIRCLE - HALL_OFFSET ; Fit high limit
	ldi	r18,0
	lds	r19,iHalfSpanL
	lds	r20,iHalfSpanH
	add	r16,r19
	adc	r17,r20
	adc	r18,ZeroReg
	cp	r16,r23
	cpc	r17,r24
	cpc	r18,r25
	brlo	flim_1
	ret
flim_1:	mov	r23,r16
	mov	r24,r17
	mov	r25,r18
	ret

UpdateTimeOffset:
	ldd	r16,Y+yHour
	ldi	r25,60
	mul	r16,r25
	movw	r21:r20,r1:r0
	ldd	r16,Y+yMinute
	add	r20,r16
	adc	r21,ZeroReg
	mul	r21,r25
	mov	r18,r1
	mov	r17,r0
	ldd	r16,Y+ySecond
	mul	r20,r25
	add	r16,r0
	adc	r17,r1
	adc	r18,ZeroReg
	ldi_w	r20,r19,53621
	rcall	M24x16	    ; r25:r24:r23 Converted to tics	
	ldd	r16,Y+yPanPosL
	ldd	r17,Y+yPanPosM
	ldd	r18,Y+yPanPosH
	sub	r16,r23
	sbc	r17,r24
	sbc	r18,r25
	sts	iTimeOffsetL,r16
	sts	iTimeOffsetM,r17
	sts	iTimeOffsetH,r18
	set
	bld	AFlags,afSaveParams
	ret

UpdateTimeFromPanPos:
	ldd	r23,Y+yPanPosL
	ldd	r24,Y+yPanPosM
	ldd	r25,Y+yPanPosH
	set
	rcall	OffsetAndFitRange
	mov	r16,r23
	mov	r17,r24
	mov	r18,r25
	ldi_w	r20,r19,40048
	rcall	M24x16
	ldi	r16,$FF
utf_1:	inc	r16
	subi_w	r24,r23,1800
	brcc	utf_1
	subi_w	r24,r23,-1800
	ldi	r17,$FF
utf_2:	inc	r17
	subi_w	r24,r23,30
	brcc	utf_2
	subi	r23,-30
	lsl	r23
	std	Y+yHour,r16
	std	Y+yMinute,r17
	std	Y+ySecond,r23
	ldi	XL,RTCBuf+10
	ldi	r16,$40
	st	X,r16
	ret

