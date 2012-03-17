/***************************************************************************
 * Gens: (GTK+) Main Window - Callback Functions.                          *
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

#include "emulator/g_main.hpp"
#include "gens_window.hpp"
#include "gens_window_callbacks.hpp"
#include "gens_window_sync.hpp"

#include "util/file/config_file.hpp"

#include "ui/gens_ui.hpp"
#include "gtk-misc.h"

#include "util/sound/gym.hpp"
#include "util/file/rom.hpp"
#include "gens_core/vdp/vdp_io.h"
#include "util/file/save.hpp"
#include "gens_core/cpu/z80/z80.h"
#include "util/gfx/imageutil.hpp"

#include "macros/file_m.h"

// Sega CD
#include "emulator/g_mcd.hpp"

// 32X
#include "gens_core/cpu/sh2/sh2.h"

// CD-ROM drive access
#ifdef GENS_CDROM
#include "segacd/cd_aspi.hpp"
#endif /* GENS_CDROM */

// Debugger
//#ifdef GENS_DEBUGGER
//#include "debugger/debugger.hpp"
//#endif /* GENS_DEBUGGER */

#include <sys/types.h>
#include <unistd.h>

// C++ includes
#include <string>
using std::string;


/**
 * Window is closed.
 */
gboolean on_gens_window_close(GtkWidget *widget, GdkEvent *event, gpointer user_data)
{
	GENS_UNUSED_PARAMETER(widget);
	GENS_UNUSED_PARAMETER(event);
	GENS_UNUSED_PARAMETER(user_data);
	
	close_gens();
	
	// TRUE tells GTK+ not to close the window. This is needed
	// in order to prevent an X11 error from occurring due to
	// the embedded SDL window.
	return TRUE;
}


/** Drag & Drop callbacks **/


void gens_window_drag_data_received(GtkWidget *widget, GdkDragContext *context, gint x, gint y,
				    GtkSelectionData *selection_data, guint target_type, guint time,
				    gpointer data)
{
	gboolean dnd_success = FALSE;
	gboolean delete_selection_data = FALSE;
	
	if ((selection_data != NULL) && (selection_data->length >= 0))
	{
		string filename;
		dnd_success = TRUE;
		
		filename = string((gchar*)(selection_data->data));
		
		if (filename.length() >= 8 && filename.substr(0, 8) == "file:///")
		{
			// "file:///" prefix. Remove the prefix.
			filename = filename.substr(7);
		}
		else if (filename.length() >= 7 && filename.substr(0, 7) == "file://")
		{
			// "file://" prefix. Remove the prefix.
			filename = filename.substr(6);
		}
		else if (filename.length() >= 6 && filename.substr(0, 6) == "file:/")
		{
			// "file:/" prefix. Remove the prefix.
			filename = filename.substr(5);
		}
		else if (filename.length() >= 10 && filename.substr(0, 9) == "desktop:/")
		{
			// "desktop:/" prefix. Remove the prefix and prepend the user's desktop directory.
			filename = string(getenv("HOME")) + "/Desktop/" + filename.substr(9);
		}
		
		// Unescape the URI.
		char* unescaped = g_uri_unescape_string(filename.c_str(), NULL);
		filename = string(unescaped);
		g_free(unescaped);
		
		// Check that the file actually exists.
		if (fileExists(filename.c_str()))
		{
			// File exists. Open it as a ROM image.
			ROM::openROM(filename.c_str());
		}
		else
		{
			// File does not exist.
			dnd_success = FALSE;
		}
	}
	
	gtk_drag_finish(context, dnd_success, delete_selection_data, time);
}


gboolean gens_window_drag_drop(GtkWidget *widget, GdkDragContext *context,
			       gint x, gint y, guint time, gpointer user_data)
{
	if (context->targets)
	{
		GdkAtom target_type = GDK_POINTER_TO_ATOM(g_list_nth_data(context->targets, 0));
		gtk_drag_get_data(widget, context, target_type, time);
		return TRUE;
	}
	return FALSE;
}


/** Focus callbacks **/


/**
 * gens_window_focus_in(): Gens window has received focus.
 * @param widget GTK+ widget.
 * @param event GDK event.
 * @param user_data User data.
 * @return TRUE to stop other handlers from being invoked; FALSE to allow the event to propagate.
 */
gboolean gens_window_focus_in(GtkWidget *widget, GdkEventFocus *event, gpointer user_data)
{
	Active = 1;
	return FALSE;
}


/**
 * gens_window_focus_in(): Gens window has lost focus.
 * @param widget GTK+ widget.
 * @param event GDK event.
 * @param user_data User data.
 * @return TRUE to stop other handlers from being invoked; FALSE to allow the event to propagate.
 */
gboolean gens_window_focus_out(GtkWidget *widget, GdkEventFocus *event, gpointer user_data)
{
	if (Auto_Pause && Active)
	{
		Active = 0;
		audio->clearSoundBuffer();
	}
	return FALSE;
}
