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

ProcessLimits:
	ldd	r25,Y+yButPress
	mov	r16,r25
	andi	r16,$FF - (1<<btEastLimit) - (1<<btWestLimit)
	std	Y+yButPress,r16
	sbrs	r25,btEastLimit	; East limit trigger
	rjmp	ph_1
	ldi_w	r17,r16,HALF_CIRCLE - HALL_OFFSET
	ldi	r18,0
	lds	r19,iHalfSpanL
	lds	r20,iHalfSpanH
	sub	r16,r19
	sbc	r17,r20
	rjmp	ph_2
ph_1:	sbrs	r25,btWestLimit	; West limit trigger
	rjmp	ph_3
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
ph_3:
	ret
