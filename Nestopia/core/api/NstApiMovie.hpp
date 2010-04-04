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

#ifndef NST_API_MOVIE_H
#define NST_API_MOVIE_H

#include <iosfwd>
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
		* Movie playing/recording interface.
		*/
		class Movie : public Base
		{
			struct EventCaller;

		public:

			/**
			* Interface constructor.
			*
			* @param instance emulator instance
			*/
			template<typename T>
			Movie(T& instance)
			: Base(instance) {}

			/**
			* Recording procedure.
			*/
			enum How
			{
				/**
				* Overwrite any previous content.
				*/
				CLEAN,
				/**
				* Keep any previous content.
				*/
				APPEND
			};

			/**
			* Plays movie.
			*
			* @param stream input stream to movie
			* @return result code
			*/
			Result Play(std::istream& stream) throw();

			/**
			* Records movie.
			*
			* @param stream stream to record movie to
			* @param how CLEAN to erase any previous content, APPEND to keep content, default is CLEAN
			* @return result code
			*/
			Result Record(std::iostream& stream,How how=CLEAN) throw();

			/**
			* Stops movie.
			*/
			void Stop() throw();

			/**
			* Ejects movie.
			*
			* @deprecated
			*/
			void Eject() {}

			/**
			* Checks if a movie is being played.
			*
			* @return true if playing
			*/
			bool IsPlaying() const throw();

			/**
			* Checks if a movie is being recorded.
			*
			* @return true if recording
			*/
			bool IsRecording() const throw();

			/**
			* Checks if a movie has stopped playing or recording.
			*
			* @return true if stopped
			*/
			bool IsStopped() const throw();

			/**
			* Movie event.
			*/
			enum Event
			{
				/**
				* Movie has started playing.
				*/
				EVENT_PLAYING,
				/**
				* Movie has stopped playing.
				*/
				EVENT_PLAYING_STOPPED,
				/**
				* Movie has started recording.
				*/
				EVENT_RECORDING,
				/**
				* Movie has stopped recording.
				*/
				EVENT_RECORDING_STOPPED
			};

			enum
			{
				NUM_EVENT_CALLBACKS = 4
			};

			/**
			* Movie event callback prototype.
			*
			* @param userData optional user data
			* @param event type of event
			* @param result result code of event
			*/
			typedef void (NST_CALLBACK *EventCallback) (UserData userData,Event event,Result result);

			/**
			* Movie event callback manager.
			*
			* Static object used for adding the user defined callback.
			*/
			static EventCaller eventCallback;
		};

		/**
		* Movie event callback invoker.
		*
		* Used internally by the core.
		*/
		struct Movie::EventCaller : Core::UserCallback<Movie::EventCallback>
		{
			void operator () (Event event,Result result=RESULT_OK) const
			{
				if (function)
					function( userdata, event, result );
			}
		};
	}
}

#if NST_MSVC >= 1200 || NST_ICC >= 810
#pragma warning( pop )
#endif

#endif
