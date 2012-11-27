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
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */

#ifndef SCUMM_IMUSE_INTERNAL
#define SCUMM_IMUSE_INTERNAL

#include "common/scummsys.h"
#include "scumm/imuse/imuse.h"
#include "scumm/imuse/instrument.h"
#include "scumm/saveload.h"
#include "audio/mididrv.h"

class MidiParser;
class OSystem;

namespace Scumm {

struct ParameterFader;
struct DeferredCommand;
struct ImTrigger;
struct SustainingNotes;
struct CommandQueue;
struct IsNoteCmdData;
class  Player;
struct Part;
class  IMuseInternal;
class  IMuseSysex_Scumm;



//////////////////////////////////////////////////
//
// Some constants
//
//////////////////////////////////////////////////

#define TICKS_PER_BEAT 480

#define TRIGGER_ID 0
#define COMMAND_ID 1

#define MDPG_TAG "MDpg"


////////////////////////////////////////
//
//  Helper functions
//
////////////////////////////////////////

inline int clamp(int val, int min, int max) {
	if (val < min)
		return min;
	if (val > max)
		return max;
	return val;
}

inline int transpose_clamp(int a, int b, int c) {
	if (b > a)
		a += (b - a + 11) / 12 * 12;
	if (c < a)
		a -= (a - c + 11) / 12 * 12;
	return a;
}



//////////////////////////////////////////////////
//
// Entity declarations
//
//////////////////////////////////////////////////

struct TimerCallbackInfo {
	IMuseInternal *imuse;
	MidiDriver *driver;
};

struct HookDatas {
	byte _jump[2];
	byte _transpose;
	byte _part_onoff[16];
	byte _part_volume[16];
	byte _part_program[16];
	byte _part_transpose[16];

	int query_param(int param, byte chan);
	int set(byte cls, byte value, byte chan);
	HookDatas() { memset(this, 0, sizeof(HookDatas)); }
};

struct ParameterFader {
	enum {
		pfVolume = 1,
		pfTranspose = 3,
		pfSpeed = 4
	};

	int param;
	int start;
	int end;
	uint32 total_time;
	uint32 current_time;

	ParameterFader() { param = 0; }
	void init() { param = 0; }
};

struct DeferredCommand {
	uint32 time_left;
	int a, b, c, d, e, f;
	DeferredCommand() { memset(this, 0, sizeof(DeferredCommand)); }
};

struct ImTrigger {
	int sound;
	byte id;
	uint16 expire;
	int command[8];
	ImTrigger() { memset(this, 0, sizeof(ImTrigger)); }
};

struct CommandQueue {
	uint16 array[8];
	CommandQueue() { memset(this, 0, sizeof(CommandQueue)); }
};



//////////////////////////////////////////////////
//
// Player class definition
//
//////////////////////////////////////////////////

class Player : public MidiDriver_BASE {
	/*
	 * External SysEx handler functions shall each be defined in
	 * a separate file. This header file shall be included at the
	 * top of the file immediately following this special #define:
	 * #define SYSEX_CALLBACK_FUNCTION nameOfHandlerFunction
	 */
#ifdef SYSEX_CALLBACK_FUNCTION
	friend void SYSEX_CALLBACK_FUNCTION(Player *, const byte *, uint16);
#endif

protected:
	// Moved from IMuseInternal.
	// This is only used by one player at a time.
	static uint16 _active_notes[128];

protected:
	MidiDriver *_midi;
	MidiParser *_parser;

	Part *_parts;
	bool _active;
	bool _scanning;
	int _id;
	byte _priority;
	byte _volume;
	int8 _pan;
	int8 _transpose;
	int8 _detune;
	int _note_offset;
	byte _vol_eff;

	uint _track_index;
	uint _loop_to_beat;
	uint _loop_from_beat;
	uint _loop_counter;
	uint _loop_to_tick;
	uint _loop_from_tick;
	byte _speed;
	bool _abort;

	// This does not get used by us! It is only
	// here for save/load purposes, and gets
	// passed on to the MidiParser during
	// fixAfterLoad().
	uint32 _music_tick;

	HookDatas _hook;
	ParameterFader _parameterFaders[4];

	bool _isMT32;
	bool _isMIDI;
	bool _supportsPercussion;

protected:
	// Player part
	void hook_clear();
	void uninit_parts();
	byte *parse_midi(byte *s);
	void part_set_transpose(uint8 chan, byte relative, int8 b);
	void parse_sysex(byte *p, uint len);
	void maybe_jump(byte cmd, uint track, uint beat, uint tick);
	void maybe_set_transpose(byte *data);
	void maybe_part_onoff(byte *data);
	void maybe_set_volume(byte *data);
	void maybe_set_program(byte *data);
	void maybe_set_transpose_part(byte *data);
	void turn_off_pedals();
	int  query_part_param(int param, byte chan);
	void turn_off_parts();
	void play_active_notes();

	void transitionParameters();

	static void decode_sysex_bytes(const byte *src, byte *dst, int len);

	// Sequencer part
	int start_seq_sound(int sound, bool reset_vars = true);
	void loadStartParameters(int sound);
	int query_param(int param);

public:
	IMuseInternal *_se;
	uint _vol_chan;

public:
	Player();
	virtual ~Player();

	int addParameterFader(int param, int target, int time);
	void clear();
	void clearLoop();
	void fixAfterLoad();
	Part *getActivePart(uint8 part);
	uint getBeatIndex();
	int8 getDetune() const { return _detune; }
	byte getEffectiveVolume() const { return _vol_eff; }
	int getID() const { return _id; }
	MidiDriver *getMidiDriver() const { return _midi; }
	int getParam(int param, byte chan);
	int8 getPan() const { return _pan; }
	Part *getPart(uint8 part);
	byte getPriority() const { return _priority; }
	uint getTicksPerBeat() const { return TICKS_PER_BEAT; }
	int8 getTranspose() const { return _transpose; }
	byte getVolume() const { return _volume; }
	bool isActive() const { return _active; }
	bool isFadingOut() const;
	bool isMIDI() const { return _isMIDI; }
	bool isMT32() const { return _isMT32; }
	bool jump(uint track, uint beat, uint tick);
	void onTimer();
	void removePart(Part *part);
	int scan(uint totrack, uint tobeat, uint totick);
	void saveLoadWithSerializer(Serializer *ser);
	int setHook(byte cls, byte value, byte chan) { return _hook.set(cls, value, chan); }
	void setDetune(int detune);
	void setOffsetNote(int offset);
	bool setLoop(uint count, uint tobeat, uint totick, uint frombeat, uint fromtick);
	void setPan(int pan);
	void setPriority(int pri);
	void setSpeed(byte speed);
	int setTranspose(byte relative, int b);
	int setVolume(byte vol);
	bool startSound(int sound, MidiDriver *midi);
	int getMusicTimer() const;

public:
	// MidiDriver interface
	void send(uint32 b);
	void sysEx(const byte *msg, uint16 length);
	void metaEvent(byte type, byte *data, uint16 length);
};



//////////////////////////////////////////////////
//
// Part pseudo-class definition
//
//////////////////////////////////////////////////

struct Part : public Serializable {
	IMuseInternal *_se;
	int _slot;
	Part *_next, *_prev;
	MidiChannel *_mc;
	Player *_player;
	int16 _pitchbend;
	byte _pitchbend_factor;
	int8 _transpose, _transpose_eff;
	byte _vol, _vol_eff;
	int8 _detune, _detune_eff;
	int8 _pan, _pan_eff;
	bool _on;
	byte _modwheel;
	bool _pedal;
	int8 _pri;
	byte _pri_eff;
	byte _chan;
	byte _effect_level;
	byte _chorus;
	byte _percussion;
	byte _bank;

	// New abstract instrument definition
	Instrument _instrument;
	bool _unassigned_instrument; // For diagnostic reporting purposes only

	// MidiChannel interface
	// (We don't currently derive from MidiChannel,
	//  but if we ever do, this will make it easy.)
	void noteOff(byte note);
	void noteOn(byte note, byte velocity);
	void programChange(byte value);
	void pitchBend(int16 value);
	void modulationWheel(byte value);
	void volume(byte value);
	void pitchBendFactor(byte value);
	void sustain(bool value);
	void effectLevel(byte value);
	void chorusLevel(byte value);
	void allNotesOff();

	void set_param(byte param, int value) { }
	void init();
	void setup(Player *player);
	void uninit();
	void off();
	void set_instrument(uint b);
	void set_instrument(byte *data);
	void set_instrument_pcspk(byte *data);
	void load_global_instrument(byte b);

	void set_transpose(int8 transpose);
	void set_detune(int8 detune);
	void set_pri(int8 pri);
	void set_pan(int8 pan);

	void set_onoff(bool on);
	void fix_after_load();

	void sendAll();
	bool clearToTransmit();

	Part();

	void saveLoadWithSerializer(Serializer *ser);

private:
	void sendPitchBend();
	void sendPanPosition(uint8 value);
	void sendEffectLevel(uint8 value);
};



/**
 * SCUMM implementation of IMuse.
 * This class implements the IMuse mixin interface for the SCUMM environment.
 */
class IMuseInternal : public IMuse {
	friend class Player;
	friend struct Part;

	/*
	 * External SysEx handler functions shall each be defined in
	 * a separate file. This header file shall be included at the
	 * top of the file immediately following this special #define:
	 * #define SYSEX_CALLBACK_FUNCTION nameOfHandlerFunction
	 */
#ifdef SYSEX_CALLBACK_FUNCTION
	friend void SYSEX_CALLBACK_FUNCTION(Player *, const byte *, uint16);
#endif

protected:
	bool _native_mt32;
	bool _enable_gs;
	bool _sc55;
	MidiDriver *_midi_adlib;
	MidiDriver *_midi_native;
	TimerCallbackInfo _timer_info_adlib;
	TimerCallbackInfo _timer_info_native;

	uint32 _game_id;

	// Plug-in SysEx handling. Right now this only supports one
	// custom SysEx handler for the hardcoded IMUSE_SYSEX_ID
	// manufacturer code. TODO: Expand this to support multiple
	// SysEx handlers for client-specified manufacturer codes.
	sysexfunc _sysex;

	OSystem *_system;
	Common::Mutex _mutex;

protected:
	bool _paused;
	bool _initialized;

	int _tempoFactor;

	int  _player_limit;       // Limits how many simultaneous music tracks are played
	bool _recycle_players;    // Can we stop a player in order to start another one?

	uint _queue_end, _queue_pos, _queue_sound;
	byte _queue_adding;

	byte _queue_marker;
	byte _queue_cleared;
	byte _master_volume; // Master volume. 0-255
	byte _music_volume; // Global music volume. 0-255

	uint16 _trigger_count;
	ImTrigger _snm_triggers[16]; // Sam & Max triggers
	uint16 _snm_trigger_index;

	uint16 _channel_volume[8];
	uint16 _channel_volume_eff[8]; // No Save
	uint16 _volchan_table[8];

	Player _players[8];
	Part _parts[32];

	bool _pcSpeaker;
	Instrument _global_instruments[32];
	CommandQueue _cmd_queue[64];
	DeferredCommand _deferredCommands[4];

protected:
	IMuseInternal();
	virtual ~IMuseInternal();

	int initialize(OSystem *syst, MidiDriver *nativeMidiDriver, MidiDriver *adlibMidiDriver);

	static void midiTimerCallback(void *data);
	void on_timer(MidiDriver *midi);

	enum ChunkType {
		kMThd = 1,
		kFORM = 2,
		kMDhd = 4,  // Used in MI2 and INDY4. Contain certain start parameters (priority, volume, etc. ) for the player.
		kMDpg = 8   // These chunks exist in DOTT and SAMNMAX. They don't get processed, however.
	};

	byte *findStartOfSound(int sound, int ct = (kMThd | kFORM));
	bool isMT32(int sound);
	bool isMIDI(int sound);
	bool supportsPercussion(int sound);
	int get_queue_sound_status(int sound) const;
	void handle_marker(uint id, byte data);
	int get_channel_volume(uint a);
	void initMidiDriver(TimerCallbackInfo *info);
	void initGM(MidiDriver *midi);
	void initMT32(MidiDriver *midi);
	void init_players();
	void init_parts();
	void init_queue();

	void sequencer_timers(MidiDriver *midi);

	MidiDriver *getBestMidiDriver(int sound);
	Player *allocate_player(byte priority);
	Part *allocate_part(byte pri, MidiDriver *midi);

	int32 ImSetTrigger(int sound, int id, int a, int b, int c, int d, int e, int f, int g, int h);
	int32 ImClearTrigger(int sound, int id);
	int32 ImFireAllTriggers(int sound);

	void addDeferredCommand(int time, int a, int b, int c, int d, int e, int f);
	void handleDeferredCommands(MidiDriver *midi);

	int enqueue_command(int a, int b, int c, int d, int e, int f, int g);
	int enqueue_trigger(int sound, int marker);
	int clear_queue();
	int query_queue(int param);
	Player *findActivePlayer(int id);

	int get_volchan_entry(uint a);
	int set_volchan_entry(uint a, uint b);
	int set_channel_volume(uint chan, uint vol);
	void update_volumes();
	void reset_tick();

	int set_volchan(int sound, int volchan);

	void fix_parts_after_load();
	void fix_players_after_load(ScummEngine *scumm);
	int setImuseMasterVolume(uint vol);

	void reallocateMidiChannels(MidiDriver *midi);
	void setGlobalInstrument(byte slot, byte *data);
	void copyGlobalInstrument(byte slot, Instrument *dest);
	bool isNativeMT32() { return _native_mt32; }

protected:
	// Internal mutex-free versions of the IMuse and MusicEngine methods.
	bool startSound_internal(int sound, int offset = 0);
	int stopSound_internal(int sound);
	int stopAllSounds_internal();
	int getSoundStatus_internal(int sound, bool ignoreFadeouts) const;
	int32 doCommand_internal(int a, int b, int c, int d, int e, int f, int g, int h);
	int32 doCommand_internal(int numargs, int args[]);

public:
	// IMuse interface
	void pause(bool paused);
	int save_or_load(Serializer *ser, ScummEngine *scumm);
	bool get_sound_active(int sound) const;
	int32 doCommand(int numargs, int args[]);
	uint32 property(int prop, uint32 value);
	virtual void addSysexHandler(byte mfgID, sysexfunc handler);

public:
	void startSoundWithNoteOffset(int sound, int offset);

	// MusicEngine interface
	void setMusicVolume(int vol);
	void startSound(int sound);
	void stopSound(int sound);
	void stopAllSounds();
	int getSoundStatus(int sound) const;
	int getMusicTimer();

public:
	// Factory function
	static IMuseInternal *create(OSystem *syst, MidiDriver *nativeMidiDriver, MidiDriver *adlibMidiDriver);
};

} // End of namespace Scumm

#endif
