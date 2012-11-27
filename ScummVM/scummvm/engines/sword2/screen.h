/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * Additional copyright for this file:
 * Copyright (C) 1994-1998 Revolution Software Ltd.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */

#ifndef	SWORD2_SCREEN_H
#define	SWORD2_SCREEN_H

#include "common/rect.h"
#include "common/stream.h"

#define MAX_bgp0_sprites 6
#define MAX_bgp1_sprites 6
#define MAX_back_sprites 30
#define MAX_sort_sprites 30
#define MAX_fore_sprites 30
#define MAX_fgp0_sprites 6
#define MAX_fgp1_sprites 6

#define PALTABLESIZE     (64 * 64 * 64)

#define BLOCKWIDTH       64
#define BLOCKHEIGHT      64
#define MAXLAYERS        5

#define MENUDEEP         40
#define RENDERWIDE       640
#define RENDERDEEP       (480 - (MENUDEEP * 2))

// Maximum scaled size of a sprite
#define SCALE_MAXWIDTH   512
#define SCALE_MAXHEIGHT  512

// Dirty grid cell size
#define CELLWIDE         10
#define CELLDEEP         20

namespace Sword2 {

class Sword2Engine;

// Sprite defines

enum {
	// This is the low byte part of the sprite type.

	RDSPR_TRANS			= 0x0001,
	RDSPR_BLEND			= 0x0004,
	RDSPR_FLIP			= 0x0008,
	RDSPR_SHADOW			= 0x0010,
	RDSPR_DISPLAYALIGN		= 0x0020,
	RDSPR_NOCOMPRESSION		= 0x0040,
	RDSPR_EDGEBLEND			= 0x0080,	// Unused

	// This is the high byte part of the sprite type, which defines what
	// type of compression is used. Unless RDSPR_NOCOMPRESSION is set.

	RDSPR_RLE16			= 0x0000,
	RDSPR_RLE256			= 0x0100,
	RDSPR_RLE256FAST		= 0x0200
};

// Fading defines

enum {
	RDFADE_NONE,
	RDFADE_UP,
	RDFADE_DOWN,
	RDFADE_BLACK
};

// Palette defines

enum {
	RDPAL_FADE,
	RDPAL_INSTANT
};

// Blitting FX defines

enum {
	RDBLTFX_SPRITEBLEND		= 0x01,
	RDBLTFX_SHADOWBLEND		= 0x02,
	RDBLTFX_EDGEBLEND		= 0x04
};

// Structure filled out by each object to register its graphic printing
// requrements

struct BuildUnit {
	int16 x;
	int16 y;
	uint16 scaled_width;
	uint16 scaled_height;
	int16 sort_y;
	uint32 anim_resource;
	uint16 anim_pc;

	// Denotes a scaling sprite at print time - and holds the scaling value
	// for the shrink routine

	uint16 scale;

	// Non-zero means this item is a layer - retrieve from background layer
	// and send to special renderer

	uint16 layer_number;

	// True means we want this frame to be affected by the shading mask

	bool shadingFlag;
};

struct ScreenInfo {
	uint16 scroll_offset_x;		// Position x
	uint16 scroll_offset_y;		// Position y
	uint16 max_scroll_offset_x;	// Calc'ed in fnInitBackground
	uint16 max_scroll_offset_y;
	int16 player_feet_x;		// Feet coordinates to use - cant just
	int16 player_feet_y;		// fetch the player compact anymore
	int16 feet_x;			// Special offset-to-player position -
	int16 feet_y;			// tweek as desired - always set in
					// screen manager object startup
	uint16 screen_wide;		// Size of background layer - hence
	uint16 screen_deep;		// size of back buffer itself (Paul
					// actually malloc's it)
	uint32 background_layer_id;	// Id of the normal background layer
					// from the header of the main
					// background layer
	uint16 number_of_layers;
	uint8 new_palette;		// Set to non zero to start the
					// palette held within layer file
					// fading up after a buildDisplay()
	uint8 scroll_flag;		// Scroll mode 0 off 1 on
	bool mask_flag;			// Using shading mask
};

// The SpriteInfo structure is used to tell the driver96 code what attributes
// are linked to a sprite for drawing.  These include position, scaling and
// compression.

struct SpriteInfo {
	int16 x;		// coords for top-left of sprite
	int16 y;
	uint16 w;		// dimensions of sprite (before scaling)
	uint16 h;
	uint16 scale;		// scale at which to draw, given in 256ths ['0' or '256' MEANS DON'T SCALE]
	uint16 scaledWidth;	// new dimensions (we calc these for the mouse area, so may as well pass to you to save time)
	uint16 scaledHeight;	//
	uint16 type;		// mask containing 'RDSPR_' bits specifying compression type, flip, transparency, etc
	uint16 blend;		// holds the blending values.
	byte *data;		// pointer to the sprite data
	byte *colorTable;	// pointer to 16-byte color table, only applicable to 16-col compression type
	bool isText;		// It is a engine-generated sprite containing text
};

struct BlockSurface {
	byte data[BLOCKWIDTH * BLOCKHEIGHT];
	bool transparent;
};

struct Parallax {
	uint16 w;
	uint16 h;

	// The dimensions are followed by an offset table, but we don't know in
	// advance how big it is. See initializeBackgroundLayer().

	static int size() {
		return 4;
	}

	void read(byte *addr);
	void write(byte *addr);
};

class Screen {
private:
	Sword2Engine *_vm;

	// _thisScreen describes the current back buffer and its in-game scroll
	// positions, etc.

	ScreenInfo _thisScreen;

	int32 _renderCaps;
	int8 _renderLevel;

	byte *_buffer;
	byte *_lightMask;

	// Game screen metrics
	int16 _screenWide;
	int16 _screenDeep;

	bool _needFullRedraw;

	// Scroll variables.  _scrollX and _scrollY hold the current scroll
	// position, and _scrollXTarget and _scrollYTarget are the target
	// position for the end of the game cycle.

	int16 _scrollX;
	int16 _scrollY;

	int16 _scrollXTarget;
	int16 _scrollYTarget;
	int16 _scrollXOld;
	int16 _scrollYOld;

	int16 _parallaxScrollX;	// current x offset to link a sprite to the
				// parallax layer
	int16 _parallaxScrollY;	// current y offset to link a sprite to the
				// parallax layer
	int16 _locationWide;
	int16 _locationDeep;

	// Dirty grid handling
	byte *_dirtyGrid;

	uint16 _gridWide;
	uint16 _gridDeep;

	byte _palette[256 * 3];
	byte _paletteMatch[PALTABLESIZE];

	uint8 _fadeStatus;
	int32 _fadeStartTime;
	int32 _fadeTotalTime;

	// 'frames per second' counting stuff
	uint32 _fps;
	uint32 _cycleTime;
	uint32 _frameCount;

	int32 _initialTime;
	int32 _startTime;
	int32 _totalTime;
	int32 _renderAverageTime;
	int32 _framesPerGameCycle;
	bool _renderTooSlow;

	void startNewPalette();

	void resetRenderEngine();

	void startRenderCycle();
	bool endRenderCycle();

	// Holds the order of the sort list, i.e. the list stays static and we
	// sort this array.

	uint16 _sortOrder[MAX_sort_sprites];

	BuildUnit _bgp0List[MAX_bgp0_sprites];
	BuildUnit _bgp1List[MAX_bgp1_sprites];
	BuildUnit _backList[MAX_back_sprites];
	BuildUnit _sortList[MAX_sort_sprites];
	BuildUnit _foreList[MAX_fore_sprites];
	BuildUnit _fgp0List[MAX_fgp0_sprites];
	BuildUnit _fgp1List[MAX_fgp1_sprites];

	uint32 _curBgp0;
	uint32 _curBgp1;
	uint32 _curBack;
	uint32 _curSort;
	uint32 _curFore;
	uint32 _curFgp0;
	uint32 _curFgp1;

	void drawBackPar0Frames();
	void drawBackPar1Frames();
	void drawBackFrames();
	void drawSortFrames(byte *file);
	void drawForeFrames();
	void drawForePar0Frames();
	void drawForePar1Frames();

	void processLayer(byte *file, uint32 layer_number);
	void processImage(BuildUnit *build_unit);

	uint8 _scrollFraction;

	// Last palette used - so that we can restore the correct one after a
	// pause (which dims the screen) and it's not always the main screen
	// palette that we want, eg. during the eclipse

	// This flag gets set in startNewPalette() and setFullPalette()

	uint32 _lastPaletteRes;

	// Debugging stuff
	uint32 _largestLayerArea;
	uint32 _largestSpriteArea;
	char _largestLayerInfo[128];
	char _largestSpriteInfo[128];

	void registerFrame(byte *ob_mouse, byte *ob_graph, byte *ob_mega, BuildUnit *build_unit);

	void mirrorSprite(byte *dst, byte *src, int16 w, int16 h);
	int32 decompressRLE256(byte *dst, byte *src, int32 decompSize);
	void unwindRaw16(byte *dst, byte *src, uint16 blockSize, byte *colTable);
	int32 decompressRLE16(byte *dst, byte *src, int32 decompSize, byte *colTable);
	void renderParallax(byte *ptr, int16 layer);


	void markAsDirty(int16 x0, int16 y0, int16 x1, int16 y1);

	uint8 _xBlocks[MAXLAYERS];
	uint8 _yBlocks[MAXLAYERS];

	// This is used to cache PSX backgrounds and parallaxes
	// data, as they are kept in a file unmanageable from
	// resource manager. These gets freed everytime an user
	// exits from a room.
	byte *_psxScrCache[3];
	bool _psxCacheEnabled[3];

	// An array of sub-blocks, one for each of the parallax layers.

	BlockSurface **_blockSurfaces[MAXLAYERS];

	uint16 _xScale[SCALE_MAXWIDTH];
	uint16 _yScale[SCALE_MAXHEIGHT];

	void blitBlockSurface(BlockSurface *s, Common::Rect *r, Common::Rect *clipRect);

	uint16 _layer;

	bool _dimPalette;

	uint32 _pauseTicks;
	uint32 _pauseStartTick;

	uint32 getTick();

public:
	Screen(Sword2Engine *vm, int16 width, int16 height);
	~Screen();

	void pauseScreen(bool pause);

	int8 getRenderLevel();
	void setRenderLevel(int8 level);

	byte *getScreen() { return _buffer; }
	byte *getPalette() { return _palette; }
	ScreenInfo *getScreenInfo() { return &_thisScreen; }

	int16 getScreenWide() { return _screenWide; }
	int16 getScreenDeep() { return _screenDeep; }

	uint32 getCurBgp0() { return _curBgp0; }
	uint32 getCurBgp1() { return _curBgp1; }
	uint32 getCurBack() { return _curBack; }
	uint32 getCurSort() { return _curSort; }
	uint32 getCurFore() { return _curFore; }
	uint32 getCurFgp0() { return _curFgp0; }
	uint32 getCurFgp1() { return _curFgp1; }

	uint32 getFps() { return _fps; }

	uint32 getLargestLayerArea() { return _largestLayerArea; }
	uint32 getLargestSpriteArea() { return _largestSpriteArea; }
	char *getLargestLayerInfo() { return _largestLayerInfo; }
	char *getLargestSpriteInfo() { return _largestSpriteInfo; }

	void setNeedFullRedraw();

	void clearScene();

	void resetRenderLists();

	void setLocationMetrics(uint16 w, uint16 h);
	int32 initializeBackgroundLayer(byte *parallax);
	int32 initializePsxParallaxLayer(byte *parallax);   // These are used to initialize psx backgrounds and
	int32 initializePsxBackgroundLayer(byte *parallax); // parallaxes, which are different from pc counterparts.
	void closeBackgroundLayer();

	void initializeRenderCycle();

	void initBackground(int32 res, int32 new_palette);
	void initPsxBackground(int32 res, int32 new_palette);
	void registerFrame(byte *ob_mouse, byte *ob_graph, byte *ob_mega);

	void setScrollFraction(uint8 f) { _scrollFraction = f; }
	void setScrollTarget(int16 x, int16 y);
	void setScrolling();

	void setFullPalette(int32 palRes);
	void setPalette(int16 startEntry, int16 noEntries, byte *palette, uint8 setNow);
	void setSystemPalette(const byte *colors, uint start, uint num);
	uint8 quickMatch(uint8 r, uint8 g, uint8 b);
	int32 fadeUp(float time = 0.75f);
	int32 fadeDown(float time = 0.75f);
	uint8 getFadeStatus();
	void dimPalette(bool dim);
	void waitForFade();
	void fadeServer();

	void updateDisplay(bool redrawScene = true);

	void displayMsg(byte *text, int time);

	int32 createSurface(SpriteInfo *s, byte **surface);
	void drawSurface(SpriteInfo *s, byte *surface, Common::Rect *clipRect = NULL);
	void deleteSurface(byte *surface);
	int32 drawSprite(SpriteInfo *s);

	void scaleImageFast(byte *dst, uint16 dstPitch, uint16 dstWidth,
		uint16 dstHeight, byte *src, uint16 srcPitch, uint16 srcWidth,
		uint16 srcHeight);
	void scaleImageGood(byte *dst, uint16 dstPitch, uint16 dstWidth,
		uint16 dstHeight, byte *src, uint16 srcPitch, uint16 srcWidth,
		uint16 srcHeight, byte *backBuf, int16 bbXPos, int16 bbYPos);

	void updateRect(Common::Rect *r);

	int32 openLightMask(SpriteInfo *s);
	int32 closeLightMask();

	void buildDisplay();

	void plotPoint(int x, int y, uint8 color);
	void drawLine(int x0, int y0, int x1, int y1, uint8 color);

	void rollCredits();
	void splashScreen();

	// Some sprites are compressed in HIF format
	static uint32 decompressHIF(byte *src, byte *dst, uint32 *skipData = NULL);
	// This is used to resize psx sprites back to original resolution
	static void resizePsxSprite(byte *dst, byte *src, uint16 destW, uint16 destH);
	// Some sprites are divided into 254 pixel wide stripes, this recomposes them
	// and generates a "normal" sprite.
	static void recomposePsxSprite(SpriteInfo *s);
	static void recomposeCompPsxSprite(SpriteInfo *s);

	// These functions manage the PSX screen cache
	void setPsxScrCache(byte *psxScrCache, uint8 level);
	byte *getPsxScrCache(uint8 level);
	bool getPsxScrCacheStatus(uint8 level);
	void flushPsxScrCache();

};

} // End of namespace Sword2

#endif
