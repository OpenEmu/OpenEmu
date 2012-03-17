/***************************************************************************
 * Gens: (Win32) General Options Window.                                   *
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

#ifndef GENS_WIN32_GENERAL_OPTIONS_WINDOW_HPP
#define GENS_WIN32_GENERAL_OPTIONS_WINDOW_HPP

#ifdef __cplusplus

#include "wndbase.hpp"
#include <windows.h>

class GeneralOptionsWindow : public WndBase
{
	public:
		static GeneralOptionsWindow* Instance(HWND parent = NULL);
		static bool isOpen(void) { return (m_Instance != NULL); }
		
		bool isDialogMessage(MSG *msg) { return IsDialogMessage((HWND)m_Window, msg); }
	
	protected:
		GeneralOptionsWindow();
		~GeneralOptionsWindow();
		
		static GeneralOptionsWindow* m_Instance;
		
		static LRESULT CALLBACK WndProc_STATIC(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);
		LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);
		
		bool m_childWindowsCreated;
		void createChildWindows(HWND hWnd);
		
		// Custom radio button drawing functions.
		void drawMsgRadioButton(int identifier, LPDRAWITEMSTRUCT lpDrawItem);
		void selectRadioButton(int identifier);
		
		// Settings functions.
		void load(void);
		void save(void);
		
		// OSD items. [0 == FPS; 1 == MSG]
		HWND chkOSD_Enable[2];
		HWND chkOSD_DoubleSized[2];
		HWND chkOSD_Transparency[2];
		HWND optOSD_Color[2][4];
		
		void createOSDFrame(HWND hWnd, const int index,
				    const short frameLeft,
				    const short frameTop,
				    const short frameWidth,
				    const short frameHeight,
				    const char* title);
		
		// Intro Effect color buttons.
		HWND optIntroEffectColor[8];
		
		// Color states.
		unsigned short state_optColor[3];
		
		// Miscellaneous settings.
		HWND chkMisc_AutoFixChecksum;
		HWND chkMisc_AutoPause;
		HWND chkMisc_FastBlur;
		HWND chkMisc_SegaCDLEDs;
		HWND chkMisc_BorderColorEmulation;
		HWND chkMisc_PauseTint;
		
		// GDI objects
		HBRUSH brushOSD[4][2];
		HPEN penOSD[4][2];
		HBRUSH brushIntroEffect[8][2];
		HPEN penIntroEffect[8][2];
		
		// Color data
		static const COLORREF Colors_OSD[4][2];
		static const COLORREF Colors_IntroEffect[8][2];
};
#endif /* __cplusplus */

#endif /* GENS_WIN32_GENERAL_OPTIONS_WINDOW_HPP */
