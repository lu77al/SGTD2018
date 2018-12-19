.include "m8def.inc"
.include "Constants.inc"
.include "Macro.inc"
.include "Pinout.inc"
.include "Vars.inc"

rjmp	RESET		; Reset
reti			; INT0
reti			; INT1
reti			; TIMER2 COMP
reti			; TIMER2 OVF
reti			; TIMER1 CAPT
rjmp	SetKeysHigh	; TIMER1 COMPA
rjmp	SetKeysLow	; TIMER1 COMPB
reti			; TIMER1 OVF
reti			; TIMER0 OVF
reti			; SPI,STC
reti			; UART, RX
reti			; UART, UDRE
reti			; UART, TX
reti			; ADC
reti			; EE_RDY

DriveKeyTable:	; !!! Must be within first 256 bytes of PROM
;HS	  High	   Dead		Halls
.db	0b101010,0b000000 ; Not normal state (is used for key tests)
;000	  + + +    0 0 0
.db	0b100001,0b000001
;001	  + 0 -    0 0 -   Ok
.db	0b000110,0b000100
;010	  0 - +    0 - 0   Ok
.db	0b100100,0b000100
;011	  + - 0    0 - 0   Ok
.db	0b011000,0b010000
;100	  - + 0    - 0 0   Ok
.db	0b001001,0b000001
;101	  0 + -	   0 0 -   Ok
.db	0b010010,0b010000
;110	  - 0 +    - 0 0   Ok
.db	0b101010,0b000000 ; Not normal state (is used for key tests) 
;111	  + + +    0 0 0 

.include "UART.inc"
.include "Tact.inc"
.include "Subs.inc"
.include "DriverLowLayer.inc"
.include "Time.inc"
.include "Regular.inc"
.include "Setup.inc"
.include "DS1307.inc"
.include "EEPROM.inc"

;.include "Tests.inc"

RESET:

.include "Init.inc"

;	rcall	SetName

;	rcall	TestCalcOffset

Main:
	tskTact
RetTact:
	tskUart
RetUart:
	ldd	r25,Y+yRTCStage
	cpse	r25,ZeroReg
	rcall	Chat_RTC

	rjmp	Main
	
;********** BootLoader V0.0 **********
.ORG	SECONDBOOTSTART
BootLoader:
