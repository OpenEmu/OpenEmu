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

/*
 * This file is based on WME Lite.
 * http://dead-code.org/redir.php?target=wmelite
 * Copyright (c) 2011 Jan Nedoma
 */

#include "engines/wintermute/base/base_active_rect.h"
#include "engines/wintermute/base/gfx/base_renderer.h"
#include "engines/wintermute/base/gfx/base_surface.h"
#include "engines/wintermute/base/base_sub_frame.h"
#include "engines/wintermute/base/base_region.h"
#include "engines/wintermute/platform_osystem.h"
#include "engines/wintermute/base/base_persistence_manager.h"

namespace Wintermute {

//////////////////////////////////////////////////////////////////////
BaseRenderer::BaseRenderer(BaseGame *inGame) : BaseClass(inGame) {
	_window = 0;
	_clipperWindow = 0;
	_active = false;
	_ready = false;
	_windowed = true;
	_forceAlphaColor = 0x00;

	_indicatorDisplay = false;
	_indicatorColor = BYTETORGBA(255, 0, 0, 128);
	_indicatorProgress = 0;
	_indicatorX = -1;
	_indicatorY = -1;
	_indicatorWidth = -1;
	_indicatorHeight = 8;
	_indicatorWidthDrawn = 0;

	_loadImageName = "";
	_saveImageName = "";
	_saveLoadImage = NULL;
	_loadInProgress = false;
	_hasDrawnSaveLoadImage = false;

	_saveImageX = _saveImageY = 0;
	_loadImageX = _loadImageY = 0;

	_width = _height = _bPP = 0;
	BasePlatform::setRectEmpty(&_monitorRect);

	_realWidth = _realHeight = 0;
	_drawOffsetX = _drawOffsetY = 0;
}


//////////////////////////////////////////////////////////////////////
BaseRenderer::~BaseRenderer() {
	deleteRectList();
	unclipCursor();
	delete _saveLoadImage;
}


//////////////////////////////////////////////////////////////////////
void BaseRenderer::initLoop() {
	deleteRectList();
}

void BaseRenderer::initIndicator() {
	if (_indicatorY == -1) {
		_indicatorY = _height - _indicatorHeight;
	}
	if (_indicatorX == -1) {
		_indicatorX = 0;
	}
	if (_indicatorWidth == -1) {
		_indicatorWidth = _width;
	}
}

void BaseRenderer::setIndicator(int width, int height, int x, int y, uint32 color) {
	_indicatorWidth = width;
	_indicatorHeight = height;
	_indicatorX = x;
	_indicatorY = y;
	_indicatorColor = color;
}

void BaseRenderer::setIndicatorVal(int value) {
	bool redisplay = (_indicatorProgress != value);
	_indicatorProgress = value;
	if (redisplay)
		displayIndicator();
}

void BaseRenderer::setLoadingScreen(const char *filename, int x, int y) {
	// TODO: Handle NULL
	_loadImageName = filename;
	_loadImageX = x;
	_loadImageY = y;
}

void BaseRenderer::setSaveImage(const char *filename, int x, int y) {
	// TODO: Handle NULL
	_saveImageName = filename;
	_saveImageX = x;
	_saveImageY = y;
}

void BaseRenderer::initSaveLoad(bool isSaving, bool quickSave) {
	_indicatorDisplay = true;
	_indicatorProgress = 0;
	_hasDrawnSaveLoadImage = false;

	if (isSaving && !quickSave) {
		delete _saveLoadImage;
		_saveLoadImage = NULL;
		if (_saveImageName.size()) {
			_saveLoadImage = createSurface();

			if (!_saveLoadImage || DID_FAIL(_saveLoadImage->create(_saveImageName, true, 0, 0, 0))) {
				delete _saveLoadImage;
				_saveLoadImage = NULL;
			}
		}
	} else {
		delete _saveLoadImage;
		_saveLoadImage = NULL;
		if (_loadImageName.size()) {
			_saveLoadImage = createSurface();

			if (!_saveLoadImage || DID_FAIL(_saveLoadImage->create(_loadImageName, true, 0, 0, 0))) {
				delete _saveLoadImage;
				_saveLoadImage = NULL;
			}
		}
		_loadInProgress = true;
	}
}

void BaseRenderer::endSaveLoad() {
	_loadInProgress = false;
	_indicatorDisplay = false;
	_indicatorWidthDrawn = 0;

	delete _saveLoadImage;
	_saveLoadImage = NULL;
}

void BaseRenderer::persistSaveLoadImages(BasePersistenceManager *persistMgr) {
	persistMgr->transfer(TMEMBER(_loadImageName));
	persistMgr->transfer(TMEMBER(_saveImageName));
	persistMgr->transfer(TMEMBER(_saveImageX));
	persistMgr->transfer(TMEMBER(_saveImageY));
	persistMgr->transfer(TMEMBER(_loadImageX));
	persistMgr->transfer(TMEMBER(_loadImageY));
}

//////////////////////////////////////////////////////////////////////
BaseObject *BaseRenderer::getObjectAt(int x, int y) {
	Point32 point;
	point.x = x;
	point.y = y;

	for (int i = _rectList.size() - 1; i >= 0; i--) {
		if (BasePlatform::ptInRect(&_rectList[i]->_rect, point)) {
			if (_rectList[i]->_precise) {
				// frame
				if (_rectList[i]->_frame) {
					int xx = (int)((_rectList[i]->_frame->getRect().left + x - _rectList[i]->_rect.left + _rectList[i]->_offsetX) / (float)((float)_rectList[i]->_zoomX / (float)100));
					int yy = (int)((_rectList[i]->_frame->getRect().top  + y - _rectList[i]->_rect.top  + _rectList[i]->_offsetY) / (float)((float)_rectList[i]->_zoomY / (float)100));

					if (_rectList[i]->_frame->_mirrorX) {
						int width = _rectList[i]->_frame->getRect().right - _rectList[i]->_frame->getRect().left;
						xx = width - xx;
					}

					if (_rectList[i]->_frame->_mirrorY) {
						int height = _rectList[i]->_frame->getRect().bottom - _rectList[i]->_frame->getRect().top;
						yy = height - yy;
					}

					if (!_rectList[i]->_frame->_surface->isTransparentAt(xx, yy)) {
						return _rectList[i]->_owner;
					}
				}
				// region
				else if (_rectList[i]->_region) {
					if (_rectList[i]->_region->pointInRegion(x + _rectList[i]->_offsetX, y + _rectList[i]->_offsetY)) {
						return _rectList[i]->_owner;
					}
				}
			} else {
				return _rectList[i]->_owner;
			}
		}
	}

	return (BaseObject *)NULL;
}


//////////////////////////////////////////////////////////////////////////
void BaseRenderer::deleteRectList() {
	for (uint32 i = 0; i < _rectList.size(); i++) {
		delete _rectList[i];
	}
	_rectList.clear();
}

//////////////////////////////////////////////////////////////////////
bool BaseRenderer::initRenderer(int width, int height, bool windowed) {
	return STATUS_FAILED;
}


//////////////////////////////////////////////////////////////////////
void BaseRenderer::onWindowChange() {
}


//////////////////////////////////////////////////////////////////////////
bool BaseRenderer::windowedBlt() {
	return STATUS_FAILED;
}


//////////////////////////////////////////////////////////////////////////
bool BaseRenderer::setup2D(bool Force) {
	return STATUS_FAILED;
}


//////////////////////////////////////////////////////////////////////////
bool BaseRenderer::setupLines() {
	return STATUS_FAILED;
}

//////////////////////////////////////////////////////////////////////////
bool BaseRenderer::drawLine(int x1, int y1, int x2, int y2, uint32 color) {
	return STATUS_FAILED;
}


//////////////////////////////////////////////////////////////////////////
bool BaseRenderer::drawRect(int x1, int y1, int x2, int y2, uint32 color, int width) {
	for (int i = 0; i < width; i++) {
		drawLine(x1 + i, y1 + i, x2 - i,   y1 + i,   color); // up
		drawLine(x1 + i, y2 - i, x2 - i + 1, y2 - i, color); // down

		drawLine(x1 + i, y1 + i, x1 + i, y2 - i,   color); // left
		drawLine(x2 - i, y1 + i, x2 - i, y2 - i + 1, color); // right
	}
	return STATUS_OK;
}


//////////////////////////////////////////////////////////////////////////
bool BaseRenderer::setViewport(int left, int top, int right, int bottom) {
	return STATUS_FAILED;
}


//////////////////////////////////////////////////////////////////////////
bool BaseRenderer::setScreenViewport() {
	return setViewport(_drawOffsetX, _drawOffsetY, _width + _drawOffsetX, _height + _drawOffsetY);
}


//////////////////////////////////////////////////////////////////////////
bool BaseRenderer::setViewport(Rect32 *rect) {
	return setViewport(rect->left + _drawOffsetX,
	                   rect->top + _drawOffsetY,
	                   rect->right + _drawOffsetX,
	                   rect->bottom + _drawOffsetY);
}


//////////////////////////////////////////////////////////////////////////
bool BaseRenderer::clipCursor() {
	// TODO: Reimplement this. (Currently aspect-indpendence isn't quite finished)
	/*
	if (!_windowed) {
	    Rect32 rc;
	    GetWindowRect(_window, &rc);

	    // if "maintain aspect ratio" is in effect, lock mouse to visible area
	    rc.left = _drawOffsetX;
	    rc.top = _drawOffsetY;
	    rc.right = rc.left + _width;
	    rc.bottom = rc.top + _height;

	    ::ClipCursor(&rc);
	}
	 */
	return STATUS_OK;
}

//////////////////////////////////////////////////////////////////////////
bool BaseRenderer::unclipCursor() {
	/*
	if (!_windowed) ::ClipCursor(NULL);
	*/
	return STATUS_OK;
}

//////////////////////////////////////////////////////////////////////////
bool BaseRenderer::pointInViewport(Point32 *p) {
	if (p->x < _drawOffsetX) {
		return false;
	}
	if (p->y < _drawOffsetY) {
		return false;
	}
	if (p->x > _drawOffsetX + _width) {
		return false;
	}
	if (p->y > _drawOffsetY + _height) {
		return false;
	}

	return true;
}

void BaseRenderer::addRectToList(BaseActiveRect *rect) {
	_rectList.push_back(rect);
}

//////////////////////////////////////////////////////////////////////////
bool BaseRenderer::displayIndicator() {
	if (!_indicatorDisplay || !_indicatorProgress) {
		return STATUS_OK;
	}
	if (_saveLoadImage && !_hasDrawnSaveLoadImage) {
		Rect32 rc;
		BasePlatform::setRect(&rc, 0, 0, _saveLoadImage->getWidth(), _saveLoadImage->getHeight());
		if (_loadInProgress) {
			_saveLoadImage->displayTrans(_loadImageX, _loadImageY, rc);
		} else {
			_saveLoadImage->displayTrans(_saveImageX, _saveImageY, rc);
		}
		flip();
		_hasDrawnSaveLoadImage = true;
	}

	if ((!_indicatorDisplay && _indicatorWidth <= 0) || _indicatorHeight <= 0) {
		return STATUS_OK;
	}
	setupLines();
	int curWidth = (int)(_indicatorWidth * (float)((float)_indicatorProgress / 100.0f));
	for (int i = 0; i < _indicatorHeight; i++) {
		drawLine(_indicatorX, _indicatorY + i, _indicatorX + curWidth, _indicatorY + i, _indicatorColor);
	}

	setup2D();
	_indicatorWidthDrawn = curWidth;
	if (_indicatorWidthDrawn) {
		indicatorFlip();
	}
	return STATUS_OK;
}

} // end of namespace Wintermute
