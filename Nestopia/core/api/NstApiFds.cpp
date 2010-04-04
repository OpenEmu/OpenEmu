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
#include "../NstStream.hpp"
#include "../NstFds.hpp"
#include "NstApiCartridge.hpp"
#include "NstApiMachine.hpp"
#include "NstApiFds.hpp"

namespace Nes
{
	namespace Api
	{
		#ifdef NST_MSVC_OPTIMIZE
		#pragma optimize("s", on)
		#endif

		Fds::DiskCaller Fds::diskCallback;
		Fds::DriveCaller Fds::driveCallback;

		Fds::DiskData::File::File() throw()
		:
		id      (0),
		index   (0),
		address (0),
		type    (TYPE_UNKNOWN)
		{
			for (uint i=0; i < sizeof(name); ++i)
				name[i] = '\0';
		}

		Fds::DiskData::DiskData() throw()
		{
		}

		Fds::DiskData::~DiskData() throw()
		{
		}

		Result Fds::GetDiskData(uint side,DiskData& data) const throw()
		{
			if (emulator.Is(Machine::DISK))
				return static_cast<Core::Fds*>(emulator.image)->GetDiskData( side, data );

			return RESULT_ERR_NOT_READY;
		}

		bool Fds::IsAnyDiskInserted() const throw()
		{
			if (emulator.Is(Machine::DISK))
				return static_cast<const Core::Fds*>(emulator.image)->IsAnyDiskInserted();

			return false;
		}

		Result Fds::InsertDisk(uint disk,uint side) throw()
		{
			if (emulator.Is(Machine::DISK) && !emulator.tracker.IsLocked())
				return emulator.tracker.TryResync( static_cast<Core::Fds*>(emulator.image)->InsertDisk( disk, side ) );

			return RESULT_ERR_NOT_READY;
		}

		Result Fds::ChangeSide() throw()
		{
			const int disk = GetCurrentDisk();

			if (disk != NO_DISK)
				return InsertDisk( disk, GetCurrentDiskSide() ^ 1 );

			return RESULT_ERR_NOT_READY;
		}

		Result Fds::EjectDisk() throw()
		{
			if (emulator.Is(Machine::DISK) && !emulator.tracker.IsLocked())
				return emulator.tracker.TryResync( static_cast<Core::Fds*>(emulator.image)->EjectDisk() );

			return RESULT_ERR_NOT_READY;
		}

		Result Fds::SetBIOS(std::istream* const stdStream) throw()
		{
			if (emulator.Is(Machine::GAME,Machine::ON))
				return RESULT_ERR_NOT_READY;

			try
			{
				if (stdStream)
				{
					Core::Stream::In stream( stdStream );

					idword offset;

					Cartridge::NesHeader setup;

					byte header[16];
					stream.Read( header );

					if (NES_FAILED(setup.Import( header, 16 )))
					{
						offset = -16;
					}
					else if (setup.prgRom >= Core::SIZE_8K)
					{
						offset = (setup.trainer ? 512 : 0) + (setup.prgRom - Core::SIZE_8K);
					}
					else
					{
						return RESULT_ERR_CORRUPT_FILE;
					}

					stream.Seek( offset );
				}

				Core::Fds::SetBios( stdStream );
			}
			catch (Result result)
			{
				return result;
			}
			catch (const std::bad_alloc&)
			{
				return RESULT_ERR_OUT_OF_MEMORY;
			}
			catch (...)
			{
				return RESULT_ERR_GENERIC;
			}

			return RESULT_OK;
		}

		Result Fds::GetBIOS(std::ostream& stream) const throw()
		{
			try
			{
				return Core::Fds::GetBios( stream );
			}
			catch (Result result)
			{
				return result;
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

		bool Fds::HasBIOS() const throw()
		{
			return Core::Fds::HasBios();
		}

		uint Fds::GetNumDisks() const throw()
		{
			if (emulator.Is(Machine::DISK))
				return static_cast<const Core::Fds*>(emulator.image)->NumDisks();

			return 0;
		}

		uint Fds::GetNumSides() const throw()
		{
			if (emulator.Is(Machine::DISK))
				return static_cast<const Core::Fds*>(emulator.image)->NumSides();

			return 0;
		}

		int Fds::GetCurrentDisk() const throw()
		{
			if (emulator.Is(Machine::DISK))
				return static_cast<const Core::Fds*>(emulator.image)->CurrentDisk();

			return NO_DISK;
		}

		int Fds::GetCurrentDiskSide() const throw()
		{
			if (emulator.Is(Machine::DISK))
				return static_cast<const Core::Fds*>(emulator.image)->CurrentDiskSide();

			return NO_DISK;
		}

		bool Fds::CanChangeDiskSide() const throw()
		{
			if (emulator.Is(Machine::DISK))
				return static_cast<const Core::Fds*>(emulator.image)->CanChangeDiskSide();

			return false;
		}

		bool Fds::HasHeader() const throw()
		{
			return emulator.Is(Machine::DISK) && static_cast<const Core::Fds*>(emulator.image)->HasHeader();
		}

		#ifdef NST_MSVC_OPTIMIZE
		#pragma optimize("", on)
		#endif
	}
}
