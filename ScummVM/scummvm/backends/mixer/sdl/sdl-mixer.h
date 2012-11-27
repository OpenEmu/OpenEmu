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

#ifndef BACKENDS_MIXER_SDL_H
#define BACKENDS_MIXER_SDL_H

#include "backends/platform/sdl/sdl-sys.h"
#include "audio/mixer_intern.h"

/**
 * SDL mixer manager. It wraps the actual implementation
 * of the Audio:Mixer used by the engine, and setups
 * the SDL audio subsystem and the callback for the
 * audio mixer implementation.
 */
class SdlMixerManager {
public:
	SdlMixerManager();
	virtual ~SdlMixerManager();

	/**
	 * Initialize and setups the mixer
	 */
	virtual void init();

	/**
	 * Get the audio mixer implementation
	 */
	Audio::Mixer *getMixer() { return (Audio::Mixer *)_mixer; }

	// Used by LinuxMoto Port

	/**
	 * Pauses the audio system
	 */
	virtual void suspendAudio();

	/**
	 * Resumes the audio system
	 */
	virtual int resumeAudio();

protected:
	/** The mixer implementation */
	Audio::MixerImpl *_mixer;

	/**
	 * The obtained audio specification after opening the
	 * audio system.
	 */
	SDL_AudioSpec _obtained;

	/** State of the audio system */
	bool _audioSuspended;

	/**
	 * Returns the desired audio specification
	 */
	virtual SDL_AudioSpec getAudioSpec(uint32 rate);

	/**
	 * Starts SDL audio
	 */
	virtual void startAudio();

	/**
	 * Handles the audio callback
	 */
	virtual void callbackHandler(byte *samples, int len);

	/**
	 * The mixer callback entry point. Static functions can't be overrided
	 * by subclasses, so it invokes the non-static function callbackHandler()
	 */
	static void sdlCallback(void *this_, byte *samples, int len);
};

#endif
