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

#ifndef TOWNS_EUP_H
#define TOWNS_EUP_H

#include "audio/softsynth/fmtowns_pc98/towns_audio.h"

class TownsEuphonyDriver : public TownsAudioInterfacePluginDriver {
public:
	TownsEuphonyDriver(Audio::Mixer *mixer);
	virtual ~TownsEuphonyDriver();

	bool init();
	void reset();

	void loadInstrument(int chanType, int id, const uint8 *data);
	void loadWaveTable(const uint8 *data);
	void unloadWaveTable(int id);
	void reserveSoundEffectChannels(int num);

	int setMusicTempo(int tempo);
	int startMusicTrack(const uint8 *data, int trackSize, int startTick);
	void setMusicLoop(bool loop);
	void stopParser();
	bool parserIsPlaying() {return _playing; }
	void continueParsing();

	void playSoundEffect(int chan, int note, int velo, const uint8 *data);
	void stopSoundEffect(int chan);
	bool soundEffectIsPlaying(int chan);

	void chanPanPos(int chan, int mode);
	void chanPitch(int chan, int pitch);
	void chanVolume(int chan, int vol);

	void setOutputVolume(int chanType, int volLeft, int volRight);

	int configChan_enable(int tableEntry, int val);
	int configChan_setMode(int tableEntry, int val);
	int configChan_remap(int tableEntry, int val);
	int configChan_adjustVolume(int tableEntry, int val);
	int configChan_setTranspose(int tableEntry, int val);

	int assignChannel(int chan, int tableEntry);

	void timerCallback(int timerId);

	void setMusicVolume(int volume);
	void setSoundEffectVolume(int volume);

	TownsAudioInterface *intf() {
		return _intf;
	}

private:
	void resetTables();

	void resetTempo();
	void setTempoIntern(int tempo);
	void setTimerA(bool enable, int tempo);
	void setTimerB(bool enable, int tempo);

	void updatePulseCount();
	void updateTimeStampBase();
	void updateParser();
	void updateCheckEot();

	bool parseNext();
	void jumpNextLoop();

	void updateEventBuffer();
	void flushEventBuffer();
	void processBufferNote(int mode, int evt, int note, int velo);

	void resetControl();
	void resetControlIntern(int mode, int chan);
	uint8 appendEvent(uint8 evt, uint8 chan);

	void sendEvent(uint8 mode, uint8 command);

	typedef bool(TownsEuphonyDriver::*EuphonyOpcode)();
	bool evtSetupNote();
	bool evtPolyphonicAftertouch();
	bool evtControlPitch();
	bool evtInstrumentChanAftertouch();
	bool evtLoadInstrument();
	bool evtAdvanceTimestampOffset();
	bool evtTempo();
	bool evtModeOrdrChange();
	bool evtNotImpl() {
		return false;
	}

	uint8 applyTranspose(uint8 in);
	uint8 applyVolumeAdjust(uint8 in);

	void sendNoteOff();
	void sendNoteOn();
	void sendChanVolume();
	void sendPanPosition();
	void sendAllNotesOff();
	void sendSetInstrument();
	void sendPitch();

	int8 *_activeChannels;
	int8 *_sustainChannels;

	struct ActiveChannel {
		int8 chan;
		int8 next;
		uint8 note;
		uint8 sub;
	} *_assignedChannels;

	uint8 *_tEnable;
	uint8 *_tMode;
	uint8 *_tOrdr;
	int8 *_tLevel;
	int8 *_tTranspose;

	struct DlEvent {
		uint8 evt;
		uint8 mode;
		uint8 note;
		uint8 velo;
		uint16 len;
	} *_eventBuffer;
	int _bufferedEventsCount;

	uint8 _para[2];
	uint8 _paraCount;
	uint8 _command;

	uint8 _defaultBaseTickLen;
	uint8 _baseTickLen;
	uint32 _pulseCount;
	int _tempoControlMode;
	int _extraTimingControlRemainder;
	int _extraTimingControl;
	int _timerSetting;
	int8 _tempoDiff;
	int _tempoModifier;
	uint32 _timeStampDest;
	uint32 _timeStampBase;
	int8 _elapsedEvents;
	uint8 _deltaTicks;
	uint32 _tickCounter;
	uint8 _defaultTempo;
	int _trackTempo;

	bool _loop;
	bool _playing;
	bool _endOfTrack;
	bool _suspendParsing;

	const uint8 *_musicStart;
	const uint8 *_musicPos;
	uint32 _musicTrackSize;

	TownsAudioInterface *_intf;
};

#endif
