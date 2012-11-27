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

// Disable symbol overrides so that we can use system headers.
#define FORBIDDEN_SYMBOL_ALLOW_ALL

#include "common/scummsys.h"
#include "common/str.h"
#include "common/stream.h"
#include "common/archive.h"
#include "common/events.h"
#include "common/ptr.h"
#include "gui/message.h"
#include "engines/engine.h"
#include "backends/platform/psp/input.h"
#include "backends/platform/psp/display_manager.h"
#include "backends/platform/psp/display_client.h"
#include "backends/platform/psp/image_viewer.h"
#include "backends/platform/psp/png_loader.h"
#include "backends/platform/psp/thread.h"

static const char *imageName = "psp_image";
#define PSP_SCREEN_HEIGHT 272
#define PSP_SCREEN_WIDTH 480

bool ImageViewer::load(int imageNum) {
	if (_init)
		unload();

	// build string
	char number[8];
	sprintf(number, "%d", imageNum);
	Common::String imageNameStr(imageName);
	Common::String specificImageName = imageNameStr + Common::String(number) + Common::String(".png");

	// search for image file
	if (!SearchMan.hasFile(specificImageName)) {
		PSP_ERROR("file %s not found\n", specificImageName.c_str());
		return false;
	}

	Common::ScopedPtr<Common::SeekableReadStream> file(SearchMan.createReadStreamForMember(specificImageName));

	_buffer = new Buffer();
	_palette = new Palette();
	_renderer = new GuRenderer();

	assert(_buffer);
	assert(_palette);
	assert(_renderer);

	// Load a PNG into our buffer and palette. Size it by the actual size of the image
	PngLoader image(*file, *_buffer, *_palette, Buffer::kSizeBySourceSize);

	PngLoader::Status status = image.allocate();	// allocate the buffers for the file

	char error[100];
	if (status == PngLoader::BAD_FILE) {
		sprintf(error, "Cannot display %s. Not a proper PNG file", specificImageName.c_str());
		GUI::TimedMessageDialog dialog(error, 4000);
		dialog.runModal();
		return false;
	} else if (status == PngLoader::OUT_OF_MEMORY) {
		sprintf(error, "Out of memory loading %s. Try making the image smaller", specificImageName.c_str());
		GUI::TimedMessageDialog dialog(error, 4000);
		dialog.runModal();
		return false;
	}
	// try to load the image file
	if (!image.load()) {
		sprintf(error, "Cannot display %s. Not a proper PNG file", specificImageName.c_str());
		GUI::TimedMessageDialog dialog(error, 4000);
		dialog.runModal();
		return false;
	}

	setConstantRendererOptions();
	setFullScreenImageParams();		// prepare renderer for full screen view

	_imageNum = imageNum;			// now we can say we displayed this image
	_init = true;

	return true;
}

void ImageViewer::setConstantRendererOptions() {
	_renderer->setBuffer(_buffer);
	_renderer->setPalette(_palette);

	_renderer->setAlphaBlending(false);
	_renderer->setColorTest(false);
	_renderer->setUseGlobalScaler(false);
	_renderer->setStretch(true);
	_renderer->setOffsetInBuffer(0, 0);
	_renderer->setDrawWholeBuffer();
}

void ImageViewer::unload() {
	_init = false;
	delete _buffer;
	delete _palette;
	delete _renderer;
	_buffer = 0;
	_palette = 0;
	_renderer = 0;
}

void ImageViewer::resetOnEngineDone() {
	_imageNum = 0;
}

void ImageViewer::setVisible(bool visible) {
	DEBUG_ENTER_FUNC();

	if (_visible == visible)
		return;

	// from here on, we're making the loader visible
	if (visible && g_engine) {	// we can only run the image viewer when there's an engine
		g_engine->pauseEngine(true);

		load(_imageNum ? _imageNum : 1); 	// load the 1st image or the current
	}

	if (visible && _init) {	// we managed to load
		_visible = true;
		setViewerButtons(true);

		{ // so dialog goes out of scope, destroying all allocations
			GUI::TimedMessageDialog dialog("Image Viewer", 1000);
			dialog.runModal();
		}

		runLoop();	// only listen to viewer events
	} else {	// we were asked to make invisible or failed to load
		_visible = false;
		unload();
		setViewerButtons(false);

		if (g_engine && g_engine->isPaused())
			g_engine->pauseEngine(false);
	}
	setDirty();
}

// This is the only way we can truly pause the games
// Sad but true.
void ImageViewer::runLoop() {
	while (_visible) {
		Common::Event event;
		PspThread::delayMillis(30);
		_inputHandler->getAllInputs(event);
		_displayManager->renderAll();
	}
}

void ImageViewer::setViewerButtons(bool active) {
	_inputHandler->setImageViewerMode(active);
}

void ImageViewer::loadNextImage() {
	if (!load(_imageNum+1)) {		// try to load the next image
		if (!load(_imageNum))		// we failed, so reload the current image
			setVisible(false);		// just hide
	}
	setDirty();
}

void ImageViewer::loadLastImage() {
	if (_imageNum - 1 > 0) {
		if (!load(_imageNum-1))
			if (!load(_imageNum))
				setVisible(false);	// we can't even show the old image so hide
	}
	setDirty();
}

void ImageViewer::setFullScreenImageParams() {
	// we try to fit the image fullscreen at least in one dimension
	uint32 width = _buffer->getSourceWidth();
	uint32 height = _buffer->getSourceHeight();

	_centerX = PSP_SCREEN_WIDTH / 2.0f;
	_centerY = PSP_SCREEN_HEIGHT / 2.0f;

	// see if we fit width wise
	if (PSP_SCREEN_HEIGHT >= (int)((height * PSP_SCREEN_WIDTH) / (float)width)) {
		setZoom(PSP_SCREEN_WIDTH / (float)width);
	} else {
		setZoom(PSP_SCREEN_HEIGHT / (float)height);
	}
}

void ImageViewer::render() {
	if (_init) {
		assert(_buffer);
		assert(_renderer);

		// move the image slightly. Note that we count on the renderer's timing
		switch (_movement) {
		case EVENT_MOVE_LEFT:
			moveImageX(-_visibleWidth / 100.0f);
			break;
		case EVENT_MOVE_UP:
			moveImageY(-_visibleHeight / 100.0f);
			break;
		case EVENT_MOVE_RIGHT:
			moveImageX(_visibleWidth / 100.0f);
			break;
		case EVENT_MOVE_DOWN:
			moveImageY(_visibleHeight / 100.0f);
			break;
		default:
			break;
		}
		_renderer->render();
	}
}

void ImageViewer::modifyZoom(bool up) {
	float factor = _zoomFactor;
	if (up)
		factor += 0.1f;
	else // down
		factor -= 0.1f;

	setZoom(factor);
}

void ImageViewer::setZoom(float value) {
	if (value <= 0.0f)		// don't want 0 or negative zoom
		return;

	_zoomFactor = value;
	_renderer->setStretchXY(value, value);
	setOffsetParams();
}

void ImageViewer::moveImageX(float val) {
	float newVal = _centerX + val;

	if (newVal - (_visibleWidth / 2) > PSP_SCREEN_WIDTH - 4 || newVal + (_visibleWidth / 2) < 4)
		return;
	_centerX = newVal;
	setOffsetParams();
}

void ImageViewer::moveImageY(float val) {
	float newVal = _centerY + val;

	if (newVal - (_visibleHeight / 2) > PSP_SCREEN_HEIGHT - 4 || newVal + (_visibleHeight / 2) < 4)
		return;
	_centerY = newVal;
	setOffsetParams();
}

//	Set the renderer with the proper offset on the screen
//
void ImageViewer::setOffsetParams() {
	_visibleWidth = _zoomFactor * _buffer->getSourceWidth();
	_visibleHeight = _zoomFactor * _buffer->getSourceHeight();

	int offsetX = _centerX - (int)(_visibleWidth * 0.5f);
	int offsetY = _centerY - (int)(_visibleHeight * 0.5f);

	_renderer->setOffsetOnScreen(offsetX, offsetY);
	setDirty();
}

//	Handler events coming in from the inputHandler
//
void ImageViewer::handleEvent(uint32 event) {
	DEBUG_ENTER_FUNC();

	switch (event) {
	case EVENT_HIDE:
		setVisible(false);
		break;
	case EVENT_SHOW:
		setVisible(true);
		break;
	case EVENT_ZOOM_IN:
		modifyZoom(true);
		break;
	case EVENT_ZOOM_OUT:
		modifyZoom(false);
		break;
	case EVENT_MOVE_LEFT:
	case EVENT_MOVE_UP:
	case EVENT_MOVE_RIGHT:
	case EVENT_MOVE_DOWN:
	case EVENT_MOVE_STOP:
		_movement = (Event)event;
		break;
	case EVENT_NEXT_IMAGE:
		loadNextImage();
		break;
	case EVENT_LAST_IMAGE:
		loadLastImage();
		break;
	default:
		PSP_ERROR("Unknown event %d\n", event);
		break;
	}
}
