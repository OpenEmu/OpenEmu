/***************************************************************************
 * Gens: (GTK+) Window base class.                                         *
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

#include "wndbase.hpp"
#include <gtk/gtk.h>
#include <gdk/gdkkeysyms.h>


void WndBase::setFocus(void)
{
	if (m_Window)
		gtk_widget_grab_focus(m_Window);
}


void WndBase::setModal(GtkWindow *parent)
{
	if (m_Window)
		gtk_window_set_transient_for(GTK_WINDOW(m_Window), parent);
}


void WndBase::setVisible(bool newVisible)
{
	if (newVisible)
		gtk_widget_show_all(m_Window);
	else
		gtk_widget_hide_all(m_Window);
}


static inline void WndBase_GTK_AddBtnToContainer(void *container, GtkWidget *button,
						 GtkResponseType responseID)
{
	if (GTK_IS_DIALOG(container))
	{
		gtk_dialog_add_action_widget(GTK_DIALOG(container), button, responseID);
	}
	else if (GTK_IS_BOX(container))
	{
		gtk_box_pack_start(GTK_BOX(container), button, FALSE, FALSE, 0);
	}
	else if (GTK_IS_CONTAINER(container))
	{
		gtk_container_add(GTK_CONTAINER(container), button);
	}
}


#define WndBase_GTK_btnCallback(btnName, BUTTON_VALUE) \
static void WndBase_GTK_##btnName(GtkButton *button, gpointer user_data)	\
{										\
	reinterpret_cast<WndBase*>(user_data)->dlgButtonPress(BUTTON_VALUE);	\
}

WndBase_GTK_btnCallback(btnCancel, WndBase::BUTTON_CANCEL);
WndBase_GTK_btnCallback(btnApply, WndBase::BUTTON_APPLY);
WndBase_GTK_btnCallback(btnSave, WndBase::BUTTON_SAVE);
WndBase_GTK_btnCallback(btnOK, WndBase::BUTTON_OK);


/**
 * addDialogButtons(): Add dialog buttons to the window.
 * @param container Container to add the buttons to.
 * @param alignment Button alignment.
 * @param buttons Bitfield containing the buttons to add.
 * @param buttonFocus Button to set focus to, if any.
 * @param buttonAccel Bitfield containing which buttons should have accelerators.
 */
void WndBase::addDialogButtons(void *container, ButtonAlignment alignment,
			       uint32_t buttons, uint32_t buttonFocus,
			       uint32_t buttonAccel)
{
	if (buttons & BUTTON_CANCEL)
	{
		GtkWidget *btnCancel = gtk_button_new_from_stock("gtk-cancel");
		gtk_widget_set_name(btnCancel, "btnCancel");
		gtk_widget_show(btnCancel);
		WndBase_GTK_AddBtnToContainer(container, btnCancel, GTK_RESPONSE_CANCEL);
		
		g_object_set_data_full(G_OBJECT(m_Window), "btnCancel",
				       g_object_ref(btnCancel), (GDestroyNotify)g_object_unref);
		
		g_signal_connect((gpointer)btnCancel, "clicked",
				  G_CALLBACK(WndBase_GTK_btnCancel), this);
		
		// Accelerator
		if (buttonAccel & BUTTON_CANCEL)
		{
			gtk_widget_add_accelerator(btnCancel, "activate", m_AccelTable,
						   GDK_Escape, (GdkModifierType)(0), (GtkAccelFlags)(0));
		}
		
		if (buttonFocus & BUTTON_CANCEL)
			gtk_widget_grab_focus(btnCancel);
	}

	if (buttons & BUTTON_APPLY)
	{
		GtkWidget *btnApply = gtk_button_new_from_stock("gtk-apply");
		gtk_widget_set_name(btnApply, "btnApply");
		gtk_widget_show(btnApply);
		WndBase_GTK_AddBtnToContainer(container, btnApply, GTK_RESPONSE_APPLY);
		
		g_signal_connect((gpointer)btnApply, "clicked",
				  G_CALLBACK(WndBase_GTK_btnApply), this);
		
		g_object_set_data_full(G_OBJECT(m_Window), "btnApply",
				       g_object_ref(btnApply), (GDestroyNotify)g_object_unref);
		
		if (buttonFocus & BUTTON_APPLY)
			gtk_widget_grab_focus(btnApply);
	}
	
	if (buttons & (BUTTON_SAVE | BUTTON_OK))
	{
		GtkWidget *btnOK;
		
		if (buttons & BUTTON_SAVE)
		{
			btnOK = gtk_button_new_from_stock("gtk-save");
			gtk_widget_set_name(btnOK, "btnSave");
		}
		else //if (buttons & BUTTON_OK)
		{
			btnOK = gtk_button_new_from_stock("gtk-ok");
			gtk_widget_set_name(btnOK, "btnOK");
		}
		
		gtk_widget_show(btnOK);
		WndBase_GTK_AddBtnToContainer(container, btnOK, GTK_RESPONSE_OK);
		
		if (buttons & BUTTON_SAVE)
		{
			g_signal_connect((gpointer)btnOK, "clicked",
					  G_CALLBACK(WndBase_GTK_btnSave), this);
			
			g_object_set_data_full(G_OBJECT(m_Window), "btnSave",
					       g_object_ref(btnOK), (GDestroyNotify)g_object_unref);
		}
		else
		{
			g_signal_connect((gpointer)btnOK, "clicked",
					  G_CALLBACK(WndBase_GTK_btnOK), this);
			
			g_object_set_data_full(G_OBJECT(m_Window), "btnCancel",
					       g_object_ref(btnOK), (GDestroyNotify)g_object_unref);
		}
		
		// Accelerators
		if (buttonAccel & (BUTTON_SAVE | BUTTON_OK))
		{
			gtk_widget_add_accelerator(btnOK, "activate", m_AccelTable,
						   GDK_Return, (GdkModifierType)(0), (GtkAccelFlags)(0));
			gtk_widget_add_accelerator(btnOK, "activate", m_AccelTable,
						   GDK_KP_Enter, (GdkModifierType)(0), (GtkAccelFlags)(0));
		}
		
		if (buttonFocus & (BUTTON_SAVE | BUTTON_OK))
			gtk_widget_grab_focus(btnOK);
	}
}
