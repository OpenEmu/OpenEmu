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

#include "gens_window.hpp"
#include "gens_window_sync.hpp"
#include "ui/common/gens/gens_menu.h"

#include "gtk-misc.h"

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
 * Sync_Gens_Window(): Synchronize the GENS Main Window.
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
	// Disable callbacks so nothing gets screwed up.
	do_callbacks = 0;
	
	// Netplay is currently not usable.
	GtkWidget *mnuNetplay = findMenuItem(IDM_FILE_NETPLAY);
	gtk_widget_set_sensitive(mnuNetplay, FALSE);
	
	// Disable "Close ROM" if no ROM is loaded.
	gtk_widget_set_sensitive(findMenuItem(IDM_FILE_CLOSEROM), (Game != NULL));
	
	// Savestate menu items
	gboolean saveStateEnable = (Game != NULL);
	gtk_widget_set_sensitive(findMenuItem(IDM_FILE_LOADSTATE), saveStateEnable);
	gtk_widget_set_sensitive(findMenuItem(IDM_FILE_SAVESTATE), saveStateEnable);
	gtk_widget_set_sensitive(findMenuItem(IDM_FILE_QUICKLOAD), saveStateEnable);
	gtk_widget_set_sensitive(findMenuItem(IDM_FILE_QUICKSAVE), saveStateEnable);
	
	// Current savestate
	gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(findMenuItem(IDM_FILE_CHANGESTATE_0 + Current_State)), TRUE);
	
	// Enable callbacks.
	do_callbacks = 1;
}


/**
 * Sync_Gens_Window_FileMenu_ROMHistory(): Synchronize the File, ROM History submenu.
 * NOTE: If some ROMs are located on network shares, this function will be SLOW,
 * since it has to check the contents of the ROM to determine its type.
 */
void Sync_Gens_Window_FileMenu_ROMHistory(void)
{
	// Disable callbacks so nothing gets screwed up.
	do_callbacks = 0;
	
	// ROM Format prefixes
	// TODO: Move this somewhere else.
	static const char* ROM_Format_Prefix[6] = {"[----]", "[MD]", "[32X]", "[SCD]", "[SCDX]", NULL};
	
	// ROM History
	GtkWidget *mnuROMHistory = findMenuItem(IDM_FILE_ROMHISTORY);
	
	// Check if the ROM History submenu already exists.
	GtkWidget *mnuROMHistory_sub = gtk_menu_item_get_submenu(GTK_MENU_ITEM(mnuROMHistory));
	if (mnuROMHistory_sub)
	{
		// Submenu currently exists. Delete it.
		gtk_widget_destroy(mnuROMHistory_sub);
	}
	
	// Create a new submenu.
	mnuROMHistory_sub = gtk_menu_new();
	gtk_widget_set_name(mnuROMHistory_sub, "FileMenu_ROMHistory_SubMenu");
	gtk_menu_item_set_submenu(GTK_MENU_ITEM(mnuROMHistory), mnuROMHistory_sub);
	
	g_object_set_data_full(G_OBJECT(mnuROMHistory), "FileMenu_ROMHistory_SubMenu",
			       g_object_ref(mnuROMHistory_sub),
			       (GDestroyNotify)g_object_unref);
	
	GtkWidget *mnuROMHistory_item;
	string sROMHistoryEntry;
	char sMenuKey[24];
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
		sROMHistoryEntry += ROM::getNameFromPath(Recent_Rom[i]);;
		
		// Add the ROM item to the ROM History submenu.
		mnuROMHistory_item = gtk_menu_item_new_with_label(sROMHistoryEntry.c_str());
		gtk_widget_show(mnuROMHistory_item);
		gtk_container_add(GTK_CONTAINER(mnuROMHistory_sub), mnuROMHistory_item);
		
		// Make sure the menu item is deleted when the submenu is deleted.
		sprintf(sMenuKey, "ROMHistory_Sub_%d", i);
		g_object_set_data_full(G_OBJECT(mnuROMHistory_sub), sMenuKey,
				       g_object_ref(mnuROMHistory_item),
						       (GDestroyNotify)g_object_unref);
		
		// Connect the signal.
		g_signal_connect((gpointer)mnuROMHistory_item, "activate",
				  G_CALLBACK(GensWindow_GTK_MenuItemCallback),
				  GINT_TO_POINTER(IDM_FILE_ROMHISTORY_0 + i));
	}
	
	// If no recent ROMs were found, disable the ROM History menu.
	gtk_widget_set_sensitive(mnuROMHistory, romsFound);
	
	// Enable callbacks.
	do_callbacks = 1;
}


/**
 * Sync_Gens_Window_GraphicsMenu(): Synchronize the Graphics menu.
 */
void Sync_Gens_Window_GraphicsMenu(void)
{
	uint16_t id;
	
	// Disable callbacks so nothing gets screwed up.
	do_callbacks = 0;
	
	// Simple checkbox items
	gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(findMenuItem(IDM_GRAPHICS_VSYNC)), Video.VSync_W);
	gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(findMenuItem(IDM_GRAPHICS_SPRITELIMIT)), Sprite_Over);
	
	// Stretch mode
	gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(findMenuItem(IDM_GRAPHICS_STRETCH_NONE + Options::stretch())), TRUE);
	
	// Bits per pixel
	switch (bppOut)
	{
		case 15:
			id = IDM_GRAPHICS_BPP_15;
			break;
		case 16:
			id = IDM_GRAPHICS_BPP_16;
			break;
		case 32:
			id = IDM_GRAPHICS_BPP_32;
			break;
		default:
			id = 0;
			break;
	}
	
	if (id != 0)
		gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(findMenuItem(id)), TRUE);
	
	// Rebuild the Render submenu
	Sync_Gens_Window_GraphicsMenu_Render(findMenuItem(IDM_GRAPHICS_RENDER));
	
	// Selected Render Mode
	int rendMode = (draw->fullScreen() ? Video.Render_FS : Video.Render_W);
	gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(findMenuItem(IDM_GRAPHICS_RENDER_NORMAL + rendMode)), TRUE);
	
	// Frame Skip
	id = (IDM_GRAPHICS_FRAMESKIP_AUTO + 1) + Frame_Skip;
	gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(findMenuItem(id)), TRUE);
	
	// Screen Shot
	gtk_widget_set_sensitive(findMenuItem(IDM_GRAPHICS_SCREENSHOT), (Game != NULL));
	
#ifdef GENS_OPENGL
	gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(findMenuItem(IDM_GRAPHICS_OPENGL)), Video.OpenGL);
	gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(findMenuItem(IDM_GRAPHICS_OPENGL_FILTER)), Video.glLinearFilter);
	
	// OpenGL Resolution
	
	// TODO: Optimize this.
	if (Video.Width_GL == 320 && Video.Height_GL == 240)
		id = IDM_GRAPHICS_OPENGL_RES_320x240;
	else if (Video.Width_GL == 640 && Video.Height_GL == 480)
		id = IDM_GRAPHICS_OPENGL_RES_640x480;
	else if (Video.Width_GL == 800 && Video.Height_GL == 600)
		id = IDM_GRAPHICS_OPENGL_RES_800x600;
	else if (Video.Width_GL == 960 && Video.Height_GL == 720)
		id = IDM_GRAPHICS_OPENGL_RES_960x720;
	else if (Video.Width_GL == 1024 && Video.Height_GL == 768)
		id = IDM_GRAPHICS_OPENGL_RES_1024x768;
	else if (Video.Width_GL == 1280 && Video.Height_GL == 960)
		id = IDM_GRAPHICS_OPENGL_RES_1280x960;
	else if (Video.Width_GL == 1280 && Video.Height_GL == 1024)
		id = IDM_GRAPHICS_OPENGL_RES_1280x1024;
	else if (Video.Width_GL == 1400 && Video.Height_GL == 1050)
		id = IDM_GRAPHICS_OPENGL_RES_1400x1050;
	else if (Video.Width_GL == 1600 && Video.Height_GL == 1200)
		id = IDM_GRAPHICS_OPENGL_RES_1600x1200;
	else
		id = IDM_GRAPHICS_OPENGL_RES_CUSTOM;
	
	gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(findMenuItem(id)), TRUE);
	
	// Set the text of the custom resolution item.
	GtkWidget *mnuGLResCustom = findMenuItem(IDM_GRAPHICS_OPENGL_RES_CUSTOM);
	if (id == IDM_GRAPHICS_OPENGL_RES_CUSTOM)
	{
		// Custom resolution. Set the text.
		char sCustomRes[32];
		sprintf(sCustomRes, "Custom... (%dx%d)", Video.Width_GL, Video.Height_GL);
		gtk_label_set_text(GTK_LABEL(GTK_BIN(mnuGLResCustom)->child), sCustomRes);
	}
	else
	{
		// Predefined resolution.
		gtk_label_set_text(GTK_LABEL(GTK_BIN(mnuGLResCustom)->child), "Custom...");
	}
#endif
	
	// Enable callbacks.
	do_callbacks = 1;
}


/**
 * Sync_Gens_Window_GraphicsMenu_Render(): Synchronize the Graphics, Render submenu.
 * @param container Container for this menu.
 */
void Sync_Gens_Window_GraphicsMenu_Render(GtkWidget *container)
{
	if (!container)
		return;
	
	GtkWidget *mnuItem;
	GSList *radioGroup = NULL;
	
	char sObjName[64];
	
	// Check if the Render submenu already exists.
	GtkWidget *mnuSubMenu = gtk_menu_item_get_submenu(GTK_MENU_ITEM(container));
	if (mnuSubMenu)
	{
		// Submenu currently exists. Delete it.
		gtk_widget_destroy(mnuSubMenu);
	}
	
	// Create a new submenu.
	mnuSubMenu = gtk_menu_new();
	gtk_widget_set_name(mnuSubMenu, "GraphicsMenu_Render_SubMenu");
	gtk_menu_item_set_submenu(GTK_MENU_ITEM(container), mnuSubMenu);
	
	g_object_set_data_full(G_OBJECT(container), "GraphicsMenu_Render_SubMenu",
			       g_object_ref(mnuSubMenu),
			       (GDestroyNotify)g_object_unref);
	
	// Create the render entries.
	for (unsigned int i = 0; i < PluginMgr::vRenderPlugins.size(); i++)
	{
		// Delete the menu item from the map, if it exists.
		gensMenuMap.erase(IDM_GRAPHICS_RENDER_NORMAL + i);
		
		sprintf(sObjName, "GraphicsMenu_Render_SubMenu_%d", i);
		
		mnuItem = gtk_radio_menu_item_new_with_mnemonic(radioGroup, PluginMgr::getPluginFromID_Render(i)->tag);
		radioGroup = gtk_radio_menu_item_get_group(GTK_RADIO_MENU_ITEM(mnuItem));
		gtk_widget_set_name(mnuItem, sObjName);
		gtk_widget_show(mnuItem);
		gtk_container_add(GTK_CONTAINER(mnuSubMenu), mnuItem);
		
		// Make sure the menu item is deleted when the submenu is deleted.
		g_object_set_data_full(G_OBJECT(mnuSubMenu), sObjName,
				       g_object_ref(mnuItem),
				       (GDestroyNotify)g_object_unref);
		
		// Connect the signal.
		g_signal_connect((gpointer)mnuItem, "activate",
				  G_CALLBACK(GensWindow_GTK_MenuItemCallback),
				  GINT_TO_POINTER(IDM_GRAPHICS_RENDER_NORMAL + i));
		
		// Add the menu item to the map.
		gensMenuMap.insert(gtkMenuMapItem(IDM_GRAPHICS_RENDER_NORMAL + i, mnuItem));
	}
}


/**
 * Sync_Gens_Window_CPUMenu(): Synchronize the CPU menu.
 */
void Sync_Gens_Window_CPUMenu(void)
{
	// Disable callbacks so nothing gets screwed up.
	do_callbacks = 0;
	
#ifdef GENS_DEBUGGER
	gtk_widget_set_sensitive(findMenuItem(IDM_CPU_DEBUG), (Game != NULL));
	
	// Hide/Show debug entries depending on the active console.
	if (Game != NULL)
	{
		GtkWidget *mnuDebugItem;
		bool checkDebug;
		for (unsigned short i = 1; i <= 9; i++)
		{
			// TODO: Use debug constants instead?
			if (i >= 1 && i <= 3)
				checkDebug = 1;
			else if (i >= 4 && i <= 6)
				checkDebug = SegaCD_Started;
			else if (i >= 7 && i <= 9)
				checkDebug = _32X_Started;
			else
			{
				// Shouldn't happen...
				fprintf(stderr, "%s: ERROR: i == %d\n", __func__, i);
				checkDebug = 0;
			}
			
			mnuDebugItem = findMenuItem(IDM_CPU_DEBUG + i);
			if (checkDebug)
				gtk_widget_show(mnuDebugItem);
			else
				gtk_widget_hide(mnuDebugItem);
			
			// Make sure the check state for this debug item is correct.
			gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(mnuDebugItem), (Debug == i));
		}
		
		// Separators
		mnuDebugItem = findMenuItem(IDM_CPU_DEBUG_SEGACD_SEPARATOR);
		if (SegaCD_Started)
			gtk_widget_show(mnuDebugItem);
		else
			gtk_widget_hide(mnuDebugItem);
		
		mnuDebugItem = findMenuItem(IDM_CPU_DEBUG_32X_SEPARATOR);
		if (_32X_Started)
			gtk_widget_show(mnuDebugItem);
		else
			gtk_widget_hide(mnuDebugItem);
	}
#endif /* GENS_DEBUGGER */
	
	// Country code
	gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(findMenuItem(IDM_CPU_COUNTRY_AUTO + Country + 1)), TRUE);
	
	// Hide and show the appropriate RESET items.
	GtkWidget *mnuReset68K  = findMenuItem(IDM_CPU_RESET68K);
	GtkWidget *mnuResetM68K = findMenuItem(IDM_CPU_RESETMAIN68K);
	GtkWidget *mnuResetS68K = findMenuItem(IDM_CPU_RESETSUB68K);
	GtkWidget *mnuResetMSH2 = findMenuItem(IDM_CPU_RESETMAINSH2);
	GtkWidget *mnuResetSSH2 = findMenuItem(IDM_CPU_RESETSUBSH2);
	
	if (SegaCD_Started)
	{
		// SegaCD: Hide regular 68000; show Main 68000 and Sub 68000.
		gtk_widget_hide(mnuReset68K);
		gtk_widget_show(mnuResetM68K);
		gtk_widget_show(mnuResetS68K);
	}
	else
	{
		// No SegaCD: Show regular 68000; hide Main 68000 and Sub 68000;
		gtk_widget_show(mnuReset68K);
		gtk_widget_hide(mnuResetM68K);
		gtk_widget_hide(mnuResetS68K);
	}
	
	if (_32X_Started)
	{
		// 32X: Show Main SH2 and Sub SH2.
		gtk_widget_show(mnuResetMSH2);
		gtk_widget_show(mnuResetSSH2);
	}
	else
	{
		// 32X: Hide Main SH2 and Sub SH2.
		gtk_widget_hide(mnuResetMSH2);
		gtk_widget_hide(mnuResetSSH2);
	}
	
	// SegaCD Perfect Sync
	gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(findMenuItem(IDM_CPU_SEGACDPERFECTSYNC)), SegaCD_Accurate);
	
	// Enable callbacks.
	do_callbacks = 1;
}


/**
 * Sync_Gens_Window_SoundMenu(): Synchronize the Sound menu.
 */
void Sync_Gens_Window_SoundMenu(void)
{
	// Disable callbacks so nothing gets screwed up.
	do_callbacks = 0;
	
	// Get the Enabled flag for the other menu items.
	bool soundEnabled = audio->enabled();
	
	// Enabled
	gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(findMenuItem(IDM_SOUND_ENABLE)), soundEnabled);
	
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
	
	GtkWidget *mnuItem;
	for (int i = 0; i < 11; i++)
	{
		mnuItem = findMenuItem(soundMenuItems[i][0]);
		
		gtk_widget_set_sensitive(mnuItem, soundEnabled);
		gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(mnuItem), soundMenuItems[i][1]);
	}
	
	// Rate
	// TODO: Improve this by using a hash or something.
	uint16_t id;
	switch (audio->soundRate())
	{
		case 11025:
			id = IDM_SOUND_RATE_11025;
			break;
		case 22050:
			id = IDM_SOUND_RATE_22050;
			break;
		case 44100:
			id = IDM_SOUND_RATE_44100;
			break;
		default:
			// Default to 22,050 Hz.
			id = IDM_SOUND_RATE_22050;
			break;
	}
	gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(findMenuItem(id)), TRUE);
	
	const char* sLabel;
	
	// WAV dumping
	sLabel = (audio->dumpingWAV() ? "Stop WAV Dump" : "Start WAV Dump");
	GtkWidget *mnuWAVDump = findMenuItem(IDM_SOUND_WAVDUMP);
	gtk_label_set_text(GTK_LABEL(GTK_BIN(mnuWAVDump)->child), sLabel);
	
	// GYM dumping
	sLabel = (GYM_Dumping ? "Stop GYM Dump" : "Start GYM Dump");
	GtkWidget *mnuGYMDump = findMenuItem(IDM_SOUND_GYMDUMP);
	gtk_label_set_text(GTK_LABEL(GTK_BIN(mnuGYMDump)->child), sLabel);
	
	// Enable or disable GYM/WAV dumping, depending on if a game is running or not.
	// Also, don't enable this if sound is disabled.
	
	bool allowAudioDump = (Game != NULL) && audio->enabled();
	
	// WAV dump
	// TODO: Change from FALSE to allowAudioDump after WAV dumping has been reimplemented.
	gtk_widget_set_sensitive(mnuWAVDump, FALSE);
	
	// GYM dump
	gtk_widget_set_sensitive(mnuGYMDump, allowAudioDump);
	
	// Enable callbacks.
	do_callbacks = 1;
}


/**
 * Sync_Gens_Window_SoundMenu(): Synchronize the Options menu.
 */
void Sync_Gens_Window_OptionsMenu(void)
{
	// Disable callbacks so nothing gets screwed up.
	do_callbacks = 0;
	
	// SegaCD SRAM Size
	int bramID = ((BRAM_Ex_State & 0x100) ? BRAM_Ex_Size : -1);
	GtkWidget *mnuBRAMSize = findMenuItem(IDM_OPTIONS_SEGACDSRAMSIZE_NONE + (bramID + 1));
	gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(mnuBRAMSize), TRUE);
	
	// Enable callbacks.
	do_callbacks = 1;
}
