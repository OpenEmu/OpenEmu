/***************************************************************************
 *   Copyright (C) 2010 by Sindre Aamås                                    *
 *   sinamas@users.sourceforge.net                                         *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License version 2 as     *
 *   published by the Free Software Foundation.                            *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License version 2 for more details.                *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   version 2 along with this program; if not, write to the               *
 *   Free Software Foundation, Inc.,                                       *
 *   59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.             *
 ***************************************************************************/
#ifndef PPU_H
#define PPU_H

#include "video/ly_counter.h"
#include "video/sprite_mapper.h"
#include "gbint.h"

namespace gambatte {

class PPUFrameBuf {
	uint_least32_t *buf_;
	uint_least32_t *fbline_;
	int pitch_;
	
	static uint_least32_t * nullfbline() { static uint_least32_t nullfbline_[160]; return nullfbline_; }
	
public:
	PPUFrameBuf() : buf_(0), fbline_(nullfbline()), pitch_(0) {}
	uint_least32_t * fb() const { return buf_; }
	uint_least32_t * fbline() const { return fbline_; }
	int pitch() const { return pitch_; }
	void setBuf(uint_least32_t *const buf, const int pitch) { buf_ = buf; pitch_ = pitch; fbline_ = nullfbline(); }
	void setFbline(const unsigned ly) { fbline_ = buf_ ? buf_ + static_cast<long>(ly) * static_cast<long>(pitch_) : nullfbline(); }
};

struct PPUState {
	void (*f)(struct PPUPriv &v);
	unsigned (*predictCyclesUntilXpos_f)(const struct PPUPriv &v, int targetxpos, unsigned cycles);
	unsigned char id;
};

// The PPU loop accesses a lot of state at once, so it's difficult to split this up much beyond grouping stuff into smaller structs.
struct PPUPriv {
	unsigned long bgPalette[8 * 4];
	unsigned long spPalette[8 * 4];
	struct Sprite { unsigned char spx, oampos, line, attrib; } spriteList[11];
	unsigned short spwordList[11];
	unsigned char nextSprite;
	unsigned char currentSprite;

	const unsigned char *vram;
	const PPUState *nextCallPtr;

	unsigned long now;
	unsigned long lastM0Time;
	long cycles;

	unsigned tileword;
	unsigned ntileword;

	SpriteMapper spriteMapper;
	LyCounter lyCounter;
	PPUFrameBuf framebuf;

	unsigned char lcdc;
	unsigned char scy;
	unsigned char scx;
	unsigned char wy;
	unsigned char wy2;
	unsigned char wx;
	unsigned char winDrawState;
	unsigned char wscx;
	unsigned char winYPos;
	unsigned char reg0;
	unsigned char reg1;
	unsigned char attrib;
	unsigned char nattrib;
	unsigned char xpos;
	unsigned char endx;

	bool cgb;
	bool weMaster;
	
	PPUPriv(NextM0Time &nextM0Time, const unsigned char *oamram, const unsigned char *vram);
};

class PPU {
	PPUPriv p_;
public:
	PPU(NextM0Time &nextM0Time, const unsigned char *oamram, const unsigned char *vram)
	: p_(nextM0Time, oamram, vram)
	{
	}
	
	unsigned long * bgPalette() { return p_.bgPalette; }
	bool cgb() const { return p_.cgb; }
	void doLyCountEvent() { p_.lyCounter.doEvent(); }
	unsigned long doSpriteMapEvent(unsigned long time) { return p_.spriteMapper.doEvent(time); }
	const PPUFrameBuf & frameBuf() const { return p_.framebuf; }
	bool inactivePeriodAfterDisplayEnable(unsigned long cc) const { return p_.spriteMapper.inactivePeriodAfterDisplayEnable(cc); }
	unsigned long lastM0Time() const { return p_.lastM0Time; }
	unsigned lcdc() const { return p_.lcdc; }
	void loadState(const SaveState &state, const unsigned char *oamram);
	const LyCounter & lyCounter() const { return p_.lyCounter; }
	unsigned long now() const { return p_.now; }
	void oamChange(unsigned long cc) { p_.spriteMapper.oamChange(cc); }
	void oamChange(const unsigned char *oamram, unsigned long cc) { p_.spriteMapper.oamChange(oamram, cc); }
	unsigned long predictedNextXposTime(unsigned xpos) const;
	void reset(const unsigned char *oamram, const unsigned char *vram, bool cgb);
	void resetCc(unsigned long oldCc, unsigned long newCc);
	void saveState(SaveState &ss) const;
	void setFrameBuf(uint_least32_t *buf, unsigned pitch) { p_.framebuf.setBuf(buf, pitch); }
	void setLcdc(unsigned lcdc, unsigned long cc);
	void setScx(const unsigned scx) { p_.scx = scx; }
	void setScy(const unsigned scy) { p_.scy = scy; }
	void setStatePtrs(SaveState &ss) { p_.spriteMapper.setStatePtrs(ss); }
	void setWx(const unsigned wx) { p_.wx = wx; }
	void setWy(const unsigned wy) { p_.wy = wy; }
	void updateWy2() { p_.wy2 = p_.wy; }
	void speedChange(unsigned long cycleCounter);
	unsigned long * spPalette() { return p_.spPalette; }
	void update(unsigned long cc);
};

}

#endif
