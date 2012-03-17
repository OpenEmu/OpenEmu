/***************************************************************************
 * Gens: (Win32) Zip File Selection Dialog.                                *
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

#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>

#include "zip_select_dialog.hpp"
#include "gens/gens_window.hpp"

#include "emulator/g_main.hpp"

// Gens Win32 resources
#include "ui/win32/resource.h"

// Win32 common controls
#include <commctrl.h>

#include <windowsx.h>

#include <string>
#include <list>
using std::string;
using std::list;


static inline int getCurListItemData(HWND hWndDlg, int nIDDlgItem);
static LRESULT CALLBACK Zip_Select_Dialog_DlgProc(HWND hWndDlg, UINT message, WPARAM wParam, LPARAM lParam);


ZipSelectDialog::ZipSelectDialog(HWND parent)
{
	m_Parent = parent;
}

ZipSelectDialog::~ZipSelectDialog()
{
}


/**
 * getFile(): Opens the Zip File Selection Dialog.
 */
CompressedFile* ZipSelectDialog::getFile(list<CompressedFile>* lst)
{
	if (!lst)
	{
		// NULL list pointer passed. Don't do anything.
		return NULL;
	}
	
	m_fileList = lst;
	
	CompressedFile* file;
	file = reinterpret_cast<CompressedFile*>(DialogBoxParam(
				  ghInstance, MAKEINTRESOURCE(IDD_ZIPSELECT),
				  m_Parent,
				  reinterpret_cast<DLGPROC>(Zip_Select_Dialog_DlgProc),
				  reinterpret_cast<LPARAM>(this)));
	
	// File was selected.
	return file;
}


static LRESULT CALLBACK Zip_Select_Dialog_DlgProc(HWND hWndDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
	switch (message)
	{
		case WM_INITDIALOG:
			reinterpret_cast<ZipSelectDialog*>(lParam)->init(hWndDlg);
			return TRUE;
		
		case WM_COMMAND:
			switch (LOWORD(wParam))
			{
				case IDOK:
					EndDialog(hWndDlg, getCurListItemData(hWndDlg, IDC_ZIPSELECT_LSTFILES));
					return TRUE;
				case IDCANCEL:
					EndDialog(hWndDlg, NULL);
					return TRUE;
				case IDC_ZIPSELECT_LSTFILES:
					if (HIWORD(wParam) == LBN_DBLCLK)
					{
						// Listbox double-click. Interpret this as "OK".
						EndDialog(hWndDlg, getCurListItemData(hWndDlg, IDC_ZIPSELECT_LSTFILES));
						return TRUE;
					}
			}
			break;
	}
	
	return FALSE;
}


/**
 * init(): Initialize the Zip Select dialog.
 * @param hWndDlg hWnd of the Zip Select dialog.
 */
void ZipSelectDialog::init(HWND hWndDlg)
{
	Win32_centerOnGensWindow(hWndDlg);
	
	// Get the listbox hWnd.
	HWND lstFiles = GetDlgItem(hWndDlg, IDC_ZIPSELECT_LSTFILES);
	
	// Reserve space in the listbox for the file listing.
	// Assuming maximum of 128 bytes per filename.
	SendMessage(lstFiles, LB_INITSTORAGE, static_cast<WPARAM>(m_fileList->size()), 128);
	
	// Add all strings.
	list<CompressedFile>::iterator lstIter;
	int index;
	for (lstIter = m_fileList->begin(); lstIter != m_fileList->end(); lstIter++)
	{
		index = ListBox_AddString(lstFiles, (*lstIter).filename.c_str());
		ListBox_SetItemData(lstFiles, index, &(*lstIter));
	}
	
	// Select the first item by default.
	ListBox_SetCurSel(lstFiles, 0);
}


/**
 * getCurListItemData(): Gets the data of the selected list item.
 * @param hWndDlg Dialog containing the listbox.
 * @param nIDDlgItem ID of the listbox.
 * @return Data of the selected list item. (-1 if nothing is selected.)
 */
static inline int getCurListItemData(HWND hWndDlg, int nIDDlgItem)
{
	HWND lstBox = GetDlgItem(hWndDlg, nIDDlgItem);
	int index = ListBox_GetCurSel(lstBox);
	return ListBox_GetItemData(lstBox, index);
}
