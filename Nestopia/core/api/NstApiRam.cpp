////////////////////////////////////////////////////////////////////////////////////////
// Josh Weinberg
////////////////////////////////////////////////////////////////////////////////////////

#include "../NstMachine.hpp"
#include "../NstPpu.hpp"
#include "../NstCpu.hpp"
#include "NstApiRam.h"

namespace Nes
{
	namespace Api
	{
#ifdef NST_MSVC_OPTIMIZE
#pragma optimize("s", on)
#endif
		
		void Ram::SetRAM(int offset, int value) throw()
		{
			if(offset >= Nes::Core::Cpu::RAM_SIZE)
				offset = Nes::Core::Cpu::RAM_SIZE-1;
			emulator.cpu.GetRam()[offset] = value;
		}
		
		void Ram::SetCHR(int page, int offset, int value) throw()
		{
			emulator.ppu.GetChrMem()[page][offset] = value;
		}
		
		void Ram::SetNmtRam(int page, int offset, int value) throw()
		{
			if(page > 4)
				page = 4;
			if(offset >= Nes::Core::SIZE_4K)
				offset = Nes::Core::SIZE_4K-1;
			emulator.ppu.GetNmtMem()[page][offset] = value;
		}

#ifdef NST_MSVC_OPTIMIZE
#pragma optimize("", on)
#endif
	}
}
