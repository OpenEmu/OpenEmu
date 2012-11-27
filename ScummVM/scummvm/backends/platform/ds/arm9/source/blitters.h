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

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

 #ifndef _BLITTERS_H_
 #define _BLITTERS_H_

#define USING_ARM_BLITTERS

#ifndef USING_ARM_BLITTERS

namespace DS {

void asmDrawStripToScreen(int height, int width, byte const *text, byte const *src, byte *dst,
	int vsPitch, int vmScreenWidth, int textSurfacePitch);
void asmCopy8Col(byte *dst, int dstPitch, const byte *src, int height);
void Rescale_320x256xPAL8_To_256x256x1555(u16 *dest, const u8 *src, int destStride, int srcStride, const u16 *palette);
void Rescale_320x256x1555_To_256x256x1555(u16 *dest, const u16 *src, int destStride, int srcStride);

}	// End of namespace DS

#else

extern "C" {

void ITCM_CODE asmDrawStripToScreen(int height, int width, byte const *text, byte const *src, byte *dst,
	int vsPitch, int vmScreenWidth, int textSurfacePitch);
void ITCM_CODE asmCopy8Col(byte *dst, int dstPitch, const byte *src, int height);
void ITCM_CODE Rescale_320x256xPAL8_To_256x256x1555(u16 *dest, const u8 *src, int destStride, int srcStride, const u16 *palette, u32 numLines);
void ITCM_CODE Rescale_320x256x1555_To_256x256x1555(u16 *dest, const u16 *src, int destStride, int srcStride);

}

#endif

#endif
