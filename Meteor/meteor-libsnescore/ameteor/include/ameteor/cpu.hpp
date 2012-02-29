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

#ifndef __CPU_H__
#define __CPU_H__

#include <stdint.h>
#include <istream>
#include <ostream>

namespace AMeteor
{
	class Cpu
	{
		public :
			union Psr
			{
				uint32_t dw;
				struct
				{
					unsigned int mode     :  5;
					bool thumb			      :  1;
					bool fiq_d            :  1;
					bool irq_d            :  1;
					unsigned int reserved : 19;
					bool s_overflow       :  1;
					bool f_overflow       :  1;
					bool f_carry          :  1;
					bool f_zero           :  1;
					bool f_sign           :  1;
				} b;
			};
			struct IPsr
			{
				uint8_t mode;
				bool thumb;
				bool fiq_d;
				bool irq_d;
				bool s_overflow;
				bool f_overflow;
				bool f_carry;
				bool f_zero;
				bool f_sign;
			};
			enum Modes
			{
				M_USR = 0x10,
				M_FIQ = 0x11,
				M_IRQ = 0x12,
				M_SVC = 0x13,
				M_ABT = 0x17,
				M_UND = 0x1B,
				M_SYS = 0x1F
			};

			Cpu ();
			virtual ~Cpu () {}

			virtual void Reset ();
			virtual void SoftReset ();

			void UpdateICpsr ();
			void UpdateCpsr ();
			void SwitchToMode (uint8_t newmode);
			void SwitchModeBack ();

			virtual void SendInterrupt (uint16_t interrupt) = 0;
			virtual void CheckInterrupt () = 0;
			void Interrupt ();
			void SoftwareInterrupt (uint32_t comment);
			void SoftwareInterrupt ();

			uint32_t& Reg(uint8_t r)
			{
				return m_st.r[r];
			}

			Psr& Cpsr()
			{
				return m_st.cpsr;
			}
			IPsr& ICpsr()
			{
				return m_st.icpsr;
			}
			Psr& Spsr()
			{
				return m_st.spsr;
			}

			bool SaveState (std::ostream& stream);
			bool LoadState (std::istream& stream);

		protected :
			struct CPUState
			{
				// Current registers
				uint32_t r[16];
				Psr cpsr, spsr;
				IPsr icpsr;

				// System/User
				uint32_t usr_r[7]; // from 8 to 14

				// FIQ
				uint32_t fiq_r[7]; // from 8 to 14
				Psr fiq_spsr;

				// Supervisor
				uint32_t svc_r[2]; // 13 and 14
				Psr svc_spsr;

				// Abort
				uint32_t abt_r[2]; // 13 and 14
				Psr abt_spsr;

				// IRQ
				uint32_t irq_r[2]; // 13 and 14
				Psr irq_spsr;

				// Undefined
				uint32_t und_r[2]; // 13 and 14
				Psr und_spsr;
			};

			CPUState m_st;

			virtual void SetInterrupt (bool interrupt) = 0;

		private :
			void SaveMode (uint8_t mode);
	};
}

#endif
