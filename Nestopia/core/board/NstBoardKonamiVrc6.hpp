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

#ifndef NST_BOARD_KONAMI_VRC6_H
#define NST_BOARD_KONAMI_VRC6_H

#ifdef NST_PRAGMA_ONCE
#pragma once
#endif

namespace Nes
{
	namespace Core
	{
		namespace Boards
		{
			namespace Konami
			{
				class Vrc6 : public Board
				{
				public:

					explicit Vrc6(const Context&);

					class Sound : public Apu::Channel
					{
					public:

						explicit Sound(Apu&,bool=true);

						void WriteSquareReg0 (uint,uint);
						void WriteSquareReg1 (uint,uint);
						void WriteSquareReg2 (uint,uint);
						void WriteSawReg0    (uint);
						void WriteSawReg1    (uint);
						void WriteSawReg2    (uint);

						void SaveState(State::Saver&,dword) const;
						void LoadState(State::Loader&);

					protected:

						void Reset();
						bool UpdateSettings();
						Sample GetSample();

					private:

						class BaseChannel
						{
						protected:

							void Reset();

							ibool  enabled;
							uint   waveLength;
							ibool  active;
							idword timer;
							Cycle  frequency;
							uint   step;
						};

						class Square : BaseChannel
						{
						public:

							void Reset();

							NST_SINGLE_CALL dword GetSample(Cycle);
							NST_SINGLE_CALL void WriteReg0(uint);
							NST_SINGLE_CALL void WriteReg1(uint,dword);
							NST_SINGLE_CALL void WriteReg2(uint,dword);

							void SaveState(State::Saver&,dword) const;
							void LoadState(State::Loader&,uint);

							void UpdateSettings(uint);

						private:

							bool CanOutput() const;

							enum
							{
								VOLUME = OUTPUT_MUL * 2,
								MIN_FRQ = 0x04
							};

							enum
							{
								REG0_VOLUME          = 0x0F,
								REG0_DUTY            = 0x70,
								REG0_DIGITIZED       = 0x80,
								REG1_WAVELENGTH_LOW  = 0xFF,
								REG2_WAVELENGTH_HIGH = 0x0F,
								REG2_ENABLE          = 0x80,
								REG0_DUTY_SHIFT      = 4
							};

							uint duty;
							dword volume;
							ibool digitized;
						};

						class Saw : BaseChannel
						{
						public:

							void Reset();

							NST_SINGLE_CALL dword GetSample(Cycle);
							NST_SINGLE_CALL void WriteReg0(uint);
							NST_SINGLE_CALL void WriteReg1(uint,dword);
							NST_SINGLE_CALL void WriteReg2(uint,dword);

							void SaveState(State::Saver&,dword) const;
							void LoadState(State::Loader&,uint);

							void UpdateSettings(uint);

						private:

							bool CanOutput() const;

							enum
							{
								VOLUME = OUTPUT_MUL * 2,
								MIN_FRQ = 0x4,
								FRQ_SHIFT = 1
							};

							enum
							{
								REG0_PHASE           = 0x3F,
								REG1_WAVELENGTH_LOW  = 0xFF,
								REG2_WAVELENGTH_HIGH = 0x0F,
								REG2_ENABLE          = 0x80
							};

							uint phase;
							dword amp;
						};

						uint output;
						Cycle rate;
						uint fixed;
						Square square[2];
						Saw saw;
						DcBlocker dcBlocker;
					};

				private:

					void SubReset(bool);
					void SubSave(State::Saver&) const;
					void SubLoad(State::Loader&,dword);
					void Sync(Event,Input::Controllers*);

					static uint GetPrgLineShift(const Context&,uint,uint);

					NES_DECL_POKE( 9000 );
					NES_DECL_POKE( 9001 );
					NES_DECL_POKE( 9002 );
					NES_DECL_POKE( A000 );
					NES_DECL_POKE( A001 );
					NES_DECL_POKE( A002 );
					NES_DECL_POKE( B000 );
					NES_DECL_POKE( B001 );
					NES_DECL_POKE( B002 );
					NES_DECL_POKE( B003 );
					NES_DECL_POKE( F000 );
					NES_DECL_POKE( F001 );
					NES_DECL_POKE( F002 );

					Vrc4::Irq irq;
					Sound sound;
					const uint prgLineA;
					const uint prgLineB;
				};
			}
		}
	}
}

#endif
