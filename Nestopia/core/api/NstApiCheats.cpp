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

#include <new>
#include "../NstMachine.hpp"
#include "../NstCheats.hpp"
#include "NstApiCheats.hpp"
#include "NstApiMachine.hpp"

namespace Nes
{
	namespace Api
	{
		#ifdef NST_MSVC_OPTIMIZE
		#pragma optimize("s", on)
		#endif

		Result NST_CALL Cheats::GameGenieEncode(const Code& code,char (&characters)[9]) throw()
		{
			if (code.address < 0x8000)
				return RESULT_ERR_INVALID_PARAM;

			const byte codes[8] =
			{
				(code.value   >>  0 & 0x7U) | (code.value   >> 4 & 0x8U),
				(code.value   >>  4 & 0x7U) | (code.address >> 4 & 0x8U),
				(code.address >>  4 & 0x7U) | (code.useCompare   ? 0x8U : 0x0U),
				(code.address >> 12 & 0x7U) | (code.address >> 0 & 0x8U),
				(code.address >>  0 & 0x7U) | (code.address >> 8 & 0x8U),
				(code.address >>  8 & 0x7U) | ((code.useCompare ? code.compare : code.value) & 0x8U),
				(code.useCompare ? ((code.compare >> 0 & 0x7U) | (code.compare >> 4 & 0x8U)) : 0),
				(code.useCompare ? ((code.compare >> 4 & 0x7U) | (code.value   >> 0 & 0x8U)) : 0)
			};

			uint i = (code.useCompare ? 8 : 6);

			characters[i--] = '\0';

			do
			{
				static const char lut[] =
				{
					'A','P','Z','L','G','I','T','Y',
					'E','O','X','U','K','S','V','N'
				};

				characters[i] = lut[codes[i]];
			}
			while (i--);

			return RESULT_OK;
		}

		Result NST_CALL Cheats::GameGenieDecode(const char* const characters,Code& code) throw()
		{
			if (characters == NULL)
				return RESULT_ERR_INVALID_PARAM;

			byte codes[8];

			uint length = 6;

			for (uint i=0; i < length; ++i)
			{
				switch (characters[i])
				{
					case 'A': case 'a': codes[i] = 0x0; break;
					case 'P': case 'p': codes[i] = 0x1; break;
					case 'Z': case 'z': codes[i] = 0x2; break;
					case 'L': case 'l': codes[i] = 0x3; break;
					case 'G': case 'g': codes[i] = 0x4; break;
					case 'I': case 'i': codes[i] = 0x5; break;
					case 'T': case 't': codes[i] = 0x6; break;
					case 'Y': case 'y': codes[i] = 0x7; break;
					case 'E': case 'e': codes[i] = 0x8; break;
					case 'O': case 'o': codes[i] = 0x9; break;
					case 'X': case 'x': codes[i] = 0xA; break;
					case 'U': case 'u': codes[i] = 0xB; break;
					case 'K': case 'k': codes[i] = 0xC; break;
					case 'S': case 's': codes[i] = 0xD; break;
					case 'V': case 'v': codes[i] = 0xE; break;
					case 'N': case 'n': codes[i] = 0xF; break;

					default: return RESULT_ERR_INVALID_PARAM;
				}

				if (i == 2 && codes[2] & 0x8U)
					length = 8;
			}

			code.address = 0x8000 |
			(
				( codes[4] & 0x1U ) << 0x0 |
				( codes[4] & 0x2U ) << 0x0 |
				( codes[4] & 0x4U ) << 0x0 |
				( codes[3] & 0x8U ) << 0x0 |
				( codes[2] & 0x1U ) << 0x4 |
				( codes[2] & 0x2U ) << 0x4 |
				( codes[2] & 0x4U ) << 0x4 |
				( codes[1] & 0x8U ) << 0x4 |
				( codes[5] & 0x1U ) << 0x8 |
				( codes[5] & 0x2U ) << 0x8 |
				( codes[5] & 0x4U ) << 0x8 |
				( codes[4] & 0x8U ) << 0x8 |
				( codes[3] & 0x1U ) << 0xC |
				( codes[3] & 0x2U ) << 0xC |
				( codes[3] & 0x4U ) << 0xC
			);

			code.value =
			(
				( codes[0] & 0x1U ) << 0x0 |
				( codes[0] & 0x2U ) << 0x0 |
				( codes[0] & 0x4U ) << 0x0 |
				( codes[1] & 0x1U ) << 0x4 |
				( codes[1] & 0x2U ) << 0x4 |
				( codes[1] & 0x4U ) << 0x4 |
				( codes[0] & 0x8U ) << 0x4
			);

			if (length == 8)
			{
				code.useCompare = true;
				code.value |= codes[7] & 0x8U;
				code.compare =
				(
					( codes[6] & 0x1U ) << 0x0 |
					( codes[6] & 0x2U ) << 0x0 |
					( codes[6] & 0x4U ) << 0x0 |
					( codes[5] & 0x8U ) << 0x0 |
					( codes[7] & 0x1U ) << 0x4 |
					( codes[7] & 0x2U ) << 0x4 |
					( codes[7] & 0x4U ) << 0x4 |
					( codes[6] & 0x8U ) << 0x4
				);
			}
			else
			{
				code.useCompare = false;
				code.value |= codes[5] & 0x8U;
				code.compare = 0x00;
			}

			return RESULT_OK;
		}

		struct Cheats::Lut
		{
			static const byte rocky[];
		};

		const byte Cheats::Lut::rocky[] =
		{
			3,  13, 14,  1,  6,  9,  5,  0,
			12,  7,  2,  8, 10, 11,  4, 19,
			21, 23, 22, 20, 17, 16, 18, 29,
			31, 24, 26, 25, 30, 27, 28
		};

		Result NST_CALL Cheats::ProActionRockyEncode(const Code& code,char (&characters)[9]) throw()
		{
			if (code.address < 0x8000 || !code.useCompare)
				return RESULT_ERR_INVALID_PARAM;

			const dword input = (code.address & 0x7FFFU) | dword(code.compare) << 16 | dword(code.value) << 24;
			dword output = 0;

			for (dword i=31, key=0xFCBDD274; i--; key = key << 1 & 0xFFFFFFFF)
			{
				const uint ctrl = input >> Lut::rocky[i] & 0x1;
				output |= (key >> 31 ^ ctrl) << (i+1);

				if (ctrl)
					key ^= 0xB8309722;
			}

			characters[8] = '\0';

			for (uint i=0; i < 8; ++i)
			{
				const int value = (output >> (i * 4)) & 0xF;
				characters[i ^ 7] = (value >= 0xA) ? (value - 0xA + 'A') : (value + '0');
			}

			return RESULT_OK;
		}

		Result NST_CALL Cheats::ProActionRockyDecode(const char* const characters,Code& code) throw()
		{
			if (characters == NULL)
				return RESULT_ERR_INVALID_PARAM;

			dword input=0, output=0;

			for (uint i=0; i < 8; ++i)
			{
				dword num;
				const int character = characters[i ^ 7];

				if (character >= '0' && character <= '9')
				{
					num = character - '0';
				}
				else if (character >= 'A' && character <= 'F')
				{
					num = character - 'A' + 0xA;
				}
				else if (character >= 'a' && character <= 'f')
				{
					num = character - 'a' + 0xA;
				}
				else
				{
					return RESULT_ERR_INVALID_PARAM;
				}

				input |= num << (i * 4);
			}

			for (dword i=31, key=0xFCBDD274; i--; input <<= 1, key <<= 1)
			{
				if ((key ^ input) & 0x80000000)
				{
					output |= 1UL << Lut::rocky[i];
					key ^= 0xB8309722;
				}
			}

			code.address    = output & 0x7FFF | 0x8000;
			code.compare    = output >> 16 & 0xFF;
			code.value      = output >> 24 & 0xFF;
			code.useCompare = true;

			return RESULT_OK;
		}

		Result Cheats::SetCode(const Code& code) throw()
		{
			if (emulator.tracker.IsLocked( true ))
				return RESULT_ERR_NOT_READY;

			try
			{
				if (emulator.cheats == NULL)
					emulator.cheats = new Core::Cheats( emulator.cpu );

				return emulator.tracker.TryResync
				(
					emulator.cheats->SetCode
					(
						code.address,
						code.value,
						code.compare,
						code.useCompare,
						emulator.Is(Machine::GAME)
					),
					true
				);
			}
			catch (const std::bad_alloc&)
			{
				return RESULT_ERR_OUT_OF_MEMORY;
			}
			catch (...)
			{
				return RESULT_ERR_GENERIC;
			}
		}

		Result Cheats::DeleteCode(const ulong index) throw()
		{
			if (emulator.tracker.IsLocked( true ))
				return RESULT_ERR_NOT_READY;

			if (!emulator.cheats)
				return RESULT_ERR_INVALID_PARAM;

			const Result result = emulator.tracker.TryResync( emulator.cheats->DeleteCode( index ), true );

			if (!emulator.cheats->NumCodes())
			{
				delete emulator.cheats;
				emulator.cheats = NULL;
			}

			return result;
		}

		ulong Cheats::NumCodes() const throw()
		{
			return emulator.cheats ? emulator.cheats->NumCodes() : 0;
		}

		Result Cheats::GetCode(ulong index,ushort* address,uchar* value,uchar* compare,bool* useCompare) const throw()
		{
			if (emulator.cheats)
				return emulator.cheats->GetCode( index, address, value, compare, useCompare );
			else
				return RESULT_ERR_INVALID_PARAM;
		}

		Result Cheats::GetCode(ulong index,Code& code) const throw()
		{
			return GetCode( index, &code.address, &code.value, &code.compare, &code.useCompare );
		}

		Result Cheats::ClearCodes() throw()
		{
			if (emulator.tracker.IsLocked( true ))
				return RESULT_ERR_NOT_READY;

			if (!emulator.cheats)
				return RESULT_NOP;

			if (emulator.cheats->NumCodes())
				emulator.tracker.Resync( true );

			delete emulator.cheats;
			emulator.cheats = NULL;

			return RESULT_OK;
		}

		Cheats::Ram Cheats::GetRam() const throw()
		{
			return emulator.cpu.GetRam();
		}

		#ifdef NST_MSVC_OPTIMIZE
		#pragma optimize("", on)
		#endif
	}
}

