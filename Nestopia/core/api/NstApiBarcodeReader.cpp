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

#include <cstdlib>
#include <ctime>
#include "../NstMachine.hpp"
#include "../NstImage.hpp"
#include "../NstBarcodeReader.hpp"
#include "../input/NstInpDevice.hpp"
#include "../input/NstInpBarcodeWorld.hpp"
#include "NstApiInput.hpp"
#include "NstApiBarcodeReader.hpp"

namespace Nes
{
	namespace Api
	{
		#ifdef NST_MSVC_OPTIMIZE
		#pragma optimize("s", on)
		#endif

		Core::BarcodeReader* BarcodeReader::Query() const
		{
			if (emulator.image)
			{
				if (Core::Image::ExternalDevice device = emulator.image->QueryExternalDevice( Core::Image::EXT_BARCODE_READER ))
				{
					return static_cast<Core::BarcodeReader*>(device);
				}
				else if (emulator.expPort->GetType() == Input::BARCODEWORLD)
				{
					return &static_cast<Core::Input::BarcodeWorld*>(emulator.expPort)->GetReader();
				}
			}

			return NULL;
		}

		uint BarcodeReader::Randomize(char (&string)[MAX_DIGITS+1]) const throw()
		{
			uint digits = 0;

			if (Core::BarcodeReader* const barcodeReader = Query())
			{
				static uint extra = 0x1234;
				std::srand( std::time(NULL) + extra++ );

				if (!barcodeReader->IsDigitsSupported( MIN_DIGITS ))
				{
					digits = MAX_DIGITS;
				}
				else if (barcodeReader->IsDigitsSupported( MAX_DIGITS ) && (std::rand() & 0x1U))
				{
					digits = MAX_DIGITS;
				}
				else
				{
					digits = MIN_DIGITS;
				}

				uint sum = 0;

				for (uint i=0; i < digits-1; ++i)
				{
					const uint digit = uint(std::rand()) / (RAND_MAX / 10 + 1);
					string[i] = '0' + digit;
					sum += (i & 1) ? (digit * 3) : (digit * 1);
				}

				string[digits-1] = '0' + (10 - sum % 10) % 10;
			}

			string[digits] = '\0';

			return digits;
		}

		bool BarcodeReader::IsDigitsSupported(uint count) const throw()
		{
			if (Core::BarcodeReader* const barcodeReader = Query())
				return barcodeReader->IsDigitsSupported( count );

			return false;
		}

		bool BarcodeReader::CanTransfer() const throw()
		{
			return !emulator.tracker.IsLocked() && Query();
		}

		Result BarcodeReader::Transfer(const char* string,uint length) throw()
		{
			if (!emulator.tracker.IsLocked())
			{
				if (Core::BarcodeReader* const barcodeReader = Query())
					return emulator.tracker.TryResync( barcodeReader->Transfer( string, length ) ? RESULT_OK : RESULT_ERR_INVALID_PARAM );
			}

			return RESULT_ERR_NOT_READY;
		}

		#ifdef NST_MSVC_OPTIMIZE
		#pragma optimize("", on)
		#endif
	}
}
