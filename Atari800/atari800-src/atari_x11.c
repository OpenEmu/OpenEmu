/*
 * atari_x11.c - X11 specific port code
 *
 * Copyright (c) 1995-1998 David Firth
 * Copyright (C) 1998-2008 Atari800 development team (see DOC/CREDITS)
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

#include "config.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include "pokey.h"

#ifdef VMS
#include <stat.h>
#else
#include <sys/stat.h>
#endif

#include <signal.h>
#include <sys/time.h>

typedef unsigned char ubyte;
typedef unsigned short uword;

#ifdef XVIEW
#include <xview/xview.h>
#include <xview/frame.h>
#include <xview/panel.h>
#include <xview/canvas.h>
#include <xview/notice.h>
#include <xview/file_chsr.h>
#endif

#ifdef MOTIF
#include <Xm/MainW.h>
#include <Xm/DrawingA.h>
#include <Xm/MessageB.h>
#include <Xm/FileSB.h>
#include <Xm/RowColumn.h>
#include <Xm/ToggleBG.h>

static XtAppContext app;
static Widget toplevel;
static Widget main_w;
static Widget drawing_area;
static Widget fsel_b;
static Widget fsel_d;
static Widget fsel_r;
static Widget rbox_d;
static Widget togg_d1, togg_d2, togg_d3, togg_d4;
static Widget togg_d5, togg_d6, togg_d7, togg_d8;
static Widget eject_menu;
static Widget disable_menu;
static Widget system_menu;
static int motif_disk_sel = 1;
#endif /* MOTIF */

#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/keysym.h>

#include "atari.h"
#include "cartridge.h"
#include "colours.h"
#include "input.h"
#include "akey.h"
#include "log.h"
#include "monitor.h"
#include "memory.h"
#include "screen.h"
#include "sio.h"
#include "sound.h"
#include "platform.h"
#include "ui.h"
#include "util.h"

#ifdef SHM
#include <sys/ipc.h>
#include <sys/shm.h>
#include <X11/extensions/XShm.h>

static XShmSegmentInfo shminfo;
static XImage *image = NULL;
#ifdef USE_COLOUR_TRANSLATION_TABLE
extern int colour_translation_table[256];
#endif
#endif /* SHM */

static int invisible = 0;

#ifdef LINUX_JOYSTICK
#include <linux/joystick.h>

static int js0;
static int js1;

static int js0_centre_x;
static int js0_centre_y;
static int js1_centre_x;
static int js1_centre_y;

static struct JS_DATA_TYPE js_data;
#endif /* LINUX_JOYSTICK */

typedef enum {
	Small,
	Large,
	Huge
} WindowSize;

static WindowSize windowsize = Large;

enum {
	MONITOR_NOTHING,
	MONITOR_SIO
} x11_monitor = MONITOR_NOTHING;

static int x11bug = FALSE;
static int private_cmap = FALSE;

static int window_width = 336;
static int window_height = Screen_HEIGHT;

static int clipping_factor = 1;
static int clipping_x = 24;
static int clipping_y = 0;
static int clipping_width = 336;
static int clipping_height = Screen_HEIGHT;


static Display *display = NULL;
static Screen *screen = NULL;
static Window window;
#ifndef SHM
static Pixmap pixmap;
#endif
static Visual *visual = NULL;
static Colormap cmap;

static GC gc;
static GC gc_colour[256];
static int colours[256];

#ifdef XVIEW
static Frame frame;
static Panel panel;
static Canvas canvas;
static Menu system_menu;
static Menu consol_menu;
static Menu options_menu;
static Frame chooser;

static Frame controllers_frame;
static Panel controllers_panel;
static Panel_item keypad_item;
static Panel_item mouse_item;

#ifdef LINUX_JOYSTICK
static Panel_item js0_item;
static Panel_item js1_item;
#endif

static Frame performance_frame;
static Panel performance_panel;
static Panel_item refresh_slider;
#endif /* XVIEW */

static int SHIFT = 0x00;
static int CONTROL = 0x00;
static UBYTE *image_data = NULL;
static int modified;

static int keypad_mode = -1;	/* Joystick */
static int keypad_trig = 1;		/* Keypad Trigger Position */
static int keypad_stick = 0x0f;	/* Keypad Joystick Position */

static int xmouse_mode = -1;		/* Joystick, Paddle and Light Pen */
static int mouse_stick;			/* Mouse Joystick Position */

static int js0_mode = -1;
static int js1_mode = -1;

#ifndef SHM

#define	NPOINTS	(4096 / 4)
#define	NRECTS	(4096 / 4)

static XPoint points[NPOINTS];
static XRectangle rectangles[NRECTS];
#endif

static int keyboard_consol = INPUT_CONSOL_NONE;
static int menu_consol = INPUT_CONSOL_NONE;

static int autorepeat = 1;
static int last_focus = FocusOut;

static void autorepeat_get(void)
{
	XKeyboardState kstat;

	XGetKeyboardControl(display, &kstat);
	autorepeat = kstat.global_auto_repeat;
}

static void autorepeat_off(void)
{
	XAutoRepeatOff(display);
}

static void autorepeat_restore(void)
{
	if (autorepeat)
		XAutoRepeatOn(display);
	else
		XAutoRepeatOff(display);
}

static void segmentationfault(int x)
{
	PLATFORM_Exit(0);
	exit(0);
}

static int GetKeyCode(XEvent *event)
{
	KeySym keysym;
	char buffer[128];
	static int keycode = AKEY_NONE;

	if (event->type == KeyPress || event->type == KeyRelease) {
		XLookupString((XKeyEvent *) event, buffer, sizeof(buffer), &keysym, NULL);
	}

	switch (event->type) {
	case Expose:
#ifndef SHM
		XCopyArea(display, pixmap, window, gc,
				  0, 0,
				  window_width, window_height,
				  0, 0);
#else
		modified = TRUE;
#endif
		break;
	case FocusIn:
		autorepeat_off();
		last_focus = FocusIn;
		break;
	case FocusOut:
		autorepeat_restore();
		last_focus = FocusOut;
		break;
	case VisibilityNotify:
		if (((XVisibilityEvent*) event)->state == VisibilityFullyObscured)
			invisible = 1;
		else
			invisible = 0;
		break;
	case KeyPress:
		switch (keysym) {
		case XK_Shift_L:
		case XK_Shift_R:
			SHIFT = AKEY_SHFT;
			INPUT_key_shift = 1;
			break;
		case XK_Control_L:
			keypad_trig = 0;
			/* FALLTHROUGH */
		case XK_Control_R:
			CONTROL = AKEY_CTRL;
			break;
		case XK_F1:
			keycode = AKEY_UI;
			break;
		case XK_F5:
		case XK_L5:
			keycode = SHIFT ? AKEY_COLDSTART : AKEY_WARMSTART;
			break;
		case XK_F8:
			keycode = PLATFORM_Exit(TRUE) ? AKEY_NONE : AKEY_EXIT;
			break;
		case XK_F9:
			keycode = AKEY_EXIT;
			break;
		case XK_F10:
		case XK_L10:
			keycode = SHIFT ? AKEY_SCREENSHOT_INTERLACE : AKEY_SCREENSHOT;
			break;
		case XK_F12:
			keycode = AKEY_TURBO;
			break;
		case XK_Left:
			keycode = AKEY_LEFT;
			keypad_stick &= INPUT_STICK_LEFT;
			break;
		case XK_Up:
			keycode = AKEY_UP;
			keypad_stick &= INPUT_STICK_FORWARD;
			break;
		case XK_Right:
			keycode = AKEY_RIGHT;
			keypad_stick &= INPUT_STICK_RIGHT;
			break;
		case XK_Down:
			keycode = AKEY_DOWN;
			keypad_stick &= INPUT_STICK_BACK;
			break;
		case XK_KP_0:
			keypad_trig = 0;
			keycode = AKEY_NONE;
			break;
		case XK_KP_1:
			keypad_stick = INPUT_STICK_LL;
			keycode = AKEY_NONE;
			break;
		case XK_KP_2:
			keypad_stick &= INPUT_STICK_BACK;
			keycode = AKEY_NONE;
			break;
		case XK_KP_3:
			keypad_stick = INPUT_STICK_LR;
			keycode = AKEY_NONE;
			break;
		case XK_KP_4:
			keypad_stick &= INPUT_STICK_LEFT;
			keycode = AKEY_NONE;
			break;
		case XK_KP_5:
			keypad_stick = INPUT_STICK_CENTRE;
			keycode = AKEY_NONE;
			break;
		case XK_KP_6:
			keypad_stick &= INPUT_STICK_RIGHT;
			keycode = AKEY_NONE;
			break;
		case XK_KP_7:
			keypad_stick = INPUT_STICK_UL;
			keycode = AKEY_NONE;
			break;
		case XK_KP_8:
			keypad_stick &= INPUT_STICK_FORWARD;
			keycode = AKEY_NONE;
			break;
		case XK_KP_9:
			keypad_stick = INPUT_STICK_UR;
			keycode = AKEY_NONE;
			break;
		}
		if (Atari800_machine_type == Atari800_MACHINE_5200 && !UI_is_active) {
			switch (keysym) {
			case XK_F4:
				keycode = SHIFT | AKEY_5200_START;
				break;
			case XK_P:
			case XK_p:
				keycode = SHIFT | AKEY_5200_PAUSE;
				break;
			case XK_R:
			case XK_r:
				keycode = SHIFT | AKEY_5200_RESET;
				break;
			case XK_0:
				keycode = SHIFT | AKEY_5200_0;
				break;
			case XK_1:
				keycode = SHIFT | AKEY_5200_1;
				break;
			case XK_2:
				keycode = SHIFT | AKEY_5200_2;
				break;
			case XK_3:
				keycode = SHIFT | AKEY_5200_3;
				break;
			case XK_4:
				keycode = SHIFT | AKEY_5200_4;
				break;
			case XK_5:
				keycode = SHIFT | AKEY_5200_5;
				break;
			case XK_6:
				keycode = SHIFT | AKEY_5200_6;
				break;
			case XK_7:
				keycode = SHIFT | AKEY_5200_7;
				break;
			case XK_8:
				keycode = SHIFT | AKEY_5200_8;
				break;
			case XK_9:
				keycode = SHIFT | AKEY_5200_9;
				break;
			/* XXX: "SHIFT | " harmful for '#' and '*' ? */
			case XK_numbersign:
			case XK_equal:
				keycode = AKEY_5200_HASH;
				break;
			case XK_asterisk:
				keycode = AKEY_5200_ASTERISK;
				break;
			}
			break;
		}
		switch (keysym) {
		case XK_Caps_Lock:
			keycode = SHIFT | CONTROL | AKEY_CAPSTOGGLE;
			break;
		case XK_Shift_Lock:
			if (x11bug)
				printf("XK_Shift_Lock\n");
			break;
		case XK_Alt_L:
		case XK_Alt_R:
			keycode = AKEY_ATARI;
			break;
		case XK_F2:
			keyboard_consol &= (~INPUT_CONSOL_OPTION);
			keycode = AKEY_NONE;
			break;
		case XK_F3:
			keyboard_consol &= (~INPUT_CONSOL_SELECT);
			keycode = AKEY_NONE;
			break;
		case XK_F4:
			keyboard_consol &= (~INPUT_CONSOL_START);
			keycode = AKEY_NONE;
			break;
		case XK_F6:
			keycode = SHIFT | CONTROL | AKEY_HELP;
			break;
		case XK_Break:
		case XK_F7:
			keycode = AKEY_BREAK;
			break;
		case XK_Home:
			keycode = AKEY_CLEAR;
			break;
		case XK_Insert:
			if (SHIFT)
				keycode = AKEY_INSERT_LINE;
			else
				keycode = AKEY_INSERT_CHAR;
			break;
		case XK_BackSpace:
			if (CONTROL)
				keycode = AKEY_DELETE_CHAR;
			else if (SHIFT)
				keycode = AKEY_DELETE_LINE;
			else
				keycode = AKEY_BACKSPACE;
			break;
		case XK_Delete:
			if (CONTROL)
				keycode = AKEY_DELETE_CHAR;
			else if (SHIFT)
				keycode = AKEY_DELETE_LINE;
			else
				keycode = AKEY_BACKSPACE; /* XXX */
			break;
		case XK_End:
			keycode = SHIFT | CONTROL | AKEY_HELP;
			break;
		case XK_Escape:
			keycode = SHIFT | CONTROL | AKEY_ESCAPE;
			break;
		case XK_Tab:
			keycode = SHIFT | CONTROL | AKEY_TAB;
			break;
		case XK_exclam:
			keycode = CONTROL | AKEY_EXCLAMATION;
			break;
		case XK_quotedbl:
			keycode = CONTROL | AKEY_DBLQUOTE;
			break;
		case XK_numbersign:
			keycode = CONTROL | AKEY_HASH;
			break;
		case XK_dollar:
			keycode = CONTROL | AKEY_DOLLAR;
			break;
		case XK_percent:
			keycode = CONTROL | AKEY_PERCENT;
			break;
		case XK_ampersand:
			keycode = CONTROL | AKEY_AMPERSAND;
			break;
		case XK_quoteright:
			keycode = CONTROL | AKEY_QUOTE;
			break;
		case XK_at:
			keycode = CONTROL | AKEY_AT;
			break;
		case XK_parenleft:
			keycode = CONTROL | AKEY_PARENLEFT;
			break;
		case XK_parenright:
			keycode = CONTROL | AKEY_PARENRIGHT;
			break;
		case XK_less:
			keycode = CONTROL | AKEY_LESS;
			break;
		case XK_greater:
			keycode = CONTROL | AKEY_GREATER;
			break;
		case XK_equal:
			keycode = CONTROL | AKEY_EQUAL;
			break;
		case XK_question:
			keycode = CONTROL | AKEY_QUESTION;
			break;
		case XK_minus:
			keycode = CONTROL | AKEY_MINUS;
			break;
		case XK_plus:
			keycode = CONTROL | AKEY_PLUS;
			break;
		case XK_asterisk:
			keycode = CONTROL | AKEY_ASTERISK;
			break;
		case XK_slash:
			keycode = CONTROL | AKEY_SLASH;
			break;
		case XK_colon:
			keycode = CONTROL | AKEY_COLON;
			break;
		case XK_semicolon:
			keycode = CONTROL | AKEY_SEMICOLON;
			break;
		case XK_comma:
			keycode = CONTROL | AKEY_COMMA;
			break;
		case XK_period:
			keycode = CONTROL | AKEY_FULLSTOP;
			break;
		case XK_underscore:
			keycode = CONTROL | AKEY_UNDERSCORE;
			break;
		case XK_bracketleft:
			keycode = CONTROL | AKEY_BRACKETLEFT;
			break;
		case XK_bracketright:
			keycode = CONTROL | AKEY_BRACKETRIGHT;
			break;
		case XK_asciicircum:
			keycode = CONTROL | AKEY_CIRCUMFLEX;
			break;
		case XK_backslash:
			keycode = CONTROL | AKEY_BACKSLASH;
			break;
		case XK_bar:
			keycode = CONTROL | AKEY_BAR;
			break;
		case XK_space:
			keycode = SHIFT | CONTROL | AKEY_SPACE;
			keypad_trig = 0;
			break;
		case XK_Return:
			keycode = SHIFT | CONTROL | AKEY_RETURN;
			keypad_stick = INPUT_STICK_CENTRE;
			break;
		case XK_0:
			keycode = CONTROL | AKEY_0;
			break;
		case XK_1:
			keycode = CONTROL | AKEY_1;
			break;
		case XK_2:
			keycode = CONTROL | AKEY_2;
			break;
		case XK_3:
			keycode = CONTROL | AKEY_3;
			break;
		case XK_4:
			keycode = CONTROL | AKEY_4;
			break;
		case XK_5:
			keycode = CONTROL | AKEY_5;
			break;
		case XK_6:
			keycode = CONTROL | AKEY_6;
			break;
		case XK_7:
			keycode = CONTROL | AKEY_7;
			break;
		case XK_8:
			keycode = CONTROL | AKEY_8;
			break;
		case XK_9:
			keycode = CONTROL | AKEY_9;
			break;
		case XK_A:
		case XK_a:
			keycode = SHIFT | CONTROL | AKEY_a;
			break;
		case XK_B:
		case XK_b:
			keycode = SHIFT | CONTROL | AKEY_b;
			break;
		case XK_C:
		case XK_c:
			keycode = SHIFT | CONTROL | AKEY_c;
			break;
		case XK_D:
		case XK_d:
			keycode = SHIFT | CONTROL | AKEY_d;
			break;
		case XK_E:
		case XK_e:
			keycode = SHIFT | CONTROL | AKEY_e;
			break;
		case XK_F:
		case XK_f:
			keycode = SHIFT | CONTROL | AKEY_f;
			break;
		case XK_G:
		case XK_g:
			keycode = SHIFT | CONTROL | AKEY_g;
			break;
		case XK_H:
		case XK_h:
			keycode = SHIFT | CONTROL | AKEY_h;
			break;
		case XK_I:
		case XK_i:
			keycode = SHIFT | CONTROL | AKEY_i;
			break;
		case XK_J:
		case XK_j:
			keycode = SHIFT | CONTROL | AKEY_j;
			break;
		case XK_K:
		case XK_k:
			keycode = SHIFT | CONTROL | AKEY_k;
			break;
		case XK_L:
		case XK_l:
			keycode = SHIFT | CONTROL | AKEY_l;
			break;
		case XK_M:
		case XK_m:
			keycode = SHIFT | CONTROL | AKEY_m;
			break;
		case XK_N:
		case XK_n:
			keycode = SHIFT | CONTROL | AKEY_n;
			break;
		case XK_O:
		case XK_o:
			keycode = SHIFT | CONTROL | AKEY_o;
			break;
		case XK_P:
		case XK_p:
			keycode = SHIFT | CONTROL | AKEY_p;
			break;
		case XK_Q:
		case XK_q:
			keycode = SHIFT | CONTROL | AKEY_q;
			break;
		case XK_R:
		case XK_r:
			keycode = SHIFT | CONTROL | AKEY_r;
			break;
		case XK_S:
		case XK_s:
			keycode = SHIFT | CONTROL | AKEY_s;
			break;
		case XK_T:
		case XK_t:
			keycode = SHIFT | CONTROL | AKEY_t;
			break;
		case XK_U:
		case XK_u:
			keycode = SHIFT | CONTROL | AKEY_u;
			break;
		case XK_V:
		case XK_v:
			keycode = SHIFT | CONTROL | AKEY_v;
			break;
		case XK_W:
		case XK_w:
			keycode = SHIFT | CONTROL | AKEY_w;
			break;
		case XK_X:
		case XK_x:
			keycode = SHIFT | CONTROL | AKEY_x;
			break;
		case XK_Y:
		case XK_y:
			keycode = SHIFT | CONTROL | AKEY_y;
			break;
		case XK_Z:
		case XK_z:
			keycode = SHIFT | CONTROL | AKEY_z;
			break;
		default:
			if (x11bug)
				printf("Pressed Keysym = %x\n", (int) keysym);
			break;
		}
		break;
	case KeyRelease:
		keycode = AKEY_NONE;
		switch (keysym) {
		case XK_Shift_L:
		case XK_Shift_R:
			INPUT_key_shift = 0;
			SHIFT = 0x00;
			break;
		case XK_Control_L:
			keypad_trig = 1;
			/* FALLTHROUGH */
		case XK_Control_R:
			CONTROL = 0x00;
			break;
		case XK_Shift_Lock:
			if (x11bug)
				printf("XK_Shift_Lock\n");
			break;
		case XK_F2:
			keyboard_consol |= INPUT_CONSOL_OPTION;
			break;
		case XK_F3:
			keyboard_consol |= INPUT_CONSOL_SELECT;
			break;
		case XK_F4:
			keyboard_consol |= INPUT_CONSOL_START;
			break;
		case XK_space:
		case XK_KP_0:
			keypad_trig = 1;
			break;
		case XK_Down:
		case XK_KP_2:
			keypad_stick |= INPUT_STICK_CENTRE ^ INPUT_STICK_BACK;
			break;
		case XK_Left:
		case XK_KP_4:
			keypad_stick |= INPUT_STICK_CENTRE ^ INPUT_STICK_LEFT;
			break;
		case XK_Right:
		case XK_KP_6:
			keypad_stick |= INPUT_STICK_CENTRE ^ INPUT_STICK_RIGHT;
			break;
		case XK_Up:
		case XK_KP_8:
			keypad_stick |= INPUT_STICK_CENTRE ^ INPUT_STICK_FORWARD;
			break;
		case XK_KP_1:
		case XK_KP_3:
		case XK_KP_5:
		case XK_KP_7:
		case XK_KP_9:
			keypad_stick = INPUT_STICK_CENTRE;
			break;
		default:
			break;
		}
		break;
	}
	return keycode;
}

#if defined(XVIEW) || defined(MOTIF)

static int insert_rom(const char *filename)
{
	int r;
	int i;
	r = CARTRIDGE_Insert(filename);
	if (r < 0)
		return FALSE;
	if (r == 0) {
		Atari800_Coldstart();
		return TRUE;
	}
	/* TODO: select cartridge type */
	for (i = 1; i < CARTRIDGE_LAST_SUPPORTED; i++) {
		if (CARTRIDGE_kb[i] == r) {
			CARTRIDGE_type = i;
			Atari800_Coldstart();
			return TRUE;
		}
	}
	return FALSE;
}

static int xview_keycode = AKEY_NONE;

#endif /* defined(XVIEW) || defined(MOTIF) */

#ifdef XVIEW

static void event_proc(Xv_Window window, Event * event, Notify_arg arg)
{
	xview_keycode = GetKeyCode(event->ie_xevent);
}

static int auto_reboot;

static int disk_change(char *a, char *full_filename, char *filename)
{
	int diskno;
	int status;

	diskno = 1;

	if (!auto_reboot)
		diskno = notice_prompt(panel, NULL,
							   NOTICE_MESSAGE_STRINGS,
							   "Insert Disk into which drive?",
							   NULL,
							   NOTICE_BUTTON, "1", 1,
							   NOTICE_BUTTON, "2", 2,
							   NOTICE_BUTTON, "3", 3,
							   NOTICE_BUTTON, "4", 4,
							   NOTICE_BUTTON, "5", 5,
							   NOTICE_BUTTON, "6", 6,
							   NOTICE_BUTTON, "7", 7,
							   NOTICE_BUTTON, "8", 8,
							   NULL);

	if ((diskno < 1) || (diskno > 8)) {
		printf("Invalid diskno: %d\n", diskno);
		exit(1);
	}
	SIO_Dismount(diskno);
	if (!SIO_Mount(diskno, full_filename, FALSE))
		status = XV_ERROR;
	else {
		if (auto_reboot)
			Atari800_Coldstart();
		status = XV_OK;
	}

	return status;
}

static void boot_callback(void)
{
	static char dir[FILENAME_MAX];
	if (UI_n_atari_files_dir > 0)
		strcpy(dir, UI_atari_files_dir[0]);
	else
		dir[0] = '\0';
	auto_reboot = TRUE;
	xv_set(chooser,
		   FRAME_LABEL, "Disk Selector",
		   FILE_CHOOSER_DIRECTORY, dir,
		   FILE_CHOOSER_NOTIFY_FUNC, disk_change,
		   XV_SHOW, TRUE,
		   NULL);
}

static void insert_callback(void)
{
	static char dir[FILENAME_MAX];
	if (UI_n_atari_files_dir > 0)
		strcpy(dir, UI_atari_files_dir[0]);
	else
		dir[0] = '\0';
	auto_reboot = FALSE;
	xv_set(chooser,
		   FRAME_LABEL, "Disk Selector",
		   FILE_CHOOSER_DIRECTORY, dir,
		   FILE_CHOOSER_NOTIFY_FUNC, disk_change,
		   XV_SHOW, TRUE,
		   NULL);
}

static void eject_callback(void)
{
	int diskno;

	diskno = notice_prompt(panel, NULL,
						   NOTICE_MESSAGE_STRINGS,
						   "Eject Disk from drive?",
						   NULL,
						   NOTICE_BUTTON, "1", 1,
						   NOTICE_BUTTON, "2", 2,
						   NOTICE_BUTTON, "3", 3,
						   NOTICE_BUTTON, "4", 4,
						   NOTICE_BUTTON, "5", 5,
						   NOTICE_BUTTON, "6", 6,
						   NOTICE_BUTTON, "7", 7,
						   NOTICE_BUTTON, "8", 8,
						   NULL);

	if (diskno >= 1 && diskno <= 8)
		SIO_Dismount(diskno);
}

static void disable_callback(void)
{
	int diskno;

	diskno = notice_prompt(panel, NULL,
						   NOTICE_MESSAGE_STRINGS,
						   "Drive to Disable?",
						   NULL,
						   NOTICE_BUTTON, "1", 1,
						   NOTICE_BUTTON, "2", 2,
						   NOTICE_BUTTON, "3", 3,
						   NOTICE_BUTTON, "4", 4,
						   NOTICE_BUTTON, "5", 5,
						   NOTICE_BUTTON, "6", 6,
						   NOTICE_BUTTON, "7", 7,
						   NOTICE_BUTTON, "8", 8,
						   NULL);

	if (diskno >= 1 && diskno <= 8)
		SIO_DisableDrive(diskno);
}

static int rom_change(char *a, char *full_filename, char *filename)
{
	return insert_rom(full_filename) ? XV_OK : XV_ERROR;
}

static void insert_rom_callback(void)
{
	static char dir[FILENAME_MAX];
	if (UI_n_atari_files_dir > 0)
		strcpy(dir, UI_atari_files_dir[0]);
	else
		dir[0] = '\0';
	xv_set(chooser,
		   FRAME_LABEL, "ROM Selector",
		   FILE_CHOOSER_DIRECTORY, dir,
		   FILE_CHOOSER_NOTIFY_FUNC, rom_change,
		   XV_SHOW, TRUE,
		   NULL);
}

static void remove_rom_callback(void)
{
	CARTRIDGE_Remove();
	Atari800_Coldstart();
}

static void exit_callback(void)
{
	PLATFORM_Exit(FALSE);
	exit(1);
}

static void option_callback(void)
{
	menu_consol &= (~INPUT_CONSOL_OPTION);
}

static void select_callback(void)
{
	menu_consol &= (~INPUT_CONSOL_SELECT);
}

static void start_callback(void)
{
	menu_consol &= (~INPUT_CONSOL_START);
}

static void reset_callback(void)
{
	Atari800_Warmstart();
}

static void coldstart_callback(void)
{
	Atari800_Coldstart();
}

static void coldstart_sys(int machtype, int ram, const char *errmsg)
{
	Atari800_machine_type = machtype;
	MEMORY_ram_size = ram;
	if (!Atari800_InitialiseMachine()) {
		notice_prompt(panel, NULL,
					  NOTICE_MESSAGE_STRINGS,
					  errmsg,
					  NULL,
					  NOTICE_BUTTON, "Cancel", 1,
					  NULL);
	}
}

static void coldstart_osa_callback(void)
{
	coldstart_sys(Atari800_MACHINE_OSA, 48, "Sorry, OS/A ROM Unavailable");
}

static void coldstart_osb_callback(void)
{
	coldstart_sys(Atari800_MACHINE_OSB, 48, "Sorry, OS/B ROM Unavailable");
}

static void coldstart_xl_callback(void)
{
	coldstart_sys(Atari800_MACHINE_XLXE, 64, "Sorry, XL/XE ROM Unavailable");
}

static void coldstart_xe_callback(void)
{
	coldstart_sys(Atari800_MACHINE_XLXE, 128, "Sorry, XL/XE ROM Unavailable");
}

static void coldstart_5200_callback(void)
{
	coldstart_sys(Atari800_MACHINE_5200, 16, "Sorry, 5200 ROM Unavailable");
}

static void controllers_ok_callback(void)
{
	xv_set(controllers_frame,
		   XV_SHOW, FALSE,
		   NULL);
}

static void controllers_callback(void)
{
	xv_set(controllers_frame,
		   XV_SHOW, TRUE,
		   NULL);
}

static void sorry_message(void)
{
	notice_prompt(panel, NULL,
				  NOTICE_MESSAGE_STRINGS,
				  "Sorry, controller already assigned",
				  "to another device",
				  NULL,
				  NOTICE_BUTTON, "Cancel", 1,
				  NULL);
}

static void keypad_callback(void)
{
	int new_mode;

	new_mode = xv_get(keypad_item, PANEL_VALUE);

	if ((new_mode != xmouse_mode) &&
		(new_mode != js0_mode) &&
		(new_mode != js1_mode)) {
		keypad_mode = new_mode;
	}
	else {
		sorry_message();
		xv_set(keypad_item,
			   PANEL_VALUE, keypad_mode,
			   NULL);
	}
}

static void mouse_callback(void)
{
	int new_mode;

	new_mode = xv_get(mouse_item, PANEL_VALUE);

	if ((new_mode != keypad_mode) &&
		(new_mode != js0_mode) &&
		(new_mode != js1_mode)) {
		xmouse_mode = new_mode;
	}
	else {
		sorry_message();
		xv_set(mouse_item,
			   PANEL_VALUE, xmouse_mode,
			   NULL);
	}
}

#ifdef LINUX_JOYSTICK
static void js0_callback(void)
{
	int new_mode;

	new_mode = xv_get(js0_item, PANEL_VALUE);

	if ((new_mode != keypad_mode) &&
		(new_mode != xmouse_mode) &&
		(new_mode != js1_mode)) {
		js0_mode = new_mode;
	}
	else {
		sorry_message();
		xv_set(js0_item,
			   PANEL_VALUE, js0_mode,
			   NULL);
	}
}

static void js1_callback(void)
{
	int new_mode;

	new_mode = xv_get(js1_item, PANEL_VALUE);

	if ((new_mode != keypad_mode) &&
		(new_mode != xmouse_mode) &&
		(new_mode != js0_mode)) {
		js1_mode = new_mode;
	}
	else {
		sorry_message();
		xv_set(js1_item,
			   PANEL_VALUE, js1_mode,
			   NULL);
	}
}
#endif /* LINUX_JOYSTICK */

static void performance_ok_callback(void)
{
	xv_set(performance_frame,
		   XV_SHOW, FALSE,
		   NULL);
}

static void performance_callback(void)
{
	xv_set(performance_frame,
		   XV_SHOW, TRUE,
		   NULL);
}

static void refresh_callback(Panel_item item, int value, Event * event)
{
	Atari800_refresh_rate = value;
}

#endif /* XVIEW */

static void Atari_WhatIs(int mode)
{
	switch (mode) {
	case 0:
		printf("Joystick 0");
		break;
	case 1:
		printf("Joystick 1");
		break;
	case 2:
		printf("Joystick 2");
		break;
	case 3:
		printf("Joystick 3");
		break;
	default:
		printf("not available");
		break;
	}
}

#ifdef MOTIF

static void motif_boot_disk(Widget fs, XtPointer client_data,
					 XtPointer cbs)
{
	char *filename;

	if (XmStringGetLtoR(((XmFileSelectionBoxCallbackStruct *) cbs)->value,
						XmSTRING_DEFAULT_CHARSET, &filename)) {
		if (*filename) {
			SIO_Dismount(1);
			if (SIO_Mount(1, filename, FALSE))
				Atari800_Coldstart();
		}
		XtFree(filename);
	}
	XtUnmanageChild(fs);
	XtPopdown(XtParent(fs));
}

static void motif_select_disk(Widget toggle, XtPointer client_data, XtPointer cbs)
{
	motif_disk_sel = (int) client_data;
}

static void motif_insert_disk(Widget fs, XtPointer client_data, XtPointer cbs)
{
	char *filename;

	if (XmStringGetLtoR(((XmFileSelectionBoxCallbackStruct *) cbs)->value,
						XmSTRING_DEFAULT_CHARSET, &filename)) {
		if (*filename) {
			SIO_Dismount(motif_disk_sel);
			SIO_Mount(motif_disk_sel, filename, FALSE);
		}
		XtFree(filename);
	}
	XtUnmanageChild(fs);
	XtPopdown(XtParent(fs));
}

static void motif_insert_rom(Widget fs, XtPointer client_data, XtPointer cbs)
{
	char *filename;

	if (XmStringGetLtoR(((XmFileSelectionBoxCallbackStruct *) cbs)->value,
						XmSTRING_DEFAULT_CHARSET, &filename)) {
		if (*filename) {
			insert_rom(filename);
		}
		XtFree(filename);
	}
	XtUnmanageChild(fs);
	XtPopdown(XtParent(fs));
}

static void motif_fs_cancel(Widget fs, XtPointer client_data, XtPointer call_data)
{
	XtUnmanageChild(fs);
	XtPopdown(XtParent(fs));
}

static void motif_eject_cback(Widget button, XtPointer client_data, XtPointer cbs)
{
	SIO_Dismount(((int) client_data) + 1);
}

static void motif_disable_cback(Widget button, XtPointer client_data, XtPointer cbs)
{
	SIO_DisableDrive(((int) client_data) + 1);
}

static void update_fsel(Widget fsel)
{
	XmString dirmask;

	XtVaGetValues(fsel, XmNdirMask, &dirmask, NULL);
	XmFileSelectionDoSearch(fsel, dirmask);
}

static void motif_system_cback(Widget w, XtPointer item_no, XtPointer cbs)
{
	XmString t;
	int status;
	char *errmsg = NULL;

	switch ((int) item_no) {
	case 0:
		update_fsel(fsel_b);
		XtManageChild(fsel_b);
		XtPopup(XtParent(fsel_b), XtGrabNone);
		break;
	case 1:
		/* insert disk */
		update_fsel(fsel_d);
		XtManageChild(fsel_d);
		XtPopup(XtParent(fsel_d), XtGrabNone);
		break;
	case 2:
		/* eject disk */
		/* handled by pullright menu */
		break;
	case 3:
		/* disable drive */
		/* handled by pullright menu */
		break;
	case 4:
		/* insert rom */
		update_fsel(fsel_r);
		XtManageChild(fsel_r);
		XtPopup(XtParent(fsel_r), XtGrabNone);
		break;
	case 5:
		CARTRIDGE_Remove();
		Atari800_Coldstart();
		break;
	case 6:
		Atari800_machine_type = Atari800_MACHINE_OSA;
		MEMORY_ram_size = 48;
		status = Atari800_InitialiseMachine();
		if (status == 0)
			errmsg = "Sorry, OS/A ROM Unavailable";
		break;
	case 7:
		Atari800_machine_type = Atari800_MACHINE_OSB;
		MEMORY_ram_size = 48;
		status = Atari800_InitialiseMachine();
		if (status == 0)
			errmsg = "Sorry, OS/B ROM Unavailable";
		break;
	case 8:
		Atari800_machine_type = Atari800_MACHINE_XLXE;
		MEMORY_ram_size = 64;
		status = Atari800_InitialiseMachine();
		if (status == 0)
			errmsg = "Sorry, XL/XE ROM Unavailable";
		break;
	case 9:
		Atari800_machine_type = Atari800_MACHINE_XLXE;
		MEMORY_ram_size = 128;
		status = Atari800_InitialiseMachine();
		if (status == 0)
			errmsg = "Sorry, XL/XE ROM Unavailable";
		break;
	case 10:
		Atari800_machine_type = Atari800_MACHINE_5200;
		MEMORY_ram_size = 16;
		status = Atari800_InitialiseMachine();
		if (status == 0)
			errmsg = "Sorry, 5200 ROM Unavailable";
		break;
	case 11:
		Atari800_Exit(FALSE);
		exit(0);
	}

	if (errmsg) {
		static Widget dialog = NULL;

		if (!dialog) {
			Arg arg[1];

			dialog = XmCreateErrorDialog(main_w, "message", arg, 0);

			XtVaSetValues(dialog,
						  XmNdialogStyle, XmDIALOG_FULL_APPLICATION_MODAL,
						  NULL);

			XtUnmanageChild(XmMessageBoxGetChild(dialog, XmDIALOG_OK_BUTTON));
			XtUnmanageChild(XmMessageBoxGetChild(dialog, XmDIALOG_HELP_BUTTON));
		}
		t = XmStringCreateSimple(errmsg);
		XtVaSetValues(dialog,
					  XmNmessageString, t,
					  NULL);
		XmStringFree(t);
		XtManageChild(dialog);
	}
}

static void motif_consol_cback(Widget w, XtPointer item_no, XtPointer cbs)
{
	switch ((int) item_no) {
	case 0:
		menu_consol &= (~INPUT_CONSOL_OPTION);
		break;
	case 1:
		menu_consol &= (~INPUT_CONSOL_SELECT);
		break;
	case 2:
		menu_consol &= (~INPUT_CONSOL_START);
		break;
	case 3:
		Atari800_Warmstart();
		break;
	case 4:
		Atari800_Coldstart();
		break;
	}
}

static void motif_keypress(Widget w, XtPointer client_data, XEvent *event,
                           Boolean *continue_to_dispatch)
{
	xview_keycode = GetKeyCode(event);
}

static void motif_exposure(Widget w, XtPointer client_data, XEvent *event,
					Boolean *continue_to_dispatch)
{
	modified = TRUE;
}

#endif /* MOTIF */

int PLATFORM_Initialise(int *argc, char *argv[])
{
#if !defined(XVIEW) && !defined(MOTIF)
	XSetWindowAttributes xswda;
#endif

	XGCValues xgcvl;

	int depth;
	int colorstep;
	int i, j;
	int mode = 0;
	int help_only = FALSE;

#ifdef XVIEW
	int ypos;

	xv_init(XV_INIT_ARGC_PTR_ARGV, argc, argv, NULL);
#endif

#ifdef MOTIF
	toplevel = XtVaAppInitialize(&app, "Atari800",
								 NULL, 0,
								 argc, argv, NULL,
								 XtNtitle, Atari800_TITLE,
								 NULL);
#endif

	for (i = j = 1; i < *argc; i++) {
		int i_a = (i + 1 < *argc);		/* is argument available? */
		int a_m = FALSE;			/* error, argument missing! */
		
		if (strcmp(argv[i], "-small") == 0)
			windowsize = Small;
		else if (strcmp(argv[i], "-large") == 0)
			windowsize = Large;
		else if (strcmp(argv[i], "-huge") == 0)
			windowsize = Huge;
		else if (strcmp(argv[i], "-clip_x") == 0)
			if (i_a)
				clipping_x = atoi(argv[++i]);
			else a_m = TRUE;
		else if (strcmp(argv[i], "-clip_y") == 0)
			if (i_a)
				clipping_y = atoi(argv[++i]);
			else a_m = TRUE;
		else if (strcmp(argv[i], "-clip_width") == 0)
			if (i_a)
				clipping_width = atoi(argv[++i]);
			else a_m = TRUE;
		else if (strcmp(argv[i], "-clip_height") == 0)
			if (i_a)
				clipping_height = atoi(argv[++i]);
			else a_m = TRUE;
		else if (strcmp(argv[i], "-x11bug") == 0)
			x11bug = TRUE;
		else if (strcmp(argv[i], "-sio") == 0)
			x11_monitor = MONITOR_SIO;
		else if (strcmp(argv[i], "-private_cmap") == 0)
			private_cmap = TRUE;
		else if (strcmp(argv[i], "-keypad") == 0) {
			if (keypad_mode == -1)
				keypad_mode = mode++;
		}
		else {
			if (strcmp(argv[i], "-help") == 0) {
				help_only = TRUE;
				printf("\t-small           Small window (%dx%d)\n",
					   clipping_width, clipping_height);
				printf("\t-large           Large window (%dx%d)\n",
					   clipping_width * 2, clipping_height * 2);
				printf("\t-huge            Huge window (%dx%d)\n",
					   clipping_width * 3, clipping_height * 3);
				printf("\t-x11bug          Enable debug code in atari_x11.c\n");
				printf("\t-clip_x <n>      Set left offset in pixels for clipping\n");
				printf("\t-clip_width <n>  Set window clip-width\n");
				printf("\t-clip_y <n>      Set top offset for clipping\n");
				printf("\t-clip_height <n> Set window clip-height\n");
				printf("\t-private_cmap    Use private colormap\n");
				printf("\t-sio             Show SIO monitor\n");
				printf("\t-keypad          Keypad mode\n");
			}
			argv[j++] = argv[i];
		}
	}

	*argc = j;

#ifdef SOUND
	if (!Sound_Initialise(argc, argv))
		return FALSE;
#endif

	if (help_only)
		return TRUE;

	if ((clipping_x < 0) || (clipping_x >= Screen_WIDTH))
		clipping_x = 0;
	if ((clipping_y < 0) || (clipping_y >= Screen_HEIGHT))
		clipping_y = 0;
	if ((clipping_width <= 0) || (clipping_x + clipping_width > Screen_WIDTH))
		clipping_width = Screen_WIDTH - clipping_x;
	if ((clipping_height <= 0) || (clipping_y + clipping_height > Screen_HEIGHT))
		clipping_height = Screen_HEIGHT - clipping_y;
	Screen_visible_x1 = clipping_x;
	Screen_visible_x2 = clipping_x + clipping_width;
	Screen_visible_y1 = clipping_y;
	Screen_visible_y2 = clipping_y + clipping_height;
	switch (windowsize) {
	case Small:
		clipping_factor = 1;
		window_width = clipping_width;
		window_height = clipping_height;
		break;
	case Large:
		clipping_factor = 2;
		window_width = clipping_width * 2;
		window_height = clipping_height * 2;
		break;
	case Huge:
		clipping_factor = 3;
		window_width = clipping_width * 3;
		window_height = clipping_height * 3;
		break;
	}

#ifdef LINUX_JOYSTICK
	js0 = open("/dev/js0", O_RDONLY, 0777);
	if (js0 != -1) {
		int status;

		status = read(js0, &js_data, JS_RETURN);
		if (status != JS_RETURN) {
			perror("/dev/js0");
			exit(1);
		}
		js0_centre_x = js_data.x;
		js0_centre_y = js_data.y;

		if (x11bug)
			printf("Joystick 0: centre_x = %d, centry_y = %d\n",
				   js0_centre_x, js0_centre_y);

		js0_mode = mode++;
	}
	js1 = open("/dev/js1", O_RDONLY, 0777);
	if (js1 != -1) {
		int status;

		status = read(js1, &js_data, JS_RETURN);
		if (status != JS_RETURN) {
			perror("/dev/js1");
			exit(1);
		}
		js1_centre_x = js_data.x;
		js1_centre_y = js_data.y;

		if (x11bug)
			printf("Joystick 1: centre_x = %d, centry_y = %d\n",
				   js1_centre_x, js1_centre_y);

		js1_mode = mode++;
	}
#endif

	xmouse_mode = mode++;
	if (keypad_mode == -1)
		keypad_mode = mode++;

#ifdef XVIEW
	frame = (Frame) xv_create((Xv_opaque) NULL, FRAME,
							  FRAME_LABEL, Atari800_TITLE,
							  FRAME_SHOW_RESIZE_CORNER, FALSE,
							  XV_WIDTH, window_width,
							  XV_HEIGHT, window_height + 27,
							  FRAME_SHOW_FOOTER, TRUE,
							  XV_SHOW, TRUE,
							  NULL);

	panel = (Panel) xv_create(frame, PANEL,
							  XV_HEIGHT, 25,
							  XV_SHOW, TRUE,
							  NULL);

	system_menu = xv_create((Xv_opaque) NULL, MENU,
							MENU_ITEM,
							MENU_STRING, "Boot Disk",
							MENU_NOTIFY_PROC, boot_callback,
							NULL,
							MENU_ITEM,
							MENU_STRING, "Insert Disk",
							MENU_NOTIFY_PROC, insert_callback,
							NULL,
							MENU_ITEM,
							MENU_STRING, "Eject Disk",
							MENU_NOTIFY_PROC, eject_callback,
							NULL,
							MENU_ITEM,
							MENU_STRING, "Disable Drive",
							MENU_NOTIFY_PROC, disable_callback,
							NULL,
							MENU_ITEM,
							MENU_STRING, "Insert Cartridge",
							MENU_NOTIFY_PROC, insert_rom_callback,
							NULL,
							MENU_ITEM,
							MENU_STRING, "Remove Cartridge",
							MENU_NOTIFY_PROC, remove_rom_callback,
							NULL,
							MENU_ITEM,
							MENU_STRING, "Atari 800 OS/A",
							MENU_NOTIFY_PROC, coldstart_osa_callback,
							NULL,
							MENU_ITEM,
							MENU_STRING, "Atari 800 OS/B",
							MENU_NOTIFY_PROC, coldstart_osb_callback,
							NULL,
							MENU_ITEM,
							MENU_STRING, "Atari 800XL",
							MENU_NOTIFY_PROC, coldstart_xl_callback,
							NULL,
							MENU_ITEM,
							MENU_STRING, "Atari 130XE",
							MENU_NOTIFY_PROC, coldstart_xe_callback,
							NULL,
							MENU_ITEM,
							MENU_STRING, "Atari 5200",
							MENU_NOTIFY_PROC, coldstart_5200_callback,
							NULL,
							MENU_ITEM,
							MENU_STRING, "Exit",
							MENU_NOTIFY_PROC, exit_callback,
							NULL,
							NULL);

	xv_create(panel, PANEL_BUTTON,
			  PANEL_LABEL_STRING, "System",
			  PANEL_ITEM_MENU, system_menu,
			  NULL);

	consol_menu = (Menu) xv_create((Xv_opaque) NULL, MENU,
								   MENU_ITEM,
								   MENU_STRING, "Option",
								   MENU_NOTIFY_PROC, option_callback,
								   NULL,
								   MENU_ITEM,
								   MENU_STRING, "Select",
								   MENU_NOTIFY_PROC, select_callback,
								   NULL,
								   MENU_ITEM,
								   MENU_STRING, "Start",
								   MENU_NOTIFY_PROC, start_callback,
								   NULL,
								   MENU_ITEM,
								   MENU_STRING, "Reset",
								   MENU_NOTIFY_PROC, reset_callback,
								   NULL,
								   MENU_ITEM,
								   MENU_STRING, "Coldstart",
								   MENU_NOTIFY_PROC, coldstart_callback,
								   NULL,
								   NULL);

	xv_create(panel, PANEL_BUTTON,
			  PANEL_LABEL_STRING, "Console",
			  PANEL_ITEM_MENU, consol_menu,
			  NULL);

	options_menu = (Menu) xv_create((Xv_opaque) NULL, MENU,
									MENU_ITEM,
									MENU_STRING, "Controllers",
								  MENU_NOTIFY_PROC, controllers_callback,
									NULL,
									MENU_ITEM,
									MENU_STRING, "Performance",
								  MENU_NOTIFY_PROC, performance_callback,
									NULL,
									NULL);

	xv_create(panel, PANEL_BUTTON,
			  PANEL_LABEL_STRING, "Options",
			  PANEL_ITEM_MENU, options_menu,
			  NULL);

	canvas = (Canvas) xv_create(frame, CANVAS,
								CANVAS_WIDTH, window_width,
								CANVAS_HEIGHT, window_height,
								NULL);
/*
   =====================================
   Create Controller Configuration Frame
   =====================================
 */
	controllers_frame = (Frame) xv_create(frame, FRAME_CMD,
								 FRAME_LABEL, "Controller Configuration",
										  XV_WIDTH, 300,
										  XV_HEIGHT, 150,
										  NULL);

	controllers_panel = (Panel) xv_get(controllers_frame, FRAME_CMD_PANEL,
									   NULL);

	ypos = 10;
	keypad_item = (Panel_item) xv_create(controllers_panel, PANEL_CHOICE_STACK,
										 PANEL_VALUE_X, 150,
										 PANEL_VALUE_Y, ypos,
										 PANEL_LAYOUT, PANEL_HORIZONTAL,
										 PANEL_LABEL_STRING, "Numeric Keypad",
										 PANEL_CHOICE_STRINGS,
										 "Joystick 1",
										 "Joystick 2",
										 "Joystick 3",
										 "Joystick 4",
										 NULL,
										 PANEL_VALUE, keypad_mode,
										 PANEL_NOTIFY_PROC, keypad_callback,
										 NULL);
	ypos += 25;

	mouse_item = (Panel_item) xv_create(controllers_panel, PANEL_CHOICE_STACK,
										PANEL_VALUE_X, 150,
										PANEL_VALUE_Y, ypos,
										PANEL_LAYOUT, PANEL_HORIZONTAL,
										PANEL_LABEL_STRING, "Mouse",
										PANEL_CHOICE_STRINGS,
										"Joystick 1",
										"Joystick 2",
										"Joystick 3",
										"Joystick 4",
										NULL,
										PANEL_VALUE, xmouse_mode,
										PANEL_NOTIFY_PROC, mouse_callback,
										NULL);
	ypos += 25;

#ifdef LINUX_JOYSTICK
	if (js0 != -1) {
		js0_item = (Panel_item) xv_create(controllers_panel, PANEL_CHOICE_STACK,
										  PANEL_VALUE_X, 150,
										  PANEL_VALUE_Y, ypos,
										  PANEL_LAYOUT, PANEL_HORIZONTAL,
										  PANEL_LABEL_STRING, "/dev/js0",
										  PANEL_CHOICE_STRINGS,
										  "Joystick 1",
										  "Joystick 2",
										  "Joystick 3",
										  "Joystick 4",
										  NULL,
										  PANEL_VALUE, js0_mode,
										  PANEL_NOTIFY_PROC, js0_callback,
										  NULL);
		ypos += 25;
	}
	if (js1 != -1) {
		js1_item = (Panel_item) xv_create(controllers_panel, PANEL_CHOICE_STACK,
										  PANEL_VALUE_X, 150,
										  PANEL_VALUE_Y, ypos,
										  PANEL_LAYOUT, PANEL_HORIZONTAL,
										  PANEL_LABEL_STRING, "/dev/js1",
										  PANEL_CHOICE_STRINGS,
										  "Joystick 1",
										  "Joystick 2",
										  "Joystick 3",
										  "Joystick 4",
										  NULL,
										  PANEL_VALUE, js1_mode,
										  PANEL_NOTIFY_PROC, js1_callback,
										  NULL);
		ypos += 25;
	}
#endif

	xv_create(controllers_panel, PANEL_BUTTON,
			  XV_X, 130,
			  XV_Y, 125,
			  PANEL_LABEL_STRING, "OK",
			  PANEL_NOTIFY_PROC, controllers_ok_callback,
			  NULL);
/*
   ======================================
   Create Performance Configuration Frame
   ======================================
 */
	performance_frame = (Frame) xv_create(frame, FRAME_CMD,
								FRAME_LABEL, "Performance Configuration",
										  XV_WIDTH, 400,
										  XV_HEIGHT, 100,
										  NULL);

	performance_panel = (Panel) xv_get(performance_frame, FRAME_CMD_PANEL,
									   NULL);

	ypos = 10;
	refresh_slider = (Panel_item) xv_create(performance_panel, PANEL_SLIDER,
											PANEL_VALUE_X, 155,
											PANEL_VALUE_Y, ypos,
											PANEL_LAYOUT, PANEL_HORIZONTAL,
											PANEL_LABEL_STRING, "Screen Refresh Rate",
											PANEL_VALUE, Atari800_refresh_rate,
											PANEL_MIN_VALUE, 1,
											PANEL_MAX_VALUE, 32,
											PANEL_SLIDER_WIDTH, 100,
											PANEL_TICKS, 32,
											PANEL_NOTIFY_PROC, refresh_callback,
											NULL);
	ypos += 25;

	xv_create(performance_panel, PANEL_BUTTON,
			  XV_X, 180,
			  XV_Y, 75,
			  PANEL_LABEL_STRING, "OK",
			  PANEL_NOTIFY_PROC, performance_ok_callback,
			  NULL);
/*
   ====================
   Get X Window Objects
   ====================
 */
	display = (Display *) xv_get(frame, XV_DISPLAY);
	if (!display) {
		printf("Failed to open display\n");
		exit(1);
	}
	autorepeat_get();
	screen = XDefaultScreenOfDisplay(display);
	if (!screen) {
		printf("Unable to get screen\n");
		exit(1);
	}
	visual = XDefaultVisualOfScreen(screen);
	if (!visual) {
		printf("Unable to get visual\n");
		exit(1);
	}
	window = (Window) xv_get(canvas_paint_window(canvas), XV_XID);
	depth = XDefaultDepthOfScreen(screen);
	cmap = XDefaultColormapOfScreen(screen);

	chooser = (Frame) xv_create(frame, FILE_CHOOSER,
								FILE_CHOOSER_TYPE, FILE_CHOOSER_OPEN,
								NULL);

	xv_set(canvas_paint_window(canvas),
		   WIN_EVENT_PROC, event_proc,
		   WIN_CONSUME_EVENTS, WIN_ASCII_EVENTS, WIN_MOUSE_BUTTONS,
		   WIN_VISIBILITY_NOTIFY, /* mmm */
		   NULL,
		   NULL);

#endif /* XVIEW */

#ifdef MOTIF
	{
		Widget menubar;

		XmString s_system;
		XmString s_boot_disk;
		XmString s_insert_disk;
		XmString s_eject_disk;
		XmString s_disable_drive;
		XmString s_insert_cart;
		XmString s_remove_cart;
		XmString s_osa;
		XmString s_osb;
		XmString s_osxl;
		XmString s_osxe;
		XmString s_os5200;
		XmString s_exit;

		XmString s_console;
		XmString s_option;
		XmString s_select;
		XmString s_start;
		XmString s_warmstart;
		XmString s_coldstart;

		XmString s_label;

		XmString s_d1, s_d2, s_d3, s_d4;
		XmString s_d5, s_d6, s_d7, s_d8;

		char *tmpstr;
		XmString xmtmpstr;

		Arg args[8];
		int n;

		main_w = XtVaCreateManagedWidget("main_window",
									   xmMainWindowWidgetClass, toplevel,
										 NULL);

		s_system = XmStringCreateSimple("System");
		s_boot_disk = XmStringCreateSimple("Boot Disk...");
		s_insert_disk = XmStringCreateSimple("Insert Disk...");
		s_eject_disk = XmStringCreateSimple("Eject Disk");
		s_disable_drive = XmStringCreateSimple("Disable Drive");
		s_insert_cart = XmStringCreateSimple("Insert Cartridge...");
		s_remove_cart = XmStringCreateSimple("Remove Cartridge");
		s_osa = XmStringCreateSimple("Atari 800 OS/A");
		s_osb = XmStringCreateSimple("Atari 800 OS/B");
		s_osxl = XmStringCreateSimple("Atari 800XL");
		s_osxe = XmStringCreateSimple("Atari 130XE");
		s_os5200 = XmStringCreateSimple("Atari 5200");
		s_exit = XmStringCreateSimple("Exit");

		s_console = XmStringCreateSimple("Console");
		s_option = XmStringCreateSimple("Option");
		s_select = XmStringCreateSimple("Select");
		s_start = XmStringCreateSimple("Start");
		s_warmstart = XmStringCreateSimple("Warmstart");
		s_coldstart = XmStringCreateSimple("Coldstart");

		menubar = XmVaCreateSimpleMenuBar(main_w, "menubar",
										XmVaCASCADEBUTTON, s_system, 'S',
									   XmVaCASCADEBUTTON, s_console, 'C',
										  NULL);

		system_menu =
			XmVaCreateSimplePulldownMenu(menubar, "system_menu", 0, motif_system_cback,
							XmVaPUSHBUTTON, s_boot_disk, 'o', NULL, NULL,
						  XmVaPUSHBUTTON, s_insert_disk, 'I', NULL, NULL,
									XmVaCASCADEBUTTON, s_eject_disk, 'j',
								 XmVaCASCADEBUTTON, s_disable_drive, 'D',
										 XmVaSEPARATOR,
						  XmVaPUSHBUTTON, s_insert_cart, 'n', NULL, NULL,
						  XmVaPUSHBUTTON, s_remove_cart, 'R', NULL, NULL,
										 XmVaSEPARATOR,
								  XmVaPUSHBUTTON, s_osa, 'A', NULL, NULL,
								  XmVaPUSHBUTTON, s_osb, 'B', NULL, NULL,
								 XmVaPUSHBUTTON, s_osxl, 'L', NULL, NULL,
								 XmVaPUSHBUTTON, s_osxe, 'E', NULL, NULL,
							   XmVaPUSHBUTTON, s_os5200, '5', NULL, NULL,
										 XmVaSEPARATOR,
								 XmVaPUSHBUTTON, s_exit, 'x', NULL, NULL,
										 NULL);

		XmVaCreateSimplePulldownMenu(menubar, "console_menu", 1, motif_consol_cback,
							   XmVaPUSHBUTTON, s_option, 'O', NULL, NULL,
							   XmVaPUSHBUTTON, s_select, 't', NULL, NULL,
								XmVaPUSHBUTTON, s_start, 'S', NULL, NULL,
									 XmVaSEPARATOR,
							XmVaPUSHBUTTON, s_warmstart, 'W', NULL, NULL,
							XmVaPUSHBUTTON, s_coldstart, 'C', NULL, NULL,
									 NULL);

		XmStringFree(s_system);
		XmStringFree(s_boot_disk);
		XmStringFree(s_insert_disk);
		XmStringFree(s_eject_disk);
		XmStringFree(s_disable_drive);
		XmStringFree(s_insert_cart);
		XmStringFree(s_remove_cart);
		XmStringFree(s_osa);
		XmStringFree(s_osb);
		XmStringFree(s_osxl);
		XmStringFree(s_osxe);
		XmStringFree(s_os5200);
		XmStringFree(s_exit);

		XmStringFree(s_console);
		XmStringFree(s_option);
		XmStringFree(s_select);
		XmStringFree(s_start);
		XmStringFree(s_warmstart);
		XmStringFree(s_coldstart);

		XtManageChild(menubar);

		fsel_b = XmCreateFileSelectionDialog(toplevel, "boot_disk", NULL, 0);
		XtAddCallback(fsel_b, XmNokCallback, motif_boot_disk, NULL);
		XtAddCallback(fsel_b, XmNcancelCallback, motif_fs_cancel, NULL);

		fsel_d = XmCreateFileSelectionDialog(toplevel, "load_disk", NULL, 0);
		XtAddCallback(fsel_d, XmNokCallback, motif_insert_disk, NULL);
		XtAddCallback(fsel_d, XmNcancelCallback, motif_fs_cancel, NULL);

		n = 0;
		XtSetArg(args[n], XmNradioBehavior, True);
		n++;
		XtSetArg(args[n], XmNradioAlwaysOne, True);
		n++;
		XtSetArg(args[n], XmNorientation, XmHORIZONTAL);
		n++;
		rbox_d = XmCreateWorkArea(fsel_d, "rbox_d", args, n);
		XtManageChild(rbox_d);

		s_label = XmStringCreateSimple("D1:");
		n = 0;
		XtSetArg(args[n], XmNlabelString, s_label);
		n++;
		XtSetArg(args[n], XmNset, True);
		n++;
		togg_d1 = XmCreateToggleButtonGadget(rbox_d, "togg_d1", args, n);
		XtManageChild(togg_d1);
		XmStringFree(s_label);
		XtAddCallback(togg_d1, XmNarmCallback, motif_select_disk, (XtPointer) 1);

		s_label = XmStringCreateSimple("D2:");
		n = 0;
		XtSetArg(args[n], XmNlabelString, s_label);
		n++;
		togg_d2 = XmCreateToggleButtonGadget(rbox_d, "togg_d2", args, n);
		XtManageChild(togg_d2);
		XmStringFree(s_label);
		XtAddCallback(togg_d2, XmNarmCallback, motif_select_disk, (XtPointer) 2);

		s_label = XmStringCreateSimple("D3:");
		n = 0;
		XtSetArg(args[n], XmNlabelString, s_label);
		n++;
		togg_d3 = XmCreateToggleButtonGadget(rbox_d, "togg_d3", args, n);
		XtManageChild(togg_d3);
		XmStringFree(s_label);
		XtAddCallback(togg_d3, XmNarmCallback, motif_select_disk, (XtPointer) 3);

		s_label = XmStringCreateSimple("D4:");
		n = 0;
		XtSetArg(args[n], XmNlabelString, s_label);
		n++;
		togg_d4 = XmCreateToggleButtonGadget(rbox_d, "togg_d4", args, n);
		XtManageChild(togg_d4);
		XmStringFree(s_label);
		XtAddCallback(togg_d4, XmNarmCallback, motif_select_disk, (XtPointer) 4);

		s_label = XmStringCreateSimple("D5:");
		n = 0;
		XtSetArg(args[n], XmNlabelString, s_label);
		n++;
		togg_d5 = XmCreateToggleButtonGadget(rbox_d, "togg_d5", args, n);
		XtManageChild(togg_d5);
		XmStringFree(s_label);
		XtAddCallback(togg_d5, XmNarmCallback, motif_select_disk, (XtPointer) 5);

		s_label = XmStringCreateSimple("D6:");
		n = 0;
		XtSetArg(args[n], XmNlabelString, s_label);
		n++;
		togg_d6 = XmCreateToggleButtonGadget(rbox_d, "togg_d6", args, n);
		XtManageChild(togg_d6);
		XmStringFree(s_label);
		XtAddCallback(togg_d6, XmNarmCallback, motif_select_disk, (XtPointer) 6);

		s_label = XmStringCreateSimple("D7:");
		n = 0;
		XtSetArg(args[n], XmNlabelString, s_label);
		n++;
		togg_d7 = XmCreateToggleButtonGadget(rbox_d, "togg_d7", args, n);
		XtManageChild(togg_d7);
		XmStringFree(s_label);
		XtAddCallback(togg_d7, XmNarmCallback, motif_select_disk, (XtPointer) 7);

		s_label = XmStringCreateSimple("D8:");
		n = 0;
		XtSetArg(args[n], XmNlabelString, s_label);
		n++;
		togg_d8 = XmCreateToggleButtonGadget(rbox_d, "togg_d8", args, n);
		XtManageChild(togg_d8);
		XmStringFree(s_label);
		XtAddCallback(togg_d8, XmNarmCallback, motif_select_disk, (XtPointer) 8);


		fsel_r = XmCreateFileSelectionDialog(toplevel, "load_rom", NULL, 0);
		XtAddCallback(fsel_r, XmNokCallback, motif_insert_rom, NULL);
		XtAddCallback(fsel_r, XmNcancelCallback, motif_fs_cancel, NULL);

		if (UI_n_atari_files_dir > 0) {
			tmpstr = (char *) XtMalloc(strlen(UI_atari_files_dir[0] + 3));
			strcpy(Util_stpcpy(tmpstr, UI_atari_files_dir[0]), "/*");
		}
		else {
			tmpstr = (char *) XtMalloc(4);
			strcpy(tmpstr, "./*");
		}
		xmtmpstr = XmStringCreateSimple(tmpstr);
		XmFileSelectionDoSearch(fsel_b, xmtmpstr);
		XmFileSelectionDoSearch(fsel_d, xmtmpstr);
		XmStringFree(xmtmpstr);
		/* XXX: can use the same tmpstr? can use the same xmtmpstr? */
		xmtmpstr = XmStringCreateSimple(tmpstr);
		XmFileSelectionDoSearch(fsel_r, xmtmpstr);
		XmStringFree(xmtmpstr);
		XtFree(tmpstr);

		s_d1 = XmStringCreateSimple("D1:");
		s_d2 = XmStringCreateSimple("D2:");
		s_d3 = XmStringCreateSimple("D3:");
		s_d4 = XmStringCreateSimple("D4:");
		s_d5 = XmStringCreateSimple("D5:");
		s_d6 = XmStringCreateSimple("D6:");
		s_d7 = XmStringCreateSimple("D7:");
		s_d8 = XmStringCreateSimple("D8:");
		eject_menu = XmVaCreateSimplePulldownMenu(system_menu,
												  "eject_disk", 2,
												  motif_eject_cback,
								   XmVaPUSHBUTTON, s_d1, '1', NULL, NULL,
								   XmVaPUSHBUTTON, s_d2, '2', NULL, NULL,
								   XmVaPUSHBUTTON, s_d3, '3', NULL, NULL,
								   XmVaPUSHBUTTON, s_d4, '4', NULL, NULL,
								   XmVaPUSHBUTTON, s_d5, '5', NULL, NULL,
								   XmVaPUSHBUTTON, s_d6, '6', NULL, NULL,
								   XmVaPUSHBUTTON, s_d7, '7', NULL, NULL,
								   XmVaPUSHBUTTON, s_d8, '8', NULL, NULL,
												  NULL);
		disable_menu = XmVaCreateSimplePulldownMenu(system_menu,
													"disable_disk", 3,
													motif_disable_cback,
								   XmVaPUSHBUTTON, s_d1, '1', NULL, NULL,
								   XmVaPUSHBUTTON, s_d2, '2', NULL, NULL,
								   XmVaPUSHBUTTON, s_d3, '3', NULL, NULL,
								   XmVaPUSHBUTTON, s_d4, '4', NULL, NULL,
								   XmVaPUSHBUTTON, s_d5, '5', NULL, NULL,
								   XmVaPUSHBUTTON, s_d6, '6', NULL, NULL,
								   XmVaPUSHBUTTON, s_d7, '7', NULL, NULL,
								   XmVaPUSHBUTTON, s_d8, '8', NULL, NULL,
													NULL);
		XmStringFree(s_d1);
		XmStringFree(s_d2);
		XmStringFree(s_d3);
		XmStringFree(s_d4);
		XmStringFree(s_d5);
		XmStringFree(s_d6);
		XmStringFree(s_d7);
		XmStringFree(s_d8);

		drawing_area = XtVaCreateManagedWidget("Canvas",
										xmDrawingAreaWidgetClass, main_w,
											   XmNunitType, XmPIXELS,
											   XmNheight, window_height,
											   XmNwidth, window_width,
											   XmNresizePolicy, XmNONE,
											   NULL);

		XtAddEventHandler(drawing_area,
						  KeyPressMask | KeyReleaseMask | VisibilityChangeMask | FocusChangeMask, /* mmm */
						  False,
						  motif_keypress, NULL);

		XtAddEventHandler(drawing_area,
						  ExposureMask,
						  False,
						  motif_exposure, NULL);

		XtRealizeWidget(toplevel);
	}

	display = XtDisplay(drawing_area);

	window = XtWindow(drawing_area);

	screen = XDefaultScreenOfDisplay(display);
	if (!screen) {
		printf("Unable to get screen\n");
		exit(1);
	}
	visual = XDefaultVisualOfScreen(screen);
	if (!visual) {
		printf("Unable to get visual\n");
		exit(1);
	}
	depth = XDefaultDepthOfScreen(screen);
	cmap = XDefaultColormapOfScreen(screen);
#endif /* MOTIF */

#if !defined(XVIEW) && !defined(MOTIF)
	display = XOpenDisplay(NULL);
	if (!display) {
		printf("Failed to open display\n");
		exit(1);
	}
	screen = XDefaultScreenOfDisplay(display);
	if (!screen) {
		printf("Unable to get screen\n");
		exit(1);
	}
	visual = XDefaultVisualOfScreen(screen);
	if (!visual) {
		printf("Unable to get visual\n");
		exit(1);
	}
	depth = XDefaultDepthOfScreen(screen);

	if (private_cmap)
		cmap = XCreateColormap(display,
							   XRootWindowOfScreen(screen),
							   visual,
							   AllocNone);
	else
		cmap = XDefaultColormapOfScreen(screen);

	xswda.event_mask = KeyPressMask | KeyReleaseMask | ExposureMask | VisibilityChangeMask | FocusChangeMask /* mmm */;
	xswda.colormap = cmap;

	window = XCreateWindow(display,
						   XRootWindowOfScreen(screen),
						   50, 50,
						   window_width, window_height, 3, depth,
						   InputOutput, visual,
						   CWEventMask | CWBackPixel | CWColormap,
						   &xswda);

	XStoreName(display, window, Atari800_TITLE);
#endif /* !defined(XVIEW) && !defined(MOTIF) */

#ifdef SHM
	{
		int major;
		int minor;
		Bool pixmaps;
		Status status;
		int shmsize;

		status = XShmQueryVersion(display, &major, &minor, &pixmaps);
		if (!status) {
			printf("X Shared Memory extensions not available\n");
			exit(1);
		}
		printf("Using X11 Shared Memory Extensions\n");

		image = XShmCreateImage(display, visual, depth, ZPixmap,
						NULL, &shminfo, window_width, window_height);
		shmsize = (window_width * window_height *
					image->bits_per_pixel) / 8;

		shminfo.shmid = shmget(IPC_PRIVATE, shmsize, IPC_CREAT | 0777);
		shminfo.shmaddr = image->data = shmat(shminfo.shmid, 0, 0);
		shminfo.readOnly = False;

		XShmAttach(display, &shminfo);

		XSync(display, False);

		shmctl(shminfo.shmid, IPC_RMID, 0);

	}
#else
	pixmap = XCreatePixmap(display, window,
						   window_width, window_height, depth);
#endif /* SHM */

	if (depth <= 8)
		colorstep = 2;
	else
		colorstep = 1;
	for (i = 0; i < 256; i += colorstep) {
		XColor colour;

		int rgb = Colours_table[i];
		int status;

		colour.red = (rgb & 0x00ff0000) >> 8;
		colour.green = (rgb & 0x0000ff00);
		colour.blue = (rgb & 0x000000ff) << 8;

		status = XAllocColor(display,
							 cmap,
							 &colour);

		for (j = 0; j < colorstep; j++)
			colours[i + j] = colour.pixel;

#ifdef SHM
#ifdef USE_COLOUR_TRANSLATION_TABLE
		for (j = 0; j < colorstep; j++)
			colour_translation_table[i + j] = colours[i + j] | (colours[i + j] << 8);
#endif
#endif
	}

	for (i = 0; i < 256; i++) {
		xgcvl.background = colours[0];
		xgcvl.foreground = colours[i];

		gc_colour[i] = XCreateGC(display, window,
								 GCForeground | GCBackground,
								 &xgcvl);
	}

	xgcvl.background = colours[0];
	xgcvl.foreground = colours[0];

	gc = XCreateGC(display, window,
				   GCForeground | GCBackground,
				   &xgcvl);

#ifndef SHM
	XFillRectangle(display, pixmap, gc, 0, 0,
				   window_width, window_height);
	for (i = 0; i < NRECTS; i++)
		rectangles[i].height = (windowsize == Huge) ? 3 : 2;
#endif

	XMapWindow(display, window);

	XSync(display, False);
	autorepeat_get();

 /*
   ============================
   Storage for Atari 800 Screen
   ============================
 */
	image_data = (UBYTE *) Util_malloc(Screen_WIDTH * Screen_HEIGHT);

	keyboard_consol = INPUT_CONSOL_NONE;

	if (x11bug) {
		printf("Initial X11 controller configuration\n");
		printf("------------------------------------\n\n");
		printf("Keypad is ");
		Atari_WhatIs(keypad_mode);
		printf("\n");
		printf("Mouse is ");
		Atari_WhatIs(xmouse_mode);
		printf("\n");
		printf("/dev/js0 is ");
		Atari_WhatIs(js0_mode);
		printf("\n");
		printf("/dev/js1 is ");
		Atari_WhatIs(js1_mode);
		printf("\n");
	}
	signal(SIGSEGV, segmentationfault);

	return TRUE;
}

int PLATFORM_Exit(int run_monitor)
{
	int restart;

	Log_flushlog();
	if (run_monitor) {
		autorepeat_restore();
		restart = MONITOR_Run();
		autorepeat_off();
	}
	else
		restart = FALSE;

	if (!restart) {
		if (image_data != NULL)
			free(image_data);

		if (display != NULL) {
			XSync(display, True);

			if (private_cmap)
				XFreeColormap(display, cmap);

#ifdef SHM
			if (image != NULL)
				XDestroyImage(image);
#else
			XFreePixmap(display, pixmap);
#endif
			XUnmapWindow(display, window);
			XDestroyWindow(display, window);
			autorepeat_restore();
			XCloseDisplay(display);
		}

#ifdef LINUX_JOYSTICK
		if (js0 != -1)
			close(js0);

		if (js1 != -1)
			close(js1);
#endif

#ifdef SOUND
		Sound_Exit();
#endif
	}
	return restart;
}

void PLATFORM_DisplayScreen(void)
{
	static char status_line[64];
	int update_status_line = FALSE;

	if (!invisible) {
		const UBYTE *ptr2 = (const UBYTE *) Screen_atari + clipping_y * Screen_WIDTH + clipping_x;

#ifdef SHM

		int first_x = Screen_WIDTH;
		int last_x = -1000;
		int first_y = Screen_HEIGHT;
		int last_y = -1000;
		int x;
		int y;

#define SHM_SET_LAST \
		last_y = y; \
		if (x > last_x) \
			last_x = x; \
		if (x < first_x) \
			first_x = x;

#define SHM_DISPLAY_SCREEN(pixel_type) \
			pixel_type *ptr = (pixel_type *) image->data; \
			pixel_type help_color; \
			if (windowsize == Small) { \
				for (y = clipping_y; y < (clipping_y + clipping_height); y++) { \
					for (x = clipping_x; x < (clipping_x + clipping_width); x++) { \
						help_color = colours[*ptr2++]; \
						if (help_color != *ptr) { \
							SHM_SET_LAST \
							*ptr = help_color; \
						} \
						ptr++; \
					} \
					if (first_y > last_y && last_y >= 0) \
						first_y = last_y; \
					ptr2 += Screen_WIDTH - clipping_width; \
				} \
			} \
			else if (windowsize == Large) { \
				for (y = clipping_y; y < (clipping_y + clipping_height); y++) { \
					pixel_type *ptr_second_line = ptr + window_width; \
					for (x = clipping_x; x < (clipping_x + clipping_width); x++) { \
						help_color = colours[*ptr2++]; \
						if (help_color != *ptr) { \
							SHM_SET_LAST \
							ptr[0] = help_color; \
							ptr[1] = help_color; \
							ptr_second_line[0] = help_color; \
							ptr_second_line[1] = help_color; \
						} \
						ptr += 2; \
						ptr_second_line += 2; \
					} \
					if (first_y > last_y && last_y >= 0) \
						first_y = last_y; \
					ptr2 += Screen_WIDTH - clipping_width; \
					ptr += window_width; \
				} \
			} \
			else { \
				for (y = clipping_y; y < (clipping_y + clipping_height); y++) { \
					pixel_type *ptr_second_line = ptr + window_width; \
					pixel_type *ptr_third_line = ptr + window_width + window_width; \
					for (x = clipping_x; x < (clipping_x + clipping_width); x++) { \
						help_color = colours[*ptr2++]; \
						if (help_color != *ptr) { \
							SHM_SET_LAST \
							ptr[0] = help_color; \
							ptr[1] = help_color; \
							ptr[2] = help_color; \
							ptr_second_line[0] = help_color; \
							ptr_second_line[1] = help_color; \
							ptr_second_line[2] = help_color; \
							ptr_third_line[0] = help_color; \
							ptr_third_line[1] = help_color; \
							ptr_third_line[2] = help_color; \
						} \
						ptr += 3; \
						ptr_second_line += 3; \
						ptr_third_line += 3; \
					} \
					if (first_y > last_y && last_y >= 0) \
						first_y = last_y; \
					ptr2 += Screen_WIDTH - clipping_width; \
					ptr += window_width + window_width; \
				} \
			}

		if (image->bits_per_pixel == 32) {
			SHM_DISPLAY_SCREEN(ULONG)
		}
		else if (image->bits_per_pixel == 16) {
			SHM_DISPLAY_SCREEN(UWORD)
		}
		else if (image->bits_per_pixel == 8) {
			SHM_DISPLAY_SCREEN(UBYTE)
		}

		if (modified) {
			XShmPutImage(display, window, gc, image, 0, 0, 0, 0,
					 window_width, window_height, 0);
			modified = FALSE;
		}
		else if (last_y >= 0) {
			last_x++;
			last_y++;
			if (first_x < clipping_x)
				first_x = clipping_x;
			if (last_x > clipping_x + clipping_width)
				last_x = clipping_x + clipping_width;
			else if (last_x <= first_x)
				last_x = first_x + 1;
			if (first_y < clipping_y)
				first_y = clipping_y;
			if (last_y > clipping_y + clipping_height)
				last_y = clipping_y + clipping_height;
			else if (last_y <= first_y)
				last_y = first_y + 1;

			first_x *= clipping_factor;
			last_x *= clipping_factor;
			first_y *= clipping_factor;
			last_y *= clipping_factor;

			XShmPutImage(display, window, gc, image,
					 first_x - (clipping_x * clipping_factor),
					 first_y - (clipping_y * clipping_factor),
					 first_x - (clipping_x * clipping_factor),
					 first_y - (clipping_y * clipping_factor),
					 last_x - first_x, last_y - first_y, 0);
		}

		XSync(display, FALSE);

#else /* SHM */

		UBYTE *ptr = image_data + clipping_y * Screen_WIDTH + clipping_x;
		int n = 0;
		int last_colour = -1;
		int x;
		int y;

		switch (windowsize) {
		case Small:
			for (y = 0; y < clipping_height; y++) {
				for (x = 0; x < clipping_width; x++) {
					UBYTE colour = *ptr2++;
					if (colour != *ptr) {
						*ptr = colour;
						if (colour != last_colour || n >= NPOINTS) {
							if (n > 0) {
								XDrawPoints(display, pixmap, gc_colour[last_colour],
											points, n, CoordModeOrigin);
								n = 0;
								modified = TRUE;
							}
							last_colour = colour;
						}
						points[n].x = x;
						points[n].y = y;
						n++;
					}
					ptr++;
				}
				ptr += Screen_WIDTH - clipping_width;
				ptr2 += Screen_WIDTH - clipping_width;
			}
			if (n > 0) {
				XDrawPoints(display, pixmap, gc_colour[last_colour],
							points, n, CoordModeOrigin);
				modified = TRUE;
			}
			break;
		case Large:
			for (y = 0; y < window_height; y += 2) {
				for (x = 0; x < window_width; ) {
					UBYTE colour = *ptr2++;
					if (colour != *ptr) {
						int width = 2;
						*ptr++ = colour;
						if (colour != last_colour || n >= NRECTS) {
							if (n > 0) {
								XFillRectangles(display, pixmap, gc_colour[last_colour],
												rectangles, n);
								n = 0;
								modified = TRUE;
							}
							last_colour = colour;
						}
						rectangles[n].x = x;
						rectangles[n].y = y;
						while ((x += 2) < window_width && colour == *ptr2 && colour != *ptr) {
							width += 2;
							ptr2++;
							*ptr++ = colour;
						}
						rectangles[n].width = width;
						/* rectangles[n].height = 2; */
						n++;
						continue;
					}
					ptr++;
					x += 2;
				}
				ptr += Screen_WIDTH - clipping_width;
				ptr2 += Screen_WIDTH - clipping_width;
			}
			if (n > 0) {
				XFillRectangles(display, pixmap, gc_colour[last_colour],
								rectangles, n);
				modified = TRUE;
			}
			break;
		case Huge:
			for (y = 0; y < window_height; y += 3) {
				for (x = 0; x < window_width; ) {
					UBYTE colour = *ptr2++;
					if (colour != *ptr) {
						int width = 3;
						*ptr++ = colour;
						if (colour != last_colour || n >= NRECTS) {
							if (n > 0) {
								XFillRectangles(display, pixmap, gc_colour[last_colour],
												rectangles, n);
								n = 0;
								modified = TRUE;
							}
							last_colour = colour;
						}
						rectangles[n].x = x;
						rectangles[n].y = y;
						while ((x += 3) < window_width && colour == *ptr2 && colour != *ptr) {
							width += 3;
							ptr2++;
							*ptr++ = colour;
						}
						rectangles[n].width = width;
						/* rectangles[n].height = 3; */
						n++;
						continue;
					}
					ptr++;
					x += 3;
				}
				ptr2 += Screen_WIDTH - clipping_width;
				ptr += Screen_WIDTH - clipping_width;
			}
			if (n > 0) {
				XFillRectangles(display, pixmap, gc_colour[last_colour],
								rectangles, n);
				modified = TRUE;
			}
			break;
		}

		if (modified) {
			XCopyArea(display, pixmap, window, gc, 0, 0,
					  window_width, window_height, 0, 0);
			XSync(display, FALSE);
			modified = FALSE;
		}

#endif /* SHM */

	}

	switch (x11_monitor) {
	case MONITOR_SIO:
		if (SIO_status[0] != '\0') {
#ifdef XVIEW
			strcpy(status_line, SIO_status);
#else
			sprintf(status_line, "%s - %s",
					Atari800_TITLE, SIO_status);
#endif
			SIO_status[0] = '\0';
			update_status_line = TRUE;
		}
		else {
			update_status_line = FALSE;
		}
		break;
	default:
		update_status_line = FALSE;
		break;
	}

	if (update_status_line) {
#ifdef XVIEW
		xv_set(frame,
			   FRAME_LEFT_FOOTER, status_line,
			   NULL);
#else
#ifdef MOTIF
		XtVaSetValues(toplevel,
					  XtNtitle, status_line,
					  NULL);
#else
		XStoreName(display, window, status_line);
#endif
#endif
	}

}

int PLATFORM_Keyboard(void)
{
	static int keycode = AKEY_NONE;

#ifdef XVIEW
	notify_dispatch();
	XFlush(display);
#endif

#ifdef MOTIF
	while (XtAppPending(app)) {
		static XEvent event;

		XtAppNextEvent(app, &event);
		XtDispatchEvent(&event);
	}
#endif

#if defined(XVIEW) || defined(MOTIF)
	keycode = xview_keycode;
#else
	if (XEventsQueued(display, QueuedAfterFlush) > 0) {
		XEvent event;

		XNextEvent(display, &event);
		keycode = GetKeyCode(&event);
	}
#endif

	return keycode;
}

#if 0
void experimental_mouse_joystick(int mode)	/* Don't use ;-) */
{
	Window root_return;
	Window child_return;
	int root_x_return;
	int root_y_return;
	int win_x_return;
	int win_y_return;
	int mask_return;

	static int prev_x=-1,prev_y=-1;

	XQueryPointer(display, window, &root_return, &child_return,
				  &root_x_return, &root_y_return,
				  &win_x_return, &win_y_return,
				  &mask_return);

	if (mode < 5) {
		int dx,dy;
		int course,rc;

		if( prev_x<0 )	prev_x=root_x_return;
		if( prev_y<0 )	prev_y=root_y_return;
		dx=(root_x_return-prev_x)<<1;
		dy=(root_y_return-prev_y)*3;
#define Ms	8
#define Mc	3	/* Mc/Mm = 2 45 deg.  <2 => >45 deg for x or y only */
#define Mm	2
		if( dx>Ms && dy>Ms )
		{	if( dx*Mm>Mc*dy )
				course = 0x08; /* RIGHT */
			else if ( dx*Mc<dy*Mm )
				course = 0x02; /* DOWN */
			else	course = 0x0a; /* RIGHT DOWN */
		} else if( dx<-Ms && dy<-Ms )
		{	if( dx*Mm<Mc*dy )
				course = 0x04; /* LEFT */
			else if ( dx*Mc>dy*Mm )
				course = 0x01; /* UP */
			else	course = 0x05; /* LEFT UP */
		} else if( dx<-Ms && dy>Ms )
		{	if( -dx*Mm>Mc*dy )
				course = 0x04; /* LEFT */
			else if ( -dx*Mc<dy*Mm )
				course = 0x02; /* DOWN */
			else	course = 0x06; /* LEFT DOWN */
		} else if( dx>Ms && dy<-Ms )
		{	if( -dx*Mm<Mc*dy )
				course = 0x08; /* RIGHT */
			else if ( -dx*Mc>dy*Mm )
				course = 0x01; /* UP */
			else	course = 0x09; /* RIGHT UP */
		}
		else if( dx>Ms )
				course = 0x08; /* RIGHT */
		else if( dx<-Ms )
				course = 0x04; /* LEFT */
		else if( dy>Ms )
				course = 0x02; /* DOWN */
		else if( dy<-Ms )
				course = 0x01; /* UP */
		else	course=0;

		rc=(((course&0x5)<<1)|((course&0xa)>>1));
		rc&= ~mouse_stick;
		mouse_stick|=rc;
		course&=~(((rc&0x5)<<1)|((rc&0xa)>>1));
		mouse_stick&=~course;

		prev_x=root_x_return;
		prev_y=root_y_return;
	}
	else {
		if (mask_return)
			mouse_stick &= 0xfb;
	}
}
#endif

static void mouse_joystick(int mode)
{
	Window root_return;
	Window child_return;
	int root_x_return;
	int root_y_return;
	int win_x_return;
	int win_y_return;
	unsigned int mask_return;

	mouse_stick = 0x0f;

	XQueryPointer(display, window, &root_return, &child_return,
				  &root_x_return, &root_y_return,
				  &win_x_return, &win_y_return,
				  &mask_return);

	if (mode < 5) {
		int center_x;
		int center_y;
		int threshold;

		if (windowsize == Small) {
			center_x = window_width / 2;
			center_y = window_height / 2;
			threshold = 32;
		}
		else if (windowsize == Large) {
			center_x = window_width / 2;
			center_y = window_height / 2;
			threshold = 64;
		}
		else {
			center_x = window_width / 2;
			center_y = window_height / 2;
			threshold = 96;
		}

		if (win_x_return < 0 || win_x_return > center_x * 2 ||
		    win_y_return < 0 || win_y_return > center_y * 2 )
			mouse_stick = 0x0f;
		else
		{
		if (win_x_return < (center_x - threshold))
			mouse_stick &= 0xfb;
		if (win_x_return > (center_x + threshold))
			mouse_stick &= 0xf7;
		if (win_y_return < (center_y - threshold))
			mouse_stick &= 0xfe;
		if (win_y_return > (center_y + threshold))
			mouse_stick &= 0xfd;
		}
	}
	else {
		if (mask_return)
			mouse_stick &= 0xfb;
	}
}

#ifdef LINUX_JOYSTICK

static void read_joystick(int js, int centre_x, int centre_y)
{
	const int threshold = 50;
	int status;

	mouse_stick = 0x0f;

	status = read(js, &js_data, JS_RETURN);
	if (status != JS_RETURN) {
		perror("/dev/js");
		exit(1);
	}
	if (js_data.x < (centre_x - threshold))
		mouse_stick &= 0xfb;
	if (js_data.x > (centre_x + threshold))
		mouse_stick &= 0xf7;
	if (js_data.y < (centre_y - threshold))
		mouse_stick &= 0xfe;
	if (js_data.y > (centre_y + threshold))
		mouse_stick &= 0xfd;
}
#endif

int PLATFORM_PORT(int num)
{
	int nibble_0 = 0x0f;
	int nibble_1 = 0x0f;

	if (num == 0) {
		if (keypad_mode == 0)
			nibble_0 = keypad_stick;
		else if (keypad_mode == 1)
			nibble_1 = keypad_stick;

		if (INPUT_mouse_mode == INPUT_MOUSE_OFF) {
			if (xmouse_mode == 0) {
				mouse_joystick(xmouse_mode);
				nibble_0 = mouse_stick;
			}
			else if (xmouse_mode == 1) {
				mouse_joystick(xmouse_mode);
				nibble_1 = mouse_stick;
			}
		}
#ifdef LINUX_JOYSTICK
		if (js0_mode == 0) {
			read_joystick(js0, js0_centre_x, js0_centre_y);
			nibble_0 = mouse_stick;
		}
		else if (js0_mode == 1) {
			read_joystick(js0, js0_centre_x, js0_centre_y);
			nibble_1 = mouse_stick;
		}
		if (js1_mode == 0) {
			read_joystick(js1, js1_centre_x, js1_centre_y);
			nibble_0 = mouse_stick;
		}
		else if (js1_mode == 1) {
			read_joystick(js1, js1_centre_x, js1_centre_y);
			nibble_1 = mouse_stick;
		}
#endif
	}
	else {
		if (keypad_mode == 2)
			nibble_0 = keypad_stick;
		else if (keypad_mode == 3)
			nibble_1 = keypad_stick;

		if (INPUT_mouse_mode == INPUT_MOUSE_OFF) {
			if (xmouse_mode == 2) {
				mouse_joystick(xmouse_mode);
				nibble_0 = mouse_stick;
			}
			else if (xmouse_mode == 3) {
				mouse_joystick(xmouse_mode);
				nibble_1 = mouse_stick;
			}
		}

#ifdef LINUX_JOYSTICK
		if (js0_mode == 2) {
			read_joystick(js0, js0_centre_x, js0_centre_y);
			nibble_0 = mouse_stick;
		}
		else if (js0_mode == 3) {
			read_joystick(js0, js0_centre_x, js0_centre_y);
			nibble_1 = mouse_stick;
		}
		if (js1_mode == 2) {
			read_joystick(js1, js1_centre_x, js1_centre_y);
			nibble_0 = mouse_stick;
		}
		else if (js1_mode == 3) {
			read_joystick(js1, js1_centre_x, js1_centre_y);
			nibble_1 = mouse_stick;
		}
#endif
	}

	return (nibble_1 << 4) | nibble_0;
}

int PLATFORM_TRIG(int num)
{
	int trig = 1;				/* Trigger not pressed */

	if (num == keypad_mode) {
		trig = keypad_trig;
	}
	if (num == xmouse_mode) {
		Window root_return;
		Window child_return;
		int root_x_return;
		int root_y_return;
		int win_x_return;
		int win_y_return;
		unsigned int mask_return;

		if (XQueryPointer(display, window, &root_return, &child_return,
						  &root_x_return, &root_y_return,
						  &win_x_return, &win_y_return,
						  &mask_return)) {
			if (win_x_return < 0 || win_x_return > window_width ||
			    win_y_return < 0 || win_y_return > window_height)
				trig = 1;
			else if (mask_return & Button1Mask)
				trig = 0;
		}
	}
#ifdef LINUX_JOYSTICK
	if (num == js0_mode) {
		int status;

		status = read(js0, &js_data, JS_RETURN);
		if (status != JS_RETURN) {
			perror("/dev/js0");
			exit(1);
		}
		if (js_data.buttons & 0x01)
			trig = 0;
		else
			trig = 1;
	}
	if (num == js1_mode) {
		int status;

		status = read(js1, &js_data, JS_RETURN);
		if (status != JS_RETURN) {
			perror("/dev/js1");
			exit(1);
		}
		trig = (js_data.buttons & 0x0f) ? 0 : 1;
	}
#endif

	return trig;
}

void Atari_Mouse(void)
{
	static int last_x = 0;
	static int last_y = 0;
	Window root_return;
	Window child_return;
	int root_x_return;
	int root_y_return;
	int win_x_return;
	int win_y_return;
	unsigned int mask_return;

	if (INPUT_mouse_mode == INPUT_MOUSE_OFF)
		return;
	if (XQueryPointer(display, window, &root_return,
					  &child_return, &root_x_return, &root_y_return,
					  &win_x_return, &win_y_return, &mask_return)) {
		if(INPUT_direct_mouse) {
			int potx = win_x_return, poty = win_y_return;
			if(potx < 0) potx = 0;
			if(poty < 0) poty = 0;
			potx = (double)potx * (228.0 / (double)window_width);
			poty = (double)poty * (228.0 / (double)window_height);
			if(potx > 227) potx = 227;
			if(poty > 227) poty = 227;
			POKEY_POT_input[INPUT_mouse_port << 1] = 227 - potx;
			POKEY_POT_input[(INPUT_mouse_port << 1) + 1] = 227 - poty;
		} else {
			INPUT_mouse_delta_x = win_x_return - last_x;
			INPUT_mouse_delta_y = win_y_return - last_y;
			last_x = win_x_return;
			last_y = win_y_return;
		}

		INPUT_mouse_buttons = (mask_return & Button1Mask ? 1 : 0)
		              | (mask_return & Button3Mask ? 2 : 0)
		              | (mask_return & Button2Mask ? 4 : 0);
	}
}

int main(int argc, char **argv)
{
	/* initialise Atari800 core */
	if (!Atari800_Initialise(&argc, argv))
		return 3;

	/* main loop */
	for (;;) {
		INPUT_key_code = PLATFORM_Keyboard();

		if (menu_consol != INPUT_CONSOL_NONE) {
			INPUT_key_consol = menu_consol;
			menu_consol = INPUT_CONSOL_NONE;
		}
		else
			INPUT_key_consol = keyboard_consol;

		Atari_Mouse();

		Atari800_Frame();
		if (Atari800_display_screen)
			PLATFORM_DisplayScreen();
	}
}
