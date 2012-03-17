/***************************************************************************
 * Gens: (GTK+) Main Window - Synchronization Functions.                   *
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
#endif

#include <string.h>
#include <windows.h>

#include "gens_window.hpp"
#include "gens_window_sync.hpp"
#include "ui/common/gens/gens_menu.h"
#include "gens_window_callbacks.hpp"

#include "emulator/g_main.hpp"
#include "emulator/options.hpp"
#include "segacd/cd_sys.hpp"
#include "util/sound/gym.hpp"

#include "gens_core/vdp/vdp_io.h"
#include "gens_core/vdp/vdp_rend.h"

#include "gens_core/mem/mem_m68k.h"
#include "gens_core/mem/mem_m68k_cd.h"

#include "gens_core/sound/ym2612.h"
#include "gens_core/sound/psg.h"
#include "gens_core/sound/pcm.h"
#include "gens_core/sound/pwm.h"

// Renderer / Blitter selection stuff.
#include "gens_core/vdp/vdp_rend.h"
#include "gens_core/misc/cpuflags.h"

// Plugin Manager
#include "plugins/pluginmgr.hpp"

// C++ includes
#include <string>
using std::string;


/**
 * Sync_Gens_Window(): Synchronize the Gens Main Window.
 */
void Sync_Gens_Window(void)
{
	// Synchronize all menus.
	Sync_Gens_Window_FileMenu();
	Sync_Gens_Window_FileMenu_ROMHistory();
	Sync_Gens_Window_GraphicsMenu();
	Sync_Gens_Window_CPUMenu();
	Sync_Gens_Window_SoundMenu();
	Sync_Gens_Window_OptionsMenu();
}


/**
 * Sync_Gens_Window_FileMenu(): Synchronize the File Menu.
 * This does NOT synchronize the ROM History submenu, since synchronizing the
 * ROM History submenu can be slow if some ROMs are located on network shares.
 */
void Sync_Gens_Window_FileMenu(void)
{
	// Find the file menu.
	HMENU mnuFile = findMenuItem(IDM_FILE_MENU);
	
        // Netplay is currently not usable.
	EnableMenuItem(mnuFile, IDM_FILE_NETPLAY, MF_BYCOMMAND | MF_GRAYED);
	
	// Some menu items should be enabled or disabled, depending on if a game is loaded or not.
	const unsigned int enableFlags = ((Game != NULL) ? MF_ENABLED : MF_GRAYED);
	
	// Disable "Close ROM" if no ROM is loaded.
	EnableMenuItem(mnuFile, IDM_FILE_CLOSEROM, MF_BYCOMMAND | enableFlags);
	
	// Savestate menu items
	EnableMenuItem(mnuFile, IDM_FILE_LOADSTATE, MF_BYCOMMAND | enableFlags);
	EnableMenuItem(mnuFile, IDM_FILE_SAVESTATE, MF_BYCOMMAND | enableFlags);
	EnableMenuItem(mnuFile, IDM_FILE_QUICKLOAD, MF_BYCOMMAND | enableFlags);
	EnableMenuItem(mnuFile, IDM_FILE_QUICKSAVE, MF_BYCOMMAND | enableFlags);
	
	// Current savestate
	HMENU mnuChangeState = findMenuItem(IDM_FILE_CHANGESTATE);
	CheckMenuRadioItem(mnuChangeState,
			   IDM_FILE_CHANGESTATE_0,
			   IDM_FILE_CHANGESTATE_9,
			   IDM_FILE_CHANGESTATE_0 + Current_State,
			   MF_BYCOMMAND);
}


/**
 * Sync_Gens_Window_FileMenu_ROMHistory(): Synchronize the File, ROM History submenu.
 * NOTE: If some ROMs are located on network shares, this function will be SLOW,
 * since it has to check the contents of the ROM to determine its type.
 */
void Sync_Gens_Window_FileMenu_ROMHistory(void)
{
	// ROM Format prefixes
	// TODO: Move this somewhere else.
	static const char* ROM_Format_Prefix[6] = {"[----]", "[MD]", "[32X]", "[SCD]", "[SCDX]", NULL};
	
	// Find the file menu.
	HMENU mnuFile = findMenuItem(IDM_FILE_MENU);
	
	// Find the ROM History submenu.
	HMENU mnuROMHistory = findMenuItem(IDM_FILE_ROMHISTORY);
	
	// Delete and/or recreate the ROM History submenu.
#ifdef GENS_CDROM
	static const unsigned short posROMHistory = 3;
#else /* !GENS_CDROM */
	static const unsigned short posROMHistory = 2;
#endif /* GENS_CDROM */
	
	DeleteMenu(mnuFile, posROMHistory, MF_BYPOSITION);
	gensMenuMap.erase(IDM_FILE_ROMHISTORY);
	if (mnuROMHistory)
		DestroyMenu(mnuROMHistory);
	
	mnuROMHistory = CreatePopupMenu();
	InsertMenu(mnuFile, posROMHistory, MF_BYPOSITION | MF_POPUP | MF_STRING, (UINT_PTR)mnuROMHistory, "ROM &History");
	gensMenuMap.insert(win32MenuMapItem(IDM_FILE_ROMHISTORY, mnuROMHistory));
	
	string sROMHistoryEntry;
	int romFormat;
	int romsFound = 0;
	for (unsigned short i = 0; i < 9; i++)
	{
		// Make sure this Recent ROM entry actually has an entry.
		if (strlen(Recent_Rom[i]) == 0)
			continue;
		
		// Increment the ROMs Found counter.
		romsFound++;
		
		// Determine the ROM format.
		// TODO: Improve the return variable from Detect_Format()
		romFormat = ROM::detectFormat_fopen(Recent_Rom[i]) >> 1;
		if (romFormat >= 1 && romFormat <= 4)
			sROMHistoryEntry = ROM_Format_Prefix[romFormat];
		else
			sROMHistoryEntry = ROM_Format_Prefix[0];
		
		// Add a tab, a dash, and a space.
		sROMHistoryEntry += "\t- ";
		
		// Get the ROM filename.
		sROMHistoryEntry += ROM::getNameFromPath(Recent_Rom[i]);
		
		// Add the ROM item to the ROM History submenu.
		InsertMenu(mnuROMHistory, -1, MF_BYPOSITION | MF_STRING,
			   IDM_FILE_ROMHISTORY_0 + i, sROMHistoryEntry.c_str());
	}
	
	// If no recent ROMs were found, disable the ROM History menu.
	if (romsFound == 0)
		EnableMenuItem(mnuFile, posROMHistory, MF_BYPOSITION | MF_GRAYED);
}


/**
 * Sync_Gens_Window_GraphicsMenu(): Synchronize the Graphics menu.
 */
void Sync_Gens_Window_GraphicsMenu(void)
{
	HMENU mnuGraphics = findMenuItem(IDM_GRAPHICS_MENU);
	
	// Full Screen
	CheckMenuItem(mnuGraphics, IDM_GRAPHICS_FULLSCREEN,
		      MF_BYCOMMAND | (draw->fullScreen() ? MF_CHECKED : MF_UNCHECKED));
	
	// VSync
	unsigned int checkFlags;
	if (draw->fullScreen())
		checkFlags = (Video.VSync_FS ? MF_CHECKED : MF_UNCHECKED);
	else
		checkFlags = (Video.VSync_W ? MF_CHECKED : MF_UNCHECKED);
	CheckMenuItem(mnuGraphics, IDM_GRAPHICS_VSYNC, MF_BYCOMMAND | checkFlags);
	
	// Stretch
	HMENU mnuStretch = findMenuItem(IDM_GRAPHICS_STRETCH);
	CheckMenuRadioItem(mnuStretch,
			   IDM_GRAPHICS_STRETCH_NONE,
			   IDM_GRAPHICS_STRETCH_FULL,
			   IDM_GRAPHICS_STRETCH_NONE + Options::stretch(),
			   MF_BYCOMMAND);
	
	// Render
	Sync_Gens_Window_GraphicsMenu_Render(mnuGraphics, 5);
	
	// Sprite Limit
	CheckMenuItem(mnuGraphics, IDM_GRAPHICS_SPRITELIMIT,
		      MF_BYCOMMAND | (Sprite_Over ? MF_CHECKED : MF_UNCHECKED));
	
	// Frame Skip
	HMENU mnuFrameSkip = findMenuItem(IDM_GRAPHICS_FRAMESKIP);
	CheckMenuRadioItem(mnuFrameSkip,
			   IDM_GRAPHICS_FRAMESKIP_AUTO,
			   IDM_GRAPHICS_FRAMESKIP_8,
			   IDM_GRAPHICS_FRAMESKIP_AUTO + (Frame_Skip + 1),
			   MF_BYCOMMAND);
	
	// Screen Shot
	CheckMenuItem(mnuGraphics, IDM_GRAPHICS_SCREENSHOT,
		      MF_BYCOMMAND | ((Game != NULL) ? MF_CHECKED : MF_UNCHECKED));
}


/**
 * Sync_Gens_Window_GraphicsMenu_Render(): Synchronize the Graphics, Render submenu.
 * @param parent Parent menu.
 * @param position Position in the parent menu.
 */
void Sync_Gens_Window_GraphicsMenu_Render(HMENU parent, int position)
{
	HMENU mnuRender = findMenuItem(IDM_GRAPHICS_RENDER);
	
	// Delete and/or recreate the Render submenu.
	DeleteMenu(parent, position, MF_BYPOSITION);
	gensMenuMap.erase(IDM_GRAPHICS_RENDER);
	if (mnuRender)
		DestroyMenu(mnuRender);
	
	// Render submenu
	mnuRender = CreatePopupMenu();
	InsertMenu(parent, position, MF_BYPOSITION | MF_POPUP | MF_STRING, (UINT_PTR)mnuRender, "&Render");
	gensMenuMap.insert(win32MenuMapItem(IDM_GRAPHICS_RENDER, mnuRender));
	
	// Create the render entries.
	bool renderSelected = false;
	
	// Create the render entries.
	for (unsigned int i = 0; i < PluginMgr::vRenderPlugins.size(); i++)
	{
		if (draw->fullScreen())
			renderSelected = (Video.Render_FS == (int)i);
		else
			renderSelected = (Video.Render_W == (int)i);
		
		InsertMenu(mnuRender, -1, MF_BYPOSITION | MF_STRING,
			   IDM_GRAPHICS_RENDER_NORMAL + i, PluginMgr::getPluginFromID_Render(i)->tag);
		
		if (renderSelected)
		{
			CheckMenuRadioItem(mnuRender,
					   IDM_GRAPHICS_RENDER_NORMAL,
					   IDM_GRAPHICS_RENDER_NORMAL + (PluginMgr::vRenderPlugins.size() - 1),
					   IDM_GRAPHICS_RENDER_NORMAL + i,
					   MF_BYCOMMAND);
		}
	}
}


/**
 * Sync_Gens_Window_CPUMenu(): Synchronize the CPU menu.
 */
void Sync_Gens_Window_CPUMenu(void)
{
	// TODO: Figure out how to hide menu items instead of deleting/recreating them.
	
	static const unsigned int flags = MF_BYCOMMAND | MF_STRING;
	
	HMENU mnuCPU = findMenuItem(IDM_CPU_MENU);
	
#ifdef GENS_DEBUGGER
	// Synchronize the Debug submenu.
	Sync_Gens_Window_CPUMenu_Debug(mnuCPU, 0);
#endif /* GENS_DEBUGGER */
	
	// Hide and show appropriate RESET items.
	RemoveMenu(mnuCPU, IDM_CPU_RESET68K, MF_BYCOMMAND);
	RemoveMenu(mnuCPU, IDM_CPU_RESETMAIN68K, MF_BYCOMMAND);
	RemoveMenu(mnuCPU, IDM_CPU_RESETSUB68K, MF_BYCOMMAND);
	RemoveMenu(mnuCPU, IDM_CPU_RESETMAINSH2, MF_BYCOMMAND);
	RemoveMenu(mnuCPU, IDM_CPU_RESETSUBSH2, MF_BYCOMMAND);
	
	if (SegaCD_Started)
	{
		// SegaCD: Show Main 68000 and Sub 68000.
		InsertMenu(mnuCPU, IDM_CPU_RESETZ80, flags, IDM_CPU_RESETMAIN68K, "Reset Main 68000");
		InsertMenu(mnuCPU, IDM_CPU_RESETZ80, flags, IDM_CPU_RESETSUB68K, "Reset Sub 68000");
	}
	else
	{
		// No SegaCD: Only show one 68000.
		InsertMenu(mnuCPU, IDM_CPU_RESETZ80, flags, IDM_CPU_RESET68K, "Reset 68000");
	}
	
	if (_32X_Started)
	{
		// 32X: Show Main SH2 and Sub SH2.
		InsertMenu(mnuCPU, IDM_CPU_RESETZ80, flags, IDM_CPU_RESETMAINSH2, "Reset Main SH2");
		InsertMenu(mnuCPU, IDM_CPU_RESETZ80, flags, IDM_CPU_RESETSUBSH2, "Reset Sub SH2");
	}
	
	// Country code
	HMENU mnuCountry = findMenuItem(IDM_CPU_COUNTRY);
	CheckMenuRadioItem(mnuCountry,
			   IDM_CPU_COUNTRY_AUTO,
			   IDM_CPU_COUNTRY_JAPAN_PAL,
			   IDM_CPU_COUNTRY_AUTO + (Country + 1),
			   MF_BYCOMMAND);
	
	// SegaCD Perfect Sync
	CheckMenuItem(mnuCPU, IDM_CPU_SEGACDPERFECTSYNC,
		      MF_BYCOMMAND | (SegaCD_Accurate ? MF_CHECKED : MF_UNCHECKED));
}


#ifdef GENS_DEBUGGER
/**
 * Sync_Gens_Window_CPUMenu_Debug(): Synchronize the Graphics, Render submenu.
 * @param parent Parent menu.
 * @param position Position in the parent menu.
 */
void Sync_Gens_Window_CPUMenu_Debug(HMENU parent, int position)
{
	// Debug submenu
	unsigned int flags = MF_BYPOSITION | MF_POPUP | MF_STRING;
	if (Game == NULL)
		flags |= MF_GRAYED;
	
	HMENU mnuDebug = findMenuItem(IDM_CPU_DEBUG);
	
	// Delete and/or recreate the Debug submenu.
	DeleteMenu(parent, position, MF_BYPOSITION);
	gensMenuMap.erase(IDM_CPU_DEBUG);
	if (mnuDebug)
		DestroyMenu(mnuDebug);
	
	// Debug submenu
	mnuDebug = CreatePopupMenu();
	InsertMenu(parent, position, flags, (UINT_PTR)mnuDebug, "&Debug");
	gensMenuMap.insert(win32MenuMapItem(IDM_CPU_DEBUG, mnuDebug));
	
	if (flags & MF_GRAYED)
		return;
	
	// TODO: Move this array somewhere else.
	const char* DebugStr[9] =
	{
		"&Genesis - 68000",
		"Genesis - &Z80",
		"Genesis - &VDP",
		"&SegaCD - 68000",
		"SegaCD - &CDC",
		"SegaCD - GF&X",
		"32X - Main SH2",
		"32X - Sub SH2",
		"32X - VDP",
	};
	
	int i;
	
	// Create the debug entries.
	for (i = 0; i < 9; i++)
	{
		if ((i >= 0 && i <= 2) ||
		    (i >= 3 && i <= 5 && SegaCD_Started) ||
		    (i >= 6 && i <= 8 && _32X_Started))
		{
			if (i % 3 == 0 && (i >= 3 && i <= 6))
			{
				// Every three entires, add a separator.
				InsertMenu(mnuDebug, i + 1, MF_SEPARATOR,
					   IDM_CPU_DEBUG_SEGACD_SEPARATOR + ((i / 3) - 1), NULL);
			}
			
			InsertMenu(mnuDebug, i + (i / 3), MF_BYPOSITION | MF_STRING,
				   IDM_CPU_DEBUG_MC68000 + i, DebugStr[i]);
			
			if (Debug == (i + 1))
				CheckMenuItem(mnuDebug, IDM_CPU_DEBUG_MC68000 + i, MF_BYCOMMAND | MF_CHECKED);
		}
	}
}
#endif /* GENS_DEBUGGER */


/**
 * Sync_Gens_Window_SoundMenu(): Synchronize the Sound menu.
 */
void Sync_Gens_Window_SoundMenu(void)
{
	HMENU mnuSound = findMenuItem(IDM_SOUND_MENU);
	
	// Get the Enabled flag for the other menu items.
	bool soundEnabled = audio->enabled();
	
	// Enabled
	CheckMenuItem(mnuSound, IDM_SOUND_ENABLE,
		      MF_BYCOMMAND | (soundEnabled ? MF_CHECKED : MF_UNCHECKED));
	
	const uint16_t soundMenuItems[11][2] =
	{
		{IDM_SOUND_STEREO,		audio->stereo()},
		{IDM_SOUND_Z80,			Z80_State & 1},
		{IDM_SOUND_YM2612,		YM2612_Enable},
		{IDM_SOUND_YM2612_IMPROVED,	YM2612_Improv},
		{IDM_SOUND_DAC,			DAC_Enable},
		{IDM_SOUND_DAC_IMPROVED,	DAC_Improv},
		{IDM_SOUND_PSG,			PSG_Enable},
		{IDM_SOUND_PSG_SINE,		PSG_Improv},
		{IDM_SOUND_PCM,			PCM_Enable},
		{IDM_SOUND_PWM,			PWM_Enable},
		{IDM_SOUND_CDDA,		CDDA_Enable},
	};
	
	for (int i = 0; i < 11; i++)
	{
		EnableMenuItem(mnuSound, soundMenuItems[i][0],
			       MF_BYCOMMAND | (soundEnabled ? MF_ENABLED : MF_GRAYED));
		
		CheckMenuItem(mnuSound, soundMenuItems[i][0],
			      MF_BYCOMMAND | (soundMenuItems[i][1] ? MF_CHECKED : MF_UNCHECKED));
	}
	
	// Rate
	// TODO: This const array is from gens_window.c.
	// Move it somewhere else.
	const int SndRates[6][2] = {{0, 11025}, {1, 22050}, {2, 44100}};
	
	HMENU mnuRate = findMenuItem(IDM_SOUND_RATE);
	for (int i = 0; i < 3; i++)
	{
		if (SndRates[i][1] == audio->soundRate())
		{
			CheckMenuRadioItem(mnuRate,
					   IDM_SOUND_RATE_11025,
					   IDM_SOUND_RATE_44100,
					   IDM_SOUND_RATE_11025 + SndRates[i][0],
					   MF_BYCOMMAND);
			break;
		}
	}
	
	char dumpLabel[16];
	
	// WAV dumping
	// TODO: Always disabled for now, since WAV dumping isn't implemented yet.
	strcpy(dumpLabel, (audio->dumpingWAV() ? "Stop WAV Dump" : "Start WAV Dump"));
	ModifyMenu(mnuSound, IDM_SOUND_WAVDUMP,
		   MF_BYCOMMAND | MF_STRING, IDM_SOUND_WAVDUMP, dumpLabel);
	EnableMenuItem(mnuSound, IDM_SOUND_WAVDUMP,
		       MF_BYCOMMAND | MF_GRAYED);
	
	// GYM dumping
	strcpy(dumpLabel, (GYM_Dumping ? "Stop GYM Dump" : "Start GYM Dump"));
	ModifyMenu(mnuSound, IDM_SOUND_GYMDUMP, MF_BYCOMMAND | MF_STRING,
		   IDM_SOUND_GYMDUMP, dumpLabel);
	EnableMenuItem(mnuSound, IDM_SOUND_GYMDUMP,
		       MF_BYCOMMAND | ((Game != NULL) ? MF_ENABLED : MF_GRAYED));
}


/**
 * Sync_Gens_Window_SoundMenu(): Synchronize the Options menu.
 */
void Sync_Gens_Window_OptionsMenu(void)
{
	HMENU mnuOptions = findMenuItem(IDM_OPTIONS_MENU);
	
	// SegaCD SRAM Size
	int SRAM_ID = (BRAM_Ex_State & 0x100 ? BRAM_Ex_Size : -1);
	CheckMenuRadioItem(mnuOptions,
			   IDM_OPTIONS_SEGACDSRAMSIZE_NONE,
			   IDM_OPTIONS_SEGACDSRAMSIZE_64KB,
			   IDM_OPTIONS_SEGACDSRAMSIZE_NONE + (SRAM_ID + 1),
			   MF_BYCOMMAND);
}
