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

#ifndef NST_CARTRIDGE_H
#define NST_CARTRIDGE_H

#include "NstRam.hpp"
#include "NstImage.hpp"
#include "NstFile.hpp"
#include "api/NstApiCartridge.hpp"

#ifdef NST_PRAGMA_ONCE
#pragma once
#endif

namespace Nes
{
	namespace Core
	{
		namespace Boards
		{
			class Board;
		}

		class Cartridge : public Image
		{
		public:

			explicit Cartridge(Context&);

			void BeginFrame(const Api::Input&,Input::Controllers*);

			typedef Api::Cartridge::Profile Profile;

			static void ReadRomset(std::istream&,FavoredSystem,bool,Profile&);
			static void ReadInes(std::istream&,FavoredSystem,Profile&);
			static void ReadUnif(std::istream&,FavoredSystem,Profile&);

			class Ines;
			class Unif;
			class Romset;

		private:

			~Cartridge();

			struct ProfileEx
			{
				ProfileEx();

				enum Nmt
				{
					NMT_DEFAULT,
					NMT_HORIZONTAL,
					NMT_VERTICAL,
					NMT_SINGLESCREEN,
					NMT_FOURSCREEN,
					NMT_CONTROLLED
				};

				Nmt nmt;
				bool battery;
				bool wramAuto;
				Ram trainer;
			};

			class VsSystem;

			static Result SetupBoard
			(
				Ram&,
				Ram&,
				Boards::Board**,
				const Context*,
				Profile&,
				const ProfileEx&,
				dword*,
				bool=false
			);

			void Reset(bool);
			bool PowerOff();
			void LoadState(State::Loader&);
			void SaveState(State::Saver&,dword) const;
			void Destroy();
			void VSync();

			uint GetDesiredController(uint) const;
			uint GetDesiredAdapter() const;
			void DetectControllers(uint);
			Region GetDesiredRegion() const;
			System GetDesiredSystem(Region,CpuModel*,PpuModel*) const;

			ExternalDevice QueryExternalDevice(ExternalDeviceType);

			Boards::Board* board;
			VsSystem* vs;
			Ram prg;
			Ram chr;
			Profile profile;
			dword prgCrc;
			File savefile;

		public:

			const Profile& GetProfile() const
			{
				return profile;
			}

			dword GetPrgCrc() const
			{
				return prgCrc;
			}
		};
	}
}

#endif
