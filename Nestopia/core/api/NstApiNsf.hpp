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

#ifndef NST_API_NSF_H
#define NST_API_NSF_H

#include "NstApi.hpp"

#ifdef NST_PRAGMA_ONCE
#pragma once
#endif

#if NST_ICC >= 810
#pragma warning( push )
#pragma warning( disable : 444 )
#elif NST_MSVC >= 1200
#pragma warning( push )
#pragma warning( disable : 4512 )
#endif

namespace Nes
{
	namespace Api
	{
		/**
		* NES Sound Files interface.
		*/
		class Nsf : public Base
		{
			struct EventCaller;

		public:

			/**
			* Interface constructor.
			*
			* @param instance emulator instance
			*/
			template<typename T>
			Nsf(T& instance)
			: Base(instance) {}

			enum
			{
				NO_SONG = -1
			};

			/**
			* Tune mode.
			*/
			enum TuneMode
			{
				/**
				* NTSC only.
				*/
				TUNE_MODE_NTSC,
				/**
				* PAL only.
				*/
				TUNE_MODE_PAL,
				/**
				* Both NTSC and PAL.
				*/
				TUNE_MODE_BOTH
			};

			enum
			{
				CHIP_VRC6 = 0x01,
				CHIP_VRC7 = 0x02,
				CHIP_FDS  = 0x04,
				CHIP_MMC5 = 0x08,
				CHIP_N163 = 0x10,
				CHIP_S5B  = 0x20,
				CHIP_ALL  = 0x3F
			};

			/**
			* Returns the name of the NSF.
			*
			* @return name or empty string if NSF hasn't been loaded
			*/
			const char* GetName() const throw();

			/**
			* Returns the name of the artists.
			*
			* @return artist names or empty string if NSF hasn't been loaded
			*/
			const char* GetArtist() const throw();

			/**
			* Returns the copyright string.
			*
			* @return copyright or empty string if NSF hasn't been loaded
			*/
			const char* GetCopyright() const throw();

			/**
			* Return the tune mode.
			*
			* @return tune mode
			*/
			TuneMode GetMode() const throw();

			/**
			* Returns the init-address.
			*
			* @return address
			*/
			uint GetInitAddress() const throw();

			/**
			* Returns the load-address.
			*
			* @return address
			*/
			uint GetLoadAddress() const throw();

			/**
			* Returns the play-address.
			*
			* @return address
			*/
			uint GetPlayAddress() const throw();

			/**
			* Returns the total number of songs.
			*
			* @return number
			*/
			uint GetNumSongs() const throw();

			/**
			* Returns the current song index.
			*
			* @return song index or NO_SONG if NSF hasn't been loaded
			*/
			int GetCurrentSong() const throw();

			/**
			* Returns the starting song index.
			*
			* @return song index or NO_SONG if NSF hasn't been loaded
			*/
			int GetStartingSong() const throw();

			/**
			* Returns the OR:ed chips in use.
			*
			* @return OR:ed chips used
			*/
			uint GetChips() const throw();

			/**
			* Checks if a song is currently being played.
			*
			* @return true if playing
			*/
			bool IsPlaying() const throw();

			/**
			* Checks if the NSF uses bank-switching.
			*
			* @return true if NSF uses bank-switching
			*/
			bool UsesBankSwitching() const throw();

			/**
			* Selects a song.
			*
			* @param song index
			* @return result code
			*/
			Result SelectSong(uint song) throw();

			/**
			* Selects the next song.
			*
			* @return result code
			*/
			Result SelectNextSong() throw();

			/**
			* Selects the previous song.
			*
			* @return result code
			*/
			Result SelectPrevSong() throw();

			/**
			* Plays current selected song.
			*
			* @return result code
			*/
			Result PlaySong() throw();

			/**
			* Stops current selected song.
			*
			* @return result code
			*/
			Result StopSong() throw();

			/**
			* Event.
			*/
			enum Event
			{
				/**
				* A new song has been selected.
				*/
				EVENT_SELECT_SONG,
				/*
				* Song has started playing.
				*/
				EVENT_PLAY_SONG,
				/**
				* Song has stopped playing.
				*/
				EVENT_STOP_SONG
			};

			enum
			{
				NUM_EVENT_CALLBACKS = 3
			};

			/**
			* Event callback prototype.
			*
			* @param userData optional user data
			* @param event type of event
			*/
			typedef void (NST_CALLBACK *EventCallback) (UserData userData,Event event);

			/**
			* Event callback manager.
			*
			* Static object used for adding the user defined callback.
			*/
			static EventCaller eventCallback;
		};

		/**
		* Song event callback invoker.
		*
		* Used internally by the core.
		*/
		struct Nsf::EventCaller : Core::UserCallback<Nsf::EventCallback>
		{
			void operator () (Event event) const
			{
				if (function)
					function( userdata, event );
			}
		};
	}
}

#if NST_MSVC >= 1200 || NST_ICC >= 810
#pragma warning( pop )
#endif

#endif
