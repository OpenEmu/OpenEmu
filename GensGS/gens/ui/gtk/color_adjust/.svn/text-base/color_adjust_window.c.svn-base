/***************************************************************************
 * Gens: (GTK+) Color Adjustment Window.                                   *
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

#include "color_adjust_window.h"
#include "color_adjust_window_callbacks.h"
#include "gens/gens_window.hpp"

#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <string.h>
#include <stdio.h>

#include <gtk/gtk.h>

// GENS GTK+ miscellaneous functions
#include "gtk-misc.h"

#include "emulator/g_main.hpp"


// Macro to add a slider to the table.
#define CREATE_TABLE_SLIDER(Table, TableRow, Caption, SliderGroup, 		\
			    LabelWidget, SliderWidget)				\
{										\
	LabelWidget = gtk_label_new(Caption);					\
	gtk_widget_set_name(LabelWidget, "label_slider_" SliderGroup);		\
	gtk_misc_set_alignment(GTK_MISC(LabelWidget), 0, 0.5);			\
	gtk_widget_show(LabelWidget);						\
	gtk_table_attach(GTK_TABLE(Table), LabelWidget,				\
			 0, 1, TableRow, TableRow + 1,				\
			 (GtkAttachOptions)(GTK_FILL),				\
			 (GtkAttachOptions)(0), 0, 0);				\
	GLADE_HOOKUP_OBJECT(color_adjust_window, LabelWidget,			\
			    "label_slider_" SliderGroup);			\
										\
	SliderWidget = gtk_hscale_new(GTK_ADJUSTMENT(				\
			gtk_adjustment_new(0, -100, 100, 1, 0, 0)));		\
	gtk_widget_set_name(SliderWidget, "hscale_slider_" SliderGroup);	\
	gtk_scale_set_value_pos(GTK_SCALE(SliderWidget), GTK_POS_RIGHT);	\
	gtk_scale_set_digits(GTK_SCALE(SliderWidget), 0);			\
	gtk_widget_set_size_request(SliderWidget, 300, -1);			\
	gtk_widget_show(SliderWidget);						\
	gtk_table_attach(GTK_TABLE(Table), SliderWidget,			\
			 1, 2, TableRow, TableRow + 1,				\
			 (GtkAttachOptions)(GTK_EXPAND | GTK_FILL),		\
			 (GtkAttachOptions)(GTK_FILL), 0, 0);			\
	GLADE_HOOKUP_OBJECT(color_adjust_window, SliderWidget,			\
			    "hscale_slider_" SliderGroup);			\
}


// Macro to add a checkbox to a Box.
#define CREATE_BOX_CHECKBOX(Container, Caption, CheckWidget, CheckName)		\
{										\
	CheckWidget = gtk_check_button_new_with_mnemonic(Caption);		\
	gtk_widget_set_name(CheckWidget, CheckName);				\
	gtk_widget_show(CheckWidget);						\
	gtk_box_pack_start(GTK_BOX(Container), CheckWidget, FALSE, FALSE, 0);	\
	GLADE_HOOKUP_OBJECT(color_adjust_window, CheckWidget, CheckName);	\
}


GtkWidget *color_adjust_window = NULL;

GtkAccelGroup *accel_group;


/**
 * create_color_adjust_window(): Create the Color Adjustment Window.
 * @return Color Adjustment Window.
 */
GtkWidget* create_color_adjust_window(void)
{
	GdkPixbuf *color_adjust_window_icon_pixbuf;
	GtkWidget *vbox_ca;
	GtkWidget *vbox_adjustment;
	GtkWidget *table_sliders;
	GtkWidget *label_slider_contrast, *hscale_slider_contrast;
	GtkWidget *label_slider_brightness, *hscale_slider_brightness;
	GtkWidget *hbutton_box_options;
	GtkWidget *check_options_grayscale, *check_options_inverted;
	GtkWidget *hbutton_box_bottomRow;
	GtkWidget *button_ca_Cancel, *button_ca_Apply, *button_ca_Save;
	
	if (color_adjust_window)
	{
		// Color Adjustment window is already created. Set focus.
		gtk_widget_grab_focus(color_adjust_window);
		return NULL;
	}
	
	accel_group = gtk_accel_group_new();
	
	// Create the Color Adjustment window.
	CREATE_GTK_WINDOW(color_adjust_window,
			  "color_adjust_window",
			  "Color Adjustment",
			  color_adjust_window_icon_pixbuf, "Gens2.ico");
	
	// Callbacks for if the window is closed.
	g_signal_connect((gpointer)color_adjust_window, "delete_event",
			 G_CALLBACK(on_color_adjust_window_close), NULL);
	g_signal_connect((gpointer)color_adjust_window, "destroy_event",
			 G_CALLBACK(on_color_adjust_window_close), NULL);
	
	// Create the main VBox.
	vbox_ca = gtk_vbox_new(FALSE, 5);
	gtk_widget_set_name(vbox_ca, "vbox_ca");
	gtk_widget_show(vbox_ca);
	gtk_container_add(GTK_CONTAINER(color_adjust_window), vbox_ca);
	GLADE_HOOKUP_OBJECT(color_adjust_window, vbox_ca, "vbox_ca");
	
	// Create the VBox for the color adjustment widgets.
	vbox_adjustment = gtk_vbox_new(FALSE, 5);
	gtk_widget_set_name(vbox_adjustment, "vbox_adjustment");
	gtk_widget_show(vbox_adjustment);
	gtk_box_pack_start(GTK_BOX(vbox_ca), vbox_adjustment, TRUE, TRUE, 0);
	GLADE_HOOKUP_OBJECT(color_adjust_window, vbox_adjustment, "vbox_adjustment");
	
	// Table for the sliders.
	table_sliders = gtk_table_new(2, 2, FALSE);
	gtk_widget_set_name(table_sliders, "table_sliders");
	gtk_table_set_row_spacings(GTK_TABLE(table_sliders), 5);
	gtk_table_set_col_spacings(GTK_TABLE(table_sliders), 5);
	gtk_widget_show(table_sliders);
	gtk_box_pack_start(GTK_BOX(vbox_adjustment), table_sliders, TRUE, FALSE, 0);
	GLADE_HOOKUP_OBJECT(color_adjust_window, table_sliders, "table_sliders");
	
	// Add the sliders.
	CREATE_TABLE_SLIDER(table_sliders, 0, "Contrast", "contrast",
			    label_slider_contrast, hscale_slider_contrast);
	CREATE_TABLE_SLIDER(table_sliders, 1, "Brightness", "brightness",
			    label_slider_brightness, hscale_slider_brightness);
	
	// Create an HButton Box for some miscellaneous options.
	hbutton_box_options = gtk_hbutton_box_new();
	gtk_widget_set_name(hbutton_box_options, "hbutton_box_options");
	gtk_button_box_set_layout(GTK_BUTTON_BOX(hbutton_box_options), GTK_BUTTONBOX_SPREAD);
	gtk_widget_show(hbutton_box_options);
	gtk_box_pack_start(GTK_BOX(vbox_ca), hbutton_box_options, FALSE, FALSE, 0);
	GLADE_HOOKUP_OBJECT(color_adjust_window, hbutton_box_options, "hbutton_box_options");
	
	// Miscellaneous options.
	CREATE_BOX_CHECKBOX(hbutton_box_options, "_Grayscale",
			    check_options_grayscale, "check_options_grayscale");
	CREATE_BOX_CHECKBOX(hbutton_box_options, "_Inverted",
			    check_options_inverted, "check_options_inverted");
	
	// Create an HButton Box for the buttons on the bottom.
	hbutton_box_bottomRow = gtk_hbutton_box_new();
	gtk_widget_set_name(hbutton_box_bottomRow, "hbutton_box_bottomRow");
	gtk_button_box_set_layout(GTK_BUTTON_BOX(hbutton_box_bottomRow), GTK_BUTTONBOX_END);
	gtk_widget_show(hbutton_box_bottomRow);
	gtk_box_pack_start(GTK_BOX(vbox_ca), hbutton_box_bottomRow, FALSE, FALSE, 0);
	GLADE_HOOKUP_OBJECT(color_adjust_window, hbutton_box_bottomRow, "hbutton_box_bottomRow");
	
	// Cancel
	button_ca_Cancel = gtk_button_new_from_stock(GTK_STOCK_CANCEL);
	gtk_widget_set_name(button_ca_Cancel, "button_ca_Cancel");
	gtk_widget_show(button_ca_Cancel);
	gtk_box_pack_start(GTK_BOX(hbutton_box_bottomRow), button_ca_Cancel, FALSE, FALSE, 0);
	gtk_widget_add_accelerator(button_ca_Cancel, "activate", accel_group,
				   GDK_Escape, (GdkModifierType)(0), (GtkAccelFlags)(0));
	AddButtonCallback_Clicked(button_ca_Cancel, on_button_ca_Cancel_clicked);
	GLADE_HOOKUP_OBJECT(color_adjust_window, button_ca_Cancel, "button_ca_Cancel");
	
	// Apply
	button_ca_Apply = gtk_button_new_from_stock(GTK_STOCK_APPLY);
	gtk_widget_set_name(button_ca_Apply, "button_ca_Apply");
	gtk_widget_show(button_ca_Apply);
	gtk_box_pack_start(GTK_BOX(hbutton_box_bottomRow), button_ca_Apply, FALSE, FALSE, 0);
	AddButtonCallback_Clicked(button_ca_Apply, on_button_ca_Apply_clicked);
	GLADE_HOOKUP_OBJECT(color_adjust_window, button_ca_Apply, "button_ca_Apply");
	
	// Save
	button_ca_Save = gtk_button_new_from_stock(GTK_STOCK_SAVE);
	gtk_widget_set_name(button_ca_Save, "button_ca_Save");
	gtk_widget_show(button_ca_Save);
	gtk_box_pack_start(GTK_BOX(hbutton_box_bottomRow), button_ca_Save, FALSE, FALSE, 0);
	gtk_widget_add_accelerator(button_ca_Save, "activate", accel_group,
				   GDK_Return, (GdkModifierType)(0), (GtkAccelFlags)(0));
	gtk_widget_add_accelerator(button_ca_Save, "activate", accel_group,
				   GDK_KP_Enter, (GdkModifierType)(0), (GtkAccelFlags)(0));
	AddButtonCallback_Clicked(button_ca_Save, on_button_ca_Save_clicked);
	GLADE_HOOKUP_OBJECT(color_adjust_window, button_ca_Save, "button_ca_Save");
	
	// Add the accel group.
	gtk_window_add_accel_group(GTK_WINDOW(color_adjust_window), accel_group);
	
	return color_adjust_window;
}
