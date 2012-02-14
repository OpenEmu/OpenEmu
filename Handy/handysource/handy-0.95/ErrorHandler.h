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

// ErrorHandler.h: interface for the ErrorHandler class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_ERRORHANDLER_H__A3B25DE1_6F78_11D2_8E90_444553540000__INCLUDED_)
#define AFX_ERRORHANDLER_H__A3B25DE1_6F78_11D2_8E90_444553540000__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

class CErrorHandler : public CErrorInterface
{
public:
#ifndef SDL_PATCH
	CErrorHandler(CLynxWindow *lwin);
#else
	CErrorHandler();
#endif
	virtual ~CErrorHandler();
	int Warning(const char *message);
	int Fatal(const char *message);

private:
#ifndef SDL_PATCH
	CLynxWindow *mpLynxWin;
#endif
};

#endif // !defined(AFX_ERRORHANDLER_H__A3B25DE1_6F78_11D2_8E90_444553540000__INCLUDED_)
