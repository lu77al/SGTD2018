.include "m8def.inc"
.include "Macro.inc"
;------- <SETTINGS> -------
.macro	InitPorts  ; <- Initial ports state
	outi	PORTB,0b000000
	outi	DDRB, 0b111111
	outi	PORTC,0b001111
	outi	DDRC, 0b000001
	outi	PORTD,0b01011111
	outi	DDRD, 0b10100100
.endm
//#define	UART_DIR_PORT	PORTD,2	; <- UartDir
//#define	UART_DIR_DDR	 DDRD,2
.equ	CPU_FREQ  = 8000000	; <- FCPU
.equ	UART_RATE = 9600	; <- BAUD RATE
.equ	UBRRB_V   = (CPU_FREQ/(16*UART_RATE)-1)
;------- </SETTINGS> -------

rjmp	RESET
reti			; INT0
reti			; INT1
reti			; TIMER2 COMP
reti			; TIMER2 OVF
reti			; TIMER1 CAPT
reti			; TIMER1 COMPA
reti			; TIMER1 COMPB
reti			; TIMER1 OVF
reti			; TIMER0 OVF
reti			; SPI,STC
reti			; UART, RX
reti			; UART, UDRE
reti			; UART, TX
reti			; ADC
reti			; EE_RDY
reti			; ANA_COMP
reti			; TWI
reti			; SPM_RDY


RESET:
	cli
;-- Init ports state --
	InitPorts
;-- Стек и указатели --
	outi	SPH,high(RAMEND)
	outi	SPL,low(RAMEND)

	rjmp	Boot
Main:

	rjmp	Main

.macro	TR_Start
#ifdef	UART_DIR_PORT
	sbi	UART_DIR_PORT
#endif
	sbi	UCSRA,6
.endm

.macro	TR_Wait
	sbis	UCSRA,6
	rjmp	PC-1
#ifdef	UART_DIR_PORT
	cbi	UART_DIR_PORT
#endif
.endm

;********** BootLoader V0.0 **********
.ORG	SECONDBOOTSTART
Boot:
	cli
;-- Порты --
	InitPorts
#ifdef	UART_DIR_PORT
	cbi	UART_DIR_PORT
	sbi	UART_DIR_DDR
#endif
;--- Стек и указатели ---
	outi	SPH,high(RAMEND)
	outi	SPL,low(RAMEND)
	ldi	XH,1
	ldi_w	YH,YL,$60
;--- Остановить таймеры ---
	out	TCCR1A,YH
	out	TCCR1B,YH
	out	TCCR0,YH
	out	TCCR2,YH
;-- Очиститка памяти с $100 по $1ff
	ldi_w	YH,YL,$60
	ldi_w	XH,XL,$100

	ldi_w	ZH,ZL,$60
bini_1:	st	Z+,YH
	cpi	ZH,2
	brlo	bini_1
;--- Инициализация UART ---
	out	UCSRA,YH
	outi	UCSRB,(1<<RXEN) | (1<<TXEN)	; Разрешить прием и передачу
	outi	UCSRC,0b10000110	; 8-бит
	outi	UBRRH,High(UBRRB_V)
	outi	UBRRL,Low(UBRRB_V)

;--- Обмен "приветстивием" $A5-$5A ---
	in	r16,UDR
	TR_Start
	outi	UDR,$A5	; Отправка признака входа в загрузчик
	TR_Wait
	ldi	r18,High(65000)
BTL_1:	subi_w	r18,r17,1
	brne	BTL_2
BTL_3:	rjmp	BTL_EX
BTL_2:	sbis	UCSRA,RXC
	rjmp	BTL_1
	in	r16,UDR
	cpi	r16,$5A
	brne	BTL_3
;--- Инициализация загрузчика
	outi	TCCR1B,0b011	; CK/64 -> 8e6/64/65536 = 1.907Hz Overflow
	ldi	r25,58		; Счетчик выхода из загрузчика через 30 сек
;--- Получение посылки по UART ---
BTL_9: 	ldi	XL,0
	ldi	r24,0
BTL_4:	in	r16,TIFR
	sbrs	r25,7
	sbrs	r16,TOV1
	rjmp	BTL_5
	outi	TIFR,1<<TOV1
	dec	r25
	breq	BTL_3
BTL_5:	sbis	UCSRA,RXC
	rjmp	BTL_4
	in	r16,UDR
	cpi	XL,0
	brne	BTL_6
	cpi	r16,'B'
	brne	BTL_4
BTL_8:	add	r24,r16
	st	X+,r16
	rjmp	BTL_4
BTL_6:	cpi	XL,1
	brne	BTL_7
	ldi	r23,3+64
	cpi	r16,'W'
	breq	BTL_8
	ldi	r23,3
	cpi	r16,'R'
	breq	BTL_8
	ldi	r23,2+6
	cpi	r16,'O'
	breq	BTL_8
	ldi	r23,2
	cpi	r16,'I'
	breq	BTL_8
	ldi	r23,2
	cpi	r16,'Q'
	breq	BTL_8
	ldi	r23,4
	cpi	r16,'E'
	breq	BTL_8
	rjmp	BTL_9
BTL_7:	cp	XL,r23
	brlo	BTL_8
	cp	r24,r16
	brne	BTL_9
	sbrs	r25,7
	ldi	r25,58

;--- Обработка команд ---
	ldi	XL,0
	ldi	r16,'b'
	st	X+,r16
	ld	r16,X+
	cpi	r16,'W'	;--- Запись страницы ---
	brne	BWP_1
	ld	ZH,X+
	rcall	ERASE_PAGE
BWP_2:	ld	r0,X+
	ld	r1,X+
	ldi	r16,1<<SPMEN
	rcall	DO_SPM
	subi	ZL,-2
	cpi	XL,3+64
	brlo	BWP_2
	subi	ZL,64
	ldi	r16,(1<<PGWRT) | (1<<SPMEN)
	rcall	DO_SPM
BWP_3:	ldi	r16,(1<<RWWSRE) | (1<<SPMEN)
	rcall	DO_SPM
	in	r16,SPMCR
	sbrc	r16,RWWSB
	rjmp	BWP_3
	ldi	r25,$FF
	ldi	r23,3
	rjmp	BSU_3
BWP_1:
	cpi	r16,'E'	;--- Стирание страниц ---
	brne	BEP_1
	ld	r10,X+
	ld	r11,X+
BEP_2:	mov	ZH,r10
	rcall	ERASE_PAGE
	cp	r10,r11
	brsh	BEP_3
	inc	r10
	rjmp	BEP_2
BEP_3:	ldi	r25,$FF
	ldi	r23,4
	rjmp	BSU_3
BEP_1:

	cpi	r16,'R'	;--- Чтение страницы ---
	brne	BRP_1
BRP_3:	in	r17,SPMCR
	sbrc	r17,SPMEN
	rjmp	BRP_3
	ld	ZH,X+
	ldi	ZL,0
	lsr16	ZH,ZL
	lsr16	ZH,ZL
	ldi	XL,3
BRP_2:	lpm	r16,Z+
	st	X+,r16
	cpi	XL,3+64
	brlo	BRP_2
	ldi	r23,64+3
	rjmp	BSU_3
BRP_1:
	cpi	r16,'O'	;--- Установка GPIO ---
	brne	BOP_1
	ldi	ZL,$39
	ldi	ZH,0
BOP_2:	ld	r16,X+
	ld	r17,X+
	st	-Z,r16
	st	-Z,r17
	dec	ZL
	cpi	XL,2+6
	brlo	BOP_2
	rjmp	BIP_3
BOP_1:
	cpi	r16,'I'	;--- Чтение GPIO ---
	brne	BIP_1
BIP_3:	ldi	ZL,$39
	ldi	ZH,0
	ldi	XL,2
BIP_2:	ld	r16,-Z
	st	X+,r16
	cpi	XL,2+9
	brlo	BIP_2
	ldi	r23,2+9
	rjmp	BSU_3
BIP_1:
;--- Выход из загрузчика ---
BTL_EX:	InitPorts
;	out	UCSRB,YH
 	rjmp	0

;--- Отправка данных из буфера UART ---
BSU_3:	ldi	r17,9
BSU_4:	subi_w	r17,r16,1
	brcc	BSU_4
	TR_Start
	ldi	r24,0
	ldi	XL,0
BSU_1:	sbis	UCSRA,UDRE
	rjmp	BSU_1
	ld	r16,X+
	out	UDR,r16
	add	r24,r16
	cp	XL,r23
	brlo	BSU_1
BSU_2:	sbis	UCSRA,UDRE
	rjmp	BSU_2
	out	UDR,r24
	TR_Wait
	rjmp	BTL_9

ERASE_PAGE:
	ldi	ZL,0
	lsr16	ZH,ZL
	lsr16	ZH,ZL
	ldi	r16,(1<<PGERS) | (1<<SPMEN)
	rcall	DO_SPM
	ldi	r16,(1<<RWWSRE) | (1<<SPMEN)
	rjmp	DO_SPM

;--- Процедурка из мануалки ---
DO_SPM:	in	r17,SPMCR
	sbrc	r17,SPMEN
	rjmp	DO_SPM
DSPM_1:	sbic	EECR,EEWE
	rjmp	DSPM_1
	out	SPMCR,r16
	spm
	ret
	
.db	"The end of boot+"
