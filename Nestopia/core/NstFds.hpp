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

#ifndef NST_FDS_H
#define NST_FDS_H

#include "NstImage.hpp"
#include "NstClock.hpp"
#include "NstFile.hpp"
#include "NstChecksum.hpp"
#include "api/NstApiFds.hpp"

#ifdef NST_PRAGMA_ONCE
#pragma once
#endif

namespace Nes
{
	namespace Core
	{
		class Ppu;

		class Fds : public Image
		{
		public:

			explicit Fds(Context&);

			Result InsertDisk(uint,uint);
			Result EjectDisk();
			Result GetDiskData(uint,Api::Fds::DiskData&) const;

			static void SetBios(std::istream*);
			static Result GetBios(std::ostream&);
			static bool HasBios();

			class Sound : public Apu::Channel
			{
			public:

				explicit Sound(Apu&,bool=true);

				uint ReadWaveData(uint) const;
				void WriteWaveData(uint,uint);

				void WriteReg0(uint);
				void WriteReg1(uint);
				void WriteReg2(uint);
				void WriteReg3(uint);
				void WriteReg4(uint);
				void WriteReg5(uint);
				void WriteReg6(uint);
				void WriteReg7(uint);
				void WriteReg8(uint);
				void WriteReg9(uint);

				uint ReadVolumeGain() const;
				uint ReadSweepGain() const;

				void SaveState(State::Saver&,dword) const;
				void LoadState(State::Loader&);

			protected:

				void Reset();
				bool UpdateSettings();
				Sample GetSample();
				Cycle Clock(Cycle,Cycle,Cycle);

			private:

				bool CanOutput() const;
				inline bool CanModulate() const;

				NST_SINGLE_CALL dword GetModulation() const;

				enum
				{
					REG2_WAVELENGTH_LOW      = 0xFF,
					REG3_WAVELENGTH_HIGH     = 0x0F,
					REG3_ENVELOPE_DISABLE    = 0x40,
					REG3_OUTPUT_DISABLE      = 0x80,
					REG5_MOD_SWEEP           = 0x3F,
					REG5_MOD_NEGATE          = 0x40,
					REG6_MOD_WAVELENGTH_LOW  = 0xFF,
					REG7_MOD_WAVELENGTH_HIGH = 0x0F,
					REG7_MOD_WRITE_MODE      = 0x80,
					REG8_MOD_DATA            = 0x07,
					REG9_VOLUME              = 0x03,
					REG9_WRITE_MODE          = 0x80
				};

				enum
				{
					STATUS_OUTPUT_ENABLED    = REG3_OUTPUT_DISABLE,
					STATUS_ENVELOPES_ENABLED = REG3_ENVELOPE_DISABLE
				};

				class Envelope
				{
				public:

					void Reset();
					void Write(uint);

					NST_SINGLE_CALL void Clock();

					inline uint Gain() const;
					inline uint Output() const;

					void SaveState(State::Saver&,dword) const;
					void LoadState(State::Loader&);

					enum
					{
						CTRL_COUNT   = 0x3F,
						CTRL_UP      = 0x40,
						CTRL_DISABLE = 0x80
					};

				private:

					enum
					{
						GAIN_MAX = 0x20,
						GAIN_MIN = 0x00
					};

					byte counter;
					byte ctrl;
					byte gain;
					byte output;
				};

				struct Modulator
				{
					enum
					{
						TIMER_CARRY = 0x80000000,
						SIZE = 0x20
					};

					bool active;
					bool writing;
					byte sweep;
					byte pos;
					uint length;
					dword rate;
					dword timer;
					dword clock;
					byte table[SIZE];

					static const byte steps[8];
				};

				enum
				{
					VOLUME,
					SWEEP
				};

				struct Envelopes
				{
					enum {PULSE = 8};

					byte counter;
					byte length;
					word clock;
					Envelope units[2];
				};

				struct Wave
				{
					enum {SIZE = 0x40};

					word length;
					byte volume;
					bool writing;
					dword pos;
					dword rate;
					dword frame;
					dword clock;
					byte table[SIZE];
				};

				ibool active;

				Wave wave;
				Envelopes envelopes;
				Modulator modulator;

				uint volume;
				dword amp;
				uint output;
				uint status;
				DcBlocker dcBlocker;

				static const byte volumes[4];
			};

		private:

			~Fds();

			void Reset(bool);
			void VSync();
			uint GetDesiredController(uint) const;
			uint GetDesiredAdapter() const;
			Region GetDesiredRegion() const;
			System GetDesiredSystem(Region,CpuModel*,PpuModel*) const;
			void LoadState(State::Loader&);
			void SaveState(State::Saver&,dword) const;
			bool PowerOff();

			NES_DECL_PEEK( Nop  );
			NES_DECL_POKE( Nop  );
			NES_DECL_POKE( 4023 );
			NES_DECL_POKE( 4025 );
			NES_DECL_POKE( 4026 );
			NES_DECL_PEEK( 4031 );
			NES_DECL_PEEK( 4033 );
			NES_DECL_PEEK( 4040 );
			NES_DECL_POKE( 4040 );
			NES_DECL_POKE( 4080 );
			NES_DECL_POKE( 4082 );
			NES_DECL_POKE( 4083 );
			NES_DECL_POKE( 4084 );
			NES_DECL_POKE( 4085 );
			NES_DECL_POKE( 4086 );
			NES_DECL_POKE( 4087 );
			NES_DECL_POKE( 4088 );
			NES_DECL_POKE( 4089 );
			NES_DECL_POKE( 408A );
			NES_DECL_PEEK( 4090 );
			NES_DECL_PEEK( 4092 );

			enum
			{
				SIDE_SIZE            = 65500,
				MAX_SIDE_SIZE        = 68000,
				CTRL1_NMT_HORIZONTAL = 0x08,
				OPEN_BUS             = 0x40,
				DOREMIKKO_ID         = 0xA4445245
			};

			struct Disks
			{
				explicit Disks(std::istream&);

				enum
				{
					EJECTED  = 0xFFF,
					MOUNTING = 180
				};

				class Sides
				{
				public:

					explicit Sides(std::istream&);
					~Sides();

					inline byte* operator [] (uint) const;
					void Save() const;

					uint count;

				private:

					enum
					{
						HEADER_SIZE = 16
					};

					byte* data;
					File file;

				public:

					bool HasHeader() const
					{
						return data[-HEADER_SIZE];
					}
				};

				const Sides sides;
				const dword crc;
				const dword id;
				word current;
				byte mounting;
				bool writeProtected;
			};

			struct Ram
			{
				void Reset();

				NES_DECL_PEEK( Ram );
				NES_DECL_POKE( Ram );

				byte mem[SIZE_32K];
			};

			struct Unit
			{
				explicit Unit(const Disks::Sides&);

				void Reset(bool);
				ibool Clock();

				enum
				{
					STATUS_PENDING_IRQ = 0x1,
					STATUS_TRANSFERED  = 0x2
				};

				struct Timer
				{
					Timer();

					void Reset();
					void Advance(uint&);

					NST_SINGLE_CALL bool Clock();

					enum
					{
						CTRL_REPEAT  = 0x1,
						CTRL_ENABLED = 0x2
					};

					uint ctrl;
					word count;
					word latch;
				};

				struct Drive
				{
					explicit Drive(const Disks::Sides&);

					static Result Analyze(const byte*,Api::Fds::DiskData&);

					void  Reset();
					void  Mount(byte*,bool);
					ibool Advance(uint&);

					NST_SINGLE_CALL bool Clock();
					NST_SINGLE_CALL void Write(uint);

					enum
					{
						CLK_HEAD = 96400,

						BYTES_GAP_INIT = CLK_HEAD/8UL * 398 / 1000,
						BYTES_GAP_NEXT = CLK_HEAD/8UL * 10  / 1000,

						CLK_BYTE = CLK_NTSC / (CLK_HEAD/8UL * CPU_RP2A03_CC * CLK_NTSC_DIV),

						CLK_MOTOR  = CLK_HEAD/8UL * 100 * CLK_BYTE / 1000,
						CLK_REWIND = CLK_HEAD/8UL * 135 * CLK_BYTE / 1000,

						CTRL_ON        = 0x01,
						CTRL_STOP      = 0x02,
						CTRL_READ_MODE = 0x04,
						CTRL_CRC       = 0x10,
						CTRL_IO_MODE   = 0x40,
						CTRL_GEN_IRQ   = 0x80,

						STATUS_EJECTED   = 0x01,
						STATUS_UNREADY   = 0x02,
						STATUS_PROTECTED = 0x04,

						BLOCK_VOLUME = 1,
						BLOCK_COUNT,
						BLOCK_HEADER,
						BLOCK_DATA,

						LENGTH_HEADER  = 15,
						LENGTH_VOLUME  = 55,
						LENGTH_COUNT   = 1,
						LENGTH_UNKNOWN = 0xFFFF
					};

					dword count;
					dword headPos;
					byte* io;
					word dataPos;
					word gap;
					word length;
					word in;
					byte out;
					byte ctrl;
					byte status;
					const Disks::Sides& sides;
				};

				Timer timer;
				Drive drive;
				uint status;
			};

			class Adapter : ClockUnits::M2<Unit>
			{
				NES_DECL_PEEK( Nop  );
				NES_DECL_POKE( Nop  );
				NES_DECL_POKE( 4020 );
				NES_DECL_POKE( 4021 );
				NES_DECL_POKE( 4022 );
				NES_DECL_POKE( 4024 );
				NES_DECL_PEEK( 4030 );
				NES_DECL_PEEK( 4032 );

			public:

				Adapter(Cpu&,const Disks::Sides&);

				void Reset(Cpu&,byte*,bool=false);
				void LoadState(State::Loader&,dword,Ppu&);
				void SaveState(State::Saver&) const;

				inline void Mount(byte*,bool=false);

				NST_SINGLE_CALL void Write(uint);
				NST_SINGLE_CALL uint Read();
				NST_SINGLE_CALL void WriteProtect();
				NST_SINGLE_CALL uint Activity() const;

				using ClockUnits::M2<Unit>::VSync;
			};

			struct Io
			{
				Io();

				void Reset();

				enum
				{
					CTRL0_DISK_ENABLED = 0x01,
					BATTERY_CHARGED    = 0x80
				};

				byte ctrl;
				byte port;
				mutable word led;
			};

			Disks disks;
			Adapter adapter;
			Io io;
			Cpu& cpu;
			Ppu& ppu;
			Ram ram;
			Sound sound;
			mutable Checksum checksum;

			class Bios;
			static Bios bios;

		public:

			bool IsAnyDiskInserted() const
			{
				return disks.current != Disks::EJECTED;
			}

			int CurrentDisk() const
			{
				return disks.current != Disks::EJECTED ? int(disks.current / 2U) : -1;
			}

			int CurrentDiskSide() const
			{
				return disks.current != Disks::EJECTED ? int(disks.current % 2U) : -1;
			}

			uint NumSides() const
			{
				return disks.sides.count;
			}

			uint NumDisks() const
			{
				return (disks.sides.count / 2U) + (disks.sides.count % 2U);
			}

			dword GetPrgCrc() const
			{
				return disks.crc;
			}

			bool CanChangeDiskSide() const
			{
				return disks.current != Disks::EJECTED && (disks.current | 1U) < disks.sides.count;
			}

			bool HasHeader() const
			{
				return disks.sides.HasHeader();
			}
		};
	}
}

#endif
