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

#ifdef ENABLE_SAGA2

// "Dinotopia" and "Faery Tale Adventure II: Halls of the Dead" Intro sequence scene procedures

#include "saga/saga.h"
#include "saga/scene.h"
#include "saga/gfx.h"

#include "common/events.h"
#include "common/keyboard.h"
#include "common/system.h"
#include "common/textconsole.h"
#include "graphics/palette.h"
#include "graphics/surface.h"
#include "video/smk_decoder.h"

namespace Saga {

int Scene::DinoStartProc() {
	_vm->_gfx->showCursor(false);

	playMovie("testvid.smk");

	// HACK: Forcibly quit here
	_vm->quitGame();

	return SUCCESS;
}

int Scene::FTA2StartProc() {
	_vm->_gfx->showCursor(false);

	playMovie("trimark.smk");
	playMovie("intro.smk");

	// HACK: Forcibly quit here
	_vm->quitGame();

	return SUCCESS;
}

int Scene::FTA2EndProc(FTA2Endings whichEnding) {
	char videoName[20];

	switch (whichEnding) {
	case kFta2BadEndingLaw:
		strcpy(videoName, "end_1.smk");
		break;
	case kFta2BadEndingChaos:
		strcpy(videoName, "end_2.smk");
		break;
	case kFta2GoodEnding1:
		strcpy(videoName, "end_3a.smk");
		break;
	case kFta2GoodEnding2:
		strcpy(videoName, "end_3b.smk");
		break;
	case kFta2BadEndingDeath:
		strcpy(videoName, "end_4.smk");
		break;
	default:
		error("Unknown FTA2 ending");
	}

	_vm->_gfx->showCursor(false);

	// Play ending
	playMovie(videoName);

	return SUCCESS;
}

void Scene::playMovie(const char *filename) {
	Video::SmackerDecoder *smkDecoder = new Video::SmackerDecoder();

	if (!smkDecoder->loadFile(filename))
		return;

	uint16 x = (g_system->getWidth() - smkDecoder->getWidth()) / 2;
	uint16 y = (g_system->getHeight() - smkDecoder->getHeight()) / 2;
	bool skipVideo = false;

	smkDecoder->start();

	while (!_vm->shouldQuit() && !smkDecoder->endOfVideo() && !skipVideo) {
		if (smkDecoder->needsUpdate()) {
			const Graphics::Surface *frame = smkDecoder->decodeNextFrame();
			if (frame) {
				_vm->_system->copyRectToScreen(frame->pixels, frame->pitch, x, y, frame->w, frame->h);

				if (smkDecoder->hasDirtyPalette())
					_vm->_system->getPaletteManager()->setPalette(smkDecoder->getPalette(), 0, 256);

				_vm->_system->updateScreen();
			}
		}

		Common::Event event;
		while (_vm->_system->getEventManager()->pollEvent(event)) {
			if ((event.type == Common::EVENT_KEYDOWN && event.kbd.keycode == Common::KEYCODE_ESCAPE) || event.type == Common::EVENT_LBUTTONUP)
				skipVideo = true;
		}

		_vm->_system->delayMillis(10);
	}
}

} // End of namespace Saga

#endif
