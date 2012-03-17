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

#include "about_window.hpp"
#include "gens/gens_window.hpp"

#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <string.h>
#include <stdio.h>

// git version
#include "macros/git.h"

#include "emulator/g_main.hpp"

#include "ui/win32/charset.hpp"
#include "ui/win32/resource.h"

#include <windowsx.h>
#include <commctrl.h>

#define ID_TIMER_ICE 0x1234

#ifdef GENS_GIT_VERSION
static const unsigned short lblTitle_HeightInc = 16;
#else
static const unsigned short lblTitle_HeightInc = 0;
#endif /* GENS_GIT_VERSION */

static const unsigned short iceOffsetX = 32;
static const unsigned short iceOffsetY = 8;


static WNDCLASS m_WndClass;


AboutWindow* AboutWindow::m_Instance = NULL;
AboutWindow* AboutWindow::Instance(HWND parent)
{
	if (m_Instance == NULL)
	{
		// Instance is deleted. Initialize the About window.
		m_Instance = new AboutWindow();
	}
	else
	{
		// Instance already exists. Set focus.
		m_Instance->setFocus();
	}
	
	// Set modality of the window.
	m_Instance->setModal(parent);
	
	return m_Instance;
}


/**
 * AboutWindow(): Create the About Window.
 */
AboutWindow::AboutWindow()
{
	tmrIce = NULL;
	bmpGensLogo = NULL;
	m_childWindowsCreated = false;
	
	// Create the window class.
	if (m_WndClass.lpfnWndProc != WndProc_STATIC)
	{
		m_WndClass.style = CS_OWNDC | CS_HREDRAW | CS_VREDRAW;
		m_WndClass.lpfnWndProc = WndProc_STATIC;
		m_WndClass.cbClsExtra = 0;
		m_WndClass.cbWndExtra = 0;
		m_WndClass.hInstance = ghInstance;
		m_WndClass.hIcon = LoadIcon(ghInstance, MAKEINTRESOURCE(IDI_SONIC_HEAD));
		m_WndClass.hCursor = LoadCursor(NULL, IDC_ARROW);
		m_WndClass.hbrBackground = GetSysColorBrush(COLOR_3DFACE);
		m_WndClass.lpszMenuName = NULL;
		m_WndClass.lpszClassName = "Gens_About";
		
		RegisterClass(&m_WndClass);
	}
	
	// Messages are processed before the object is finished being created,
	// so this assignment is needed.
	m_Instance = this;
	
	// Create the window.
	// TODO: Don't hardcode the parent window.
	m_Window = CreateWindowEx(NULL, "Gens_About", "About Gens",
				  WS_DLGFRAME | WS_POPUP | WS_SYSMENU | WS_CAPTION,
				  CW_USEDEFAULT, CW_USEDEFAULT,
				  320, 320+lblTitle_HeightInc,
				  Gens_hWnd, NULL, ghInstance, NULL);
	
	// Set the actual window size.
	Win32_setActualWindowSize(m_Window, 320, 320+lblTitle_HeightInc);
	
	// Center the window on the Gens window.
	Win32_centerOnGensWindow(m_Window);
	
	UpdateWindow(m_Window);
	ShowWindow(m_Window, 1);
}


AboutWindow::~AboutWindow()
{
	if (bmpGensLogo)
		DeleteObject(bmpGensLogo);
	
	m_Instance = NULL;
}


LRESULT CALLBACK AboutWindow::WndProc_STATIC(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	return m_Instance->WndProc(hWnd, message, wParam, lParam);
}


/**
 * About_Window_WndProc(): The About window procedure.
 * @param hWnd hWnd of the object sending a message.
 * @param message Message being sent by the object.
 * @param wParam
 * @param lParam
 * @return
 */
LRESULT CALLBACK AboutWindow::WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	switch(message)
	{
		case WM_CREATE:
			if (!m_childWindowsCreated)
				createChildWindows(hWnd);
			break;
		
		case WM_CLOSE:
			DestroyWindow(m_Window);
			return 0;
		
		case WM_MENUSELECT:
		case WM_ENTERSIZEMOVE:
		case WM_NCLBUTTONDOWN:
		case WM_NCRBUTTONDOWN:
			// Prevent audio stuttering when one of the following events occurs:
			// - Menu is opened.
			// - Window is resized.
			// - Left/Right mouse button down on title bar.
			audio->clearSoundBuffer();
			break;
		
		case WM_PAINT:
			if (ice == 3)
				updateIce();
			break;
		
		case WM_CTLCOLORSTATIC:
			if (hWnd != m_Window)
				break;
			
			// Set the title and version labels to transparent.
			if ((HWND)lParam == lblGensTitle ||
			    (HWND)lParam == lblGensDesc ||
			    (HWND)lParam == imgGensLogo)
			{
				SetBkMode((HDC)wParam, TRANSPARENT);
				return (LRESULT)GetStockObject(NULL_BRUSH);
			}
			return TRUE;
			break;
		
		case WM_COMMAND:
			if (LOWORD(wParam) == IDC_BTN_OK || LOWORD(wParam) == IDOK ||
			    LOWORD(wParam) == IDC_BTN_CANCEL || LOWORD(wParam) == IDCANCEL)
			{
				DestroyWindow(m_Window);
			}
			
			break;
		
		case WM_DESTROY:
			if (hWnd != m_Window)
				break;
			
			if (tmrIce)
			{
				KillTimer(m_Window, tmrIce);
				tmrIce = 0;
			}
			
			delete this;
			break;
	}
	
	return DefWindowProc(hWnd, message, wParam, lParam);
}


void AboutWindow::createChildWindows(HWND hWnd)
{
	if (m_childWindowsCreated)
		return;
	
	cx = 0; iceLastTicks = 0;
	if (ice != 3)
	{
		// Gens logo
		imgGensLogo = CreateWindow("Static", NULL, WS_CHILD | WS_VISIBLE | SS_BITMAP,
					   12, 0, 128, 96, hWnd, NULL, ghInstance, NULL);
		bmpGensLogo = (HBITMAP)LoadImage(ghInstance, MAKEINTRESOURCE(IDB_GENS_LOGO_SMALL),
						 IMAGE_BITMAP, 0, 0,
						 LR_DEFAULTSIZE | LR_LOADMAP3DCOLORS | LR_LOADTRANSPARENT);
		SendMessage(imgGensLogo, STM_SETIMAGE, (WPARAM)IMAGE_BITMAP, (LPARAM)bmpGensLogo);
	}
	else
	{
		// "ice" timer
		ax = 0; bx = 0; cx = 1;
		tmrIce = SetTimer(hWnd, ID_TIMER_ICE, 10, (TIMERPROC)iceTime_STATIC);
		
		m_Window = hWnd;
		updateIce();
	}
	
	// Title and version information.
	lblGensTitle = CreateWindow(WC_STATIC, StrTitle, WS_CHILD | WS_VISIBLE | SS_CENTER,
				    128, 8, 184, 32+lblTitle_HeightInc,
				    hWnd, NULL, ghInstance, NULL);
	SetWindowFont(lblGensTitle, fntTitle, TRUE);
	
	lblGensDesc = CreateWindow(WC_STATIC, StrDescription, WS_CHILD | WS_VISIBLE | SS_CENTER,
				   128, 42+lblTitle_HeightInc, 184, 100,
				   hWnd, NULL, ghInstance, NULL);
	SetWindowFont(lblGensDesc, fntMain, TRUE);
	
	// Box for the copyright message.
	HWND grpGensCopyright;
	grpGensCopyright = CreateWindow(WC_BUTTON, NULL, WS_CHILD | WS_VISIBLE | BS_GROUPBOX,
					8, 88+lblTitle_HeightInc, 304, 192,
					hWnd, NULL, ghInstance, NULL);
	
	// Copyright message.
	HWND lblGensCopyright;
	string sCopyright = charset_utf8_to_cp1252(StrCopyright);
	lblGensCopyright = CreateWindow(WC_STATIC, sCopyright.c_str(), WS_CHILD | WS_VISIBLE | SS_LEFT,
					8, 16, 288, 168,
					grpGensCopyright, NULL, ghInstance, NULL);
	SetWindowFont(lblGensCopyright, fntMain, TRUE);
	
	// Add the OK button.
	addDialogButtons(hWnd, WndBase::BAlign_Right,
			 WndBase::BUTTON_OK, WndBase::BUTTON_OK);
	
	// Child windows created.
	m_childWindowsCreated = true;
}


void AboutWindow::updateIce(void)
{
	HDC hDC;
	PAINTSTRUCT ps;
	
	hDC = BeginPaint(m_Window, &ps);
	
	int x, y;
	const unsigned char *src = &Data[ax*01440];
	const unsigned char *src2 = &DX[bx*040];
	unsigned char px1, px2;
	
	int bgc = GetSysColor(COLOR_3DFACE);
	int pxc;
	
	for (y = 0; y < 0120; y += 2)
	{
		for (x = 0; x < 0120; x += 4)
		{
			px1 = (*src & 0360) >> 3;
			px2 = (*src & 0017) << 1;
			
			if (!px1)
			{
				pxc = bgc;
			}
			else
			{
				pxc = RGB((src2[px1 + 1] & 0017) << 4,
					  (src2[px1 + 1] & 0360),
					  (src2[px1 + 0] & 0017) << 4);
			}
			
			SetPixel(hDC, x + 0 + iceOffsetX, y + 0 + iceOffsetY, pxc);
			SetPixel(hDC, x + 1 + iceOffsetX, y + 0 + iceOffsetY, pxc);
			SetPixel(hDC, x + 0 + iceOffsetX, y + 1 + iceOffsetY, pxc);
			SetPixel(hDC, x + 1 + iceOffsetX, y + 1 + iceOffsetY, pxc);
			
			if (!px2)
			{
				pxc = bgc;
			}
			else
			{
				pxc = RGB((src2[px2 + 1] & 0017) << 4,
					  (src2[px2 + 1] & 0360),
					  (src2[px2 + 0] & 0017) << 4);
			}
			SetPixel(hDC, x + 2 + iceOffsetX, y + 0 + iceOffsetY, pxc);
			SetPixel(hDC, x + 3 + iceOffsetX, y + 0 + iceOffsetY, pxc);
			SetPixel(hDC, x + 2 + iceOffsetX, y + 1 + iceOffsetY, pxc);
			SetPixel(hDC, x + 3 + iceOffsetX, y + 1 + iceOffsetY, pxc);
			
			src++;
		}
	}
	
	EndPaint(m_Window, &ps);
}


void AboutWindow::iceTime_STATIC(HWND hWnd, UINT uMsg, UINT_PTR idEvent, DWORD dwTime)
{
	m_Instance->iceTime(hWnd, uMsg, idEvent, dwTime);
}


void AboutWindow::iceTime(HWND hWnd, UINT uMsg, UINT_PTR idEvent, DWORD dwTime)
{
	if (!(hWnd == m_Window && idEvent == ID_TIMER_ICE && ice == 3))
		return;
	
	if (iceLastTicks + 100 > dwTime)
		return;
	
	if (!cx)
		return;
	
	ax ^= 1;
	bx++;
	if (bx >= 10)
		bx = 0;
	
	// Force a repaint.
	RECT rIce;
	rIce.left = iceOffsetX;
	rIce.top = iceOffsetY;
	rIce.right = iceOffsetX + 80 - 1;
	rIce.bottom = iceOffsetY + 80 - 1;
	InvalidateRect(m_Window, &rIce, FALSE);
	SendMessage(m_Window, WM_PAINT, 0, 0);
	
	iceLastTicks = dwTime;
}
