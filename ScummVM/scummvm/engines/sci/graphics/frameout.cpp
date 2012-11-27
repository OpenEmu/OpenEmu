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

#include "common/algorithm.h"
#include "common/events.h"
#include "common/keyboard.h"
#include "common/list_intern.h"
#include "common/str.h"
#include "common/system.h"
#include "common/textconsole.h"
#include "engines/engine.h"
#include "graphics/palette.h"
#include "graphics/surface.h"

#include "sci/sci.h"
#include "sci/console.h"
#include "sci/engine/kernel.h"
#include "sci/engine/state.h"
#include "sci/engine/selector.h"
#include "sci/engine/vm.h"
#include "sci/graphics/cache.h"
#include "sci/graphics/coordadjuster.h"
#include "sci/graphics/compare.h"
#include "sci/graphics/font.h"
#include "sci/graphics/view.h"
#include "sci/graphics/screen.h"
#include "sci/graphics/paint32.h"
#include "sci/graphics/palette.h"
#include "sci/graphics/picture.h"
#include "sci/graphics/text32.h"
#include "sci/graphics/frameout.h"
#include "sci/video/robot_decoder.h"

namespace Sci {

// TODO/FIXME: This is all guesswork

enum SciSpeciaPlanelPictureCodes {
	kPlaneTranslucent  = 0xfffe,	// -2
	kPlanePlainColored = 0xffff		// -1
};

GfxFrameout::GfxFrameout(SegManager *segMan, ResourceManager *resMan, GfxCoordAdjuster *coordAdjuster, GfxCache *cache, GfxScreen *screen, GfxPalette *palette, GfxPaint32 *paint32)
	: _segMan(segMan), _resMan(resMan), _cache(cache), _screen(screen), _palette(palette), _paint32(paint32) {

	_coordAdjuster = (GfxCoordAdjuster32 *)coordAdjuster;
	_curScrollText = -1;
	_showScrollText = false;
	_maxScrollTexts = 0;
}

GfxFrameout::~GfxFrameout() {
	clear();
}

void GfxFrameout::clear() {
	deletePlaneItems(NULL_REG);
	_planes.clear();
	deletePlanePictures(NULL_REG);
	clearScrollTexts();
}

void GfxFrameout::clearScrollTexts() {
	_scrollTexts.clear();
	_curScrollText = -1;
}

void GfxFrameout::addScrollTextEntry(Common::String &text, reg_t kWindow, uint16 x, uint16 y, bool replace) {
	//reg_t bitmapHandle = g_sci->_gfxText32->createScrollTextBitmap(text, kWindow);
	// HACK: We set the container dimensions manually
	reg_t bitmapHandle = g_sci->_gfxText32->createScrollTextBitmap(text, kWindow, 480, 70);
	ScrollTextEntry textEntry;
	textEntry.bitmapHandle = bitmapHandle;
	textEntry.kWindow = kWindow;
	textEntry.x = x;
	textEntry.y = y;
	if (!replace || _scrollTexts.size() == 0) {
		if (_scrollTexts.size() > _maxScrollTexts) {
			_scrollTexts.remove_at(0);
			_curScrollText--;
		}
		_scrollTexts.push_back(textEntry);
		_curScrollText++;
	} else {
		_scrollTexts.pop_back();
		_scrollTexts.push_back(textEntry);
	}
}

void GfxFrameout::showCurrentScrollText() {
	if (!_showScrollText || _curScrollText < 0)
		return;

	uint16 size = (uint16)_scrollTexts.size();
	if (size > 0) {
		assert(_curScrollText < size);
		ScrollTextEntry textEntry = _scrollTexts[_curScrollText];
		g_sci->_gfxText32->drawScrollTextBitmap(textEntry.kWindow, textEntry.bitmapHandle, textEntry.x, textEntry.y);
	}
}

extern void showScummVMDialog(const Common::String &message);

void GfxFrameout::kernelAddPlane(reg_t object) {
	PlaneEntry newPlane;

	if (_planes.empty()) {
		// There has to be another way for sierra sci to do this or maybe script resolution is compiled into
		//  interpreter (TODO)
		uint16 scriptWidth = readSelectorValue(_segMan, object, SELECTOR(resX));
		uint16 scriptHeight = readSelectorValue(_segMan, object, SELECTOR(resY));

		// Phantasmagoria 2 doesn't specify a script width/height
		if (g_sci->getGameId() == GID_PHANTASMAGORIA2) {
			scriptWidth = 640;
			scriptHeight = 480;
		}

		assert(scriptWidth > 0 && scriptHeight > 0);
		_coordAdjuster->setScriptsResolution(scriptWidth, scriptHeight);
	}

	// Import of QfG character files dialog is shown in QFG4.
	// Display additional popup information before letting user use it.
	// For the SCI0-SCI1.1 version of this, check kDrawControl().
	if (g_sci->inQfGImportRoom() && !strcmp(_segMan->getObjectName(object), "DSPlane")) {
		showScummVMDialog("Characters saved inside ScummVM are shown "
				"automatically. Character files saved in the original "
				"interpreter need to be put inside ScummVM's saved games "
				"directory and a prefix needs to be added depending on which "
				"game it was saved in: 'qfg1-' for Quest for Glory 1, 'qfg2-' "
				"for Quest for Glory 2, 'qfg3-' for Quest for Glory 3. "
				"Example: 'qfg2-thief.sav'.");
	}

	newPlane.object = object;
	newPlane.priority = readSelectorValue(_segMan, object, SELECTOR(priority));
	newPlane.lastPriority = -1; // hidden
	newPlane.planeOffsetX = 0;
	newPlane.planeOffsetY = 0;
	newPlane.pictureId = kPlanePlainColored;
	newPlane.planePictureMirrored = false;
	newPlane.planeBack = 0;
	_planes.push_back(newPlane);

	kernelUpdatePlane(object);
}

void GfxFrameout::kernelUpdatePlane(reg_t object) {
	for (PlaneList::iterator it = _planes.begin(); it != _planes.end(); ++it) {
		if (it->object == object) {
			// Read some information
			it->priority = readSelectorValue(_segMan, object, SELECTOR(priority));
			GuiResourceId lastPictureId = it->pictureId;
			it->pictureId = readSelectorValue(_segMan, object, SELECTOR(picture));
			if (lastPictureId != it->pictureId) {
				// picture got changed, load new picture
				deletePlanePictures(object);
				// Draw the plane's picture if it's not a translucent/plane colored frame
				if ((it->pictureId != kPlanePlainColored) && (it->pictureId != kPlaneTranslucent)) {
					// SQ6 gives us a bad picture number for the control menu
					if (_resMan->testResource(ResourceId(kResourceTypePic, it->pictureId)))
						addPlanePicture(object, it->pictureId, 0);
				}
			}
			it->planeRect.top = readSelectorValue(_segMan, object, SELECTOR(top));
			it->planeRect.left = readSelectorValue(_segMan, object, SELECTOR(left));
			it->planeRect.bottom = readSelectorValue(_segMan, object, SELECTOR(bottom));
			it->planeRect.right = readSelectorValue(_segMan, object, SELECTOR(right));

			_coordAdjuster->fromScriptToDisplay(it->planeRect.top, it->planeRect.left);
			_coordAdjuster->fromScriptToDisplay(it->planeRect.bottom, it->planeRect.right);

			// We get negative left in kq7 in scrolling rooms
			if (it->planeRect.left < 0) {
				it->planeOffsetX = -it->planeRect.left;
				it->planeRect.left = 0;
			} else {
				it->planeOffsetX = 0;
			}

			if (it->planeRect.top < 0) {
				it->planeOffsetY = -it->planeRect.top;
				it->planeRect.top = 0;
			} else {
				it->planeOffsetY = 0;
			}

			// We get bad plane-bottom in sq6
			if (it->planeRect.right > _screen->getWidth())
				it->planeRect.right = _screen->getWidth();
			if (it->planeRect.bottom > _screen->getHeight())
				it->planeRect.bottom = _screen->getHeight();

			it->planeClipRect = Common::Rect(it->planeRect.width(), it->planeRect.height());
			it->upscaledPlaneRect = it->planeRect;
			it->upscaledPlaneClipRect = it->planeClipRect;
			if (_screen->getUpscaledHires()) {
				_screen->adjustToUpscaledCoordinates(it->upscaledPlaneRect.top, it->upscaledPlaneRect.left);
				_screen->adjustToUpscaledCoordinates(it->upscaledPlaneRect.bottom, it->upscaledPlaneRect.right);
				_screen->adjustToUpscaledCoordinates(it->upscaledPlaneClipRect.top, it->upscaledPlaneClipRect.left);
				_screen->adjustToUpscaledCoordinates(it->upscaledPlaneClipRect.bottom, it->upscaledPlaneClipRect.right);
			}

			it->planePictureMirrored = readSelectorValue(_segMan, object, SELECTOR(mirrored));
			it->planeBack = readSelectorValue(_segMan, object, SELECTOR(back));

			sortPlanes();

			// Update the items in the plane
			for (FrameoutList::iterator listIterator = _screenItems.begin(); listIterator != _screenItems.end(); listIterator++) {
				reg_t itemPlane = readSelector(_segMan, (*listIterator)->object, SELECTOR(plane));
				if (object == itemPlane) {
					kernelUpdateScreenItem((*listIterator)->object);
				}
			}

			return;
		}
	}
	error("kUpdatePlane called on plane that wasn't added before");
}

void GfxFrameout::kernelDeletePlane(reg_t object) {
	deletePlaneItems(object);
	deletePlanePictures(object);

	for (PlaneList::iterator it = _planes.begin(); it != _planes.end(); ++it) {
		if (it->object == object) {
			_planes.erase(it);
			Common::Rect planeRect;
			planeRect.top = readSelectorValue(_segMan, object, SELECTOR(top));
			planeRect.left = readSelectorValue(_segMan, object, SELECTOR(left));
			planeRect.bottom = readSelectorValue(_segMan, object, SELECTOR(bottom));
			planeRect.right = readSelectorValue(_segMan, object, SELECTOR(right));

			_coordAdjuster->fromScriptToDisplay(planeRect.top, planeRect.left);
			_coordAdjuster->fromScriptToDisplay(planeRect.bottom, planeRect.right);

			// Blackout removed plane rect
			_paint32->fillRect(planeRect, 0);
			return;
		}
	}
}

void GfxFrameout::addPlanePicture(reg_t object, GuiResourceId pictureId, uint16 startX, uint16 startY) {
	if (pictureId == kPlanePlainColored || pictureId == kPlaneTranslucent)	// sanity check
		return;

	PlanePictureEntry newPicture;
	newPicture.object = object;
	newPicture.pictureId = pictureId;
	newPicture.picture = new GfxPicture(_resMan, _coordAdjuster, 0, _screen, _palette, pictureId, false);
	newPicture.startX = startX;
	newPicture.startY = startY;
	newPicture.pictureCels = 0;
	_planePictures.push_back(newPicture);
}

void GfxFrameout::deletePlanePictures(reg_t object) {
	PlanePictureList::iterator it = _planePictures.begin();

	while (it != _planePictures.end()) {
		if (it->object == object || object.isNull()) {
			delete it->pictureCels;
			delete it->picture;
			it = _planePictures.erase(it);
		} else {
			++it;
		}
	}
}

// Provides the same functionality as kGraph(DrawLine)
reg_t GfxFrameout::addPlaneLine(reg_t object, Common::Point startPoint, Common::Point endPoint, byte color, byte priority, byte control) {
	for (PlaneList::iterator it = _planes.begin(); it != _planes.end(); ++it) {
		if (it->object == object) {
			PlaneLineEntry line;
			line.hunkId = _segMan->allocateHunkEntry("PlaneLine()", 1);	// we basically use this for a unique ID
			line.startPoint = startPoint;
			line.endPoint = endPoint;
			line.color = color;
			line.priority = priority;
			line.control = control;
			it->lines.push_back(line);
			return line.hunkId;
		}
	}

	return NULL_REG;
}

void GfxFrameout::updatePlaneLine(reg_t object, reg_t hunkId, Common::Point startPoint, Common::Point endPoint, byte color, byte priority, byte control) {
	// Check if we're asked to update a line that was never added
	if (hunkId.isNull())
		return;

	for (PlaneList::iterator it = _planes.begin(); it != _planes.end(); ++it) {
		if (it->object == object) {
			for (PlaneLineList::iterator it2 = it->lines.begin(); it2 != it->lines.end(); ++it2) {
				if (it2->hunkId == hunkId) {
					it2->startPoint = startPoint;
					it2->endPoint = endPoint;
					it2->color = color;
					it2->priority = priority;
					it2->control = control;
					return;
				}
			}
		}
	}
}

void GfxFrameout::deletePlaneLine(reg_t object, reg_t hunkId) {
	// Check if we're asked to delete a line that was never added (happens during the intro of LSL6)
	if (hunkId.isNull())
		return;

	for (PlaneList::iterator it = _planes.begin(); it != _planes.end(); ++it) {
		if (it->object == object) {
			for (PlaneLineList::iterator it2 = it->lines.begin(); it2 != it->lines.end(); ++it2) {
				if (it2->hunkId == hunkId) {
					_segMan->freeHunkEntry(hunkId);
					it2 = it->lines.erase(it2);
					return;
				}
			}
		}
	}
}

void GfxFrameout::kernelAddScreenItem(reg_t object) {
	// Ignore invalid items
	if (!_segMan->isObject(object)) {
		warning("kernelAddScreenItem: Attempt to add an invalid object (%04x:%04x)", PRINT_REG(object));
		return;
	}

	FrameoutEntry *itemEntry = new FrameoutEntry();
	memset(itemEntry, 0, sizeof(FrameoutEntry));
	itemEntry->object = object;
	itemEntry->givenOrderNr = _screenItems.size();
	itemEntry->visible = true;
	_screenItems.push_back(itemEntry);

	kernelUpdateScreenItem(object);
}

void GfxFrameout::kernelUpdateScreenItem(reg_t object) {
	// Ignore invalid items
	if (!_segMan->isObject(object)) {
		warning("kernelUpdateScreenItem: Attempt to update an invalid object (%04x:%04x)", PRINT_REG(object));
		return;
	}

	FrameoutEntry *itemEntry = findScreenItem(object);
	if (!itemEntry) {
		warning("kernelUpdateScreenItem: invalid object %04x:%04x", PRINT_REG(object));
		return;
	}

	itemEntry->viewId = readSelectorValue(_segMan, object, SELECTOR(view));
	itemEntry->loopNo = readSelectorValue(_segMan, object, SELECTOR(loop));
	itemEntry->celNo = readSelectorValue(_segMan, object, SELECTOR(cel));
	itemEntry->x = readSelectorValue(_segMan, object, SELECTOR(x));
	itemEntry->y = readSelectorValue(_segMan, object, SELECTOR(y));
	itemEntry->z = readSelectorValue(_segMan, object, SELECTOR(z));
	itemEntry->priority = readSelectorValue(_segMan, object, SELECTOR(priority));
	if (readSelectorValue(_segMan, object, SELECTOR(fixPriority)) == 0)
		itemEntry->priority = itemEntry->y;

	itemEntry->signal = readSelectorValue(_segMan, object, SELECTOR(signal));
	itemEntry->scaleX = readSelectorValue(_segMan, object, SELECTOR(scaleX));
	itemEntry->scaleY = readSelectorValue(_segMan, object, SELECTOR(scaleY));
	itemEntry->visible = true;

	// Check if the entry can be hidden
	if (lookupSelector(_segMan, object, SELECTOR(visible), NULL, NULL) != kSelectorNone)
		itemEntry->visible = readSelectorValue(_segMan, object, SELECTOR(visible));
}

void GfxFrameout::kernelDeleteScreenItem(reg_t object) {
	FrameoutEntry *itemEntry = findScreenItem(object);
	// If the item could not be found, it may already have been deleted
	if (!itemEntry)
		return;

	_screenItems.remove(itemEntry);
	delete itemEntry;
}

void GfxFrameout::deletePlaneItems(reg_t planeObject) {
	FrameoutList::iterator listIterator = _screenItems.begin();

	while (listIterator != _screenItems.end()) {
		bool objectMatches = false;
		if (!planeObject.isNull()) {
			reg_t itemPlane = readSelector(_segMan, (*listIterator)->object, SELECTOR(plane));
			objectMatches = (planeObject == itemPlane);
		} else {
			objectMatches = true;
		}

		if (objectMatches) {
			FrameoutEntry *itemEntry = *listIterator;
			listIterator = _screenItems.erase(listIterator);
			delete itemEntry;
		} else {
			++listIterator;
		}
	}
}

FrameoutEntry *GfxFrameout::findScreenItem(reg_t object) {
	for (FrameoutList::iterator listIterator = _screenItems.begin(); listIterator != _screenItems.end(); listIterator++) {
		FrameoutEntry *itemEntry = *listIterator;
		if (itemEntry->object == object)
			return itemEntry;
	}

	return NULL;
}

int16 GfxFrameout::kernelGetHighPlanePri() {
	sortPlanes();
	return readSelectorValue(g_sci->getEngineState()->_segMan, _planes.back().object, SELECTOR(priority));
}

void GfxFrameout::kernelAddPicAt(reg_t planeObj, GuiResourceId pictureId, int16 pictureX, int16 pictureY) {
	addPlanePicture(planeObj, pictureId, pictureX, pictureY);
}

bool sortHelper(const FrameoutEntry* entry1, const FrameoutEntry* entry2) {
	if (entry1->priority == entry2->priority) {
		if (entry1->y == entry2->y)
			return (entry1->givenOrderNr < entry2->givenOrderNr);
		return (entry1->y < entry2->y);
	}
	return (entry1->priority < entry2->priority);
}

bool planeSortHelper(const PlaneEntry &entry1, const PlaneEntry &entry2) {
	if (entry1.priority < 0)
		return true;

	if (entry2.priority < 0)
		return false;

	return entry1.priority < entry2.priority;
}

void GfxFrameout::sortPlanes() {
	// First, remove any invalid planes
	for (PlaneList::iterator it = _planes.begin(); it != _planes.end();) {
		if (!_segMan->isObject(it->object))
			it = _planes.erase(it);
		else
			it++;
	}

	// Sort the rest of them
	Common::sort(_planes.begin(), _planes.end(), planeSortHelper);
}

void GfxFrameout::showVideo() {
	bool skipVideo = false;
	RobotDecoder *videoDecoder = g_sci->_robotDecoder;
	uint16 x = videoDecoder->getPos().x;
	uint16 y = videoDecoder->getPos().y;

	if (videoDecoder->hasDirtyPalette())
		g_system->getPaletteManager()->setPalette(videoDecoder->getPalette(), 0, 256);

	while (!g_engine->shouldQuit() && !videoDecoder->endOfVideo() && !skipVideo) {
		if (videoDecoder->needsUpdate()) {
			const Graphics::Surface *frame = videoDecoder->decodeNextFrame();
			if (frame) {
				g_system->copyRectToScreen(frame->pixels, frame->pitch, x, y, frame->w, frame->h);

				if (videoDecoder->hasDirtyPalette())
					g_system->getPaletteManager()->setPalette(videoDecoder->getPalette(), 0, 256);

				g_system->updateScreen();
			}
		}

		Common::Event event;
		while (g_system->getEventManager()->pollEvent(event)) {
			if ((event.type == Common::EVENT_KEYDOWN && event.kbd.keycode == Common::KEYCODE_ESCAPE) || event.type == Common::EVENT_LBUTTONUP)
				skipVideo = true;
		}

		g_system->delayMillis(10);
	}
}

void GfxFrameout::createPlaneItemList(reg_t planeObject, FrameoutList &itemList) {
	// Copy screen items of the current frame to the list of items to be drawn
	for (FrameoutList::iterator listIterator = _screenItems.begin(); listIterator != _screenItems.end(); listIterator++) {
		reg_t itemPlane = readSelector(_segMan, (*listIterator)->object, SELECTOR(plane));
		if (planeObject == itemPlane) {
			kernelUpdateScreenItem((*listIterator)->object);	// TODO: Why is this necessary?
			itemList.push_back(*listIterator);
		}
	}

	for (PlanePictureList::iterator pictureIt = _planePictures.begin(); pictureIt != _planePictures.end(); pictureIt++) {
		if (pictureIt->object == planeObject) {
			GfxPicture *planePicture = pictureIt->picture;
			// Allocate memory for picture cels
			pictureIt->pictureCels = new FrameoutEntry[planePicture->getSci32celCount()];

			// Add following cels to the itemlist
			FrameoutEntry *picEntry = pictureIt->pictureCels;
			int planePictureCels = planePicture->getSci32celCount();
			for (int pictureCelNr = 0; pictureCelNr < planePictureCels; pictureCelNr++) {
				picEntry->celNo = pictureCelNr;
				picEntry->object = NULL_REG;
				picEntry->picture = planePicture;
				picEntry->y = planePicture->getSci32celY(pictureCelNr);
				picEntry->x = planePicture->getSci32celX(pictureCelNr);
				picEntry->picStartX = pictureIt->startX;
				picEntry->picStartY = pictureIt->startY;
				picEntry->visible = true;

				picEntry->priority = planePicture->getSci32celPriority(pictureCelNr);

				itemList.push_back(picEntry);
				picEntry++;
			}
		}
	}

	// Now sort our itemlist
	Common::sort(itemList.begin(), itemList.end(), sortHelper);
}

bool GfxFrameout::isPictureOutOfView(FrameoutEntry *itemEntry, Common::Rect planeRect, int16 planeOffsetX, int16 planeOffsetY) {
	// Out of view horizontally (sanity checks)
	int16 pictureCelStartX = itemEntry->picStartX + itemEntry->x;
	int16 pictureCelEndX = pictureCelStartX + itemEntry->picture->getSci32celWidth(itemEntry->celNo);
	int16 planeStartX = planeOffsetX;
	int16 planeEndX = planeStartX + planeRect.width();
	if (pictureCelEndX < planeStartX)
		return true;
	if (pictureCelStartX > planeEndX)
		return true;

	// Out of view vertically (sanity checks)
	int16 pictureCelStartY = itemEntry->picStartY + itemEntry->y;
	int16 pictureCelEndY = pictureCelStartY + itemEntry->picture->getSci32celHeight(itemEntry->celNo);
	int16 planeStartY = planeOffsetY;
	int16 planeEndY = planeStartY + planeRect.height();
	if (pictureCelEndY < planeStartY)
		return true;
	if (pictureCelStartY > planeEndY)
		return true;

	return false;
}

void GfxFrameout::drawPicture(FrameoutEntry *itemEntry, int16 planeOffsetX, int16 planeOffsetY, bool planePictureMirrored) {
	int16 pictureOffsetX = planeOffsetX;
	int16 pictureX = itemEntry->x;
	if ((planeOffsetX) || (itemEntry->picStartX)) {
		if (planeOffsetX <= itemEntry->picStartX) {
			pictureX += itemEntry->picStartX - planeOffsetX;
			pictureOffsetX = 0;
		} else {
			pictureOffsetX = planeOffsetX - itemEntry->picStartX;
		}
	}

	int16 pictureOffsetY = planeOffsetY;
	int16 pictureY = itemEntry->y;
	if ((planeOffsetY) || (itemEntry->picStartY)) {
		if (planeOffsetY <= itemEntry->picStartY) {
			pictureY += itemEntry->picStartY - planeOffsetY;
			pictureOffsetY = 0;
		} else {
			pictureOffsetY = planeOffsetY - itemEntry->picStartY;
		}
	}

	itemEntry->picture->drawSci32Vga(itemEntry->celNo, pictureX, itemEntry->y, pictureOffsetX, pictureOffsetY, planePictureMirrored);
	//	warning("picture cel %d %d", itemEntry->celNo, itemEntry->priority);
}

void GfxFrameout::kernelFrameout() {
	if (g_sci->_robotDecoder->isVideoLoaded()) {
		showVideo();
		return;
	}

	_palette->palVaryUpdate();

	for (PlaneList::iterator it = _planes.begin(); it != _planes.end(); it++) {
		reg_t planeObject = it->object;

		// Draw any plane lines, if they exist
		// These are drawn on invisible planes as well. (e.g. "invisiblePlane" in LSL6 hires)
		// FIXME: Lines aren't always drawn (e.g. when the narrator speaks in LSL6 hires).
		// Perhaps something is painted over them?
		for (PlaneLineList::iterator it2 = it->lines.begin(); it2 != it->lines.end(); ++it2) {
			Common::Point startPoint = it2->startPoint;
			Common::Point endPoint = it2->endPoint;
			_coordAdjuster->kernelLocalToGlobal(startPoint.x, startPoint.y, it->object);
			_coordAdjuster->kernelLocalToGlobal(endPoint.x, endPoint.y, it->object);
			_screen->drawLine(startPoint, endPoint, it2->color, it2->priority, it2->control);
		}

		int16 planeLastPriority = it->lastPriority;

		// Update priority here, sq6 sets it w/o UpdatePlane
		int16 planePriority = it->priority = readSelectorValue(_segMan, planeObject, SELECTOR(priority));

		it->lastPriority = planePriority;
		if (planePriority < 0) { // Plane currently not meant to be shown
			// If plane was shown before, delete plane rect
			if (planePriority != planeLastPriority)
				_paint32->fillRect(it->planeRect, 0);
			continue;
		}

		// There is a race condition lurking in SQ6, which causes the game to hang in the intro, when teleporting to Polysorbate LX.
		// Since I first wrote the patch, the race has stopped occurring for me though.
		// I'll leave this for investigation later, when someone can reproduce.
		//if (it->pictureId == kPlanePlainColored)	// FIXME: This is what SSCI does, and fixes the intro of LSL7, but breaks the dialogs in GK1 (adds black boxes)
		if (it->pictureId == kPlanePlainColored && (it->planeBack || g_sci->getGameId() != GID_GK1))
			_paint32->fillRect(it->planeRect, it->planeBack);

		_coordAdjuster->pictureSetDisplayArea(it->planeRect);
		_palette->drewPicture(it->pictureId);

		FrameoutList itemList;

		createPlaneItemList(planeObject, itemList);

		for (FrameoutList::iterator listIterator = itemList.begin(); listIterator != itemList.end(); listIterator++) {
			FrameoutEntry *itemEntry = *listIterator;

			if (!itemEntry->visible)
				continue;

			if (itemEntry->object.isNull()) {
				// Picture cel data
				_coordAdjuster->fromScriptToDisplay(itemEntry->y, itemEntry->x);
				_coordAdjuster->fromScriptToDisplay(itemEntry->picStartY, itemEntry->picStartX);

				if (!isPictureOutOfView(itemEntry, it->planeRect, it->planeOffsetX, it->planeOffsetY))
					drawPicture(itemEntry, it->planeOffsetX, it->planeOffsetY, it->planePictureMirrored);
			} else {
				GfxView *view = (itemEntry->viewId != 0xFFFF) ? _cache->getView(itemEntry->viewId) : NULL;
				int16 dummyX = 0;

				if (view && view->isSci2Hires()) {
					view->adjustToUpscaledCoordinates(itemEntry->y, itemEntry->x);
					view->adjustToUpscaledCoordinates(itemEntry->z, dummyX);
				} else if (getSciVersion() >= SCI_VERSION_2_1) {
					_coordAdjuster->fromScriptToDisplay(itemEntry->y, itemEntry->x);
					_coordAdjuster->fromScriptToDisplay(itemEntry->z, dummyX);
				}

				// Adjust according to current scroll position
				itemEntry->x -= it->planeOffsetX;
				itemEntry->y -= it->planeOffsetY;

				uint16 useInsetRect = readSelectorValue(_segMan, itemEntry->object, SELECTOR(useInsetRect));
				if (useInsetRect) {
					itemEntry->celRect.top = readSelectorValue(_segMan, itemEntry->object, SELECTOR(inTop));
					itemEntry->celRect.left = readSelectorValue(_segMan, itemEntry->object, SELECTOR(inLeft));
					itemEntry->celRect.bottom = readSelectorValue(_segMan, itemEntry->object, SELECTOR(inBottom));
					itemEntry->celRect.right = readSelectorValue(_segMan, itemEntry->object, SELECTOR(inRight));
					if (view && view->isSci2Hires()) {
						view->adjustToUpscaledCoordinates(itemEntry->celRect.top, itemEntry->celRect.left);
						view->adjustToUpscaledCoordinates(itemEntry->celRect.bottom, itemEntry->celRect.right);
					}
					itemEntry->celRect.translate(itemEntry->x, itemEntry->y);
					// TODO: maybe we should clip the cels rect with this, i'm not sure
					//  the only currently known usage is game menu of gk1
				} else if (view) {
					if ((itemEntry->scaleX == 128) && (itemEntry->scaleY == 128))
						view->getCelRect(itemEntry->loopNo, itemEntry->celNo,
							itemEntry->x, itemEntry->y, itemEntry->z, itemEntry->celRect);
					else
						view->getCelScaledRect(itemEntry->loopNo, itemEntry->celNo,
							itemEntry->x, itemEntry->y, itemEntry->z, itemEntry->scaleX,
							itemEntry->scaleY, itemEntry->celRect);

					Common::Rect nsRect = itemEntry->celRect;
					// Translate back to actual coordinate within scrollable plane
					nsRect.translate(it->planeOffsetX, it->planeOffsetY);

					if (view && view->isSci2Hires()) {
						view->adjustBackUpscaledCoordinates(nsRect.top, nsRect.left);
						view->adjustBackUpscaledCoordinates(nsRect.bottom, nsRect.right);
					} else if (getSciVersion() >= SCI_VERSION_2_1) {
						_coordAdjuster->fromDisplayToScript(nsRect.top, nsRect.left);
						_coordAdjuster->fromDisplayToScript(nsRect.bottom, nsRect.right);
					}

					if (g_sci->getGameId() == GID_PHANTASMAGORIA2) {
						// HACK: Some (?) objects in Phantasmagoria 2 have no NS rect. Skip them for now.
						// TODO: Remove once we figure out how Phantasmagoria 2 draws objects on screen.
						if (lookupSelector(_segMan, itemEntry->object, SELECTOR(nsLeft), NULL, NULL) != kSelectorVariable)
							continue;
					}

					g_sci->_gfxCompare->setNSRect(itemEntry->object, nsRect);
				}

				// Don't attempt to draw sprites that are outside the visible
				// screen area. An example is the random people walking in
				// Jackson Square in GK1.
				if (itemEntry->celRect.bottom < 0 || itemEntry->celRect.top  >= _screen->getDisplayHeight() ||
				    itemEntry->celRect.right  < 0 || itemEntry->celRect.left >= _screen->getDisplayWidth())
					continue;

				Common::Rect clipRect, translatedClipRect;
				clipRect = itemEntry->celRect;

				if (view && view->isSci2Hires()) {
					clipRect.clip(it->upscaledPlaneClipRect);
					translatedClipRect = clipRect;
					translatedClipRect.translate(it->upscaledPlaneRect.left, it->upscaledPlaneRect.top);
				} else {
					// QFG4 passes invalid rectangles when a battle is starting
					if (!clipRect.isValidRect())
						continue;
					clipRect.clip(it->planeClipRect);
					translatedClipRect = clipRect;
					translatedClipRect.translate(it->planeRect.left, it->planeRect.top);
				}

				if (view) {
					if (!clipRect.isEmpty()) {
						if ((itemEntry->scaleX == 128) && (itemEntry->scaleY == 128))
							view->draw(itemEntry->celRect, clipRect, translatedClipRect,
								itemEntry->loopNo, itemEntry->celNo, 255, 0, view->isSci2Hires());
						else
							view->drawScaled(itemEntry->celRect, clipRect, translatedClipRect,
								itemEntry->loopNo, itemEntry->celNo, 255, itemEntry->scaleX, itemEntry->scaleY);
					}
				}

				// Draw text, if it exists
				if (lookupSelector(_segMan, itemEntry->object, SELECTOR(text), NULL, NULL) == kSelectorVariable) {
					g_sci->_gfxText32->drawTextBitmap(itemEntry->x, itemEntry->y, it->planeRect, itemEntry->object);
				}
			}
		}

		for (PlanePictureList::iterator pictureIt = _planePictures.begin(); pictureIt != _planePictures.end(); pictureIt++) {
			if (pictureIt->object == planeObject) {
				delete[] pictureIt->pictureCels;
				pictureIt->pictureCels = 0;
			}
		}
	}

	showCurrentScrollText();

	_screen->copyToScreen();

	g_sci->getEngineState()->_throttleTrigger = true;
}

void GfxFrameout::printPlaneList(Console *con) {
	for (PlaneList::const_iterator it = _planes.begin(); it != _planes.end(); ++it) {
		PlaneEntry p = *it;
		Common::String curPlaneName = _segMan->getObjectName(p.object);
		Common::Rect r = p.upscaledPlaneRect;
		Common::Rect cr = p.upscaledPlaneClipRect;

		con->DebugPrintf("%04x:%04x (%s): prio %d, lastprio %d, offsetX %d, offsetY %d, pic %d, mirror %d, back %d\n",
							PRINT_REG(p.object), curPlaneName.c_str(),
							(int16)p.priority, (int16)p.lastPriority,
							p.planeOffsetX, p.planeOffsetY, p.pictureId,
							p.planePictureMirrored, p.planeBack);
		con->DebugPrintf("  rect: (%d, %d, %d, %d), clip rect: (%d, %d, %d, %d)\n",
							r.left, r.top, r.right, r.bottom,
							cr.left, cr.top, cr.right, cr.bottom);

		if (p.pictureId != 0xffff && p.pictureId != 0xfffe) {
			con->DebugPrintf("Pictures:\n");

			for (PlanePictureList::iterator pictureIt = _planePictures.begin(); pictureIt != _planePictures.end(); pictureIt++) {
				if (pictureIt->object == p.object) {
					con->DebugPrintf("    Picture %d: x %d, y %d\n", pictureIt->pictureId, pictureIt->startX, pictureIt->startY);
				}
			}
		}
	}
}

void GfxFrameout::printPlaneItemList(Console *con, reg_t planeObject) {
	for (FrameoutList::iterator listIterator = _screenItems.begin(); listIterator != _screenItems.end(); listIterator++) {
		FrameoutEntry *e = *listIterator;
		reg_t itemPlane = readSelector(_segMan, e->object, SELECTOR(plane));

		if (planeObject == itemPlane) {
			Common::String curItemName = _segMan->getObjectName(e->object);
			Common::Rect icr = e->celRect;
			GuiResourceId picId = e->picture ? e->picture->getResourceId() : 0;

			con->DebugPrintf("%d: %04x:%04x (%s), view %d, loop %d, cel %d, x %d, y %d, z %d, "
							 "signal %d, scale signal %d, scaleX %d, scaleY %d, rect (%d, %d, %d, %d), "
							 "pic %d, picX %d, picY %d, visible %d\n",
							 e->givenOrderNr, PRINT_REG(e->object), curItemName.c_str(),
							 e->viewId, e->loopNo, e->celNo, e->x, e->y, e->z,
							 e->signal, e->scaleSignal, e->scaleX, e->scaleY,
							 icr.left, icr.top, icr.right, icr.bottom,
							 picId, e->picStartX, e->picStartY, e->visible);
		}
	}
}

} // End of namespace Sci
