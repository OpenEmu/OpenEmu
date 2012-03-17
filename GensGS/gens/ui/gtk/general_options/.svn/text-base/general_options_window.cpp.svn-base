/***************************************************************************
 * Gens: (GTK+) General Options Window.                                    *
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

#include "general_options_window.hpp"
#include "gens/gens_window.hpp"

#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <string.h>
#include <stdio.h>

#include <gtk/gtk.h>

// TODO: Get rid of gtk-misc.h
#include "gtk-misc.h"

#include "emulator/g_main.hpp"


// On-Screen Display colors.
// Index: 0 = name; 1 = normal; 2 = active; 3 = prelight; 4 = selected; 5 = insensitive
const char* GeneralOptionsWindow::Colors_OSD[5][6] =
{
	{"white", "#FFFFFF", "#E0E0E0", "#FFFFFF", "#FFFFFF", "#C0C0C0"},
	{"blue",  "#0000FF", "#0000E0", "#8080FF", "#0000FF", "#1C1C1C"},
	{"green", "#00FF00", "#00E000", "#80FF80", "#00FF00", "#979797"},
	{"red",   "#FF0000", "#E00000", "#FF8080", "#FF0000", "#4C4C4C"},
	{NULL, NULL, NULL, NULL, NULL, NULL}
};

// Intro effect colors.
// Index: 0 = name; 1 = normal; 2 = active; 3 = prelight; 4 = selected; 5 = insensitive
const char* GeneralOptionsWindow::Colors_IntroEffect[9][6] =
{
	{"black",  "#000000", "#000000", "#808080", "#000000", "#808080"},
	{"blue",   "#0000FF", "#0000E0", "#8080FF", "#0000FF", "#202020"},
	{"green",  "#00FF00", "#00E000", "#80FF80", "#00FF00", "#989898"},
	{"cyan",   "#00FFFF", "#00E0E0", "#80FFFF", "#00FFFF", "#B3B3B3"},
	{"red",    "#FF0000", "#E00000", "#FF8080", "#FF0000", "#4C4C4C"},
	{"purple", "#FF00FF", "#E000E0", "#FF80FF", "#FF00FF", "#686868"},
	{"yellow", "#FFFF00", "#E0E000", "#FFFF80", "#FFFF00", "#E3E3E3"},
	{"white",  "#FFFFFF", "#E0E0E0", "#FFFFFF", "#FFFFFF", "#C0C0C0"},
	{NULL, NULL, NULL, NULL, NULL, NULL}
};


GeneralOptionsWindow* GeneralOptionsWindow::m_Instance = NULL;
GeneralOptionsWindow* GeneralOptionsWindow::Instance(GtkWindow *parent)
{
	if (m_Instance == NULL)
	{
		// Instance is deleted. Initialize the General Options window.
		m_Instance = new GeneralOptionsWindow();
	}
	else
	{
		// Instance already exists. Set focus.
		m_Instance->setFocus();
	}
	
	// Set modality of the window.
	if (!parent)
		parent = GTK_WINDOW(gens_window);
	m_Instance->setModal(parent);
	
	return m_Instance;
}


GeneralOptionsWindow::~GeneralOptionsWindow()
{
	if (m_Window)
		gtk_widget_destroy(GTK_WIDGET(m_Window));
	
	m_Instance = NULL;
}


gboolean GeneralOptionsWindow::GTK_Close(GtkWidget *widget, GdkEvent *event, gpointer user_data)
{
	return reinterpret_cast<GeneralOptionsWindow*>(user_data)->close();
}


void GeneralOptionsWindow::dlgButtonPress(uint32_t button)
{
	switch (button)
	{
		case WndBase::BUTTON_SAVE:
			save();
			close();
			break;
		
		case WndBase::BUTTON_APPLY:
			save();
			break;
		
		case WndBase::BUTTON_CANCEL:
			close();
			break;
	}
}


/**
 * Window is closed.
 */
gboolean GeneralOptionsWindow::close(void)
{
	delete this;
	return FALSE;
}


/**
 * GeneralOptionsWindow: Create the General Options Window.
 */
GeneralOptionsWindow::GeneralOptionsWindow()
{
	m_Window = gtk_dialog_new();
	gtk_widget_set_name(GTK_WIDGET(m_Window), "GeneralOptionsWindow");
	gtk_container_set_border_width(GTK_CONTAINER(m_Window), 0);
	gtk_window_set_title(GTK_WINDOW(m_Window), "General Options");
	gtk_window_set_position(GTK_WINDOW(m_Window), GTK_WIN_POS_CENTER);
	gtk_window_set_resizable(GTK_WINDOW(m_Window), FALSE);
	gtk_window_set_type_hint(GTK_WINDOW(m_Window), GDK_WINDOW_TYPE_HINT_DIALOG);
	gtk_dialog_set_has_separator(GTK_DIALOG(m_Window), FALSE);
	
	// Set the window data.
	g_object_set_data(G_OBJECT(m_Window), "GeneralOptionsWindow", m_Window);
	
	// Load the Gens icon.
	GdkPixbuf *icon = create_pixbuf("Gens2.ico");
	if (icon)
	{
		gtk_window_set_icon(GTK_WINDOW(m_Window), icon);
		gdk_pixbuf_unref(icon);
	}
	
	// Callbacks for if the window is closed.
	g_signal_connect((gpointer)m_Window, "delete_event",
			  G_CALLBACK(GeneralOptionsWindow::GTK_Close), (gpointer)this);
	g_signal_connect((gpointer)m_Window, "destroy_event",
			  G_CALLBACK(GeneralOptionsWindow::GTK_Close), (gpointer)this);
	
	// Get the dialog VBox.
	GtkWidget *vboxDialog = GTK_DIALOG(m_Window)->vbox;
	gtk_widget_set_name(vboxDialog, "vboxDialog");
	gtk_widget_show(vboxDialog);
	g_object_set_data_full(G_OBJECT(m_Window), "vboxDialog",
			       g_object_ref(vboxDialog), (GDestroyNotify)g_object_unref);
	
	// Create the main HBox.
	GtkWidget *hboxMain = gtk_hbox_new(FALSE, 0);
	gtk_widget_set_name(hboxMain, "hboxMain");
	gtk_widget_show(hboxMain);
	g_object_set_data_full(G_OBJECT(m_Window), "hboxMain",
			       g_object_ref(hboxMain), (GDestroyNotify)g_object_unref);
	gtk_box_pack_start(GTK_BOX(vboxDialog), hboxMain, TRUE, TRUE, 0);
	
	// On-screen display frame.
	GtkWidget *fraOSDMain = gtk_frame_new(NULL);
	gtk_widget_set_name(fraOSDMain, "fraOSDMain");
	gtk_frame_set_shadow_type(GTK_FRAME(fraOSDMain), GTK_SHADOW_ETCHED_IN);
	gtk_container_set_border_width(GTK_CONTAINER(fraOSDMain), 4);
	gtk_widget_show(fraOSDMain);
	g_object_set_data_full(G_OBJECT(m_Window), "fraOSDMain",
			       g_object_ref(fraOSDMain), (GDestroyNotify)g_object_unref);
	gtk_box_pack_start(GTK_BOX(hboxMain), fraOSDMain, TRUE, TRUE, 0);
	
	// Label for the on-screen display frame.
	GtkWidget *lblOSDMain = gtk_label_new("<b><i>On-Screen Display</i></b>");
	gtk_widget_set_name(lblOSDMain, "lblOSDMain");
	gtk_label_set_use_markup(GTK_LABEL(lblOSDMain), TRUE);
	gtk_widget_show(lblOSDMain);
	g_object_set_data_full(G_OBJECT(m_Window), "lblOSDMain",
			       g_object_ref(lblOSDMain), (GDestroyNotify)g_object_unref);
	gtk_frame_set_label_widget(GTK_FRAME(fraOSDMain), lblOSDMain);
	
	// VBox for the on-screen display frame.
	GtkWidget *vboxOSDMain = gtk_vbox_new(FALSE, 0);
	gtk_widget_set_name(vboxOSDMain, "vboxOSDMain");
	gtk_widget_show(vboxOSDMain);
	g_object_set_data_full(G_OBJECT(m_Window), "vboxOSDMain",
			       g_object_ref(vboxOSDMain), (GDestroyNotify)g_object_unref);
	gtk_container_add(GTK_CONTAINER(fraOSDMain), vboxOSDMain);
	
	// Add FPS and Message frames to the OSD frame.
	createOSDFrame("FPS counter", 0, vboxOSDMain);
	createOSDFrame("Message", 1, vboxOSDMain);
	
	// Miscellaneous settings frame.
	GtkWidget *fraMisc = gtk_frame_new(NULL);
	gtk_widget_set_name(fraMisc, "fraMisc");
	gtk_frame_set_shadow_type(GTK_FRAME(fraMisc), GTK_SHADOW_ETCHED_IN);
	gtk_container_set_border_width(GTK_CONTAINER(fraMisc), 4);
	gtk_widget_show(fraMisc);
	g_object_set_data_full(G_OBJECT(m_Window), "fraMisc",
			       g_object_ref(fraMisc), (GDestroyNotify)g_object_unref);
	gtk_box_pack_start(GTK_BOX(hboxMain), fraMisc, TRUE, TRUE, 0);
	
	// Label for the miscellaneous settings frame.
	GtkWidget *lblMisc = gtk_label_new("<b><i>Miscellaneous</i></b>");
	gtk_widget_set_name(lblMisc, "lblMisc");
	gtk_label_set_use_markup(GTK_LABEL(lblMisc), TRUE);
	gtk_widget_show(lblMisc);
	g_object_set_data_full(G_OBJECT(m_Window), "lblMisc",
			       g_object_ref(lblMisc), (GDestroyNotify)g_object_unref);
	gtk_frame_set_label_widget(GTK_FRAME(fraMisc), lblMisc);
	
	// VBox for the miscellaneous settings frame.
	GtkWidget *vboxMisc = gtk_vbox_new(FALSE, 0);
	gtk_widget_set_name(vboxMisc, "vboxMisc");
	gtk_widget_show(vboxMisc);
	g_object_set_data_full(G_OBJECT(m_Window), "vboxMisc",
			       g_object_ref(vboxMisc), (GDestroyNotify)g_object_unref);
	gtk_container_add(GTK_CONTAINER(fraMisc), vboxMisc);
	
	// Miscellaneous settings checkboxes.
	
	// Auto Fix Checksum
	chkMisc_AutoFixChecksum = gtk_check_button_new_with_label("Auto Fix Checksum");
	gtk_widget_set_name(chkMisc_AutoFixChecksum, "chkMisc_AutoFixChecksum");
	gtk_container_set_border_width(GTK_CONTAINER(chkMisc_AutoFixChecksum), 2);
	gtk_widget_show(chkMisc_AutoFixChecksum);
	g_object_set_data_full(G_OBJECT(m_Window), "chkMisc_AutoFixChecksum",
			       g_object_ref(chkMisc_AutoFixChecksum), (GDestroyNotify)g_object_unref);
	gtk_box_pack_start(GTK_BOX(vboxMisc), chkMisc_AutoFixChecksum, FALSE, FALSE, 0);
	
	// Auto Fix Checksum
	chkMisc_AutoPause = gtk_check_button_new_with_label("Auto Pause");
	gtk_widget_set_name(chkMisc_AutoPause, "chkMisc_AutoPause");
	gtk_container_set_border_width(GTK_CONTAINER(chkMisc_AutoPause), 2);
	gtk_widget_show(chkMisc_AutoPause);
	g_object_set_data_full(G_OBJECT(m_Window), "chkMisc_AutoPause",
			       g_object_ref(chkMisc_AutoPause), (GDestroyNotify)g_object_unref);
	gtk_box_pack_start(GTK_BOX(vboxMisc), chkMisc_AutoPause, FALSE, FALSE, 0);
	
	// Fast Blur
	chkMisc_FastBlur = gtk_check_button_new_with_label("Fast Blur");
	gtk_widget_set_name(chkMisc_FastBlur, "chkMisc_FastBlur");
	gtk_container_set_border_width(GTK_CONTAINER(chkMisc_FastBlur), 2);
	gtk_widget_show(chkMisc_FastBlur);
	g_object_set_data_full(G_OBJECT(m_Window), "chkMisc_FastBlur",
			       g_object_ref(chkMisc_FastBlur), (GDestroyNotify)g_object_unref);
	gtk_box_pack_start(GTK_BOX(vboxMisc), chkMisc_FastBlur, FALSE, FALSE, 0);
	
	// Show SegaCD LEDs
	chkMisc_SegaCDLEDs = gtk_check_button_new_with_label("Show SegaCD LEDs");
	gtk_widget_set_name(chkMisc_SegaCDLEDs, "chkMisc_SegaCDLEDs");
	gtk_container_set_border_width(GTK_CONTAINER(chkMisc_SegaCDLEDs), 2);
	gtk_widget_show(chkMisc_SegaCDLEDs);
	g_object_set_data_full(G_OBJECT(m_Window), "chkMisc_SegaCDLEDs",
			       g_object_ref(chkMisc_SegaCDLEDs), (GDestroyNotify)g_object_unref);
	gtk_box_pack_start(GTK_BOX(vboxMisc), chkMisc_SegaCDLEDs, FALSE, FALSE, 0);
	
	// Border Color Emulation
	chkMisc_BorderColorEmulation = gtk_check_button_new_with_label("Border Color Emulation");
	gtk_widget_set_name(chkMisc_BorderColorEmulation, "chkMisc_BorderColorEmulation");
	gtk_container_set_border_width(GTK_CONTAINER(chkMisc_BorderColorEmulation), 2);
	gtk_widget_show(chkMisc_BorderColorEmulation);
	g_object_set_data_full(G_OBJECT(m_Window), "chkMisc_BorderColorEmulation",
			       g_object_ref(chkMisc_BorderColorEmulation), (GDestroyNotify)g_object_unref);
	gtk_box_pack_start(GTK_BOX(vboxMisc), chkMisc_BorderColorEmulation, FALSE, FALSE, 0);
	
	// Pause Tint
	chkMisc_PauseTint = gtk_check_button_new_with_label("Pause Tint");
	gtk_widget_set_name(chkMisc_PauseTint, "chkMisc_PauseTint");
	gtk_container_set_border_width(GTK_CONTAINER(chkMisc_PauseTint), 2);
	gtk_widget_show(chkMisc_PauseTint);
	g_object_set_data_full(G_OBJECT(m_Window), "chkMisc_PauseTint",
			       g_object_ref(chkMisc_PauseTint), (GDestroyNotify)g_object_unref);
	gtk_box_pack_start(GTK_BOX(vboxMisc), chkMisc_PauseTint, FALSE, FALSE, 0);
	
	// VBox for intro effect color
	GtkWidget *vboxIntroEffectColor = gtk_vbox_new(FALSE, 0);
	gtk_widget_set_name(vboxIntroEffectColor, "vboxIntroEffectColor");
	gtk_container_set_border_width(GTK_CONTAINER(vboxIntroEffectColor), 8);
	gtk_widget_show(vboxIntroEffectColor);
	g_object_set_data_full(G_OBJECT(m_Window), "vboxIntroEffectColor",
			       g_object_ref(vboxIntroEffectColor), (GDestroyNotify)g_object_unref);
	gtk_container_add(GTK_CONTAINER(vboxMisc), vboxIntroEffectColor);
	
	// Label for intro effect color
	GtkWidget *lblIntroEffectColor = gtk_label_new("Intro Effect Color:");
	gtk_widget_set_name(lblIntroEffectColor, "lblIntroEffectColor");
	gtk_widget_show(lblIntroEffectColor);
	g_object_set_data_full(G_OBJECT(m_Window), "lblIntroEffectColor",
			       g_object_ref(lblIntroEffectColor), (GDestroyNotify)g_object_unref);
	gtk_box_pack_start(GTK_BOX(vboxIntroEffectColor), lblIntroEffectColor, FALSE, FALSE, 0);
	
	createColorRadioButtons(NULL, "misc_intro_effect",
				Colors_IntroEffect, 0, vboxIntroEffectColor);
	
	// Create an accelerator group.
	m_AccelTable = gtk_accel_group_new();
	
	// Add the OK button.
	addDialogButtons(m_Window, WndBase::BAlign_Default,
			 WndBase::BUTTON_CANCEL | WndBase::BUTTON_APPLY | WndBase::BUTTON_SAVE, 0,
			 WndBase::BUTTON_ALL);
	
	// Add the accel group to the window.
	gtk_window_add_accel_group(GTK_WINDOW(m_Window), GTK_ACCEL_GROUP(m_AccelTable));
	
	// Load settings.
	load();
	
	// Show the window.
	setVisible(true);
}


/**
 * createOSDFrame(): Creates an on-screen display frame.
 * @param title Title for this frame.
 * @param index Frame index.
 * @param container Container for the frame.
 */
void GeneralOptionsWindow::createOSDFrame(const char* title, const int index, GtkWidget *container)
{
	char tmp[64];
	
	// Create the frame.
	GtkWidget *fraOSD = gtk_frame_new(NULL);
	gtk_widget_set_name(fraOSD, "fraOSD");
	gtk_frame_set_shadow_type(GTK_FRAME(fraOSD), GTK_SHADOW_ETCHED_IN);
	gtk_container_set_border_width(GTK_CONTAINER(fraOSD), 4);
	gtk_widget_show(fraOSD);
	sprintf(tmp, "fraOSD_%d", index);
	g_object_set_data_full(G_OBJECT(m_Window), tmp,
			       g_object_ref(fraOSD), (GDestroyNotify)g_object_unref);
	gtk_box_pack_start(GTK_BOX(container), fraOSD, TRUE, TRUE, 0);
	
	// Create the label for the frame.
	snprintf(tmp, sizeof(tmp), "<b><i>%s</i></b>", title);
	tmp[sizeof(tmp) - 1] = 0x00;
	GtkWidget *lblOSD = gtk_label_new(tmp);
	
	sprintf(tmp, "lblOSD_%d", index);
	gtk_widget_set_name(lblOSD, tmp);
	gtk_label_set_use_markup(GTK_LABEL(lblOSD), TRUE);
	gtk_widget_show(lblOSD);
	g_object_set_data_full(G_OBJECT(m_Window), tmp,
			       g_object_ref(lblOSD), (GDestroyNotify)g_object_unref);
	gtk_frame_set_label_widget(GTK_FRAME(fraOSD), lblOSD);
	
	// Create the vbox for the frame.
	GtkWidget *vboxOSD = gtk_vbox_new(FALSE, 0);
	sprintf(tmp, "vboxOSD_%d", index);
	gtk_widget_set_name(vboxOSD, tmp);
	gtk_widget_show(vboxOSD);
	g_object_set_data_full(G_OBJECT(m_Window), tmp,
			       g_object_ref(vboxOSD), (GDestroyNotify)g_object_unref);
	gtk_container_add(GTK_CONTAINER(fraOSD), vboxOSD);
	
	// Create the checkboxes.
	
	// Enable
	chkOSD_Enable[index] = gtk_check_button_new_with_label("Enable");
	sprintf(tmp, "chkOSD_Enable_%d", index);
	gtk_widget_set_name(chkOSD_Enable[index], tmp);
	gtk_container_set_border_width(GTK_CONTAINER(chkOSD_Enable[index]), 2);
	gtk_widget_show(chkOSD_Enable[index]);
	g_object_set_data_full(G_OBJECT(m_Window), tmp,
			       g_object_ref(chkOSD_Enable[index]), (GDestroyNotify)g_object_unref);
	gtk_box_pack_start(GTK_BOX(vboxOSD), chkOSD_Enable[index], FALSE, FALSE, 0);
	
	// Double Sized
	chkOSD_DoubleSized[index] = gtk_check_button_new_with_label("Double Sized");
	sprintf(tmp, "chkOSD_DoubleSized_%d", index);
	gtk_widget_set_name(chkOSD_DoubleSized[index], tmp);
	gtk_container_set_border_width(GTK_CONTAINER(chkOSD_DoubleSized[index]), 2);
	gtk_widget_show(chkOSD_DoubleSized[index]);
	g_object_set_data_full(G_OBJECT(m_Window), tmp,
			       g_object_ref(chkOSD_DoubleSized[index]), (GDestroyNotify)g_object_unref);
	gtk_box_pack_start(GTK_BOX(vboxOSD), chkOSD_DoubleSized[index], FALSE, FALSE, 0);
	
	// Transparency
	chkOSD_Transparency[index] = gtk_check_button_new_with_label("Transparency");
	sprintf(tmp, "chkOSD_Transparency_%d", index);
	gtk_widget_set_name(chkOSD_Transparency[index], tmp);
	gtk_container_set_border_width(GTK_CONTAINER(chkOSD_Transparency[index]), 2);
	gtk_widget_show(chkOSD_Transparency[index]);
	g_object_set_data_full(G_OBJECT(m_Window), tmp,
			       g_object_ref(chkOSD_Transparency[index]), (GDestroyNotify)g_object_unref);
	gtk_box_pack_start(GTK_BOX(vboxOSD), chkOSD_Transparency[index], FALSE, FALSE, 0);
	
	// Color HBox
	GtkWidget *hboxOSD_Color = gtk_hbox_new(FALSE, 0);
	sprintf(tmp, "hboxOSD_Color_%d", index);
	gtk_widget_set_name(hboxOSD_Color, tmp);
	gtk_container_set_border_width(GTK_CONTAINER(hboxOSD_Color), 8);
	gtk_widget_show(hboxOSD_Color);
	g_object_set_data_full(G_OBJECT(m_Window), tmp,
			       g_object_ref(hboxOSD_Color), (GDestroyNotify)g_object_unref);
	gtk_container_add(GTK_CONTAINER(vboxOSD), hboxOSD_Color);
	
	// Color Radio Buttons
	sprintf(tmp, "OSD_%d", index);
	createColorRadioButtons("Color:", tmp, Colors_OSD, index, hboxOSD_Color);
}


/**
 * createColorRadioButtons(): Create color radio buttons.
 * @param title Title for this color button group.
 * @param groupName Prefix for each button's name.
 * @param colors Array of colors.
 * @param num Number of colors to use.
 * @param buttonSet Button set ID.
 * @param container Container for the radio buttons.
 */
void GeneralOptionsWindow::createColorRadioButtons(const char* title,
						   const char* groupName,
						   const char* colors[][6],
						   const int buttonSet,
						   GtkWidget* container)
{
	int numColors = 0;
	
	// Make sure a valid button set is specified.
	if (colors == Colors_OSD)
	{
		if (buttonSet < 0 || buttonSet > 1)
			return;
		numColors = 4;
	}
	else if (colors == Colors_IntroEffect)
	{
		if (buttonSet != 0)
			return;
		numColors = 8;
	}
	else
	{
		return;
	}
	
	char tmp[64];
	
	// Color label
	if (title)
	{
		// Alignment object.
		GtkWidget *alignColor = gtk_alignment_new(0.0f, 0.0f, 1.0f, 1.0f);
		sprintf(tmp, "alignColor_%s", groupName);
		gtk_alignment_set_padding(GTK_ALIGNMENT(alignColor), 0, 0, 0, 8);
		gtk_widget_show(alignColor);
		g_object_set_data_full(G_OBJECT(m_Window), tmp,
				       g_object_ref(alignColor), (GDestroyNotify)g_object_unref);
		gtk_box_pack_start(GTK_BOX(container), alignColor, FALSE, FALSE, 0);
		
		// Label object.
		GtkWidget *lblColor = gtk_label_new(title);
		sprintf(tmp, "lblColor_%s", groupName);
		gtk_widget_set_name(lblColor, tmp);
		gtk_widget_show(lblColor);
		g_object_set_data_full(G_OBJECT(m_Window), tmp,
				       g_object_ref(lblColor), (GDestroyNotify)g_object_unref);
		gtk_container_add(GTK_CONTAINER(alignColor), lblColor);
	}
	
	// HBox for the color buttons.
	GtkWidget *hboxButtons = gtk_hbox_new(TRUE, 0);
	sprintf(tmp, "hboxButtons_%s", groupName);
	gtk_widget_set_name(hboxButtons, tmp);
	gtk_widget_show(hboxButtons);
	g_object_set_data_full(G_OBJECT(m_Window), tmp,
			       g_object_ref(hboxButtons), (GDestroyNotify)g_object_unref);
	gtk_box_pack_start(GTK_BOX(container), hboxButtons, FALSE, FALSE, 0);
	
	// Color Buttons
	GtkWidget *optColorButton;
	GSList *gslColorButton = NULL;
	GdkColor tmpColor;
	
	for (int i = 0; i < numColors; i++)
	{
		if (!colors[i][0])
			return;
		
		optColorButton = gtk_radio_button_new(gslColorButton);
		gslColorButton = gtk_radio_button_get_group(GTK_RADIO_BUTTON(optColorButton));
		sprintf(tmp, "optColorButton_%s_%d", groupName, i);
		gtk_widget_set_name(optColorButton, tmp);
		
		// Display as a regular button, not as a radio button.
		gtk_toggle_button_set_mode(GTK_TOGGLE_BUTTON(optColorButton), FALSE);
		
		// Set colors.
		// GTK_STATE_NORMAL == 0
		// GTK_STATE_INSENSITIVE == 4
		for (int j = GTK_STATE_NORMAL; j <= GTK_STATE_INSENSITIVE; j++)
		{
			gdk_color_parse(colors[i][j + 1], &tmpColor);
			gtk_widget_modify_bg(optColorButton, (GtkStateType)j, &tmpColor);
		}
		
		gtk_widget_set_size_request(optColorButton, 16, 16);
		gtk_widget_show(optColorButton);
		g_object_set_data_full(G_OBJECT(m_Window), tmp,
				       g_object_ref(optColorButton), (GDestroyNotify)g_object_unref);
		gtk_box_pack_start(GTK_BOX(hboxButtons), optColorButton, TRUE, TRUE, 0);
		
		// Save the color button.
		if (colors == Colors_OSD)
		{
			// OSD color button.
			optOSD_Color[buttonSet][i] = optColorButton;
		}
		else //if (colors == Colors_IntroEffect)
		{
			// Intro Effect color button.
			optIntroEffectColor[i] = optColorButton;
		}
	}
}


/**
 * load(): Load settings.
 */
void GeneralOptionsWindow::load(void)
{
	// Get the current options.
	
	// Miscellaneous
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(chkMisc_AutoFixChecksum), Auto_Fix_CS);
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(chkMisc_AutoPause), Auto_Pause);
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(chkMisc_FastBlur), draw->fastBlur());
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(chkMisc_SegaCDLEDs), Show_LED);
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(chkMisc_BorderColorEmulation), Video.borderColorEmulation);
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(chkMisc_PauseTint), Video.pauseTint);
	
	// FPS counter
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(chkOSD_Enable[0]), (draw->fpsEnabled() ? 1 : 0));
	
	unsigned char curFPSStyle = draw->fpsStyle();
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(chkOSD_DoubleSized[0]), (curFPSStyle & 0x10));
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(chkOSD_Transparency[0]), (curFPSStyle & 0x08));
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(optOSD_Color[0][(curFPSStyle & 0x06) >> 1]), TRUE);
	
	// Message
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(chkOSD_Enable[1]), (draw->msgEnabled() ? 1 : 0));
	
	unsigned char curMsgStyle = draw->msgStyle();
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(chkOSD_DoubleSized[1]), (curMsgStyle & 0x10));
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(chkOSD_Transparency[1]), (curMsgStyle & 0x08));
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(optOSD_Color[1][(curMsgStyle & 0x06) >> 1]), TRUE);
	
	// Intro effect color
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(optIntroEffectColor[draw->introEffectColor()]), TRUE);
}


/**
 * save(): Save settings.
 */
void GeneralOptionsWindow::save(void)
{
	// Save the current options.
	
	// Miscellaneous
	Auto_Fix_CS = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(chkMisc_AutoFixChecksum));
	Auto_Pause = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(chkMisc_AutoPause));
	draw->setFastBlur(gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(chkMisc_FastBlur)));
	Show_LED = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(chkMisc_SegaCDLEDs));
	Video.borderColorEmulation = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(chkMisc_BorderColorEmulation));
	Video.pauseTint = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(chkMisc_PauseTint));
	
	// If Auto Pause is enabled, deactivate emulation.
	// If Auto Pause is disabled, activate emulation.
	Active = !Auto_Pause;
	
	// FPS counter
	draw->setFPSEnabled(gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(chkOSD_Enable[0])));
	
	unsigned char curFPSStyle = draw->fpsStyle() & ~0x18;
	curFPSStyle |= (gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(chkOSD_DoubleSized[0])) ? 0x10 : 0x00);
	curFPSStyle |= (gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(chkOSD_Transparency[0])) ? 0x08 : 0x00);
	
	// FPS counter color
	for (unsigned char i = 0; i < 4; i++)
	{
		if (gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(optOSD_Color[0][i])))
		{
			curFPSStyle &= ~0x06;
			curFPSStyle |= (i << 1);
			break;
		}
	}
	
	draw->setFPSStyle(curFPSStyle);
	
	// Message
	draw->setMsgEnabled(gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(chkOSD_Enable[1])));
	
	unsigned char curMsgStyle = draw->msgStyle() & ~0x18;
	curMsgStyle |= (gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(chkOSD_DoubleSized[1])) ? 0x10 : 0x00);
	curMsgStyle |= (gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(chkOSD_Transparency[1])) ? 0x08 : 0x00);
	
	// Message color
	for (unsigned char i = 0; i < 4; i++)
	{
		if (gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(optOSD_Color[1][i])))
		{
			curMsgStyle &= ~0x06;
			curMsgStyle |= (i << 1);
			break;
		}
	}
	
	draw->setMsgStyle(curMsgStyle);
	
	// Intro effect color
	for (unsigned char i = 0; i < 8; i++)
	{
		if (gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(optIntroEffectColor[i])))
		{
			draw->setIntroEffectColor((unsigned char)i);
			break;
		}
	}
}
