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

#ifndef NST_BASE_H
#define NST_BASE_H

#include <climits>
#include "api/NstApiConfig.hpp"

//--------------------------------------------------------------------------------------
// Microsoft Visual C++
//--------------------------------------------------------------------------------------

#ifdef _MSC_VER
#define NST_MSVC _MSC_VER
#else
#define NST_MSVC 0
#endif

//--------------------------------------------------------------------------------------
// Intel C/C++ Compiler
//--------------------------------------------------------------------------------------

#ifdef __INTEL_COMPILER
#define NST_ICC __INTEL_COMPILER
#else
#define NST_ICC 0
#endif

//--------------------------------------------------------------------------------------
// GNU Compiler Collection
//--------------------------------------------------------------------------------------

#ifdef __GNUC__
#define NST_GCC (__GNUC__ * 100 + __GNUC_MINOR__)
#else
#define NST_GCC 0
#endif

//--------------------------------------------------------------------------------------
// Borland C++
//--------------------------------------------------------------------------------------

#ifdef __BORLANDC__
#define NST_BCB __BORLANDC__
#else
#define NST_BCB 0
#endif

//--------------------------------------------------------------------------------------
// Metrowerks CodeWarrior
//--------------------------------------------------------------------------------------

#ifdef __MWERKS__
#define NST_MWERKS __MWERKS__
#else
#define NST_MWERKS 0
#endif

//--------------------------------------------------------------------------------------

#ifdef NST_PRAGMA_ONCE
#pragma once
#elif NST_MSVC >= 1020 || NST_MWERKS >= 0x3000
#pragma once
#define NST_PRAGMA_ONCE
#endif

//--------------------------------------------------------------------------------------

#ifndef NST_CALL
#define NST_CALL
#endif

namespace Nes
{
	typedef signed char schar;
	typedef unsigned char uchar;
	typedef unsigned short ushort;
	typedef unsigned int uint;
	typedef unsigned long ulong;

	#if UCHAR_MAX >= 0xFF
	typedef unsigned char byte;
	#else
	#error Unsupported plattform!
	#endif

	#if UCHAR_MAX >= 0xFFFF
	typedef unsigned char word;
	#elif USHRT_MAX >= 0xFFFF
	typedef unsigned short word;
	#elif UINT_MAX >= 0xFFFF
	typedef unsigned int word;
	#else
	#error Unsupported plattform!
	#endif

	#if SCHAR_MAX >= 32767 && SCHAR_MIN <= -32767
	typedef signed char iword;
	#elif SHRT_MAX >= 32767 && SHRT_MIN <= -32767
	typedef signed short iword;
	#elif INT_MAX >= 32767 && INT_MIN <= -32767
	typedef signed int iword;
	#else
	#error Unsupported plattform!
	#endif

	#if UCHAR_MAX >= 0xFFFFFFFF
	typedef unsigned char dword;
	#elif USHRT_MAX >= 0xFFFFFFFF
	typedef unsigned short dword;
	#elif UINT_MAX >= 0xFFFFFFFF
	typedef unsigned int dword;
	#elif ULONG_MAX >= 0xFFFFFFFF
	typedef unsigned long dword;
	#else
	#error Unsupported plattform!
	#endif

	#if SCHAR_MAX >= 2147483647 && SCHAR_MIN <= -2147483647
	typedef signed char idword;
	#elif SHRT_MAX >= 2147483647 && SHRT_MIN <= -2147483647
	typedef signed short idword;
	#elif INT_MAX >= 2147483647 && INT_MIN <= -2147483647
	typedef signed int idword;
	#elif LONG_MAX >= 2147483647 && LONG_MIN <= -2147483647
	typedef signed long idword;
	#else
	#error Unsupported plattform!
	#endif

	/**
	* General result codes.
	*/
	enum Result
	{
		/**
		* NTSC/PAL region mismatch.
		*/
		RESULT_ERR_WRONG_MODE = -13,
		/**
		* Missing FDS BIOS.
		*/
		RESULT_ERR_MISSING_BIOS = -12,
		/**
		* Unsupported or malformed mapper.
		*/
		RESULT_ERR_UNSUPPORTED_MAPPER = -11,
		/**
		* Vs DualSystem is unsupported.
		*/
		RESULT_ERR_UNSUPPORTED_VSSYSTEM = -10,
		/**
		* File format version is no longer supported.
		*/
		RESULT_ERR_UNSUPPORTED_FILE_VERSION = -9,
		/**
		* Unsupported operation.
		*/
		RESULT_ERR_UNSUPPORTED = -8,
		/**
		* Invalid CRC checksum.
		*/
		RESULT_ERR_INVALID_CRC = -7,
		/**
		* Corrupt file.
		*/
		RESULT_ERR_CORRUPT_FILE = -6,
		/**
		* Invalid file.
		*/
		RESULT_ERR_INVALID_FILE = -5,
		/**
		* Invalid parameter(s).
		*/
		RESULT_ERR_INVALID_PARAM = -4,
		/**
		* System not ready.
		*/
		RESULT_ERR_NOT_READY = -3,
		/**
		* Out of memory.
		*/
		RESULT_ERR_OUT_OF_MEMORY = -2,
		/**
		* Generic error.
		*/
		RESULT_ERR_GENERIC = -1,
		/**
		* Success.
		*/
		RESULT_OK = 0,
		/**
		* Success but operation had no effect.
		*/
		RESULT_NOP = 1,
		/**
		* Success but image dump may be bad.
		*/
		RESULT_WARN_BAD_DUMP = 2,
		/**
		* Success but PRG-ROM may be bad.
		*/
		RESULT_WARN_BAD_PROM = 3,
		/**
		* Success but CHR-ROM may be bad.
		*/
		RESULT_WARN_BAD_CROM = 4,
		/**
		* Success but file header may have incorrect data.
		*/
		RESULT_WARN_BAD_FILE_HEADER = 5,
		/**
		* Success but save data has been lost.
		*/
		RESULT_WARN_SAVEDATA_LOST = 6,
		/**
		* Success but data may have been replaced.
		*/
		RESULT_WARN_DATA_REPLACED = 8
	};

	namespace Core
	{
		enum Region
		{
			REGION_NTSC,
			REGION_PAL
		};

		enum System
		{
			SYSTEM_NES_NTSC,
			SYSTEM_NES_PAL,
			SYSTEM_NES_PAL_A,
			SYSTEM_NES_PAL_B,
			SYSTEM_FAMICOM,
			SYSTEM_VS_UNISYSTEM,
			SYSTEM_VS_DUALSYSTEM,
			SYSTEM_PLAYCHOICE_10
		};

		enum FavoredSystem
		{
			FAVORED_NES_NTSC,
			FAVORED_NES_PAL,
			FAVORED_FAMICOM
		};

		enum CpuModel
		{
			CPU_RP2A03,
			CPU_RP2A07
		};

		enum PpuModel
		{
			PPU_RP2C02,
			PPU_RP2C03B,
			PPU_RP2C03G,
			PPU_RP2C04_0001,
			PPU_RP2C04_0002,
			PPU_RP2C04_0003,
			PPU_RP2C04_0004,
			PPU_RC2C03B,
			PPU_RC2C03C,
			PPU_RC2C05_01,
			PPU_RC2C05_02,
			PPU_RC2C05_03,
			PPU_RC2C05_04,
			PPU_RC2C05_05,
			PPU_RP2C07
		};

		enum
		{
			CLK_M2_MUL      = 6,
			CLK_NTSC        = 39375000UL * CLK_M2_MUL,
			CLK_NTSC_DIV    = 11,
			CLK_NTSC_HVSYNC = 525UL * 455 * CLK_NTSC_DIV * CLK_M2_MUL / 4,
			CLK_PAL         = 35468950UL * CLK_M2_MUL,
			CLK_PAL_DIV     = 8,
			CLK_PAL_HVSYNC  = 625UL * 1418758 / (10000/CLK_PAL_DIV) * CLK_M2_MUL
		};

		enum
		{
			CPU_RP2A03_CC = 12,
			CPU_RP2A07_CC = 16
		};

		enum
		{
			PPU_RP2C02_CC         = 4,
			PPU_RP2C02_HACTIVE    = PPU_RP2C02_CC * 256,
			PPU_RP2C02_HBLANK     = PPU_RP2C02_CC * 85,
			PPU_RP2C02_HSYNC      = PPU_RP2C02_HACTIVE + PPU_RP2C02_HBLANK,
			PPU_RP2C02_VACTIVE    = 240,
			PPU_RP2C02_VSLEEP     = 1,
			PPU_RP2C02_VINT       = 20,
			PPU_RP2C02_VDUMMY     = 1,
			PPU_RP2C02_VBLANK     = PPU_RP2C02_VSLEEP + PPU_RP2C02_VINT + PPU_RP2C02_VDUMMY,
			PPU_RP2C02_VSYNC      = PPU_RP2C02_VACTIVE + PPU_RP2C02_VBLANK,
			PPU_RP2C02_HVSYNCBOOT = PPU_RP2C02_VACTIVE * PPU_RP2C02_HSYNC + PPU_RP2C02_CC * 312,
			PPU_RP2C02_HVREGBOOT  = (PPU_RP2C02_VACTIVE + PPU_RP2C02_VINT) * PPU_RP2C02_HSYNC + PPU_RP2C02_CC * 314,
			PPU_RP2C02_HVINT      = PPU_RP2C02_VINT * ulong(PPU_RP2C02_HSYNC),
			PPU_RP2C02_HVSYNC_0   = PPU_RP2C02_VSYNC * ulong(PPU_RP2C02_HSYNC),
			PPU_RP2C02_HVSYNC_1   = PPU_RP2C02_VSYNC * ulong(PPU_RP2C02_HSYNC) - PPU_RP2C02_CC,
			PPU_RP2C02_HVSYNC     = (PPU_RP2C02_HVSYNC_0 + ulong(PPU_RP2C02_HVSYNC_1)) / 2,
			PPU_RP2C02_FPS        = (CLK_NTSC + CLK_NTSC_DIV * ulong(PPU_RP2C02_HVSYNC) / 2) / (CLK_NTSC_DIV * ulong(PPU_RP2C02_HVSYNC)),
			PPU_RP2C07_CC         = 5,
			PPU_RP2C07_HACTIVE    = PPU_RP2C07_CC * 256,
			PPU_RP2C07_HBLANK     = PPU_RP2C07_CC * 85,
			PPU_RP2C07_HSYNC      = PPU_RP2C07_HACTIVE + PPU_RP2C07_HBLANK,
			PPU_RP2C07_VACTIVE    = 240,
			PPU_RP2C07_VSLEEP     = 1,
			PPU_RP2C07_VINT       = 70,
			PPU_RP2C07_VDUMMY     = 1,
			PPU_RP2C07_VBLANK     = PPU_RP2C07_VSLEEP + PPU_RP2C07_VINT + PPU_RP2C07_VDUMMY,
			PPU_RP2C07_VSYNC      = PPU_RP2C07_VACTIVE + PPU_RP2C07_VBLANK,
			PPU_RP2C07_HVSYNCBOOT = PPU_RP2C07_VACTIVE * PPU_RP2C07_HSYNC + PPU_RP2C07_CC * 312,
			PPU_RP2C07_HVREGBOOT  = (PPU_RP2C07_VACTIVE + PPU_RP2C07_VINT) * PPU_RP2C07_HSYNC + PPU_RP2C07_CC * 314,
			PPU_RP2C07_HVINT      = PPU_RP2C07_VINT * ulong(PPU_RP2C07_HSYNC),
			PPU_RP2C07_HVSYNC     = PPU_RP2C07_VSYNC * ulong(PPU_RP2C07_HSYNC),
			PPU_RP2C07_FPS        = (CLK_PAL + CLK_PAL_DIV * ulong(PPU_RP2C07_HVSYNC) / 2) / (CLK_PAL_DIV * ulong(PPU_RP2C07_HVSYNC))
		};

		template<typename T>
		class ImplicitBool;

		template<>
		class ImplicitBool<void>
		{
		public:

			int type;
			typedef int ImplicitBool<void>::*Type;
		};

		template<typename T>
		class ImplicitBool
		{
			template<typename U> void operator == (const ImplicitBool<U>&) const;
			template<typename U> void operator != (const ImplicitBool<U>&) const;

		public:

			operator ImplicitBool<void>::Type () const
			{
				return !static_cast<const T&>(*this) ? 0 : &ImplicitBool<void>::type;
			}
		};
	}
}

#define NES_FAILED(x_) ((x_) < Nes::RESULT_OK)
#define NES_SUCCEEDED(x_) ((x_) >= Nes::RESULT_OK)

#endif
