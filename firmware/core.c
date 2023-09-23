
#include "pff.h"
#include "core.h"

#include "mcc_generated_files/system.h"
#include "mcc_generated_files/clc1.h"
#include "mcc_generated_files/interrupt_manager.h"
#include "mcc_generated_files/pin_manager.h"

#define ST_OFF					0x86	// stx(0)
#define ST_ON					0x84	// sty(2)
#define ADDR_RIGHT_LINE			0x3e
#define ADDR_END_LINES			0xb7
// 

extern void	flash_led(uint8_t num);

// used by main + interrupt
volatile bool			mr_endFrame = 1;

bool			mr_bufferIndex = false;

uint_fast8_t	mr_swcha = 0xff;
uint_fast8_t	mr_swchb = 0xff;
uint_fast8_t	mr_inpt4 = 0xff;
uint_fast8_t	mr_inpt5 = 0xff;	// not used

// following r_* only used by interrupt code

uint_fast8_t	r_peekBus = 0xff;
uint_fast8_t*	r_storeAddress = &r_peekBus;

uint_fast8_t	r_breakLoops = 255;	// break 255 times before starting main frame loop

uint_fast8_t	r_lines = 190;

struct frameInfo		r_frameInfo;
struct frameInfo		mr_frameInfo1, mr_frameInfo2;

uint_fast8_t	r_hiAddress = 0xf0;
uint_fast8_t	r_vblankState = ST_OFF;
uint_fast8_t	r_vsyncState = ST_OFF;
uint_fast8_t	r_endState = 0;
uint_fast8_t	r_nextLineJump = ADDR_RIGHT_LINE;
uint_fast8_t	r_data = 0;



bool			r_audioPushed = false;
uint_fast8_t	r_audioVal;

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
void
coreInit()
{
	DATA_OUTPUT_DIR;
	DATA_OUTPUT;

	// get this started as soon as possible
	PIN_MANAGER_enableInterrupt_On_Change();
}


// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

void __attribute__((interrupt, no_auto_psv, context)) _CNCInterrupt(void)
{
	static const void* const romData[512] = 
	{
		&&gstore, &&gstore, &&gstore, &&gstore, &&gstore, &&gstore, &&gstore, &&gstore, &&gstore, &&gstore, &&gstore, &&gstore, &&gstore, &&gstore, &&gstore, &&gstore,
		&&gstore, &&gstore, &&gstore, &&gstore, &&gstore, &&gstore, &&gstore, &&gstore, &&gstore, &&gstore, &&gstore, &&gstore, &&gstore, &&gstore, &&gstore, &&gstore,
		&&gstore, &&gstore, &&gstore, &&gstore, &&gstore, &&gstore, &&gstore, &&gstore, &&gstore, &&gstore, &&gstore, &&gstore, &&gstore, &&gstore, &&gstore, &&gstore,
		&&gstore, &&gstore, &&gstore, &&gstore, &&gstore, &&gstore, &&gstore, &&gstore, &&gstore, &&gstore, &&gstore, &&gstore, &&gstore, &&gstore, &&gstore, &&gstore,
		&&gstore, &&gstore, &&gstore, &&gstore, &&gstore, &&gstore, &&gstore, &&gstore, &&gstore, &&gstore, &&gstore, &&gstore, &&gstore, &&gstore, &&gstore, &&gstore,
		&&gstore, &&gstore, &&gstore, &&gstore, &&gstore, &&gstore, &&gstore, &&gstore, &&gstore, &&gstore, &&gstore, &&gstore, &&gstore, &&gstore, &&gstore, &&gstore,
		&&gstore, &&gstore, &&gstore, &&gstore, &&gstore, &&gstore, &&gstore, &&gstore, &&gstore, &&gstore, &&gstore, &&gstore, &&gstore, &&gstore, &&gstore, &&gstore,
		&&gstore, &&gstore, &&gstore, &&gstore, &&gstore, &&gstore, &&gstore, &&gstore, &&gstore, &&gstore, &&gstore, &&gstore, &&gstore, &&gstore, &&gstore, &&gstore,
		&&gstore, &&gstore, &&gstore, &&gstore, &&gstore, &&gstore, &&gstore, &&gstore, &&gstore, &&gstore, &&gstore, &&gstore, &&gstore, &&gstore, &&gstore, &&gstore,
		&&gstore, &&gstore, &&gstore, &&gstore, &&gstore, &&gstore, &&gstore, &&gstore, &&gstore, &&gstore, &&gstore, &&gstore, &&gstore, &&gstore, &&gstore, &&gstore,
		&&gstore, &&gstore, &&gstore, &&gstore, &&gstore, &&gstore, &&gstore, &&gstore, &&gstore, &&gstore, &&gstore, &&gstore, &&gstore, &&gstore, &&gstore, &&gstore,
		&&gstore, &&gstore, &&gstore, &&gstore, &&gstore, &&gstore, &&gstore, &&gstore, &&gstore, &&gstore, &&gstore, &&gstore, &&gstore, &&gstore, &&gstore, &&gstore,
		&&gstore, &&gstore, &&gstore, &&gstore, &&gstore, &&gstore, &&gstore, &&gstore, &&gstore, &&gstore, &&gstore, &&gstore, &&gstore, &&gstore, &&gstore, &&gstore,
		&&gstore, &&gstore, &&gstore, &&gstore, &&gstore, &&gstore, &&gstore, &&gstore, &&gstore, &&gstore, &&gstore, &&gstore, &&gstore, &&gstore, &&gstore, &&gstore,
		&&gstore, &&gstore, &&gstore, &&gstore, &&gstore, &&gstore, &&gstore, &&gstore, &&gstore, &&gstore, &&gstore, &&gstore, &&gstore, &&gstore, &&gstore, &&gstore,
		&&gstore, &&gstore, &&gstore, &&gstore, &&gstore, &&gstore, &&gstore, &&gstore, &&gstore, &&gstore, &&gstore, &&gstore, &&gstore, &&gstore, &&gstore, &&gstore,

		&&g0x00, &&g0x01, &&g0x02, &&g0x03, &&g0x04, &&g0x05, &&g0x06, &&g0x07, &&g0x08, &&g0x09, &&g0x0a, &&g0x0b, &&g0x0c, &&g0x0d, &&g0x0e, &&g0x0f,
		&&g0x10, &&g0x11, &&g0x12, &&g0x13, &&g0x14, &&g0x15, &&g0x16, &&g0x17, &&g0x18, &&g0x19, &&g0x1a, &&g0x1b, &&g0x1c, &&g0x1d, &&g0x1e, &&g0x1f,
		&&g0x20, &&g0x21, &&g0x22, &&g0x23, &&g0x24, &&g0x25, &&g0x26, &&g0x27, &&g0x28, &&g0x29, &&g0x2a, &&g0x2b, &&g0x2c, &&g0x2d, &&g0x2e, &&g0x2f,
		&&g0x30, &&g0x31, &&g0x32, &&g0x33, &&g0x34, &&g0x35, &&g0x36, &&g0x37, &&g0x38, &&g0x39, &&g0x3a, &&g0x3b, &&g0x3c, &&g0x3d, &&g0x3e, &&g0x3f,
		&&g0x40, &&g0x41, &&g0x42, &&g0x43, &&g0x44, &&g0x45, &&g0x46, &&g0x47, &&g0x48, &&g0x49, &&g0x4a, &&g0x4b, &&g0x4c, &&g0x4d, &&g0x4e, &&g0x4f,
		&&g0x50, &&g0x51, &&g0x52, &&g0x53, &&g0x54, &&g0x55, &&g0x56, &&g0x57, &&g0x58, &&g0x59, &&g0x5a, &&g0x5b, &&g0x5c, &&g0x5d, &&g0x5e, &&g0x5f,
		&&g0x60, &&g0x61, &&g0x62, &&g0x63, &&g0x64, &&g0x65, &&g0x66, &&g0x67, &&g0x68, &&g0x69, &&g0x6a, &&g0x6b, &&g0x6c, &&g0x6d, &&g0x6e, &&g0x6f,
		&&g0x70, &&g0x71, &&g0x72, &&g0x73, &&g0x74, &&g0x75, &&g0x76, &&g0x77, &&g0x78, &&g0x79, &&g0x7a, &&g0x7b, &&g0x7c, &&g0x7d, &&g0x7e, &&g0x7f,
		&&g0x80, &&g0x81, &&g0x82, &&g0x83, &&g0x84, &&g0x85, &&g0x86, &&g0x87, &&g0x88, &&g0x89, &&g0x8a, &&g0x8b, &&g0x8c, &&g0x8d, &&g0x8e, &&g0x8f,
		&&g0x90, &&g0x91, &&g0x92, &&g0x93, &&g0x94, &&g0x95, &&g0x96, &&g0x97, &&g0x98, &&g0x99, &&g0x9a, &&g0x9b, &&g0x9c, &&g0x9d, &&g0x9e, &&g0x9f,
		&&g0xa0, &&g0xa1, &&g0xa2, &&g0xa3, &&g0xa4, &&g0xa5, &&g0xa6, &&g0xa7, &&g0xa8, &&g0xa9, &&g0xaa, &&g0xab, &&g0xac, &&g0xad, &&g0xae, &&g0xaf,
		&&g0xb0, &&g0xb1, &&g0xb2, &&g0xb3, &&g0xb4, &&g0xb5, &&g0xb6, &&g0xb7, &&g0xb8, &&g0xb9, &&g0xba, &&g0xbb, &&g0xbc, &&g0xbd, &&g0xbe, &&g0xbf,
		&&g0xc0, &&g0xc1, &&g0xc2, &&g0xc3, &&g0xc4, &&g0xc5, &&g0xc6, &&g0xc7, &&g0xc8, &&g0xc9, &&g0xca, &&g0xcb, &&g0xcc, &&g0xcd, &&g0xce, &&g0xcf,
		&&g0xd0, &&g0xd1, &&g0xd2, &&g0xd3, &&g0xd4, &&g0xd5, &&g0xd6, &&g0xd7, &&g0xd8, &&g0xd9, &&g0xda, &&g0xdb, &&g0xdc, &&g0xdd, &&g0xde, &&g0xdf,
		&&g0xe0, &&g0xe1, &&g0xe2, &&g0xe3, &&g0xe4, &&g0xe5, &&g0xe6, &&g0xe7, &&g0xe8, &&g0xe9, &&g0xea, &&g0xeb, &&g0xec, &&g0xed, &&g0xee, &&g0xef,
		&&g0xf0, &&g0xf1, &&g0xf2, &&g0xf3, &&g0xf4, &&g0xf5, &&g0xf6, &&g0xf7, &&g0xf8, &&g0xf9, &&g0xfa, &&g0xfb, &&g0xfc, &&g0xfd, &&g0xfe, &&g0xff
	};

   
    if (CLC1CONLbits.LCOUT)
    {
        EMULATE_DONE
    }

	goto *romData[LO_ADDRESS];

gstore:
	*r_storeAddress = LO_ADDRESS;
	EMULATE_DONE

g0x00:
	SET_DATA(0x78); // sei
	EMULATE_DONE

g0x01:
	SET_DATA(0xd8); // cld
	EMULATE_DONE

g0x02:
	SET_DATA(0xa2); // ldx #$FF
	EMULATE_DONE

g0x03:
	SET_DATA(0xff);
	EMULATE_DONE

g0x04:
	SET_DATA(0x9a); // txs
	EMULATE_DONE

g0x05:
	SET_DATA(0x85); // sta WSYNC
	EMULATE_DONE

g0x06:
	SET_DATA(0x02);
	EMULATE_DONE

g0x07:
	SET_DATA(0xa9); // lda #0	//zero memory
	EMULATE_DONE

g0x08:
	SET_DATA(0x00);
	EMULATE_DONE

g0x09:
	SET_DATA(0x95); // sta 0,X	// ClearMem
	EMULATE_DONE

g0x0a:
	SET_DATA(0x00);
	EMULATE_DONE

g0x0b:
	SET_DATA(0xca); // dex
	EMULATE_DONE

g0x0c:
	SET_DATA(0xd0); // bne ClearMem
	EMULATE_DONE

g0x0d:
	SET_DATA(0xfb);
	EMULATE_DONE

g0x0e:
	SET_DATA(0xa9); // lda #1
	EMULATE_DONE

g0x0f:
	SET_DATA(0x01);
	EMULATE_DONE

g0x10:
	SET_DATA(0x85); // sta VDELP1
	EMULATE_DONE

g0x11:
	SET_DATA(0x26);
	EMULATE_DONE

g0x12:
	SET_DATA(0xa9); // lda #$CF
	EMULATE_DONE

g0x13:
	SET_DATA(0xcf);
	EMULATE_DONE

g0x14:
	SET_DATA(0x85); // sta PF0
	EMULATE_DONE

g0x15:
	SET_DATA(0x0d);
	EMULATE_DONE

g0x16:
	SET_DATA(0xa9); // lda #$33
	EMULATE_DONE

g0x17:
	SET_DATA(0x33);
	EMULATE_DONE

g0x18:
	SET_DATA(0x85); // sta PF1
	EMULATE_DONE

g0x19:
	SET_DATA(0x0e);
	EMULATE_DONE

g0x1a:
	SET_DATA(0xa9); // lda #$CC
	EMULATE_DONE

g0x1b:
	SET_DATA(0xcc);
	EMULATE_DONE

g0x1c:
	SET_DATA(0x85); //sta PF2
	EMULATE_DONE

g0x1d:
	SET_DATA(0x0f);
	EMULATE_DONE

g0x1e:
	SET_DATA(0xea); // nop
	EMULATE_DONE

g0x1f:
	SET_DATA(0x85); // sta RESP0
	EMULATE_DONE


g0x20:
	SET_DATA(0x10);
	EMULATE_DONE

g0x21:
	SET_DATA(0xea); // nop
	EMULATE_DONE

g0x22:
	SET_DATA(0x85); //sta RESP1
	EMULATE_DONE

g0x23:
	SET_DATA(0x11);
	EMULATE_DONE

g0x24:
	SET_DATA(0xa9); // lda #$06	//3 copies medium
	EMULATE_DONE

g0x25:
	SET_DATA(0x06); // lda #$06	//3 copies medium
	EMULATE_DONE

g0x26:
	SET_DATA(0x85); // sta NUSIZ0
	EMULATE_DONE

g0x27:
	SET_DATA(0x04);
	EMULATE_DONE

g0x28:
	SET_DATA(0xa9); // lda #$02	//2 copies medium
	EMULATE_DONE

g0x29:
	SET_DATA(0x02);
	EMULATE_DONE

g0x2a:
	SET_DATA(0x85); // sta NUSIZ1
	EMULATE_DONE

g0x2b:
	SET_DATA(0x05);
	EMULATE_DONE

g0x2c:
	SET_DATA(0xa9); // lda #$10
	EMULATE_DONE

g0x2d:
	SET_DATA(0x10);
	EMULATE_DONE

g0x2e:
	SET_DATA(0x85); // sta HMP0
	EMULATE_DONE

g0x2f:
	SET_DATA(0x20);
	EMULATE_DONE


g0x30:
	SET_DATA(0xa9); // lda #$00
	EMULATE_DONE

g0x31:
	SET_DATA(0x00);
	EMULATE_DONE

g0x32:
	SET_DATA(0x85); // sta HMP1
	EMULATE_DONE

g0x33:
	SET_DATA(0x21);
	EMULATE_DONE

g0x34:
	SET_DATA(0x85); // sta HMOVE
	EMULATE_DONE

g0x35:
	SET_DATA(0x2a);
	EMULATE_DONE

g0x36:
	SET_DATA(0xa2); // ldx #15
	EMULATE_DONE

g0x37:
	SET_DATA(0x0f);
	EMULATE_DONE

g0x38:
	SET_DATA(0xca); // dex ; wait_cnt
	EMULATE_DONE

g0x39:
	SET_DATA(0xd0); // bne wait_cnt
	EMULATE_DONE

g0x3a:
	SET_DATA(0xfd);
	EMULATE_DONE

g0x3b:
	SET_DATA(0x85); // sta HMCLR
	EMULATE_DONE

g0x3c:
	SET_DATA(0x2b);
	EMULATE_DONE

g0x3d:
	SET_DATA(0xea); // nop
	EMULATE_DONE

g0x3e:
	SET_DATA(0xa9); // lda #GDATA6 	// 2	// right_line
	r_data = r_frameInfo.graphBuf[1];
	EMULATE_DONE

g0x3f:
	SET_DATA(r_data);
	EMULATE_DONE



g0x40:
	SET_DATA(0x85); // sta GRP1 	// 3 VDELed
	EMULATE_DONE

g0x41:
	SET_DATA(0x1c);
	EMULATE_DONE

g0x42:
	SET_DATA(0x85); // 2a sta HMOVE 	// 3 @03 +8 pixel
	EMULATE_DONE

g0x43:
	SET_DATA(0x2a);
	EMULATE_DONE

g0x44:
	SET_DATA(0xa9); // lda #AUD_DATA 	// 2
	r_data = r_audioPushed ? r_audioVal : *r_frameInfo.audioBuf++;
	EMULATE_DONE

g0x45:
	SET_DATA(r_data);
	r_audioPushed = false;
	EMULATE_DONE

g0x46:
	SET_DATA(0xa2);	// ldx #GDATA9 	// 2
	r_data = r_frameInfo.graphBuf[4];
	EMULATE_DONE

g0x47:
	SET_DATA(r_data);
	EMULATE_DONE

g0x48:
	SET_DATA(0x85);	// sta AUDV0 	// 3 @10
	EMULATE_DONE

g0x49:
	SET_DATA(0x19);
	EMULATE_DONE

g0x4a:
	SET_DATA(0xa0); // ldy #GCOL9 	// 2
	r_data = r_frameInfo.colorBuf[4];
	EMULATE_DONE

g0x4b:
	SET_DATA(r_data);
	EMULATE_DONE

g0x4c:
	SET_DATA(0xa9); // lda #GCOL6 	// 2
	r_data = r_frameInfo.colorBuf[1];
	EMULATE_DONE

g0x4d:
	SET_DATA(r_data);
	EMULATE_DONE

g0x4e:
	SET_DATA(0x85); // sta COLUP1 	// 3
	EMULATE_DONE

g0x4f:
	SET_DATA(0x07);
	EMULATE_DONE


g0x50:
	SET_DATA(0xa9);	// lda #GDATA5 	// 2
	r_data = r_frameInfo.graphBuf[0];
	EMULATE_DONE

g0x51:
	SET_DATA(r_data);
	EMULATE_DONE

g0x52:
	SET_DATA(0x85);	// sta GRP0 	// 3
	EMULATE_DONE

g0x53:
	SET_DATA(0x1b);
	EMULATE_DONE

g0x54:
	SET_DATA(0xa9); // lda #GCOL5 	// 2
	r_data = r_frameInfo.colorBuf[0];
	EMULATE_DONE

g0x55:
	SET_DATA(r_data);
	EMULATE_DONE

g0x56:
	SET_DATA(0x85);	// sta COLUP0 	// 3
	EMULATE_DONE

g0x57:
	SET_DATA(0x06);
	EMULATE_DONE

g0x58:
	SET_DATA(0xa9);	// lda #GDATA8 	// 2
	r_data = r_frameInfo.graphBuf[3];
	EMULATE_DONE

g0x59:
	SET_DATA(r_data);
	EMULATE_DONE

g0x5a:
	SET_DATA(0x85);	// sta GRP1 	// 3 VDELed
	EMULATE_DONE

g0x5b:
	SET_DATA(0x1c);
	EMULATE_DONE

g0x5c:
	SET_DATA(0xa9);	// lda #$00 	// 2 background color
	r_data = *r_frameInfo.colorBKBuf++;
	EMULATE_DONE

g0x5d:
	SET_DATA(r_data);
	EMULATE_DONE

g0x5e:
	SET_DATA(0x85);	// sta COLUBK 	// 3 background color
	EMULATE_DONE

g0x5f:
	SET_DATA(0x09);
	EMULATE_DONE

g0x60:
	SET_DATA(0xa9); // lda #GCOL7 	// 2
	r_data = r_frameInfo.colorBuf[2];
	EMULATE_DONE

g0x61:
	SET_DATA(r_data);
	EMULATE_DONE

g0x62:
	SET_DATA(0x85);	// sta COLUP0 	// 3 @42! end of GRP0a display
	EMULATE_DONE

g0x63:
	SET_DATA(0x06);
	EMULATE_DONE

g0x64:
	SET_DATA(0xa9);	// lda #GDATA7 	// 2
	r_data = r_frameInfo.graphBuf[2];
	EMULATE_DONE

g0x65:
	SET_DATA(r_data);
	EMULATE_DONE

g0x66:
	SET_DATA(0x85);	// sta GRP0 	// 3 @47! end of GRP1a display
	EMULATE_DONE

g0x67:
	SET_DATA(0x1b);
	EMULATE_DONE

g0x68:
	SET_DATA(0xa9); // lda #GCOL8 	// 2
	r_data = r_frameInfo.colorBuf[3];
	EMULATE_DONE

g0x69:
	SET_DATA(r_data);
	EMULATE_DONE

g0x6a:
	SET_DATA(0x85);	// sta COLUP1 	// 3 @52
	EMULATE_DONE

g0x6b:
	SET_DATA(0x07);
	EMULATE_DONE

g0x6c:
	SET_DATA(0x86);	// stx GRP0 	// 3 @55
	EMULATE_DONE

g0x6d:
	SET_DATA(0x1b);
	EMULATE_DONE

g0x6e:
	SET_DATA(0x84);	// sty COLUP0 	// 3 @58<=@60
	EMULATE_DONE

g0x6f:
	SET_DATA(0x06);
	EMULATE_DONE


g0x70:
	SET_DATA(0xa9);	// lda #$00 	// 2 turn off background color
	EMULATE_DONE

g0x71:
	SET_DATA(0x00);
	EMULATE_DONE

g0x72:
	SET_DATA(0x85);	// 09 sta COLUBK 	// 3 background color
	EMULATE_DONE

g0x73:
	SET_DATA(0x09);
	EMULATE_DONE

g0x74:
	SET_DATA(0x85);	// sta HMCLR 	// 3
	EMULATE_DONE

g0x75:
	SET_DATA(0x2b);
	EMULATE_DONE

g0x76:
	SET_DATA(0xa9);	// lda #00 	// 2 dummy	// left_line
	EMULATE_DONE

g0x77:
	SET_DATA(0x00);
	EMULATE_DONE

g0x78:
	SET_DATA(0xa9);	// lda #GDATA1 	// 2
	r_data = r_frameInfo.graphBuf[6];
	EMULATE_DONE

g0x79:
	SET_DATA(r_data);
	EMULATE_DONE

g0x7a:
	SET_DATA(0x85);	// sta HMOVE	//back 8, late hmove 	//needs to be on cycle 71
	EMULATE_DONE

g0x7b:
	SET_DATA(0x2a);
	EMULATE_DONE

g0x7c:
	SET_DATA(0x85);	// sta GRP1 	// 3 VDELed
	EMULATE_DONE

g0x7d:
	SET_DATA(0x1c);
	EMULATE_DONE

g0x7e:
	SET_DATA(0xa9);	// lda #GCOL1 	// 2
	r_data = r_frameInfo.colorBuf[6];
	EMULATE_DONE

g0x7f:
	SET_DATA(r_data);
	EMULATE_DONE


g0x80:
	SET_DATA(0x85);	// sta COLUP1 	// 3
	EMULATE_DONE

g0x81:
	SET_DATA(0x07);
	EMULATE_DONE

g0x82:
	SET_DATA(0xa9); // lda #AUD_DATA 	// 2
	r_data = *r_frameInfo.audioBuf++;
	EMULATE_DONE

g0x83:
	SET_DATA(r_data);
	EMULATE_DONE

g0x84:
	SET_DATA(0x85);	// sta AUDV0 	// 3 @10
	EMULATE_DONE

g0x85:
	SET_DATA(0x19);
	EMULATE_DONE

g0x86:
	SET_DATA(0xa2); // ldx #GDATA4 	// 2
	r_data = r_frameInfo.graphBuf[9];
	EMULATE_DONE

g0x87:
	SET_DATA(r_data);
	EMULATE_DONE

g0x88:
	SET_DATA(0xa0); // ldy #GCOL4 	// 2
	r_data = r_frameInfo.colorBuf[9];
	EMULATE_DONE

g0x89:
	SET_DATA(r_data);
	EMULATE_DONE

g0x8a:
	SET_DATA(0xa9); // lda #GDATA0 	// 2
	r_data = r_frameInfo.graphBuf[5];
	EMULATE_DONE

g0x8b:
	SET_DATA(r_data);
	EMULATE_DONE

g0x8c:
	SET_DATA(0x85); // sta GRP0 	// 3
	EMULATE_DONE

g0x8d:
	SET_DATA(0x1b);
	EMULATE_DONE

g0x8e:
	SET_DATA(0xa9); // lda #GCOL0 	// 2
	r_data = r_frameInfo.colorBuf[5];
	EMULATE_DONE

g0x8f:
	SET_DATA(r_data);
	EMULATE_DONE

g0x90:
	SET_DATA(0x85);	// sta COLUP0 	// 3
	EMULATE_DONE

g0x91:
	SET_DATA(0x06);
	EMULATE_DONE

g0x92:
	SET_DATA(0xa9); // lda #GDATA3 	// 2
	r_data = r_frameInfo.graphBuf[8];
	EMULATE_DONE

g0x93:
	SET_DATA(r_data);
	EMULATE_DONE

g0x94:
	SET_DATA(0x85);	// sta GRP1 	// 3 VDELed
	EMULATE_DONE

g0x95:
	SET_DATA(0x1c);
	EMULATE_DONE

g0x96:
	SET_DATA(0xa9);	// lda #$00 	// 2 playfield color
	r_data = *r_frameInfo.colorBKBuf++;
	EMULATE_DONE

g0x97:
	SET_DATA(r_data);
	EMULATE_DONE

g0x98:
	SET_DATA(0x85);	// sta COLUPF 	// 3 playfield color
	EMULATE_DONE

g0x99:
	SET_DATA(0x08);
	EMULATE_DONE

g0x9a:
	SET_DATA(0xa9); // lda #GCOL2 	// 2
	r_data = r_frameInfo.colorBuf[7];
	EMULATE_DONE

g0x9b:
	SET_DATA(r_data);
	EMULATE_DONE

g0x9c:
	SET_DATA(0x85);	// sta COLUP0 	// 3 @39! end of GRP0a display
	EMULATE_DONE

g0x9d:
	SET_DATA(0x06);
	EMULATE_DONE

g0x9e:
	SET_DATA(0xa9); // lda #GDATA2 	// 2
	r_data = r_frameInfo.graphBuf[7];
	EMULATE_DONE

g0x9f:
	SET_DATA(r_data);
	EMULATE_DONE

g0xa0:
	SET_DATA(0x85);	// sta GRP0 	// 3 @44! end of GRP1a display
	EMULATE_DONE

g0xa1:
	SET_DATA(0x1b);
	EMULATE_DONE

g0xa2:
	SET_DATA(0xa9); // lda #GCOL3 	// 2
	r_data = r_frameInfo.colorBuf[8];
	EMULATE_DONE

g0xa3:
	SET_DATA(r_data);
	EMULATE_DONE

g0xa4:
	SET_DATA(0x85);	// sta COLUP1 	// 3 @49
	EMULATE_DONE

g0xa5:
	SET_DATA(0x07);
	EMULATE_DONE

g0xa6:
	SET_DATA(0x86);	// stx GRP0 	// 3 @52
	EMULATE_DONE

g0xa7:
	SET_DATA(0x1b);
	EMULATE_DONE

g0xa8:
	SET_DATA(0x84);	// sty COLUP0 	// 3 @55<=@57
	EMULATE_DONE

g0xa9:
	SET_DATA(0x06);
	EMULATE_DONE

g0xaa:
	SET_DATA(0xa9);	// lda #$00 	// 2 turn off playfield
	EMULATE_DONE

g0xab:
	SET_DATA(0x00);
	EMULATE_DONE

g0xac:
	SET_DATA(0x85);	// sta COLUPF 	// 3
	EMULATE_DONE

g0xad:
	SET_DATA(0x08);
	EMULATE_DONE

g0xae:
	SET_DATA(0xa9);	// lda #$80 	// 2
	EMULATE_DONE

g0xaf:
	SET_DATA(0x80);	// lda #$80 	// 2
	r_lines -= 2;
	EMULATE_DONE

g0xb0:
	SET_DATA(0x85);	// sta HMP0 	// 3
	if (r_lines == 0)
	{
		r_nextLineJump = ADDR_END_LINES;

		if (!r_frameInfo.odd)
			r_lines = r_frameInfo.overscanLines;
		else
			r_lines = r_frameInfo.overscanLines-1;

		r_vblankState = ST_ON;
	}
	else
	{
		r_frameInfo.graphBuf += 10;
		r_frameInfo.colorBuf += 10;
	}
	EMULATE_DONE

g0xb1:
	SET_DATA(0x20);
	EMULATE_DONE

g0xb2:
	SET_DATA(0x85);	// sta HMP1 	// 3 @63
	EMULATE_DONE

g0xb3:
	SET_DATA(0x21);
	EMULATE_DONE

g0xb4:
	SET_DATA(0x4c); // jmp right_line(3e) / end_lines(b7)
	EMULATE_DONE

g0xb5:
	SET_DATA(r_nextLineJump);
	EMULATE_DONE

g0xb6:
	SET_DATA(0xff);
	EMULATE_DONE

g0xb7:
	SET_DATA(0xa0);	// ldy #2	// end_lines 	//@213	// end of current line
	EMULATE_DONE

g0xb8:
	SET_DATA(0x02);
	EMULATE_DONE

g0xb9:
	SET_DATA(0xa2);
	EMULATE_DONE	// ldx #0

g0xba:
	SET_DATA(0x00);
	EMULATE_DONE

g0xbb:
	SET_DATA(r_vsyncState);
	EMULATE_DONE	// stx VSYNC	// beginning of new line

g0xbc:
	SET_DATA(0x00);
	EMULATE_DONE

g0xbd:
	SET_DATA(r_vblankState);
	EMULATE_DONE	// stx VBLANK

g0xbe:
	SET_DATA(0x01);
	EMULATE_DONE

g0xbf:
	SET_DATA(0xa9); // lda #AUD_DATA 	// 2
	r_data = *r_frameInfo.audioBuf++;
	EMULATE_DONE


g0xc0:
	SET_DATA(r_data);
	EMULATE_DONE

g0xc1:
	SET_DATA(0x85); // sta AUDV0 	// 3 @10
	EMULATE_DONE

g0xc2:
	SET_DATA(0x19);
	EMULATE_DONE

g0xc3:
	SET_DATA(0xae); // ldx SWCHA 	// 4 ad 80 02 0x280 RLDU(1) RLDU(2) 	// 1111 1111 by default
	EMULATE_DONE

g0xc4:
	SET_DATA(0x80);
	EMULATE_DONE

g0xc5:
	SET_DATA(0x02);
	EMULATE_DONE

g0xc6:
	SET_DATA(0x9d);  // sta   $FE00,x //	 5  first 256 bytes
	EMULATE_DONE

g0xc7:
	SET_DATA(0x00);
	EMULATE_DONE

g0xc8:
	SET_DATA(0xfe);
	r_storeAddress = &mr_swcha;
	EMULATE_DONE

g0xc9:
	SET_DATA(0xae); // ldx SWCHB 	// 4 ad 82 02 0x282 a b - - color - select reset
	EMULATE_DONE

g0xca:
	SET_DATA(0x82);
	EMULATE_DONE

g0xcb:
	SET_DATA(0x02);
	EMULATE_DONE

g0xcc:
	SET_DATA(0x9d);  // sta   $FE00,x //	 5  first 256 bytes
	EMULATE_DONE

g0xcd:
	SET_DATA(0x00);
	EMULATE_DONE

g0xce:
	SET_DATA(0xfe);
	r_storeAddress = &mr_swchb;
	EMULATE_DONE

g0xcf:
	SET_DATA(0xa6); // ldx INPT4 	// 3 a5 0c 0x0c button - - - - - - -
	EMULATE_DONE


g0xd0:
	SET_DATA(0x0c);
	EMULATE_DONE

g0xd1:
	SET_DATA(0x9d);  // sta   $FE00,x //	 5  first 256 bytes
	EMULATE_DONE

g0xd2:
	SET_DATA(0x00);
	EMULATE_DONE

g0xd3:
	SET_DATA(0xfe);
	r_storeAddress = &mr_inpt4;
	EMULATE_DONE

g0xd4:
	SET_DATA(0xa6); // ldx INPT5 	// 3 a5 0d 0x0d button - - - - - - -
	EMULATE_DONE

g0xd5:
	SET_DATA(0x0d);
	EMULATE_DONE

g0xd6:
	SET_DATA(0x9d);  // sta   $FE00,x //	 5  first 256 bytes
	EMULATE_DONE

g0xd7:
	SET_DATA(0x00);
	EMULATE_DONE

g0xd8:
	SET_DATA(0xfe);
	r_storeAddress = &mr_inpt5;
	EMULATE_DONE

g0xd9:
	SET_DATA(0xa2); // ldx   #0 
	r_lines--;
	if (r_lines == 0)
	{
		switch (r_endState)
		{
			case 0:
				TESTA1_HIGH
				r_endState++;
				r_lines = r_frameInfo.vsyncLines;
				r_vsyncState = ST_ON;

				EMULATE_DONE

			case 1:
				r_endState++;
				if (!r_frameInfo.odd)
					r_lines = r_frameInfo.blankLines+1;
				else
					r_lines = r_frameInfo.blankLines;

				r_vsyncState = ST_OFF;
				EMULATE_DONE

			case 2:
				r_endState++;
				r_vblankState = ST_OFF;
				r_nextLineJump = ADDR_RIGHT_LINE;
				EMULATE_DONE
		}
	}
	EMULATE_DONE

g0xda:
	SET_DATA(0x00);
	if (r_endState == 3)
	{
		if (r_frameInfo.odd)
		{
			r_audioVal = *r_frameInfo.audioBuf++;
			r_audioPushed = true;
		}
	}
	EMULATE_DONE

g0xdb:
	SET_DATA(0xea); // nop
	if (r_endState == 3)
	{
		if (mr_bufferIndex == 0)
		{
			r_frameInfo.colorBuf = mr_frameInfo2.colorBuf;
			r_frameInfo.colorBKBuf = mr_frameInfo2.colorBKBuf;
		}
		else
		{
			r_frameInfo.colorBuf = mr_frameInfo1.colorBuf;
			r_frameInfo.colorBKBuf = mr_frameInfo1.colorBKBuf;
		}
	}
	EMULATE_DONE

g0xdc:
	SET_DATA(0xea); // nop
	if (r_endState == 3)
	{
		if (mr_bufferIndex == 0)
		{
			r_frameInfo.audioBuf = mr_frameInfo2.audioBuf;
			r_frameInfo.graphBuf = mr_frameInfo2.graphBuf;
		}
		else
		{
			r_frameInfo.audioBuf = mr_frameInfo1.audioBuf;
			r_frameInfo.graphBuf = mr_frameInfo1.graphBuf;
		}
	}
	EMULATE_DONE

g0xdd:
	SET_DATA(0xea); // nop
	if (r_endState == 3)
	{
		if (mr_bufferIndex == 0)
		{
			r_frameInfo.visibleLines = mr_frameInfo2.visibleLines;
			r_frameInfo.overscanLines = mr_frameInfo2.overscanLines;
		}
		else
		{
			r_frameInfo.visibleLines = mr_frameInfo1.visibleLines;
			r_frameInfo.overscanLines = mr_frameInfo1.overscanLines;
		}
	}
	EMULATE_DONE

g0xde:
	SET_DATA(0xea); // nop
	if (r_endState == 3)
	{
		if (mr_bufferIndex == 0)
		{
			r_frameInfo.vsyncLines = mr_frameInfo2.vsyncLines;
			r_frameInfo.blankLines = mr_frameInfo2.blankLines;
		}
		else
		{
			r_frameInfo.vsyncLines = mr_frameInfo1.vsyncLines;
			r_frameInfo.blankLines = mr_frameInfo1.blankLines;
		}
	}
	EMULATE_DONE

g0xdf:
	SET_DATA(0xea); // nop
	if (r_endState == 3)
	{
		if (mr_bufferIndex == 0)
		{
			r_frameInfo.odd = mr_frameInfo2.odd;
		}
		else
		{
			r_frameInfo.odd = mr_frameInfo1.odd;
		}
	}
	EMULATE_DONE	// nop

g0xe0:
	SET_DATA(0xea); // nop
	if (r_endState == 3)
	{
		mr_bufferIndex = !mr_bufferIndex;
	}
	EMULATE_DONE	// nop

g0xe1:
	SET_DATA(0xea); // nop
	EMULATE_DONE	// nop

g0xe2:
	SET_DATA(0xea); // nop
	EMULATE_DONE	// nop

g0xe3:
	SET_DATA(r_vsyncState);	// stx VSYNC
	if (r_endState == 3)
	{
		r_lines = r_frameInfo.visibleLines;

		r_endState = 0;
		mr_endFrame = true;
		TESTA1_LOW
	}
	EMULATE_DONE

g0xe4:
	SET_DATA(0x00);
	EMULATE_DONE

g0xe5:
	SET_DATA(r_vblankState);	// stx VBLANK
	EMULATE_DONE

g0xe6:
	SET_DATA(0x01);
	EMULATE_DONE

g0xe7:
	SET_DATA(0x4c);	// jmp
	EMULATE_DONE

g0xe8:
	SET_DATA(r_nextLineJump);	//	 end_lines (b7)  right_line(3e)
	EMULATE_DONE

g0xe9:
	SET_DATA(0xff);
	EMULATE_DONE

g0xea:
	SET_DATA(0x00); // invalid
	EMULATE_DONE

g0xeb:
	SET_DATA(0x00); // invalid
	EMULATE_DONE

g0xec:
	SET_DATA(0x00); // invalid
	EMULATE_DONE

g0xed:
	SET_DATA(0x00); // invalid
	EMULATE_DONE

g0xee:
	SET_DATA(0x00); // invalid
	EMULATE_DONE

g0xef:
	SET_DATA(0x00); // invalid
	EMULATE_DONE

g0xf0:
	SET_DATA(0x00); // break
	EMULATE_DONE

g0xf1:
	SET_DATA(0x00); // invalid
	EMULATE_DONE

g0xf2:
	SET_DATA(0x00); // invalid
	EMULATE_DONE

g0xf3:
	SET_DATA(0x00); // invalid
	EMULATE_DONE

g0xf4:
	SET_DATA(0x00); // invalid
	EMULATE_DONE

g0xf5:
	SET_DATA(0x00); // invalid
	EMULATE_DONE

g0xf6:
	SET_DATA(0x00); // invalid
	EMULATE_DONE

g0xf7:
	SET_DATA(0x00); // invalid
	EMULATE_DONE

g0xf8:
	SET_DATA(0x00); // invalid
	EMULATE_DONE

g0xf9:
	SET_DATA(0x00); // invalid
	EMULATE_DONE

   // break a number of times to make sure the system is actually stable

g0xfa:
	if (r_breakLoops)
	{
		SET_DATA(0xf0); // .word.w main_start	// NMI
		r_breakLoops--;
	}
	else
	{
		SET_DATA(0x00); // .word.w main_start	// NMI
	}
	EMULATE_DONE

g0xfb:
	SET_DATA(0xff);
	EMULATE_DONE

g0xfc:
	if (r_breakLoops)
	{
		SET_DATA(0xf0); // .word.w main_start	// RESET
		r_breakLoops--;
	}
	else
	{
		SET_DATA(0x00); // .word.w main_start	// RESET
	}
	EMULATE_DONE

g0xfd:
	SET_DATA(0xff);
	EMULATE_DONE

g0xfe:
	if (r_breakLoops)
	{
		SET_DATA(0xf0); // .word.w main_start	// IRQ/BRK
		r_breakLoops--;
	}
	else
	{
		SET_DATA(0x00); // .word.w main_start	// IRQ/BRK
	}
	EMULATE_DONE

g0xff:
	SET_DATA(0xff);
	EMULATE_DONE

}
