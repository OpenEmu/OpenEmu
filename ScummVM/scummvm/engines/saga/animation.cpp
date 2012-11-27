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

// Background animation management module

#include "saga/saga.h"
#include "saga/gfx.h"

#include "saga/console.h"
#include "saga/events.h"
#include "saga/interface.h"
#include "saga/render.h"
#include "saga/resource.h"
#include "saga/scene.h"

#include "saga/animation.h"

namespace Saga {

Anim::Anim(SagaEngine *vm) : _vm(vm) {
	uint16 i;

	_cutawayActive = false;

	for (i = 0; i < MAX_ANIMATIONS; i++)
		_animations[i] = NULL;

	for (i = 0; i < ARRAYSIZE(_cutawayAnimations); i++)
		_cutawayAnimations[i] = NULL;
}

Anim::~Anim() {
	reset();
}

#ifdef ENABLE_IHNM

void Anim::loadCutawayList(const ByteArray &resourceData) {
	_cutawayList.resize(resourceData.size() / 8);

	ByteArrayReadStreamEndian cutawayS(resourceData);

	for (uint i = 0; i < _cutawayList.size(); i++) {
		_cutawayList[i].backgroundResourceId = cutawayS.readUint16LE();
		_cutawayList[i].animResourceId = cutawayS.readUint16LE();
		_cutawayList[i].cycles = cutawayS.readSint16LE();
		_cutawayList[i].frameRate = cutawayS.readSint16LE();
	}
}

void Anim::clearCutawayList() {
	_cutawayList.clear();
}

int Anim::playCutaway(int cut, bool fade) {
	debug(0, "playCutaway(%d, %d)", cut, fade);

	Event event;
	EventColumns *eventColumns = NULL;
	bool startImmediately = false;
	ResourceContext *context = _vm->_resource->getContext(GAME_RESOURCEFILE);

	_cutAwayFade = fade;

	_vm->_gfx->savePalette();
	_vm->_gfx->getCurrentPal(saved_pal);

	if (fade) {
		static PalEntry cur_pal[PAL_ENTRIES];

		_vm->_interface->setFadeMode(kFadeOut);

		// Fade to black out
		_vm->_gfx->getCurrentPal(cur_pal);
		event.type = kEvTImmediate;
		event.code = kPalEvent;
		event.op = kEventPalToBlack;
		event.time = 0;
		event.duration = kNormalFadeDuration;
		event.data = cur_pal;
		eventColumns = _vm->_events->queue(event);

		// set fade mode
		event.type = kEvTImmediate;
		event.code = kInterfaceEvent;
		event.op = kEventSetFadeMode;
		event.param = kNoFade;
		event.time = 0;
		event.duration = 0;
		_vm->_events->chain(eventColumns, event);
	}

	// Prepare cutaway
	if (!_cutawayActive) {
		_vm->_gfx->showCursor(false);
		_vm->_interface->setStatusText("");
		_vm->_interface->setSaveReminderState(0);
		_vm->_interface->rememberMode();
		_cutawayActive = true;
	} else {
		// If another cutaway is up, start the next cutaway immediately
		startImmediately = true;
	}

	if (_cutAwayMode == kPanelVideo)
		_vm->_interface->setMode(kPanelVideo);
	else
		_vm->_interface->setMode(kPanelCutaway);

	if (fade) {
		// Set the initial background and palette for the cutaway
		event.type = kEvTImmediate;
		event.code = kCutawayEvent;
		event.op = kEventShowCutawayBg;
		event.time = 0;
		event.duration = 0;
		event.param = _cutawayList[cut].backgroundResourceId;
		eventColumns = _vm->_events->chain(eventColumns, event);
	} else {
		showCutawayBg(_cutawayList[cut].backgroundResourceId);
	}

	// Play the animation

	int cutawaySlot = -1;

	for (int i = 0; i < ARRAYSIZE(_cutawayAnimations); i++) {
		if (!_cutawayAnimations[i]) {
			cutawaySlot = i;
		} else if (_cutawayAnimations[i]->state == ANIM_PAUSE) {
			delete _cutawayAnimations[i];
			_cutawayAnimations[i] = NULL;
			cutawaySlot = i;
		} else if (_cutawayAnimations[i]->state == ANIM_PLAYING) {
			_cutawayAnimations[i]->state = ANIM_PAUSE;
		}
	}

	if (cutawaySlot == -1) {
		warning("Could not allocate cutaway animation slot");
		return 0;
	}

	// Some cutaways in IHNM have animResourceId equal to 0, which means that they only have
	// a background frame and no animation. Those animations are actually game scripts.
	// An example is the "nightfall" animation in Ben's chapter (fadein-fadeout), the animation
	// for the second from the left monitor in Ellen's chapter etc
	// Therefore, skip the animation bit if animResourceId is 0 and only show the background
	if (_cutawayList[cut].animResourceId != 0) {
		ByteArray resourceData;
		_vm->_resource->loadResource(context, _cutawayList[cut].animResourceId, resourceData);
		load(MAX_ANIMATIONS + cutawaySlot, resourceData);


		setCycles(MAX_ANIMATIONS + cutawaySlot, _cutawayList[cut].cycles);
		setFrameTime(MAX_ANIMATIONS + cutawaySlot, 1000 / _cutawayList[cut].frameRate);
	}

	if (_cutAwayMode != kPanelVideo || startImmediately)
		play(MAX_ANIMATIONS + cutawaySlot, 0);
	else {
		event.type = kEvTOneshot;
		event.code = kAnimEvent;
		event.op = kEventPlay;
		event.param = MAX_ANIMATIONS + cutawaySlot;
		event.time = (40 / 3) * 1000 / _cutawayList[cut].frameRate;

		if (fade)
			eventColumns = _vm->_events->chain(eventColumns, event);
		else
			_vm->_events->queue(event);
	}

	return MAX_ANIMATIONS + cutawaySlot;
}

void Anim::endCutaway() {
	// This is called by scripts after a cutaway is finished. At this time,
	// nothing needs to be done here.

	debug(0, "endCutaway()");
}

void Anim::returnFromCutaway() {
	// This is called by scripts after a cutaway is finished, to return back
	// to the scene that the cutaway was called from. It's not called by the
	// IHNM intro, as there is no old scene to return to.

	debug(0, "returnFromCutaway()");

	if (_cutawayActive) {
		Event event;
		EventColumns *eventColumns = NULL;

		if (_cutAwayFade) {
			static PalEntry cur_pal[PAL_ENTRIES];

			_vm->_interface->setFadeMode(kFadeOut);

			// Fade to black out
			_vm->_gfx->getCurrentPal(cur_pal);
			event.type = kEvTImmediate;
			event.code = kPalEvent;
			event.op = kEventPalToBlack;
			event.time = 0;
			event.duration = kNormalFadeDuration;
			event.data = cur_pal;
			eventColumns = _vm->_events->queue(event);

			// set fade mode
			event.type = kEvTImmediate;
			event.code = kInterfaceEvent;
			event.op = kEventSetFadeMode;
			event.param = kNoFade;
			event.time = 0;
			event.duration = 0;
			_vm->_events->chain(eventColumns, event);
		}

		// Clear the cutaway. Note that this sets _cutawayActive to false
		event.type = kEvTImmediate;
		event.code = kCutawayEvent;
		event.op = kEventClear;
		event.time = 0;
		event.duration = 0;

		if (_cutAwayFade)
			eventColumns = _vm->_events->chain(eventColumns, event);		// chain with the other events
		else
			eventColumns = _vm->_events->queue(event);

		_vm->_scene->restoreScene();

		// WORKAROUND: Restart all scene animations before restoring them below
		// This is mostly needed so that the animation of the mob of prisoners
		// in Nimdok's chapter is shown correctly after the cutaway where the
		// prisoner drops the jar on the ground
		for (int i = 0; i < MAX_ANIMATIONS; i++) {
			if (_animations[i] && _animations[i]->state == ANIM_PLAYING) {
				_animations[i]->currentFrame = -1;	// start from -1 (check Anim::load())
			}
		}

		// Restore the animations
		event.type = kEvTImmediate;
		event.code = kAnimEvent;
		event.op = kEventResumeAll;
		event.time = 0;
		event.duration = 0;
		_vm->_events->chain(eventColumns, event);		// chain with the other events

		// Draw the scene
		event.type = kEvTImmediate;
		event.code = kSceneEvent;
		event.op = kEventDraw;
		event.time = 0;
		event.duration = 0;
		_vm->_events->chain(eventColumns, event);		// chain with the other events

		// Handle fade up, if we previously faded down
		if (_cutAwayFade) {
			event.type = kEvTImmediate;
			event.code = kPalEvent;
			event.op = kEventBlackToPal;
			event.time = 0;
			event.duration = kNormalFadeDuration;
			event.data = saved_pal;
			_vm->_events->chain(eventColumns, event);
		}

		event.type = kEvTOneshot;
		event.code = kScriptEvent;
		event.op = kEventThreadWake;
		event.param = kWaitTypeWakeUp;
		_vm->_events->chain(eventColumns, event);
	}
}

void Anim::clearCutaway() {
	PalEntry *pal;

	debug(1, "clearCutaway()");

	if (_cutawayActive) {
		_cutawayActive = false;

		for (int i = 0; i < ARRAYSIZE(_cutawayAnimations); i++) {
			delete _cutawayAnimations[i];
			_cutawayAnimations[i] = NULL;
		}

		_vm->_interface->restoreMode();
		_vm->_gfx->showCursor(true);

		if (_vm->isIHNMDemo()) {
			// Enable the save reminder state after each cutaway for the IHNM demo
			_vm->_interface->setSaveReminderState(true);
		}

		// Set the scene's palette
		_vm->_scene->getBGPal(pal);
		_vm->_gfx->setPalette(pal);
	}
}

void Anim::showCutawayBg(int bg) {
	ResourceContext *context = _vm->_resource->getContext(GAME_RESOURCEFILE);

	ByteArray resourceData;
	ByteArray image;
	int width;
	int height;
	Event event;
	static PalEntry pal[PAL_ENTRIES];

	_vm->_resource->loadResource(context, bg, resourceData);
	_vm->decodeBGImage(resourceData, image, &width, &height);

	const byte *palPointer = _vm->getImagePal(resourceData);
	memcpy(pal, palPointer, sizeof(pal));
	const Rect rect(width, height);
	_vm->_render->getBackGroundSurface()->blit(rect, image.getBuffer());
	_vm->_render->setFullRefresh(true);
	_vm->_frameCount++;

	if (_cutAwayFade) {
		// Handle fade up, if we previously faded down
		event.type = kEvTImmediate;
		event.code = kPalEvent;
		event.op = kEventBlackToPal;
		event.time = 0;
		event.duration = kNormalFadeDuration;
		event.data = pal;
		_vm->_events->queue(event);
	} else {
		_vm->_gfx->setPalette(pal);
	}
}

void Anim::startVideo(int vid, bool fade) {
	debug(0, "startVideo(%d, %d)", vid, fade);

	_vm->_interface->setStatusText("");
	_vm->_frameCount = 0;

	playCutaway(vid, fade);
}

void Anim::endVideo() {
	debug(0, "endVideo()");

	clearCutaway();
}

void Anim::returnFromVideo() {
	debug(0, "returnFromVideo()");

	returnFromCutaway();
}

#endif

void Anim::load(uint16 animId, const ByteArray &resourceData) {
	AnimationData *anim;
	uint16 temp;

	if (animId >= MAX_ANIMATIONS) {
		if (animId >= MAX_ANIMATIONS + ARRAYSIZE(_cutawayAnimations))
			error("Anim::load could not find unused animation slot");
		anim = _cutawayAnimations[animId - MAX_ANIMATIONS] = new AnimationData();
	} else
		anim = _animations[animId] = new AnimationData();

	ByteArrayReadStreamEndian headerReadS(resourceData, _vm->isBigEndian());
	anim->magic = headerReadS.readUint16LE(); // cause ALWAYS LE
	anim->screenWidth = headerReadS.readUint16();
	anim->screenHeight = headerReadS.readUint16();

	anim->unknown06 = headerReadS.readByte();
	anim->unknown07 = headerReadS.readByte();
	anim->maxFrame = headerReadS.readByte() - 1;
	anim->loopFrame = headerReadS.readByte() - 1;
	temp = headerReadS.readUint16BE();
	size_t start;

	start = headerReadS.pos();
	if (temp == (uint16)(-1)) {
		temp = 0;
	}
	start += temp;

	size_t dataOffset = headerReadS.pos();
	if (dataOffset != start) {
		warning("Anim::load animId=%d start != dataOffset 0x%X 0x%X", animId, uint(start), uint(dataOffset));
	}

	anim->resourceData.resize(resourceData.size() - dataOffset);

	memcpy(anim->resourceData.getBuffer(), resourceData.getBuffer() + dataOffset, anim->resourceData.size());
	// Cache frame offsets

	// WORKAROUND: Cutaway with background resource ID 37 (loaded as cutaway #4) is ending credits.
	// For some reason it has wrong number of frames specified in its header. So we calculate it here:
	if (animId > MAX_ANIMATIONS && _cutawayList.size() > 4 && _cutawayList[4].backgroundResourceId == 37 && anim->maxFrame == 143)
		anim->maxFrame = fillFrameOffsets(anim, false);

	anim->frameOffsets.resize(anim->maxFrame + 1);

	fillFrameOffsets(anim);

	// Set animation data
	// HACK: We set currentFrame to -1, as the first frame of the animation is never drawn otherwise
	// (check Anim::play)
	anim->currentFrame = -1;
	anim->completed = 0;
	anim->cycles = anim->maxFrame;

	anim->frameTime = DEFAULT_FRAME_TIME;
	anim->flags = ANIM_FLAG_NONE;
	anim->linkId = -1;
	anim->state = ANIM_PAUSE;
}

void Anim::link(int16 animId1, int16 animId2) {
	AnimationData *anim1;
	AnimationData *anim2;

	anim1 = getAnimation(animId1);

	anim1->linkId = animId2;

	if (animId2 == -1) {
		return;
	}

	anim2 = getAnimation(animId2);
	anim2->frameTime = anim1->frameTime;
}

void Anim::setCycles(uint16 animId, int cycles) {
	getAnimation(animId)->cycles = cycles;
}

int Anim::getCycles(uint16 animId) {
	if (animId >= MAX_ANIMATIONS && _cutawayAnimations[animId - MAX_ANIMATIONS] == NULL)
		return 0;

	return getAnimation(animId)->cycles;
}

void Anim::play(uint16 animId, int vectorTime, bool playing) {
	Event event;
	byte *displayBuffer;

	uint16 frame;
	int frameTime;

	AnimationData *anim;
	AnimationData *linkAnim;

	if (animId > MAX_ANIMATIONS && !_cutawayActive)
		return;

	if (animId < MAX_ANIMATIONS && _cutawayActive)
		return;

	if (animId >= MAX_ANIMATIONS && _cutawayAnimations[animId - MAX_ANIMATIONS] == NULL) {
		// In IHNM, cutaways without an animation bit are not rendered, but the framecount
		// needs to be updated
		_vm->_frameCount++;

		event.type = kEvTOneshot;
		event.code = kAnimEvent;
		event.op = kEventFrame;
		event.param = animId;
		event.time = 10;
		_vm->_events->queue(event);

		// Nothing to render here (apart from the background, which is already rendered),
		// so return
		return;
	}

	anim = getAnimation(animId);
	displayBuffer = (byte *)_vm->_render->getBackGroundSurface()->pixels;

	if (playing) {
		anim->state = ANIM_PLAYING;
	}

	if (anim->state == ANIM_PAUSE) {
		return;
	}

	// HACK: The first frame of the animation is never shown when entering a scene
	// For now, we initialize currentFrame to be -1 instead of 0 and we draw the
	// first frame of the animation on the next iteration of Anim::play
	// FIXME: find out why this occurs and remove this hack. Note that when this
	// hack is removed, currentFrame should be initialized to 0 again in Anim::load
	if (anim->currentFrame < 0) {
		anim->currentFrame = 0;
		event.type = kEvTOneshot;
		event.code = kAnimEvent;
		event.op = kEventFrame;
		event.param = animId;
		event.time = 0;
		_vm->_events->queue(event);

		return;
	}

	if (anim->completed < anim->cycles) {
		if (anim->currentFrame < 0)
			anim->currentFrame = 0;

		frame = anim->currentFrame;

		// FIXME: if start > 0, then this works incorrectly
		decodeFrame(anim, anim->frameOffsets[frame], displayBuffer, _vm->getDisplayInfo().width * _vm->getDisplayInfo().height);
		_vm->_render->addDirtyRect(Common::Rect(0, 0, _vm->getDisplayInfo().width, _vm->getDisplayInfo().height));
		_vm->_frameCount++;
		anim->currentFrame++;
		if (anim->completed != 65535) {
			anim->completed++;
		}

		if (anim->currentFrame > anim->maxFrame) {

			anim->currentFrame = anim->loopFrame;
			_vm->_frameCount++;

			if (anim->state == ANIM_STOPPING || anim->currentFrame == -1) {
				anim->state = ANIM_PAUSE;
			}
		}
	} else {
		_vm->_frameCount += 100;	// make sure the waiting thread stops waiting
		// Animation done playing
		anim->state = ANIM_PAUSE;

		if (anim->linkId == -1) {
			if (anim->flags & ANIM_FLAG_ENDSCENE) {
				// This animation ends the scene
				event.type = kEvTOneshot;
				event.code = kSceneEvent;
				event.op = kEventEnd;
				event.time = anim->frameTime + vectorTime;
				_vm->_events->queue(event);
			}
			return;
		} else {
			anim->currentFrame = 0;
			anim->completed = 0;
		}
	}

	if (anim->state == ANIM_PAUSE && anim->linkId != -1) {
		// If this animation has a link, follow it
		linkAnim = getAnimation(anim->linkId);

		debug(5, "Animation ended going to %d", anim->linkId);
		linkAnim->state = ANIM_PLAYING;
		animId = anim->linkId;
		frameTime = 0;
	} else {
		frameTime = anim->frameTime + vectorTime;
	}

	event.type = kEvTOneshot;
	event.code = kAnimEvent;
	event.op = kEventFrame;
	event.param = animId;
	event.time = frameTime;
	_vm->_events->queue(event);
}

void Anim::stop(uint16 animId) {
	getAnimation(animId)->state = ANIM_PAUSE;
}

void Anim::finish(uint16 animId) {
	getAnimation(animId)->state = ANIM_STOPPING;
}

void Anim::resume(uint16 animId, int cycles) {
	getAnimation(animId)->cycles += cycles;
	play(animId, 0, true);
}

void Anim::reset() {
	uint16 i;

	for (i = 0; i < MAX_ANIMATIONS; i++) {
		if (_animations[i] != NULL) {
			delete _animations[i];
			_animations[i] = NULL;
		}
	}

	for (i = 0; i < ARRAYSIZE(_cutawayAnimations); i++) {
		if (_cutawayAnimations[i] != NULL) {
			delete _cutawayAnimations[i];
			_cutawayAnimations[i] = NULL;
		}
	}
}

void Anim::setFlag(uint16 animId, uint16 flag) {
	getAnimation(animId)->flags |= flag;
}

void Anim::clearFlag(uint16 animId, uint16 flag) {
	getAnimation(animId)->flags &= ~flag;
}

void Anim::setFrameTime(uint16 animId, int time) {
	getAnimation(animId)->frameTime = time;
}

int Anim::getFrameTime(uint16 animId) {
	return getAnimation(animId)->frameTime;
}

bool Anim::isPlaying(uint16 animId) {
	return (getAnimation(animId)->state == ANIM_PLAYING);
}

int16 Anim::getCurrentFrame(uint16 animId) {
	return getAnimation(animId)->currentFrame;
}

void Anim::decodeFrame(AnimationData *anim, size_t frameOffset, byte *buf, size_t bufLength) {
	byte *writePointer = NULL;

	uint16 xStart = 0;
	uint16 yStart = 0;
	uint32 screenWidth;
	uint32 screenHeight;

	int markByte;
	byte dataByte;
	int newRow;

	uint16 controlChar;
	uint16 paramChar;

	uint16 runcount;
	int xVector;

	uint16 i;
	bool longData = isLongData();

	screenWidth = anim->screenWidth;
	screenHeight = anim->screenHeight;

	if ((screenWidth * screenHeight) > bufLength) {
		// Buffer argument is too small to hold decoded frame, abort.
		error("decodeFrame() Buffer size inadequate");
	}

	Common::MemoryReadStream readS(&anim->resourceData[frameOffset], anim->resourceData.size() - frameOffset);

// FIXME: This is thrown when the first video of the IHNM end sequence is shown (the "turn off screen"
// video), however the video is played correctly and the rest of the end sequence continues normally
#if 1
#define VALIDATE_WRITE_POINTER \
	if ((writePointer < buf) || (writePointer >= (buf + screenWidth * screenHeight))) { \
		warning("VALIDATE_WRITE_POINTER: writePointer=%p buf=%p", (void *)writePointer, (void *)buf); \
	}
#else
#define VALIDATE_WRITE_POINTER
#endif


	// Begin RLE decompression to output buffer
	do {
		markByte = readS.readByte();
		switch (markByte) {
		case SAGA_FRAME_START:
			xStart = readS.readUint16BE();
			if (longData)
				yStart = readS.readUint16BE();
			else
				yStart = readS.readByte();
			readS.readByte();		// Skip pad byte
			/*xPos = */readS.readUint16BE();
			/*yPos = */readS.readUint16BE();
			/*width = */readS.readUint16BE();
			/*height = */readS.readUint16BE();

			// Setup write pointer to the draw origin
			writePointer = (buf + (yStart * screenWidth) + xStart);
			VALIDATE_WRITE_POINTER;
			continue;
			break;
		case SAGA_FRAME_NOOP: // Does nothing
			readS.readByte();
			readS.readByte();
			readS.readByte();
			continue;
			break;
		case SAGA_FRAME_LONG_UNCOMPRESSED_RUN: // Long Unencoded Run
			runcount = readS.readSint16BE();
			for (i = 0; i < runcount; i++) {
				dataByte = readS.readByte();
				if (dataByte != 0) {
					*writePointer = dataByte;
				}
				writePointer++;
				VALIDATE_WRITE_POINTER;
			}
			continue;
			break;
		case SAGA_FRAME_LONG_COMPRESSED_RUN: // Long encoded run
			runcount = readS.readSint16BE();
			dataByte = readS.readByte();
			for (i = 0; i < runcount; i++) {
				*writePointer++ = dataByte;
				VALIDATE_WRITE_POINTER;
			}
			continue;
			break;
		case SAGA_FRAME_ROW_END: // End of row
			xVector = readS.readSint16BE();

			if (longData)
				newRow = readS.readSint16BE();
			else
				newRow = readS.readByte();

			// Set write pointer to the new draw origin
			writePointer = buf + ((yStart + newRow) * screenWidth) + xStart + xVector;
			VALIDATE_WRITE_POINTER;
			continue;
			break;
		case SAGA_FRAME_REPOSITION: // Reposition command
			xVector = readS.readSint16BE();
			writePointer += xVector;
			VALIDATE_WRITE_POINTER;
			continue;
			break;
		case SAGA_FRAME_END: // End of frame marker
			return;
		default:
			break;
		}

		// Mask all but two high order control bits
		controlChar = markByte & 0xC0U;
		paramChar = markByte & 0x3FU;
		switch (controlChar) {
		case SAGA_FRAME_EMPTY_RUN: // 1100 0000
			// Run of empty pixels
			runcount = paramChar + 1;
			writePointer += runcount;
			VALIDATE_WRITE_POINTER;
			continue;
			break;
		case SAGA_FRAME_COMPRESSED_RUN: // 1000 0000
			// Run of compressed data
			runcount = paramChar + 1;
			dataByte = readS.readByte();
			for (i = 0; i < runcount; i++) {
				*writePointer++ = dataByte;
				VALIDATE_WRITE_POINTER;
			}
			continue;
			break;
		case SAGA_FRAME_UNCOMPRESSED_RUN: // 0100 0000
			// Uncompressed run
			runcount = paramChar + 1;
			for (i = 0; i < runcount; i++) {
				dataByte = readS.readByte();
				if (dataByte != 0) {
					*writePointer = dataByte;
				}
				writePointer++;
				VALIDATE_WRITE_POINTER;
			}
			continue;
			break;
		default:
			// Unknown marker found - abort
			error("decodeFrame() Invalid RLE marker encountered");
			break;
		}
	} while (1);
}

int Anim::fillFrameOffsets(AnimationData *anim, bool reallyFill) {
	uint16 currentFrame = 0;
	byte markByte;
	uint16 control;
	uint16 runcount;
	int i;
	bool longData = isLongData();

	Common::MemoryReadStreamEndian readS(&anim->resourceData.front(), anim->resourceData.size(), !_vm->isBigEndian()); // RLE has inversion BE<>LE

	while (readS.pos() != readS.size()) {
		if (reallyFill) {
			anim->frameOffsets[currentFrame] = readS.pos();

			if (currentFrame == anim->maxFrame)
				break;
		}
		currentFrame++;

		// For some strange reason, the animation header is in little
		// endian format, but the actual RLE encoded frame data,
		// including the frame header, is in big endian format
		do {
			markByte = readS.readByte();
//			debug(7, "_pos=%X currentFrame=%i markByte=%X", readS.pos(), currentFrame, markByte);

			switch (markByte) {
			case SAGA_FRAME_START: // Start of frame
				// skip header
				if (longData) {
					readS.seek(13, SEEK_CUR);
				} else {
					readS.seek(12, SEEK_CUR);
				}
				continue;
				break;

			case SAGA_FRAME_END: // End of frame marker
				continue;
				break;
			case SAGA_FRAME_REPOSITION: // Reposition command
				readS.readSint16BE();
				continue;
				break;
			case SAGA_FRAME_ROW_END: // End of row marker
				readS.readSint16BE();
				if (longData)
					readS.readSint16BE();
				else
					readS.readByte();
				continue;
				break;
			case SAGA_FRAME_LONG_COMPRESSED_RUN: // Long compressed run marker
				readS.readSint16BE();
				readS.readByte();
				continue;
				break;
			case SAGA_FRAME_LONG_UNCOMPRESSED_RUN: // (16) 0001 0000
				// Long Uncompressed Run
				runcount = readS.readSint16BE();
				readS.seek(runcount, SEEK_CUR);
				continue;
				break;
			case SAGA_FRAME_NOOP: // Does nothing
				readS.readByte();
				readS.readByte();
				readS.readByte();
				continue;
				break;
			default:
				break;
			}

			// Mask all but two high order (control) bits
			control = markByte & 0xC0;
			switch (control) {
			case SAGA_FRAME_EMPTY_RUN:
				// Run of empty pixels
				continue;
				break;
			case SAGA_FRAME_COMPRESSED_RUN:
				// Run of compressed data
				readS.readByte(); // Skip data byte
				continue;
				break;
			case SAGA_FRAME_UNCOMPRESSED_RUN:
				// Uncompressed run
				runcount = (markByte & 0x3f) + 1;
				for (i = 0; i < runcount; i++)
					readS.readByte();
				continue;
				break;
			default:
				error("Encountered unknown RLE marker %i", markByte);
				break;
			}
		} while (markByte != SAGA_FRAME_END);
	}

	return currentFrame;
}

void Anim::animInfo() {
	uint16 animCount;
	uint16 i;

	animCount = getAnimationCount();

	_vm->_console->DebugPrintf("There are %d animations loaded:\n", animCount);

	for (i = 0; i < MAX_ANIMATIONS; i++) {
		if (_animations[i] == NULL) {
			continue;
		}

		_vm->_console->DebugPrintf("%02d: Frames: %u Flags: %u\n", i, _animations[i]->maxFrame, _animations[i]->flags);
	}
}

#ifdef ENABLE_IHNM
void Anim::cutawayInfo() {
	uint16 i;

	_vm->_console->DebugPrintf("There are %d cutaways loaded:\n", _cutawayList.size());

	for (i = 0; i < _cutawayList.size(); i++) {
		_vm->_console->DebugPrintf("%02d: Bg res: %u Anim res: %u Cycles: %u Framerate: %u\n", i,
			_cutawayList[i].backgroundResourceId, _cutawayList[i].animResourceId,
			_cutawayList[i].cycles, _cutawayList[i].frameRate);
	}
}
#endif

void Anim::resumeAll() {
	// Restore the animations
	for (int i = 0; i < MAX_ANIMATIONS; i++) {
		if (_animations[i] && _animations[i]->state == ANIM_PLAYING) {
			resume(i, 0);
		}
	}
}

} // End of namespace Saga
