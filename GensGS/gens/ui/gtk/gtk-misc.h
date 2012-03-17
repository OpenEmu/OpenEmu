/***************************************************************************
 * Gens: (GTK+) Miscellaneous Functions.                                   *
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


#ifndef GTK_MISC_H
#define GTK_MISC_H

#ifdef __cplusplus
extern "C" {
#endif

#include <gtk/gtk.h>


GtkWidget *lookup_widget(GtkWidget *widget, const gchar *widget_name);
void add_pixmap_directory(const gchar * directory);
GdkPixbuf *create_pixbuf(const gchar *filename);
GtkWidget *create_pixmap(const gchar *filename);


// Macros from Glade used to store GtkWidget pointers.
#define GLADE_HOOKUP_OBJECT(component, widget, name)			\
{									\
	g_object_set_data_full(G_OBJECT(component), name,		\
			       g_object_ref(widget),			\
			       (GDestroyNotify)g_object_unref);		\
}


#define GLADE_HOOKUP_OBJECT_NO_REF(component, widget, name)		\
{									\
	g_object_set_data(G_OBJECT(component), name, widget);		\
};


// Macro to add a "clicked" event callback to a button.
#define AddButtonCallback_Clicked(ButtonWidget, CallbackFunction)				\
{												\
	g_signal_connect((gpointer)ButtonWidget, "clicked",					\
			 G_CALLBACK(CallbackFunction), NULL);					\
}


// Create a GtkWindow.
#define CREATE_GTK_WINDOW(Widget, Name, Title, IconWidget, IconFilename)			\
{												\
	Widget = gtk_window_new(GTK_WINDOW_TOPLEVEL);						\
	gtk_widget_set_name(Widget, Name);							\
	gtk_container_set_border_width(GTK_CONTAINER(Widget), 5);				\
	gtk_window_set_title(GTK_WINDOW(Widget), Title);					\
	gtk_window_set_position(GTK_WINDOW(Widget), GTK_WIN_POS_CENTER);			\
	GLADE_HOOKUP_OBJECT_NO_REF(Widget, Widget, Name);					\
												\
	IconWidget = create_pixbuf(IconFilename);						\
	if (IconWidget)										\
	{											\
		gtk_window_set_icon(GTK_WINDOW(Widget), IconWidget);				\
		gdk_pixbuf_unref(IconWidget);							\
	}											\
}


#ifdef __cplusplus
}
#endif

#endif
