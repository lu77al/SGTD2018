;*********************************
; Print number (0..65535)
;  r18 = Digits-1; r16,r17 = Value; X -> address
;*********************************
B2Dec:	ldi	r19,0
	ldi_w	ZH,ZL,Razr*2
	mov	r14,r18
	ldi	r18,4
;	cpi	r19,$30
	breq	b2d_5
;	ldi	r19,$20
b2d_5:	lpm	r15,Z+
	lpm	r0,Z+
	ldi	r19,0
b2d_2:	sub	r16,r15
	sbc	r17,r0
	brcs	b2d_1
;	ori	r19,$30
	inc	r19
	rjmp	b2d_2
b2d_1:	add	r16,r15
	adc	r17,r0
	cp	r14,r18
	brlo	b2d_3
	st	X+,r19
b2d_3:	dec	r18
	brne	b2d_5
;	subi	r16,-$30
	st	X+,r16
	ret

Razr:
	.db	low(10000), high(10000)
	.db	low(1000), high(1000)
	.db	low(100), high(100)
	.db	low(10), high(10)

;*********************************
; Print 2 digits with leading zero
; r16 = Value; X -> address
;*********************************
Print2:	ldi	r17,$2F
p2_1:	inc	r17
	subi	r16,10
	brcc	p2_1
	st	X+,r17
	subi	r16,-(10+$30)
	st	X+,r16
	ret

;*********************************
; Print byte with zeroes r16
;*********************************
PrintByte:
	ldi	r17,$2F
prb_1:	inc	r17
	subi	r16,100
	brcc	prb_1
	st	X+,r17
	subi	r16,-100
	ldi	r17,$2F
prb_2:	inc	r17
	subi	r16,10
	brcc	prb_2
	st	X+,r17
	subi	r16,-(10+$30)
	st	X+,r16
	ret

;*****
; Print decimal point
;*****
Point:
	subi	XL,2
	ld	r16,X
	cpi	r16,$20
	brne	pnt_1
	ldi	r16,$30
pnt_1:	st	X+,r16
	ld	r16,X
	ldi	r17,$2E
	st	X+,r17
	st	X+,r16
	ret

DPoint:
	subi	XL,3
	ld	r16,X
	cpi	r16,$20
	brne	dpnt_1
	ldi	r16,$30
dpnt_1:	st	X+,r16
	ld	r16,X
	cpi	r16,$20
	brne	dpnt_2
	ldi	r16,$30
dpnt_2:	ldi	r17,$2E
	st	X+,r17
	ld	r17,X
	st	X+,r16
	st	X+,r17
	ret

;-----------------------------------
; r17:r16 * r19:r18 shr 16 = r17:r16
;-----------------------------------
K16x16:	push	r21
	push	r20
	mul	r17,r19
	movw	r21:r20,r1:r0
	mul	r16,r19
	mov	r19,r0
	add	r20,r1
	adc	r21,ZeroReg
	mul	r17,r18
	add	r19,r0
	adc	r20,r1
	adc	r21,ZeroReg
	mul	r17,r16
	add	r19,r1
	adc	r20,ZeroReg
	adc	r21,ZeroReg
	movw	r17:r16,r21:r20
	pop	r20
	pop	r21
	ret



;***************************
; Divide Result = Val1/Val2
;***************************
Div:	clr	r14		;clear remainder Low byte
	sub	r15,r15		;clear remainder High byte and carry
	ldi	r20,17		;init loop counter
d16u_1:	rol	r16		;shift left dividend
	rol	r17
	dec	r20		;decrement counter
	brne	d16u_2		;if done
	ret			;    return
d16u_2:	rol	r14		;shift dividend into remainder
	rol	r15
	sub	r14,r18		;remainder = remainder - divisor
	sbc	r15,r19
	brcc	d16u_3		;if result negative
	add	r14,r18		;    restore remainder
	adc	r15,r19
	clc			;    clear carry to be shifted into result
	rjmp	d16u_1		;else
d16u_3:	sec			;    set carry to be shifted into result
	rjmp	d16u_1

;------ 24bit division -------

.def	remL=r13
.def	remM=r14
.def	remH=r15

.def	ddrL=r16
.def	ddrM=r17
.def	ddrH=r18

.def	dvL=r19
.def	dvM=r20
.def	dvH=r21

.def	divcnt=r22

;***** Code

Div24:	clr	remL
	clr	remM
	sub	remH,remH
	ldi	divcnt,25

div_3:	rol	ddrL
	rol	ddrM
	rol	ddrH
	dec	divcnt
	brne	div_1
	ret
div_1:	rol	remL
	rol	remM
	rol	remH
	sub	remL,dvL
	sbc	remM,dvM
	sbc	remH,dvH
	brcc	div_2
	add	remL,dvL
	adc	remM,dvM
	adc	remH,dvH
	clc
	rjmp	div_3
div_2:	sec
	rjmp	div_3


;***** Subroutine Register Variables

#define	drem8u	r15		;remainder
#define	dres8u	r16		;result
#define	dd8u	r16		;dividend
#define	dv8u	r17		;divisor
#define	dcnt8u	r18		;loop counter

;***** Code

div8:	sub	drem8u,drem8u	;clear remainder and carry
	ldi	dcnt8u,9	;init loop counter
d8u_1:	rol	dd8u		;shift left dividend
	dec	dcnt8u		;decrement counter
	brne	d8u_2		;if done
	com	dres8u
	ret			;    return
d8u_2:	rol	drem8u		;shift dividend into remainder
	sub	drem8u,dv8u	;remainder = remainder - divisor
	brcc	d8u_1		;if result negative
	add	drem8u,dv8u	;    restore remainder
	rjmp	d8u_1

;******************
; Copy PROM -> SRAM
;******************
CopPS:	lpm	r0,Z+
	st	X+,r0
	loop	r16,CopPS
	ret

EERead:	sbic	EECR,EEWE
	rjmp	EERead
	out	EEARL,r24
	out	EEARH,r25
	sbi	EECR,0
	in	r16,EEDR
	subi_w	r25,r24,-1
	ret

EEWrite:
	push	r17
EEW_2:	sbic	EECR,EEWE
	rjmp	EEW_2
	out	EEARL,r24
	out	EEARH,r25
	subi_w	r25,r24,-1
	sbi	EECR,0
	in	r17,EEDR
	cp	r17,r16
	breq	eew_1
	out	EEDR,r16
	cli
	sbi	EECR,EEMWE
	sbi	EECR,EEWE
	sei
eew_1:	pop	r17
	ret

