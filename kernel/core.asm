
	processor 6502
	include vcs.h

;user memory 128 to 255
DUMMY	equ $80


GAUDIO	equ #0
NUM_LINES equ	33

#if 1	; NTSC
VISIBLE_LINES	equ (192 - NUM_LINES*2)
GCOL0			equ $42	;red
GCOL5			equ $36	;orange
GCOL1			equ $EC	;yellow
GCOL6			equ $D8	;light green
GCOL2			equ $72	;blue
GCOL7			equ $62	;purple
GCOL3			equ $B8	;cyan
GCOL8			equ $6C	;light purple
GCOL4			equ $06	;dark grey
GCOL9			equ $0A	;light grey
GBKCOLOR		equ $0E ; white

#else	; PAl

VISIBLE_LINES	equ (242 - NUM_LINES*2)
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
GBKCOLOR		equ $0E ; white
#endif

GDATA0	set	%01111110
GDATA5	set	%10111101
GDATA1	set	%11011011
GDATA6	set	%11100111
GDATA2	set	%11101011
GDATA7	set	%11011101
GDATA3	set	%10111110
GDATA8	set	%11010101
GDATA4	set	%11101011
GDATA9	set	%11100100



	seg

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

	MAC line_pair

	right_line
	left_line
	jmp .lineX
.lineX

	ENDM


	MAC left_line

	lda #$00		; 2 dummy
	lda #GDATA1		; 2
	sta HMOVE		;back 8, late hmove    ;needs to be on cycle 71
	sta GRP1		; 3 VDELed
	lda #GCOL1		; 2
	sta COLUP1		; 3
	lda #GAUDIO		; 2
	sta AUDV0		; 3 @10
	ldx #GDATA4		; 2
	ldy #GCOL4		; 2
	lda #GDATA0		; 2
	sta GRP0		; 3
	lda #GCOL0		; 2
	sta COLUP0		; 3
	lda #GDATA3		; 2
	sta GRP1		; 3 VDELed
	lda #GBKCOLOR	; 2 playfield color
	sta COLUPF		; 3 playfield color
	lda #GCOL2		; 2
	sta COLUP0		; 3 @39! end of GRP0a display
	lda #GDATA2		; 2
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

	ENDM


	MAC right_line

	lda #GDATA6		; 2
	sta GRP1		; 3 VDELed
	sta HMOVE		; 3 @03 +8 pixel
	lda #GAUDIO		; 2
	ldx #GDATA9		; 2
	sta AUDV0		; 3 @10
	ldy #GCOL9		; 2
	lda #GCOL6		; 2
	sta COLUP1		; 3
	lda #GDATA5		; 2
	sta GRP0		; 3
	lda #GCOL5		; 2
	sta COLUP0		; 3
	lda #GDATA8		; 2
	sta GRP1		; 3 VDELed
	lda #GBKCOLOR	; 2
	sta COLUBK		; 3 background color
	lda #GCOL7		; 2
	sta COLUP0		; 3 @42! end of GRP0a display
	lda #GDATA7		; 2
	sta GRP0		; 3 @47! end of GRP1a display
	lda #GCOL8		; 2
	sta COLUP1		; 3 @52
	stx GRP0		; 3 @55
	sty COLUP0		; 3 @58<=@60
	lda #$00		; 2 turn off background color
	sta COLUBK		; 3 background color
	sta HMCLR		; 3

	ENDM

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

	REPEAT (NUM_LINES-1)
		line_pair
	REPEND

	right_line
;	left_line
;	jmp line8

	;clear
end_lines

	lda #0
	sta GRP0
	sta GRP1
	sta GRP0

	; remainder of lines
	ldx #VISIBLE_LINES
	jsr wait_lines

	; overscan 30
	ldx #30
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

	;; wait...

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


