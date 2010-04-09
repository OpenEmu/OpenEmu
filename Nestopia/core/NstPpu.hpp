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

#ifndef NST_PPU_H
#define NST_PPU_H

#ifndef NST_IO_PORT_H
#include "NstIoPort.hpp"
#endif

#include "NstIoAccessor.hpp"
#include "NstIoLine.hpp"
#include "NstHook.hpp"
#include "NstMemory.hpp"
#include "NstVideoScreen.hpp"

#ifdef NST_PRAGMA_ONCE
#pragma once
#endif

namespace Nes
{
	namespace Core
	{
		class Ppu
		{
		public:

			explicit Ppu(Cpu&);

			void Reset(bool,bool);
			void PowerOff();
			void BeginFrame(bool);
			void EndFrame();

			enum NmtMirroring
			{
				NMT_H = 0xC,
				NMT_V = 0xA,
				NMT_0 = 0x0,
				NMT_1 = 0xF
			};

			void SetModel(PpuModel,bool);
			void SetMirroring(NmtMirroring);
			void SetMirroring(const byte (&)[4]);
			void SetHActiveHook(const Hook&);
			void SetHBlankHook(const Hook&);
			uint GetPixelCycles() const;

			void EnableCpuSynchronization();

			void LoadState(State::Loader&);
			void SaveState(State::Saver&,dword) const;

			class ChrMem : public Memory<SIZE_8K,SIZE_1K,2>
			{
				NES_DECL_ACCESSOR( Pattern );

			protected:

				Io::Accessor accessors[2];

			public:

				void ResetAccessors();
				void SetDefaultAccessor(uint);

				template<typename T,typename U>
				void SetAccessor(uint i,T t,U u)
				{
					NST_ASSERT( i < 2 );
					accessors[i].Set( t, u );
				}

				template<typename T,typename U,typename V>
				void SetAccessors(T t,U u,V v)
				{
					accessors[0].Set( t, u );
					accessors[1].Set( t, v );
				}
			};

			class NmtMem : public Memory<SIZE_4K,SIZE_1K,2>
			{
				NES_DECL_ACCESSOR( Name_2000 );
				NES_DECL_ACCESSOR( Name_2400 );
				NES_DECL_ACCESSOR( Name_2800 );
				NES_DECL_ACCESSOR( Name_2C00 );

			protected:

				Io::Accessor accessors[4][2];

			public:

				void ResetAccessors();
				void SetDefaultAccessors(uint);
				void SetDefaultAccessor(uint,uint);

				template<typename T,typename U>
				void SetAccessor(uint i,uint j,T t,U u)
				{
					NST_ASSERT( i < 4 && j < 2 );
					accessors[i][j].Set( t, u );
				}

				template<typename T,typename U>
				void SetAccessors(uint i,T t,U u)
				{
					NST_ASSERT( i < 4 );
					accessors[i][0].Set( t, u[0] );
					accessors[i][1].Set( t, u[1] );
				}

				template<typename T,typename U,typename V,typename W,typename X>
				void SetAccessors(T t,U u,V v,W w,X x)
				{
					accessors[0][0].Set( t, u[0] );
					accessors[0][1].Set( t, u[1] );
					accessors[1][0].Set( t, v[0] );
					accessors[1][1].Set( t, v[1] );
					accessors[2][0].Set( t, w[0] );
					accessors[2][1].Set( t, w[1] );
					accessors[3][0].Set( t, x[0] );
					accessors[3][1].Set( t, x[1] );
				}
			};

		private:

			struct Chr : ChrMem
			{
				NST_FORCE_INLINE uint FetchPattern(uint) const;
			};

			struct Nmt : NmtMem
			{
				NST_FORCE_INLINE uint FetchName(uint) const;
				NST_FORCE_INLINE uint FetchAttribute(uint) const;
			};

			enum
			{
				SCANLINE_HDUMMY = -1,
				SCANLINE_VBLANK = 255
			};

			enum
			{
				HCLOCK_DUMMY    = 341,
				HCLOCK_VBLANK_0 = 681,
				HCLOCK_VBLANK_1 = 682,
				HCLOCK_VBLANK_2 = 684,
				HCLOCK_BOOT     = 685
			};

			NES_DECL_POKE( 2000 );
			NES_DECL_PEEK( 2002 );
			NES_DECL_PEEK( 2002_RC2C05_01_04 );
			NES_DECL_PEEK( 2002_RC2C05_02 );
			NES_DECL_PEEK( 2002_RC2C05_03 );
			NES_DECL_POKE( 2001 );
			NES_DECL_POKE( 2003 );
			NES_DECL_PEEK( 2004 );
			NES_DECL_POKE( 2004 );
			NES_DECL_POKE( 2005 );
			NES_DECL_POKE( 2006 );
			NES_DECL_PEEK( 2007 );
			NES_DECL_POKE( 2007 );
			NES_DECL_PEEK( 2xxx );
			NES_DECL_POKE( 2xxx );
			NES_DECL_PEEK( 3000 );
			NES_DECL_PEEK( 4014 );
			NES_DECL_POKE( 4014 );

			NES_DECL_HOOK( Sync );
			NES_DECL_HOOK( Nop  );

			inline Cycle GetCycles() const;
			inline Cycle GetLocalCycles(Cycle) const;

			inline bool IsDead() const;
			inline void UpdateScrollAddress(uint);
			inline uint Coloring() const;
			inline uint Emphasis() const;

			NST_FORCE_INLINE uint FetchName() const;
			NST_FORCE_INLINE uint FetchAttribute() const;

			NST_FORCE_INLINE void EvaluateSpritesEven();
			NST_FORCE_INLINE void EvaluateSpritesOdd();

			void EvaluateSpritesPhase0();
			void EvaluateSpritesPhase1();
			void EvaluateSpritesPhase2();
			void EvaluateSpritesPhase3();
			void EvaluateSpritesPhase4();
			void EvaluateSpritesPhase5();
			void EvaluateSpritesPhase6();
			void EvaluateSpritesPhase7();
			void EvaluateSpritesPhase8();
			void EvaluateSpritesPhase9();

			void Reset(bool,bool,bool);
			void Update(Cycle);
			void UpdateStates();
			void UpdatePalette();
			void LoadSprite(const byte* NST_RESTRICT,uint=0x1000);

			NST_SINGLE_CALL void PreLoadTiles();
			NST_SINGLE_CALL void LoadTiles();
			NST_FORCE_INLINE void RenderPixel();
			NST_SINGLE_CALL void RenderPixel255();
			NST_NO_INLINE void Run();

			struct Regs
			{
				enum
				{
					CTRL0_NAME_OFFSET        = 0x03,
					CTRL0_INC32              = 0x04,
					CTRL0_SP_OFFSET          = 0x08,
					CTRL0_BG_OFFSET          = 0x10,
					CTRL0_SP8X16             = 0x20,
					CTRL0_NMI                = 0x80,
					CTRL0_NMI_OCCUR          = 0x100,
					CTRL1_MONOCHROME         = 0x01,
					CTRL1_BG_NO_CLIP         = 0x02,
					CTRL1_SP_NO_CLIP         = 0x04,
					CTRL1_BG_ENABLED         = 0x08,
					CTRL1_SP_ENABLED         = 0x10,
					CTRL1_BG_ENABLED_NO_CLIP = CTRL1_BG_ENABLED|CTRL1_BG_NO_CLIP,
					CTRL1_SP_ENABLED_NO_CLIP = CTRL1_SP_ENABLED|CTRL1_SP_NO_CLIP,
					CTRL1_BG_SP_ENABLED      = CTRL1_BG_ENABLED|CTRL1_SP_ENABLED,
					CTRL1_EMPHASIS           = 0xE0,
					STATUS_LATCH             = 0x1F,
					STATUS_SP_OVERFLOW       = 0x20,
					STATUS_SP_ZERO_HIT       = 0x40,
					STATUS_VBLANK            = 0x80,
					STATUS_BITS              = STATUS_SP_OVERFLOW|STATUS_SP_ZERO_HIT|STATUS_VBLANK,
					STATUS_VBLANKING         = 0x100,
					FRAME_ODD                = CTRL1_BG_ENABLED|CTRL1_SP_ENABLED
				};

				uint ctrl0;
				uint ctrl1;
				uint status;
				uint frame;
				uint oam;
			};

			struct Scroll
			{
				enum
				{
					X_TILE    = 0x001F,
					Y_TILE    = 0x03E0,
					Y_FINE    = 0x7000,
					LOW       = 0x00FF,
					HIGH      = 0xFF00,
					NAME      = 0x0C00,
					NAME_LOW  = 0x0400,
					NAME_HIGH = 0x0800
				};

				NST_FORCE_INLINE void ClockX();
				NST_SINGLE_CALL  void ResetX();
				NST_SINGLE_CALL  void ClockY();

				uint address;
				uint toggle;
				uint latch;
				uint increase;
				uint xFine;
				uint pattern;
			};

			struct Tiles
			{
				Tiles();

				byte pattern[2];
				byte attribute;
				byte index;
				byte pixels[16];
				uint mask;
				byte show[2];
				const byte padding0;
				const byte padding1;
			};

			struct Palette
			{
				enum
				{
					SIZE          = 0x20,
					COLORS        = 0x40,
					SPRITE_OFFSET = 0x10,
					COLOR         = 0x3F,
					MONO          = 0x30
				};

				byte ram[SIZE];
			};

			struct Output
			{
				explicit Output(Video::Screen::Pixel*);

				Video::Screen::Pixel* target;
				Video::Screen::Pixel* pixels;
				uint burstPhase;
				word palette[Palette::SIZE];
			};

			struct Oam
			{
				Oam();

				enum
				{
					SIZE             = 0x100,
					OFFSET_TO_0_1    = 0xF8,
					STD_LINE_SPRITES = 8,
					MAX_LINE_SPRITES = 32,
					GARBAGE          = 0xFF,
					COLOR            = 0x03,
					BEHIND           = 0x20,
					X_FLIP           = 0x40,
					Y_FLIP           = 0x80,
					XFINE            = 0x07,
					RANGE_MSB        = 0x08,
					TILE_LSB         = 0x01
				};

				struct Output
				{
					byte x;
					byte behind;
					byte zero;
					byte palette;
					byte pixels[8];
				};

				typedef void (Ppu::*Phase)();

				const byte* limit;
				Output* visible;
				Phase phase;
				uint latch;
				uint index;
				byte* buffered;
				uint address;
				uint height;
				uint mask;
				byte show[2];
				bool spriteZeroInLine;
				bool spriteLimit;

				byte ram[0x100];
				byte buffer[MAX_LINE_SPRITES*4];

				Output output[MAX_LINE_SPRITES];
			};

			struct NameTable
			{
				enum
				{
					SIZE = SIZE_2K,
					GARBAGE = 0xFF
				};

				byte ram[SIZE];
			};

			struct TileLut
			{
				TileLut();

				byte block[0x400][4];
			};

			struct Io
			{
				enum
				{
					BUFFER_GARBAGE = 0xE8
				};

				uint address;
				uint pattern;
				uint latch;
				uint buffer;
				Core::Io::Line a12;
			};

			Cpu& cpu;

			struct
			{
				Cycle count;
				Cycle hClock;
				Cycle vClock;
				uint  one;
				Cycle reset;
			}   cycles;

			Io io;
			Regs regs;
			Scroll scroll;
			Tiles tiles;
			Chr chr;
			Nmt nmt;
			int scanline;
			Output output;
			PpuModel model;
			Hook hActiveHook;
			Hook hBlankHook;
			const byte* rgbMap;
			const byte* yuvMap;
			Oam oam;
			Palette palette;
			NameTable nameTable;
			const TileLut tileLut;
			Video::Screen screen;

			static const byte yuvMaps[4][0x40];

		public:

			void Update()
			{
				Update(0);
			}

			PpuModel GetModel() const
			{
				return model;
			}

			ibool IsEnabled() const
			{
				return regs.ctrl1 & Regs::CTRL1_BG_SP_ENABLED;
			}

			bool IsActive() const
			{
				return (regs.ctrl1 & Regs::CTRL1_BG_SP_ENABLED) && scanline < Video::Screen::HEIGHT;
			}

			int GetScanline() const
			{
				return scanline;
			}

			uint GetCtrl0(uint flags) const
			{
				return regs.ctrl0 & flags;
			}

			uint GetCtrl1(uint flags) const
			{
				return regs.ctrl1 & flags;
			}

			Core::Io::Line& A12()
			{
				return io.a12;
			}

			Video::Screen& GetScreen()
			{
				return screen;
			}

			Video::Screen::Pixel* GetOutputPixels()
			{
				return output.pixels;
			}

			void SetOutputPixels(Video::Screen::Pixel* pixels)
			{
				NST_ASSERT( pixels );
				output.pixels = pixels;
			}

			const Palette& GetPalette() const
			{
				return palette;
			}

			uint GetPixel(uint i) const
			{
				NST_ASSERT( i < Video::Screen::PIXELS );
				return output.pixels[i];
			}

			uint GetYuvColor(uint i) const
			{
				NST_ASSERT( i < Palette::COLORS );
				return yuvMap ? yuvMap[i] : i;
			}

			ChrMem& GetChrMem()
			{
				return chr;
			}

			NmtMem& GetNmtMem()
			{
				return nmt;
			}

			Cycle GetClock(dword count=1) const
			{
				NST_ASSERT( count );
				return cycles.one * count;
			}

			uint GetVRamAddress() const
			{
				return scroll.address;
			}

			bool IsShortFrame() const
			{
				return regs.ctrl1 & regs.frame;
			}

			uint GetBurstPhase() const
			{
				return output.burstPhase;
			}

			void EnableSpriteLimit(bool enable)
			{
				oam.spriteLimit = enable;
			}

			bool HasSpriteLimit() const
			{
				return oam.spriteLimit;
			}
		};
	}
}

#endif
