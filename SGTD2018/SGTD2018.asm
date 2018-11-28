.include "m8def.inc"
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

.include "UART.inc"
.include "Tact.inc"
.include "Subs.inc"
.include "DriverLowLayer.inc"

RESET:

.include "Init.inc"

	rcall	StartKeysTest
Main:
	tskTact
RetTact:
	tskUart
RetUart:
	rjmp	Main
	
;********** BootLoader V0.0 **********
.ORG	SECONDBOOTSTART
BootLoader:
