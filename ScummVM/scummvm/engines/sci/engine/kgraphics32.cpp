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

#include "common/system.h"

#include "engines/util.h"
#include "graphics/cursorman.h"
#include "graphics/surface.h"

#include "gui/message.h"

#include "sci/sci.h"
#include "sci/event.h"
#include "sci/resource.h"
#include "sci/engine/features.h"
#include "sci/engine/state.h"
#include "sci/engine/selector.h"
#include "sci/engine/kernel.h"
#include "sci/graphics/animate.h"
#include "sci/graphics/cache.h"
#include "sci/graphics/compare.h"
#include "sci/graphics/controls16.h"
#include "sci/graphics/coordadjuster.h"
#include "sci/graphics/cursor.h"
#include "sci/graphics/palette.h"
#include "sci/graphics/paint16.h"
#include "sci/graphics/picture.h"
#include "sci/graphics/ports.h"
#include "sci/graphics/screen.h"
#include "sci/graphics/text16.h"
#include "sci/graphics/view.h"
#ifdef ENABLE_SCI32
#include "sci/graphics/controls32.h"
#include "sci/graphics/font.h"	// TODO: remove once kBitmap is moved in a separate class
#include "sci/graphics/text32.h"
#include "sci/graphics/frameout.h"
#endif

namespace Sci {
#ifdef ENABLE_SCI32

extern void showScummVMDialog(const Common::String &message);

reg_t kIsHiRes(EngineState *s, int argc, reg_t *argv) {
	// Returns 0 if the screen width or height is less than 640 or 400,
	// respectively.
	if (g_system->getWidth() < 640 || g_system->getHeight() < 400)
		return make_reg(0, 0);

	return make_reg(0, 1);
}

// SCI32 variant, can't work like sci16 variants
reg_t kCantBeHere32(EngineState *s, int argc, reg_t *argv) {
	// TODO
//	reg_t curObject = argv[0];
//	reg_t listReference = (argc > 1) ? argv[1] : NULL_REG;

	return NULL_REG;
}

reg_t kAddScreenItem(EngineState *s, int argc, reg_t *argv) {
	if (g_sci->_gfxFrameout->findScreenItem(argv[0]) == NULL)
		g_sci->_gfxFrameout->kernelAddScreenItem(argv[0]);
	else
		g_sci->_gfxFrameout->kernelUpdateScreenItem(argv[0]);
	return s->r_acc;
}

reg_t kUpdateScreenItem(EngineState *s, int argc, reg_t *argv) {
	g_sci->_gfxFrameout->kernelUpdateScreenItem(argv[0]);
	return s->r_acc;
}

reg_t kDeleteScreenItem(EngineState *s, int argc, reg_t *argv) {
	g_sci->_gfxFrameout->kernelDeleteScreenItem(argv[0]);
	return s->r_acc;
}

reg_t kAddPlane(EngineState *s, int argc, reg_t *argv) {
	g_sci->_gfxFrameout->kernelAddPlane(argv[0]);
	return s->r_acc;
}

reg_t kDeletePlane(EngineState *s, int argc, reg_t *argv) {
	g_sci->_gfxFrameout->kernelDeletePlane(argv[0]);
	return s->r_acc;
}

reg_t kUpdatePlane(EngineState *s, int argc, reg_t *argv) {
	g_sci->_gfxFrameout->kernelUpdatePlane(argv[0]);
	return s->r_acc;
}

reg_t kAddPicAt(EngineState *s, int argc, reg_t *argv) {
	reg_t planeObj = argv[0];
	GuiResourceId pictureId = argv[1].toUint16();
	int16 pictureX = argv[2].toSint16();
	int16 pictureY = argv[3].toSint16();

	g_sci->_gfxFrameout->kernelAddPicAt(planeObj, pictureId, pictureX, pictureY);
	return s->r_acc;
}

reg_t kGetHighPlanePri(EngineState *s, int argc, reg_t *argv) {
	return make_reg(0, g_sci->_gfxFrameout->kernelGetHighPlanePri());
}

reg_t kFrameOut(EngineState *s, int argc, reg_t *argv) {
	g_sci->_gfxFrameout->kernelFrameout();
	return NULL_REG;
}

reg_t kObjectIntersect(EngineState *s, int argc, reg_t *argv) {
	Common::Rect objRect1 = g_sci->_gfxCompare->getNSRect(argv[0]);
	Common::Rect objRect2 = g_sci->_gfxCompare->getNSRect(argv[1]);
	return make_reg(0, objRect1.intersects(objRect2));
}

// Tests if the coordinate is on the passed object
reg_t kIsOnMe(EngineState *s, int argc, reg_t *argv) {
	uint16 x = argv[0].toUint16();
	uint16 y = argv[1].toUint16();
	reg_t targetObject = argv[2];
	uint16 illegalBits = argv[3].getOffset();
	Common::Rect nsRect = g_sci->_gfxCompare->getNSRect(targetObject, true);

	// we assume that x, y are local coordinates

	bool contained = nsRect.contains(x, y);
	if (contained && illegalBits) {
		// If illegalbits are set, we check the color of the pixel that got clicked on
		//  for now, we return false if the pixel is transparent
		//  although illegalBits may get differently set, don't know yet how this really works out
		uint16 viewId = readSelectorValue(s->_segMan, targetObject, SELECTOR(view));
		int16 loopNo = readSelectorValue(s->_segMan, targetObject, SELECTOR(loop));
		int16 celNo = readSelectorValue(s->_segMan, targetObject, SELECTOR(cel));
		if (g_sci->_gfxCompare->kernelIsItSkip(viewId, loopNo, celNo, Common::Point(x - nsRect.left, y - nsRect.top)))
			contained = false;
	}
	return make_reg(0, contained);
}

reg_t kCreateTextBitmap(EngineState *s, int argc, reg_t *argv) {
	switch (argv[0].toUint16()) {
	case 0: {
		if (argc != 4) {
			warning("kCreateTextBitmap(0): expected 4 arguments, got %i", argc);
			return NULL_REG;
		}
		reg_t object = argv[3];
		Common::String text = s->_segMan->getString(readSelector(s->_segMan, object, SELECTOR(text)));
		debugC(kDebugLevelStrings, "kCreateTextBitmap case 0 (%04x:%04x, %04x:%04x, %04x:%04x)",
				PRINT_REG(argv[1]), PRINT_REG(argv[2]), PRINT_REG(argv[3]));
		debugC(kDebugLevelStrings, "%s", text.c_str());
		int16 maxWidth = argv[1].toUint16();
		int16 maxHeight = argv[2].toUint16();
		g_sci->_gfxCoordAdjuster->fromScriptToDisplay(maxHeight, maxWidth);
		// These values can be larger than the screen in the SQ6 demo, room 100
		// TODO: Find out why. For now, don't show any text in that room.
		if (g_sci->getGameId() == GID_SQ6 && g_sci->isDemo() && s->currentRoomNumber() == 100)
			return NULL_REG;
		return g_sci->_gfxText32->createTextBitmap(object, maxWidth, maxHeight);
	}
	case 1: {
		if (argc != 2) {
			warning("kCreateTextBitmap(1): expected 2 arguments, got %i", argc);
			return NULL_REG;
		}
		reg_t object = argv[1];
		Common::String text = s->_segMan->getString(readSelector(s->_segMan, object, SELECTOR(text)));
		debugC(kDebugLevelStrings, "kCreateTextBitmap case 1 (%04x:%04x)", PRINT_REG(argv[1]));
		debugC(kDebugLevelStrings, "%s", text.c_str());
		return g_sci->_gfxText32->createTextBitmap(object);
	}
	default:
		warning("CreateTextBitmap(%d)", argv[0].toUint16());
		return NULL_REG;
	}
}

reg_t kDisposeTextBitmap(EngineState *s, int argc, reg_t *argv) {
	g_sci->_gfxText32->disposeTextBitmap(argv[0]);
	return s->r_acc;
}

reg_t kWinHelp(EngineState *s, int argc, reg_t *argv) {
	switch (argv[0].toUint16()) {
	case 1:
		// Load a help file
		// Maybe in the future we can implement this, but for now this message should suffice
		showScummVMDialog("Please use an external viewer to open the game's help file: " + s->_segMan->getString(argv[1]));
		break;
	case 2:
		// Looks like some init function
		break;
	default:
		warning("Unknown kWinHelp subop %d", argv[0].toUint16());
	}

	return s->r_acc;
}

/**
 * Used for scene transitions, replacing (but reusing parts of) the old
 * transition code.
 */
reg_t kSetShowStyle(EngineState *s, int argc, reg_t *argv) {
	// Can be called with 7 or 8 parameters
	// The style defines which transition to perform. Related to the transition
	// tables inside graphics/transitions.cpp
	uint16 showStyle = argv[0].toUint16();	// 0 - 15
	reg_t planeObj = argv[1];	// the affected plane
	Common::String planeObjName = s->_segMan->getObjectName(planeObj);
	uint16 seconds = argv[2].toUint16();	// seconds that the transition lasts
	uint16 backColor =  argv[3].toUint16();	// target back color(?). When fading out, it's 0x0000. When fading in, it's 0xffff
	int16 priority = argv[4].toSint16();	// always 0xc8 (200) when fading in/out
	uint16 animate = argv[5].toUint16();	// boolean, animate or not while the transition lasts
	uint16 refFrame = argv[6].toUint16();	// refFrame, always 0 when fading in/out
	int16 divisions;

	// If the game has the pFadeArray selector, another parameter is used here,
	// before the optional last parameter
	bool hasFadeArray = g_sci->getKernel()->findSelector("pFadeArray") > 0;
	if (hasFadeArray) {
		// argv[7]
		divisions = (argc >= 9) ? argv[8].toSint16() : -1;	// divisions (transition steps?)
	} else {
		divisions = (argc >= 8) ? argv[7].toSint16() : -1;	// divisions (transition steps?)
	}

	if (showStyle > 15) {
		warning("kSetShowStyle: Illegal style %d for plane %04x:%04x", showStyle, PRINT_REG(planeObj));
		return s->r_acc;
	}

	// GK1 calls fadeout (13) / fadein (14) with the following parameters:
	// seconds: 1
	// backColor: 0 / -1
	// fade: 200
	// animate: 0
	// refFrame: 0
	// divisions: 0 / 20

	// TODO: Check if the plane is in the list of planes to draw

	Common::String effectName = "unknown";

	switch (showStyle) {
	case 0:		// no transition / show
		effectName = "show";
		break;
	case 13:	// fade out
		effectName = "fade out";
		// TODO
		break;
	case 14:	// fade in
		effectName = "fade in";
		// TODO
		break;
	default:
		// TODO
		break;
	}

	warning("kSetShowStyle: effect %d (%s) - plane: %04x:%04x (%s), sec: %d, "
			"back: %d, prio: %d, animate: %d, ref frame: %d, divisions: %d",
			showStyle, effectName.c_str(), PRINT_REG(planeObj), planeObjName.c_str(),
			seconds, backColor, priority, animate, refFrame, divisions);
	return s->r_acc;
}

reg_t kCelInfo(EngineState *s, int argc, reg_t *argv) {
	// Used by Shivers 1, room 23601 to determine what blocks on the red door puzzle board
	// are occupied by pieces already

	switch (argv[0].toUint16()) {	// subops 0 - 4
		// 0 - return the view
		// 1 - return the loop
		// 2, 3 - nop
		case 4: {
			GuiResourceId viewId = argv[1].toSint16();
			int16 loopNo = argv[2].toSint16();
			int16 celNo = argv[3].toSint16();
			int16 x = argv[4].toUint16();
			int16 y = argv[5].toUint16();
			byte color = g_sci->_gfxCache->kernelViewGetColorAtCoordinate(viewId, loopNo, celNo, x, y);
			return make_reg(0, color);
		}
		default: {
			kStub(s, argc, argv);
			return s->r_acc;
		}
	}
}

reg_t kScrollWindow(EngineState *s, int argc, reg_t *argv) {
	// Used by SQ6 and LSL6 hires for the text area in the bottom of the
	// screen. The relevant scripts also exist in Phantasmagoria 1, but they're
	// unused. This is always called by scripts 64906 (ScrollerWindow) and
	// 64907 (ScrollableWindow).

	reg_t kWindow = argv[1];
	uint16 op = argv[0].toUint16();
	switch (op) {
	case 0:	// Init
		g_sci->_gfxFrameout->initScrollText(argv[2].toUint16());	// maxItems
		g_sci->_gfxFrameout->clearScrollTexts();
		return argv[1];	// kWindow
	case 1: // Show message, called by ScrollableWindow::addString
	case 14: // Modify message, called by ScrollableWindow::modifyString
		// 5 or 6 parameters
		// Seems to be called with 5 parameters when the narrator speaks, and
		// with 6 when Roger speaks
		{
		Common::String text = s->_segMan->getString(argv[2]);
		uint16 x = 0;//argv[3].toUint16();	// TODO: can't be x (values are all wrong)
		uint16 y = 0;//argv[4].toUint16();	// TODO: can't be y (values are all wrong)
		// TODO: argv[5] is an optional unknown parameter (an integer set to 0)
		g_sci->_gfxFrameout->addScrollTextEntry(text, kWindow, x, y, (op == 14));
		}
		break;
	case 2: // Clear, called by ScrollableWindow::erase
		g_sci->_gfxFrameout->clearScrollTexts();
		break;
	case 3: // Page up, called by ScrollableWindow::scrollTo
		// TODO
		kStub(s, argc, argv);
		break;
	case 4: // Page down, called by ScrollableWindow::scrollTo
		// TODO
		kStub(s, argc, argv);
		break;
	case 5: // Up arrow, called by ScrollableWindow::scrollTo
		g_sci->_gfxFrameout->prevScrollText();
		break;
	case 6: // Down arrow, called by ScrollableWindow::scrollTo
		g_sci->_gfxFrameout->nextScrollText();
		break;
	case 7: // Home, called by ScrollableWindow::scrollTo
		g_sci->_gfxFrameout->firstScrollText();
		break;
	case 8: // End, called by ScrollableWindow::scrollTo
		g_sci->_gfxFrameout->lastScrollText();
		break;
	case 9: // Resize, called by ScrollableWindow::resize and ScrollerWindow::resize
		// TODO
		kStub(s, argc, argv);
		break;
	case 10: // Where, called by ScrollableWindow::where
		// TODO
		// argv[2] is an unknown integer
		// Silenced the warnings because of the high amount of console spam
		//kStub(s, argc, argv);
		break;
	case 11: // Go, called by ScrollableWindow::scrollTo
		// 2 extra parameters here
		// TODO
		kStub(s, argc, argv);
		break;
	case 12: // Insert, called by ScrollableWindow::insertString
		// 3 extra parameters here
		// TODO
		kStub(s, argc, argv);
		break;
	// case 13 (Delete) is handled below
	// case 14 (Modify) is handled above
	case 15: // Hide, called by ScrollableWindow::hide
		g_sci->_gfxFrameout->toggleScrollText(false);
		break;
	case 16: // Show, called by ScrollableWindow::show
		g_sci->_gfxFrameout->toggleScrollText(true);
		break;
	case 17: // Destroy, called by ScrollableWindow::dispose
		g_sci->_gfxFrameout->clearScrollTexts();
		break;
	case 13: // Delete, unused
	case 18: // Text, unused
	case 19: // Reconstruct, unused
		error("kScrollWindow: Unused subop %d invoked", op);
		break;
	default:
		error("kScrollWindow: unknown subop %d", op);
		break;
	}

	return s->r_acc;
}

reg_t kSetFontRes(EngineState *s, int argc, reg_t *argv) {
	// TODO: This defines the resolution that the fonts are supposed to be displayed
	// in. Currently, this is only used for showing high-res fonts in GK1 Mac, but
	// should be extended to handle other font resolutions such as those

	int xResolution = argv[0].toUint16();
	//int yResolution = argv[1].toUint16();

	g_sci->_gfxScreen->setFontIsUpscaled(xResolution == 640 &&
			g_sci->_gfxScreen->getUpscaledHires() != GFX_SCREEN_UPSCALED_DISABLED);

	return s->r_acc;
}

reg_t kFont(EngineState *s, int argc, reg_t *argv) {
	// Handle font settings for SCI2.1

	switch (argv[0].toUint16()) {
	case 1:
		// Set font resolution
		return kSetFontRes(s, argc - 1, argv + 1);
	default:
		warning("kFont: unknown subop %d", argv[0].toUint16());
	}

	return s->r_acc;
}

// TODO: Eventually, all of the kBitmap operations should be put
// in a separate class

#define BITMAP_HEADER_SIZE 46

reg_t kBitmap(EngineState *s, int argc, reg_t *argv) {
	// Used for bitmap operations in SCI2.1 and SCI3.
	// This is the SCI2.1 version, the functionality seems to have changed in SCI3.

	switch (argv[0].toUint16()) {
	case 0:	// init bitmap surface
		{
		// 6 params, called e.g. from TextView::init() in Torin's Passage,
		// script 64890 and TransView::init() in script 64884
		uint16 width = argv[1].toUint16();
		uint16 height = argv[2].toUint16();
		//uint16 skip = argv[3].toUint16();
		uint16 back = argv[4].toUint16();	// usually equals skip
		//uint16 width2 = (argc >= 6) ? argv[5].toUint16() : 0;
		//uint16 height2 = (argc >= 7) ? argv[6].toUint16() : 0;
		//uint16 transparentFlag = (argc >= 8) ? argv[7].toUint16() : 0;

		// TODO: skip, width2, height2, transparentFlag
		// (used for transparent bitmaps)
		int entrySize = width * height + BITMAP_HEADER_SIZE;
		reg_t memoryId = s->_segMan->allocateHunkEntry("Bitmap()", entrySize);
		byte *memoryPtr = s->_segMan->getHunkPointer(memoryId);
		memset(memoryPtr, 0, BITMAP_HEADER_SIZE);	// zero out the bitmap header
		memset(memoryPtr + BITMAP_HEADER_SIZE, back, width * height);
		// Save totalWidth, totalHeight
		// TODO: Save the whole bitmap header, like SSCI does
		WRITE_LE_UINT16(memoryPtr, width);
		WRITE_LE_UINT16(memoryPtr + 2, height);
		return memoryId;
		}
		break;
	case 1:	// dispose text bitmap surface
		return kDisposeTextBitmap(s, argc - 1, argv + 1);
	case 2:	// dispose bitmap surface, with extra param
		// 2 params, called e.g. from MenuItem::dispose in Torin's Passage,
		// script 64893
		warning("kBitmap(2), unk1 %d, bitmap ptr %04x:%04x", argv[1].toUint16(), PRINT_REG(argv[2]));
		break;
	case 3:	// tiled surface
		{
		// 6 params, called e.g. from TiledBitmap::resize() in Torin's Passage,
		// script 64869
		reg_t hunkId = argv[1];	// obtained from kBitmap(0)
		// The tiled view seems to always have 2 loops.
		// These loops need to have 1 cel in loop 0 and 8 cels in loop 1.
		uint16 viewNum = argv[2].toUint16();	// vTiles selector
		uint16 loop = argv[3].toUint16();
		uint16 cel = argv[4].toUint16();
		uint16 x = argv[5].toUint16();
		uint16 y = argv[6].toUint16();

		byte *memoryPtr = s->_segMan->getHunkPointer(hunkId);
		// Get totalWidth, totalHeight
		uint16 totalWidth = READ_LE_UINT16(memoryPtr);
		uint16 totalHeight = READ_LE_UINT16(memoryPtr + 2);
		byte *bitmap = memoryPtr + BITMAP_HEADER_SIZE;

		GfxView *view = g_sci->_gfxCache->getView(viewNum);
		uint16 tileWidth = view->getWidth(loop, cel);
		uint16 tileHeight = view->getHeight(loop, cel);
		const byte *tileBitmap = view->getBitmap(loop, cel);
		uint16 width = MIN<uint16>(totalWidth - x, tileWidth);
		uint16 height = MIN<uint16>(totalHeight - y, tileHeight);

		for (uint16 curY = 0; curY < height; curY++) {
			for (uint16 curX = 0; curX < width; curX++) {
				bitmap[(curY + y) * totalWidth + (curX + x)] = tileBitmap[curY * tileWidth + curX];
			}
		}

		}
		break;
	case 4:	// add text to bitmap
		{
		// 13 params, called e.g. from TextButton::createBitmap() in Torin's Passage,
		// script 64894
		reg_t hunkId = argv[1];	// obtained from kBitmap(0)
		Common::String text = s->_segMan->getString(argv[2]);
		uint16 textX = argv[3].toUint16();
		uint16 textY = argv[4].toUint16();
		//reg_t unk5 = argv[5];
		//reg_t unk6 = argv[6];
		//reg_t unk7 = argv[7];	// skip?
		//reg_t unk8 = argv[8];	// back?
		//reg_t unk9 = argv[9];
		uint16 fontId = argv[10].toUint16();
		//uint16 mode = argv[11].toUint16();
		uint16 dimmed = argv[12].toUint16();
		//warning("kBitmap(4): bitmap ptr %04x:%04x, font %d, mode %d, dimmed %d - text: \"%s\"",
		//		PRINT_REG(bitmapPtr), font, mode, dimmed, text.c_str());
		uint16 foreColor = 255;	// TODO

		byte *memoryPtr = s->_segMan->getHunkPointer(hunkId);
		// Get totalWidth, totalHeight
		uint16 totalWidth = READ_LE_UINT16(memoryPtr);
		uint16 totalHeight = READ_LE_UINT16(memoryPtr + 2);
		byte *bitmap = memoryPtr + BITMAP_HEADER_SIZE;

		GfxFont *font = g_sci->_gfxCache->getFont(fontId);

		int16 charCount = 0;
		uint16 curX = textX, curY = textY;
		const char *txt = text.c_str();

		while (*txt) {
			charCount = g_sci->_gfxText32->GetLongest(txt, totalWidth, font);
			if (charCount == 0)
				break;

			for (int i = 0; i < charCount; i++) {
				unsigned char curChar = txt[i];
				font->drawToBuffer(curChar, curY, curX, foreColor, dimmed, bitmap, totalWidth, totalHeight);
				curX += font->getCharWidth(curChar);
			}

			curX = textX;
			curY += font->getHeight();
			txt += charCount;
			while (*txt == ' ')
				txt++; // skip over breaking spaces
		}

		}
		break;
	case 5:	// fill with color
		{
		// 6 params, called e.g. from TextView::init() and TextView::draw()
		// in Torin's Passage, script 64890
		reg_t hunkId = argv[1];	// obtained from kBitmap(0)
		uint16 x = argv[2].toUint16();
		uint16 y = argv[3].toUint16();
		uint16 fillWidth = argv[4].toUint16();	// width - 1
		uint16 fillHeight = argv[5].toUint16();	// height - 1
		uint16 back = argv[6].toUint16();

		byte *memoryPtr = s->_segMan->getHunkPointer(hunkId);
		// Get totalWidth, totalHeight
		uint16 totalWidth = READ_LE_UINT16(memoryPtr);
		uint16 totalHeight = READ_LE_UINT16(memoryPtr + 2);
		uint16 width = MIN<uint16>(totalWidth - x, fillWidth);
		uint16 height = MIN<uint16>(totalHeight - y, fillHeight);
		byte *bitmap = memoryPtr + BITMAP_HEADER_SIZE;

		for (uint16 curY = 0; curY < height; curY++) {
			for (uint16 curX = 0; curX < width; curX++) {
				bitmap[(curY + y) * totalWidth + (curX + x)] = back;
			}
		}

		}
		break;
	default:
		kStub(s, argc, argv);
		break;
	}

	return s->r_acc;
}

// Used for edit boxes in save/load dialogs. It's a rewritten version of kEditControl,
// but it handles events on its own, using an internal loop, instead of using SCI
// scripts for event management like kEditControl does. Called by script 64914,
// DEdit::hilite().
reg_t kEditText(EngineState *s, int argc, reg_t *argv) {
	reg_t controlObject = argv[0];

	if (!controlObject.isNull()) {
		g_sci->_gfxControls32->kernelTexteditChange(controlObject);
	}

	return s->r_acc;
}

reg_t kAddLine(EngineState *s, int argc, reg_t *argv) {
	reg_t plane = argv[0];
	Common::Point startPoint(argv[1].toUint16(), argv[2].toUint16());
	Common::Point endPoint(argv[3].toUint16(), argv[4].toUint16());
	// argv[5] is unknown (a number, usually 200)
	byte color = (byte)argv[6].toUint16();
	byte priority = (byte)argv[7].toUint16();
	byte control = (byte)argv[8].toUint16();
	// argv[9] is unknown (usually a small number, 1 or 2). Thickness, perhaps?
	return g_sci->_gfxFrameout->addPlaneLine(plane, startPoint, endPoint, color, priority, control);
}

reg_t kUpdateLine(EngineState *s, int argc, reg_t *argv) {
	reg_t hunkId = argv[0];
	reg_t plane = argv[1];
	Common::Point startPoint(argv[2].toUint16(), argv[3].toUint16());
	Common::Point endPoint(argv[4].toUint16(), argv[5].toUint16());
	// argv[6] is unknown (a number, usually 200)
	byte color = (byte)argv[7].toUint16();
	byte priority = (byte)argv[8].toUint16();
	byte control = (byte)argv[9].toUint16();
	// argv[10] is unknown (usually a small number, 1 or 2). Thickness, perhaps?
	g_sci->_gfxFrameout->updatePlaneLine(plane, hunkId, startPoint, endPoint, color, priority, control);
	return s->r_acc;
}
reg_t kDeleteLine(EngineState *s, int argc, reg_t *argv) {
	reg_t hunkId = argv[0];
	reg_t plane = argv[1];
	g_sci->_gfxFrameout->deletePlaneLine(plane, hunkId);
	return s->r_acc;
}

reg_t kSetScroll(EngineState *s, int argc, reg_t *argv) {
	// Called in the intro of LSL6 hires (room 110)
	// The end effect of this is the same as the old screen scroll transition

	// 7 parameters
	reg_t planeObject = argv[0];
	//int16 x = argv[1].toSint16();
	//int16 y = argv[2].toSint16();
	uint16 pictureId = argv[3].toUint16();
	// param 4: int (0 in LSL6, probably scroll direction? The picture in LSL6 scrolls down)
	// param 5: int (first call is 1, then the subsequent one is 0 in LSL6)
	// param 6: optional int (0 in LSL6)

	// Set the new picture directly for now
	//writeSelectorValue(s->_segMan, planeObject, SELECTOR(left), x);
	//writeSelectorValue(s->_segMan, planeObject, SELECTOR(top), y);
	writeSelectorValue(s->_segMan, planeObject, SELECTOR(picture), pictureId);
	// and update our draw list
	g_sci->_gfxFrameout->kernelUpdatePlane(planeObject);

	// TODO
	return kStub(s, argc, argv);
}

reg_t kPalCycle(EngineState *s, int argc, reg_t *argv) {
	// Examples: GK1 room 480 (Bayou ritual), LSL6 room 100 (title screen)

	switch (argv[0].toUint16()) {
	case 0: {	// Palette animation initialization
		// 3 or 4 extra params
		// Case 1 sends fromColor and speed again, so we don't need them here.
		// Only toColor is stored
		//uint16 fromColor = argv[1].toUint16();
		s->_palCycleToColor = argv[2].toUint16();
		//uint16 speed = argv[3].toUint16();

		// Invalidate the picture, so that the palette steps calls (case 1
		// below) can update its palette without it being overwritten by the
		// view/picture palettes.
		g_sci->_gfxScreen->_picNotValid = 1;

		// TODO: The fourth optional parameter is an unknown integer, and is 0 by default
		if (argc == 5) {
			// When this variant is used, picNotValid doesn't seem to be set
			// (e.g. GK1 room 480). In this case, the animation step calls are
			// not made, so perhaps this signifies the palette cycling steps
			// to make.
			// GK1 sets this to 6 (6 palette steps?)
			g_sci->_gfxScreen->_picNotValid = 0;
		}
		kStub(s, argc, argv);
		}
		break;
	case 1:	{ // Palette animation step
		// This is the same as the old kPaletteAnimate call, with 1 set of colors.
		// The end color is set up during initialization in case 0 above.

		// 1 or 2 extra params
		uint16 fromColor = argv[1].toUint16();
		uint16 speed = (argc == 2) ? 1 : argv[2].toUint16();
		// TODO: For some reason, this doesn't set the color correctly
		// (e.g. LSL6 intro, room 100, Sierra logo)
		if (g_sci->_gfxPalette->kernelAnimate(fromColor, s->_palCycleToColor, speed))
			g_sci->_gfxPalette->kernelAnimateSet();
		}
		// No kStub() call here, as this gets called loads of times, like kPaletteAnimate
		break;
	// case 2 hasn't been encountered
	// case 3 hasn't been encountered
	case 4:	// reset any palette cycling and make the picture valid again
		// Gets called when changing rooms and after palette cycling animations finish
		// 0 or 1 extra params
		if (argc == 1) {
			g_sci->_gfxScreen->_picNotValid = 0;
			// TODO: This also seems to perform more steps
		} else {
			// The variant with the 1 extra param resets remapping to base
			// TODO
		}
		kStub(s, argc, argv);
		break;
	default:
		// TODO
		kStub(s, argc, argv);
		break;
	}

	return s->r_acc;
}

reg_t kRemapColors32(EngineState *s, int argc, reg_t *argv) {
	uint16 operation = argv[0].toUint16();

	switch (operation) {
	case 0:	{ // turn remapping off
		// WORKAROUND: Game scripts in QFG4 erroneously turn remapping off in room
		// 140 (the character point allocation screen) and never turn it back on,
		// even if it's clearly used in that screen.
		if (g_sci->getGameId() == GID_QFG4 && s->currentRoomNumber() == 140)
			return s->r_acc;

		int16 base = (argc >= 2) ? argv[1].toSint16() : 0;
		if (base > 0)
			warning("kRemapColors(0) called with base %d", base);
		g_sci->_gfxPalette->resetRemapping();
		}
		break;
	case 1:	{ // remap by range
		uint16 color = argv[1].toUint16();
		uint16 from = argv[2].toUint16();
		uint16 to = argv[3].toUint16();
		uint16 base = argv[4].toUint16();
		uint16 unk5 = (argc >= 6) ? argv[5].toUint16() : 0;
		if (unk5 > 0)
			warning("kRemapColors(1) called with 6 parameters, unknown parameter is %d", unk5);
		g_sci->_gfxPalette->setRemappingRange(color, from, to, base);
		}
		break;
	case 2:	{ // remap by percent
		uint16 color = argv[1].toUint16();
		uint16 percent = argv[2].toUint16(); // 0 - 100
		if (argc >= 4)
			warning("RemapByPercent called with 4 parameters, unknown parameter is %d", argv[3].toUint16());
		g_sci->_gfxPalette->setRemappingPercent(color, percent);
		}
		break;
	case 3:	{ // remap to gray
		// Example call: QFG4 room 490 (Baba Yaga's hut) - params are color 253, 75% and 0.
		// In this room, it's used for the cloud before Baba Yaga appears.
		int16 color = argv[1].toSint16();
		int16 percent = argv[2].toSint16(); // 0 - 100
		if (argc >= 4)
			warning("RemapToGray called with 4 parameters, unknown parameter is %d", argv[3].toUint16());
		g_sci->_gfxPalette->setRemappingPercentGray(color, percent);
		}
		break;
	case 4:	{ // remap to percent gray
		// Example call: QFG4 rooms 530/535 (swamp) - params are 253, 100%, 200
		int16 color = argv[1].toSint16();
		int16 percent = argv[2].toSint16(); // 0 - 100
		// argv[3] is unknown (a number, e.g. 200) - start color, perhaps?
		if (argc >= 5)
			warning("RemapToGrayPercent called with 5 parameters, unknown parameter is %d", argv[4].toUint16());
		g_sci->_gfxPalette->setRemappingPercentGray(color, percent);
		}
		break;
	case 5:	{ // don't map to range
		//int16 mapping = argv[1].toSint16();
		uint16 intensity = argv[2].toUint16();
		// HACK for PQ4
		if (g_sci->getGameId() == GID_PQ4)
			g_sci->_gfxPalette->kernelSetIntensity(0, 255, intensity, true);

		kStub(s, argc, argv);
		}
		break;
	default:
		break;
	}

	return s->r_acc;
}

#endif

} // End of namespace Sci
