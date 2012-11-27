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
#include "common/file.h"
#include "common/memstream.h"

#include "gob/gob.h"
#include "gob/demos/demoplayer.h"
#include "gob/global.h"
#include "gob/util.h"
#include "gob/draw.h"
#include "gob/inter.h"
#include "gob/videoplayer.h"
#include "gob/sound/sound.h"

namespace Gob {

DemoPlayer::Script DemoPlayer::_scripts[] = {
	{kScriptSourceFile, "demo.scn"},
	{kScriptSourceFile, "wdemo.s24"},
	{kScriptSourceFile, "play123.scn"},
	{kScriptSourceFile, "e.scn"},
	{kScriptSourceFile, "i.scn"},
	{kScriptSourceFile, "s.scn"},
	{kScriptSourceDirect,
		"slide machu.imd 20\nslide conseil.imd 20\nslide cons.imd 20\n" \
		"slide tumia.imd 1\nslide tumib.imd 1\nslide tumic.imd 1\n"     \
		"slide tumid.imd 1\nslide post.imd 1\nslide posta.imd 1\n"      \
		"slide postb.imd 1\nslide postc.imd 1\nslide xdome.imd 20\n"    \
		"slide xant.imd 20\nslide tum.imd 20\nslide voile.imd 20\n"     \
		"slide int.imd 20\nslide voila.imd 1\nslide voilb.imd 1\n"},
	{kScriptSourceFile, "coktelplayer.scn"},
	{kScriptSourceFile, "demogb.scn"},
	{kScriptSourceFile, "demoall.scn"},
	{kScriptSourceFile, "demofra.scn"}
};

DemoPlayer::DemoPlayer(GobEngine *vm) : _vm(vm) {
	_autoDouble = false;
	_doubleMode = false;
	_rebase0 = false;
}

DemoPlayer::~DemoPlayer() {
}

bool DemoPlayer::play(const char *fileName) {
	if (!fileName)
		return false;

	debugC(1, kDebugDemo, "Playing \"%s\"", fileName);

	init();

	Common::File bat;

	if (!bat.open(fileName))
		return false;

	return playStream(bat);
}

bool DemoPlayer::play(uint32 index) {
	if (index >= ARRAYSIZE(_scripts))
		return false;

	Script &script = _scripts[index];

	debugC(1, kDebugDemo, "Playing demoIndex %d: %d", index, script.source);

	switch (script.source) {
	case kScriptSourceFile:
		return play(script.script);

	case kScriptSourceDirect:
		{
			Common::MemoryReadStream stream((const byte *)script.script, strlen(script.script));

			init();
			return playStream(stream);
		}

	default:
		return false;
	}
}

bool DemoPlayer::lineStartsWith(const Common::String &line, const char *start) {
	return (strstr(line.c_str(), start) == line.c_str());
}

void DemoPlayer::init() {
	// The video player needs some fake variables
	_vm->_inter->allocateVars(32);

	// Init the screen
	_vm->_draw->initScreen();
	_vm->_draw->_cursorIndex = -1;

	_vm->_util->longDelay(200); // Letting everything settle

}

void DemoPlayer::clearScreen() {
	debugC(1, kDebugDemo, "Clearing the screen");
	_vm->_draw->_backSurface->clear();
	_vm->_draw->forceBlit();
	_vm->_video->retrace();
}

void DemoPlayer::playVideo(const char *fileName) {
	uint32 waitTime = 0;
	char *file, *filePtr;

	file = filePtr = strdup(fileName);

	// Trimming spaces front
	while (*file == ' ')
		file++;

	char *spaceBack = strchr(file, ' ');
	if (spaceBack) {
		char *nextSpace = strchr(spaceBack, ' ');

		if (nextSpace)
			*nextSpace = '\0';

		*spaceBack++ = '\0';

		waitTime = atoi(spaceBack) * 100;
	}

	debugC(1, kDebugDemo, "Playing video \"%s\"", file);

	VideoPlayer::Properties props;

	props.x = _rebase0 ? 0 : -1;
	props.y = _rebase0 ? 0 : -1;

	props.switchColorMode = true;

	int slot;
	if ((slot = _vm->_vidPlayer->openVideo(true, file, props)) >= 0) {
		if (_autoDouble) {
			int16 defX = _rebase0 ? 0 : _vm->_vidPlayer->getDefaultX();
			int16 defY = _rebase0 ? 0 : _vm->_vidPlayer->getDefaultY();
			int16 right  = defX + _vm->_vidPlayer->getWidth()  - 1;
			int16 bottom = defY + _vm->_vidPlayer->getHeight() - 1;

			_doubleMode = ((right < 320) && (bottom < 200));
		}

		if (_doubleMode)
			playVideoDoubled(slot);
		else
			playVideoNormal(slot);

		_vm->_vidPlayer->closeVideo(slot);

		if (waitTime > 0)
			_vm->_util->longDelay(waitTime);
	}


	free(filePtr);
}

void DemoPlayer::playADL(const char *params) {
	const char *end;

	end = strchr(params, ' ');
	if (!end)
		end = params + strlen(params);

	Common::String fileName(params, end);
	bool  waitEsc = true;
	int32 repeat  = -1;

	if (*end != '\0') {
		const char *start = end + 1;

		waitEsc = (*start != '0');

		end = strchr(start, ' ');
		if (end)
			repeat = atoi(end + 1);
	}

	playADL(fileName, waitEsc, repeat);
}

void DemoPlayer::playVideoNormal(int slot) {
	VideoPlayer::Properties props;

	_vm->_vidPlayer->play(slot, props);
}

void DemoPlayer::playVideoDoubled(int slot) {
	Common::String fileNameOpened = _vm->_vidPlayer->getFileName(slot);
	_vm->_vidPlayer->closeVideo(slot);

	VideoPlayer::Properties props;

	props.x            = _rebase0 ? 0 : -1;
	props.y            = _rebase0 ? 0 : -1;
	props.flags        = VideoPlayer::kFlagScreenSurface;
	props.waitEndFrame = false;

	_vm->_vidPlayer->evaluateFlags(props);

	slot = _vm->_vidPlayer->openVideo(true, fileNameOpened, props);
	if (slot < 0)
		return;

	for (uint i = 0; i < _vm->_vidPlayer->getFrameCount(slot); i++) {
		props.startFrame = _vm->_vidPlayer->getCurrentFrame(slot) + 1;
		props.lastFrame  = _vm->_vidPlayer->getCurrentFrame(slot) + 1;

		_vm->_vidPlayer->play(slot, props);

		const Common::List<Common::Rect> *rects = _vm->_vidPlayer->getDirtyRects(slot);
		if (rects) {
			for (Common::List<Common::Rect>::const_iterator rect = rects->begin(); rect != rects->end(); ++rect) {
				int16 w  = rect->right  - rect->left;
				int16 h  = rect->bottom - rect->top;
				int16 wD = (rect->left * 2) + (w * 2);
				int16 hD = (rect->top  * 2) + (h * 2);

				_vm->_draw->_frontSurface->blitScaled(*_vm->_draw->_spritesArray[0],
						rect->left, rect->top, rect->right - 1, rect->bottom - 1, rect->left * 2, rect->top * 2, 2);

				_vm->_draw->dirtiedRect(_vm->_draw->_frontSurface,
						rect->left * 2, rect->top * 2, wD, hD);
			}
		}

		_vm->_video->retrace();

		_vm->_util->processInput();
		if (_vm->shouldQuit())
			break;

		int16 key;
		bool end = false;
		while (_vm->_util->checkKey(key))
			if (key == kKeyEscape)
				end = true;
		if (end)
			break;

		_vm->_vidPlayer->waitEndFrame(slot);
	}

}

void DemoPlayer::playADL(const Common::String &fileName, bool waitEsc, int32 repeat) {
	debugC(1, kDebugDemo, "Playing ADL \"%s\" (%d, %d)", fileName.c_str(), waitEsc, repeat);

	_vm->_sound->adlibUnload();
	_vm->_sound->adlibLoadADL(fileName.c_str());
	_vm->_sound->adlibSetRepeating(repeat);
	_vm->_sound->adlibPlay();

	if (!waitEsc)
		return;

	int16 key = 0;
	while (!_vm->shouldQuit() && (key != kKeyEscape) && _vm->_sound->adlibIsPlaying()) {
		_vm->_util->longDelay(1);
		while (_vm->_util->checkKey(key))
			if (key == kKeyEscape)
				break;
	}
}

void DemoPlayer::evaluateVideoMode(const char *mode) {
	debugC(2, kDebugDemo, "Video mode \"%s\"", mode);

	_autoDouble = false;
	_doubleMode = false;

	// Only applicable when we actually can double
	if (_vm->is640x480() || _vm->is800x600()) {
		if      (!scumm_strnicmp(mode, "AUTO", 4))
			_autoDouble = true;
		else if (!scumm_strnicmp(mode, "VGA", 3))
			_doubleMode = true;
	}
}

} // End of namespace Gob
