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



#ifdef ENABLE_AGOS2

#include "common/config-manager.h"

#include "agos/intern.h"
#include "agos/agos.h"
#include "agos/animation.h"

namespace AGOS {

AGOSEngine_Feeble::AGOSEngine_Feeble(OSystem *system, const AGOSGameDescription *gd)
	: AGOSEngine_Simon2(system, gd) {

	_interactiveVideo = 0;
	_moviePlayer = 0;
	_vgaCurSpritePriority = 0;
}

AGOSEngine_Feeble::~AGOSEngine_Feeble() {
	delete _moviePlayer;
}

static const GameSpecificSettings feeblefiles_settings = {
	"game22",                               // base_filename
	"save.999",                             // restore_filename
	"tbllist",                              // tbl_filename
	"",                                     // effects_filename
	"VOICES",                               // speech_filename
};

void AGOSEngine_Feeble::setupGame() {
	gss = &feeblefiles_settings;
	_numVideoOpcodes = 85;
	_vgaMemSize = 7500000;
	_itemMemSize = 20000;
	_tableMemSize = 200000;
	_frameCount = 1;
	_vgaBaseDelay = 5;
	_vgaPeriod = 50;
	_numBitArray1 = 16;
	_numBitArray2 = 16;
	_numBitArray3 = 16;
	_numItemStore = 10;
	_numTextBoxes = 40;
	_numVars = 255;

	_numSpeech = 10000;
	_numZone = 450;

	AGOSEngine::setupGame();
}

void AGOSEngine_Feeble::runSubroutine101() {
	if ((getPlatform() == Common::kPlatformAmiga || getPlatform() == Common::kPlatformMacintosh) &&
		getGameType() == GType_FF) {
		playVideo("epic.dxa");
	}

	AGOSEngine::runSubroutine101();
}

void AGOSEngine_Feeble::playVideo(const char *filename, bool lastSceneUsed) {
	if (shouldQuit())
		return;

	if (lastSceneUsed)
		setBitFlag(41, true);

	_moviePlayer = makeMoviePlayer(this, filename);
	assert(_moviePlayer);

	_moviePlayer->load();
	_moviePlayer->play();

	delete _moviePlayer;
	_moviePlayer = NULL;

	if (lastSceneUsed)
		setBitFlag(41, false);
}

void AGOSEngine_Feeble::stopInteractiveVideo() {
	if (_interactiveVideo) {
		_interactiveVideo = 0;
		_moviePlayer->stopVideo();
		delete _moviePlayer;
		_moviePlayer = NULL;
	}
}

AGOSEngine_FeebleDemo::AGOSEngine_FeebleDemo(OSystem *system, const AGOSGameDescription *gd)
	: AGOSEngine_Feeble(system, gd) {

	_filmMenuUsed = 0;
}

Common::Error AGOSEngine_FeebleDemo::go() {
	// Main menu
	defineBox( 1,  80,  75,  81, 117, kBFBoxDead, 0, NULL);
	defineBox( 2, 267,  21, 105,  97, kBFBoxDead, 0, NULL);
	defineBox( 3, 456,  89, 125, 103, kBFBoxDead, 0, NULL);
	defineBox( 4, 151, 225, 345,  41, kBFBoxDead, 0, NULL);
	defineBox( 5, 169, 319, 109, 113, kBFBoxDead, 0, NULL);
	defineBox( 6, 404, 308,  62, 117, kBFBoxDead, 0, NULL);

	// Film menu
	defineBox(11,  28,  81, 123,  93, kBFBoxDead, 0, NULL);
	defineBox(12, 182,  81, 123,  93, kBFBoxDead, 0, NULL);
	defineBox(13, 335,  81, 123,  93, kBFBoxDead, 0, NULL);
	defineBox(14, 488,  81, 123,  93, kBFBoxDead, 0, NULL);
	defineBox(15,  28, 201, 123,  93, kBFBoxDead, 0, NULL);
	defineBox(16, 182, 201, 123,  93, kBFBoxDead, 0, NULL);
	defineBox(17, 335, 201, 123,  93, kBFBoxDead, 0, NULL);
	defineBox(18, 488, 201, 123,  93, kBFBoxDead, 0, NULL);
	defineBox(19, 255, 357, 135,  45, kBFBoxDead, 0, NULL);

	// Exit Menu
	defineBox(21, 548, 421,  42,  21, kBFBoxDead, 0, NULL);

	// Text Window used by Feeble Files Data section
	if (_language ==Common::DE_DEU) {
		_textWindow = openWindow(322, 457, 196, 15, 1, 0, 255);
	} else {
		_textWindow = openWindow(444, 452, 196, 15, 1, 0, 255);
	}

	playVideo("winasoft.smk");
	playVideo("fbigtalk.smk");

	while (!shouldQuit())
		mainMenu();

	return Common::kNoError;
}

void AGOSEngine_FeebleDemo::exitMenu() {
	for (int i = 1; i <= 20; i++)
		disableBox(i);

	enableBox(21);

	playVideo("fhypno.smk");
	playVideo("fbye1.smk", true);

	HitArea *ha;
	do {
		_lastHitArea = NULL;
		_lastHitArea3 = NULL;

		while (!shouldQuit() && _lastHitArea3 == 0) {
			delay(1);
		}

		ha = _lastHitArea;
	} while (!shouldQuit() && !(ha != NULL && ha->id == 21));

	playVideo("fbye2.smk");
	quitGame();
	delay(0);
}

void AGOSEngine_FeebleDemo::filmMenu() {
	for (int i = 1; i <= 6; i++)
		disableBox(i);

	for (int i = 11; i <= 19; i++)
		enableBox(i);

	if (!_filmMenuUsed) {
		playVideo("fclipsin.smk", true);
	} else {
		playVideo("fclipin2.smk", true);
	}

	_filmMenuUsed = true;

	HitArea *ha;
	while (!shouldQuit()) {
		_lastHitArea = NULL;
		_lastHitArea3 = NULL;

		while (!shouldQuit() && _lastHitArea3 == 0) {
			handleWobble();
			delay(1);
		}

		ha = _lastHitArea;

		if (ha == NULL)
			continue;

		stopInteractiveVideo();

		if (ha->id == 11) {
			playVideo("fgo1.smk");
			playVideo("maze.smk");
		} else if (ha->id == 12) {
			playVideo("fgo2.smk");
			playVideo("radioin.smk");
		} else if (ha->id == 13) {
			playVideo("fgo3.smk");
			playVideo("pad.smk");
		} else if (ha->id == 14) {
			playVideo("fgo4.smk");
			playVideo("bridge.smk");
		} else if (ha->id == 15) {
			playVideo("fgo5.smk");
			playVideo("pilldie.smk");
		} else if (ha->id == 16) {
			playVideo("fgo6.smk");
			playVideo("bikebust.smk");
		} else if (ha->id == 17) {
			playVideo("fgo7.smk");
			playVideo("statue.smk");
		} else if (ha->id == 18) {
			playVideo("fgo8.smk");
			playVideo("junkout.smk");
		} else if (ha->id == 19) {
			playVideo("fgo9.smk");
			break;
		}

		playVideo("fclipin2.smk", true);
	}
}

void AGOSEngine_FeebleDemo::handleText() {
	if (_lastClickRem == _currentBox)
		return;

	if (_currentBox && (_currentBox->id >= 1 && _currentBox->id <= 6)) {
		// TODO: Add the subtitles for menu options
	}

	_lastClickRem = _currentBox;
}

void AGOSEngine_FeebleDemo::handleWobble() {
	if (_lastClickRem == _currentBox)
		return;

	stopInteractiveVideo();

	if (_currentBox && (_currentBox->id >= 11 && _currentBox->id <= 19)) {
		char filename[15];
		sprintf(filename, "wobble%d.smk", _currentBox->id - 10);

		startInteractiveVideo(filename);
	}

	_lastClickRem = _currentBox;
}

void AGOSEngine_FeebleDemo::mainMenu() {
	for (int i = 1; i <= 6; i++)
		enableBox(i);

	for (int i = 11; i <= 19; i++)
		disableBox(i);

	playVideo("mmfadein.smk", true);

	startInteractiveVideo("mainmenu.smk");

	HitArea *ha = 0;
	do {
		_lastHitArea = NULL;
		_lastHitArea3 = NULL;

		while (_lastHitArea3 == 0) {
			if (shouldQuit())
				return;
			handleText();
			delay(1);
		}

		ha = _lastHitArea;
	} while (ha == NULL || !(ha->id >= 1 && ha->id <= 6));

	if (shouldQuit())
		return;

	stopInteractiveVideo();

	if (ha->id == 1) {
		// Feeble Files Data
		playVideo("ffade5.smk");
		playVideo("ftext0.smk");
		playVideo("ftext1.smk", true);
		waitForSpace();
		playVideo("ftext2.smk", true);
		waitForSpace();
		playVideo("ftext3.smk", true);
		waitForSpace();
		playVideo("ftext4.smk", true);
		waitForSpace();
		playVideo("ftext5.smk", true);
		waitForSpace();
	} else if (ha->id == 2) {
		// Opening Sequence
		playVideo("ffade1.smk");
		playVideo("musosp1.smk");
		playVideo("newcred.smk");
		playVideo("fasall.smk");
		playVideo("mus5p2.smk");
		playVideo("coach.smk");
		playVideo("outmin.smk");
	} else if (ha->id == 3) {
		// Technical Information
		playVideo("ffade3.smk");
		playVideo("idfx4a.smk");
		playVideo("idfx4b.smk");
		playVideo("idfx4c.smk");
		playVideo("idfx4d.smk");
		playVideo("idfx4e.smk");
		playVideo("idfx4f.smk");
		playVideo("idfx4g.smk");
	} else if (ha->id == 4) {
		// About AdventureSoft
		playVideo("ffade2.smk");
		playVideo("fscene3b.smk");
		playVideo("fscene3a.smk");
		playVideo("fscene3c.smk");
		playVideo("fscene3g.smk");
	} else if (ha->id == 5) {
		// Video Clips
		playVideo("ffade4.smk");
		filmMenu();
	} else if (ha->id == 6) {
		// Exit InfoDisk
		playVideo("ffade6.smk");
		exitMenu();
	}
}

void AGOSEngine_FeebleDemo::startInteractiveVideo(const char *filename) {
	setBitFlag(40, true);
	_interactiveVideo = MoviePlayer::TYPE_LOOPING;
	_moviePlayer = makeMoviePlayer(this, filename);
	assert(_moviePlayer);
	_moviePlayer->load();
	_moviePlayer->play();
	setBitFlag(40, false);
}

void AGOSEngine_FeebleDemo::waitForSpace() {
	const char *message;

	if (_language == Common::DE_DEU) {
		message = "Dr\x81""cken Sie die <Leertaste>, um fortzufahren...";
	} else {
		message = "Press <SPACE> to continue...";
	}

	windowPutChar(_textWindow, 12);
	for (; *message; message++)
		windowPutChar(_textWindow, *message);

	mouseOff();
	do {
		delay(1);
	} while (!shouldQuit() && (_keyPressed.keycode != Common::KEYCODE_SPACE));
	_keyPressed.reset();
	mouseOn();
}

} // End of namespace AGOS

#endif // ENABLE_AGOS2
