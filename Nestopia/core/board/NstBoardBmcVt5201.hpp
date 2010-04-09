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

#ifndef NST_BOARD_BMC_VT5201_H
#define NST_BOARD_BMC_VT5201_H

#ifdef NST_PRAGMA_ONCE
#pragma once
#endif

namespace Nes
{
	namespace Core
	{
		namespace Boards
		{
			namespace Bmc
			{
				class Vt5201 : public Board
				{
				public:

					explicit Vt5201(const Context&);

				private:

					class CartSwitches : public DipSwitches
					{
						enum Type
						{
							VT5201_XIN1  = 0,
							VT5201_6IN1  = 0x766130C4,
							VT5201_12IN1 = 0xBA6A6F73,
							VT5201_28IN1 = 0x7A423007,
							VT5201_54IN1 = 0x2B81E99F,
							VT5201_55IN1 = 0x4978BA70,
							VT5201_65IN1 = 0x487F8A54
						};

						uint mode;
						const Type type;

					public:

						explicit CartSwitches(const Context&);

						void SetMode(uint);
						uint GetMode() const;

					private:

						static Type DetectType(const Context&);

						uint GetValue(uint) const;
						void SetValue(uint,uint);
						uint NumDips() const;
						uint NumValues(uint) const;
						cstring GetDipName(uint) const;
						cstring GetValueName(uint,uint) const;
					};

					void SubReset(bool);
					void SubSave(State::Saver&) const;
					void SubLoad(State::Loader&,dword);
					Device QueryDevice(DeviceType);

					NES_DECL_PEEK( 8000 );
					NES_DECL_POKE( 8000 );

					uint cartMode;
					CartSwitches cartSwitches;
				};
			}
		}
	}
}

#endif
