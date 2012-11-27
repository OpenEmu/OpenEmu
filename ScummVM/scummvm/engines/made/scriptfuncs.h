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

#ifndef MADE_SCRIPTFUNCS_H
#define MADE_SCRIPTFUNCS_H

#include "made/resource.h"

#include "audio/mixer.h"

#include "common/debug.h"
#include "common/system.h"

namespace Audio {
class PCSpeaker;
}

namespace Made {

class MadeEngine;

typedef Common::Functor2<int16, int16*, int16> ExternalFunc;

class ScriptFunctions {
public:
	ScriptFunctions(MadeEngine *vm);
	virtual ~ScriptFunctions();

	int16 callFunction(uint16 index, int16 argc, int16 *argv)  {
		if (index >= _externalFuncs.size())
			error("ScriptFunctions::callFunction() Invalid function index %d", index);
		debug(4, "%s", _externalFuncNames[index]);
		return (*_externalFuncs[index])(argc, argv);
	}

	void setupExternalsTable();
	const char* getFuncName(int index) { return _externalFuncNames[index]; }
	int getCount() const { return _externalFuncs.size(); }
	void stopSound();

protected:
	MadeEngine *_vm;
	Audio::SoundHandle _audioStreamHandle;
	Audio::SoundHandle _voiceStreamHandle;
	SoundResource* _soundResource;
	bool _soundStarted;

	// PlayNote/StopNote and PlayTele/StopTele wave generators
	Audio::SoundHandle _pcSpeakerHandle1, _pcSpeakerHandle2;
	Audio::PCSpeaker *_pcSpeaker1, *_pcSpeaker2;

	Common::Array<const ExternalFunc *> _externalFuncs;
	Common::Array<const char *> _externalFuncNames;
	GenericResource *_musicRes;

	int16 sfSystemCall(int16 argc, int16 *argv);
	int16 sfInitGraf(int16 argc, int16 *argv);
	int16 sfRestoreGraf(int16 argc, int16 *argv);
	int16 sfDrawPicture(int16 argc, int16 *argv);
	int16 sfClearScreen(int16 argc, int16 *argv);
	int16 sfShowPage(int16 argc, int16 *argv);
	int16 sfPollEvent(int16 argc, int16 *argv);
	int16 sfGetMouseX(int16 argc, int16 *argv);
	int16 sfGetMouseY(int16 argc, int16 *argv);
	int16 sfGetKey(int16 argc, int16 *argv);
	int16 sfSetVisualEffect(int16 argc, int16 *argv);
	int16 sfPlaySound(int16 argc, int16 *argv);
	int16 sfPlayMusic(int16 argc, int16 *argv);
	int16 sfStopMusic(int16 argc, int16 *argv);
	int16 sfIsMusicPlaying(int16 argc, int16 *argv);
	int16 sfSetTextPos(int16 argc, int16 *argv);
	int16 sfFlashScreen(int16 argc, int16 *argv);
	int16 sfPlayNote(int16 argc, int16 *argv);
	int16 sfStopNote(int16 argc, int16 *argv);
	int16 sfPlayTele(int16 argc, int16 *argv);
	int16 sfStopTele(int16 argc, int16 *argv);
	int16 sfHideMouseCursor(int16 argc, int16 *argv);
	int16 sfShowMouseCursor(int16 argc, int16 *argv);
	int16 sfGetMusicBeat(int16 argc, int16 *argv);
	int16 sfSetScreenLock(int16 argc, int16 *argv);
	int16 sfAddSprite(int16 argc, int16 *argv);
	int16 sfFreeAnim(int16 argc, int16 *argv);
	int16 sfDrawSprite(int16 argc, int16 *argv);
	int16 sfEraseSprites(int16 argc, int16 *argv);
	int16 sfUpdateSprites(int16 argc, int16 *argv);
	int16 sfGetTimer(int16 argc, int16 *argv);
	int16 sfSetTimer(int16 argc, int16 *argv);
	int16 sfResetTimer(int16 argc, int16 *argv);
	int16 sfAllocTimer(int16 argc, int16 *argv);
	int16 sfFreeTimer(int16 argc, int16 *argv);
	int16 sfSetPaletteLock(int16 argc, int16 *argv);
	int16 sfSetFont(int16 argc, int16 *argv);
	int16 sfDrawText(int16 argc, int16 *argv);
	int16 sfHomeText(int16 argc, int16 *argv);
	int16 sfSetTextRect(int16 argc, int16 *argv);
	int16 sfSetTextXY(int16 argc, int16 *argv);
	int16 sfSetFontDropShadow(int16 argc, int16 *argv);
	int16 sfSetFontColor(int16 argc, int16 *argv);
	int16 sfSetFontOutline(int16 argc, int16 *argv);
	int16 sfLoadMouseCursor(int16 argc, int16 *argv);
	int16 sfSetSpriteGround(int16 argc, int16 *argv);
	int16 sfLoadResText(int16 argc, int16 *argv);
	int16 sfSetClipArea(int16 argc, int16 *argv);
	int16 sfSetSpriteClip(int16 argc, int16 *argv);
	int16 sfAddScreenMask(int16 argc, int16 *argv);
	int16 sfSetSpriteMask(int16 argc, int16 *argv);
	int16 sfSoundPlaying(int16 argc, int16 *argv);
	int16 sfStopSound(int16 argc, int16 *argv);
	int16 sfPlayVoice(int16 argc, int16 *argv);
	int16 sfPlayCd(int16 argc, int16 *argv);
	int16 sfStopCd(int16 argc, int16 *argv);
	int16 sfGetCdStatus(int16 argc, int16 *argv);
	int16 sfGetCdTime(int16 argc, int16 *argv);
	int16 sfPlayCdSegment(int16 argc, int16 *argv);
	int16 sfPrintf(int16 argc, int16 *argv);
	int16 sfClearMono(int16 argc, int16 *argv);
	int16 sfGetSoundEnergy(int16 argc, int16 *argv);
	int16 sfClearText(int16 argc, int16 *argv);
	int16 sfAnimText(int16 argc, int16 *argv);
	int16 sfGetTextWidth(int16 argc, int16 *argv);
	int16 sfPlayMovie(int16 argc, int16 *argv);
	int16 sfLoadSound(int16 argc, int16 *argv);
	int16 sfLoadMusic(int16 argc, int16 *argv);
	int16 sfLoadPicture(int16 argc, int16 *argv);
	int16 sfSetMusicVolume(int16 argc, int16 *argv);
	int16 sfRestartEvents(int16 argc, int16 *argv);
	int16 sfPlaceSprite(int16 argc, int16 *argv);
	int16 sfPlaceText(int16 argc, int16 *argv);
	int16 sfDeleteChannel(int16 argc, int16 *argv);
	int16 sfGetChannelType(int16 argc, int16 *argv);
	int16 sfSetChannelState(int16 argc, int16 *argv);
	int16 sfSetChannelLocation(int16 argc, int16 *argv);
	int16 sfSetChannelContent(int16 argc, int16 *argv);
	int16 sfSetExcludeArea(int16 argc, int16 *argv);
	int16 sfSetSpriteExclude(int16 argc, int16 *argv);
	int16 sfGetChannelState(int16 argc, int16 *argv);
	int16 sfPlaceAnim(int16 argc, int16 *argv);
	int16 sfSetAnimFrame(int16 argc, int16 *argv);
	int16 sfGetAnimFrame(int16 argc, int16 *argv);
	int16 sfGetAnimFrameCount(int16 argc, int16 *argv);
	int16 sfGetPictureWidth(int16 argc, int16 *argv);
	int16 sfGetPictureHeight(int16 argc, int16 *argv);
	int16 sfSetSoundRate(int16 argc, int16 *argv);
	int16 sfDrawAnimPic(int16 argc, int16 *argv);
	int16 sfLoadAnim(int16 argc, int16 *argv);
	int16 sfReadText(int16 argc, int16 *argv);
	int16 sfReadMenu(int16 argc, int16 *argv);
	int16 sfDrawMenu(int16 argc, int16 *argv);
	int16 sfGetMenuCount(int16 argc, int16 *argv);
	int16 sfSaveGame(int16 argc, int16 *argv);
	int16 sfLoadGame(int16 argc, int16 *argv);
	int16 sfGetGameDescription(int16 argc, int16 *argv);
	int16 sfShakeScreen(int16 argc, int16 *argv);
	int16 sfPlaceMenu(int16 argc, int16 *argv);
	int16 sfSetSoundVolume(int16 argc, int16 *argv);
	int16 sfGetSynthType(int16 argc, int16 *argv);
	int16 sfIsSlowSystem(int16 argc, int16 *argv);

};

} // End of namespace Made

#endif /* MADE_H */
