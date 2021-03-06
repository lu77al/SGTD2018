.equ	UBRR_V	 = 51
.equ	RXBufLen = 32
.equ	UARTMesDelay = 16  ; 4 Bytes
.equ	UARTByteDelay = 8  ; 2 Bytes
.equ	UARTStartByte = $14

.equ	InitDevAddr = $51

InitUart:
	outi	UCSRA,0
	outi	UCSRB,0b00011000	; ��������� ����� � ��������
	outi	UCSRC,0b10000110	; 8-���
	outi	UBRRH,High(UBRR_V)
	outi	UBRRL,Low(UBRR_V)
	stdi	yTXPointer,TXBuf
	std	Y+yTXEnd,r16
	stdi	yRXPointer,RXBuf-1
	std	Y+yRXTimer,ZeroReg
#ifdef	UART_DIR
	cbi	UART_DIR
#endif
	stdi	yMyAddr,InitDevAddr
	std	Y+yRXTXCHS,ZeroReg
	ret

.macro	macRXTimer
	ldd	r16,Y+yRXTimer
	inc	r16
	breq	rxtm_1
	std	Y+yRXTimer,r16
	cpi	r16,UARTByteDelay
	brne	rxtm_1
	stdi	yRXPointer,RXBuf-1	
rxtm_1:
.endm

; ���������� USART � �������� ������
.macro	tskUart
	sbrc	BFlags,bfATSetup
	rjmp	ATSetup
	ldd	XL,Y+yTXPointer
	ldd	r17,Y+yTXEnd
	cp	XL,r17
	breq	uart_1
	sbis	UCSRA,UDRE ; ++ ��������� � ������ ��������
	rjmp	RetUart  ; ����� ���� ����� �������� �����
#ifdef	UART_DIR
	sbi	UART_DIR  ; ����������� ���������� �� ��������
#endif
	ld	r16,X+
	cp	XL,r17
	brne	uart_2
	ldd	r16,Y+yRXTXCHS
uart_2:	out	UDR,r16   ; ��������� ���������� ���� � �����
	std	Y+yTXPointer,XL
	ldd	r17,Y+yRXTXCHS
	add	r17,r16
	std	Y+yRXTXCHS,r17
	sbi	UCSRA,TXC ; ��������� ����� ��� ����������� �������� �� �����
	rjmp	RetUart
uart_1:
#ifdef	UART_DIR
	sbic	UCSRA,TXC ; ++ ��������� � ������ ������
	cbi	UART_DIR  ;  ����������� ���������� �� ����� ����� �������� �������� �����
#endif
	sbic	UCSRA,RXC  ; ����� �� ����������� ���� ��� ��������� �����
	rjmp	ReadUart   ; ���������� �������� ����
.endm

; -1 00 01 02 03 .. NN
; ST LN DN SR D0 .. CS
ReadUart:
	in	r16,UDR
	ldd	XL,Y+yRXPointer
	cpi	XL,RXBuf-1
	breq	rx_1
	cpi	XL,RXBuf
	breq	rx_2
	lds	r17,XSEG+RXBuf	; ++ ������ ���� 01...
	cp	XL,r17		; �������� �� ���������� �������
	breq	rx_3
rx_8:	st	X+,r16		; ����������� ���������� ����� � ������
	ldd	r17,Y+yRXTXCHS
	add	r17,r16
	std	Y+yRXTXCHS,r17
rx_5:	std	Y+yRXPointer,XL
rx_4:	std	Y+yRXTimer,ZeroReg
	rjmp	RetUart
rx_1:	cpi	r16,UARTStartByte ; ++ ������ ����� -1
	brne	rx_4		;  �������� �� ���������� �����-����
	ldd	r16,Y+yRXTimer
	cpi	r16,UARTMesDelay
	brlo	rx_4		;  �������� �� ����������� �������� ����� ��������
	stdi	yRXTXCHS,UARTStartByte-(RXBuf+3)
	inc	XL
	rjmp	rx_5
rx_2:	cpi	r16,1		; ++ ������ ����� 00 (�����)
	brlo	rx_6		;  �������� �� ����������� �����
	cpi	r16,RXBufLen-2
	brlo	rx_7		;  �������� �� ������������ �����
rx_6:	ldi	XL,RXBuf-1	; ����� ������ ��� ��������� �����
	rjmp	rx_5
rx_7:	subi	r16,-(RXBuf+3)	;  ������������ ������ ����� �������
	rjmp	rx_8
rx_3:		; ++ ������� ��� ������� ++
rx_9:	ldd	r17,Y+yRXTXCHS ; �������� ����������� �����
	cp	r16,r17
	brne	rx_6	; ����� ������ ��� �������� �����
	stdi	yRXTimer,UARTMesDelay+1	; ������ ����� � �������� ����� �������
		; ++ ��������� ����������� �������
	stdi	yTXPointer,TXBuf+4
	ldi	XL,RXBuf+1
	ld	r16,X+	; ����� ����������
	ld	r17,X+	; ����� �����������
	std	Y+yRXPointer,XL
	ldd	r18,Y+yMyAddr ; *** ����� ����������� ������
	cp	r16,r18
	breq	MyCMDProcess
;	andi	r18,$F0
;	cp	r16,r18
;	breq	MyCMDProcess
				; �� ������� ����������� �����������
rx_10:	stdi	yTXPointer,TXBuf
	std	Y+yTXEnd,r16
	stdi	yRXPointer,RXBuf-1
	rjmp	RetUart
RetCMDProc:
	ldd	XL,Y+yTXPointer	; �������������� ������ ��� ������� ������
	cpi	XL,TXBuf+4
	breq	rx_10
	rcall	ComplUartMes
	rjmp	RetUart

TestUart:
	ldi	XL,TXBuf+4
	ldi	r16,$15
	st	X+,r16
	std	Y+yTXPointer,XL
	ldd	r17,Y+yMyAddr
	ldi	r16,$01

ComplUartMes:
	mov	r20,XL
	subi	r20,TXBuf+4
	inc	XL
	std	Y+yTXEnd,XL
	ldi	XL,TXBuf
	std	Y+yTXPointer,XL
	ldi	r18,UARTStartByte
	st	X+,r18
	st	X+,r20
	st	X+,r16
	st	X+,r17
	stdi	yRXPointer,RXBuf-1
	std	Y+yRXTXCHS,ZeroReg
	ret

MyCMDProcess:
	ldd	XL,Y+yRXPointer
cm_S2:
	ld	r16,X+
	std	Y+yRXPointer,XL

	cpi	r16,$00
	breq	cm0_st
	rjmp	cm0_0
cm0_st:
	push	r16
	stsi	iNoRequestsCnt,$ff

	ld	r16,X+
	std	Y+yRXPointer,XL
	cpi	r16,$50	; Request
	breq	cm0_10_

	cpi	r16,$51	; Move East
	brne	cm0_1
	rcall	MoveEastCMD
cm0_10_:
	rjmp	cm0_10
cm0_1:
	cpi	r16,$52	; Move West
	brne	cm0_2
	rcall	MoveWestCMD
	rjmp	cm0_10
cm0_2:
	cpi	r16,$53	; Stop
	brne	cm0_3
	rcall	StopCMD
	rjmp	cm0_10
cm0_3:
	cpi	r16,$54	; Delay
	brne	cm0_7
	rcall	SetupDelayCMD
	rjmp	cm0_10
cm0_7:
	cpi	r16,$55	; Set time
	brne	cm0_4
	ldi	r25,0
cm0_41:	ld	r16,X+
	std	Y+yHour,r16
	ld	r16,X+
	std	Y+yMinute,r16
	ld	r16,X+
	std	Y+ySecond,r16
	std	Y+yRXPointer,XL
	ldi	XL,RTCBuf+10
	ldi	r16,$40
	st	X,r16
	std	Y+yNextDelay,ZeroReg
	std	Y+yDelayCntH,ZeroReg
	stdi	yDelayCntL,15
	sbrc	r25,0
	rcall	UpdateTimeOffset
	rjmp	cm0_10
cm0_4:
	cpi	r16,$56	; Set time offset
	brne	cm0_5
	ldi	r25,1
	rjmp	cm0_41
cm0_5:
	cpi	r16,$57	; Set parking
	brne	cm0_6
	ld	r16,X+
	sts	iMorningL,r16
	ld	r16,X+
	sts	iMorningH,r16
	ld	r16,X+
	sts	iEveningL,r16
	ld	r16,X+
	sts	iEveningH,r16
	ld	r16,X+
	sts	iParkingL,r16
	ld	r16,X+
	sts	iParkingH,r16
	std	Y+yRXPointer,XL
	set
	bld	AFlags,afSaveParams
	std	Y+yNextDelay,ZeroReg
	std	Y+yDelayCntH,ZeroReg
	stdi	yDelayCntL,15
	rjmp	cm0_10
cm0_6:
	cpi	r16,$58	; Set parking
	brne	cm0_8
	sbrs	AFlags,afActive
	rjmp	cm0_10
	ldi	r16,$ff - (1<<btWestLimit)
	sbrc	AFlags,afReverse
	ldi	r16,$ff - (1<<btEastLimit)
	std	Y+ySimButMask,r16
	rjmp	cm0_10
cm0_8:

	rjmp	cm_S4
cm0_10:
	ldd	XL,Y+yTXPointer
	pop	r25
	ori	r25,$80
	st	X+,r25
	ldd	r16,Y+yHour
	st	X+,r16
	ldd	r16,Y+yMinute
	st	X+,r16
	ldd	r16,Y+ySecond
	st	X+,r16
	ldd	r16,Y+yPanPosL
	st	X+,r16
	ldd	r16,Y+yPanPosM
	st	X+,r16
	ldd	r16,Y+yPanPosH
	st	X+,r16
	mov	r16,AFlags
	andi	r16,0b11
	ldd	r17,Y+yButState
	swap	r17
	andi	r17,$F0
	or	r16,r17
	st	X+,r16
	ldd	r16,Y+yStopPosL
	st	X+,r16
	ldd	r16,Y+yStopPosM
	st	X+,r16
	ldd	r16,Y+yStopPosH
	st	X+,r16
	lds	r16,iHalfSpanL
	st	X+,r16
	lds	r16,iHalfSpanH
	st	X+,r16
	lds	r16,iTimeOffsetL
	st	X+,r16
	lds	r16,iTimeOffsetM
	st	X+,r16
	lds	r16,iTimeOffsetH
	st	X+,r16
	lds	r16,iMorningL
	st	X+,r16
	lds	r16,iMorningH
	st	X+,r16
	lds	r16,iEveningL
	st	X+,r16
	lds	r16,iEveningH
	st	X+,r16
	lds	r16,iParkingL
	st	X+,r16
	lds	r16,iParkingH
	st	X+,r16
	cpi	r25,$80
	breq	cm0_11
	lds	r16,iUL
	st	X+,r16
	lds	r16,iUH
	st	X+,r16
	lds	r16,iCurrent
	st	X+,r16
cm0_11:
	std	Y+yTXPointer,XL
	rjmp	cm_S4
cm0_0:

	cpi	r16,$01
	brne	cm1_0
	rjmp	cm0_st
cm1_0:

	cpi	r16,$70 ; *** ������ ������� ������ ***
	brne	cm70_0
	ld	ZH,X+	; ZH[2..0]:ZL - ��������� �� ������ ����
	ld	ZL,X+	; ������� 5 ��� ZH - ����� �������
	std	Y+yRXPointer,XL
	ldd	XL,Y+yTXPointer
	ldi	r17,$70+$80
	st	X+,r17
	mov	r25,ZH
	andi	ZH,0b111
	lsr	r25
	lsr	r25
	lsr	r25
	breq	cm70_1
cm70_2:	ld	r16,Z+
	st	X+,r16
	loop	r25,cm70_2
cm70_1:	std	Y+yTXPointer,XL
	rjmp	cm_S4
cm70_0:
	cpi	r16,$71 ; *** ������ ������� ������ ***
	brne	cm71_0
	ld	ZH,X+	; ZH[2..0]:ZL - ��������� �� ������ ����
	ld	ZL,X+	; ������� 5 ��� ZH - ����� �������
	mov	r25,ZH
	andi	ZH,0b111
	lsr	r25
	lsr	r25
	lsr	r25
	breq	cm71_1
cm71_2:	ld	r16,X+
	st	Z+,r16
	loop	r25,cm71_2
cm71_1:	std	Y+yRXPointer,XL
	rjmp	cm_S4
cm71_0:
	cpi	r16,$72 ; *** ��������� ����� ������ �������� ***
	brne	cm72_0
	ld	ZH,X+	; ZH:ZL - ��������� �� ����
	ld	ZL,X+
	ld	r16,X+	; AND_Mask
	ld	r17,X+	; OR_Mask
	ld	r18,X+	; XOR_Mask
	std	Y+yRXPointer,XL
	cli
	ld	r19,Z
	and	r19,r16
	or	r19,r17
	eor	r19,r18
	st	Z,r19
	sei
	rjmp	cm_S4
cm72_0:

	cpi	r16,$7F		; *** ������� � ���������� ***
	brne	cm7F_0
	rjmp	BootLoader
cm7F_0:

	lds	XL,XSEG+RXBuf
	std	Y+yRXPointer,XL
cm_S4:
	lds	r16,XSEG+RXBuf
	ldd	XL,Y+yRXPointer
	cp	XL,r16
	brsh	cm_S3
	rjmp	cm_S2
cm_S3:	lds	r16,XSEG+RXBuf+2
	ldd	r17,Y+yMyAddr
	rjmp	RetCMDProc

#define AT_CMD_DELAY 24

;--- Init pushing setup data into UART ---
ATSetupStart:
	stdi	yATSetupDelay,AT_CMD_DELAY
	std	Y+yATSetupPos,ZeroReg
	set
	bld	BFlags,bfATSetup
	ret

;--- Tact driven delay 15.2587890625 Hz ---
ATSetupDelayCountDown:
	ldd	r16,Y+yATSetupDelay
	subi	r16,1
	brcs	atsd_1
	std	Y+yATSetupDelay,r16
atsd_1:	ret

;--- Push setup data into UART ---
ATSetup:
	sbis	UCSRA,UDRE ; Wait for buffer ready
	rjmp	RetUart
#ifdef	UART_DIR
	sbi	UART_DIR  ; Ensure data dir
#endif
	ldd	r16,Y+yATSetupDelay ; Delay between commands (Tact driven)
	cpi	r16,0
	brne	ats_1

	ldd	ZL,Y+yATSetupPos ; Next char position
	inc	ZL
	brne	ats_2	; Boundary overflow check 
ats_3:	
	clt		; Exit ATSetup mode
	bld	BFlags,bfATSetup
	in	r16,UDR
	rjmp	RetUart

ats_2:	std	Y+yATSetupPos,ZL ; Read next char and shift pointer
	ldi	ZH,0
	subi_w	ZH,ZL,-(ATSetupData*2-1)
	lpm	r16,Z

	cpi	r16,0		; EndOfSetup marker
	breq	ats_3

	out	UDR,r16		; Push next char
	sbi	UCSRA,TXC ; For changing data direction

	cpi	r16,10		; Init delay at the end of command
	brne	ats_1
	stdi	yATSetupDelay,AT_CMD_DELAY
ats_1:	rjmp	RetUart

ATSetupData:
.db	BLE_NAME,13,10, BLE_PIN,13,10, BLE_SCAN,13,10, 0

