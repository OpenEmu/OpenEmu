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

#include "sci/sci.h"

#include "common/config-manager.h"
#include "common/file.h"
#include "common/memstream.h"
#include "common/system.h"

#include "audio/fmopl.h"

#include "sci/resource.h"
#include "sci/engine/features.h"
#include "sci/sound/drivers/gm_names.h"
#include "sci/sound/drivers/mididriver.h"
#include "sci/sound/drivers/map-mt32-to-gm.h"

namespace Sci {

Mt32ToGmMapList *Mt32dynamicMappings = NULL;

class MidiPlayer_Midi : public MidiPlayer {
public:
	enum {
		kVoices = 32,
		kReverbConfigNr = 11,
		kMaxSysExSize = 264
	};

	MidiPlayer_Midi(SciVersion version);
	virtual ~MidiPlayer_Midi();

	int open(ResourceManager *resMan);
	void close();
	void send(uint32 b);
	void sysEx(const byte *msg, uint16 length);
	bool hasRhythmChannel() const { return true; }
	byte getPlayId() const;
	int getPolyphony() const {
		if (g_sci && g_sci->_features->useAltWinGMSound())
			return 16;
		else
			return kVoices;
	}
	int getFirstChannel() const;
	int getLastChannel() const;
	void setVolume(byte volume);
	int getVolume();
	void setReverb(int8 reverb);
	void playSwitch(bool play);

private:
	bool isMt32GmPatch(const byte *data, int size);
	void readMt32GmPatch(const byte *data, int size);
	void readMt32Patch(const byte *data, int size);
	void readMt32DrvData();

	void mapMt32ToGm(byte *data, size_t size);
	uint8 lookupGmInstrument(const char *iname);
	uint8 lookupGmRhythmKey(const char *iname);
	uint8 getGmInstrument(const Mt32ToGmMap &Mt32Ins);

	void sendMt32SysEx(const uint32 addr, Common::SeekableReadStream *str, int len, bool noDelay);
	void sendMt32SysEx(const uint32 addr, const byte *buf, int len, bool noDelay);
	void setMt32Volume(byte volume);
	void resetMt32();

	void noteOn(int channel, int note, int velocity);
	void setPatch(int channel, int patch);
	void controlChange(int channel, int control, int value);

	struct Channel {
		byte mappedPatch;
		byte patch;
		int velocityMapIdx;
		bool playing;
		int8 keyShift;
		int8 volAdjust;
		uint8 pan;
		uint8 hold;
		uint8 volume;

		Channel() : mappedPatch(MIDI_UNMAPPED), patch(MIDI_UNMAPPED), velocityMapIdx(0), playing(false),
			keyShift(0), volAdjust(0), pan(0x40), hold(0), volume(0x7f) { }
	};

	bool _isMt32;
	bool _useMT32Track;
	bool _hasReverb;
	bool _playSwitch;
	int _masterVolume;

	byte _reverbConfig[kReverbConfigNr][3];
	Channel _channels[16];
	uint8 _percussionMap[128];
	int8 _keyShift[128];
	int8 _volAdjust[128];
	uint8 _patchMap[128];
	uint8 _velocityMapIdx[128];
	uint8 _velocityMap[4][128];

	// These are extensions used for our own MT-32 to GM mapping
	uint8 _pitchBendRange[128];
	uint8 _percussionVelocityScale[128];

	byte _goodbyeMsg[20];
	byte _sysExBuf[kMaxSysExSize];
};

MidiPlayer_Midi::MidiPlayer_Midi(SciVersion version) : MidiPlayer(version), _playSwitch(true), _masterVolume(15), _isMt32(false), _hasReverb(false), _useMT32Track(true) {
	MidiDriver::DeviceHandle dev = MidiDriver::detectDevice(MDT_MIDI);
	_driver = MidiDriver::createMidi(dev);

	if (MidiDriver::getMusicType(dev) == MT_MT32 || ConfMan.getBool("native_mt32"))
		_isMt32 = true;

	_sysExBuf[0] = 0x41;
	_sysExBuf[1] = 0x10;
	_sysExBuf[2] = 0x16;
	_sysExBuf[3] = 0x12;

	Mt32dynamicMappings = new Mt32ToGmMapList();
}

MidiPlayer_Midi::~MidiPlayer_Midi() {
	delete _driver;

	const Mt32ToGmMapList::iterator end = Mt32dynamicMappings->end();
	for (Mt32ToGmMapList::iterator it = Mt32dynamicMappings->begin(); it != end; ++it) {
		delete[] (*it).name;
		(*it).name = 0;
	}

	Mt32dynamicMappings->clear();
	delete Mt32dynamicMappings;
}

void MidiPlayer_Midi::noteOn(int channel, int note, int velocity) {
	uint8 patch = _channels[channel].mappedPatch;

	assert(channel <= 15);
	assert(note <= 127);
	assert(velocity <= 127);

	if (channel == MIDI_RHYTHM_CHANNEL) {
		if (_percussionMap[note] == MIDI_UNMAPPED) {
			debugC(kDebugLevelSound, "[Midi] Percussion instrument %i is unmapped", note);
			return;
		}

		note = _percussionMap[note];
		// Scale velocity;
		velocity = velocity * _percussionVelocityScale[note] / 127;
	} else if (patch >= 128) {
		if (patch == MIDI_UNMAPPED)
			return;

		// Map to rhythm
		channel = MIDI_RHYTHM_CHANNEL;
		note = patch - 128;

		// Scale velocity;
		velocity = velocity * _percussionVelocityScale[note] / 127;
	} else {
		int8 keyshift = _channels[channel].keyShift;

		int shiftNote = note + keyshift;

		if (keyshift > 0) {
			while (shiftNote > 127)
				shiftNote -= 12;
		} else {
			while (shiftNote < 0)
				shiftNote += 12;
		}

		note = shiftNote;

		// We assume that velocity 0 maps to 0 (for note off)
		int mapIndex = _channels[channel].velocityMapIdx;
		assert(velocity <= 127);
		velocity = _velocityMap[mapIndex][velocity];
	}

	_channels[channel].playing = true;
	_driver->send(0x90 | channel, note, velocity);
}

void MidiPlayer_Midi::controlChange(int channel, int control, int value) {
	assert(channel <= 15);

	switch (control) {
	case 0x07:
		_channels[channel].volume = value;

		if (!_playSwitch)
			return;

		value += _channels[channel].volAdjust;

		if (value > 0x7f)
			value = 0x7f;

		if (value < 0)
			value = 1;

		value *= _masterVolume;

		if (value != 0) {
			value /= 15;

			if (value == 0)
				value = 1;
		}
		break;
	case 0x0a:
		if (_channels[channel].pan == value)
			return;

		_channels[channel].pan = value;
		break;
	case 0x40:
		if (_channels[channel].hold == value)
			return;

		_channels[channel].hold = value;
		break;
	case 0x4b:	// voice mapping
		break;
	case 0x4e:	// velocity
		break;
	case 0x7b:
		if (!_channels[channel].playing)
			return;

		_channels[channel].playing = false;
	default:
		break;
	}

	_driver->send(0xb0 | channel, control, value);
}

void MidiPlayer_Midi::setPatch(int channel, int patch) {
	bool resetVol = false;

	assert(channel <= 15);

	if ((channel == MIDI_RHYTHM_CHANNEL) || (_channels[channel].patch == patch))
		return;

	_channels[channel].patch = patch;
	_channels[channel].velocityMapIdx = _velocityMapIdx[patch];

	if (_channels[channel].mappedPatch == MIDI_UNMAPPED)
		resetVol = true;

	_channels[channel].mappedPatch = _patchMap[patch];

	if (_patchMap[patch] == MIDI_UNMAPPED) {
		debugC(kDebugLevelSound, "[Midi] Channel %i set to unmapped patch %i", channel, patch);
		_driver->send(0xb0 | channel, 0x7b, 0);
		_driver->send(0xb0 | channel, 0x40, 0);
		return;
	}

	if (_patchMap[patch] >= 128) {
		// Mapped to rhythm, don't send channel commands
		return;
	}

	if (_channels[channel].keyShift != _keyShift[patch]) {
		_channels[channel].keyShift = _keyShift[patch];
		_driver->send(0xb0 | channel, 0x7b, 0);
		_driver->send(0xb0 | channel, 0x40, 0);
		resetVol = true;
	}

	if (resetVol || (_channels[channel].volAdjust != _volAdjust[patch])) {
		_channels[channel].volAdjust = _volAdjust[patch];
		controlChange(channel, 0x07, _channels[channel].volume);
	}

	uint8 bendRange = _pitchBendRange[patch];
	if (bendRange != MIDI_UNMAPPED)
		_driver->setPitchBendRange(channel, bendRange);

	_driver->send(0xc0 | channel, _patchMap[patch], 0);

	// Send a pointless command to work around a firmware bug in common
	// USB-MIDI cables. If the first MIDI command in a USB packet is a
	// Cx or Dx command, the second command in the packet is dropped
	// somewhere.
	// FIXME: consider putting a workaround in the MIDI backend drivers
	// instead.
	// Known to be affected: alsa, coremidi
	// Known *not* to be affected: windows (only seems to send one MIDI
	// command per USB packet even if the device allows larger packets).
	_driver->send(0xb0 | channel, 0x0a, _channels[channel].pan);
}

void MidiPlayer_Midi::send(uint32 b) {
	byte command = b & 0xf0;
	byte channel = b & 0xf;
	byte op1 = (b >> 8) & 0x7f;
	byte op2 = (b >> 16) & 0x7f;

	// In early SCI0, we may also get events for AdLib rhythm channels.
	// While an MT-32 would ignore those with the default channel mapping,
	// we filter these out for the benefit of other MIDI devices.
	if (_version == SCI_VERSION_0_EARLY) {
		if (channel < 1 || channel > 9)
			return;
	}

	switch (command) {
	case 0x80:
		noteOn(channel, op1, 0);
		break;
	case 0x90:
		noteOn(channel, op1, op2);
		break;
	case 0xb0:
		controlChange(channel, op1, op2);
		break;
	case 0xc0:
		setPatch(channel, op1);
		break;
	// The original MIDI driver from sierra ignores aftertouch completely, so should we
	case 0xa0: // Polyphonic key pressure (aftertouch)
	case 0xd0: // Channel pressure (aftertouch)
		break;
	case 0xe0:
		_driver->send(b);
		break;
	default:
		warning("Ignoring MIDI event %02x", command);
	}
}

// We return 1 for mt32, because if we remap channels to 0 for mt32, those won't get played at all
// NOTE: SSCI uses channels 1 through 8 for General MIDI as well, in the drivers I checked
int MidiPlayer_Midi::getFirstChannel() const {
	if (_isMt32)
		return 1;
	return 0;
}

int MidiPlayer_Midi::getLastChannel() const {
	if (_isMt32)
		return 8;
	return 15;
}

void MidiPlayer_Midi::setVolume(byte volume) {
	_masterVolume = volume;

	if (!_playSwitch)
		return;

	for (uint i = 1; i < 10; i++) {
		if (_channels[i].volume != 0xff)
			controlChange(i, 0x07, _channels[i].volume & 0x7f);
	}
}

int MidiPlayer_Midi::getVolume() {
	return _masterVolume;
}

void MidiPlayer_Midi::setReverb(int8 reverb) {
	assert(reverb < kReverbConfigNr);

	if (_hasReverb && (_reverb != reverb))
		sendMt32SysEx(0x100001, _reverbConfig[reverb], 3, true);

	_reverb = reverb;
}

void MidiPlayer_Midi::playSwitch(bool play) {
	_playSwitch = play;
	if (play)
		setVolume(_masterVolume);
	else {
		for (uint i = 1; i < 10; i++)
			_driver->send(0xb0 | i, 7, 0);
	}
}

bool MidiPlayer_Midi::isMt32GmPatch(const byte *data, int size)
{
	if (size < 1155)
		return false;
	if (size > 16889)
		return true;

	bool isMt32 = false;
	bool isMt32Gm = false;

	if (READ_LE_UINT16(data + 1153) + 1155 == size)
		isMt32Gm = true;

	int pos = 492 + 246 * data[491];

	if ((size >= (pos + 386)) && (READ_BE_UINT16(data + pos) == 0xabcd))
		pos += 386;

	if ((size >= (pos + 267)) && (READ_BE_UINT16(data + pos) == 0xdcba))
		pos += 267;

	if (size == pos)
		isMt32 = true;

	if (isMt32 == isMt32Gm)
		error("Failed to detect MT-32 patch format");

	return isMt32Gm;
}

void MidiPlayer_Midi::sendMt32SysEx(const uint32 addr, Common::SeekableReadStream *str, int len, bool noDelay = false) {
	if (len + 8 > kMaxSysExSize) {
		warning("SysEx message exceed maximum size; ignoring");
		return;
	}

	uint16 chk = 0;

	_sysExBuf[4] = (addr >> 16) & 0xff;
	_sysExBuf[5] = (addr >> 8) & 0xff;
	_sysExBuf[6] = addr & 0xff;

	for (int i = 0; i < len; i++)
		_sysExBuf[7 + i] = str->readByte();

	for (int i = 4; i < 7 + len; i++)
		chk -= _sysExBuf[i];

	_sysExBuf[7 + len] = chk & 0x7f;

	if (noDelay)
		_driver->sysEx(_sysExBuf, len + 8);
	else
		sysEx(_sysExBuf, len + 8);
}

void MidiPlayer_Midi::sendMt32SysEx(const uint32 addr, const byte *buf, int len, bool noDelay = false) {
	Common::MemoryReadStream *str = new Common::MemoryReadStream(buf, len);
	sendMt32SysEx(addr, str, len, noDelay);
	delete str;
}

void MidiPlayer_Midi::readMt32Patch(const byte *data, int size) {
	Common::MemoryReadStream *str = new Common::MemoryReadStream(data, size);

	// Send before-SysEx text
	str->seek(0x14);
	sendMt32SysEx(0x200000, str, 20);

	// Save goodbye message
	str->read(_goodbyeMsg, 20);

	byte volume = CLIP<uint16>(str->readUint16LE(), 0, 100);
	setMt32Volume(volume);

	// Reverb default only used in (roughly) SCI0/SCI01
	byte reverb = str->readByte();

	_hasReverb = true;

	// Skip reverb SysEx message
	str->seek(11, SEEK_CUR);

	// Read reverb data (stored vertically - patch #3117434)
	for (int j = 0; j < 3; ++j) {
		for (int i = 0; i < kReverbConfigNr; i++) {
			_reverbConfig[i][j] = str->readByte();
		}
	}

	// Patches 1-48
	sendMt32SysEx(0x50000, str, 256);
	sendMt32SysEx(0x50200, str, 128);

	// Timbres
	byte timbresNr = str->readByte();
	for (int i = 0; i < timbresNr; i++)
		sendMt32SysEx(0x80000 + (i << 9), str, 246);

	uint16 flag = str->readUint16BE();

	if (!str->eos() && (flag == 0xabcd)) {
		// Patches 49-96
		sendMt32SysEx(0x50300, str, 256);
		sendMt32SysEx(0x50500, str, 128);
		flag = str->readUint16BE();
	}

	if (!str->eos() && (flag == 0xdcba)) {
		// Rhythm key map
		sendMt32SysEx(0x30110, str, 256);
		// Partial reserve
		sendMt32SysEx(0x100004, str, 9);
	}

	// Reverb for SCI0
	if (_version <= SCI_VERSION_0_LATE)
		setReverb(reverb);

	// Send after-SysEx text
	str->seek(0);
	sendMt32SysEx(0x200000, str, 20);

	// Send the mystery SysEx
	sendMt32SysEx(0x52000a, (const byte *)"\x16\x16\x16\x16\x16\x16", 6);

	delete str;
}

void MidiPlayer_Midi::readMt32GmPatch(const byte *data, int size) {
	memcpy(_patchMap, data, 0x80);
	memcpy(_keyShift, data + 0x80, 0x80);
	memcpy(_volAdjust, data + 0x100, 0x80);
	memcpy(_percussionMap, data + 0x180, 0x80);
	_channels[MIDI_RHYTHM_CHANNEL].volAdjust = data[0x200];
	memcpy(_velocityMapIdx, data + 0x201, 0x80);
	memcpy(_velocityMap, data + 0x281, 0x200);

	uint16 midiSize = READ_LE_UINT16(data + 0x481);

	if (midiSize > 0) {
		if (size < midiSize + 1155)
			error("Failed to read MIDI data");

		const byte *midi = data + 1155;
		byte command = 0;
		uint i = 0;

		while (i < midiSize) {
			byte op1, op2;

			if (midi[i] & 0x80)
				command = midi[i++];

			switch (command & 0xf0) {
			case 0xf0: {
				const byte *sysExEnd = (const byte *)memchr(midi + i, 0xf7, midiSize - i);

				if (!sysExEnd)
					error("Failed to find end of sysEx");

				int len = sysExEnd - (midi + i);
				sysEx(midi + i, len);

				i += len + 1; // One more for the 0x7f
				break;
			}
			case 0x80:
			case 0x90:
			case 0xa0:
			case 0xb0:
			case 0xe0:
				if (i + 1 >= midiSize)
					error("MIDI command exceeds data size");

				op1 = midi[i++];
				op2 = midi[i++];
				_driver->send(command, op1, op2);
				break;
			case 0xc0:
			case 0xd0:
				if (i >= midiSize)
					error("MIDI command exceeds data size");

				op1 = midi[i++];
				_driver->send(command, op1, 0);
				break;
			default:
				error("Failed to find MIDI command byte");
			}
		}
	}
}

void MidiPlayer_Midi::readMt32DrvData() {
	Common::File f;

	if (f.open("MT32.DRV")) {
		int size = f.size();

		// Skip before-SysEx text
		if (size == 1773 || size == 1759 || size == 1747)	// XMAS88 / KQ4 early (0.000.253 / 0.000.274)
			f.seek(0x59);
		else if (size == 2771)				// LSL2 early
			f.seek(0x29);
		else
			error("Unknown MT32.DRV size (%d)", size);

		// Skip 2 extra 0 bytes in some drivers
		if (f.readUint16LE() != 0)
			f.seek(-2, SEEK_CUR);

		// Send before-SysEx text
		sendMt32SysEx(0x200000, static_cast<Common::SeekableReadStream *>(&f), 20);

		if (size != 2271) {
			// Send after-SysEx text (SSCI sends this before every song).
			// There aren't any SysEx calls in old drivers, so this can
			// be sent right after the before-SysEx text.
			sendMt32SysEx(0x200000, static_cast<Common::SeekableReadStream *>(&f), 20);
		} else {
			// Skip the after-SysEx text in the newer patch version, we'll send
			// it after the SysEx messages are sent.
			f.skip(20);
		}

		// Save goodbye message. This isn't a C string, so it may not be
		// nul-terminated.
		f.read(_goodbyeMsg, 20);

		// Set volume
		byte volume = CLIP<uint16>(f.readUint16LE(), 0, 100);
		setMt32Volume(volume);

		if (size == 2771) {
			// MT32.DRV in LSL2 early contains more data, like a normal patch
			byte reverb = f.readByte();

			_hasReverb = true;

			// Skip reverb SysEx message
			f.skip(11);

			// Read reverb data (stored vertically - patch #3117434)
			for (int j = 0; j < 3; ++j) {
				for (int i = 0; i < kReverbConfigNr; i++) {
					_reverbConfig[i][j] = f.readByte();
				}
			}

			f.skip(2235);	// skip driver code

			// Patches 1-48
			sendMt32SysEx(0x50000, static_cast<Common::SeekableReadStream *>(&f), 256);
			sendMt32SysEx(0x50200, static_cast<Common::SeekableReadStream *>(&f), 128);

			setReverb(reverb);

			// Send the after-SysEx text
			f.seek(0x3d);
			sendMt32SysEx(0x200000, static_cast<Common::SeekableReadStream *>(&f), 20);
		} else {
			byte reverbSysEx[13];
			// This old driver should have a full reverb SysEx
			if ((f.read(reverbSysEx, 13) != 13) || (reverbSysEx[0] != 0xf0) || (reverbSysEx[12] != 0xf7))
				error("Error reading MT32.DRV");

			// Send reverb SysEx
			sysEx(reverbSysEx + 1, 11);
			_hasReverb = false;

			f.seek(0x29);

			// Read AdLib->MT-32 patch map
			for (int i = 0; i < 48; i++) {
				_patchMap[i] = f.readByte();
			}
		}

		f.close();
	} else {
		error("Failed to open MT32.DRV");
	}
}

byte MidiPlayer_Midi::lookupGmInstrument(const char *iname) {
	int i = 0;

	if (Mt32dynamicMappings != NULL) {
		const Mt32ToGmMapList::iterator end = Mt32dynamicMappings->end();
		for (Mt32ToGmMapList::iterator it = Mt32dynamicMappings->begin(); it != end; ++it) {
			if (scumm_strnicmp(iname, (*it).name, 10) == 0)
				return getGmInstrument((*it));
		}
	}

	while (Mt32MemoryTimbreMaps[i].name) {
		if (scumm_strnicmp(iname, Mt32MemoryTimbreMaps[i].name, 10) == 0)
			return getGmInstrument(Mt32MemoryTimbreMaps[i]);
		i++;
	}

	return MIDI_UNMAPPED;
}

byte MidiPlayer_Midi::lookupGmRhythmKey(const char *iname) {
	int i = 0;

	if (Mt32dynamicMappings != NULL) {
		const Mt32ToGmMapList::iterator end = Mt32dynamicMappings->end();
		for (Mt32ToGmMapList::iterator it = Mt32dynamicMappings->begin(); it != end; ++it) {
			if (scumm_strnicmp(iname, (*it).name, 10) == 0)
				return (*it).gmRhythmKey;
		}
	}

	while (Mt32MemoryTimbreMaps[i].name) {
		if (scumm_strnicmp(iname, Mt32MemoryTimbreMaps[i].name, 10) == 0)
			return Mt32MemoryTimbreMaps[i].gmRhythmKey;
		i++;
	}

	return MIDI_UNMAPPED;
}

uint8 MidiPlayer_Midi::getGmInstrument(const Mt32ToGmMap &Mt32Ins) {
	if (Mt32Ins.gmInstr == MIDI_MAPPED_TO_RHYTHM)
		return Mt32Ins.gmRhythmKey + 0x80;
	else
		return Mt32Ins.gmInstr;
}

void MidiPlayer_Midi::mapMt32ToGm(byte *data, size_t size) {
	// FIXME: Clean this up
	int memtimbres, patches;
	uint8 group, number, keyshift, /*finetune,*/ bender_range;
	uint8 *patchpointer;
	uint32 pos;
	int i;

	for (i = 0; i < 128; i++) {
		_patchMap[i] = getGmInstrument(Mt32PresetTimbreMaps[i]);
		_pitchBendRange[i] = 12;
	}

	for (i = 0; i < 128; i++)
		_percussionMap[i] = Mt32PresetRhythmKeymap[i];

	memtimbres = *(data + 0x1eb);
	pos = 0x1ec + memtimbres * 0xf6;

	if (size > pos && ((0x100 * *(data + pos) + *(data + pos + 1)) == 0xabcd)) {
		patches = 96;
		pos += 2 + 8 * 48;
	} else {
		patches = 48;
	}

	debugC(kDebugLevelSound, "[MT32-to-GM] %d MT-32 Patches detected", patches);
	debugC(kDebugLevelSound, "[MT32-to-GM] %d MT-32 Memory Timbres", memtimbres);

	debugC(kDebugLevelSound, "\n[MT32-to-GM] Mapping patches..");

	for (i = 0; i < patches; i++) {
		char name[11];

		if (i < 48)
			patchpointer = data + 0x6b + 8 * i;
		else
			patchpointer = data + 0x1ec + 8 * (i - 48) + memtimbres * 0xf6 + 2;

		group = *patchpointer;
		number = *(patchpointer + 1);
		keyshift = *(patchpointer + 2);
		//finetune = *(patchpointer + 3);
		bender_range = *(patchpointer + 4);

		debugCN(kDebugLevelSound, "  [%03d] ", i);

		switch (group) {
		case 1:
			number += 64;
			// Fall through
		case 0:
			_patchMap[i] = getGmInstrument(Mt32PresetTimbreMaps[number]);
			debugCN(kDebugLevelSound, "%s -> ", Mt32PresetTimbreMaps[number].name);
			break;
		case 2:
			if (number < memtimbres) {
				strncpy(name, (const char *)data + 0x1ec + number * 0xf6, 10);
				name[10] = 0;
				_patchMap[i] = lookupGmInstrument(name);
				debugCN(kDebugLevelSound, "%s -> ", name);
			} else {
				_patchMap[i] = 0xff;
				debugCN(kDebugLevelSound, "[Invalid]  -> ");
			}
			break;
		case 3:
			_patchMap[i] = getGmInstrument(Mt32RhythmTimbreMaps[number]);
			debugCN(kDebugLevelSound, "%s -> ", Mt32RhythmTimbreMaps[number].name);
			break;
		default:
			break;
		}

		if (_patchMap[i] == MIDI_UNMAPPED) {
			debugC(kDebugLevelSound, "[Unmapped]");
		} else {
#ifndef REDUCE_MEMORY_USAGE
			if (_patchMap[i] >= 128) {
				debugC(kDebugLevelSound, "%s [Rhythm]", GmPercussionNames[_patchMap[i] - 128]);
			} else {
				debugC(kDebugLevelSound, "%s", GmInstrumentNames[_patchMap[i]]);
			}
#endif
		}

		_keyShift[i] = CLIP<uint8>(keyshift, 0, 48) - 24;
		_pitchBendRange[i] = CLIP<uint8>(bender_range, 0, 24);
	}

	if (size > pos && ((0x100 * *(data + pos) + *(data + pos + 1)) == 0xdcba)) {
		debugC(kDebugLevelSound, "\n[MT32-to-GM] Mapping percussion..");

		for (i = 0; i < 64; i++) {
			number = *(data + pos + 4 * i + 2);
			byte ins = i + 24;

			debugCN(kDebugLevelSound, "  [%03d] ", ins);

			if (number < 64) {
				char name[11];
				strncpy(name, (const char *)data + 0x1ec + number * 0xf6, 10);
				name[10] = 0;
				debugCN(kDebugLevelSound, "%s -> ", name);
				_percussionMap[ins] = lookupGmRhythmKey(name);
			} else {
				if (number < 94) {
					debugCN(kDebugLevelSound, "%s -> ", Mt32RhythmTimbreMaps[number - 64].name);
					_percussionMap[ins] = Mt32RhythmTimbreMaps[number - 64].gmRhythmKey;
				} else {
					debugCN(kDebugLevelSound, "[Key  %03i] -> ", number);
					_percussionMap[ins] = MIDI_UNMAPPED;
				}
			}

#ifndef REDUCE_MEMORY_USAGE
			if (_percussionMap[ins] == MIDI_UNMAPPED)
				debugC(kDebugLevelSound, "[Unmapped]");
			else
				debugC(kDebugLevelSound, "%s", GmPercussionNames[_percussionMap[ins]]);
#endif

			_percussionVelocityScale[ins] = *(data + pos + 4 * i + 3) * 127 / 100;
		}
	}
}

void MidiPlayer_Midi::setMt32Volume(byte volume) {
	sendMt32SysEx(0x100016, &volume, 1);
}

void MidiPlayer_Midi::resetMt32() {
	sendMt32SysEx(0x7f0000, (const byte *)"\x01\x00", 2, true);

	// This seems to require a longer delay than usual
	g_system->delayMillis(150);
}

int MidiPlayer_Midi::open(ResourceManager *resMan) {
	assert(resMan != NULL);

	int retval = _driver->open();
	if (retval != 0) {
		warning("Failed to open MIDI driver");
		return retval;
	}

	// By default use no mapping
	for (uint i = 0; i < 128; i++) {
		_percussionMap[i] = i;
		_patchMap[i] = i;
		_velocityMap[0][i] = i;
		_velocityMap[1][i] = i;
		_velocityMap[2][i] = i;
		_velocityMap[3][i] = i;
		_keyShift[i] = 0;
		_volAdjust[i] = 0;
		_velocityMapIdx[i] = 0;
		_pitchBendRange[i] = MIDI_UNMAPPED;
		_percussionVelocityScale[i] = 127;
	}

	Resource *res = NULL;

	if (g_sci && g_sci->_features->useAltWinGMSound()) {
		res = resMan->findResource(ResourceId(kResourceTypePatch, 4), 0);
		if (!(res && isMt32GmPatch(res->data, res->size))) {
			// Don't do any mapping when a Windows alternative track is selected
			// and no MIDI patch is available
			_useMT32Track = false;
			return 0;
		}
	}

	if (_isMt32) {
		// MT-32
		resetMt32();

		res = resMan->findResource(ResourceId(kResourceTypePatch, 1), 0);

		if (res) {
			if (isMt32GmPatch(res->data, res->size)) {
				readMt32GmPatch(res->data, res->size);
				strncpy((char *)_goodbyeMsg, "      ScummVM       ", 20);
			} else {
				readMt32Patch(res->data, res->size);
			}
		} else {
			// Early SCI0 games have the sound bank embedded in the MT-32 driver
			readMt32DrvData();
		}
	} else {
		// General MIDI
		res = resMan->findResource(ResourceId(kResourceTypePatch, 4), 0);

		if (res && isMt32GmPatch(res->data, res->size)) {
			// There is a GM patch
			readMt32GmPatch(res->data, res->size);

			if (g_sci && g_sci->_features->useAltWinGMSound()) {
				// Always use the GM track if an alternative GM Windows soundtrack is selected
				_useMT32Track = false;
			} else {
				// Detect the format of patch 1, so that we know what play mask to use
				res = resMan->findResource(ResourceId(kResourceTypePatch, 1), 0);
				if (!res)
					_useMT32Track = false;
				else
					_useMT32Track = !isMt32GmPatch(res->data, res->size);

				// Check if the songs themselves have a GM track
				if (!_useMT32Track) {
					if (!resMan->isGMTrackIncluded())
						_useMT32Track = true;
				}
			}
		} else {
			// No GM patch found, map instruments using MT-32 patch

			warning("Game has no native support for General MIDI, applying auto-mapping");

			// TODO: The MT-32 <-> GM mapping hasn't been worked on for SCI1 games. Throw
			// a warning to the user
			if (getSciVersion() >= SCI_VERSION_1_EGA_ONLY)
				warning("The automatic mapping for General MIDI hasn't been worked on for "
						"SCI1 games. Music might sound wrong or broken. Please choose another "
						"music driver for this game (e.g. Adlib or MT-32) if you are "
						"experiencing issues with music");

			// Modify velocity map to make low velocity notes a little louder
			for (uint i = 1; i < 0x40; i++) {
				_velocityMap[0][i] = 0x20 + (i - 1) / 2;
				_velocityMap[1][i] = 0x20 + (i - 1) / 2;
				_velocityMap[2][i] = 0x20 + (i - 1) / 2;
				_velocityMap[3][i] = 0x20 + (i - 1) / 2;
			}

			res = resMan->findResource(ResourceId(kResourceTypePatch, 1), 0);

			if (res) {
				if (!isMt32GmPatch(res->data, res->size)) {
					mapMt32ToGm(res->data, res->size);
				} else {
					if (getSciVersion() <= SCI_VERSION_2_1) {
						error("MT-32 patch has wrong type");
					} else {
						// Happens in the SCI3 interactive demo of Lighthouse
						warning("TODO: Ignoring new SCI3 type of MT-32 patch for now (size = %d)", res->size);
					}
				}
			} else {
				// Early SCI0 games have the sound bank embedded in the MT-32 driver

				// No MT-32 patch present, try to read from MT32.DRV
				Common::File f;

				if (f.open("MT32.DRV")) {
					int size = f.size();

					assert(size >= 70);

					f.seek(0x29);

					// Read AdLib->MT-32 patch map
					for (int i = 0; i < 48; i++)
						_patchMap[i] = getGmInstrument(Mt32PresetTimbreMaps[f.readByte() & 0x7f]);
				}
			}
		}
	}

	return 0;
}

void MidiPlayer_Midi::close() {
	if (_isMt32) {
		// Send goodbye message
		sendMt32SysEx(0x200000, _goodbyeMsg, 20);
	}

	_driver->close();
}

void MidiPlayer_Midi::sysEx(const byte *msg, uint16 length) {
	_driver->sysEx(msg, length);

	// Wait the time it takes to send the SysEx data
	uint32 delay = (length + 2) * 1000 / 3125;

	// Plus an additional delay for the MT-32 rev00
	if (_isMt32)
		delay += 40;

	g_system->delayMillis(delay);
	g_system->updateScreen();
}

byte MidiPlayer_Midi::getPlayId() const {
	switch (_version) {
	case SCI_VERSION_0_EARLY:
	case SCI_VERSION_0_LATE:
		return 0x01;
	default:
		if (_isMt32)
			return 0x0c;
		else
			return _useMT32Track ? 0x0c : 0x07;
	}
}

MidiPlayer *MidiPlayer_Midi_create(SciVersion version) {
	return new MidiPlayer_Midi(version);
}

} // End of namespace Sci
