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

#include "scumm/player_v2base.h"
#include "scumm/scumm.h"

#define FREQ_HZ 236 // Don't change!

#define MAX_OUTPUT 0x7fff

namespace Scumm {

const uint8 note_lengths[] = {
	0,
	0,  0,  2,
	0,  3,  4,
	5,  6,  8,
	9, 12, 16,
	18, 24, 32,
	36, 48, 64,
	72, 96
};

static const uint16 hull_offsets[] = {
	0, 12, 24, 36, 48, 60,
	72, 88, 104, 120, 136, 256,
	152, 164, 180
};

static const int16 hulls[] = {
	// hull 0
	3, -1, 0, 0, 0, 0, 0, 0,
	0, -1, 0, 0,
	// hull 1 (staccato)
	3, -1, 0, 32, 0, -1, 0, 0,
	0, -1, 0, 0,
	// hull 2 (legato)
	3, -1, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0,
	// hull 3 (staccatissimo)
	3, -1, 0, 2, 0, -1, 0, 0,
	0, -1, 0, 0,
	// hull 4
	3, -1, 0, 6, 0, -1, 0, 0,
	0, -1, 0, 0,
	// hull 5
	3, -1, 0, 16, 0, -1, 0, 0,
	0, -1, 0, 0,
	// hull 6
	(int16) 60000, -1, -1000, 20, 0, 0, 0, 0,
	(int16) 40000, -1, -5000,  5, 0, -1, 0, 0,
	// hull 7
	(int16) 50000, -1, 0, 8, 30000, -1, 0, 0,
	28000, -1, -5000,  5, 0, -1, 0, 0,
	// hull 8
	(int16) 60000, -1, -2000, 16, 0, 0, 0, 0,
	28000, -1, -6000,  5, 0, -1, 0, 0,
	// hull 9
	(int16) 55000, -1,     0,  8, (int16) 35000, -1, 0, 0,
	(int16) 40000, -1, -2000, 10, 0, -1, 0, 0,
	// hull 10
	(int16) 60000, -1,     0,  4, -2000, 8, 0, 0,
	(int16) 40000, -1, -6000,  5, 0, -1, 0, 0,
	// hull 12
	0, -1,   150, 340, -150, 340, 0, -1,
	0, -1, 0, 0,
	// hull 13  == 164
	20000, -1,  4000,  7, 1000, 15, 0, 0,
	(int16) 35000, -1, -2000, 15, 0, -1, 0, 0,

	// hull 14  == 180
	(int16) 35000, -1,   500, 20, 0,  0, 0, 0,
	(int16) 45000, -1,  -500, 60, 0, -1, 0, 0,

	// hull misc = 196
	(int16) 44000, -1, -4400, 10, 0, -1, 0, 0,
	0, -1, 0, 0,

	(int16) 53000, -1, -5300, 10, 0, -1, 0, 0,
	0, -1, 0, 0,

	(int16) 63000, -1, -6300, 10, 0, -1, 0, 0,
	0, -1, 0, 0,

	(int16) 44000, -1, -1375, 32, 0, -1, 0, 0,
	0, -1, 0, 0,

	(int16) 53000, -1, -1656, 32, 0, -1, 0, 0,
	0, -1, 0, 0,

	// hull 11 == 256
	(int16) 63000, -1, -1968, 32, 0, -1, 0, 0,
	0, -1, 0, 0,

	(int16) 44000, -1, - 733, 60, 0, -1, 0, 0,
	0, -1, 0, 0,

	(int16) 53000, -1, - 883, 60, 0, -1, 0, 0,
	0, -1, 0, 0,

	(int16) 63000, -1, -1050, 60, 0, -1, 0, 0,
	0, -1, 0, 0,

	(int16) 44000, -1, - 488, 90, 0, -1, 0, 0,
	0, -1, 0, 0,

	(int16) 53000, -1, - 588, 90, 0, -1, 0, 0,
	0, -1, 0, 0,

	(int16) 63000, -1, - 700, 90, 0, -1, 0, 0,
	0, -1, 0, 0
};

static const uint16 freqmod_lengths[] = {
	0x1000, 0x1000, 0x20, 0x2000, 0x1000
};

static const uint16 freqmod_offsets[] = {
	0, 0x100, 0x200, 0x302, 0x202
};

static const int8 freqmod_table[0x502] = {
     0,   3,   6,   9,  12,  15,  18,  21,
    24,  27,  30,  33,  36,  39,  42,  45,
    48,  51,  54,  57,  59,  62,  65,  67,
    70,  73,  75,  78,  80,  82,  85,  87,
    89,  91,  94,  96,  98, 100, 102, 103,
   105, 107, 108, 110, 112, 113, 114, 116,
   117, 118, 119, 120, 121, 122, 123, 123,
   124, 125, 125, 126, 126, 126, 126, 126,
   126, 126, 126, 126, 126, 126, 125, 125,
   124, 123, 123, 122, 121, 120, 119, 118,
   117, 116, 114, 113, 112, 110, 108, 107,
   105, 103, 102, 100,  98,  96,  94,  91,
    89,  87,  85,  82,  80,  78,  75,  73,
    70,  67,  65,  62,  59,  57,  54,  51,
    48,  45,  42,  39,  36,  33,  30,  27,
    24,  21,  18,  15,  12,   9,   6,   3,
     0,  -3,  -6,  -9, -12, -15, -18, -21,
   -24, -27, -30, -33, -36, -39, -42, -45,
   -48, -51, -54, -57, -59, -62, -65, -67,
   -70, -73, -75, -78, -80, -82, -85, -87,
   -89, -91, -94, -96, -98,-100,-102,-103,
  -105,-107,-108,-110,-112,-113,-114,-116,
  -117,-118,-119,-120,-121,-122,-123,-123,
  -124,-125,-125,-126,-126,-126,-126,-126,
  -126,-126,-126,-126,-126,-126,-125,-125,
  -124,-123,-123,-122,-121,-120,-119,-118,
  -117,-116,-114,-113,-112,-110,-108,-107,
  -105,-103,-102,-100, -98, -96, -94, -91,
   -89, -87, -85, -82, -80, -78, -75, -73,
   -70, -67, -65, -62, -59, -57, -54, -51,
   -48, -45, -42, -39, -36, -33, -30, -27,
   -24, -21, -18, -15, -12,  -9,  -6,  -3,

     0,   1,   2,   3,   4,   5,   6,   7,
     8,   9,  10,  11,  12,  13,  14,  15,
    16,  17,  18,  19,  20,  21,  22,  23,
    24,  25,  26,  27,  28,  29,  30,  31,
    32,  33,  34,  35,  36,  37,  38,  39,
    40,  41,  42,  43,  44,  45,  46,  47,
    48,  49,  50,  51,  52,  53,  54,  55,
    56,  57,  58,  59,  60,  61,  62,  63,
    64,  65,  66,  67,  68,  69,  70,  71,
    72,  73,  74,  75,  76,  77,  78,  79,
    80,  81,  82,  83,  84,  85,  86,  87,
    88,  89,  90,  91,  92,  93,  94,  95,
    96,  97,  98,  99, 100, 101, 102, 103,
   104, 105, 106, 107, 108, 109, 110, 111,
   112, 113, 114, 115, 116, 117, 118, 119,
   120, 121, 122, 123, 124, 125, 126, 127,
  -128,-127,-126,-125,-124,-123,-122,-121,
  -120,-119,-118,-117,-116,-115,-114,-113,
  -112,-111,-110,-109,-108,-107,-106,-105,
  -104,-103,-102,-101,-100, -99, -98, -97,
   -96, -95, -94, -93, -92, -91, -90, -89,
   -88, -87, -86, -85, -84, -83, -82, -81,
   -80, -79, -78, -77, -76, -75, -74, -73,
   -72, -71, -70, -69, -68, -67, -66, -65,
   -64, -63, -62, -61, -60, -59, -58, -57,
   -56, -55, -54, -53, -52, -51, -50, -49,
   -48, -47, -46, -45, -44, -43, -42, -41,
   -40, -39, -38, -37, -36, -35, -34, -33,
   -32, -31, -30, -29, -28, -27, -26, -25,
   -24, -23, -22, -21, -20, -19, -18, -17,
   -16, -15, -14, -13, -12, -11, -10,  -9,
    -8,  -7,  -6,  -5,  -4,  -3,  -2,  -1,

  -120, 120,

  -120,-120,-120,-120,-120,-120,-120,-120,
  -120,-120,-120,-120,-120,-120,-120,-120,
  -120,-120,-120,-120,-120,-120,-120,-120,
  -120,-120,-120,-120,-120,-120,-120,-120,
  -120,-120,-120,-120,-120,-120,-120,-120,
  -120,-120,-120,-120,-120,-120,-120,-120,
  -120,-120,-120,-120,-120,-120,-120,-120,
  -120,-120,-120,-120,-120,-120,-120,-120,
  -120,-120,-120,-120,-120,-120,-120,-120,
  -120,-120,-120,-120,-120,-120,-120,-120,
  -120,-120,-120,-120,-120,-120,-120,-120,
  -120,-120,-120,-120,-120,-120,-120,-120,
  -120,-120,-120,-120,-120,-120,-120,-120,
  -120,-120,-120,-120,-120,-120,-120,-120,
  -120,-120,-120,-120,-120,-120,-120,-120,
  -120,-120,-120,-120,-120,-120,-120,-120,
   120, 120, 120, 120, 120, 120, 120, 120,
   120, 120, 120, 120, 120, 120, 120, 120,
   120, 120, 120, 120, 120, 120, 120, 120,
   120, 120, 120, 120, 120, 120, 120, 120,
   120, 120, 120, 120, 120, 120, 120, 120,
   120, 120, 120, 120, 120, 120, 120, 120,
   120, 120, 120, 120, 120, 120, 120, 120,
   120, 120, 120, 120, 120, 120, 120, 120,
   120, 120, 120, 120, 120, 120, 120, 120,
   120, 120, 120, 120, 120, 120, 120, 120,
   120, 120, 120, 120, 120, 120, 120, 120,
   120, 120, 120, 120, 120, 120, 120, 120,
   120, 120, 120, 120, 120, 120, 120, 120,
   120, 120, 120, 120, 120, 120, 120, 120,
   120, 120, 120, 120, 120, 120, 120, 120,
   120, 120, 120, 120, 120, 120, 120, 120,

    41,  35, -66,-124, -31, 108, -42, -82,
    82,-112,  73, -15, -15, -69, -23, -21,
   -77, -90, -37,  60,-121,  12,  62,-103,
    36,  94,  13,  28,   6, -73,  71, -34,
   -77,  18,  77, -56,  67, -69,-117, -90,
    31,   3,  90, 125,   9,  56,  37,  31,
    93, -44, -53,  -4,-106, -11,  69,  59,
    19,  13,-119,  10,  28, -37, -82,  50,
    32,-102,  80, -18,  64, 120,  54,  -3,
    18,  73,  50, -10, -98, 125,  73, -36,
   -83,  79,  20, -14,  68,  64, 102, -48,
   107, -60,  48, -73,  50,  59, -95,  34,
   -10,  34,-111, -99, -31,-117,  31, -38,
   -80, -54,-103,   2, -71, 114, -99,  73,
    44,-128, 126, -59,-103, -43, -23,-128,
   -78, -22, -55, -52,  83, -65, 103, -42,
   -65,  20, -42, 126,  45, -36,-114, 102,
  -125, -17,  87,  73,  97,  -1, 105,-113,
    97, -51, -47,  30, -99,-100,  22, 114,
   114, -26,  29, -16,-124,  79,  74, 119,
     2, -41, -24,  57,  44,  83, -53, -55,
    18,  30,  51, 116, -98,  12, -12, -43,
   -44, -97, -44, -92,  89, 126,  53, -49,
    50,  34, -12, -52, -49, -45,-112,  45,
    72, -45,-113, 117, -26, -39,  29,  42,
   -27, -64,  -9,  43, 120,-127,-121,  68,
    14,  95,  80,   0, -44,  97,-115, -66,
   123,   5,  21,   7,  59,  51,-126,  31,
    24, 112,-110, -38, 100,  84, -50, -79,
  -123,  62, 105,  21,  -8,  70, 106,   4,
  -106, 115,  14, -39,  22,  47, 103, 104,
   -44,  -9,  74,  74, -48,  87, 104, 118,
    -6,  22, -69,  17, -83, -82,  36,-120,
   121,  -2,  82, -37,  37,  67, -27,  60,
   -12,  69, -45, -40,  40, -50,  11, -11,
   -59,  96,  89,  61,-105,  39,-118,  89,
   118,  45, -48, -62, -55, -51, 104, -44,
    73, 106, 121,  37,   8,  97,  64,  20,
   -79,  59, 106, -91,  17,  40, -63,-116,
   -42, -87,  11,-121,-105,-116,  47, -15,
    21,  29,-102,-107, -63,-101, -31, -64,
   126, -23, -88,-102, -89,-122, -62, -75,
    84, -65,-102, -25, -39,  35, -47,  85,
  -112,  56,  40, -47, -39, 108, -95, 102,
    94,  78, -31,  48,-100,  -2, -39, 113,
   -97, -30, -91, -30,  12,-101, -76,  71,
   101,  56,  42,  70,-119, -87,-126, 121,
   122, 118, 120, -62,  99, -79,  38, -33,
   -38,  41, 109,  62,  98, -32,-106,  18,
    52, -65,  57, -90,  63,-119,  94, -15,
   109,  14, -29, 108,  40, -95,  30,  32,
    29, -53, -62,   3,  63,  65,   7,-124,
    15,  20,   5, 101,  27,  40,  97, -55,
   -59, -25,  44,-114,  70,  54,   8, -36,
   -13, -88,-115,  -2, -66, -14, -21, 113,
    -1, -96, -48,  59, 117,   6,-116, 126,
  -121, 120, 115,  77, -48, -66,-126, -66,
   -37, -62,  70,  65,  43,-116,  -6,  48,
   127, 112, -16, -89,  84,-122,  50,-107,
   -86,  91, 104,  19,  11, -26,  -4, -11,
   -54, -66, 125, -97,-119,-118,  65,  27,
    -3, -72,  79, 104, -10, 114, 123,  20,
  -103, -51, -45,  13, -16,  68,  58, -76,
   -90, 102,  83,  51,  11, -53, -95,  16
};

static const uint16  spk_freq_table[12] = {
	36484, 34436, 32503, 30679, 28957, 27332,
	25798, 24350, 22983, 21693, 20476, 19326
};

static const uint16 pcjr_freq_table[12] = {
	65472, 61760, 58304, 55040, 52032, 49024,
	46272, 43648, 41216, 38912, 36736, 34624
};


Player_V2Base::Player_V2Base(ScummEngine *scumm, Audio::Mixer *mixer, bool pcjr)
	: _vm(scumm),
	_mixer(mixer),
	_pcjr(pcjr),
	_sampleRate(_mixer->getOutputRate()) {

	_isV3Game = (scumm->_game.version >= 3);

	_header_len = (scumm->_game.features & GF_OLD_BUNDLE) ? 4 : 6;

	// Initialize sound queue
	_current_nr = _next_nr = 0;
	_current_data = _next_data = 0;

	// Initialize channel code
	for (int i = 0; i < 4; ++i)
		clear_channel(i);

	_next_tick = 0;
	_tick_len = (_sampleRate << FIXP_SHIFT) / FREQ_HZ;

	// Initialize V3 music timer
	_music_timer_ctr = _music_timer = 0;
	_ticks_per_music_timer = 65535;

	if (_pcjr) {
		_freqs_table = pcjr_freq_table;
	} else {
		_freqs_table = spk_freq_table;
	}
}

Player_V2Base::~Player_V2Base() {
}

void Player_V2Base::chainSound(int nr, byte *data) {
	int offset = _header_len + (_pcjr ? 10 : 2);

	_current_nr = nr;
	_current_data = data;

	for (int i = 0; i < 4; i++) {
		clear_channel(i);

		_channels[i].d.music_script_nr = nr;
		if (data) {
			_channels[i].d.next_cmd = READ_LE_UINT16(data + offset + 2 * i);
			if (_channels[i].d.next_cmd) {
				_channels[i].d.time_left = 1;
			}
		}
	}
	_music_timer = 0;
}

void Player_V2Base::chainNextSound() {
	if (_next_nr) {
		chainSound(_next_nr, _next_data);
		_next_nr = 0;
		_next_data = 0;
	}
}

// TODO: Merge stopAllSounds, stopSound() and startSound(), using some overriding
// perhaps? Player_V2CMS's implementations start like that of Player_V2
// but then add some MIDI related stuff.

void Player_V2Base::clear_channel(int i) {
	ChannelInfo *channel = &_channels[i];
	memset(channel, 0, sizeof(ChannelInfo));
}

int Player_V2Base::getMusicTimer() {
	if (_isV3Game)
		return _music_timer;
	else
		return _channels[0].d.music_timer;
}

void Player_V2Base::execute_cmd(ChannelInfo *channel) {
	uint16 value;
	int16 offset;
	uint8 *script_ptr;
	ChannelInfo * current_channel;
	ChannelInfo * dest_channel;

	current_channel = channel;

	if (channel->d.next_cmd == 0)
		goto check_stopped;
	script_ptr = &_current_data[channel->d.next_cmd];

	for (;;) {
		uint8 opcode = *script_ptr++;
		if (opcode >= 0xf8) {
			switch (opcode) {
			case 0xf8: // set hull curve
				debug(7, "channels[%d]: hull curve %2d",
				(uint)(channel - _channels), *script_ptr);
				channel->d.hull_curve = hull_offsets[*script_ptr / 2];
				script_ptr++;
				break;

			case 0xf9: // set freqmod curve
				debug(7, "channels[%d]: freqmod curve %2d",
				(uint)(channel - _channels), *script_ptr);
				channel->d.freqmod_table = freqmod_offsets[*script_ptr / 4];
				channel->d.freqmod_modulo = freqmod_lengths[*script_ptr / 4];
				script_ptr++;
				break;

			case 0xfd: // clear other channel
				value = READ_LE_UINT16 (script_ptr) / sizeof (ChannelInfo);
				debug(7, "clear channel %d", value);
				script_ptr += 2;
				// In Indy3, when traveling to Venice a command is
				// issued to clear channel 4. So we introduce a 4th
				// channel, which is never used.  All OOB accesses are
				// mapped to this channel.
				//
				// The original game had room for 8 channels, but only
				// channels 0-3 are read, changes to other channels
				// had no effect.
				if (value >= ARRAYSIZE (_channels))
					value = 4;
				channel = &_channels[value];
				// fall through

			case 0xfa: // clear current channel
				if (opcode == 0xfa)
					debug(7, "clear channel");
				channel->d.next_cmd   = 0;
				channel->d.base_freq  = 0;
				channel->d.freq_delta = 0;
				channel->d.freq = 0;
				channel->d.volume = 0;
				channel->d.volume_delta = 0;
				channel->d.inter_note_pause = 0;
				channel->d.transpose = 0;
				channel->d.hull_curve = 0;
				channel->d.hull_offset = 0;
				channel->d.hull_counter = 0;
				channel->d.freqmod_table = 0;
				channel->d.freqmod_offset = 0;
				channel->d.freqmod_incr = 0;
				channel->d.freqmod_multiplier = 0;
				channel->d.freqmod_modulo = 0;
				break;

			case 0xfb: // ret from subroutine
				debug(7, "ret from sub");
				script_ptr = _retaddr;
				break;

			case 0xfc: // call subroutine
				offset = READ_LE_UINT16 (script_ptr);
				debug(7, "subroutine %d", offset);
				script_ptr += 2;
				_retaddr = script_ptr;
				script_ptr = _current_data + offset;
				break;

			case 0xfe: // loop music
				opcode = *script_ptr++;
				offset = READ_LE_UINT16 (script_ptr);
				script_ptr += 2;
				debug(7, "loop if %d to %d", opcode, offset);
				if (!channel->array[opcode / 2] || --channel->array[opcode/2])
					script_ptr += offset;
				break;

			case 0xff: // set parameter
				opcode = *script_ptr++;
				value = READ_LE_UINT16 (script_ptr);
				channel->array[opcode / 2] = value;
				debug(7, "channels[%d]: set param %2d = %5d",
						(uint)(channel - _channels), opcode, value);
				script_ptr += 2;
				if (opcode == 14) {
				    /* tempo var */
				    _ticks_per_music_timer = 125;
				}
				if (opcode == 0)
					goto end;
				break;
			}
		} else { // opcode < 0xf8
			for (;;) {
				int16 note, octave;
				int is_last_note;
				dest_channel = &_channels[(opcode >> 5) & 3];

				if (!(opcode & 0x80)) {

					int tempo = channel->d.tempo;
					if (!tempo)
						tempo = 1;
					channel->d.time_left = tempo * note_lengths[opcode & 0x1f];

					note = *script_ptr++;
					is_last_note = note & 0x80;
					note &= 0x7f;
					if (note == 0x7f) {
						debug(8, "channels[%d]: pause %d",
							  (uint)(channel - _channels), channel->d.time_left);
						goto end;
					}
				} else {

					channel->d.time_left = ((opcode & 7) << 8) | *script_ptr++;

					if ((opcode & 0x10)) {
						debug(8, "channels[%d]: pause %d",
							  (uint)(channel - _channels), channel->d.time_left);
						goto end;
					}

					is_last_note = 0;
					note = (*script_ptr++) & 0x7f;
				}

				debug(8, "channels[%d]: @%04x note: %3d+%d len: %2d hull: %d mod: %d/%d/%d %s",
						(uint)(dest_channel - channel), script_ptr ? (uint)(script_ptr - _current_data - 2) : 0,
						note, (signed short) dest_channel->d.transpose, channel->d.time_left,
						dest_channel->d.hull_curve, dest_channel->d.freqmod_table,
						dest_channel->d.freqmod_incr,dest_channel->d.freqmod_multiplier,
						is_last_note ? "last":"");

				uint16 myfreq;
				dest_channel->d.time_left = channel->d.time_left;
				dest_channel->d.note_length =
					channel->d.time_left - dest_channel->d.inter_note_pause;
				note += dest_channel->d.transpose;
				while (note < 0)
					note += 12;
				octave = note / 12;
				note = note % 12;
				dest_channel->d.hull_offset = 0;
				dest_channel->d.hull_counter = 1;
				if (_pcjr && dest_channel == &_channels[3]) {
					dest_channel->d.hull_curve = 196 + note * 12;
					myfreq = 384 - 64 * octave;
				} else {
					myfreq = _freqs_table[note] >> octave;
				}
				dest_channel->d.freq = dest_channel->d.base_freq = myfreq;
				if (is_last_note)
					goto end;
				opcode = *script_ptr++;
			}
		}
	}

end:
	channel = current_channel;
	if (channel->d.time_left) {
		channel->d.next_cmd = script_ptr - _current_data;
		return;
	}

	channel->d.next_cmd = 0;

check_stopped:
	int i;
	for (i = 0; i < 4; i++) {
		if (_channels[i].d.time_left)
			return;
	}

	_current_nr = 0;
	_current_data = 0;
	chainNextSound();
}

void Player_V2Base::next_freqs(ChannelInfo *channel) {
	channel->d.volume    += channel->d.volume_delta;
	channel->d.base_freq += channel->d.freq_delta;

	channel->d.freqmod_offset += channel->d.freqmod_incr;
	if (channel->d.freqmod_offset > channel->d.freqmod_modulo)
		channel->d.freqmod_offset -= channel->d.freqmod_modulo;

	channel->d.freq =
		(int) (freqmod_table[channel->d.freqmod_table + (channel->d.freqmod_offset >> 4)])
		* (int) channel->d.freqmod_multiplier / 256
		+ channel->d.base_freq;

	debug(9, "Freq: %d/%d, %d/%d/%d*%d %d",
			channel->d.base_freq, (int16)channel->d.freq_delta,
			channel->d.freqmod_table, channel->d.freqmod_offset,
			channel->d.freqmod_incr, channel->d.freqmod_multiplier,
			channel->d.freq);

	if (channel->d.note_length && !--channel->d.note_length) {
		channel->d.hull_offset  = 16;
		channel->d.hull_counter = 1;
	}

	if (!--channel->d.time_left) {
		execute_cmd(channel);
	}

	if (channel->d.hull_counter && !--channel->d.hull_counter) {
		for (;;) {
			const int16 *hull_ptr = hulls
			+ channel->d.hull_curve + channel->d.hull_offset / 2;
			if (hull_ptr[1] == -1) {
				channel->d.volume = hull_ptr[0];
				if (hull_ptr[0] == 0)
				channel->d.volume_delta = 0;
				channel->d.hull_offset += 4;
			} else {
				channel->d.volume_delta = hull_ptr[0];
				channel->d.hull_counter = hull_ptr[1];
				channel->d.hull_offset += 4;
				break;
			}
		}
	}
}

void Player_V2Base::nextTick() {
	for (int i = 0; i < 4; i++) {
		if (!_channels[i].d.time_left)
			continue;
		next_freqs(&_channels[i]);
	}
	if (_music_timer_ctr++ >= _ticks_per_music_timer) {
		_music_timer_ctr = 0;
		_music_timer++;
	}
}


} // End of namespace Scumm
