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

#ifndef NST_BOARD_NAMCOT_163_H
#define NST_BOARD_NAMCOT_163_H

#ifdef NST_PRAGMA_ONCE
#pragma once
#endif

namespace Nes
{
	namespace Core
	{
		namespace Boards
		{
			namespace Namcot
			{
				class N163 : public Board
				{
				public:

					explicit N163(const Context&);

					class Sound : public Apu::Channel
					{
					public:

						explicit Sound(Apu&,bool=true);

						enum
						{
							EXRAM_SIZE = 0x80
						};

						void WriteData(uint);
						uint ReadData();
						void WriteAddress(uint);

						byte* GetExRam();
						const byte* GetExRam() const;

						void SaveState(State::Saver&,dword) const;
						void LoadState(State::Loader&);

					protected:

						void Reset();
						bool UpdateSettings();
						Sample GetSample();

					private:

						inline void SetChannelState(uint);

						inline void WriteWave(uint);
						inline dword FetchFrequency(uint) const;

						enum
						{
							NUM_CHANNELS     = 8,
							EXRAM_INC        = 0x80,
							REG_WAVELENGTH   = 0x1C,
							REG_ENABLE_SHIFT = 5,
							REG_VOLUME       = 0x0F,
							PHASE_SHIFT      = 18,
							SPEED_SHIFT      = 20
						};

						class BaseChannel
						{
						public:

							void Reset();

							inline dword GetSample(Cycle,Cycle,const byte (&)[0x100]);

							inline void SetFrequency  (uint);
							inline void SetWaveLength (uint);
							inline void SetWaveOffset (uint);
							inline void SetVolume     (uint);

							inline void Validate();

						private:

							enum
							{
								VOLUME = OUTPUT_MUL / 16
							};

							inline bool CanOutput() const;

							ibool enabled;
							ibool active;
							Cycle timer;
							Cycle frequency;
							Cycle phase;
							dword waveLength;
							uint  waveOffset;
							uint  volume;
						};

						uint output;

						Cycle rate;
						Cycle frequency;

						uint exAddress;
						uint exIncrease;
						uint startChannel;

						byte wave[0x100];
						byte exRam[0x80];

						BaseChannel channels[NUM_CHANNELS];

						DcBlocker dcBlocker;
					};

				private:

					void SubReset(bool);
					void SubSave(State::Saver&) const;
					void SubLoad(State::Loader&,dword);
					void Load(File&);
					void Save(File&) const;
					void Sync(Event,Input::Controllers*);
					void SwapChr(uint,uint,uint) const;
					void SwapNmt(uint,uint) const;

					struct Irq
					{
						void Reset(bool);
						bool Clock();

						uint count;
					};

					NES_DECL_PEEK( 4800 );
					NES_DECL_POKE( 4800 );
					NES_DECL_PEEK( 5000 );
					NES_DECL_POKE( 5000 );
					NES_DECL_PEEK( 5800 );
					NES_DECL_POKE( 5800 );
					NES_DECL_POKE( C000 );
					NES_DECL_POKE( C800 );
					NES_DECL_POKE( D000 );
					NES_DECL_POKE( D800 );
					NES_DECL_POKE( F800 );

					ClockUnits::M2<Irq> irq;
					Sound sound;
				};
			}
		}
	}
}

#endif
