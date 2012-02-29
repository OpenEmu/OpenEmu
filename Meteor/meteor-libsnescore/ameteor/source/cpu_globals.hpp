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

#ifndef __CPU_GLOBALS_H__
#define __CPU_GLOBALS_H__

#undef R
#define R(reg) m_st.r[reg]

#undef CPU
#define CPU (*this)

#undef CPSR
#undef SPSR
#undef FLAG_Z
#undef FLAG_N
#undef FLAG_C
#undef FLAG_V
#undef FLAG_T
#define CPSR   (m_st.cpsr.dw)
#define SPSR   (m_st.spsr.dw)
#define FLAG_Z (m_st.icpsr.f_zero)
#define FLAG_N (m_st.icpsr.f_sign)
#define FLAG_C (m_st.icpsr.f_carry)
#define FLAG_V (m_st.icpsr.f_overflow)
#define FLAG_T (m_st.icpsr.thumb)

#endif
