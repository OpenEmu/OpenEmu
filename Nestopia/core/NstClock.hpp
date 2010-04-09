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

#ifndef NST_CLOCK_H
#define NST_CLOCK_H

#ifndef NST_CPU_H
#include "NstCpu.hpp"
#endif

#ifndef NST_PPU_H
#include "NstPpu.hpp"
#endif

#ifdef NST_PRAGMA_ONCE
#pragma once
#endif

namespace Nes
{
	namespace Core
	{
		namespace ClockUnits
		{
			template<typename Unit,uint Divider=1>
			class M2
			{
			public:

				explicit M2(Cpu&);

				template<typename Param>
				M2(Cpu&,Param&);

				template<typename Param>
				M2(Cpu&,const Param&);

				void Reset(bool,bool);
				void VSync();

			private:

				enum
				{
					IRQ_SETUP = 2
				};

				NES_DECL_HOOK( Signaled );

				Cycle count;
				ibool connected;
				Cpu& cpu;

			public:

				Unit unit;

				bool Connect(bool connect)
				{
					connected = connect;
					return connect;
				}

				bool Connected() const
				{
					return connected;
				}

				void Update()
				{
					M2::NES_DO_HOOK( Signaled );
				}

				void ClearIRQ() const
				{
					cpu.ClearIRQ();
				}
			};

			template<typename Unit,uint Divider>
			M2<Unit,Divider>::M2(Cpu& c)
			: count(0), connected(false), cpu(c)
			{
			}

			template<typename Unit,uint Divider>
			template<typename Param>
			M2<Unit,Divider>::M2(Cpu& c,Param& p)
			: count(0), connected(false), cpu(c), unit(p)
			{
			}

			template<typename Unit,uint Divider>
			template<typename Param>
			M2<Unit,Divider>::M2(Cpu& c,const Param& p)
			: count(0), connected(false), cpu(c), unit(p)
			{
			}

			template<typename Unit,uint Divider>
			void M2<Unit,Divider>::Reset(const bool hard,const bool connect)
			{
				count = 0;
				connected = connect;
				unit.Reset( hard );
				cpu.AddHook( Hook(this,&M2::Hook_Signaled) );
			}

			NES_HOOK_T(template<typename Unit NST_COMMA uint Divider>,M2<Unit NST_COMMA Divider>,Signaled)
			{
				NST_COMPILE_ASSERT( Divider <= 8 );

				while (count <= cpu.GetCycles())
				{
					if (connected && unit.Clock())
						cpu.DoIRQ( Cpu::IRQ_EXT, count + cpu.GetClock(IRQ_SETUP) );

					count += cpu.GetClock(Divider);
				}
			}

			template<typename Unit,uint Divider>
			void M2<Unit,Divider>::VSync()
			{
				NST_VERIFY( count == 0 || count >= cpu.GetFrameCycles());
				count = (count > cpu.GetFrameCycles() ? count - cpu.GetFrameCycles() : 0);
			}

			template<typename Unit,uint Hold,uint Delay>
			class A12;

			template<>
			class A12<void,0,0>
			{
			protected:

				template<uint Hold>
				class Filter
				{
					Cycle clock;

				public:

					Filter()
					: clock(0) {}

					void Reset()
					{
						clock = 0;
					}

					bool Clock(const Ppu& ppu,Cycle cycle)
					{
						const Cycle next = clock;
						clock = cycle + ppu.GetClock(Hold);
						return cycle >= next;
					}

					void VSync(const Cpu& cpu)
					{
						clock = (clock > cpu.GetFrameCycles() ? clock - cpu.GetFrameCycles() : 0);
					}
				};
			};

			template<>
			class A12<void,0,0>::Filter<0>
			{
				const Cycle clock;

			public:

				Filter()
				: clock(0) {}

				void Reset() const
				{
				}

				bool Clock(const Ppu&,Cycle) const
				{
					return true;
				}

				void VSync(const Cpu&) const
				{
				}
			};

			template<typename Unit,uint Hold=0,uint Delay=0>
			class A12 : A12<void,0,0>
			{
			public:

				void Reset(bool,bool=true);

			private:

				NES_DECL_LINE( Signaled );

				Filter<Hold> filter;
				Cpu& cpu;
				Ppu& ppu;

			public:

				Unit unit;

				A12(Cpu& c,Ppu& p)
				: cpu(c), ppu(p) {}

				template<typename Param>
				A12(Cpu& c,Ppu& p,Param& a)
				: cpu(c), ppu(p), unit(a) {}

				void Connect(bool connect)
				{
					if (connect)
						ppu.A12().Set( this, &A12<Unit,Hold,Delay>::Line_Signaled );
					else
						ppu.A12().Unset();
				}

				bool Connected() const
				{
					return ppu.A12();
				}

				void Update() const
				{
					ppu.Update();
				}

				void ClearIRQ() const
				{
					cpu.ClearIRQ();
				}

				void VSync()
				{
					filter.VSync( cpu );
				}
			};

			template<typename Unit,uint Hold,uint Delay>
			void A12<Unit,Hold,Delay>::Reset(bool hard,bool connect)
			{
				filter.Reset();
				unit.Reset( hard );
				Connect( connect );
				ppu.EnableCpuSynchronization();
			}

			NES_LINE_T(template<typename Unit NST_COMMA uint Hold NST_COMMA uint Delay>,A12<Unit NST_COMMA Hold NST_COMMA Delay>,Signaled)
			{
				NST_COMPILE_ASSERT( Delay <= 8 );

				if (filter.Clock(ppu,cycle) && unit.Clock())
					cpu.DoIRQ( Cpu::IRQ_EXT, cycle + (Delay ? cpu.GetClock(Delay) : 0) );
			}
		}
	}
}

#endif
