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

// Main rendering loop

#include "saga/saga.h"

#include "saga/actor.h"
#include "saga/font.h"
#include "saga/gfx.h"
#include "saga/interface.h"
#include "saga/objectmap.h"
#include "saga/puzzle.h"
#include "saga/render.h"
#include "saga/scene.h"

#include "common/timer.h"
#include "common/system.h"

namespace Saga {

const char *test_txt = "The quick brown fox jumped over the lazy dog. She sells sea shells down by the sea shore.";
const char *pauseStringITE = "PAWS GAME";
const char *pauseStringIHNM = "Game Paused";

Render::Render(SagaEngine *vm, OSystem *system) {
	_vm = vm;
	_system = system;
	_initialized = false;
	_fullRefresh = true;

#ifdef SAGA_DEBUG
	// Initialize FPS timer callback
	_vm->getTimerManager()->installTimerProc(&fpsTimerCallback, 1000000, this, "sagaFPS");
#endif

	_backGroundSurface.create(_vm->getDisplayInfo().width, _vm->getDisplayInfo().height, Graphics::PixelFormat::createFormatCLUT8());

	_flags = 0;

	_initialized = true;
}

Render::~Render() {
#ifdef SAGA_DEBUG
	_vm->getTimerManager()->removeTimerProc(&fpsTimerCallback);
#endif

	_backGroundSurface.free();

	_initialized = false;
}

bool Render::initialized() {
	return _initialized;
}

void Render::drawScene() {
	Point mousePoint;
	Point textPoint;
	int curMode = _vm->_interface->getMode();
	assert(_initialized);

#ifdef SAGA_DEBUG
	_renderedFrameCount++;
#endif

	// Get mouse coordinates
	mousePoint = _vm->mousePos();

	if (!_fullRefresh)
		restoreChangedRects();
	else
		_dirtyRects.clear();

	if (!(_flags & (RF_DEMO_SUBST | RF_MAP) || curMode == kPanelPlacard)) {
		if (_vm->_interface->getFadeMode() != kFadeOut) {
			// Display scene background
			if (!(_flags & RF_DISABLE_ACTORS) || _vm->getGameId() == GID_ITE)
				_vm->_scene->draw();

			if (_vm->_scene->isITEPuzzleScene()) {
				_vm->_puzzle->movePiece(mousePoint);
				_vm->_actor->drawSpeech();
			} else {
				// Draw queued actors
				if (!(_flags & RF_DISABLE_ACTORS))
					_vm->_actor->drawActors();
			}

			// WORKAROUND
			// Bug #2886130: "ITE: Graphic Glitches during Cat Tribe Celebration"
			if (_vm->_scene->currentSceneNumber() == 274) {
				_vm->_interface->drawStatusBar();
			}

#ifdef SAGA_DEBUG
			if (getFlags() & RF_OBJECTMAP_TEST) {
				if (_vm->_scene->_objectMap)
					_vm->_scene->_objectMap->draw(mousePoint, kITEColorBrightWhite, kITEColorBlack);
				if (_vm->_scene->_actionMap)
					_vm->_scene->_actionMap->draw(mousePoint, kITEColorRed, kITEColorBlack);
			}
#endif

#ifdef ACTOR_DEBUG
			if (getFlags() & RF_ACTOR_PATH_TEST) {
				_vm->_actor->drawPathTest();
			}
#endif
		}
	} else {
		_fullRefresh = true;
	}

	if (_flags & RF_MAP)
		_vm->_interface->mapPanelDrawCrossHair();

	if ((curMode == kPanelOption) ||
		(curMode == kPanelQuit) ||
		(curMode == kPanelLoad) ||
		(curMode == kPanelSave)) {
		_vm->_interface->drawOption();

		if (curMode == kPanelQuit) {
			_vm->_interface->drawQuit();
		}
		if (curMode == kPanelLoad) {
			_vm->_interface->drawLoad();
		}
		if (curMode == kPanelSave) {
			_vm->_interface->drawSave();
		}
	}

	if (curMode == kPanelProtect) {
		_vm->_interface->drawProtect();
	}

	// Draw queued text strings
	_vm->_scene->drawTextList();

	// Handle user input
	_vm->processInput();

#ifdef SAGA_DEBUG
	// Display rendering information
	if (_flags & RF_SHOW_FPS) {
		char txtBuffer[20];
		sprintf(txtBuffer, "%d", _fps);
		textPoint.x = _vm->_gfx->getBackBufferWidth() - _vm->_font->getStringWidth(kKnownFontSmall, txtBuffer, 0, kFontOutline);
		textPoint.y = 2;

		_vm->_font->textDraw(kKnownFontSmall, txtBuffer, textPoint, kITEColorBrightWhite, kITEColorBlack, kFontOutline);
	}
#endif

	// Display "paused game" message, if applicable
	if (_flags & RF_RENDERPAUSE) {
		const char *pauseString = (_vm->getGameId() == GID_ITE) ? pauseStringITE : pauseStringIHNM;
		textPoint.x = (_vm->_gfx->getBackBufferWidth() - _vm->_font->getStringWidth(kKnownFontPause, pauseString, 0, kFontOutline)) / 2;
		textPoint.y = 90;

		_vm->_font->textDraw(kKnownFontPause, pauseString, textPoint,
							_vm->KnownColor2ColorId(kKnownColorBrightWhite), _vm->KnownColor2ColorId(kKnownColorBlack), kFontOutline);
	}

	// Update user interface
	_vm->_interface->update(mousePoint, UPDATE_MOUSEMOVE);

#ifdef SAGA_DEBUG
	// Display text formatting test, if applicable
	if (_flags & RF_TEXT_TEST) {
		Rect rect(mousePoint.x, mousePoint.y, mousePoint.x + 100, mousePoint.y + 50);
		_vm->_font->textDrawRect(kKnownFontMedium, test_txt, rect,
				kITEColorBrightWhite, kITEColorBlack, (FontEffectFlags)(kFontOutline | kFontCentered));
	}

	// Display palette test, if applicable
	if (_flags & RF_PALETTE_TEST) {
		_vm->_gfx->drawPalette();
	}
#endif

	drawDirtyRects();

	_system->updateScreen();

	// TODO: Change this to false to use dirty rectangles
	// Still quite buggy
	_fullRefresh = true;
}

void Render::addDirtyRect(Common::Rect r) {
	if (_fullRefresh)
		return;

	// Clip rectangle
	r.clip(_backGroundSurface.w, _backGroundSurface.h);

	// If it is empty after clipping, we are done
	if (r.isEmpty())
		return;

	// Check if the new rectangle is contained within another in the list
	Common::List<Common::Rect>::iterator it;
	for (it = _dirtyRects.begin(); it != _dirtyRects.end(); ) {
		// If we find a rectangle which fully contains the new one,
		// we can abort the search.
		if (it->contains(r))
			return;

		// Conversely, if we find rectangles which are contained in
		// the new one, we can remove them
		if (r.contains(*it))
			it = _dirtyRects.erase(it);
		else
			++it;
	}

	// If we got here, we can safely add r to the list of dirty rects.
	if (_vm->_interface->getFadeMode() != kFadeOut)
		_dirtyRects.push_back(r);
}

void Render::restoreChangedRects() {
	if (!_fullRefresh) {
		Common::List<Common::Rect>::const_iterator it;
		for (it = _dirtyRects.begin(); it != _dirtyRects.end(); ++it) {
			//_backGroundSurface.frameRect(*it, 1);		// DEBUG
			if (_vm->_interface->getFadeMode() != kFadeOut)
				g_system->copyRectToScreen(_vm->_gfx->getBackBufferPixels(), _backGroundSurface.w, it->left, it->top, it->width(), it->height());
		}
	}
	_dirtyRects.clear();
}

void Render::drawDirtyRects() {
	if (!_fullRefresh) {
		Common::List<Common::Rect>::const_iterator it;
		for (it = _dirtyRects.begin(); it != _dirtyRects.end(); ++it) {
			//_backGroundSurface.frameRect(*it, 2);		// DEBUG
			if (_vm->_interface->getFadeMode() != kFadeOut)
				g_system->copyRectToScreen(_vm->_gfx->getBackBufferPixels(), _backGroundSurface.w, it->left, it->top, it->width(), it->height());
		}
	} else {
		_system->copyRectToScreen(_vm->_gfx->getBackBufferPixels(), _vm->_gfx->getBackBufferWidth(), 0, 0,
								  _vm->_gfx->getBackBufferWidth(), _vm->_gfx->getBackBufferHeight());
	}

	_dirtyRects.clear();
}

#ifdef SAGA_DEBUG
void Render::fpsTimerCallback(void *refCon) {
	((Render *)refCon)->fpsTimer();
}

void Render::fpsTimer() {
	_fps = _renderedFrameCount;
	_renderedFrameCount = 0;
}
#endif

} // End of namespace Saga
