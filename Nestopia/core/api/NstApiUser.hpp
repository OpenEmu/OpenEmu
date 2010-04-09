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

#ifndef NST_API_USER_H
#define NST_API_USER_H

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
		* User IO interfaces.
		*/
		class User : public Base
		{
			struct LogCaller;
			struct EventCaller;
			struct QuestionCaller;
			struct FileIoCaller;

		public:

			/**
			* Interface constructor.
			*
			* @param instance emulator instance
			*/
			template<typename T>
			User(T& instance)
			: Base(instance) {}

			/**
			* User questions.
			*/
			enum Question
			{
				/**
				* Whether to proceed or abort if CRC validation fails when loading a save state.
				*/
				QUESTION_NST_PRG_CRC_FAIL_CONTINUE = 1,
				/**
				* Whether to proceed or abort if CRC validation fails when playing a move.
				*/
				QUESTION_NSV_PRG_CRC_FAIL_CONTINUE
			};

			/**
			* User answer.
			*/
			enum Answer
			{
				/**
				* No.
				*/
				ANSWER_NO,
				/**
				* Yes.
				*/
				ANSWER_YES,
				/**
				* Default answer (default).
				*/
				ANSWER_DEFAULT
			};

			/**
			* User events.
			*/
			enum Event
			{
				/**
				* CPU jam.
				*/
				EVENT_CPU_JAM = 1,
				/**
				* Can display an in-game timer.
				*/
				EVENT_DISPLAY_TIMER,
				/**
				* An unofficial CPU opcode was executed.
				*/
				EVENT_CPU_UNOFFICIAL_OPCODE
			};

			/**
			* File IO interface.
			*/
			struct File
			{
				/**
				* Action event.
				*/
				enum Action
				{
					/**
					* For loading battery-backed RAM into a cartridge.
					*/
					LOAD_BATTERY = 1,
					/**
					* For saving the battery-backed RAM in a cartridge.
					*/
					SAVE_BATTERY,
					/**
					* For patching a Famicom Disk System image.
					*/
					LOAD_FDS,
					/**
					* For saving a modified Famicom Disk System image to patch or directly to image.
					*/
					SAVE_FDS,
					/**
					* For loading EEPROM into a cartridge.
					*/
					LOAD_EEPROM,
					/**
					* For saving the EEPROM in a cartridge.
					*/
					SAVE_EEPROM,
					/**
					* For loading cassette tape recording.
					*/
					LOAD_TAPE,
					/**
					* For saving the cassette tape recording.
					*/
					SAVE_TAPE,
					/**
					* For loading Turbo File device data.
					*/
					LOAD_TURBOFILE,
					/**
					* For saving Turbo File device data.
					*/
					SAVE_TURBOFILE,
					/**
					* For loading ROM into a cartridge.
					*/
					LOAD_ROM,
					/**
					* For loading raw PCM audio samples.
					*/
					LOAD_SAMPLE,
					/**
					* For loading raw PCM audio samples used in Moero Pro Yakyuu.
					*/
					LOAD_SAMPLE_MOERO_PRO_YAKYUU,
					/**
					* For loading raw PCM audio samples used in Moero Pro Yakyuu 88.
					*/
					LOAD_SAMPLE_MOERO_PRO_YAKYUU_88,
					/**
					* For loading raw PCM audio samples used in Moero Pro Tennis.
					*/
					LOAD_SAMPLE_MOERO_PRO_TENNIS,
					/**
					* For loading raw PCM audio samples used in Terao No Dosukoi Oozumou.
					*/
					LOAD_SAMPLE_TERAO_NO_DOSUKOI_OOZUMOU,
					/**
					* For loading raw PCM audio samples used in Aerobics Studio.
					*/
					LOAD_SAMPLE_AEROBICS_STUDIO
				};

				/**
				* Supported patch formats.
				*/
				enum Patch
				{
					/**
					* UPS.
					*/
					PATCH_UPS,
					/**
					* IPS.
					*/
					PATCH_IPS
				};

				/**
				* Returns type of action.
				*
				* @return action
				*/
				virtual Action GetAction() const throw() = 0;

				/**
				* Returns the name of the file to load.
				*
				* Used only with the LOAD_ROM and LOAD_SAMPLE action callbacks.
				*
				* @return filename
				*/
				virtual const wchar_t* GetName() const throw();

				/**
				* Returns the sound file ID to load.
				*
				* Used only with the LOAD_SAMPLE_xx action callbacks.
				*
				* @return sample id
				*/
				virtual uint GetId() const throw();

				/**
				* Returns the maximum allowed size for the content to load.
				*
				* Used only with the LOAD_xx action callbacks.
				*
				* @return max size
				*/
				virtual ulong GetMaxSize() const throw();

				/**
				* Saves the content into an output stream.
				*
				* Used only with the SAVE_xx action callbacks.
				*
				* @param stream output stream
				* @param result code
				*/
				virtual Result GetContent(std::ostream& stream) const throw();

				/**
				* Returns a pointer to the content to be saved and its size.
				*
				* Used only with the SAVE_xx action callbacks.
				*
				* @param mem pointer to content
				* @param size size of content
				* @param result code
				*/
				virtual Result GetContent(const void*& mem,ulong& size) const throw();

				/**
				* Saves the patch content into an output stream.
				*
				* Used only with the FDS_SAVE action callback.
				*
				* @param patch patch format to use
				* @param stream output stream
				*/
				virtual Result GetPatchContent(Patch patch,std::ostream& stream) const throw();

				/**
				* Loads content into the core through stream.
				*
				* Used only with the LOAD_xx action callbacks.
				* This method can't be used for audio or patch content. Instead, use LoadSampleContent(..)
				* and SetPatchContent(..) for those.
				*
				* @param stream input stream
				* @return result code
				*/
				virtual Result SetContent(std::istream& stream) throw();

				/**
				* Loads content into the core.
				*
				* Used only with the LOAD_xx action callbacks.
				* This method can't be used for audio or patch content. Instead, use LoadSampleContent(..)
				* and SetPatchContent(..) for those.
				*
				* @param mem content
				* @param size size of content
				* @return result code
				*/
				virtual Result SetContent(const void* mem,ulong size) throw();

				/**
				* Loads patch content into the core.
				*
				* Used only with LOAD_FDS action callback.
				*
				* @param stream input stream to patch
				* @return result code
				*/
				virtual Result SetPatchContent(std::istream& stream) throw();

				/**
				* Loads audio content into the core.
				*
				* Used only with the LOAD_SAMPLE and LOAD_SAMPLE_xx action callbacks.
				*
				* @param mem sample content
				* @param length number of samples
				* @param stereo dual channel if true
				* @param bits bits per sample
				* @param rate sample rate
				* @return result code
				*/
				virtual Result SetSampleContent(const void* mem,ulong length,bool stereo,uint bits,ulong rate) throw();
			};

			enum
			{
				NUM_QUESTION_CALLBACKS = 2,
				NUM_EVENT_CALLBACKS = 3,
				NUM_FILE_CALLBACKS = 17
			};

			/**
			* Logfile callback prototype.
			*
			* @param userData optional user data
			* @param string string content
			* @param length string length
			*/
			typedef void (NST_CALLBACK *LogCallback) (UserData userData,const char* string,ulong length);

			/**
			* Logfile callback prototype.
			*
			* @param userData optional user data
			* @param event type of event
			* @param context context depending on event
			*/
			typedef void (NST_CALLBACK *EventCallback) (UserData userData,Event event,const void* context);

			/**
			* User question callback prototype.
			*
			* @param userData optional user data
			* @param question type of question
			* @return user answer
			*/
			typedef Answer (NST_CALLBACK *QuestionCallback) (UserData userData,Question question);

			/**
			* File IO callback prototype.
			*
			* @param userData optional user data
			* @param file File IO interface
			*/
			typedef void (NST_CALLBACK *FileIoCallback) (UserData userData,File& file);

			/**
			* Logfile callback manager.
			*
			* Static object used for adding the user defined callback.
			*/
			static LogCaller logCallback;

			/**
			* User event callback manager.
			*
			* Static object used for adding the user defined callback.
			*/
			static EventCaller eventCallback;

			/**
			* User question callback manager.
			*
			* Static object used for adding the user defined callback.
			*/
			static QuestionCaller questionCallback;

			/**
			* File IO callback manager.
			*
			* Static object used for adding the user defined callback.
			*/
			static FileIoCaller fileIoCallback;
		};

		/**
		* Logfile callback invoker.
		*
		* Used internally by the core.
		*/
		struct User::LogCaller : Core::UserCallback<User::LogCallback>
		{
			void operator () (const char* text,ulong length) const
			{
				if (function)
					function( userdata, text, length );
			}

			template<ulong N>
			void operator () (const char (&c)[N]) const
			{
				(*this)( c, N-1 );
			}
		};

		/**
		* User event callback invoker.
		*
		* Used internally by the core.
		*/
		struct User::EventCaller : Core::UserCallback<User::EventCallback>
		{
			void operator () (Event event,const void* data=0) const
			{
				if (function)
					function( userdata, event, data );
			}
		};

		/**
		* User question callback invoker.
		*
		* Used internally by the core.
		*/
		struct User::QuestionCaller : Core::UserCallback<User::QuestionCallback>
		{
			Answer operator () (Question question) const
			{
				return function ? function( userdata, question ) : ANSWER_DEFAULT;
			}
		};

		/**
		* File IO callback invoker.
		*
		* Used internally by the core.
		*/
		struct User::FileIoCaller : Core::UserCallback<User::FileIoCallback>
		{
			void operator () (File& file) const
			{
				if (function)
					function( userdata, file );
			}
		};
	}
}

#if NST_MSVC >= 1200 || NST_ICC >= 810
#pragma warning( pop )
#endif

#endif
