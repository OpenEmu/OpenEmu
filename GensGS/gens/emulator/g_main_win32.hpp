/**
 * Gens: Main loop. (Win32-specific code)
 */

#ifndef G_MAIN_WIN32_HPP
#define G_MAIN_WIN32_HPP

#ifdef __cplusplus
extern "C" {
#endif

#define GENS_DIR_SEPARATOR_STR "\\"
#define GENS_DIR_SEPARATOR_CHR '\\'

#include <windows.h>

// Win32 instance
extern HINSTANCE ghInstance;

// Windows version
extern OSVERSIONINFO winVersion;

// If extended Common Controls are enabled, this is set to non-zero.
extern int win32_CommCtrlEx;

// Set actual window size.
void Win32_setActualWindowSize(HWND hWnd, const int reqW, const int reqH);

// Fonts
extern HFONT fntMain;
extern HFONT fntTitle;

void Get_Save_Path(char *buf, size_t n);
void Create_Save_Directory(const char *dir);

// TODO: Move this stuff to GensUI.
void Win32_centerOnGensWindow(HWND hWnd);
void Win32_clientResize(HWND hWnd, int width, int height);

int PASCAL WinMain(HINSTANCE hInst, HINSTANCE hPrevInst, LPSTR lpCmdLine, int nCmdShow);

#ifdef __cplusplus
}
#endif

#endif /* G_MAIN_WIN32_HPP */
