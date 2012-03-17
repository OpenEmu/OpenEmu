/***************************************************************************
 * Gens: (GTK+) Color Adjustment Window - Miscellaneous Functions.         *
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

#include "color_adjust_window.h"
#include "color_adjust_window_callbacks.h"
#include "color_adjust_window_misc.h"
#include "gens/gens_window.hpp"

#include <gtk/gtk.h>
#include "gtk-misc.h"

#include "emulator/g_palette.h"
#include "gens_core/vdp/vdp_io.h"
#include "emulator/g_main.hpp"
#include "emulator/g_update.hpp"

#ifdef GENS_DEBUGGER
#include "debugger/debugger.hpp"
#endif /* GENS_DEBUGGER */


/**
 * Open_Color_Adjust(): Opens the Color Adjustment window.
 */
void Open_Color_Adjust(void)
{
	GtkWidget *ca;
	GtkWidget *hscale_slider_contrast, *hscale_slider_brightness;
	GtkWidget *check_options_grayscale, *check_options_inverted;
	
	ca = create_color_adjust_window();
	if (!ca)
	{
		// Either an error occurred while creating the Color Adjustment window,
		// or the Color Adjustment window is already created.
		return;
	}
	gtk_window_set_transient_for(GTK_WINDOW(ca), GTK_WINDOW(gens_window));
	
	// Load settings.
	hscale_slider_contrast = lookup_widget(ca, "hscale_slider_contrast");
	gtk_range_set_value(GTK_RANGE(hscale_slider_contrast), Contrast_Level - 100);
	hscale_slider_brightness = lookup_widget(ca, "hscale_slider_brightness");
	gtk_range_set_value(GTK_RANGE(hscale_slider_brightness), Brightness_Level - 100);
	check_options_grayscale = lookup_widget(ca, "check_options_grayscale");
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(check_options_grayscale), Greyscale);
	check_options_inverted = lookup_widget(ca, "check_options_inverted");
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(check_options_inverted), Invert_Color);
	
	// Show the Color Adjustment window.
	gtk_widget_show_all(ca);
}


/**
 * CA_Save(): Save the settings.
 */
void CA_Save(void)
{
	GtkWidget *hscale_slider_contrast, *hscale_slider_brightness;
	GtkWidget *check_options_grayscale, *check_options_inverted;
	
	// Save settings.
	hscale_slider_contrast = lookup_widget(color_adjust_window, "hscale_slider_contrast");
	Contrast_Level = gtk_range_get_value(GTK_RANGE(hscale_slider_contrast)) + 100;
	hscale_slider_brightness = lookup_widget(color_adjust_window, "hscale_slider_brightness");
	Brightness_Level = gtk_range_get_value(GTK_RANGE(hscale_slider_brightness)) + 100;
	check_options_grayscale = lookup_widget(color_adjust_window, "check_options_grayscale");
	Greyscale = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(check_options_grayscale));
	check_options_inverted = lookup_widget(color_adjust_window, "check_options_inverted");
	Invert_Color = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(check_options_inverted));
	
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
