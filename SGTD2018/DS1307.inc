; TWI statuses after successful operation (fait for)
.equ	TWI_S_START		= $08
.equ	TWI_S_REPSTART		= $10
.equ	TWI_S_SLA_W_ACK		= $18
.equ	TWI_S_DATA_W_ACK	= $28
.equ	TWI_S_SLA_R_ACK		= $40
.equ	TWI_S_DATA_R_NACK	= $58
.equ	TWI_S_DATA_R_ACK	= $50

; DS1307 addresses for write/read operation
.equ	RTC_ADDR_W	= 0b11010000
.equ	RTC_ADDR_R	= 0b11010001

; DS1307 buffer transaction length
.equ	RTC_BUFLEN = 12

Chat_RTC: ; r25 - RTC_stage
	cpi	r25,$FF ; *** Disable TWI after generating "stop condition"
	brne	crt_16
	in	r16,TWCR
	sbrc	r16,TWSTO
	ret
crt_10:	outi	TWCR,(1<<TWINT)
	std	Y+yRTCStage,ZeroReg
	std	Y+yRTCWaitStatus,ZeroReg
	ret

crt_16:	ldd	r17,Y+yRTCWaitStatus ; *** If wait action result
	cpi	r17,0
	breq	crt_1
	in	r16,TWCR	; Wait for INT
	sbrs	r16,TWINT	
	ret
	in	r16,TWSR	; Check status
	andi	r16,$F8
	cp	r16,r17
	brne	crt_10	; *** Error
crt_1:
	subi	r25,101	; *** Select Read(1...)/Write(101..) branch
	brcc	crt_2		; Convert it to 0..
	subi	r25,-100
crt_14:			; *** Read
	cpi	r25,0	; 0 - TWI START
	brne	crt_3
	ldi	r17,(1<<TWINT) | (1<<TWSTA) | (1<<TWEN)
	ldi	r18,TWI_S_START
	rjmp	crt_5
crt_3:
	cpi	r25,1	; 1 - TWI ADDR + W
	brne	crt_4
	ldi	r16,RTC_ADDR_W
	ldi	r17,(1<<TWINT) | (1<<TWEN)
	ldi	r18,TWI_S_SLA_W_ACK
crt_9:	out	TWDR,r16
crt_5:	out	TWCR,r17
	std	Y+yRTCWaitStatus,r18
	ldd	r16,Y+yRTCStage
	inc	r16
	std	Y+yRTCStage,r16
	ret
crt_4:
	cpi	r25,2	; 2 - Register address (0)
	brne	crt_6
	ldi	r16,0
crt_15:	ldi	r17,(1<<TWINT) | (1<<TWEN)
	ldi	r18,TWI_S_DATA_W_ACK
	rjmp	crt_9
crt_6:
	cpi	r25,3	; 3 - Repeated start
	brne	crt_7
	ldi	r17,(1<<TWINT) | (1<<TWSTA) | (1<<TWEN)
	ldi	r18,TWI_S_REPSTART
	rjmp	crt_5
crt_7:
	cpi	r25,4	; 4 - TWI ADDR + R
	brne	crt_8
	ldi	r16,RTC_ADDR_R
	ldi	r17,(1<<TWINT) | (1<<TWEN)
	ldi	r18,TWI_S_SLA_R_ACK
	rjmp	crt_9
crt_8:
	cpi	r25,RTC_BUFLEN + 6 ; 5..(5+RTC_BUFLEN) - Data  ( 5-00a, 6-01a, .. [RTC_BUFLEN+5]-STOP )
	brsh	crt_10	;  algorithm error -> Stop transactions
	cpi	r25,6
	brlo	crt_11
	mov	XL,r25	;  save byte to buffer (previous step result)
	subi	XL,(6-RTCBuf)
	in	r16,TWDR
	st	X,r16
crt_11:	cpi	r25,RTC_BUFLEN + 4 ; READ ACK
	brsh	crt_12
	ldi	r17,(1<<TWINT) | (1<<TWEN) | (1<<TWEA)
	ldi	r18,TWI_S_DATA_R_ACK
	rjmp	crt_5
crt_12:	brne	crt_13	; READ NOACK
	ldi	r17,(1<<TWINT) | (1<<TWEN)
	ldi	r18,TWI_S_DATA_R_NACK
	rjmp	crt_5
crt_13: rcall	CheckRTCBuf
	; STOP (and finish process)
crt_19:	outi	TWCR,(1<<TWINT) | (1<<TWSTO) | (1<<TWEN)
	stdi	yRTCStage,$FF
	ret

crt_17:	rjmp	crt_10	; Algorithm error (jump bridge)	

		; *** WriteRTC
crt_2:	cpi	r25,3	; First 3 stages (START, SLA_W, REG_ADDR) are same as ReadRTC
	brlo	crt_14
		; Data ( 3-00, 4-01, .. [RTC_BUFLEN+3]-STOP )
	cpi	r25,RTC_BUFLEN + 4
	brsh	crt_17	; Algorithm error
	cpi	r25,RTC_BUFLEN + 3
	breq	crt_19	; STOP
	mov	XL,r25	; Send byte
	subi	XL,(3-RTCBuf)
	ld	r16,X
	rjmp	crt_15


CheckForRTCWrite:
	ldi	XL,RTCBuf+8
	ld	r16,X+
	ld	r17,X+
	ld	r18,X+
	ldd	r19,Y+yPanPosL
	ldd	r20,Y+yPanPosM
	ldd	r21,Y+yPanPosH
	cp	r16,r19
	cpc	r17,r20
	cpc	r18,r21
	breq	csp_1
	rcall	StartRTCWrite
csp_1:	ret


StartRTCRead:
	outi	TWCR,(1<<TWINT)
	out	TWCR,ZeroReg
	stdi	yRTCStage,1
	std	Y+yRTCWaitStatus,ZeroReg
	ret

StartRTCWrite:
	rcall	FormRTCBuf
	outi	TWCR,(1<<TWINT)
	out	TWCR,ZeroReg
	stdi	yRTCStage,101
	std	Y+yRTCWaitStatus,ZeroReg
	ret

/*
 * 0  - Seconds (BCD)
 * 1  - Minutes (BCD)
 * 2  - Hours   (BCD)
 * 3  - Day of week  (set 1)
 * 4  - Date	     (set 1)
 * 5  - Month	     (set 1)
 * 6  - Year	     (set 0)
 * 7  - Control register (set 0)
 * 8  - PanelPosL
 * 9  - PanelPosM
 * 10 - PanelPosH
 * 11 - CheckSum + $A5 (0..6 7-th bits, 7..12 all bits)
 */

FormRTCBuf:
	ldi	XL,RTCBuf
	ldd	r16,Y+ySecond
	rcall	ConvertToBCD
	st	X+,r16
	andi	r16,$80
	ldd	r16,Y+yMinute
	rcall	ConvertToBCD
	st	X+,r16
	andi	r16,$80
	ldd	r16,Y+yHour
	rcall	ConvertToBCD
	st	X+,r16
	andi	r16,$80
	ldi	r16,1
	st	X+,r16
	st	X+,r16
	st	X+,r16
	st	X+,ZeroReg
	st	X+,ZeroReg
	ldi	r23,$A5
	ldd	r16,Y+yPanPosL
	st	X+,r16
	rcall	AddByteCS
	ldd	r16,Y+yPanPosM
	st	X+,r16
	rcall	AddByteCS
	ldd	r16,Y+yPanPosH
	st	X+,r16
	rcall	AddByteCS
	st	X+,r23
	ret

CheckRTCBuf:
	ldi	XL,RTCBuf
crtc_2:	ld	r16,X+
	andi	r16,$80
	brne	crtc_1
	cpi	XL,RTCBuf+7
	brlo	crtc_2
crtc_1:	ld	r16,X+
	cpi	r16,0
	brne	crtc_3
	ldi	r23,$A5
	ld	r17,X+
	mov	r16,r17
	rcall	AddByteCS
	ld	r18,X+
	mov	r16,r18
	rcall	AddByteCS
	ld	r19,X+
	mov	r16,r19
	rcall	AddByteCS
	ld	r20,X+
	cp	r20,r23
	brne	crtc_3
	std	Y+yPanPosL,r17
	std	Y+yPanPosM,r18
	std	Y+yPanPosH,r19
	ldi	XL,RTCBuf
	ld	r16,X+
	rcall	DecodeBCD
	std	Y+ySecond,r16
	ld	r16,X+
	rcall	DecodeBCD
	std	Y+yMinute,r16
	ld	r16,X+
	rcall	DecodeBCD
	std	Y+yHour,r16
	sts	iInitCnt,ZeroReg
crtc_3:	ret

AddByteCS:
	lsl	r23
	brcc	abc_1
	ori	r23,1
abc_1:	add	r23,r16
	brcc	abc_2
	inc	r23
abc_2:	ret

ConvertToBCD: ; r16 -> r16 
	ldi	r17,-$10
tbcd_1:	subi	r17,-$10
	subi	r16,10
	brcc	tbcd_1
	subi	r16,-10
	or	r16,r17
	ret

DecodeBCD: ; r16 -> r16
	mov	r17,r16
	andi	r16,$0F
	swap	r17
	andi	r17,$0F
	ldi	r18,10
	mul	r17,r18
	add	r16,r0
	ret


