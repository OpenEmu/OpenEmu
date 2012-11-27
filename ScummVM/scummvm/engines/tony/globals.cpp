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

#include "common/algorithm.h"
#include "tony/globals.h"

namespace Tony {

Globals::Globals() {
	_nextLoop = false;
	_nextChannel = 0;
	_nextSync = 0;
	_curChannel = 0;
	_flipflop = 0;
	_curBackText = NULL;
	_bTonyIsSpeaking = false;
	_curChangedHotspot = 0;
	_tony = NULL;
	_pointer = NULL;
	_boxes = NULL;
	_loc = NULL;
	_inventory = NULL;
	_input = NULL;
	_gfxEngine = NULL;
	EnableGUI = NULL;
	DisableGUI = NULL;

	_dwTonyNumTexts = 0;
	_bTonyInTexts = false;
	_bStaticTalk = false;
	_bAlwaysDisplay = false;
	_bIdleExited = false;
	_bSkipSfxNoLoop = false;
	_bNoBullsEye = false;
	_curDialog = 0;
	_curSoundEffect = 0;
	_bFadeOutStop = false;

	_bSkipIdle = false;
	_hSkipIdle = 0;
	_lastMusic = 0;
	_lastTappeto = 0;
	Common::fill(&_ambiance[0], &_ambiance[200], 0);
	_fullScreenMessageLoc = 0;

	// MPAL global variables
	_mpalError = 0;
	_lpiifCustom = NULL;
	_lplpFunctions = NULL;
	_lplpFunctionStrings = NULL;
	_nObjs = 0;
	_nVars = 0;
	_hVars = NULL;
	_lpmvVars = NULL;
	_nMsgs = 0;
	_hMsgs = NULL;
	_lpmmMsgs = NULL;
	_nDialogs = 0;
	_hDialogs = NULL;
	_lpmdDialogs = NULL;
	_nItems = 0;
	_hItems = NULL;
	_lpmiItems = NULL;
	_nLocations = 0;
	_hLocations = NULL;
	_lpmlLocations = NULL;
	_nScripts = 0;
	_hScripts = NULL;
	_lpmsScripts = NULL;
	_nResources = 0;
	_lpResources = NULL;
	_bExecutingAction = false;
	_bExecutingDialog = false;
	Common::fill(&_nPollingLocations[0], &_nPollingLocations[MAXPOLLINGLOCATIONS], 0);
	Common::fill(&_hEndPollingLocations[0], &_hEndPollingLocations[MAXPOLLINGLOCATIONS], 0);
	Common::fill(&_pollingThreads[0], &_pollingThreads[MAXPOLLINGLOCATIONS], 0);
	_hAskChoice = 0;
	_hDoneChoice = 0;
	_nExecutingAction = 0;
	_nExecutingDialog = 0;
	_nExecutingChoice = 0;
	_nSelectedChoice = 0;
	_nTonyNextTalkType = RMTony::TALK_NORMAL;
	_saveTonyLoc = 0;

	for (int i = 0; i < 16; ++i) {
		Common::fill((byte *)&_character[i], (byte *)&_character[i] + sizeof(CharacterStruct), 0);
		_isMChar[i] = false;
	}

	for (int i = 0; i < 10; ++i)
		Common::fill((byte *)&_mCharacter[i], (byte *)&_mCharacter[i] + sizeof(MCharacterStruct), 0);
	for (int i = 0; i < 256; ++i)
		Common::fill((byte *)&_changedHotspot[i], (byte *)&_changedHotspot[i] + sizeof(ChangedHotspotStruct), 0);

	// Set up globals that have explicit initial values
	_bCfgInvLocked = false;
	_bCfgInvNoScroll = false;
	_bCfgTimerizedText = true;
	_bCfgInvUp = false;
	_bCfgAnni30 = false;
	_bCfgAntiAlias = false;
	_bCfgTransparence = true;
	_bCfgInterTips = true;
	_bShowSubtitles = true;
	_nCfgTonySpeed = 3;
	_nCfgTextSpeed = 5;
	_bCfgDubbing = true;
	_bCfgMusic = true;
	_bCfgSFX = true;
	_nCfgDubbingVolume = 10;
	_nCfgMusicVolume = 7;
	_nCfgSFXVolume = 10;
}

} // End of namespace Tony
