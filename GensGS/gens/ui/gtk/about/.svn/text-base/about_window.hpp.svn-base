/***************************************************************************
 * Gens: (GTK+) About Window.                                              *
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

#ifndef GENS_GTK_ABOUT_WINDOW_HPP
#define GENS_GTK_ABOUT_WINDOW_HPP

#ifdef __cplusplus

#include "wndbase.hpp"

class AboutWindow : public WndBase
{
	public:
		static AboutWindow* Instance(GtkWindow *parent = NULL);
		
		gboolean close(void);
		gboolean iceTime(void);
	
	protected:
		AboutWindow();
		~AboutWindow();
		
		static AboutWindow* m_Instance;
		
		void dlgButtonPress(uint32_t button);
		
		// Static functions required for GTK+ callbacks.
		static gboolean GTK_Close(GtkWidget *widget, GdkEvent *event, gpointer user_data);
		static gboolean GTK_iceTime(gpointer user_data);
		
		GtkWidget *m_imgGensLogo;
		GdkPixbuf *m_pbufIce;
		unsigned short ax, bx, cx;
		
		void updateIce(void);
		
		// Strings
		static const char* StrTitle;
		static const char* StrDescription;
		static const char* StrCopyright;
		
		// Data
		static const unsigned char Data[];
		static const unsigned char DX[];
};

#endif

#endif /* GENS_GTK_ABOUT_WINDOW_HPP */
