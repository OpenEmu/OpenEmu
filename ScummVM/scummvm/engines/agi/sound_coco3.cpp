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

#include "agi/sound_coco3.h"

namespace Agi {

static int cocoFrequencies[] = {
	 130,  138,  146,  155,  164,  174,  184,  195,  207,  220,  233,  246,
	 261,  277,  293,  311,  329,  349,  369,  391,  415,  440,  466,  493,
	 523,  554,  587,  622,  659,  698,  739,  783,  830,  880,  932,  987,
	1046, 1108, 1174, 1244, 1318, 1396, 1479, 1567, 1661, 1760, 1864, 1975,
	2093, 2217, 2349, 2489, 2637, 2793, 2959, 3135, 3322, 3520, 3729, 3951
};

SoundGenCoCo3::SoundGenCoCo3(AgiBase *vm, Audio::Mixer *pMixer) : SoundGen(vm, pMixer) {
}

SoundGenCoCo3::~SoundGenCoCo3() {
}

void SoundGenCoCo3::play(int resnum) {
	int i = cocoFrequencies[0]; // Silence warning

	i = i + 1;

#if 0
	int i = 0;
	CoCoNote note;

	do {
		note.read(_chn[i].ptr);

		if (note.freq != 0xff) {
			playNote(0, cocoFrequencies[note.freq], note.volume);

			uint32 start_time = _vm->_system->getMillis();

			while (_vm->_system->getMillis() < start_time + note.duration) {
				_vm->_system->updateScreen();

				_vm->_system->delayMillis(10);
			}
		}
	} while (note.freq != 0xff);
#endif
}

void SoundGenCoCo3::stop() {
}

int SoundGenCoCo3::readBuffer(int16 *buffer, const int numSamples) {
	return numSamples;
}

} // End of namespace Agi
