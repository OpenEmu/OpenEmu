/***************************************************************************
 * Gens: (Win32) About Window.                                             *
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

#ifndef GENS_WIN32_ABOUT_WINDOW_HPP
#define GENS_WIN32_ABOUT_WINDOW_HPP

#ifdef __cplusplus

#include "wndbase.hpp"
#include <windows.h>

class AboutWindow : WndBase
{
	public:
		static AboutWindow* Instance(HWND parent = NULL);
		static bool isOpen(void) { return (m_Instance != NULL); }
		
		bool isDialogMessage(MSG *msg) { return IsDialogMessage((HWND)m_Window, msg); }
	
	protected:
		AboutWindow();
		~AboutWindow();
		
		static AboutWindow* m_Instance;
		
		static LRESULT CALLBACK WndProc_STATIC(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);
		LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);
		
		bool m_childWindowsCreated;
		void createChildWindows(HWND hWnd);
		
		unsigned int iceLastTicks;
		UINT_PTR tmrIce;
		
		static void iceTime_STATIC(HWND hWnd, UINT uMsg, UINT_PTR idEvent, DWORD dwTime);
		void iceTime(HWND hWnd, UINT uMsg, UINT_PTR idEvent, DWORD dwTime);
		void updateIce(void);
		
		// Labels and boxes
		HWND lblGensTitle;
		HWND lblGensDesc;
		HWND imgGensLogo;
		
		// Gens logo
		HWND m_imgGensLogo;
		HBITMAP bmpGensLogo;
		
		unsigned short ax, bx, cx;
		
		// Strings
		static const char* StrTitle;
		static const char* StrDescription;
		static const char* StrCopyright;
		
		// Data
		static const unsigned char Data[];
		static const unsigned char DX[];

};

#endif /* __cplusplus */

#endif /* GENS_WIN32_ABOUT_WINDOW_HPP */
