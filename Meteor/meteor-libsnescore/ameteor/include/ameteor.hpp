// Meteor - A Nintendo Gameboy Advance emulator
// Copyright (C) 2009-2011 Philippe Daouadi
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program.  If not, see <http://www.gnu.org/licenses/>.

#ifndef __AMETEOR_H__
#define __AMETEOR_H__

#include "memory.hpp"
#include "io.hpp"
#include "dma.hpp"
#include "interpreter.hpp"
#include "lcd.hpp"
#include "clock.hpp"
#include "timer.hpp"
#include "sound.hpp"
#include "keypad.hpp"

namespace AMeteor
{
	extern Clock _clock;
	extern Io _io;
	extern Interpreter _cpu;
	extern Memory _memory;
	extern Dma _dma;
	extern Lcd _lcd;
	extern Sound _sound;
	extern Keypad _keypad;
	extern Timer _timer0;
	extern Timer _timer1;
	extern Timer _timer2;
	extern Timer _timer3;

	const uint32_t UNIT_CLOCK  = 0x0001;
	const uint32_t UNIT_IO     = 0x0002;
	const uint32_t UNIT_CPU    = 0x0004;
	const uint32_t UNIT_MEMORY = 0x0008;
	const uint32_t UNIT_DMA    = 0x0010;
	const uint32_t UNIT_LCD    = 0x0020;
	const uint32_t UNIT_SOUND  = 0x0040;
	const uint32_t UNIT_KEYPAD = 0x0080;
	const uint32_t UNIT_TIMER0 = 0x0100;
	const uint32_t UNIT_TIMER1 = 0x0200;
	const uint32_t UNIT_TIMER2 = 0x0400;
	const uint32_t UNIT_TIMER3 = 0x0800;
	const uint32_t UNIT_MEMORY_ROM = 0x1000;
	const uint32_t UNIT_MEMORY_BIOS = 0x2000;
	const uint32_t UNIT_ALL = 0x3FFF;

	void Reset (uint32_t units);

	bool SaveState (const char* filename);
	bool LoadState (const char* filename);

	bool SaveState (std::ostream& stream);
	bool LoadState (std::istream& stream);

	inline void Run (unsigned int cycles)
	{
		_cpu.Run(cycles);
	}

	inline void Stop ()
	{
		_cpu.Stop();
	}
}

#endif
