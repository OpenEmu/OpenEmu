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
 * Error.cpp
 * 
 * Error reporting for 3D renderer. There are some situations in which it is 
 * appropriate to inform the user of an error during rendering. However, these 
 * will frequently lead to an avalanche of error messages. Therefore, error 
 * messages are managed by this interface, ensuring that they are displayed not
 * more than once per frame.
 *
 * Error functions can always be counted on to return FAIL, like ErrorLog().
 */

#include "Supermodel.h"


// Error bit flags (must not conflict)
#define ERROR_LOCAL_VERTEX_OVERFLOW	0x1
#define ERROR_UNABLE_TO_CACHE_MODEL	0x2


// Overflow in the local vertex buffer, which holds one model
bool CRender3D::ErrorLocalVertexOverflow(void)
{
	if ((errorMsgFlags&ERROR_LOCAL_VERTEX_OVERFLOW))
		return FAIL;
	errorMsgFlags |= ERROR_LOCAL_VERTEX_OVERFLOW;
	return ErrorLog("Overflow in local vertex buffer!");
}

// Model could not be cached, even after dumping display list and re-caching
bool CRender3D::ErrorUnableToCacheModel(UINT32 modelAddr)
{
	if ((errorMsgFlags&ERROR_UNABLE_TO_CACHE_MODEL))
		return FAIL;
	errorMsgFlags |= ERROR_UNABLE_TO_CACHE_MODEL;
	return ErrorLog("Encountered a model that was too large to cache (at %08X)!", modelAddr);
}
	
// Call this every frame to clear the error flag, allowing errors to be printed
void CRender3D::ClearErrors(void)
{
	errorMsgFlags = 0;
}