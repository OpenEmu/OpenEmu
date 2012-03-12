// Additions to the Cygwin/MinGW win32 headers
// ***********************************************************************************************************//
// Updated August 18, 2008 - Made adjustmens for newer versions of MinGW Win32 API definitions (CaptainCPS-X)   //
// ***********************************************************************************************************//
#include <w32api.h>

#undef NM_FIRST
#define NM_FIRST				(0U - 0U)

#if (_WIN32_IE >= 0x0300)
 #define NMLVKEYDOWN			LV_KEYDOWN
#endif

#ifndef MNS_NOCHECK
#define MNS_NOCHECK         	(0x80000000)
#endif

#ifndef MNS_MODELESS
#define MNS_MODELESS        	(0x40000000)
#endif

#ifndef MNS_DRAGDROP
#define MNS_DRAGDROP        	(0x20000000)
#endif

#ifndef MNS_AUTODISMISS
#define MNS_AUTODISMISS     	(0x10000000)
#endif

#ifndef MNS_NOTIFYBYPOS
#define MNS_NOTIFYBYPOS     	(0x08000000)
#endif

#ifndef MNS_CHECKORBMP
#define MNS_CHECKORBMP      	(0x04000000)
#endif

//#define MIM_MAXHEIGHT			(0x00000001)
//#define MIM_BACKGROUND		(0x00000002)
//#define MIM_HELPID			(0x00000004)
//#define MIM_MENUDATA			(0x00000008)
//#define MIM_STYLE				(0x00000010)
//#define MIM_APPLYTOSUBMENUS	(0x80000000)

#ifndef WM_UNINITMENUPOPUP
#define WM_UNINITMENUPOPUP		(0x0125)
#endif

#define ENM_SCROLLEVENTS		(8)

// Rich edit
#define ST_DEFAULT				(0)
#define ST_KEEPUNDO				(1)
#define ST_SELECTION			(2)

#if __W32API_MAJOR_VERSION >=3 && __W32API_MINOR_VERSION < 8
typedef struct _settextex
{
	DWORD	flags;
	UINT	codepage;
} SETTEXTEX;
#endif

// Macro used for handling Window Messages
#if not defined HANDLE_WM_DISPLAYCHANGE
 #define HANDLE_WM_DISPLAYCHANGE(hwnd, wParam, lParam, fn)		((fn)((hwnd), (UINT)(wParam), (UINT)LOWORD(lParam), (UINT)HIWORD(wParam)), 0L)
#endif

#if _WIN32_WINNT < 0x0500
 #define VK_OEM_PLUS	0xBB
 #define VK_OEM_COMMA	0xBC
 #define VK_OEM_MINUS	0xBD
 #define VK_OEM_PERIOD	0xBE
#endif

#ifndef LVM_SORTITEMSEX
#define LVM_SORTITEMSEX          (LVM_FIRST + 81)
#endif

#ifndef ListView_SortItemsEx
#define ListView_SortItemsEx(hwndLV, _pfnCompare, _lPrm) \
  (BOOL)SNDMSG((hwndLV), LVM_SORTITEMSEX, (WPARAM)(LPARAM)(_lPrm), (LPARAM)(PFNLVCOMPARE)(_pfnCompare))
#endif

// prevent some warnings on some header defines  (warning: right-hand operand of comma has no effect)
#undef HANDLE_WM_PAINT
#define HANDLE_WM_PAINT(hwnd,wParam,lParam,fn) ((fn)(hwnd))
#undef HANDLE_WM_INITMENUPOPUP
#define HANDLE_WM_INITMENUPOPUP(hwnd,wParam,lParam,fn) ((fn)((hwnd),(HMENU)(wParam),(UINT)LOWORD(lParam),(BOOL)HIWORD(lParam)))
