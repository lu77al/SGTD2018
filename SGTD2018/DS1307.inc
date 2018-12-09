
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
.equ	RTC_BUFLEN = 23

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


CheckRTCBuf:
	ret


StartRTCRead:
	outi	TWCR,(1<<TWINT)
	out	TWCR,ZeroReg
	stdi	yRTCStage,1
	std	Y+yRTCWaitStatus,ZeroReg
	ret
