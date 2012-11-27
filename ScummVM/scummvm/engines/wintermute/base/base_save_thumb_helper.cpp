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

#include "engines/wintermute/base/base_save_thumb_helper.h"
#include "engines/wintermute/base/gfx/base_image.h"
#include "engines/wintermute/base/gfx/base_renderer.h"
#include "engines/wintermute/base/base_game.h"

namespace Wintermute {

//////////////////////////////////////////////////////////////////////////
BaseSaveThumbHelper::BaseSaveThumbHelper(BaseGame *inGame) : BaseClass(inGame) {
	_thumbnail = NULL;
}

//////////////////////////////////////////////////////////////////////////
BaseSaveThumbHelper::~BaseSaveThumbHelper(void) {
	delete _thumbnail;
	_thumbnail = NULL;
}

//////////////////////////////////////////////////////////////////////////
bool BaseSaveThumbHelper::storeThumbnail(bool doFlip) {
	delete _thumbnail;
	_thumbnail = NULL;

	if (_gameRef->_thumbnailWidth > 0 && _gameRef->_thumbnailHeight > 0) {
		if (doFlip) {
			// when using opengl on windows it seems to be necessary to do this twice
			// works normally for direct3d
			_gameRef->displayContent(false);
			_gameRef->_renderer->flip();

			_gameRef->displayContent(false);
			_gameRef->_renderer->flip();
		}

		BaseImage *screenshot = _gameRef->_renderer->takeScreenshot();
		if (!screenshot) {
			return STATUS_FAILED;
		}

		// normal thumbnail
		if (_gameRef->_thumbnailWidth > 0 && _gameRef->_thumbnailHeight > 0) {
			_thumbnail = new BaseImage();
			_thumbnail->copyFrom(screenshot, _gameRef->_thumbnailWidth, _gameRef->_thumbnailHeight);
		}


		delete screenshot;
		screenshot = NULL;
	}
	return STATUS_OK;
}

} // end of namespace Wintermute
