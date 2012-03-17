/***************************************************************************
 * Gens: (Win32) Controller Configuration Window - Miscellaneous Functions.*
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

#include <string.h>

#include "controller_config_window.hpp"
#include "controller_config_window_callbacks.hpp"
#include "controller_config_window_misc.hpp"
#include "gens/gens_window.hpp"

#include "emulator/g_main.hpp"
#include "gens_core/io/io.h"
#include "gens_core/io/io_teamplayer.h"
#include "ui/gens_ui.hpp"

#include <windows.h>
#include <windowsx.h>

// Win32 common controls
#include <commctrl.h>


/**
 * Open_Controller_Config(): Opens the Controller Configuration window.
 */
void Open_Controller_Config(void)
{
	HWND cc = create_controller_config_window();
	if (!cc)
	{
		// Either an error occurred while creating the Controller Configuration window,
		// or the Controller Configuration window is already created.
		return;
	}
	
	// TODO: Make the window modal.
	//gtk_window_set_transient_for(GTK_WINDOW(cc), GTK_WINDOW(gens_window));
	
	// Copy the current controller key configuration.
	memcpy(keyConfig, input->m_keyMap, sizeof(keyConfig));
	
	// Set the Teamplayer options.
	Button_SetCheck(cc_chkTeamPlayer[0], (Controller_1_Type & 0x10));
	Button_SetCheck(cc_chkTeamPlayer[1], (Controller_2_Type & 0x10));
	
	// Set 3/6 button options.
	ComboBox_SetCurSel(cc_cboControllerType[0], ((Controller_1_Type & 0x01) ? 1 : 0));
	ComboBox_SetCurSel(cc_cboControllerType[2], ((Controller_1B_Type & 0x01) ? 1 : 0));
	ComboBox_SetCurSel(cc_cboControllerType[3], ((Controller_1C_Type & 0x01) ? 1 : 0));
	ComboBox_SetCurSel(cc_cboControllerType[4], ((Controller_1D_Type & 0x01) ? 1 : 0));
	
	ComboBox_SetCurSel(cc_cboControllerType[1], ((Controller_2_Type & 0x01) ? 1 : 0));
	ComboBox_SetCurSel(cc_cboControllerType[5], ((Controller_2B_Type & 0x01) ? 1 : 0));
	ComboBox_SetCurSel(cc_cboControllerType[6], ((Controller_2C_Type & 0x01) ? 1 : 0));
	ComboBox_SetCurSel(cc_cboControllerType[7], ((Controller_2D_Type & 0x01) ? 1 : 0));
	
	// Initialize the Teamplayer state.
	adjustTeamplayer(0);
	adjustTeamplayer(1);
	
	// Show the Controller Configuration window.
	ShowWindow(cc, 1);
}


/**
 * Reconfigure_Input(): Reconfigure an input device.
 * @param player Player number.
 * @param padtype Pad type. (0 == 3-button; 1 == 6-button.)
 * @return 1 on success.
 */
int Reconfigure_Input(int player, int padtype)
{
	// TODO: Somehow condense this code.
	
	if (player < 0 || player >= 8 || padtype < 0 || padtype > 1)
		return 0;
	
	// Prevent audio stuttering.
	audio->clearSoundBuffer();
	
	SetWindowText(cc_lblSettingKeys, "INPUT KEY FOR UP");
	keyConfig[player].Up = input->getKey();
	GensUI::sleep(250, true);
	
	SetWindowText(cc_lblSettingKeys, "INPUT KEY FOR DOWN");
	keyConfig[player].Down = input->getKey();
	GensUI::sleep(250, true);
	
	SetWindowText(cc_lblSettingKeys, "INPUT KEY FOR LEFT");
	keyConfig[player].Left = input->getKey();
	GensUI::sleep(250, true);
	
	SetWindowText(cc_lblSettingKeys, "INPUT KEY FOR RIGHT");
	keyConfig[player].Right = input->getKey();
	GensUI::sleep(250, true);
	
	SetWindowText(cc_lblSettingKeys, "INPUT KEY FOR START");
	keyConfig[player].Start = input->getKey();
	GensUI::sleep(250, true);
	
	SetWindowText(cc_lblSettingKeys, "INPUT KEY FOR A");
	keyConfig[player].A = input->getKey();
	GensUI::sleep(250, true);
	
	SetWindowText(cc_lblSettingKeys, "INPUT KEY FOR B");
	keyConfig[player].B = input->getKey();
	GensUI::sleep(250, true);
	
	SetWindowText(cc_lblSettingKeys, "INPUT KEY FOR C");
	keyConfig[player].C = input->getKey();
	GensUI::sleep(250, true);
	
	if (padtype & 0x01)
	{
		// 6-button control pad. Get additional keys.
		SetWindowText(cc_lblSettingKeys, "INPUT KEY FOR MODE");
		keyConfig[player].Mode = input->getKey();
		GensUI::sleep(250, true);
		
		SetWindowText(cc_lblSettingKeys, "INPUT KEY FOR X");
		keyConfig[player].X = input->getKey();
		GensUI::sleep(250, true);
		
		SetWindowText(cc_lblSettingKeys, "INPUT KEY FOR Y");
		keyConfig[player].Y = input->getKey();
		GensUI::sleep(250, true);
		
		SetWindowText(cc_lblSettingKeys, "INPUT KEY FOR Z");
		keyConfig[player].Z = input->getKey();
		GensUI::sleep(250, true);
	}
	
	// Configuration successful.
	SetWindowText(cc_lblSettingKeys,
			"CONFIGURATION SUCCESSFUL.\n"
			"PRESS ANY KEY TO CONTINUE...");
	input->getKey();
	GensUI::sleep(500, true);
	SetWindowText(cc_lblSettingKeys, "");
	
	// Remove all WM_KEYDOWN messages from the window message queue.
	MSG msg;
	while (PeekMessage(&msg, controller_config_window, WM_KEYDOWN, WM_KEYDOWN, PM_REMOVE)) { }
	
	return 1;
}


/**
 * Controller_Config_Save(): Copy the new controller config to the main Gens config.
 */
void Controller_Config_Save(void)
{
	// Copy the new controller key configuration.
	memcpy(input->m_keyMap, keyConfig, sizeof(input->m_keyMap));

	// Set the controller types.
	// (Controller_1_Type & 0x10) == Teamplayer enabled
	// (Controller_1_Type & 0x01) == 6-button
	// TODO: Clean up controller type handling.
	
	// Set the Teamplayer options.
	if (Button_GetCheck(cc_chkTeamPlayer[0]) == BST_CHECKED)
		Controller_1_Type |= 0x10;
	else
		Controller_1_Type &= ~0x10;
	
	if (Button_GetCheck(cc_chkTeamPlayer[1]) == BST_CHECKED)
		Controller_2_Type |= 0x10;
	else
		Controller_2_Type &= ~0x10;
	
	// Set 3/6 button options.
	if (ComboBox_GetCurSel(cc_cboControllerType[0]) == 1)
		Controller_1_Type |= 0x01;
	else
		Controller_1_Type &= ~0x01;
	
	if (ComboBox_GetCurSel(cc_cboControllerType[2]) == 1)
		Controller_1B_Type |= 0x01;
	else
		Controller_1B_Type &= ~0x01;
	
	if (ComboBox_GetCurSel(cc_cboControllerType[3]) == 1)
		Controller_1C_Type |= 0x01;
	else
		Controller_1C_Type &= ~0x01;
	
	if (ComboBox_GetCurSel(cc_cboControllerType[4]) == 1)
		Controller_1D_Type |= 0x01;
	else
		Controller_1D_Type &= ~0x01;
	
	if (ComboBox_GetCurSel(cc_cboControllerType[1]) == 1)
		Controller_2_Type |= 0x01;
	else
		Controller_2_Type &= ~0x01;
	
	if (ComboBox_GetCurSel(cc_cboControllerType[5]) == 1)
		Controller_2B_Type |= 0x01;
	else
		Controller_2B_Type &= ~0x01;
	
	if (ComboBox_GetCurSel(cc_cboControllerType[6]) == 1)
		Controller_2C_Type |= 0x01;
	else
		Controller_2C_Type &= ~0x01;
	
	if (ComboBox_GetCurSel(cc_cboControllerType[7]) == 1)
		Controller_2D_Type |= 0x01;
	else
		Controller_2D_Type &= ~0x01;
	
	// Rebuild the I/O table for teamplayer.
	Make_IO_Table();
}
