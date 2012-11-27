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

#ifndef AUDIO_MIDIPLAYER_H
#define AUDIO_MIDIPLAYER_H

#include "common/scummsys.h"
#include "common/mutex.h"
#include "audio/mididrv.h"

class MidiParser;

namespace Audio {

/**
 * Simple MIDI playback class.
 *
 * @note Currently incomplete, as it lacks play() methods. This is just a
 * start of the real thing, which tries to include code replicated between
 * several of our engines.
 *
 * Eventually, this should offer ways to start playback of SMF and XMIDI
 * data (and possibly make it easy to add further playback methods),
 * should be able to automatically instantiate _driver as needed,
 * should perform memory management on the MidiParser object(s) being
 * used, and possibly more.
 *
 * Also, pause/resume handling should be unified (we provide an implementation,
 * but several subclasses use different ones).
 *
 * Also, care should be taken to ensure that mutex locking is done right.
 *
 * @todo Document origin of this class. It is based on code shared by
 * several engines (e.g. DRACI says it copied it from MADE, which took
 * it from SAGE).
 */
class MidiPlayer : public MidiDriver_BASE {
public:
	MidiPlayer();
	~MidiPlayer();

	// TODO: Implement ways to actually play stuff
	//virtual void play(TODO);

	// TODO: Document these
	virtual void stop();
	virtual void pause();
	virtual void resume();

	/**
	 * Return whether there is currently any MIDI music playing.
	 *
	 * @todo There is a subtle difference between the semantics of this in
	 *       various subclasses, related to paused music: Namely, should this
	 *       function return true or false if a MIDI song is currently loaded,
	 *       but paused? In the base implementation of pause/resume, "false"
	 *       will be returned (that is, it is not possible to distinguish between
	 *       nothing being played, and an active but paused MIDI tune).
	 *       But in several subclasses (e.g. in HUGO), there is a separate _paused
	 *       variable, which is used to pause playback, and for these, "true"
	 *       will be returned.
	 *       And in SAGA, isPlaying is overwritten to return the value
	 *       of _parser->isPlaying() (which should amount to "true" in the
	 *       described situation).
	 *       We really should unify this and clearly define the desired
	 *       semantics of this method.
	 */
	bool isPlaying() const { return _isPlaying; }

	/**
	 * Return the currently active master volume, in the range 0-255.
	 */
	int getVolume() const { return _masterVolume; }

	/**
	 * Set the master volume to the specified value in the range 0-255.
	 * (values outside this range are automatically clipped).
	 * This may cause suitable MIDI events to be sent to active channels.
	 *
	 * @todo This method currently does not do anything if the new volume
	 *       matches the old volume. But some engines always update the
	 *       volume (Parallaction, Tinsel, Touche, ...). This is why
	 *       this method is currently virtual. We really should figure
	 *       which way to do it, and then make this the default, and make
	 *       this method non-virtual again.
	 */
	virtual void setVolume(int volume);

	/**
	 * Update the volume according to the ConfMan's 'music_volume'
	 * setting. also respects the 'mute' setting.
	 */
	void syncVolume();

	// TODO: Document this
	bool hasNativeMT32() const { return _nativeMT32; }

	// MidiDriver_BASE implementation
	virtual void send(uint32 b);
	virtual void metaEvent(byte type, byte *data, uint16 length);

protected:
	/**
	 * This method is invoked by the default send() implementation,
	 * after suitably filtering the message b.
	 */
	virtual void sendToChannel(byte ch, uint32 b);

	/**
	 * This method is invoked by metaEvent when an end-of-track
	 * event arrives. By default, this tells the parser
	 * to jump to the start (if looping is enabled) resp.
	 * invokes stope():
	 * Overload this to customize behavior.
	 */
	virtual void endOfTrack();

	// TODO: Document this
	virtual void onTimer();

	static void timerCallback(void *data);

	void createDriver(int flags = MDT_MIDI | MDT_ADLIB | MDT_PREFER_GM);

protected:
	enum {
		/**
		 * The number of MIDI channels supported.
		 */
		kNumChannels = 16
	};

	Common::Mutex _mutex;
	MidiDriver *_driver;

	/**
	 * A MidiParser instances, to be created by methods of a MidiPlayer
	 * subclass.
	 * @note The stop() method (and hence the destructor) invoke the
	 * unloadMusic() method of _parser and then delete it.
	 */
	MidiParser *_parser;

	/**
	 * This is an (optional) pointer to a malloc'ed buffer containing
	 * MIDI data used by _parser. The stop() method (and hence the
	 * destructor) will free this if set.
	 * Subclasses of this class may use _midiData, but don't have to
	 */
	byte *_midiData;

	MidiChannel *_channelsTable[kNumChannels];
	uint8 _channelsVolume[kNumChannels];

	bool _isLooping;
	bool _isPlaying;

	/**
	 * The master volume, in the range 0-255.
	 */
	int _masterVolume;	// FIXME: byte or int ?

	bool _nativeMT32;
};


} // End of namespace Audio

#endif
