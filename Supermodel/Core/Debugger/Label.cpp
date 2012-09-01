/**
 ** Supermodel
 ** A Sega Model 3 Arcade Emulator.
 ** Copyright 2011 Bart Trzynadlowski, Nik Henson
 **
 ** This file is part of Supermodel.
 **
 ** Supermodel is free software: you can redistribute it and/or modify it under
 ** the terms of the GNU General Public License as published by the Free 
 ** Software Foundation, either version 3 of the License, or (at your option)
 ** any later version.
 **
 ** Supermodel is distributed in the hope that it will be useful, but WITHOUT
 ** ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 ** FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for
 ** more details.
 **
 ** You should have received a copy of the GNU General Public License along
 ** with Supermodel.  If not, see <http://www.gnu.org/licenses/>.
 **/

/*
 * Label.cpp
 */

#ifdef SUPERMODEL_DEBUGGER

#include "Label.h"

#include <cstring>
#include <string>

namespace Debugger
{
	CLabel::CLabel(CCPUDebug *lCPU, UINT32 lAddr, const char *lName) : CAddressRef(lCPU, lAddr)
	{
		strncpy(nameStr, lName, MAX_LABEL_LENGTH);
		nameStr[MAX_LABEL_LENGTH] = '\0';
		name = nameStr;
	}

	CLabel::CLabel(CCPUDebug *lCPU, UINT32 lAddrStart, UINT32 lAddrEnd, const char *lName) :
		CAddressRef(lCPU, lAddrStart, lAddrEnd - lAddrStart + 1)
	{
		strncpy(nameStr, lName, MAX_LABEL_LENGTH);
		nameStr[MAX_LABEL_LENGTH] = '\0';
		name = nameStr;
	}

	CComment::CComment(CCPUDebug *cCPU, UINT32 cAddr, const char *cText) : CAddressRef(cCPU, cAddr)
	{
		size_t size = strlen(cText);
		char *textStr = new char[size + 1];
		strncpy(textStr, cText, size);
		textStr[size] = '\0';
		text = textStr;
	}

	CComment::~CComment()
	{
		delete text;
	}

	CRegion::CRegion(CCPUDebug *rCPU, UINT32 rAddrStart, UINT32 rAddrEnd, bool rIsCode, bool rIsReadOnly, const char *rName) : 
		CLabel(rCPU, rAddrStart, rAddrEnd, rName), isCode(rIsCode), isReadOnly(rIsReadOnly), prevRegion(NULL), nextRegion(NULL)
	{
		//
	}
}

#endif  // SUPERMODEL_DEBUGGER
