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

#ifndef NST_BOARD_WAIXING_H
#define NST_BOARD_WAIXING_H

#ifdef NST_PRAGMA_ONCE
#pragma once
#endif

#include "NstBoardMmc3.hpp"
#include "NstBoardWaixingPs2.hpp"
#include "NstBoardWaixingFfv.hpp"
#include "NstBoardWaixingSh2.hpp"
#include "NstBoardWaixingZs.hpp"
#include "NstBoardWaixingSecurity.hpp"
#include "NstBoardWaixingSgz.hpp"
#include "NstBoardWaixingSgzlz.hpp"

namespace Nes
{
	namespace Core
	{
		namespace Boards
		{
			namespace Waixing
			{
				class TypeI : public Mmc3
				{
				public:

					explicit TypeI(const Context& c)
					: Mmc3(c) {}

				protected:

					void SubReset(bool);
					uint GetChrIndex(uint) const;
					uint GetPrgIndex(uint) const;

				private:

					NES_DECL_PEEK( 5000 );
					NES_DECL_POKE( 5000 );
				};

				class TypeA : public TypeI
				{
				public:

					explicit TypeA(const Context& c)
					: TypeI(c) {}

				protected:

					void SubReset(bool);

				private:

					virtual uint NST_FASTCALL GetChrSource(uint) const;
					void NST_FASTCALL UpdateChr(uint,uint) const;
				};

				class TypeB : public TypeA
				{
				public:

					explicit TypeB(const Context& c)
					: TypeA(c) {}

				private:

					uint NST_FASTCALL GetChrSource(uint) const;
				};

				class TypeC : public TypeA
				{
				public:

					explicit TypeC(const Context& c)
					: TypeA(c) {}

				private:

					uint NST_FASTCALL GetChrSource(uint) const;
				};

				class TypeD : public TypeA
				{
				public:

					explicit TypeD(const Context& c)
					: TypeA(c) {}

				private:

					uint NST_FASTCALL GetChrSource(uint) const;
				};

				class TypeE : public TypeA
				{
				public:

					explicit TypeE(const Context& c)
					: TypeA(c) {}

				private:

					uint NST_FASTCALL GetChrSource(uint) const;
				};

				class TypeF : public TypeA
				{
				public:

					explicit TypeF(const Context& c)
					: TypeA(c) {}

				private:

					void SubReset(bool);
					void SubLoad(State::Loader&,dword);

					uint NST_FASTCALL GetChrSource(uint) const;
					void NST_FASTCALL UpdatePrg(uint,uint);

					NES_DECL_POKE( 8001 );

					byte exPrg[4];
				};

				class TypeG : public TypeA
				{
				public:

					explicit TypeG(const Context& c)
					: TypeA(c) {}

				private:

					void SubReset(bool);
					void SubLoad(State::Loader&,dword);

					void NST_FASTCALL UpdatePrg(uint,uint);
					void NST_FASTCALL UpdateChr(uint,uint) const;

					NES_DECL_POKE( 8001 );

					byte exPrg[4];
					byte exChr[8];
				};

				class TypeH : public Mmc3
				{
				public:

					explicit TypeH(const Context& c)
					: Mmc3(c) {}

				private:

					void SubReset(bool);
					void SubLoad(State::Loader&,dword);

					void NST_FASTCALL UpdatePrg(uint,uint);
					void NST_FASTCALL UpdateChr(uint,uint) const;

					NES_DECL_POKE( 8001 );

					uint exPrg;
				};

				class TypeJ : public TypeI
				{
				public:

					explicit TypeJ(const Context& c)
					: TypeI(c) {}

				private:

					void SubReset(bool);
					void SubLoad(State::Loader&,dword);

					void NST_FASTCALL UpdatePrg(uint,uint);

					NES_DECL_POKE( 8001 );

					byte exPrg[4];
				};
			}
		}
	}
}

#endif
