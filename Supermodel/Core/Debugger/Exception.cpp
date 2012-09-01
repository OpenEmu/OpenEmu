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
 * Exception.cpp
 */

#ifdef SUPERMODEL_DEBUGGER

#include "Exception.h"

namespace Debugger
{
	CException::CException(CCPUDebug *exCPU, const char *exId, UINT16 exCode, const char *exName) : 
		cpu(exCPU), id(exId), code(exCode), name(exName), trap(false), count(0)
	{
		//
	}
}

#endif  // SUPERMODEL_DEBUGGER