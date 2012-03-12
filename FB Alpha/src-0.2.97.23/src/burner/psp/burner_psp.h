
#include <pspkernel.h>
#include <pspdebug.h>
#include <pspctrl.h>
#include <stdio.h>
#include <stdlib.h>
//#include <unistd.h>
//#include <fcntl.h>
#include <string.h>
#include <math.h>


//use this define to enable crappy logging functions
#define logging_build

#ifndef _WIN32
typedef struct tagRECT {
	int left;
	int top;
	int right;
	int bottom;
} RECT,*PRECT,*LPRECT;
typedef const RECT *LPCRECT;
#endif


#ifndef MAX_PATH
#define MAX_PATH 511
#endif
#define stricmp strcasecmp

#ifdef	__GNU_C__
#undef	_strncmpi
#define	_strncmpi strncasecmp
#undef	_strnicmp
#define	_strnicmp strncasecmp
#endif

#ifndef __cdecl
#define	__cdecl
#endif

//main.cpp
int SetBurnHighCol(int nDepth);
extern int nAppVirtualFps;
extern bool bRunPause;
extern bool bAlwaysProcessKeyboardInput;
TCHAR* ANSIToTCHAR(const char* pszInString, TCHAR* pszOutString, int nOutSize);
char* TCHARToANSI(const TCHAR* pszInString, char* pszOutString, int nOutSize);
bool AppProcessKeyboardInput();
char* DecorateGameName(unsigned int nBurnDrv);


//gui_menu.cpp
int init_guimenu();
void exit_guimenu();

//config.cpp
int ConfigAppLoad();	
int ConfigAppSave();
char *LabelCheck(char* s, char* szLabel);


// drv.cpp
extern int bDrvOkay; // 1 if the Driver has been initted okay, and it's okay to use the BurnDrv functions
extern char szAppRomPaths[DIRS_MAX][MAX_PATH];
int DrvInit(int nDrvNum, bool bRestore);
int DrvInitCallback(); // Used when Burn library needs to load a game. DrvInit(nBurnSelect, false)
int DrvExit();
int ProgressUpdateBurner(double dProgress, const TCHAR* pszText, bool bAbs);
int AppError(TCHAR* szText, int bWarning);

//run.cpp
extern int RunMessageLoop();
extern int RunReset();

//inpdipsw.cpp
void InpDIPSWResetDIPs();

//interface/inp_interface.cpp
int InputInit();
int InputExit();
int InputMake(bool bCopy);

#ifdef logging_build
void log_printf(const char *format, ...);
#endif
//TODO:
#define szAppBurnVer 123142

