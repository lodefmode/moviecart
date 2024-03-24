
	processor 6502
	include vcs.h

;user memory 128 to 255
DUMMY			equ $80
FIELD			equ $81

GAUDIO	equ #0
NUM_LINES equ	8
PREROLL			equ 50

#if 1	; NTSC
VISIBLE_LINES	equ (192 - NUM_LINES*2 - PREROLL + 1)
GCOL0			equ $42	;red
GCOL5			equ $36	;orange
GCOL1			equ $EC	;yellow
GCOL6			equ $D8	;light green
GCOL2			equ $72	;blue
GCOL7			equ $64	;purple
GCOL3			equ $B8	;cyan
GCOL8			equ $6C	;light purple
GCOL4			equ $06	;dark grey
GCOL9			equ $0A	;light grey
GBKCOLOR		equ $02 ;dark grey

#else	; PAl

VISIBLE_LINES	equ (242 - NUM_LINES*2 - PREROLL + 1)
GCOL0			equ $44	;red
GCOL5			equ $46	;orange
GCOL1			equ $2C	;yellow
GCOL6			equ $36	;green
GCOL2			equ $B2	;blue
GCOL7			equ $C4	;purple

GCOL3			equ $9A	;cyan
GCOL8			equ $AC	;light purple

GCOL4			equ $F8	;dark grey
GCOL9			equ $FC	;light grey
GBKCOLOR		equ $04 ;dark grey
#endif


	seg

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

	MAC kernel

	; right_line


	lda #{4}		; 2
	sta GRP1		; 3 VDELed
	sta HMOVE		; 3 @03 +8 pixel
	lda #GAUDIO		; 2
	ldx #{10}		; 2
	sta AUDV0		; 3 @10
	ldy #GCOL9		; 2
	lda #GCOL6		; 2
	sta COLUP1		; 3
	lda #{2}		; 2
	sta GRP0		; 3
	lda #GCOL5		; 2
	sta COLUP0		; 3
	lda #{8}		; 2
	sta GRP1		; 3 VDELed
	lda #GBKCOLOR	; 2
	sta COLUBK		; 3 background color
	lda #GCOL7		; 2
	sta COLUP0		; 3 @42! end of GRP0a display
	lda #{6}		; 2
	sta GRP0		; 3 @47! end of GRP1a display
	lda #GCOL8		; 2
	sta COLUP1		; 3 @52
	stx GRP0		; 3 @55
	sty COLUP0		; 3 @58<=@60
	lda #$00		; 2 turn off background color
	sta COLUBK		; 3 background color
	sta HMCLR		; 3


	; left_line

	lda #$00		; 2 dummy
	lda #{3}		; 2
	sta HMOVE		;back 8, late hmove    ;needs to be on cycle 71
	sta GRP1		; 3 VDELed
	lda #GCOL1		; 2
	sta COLUP1		; 3
	lda #GAUDIO		; 2
	sta AUDV0		; 3 @10
	ldx #{9}		; 2
	ldy #GCOL4		; 2
	lda #{1} 		; 2
	sta GRP0		; 3
	lda #GCOL0		; 2
	sta COLUP0		; 3
	lda #{7}		; 2
	sta GRP1		; 3 VDELed
	lda #GBKCOLOR	; 2 playfield color
	sta COLUPF		; 3 playfield color
	lda #GCOL2		; 2
	sta COLUP0		; 3 @39! end of GRP0a display
	lda #{5}		; 2
	sta GRP0		; 3 @44! end of GRP1a display
	lda #GCOL3		; 2
	sta COLUP1		; 3 @49
	stx GRP0		; 3 @52
	sty COLUP0		; 3 @55<=@57
	lda #$00		; 2 turn off playfield
	sta COLUPF		; 3
	lda #$80		; 2
	sta HMP0		; 3
	sta HMP1		; 3 @63

	jmp .lineX
.lineX

	ENDM

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

	org $FC00 

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

	lda #1
	sta VDELP1

	lda #$CF
	sta PF0
	lda #$33
	sta PF1
	lda #$CC
	sta PF2

	nop
	sta RESP0
	nop
	sta RESP1
	lda #$06  ;3 copies medium
	sta NUSIZ0
	lda #$02  ;2 copies medium
	sta NUSIZ1

	lda #$10
	sta HMP0
	lda #$00
	sta HMP1
	sta HMOVE

	ldx #15
wait_cnt
	dex
	bne wait_cnt

	sta HMCLR
	nop

line0

	kernel %01111100, %00110000, %01111100, %01111100, %00011100, %11111110, %01111100, %11111110, %01111100, %01111100
	kernel %10000110, %01010000, %10000010, %10000010, %00100100, %10000000, %10000010, %00000010, %10000010, %10000010
	kernel %10001010, %10010000, %00000100, %00000100, %01000100, %10000000, %10000000, %00000100, %10000010, %10000001
	kernel %10010010, %00010000, %00011000, %00111000, %10000100, %01111100, %11111100, %00001000, %01111100, %10000011
	kernel %10100010, %00010000, %01100000, %00000100, %11111110, %00000010, %10000010, %00010000, %10000010, %01111101
	kernel %11000010, %00010000, %11000000, %10000010, %00000100, %10000010, %10000010, %00100000, %10000010, %00000001
	kernel %01111100, %11111110, %11111110, %01111100, %00000100, %01111100, %01111100, %01000000, %01111100, %11111110

end_lines

	;clear
	lda #0
	sta GRP0
	sta GRP1
	sta GRP0

	ldx #VISIBLE_LINES
	jsr wait_lines


	inc FIELD
    lda FIELD
    lsr
    bcc .start_odd

.start_even

	ldx #29	; overscan
	ldy #PREROLL
	jmp	.start3

.start_odd

	ldx #30	; overscan
	ldy #(PREROLL+1)

.start3

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

	; preroll
	tya
	tax
	jsr wait_lines


	;; wait...

	lda  #0
	sta DUMMY
	sta DUMMY
	sta DUMMY
	sta DUMMY

	ldx #7
busy_wait
	dex		
	bne	busy_wait

	sta DUMMY
	sta DUMMY
	nop

	jmp line0

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


