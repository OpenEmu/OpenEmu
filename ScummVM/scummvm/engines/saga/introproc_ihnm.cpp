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

// "I Have No Mouth" Intro sequence scene procedures

#include "saga/saga.h"
#include "saga/gfx.h"

#include "saga/animation.h"
#include "saga/events.h"
#include "saga/interface.h"
#include "saga/render.h"
#include "saga/resource.h"
#include "saga/sndres.h"
#include "saga/music.h"

#include "saga/scene.h"

#include "common/events.h"
#include "common/system.h"

namespace Saga {

// IHNM cutaway intro resource IDs
#define RID_IHNM_INTRO_CUTAWAYS 39
#define RID_IHNMDEMO_INTRO_CUTAWAYS 25

int Scene::IHNMStartProc() {
	LoadSceneParams firstScene;

	IHNMLoadCutaways();

	if (!_vm->isIHNMDemo()) {
		int logoLength = -168;

		if (_vm->getLanguage() == Common::DE_DEU || _vm->getLanguage() == Common::ES_ESP)
			logoLength = -128;

		// Play Cyberdreams logo for 168 frames
		if (!playTitle(0, logoLength, true)) {
			if (_vm->shouldQuit())
				return !SUCCESS;
			// Play Dreamers Guild logo for 10 seconds
			if (!playLoopingTitle(1, 10)) {
				if (_vm->shouldQuit())
					return !SUCCESS;
				// Play the title music
				_vm->_music->play(1, MUSIC_NORMAL);
				// Play title screen
				playTitle(2, 17);
			}
		}
	} else {
		_vm->_music->play(1, MUSIC_NORMAL);
		playTitle(0, 10);
		if (_vm->shouldQuit())
			return !SUCCESS;
		playTitle(2, 12);
	}

	_vm->_music->setVolume(0, 1000);
	_vm->_anim->clearCutawayList();

	// Queue first scene
	firstScene.loadFlag = kLoadBySceneNumber;
	firstScene.sceneDescriptor = -1;
	firstScene.sceneSkipTarget = false;
	firstScene.sceneProc = NULL;
	firstScene.transitionType = kTransitionFade;
	firstScene.actorsEntrance = 0;
	firstScene.chapter = -1;

	_vm->_scene->queueScene(firstScene);

	return SUCCESS;
}

int Scene::IHNMCreditsProc() {
	IHNMLoadCutaways();

	_vm->_music->play(0, MUSIC_NORMAL);

	if (!_vm->isIHNMDemo()) {
		// Display the credits for 400 frames
		playTitle(4, -400, true);
	} else {
		// Display sales info for 60 seconds
		playTitle(3, 60, true);
	}

	_vm->_music->setVolume(0, 1000);
	_vm->_anim->clearCutawayList();

	return SUCCESS;
}

void Scene::IHNMLoadCutaways() {
	ResourceContext *resourceContext;
	//ResourceContext *soundContext;
	ByteArray resourceData;

	resourceContext = _vm->_resource->getContext(GAME_RESOURCEFILE);
	if (resourceContext == NULL) {
		error("Scene::IHNMStartProc() resource context not found");
	}

	if (!_vm->isIHNMDemo())
		_vm->_resource->loadResource(resourceContext, RID_IHNM_INTRO_CUTAWAYS, resourceData);
	else
		_vm->_resource->loadResource(resourceContext, RID_IHNMDEMO_INTRO_CUTAWAYS, resourceData);

	if (resourceData.empty()) {
		error("Scene::IHNMStartProc() Can't load cutaway list");
	}

	// Load the cutaways for the title screens
	_vm->_anim->loadCutawayList(resourceData);
}

bool Scene::checkKey() {
	Common::Event event;
	bool res = false;

	while (_vm->_eventMan->pollEvent(event)) {
		switch (event.type) {
		case Common::EVENT_RTL:
		case Common::EVENT_QUIT:
			res = true;
			break;
		case Common::EVENT_KEYDOWN:
			// Don't react to modifier keys alone. The original did
			// non, and the user may want to change scaler without
			// terminating the intro.
			if (event.kbd.ascii)
				res = true;
			break;
		default:
			break;
		}
	}

	return res;
}

bool Scene::playTitle(int title, int time, int mode) {
	bool interrupted = false;
	int startTime = _vm->_system->getMillis();
	int frameTime = 0;
	int curTime;
	int assignedId;
	int phase = 0;
	bool done = false;
	bool playParameter = true;
	static PalEntry cur_pal[PAL_ENTRIES];
	static PalEntry pal_cut[PAL_ENTRIES];
	Surface *backBufferSurface = _vm->_render->getBackGroundSurface();

	// Load the cutaway

	_vm->_anim->setCutAwayMode(mode);
	_vm->_frameCount = 0;

	_vm->_gfx->getCurrentPal(cur_pal);

	assignedId = _vm->_anim->playCutaway(title, false);

	_vm->_gfx->getCurrentPal(pal_cut);

	while (!done && !_vm->shouldQuit()) {
		curTime = _vm->_system->getMillis();

		switch (phase) {
		case 0: // fadeout
		case 1: // fadeout 100%
		case 7: // fadeout
		case 8: // fadeout 100%
			_vm->_gfx->palToBlack(cur_pal, (double)(curTime - startTime) / kNormalFadeDuration);
			// fall through

		case 3: // fadein
		case 4: // fadein 100%
			if (phase == 3 || phase == 4)
				_vm->_gfx->blackToPal(pal_cut, (double)(curTime - startTime) / kNormalFadeDuration);

			if (curTime - startTime > kNormalFadeDuration) {
				phase++;
				if (phase == 2 || phase == 5 || phase == 9)
					startTime = curTime;
				break;
			}
			break;

		case 2: // display background
			_vm->_system->copyRectToScreen(backBufferSurface->pixels, backBufferSurface->w, 0, 0,
							  backBufferSurface->w, backBufferSurface->h);
			phase++;
			startTime = curTime;
			break;

		case 5: // playback
			if (time < 0) {
				if (_vm->_frameCount >= -time) {
					phase++;
					break;
				}
			} else {
				if (curTime - startTime >= time * 1000) {
					phase++;
					break;
				}
			}

			if (checkKey()) {
				_vm->_scene->cutawaySkip();
				interrupted = true;
				phase = 6;	// end playback and fade out
				break;
			}

			if (_vm->_anim->getCycles(assignedId)) { // IHNM demo has 0 frames logo
				if (curTime - frameTime > _vm->_anim->getFrameTime(assignedId)) {
					_vm->_anim->play(assignedId, 0, playParameter);

					if (playParameter == true) // Do not loop animations
						playParameter = false;

					frameTime = curTime;

					_vm->_system->copyRectToScreen(backBufferSurface->pixels, backBufferSurface->w, 0, 0,
							  backBufferSurface->w, backBufferSurface->h);
				}

			}
			break;

		case 6: // playback end
			startTime = curTime;
			_vm->_gfx->getCurrentPal(cur_pal);
			phase++;
			break;

		case 9: // end
			done = true;
			break;
		}

		_vm->_system->updateScreen();
		_vm->_system->delayMillis(10);
	}

	// Clean up

	_vm->_anim->endVideo();

	memset((byte *)backBufferSurface->pixels, 0,  backBufferSurface->w *  backBufferSurface->h);
	_vm->_system->copyRectToScreen(backBufferSurface->pixels, backBufferSurface->w, 0, 0,
							  backBufferSurface->w, backBufferSurface->h);

	return interrupted;
}

bool Scene::playLoopingTitle(int title, int seconds) {
	return playTitle(title, seconds, kPanelCutaway);
}

} // End of namespace Saga

#endif
