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

/* Setup Commands:
 *   - stop move / enter setup / update offset
 *   - start move
 *   - start search
 *   - exit setup
 *   - set parameters (later)
 */

StopCMD:
	rcall	StopDrive
	ret

ExitSetupCMD:
	clt
	bld	AFlags,afSetup
	ret

MoveEastCMD:
	ldi_w	r17,r16,0
	ldi	r18,0
	rjmp	smv_1
	ret

MoveWestCMD:
	ldi_w	r17,r16,FULL_CIRCLE
	ldi	r18,BYTE2(FULL_CIRCLE)
smv_1:	set
	bld	AFlags,afSetup
	std	Y+yStopPosL,r16
	std	Y+yStopPosM,r17
	std	Y+yStopPosH,r18
	rjmp	DriveOn
	ret

SearchEastCMD:
	ret

SearchWestCMD:
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

EastLimitSetup:
	sbrs	AFLags,afReverse
	ret

	ret
