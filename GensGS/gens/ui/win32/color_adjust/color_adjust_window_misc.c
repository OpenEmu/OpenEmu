/***************************************************************************
 * Gens: (Win32) Color Adjustment Window - Miscellaneous Functions.        *
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

#include <stdio.h>

#include "color_adjust_window.h"
#include "color_adjust_window_callbacks.h"
#include "color_adjust_window_misc.h"
#include "gens/gens_window.hpp"

#include "emulator/g_palette.h"
#include "gens_core/vdp/vdp_io.h"
#include "emulator/g_main.hpp"
#include "emulator/g_update.hpp"

#include <windows.h>
#include <commctrl.h>

#ifdef GENS_DEBUGGER
#include "debugger/debugger.hpp"
#endif /* GENS_DEBUGGER */


/**
 * Open_Color_Adjust(): Opens the Color Adjustment window.
 */
void Open_Color_Adjust(void)
{
	HWND ca = create_color_adjust_window();
	if (!ca)
	{
		// Either an error occurred while creating the Color Adjustment window,
		// or the Color Adjustment window is already created.
		return;
	}
	
	// TODO: Make the window modal.
	//gtk_window_set_transient_for(GTK_WINDOW(ca), GTK_WINDOW(gens_window));
	
	// Load settings.
	char buf[16];
	
	// Contrast
	SendMessage(ca_trkContrast, TBM_SETPOS, TRUE, Contrast_Level - 100);
	sprintf(buf, "%d", Contrast_Level - 100);
	SetWindowText(ca_lblContrastVal, buf);
	
	// Brightness
	SendMessage(ca_trkBrightness, TBM_SETPOS, TRUE, Brightness_Level - 100);
	sprintf(buf, "%d", Brightness_Level - 100);
	SetWindowText(ca_lblBrightnessVal, buf);
	
	// Show the Color Adjustment window.
	ShowWindow(ca, 1);
}


/**
 * CA_Save(): Save the settings.
 */
void CA_Save(void)
{
	// Save settings.
	Contrast_Level = SendMessage(ca_trkContrast, TBM_GETPOS, 0, 0) + 100;
	Brightness_Level = SendMessage(ca_trkBrightness, TBM_GETPOS, 0, 0) + 100;
	
	// Recalculate palettes.
	Recalculate_Palettes();
	if (Genesis_Started || _32X_Started || SegaCD_Started)
	{
		// Emulation is running. Update the CRAM.
		CRam_Flag = 1;
		
		if (!Paused)
		{
			// TODO: Just update CRAM. Don't update the frame.
			Update_Emulation_One();
#ifdef GENS_DEBUGGER
			if (Debug)
				Update_Debug_Screen();
#endif /* GENS_DEBUGGER */
		}
	}
}
