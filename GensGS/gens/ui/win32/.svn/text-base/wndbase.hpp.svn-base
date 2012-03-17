/***************************************************************************
 * Gens: Window base class.                                                *
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

#ifndef GENS_WIN32_WNDBASE_HPP
#define GENS_WIN32_WNDBASE_HPP

#include <stdint.h>
#include <unistd.h>

#include <windows.h>

#ifdef __cplusplus

class WndBase
{
	public:
		void setFocus(void);
		void setModal(void *parent);
		
		// Button constants
		static const uint32_t BUTTON_OK		= (1 << 0);
		static const uint32_t BUTTON_SAVE	= (1 << 1);
		static const uint32_t BUTTON_APPLY	= (1 << 2);
		static const uint32_t BUTTON_CANCEL	= (1 << 3);
		static const uint32_t BUTTON_ALL	= ~0;
		
		// GTK+ only.
		virtual void dlgButtonPress(uint32_t button) { };
		
	protected:
		WndBase() { }
		~WndBase() { }
		
		enum ButtonAlignment
		{
			BAlign_Default = 0,
			BAlign_Left = 1,
			BAlign_Center = 2,
			BAlign_Right = 3,
		};
		
		void addDialogButtons(void *container, ButtonAlignment alignment,
				      uint32_t buttons, uint32_t buttonFocus = 0);
		
		HWND m_Window;
		HACCEL m_AccelTable;
};

#endif /* __cplusplus */

#endif /* GENS_WIN32_WNDBASE_HPP */
