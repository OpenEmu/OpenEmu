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

#ifndef __INTERPRETER_H__
#define __INTERPRETER_H__

#include <stdint.h>
#include <istream>
#include <ostream>
#include "cpu.hpp"

#define ARM(name) \
	inline void a##name ()
#define NIARM(name) \
	void a##name ()
#define THUMB(name) \
	inline void t##name ()
#define NITHUMB(name) \
	void t##name ()

namespace AMeteor
{
	class Interpreter : public Cpu
	{
		public :
			Interpreter();

			void Reset ()
			{
				m_interrupt = m_interrupt_ = false;
				m_run = false;
				Cpu::Reset();
			}
			void SoftReset ()
			{
				m_interrupt_ = m_interrupt = false;
				Cpu::SoftReset();
			}

			void SendInterrupt (uint16_t interrupt);
			void CheckInterrupt ();

			void Run (unsigned int cycles);
			void Stop ()
			{
				m_run = false;
			}
			bool IsRunning ()
			{
				return m_run;
			}

			bool SaveState (std::ostream& stream);
			bool LoadState (std::istream& stream);

		protected :
			void SetInterrupt (bool interrupt)
			{
				m_interrupt = interrupt;
			}

		private :
			bool m_run;

			bool m_interrupt;
			bool m_interrupt_;
			uint32_t code;
			uint8_t& m_haltcnt;
			// only for use in halt mode, in normal mode we use m_interrupt
			uint16_t& m_if;
			uint16_t& m_ie;

			NIARM(_Code);
			inline bool a_CheckCondition (uint8_t cond);
			inline void a_DataProcCore(uint8_t rd, uint32_t op1, uint32_t op2,
					bool shiftcarry);

			ARM(BXBLX);
			ARM(BBL);
			NIARM(_DataProcShiftImm);
			NIARM(_DataProcShiftReg);
			NIARM(_DataProcImm);
			ARM(PSR);
			ARM(_Multiply);
			ARM(LDRSTR);
			ARM(STRLDR_HD);
			ARM(LDMSTM);
			ARM(SWP);
			ARM(SWI);

			NITHUMB(_Code);

			THUMB(_Shift);
			THUMB(ADDSUB);
			THUMB(_Imm);
			THUMB(_ALU);
			THUMB(_HiRegOp);
			THUMB(LDRimm);
			THUMB(STRLDRreg);
			THUMB(STRLDRoff);
			THUMB(LDRHSTRHoff);
			THUMB(STRLDRsp);
			THUMB(ADDpcsp);
			THUMB(ADDsp);
			THUMB(PUSHPOP);
			THUMB(STMLDM);
			THUMB(_CondBranch);
			THUMB(SWI);
			THUMB(B);
			THUMB(_BL1);
			THUMB(_BL2);
	};
}

#undef ARM
#undef NIARM
#undef THUMB
#undef NITHUMB

#endif
