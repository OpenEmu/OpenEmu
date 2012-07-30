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

#ifndef NST_MEMORY_H
#define NST_MEMORY_H

#include "NstRam.hpp"

#ifdef NST_PRAGMA_ONCE
#pragma once
#endif

namespace Nes
{
	namespace Core
	{
		template<typename T> class Pointer : public ImplicitBool< Pointer<T> >
		{
			Pointer(const Pointer&);

			T* const ptr;

		public:

			explicit Pointer(T* t)
			: ptr(t) {}

			~Pointer()
			{
				typedef char TypeComplete[sizeof(T)];
				delete ptr;
			}

			T* operator -> () const
			{
				return ptr;
			}

			T& operator * () const
			{
				return *ptr;
			}

			bool operator ! () const
			{
				return !ptr;
			}
		};

		namespace State
		{
			class Saver;
			class Loader;
		}

		template<dword SPACE,uint U,uint V>
		class Memory;

		template<>
		class Memory<0,0,0>
		{
		protected:

			enum
			{
				MAX_SOURCES = 2
			};

			void SaveState
			(
				State::Saver&,
				dword,
				const Ram* NST_RESTRICT,
				uint,
				const byte* NST_RESTRICT,
				uint
			)   const;

			bool LoadState
			(
				State::Loader&,
				Ram* NST_RESTRICT,
				uint,
				byte* NST_RESTRICT,
				uint
			)   const;

			template<uint N> struct Pages
			{
				byte* mem[N];
				byte ref[N];
			};

			template<uint OFFSET,uint COUNT,uint SIZE,uint I=COUNT>
			struct Unroller
			{
				template<typename Pages>
				static NST_FORCE_INLINE void SwapBank
				(
					Pages* const NST_RESTRICT pages,
					byte* const NST_RESTRICT mem,
					const dword mask,
					const dword bank,
					const uint offset=0,
					const uint source=0
				)
				{
					pages->mem[OFFSET+COUNT-I+offset] = mem + (bank & mask);
					pages->ref[OFFSET+COUNT-I+offset] = source;

					Unroller<OFFSET,COUNT,SIZE,I-1>::SwapBank( pages, mem, mask, bank + SIZE, offset, source );
				}

				template<typename Pages>
				static NST_FORCE_INLINE void SwapBanks
				(
					Pages* const NST_RESTRICT pages,
					byte* const NST_RESTRICT mem,
					const dword mask,
					const dword bank0,
					const dword bank1,
					const uint offset=0
				)
				{
					Unroller<OFFSET+COUNT*0,COUNT,SIZE,I>::SwapBank( pages, mem, mask, bank0, offset );
					Unroller<OFFSET+COUNT*1,COUNT,SIZE,I>::SwapBank( pages, mem, mask, bank1, offset );
				}

				template<typename Pages>
				static NST_FORCE_INLINE void SwapBanks
				(
					Pages* const NST_RESTRICT pages,
					byte* const NST_RESTRICT mem,
					const dword mask,
					const dword bank0,
					const dword bank1,
					const dword bank2,
					const dword bank3,
					const uint offset=0
				)
				{
					Unroller<OFFSET+COUNT*0,COUNT,SIZE,I>::SwapBank( pages, mem, mask, bank0, offset );
					Unroller<OFFSET+COUNT*1,COUNT,SIZE,I>::SwapBank( pages, mem, mask, bank1, offset );
					Unroller<OFFSET+COUNT*2,COUNT,SIZE,I>::SwapBank( pages, mem, mask, bank2, offset );
					Unroller<OFFSET+COUNT*3,COUNT,SIZE,I>::SwapBank( pages, mem, mask, bank3, offset );
				}

				template<typename Pages>
				static NST_FORCE_INLINE void SwapBanks
				(
					Pages* const NST_RESTRICT pages,
					byte* const NST_RESTRICT mem,
					const dword mask,
					const dword bank0,
					const dword bank1,
					const dword bank2,
					const dword bank3,
					const dword bank4,
					const dword bank5,
					const dword bank6,
					const dword bank7,
					const uint offset=0
				)
				{
					Unroller<OFFSET+COUNT*0,COUNT,SIZE,I>::SwapBank( pages, mem, mask, bank0, offset );
					Unroller<OFFSET+COUNT*1,COUNT,SIZE,I>::SwapBank( pages, mem, mask, bank1, offset );
					Unroller<OFFSET+COUNT*2,COUNT,SIZE,I>::SwapBank( pages, mem, mask, bank2, offset );
					Unroller<OFFSET+COUNT*3,COUNT,SIZE,I>::SwapBank( pages, mem, mask, bank3, offset );
					Unroller<OFFSET+COUNT*4,COUNT,SIZE,I>::SwapBank( pages, mem, mask, bank4, offset );
					Unroller<OFFSET+COUNT*5,COUNT,SIZE,I>::SwapBank( pages, mem, mask, bank5, offset );
					Unroller<OFFSET+COUNT*6,COUNT,SIZE,I>::SwapBank( pages, mem, mask, bank6, offset );
					Unroller<OFFSET+COUNT*7,COUNT,SIZE,I>::SwapBank( pages, mem, mask, bank7, offset );
				}
			};
		};

		template<>
		struct Memory<0,0,0>::Pages<1>
		{
			byte* mem[1];
			dword ref[1];
		};

		template<>
		struct Memory<0,0,0>::Pages<2>
		{
			byte* mem[2];
			word ref[2];
		};

		template<uint OFFSET,uint COUNT,uint SIZE>
		struct Memory<0,0,0>::Unroller<OFFSET,COUNT,SIZE,0U>
		{
			template<typename Pages>
			static NST_FORCE_INLINE void SwapBank(Pages*,byte*,dword,dword,uint,uint) {}
		};

		template<dword SPACE,uint U,uint V=1>
		class Memory : Memory<0,0,0>
		{
		public:

			enum
			{
				NUM_SOURCES = V
			};

		private:

			NST_COMPILE_ASSERT
			(
				((SPACE & (SPACE-1)) == 0) &&
				((U & (U-1)) == 0) &&
				(SPACE % U == 0) &&
				(V >= 1 && V <= MAX_SOURCES)
			);

			enum
			{
				MEM_PAGE_SIZE = U,
				MEM_PAGE_SHIFT = ValueBits<MEM_PAGE_SIZE>::VALUE-1,
				MEM_PAGE_MASK = MEM_PAGE_SIZE - 1,
				MEM_NUM_PAGES = SPACE / U
			};

			typedef Memory<0,0,0>::Pages<MEM_NUM_PAGES> Pages;

			Pages pages;
			Ram sources[NUM_SOURCES];

		public:

			bool Readable(uint page) const
			{
				return sources[pages.ref[page]].Readable();
			}

			bool Writable(uint page) const
			{
				return sources[pages.ref[page]].Writable();
			}

			const byte& Peek(uint address) const
			{
				return pages.mem[address >> MEM_PAGE_SHIFT][address & MEM_PAGE_MASK];
			}

			byte* operator [] (uint page)
			{
				return pages.mem[page];
			}

			const byte* operator [] (uint page) const
			{
				return pages.mem[page];
			}

			void Poke(uint address,uint data)
			{
				const uint page = address >> MEM_PAGE_SHIFT;
				NST_VERIFY( Writable( page ) );

				if (Writable( page ))
					pages.mem[page][address & MEM_PAGE_MASK] = data;
			}

			template<uint SIZE,uint ADDRESS>
			void SwapBank(dword);

			template<uint SIZE,uint ADDRESS>
			void SwapBanks(dword,dword);

			template<uint SIZE,uint ADDRESS>
			void SwapBanks(dword,dword,dword,dword);

			template<uint SIZE,uint ADDRESS>
			void SwapBanks(dword,dword,dword,dword,dword,dword,dword,dword);

			template<uint SIZE>
			void SwapBank(uint,dword);

			template<uint SIZE>
			void SwapBanks(uint,dword,dword);

			template<uint SIZE>
			void SwapBanks(uint,dword,dword,dword,dword);

			template<uint SIZE>
			void SwapBanks(uint,dword,dword,dword,dword,dword,dword,dword,dword);

			template<uint SIZE,uint A,uint B>
			void SwapPages();

			void SaveState(State::Saver&,dword) const;
			void LoadState(State::Loader&);

			class SourceProxy
			{
				typedef Memory<SPACE,U,V> Ref;

				const uint source;
				Ref& ref;

			public:

				SourceProxy(uint s,Ref& r)
				: source(s), ref(r)
				{
					NST_ASSERT( s < NUM_SOURCES );
				}

				template<uint SIZE,uint ADDRESS>
				void SwapBank(dword) const;

				template<uint SIZE>
				void SwapBank(uint,dword) const;

				void ReadEnable(bool read) const
				{
					ref.sources[source].ReadEnable( read );
				}

				void WriteEnable(bool write) const
				{
					ref.sources[source].WriteEnable( write );
				}

				void SetSecurity(bool read,bool write) const
				{
					ref.sources[source].SetSecurity( read, write );
				}

				bool Readable() const
				{
					return ref.sources[source].Readable();
				}

				bool Writable() const
				{
					return ref.sources[source].Writable();
				}

				Ram::Type GetType() const
				{
					return ref.sources[source].GetType();
				}

				void Set(Ram::Type type,bool read,bool write,dword size,byte* mem) const
				{
					ref.sources[source].Set( type, read, write, size, mem );
				}

				void Set(Ram::Type type,bool read,bool write,dword size) const
				{
					ref.sources[source].Set( type, read, write, size );
				}

				void Set(const Ram& ram) const
				{
					ref.sources[source] = ram;
				}

				void Fill(uint value) const
				{
					ref.sources[source].Fill( value );
				}

				byte* Mem(dword offset=0) const
				{
					return ref.sources[source].Mem(offset);
				}

				byte& operator [] (dword i) const
				{
					return ref.sources[source][i];
				}

				dword Size() const
				{
					return ref.sources[source].Size();
				}

				dword Masking() const
				{
					return ref.sources[source].Masking();
				}

				bool Empty() const
				{
					return ref.sources[source].Size() == 0;
				}

				const Ram& Reference() const
				{
					return ref.sources[source];
				}
			};

		public:

			const SourceProxy Source(uint i=0)
			{
				NST_ASSERT( i < NUM_SOURCES );
				return SourceProxy( i, *this );
			}

			const Ram& Source(uint i=0) const
			{
				NST_ASSERT( i < NUM_SOURCES );
				return sources[i];
			}

			Memory()
			{
			}

			Memory(byte* mem,dword size,bool read,bool write)
			{
				Source().Set( mem, size, read, write );
			}

			Memory(dword size,bool read,bool write)
			{
				Source().Set( size, read, write );
			}

			template<uint SIZE,uint ADDRESS>
			dword GetBank() const
			{
				NST_COMPILE_ASSERT( SIZE && (SIZE % MEM_PAGE_SIZE == 0) && (SPACE >= ADDRESS + SIZE) );

				enum {MEM_PAGE = ADDRESS >> MEM_PAGE_SHIFT};
				return dword(pages.mem[MEM_PAGE] - sources[pages.ref[MEM_PAGE]].Mem()) >> (ValueBits<SIZE>::VALUE-1);
			}

			template<uint SIZE>
			dword GetBank(uint address) const
			{
				NST_COMPILE_ASSERT( SIZE && (SIZE % MEM_PAGE_SIZE == 0) );
				NST_ASSERT( SPACE >= address + SIZE );

				address >>= MEM_PAGE_SHIFT;
				return dword(pages.mem[address] - sources[pages.ref[address]].Mem()) >> (ValueBits<SIZE>::VALUE-1);
			}
		};

		template<dword SPACE,uint U,uint V> template<uint SIZE,uint ADDRESS>
		void Memory<SPACE,U,V>::SwapBank(dword bank)
		{
			NST_COMPILE_ASSERT( (SPACE >= ADDRESS + SIZE) && SIZE && (SIZE % MEM_PAGE_SIZE == 0) );

			enum
			{
				MEM_OFFSET = ValueBits<SIZE>::VALUE-1,
				MEM_PAGE_BEGIN = ADDRESS / MEM_PAGE_SIZE,
				MEM_PAGE_COUNT = SIZE / MEM_PAGE_SIZE
			};

			Memory<0,0,0>::Unroller<MEM_PAGE_BEGIN,MEM_PAGE_COUNT,MEM_PAGE_SIZE>::SwapBank
			(
				&pages,
				sources[0].Mem(),
				sources[0].Masking(),
				bank << MEM_OFFSET
			);
		}

		template<dword SPACE,uint U,uint V> template<uint SIZE>
		void Memory<SPACE,U,V>::SwapBank(uint address,dword bank)
		{
			NST_COMPILE_ASSERT( SIZE && (SIZE % MEM_PAGE_SIZE == 0) );
			NST_ASSERT( SPACE >= address + SIZE );

			enum
			{
				MEM_OFFSET = ValueBits<SIZE>::VALUE-1,
				MEM_PAGE_COUNT = SIZE / MEM_PAGE_SIZE
			};

			Memory<0,0,0>::Unroller<0,MEM_PAGE_COUNT,MEM_PAGE_SIZE>::SwapBank
			(
				&pages,
				sources[0].Mem(),
				sources[0].Masking(),
				bank << MEM_OFFSET,
				address >> MEM_PAGE_SHIFT
			);
		}

		template<dword SPACE,uint U,uint V> template<uint SIZE,uint ADDRESS>
		void Memory<SPACE,U,V>::SwapBanks(dword bank0,dword bank1)
		{
			NST_COMPILE_ASSERT( (SPACE >= ADDRESS + SIZE * 2) && SIZE && (SIZE % MEM_PAGE_SIZE == 0) );

			enum
			{
				MEM_OFFSET = ValueBits<SIZE>::VALUE-1,
				MEM_PAGE_BEGIN = ADDRESS / MEM_PAGE_SIZE,
				MEM_PAGE_COUNT = SIZE / MEM_PAGE_SIZE
			};

			Memory<0,0,0>::Unroller<MEM_PAGE_BEGIN,MEM_PAGE_COUNT,MEM_PAGE_SIZE>::SwapBanks
			(
				&pages,
				sources[0].Mem(),
				sources[0].Masking(),
				bank0 << MEM_OFFSET,
				bank1 << MEM_OFFSET
			);
		}

		template<dword SPACE,uint U,uint V> template<uint SIZE,uint ADDRESS>
		void Memory<SPACE,U,V>::SwapBanks(dword bank0,dword bank1,dword bank2,dword bank3)
		{
			NST_COMPILE_ASSERT( (SPACE >= ADDRESS + SIZE * 4) && SIZE && (SIZE % MEM_PAGE_SIZE == 0) );

			enum
			{
				MEM_OFFSET = ValueBits<SIZE>::VALUE-1,
				MEM_PAGE_BEGIN = ADDRESS / MEM_PAGE_SIZE,
				MEM_PAGE_COUNT = SIZE / MEM_PAGE_SIZE
			};

			Memory<0,0,0>::Unroller<MEM_PAGE_BEGIN,MEM_PAGE_COUNT,MEM_PAGE_SIZE>::SwapBanks
			(
				&pages,
				sources[0].Mem(),
				sources[0].Masking(),
				bank0 << MEM_OFFSET,
				bank1 << MEM_OFFSET,
				bank2 << MEM_OFFSET,
				bank3 << MEM_OFFSET
			);
		}

		template<dword SPACE,uint U,uint V> template<uint SIZE,uint ADDRESS>
		void Memory<SPACE,U,V>::SwapBanks(dword bank0,dword bank1,dword bank2,dword bank3,dword bank4,dword bank5,dword bank6,dword bank7)
		{
			NST_COMPILE_ASSERT( (SPACE >= ADDRESS + SIZE * 4) && SIZE && (SIZE % MEM_PAGE_SIZE == 0) );

			enum
			{
				MEM_OFFSET = ValueBits<SIZE>::VALUE-1,
				MEM_PAGE_BEGIN = ADDRESS / MEM_PAGE_SIZE,
				MEM_PAGE_COUNT = SIZE / MEM_PAGE_SIZE
			};

			Memory<0,0,0>::Unroller<MEM_PAGE_BEGIN,MEM_PAGE_COUNT,MEM_PAGE_SIZE>::SwapBanks
			(
				&pages,
				sources[0].Mem(),
				sources[0].Masking(),
				bank0 << MEM_OFFSET,
				bank1 << MEM_OFFSET,
				bank2 << MEM_OFFSET,
				bank3 << MEM_OFFSET,
				bank4 << MEM_OFFSET,
				bank5 << MEM_OFFSET,
				bank6 << MEM_OFFSET,
				bank7 << MEM_OFFSET
			);
		}

		template<dword SPACE,uint U,uint V> template<uint SIZE>
		void Memory<SPACE,U,V>::SwapBanks(uint address,dword bank0,dword bank1)
		{
			NST_COMPILE_ASSERT( SIZE && (SIZE % MEM_PAGE_SIZE == 0) );
			NST_ASSERT( SPACE >= address + SIZE * 2 );

			enum
			{
				MEM_OFFSET = ValueBits<SIZE>::VALUE-1,
				MEM_PAGE_COUNT = SIZE / MEM_PAGE_SIZE
			};

			Memory<0,0,0>::Unroller<0,MEM_PAGE_COUNT,MEM_PAGE_SIZE>::SwapBanks
			(
				&pages,
				sources[0].Mem(),
				sources[0].Masking(),
				bank0 << MEM_OFFSET,
				bank1 << MEM_OFFSET,
				address >> MEM_PAGE_SHIFT
			);
		}

		template<dword SPACE,uint U,uint V> template<uint SIZE>
		void Memory<SPACE,U,V>::SwapBanks(uint address,dword bank0,dword bank1,dword bank2,dword bank3)
		{
			NST_COMPILE_ASSERT( SIZE && (SIZE % MEM_PAGE_SIZE == 0) );
			NST_ASSERT( SPACE >= address + SIZE * 4 );

			enum
			{
				MEM_OFFSET = ValueBits<SIZE>::VALUE-1,
				MEM_PAGE_COUNT = SIZE / MEM_PAGE_SIZE
			};

			Memory<0,0,0>::Unroller<0,MEM_PAGE_COUNT,MEM_PAGE_SIZE>::SwapBanks
			(
				&pages,
				sources[0].Mem(),
				sources[0].Masking(),
				bank0 << MEM_OFFSET,
				bank1 << MEM_OFFSET,
				bank2 << MEM_OFFSET,
				bank3 << MEM_OFFSET,
				address >> MEM_PAGE_SHIFT
			);
		}

		template<dword SPACE,uint U,uint V> template<uint SIZE>
		void Memory<SPACE,U,V>::SwapBanks(uint address,dword bank0,dword bank1,dword bank2,dword bank3,dword bank4,dword bank5,dword bank6,dword bank7)
		{
			NST_COMPILE_ASSERT( SIZE && (SIZE % MEM_PAGE_SIZE == 0) );
			NST_ASSERT( SPACE >= address + SIZE * 4 );

			enum
			{
				MEM_OFFSET = ValueBits<SIZE>::VALUE-1,
				MEM_PAGE_COUNT = SIZE / MEM_PAGE_SIZE
			};

			Memory<0,0,0>::Unroller<0,MEM_PAGE_COUNT,MEM_PAGE_SIZE>::SwapBanks
			(
				&pages,
				sources[0].Mem(),
				sources[0].Masking(),
				bank0 << MEM_OFFSET,
				bank1 << MEM_OFFSET,
				bank2 << MEM_OFFSET,
				bank3 << MEM_OFFSET,
				bank4 << MEM_OFFSET,
				bank5 << MEM_OFFSET,
				bank6 << MEM_OFFSET,
				bank7 << MEM_OFFSET,
				address >> MEM_PAGE_SHIFT
			);
		}

		template<dword SPACE,uint U,uint V> template<uint SIZE,uint ADDRESS>
		void Memory<SPACE,U,V>::SourceProxy::SwapBank(dword bank) const
		{
			NST_COMPILE_ASSERT( (SPACE >= ADDRESS + SIZE) && SIZE && (SIZE % MEM_PAGE_SIZE == 0) );

			enum
			{
				MEM_OFFSET = ValueBits<SIZE>::VALUE-1,
				MEM_PAGE_BEGIN = ADDRESS / MEM_PAGE_SIZE,
				MEM_PAGE_COUNT = SIZE / MEM_PAGE_SIZE
			};

			Memory<0,0,0>::Unroller<MEM_PAGE_BEGIN,MEM_PAGE_COUNT,MEM_PAGE_SIZE>::SwapBank
			(
				&ref.pages,
				ref.sources[source].Mem(),
				ref.sources[source].Masking(),
				bank << MEM_OFFSET,
				0,
				source
			);
		}

		template<dword SPACE,uint U,uint V> template<uint SIZE>
		void Memory<SPACE,U,V>::SourceProxy::SwapBank(uint address,dword bank) const
		{
			NST_COMPILE_ASSERT( SIZE && (SIZE % MEM_PAGE_SIZE == 0) );
			NST_ASSERT( SPACE >= address + SIZE );

			enum
			{
				MEM_OFFSET = ValueBits<SIZE>::VALUE-1,
				MEM_PAGE_COUNT = SIZE / MEM_PAGE_SIZE
			};

			Memory<0,0,0>::Unroller<0,MEM_PAGE_COUNT,MEM_PAGE_SIZE>::SwapBank
			(
				&ref.pages,
				ref.sources[source].Mem(),
				ref.sources[source].Masking(),
				bank << MEM_OFFSET,
				address >> MEM_PAGE_SHIFT,
				source
			);
		}

		template<dword SPACE,uint U,uint V> template<uint SIZE,uint A,uint B>
		void Memory<SPACE,U,V>::SwapPages()
		{
			NST_COMPILE_ASSERT
			(
				(A != B) &&
				(SPACE >= A + SIZE) &&
				(SPACE >= B + SIZE) &&
				(SIZE && (SIZE % MEM_PAGE_SIZE) == 0)
			);

			enum
			{
				MEM_A_BEGIN = A / MEM_PAGE_SIZE,
				MEM_B_BEGIN = B / MEM_PAGE_SIZE,
				MEM_PAGE_COUNT = SIZE / MEM_PAGE_SIZE
			};

			for (uint i=0; i < MEM_PAGE_COUNT; ++i)
			{
				byte* const mem = pages.mem[MEM_A_BEGIN+i];
				const byte ref = pages.ref[MEM_A_BEGIN+i];

				pages.mem[MEM_A_BEGIN+i] = pages.mem[MEM_B_BEGIN+i];
				pages.ref[MEM_A_BEGIN+i] = pages.ref[MEM_B_BEGIN+i];

				pages.mem[MEM_B_BEGIN+i] = mem;
				pages.ref[MEM_B_BEGIN+i] = ref;
			}
		}

		template<dword SPACE,uint U,uint V>
		void Memory<SPACE,U,V>::SaveState(State::Saver& state,const dword baseChunk) const
		{
			byte pageData[MEM_NUM_PAGES*3];

			for (uint i=0; i < MEM_NUM_PAGES; ++i)
			{
				const uint bank = GetBank<MEM_PAGE_SIZE>( i * MEM_PAGE_SIZE );

				pageData[i*3+0] = pages.ref[i];
				pageData[i*3+1] = bank & 0xFF;
				pageData[i*3+2] = bank >> 8;
			}

			Memory<0,0,0>::SaveState( state, baseChunk, sources, NUM_SOURCES, pageData, MEM_NUM_PAGES );
		}

		template<dword SPACE,uint U,uint V>
		void Memory<SPACE,U,V>::LoadState(State::Loader& state)
		{
			byte pageData[MEM_NUM_PAGES*3];

			if (Memory<0,0,0>::LoadState( state, sources, NUM_SOURCES, pageData, MEM_NUM_PAGES ))
			{
				for (uint i=0; i < MEM_NUM_PAGES; ++i)
				{
					if (pageData[i*3+0] < NUM_SOURCES)
						Source( pageData[i*3+0] ).template SwapBank<MEM_PAGE_SIZE>( i * MEM_PAGE_SIZE, pageData[i*3+1] | uint(pageData[i*3+2]) << 8 );
					else
						throw RESULT_ERR_CORRUPT_FILE;
				}
			}
		}
	}
}

#endif
