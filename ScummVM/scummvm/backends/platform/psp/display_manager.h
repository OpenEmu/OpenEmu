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

#ifndef PSP_DISPLAY_MAN_H
#define PSP_DISPLAY_MAN_H

#include "backends/platform/psp/thread.h"
#include "common/list.h"

#define UNCACHED(x)		((byte *)(((uint32)(x)) | 0x40000000))	/* make an uncached access */
#define CACHED(x)		((byte *)(((uint32)(x)) & 0xBFFFFFFF))	/* make an uncached access into a cached one */

/**
 *	Class that allocates memory in the VRAM
 */
class VramAllocator : public Common::Singleton<VramAllocator> {
public:
	VramAllocator() : _bytesAllocated(0) {}
	void *allocate(int32 size, bool smallAllocation = false);	// smallAllocation e.g. palettes
	void deallocate(void *pointer);

	static inline bool isAddressInVram(void *address) {
		if ((uint32)(CACHED(address)) >= VRAM_START_ADDRESS && (uint32)(CACHED(address)) < VRAM_END_ADDRESS)
			return true;
		return false;
	}


private:
	/**
	 *	Used to allocate in VRAM
	 */
	struct Allocation {
		byte *address;
		uint32 size;
		void *getEnd() { return address + size; }
		Allocation(void *Address, uint32 Size) : address((byte *)Address), size(Size) {}
		Allocation() : address(0), size(0) {}
	};

	enum {
		VRAM_START_ADDRESS = 0x04000000,
		VRAM_END_ADDRESS   = 0x04200000,
		VRAM_SMALL_ADDRESS = VRAM_END_ADDRESS - (4 * 1024)	// 4K in the end for small allocations
	};
	Common::List <Allocation> _allocList;		// List of allocations
	uint32 _bytesAllocated;
};


/**
 *	Class used only by DisplayManager to start/stop GU rendering
 */
class MasterGuRenderer : public PspThreadable {
public:
	MasterGuRenderer() : _lastRenderTime(0), _renderFinished(true),
		_renderSema(1, 1), _callbackId(-1) {}
	void guInit();
	void guPreRender();
	void guPostRender();
	void guShutDown();
	bool isRenderFinished() { return _renderFinished; }
	void sleepUntilRenderFinished();
	void setupCallbackThread();
private:
	virtual void threadFunction();			// for the display callback thread
	static uint32 _displayList[];
	uint32 _lastRenderTime;					// For measuring rendering time
	void guProgramDisplayBufferSizes();
	static int guCallback(int, int, void *__this);	// for the display callback
	bool _renderFinished;					// for sync with render callback
	PspSemaphore _renderSema;				// semaphore for syncing
	int _callbackId;						// to keep track of render callback
};

class Screen;
class Overlay;
class Cursor;
class PSPKeyboard;
class ImageViewer;

/**
 *	Class that manages all display clients
 */
class DisplayManager {
public:
	enum GraphicsModeID {			///> Possible output formats onscreen
		ORIGINAL_RESOLUTION,
		KEEP_ASPECT_RATIO,
		STRETCHED_FULL_SCREEN
	};
	DisplayManager() : _screen(0), _cursor(0), _overlay(0), _keyboard(0),
					  _imageViewer(0), _lastUpdateTime(0), _graphicsMode(0) {}
	~DisplayManager();

	void init();
	bool renderAll();	// return true if rendered or nothing dirty. False otherwise
	void waitUntilRenderFinished();
	bool setGraphicsMode(int mode);
	bool setGraphicsMode(const char *name);
	int getGraphicsMode() const { return _graphicsMode; }
	uint32 getDefaultGraphicsMode() const { return STRETCHED_FULL_SCREEN; }
	const OSystem::GraphicsMode* getSupportedGraphicsModes() const { return _supportedModes; }

	// Setters for pointers
	void setScreen(Screen *screen) { _screen = screen; }
	void setCursor(Cursor *cursor) { _cursor = cursor; }
	void setOverlay(Overlay *overlay) { _overlay = overlay; }
	void setKeyboard(PSPKeyboard *keyboard) { _keyboard = keyboard; }
	void setImageViewer(ImageViewer *imageViewer) { _imageViewer = imageViewer; }

	void setSizeAndPixelFormat(uint width, uint height, const Graphics::PixelFormat *format);

	// Getters
	float getScaleX() const { return _displayParams.scaleX; }
	float getScaleY() const { return _displayParams.scaleY; }
	uint32 getOutputWidth() const { return _displayParams.screenOutput.width; }
	uint32 getOutputHeight() const { return _displayParams.screenOutput.height; }
	uint32 getOutputBitsPerPixel() const { return _displayParams.outputBitsPerPixel; }
	Common::List<Graphics::PixelFormat> getSupportedPixelFormats() const;

private:
	struct GlobalDisplayParams {
		Dimensions screenOutput;
		Dimensions screenSource;
		float scaleX;
		float scaleY;
		uint32 outputBitsPerPixel;		// How many bits end up on-screen
		GlobalDisplayParams() : scaleX(0.0f), scaleY(0.0f), outputBitsPerPixel(0) {}
	};

	// Pointers to DisplayClients
	Screen *_screen;
	Cursor *_cursor;
	Overlay *_overlay;
	PSPKeyboard *_keyboard;
	ImageViewer *_imageViewer;

	MasterGuRenderer _masterGuRenderer;
	uint32 _lastUpdateTime;					// For limiting FPS
	int _graphicsMode;
	GlobalDisplayParams _displayParams;
	static const OSystem::GraphicsMode _supportedModes[];

	void calculateScaleParams();	// calculates scaling factor
	bool isTimeToUpdate();			// should we update the screen now
};


#endif /* PSP_DISPLAY_MAN_H */
