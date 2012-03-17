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

#ifndef GENS_GTK_GENERAL_OPTIONS_WINDOW_HPP
#define GENS_GTK_GENERAL_OPTIONS_WINDOW_HPP

#ifdef __cplusplus

#include "wndbase.hpp"

class GeneralOptionsWindow : public WndBase
{
	public:
		static GeneralOptionsWindow* Instance(GtkWindow *parent = NULL);
		
		gboolean close(void);
	
	protected:
		GeneralOptionsWindow();
		~GeneralOptionsWindow();
		
		static GeneralOptionsWindow* m_Instance;
		
		void dlgButtonPress(uint32_t button);
		
		void createColorRadioButtons(const char* title,
					     const char* groupName, 
					     const char* colors[][6],
					     const int buttonSet,
					     GtkWidget* container);
		
		// Settings functions.
		void load(void);
		void save(void);
		
		// OSD items. [0 == FPS; 1 == MSG]
		GtkWidget *chkOSD_Enable[2];
		GtkWidget *chkOSD_DoubleSized[2];
		GtkWidget *chkOSD_Transparency[2];
		GtkWidget *optOSD_Color[2][4];
		
		void createOSDFrame(const char* title, const int index, GtkWidget *container);
		
		// Intro Effect color buttons.
		GtkWidget *optIntroEffectColor[8];
		
		// Miscellaneous settings.
		GtkWidget *chkMisc_AutoFixChecksum;
		GtkWidget *chkMisc_AutoPause;
		GtkWidget *chkMisc_FastBlur;
		GtkWidget *chkMisc_SegaCDLEDs;
		GtkWidget *chkMisc_BorderColorEmulation;
		GtkWidget *chkMisc_PauseTint;
		
		// Static functions required for GTK+ callbacks.
		static gboolean GTK_Close(GtkWidget *widget, GdkEvent *event, gpointer user_data);
		
		// Color data
		static const char* Colors_OSD[5][6];
		static const char* Colors_IntroEffect[9][6];
};

#endif /* __cplusplus */

#endif /* GENS_GTK_GENERAL_OPTIONS_WINDOW_HPP */
