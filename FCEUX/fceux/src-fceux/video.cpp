/* FCE Ultra - NES/Famicom Emulator
*
* Copyright notice for this file:
*  Copyright (C) 2002 Xodnizel
*
* This program is free software; you can redistribute it and/or modify
* it under the terms of the GNU General Public License as published by
* the Free Software Foundation; either version 2 of the License, or
* (at your option) any later version.
*
* This program is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
* GNU General Public License for more details.
*
* You should have received a copy of the GNU General Public License
* along with this program; if not, write to the Free Software
* Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
*/
#include <stdint.h>

#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <zlib.h>

#include "types.h"
#include "video.h"
#include "fceu.h"
#include "file.h"
#include "utils/memory.h"
#include "utils/crc32.h"
#include "state.h"
#include "palette.h"
#include "input.h"
#include "vsuni.h"
#include "driver.h"

uint8 *XBuf=NULL;
uint8 *XBackBuf=NULL;

GUIMESSAGE guiMessage;

/**
* TODO: This function is Windows-only. It should probably be moved.
**/
int FCEU_InitVirtualVideo(void)
{
	if(!XBuf)		/* Some driver code may allocate XBuf externally. */
		/* 256 bytes per scanline, * 240 scanline maximum, +16 for alignment,
		*/


		if(!(XBuf= (uint8*) (realloc(XBuf, 256 * 256 + 16))) || !(XBackBuf= (uint8*) (realloc(XBackBuf, 256 * 256 + 16))))
		{
			return 0;
		}

		if( sizeof(uint8*) == 4 )
		{
			uintptr_t m = (uintptr_t)XBuf;
			m = ( 8 - m) & 7;
			XBuf+=m;
		}

		memset(XBuf,128,256*256); //*240);
		memset(XBackBuf,128,256*256);

		return 1;
}

void FCEU_DispMessage(const char *format, int disppos=0, ...)
{
	va_list ap;

	va_start(ap,disppos);
	vsnprintf(guiMessage.errmsg,sizeof(guiMessage.errmsg),format,ap);
	va_end(ap);
}
