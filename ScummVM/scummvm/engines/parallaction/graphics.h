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

#ifndef PARALLACTION_GRAPHICS_H
#define PARALLACTION_GRAPHICS_H

#include "common/list.h"
#include "common/rect.h"
#include "common/hashmap.h"
#include "common/hash-str.h"
#include "common/stream.h"
#include "common/array.h"

#include "graphics/surface.h"


namespace Parallaction {


#include "common/pack-start.h"	// START STRUCT PACKING

struct PaletteFxRange {

	uint16	_timer;
	uint16	_step;
	uint16	_flags;
	byte	_first;
	byte	_last;

} PACKED_STRUCT;

#include "common/pack-end.h"	// END STRUCT PACKING

class Font {

protected:
	byte _color;


public:
	Font() {}
	virtual ~Font() {}

	virtual void setColor(byte color) {
		_color = color;
	}
	virtual uint32 getStringWidth(const char *s) = 0;
	virtual uint16 height() = 0;

	virtual void drawString(byte* buffer, uint32 pitch, const char *s) = 0;


};


struct Frames {

	virtual uint16	getNum() = 0;
	virtual byte*	getData(uint16 index) = 0;
	virtual void	getRect(uint16 index, Common::Rect &r) = 0;
	virtual uint	getRawSize(uint16 index) = 0;
	virtual uint	getSize(uint16 index) = 0;

	virtual ~Frames() { }

};


struct SurfaceToFrames : public Frames {

	Graphics::Surface	*_surf;

public:
	SurfaceToFrames(Graphics::Surface *surf) : _surf(surf) {
	}

	~SurfaceToFrames() {
		_surf->free();
		delete _surf;
	}

	uint16	getNum() {
		return 1;
	}
	byte*	getData(uint16 index) {
		assert(index == 0);
		return (byte *)_surf->getBasePtr(0,0);
	}
	void	getRect(uint16 index, Common::Rect &r) {
		assert(index == 0);
		r.left = 0;
		r.top = 0;
		r.setWidth(_surf->w);
		r.setHeight(_surf->h);
	}
	uint	getRawSize(uint16 index) {
		assert(index == 0);
		return getSize(index);
	}
	uint	getSize(uint16 index) {
		assert(index == 0);
		return _surf->w * _surf->h;
	}

};

struct Cnv : public Frames {
	uint16	_count;		// # of frames
	uint16	_width;		//
	uint16	_height;	//
	byte**	field_8;	// unused
	byte*	_data;
	bool	_freeData;

public:
	Cnv() {
		_width = _height = _count = 0;
		_data = NULL;
	}

	Cnv(uint16 numFrames, uint16 width, uint16 height, byte* data, bool freeData = false)
		: _count(numFrames), _width(width), _height(height), _data(data), _freeData(freeData) {

	}

	~Cnv() {
		if (_freeData)
			delete[] _data;
	}

	byte* getFramePtr(uint16 index) {
		if (index >= _count)
			return NULL;
		return &_data[index * _width * _height];
	}

	uint16	getNum() {
		return _count;
	}

	byte	*getData(uint16 index) {
		return getFramePtr(index);
	}

	void getRect(uint16 index, Common::Rect &r) {
		r.left = 0;
		r.top = 0;
		r.setWidth(_width);
		r.setHeight(_height);
	}
	uint	getRawSize(uint16 index) {
		assert(index < _count);
		return getSize(index);
	}
	uint	getSize(uint16 index) {
		assert(index < _count);
		return _width * _height;
	}

};


struct MaskBuffer {
	// handles a 2-bit depth buffer used for z-buffering

	uint16	w;
	uint16  internalWidth;
	uint16	h;
	uint	size;
	byte	*data;
	bool	bigEndian;

	byte* getPtr(uint16 x, uint16 y) const;
	void bltOr(uint16 dx, uint16 dy, const MaskBuffer &src, uint16 sx, uint16 sy, uint width, uint height);
	void bltCopy(uint16 dx, uint16 dy, const MaskBuffer &src, uint16 sx, uint16 sy, uint width, uint height);

public:
	MaskBuffer();
	~MaskBuffer();

	void clone(const MaskBuffer &buf);
	void create(uint16 width, uint16 height);
	void free();

	byte getValue(uint16 x, uint16 y) const;
};


struct PathBuffer {
	// handles a 1-bit depth buffer used for masking non-walkable areas

	uint16	w;
	uint16  internalWidth;
	uint16	h;
	uint	size;
	byte	*data;
	bool	bigEndian;

	byte* getPtr(uint16 x, uint16 y) const;
	void bltCopy(uint16 dx, uint16 dy, const PathBuffer &src, uint16 sx, uint16 sy, uint width, uint height);

public:
	PathBuffer();
	~PathBuffer();

	void clone(const PathBuffer &buf);
	void create(uint16 width, uint16 height);
	void free();
	byte getValue(uint16 x, uint16 y) const;
};


class Palette {

	byte	_data[768];
	uint	_colors;
	uint	_size;
	bool	_hb;

public:
	Palette();
	Palette(const Palette &pal);

	void clone(const Palette &pal);

	void makeBlack();
	void setEntries(byte* data, uint first, uint num);
	void getEntry(uint index, int &red, int &green, int &blue);
	void setEntry(uint index, int red, int green, int blue);
	void makeGrayscale();
	void fadeTo(const Palette& target, uint step);
	uint fillRGB(byte *rgb);

	void rotate(uint first, uint last, bool forward);
};


#define CENTER_LABEL_HORIZONTAL	-1
#define CENTER_LABEL_VERTICAL	-1



#define MAX_BALLOON_WIDTH 130

class Parallaction;

struct DoorData;
struct GetData;
struct Label;
class Disk;

enum {
	kGfxObjVisible = 1,

	kGfxObjTypeDoor = 0,
	kGfxObjTypeGet = 1,
	kGfxObjTypeAnim = 2,
	kGfxObjTypeLabel = 3,
	kGfxObjTypeBalloon = 4,
	kGfxObjTypeCharacter = 8,
	kGfxObjTypeMenu = 16
};

enum {
	kGfxObjDoorZ = -200,
	kGfxObjGetZ = -100
};

class GfxObj {
	char *_name;
	Frames *_frames;

	bool _keep;

public:
	int16 x, y;

	int32 z;
	uint32 _prog;	// this value is used when sorting, in case that comparing z is not enough to tell which object goes on front

	uint32 _flags;

	uint type;
	uint frame;
	uint layer;
	uint transparentKey;
	uint scale;

	int	_maskId;
	bool _hasMask;
	int _pathId;
	bool _hasPath;


	GfxObj(uint type, Frames *frames, const char *name = NULL);
	virtual ~GfxObj();

	const char *getName() const;

	uint getNum();
	void getRect(uint frame, Common::Rect &r);
	byte *getData(uint frame);
	uint getRawSize(uint frame);
	uint getSize(uint frame);


	void setFlags(uint32 flags);
	void clearFlags(uint32 flags);
	bool isVisible() {
		return (_flags & kGfxObjVisible) == kGfxObjVisible;
	}

	void release();
};

#define LAYER_FOREGROUND   3

/*
	BackgroundInfo keeps information about the background bitmap that can be seen in the game.
	These bitmaps can be of any size, smaller or larger than the visible screen, the latter
	being the most common options.
*/
struct BackgroundInfo {
protected:
	typedef Common::Array<MaskBuffer *> MaskPatches;
	MaskPatches	_maskPatches;
	MaskBuffer		_maskBackup;
	void clearMaskData();

	typedef Common::Array<PathBuffer *> PathPatches;
	PathPatches	_pathPatches;
	PathBuffer		_pathBackup;
	void clearPathData();

public:
	int _x, _y;		// used to display bitmaps smaller than the screen
	int width;
	int height;

	Graphics::Surface	bg;
	MaskBuffer			*_mask;
	PathBuffer			*_path;

	Palette				palette;

	int				layers[4];
	PaletteFxRange		ranges[6];


	BackgroundInfo();
	~BackgroundInfo();

	void setPaletteRange(int index, const PaletteFxRange& range);

	// mask management
	bool hasMask();
	uint addMaskPatch(MaskBuffer *patch);
	void toggleMaskPatch(uint id, int x, int y, bool apply);
	uint16 getMaskLayer(uint16 z) const;
	void finalizeMask();
	void loadGfxObjMask(Parallaction *vm, const char *name, GfxObj *obj);

	// path management
	bool hasPath();
	uint addPathPatch(PathBuffer *patch);
	void togglePathPatch(uint id, int x, int y, bool apply);
	void finalizePath();
	void loadGfxObjPath(Parallaction *vm, const char *name, GfxObj *obj);
};




enum {
	kBackgroundLocation = 1,
	kBackgroundSlide = 2
};


class BalloonManager {
public:
	enum TextColor {
		kSelectedColor = 0,
		kUnselectedColor = 1,
		kNormalColor = 2
	};

	virtual ~BalloonManager() { }

	virtual void reset() = 0;
	virtual int setLocationBalloon(const Common::String &text, bool endGame) = 0;
	virtual int setDialogueBalloon(const Common::String &text, uint16 winding, TextColor textColor) = 0;
	virtual int setSingleBalloon(const Common::String &text, uint16 x, uint16 y, uint16 winding, TextColor textColor) = 0;
	virtual void setBalloonText(uint id, const Common::String &text, TextColor textColor) = 0;
	virtual int hitTestDialogueBalloon(int x, int y) = 0;
};


typedef Common::Array<GfxObj *> GfxObjArray;
#define SCENE_DRAWLIST_SIZE 100

class Gfx {

protected:
	Parallaction*		_vm;
	void resetSceneDrawList();

public:
	Disk *_disk;

	void beginFrame();
	void addObjectToScene(GfxObj *obj);
	GfxObjArray _sceneObjects;
	GfxObj* loadAnim(const char *name);
	GfxObj* loadGet(const char *name);
	GfxObj* loadDoor(const char *name);
	GfxObj* loadCharacterAnim(const char *name);
	void sortScene();
	void freeCharacterObjects();
	void freeLocationObjects();
	void showGfxObj(GfxObj* obj, bool visible);
	void blt(const Common::Rect& r, byte *data, Graphics::Surface *surf, uint16 z, uint scale, byte transparentColor);
	void unpackBlt(const Common::Rect& r, byte *data, uint size, Graphics::Surface *surf, uint16 z, uint scale, byte transparentColor);

	// labels
	void showFloatingLabel(GfxObj *label);
	void hideFloatingLabel();

	GfxObj *renderFloatingLabel(Font *font, char *text);
	GfxObj *createLabel(Font *font, const char *text, byte color);
	void showLabel(GfxObj *label, int16 x, int16 y);
	void hideLabel(GfxObj *label);
	void freeLabels();
	void unregisterLabel(GfxObj *label);

	// dialogue handling
	GfxObj* registerBalloon(Frames *frames, const char *text);
	int setItem(GfxObj* obj, uint16 x, uint16 y, byte transparentColor = 0);
	void setItemFrame(uint item, uint16 f);
	void freeDialogueObjects();

	// background surface
	BackgroundInfo	*_backgroundInfo;
	void setBackground(uint type, BackgroundInfo *info);
	void patchBackground(Graphics::Surface &surf, int16 x, int16 y, bool mask = false);
	void grabBackground(const Common::Rect& r, Graphics::Surface &dst);
	void fillBackground(const Common::Rect& r, byte color);
	void invertBackground(const Common::Rect& r);

	// palette
	void setPalette(Palette palette);
	void setBlackPalette();
	void animatePalette();

	// amiga specific
	void applyHalfbriteEffect_NS(Graphics::Surface &surf);
	void setHalfbriteMode(bool enable);
	void setProjectorPos(int x, int y);
	void setProjectorProgram(int16 *data);
	int16 *_nextProjectorPos;

	// start programmatic relative scroll
	void initiateScroll(int deltaX, int deltaY);
	// immediate and absolute x,y scroll
	void setScrollPosX(int scrollX);
	void setScrollPosY(int scrollY);
	// return current scroll position
	void getScrollPos(Common::Point &p);

	// init
	Gfx(Parallaction* vm);
	virtual ~Gfx();

	void clearScreen();
	void updateScreen();

public:
	Palette				_palette;

	byte				*_unpackedBitmap;

protected:
	bool				_halfbrite;

	Common::Point		_hbCirclePos;
	int				_hbCircleRadius;

	// BRA specific
	Palette				_backupPal;


	Graphics::Surface	*lockScreen();
	void				unlockScreen();
	void				updateScreenIntern();

	bool				_doubleBuffering;
	int					_gameType;
	Graphics::Surface	_backBuffer;
	void				copyRectToScreen(const byte *buf, int pitch, int x, int y, int w, int h);

	int					_scrollPosX, _scrollPosY;
	int					_minScrollX, _maxScrollX, _minScrollY, _maxScrollY;

	uint32				_requestedHScrollSteps;
	uint32				_requestedVScrollSteps;
	int32				_requestedHScrollDir;
	int32				_requestedVScrollDir;
	void				scroll();
	#define NO_FLOATING_LABEL	1000

	struct Label {
		Common::String _text;
		int  _x, _y;
		int color;
		bool _floating;
	};

	GfxObjArray	_labels;
	GfxObjArray _balloons;
	GfxObjArray	_items;

	GfxObj *_floatingLabel;

	// overlay mode enables drawing of graphics with automatic screen-to-game coordinate translation
	bool				_overlayMode;
	void				drawOverlay(Graphics::Surface &surf);
	void				drawInventory();

	void drawList(Graphics::Surface &surface, GfxObjArray &list);
	void updateFloatingLabel();
	void copyRect(const Common::Rect &r, Graphics::Surface &src, Graphics::Surface &dst);
	void drawText(Font *font, Graphics::Surface* surf, uint16 x, uint16 y, const char *text, byte color);
	void drawGfxObject(GfxObj *obj, Graphics::Surface &surf);
	void bltMaskScale(const Common::Rect& r, byte *data, Graphics::Surface *surf, uint16 z, uint scale, byte transparentColor);
	void bltMaskNoScale(const Common::Rect& r, byte *data, Graphics::Surface *surf, uint16 z, byte transparentColor);
	void bltNoMaskNoScale(const Common::Rect& r, byte *data, Graphics::Surface *surf, byte transparentColor);
};


} // Parallaction


#endif
