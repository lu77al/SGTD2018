TimeGo:	; RTC H:M:S
	ldd	r16,Y+ySecondCntL
	ldd	r17,Y+ySecondCntH
	subi_w	r17,r16,SECOND_FRACTION
	brcc	tg_1
	subi_w	r17,r16,SECOND_FRACTION
	ldd	r18,Y+ySecond
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
	ldi	r24,0
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
	mov	r18,r25
	mov	r17,r24
tt_1:	ldi	r16,0
	ldi_w	r20,r19,12567
	rcall	M24x16		; r25:r24:r23 Converted to tics
	rcall	OffsetAndFitRange
	std	Y+yTargetPosL,r23
	std	Y+yTargetPosM,r24
	std	Y+yTargetPosH,r25
	ret

CheckForParking: ; r17:r16 - input minutes from 00:00 / SREG.T and r18:r17 - output
	clt
	lds	r18,iMorningL
	lds	r19,iMorningH
	cp	r16,r18
	cpc	r17,r19
	brlo	cp_1
	lds	r18,iEveningL
	lds	r19,iEveningH
	cp	r18,r16
	cpc	r19,r17
	brlo	cp_1
	ret
cp_1:	set
	lds	r17,iParkingL
	lds	r18,iParkingH
	ret

OffsetAndFitRange: ; r25:r24:r23 - input/output position
	lds	r16,iTimeOffsetL
	lds	r17,iTimeOffsetH
	ldi	r18,0
	sbrc	r17,7
	ldi	r18,$FF
	lsl16	r17,r16		; Prepare offset (unpack 16bit)
	lsl16	r17,r16
	rol	r18
	add	r23,r16		; Apply offset
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
fr_3:	ldi_w	r17,r16,HALF_CIRCLE + HALL_OFFSET ; Fit low limit
	ldi	r18,0
	lds	r19,iHalfSpanL
	lds	r20,iHalfSpanH
	sub	r16,r19
	sbc	r17,r20
	cp	r23,r16
	cpc	r24,r17
	cpc	r25,r18
	brlo	fr_5
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
	brlo	fr_5
	ret
fr_5:	mov	r23,r16
	mov	r24,r17
	mov	r25,r18
	ret

UpdateTimeOffset:
	ldd	r16,Y+yHour
	ldi	r17,60
	mul	r17,r16
	mov	r18,r1
	mov	r17,r0
	ldi	r16,0
	ldd	r19,Y+yMinute
	add	r17,r19
	adc	r18,ZeroReg ; Minutes from 00:00
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
	set
	bld	AFlags,afSaveParams
	ret
