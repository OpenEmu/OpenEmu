/***************************************************************************
 * Gens: (GTK+) OpenGL Resolution Window.                                  *
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

#include "opengl_resolution_window.h"
#include "opengl_resolution_window_callbacks.h"
#include "gens/gens_window.hpp"

#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <string.h>
#include <stdio.h>

#include <gtk/gtk.h>

// GENS GTK+ miscellaneous functions
#include "gtk-misc.h"


// Macro to add a spinbutton with a label.
#define CREATE_BOX_SPINBUTTON(GroupName, LabelWidget, Caption,				\
			      SpinbuttonWidget, Container)				\
{											\
	LabelWidget = gtk_label_new_with_mnemonic(Caption);				\
	gtk_widget_set_name(LabelWidget, "label_" GroupName);				\
	gtk_misc_set_alignment(GTK_MISC(LabelWidget), 0, 0.5);				\
	gtk_widget_show(LabelWidget);							\
	gtk_box_pack_start(GTK_BOX(Container), LabelWidget, FALSE, FALSE, 0);		\
	GLADE_HOOKUP_OBJECT(opengl_resolution_window, LabelWidget,			\
			    "label_" GroupName);					\
											\
	SpinbuttonWidget = gtk_spin_button_new(GTK_ADJUSTMENT(				\
				gtk_adjustment_new(1, 0, 9999, 1, 10, 0)), 1, 0);	\
	gtk_widget_set_name(SpinbuttonWidget, "spinbutton_" GroupName);			\
	gtk_widget_show(SpinbuttonWidget);						\
	gtk_box_pack_start(GTK_BOX(Container), SpinbuttonWidget, FALSE, FALSE, 0);	\
	gtk_label_set_mnemonic_widget(GTK_LABEL(LabelWidget), SpinbuttonWidget);	\
	GLADE_HOOKUP_OBJECT(opengl_resolution_window, SpinbuttonWidget,			\
			    "spinbutton_" GroupName);					\
}


GtkWidget *opengl_resolution_window = NULL;

GtkAccelGroup *accel_group;


/**
 * create_opengl_resolution_window(): Create the OpenGL Resolution Window.
 * @return OpenGL Resolution Window.
 */
GtkWidget* create_opengl_resolution_window(void)
{
	GdkPixbuf *opengl_resolution_window_icon_pixbuf;
	GtkWidget *vbox_GLRes;
	GtkWidget *hbox_resolution;
	GtkWidget *label_width, *spinbutton_width;
	GtkWidget *separator_resolution;
	GtkWidget *label_height, *spinbutton_height;
	GtkWidget *hbutton_box_bottomRow;
	GtkWidget *button_GLRes_Cancel, *button_GLRes_Apply, *button_GLRes_Save;
	
	if (opengl_resolution_window)
	{
		// OpenGL Resolution window is already created. Set focus.
		gtk_widget_grab_focus(opengl_resolution_window);
		return NULL;
	}
	
	accel_group = gtk_accel_group_new();
	
	// Create the OpenGL Resolution window.
	CREATE_GTK_WINDOW(opengl_resolution_window,
			  "opengl_resolution_window",
			  "OpenGL Resolution",
			  opengl_resolution_window_icon_pixbuf, "Gens2.ico");
	
	// Disable resizing.
	gtk_window_set_resizable(GTK_WINDOW(opengl_resolution_window), FALSE);
	
	// Callbacks for if the window is closed.
	g_signal_connect((gpointer)opengl_resolution_window, "delete_event",
			 G_CALLBACK(on_opengl_resolution_window_close), NULL);
	g_signal_connect((gpointer)opengl_resolution_window, "destroy_event",
			 G_CALLBACK(on_opengl_resolution_window_close), NULL);
	
	// Create the main VBox.
	vbox_GLRes = gtk_vbox_new(FALSE, 5);
	gtk_widget_set_name(vbox_GLRes, "vbox_GLRes");
	gtk_widget_show(vbox_GLRes);
	gtk_container_add(GTK_CONTAINER(opengl_resolution_window), vbox_GLRes);
	GLADE_HOOKUP_OBJECT(opengl_resolution_window, vbox_GLRes, "vbox_GLRes");
	
	// Create the HBox for the OpenGL resolution widgets.
	hbox_resolution = gtk_hbox_new(FALSE, 5);
	gtk_widget_set_name(hbox_resolution, "hbox_resolution");
	gtk_widget_show(hbox_resolution);
	gtk_box_pack_start(GTK_BOX(vbox_GLRes), hbox_resolution, TRUE, TRUE, 0);
	GLADE_HOOKUP_OBJECT(opengl_resolution_window, hbox_resolution, "hbox_resolution");
	
	// Width
	CREATE_BOX_SPINBUTTON("width", label_width, "_Width",
			      spinbutton_width, hbox_resolution);
	
	// Separator
	separator_resolution = gtk_vseparator_new();
	gtk_widget_set_name(separator_resolution, "separator_resolution");
	gtk_widget_show(separator_resolution);
	gtk_box_pack_start(GTK_BOX(hbox_resolution), separator_resolution, TRUE, TRUE, 0);
	
	// Height
	CREATE_BOX_SPINBUTTON("height", label_height, "_Height",
			      spinbutton_height, hbox_resolution);
	
	// Create an HButton Box for the buttons on the bottom.
	hbutton_box_bottomRow = gtk_hbutton_box_new();
	gtk_widget_set_name(hbutton_box_bottomRow, "hbutton_box_bottomRow");
	gtk_button_box_set_layout(GTK_BUTTON_BOX(hbutton_box_bottomRow), GTK_BUTTONBOX_END);
	gtk_widget_show(hbutton_box_bottomRow);
	gtk_box_pack_start(GTK_BOX(vbox_GLRes), hbutton_box_bottomRow, FALSE, FALSE, 0);
	GLADE_HOOKUP_OBJECT(opengl_resolution_window, hbutton_box_bottomRow, "hbutton_box_bottomRow");
	
	// Cancel
	button_GLRes_Cancel = gtk_button_new_from_stock(GTK_STOCK_CANCEL);
	gtk_widget_set_name(button_GLRes_Cancel, "button_GLRes_Cancel");
	gtk_widget_show(button_GLRes_Cancel);
	gtk_box_pack_start(GTK_BOX(hbutton_box_bottomRow), button_GLRes_Cancel, FALSE, FALSE, 0);
	gtk_widget_add_accelerator(button_GLRes_Cancel, "activate", accel_group,
				   GDK_Escape, (GdkModifierType)(0), (GtkAccelFlags)(0));
	AddButtonCallback_Clicked(button_GLRes_Cancel, on_button_GLRes_Cancel_clicked);
	GLADE_HOOKUP_OBJECT(opengl_resolution_window, button_GLRes_Cancel, "button_GLRes_Cancel");
	
	// Apply
	button_GLRes_Apply = gtk_button_new_from_stock(GTK_STOCK_APPLY);
	gtk_widget_set_name(button_GLRes_Apply, "button_GLRes_Apply");
	gtk_widget_show(button_GLRes_Apply);
	gtk_box_pack_start(GTK_BOX(hbutton_box_bottomRow), button_GLRes_Apply, FALSE, FALSE, 0);
	AddButtonCallback_Clicked(button_GLRes_Apply, on_button_GLRes_Apply_clicked);
	GLADE_HOOKUP_OBJECT(opengl_resolution_window, button_GLRes_Apply, "button_GLRes_Apply");
	
	// Save
	button_GLRes_Save = gtk_button_new_from_stock(GTK_STOCK_SAVE);
	gtk_widget_set_name(button_GLRes_Save, "button_GLRes_Save");
	gtk_widget_show(button_GLRes_Save);
	gtk_box_pack_start(GTK_BOX(hbutton_box_bottomRow), button_GLRes_Save, FALSE, FALSE, 0);
	gtk_widget_add_accelerator(button_GLRes_Save, "activate", accel_group,
				   GDK_Return, (GdkModifierType)(0), (GtkAccelFlags)(0));
	gtk_widget_add_accelerator(button_GLRes_Save, "activate", accel_group,
				   GDK_KP_Enter, (GdkModifierType)(0), (GtkAccelFlags)(0));
	AddButtonCallback_Clicked(button_GLRes_Save, on_button_GLRes_Save_clicked);
	GLADE_HOOKUP_OBJECT(opengl_resolution_window, button_GLRes_Save, "button_GLRes_Save");
	
	// Add the accel group.
	gtk_window_add_accel_group(GTK_WINDOW(opengl_resolution_window), accel_group);
	
	return opengl_resolution_window;
}
