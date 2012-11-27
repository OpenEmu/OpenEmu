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

#include "agi/agi.h"
#include "agi/opcodes.h"

namespace Agi {

#define ip	(_game.logics[lognum].cIP)
#define code	(_game.logics[lognum].data)

AgiInstruction logicNamesIf[] = {
    { "OR", "", NULL },
    { "NOT", "", NULL },
    { "ELSE", "", NULL },
    { "IF", "", NULL }
};

void AgiEngine::debugConsole(int lognum, int mode, const char *str) {
	AgiInstruction *x;
	uint8 a, z;
	const char *c;

	if (str) {
		debug(0, "         %s", str);
		return;
	}

	debugN(0, "%03d:%04x ", lognum, ip);

	switch (*(code + ip)) {
	case 0xFC:
	case 0xFD:
	case 0xFE:
	case 0xFF:
		x = logicNamesIf;

		if (_debug.opcodes) {
			debugN(0, "%02X %02X %02X %02X %02X %02X %02X %02X %02X\n"
			    "         ",
			    (uint8)*(code + (0 + ip)) & 0xFF,
			    (uint8)*(code + (1 + ip)) & 0xFF,
			    (uint8)*(code + (2 + ip)) & 0xFF,
			    (uint8)*(code + (3 + ip)) & 0xFF,
			    (uint8)*(code + (4 + ip)) & 0xFF,
			    (uint8)*(code + (5 + ip)) & 0xFF,
			    (uint8)*(code + (6 + ip)) & 0xFF,
			    (uint8)*(code + (7 + ip)) & 0xFF,
			    (uint8)*(code + (8 + ip)) & 0xFF);
		}
		debugN(0, "%s ", (x + *(code + ip) - 0xFC)->name);
		break;
	default:
		x = mode == lCOMMAND_MODE ? logicNamesCmd : logicNamesTest;
		a = x[*(code + ip)].argumentsLength();
		c = x[*(code + ip)].args;

		if (_debug.opcodes) {
			debugN(0, "%02X %02X %02X %02X %02X %02X %02X %02X %02X\n"
			    "         ",
			    (uint8)*(code + (0 + ip)) & 0xFF,
			    (uint8)*(code + (1 + ip)) & 0xFF,
			    (uint8)*(code + (2 + ip)) & 0xFF,
			    (uint8)*(code + (3 + ip)) & 0xFF,
			    (uint8)*(code + (4 + ip)) & 0xFF,
			    (uint8)*(code + (5 + ip)) & 0xFF,
			    (uint8)*(code + (6 + ip)) & 0xFF,
			    (uint8)*(code + (7 + ip)) & 0xFF,
			    (uint8)*(code + (8 + ip)) & 0xFF);
		}
		debugN(0, "%s ", (x + *(code + ip))->name);

		for (z = 1; a > 0;) {
			if (*c == 'n') {
				debugN(0, "%d", *(code + (ip + z)));
			} else {
				debugN(0, "v%d[%d]", *(code + (ip + z)), getvar(*(code + (ip + z))));
			}
			c++;
			z++;
			if (--a > 0)
				debugN(0, ",");
		}
		break;
	}

	debugN(0, "\n");
}

} // End of namespace Agi
