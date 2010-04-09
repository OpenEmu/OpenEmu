////////////////////////////////////////////////////////////////////////////////////////
//
// Nestopia - NES/Famicom emulator written in C++
//
// Copyright (C) 2003-2008 Martin Freij
//
// This file is part of Nestopia.
//
// Nestopia is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 2 of the License, or
// (at your option) any later version.
//
// Nestopia is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with Nestopia; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
//
////////////////////////////////////////////////////////////////////////////////////////

#ifndef NST_CPU_H
#define NST_CPU_H

#include "NstAssert.hpp"
#include "NstIoMap.hpp"
#include "NstApu.hpp"

#ifdef NST_PRAGMA_ONCE
#pragma once
#endif

namespace Nes
{
	namespace Core
	{
		class Hook;

		class Cpu
		{
		public:

			Cpu();

			enum
			{
				CYCLE_MAX = Cycle(~0UL),
				RAM_SIZE  = SIZE_2K
			};

			enum IrqLine
			{
				IRQ_EXT   = 0x01,
				IRQ_FRAME = 0x40,
				IRQ_DMC   = 0x80
			};

			enum Level
			{
				LEVEL_LOW     = 1,
				LEVEL_HIGH    = 9,
				LEVEL_HIGHEST = 10
			};

			void Reset(bool);
			void Boot(bool);
			void ExecuteFrame(Sound::Output*);
			void EndFrame();
			void PowerOff();

			void DoNMI(Cycle);
			void DoIRQ(IrqLine,Cycle);

			uint Peek(uint) const;
			void Poke(uint,uint) const;

			bool IsOddCycle() const;
			bool IsWriteCycle(Cycle) const;

			void SetModel(CpuModel);
			void AddHook(const Hook&);
			void RemoveHook(const Hook&);

			void SaveState(State::Saver&,dword,dword) const;
			void LoadState(State::Loader&,dword,dword,dword);

			static Cycle ClockConvert(Cycle,CpuModel);

		private:

			static void NotifyOp(const char (&)[4],dword);

			enum
			{
				NMI_VECTOR     = 0xFFFA,
				RESET_VECTOR   = 0xFFFC,
				IRQ_VECTOR     = 0xFFFE,
				RESET_CYCLES   = 7,
				INT_CYCLES     = 7,
				BRK_CYCLES     = 7,
				RTI_CYCLES     = 6,
				RTS_CYCLES     = 6,
				PHA_CYCLES     = 3,
				PHP_CYCLES     = 3,
				PLA_CYCLES     = 4,
				PLP_CYCLES     = 4,
				JSR_CYCLES     = 6,
				JMP_ABS_CYCLES = 3,
				JMP_IND_CYCLES = 5
			};

			void Reset(bool,bool);

			NES_DECL_POKE( Nop      );
			NES_DECL_PEEK( Nop      );
			NES_DECL_POKE( Overflow );
			NES_DECL_PEEK( Overflow );
			NES_DECL_PEEK( Jam_1    );
			NES_DECL_PEEK( Jam_2    );

			void DoISR(uint);
			uint FetchIRQISRVector();
			void Clock();

			void Run0();
			void Run1();
			void Run2();

			inline void ExecuteOp();
			inline uint FetchPc8();
			inline uint FetchPc16();
			inline uint FetchZpg16(uint) const;

			inline void Push8(uint);
			NST_FORCE_INLINE void Push16(uint);
			inline uint Pull8();
			inline uint Pull16();

			inline uint ZpgReg_R  (uint);
			inline uint ZpgReg_RW (uint&,uint);
			inline uint ZpgReg_W  (uint);

			uint AbsReg_R  (uint);
			uint AbsReg_RW (uint&,uint);
			NST_FORCE_INLINE uint AbsReg_W (uint);

			template<bool STATE>
			NST_FORCE_INLINE void Branch(uint);

			inline uint Imm_R  ();
			inline uint Zpg_R  ();
			inline uint ZpgX_R ();
			inline uint ZpgY_R ();
			uint Abs_R  ();
			inline uint AbsX_R ();
			inline uint AbsY_R ();
			uint IndX_R ();
			uint IndY_R ();

			inline uint Zpg_RW  (uint&);
			inline uint ZpgX_RW (uint&);
			inline uint ZpgY_RW (uint&);
			uint Abs_RW (uint&);
			inline uint AbsY_RW (uint&);
			inline uint AbsX_RW (uint&);
			inline uint IndX_RW (uint&);
			inline uint IndY_RW (uint&);

			inline uint Zpg_W  ();
			inline uint ZpgX_W ();
			inline uint ZpgY_W ();
			inline uint Abs_W  ();
			NST_FORCE_INLINE uint AbsX_W ();
			NST_FORCE_INLINE uint AbsY_W ();
			inline uint IndX_W ();
			NST_FORCE_INLINE uint IndY_W ();

			inline void StoreMem (uint,uint);
			inline void StoreZpg (uint,uint);

			inline void Lda (uint);
			inline void Ldx (uint);
			inline void Ldy (uint);

			inline uint Sta() const;
			inline uint Stx() const;
			inline uint Sty() const;

			NST_SINGLE_CALL void Tax ();
			NST_SINGLE_CALL void Tay ();
			NST_SINGLE_CALL void Txa ();
			NST_SINGLE_CALL void Tya ();

			NST_SINGLE_CALL void JmpAbs ();
			NST_SINGLE_CALL void JmpInd ();
			NST_SINGLE_CALL void Jsr    ();
			NST_SINGLE_CALL void Rts    ();
			NST_SINGLE_CALL void Rti    ();

			NST_SINGLE_CALL void Bcc ();
			NST_SINGLE_CALL void Bcs ();
			NST_SINGLE_CALL void Beq ();
			NST_SINGLE_CALL void Bmi ();
			NST_SINGLE_CALL void Bne ();
			NST_SINGLE_CALL void Bpl ();
			NST_SINGLE_CALL void Bvc ();
			NST_SINGLE_CALL void Bvs ();

			inline void Adc (uint);
			inline void Sbc (uint);

			inline void And (uint);
			inline void Ora (uint);
			inline void Eor (uint);

			inline void Bit (uint);
			inline void Cmp (uint);
			inline void Cpx (uint);
			inline void Cpy (uint);

			inline uint Asl (uint);
			inline uint Lsr (uint);
			inline uint Rol (uint);
			inline uint Ror (uint);

			inline uint Dec (uint);
			inline uint Inc (uint);

			NST_SINGLE_CALL void Dex ();
			NST_SINGLE_CALL void Dey ();
			NST_SINGLE_CALL void Inx ();
			NST_SINGLE_CALL void Iny ();

			NST_SINGLE_CALL void Clc ();
			NST_SINGLE_CALL void Cld ();
			NST_SINGLE_CALL void Clv ();
			NST_SINGLE_CALL void Sec ();
			NST_SINGLE_CALL void Sed ();
			NST_SINGLE_CALL void Sei ();
			NST_SINGLE_CALL void Cli ();

			NST_SINGLE_CALL void Php ();
			NST_SINGLE_CALL void Plp ();
			NST_SINGLE_CALL void Pha ();
			NST_SINGLE_CALL void Pla ();
			NST_SINGLE_CALL void Tsx ();
			NST_SINGLE_CALL void Txs ();

			NST_SINGLE_CALL void Ane (uint);
			NST_SINGLE_CALL void Arr (uint);
			NST_SINGLE_CALL void Asr (uint);
			NST_SINGLE_CALL void Las (uint);
			NST_SINGLE_CALL void Lxa (uint);
			NST_SINGLE_CALL void Sbx (uint);
			NST_SINGLE_CALL uint Shs (uint);
			NST_SINGLE_CALL uint Shx (uint);
			NST_SINGLE_CALL uint Shy (uint);

			NST_NO_INLINE void Anc (uint);
			NST_NO_INLINE uint Dcp (uint);
			NST_NO_INLINE uint Isb (uint);
			NST_NO_INLINE void Lax (uint);
			NST_NO_INLINE uint Rla (uint);
			NST_NO_INLINE uint Rra (uint);
			NST_NO_INLINE uint Sax ();
			NST_NO_INLINE uint Sha (uint);
			NST_NO_INLINE uint Slo (uint);
			NST_NO_INLINE uint Sre (uint);

			void Dop ();
			void Top (uint);

			NST_SINGLE_CALL void Brk ();
			NST_NO_INLINE void Jam ();

			void op0x00(); void op0x01(); void op0x02(); void op0x03();
			void op0x04(); void op0x05(); void op0x06(); void op0x07();
			void op0x08(); void op0x09(); void op0x0A(); void op0x0B();
			void op0x0C(); void op0x0D(); void op0x0E(); void op0x0F();
			void op0x10(); void op0x11(); void op0x12(); void op0x13();
			void op0x14(); void op0x15(); void op0x16(); void op0x17();
			void op0x18(); void op0x19(); void op0x1A(); void op0x1B();
			void op0x1C(); void op0x1D(); void op0x1E(); void op0x1F();
			void op0x20(); void op0x21(); void op0x22(); void op0x23();
			void op0x24(); void op0x25(); void op0x26(); void op0x27();
			void op0x28(); void op0x29(); void op0x2A(); void op0x2B();
			void op0x2C(); void op0x2D(); void op0x2E(); void op0x2F();
			void op0x30(); void op0x31(); void op0x32(); void op0x33();
			void op0x34(); void op0x35(); void op0x36(); void op0x37();
			void op0x38(); void op0x39(); void op0x3A(); void op0x3B();
			void op0x3C(); void op0x3D(); void op0x3E(); void op0x3F();
			void op0x40(); void op0x41(); void op0x42(); void op0x43();
			void op0x44(); void op0x45(); void op0x46(); void op0x47();
			void op0x48(); void op0x49(); void op0x4A(); void op0x4B();
			void op0x4C(); void op0x4D(); void op0x4E(); void op0x4F();
			void op0x50(); void op0x51(); void op0x52(); void op0x53();
			void op0x54(); void op0x55(); void op0x56(); void op0x57();
			void op0x58(); void op0x59(); void op0x5A(); void op0x5B();
			void op0x5C(); void op0x5D(); void op0x5E(); void op0x5F();
			void op0x60(); void op0x61(); void op0x62(); void op0x63();
			void op0x64(); void op0x65(); void op0x66(); void op0x67();
			void op0x68(); void op0x69(); void op0x6A(); void op0x6B();
			void op0x6C(); void op0x6D(); void op0x6E(); void op0x6F();
			void op0x70(); void op0x71(); void op0x72(); void op0x73();
			void op0x74(); void op0x75(); void op0x76(); void op0x77();
			void op0x78(); void op0x79(); void op0x7A(); void op0x7B();
			void op0x7C(); void op0x7D(); void op0x7E(); void op0x7F();
			void op0x80(); void op0x81(); void op0x82(); void op0x83();
			void op0x84(); void op0x85(); void op0x86(); void op0x87();
			void op0x88(); void op0x89(); void op0x8A(); void op0x8B();
			void op0x8C(); void op0x8D(); void op0x8E(); void op0x8F();
			void op0x90(); void op0x91(); void op0x92(); void op0x93();
			void op0x94(); void op0x95(); void op0x96(); void op0x97();
			void op0x98(); void op0x99(); void op0x9A(); void op0x9B();
			void op0x9C(); void op0x9D(); void op0x9E(); void op0x9F();
			void op0xA0(); void op0xA1(); void op0xA2(); void op0xA3();
			void op0xA4(); void op0xA5(); void op0xA6(); void op0xA7();
			void op0xA8(); void op0xA9(); void op0xAA(); void op0xAB();
			void op0xAC(); void op0xAD(); void op0xAE(); void op0xAF();
			void op0xB0(); void op0xB1(); void op0xB2(); void op0xB3();
			void op0xB4(); void op0xB5(); void op0xB6(); void op0xB7();
			void op0xB8(); void op0xB9(); void op0xBA(); void op0xBB();
			void op0xBC(); void op0xBD(); void op0xBE(); void op0xBF();
			void op0xC0(); void op0xC1(); void op0xC2(); void op0xC3();
			void op0xC4(); void op0xC5(); void op0xC6(); void op0xC7();
			void op0xC8(); void op0xC9(); void op0xCA(); void op0xCB();
			void op0xCC(); void op0xCD(); void op0xCE(); void op0xCF();
			void op0xD0(); void op0xD1(); void op0xD2(); void op0xD3();
			void op0xD4(); void op0xD5(); void op0xD6(); void op0xD7();
			void op0xD8(); void op0xD9(); void op0xDA(); void op0xDB();
			void op0xDC(); void op0xDD(); void op0xDE(); void op0xDF();
			void op0xE0(); void op0xE1(); void op0xE2(); void op0xE3();
			void op0xE4(); void op0xE5(); void op0xE6(); void op0xE7();
			void op0xE8(); void op0xE9(); void op0xEA(); void op0xEB();
			void op0xEC(); void op0xED(); void op0xEE(); void op0xEF();
			void op0xF0(); void op0xF1(); void op0xF2(); void op0xF3();
			void op0xF4(); void op0xF5(); void op0xF6(); void op0xF7();
			void op0xF8(); void op0xF9(); void op0xFA(); void op0xFB();
			void op0xFC(); void op0xFD(); void op0xFE(); void op0xFF();

			struct Cycles
			{
				void SetModel(CpuModel);
				inline uint InterruptEdge() const;

				Cycle count;
				byte clock[8];
				Cycle offset;
				Cycle round;
				Cycle frame;

				void NextRound(Cycle next)
				{
					if (round > next)
						round = next;
				}
			};

			struct Flags
			{
				uint Pack() const;
				void Unpack(uint);

				enum
				{
					C = 0x01, // carry
					Z = 0x02, // zero
					I = 0x04, // interrupt enable/disable
					D = 0x08, // decimal mode (not supported on the N2A03)
					B = 0x10, // software interrupt
					R = 0x20, // unused but always set
					V = 0x40, // overflow
					N = 0x80  // negative
				};

				uint nz;
				uint c;
				uint v;
				uint i;
				uint d;
			};

			struct Interrupt
			{
				void Reset();
				void SetModel(CpuModel);

				Cycle nmiClock;
				Cycle irqClock;
				uint low;
			};

			class Hooks
			{
			public:

				Hooks();
				~Hooks();

				void Add(const Hook&);
				void Remove(const Hook&);

				void Clear();
				inline uint Size() const;
				inline const Hook* Ptr() const;

			private:

				Hook* hooks;
				word size;
				word capacity;
			};

			struct Ram
			{
				typedef byte (&Ref)[RAM_SIZE];
				typedef const byte (&ConstRef)[RAM_SIZE];

				void Reset();

				NES_DECL_PEEK( Ram_0 );
				NES_DECL_POKE( Ram_0 );
				NES_DECL_PEEK( Ram_1 );
				NES_DECL_POKE( Ram_1 );
				NES_DECL_PEEK( Ram_2 );
				NES_DECL_POKE( Ram_2 );
				NES_DECL_PEEK( Ram_3 );
				NES_DECL_POKE( Ram_3 );

				byte mem[RAM_SIZE];
			};

			struct IoMap : Io::Map<SIZE_64K>
			{
				template<typename T,typename U>
				IoMap(Cpu*,T,U);

				inline uint Peek8(uint) const;
				inline uint Peek16(uint) const;
				inline void Poke8(uint,uint) const;
			};

			class Linker
			{
				struct Chain : Io::Port
				{
					Chain(const Port&,uint,uint=0);

					uint address;
					uint level;
					Chain* next;
				};

				Chain* chain;

			public:

				Linker();
				~Linker();

				void Clear();
				const Io::Port* Add(Address,uint,const Io::Port&,IoMap&);
				void Remove(Address,const Io::Port&,IoMap&);
			};

			uint pc;
			Cycles cycles;
			uint a;
			uint x;
			uint y;
			uint sp;
			Flags flags;
			Interrupt interrupt;
			Hooks hooks;
			uint opcode;
			word jammed;
			word model;
			Linker linker;
			qword ticks;
			Ram ram;
			Apu apu;
			IoMap map;

			static dword logged;
			static void (Cpu::*const opcodes[0x100])();
			static const byte writeClocks[0x100];

		public:

			Apu& GetApu()
			{
				return apu;
			}

			Cycle Update()
			{
				apu.ClockDMA();
				return cycles.count;
			}

			void DoIRQ(IrqLine line=IRQ_EXT)
			{
				DoIRQ( line, cycles.count );
			}

			void ClearIRQ(IrqLine line=IRQ_EXT)
			{
				NST_VERIFY( interrupt.irqClock == CYCLE_MAX );

				interrupt.low &= line ^ uint(IRQ_EXT|IRQ_FRAME|IRQ_DMC);

				if (!interrupt.low)
					interrupt.irqClock = CYCLE_MAX;
			}

			uint GetIRQ() const
			{
				return interrupt.low;
			}

			CpuModel GetModel() const
			{
				return static_cast<CpuModel>(model);
			}

			Region GetRegion() const
			{
				return GetModel() == CPU_RP2A03 ? REGION_NTSC : REGION_PAL;
			}

			Cycle GetClock(uint count=1) const
			{
				NST_ASSERT( count >= 1 && count <= 8 );
				return cycles.clock[count-1];
			}

			Cycle GetCycles() const
			{
				return cycles.count;
			}

			void StealCycles(Cycle count)
			{
				cycles.count += count;
			}

			Cycle GetFrameCycles() const
			{
				return cycles.frame;
			}

			void SetFrameCycles(Cycle count)
			{
				cycles.frame = count;
				cycles.NextRound( count );
			}

			Ram::Ref GetRam()
			{
				return ram.mem;
			}

			Ram::ConstRef GetRam() const
			{
				return ram.mem;
			}

			Io::Port& Map(Address address)
			{
				return map( address );
			}

			IoMap::Section Map(Address first,Address last)
			{
				return map( first, last );
			}

			template<typename T,typename U,typename V>
			const Io::Port* Link(Address address,Level level,T t,U u,V v)
			{
				return linker.Add( address, level, Io::Port(t,u,v), map );
			}

			template<typename T,typename U,typename V>
			void Unlink(Address address,T t,U u,V v)
			{
				linker.Remove( address, Io::Port(t,u,v), map );
			}
		};
	}
}

#endif
