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

#ifndef NST_BOARD_SUNSOFT_5B_H
#define NST_BOARD_SUNSOFT_5B_H

#ifdef NST_PRAGMA_ONCE
#pragma once
#endif

namespace Nes
{
	namespace Core
	{
		namespace Boards
		{
			namespace Sunsoft
			{
				class S5b : public Fme7
				{
				public:

					explicit S5b(const Context&);

					class Sound : public Apu::Channel
					{
					public:

						explicit Sound(Apu&,bool=true);

						void WriteReg(uint);

						void LoadState(State::Loader&);
						void SaveState(State::Saver&,dword) const;

					protected:

						void Reset();
						bool UpdateSettings();
						Sample GetSample();

					private:

						enum
						{
							NUM_SQUARES = 3
						};

						class Envelope
						{
						public:

							void Reset(uint);
							void UpdateSettings(uint);
							void SaveState(State::Saver&,dword) const;
							void LoadState(State::Loader&,uint);

							void WriteReg0(uint,uint);
							void WriteReg1(uint,uint);
							void WriteReg2(uint);

							NST_SINGLE_CALL dword Clock(Cycle);

						private:

							void UpdateFrequency(uint);

							byte   holding;
							byte   hold;
							byte   alternate;
							byte   attack;
							idword timer;
							dword  frequency;
							uint   count;
							uint   volume;
							uint   length;
						};

						class Noise
						{
						public:

							void Reset(uint);
							void UpdateSettings(uint);
							void SaveState(State::Saver&,dword) const;
							void LoadState(State::Loader&,uint);

							void WriteReg(uint,uint);

							NST_SINGLE_CALL dword Clock(Cycle);

						private:

							void UpdateFrequency(uint);

							idword timer;
							dword frequency;
							dword rng;
							dword dc;
							uint length;
						};

						class Square
						{
						public:

							void Reset(uint);
							void UpdateSettings(uint);
							void SaveState(State::Saver&,dword) const;
							void LoadState(State::Loader&,uint);

							void WriteReg0(uint,uint);
							void WriteReg1(uint,uint);
							void WriteReg2(uint);
							void WriteReg3(uint);

							NST_SINGLE_CALL dword GetSample(Cycle,uint,uint);

						private:

							void UpdateFrequency(uint);

							idword timer;
							dword frequency;
							uint status;
							uint ctrl;
							uint volume;
							dword dc;
							uint length;
						};

						ibool active;
						uint output;
						Cycle rate;
						uint fixed;
						uint regSelect;
						Envelope envelope;
						Noise noise;
						Square squares[NUM_SQUARES];
						DcBlocker dcBlocker;

						static const word levels[32];

					public:

						void SelectReg(uint data)
						{
							regSelect = data;
						}
					};

				private:

					void SubReset(bool);
					void SubSave(State::Saver&) const;
					void SubLoad(State::Loader&,dword);

					NES_DECL_POKE( C000  );
					NES_DECL_POKE( E000  );

					Sound sound;
				};
			}
		}
	}
}

#endif
