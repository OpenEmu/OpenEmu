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

#ifndef NST_MACHINE_H
#define NST_MACHINE_H

#include <iosfwd>
#include "NstCpu.hpp"
#include "NstPpu.hpp"
#include "NstTracker.hpp"
#include "NstVideoRenderer.hpp"

#ifdef NST_PRAGMA_ONCE
#pragma once
#endif

namespace Nes
{
	namespace Core
	{
		namespace Video
		{
			class Output;
		}

		namespace Sound
		{
			class Output;
		}

		namespace Input
		{
			class Device;
			class Adapter;
			class Controllers;
		}

		class Image;
		class Cheats;
		class ImageDatabase;

		class Machine
		{
		public:

			Machine();
			~Machine();

			void Execute
			(
				Video::Output*,
				Sound::Output*,
				Input::Controllers*
			);

			enum ColorMode
			{
				COLORMODE_YUV,
				COLORMODE_RGB,
				COLORMODE_CUSTOM
			};

			Result Load
			(
				std::istream&,
				FavoredSystem,
				bool,
				std::istream*,
				bool,
				Result*,
				uint
			);

			Result Unload();
			Result PowerOff(Result=RESULT_OK);
			void   Reset(bool);
			void   SwitchMode();
			bool   LoadState(State::Loader&,bool);
			void   SaveState(State::Saver&) const;
			void   InitializeInputDevices() const;
			Result UpdateColorMode();
			Result UpdateColorMode(ColorMode);

		private:

			void UpdateModels();

			enum
			{
				OPEN_BUS = 0x40
			};

			NES_DECL_POKE( 4016 );
			NES_DECL_PEEK( 4016 );
			NES_DECL_POKE( 4017 );
			NES_DECL_PEEK( 4017 );

			uint state;
			dword frame;

		public:

			Input::Adapter* extPort;
			Input::Device* expPort;
			Image* image;
			Cheats* cheats;
			ImageDatabase* imageDatabase;
			Tracker tracker;
			Cpu cpu;
			Ppu ppu;
			Video::Renderer renderer;

			uint Is(uint a) const
			{
				return state & a;
			}

			bool Is(uint a,uint b) const
			{
				return (state & a) && (state & b);
			}
		};
	}
}

#endif
