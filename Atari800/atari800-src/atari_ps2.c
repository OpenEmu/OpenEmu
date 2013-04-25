/*
 * atari_ps2.c - Sony PlayStation 2 port code
 *
 * Copyright (c) 2005 Troy Ayers and Piotr Fusik
 * Copyright (c) 2005 Atari800 development team (see DOC/CREDITS)
 *
 * This file is part of the Atari800 emulator project which emulates
 * the Atari 400, 800, 800XL, 130XE, and 5200 8-bit computers.
 *
 * Atari800 is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * Atari800 is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Atari800; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
 */
			//TODO: map the following keys
			//control <>: (suits)
			//akey_clear
			//AKEY_BREAK
			//all ALT+key shortcut keys
			//Atari OPTION key (temple of apshai, others?)
			//keyboard repeat (when shift nor control pressed)
			//mouse support
			//autodetect pal vs ntsc
			//allow user to map own keyboard keys to controller
			//enable cdfs, hdd, mass, and host support.

#include "config.h"
#include <stdio.h>
#include <string.h>
#include <tamtypes.h>
#include <loadfile.h>
#include <fileio.h>
#include <sifcmd.h>
#include <sifrpc.h>
#include <kernel.h>
#include <debug.h>
#include <libmc.h>
#include <libkbd.h>
#include <libpad.h>
#include <gsKit.h>
#include <dmaKit.h>

#include "atari.h"
#include "colours.h"
#include "input.h"
#include "akey.h"
#include "log.h"
#include "monitor.h"
#include "screen.h"
#include "ui.h"
#include "util.h"

//#ifdef SOUND
#include <audsrv.h>
#include "pokeysnd.h"
#include "sound.h"
extern unsigned char audsrv[];
extern unsigned int size_audsrv;
//#endif

// define to use T0 and T1 timers
#define USE_TIMERS
extern unsigned char usbd[];
extern unsigned int size_usbd;

extern unsigned char ps2kbd[];
extern unsigned int size_ps2kbd;

static GSGLOBAL *gsGlobal = NULL;

static int clut[256] __attribute__((aligned(16)));

//int PS2KbdCAPS = 0;
int PS2KbdSHIFT = 0;
int PS2KbdCONTROL = 0;
int PS2KbdALT = 0;

#define PAD_PORT 0
#define PAD_SLOT 0

static char padBuf[256] __attribute__((aligned(64)));

#ifdef USE_TIMERS

/* We use T0 for Atari_time() and T1 for Atari_sleep().
   Note that both timers are just 16-bit. */

#define T0_COUNT  (*(volatile unsigned long *) 0x10000000)
#define T0_MODE   (*(volatile unsigned long *) 0x10000010)
#define T0_COMP   (*(volatile unsigned long *) 0x10000020)
#define T1_COUNT  (*(volatile unsigned long *) 0x10000800)
#define T1_MODE   (*(volatile unsigned long *) 0x10000810)
#define T1_COMP   (*(volatile unsigned long *) 0x10000820)

#define INTC_TIM0 9
#define INTC_TIM1 10

static int t0_interrupt_id = -1;
static int t1_interrupt_id = -1;

/* The range of int is enough for about 7 years
   of continuous running. */
static volatile int timer_interrupt_ticks = 0;

static int sleeping_thread_id = 0;

static int t0_interrupt_handler(int ca)
{
	timer_interrupt_ticks++;
	T0_MODE |= 0x800; // clear overflow status
	// __asm__ volatile("sync.l; ei"); // XXX: necessary?
	return -1; // XXX: or 0? what does it mean?
}

static int t1_interrupt_handler(int ca)
{
	iWakeupThread(sleeping_thread_id);
	T1_MODE = 0; // disable
	// __asm__ volatile("sync.l; ei"); // XXX: necessary?
	return -1; // XXX: or 0? what does it mean?
}

static void timer_initialize(void)
{
	T0_MODE = 0; // disable
	t0_interrupt_id = AddIntcHandler(INTC_TIM0, t0_interrupt_handler, 0);
	EnableIntc(INTC_TIM0);
	T0_COUNT = 0;
	T0_MODE = 0x002  // 576000 Hz clock
			+ 0x080  // start counting
			+ 0x200; // generate interrupt on overflow

	T1_MODE = 0; // disable
	t1_interrupt_id = AddIntcHandler(INTC_TIM1, t1_interrupt_handler, 0);
	EnableIntc(INTC_TIM1);
}

static void timer_shutdown(void)
{
	T0_MODE = 0;
	if (t0_interrupt_id >= 0) {
		DisableIntc(INTC_TIM0);
		RemoveIntcHandler(INTC_TIM0, t0_interrupt_id);
		t0_interrupt_id = -1;
	}
	T1_MODE = 0;
	if (t1_interrupt_id >= 0) {
		DisableIntc(INTC_TIM1);
		RemoveIntcHandler(INTC_TIM1, t1_interrupt_id);
		t1_interrupt_id = -1;
	}
}

#endif /* USE_TIMERS */

double Atari_time(void)
{
#ifdef USE_TIMERS
	/* AFAIK, multiplication is faster than division,
	   on every CPU architecture */
	return (timer_interrupt_ticks * 65536.0 + T0_COUNT) * (1.0 / 576000);
#else
	static double fake_timer = 0;
	return fake_timer++;
#endif
}

/* this Atari_sleep() supports times only up to 0.11 sec,
   which is enough for Atari800 purposes */

/* void Atari_sleep(double s)
{
#ifdef USE_TIMERS
	unsigned long count = 65536 - (unsigned long) (s * 576000);
	// do nothing if s is less than one T1 tick
	if (count >= 65536)
		return;
	sleeping_thread_id = GetThreadId();
	T1_COUNT = count;
	T1_MODE = 0x002  // 576000 Hz clock
			+ 0x080  // start counting
			+ 0x200; // generate interrupt on overflow
//	SleepThread();
#endif
}
*/
//volatile int locked = 1;

//void wakeup(s32 id, u16 time, void *arg)
//{
//        locked = 0;
//}
//
//void Atari_sleep(double s)
//{
//
//        /* 15734 is around 1 second on NTSC */
//	/* is about 1ms? */
//        SetAlarm(15734 * s, wakeup, 0);
//        while (locked);
//	locked = 1;
//}
void Atari_sleep(double s)
{

	if (UI_is_active){
	        int i,ret;
	        for (i=0;i<s * 100.0;i++){
	
			ee_sema_t sema;
	                sema.attr = 0;
	                sema.count = 0;
	                sema.init_count = 0;
	                sema.max_count = 1;
	                ret = CreateSema(&sema);
	                if (ret <= 0) {
	                        //could not create sema, strange!  continue anyway.
	                        return;
	                }
	
	                iSignalSema(ret);
	                WaitSema(ret);
	                DeleteSema(ret);
	        }
	}
}


void loadModules(void)
{
	int ret;
	//init_scr();

	ret = SifLoadModule("rom0:SIO2MAN", 0, NULL);
	if (ret < 0) {
		Log_print("Sio2man loading failed: %d", ret);
		SleepThread();
	}

//	Log_print("mcman");
	SifLoadModule("rom0:MCMAN", 0, NULL);

//	Log_print("mcserv");
	SifLoadModule("rom0:MCSERV", 0, NULL);

//	Log_print("padman");
	ret = SifLoadModule("rom0:PADMAN", 0, NULL);
	if (ret < 0) {
		Log_print("Padman loading failed: %d", ret);
		SleepThread();
	}

	mcInit(MC_TYPE_MC);

//	cdinit(1);
	SifInitRpc(0);

	SifExecModuleBuffer(usbd, size_usbd, 0, NULL, &ret);
	SifExecModuleBuffer(ps2kbd, size_ps2kbd, 0, NULL, &ret);

	if (PS2KbdInit() == 0) {
		Log_print("Failed to Init Keyboard.");
	}
	PS2KbdSetReadmode(PS2KBD_READMODE_RAW);

#ifdef SOUND
	ret = SifLoadModule("rom0:LIBSD", 0, NULL);

	ret = SifExecModuleBuffer(audsrv, size_audsrv, 0, NULL, &ret);
#endif

}

int PLATFORM_Initialise(int *argc, char *argv[])
{
	// Swap Red and Blue components
	int i;
	for (i = 0; i < 256; i++) {
		int c = Colours_table[i];
//		clut[i] = (c >> 16) + (c & 0xff00) + ((c & 0xff) << 16);
		// swap bits 3 and 4 to workaround a bug in gsKit
		clut[(i ^ i * 2) & 16 ? i ^ 24 : i] = (c >> 16) + (c & 0xff00) + ((c & 0xff) << 16);
	}
	// Clear debug from screen
	init_scr();
	// Initialize graphics
	gsGlobal = gsKit_init_global(GS_MODE_NTSC);
	dmaKit_init(D_CTRL_RELE_ON, D_CTRL_MFD_OFF, D_CTRL_STS_UNSPEC,
	            D_CTRL_STD_OFF, D_CTRL_RCYC_8);
	dmaKit_chan_init(DMA_CHANNEL_GIF);
	gsGlobal->PSM = GS_PSM_CT32;
	gsGlobal->Test->ZTE = 0;
	gsKit_init_screen(gsGlobal);
	// Init joypad
	padInit(0);
	padPortOpen(PAD_PORT, PAD_SLOT, padBuf);
#ifdef USE_TIMERS
	timer_initialize();
#endif
#ifdef SOUND
	if (!Sound_Initialise(argc, argv))
		return FALSE;
#endif

	return TRUE;
}

int PLATFORM_Exit(int run_monitor)
{
	// TODO: shutdown graphics mode
	Log_flushlog();
#if 0
	if (run_monitor && MONITOR_Run()) {
		// TODO: reinitialize graphics mode
		return TRUE;
	}
#endif
#ifdef USE_TIMERS
	timer_shutdown();
#endif
#ifdef SOUND
	Sound_Exit();
#endif
//zzz temp exit procedure
//Hard coded to go back to ulaunch
	fioExit();
	SifExitRpc();
	LoadExecPS2("mc0:/BOOT/BOOT.ELF", 0, NULL);
//zzz end
	return FALSE;
}

void PLATFORM_DisplayScreen(void)
{
	GSTEXTURE tex;
	tex.Width = Screen_WIDTH;
	tex.Height = Screen_HEIGHT;
	tex.PSM = GS_PSM_T8;
	tex.Mem = (UBYTE *) Screen_atari;
	tex.Clut = clut;
	tex.Vram = 0x200000;
	tex.VramClut = 0x280000;
	tex.Filter = GS_FILTER_LINEAR;
	// TODO: upload clut just once
	gsKit_texture_upload(gsGlobal, &tex);
	gsKit_prim_sprite_texture(gsGlobal, &tex, 0, 0, 32, 0, 640, 480, 32 + 320, 240, 0, 0x80808080);
#if 0
	gsKit_sync_flip(gsGlobal);
#else
	// flip without vsync
	// this is a copy of gsKit_sync_flip() code with just gsKit_vsync() call removed
	GS_SET_DISPFB2(gsGlobal->ScreenBuffer[gsGlobal->ActiveBuffer & 1] / 8192,
		gsGlobal->Width / 64, gsGlobal->PSM, 0, 0 );
	gsGlobal->ActiveBuffer ^= 1;
	gsGlobal->PrimContext ^= 1;
	gsGlobal->EvenOrOdd = ((GSREG *) GS_CSR)->FIELD;
	gsKit_setactive(gsGlobal);
#endif

}

static int PadButtons(void)
{
	struct padButtonStatus buttons;
	for (;;) {
		int ret = padGetState(PAD_PORT, PAD_SLOT);
		if (ret == PAD_STATE_STABLE || ret == PAD_STATE_FINDCTP1)
			break;
		if (ret == PAD_STATE_DISCONN)
			return 0;
	}
	padRead(PAD_PORT, PAD_SLOT, &buttons);
	return ~buttons.btns;
}

int PLATFORM_Keyboard(void)
{
	int new_pad = PadButtons();
	PS2KbdRawKey key;
	INPUT_key_consol = INPUT_CONSOL_NONE;

	if (UI_is_active) {
		if (new_pad & PAD_CROSS)
			return AKEY_RETURN;
		if (new_pad & PAD_CIRCLE)
			return AKEY_ESCAPE;
		if (new_pad & PAD_LEFT)
			return AKEY_LEFT;
		if (new_pad & PAD_RIGHT)
			return AKEY_RIGHT;
		if (new_pad & PAD_UP)
			return AKEY_UP;
		if (new_pad & PAD_DOWN)
			return AKEY_DOWN;
		if (new_pad & PAD_L1)
		    return AKEY_COLDSTART;
		if (new_pad & PAD_R1)
			return AKEY_WARMSTART;
	}
	//PAD_CROSS is used for PLATFORM_TRIG().
	if (new_pad & PAD_TRIANGLE)
		return AKEY_UI;
	if (new_pad & PAD_SQUARE)
		return AKEY_SPACE;
	if (new_pad & PAD_CIRCLE)
		return AKEY_RETURN;
	if (new_pad & PAD_L1)
		return AKEY_COLDSTART;
	if (new_pad & PAD_R1)
		return AKEY_WARMSTART;
	if (Atari800_machine_type == Atari800_MACHINE_5200) {
		if (new_pad & PAD_START)
			return AKEY_5200_START;
	}
	else {
		if (new_pad & PAD_START)
			INPUT_key_consol ^= INPUT_CONSOL_START;
		if (new_pad & PAD_SELECT)
			INPUT_key_consol ^= INPUT_CONSOL_SELECT;
		if (new_pad & PAD_CROSS)
			return AKEY_HELP;
	}
if (Atari800_machine_type != Atari800_MACHINE_5200 || UI_is_active) {

	while (PS2KbdReadRaw(&key) != 0) {
		if (key.state == PS2KBD_RAWKEY_DOWN) {
			switch (key.key) {
			case EOF:
				Atari800_Exit(FALSE);
				exit(0);
			    break;
			case 0x28:
				return AKEY_RETURN;
			case 0x29:
				return AKEY_ESCAPE;
			case 0x2A:
				return AKEY_BACKSPACE;
			case 0x2B:
				return AKEY_TAB;
			case 0x2C:
				return AKEY_SPACE;
			case 0x46://Print Screen Button
				return AKEY_SCREENSHOT;
			case 0x4F:
			    return AKEY_RIGHT;
			case 0x50:
			    return AKEY_LEFT;
			case 0x51:
			    return AKEY_DOWN;
			case 0x52:
			    return AKEY_UP;
			case 0x58:
			    return AKEY_RETURN;

			case 0xE0:
				PS2KbdCONTROL = 1;
				return AKEY_NONE;
			case 0xE4:
				PS2KbdCONTROL = 1;
				return AKEY_NONE;
			case 0xE1:
				PS2KbdSHIFT = 1;
				return AKEY_NONE;
			case 0xE2:
				PS2KbdALT = 1;
				return AKEY_NONE;
			case 0xE5:
				PS2KbdSHIFT = 1;
				return AKEY_NONE;
			case 0xE6:
				PS2KbdALT = 1;
				return AKEY_NONE;
			default:
				break;
			}
		}

		if ((key.state == PS2KBD_RAWKEY_DOWN) && !PS2KbdSHIFT && !PS2KbdALT) {
			switch (key.key) {
			case 0x1E:
				return AKEY_1;
			case 0X1F:
				return AKEY_2;
			case 0x20:
				return AKEY_3;
			case 0x21:
				return AKEY_4;
			case 0x22:
				return AKEY_5;
			case 0x23:
				return AKEY_6;
			case 0x24:
				return AKEY_7;
			case 0x25:
				return AKEY_8;
			case 0x26:
				return AKEY_9;
			case 0x27:
				return AKEY_0;
			case 0x2D:
				return AKEY_MINUS;
			case 0x2E:
				return AKEY_EQUAL;
			case 0x2F:
				return AKEY_BRACKETLEFT;
			case 0x30:
				return AKEY_BRACKETRIGHT;
			case 0x31:
				return AKEY_BACKSLASH;
			case 0x33:
				return AKEY_SEMICOLON;
			case 0x34:
				return AKEY_QUOTE;
			case 0x35:
				return AKEY_ATARI;
			case 0x36:
				return AKEY_COMMA;
			case 0x37:
				return AKEY_FULLSTOP;
			case 0x38:
				return AKEY_SLASH;
			case 0x3A://F1
				return AKEY_UI;
			case 0x3E://F5
				return AKEY_WARMSTART;
			case 0x42://F9
				return AKEY_EXIT;
			case 0x43://F10
				return AKEY_SCREENSHOT;
			default:
				break;
			}
		}
		if ((key.state == PS2KBD_RAWKEY_DOWN) && PS2KbdSHIFT && !PS2KbdCONTROL && !PS2KbdALT) {
			switch (key.key) {
			case 0x4:
				return AKEY_A;
			case 0x5:
				return AKEY_B;
			case 0x6:
				return AKEY_C;
			case 0x7:
				return AKEY_D;
			case 0x8:
				return AKEY_E;
			case 0x9:
				return AKEY_F;
			case 0xA:
				return AKEY_G;
			case 0xB:
				return AKEY_H;
			case 0xC:
				return AKEY_I;
			case 0xD:
				return AKEY_J;
			case 0xE:
				return AKEY_K;
			case 0xF:
				return AKEY_L;
			case 0x10:
				return AKEY_M;
			case 0x11:
				return AKEY_N;
			case 0x12:
				return AKEY_O;
			case 0x13:
				return AKEY_P;
			case 0x14:
				return AKEY_Q;
			case 0x15:
				return AKEY_R;
			case 0x16:
				return AKEY_S;
			case 0x17:
				return AKEY_T;
			case 0x18:
				return AKEY_U;
			case 0x19:
				return AKEY_V;
			case 0x1A:
				return AKEY_W;
			case 0x1B:
				return AKEY_X;
			case 0x1C:
				return AKEY_Y;
			case 0x1D:
				return AKEY_Z;
			case 0x1E:
				return AKEY_EXCLAMATION;
			case 0X1F:
				return AKEY_AT;
			case 0x20:
				return AKEY_HASH;
			case 0x21:
				return AKEY_DOLLAR;
			case 0x22:
				return AKEY_PERCENT;
			case 0x23:
//				return AKEY_CIRCUMFLEX;
				return AKEY_CARET;
			case 0x24:
				return AKEY_AMPERSAND;
			case 0x25:
				return AKEY_ASTERISK;
			case 0x26:
				return AKEY_PARENLEFT;
			case 0x27:
				return AKEY_PARENRIGHT;
			case 0x2B:
				return AKEY_SETTAB;
			case 0x2D:
				return AKEY_UNDERSCORE;
			case 0x2E:
				return AKEY_PLUS;
			case 0x31:
				return AKEY_BAR;
			case 0x33:
				return AKEY_COLON;
			case 0x34:
				return AKEY_DBLQUOTE;
			case 0x36:
				return AKEY_LESS;
			case 0x37:
				return AKEY_GREATER;
			case 0x38:
				return AKEY_QUESTION;
			case 0x3E://Shift+F5
				return AKEY_COLDSTART;
			case 0x43://Shift+F10
				return AKEY_SCREENSHOT_INTERLACE;
			case 0x49://Shift+Insert key
				return AKEY_INSERT_LINE;
			case 0x4C://Shift+Backspace Key
				return AKEY_DELETE_LINE;
			default:
				break;
			}
		}
		if ((key.state == PS2KBD_RAWKEY_DOWN) && !PS2KbdSHIFT && !PS2KbdCONTROL && !PS2KbdALT) {
			switch (key.key) {
			case 0x4:
				return AKEY_a;
			case 0x5:
				return AKEY_b;
			case 0x6:
				return AKEY_c;
			case 0x7:
				return AKEY_d;
			case 0x8:
				return AKEY_e;
			case 0x9:
				return AKEY_f;
			case 0xA:
				return AKEY_g;
			case 0xB:
				return AKEY_h;
			case 0xC:
				return AKEY_i;
			case 0xD:
				return AKEY_j;
			case 0xE:
				return AKEY_k;
			case 0xF:
				return AKEY_l;
			case 0x10:
				return AKEY_m;
			case 0x11:
				return AKEY_n;
			case 0x12:
				return AKEY_o;
			case 0x13:
				return AKEY_p;
			case 0x14:
				return AKEY_q;
			case 0x15:
				return AKEY_r;
			case 0x16:
				return AKEY_s;
			case 0x17:
				return AKEY_t;
			case 0x18:
				return AKEY_u;
			case 0x19:
				return AKEY_v;
			case 0x1A:
				return AKEY_w;
			case 0x1B:
				return AKEY_x;
			case 0x1C:
				return AKEY_y;
			case 0x1D:
				return AKEY_z;
			case 0x49:
				return AKEY_INSERT_CHAR;
			case 0x4C:
				return AKEY_DELETE_CHAR;
			default:
				break;
			}
		}
		if ((key.state == PS2KBD_RAWKEY_DOWN) && PS2KbdCONTROL && !PS2KbdALT) {
			switch(key.key) {
			case 0x4:
				return AKEY_CTRL_a;
			case 0x5:
				return AKEY_CTRL_b;
			case 0x6:
				return AKEY_CTRL_c;
			case 0x7:
				return AKEY_CTRL_d;
			case 0x8:
				return AKEY_CTRL_e;
			case 0x9:
				return AKEY_CTRL_f;
			case 0xA:
				return AKEY_CTRL_g;
			case 0xB:
				return AKEY_CTRL_h;
			case 0xC:
				return AKEY_CTRL_i;
			case 0xD:
				return AKEY_CTRL_j;
			case 0xE:
				return AKEY_CTRL_k;
			case 0xF:
				return AKEY_CTRL_l;
			case 0x10:
				return AKEY_CTRL_m;
			case 0x11:
				return AKEY_CTRL_n;
			case 0x12:
				return AKEY_CTRL_o;
			case 0x13:
				return AKEY_CTRL_p;
			case 0x14:
				return AKEY_CTRL_q;
			case 0x15:
				return AKEY_CTRL_r;
			case 0x16:
				return AKEY_CTRL_s;
			case 0x17:
				return AKEY_CTRL_t;
			case 0x18:
				return AKEY_CTRL_u;
			case 0x19:
				return AKEY_CTRL_v;
			case 0x1A:
				return AKEY_CTRL_w;
			case 0x1B:
				return AKEY_CTRL_x;
			case 0x1C:
				return AKEY_CTRL_y;
			case 0x1D:
				return AKEY_CTRL_z;
			case 0x1E:
				return AKEY_CTRL_1;
			case 0x1F:
				return AKEY_CTRL_2;
			case 0x20:
				return AKEY_CTRL_3;
			case 0x21:
				return AKEY_CTRL_4;
			case 0x22:
				return AKEY_CTRL_5;
			case 0x23:
				return AKEY_CTRL_6;
			case 0x24:
				return AKEY_CTRL_7;
			case 0x25:
				return AKEY_CTRL_8;
			case 0x26:
				return AKEY_CTRL_9;
			case 0x27:
				return AKEY_CTRL_0;
			case 0x2B:
				return AKEY_CLRTAB;
			case 0x33:
				return AKEY_SEMICOLON | AKEY_CTRL;
			case 0x36:
				return AKEY_LESS | AKEY_CTRL;
			case 0x37:
				return AKEY_GREATER | AKEY_CTRL;
			default:
				break;
			}
		}
		if ((key.state == PS2KBD_RAWKEY_DOWN) && PS2KbdALT) {
			switch(key.key) {
			//case dcr ylsa
			case 0x7:
				UI_alt_function = UI_MENU_DISK;
				return AKEY_UI;
			case 0x6:
				UI_alt_function = UI_MENU_CARTRIDGE;
				return AKEY_UI;
			case 0x15:
				UI_alt_function = UI_MENU_RUN;
				return AKEY_UI;
			case 0x1C:
				UI_alt_function = UI_MENU_SYSTEM;
				return AKEY_UI;
			case 0xF:
				UI_alt_function = UI_MENU_LOADSTATE;
				return AKEY_UI;
			case 0x16:
				UI_alt_function = UI_MENU_SAVESTATE;
				return AKEY_UI;
			case 0x17:
				UI_alt_function = UI_MENU_CASSETTE;
				return AKEY_UI;
			case 0x4:
				UI_alt_function = UI_MENU_ABOUT;
				return AKEY_UI;
			default:
				break;
			}
		}


		if (key.state == PS2KBD_RAWKEY_UP) {
			switch (key.key) {
			case 0x39:

			return AKEY_CAPSTOGGLE;
			case 0xE0:
				PS2KbdCONTROL = 0;
				return AKEY_NONE;
			case 0xE4:
				PS2KbdCONTROL = 0;
				return AKEY_NONE;
			case 0xE1:
				PS2KbdSHIFT = 0;
				return AKEY_NONE;
			case 0xE2:
				PS2KbdALT = 0;
				return AKEY_NONE;
			case 0xE5:
				PS2KbdSHIFT = 0;
				return AKEY_NONE;
			case 0xE6:
				PS2KbdALT = 0;
				return AKEY_NONE;
			default:
				break;
			}
		}
	}
}
	return AKEY_NONE;
}

int PLATFORM_PORT(int num)
{
	int ret = 0xff;
	if (num == 0) {
		int pad = PadButtons();
		if (pad & PAD_LEFT)
			ret &= 0xf0 | INPUT_STICK_LEFT;
		if (pad & PAD_RIGHT)
			ret &= 0xf0 | INPUT_STICK_RIGHT;
		if (pad & PAD_UP)
			ret &= 0xf0 | INPUT_STICK_FORWARD;
		if (pad & PAD_DOWN)
			ret &= 0xf0 | INPUT_STICK_BACK;
	}
	return ret;
}

int PLATFORM_TRIG(int num)
{
	if (num == 0 && PadButtons() & PAD_CROSS)
		return 0;
	return 1;
}

char dir_path[FILENAME_MAX];

static int dir_n;
static int dir_i;

// XXX: use followup calls to get directory entries one-by-one?

#define MAX_FILES_PER_DIR 1000

static mcTable mcDir[MAX_FILES_PER_DIR];

int Atari_OpenDir(const char *filename)
{
	// TODO: support other devices
	if (strncmp(filename, "mc0:/", 5) != 0)
		return FALSE;
	dir_n = mcGetDir(0, 0, filename + 4, 0 /* followup flag */, MAX_FILES_PER_DIR, mcDir);
	mcSync(0,NULL,&dir_n);
	if (dir_n < 0)
		return FALSE;
	dir_i = 0;
	// XXX: does it know (and needs to know) that "mc0:/" is a root directory?
	Util_splitpath(filename, dir_path, NULL);
	return TRUE;
}

int Atari_ReadDir(char *fullpath, char *filename, int *isdir,
                  int *readonly, int *size, char *timetext)
{
	const mcTable *p;
	if (dir_i >= dir_n)
		return FALSE;
	p = mcDir + dir_i;
	if (fullpath != NULL)
		Util_catpath(fullpath, dir_path, p->name);
	if (filename != NULL)
		strcpy(filename, p->name);
	if (isdir != NULL)
		*isdir = (p->attrFile & MC_ATTR_SUBDIR) ? TRUE : FALSE;
	if (readonly != NULL)
		*readonly = (p->attrFile & MC_ATTR_WRITEABLE) ? FALSE : TRUE; // XXX: MC_ATTR_PROTECTED ?
	if (size != NULL)
		*size = (int) (p->fileSizeByte);
	if (timetext != NULL) {
		// FIXME: adjust from GMT to local time
		int hour = p->_modify.hour;
		char ampm = 'a';
		if (hour >= 12) {
			hour -= 12;
			ampm = 'p';
		}
		if (hour == 0)
			hour = 12;
		sprintf(timetext, "%2d-%02d-%02d %2d:%02d%c",
			p->_modify.month, p->_modify.day, p->_modify.year % 100, hour, p->_modify.sec, ampm);
	}
	dir_i++;
	return TRUE;
}

#ifdef SOUND

int Sound_Initialise(int *argc, char *argv[])
{
	if (audsrv_init() != 0)
		Log_print("failed to initialize audsrv: %s", audsrv_get_error_string());
	else {
		struct audsrv_fmt_t format;
		format.bits = 8;
		format.freq = 44100;
		format.channels = 1;
		audsrv_set_format(&format);
		audsrv_set_volume(MAX_VOLUME);
		POKEYSND_Init(POKEYSND_FREQ_17_EXACT, 44100, 1, 0);
	}
	return TRUE;
}

void Sound_Exit(void)
{
	audsrv_quit();
}

void Sound_Update(void)
{
	static char buffer[44100 / 50];
	unsigned int nsamples = (Atari800_tv_mode == Atari800_TV_NTSC) ? (44100 / 60) : (44100 / 50);
	POKEYSND_Process(buffer, nsamples);
	audsrv_wait_audio(nsamples);
	audsrv_play_audio(buffer, nsamples);
}

void Sound_Pause(void)
{
	audsrv_stop_audio();
}

void Sound_Continue(void)
{
	if (audsrv_init() != 0)
		Log_print("failed to initialize audsrv: %s", audsrv_get_error_string());
	else {
		struct audsrv_fmt_t format;
		format.bits = 8;
		format.freq = 44100;
		format.channels = 1;
		audsrv_set_format(&format);
		audsrv_set_volume(MAX_VOLUME);
	}
}

#endif /* SOUND */

int main(int argc, char **argv)
{
	loadModules();
	/* initialise Atari800 core */
	if (!Atari800_Initialise(&argc, argv))
		return 3;

	/* main loop */
	for (;;) {
		INPUT_key_code = PLATFORM_Keyboard();
		Atari800_Frame();
		if (Atari800_display_screen){
			PLATFORM_DisplayScreen();}
	}
}
