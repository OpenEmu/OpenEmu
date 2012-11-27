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

#include "common/scummsys.h"
#include "common/error.h"
#include "common/system.h"
#include "common/textconsole.h"

#include "mohawk/mohawk.h"
#include "mohawk/cursors.h"
#include "mohawk/dialogs.h"
#include "mohawk/sound.h"
#include "mohawk/video.h"

namespace Mohawk {

MohawkEngine::MohawkEngine(OSystem *syst, const MohawkGameDescription *gamedesc) : Engine(syst), _gameDescription(gamedesc) {
	if (!_mixer->isReady())
		error ("Sound initialization failed");

	// Setup mixer
	syncSoundSettings();

	_sound = 0;
	_video = 0;
	_pauseDialog = 0;
	_cursor = 0;
}

MohawkEngine::~MohawkEngine() {
	delete _sound;
	delete _video;
	delete _pauseDialog;
	delete _cursor;

	for (uint32 i = 0; i < _mhk.size(); i++)
		delete _mhk[i];
	_mhk.clear();
}

Common::Error MohawkEngine::run() {
	_sound = new Sound(this);
	_video = new VideoManager(this);
	_pauseDialog = new PauseDialog(this, "The game is paused. Press any key to continue.");

	return Common::kNoError;
}

void MohawkEngine::pauseEngineIntern(bool pause) {
	if (pause) {
		_video->pauseVideos();
		_sound->pauseSound();
		_sound->pauseSLST();
	} else {
		_video->resumeVideos();
		_sound->resumeSound();
		_sound->resumeSLST();
		_system->updateScreen();
	}
}

void MohawkEngine::pauseGame() {
	runDialog(*_pauseDialog);
}

Common::SeekableReadStream *MohawkEngine::getResource(uint32 tag, uint16 id) {
	for (uint32 i = 0; i < _mhk.size(); i++)
		if (_mhk[i]->hasResource(tag, id))
			return _mhk[i]->getResource(tag, id);

	error("Could not find a '%s' resource with ID %04x", tag2str(tag), id);
	return NULL;
}

bool MohawkEngine::hasResource(uint32 tag, uint16 id) {
	for (uint32 i = 0; i < _mhk.size(); i++)
		if (_mhk[i]->hasResource(tag, id))
			return true;

	return false;
}

bool MohawkEngine::hasResource(uint32 tag, const Common::String &resName) {
	for (uint32 i = 0; i < _mhk.size(); i++)
		if (_mhk[i]->hasResource(tag, resName))
			return true;

	return false;
}

uint32 MohawkEngine::getResourceOffset(uint32 tag, uint16 id) {
	for (uint32 i = 0; i < _mhk.size(); i++)
		if (_mhk[i]->hasResource(tag, id))
			return _mhk[i]->getOffset(tag, id);

	error("Could not find a '%s' resource with ID %04x", tag2str(tag), id);
	return 0;
}

uint16 MohawkEngine::findResourceID(uint32 tag, const Common::String &resName) {
	for (uint32 i = 0; i < _mhk.size(); i++)
		if (_mhk[i]->hasResource(tag, resName))
			return _mhk[i]->findResourceID(tag, resName);

	error("Could not find a '%s' resource matching name '%s'", tag2str(tag), resName.c_str());
	return 0xFFFF;
}

Common::String MohawkEngine::getResourceName(uint32 tag, uint16 id) {
	for (uint32 i = 0; i < _mhk.size(); i++)
		if (_mhk[i]->hasResource(tag, id)) {
			return _mhk[i]->getName(tag, id);
		}

	error("Could not find a \'%s\' resource with ID %04x", tag2str(tag), id);
	return 0;
}

} // End of namespace Mohawk
