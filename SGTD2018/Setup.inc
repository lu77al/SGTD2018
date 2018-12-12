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

SetupDelayCMD:
	std	Y+yLimSetupStage,ZeroReg
	set
	sbrc	AFlags,afActive
	bld	AFlags,afSetupMove
	ldi_w	r17,r16,6866
	std	Y+yDelayCntL,r16
	std	Y+yDelayCntH,r17
	std	Y+yNextDelay,ZeroReg
	ret

StopCMD:
	std	Y+yLimSetupStage,ZeroReg
	sbrc	AFlags,afSetupMove
	rcall	UpdateTimeOffset
	rcall	StopDrive
	set
	bld	AFlags,afSetupMove
	std	Y+yNextDelay,ZeroReg
	std	Y+yDelayCntH,ZeroReg
	stdi	yDelayCntL,255
	ret

MoveEastCMD:
	ldd	r16,Y+yButState
	sbrc	r16,btEastLimit
	ret	
	ldi_w	r17,r16,0
	ldi	r18,0
	rjmp	smv_1


MoveWestCMD:
	ldd	r16,Y+yButState
	sbrc	r16,btWestLimit
	ret	
	ldi_w	r17,r16,FULL_CIRCLE
	ldi	r18,BYTE3(FULL_CIRCLE)
smv_1:	stdi	yLimSetupStage,2
smv_2:	ldi	r19,25
smv_3:	std	Y+yDelayCntL,r19
	std	Y+yNextDelay,ZeroReg
	std	Y+yDelayCntH,ZeroReg
	set
	bld	AFlags,afSetupMove
	std	Y+yStopPosL,r16
	std	Y+yStopPosM,r17
	std	Y+yStopPosH,r18
	rjmp	DriveOn


ContinueLimitSetup:
	std	Y+yLimSetupStage,r16
	cpi	r16,1
	brne	cls_1
	sbrs	AFlags,afReverse
	rjmp	cls_2
	ldi_w	r17,r16,FULL_CIRCLE
	ldi	r18,BYTE3(FULL_CIRCLE)
	rjmp	smv_2
cls_2:	ldi_w	r17,r16,0
	ldi	r18,0
	rjmp	smv_2
cls_1:	std	Y+yLimSetupStage,ZeroReg
	ldi_w	r17,r16,HALF_CIRCLE
	ldi	r18,BYTE3(HALF_CIRCLE)
	ldi	r19,255
	rjmp	smv_3


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
	sbrs	AFLags,afActive	  ; If drive is active -> only process press for stop
	rjmp	pb_4
	ldd	r23,Y+yButPress
	andi	r23,(1<<btEastButton) + (1<<btWestButton)
	breq	pb_3
	rcall	StopCMD
	rjmp	pb_6
pb_4:
	ldd	r23,Y+yButPress  ; Press when not active
	andi	r23,(1<<btEastButton) + (1<<btWestButton)
	breq	pb_8
	ldi	r16,(1<<btEastButton) + (1<<btWestButton)
	and	r16,r24
	cpi	r16,(1<<btEastButton) + (1<<btWestButton) ; And both buttons are pressed
	brne	pb_8
	rcall	SetupDelayCMD
	rjmp	pb_6
pb_8:
	cpi	r25,40		; Long ...
	brlo	pb_5
	sbrs	r24,btWestButton  ; Press west
	rjmp	pb_7
	rcall	MoveWestCMD		; -> Move west
	rjmp	pb_6
pb_7:
	sbrs	r24,btEastButton  ; Press east
	rjmp	pb_5
	rcall	MoveEastCMD		; -> Move east
	rjmp	pb_6
pb_5:
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


; West limit sensor "pressed" during setup move
;  - update halfspan (HS = PP - HC + HS) / 2 
;  - limit ranges
;  - update PP
;  - update time offset
;     HS - HalfSpan; PP - PanelPos; HC - HalfCircle   

WestLimitSetup:
	sbrc	AFLags,afReverse ; Skip wrong direction
	ret
	ldd	r16,Y+yPanPosL	
	ldd	r17,Y+yPanPosM
	ldd	r18,Y+yPanPosH
	subi	r16,LOW(HALF_CIRCLE)
	sbci	r17,HIGH(HALF_CIRCLE)
	sbci	r18,BYTE3(HALF_CIRCLE)
ls_3:	lds	r19,iHalfSpanL
	lds	r20,iHalfSpanH
	add	r16,r19
	adc	r17,r20
	adc	r18,ZeroReg
	ldi_w	r20,r19,MIN_SPAN
	ldi	r21,BYTE3(MIN_SPAN)
	cp	r16,r19
	cpc	r17,r20
	cpc	r18,r21
	brlt	ls_1
	ldi_w	r20,r19,MAX_SPAN
	ldi	r21,BYTE3(MAX_SPAN)
	cp	r16,r19
	cpc	r17,r20
	cpc	r18,r21
	brlt	ls_2
ls_1:	mov	r16,r19
	mov	r17,r20
	mov	r18,r21
ls_2:	lsr16	r18,r17
	ror	r16
	sts	iHalfSpanL,r16
	sts	iHalfSpanH,r17
	set
	bld	AFlags,afSaveParams
	ret

; West limit sensor "pressed" during setup move
;  (HS = HC - PP + HS) / 2 
;  - limit ranges
EastLimitSetup:
	sbrs	AFLags,afReverse ; Skip wrong direction
	ret
	ldi_w	r17,r16,HALF_CIRCLE
	ldi	r18,BYTE3(HALF_CIRCLE)
	ldd	r19,Y+yPanPosL	
	ldd	r20,Y+yPanPosM
	ldd	r21,Y+yPanPosH
	sub	r16,r19
	sbc	r17,r20
	sbc	r18,r21
	rjmp	ls_3
