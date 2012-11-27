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

#ifndef SCI_GRAPHICS_ANIMATE_H
#define SCI_GRAPHICS_ANIMATE_H

#include "sci/graphics/helpers.h"

namespace Sci {

// Flags for the signal selector
enum ViewSignals {
	kSignalStopUpdate    = 0x0001,
	kSignalViewUpdated   = 0x0002,
	kSignalNoUpdate      = 0x0004,
	kSignalHidden        = 0x0008,
	kSignalFixedPriority = 0x0010,
	kSignalAlwaysUpdate  = 0x0020,
	kSignalForceUpdate   = 0x0040,
	kSignalRemoveView    = 0x0080,
	kSignalFrozen        = 0x0100, // I got frozen today!!
	//kSignalExtraActor	 = 0x0200, // unused by us, defines all actors that may be included into the background if speed is too slow
	kSignalHitObstacle	 = 0x0400, // used in the actor movement code by kDoBresen()
	kSignalDoesntTurn	 = 0x0800, // used by _k_dirloop() to determine if an actor can turn or not
	//kSignalNoCycler		 = 0x1000, // unused by us
	//kSignalIgnoreHorizon = 0x2000, // unused by us, defines actor that can ignore horizon
	kSignalIgnoreActor   = 0x4000,
	kSignalDisposeMe     = 0x8000
};

enum ViewScaleSignals {
	kScaleSignalDoScaling				= 0x0001, // enables scaling when drawing that cel (involves scaleX and scaleY)
	kScaleSignalGlobalScaling			= 0x0002, // means that global scaling shall get applied on that cel (sets scaleX/scaleY)
	kScaleSignalHoyle4SpecialHandling	= 0x0004  // HOYLE4-exclusive: special handling inside kAnimate, is used when giving out cards

};

struct AnimateEntry {
	int16 givenOrderNo;
	reg_t object;
	GuiResourceId viewId;
	int16 loopNo;
	int16 celNo;
	int16 paletteNo;
	int16 x, y, z;
	int16 priority;
	uint16 signal;
	uint16 scaleSignal;
	int16 scaleX;
	int16 scaleY;
	Common::Rect celRect;
	bool showBitsFlag;
	reg_t castHandle;
};
typedef Common::List<AnimateEntry> AnimateList;
typedef Common::Array<AnimateEntry> AnimateArray;

class Console;
class GfxCache;
class GfxCursor;
class GfxPorts;
class GfxPaint16;
class GfxScreen;
class GfxPalette;
class GfxTransitions;
class GfxView;
/**
 * Animate class, kAnimate and relevant functions for SCI16 (SCI0-SCI1.1) games
 */
class GfxAnimate {
public:
	GfxAnimate(EngineState *state, GfxCache *cache, GfxPorts *ports, GfxPaint16 *paint16, GfxScreen *screen, GfxPalette *palette, GfxCursor *cursor, GfxTransitions *transitions);
	virtual ~GfxAnimate();

	void disposeLastCast();
	bool invoke(List *list, int argc, reg_t *argv);
	void makeSortedList(List *list);
	void applyGlobalScaling(AnimateList::iterator entry, GfxView *view);
	void fill(byte &oldPicNotValid);
	void update();
	void drawCels();
	void updateScreen(byte oldPicNotValid);
	void restoreAndDelete(int argc, reg_t *argv);
	void reAnimate(Common::Rect rect);
	void addToPicDrawCels();
	void addToPicDrawView(GuiResourceId viewId, int16 loopNo, int16 celNo, int16 leftPos, int16 topPos, int16 priority, int16 control);
	void printAnimateList(Console *con);

	virtual void kernelAnimate(reg_t listReference, bool cycle, int argc, reg_t *argv);
	virtual void kernelAddToPicList(reg_t listReference, int argc, reg_t *argv);
	virtual void kernelAddToPicView(GuiResourceId viewId, int16 loopNo, int16 celNo, int16 leftPos, int16 topPos, int16 priority, int16 control);

private:
	void init();

	void addToPicSetPicNotValid();
	void animateShowPic();
	void throttleSpeed();
	void adjustInvalidCels(GfxView *view, AnimateList::iterator it);
	void processViewScaling(GfxView *view, AnimateList::iterator it);
	void setNsRect(GfxView *view, AnimateList::iterator it);

	EngineState *_s;
	GfxCache *_cache;
	GfxPorts *_ports;
	GfxPaint16 *_paint16;
	GfxScreen *_screen;
	GfxPalette *_palette;
	GfxCursor *_cursor;
	GfxTransitions *_transitions;

	AnimateList _list;
	AnimateArray _lastCastData;

	bool _ignoreFastCast;
};

} // End of namespace Sci

#endif
