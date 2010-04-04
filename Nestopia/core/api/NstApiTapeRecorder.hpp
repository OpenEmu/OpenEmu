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

#ifndef NST_API_TAPERECORDER_H
#define NST_API_TAPERECORDER_H

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
	namespace Core
	{
		namespace Input
		{
			class FamilyKeyboard;
		}
	}

	namespace Api
	{
		/**
		* Tape interface.
		*/
		class TapeRecorder : public Base
		{
			struct EventCaller;

			Core::Input::FamilyKeyboard* Query() const;

		public:

			/**
			* Interface constructor.
			*
			* @param instance emulator instance
			*/
			template<typename T>
			TapeRecorder(T& instance)
			: Base(instance) {}

			/**
			* Checks if tape is playing.
			*
			* @return true if playing
			*/
			bool IsPlaying() const throw();

			/**
			* Checks if tape is recording.
			*
			* @return true if recording
			*/
			bool IsRecording() const throw();

			/**
			* Checks if tape has stopped playing or recording.
			*
			* @return true if stopped
			*/
			bool IsStopped() const throw();

			/**
			* Checks if tape can be played
			*
			* @return true if playable
			*/
			bool IsPlayable() const throw();

			/**
			* Plays tape.
			*
			* @return result code
			*/
			Result Play() throw();

			/**
			* Records tape.
			*
			* @return result code
			*/
			Result Record() throw();

			/**
			* Stops tape.
			*
			* @return result code
			*/
			Result Stop() throw();

			/**
			* Checks if a tape recorder is connected.
			*
			* @param true connected
			*/
			bool IsConnected() const throw()
			{
				return Query();
			}

			/**
			* Tape events.
			*/
			enum Event
			{
				/**
				* Tape is playing.
				*/
				EVENT_PLAYING,
				/**
				* Tape is recording.
				*/
				EVENT_RECORDING,
				/**
				* Tape has stopped playing or recording.
				*/
				EVENT_STOPPED
			};

			enum
			{
				NUM_EVENT_CALLBACKS = 3
			};

			/**
			* Tape event callback prototype.
			*
			* @param userData optional user data
			* @param event type of event
			*/
			typedef void (NST_CALLBACK *EventCallback) (UserData userData,Event event);

			/**
			* Tape event callback manager.
			*
			* Static object used for adding the user defined callback.
			*/
			static EventCaller eventCallback;
		};

		/**
		* Tape event callback invoker.
		*
		* Used internally by the core.
		*/
		struct TapeRecorder::EventCaller : Core::UserCallback<TapeRecorder::EventCallback>
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
