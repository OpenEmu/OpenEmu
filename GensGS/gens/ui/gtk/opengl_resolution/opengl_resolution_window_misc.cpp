/***************************************************************************
 * Gens: (GTK+) OpenGL Resolution Window - Miscellaneous Functions.        *
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

#include "opengl_resolution_window.h"
#include "opengl_resolution_window_callbacks.h"
#include "opengl_resolution_window_misc.hpp"
#include "gens/gens_window.hpp"

#include <gtk/gtk.h>
#include "gtk-misc.h"

#include "emulator/g_main.hpp"
#include "emulator/options.hpp"


/**
 * Open_OpenGL_Resolution(): Opens the OpenGL Resolution window.
 */
void Open_OpenGL_Resolution(void)
{
	GtkWidget *GLRes;
	GtkWidget *spinbutton_width, *spinbutton_height;
	
	GLRes = create_opengl_resolution_window();
	if (!GLRes)
	{
		// Either an error occurred while creating the OpenGL Resolution window,
		// or the OpenGL Resolution window is already created.
		return;
	}
	gtk_window_set_transient_for(GTK_WINDOW(GLRes), GTK_WINDOW(gens_window));
	
	// Load settings.
	spinbutton_width = lookup_widget(GLRes, "spinbutton_width");
	gtk_spin_button_set_value(GTK_SPIN_BUTTON(spinbutton_width), Video.Width_GL);
	spinbutton_height = lookup_widget(GLRes, "spinbutton_height");
	gtk_spin_button_set_value(GTK_SPIN_BUTTON(spinbutton_height), Video.Height_GL);
	
	// Show the OpenGL Resolution window.
	gtk_widget_show_all(GLRes);
}


/**
 * GLRes_Save(): Save the settings.
 */
void GLRes_Save(void)
{
	GtkWidget *spinbutton_width, *spinbutton_height;
	
	// Save settings.
	// TODO: If this is a predefined resolution,
	// uncheck "Custom" and check the predefined resolution.
	spinbutton_width = lookup_widget(opengl_resolution_window, "spinbutton_width");
	spinbutton_height = lookup_widget(opengl_resolution_window, "spinbutton_height");
	
	Options::setOpenGL_Resolution
	(
		gtk_spin_button_get_value(GTK_SPIN_BUTTON(spinbutton_width)),
		gtk_spin_button_get_value(GTK_SPIN_BUTTON(spinbutton_height))
	);
}
