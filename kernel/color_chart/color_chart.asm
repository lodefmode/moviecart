
	processor 6502
	include vcs.h

;user memory 128 to 255
DUMMY			equ $80

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

	MAC linerow

	ldx #10
.again{1}
	sta WSYNC		; 3

	lda #0
	sty COLUBK		; 3
	sty COLUBK		; 3
	sty COLUBK		; 3
	sty COLUBK		; 3
	sty COLUBK		; 3
	sty COLUBK		; 3
	sty COLUBK		; 3

	ldy #(${1} + 0x0)
	sty COLUBK		; 3
	ldy #(${1} + 0x2)
	sty COLUBK		; 3
	ldy #(${1} + 0x4)
	sty COLUBK		; 3
	ldy #(${1} + 0x6)
	sty COLUBK		; 3
	ldy #(${1} + 0x8)
	sty COLUBK		; 3
	ldy #(${1} + 0xA)
	sty COLUBK		; 3
	ldy #(${1} + 0xC)
	sty COLUBK		; 3
	ldy #(${1} + 0xE)
	sty COLUBK		; 3

	; black
	ldy #$00
	sty COLUBK		; 3

	dex
	bne .again{1}

	sta WSYNC

	ENDM


	seg

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
	org $F000 

main_start

	sei	
	cld  	
	ldx #$FF	
	txs	

	;zero memory
	lda #0		
ClearMem 
	sta 0,X		
	dex		
	bne ClearMem	

	jmp line_start


	; page align to avoid longer branches
;;
	org $F100 
line_start
line0
	linerow 00
	jmp line1
;;
	org $F180 
line1
	linerow 10
	jmp line2
;;
	org $F200 
line2
	linerow 20
	jmp line3
;;
	org $F280 
line3
	linerow 30
	jmp line4
;;
	org $F300 
line4
	linerow 40
	jmp line5
;;
	org $F380 
line5
	linerow 50
	jmp line6
;;
	org $F400 
line6
	linerow 60
	jmp line7
;;
	org $F480 
line7
	linerow 70
	jmp line8
;;
	org $F500 
line8
	linerow 80
	jmp line9
;;
	org $F580 
line9
	linerow 90
	jmp lineA
;;
	org $F600 
lineA
	linerow A0
	jmp lineB
;;
	org $F680 
lineB
	linerow B0
	jmp lineC
;;
	org $F700 
lineC
	linerow C0
	jmp lineD
;;
	org $F780 
lineD
	linerow D0
	jmp lineE
;;
	org $F800 
lineE
	linerow E0
	jmp lineF
;;
	org $F880 
lineF
	linerow F0


end_lines

	; overscan
#if 1	; NTSC
	ldx #(262 - 216)
#else	; PAL
	ldx #(312 - 216)
#endif

	jsr wait_lines

	; vsync 3
	lda  #2
	sta  VSYNC	
		ldx #3
		jsr wait_lines
	lda  #0
	sta  VSYNC	

	; vblank 37
	lda  #2
	sta  VBLANK	
		ldx #37
		jsr wait_lines
	lda  #0
	sta  VBLANK	


	jmp line_start

wait_lines 
	sta WSYNC
	dex		
	bne wait_lines	
	rts

	org $FFFA
reset_loop
	.word main_start		;NMI 
	.word main_start		;RESET
	.word main_start 		;IRQ/BRK


