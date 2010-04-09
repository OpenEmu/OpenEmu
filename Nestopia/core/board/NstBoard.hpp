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

#ifndef NST_BOARD_H
#define NST_BOARD_H

#include "../NstCpu.hpp"
#include "../NstPpu.hpp"
#include "../NstChips.hpp"
#include "../NstState.hpp"

#ifdef NST_PRAGMA_ONCE
#pragma once
#endif

namespace Nes
{
	namespace Core
	{
		class File;

		namespace Input
		{
			class Controllers;
		}

		namespace Boards
		{
			class NST_NO_VTABLE Board
			{
			public:

				class Type
				{
				public:

					enum Nmt
					{
						NMT_HORIZONTAL,
						NMT_VERTICAL,
						NMT_FOURSCREEN,
						NMT_SINGLESCREEN,
						NMT_CONTROLLED
					};

				private:

					enum NmtInit
					{
						NMT_X,
						NMT_H,
						NMT_V,
						NMT_Z,
						NMT_1,
						NMT_2,
						NMT_4
					};

					enum Cram
					{
						CRM_0,
						CRM_1,
						CRM_2,
						CRM_4,
						CRM_6,
						CRM_8,
						CRM_16,
						CRM_32
					};

					enum
					{
						NMPR = 100
					};

					template<byte MPR,word PROM,word CROM,byte NVWRAM,byte WRAM,Cram CRAM,NmtInit NMT,byte UNIQUE>
					struct MakeId
					{
						NST_COMPILE_ASSERT
						(
							( PROM >= 8 && PROM <= 4096 && !(PROM & (PROM-1U)) ) &&
							( !CROM || (CROM >= 8 && CROM <= 4096 && !(CROM & (CROM-1U))) ) &&
							( NVWRAM <= 64 && !(NVWRAM & (NVWRAM-1U)) ) &&
							( WRAM <= 64 && !(WRAM & (WRAM-1U)))  &&
							( NVWRAM + WRAM <= 64 ) &&
							( CROM || CRAM >= CRM_8 ) &&
							( UNIQUE < 16 )
						);

						enum
						{
							ID =
							(
								dword( MPR                               ) << 24 |
								dword( ValueBits< (PROM   >> 4) >::VALUE ) << 20 |
								dword( ValueBits< (CROM   >> 3) >::VALUE ) << 16 |
								dword( ValueBits< (NVWRAM >> 0) >::VALUE ) << 13 |
								dword( ValueBits< (WRAM   >> 0) >::VALUE ) << 10 |
								dword( CRAM                              ) <<  7 |
								dword( NMT                               ) <<  4 |
								dword( UNIQUE                            ) <<  0
							)
						};
					};

				public:

					enum Id
					{
						// NROM
						STD_NROM                   = MakeId<    0,   32,    8,  0,  0, CRM_0,  NMT_X,  0 >::ID,
						// AxROM
						STD_AMROM                  = MakeId<    7,  128,    0,  0,  0, CRM_8,  NMT_Z,  0 >::ID,
						STD_ANROM                  = MakeId<    7,  128,    0,  0,  0, CRM_8,  NMT_Z,  1 >::ID,
						STD_AN1ROM                 = MakeId<    7,   64,    0,  0,  0, CRM_8,  NMT_Z,  0 >::ID,
						STD_AOROM                  = MakeId<    7,  256,    0,  0,  0, CRM_8,  NMT_Z,  0 >::ID,
						// BxROM
						STD_BNROM                  = MakeId<   34,  128,    0,  0,  0, CRM_8,  NMT_X,  0 >::ID,
						// CxROM
						STD_CNROM                  = MakeId<    3,   32,   32,  0,  0, CRM_0,  NMT_X,  0 >::ID,
						STD_CXROM                  = MakeId<    3,   32,   32,  0,  0, CRM_0,  NMT_X,  1 >::ID,
						STD_CPROM                  = MakeId<   13,   32,    0,  0,  0, CRM_16, NMT_X,  0 >::ID,
						// DxROM
						STD_DEROM                  = MakeId<  206,   64,   64,  0,  0, CRM_0,  NMT_X,  0 >::ID,
						STD_DE1ROM                 = MakeId<  206,  128,   64,  0,  0, CRM_0,  NMT_X,  0 >::ID,
						STD_DRROM                  = MakeId<  206,  128,   64,  0,  0, CRM_0,  NMT_2,  0 >::ID,
						// ExROM
						STD_ELROM                  = MakeId<    5,  512,  512,  0,  0, CRM_0,  NMT_X,  0 >::ID,
						STD_EKROM                  = MakeId<    5,  512,  512,  8,  0, CRM_0,  NMT_X,  0 >::ID,
						STD_ETROM                  = MakeId<    5,  512,  512,  8,  8, CRM_0,  NMT_X,  0 >::ID,
						STD_EWROM                  = MakeId<    5,  512,  512, 32,  0, CRM_0,  NMT_X,  0 >::ID,
						STD_EXROM_0                = MakeId<    5, 1024, 1024,  0,  0, CRM_0,  NMT_X,  0 >::ID,
						STD_EXROM_1                = MakeId<    5, 1024, 1024,  8,  0, CRM_0,  NMT_X,  0 >::ID,
						STD_EXROM_2                = MakeId<    5, 1024, 1024,  8,  8, CRM_0,  NMT_X,  0 >::ID,
						STD_EXROM_3                = MakeId<    5, 1024, 1024, 32,  0, CRM_0,  NMT_X,  0 >::ID,
						STD_EXROM_4                = MakeId<    5, 1024, 1024, 32,  8, CRM_0,  NMT_X,  0 >::ID,
						STD_EXROM_5                = MakeId<    5, 1024, 1024, 32, 32, CRM_0,  NMT_X,  0 >::ID,
						// FxROM
						STD_FJROM                  = MakeId<   10,  128,  128,  8,  0, CRM_0,  NMT_V,  0 >::ID,
						STD_FKROM                  = MakeId<   10,  256,  128,  8,  0, CRM_0,  NMT_V,  0 >::ID,
						// GxROM
						STD_GNROM                  = MakeId<   66,  128,   32,  0,  0, CRM_0,  NMT_X,  0 >::ID,
						// MxROM
						STD_MHROM                  = MakeId<   66,   64,   32,  0,  0, CRM_0,  NMT_X,  0 >::ID,
						// HxROM
						STD_HKROM                  = MakeId<    4,  512,  256,  0,  0, CRM_0,  NMT_V,  0 >::ID,
						// JxROM
						STD_JLROM                  = MakeId<   69,  256,  256,  0,  0, CRM_0,  NMT_V,  1 >::ID,
						STD_JSROM                  = MakeId<   69,  256,  256,  8,  0, CRM_0,  NMT_V,  1 >::ID,
						// NxROM
						STD_NTBROM                 = MakeId<   68,  256,  128,  8,  0, CRM_0,  NMT_V,  0 >::ID,
						// PxROM
						STD_PNROM                  = MakeId<    9,  128,  128,  0,  0, CRM_0,  NMT_V,  0 >::ID,
						STD_PNROM_PC10             = MakeId<    9,  128,  128,  8,  0, CRM_0,  NMT_V,  0 >::ID,
						STD_PEEOROM                = MakeId<    9,  128,  128,  0,  0, CRM_0,  NMT_V,  1 >::ID,
						// SxROM
						STD_SAROM                  = MakeId<    1,   64,   64,  8,  0, CRM_0,  NMT_H,  0 >::ID,
						STD_SBROM                  = MakeId<    1,   64,   64,  0,  0, CRM_0,  NMT_H,  0 >::ID,
						STD_SCROM                  = MakeId<    1,   64,  128,  0,  0, CRM_0,  NMT_H,  0 >::ID,
						STD_SEROM                  = MakeId<    1,   32,   64,  0,  0, CRM_0,  NMT_H,  0 >::ID,
						STD_SFROM                  = MakeId<    1,  256,   64,  0,  0, CRM_0,  NMT_H,  0 >::ID,
						STD_SGROM                  = MakeId<    1,  256,    0,  0,  0, CRM_8,  NMT_H,  0 >::ID,
						STD_SHROM                  = MakeId<    1,   32,  128,  0,  0, CRM_0,  NMT_H,  0 >::ID,
						STD_SJROM                  = MakeId<    1,  256,   64,  8,  0, CRM_0,  NMT_H,  0 >::ID,
						STD_SKROM                  = MakeId<    1,  256,  128,  8,  0, CRM_0,  NMT_H,  0 >::ID,
						STD_SLROM                  = MakeId<    1,  256,  128,  0,  0, CRM_0,  NMT_H,  0 >::ID,
						STD_SNROM                  = MakeId<    1,  256,    0,  8,  0, CRM_8,  NMT_H,  0 >::ID,
						STD_SOROM                  = MakeId<    1,  256,    0,  8,  8, CRM_8,  NMT_H,  0 >::ID,
						STD_SUROM                  = MakeId<    1,  512,    0,  8,  0, CRM_8,  NMT_H,  0 >::ID,
						STD_SXROM                  = MakeId<    1,  512,    0, 32,  0, CRM_8,  NMT_H,  0 >::ID,
						// TxROM
						STD_TEROM                  = MakeId<    4,   32,   64,  0,  0, CRM_0,  NMT_X,  0 >::ID,
						STD_TBROM                  = MakeId<    4,   64,   64,  0,  0, CRM_0,  NMT_X,  0 >::ID,
						STD_TFROM                  = MakeId<    4,  512,   64,  0,  0, CRM_0,  NMT_X,  0 >::ID,
						STD_TGROM                  = MakeId<    4,  512,    0,  0,  0, CRM_8,  NMT_X,  0 >::ID,
						STD_TKROM                  = MakeId<    4,  512,  256,  8,  0, CRM_0,  NMT_X,  0 >::ID,
						STD_TKSROM                 = MakeId<  118,  512,  128,  8,  0, CRM_0,  NMT_Z,  0 >::ID,
						STD_TLROM                  = MakeId<    4,  512,  256,  0,  0, CRM_0,  NMT_X,  0 >::ID,
						STD_TLSROM                 = MakeId<  118,  512,  128,  0,  0, CRM_0,  NMT_Z,  0 >::ID,
						STD_TNROM                  = MakeId<    4,  512,    0,  8,  0, CRM_8,  NMT_X,  0 >::ID,
						STD_TQROM                  = MakeId<  119,  128,   64,  0,  0, CRM_8,  NMT_V,  0 >::ID,
						STD_TR1ROM                 = MakeId<    4,  512,   64,  0,  0, CRM_0,  NMT_4,  0 >::ID,
						STD_TSROM                  = MakeId<    4,  512,  256,  0,  8, CRM_0,  NMT_X,  0 >::ID,
						STD_TVROM                  = MakeId<    4,   64,   64,  0,  0, CRM_0,  NMT_4,  0 >::ID,
						// UxROM
						STD_UNROM                  = MakeId<    2,  128,    0,  0,  0, CRM_8,  NMT_X,  0 >::ID,
						STD_UN1ROM                 = MakeId<   94,  128,    0,  0,  0, CRM_8,  NMT_X,  0 >::ID,
						STD_UOROM                  = MakeId<    2,  256,    0,  0,  0, CRM_8,  NMT_X,  0 >::ID,
						STD_UXROM                  = MakeId<    2,  256,    0,  0,  0, CRM_8,  NMT_X,  1 >::ID,
						// Discrete Logic
						DISCRETE_74_377            = MakeId<   11,  128,  128,  0,  0, CRM_0,  NMT_X,  0 >::ID,
						DISCRETE_74_139_74         = MakeId<   87,   32,   32,  0,  0, CRM_0,  NMT_X,  4 >::ID,
						DISCRETE_74_161_138        = MakeId<   38,  128,   32,  0,  0, CRM_0,  NMT_X,  0 >::ID,
						DISCRETE_74_161_161_32_A   = MakeId<   70,  128,  128,  0,  0, CRM_0,  NMT_X,  0 >::ID,
						DISCRETE_74_161_161_32_B   = MakeId<  152,  128,  128,  0,  0, CRM_0,  NMT_Z,  0 >::ID,
						// Other
						CUSTOM_B4                  = MakeId<    4,  128,  128,  0,  0, CRM_0,  NMT_V,  0 >::ID,
						CUSTOM_BTR                 = MakeId<   69,  512,  256,  8,  0, CRM_0,  NMT_V,  0 >::ID,
						CUSTOM_EVENT               = MakeId<  105,  256,    0,  8,  0, CRM_8,  NMT_H,  0 >::ID,
						CUSTOM_FFE3                = MakeId<    8,  256,  256,  8,  0, CRM_0,  NMT_X,  0 >::ID,
						CUSTOM_FFE4                = MakeId<    6,  256,  256,  8,  0, CRM_32, NMT_X,  0 >::ID,
						CUSTOM_FFE8                = MakeId<   17,  256,  256,  8,  0, CRM_0,  NMT_X,  0 >::ID,
						CUSTOM_FB02                = MakeId<    0,   32,    8,  2,  0, CRM_0,  NMT_X,  0 >::ID,
						CUSTOM_FB04                = MakeId<    0,   32,    8,  4,  0, CRM_0,  NMT_X,  0 >::ID,
						CUSTOM_RUMBLESTATION       = MakeId<   46, 1024, 1024,  0,  0, CRM_0,  NMT_X,  0 >::ID,
						CUSTOM_QJ                  = MakeId<   47,  256,  256,  0,  0, CRM_0,  NMT_V,  0 >::ID,
						CUSTOM_VSSYSTEM_0          = MakeId<   99,   64,   16,  0,  0, CRM_0,  NMT_4,  0 >::ID,
						CUSTOM_VSSYSTEM_1          = MakeId<   99,   64,   16,  8,  0, CRM_0,  NMT_4,  0 >::ID,
						CUSTOM_WH                  = MakeId<    1,  128,   64,  0,  0, CRM_0,  NMT_H,  0 >::ID,
						CUSTOM_X79B                = MakeId<    3,   32,   32,  8,  0, CRM_0,  NMT_X,  0 >::ID,
						CUSTOM_ZZ                  = MakeId<   37,  256,  256,  0,  0, CRM_0,  NMT_V,  0 >::ID,
						// Active Enterprises
						AE_STD                     = MakeId<  228, 2048,  512,  0,  0, CRM_0,  NMT_V,  0 >::ID,
						// AGCI
						AGCI_50282                 = MakeId<  144,   64,   64,  0,  0, CRM_0,  NMT_X,  0 >::ID,
						// AVE
						AVE_NINA001                = MakeId<   34,   64,   64,  8,  0, CRM_0,  NMT_X,  0 >::ID,
						AVE_NINA002                = MakeId<   34,   64,   64,  8,  0, CRM_0,  NMT_X,  1 >::ID,
						AVE_NINA03                 = MakeId<   79,   32,   32,  0,  0, CRM_0,  NMT_X,  0 >::ID,
						AVE_NINA06                 = MakeId<   79,   64,   64,  0,  0, CRM_0,  NMT_X,  0 >::ID,
						AVE_NINA07                 = MakeId<   11,  128,  128,  0,  0, CRM_0,  NMT_X,  1 >::ID,
						AVE_MB_91                  = MakeId<   79,   64,   64,  0,  0, CRM_0,  NMT_X,  1 >::ID,
						AVE_D1012                  = MakeId<  234, 1024, 1024,  0,  0, CRM_0,  NMT_V,  0 >::ID,
						// Bandai
						BANDAI_FCG1                = MakeId<  153,  256,  256,  0,  0, CRM_0,  NMT_V,  0 >::ID,
						BANDAI_FCG2                = MakeId<  153,  256,  256,  0,  0, CRM_0,  NMT_V,  1 >::ID,
						BANDAI_BAJUMP2             = MakeId<  153,  512,    0,  8,  0, CRM_8,  NMT_V,  0 >::ID,
						BANDAI_LZ93D50_24C01       = MakeId<  159,  256,  256,  0,  0, CRM_0,  NMT_V,  0 >::ID,
						BANDAI_LZ93D50_24C02       = MakeId<   16,  256,  256,  0,  0, CRM_0,  NMT_V,  0 >::ID,
						BANDAI_DATACH              = MakeId<  157,  256,    0,  0,  0, CRM_8,  NMT_V,  0 >::ID,
						BANDAI_KARAOKESTUDIO       = MakeId<  188,  256,    0,  0,  0, CRM_8,  NMT_X,  0 >::ID,
						BANDAI_AEROBICSSTUDIO      = MakeId<    3,   32,   32,  0,  0, CRM_0,  NMT_X,  2 >::ID,
						BANDAI_OEKAKIDS            = MakeId<   96,  128,    0,  0,  0, CRM_32, NMT_1,  0 >::ID,
						// Bensheng
						BENSHENG_BS5               = MakeId< NMPR,  128,   64,  0,  0, CRM_0,  NMT_V,  2 >::ID,
						// Bootleg multicarts
						BMC_110IN1                 = MakeId<  255, 2048, 1024,  0,  0, CRM_0,  NMT_V,  0 >::ID,
						BMC_150IN1                 = MakeId<  202,  128,   64,  0,  0, CRM_0,  NMT_V,  0 >::ID,
						BMC_15IN1                  = MakeId<  205,  512,  512,  0,  0, CRM_0,  NMT_X,  0 >::ID,
						BMC_1200IN1                = MakeId<  227,  512,    0,  0,  0, CRM_8,  NMT_V,  0 >::ID,
						BMC_20IN1                  = MakeId<  231,  512,    0,  0,  0, CRM_8,  NMT_V,  0 >::ID,
						BMC_21IN1                  = MakeId<  201,  128,   32,  0,  0, CRM_0,  NMT_V,  0 >::ID,
						BMC_22GAMES                = MakeId<  230, 1024,    0,  0,  0, CRM_8,  NMT_V,  0 >::ID,
						BMC_31IN1                  = MakeId<  229,  512,  256,  0,  0, CRM_0,  NMT_V,  0 >::ID,
						BMC_35IN1                  = MakeId<  203,  128,   64,  0,  0, CRM_0,  NMT_V,  0 >::ID,
						BMC_36IN1                  = MakeId<  200,  128,   64,  0,  0, CRM_0,  NMT_V,  0 >::ID,
						BMC_64IN1                  = MakeId<  204,  128,   64,  0,  0, CRM_0,  NMT_V,  0 >::ID,
						BMC_72IN1                  = MakeId<  225, 1024,  512,  0,  0, CRM_0,  NMT_V,  0 >::ID,
						BMC_76IN1                  = MakeId<  226, 2048,    0,  0,  0, CRM_8,  NMT_H,  0 >::ID,
						BMC_8157                   = MakeId< NMPR,  512,    0,  0,  0, CRM_8,  NMT_V,  0 >::ID,
						BMC_9999999IN1             = MakeId<  213,  128,   64,  0,  0, CRM_0,  NMT_X,  0 >::ID,
						BMC_A65AS                  = MakeId< NMPR,  512,    0,  0,  0, CRM_8,  NMT_V,  1 >::ID,
						BMC_BALLGAMES_11IN1        = MakeId<   51,  512,    0,  0,  0, CRM_8,  NMT_V,  0 >::ID,
						BMC_CTC65                  = MakeId< NMPR, 2048,    0,  0,  0, CRM_8,  NMT_H,  0 >::ID,
						BMC_DRAGONBOLLPARTY        = MakeId<   83, 1024, 1024,  8,  0, CRM_0,  NMT_V,  0 >::ID,
						BMC_FAMILY_4646B           = MakeId<  134,  512,  512,  0,  0, CRM_0,  NMT_X,  0 >::ID,
						BMC_FKC23C                 = MakeId< NMPR, 1024, 1024,  0,  0, CRM_0,  NMT_X,  0 >::ID,
						BMC_GAME_800IN1            = MakeId<  236,  512,   64,  0,  0, CRM_8,  NMT_V,  0 >::ID,
						BMC_GOLDEN_190IN1          = MakeId< NMPR,  128,   64,  0,  0, CRM_0,  NMT_V,  0 >::ID,
						BMC_GOLDENGAME_150IN1      = MakeId<  235, 2048,    0,  0,  0, CRM_8,  NMT_V,  0 >::ID,
						BMC_GOLDENGAME_260IN1      = MakeId<  235, 4096,    0,  0,  0, CRM_8,  NMT_V,  0 >::ID,
						BMC_GKA                    = MakeId<   57,  128,  128,  0,  0, CRM_0,  NMT_V,  0 >::ID,
						BMC_GKB                    = MakeId<   58,  128,   64,  0,  0, CRM_0,  NMT_V,  0 >::ID,
						BMC_GOLDENCARD_6IN1        = MakeId<  217, 1024, 1024,  0,  0, CRM_0,  NMT_V,  0 >::ID,
						BMC_HERO                   = MakeId<   45, 4096, 2048,  8,  0, CRM_0,  NMT_X,  0 >::ID,
						BMC_MARIOPARTY_7IN1        = MakeId<   52, 1024, 1024,  8,  0, CRM_0,  NMT_V,  0 >::ID,
						BMC_NOVELDIAMOND           = MakeId<   54,  128,   64,  0,  0, CRM_0,  NMT_X,  0 >::ID,
						BMC_CH001                  = MakeId<   63, 4096,    0,  0,  0, CRM_8,  NMT_V,  0 >::ID,
						BMC_POWERJOY_84IN1         = MakeId<  126, 2048, 1024,  0,  0, CRM_0,  NMT_X,  0 >::ID,
						BMC_RESETBASED_4IN1        = MakeId<   60,   64,   32,  0,  0, CRM_0,  NMT_X,  0 >::ID,
						BMC_SUPER_24IN1            = MakeId< NMPR, 4096, 2048,  8,  0, CRM_8,  NMT_X,  0 >::ID,
						BMC_SUPER_22GAMES          = MakeId<  233, 1024,    0,  0,  0, CRM_8,  NMT_Z,  0 >::ID,
						BMC_SUPER_40IN1            = MakeId< NMPR,  128,   64,  0,  0, CRM_0,  NMT_V,  1 >::ID,
						BMC_SUPER_42IN1            = MakeId<  226, 1024,    0,  0,  0, CRM_8,  NMT_H,  0 >::ID,
						BMC_SUPER_700IN1           = MakeId<   62, 2048, 1024,  0,  0, CRM_0,  NMT_V,  0 >::ID,
						BMC_SUPERBIG_7IN1          = MakeId<   44, 1024, 1024,  0,  0, CRM_0,  NMT_X,  0 >::ID,
						BMC_SUPERGUN_20IN1         = MakeId<  214,   64,   32,  0,  0, CRM_0,  NMT_X,  0 >::ID,
						BMC_SUPERHIK_4IN1          = MakeId<   49,  512,  512,  0,  0, CRM_0,  NMT_X,  0 >::ID,
						BMC_SUPERHIK_300IN1        = MakeId<  212,  128,   64,  0,  0, CRM_0,  NMT_V,  0 >::ID,
						BMC_SUPERVISION_16IN1      = MakeId<   53, 4096,    0,  0,  0, CRM_8,  NMT_V,  0 >::ID,
						BMC_T262                   = MakeId< NMPR, 1024,    0,  0,  0, CRM_8,  NMT_V,  0 >::ID,
						BMC_VRC4                   = MakeId<   23,  512,    0,  8,  0, CRM_8,  NMT_V,  0 >::ID,
						BMC_VT5201                 = MakeId<   60,  128,   64,  0,  0, CRM_0,  NMT_V,  0 >::ID,
						BMC_Y2K_64IN1              = MakeId< NMPR, 1024,  512,  0,  0, CRM_0,  NMT_V,  0 >::ID,
						// Bootlegs
						BTL_2708                   = MakeId<  103,  128,    0,  0, 16, CRM_8,  NMT_V,  0 >::ID,
						BTL_6035052                = MakeId< NMPR,  512,  256,  0,  0, CRM_0,  NMT_X,  0 >::ID,
						BTL_AISENSHINICOL          = MakeId<   42,  128,    0,  0,  0, CRM_8,  NMT_X,  0 >::ID,
						BTL_AX5705                 = MakeId< NMPR,  128,  256,  0,  0, CRM_0,  NMT_X,  0 >::ID,
						BTL_DRAGONNINJA            = MakeId<  222,  128,  256,  0,  0, CRM_0,  NMT_X,  0 >::ID,
						BTL_GENIUSMERIOBROS        = MakeId<   55,   64,    8,  2,  0, CRM_0,  NMT_X,  0 >::ID,
						BTL_MARIOBABY              = MakeId<   42,  128,  128,  0,  0, CRM_0,  NMT_X,  0 >::ID,
						BTL_PIKACHUY2K             = MakeId<  254,  512,  256,  8,  0, CRM_0,  NMT_X,  0 >::ID,
						BTL_SHUIGUANPIPE           = MakeId<  183,  256,  128,  0,  0, CRM_0,  NMT_X,  0 >::ID,
						BTL_SMB2_A                 = MakeId<   40,   64,    8,  0,  0, CRM_0,  NMT_X,  0 >::ID,
						BTL_SMB2_B                 = MakeId<   50,  128,    0,  0,  0, CRM_8,  NMT_X,  0 >::ID,
						BTL_SMB2_C                 = MakeId<   43,  128,    8,  0,  0, CRM_0,  NMT_X,  0 >::ID,
						BTL_SMB3                   = MakeId<  106,  256,  128,  8,  0, CRM_0,  NMT_H,  0 >::ID,
						BTL_SUPERBROS11            = MakeId<  196,  512,  128,  0,  0, CRM_0,  NMT_V,  0 >::ID,
						BTL_T230                   = MakeId< NMPR,  256,    0,  0,  0, CRM_8,  NMT_V,  0 >::ID,
						BTL_TOBIDASEDAISAKUSEN     = MakeId<  120,  128,    8,  0,  0, CRM_0,  NMT_X,  0 >::ID,
						// Camerica
						CAMERICA_BF9093            = MakeId<   71,  256,    0,  0,  0, CRM_8,  NMT_X,  0 >::ID,
						CAMERICA_BF9096            = MakeId<  232,  256,    0,  0,  0, CRM_8,  NMT_X,  0 >::ID,
						CAMERICA_BF9097            = MakeId<   71,  128,    0,  0,  0, CRM_8,  NMT_Z,  0 >::ID,
						CAMERICA_BF909X            = MakeId<   71,  256,    0,  0,  0, CRM_8,  NMT_X,  1 >::ID,
						CAMERICA_ALGNV11           = MakeId<   71,  256,    0,  0,  0, CRM_8,  NMT_X,  2 >::ID,
						CAMERICA_ALGQV11           = MakeId<  232,  256,    0,  0,  0, CRM_8,  NMT_X,  1 >::ID,
						CAMERICA_GOLDENFIVE        = MakeId<  104, 2048,    0,  0,  0, CRM_8,  NMT_X,  0 >::ID,
						// Camerica
						CALTRON_6IN1               = MakeId<   41,  256,  128,  0,  0, CRM_0,  NMT_V,  0 >::ID,
						// C&E
						CNE_SHLZ                   = MakeId<  240,  512,  128,  8,  0, CRM_0,  NMT_X,  0 >::ID,
						CNE_DECATHLON              = MakeId<  244,  128,   64,  0,  0, CRM_0,  NMT_X,  0 >::ID,
						CNE_PSB                    = MakeId<  246,  512,  512,  8,  0, CRM_0,  NMT_X,  0 >::ID,
						// Cony
						CONY_STD                   = MakeId<   83,  256,  512,  0,  0, CRM_0,  NMT_V,  0 >::ID,
						// Dreamtech
						DREAMTECH_01               = MakeId< NMPR,  256,    0,  0,  0, CRM_8,  NMT_X,  0 >::ID,
						// Fujiya
						FUJIYA_STD                 = MakeId<  170,   32,    8,  0,  0, CRM_0,  NMT_X,  0 >::ID,
						// Fukutake
						FUKUTAKE_SBX               = MakeId<  186,  256,    0,  4,  0, CRM_8,  NMT_X,  0 >::ID,
						// Future Media
						FUTUREMEDIA_STD            = MakeId<  117,  256,  256,  0,  0, CRM_0,  NMT_X,  0 >::ID,
						// Gouder
						GOUDER_37017               = MakeId<  208,  128,  256,  0,  0, CRM_0,  NMT_X,  0 >::ID,
						// HES
						HES_STD                    = MakeId<  113,  256,  128,  0,  0, CRM_0,  NMT_H,  0 >::ID,
						// Hosenkan
						HOSENKAN_STD               = MakeId<  182,  512,  256,  0,  0, CRM_0,  NMT_X,  0 >::ID,
						// Irem
						IREM_G101A_0               = MakeId<   32,  256,  128,  0,  0, CRM_0,  NMT_V,  0 >::ID,
						IREM_G101A_1               = MakeId<   32,  256,  128,  8,  0, CRM_0,  NMT_V,  0 >::ID,
						IREM_G101B_0               = MakeId<   32,  256,  128,  0,  0, CRM_0,  NMT_Z,  0 >::ID,
						IREM_G101B_1               = MakeId<   32,  256,  128,  8,  0, CRM_0,  NMT_Z,  0 >::ID,
						IREM_H3001                 = MakeId<   65,  256,  256,  0,  0, CRM_0,  NMT_X,  0 >::ID,
						IREM_LROG017               = MakeId<   77,  128,   32,  0,  0, CRM_6,  NMT_2,  0 >::ID,
						IREM_HOLYDIVER             = MakeId<   78,  128,  128,  0,  0, CRM_0,  NMT_H,  0 >::ID,
						IREM_KAIKETSU              = MakeId<   97,  256,    0,  0,  0, CRM_8,  NMT_H,  0 >::ID,
						// Jaleco
						JALECO_JF01                = MakeId<    0,   16,    8,  0,  0, CRM_0,  NMT_X,  1 >::ID,
						JALECO_JF02                = MakeId<    0,   16,    8,  0,  0, CRM_0,  NMT_X,  2 >::ID,
						JALECO_JF03                = MakeId<    0,   16,    8,  0,  0, CRM_0,  NMT_X,  3 >::ID,
						JALECO_JF04                = MakeId<    0,   16,    8,  0,  0, CRM_0,  NMT_X,  4 >::ID,
						JALECO_JF05                = MakeId<   87,   16,   16,  0,  0, CRM_0,  NMT_X,  0 >::ID,
						JALECO_JF06                = MakeId<   87,   16,   16,  0,  0, CRM_0,  NMT_X,  1 >::ID,
						JALECO_JF07                = MakeId<   87,   32,   16,  0,  0, CRM_0,  NMT_X,  0 >::ID,
						JALECO_JF08                = MakeId<   87,   32,   32,  0,  0, CRM_0,  NMT_X,  1 >::ID,
						JALECO_JF09                = MakeId<   87,   32,   32,  0,  0, CRM_0,  NMT_X,  2 >::ID,
						JALECO_JF10                = MakeId<   87,   32,   32,  0,  0, CRM_0,  NMT_X,  3 >::ID,
						JALECO_JF11                = MakeId<  140,  128,   32,  0,  0, CRM_0,  NMT_X,  0 >::ID,
						JALECO_JF12                = MakeId<  140,  128,   32,  0,  0, CRM_0,  NMT_X,  1 >::ID,
						JALECO_JF13                = MakeId<   86,  128,   64,  0,  0, CRM_0,  NMT_X,  0 >::ID,
						JALECO_JF14                = MakeId<  140,  128,  128,  0,  0, CRM_0,  NMT_X,  0 >::ID,
						JALECO_JF15                = MakeId<    2,  256,    0,  0,  0, CRM_8,  NMT_X,  2 >::ID,
						JALECO_JF16                = MakeId<   78,  128,  128,  0,  0, CRM_0,  NMT_Z,  1 >::ID,
						JALECO_JF17                = MakeId<   72,  128,  128,  0,  0, CRM_0,  NMT_X,  0 >::ID,
						JALECO_JF18                = MakeId<    2,  256,    0,  0,  0, CRM_8,  NMT_X,  3 >::ID,
						JALECO_JF19                = MakeId<   92,  256,  128,  0,  0, CRM_0,  NMT_X,  0 >::ID,
						JALECO_JF20                = MakeId<   75,  128,  128,  0,  0, CRM_0,  NMT_V,  1 >::ID,
						JALECO_JF21                = MakeId<   92,  256,  128,  0,  0, CRM_0,  NMT_X,  1 >::ID,
						JALECO_JF22                = MakeId<   75,  128,  128,  0,  0, CRM_0,  NMT_V,  2 >::ID,
						JALECO_JF23                = MakeId<   18,  256,  128,  0,  0, CRM_0,  NMT_H,  0 >::ID,
						JALECO_JF24                = MakeId<   18,  128,  128,  0,  0, CRM_0,  NMT_H,  0 >::ID,
						JALECO_JF25                = MakeId<   18,  128,  128,  0,  0, CRM_0,  NMT_H,  1 >::ID,
						JALECO_JF26                = MakeId<   72,  128,  128,  0,  0, CRM_0,  NMT_X,  1 >::ID,
						JALECO_JF27                = MakeId<   18,  128,  128,  8,  0, CRM_0,  NMT_H,  0 >::ID,
						JALECO_JF28                = MakeId<   72,  128,  128,  0,  0, CRM_0,  NMT_X,  2 >::ID,
						JALECO_JF29                = MakeId<   18,  256,  128,  0,  0, CRM_0,  NMT_H,  1 >::ID,
						JALECO_JF30                = MakeId<   18,  128,  128,  0,  0, CRM_0,  NMT_H,  2 >::ID,
						JALECO_JF31                = MakeId<   18,  128,  128,  0,  0, CRM_0,  NMT_H,  3 >::ID,
						JALECO_JF32                = MakeId<   18,  128,  128,  0,  0, CRM_0,  NMT_H,  4 >::ID,
						JALECO_JF33                = MakeId<   18,  256,  128,  0,  0, CRM_0,  NMT_H,  2 >::ID,
						JALECO_JF34                = MakeId<   18,  128,  128,  0,  0, CRM_0,  NMT_H,  5 >::ID,
						JALECO_JF35                = MakeId<   18,  128,  128,  0,  0, CRM_0,  NMT_H,  6 >::ID,
						JALECO_JF36                = MakeId<   18,  128,  128,  0,  0, CRM_0,  NMT_H,  7 >::ID,
						JALECO_JF37                = MakeId<   18,  128,  256,  0,  0, CRM_0,  NMT_H,  0 >::ID,
						JALECO_JF38                = MakeId<   18,  128,  128,  0,  0, CRM_0,  NMT_H,  8 >::ID,
						JALECO_JF39                = MakeId<    2,  128,    0,  0,  0, CRM_8,  NMT_X,  1 >::ID,
						JALECO_JF40                = MakeId<   18,  128,  128,  0,  0, CRM_0,  NMT_H,  9 >::ID,
						JALECO_JF41                = MakeId<   18,  256,  128,  8,  0, CRM_0,  NMT_H,  3 >::ID,
						JALECO_SS88006             = MakeId<   18,  256,  256,  8,  0, CRM_0,  NMT_H,  4 >::ID,
						// J.Y.Company
						JYCOMPANY_TYPE_A           = MakeId<   90, 2048, 2048,  0,  0, CRM_0,  NMT_X,  0 >::ID,
						JYCOMPANY_TYPE_B           = MakeId<  209, 2048, 2048,  0,  0, CRM_0,  NMT_X,  0 >::ID,
						JYCOMPANY_TYPE_C           = MakeId<  211, 2048, 2048,  0,  0, CRM_0,  NMT_X,  0 >::ID,
						// Kaiser
						KAISER_KS202               = MakeId<   56,  256,  128,  8,  0, CRM_0,  NMT_V,  0 >::ID,
						KAISER_KS7022              = MakeId<  175,  256,  128,  0,  0, CRM_0,  NMT_V,  0 >::ID,
						KAISER_KS7032              = MakeId<  142,  128,    0,  0,  0, CRM_8,  NMT_X,  0 >::ID,
						KAISER_KS7058              = MakeId<  171,   32,   32,  0,  0, CRM_0,  NMT_X,  0 >::ID,
						// Kasing
						KASING_STD                 = MakeId<  115,  512,  512,  0,  0, CRM_0,  NMT_V,  0 >::ID,
						// K
						KAY_H2288                  = MakeId<  123,  512,  256,  8,  0, CRM_0,  NMT_V,  0 >::ID,
						KAY_PANDAPRINCE            = MakeId<  121,  256,  256,  0,  0, CRM_0,  NMT_V,  0 >::ID,
						// Konami
						KONAMI_VRC1                = MakeId<   75,  128,  128,  0,  0, CRM_0,  NMT_V,  0 >::ID,
						KONAMI_VRC2                = MakeId<   22,  128,  256,  0,  0, CRM_0,  NMT_V,  0 >::ID,
						KONAMI_VRC3                = MakeId<   73,  128,    0,  8,  0, CRM_8,  NMT_X,  0 >::ID,
						KONAMI_VRC4_0              = MakeId<   21,  256,  256,  0,  0, CRM_0,  NMT_V,  0 >::ID,
						KONAMI_VRC4_1              = MakeId<   21,  256,  256,  2,  0, CRM_0,  NMT_V,  0 >::ID,
						KONAMI_VRC4_2              = MakeId<   21,  256,  256,  8,  0, CRM_0,  NMT_V,  0 >::ID,
						KONAMI_VRC6_0              = MakeId<   24,  256,  256,  0,  0, CRM_0,  NMT_V,  0 >::ID,
						KONAMI_VRC6_1              = MakeId<   24,  256,  256,  8,  0, CRM_0,  NMT_V,  0 >::ID,
						KONAMI_VRC7_0              = MakeId<   85,  512,  256,  0,  0, CRM_0,  NMT_V,  0 >::ID,
						KONAMI_VRC7_1              = MakeId<   85,  512,  256,  8,  0, CRM_0,  NMT_V,  0 >::ID,
						KONAMI_VSSYSTEM            = MakeId<  151,   64,   64,  0,  0, CRM_0,  NMT_4,  0 >::ID,
						// Magic Series
						MAGICSERIES_MAGICDRAGON    = MakeId<  107,  128,   64,  0,  0, CRM_0,  NMT_X,  0 >::ID,
						// Namcot
						NAMCOT_3433                = MakeId<   88,  128,  128,  0,  0, CRM_0,  NMT_H,  0 >::ID,
						NAMCOT_3443                = MakeId<   88,  128,  128,  0,  0, CRM_0,  NMT_V,  0 >::ID,
						NAMCOT_3446                = MakeId<   76,  128,  128,  0,  0, CRM_0,  NMT_X,  0 >::ID,
						NAMCOT_3425                = MakeId<   95,  128,  128,  0,  0, CRM_0,  NMT_Z,  0 >::ID,
						NAMCOT_34XX                = MakeId<  154,  128,  128,  0,  0, CRM_0,  NMT_Z,  0 >::ID,
						NAMCOT_163_0               = MakeId<   19,  512,  256,  0,  0, CRM_0,  NMT_X,  0 >::ID,
						NAMCOT_163_1               = MakeId<   19,  512,  256,  8,  0, CRM_0,  NMT_X,  0 >::ID,
						NAMCOT_163_S_0             = MakeId<   19,  512,  256,  0,  0, CRM_0,  NMT_X,  1 >::ID,
						NAMCOT_163_S_1             = MakeId<   19,  512,  256,  8,  0, CRM_0,  NMT_X,  1 >::ID,
						// Nitra
						NITRA_TDA                  = MakeId<  250,  512,  256,  0,  0, CRM_0,  NMT_V,  0 >::ID,
						// NTDEC
						NTDEC_N715062              = MakeId<    3,   32,   32,  0,  0, CRM_0,  NMT_X,  3 >::ID,
						NTDEC_ASDER_0              = MakeId<  112,  256,  512,  0,  0, CRM_0,  NMT_X,  0 >::ID,
						NTDEC_ASDER_1              = MakeId<  112,  256,  512,  8,  0, CRM_0,  NMT_X,  0 >::ID,
						NTDEC_FIGHTINGHERO         = MakeId<  193,  128,  256,  0,  0, CRM_0,  NMT_X,  0 >::ID,
						// Nanjing
						NANJING_STD                = MakeId<  163, 2048,    0,  8,  0, CRM_8,  NMT_X,  0 >::ID,
						// Nihon Bussan
						NIHON_UNROM_M5             = MakeId<  180,  128,    0,  0,  0, CRM_8,  NMT_X,  0 >::ID,
						// Open Corp
						OPENCORP_DAOU306           = MakeId<  156,  256,  256,  8,  0, CRM_0,  NMT_1,  0 >::ID,
						// RCM
						RCM_GS2004                 = MakeId< NMPR,  512,    0,  0,  0, CRM_8,  NMT_X,  0 >::ID,
						RCM_GS2013                 = MakeId< NMPR,  512,    0,  0,  0, CRM_8,  NMT_X,  1 >::ID,
						RCM_GS2015                 = MakeId<  216,   64,   64,  0,  0, CRM_0,  NMT_X,  0 >::ID,
						RCM_TETRISFAMILY           = MakeId<   61,  512,    0,  0,  0, CRM_8,  NMT_V,  0 >::ID,
						// Rex Soft
						REXSOFT_DBZ5               = MakeId<   12,  256,  512,  0,  0, CRM_0,  NMT_V,  0 >::ID,
						REXSOFT_SL1632             = MakeId<   14,  256,  512,  0,  0, CRM_0,  NMT_V,  0 >::ID,
						// Sachen
						SACHEN_8259A               = MakeId<  141,  256,  256,  0,  0, CRM_0,  NMT_X,  0 >::ID,
						SACHEN_8259B               = MakeId<  138,  256,  256,  0,  0, CRM_0,  NMT_X,  0 >::ID,
						SACHEN_8259C               = MakeId<  139,  256,  256,  0,  0, CRM_0,  NMT_X,  0 >::ID,
						SACHEN_8259D               = MakeId<  137,  256,  256,  0,  0, CRM_0,  NMT_X,  0 >::ID,
						SACHEN_TCA01               = MakeId<  143,   32,    8,  0,  0, CRM_0,  NMT_X,  0 >::ID,
						SACHEN_TCU01               = MakeId<  147,  128,  128,  0,  0, CRM_0,  NMT_X,  0 >::ID,
						SACHEN_TCU02               = MakeId<  136,   32,   32,  0,  0, CRM_0,  NMT_X,  0 >::ID,
						SACHEN_SA0036              = MakeId<  149,   32,   16,  0,  0, CRM_0,  NMT_X,  0 >::ID,
						SACHEN_SA0037              = MakeId<  148,   64,   64,  0,  0, CRM_0,  NMT_X,  0 >::ID,
						SACHEN_SA0161M             = MakeId<  146,   64,   64,  0,  0, CRM_0,  NMT_X,  0 >::ID,
						SACHEN_SA72007             = MakeId<  145,   16,   16,  0,  0, CRM_0,  NMT_X,  0 >::ID,
						SACHEN_SA72008             = MakeId<  133,   64,   32,  0,  0, CRM_0,  NMT_X,  0 >::ID,
						SACHEN_74_374A             = MakeId<  243,   64,  128,  0,  0, CRM_0,  NMT_V,  0 >::ID,
						SACHEN_74_374B             = MakeId<  150,   64,  128,  0,  0, CRM_0,  NMT_X,  0 >::ID,
						SACHEN_STREETHEROES        = MakeId< NMPR,  512,  512,  0,  0, CRM_8,  NMT_2,  0 >::ID,
						// Someri Team
						SOMERITEAM_SL12            = MakeId<  116,  256,  512,  0,  0, CRM_0,  NMT_X,  0 >::ID,
						// Subor
						SUBOR_TYPE0                = MakeId<  167, 1024,    0,  8,  0, CRM_8,  NMT_X,  0 >::ID,
						SUBOR_TYPE1                = MakeId<  166, 1024,    0,  8,  0, CRM_8,  NMT_X,  0 >::ID,
						SUBOR_STUDYNGAME           = MakeId<   39, 1024,    0,  8,  0, CRM_8,  NMT_X,  0 >::ID,
						// Sunsoft
						SUNSOFT_1                  = MakeId<  184,   32,   32,  0,  0, CRM_0,  NMT_X,  0 >::ID,
						SUNSOFT_2A                 = MakeId<   93,  128,  128,  0,  0, CRM_0,  NMT_X,  0 >::ID,
						SUNSOFT_2B                 = MakeId<   89,  128,  128,  0,  0, CRM_0,  NMT_Z,  0 >::ID,
						SUNSOFT_3                  = MakeId<   67,  128,  128,  0,  0, CRM_0,  NMT_V,  0 >::ID,
						SUNSOFT_4_0                = MakeId<   68,  128,  256,  0,  0, CRM_0,  NMT_V,  0 >::ID,
						SUNSOFT_4_1                = MakeId<   68,  128,  256,  8,  0, CRM_0,  NMT_V,  0 >::ID,
						SUNSOFT_5B_0               = MakeId<   69,  256,  256,  0,  0, CRM_0,  NMT_V,  2 >::ID,
						SUNSOFT_5B_1               = MakeId<   69,  256,  256,  8,  0, CRM_0,  NMT_V,  2 >::ID,
						SUNSOFT_DCS                = MakeId<   68,  256,  256,  8,  0, CRM_0,  NMT_V,  0 >::ID,
						SUNSOFT_FME7_0             = MakeId<   69,  256,  256,  0,  0, CRM_0,  NMT_V,  0 >::ID,
						SUNSOFT_FME7_1             = MakeId<   69,  256,  256,  8,  0, CRM_0,  NMT_V,  0 >::ID,
						// Super Game
						SUPERGAME_LIONKING         = MakeId<  114,  256,  512,  0,  0, CRM_0,  NMT_V,  0 >::ID,
						SUPERGAME_BOOGERMAN        = MakeId<  215,  512,  512,  0,  0, CRM_0,  NMT_X,  0 >::ID,
						SUPERGAME_MK3E             = MakeId<  215,  256,  512,  0,  0, CRM_0,  NMT_X,  0 >::ID,
						SUPERGAME_POCAHONTAS2      = MakeId< NMPR,  512,  512,  0,  0, CRM_0,  NMT_X,  0 >::ID,
						// Taito
						TAITO_TC0190FMC            = MakeId<   33,  256,  256,  0,  0, CRM_0,  NMT_V,  0 >::ID,
						TAITO_TC0190FMC_PAL16R4    = MakeId<   48,  256,  256,  0,  0, CRM_0,  NMT_V,  0 >::ID,
						TAITO_X1005                = MakeId<   80,  256,  256,  0,  0, CRM_0,  NMT_X,  0 >::ID,
						TAITO_X1017                = MakeId<   82,  256,  256,  0,  0, CRM_0,  NMT_H,  0 >::ID,
						// Tengen
						TENGEN_800002              = MakeId<  206,   64,   64,  0,  0, CRM_0,  NMT_X,  1 >::ID,
						TENGEN_800004              = MakeId<  206,  128,   64,  0,  0, CRM_0,  NMT_2,  1 >::ID,
						TENGEN_800008              = MakeId<    3,   64,   64,  0,  0, CRM_0,  NMT_X,  0 >::ID,
						TENGEN_800030              = MakeId<  206,  128,   64,  0,  0, CRM_0,  NMT_X,  1 >::ID,
						TENGEN_800032              = MakeId<   64,  128,  128,  0,  0, CRM_0,  NMT_V,  0 >::ID,
						TENGEN_800037              = MakeId<  158,  128,  128,  0,  0, CRM_0,  NMT_Z,  0 >::ID,
						TENGEN_800042              = MakeId<   68,  128,  256,  0,  0, CRM_0,  NMT_V,  1 >::ID,
						// TXC
						TXC_22211A                 = MakeId<  132,   64,   32,  0,  0, CRM_0,  NMT_X,  0 >::ID,
						TXC_22211B                 = MakeId<  172,   64,   32,  0,  0, CRM_0,  NMT_X,  0 >::ID,
						TXC_22211C                 = MakeId<  173,   64,   32,  0,  0, CRM_0,  NMT_X,  0 >::ID,
						TXC_MXMDHTWO               = MakeId<  241, 1024,    0,  8,  0, CRM_8,  NMT_X,  0 >::ID,
						TXC_POLICEMAN              = MakeId<   36,  512,  128,  0,  0, CRM_0,  NMT_X,  0 >::ID,
						TXC_TW                     = MakeId<  189,  128,  256,  0,  0, CRM_0,  NMT_X,  0 >::ID,
						// Unlicensed
						UNL_CC21                   = MakeId< NMPR,   32,   16,  0,  0, CRM_0,  NMT_Z,  0 >::ID,
						UNL_EDU2000                = MakeId< NMPR, 1024,    0, 32,  0, CRM_8,  NMT_Z,  0 >::ID,
						UNL_KINGOFFIGHTERS96       = MakeId<  187,  512,  512,  0,  0, CRM_0,  NMT_X,  0 >::ID,
						UNL_KINGOFFIGHTERS97       = MakeId< NMPR,  512,  256,  0,  0, CRM_0,  NMT_X,  1 >::ID,
						UNL_MORTALKOMBAT2          = MakeId<   91,  256,  512,  0,  0, CRM_0,  NMT_X,  0 >::ID,
						UNL_SUPERFIGHTER3          = MakeId<  197,  512,  512,  0,  0, CRM_0,  NMT_X,  0 >::ID,
						UNL_TF1201                 = MakeId< NMPR,  256,  256,  0,  0, CRM_0,  NMT_V,  0 >::ID,
						UNL_WORLDHERO              = MakeId<   27,  128,  512,  0,  0, CRM_0,  NMT_X,  0 >::ID,
						UNL_AXROM                  = MakeId<    7,  512,    8,  8,  0, CRM_0,  NMT_Z,  0 >::ID,
						UNL_BXROM                  = MakeId<   34, 4096,    8,  8,  0, CRM_8,  NMT_X,  0 >::ID,
						UNL_CXROM                  = MakeId<    3,   32, 2048,  8,  0, CRM_0,  NMT_X,  0 >::ID,
						UNL_GXROM                  = MakeId<   66,  512,  128,  8,  0, CRM_0,  NMT_X,  0 >::ID,
						UNL_NROM                   = MakeId<    0,   32,    8,  8,  0, CRM_8,  NMT_X,  0 >::ID,
						UNL_UXROM                  = MakeId<    2, 4096,    8,  8,  0, CRM_0,  NMT_X,  0 >::ID,
						UNL_TRXROM                 = MakeId<    4,  512,  256,  8,  0, CRM_0,  NMT_4,  0 >::ID,
						UNL_XZY                    = MakeId<  176, 1024,  256,  8,  0, CRM_0,  NMT_X,  0 >::ID,
						// Waixing
						WAIXING_PS2_0              = MakeId<   15, 1024,    0,  0,  0, CRM_8,  NMT_V,  0 >::ID,
						WAIXING_PS2_1              = MakeId<   15, 1024,    0,  8,  0, CRM_8,  NMT_V,  0 >::ID,
						WAIXING_TYPE_A             = MakeId<   74, 1024,  256,  8,  1, CRM_2,  NMT_X,  0 >::ID,
						WAIXING_TYPE_B             = MakeId<  191,  512,  256,  8,  1, CRM_2,  NMT_X,  0 >::ID,
						WAIXING_TYPE_C             = MakeId<  192,  512,  256,  8,  1, CRM_4,  NMT_X,  0 >::ID,
						WAIXING_TYPE_D             = MakeId<  194,  512,  256,  8,  1, CRM_2,  NMT_X,  0 >::ID,
						WAIXING_TYPE_E             = MakeId<  195,  512,  256,  8,  1, CRM_4,  NMT_X,  0 >::ID,
						WAIXING_TYPE_F             = MakeId<  198, 1024,    0,  8,  1, CRM_8,  NMT_X,  0 >::ID,
						WAIXING_TYPE_G             = MakeId<  199,  512,  256,  8,  1, CRM_8,  NMT_X,  0 >::ID,
						WAIXING_TYPE_H             = MakeId<  245, 1024,  256,  8,  0, CRM_0,  NMT_X,  0 >::ID,
						WAIXING_TYPE_I             = MakeId<  223,  512,  256,  8,  1, CRM_0,  NMT_X,  0 >::ID,
						WAIXING_TYPE_J             = MakeId<  224, 1024,  256,  8,  1, CRM_0,  NMT_X,  0 >::ID,
						WAIXING_FFV_0              = MakeId<  164, 1024,   32,  0,  0, CRM_8,  NMT_X,  0 >::ID,
						WAIXING_FFV_1              = MakeId<  164, 1024,   32,  8,  0, CRM_8,  NMT_X,  0 >::ID,
						WAIXING_SH2_0              = MakeId<  165,  512,  256,  0,  0, CRM_4,  NMT_X,  0 >::ID,
						WAIXING_SH2_1              = MakeId<  165,  512,  256,  8,  0, CRM_4,  NMT_X,  0 >::ID,
						WAIXING_SGZLZ              = MakeId<  178, 1024,    0,  8,  0, CRM_8,  NMT_X,  0 >::ID,
						WAIXING_ZS                 = MakeId<  242,  512,    0,  8,  0, CRM_8,  NMT_V,  0 >::ID,
						WAIXING_DQVII              = MakeId<  242,  512,    0,  8,  0, CRM_8,  NMT_X,  0 >::ID,
						WAIXING_SGZ                = MakeId<  252,  512,  256,  8,  0, CRM_0,  NMT_X,  0 >::ID,
						WAIXING_SECURITY_0         = MakeId<  249,  512,  256,  0,  0, CRM_0,  NMT_V,  0 >::ID,
						WAIXING_SECURITY_1         = MakeId<  249,  512,  256,  8,  0, CRM_0,  NMT_V,  0 >::ID,
						// Whirlwind
						WHIRLWIND_2706             = MakeId<  108,  128,    0,  0,  0, CRM_8,  NMT_X,  0 >::ID,
						// Unknown
						UNKNOWN                    = 0
					};

					Type();
					Type(Id,Ram&,Ram&,Nmt,bool,bool);

					dword GetMaxPrg() const;
					dword GetMaxChr() const;
					uint  GetWram() const;
					uint  GetSavableWram() const;
					uint  GetNonSavableWram() const;
					uint  GetChrRam() const;
					uint  GetNmtRam() const;
					uint  GetVram() const;
					uint  GetSavableVram() const;
					uint  GetNonSavableVram() const;
					Nmt   GetStartupNmt() const;

				private:

					Id id;
					byte nmt;
					byte chrRam;
					bool battery;
					bool wramAuto;

				public:

					Id GetId() const
					{
						return id;
					}

					bool operator == (Id i) const
					{
						return id == i;
					}

					bool operator != (Id i) const
					{
						return id != i;
					}

					Nmt GetNmt() const
					{
						return static_cast<Nmt>(nmt);
					}

					bool HasBattery() const
					{
						return battery;
					}

					bool IsAutoWram() const
					{
						return wramAuto;
					}
				};

				class Context
				{
					struct Element;

				public:

					Context(Cpu*,Apu*,Ppu*,Ram&,Ram&,const Ram&,Type::Nmt,bool,bool,Chips&);

					bool DetectBoard(wcstring,dword);
					bool DetectBoard(byte,dword,bool);

					cstring name;
					Type type;
					Cpu* const cpu;
					Apu* const apu;
					Ppu* const ppu;
					Ram& prg;
					Ram& chr;
					const Ram& trainer;
					const Type::Nmt nmt;
					Chips& chips;
					const bool wramBattery;
					const bool mmcBattery;
				};

				static Board* Create(const Context&);
				static void Destroy(Board*);

				void Reset(bool);

				virtual void Load(File&);
				virtual void Save(File&) const;

				void SaveState(State::Saver&,dword) const;
				void LoadState(State::Loader&);

				enum Event
				{
					EVENT_END_FRAME,
					EVENT_BEGIN_FRAME,
					EVENT_POWER_OFF
				};

				virtual void Sync(Event,Input::Controllers*) {};

				typedef void* Device;

				enum DeviceType
				{
					DEVICE_DIP_SWITCHES = 1,
					DEVICE_BARCODE_READER
				};

				virtual Device QueryDevice(DeviceType)
				{
					return NULL;
				}

			protected:

				explicit Board(const Context&);
				virtual ~Board() {}

				typedef Memory<SIZE_32K,SIZE_8K,2> Prg;
				typedef Memory<SIZE_8K,SIZE_8K,2> Wrk;
				typedef Ppu::ChrMem Chr;
				typedef Ppu::NmtMem Nmt;
				typedef Ram Vram;

				Prg prg;
				Cpu& cpu;
				Ppu& ppu;
				Chr& chr;
				Nmt& nmt;
				Wrk wrk;
				const Vram vram;
				const Type board;

			private:

				virtual void SubReset(bool) = 0;
				virtual void SubSave(State::Saver&) const {}
				virtual void SubLoad(State::Loader&,dword) {}

				NES_DECL_PEEK( Prg_8 );
				NES_DECL_PEEK( Prg_A );
				NES_DECL_PEEK( Prg_C );
				NES_DECL_PEEK( Prg_E );

				NES_DECL_POKE( Prg_8k_0     );
				NES_DECL_POKE( Prg_8k_0_bc  );
				NES_DECL_POKE( Prg_8k_1     );
				NES_DECL_POKE( Prg_8k_2     );
				NES_DECL_POKE( Prg_8k_3     );
				NES_DECL_POKE( Prg_16k_0    );
				NES_DECL_POKE( Prg_16k_0_bc );
				NES_DECL_POKE( Prg_16k_1    );
				NES_DECL_POKE( Prg_32k      );
				NES_DECL_POKE( Prg_32k_bc   );

				NES_DECL_POKE( Chr_1k_0    );
				NES_DECL_POKE( Chr_1k_1    );
				NES_DECL_POKE( Chr_1k_2    );
				NES_DECL_POKE( Chr_1k_3    );
				NES_DECL_POKE( Chr_1k_4    );
				NES_DECL_POKE( Chr_1k_5    );
				NES_DECL_POKE( Chr_1k_6    );
				NES_DECL_POKE( Chr_1k_7    );
				NES_DECL_POKE( Chr_2k_0    );
				NES_DECL_POKE( Chr_2k_1    );
				NES_DECL_POKE( Chr_2k_2    );
				NES_DECL_POKE( Chr_2k_3    );
				NES_DECL_POKE( Chr_4k_0    );
				NES_DECL_POKE( Chr_4k_1    );
				NES_DECL_POKE( Chr_4k_1_bc );
				NES_DECL_POKE( Chr_8k      );
				NES_DECL_POKE( Chr_8k_bc   );

				NES_DECL_PEEK( Wram_6 );
				NES_DECL_POKE( Wram_6 );

				NES_DECL_POKE( Nmt_Hv );
				NES_DECL_POKE( Nmt_Vh );
				NES_DECL_POKE( Nmt_Vh01 );
				NES_DECL_POKE( Nmt_Hv01 );

				NES_DECL_PEEK( Nop );
				NES_DECL_POKE( Nop );

			protected:

				enum PrgSwap8k0    { PRG_SWAP_8K_0      };
				enum PrgSwap8k0Bc  { PRG_SWAP_8K_0_BC   };
				enum PrgSwap8k1    { PRG_SWAP_8K_1      };
				enum PrgSwap8k2    { PRG_SWAP_8K_2      };
				enum PrgSwap8k3    { PRG_SWAP_8K_3      };
				enum PrgSwap16k0   { PRG_SWAP_16K_0     };
				enum PrgSwap16k0Bc { PRG_SWAP_16K_0_BC  };
				enum PrgSwap16k1   { PRG_SWAP_16K_1     };
				enum PrgSwap32k    { PRG_SWAP_32K       };
				enum PrgSwap32kBc  { PRG_SWAP_32K_BC    };
				enum ChrSwap1k0    { CHR_SWAP_1K_0      };
				enum ChrSwap1k1    { CHR_SWAP_1K_1      };
				enum ChrSwap1k2    { CHR_SWAP_1K_2      };
				enum ChrSwap1k3    { CHR_SWAP_1K_3      };
				enum ChrSwap1k4    { CHR_SWAP_1K_4      };
				enum ChrSwap1k5    { CHR_SWAP_1K_5      };
				enum ChrSwap1k6    { CHR_SWAP_1K_6      };
				enum ChrSwap1k7    { CHR_SWAP_1K_7      };
				enum ChrSwap2k0    { CHR_SWAP_2K_0      };
				enum ChrSwap2k1    { CHR_SWAP_2K_1      };
				enum ChrSwap2k2    { CHR_SWAP_2K_2      };
				enum ChrSwap2k3    { CHR_SWAP_2K_3      };
				enum ChrSwap4k0    { CHR_SWAP_4K_0      };
				enum ChrSwap4k1    { CHR_SWAP_4K_1      };
				enum ChrSwap4k1Bc  { CHR_SWAP_4K_1_BC   };
				enum ChrSwap8k     { CHR_SWAP_8K        };
				enum ChrSwap8kBc   { CHR_SWAP_8K_BC     };
				enum NmtSwapHv     { NMT_SWAP_HV        };
				enum NmtSwapVh     { NMT_SWAP_VH        };
				enum NmtSwapVh01   { NMT_SWAP_VH01      };
				enum NmtSwapHv01   { NMT_SWAP_HV01      };
				enum NopPeek       { NOP_PEEK           };
				enum NopPoke       { NOP_POKE           };
				enum NopPeekPoke   { NOP_PEEK_POKE      };

				uint GetBusData(uint,uint=0xFF) const;

				void SetMirroringHV(uint data) { NES_DO_POKE(Nmt_Hv,0,data); }
				void SetMirroringVH(uint data) { NES_DO_POKE(Nmt_Vh,0,data); }

				void SetMirroringVH01(uint data) { NES_DO_POKE(Nmt_Vh01,0,data); }
				void SetMirroringHV01(uint data) { NES_DO_POKE(Nmt_Hv01,0,data); }

				void Map( uint,uint,PrgSwap8k0  ) const;
				void Map( uint,uint,PrgSwap8k1  ) const;
				void Map( uint,uint,PrgSwap8k2  ) const;
				void Map( uint,uint,PrgSwap8k3  ) const;
				void Map( uint,uint,PrgSwap16k0 ) const;
				void Map( uint,uint,PrgSwap16k1 ) const;
				void Map( uint,uint,PrgSwap32k  ) const;
				void Map( uint,uint,ChrSwap1k0  ) const;
				void Map( uint,uint,ChrSwap1k1  ) const;
				void Map( uint,uint,ChrSwap1k2  ) const;
				void Map( uint,uint,ChrSwap1k3  ) const;
				void Map( uint,uint,ChrSwap1k4  ) const;
				void Map( uint,uint,ChrSwap1k5  ) const;
				void Map( uint,uint,ChrSwap1k6  ) const;
				void Map( uint,uint,ChrSwap1k7  ) const;
				void Map( uint,uint,ChrSwap2k0  ) const;
				void Map( uint,uint,ChrSwap2k1  ) const;
				void Map( uint,uint,ChrSwap2k2  ) const;
				void Map( uint,uint,ChrSwap2k3  ) const;
				void Map( uint,uint,ChrSwap4k0  ) const;
				void Map( uint,uint,ChrSwap4k1  ) const;
				void Map( uint,uint,ChrSwap8k   ) const;
				void Map( uint,uint,NmtSwapHv   ) const;
				void Map( uint,uint,NmtSwapVh   ) const;
				void Map( uint,uint,NmtSwapVh01 ) const;
				void Map( uint,uint,NmtSwapHv01 ) const;
				void Map( uint,uint,NopPeek     ) const;
				void Map( uint,uint,NopPoke     ) const;
				void Map( uint,uint,NopPeekPoke ) const;

				void Map( PrgSwap8k0Bc  ) const;
				void Map( PrgSwap16k0Bc ) const;
				void Map( PrgSwap32kBc  ) const;
				void Map( ChrSwap4k1Bc  ) const;
				void Map( ChrSwap8kBc   ) const;

				template<typename T>
				void Map(uint first,uint last,T t) const
				{
					cpu.Map( first, last ).Set( t );
				}

				template<typename T,typename U>
				void Map(uint first,uint last,T t,U u) const
				{
					cpu.Map( first, last ).Set( t, u );
				}

				template<typename T>
				void Map(uint address,T t) const
				{
					Map( address, address, t );
				}

				template<typename T,typename U>
				void Map(uint address,T t,U u) const
				{
					cpu.Map( address ).Set( t, u );
				}
			};
		}
	}
}

#endif
