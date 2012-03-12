// Menu handling

#include "burner.h"

#ifdef _MSC_VER
// #include <winable.h>
#endif

int nOtherRes = 0;

HMENU hMenu = NULL;
HMENU hMenuPopup = NULL;
HWND hMenubar = NULL;			  			// Handle to the Toolbar control comprising the menu
static HMENU hBlitterMenu[8] = {NULL, };	// Handles to the blitter-specific sub-menus
static HMENU hAudioPluginMenu[8] = {NULL, };

bool bMenuDisplayed = false;
bool bModelessMenu = false;
int nLastMenu = 0;
static int nRecursions = -1;
static HMENU hCurrentMenu;
static int nCurrentItem;
static int nCurrentItemFlags;

int nMenuHeight = 0;
int nWindowSize = 0;
int nScreenSize = 0;
int nScreenSizeHor = 0;
int nScreenSizeVer = 0;

TCHAR szPrevGames[SHOW_PREV_GAMES][32];

static HHOOK hMenuHook;

static LRESULT CALLBACK MenuHook(int nCode, WPARAM wParam, LPARAM lParam)
{

	switch (((MSG*)lParam)->message) {

		case WM_MOUSEMOVE: {				// Translate coordinates to menubar client coordinates
			RECT rect;
			POINT point = {GET_X_LPARAM(((MSG*)lParam)->lParam), GET_Y_LPARAM(((MSG*)lParam)->lParam)};

			GetWindowRect(hMenubar, &rect);

			if (point.x >= rect.left && point.y >= rect.top && point.x < rect.right && point.y < rect.bottom) {
				RECT buttonrect;
				SendMessage(hMenubar, TB_GETITEMRECT, nLastMenu, (LPARAM)&buttonrect);

				if (!(point.x >= rect.left + buttonrect.left && point.y >= rect.top + buttonrect.top && point.x < rect.left + buttonrect.right && point.y < rect.top + buttonrect.bottom)) {
					SendNotifyMessage(hMenubar, WM_MOUSEMOVE, wParam, MAKELONG(point.x - rect.left, point.y - rect.top));
				}
			}
			break;
		}

		case WM_LBUTTONDOWN: {
			RECT rect;
			RECT buttonrect;
			POINT point = {GET_X_LPARAM(((MSG*)lParam)->lParam), GET_Y_LPARAM(((MSG*)lParam)->lParam)};

			GetWindowRect(hMenubar, &rect);
			SendMessage(hMenubar, TB_GETITEMRECT, nLastMenu, (LPARAM)&buttonrect);

			// If the button is pressed over the currently selected menu title, eat the message and close the menu
			if (point.x >= rect.left + buttonrect.left && point.y >= rect.top + buttonrect.top && point.x < rect.left + buttonrect.right && point.y < rect.top + buttonrect.bottom) {
				PostMessage(hMenubar, WM_CANCELMODE, 0, 0);
				return 1;
			}
			break;
		}

	}

	MenuHandleKeyboard((MSG*)lParam);

	return CallNextHookEx(hMenuHook, nCode, wParam, lParam);
}

void DisplayPopupMenu(int nMenu)
{
	if (bMenuDisplayed) {
		EndMenu();
		if (nLastMenu != nMenu) {
			PostMessage(hScrnWnd, UM_DISPLAYPOPUP, nMenu, 0);
		}
	} else {
		HMENU hPopupMenu = GetSubMenu(hMenu, nMenu);
		RECT clientRect;
		RECT buttonRect;

		nLastMenu = nMenu;
		nRecursions = 0;
		nCurrentItemFlags = 0;

		GetWindowRect(hMenubar, &clientRect);
		SendMessage(hMenubar, TB_GETITEMRECT, nMenu, (LPARAM)&buttonRect);

	 	if (!bModelessMenu) {
			hMenuHook = SetWindowsHookEx(WH_MSGFILTER, MenuHook, NULL, GetCurrentThreadId());
		}
  		TrackPopupMenuEx(hPopupMenu, TPM_LEFTALIGN | TPM_TOPALIGN, clientRect.left + buttonRect.left, clientRect.top + buttonRect.bottom, hScrnWnd, NULL);
	 	if (!bModelessMenu) {
			UnhookWindowsHookEx(hMenuHook);
		}
	}
}

int OnNotify(HWND, int, NMHDR* lpnmhdr)		// HWND hwnd, int id, NMHDR* lpnmhdr
{
	switch (((TBNOTIFY*)lpnmhdr)->hdr.code) {

		case TBN_DROPDOWN: {
			if (!bMenuDisplayed || ((TBNOTIFY*)lpnmhdr)->iItem - MENU_MENU_0 != nLastMenu) {
				DisplayPopupMenu(((TBNOTIFY*)lpnmhdr)->iItem - MENU_MENU_0);
				nLastMenu = ((TBNOTIFY*)lpnmhdr)->iItem - MENU_MENU_0;
			}
			return TBDDRET_DEFAULT;
		}

		case TBN_HOTITEMCHANGE: {
			int nItem = ((NMTBHOTITEM*)lpnmhdr)->idNew;

			// If a menu is being displayed, close it and display the new one
			if (bMenuDisplayed) {
				if (nItem) {
					if (nItem - MENU_MENU_0 != nLastMenu) {
						DisplayPopupMenu(nItem - MENU_MENU_0);
						return 1;
					}
				}
			} else {
				nLastMenu = nItem - MENU_MENU_0;
				if (!nItem && !bAltPause) {
					GameInpCheckMouse();
				}
				return 0;
			}
		}
	}
	return 0;
}

int OnMenuSelect(HWND, HMENU, int nItem, HMENU, UINT nFlags)
{
//	printf("WM_MENUSELECT: %4i, %08x\n", nItem, nFlags);
	nCurrentItem = nItem;
	nCurrentItemFlags = nFlags;
	return 0;
}

int OnInitMenuPopup(HWND, HMENU hNewMenuPopup, UINT, BOOL bWindowsMenu)
{
	if (!bWindowsMenu) {
		if (!nRecursions) {
			bMenuDisplayed = true;
			SendMessage(hMenubar, TB_PRESSBUTTON, nLastMenu + MENU_MENU_0, MAKELONG(1, 0));
			hCurrentMenu = hNewMenuPopup;
		}
		nRecursions++;
	}
	return 0;
}

int OnUnInitMenuPopup(HWND, HMENU, UINT, BOOL)
{
	if (nRecursions <= 1) {
		bMenuDisplayed = false;
  		SendMessage(hMenubar, TB_PRESSBUTTON, nLastMenu + MENU_MENU_0, MAKELONG(0, 0));
		if(!bAltPause) {
			if (bRunPause) {
				bRunPause = 0;
			}
			GameInpCheckMouse();
		}
	}
	nRecursions--;

	return 0;
}

bool MenuHandleKeyboard(MSG* Msg)
{
	static bool bProcessAltKeyUp = true;

	if (!bMenuEnabled) {
		return 0;
	}

	if (Msg->message == WM_KEYDOWN) {
		switch (Msg->wParam) {
			case VK_ESCAPE: {
				if (bMenuDisplayed) {
					if (nRecursions > 1) {
						break;
					} else {
						EndMenu();
						SendMessage(hMenubar, TB_SETHOTITEM, nLastMenu, 0);
						return 1;
					}
				} else {
					if (SendMessage(hMenubar, TB_GETHOTITEM, 0, 0) != -1) {
						SendMessage(hMenubar, TB_SETHOTITEM, (WPARAM)-1, 0);
						if (!bAltPause) {
							GameInpCheckMouse();
						}
						return 1;
					}
				}
				break;
			}
			case VK_LEFT: {
				int nItem;
				if (bMenuDisplayed) {
					if (nRecursions > 1) {
						break;
					} else {
						nItem = nLastMenu;
					}
				} else {
					nItem = SendMessage(hMenubar, TB_GETHOTITEM, 0, 0);
				}
				if (nItem != -1) {
					if (nItem) {
						nItem -= 1;
					} else {
						nItem = 5;
					}
					SendMessage(hMenubar, TB_SETHOTITEM, (WPARAM)nItem, 0);
					return 1;
				}
				break;
			}
			case VK_RIGHT: {
				int nItem;
				if (bMenuDisplayed) {
					if (nCurrentItemFlags & MF_POPUP) {
						break;
					} else {
						nItem = nLastMenu;
					}
				} else {
					nItem = SendMessage(hMenubar, TB_GETHOTITEM, 0, 0);
				}
				if (nItem != -1) {
					if (nItem != 5) {
						nItem += 1;
					} else {
						nItem = 0;
					}
					SendMessage(hMenubar, TB_SETHOTITEM, (WPARAM)nItem, 0);
					return 1;
				}
				break;
			}
			case VK_UP: {
				if (!bMenuDisplayed) {
					int nItem = SendMessage(hMenubar, TB_GETHOTITEM, 0, 0);
					if (nItem == -1) {
						break;
					} else {
						SendMessage(hMenubar, TB_SETHOTITEM, (WPARAM)-1, 0);
						DisplayPopupMenu(nItem);
						return 1;
					}
				}
				break;
			}
			case VK_DOWN: {
				if (!bMenuDisplayed) {
					int nItem = SendMessage(hMenubar, TB_GETHOTITEM, 0, 0);
					if (nItem == -1) {
						break;
					} else {
						SendMessage(hMenubar, TB_SETHOTITEM, (WPARAM)-1, 0);
						DisplayPopupMenu(nItem);
						return 1;
					}
				}
				break;
			}
		}
	} else {
		if (!bLeftAltkeyMapped || bRunPause || !bDrvOkay) {
			if (Msg->message == WM_SYSKEYDOWN && Msg->wParam == VK_MENU) {
				if (bMenuDisplayed) {
					bProcessAltKeyUp = false;
					EndMenu();
					if (!bAltPause) {
						GameInpCheckMouse();
					}
					return 1;
				}
			} else {
				if (Msg->message == WM_SYSKEYUP && Msg->wParam == VK_MENU) {
					if (bProcessAltKeyUp) {
						int nItem = SendMessage(hMenubar, TB_GETHOTITEM, 0, 0);
						if (nItem == -1) {													// Menu enabled
							SendMessage(hMenubar, TB_SETHOTITEM, 0, 0);
							InputSetCooperativeLevel(false, bAlwaysProcessKeyboardInput);
							return 1;
						} else {															// Menu disabled
							SendMessage(hMenubar, TB_SETHOTITEM, (WPARAM)-1, 0);
							if (!bAltPause) {
								GameInpCheckMouse();
							}
							return 1;
						}
					}
				} else {
					if (Msg->message == UM_DISPLAYPOPUP) {
						DisplayPopupMenu(Msg->wParam);
						InputSetCooperativeLevel(false, bAlwaysProcessKeyboardInput);
						return 1;
					}
				}
			}
		}
	}

	bProcessAltKeyUp = true;

	return 0;
}

int MenuCreate()
{
	TBBUTTON button;
	TCHAR szButtonText[32];
	MENUITEMINFO menuItemInfo;
	MENUINFO menu;

	if (hMenu == NULL) {
		hMenu = FBALoadMenu(hAppInst, MAKEINTRESOURCE(IDR_MENU));					// Main application menu
		hBlitterMenu[0] = FBALoadMenu(hAppInst, MAKEINTRESOURCE(IDR_MENU_BLITTER_1));	// DirectDraw Standard blitter
		hBlitterMenu[1] = FBALoadMenu(hAppInst, MAKEINTRESOURCE(IDR_MENU_BLITTER_2));	// Direct3D
		hBlitterMenu[2] = FBALoadMenu(hAppInst, MAKEINTRESOURCE(IDR_MENU_BLITTER_3));	// Software effects blitter
		hBlitterMenu[3] = FBALoadMenu(hAppInst, MAKEINTRESOURCE(IDR_MENU_BLITTER_4));	// DirectX 9
		hBlitterMenu[4] = FBALoadMenu(hAppInst, MAKEINTRESOURCE(IDR_MENU_BLITTER_5));	// DirectX 9 Alt

		hAudioPluginMenu[0] = FBALoadMenu(hAppInst, MAKEINTRESOURCE(IDR_MENU_AUD_PLUGIN_1));
		hAudioPluginMenu[1] = FBALoadMenu(hAppInst, MAKEINTRESOURCE(IDR_MENU_AUD_PLUGIN_2));
	}
	
	if (hMenuPopup == NULL) {
		hMenuPopup = CreatePopupMenu();

		menuItemInfo.cbSize = sizeof(MENUITEMINFO);
		menuItemInfo.fMask = MIIM_TYPE;
		menuItemInfo.dwTypeData = szButtonText;

		for (int i = 0; i < 6; i++) {
			menuItemInfo.cch = 32;
			GetMenuItemInfo(hMenu, i, true, &menuItemInfo);
			AppendMenu(hMenuPopup, MF_POPUP | MF_STRING, (UINT_PTR)GetSubMenu(hMenu, i), szButtonText);
		}
	}

	MenuEnableItems();
	MenuUpdate();

	bMenuDisplayed = false;
	nLastMenu = -1;

	hMenubar = CreateWindowEx(0,
		TOOLBARCLASSNAME, NULL,
		TBSTYLE_FLAT | TBSTYLE_LIST | CCS_NODIVIDER | CCS_NOPARENTALIGN | CCS_NORESIZE | WS_CHILD | WS_CLIPCHILDREN | WS_CLIPSIBLINGS | WS_VISIBLE,
		0, 0, 0, 0,
		hScrnWnd, NULL, hAppInst, NULL);

	SendMessage(hMenubar, TB_BUTTONSTRUCTSIZE, sizeof(TBBUTTON), 0);
	SendMessage(hMenubar, TB_SETBITMAPSIZE, 0, 0);
	
	// Reset window menu to default
	GetSystemMenu(hScrnWnd, TRUE);

	if (bModelessMenu) {							// Make menu modeless

		memset(&menu, 0, sizeof(MENUINFO));
		menu.cbSize = sizeof(MENUINFO);
		menu.fMask = MIM_APPLYTOSUBMENUS | MIM_STYLE;
		menu.dwStyle = MNS_MODELESS | MNS_CHECKORBMP;

		SetMenuInfo(hMenuPopup, &menu);

		for (int i = 0; i < 6; i++) {
			SetMenuInfo(GetSubMenu(hMenu, i), &menu);
		}

		memset(&menu, 0, sizeof(MENUINFO));
		menu.cbSize = sizeof(MENUINFO);
		menu.fMask = MIM_STYLE | MIM_MAXHEIGHT;

#if 0
		// Doesn't seem to work
		{
			HMENU hWindowMenu = GetSystemMenu(hScrnWnd, FALSE);
			GetMenuInfo(hWindowMenu, &menu);
			menu.dwStyle |= MNS_MODELESS;
			SetMenuInfo(hWindowMenu, &menu);
		}
#endif
	}

	// Add buttons to the menu toolbar
	memset(&button, 0, sizeof(TBBUTTON));
	memset(&menuItemInfo, 0, sizeof(MENUITEMINFO));

	menuItemInfo.cbSize = sizeof(MENUITEMINFO);
	menuItemInfo.fMask = MIIM_TYPE;
	menuItemInfo.dwTypeData = szButtonText;

	for (int i = 0; i < 6; i++) {

		menuItemInfo.cch = 32;
		GetMenuItemInfo(hMenu, i, true, &menuItemInfo);

		button.iBitmap = 0;
		button.idCommand = MENU_MENU_0 + i;
		button.fsState = TBSTATE_ENABLED;
		button.fsStyle = TBSTYLE_DROPDOWN | TBSTYLE_AUTOSIZE;

		button.iString = (INT_PTR)szButtonText;

		SendMessage(hMenubar, TB_ADDBUTTONS, 1, (LPARAM)&button);
	}
	
	SendMessage(hMenubar, TB_AUTOSIZE, 0, 0);

	return 0;
}

void MenuDestroy()
{
	if (hMenubar) {
		DestroyWindow(hMenubar);
		hMenubar = NULL;
	}

	{
		MENUITEMINFO myMenuItemInfo;
		myMenuItemInfo.cbSize = sizeof(MENUITEMINFO);
		myMenuItemInfo.fMask = MIIM_SUBMENU | MIIM_STATE;
        myMenuItemInfo.fState = MFS_GRAYED;
        myMenuItemInfo.hSubMenu = NULL;
		if (hMenu) {
			SetMenuItemInfo(GetSubMenu(hMenu, 1), 1, TRUE, &myMenuItemInfo);
		}
		if (hMenuPopup) {
			SetMenuItemInfo(GetSubMenu(hMenuPopup, 1), 1, TRUE, &myMenuItemInfo);
		}

		for (int i = 0; i < 5; i++) {
			if (hBlitterMenu[i]) {
				DestroyMenu(hBlitterMenu[i]);
				hBlitterMenu[i] = 0;
			}
		}
		
		for (int i = 0; i < 2; i++) {
			if (hAudioPluginMenu[i]) {
				DestroyMenu(hAudioPluginMenu[i]);
				hAudioPluginMenu[i] = 0;
			}
		}
	}

	if(hMenu) {
		DestroyMenu(hMenu);
		hMenu = NULL;
	}
	if (hMenuPopup) {
		DestroyMenu(hMenuPopup);
		hMenuPopup = NULL;
	}
}

// Update the arade resolution menuitem
void CreateArcaderesItem()
{
	int nGameWidth, nGameHeight;

	TCHAR szItemText[256];
	MENUITEMINFO menuItem = {sizeof(MENUITEMINFO), MIIM_TYPE, MFT_STRING, 0, 0, NULL, NULL, NULL, 0, szItemText, 0, 0, };

	// horizontal oriented
	FBALoadString(hAppInst, IDS_MENU_0, szItemText, 256);
	if ((bDrvOkay) && !(BurnDrvGetFlags() & BDF_ORIENTATION_VERTICAL)) {
		BurnDrvGetVisibleSize(&nGameWidth, &nGameHeight);
		_stprintf(szItemText + _tcslen(szItemText), _T("\t(%i x %i)"), nGameWidth, nGameHeight);
	}
	menuItem.cch = _tcslen(szItemText);
	SetMenuItemInfo(hMenu, MENU_RES_ARCADE, 0, &menuItem);

	// vertical oriented
	TCHAR szItemText2[256];
	MENUITEMINFO menuItem2 = {sizeof(MENUITEMINFO), MIIM_TYPE, MFT_STRING, 0, 0, NULL, NULL, NULL, 0, szItemText2, 0, 0, };
	
	FBALoadString(hAppInst, IDS_MENU_0, szItemText2, 256);
	if (bDrvOkay && BurnDrvGetFlags() & BDF_ORIENTATION_VERTICAL) {
		BurnDrvGetVisibleSize(&nGameWidth, &nGameHeight);
		_stprintf(szItemText + _tcslen(szItemText2), _T("\t(%i x %i)"), nGameWidth, nGameHeight);
	}
	menuItem2.cch = _tcslen(szItemText2);
	SetMenuItemInfo(hMenu, MENU_RES_ARCADE_VERTICAL, 0, &menuItem2);
}

// (Horizontal oriented) Update the custom resolution menuitem
static void CreateOtherresItem(bool bOther)
{
	TCHAR szItemText[256];
	MENUITEMINFO menuItem = {sizeof(MENUITEMINFO), MIIM_TYPE, MFT_STRING, 0, 0, NULL, NULL, NULL, 0, szItemText, 0, 0 };

	FBALoadString(hAppInst, IDS_MENU_1, szItemText, 256);
	if (bOther) {
		_stprintf(szItemText + _tcslen(szItemText), _T("\t(%i x %i)"), nVidHorWidth, nVidHorHeight);
	}
	menuItem.cch = _tcslen(szItemText);
	SetMenuItemInfo(hMenu, MENU_RES_OTHER, 0, &menuItem);
}

// (Vertical oriented) Update the custom resolution menuitem
static void CreateOtherresItemVer(bool bOther)
{
	TCHAR szItemText[256];
	MENUITEMINFO menuItem = {sizeof(MENUITEMINFO), MIIM_TYPE, MFT_STRING, 0, 0, NULL, NULL, NULL, 0, szItemText, 0, 0 };

	FBALoadString(hAppInst, IDS_MENU_1, szItemText, 256);
	if (bOther) {
		_stprintf(szItemText + _tcslen(szItemText), _T("\t(%i x %i)"), nVidVerWidth, nVidVerHeight);
	}
	menuItem.cch = _tcslen(szItemText);
	SetMenuItemInfo(hMenu, MENU_RES_OTHER_VERTICAL, 0, &menuItem);
}

static void CreatepresetResItems()
{
	TCHAR szItemText[256];
	MENUITEMINFO menuItem = {sizeof(MENUITEMINFO), MIIM_TYPE, MFT_STRING, 0, 0, NULL, NULL, NULL, 0, szItemText, 0, 0 };

	// horizontal oriented
	for (int i = 0; i < 4; i++) {
		_stprintf(szItemText, _T("%i x %i"), VidPreset[i].nWidth, VidPreset[i].nHeight);
		menuItem.cch = _tcslen(szItemText);
		SetMenuItemInfo(hMenu, MENU_RES_1 + i, 0, &menuItem);
	}

	// vertical oriented
	for (int i = 0; i < 4; i++) {
		_stprintf(szItemText, _T("%i x %i"), VidPresetVer[i].nWidth, VidPresetVer[i].nHeight);
		menuItem.cch = _tcslen(szItemText);
		SetMenuItemInfo(hMenu, MENU_RES_1_VERTICAL + i, 0, &menuItem);
	}
	return;
}

// Update the other gamma menuitem
static void CreateOtherGammaItem(bool bOther)
{
	TCHAR szItemText[256];
	MENUITEMINFO menuItem = {sizeof(MENUITEMINFO), MIIM_TYPE, MFT_STRING, 0, 0, NULL, NULL, NULL, 0, szItemText, 0, 0 };

	FBALoadString(hAppInst, IDS_MENU_2, szItemText, 256);
	if (bOther) {
		_stprintf(szItemText + _tcslen(szItemText), _T("\t(%1.2f)"), nGamma);
	}
	menuItem.cch = _tcslen(szItemText);
	SetMenuItemInfo(hMenu, MENU_GAMMA_OTHER, 0, &menuItem);
}

// Update the cpu speed menuitem
static void CreateCPUSpeedItem(bool bOther)
{
	TCHAR szItemText[256];
	MENUITEMINFO menuItem = {sizeof(MENUITEMINFO), MIIM_TYPE, MFT_STRING, 0, 0, NULL, NULL, NULL, 0, szItemText, 0, 0 };

	FBALoadString(hAppInst, IDS_MENU_3, szItemText, 256);
	if (bOther) {
		_stprintf(szItemText + _tcslen(szItemText), _T("\t(%d%%)"), nBurnCPUSpeedAdjust * 100 / 256);
	}
	menuItem.cch = _tcslen(szItemText);
	SetMenuItemInfo(hMenu, MENU_SETCPUCLOCK, 0, &menuItem);
}

// Update state slot menuitmes
static void CreateStateslotItems()
{
	TCHAR szItemText[256];
	MENUITEMINFO menuItem = {sizeof(MENUITEMINFO), MIIM_TYPE, MFT_STRING, 0, 0, NULL, NULL, NULL, 0, szItemText, 0, 0 };

	_sntprintf(szItemText, 256, FBALoadStringEx(hAppInst, IDS_MENU_4, true), nSavestateSlot);
	menuItem.cch = _tcslen(szItemText);
	SetMenuItemInfo(hMenu, MENU_STATE_LOAD_SLOT, 0, &menuItem);

	_sntprintf(szItemText, 256, FBALoadStringEx(hAppInst, IDS_MENU_5, true), nSavestateSlot);
	menuItem.cch = _tcslen(szItemText);
	SetMenuItemInfo(hMenu, MENU_STATE_SAVE_SLOT, 0, &menuItem);
}
#if 0
// Update CD menu items
static void CreateCDItems()
{
	TCHAR szItemText[256];
	MENUITEMINFO menuItem = { sizeof(MENUITEMINFO), MIIM_TYPE, MFT_STRING, 0, 0, NULL, NULL, NULL, 0, szItemText, 0, NULL };

	_sntprintf(szItemText, 256, FBALoadStringEx(hAppInst, IDS_CD_SELECT_IMAGE, true));
	if (nCDEmuSelect == 0) {
		TCHAR szImage[32];
		int length = _tcslen(szItemText);

		_sntprintf(szImage, 13, _T("\t%s"), CDEmuImage);
		if (_tcslen(szImage) > 13) {
			_sntprintf(szImage + 13, 8, _T("..."));
		}
		
		_sntprintf(szItemText + length - 1, 256 - length, szImage);
	}
	menuItem.cch = _tcslen(szItemText);
	SetMenuItemInfo(hMenu, MENU_CDIMAGE, 0, &menuItem);
}
#endif
// Update bullets, checkmarks, and item text
void MenuUpdate()
{
	int var;

//	if (!bMenuEnabled) {
//		return;
//	}

	CreateStateslotItems();

	CheckMenuItem(hMenu, MENU_PAUSE, bAltPause ? MF_CHECKED : MF_UNCHECKED);
	CheckMenuItem(hMenu, MENU_ALLRAM, bDrvSaveAll ? MF_CHECKED : MF_UNCHECKED);
	
//	CreateCDItems();
//	CheckMenuRadioItem(hMenu, MENU_CDIMAGE, MENU_CDIMAGE, MENU_CDIMAGE + nCDEmuSelect, MF_BYCOMMAND);

	CheckMenuItem(hMenu, MENU_SETCPUCLOCK, nBurnCPUSpeedAdjust != 0x0100 ? MF_CHECKED : MF_UNCHECKED);
	CreateCPUSpeedItem(nBurnCPUSpeedAdjust != 0x0100);

	if (bVidFullStretch) {
		var = MENU_STRETCH;
	} else {
		if (bVidCorrectAspect) {
			var = MENU_ASPECT;
		} else {
			var = MENU_NOSTRETCH;
		}
	}
	CheckMenuRadioItem(hMenu, MENU_NOSTRETCH, MENU_ASPECT, var, MF_BYCOMMAND);
	CheckMenuItem(hMenu, MENU_STRETCH, bVidFullStretch ? MF_CHECKED : MF_UNCHECKED);

	CheckMenuItem(hMenu, MENU_TRIPLE, bVidTripleBuffer ? MF_CHECKED : MF_UNCHECKED);

	CheckMenuItem(hMenu, MENU_DWMFIX, bVidDWMCore ? MF_CHECKED : MF_UNCHECKED);

	var = nVidSelect + MENU_BLITTER_1;
	CheckMenuRadioItem(hMenu, MENU_BLITTER_1, MENU_BLITTER_8, var, MF_BYCOMMAND);

	switch (nVidSelect) {
		case 0:
			if (bVidScanlines) {
				if (bVidScanHalf) {
					var = MENU_SCAN50;
				} else {
					var = MENU_BASIC_SCAN;
				}
			} else {
				var = MENU_BASIC_NORMAL;
			}
			CheckMenuRadioItem(hMenu, MENU_BASIC_NORMAL, MENU_SCAN50, var, MF_BYCOMMAND);
			CheckMenuItem(hMenu, MENU_BASIC_ROTSCAN, bVidScanRotate ? MF_CHECKED : MF_UNCHECKED);
			CheckMenuItem(hMenu, MENU_FORCE_FLIP, bVidForceFlip ? MF_CHECKED : MF_UNCHECKED);
			CheckMenuItem(hMenu, MENU_RES_ARCADE, bVidArcaderes ? MF_CHECKED : MF_UNCHECKED);
			break;
		case 1:
			CheckMenuItem(hMenu, MENU_BILINEAR, bVidBilinear ? MF_CHECKED : MF_UNCHECKED);
			CheckMenuItem(hMenu, MENU_ENHANCED_SCAN, bVidScanlines ? MF_CHECKED : MF_UNCHECKED);
			CheckMenuItem(hMenu, MENU_PHOSPHOR, bVidScanDelay ? MF_CHECKED : MF_UNCHECKED);

			CheckMenuItem(hMenu, MENU_PRESCALE, nVidBlitterOpt[nVidSelect] & 0x01000000 ? MF_CHECKED : MF_UNCHECKED);
			CheckMenuItem(hMenu, MENU_SOFTFX, nVidBlitterOpt[nVidSelect] & 0x02000000 ? MF_CHECKED : MF_UNCHECKED);

			var = ((unsigned long long)nVidBlitterOpt[nVidSelect] >> 32) + MENU_ENHANCED_SOFT_STRETCH;
			CheckMenuRadioItem(hMenu, MENU_ENHANCED_SOFT_STRETCH, MENU_ENHANCED_SOFT_STRETCH + 25, var, MF_BYCOMMAND);
			CheckMenuItem(hMenu, MENU_ENHANCED_SOFT_AUTOSIZE, (nVidBlitterOpt[nVidSelect] & 0x04000000) ? MF_CHECKED : MF_UNCHECKED);
			if (nVidBlitterOpt[nVidSelect] & 0x00100000) {
				var = MENU_3DPROJECTION;
			} else {
				if (nVidBlitterOpt[nVidSelect] & 0x00010000) {
					var = MENU_RGBEFFECTS;
				} else {
   					var = MENU_ENHANCED_NORMAL;
				}
			}
			CheckMenuRadioItem(hMenu, MENU_ENHANCED_NORMAL, MENU_ENHANCED_NORMAL, var, MF_BYCOMMAND);
			CheckMenuRadioItem(hMenu, MENU_RGBEFFECTS, MENU_3DPROJECTION, var, MF_BYCOMMAND);

			CheckMenuItem(hMenu, MENU_EFFECT_AUTO, (nVidBlitterOpt[nVidSelect] & 0x00020000) ? MF_CHECKED : MF_UNCHECKED);
			var = MENU_EFFECT_01 + (nVidBlitterOpt[nVidSelect] & 0x000000FF) - 8;
			CheckMenuRadioItem(hMenu, MENU_EFFECT_01, MENU_EFFECT_08, var, MF_BYCOMMAND);
			CheckMenuItem(hMenu, MENU_3DUSELIGHTING, nVidBlitterOpt[nVidSelect] & 0x00200000 ? MF_CHECKED : MF_UNCHECKED);
			CheckMenuItem(hMenu, MENU_ENHANCED_ROTSCAN, bVidScanRotate ? MF_CHECKED : MF_UNCHECKED);
			CheckMenuItem(hMenu, MENU_FORCE_16BIT, bVidForce16bit ? MF_CHECKED : MF_UNCHECKED);
			CheckMenuItem(hMenu, MENU_TEXTUREMANAGE, (nVidTransferMethod & 1) ? MF_CHECKED : MF_UNCHECKED);
			CheckMenuItem(hMenu, MENU_RES_ARCADE, bVidArcaderes ? MF_CHECKED : MF_UNCHECKED);
			break;
		case 2:
			var = (nVidBlitterOpt[nVidSelect] & 0xFF) + MENU_SOFTFX_SOFT_STRETCH;
			CheckMenuRadioItem(hMenu, MENU_SOFTFX_SOFT_STRETCH, MENU_SOFTFX_SOFT_STRETCH + 25, var, MF_BYCOMMAND);
			CheckMenuItem(hMenu, MENU_SOFTFX_SOFT_AUTOSIZE, (nVidBlitterOpt[nVidSelect] & 0x0100) ? MF_CHECKED : MF_UNCHECKED);
			CheckMenuItem(hMenu, MENU_SOFT_DIRECTACCESS, !(nVidBlitterOpt[nVidSelect] & 0x0200) ? MF_CHECKED : MF_UNCHECKED);
			break;
		case 3:
			var = ((nVidBlitterOpt[nVidSelect] >> 24) & 0x03) + MENU_DX9_POINT;
			CheckMenuRadioItem(hMenu, MENU_DX9_POINT, MENU_DX9_POINT + 2, var, MF_BYCOMMAND);
			CheckMenuItem(hMenu, MENU_EXP_SCAN, bVidScanlines ? MF_CHECKED : MF_UNCHECKED);

			var = MENU_DX9_CUBIC_CUSTOM;
			if (dVidCubicB > -0.001 && dVidCubicB <  0.001 && dVidCubicC > -0.001 && dVidCubicC <  0.001) {
				var = MENU_DX9_CUBIC_LIGHT;
			}
			if (dVidCubicB >  1.499 && dVidCubicB <  1.501 && dVidCubicC > -0.251 && dVidCubicC < -0.249) {
				var = MENU_DX9_CUBIC_NOTCH;
			}
			if (dVidCubicB >  0.999 && dVidCubicB <  1.001 && dVidCubicC > -0.001 && dVidCubicC <  0.001) {
				var = MENU_DX9_CUBIC_BSPLINE;
			}
			if (dVidCubicB >  0.333 && dVidCubicB <  0.334 && dVidCubicC >  0.333 && dVidCubicC <  0.334) {
				var = MENU_DX9_CUBIC_OPTIMAL;
			}
			if (dVidCubicB > -0.001 && dVidCubicB <  0.001 && dVidCubicC >  0.499 && dVidCubicC <  0.501) {
				var = MENU_DX9_CUBIC_CATMULL;
			}
			if (dVidCubicB > -0.001 && dVidCubicB <  0.001 && dVidCubicC >  0.999 && dVidCubicC <  1.001) {
				var = MENU_DX9_CUBIC_SHARP;
			}
			CheckMenuRadioItem(hMenu, MENU_DX9_CUBIC_LIGHT, MENU_DX9_CUBIC_BSPLINE + 8, var, MF_BYCOMMAND);

			CheckMenuItem(hMenu, MENU_DX9_FORCE_PS14, !(nVidBlitterOpt[nVidSelect] & (1 <<  9)) ? MF_CHECKED : MF_UNCHECKED);
			CheckMenuItem(hMenu, MENU_DX9_FPTERXTURES, (nVidBlitterOpt[nVidSelect] & (1 <<  8)) ? MF_CHECKED : MF_UNCHECKED);
			if (!(nVidBlitterOpt[nVidSelect] & (1 <<  9))) {
				var = MENU_DX9_CUBIC4;
			} else {
				var = ((nVidBlitterOpt[nVidSelect] >> 28) & 0x07) + MENU_DX9_CUBIC0;
			}
			CheckMenuRadioItem(hMenu, MENU_DX9_CUBIC0, MENU_DX9_CUBIC0 + 8, var, MF_BYCOMMAND);
			break;
		case 4:
			var = (nVidBlitterOpt[nVidSelect] & 0xFF) + MENU_DX9_ALT_SOFT_STRETCH;
			CheckMenuRadioItem(hMenu, MENU_DX9_ALT_SOFT_STRETCH, MENU_DX9_ALT_SOFT_STRETCH + 25, var, MF_BYCOMMAND);
			CheckMenuItem(hMenu, MENU_DX9_ALT_SOFT_AUTOSIZE, (nVidBlitterOpt[nVidSelect] & 0x0100) ? MF_CHECKED : MF_UNCHECKED);
			CheckMenuRadioItem(hMenu, MENU_DX9_ALT_POINT, MENU_DX9_ALT_POINT + 1, MENU_DX9_ALT_POINT + bVidDX9Bilinear, MF_BYCOMMAND);
			CheckMenuItem(hMenu, MENU_DX9_ALT_HARDWAREVERTEX, (bVidHardwareVertex) ? MF_CHECKED : MF_UNCHECKED);
			CheckMenuItem(hMenu, MENU_DX9_ALT_MOTIONBLUR, (bVidMotionBlur) ? MF_CHECKED : MF_UNCHECKED);
			break;
	}

	var = 0;
	if (nScreenSizeHor) {
		var = MENU_SINGLESIZESCREEN + nScreenSizeHor - 1;
		CreateOtherresItem(0);
	} else {
		for (int n = 0; n < 4; n++) {
			if (nVidHorWidth == VidPreset[n].nWidth && nVidHorHeight == VidPreset[n].nHeight) {
				var = MENU_RES_1 + n;
				break;
			}
		}
		if (var) {
			CreateOtherresItem(0);
		} else {
			var = MENU_RES_OTHER;
			CreateOtherresItem(1);
		}
		if (bVidArcaderesHor) {
			var = MENU_RES_ARCADE;
		}
	}
	CheckMenuRadioItem(hMenu, MENU_RES_1, MENU_QUADSIZESCREEN, var, MF_BYCOMMAND);

	var = 0;
	if (nScreenSizeVer) {
		var = MENU_SINGLESIZESCREEN_VERTICAL + nScreenSizeVer - 1;
		CreateOtherresItemVer(0);
	} else {
		for (int n = 0; n < 4; n++) {
			if (nVidVerWidth == VidPresetVer[n].nWidth && nVidVerHeight == VidPresetVer[n].nHeight) {
				var = MENU_RES_1_VERTICAL + n;
				break;
			}
		}
		if (var) {
			CreateOtherresItemVer(0);
		} else {
			var = MENU_RES_OTHER_VERTICAL;
			CreateOtherresItemVer(1);
		}
		if (bVidArcaderesVer) {
			var = MENU_RES_ARCADE_VERTICAL;
		}
	}
	CheckMenuRadioItem(hMenu, MENU_RES_1_VERTICAL, MENU_QUADSIZESCREEN_VERTICAL, var, MF_BYCOMMAND);

	if (nVidDepth == 16) {
		var = MENU_16;
	} else {
		if (nVidDepth == 24) {
			var = MENU_24;
		} else {
			var = MENU_32;
		}
	}
	CheckMenuRadioItem(hMenu, MENU_16, MENU_32, var, MF_BYCOMMAND);

	if (nGamma > 1.249 && nGamma < 1.251) {
		var = MENU_GAMMA_DARKER;
	} else {
		if (nGamma > 0.799 && nGamma < 0.801) {
			var = MENU_GAMMA_LIGHTER;
		} else {
			var = MENU_GAMMA_OTHER;
		}
	}
	if (var == MENU_GAMMA_OTHER) {
		CreateOtherGammaItem(1);
	} else {
		CreateOtherGammaItem(0);
	}
	if (!bDoGamma) {
		var = MENU_GAMMA_DO;
	}
	CheckMenuRadioItem(hMenu, MENU_GAMMA_DARKER, MENU_GAMMA_OTHER, var, MF_BYCOMMAND);
	CheckMenuItem(hMenu, MENU_GAMMA_DO, bDoGamma ? MF_CHECKED : MF_UNCHECKED);

	var = MENU_GAMMA_SOFTWARE_ONLY;
	if (bVidUseHardwareGamma) {
		if (bHardwareGammaOnly) {
			var = MENU_GAMMA_HARDWARE_ONLY;
		} else {
			var = MENU_GAMMA_USE_HARDWARE;
		}
	}
	CheckMenuRadioItem(hMenu, MENU_GAMMA_USE_HARDWARE, MENU_GAMMA_SOFTWARE_ONLY, var, MF_BYCOMMAND);
	
	CheckMenuItem(hMenu, MENU_AUTOSWITCHFULL, bVidAutoSwitchFull ? MF_CHECKED : MF_UNCHECKED);

	if (nVidTransferMethod == 0) {
		var = MENU_BASIC_VIDEOMEM;
	} else {
		if (nVidTransferMethod == -1) {
			var = MENU_BASIC_MEMAUTO;
		} else {
			var = MENU_BASIC_SYSMEM;
		}
	}
	CheckMenuRadioItem(hMenu, MENU_BASIC_MEMAUTO, MENU_BASIC_SYSMEM, var, MF_BYCOMMAND);
	
	if (nVidTransferMethod == 0) {
		var = MENU_SOFTFX_VIDEOMEM;
	} else {
		if (nVidTransferMethod == -1) {
			var = MENU_SOFTFX_MEMAUTO;
		} else {
			var = MENU_SOFTFX_SYSMEM;
		}
	}
	CheckMenuRadioItem(hMenu, MENU_SOFTFX_MEMAUTO, MENU_SOFTFX_SYSMEM, var, MF_BYCOMMAND);
	
	if (nWindowSize <= 4) {
		var = MENU_AUTOSIZE + nWindowSize;
	} else {
		var = MENU_MAXIMUMSIZEWINDOW;
	}
	CheckMenuRadioItem(hMenu, MENU_AUTOSIZE, MENU_MAXIMUMSIZEWINDOW, var, MF_BYCOMMAND);
	
	CheckMenuItem(hMenu, MENU_MONITORAUTOCHECK, bMonitorAutoCheck ? MF_CHECKED : MF_UNCHECKED);

	var = -1;
	if (nVidScrnAspectX == 4 && nVidScrnAspectY == 3) {
		var = MENU_ASPECTNORMAL;
	} else {
		if (nVidScrnAspectX == 5 && nVidScrnAspectY == 4) {
			var = MENU_ASPECTLCD;
		} else {
			if (nVidScrnAspectX == 16 && nVidScrnAspectY == 9) {
				var = MENU_ASPECTWIDE;
			} else {
				if (nVidScrnAspectX == 16 && nVidScrnAspectY == 10) {
					var = MENU_ASPECTWIDELCD;
				}
			}
		}
	}
	CheckMenuRadioItem(hMenu, MENU_ASPECTNORMAL, MENU_ASPECTWIDELCD, var, MF_BYCOMMAND);
	CheckMenuItem(hMenu, MENU_MONITORMIRRORVERT, (nVidRotationAdjust & 2) ? MF_CHECKED : MF_UNCHECKED);

	CheckMenuItem(hMenu, MENU_ROTATEVERTICAL, !(nVidRotationAdjust & 1) ? MF_CHECKED : MF_UNCHECKED);
	CheckMenuItem(hMenu, MENU_FORCE60HZ, bForce60Hz ? MF_CHECKED : MF_UNCHECKED);
	CheckMenuItem(hMenu, MENU_VIDEOVSYNC, bVidVSync ? MF_CHECKED : MF_UNCHECKED);
	CheckMenuItem(hMenu, MENU_AUTOFRAMESKIP, !bAlwaysDrawFrames ? MF_CHECKED : MF_UNCHECKED);
	
	var = nAudSelect + MENU_AUD_PLUGIN_1;
	CheckMenuRadioItem(hMenu, MENU_AUD_PLUGIN_1, MENU_AUD_PLUGIN_8, var, MF_BYCOMMAND);

	switch (nAudSelect) {
		case 0: {
			var = MENU_DSOUND_NOSOUND;
			if (nAudSampleRate[0] > 0) {
				if (nAudSampleRate[0] <= 11025) {
					var = MENU_DSOUND_11025;
				} else {
					if (nAudSampleRate[0] <= 22050) {
						var = MENU_DSOUND_22050;
					} else {
						if (nAudSampleRate[0] <= 44100) {
							var = MENU_DSOUND_44100;
						} else {
							var = MENU_DSOUND_48000;
						}
					}
				}
			}
			CheckMenuRadioItem(hMenu, MENU_DSOUND_NOSOUND, MENU_DSOUND_48000, var, MF_BYCOMMAND);
			CheckMenuItem(hMenu, MENU_DSOUND_BASS, nAudDSPModule[0] ? MF_CHECKED : MF_UNCHECKED);
			break;
		}
		
		case 1: {
			var = MENU_XAUDIO_NOSOUND;
			if (nAudSampleRate[1] > 0) {
				if (nAudSampleRate[1] <= 11025) {
					var = MENU_XAUDIO_11025;
				} else {
					if (nAudSampleRate[1] <= 22050) {
						var = MENU_XAUDIO_22050;
					} else {
						if (nAudSampleRate[1] <= 44100) {
							var = MENU_XAUDIO_44100;
						} else {
							var = MENU_XAUDIO_48000;
						}
					}
				}
			}
			CheckMenuRadioItem(hMenu, MENU_XAUDIO_NOSOUND, MENU_XAUDIO_48000, var, MF_BYCOMMAND);
			CheckMenuItem(hMenu, MENU_XAUDIO_BASS, (nAudDSPModule[1] & 1) ? MF_CHECKED : MF_UNCHECKED);
			CheckMenuItem(hMenu, MENU_XAUDIO_REVERB, (nAudDSPModule[1] & 2) ? MF_CHECKED : MF_UNCHECKED);
			break;
		}
	}

	var = MENU_INTERPOLATE_0 + nInterpolation;
	if (bDrvOkay) {
		if ((BurnDrvGetHardwareCode() & HARDWARE_PUBLIC_MASK) == HARDWARE_SNK_NEOGEO) {
			var = MENU_INTERPOLATE_0 + 1;
		}
	}
	CheckMenuRadioItem(hMenu, MENU_INTERPOLATE_0, MENU_INTERPOLATE_5, var, MF_BYCOMMAND);

	var = MENU_INTERPOLATE_FM_0 + nFMInterpolation;
	CheckMenuRadioItem(hMenu, MENU_INTERPOLATE_FM_0, MENU_INTERPOLATE_FM_5, var, MF_BYCOMMAND);

	CheckMenuItem(hMenu, MENU_AUTOPAUSE, bAutoPause ? MF_CHECKED : MF_UNCHECKED);
	CheckMenuItem(hMenu, MENU_PROCESSINPUT, !bAlwaysProcessKeyboardInput ? MF_CHECKED : MF_UNCHECKED);
	if (bAutoPause) {
		EnableMenuItem(hMenu, MENU_PROCESSINPUT, MF_GRAYED | MF_BYCOMMAND);
	} else {
		EnableMenuItem(hMenu, MENU_PROCESSINPUT, MF_ENABLED | MF_BYCOMMAND);
	}
	CheckMenuItem(hMenu, MENU_DISPLAYINDICATOR, nVidSDisplayStatus ? MF_CHECKED : MF_UNCHECKED);
	CheckMenuItem(hMenu, MENU_MODELESS, bModelessMenu ? MF_CHECKED : MF_UNCHECKED);
	CheckMenuItem(hMenu, MENU_NOCHANGENUMLOCK, bNoChangeNumLock ? MF_CHECKED : MF_UNCHECKED);
	CheckMenuItem(hMenu, MENU_SAVEHISCORES, EnableHiscores ? MF_CHECKED : MF_UNCHECKED);

	if (nAppThreadPriority == THREAD_PRIORITY_TIME_CRITICAL) {
		var = MENU_PRIORITY_REALTIME;
	} else {
		if (nAppThreadPriority == THREAD_PRIORITY_HIGHEST) {
			var = MENU_PRIORITY_HIGH;
		} else {
			if (nAppThreadPriority == THREAD_PRIORITY_ABOVE_NORMAL) {
				var = MENU_PRIORITY_ABOVE_NORMAL;
			} else {
				if (nAppThreadPriority == THREAD_PRIORITY_BELOW_NORMAL) {
					var = MENU_PRIORITY_BELOW_NORMAL;
				} else {
					if (nAppThreadPriority == THREAD_PRIORITY_LOWEST) {
						var = MENU_PRIORITY_LOW;
					} else {
						var = MENU_PRIORITY_NORMAL;
					}
				}
			}
		}
	}
	CheckMenuRadioItem(hMenu, MENU_PRIORITY_REALTIME, MENU_PRIORITY_LOW, var, MF_BYCOMMAND);
	CheckMenuItem(hMenu, MENU_SAVEGAMEINPUT, bSaveInputs ? MF_CHECKED : MF_UNCHECKED);

#if defined BUILD_A68K
	CheckMenuItem(hMenu, MENU_ASSEMBLYCORE, bBurnUseASMCPUEmulation ? MF_CHECKED : MF_UNCHECKED);
#else
	CheckMenuItem(hMenu, MENU_ASSEMBLYCORE, MF_UNCHECKED);
	EnableMenuItem(hMenu, MENU_ASSEMBLYCORE, MF_GRAYED  | MF_BYCOMMAND);
#endif

	var = MENU_ICONS_SIZE_16;
	switch (nIconsSize) {
		case ICON_16x16: var = MENU_ICONS_SIZE_16;	break;			
		case ICON_24x24: var = MENU_ICONS_SIZE_24;	break;			
		case ICON_32x32: var = MENU_ICONS_SIZE_32;	break;
	}
	CheckMenuRadioItem(hMenu, MENU_ICONS_SIZE_16, MENU_ICONS_SIZE_32, var, MF_BYCOMMAND);
	CheckMenuItem(hMenu, MENU_ENABLEICONS, bEnableIcons ? MF_CHECKED : MF_UNCHECKED);
	
	// Previous games list
	for (int i = 0; i < SHOW_PREV_GAMES; i++) {
		MENUITEMINFO menuItemInfo;
		TCHAR szText[256] = _T("");
		int OldDrvSelect = nBurnDrvActive;
		
		memset(&menuItemInfo, 0, sizeof(MENUITEMINFO));
		menuItemInfo.cbSize = sizeof(MENUITEMINFO);
		menuItemInfo.fType = MFT_STRING;
		menuItemInfo.fMask = MIIM_TYPE;			
		
		if (_tcslen(szPrevGames[i])) {
			for (unsigned int j = 0; j < nBurnDrvCount; j++) {
				nBurnDrvActive = j;
				if (!_tcsicmp(szPrevGames[i], BurnDrvGetText(DRV_NAME))) {
					_stprintf(szText, _T("%s\t%s"), BurnDrvGetText(DRV_FULLNAME), BurnDrvGetText(DRV_NAME));
					
					break;
				}
			}
			
			// Check for &s and change to &&
			TCHAR szText2[256] = _T("");
			TCHAR *Tokens = NULL;
			int NumTokens = 0;
			
			Tokens = _tcstok(szText, _T("&"));
			while (Tokens != NULL) {
				if (NumTokens) {
					_stprintf(szText2, _T("%s&&%s"), szText2, Tokens);
				} else {
					_stprintf(szText2, _T("%s"), Tokens);
				}
				
				Tokens = _tcstok(NULL, _T("&"));
				NumTokens++;
			}
			
			menuItemInfo.dwTypeData = szText2;
			menuItemInfo.cch = _tcslen(szText2);
			
			SetMenuItemInfo(hMenu, MENU_PREVIOUSGAMES1 + i, FALSE, &menuItemInfo);
			EnableMenuItem(hMenu, MENU_PREVIOUSGAMES1 + i, MF_ENABLED | MF_BYCOMMAND);
		} else {
			_tcscpy(szText, FBALoadStringEx(hAppInst, IDS_MENU_6, true));
			
			menuItemInfo.dwTypeData = szText;
			menuItemInfo.cch = _tcslen(szText);
			
			SetMenuItemInfo(hMenu, MENU_PREVIOUSGAMES1 + i, FALSE, &menuItemInfo);
			EnableMenuItem(hMenu, MENU_PREVIOUSGAMES1 + i, MF_GRAYED | MF_BYCOMMAND);
		}
		
		nBurnDrvActive = OldDrvSelect;		
	}
	
#if !defined BUILD_X86_ASM
	EnableMenuItem(hMenu, MENU_ENHANCED_SOFT_2XPM_LQ, MF_GRAYED  | MF_BYCOMMAND);
	EnableMenuItem(hMenu, MENU_ENHANCED_SOFT_2XPM_HQ, MF_GRAYED  | MF_BYCOMMAND);
	EnableMenuItem(hMenu, MENU_ENHANCED_SOFT_EAGLE, MF_GRAYED  | MF_BYCOMMAND);
	EnableMenuItem(hMenu, MENU_ENHANCED_SOFT_SUPEREAGLE, MF_GRAYED  | MF_BYCOMMAND);
	EnableMenuItem(hMenu, MENU_ENHANCED_SOFT_2XSAI, MF_GRAYED  | MF_BYCOMMAND);
	EnableMenuItem(hMenu, MENU_ENHANCED_SOFT_SUPER2XSAI, MF_GRAYED  | MF_BYCOMMAND);
	EnableMenuItem(hMenu, MENU_ENHANCED_SOFT_SUPEREAGLE_VBA, MF_GRAYED  | MF_BYCOMMAND);
	EnableMenuItem(hMenu, MENU_ENHANCED_SOFT_2XSAI_VBA, MF_GRAYED  | MF_BYCOMMAND);
	EnableMenuItem(hMenu, MENU_ENHANCED_SOFT_SUPER2XSAI_VBA, MF_GRAYED  | MF_BYCOMMAND);
	EnableMenuItem(hMenu, MENU_ENHANCED_SOFT_SUPERSCALE, MF_GRAYED  | MF_BYCOMMAND);
	EnableMenuItem(hMenu, MENU_ENHANCED_SOFT_SUPERSCALE75, MF_GRAYED  | MF_BYCOMMAND);
	EnableMenuItem(hMenu, MENU_ENHANCED_SOFT_HQ2X, MF_GRAYED  | MF_BYCOMMAND);
	EnableMenuItem(hMenu, MENU_ENHANCED_SOFT_HQ3X, MF_GRAYED  | MF_BYCOMMAND);
	EnableMenuItem(hMenu, MENU_ENHANCED_SOFT_HQ4X, MF_GRAYED  | MF_BYCOMMAND);
	
	EnableMenuItem(hMenu, MENU_SOFTFX_SOFT_2XPM_LQ, MF_GRAYED  | MF_BYCOMMAND);
	EnableMenuItem(hMenu, MENU_SOFTFX_SOFT_2XPM_HQ, MF_GRAYED  | MF_BYCOMMAND);
	EnableMenuItem(hMenu, MENU_SOFTFX_SOFT_EAGLE, MF_GRAYED  | MF_BYCOMMAND);
	EnableMenuItem(hMenu, MENU_SOFTFX_SOFT_SUPEREAGLE, MF_GRAYED  | MF_BYCOMMAND);
	EnableMenuItem(hMenu, MENU_SOFTFX_SOFT_2XSAI, MF_GRAYED  | MF_BYCOMMAND);
	EnableMenuItem(hMenu, MENU_SOFTFX_SOFT_SUPER2XSAI, MF_GRAYED  | MF_BYCOMMAND);
	EnableMenuItem(hMenu, MENU_SOFTFX_SOFT_SUPEREAGLE_VBA, MF_GRAYED  | MF_BYCOMMAND);
	EnableMenuItem(hMenu, MENU_SOFTFX_SOFT_2XSAI_VBA, MF_GRAYED  | MF_BYCOMMAND);
	EnableMenuItem(hMenu, MENU_SOFTFX_SOFT_SUPER2XSAI_VBA, MF_GRAYED  | MF_BYCOMMAND);
	EnableMenuItem(hMenu, MENU_SOFTFX_SOFT_SUPERSCALE, MF_GRAYED  | MF_BYCOMMAND);
	EnableMenuItem(hMenu, MENU_SOFTFX_SOFT_SUPERSCALE75, MF_GRAYED  | MF_BYCOMMAND);
	EnableMenuItem(hMenu, MENU_SOFTFX_SOFT_HQ2X, MF_GRAYED  | MF_BYCOMMAND);
	EnableMenuItem(hMenu, MENU_SOFTFX_SOFT_HQ3X, MF_GRAYED  | MF_BYCOMMAND);
	EnableMenuItem(hMenu, MENU_SOFTFX_SOFT_HQ4X, MF_GRAYED  | MF_BYCOMMAND);
	
	EnableMenuItem(hMenu, MENU_DX9_ALT_SOFT_2XPM_LQ, MF_GRAYED  | MF_BYCOMMAND);
	EnableMenuItem(hMenu, MENU_DX9_ALT_SOFT_2XPM_HQ, MF_GRAYED  | MF_BYCOMMAND);
	EnableMenuItem(hMenu, MENU_DX9_ALT_SOFT_EAGLE, MF_GRAYED  | MF_BYCOMMAND);
	EnableMenuItem(hMenu, MENU_DX9_ALT_SOFT_SUPEREAGLE, MF_GRAYED  | MF_BYCOMMAND);
	EnableMenuItem(hMenu, MENU_DX9_ALT_SOFT_2XSAI, MF_GRAYED  | MF_BYCOMMAND);
	EnableMenuItem(hMenu, MENU_DX9_ALT_SOFT_SUPER2XSAI, MF_GRAYED  | MF_BYCOMMAND);
	EnableMenuItem(hMenu, MENU_DX9_ALT_SOFT_SUPEREAGLE_VBA, MF_GRAYED  | MF_BYCOMMAND);
	EnableMenuItem(hMenu, MENU_DX9_ALT_SOFT_2XSAI_VBA, MF_GRAYED  | MF_BYCOMMAND);
	EnableMenuItem(hMenu, MENU_DX9_ALT_SOFT_SUPER2XSAI_VBA, MF_GRAYED  | MF_BYCOMMAND);
	EnableMenuItem(hMenu, MENU_DX9_ALT_SOFT_SUPERSCALE, MF_GRAYED  | MF_BYCOMMAND);
	EnableMenuItem(hMenu, MENU_DX9_ALT_SOFT_SUPERSCALE75, MF_GRAYED  | MF_BYCOMMAND);
	EnableMenuItem(hMenu, MENU_DX9_ALT_SOFT_HQ2X, MF_GRAYED  | MF_BYCOMMAND);
	EnableMenuItem(hMenu, MENU_DX9_ALT_SOFT_HQ3X, MF_GRAYED  | MF_BYCOMMAND);
	EnableMenuItem(hMenu, MENU_DX9_ALT_SOFT_HQ4X, MF_GRAYED  | MF_BYCOMMAND);
#endif

#if defined BUILD_X64_EXE
	EnableMenuItem(hMenu, MENU_BLITTER_2, MF_GRAYED  | MF_BYCOMMAND);
#endif
}

void MenuEnableItems()
{
	CreateArcaderesItem();
	CreatepresetResItems();
	
	if (hBlitterMenu[nVidSelect]) {
		MENUITEMINFO myMenuItemInfo;
		myMenuItemInfo.cbSize = sizeof(MENUITEMINFO);
		myMenuItemInfo.fMask = MIIM_SUBMENU | MIIM_STATE;
        	myMenuItemInfo.fState = MFS_ENABLED;
        	myMenuItemInfo.hSubMenu = GetSubMenu(hBlitterMenu[nVidSelect], 0);
		SetMenuItemInfo(GetSubMenu(hMenu, 1), 1, TRUE, &myMenuItemInfo);
	} else {
		MENUITEMINFO myMenuItemInfo;
		myMenuItemInfo.cbSize = sizeof(MENUITEMINFO);
		myMenuItemInfo.fMask = MIIM_SUBMENU | MIIM_STATE;
        	myMenuItemInfo.fState = MFS_GRAYED;
        	myMenuItemInfo.hSubMenu = NULL;
		SetMenuItemInfo(GetSubMenu(hMenu, 1), 1, TRUE, &myMenuItemInfo);
	}
	
	if (hBlitterMenu[nAudSelect]) {
		MENUITEMINFO myMenuItemInfo;
		myMenuItemInfo.cbSize = sizeof(MENUITEMINFO);
		myMenuItemInfo.fMask = MIIM_SUBMENU | MIIM_STATE;
        	myMenuItemInfo.fState = MFS_ENABLED;
        	myMenuItemInfo.hSubMenu = GetSubMenu(hAudioPluginMenu[nAudSelect], 0);
		SetMenuItemInfo(GetSubMenu(hMenu, 2), 1, TRUE, &myMenuItemInfo);
	} else {
		MENUITEMINFO myMenuItemInfo;
		myMenuItemInfo.cbSize = sizeof(MENUITEMINFO);
		myMenuItemInfo.fMask = MIIM_SUBMENU | MIIM_STATE;
        	myMenuItemInfo.fState = MFS_GRAYED;
        	myMenuItemInfo.hSubMenu = NULL;
		SetMenuItemInfo(GetSubMenu(hMenu, 2), 1, TRUE, &myMenuItemInfo);
	}

	// Auto-size image when software blitter effects anre enabled
	if ((nVidSelect == 1 && (nVidBlitterOpt[1] & 0x07000000) == 0x07000000) || (nVidSelect == 2 && nVidBlitterOpt[2] & 0x0100)) {
		EnableMenuItem(GetSubMenu(hMenu, 1),  2,	MF_GRAYED | MF_BYPOSITION);
		EnableMenuItem(GetSubMenu(hMenu, 1),  7,	MF_GRAYED | MF_BYPOSITION);
		EnableMenuItem(GetSubMenu(hMenu, 1), 10,	MF_GRAYED | MF_BYPOSITION);
	} else {
		EnableMenuItem(GetSubMenu(hMenu, 1),  2,	MF_ENABLED | MF_BYPOSITION);
		EnableMenuItem(GetSubMenu(hMenu, 1),  7,	MF_ENABLED | MF_BYPOSITION);
		EnableMenuItem(GetSubMenu(hMenu, 1), 10,	MF_ENABLED | MF_BYPOSITION);
	}
	EnableMenuItem(GetSubMenu(hMenu, 1), 11,		MF_ENABLED | MF_BYPOSITION);

#if 0
	if (nVidSelect == 3 && !(nVidBlitterOpt[3] & (1 <<  9))) {
		EnableMenuItem(hMenu, MENU_DX9_CUBIC0,		MF_GRAYED  | MF_BYCOMMAND);
		EnableMenuItem(hMenu, MENU_DX9_CUBIC1,		MF_GRAYED  | MF_BYCOMMAND);
		EnableMenuItem(hMenu, MENU_DX9_CUBIC2,		MF_GRAYED  | MF_BYCOMMAND);
		EnableMenuItem(hMenu, MENU_DX9_CUBIC3,		MF_GRAYED  | MF_BYCOMMAND);
		EnableMenuItem(hMenu, MENU_DX9_CUBIC4,		MF_GRAYED  | MF_BYCOMMAND);
	} else {
		EnableMenuItem(hMenu, MENU_DX9_CUBIC0,		MF_ENABLED | MF_BYCOMMAND);
		EnableMenuItem(hMenu, MENU_DX9_CUBIC1,		MF_ENABLED | MF_BYCOMMAND);
		EnableMenuItem(hMenu, MENU_DX9_CUBIC2,		MF_ENABLED | MF_BYCOMMAND);
		EnableMenuItem(hMenu, MENU_DX9_CUBIC3,		MF_ENABLED | MF_BYCOMMAND);
		EnableMenuItem(hMenu, MENU_DX9_CUBIC4,		MF_ENABLED | MF_BYCOMMAND);
	}
#endif

	if (nVidSelect == 3 && (!(nVidBlitterOpt[3] & (1 <<  9)) || (nVidBlitterOpt[nVidSelect] & (7 << 28)) == (4 << 28))) {
		EnableMenuItem(hMenu, MENU_DX9_CUBIC_BSPLINE,	MF_GRAYED  | MF_BYCOMMAND);
		EnableMenuItem(hMenu, MENU_DX9_CUBIC_NOTCH,		MF_GRAYED  | MF_BYCOMMAND);
		EnableMenuItem(hMenu, MENU_DX9_CUBIC_OPTIMAL,	MF_GRAYED  | MF_BYCOMMAND);
	} else {
		EnableMenuItem(hMenu, MENU_DX9_CUBIC_BSPLINE,	MF_ENABLED | MF_BYCOMMAND);
		EnableMenuItem(hMenu, MENU_DX9_CUBIC_NOTCH,		MF_ENABLED | MF_BYCOMMAND);
		EnableMenuItem(hMenu, MENU_DX9_CUBIC_OPTIMAL,	MF_ENABLED | MF_BYCOMMAND);
	}
	if (nVidSelect == 3) {
		EnableMenuItem(hMenu, MENU_24,	                MF_GRAYED  | MF_BYCOMMAND);
	}

	EnableMenuItem(hMenu, MENU_MODELESS,	                MF_ENABLED  | MF_BYCOMMAND);

#if defined _MSC_VER && defined BUILD_X86_ASM
	EnableMenuItem(hBlitterMenu[1], MENU_ENHANCED_SOFT_HQ3XS_VBA, MF_ENABLED | MF_BYCOMMAND);
	EnableMenuItem(hBlitterMenu[2], MENU_SOFTFX_SOFT_HQ3XS_VBA, MF_ENABLED | MF_BYCOMMAND);
	EnableMenuItem(hBlitterMenu[4], MENU_DX9_ALT_SOFT_HQ3XS_VBA, MF_ENABLED | MF_BYCOMMAND);
#endif

	if (bDrvOkay) {
		EnableMenuItem(hMenu, MENU_QUIT,				MF_ENABLED | MF_BYCOMMAND);
		EnableMenuItem(hMenu, MENU_INPUT,				MF_ENABLED | MF_BYCOMMAND);
		EnableMenuItem(hMenu, MENU_FORCE60HZ,				MF_GRAYED  | MF_BYCOMMAND);
		EnableMenuItem(hMenu, MENU_DSOUND_NOSOUND,				MF_GRAYED  | MF_BYCOMMAND);
		EnableMenuItem(hMenu, MENU_DSOUND_11025,				MF_GRAYED  | MF_BYCOMMAND);
		EnableMenuItem(hMenu, MENU_DSOUND_22050,				MF_GRAYED  | MF_BYCOMMAND);
		EnableMenuItem(hMenu, MENU_DSOUND_44100,				MF_GRAYED  | MF_BYCOMMAND);
		EnableMenuItem(hMenu, MENU_DSOUND_48000,				MF_GRAYED  | MF_BYCOMMAND);
		EnableMenuItem(hMenu, MENU_XAUDIO_NOSOUND,				MF_GRAYED  | MF_BYCOMMAND);
		EnableMenuItem(hMenu, MENU_XAUDIO_11025,				MF_GRAYED  | MF_BYCOMMAND);
		EnableMenuItem(hMenu, MENU_XAUDIO_22050,				MF_GRAYED  | MF_BYCOMMAND);
		EnableMenuItem(hMenu, MENU_XAUDIO_44100,				MF_GRAYED  | MF_BYCOMMAND);
		EnableMenuItem(hMenu, MENU_XAUDIO_48000,				MF_GRAYED  | MF_BYCOMMAND);
		EnableMenuItem(hMenu, MENU_INTERPOLATE_FM_0,	MF_GRAYED  | MF_BYCOMMAND);
		EnableMenuItem(hMenu, MENU_INTERPOLATE_FM_3,	MF_GRAYED  | MF_BYCOMMAND);
		EnableMenuItem(hMenu, MENU_FRAMES,				MF_GRAYED  | MF_BYCOMMAND);
		EnableMenuItem(hMenu, MENU_SAVESNAP,			MF_ENABLED | MF_BYCOMMAND);
		EnableMenuItem(hMenu, MENU_SAVEGAMEINPUTNOW,	MF_ENABLED | MF_BYCOMMAND);
		EnableMenuItem(hMenu, MENU_ASSEMBLYCORE,		MF_GRAYED  | MF_BYCOMMAND);
		EnableMenuItem(hMenu, MENU_SAVEHISCORES,		MF_GRAYED  | MF_BYCOMMAND);
		
		EnableMenuItem(hMenu, MENU_CDIMAGE,				MF_GRAYED | MF_BYCOMMAND);
		
		EnableMenuItem(hMenu, MENU_AUD_PLUGIN_1, MF_GRAYED  | MF_BYCOMMAND);
		EnableMenuItem(hMenu, MENU_AUD_PLUGIN_2, MF_GRAYED  | MF_BYCOMMAND);

		BurnDIPInfo bdi;
		if (BurnDrvGetDIPInfo(&bdi, 0) == 0) {
			EnableMenuItem(hMenu, MENU_DIPSW,			MF_ENABLED | MF_BYCOMMAND);
		} else {
			EnableMenuItem(hMenu, MENU_DIPSW,			MF_GRAYED | MF_BYCOMMAND);
		}

		EnableMenuItem(hMenu, MENU_INTERPOLATE_1,		MF_ENABLED | MF_BYCOMMAND);
		EnableMenuItem(hMenu, MENU_INTERPOLATE_3,		MF_ENABLED | MF_BYCOMMAND);

		EnableMenuItem(hMenu, MENU_MEMCARD_CREATE,		MF_GRAYED | MF_BYCOMMAND);
		EnableMenuItem(hMenu, MENU_MEMCARD_SELECT,		MF_GRAYED | MF_BYCOMMAND);
		EnableMenuItem(hMenu, MENU_MEMCARD_INSERT,		MF_GRAYED | MF_BYCOMMAND);
		EnableMenuItem(hMenu, MENU_MEMCARD_EJECT,		MF_GRAYED | MF_BYCOMMAND);

		if ((BurnDrvGetHardwareCode() & HARDWARE_PUBLIC_MASK) == HARDWARE_SNK_NEOGEO) {
			EnableMenuItem(hMenu, MENU_INTERPOLATE_1,				MF_GRAYED | MF_BYCOMMAND);
			EnableMenuItem(hMenu, MENU_INTERPOLATE_3,				MF_GRAYED | MF_BYCOMMAND);

			if (!kNetGame) {
				EnableMenuItem(hMenu, MENU_MEMCARD_CREATE,			MF_ENABLED | MF_BYCOMMAND);
				EnableMenuItem(hMenu, MENU_MEMCARD_SELECT,			MF_ENABLED | MF_BYCOMMAND);
				if (nMemoryCardStatus & 1) {
					if (nMemoryCardStatus & 2) {
						EnableMenuItem(hMenu, MENU_MEMCARD_EJECT,	MF_ENABLED | MF_BYCOMMAND);
					} else {
						EnableMenuItem(hMenu, MENU_MEMCARD_INSERT,	MF_ENABLED | MF_BYCOMMAND);
					}
				}
			}
		}

		if (kNetGame) {
			EnableMenuItem(hMenu, MENU_LOAD,			MF_GRAYED | MF_BYCOMMAND);
			EnableMenuItem(hMenu, MENU_STARTNET,		MF_GRAYED | MF_BYCOMMAND);
			EnableMenuItem(hMenu, MENU_EXIT,			MF_GRAYED | MF_BYCOMMAND);
			EnableMenuItem(hMenu, MENU_SETCPUCLOCK,		MF_GRAYED | MF_BYCOMMAND);
			EnableMenuItem(hMenu, MENU_RESETCPUCLOCK,	MF_GRAYED | MF_BYCOMMAND);
			EnableMenuItem(hMenu, MENU_PAUSE,			MF_GRAYED | MF_BYCOMMAND);
			EnableMenuItem(hMenu, MENU_ENABLECHEAT,		MF_GRAYED | MF_BYCOMMAND);
			EnableMenuItem(hMenu, MENU_SNAPFACT,		MF_GRAYED | MF_BYCOMMAND);
		} else {
			EnableMenuItem(hMenu, MENU_LOAD,			MF_ENABLED | MF_BYCOMMAND);
			EnableMenuItem(hMenu, MENU_STARTNET,		MF_ENABLED | MF_BYCOMMAND);
			EnableMenuItem(hMenu, MENU_EXIT,			MF_ENABLED | MF_BYCOMMAND);
			EnableMenuItem(hMenu, MENU_SETCPUCLOCK,		MF_ENABLED | MF_BYCOMMAND);
			EnableMenuItem(hMenu, MENU_RESETCPUCLOCK,	(nBurnCPUSpeedAdjust != 0x0100 ? MF_ENABLED : MF_GRAYED) | MF_BYCOMMAND);
			EnableMenuItem(hMenu, MENU_PAUSE,			MF_ENABLED | MF_BYCOMMAND);
			if (pCheatInfo) {
				EnableMenuItem(hMenu, MENU_ENABLECHEAT,	MF_ENABLED | MF_BYCOMMAND);
			}
			
			if (CheatSearchInit()) {
				EnableMenuItem(hMenu, MENU_CHEATSEARCH_START, MF_ENABLED | MF_BYCOMMAND);
			}

#if defined (FBA_DEBUG)
			extern UINT8 DebugCPU_SekInitted;
			if (DebugCPU_SekInitted) {
				EnableMenuItem(hMenu, MENU_DEBUG,		MF_ENABLED | MF_BYCOMMAND);
			} else {
				EnableMenuItem(hMenu, MENU_DEBUG,		MF_GRAYED | MF_BYCOMMAND);
			}
#else
			EnableMenuItem(hMenu, MENU_DEBUG,			MF_GRAYED  | MF_BYCOMMAND);
#endif

			if (BurnDrvGetPaletteEntries()) EnableMenuItem(hMenu, MENU_PALETTEVIEWER,	MF_ENABLED | MF_BYCOMMAND);
			EnableMenuItem(hMenu, MENU_SNAPFACT,		MF_ENABLED | MF_BYCOMMAND);
		}

		if (WaveLog) {
			EnableMenuItem(hMenu, MENU_WLOGEND,		MF_ENABLED | MF_BYCOMMAND);
			EnableMenuItem(hMenu, MENU_WLOGSTART,	MF_GRAYED  | MF_BYCOMMAND);
		} else {
			EnableMenuItem(hMenu, MENU_WLOGEND,		MF_GRAYED  | MF_BYCOMMAND);
			EnableMenuItem(hMenu, MENU_WLOGSTART,	MF_ENABLED | MF_BYCOMMAND);
		}

		if (nReplayStatus) {
			EnableMenuItem(hMenu, MENU_STOPREPLAY,				MF_ENABLED | MF_BYCOMMAND);
			EnableMenuItem(hMenu, MENU_STARTRECORD,				MF_GRAYED  | MF_BYCOMMAND);
			EnableMenuItem(hMenu, MENU_STARTREPLAY,				MF_GRAYED  | MF_BYCOMMAND);
			EnableMenuItem(hMenu, MENU_STATE_LOAD_SLOT,			MF_GRAYED  | MF_BYCOMMAND);
			EnableMenuItem(hMenu, MENU_STATE_LOAD_DIALOG,		MF_GRAYED  | MF_BYCOMMAND);
		} else {
			EnableMenuItem(hMenu, MENU_STOPREPLAY,				MF_GRAYED  | MF_BYCOMMAND);

			if (kNetGame) {
				EnableMenuItem(hMenu, MENU_STARTRECORD,			MF_ENABLED | MF_BYCOMMAND);
				EnableMenuItem(hMenu, MENU_STARTREPLAY,			MF_GRAYED  | MF_BYCOMMAND);
				EnableMenuItem(hMenu, MENU_STATE_SAVE_SLOT,		MF_ENABLED | MF_BYCOMMAND);
				EnableMenuItem(hMenu, MENU_STATE_SAVE_DIALOG,	MF_ENABLED | MF_BYCOMMAND);
				EnableMenuItem(hMenu, MENU_STATE_LOAD_SLOT,		MF_GRAYED  | MF_BYCOMMAND);
				EnableMenuItem(hMenu, MENU_STATE_LOAD_DIALOG,	MF_GRAYED  | MF_BYCOMMAND);
			} else {
				EnableMenuItem(hMenu, MENU_STARTRECORD,			MF_ENABLED | MF_BYCOMMAND);
				EnableMenuItem(hMenu, MENU_STARTREPLAY,			MF_ENABLED | MF_BYCOMMAND);
				EnableMenuItem(hMenu, MENU_STATE_SAVE_SLOT,		MF_ENABLED | MF_BYCOMMAND);
				EnableMenuItem(hMenu, MENU_STATE_SAVE_DIALOG,	MF_ENABLED | MF_BYCOMMAND);
				EnableMenuItem(hMenu, MENU_STATE_LOAD_SLOT,		MF_ENABLED | MF_BYCOMMAND);
				EnableMenuItem(hMenu, MENU_STATE_LOAD_DIALOG,	MF_ENABLED | MF_BYCOMMAND);
			}
		}

	} else {
		bAltPause = 0;

		EnableMenuItem(hMenu, MENU_LOAD,				MF_ENABLED | MF_BYCOMMAND);
		EnableMenuItem(hMenu, MENU_MEMCARD_CREATE,		MF_GRAYED  | MF_BYCOMMAND);
		EnableMenuItem(hMenu, MENU_MEMCARD_SELECT,		MF_GRAYED  | MF_BYCOMMAND);
		EnableMenuItem(hMenu, MENU_MEMCARD_INSERT,		MF_GRAYED  | MF_BYCOMMAND);
		EnableMenuItem(hMenu, MENU_MEMCARD_EJECT,		MF_GRAYED  | MF_BYCOMMAND);
		EnableMenuItem(hMenu, MENU_STARTNET,			MF_ENABLED | MF_BYCOMMAND);
		EnableMenuItem(hMenu, MENU_STARTREPLAY,			MF_ENABLED | MF_BYCOMMAND);
		EnableMenuItem(hMenu, MENU_STARTRECORD,			MF_GRAYED  | MF_BYCOMMAND);
		EnableMenuItem(hMenu, MENU_STOPREPLAY,			MF_GRAYED  | MF_BYCOMMAND);
		EnableMenuItem(hMenu, MENU_QUIT,				MF_GRAYED  | MF_BYCOMMAND);
		EnableMenuItem(hMenu, MENU_EXIT,				MF_ENABLED | MF_BYCOMMAND);
		EnableMenuItem(hMenu, MENU_INPUT,				MF_GRAYED  | MF_BYCOMMAND);
		EnableMenuItem(hMenu, MENU_SETCPUCLOCK,			MF_GRAYED  | MF_BYCOMMAND);
		EnableMenuItem(hMenu, MENU_RESETCPUCLOCK,		MF_GRAYED  | MF_BYCOMMAND);
		EnableMenuItem(hMenu, MENU_PAUSE,				MF_GRAYED  | MF_BYCOMMAND);
		EnableMenuItem(hMenu, MENU_FORCE60HZ,			MF_ENABLED | MF_BYCOMMAND);
		EnableMenuItem(hMenu, MENU_DIPSW,				MF_GRAYED  | MF_BYCOMMAND);
		EnableMenuItem(hMenu, MENU_ENABLECHEAT,			MF_GRAYED  | MF_BYCOMMAND);
		EnableMenuItem(hMenu, MENU_DEBUG,				MF_GRAYED  | MF_BYCOMMAND);
		EnableMenuItem(hMenu, MENU_ASSEMBLYCORE,		MF_ENABLED | MF_BYCOMMAND);
		EnableMenuItem(hMenu, MENU_SAVEHISCORES,		MF_ENABLED  | MF_BYCOMMAND);
		EnableMenuItem(hMenu, MENU_STATE_LOAD_SLOT,		MF_GRAYED  | MF_BYCOMMAND);
		EnableMenuItem(hMenu, MENU_STATE_LOAD_DIALOG,	MF_ENABLED | MF_BYCOMMAND);
		EnableMenuItem(hMenu, MENU_STATE_SAVE_SLOT,		MF_GRAYED  | MF_BYCOMMAND);
		EnableMenuItem(hMenu, MENU_STATE_SAVE_DIALOG,	MF_GRAYED  | MF_BYCOMMAND);
		EnableMenuItem(hMenu, MENU_FRAMES,				MF_ENABLED | MF_BYCOMMAND);
		EnableMenuItem(hMenu, MENU_DSOUND_NOSOUND,				MF_ENABLED | MF_BYCOMMAND);
		EnableMenuItem(hMenu, MENU_DSOUND_11025,				MF_ENABLED | MF_BYCOMMAND);
		EnableMenuItem(hMenu, MENU_DSOUND_22050,				MF_ENABLED | MF_BYCOMMAND);
		EnableMenuItem(hMenu, MENU_DSOUND_44100,				MF_ENABLED | MF_BYCOMMAND);
		EnableMenuItem(hMenu, MENU_DSOUND_48000,				MF_ENABLED | MF_BYCOMMAND);
		EnableMenuItem(hMenu, MENU_XAUDIO_NOSOUND,				MF_ENABLED | MF_BYCOMMAND);
		EnableMenuItem(hMenu, MENU_XAUDIO_11025,				MF_ENABLED | MF_BYCOMMAND);
		EnableMenuItem(hMenu, MENU_XAUDIO_22050,				MF_ENABLED | MF_BYCOMMAND);
		EnableMenuItem(hMenu, MENU_XAUDIO_44100,				MF_ENABLED | MF_BYCOMMAND);
		EnableMenuItem(hMenu, MENU_XAUDIO_48000,				MF_ENABLED | MF_BYCOMMAND);
//		EnableMenuItem(hMenu, MENU_INTERPOLATE_1,		MF_ENABLED | MF_BYCOMMAND);
//		EnableMenuItem(hMenu, MENU_INTERPOLATE_3,		MF_ENABLED | MF_BYCOMMAND);
		EnableMenuItem(hMenu, MENU_WLOGSTART,			MF_GRAYED  | MF_BYCOMMAND);
		EnableMenuItem(hMenu, MENU_INTERPOLATE_FM_0,	MF_ENABLED | MF_BYCOMMAND);
		EnableMenuItem(hMenu, MENU_INTERPOLATE_FM_3,	MF_ENABLED | MF_BYCOMMAND);
		EnableMenuItem(hMenu, MENU_WLOGEND,				MF_GRAYED  | MF_BYCOMMAND);
		EnableMenuItem(hMenu, MENU_SAVESNAP,			MF_GRAYED  | MF_BYCOMMAND);
		EnableMenuItem(hMenu, MENU_SNAPFACT,			MF_GRAYED  | MF_BYCOMMAND);
		EnableMenuItem(hMenu, MENU_PALETTEVIEWER,		MF_GRAYED | MF_BYCOMMAND);
		EnableMenuItem(hMenu, MENU_SAVEGAMEINPUTNOW,		MF_GRAYED  | MF_BYCOMMAND);
		
		EnableMenuItem(hMenu, MENU_CDIMAGE,				MF_ENABLED | MF_BYCOMMAND);
		
		EnableMenuItem(hMenu, MENU_AUD_PLUGIN_1, 		MF_ENABLED  | MF_BYCOMMAND);
		EnableMenuItem(hMenu, MENU_AUD_PLUGIN_2,		 MF_ENABLED  | MF_BYCOMMAND);
	}
}
