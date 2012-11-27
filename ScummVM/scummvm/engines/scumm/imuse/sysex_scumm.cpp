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


#include "common/endian.h"
#include "common/textconsole.h"
#include "common/util.h"

/*
 * SysEx command handlers must have full access to the
 * internal IMuse implementation classes. Before including
 * the relevant header file, two things must happen:
 *   1. A function declaration must be made.
 *   2. The following #define must be established:
 *      #define SYSEX_CALLBACK_FUNCTION functionName
 */
#define SYSEX_CALLBACK_FUNCTION sysexHandler_Scumm
#include "scumm/imuse/imuse_internal.h"

namespace Scumm {

void sysexHandler_Scumm(Player *player, const byte *msg, uint16 len) {
	Part *part;
	byte a;
	byte buf[128];

	IMuseInternal *se = player->_se;
	const byte *p = msg;

	byte code = 0;
	switch (code = *p++) {
	case 0:
		// Allocate new part.
		// There are 8 bytes (after decoding!) of useful information here.
		// Here is what we know about them so far:
		//   BYTE 0: Channel #
		//   BYTE 1: BIT 01(0x01): Part on?(1 = yes)
		//            BIT 02(0x02): Reverb? (1 = yes) [bug #1088045]
		//   BYTE 2: Priority adjustment
		//   BYTE 3: Volume [guessing]
		//   BYTE 4: Pan [bug #1088045]
		//   BYTE 5: BIT 8(0x80): Percussion?(1 = yes) [guessed?]
		//   BYTE 5: Transpose, if set to 0x80(=-1) it means no transpose
		//   BYTE 6: Detune
		//   BYTE 7: Pitchbend factor [bug #1088045]
		//   BYTE 8: Program

		part = player->getPart(p[0] & 0x0F);
		player->decode_sysex_bytes(p + 1, buf + 1, len - 1);
		if (part) {
			part->set_onoff(buf[1] & 0x01);
			part->effectLevel((buf[1] & 0x02) ? 127 : 0);
			part->set_pri(buf[2]);
			part->volume(buf[3]);
			part->set_pan(buf[4]);
			part->_percussion = player->_supportsPercussion ? ((buf[5] & 0x80) > 0) : false;
			part->set_transpose(buf[5]);
			part->set_detune(buf[6]);
			part->pitchBendFactor(buf[7]);
			if (part->_percussion) {
				if (part->_mc) {
					part->off();
					se->reallocateMidiChannels(player->_midi);
				}
			} else {
				if (player->_isMIDI) {
					// Even in cases where a program does not seem to be specified,
					// i.e. bytes 15 and 16 are 0, we send a program change because
					// 0 is a valid program number. MI2 tests show that in such
					// cases, a regular program change message always seems to follow
					// anyway.
					part->_instrument.program(buf[8], player->_isMT32);
				} else {
					// Like the original we set up the instrument data of the
					// specified program here too. In case the global
					// instrument data is not loaded already, this will take
					// care of setting a default instrument too.
					se->copyGlobalInstrument(buf[8], &part->_instrument);
				}
				part->sendAll();
			}
		}
		break;

	case 1:
		// Shut down a part. [Bug 1088045, comments]
		part = player->getPart(p[0]);
		if (part != NULL)
			part->uninit();
		break;

	case 2: // Start of song. Ignore for now.
		break;

	case 16: // AdLib instrument definition(Part)
		a = *p++ & 0x0F;
		++p; // Skip hardware type
		part = player->getPart(a);
		if (part) {
			if (len == 62 || len == 48) {
				player->decode_sysex_bytes(p, buf, len - 2);
				part->set_instrument((byte *)buf);
			} else {
				part->programChange(254); // Must be invalid, but not 255 (which is reserved)
			}
		}
		break;

	case 17: // AdLib instrument definition(Global)
		p += 2; // Skip hardware type and... whatever came right before it
		a = *p++;
		player->decode_sysex_bytes(p, buf, len - 3);
		if (len == 63 || len == 49)
			se->setGlobalInstrument(a, buf);
		break;

	case 33: // Parameter adjust
		a = *p++ & 0x0F;
		++p; // Skip hardware type
		player->decode_sysex_bytes(p, buf, len - 2);
		part = player->getPart(a);
		if (part)
			part->set_param(READ_BE_UINT16(buf), READ_BE_UINT16(buf + 2));
		break;

	case 48: // Hook - jump
		if (player->_scanning)
			break;
		player->decode_sysex_bytes(p + 1, buf, len - 1);
		player->maybe_jump(buf[0], READ_BE_UINT16(buf + 1), READ_BE_UINT16(buf + 3), READ_BE_UINT16(buf + 5));
		break;

	case 49: // Hook - global transpose
		player->decode_sysex_bytes(p + 1, buf, len - 1);
		player->maybe_set_transpose(buf);
		break;

	case 50: // Hook - part on/off
		buf[0] = *p++ & 0x0F;
		player->decode_sysex_bytes(p, buf + 1, len - 1);
		player->maybe_part_onoff(buf);
		break;

	case 51: // Hook - set volume
		buf[0] = *p++ & 0x0F;
		player->decode_sysex_bytes(p, buf + 1, len - 1);
		player->maybe_set_volume(buf);
		break;

	case 52: // Hook - set program
		buf[0] = *p++ & 0x0F;
		player->decode_sysex_bytes(p, buf + 1, len - 1);
		player->maybe_set_program(buf);
		break;

	case 53: // Hook - set transpose
		buf[0] = *p++ & 0x0F;
		player->decode_sysex_bytes(p, buf + 1, len - 1);
		player->maybe_set_transpose_part(buf);
		break;

	case 64: // Marker
		p++;
		len--;
		while (len--) {
			se->handle_marker(player->_id, *p++);
		}
		break;

	case 80: // Loop
		player->decode_sysex_bytes(p + 1, buf, len - 1);
		player->setLoop(READ_BE_UINT16(buf), READ_BE_UINT16(buf + 2),
		                READ_BE_UINT16(buf + 4), READ_BE_UINT16(buf + 6),
		                READ_BE_UINT16(buf + 8));
		break;

	case 81: // End loop
		player->clearLoop();
		break;

	case 96: // Set instrument
		part = player->getPart(p[0] & 0x0F);
		a = (p[1] & 0x0F) << 12 | (p[2] & 0x0F) << 8 | (p[3] & 0x0F) << 4 | (p[4] & 0x0F);
		if (part)
			part->set_instrument(a);
		break;

	default:
		error("Unknown SysEx command %d", (int)code);
	}
}

} // End of namespace Scumm
