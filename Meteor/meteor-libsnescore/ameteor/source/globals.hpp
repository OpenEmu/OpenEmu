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

#ifndef __GLOBALS_H__
#define __GLOBALS_H__

#include <algorithm>
#include <stdint.h>

#define SPDUP_FRMSKIP 9
#define SPDUP_SNDSKIP 3969

#define R(reg) CPU.Reg(reg)

#define CPU    _cpu
#define MEM    _memory
#define IO     _io
#define DMA    _dma
#define LCD    _lcd
#define SOUND  _sound
#define KEYPAD _keypad
#define CLOCK  _clock
#define TIMER0 _timer0
#define TIMER1 _timer1
#define TIMER2 _timer2
#define TIMER3 _timer3

#define CYCLES16NSeq(add, count) \
	CLOCK.TimePass(MEM.GetCycles16NoSeq(add, count))
#define CYCLES16Seq(add, count) \
	CLOCK.TimePass(MEM.GetCycles16Seq(add, count))
#define CYCLES32NSeq(add, count) \
	CLOCK.TimePass(MEM.GetCycles32NoSeq(add, count))
#define CYCLES32Seq(add, count) \
	CLOCK.TimePass(MEM.GetCycles32Seq(add, count))
#define ICYCLES(i) \
	CLOCK.TimePass(i)
// we don't mind if the following code is slow since a MUL
// instruction is slow on a GBA too
#define MULICYCLES(reg) \
	{ \
		uint32_t rs = R(reg) >> 8; \
		for (uint8_t i = 1; i <= 4; ++i, rs >>= 8) \
		{ \
			if (!rs || rs == (0xFFFFFFFF >> (i*8))) \
			{ \
				ICYCLES(i); \
				break; \
			} \
		} \
	}

#define CPSR   (CPU.Cpsr().dw)
#define ICPSR   (CPU.ICpsr())
#define SPSR   (CPU.Spsr().dw)
#define FLAG_Z (CPU.ICpsr().f_zero)
#define FLAG_N (CPU.ICpsr().f_sign)
#define FLAG_C (CPU.ICpsr().f_carry)
#define FLAG_V (CPU.ICpsr().f_overflow)
#define FLAG_T (CPU.ICpsr().thumb)
#define SETF(flag, val) \
	FLAG_##flag = (val) ? 1 : 0
#define SETFB(flag, val) \
	FLAG_##flag = (val)

#define FZ(val) \
	SETF(Z, !(val))
#define FN(val) \
	SETF(N, NEG(val))

#define POS(a) ((~a) >> 31)
#define NEG(a) ((a) >> 31)

// inspired by VisualBoyAdvance
#define ADDCARRY(a, b, c) \
	((NEG(a) & NEG(b)) | \
	 (NEG(a) & POS(c)) | \
	 (NEG(b) & POS(c)))
#define ADDOVERFLOW(a, b, c) \
	((NEG(a) & NEG(b) & POS(c)) | \
	 (POS(a) & POS(b) & NEG(c)))
#define SUBCARRY(a, b, c) \
	((NEG(a) & POS(b)) | \
	 (NEG(a) & POS(c)) | \
	 (POS(b) & POS(c)))
#define SUBOVERFLOW(a, b, c) \
	((NEG(a) & POS(b) & POS(c)) | \
	 (POS(a) & NEG(b) & NEG(c)))

// Save states macros
#define SS_WRITE_VAR(var) \
	if (!stream.write((char*)&var, sizeof(var))) \
		return false
#define SS_WRITE_ARRAY(var) \
	if (!stream.write((char*)var, sizeof(var))) \
		return false
#define SS_WRITE_DATA(var, size) \
	if (!stream.write((char*)var, size)) \
		return false
#define SS_READ_VAR(var) \
	if (!stream.read((char*)&var, sizeof(var))) \
		return false
#define SS_READ_ARRAY(var) \
	if (!stream.read((char*)var, sizeof(var))) \
		return false
#define SS_READ_DATA(var, size) \
	if (!stream.read((char*)var, size)) \
		return false

// macro to avoid getting warnings about and unused parameter on GCC
#define MET_UNUSED(v) \
	__attribute__((unused)) MET_UNUSED_##v

namespace AMeteor
{
	// ROtate Right
	inline uint32_t ROR(uint32_t val, uint8_t shift)
	{
		return (val >> shift) | (val << (32 - shift));
	}
}

#endif
