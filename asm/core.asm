	processor 6502
	include vcs.h


;Loop to fill in color + audio video data, with title screen and transport controls
;Original implementation by Rob Bairos 2017-2020

;user memory 128 to 255
USER_TEMP			equ	$80 ; dummy cycles
TRANSPORT_DATA		equ	$81 ; switches + joystick + button, shifted
AUD_BANK_LO			equ	$82
AUD_BANK_HI			equ	$83

BG_ANIMATE1			equ $84
BG_ANIMATE2			equ $85
BG_ANIMATE3			equ $86

TEMP_TITLE1			equ $87
TEMP_TITLE2			equ $88
TITLE_REPEAT		equ $89

#if 1

AUD_DATA	equ #0

GDATA9	equ	%11011111
GDATA8	equ	%11110111
GDATA7	equ	%11111101
GDATA6	equ	%10011111
GDATA5	equ	%11100111

GDATA4	equ	%10111111
GDATA3	equ	%11101111
GDATA2	equ	%11111011
GDATA1	equ	%11001111
GDATA0	equ	%11110011

GCOL0	equ $42	;red
GCOL1	equ $36	;orange
GCOL2	equ $EE	;yellow
GCOL3	equ $B6	;green
GCOL4	equ $72	;blue

GCOL5	equ $F2	;brown
GCOL6	equ $F8	;tan
GCOL7	equ $AC	;light blue
GCOL8	equ $D8	;light green
GCOL9	equ $62	;purple
#endif




	seg

;;;;;;;;;;;;;;;;;;;

;org $F800 (page 0) title_48  	  					---1 1000 0000 0000 / 0    (128)
;org $F880 (page 1) transport						---1 1000 1000 0000 / 128  (128)
;org $F900 (page 2) line_right      				---1 1001 0000 0000 / 256  (128)
;org $F980 (page 3) line_left						---1 1001 1000 0000 / 384  (128)
;org $FA00 (page 4) main          					---1 1010 0000 0000 / 512  (128)
;org $FA80 (page 5) endlines     					---1 1011 0000 0000 / 640  (128)
;org $FB00 (page 6) title        					---1 1011 0000 0000 / 768  (128)
;org $FB80 (page 7) audio							---1 1011 1000 0000 / 896  (128)
;org $FC00 -end-                  					---1 1100 0000 0000 / 1024 // not reached
;org fffa                         					---1 1111 1111 1010 ;NMI
;org fffc                         					---1 1111 1111 1100 ;RESET
;org fffe                         					---1 1111 1111 1110 ;IRQ

;org $1480 transport signal    						---1 -1-- 1--- ----	;maps onto 1080 ->  F080 -> page 1

;A12 - chip enable
;A11 - chip enable

;A10 - transport count
;A7  - page

;;;;;;;;;;;;;;;;;;;  page 0	 - title48
	org $F800

kernel_48
	sta WSYNC
	
	lda A48,Y
	sta GRP0
	lda B48,Y
	sta GRP1
	lda C48,Y
	sta GRP0


	lda D48,Y
	sta TEMP_TITLE1
	lda E48,Y
	ldx F48,Y
	sty TEMP_TITLE2
	ldy TEMP_TITLE1
	

	sty GRP1
	sta GRP0
	stx GRP1
	stx GRP0

	ldy TEMP_TITLE2

	ror TITLE_REPEAT
	bcs kernel_48

	dey
	bpl kernel_48
	
	rts
	
A48
	.byte  $00, $01, $01, $01, $00, $00, $30, $30, $32, $35, $38, $30
B48
	.byte  $f9, $81, $81, $80, $f8, $00, $63, $66, $66, $66, $e6, $63
C48
	.byte  $8c, $fc, $8c, $88, $70, $00, $c0, $61, $63, $66, $66, $c6
D48
	.byte  $c6, $c6, $f8, $c6, $f8, $00, $c1, $20, $30, $18, $18, $19
E48
	.byte  $18, $18, $18, $18, $7e, $00, $f9, $61, $61, $61, $61, $f9
F48
	.byte  $00, $00, $00, $00, $00, $00, $f8, $80, $80, $e0, $80, $f8

	
;;;;;;;;;;;;;;;;;; page 1 - transport
	org $F880

transport_direction
	lda SWCHA		; RLDU(1) RLDU(2)	; 1111 1111 by default, so joy2 will override some console switches
	lsr
	lsr
	lsr				; 000 RLDUr

	sta TRANSPORT_DATA

	; have to re-enter at correct cycle
	nop
	sta USER_TEMP	; 3 dummy
	sta USER_TEMP	; 3 dummy
	sta USER_TEMP	; 3 dummy
	sta USER_TEMP	; 3 dummy

	lda #00
	jmp	right_line


transport_buttons
	;setup console
	lda INPT4       ; button - - - - - - -  ; even page, but A12=0
    asl             ; carry has button
	lda SWCHB		; a b - - color - select reset
	rol				; b - - color - select reset button
	and #%00010111	; 0 0 0 color 0 select reset button


	sta TRANSPORT_DATA

	; have to re-enter at correct cycle
	nop				; dummy
	sta USER_TEMP	; dummy
	sta USER_TEMP	; dummy
	sta USER_TEMP	; dummy

	lda #00
	jmp	right_line


;;;;;;;;;;;;;;;;;;; page 2 - right
	org $F900 

	;; empty chunk available.

	;; needs to cross to page 3,  (same 256-byte block)

	org $F944

		;;;;;;;;;;;;;;;;;;;;;
		;;
		;; right hand
		;;
		;;;;;;;;;;;;;;;;;;;;;

	; enter at pixel pos 151 / color clock 219
	; left_line/right_line kernel adapted by Thomas Jentzsch

right_line

	sta		COLUPF		; 3  background color (WSYNC)
    sta     HMOVE       ; 3     @03     +8 pixel
;---------------------------------------
; - 0a - 1a - 0b - 1b - 0c
set_aud_right
	lda 	#AUD_DATA	; 2
set_gdata9
    ldx     #GDATA9     ; 2
    sta     AUDV0       ; 3     @10
set_gcol9
    ldy     #GCOL9   	; 2

set_gdata6
    lda     #GDATA6     ; 2
    sta     GRP1        ; 3     VDELed
set_gcol6
    lda     #GCOL6   	; 2
    sta     COLUP1      ; 3

set_gdata5
    lda     #GDATA5     ; 2
    sta     GRP0        ; 3
set_gcol5
    lda     #GCOL5   	; 2
    sta     COLUP0      ; 3

set_gdata8
    lda     #GDATA8     ; 2
    sta     GRP1        ; 3     VDELed

set_gcol7
    lda     #GCOL7   	; 2
    sta     COLUP0      ; 3     @42!    end of GRP0a display
set_gdata7
    lda     #GDATA7     ; 2
    sta     GRP0        ; 3     @47!    end of GRP1a display

set_gcol8
    lda     #GCOL8   	; 2
    sta     COLUP1      ; 3     @52

    stx     GRP0        ; 3     @55
    sty     COLUP0      ; 3     @58     <=@60

    sta     HMCLR       ; 3

set_colubk_r
	lda		#$00		; 2	 background color
	sta		COLUBK		; 3  background color

	lda		#$00		; 2  dummy

set_colupf_r
	lda		#$00		; 2  playfield color

	;back 8, late hmove ;needs to be on cycle 71
	sta		HMOVE		

;;;;;;;;;;;;;;;;;;; page 3 - left
	org $F980

left_line
		;;;;;;;;;;;;;;;;;;;;;
		;;
		;; left hand
		;;
		;;;;;;;;;;;;;;;;;;;;;


;EnterLeftKernel
	sta		COLUPF		; 3		playfield color
;---------------------------------------
; 0a - 1a - 0b - 1b - 0c -
set_gdata1
    lda     #GDATA1     ; 2
    sta     GRP1        ; 3     VDELed

set_aud_left
	lda 	#AUD_DATA	; 2
    sta     AUDV0       ; 3     @10
set_gdata4
    ldx     #GDATA4     ; 2
set_gcol4
    ldy     #GCOL4   	; 2

set_gcol1
    lda     #GCOL1   	; 2
    sta     COLUP1      ; 3

set_gdata0
    lda     #GDATA0     ; 2
    sta     GRP0        ; 3
set_gcol0
    lda     #GCOL0   	; 2
    sta     COLUP0      ; 3

set_gdata3
    lda     #GDATA3     ; 2
    sta     GRP1        ; 3     VDELed

set_gcol2
    lda     #GCOL2   	; 2
    sta     COLUP0      ; 3     @39!    end of GRP0a display
set_gdata2
    lda     #GDATA2     ; 2
    sta     GRP0        ; 3     @44!    end of GRP1a display

set_gcol3
    lda     #GCOL3   	; 2
    sta     COLUP1      ; 3     @49

    stx     GRP0        ; 3     @52
    sty     COLUP0      ; 3     @55     <=@57

    lda     #$80        ; 2
    sta     HMP0        ; 3
    sta     HMP1        ; 3     @63


set_colubk_l
	lda		#$00		; 2	 background color
	sta		COLUBK		; 3  background color
set_colupf_l
	lda		#$00		; 2	 background color

	; have to re-enter at correct cycle
pick_continue
	jmp		right_line
;	jmp		end_lines

;;;;;;;;;;;;;;;;;;; page 4 - main
	org $FA00 

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

	; bank audio
aud_bank_setup
	lda #(audio_bank & 255)
	sta AUD_BANK_LO
	lda #(audio_bank >> 8)
	sta AUD_BANK_HI

	;vdelp
	lda #1
	sta VDELP0
	sta VDELP1

	;setup title graphics
	;3 copies close
	lda #$03
	sta NUSIZ0
	sta NUSIZ1


	sta WSYNC
	ldx #4
tg0:
	sta USER_TEMP
	dex
	bne tg0
	sta USER_TEMP
	sta RESP0
	sta RESP1

	;shift_graphics
	lda #$D0
	sta HMP0
	lda #$E0
	sta HMP1
	sta WSYNC
	sta HMOVE

title_again

	lda BG_ANIMATE1 ; #$20
	ror 
	ror 
	sta COLUP0
	sta COLUP1

	lda #%01010101
	sta TITLE_REPEAT

	jsr draw_title
	bcs title_again
;	bcc title_again

	; back to black
	;joystick/console
	lda #$00
	sta COLUBK	
	sta VDELP0
	sta GRP0

	sta RESP0
	nop
	sta RESP1

	sta GRP1
	sta GRP0
	sta GRP1

	;3 copies medium
	lda #$06
	sta NUSIZ0

	;2 copies medium
	lda #$02
	sta NUSIZ1

	;vdelp
	lda #1
	sta VDELP1

#if 0
	0x70	-7	Move left indicated number of clocks
	0x60	-6
	0x50	-5
	0x40	-4
	0x30	-3
	0x20	-2
	0x10	-1
	0x00	0	No Motion
	0xF0	+1	Move right indicated number of clocks
	0xE0	+2
	0xD0	+3
	0xC0	+4
	0xB0	+5
	0xA0	+6
	0x90	+7
	0x80	+8
#endif

	; have 48, 63
	; need 48, 64
	; shift_graphics

	lda #$00
	sta HMP0
	lda #$F0
	sta HMP1
	sta HMOVE

	ldx	#05
wait_cnt
	dex
	bne wait_cnt

    sta HMCLR

	jmp end_lines


;;;;;;;;;;;;;;;;;;; page 5 - end blank
	org $FA80 

end_lines

	;continue overscan with audio bank memory
	ldy #0

	sty GRP0
	sty GRP1	;VDEL'd
	sty GRP0

	;no time to get to audio routine below, so cross to new line and setup audio now
    lda     #0			; 2  dummy
    lda     #0			; 2  dummy
set_aud_endlines
	lda 	#AUD_DATA	; 2
    sta     AUDV0       ; 3     @10

	; setup playfield here...
	; alternate blocks
	lda #$30
	sta PF0
	lda #$CC
	sta PF1
	lda #$33
	sta PF2


	;overscan 29;	one above
set_overscan_size
	ldx #29
	jsr	wait_lines

	;vsync 3
	lda  #2
	sta  VSYNC	
		ldx #3
		jsr	wait_lines
	lda  #0
	sta  VSYNC	

	;vblank 37
	lda  #2
	sta  VBLANK	
set_vblank_size
	ldx	#37
	jsr	wait_lines
	ldx  #0
	stx  VBLANK	

pick_extra_lines
	ldx	#00
	beq pick_transport
	jsr	wait_lines

	nop				; 2 dummy
	nop				; 2 dummy
	sta USER_TEMP	; 3 dummy
	sta USER_TEMP	; 3 dummy

pick_transport
	jmp	transport_buttons


wait_lines
	sta WSYNC			;3
    lda #0	 			;2 dummy
	lda (AUD_BANK_LO),y	;5
	sta AUDV0			;3 @ 10

	;dec transport count
	;dont change X or Y !
	lda TRANSPORT_DATA
	beq	transport_done1
	dec TRANSPORT_DATA
	lda	$1480		; A12 = 1, A10 = 1, A7 = 1 (stay in odd)

transport_done1:

	iny
	dex
	bne wait_lines
	rts

;;;;;;;;;;;;;;;;;;; page 6 - rainbow bars
	org $FB00


RAINBOW_HEIGHT	equ 30
TITLE_HEIGHT	equ 12

draw_title

	; overscan 30
	ldx #30
	jsr black_bar

	;vsync 3
	lda  #2
	sta  VSYNC	
		ldx #3
		jsr black_bar
	lda  #0
	sta  VSYNC	

	;vblank 37
	lda  #2
	sta  VBLANK	
		ldx #37
		jsr black_bar
	lda  #0
	sta  VBLANK	

	dec BG_ANIMATE1
	lda BG_ANIMATE1
	sta BG_ANIMATE2

	ldy #-1
	ldx #RAINBOW_HEIGHT
	jsr animate_bar1

	ldx (#192 - #RAINBOW_HEIGHT - #RAINBOW_HEIGHT - #TITLE_HEIGHT*2)/2
	jsr black_bar

	;center text

	ldy #TITLE_HEIGHT-1
	jsr kernel_48

	lda #0
	sta GRP0
	sta GRP1
	sta GRP0
	sta GRP1

	ldx (#192 - #RAINBOW_HEIGHT - #RAINBOW_HEIGHT - #TITLE_HEIGHT*2)/2
	jsr black_bar

	lda BG_ANIMATE1
	sta BG_ANIMATE2

	ldy #1
	ldx #RAINBOW_HEIGHT
	jsr animate_bar1

title_loop
	sec
;	clc

	rts

black_bar
	lda #0
	sta BG_ANIMATE2
	ldy #0
;	fall through

animate_bar1
	sty BG_ANIMATE3
animate_bar_again1
	clc 
	lda BG_ANIMATE2
	adc BG_ANIMATE3
	sta BG_ANIMATE2

	sta WSYNC
	sta COLUBK	
animate_dex1
	dex
	bne animate_bar_again1
	rts

;;;;;;;;;;;;;;;;;;; page 7 - audio bank
	;stay in odd-page
	org $FB80 

	; stay in odd-page
audio_bank

	;70 bytes for audio bank (3+37+30)
	;but all 128 bytes available


	;end of 1k, 1k left

	; setup  -- FB   --- $00    (brk)
	; setup  FC FD   $FB $F3
	; setup  FE FF   $FB $F3
	; (should start looping with those 5 consecutive bytes)
	; setup  remaining bytes
	; when ready...
	; pause long enough to make sure 6502 is looping  
	; setup  FC FD with original address from FE FF
	
	; while  FB      $20        (only 1 bit difference, jsr reset-vector)
	; optional: main_start begins with 1024cycle delay

;;;;;;;;;;;;;;;;;;; page 7 -at end

	org $FBFA   ; 1k
reset_loop
;	.byte $FF, $00			;NMI   		// not-used   ,   BRK(00) / JSR(20)
;	.word reset_loop+1 		;RESET		// jump to FFFB=00 -> brk
;	.word reset_loop+1 		;IRQ/BRK	// after first brk 

	.word main_start		;NMI 
	.word main_start		;RESET
	.word main_start 		;IRQ/BRK



