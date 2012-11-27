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
 */

#include "backends/platform/symbian/src/ScummApp.h"
#include "backends/platform/symbian/src/ScummVM.hrh"

#define _PAGESIZE_ 0x1000

#if defined(__WINS__) && !defined(S60V3) && !defined(UIQ3)
extern "C" int _chkstk(int /*a*/) {
_asm {
	push ecx
	cmp eax,_PAGESIZE_
	lea ecx,[esp] + 8
	jb short lastpage

	probepages:
	sub ecx,_PAGESIZE_
	sub eax,_PAGESIZE_

	test dword ptr [ecx],eax

	cmp eax,_PAGESIZE_
	jae short probepages

	lastpage:
	sub ecx,eax
	mov eax,esp

	test dword ptr [ecx],eax

	mov esp,ecx

	mov ecx,dword ptr [eax]
	mov eax,dword ptr [eax + 4]

	push eax
	ret
	}
	return 1;
}
#endif

#ifdef EPOC_AS_APP

// this function is called automatically by the SymbianOS to deliver the new CApaApplication object
#if !defined(UIQ3) && !defined(S60V3)
EXPORT_C
#endif
CApaApplication* NewApplication() {
	// Return pointer to newly created CQMApp
	return new CScummApp;
}

#if defined(UIQ3) || defined(S60V3)
#include <eikstart.h>
// E32Main() contains the program's start up code, the entry point for an EXE.
GLDEF_C TInt E32Main() {
	return EikStart::RunApplication(NewApplication);
}
#endif

#endif // EPOC_AS_APP

#if !defined(UIQ3) && !defined(S60V3)
GLDEF_C  TInt E32Dll(TDllReason) {
	return KErrNone;
}
#endif

CScummApp::CScummApp() {
}

CScummApp::~CScummApp() {
}

#if defined(UIQ3)
#include <scummvm.rsg>
/**
 * Returns the resource id to be used to declare the views supported by this UIQ3 app
 * @return TInt, resource id
 */
TInt CScummApp::ViewResourceId() {
	return R_SDL_VIEW_UI_CONFIGURATIONS;
}
#endif

/**
 *   Responsible for returning the unique UID of this application
 * @return unique UID for this application in a TUid
 **/
TUid CScummApp::AppDllUid() const {
	return TUid::Uid(ScummUid);
}

void CScummApp::GetDataFolder(TDes& aDataFolder)
{
	aDataFolder = _L("ScummVM");
}
/////////////////////////////////////////////////////////////////////////////////////////////////
