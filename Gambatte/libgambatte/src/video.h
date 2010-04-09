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
#ifndef VIDEO_H
#define VIDEO_H

namespace Gambatte {
class VideoBlitter;
struct FilterInfo;
}

class Filter;
class SaveState;

#include <vector>
#include <memory>
#include "event_queue.h"
#include "videoblitter.h"
#include "array.h"
#include "int.h"
#include "colorconversion.h"
#include "osd_element.h"

#include "video/video_event_comparer.h"
#include "video/ly_counter.h"
#include "video/window.h"
#include "video/scx_reader.h"
#include "video/sprite_mapper.h"
#include "video/sc_reader.h"
#include "video/break_event.h"
#include "video/mode3_event.h"

#include "video/lyc_irq.h"
#include "video/mode0_irq.h"
#include "video/mode1_irq.h"
#include "video/mode2_irq.h"
#include "video/irq_event.h"
#include "video/m3_extra_cycles.h"

class LCD {
	//static const uint8_t xflipt[0x100];
	unsigned long dmgColorsRgb32[3 * 4];
	unsigned long dmgColorsRgb16[3 * 4];
	unsigned long dmgColorsUyvy[3 * 4];

	unsigned long bgPalette[8 * 4];
	unsigned long spPalette[8 * 4];

	unsigned char bgpData[8 * 8];
	unsigned char objpData[8 * 8];

	const unsigned char *const vram;
	const unsigned char *bgTileData;
	const unsigned char *bgTileMap;
	const unsigned char *wdTileMap;

	Gambatte::VideoBlitter *vBlitter;
	Filter *filter;

	void *dbuffer;
	void (LCD::*draw)(unsigned xpos, unsigned ypos, unsigned endX);
	unsigned long (*gbcToFormat)(unsigned bgr15);
	const unsigned long *dmgColors;

	unsigned long lastUpdate;
	unsigned long videoCycles;

	unsigned dpitch;
	unsigned winYPos;

	event_queue<VideoEvent*,VideoEventComparer> m3EventQueue;
	event_queue<VideoEvent*,VideoEventComparer> irqEventQueue;
	event_queue<VideoEvent*,VideoEventComparer> vEventQueue;

	LyCounter lyCounter;
	Window win;
	ScxReader scxReader;
	SpriteMapper spriteMapper;
	M3ExtraCycles m3ExtraCycles;
	ScReader scReader;
	BreakEvent breakEvent;
	Mode3Event mode3Event;

	LycIrq lycIrq;
	Mode0Irq mode0Irq;
	Mode1Irq mode1Irq;
	Mode2Irq mode2Irq;
	IrqEvent irqEvent;

	Gambatte::PixelBuffer pb;
	Array<Gambatte::uint_least32_t> tmpbuf;
	Rgb32ToUyvy rgb32ToUyvy;
	std::auto_ptr<OsdElement> osdElement;

	std::vector<Filter*> filters;

	unsigned char drawStartCycle;
	unsigned char scReadOffset;
	unsigned char ifReg;
	unsigned char tileIndexSign;
	unsigned char statReg;

	bool doubleSpeed;
	bool enabled;
	bool cgb;
	bool bgEnable;
	bool spriteEnable;

	static void setDmgPalette(unsigned long *palette, const unsigned long *dmgColors, unsigned data);
	void setDmgPaletteColor(unsigned index, unsigned long rgb32);
	static unsigned long gbcToRgb32(unsigned bgr15);
	static unsigned long gbcToRgb16(unsigned bgr15);
	static unsigned long gbcToUyvy(unsigned bgr15);

	void refreshPalettes();
	void setDBuffer();
	void resetVideoState(unsigned long cycleCounter);

	void setDoubleSpeed(bool enabled);

	void event();

	bool cgbpAccessible(unsigned long cycleCounter);
	bool isMode0IrqPeriod(unsigned long cycleCounter);
	bool isMode2IrqPeriod(unsigned long cycleCounter);
	bool isLycIrqPeriod(unsigned lycReg, unsigned endCycles, unsigned long cycleCounter);
	bool isMode1IrqPeriod(unsigned long cycleCounter);

	template<typename T> void bg_drawPixels(T *buffer_line, unsigned xpos, unsigned end, unsigned scx, unsigned tilemappos,
			const unsigned char *tilemap, const unsigned char *tiledata);
	template<typename T> void drawSprites(T *buffer_line, unsigned ypos);

	template<typename T> void cgb_bg_drawPixels(T *buffer_line, unsigned xpos, unsigned end, unsigned scx, unsigned tilemappos,
			const unsigned char *tilemap, const unsigned char *tiledata, unsigned tileline);
	template<typename T> void cgb_drawSprites(T *buffer_line, unsigned ypos);

	void null_draw(unsigned xpos, unsigned ypos, unsigned endX);
	template<typename T> void dmg_draw(unsigned xpos, unsigned ypos, unsigned endX);
	template<typename T> void cgb_draw(unsigned xpos, unsigned ypos, unsigned endX);

	void do_update(unsigned cycles);
	void update(unsigned long cycleCounter);

public:
	LCD(const unsigned char *oamram, const unsigned char *vram_in);
	~LCD();
	void reset(const unsigned char *oamram, bool cgb);
	void setStatePtrs(SaveState &state);
	void saveState(SaveState &state) const;
	void loadState(const SaveState &state, const unsigned char *oamram);
	void setVideoBlitter(Gambatte::VideoBlitter *vb);
	void videoBufferChange();
	void setVideoFilter(unsigned n);
	std::vector<const Gambatte::FilterInfo*> filterInfo() const;
	unsigned videoWidth() const;
	unsigned videoHeight() const;
	void setDmgPaletteColor(unsigned palNum, unsigned colorNum, unsigned long rgb32);

	void setOsdElement(std::auto_ptr<OsdElement> osdElement) {
		this->osdElement = osdElement;
	}

	void wdTileMapSelectChange(bool newValue, unsigned long cycleCounter);
	void bgTileMapSelectChange(bool newValue, unsigned long cycleCounter);
	void bgTileDataSelectChange(bool newValue, unsigned long cycleCounter);
	void bgEnableChange(bool newValue, unsigned long cycleCounter);
	void spriteEnableChange(bool newValue, unsigned long cycleCounter);

	void dmgBgPaletteChange(const unsigned data, const unsigned long cycleCounter) {
		update(cycleCounter);
		bgpData[0] = data;
		setDmgPalette(bgPalette, dmgColors, data);
	}

	void dmgSpPalette1Change(const unsigned data, const unsigned long cycleCounter) {
		update(cycleCounter);
		objpData[0] = data;
		setDmgPalette(spPalette, dmgColors + 4, data);
	}

	void dmgSpPalette2Change(const unsigned data, const unsigned long cycleCounter) {
		update(cycleCounter);
		objpData[1] = data;
		setDmgPalette(spPalette + 4, dmgColors + 8, data);
	}

	void cgbBgColorChange(unsigned index, const unsigned data, const unsigned long cycleCounter) {
		if (bgpData[index] != data && cgbpAccessible(cycleCounter)) {
			update(cycleCounter);
			bgpData[index] = data;
			index >>= 1;
			bgPalette[index] = (*gbcToFormat)(bgpData[index << 1] | bgpData[(index << 1) + 1] << 8);
		}
	}

	void cgbSpColorChange(unsigned index, const unsigned data, const unsigned long cycleCounter) {
		if (objpData[index] != data && cgbpAccessible(cycleCounter)) {
			update(cycleCounter);
			objpData[index] = data;
			index >>= 1;
			spPalette[index] = (*gbcToFormat)(objpData[index << 1] | objpData[(index << 1) + 1] << 8);
		}
	}

	unsigned cgbBgColorRead(const unsigned index, const unsigned long cycleCounter) {
		return cgb & cgbpAccessible(cycleCounter) ? bgpData[index] : 0xFF;
	}

	unsigned cgbSpColorRead(const unsigned index, const unsigned long cycleCounter) {
		return cgb & cgbpAccessible(cycleCounter) ? objpData[index] : 0xFF;
	}

	void updateScreen(unsigned long cc);
	void enableChange(unsigned long cycleCounter);
	void preResetCounter(unsigned long cycleCounter);
	void postResetCounter(unsigned long oldCC, unsigned long cycleCounter);
	void preSpeedChange(unsigned long cycleCounter);
	void postSpeedChange(unsigned long cycleCounter);
// 	unsigned get_mode(unsigned cycleCounter) /*const*/;
	bool vramAccessible(unsigned long cycleCounter);
	bool oamAccessible(unsigned long cycleCounter);
	void weChange(bool newValue, unsigned long cycleCounter);
	void wxChange(unsigned newValue, unsigned long cycleCounter);
	void wyChange(unsigned newValue, unsigned long cycleCounter);
	void oamChange(unsigned long cycleCounter);
	void oamChange(const unsigned char *oamram, unsigned long cycleCounter);
	void scxChange(unsigned newScx, unsigned long cycleCounter);
	void scyChange(unsigned newValue, unsigned long cycleCounter);
	void spriteSizeChange(bool newLarge, unsigned long cycleCounter);

	void vramChange(const unsigned long cycleCounter) {
		update(cycleCounter);
	}

	unsigned get_stat(unsigned lycReg, unsigned long cycleCounter);

	unsigned getLyReg(const unsigned long cycleCounter) {
		unsigned lyReg = 0;

		if (enabled) {
			if (cycleCounter >= lyCounter.time())
				update(cycleCounter);

			lyReg = lyCounter.ly();

			if (lyCounter.time() - cycleCounter <= 4) {
				if (lyReg == 153)
					lyReg = 0;
				else
					++lyReg;
			} else if (lyReg == 153)
				lyReg = 0;
		}

		return lyReg;
	}

	unsigned long nextMode1IrqTime() const {
		return mode1Irq.time();
	}

	void lyWrite(unsigned long cycleCounter);
	void lcdstatChange(unsigned data, unsigned long cycleCounter);
	void lycRegChange(unsigned data, unsigned long cycleCounter);
	unsigned long nextIrqEvent() const;
	unsigned getIfReg(unsigned long cycleCounter);
	void setIfReg(unsigned ifReg_in, unsigned long cycleCounter);

	unsigned long nextHdmaTime(unsigned long cycleCounter);
	bool isHdmaPeriod(unsigned long cycleCounter);

	unsigned long nextHdmaTimeInvalid() const {
		return mode3Event.time();
	}
};

#endif
