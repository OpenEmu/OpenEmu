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

#ifndef PSP_IMAGE_VIEWER_H
#define PSP_IMAGE_VIEWER_H

class InputHandler;

class ImageViewer : public DisplayClient {
public:
	enum Event {
		EVENT_NONE = -1,
		EVENT_HIDE = 0,
		EVENT_SHOW = 1,
		EVENT_ZOOM_IN,
		EVENT_ZOOM_OUT,
		EVENT_MOVE_LEFT,
		EVENT_MOVE_UP,
		EVENT_MOVE_RIGHT,
		EVENT_MOVE_DOWN,
		EVENT_MOVE_STOP,
		EVENT_NEXT_IMAGE,
		EVENT_LAST_IMAGE,
	};

private:
	Buffer *_buffer;
	Palette *_palette;
	GuRenderer *_renderer;
	bool _visible;
	bool _dirty;
	bool _init;
	uint32 _imageNum;	// current image number
	float _zoomFactor;	// how much we're zooming in/out on the image
	float _visibleHeight, _visibleWidth;
	float _centerX, _centerY;
	Event _movement;

	InputHandler *_inputHandler;
	DisplayManager *_displayManager;

	void setFullScreenImageParams();
	void loadNextImage();
	void loadLastImage();
	void setViewerButtons(bool active);
	void setConstantRendererOptions();
	void moveImageX(float val);
	void moveImageY(float val);
	bool load(int imageNum);
	void unload();
	void runLoop(); // to get total pausing we have to do our own loop

	void setZoom(float value);
	void setOffsetParams();
	void modifyZoom(bool up);	// up or down
	void setVisible(bool visible);

public:

	ImageViewer() : _buffer(0), _palette(0), _visible(false),
					_dirty(false), _init(false), _imageNum(0),
					_zoomFactor(0.0f), _visibleHeight(0.0f), _visibleWidth(0.0f),
					_centerX(0.0f), _centerY(0.0f), _movement(EVENT_MOVE_STOP),
					_inputHandler(0), _displayManager(0) {}
	~ImageViewer() { unload(); }	// deallocate images
	bool load();
	void render();
	bool isVisible() { return _visible; }
	bool isDirty() { return _dirty; }
	void setDirty() { _dirty = true; }
	void setClean() { if (!_visible) // otherwise we want to keep rendering
							_dirty = false;
	}
	void resetOnEngineDone();

	void handleEvent(uint32 event);

	// pointer setters
	void setInputHandler(InputHandler *inputHandler) { _inputHandler = inputHandler; }
	void setDisplayManager(DisplayManager *displayManager) { _displayManager = displayManager; }
};

#endif /* PSP_IMAGE_VIEWER_H */
