	processor 6502
	include vcs.h



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
;org $F880 (page 1) tranport 						---1 1000 1000 0000 / 128  (128)
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

transport_buttons
	;setup console
	lda INPT4       ; button - - - - - - -  ; even page, but A12=0
    asl             ; carry has button
	lda SWCHB		; a b - - color - select reset
	rol				; b - - color - select reset button
	and #%00010111	; 0 0 0 color 0 select reset button
	sta TRANSPORT_DATA

	; have to enter at pixel clock -50
	sta  USER_TEMP	
	sta  USER_TEMP	
	sta  USER_TEMP	
	sta  USER_TEMP	
	sta  USER_TEMP	
	nop

	rts

transport_direction
	lda SWCHA			; RLDU(1) RLDU(2)	; 1111 1111 by default, so joy2 will override some console switches
	lsr
	lsr
	lsr					; 000 RLDUr
	sta TRANSPORT_DATA

	; have to enter at pixel clock -50
	sta  USER_TEMP	
	sta  USER_TEMP	
	sta  USER_TEMP	
	sta  USER_TEMP	
	sta  USER_TEMP	
	sta  USER_TEMP	
	nop

	rts


;;;;;;;;;;;;;;;;;;; page 2 - right
	org $F900 

	;; empty chunk available.

	;; needs to cross to page 3,  (same 256-byte block)

	org $F94C

		;;;;;;;;;;;;;;;;;;;;;
		;;
		;; right hand
		;;
		;;;;;;;;;;;;;;;;;;;;;

	; enter at clock -50

right_line
		sta AUDV0		;3 cycles	; pixel clock -50
		sta USER_TEMP	;3 cycles	; dummy

		;back 8, late hmove
		;x has 0

		stx	HMP0
		stx	HMP1

		lda #GDATA0
		sta GRP0
		lda #GDATA1
		sta GRP1
		lda #GDATA2
		sta GRP0

		lda #GDATA3
		ldx #GDATA4
		ldy #GCOL0

		sta GRP1	;need 1 pixel before
		lda	#GCOL1
		sta COLUP0
		stx GRP0
		lda	#GCOL2
		sta COLUP1
		sty COLUP0
		sty GRP1	;arbitrary

		lda	#GCOL3
		sta COLUP0
		lda	#GCOL4
		sta COLUP1
		;;;;;;;;;;;;;;;;;;;;;

		;needs to be on cycle 71 !!!!
		;back 8, late hmove
		sta HMOVE		

;;;;;;;;;;;;;;;;;;; page 3 - left
	org $F980
left_line
		;;;;;;;;;;;;;;;;;;;;;
		;;
		;; left hand
		;;
		;;;;;;;;;;;;;;;;;;;;;

			ldx	#$00	; dummy
			ldx	#$00	; dummy		(4cycles *3 = 12 pixels, 1 too short)

		lda #AUD_DATA	;2 cycles
			ldx	#$80 ;forward 8
		sta AUDV0		;3 cycles	; pixel clock -50

			stx	HMP0
			stx	HMP1

		lda #GDATA5
		sta GRP0
		lda #GDATA6
		sta GRP1
		lda #GDATA7
		sta GRP0

		lda #GDATA8
		ldx #GDATA9
		ldy #GCOL5

		sta GRP1	;2-before on left
		lda	#GCOL6
		sta COLUP0
		stx GRP0
		lda	#GCOL7
		sta COLUP1
		sty COLUP0
		sty GRP1	;arbitrary

		lda	#GCOL8
		sta COLUP0
		lda	#GCOL9
		sta COLUP1
		;;;;;;;;;;;;;;;;;;;;;



		lda #AUD_DATA	;2 cycles

		ldx #0			;for hmps 2 cycles
		ldx #0			;dummy 2 cycles
		ldx #0			;dummy 2 cyels
		
		sta HMOVE		;after wsync

		; have to re-enter at correct cycle
pick_continue
		jmp right_line
;		jmp end_lines

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
	sta SWACNT
	sta SWBCNT

	;3 copies medium
	lda #$06
	sta NUSIZ0

	;2 copies medium
	lda #$02
	sta NUSIZ1

;	jmp	blank_lines
	lda #0
	sta WSYNC
	sta USER_TEMP
	jmp end_lines


;;;;;;;;;;;;;;;;;;; page 5 - end blank
	org $FA80 

end_lines

    sta AUDV0

	;turn off graphics
	lda #$0
	sta GRP0
	sta GRP1
	sta GRP0
	sta GRP1

	;;;;;;;;;;;;;;;;;;;;
	;setup resp01, 2 scanlines of overscan

	sta USER_TEMP
	sta USER_TEMP
	sta USER_TEMP
	lda #$0	; dummy

	sta RESP0	; pixel pos 34
	nop
	sta RESP1	; pixel pos 49


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


	; shift p0,p1;
	; currently 48, 63
	; need 48, 64  -> actually 51, 67
	; then 40, 56  -> actually 43, 59

;;shift_graphics
	lda #$D0  ;#$70
	sta HMP0
	lda #$C0  ;#$60
	sta HMP1

end_lines_audio
	lda #AUD_DATA	;2 cycles

	sta WSYNC		;3 cycles
	sta HMOVE		;3 cycles

	sta USER_TEMP	;3 cycles
	sta AUDV0		;3 cycles	pixel clock -50

	;continue overscan with audio bank memory

	ldy #0

	;overscan 28 (2 above)
set_overscan_size
	ldx #28
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
		ldx #37
		jsr	wait_lines
	ldx  #0		;dont use A, it has last audio
	stx  VBLANK	

pick_transport
	jsr	transport_buttons
;	jsr	transport_direction

last_audio
	lda #AUD_DATA	; 2 cycles
	jmp	right_line

wait_lines

	lda (AUD_BANK_LO),y	;5 cycles
	sta WSYNC			;3 cycles
	sta  USER_TEMP		;3 cycles
	sta  USER_TEMP		;3 cycles
	sta AUDV0			;3 cycles	, pixel clock -50

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


;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
; kernel 3
;
; frame1   40, 56  -5($50)  -4($60)
; frame2   48, 64  +3($D0)  +4($C0)
;
; frame 1
;
;  colors:  ABCDE-
;color                colup0          colup1          A-e1            A-e3            p0-e4           ----
;graphic              grp0del         grp1del         gp0             A-e0            X-e2            ----
;            40       ........        ........        ........        ........        ........        ........
;            40       0000000?        11111111        00000000        11111111        00000000        --------
;                             --------        --------        --------        --------        --------        ----------------
;            38     sta---gp1+ e0
;                            lda---sta---cl0+ e1
;                                           stx---gp0+ e2
;                                                    lda---sta---cl1+ e3
;                                                                   sty---cl0+ e4
;                                                                            sty---gp1+ e4
; frame 1b
;
;  colors:  ABCDE-
;
;color                        colup0          colup1          A-e1            A-e3            p0-e4           A-e6
;graphic                      grp0del         grp1del         gp0             A-e0            X-e2            Y-e4
;            48               ........        ........        ........        ........        ........        ........
;            48               00000000        11111111        00000000        11111111        00000000        --------
;                                     --------        --------        --------        --------?       --------        ----------------
;            47              sta---gp1+ e0
;                                     lda---sta---cl0+ e1
;                                                    stx---gp0+ e2
;                                                             lda---sta---cl1+ e3
;                                                                            sty---cl0+ e4
;                                                                                     sty---gp1+ e4
;
;   ABCDE-  ABCDE-  -> AaBbCcDdEe--
;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
