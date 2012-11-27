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



#include "base/version.h"

#include "common/util.h"
#include "common/system.h"
#include "common/endian.h"

#include "scumm/imuse/imuse.h"
#include "scumm/imuse/imuse_internal.h"
#include "scumm/imuse/instrument.h"
#include "scumm/resource.h"
#include "scumm/saveload.h"
#include "scumm/scumm.h"

namespace Scumm {

////////////////////////////////////////
//
//  IMuseInternal implementation
//
////////////////////////////////////////

IMuseInternal::IMuseInternal() :
	_native_mt32(false),
	_enable_gs(false),
	_sc55(false),
	_midi_adlib(NULL),
	_midi_native(NULL),
	_sysex(NULL),
	_paused(false),
	_initialized(false),
	_tempoFactor(0),
	_player_limit(ARRAYSIZE(_players)),
	_recycle_players(false),
	_queue_end(0),
	_queue_pos(0),
	_queue_sound(0),
	_queue_adding(0),
	_queue_marker(0),
	_queue_cleared(0),
	_master_volume(0),
	_music_volume(0),
	_trigger_count(0),
	_snm_trigger_index(0),
	_pcSpeaker(false) {
	memset(_channel_volume, 0, sizeof(_channel_volume));
	memset(_channel_volume_eff, 0, sizeof(_channel_volume_eff));
	memset(_volchan_table, 0, sizeof(_volchan_table));
}

IMuseInternal::~IMuseInternal() {
	// Do just enough stuff inside the mutex to
	// make sure any MIDI timing threads won't
	// interrupt us, and then do the rest outside
	// the mutex.
	{
		Common::StackLock lock(_mutex, "IMuseInternal::~IMuseInternal()");
		_initialized = false;
		stopAllSounds_internal();
	}

	if (_midi_adlib) {
		_midi_adlib->close();
		delete _midi_adlib;
		_midi_adlib = 0;
	}

	if (_midi_native) {
		if (_native_mt32) {
			// Reset the MT-32
			_midi_native->sysEx((const byte *) "\x41\x10\x16\x12\x7f\x00\x00\x01\x00", 9);
			_system->delayMillis(250);
		}

		_midi_native->close();
		delete _midi_native;
		_midi_native = 0;
	}
}

byte *IMuseInternal::findStartOfSound(int sound, int ct) {
	int32 size, pos;

	static const uint32 id[] = {
		MKTAG('M', 'T', 'h', 'd'),
		MKTAG('F', 'O', 'R', 'M'),
		MKTAG('M', 'D', 'h', 'd'),
		MKTAG('M', 'D', 'p', 'g')
	};

	byte *ptr = g_scumm->_res->_types[rtSound][sound]._address;

	if (ptr == NULL) {
		debug(1, "IMuseInternal::findStartOfSound(): Sound %d doesn't exist", sound);
		return NULL;
	}

	// Check for old-style headers first, like 'RO'
	int trFlag = (kMThd | kFORM);
	if (ptr[0] == 'R' && ptr[1] == 'O' && ptr[2] != 'L')
		return ct == trFlag ? ptr : 0;
	if (ptr[4] == 'S' && ptr[5] == 'O')
		return ct == trFlag ? ptr + 4 : 0;

	ptr += 4;
	size = READ_BE_UINT32(ptr);
	ptr += 4;

	// Okay, we're looking for one of those things: either
	// an 'MThd' tag (for SMF), or a 'FORM' tag (for XMIDI).
	size = 48; // Arbitrary; we should find our tag within the first 48 bytes of the resource
	pos = 0;
	while (pos < size) {
		for (int i = 0; i < ARRAYSIZE(id); ++i) {
			if ((ct & (1 << i)) && (READ_BE_UINT32(ptr + pos) == id[i]))
				return ptr + pos;
		}
		++pos; // We could probably iterate more intelligently
	}

	if (ct == (kMThd | kFORM))
		debug(3, "IMuseInternal::findStartOfSound(): Failed to align on sound %d", sound);

	return 0;
}

bool IMuseInternal::isMT32(int sound) {
	byte *ptr = g_scumm->_res->_types[rtSound][sound]._address;
	if (ptr == NULL)
		return false;

	uint32 tag = READ_BE_UINT32(ptr);
	switch (tag) {
	case MKTAG('A', 'D', 'L', ' '):
	case MKTAG('A', 'S', 'F', 'X'): // Special AD class for old AdLib sound effects
	case MKTAG('S', 'P', 'K', ' '):
		return false;

	case MKTAG('A', 'M', 'I', ' '):
	case MKTAG('R', 'O', 'L', ' '):
		return true;

	case MKTAG('M', 'A', 'C', ' '): // Occurs in the Mac version of FOA and MI2
		return false;

	case MKTAG('G', 'M', 'D', ' '):
		return false;

	case MKTAG('M', 'I', 'D', 'I'): // Occurs in Sam & Max
		// HE games use Roland music
		if (ptr[8] == 'H' && ptr[9] == 'S')
			return true;
		else
			return false;
	}

	// Old style 'RO' has equivalent properties to 'ROL'
	if (ptr[0] == 'R' && ptr[1] == 'O')
		return true;
	// Euphony tracks show as 'SO' and have equivalent properties to 'ADL'
	if (ptr[4] == 'S' && ptr[5] == 'O')
		return false;

	error("Unknown music type: '%c%c%c%c'", (char)tag >> 24, (char)tag >> 16, (char)tag >> 8, (char)tag);

	return false;
}

bool IMuseInternal::isMIDI(int sound) {
	byte *ptr = g_scumm->_res->_types[rtSound][sound]._address;
	if (ptr == NULL)
		return false;

	uint32 tag = READ_BE_UINT32(ptr);
	switch (tag) {
	case MKTAG('A', 'D', 'L', ' '):
	case MKTAG('A', 'S', 'F', 'X'): // Special AD class for old AdLib sound effects
	case MKTAG('S', 'P', 'K', ' '):
		return false;

	case MKTAG('A', 'M', 'I', ' '):
	case MKTAG('R', 'O', 'L', ' '):
		return true;

	case MKTAG('M', 'A', 'C', ' '): // Occurs in the Mac version of FOA and MI2
		return true;

	case MKTAG('G', 'M', 'D', ' '):
	case MKTAG('M', 'I', 'D', 'I'): // Occurs in Sam & Max
		return true;
	}

	// Old style 'RO' has equivalent properties to 'ROL'
	if (ptr[0] == 'R' && ptr[1] == 'O')
		return true;
	// Euphony tracks show as 'SO' and have equivalent properties to 'ADL'
	// FIXME: Right now we're pretending it's GM.
	if (ptr[4] == 'S' && ptr[5] == 'O')
		return true;

	error("Unknown music type: '%c%c%c%c'", (char)tag >> 24, (char)tag >> 16, (char)tag >> 8, (char)tag);

	return false;
}

bool IMuseInternal::supportsPercussion(int sound) {
	byte *ptr = g_scumm->_res->_types[rtSound][sound]._address;
	if (ptr == NULL)
		return false;

	uint32 tag = READ_BE_UINT32(ptr);
	switch (tag) {
	case MKTAG('A', 'D', 'L', ' '):
	case MKTAG('A', 'S', 'F', 'X'): // Special AD class for old AdLib sound effects
	case MKTAG('S', 'P', 'K', ' '):
		return false;

	case MKTAG('A', 'M', 'I', ' '):
	case MKTAG('R', 'O', 'L', ' '):
		return true;

	case MKTAG('M', 'A', 'C', ' '): // Occurs in the Mac version of FOA and MI2
		// This is MIDI, i.e. uses MIDI style program changes, but without a
		// special percussion channel.
		return false;

	case MKTAG('G', 'M', 'D', ' '):
	case MKTAG('M', 'I', 'D', 'I'): // Occurs in Sam & Max
		return true;
	}

	// Old style 'RO' has equivalent properties to 'ROL'
	if (ptr[0] == 'R' && ptr[1] == 'O')
		return true;
	// Euphony tracks show as 'SO' and have equivalent properties to 'ADL'
	// FIXME: Right now we're pretending it's GM.
	if (ptr[4] == 'S' && ptr[5] == 'O')
		return true;

	error("Unknown music type: '%c%c%c%c'", (char)tag >> 24, (char)tag >> 16, (char)tag >> 8, (char)tag);

	return false;
}

MidiDriver *IMuseInternal::getBestMidiDriver(int sound) {
	MidiDriver *driver = NULL;

	if (isMIDI(sound)) {
		if (_midi_native) {
			driver = _midi_native;
		} else {
			// Route it through AdLib anyway.
			driver = _midi_adlib;
		}
	} else {
		driver = _midi_adlib;
	}
	return driver;
}

Player *IMuseInternal::allocate_player(byte priority) {
	Player *player = _players, *best = NULL;
	int i;
	byte bestpri = 255;

	for (i = _player_limit; i != 0; i--, player++) {
		if (!player->isActive())
			return player;
		if (player->getPriority() < bestpri) {
			best = player;
			bestpri = player->getPriority();
		}
	}

	if (bestpri < priority || _recycle_players)
		return best;

	debug(1, "Denying player request");
	return NULL;
}

void IMuseInternal::init_players() {
	Player *player = _players;
	int i;

	for (i = ARRAYSIZE(_players); i != 0; i--, player++) {
		player->_se = this;
		player->clear(); // Used to just set _active to false
	}
}

void IMuseInternal::init_parts() {
	Part *part;
	int i;

	for (i = 0, part = _parts; i != ARRAYSIZE(_parts); i++, part++) {
		part->init();
		part->_se = this;
		part->_slot = i;
	}
}

////////////////////////////////////////
//
// IMuse mixin interface methods
//
////////////////////////////////////////

void IMuseInternal::on_timer(MidiDriver *midi) {
	Common::StackLock lock(_mutex, "IMuseInternal::on_timer()");
	if (_paused || !_initialized)
		return;

	if (midi == _midi_native || !_midi_native)
		handleDeferredCommands(midi);
	sequencer_timers(midi);
}

void IMuseInternal::pause(bool paused) {
	Common::StackLock lock(_mutex, "IMuseInternal::pause()");
	if (_paused == paused)
		return;
	int vol = _music_volume;
	if (paused)
		_music_volume = 0;
	update_volumes();
	_music_volume = vol;

	// Fix for Bug #817871. The MT-32 apparently fails
	// sometimes to respond to a channel volume message
	// (or only uses it for subsequent note events).
	// The result is hanging notes on pause. Reportedly
	// happens in the original distro, too. To fix that,
	// just send AllNotesOff to the channels.
	if (_midi_native && _native_mt32) {
		for (int i = 0; i < 16; ++i)
			_midi_native->send(123 << 8 | 0xB0 | i);
	}

	_paused = paused;
}

int IMuseInternal::save_or_load(Serializer *ser, ScummEngine *scumm) {
	Common::StackLock lock(_mutex, "IMuseInternal::save_or_load()");
	const SaveLoadEntry mainEntries[] = {
		MKLINE(IMuseInternal, _queue_end, sleUint8, VER(8)),
		MKLINE(IMuseInternal, _queue_pos, sleUint8, VER(8)),
		MKLINE(IMuseInternal, _queue_sound, sleUint16, VER(8)),
		MKLINE(IMuseInternal, _queue_adding, sleByte, VER(8)),
		MKLINE(IMuseInternal, _queue_marker, sleByte, VER(8)),
		MKLINE(IMuseInternal, _queue_cleared, sleByte, VER(8)),
		MKLINE(IMuseInternal, _master_volume, sleByte, VER(8)),
		MKLINE(IMuseInternal, _trigger_count, sleUint16, VER(8)),
		MKLINE(IMuseInternal, _snm_trigger_index, sleUint16, VER(54)),
		MKARRAY(IMuseInternal, _channel_volume[0], sleUint16, 8, VER(8)),
		MKARRAY(IMuseInternal, _volchan_table[0], sleUint16, 8, VER(8)),
		MKEND()
	};

	const SaveLoadEntry cmdQueueEntries[] = {
		MKARRAY(CommandQueue, array[0], sleUint16, 8, VER(23)),
		MKEND()
	};

	// VolumeFader is obsolete.
	const SaveLoadEntry volumeFaderEntries[] = {
		MK_OBSOLETE(VolumeFader, player, sleUint16, VER(8), VER(16)),
		MK_OBSOLETE(VolumeFader, active, sleUint8, VER(8), VER(16)),
		MK_OBSOLETE(VolumeFader, curvol, sleUint8, VER(8), VER(16)),
		MK_OBSOLETE(VolumeFader, speed_lo_max, sleUint16, VER(8), VER(16)),
		MK_OBSOLETE(VolumeFader, num_steps, sleUint16, VER(8), VER(16)),
		MK_OBSOLETE(VolumeFader, speed_hi, sleInt8, VER(8), VER(16)),
		MK_OBSOLETE(VolumeFader, direction, sleInt8, VER(8), VER(16)),
		MK_OBSOLETE(VolumeFader, speed_lo, sleInt8, VER(8), VER(16)),
		MK_OBSOLETE(VolumeFader, speed_lo_counter, sleUint16, VER(8), VER(16)),
		MKEND()
	};

	const SaveLoadEntry snmTriggerEntries[] = {
		MKLINE(ImTrigger, sound, sleInt16, VER(54)),
		MKLINE(ImTrigger, id, sleByte, VER(54)),
		MKLINE(ImTrigger, expire, sleUint16, VER(54)),
		MKARRAY(ImTrigger, command[0], sleUint16, 8, VER(54)),
		MKEND()
	};

	int i;

	ser->saveLoadEntries(this, mainEntries);
	ser->saveLoadArrayOf(_cmd_queue, ARRAYSIZE(_cmd_queue), sizeof(_cmd_queue[0]), cmdQueueEntries);
	ser->saveLoadArrayOf(_snm_triggers, ARRAYSIZE(_snm_triggers), sizeof(_snm_triggers[0]), snmTriggerEntries);

	// The players
	for (i = 0; i < ARRAYSIZE(_players); ++i)
		_players[i].saveLoadWithSerializer(ser);

	// The parts
	for (i = 0; i < ARRAYSIZE(_parts); ++i)
		_parts[i].saveLoadWithSerializer(ser);

	{
		// Load/save the instrument definitions, which were revamped with V11.
		Part *part = &_parts[0];
		if (ser->getVersion() >= VER(11)) {
			for (i = ARRAYSIZE(_parts); i; --i, ++part) {
				part->_instrument.saveOrLoad(ser);
			}
		} else {
			for (i = ARRAYSIZE(_parts); i; --i, ++part)
				part->_instrument.clear();
		}
	}

	// VolumeFader has been replaced with the more generic ParameterFader.
	// FIXME: replace this loop by something like
	// if (loading && version <= 16)  ser->skip(XXX bytes);
	for (i = 0; i < 8; ++i)
		ser->saveLoadEntries(0, volumeFaderEntries);

	if (ser->isLoading()) {
		// Load all sounds that we need
		fix_players_after_load(scumm);
		fix_parts_after_load();
		setImuseMasterVolume(_master_volume);

		if (_midi_native)
			reallocateMidiChannels(_midi_native);
		if (_midi_adlib)
			reallocateMidiChannels(_midi_adlib);
	}

	return 0;
}

bool IMuseInternal::get_sound_active(int sound) const {
	Common::StackLock lock(_mutex, "IMuseInternal::get_sound_active()");
	return getSoundStatus_internal(sound, false) != 0;
}

int32 IMuseInternal::doCommand(int numargs, int a[]) {
	Common::StackLock lock(_mutex, "IMuseInternal::doCommand()");
	return doCommand_internal(numargs, a);
}

uint32 IMuseInternal::property(int prop, uint32 value) {
	Common::StackLock lock(_mutex, "IMuseInternal::property()");
	switch (prop) {
	case IMuse::PROP_TEMPO_BASE:
		// This is a specified as a percentage of normal
		// music speed. The number must be an integer
		// ranging from 50 to 200(for 50% to 200% normal speed).
		if (value >= 50 && value <= 200)
			_tempoFactor = value;
		break;

	case IMuse::PROP_NATIVE_MT32:
		_native_mt32 = (value > 0);
		Instrument::nativeMT32(_native_mt32);
		if (_midi_native && _native_mt32)
			initMT32(_midi_native);
		break;

	case IMuse::PROP_GS:
		_enable_gs = (value > 0);

		// If True Roland MT-32 is not selected, run in GM or GS mode.
		// If it is selected, change the Roland GS synth to MT-32 mode.
		if (_midi_native && !_native_mt32)
			initGM(_midi_native);
		else if (_midi_native && _native_mt32 && _enable_gs) {
			_sc55 = true;
			initGM(_midi_native);
		}
		break;

	case IMuse::PROP_LIMIT_PLAYERS:
		if (value > 0 && value <= ARRAYSIZE(_players))
			_player_limit = (int)value;
		break;

	case IMuse::PROP_RECYCLE_PLAYERS:
		_recycle_players = (value != 0);
		break;

	case IMuse::PROP_GAME_ID:
		_game_id = value;
		break;

	case IMuse::PROP_PC_SPEAKER:
		_pcSpeaker = (value != 0);
		break;
	}

	return 0;
}

void IMuseInternal::addSysexHandler(byte mfgID, sysexfunc handler) {
	// TODO: Eventually support multiple sysEx handlers and pay
	// attention to the client-supplied manufacturer ID.
	Common::StackLock lock(_mutex, "IMuseInternal::property()");
	_sysex = handler;
}

void IMuseInternal::startSoundWithNoteOffset(int sound, int offset) {
	Common::StackLock lock(_mutex, "IMuseInternal::startSound()");
	startSound_internal(sound, offset);
}

////////////////////////////////////////
//
// MusicEngine interface methods
//
////////////////////////////////////////

void IMuseInternal::setMusicVolume(int vol) {
	Common::StackLock lock(_mutex, "IMuseInternal::setMusicVolume()");
	if (vol > 255)
		vol = 255;
	if (_music_volume == vol)
		return;
	_music_volume = vol;
	vol = _master_volume * _music_volume / 255;
	for (uint i = 0; i < ARRAYSIZE(_channel_volume); i++) {
		_channel_volume_eff[i] = _channel_volume[i] * vol / 255;
	}
	if (!_paused)
		update_volumes();
}

void IMuseInternal::startSound(int sound) {
	Common::StackLock lock(_mutex, "IMuseInternal::startSound()");
	startSound_internal(sound);
}

void IMuseInternal::stopSound(int sound) {
	Common::StackLock lock(_mutex, "IMuseInternal::stopSound()");
	stopSound_internal(sound);
}

void IMuseInternal::stopAllSounds() {
	Common::StackLock lock(_mutex, "IMuseInternal::stopAllSounds()");
	stopAllSounds_internal();
}

int IMuseInternal::getSoundStatus(int sound) const {
	Common::StackLock lock(_mutex, "IMuseInternal::getSoundStatus()");
	return getSoundStatus_internal(sound, true);
}

int IMuseInternal::getMusicTimer() {
	Common::StackLock lock(_mutex, "IMuseInternal::getMusicTimer()");
	int best_time = 0;
	const Player *player = _players;
	for (int i = ARRAYSIZE(_players); i; i--, player++) {
		if (player->isActive()) {
			int timer = player->getMusicTimer();
			if (timer > best_time)
				best_time = timer;
		}
	}
	return best_time;
}

////////////////////////////////////////
//
// Internal versions of the IMuse and
// MusicEngine base class methods.
// These methods assume the appropriate
// mutex locks have already been set,
// and may also have slightly different
// semantics than the interface methods.
//
////////////////////////////////////////

bool IMuseInternal::startSound_internal(int sound, int offset) {
	// Do not start a sound if it is already set to start on an ImTrigger
	// event. This fixes carnival music problems where a sound has been set
	// to trigger at the right time, but then is started up immediately
	// anyway, only to be restarted later when the trigger occurs.
	//
	// However, we have to make sure the sound with the trigger is actually
	// playing, otherwise the music may stop when Sam and Max are thrown
	// out of Bumpusville, because entering the mansion sets up a trigger
	// for a sound that isn't necessarily playing. This is somewhat related
	// to bug #780918.

	int i;
	ImTrigger *trigger = _snm_triggers;
	for (i = ARRAYSIZE(_snm_triggers); i; --i, ++trigger) {
		if (trigger->sound && trigger->id && trigger->command[0] == 8 && trigger->command[1] == sound && getSoundStatus_internal(trigger->sound, true))
			return false;
	}

	byte *ptr = findStartOfSound(sound);
	if (!ptr) {
		debug(2, "IMuseInternal::startSound(): Couldn't find sound %d", sound);
		return false;
	}

	// Check which MIDI driver this track should use.
	// If it's NULL, it ain't something we can play.
	MidiDriver *driver = getBestMidiDriver(sound);
	if (!driver)
		return false;

	// If the requested sound is already playing, start it over
	// from scratch. This was originally a hack to prevent Sam & Max
	// iMuse messiness while upgrading the iMuse engine, but it
	// is apparently necessary to deal with fade-and-restart
	// race conditions that were observed in MI2. Reference
	// Bug #590511 and Patch #607175 (which was reversed to fix
	// an FOA regression: Bug #622606).
	Player *player = findActivePlayer(sound);
	if (!player) {
		ptr = findStartOfSound(sound, IMuseInternal::kMDhd);
		player = allocate_player(ptr ? (READ_BE_UINT32(&ptr[4]) && ptr[10] ? ptr[10] : 128) : 128);
	}

	if (!player)
		return false;

	// WORKAROUND: This is to work around a problem at the Dino Bungie
	// Memorial.
	//
	// There are three pieces of music involved here:
	//
	// 80 - Main theme (looping)
	// 81 - Music when entering Rex's and Wally's room (not looping)
	// 82 - Music when listening to Rex or Wally
	//
	// When entering, tune 81 starts, tune 80 is faded down (not out) and
	// a trigger is set in tune 81 to fade tune 80 back up.
	//
	// When listening to Rex or Wally, tune 82 is started, tune 81 is faded
	// out and tune 80 is faded down even further.
	//
	// However, when tune 81 is faded out its trigger will cause tune 80 to
	// fade back up, resulting in two tunes being played simultaneously at
	// full blast. It's no use trying to keep tune 81 playing at volume 0.
	// It doesn't loop, so eventually it will terminate on its own.
	//
	// I don't know how the original interpreter handled this - or even if
	// it handled it at all - but it looks like sloppy scripting to me. Our
	// workaround is to clear the trigger if the player listens to Rex or
	// Wally before tune 81 has finished on its own.

	if (_game_id == GID_SAMNMAX && sound == 82 && getSoundStatus_internal(81, false))
		ImClearTrigger(81, 1);

	player->clear();
	player->setOffsetNote(offset);
	return player->startSound(sound, driver);
}

int IMuseInternal::stopSound_internal(int sound) {
	int r = -1;
	Player *player = findActivePlayer(sound);
	if (player) {
		player->clear();
		r = 0;
	}
	return r;
}

int IMuseInternal::stopAllSounds_internal() {
	clear_queue();
	Player *player = _players;
	for (int i = ARRAYSIZE(_players); i; i--, player++) {
		if (player->isActive())
			player->clear();
	}
	return 0;
}

int IMuseInternal::getSoundStatus_internal(int sound, bool ignoreFadeouts) const {
	const Player *player = _players;
	for (int i = ARRAYSIZE(_players); i; i--, player++) {
		if (player->isActive() && (!ignoreFadeouts || !player->isFadingOut())) {
			if (sound == -1)
				return player->getID();
			else if (player->getID() == (uint16)sound)
				return 1;
		}
	}
	return (sound == -1) ? 0 : get_queue_sound_status(sound);
}

int32 IMuseInternal::doCommand_internal(int a, int b, int c, int d, int e, int f, int g, int h) {
	int args[8];
	args[0] = a;
	args[1] = b;
	args[2] = c;
	args[3] = d;
	args[4] = e;
	args[5] = f;
	args[6] = g;
	args[7] = h;
	return doCommand_internal(8, args);
}

int32 IMuseInternal::doCommand_internal(int numargs, int a[]) {
	if (numargs < 1)
		return -1;

	int i;
	byte cmd = a[0] & 0xFF;
	byte param = a[0] >> 8;
	Player *player = NULL;

	if (!_initialized && (cmd || param))
		return -1;

	{
		Common::String string = "doCommand - ";
		string += Common::String::format("%d (%d/%d)", a[0], (int)param, (int)cmd);
		for (i = 1; i < numargs; ++i)
			string += Common::String::format(", %d", a[i]);
		debugC(DEBUG_IMUSE, "%s", string.c_str());
	}

	if (param == 0) {
		switch (cmd) {
		case 6:
			if (a[1] > 127)
				return -1;
			else {
				debug(0, "IMuse doCommand(6) - setImuseMasterVolume (%d)", a[1]);
				return setImuseMasterVolume((a[1] << 1) | (a[1] ? 0 : 1)); // Convert from 0-127 to 0-255
			}
		case 7:
			debug(0, "IMuse doCommand(7) - getMasterVolume (%d)", a[1]);
			return _master_volume / 2; // Convert from 0-255 to 0-127
		case 8:
			return startSound_internal(a[1]) ? 0 : -1;
		case 9:
			return stopSound_internal(a[1]);
		case 10: // FIXME: Sam and Max - Not sure if this is correct
			return stopAllSounds_internal();
		case 11:
			return stopAllSounds_internal();
		case 12:
			// Sam & Max: Player-scope commands
			player = findActivePlayer(a[1]);
			if (player == NULL)
				return -1;

			switch (a[3]) {
			case 6:
				// Set player volume.
				return player->setVolume(a[4]);
			default:
				error("IMuseInternal::doCommand(12) unsupported sub-command %d", a[3]);
			}
			return -1;
		case 13:
			return getSoundStatus_internal(a[1], true);
		case 14:
			// Sam and Max: Parameter fade
			player = findActivePlayer(a[1]);
			if (player)
				return player->addParameterFader(a[3], a[4], a[5]);
			return -1;

		case 15:
			// Sam & Max: Set hook for a "maybe" jump
			player = findActivePlayer(a[1]);
			if (player) {
				player->setHook(0, a[3], 0);
				return 0;
			}
			return -1;
		case 16:
			debug(0, "IMuse doCommand(16) - set_volchan (%d, %d)", a[1], a[2]);
			return set_volchan(a[1], a[2]);
		case 17:
			if (_game_id != GID_SAMNMAX) {
				debug(0, "IMuse doCommand(17) - set_channel_volume (%d, %d)", a[1], a[2]);
				return set_channel_volume(a[1], a[2]);
			} else {
				if (a[4]) {
					int b[16];
					memset(b, 0, sizeof(b));
					for (i = 0; i < numargs; ++i)
						b[i] = a[i];
					return ImSetTrigger(b[1], b[3], b[4], b[5], b[6], b[7], b[8], b[9], b[10], b[11]);
				} else {
					return ImClearTrigger(a[1], a[3]);
				}
			}
		case 18:
			if (_game_id != GID_SAMNMAX) {
				return set_volchan_entry(a[1], a[2]);
			} else {
				// Sam & Max: ImCheckTrigger.
				// According to Mike's notes to Ender,
				// this function returns the number of triggers
				// associated with a particular player ID and
				// trigger ID.
				a[0] = 0;
				for (i = 0; i < ARRAYSIZE(_snm_triggers); ++i) {
					if (_snm_triggers[i].sound == a[1] && _snm_triggers[i].id &&
					        (a[3] == -1 || _snm_triggers[i].id == a[3])) {
						++a[0];
					}
				}
				return a[0];
			}
		case 19:
			// Sam & Max: ImClearTrigger
			// This should clear a trigger that's been set up
			// with ImSetTrigger(cmd == 17). Seems to work....
			return ImClearTrigger(a[1], a[3]);
		case 20:
			// Sam & Max: Deferred Command
			addDeferredCommand(a[1], a[2], a[3], a[4], a[5], a[6], a[7]);
			return 0;
		case 2:
		case 3:
			return 0;
		default:
			error("doCommand(%d [%d/%d], %d, %d, %d, %d, %d, %d, %d) unsupported", a[0], param, cmd, a[1], a[2], a[3], a[4], a[5], a[6], a[7]);
		}
	} else if (param == 1) {
		if ((1 << cmd) & 0x783FFF) {
			player = findActivePlayer(a[1]);
			if (!player)
				return -1;
			if ((1 << cmd) & (1 << 11 | 1 << 22)) {
				assert(a[2] >= 0 && a[2] <= 15);
				player = (Player *)player->getPart(a[2]);
				if (!player)
					return -1;
			}
		}

		switch (cmd) {
		case 0:
			if (_game_id == GID_SAMNMAX) {
				if (a[3] == 1) // Measure number
					return ((player->getBeatIndex() - 1) >> 2) + 1;
				else if (a[3] == 2) // Beat number
					return player->getBeatIndex();
				return -1;
			} else {
				return player->getParam(a[2], a[3]);
			}
		case 1:
			if (_game_id == GID_SAMNMAX) {
				// FIXME: Could someone verify this?
				//
				// This jump instruction is known to be used in
				// the following cases:
				//
				// 1) Going anywhere on the USA map
				// 2) Winning the Wak-A-Rat game
				// 3) Losing or quitting the Wak-A-Rat game
				// 4) Conroy hitting Max with a golf club
				//
				// For all these cases the position parameters
				// are always the same: 2, 1, 0, 0.
				//
				// 5) When leaving the bigfoot party. The
				//    position parameters are: 3, 4, 300, 0
				// 6) At Frog Rock, when the UFO appears. The
				//    position parameters are: 10, 4, 400, 1
				//
				// The last two cases used to be buggy, so I
				// have made a change to how the last two
				// position parameters are handled. I still do
				// not know if it's correct, but it sounds
				// good to me at least.

				debug(0, "doCommand(%d [%d/%d], %d, %d, %d, %d, %d, %d, %d)", a[0], param, cmd, a[1], a[2], a[3], a[4], a[5], a[6], a[7]);
				player->jump(a[3] - 1, (a[4] - 1) * 4 + a[5], a[6] + ((a[7] * player->getTicksPerBeat()) >> 2));
			} else
				player->setPriority(a[2]);
			return 0;
		case 2:
			return player->setVolume(a[2]);
		case 3:
			player->setPan(a[2]);
			return 0;
		case 4:
			return player->setTranspose(a[2], a[3]);
		case 5:
			player->setDetune(a[2]);
			return 0;
		case 6:
			// WORKAROUND for bug #1324106. When playing the
			// "flourishes" as Rapp's body appears from his ashes,
			// MI2 sets up triggers to pause the music, in case the
			// animation plays too slowly, and then the music is
			// manually unpaused for the next part of the music.
			//
			// In ScummVM, the animation finishes slightly too
			// quickly, and the pause command is run *after* the
			// unpause command. So we work around it by ignoring
			// all attempts at pausing this particular sound.
			//
			// I could have sworn this wasn't needed after the
			// recent timer change, but now it looks like it's
			// still needed after all.
			if (_game_id != GID_MONKEY2 || player->getID() != 183 || a[2] != 0) {
				player->setSpeed(a[2]);
			}
			return 0;
		case 7:
			return player->jump(a[2], a[3], a[4]) ? 0 : -1;
		case 8:
			return player->scan(a[2], a[3], a[4]);
		case 9:
			return player->setLoop(a[2], a[3], a[4], a[5], a[6]) ? 0 : -1;
		case 10:
			player->clearLoop();
			return 0;
		case 11:
			((Part *)player)->set_onoff(a[3] != 0);
			return 0;
		case 12:
			return player->setHook(a[2], a[3], a[4]);
		case 13:
			return player->addParameterFader(ParameterFader::pfVolume, a[2], a[3]);
		case 14:
			return enqueue_trigger(a[1], a[2]);
		case 15:
			return enqueue_command(a[1], a[2], a[3], a[4], a[5], a[6], a[7]);
		case 16:
			return clear_queue();
		case 19:
			return player->getParam(a[2], a[3]);
		case 20:
			return player->setHook(a[2], a[3], a[4]);
		case 21:
			return -1;
		case 22:
			((Part *)player)->volume(a[3]);
			return 0;
		case 23:
			return query_queue(a[1]);
		case 24:
			return 0;
		default:
			error("doCommand(%d [%d/%d], %d, %d, %d, %d, %d, %d, %d) unsupported", a[0], param, cmd, a[1], a[2], a[3], a[4], a[5], a[6], a[7]);
			return -1;
		}
	}

	return -1;
}

// mixin end

void IMuseInternal::sequencer_timers(MidiDriver *midi) {
	Player *player = _players;
	int i;
	for (i = ARRAYSIZE(_players); i != 0; i--, player++) {
		if (player->isActive() && player->getMidiDriver() == midi) {
			player->onTimer();
		}
	}
}

void IMuseInternal::handle_marker(uint id, byte data) {
	if ((_queue_end == _queue_pos) || (_queue_adding && _queue_sound == id && data == _queue_marker))
		return;

	uint16 *p = _cmd_queue[_queue_end].array;
	if (p[0] != TRIGGER_ID || id != p[1] || data != p[2])
		return;

	_trigger_count--;
	_queue_cleared = false;
	_queue_end = (_queue_end + 1) % ARRAYSIZE(_cmd_queue);

	while (_queue_end != _queue_pos && _cmd_queue[_queue_end].array[0] == COMMAND_ID && !_queue_cleared) {
		p = _cmd_queue[_queue_end].array;
		doCommand_internal(p[1], p[2], p[3], p[4], p[5], p[6], p[7], 0);
		_queue_end = (_queue_end + 1) % ARRAYSIZE(_cmd_queue);
	}
}

int IMuseInternal::get_channel_volume(uint a) {
	if (a < 8)
		return _channel_volume_eff[a];
	return (_master_volume * _music_volume / 255) / 2;
}

Part *IMuseInternal::allocate_part(byte pri, MidiDriver *midi) {
	Part *part, *best = NULL;
	int i;

	for (i = ARRAYSIZE(_parts), part = _parts; i != 0; i--, part++) {
		if (!part->_player)
			return part;
		if (pri >= part->_pri_eff) {
			pri = part->_pri_eff;
			best = part;
		}
	}

	if (best) {
		best->uninit();
		reallocateMidiChannels(midi);
	} else {
		debug(1, "Denying part request");
	}
	return best;
}

int IMuseInternal::get_queue_sound_status(int sound) const {
	const uint16 *a;
	int i, j;

	j = _queue_pos;
	i = _queue_end;

	while (i != j) {
		a = _cmd_queue[i].array;
		if (a[0] == COMMAND_ID && a[1] == 8 && a[2] == (uint16)sound)
			return 2;
		i = (i + 1) % ARRAYSIZE(_cmd_queue);
	}

	for (i = 0; i < ARRAYSIZE(_deferredCommands); ++i) {
		if (_deferredCommands[i].time_left && _deferredCommands[i].a == 8 &&
		        _deferredCommands[i].b == sound) {
			return 2;
		}
	}

	return 0;
}

int IMuseInternal::set_volchan(int sound, int volchan) {
	int r;
	int i;
	int num;
	Player *player, *best, *sameid;

	r = get_volchan_entry(volchan);
	if (r == -1)
		return -1;

	if (r >= 8) {
		player = findActivePlayer(sound);
		if (player && player->_vol_chan != (uint16)volchan) {
			player->_vol_chan = volchan;
			player->setVolume(player->getVolume());
			return 0;
		}
		return -1;
	} else {
		best = NULL;
		num = 0;
		sameid = NULL;
		for (i = ARRAYSIZE(_players), player = _players; i != 0; i--, player++) {
			if (player->isActive()) {
				if (player->_vol_chan == (uint16)volchan) {
					num++;
					if (!best || player->getPriority() <= best->getPriority())
						best = player;
				} else if (player->getID() == (uint16)sound) {
					sameid = player;
				}
			}
		}
		if (sameid == NULL)
			return -1;
		if (num >= r)
			best->clear();
		player->_vol_chan = volchan;
		player->setVolume(player->getVolume());
		return 0;
	}
}

int IMuseInternal::clear_queue() {
	_queue_adding = false;
	_queue_cleared = true;
	_queue_pos = 0;
	_queue_end = 0;
	_trigger_count = 0;
	return 0;
}

int IMuseInternal::enqueue_command(int a, int b, int c, int d, int e, int f, int g) {
	uint16 *p;
	uint i;

	i = _queue_pos;

	if (i == _queue_end)
		return -1;

	if (a == -1) {
		_queue_adding = false;
		_trigger_count++;
		return 0;
	}

	p = _cmd_queue[_queue_pos].array;
	p[0] = COMMAND_ID;
	p[1] = a;
	p[2] = b;
	p[3] = c;
	p[4] = d;
	p[5] = e;
	p[6] = f;
	p[7] = g;

	i = (i + 1) % ARRAYSIZE(_cmd_queue);

	if (_queue_end != i) {
		_queue_pos = i;
		return 0;
	} else {
		_queue_pos = (i - 1) % ARRAYSIZE(_cmd_queue);
		return -1;
	}
}

int IMuseInternal::query_queue(int param) {
	switch (param) {
	case 0: // Get trigger count
		return _trigger_count;
	case 1: // Get trigger type
		if (_queue_end == _queue_pos)
			return -1;
		return _cmd_queue[_queue_end].array[1];
	case 2: // Get trigger sound
		if (_queue_end == _queue_pos)
			return 0xFF;
		return _cmd_queue[_queue_end].array[2];
	default:
		return -1;
	}
}

int IMuseInternal::setImuseMasterVolume(uint vol) {
	if (vol > 255)
		vol = 255;
	if (_master_volume == vol)
		return 0;
	_master_volume = vol;
	vol = _master_volume * _music_volume / 255;
	for (uint i = 0; i < ARRAYSIZE(_channel_volume); i++) {
		_channel_volume_eff[i] = _channel_volume[i] * vol / 255;
	}
	if (!_paused)
		update_volumes();
	return 0;
}

int IMuseInternal::enqueue_trigger(int sound, int marker) {
	uint16 *p;
	uint pos;

	pos = _queue_pos;

	p = _cmd_queue[pos].array;
	p[0] = TRIGGER_ID;
	p[1] = sound;
	p[2] = marker;

	pos = (pos + 1) % ARRAYSIZE(_cmd_queue);
	if (_queue_end == pos) {
		_queue_pos = (pos - 1) % ARRAYSIZE(_cmd_queue);
		return -1;
	}

	_queue_pos = pos;
	_queue_adding = true;
	_queue_sound = sound;
	_queue_marker = marker;
	return 0;
}

int32 IMuseInternal::ImSetTrigger(int sound, int id, int a, int b, int c, int d, int e, int f, int g, int h) {
	// Sam & Max: ImSetTrigger.
	// Sets a trigger for a particular player and
	// marker ID, along with doCommand parameters
	// to invoke at the marker. The marker is
	// represented by MIDI SysEx block 00 xx(F7)
	// where "xx" is the marker ID.
	uint16 oldest_trigger = 0;
	ImTrigger *oldest_ptr = NULL;

	int i;
	ImTrigger *trig = _snm_triggers;
	for (i = ARRAYSIZE(_snm_triggers); i; --i, ++trig) {
		if (!trig->id)
			break;
		// We used to only compare 'id' and 'sound' here, but at least
		// at the Dino Bungie Memorial that causes the music to stop
		// after getting the T-Rex tooth. See bug #888161.
		if (trig->id == id && trig->sound == sound && trig->command[0] == a)
			break;

		uint16 diff;
		if (trig->expire <= _snm_trigger_index)
			diff = _snm_trigger_index - trig->expire;
		else
			diff = 0x10000 - trig->expire + _snm_trigger_index;

		if (!oldest_ptr || oldest_trigger < diff) {
			oldest_ptr = trig;
			oldest_trigger = diff;
		}
	}

	// If we didn't find a trigger, see if we can expire one.
	if (!i) {
		if (!oldest_ptr)
			return -1;
		trig = oldest_ptr;
	}

	trig->id = id;
	trig->sound = sound;
	trig->expire = (++_snm_trigger_index & 0xFFFF);
	trig->command[0] = a;
	trig->command[1] = b;
	trig->command[2] = c;
	trig->command[3] = d;
	trig->command[4] = e;
	trig->command[5] = f;
	trig->command[6] = g;
	trig->command[7] = h;

	// If the command is to start a sound, stop that sound if it's already playing.
	// This fixes some carnival music problems.
	// NOTE: We ONLY do this if the sound that will trigger the command is actually
	// playing. Otherwise, there's a problem when exiting and re-entering the
	// Bumpusville mansion. Ref Bug #780918.
	if (trig->command[0] == 8 && getSoundStatus_internal(trig->command[1], true) && getSoundStatus_internal(sound, true))
		stopSound_internal(trig->command[1]);
	return 0;
}

int32 IMuseInternal::ImClearTrigger(int sound, int id) {
	int count = 0;
	int i;
	ImTrigger *trig = _snm_triggers;
	for (i = ARRAYSIZE(_snm_triggers); i; --i, ++trig) {
		if ((sound == -1 || trig->sound == sound) && trig->id && (id == -1 || trig->id == id)) {
			trig->sound = trig->id = 0;
			++count;
		}
	}
	return (count > 0) ? 0 : -1;
}

int32 IMuseInternal::ImFireAllTriggers(int sound) {
	if (!sound)
		return 0;
	int count = 0;
	int i;
	for (i = 0; i < ARRAYSIZE(_snm_triggers); ++i) {
		if (_snm_triggers[i].sound == sound) {
			_snm_triggers[i].sound = _snm_triggers[i].id = 0;
			doCommand_internal(8, _snm_triggers[i].command);
			++count;
		}
	}
	return (count > 0) ? 0 : -1;
}

int IMuseInternal::set_channel_volume(uint chan, uint vol) {
	if (chan >= 8 || vol > 127)
		return -1;

	_channel_volume[chan] = vol;
	_channel_volume_eff[chan] = _master_volume * _music_volume * vol / 255 / 255;
	update_volumes();
	return 0;
}

void IMuseInternal::update_volumes() {
	Player *player;
	int i;

	for (i = ARRAYSIZE(_players), player = _players; i != 0; i--, player++) {
		if (player->isActive())
			player->setVolume(player->getVolume());
	}
}

int IMuseInternal::set_volchan_entry(uint a, uint b) {
	if (a >= 8)
		return -1;
	_volchan_table[a] = b;
	return 0;
}

int HookDatas::query_param(int param, byte chan) {
	switch (param) {
	case 18:
		return _jump[0];
	case 19:
		return _transpose;
	case 20:
		return _part_onoff[chan];
	case 21:
		return _part_volume[chan];
	case 22:
		return _part_program[chan];
	case 23:
		return _part_transpose[chan];
	default:
		return -1;
	}
}

int HookDatas::set(byte cls, byte value, byte chan) {
	switch (cls) {
	case 0:
		if (value != _jump[0]) {
			_jump[1] = _jump[0];
			_jump[0] = value;
		}
		break;
	case 1:
		_transpose = value;
		break;
	case 2:
		if (chan < 16)
			_part_onoff[chan] = value;
		else if (chan == 16)
			memset(_part_onoff, value, 16);
		break;
	case 3:
		if (chan < 16)
			_part_volume[chan] = value;
		else if (chan == 16)
			memset(_part_volume, value, 16);
		break;
	case 4:
		if (chan < 16)
			_part_program[chan] = value;
		else if (chan == 16)
			memset(_part_program, value, 16);
		break;
	case 5:
		if (chan < 16)
			_part_transpose[chan] = value;
		else if (chan == 16)
			memset(_part_transpose, value, 16);
		break;
	default:
		return -1;
	}
	return 0;
}

Player *IMuseInternal::findActivePlayer(int id) {
	int i;
	Player *player = _players;

	for (i = ARRAYSIZE(_players); i != 0; i--, player++) {
		if (player->isActive() && player->getID() == (uint16)id)
			return player;
	}
	return NULL;
}

int IMuseInternal::get_volchan_entry(uint a) {
	if (a < 8)
		return _volchan_table[a];
	return -1;
}

IMuseInternal *IMuseInternal::create(OSystem *syst, MidiDriver *nativeMidiDriver, MidiDriver *adlibMidiDriver) {
	IMuseInternal *i = new IMuseInternal;
	i->initialize(syst, nativeMidiDriver, adlibMidiDriver);
	return i;
}

int IMuseInternal::initialize(OSystem *syst, MidiDriver *native_midi, MidiDriver *adlib_midi) {
	int i;

	_system = syst;
	_midi_native = native_midi;
	_midi_adlib = adlib_midi;
	if (native_midi != NULL) {
		_timer_info_native.imuse = this;
		_timer_info_native.driver = native_midi;
		initMidiDriver(&_timer_info_native);
	}
	if (adlib_midi != NULL) {
		_timer_info_adlib.imuse = this;
		_timer_info_adlib.driver = adlib_midi;
		initMidiDriver(&_timer_info_adlib);
	}

	if (!_tempoFactor)
		_tempoFactor = 100;
	_master_volume = 255;

	for (i = 0; i != 8; i++)
		_channel_volume[i] = _channel_volume_eff[i] = _volchan_table[i] = 127;

	init_players();
	init_queue();
	init_parts();

	_initialized = true;

	return 0;
}

void IMuseInternal::initMidiDriver(TimerCallbackInfo *info) {
	// Open MIDI driver
	int result = info->driver->open();
	if (result)
		error("IMuse initialization - %s", MidiDriver::getErrorName(result));

	// Connect to the driver's timer
	info->driver->setTimerCallback(info, &IMuseInternal::midiTimerCallback);
}

void IMuseInternal::initMT32(MidiDriver *midi) {
	byte buffer[52];
	char info[256] = "ScummVM ";
	int len;

	// Reset the MT-32
	midi->sysEx((const byte *) "\x41\x10\x16\x12\x7f\x00\x00\x01\x00", 9);
	_system->delayMillis(250);

	// Setup master tune, reverb mode, reverb time, reverb level,
	// channel mapping, partial reserve and master volume
	midi->sysEx((const byte *) "\x41\x10\x16\x12\x10\x00\x00\x40\x00\x04\x04\x04\x04\x04\x04\x04\x04\x04\x04\x00\x01\x02\x03\x04\x05\x06\x07\x08\x09\x64\x77", 31);
	_system->delayMillis(250);

	// Map percussion to notes 24 - 34 without reverb
	midi->sysEx((const byte *) "\x41\x10\x16\x12\x03\x01\x10\x40\x64\x07\x00\x4a\x64\x06\x00\x41\x64\x07\x00\x4b\x64\x08\x00\x45\x64\x06\x00\x44\x64\x0b\x00\x51\x64\x05\x00\x43\x64\x08\x00\x50\x64\x07\x00\x42\x64\x03\x00\x4c\x64\x07\x00\x44", 52);
	_system->delayMillis(250);

	// Compute version string (truncated to 20 chars max.)
	strcat(info, gScummVMVersion);
	len = strlen(info);
	if (len > 20)
		len = 20;

	// Display a welcome message on MT-32 displays.
	memcpy(&buffer[0], "\x41\x10\x16\x12\x20\x00\x00", 7);
	memcpy(&buffer[7], "                    ", 20);
	memcpy(buffer + 7 + (20 - len) / 2, info, len);
	byte checksum = 0;
	for (int i = 4; i < 27; ++i)
		checksum -= buffer[i];
	buffer[27] = checksum & 0x7F;
	midi->sysEx(buffer, 28);
	_system->delayMillis(1000);
}

void IMuseInternal::initGM(MidiDriver *midi) {
	byte buffer[11];
	int i;

	// General MIDI System On message
	// Resets all GM devices to default settings
	memcpy(&buffer[0], "\x7E\x7F\x09\x01", 4);
	midi->sysEx(buffer, 4);
	debug(2, "GM SysEx: GM System On");
	_system->delayMillis(200);

	if (_enable_gs) {
		// All GS devices recognize the GS Reset command,
		// even with Roland's ID. It is impractical to
		// support other manufacturers' devices for
		// further GS settings, as there are limitless
		// numbers of them out there that would each
		// require individual SysEx commands with unique IDs.

		// Roland GS SysEx ID
		memcpy(&buffer[0], "\x41\x10\x42\x12", 4);

		// GS Reset
		memcpy(&buffer[4], "\x40\x00\x7F\x00\x41", 5);
		midi->sysEx(buffer, 9);
		debug(2, "GS SysEx: GS Reset");
		_system->delayMillis(200);

		if (_sc55) {
			// This mode is for GS devices that support an MT-32-compatible
			// Map, such as the Roland Sound Canvas line of modules. It
			// will allow them to work with True MT-32 mode, but will
			// obviously still ignore MT-32 SysEx (and thus custom
			// instruments).

			// Set Channels 1-16 to SC-55 Map, then CM-64/32L Variation
			for (i = 0; i < 16; ++i) {
				midi->send((127 << 16) | (0  << 8) | (0xB0 | i));
				midi->send((1   << 16) | (32 << 8) | (0xB0 | i));
				midi->send((0   << 16) | (0  << 8) | (0xC0 | i));
			}
			debug(2, "GS Program Change: CM-64/32L Map Selected");

			// Set Percussion Channel to SC-55 Map (CC#32, 01H), then
			// Switch Drum Map to CM-64/32L (MT-32 Compatible Drums)
			midi->getPercussionChannel()->controlChange(0, 0);
			midi->getPercussionChannel()->controlChange(32, 1);
			midi->send(127 << 8 | 0xC0 | 9);
			debug(2, "GS Program Change: Drum Map is CM-64/32L");

		}

		// Set Master Chorus to 0. The MT-32 has no chorus capability.
		memcpy(&buffer[4], "\x40\x01\x3A\x00\x05", 5);
		midi->sysEx(buffer, 9);
		debug(2, "GS SysEx: Master Chorus Level is 0");

		// Set Channels 1-16 Reverb to 64, which is the
		// equivalent of MT-32 default Reverb Level 5
		for (i = 0; i < 16; ++i)
			midi->send((64   << 16) | (91 << 8) | (0xB0 | i));
		debug(2, "GM Controller 91 Change: Channels 1-16 Reverb Level is 64");

		// Set Channels 1-16 Pitch Bend Sensitivity to
		// 12 semitones; then lock the RPN by setting null.
		for (i = 0; i < 16; ++i) {
			midi->setPitchBendRange(i, 12);
		}
		debug(2, "GM Controller 6 Change: Channels 1-16 Pitch Bend Sensitivity is 12 semitones");

		// Set channels 1-16 Mod. LFO1 Pitch Depth to 4
		memcpy(&buffer[4], "\x40\x20\x04\x04\x18", 5);
		for (i = 0; i < 16; ++i) {
			buffer[5] = 0x20 + i;
			buffer[8] = 0x18 - i;
			midi->sysEx(buffer, 9);
		}
		debug(2, "GS SysEx: Channels 1-16 Mod. LFO1 Pitch Depth Level is 4");

		// Set Percussion Channel Expression to 80
		midi->getPercussionChannel()->controlChange(11, 80);
		debug(2, "GM Controller 11 Change: Percussion Channel Expression Level is 80");

		// Turn off Percussion Channel Rx. Expression so that
		// Expression cannot be modified. I don't know why, but
		// Roland does it this way.
		memcpy(&buffer[4], "\x40\x10\x0E\x00\x22", 5);
		midi->sysEx(buffer, 9);
		debug(2, "GS SysEx: Percussion Channel Rx. Expression is OFF");

		// Change Reverb Character to 0. I don't think this
		// sounds most like MT-32, but apparently Roland does.
		memcpy(&buffer[4], "\x40\x01\x31\x00\x0E", 5);
		midi->sysEx(buffer, 9);
		debug(2, "GS SysEx: Reverb Character is 0");

		// Change Reverb Pre-LF to 4, which is similar to
		// what MT-32 reverb does.
		memcpy(&buffer[4], "\x40\x01\x32\x04\x09", 5);
		midi->sysEx(buffer, 9);
		debug(2, "GS SysEx: Reverb Pre-LF is 4");

		// Change Reverb Time to 106; the decay on Hall 2
		// Reverb is too fast compared to the MT-32's
		memcpy(&buffer[4], "\x40\x01\x34\x6A\x21", 5);
		midi->sysEx(buffer, 9);
		debug(2, "GS SysEx: Reverb Time is 106");
	}
}

void IMuseInternal::init_queue() {
	_queue_adding = false;
	_queue_pos = 0;
	_queue_end = 0;
	_trigger_count = 0;
}

void IMuseInternal::handleDeferredCommands(MidiDriver *midi) {
	uint32 advance = midi->getBaseTempo();

	DeferredCommand *ptr = &_deferredCommands[0];
	int i;
	for (i = ARRAYSIZE(_deferredCommands); i; --i, ++ptr) {
		if (!ptr->time_left)
			continue;
		if (ptr->time_left <= advance) {
			doCommand_internal(ptr->a, ptr->b, ptr->c, ptr->d, ptr->e, ptr->f, 0, 0);
			ptr->time_left = advance;
		}
		ptr->time_left -= advance;
	}
}

// "time" is interpreted as hundredths of a second.
// FIXME: Is that correct?
// We convert it to microseconds before prceeding
void IMuseInternal::addDeferredCommand(int time, int a, int b, int c, int d, int e, int f) {
	DeferredCommand *ptr = &_deferredCommands[0];
	int i;
	for (i = ARRAYSIZE(_deferredCommands); i; --i, ++ptr) {
		if (!ptr->time_left)
			break;
	}

	if (i) {
		ptr->time_left = time * 10000;
		ptr->a = a;
		ptr->b = b;
		ptr->c = c;
		ptr->d = d;
		ptr->e = e;
		ptr->f = f;
	}
}

void IMuseInternal::fix_parts_after_load() {
	Part *part;
	int i;

	for (i = ARRAYSIZE(_parts), part = _parts; i != 0; i--, part++) {
		if (part->_player)
			part->fix_after_load();
	}
}

// Only call this routine from the main thread,
// since it uses getResourceAddress
void IMuseInternal::fix_players_after_load(ScummEngine *scumm) {
	Player *player = _players;
	int i;

	for (i = ARRAYSIZE(_players); i != 0; i--, player++) {
		if (player->isActive()) {
			scumm->getResourceAddress(rtSound, player->getID());
			player->fixAfterLoad();
		}
	}
}

////////////////////////////////////////
//
// Some more IMuseInternal stuff
//
////////////////////////////////////////

void IMuseInternal::midiTimerCallback(void *data) {
	TimerCallbackInfo *info = (TimerCallbackInfo *)data;
	info->imuse->on_timer(info->driver);
}

void IMuseInternal::reallocateMidiChannels(MidiDriver *midi) {
	Part *part, *hipart;
	int i;
	byte hipri, lopri;
	Part *lopart;

	while (true) {
		hipri = 0;
		hipart = NULL;
		for (i = 32, part = _parts; i; i--, part++) {
			if (part->_player && part->_player->getMidiDriver() == midi &&
			        !part->_percussion && part->_on &&
			        !part->_mc && part->_pri_eff >= hipri) {
				hipri = part->_pri_eff;
				hipart = part;
			}
		}

		if (!hipart)
			return;

		if ((hipart->_mc = midi->allocateChannel()) == NULL) {
			lopri = 255;
			lopart = NULL;
			for (i = 32, part = _parts; i; i--, part++) {
				if (part->_mc && part->_mc->device() == midi && part->_pri_eff <= lopri) {
					lopri = part->_pri_eff;
					lopart = part;
				}
			}

			if (lopart == NULL || lopri >= hipri)
				return;
			lopart->off();

			if ((hipart->_mc = midi->allocateChannel()) == NULL)
				return;
		}
		hipart->sendAll();
	}
}

void IMuseInternal::setGlobalInstrument(byte slot, byte *data) {
	if (slot < 32) {
		if (_pcSpeaker)
			_global_instruments[slot].pcspk(data);
		else
			_global_instruments[slot].adlib(data);
	}
}

void IMuseInternal::copyGlobalInstrument(byte slot, Instrument *dest) {
	if (slot >= 32)
		return;

	// Both the AdLib code and the PC Speaker code use an all zero instrument
	// as default in the original, thus we do the same.
	// PC Speaker instrument size is 23, while AdLib instrument size is 30.
	// Thus we just use a 30 byte instrument data array as default.
	const byte defaultInstr[30] = { 0 };

	if (_global_instruments[slot].isValid()) {
		// In case we have an valid instrument set up, copy it to the part.
		_global_instruments[slot].copy_to(dest);
	} else if (_pcSpeaker) {
		debug(0, "Trying to use non-existant global PC Speaker instrument %d", slot);
		dest->pcspk(defaultInstr);
	} else {
		debug(0, "Trying to use non-existant global AdLib instrument %d", slot);
		dest->adlib(defaultInstr);
	}
}



/**
 * IMuseInternal factory creation method.
 * This method provides a means for creating an IMuse
 * implementation without requiring that the details
 * of that implementation be exposed to the client
 * through a header file. This allows the internals
 * of the implementation to be changed and updated
 * without requiring a recompile of the client code.
 */
IMuse *IMuse::create(OSystem *syst, MidiDriver *nativeMidiDriver, MidiDriver *adlibMidiDriver) {
	IMuseInternal *engine = IMuseInternal::create(syst, nativeMidiDriver, adlibMidiDriver);
	return engine;
}

} // End of namespace Scumm
