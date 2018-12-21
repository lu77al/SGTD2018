SaveParams:
	ldi_w	r25,r24,0
	ldi	r23,$A5
	lds	r16,iHalfSpanL
	rcall	EEWrite
	rcall	AddByteCS
	lds	r16,iHalfSpanH
	rcall	EEWrite
	rcall	AddByteCS
	lds	r16,iTimeOffsetL
	rcall	EEWrite
	rcall	AddByteCS
	lds	r16,iTimeOffsetM
	rcall	EEWrite
	rcall	AddByteCS
	lds	r16,iTimeOffsetH
	rcall	EEWrite
	rcall	AddByteCS
	lds	r16,iMorningL
	rcall	EEWrite
	rcall	AddByteCS
	lds	r16,iMorningH
	rcall	EEWrite
	rcall	AddByteCS
	lds	r16,iEveningL
	rcall	EEWrite
	rcall	AddByteCS
	lds	r16,iEveningH
	rcall	EEWrite
	rcall	AddByteCS
	lds	r16,iParkingL
	rcall	EEWrite
	rcall	AddByteCS
	lds	r16,iParkingH
	rcall	EEWrite
	rcall	AddByteCS
	mov	r16,r23
	rcall	EEWrite
	ret

ReadParams:
	ldi_w	r25,r24,0
	ldi	r23,$A5
	rcall	EERead
	sts	iHalfSpanL,r16
	rcall	AddByteCS
	rcall	EERead
	sts	iHalfSpanH,r16
	rcall	AddByteCS
	rcall	EERead
	sts	iTimeOffsetL,r16
	rcall	AddByteCS
	rcall	EERead
	sts	iTimeOffsetM,r16
	rcall	AddByteCS
	rcall	EERead
	sts	iTimeOffsetH,r16
	rcall	AddByteCS
	rcall	EERead
	sts	iMorningL,r16
	rcall	AddByteCS
	rcall	EERead
	sts	iMorningH,r16
	rcall	AddByteCS
	rcall	EERead
	sts	iEveningL,r16
	rcall	AddByteCS
	rcall	EERead
	sts	iEveningH,r16
	rcall	AddByteCS
	rcall	EERead
	sts	iParkingL,r16
	rcall	AddByteCS
	rcall	EERead
	sts	iParkingH,r16
	rcall	AddByteCS
	rcall	EERead
	cp	r16,r23
	brne	DefaultParams
	ret

DefaultParams:
	ldi_w	r17,r16,2*60
	sts	iMorningL,r16
	sts	iMorningH,r17
	ldi_w	r17,r16,22*60
	sts	iEveningL,r16
	sts	iEveningH,r17
	ldi_w	r17,r16,2*60
	sts	iParkingL,r16
	sts	iParkingH,r17
	ldi_w	r17,r16,70693/24*5  
	sts	iHalfSpanL,r16
	sts	iHalfSpanH,r17
	sts	iTimeOffsetL,ZeroReg
	sts	iTimeOffsetM,ZeroReg
	sts	iTimeOffsetH,ZeroReg
	rjmp	SaveParams

CheckSaveParam:
	sbrs	AFlags,afSaveParams
	ret
	clt
	bld	AFlags,afSaveParams
	rjmp	SaveParams
