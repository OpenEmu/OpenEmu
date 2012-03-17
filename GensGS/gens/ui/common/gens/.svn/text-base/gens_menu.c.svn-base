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

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif /* HAVE_CONFIG_H */

#include "gens_menu.h"

#include <unistd.h>
#ifndef NULL
#define NULL ((void*)0)
#endif


/** Main Menu **/


static struct GensMenuItem_t gmiFile[];
static struct GensMenuItem_t gmiGraphics[];
static struct GensMenuItem_t gmiCPU[];
static struct GensMenuItem_t gmiSound[];
static struct GensMenuItem_t gmiOptions[];
static struct GensMenuItem_t gmiHelp[];

struct GensMenuItem_t gmiMain[] =
{
	{IDM_FILE_MENU,		GMF_ITEM_SUBMENU,	"&File",		&gmiFile[0],		0, 0, 0},
	{IDM_GRAPHICS_MENU,	GMF_ITEM_SUBMENU,	"&Graphics",		&gmiGraphics[0],	0, 0, 0},
	{IDM_CPU_MENU,		GMF_ITEM_SUBMENU,	"&CPU",			&gmiCPU[0],		0, 0, 0},
	{IDM_SOUND_MENU,	GMF_ITEM_SUBMENU,	"&Sound",		&gmiSound[0],		0, 0, 0},
	{IDM_OPTIONS_MENU,	GMF_ITEM_SUBMENU,	"&Options",		&gmiOptions[0],		0, 0, 0},
	{IDM_HELP_MENU,		GMF_ITEM_SUBMENU,	"&Help",		&gmiHelp[0],		0, 0, 0},
	{0, 0, NULL, NULL, 0, 0, 0}
};


/** File Menu **/


static struct GensMenuItem_t gmiFile_ChgState[];

static struct GensMenuItem_t gmiFile[] =
{
	{IDM_FILE_OPENROM,	GMF_ITEM_NORMAL,	"&Open ROM...",		NULL,	GMAM_CTRL, 'O', IDIM_OPEN},
#ifdef GENS_CDROM
	{IDM_FILE_BOOTCD,	GMF_ITEM_NORMAL,	"&Boot CD",		NULL,	GMAM_CTRL, 'B', IDIM_CDROM},
#endif /* GENS_CDROM */
	{IDM_FILE_NETPLAY,	GMF_ITEM_NORMAL,	"&Netplay",		NULL,	0, 0, IDIM_NETPLAY},
	{IDM_FILE_ROMHISTORY,	GMF_ITEM_SUBMENU,	"ROM &History", 	NULL, 0, 0, IDIM_ROMHISTORY},
	{IDM_FILE_CLOSEROM,	GMF_ITEM_NORMAL,	"&Close ROM",		NULL,	GMAM_CTRL, 'W', IDIM_CLOSE},
	{IDM_SEPARATOR,		GMF_ITEM_SEPARATOR,	NULL,			NULL,	0, 0, 0},
	{IDM_FILE_GAMEGENIE,	GMF_ITEM_NORMAL,	"&Game Genie",		NULL,	GMAM_CTRL, 'G', IDIM_GAME_GENIE},
	{IDM_SEPARATOR,		GMF_ITEM_SEPARATOR,	NULL,			NULL,	0, 0, 0},
	{IDM_FILE_LOADSTATE,	GMF_ITEM_NORMAL,	"&Load State...",	NULL,	GMAM_SHIFT, GMAK_F8, IDIM_OPEN},
	{IDM_FILE_SAVESTATE,	GMF_ITEM_NORMAL,	"&Save State As...",	NULL,	GMAM_SHIFT, GMAK_F5, IDIM_SAVE_AS},
	{IDM_FILE_QUICKLOAD,	GMF_ITEM_NORMAL,	"Quick Load",		NULL,	0, GMAK_F8, IDIM_REFRESH},
	{IDM_FILE_QUICKSAVE,	GMF_ITEM_NORMAL,	"Quick Save",		NULL,	0, GMAK_F5, IDIM_SAVE},
	{IDM_FILE_CHANGESTATE,	GMF_ITEM_SUBMENU, 	"Change State",		&gmiFile_ChgState[0], 0, 0, IDIM_REVERT},
	{IDM_SEPARATOR,		GMF_ITEM_SEPARATOR,	NULL,			NULL,	0, 0, 0},
#ifdef GENS_OS_WIN32
	{IDM_FILE_EXIT,		GMF_ITEM_NORMAL,	"E&xit",		NULL,	GMAM_CTRL, 'Q', IDIM_QUIT},
#else /* !GENS_OS_WIN32 */
 	{IDM_FILE_QUIT,		GMF_ITEM_NORMAL,	"&Quit",		NULL,	GMAM_CTRL, 'Q', IDIM_QUIT},
#endif /* GENS_OS_WIN32 */
	{0, 0, NULL, NULL, 0, 0, 0}
};

static struct GensMenuItem_t gmiFile_ChgState[] =
{
	{IDM_FILE_CHANGESTATE_0, GMF_ITEM_RADIO, "0", NULL, 0, 0, 0},
	{IDM_FILE_CHANGESTATE_1, GMF_ITEM_RADIO, "1", NULL, 0, 0, 0},
	{IDM_FILE_CHANGESTATE_2, GMF_ITEM_RADIO, "2", NULL, 0, 0, 0},
	{IDM_FILE_CHANGESTATE_3, GMF_ITEM_RADIO, "3", NULL, 0, 0, 0},
	{IDM_FILE_CHANGESTATE_4, GMF_ITEM_RADIO, "4", NULL, 0, 0, 0},
	{IDM_FILE_CHANGESTATE_5, GMF_ITEM_RADIO, "5", NULL, 0, 0, 0},
	{IDM_FILE_CHANGESTATE_6, GMF_ITEM_RADIO, "6", NULL, 0, 0, 0},
	{IDM_FILE_CHANGESTATE_7, GMF_ITEM_RADIO, "7", NULL, 0, 0, 0},
	{IDM_FILE_CHANGESTATE_8, GMF_ITEM_RADIO, "8", NULL, 0, 0, 0},
	{IDM_FILE_CHANGESTATE_9, GMF_ITEM_RADIO, "9", NULL, 0, 0, 0},
	{0, 0, NULL, NULL, 0, 0, 0}
};


/** Graphics Menu **/


static struct GensMenuItem_t gmiGraphics_Stretch[];
#ifdef GENS_OPENGL
static struct GensMenuItem_t gmiGraphics_GLRes[];
#endif
#ifdef GENS_OS_UNIX
static struct GensMenuItem_t gmiGraphics_bpp[];
#endif
static struct GensMenuItem_t gmiGraphics_FrameSkip[];

static struct GensMenuItem_t gmiGraphics[] =
{
	{IDM_GRAPHICS_FULLSCREEN,	GMF_ITEM_NORMAL,	"&Full Screen",			NULL,	GMAM_ALT, GMAK_ENTER, IDIM_FULLSCREEN},
	{IDM_GRAPHICS_VSYNC,		GMF_ITEM_CHECK,		"&VSync",			NULL,	GMAM_SHIFT, GMAK_F3, 0},
	{IDM_GRAPHICS_STRETCH,		GMF_ITEM_SUBMENU,	"&Stretch",			&gmiGraphics_Stretch[0], GMAM_SHIFT, GMAK_F2, 0},
	{IDM_SEPARATOR,			GMF_ITEM_SEPARATOR,	NULL,				NULL,	0, 0, 0},
#ifdef GENS_OPENGL
	{IDM_GRAPHICS_OPENGL,		GMF_ITEM_CHECK,		"Open&GL",			NULL,	GMAM_SHIFT, 'R', 0},
	{IDM_GRAPHICS_OPENGL_FILTER,	GMF_ITEM_CHECK,		"OpenGL &Linear Filter",	NULL,	0, 0, 0},
	{IDM_GRAPHICS_OPENGL_RES,	GMF_ITEM_SUBMENU,	"OpenGL Resolution",		&gmiGraphics_GLRes[0],	0, 0, 0},
#ifndef GENS_OS_UNIX
	{IDM_SEPARATOR,			GMF_ITEM_SEPARATOR,	NULL,				NULL,	0, 0, 0},
#endif /* !GENS_OS_UNIX */
#endif /* GENS_OPENGL */
#ifdef GENS_OS_UNIX
	{IDM_GRAPHICS_BPP,		GMF_ITEM_SUBMENU,	"Bits per pixel",		&gmiGraphics_bpp[0],	0, 0, 0},
	{IDM_SEPARATOR,			GMF_ITEM_SEPARATOR,	NULL,				NULL,	0, 0, 0},
#endif /* GENS_OS_UNIX */
	{IDM_GRAPHICS_COLORADJUST,	GMF_ITEM_NORMAL,	"Color Adjust...",		NULL,	0, 0, IDIM_COLOR_ADJUST},
	{IDM_GRAPHICS_RENDER,		GMF_ITEM_SUBMENU,	"&Render",			NULL,	0, 0, IDIM_RENDER},
	{IDM_SEPARATOR,			GMF_ITEM_SEPARATOR,	NULL,				NULL,	0, 0, 0},
	{IDM_GRAPHICS_SPRITELIMIT,	GMF_ITEM_CHECK,		"Sprite Limit",			NULL,	0, 0, 0},
	{IDM_SEPARATOR,			GMF_ITEM_SEPARATOR,	NULL,				NULL,	0, 0, 0},
	{IDM_GRAPHICS_FRAMESKIP,	GMF_ITEM_SUBMENU,	"Frame Skip",			&gmiGraphics_FrameSkip[0], 0, 0, IDIM_FRAMESKIP},
	{IDM_GRAPHICS_SCREENSHOT,	GMF_ITEM_NORMAL,	"Screen Shot",			NULL,	GMAM_SHIFT, GMAK_BACKSPACE, IDIM_SCREENSHOT},
	{0, 0, NULL, NULL, 0, 0, 0}
};

static struct GensMenuItem_t gmiGraphics_Stretch[] =
{
	{IDM_GRAPHICS_STRETCH_NONE,	GMF_ITEM_RADIO,		"&None",		NULL, 0, 0, 0},
	{IDM_GRAPHICS_STRETCH_H,	GMF_ITEM_RADIO,		"&Horizontal Only",	NULL, 0, 0, 0},
	{IDM_GRAPHICS_STRETCH_V,	GMF_ITEM_RADIO,		"&Vertical Only",	NULL, 0, 0, 0},
	{IDM_GRAPHICS_STRETCH_FULL,	GMF_ITEM_RADIO,		"&Full Stretch",	NULL, 0, 0, 0},
	{0, 0, NULL, NULL, 0, 0, 0}
};

#ifdef GENS_OPENGL
static struct GensMenuItem_t gmiGraphics_GLRes[] =
{
	{IDM_GRAPHICS_OPENGL_RES_320x240,	GMF_ITEM_RADIO,		"320x240 (1x)",		NULL,	0, 0, 0},
	{IDM_GRAPHICS_OPENGL_RES_640x480,	GMF_ITEM_RADIO,		"640x480 (2x)",		NULL,	0, 0, 0},
	{IDM_GRAPHICS_OPENGL_RES_800x600,	GMF_ITEM_RADIO,		"800x600",		NULL,	0, 0, 0},
	{IDM_GRAPHICS_OPENGL_RES_960x720,	GMF_ITEM_RADIO,		"960x720 (3x)",		NULL,	0, 0, 0},
	{IDM_GRAPHICS_OPENGL_RES_1024x768,	GMF_ITEM_RADIO,		"1024x768",		NULL,	0, 0, 0},
	{IDM_GRAPHICS_OPENGL_RES_1280x960,	GMF_ITEM_RADIO,		"1280x960 (4x)",	NULL,	0, 0, 0},
	{IDM_GRAPHICS_OPENGL_RES_1280x1024,	GMF_ITEM_RADIO,		"1280x1024",		NULL,	0, 0, 0},
	{IDM_GRAPHICS_OPENGL_RES_1400x1050,	GMF_ITEM_RADIO,		"1400x1050",		NULL,	0, 0, 0},
	{IDM_GRAPHICS_OPENGL_RES_1600x1200,	GMF_ITEM_RADIO,		"1600x1200",		NULL,	0, 0, 0},
	{IDM_GRAPHICS_OPENGL_RES_CUSTOM,	GMF_ITEM_RADIO,		"Custom",		NULL,	0, 0, 0},
	{0, 0, NULL, NULL, 0, 0, 0}
};
#endif

#ifdef GENS_OS_UNIX
static struct GensMenuItem_t gmiGraphics_bpp[] =
{
	{IDM_GRAPHICS_BPP_15,	GMF_ITEM_RADIO,		"15 (555)",	NULL,	0, 0, 0},
	{IDM_GRAPHICS_BPP_16,	GMF_ITEM_RADIO,		"16 (565)",	NULL,	0, 0, 0},
	{IDM_GRAPHICS_BPP_32,	GMF_ITEM_RADIO,		"32",		NULL,	0, 0, 0},
	{0, 0, NULL, NULL, 0, 0, 0}
};
#endif

static struct GensMenuItem_t gmiGraphics_FrameSkip[] =
{
	{IDM_GRAPHICS_FRAMESKIP_AUTO,	GMF_ITEM_RADIO,		"Auto",		NULL,	0, 0, 0},
	{IDM_GRAPHICS_FRAMESKIP_0,	GMF_ITEM_RADIO,		"0",		NULL,	0, 0, 0},
	{IDM_GRAPHICS_FRAMESKIP_1,	GMF_ITEM_RADIO,		"1",		NULL,	0, 0, 0},
	{IDM_GRAPHICS_FRAMESKIP_2,	GMF_ITEM_RADIO,		"2",		NULL,	0, 0, 0},
	{IDM_GRAPHICS_FRAMESKIP_3,	GMF_ITEM_RADIO,		"3",		NULL,	0, 0, 0},
	{IDM_GRAPHICS_FRAMESKIP_4,	GMF_ITEM_RADIO,		"4",		NULL,	0, 0, 0},
	{IDM_GRAPHICS_FRAMESKIP_5,	GMF_ITEM_RADIO,		"5",		NULL,	0, 0, 0},
	{IDM_GRAPHICS_FRAMESKIP_6,	GMF_ITEM_RADIO,		"6",		NULL,	0, 0, 0},
	{IDM_GRAPHICS_FRAMESKIP_7,	GMF_ITEM_RADIO,		"7",		NULL,	0, 0, 0},
	{IDM_GRAPHICS_FRAMESKIP_8,	GMF_ITEM_RADIO,		"8",		NULL,	0, 0, 0},
	{0, 0, NULL, NULL, 0, 0, 0}
};


/** CPU Menu **/


#ifdef GENS_DEBUGGER
static struct GensMenuItem_t gmiCPU_Debug[];
#endif /* GENS_DEBUGGER */
static struct GensMenuItem_t gmiCPU_Country[];

static struct GensMenuItem_t gmiCPU[] =
{
#ifdef GENS_DEBUGGER
	{IDM_CPU_DEBUG,			GMF_ITEM_SUBMENU,	"&Debug",			&gmiCPU_Debug[0], 0, 0, 0},
	{IDM_SEPARATOR,			GMF_ITEM_SEPARATOR,	NULL,				NULL,	0, 0, 0},
#endif /* GENS_DEBUGGER */
	{IDM_CPU_COUNTRY,		GMF_ITEM_SUBMENU,	"&Country",			&gmiCPU_Country[0], 0, 0, 0},
	{IDM_SEPARATOR,			GMF_ITEM_SEPARATOR,	NULL,				NULL,	0, 0, 0},
	{IDM_CPU_HARDRESET,		GMF_ITEM_NORMAL,	"Hard &Reset",			NULL,	0, GMAK_TAB, IDIM_RESET},
	{IDM_CPU_RESET68K,		GMF_ITEM_NORMAL,	"Reset 68000",			NULL,	0, 0, 0},
	{IDM_CPU_RESETMAIN68K,		GMF_ITEM_NORMAL,	"Reset Main 68000",		NULL,	0, 0, 0},
	{IDM_CPU_RESETSUB68K,		GMF_ITEM_NORMAL,	"Reset Sub 68000",		NULL,	0, 0, 0},
	{IDM_CPU_RESETMAINSH2,		GMF_ITEM_NORMAL,	"Reset Main SH2",		NULL,	0, 0, 0},
	{IDM_CPU_RESETSUBSH2,		GMF_ITEM_NORMAL,	"Reset Sub SH2",		NULL,	0, 0, 0},
	{IDM_CPU_RESETZ80,		GMF_ITEM_NORMAL,	"Reset Z80",			NULL,	0, 0, 0},
	{IDM_SEPARATOR,			GMF_ITEM_SEPARATOR,	NULL,				NULL,	0, 0, 0},
	{IDM_CPU_SEGACDPERFECTSYNC,	GMF_ITEM_CHECK,		"SegaCD Perfect Sync (SLOW)",	NULL,	0, 0, 0},
	{0, 0, NULL, NULL, 0, 0, 0}
};

#ifdef GENS_DEBUGGER
static struct GensMenuItem_t gmiCPU_Debug[] =
{
	{IDM_CPU_DEBUG_MC68000,			GMF_ITEM_CHECK,		"&Genesis - 680000",	NULL, 0, 0, 0},
	{IDM_CPU_DEBUG_Z80,			GMF_ITEM_CHECK,		"Genesis - &Z80",	NULL, 0, 0, 0},
	{IDM_CPU_DEBUG_VDP,			GMF_ITEM_CHECK,		"Genesis - &VDP",	NULL, 0, 0, 0},
	{IDM_CPU_DEBUG_SEGACD_SEPARATOR,	GMF_ITEM_SEPARATOR,	NULL,			NULL, 0, 0, 0},
	{IDM_CPU_DEBUG_SEGACD_MC68000,		GMF_ITEM_CHECK,		"&SegaCD - 68000",	NULL, 0, 0, 0},
	{IDM_CPU_DEBUG_SEGACD_CDC,		GMF_ITEM_CHECK,		"SegaCD - &CDC",	NULL, 0, 0, 0},
	{IDM_CPU_DEBUG_SEGACD_GFX,		GMF_ITEM_CHECK,		"SegaCD - GF&X",	NULL, 0, 0, 0},
	{IDM_CPU_DEBUG_32X_SEPARATOR,		GMF_ITEM_SEPARATOR,	NULL,			NULL, 0, 0, 0},
	{IDM_CPU_DEBUG_32X_MSH2,		GMF_ITEM_CHECK,		"32X - Main SH2",	NULL, 0, 0, 0},
	{IDM_CPU_DEBUG_32X_SSH2,		GMF_ITEM_CHECK,		"32X - Sub SH2",	NULL, 0, 0, 0},
	{IDM_CPU_DEBUG_32X_VDP,			GMF_ITEM_CHECK,		"32X - VDP",		NULL, 0, 0, 0},
	{0, 0, NULL, NULL, 0, 0, 0}
};
#endif /* GENS_DEBUGGER */

static struct GensMenuItem_t gmiCPU_Country[] =
{
	{IDM_CPU_COUNTRY_AUTO,		GMF_ITEM_RADIO,		"Auto Detect",		NULL, 0, 0, 0},
	{IDM_CPU_COUNTRY_JAPAN_NTSC,	GMF_ITEM_RADIO,		"Japan (NTSC)",		NULL, 0, 0, 0},
	{IDM_CPU_COUNTRY_USA,		GMF_ITEM_RADIO,		"USA (NTSC)",		NULL, 0, 0, 0},
	{IDM_CPU_COUNTRY_EUROPE,	GMF_ITEM_RADIO,		"Europe (PAL)",		NULL, 0, 0, 0},
	{IDM_CPU_COUNTRY_JAPAN_PAL,	GMF_ITEM_RADIO,		"Japan (PAL)",		NULL, 0, 0, 0},
	{IDM_SEPARATOR,			GMF_ITEM_SEPARATOR,	NULL,			NULL, 0, 0, 0},
	{IDM_CPU_COUNTRY_ORDER,		GMF_ITEM_NORMAL,	"Auto-Detection Order...", NULL, 0, 0, 0},
	{0, 0, NULL, NULL, 0, 0, 0}
};


/** Sound Menu **/


static struct GensMenuItem_t gmiSound_Rate[];

static struct GensMenuItem_t gmiSound[] =
{
	{IDM_SOUND_ENABLE,		GMF_ITEM_CHECK,		"&Enable",		NULL, 0, 0, 0},
	{IDM_SEPARATOR,			GMF_ITEM_SEPARATOR,	NULL,			NULL, 0, 0, 0},
	{IDM_SOUND_RATE,		GMF_ITEM_SUBMENU,	"&Rate",		&gmiSound_Rate[0], 0, 0, 0},
	{IDM_SOUND_STEREO,		GMF_ITEM_CHECK,		"&Stereo",		NULL, 0, 0, 0},
	{IDM_SEPARATOR,			GMF_ITEM_SEPARATOR,	NULL,			NULL, 0, 0, 0},
	{IDM_SOUND_Z80,			GMF_ITEM_CHECK,		"&Z80",			NULL, 0, 0, 0},
	{IDM_SEPARATOR,			GMF_ITEM_SEPARATOR,	NULL,			NULL, 0, 0, 0},
	{IDM_SOUND_YM2612,		GMF_ITEM_CHECK,		"&YM2612",		NULL, 0, 0, 0},
	{IDM_SOUND_YM2612_IMPROVED,	GMF_ITEM_CHECK,		"YM2612 Improved",	NULL, 0, 0, 0},
	{IDM_SOUND_DAC,			GMF_ITEM_CHECK,		"&DAC",			NULL, 0, 0, 0},
	{IDM_SOUND_DAC_IMPROVED,	GMF_ITEM_CHECK,		"DAC Improved",		NULL, 0, 0, 0},
	{IDM_SOUND_PSG,			GMF_ITEM_CHECK,		"&PSG",			NULL, 0, 0, 0},
	{IDM_SOUND_PSG_SINE,		GMF_ITEM_CHECK,		"PSG (Sine Wave)",	NULL, 0, 0, 0},
	{IDM_SOUND_PCM,			GMF_ITEM_CHECK,		"P&CM",			NULL, 0, 0, 0},
	{IDM_SOUND_PWM,			GMF_ITEM_CHECK,		"P&WM",			NULL, 0, 0, 0},
	{IDM_SOUND_CDDA,		GMF_ITEM_CHECK,		"CDDA (CD Audio)",	NULL, 0, 0, 0},
	{IDM_SEPARATOR,			GMF_ITEM_SEPARATOR,	NULL,			NULL, 0, 0, 0},
	{IDM_SOUND_WAVDUMP,		GMF_ITEM_NORMAL,	"Start WAV Dump",	NULL, 0, 0, 0},
	{IDM_SOUND_GYMDUMP,		GMF_ITEM_NORMAL,	"Start GYM Dump",	NULL, 0, 0, 0},
	{0, 0, NULL, NULL, 0, 0, 0}
};

static struct GensMenuItem_t gmiSound_Rate[] =
{
	{IDM_SOUND_RATE_11025,		GMF_ITEM_RADIO,		"11,025 Hz",		NULL, 0, 0, 0},
	{IDM_SOUND_RATE_22050,		GMF_ITEM_RADIO,		"22,050 Hz",		NULL, 0, 0, 0},
	{IDM_SOUND_RATE_44100,		GMF_ITEM_RADIO,		"44,100 Hz",		NULL, 0, 0, 0},
	{0, 0, NULL, NULL, 0, 0, 0}
};


/** Options Menu **/


static struct GensMenuItem_t gmiOptions_SegaCDSRAMSize[];

static struct GensMenuItem_t gmiOptions[] =
{
	{IDM_OPTIONS_GENERAL,		GMF_ITEM_NORMAL,	"&General Options...",	NULL, 0, 0, IDIM_GENERAL_OPTIONS},
	{IDM_OPTIONS_JOYPADS,		GMF_ITEM_NORMAL,	"&Joypads...",		NULL, 0, 0, IDIM_JOYPADS},
	{IDM_OPTIONS_DIRECTORIES,	GMF_ITEM_NORMAL,	"&Directories...",	NULL, 0, 0, IDIM_DIRECTORIES},
	{IDM_OPTIONS_BIOSMISCFILES,	GMF_ITEM_NORMAL,	"&BIOS/Misc Files...",	NULL, 0, 0, IDIM_BIOSMISCFILES},
	{IDM_SEPARATOR,			GMF_ITEM_SEPARATOR,	NULL,			NULL, 0, 0, 0},
#ifdef GENS_CDROM
	{IDM_OPTIONS_CURRENT_CD_DRIVE,	GMF_ITEM_NORMAL,	"Current &CD Drive...",	NULL, 0, 0, IDIM_CDROM_DRIVE},
#endif /* GENS_CDROM */
	{IDM_OPTIONS_SEGACDSRAMSIZE,	GMF_ITEM_SUBMENU,	"Sega CD S&RAM Size", &gmiOptions_SegaCDSRAMSize[0], 0, 0, IDIM_MEMORY},
	{IDM_SEPARATOR,			GMF_ITEM_SEPARATOR,	NULL,			NULL, 0, 0, 0},
	{IDM_OPTIONS_LOADCONFIG,	GMF_ITEM_NORMAL,	"&Load Config...",	NULL, 0, 0, IDIM_OPEN},
	{IDM_OPTIONS_SAVECONFIGAS,	GMF_ITEM_NORMAL,	"&Save Config As...",	NULL, 0, 0, IDIM_SAVE_AS},
	{IDM_SEPARATOR,			GMF_ITEM_SEPARATOR,	NULL,			NULL, 0, 0, 0},
	{IDM_OPTIONS_PLUGINMANAGER,	GMF_ITEM_NORMAL,	"&Plugin Manager",	NULL, 0, 0, 0},
	{0, 0, NULL, NULL, 0, 0, 0}
};

static struct GensMenuItem_t gmiOptions_SegaCDSRAMSize[] =
{
	{IDM_OPTIONS_SEGACDSRAMSIZE_NONE,	GMF_ITEM_RADIO, "None",  NULL, 0, 0, 0},
	{IDM_OPTIONS_SEGACDSRAMSIZE_8KB,	GMF_ITEM_RADIO, "8 KB",  NULL, 0, 0, 0},
	{IDM_OPTIONS_SEGACDSRAMSIZE_16KB,	GMF_ITEM_RADIO, "16 KB", NULL, 0, 0, 0},
	{IDM_OPTIONS_SEGACDSRAMSIZE_32KB,	GMF_ITEM_RADIO, "32 KB", NULL, 0, 0, 0},
	{IDM_OPTIONS_SEGACDSRAMSIZE_64KB,	GMF_ITEM_RADIO, "64 KB", NULL, 0, 0, 0},
	{0, 0, NULL, NULL, 0, 0, 0}
};


/** Help Menu **/


static struct GensMenuItem_t gmiHelp[] =
{
	{IDM_HELP_ABOUT,	GMF_ITEM_NORMAL,	"&About",	NULL,	0, 0, IDIM_HELP},
	{0, 0, NULL, NULL, 0, 0, 0}
};
