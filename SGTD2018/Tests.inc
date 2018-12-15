/*
StartSimplePWM:
	clt
	bld	AFlags,afReverse
	rcall	EnablePWM
;	ldi_w	r17,r16,PWMPeriod/4 ; Minimum phase
;	out	OCR1BH,r17
;	out	OCR1BL,r16
	stdi	yTargetPeriod,54
	ret
*/

/*
StartTest:
	ldi_w	r17,r16,3*60
	sts	iMorningL,r16
	sts	iMorningH,r17
	ldi_w	r17,r16,21*60
	sts	iEveningL,r16
	sts	iEveningH,r17
	ldi_w	r17,r16,12*60
	sts	iParkingL,r16
	sts	iParkingH,r17
	ldi_w	r17,r16,17673
	sts	iHalfSpanL,r16
	sts	iHalfSpanH,r17
	stdi	yHour,12
	rcall	TimeToTargetPos
	ldd	r16,Y+yTargetPosL
	ldd	r17,Y+yTargetPosM
	ldd	r18,Y+yTargetPosH
	std	Y+yPanPosL,r16
	std	Y+yPanPosM,r17
	std	Y+yPanPosH,r18
	ret
*/

/*
TestStop:
	ldd	r25,Y+yButPress
	mov	r16,r25
	andi	r16,$FF-(1<<btEastButton) - (1<<btWestButton)
	std	Y+yButPress,r16
	sbrc	r25,btEastButton
	rjmp	tst_1
	sbrs	r25,btWestButton
	rjmp	tst_2
	set
	bld	AFlags,afFastStop
tst_1:	rcall	StopDrive
	ldd	r25,Y+yAfterFailPeriod
	lsl	r25
	inc	r25
	std	Y+yAfterFailPeriod,r25
	std	Y+yAfterFailCntH,r25
	std	Y+yAfterFailCntL,ZeroReg
	set
	bld	AFlags,afFailStop
tst_2:	ret
*/

/*
TestSimpleMove:
	ldi_w	r17,r16,HALF_CIRCLE
	ldi	r18,0
	lds	r19,iHalfSpanL
	lds	r20,iHalfSpanH
	ldd	r25,Y+yButPress
	mov	r24,r25
	andi	r24,$FF-(1<<btEastButton) - (1<<btWestButton)
	std	Y+yButPress,r24
	sbrs	r25,btWestButton
	rjmp	tsm_1
	add	r16,r19
	adc	r17,r20
	adc	r18,ZeroReg
tsm_3:	std	Y+yStopPosL,r16
	std	Y+yStopPosM,r17
	std	Y+yStopPosH,r18
	set
	bld	AFlags,afSetup
	rjmp	DriveOn
tsm_1:
	sbrs	r25,btEastButton
	rjmp	tsm_2
	sub	r16,r19
	sbc	r17,r20
	sbc	r18,ZeroReg
	rjmp	tsm_3
tsm_2:
	ret	
*/		

/* Stop procedure variants
 - regular: (speed profile) + immediate stop
 - fast: set new target pos + double acceleration profile
 */

/*
TestKeySwitching:
	std	Y+yTargetPosL,ZeroReg
	std	Y+yTargetPosM,ZeroReg
	std	Y+yTargetPosH,ZeroReg
	set
	bld	AFlags,afSetupMove
	rcall	DriveOn
dbdg:	rjmp	dbdg

*/


TestCalcOffset:
	ldi_w	r17,r16,$78
	sts	iMorningL,r16
	sts	iMorningH,r17
	ldi_w	r17,r16,$528
	sts	iEveningL,r16
	sts	iEveningH,r17
	ldi_w	r17,r16,$7450
	std	Y+yPanPosL,r16
	std	Y+yPanPosM,r17
	stdi	yHour,$0C
	stdi	yMinute,$24
	rcall	UpdateTimeOffset
	rcall	TimeToTargetPos
	ldd	r16,Y+yPanPosL
	ldd	r17,Y+yPanPosM
	ldd	r18,Y+yPanPosH
	ldd	r19,Y+yTargetPosL
	ldd	r20,Y+yTargetPosM
	ldd	r21,Y+yTargetPosH


	ret


