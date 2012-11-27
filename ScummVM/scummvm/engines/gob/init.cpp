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

#include "common/endian.h"

#include "gob/gob.h"
#include "gob/init.h"
#include "gob/global.h"
#include "gob/util.h"
#include "gob/dataio.h"
#include "gob/draw.h"
#include "gob/game.h"
#include "gob/script.h"
#include "gob/palanim.h"
#include "gob/inter.h"
#include "gob/video.h"
#include "gob/videoplayer.h"

#include "gob/sound/sound.h"

#include "gob/demos/scnplayer.h"
#include "gob/demos/batplayer.h"

#include "gob/pregob/pregob.h"

namespace Gob {

const char *const Init::_fontNames[] = { "jeulet1.let", "jeulet2.let", "jeucar1.let", "jeumath.let" };

Init::Init(GobEngine *vm) : _vm(vm) {
	_palDesc = 0;
}

Init::~Init() {
}

void Init::cleanup() {
	_vm->_global->_primarySurfDesc.reset();

	_vm->_sound->speakerOff();
	_vm->_sound->blasterStop(0);
	_vm->_dataIO->closeArchive(true);
}

void Init::doDemo() {
	if (_vm->isSCNDemo()) {
		// This is a non-interactive demo with a SCN script and VMD videos

		SCNPlayer scnPlayer(_vm);

		if (_vm->_demoIndex > 0)
			scnPlayer.play(_vm->_demoIndex - 1);
	}

	if (_vm->isBATDemo()) {
		// This is a non-interactive demo with a BAT script and videos

		BATPlayer batPlayer(_vm);

		if (_vm->_demoIndex > 0)
			batPlayer.play(_vm->_demoIndex - 1);
	}
}

void Init::initGame() {
	initVideo();
	updateConfig();

	if (!_vm->isDemo()) {
		if (_vm->_dataIO->hasFile(_vm->_startStk))
			_vm->_dataIO->openArchive(_vm->_startStk, true);
	}

	_vm->_util->initInput();

	_vm->_video->initPrimary(_vm->_global->_videoMode);
	_vm->_global->_mouseXShift = 1;
	_vm->_global->_mouseYShift = 1;

	_palDesc = new Video::PalDesc;

	_vm->validateVideoMode(_vm->_global->_videoMode);

	_vm->_global->_setAllPalette = true;
	_palDesc->vgaPal = _vm->_draw->_vgaPalette;
	_palDesc->unused1 = _vm->_draw->_unusedPalette1;
	_palDesc->unused2 = _vm->_draw->_unusedPalette2;
	_vm->_video->setFullPalette(_palDesc);

	for (int i = 0; i < 10; i++)
		_vm->_draw->_fascinWin[i].id = -1;

	_vm->_draw->_winCount = 0;

	for (int i = 0; i < 8; i++)
		_vm->_draw->_fonts[i] = 0;

	if (_vm->isDemo()) {
		doDemo();
		delete _palDesc;
		_vm->_video->initPrimary(-1);
		cleanup();
		return;
	}

	if (_vm->_preGob) {
		_vm->_preGob->run();
		delete _palDesc;
		_vm->_video->initPrimary(-1);
		cleanup();
		return;
	}

	Common::SeekableReadStream *infFile = _vm->_dataIO->getFile("intro.inf");
	if (!infFile) {

		for (int i = 0; i < 4; i++)
			_vm->_draw->loadFont(i, _fontNames[i]);

	} else {

		for (int i = 0; i < 8; i++) {
			if (infFile->eos())
				break;

			Common::String font = infFile->readLine();
			if (infFile->eos() && font.empty())
				break;

			font += ".let";

			_vm->_draw->loadFont(i, font.c_str());
		}

		delete infFile;
	}

	if (_vm->_dataIO->hasFile(_vm->_startTot)) {
		_vm->_inter->allocateVars(Script::getVariablesCount(_vm->_startTot.c_str(), _vm));

		_vm->_game->_curTotFile = _vm->_startTot;

		_vm->_sound->cdTest(1, "GOB");
		_vm->_sound->cdLoadLIC("gob.lic");

		// Search for a Coktel logo animation or image to display
		if (_vm->_dataIO->hasFile("coktel.imd")) {
			_vm->_draw->initScreen();
			_vm->_draw->_cursorIndex = -1;

			_vm->_util->longDelay(200); // Letting everything settle

			VideoPlayer::Properties props;
			int slot;
			if ((slot = _vm->_vidPlayer->openVideo(true, "coktel.imd", props)) >= 0) {
				_vm->_vidPlayer->play(slot, props);
				_vm->_vidPlayer->closeVideo(slot);
			}

			_vm->_draw->closeScreen();
		} else if (_vm->_dataIO->hasFile("coktel.clt")) {
			Common::SeekableReadStream *stream = _vm->_dataIO->getFile("coktel.clt");
			if (stream) {
				_vm->_draw->initScreen();
				_vm->_util->clearPalette();

				stream->read((byte *)_vm->_draw->_vgaPalette, 768);
				delete stream;

				int32 size;
				byte *sprite = _vm->_dataIO->getFile("coktel.ims", size);
				if (sprite) {
					_vm->_video->drawPackedSprite(sprite, 320, 200, 0, 0, 0,
							*_vm->_draw->_frontSurface);
					_vm->_palAnim->fade(_palDesc, 0, 0);
					_vm->_util->delay(500);

					delete[] sprite;
				}

				_vm->_draw->closeScreen();
			}
		}

		_vm->_game->start();

		_vm->_sound->cdStop();
		_vm->_sound->cdUnloadLIC();

	}

	delete _palDesc;
	_vm->_dataIO->closeArchive(true);
	_vm->_video->initPrimary(-1);
	cleanup();
}

void Init::updateConfig() {
}

} // End of namespace Gob
