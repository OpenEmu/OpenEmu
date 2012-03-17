/***************************************************************************
 * Gens: Configuration file handler.                                       *
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

#include <stdio.h>
#include <unistd.h>
#include <algorithm>
#include <cstring>

#include "save.hpp"
#include "config_file.hpp"
#include "port/ini.hpp"

#include "emulator/gens.hpp"
#include "emulator/g_main.hpp"
#include "emulator/g_input.hpp"
#include "util/gfx/imageutil.hpp"
#include "util/file/ggenie.h"
#include "gens_core/misc/misc.h"
#include "emulator/g_palette.h"
#include "emulator/g_update.hpp"

// CPU
#include "gens_core/cpu/68k/cpu_68k.h"
#include "gens_core/cpu/sh2/cpu_sh2.h"
#include "gens_core/cpu/sh2/sh2.h"
#include "gens_core/cpu/z80/z80.h"

// Memory
#include "gens_core/mem/mem_m68k.h"
#include "gens_core/mem/mem_m68k_cd.h"
#include "gens_core/mem/mem_s68k.h"
#include "gens_core/mem/mem_sh2.h"
#include "gens_core/mem/mem_z80.h"
#include "gens_core/io/io.h"
#include "gens_core/io/io_teamplayer.h"
#include "util/file/rom.hpp"

// VDP
#include "gens_core/vdp/vdp_io.h"
#include "gens_core/vdp/vdp_rend.h"
#include "gens_core/vdp/vdp_32x.h"

// Audio
#include "gens_core/sound/ym2612.h"
#include "gens_core/sound/psg.h"
#include "gens_core/sound/pcm.h"
#include "gens_core/sound/pwm.h"

// SegaCD
#include "segacd/cd_sys.hpp"
#include "gens_core/gfx/gfx_cd.h"
#include "segacd/lc89510.h"
#include "segacd/cd_file.h"

// CD-ROM drive access
#ifdef GENS_CDROM
#include "segacd/cd_aspi.hpp"
#endif /* GENS_CDROM */

// UI
#include "emulator/options.hpp"
#include "ui/gens_ui.hpp"

// Plugin Manager
#include "plugins/pluginmgr.hpp"

// Needed for SetCurrentDirectory.
#ifdef GENS_OS_WIN32
#include <windows.h>
#endif /* GENS_OS_WIN32 */


// TODO: Make this a global array somewhere better.
const char* PlayerNames[8] = {"P1", "P1B", "P1C", "P1D", "P2", "P2B", "P2C", "P2D"};


/**
 * save(): Save Gens configuration.
 * @param filename Configuration filename.
 */
int Config::save(const string& filename)
{
	int i;
	char buf[256];
	
#ifdef GENS_OS_WIN32
	SetCurrentDirectory(PathNames.Gens_EXE_Path);
#endif /* GENS_OS_WIN32 */
	
	// Load the configuration file into the INI handler.
	INI cfg(filename);
	
	// Paths
	cfg.writeString("General", "ROM Path", Rom_Dir);
	cfg.writeString("General", "Save Path", State_Dir);
	cfg.writeString("General", "SRAM Path", SRAM_Dir);
	cfg.writeString("General", "BRAM Path", BRAM_Dir);
	cfg.writeString("General", "Dump Path", PathNames.Dump_WAV_Dir);
	cfg.writeString("General", "Dump GYM Path", PathNames.Dump_GYM_Dir);
	cfg.writeString("General", "Screen Shot Path", PathNames.Screenshot_Dir);
	cfg.writeString("General", "Patch Path", Patch_Dir);
	cfg.writeString("General", "IPS Patch Path", IPS_Dir);
	
	// Genesis BIOS
	cfg.writeString("General", "Genesis BIOS", BIOS_Filenames.MD_TMSS);
	
	// SegaCD BIOSes
	cfg.writeString("General", "USA CD BIOS", BIOS_Filenames.SegaCD_US);
	cfg.writeString("General", "Europe CD BIOS", BIOS_Filenames.MegaCD_EU);
	cfg.writeString("General", "Japan CD BIOS", BIOS_Filenames.MegaCD_JP);
	
	// 32X BIOSes
	cfg.writeString("General", "32X 68000 BIOS", BIOS_Filenames._32X_MC68000);
	cfg.writeString("General", "32X Master SH2 BIOS", BIOS_Filenames._32X_MSH2);
	cfg.writeString("General", "32X Slave SH2 BIOS", BIOS_Filenames._32X_SSH2);
	
	// Last 9 ROMs
	for (i = 0; i < 9; i++)
	{
		sprintf(buf, "ROM %d", i + 1);
		cfg.writeString("General", buf, Recent_Rom[i]);
	}
	
#ifdef GENS_CDROM
	// Physical CD-ROM support for SegaCD
#if defined(GENS_OS_WIN32)
	// Win32 ASPI uses a device ID number.
	cfg.writeInt("Options", "CD Drive", cdromDeviceID);
#elif defined(GENS_OS_UNIX)
	// Unix uses a device name.
	cfg.writeString("General", "CD Drive", cdromDeviceName);
#endif /* GENS_OS_WIN32 / GENS_OS_UNIX */
	cfg.writeInt("General", "CD Speed", cdromSpeed);
#endif /* GENS_CDROM */
	
	cfg.writeInt("General", "State Number", Current_State);
	cfg.writeInt("General", "Language", Language);
	cfg.writeInt("General", "Window X", Window_Pos.x);
	cfg.writeInt("General", "Window Y", Window_Pos.y);
	cfg.writeInt("General", "Intro Style", Intro_Style);
	cfg.writeInt("General", "Free Mode Color", draw->introEffectColor());
	
	// Video adjustments
	cfg.writeInt("Graphics", "Contrast", Contrast_Level);
	cfg.writeInt("Graphics", "Brightness", Brightness_Level);
	cfg.writeInt("Graphics", "Greyscale", Greyscale & 1);
	cfg.writeInt("Graphics", "Invert", Invert_Color & 1);
	
	// Video settings
	// Render_Mode is incremented by 1 for compatibility with old Gens.
	cfg.writeBool("Graphics", "Full Screen", draw->fullScreen());
	cfg.writeString("Graphics", "Render Fullscreen", PluginMgr::getPluginFromID_Render(Video.Render_FS)->tag);
	cfg.writeString("Graphics", "Render Windowed", PluginMgr::getPluginFromID_Render(Video.Render_W)->tag);
	cfg.writeInt("Graphics", "Full Screen VSync", Video.VSync_FS & 1);
	cfg.writeInt("Graphics", "Windows VSync", Video.VSync_W & 1);
	cfg.writeBool("Graphics", "Border Color Emulation", Video.borderColorEmulation);
	cfg.writeBool("Graphics", "Pause Tint", Video.pauseTint);
	
#ifndef GENS_OS_WIN32
	cfg.writeInt("Graphics", "Bits Per Pixel", bppOut);
#endif /* GENS_OS_WIN32 */
	
#ifdef GENS_OPENGL
	cfg.writeInt("Graphics", "Render OpenGL", (Video.OpenGL ? 1 : 0));
	cfg.writeInt("Graphics", "OpenGL Width", Video.Width_GL);
	cfg.writeInt("Graphics", "OpenGL Height", Video.Height_GL);
	cfg.writeInt("Graphics", "OpenGL Filter", Video.glLinearFilter);
#endif /* GENS_OPENGL */
	
	cfg.writeInt("Graphics", "Stretch", Options::stretch());
#ifdef GENS_OS_WIN32
	cfg.writeBool("Graphics", "Software Blit", Options::swRender());
#endif /* GENS_OS_WIN32 */
	cfg.writeInt("Graphics", "Sprite Limit", Sprite_Over & 1);
	cfg.writeInt("Graphics", "Frame Skip", Frame_Skip);
	
	// Sound settings
	cfg.writeBool("Sound", "State", audio->enabled());
	cfg.writeInt("Sound", "Rate", audio->soundRate());
	cfg.writeBool("Sound", "Stereo", audio->stereo());
	
	cfg.writeInt("Sound", "Z80 State", Z80_State & 1);
	cfg.writeInt("Sound", "YM2612 State", YM2612_Enable & 1);
	cfg.writeInt("Sound", "PSG State", PSG_Enable & 1);
	cfg.writeInt("Sound", "DAC State", DAC_Enable & 1);
	cfg.writeInt("Sound", "PCM State", PCM_Enable & 1);
	cfg.writeInt("Sound", "PWM State", PWM_Enable & 1);
	cfg.writeInt("Sound", "CDDA State", CDDA_Enable & 1);
	
	// Improved sound options
	cfg.writeInt("Sound", "YM2612 Improvement", YM2612_Improv & 1);
	cfg.writeInt("Sound", "DAC Improvement", DAC_Improv & 1);
	cfg.writeInt("Sound", "PSG Improvement", PSG_Improv & 1);
	
	// Country codes
	cfg.writeInt("CPU", "Country", Country);
	cfg.writeInt("CPU", "Prefered Country 1", Country_Order[0]);
	cfg.writeInt("CPU", "Prefered Country 2", Country_Order[1]);
	cfg.writeInt("CPU", "Prefered Country 3", Country_Order[2]);
	
	// CPU options
	
	cfg.writeInt("CPU", "Perfect synchro between main and sub CPU (Sega CD)", SegaCD_Accurate);
	
	cfg.writeInt("CPU", "Main SH2 Speed", MSH2_Speed);
	cfg.writeInt("CPU", "Slave SH2 Speed", SSH2_Speed);
	
	// Various settings
	cfg.writeBool("Options", "Fast Blur", draw->fastBlur());
	cfg.writeBool("Options", "FPS", draw->fpsEnabled());
	cfg.writeInt("Options", "FPS Style", draw->fpsStyle());
	cfg.writeBool("Options", "Message", draw->msgEnabled());
	cfg.writeInt("Options", "Message Style", draw->msgStyle());
	cfg.writeInt("Options", "LED", Show_LED & 1);
	cfg.writeInt("Options", "Auto Fix Checksum", Auto_Fix_CS & 1);
	cfg.writeInt("Options", "Auto Pause", Auto_Pause & 1);
	
	// SegaCD BRAM cartridge
	if (BRAM_Ex_State & 0x100)
		cfg.writeInt("Options", "RAM Cart Size", BRAM_Ex_Size);
	else
		cfg.writeInt("Options", "RAM Cart Size", -1);
	
	// Miscellaneous files
	cfg.writeString("Options", "7z Binary", Misc_Filenames._7z_Binary);
	cfg.writeString("Options", "RAR Binary", Misc_Filenames.RAR_Binary);
	
	// Controller settings
	cfg.writeInt("Input", "P1.Type", Controller_1_Type & 0x13);
	cfg.writeInt("Input", "P1B.Type", Controller_2B_Type & 0x13);
	cfg.writeInt("Input", "P1C.Type", Controller_2C_Type & 0x13);
	cfg.writeInt("Input", "P1D.Type", Controller_2D_Type & 0x13);
	cfg.writeInt("Input", "P2.Type", Controller_2_Type & 0x13);
	cfg.writeInt("Input", "P2B.Type", Controller_2B_Type & 0x13);
	cfg.writeInt("Input", "P2C.Type", Controller_2C_Type & 0x13);
	cfg.writeInt("Input", "P2D.Type", Controller_2D_Type & 0x13);
	
	char tmpKey[16];
	for (i = 0; i < 8; i++)
	{
		sprintf(tmpKey, "%s.Up", PlayerNames[i]);
		cfg.writeInt("Input", tmpKey, input->m_keyMap[i].Up, true, 4);
		sprintf(tmpKey, "%s.Down", PlayerNames[i]);
		cfg.writeInt("Input", tmpKey, input->m_keyMap[i].Down, true, 4);
		sprintf(tmpKey, "%s.Left", PlayerNames[i]);
		cfg.writeInt("Input", tmpKey, input->m_keyMap[i].Left, true, 4);
		sprintf(tmpKey, "%s.Right", PlayerNames[i]);
		cfg.writeInt("Input", tmpKey, input->m_keyMap[i].Right, true, 4);
		sprintf(tmpKey, "%s.Start", PlayerNames[i]);
		cfg.writeInt("Input", tmpKey, input->m_keyMap[i].Start, true, 4);
		sprintf(tmpKey, "%s.A", PlayerNames[i]);
		cfg.writeInt("Input", tmpKey, input->m_keyMap[i].A, true, 4);
		sprintf(tmpKey, "%s.B", PlayerNames[i]);
		cfg.writeInt("Input", tmpKey, input->m_keyMap[i].B, true, 4);
		sprintf(tmpKey, "%s.C", PlayerNames[i]);
		cfg.writeInt("Input", tmpKey, input->m_keyMap[i].C, true, 4);
		sprintf(tmpKey, "%s.Mode", PlayerNames[i]);
		cfg.writeInt("Input", tmpKey, input->m_keyMap[i].Mode, true, 4);
		sprintf(tmpKey, "%s.X", PlayerNames[i]);
		cfg.writeInt("Input", tmpKey, input->m_keyMap[i].X, true, 4);
		sprintf(tmpKey, "%s.Y", PlayerNames[i]);
		cfg.writeInt("Input", tmpKey, input->m_keyMap[i].Y, true, 4);
		sprintf(tmpKey, "%s.Z", PlayerNames[i]);
		cfg.writeInt("Input", tmpKey, input->m_keyMap[i].Z, true, 4);
	}
	
	// Save the INI file.
	cfg.save(filename);
	
	// Done.
	return 1;
}


/**
 * saveAs(): Save the current configuration using a user-selected filename.
 * @return 1 if a file was selected.
 */
int Config::saveAs(void)
{
	string filename;
	
	filename = GensUI::saveFile("Save Config As", "", ConfigFile);
	if (filename.length() == 0)
		return 0;
	
	// Filename selected for the config file.
	save(filename);
	draw->writeText("Config saved in " + filename, 2000);
	return 1;
}


/**
 * load(): Load Gens configuration.
 * @param filename Configuration filename.
 * @param Game_Active ???
 */
int Config::load(const string& filename, void* gameActive)
{
	int new_val, i;
	char buf[256];
	
#ifdef GENS_OS_WIN32
	SetCurrentDirectory(PathNames.Gens_EXE_Path);
#endif /* GENS_OS_WIN32 */
	
	INI cfg(filename);
	
	CRam_Flag = 1;
	
	// Paths
	cfg.getString("General", "ROM Path", PathNames.Gens_Path, Rom_Dir, sizeof(Rom_Dir));
	cfg.getString("General", "Save Path", PathNames.Gens_Path, State_Dir, sizeof(State_Dir));
	cfg.getString("General", "SRAM Path", PathNames.Gens_Path, SRAM_Dir, sizeof(SRAM_Dir));
	cfg.getString("General", "BRAM Path", PathNames.Gens_Path, BRAM_Dir, sizeof(BRAM_Dir));
	cfg.getString("General", "Dump Path", PathNames.Gens_Path, PathNames.Dump_WAV_Dir, sizeof(PathNames.Dump_WAV_Dir));
	cfg.getString("General", "Dump GYM Path", PathNames.Gens_Path, PathNames.Dump_GYM_Dir, sizeof(PathNames.Dump_GYM_Dir));
	cfg.getString("General", "Screen Shot Path", PathNames.Gens_Path, PathNames.Screenshot_Dir, sizeof(PathNames.Screenshot_Dir));
	cfg.getString("General", "Patch Path", PathNames.Gens_Path, Patch_Dir, sizeof(Patch_Dir));
	cfg.getString("General", "IPS Patch Path", PathNames.Gens_Path, IPS_Dir, sizeof(IPS_Dir));
	
	// Genesis BIOS
	cfg.getString("General", "Genesis BIOS", "", BIOS_Filenames.MD_TMSS, sizeof(BIOS_Filenames.MD_TMSS));
	
	// SegaCD BIOSes
	cfg.getString("General", "USA CD BIOS", "", BIOS_Filenames.SegaCD_US, sizeof(BIOS_Filenames.SegaCD_US));
	cfg.getString("General", "Europe CD BIOS", "", BIOS_Filenames.MegaCD_EU, sizeof(BIOS_Filenames.MegaCD_EU));
	cfg.getString("General", "Japan CD BIOS", "", BIOS_Filenames.MegaCD_JP, sizeof(BIOS_Filenames.MegaCD_JP));
	
	// 32X BIOSes
	cfg.getString("General", "32X 68000 BIOS", "", BIOS_Filenames._32X_MC68000, sizeof(BIOS_Filenames._32X_MC68000));
	cfg.getString("General", "32X Master SH2 BIOS", "", BIOS_Filenames._32X_MSH2, sizeof(BIOS_Filenames._32X_MSH2));
	cfg.getString("General", "32X Slave SH2 BIOS", "", BIOS_Filenames._32X_SSH2, sizeof(BIOS_Filenames._32X_SSH2));
	
	// Last 9 ROMs
	for (i = 0; i < 9; i++)
	{
		sprintf(buf, "ROM %d", i + 1);
		cfg.getString("General", buf, "", Recent_Rom[i], sizeof(Recent_Rom[i]));
	}
	
#ifdef GENS_CDROM
	// Physical CD-ROM support for SegaCD
#if defined(GENS_OS_WIN32)
	// Win32 ASPI uses a device ID number.
	cdromDeviceID = cfg.getInt("Options", "CD Drive", 0);
#elif defined(GENS_OS_UNIX)
	// Unix uses a device name.
	cfg.getString("General", "CD Drive", "/dev/cdrom", cdromDeviceName, sizeof(cdromDeviceName));
#endif /* GENS_OS_WIN32 / GENS_OS_UNIX */
	cdromSpeed = cfg.getInt("General", "CD Speed", 0);
#endif /* GENS_CDROM */
	
	Current_State = cfg.getInt("General", "State Number", 0);
	Language = cfg.getInt("General", "Language", 0);
	Window_Pos.x = cfg.getInt("General", "Window X", 0);
	Window_Pos.y = cfg.getInt("General", "Window Y", 0);
	Intro_Style = cfg.getInt("General", "Intro Style", 0);
	draw->setIntroEffectColor(cfg.getInt("General", "Free Mode Color", 7));
	Sleep_Time = cfg.getInt("General", "Allow Idle", 0) & 1;
	
	// Video adjustments
	Contrast_Level = cfg.getInt("Graphics", "Contrast", 100);
	Brightness_Level = cfg.getInt("Graphics", "Brightness", 100);
	Greyscale = cfg.getInt("Graphics", "Greyscale", 0);
	Invert_Color = cfg.getInt("Graphics", "Invert", 0);
	
	// Video settings
	Video.VSync_FS = cfg.getInt("Graphics", "Full Screen VSync", 0) & 1;
	Video.VSync_W = cfg.getInt("Graphics", "Windows VSync", 0) & 1;
	draw->setFullScreen(cfg.getBool("Graphics", "Full Screen", false));
	Video.borderColorEmulation = cfg.getBool("Graphics", "Border Color Emulation", true);
	Video.pauseTint = cfg.getBool("Graphics", "Pause Tint", true);
	
	// Renderer: Full Screen
	string renderTag = cfg.getString("Graphics", "Render Fullscreen", "");
	mapStrToInt::iterator renderMDP;
	if (renderTag.empty())
		Video.Render_FS = 1;
	else
	{
		std::transform(renderTag.begin(), renderTag.end(), renderTag.begin(), ::tolower);
		renderMDP = PluginMgr::tblRenderPlugins.find(renderTag);
		if (renderMDP == PluginMgr::tblRenderPlugins.end())
			Video.Render_FS = 1;
		else
			Video.Render_FS = (*renderMDP).second;
	}
	
	// Renderer: Windowed
	renderTag = cfg.getString("Graphics", "Render Windowed", "");
	if (renderTag.empty())
		Video.Render_W = 1;
	else
	{
		std::transform(renderTag.begin(), renderTag.end(), renderTag.begin(), ::tolower);
		renderMDP = PluginMgr::tblRenderPlugins.find(renderTag);
		if (renderMDP == PluginMgr::tblRenderPlugins.end())
			Video.Render_W = 1;
		else
			Video.Render_W = (*renderMDP).second;
	}
	
#ifndef GENS_OS_WIN32
	// TODO: Add a 555/565 override for Win32.
	bppOut = (unsigned char)(cfg.getInt("Graphics", "Bits Per Pixel", 32));
	if (bppOut != 15 && bppOut != 16 && bppOut != 32)
	{
		// Invalid bpp. Set it to 32 by default.
		bppOut = 32;
	}
	bppMD = bppOut;
#endif /* GENS_OS_WIN32 */
	
#ifdef GENS_OPENGL
	Video.OpenGL = cfg.getInt("Graphics", "Render OpenGL", 0);
	Video.Width_GL = cfg.getInt("Graphics", "OpenGL Width", 640);
	Video.Height_GL = cfg.getInt("Graphics", "OpenGL Height", 480);
	Video.glLinearFilter = cfg.getInt("Graphics", "OpenGL Filter", 0);
	
	// Set the OpenGL renderer.
	// NOTE: Don't do this while Gens is loading; otherwise, GTK+ raises an assert
	// because the window hasn't been created yet.
	if (is_gens_running())
		Options::setOpenGL(Video.OpenGL);
#endif
	
	//Set_Render(Full_Screen, -1, 1);
	
	// Recalculate the MD and 32X palettes using the new color and video mode settings.
	// NOTE: This is only done if Gens is currently running.
	// If Gens isn't running, then this is before Gens had a chance to parse
	// command line arguments, so things may change later.
	if (is_gens_running())
		Recalculate_Palettes();
	
	Options::setStretch(cfg.getInt("Graphics", "Stretch", 0));
#ifdef GENS_OS_WIN32
	Options::setSwRender(cfg.getBool("Graphics", "Software Blit", false));
#endif /* GENS_OS_WIN32 */
	Sprite_Over = cfg.getInt("Graphics", "Sprite Limit", 1);
	Frame_Skip = cfg.getInt("Graphics", "Frame Skip", -1);
	
	// Sound settings
	audio->setSoundRate(cfg.getInt("Sound", "Rate", 22050));
	audio->setStereo(cfg.getBool("Sound", "Stereo", true));
	
	if (cfg.getInt("Sound", "Z80 State", 1))
		Z80_State |= 1;
	else
		Z80_State &= ~1;
	
	// Only load the IC sound settings if sound can be initialized.
	new_val = cfg.getInt("Sound", "State", 1);
	if (new_val == audio->enabled() ||
	    (new_val != audio->enabled() && Options::setSoundEnable(true)))
	{
		YM2612_Enable = cfg.getInt("Sound", "YM2612 State", 1);
		PSG_Enable = cfg.getInt("Sound", "PSG State", 1);
		DAC_Enable = cfg.getInt("Sound", "DAC State", 1);
		PCM_Enable = cfg.getInt("Sound", "PCM State", 1);
		PWM_Enable = cfg.getInt("Sound", "PWM State", 1);
		CDDA_Enable = cfg.getInt("Sound", "CDDA State", 1);
		
		// Improved sound options
		YM2612_Improv = cfg.getInt("Sound", "YM2612 Improvement", 0);
		DAC_Improv = cfg.getInt("Sound", "DAC Improvement", 0);
		PSG_Improv = cfg.getInt("Sound", "PSG Improvement", 0);
	}
	
	// Country codes
	Country = cfg.getInt("CPU", "Country", -1);
	Country_Order[0] = cfg.getInt("CPU", "Prefered Country 1", 0);
	Country_Order[1] = cfg.getInt("CPU", "Prefered Country 2", 1);
	Country_Order[2] = cfg.getInt("CPU", "Prefered Country 3", 2);
	Check_Country_Order();
	
	// CPU options
	
	SegaCD_Accurate = cfg.getInt("CPU", "Perfect synchro between main and sub CPU (Sega CD)", 0);
	MSH2_Speed = cfg.getInt("CPU", "Main SH2 Speed", 100);
	SSH2_Speed = cfg.getInt("CPU", "Slave SH2 Speed", 100);
	
	// Make sure the SH2 speeds aren't below 0.
	if (MSH2_Speed < 0)
		MSH2_Speed = 0;
	if (SSH2_Speed < 0)
		SSH2_Speed = 0;
	
	// Various settings
	draw->setFastBlur(cfg.getBool("Options", "Fast Blur", false));
	draw->setFPSEnabled(cfg.getBool("Options", "FPS", false));
	draw->setFPSStyle(cfg.getInt("Options", "FPS Style", 0));
	draw->setMsgEnabled(cfg.getBool("Options", "Message", true));
	draw->setMsgStyle(cfg.getInt("Options", "Message Style", 0));
	Show_LED = cfg.getInt("Options", "LED", 1);
	Auto_Fix_CS = cfg.getInt("Options", "Auto Fix Checksum", 0);
	Auto_Pause = cfg.getInt("Options", "Auto Pause", 0);
	
	// SegaCD BRAM cartridge size
	BRAM_Ex_Size = cfg.getInt("Options", "RAM Cart Size", 3);
	if (BRAM_Ex_Size == -1)
	{
		BRAM_Ex_State &= 1;
		BRAM_Ex_Size = 0;
	}
	else if (BRAM_Ex_Size < -1 || BRAM_Ex_Size > 3)
		BRAM_Ex_Size = 3;
	else
		BRAM_Ex_State |= 0x100;
	
	// Miscellaneous files
#if defined(__WIN32__)
	cfg.getString("Options", "7z Binary", "C:\\Program Files\\7-Zip\\7z.exe",
		      Misc_Filenames._7z_Binary, sizeof(Misc_Filenames._7z_Binary));
	cfg.getString("Options", "RAR Binary", "C:\\Program Files\\WinRAR\\Rar.exe",
		      Misc_Filenames.RAR_Binary, sizeof(Misc_Filenames.RAR_Binary));
#else /* !defined(__WIN32__) */
	cfg.getString("Options", "7z Binary", "/usr/bin/7z",
		      Misc_Filenames._7z_Binary, sizeof(Misc_Filenames._7z_Binary));
	cfg.getString("Options", "RAR Binary", "/usr/bin/rar",
		      Misc_Filenames.RAR_Binary, sizeof(Misc_Filenames.RAR_Binary));
#endif	
	
	// Controller settings
	Controller_1_Type = cfg.getInt("Input", "P1.Type", 1);
	Controller_1B_Type = cfg.getInt("Input", "P1B.Type", 1);
	Controller_1C_Type = cfg.getInt("Input", "P1C.Type", 1);
	Controller_1D_Type = cfg.getInt("Input", "P1D.Type", 1);
	Controller_2_Type = cfg.getInt("Input", "P2.Type", 1);
	Controller_2B_Type = cfg.getInt("Input", "P2B.Type", 1);
	Controller_2C_Type = cfg.getInt("Input", "P2C.Type", 1);
	Controller_2D_Type = cfg.getInt("Input", "P2D.Type", 1);
	
	for (i = 0; i < 8; i++)
	{
		sprintf(buf, "%s.Up", PlayerNames[i]);
		input->m_keyMap[i].Up = cfg.getInt("Input", buf, keyDefault[i].Up);
		sprintf(buf, "%s.Down", PlayerNames[i]);
		input->m_keyMap[i].Down = cfg.getInt("Input", buf, keyDefault[i].Down);
		sprintf(buf, "%s.Left", PlayerNames[i]);
		input->m_keyMap[i].Left = cfg.getInt("Input", buf, keyDefault[i].Left);
		sprintf(buf, "%s.Right", PlayerNames[i]);
		input->m_keyMap[i].Right = cfg.getInt("Input", buf, keyDefault[i].Right);
		sprintf(buf, "%s.Start", PlayerNames[i]);
		input->m_keyMap[i].Start = cfg.getInt("Input", buf, keyDefault[i].Start);
		sprintf(buf, "%s.A", PlayerNames[i]);
		input->m_keyMap[i].A = cfg.getInt("Input", buf, keyDefault[i].A);
		sprintf(buf, "%s.B", PlayerNames[i]);
		input->m_keyMap[i].B = cfg.getInt("Input", buf, keyDefault[i].B);
		sprintf(buf, "%s.C", PlayerNames[i]);
		input->m_keyMap[i].C = cfg.getInt("Input", buf, keyDefault[i].C);
		sprintf(buf, "%s.Mode", PlayerNames[i]);
		input->m_keyMap[i].Mode = cfg.getInt("Input", buf, keyDefault[i].Mode);
		sprintf(buf, "%s.X", PlayerNames[i]);
		input->m_keyMap[i].X = cfg.getInt("Input", buf, keyDefault[i].X);
		sprintf(buf, "%s.Y", PlayerNames[i]);
		input->m_keyMap[i].Y = cfg.getInt("Input", buf, keyDefault[i].Y);
		sprintf(buf, "%s.Z", PlayerNames[i]);
		input->m_keyMap[i].Z = cfg.getInt("Input", buf, keyDefault[i].Z);
	}
	
	// Create the TeamPlayer I/O table.
	Make_IO_Table();
	
	// Done.
	return 1;
}


/**
 * Load_As_Config(): Load a user-selected configuration file.
 * @param Game_Active ???
 * @return 1 if a file was selected.
 */
int Config::loadAs(void* gameActive)
{
	string filename;
	
	filename = GensUI::openFile("Load Config", "", ConfigFile);
	if (filename.length() == 0)
		return 0;
	
	// Filename selected for the config file.
	load(filename, gameActive);
	draw->writeText("Config loaded from " + filename, 2000);
	return 1;
}
