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

#ifndef GENS_GTK_GENS_WINDOW_HPP
#define GENS_GTK_GENS_WINDOW_HPP

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#ifdef __cplusplus
extern "C" {
#endif

#include <gtk/gtk.h>
#include <stdint.h>

GtkWidget* create_gens_window(void); 
extern GtkWidget *gens_window;
extern int do_callbacks;

// New menu handler.
void GensWindow_GTK_MenuItemCallback(GtkMenuItem *menuitem, gpointer user_data);
GtkWidget* findMenuItem(uint16_t id);

#ifdef __cplusplus
}
#endif

#ifdef __cplusplus

// Hash table containing all the menu items.
// Key is the menu ID.
#include "macros/hashtable.hpp"
#include <utility>
typedef GENS_HASHTABLE<uint16_t, GtkWidget*> gtkMenuMap;
extern gtkMenuMap gensMenuMap;
typedef std::pair<uint16_t, GtkWidget*> gtkMenuMapItem;

#endif /* __cplusplus */

#endif /* GENS_GTK_GENS_WINDOW_HPP */
