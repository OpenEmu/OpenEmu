/***************************************************************************
 *   Copyright (C) 2007 by Sindre Aamås                                    *
 *   aamas@stud.ntnu.no                                                    *
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
#include "video.h"
#include "savestate.h"
#include <cstring>
#include <algorithm>

namespace gambatte {

void LCD::setDmgPalette(unsigned long *const palette, const unsigned long *const dmgColors, const unsigned data) {
	palette[0] = dmgColors[data      & 3];
	palette[1] = dmgColors[data >> 2 & 3];
	palette[2] = dmgColors[data >> 4 & 3];
	palette[3] = dmgColors[data >> 6 & 3];
}

static unsigned long gbcToRgb32(const unsigned bgr15) {
	const unsigned long r = bgr15       & 0x1F;
	const unsigned long g = bgr15 >>  5 & 0x1F;
	const unsigned long b = bgr15 >> 10 & 0x1F;

	return ((r * 13 + g * 2 + b) >> 1) << 16 | (g * 3 + b) << 9 | (r * 3 + g * 2 + b * 11) >> 1;
}

/*static unsigned long gbcToRgb16(const unsigned bgr15) {
	const unsigned r = bgr15 & 0x1F;
	const unsigned g = bgr15 >> 5 & 0x1F;
	const unsigned b = bgr15 >> 10 & 0x1F;

	return (((r * 13 + g * 2 + b + 8) << 7) & 0xF800) | ((g * 3 + b + 1) >> 1) << 5 | ((r * 3 + g * 2 + b * 11 + 8) >> 4);
}

static unsigned long gbcToUyvy(const unsigned bgr15) {
	const unsigned r5 = bgr15 & 0x1F;
	const unsigned g5 = bgr15 >> 5 & 0x1F;
	const unsigned b5 = bgr15 >> 10 & 0x1F;

	// y = (r5 * 926151 + g5 * 1723530 + b5 * 854319) / 510000 + 16;
	// u = (b5 * 397544 - r5 * 68824 - g5 * 328720) / 225930 + 128;
	// v = (r5 * 491176 - g5 * 328720 - b5 * 162456) / 178755 + 128;

	const unsigned long y = (r5 * 116 + g5 * 216 + b5 * 107 + 16 * 64 + 32) >> 6;
	const unsigned long u = (b5 * 225 - r5 * 39 - g5 * 186 + 128 * 128 + 64) >> 7;
	const unsigned long v = (r5 * 176 - g5 * 118 - b5 * 58 + 128 * 64 + 32) >> 6;

#ifdef WORDS_BIGENDIAN
	return u << 24 | y << 16 | v << 8 | y;
#else
	return y << 24 | v << 16 | y << 8 | u;
#endif
}*/

LCD::LCD(const unsigned char *const oamram, const unsigned char *const vram, const VideoInterruptRequester memEventRequester) :
	ppu(nextM0Time_, oamram, vram),
	eventTimes_(memEventRequester),
	statReg(0),
	m2IrqStatReg_(0),
	m1IrqStatReg_(0)
{
	std::memset( bgpData, 0, sizeof  bgpData);
	std::memset(objpData, 0, sizeof objpData);

	for (std::size_t i = 0; i < sizeof(dmgColorsRgb32) / sizeof(dmgColorsRgb32[0]); ++i)
		setDmgPaletteColor(i, (3 - (i & 3)) * 85 * 0x010101);

	reset(oamram, false);
	setVideoBuffer(0, 160);
}

void LCD::reset(const unsigned char *const oamram, const bool cgb) {
	ppu.reset(oamram, cgb);
	lycIrq.setCgb(cgb);
	refreshPalettes();
}

static unsigned long mode2IrqSchedule(const unsigned statReg, const LyCounter &lyCounter, const unsigned long cycleCounter) {
	if (!(statReg & 0x20))
		return DISABLED_TIME;
	
	unsigned next = lyCounter.time() - cycleCounter;
	
	if (lyCounter.ly() >= 143 || (lyCounter.ly() == 142 && next <= 4) || (statReg & 0x08)) {
		next += (153u - lyCounter.ly()) * lyCounter.lineTime();
	} else {
		if (next <= 4)
			next += lyCounter.lineTime();
		
		next -= 4;
	}
	
	return cycleCounter + next;
}

static inline unsigned long m0IrqTimeFromXpos166Time(const unsigned long xpos166Time, const bool cgb, const bool ds) {
	return xpos166Time + cgb - ds;
}

static inline unsigned long hdmaTimeFromM0Time(const unsigned long m0Time, const bool ds) {
	return m0Time + 1 - ds;
}

static unsigned long nextHdmaTime(const unsigned long lastM0Time,
		const unsigned long nextM0Time, const unsigned long cycleCounter, const bool ds) {
	return cycleCounter < hdmaTimeFromM0Time(lastM0Time, ds)
	                    ? hdmaTimeFromM0Time(lastM0Time, ds)
	                    : hdmaTimeFromM0Time(nextM0Time, ds);
}

void LCD::setStatePtrs(SaveState &state) {
	state.ppu.bgpData.set(  bgpData, sizeof  bgpData);
	state.ppu.objpData.set(objpData, sizeof objpData);
	ppu.setStatePtrs(state);
}

void LCD::saveState(SaveState &state) const {
	state.mem.hdmaTransfer = hdmaIsEnabled();
	state.ppu.nextM0Irq = eventTimes_(MODE0_IRQ) - ppu.now();
	state.ppu.pendingLcdstatIrq = eventTimes_(ONESHOT_LCDSTATIRQ) != DISABLED_TIME;
	
	lycIrq.saveState(state);
	m0Irq_.saveState(state);
	ppu.saveState(state);
}

void LCD::loadState(const SaveState &state, const unsigned char *const oamram) {
	statReg = state.mem.ioamhram.get()[0x141];
	m2IrqStatReg_ = statReg;
	m1IrqStatReg_ = statReg;

	ppu.loadState(state, oamram);
	lycIrq.loadState(state);
	m0Irq_.loadState(state);

	if (ppu.lcdc() & 0x80) {
		nextM0Time_.predictNextM0Time(ppu);
		lycIrq.reschedule(ppu.lyCounter(), ppu.now());
		
		eventTimes_.setm<ONESHOT_LCDSTATIRQ>(state.ppu.pendingLcdstatIrq
							? ppu.now() + 1 : static_cast<unsigned long>(DISABLED_TIME));
		eventTimes_.setm<ONESHOT_UPDATEWY2>(state.ppu.oldWy != state.mem.ioamhram.get()[0x14A]
							? ppu.now() + 1 : static_cast<unsigned long>(DISABLED_TIME));
		eventTimes_.set<LY_COUNT>(ppu.lyCounter().time());
		eventTimes_.setm<SPRITE_MAP>(SpriteMapper::schedule(ppu.lyCounter(), ppu.now()));
		eventTimes_.setm<LYC_IRQ>(lycIrq.time());
		eventTimes_.setm<MODE1_IRQ>(ppu.lyCounter().nextFrameCycle(144 * 456, ppu.now()));
		eventTimes_.setm<MODE2_IRQ>(mode2IrqSchedule(statReg, ppu.lyCounter(), ppu.now()));
		eventTimes_.setm<MODE0_IRQ>((statReg & 0x08) ? ppu.now() + state.ppu.nextM0Irq : static_cast<unsigned long>(DISABLED_TIME));
		eventTimes_.setm<HDMA_REQ>(state.mem.hdmaTransfer
				? nextHdmaTime(ppu.lastM0Time(), nextM0Time_.predictedNextM0Time(), ppu.now(), isDoubleSpeed())
				: static_cast<unsigned long>(DISABLED_TIME));
	} else for (int i = 0; i < NUM_MEM_EVENTS; ++i)
		eventTimes_.set(static_cast<MemEvent>(i), DISABLED_TIME);
	
	refreshPalettes();
}

void LCD::refreshPalettes() {
	if (ppu.cgb()) {
		for (unsigned i = 0; i < 8 * 8; i += 2) {
			ppu.bgPalette()[i >> 1] = gbcToRgb32( bgpData[i] |  bgpData[i + 1] << 8);
			ppu.spPalette()[i >> 1] = gbcToRgb32(objpData[i] | objpData[i + 1] << 8);
		}
	} else {
		setDmgPalette(ppu.bgPalette()    , dmgColorsRgb32    ,  bgpData[0]);
		setDmgPalette(ppu.spPalette()    , dmgColorsRgb32 + 4, objpData[0]);
		setDmgPalette(ppu.spPalette() + 4, dmgColorsRgb32 + 8, objpData[1]);
	}
}

namespace {

template<class Blend>
static void blitOsdElement(uint_least32_t *d,
		const uint_least32_t *s, const unsigned width, unsigned h, const int dpitch, Blend blend)
{
	while (h--) {
		for (unsigned w = width; w--;) {
			if (*s != 0xFFFFFFFF)
				*d = blend(*s, *d);

			++d;
			++s;
		}

		d += dpitch - static_cast<int>(width);
	}
}

template<unsigned weight>
struct Blend {
	enum { SW = weight - 1 };
	enum { LOWMASK = SW * 0x010101ul };
	uint_least32_t operator()(const uint_least32_t s, const uint_least32_t d) const {
		return (s * SW + d - (((s & LOWMASK) * SW + (d & LOWMASK)) & LOWMASK)) / weight;
	}
};

template<typename T>
static void clear(T *buf, const unsigned long color, const int dpitch) {
	unsigned lines = 144;

	while (lines--) {
		std::fill_n(buf, 160, color);
		buf += dpitch;
	}
}

}

void LCD::updateScreen(const bool blanklcd, const unsigned long cycleCounter) {
	update(cycleCounter);
	
	if (blanklcd && ppu.frameBuf().fb()) {
		const unsigned long color = ppu.cgb() ? gbcToRgb32(0xFFFF) : dmgColorsRgb32[0];
		clear(ppu.frameBuf().fb(), color, ppu.frameBuf().pitch());
	}

	if (ppu.frameBuf().fb() && osdElement.get()) {
		if (const uint_least32_t *const s = osdElement->update()) {
			uint_least32_t *const d = ppu.frameBuf().fb()
					+ static_cast<long>(osdElement->y()) * ppu.frameBuf().pitch() + osdElement->x();

			switch (osdElement->opacity()) {
			case OsdElement::SEVEN_EIGHTHS:
				blitOsdElement(d, s, osdElement->w(), osdElement->h(), ppu.frameBuf().pitch(), Blend<8>()); break;
			case OsdElement::THREE_FOURTHS:
				blitOsdElement(d, s, osdElement->w(), osdElement->h(), ppu.frameBuf().pitch(), Blend<4>()); break;
			}
		} else
			osdElement.reset();
	}
}

void LCD::resetCc(const unsigned long oldCc, const unsigned long newCc) {
	update(oldCc);
	ppu.resetCc(oldCc, newCc);
	
	if (ppu.lcdc() & 0x80) {
		const unsigned long dec = oldCc - newCc;
		
		nextM0Time_.invalidatePredictedNextM0Time();
		lycIrq.reschedule(ppu.lyCounter(), newCc);
		
		for (int i = 0; i < NUM_MEM_EVENTS; ++i) {
			if (eventTimes_(static_cast<MemEvent>(i)) != DISABLED_TIME)
				eventTimes_.set(static_cast<MemEvent>(i), eventTimes_(static_cast<MemEvent>(i)) - dec);
		}
		
		eventTimes_.set<LY_COUNT>(ppu.lyCounter().time());
	}
}

void LCD::speedChange(const unsigned long cycleCounter) {
	update(cycleCounter);
	ppu.speedChange(cycleCounter);
	
	if (ppu.lcdc() & 0x80) {
		nextM0Time_.predictNextM0Time(ppu);
		lycIrq.reschedule(ppu.lyCounter(), cycleCounter);
		
		eventTimes_.set<LY_COUNT>(ppu.lyCounter().time());
		eventTimes_.setm<SPRITE_MAP>(SpriteMapper::schedule(ppu.lyCounter(), cycleCounter));
		eventTimes_.setm<LYC_IRQ>(lycIrq.time());
		eventTimes_.setm<MODE1_IRQ>(ppu.lyCounter().nextFrameCycle(144 * 456, cycleCounter));
		eventTimes_.setm<MODE2_IRQ>(mode2IrqSchedule(statReg, ppu.lyCounter(), cycleCounter));
		
		if (eventTimes_(MODE0_IRQ) != DISABLED_TIME && eventTimes_(MODE0_IRQ) - cycleCounter > 1)
			eventTimes_.setm<MODE0_IRQ>(m0IrqTimeFromXpos166Time(ppu.predictedNextXposTime(166), ppu.cgb(), isDoubleSpeed()));
		
		if (hdmaIsEnabled() && eventTimes_(HDMA_REQ) - cycleCounter > 1) {
			eventTimes_.setm<HDMA_REQ>(nextHdmaTime(ppu.lastM0Time(),
					nextM0Time_.predictedNextM0Time(), cycleCounter, isDoubleSpeed()));
		}
	}
}

static inline unsigned long m0TimeOfCurrentLine(const unsigned long nextLyTime,
		const unsigned long lastM0Time, const unsigned long nextM0Time)
{
	return nextM0Time < nextLyTime ? nextM0Time : lastM0Time;
}

unsigned long LCD::m0TimeOfCurrentLine(const unsigned long cc) {
	if (cc >= nextM0Time_.predictedNextM0Time()) {
		update(cc);
		nextM0Time_.predictNextM0Time(ppu);
	}
	
	return gambatte::m0TimeOfCurrentLine(ppu.lyCounter().time(), ppu.lastM0Time(), nextM0Time_.predictedNextM0Time());
}

static bool isHdmaPeriod(const LyCounter &lyCounter,
		const unsigned long m0TimeOfCurrentLy, const unsigned long cycleCounter)
{
	const unsigned timeToNextLy = lyCounter.time() - cycleCounter;
	
	return /*(ppu.lcdc & 0x80) && */lyCounter.ly() < 144 && timeToNextLy > 4
			&& cycleCounter >= hdmaTimeFromM0Time(m0TimeOfCurrentLy, lyCounter.isDoubleSpeed());
}

void LCD::enableHdma(const unsigned long cycleCounter) {
	if (cycleCounter >= nextM0Time_.predictedNextM0Time()) {
		update(cycleCounter);
		nextM0Time_.predictNextM0Time(ppu);
	} else if (cycleCounter >= eventTimes_.nextEventTime())
		update(cycleCounter);
	
	if (isHdmaPeriod(ppu.lyCounter(),
			gambatte::m0TimeOfCurrentLine(ppu.lyCounter().time(),
				ppu.lastM0Time(), nextM0Time_.predictedNextM0Time()), cycleCounter)) {
		eventTimes_.flagHdmaReq();
	}
	
	eventTimes_.setm<HDMA_REQ>(nextHdmaTime(ppu.lastM0Time(), nextM0Time_.predictedNextM0Time(), cycleCounter, isDoubleSpeed()));
}

void LCD::disableHdma(const unsigned long cycleCounter) {
	if (cycleCounter >= eventTimes_.nextEventTime())
		update(cycleCounter);
	
	eventTimes_.setm<HDMA_REQ>(DISABLED_TIME);
}

bool LCD::vramAccessible(const unsigned long cycleCounter) {
	if (cycleCounter >= eventTimes_.nextEventTime())
		update(cycleCounter);
	
	return !(ppu.lcdc() & 0x80) || ppu.lyCounter().ly() >= 144
			|| ppu.lyCounter().lineCycles(cycleCounter) < 80U
			|| cycleCounter + isDoubleSpeed() - ppu.cgb() + 2 >= m0TimeOfCurrentLine(cycleCounter);
}

bool LCD::cgbpAccessible(const unsigned long cycleCounter) {
	if (cycleCounter >= eventTimes_.nextEventTime())
		update(cycleCounter);
	
	return !(ppu.lcdc() & 0x80) || ppu.lyCounter().ly() >= 144
			|| ppu.lyCounter().lineCycles(cycleCounter) < 80U + isDoubleSpeed()
			|| cycleCounter >= m0TimeOfCurrentLine(cycleCounter) + 3 - isDoubleSpeed();
}

static void doCgbColorChange(unsigned char *const pdata,
		unsigned long *const palette, unsigned index, const unsigned data) {
	pdata[index] = data;
	index >>= 1;
	palette[index] = gbcToRgb32(pdata[index << 1] | pdata[(index << 1) + 1] << 8);
}

void LCD::doCgbBgColorChange(unsigned index, const unsigned data, const unsigned long cycleCounter) {
	if (cgbpAccessible(cycleCounter)) {
		update(cycleCounter);
		doCgbColorChange(bgpData, ppu.bgPalette(), index, data);
	}
}

void LCD::doCgbSpColorChange(unsigned index, const unsigned data, const unsigned long cycleCounter) {
	if (cgbpAccessible(cycleCounter)) {
		update(cycleCounter);
		doCgbColorChange(objpData, ppu.spPalette(), index, data);
	}
}

bool LCD::oamReadable(const unsigned long cycleCounter) {
	if (!(ppu.lcdc() & 0x80) || ppu.inactivePeriodAfterDisplayEnable(cycleCounter))
		return true;
	
	if (cycleCounter >= eventTimes_.nextEventTime())
		update(cycleCounter);

	if (ppu.lyCounter().lineCycles(cycleCounter) + 4 - ppu.lyCounter().isDoubleSpeed() * 3u >= 456)
		return ppu.lyCounter().ly() >= 144-1 && ppu.lyCounter().ly() != 153;

	return ppu.lyCounter().ly() >= 144 || cycleCounter + isDoubleSpeed() - ppu.cgb() + 2 >= m0TimeOfCurrentLine(cycleCounter);
}

bool LCD::oamWritable(const unsigned long cycleCounter) {
	if (!(ppu.lcdc() & 0x80) || ppu.inactivePeriodAfterDisplayEnable(cycleCounter))
		return true;
	
	if (cycleCounter >= eventTimes_.nextEventTime())
		update(cycleCounter);

	if (ppu.lyCounter().lineCycles(cycleCounter) + 3 + ppu.cgb() - ppu.lyCounter().isDoubleSpeed() * 2u >= 456)
		return ppu.lyCounter().ly() >= 144-1 && ppu.lyCounter().ly() != 153;

	return ppu.lyCounter().ly() >= 144 || cycleCounter + isDoubleSpeed() - ppu.cgb() + 2 >= m0TimeOfCurrentLine(cycleCounter);
}

void LCD::mode3CyclesChange() {
	nextM0Time_.invalidatePredictedNextM0Time();

	if (eventTimes_(MODE0_IRQ) != DISABLED_TIME
			&& eventTimes_(MODE0_IRQ) > m0IrqTimeFromXpos166Time(ppu.now(), ppu.cgb(), isDoubleSpeed())) {
		eventTimes_.setm<MODE0_IRQ>(m0IrqTimeFromXpos166Time(ppu.predictedNextXposTime(166), ppu.cgb(), isDoubleSpeed()));
	}

	if (eventTimes_(HDMA_REQ) != DISABLED_TIME
			&& eventTimes_(HDMA_REQ) > hdmaTimeFromM0Time(ppu.lastM0Time(), isDoubleSpeed())) {
		nextM0Time_.predictNextM0Time(ppu);
		eventTimes_.setm<HDMA_REQ>(hdmaTimeFromM0Time(nextM0Time_.predictedNextM0Time(), isDoubleSpeed()));
	}
}

void LCD::wxChange(const unsigned newValue, const unsigned long cycleCounter) {
	update(cycleCounter + isDoubleSpeed() + 1);
	ppu.setWx(newValue);
	mode3CyclesChange();
}

void LCD::wyChange(const unsigned newValue, const unsigned long cycleCounter) {
	update(cycleCounter + 1);
	ppu.setWy(newValue);
// 	mode3CyclesChange(); // should be safe to wait until after wy2 delay, because no mode3 events are close to when wy1 is read.
	
	// wy2 is a delayed version of wy. really just slowness of ly == wy comparison.
	if (ppu.cgb() && (ppu.lcdc() & 0x80)) {
		eventTimes_.setm<ONESHOT_UPDATEWY2>(cycleCounter + 5);
	} else {
		update(cycleCounter + 2);
		ppu.updateWy2();
		mode3CyclesChange();
	}
}

void LCD::scxChange(const unsigned newScx, const unsigned long cycleCounter) {
	update(cycleCounter + ppu.cgb() + isDoubleSpeed());
	ppu.setScx(newScx);
	mode3CyclesChange();
}

void LCD::scyChange(const unsigned newValue, const unsigned long cycleCounter) {
	update(cycleCounter + ppu.cgb() + isDoubleSpeed());
	ppu.setScy(newValue);
}

void LCD::oamChange(const unsigned long cycleCounter) {
	if (ppu.lcdc() & 0x80) {
		update(cycleCounter);
		ppu.oamChange(cycleCounter);
		eventTimes_.setm<SPRITE_MAP>(SpriteMapper::schedule(ppu.lyCounter(), cycleCounter));
	}
}

void LCD::oamChange(const unsigned char *const oamram, const unsigned long cycleCounter) {
	update(cycleCounter);
	ppu.oamChange(oamram, cycleCounter);
	
	if (ppu.lcdc() & 0x80)
		eventTimes_.setm<SPRITE_MAP>(SpriteMapper::schedule(ppu.lyCounter(), cycleCounter));
}

void LCD::lcdcChange(const unsigned data, const unsigned long cycleCounter) {
	const unsigned oldLcdc = ppu.lcdc();
	update(cycleCounter);
	
	if ((oldLcdc ^ data) & 0x80) {
		ppu.setLcdc(data, cycleCounter);
		
		if (data & 0x80) {
			lycIrq.lcdReset();
			m0Irq_.lcdReset(statReg, lycIrq.lycReg());
			
			if (lycIrq.lycReg() == 0 && (statReg & 0x40))
				eventTimes_.flagIrq(2);

			nextM0Time_.predictNextM0Time(ppu);
			lycIrq.reschedule(ppu.lyCounter(), cycleCounter);
			
			eventTimes_.set<LY_COUNT>(ppu.lyCounter().time());
			eventTimes_.setm<SPRITE_MAP>(SpriteMapper::schedule(ppu.lyCounter(), cycleCounter));
			eventTimes_.setm<LYC_IRQ>(lycIrq.time());
			eventTimes_.setm<MODE1_IRQ>(ppu.lyCounter().nextFrameCycle(144 * 456, cycleCounter));
			eventTimes_.setm<MODE2_IRQ>(mode2IrqSchedule(statReg, ppu.lyCounter(), cycleCounter));
			
			if (statReg & 0x08)
				eventTimes_.setm<MODE0_IRQ>(m0IrqTimeFromXpos166Time(ppu.predictedNextXposTime(166), ppu.cgb(), isDoubleSpeed()));
			
			if (hdmaIsEnabled()) {
				eventTimes_.setm<HDMA_REQ>(nextHdmaTime(ppu.lastM0Time(),
						nextM0Time_.predictedNextM0Time(), cycleCounter, isDoubleSpeed()));
			}
		} else for (int i = 0; i < NUM_MEM_EVENTS; ++i)
			eventTimes_.set(static_cast<MemEvent>(i), DISABLED_TIME);
	} else if (data & 0x80) {
		if (ppu.cgb()) {
			ppu.setLcdc((oldLcdc & ~0x14) | (data & 0x14), cycleCounter);
			
			if ((oldLcdc ^ data) & 0x04)
				eventTimes_.setm<SPRITE_MAP>(SpriteMapper::schedule(ppu.lyCounter(), cycleCounter));
			
			update(cycleCounter + isDoubleSpeed() + 1);
			ppu.setLcdc(data, cycleCounter + isDoubleSpeed() + 1);
			
			if ((oldLcdc ^ data) & 0x20)
				mode3CyclesChange();
		} else {
			ppu.setLcdc(data, cycleCounter);
			
			if ((oldLcdc ^ data) & 0x04)
				eventTimes_.setm<SPRITE_MAP>(SpriteMapper::schedule(ppu.lyCounter(), cycleCounter));
			
			if ((oldLcdc ^ data) & 0x22)
				mode3CyclesChange();
		}
	} else
		ppu.setLcdc(data, cycleCounter);
}

void LCD::lcdstatChange(const unsigned data, const unsigned long cycleCounter) {
	if (cycleCounter >= eventTimes_.nextEventTime())
		update(cycleCounter);

	const unsigned old = statReg;
	statReg = data;
	lycIrq.statRegChange(data, ppu.lyCounter(), cycleCounter);
	
	if (ppu.lcdc() & 0x80) {
		if (!ppu.cgb()) {
			const unsigned timeToNextLy = ppu.lyCounter().time() - cycleCounter;
			const unsigned lycCmpLy = ppu.lyCounter().ly() == 153 && timeToNextLy <= 444U + 8 ? 0 : ppu.lyCounter().ly();

			if (lycCmpLy == lycIrq.lycReg()) {
				if (!(old & 0x40) && (ppu.lyCounter().ly() < 144 || !(old & 0x10)))
					eventTimes_.flagIrq(2);
			} else if (ppu.lyCounter().ly() >= 144) {
				if (!(old & 0x10))
					eventTimes_.flagIrq(2);
			} else if (cycleCounter + 1 >= m0TimeOfCurrentLine(cycleCounter)) {
				if (!(old & 0x08))
					eventTimes_.flagIrq(2);
			}
		} else {
			const unsigned timeToNextLy = ppu.lyCounter().time() - cycleCounter;
			const unsigned lycCmpLy = ppu.lyCounter().ly() == 153 && timeToNextLy <= (444U << isDoubleSpeed()) + 8
													? 0 : ppu.lyCounter().ly();
			const bool lycperiod = lycCmpLy == lycIrq.lycReg() && timeToNextLy > 4U - isDoubleSpeed() * 4U;
			const bool lycperiodActive = lycperiod && (old & 0x40);
			
			if (lycperiod && (data & 0x40) - (old & 0x40) == 0x40 && (ppu.lyCounter().ly() < 144 || !(old & 0x10))) {
				eventTimes_.flagIrq(2);
			} else if (ppu.lyCounter().ly() >= 144 && (data & 0x10) - (old & 0x10) == 0x10 &&
					(ppu.lyCounter().ly() < 153 || timeToNextLy > 4U - isDoubleSpeed() * 4U) && !lycperiodActive) {
				eventTimes_.flagIrq(2);
			} else if ((data & 0x08) - (old & 0x08) == 0x08 && ppu.lyCounter().ly() < 144 && !lycperiodActive &&
					timeToNextLy > 4 && cycleCounter + isDoubleSpeed() * 2 >= m0TimeOfCurrentLine(cycleCounter)) {
				eventTimes_.flagIrq(2);
			} else if ((data & 0x28) == 0x20 && !(old & 0x20)
					&& ((timeToNextLy <= 4 && ppu.lyCounter().ly() < 143)
						|| (timeToNextLy == 456*2 && ppu.lyCounter().ly() < 144))) {
				eventTimes_.flagIrq(2);
			}
		}

		if ((data & 0x08) && eventTimes_(MODE0_IRQ) == DISABLED_TIME) {
			update(cycleCounter);
			eventTimes_.setm<MODE0_IRQ>(m0IrqTimeFromXpos166Time(ppu.predictedNextXposTime(166), ppu.cgb(), isDoubleSpeed()));
		}

		eventTimes_.setm<MODE2_IRQ>(mode2IrqSchedule(data, ppu.lyCounter(), cycleCounter));
		eventTimes_.setm<LYC_IRQ>(lycIrq.time());
	}
	
	m2IrqStatReg_ = eventTimes_(MODE2_IRQ) - cycleCounter > (ppu.cgb() - isDoubleSpeed()) * 4U
							? data : (m2IrqStatReg_ & 0x10) | (statReg & ~0x10);
	m1IrqStatReg_ = eventTimes_(MODE1_IRQ) - cycleCounter > (ppu.cgb() - isDoubleSpeed()) * 4U
							? data : (m1IrqStatReg_ & 0x08) | (statReg & ~0x08);
	
	m0Irq_.statRegChange(data, eventTimes_(MODE0_IRQ), cycleCounter, ppu.cgb());
}

void LCD::lycRegChange(const unsigned data, const unsigned long cycleCounter) {
	if (data == lycIrq.lycReg())
		return;

	if (cycleCounter >= eventTimes_.nextEventTime())
		update(cycleCounter);

	m0Irq_.lycRegChange(data, eventTimes_(MODE0_IRQ), cycleCounter, isDoubleSpeed(), ppu.cgb());	
	lycIrq.lycRegChange(data, ppu.lyCounter(), cycleCounter);
	
	if (!(ppu.lcdc() & 0x80))
		return;
	
	eventTimes_.setm<LYC_IRQ>(lycIrq.time());

	const unsigned timeToNextLy = ppu.lyCounter().time() - cycleCounter;
	
	if ((statReg & 0x40) && data < 154 &&
			(ppu.lyCounter().ly() < 144 || !(statReg & 0x10) || (ppu.lyCounter().ly() == 153 && timeToNextLy <= 4U))) {
		unsigned lycCmpLy = ppu.lyCounter().ly();
		
		if (ppu.cgb()) {
			if (timeToNextLy <= 4U >> isDoubleSpeed()) {
				lycCmpLy = lycCmpLy < 153 ? lycCmpLy + 1 : 0;
			} else if (timeToNextLy <= 8) {
				lycCmpLy = 0xFF;
			} else if (lycCmpLy == 153 && timeToNextLy <= (448U << isDoubleSpeed()) + 8)
				lycCmpLy = 0;
		} else {
			if (timeToNextLy <= 4) {
				lycCmpLy = 0xFF;
			} else if (lycCmpLy == 153 && timeToNextLy <= 444U + 8)
				lycCmpLy = 0;
		}
		
		if (data == lycCmpLy) {
			if (ppu.cgb() && !isDoubleSpeed()) {
				eventTimes_.setm<ONESHOT_LCDSTATIRQ>(cycleCounter + 5);
			} else
				eventTimes_.flagIrq(2);
		}
	}
}

unsigned LCD::getStat(const unsigned lycReg, const unsigned long cycleCounter) {
	unsigned stat = 0;

	if (ppu.lcdc() & 0x80) {
		if (cycleCounter >= eventTimes_.nextEventTime())
			update(cycleCounter);

		const unsigned timeToNextLy = ppu.lyCounter().time() - cycleCounter;

		if (ppu.lyCounter().ly() > 143) {
			if (ppu.lyCounter().ly() < 153 || timeToNextLy > 4 - isDoubleSpeed() * 4U)
				stat = 1;
		} else {
			const unsigned lineCycles = 456 - (timeToNextLy >> isDoubleSpeed());

			if (lineCycles < 80) {
				if (!ppu.inactivePeriodAfterDisplayEnable(cycleCounter))
					stat = 2;
			} else if (cycleCounter + isDoubleSpeed() - ppu.cgb() + 2 < m0TimeOfCurrentLine(cycleCounter))
				stat = 3;
		}

		if ((ppu.lyCounter().ly() == lycReg && timeToNextLy > 4 - isDoubleSpeed() * 4U) ||
				(lycReg == 0 && ppu.lyCounter().ly() == 153 && timeToNextLy >> isDoubleSpeed() <= 456 - 8)) {
			stat |= 4;
		}
	}

	return stat;
}

inline void LCD::doMode2IrqEvent() {
	const unsigned ly = eventTimes_(LY_COUNT) - eventTimes_(MODE2_IRQ) < 8
			? (ppu.lyCounter().ly() == 153 ? 0 : ppu.lyCounter().ly() + 1)
			: ppu.lyCounter().ly();
	
	if ((ly != 0 || !(m2IrqStatReg_ & 0x10)) &&
			(!(m2IrqStatReg_ & 0x40) || (lycIrq.lycReg() != 0 ? ly != (lycIrq.lycReg() + 1U) : ly > 1))) {
		eventTimes_.flagIrq(2);
	}
	
	m2IrqStatReg_ = statReg;
	
	if (!(statReg & 0x08)) {
		unsigned long nextTime = eventTimes_(MODE2_IRQ) + ppu.lyCounter().lineTime();
		
		if (ly == 0) {
			nextTime -= 4;
		} else if (ly == 143)
			nextTime += ppu.lyCounter().lineTime() * 10 + 4;
		
		eventTimes_.setm<MODE2_IRQ>(nextTime);
	} else
		eventTimes_.setm<MODE2_IRQ>(eventTimes_(MODE2_IRQ) + (70224 << isDoubleSpeed()));
}

inline void LCD::event() {
	switch (eventTimes_.nextEvent()) {
	case MEM_EVENT:
		switch (eventTimes_.nextMemEvent()) {
		case MODE1_IRQ:
			eventTimes_.flagIrq((m1IrqStatReg_ & 0x18) == 0x10 ? 3 : 1);
			m1IrqStatReg_ = statReg;
			eventTimes_.setm<MODE1_IRQ>(eventTimes_(MODE1_IRQ) + (70224 << isDoubleSpeed()));
			break;
			
		case LYC_IRQ: {
			unsigned char ifreg = 0;
			lycIrq.doEvent(&ifreg, ppu.lyCounter());
			eventTimes_.flagIrq(ifreg);
			eventTimes_.setm<LYC_IRQ>(lycIrq.time());
			break;
		}
		
		case SPRITE_MAP:
			eventTimes_.setm<SPRITE_MAP>(ppu.doSpriteMapEvent(eventTimes_(SPRITE_MAP)));
			mode3CyclesChange();
			break;
		
		case HDMA_REQ:
			eventTimes_.flagHdmaReq();
			nextM0Time_.predictNextM0Time(ppu);
			eventTimes_.setm<HDMA_REQ>(hdmaTimeFromM0Time(nextM0Time_.predictedNextM0Time(), isDoubleSpeed()));
			break;
		
		case MODE2_IRQ:
			doMode2IrqEvent();
			break;
		
		case MODE0_IRQ:
			{
				unsigned char ifreg = 0;
				m0Irq_.doEvent(&ifreg, ppu.lyCounter().ly(), statReg, lycIrq.lycReg());
				eventTimes_.flagIrq(ifreg);
			}
			
			eventTimes_.setm<MODE0_IRQ>((statReg & 0x08)
					? m0IrqTimeFromXpos166Time(ppu.predictedNextXposTime(166), ppu.cgb(), isDoubleSpeed())
					: static_cast<unsigned long>(DISABLED_TIME));
			break;
		
		case ONESHOT_LCDSTATIRQ:
			eventTimes_.flagIrq(2);
			eventTimes_.setm<ONESHOT_LCDSTATIRQ>(DISABLED_TIME);
			break;
		
		case ONESHOT_UPDATEWY2:
			ppu.updateWy2();
			mode3CyclesChange();
			eventTimes_.setm<ONESHOT_UPDATEWY2>(DISABLED_TIME);
			break;
		}
		
		break;
		
	case LY_COUNT:
		ppu.doLyCountEvent();
		eventTimes_.set<LY_COUNT>(ppu.lyCounter().time());
		break;
	}
}

void LCD::update(const unsigned long cycleCounter) {
	if (!(ppu.lcdc() & 0x80))
		return;
	
	while (cycleCounter >= eventTimes_.nextEventTime()) {
		ppu.update(eventTimes_.nextEventTime());
		event();
	}
	
	ppu.update(cycleCounter);
}

void LCD::setVideoBuffer(uint_least32_t *const videoBuf, const int pitch) {
	ppu.setFrameBuf(videoBuf, pitch);
}

void LCD::setDmgPaletteColor(const unsigned index, const unsigned long rgb32) {
	dmgColorsRgb32[index] = rgb32;
}

void LCD::setDmgPaletteColor(const unsigned palNum, const unsigned colorNum, const unsigned long rgb32) {
	if (palNum > 2 || colorNum > 3)
		return;

	setDmgPaletteColor(palNum * 4 | colorNum, rgb32);
	refreshPalettes();
}

}
