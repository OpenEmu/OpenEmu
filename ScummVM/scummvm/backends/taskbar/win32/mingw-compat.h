/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

// TODO: Remove header when the latest changes to the Windows SDK have been integrated into MingW
//       For reference, the interface definitions here are imported the SDK headers and from the
//       EcWin7 project (https://code.google.com/p/dukto/)

#ifndef BACKEND_WIN32_TASKBAR_MINGW_H
#define BACKEND_WIN32_TASKBAR_MINGW_H

#if defined(WIN32)
#if defined(__GNUC__)
#ifdef __MINGW32__

#ifdef _WIN32_WINNT
	#undef _WIN32_WINNT
#endif
#define _WIN32_WINNT 0x0501
#include <windows.h>
#include <commctrl.h>
#include <initguid.h>
#include <shlwapi.h>
#include <shlguid.h>
#define CMIC_MASK_ASYNCOK SEE_MASK_ASYNCOK

extern const GUID CLSID_ShellLink;

// Shard enumeration value
#define SHARD_LINK 0x00000006

// Taskbar GUID definitions
DEFINE_GUID(CLSID_TaskbarList,0x56fdf344,0xfd6d,0x11d0,0x95,0x8a,0x0,0x60,0x97,0xc9,0xa0,0x90);
DEFINE_GUID(IID_ITaskbarList3,0xea1afb91,0x9e28,0x4b86,0x90,0xE9,0x9e,0x9f,0x8a,0x5e,0xef,0xaf);
DEFINE_GUID(IID_IPropertyStore,0x886d8eeb,0x8cf2,0x4446,0x8d,0x02,0xcd,0xba,0x1d,0xbd,0xcf,0x99);

// Property key
typedef struct _tagpropertykey {
	GUID fmtid;
	DWORD pid;
} PROPERTYKEY;

#define REFPROPERTYKEY const PROPERTYKEY &

typedef struct tagPROPVARIANT PROPVARIANT;
#define REFPROPVARIANT const PROPVARIANT &

// Property store
DECLARE_INTERFACE_(IPropertyStore, IUnknown) {
	STDMETHOD (GetCount) (DWORD *cProps) PURE;
	STDMETHOD (GetAt) (DWORD iProp, PROPERTYKEY *pkey) PURE;
	STDMETHOD (GetValue) (REFPROPERTYKEY key, PROPVARIANT *pv) PURE;
	STDMETHOD (SetValue) (REFPROPERTYKEY key, REFPROPVARIANT propvar) PURE;
	STDMETHOD (Commit) (void) PURE;

private:
	~IPropertyStore();
};
typedef IPropertyStore *LPIPropertyStore;

// Mingw-specific defines for taskbar integration
typedef enum THUMBBUTTONMASK {
	THB_BITMAP = 0x1,
	THB_ICON = 0x2,
	THB_TOOLTIP = 0x4,
	THB_FLAGS = 0x8
} THUMBBUTTONMASK;

typedef enum THUMBBUTTONFLAGS {
	THBF_ENABLED = 0,
	THBF_DISABLED = 0x1,
	THBF_DISMISSONCLICK = 0x2,
	THBF_NOBACKGROUND = 0x4,
	THBF_HIDDEN = 0x8,
	THBF_NONINTERACTIVE = 0x10
} THUMBBUTTONFLAGS;

typedef struct THUMBBUTTON {
	THUMBBUTTONMASK dwMask;
	UINT iId;
	UINT iBitmap;
	HICON hIcon;
	WCHAR szTip[260];
	THUMBBUTTONFLAGS dwFlags;
} THUMBBUTTON;
typedef struct THUMBBUTTON *LPTHUMBBUTTON;

typedef enum TBPFLAG {
	TBPF_NOPROGRESS = 0,
	TBPF_INDETERMINATE = 0x1,
	TBPF_NORMAL = 0x2,
	TBPF_ERROR = 0x4,
	TBPF_PAUSED = 0x8
} TBPFLAG;

// Taskbar interface
DECLARE_INTERFACE_(ITaskbarList3, IUnknown) {
	// IUnknown
	STDMETHOD(QueryInterface) (THIS_ REFIID riid, void **ppv) PURE;
	STDMETHOD_(ULONG,AddRef) (THIS) PURE;
	STDMETHOD_(ULONG,Release) (THIS) PURE;
	// ITaskbarList
	STDMETHOD(HrInit) (THIS) PURE;
	STDMETHOD(AddTab) (THIS_ HWND hwnd) PURE;
	STDMETHOD(DeleteTab) (THIS_ HWND hwnd) PURE;
	STDMETHOD(ActivateTab) (THIS_ HWND hwnd) PURE;
	STDMETHOD(SetActiveAlt) (THIS_ HWND hwnd) PURE;
	STDMETHOD (MarkFullscreenWindow) (THIS_ HWND hwnd, int fFullscreen) PURE;
	// ITaskbarList3
	STDMETHOD (SetProgressValue) (THIS_ HWND hwnd, ULONGLONG ullCompleted, ULONGLONG ullTotal) PURE;
	STDMETHOD (SetProgressState) (THIS_ HWND hwnd, TBPFLAG tbpFlags) PURE;
	STDMETHOD (RegisterTab) (THIS_ HWND hwndTab, HWND hwndMDI) PURE;
	STDMETHOD (UnregisterTab) (THIS_ HWND hwndTab) PURE;
	STDMETHOD (SetTabOrder) (THIS_ HWND hwndTab, HWND hwndInsertBefore) PURE;
	STDMETHOD (SetTabActive) (THIS_ HWND hwndTab, HWND hwndMDI, DWORD dwReserved) PURE;
	STDMETHOD (ThumbBarAddButtons) (THIS_ HWND hwnd, UINT cButtons, LPTHUMBBUTTON pButton) PURE;
	STDMETHOD (ThumbBarUpdateButtons) (THIS_ HWND hwnd, UINT cButtons, LPTHUMBBUTTON pButton) PURE;
	STDMETHOD (ThumbBarSetImageList) (THIS_ HWND hwnd, HIMAGELIST himl) PURE;
	STDMETHOD (SetOverlayIcon) (THIS_ HWND hwnd, HICON hIcon, LPCWSTR pszDescription) PURE;
	STDMETHOD (SetThumbnailTooltip) (THIS_ HWND hwnd, LPCWSTR pszTip) PURE;
	STDMETHOD (SetThumbnailClip) (THIS_ HWND hwnd, RECT *prcClip) PURE;

private:
	~ITaskbarList3();
};

typedef ITaskbarList3 *LPITaskbarList3;

#endif // __MINGW32__
#endif // __GNUC__
#endif // WIN32

#endif // BACKEND_WIN32_TASKBAR_MINGW_H
