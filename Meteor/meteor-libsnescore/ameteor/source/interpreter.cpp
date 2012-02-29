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

#include "ameteor/interpreter.hpp"
#include "ameteor/cpu.hpp"
#include "ameteor/memory.hpp"
#include "ameteor/bios.hpp"
#include "ameteor/disassembler/instruction.hpp"
#include "globals.hpp"
#include "cpu_globals.hpp"
#include "ameteor.hpp"

#include "debug.hpp"

namespace AMeteor
{
	Interpreter::Interpreter() :
		m_run(false),
		m_interrupt_(false),
		m_haltcnt(IO.GetRef8(Io::HALTCNT)),
		m_if(IO.GetRef16(Io::IF)),
		m_ie(IO.GetRef16(Io::IE))
	{
	}

	void Interpreter::SendInterrupt (uint16_t interrupt)
	{
		IO.GetRef16(Io::IF) |= interrupt;
		if ((interrupt & IO.DRead16(Io::IE)) &&
				(IO.DRead16(Io::IME) & 0x1) &&
				!m_st.icpsr.irq_d)
			// irq are enabled and theses irq are enabled...
			m_interrupt = true;
	}

	void Interpreter::CheckInterrupt ()
	{
		m_interrupt =
			(IO.DRead16(Io::IF) & IO.DRead16(Io::IE)) &&
			(IO.DRead16(Io::IME) & 0x1) &&
			!m_st.icpsr.irq_d;
	}

	void Interpreter::Run (unsigned int cycles)
	{
		m_run = true;
		CLOCK.ResetCounter();
		while(m_run && CLOCK.GetCounter() < cycles)
		{
			switch (m_haltcnt)
			{
				case 255: // normal mode
					PrintRegs();
					if (FLAG_T)
					{
						if (R(15) & 0x1)
							met_abort("PC not 16 bit aligned : " << IOS_ADD << R(15));

						code = MEM.Read16(R(15)-2);
						//std::cerr << IOS_ADD << R(15) << ' ' << Disassembler::Instruction(R(15), (uint16_t)code).ToString() << std::endl;
						R(15) += 2;
						t_Code();
					}
					else
					{
						if (R(15) & 0x3)
							met_abort("PC not 32 bit aligned : " << IOS_ADD << R(15));

						if (R(15) < 0x01000000 && !MEM.HasBios())
						{
							switch (R(15))
							{
								case 0x004:
									Bios::Bios000h();
									break;
								case 0x00C:
									Bios::Bios008h();
									break;
								case 0x01C:
									Bios::Bios018h();
									break;
								case 0x134:
									Bios::Bios130h();
									break;
								case 0x33C:
									Bios::Bios338h();
									break;
								case 0x16C:
									Bios::Bios168h();
									break;
								default:
									met_abort("Jump to " << IOS_ADD << R(15));
							}
						}
						else
						{
							code = MEM.Read32(R(15)-4);
							//std::cerr << IOS_ADD << R(15) << ' ' << Disassembler::Instruction(R(15), (uint32_t)code).ToString() << std::endl;
							R(15) += 4;
							a_Code();
						}
					}
					if (R(15) < 0x01000000 && FLAG_T && !MEM.HasBios())
						met_abort("Jump to " << IOS_ADD << R(15));

					CLOCK.Commit();

					if (m_interrupt)
						// irq are enabled and there are irq waiting...
					{
						// FIXME : do we really need this ??
						// if not, we can get rid of save and load state and reset
						if (m_interrupt_)
						{
							m_interrupt_ = false;
							// XXX we must be sure the cpu isn't halted when an interrupt
							// occurs
							// should be removed after since it takes no time to make a new
							// iteration of the loop
							m_haltcnt = 255;
							CPU.Interrupt();
						}
						else
						{
							// XXX
							if (m_haltcnt != 255)
							{
								m_haltcnt = 255;
								CPU.Interrupt();
							}
							else
								m_interrupt_ = true;
						}
					}

					break;
				case 0: // halt mode
					if (m_if & m_ie) // interrupt occured
					{
						m_haltcnt = 255; // return to normal mode
						CPU.Interrupt();
						// XXX use an else
						break;
					}

					CLOCK.WaitForNext();

					// XXX remove this block
					if (m_if & m_ie) // interrupt occured
					{
						m_haltcnt = 255; // return to normal mode
						CPU.Interrupt();
					}

					break;
				case 1: // stop mode
					met_abort("Stop mode not implemented");
					break;
				default:
					met_abort("Unknown HALTCNT value : " << (int)m_haltcnt);
					break;
			}
		}
		m_run = false;
	}

	bool Interpreter::SaveState (std::ostream& stream)
	{
		SS_WRITE_VAR(m_interrupt_);

		return Cpu::SaveState(stream);
	}

	bool Interpreter::LoadState (std::istream& stream)
	{
		SS_READ_VAR(m_interrupt_);

		return Cpu::LoadState(stream);
	}
}
