/***************************************************************************
 * Gens: (Win32) Window base class.                                        *
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
#include <windows.h>
#include <windowsx.h>
#include <commctrl.h>

#include "emulator/g_main.hpp"

#include "ui/win32/resource.h"


void WndBase::setFocus(void)
{
	// TODO
}


void WndBase::setModal(void *parent)
{
	// TODO
}


static inline uint8_t numOneBits(uint32_t number)
{
	uint8_t num = 0;
	for (uint8_t i = 0; i < 32; i++)
	{
		num += (number & 0x01);
		number >>= 1;
	}
	return num;
}


/**
 * addDialogButtons(): Add dialog buttons to the window.
 * @param container Container to add the buttons to.
 * @param alignment Button alignment.
 * @param buttons Bitfield containing the buttons to add.
 * @param buttonFocus Button to set focus to, if any.
 */
void WndBase::addDialogButtons(void *container, ButtonAlignment alignment,
			       uint32_t buttons, uint32_t buttonFocus)
{
	RECT r;
	GetWindowRect((HWND)container, &r);
	
	const int w = r.right - r.left;
	const int h = r.bottom - r.top;
	
	const unsigned short btnWidth = 75;
	const unsigned short btnHeight = 23;
	
	const uint8_t totalBtns = numOneBits(buttons);
	const unsigned short totalBtnWidth = (btnWidth * totalBtns) + (8 * (totalBtns - 1));
	
	const int btnTop = h - (btnHeight + 8);
	int btnLeft;
	
	switch (alignment)
	{
		case WndBase::BAlign_Left:
			btnLeft = 8;
			break;
		case WndBase::BAlign_Right:
			btnLeft = w - (8 + totalBtnWidth);
			break;
		case WndBase::BAlign_Center:
		case WndBase::BAlign_Default:
		default:
			btnLeft = (w - totalBtnWidth) / 2;
			break;
	}
	
	if (buttons & (BUTTON_SAVE | BUTTON_OK))
	{
		// TODO: Add "Save" button.
		// It isn't currently used on Win32, so I'm not implementing it right now
		
		HWND btnOK;
		btnOK = CreateWindow(WC_BUTTON, "&OK", WS_CHILD | WS_VISIBLE | WS_TABSTOP | BS_DEFPUSHBUTTON,
				     btnLeft, btnTop, btnWidth, btnHeight,
				     (HWND)container, (HMENU)IDC_BTN_OK, ghInstance, NULL);
		SetWindowFont(btnOK, fntMain, TRUE);
		
		if (buttonFocus & (BUTTON_SAVE | BUTTON_OK))
			SetFocus(btnOK);
		
		btnLeft += (btnWidth + 8);
	}
	
	if (buttons & BUTTON_APPLY)
	{
		HWND btnApply;
		btnApply = CreateWindow(WC_BUTTON, "&Apply", WS_CHILD | WS_VISIBLE | WS_TABSTOP,
					btnLeft, btnTop, btnWidth, btnHeight,
					(HWND)container, (HMENU)IDC_BTN_APPLY, ghInstance, NULL);
		SetWindowFont(btnApply, fntMain, TRUE);
		
		if (buttonFocus & BUTTON_APPLY)
			SetFocus(btnApply);
		
		btnLeft += (btnWidth + 8);
	}
	
	if (buttons & BUTTON_CANCEL)
	{
		HWND btnCancel;
		btnCancel = CreateWindow(WC_BUTTON, "&Cancel", WS_CHILD | WS_VISIBLE | WS_TABSTOP,
					 btnLeft, btnTop, btnWidth, btnHeight,
					 (HWND)container, (HMENU)IDC_BTN_CANCEL, ghInstance, NULL);
		SetWindowFont(btnCancel, fntMain, TRUE);
		
		if (buttonFocus & BUTTON_CANCEL)
			SetFocus(btnCancel);
		
		btnLeft += (btnWidth + 8);
	}
}
