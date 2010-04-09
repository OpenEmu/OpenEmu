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

#ifndef NST_VSSYSTEM_H
#define NST_VSSYSTEM_H

#include "../api/NstApiInput.hpp"
#include "../NstDipSwitches.hpp"
#include "../NstCartridge.hpp"

#ifdef NST_PRAGMA_ONCE
#pragma once
#endif

namespace Nes
{
	namespace Core
	{
		class Cartridge::VsSystem
		{
		public:

			enum Mode
			{
				MODE_STD,
				MODE_RBI,
				MODE_TKO,
				MODE_XEV
			};

			static VsSystem* Create(Cpu&,Ppu&,PpuModel,dword);
			static void Destroy(VsSystem*);

			void Reset(bool);
			void SaveState(State::Saver&,dword) const;
			void LoadState(State::Loader&);

		protected:

			struct Context;

			explicit VsSystem(Context&);
			virtual ~VsSystem();

		private:

			class RbiBaseball;
			class SuperXevious;
			class TkoBoxing;

			virtual void Reset() {}
			virtual void SubSave(State::Saver&) const {}
			virtual void SubLoad(State::Loader&,dword) {}

			class Dip;

			class VsDipSwitches : public DipSwitches
			{
			public:

				VsDipSwitches(Dip*&,uint);
				~VsDipSwitches();

				inline uint Reg(uint) const;
				inline void Reset();

				void BeginFrame(Input::Controllers*);

			private:

				uint NumDips() const;
				uint NumValues(uint) const;
				cstring GetDipName(uint) const;
				cstring GetValueName(uint,uint) const;
				uint GetValue(uint) const;
				void SetValue(uint,uint);

				uint coinTimer;
				Dip* const table;
				const uint size;
				uint regs[2];
			};

			enum
			{
				DIPSWITCH_4016_MASK  = 0x03,
				DIPSWITCH_4016_SHIFT = 3,
				DIPSWITCH_4017_MASK  = 0xFC,
				DIPSWITCH_4017_SHIFT = 0,
				COIN_1               = Input::Controllers::VsSystem::COIN_1,
				COIN_2               = Input::Controllers::VsSystem::COIN_2,
				COIN                 = COIN_1|COIN_2,
				STATUS_4016_MASK     = uint(DIPSWITCH_4016_MASK) << DIPSWITCH_4016_SHIFT | COIN,
				STATUS_4017_MASK     = uint(DIPSWITCH_4017_MASK) << DIPSWITCH_4017_SHIFT
			};

			NES_DECL_PEEK( Nop  );
			NES_DECL_POKE( Nop  );
			NES_DECL_PEEK( 4016 );
			NES_DECL_POKE( 4016 );
			NES_DECL_PEEK( 4017 );
			NES_DECL_POKE( 4017 );
			NES_DECL_PEEK( 4020 );
			NES_DECL_POKE( 4020 );

		protected:

			Cpu& cpu;
			Ppu& ppu;

		private:

			class InputMapper
			{
				typedef Input::Controllers::Pad Pad;

				virtual void Fix(Pad (&)[4],const uint (&)[2]) const = 0;

				void* userData;
				Pad::PollCallback userCallback;

				struct Type1;
				struct Type2;
				struct Type3;
				struct Type4;
				struct Type5;

			public:

				enum Type
				{
					TYPE_NONE,
					TYPE_1,
					TYPE_2,
					TYPE_3,
					TYPE_4,
					TYPE_5
				};

				static InputMapper* Create(Type);
				virtual ~InputMapper() {}

				void Begin(const Api::Input,Input::Controllers*);
				void End() const;
			};

			InputMapper* const inputMapper;
			Io::Port p4016;
			Io::Port p4017;
			VsDipSwitches dips;
			uint coin;
			const PpuModel ppuModel;

		public:

			void BeginFrame(const Api::Input& input,Input::Controllers* controllers)
			{
				dips.BeginFrame( controllers );

				if (inputMapper)
					inputMapper->Begin( input, controllers );
			}

			void VSync() const
			{
				if (inputMapper)
					inputMapper->End();
			}

			PpuModel GetPpuModel() const
			{
				return ppuModel;
			}

			DipSwitches& GetDipSwiches()
			{
				return dips;
			}
		};
	}
}

#endif
