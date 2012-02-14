//
// Copyright (c) 2004 K. Wilkins
//
// This software is provided 'as-is', without any express or implied warranty.
// In no event will the authors be held liable for any damages arising from
// the use of this software.
//
// Permission is granted to anyone to use this software for any purpose,
// including commercial applications, and to alter it and redistribute it
// freely, subject to the following restrictions:
//
// 1. The origin of this software must not be misrepresented; you must not
//    claim that you wrote the original software. If you use this software
//    in a product, an acknowledgment in the product documentation would be
//    appreciated but is not required.
//
// 2. Altered source versions must be plainly marked as such, and must not
//    be misrepresented as being the original software.
//
// 3. This notice may not be removed or altered from any source distribution.
//

// ErrorHandler.cpp: implementation of the ErrorHandler class.
//
//////////////////////////////////////////////////////////////////////

#ifndef ANSI_GCC
#include "lynxwin.h"
#include "errorhandler.h"
#include "warndlg.h"

//#ifdef _DEBUG
//#undef THIS_FILE
//static char THIS_FILE[]=__FILE__;
//#define new DEBUG_NEW
//#endif

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CErrorHandler::CErrorHandler(CLynxWindow *lwin)
{
	mpLynxWin=lwin;
}

CErrorHandler::~CErrorHandler()
{

}

int CErrorHandler::Fatal(const char *message)
{
	if(mpLynxWin->DisplayModeWindowed()==FALSE) mpLynxWin->DisplayModeSet(DISPLAY_WINDOWED,DISPLAY_PRESERVE,DISPLAY_PRESERVE,DISPLAY_PRESERVE);

	mpLynxWin->MessageBox(message,"Handy Fatal Error",MB_OK | MB_ICONERROR);
	mpLynxWin->PostMessage(WM_CLOSE);
	mpLynxWin->mInitOK=FALSE;
	return FALSE;
}

int CErrorHandler::Warning(const char *message)
{
#ifdef _LYNXDBG
	// If in debug mode then update all open windows
	mpLynxWin->UpdateWindows();
#endif

	// Construct and display the warning dialog
	CWarnDialog warning(IDD_WARNING,mpLynxWin,message);

	// Display
	switch(warning.DoModal())
	{
		case IDCANCEL:
			if(mpLynxWin->mpLynx!=NULL) mpLynxWin->mpLynx->Reset();
			gBreakpointHit=TRUE;
			return FALSE;
			break;
		case IDOK:
			return TRUE;
			break;
	}
	return TRUE;
}
#else

#include <cstdio>
#include <cstdlib>
#include <cstring>
#include "system.h"
#include "errorhandler.h"

CErrorHandler::CErrorHandler()
{

}

CErrorHandler::~CErrorHandler()
{

}

int CErrorHandler::Fatal(const char *message)
{
	printf("Fatal Error: %s\n", message);
	exit(EXIT_FAILURE);
}

int CErrorHandler::Warning(const char *message)
{
	printf("Warning: %s\n", message);
	exit(EXIT_FAILURE);
}
#endif
