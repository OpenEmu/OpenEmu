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

#ifndef NST_API_MACHINE_H
#define NST_API_MACHINE_H

#include <iosfwd>
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
		* Machine interface.
		*/
		class Machine : public Base
		{
			struct EventCaller;

		public:

			/**
			* Interface constructor.
			*
			* @param instance emulator instance
			*/
			template<typename T>
			Machine(T& instance)
			: Base(instance) {}

			enum
			{
				ON        = 0x001,
				VS        = 0x010,
				PC10      = 0x020,
				CARTRIDGE = 0x040,
				DISK      = 0x080,
				SOUND     = 0x100,
				GAME      = CARTRIDGE|DISK,
				IMAGE     = GAME|SOUND
			};

			/**
			* NTSC/PAL mode.
			*/
			enum Mode
			{
				/**
				* NTSC.
				*/
				NTSC = 0x04,
				/**
				* PAL.
				*/
				PAL = 0x08
			};

			/**
			* Favored System.
			*
			* Used for telling what console to emulate if the core can't decide by itself.
			*/
			enum FavoredSystem
			{
				/**
				* NES NTSC.
				*/
				FAVORED_NES_NTSC = Core::FAVORED_NES_NTSC,
				/**
				* NES PAL.
				*/
				FAVORED_NES_PAL = Core::FAVORED_NES_PAL,
				/**
				* Famicom.
				*/
				FAVORED_FAMICOM = Core::FAVORED_FAMICOM
			};

			/**
			* Image profile questioning state.
			*
			* Used for allowing callback triggering if an image has multiple media profiles.
			*/
			enum AskProfile
			{
				/**
				* Don't trigger callback (default).
				*/
				DONT_ASK_PROFILE,
				/**
				* Trigger callback.
				*/
				ASK_PROFILE
			};

			enum
			{
				CLK_NTSC_DOT   = Core::CLK_NTSC,
				CLK_NTSC_DIV   = Core::CLK_NTSC_DIV,
				CLK_NTSC_VSYNC = Core::PPU_RP2C02_HVSYNC * ulong(Core::CLK_NTSC_DIV),
				CLK_PAL_DOT    = Core::CLK_PAL,
				CLK_PAL_DIV    = Core::CLK_PAL_DIV,
				CLK_PAL_VSYNC  = Core::PPU_RP2C07_HVSYNC * ulong(Core::CLK_PAL_DIV)
			};

			/**
			* Soft-patching context object.
			*
			* Used as input parameter to some of the image loading methods.
			*/
			struct Patch
			{
				/**
				* Input stream containing the patch in UPS or IPS format.
				*/
				std::istream& stream;

				/**
				* Set to true to bypass checksum validation.
				*/
				bool bypassChecksum;

				/**
				* Will contain the result of the operation after the image has been loaded.
				*/
				Result result;

				/**
				* Constructor.
				*
				* @param s input stream
				* @param b true to bypass checksum validation, default is false
				*/
				Patch(std::istream& s,bool b=false)
				: stream(s), bypassChecksum(b), result(RESULT_NOP) {}
			};

			/**
			* Loads any image. Input stream can be in XML, iNES, UNIF, FDS or NSF format.
			*
			* @param stream input stream containing the image to load
			* @param system console to emulate if the core can't do automatic detection
			* @param askProfile to allow callback triggering if the image has multiple media profiles, default is false
			* @return result code
			*/
			Result Load(std::istream& stream,FavoredSystem system,AskProfile askProfile=DONT_ASK_PROFILE) throw();

			/**
			* Loads any image. Input stream can be in XML, iNES, UNIF, FDS or NSF format.
			*
			* @param stream input stream containing the image to load
			* @param system console to emulate if the core can't do automatic detection
			* @param patch object for performing soft-patching on the image
			* @param askProfile to allow callback triggering if the image has multiple media profiles, default is false
			* @return result code
			*/
			Result Load(std::istream& stream,FavoredSystem system,Patch& patch,AskProfile askProfile=DONT_ASK_PROFILE) throw();

			/**
			* Loads a cartridge image. Input stream can be in XML, iNES or UNIF format.
			*
			* @param stream input stream containing the image to load
			* @param system console to emulate if the core can't do automatic detection
			* @param askProfile to allow callback triggering if the image has multiple media profiles, default is false
			* @return result code
			*/
			Result LoadCartridge(std::istream& stream,FavoredSystem system,AskProfile askProfile=DONT_ASK_PROFILE) throw();

			/**
			* Loads a cartridge image. Input stream can be in XML, iNES or UNIF format.
			*
			* @param stream input stream containing the image to load
			* @param system console to emulate if the core can't do automatic detection
			* @param patch object for performing soft-patching on the image
			* @param askProfile to allow callback triggering if the image has multiple media profiles, default is false
			* @return result code
			*/
			Result LoadCartridge(std::istream& stream,FavoredSystem system,Patch& patch,AskProfile askProfile=DONT_ASK_PROFILE) throw();

			/**
			* Loads a Famicom Disk System image. Input stream can be in FDS format.
			*
			* @param stream input stream containing the image to load
			* @param system console to emulate if the core can't do automatic detection
			* @return result code
			*/
			Result LoadDisk(std::istream& stream,FavoredSystem system) throw();

			/**
			* Loads a sound image. Input stream can be in NSF format.
			*
			* @param stream input stream containing the image to load
			* @param system console to emulate if the core can't do automatic detection
			* @return result code
			*/
			Result LoadSound(std::istream& stream,FavoredSystem system) throw();

			/**
			* Unloads the current image.
			*
			* @return result code
			*/
			Result Unload() throw();

			/**
			* Powers ON or OFF the machine.
			*
			* @param state ON if true
			* @return result code
			*/
			Result Power(bool state) throw();

			/**
			* Resets the machine.
			*
			* @param state hard-reset if true, soft-reset otherwise
			* @return result code
			*/
			Result Reset(bool state) throw();

			/**
			* Returns the current mode.
			*
			* @return mode
			*/
			Mode GetMode() const throw();

			/**
			* Returns the mode most appropriate for the current image.
			*
			* @return mode
			*/
			Mode GetDesiredMode() const throw();

			/**
			* Sets the mode.
			*
			* @param mode new mode
			* @return result code
			*/
			Result SetMode (Mode mode) throw();

			void PokeNmt(uint address, uint data) throw();
			void PokeChr(uint address, uint data) throw();
			
			/**
			* Internal compression on states.
			*/
			enum Compression
			{
				/**
				* No compression.
				*/
				NO_COMPRESSION,
				/**
				* Compression enabled (default).
				*/
				USE_COMPRESSION
			};

			/**
			* Loads a state.
			*
			* @param stream input stream containing the state
			* @return result code
			*/
			Result LoadState (std::istream& stream) throw();

			/**
			* Saves a state.
			*
			* @param stream output stream which the state will be written to
			* @param compression to allow internal compression in the state, default is USE_COMPRESSION
			* @return result code
			*/
			Result SaveState (std::ostream& stream,Compression compression=USE_COMPRESSION) const throw();

			/**
			* Returns a machine state.
			*
			* @param flags OR:ed flags to check
			* @return OR:ed flags evaluated to true
			*/
			uint Is (uint flags) const throw();

			/**
			* Returns a machine state.
			*
			* @param flags1 OR:ed flags to check
			* @param flags2 OR:ed flags to check
			* @return true if <b>both</b> parameters has at least one flag evaluated to true
			*/
			bool Is (uint flags1,uint flags2) const throw();

			/**
			* Tells if the machine is in a locked state.
			*
			* A locked state means that the machine can't respond to
			* certain operations because it's doing something special,
			* like playing a movie or rewinding.
			*
			* @return true if machine is in a locked state.
			*/
			bool IsLocked() const;

			/**
			* Machine events.
			*/
			enum Event
			{
				/**
				* A new image has been loaded into the system.
				*/
				EVENT_LOAD,
				/**
				* An image has been unloaded from the system.
				*/
				EVENT_UNLOAD,
				/**
				* Machine power ON.
				*/
				EVENT_POWER_ON,
				/**
				* Machine power OFF.
				*/
				EVENT_POWER_OFF,
				/**
				* Machine soft-reset.
				*/
				EVENT_RESET_SOFT,
				/**
				* Machine hard-reset.
				*/
				EVENT_RESET_HARD,
				/**
				* Mode has changed to NTSC.
				*/
				EVENT_MODE_NTSC,
				/**
				* Mode has changed to PAL.
				*/
				EVENT_MODE_PAL
			};

			enum
			{
				NUM_EVENT_CALLBACKS = 8
			};

			/**
			* Machine event callback prototype.
			*
			* @param userData optional user data
			* @param event the event
			* @param result result code of event operation, in success or warning state
			*/
			typedef void (NST_CALLBACK *EventCallback) (UserData userData,Event event,Result result);

			/**
			* Machine event callback manager.
			*
			* Static object used for adding the user defined callback.
			*/
			static EventCaller eventCallback;

		private:

			Result Load(std::istream&,FavoredSystem,AskProfile,Patch*,uint);
		};

		/**
		* Machine event callback invoker.
		*
		* Used internally by the core.
		*/
		struct Machine::EventCaller : Core::UserCallback<Machine::EventCallback>
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
