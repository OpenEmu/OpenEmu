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
 * Label.h
 */

#ifdef SUPERMODEL_DEBUGGER
#ifndef INCLUDED_LABEL_H
#define INCLUDED_LABEL_H

#include "AddressTable.h"
#include "Types.h"

#define MAX_LABEL_LENGTH 255

namespace Debugger
{
	class CCPUDebug;

	/*
	 * Class that represents a custom label added by user to a particular memory location.
	 */
	class CLabel : public CAddressRef
	{
	private:
		char nameStr[MAX_LABEL_LENGTH + 1];

	public:
		const char *name;

		CLabel(CCPUDebug *lCPU, UINT32 lAddr, const char *lName);

		CLabel(CCPUDebug *lCPU, UINT32 lAddrStart, UINT32 lAddrEnd, const char *lName);
	};

	/*
	 * Class that stores a comment added by a user to a particular memory location.
	 */
	class CComment : public CAddressRef
	{
	public:
		const char *text;

		CComment(CCPUDebug *cCPU, UINT32 cAddr, const char *cText);

		~CComment();
	};

	/*
	 * Class that represents various memory regions of a CPU, with information about the region.
	 */
	class CRegion : public CLabel
	{
	public:
		const bool isCode;
		const bool isReadOnly;

		CRegion *prevRegion;
		CRegion *nextRegion;

		CRegion(CCPUDebug *rCPU, UINT32 rAddrStart, UINT32 rAddrEnd, bool rIsCode, bool rIsReadOnly, const char *rName);
	};
}

#endif	// INCLUDED_LABEL_H
#endif  // SUPERMODEL_DEBUGGER