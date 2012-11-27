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

#include "draci/draci.h"
#include "draci/animation.h"
#include "draci/barchive.h"
#include "draci/game.h"
#include "draci/screen.h"
#include "draci/sound.h"
#include "draci/surface.h"

#include "common/memstream.h"
#include "common/system.h"

namespace Draci {

Animation::Animation(DraciEngine *vm, int id, uint z, bool playing) : _vm(vm) {
	_id = id;
	_index = kIgnoreIndex;
	_z = z;
	clearShift();
	_displacement = kNoDisplacement;
	_playing = playing;
	_looping = false;
	_paused = false;
	_canBeQuick = false;
	_tick = _vm->_system->getMillis();
	_currentFrame = 0;
	_hasChangedFrame = true;
	_callback = &Animation::doNothing;
	_isRelative = false;
}

Animation::~Animation() {
	deleteFrames();
}

void Animation::setRelative(int relx, int rely) {
	// Delete the previous frame if there is one
	if (_frames.size() > 0)
		markDirtyRect(_vm->_screen->getSurface());

	_displacement.relX = relx;
	_displacement.relY = rely;
}

Displacement Animation::getCurrentFrameDisplacement() const {
	Displacement dis = _displacement;
	dis.relX += scummvm_lround(dis.extraScaleX * _shift.x);
	dis.relY += scummvm_lround(dis.extraScaleY * _shift.y);
	return dis;
}

Common::Point Animation::getCurrentFramePosition() const {
	Displacement dis = getCurrentFrameDisplacement();
	return Common::Point(dis.relX, dis.relY);
}

void Animation::setLooping(bool looping) {
	_looping = looping;
	debugC(7, kDraciAnimationDebugLevel, "Setting looping to %d on animation %d",
		looping, _id);
}

void Animation::markDirtyRect(Surface *surface) const {
	if (getFrameCount() == 0)
		return;

	// Fetch the current frame's rectangle
	const Drawable *frame = getConstCurrentFrame();
	Common::Rect frameRect = frame->getRect(getCurrentFrameDisplacement());

	// Mark the rectangle dirty on the surface
	surface->markDirtyRect(frameRect);
}

void Animation::nextFrame(bool force) {
	// If there are no frames or if the animation is not playing, return
	if (getFrameCount() == 0 || !_playing)
		return;

	const Drawable *frame = getConstCurrentFrame();
	Surface *surface = _vm->_screen->getSurface();

	if (force || (_tick + frame->getDelay() <= _vm->_system->getMillis()) ||
	    (_canBeQuick && _vm->_game->getEnableQuickHero() && _vm->_game->getWantQuickHero())) {
		// If we are at the last frame and not looping, stop the animation
		// The animation is also restarted to frame zero
		if ((_currentFrame == getFrameCount() - 1) && !_looping) {
			// When the animation reaches its end, call the preset callback
			(this->*_callback)();
		} else {
			// Mark old frame dirty so it gets deleted
			markDirtyRect(surface);

			_shift.x += _relativeShifts[_currentFrame].x;
			_shift.y += _relativeShifts[_currentFrame].y;
			_currentFrame = nextFrameNum();
			_tick = _vm->_system->getMillis();

			// Fetch new frame and mark it dirty
			markDirtyRect(surface);

			// If the animation is paused, then nextFrameNum()
			// returns the same frame number even though the time
			// has elapsed to switch to another frame.  We must not
			// flip _hasChangedFrame to true, otherwise the sample
			// assigned to this frame will be re-started over and
			// over until all sound handles are exhausted (happens,
			// e.g., when switching to the inventory which pauses
			// all animations).
			_hasChangedFrame = !_paused;
		}
	}

	debugC(6, kDraciAnimationDebugLevel,
	"anim=%d tick=%d delay=%d tick+delay=%d currenttime=%d frame=%d framenum=%d x=%d y=%d z=%d",
	_id, _tick, frame->getDelay(), _tick + frame->getDelay(), _vm->_system->getMillis(),
	_currentFrame, _frames.size(), frame->getX() + getRelativeX(), frame->getY() + getRelativeY(), _z);
}

uint Animation::nextFrameNum() const {
	if (_paused)
		return _currentFrame;

	if ((_currentFrame == getFrameCount() - 1) && _looping)
		return 0;
	else
		return _currentFrame + 1;
}

void Animation::drawFrame(Surface *surface) {
	// If there are no frames or the animation is not playing, return
	if (_frames.size() == 0 || !_playing)
		return;

	const Drawable *frame = getConstCurrentFrame();

	if (_id == kOverlayImage) {
		// No displacement or relative animations is supported.
		frame->draw(surface, false, 0, 0);
	} else {
		// Draw frame: first shifted by the relative shift and then
		// scaled/shifted by the given displacement.
		frame->drawReScaled(surface, false, getCurrentFrameDisplacement());
	}

	const SoundSample *sample = _samples[_currentFrame];
	if (_hasChangedFrame && sample) {
		uint duration = _vm->_sound->playSound(sample, Audio::Mixer::kMaxChannelVolume, false);
		debugC(3, kDraciSoundDebugLevel,
			"Playing sample on animation %d, frame %d: %d+%d at %dHz: %dms",
			_id, _currentFrame, sample->_offset, sample->_length, sample->_frequency, duration);
	}
	_hasChangedFrame = false;
}

void Animation::setPlaying(bool playing) {
	_tick = _vm->_system->getMillis();
	_playing = playing;

	// When restarting an animation, allow playing sounds.
	_hasChangedFrame |= playing;
}

void Animation::setScaleFactors(double scaleX, double scaleY) {
	debugC(5, kDraciAnimationDebugLevel,
		"Setting scaling factors on anim %d (scaleX: %.3f scaleY: %.3f)",
		_id, scaleX, scaleY);

	markDirtyRect(_vm->_screen->getSurface());

	_displacement.extraScaleX = scaleX;
	_displacement.extraScaleY = scaleY;
}

void Animation::addFrame(Drawable *frame, const SoundSample *sample) {
	_frames.push_back(frame);
	_samples.push_back(sample);
	_relativeShifts.push_back(Common::Point(0, 0));
}

void Animation::makeLastFrameRelative(int x, int y) {
	_relativeShifts.back() = Common::Point(x, y);
}

void Animation::clearShift() {
	_shift = Common::Point(0, 0);
}

void Animation::replaceFrame(int i, Drawable *frame, const SoundSample *sample) {
	_frames[i] = frame;
	_samples[i] = sample;
}

const Drawable *Animation::getConstCurrentFrame() const {
	// If there are no frames stored, return NULL
	return _frames.size() > 0 ? _frames[_currentFrame] : NULL;
}

Drawable *Animation::getCurrentFrame() {
	// If there are no frames stored, return NULL
	return _frames.size() > 0 ? _frames[_currentFrame] : NULL;
}

Drawable *Animation::getFrame(int frameNum) {
	// If there are no frames stored, return NULL
	return _frames.size() > 0 ? _frames[frameNum] : NULL;
}

void Animation::setCurrentFrame(uint frame) {
	// Check whether the value is sane
	if (frame >= _frames.size()) {
		return;
	}

	_currentFrame = frame;
}

void Animation::deleteFrames() {
	// If there are no frames to delete, return
	if (_frames.size() == 0) {
		return;
	}

	markDirtyRect(_vm->_screen->getSurface());

	for (int i = getFrameCount() - 1; i >= 0; --i) {
		delete _frames[i];
		_frames.pop_back();
	}
	_relativeShifts.clear();
	_samples.clear();
}

void Animation::exitGameLoop() {
	_vm->_game->setExitLoop(true);
}

void Animation::tellWalkingState() {
	_vm->_game->heroAnimationFinished();
}

void Animation::play() {
	if (isPlaying()) {
		return;
	}

	// Mark the first frame dirty so it gets displayed
	markDirtyRect(_vm->_screen->getSurface());

	setPlaying(true);

	debugC(3, kDraciAnimationDebugLevel, "Playing animation %d...", getID());
}

void Animation::stop() {
	if (!isPlaying()) {
		return;
	}

	// Clean up the last frame that was drawn before stopping
	markDirtyRect(_vm->_screen->getSurface());

	setPlaying(false);

	// Reset the animation to the beginning
	setCurrentFrame(0);
	clearShift();

	debugC(3, kDraciAnimationDebugLevel, "Stopping animation %d...", getID());
}

void Animation::del() {
	_vm->_anims->deleteAnimation(this);
}

void AnimationManager::pauseAnimations() {
	if (_animationPauseCounter++) {
		// Already paused
		return;
	}

	Common::List<Animation *>::iterator it;

	for (it = _animations.begin(); it != _animations.end(); ++it) {
		if ((*it)->getID() > 0 || (*it)->getID() == kTitleText) {
			// Clean up the last frame that was drawn before stopping
			(*it)->markDirtyRect(_vm->_screen->getSurface());

			(*it)->setPaused(true);
		}
	}
}

void AnimationManager::unpauseAnimations() {
	if (--_animationPauseCounter) {
		// Still paused
		return;
	}

	Common::List<Animation *>::iterator it;

	for (it = _animations.begin(); it != _animations.end(); ++it) {
		if ((*it)->isPaused()) {
			// Clean up the last frame that was drawn before stopping
			(*it)->markDirtyRect(_vm->_screen->getSurface());

			(*it)->setPaused(false);
		}
	}
}

Animation *AnimationManager::getAnimation(int id) {
	Common::List<Animation *>::iterator it;

	for (it = _animations.begin(); it != _animations.end(); ++it) {
		if ((*it)->getID() == id) {
			return *it;
		}
	}

	return NULL;
}

void AnimationManager::insert(Animation *anim, bool allocateIndex) {
	if (allocateIndex)
		anim->setIndex(++_lastIndex);

	Common::List<Animation *>::iterator it;

	for (it = _animations.begin(); it != _animations.end(); ++it) {
		if (anim->getZ() < (*it)->getZ())
			break;
	}

	_animations.insert(it, anim);
}

void AnimationManager::drawScene(Surface *surf) {
	// Fill the screen with color zero since some rooms may rely on the screen being black
	_vm->_screen->getSurface()->fill(0);

	sortAnimations();

	Common::List<Animation *>::iterator it;

	for (it = _animations.begin(); it != _animations.end(); ++it) {
		if (! ((*it)->isPlaying()) ) {
			continue;
		}

		(*it)->nextFrame(false);
		(*it)->drawFrame(surf);
	}
}

void AnimationManager::sortAnimations() {
	Common::List<Animation *>::iterator cur;
	Common::List<Animation *>::iterator next;

	cur = _animations.begin();

	// If the list is empty, we're done
	if (cur == _animations.end())
		return;

	bool hasChanged;

	do {
		hasChanged = false;
		cur = _animations.begin();
		next = cur;

		while (true) {
			next++;

			// If we are at the last element, we're done
			if (next == _animations.end())
				break;

			// If we find an animation out of order, reinsert it
			if ((*next)->getZ() < (*cur)->getZ()) {

				Animation *anim = *next;
				next = _animations.reverse_erase(next);

				insert(anim, false);
				hasChanged = true;
			}

			// Advance to next animation
			cur = next;
		}
	} while (hasChanged);
}

void AnimationManager::deleteAnimation(Animation *anim) {
	if (!anim) {
		return;
	}
	Common::List<Animation *>::iterator it;

	int index = -1;

	// Iterate for the first time to delete the animation
	for (it = _animations.begin(); it != _animations.end(); ++it) {
		if (*it == anim) {
			// Remember index of the deleted animation
			index = (*it)->getIndex();

			debugC(3, kDraciAnimationDebugLevel, "Deleting animation %d...", anim->getID());

			delete *it;
			_animations.erase(it);

			break;
		}
	}

	// Iterate the second time to decrease indexes greater than the deleted animation index
	for (it = _animations.begin(); it != _animations.end(); ++it) {
		if ((*it)->getIndex() > index && (*it)->getIndex() != kIgnoreIndex) {
			(*it)->setIndex((*it)->getIndex() - 1);
		}
	}

	// Decrement index of last animation
	_lastIndex -= 1;
}

void AnimationManager::deleteOverlays() {
	debugC(3, kDraciAnimationDebugLevel, "Deleting overlays...");

	Common::List<Animation *>::iterator it;

	for (it = _animations.begin(); it != _animations.end(); ++it) {
		if ((*it)->getID() == kOverlayImage) {
			delete *it;
			it = _animations.reverse_erase(it);
		}
	}
}

void AnimationManager::deleteAll() {
	debugC(3, kDraciAnimationDebugLevel, "Deleting all animations...");

	Common::List<Animation *>::iterator it;

	for (it = _animations.begin(); it != _animations.end(); ++it) {
		delete *it;
	}

	_animations.clear();

	_lastIndex = -1;
}

void AnimationManager::deleteAfterIndex(int index) {
	Common::List<Animation *>::iterator it;

	for (it = _animations.begin(); it != _animations.end(); ++it) {
		if ((*it)->getIndex() > index) {

			debugC(3, kDraciAnimationDebugLevel, "Deleting animation %d...", (*it)->getID());

			delete *it;
			it = _animations.reverse_erase(it);
		}
	}

	_lastIndex = index;
}

const Animation *AnimationManager::getTopAnimation(int x, int y) const {
	Common::List<Animation *>::const_iterator it;

	Animation *retval = NULL;

	// Get transparent color for the current screen
	const int transparent = _vm->_screen->getSurface()->getTransparentColor();

	for (it = _animations.reverse_begin(); it != _animations.end(); --it) {

		Animation *anim = *it;

		// If the animation is not playing, ignore it
		if (!anim->isPlaying() || anim->isPaused()) {
			continue;
		}

		const Drawable *frame = anim->getConstCurrentFrame();

		if (frame == NULL) {
			continue;
		}

		bool matches = false;
		if (frame->getRect(anim->getCurrentFrameDisplacement()).contains(x, y)) {
			if (frame->getType() == kDrawableText) {

				matches = true;

			} else if (frame->getType() == kDrawableSprite &&
					   reinterpret_cast<const Sprite *>(frame)->getPixel(x, y, anim->getCurrentFrameDisplacement()) != transparent) {

				matches = true;
			}
		}

		// Return the top-most animation object, unless it is a
		// non-clickable sprite (overlay, debugging sprites for
		// walking, or title/speech text) and there is an actual object
		// underneath it.
		if (matches) {
			if (anim->getID() > kOverlayImage || anim->getID() < kSpeechText) {
				return anim;
			} else if (retval == NULL) {
				retval = anim;
			}
		}
	}

	// The default return value if no animations were found on these coordinates (not even overlays)
	return retval;
}

Animation *AnimationManager::load(uint animNum) {
	// Make double-sure that an animation isn't loaded more than twice,
	// otherwise horrible things happen in the AnimationManager, because
	// they use a simple link-list without duplicate checking.  This should
	// never happen unless there is a bug in the game, because all GPL2
	// commands are guarded.
	assert(!getAnimation(animNum));

	const BAFile *animFile = _vm->_animationsArchive->getFile(animNum);
	Common::MemoryReadStream animationReader(animFile->_data, animFile->_length);

	uint numFrames = animationReader.readByte();

	// The following two flags are ignored by the played.  Memory logic was
	// a hint to the old player whether it should cache the sprites or load
	// them on demand.  We have 1 memory manager and ignore these hints.
	animationReader.readByte();
	// The disable erasing field is just a (poor) optimization flag that
	// turns of drawing the background underneath the sprite.  By reading
	// the source code of the old player, I'm not sure if that would ever
	// have worked.  There are only 6 animations in the game with this flag
	// true.  All of them have just 1 animation phase and they are used to
	// patch a part of the original background by a new sprite.  This
	// should work with the default logic as well---just play this
	// animation on top of the background.  Since the only meaning of the
	// flag was optimization, ignoring should be OK even without dipping
	// into details.
	animationReader.readByte();
	const bool cyclic = animationReader.readByte();
	const bool relative = animationReader.readByte();

	Animation *anim = new Animation(_vm, animNum, 0, false);
	insert(anim, true);

	anim->setLooping(cyclic);
	anim->setIsRelative(relative);

	for (uint i = 0; i < numFrames; ++i) {
		uint spriteNum = animationReader.readUint16LE() - 1;
		int x = animationReader.readSint16LE();
		int y = animationReader.readSint16LE();
		uint scaledWidth = animationReader.readUint16LE();
		uint scaledHeight = animationReader.readUint16LE();
		byte mirror = animationReader.readByte();
		int sample = animationReader.readUint16LE() - 1;
		uint freq = animationReader.readUint16LE();
		uint delay = animationReader.readUint16LE();

		// _spritesArchive is flushed when entering a room.  All
		// scripts in a room are responsible for loading their animations.
		const BAFile *spriteFile = _vm->_spritesArchive->getFile(spriteNum);
		Sprite *sp = new Sprite(spriteFile->_data, spriteFile->_length,
			relative ? 0 : x, relative ? 0 : y, true);

		// Some frames set the scaled dimensions to 0 even though other frames
		// from the same animations have them set to normal values
		// We work around this by assuming it means no scaling is necessary
		if (scaledWidth == 0) {
			scaledWidth = sp->getWidth();
		}

		if (scaledHeight == 0) {
			scaledHeight = sp->getHeight();
		}

		sp->setScaled(scaledWidth, scaledHeight);

		if (mirror)
			sp->setMirrorOn();

		sp->setDelay(delay * 10);

		anim->addFrame(sp, _vm->_soundsArchive->getSample(sample, freq));
		if (relative) {
			anim->makeLastFrameRelative(x, y);
		}
	}

	return anim;
}

} // End of namespace Draci
