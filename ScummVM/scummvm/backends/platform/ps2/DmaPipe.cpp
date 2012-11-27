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

#include <kernel.h>
#include <malloc.h>
#include "backends/platform/ps2/DmaPipe.h"
#include "backends/platform/ps2/GsDefs.h"

class SinglePipe {
public:
	SinglePipe(uint64 *buf, uint32 size);
	uint32 spaceLeft(void);
	void flush(void);
	void setGifRegListTag(uint8 numRegs, uint64 list);
	void setGifLoopTag(uint16 nLoop);
	void setReg(uint64 reg, uint64 value);
	void setListReg(uint64 value1, uint64 value2);
	void appendChain(uint64 dmaTag);
	void init(void);
	uint64 *_chainHead;
private:
	uint64 *_bufPos;
	uint16 *_chainSize;
	uint64 *_buf;
	uint32 _size;
};

DmaPipe::DmaPipe(uint32 size) {
	size &= ~0x1F;
	_buf = (uint64 *)memalign(64, size);
	_curPipe = 0;
    _pipes[0] = new SinglePipe(_buf, size >> 4);
	_pipes[1] = new SinglePipe(_buf + (size >> 4), size >> 4);

	// reset DMAC Channel 2
	D2_CHCR = 0;
	D2_TADR = 0;
	D2_MADR = 0;
	D2_ASR1 = 0;
	D2_ASR0 = 0;
	D_STAT  = 0xFF1F;
	D_CTRL  = 0;
	D_PCR   = 0;
	D_SQWC  = 0;
	D_RBOR  = 0;
	D_RBSR  = 0;
	D_CTRL  = 1;
	if (!(D_STAT & CIM2)) // channel 2 interrupts enabled?
		D_STAT = CIM2;	  // enable them
	if (D_STAT & CIS2)	  // is there an old interrupt we have to acknowledge?
		D_STAT = CIS2;	  // do so...
	SifSetDChain();
}

void DmaPipe::uploadTex(uint32 dest, uint16 bufWidth, uint16 destOfsX, uint16 destOfsY, uint8 pixelFmt, const void *src, uint16 width, uint16 height) {

	checkSpace(5);
	*(_pipes[_curPipe]->_chainHead) &= 0xffffffff8fffffff; // change last chain tag id, from 'end' to 'cnt'
	*(_pipes[_curPipe]->_chainHead) |= (1 << 28);
	_pipes[_curPipe]->setGifLoopTag(4);
	_pipes[_curPipe]->setReg(GPR_BITBLTBUF, GS_SET_DEST_BLTBUF((dest/256) & 0x3fff, (bufWidth/64) & 0x3f, pixelFmt & 0x3f));
	_pipes[_curPipe]->setReg(	GPR_TRXPOS, GS_SET_DEST_TRXPOS(destOfsX, destOfsY));
	_pipes[_curPipe]->setReg(   GPR_TRXREG, GS_SET_TRXREG(width, height));
	_pipes[_curPipe]->setReg(   GPR_TRXDIR, 0);

	checkSpace(15);
	uint32 numq = width * height;
	switch (pixelFmt) {
		case GS_PSMCT32:
			numq = (numq + 3) >> 2; break;
		case GS_PSMCT24:
			numq = (numq + 2) / 3; break;
		case GS_PSMCT16:
		case GS_PSMCT16S:
			numq = (numq + 7) >> 3; break;
		case GS_PSMT8:
		case GS_PSMT8H:
			numq = (numq + 15) >> 4; break;
		case GS_PSMT4HL:
		case GS_PSMT4HH:
		case GS_PSMT4:
			numq = (numq + 31) >> 5; break;
		default:
			numq = 0;
	}
	uint64 texSrc = (uint32)src & 0x7fffffff;
	while (numq) {
		uint64 sendQuads = (numq <= 0x7FF0) ? numq : 0x7FF0;
		_pipes[_curPipe]->appendChain((1 << 28) | 1);
		_pipes[_curPipe]->appendChain(0x0800000000000000 + sendQuads); // IMAGE mode giftag. Flg = 10b, nloop = currq
		_pipes[_curPipe]->appendChain((texSrc << 32) | 0x0000000030000000 | sendQuads); // set up dma tag for image transfer. next = tex addr, id = 11b, qwc = numq
		numq -= sendQuads;
		texSrc += sendQuads * 16;
	}
	_pipes[_curPipe]->appendChain(0x0000000070000000); // next dma tag
	_pipes[_curPipe]->setGifLoopTag(1);
	_pipes[_curPipe]->setReg(GPR_TEXFLUSH, 1);
}

void DmaPipe::setTex(uint32 tex, uint32 texBufWidth, uint8 texPowW, uint8 texPowH, uint8 texPixFmt, uint32 clut, uint8 csm, uint32 clutBufWidth, uint32 clutPixFmt) {
	checkSpace(7);
	_pipes[_curPipe]->setGifLoopTag(6);
	_pipes[_curPipe]->setReg( GPR_TEXCLUT, 256 / 64);
	_pipes[_curPipe]->setReg(GPR_TEXFLUSH, 0);
	_pipes[_curPipe]->setReg(    GPR_TEXA, GS_SET_TEXA(128, 1, 0));
	_pipes[_curPipe]->setReg(  GPR_TEX1_1, GS_SET_TEX1(0, 0, FILTER_LINEAR, FILTER_LINEAR, 0, 0, 0));
	_pipes[_curPipe]->setReg(  GPR_TEX0_1, GS_SET_TEX0(tex / 256, texBufWidth / 64, texPixFmt, texPowW, texPowH, 1, 0, clut / 256, clutBufWidth / 64, csm, 0, 1));
	_pipes[_curPipe]->setReg( GPR_CLAMP_1, 0);
}

void DmaPipe::textureRect(const GsVertex *p1, const GsVertex *p2, const TexVertex *t1, const TexVertex *t2) {
	checkSpace(4);
	_pipes[_curPipe]->setGifRegListTag( 6, 0xffffffffff535310);
	_pipes[_curPipe]->setListReg( GS_SET_PRIM(PR_SPRITE, 0, 1, 0, 1, 0, 1, 0, 0),
								  GS_SET_COLQ(GS_RGBA(0x80, 0x80, 0x80, 0x80)));
	_pipes[_curPipe]->setListReg( GS_SET_UV(t1->u, t1->v),
								  GS_SET_XYZ(p1->x, p1->y, p1->z));
	_pipes[_curPipe]->setListReg( GS_SET_UV(t2->u, t2->v),
								  GS_SET_XYZ(p2->x, p2->y, p2->z));
}

void DmaPipe::textureRect(const GsVertex *p1, const GsVertex *p2, const GsVertex *p3, const GsVertex *p4, const TexVertex *t1, const TexVertex *t2, const TexVertex *t3, const TexVertex *t4, uint32 rgba) {
	checkSpace(6);
	_pipes[_curPipe]->setGifRegListTag(10, 0xffffff5353535310);

	_pipes[_curPipe]->setListReg( GS_SET_PRIM(PR_TRIANGLESTRIP, 0, 1, 0, 1, 0, 1, 0, 0),
								  GS_SET_COLQ(rgba));
	_pipes[_curPipe]->setListReg( GS_SET_UV(t1->u, t1->v),
								  GS_SET_XYZ(p1->x, p1->y, p1->z));
	_pipes[_curPipe]->setListReg( GS_SET_UV(t2->u, t2->v),
								  GS_SET_XYZ(p2->x, p2->y, p2->z));
	_pipes[_curPipe]->setListReg( GS_SET_UV(t3->u, t3->v),
								  GS_SET_XYZ(p3->x, p3->y, p3->z));
	_pipes[_curPipe]->setListReg( GS_SET_UV(t4->u, t4->v),
								  GS_SET_XYZ(p4->x, p4->y, p4->z));
}

void DmaPipe::flatRect(const GsVertex *p1, const GsVertex *p2, const GsVertex *p3, const GsVertex *p4, uint32 rgba) {
	checkSpace(4);
	_pipes[_curPipe]->setGifRegListTag( 6, 0xffffffffff555510);
	_pipes[_curPipe]->setListReg( GS_SET_PRIM(PR_TRIANGLESTRIP, 0, 0, 0, 0, 0, 0, 0, 0),
								  GS_SET_COLQ(rgba));
	_pipes[_curPipe]->setListReg( GS_SET_XYZ(p1->x, p1->y, p1->z),
								  GS_SET_XYZ(p2->x, p2->y, p2->z));
	_pipes[_curPipe]->setListReg( GS_SET_XYZ(p3->x, p3->y, p3->z),
								  GS_SET_XYZ(p4->x, p4->y, p4->z));
}

void DmaPipe::flatRect(const GsVertex *p1, const GsVertex *p2, uint32 rgba) {
	checkSpace(3);
	_pipes[_curPipe]->setGifRegListTag( 4, 0xffffffffffff5510);
	_pipes[_curPipe]->setListReg( GS_SET_PRIM(PR_SPRITE, 0, 0, 0, 1, 0, 0, 0, 0),
								  GS_SET_COLQ(rgba));
	_pipes[_curPipe]->setListReg( GS_SET_XYZ(p1->x, p1->y, p1->z),
								  GS_SET_XYZ(p2->x, p2->y, p2->z));
}

void DmaPipe::setOrigin(uint16 x, uint16 y) {
	checkSpace(2);
	_pipes[_curPipe]->setGifLoopTag(1);
	_pipes[_curPipe]->setReg( GPR_XYOFFSET_1, GS_SET_XYOFFSET(x, y));
}

void DmaPipe::setAlphaBlend(AlphaBlendColor a, AlphaBlendColor b, AlphaBlendAlpha c, AlphaBlendColor d, uint8 fix) {
	checkSpace(2);
	_pipes[_curPipe]->setGifLoopTag(1);
	_pipes[_curPipe]->setReg(GPR_ALPHA_1, GS_SET_ALPHA(a, b, c, d, fix));
}

void DmaPipe::setConfig(uint8 prModeCont, uint8 dither, uint8 colClamp) {
	checkSpace(9);
	_pipes[_curPipe]->setGifLoopTag(8);

	// set some defaults
	// alpha blending formula: (A-B) * C + D
		// set: A = dest pixel, b = 0, C = source alpha, D = source pixel, fix = don't care

	_pipes[_curPipe]->setReg(GPR_ALPHA_1, GS_SET_ALPHA(DEST_COLOR, ZERO_COLOR, SOURCE_ALPHA, SOURCE_COLOR, 0));
	_pipes[_curPipe]->setReg(   GPR_PRIM, 0);
	_pipes[_curPipe]->setReg(   GPR_PABE, 0); // alpha blending off
	_pipes[_curPipe]->setReg( GPR_TEST_1, GS_SET_TEST(0, 0, 0, 0, 0, 0, 1, 1)); // ztest off
	_pipes[_curPipe]->setReg( GPR_ZBUF_1, (uint64)1 << 32); // zbuffer off

	_pipes[_curPipe]->setReg( GPR_PRMODECONT, prModeCont & 1);
	_pipes[_curPipe]->setReg( GPR_DTHE, dither & 1);
	_pipes[_curPipe]->setReg( GPR_COLCLAMP, colClamp & 1);
}

void DmaPipe::setScissorRect(uint64 x1, uint64 y1, uint64 x2, uint64 y2) {
	checkSpace(2);
	_pipes[_curPipe]->setGifLoopTag(1);
	_pipes[_curPipe]->setReg( GPR_SCISSOR_1, GS_SET_SCISSOR(x1, x2, y1, y2));
}

void DmaPipe::setDrawBuffer(uint64 base, uint64 width, uint8 pixelFmt, uint64 mask) {
	checkSpace(2);
	_pipes[_curPipe]->setGifLoopTag(1);
	_pipes[_curPipe]->setReg( GPR_FRAME_1, GS_SET_FRAME(base / 8192, width / 64, pixelFmt, mask));
}

void DmaPipe::setFinishEvent(void) {
	checkSpace(3);
	// make GS generate a FINISH interrupt when it's done.
	_pipes[_curPipe]->setGifLoopTag(2);
	_pipes[_curPipe]->setReg( GPR_FINISH, 1);
	_pipes[_curPipe]->setReg( GPR_SIGNAL, 1);
}

void DmaPipe::checkSpace(uint32 needed) {
	if (_pipes[_curPipe]->spaceLeft() < (needed << 1))
		flush();
}

void DmaPipe::waitForDma(void) {
	while (D2_CHCR & 0x100) {}
}

void DmaPipe::flush(void) {
	waitForDma();
	FlushCache(0);
	_pipes[_curPipe]->flush();
	_curPipe ^= 1;
	_pipes[_curPipe]->init();
}

SinglePipe::SinglePipe(uint64 *buf, uint32 size) {
	_buf = buf;
	_size = size;
	init();
}

void SinglePipe::flush(void) {
	D2_TADR = (uint32)_buf;
	D2_QWC  = 0;
	D2_CHCR |= 0x185;
}

void SinglePipe::init(void) {
	_buf[0] = 0x0000000070000000;
	_buf[1] = 0;
	_chainHead = _buf;
	_chainSize = (uint16 *)_chainHead;
	_bufPos = _buf + 2;
}

uint32 SinglePipe::spaceLeft(void) {
	return (_size - (_bufPos - _buf));
}

void SinglePipe::appendChain(uint64 dmaTag) {
	_chainHead = _bufPos;
	_chainHead[0] = dmaTag;
	_chainHead[1] = 0;
	_chainSize = (uint16 *)_chainHead;
	_bufPos += 2;
}

void SinglePipe::setReg(uint64 reg, uint64 value) {
	*_bufPos++ = value;
	*_bufPos++ = reg;
    (*_chainSize)++;
}

void SinglePipe::setListReg(uint64 value1, uint64 value2) {
	*_bufPos++ = value1;
	*_bufPos++ = value2;
	(*_chainSize)++;
}

void SinglePipe::setGifRegListTag(uint8 numRegs, uint64 list) {
	*_bufPos++ = GIF_SET_TAG(1, 1, 0, 0, 1, 0) | ((uint64)numRegs << 60);
	*_bufPos++ = list;
	(*_chainSize)++;
}

void SinglePipe::setGifLoopTag(uint16 nLoop) {
	*_bufPos++ = GIF_SET_TAG(0, 1, 0, 0, 0, 1) | nLoop;
	*_bufPos++ = 0xfffffffffffffffe;
	(*_chainSize)++;
}
