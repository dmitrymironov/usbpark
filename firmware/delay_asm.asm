INCLUDE "p18cxxx.inc"

CODE
;==============================================================================
; delay_asm_c18_32mhz
;
; Precise delay routine for Microchip C18 compiler for PIC running at 32MHz.
;
; C-function prototype:
; void delay_asm_pic18_32mhz (sint16 usec);
;
; usec must be > 0
; usec must be <= 0x7FFF (32767 us)
;
; The usec parameter is passed via the stack. See chapter "3.2 Calling
; Conventions in the MPLAB® C18 C COMPILER USER’S GUIDE" (Figure 3-2).
;
; MOVLW 0xfe => addresses low byte of usec via PLUSW1 register
; MOVLW 0xff => addresses high byte of usec via PLUSW1 register
;==============================================================================
delay_asm_c18_32mhz
	; increment high byte to compensate the decrement below (DECF PLUSW1, 0x1, 0x0)
	MOVLW 0xff				; address high byte via PLUSW1
	TSTFSZ PLUSW1, 0x0		; test if high byte is 0
	GOTO delay_big			; => if high byte != 0, we have a big delay
	GOTO delay_small		; => high byte == 0, so we just need to take care of the low byte
	
delay_big
	MOVLW 0xfe				; address low byte via PLUSW1
	TSTFSZ PLUSW1, 0x0		; test if low byte == 0
	GOTO inc_high			; if low byte != 0, one extra loop_high execution is required
	GOTO loop_low_overhead	; if low byte == 0, no extra loop_high execution is required

; one extra loop_high execution is required
inc_high
	MOVLW 0xff				; address high byte via PLUSW1
	INCF PLUSW1, 0x1, 0x0	; increment high byte since one extra loop_high execution is required

; predecrement without NOPs to reduce overhead of above calculations
loop_low_overhead
	MOVLW 0xfe				; 
	DECF PLUSW1, 0x1, 0x0	; 
	BZ loop_high			; 
	DECF PLUSW1, 0x1, 0x0	; 
	BZ loop_high			; 
	DECF PLUSW1, 0x1, 0x0	; 
	BZ loop_high			; 
	NOP

loop_low
	NOP
	NOP
	NOP
	NOP
	MOVLW 0xfe				; 1 cycle
	DECF PLUSW1, 0x1, 0x0	; 1 cycle; decrement low byte
	BNZ loop_low			; 2 cycles

loop_high
	MOVLW 0xff				; 1 cycle
	DECF PLUSW1, 0x1, 0x0	; 1 cycle; decrement high byte
	BNZ loop_low			; 2 cycle
	GOTO done


delay_small
; decrement low byte without NOPs (as it will be done inside loop_small) to make 1us
; delay more precise and reduce function all overhead
	MOVLW 0xfe				; 1 cycle	=> 125ns
	DECF PLUSW1, 0x1, 0x0	; 1 cycle	=> 125ns
	BZ done			; 2 cycles	=> 250ns

; decrement low byte without NOPs (as it will be done inside loop_small) to make 2us
; delay more precise and reduce function all overhead
	;MOVLW 0xfe				; 1 cycle	=> 125ns
	DECF PLUSW1, 0x1, 0x0	; 1 cycle	=> 125ns
	BZ done			; 2 cycles	=> 250ns

; decrement low byte with fewer NOPs (as it will be done inside loop_small) to make 2us
; delay more precise and reduce function all overhead
	DECF PLUSW1, 0x1, 0x0	; 1 cycle	=> 125ns
	BZ done			; 2 cycles	=> 250ns
	NOP
	NOP

loop_small
	NOP
	NOP
	NOP
	NOP
	MOVLW 0xfe				; 1 cycle
	DECF PLUSW1, 0x1, 0x0	; 1 cycle; decrement low byte
	BNZ loop_small			; 2 cycles

done
	RETURN

; export so linker can see it
GLOBAL delay_asm_c18_32mhz
END