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

#ifndef NST_API_FDS_H
#define NST_API_FDS_H

#include <iosfwd>
#include <vector>
#include "NstApi.hpp"

#ifdef NST_PRAGMA_ONCE
#pragma once
#endif

#if NST_ICC >= 810
#pragma warning( push )
#pragma warning( disable : 304 444 )
#elif NST_MSVC >= 1200
#pragma warning( push )
#pragma warning( disable : 4512 )
#endif

namespace Nes
{
	namespace Api
	{
		/**
		* Famicom Disk System interface.
		*/
		class Fds : public Base
		{
			struct DiskCaller;
			struct DriveCaller;

		public:

			/**
			* Interface constructor.
			*
			* @param instance emulator instance
			*/
			template<typename T>
			Fds(T& instance)
			: Base(instance) {}

			enum
			{
				NO_DISK = -1
			};

			/**
			* Checks if a disk is inserted.
			*
			* @return true if a disk is inserted
			*/
			bool IsAnyDiskInserted() const throw();

			/**
			* Inserts a disk.
			*
			* @param disk disk number
			* @param side disk side, 0(A) or 1(B)
			* @return result code
			*/
			Result InsertDisk(uint disk,uint side) throw();

			/**
			* Changes disk side.
			*
			* @return result code
			*/
			Result ChangeSide() throw();

			/**
			* Ejects disk.
			*
			* @return result code
			*/
			Result EjectDisk() throw();

			/**
			* Sets BIOS.
			*
			* @param input stream to ROM binary or iNES file, set to NULL to remove current BIOS
			* @result result code
			*/
			Result SetBIOS(std::istream* stream) throw();

			/**
			* Stores the current BIOS in an output stream.
			*
			* @param output stream
			* @return result code
			*/
			Result GetBIOS(std::ostream& stream) const throw();

			/**
			* Checks if a BIOS has been loaded.
			*
			* @return true if a BIOS has been loaded.
			*/
			bool HasBIOS() const throw();

			/**
			* Returns the total number of disks.
			*
			* @return number
			*/
			uint GetNumDisks() const throw();

			/**
			* Returns the total number of disks and their sides.
			*
			* @return number
			*/
			uint GetNumSides() const throw();

			/**
			* Returns the current disk inserted.
			*
			* @return current disk or NO_DISK if none
			*/
			int GetCurrentDisk() const throw();

			/**
			* Returns the current disk side.
			*
			* @return 0(A), 1(B) or NO_DISK if no disk inserted
			*/
			int GetCurrentDiskSide() const throw();

			/**
			* Checks if the current disk can change side.
			*
			* @return true if disk can change side
			*/
			bool CanChangeDiskSide() const throw();

			/**
			* Checks if the current loaded image comes with a file header.
			*
			* @return true if it comes with a file header
			*/
			bool HasHeader() const throw();

			/**
			* Disk data context.
			*/
			struct DiskData
			{
				DiskData() throw();
				~DiskData() throw();

				/**
				* Data content.
				*/
				typedef std::vector<uchar> Data;

				/**
				* File on disk.
				*/
				struct File
				{
					File() throw();

					/**
					* File type.
					*/
					enum Type
					{
						/**
						* Unknown file.
						*/
						TYPE_UNKNOWN,
						/**
						* PRG data file.
						*/
						TYPE_PRG,
						/**
						* CHR data file.
						*/
						TYPE_CHR,
						/**
						* Name-table data file.
						*/
						TYPE_NMT
					};

					/**
					* File ID.
					*/
					uchar id;

					/**
					* File index.
					*/
					uchar index;

					/**
					* File address.
					*/
					ushort address;

					/**
					* File type.
					*/
					Type type;

					/**
					* File content.
					*/
					Data data;

					/**
					* File name.
					*/
					char name[12];
				};

				/**
				* Files.
				*/
				typedef std::vector<File> Files;

				/**
				* Files.
				*/
				Files files;

				/**
				* Raw binary content.
				*/
				Data raw;
			};

			/**
			* Returns disk information.
			*
			* @param side disks and sides index
			* @param data object to be filled
			* @return result code
			*/
			Result GetDiskData(uint side,DiskData& data) const throw();

			/**
			* Disk event.
			*/
			enum Event
			{
				/**
				* Disk has been inserted.
				*/
				DISK_INSERT,
				/**
				* Disk has been ejected.
				*/
				DISK_EJECT,
				/**
				* Disk is in a non-standard format.
				*/
				DISK_NONSTANDARD
			};

			/**
			* Drive event.
			*/
			enum Motor
			{
				/**
				* Drive motor is OFF.
				*/
				MOTOR_OFF,
				/**
				* Drive motor is ON reading.
				*/
				MOTOR_READ,
				/**
				* Drive motor is ON writing.
				*/
				MOTOR_WRITE
			};

			enum
			{
				NUM_DISK_CALLBACKS = 3,
				NUM_DRIVE_CALLBACKS = 3
			};

			/**
			* Disk event callback prototype.
			*
			* @param userData optional user data
			* @param event type of event
			* @param disk disk number
			* @param disk side, 0(A) or 1(B)
			*/
			typedef void (NST_CALLBACK *DiskCallback)(UserData userData,Event event,uint disk,uint side);

			/**
			* Drive event callback prototype.
			*
			* @param userData optional user data
			* @param event type of event
			*/
			typedef void (NST_CALLBACK *DriveCallback)(UserData userData,Motor event);

			/**
			* Disk event callback manager.
			*
			* Static object used for adding the user defined callback.
			*/
			static DiskCaller diskCallback;

			/**
			* Drive event callback manager.
			*
			* Static object used for adding the user defined callback.
			*/
			static DriveCaller driveCallback;
		};

		/**
		* Disk event callback invoker.
		*
		* Used internally by the core.
		*/
		struct Fds::DiskCaller : Core::UserCallback<Fds::DiskCallback>
		{
			void operator () (Event event,uint disk,uint side) const
			{
				if (function)
					function( userdata, event, disk, side );
			}
		};

		/**
		* Drive event callback invoker.
		*
		* Used internally by the core.
		*/
		struct Fds::DriveCaller : Core::UserCallback<Fds::DriveCallback>
		{
			void operator () (Motor motor) const
			{
				if (function)
					function( userdata, motor );
			}
		};
	}
}

#if NST_MSVC >= 1200 || NST_ICC >= 810
#pragma warning( pop )
#endif

#endif
