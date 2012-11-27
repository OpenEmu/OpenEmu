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
 *
 */

#include "common/savefile.h"

#include "graphics/thumbnail.h"

#include "toltecs/toltecs.h"
#include "toltecs/animation.h"
#include "toltecs/music.h"
#include "toltecs/palette.h"
#include "toltecs/script.h"
#include "toltecs/screen.h"
#include "toltecs/sound.h"

namespace Toltecs {

/* TODO:
	- Saving during an animation (AnimationPlayer) is not working correctly yet
	- Maybe switch to SCUMM/Tinsel serialization approach?
*/

#define TOLTECS_SAVEGAME_VERSION 4

ToltecsEngine::kReadSaveHeaderError ToltecsEngine::readSaveHeader(Common::SeekableReadStream *in, bool loadThumbnail, SaveHeader &header) {

	header.version = in->readUint32LE();
	if (header.version > TOLTECS_SAVEGAME_VERSION)
		return kRSHEInvalidVersion;

	byte descriptionLen = in->readByte();
	header.description = "";
	while (descriptionLen--)
		header.description += (char)in->readByte();

	if (loadThumbnail) {
		header.thumbnail = Graphics::loadThumbnail(*in);
	} else {
		Graphics::skipThumbnail(*in);
	}

	// Not used yet, reserved for future usage
	header.gameID = in->readByte();
	header.flags = in->readUint32LE();

	if (header.version >= 1) {
		header.saveDate = in->readUint32LE();
		header.saveTime = in->readUint32LE();
		header.playTime = in->readUint32LE();
	} else {
		header.saveDate = 0;
		header.saveTime = 0;
		header.playTime = 0;
	}

	return ((in->eos() || in->err()) ? kRSHEIoError : kRSHENoError);
}

void ToltecsEngine::savegame(const char *filename, const char *description) {
	Common::OutSaveFile *out;
	if (!(out = g_system->getSavefileManager()->openForSaving(filename))) {
		warning("Can't create file '%s', game not saved", filename);
		return;
	}

	TimeDate curTime;
	g_system->getTimeAndDate(curTime);

	// Header start
	out->writeUint32LE(TOLTECS_SAVEGAME_VERSION);

	byte descriptionLen = strlen(description);
	out->writeByte(descriptionLen);
	out->write(description, descriptionLen);

	Graphics::saveThumbnail(*out);

	// Not used yet, reserved for future usage
	out->writeByte(0);
	out->writeUint32LE(0);
	uint32 saveDate = ((curTime.tm_mday & 0xFF) << 24) | (((curTime.tm_mon + 1) & 0xFF) << 16) | ((curTime.tm_year + 1900) & 0xFFFF);
	uint32 saveTime = ((curTime.tm_hour & 0xFF) << 16) | (((curTime.tm_min) & 0xFF) << 8) | ((curTime.tm_sec) & 0xFF);
	uint32 playTime = g_engine->getTotalPlayTime() / 1000;
	out->writeUint32LE(saveDate);
	out->writeUint32LE(saveTime);
	out->writeUint32LE(playTime);
	// Header end

	out->writeUint16LE(_cameraX);
	out->writeUint16LE(_cameraY);
	out->writeUint16LE(_cameraHeight);

	out->writeUint16LE(_guiHeight);

	out->writeUint16LE(_sceneWidth);
	out->writeUint16LE(_sceneHeight);
	out->writeUint32LE(_sceneResIndex);

	out->writeUint16LE(_walkSpeedX);
	out->writeUint16LE(_walkSpeedY);

	out->writeUint32LE(_counter01);
	out->writeUint32LE(_counter02);
	out->writeByte(_movieSceneFlag ? 1 : 0);
	out->writeByte(_flag01);

	out->writeUint16LE(_mouseX);
	out->writeUint16LE(_mouseY);
	out->writeUint16LE(_mouseDisabled);

	_palette->saveState(out);
	_script->saveState(out);
	_anim->saveState(out);
	_screen->saveState(out);
	_sound->saveState(out);
	_music->saveState(out);

	out->finalize();
	delete out;
}

void ToltecsEngine::loadgame(const char *filename) {
	Common::InSaveFile *in = g_system->getSavefileManager()->openForLoading(filename);
	if (!in) {
		warning("Can't open file '%s', game not loaded", filename);
		return;
	}

	SaveHeader header;

	kReadSaveHeaderError errorCode = readSaveHeader(in, false, header);

	if (errorCode != kRSHENoError) {
		warning("Error loading savegame '%s'", filename);
		delete in;
		return;
	}

	_sound->stopAll();
	_music->stopSequence();
	g_engine->setTotalPlayTime(header.playTime * 1000);

	_cameraX = in->readUint16LE();
	_cameraY = in->readUint16LE();
	_cameraHeight = in->readUint16LE();

	_guiHeight = in->readUint16LE();

	_sceneWidth = in->readUint16LE();
	_sceneHeight = in->readUint16LE();
	_sceneResIndex = in->readUint32LE();

	_walkSpeedX = in->readUint16LE();
	_walkSpeedY = in->readUint16LE();

	_counter01 = in->readUint32LE();
	_counter02 = in->readUint32LE();
	_movieSceneFlag = in->readByte() != 0;
	_flag01 = in->readByte();

	_mouseX = in->readUint16LE();
	_mouseY = in->readUint16LE();
	_mouseDisabled = in->readUint16LE();

	_system->warpMouse(_mouseX, _mouseY);
 	_system->showMouse(_mouseDisabled == 0);

	_palette->loadState(in);
	_script->loadState(in);
	_anim->loadState(in);
	_screen->loadState(in);
	if (header.version >= 2)
		_sound->loadState(in, header.version);
	if (header.version >= 3)
		_music->loadState(in);

	delete in;

	loadScene(_sceneResIndex);

	_newCameraX = _cameraX;
	_newCameraY = _cameraY;
}

Common::Error ToltecsEngine::loadGameState(int slot) {
	const char *fileName = getSavegameFilename(slot);
	loadgame(fileName);
	return Common::kNoError;
}

Common::Error ToltecsEngine::saveGameState(int slot, const Common::String &description) {
	const char *fileName = getSavegameFilename(slot);
	savegame(fileName, description.c_str());
	return Common::kNoError;
}

const char *ToltecsEngine::getSavegameFilename(int num) {
	static Common::String filename;
	filename = getSavegameFilename(_targetName, num);
	return filename.c_str();
}

Common::String ToltecsEngine::getSavegameFilename(const Common::String &target, int num) {
	assert(num >= 0 && num <= 999);

	char extension[5];
	sprintf(extension, "%03d", num);

	return target + "." + extension;
}

} // End of namespace Toltecs
