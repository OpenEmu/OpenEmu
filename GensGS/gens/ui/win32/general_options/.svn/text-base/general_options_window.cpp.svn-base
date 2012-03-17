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

#include "general_options_window.hpp"
#include "gens/gens_window.hpp"

#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <string.h>
#include <stdio.h>

#include "emulator/g_main.hpp"
#include "gens_ui.hpp"

#include <windowsx.h>

// Gens Win32 resources
#include "ui/win32/resource.h"

// Win32 common controls
#include <commctrl.h>


static WNDCLASS m_WndClass;


// Message colors.
const COLORREF GeneralOptionsWindow::Colors_OSD[4][2] =
{
	{RGB(0xFF, 0xFF, 0xFF), RGB(0x00, 0x00, 0x00)},
	{RGB(0x00, 0x00, 0xFF), RGB(0xFF, 0xFF, 0xFF)},
	{RGB(0x00, 0xFF, 0x00), RGB(0x00, 0x00, 0x00)},
	{RGB(0xFF, 0x00, 0x00), RGB(0x00, 0x00, 0x00)},
};

// Intro effect colors.
const COLORREF GeneralOptionsWindow::Colors_IntroEffect[8][2] =
{
	{RGB(0x00, 0x00, 0x00), RGB(0xFF, 0xFF, 0xFF)},
	{RGB(0x00, 0x00, 0xFF), RGB(0xFF, 0xFF, 0xFF)},
	{RGB(0x00, 0xFF, 0x00), RGB(0x00, 0x00, 0x00)},
	{RGB(0x00, 0xFF, 0xFF), RGB(0x00, 0x00, 0x00)},
	{RGB(0xFF, 0x00, 0x00), RGB(0x00, 0x00, 0x00)},
	{RGB(0xFF, 0x00, 0xFF), RGB(0x00, 0x00, 0x00)},
	{RGB(0xFF, 0xFF, 0x00), RGB(0x00, 0x00, 0x00)},
	{RGB(0xFF, 0xFF, 0xFF), RGB(0x00, 0x00, 0x00)},
};

static const int frameWidth = 160;
static const int frameHeight = 256-16;


GeneralOptionsWindow* GeneralOptionsWindow::m_Instance = NULL;
GeneralOptionsWindow* GeneralOptionsWindow::Instance(HWND parent)
{
	if (m_Instance == NULL)
	{
		// Instance is deleted. Initialize the General Options window.
		m_Instance = new GeneralOptionsWindow();
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
 * GeneralOptionsWindow(): Create the General Options Window.
 */
GeneralOptionsWindow::GeneralOptionsWindow()
{
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
		m_WndClass.lpszClassName = "Gens_General_Options";
		
		RegisterClass(&m_WndClass);
	}
	
	// Messages are processed before the object is finished being created,
	// so this assignment is needed.
	m_Instance = this;
	
	// Create the window.
	// TODO: Don't hardcode the parent window.
	m_Window = CreateWindowEx(NULL, "Gens_General_Options", "General Options",
				  WS_DLGFRAME | WS_POPUP | WS_SYSMENU | WS_CAPTION,
				  CW_USEDEFAULT, CW_USEDEFAULT,
				  (frameWidth*2)+16+8, frameHeight+16+24+8,
				  Gens_hWnd, NULL, ghInstance, NULL);
	
	// Set the actual window size.
	Win32_setActualWindowSize(m_Window, (frameWidth*2)+16+8, frameHeight+16+24+8);
	
	// Center the window on the Gens window.
	Win32_centerOnGensWindow(m_Window);
	
	// Load settings.
	load();
	
	// Show the window.
	UpdateWindow(m_Window);
	ShowWindow(m_Window, 1);
}


GeneralOptionsWindow::~GeneralOptionsWindow()
{
	m_Instance = NULL;
}


LRESULT CALLBACK GeneralOptionsWindow::WndProc_STATIC(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	return m_Instance->WndProc(hWnd, message, wParam, lParam);
}


/**
 * WndProc(): The General Options window procedure.
 * @param hWnd hWnd of the object sending a message.
 * @param message Message being sent by the object.
 * @param wParam
 * @param lParam
 * @return
 */
LRESULT CALLBACK GeneralOptionsWindow::WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	unsigned short i, j;
	
	switch (message)
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
			Win32_ClearSoundBuffer();
			break;
		
		case WM_DRAWITEM:
			drawMsgRadioButton(LOWORD(wParam), (LPDRAWITEMSTRUCT)lParam);
			break;
		
		case WM_COMMAND:
			// Button press
			switch (LOWORD(wParam))
			{
				case IDOK: // Standard dialog button ID
				case IDC_BTN_OK:
				case IDC_BTN_SAVE:
					save();
					DestroyWindow(hWnd);
					break;
				
				case IDC_BTN_APPLY:
					save();
					break;
				
				case IDCANCEL: // Standard dialog button ID
				case IDC_BTN_CANCEL:
					DestroyWindow(hWnd);
					break;
				
				default:
					if ((LOWORD(wParam) & 0xFF00) == 0xA000)
					{
						// Radio button selected.
						selectRadioButton(LOWORD(wParam));
					}
			}
			break;
		
		case WM_DESTROY:
			if (hWnd != m_Window)
				break;
			
			// Delete message color brushes and pens.
			for (i = 0; i < 4; i++)
			{
				for (j = 0; j < 2; j++)
				{
					DeleteBrush(brushOSD[i][j]);
					DeletePen(penOSD[i][j]);
				}
			}
			
			// Delete intro effect color brushes and pens.
			for (i = 0; i < 8; i++)
			{
				for (j = 0; j < 2; j++)
				{
					DeleteBrush(brushIntroEffect[i][j]);
					DeletePen(penIntroEffect[i][j]);
				}
			}
			
			delete this;
			break;
	}
	
	return DefWindowProc(hWnd, message, wParam, lParam);
}


void GeneralOptionsWindow::createChildWindows(HWND hWnd)
{
	if (m_childWindowsCreated)
		return;
	
	HWND grpBox, lblIntroEffectColor;
	
	unsigned short frameLeft = 8;
	unsigned short frameTop = 8;
	unsigned short i, j;
	
	// Create message color brushes and pens.
	for (i = 0; i < 4; i++)
	{
		for (j = 0; j < 2; j++)
		{
			brushOSD[i][j] = CreateSolidBrush(Colors_OSD[i][j]);
			penOSD[i][j] = CreatePen(PS_SOLID, 1, Colors_OSD[i][j]);
		}
	}
	
	// Create intro effect color brushes and pens.
	for (i = 0; i < 8; i++)
	{
		for (j = 0; j < 2; j++)
		{
			brushIntroEffect[i][j] = CreateSolidBrush(Colors_IntroEffect[i][j]);
			penIntroEffect[i][j] = CreatePen(PS_SOLID, 1, Colors_IntroEffect[i][j]);
		}
	}
	
	// Initialize the radio button states.
	state_optColor[0] = 0;
	state_optColor[1] = 0;
	state_optColor[2] = 0;
	
	// On-Screen Display frame
	HWND fraOSD = CreateWindow(WC_BUTTON, "On-Screen Display",
				   WS_CHILD | WS_VISIBLE | BS_GROUPBOX,
				   8, 8, frameWidth, frameHeight,
				   hWnd, NULL, ghInstance, NULL);
	SetWindowFont(fraOSD, fntMain, TRUE);
	
	// FPS counter frame
	createOSDFrame(hWnd, 0, 16, 24, frameWidth-16, (frameHeight/2)-16, "FPS counter");
	
	// Message counter frame
	createOSDFrame(hWnd, 1, 16, (frameHeight/2)+16, frameWidth-16, (frameHeight/2)-16, "Message");
	
	// Miscellaneous frame
	frameLeft += frameWidth + 8;
	
	grpBox = CreateWindow(WC_BUTTON, "Miscellaneous",
			      WS_CHILD | WS_VISIBLE | BS_GROUPBOX,
			      frameLeft, frameTop, frameWidth, frameHeight,
			      hWnd, NULL, ghInstance, NULL);
	SetWindowFont(grpBox, fntMain, TRUE);
	
	// Auto Fix Checksum
	frameTop += 16;
	chkMisc_AutoFixChecksum = CreateWindow(WC_BUTTON, "Auto Fix Checksum",
					       WS_CHILD | WS_VISIBLE | WS_TABSTOP | BS_AUTOCHECKBOX,
					       frameLeft+8, frameTop, 128, 20,
					       hWnd, NULL, ghInstance, NULL);
	SetWindowFont(chkMisc_AutoFixChecksum, fntMain, TRUE);
	
	// Auto Pause
	frameTop += 20;
	chkMisc_AutoPause = CreateWindow(WC_BUTTON, "Auto Pause",
					 WS_CHILD | WS_VISIBLE | WS_TABSTOP | BS_AUTOCHECKBOX,
					 frameLeft+8, frameTop, 128, 20,
					 hWnd, NULL, ghInstance, NULL);
	SetWindowFont(chkMisc_AutoPause, fntMain, TRUE);
	
	// Fast Blur
	frameTop += 20;
	chkMisc_FastBlur = CreateWindow(WC_BUTTON, "Fast Blur",
					WS_CHILD | WS_VISIBLE | WS_TABSTOP | BS_AUTOCHECKBOX,
					frameLeft+8, frameTop, 128, 20,
					hWnd, NULL, ghInstance, NULL);
	SetWindowFont(chkMisc_FastBlur, fntMain, TRUE);
	
	// Show SegaCD LEDs
	frameTop += 20;
	chkMisc_SegaCDLEDs = CreateWindow(WC_BUTTON, "Show SegaCD LEDs",
					  WS_CHILD | WS_VISIBLE | WS_TABSTOP | BS_AUTOCHECKBOX,
					  frameLeft+8, frameTop, 128, 20,
					  hWnd, NULL, ghInstance, NULL);
	SetWindowFont(chkMisc_SegaCDLEDs, fntMain, TRUE);
	
	// Border Color Emulation
	frameTop += 20;
	chkMisc_BorderColorEmulation = CreateWindow(WC_BUTTON, "Border Color Emulation",
						    WS_CHILD | WS_VISIBLE | WS_TABSTOP | BS_AUTOCHECKBOX,
						    frameLeft+8, frameTop, 128, 20,
						    hWnd, NULL, ghInstance, NULL);
	SetWindowFont(chkMisc_BorderColorEmulation, fntMain, TRUE);
	
	// Border Color Emulation isn't implemented on Win32 yet, so disable the checkbox.
	Button_Enable(chkMisc_BorderColorEmulation, FALSE);
	
	// Pause Tint
	frameTop += 20;
	chkMisc_PauseTint = CreateWindow(WC_BUTTON, "Pause Tint",
						WS_CHILD | WS_VISIBLE | WS_TABSTOP | BS_AUTOCHECKBOX,
						frameLeft+8, frameTop, 128, 20,
						hWnd, NULL, ghInstance, NULL);
	SetWindowFont(chkMisc_PauseTint, fntMain, TRUE);
	
	// Intro effect color label
	frameTop += 20+16+2;
	lblIntroEffectColor = CreateWindow(WC_STATIC, "Intro Effect Color:",
					   WS_CHILD | WS_VISIBLE | SS_CENTER,
					   frameLeft+8, frameTop, frameWidth-16, 20,
					   hWnd, NULL, ghInstance, NULL);
	SetWindowFont(lblIntroEffectColor, fntMain, TRUE);
	
	// Intro effect color buttons
	frameLeft += 8+4 + (((frameWidth-16) - (4*(16+8))) / 2);
	frameTop += 20;
	for (i = 0; i < 8; i++)
	{
		optIntroEffectColor[i] = CreateWindow(
				WC_STATIC, "", WS_CHILD | WS_VISIBLE | SS_CENTER | SS_OWNERDRAW | SS_NOTIFY,
				frameLeft + ((i%4)*(16+8)), frameTop + ((i / 4) * (16+4)), 16, 16,
				hWnd, (HMENU)(0xA010 + i), ghInstance, NULL);
	}
	
	// Add the buttons.
	addDialogButtons(hWnd, WndBase::BAlign_Default,
			 WndBase::BUTTON_OK | WndBase::BUTTON_APPLY | WndBase::BUTTON_CANCEL, 0);
	
	// Set focus to "FPS counter" - "Enable".
	SetFocus(chkOSD_Enable[0]);
	
	// Child windows created.
	m_childWindowsCreated = true;
}


void GeneralOptionsWindow::createOSDFrame(HWND hWnd, const int index,
					  const short frameLeft,
					  const short frameTop,
					  const short frameWidth,
					  const short frameHeight,
					  const char* title)
{
	// Message frame
	HWND grpBox, lblColor;
	int i;
	
	grpBox = CreateWindow(WC_BUTTON, title,
			      WS_CHILD | WS_VISIBLE | WS_TABSTOP | BS_GROUPBOX,
			      frameLeft, frameTop, frameWidth, frameHeight,
			      hWnd, NULL, ghInstance, NULL);
	SetWindowFont(grpBox, fntMain, TRUE);
	
	// Enable
	chkOSD_Enable[index] = CreateWindow(WC_BUTTON, "Enable",
					    WS_CHILD | WS_VISIBLE | WS_TABSTOP | BS_AUTOCHECKBOX,
					    frameLeft+8, frameTop+16, 128, 20,
					    hWnd, NULL, ghInstance, NULL);
	SetWindowFont(chkOSD_Enable[index], fntMain, TRUE);
	
	// Double Sized
	chkOSD_DoubleSized[index] = CreateWindow(WC_BUTTON, "Double Sized",
						 WS_CHILD | WS_VISIBLE | WS_TABSTOP | BS_AUTOCHECKBOX,
						 frameLeft+8, frameTop+16+20, 128, 20,
						 hWnd, NULL, ghInstance, NULL);
	SetWindowFont(chkOSD_DoubleSized[index], fntMain, TRUE);
	
	// Transparency
	chkOSD_Transparency[index] = CreateWindow(WC_BUTTON, "Transparency",
						  WS_CHILD | WS_VISIBLE | WS_TABSTOP | BS_AUTOCHECKBOX,
						  frameLeft+8, frameTop+16+20+20, 128, 20,
						  hWnd, NULL, ghInstance, NULL);
	SetWindowFont(chkOSD_Transparency[index], fntMain, TRUE);
	
	// Color label
	lblColor = CreateWindow(WC_STATIC, "Color:",
				WS_CHILD | WS_VISIBLE | SS_LEFT,
				frameLeft+8, frameTop+16+20+20+20+2, 36, 20,
				hWnd, NULL, ghInstance, NULL);
	SetWindowFont(lblColor, fntMain, TRUE);
	
	// Radio buttons
	for (i = 0; i < 4; i++)
	{
		optOSD_Color[index][i] = CreateWindow(
				WC_STATIC, "", WS_CHILD | WS_VISIBLE | SS_CENTER | SS_OWNERDRAW | SS_NOTIFY,
				frameLeft+8+36+4+(i*(16+8)), frameTop+16+20+20+20+2, 16, 16,
				hWnd, (HMENU)(0xA000 + ((index * 4) + i)), ghInstance, NULL);
	}
}


void GeneralOptionsWindow::drawMsgRadioButton(int identifier, LPDRAWITEMSTRUCT lpDrawItem)
{
	unsigned short index;
	unsigned short button;
	
	if ((identifier & 0xF0) == 0x00)
	{
		index = (identifier & 0x0F) / 4;
		button = (identifier & 0x0F) % 4;
	}
	else if ((identifier & 0xF0) == 0x10)
	{
		index = 2;
		button = (identifier & 0x0F);
	}
	else
	{
		// Unknown button set.
		return;
	}
	
	HDC hDC = lpDrawItem->hDC;
	RECT itemRect = lpDrawItem->rcItem;
	
	if (index < 2)
		FillRect(hDC, &itemRect, brushOSD[button][0]);
	else //if (index == 2)
		FillRect(hDC, &itemRect, brushIntroEffect[button][0]);
	
	// Check if the radio button is checked.
	if (state_optColor[index] == button)
	{
		// Checked. Draw a circle in the middle of the box.
		
		if (index < 2)
		{
			SelectObject(hDC, brushOSD[button][1]);
			SelectObject(hDC, penOSD[button][1]);
		}
		else //if (index == 2)
		{
			SelectObject(hDC, brushIntroEffect[button][1]);
			SelectObject(hDC, penIntroEffect[button][1]);
		}
		
		Ellipse(hDC, itemRect.left + 4, itemRect.top + 4,
			     itemRect.right - 4, itemRect.bottom - 4);
	}
}


void GeneralOptionsWindow::selectRadioButton(int identifier)
{
	unsigned short index;
	unsigned short button;
	
	if ((identifier & 0xF0) == 0x00)
	{
		index = (identifier & 0x0F) / 4;
		button = (identifier & 0x0F) % 4;
	}
	else if ((identifier & 0xF0) == 0x10)
	{
		index = 2;
		button = (identifier & 0x0F);
	}
	else
	{
		// Unknown button set.
		return;
	}
	
	// Make sure the index and button IDs are valid.
	if (index < 2 && button >= 4)
		return;
	else if (index == 2 && button >= 8)
		return;
	else if (index > 2)
		return;
	
	if (state_optColor[index] == button)
	{
		// State hasn't changed. Don't do anything.
		return;
	}
	
	// State has changed.
	unsigned short oldButton = state_optColor[index];
	state_optColor[index] = button;
	if (index < 2)
	{
		InvalidateRect(optOSD_Color[index][oldButton], NULL, FALSE);
		InvalidateRect(optOSD_Color[index][button], NULL, FALSE);
	}
	else //if (index == 2)
	{
		InvalidateRect(optIntroEffectColor[oldButton], NULL, FALSE);
		InvalidateRect(optIntroEffectColor[button], NULL, FALSE);
	}
}


/**
 * load(): Load settings.
 */
void GeneralOptionsWindow::load(void)
{
	// Get the current options.
	unsigned char curFPSStyle, curMsgStyle;
	
	// Miscellaneous
	Button_SetCheck(chkMisc_AutoFixChecksum, (Auto_Fix_CS ? BST_CHECKED : BST_UNCHECKED));
	Button_SetCheck(chkMisc_AutoPause, (Auto_Pause ? BST_CHECKED : BST_UNCHECKED));
	Button_SetCheck(chkMisc_FastBlur, (draw->fastBlur() ? BST_CHECKED : BST_UNCHECKED));
	Button_SetCheck(chkMisc_SegaCDLEDs, (Show_LED ? BST_CHECKED : BST_UNCHECKED));
	Button_SetCheck(chkMisc_PauseTint, (Video.pauseTint ? BST_CHECKED : BST_UNCHECKED));
	
	// Border Color Emulation isn't implemented on Win32 yet, so don't load this yet.
	//Button_SetCheck(chkMisc_BorderColorEmulation, (Video.borderColorEmulation ? BST_CHECKED : BST_UNCHECKED));
	
	// FPS counter
	Button_SetCheck(chkOSD_Enable[0], (draw->fpsEnabled() ? BST_CHECKED : BST_UNCHECKED));
	
	curFPSStyle = draw->fpsStyle();
	Button_SetCheck(chkOSD_DoubleSized[0], ((curFPSStyle & 0x10) ? BST_CHECKED : BST_UNCHECKED));
	Button_SetCheck(chkOSD_Transparency[0], ((curFPSStyle & 0x08) ? BST_CHECKED : BST_UNCHECKED));
	state_optColor[0] = (curFPSStyle & 0x06) >> 1;
	
	// Message
	Button_SetCheck(chkOSD_Enable[1], (draw->msgEnabled() ? BST_CHECKED : BST_UNCHECKED));
	
	curMsgStyle = draw->msgStyle();
	Button_SetCheck(chkOSD_DoubleSized[1], ((curMsgStyle & 0x10) ? BST_CHECKED : BST_UNCHECKED));
	Button_SetCheck(chkOSD_Transparency[1], ((curMsgStyle & 0x08) ? BST_CHECKED : BST_UNCHECKED));
	state_optColor[1] = (curMsgStyle & 0x06) >> 1;
	
	// Intro effect color
	state_optColor[2] = draw->introEffectColor();
}


/**
 * save(): Save settings.
 */
void GeneralOptionsWindow::save(void)
{
	// Save the current options.
	unsigned char curFPSStyle, curMsgStyle;
	
	// System
	Auto_Fix_CS = (Button_GetCheck(chkMisc_AutoFixChecksum) == BST_CHECKED);
	Auto_Pause = (Button_GetCheck(chkMisc_AutoPause) == BST_CHECKED);
	draw->setFastBlur(Button_GetCheck(chkMisc_FastBlur) == BST_CHECKED);
	Show_LED = (Button_GetCheck(chkMisc_SegaCDLEDs) == BST_CHECKED);
	Video.pauseTint = (Button_GetCheck(chkMisc_PauseTint) == BST_CHECKED);
	
	// If Auto Pause is enabled, deactivate emulation.
	// If Auto Pause is disabled, activate emulation.
	Active = !Auto_Pause;
	
	// Border Color Emulation isn't implemented on Win32 yet, so don't save this yet.
	//Video.borderColorEmulation = (Button_GetCheck(chkMisc_BorderColorEmulation) == BST_CHECKED);
	
	// FPS counter
	draw->setFPSEnabled(Button_GetCheck(chkOSD_Enable[0]) == BST_CHECKED);
	
	curFPSStyle = draw->fpsStyle() & ~0x18;
	curFPSStyle |= ((Button_GetCheck(chkOSD_DoubleSized[0]) == BST_CHECKED) ? 0x10 : 0x00);
	curFPSStyle |= ((Button_GetCheck(chkOSD_Transparency[0]) == BST_CHECKED) ? 0x08 : 0x00);
	curFPSStyle &= ~0x06;
	curFPSStyle |= state_optColor[0] << 1;
	draw->setFPSStyle(curFPSStyle);
	
	// Message
	draw->setMsgEnabled(Button_GetCheck(chkOSD_Enable[1]) == BST_CHECKED);
	curMsgStyle = draw->msgStyle() & ~0x18;
	curMsgStyle |= ((Button_GetCheck(chkOSD_DoubleSized[1]) == BST_CHECKED) ? 0x10 : 0x00);
	curMsgStyle |= ((Button_GetCheck(chkOSD_Transparency[1]) == BST_CHECKED) ? 0x08 : 0x00);
	curMsgStyle &= ~0x06;
	curMsgStyle |= state_optColor[1] << 1;
	draw->setMsgStyle(curMsgStyle);
	
	// Intro effect color
	draw->setIntroEffectColor(static_cast<unsigned char>(state_optColor[2]));
}
