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

// minimalistic gfx pipe implementation based on Vzzrzzn's GfxPipe.

#ifndef __DMAPIPE_H__
#define __DMAPIPE_H__

#include "backends/platform/ps2/sysdefs.h"
#include "backends/platform/ps2/GsDefs.h"

class SinglePipe;

struct GsVertex {
	uint16 x, y, z;
};

struct TexVertex {
	uint16 u, v;
};

class DmaPipe {
public:
	DmaPipe(uint32 size);
	void uploadTex(uint32 dest, uint16 bufWidth, uint16 destOfsX, uint16 destOfsY, uint8 pixelFmt, const void *src, uint16 width, uint16 height);
	void setTex(uint32 tex, uint32 texBufWidth, uint8 texPowW, uint8 texPowH, uint8 texPixFmt, uint32 clut, uint8 csm, uint32 clutBufWidth, uint32 clutPixFmt);
	void setDrawBuffer(uint64 base, uint64 width, uint8 pixelFmt, uint64 mask);
	void textureRect(const GsVertex *p1, const GsVertex *p2, const GsVertex *p3, const GsVertex *p4, const TexVertex *t1, const TexVertex *t2, const TexVertex *t3, const TexVertex *t4, uint32 rgba);
	void textureRect(const GsVertex *p1, const GsVertex *p2, const TexVertex *t1, const TexVertex *t2);
	void flatRect(const GsVertex *p1, const GsVertex *p2, const GsVertex *p3, const GsVertex *p4, uint32 rgba);
	void flatRect(const GsVertex *p1, const GsVertex *p2, uint32 rgba);

	void setOrigin(uint16 x, uint16 y);
	void setConfig(uint8 prModeCont, uint8 dither, uint8 colClamp);
	void setScissorRect(uint64 x1, uint64 y1, uint64 x2, uint64 y2);
	void setAlphaBlend(AlphaBlendColor a, AlphaBlendColor b, AlphaBlendAlpha c, AlphaBlendColor d, uint8 fix);
	void setFinishEvent(void);
	void flush(void);
	void waitForDma(void);
private:
	void checkSpace(uint32 needed);
	uint64 *_buf;
	uint8 _curPipe;
	SinglePipe *_pipes[2];
};

#endif //__DMAPIPE_H__
