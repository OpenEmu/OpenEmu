/***************************************************************************
 * Gens: Main Menu definitions.                                            *
 *                                                                         *
 * Copyright (c) 1999-2002 by Stéphane Dallongeville                       *
 * Copyright (c) 2003-2004 by Stéphane Akhoun                              *
 * Copyright (c) 2008 by David Korth                                       *
 *                                                                         *
 * This program is free software; you can redistribute it and/or modify it *
 * under the terms of the GNU General Public License as published by the   *
 * Free Software Foundation; either version 2 of the License, or (at your  *
 * option) any later version.                                              *
 *                                                                         *
 * This program is distributed in the hope that it will be useful, but     *
 * WITHOUT ANY WARRANTY; without even the implied warranty of              *
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the           *
 * GNU General Public License for more details.                            *
 *                                                                         *
 * You should have received a copy of the GNU General Public License along *
 * with this program; if not, write to the Free Software Foundation, Inc., *
 * 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.           *
 ***************************************************************************/

#ifndef GENS_MENU_H
#define GENS_MENU_H

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif /* HAVE_CONFIG_H */

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>

#define GMF_ITEM_NORMAL		0x0000
#define GMF_ITEM_SEPARATOR	0x0001
#define GMF_ITEM_SUBMENU	0x0002
#define GMF_ITEM_CHECK		0x0003
#define GMF_ITEM_RADIO		0x0004
#define GMF_ITEM_MASK		0x000F

// Accelerator modifiers
#define GMAM_CTRL		0x0001
#define GMAM_ALT		0x0002
#define GMAM_SHIFT		0x0004

// Accelerator keys
#define GMAK_BACKSPACE		0x08
#define GMAK_TAB		0x09
#define GMAK_ENTER		0x0D

#define GMAK_F1			0x0201
#define GMAK_F2			0x0202
#define GMAK_F3			0x0203
#define GMAK_F4			0x0204
#define GMAK_F5			0x0205
#define GMAK_F6			0x0206
#define GMAK_F7			0x0207
#define GMAK_F8			0x0208
#define GMAK_F9			0x0209
#define GMAK_F10		0x020A
#define GMAK_F11		0x020B
#define GMAK_F12		0x020C

// Menu icon IDs.
#define IDIM_MENU_ICON		0x0100
#define IDIM_OPEN		(IDIM_MENU_ICON + 1)
#define IDIM_SAVE		(IDIM_MENU_ICON + 2)
#define IDIM_SAVE_AS		(IDIM_MENU_ICON + 3)
#define IDIM_REFRESH		(IDIM_MENU_ICON + 4)
#define IDIM_REVERT		(IDIM_MENU_ICON + 5)
#define IDIM_CLOSE		(IDIM_MENU_ICON + 6)
#define IDIM_QUIT		(IDIM_MENU_ICON + 7)
#define IDIM_CDROM		(IDIM_MENU_ICON + 8)
#define IDIM_CDROM_DRIVE	(IDIM_MENU_ICON + 9)
#define IDIM_NETPLAY		(IDIM_MENU_ICON + 10)
#define IDIM_ROMHISTORY		(IDIM_MENU_ICON + 11)
#define IDIM_GAME_GENIE		(IDIM_MENU_ICON + 12)
#define IDIM_FULLSCREEN		(IDIM_MENU_ICON + 13)
#define IDIM_COLOR_ADJUST	(IDIM_MENU_ICON + 14)
#define IDIM_RENDER		(IDIM_MENU_ICON + 15)
#define IDIM_FRAMESKIP		(IDIM_MENU_ICON + 16)
#define IDIM_SCREENSHOT		(IDIM_MENU_ICON + 17)
#define IDIM_RESET		(IDIM_MENU_ICON + 18)
#define IDIM_GENERAL_OPTIONS	(IDIM_MENU_ICON + 19)
#define IDIM_JOYPADS		(IDIM_MENU_ICON + 20)
#define IDIM_DIRECTORIES	(IDIM_MENU_ICON + 21)
#define IDIM_BIOSMISCFILES	(IDIM_MENU_ICON + 22)
#define IDIM_MEMORY		(IDIM_MENU_ICON + 23)
#define IDIM_HELP		(IDIM_MENU_ICON + 24)
#define IDIM_MENU_ICON_MAX	IDIM_HELP

struct GensMenuItem_t
{
	uint16_t id;				// Menu identifier. (0xFFFF == separator)
	uint16_t flags;				// Menu item flags.
	char* text;				// Menu text.
	struct GensMenuItem_t* submenu;		// First element of submenu.
	
	// Accelerator. (Set both values to 0 for no accelerator.)
	uint16_t accelModifier;
	uint16_t accelKey;
	
	uint16_t icon;				// Icon ID.
};

extern struct GensMenuItem_t gmiMain[];

// Menu #defines
// Menu identifier & 0xF000 == the menu it's from

#define IDM_SEPARATOR			0xFFFF

// File Menu
#define IDM_FILE_MENU			0x1000
#define IDM_FILE_OPENROM		0x1001
#ifdef GENS_CDROM
#define IDM_FILE_BOOTCD			0x1002
#endif /* GENS_CDROM */
#define IDM_FILE_NETPLAY		0x1003
#define IDM_FILE_CLOSEROM		0x1004
#define IDM_FILE_GAMEGENIE		0x1005
#define IDM_FILE_LOADSTATE		0x1006
#define IDM_FILE_SAVESTATE		0x1007
#define IDM_FILE_QUICKLOAD		0x1008
#define IDM_FILE_QUICKSAVE		0x1009

#define IDM_FILE_ROMHISTORY		0x1100
#define IDM_FILE_ROMHISTORY_0		(IDM_FILE_ROMHISTORY + 1)
#define IDM_FILE_ROMHISTORY_1		(IDM_FILE_ROMHISTORY + 2)
#define IDM_FILE_ROMHISTORY_2		(IDM_FILE_ROMHISTORY + 3)
#define IDM_FILE_ROMHISTORY_3		(IDM_FILE_ROMHISTORY + 4)
#define IDM_FILE_ROMHISTORY_4		(IDM_FILE_ROMHISTORY + 5)
#define IDM_FILE_ROMHISTORY_5		(IDM_FILE_ROMHISTORY + 6)
#define IDM_FILE_ROMHISTORY_6		(IDM_FILE_ROMHISTORY + 7)
#define IDM_FILE_ROMHISTORY_7		(IDM_FILE_ROMHISTORY + 8)
#define IDM_FILE_ROMHISTORY_8		(IDM_FILE_ROMHISTORY + 9)
#define IDM_FILE_ROMHISTORY_9		(IDM_FILE_ROMHISTORY + 10)

#define IDM_FILE_CHANGESTATE		0x1200
#define IDM_FILE_CHANGESTATE_0		(IDM_FILE_CHANGESTATE + 1)
#define IDM_FILE_CHANGESTATE_1		(IDM_FILE_CHANGESTATE + 2)
#define IDM_FILE_CHANGESTATE_2		(IDM_FILE_CHANGESTATE + 3)
#define IDM_FILE_CHANGESTATE_3		(IDM_FILE_CHANGESTATE + 4)
#define IDM_FILE_CHANGESTATE_4		(IDM_FILE_CHANGESTATE + 5)
#define IDM_FILE_CHANGESTATE_5		(IDM_FILE_CHANGESTATE + 6)
#define IDM_FILE_CHANGESTATE_6		(IDM_FILE_CHANGESTATE + 7)
#define IDM_FILE_CHANGESTATE_7		(IDM_FILE_CHANGESTATE + 8)
#define IDM_FILE_CHANGESTATE_8		(IDM_FILE_CHANGESTATE + 9)
#define IDM_FILE_CHANGESTATE_9		(IDM_FILE_CHANGESTATE + 10)

#define IDM_FILE_QUIT			0x1FFF
#define IDM_FILE_EXIT			0x1FFF

// Graphics Menu
#define IDM_GRAPHICS_MENU		0x2000
#define IDM_GRAPHICS_FULLSCREEN		0x2001
#define IDM_GRAPHICS_VSYNC		0x2002
#define IDM_GRAPHICS_OPENGL		0x2004
#define IDM_GRAPHICS_OPENGL_FILTER	0x2005
#define IDM_GRAPHICS_COLORADJUST	0x2006
#define IDM_GRAPHICS_SPRITELIMIT	0x2007
#define IDM_GRAPHICS_SCREENSHOT		0x2008

#define IDM_GRAPHICS_STRETCH		0x2100
#define IDM_GRAPHICS_STRETCH_NONE	0x2101
#define IDM_GRAPHICS_STRETCH_H		0x2102
#define IDM_GRAPHICS_STRETCH_V		0x2103
#define IDM_GRAPHICS_STRETCH_FULL	0x2104

#define IDM_GRAPHICS_OPENGL_RES			0x2200
#define IDM_GRAPHICS_OPENGL_RES_320x240		(IDM_GRAPHICS_OPENGL_RES + 1)
#define IDM_GRAPHICS_OPENGL_RES_640x480		(IDM_GRAPHICS_OPENGL_RES + 2)
#define IDM_GRAPHICS_OPENGL_RES_800x600		(IDM_GRAPHICS_OPENGL_RES + 3)
#define IDM_GRAPHICS_OPENGL_RES_960x720		(IDM_GRAPHICS_OPENGL_RES + 4)
#define IDM_GRAPHICS_OPENGL_RES_1024x768	(IDM_GRAPHICS_OPENGL_RES + 5)
#define IDM_GRAPHICS_OPENGL_RES_1280x960	(IDM_GRAPHICS_OPENGL_RES + 6)
#define IDM_GRAPHICS_OPENGL_RES_1280x1024	(IDM_GRAPHICS_OPENGL_RES + 7)
#define IDM_GRAPHICS_OPENGL_RES_1400x1050	(IDM_GRAPHICS_OPENGL_RES + 8)
#define IDM_GRAPHICS_OPENGL_RES_1600x1200	(IDM_GRAPHICS_OPENGL_RES + 9)
#define IDM_GRAPHICS_OPENGL_RES_CUSTOM		0x22FF

#define IDM_GRAPHICS_BPP		0x2300
#define IDM_GRAPHICS_BPP_15		0x2301 // 15-bit color (555)
#define IDM_GRAPHICS_BPP_16		0x2302 // 16-bit color (565)
#define IDM_GRAPHICS_BPP_32		0x2303 // 32-bit color

#define IDM_GRAPHICS_FRAMESKIP		0x2400
#define IDM_GRAPHICS_FRAMESKIP_AUTO	(IDM_GRAPHICS_FRAMESKIP + 1)
#define IDM_GRAPHICS_FRAMESKIP_0	(IDM_GRAPHICS_FRAMESKIP + 2)
#define IDM_GRAPHICS_FRAMESKIP_1	(IDM_GRAPHICS_FRAMESKIP + 3)
#define IDM_GRAPHICS_FRAMESKIP_2	(IDM_GRAPHICS_FRAMESKIP + 4)
#define IDM_GRAPHICS_FRAMESKIP_3	(IDM_GRAPHICS_FRAMESKIP + 5)
#define IDM_GRAPHICS_FRAMESKIP_4	(IDM_GRAPHICS_FRAMESKIP + 6)
#define IDM_GRAPHICS_FRAMESKIP_5	(IDM_GRAPHICS_FRAMESKIP + 7)
#define IDM_GRAPHICS_FRAMESKIP_6	(IDM_GRAPHICS_FRAMESKIP + 8)
#define IDM_GRAPHICS_FRAMESKIP_7	(IDM_GRAPHICS_FRAMESKIP + 9)
#define IDM_GRAPHICS_FRAMESKIP_8	(IDM_GRAPHICS_FRAMESKIP + 10)

#define IDM_GRAPHICS_RENDER		0x2800
#define IDM_GRAPHICS_RENDER_NORMAL	0x2801
#define IDM_GRAPHICS_RENDER_DOUBLE	0x2802

// CPU Menu
#define IDM_CPU_MENU			0x3000
#define IDM_CPU_HARDRESET		0x3001
#define IDM_CPU_RESET68K		0x3002
#define IDM_CPU_RESETMAIN68K		0x3003
#define IDM_CPU_RESETSUB68K		0x3004
#define IDM_CPU_RESETMAINSH2		0x3005
#define IDM_CPU_RESETSUBSH2		0x3006
#define IDM_CPU_RESETZ80		0x3007
#define IDM_CPU_SEGACDPERFECTSYNC	0x3008

#ifdef GENS_DEBUGGER
#define IDM_CPU_DEBUG			0x3100
#define IDM_CPU_DEBUG_MC68000		0x3101
#define IDM_CPU_DEBUG_Z80		0x3102
#define IDM_CPU_DEBUG_VDP		0x3103
#define IDM_CPU_DEBUG_SEGACD_MC68000	0x3104
#define IDM_CPU_DEBUG_SEGACD_CDC	0x3105
#define IDM_CPU_DEBUG_SEGACD_GFX	0x3106
#define IDM_CPU_DEBUG_32X_MSH2		0x3107
#define IDM_CPU_DEBUG_32X_SSH2		0x3108
#define IDM_CPU_DEBUG_32X_VDP		0x3109
#define IDM_CPU_DEBUG_SEGACD_SEPARATOR	0x310A
#define IDM_CPU_DEBUG_32X_SEPARATOR	0x310B
#endif /* GENS_DEBUGGER */

#define IDM_CPU_COUNTRY			0x3200
#define IDM_CPU_COUNTRY_AUTO		0x3201
#define IDM_CPU_COUNTRY_JAPAN_NTSC	0x3202
#define IDM_CPU_COUNTRY_USA		0x3203
#define IDM_CPU_COUNTRY_EUROPE		0x3204
#define IDM_CPU_COUNTRY_JAPAN_PAL	0x3205
#define IDM_CPU_COUNTRY_ORDER		0x3206

// Sound Menu
#define IDM_SOUND_MENU			0x4000
#define IDM_SOUND_ENABLE		0x4001
#define IDM_SOUND_STEREO		0x4002
#define IDM_SOUND_Z80			0x4003
#define IDM_SOUND_YM2612		0x4004
#define IDM_SOUND_YM2612_IMPROVED	0x4005
#define IDM_SOUND_DAC			0x4006
#define IDM_SOUND_DAC_IMPROVED		0x4007
#define IDM_SOUND_PSG			0x4008
#define IDM_SOUND_PSG_SINE		0x4009
#define IDM_SOUND_PCM			0x400A
#define IDM_SOUND_PWM			0x400B
#define IDM_SOUND_CDDA			0x400C
#define IDM_SOUND_WAVDUMP		0x400D
#define IDM_SOUND_GYMDUMP		0x400E

#define IDM_SOUND_RATE			0x4100
#define IDM_SOUND_RATE_11025		(IDM_SOUND_RATE + 1)
#define IDM_SOUND_RATE_22050		(IDM_SOUND_RATE + 2)
#define IDM_SOUND_RATE_44100		(IDM_SOUND_RATE + 3)

// Options Menu
#define IDM_OPTIONS_MENU		0x5000
#define IDM_OPTIONS_GENERAL		0x5001
#define IDM_OPTIONS_JOYPADS		0x5002
#define IDM_OPTIONS_DIRECTORIES		0x5003
#define IDM_OPTIONS_BIOSMISCFILES	0x5004
#ifdef GENS_CDROM
#define IDM_OPTIONS_CURRENT_CD_DRIVE	0x5005
#endif /* GENS_CDROM */
#define IDM_OPTIONS_LOADCONFIG		0x5006
#define IDM_OPTIONS_SAVECONFIGAS	0x5007
#define IDM_OPTIONS_PLUGINMANAGER	0x5008

#define IDM_OPTIONS_SEGACDSRAMSIZE	0x5100
#define IDM_OPTIONS_SEGACDSRAMSIZE_NONE	(IDM_OPTIONS_SEGACDSRAMSIZE + 1)
#define IDM_OPTIONS_SEGACDSRAMSIZE_8KB	(IDM_OPTIONS_SEGACDSRAMSIZE + 2)
#define IDM_OPTIONS_SEGACDSRAMSIZE_16KB	(IDM_OPTIONS_SEGACDSRAMSIZE + 3)
#define IDM_OPTIONS_SEGACDSRAMSIZE_32KB	(IDM_OPTIONS_SEGACDSRAMSIZE + 4)
#define IDM_OPTIONS_SEGACDSRAMSIZE_64KB	(IDM_OPTIONS_SEGACDSRAMSIZE + 5)

// Help Menu
#define IDM_HELP_MENU			0x6000
#define IDM_HELP_ABOUT			0x6001

#ifdef __cplusplus
}
#endif

#endif /* GENS_MENU_H */
