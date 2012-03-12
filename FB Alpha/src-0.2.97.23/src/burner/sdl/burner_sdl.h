#include "SDL.h"

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



//config.cpp
int ConfigAppLoad();	
int ConfigAppSave();

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

//TODO:
#define szAppBurnVer 1

class StringSet {
public:
	TCHAR* szText;
	int nLen;
	// printf function to add text to the Bzip string
	int __cdecl Add(TCHAR* szFormat, ...);
	int Reset();
	StringSet();
	~StringSet();
};

