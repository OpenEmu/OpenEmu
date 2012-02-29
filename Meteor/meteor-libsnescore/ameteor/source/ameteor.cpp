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

#include "ameteor.hpp"
#include "debug.hpp"
#include "globals.hpp"
#include <cstring>
#include <sstream>

// TODO add version
#define SS_MAGIC_STRING ("AMeteor SaveState")
#define SS_MS_SIZE (sizeof(SS_MAGIC_STRING)-1)

namespace AMeteor
{
	namespace
	{
		class AMeteor
		{
			public :
				AMeteor ()
				{
					Audio::InitNoise();
				}
		} __ameteor;
	}

	// the clock must be initialized first since there are devices like
	// lcd which needs to set the timer
	Clock _clock;
	Io _io;
	// the interpreter (which is in the cpu) takes io addresses, thus the
	// cpu must be initialized after io
	Interpreter _cpu;
	Memory _memory;
	Dma _dma;
	// the lcd must be initialized after the memory since it takes
	// pointers from it
	Lcd _lcd;
	// the sound must be initialized after the io since it takes references
	// from it
	Sound _sound;
	// the keypad needs to take the vblank event from lcd, so it must be
	// initialized after lcd
	// it must also be initialized after io since it takes the keyinput
	// reference
	Keypad _keypad;
	Timer _timer3(3, NULL);
	Timer _timer2(2, &_timer3);
	Timer _timer1(1, &_timer2);
	Timer _timer0(0, &_timer1);

	void Reset (uint32_t units)
	{
#define RESET(u, e) \
	if (units & UNIT_##e) \
		_##u.Reset();
		RESET(clock, CLOCK);
		RESET(io, IO);
		RESET(cpu, CPU);
		RESET(dma, DMA);
		RESET(lcd, LCD);
		RESET(sound, SOUND);
		RESET(keypad, KEYPAD);
		RESET(timer0, TIMER0);
		RESET(timer1, TIMER1);
		RESET(timer2, TIMER2);
		RESET(timer3, TIMER3);
#undef RESET
		if (units & UNIT_MEMORY)
			_memory.Reset(units);
	}

	bool SaveState (const char* filename)
	{
		if (_cpu.IsRunning())
			return false;

		std::ostringstream ss;

		if (!SaveState(ss))
			return false;

		std::ofstream file(filename);

		if (!file)
			return false;

		std::string buf = ss.str();
		if (!file.write(buf.c_str(), buf.length()))
			return false;

		file.close();
		if (file.bad())
			return false;

		return true;
	}

	bool LoadState (const char* filename)
	{
		if (_cpu.IsRunning())
			return false;

		std::istringstream ss;
		{
			std::ifstream file(filename);
			if (!file)
				return false;

			// 1Mo
			std::vector<uint8_t> buf(0x100000);
			if (file.read((char*)&buf[0], 0x100000).bad())
				return false;
			int nread = file.gcount();

			file.close();
			if (file.bad())
				return false;

			ss.str(std::string((char*)&buf[0], nread));
		}

		return LoadState(ss);
	}

	bool SaveState (std::ostream& stream)
	{
		if (_cpu.IsRunning())
			return false;

		SS_WRITE_DATA(SS_MAGIC_STRING, SS_MS_SIZE);

#define SAVE(dev) \
	if (!dev.SaveState(stream)) \
		return false
		SAVE(_clock);
		SAVE(_io);
		SAVE(_cpu);
		SAVE(_memory);
		SAVE(_dma);
		SAVE(_lcd);
		SAVE(_sound);
		//SAVE(_keypad);
		SAVE(_timer0);
		SAVE(_timer1);
		SAVE(_timer2);
		SAVE(_timer3);
#undef SAVE

		return true;
	}

	bool LoadState (std::istream& stream)
	{
		if (_cpu.IsRunning())
			return false;

		{
			char buf[SS_MS_SIZE];
			SS_READ_DATA(buf, SS_MS_SIZE);
			if (std::memcmp(buf, SS_MAGIC_STRING, SS_MS_SIZE))
				return false;
		}


#define LOAD(dev) \
	if (!dev.LoadState(stream)) \
		return false
		LOAD(_clock);
		LOAD(_io);
		LOAD(_cpu);
		LOAD(_memory);
		LOAD(_dma);
		LOAD(_lcd);
		LOAD(_sound);
		//LOAD(_keypad);
		LOAD(_timer0);
		LOAD(_timer1);
		LOAD(_timer2);
		LOAD(_timer3);
#undef LOAD

		uint8_t xxx;
		// if there is garbage at end of file
		if (stream.read((char*)&xxx, 1))
			return false;

		return true;
	}
}
