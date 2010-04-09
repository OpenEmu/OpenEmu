/***************************************************************************
 *   Copyright (C) 2007 by Sindre Aam�s                                    *
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
#include "videoblitter.h"
#include "video/filters/filter.h"
#include "video/filters/catrom2x.h"
#include "video/filters/catrom3x.h"
#include "video/filters/kreed2xsai.h"
#include "video/filters/maxsthq2x.h"
#include "video/filters/maxsthq3x.h"
#include "filterinfo.h"
#include "savestate.h"
#include "video/basic_add_event.h"
#include <cstring>
#include <algorithm>

static void addEventIfActivated(event_queue<VideoEvent*,VideoEventComparer> &q, VideoEvent *const e, const unsigned long newTime) {
	e->setTime(newTime);

	if (newTime != VideoEvent::DISABLED_TIME)
		q.push(e);
}

void LCD::setDmgPalette(unsigned long *const palette, const unsigned long *const dmgColors, const unsigned data) {
	palette[0] = dmgColors[data & 3];
	palette[1] = dmgColors[data >> 2 & 3];
	palette[2] = dmgColors[data >> 4 & 3];
	palette[3] = dmgColors[data >> 6 & 3];
}

unsigned long LCD::gbcToRgb32(const unsigned bgr15) {
	const unsigned long r = bgr15 & 0x1F;
	const unsigned long g = bgr15 >> 5 & 0x1F;
	const unsigned long b = bgr15 >> 10 & 0x1F;

	return ((r * 13 + g * 2 + b) >> 1) << 16 | ((g * 3 + b) << 9) | ((r * 3 + g * 2 + b * 11) >> 1);
}

unsigned long LCD::gbcToRgb16(const unsigned bgr15) {
	const unsigned r = bgr15 & 0x1F;
	const unsigned g = bgr15 >> 5 & 0x1F;
	const unsigned b = bgr15 >> 10 & 0x1F;

	return (((r * 13 + g * 2 + b + 8) << 7) & 0xF800) | ((g * 3 + b + 1) >> 1) << 5 | ((r * 3 + g * 2 + b * 11 + 8) >> 4);
}

unsigned long LCD::gbcToUyvy(const unsigned bgr15) {
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
}

LCD::LCD(const unsigned char *const oamram, const unsigned char *const vram_in) :
	vram(vram_in),
	bgTileData(vram),
	bgTileMap(vram + 0x1800),
	wdTileMap(bgTileMap),
	vBlitter(NULL),
	filter(NULL),
	dbuffer(NULL),
	draw(NULL),
	gbcToFormat(gbcToRgb32),
	dmgColors(dmgColorsRgb32),
	lastUpdate(0),
	videoCycles(0),
	dpitch(0),
	winYPos(0),
	m3EventQueue(11, VideoEventComparer()),
	irqEventQueue(4, VideoEventComparer()),
	vEventQueue(5, VideoEventComparer()),
	win(m3EventQueue, lyCounter, m3ExtraCycles),
	scxReader(m3EventQueue, /*wyReg.reader3(),*/ win.wxReader, win.we.enableChecker(), win.we.disableChecker(), m3ExtraCycles),
	spriteMapper(m3ExtraCycles, lyCounter, oamram),
	m3ExtraCycles(spriteMapper, scxReader, win),
	breakEvent(drawStartCycle, scReadOffset),
	mode3Event(m3EventQueue, vEventQueue, mode0Irq, irqEvent),
	lycIrq(ifReg),
	mode0Irq(lyCounter, lycIrq, m3ExtraCycles, ifReg),
	mode1Irq(ifReg),
	mode2Irq(lyCounter, lycIrq, ifReg),
	irqEvent(irqEventQueue),
	drawStartCycle(90),
	scReadOffset(90),
	ifReg(0),
	tileIndexSign(0),
	statReg(0),
	doubleSpeed(false),
	enabled(false),
	cgb(false),
	bgEnable(false),
	spriteEnable(false)
{
	std::memset(bgpData, 0, sizeof(bgpData));
	std::memset(objpData, 0, sizeof(objpData));

	for (unsigned i = 0; i < sizeof(dmgColorsRgb32) / sizeof(unsigned long); ++i) {
		setDmgPaletteColor(i, (3 - (i & 3)) * 85 * 0x010101);
	}

	filters.push_back(NULL);
	filters.push_back(new Catrom2x);
	filters.push_back(new Catrom3x);
	filters.push_back(new Kreed_2xSaI);
	filters.push_back(new MaxSt_Hq2x);
	filters.push_back(new MaxSt_Hq3x);

	reset(oamram, false);
	setDoubleSpeed(false);

	setVideoFilter(0);
}

LCD::~LCD() {
// 	delete []filter_buffer;
	for (std::size_t i = 0; i < filters.size(); ++i)
		delete filters[i];
}

void LCD::reset(const unsigned char *const oamram, const bool cgb_in) {
	cgb = cgb_in;
	spriteMapper.reset(oamram, cgb_in);
	setDBuffer();
}

void LCD::resetVideoState(const unsigned long cycleCounter) {
	vEventQueue.clear();
	m3EventQueue.clear();
	irqEventQueue.clear();

	lyCounter.reset(videoCycles, lastUpdate);
	vEventQueue.push(&lyCounter);

	spriteMapper.resetVideoState();
	m3ExtraCycles.invalidateCache();

	addEventIfActivated(m3EventQueue, &scxReader, ScxReader::schedule(lyCounter, cycleCounter));
	addEventIfActivated(m3EventQueue, &win.wxReader, WxReader::schedule(scxReader.scxAnd7(), lyCounter, win.wxReader, cycleCounter));
	addEventIfActivated(m3EventQueue, &win.wyReg.reader1(), Wy::WyReader1::schedule(lyCounter, cycleCounter));
	addEventIfActivated(m3EventQueue, &win.wyReg.reader2(), Wy::WyReader2::schedule(lyCounter, cycleCounter));
	addEventIfActivated(m3EventQueue, &win.wyReg.reader3(), Wy::WyReader3::schedule(win.wxReader.getSource(), scxReader, lyCounter, cycleCounter));
	addEventIfActivated(m3EventQueue, &win.wyReg.reader4(), Wy::WyReader4::schedule(lyCounter, cycleCounter));
	addEventIfActivated(m3EventQueue, &spriteMapper, SpriteMapper::schedule(lyCounter, cycleCounter));
	addEventIfActivated(m3EventQueue, &win.we.enableChecker(), We::WeEnableChecker::schedule(scxReader.scxAnd7(), win.wxReader.wx(), lyCounter, cycleCounter));
	addEventIfActivated(m3EventQueue, &win.we.disableChecker(), We::WeDisableChecker::schedule(scxReader.scxAnd7(), win.wxReader.wx(), lyCounter, cycleCounter));
	addEventIfActivated(m3EventQueue, &win.weMasterChecker, WeMasterChecker::schedule(win.wyReg.getSource(), win.we.getSource(), lyCounter, cycleCounter));

	addEventIfActivated(irqEventQueue, &lycIrq, LycIrq::schedule(statReg, lycIrq.lycReg(), lyCounter, cycleCounter));
	addEventIfActivated(irqEventQueue, &mode0Irq, Mode0Irq::schedule(statReg, m3ExtraCycles, lyCounter, cycleCounter));
	addEventIfActivated(irqEventQueue, &mode1Irq, Mode1Irq::schedule(lyCounter, cycleCounter));
	addEventIfActivated(irqEventQueue, &mode2Irq, Mode2Irq::schedule(statReg, lyCounter, cycleCounter));

	addEventIfActivated(vEventQueue, &mode3Event, Mode3Event::schedule(m3EventQueue));
	addEventIfActivated(vEventQueue, &irqEvent, IrqEvent::schedule(irqEventQueue));
	addEventIfActivated(vEventQueue, &scReader, ScReader::schedule(lastUpdate, videoCycles, scReadOffset, doubleSpeed));
	addEventIfActivated(vEventQueue, &breakEvent, BreakEvent::schedule(lyCounter));
}

void LCD::setDoubleSpeed(const bool ds) {
	doubleSpeed = ds;
	lyCounter.setDoubleSpeed(doubleSpeed);
	scxReader.setDoubleSpeed(doubleSpeed);
	win.wxReader.setDoubleSpeed(doubleSpeed);
	scReader.setDoubleSpeed(doubleSpeed);
	breakEvent.setDoubleSpeed(doubleSpeed);
	lycIrq.setDoubleSpeed(doubleSpeed);
	mode1Irq.setDoubleSpeed(doubleSpeed);
}

void LCD::setStatePtrs(SaveState &state) {
	state.ppu.drawBuffer.set(static_cast<Gambatte::uint_least32_t*>(dbuffer), dpitch * 144);
	state.ppu.bgpData.set(bgpData, sizeof bgpData);
	state.ppu.objpData.set(objpData, sizeof objpData);
	spriteMapper.setStatePtrs(state);
}

void LCD::saveState(SaveState &state) const {
	state.ppu.videoCycles = videoCycles;
	state.ppu.winYPos = winYPos;
	state.ppu.drawStartCycle = drawStartCycle;
	state.ppu.scReadOffset = scReadOffset;
	state.ppu.lcdc = enabled << 7 | ((wdTileMap - vram - 0x1800) >> 4) | (tileIndexSign ^ 0x80) >> 3 | ((bgTileMap - vram - 0x1800) >> 7) | spriteEnable << 1 | bgEnable;
	state.ppu.lycIrqSkip = lycIrq.skips();

	spriteMapper.saveState(state);
	scReader.saveState(state);
	scxReader.saveState(state);
	win.weMasterChecker.saveState(state);
	win.wxReader.saveState(state);
	win.wyReg.saveState(state);
	win.we.saveState(state);
}

void LCD::loadState(const SaveState &state, const unsigned char *oamram) {
	statReg = state.mem.ioamhram.get()[0x141];
	ifReg = 0;

	setDoubleSpeed(cgb & state.mem.ioamhram.get()[0x14D] >> 7);

	lastUpdate = state.cpu.cycleCounter;
	videoCycles = std::min(state.ppu.videoCycles, 70223ul);
	winYPos = state.ppu.winYPos > 143 ? 0xFF : state.ppu.winYPos;
	drawStartCycle = state.ppu.drawStartCycle;
	scReadOffset = state.ppu.scReadOffset;
	enabled = state.ppu.lcdc >> 7 & 1;
	wdTileMap = vram + 0x1800 + (state.ppu.lcdc >> 6 & 1) * 0x400;
	tileIndexSign = ((state.ppu.lcdc >> 4 & 1) ^ 1) * 0x80;
	bgTileData = vram + ((state.ppu.lcdc >> 4 & 1) ^ 1) * 0x1000;
	bgTileMap = vram + 0x1800 + (state.ppu.lcdc >> 3 & 1) * 0x400;
	spriteEnable = state.ppu.lcdc >> 1 & 1;
	bgEnable = state.ppu.lcdc & 1;

	lycIrq.setM2IrqEnabled(statReg >> 5 & 1);
	lycIrq.setLycReg(state.mem.ioamhram.get()[0x145]);
	lycIrq.setSkip(state.ppu.lycIrqSkip);
	mode1Irq.setM1StatIrqEnabled(statReg >> 4 & 1);

	win.we.setSource(state.mem.ioamhram.get()[0x140] >> 5 & 1);
	spriteMapper.setLargeSpritesSource(state.mem.ioamhram.get()[0x140] >> 2 & 1);
	scReader.setScySource(state.mem.ioamhram.get()[0x142]);
	scxReader.setSource(state.mem.ioamhram.get()[0x143]);
	breakEvent.setScxSource(state.mem.ioamhram.get()[0x143]);
	scReader.setScxSource(state.mem.ioamhram.get()[0x143]);
	win.wyReg.setSource(state.mem.ioamhram.get()[0x14A]);
	win.wxReader.setSource(state.mem.ioamhram.get()[0x14B]);

	spriteMapper.loadState(state);
	scReader.loadState(state);
	scxReader.loadState(state);
	win.weMasterChecker.loadState(state);
	win.wxReader.loadState(state);
	win.wyReg.loadState(state);
	win.we.loadState(state);

	resetVideoState(lastUpdate);
	spriteMapper.oamChange(oamram, lastUpdate);
	refreshPalettes();
}

void LCD::refreshPalettes() {
	if (cgb) {
		for (unsigned i = 0; i < 8 * 8; i += 2) {
			bgPalette[i >> 1] = (*gbcToFormat)(bgpData[i] | bgpData[i + 1] << 8);
			spPalette[i >> 1] = (*gbcToFormat)(objpData[i] | objpData[i + 1] << 8);
		}
	} else {
		setDmgPalette(bgPalette, dmgColors, bgpData[0]);
		setDmgPalette(spPalette, dmgColors + 4, objpData[0]);
		setDmgPalette(spPalette + 4, dmgColors + 8, objpData[1]);
	}
}

void LCD::setVideoBlitter(Gambatte::VideoBlitter *vb) {
	vBlitter = vb;

	if (vBlitter) {
		vBlitter->setBufferDimensions(videoWidth(), videoHeight());
		pb = vBlitter->inBuffer();
	}

	setDBuffer();
}

void LCD::videoBufferChange() {
	if (vBlitter) {
		pb = vBlitter->inBuffer();
		setDBuffer();
	}
}

void LCD::setVideoFilter(const unsigned n) {
	const unsigned oldw = videoWidth();
	const unsigned oldh = videoHeight();

	if (filter)
		filter->outit();

	filter = filters.at(n < filters.size() ? n : 0);

	if (filter) {
		filter->init();
	}

	if (vBlitter && (oldw != videoWidth() || oldh != videoHeight())) {
		vBlitter->setBufferDimensions(videoWidth(), videoHeight());
		pb = vBlitter->inBuffer();
	}

	setDBuffer();
}

std::vector<const Gambatte::FilterInfo*> LCD::filterInfo() const {
	std::vector<const Gambatte::FilterInfo*> v;

	static Gambatte::FilterInfo noInfo = { "None", 160, 144 };
	v.push_back(&noInfo);

	for (std::size_t i = 1; i < filters.size(); ++i)
		v.push_back(&filters[i]->info());

	return v;
}

unsigned int LCD::videoWidth() const {
	return filter ? filter->info().outWidth : 160;
}

unsigned int LCD::videoHeight() const {
	return filter ? filter->info().outHeight : 144;
}

template<class Blend>
static void blitOsdElement(Gambatte::uint_least32_t *d, const Gambatte::uint_least32_t *s, const unsigned width, unsigned h, const unsigned dpitch, Blend blend) {
	while (h--) {
		for (unsigned w = width; w--;) {
			if (*s != 0xFFFFFFFF)
				*d = blend(*s, *d);

			++d;
			++s;
		}

		d += dpitch - width;
	}
}

template<const unsigned weight>
struct Blend {
	enum { SW = weight - 1 };
	enum { LOWMASK = SW * 0x010101ul };
	Gambatte::uint_least32_t operator()(const Gambatte::uint_least32_t s, const Gambatte::uint_least32_t d) const {
		return (s * SW + d - (((s & LOWMASK) * SW + (d & LOWMASK)) & LOWMASK)) / weight;
	}
};

void LCD::updateScreen(const unsigned long cycleCounter) {
	update(cycleCounter);

	if (pb.pixels) {
		if (dbuffer && osdElement.get()) {
			const Gambatte::uint_least32_t *s = osdElement->update();

			if (s) {
				Gambatte::uint_least32_t *d = static_cast<Gambatte::uint_least32_t*>(dbuffer) + osdElement->y() * dpitch + osdElement->x();

				switch (osdElement->opacity()) {
				case OsdElement::SEVEN_EIGHTHS: blitOsdElement(d, s, osdElement->w(), osdElement->h(), dpitch, Blend<8>()); break;
				case OsdElement::THREE_FOURTHS: blitOsdElement(d, s, osdElement->w(), osdElement->h(), dpitch, Blend<4>()); break;
				}
			} else
				osdElement.reset();
		}

		if (filter) {
			filter->filter(static_cast<Gambatte::uint_least32_t*>(tmpbuf ? tmpbuf : pb.pixels), (tmpbuf ? videoWidth() : pb.pitch));
		}

		if (tmpbuf) {
			switch (pb.format) {
			case Gambatte::PixelBuffer::RGB16:
				rgb32ToRgb16(tmpbuf, static_cast<Gambatte::uint_least16_t*>(pb.pixels), videoWidth(), videoHeight(), pb.pitch);
				break;
			case Gambatte::PixelBuffer::UYVY:
				rgb32ToUyvy(tmpbuf, static_cast<Gambatte::uint_least32_t*>(pb.pixels), videoWidth(), videoHeight(), pb.pitch);
				break;
			default: break;
			}
		}

		if (vBlitter)
			vBlitter->blit();
	}
}

template<typename T>
static void clear(T *buf, const unsigned long color, const unsigned dpitch) {
	unsigned lines = 144;

	while (lines--) {
		std::fill_n(buf, 160, color);
		buf += dpitch;
	}
}

void LCD::enableChange(const unsigned long cycleCounter) {
	update(cycleCounter);
	enabled = !enabled;

	if (enabled) {
		lycIrq.setSkip(false);
		videoCycles = 0;
		lastUpdate = cycleCounter;
		winYPos = 0xFF;
		win.weMasterChecker.unset();
		spriteMapper.enableDisplay(cycleCounter);
		resetVideoState(cycleCounter);
	}

	if (!enabled && dbuffer) {
		const unsigned long color = cgb ? (*gbcToFormat)(0xFFFF) : dmgColors[0];

		clear(static_cast<Gambatte::uint_least32_t*>(dbuffer), color, dpitch);

// 		updateScreen(cycleCounter);
	}
}

//FIXME: needs testing
void LCD::lyWrite(const unsigned long cycleCounter) {
	update(cycleCounter);
	lycIrq.setSkip(false);
	videoCycles = 0;
	lastUpdate = cycleCounter;
	winYPos = 0xFF;
	win.weMasterChecker.unset();
	resetVideoState(cycleCounter);

// 	if ((statReg & 0x40) && lycIrq.lycReg() == 0)
// 		ifReg |= 2;
}

void LCD::preResetCounter(const unsigned long cycleCounter) {
	preSpeedChange(cycleCounter);
}

void LCD::postResetCounter(const unsigned long oldCC, const unsigned long cycleCounter) {
	lastUpdate = cycleCounter - (oldCC - lastUpdate);
	spriteMapper.resetCycleCounter(oldCC, cycleCounter);
	resetVideoState(cycleCounter);
}

void LCD::preSpeedChange(const unsigned long cycleCounter) {
	update(cycleCounter);
	spriteMapper.preCounterChange(cycleCounter);
}

void LCD::postSpeedChange(const unsigned long cycleCounter) {
	setDoubleSpeed(!doubleSpeed);

	resetVideoState(cycleCounter);
}

bool LCD::isMode0IrqPeriod(const unsigned long cycleCounter) {
	if (cycleCounter >= vEventQueue.top()->time())
		update(cycleCounter);

	const unsigned timeToNextLy = lyCounter.time() - cycleCounter;

	return /*memory.enable_display && */lyCounter.ly() < 144 && timeToNextLy <= (456U - (169 + doubleSpeed * 3U + 80 + m3ExtraCycles(lyCounter.ly()) + 1 - doubleSpeed)) << doubleSpeed && timeToNextLy > 4;
}

bool LCD::isMode2IrqPeriod(const unsigned long cycleCounter) {
	if (cycleCounter >= lyCounter.time())
		update(cycleCounter);

	const unsigned nextLy = lyCounter.time() - cycleCounter;

	return /*memory.enable_display && */lyCounter.ly() < 143 && nextLy <= 4;
}

bool LCD::isLycIrqPeriod(const unsigned lycReg, const unsigned endCycles, const unsigned long cycleCounter) {
	if (cycleCounter >= lyCounter.time())
		update(cycleCounter);

	const unsigned timeToNextLy = lyCounter.time() - cycleCounter;

	return (lyCounter.ly() == lycReg && timeToNextLy > endCycles) || (lycReg == 0 && lyCounter.ly() == 153 && timeToNextLy <= (456U - 8U) << doubleSpeed);
}

bool LCD::isMode1IrqPeriod(const unsigned long cycleCounter) {
	if (cycleCounter >= lyCounter.time())
		update(cycleCounter);

	const unsigned timeToNextLy = lyCounter.time() - cycleCounter;

	return lyCounter.ly() > 143 && (lyCounter.ly() < 153 || timeToNextLy > 4U - doubleSpeed * 4U);
}

bool LCD::isHdmaPeriod(const unsigned long cycleCounter) {
	if (cycleCounter >= vEventQueue.top()->time())
		update(cycleCounter);

	const unsigned timeToNextLy = lyCounter.time() - cycleCounter;

	return /*memory.enable_display && */lyCounter.ly() < 144 && timeToNextLy <= ((456U - (169U + doubleSpeed * 3U + 80U + m3ExtraCycles(lyCounter.ly()) + 2 - doubleSpeed)) << doubleSpeed) && timeToNextLy > 4;
}

unsigned long LCD::nextHdmaTime(const unsigned long cycleCounter) {
	if (cycleCounter >= vEventQueue.top()->time())
		update(cycleCounter);

	unsigned line = lyCounter.ly();
	int next = static_cast<int>(169 + doubleSpeed * 3U + 80 + 2 - doubleSpeed) - static_cast<int>(lyCounter.lineCycles(cycleCounter));

	if (line < 144 && next + static_cast<int>(m3ExtraCycles(line)) <= 0) {
		next += 456;
		++line;
	}

	if (line > 143) {
		next += (154 - line) * 456;
		line = 0;
	}

	next += m3ExtraCycles(line);

	return cycleCounter + (static_cast<unsigned>(next) << doubleSpeed);
}

bool LCD::vramAccessible(const unsigned long cycleCounter) {
	if (cycleCounter >= vEventQueue.top()->time())
		update(cycleCounter);

	bool accessible = true;

	if (enabled && lyCounter.ly() < 144) {
		const unsigned lineCycles = lyCounter.lineCycles(cycleCounter);

		if (lineCycles > 79 && lineCycles < 80 + 169 + doubleSpeed * 3U + m3ExtraCycles(lyCounter.ly()))
			accessible = false;
	}

	return accessible;
}

bool LCD::cgbpAccessible(const unsigned long cycleCounter) {
	if (cycleCounter >= vEventQueue.top()->time())
		update(cycleCounter);

	bool accessible = true;

	if (enabled && lyCounter.ly() < 144) {
		const unsigned lineCycles = lyCounter.lineCycles(cycleCounter);

		if (lineCycles > 79U + doubleSpeed && lineCycles < 80U + 169U + doubleSpeed * 3U + m3ExtraCycles(lyCounter.ly()) + 4U - doubleSpeed * 2U)
			accessible = false;
	}

	return accessible;
}

bool LCD::oamAccessible(const unsigned long cycleCounter) {
	bool accessible = true;

	if (enabled) {
		if (cycleCounter >= vEventQueue.top()->time())
			update(cycleCounter);

		accessible = spriteMapper.oamAccessible(cycleCounter);
	}

	return accessible;
}

void LCD::weChange(const bool newValue, const unsigned long cycleCounter) {
	if (cycleCounter >= vEventQueue.top()->time())
		update(cycleCounter);

	win.we.setSource(newValue);
	addFixedtimeEvent(m3EventQueue, &win.weMasterChecker, WeMasterChecker::schedule(win.wyReg.getSource(), newValue, lyCounter, cycleCounter));
	addFixedtimeEvent(m3EventQueue, &win.we.disableChecker(), We::WeDisableChecker::schedule(scxReader.scxAnd7(), win.wxReader.wx(), lyCounter, cycleCounter));
	addFixedtimeEvent(m3EventQueue, &win.we.enableChecker(), We::WeEnableChecker::schedule(scxReader.scxAnd7(), win.wxReader.wx(), lyCounter, cycleCounter));
	addUnconditionalEvent(vEventQueue, &mode3Event, Mode3Event::schedule(m3EventQueue));
}

void LCD::wxChange(const unsigned newValue, const unsigned long cycleCounter) {
	if (cycleCounter >= vEventQueue.top()->time())
		update(cycleCounter);

	win.wxReader.setSource(newValue);
	addEvent(m3EventQueue, &win.wxReader, WxReader::schedule(scxReader.scxAnd7(), lyCounter, win.wxReader, cycleCounter));

	if (win.wyReg.reader3().time() != VideoEvent::DISABLED_TIME)
		addEvent(m3EventQueue, &win.wyReg.reader3(), Wy::WyReader3::schedule(win.wxReader.getSource(), scxReader, lyCounter, cycleCounter));

	addUnconditionalEvent(vEventQueue, &mode3Event, Mode3Event::schedule(m3EventQueue));
}

void LCD::wyChange(const unsigned newValue, const unsigned long cycleCounter) {
	if (cycleCounter >= vEventQueue.top()->time())
		update(cycleCounter);

	win.wyReg.setSource(newValue);
	addFixedtimeEvent(m3EventQueue, &win.wyReg.reader1(), Wy::WyReader1::schedule(lyCounter, cycleCounter));
	addFixedtimeEvent(m3EventQueue, &win.wyReg.reader2(), Wy::WyReader2::schedule(lyCounter, cycleCounter));
	addFixedtimeEvent(m3EventQueue, &win.wyReg.reader3(), Wy::WyReader3::schedule(win.wxReader.getSource(), scxReader, lyCounter, cycleCounter));
	addFixedtimeEvent(m3EventQueue, &win.wyReg.reader4(), Wy::WyReader4::schedule(lyCounter, cycleCounter));
	addEvent(m3EventQueue, &win.weMasterChecker, WeMasterChecker::schedule(win.wyReg.getSource(), win.we.getSource(), lyCounter, cycleCounter));
	addUnconditionalEvent(vEventQueue, &mode3Event, Mode3Event::schedule(m3EventQueue));
}

void LCD::scxChange(const unsigned newScx, const unsigned long cycleCounter) {
	update(cycleCounter);

	scxReader.setSource(newScx);
	breakEvent.setScxSource(newScx);
	scReader.setScxSource(newScx);

	addFixedtimeEvent(m3EventQueue, &scxReader, ScxReader::schedule(lyCounter, cycleCounter));

	if (win.wyReg.reader3().time() != VideoEvent::DISABLED_TIME)
		addEvent(m3EventQueue, &win.wyReg.reader3(), Wy::WyReader3::schedule(win.wxReader.getSource(), scxReader, lyCounter, cycleCounter));

	addUnconditionalEvent(vEventQueue, &mode3Event, Mode3Event::schedule(m3EventQueue));

	const unsigned lineCycles = lyCounter.lineCycles(cycleCounter);

	if (lineCycles < 82U + doubleSpeed * 4U)
		drawStartCycle = 90 + doubleSpeed * 4U + (newScx & 7);
	else
		addFixedtimeEvent(vEventQueue, &breakEvent, BreakEvent::schedule(lyCounter));

	if (lineCycles < 86U + doubleSpeed * 2U)
		scReadOffset = std::max(drawStartCycle - (newScx & 7), 90U + doubleSpeed * 4U);

	addEvent(vEventQueue, &scReader, ScReader::schedule(lastUpdate, videoCycles, scReadOffset, doubleSpeed));
}

void LCD::scyChange(const unsigned newValue, const unsigned long cycleCounter) {
	update(cycleCounter);

	scReader.setScySource(newValue);
	addFixedtimeEvent(vEventQueue, &scReader, ScReader::schedule(lastUpdate, videoCycles, scReadOffset, doubleSpeed));
}

void LCD::spriteSizeChange(const bool newLarge, const unsigned long cycleCounter) {
	update(cycleCounter);

	spriteMapper.oamChange(cycleCounter);
	spriteMapper.setLargeSpritesSource(newLarge);
	addFixedtimeEvent(m3EventQueue, &spriteMapper, SpriteMapper::schedule(lyCounter, cycleCounter));
	addUnconditionalEvent(vEventQueue, &mode3Event, Mode3Event::schedule(m3EventQueue));
}

void LCD::oamChange(const unsigned long cycleCounter) {
	update(cycleCounter);

	spriteMapper.oamChange(cycleCounter);
	addFixedtimeEvent(m3EventQueue, &spriteMapper, SpriteMapper::schedule(lyCounter, cycleCounter));
	addUnconditionalEvent(vEventQueue, &mode3Event, Mode3Event::schedule(m3EventQueue));
}

void LCD::oamChange(const unsigned char *const oamram, const unsigned long cycleCounter) {
	update(cycleCounter);

	spriteMapper.oamChange(oamram, cycleCounter);
	addFixedtimeEvent(m3EventQueue, &spriteMapper, SpriteMapper::schedule(lyCounter, cycleCounter));
	addUnconditionalEvent(vEventQueue, &mode3Event, Mode3Event::schedule(m3EventQueue));
}

void LCD::wdTileMapSelectChange(const bool newValue, const unsigned long cycleCounter) {
	update(cycleCounter);

	wdTileMap = vram + 0x1800 + newValue * 0x400;
}

void LCD::bgTileMapSelectChange(const bool newValue, const unsigned long cycleCounter) {
	update(cycleCounter);

	bgTileMap = vram + 0x1800 + newValue * 0x400;
}

void LCD::bgTileDataSelectChange(const bool newValue, const unsigned long cycleCounter) {
	update(cycleCounter);

	tileIndexSign = (newValue ^ 1) * 0x80;
	bgTileData = vram + (newValue ^ 1) * 0x1000;
}

void LCD::spriteEnableChange(const bool newValue, const unsigned long cycleCounter) {
	update(cycleCounter);

	spriteEnable = newValue;
}

void LCD::bgEnableChange(const bool newValue, const unsigned long cycleCounter) {
	update(cycleCounter);

	bgEnable = newValue;
}

void LCD::lcdstatChange(const unsigned data, const unsigned long cycleCounter) {
	if (cycleCounter >= vEventQueue.top()->time())
		update(cycleCounter);

	const unsigned old = statReg;
	statReg = data;
	mode1Irq.setM1StatIrqEnabled(data & 0x10);
	lycIrq.setM2IrqEnabled(data & 0x20);

	if (!enabled)
		return;

	const bool lycIrqPeriod = isLycIrqPeriod(lycIrq.lycReg(), lycIrq.lycReg() == 153 ? lyCounter.lineTime() - (4 << (doubleSpeed*2)) : 4 - doubleSpeed * 4U, cycleCounter);

	if (lycIrq.lycReg() < 154 && ((data ^ old) & 0x40)) {
		if (data & 0x40) {
			if (lycIrqPeriod)
				ifReg |= 2;
		} else {
			if (!doubleSpeed && lycIrq.time() - cycleCounter < 5 && (!(old & 0x20) || lycIrq.lycReg() > 143 || lycIrq.lycReg() == 0))
				ifReg |= 2;
		}

		addFixedtimeEvent(irqEventQueue, &lycIrq, LycIrq::schedule(data, lycIrq.lycReg(), lyCounter, cycleCounter));
	}

	if ((((data & 0x10) && !(old & 0x10)) || !cgb) && !((old & 0x40) && lycIrqPeriod) && isMode1IrqPeriod(cycleCounter))
		ifReg |= 2;

	if ((data ^ old) & 0x08) {
		if (data & 0x08) {
			if (!((old & 0x40) && lycIrqPeriod) && isMode0IrqPeriod(cycleCounter))
				ifReg |= 2;
		} else {
			if (mode0Irq.time() - cycleCounter < 3 && (lycIrq.time() == VideoEvent::DISABLED_TIME || lyCounter.ly() != lycIrq.lycReg()))
				ifReg |= 2;
		}

		addFixedtimeEvent(irqEventQueue, &mode0Irq, Mode0Irq::schedule(data, m3ExtraCycles, lyCounter, cycleCounter));
	}

	if ((data & 0x28) == 0x20 && (old & 0x28) != 0x20 && isMode2IrqPeriod(cycleCounter)) {
		ifReg |= 2;
	}

	addFixedtimeEvent(irqEventQueue, &mode2Irq, Mode2Irq::schedule(data, lyCounter, cycleCounter));

	addEvent(vEventQueue, &irqEvent, IrqEvent::schedule(irqEventQueue));
}

void LCD::lycRegChange(const unsigned data, const unsigned long cycleCounter) {
	if (data == lycIrq.lycReg())
		return;

	if (cycleCounter >= vEventQueue.top()->time())
		update(cycleCounter);

	const unsigned old = lycIrq.lycReg();
	lycIrq.setLycReg(data);

	if (!(enabled && (statReg & 0x40)))
		return;

	if (!doubleSpeed && lycIrq.time() - cycleCounter < 5 && (!(statReg & 0x20) || old > 143 || old == 0))
		ifReg |= 2;

	addEvent(irqEventQueue, &lycIrq, LycIrq::schedule(statReg, lycIrq.lycReg(), lyCounter, cycleCounter));

	if (data < 154) {
		if (isLycIrqPeriod(data, data == 153 ? lyCounter.lineTime() - doubleSpeed * 8U : 8, cycleCounter))
			ifReg |= 2;

		if (lycIrq.isSkipPeriod(cycleCounter, doubleSpeed))
			lycIrq.setSkip(true);
	}

	addEvent(vEventQueue, &irqEvent, IrqEvent::schedule(irqEventQueue));
}

unsigned long LCD::nextIrqEvent() const {
	if (!enabled)
		return VideoEvent::DISABLED_TIME;

	if (mode0Irq.time() != VideoEvent::DISABLED_TIME && mode3Event.time() < irqEvent.time())
		return mode3Event.time();

	return irqEvent.time();
}

unsigned LCD::getIfReg(const unsigned long cycleCounter) {
	if (cycleCounter >= vEventQueue.top()->time())
		update(cycleCounter);

	return ifReg;
}

void LCD::setIfReg(const unsigned ifReg_in, const unsigned long cycleCounter) {
	if (cycleCounter >= vEventQueue.top()->time())
		update(cycleCounter);

	ifReg = ifReg_in;
}

unsigned LCD::get_stat(const unsigned lycReg, const unsigned long cycleCounter) {
	unsigned stat = 0;

	if (enabled) {
		if (cycleCounter >= vEventQueue.top()->time())
			update(cycleCounter);

		const unsigned timeToNextLy = lyCounter.time() - cycleCounter;

		if (lyCounter.ly() > 143) {
			if (lyCounter.ly() < 153 || timeToNextLy > 4 - doubleSpeed * 4U)
				stat = 1;
		} else {
			const unsigned lineCycles = 456 - (timeToNextLy >> doubleSpeed);

			if (lineCycles < 80) {
				if (!spriteMapper.inactivePeriodAfterDisplayEnable(cycleCounter))
					stat = 2;
			} else if (lineCycles < 80 + 169 + doubleSpeed * 3U + m3ExtraCycles(lyCounter.ly()))
				stat = 3;
		}

		if ((lyCounter.ly() == lycReg && timeToNextLy > 4 - doubleSpeed * 4U) ||
				(lycReg == 0 && lyCounter.ly() == 153 && timeToNextLy >> doubleSpeed <= 456 - 8)) {
			stat |= 4;
		}
	}

	return stat;
}

void LCD::do_update(unsigned cycles) {
	if (lyCounter.ly() < 144) {
		const unsigned lineCycles = lyCounter.lineCycles(lastUpdate);
		const unsigned xpos = lineCycles < drawStartCycle ? 0 : lineCycles - drawStartCycle;

		const unsigned endLineCycles = lineCycles + cycles;
		unsigned endX = endLineCycles < drawStartCycle ? 0 : endLineCycles - drawStartCycle;

		if (endX > 160)
			endX = 160;

		if (xpos < endX)
			(this->*draw)(xpos, lyCounter.ly(), endX);
	} else if (lyCounter.ly() == 144) {
		winYPos = 0xFF;
		//scy[0] = scy[1] = memory.fastread(0xFF42);
		//scx[0] = scx[1] = memory.fastread(0xFF43);
		win.weMasterChecker.unset();
	}

	videoCycles += cycles;

	if (videoCycles >= 70224U)
		videoCycles -= 70224U;
}

inline void LCD::event() {
	vEventQueue.top()->doEvent();

	if (vEventQueue.top()->time() == VideoEvent::DISABLED_TIME)
		vEventQueue.pop();
	else
		vEventQueue.modify_root(vEventQueue.top());
}

void LCD::update(const unsigned long cycleCounter) {
	if (!enabled)
		return;

	for (;;) {
		const unsigned cycles = (std::max(std::min(cycleCounter, static_cast<unsigned long>(vEventQueue.top()->time())), lastUpdate) - lastUpdate) >> doubleSpeed;
		do_update(cycles);
		lastUpdate += cycles << doubleSpeed;

		if (cycleCounter >= vEventQueue.top()->time())
			event();
		else
			break;
	}
}

void LCD::setDBuffer() {
	tmpbuf.reset(pb.format == Gambatte::PixelBuffer::RGB32 ? 0 : videoWidth() * videoHeight());

	if (cgb)
		draw = &LCD::cgb_draw<Gambatte::uint_least32_t>;
	else
		draw = &LCD::dmg_draw<Gambatte::uint_least32_t>;

	gbcToFormat = &gbcToRgb32;
	dmgColors = dmgColorsRgb32;

	if (filter) {
		dbuffer = filter->inBuffer();
		dpitch = filter->inPitch();
	} else if (pb.format == Gambatte::PixelBuffer::RGB32) {
		dbuffer = pb.pixels;
		dpitch = pb.pitch;
	} else {
		dbuffer = tmpbuf;
		dpitch = 160;
	}

	if (dbuffer == NULL)
		draw = &LCD::null_draw;

	refreshPalettes();
}

void LCD::setDmgPaletteColor(const unsigned index, const unsigned long rgb32) {
	dmgColorsRgb32[index] = rgb32;
	dmgColorsRgb16[index] = rgb32ToRgb16(rgb32);
	dmgColorsUyvy[index] = ::rgb32ToUyvy(rgb32);
}

void LCD::setDmgPaletteColor(const unsigned palNum, const unsigned colorNum, const unsigned long rgb32) {
	if (palNum > 2 || colorNum > 3)
		return;

	setDmgPaletteColor((palNum * 4) | colorNum, rgb32);
	refreshPalettes();
}

void LCD::null_draw(unsigned /*xpos*/, const unsigned ypos, const unsigned endX) {
	const bool enableWindow = win.enabled(ypos);

	if (enableWindow && winYPos == 0xFF)
		winYPos = /*ypos - wyReg.value()*/ 0;

	if (endX == 160) {
		if (enableWindow)
			++winYPos;
	}
}

template<typename T>
void LCD::cgb_draw(unsigned xpos, const unsigned ypos, const unsigned endX) {
	const unsigned effectiveScx = scReader.scx();

	const bool enableWindow = win.enabled(ypos);

	if (enableWindow && winYPos == 0xFF)
		winYPos = /*ypos - wyReg.value()*/ 0;

	T *const bufLine = static_cast<T*>(dbuffer) + ypos * static_cast<unsigned long>(dpitch);

	if (!(enableWindow && win.wxReader.wx() <= xpos + 7)) {
		const unsigned fby = scReader.scy() + ypos /*& 0xFF*/;
		const unsigned end = std::min(enableWindow ? win.wxReader.wx() - 7 : 160U, endX);

		cgb_bg_drawPixels(bufLine, xpos, end, scxReader.scxAnd7(), ((xpos + effectiveScx) & ~7) + ((xpos + drawStartCycle - scReadOffset) & 7),
				bgTileMap + (fby & 0xF8) * 4, bgTileData, fby & 7);
	}

	if (enableWindow && endX + 7 > win.wxReader.wx()) {
		const unsigned start = std::max(win.wxReader.wx() < 7 ? 0U : (win.wxReader.wx() - 7), xpos);

		cgb_bg_drawPixels(bufLine, start, endX, 7u - win.wxReader.wx(), start + (7u - win.wxReader.wx()),
				wdTileMap + (winYPos & 0xF8) * 4, bgTileData, winYPos & 7);
	}

	if (endX == 160) {
		if (spriteEnable)
			cgb_drawSprites(bufLine, ypos);

		if (enableWindow)
			++winYPos;
	}
}

template<typename T>
void LCD::dmg_draw(unsigned xpos, const unsigned ypos, const unsigned endX) {
	const unsigned effectiveScx = scReader.scx();

	const bool enableWindow = win.enabled(ypos);

	if (enableWindow && winYPos == 0xFF)
		winYPos = /*ypos - wyReg.value()*/ 0;

	T *const bufLine = static_cast<T*>(dbuffer) + ypos * static_cast<unsigned long>(dpitch);

	if (bgEnable) {
		if (!(enableWindow && win.wxReader.wx() <= xpos + 7)) {
			const unsigned fby = scReader.scy() + ypos /*& 0xFF*/;
			const unsigned end = std::min(enableWindow ? win.wxReader.wx() - 7 : 160U, endX);

			bg_drawPixels(bufLine, xpos, end, scxReader.scxAnd7(), ((xpos + effectiveScx) & ~7) + ((xpos + drawStartCycle - scReadOffset) & 7),
					bgTileMap + (fby & 0xF8) * 4, bgTileData + (fby & 7) * 2);
		}

		if (enableWindow && endX + 7 > win.wxReader.wx()) {
			const unsigned start = std::max(win.wxReader.wx() < 7 ? 0U : (win.wxReader.wx() - 7), xpos);

			bg_drawPixels(bufLine, start, endX, 7u - win.wxReader.wx(), start + (7u - win.wxReader.wx()),
					wdTileMap + (winYPos & 0xF8) * 4, bgTileData + (winYPos & 7) * 2);
		}
	} else
		std::fill_n(bufLine + xpos, endX - xpos, bgPalette[0]);

	if (endX == 160) {
		if (spriteEnable)
			drawSprites(bufLine, ypos);

		if (enableWindow)
			++winYPos;
	}
}

#define FLIP(u8) ( (((u8) & 0x01) << 7) | (((u8) & 0x02) << 5) | (((u8) & 0x04) << 3) | (((u8) & 0x08) << 1) | \
(((u8) & 0x10) >> 1) | (((u8) & 0x20) >> 3) | (((u8) & 0x40) >> 5) | (((u8) & 0x80) >> 7) )

#define FLIP_ROW(n) FLIP((n)|0x0), FLIP((n)|0x1), FLIP((n)|0x2), FLIP((n)|0x3), FLIP((n)|0x4), FLIP((n)|0x5), FLIP((n)|0x6), FLIP((n)|0x7), \
FLIP((n)|0x8), FLIP((n)|0x9), FLIP((n)|0xA), FLIP((n)|0xB), FLIP((n)|0xC), FLIP((n)|0xD), FLIP((n)|0xE), FLIP((n)|0xF)

static const unsigned char xflipt[0x100] = {
	FLIP_ROW(0x00), FLIP_ROW(0x10), FLIP_ROW(0x20), FLIP_ROW(0x30),
	FLIP_ROW(0x40), FLIP_ROW(0x50), FLIP_ROW(0x60), FLIP_ROW(0x70),
	FLIP_ROW(0x80), FLIP_ROW(0x90), FLIP_ROW(0xA0), FLIP_ROW(0xB0),
	FLIP_ROW(0xC0), FLIP_ROW(0xD0), FLIP_ROW(0xE0), FLIP_ROW(0xF0)
};

#undef FLIP_ROW
#undef FLIP

#define PREP(u8) (u8)

#define EXPAND(u8) ((PREP(u8) << 7 & 0x4000) | (PREP(u8) << 6 & 0x1000) | (PREP(u8) << 5 & 0x0400) | (PREP(u8) << 4 & 0x0100) | \
                    (PREP(u8) << 3 & 0x0040) | (PREP(u8) << 2 & 0x0010) | (PREP(u8) << 1 & 0x0004) | (PREP(u8)      & 0x0001))

#define EXPAND_ROW(n) EXPAND((n)|0x0), EXPAND((n)|0x1), EXPAND((n)|0x2), EXPAND((n)|0x3), \
                      EXPAND((n)|0x4), EXPAND((n)|0x5), EXPAND((n)|0x6), EXPAND((n)|0x7), \
                      EXPAND((n)|0x8), EXPAND((n)|0x9), EXPAND((n)|0xA), EXPAND((n)|0xB), \
                      EXPAND((n)|0xC), EXPAND((n)|0xD), EXPAND((n)|0xE), EXPAND((n)|0xF)

#define EXPAND_TABLE EXPAND_ROW(0x00), EXPAND_ROW(0x10), EXPAND_ROW(0x20), EXPAND_ROW(0x30), \
                     EXPAND_ROW(0x40), EXPAND_ROW(0x50), EXPAND_ROW(0x60), EXPAND_ROW(0x70), \
                     EXPAND_ROW(0x80), EXPAND_ROW(0x90), EXPAND_ROW(0xA0), EXPAND_ROW(0xB0), \
                     EXPAND_ROW(0xC0), EXPAND_ROW(0xD0), EXPAND_ROW(0xE0), EXPAND_ROW(0xF0)

static const unsigned short expand_lut[0x200] = {
	EXPAND_TABLE,

#undef PREP
#define PREP(u8) (((u8) << 7 & 0x80) | ((u8) << 5 & 0x40) | ((u8) << 3 & 0x20) | ((u8) << 1 & 0x10) | \
                  ((u8) >> 1 & 0x08) | ((u8) >> 3 & 0x04) | ((u8) >> 5 & 0x02) | ((u8) >> 7 & 0x01))

	EXPAND_TABLE
};

#undef EXPAND_TABLE
#undef EXPAND_ROW
#undef EXPAND
#undef PREP

//shoud work for the window too, if -wx is passed as scx.
//tilemap and tiledata must point to the areas in the first vram bank
//the second vram bank has to be placed immediately after the first one in memory (0x4000 continous bytes that cover both).
//tilemap needs to be offset to the right line
template<typename T>
void LCD::cgb_bg_drawPixels(T * const buffer_line, unsigned xpos, const unsigned end, const unsigned scx, unsigned tilemappos,
			const unsigned char *const tilemap, const unsigned char *const tiledata, const unsigned tileline)
{
	const unsigned sign = tileIndexSign;
	unsigned shift = (7 - ((scx + xpos) & 7)) * 2;
	T *buf = buffer_line + xpos;
	T *const bufend = buffer_line + end;

	while (buf < bufend) {
		if ((tilemappos & 7) || bufend - buf < 8) {
			const unsigned char *const maptmp = tilemap + (tilemappos >> 3 & 0x1F);
			const unsigned attributes = maptmp[0x2000];
			const unsigned char *const dataptr = tiledata + (attributes << 10 & 0x2000) +
					maptmp[0] * 16 - (maptmp[0] & sign) * 32 + ((attributes & 0x40) ? 7 - tileline : tileline) * 2;
			const unsigned short *const exp_lut = expand_lut + (attributes << 3 & 0x100);

			const unsigned data = exp_lut[dataptr[0]] + exp_lut[dataptr[1]] * 2;
			const unsigned long *const palette = bgPalette + (attributes & 7) * 4;

			do {
				*buf++ = palette[data >> shift & 3];
				shift = (shift - 2) & 15;
			} while ((++tilemappos & 7) && buf < bufend);
		}

		while (bufend - buf > 7) {
			const unsigned char *const maptmp = tilemap + (tilemappos >> 3 & 0x1F);
			const unsigned attributes = maptmp[0x2000];
			const unsigned char *const dataptr = tiledata + (attributes << 10 & 0x2000) +
					maptmp[0] * 16 - (maptmp[0] & sign) * 32 + ((attributes & 0x40) ? 7 - tileline : tileline) * 2;
			const unsigned short *const exp_lut = expand_lut + (attributes << 3 & 0x100);

			const unsigned data = exp_lut[dataptr[0]] + exp_lut[dataptr[1]] * 2;
			const unsigned long *const palette = bgPalette + (attributes & 7) * 4;

			buf[0] = palette[data >>   shift             & 3];
			buf[1] = palette[data >> ((shift -  2) & 15) & 3];
			buf[2] = palette[data >> ((shift -  4) & 15) & 3];
			buf[3] = palette[data >> ((shift -  6) & 15) & 3];
			buf[4] = palette[data >> ((shift -  8) & 15) & 3];
			buf[5] = palette[data >> ((shift - 10) & 15) & 3];
			buf[6] = palette[data >> ((shift - 12) & 15) & 3];
			buf[7] = palette[data >> ((shift - 14) & 15) & 3];

			buf += 8;
			tilemappos += 8;
		}
	}
}

static unsigned cgb_prioritizedBG_mask(const unsigned spx, const unsigned bgStart, const unsigned bgEnd, const unsigned scx,
				       const unsigned char *const tilemap, const unsigned char *const tiledata, const unsigned tileline, const unsigned sign) {
	const unsigned spStart = spx < bgStart + 8 ? bgStart + 8 - spx : 0;

	unsigned bgbyte;

	{
		const unsigned pos = scx + spx - 8 + spStart;
		const unsigned char *maptmp = tilemap + (pos >> 3 & 0x1F);
		unsigned tile = maptmp[0];
		unsigned attributes = maptmp[0x2000];

		const unsigned char *const data = tiledata + (attributes << 10 & 0x2000) +
			tile * 16 - (tile & sign) * 32 + ((attributes & 0x40) ? 7 - tileline : tileline) * 2;

		bgbyte = (attributes & 0x20) ? xflipt[data[0] | data[1]] : (data[0] | data[1]);

		const unsigned offset = pos & 7;

		if (offset) {
			bgbyte <<= offset;
			maptmp = tilemap + (((pos >> 3) + 1) & 0x1F);
			tile = maptmp[0];
			attributes = maptmp[0x2000];

			const unsigned char *const data = tiledata + (attributes << 10 & 0x2000) +
				tile * 16 - (tile & sign) * 32 + ((attributes & 0x40) ? 7 - tileline : tileline) * 2;

			bgbyte |= ((attributes & 0x20) ? xflipt[data[0] | data[1]] : (data[0] | data[1])) >> (8 - offset);
		}
	}

	bgbyte >>= spStart;
	const unsigned spEnd = spx > bgEnd ? bgEnd + 8 - spx : 8;
	const unsigned mask = ~bgbyte | 0xFF >> spEnd;

	return mask;
}

static unsigned cgb_toplayerBG_mask(const unsigned spx, const unsigned bgStart, const unsigned bgEnd, const unsigned scx,
				    const unsigned char *const tilemap, const unsigned char *const tiledata, const unsigned tileline, const unsigned sign) {
	const unsigned spStart = spx < bgStart + 8 ? bgStart + 8 - spx : 0;

	unsigned bgbyte = 0;

	{
		const unsigned pos = scx + spx - 8 + spStart;
		const unsigned char *maptmp = tilemap + (pos >> 3 & 0x1F);
		unsigned attributes = maptmp[0x2000];

		if (attributes & 0x80) {
			const unsigned tile = maptmp[0];

			const unsigned char *const data = tiledata + (attributes << 10 & 0x2000) +
				tile * 16 - (tile & sign) * 32 + ((attributes & 0x40) ? 7 - tileline : tileline) * 2;

			bgbyte = (attributes & 0x20) ? xflipt[data[0] | data[1]] : (data[0] | data[1]);
		}

		const unsigned offset = pos & 7;

		if (offset) {
			bgbyte <<= offset;
			maptmp = tilemap + (((pos >> 3) + 1) & 0x1F);
			attributes = maptmp[0x2000];

			if (attributes & 0x80) {
				const unsigned tile = maptmp[0];

				const unsigned char *const data = tiledata + (attributes << 10 & 0x2000) +
					tile * 16 - (tile & sign) * 32 + ((attributes & 0x40) ? 7 - tileline : tileline) * 2;

				bgbyte |= ((attributes & 0x20) ? xflipt[data[0] | data[1]] : (data[0] | data[1])) >> (8 - offset);
			}
		}
	}

	bgbyte >>= spStart;
	const unsigned spEnd = spx > bgEnd ? bgEnd + 8 - spx : 8;
	const unsigned mask = ~bgbyte | 0xFF >> spEnd;

	return mask;
}

template<typename T>
void LCD::cgb_drawSprites(T * const buffer_line, const unsigned ypos) {
	const unsigned scy = scReader.scy() + ypos /*& 0xFF*/;
	const unsigned wx = win.wxReader.wx() < 7 ? 0 : win.wxReader.wx() - 7;
	const bool enableWindow = win.enabled(ypos);
	const unsigned char *const spriteMapLine = spriteMapper.sprites(ypos);

	for (int i = spriteMapper.numSprites(ypos) - 1; i >= 0; --i) {
		const unsigned spNrX2 = spriteMapLine[i];
		const unsigned spx = spriteMapper.posbuf()[spNrX2 + 1];

		if (spx < 168 && spx) {
			unsigned spLine = ypos + 16 - spriteMapper.posbuf()[spNrX2];
			unsigned spTile = spriteMapper.oamram()[spNrX2 * 2 + 2];
			const unsigned attributes = spriteMapper.oamram()[spNrX2 * 2 + 3];

			if (spriteMapper.largeSprites(spNrX2 >> 1)) {
				if (attributes & 0x40) //yflip
					spLine = 15 - spLine;

				if (spLine < 8)
					spTile &= 0xFE;
				else {
					spLine -= 8;
					spTile |= 0x01;
				}
			} else {
				if (attributes & 0x40) //yflip
					spLine = 7 - spLine;
			}

			const unsigned char *const data = vram + ((attributes * 0x400) & 0x2000) + spTile * 16 + spLine * 2;

			unsigned byte1 = data[0];
			unsigned byte2 = data[1];

			if (attributes & 0x20) {
				byte1 = xflipt[byte1];
				byte2 = xflipt[byte2];
			}

			//(Sprites with priority-bit are still allowed to cover other sprites according to GBdev-faq.)
			if (bgEnable) {
				unsigned mask = 0xFF;

				if (attributes & 0x80) {
					if (!(enableWindow && (wx == 0 || spx >= wx + 8u)))
						mask = cgb_prioritizedBG_mask(spx, 0, enableWindow ? wx : 160, scReader.scx(),
						                              bgTileMap + ((scy & 0xF8) << 2), bgTileData, scy & 7, tileIndexSign);
					if (enableWindow && spx > wx)
						mask &= cgb_prioritizedBG_mask(spx, wx, 160, 0u - wx, wdTileMap + ((winYPos & 0xF8) << 2), bgTileData, winYPos & 7, tileIndexSign);
				} else {
					if (!(enableWindow && (wx == 0 || spx >= wx + 8u)))
						mask = cgb_toplayerBG_mask(spx, 0, enableWindow ? wx : 160, scReader.scx(),
						                           bgTileMap + ((scy & 0xF8) << 2), bgTileData, scy & 7, tileIndexSign);
					if (enableWindow && spx > wx)
						mask &= cgb_toplayerBG_mask(spx, wx, 160, 0u - wx, wdTileMap + ((winYPos & 0xF8) << 2), bgTileData, winYPos & 7, tileIndexSign);
				}

				byte1 &= mask;
				byte2 &= mask;
			}

			const unsigned bytes = expand_lut[byte1] + expand_lut[byte2] * 2;
			const unsigned long *const palette = spPalette + (attributes & 7) * 4;

			if (spx > 7 && spx < 161) {
				T * const buf = buffer_line + spx - 8;
				unsigned color;

				if ((color = bytes >> 14    ))
					buf[0] = palette[color];
				if ((color = bytes >> 12 & 3))
					buf[1] = palette[color];
				if ((color = bytes >> 10 & 3))
					buf[2] = palette[color];
				if ((color = bytes >>  8 & 3))
					buf[3] = palette[color];
				if ((color = bytes >>  6 & 3))
					buf[4] = palette[color];
				if ((color = bytes >>  4 & 3))
					buf[5] = palette[color];
				if ((color = bytes >>  2 & 3))
					buf[6] = palette[color];
				if ((color = bytes       & 3))
					buf[7] = palette[color];
			} else {
				const unsigned end = spx >= 160 ? 160 : spx;
				unsigned xpos = spx <= 8 ? 0 : (spx - 8);
				unsigned shift = (7 - (xpos + 8 - spx)) * 2;

				while (xpos < end) {
					if (const unsigned color = bytes >> shift & 3)
						buffer_line[xpos] = palette[color];

					shift -= 2;
					++xpos;
				}
			}
		}
	}
}


//shoud work for the window too, if -wx is passed as scx.
//tilemap and tiledata need to be offset to the right line
template<typename T>
void LCD::bg_drawPixels(T * const buffer_line, unsigned xpos, const unsigned end, const unsigned scx, unsigned tilemappos,
			const unsigned char *const tilemap, const unsigned char *const tiledata)
{
	const unsigned sign = tileIndexSign;
	unsigned shift = (7 - ((scx + xpos) & 7)) * 2;
	T *buf = buffer_line + xpos;
	T *const bufend = buffer_line + end;

	while (buf < bufend) {
		if ((tilemappos & 7) || bufend - buf < 8) {
			const unsigned tile = tilemap[tilemappos >> 3 & 0x1F];
			const unsigned char *const dataptr = tiledata + tile * 16 - (tile & sign) * 32;
			const unsigned data = expand_lut[dataptr[0]] + expand_lut[dataptr[1]] * 2;

			do {
				*buf++ = bgPalette[data >> shift & 3];
				shift = (shift - 2) & 15;
			} while ((++tilemappos & 7) && buf < bufend);
		}

		while (bufend - buf > 7) {
			const unsigned tile = tilemap[tilemappos >> 3 & 0x1F];
			const unsigned char *const dataptr = tiledata + tile * 16 - (tile & sign) * 32;
			const unsigned data = expand_lut[dataptr[0]] + expand_lut[dataptr[1]] * 2;
			buf[0] = bgPalette[data >>   shift             & 3];
			buf[1] = bgPalette[data >> ((shift -  2) & 15) & 3];
			buf[2] = bgPalette[data >> ((shift -  4) & 15) & 3];
			buf[3] = bgPalette[data >> ((shift -  6) & 15) & 3];
			buf[4] = bgPalette[data >> ((shift -  8) & 15) & 3];
			buf[5] = bgPalette[data >> ((shift - 10) & 15) & 3];
			buf[6] = bgPalette[data >> ((shift - 12) & 15) & 3];
			buf[7] = bgPalette[data >> ((shift - 14) & 15) & 3];
			buf += 8;
			tilemappos += 8;
		}
	}
}

static unsigned prioritizedBG_mask(const unsigned spx, const unsigned bgStart, const unsigned bgEnd, const unsigned scx,
				   const unsigned char *const tilemap, const unsigned char *const tiledata, const unsigned sign) {
	const unsigned spStart = spx < bgStart + 8 ? bgStart + 8 - spx : 0;

	unsigned bgbyte;

	{
		const unsigned pos = scx + spx - 8 + spStart;
		unsigned tile = tilemap[pos >> 3 & 0x1F];
		const unsigned char *data = tiledata + tile * 16 - (tile & sign) * 32;
		bgbyte = data[0] | data[1];
		const unsigned offset = pos & 7;

		if (offset) {
			bgbyte <<= offset;
			tile = tilemap[((pos >> 3) + 1) & 0x1F];
			data = tiledata + tile * 16 - (tile & sign) * 32;
			bgbyte |= (data[0] | data[1]) >> (8 - offset);
		}
	}

	bgbyte >>= spStart;
	const unsigned spEnd = spx > bgEnd ? bgEnd + 8 - spx : 8;
	const unsigned mask = ~bgbyte | 0xFF >> spEnd;

	return mask;
}

template<typename T>
void LCD::drawSprites(T * const buffer_line, const unsigned ypos) {
	const unsigned scy = scReader.scy() + ypos /*& 0xFF*/;
	const unsigned wx = win.wxReader.wx() < 7 ? 0 : win.wxReader.wx() - 7;
	const bool enableWindow = win.enabled(ypos);
	const unsigned char *const spriteMapLine = spriteMapper.sprites(ypos);

	for (int i = spriteMapper.numSprites(ypos) - 1; i >= 0; --i) {
		const unsigned spNrX2 = spriteMapLine[i];
		const unsigned spx = spriteMapper.posbuf()[spNrX2 + 1];

		if (spx < 168 && spx) {
			unsigned spLine = ypos + 16 - spriteMapper.posbuf()[spNrX2];
			unsigned spTile = spriteMapper.oamram()[spNrX2 * 2 + 2];
			const unsigned attributes = spriteMapper.oamram()[spNrX2 * 2 + 3];

			if (spriteMapper.largeSprites(spNrX2 >> 1)) {
				if (attributes & 0x40) //yflip
					spLine = 15 - spLine;

				if (spLine < 8)
					spTile &= 0xFE;
				else {
					spLine -= 8;
					spTile |= 0x01;
				}
			} else {
				if (attributes & 0x40) //yflip
					spLine = 7 - spLine;
			}

			const unsigned char *const data = vram + spTile * 16 + spLine * 2;

			unsigned byte1 = data[0];
			unsigned byte2 = data[1];

			if (attributes & 0x20) {
				byte1 = xflipt[byte1];
				byte2 = xflipt[byte2];
			}

			//(Sprites with priority-bit are still allowed to cover other sprites according to GBdev-faq.)
			if (attributes & 0x80) {
				unsigned mask = 0xFF;

				if (bgEnable && !(enableWindow && (wx == 0 || spx >= wx + 8u)))
					mask = prioritizedBG_mask(spx, 0, enableWindow ? wx : 160, scReader.scx(),
					                          bgTileMap + ((scy & 0xF8) << 2), bgTileData + ((scy & 7) << 1), tileIndexSign);
				if (enableWindow && spx > wx)
					mask &= prioritizedBG_mask(spx, wx, 160, 0u - wx, wdTileMap + ((winYPos & 0xF8) << 2), bgTileData + ((winYPos & 7) << 1), tileIndexSign);

				byte1 &= mask;
				byte2 &= mask;
			}

			const unsigned bytes = expand_lut[byte1] + expand_lut[byte2] * 2;
			const unsigned long *const palette = spPalette + ((attributes >> 2) & 4);

			if (spx > 7 && spx < 161) {
				T * const buf = buffer_line + spx - 8;
				unsigned color;

				if ((color = bytes >> 14    ))
					buf[0] = palette[color];
				if ((color = bytes >> 12 & 3))
					buf[1] = palette[color];
				if ((color = bytes >> 10 & 3))
					buf[2] = palette[color];
				if ((color = bytes >>  8 & 3))
					buf[3] = palette[color];
				if ((color = bytes >>  6 & 3))
					buf[4] = palette[color];
				if ((color = bytes >>  4 & 3))
					buf[5] = palette[color];
				if ((color = bytes >>  2 & 3))
					buf[6] = palette[color];
				if ((color = bytes       & 3))
					buf[7] = palette[color];
			} else {
				const unsigned end = spx >= 160 ? 160 : spx;
				unsigned xpos = spx <= 8 ? 0 : (spx - 8);
				unsigned shift = (7 - (xpos + 8 - spx)) * 2;

				while (xpos < end) {
					if (const unsigned color = bytes >> shift & 3)
						buffer_line[xpos] = palette[color];

					shift -= 2;
					++xpos;
				}
			}
		}
	}
}
