/***************************************************************************
 * Gens: (GTK+) Main Window.                                               *
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

#include "gens_window.hpp"
#include "gens_window_callbacks.hpp"
#include "gens_window_sync.hpp"

#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <string.h>
#include <stdio.h>

#include <gtk/gtk.h>
#include <gdk/gdkkeysyms.h>

// Gens GTK+ miscellaneous functions
#include "gtk-misc.h"

#include "emulator/g_main.hpp"

#ifdef GENS_DEBUGGER
#include "debugger/debugger.hpp"
#endif /* GENS_DEBUGGER */

GtkWidget *gens_window;
GtkWidget *MenuBar;


GtkAccelGroup *accel_group;

static void create_gens_window_menubar(GtkWidget *container);

// New menu handler.
#include "ui/common/gens/gens_menu.h"
#include "ui/common/gens/gens_menu_callbacks.hpp"
static void GTK_ParseMenu(GensMenuItem_t *menu, GtkWidget *container);

// Hash table containing all the menu items.
// Key is the menu ID.
gtkMenuMap gensMenuMap;

// Menu icons.
// See ui/common/gens/gens_menu.h:IDIM_* for the icon constants.
#include <utility>
using std::pair;

// GTK_MenuIcon_t:
// - first: bool - true if stock icon; false if filename.
// - second: stock icon name or filename, depending on first.
typedef pair<bool, const char*> GTK_MenuIcon_t;
static GTK_MenuIcon_t GTK_MenuIcons[] =
{
	GTK_MenuIcon_t(false, NULL),	// IDIM_MENU_ICON == 0x0100 == not used
	GTK_MenuIcon_t(true,  "gtk-open"),
	GTK_MenuIcon_t(true,  "gtk-save"),
	GTK_MenuIcon_t(true,  "gtk-save-as"),
	GTK_MenuIcon_t(true,  "gtk-refresh"),
	GTK_MenuIcon_t(true,  "gtk-revert-to-saved"),
	GTK_MenuIcon_t(true,  "gtk-close"),
	GTK_MenuIcon_t(true,  "gtk-quit"),
	GTK_MenuIcon_t(true,  "gtk-cdrom"),
	GTK_MenuIcon_t(true,  "gtk-cdrom"),
	GTK_MenuIcon_t(false, "modem.png"),
	GTK_MenuIcon_t(false, "chronometer.png"),
	GTK_MenuIcon_t(false, "dialog-password.png"),
	GTK_MenuIcon_t(true,  "gtk-fullscreen"),
	GTK_MenuIcon_t(true,  "gtk-select-color"),
	GTK_MenuIcon_t(false, "viewmag.png"),
	GTK_MenuIcon_t(false, "2rightarrow.png"),
	GTK_MenuIcon_t(true,  "gtk-copy"),
	GTK_MenuIcon_t(true,  "gtk-refresh"),
	GTK_MenuIcon_t(false, "preferences-system.png"),
	GTK_MenuIcon_t(false, "preferences-desktop-gaming.png"),
	GTK_MenuIcon_t(false, "document-open-folder.png"),
	GTK_MenuIcon_t(false, "binary.png"),
	GTK_MenuIcon_t(false, "memory.png"),
	GTK_MenuIcon_t(true,  "gtk-help"),
	GTK_MenuIcon_t(false, NULL),	// End of array.
};


// Set to 0 to temporarily disable callbacks.
int do_callbacks = 1;


GtkWidget* create_gens_window(void)
{
	GdkPixbuf *gens_window_icon_pixbuf;
	GtkWidget *vbox1;
	GtkWidget *sdlsock;
	
	accel_group = gtk_accel_group_new();
	
	// Create the Gens window.
	CREATE_GTK_WINDOW(gens_window, "gens_window", "Gens",
			  gens_window_icon_pixbuf, "Gens2.ico");
	gtk_window_set_resizable(GTK_WINDOW(gens_window), FALSE);
	gtk_container_set_border_width(GTK_CONTAINER(gens_window), 0);
	
	// Layout objects.
	vbox1 = gtk_vbox_new(FALSE, 0);
	gtk_widget_set_name(vbox1, "vbox1");
	gtk_widget_show(vbox1);
	gtk_container_add(GTK_CONTAINER(gens_window), vbox1);
	GLADE_HOOKUP_OBJECT(gens_window, vbox1, "vbox1");
	
	// Create the menu bar.
	create_gens_window_menubar(vbox1);
	
	// Create the SDL socket.
	sdlsock = gtk_event_box_new();
	gtk_widget_set_name(sdlsock, "sdlsock");
	gtk_box_pack_end(GTK_BOX(lookup_widget(gens_window, "vbox1")), sdlsock, 0, 0, 0);
	GLADE_HOOKUP_OBJECT(gens_window, sdlsock, "sdlsock");
	
	// Add the accel group.
	gtk_window_add_accel_group(GTK_WINDOW(gens_window), accel_group);
	
	// Callbacks for if the window is closed.
	g_signal_connect((gpointer)gens_window, "delete-event",
			 G_CALLBACK(on_gens_window_close), NULL);
	g_signal_connect((gpointer)gens_window, "destroy-event",
			 G_CALLBACK(on_gens_window_close), NULL);
	
	// Callbacks for Auto Pause.
	g_signal_connect((gpointer)gens_window, "focus-in-event",
			 G_CALLBACK(gens_window_focus_in), NULL);
	g_signal_connect((gpointer)gens_window, "focus-out-event",
			 G_CALLBACK(gens_window_focus_out), NULL);
	
	// Enable drag & drop for ROM loading.
	const GtkTargetEntry target_list[] =
	{
		{"text/plain", 0, 0},
		{"text/uri-list", 0, 1},
	};
	
	gtk_drag_dest_set
	(
		sdlsock,
		(GtkDestDefaults)(GTK_DEST_DEFAULT_MOTION | GTK_DEST_DEFAULT_HIGHLIGHT),
		target_list,
		G_N_ELEMENTS(target_list),
		(GdkDragAction)(GDK_ACTION_COPY | GDK_ACTION_MOVE | GDK_ACTION_LINK | GDK_ACTION_PRIVATE | GDK_ACTION_ASK)
	);
	
	// Set drag & drop callbacks.
	g_signal_connect(sdlsock, "drag-data-received",
			 G_CALLBACK(gens_window_drag_data_received), NULL);
	g_signal_connect(sdlsock, "drag-drop",
			 G_CALLBACK(gens_window_drag_drop), NULL);
	
	return gens_window;
}


/**
 * create_gens_window_menubar(): Create the menu bar.
 * @param container Container for the menu bar.
 */
static void create_gens_window_menubar(GtkWidget *container)
{
	MenuBar = gtk_menu_bar_new();
	gtk_widget_set_name(MenuBar, "MenuBar");
	gtk_widget_show(MenuBar);
	gtk_container_add(GTK_CONTAINER(container), MenuBar);
	
	// Menus
	GTK_ParseMenu(&gmiMain[0], MenuBar);
}


/**
 * GTK_ParseMenu(): Parse the menu structs.
 * @param menu First item of the array of menu structs to parse.
 * @param container Container to add the menu items to.
 */
static void GTK_ParseMenu(GensMenuItem_t *menu, GtkWidget *container)
{
	GtkWidget *mnuItem, *subMenu;
	GtkWidget *icon;
	char widgetName[64];
	char *sMenuText, *mnemonicPos;
	bool bMenuTextSet, bMenuHasIcon;
	GSList *radioGroup = NULL;
	bool bSetCallbackHandler;
	
	while (menu->id != 0)
	{
		// Convert the Win32/Qt mnemonic symbol ("&") to the GTK+ mnemonic symbol ("_").
		if (menu->text)
		{
			// Menu text specified.
			bMenuTextSet = true;
			sMenuText = strdup(menu->text);
			mnemonicPos = strchr(sMenuText, '&');
			if (mnemonicPos)
				*mnemonicPos = '_';
		}
		else
		{
			// No menu text.
			sMenuText = "";
			bMenuTextSet = false;
		}
		
		// TODO: Radio/Check support.
		bMenuHasIcon = false;
		switch ((menu->flags & GMF_ITEM_MASK))
		{
			case GMF_ITEM_SEPARATOR:
				// Separator.
				mnuItem = gtk_separator_menu_item_new();
				gtk_widget_set_sensitive(mnuItem, FALSE);
				radioGroup = NULL;
				bSetCallbackHandler = false;
				break;
			
			case GMF_ITEM_CHECK:
				// Check menu item.
				mnuItem = gtk_check_menu_item_new_with_mnemonic(sMenuText);
				radioGroup = NULL;
				bSetCallbackHandler = true;
				break;
			
			case GMF_ITEM_RADIO:
				// Radio menu item.
				mnuItem = gtk_radio_menu_item_new_with_mnemonic(radioGroup, sMenuText);
				radioGroup = gtk_radio_menu_item_get_group(GTK_RADIO_MENU_ITEM(mnuItem));
				bSetCallbackHandler = true;
				break;
				
			default:
				// Not a special menu item.
				radioGroup = NULL;
				
				// If this isn't a submenu, set the callback handler.
				bSetCallbackHandler = ((menu->flags & GMF_ITEM_MASK) != GMF_ITEM_SUBMENU);
				
				// Check if an icon was specified.
				if (menu->icon > 0)
				{
					bMenuHasIcon = true;
					mnuItem = gtk_image_menu_item_new_with_mnemonic(sMenuText);
				}
				else
				{
					mnuItem = gtk_menu_item_new_with_mnemonic(sMenuText);
				}
				
				break;
		}
		
		if (bMenuTextSet)
			free(sMenuText);
		
		sprintf(widgetName, "mnu_0x%08X_0x%04X", (unsigned int)menu, menu->id);
		gtk_widget_set_name(mnuItem, widgetName);
		gtk_widget_show(mnuItem);
		gtk_container_add(GTK_CONTAINER(container), mnuItem);
		
		g_object_set_data_full(G_OBJECT(gens_window), widgetName,
				       g_object_ref(mnuItem),
				       (GDestroyNotify)g_object_unref);
		
		// Check if an icon is specified.
		if (bMenuHasIcon)
		{
			// Icon specified.
			const char* iconName = NULL;
			if (menu->icon > IDIM_MENU_ICON && menu->icon <= IDIM_MENU_ICON_MAX)
			{
				// Valid icon.
				iconName = GTK_MenuIcons[menu->icon - IDIM_MENU_ICON].second;
			}
			
			icon = NULL;
			if (iconName)
			{
				if (GTK_MenuIcons[menu->icon - IDIM_MENU_ICON].first)
				{
					// GTK+ stock icon.
					icon = gtk_image_new_from_stock(iconName, GTK_ICON_SIZE_MENU);
					if (!icon)
					{
						// Icon not found.
						fprintf(stderr, "%s: GTK+ stock icon not found: %s\n", __func__, iconName);
					}
				}
				else
				{
					// Load an icon from a file.
					icon = create_pixmap(iconName);
					if (!icon)
					{
						// Icon not found.
						fprintf(stderr, "%s: Icon file not found: %s\n", __func__, iconName);
					}
				}
			}
			
			if (icon)
			{
				// Icon loaded.
				sprintf(widgetName, "mnu_icon_0x%08X_0x%04X", (unsigned int)menu, menu->id);
				gtk_widget_set_name(icon, widgetName);
				gtk_widget_show(icon);
				gtk_image_menu_item_set_image(GTK_IMAGE_MENU_ITEM(mnuItem), icon);
				
				g_object_set_data_full(G_OBJECT(gens_window), widgetName,
						       g_object_ref(icon),
						       (GDestroyNotify)g_object_unref);
			}
		}
		
		// Check for an accelerator.
		if (menu->accelKey != 0)
		{
			// Accelerator specified.
			int accelModifier = 0;
			guint accelKey;
			
			// Determine the modifier.
			if (menu->accelModifier & GMAM_CTRL)
				accelModifier |= GDK_CONTROL_MASK;
			if (menu->accelModifier & GMAM_ALT)
				accelModifier |= GDK_MOD1_MASK;
			if (menu->accelModifier & GMAM_SHIFT)
				accelModifier |= GDK_SHIFT_MASK;
			
			// Determine the key.
			// TODO: Add more special keys.
			switch (menu->accelKey)
			{
				case GMAK_BACKSPACE:
					accelKey = GDK_BackSpace;
					break;
				
				case GMAK_ENTER:
					accelKey = GDK_Return;
					break;
				
				case GMAK_TAB:
					accelKey = GDK_Tab;
					break;
				
				case GMAK_F1: case GMAK_F2:  case GMAK_F3:  case GMAK_F4:
				case GMAK_F5: case GMAK_F6:  case GMAK_F7:  case GMAK_F8:
				case GMAK_F9: case GMAK_F10: case GMAK_F11: case GMAK_F12:
					accelKey = (menu->accelKey - GMAK_F1) + GDK_F1;
					break;
					
				default:
					accelKey = menu->accelKey;
					break;
			}
			
			// Add the accelerator.
			gtk_widget_add_accelerator(mnuItem, "activate", accel_group,
						   accelKey, (GdkModifierType)accelModifier,
						   GTK_ACCEL_VISIBLE);
		}
		
		// Check for a submenu.
		if (((menu->flags & GMF_ITEM_MASK) == GMF_ITEM_SUBMENU) && (menu->submenu))
		{
			// Submenu.
			subMenu = gtk_menu_new();
			
			sprintf(widgetName, "mnu_sub_0x%08X_0x%04X", (unsigned int)menu, menu->id);
			gtk_widget_set_name(subMenu, widgetName);
			gtk_menu_item_set_submenu(GTK_MENU_ITEM(mnuItem), subMenu);
			
			g_object_set_data_full(G_OBJECT(gens_window), widgetName,
					       g_object_ref(subMenu),
					       (GDestroyNotify)g_object_unref);
			
			// Parse the submenu.
			GTK_ParseMenu(menu->submenu, subMenu);
		}
		
		if (bSetCallbackHandler)
		{
			// Set the callback handler.
			g_signal_connect((gpointer)mnuItem, "activate",
					 G_CALLBACK(GensWindow_GTK_MenuItemCallback), GINT_TO_POINTER(menu->id));
		}
		
		// Add the menu to the menu map. (Exception is if id is 0 or IDM_SEPARATOR.)
		if (menu->id != 0 && menu->id != IDM_SEPARATOR)
		{
			gensMenuMap.insert(gtkMenuMapItem(menu->id, mnuItem));
		}
		
		// Next menu item.
		menu++;
	}
}


/**
 * GensWindow_GTK_MenuItemCallback(): Menu item callback.
 * @param menuitem Menu item widget.
 * @param user_data Menu item ID.
 */
void GensWindow_GTK_MenuItemCallback(GtkMenuItem *menuitem, gpointer user_data)
{
	if (!do_callbacks)
		return;
	
	bool state = false;
	
	if (GTK_IS_RADIO_MENU_ITEM(menuitem))
	{
		// Radio menu items should only trigger the callback if they're selected.
		if (!gtk_check_menu_item_get_active(GTK_CHECK_MENU_ITEM(menuitem)))
			return;
		state = true;
	}
	else if (GTK_IS_CHECK_MENU_ITEM(menuitem))
	{
		// Check menu items automatically toggle, so the state value should be
		// the opposite value of its current state.
		state = !gtk_check_menu_item_get_active(GTK_CHECK_MENU_ITEM(menuitem));
	}
	
	// Run the callback function.
	GensWindow_MenuItemCallback((uint16_t)(GPOINTER_TO_INT(user_data)), state);
};


/**
 * findMenuItem(): Find a menu item in the menu map.
 * @param id Menu item ID.
 * @return Menu item.
 */
GtkWidget* findMenuItem(uint16_t id)
{
	// TODO: Make this a common function.
	
	gtkMenuMap::iterator mnuIter;
	
	mnuIter = gensMenuMap.find(id);
	if (mnuIter == gensMenuMap.end())
		return NULL;
	
	return (*mnuIter).second;
}
