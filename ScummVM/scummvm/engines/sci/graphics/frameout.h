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

#ifndef SCI_GRAPHICS_FRAMEOUT_H
#define SCI_GRAPHICS_FRAMEOUT_H

namespace Sci {

class GfxPicture;

struct PlaneLineEntry {
	reg_t hunkId;
	Common::Point startPoint;
	Common::Point endPoint;
	byte color;
	byte priority;
	byte control;
};

typedef Common::List<PlaneLineEntry> PlaneLineList;

struct PlaneEntry {
	reg_t object;
	int16 priority;
	int16 lastPriority;
	int16 planeOffsetX;
	int16 planeOffsetY;
	GuiResourceId pictureId;
	Common::Rect planeRect;
	Common::Rect planeClipRect;
	Common::Rect upscaledPlaneRect;
	Common::Rect upscaledPlaneClipRect;
	bool planePictureMirrored;
	byte planeBack;
	PlaneLineList lines;
};

typedef Common::List<PlaneEntry> PlaneList;

struct FrameoutEntry {
	uint16 givenOrderNr;
	reg_t object;
	GuiResourceId viewId;
	int16 loopNo;
	int16 celNo;
	int16 x, y, z;
	int16 priority;
	uint16 signal;
	uint16 scaleSignal;
	int16 scaleX;
	int16 scaleY;
	Common::Rect celRect;
	GfxPicture *picture;
	int16 picStartX;
	int16 picStartY;
	bool visible;
};

typedef Common::List<FrameoutEntry *> FrameoutList;

struct PlanePictureEntry {
	reg_t object;
	int16 startX;
	int16 startY;
	GuiResourceId pictureId;
	GfxPicture *picture;
	FrameoutEntry *pictureCels; // temporary
};

typedef Common::List<PlanePictureEntry> PlanePictureList;

struct ScrollTextEntry {
	reg_t bitmapHandle;
	reg_t kWindow;
	uint16 x;
	uint16 y;
};

typedef Common::Array<ScrollTextEntry> ScrollTextList;

class GfxCache;
class GfxCoordAdjuster32;
class GfxPaint32;
class GfxPalette;
class GfxScreen;
/**
 * Frameout class, kFrameout and relevant functions for SCI32 games
 */
class GfxFrameout {
public:
	GfxFrameout(SegManager *segMan, ResourceManager *resMan, GfxCoordAdjuster *coordAdjuster, GfxCache *cache, GfxScreen *screen, GfxPalette *palette, GfxPaint32 *paint32);
	~GfxFrameout();

	void kernelAddPlane(reg_t object);
	void kernelUpdatePlane(reg_t object);
	void kernelDeletePlane(reg_t object);
	void kernelAddScreenItem(reg_t object);
	void kernelUpdateScreenItem(reg_t object);
	void kernelDeleteScreenItem(reg_t object);
	void deletePlaneItems(reg_t planeObject);
	FrameoutEntry *findScreenItem(reg_t object);
	int16 kernelGetHighPlanePri();
	void kernelAddPicAt(reg_t planeObj, GuiResourceId pictureId, int16 pictureX, int16 pictureY);
	void kernelFrameout();

	void addPlanePicture(reg_t object, GuiResourceId pictureId, uint16 startX, uint16 startY = 0);
	void deletePlanePictures(reg_t object);
	reg_t addPlaneLine(reg_t object, Common::Point startPoint, Common::Point endPoint, byte color, byte priority, byte control);
	void updatePlaneLine(reg_t object, reg_t hunkId, Common::Point startPoint, Common::Point endPoint, byte color, byte priority, byte control);
	void deletePlaneLine(reg_t object, reg_t hunkId);
	void clear();

	// Scroll text functions
	void addScrollTextEntry(Common::String &text, reg_t kWindow, uint16 x, uint16 y, bool replace);
	void showCurrentScrollText();
	void initScrollText(uint16 maxItems) { _maxScrollTexts = maxItems; }
	void clearScrollTexts();
	void firstScrollText() { if (_scrollTexts.size() > 0) _curScrollText = 0; }
	void lastScrollText() { if (_scrollTexts.size() > 0) _curScrollText = _scrollTexts.size() - 1; }
	void prevScrollText() { if (_curScrollText > 0) _curScrollText--; }
	void nextScrollText() { if (_curScrollText + 1 < (uint16)_scrollTexts.size()) _curScrollText++; }
	void toggleScrollText(bool show) { _showScrollText = show; }

	void printPlaneList(Console *con);
	void printPlaneItemList(Console *con, reg_t planeObject);

private:
	void showVideo();
	void createPlaneItemList(reg_t planeObject, FrameoutList &itemList);
	bool isPictureOutOfView(FrameoutEntry *itemEntry, Common::Rect planeRect, int16 planeOffsetX, int16 planeOffsetY);
	void drawPicture(FrameoutEntry *itemEntry, int16 planeOffsetX, int16 planeOffsetY, bool planePictureMirrored);

	SegManager *_segMan;
	ResourceManager *_resMan;
	GfxCoordAdjuster32 *_coordAdjuster;
	GfxCache *_cache;
	GfxPalette *_palette;
	GfxScreen *_screen;
	GfxPaint32 *_paint32;

	FrameoutList _screenItems;
	PlaneList _planes;
	PlanePictureList _planePictures;
	ScrollTextList _scrollTexts;
	int16 _curScrollText;
	bool _showScrollText;
	uint16 _maxScrollTexts;

	void sortPlanes();
};

} // End of namespace Sci

#endif
