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

#ifdef ENABLE_IHNM

// Scripting module script function component

#include "saga/saga.h"

#include "saga/gfx.h"
#include "saga/actor.h"
#include "saga/animation.h"
#include "saga/console.h"
#include "saga/events.h"
#include "saga/font.h"
#include "saga/interface.h"
#include "saga/music.h"
#include "saga/itedata.h"
#include "saga/puzzle.h"
#include "saga/render.h"
#include "saga/sound.h"
#include "saga/sndres.h"
#include "saga/resource.h"

#include "saga/script.h"
#include "saga/objectmap.h"

#include "saga/scene.h"
#include "saga/isomap.h"

#include "common/config-manager.h"

namespace Saga {

void Script::setupIHNMScriptFuncList() {
	static const ScriptFunctionDescription IHNMScriptFunctionsList[IHNM_SCRIPT_FUNCTION_MAX] = {
		OPCODE(sfNull),
		OPCODE(sfWait),
		OPCODE(sfTakeObject),
		OPCODE(sfIsCarried),
		OPCODE(sfStatusBar),
		OPCODE(sfMainMode),
		OPCODE(sfScriptWalkTo),
		OPCODE(sfScriptDoAction),
		OPCODE(sfSetActorFacing),
		OPCODE(sfStartBgdAnim),
		OPCODE(sfStopBgdAnim),
		OPCODE(sfLockUser),
		OPCODE(sfPreDialog),
		OPCODE(sfKillActorThreads),
		OPCODE(sfFaceTowards),
		OPCODE(sfSetFollower),
		OPCODE(sfScriptGotoScene),
		OPCODE(sfSetObjImage),
		OPCODE(sfSetObjName),
		OPCODE(sfGetObjImage),
		OPCODE(sfGetNumber),
		OPCODE(sfScriptOpenDoor),
		OPCODE(sfScriptCloseDoor),
		OPCODE(sfSetBgdAnimSpeed),
		OPCODE(sfCycleColors),
		OPCODE(sfDoCenterActor),
		OPCODE(sfStartBgdAnimSpeed),
		OPCODE(sfScriptWalkToAsync),
		OPCODE(sfEnableZone),
		OPCODE(sfSetActorState),
		OPCODE(sfScriptMoveTo),
		OPCODE(sfSceneEq),
		OPCODE(sfDropObject),
		OPCODE(sfFinishBgdAnim),
		OPCODE(sfSwapActors),
		OPCODE(sfSimulSpeech),
		OPCODE(sfScriptWalk),
		OPCODE(sfCycleFrames),
		OPCODE(sfSetFrame),
		OPCODE(sfSetPortrait),
		OPCODE(sfSetProtagPortrait),
		OPCODE(sfChainBgdAnim),
		OPCODE(sfScriptSpecialWalk),
		OPCODE(sfPlaceActor),
		OPCODE(sfCheckUserInterrupt),
		OPCODE(sfScriptWalkRelative),
		OPCODE(sfScriptMoveRelative),
		OPCODE(sfSimulSpeech2),
		OPCODE(sfPsychicProfile),
		OPCODE(sfPsychicProfileOff),
		OPCODE(sfSetProtagState),
		OPCODE(sfResumeBgdAnim),
		OPCODE(sfThrowActor),
		OPCODE(sfWaitWalk),
		OPCODE(sfScriptSceneID),
		OPCODE(sfChangeActorScene),
		OPCODE(sfScriptClimb),
		OPCODE(sfSetDoorState),
		OPCODE(sfSetActorZ),
		OPCODE(sfScriptText),
		OPCODE(sfGetActorX),
		OPCODE(sfGetActorY),
		OPCODE(sfEraseDelta),
		OPCODE(sfPlayMusic),
		OPCODE(sfNull),
		OPCODE(sfEnableEscape),
		OPCODE(sfPlaySound),
		OPCODE(sfPlayLoopedSound),
		OPCODE(sfGetDeltaFrame),
		OPCODE(sfNull),
		OPCODE(sfNull),
		OPCODE(sfRand),
		OPCODE(sfFadeMusic),
		OPCODE(sfNull),
		OPCODE(sfSetChapterPoints),
		OPCODE(sfSetPortraitBgColor),
		OPCODE(sfScriptStartCutAway),
		OPCODE(sfReturnFromCutAway),
		OPCODE(sfEndCutAway),
		OPCODE(sfGetMouseClicks),
		OPCODE(sfResetMouseClicks),
		OPCODE(sfWaitFrames),
		OPCODE(sfScriptFade),
		OPCODE(sfScriptStartVideo),
		OPCODE(sfScriptReturnFromVideo),
		OPCODE(sfScriptEndVideo),
		OPCODE(sfSetActorZ),
		OPCODE(sfShowIHNMDemoHelpBg),
		OPCODE(sfAddIHNMDemoHelpTextLine),
		OPCODE(sfShowIHNMDemoHelpPage),
		OPCODE(sfVstopFX),
		OPCODE(sfVstopLoopedFX),
		OPCODE(sfDemoSetInteractive),	// only used in the demo version of IHNM
		OPCODE(sfDemoIsInteractive),
		OPCODE(sfVsetTrack),
		OPCODE(sfGetPoints),
		OPCODE(sfSetGlobalFlag),
		OPCODE(sfClearGlobalFlag),
		OPCODE(sfTestGlobalFlag),
		OPCODE(sfSetPoints),
		OPCODE(sfSetSpeechBox),
		OPCODE(sfDebugShowData),
		OPCODE(sfWaitFramesEsc),
		OPCODE(sfQueueMusic),
		OPCODE(sfDisableAbortSpeeches)
	};

	_scriptFunctionsList = IHNMScriptFunctionsList;
}

void Script::sfSetChapterPoints(SCRIPTFUNC_PARAMS) {
	int chapter = _vm->_scene->currentChapterNumber();
	_vm->_ethicsPoints[chapter] = thread->pop();
	int16 barometer = thread->pop();
	static PalEntry cur_pal[PAL_ENTRIES];
	PalEntry portraitBgColor = _vm->_interface->_portraitBgColor;
	byte portraitColor = (_vm->getLanguage() == Common::ES_ESP) ? 253 : 254;

	_vm->_spiritualBarometer = _vm->_ethicsPoints[chapter] * 256 / barometer;
	_vm->_scene->setChapterPointsChanged(true);		// don't save this music when saving in IHNM

	// Set the portrait bg color, in case a saved state is restored from the
	// launcher. In this case, sfSetPortraitBgColor is not called, thus the
	// portrait color will always be 0 (black).
	if (portraitBgColor.red == 0 && portraitBgColor.green == 0 && portraitBgColor.blue == 0)
		portraitBgColor.green = 255;

	if (_vm->_spiritualBarometer > 255)
		_vm->_gfx->setPaletteColor(portraitColor, 0xff, 0xff, 0xff);
	else
		_vm->_gfx->setPaletteColor(portraitColor,
			_vm->_spiritualBarometer * portraitBgColor.red / 256,
			_vm->_spiritualBarometer * portraitBgColor.green / 256,
			_vm->_spiritualBarometer * portraitBgColor.blue / 256);

	_vm->_gfx->getCurrentPal(cur_pal);
	_vm->_gfx->setPalette(cur_pal);
}

void Script::sfSetPortraitBgColor(SCRIPTFUNC_PARAMS) {
	int16 red = thread->pop();
	int16 green = thread->pop();
	int16 blue = thread->pop();

	_vm->_interface->setPortraitBgColor(red, green, blue);
}

void Script::sfScriptStartCutAway(SCRIPTFUNC_PARAMS) {
	int16 cut = thread->pop();
	thread->pop();		// Not used
	int16 fade = thread->pop();

	_vm->_anim->setCutAwayMode(kPanelCutaway);
	_vm->_anim->playCutaway(cut, fade != 0);
}

void Script::sfReturnFromCutAway(SCRIPTFUNC_PARAMS) {
	_vm->_anim->returnFromCutaway();
	thread->wait(kWaitTypeWakeUp);
}

void Script::sfEndCutAway(SCRIPTFUNC_PARAMS) {
	_vm->_anim->endCutaway();
}

void Script::sfGetMouseClicks(SCRIPTFUNC_PARAMS) {
	thread->_returnValue = _vm->getMouseClickCount();
}

void Script::sfResetMouseClicks(SCRIPTFUNC_PARAMS) {
	_vm->resetMouseClickCount();
}

void Script::sfWaitFrames(SCRIPTFUNC_PARAMS) {
	int16 frames = thread->pop();

	if (!_skipSpeeches)
		thread->waitFrames(_vm->_frameCount + frames);
}

void Script::sfScriptFade(SCRIPTFUNC_PARAMS) {
	int16 firstPalEntry = thread->pop();
	int16 lastPalEntry = thread->pop();
	int16 startingBrightness = thread->pop();
	int16 endingBrightness = thread->pop();
	Event event;
	static PalEntry cur_pal[PAL_ENTRIES];

	_vm->_gfx->getCurrentPal(cur_pal);
	event.type = kEvTImmediate;
	event.code = kPalEvent;
	event.op = kEventPalFade;
	event.time = 0;
	event.duration = kNormalFadeDuration;
	event.data = cur_pal;
	event.param = startingBrightness;
	event.param2 = endingBrightness;
	event.param3 = firstPalEntry;
	event.param4 = lastPalEntry - firstPalEntry + 1;
	_vm->_events->queue(event);
}

void Script::sfScriptStartVideo(SCRIPTFUNC_PARAMS) {
	int16 vid = thread->pop();
	int16 fade = thread->pop();

	_vm->_anim->setCutAwayMode(kPanelVideo);
	_vm->_anim->startVideo(vid, fade != 0);
}

void Script::sfScriptReturnFromVideo(SCRIPTFUNC_PARAMS) {
	_vm->_anim->returnFromVideo();
}

void Script::sfScriptEndVideo(SCRIPTFUNC_PARAMS) {
	_vm->_anim->endVideo();
}

void Script::sfShowIHNMDemoHelpBg(SCRIPTFUNC_PARAMS) {
	_ihnmDemoCurrentY = 0;
	_vm->_scene->_textList.clear();
	_vm->_interface->setMode(kPanelConverse);
	_vm->_scene->showPsychicProfile(NULL);
}

void Script::sfAddIHNMDemoHelpTextLine(SCRIPTFUNC_PARAMS) {
	int stringId = thread->pop();
	TextListEntry textEntry;
	Event event;

	textEntry.knownColor = kKnownColorBlack;
	textEntry.useRect = true;
	textEntry.rect.left = 245;
	textEntry.rect.setHeight(210 + 76);
	textEntry.rect.setWidth(226);
	textEntry.rect.top = 76 + _ihnmDemoCurrentY;
	textEntry.font = kKnownFontVerb;
	textEntry.flags = (FontEffectFlags)(kFontCentered);
	textEntry.text = thread->_strings->getString(stringId);

	TextListEntry *_psychicProfileTextEntry = _vm->_scene->_textList.addEntry(textEntry);

	event.type = kEvTOneshot;
	event.code = kTextEvent;
	event.op = kEventDisplay;
	event.data = _psychicProfileTextEntry;
	_vm->_events->queue(event);

	_ihnmDemoCurrentY += _vm->_font->getHeight(kKnownFontVerb, thread->_strings->getString(stringId), 226, kFontCentered);
}

void Script::sfShowIHNMDemoHelpPage(SCRIPTFUNC_PARAMS) {
	// Note: The IHNM demo changes panel mode to 8 (kPanelProtect in ITE)
	// when changing pages
	_vm->_interface->setMode(kPanelPlacard);
	_ihnmDemoCurrentY = 0;
}

void Script::sfVstopFX(SCRIPTFUNC_PARAMS) {
	_vm->_sound->stopSound();
}

void Script::sfVstopLoopedFX(SCRIPTFUNC_PARAMS) {
	_vm->_sound->stopSound();
}

void Script::sfDemoSetInteractive(SCRIPTFUNC_PARAMS) {
	if (thread->pop() == 0) {
		_vm->_interface->deactivate();
		_vm->_interface->setMode(kPanelNull);
	}

	// Note: the original also sets an appropriate flag here, but we don't,
	// as we don't use it
}

void Script::sfDemoIsInteractive(SCRIPTFUNC_PARAMS) {
	thread->_returnValue = 0;
}

void Script::sfVsetTrack(SCRIPTFUNC_PARAMS) {
	int16 chapter = thread->pop();
	int16 sceneNumber = thread->pop();
	int16 actorsEntrance = thread->pop();

	debug(2, "sfVsetTrrack(%d, %d, %d)", chapter, sceneNumber, actorsEntrance);

	_vm->_scene->changeScene(sceneNumber, actorsEntrance, kTransitionFade, chapter);
}

void Script::sfGetPoints(SCRIPTFUNC_PARAMS) {
	int16 index = thread->pop();

	if (index >= 0 && index < ARRAYSIZE(_vm->_ethicsPoints))
		thread->_returnValue = _vm->_ethicsPoints[index];
	else
		thread->_returnValue = 0;
}

void Script::sfSetGlobalFlag(SCRIPTFUNC_PARAMS) {
	int16 flag = thread->pop();

	if (flag >= 0 && flag < 32)
		_vm->_globalFlags |= (1 << flag);
}

void Script::sfClearGlobalFlag(SCRIPTFUNC_PARAMS) {
	int16 flag = thread->pop();

	if (flag >= 0 && flag < 32)
		_vm->_globalFlags &= ~(1 << flag);
}

void Script::sfTestGlobalFlag(SCRIPTFUNC_PARAMS) {
	int16 flag = thread->pop();

	if (flag >= 0 && flag < 32 && _vm->_globalFlags & (1 << flag))
		thread->_returnValue = 1;
	else
		thread->_returnValue = 0;
}

void Script::sfSetPoints(SCRIPTFUNC_PARAMS) {
	int16 index = thread->pop();
	int16 points = thread->pop();

	if (index >= 0 && index < ARRAYSIZE(_vm->_ethicsPoints))
		_vm->_ethicsPoints[index] = points;
}

void Script::sfSetSpeechBox(SCRIPTFUNC_PARAMS) {
	int16 param1 = thread->pop();
	int16 param2 = thread->pop();
	int16 param3 = thread->pop();
	int16 param4 = thread->pop();

	_vm->_actor->_speechBoxScript.left = param1;
	_vm->_actor->_speechBoxScript.top = param2;
	_vm->_actor->_speechBoxScript.setWidth(param3 - param1);
	_vm->_actor->_speechBoxScript.setHeight(param4 - param2);
}

void Script::sfDebugShowData(SCRIPTFUNC_PARAMS) {
	int16 param = thread->pop();

	Common::String buf = Common::String::format("Reached breakpoint %d", param);

	_vm->_interface->setStatusText(buf.c_str());
}

void Script::sfWaitFramesEsc(SCRIPTFUNC_PARAMS) {
	thread->_returnValue = _vm->_framesEsc;
}

void Script::sfQueueMusic(SCRIPTFUNC_PARAMS) {
	int16 param1 = thread->pop();
	int16 param2 = thread->pop();
	Event event;

	if (param1 < 0) {
		_vm->_music->stop();
		return;
	}

	if (uint(param1) >= _vm->_music->_songTable.size()) {
		warning("sfQueueMusic: Wrong song number (%d > %d)", param1, _vm->_music->_songTable.size() - 1);
	} else {
		_vm->_music->setVolume(_vm->_musicVolume, 1);
		event.type = kEvTOneshot;
		event.code = kMusicEvent;
		event.param = _vm->_music->_songTable[param1];
		event.param2 = param2 ? MUSIC_LOOP : MUSIC_NORMAL;
		event.op = kEventPlay;
		event.time = _vm->ticksToMSec(1000);

		_vm->_events->queue(event);

		if (!_vm->_scene->haveChapterPointsChanged()) {
			_vm->_scene->setCurrentMusicTrack(param1);
			_vm->_scene->setCurrentMusicRepeat(param2);
		} else {
			// Don't save this music track when saving in IHNM
			_vm->_scene->setChapterPointsChanged(false);
		}
	}
}

void Script::sfDisableAbortSpeeches(SCRIPTFUNC_PARAMS) {
	_vm->_interface->disableAbortSpeeches(thread->pop() != 0);
}

void Script::sfPsychicProfile(SCRIPTFUNC_PARAMS) {
	thread->wait(kWaitTypePlacard);

	_vm->_scene->showPsychicProfile(thread->_strings->getString(thread->pop()));
}

void Script::sfPsychicProfileOff(SCRIPTFUNC_PARAMS) {
	// This is called a while after the psychic profile is
	// opened, to close it automatically
	_vm->_scene->clearPsychicProfile();
}

} // End of namespace Saga

#endif
