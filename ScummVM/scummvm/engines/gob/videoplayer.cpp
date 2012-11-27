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


#include "gob/videoplayer.h"
#include "gob/global.h"
#include "gob/dataio.h"
#include "gob/video.h"
#include "gob/game.h"
#include "gob/palanim.h"
#include "gob/inter.h"
#include "gob/map.h"
#include "gob/sound/sound.h"

namespace Gob {

VideoPlayer::Properties::Properties() : type(kVideoTypeTry), sprite(Draw::kFrontSurface),
	x(-1), y(-1), width(-1), height(-1), flags(kFlagFrontSurface), switchColorMode(false),
	startFrame(-1), lastFrame(-1), endFrame(-1), forceSeek(false),
	breakKey(kShortKeyEscape), palCmd(8), palStart(0), palEnd(255), palFrame(-1),
	noBlock(false), loop(false), fade(false), waitEndFrame(true),
	hasSound(false), canceled(false) {

}


VideoPlayer::Video::Video() : decoder(0), live(false) {
}

bool VideoPlayer::Video::isEmpty() const {
	return decoder == 0;
}

void VideoPlayer::Video::close() {
	delete decoder;

	decoder = 0;
	fileName.clear();
	surface.reset();

	live = false;
}


const char *const VideoPlayer::_extensions[] = { "IMD", "IMD", "VMD", "RMD", "SMD" };

VideoPlayer::VideoPlayer(GobEngine *vm) : _vm(vm), _needBlit(false),
	_noCursorSwitch(false), _woodruffCohCottWorkaround(false) {
}

VideoPlayer::~VideoPlayer() {
	for (int i = 0; i < kVideoSlotCount; i++)
		_videoSlots[i].close();
}

void VideoPlayer::evaluateFlags(Properties &properties) {
	if        (properties.flags & kFlagFrontSurface) {
		properties.sprite = Draw::kFrontSurface;
	} else if (properties.flags & kFlagOtherSurface) {
		properties.sprite = properties.x;
		properties.x      = 0;
	} else if (properties.flags & kFlagScreenSurface) {
		properties.sprite = 0;
	} else if (properties.flags & kFlagNoVideo) {
		properties.sprite = 0;
	} else {
		properties.sprite = Draw::kBackSurface;
	}

	if (properties.noBlock && (properties.sprite == Draw::kFrontSurface))
		properties.sprite = Draw::kBackSurface;
}

int VideoPlayer::openVideo(bool primary, const Common::String &file, Properties &properties) {
	int slot = 0;

	Video *video = 0;
	if (!primary) {
		slot = getNextFreeSlot();
		if (slot < 0) {
			warning("VideoPlayer::openVideo(): Can't open video \"%s\": No free slot", file.c_str());
			return -1;
		}

		video = &_videoSlots[slot];
	} else
		video = &_videoSlots[0];

	// Different video already in the slot => close that video
	if (!video->isEmpty() && (video->fileName.compareToIgnoreCase(file) != 0))
		video->close();

	// No video => load the requested file
	if (video->isEmpty()) {
		// Open the video
		if (!(video->decoder = openVideo(file, properties)))
			return -1;

		if (video->decoder->hasVideo() && !(properties.flags & kFlagNoVideo) &&
		    (video->decoder->isPaletted() != !_vm->isTrueColor())) {
			if (!properties.switchColorMode)
				return -1;

			_vm->setTrueColor(!video->decoder->isPaletted());

			video->decoder->colorModeChanged();
		}

		// Set the filename
		video->fileName = file;

		// WORKAROUND: In some rare cases, the cursor should still be
		// displayed while a video is playing.
		_noCursorSwitch = false;
		if (primary && (_vm->getGameType() == kGameTypeLostInTime)) {
			if (!file.compareToIgnoreCase("PORTA03") ||
			    !file.compareToIgnoreCase("PORTA03A") ||
			    !file.compareToIgnoreCase("CALE1") ||
			    !file.compareToIgnoreCase("AMIL2") ||
			    !file.compareToIgnoreCase("AMIL3B") ||
			    !file.compareToIgnoreCase("DELB"))
				_noCursorSwitch = true;
		}

		// WORKAROUND: In Woodruff, Coh Cott vanished in one video on her party.
		// This is a bug in video, so we work around it.
		_woodruffCohCottWorkaround = false;
		if (primary && (_vm->getGameType() == kGameTypeWoodruff)) {
			if (!file.compareToIgnoreCase("SQ32-03"))
				_woodruffCohCottWorkaround = true;
		}

		if (!(properties.flags & kFlagNoVideo) && (properties.sprite >= 0)) {
			bool ownSurf    = (properties.sprite != Draw::kFrontSurface) && (properties.sprite != Draw::kBackSurface);
			bool screenSize = properties.flags & kFlagScreenSurface;

			if (ownSurf) {
				_vm->_draw->_spritesArray[properties.sprite] =
					_vm->_video->initSurfDesc(screenSize ? _vm->_width  : video->decoder->getWidth(),
					                          screenSize ? _vm->_height : video->decoder->getHeight(), 0);
			}

			if (!_vm->_draw->_spritesArray[properties.sprite] &&
			    (properties.sprite != Draw::kFrontSurface) &&
			    (properties.sprite != Draw::kBackSurface)) {
				properties.sprite = -1;
				video->surface.reset();
				video->decoder->setSurfaceMemory();
				properties.x = properties.y = 0;
			} else {
				video->surface = _vm->_draw->_spritesArray[properties.sprite];
				if (properties.sprite == Draw::kFrontSurface)
					video->surface = _vm->_draw->_frontSurface;
				if (properties.sprite == Draw::kBackSurface)
					video->surface = _vm->_draw->_backSurface;

				video->decoder->setSurfaceMemory(video->surface->getData(),
						video->surface->getWidth(), video->surface->getHeight(), video->surface->getBPP());

				if (!ownSurf || (ownSurf && screenSize)) {
					if ((properties.x >= 0) || (properties.y >= 0)) {
						properties.x = (properties.x < 0) ? 0xFFFF : properties.x;
						properties.y = (properties.y < 0) ? 0xFFFF : properties.y;
					} else
						properties.x = properties.y = -1;
				} else
					properties.x = properties.y = 0;
			}

		} else {
			properties.sprite = -1;
			video->surface.reset();
			video->decoder->setSurfaceMemory();
			properties.x = properties.y = 0;
		}
	}

	video->decoder->setXY(properties.x, properties.y);

	if (primary)
		_needBlit = (properties.flags & kFlagUseBackSurfaceContent) && (properties.sprite == Draw::kFrontSurface);

	properties.hasSound = video->decoder->hasSound();

	if (!video->decoder->hasSound())
		video->decoder->setFrameRate(_vm->_util->getFrameRate());

	WRITE_VAR(7, video->decoder->getFrameCount());

	return slot;
}

bool VideoPlayer::closeVideo(int slot) {
	Video *video = getVideoBySlot(slot);
	if (!video)
		return false;

	video->close();
	return true;
}

void VideoPlayer::closeLiveSound() {
	for (int i = 1; i < kVideoSlotCount; i++) {
		Video *video = getVideoBySlot(i);
		if (!video)
			continue;

		if (video->live)
			closeVideo(i);
	}
}

void VideoPlayer::closeAll() {
	for (int i = 0; i < kVideoSlotCount; i++)
		closeVideo(i);
}

bool VideoPlayer::reopenVideo(int slot) {
	Video *video = getVideoBySlot(slot);
	if (!video)
		return true;

	return reopenVideo(*video);
}

bool VideoPlayer::reopenAll() {
	bool all = true;
	for (int i = 0; i < kVideoSlotCount; i++)
		if (!reopenVideo(i))
			all = false;

	return all;
}

void VideoPlayer::pauseVideo(int slot, bool pause) {
	Video *video = getVideoBySlot(slot);
	if (!video || !video->decoder)
		return;

	video->decoder->pauseVideo(pause);
}

void VideoPlayer::pauseAll(bool pause) {
	for (int i = 0; i < kVideoSlotCount; i++)
		pauseVideo(i, pause);
}

void VideoPlayer::finishVideoSound(int slot) {
	Video *video = getVideoBySlot(slot);
	if (!video || !video->decoder)
		return;

	video->decoder->finishSound();
}

void VideoPlayer::waitSoundEnd(int slot) {
	Video *video = getVideoBySlot(slot);
	if (!video || !video->decoder)
		return;

	video->decoder->finishSound();

	while (video->decoder->isSoundPlaying())
		_vm->_util->longDelay(1);
}

bool VideoPlayer::play(int slot, Properties &properties) {
	Video *video = getVideoBySlot(slot);
	if (!video)
		return false;

	bool primary = slot == 0;

	if (properties.startFrame < 0)
		properties.startFrame = video->decoder->getCurFrame() + 1;
	if (properties.lastFrame  < 0)
		properties.lastFrame  = video->decoder->getFrameCount() - 1;
	if (properties.endFrame   < 0)
		properties.endFrame   = properties.lastFrame;
	if (properties.palFrame   < 0)
		properties.palFrame   = properties.startFrame;

	properties.startFrame--;
	properties.endFrame--;
	properties.palFrame--;

	if (primary) {
		_vm->_draw->_showCursor = _noCursorSwitch ? 3 : 0;

		if (properties.fade)
			_vm->_palAnim->fade(0, -2, 0);
	}

	bool backwards = properties.startFrame > properties.lastFrame;

	properties.canceled = false;

	if (properties.noBlock) {
		properties.waitEndFrame = false;

		video->live       = true;
		video->properties = properties;

		updateLive(slot, true);
		return true;
	}

	if ((_vm->getGameType() != kGameTypeUrban) && (_vm->getGameType() != kGameTypeBambou))
		// NOTE: For testing (and comfort?) purposes, we enable aborting of all videos.
		//       Except for Urban Runner and Bambou, where it leads to glitches
		properties.breakKey = kShortKeyEscape;

	while ((properties.startFrame != properties.lastFrame) &&
	       (properties.startFrame < (int32)(video->decoder->getFrameCount() - 1))) {

		playFrame(slot, properties);
		if (properties.canceled)
			break;

		properties.startFrame += backwards ? -1 : 1;

		evalBgShading(*video);

		if (primary && properties.fade) {
			_vm->_palAnim->fade(_vm->_global->_pPaletteDesc, -2, 0);
			properties.fade = false;
		}

		if (!_noCursorSwitch && properties.waitEndFrame)
			waitEndFrame(slot);
	}

	evalBgShading(*video);

	return true;
}

void VideoPlayer::waitEndFrame(int slot, bool onlySound) {
	Video *video = getVideoBySlot(slot);
	if (!video)
		return;

	if (!onlySound || video->decoder->hasSound()) {
		uint32 waitTime = video->decoder->getTimeToNextFrame();
		if (!video->decoder->hasSound())
			waitTime = video->decoder->getStaticTimeToNextFrame();

		_vm->_util->delay(waitTime);
	}
}

bool VideoPlayer::isPlayingLive() const {
	const Video *video = getVideoBySlot(0);
	return video && video->live;
}

void VideoPlayer::updateLive(bool force) {
	for (int i = 0; i < kVideoSlotCount; i++)
		updateLive(i, force);
}

void VideoPlayer::updateLive(int slot, bool force) {
	Video *video = getVideoBySlot(slot);
	if (!video || !video->live)
		return;

	if (video->properties.startFrame >= (int32)(video->decoder->getFrameCount() - 1)) {
		// Video ended

		if (!video->properties.loop) {
			if (!(video->properties.flags & kFlagNoVideo))
				WRITE_VAR_OFFSET(212, (uint32)-1);
			_vm->_vidPlayer->closeVideo(slot);
			return;
		} else {
			video->decoder->seek(0, SEEK_SET, true);
			video->properties.startFrame = -1;
		}
	}

	if (video->properties.startFrame == video->properties.lastFrame)
		// Current video sequence ended
		return;

	if (!force && (video->decoder->getTimeToNextFrame() > 0))
		return;

	if (!(video->properties.flags & kFlagNoVideo))
		WRITE_VAR_OFFSET(212, video->properties.startFrame + 1);

	bool backwards = video->properties.startFrame > video->properties.lastFrame;
	playFrame(slot, video->properties);

	video->properties.startFrame += backwards ? -1 : 1;

	if (video->properties.fade) {
		_vm->_palAnim->fade(_vm->_global->_pPaletteDesc, -2, 0);
		video->properties.fade = false;
	}
}

bool VideoPlayer::playFrame(int slot, Properties &properties) {
	Video *video = getVideoBySlot(slot);
	if (!video)
		return false;

	bool primary = slot == 0;

	if (video->decoder->getCurFrame() != properties.startFrame) {

		if (properties.startFrame != -1) {
			// Seek into the middle of the video

			if (video->decoder->hasSound()) {
				// But there's sound

				if (properties.forceSeek) {
					// And we force seeking => Seek

					video->decoder->disableSound();
					video->decoder->seek(properties.startFrame + 1, SEEK_SET, true);
				}

			} else
				// No sound => We can safely seek
				video->decoder->seek(properties.startFrame + 1, SEEK_SET, true);

		} else {
			// Seek to the start => We can safely seek

			video->decoder->disableSound();
			video->decoder->seek(0, SEEK_SET, true);
			video->decoder->enableSound();
		}

	}

	if (video->decoder->getCurFrame() > properties.startFrame)
		// If the video is already beyond the wanted frame, skip
		return true;

	bool modifiedPal = false;

	if (primary) {
		// Pre-decoding palette and blitting, only for primary videos

		if ((properties.startFrame == properties.palFrame) ||
		    ((properties.startFrame == properties.endFrame) && (properties.palCmd == 8))) {

			modifiedPal = true;
			_vm->_draw->_applyPal = true;

			if (properties.palCmd >= 4)
				copyPalette(*video, properties.palStart, properties.palEnd);
		}

		if (modifiedPal && (properties.palCmd == 8) && (video->surface != _vm->_draw->_backSurface))
			_vm->_video->setFullPalette(_vm->_global->_pPaletteDesc);

		if (_needBlit)
			_vm->_draw->forceBlit();
	}

	const Graphics::Surface *surface = video->decoder->decodeNextFrame();

	WRITE_VAR(11, video->decoder->getCurFrame());

	uint32 ignoreBorder = 0;
	if (_woodruffCohCottWorkaround && (properties.startFrame == 31)) {
		// WORKAROUND: This frame mistakenly masks Coh Cott, making her vanish
		// To prevent that, we'll never draw that part
		ignoreBorder = 50;
	}

	if (surface && primary) {
		// Post-decoding palette and blitting, only for primary videos

		if (_needBlit)
			_vm->_draw->forceBlit(true);

		if (modifiedPal && (properties.palCmd == 16)) {
			if (video->surface == _vm->_draw->_backSurface)
				_vm->_draw->forceBlit();
			_vm->_palAnim->fade(_vm->_global->_pPaletteDesc, -2, 0);
			_vm->_draw->_noInvalidated = true;
			_vm->_video->dirtyRectsAll();
		}

		if (video->decoder->hasPalette() && (properties.palCmd > 1)) {
			copyPalette(*video, properties.palStart, properties.palEnd);

			if (video->surface != _vm->_draw->_backSurface)
				_vm->_video->setFullPalette(_vm->_global->_pPaletteDesc);
			else
				_vm->_draw->_applyPal = true;
		}

		const Common::List<Common::Rect> &dirtyRects = video->decoder->getDirtyRects();

		if (modifiedPal && (properties.palCmd == 8) && (video->surface == _vm->_draw->_backSurface))
			_vm->_video->setFullPalette(_vm->_global->_pPaletteDesc);

		if (video->surface == _vm->_draw->_backSurface) {

			for (Common::List<Common::Rect>::const_iterator rect = dirtyRects.begin(); rect != dirtyRects.end(); ++rect)
				_vm->_draw->invalidateRect(rect->left + ignoreBorder, rect->top, rect->right - 1, rect->bottom - 1);
			if (!video->live)
				_vm->_draw->blitInvalidated();

		} else if (video->surface == _vm->_draw->_frontSurface) {
			for (Common::List<Common::Rect>::const_iterator rect = dirtyRects.begin(); rect != dirtyRects.end(); ++rect)
				_vm->_video->dirtyRectsAdd(rect->left + ignoreBorder, rect->top, rect->right - 1, rect->bottom - 1);

		}

		if (!video->live && ((video->decoder->getCurFrame() - 1) == properties.startFrame))
			// Only retrace if we're playing the frame we actually want to play
			_vm->_video->retrace();

		int32 subtitle = video->decoder->getSubtitleIndex();
		if (subtitle != -1)
			_vm->_draw->printTotText(subtitle);

		if (modifiedPal && ((properties.palCmd == 2) || (properties.palCmd == 4)))
			_vm->_palAnim->fade(_vm->_global->_pPaletteDesc, -2, 0);
	}

	if (primary && properties.waitEndFrame)
		checkAbort(*video, properties);

	if ((video->decoder->getCurFrame() - 1) < properties.startFrame)
		// The video played a frame we actually didn't want, so we have to adjust
		properties.startFrame--;

	return true;
}

void VideoPlayer::checkAbort(Video &video, Properties &properties) {
	_vm->_util->processInput();

	if (_vm->shouldQuit()) {
		video.decoder->disableSound();

		properties.canceled = true;
		return;
	}

	if (properties.breakKey != 0) {
		_vm->_util->getMouseState(&_vm->_global->_inter_mouseX,
				&_vm->_global->_inter_mouseY, &_vm->_game->_mouseButtons);

		_vm->_inter->storeKey(_vm->_util->checkKey());

		// Check for that specific key
		bool pressedBreak = (VAR(0) == (unsigned)properties.breakKey);

		// Mouse buttons
		if (properties.breakKey < 4)
			if (_vm->_game->_mouseButtons & properties.breakKey)
				pressedBreak = true;

		// Any key
		if (properties.breakKey == 4)
			if (VAR(0) != 0)
				pressedBreak = true;

		if (pressedBreak) {
			video.decoder->disableSound();

			// Seek to the last frame. Some scripts depend on that.
			video.decoder->seek(properties.endFrame + 1, SEEK_SET, true);

			properties.canceled = true;
		}
	}
}

bool VideoPlayer::slotIsOpen(int slot) const {
	return getVideoBySlot(slot) != 0;
}

Common::String VideoPlayer::getFileName(int slot) const {
	const Video *video = getVideoBySlot(slot);
	if (!video)
		return "";

	return video->fileName;
}

uint32 VideoPlayer::getFrameCount(int slot) const {
	const Video *video = getVideoBySlot(slot);
	if (!video)
		return 0;

	return video->decoder->getFrameCount();
}

uint32 VideoPlayer::getCurrentFrame(int slot) const {
	const Video *video = getVideoBySlot(slot);
	if (!video)
		return 0;

	return video->decoder->getCurFrame();
}

uint16 VideoPlayer::getWidth(int slot) const {
	const Video *video = getVideoBySlot(slot);
	if (!video)
		return 0;

	return video->decoder->getWidth();
}

uint16 VideoPlayer::getHeight(int slot) const {
	const Video *video = getVideoBySlot(slot);
	if (!video)
		return 0;

	return video->decoder->getHeight();
}

uint16 VideoPlayer::getDefaultX(int slot) const {
	const Video *video = getVideoBySlot(slot);
	if (!video)
		return 0;

	return video->decoder->getDefaultX();
}

uint16 VideoPlayer::getDefaultY(int slot) const {
	const Video *video = getVideoBySlot(slot);
	if (!video)
		return 0;

	return video->decoder->getDefaultY();
}

const Common::List<Common::Rect> *VideoPlayer::getDirtyRects(int slot) const {
	const Video *video = getVideoBySlot(slot);
	if (!video)
		return 0;

	return &video->decoder->getDirtyRects();
}

bool VideoPlayer::hasEmbeddedFile(const Common::String &fileName, int slot) const {
	const Video *video = getVideoBySlot(slot);
	if (!video)
		return false;

	return video->decoder->hasEmbeddedFile(fileName);
}

Common::SeekableReadStream *VideoPlayer::getEmbeddedFile(const Common::String &fileName, int slot) {
	const Video *video = getVideoBySlot(slot);
	if (!video)
		return 0;

	return video->decoder->getEmbeddedFile(fileName);
}

int32 VideoPlayer::getSubtitleIndex(int slot) const {
	const Video *video = getVideoBySlot(slot);
	if (!video)
		return -1;

	return video->decoder->getSubtitleIndex();
}

void VideoPlayer::writeVideoInfo(const Common::String &file, int16 varX, int16 varY,
		int16 varFrames, int16 varWidth, int16 varHeight) {

	Properties properties;

	int slot = openVideo(false, file, properties);
	if (slot >= 0) {
		Video &video = _videoSlots[slot];

		int16 x = -1, y = -1, width = -1, height = -1;

		x      = video.decoder->getDefaultX();
		y      = video.decoder->getDefaultY();
		width  = video.decoder->getWidth();
		height = video.decoder->getHeight();

		if (VAR_OFFSET(varX) == 0xFFFFFFFF)
			video.decoder->getFrameCoords(1, x, y, width, height);

		WRITE_VAR_OFFSET(varX     , x);
		WRITE_VAR_OFFSET(varY     , y);
		WRITE_VAR_OFFSET(varFrames, video.decoder->getFrameCount());
		WRITE_VAR_OFFSET(varWidth , width);
		WRITE_VAR_OFFSET(varHeight, height);

		closeVideo(slot);

	} else {
		WRITE_VAR_OFFSET(varX     , (uint32) -1);
		WRITE_VAR_OFFSET(varY     , (uint32) -1);
		WRITE_VAR_OFFSET(varFrames, (uint32) -1);
		WRITE_VAR_OFFSET(varWidth , (uint32) -1);
		WRITE_VAR_OFFSET(varHeight, (uint32) -1);
	}
}

bool VideoPlayer::copyFrame(int slot, Surface &dest,
		uint16 left, uint16 top, uint16 width, uint16 height, uint16 x, uint16 y,
		int32 transp) const {

	const Video *video = getVideoBySlot(slot);
	if (!video)
		return false;

	const Graphics::Surface *surface = video->decoder->getSurface();
	if (!surface)
		return false;

	Surface src(surface->w, surface->h, surface->format.bytesPerPixel, (byte *)surface->pixels);

	dest.blit(src, left, top, left + width - 1, top + height - 1, x, y, transp);
	return true;
}

const VideoPlayer::Video *VideoPlayer::getVideoBySlot(int slot) const {
	if ((slot < 0) || (slot >= kVideoSlotCount))
		return 0;

	if (_videoSlots[slot].isEmpty())
		return 0;

	return &_videoSlots[slot];
}

VideoPlayer::Video *VideoPlayer::getVideoBySlot(int slot) {
	if ((slot < 0) || (slot >= kVideoSlotCount))
		return 0;

	if (_videoSlots[slot].isEmpty())
		return 0;

	return &_videoSlots[slot];
}

int VideoPlayer::getNextFreeSlot() {
	// Starting with 1, since 0 is reserved for the "primary" video
	for (int i = 1; i < kVideoSlotCount; i++)
		if (_videoSlots[i].isEmpty())
			return i;

	return -1;
}

void VideoPlayer::evalBgShading(Video &video) {
	if (video.decoder->isSoundPlaying())
		_vm->_sound->bgShade();
	else
		_vm->_sound->bgUnshade();
}

Common::String VideoPlayer::findFile(const Common::String &file, Properties &properties) {

	bool hasExtension = false;

	Common::String base     = file;
	Common::String fileName = file;

	const char *posDot = strrchr(base.c_str(), '.');
	if (posDot) {
		hasExtension = true;
		base = Common::String(base.c_str(), posDot);
		posDot++;
	}

	if (hasExtension) {
		int i;
		for (i = 0; i < ARRAYSIZE(_extensions); i++) {
			if (!scumm_stricmp(posDot, _extensions[i])) {
				if ((properties.type != kVideoTypeTry) && (properties.type == ((Type) i))) {
					warning("Attempted to open video \"%s\", but requested a different type", fileName.c_str());
					return "";
				}
				properties.type = (Type) i;
				break;
			}
		}
		if (i >= ARRAYSIZE(_extensions))
			hasExtension = false;
	}

	if (!hasExtension) {
		// No or unrecognized extension. Probing.

		int i;
		for (i = 0; i < ARRAYSIZE(_extensions); i++) {
			if ((properties.type == kVideoTypeTry) || (properties.type == ((Type) i))) {
				fileName = base + "." + _extensions[i];

				if (_vm->_dataIO->hasFile(fileName)) {
					properties.type = (Type) i;
					break;
				}
			}
		}
		if ((i >= ARRAYSIZE(_extensions)) || (properties.type == kVideoTypeTry)) {
			warning("Couldn't open video \"%s\"", file.c_str());
			return "";
		}

	}

	return fileName;
}

::Video::CoktelDecoder *VideoPlayer::openVideo(const Common::String &file, Properties &properties) {
	Common::String fileName = findFile(file, properties);
	if (fileName.empty())
		return 0;

	Common::SeekableReadStream *stream = _vm->_dataIO->getFile(fileName);
	if (!stream)
		return 0;

	::Video::CoktelDecoder *video = 0;
	if (properties.type == kVideoTypeIMD)
		video = new ::Video::IMDDecoder(_vm->_mixer, Audio::Mixer::kSFXSoundType);
	else if (properties.type == kVideoTypePreIMD)
		video = new ::Video::PreIMDDecoder(properties.width, properties.height, _vm->_mixer, Audio::Mixer::kSFXSoundType);
	else if (properties.type == kVideoTypeVMD)
		video = new ::Video::VMDDecoder(_vm->_mixer, Audio::Mixer::kSFXSoundType);
	else if (properties.type == kVideoTypeRMD)
		video = new ::Video::VMDDecoder(_vm->_mixer, Audio::Mixer::kSFXSoundType);
	else
		warning("Couldn't open video \"%s\": Invalid video Type", fileName.c_str());

	if (!video) {
		delete stream;
		return 0;
	}

	if (!video->loadStream(stream)) {
		delete video;
		return 0;
	}

	properties.width  = video->getWidth();
	properties.height = video->getHeight();

	return video;
}

bool VideoPlayer::reopenVideo(Video &video) {
	if (video.isEmpty())
		return true;

	if (video.fileName.empty()) {
		video.close();
		return false;
	}

	Properties properties;

	properties.type = video.properties.type;

	Common::String fileName = findFile(video.fileName, properties);
	if (fileName.empty()) {
		video.close();
		return false;
	}

	Common::SeekableReadStream *stream = _vm->_dataIO->getFile(fileName);
	if (!stream) {
		video.close();
		return false;
	}

	if (!video.decoder->reloadStream(stream)) {
		delete stream;
		return false;
	}

	return true;
}

void VideoPlayer::copyPalette(const Video &video, int16 palStart, int16 palEnd) {
	if (!video.decoder->hasPalette() || !video.decoder->isPaletted())
		return;

	if (palStart < 0)
		palStart = 0;
	if (palEnd < 0)
		palEnd = 255;

	palStart =  palStart      * 3;
	palEnd   = (palEnd   + 1) * 3;

	for (int i = palStart; i <= palEnd; i++)
		((char *)(_vm->_global->_pPaletteDesc->vgaPal))[i] = video.decoder->getPalette()[i] >> 2;
}

} // End of namespace Gob
