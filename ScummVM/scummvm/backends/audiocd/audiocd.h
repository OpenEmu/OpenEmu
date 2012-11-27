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

#ifndef BACKENDS_AUDIOCD_ABSTRACT_H
#define BACKENDS_AUDIOCD_ABSTRACT_H

#include "common/scummsys.h"
#include "common/noncopyable.h"

/**
* Abstract Audio CD manager class. Subclasses implement the actual
* functionality.
*/
class AudioCDManager : Common::NonCopyable {
public:
	virtual ~AudioCDManager() {}

	/**
	* A structure containing the current playback information
	*/
	struct Status {
		bool playing;
		int track;
		int start;
		int duration;
		int numLoops;
		int volume;
		int balance;
	};

	/**
	 * @name Emulated playback functions
	 * Engines should call these functions. Not all platforms
	 * support cd playback, and these functions should try to
	 * emulate it.
	 */
	//@{

	/**
	 * Start audio CD playback
	 * @param track			the track to play.
	 * @param num_loops		how often playback should be repeated (-1 = infinitely often).
	 * @param start_frame	the frame at which playback should start (75 frames = 1 second).
	 * @param duration		the number of frames to play.
	 * @param only_emulate	determines if the track should be emulated only
	 */
	virtual void play(int track, int numLoops, int startFrame, int duration, bool only_emulate = false) = 0;

	/**
	 * Get if audio is being played.
	 * @return true if CD or emulated audio is playing
	 */
	virtual bool isPlaying() const = 0;

	/**
	 * Set the audio volume
	 */
	virtual void setVolume(byte volume) = 0;

	/**
	 * Set the speakers balance
	 */
	virtual void setBalance(int8 balance) = 0;

	/**
	 * Stop CD or emulated audio playback.
	 */
	virtual void stop() = 0;

	/**
	 * Update CD or emulated audio status.
	 */
	virtual void update() = 0;

	/**
	 * Get the playback status.
	 * @return a Status struct with playback data.
	 */
	virtual Status getStatus() const = 0;

	//@}


	/**
	 * @name Real CD audio methods
	 * These functions should be called from the emulated
	 * ones if they can't emulate the audio playback.
	 */
	//@{

	/**
	 * Initialize the specified CD drive for audio playback.
	 * @param drive the drive id
	 * @return true if the CD drive was inited succesfully
	 */
	virtual bool openCD(int drive) = 0;

	/**
	 * Poll CD status.
	 * @return true if CD audio is playing
	 */
	virtual bool pollCD() const = 0;

	/**
	 * Start CD audio playback.
	 * @param track			the track to play.
	 * @param num_loops		how often playback should be repeated (-1 = infinitely often).
	 * @param start_frame	the frame at which playback should start (75 frames = 1 second).
	 * @param duration		the number of frames to play.
	 */
	virtual void playCD(int track, int num_loops, int start_frame, int duration) = 0;

	/**
	 * Stop CD audio playback.
	 */
	virtual void stopCD() = 0;

	/**
	 * Update CD audio status.
	 */
	virtual void updateCD() = 0;

	//@}
};

#endif
