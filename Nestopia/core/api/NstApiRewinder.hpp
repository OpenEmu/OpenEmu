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

#ifndef NST_API_REWINDER_H
#define NST_API_REWINDER_H

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
		* Game rewinder interface.
		*/
		class Rewinder : public Base
		{
			struct StateCaller;

		public:

			/**
			* Interface constructor.
			*
			* @param instance emulator instance
			*/
			template<typename T>
			Rewinder(T& instance)
			: Base(instance) {}

			/**
			* Direction.
			*/
			enum Direction
			{
				/**
				* Forward.
				*/
				FORWARD,
				/**
				* Backward.
				*/
				BACKWARD
			};

			/**
			* Enables rewinder.
			*
			* @param state true to enable
			* @return result code
			*/
			Result Enable(bool state=true) throw();

			/**
			* Checks if rewinder is enabled.
			*
			* @return true if enabled
			*/
			bool IsEnabled() const throw();

			/**
			* Resets rewinder.
			*/
			void Reset() throw();

			/**
			* Enables backward sound.
			*
			* @param state true to enable
			*/
			void EnableSound(bool state=true) throw();

			/**
			* Checks if backward sound is enabled.
			*
			* @return true if enabled
			*/
			bool IsSoundEnabled() const throw();

			/**
			* Sets direction.
			*
			* @param direction direction, FORWARD or BACKWARD
			* @return result code
			*/
			Result SetDirection(Direction direction) throw();

			/**
			* Returns the current direction.
			*
			* @return current direction
			*/
			Direction GetDirection() const throw();

			/**
			* Rewinder state.
			*/
			enum State
			{
				/**
				* Rewinding has stopped.
				*/
				STOPPED,
				/**
				* Rewinding will soon start.
				*/
				PREPARING,
				/**
				* Rewinding has begun.
				*/
				REWINDING
			};

			enum
			{
				NUM_STATE_CALLBACKS = 3
			};

			/**
			* Rewinder state callback prototype.
			*
			* @param userData optional user data
			* @param state type of state
			*/
			typedef void (NST_CALLBACK *StateCallback) (UserData userData,State state);

			/**
			* Rewinder state callback manager.
			*
			* Static object used for adding the user defined callback.
			*/
			static StateCaller stateCallback;
		};

		/**
		* Rewinder state callback invoker.
		*
		* Used internally by the core.
		*/
		struct Rewinder::StateCaller : Core::UserCallback<Rewinder::StateCallback>
		{
			void operator () (State state) const
			{
				if (function)
					function( userdata, state );
			}
		};
	}
}

#if NST_MSVC >= 1200 || NST_ICC >= 810
#pragma warning( pop )
#endif

#endif
