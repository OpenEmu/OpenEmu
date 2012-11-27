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

/// \brief Declarations related to the MidiParser class

#ifndef AUDIO_MIDIPARSER_H
#define AUDIO_MIDIPARSER_H

#include "common/scummsys.h"
#include "common/endian.h"

class MidiDriver_BASE;



//////////////////////////////////////////////////
//
// Support entities
//
//////////////////////////////////////////////////

/**
 * Maintains time and position state within a MIDI stream.
 * A single Tracker struct is used by MidiParser to keep track
 * of its current position in the MIDI stream. The Tracker
 * struct, however, allows alternative locations to be cached.
 * See MidiParser::jumpToTick() for an example of tracking
 * multiple locations within a MIDI stream. NOTE: It is
 * important to also maintain pre-parsed EventInfo data for
 * each Tracker location.
 */
struct Tracker {
	byte * _playPos;        ///< A pointer to the next event to be parsed
	uint32 _playTime;       ///< Current time in microseconds; may be in between event times
	uint32 _playTick;       ///< Current MIDI tick; may be in between event ticks
	uint32 _lastEventTime; ///< The time, in microseconds, of the last event that was parsed
	uint32 _lastEventTick; ///< The tick at which the last parsed event occurs
	byte   _runningStatus;  ///< Cached MIDI command, for MIDI streams that rely on implied event codes

	Tracker() { clear(); }

	/// Copy constructor for each duplication of Tracker information.
	Tracker(const Tracker &copy) :
	_playPos(copy._playPos),
	_playTime(copy._playTime),
	_playTick(copy._playTick),
	_lastEventTime(copy._lastEventTime),
	_lastEventTick(copy._lastEventTick),
	_runningStatus(copy._runningStatus)
	{ }

	/// Clears all data; used by the constructor for initialization.
	void clear() {
		_playPos = 0;
		_playTime = 0;
		_playTick = 0;
		_lastEventTime = 0;
		_lastEventTick = 0;
		_runningStatus = 0;
	}
};

/**
 * Provides comprehensive information on the next event in the MIDI stream.
 * An EventInfo struct is instantiated by format-specific implementations
 * of MidiParser::parseNextEvent() each time another event is needed.
 */
struct EventInfo {
	byte * start; ///< Position in the MIDI stream where the event starts.
	              ///< For delta-based MIDI streams (e.g. SMF and XMIDI), this points to the delta.
	uint32 delta; ///< The number of ticks after the previous event that this event should occur.
	byte   event; ///< Upper 4 bits are the command code, lower 4 bits are the MIDI channel.
	              ///< For META, event == 0xFF. For SysEx, event == 0xF0.
	union {
		struct {
			byte param1; ///< The first parameter in a simple MIDI message.
			byte param2; ///< The second parameter in a simple MIDI message.
		} basic;
		struct {
			byte   type; ///< For META events, this indicates the META type.
			byte * data; ///< For META and SysEx events, this points to the start of the data.
		} ext;
	};
	uint32 length; ///< For META and SysEx blocks, this indicates the length of the data.
	               ///< For Note On events, a non-zero value indicates that no Note Off event
	               ///< will occur, and the MidiParser will have to generate one itself.
	               ///< For all other events, this value should always be zero.

	byte channel() { return event & 0x0F; } ///< Separates the MIDI channel from the event.
	byte command() { return event >> 4; }   ///< Separates the command code from the event.
};

/**
 * Provides expiration tracking for hanging notes.
 * Hanging notes are used when a MIDI format does not include explicit Note Off
 * events, or when "Smart Jump" is enabled so that active notes are intelligently
 * expired when a jump occurs. The NoteTimer struct keeps track of how much
 * longer a note should remain active before being turned off.
 */
struct NoteTimer {
	byte channel;     ///< The MIDI channel on which the note was played
	byte note;        ///< The note number for the active note
	uint32 timeLeft; ///< The time, in microseconds, remaining before the note should be turned off
	NoteTimer() : channel(0), note(0), timeLeft(0) {}
};




//////////////////////////////////////////////////
//
// MidiParser declaration
//
//////////////////////////////////////////////////

/**
 * A framework and common functionality for parsing event-based music streams.
 * The MidiParser provides a framework in which to load,
 * parse and traverse event-based music data. Note the
 * avoidance of the phrase "MIDI data." Despite its name,
 * MidiParser derivatives can be used to manage a wide
 * variety of event-based music formats. It is, however,
 * based on the premise that the format in question can
 * be played in the form of specification MIDI events.
 *
 * In order to use MidiParser to parse your music format,
 * follow these steps:
 *
 * <b>STEP 1: Write a MidiParser derivative.</b>
 * The MidiParser base class provides functionality
 * considered common to the task of parsing event-based
 * music. In order to parse a particular format, create
 * a derived class that implements, at minimum, the
 * following format-specific methods:
 *   - loadMusic
 *   - parseNextEvent
 *
 * In addition to the above functions, the derived class
 * may also override the default MidiParser behavior for
 * the following methods:
 *   - resetTracking
 *   - allNotesOff
 *   - unloadMusic
 *   - property
 *   - getTick
 *
 * Please see the documentation for these individual
 * functions for more information on their use.
 *
 * The naming convention for classes derived from
 * MidiParser is MidiParser_XXX, where "XXX" is some
 * short designator for the format the class will
 * support. For instance, the MidiParser derivative
 * for parsing the Standard MIDI File format is
 * MidiParser_SMF.
 *
 * <b>STEP 2: Create an object of your derived class.</b>
 * Each MidiParser object can parse at most one (1) song
 * at a time. However, a MidiParser object can be reused
 * to play another song once it is no longer needed to
 * play whatever it was playing. In other words, MidiParser
 * objects do not have to be destroyed and recreated from
 * one song to the next.
 *
 * <b>STEP 3: Specify a MidiDriver to send events to.</b>
 * MidiParser works by sending MIDI and meta events to a
 * MidiDriver. In the simplest configuration, you can plug
 * a single MidiParser directly into the output MidiDriver
 * being used. However, you can only plug in one at a time;
 * otherwise channel conflicts will occur. Furthermore,
 * meta events that may be needed to interactively control
 * music flow cannot be handled because they are being
 * sent directly to the output device.
 *
 * If you need more control over the MidiParser while it's
 * playing, you can create your own "pseudo-MidiDriver" and
 * place it in between your MidiParser and the output
 * MidiDriver. The MidiParser will send events to your
 * pseudo-MidiDriver, which in turn must send them to the
 * output MidiDriver (or do whatever special handling is
 * required).
 *
 * To specify the MidiDriver to send music output to,
 * use the MidiParser::setMidiDriver method.
 *
 * <b>STEP 4: Specify the onTimer call rate.</b>
 * MidiParser bases the timing of its parsing on an external
 * clock. Every time MidiParser::onTimer is called, a bit
 * more music is parsed. You must specify how many
 * microseconds will occur between each call to onTimer,
 * in order to ensure an accurate music tempo.
 *
 * To set the onTimer call rate, in microseconds,
 * use the MidiParser::setTimerRate method. The onTimer
 * call rate will typically match the timer rate for
 * the output MidiDriver used. This rate can be obtained
 * by calling MidiDriver::getBaseTempo.
 *
 * <b>STEP 5: Load the music.</b>
 * MidiParser requires that the music data already be loaded
 * into memory. The client code is responsible for memory
 * management on this block of memory. That means that the
 * client code must ensure that the data remain in memory
 * while the MidiParser is using it, and properly freed
 * after it is no longer needed. Some MidiParser variants may
 * require internal buffers as well; memory management for those
 * buffers is the responsibility of the MidiParser object.
 *
 * To load the music into the MidiParser, use the
 * MidiParser::loadMusic method, specifying a memory pointer
 * to the music data and the size of the data. (NOTE: Some
 * MidiParser variants don't require a size, and 0 is fine.
 * However, when writing client code to use MidiParser, it is
 * best to assume that a valid size will be required.
 *
 * Convention requires that each implementation of
 * MidiParser::loadMusic automatically set up default tempo
 * and current track. This effectively means that the
 * MidiParser will start playing as soon as timer events
 * start coming in.
 *
 * <b>STEP 6: Activate a timer source for the MidiParser.</b>
 * The easiest timer source to use is the timer of the
 * output MidiDriver. You can attach the MidiDriver's
 * timer output directly to a MidiParser by calling
 * MidiDriver::setTimerCallback. In this case, the timer_proc
 * will be the static method MidiParser::timerCallback,
 * and timer_param will be a pointer to your MidiParser object.
 *
 * This configuration only allows one MidiParser to be driven
 * by the MidiDriver at a time. To drive more MidiDrivers, you
 * will need to create a "pseudo-MidiDriver" as described earlier,
 * In such a configuration, the pseudo-MidiDriver should be set
 * as the timer recipient in MidiDriver::setTimerCallback, and
 * could then call MidiParser::onTimer for each MidiParser object.
 *
 * <b>STEP 7: Music shall begin to play!</b>
 * Congratulations! At this point everything should be hooked up
 * and the MidiParser should generate music. Note that there is
 * no way to "stop" the MidiParser. You can "pause" the MidiParser
 * simply by not sending timer events to it, or you can call
 * MidiParser::unloadMusic to permanently stop the music. (This
 * method resets everything and detaches the MidiParser from the
 * memory block containing the music data.)
 */
class MidiParser {
protected:
	uint16    _activeNotes[128];   ///< Each uint16 is a bit mask for channels that have that note on.
	NoteTimer _hangingNotes[32];   ///< Maintains expiration info for up to 32 notes.
	                                ///< Used for "Smart Jump" and MIDI formats that do not include explicit Note Off events.
	byte      _hangingNotesCount; ///< Count of hanging notes, used to optimize expiration.

	MidiDriver_BASE *_driver;    ///< The device to which all events will be transmitted.
	uint32 _timerRate;     ///< The time in microseconds between onTimer() calls. Obtained from the MidiDriver.
	uint32 _ppqn;           ///< Pulses Per Quarter Note. (We refer to "pulses" as "ticks".)
	uint32 _tempo;          ///< Microseconds per quarter note.
	uint32 _psecPerTick;  ///< Microseconds per tick (_tempo / _ppqn).
	bool   _autoLoop;       ///< For lightweight clients that don't provide their own flow control.
	bool   _smartJump;      ///< Support smart expiration of hanging notes when jumping
	bool   _centerPitchWheelOnUnload;  ///< Center the pitch wheels when unloading a song
	bool   _sendSustainOffOnNotesOff;   ///< Send a sustain off on a notes off event, stopping hanging notes
	byte  *_tracks[120];    ///< Multi-track MIDI formats are supported, up to 120 tracks.
	byte   _numTracks;     ///< Count of total tracks for multi-track MIDI formats. 1 for single-track formats.
	byte   _activeTrack;   ///< Keeps track of the currently active track, in multi-track formats.

	Tracker _position;      ///< The current time/position in the active track.
	EventInfo _nextEvent;  ///< The next event to transmit. Events are preparsed
	                        ///< so each event is parsed only once; this permits
	                        ///< simulated events in certain formats.
	bool   _abortParse;    ///< If a jump or other operation interrupts parsing, flag to abort.

protected:
	static uint32 readVLQ(byte * &data);
	virtual void resetTracking();
	virtual void allNotesOff();
	virtual void parseNextEvent(EventInfo &info) = 0;

	void activeNote(byte channel, byte note, bool active);
	void hangingNote(byte channel, byte note, uint32 ticksLeft, bool recycle = true);
	void hangAllActiveNotes();

	virtual void sendToDriver(uint32 b);
	void sendToDriver(byte status, byte firstOp, byte secondOp) {
		sendToDriver(status | ((uint32)firstOp << 8) | ((uint32)secondOp << 16));
	}

	/**
	 * Platform independent BE uint32 read-and-advance.
	 * This helper function reads Big Endian 32-bit numbers
	 * from a memory pointer, at the same time advancing
	 * the pointer.
	 */
	uint32 read4high(byte * &data) {
		uint32 val = READ_BE_UINT32(data);
		data += 4;
		return val;
	}

	/**
	 * Platform independent LE uint16 read-and-advance.
	 * This helper function reads Little Endian 16-bit numbers
	 * from a memory pointer, at the same time advancing
	 * the pointer.
	 */
	uint16 read2low(byte * &data) {
		uint16 val = READ_LE_UINT16(data);
		data += 2;
		return val;
	}

public:
	/**
	 * Configuration options for MidiParser
	 * The following options can be set to modify MidiParser's
	 * behavior.
	 */
	enum {
		/**
		 * Events containing a pitch bend command should be treated as
		 * single-byte padding before the  real event. This allows the
		 * MidiParser to work with some malformed SMF files from Simon 1/2.
		 */
		mpMalformedPitchBends = 1,

		/**
		 * Sets auto-looping, which can be used by lightweight clients
		 * that don't provide their own flow control.
		 */
		mpAutoLoop = 2,

		/**
		 * Sets smart jumping, which intelligently expires notes that are
		 * active when a jump is made, rather than just cutting them off.
		 */
		mpSmartJump = 3,

		/**
		 * Center the pitch wheels when unloading music in preparation
		 * for the next piece of music.
		 */
		mpCenterPitchWheelOnUnload = 4,

		/**
		 * Sends a sustain off event when a notes off event is triggered.
		 * Stops hanging notes.
		 */
		 mpSendSustainOffOnNotesOff = 5
	};

public:
	typedef void (*XMidiCallbackProc)(byte eventData, void *refCon);

	MidiParser();
	virtual ~MidiParser() { allNotesOff(); }

	virtual bool loadMusic(byte *data, uint32 size) = 0;
	virtual void unloadMusic();
	virtual void property(int prop, int value);

	void setMidiDriver(MidiDriver_BASE *driver) { _driver = driver; }
	void setTimerRate(uint32 rate) { _timerRate = rate; }
	void setTempo(uint32 tempo);
	void onTimer();

	bool isPlaying() const { return (_position._playPos != 0); }
	void stopPlaying();

	bool setTrack(int track);
	bool jumpToTick(uint32 tick, bool fireEvents = false, bool stopNotes = true, bool dontSendNoteOn = false);

	uint32 getPPQN() { return _ppqn; }
	virtual uint32 getTick() { return _position._playTick; }

	static void defaultXMidiCallback(byte eventData, void *refCon);

	static MidiParser *createParser_SMF();
	static MidiParser *createParser_XMIDI(XMidiCallbackProc proc = defaultXMidiCallback, void *refCon = 0);
	static void timerCallback(void *data) { ((MidiParser *) data)->onTimer(); }
};

#endif
