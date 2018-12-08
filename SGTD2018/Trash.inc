/* Buttons interface V1
 *   Enter setup - long press both
 *   Exit setup  - press both
 *   Start move (with offset correction) - release
 *   Stop move  - press
 *   Start limits search - long press
 */

/* Buttons interface V2
 *   Release - Start move + enter setup
 *   Long press - Start limit search + enter setup
 *   Press when moving - Stop and enter setup
 *   Press both - exit setup 
 *     - wait for long release after each command 
 *     - automatic countdown for exit setup
 */

/* Buttons interface V3
 *   Long press - Start move
 *   Press when moving - Stop + delay
 */

/* Setup Commands:
 *   - stop move / enter setup / update offset
 *   - start move
 *   - start search
 *   - exit setup
 *   - set parameters (later)
 */

StopCMD:
	sbrc	AFlags,afSetup
	rcall	UpdateOffset
	rcall	StopDrive
	set
	bld	AFlags,afSetup
	ret

ExitSetupCMD:
	clt
	bld	AFlags,afSetup
	ret

MoveEastCMD:
	ldi_w	r17,r16,0
	ldi	r18,0
	rjmp	smv_1


MoveWestCMD:
	ldi_w	r17,r16,FULL_CIRCLE
	ldi	r18,BYTE2(FULL_CIRCLE)
smv_1:	set
	bld	AFlags,afSetup
	std	Y+yStopPosL,r16
	std	Y+yStopPosM,r17
	std	Y+yStopPosH,r18
	rjmp	DriveOn


SearchEastCMD:
	ret

SearchWestCMD:
	ret


FindLimits:	;1-0 - Exit; 2-1 - Move to midday; 3-2 - Move second side; 4-3 - Move first side
	sbrc	AFlags,afActive
fl_1:	ret
	ldd	r16,Y+yInitPhase
	subi	r16,1
	brcs	fl_1
	std	Y+yInitPhase,r16
	cpi	r16,4
	brsh	fl_1
	cpi	r16,2
	brlo	fl_2
	ldi	r16,1<<afReverse
	eor	AFlags,r16
	sbrc	AFlags,afReverse
	rjmp	fl_3
	ldi_w	r17,r16,1000
	ldi	r18,0
	ldi_w	r20,r19,FULL_CIRCLE - 1000
	ldi	r21,BYTE3(FULL_CIRCLE - 1000)
	rjmp	fl_4
fl_3:	ldi_w	r20,r19,1000
	ldi	r21,0
	ldi_w	r17,r16,FULL_CIRCLE - 1000
	ldi	r18,BYTE3(FULL_CIRCLE - 1000)
fl_4:	std	Y+yPanPosL,r16
	std	Y+yPanPosM,r17
	std	Y+yPanPosH,r18
fl_6:	std	Y+yStopPosL,r19
	std	Y+yStopPosM,r20
	std	Y+yStopPosH,r21
	rjmp	DriveOn
fl_2:	cpi	r16,1
	brne	fl_5
	ldi_w	r20,r19,HALF_CIRCLE
	ldi	r21,BYTE3(HALF_CIRCLE)
	rjmp	fl_6
fl_5:	
	ret



	


ProcessButtons:
	ldd	r24,Y+yButState
	andi	r24,(1<<btEastButton) + (1<<btWestButton)
	ldd	r25,Y+yButStateCnt ; Stable state counter
	inc	r25
	brne	pb_1
	ldi	r25,0
pb_1:	std	Y+yButStateCnt,r25
	sbrs	AFlags,afWaitButRel ; Wait for buttons released
	rjmp	pb_2
	cpi	r24,0
	brne	pb_3
	cpi	r25,7
	brlo	pb_3
	clt
	bld	AFlags,afWaitButRel
pb_2:
	ldd	r23,Y+yButPress
	ldd	r22,Y+yButRelease
	sbrs	AFLags,afActive	  ; If drive is active -> only process press for stop
	rjmp	pb_4
	; TODO add branch for search mode active
	andi	r23,(1<<btEastButton) + (1<<btWestButton)
	breq	pb_3
	rcall	StopCMD
	rjmp	pb_6
pb_4:
	andi	r23,(1<<btEastButton) + (1<<btWestButton) ; Exit setup (both pressed)
	breq	pb_5
	cpi	r24,(1<<btEastButton) + (1<<btWestButton)
	brne	pb_5
	rcall	ExitSetupCMD
	rjmp	pb_6
pb_5:
	sbrs	r22,btWestButton
	rjmp	pb_7
	rcall	MoveWestCMD
	rjmp	pb_6
pb_7:
	sbrs	r22,btEastButton
	rjmp	pb_8
	rcall	MoveEastCMD
	rjmp	pb_6
pb_8:
	cpi	r25,40
	brlo	pb_3

	sbrs	r24,btWestButton
	rjmp	pb_9
	rcall	SearchWestCMD
	rjmp	pb_6
pb_9:
	sbrs	r24,btEastButton
	rjmp	pb_10
	rcall	SearchEastCMD
	rjmp	pb_6
pb_10:
	rjmp	pb_3
pb_6:	set
	bld	AFlags,afWaitButRel
pb_3:	ldd	r16,Y+yButPress		; Clear flags after processing
	andi	r16,$ff - (1<<btEastButton) - (1<<btWestButton)
	std	Y+yButPress,r16
	ldd	r16,Y+yButRelease
	andi	r16,$ff - (1<<btEastButton) - (1<<btWestButton)
	std	Y+yButRelease,r16
	ret


WestLimitSetup:
	sbrc	AFLags,afReverse
	ret
	ldd	r16,Y+yInitPhase
	cpi	r16,2
	brne	ls_1
	ldd	r16,Y+yPanPosL
	ldd	r17,Y+yPanPosM
	ldd	r18,Y+yPanPosH
	ldi_w	r20,r19,1000
	ldi	r21,0
	rjmp	ls_3
ls_1:	ret


EastLimitSetup:
	sbrs	AFLags,afReverse
	ret
ls_1:
	ldd	r16,Y+yInitPhase
	cpi	r16,2
	brne	ls_2
	ldi_w	r17,r16,FULL_CIRCLE - 1000
	ldi	r18,BYTE3(FULL_CIRCLE - 1000)
	ldd	r19,Y+yPanPosL
	ldd	r20,Y+yPanPosM
	ldd	r21,Y+yPanPosH
ls_3:	sub	r16,r19
	sbc	r17,r20
	sbc	r18,r21
	brcs	ls_4
	ldi_w	r20,r19,MIN_SPAN
	ldi	r21,BYTE3(MIN_SPAN)
	cp	r16,r19
	cpc	r17,r20
	cpc	r18,r21
	brsh	ls_5
ls_4:	ldi_w	r17,r16,MIN_SPAN
	ldi	r18,BYTE3(MIN_SPAN)
	rjmp	ls_6
ls_5:	ldi_w	r20,r19,MAX_SPAN
	ldi	r21,BYTE3(MAX_SPAN)
	cp	r16,r19
	cpc	r17,r20
	cpc	r18,r21
	brlo	ls_6
	ldi_w	r17,r16,MAX_SPAN
	ldi	r18,BYTE3(MAX_SPAN)
ls_6:	lsr16	r18,r17
	ror	r16
	sts	iHalfSpanL,r16
	sts	iHalfSpanH,r17
	ldi_w	r20,r19,HALF_CIRCLE
	ldi	r21,BYTE3(HALF_CIRCLE)

	
	.equ	iHalfSpanH	= $A1
	


	.equ	MIN_SPAN = FULL_CIRCLE / 24 * 5
.equ	MAX_SPAN = FULL_CIRCLE / 24 * 15



	cp


ls_2:
	ret



/*
	ldd	r16,Y+yPanPosL
	ldd	r17,Y+yPanPosM
	ldi_w	ZH,ZL,20*36
	sub	ZL,r16
	sbc	ZH,r17
	brcc	tb_1
	rjmp	DisablePWM
tb_1:	lsr16	ZH,ZL
	cpi	ZL,85
	cpc	ZH,ZeroReg
	brlo	tb_2
	ldi_w	ZH,ZL,85
tb_2:	subi_w	ZH,ZL,-2*BreakTable
	lpm	r16,Z
	std	Y+yTargetPeriod,r16

*/

	ldi	XL,Y+yTimePointer
	subi	XL,2
	brcc	tr_5
	ldi	XL,10
tr_5:	std	Y+yTimePointer,XL
	ld	r16,X+
	ld	r17,X
	st	X,r24
	st	-X,r23
	sub	r16,r23
	sbc	r17,r24
	std	Y+yPeriodL,r16
	std	Y+yPeriodH,r17


/*** Start sinchronous meandr on all channels ***/
/*
StartKeysTest:
	rcall	EnablePWM
	ldi_w	r17,r16,PWMPeriod/2
	out	OCR1BH,r17
	out	OCR1BL,r16
	ldi	r16,0b101010	; High
	ldi	r17,0b010101	; Low
	ldi	XL,KS_Buf
	cli
	st	X+,ZeroReg	; Dead state
	st	X+,r16		; High
	st	X+,r17		; Low
	sei
	ret
*/

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
	out	PORTB,KS_Dead
	lds	KS_Dead,XSEG+KS_Buf+0
	lds	KS_High,XSEG+KS_Buf+1
	lds	SSReg, XSEG+KS_Buf+2
	out	PORTB,SSREG
	reti


	sbrs	BFlags,bfNoGrid
	rjmp	a1_2
	set
;	bld	

a1_2:


	ret	
a1_1:	cpi	r25,ssInv
	brne	a2_1
	ret	
a2_1:	cpi	r25,ssMainReley
	brne	a3_1
	ret	
a3_1:	cpi	r25,ssAddReley
	brne	a4_1
	ret	
a4_1:	cpi	r25,ssBothReleys
	brne	a5_1
	ret	
a5_1:	cpi	r25,ssInvAddReley
	brne	a6_1
	ret	
a6_1:	cpi	r25,ssFail
	brne	a7_1
	ret	
a7_1:



	ldi	ZH,1
	in	r24,ADCL
	in	r25,ADCH
	in	r23,ADMUX
	andi	r23,7
	cpi	r23,chUgrd
	brne	a_1
	ldi	ZL,GRDBuf
	ldd	r16,Y+yUzeroL	; "0"
	ldd	r17,Y+yUzeroH
	sub	r24,r16		; U rel "0"
	sbc	r25,r17
	asr16	r25,r24
	asr16	r25,r24		; reduce to 8 bit
	sbrc	r25,7
	neg	r24		; absolute
	ldd	r18,Z+siMaxAbs
	cp	r18,r24		; check max 
	brsh	a_2
	std	Z+siMaxAbs,r18
a_2:	ldd	r16,Z+siSideCnt		; side counter
	bst	r25,7
	bld	r17,0
	eor	r17,r16
	andi	r17,1
	breq	a_3
	subi	r16,2
	brcc	a_3
	std	Z+siMaxAbs,ZeroReg
	com
	sbrs	r16,0
	rjmp	a_4
	std	Z+siMaxNeg,r18
	rjmp	a_3
a_4:	std	Z+siMaxAbs,r18
	ldd	r17,Z+siLastChS
	ldd	r18,Y+yMainCntL
	std	Z+siLastChS,r18
	sub	r18,r17
	std	Z+siPeriod,r18






a_3:	andi	r16,1
	ori	r16,2*2
	std	Z+siSideCnt,r16


	com	r24
	com	r25
	subi_w	r25:r24,-1

	sbrc	r16,0
	rjmp	a_3
	andi	r16,1
	ori	r16,2*2
	rjmp	a_4
a_3:	subi	r16,2
	brcc	a_4
	com	r16
	andi	r16,1
	ori	r16,2*2
a_4:	

a_1:

	ld	r16,X+	; SideCnt
	ld	r17,X+	; AmplVar

	in	SSREG,SREG
	push	XL
	push	r16
	in	XL,ADMUX
	inc	XL
	andi	XL,0b11
	mov	r16,XL
	ori	r16,0b11000000
	out	ADMUX,r16
	sbi	ADCSRA,ADSC
	lsl	XL
	subi	XL,-DADCBuf
	in	r16,ADCL
	st	X+,r16
	in	r16,ADCH
	st	X,r16
	pop	r16
	pop	XL
	out	SREG,SSREG
	reti







MainAlgo: ; 7.5
	ldd	r25,Y+yUnBrakeCnt
	ldd	r24,Y+yBrakeDelCnt
	ldd	r23,Y+yReleyDelCnt
	ldd	r22,Y+yMainCntM
	sbrc	r22,0
	rjmp	mac_1
	ldd	r16,Y+yMinFrCnt
	subi	r16,1
	brcc	mac_2
	subi	r25,1
	brcc	mac_3
	ldi	r25,0
mac_3:	std	Y+yUnBrakeCnt,r25
	ldi	r16,228
mac_2:	std	Y+yMinFrCnt,r16
mac_1:
	andi	r22,0b111
	brne	mac_4
	subi	r24,1
	brcc	mac_5
	ldi	r24,0
mac_5:	std	Y+yBrakeDelCnt,r24
	subi	r23,1
	brcc	mac_6
	ldi	r23,0
mac_6:	std	Y+yReleyDelCnt,r23
mac_4:
	ldd	r22,Y+yButState
	lds	r20,CurNL
	lds	r21,CurNH
	ldd	r18,Y+yCurVoltL
	ldd	r19,Y+yCurVoltH

; **** Управлене приводом ****
	ldd	r16,Y+yFlugStage ; флюгирование при работе привода
	cpi	r16,$FF
	brne	ma_1
	cpi	r24,0  ; Не обрабатывать если не окончилась задержка
	brne	ma_1

; Флюгировать по оборотам
	ldd	r14,Y+yBrakeSpeedL
	ldd	r15,Y+yBrakeSpeedH
	ldi_w	r17,r16,351	; Выйти если отключено в меню
	cp	r14,r16
	cpc	r15,r17
	breq	ma_2
	cp	r20,r14		; Выйти если не превышены обороты
	cpc	r21,r15
	brlo	ma_2
ma_4:	set
	bld	BFlags,bfAutoBrake
	stdi	yFlugStage,7	; Флюгировать
	std	Y+yMinFrCnt,ZeroReg ; Взвести счетчики задержек
	ldd	r16,Y+yUnBrakeDelay
	std	Y+yUnBrakeCnt,r16
	stdi	yMinFrCnt,228
ma_6:	stdi	yBrakeDelCnt,171
	ret
ma_2:
; Флюгировать по напряжению
	ldd	r14,Y+yBrakeVoltL
	ldd	r15,Y+yBrakeVoltH
	ldi_w	r17,r16,755	; Выйти если отключено в меню
	cp	r14,r16
	cpc	r15,r17
	breq	ma_3
	cp	r18,r14		; Выйти если не превышены обороты
	cpc	r19,r15
	brsh	ma_4 ; Флюгировать
ma_3:
; Расфлюгировать по времени
	sbrs	BFlags,bfAutoBrake
	rjmp	ma_5
	ldd	r16,Y+yUnBrakeDelay ; Выйти если отключено в меню
	cpi	r16,61
	breq	ma_5
	sbrc	AFlags,afBrakeState ; Выйти если расфлюгировано
	sbrc	r22,btFluger ; Выйти если опущен тумблер
	rjmp	ma_5
	cpi	r25,0 ; Выйти если не прошло время
	brne	ma_5
	std	Y+yFlugStage,ZeroReg ; расфлюгировать
	ret
ma_5:
ma_1:
; **** Управлене реле ****
	cpi	r23,0  ; Не обрабатывать если не окончилась задержка
	brne	ma_7

; Отключить выход по оборотам
	sbrs	AFlags,afReleyState
	rjmp	ma_12
	ldd	r14,Y+yOffSpeedL
	ldd	r15,Y+yOffSpeedH
	ldi_w	r17,r16,351	; Выйти если отключено в меню
	cp	r14,r16
	cpc	r15,r17
	breq	ma_8
	cp	r20,r14		; Выйти если не превышены обороты
	cpc	r21,r15
	brlo	ma_8
	clt
	bld	AFlags,afReleyState
	set
	bld	BFlags,bfSpeedOff
	stdi	yReleyDelCnt,171
	ret
ma_8:
	sbrs	r22,btPhases	; Не включать выход при опущенном тумблере
	rjmp	ma_12
	clt
	bld	AFlags,afReleyState
	ret
ma_12:

; Подключить выход по оборотам
	sbrs	BFlags,bfSpeedOff
	rjmp	ma_9
	ldd	r14,Y+yOnSpeedL
	ldd	r15,Y+yOnSpeedH
	ldi_w	r17,r16,49	; Выйти если отключено в меню
	cp	r14,r16
	cpc	r15,r17
	breq	ma_9
	cp	r20,r14		; Выйти если обороты высокие
	cpc	r21,r15
	brsh	ma_9
	clt
	bld	BFlags,bfSpeedOff
	sbrc	BFlags,bfVoltageOff
	rjmp	ma_9		; Выйти если отключено по напряжению
ma_11:	set
	bld	AFlags,afReleyState
	stdi	yReleyDelCnt,7
	ret
ma_9:

; Подключить выход по напряжению
	sbrs	BFlags,bfVoltageOff
	rjmp	ma_10
	ldd	r14,Y+yOnVoltL
	ldd	r15,Y+yOnVoltH
	ldi_w	r17,r16,45	; Выйти если отключено в меню
	cp	r14,r16
	cpc	r15,r17
	breq	ma_10
	cp	r18,r14		; Выйти если напряжение высокое
	cpc	r19,r15
	brsh	ma_10
	clt
	bld	BFlags,bfVoltageOff
	sbrc	BFlags,bfSpeedOff
	rjmp	ma_10		; Выйти если отключено по оборотам
	rjmp	ma_11
ma_10:
ma_7:
	ret

CalcSpeed:
;	ldd	r16,Y+yTestN
;	ldi	r17,0
;	lsl16	r17,r16
;	rjmp	csp_d

	ldi	XL,PHTimeBuf
	ldi_w	r17,r16,0
csp_2:	ld	r18,X+
;	ldi	r18,$34
	cpi	r18,0
	breq	csp_1
	add	r16,r18
	adc	r17,ZeroReg
	cpi	XL,PHTimeBuf+20
	brlo	csp_2
csp_3:	sts	RPeriodL,r16
	sts	RPeriodH,r17
	movw	r19:r18,r17:r16
;	lsr16	r19,r18
;	lsr16	r19,r18
	ldi_w	r17,r16,58593
	rcall	Div
csp_d:	sts	CurNL,r16
	sts	CurNH,r17
	ret	
csp_1:	sts	RPeriodL,ZeroReg
	sts	RPeriodH,ZeroReg
	sts	CurNL,ZeroReg
	sts	CurNH,ZeroReg
	ret

; Снять питание  0.5c
; Полное питание 0.4с
; Снизить напряжение 0.2с
; Полное питание 0.2с
; Снизить напряжение 0.2с
; Полное питание 20с
;  ожидаем/считаем "гудки" через 2 сек после последнего обесточиваем

;
;PWM,Time,PWM,Time,
;.db	
AlgoTab:
.db	$20,42, $00,24, $02,12, $00,12, $02,12, $00,122, $00,$FF ; 0
.db	$20,42, $00,24, $02,12, $00,146, $00,$FF	         ; 7

FlugPWR:
	ldd	r16,Y+yFlugStage
	cpi	r16,$FF
	brne	fp_1
	stdi	yFPWMSize,$20
	std	Y+yFlugDelay,ZeroReg
	ret
fp_1:	cpi	r16,60
	brlo	fp_3
	ldd	r18,Y+yFlugDelay
	ldd	r16,Y+yACCnt
	lds	r17,BakACCnt
	sts	BakACCnt,r16
	cp	r17,r16
	breq	fp_5
	ldi	r18,6
fp_5:	ldd	r16,Y+yMainCntL
	andi	r16,0b11100000
	brne	fp_6
	subi	r18,1
	brcc	fp_6
	stdi	yFlugStage,$FF
	ldd	r16,Y+yACCnt
	cpi	r16,1
	brne	fp_7
	set
	bld	AFlags,afBrakeState
	rjmp	fp_8
fp_7:	cpi	r16,2
	brne	fp_9
	clt
	bld	AFlags,afBrakeState
	set
	rjmp	fp_8
fp_9:	clt
fp_8:	bld	AFlags,afBrakeOk
	ret
fp_6:	std	Y+yFlugDelay,r18
	ret
fp_3:	ldd	r17,Y+yFlugDelay
	subi	r17,1
	brcs	fp_4
	std	Y+yFlugDelay,r17
	ret
fp_4:	cpi	r16,0
	brne	fp_10
	clt
	rjmp	fp_12
fp_10:	cpi	r16,7
	brne	fp_11
	set
fp_12:	bld	BFlags,bfBraking
	bld	AFlags,afBrakeState
fp_11:	mov	r17,r16
	lsl	r17
	inc	r16
	std	Y+yFlugStage,r16
	ldi_w	ZH,ZL,2*AlgoTab
	add	ZL,r17
	adc	ZH,ZeroReg
	lpm	r16,Z+
	std	Y+yFPWMSize,r16
	lpm	r16,Z+
	cpi	r16,$FF
	breq	fp_2
	std	Y+yFlugDelay,r16
	ret
fp_2:	stdi	yFlugStage,70
	std	Y+yACCnt,ZeroReg
	stdi	yFlugDelay,137
	sts	BakACCnt,ZeroReg	
	ret
