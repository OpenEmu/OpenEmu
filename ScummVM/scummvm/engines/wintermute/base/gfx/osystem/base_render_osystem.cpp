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

#include "engines/wintermute/base/gfx/osystem/base_render_osystem.h"
#include "engines/wintermute/base/gfx/osystem/base_surface_osystem.h"
#include "engines/wintermute/base/base_surface_storage.h"
#include "engines/wintermute/base/gfx/base_image.h"
#include "engines/wintermute/math/math_util.h"
#include "engines/wintermute/base/base_game.h"
#include "engines/wintermute/base/base_sprite.h"
#include "common/system.h"
#include "engines/wintermute/graphics/transparent_surface.h"
#include "common/queue.h"
#include "common/config-manager.h"

namespace Wintermute {

RenderTicket::RenderTicket(BaseSurfaceOSystem *owner, const Graphics::Surface *surf, Common::Rect *srcRect, Common::Rect *dstRect, bool mirrorX, bool mirrorY, bool disableAlpha) : _owner(owner),
	_srcRect(*srcRect), _dstRect(*dstRect), _drawNum(0), _isValid(true), _wantsDraw(true), _hasAlpha(!disableAlpha) {
	_colorMod = 0;
	_mirror = TransparentSurface::FLIP_NONE;
	if (mirrorX) {
		_mirror |= TransparentSurface::FLIP_V;
	}
	if (mirrorY) {
		_mirror |= TransparentSurface::FLIP_H;
	}
	if (surf) {
		_surface = new Graphics::Surface();
		_surface->create((uint16)srcRect->width(), (uint16)srcRect->height(), surf->format);
		assert(_surface->format.bytesPerPixel == 4);
		// Get a clipped copy of the surface
		for (int i = 0; i < _surface->h; i++) {
			memcpy(_surface->getBasePtr(0, i), surf->getBasePtr(srcRect->left, srcRect->top + i), srcRect->width() * _surface->format.bytesPerPixel);
		}
		// Then scale it if necessary
		if (dstRect->width() != srcRect->width() || dstRect->height() != srcRect->height()) {
			TransparentSurface src(*_surface, false);
			Graphics::Surface *temp = src.scale(dstRect->width(), dstRect->height());
			_surface->free();
			delete _surface;
			_surface = temp;
		}
	} else {
		_surface = NULL;
	}
}

RenderTicket::~RenderTicket() {
	if (_surface) {
		_surface->free();
		delete _surface;
	}
}

bool RenderTicket::operator==(RenderTicket &t) {
	if ((t._srcRect != _srcRect) ||
	        (t._dstRect != _dstRect) ||
	        (t._mirror != _mirror) ||
	        (t._owner != _owner) ||
	        (t._hasAlpha != _hasAlpha) ||
	        (t._colorMod != _colorMod)) {
		return false;
	}
	return true;
}

BaseRenderer *makeOSystemRenderer(BaseGame *inGame) {
	return new BaseRenderOSystem(inGame);
}

//////////////////////////////////////////////////////////////////////////
BaseRenderOSystem::BaseRenderOSystem(BaseGame *inGame) : BaseRenderer(inGame) {
	_renderSurface = new Graphics::Surface();
	_blankSurface = new Graphics::Surface();
	_drawNum = 1;
	_needsFlip = true;

	_borderLeft = _borderRight = _borderTop = _borderBottom = 0;
	_ratioX = _ratioY = 1.0f;
	setAlphaMod(255);
	setColorMod(255, 255, 255);
	_dirtyRect = NULL;
}

//////////////////////////////////////////////////////////////////////////
BaseRenderOSystem::~BaseRenderOSystem() {
	_renderSurface->free();
	delete _renderSurface;
	_blankSurface->free();
	delete _blankSurface;
	TransparentSurface::destroyLookup();
}

//////////////////////////////////////////////////////////////////////////
bool BaseRenderOSystem::initRenderer(int width, int height, bool windowed) {
	_width = width;
	_height = height;
	_renderRect.setWidth(_width);
	_renderRect.setHeight(_height);

	_realWidth = width;
	_realHeight = height;

	//TODO: Tiny resolution-displays might want to do some resolution-selection logic here

	//_realWidth = BaseEngine::instance().getRegistry()->readInt("Debug", "ForceResWidth", _width);
	//_realHeight = BaseEngine::instance().getRegistry()->readInt("Debug", "ForceResHeight", _height);

	float origAspect = (float)_width / (float)_height;
	float realAspect = (float)_realWidth / (float)_realHeight;

	float ratio;
	if (origAspect < realAspect) {
		// normal to wide
		ratio = (float)_realHeight / (float)_height;
	} else {
		// wide to normal
		ratio = (float)_realWidth / (float)_width;
	}

	_borderLeft = (int)((_realWidth - (_width * ratio)) / 2);
	_borderRight = (int)(_realWidth - (_width * ratio) - _borderLeft);

	_borderTop = (int)((_realHeight - (_height * ratio)) / 2);
	_borderBottom = (int)(_realHeight - (_height * ratio) - _borderTop);



	_ratioX = (float)(_realWidth - _borderLeft - _borderRight) / (float)_width;
	_ratioY = (float)(_realHeight - _borderTop - _borderBottom) / (float)_height;

	_windowed = !ConfMan.getBool("fullscreen");

	Graphics::PixelFormat format(4, 8, 8, 8, 8, 16, 8, 0, 24);
	g_system->beginGFXTransaction();
	g_system->initSize(_width, _height, &format);
	OSystem::TransactionError gfxError = g_system->endGFXTransaction();

	if (gfxError != OSystem::kTransactionSuccess) {
		warning("Couldn't setup GFX-backend for %dx%dx%d", _width, _height, format.bytesPerPixel * 8);
		return STATUS_FAILED;
	}

	g_system->showMouse(false);

	_renderSurface->create(g_system->getWidth(), g_system->getHeight(), g_system->getScreenFormat());
	_blankSurface->create(g_system->getWidth(), g_system->getHeight(), g_system->getScreenFormat());
	_blankSurface->fillRect(Common::Rect(0, 0, _blankSurface->h, _blankSurface->w), _blankSurface->format.ARGBToColor(255, 0, 0, 0));
	_active = true;

	_clearColor = _renderSurface->format.ARGBToColor(255, 0, 0, 0);

	return STATUS_OK;
}

void BaseRenderOSystem::setAlphaMod(byte alpha) {
	byte r = RGBCOLGetR(_colorMod);
	byte g = RGBCOLGetB(_colorMod);
	byte b = RGBCOLGetB(_colorMod);
	_colorMod = BS_ARGB(alpha, r, g, b);
}

void BaseRenderOSystem::setColorMod(byte r, byte g, byte b) {
	byte alpha = RGBCOLGetA(_colorMod);
	_colorMod = BS_ARGB(alpha, r, g, b);
}

bool BaseRenderOSystem::indicatorFlip() {
	g_system->copyRectToScreen((byte *)_renderSurface->getBasePtr(_indicatorX, _indicatorY), _renderSurface->pitch, _indicatorX, _indicatorY, _indicatorWidthDrawn, _indicatorHeight);
	g_system->updateScreen();
	return STATUS_OK;
}

bool BaseRenderOSystem::flip() {
	if (!_disableDirtyRects) {
		drawTickets();
	} else {
		// Clear the scale-buffered tickets that wasn't reused.
		RenderQueueIterator it = _renderQueue.begin();
		while (it != _renderQueue.end()) {
			if ((*it)->_wantsDraw == false) {
				RenderTicket *ticket = *it;
				it = _renderQueue.erase(it);
				delete ticket;
			} else {
				(*it)->_wantsDraw = false;
				++it;
			}
		}
	}
	if (_needsFlip || _disableDirtyRects) {
		if (_disableDirtyRects) {
			g_system->copyRectToScreen((byte *)_renderSurface->pixels, _renderSurface->pitch, 0, 0, _renderSurface->w, _renderSurface->h);
		}
		//  g_system->copyRectToScreen((byte *)_renderSurface->pixels, _renderSurface->pitch, _dirtyRect->left, _dirtyRect->top, _dirtyRect->width(), _dirtyRect->height());
		delete _dirtyRect;
		_dirtyRect = NULL;
		g_system->updateScreen();
		_needsFlip = false;
	}
	_drawNum = 1;

	return STATUS_OK;
}

//////////////////////////////////////////////////////////////////////////
bool BaseRenderOSystem::fill(byte r, byte g, byte b, Common::Rect *rect) {
	_clearColor = _renderSurface->format.ARGBToColor(0xFF, r, g, b);
	if (!_disableDirtyRects) {
		return STATUS_OK;
	}
	if (!rect) {
// TODO: This should speed things up, but for some reason it misses the size by quite a bit.
/*		if (r == 0 && g == 0 && b == 0) {
			// Simply memcpy from the buffered black-surface, way faster than Surface::fillRect.
			memcpy(_renderSurface->pixels, _blankSurface->pixels, _renderSurface->pitch * _renderSurface->h);
			return STATUS_OK;
		}*/
		rect = &_renderRect;
	}
	// TODO: This doesn't work with dirty rects
	_renderSurface->fillRect(*rect, _clearColor);

	return STATUS_OK;
}

//////////////////////////////////////////////////////////////////////////
void BaseRenderOSystem::fade(uint16 alpha) {
	byte dwAlpha = (byte)(255 - alpha);
	return fadeToColor(0, 0, 0, dwAlpha);
}


//////////////////////////////////////////////////////////////////////////
void BaseRenderOSystem::fadeToColor(byte r, byte g, byte b, byte a, Common::Rect *rect) {
	// This particular warning is rather messy, as this function is called a ton,
	// thus we avoid printing it more than once.

	// TODO: Add fading with dirty rects.
	if (!_disableDirtyRects) {
		warning("BaseRenderOSystem::FadeToColor - Breaks when using dirty rects");
	}

	Common::Rect fillRect;

	if (rect) {
		fillRect.left = rect->left;
		fillRect.top = rect->top;
		fillRect.setWidth(rect->width());
		fillRect.setHeight(rect->height());
	} else {
		Rect32 rc;
		_gameRef->getCurrentViewportRect(&rc);
		fillRect.left = (int16)rc.left;
		fillRect.top = (int16)rc.top;
		fillRect.setWidth((int16)(rc.right - rc.left));
		fillRect.setHeight((int16)(rc.bottom - rc.top));
	}
	modTargetRect(&fillRect);

	//TODO: This is only here until I'm sure about the final pixelformat
	uint32 col = _renderSurface->format.ARGBToColor(a, r, g, b);

	setAlphaMod(255);
	setColorMod(255, 255, 255);
	Graphics::Surface surf;
	surf.create((uint16)fillRect.width(), (uint16)fillRect.height(), _renderSurface->format);
	Common::Rect sizeRect(fillRect);
	sizeRect.translate(-fillRect.top, -fillRect.left);
	surf.fillRect(fillRect, col);
	drawSurface(NULL, &surf, &sizeRect, &fillRect, false, false);
	surf.free();

	//SDL_SetRenderDrawColor(_renderer, r, g, b, a);
	//SDL_SetRenderDrawBlendMode(_renderer, SDL_BLENDMODE_BLEND);
	//SDL_RenderFillRect(_renderer, &fillRect);
}

Graphics::PixelFormat BaseRenderOSystem::getPixelFormat() const {
	return _renderSurface->format;
}

void BaseRenderOSystem::drawSurface(BaseSurfaceOSystem *owner, const Graphics::Surface *surf, Common::Rect *srcRect, Common::Rect *dstRect, bool mirrorX, bool mirrorY, bool disableAlpha) {
	// Skip rects that are completely outside the screen:
	if ((dstRect->left < 0 && dstRect->right < 0) || (dstRect->top < 0 && dstRect->bottom < 0)) {
		return;
	}

	if (owner) { // Fade-tickets are owner-less
		RenderTicket compare(owner, NULL, srcRect, dstRect, mirrorX, mirrorY, disableAlpha);
		compare._colorMod = _colorMod;
		RenderQueueIterator it;
		for (it = _renderQueue.begin(); it != _renderQueue.end(); ++it) {
			if ((*it)->_owner == owner && *(*it) == compare && (*it)->_isValid) {
				(*it)->_colorMod = _colorMod;
				if (_disableDirtyRects) {
					drawFromSurface(*it, NULL);
				} else {
					drawFromTicket(*it);
				}
				return;
			}
		}
	}
	RenderTicket *ticket = new RenderTicket(owner, surf, srcRect, dstRect, mirrorX, mirrorY, disableAlpha);
	ticket->_colorMod = _colorMod;
	if (!_disableDirtyRects) {
		drawFromTicket(ticket);
	} else {
		ticket->_wantsDraw = true;
		_renderQueue.push_back(ticket);
		drawFromSurface(ticket, NULL);
	}
}

void BaseRenderOSystem::invalidateTicket(RenderTicket *renderTicket) {
	addDirtyRect(renderTicket->_dstRect);
	renderTicket->_isValid = false;
//	renderTicket->_canDelete = true; // TODO: Maybe readd this, to avoid even more duplicates.
}

void BaseRenderOSystem::invalidateTicketsFromSurface(BaseSurfaceOSystem *surf) {
	RenderQueueIterator it;
	for (it = _renderQueue.begin(); it != _renderQueue.end(); ++it) {
		if ((*it)->_owner == surf) {
			invalidateTicket(*it);
		}
	}
}

void BaseRenderOSystem::drawFromTicket(RenderTicket *renderTicket) {
	renderTicket->_wantsDraw = true;
	// A new item always has _drawNum == 0
	if (renderTicket->_drawNum == 0) {
		// In-order
		if (_renderQueue.empty() || _drawNum > (_renderQueue.back())->_drawNum) {
			renderTicket->_drawNum = _drawNum++;
			_renderQueue.push_back(renderTicket);
			addDirtyRect(renderTicket->_dstRect);
		} else {
			// Before something
			Common::List<RenderTicket *>::iterator pos;
			for (pos = _renderQueue.begin(); pos != _renderQueue.end(); pos++) {
				if ((*pos)->_drawNum >= _drawNum) {
					break;
				}
			}
			_renderQueue.insert(pos, renderTicket);
			Common::List<RenderTicket *>::iterator it;
			renderTicket->_drawNum = _drawNum++;
			// Increment the following tickets, so they still are in line
			for (it = pos; it != _renderQueue.end(); ++it) {
				(*it)->_drawNum++;
				(*it)->_wantsDraw = false;
			}
			addDirtyRect(renderTicket->_dstRect);
		}
	} else {
		// Was drawn last round, still in the same order
		if (_drawNum == renderTicket->_drawNum) {
			_drawNum++;
		} else {
			// Remove the ticket from the list
			RenderQueueIterator it = _renderQueue.begin();
			while (it != _renderQueue.end()) {
				if ((*it) == renderTicket) {
					it = _renderQueue.erase(it);
					break;
				} else {
					++it;
				}
			}
			if (it != _renderQueue.end()) {
				// Decreement the following tickets.
				for (; it != _renderQueue.end(); ++it) {
					(*it)->_drawNum--;
				}
			}
			// Is not in order, so readd it as if it was a new ticket
			renderTicket->_drawNum = 0;
			drawFromTicket(renderTicket);
		}
	}
}

void BaseRenderOSystem::addDirtyRect(const Common::Rect &rect) {
	if (!_dirtyRect) {
		_dirtyRect = new Common::Rect(rect);
	} else {
		_dirtyRect->extend(rect);
	}
	_dirtyRect->clip(_renderRect);
}

void BaseRenderOSystem::drawTickets() {
	RenderQueueIterator it = _renderQueue.begin();
	// Clean out the old tickets
	int decrement = 0;
	while (it != _renderQueue.end()) {
		if ((*it)->_wantsDraw == false || (*it)->_isValid == false) {
			RenderTicket *ticket = *it;
			addDirtyRect((*it)->_dstRect);
			it = _renderQueue.erase(it);
			delete ticket;
			decrement++;
		} else {
			(*it)->_drawNum -= decrement;
			++it;
		}
	}
	if (!_dirtyRect || _dirtyRect->width() == 0 || _dirtyRect->height() == 0) {
		return;
	}
	// The color-mods are stored in the RenderTickets on add, since we set that state again during
	// draw, we need to keep track of what it was prior to draw.
	uint32 oldColorMod = _colorMod;

	// Apply the clear-color to the dirty rect.
	_renderSurface->fillRect(*_dirtyRect, _clearColor);
	_drawNum = 1;
	for (it = _renderQueue.begin(); it != _renderQueue.end(); ++it) {
		RenderTicket *ticket = *it;
		assert(ticket->_drawNum == _drawNum++);
		if (ticket->_isValid && ticket->_dstRect.intersects(*_dirtyRect)) {
			// dstClip is the area we want redrawn.
			Common::Rect dstClip(ticket->_dstRect);
			// reduce it to the dirty rect
			dstClip.clip(*_dirtyRect);
			// we need to keep track of the position to redraw the dirty rect
			Common::Rect pos(dstClip);
			int16 offsetX = ticket->_dstRect.left;
			int16 offsetY = ticket->_dstRect.top;
			// convert from screen-coords to surface-coords.
			dstClip.translate(-offsetX, -offsetY);

			_colorMod = ticket->_colorMod;
			drawFromSurface(ticket->getSurface(), &ticket->_srcRect, &pos, &dstClip, ticket->_mirror);
			_needsFlip = true;
		}
		// Some tickets want redraw but don't actually clip the dirty area (typically the ones that shouldnt become clear-color)
		ticket->_wantsDraw = false;
	}
	g_system->copyRectToScreen((byte *)_renderSurface->getBasePtr(_dirtyRect->left, _dirtyRect->top), _renderSurface->pitch, _dirtyRect->left, _dirtyRect->top, _dirtyRect->width(), _dirtyRect->height());

	// Revert the colorMod-state.
	_colorMod = oldColorMod;
}

// Replacement for SDL2's SDL_RenderCopy
void BaseRenderOSystem::drawFromSurface(RenderTicket *ticket, Common::Rect *clipRect) {
	TransparentSurface src(*ticket->getSurface(), false);
	bool doDelete = false;
	if (!clipRect) {
		doDelete = true;
		clipRect = new Common::Rect();
		clipRect->setWidth(ticket->getSurface()->w);
		clipRect->setHeight(ticket->getSurface()->h);
	}

	src._enableAlphaBlit = ticket->_hasAlpha;
	src.blit(*_renderSurface, ticket->_dstRect.left, ticket->_dstRect.top, ticket->_mirror, clipRect, _colorMod, clipRect->width(), clipRect->height());
	if (doDelete) {
		delete clipRect;
	}
}
void BaseRenderOSystem::drawFromSurface(const Graphics::Surface *surf, Common::Rect *srcRect, Common::Rect *dstRect, Common::Rect *clipRect, uint32 mirror) {
	TransparentSurface src(*surf, false);
	bool doDelete = false;
	if (!clipRect) {
		doDelete = true;
		clipRect = new Common::Rect();
		clipRect->setWidth(surf->w);
		clipRect->setHeight(surf->h);
	}

	src.blit(*_renderSurface, dstRect->left, dstRect->top, mirror, clipRect, _colorMod, clipRect->width(), clipRect->height());
	if (doDelete) {
		delete clipRect;
	}
}

//////////////////////////////////////////////////////////////////////////
bool BaseRenderOSystem::drawLine(int x1, int y1, int x2, int y2, uint32 color) {

	if (!_disableDirtyRects) {
		warning("BaseRenderOSystem::DrawLine - doesn't work for dirty rects yet");
	}

	byte r = RGBCOLGetR(color);
	byte g = RGBCOLGetG(color);
	byte b = RGBCOLGetB(color);
	byte a = RGBCOLGetA(color);

	//SDL_SetRenderDrawColor(_renderer, r, g, b, a);
	//SDL_SetRenderDrawBlendMode(_renderer, SDL_BLENDMODE_BLEND);

	Point32 point1, point2;
	point1.x = x1;
	point1.y = y1;
	pointToScreen(&point1);

	point2.x = x2;
	point2.y = y2;
	pointToScreen(&point2);

	// TODO: This thing is mostly here until I'm sure about the final color-format.
	uint32 colorVal = _renderSurface->format.ARGBToColor(a, r, g, b);
	_renderSurface->drawLine(point1.x, point1.y, point2.x, point2.y, colorVal);
	//SDL_RenderDrawLine(_renderer, point1.x, point1.y, point2.x, point2.y);
	return STATUS_OK;
}

//////////////////////////////////////////////////////////////////////////
BaseImage *BaseRenderOSystem::takeScreenshot() {
// TODO: Clip by viewport.
	BaseImage *screenshot = new BaseImage();
	screenshot->copyFrom(_renderSurface);
	return screenshot;
}

//////////////////////////////////////////////////////////////////////////
Common::String BaseRenderOSystem::getName() const {
	return "ScummVM-OSystem-renderer";
}

//////////////////////////////////////////////////////////////////////////
bool BaseRenderOSystem::setViewport(int left, int top, int right, int bottom) {
	Common::Rect rect;
	// TODO: Hopefully this is the same logic that ScummVM uses.
	rect.left = (int16)(left + _borderLeft);
	rect.top = (int16)(top + _borderTop);
	rect.right = (int16)((right - left) * _ratioX);
	rect.bottom = (int16)((bottom - top) * _ratioY);

	_renderRect = rect;
	return STATUS_OK;
}

Rect32 BaseRenderOSystem::getViewPort() {
	Rect32 ret;
	ret.top = _renderRect.top;
	ret.bottom = _renderRect.bottom;
	ret.left = _renderRect.left;
	ret.right = _renderRect.right;
	return ret;
}

//////////////////////////////////////////////////////////////////////////
void BaseRenderOSystem::modTargetRect(Common::Rect *rect) {
	// FIXME: This is wrong in quite a few ways right now, and ends up
	// breaking the notebook in Dirty Split, so we disable the correction
	// for now, this will need fixing when a game with odd aspect-ratios
	// show up.
	return;
	rect->left = (int16)MathUtil::round(rect->left * _ratioX + _borderLeft - _renderRect.left);
	rect->top = (int16)MathUtil::round(rect->top * _ratioY + _borderTop - _renderRect.top);
	rect->setWidth((int16)MathUtil::roundUp(rect->width() * _ratioX));
	rect->setHeight((int16)MathUtil::roundUp(rect->height() * _ratioY));
}

//////////////////////////////////////////////////////////////////////////
void BaseRenderOSystem::pointFromScreen(Point32 *point) {
	point->x = (int16)(point->x / _ratioX - _borderLeft / _ratioX + _renderRect.left);
	point->y = (int16)(point->y / _ratioY - _borderTop / _ratioY + _renderRect.top);
}


//////////////////////////////////////////////////////////////////////////
void BaseRenderOSystem::pointToScreen(Point32 *point) {
	point->x = (int16)MathUtil::roundUp(point->x * _ratioX) + _borderLeft - _renderRect.left;
	point->y = (int16)MathUtil::roundUp(point->y * _ratioY) + _borderTop - _renderRect.top;
}

//////////////////////////////////////////////////////////////////////////
void BaseRenderOSystem::dumpData(const char *filename) {
	warning("BaseRenderOSystem::DumpData(%s) - stubbed", filename); // TODO
}

BaseSurface *BaseRenderOSystem::createSurface() {
	return new BaseSurfaceOSystem(_gameRef);
}

} // end of namespace Wintermute
