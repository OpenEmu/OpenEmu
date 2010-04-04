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

#ifndef NST_API_SOUND_H
#define NST_API_SOUND_H

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
	namespace Core
	{
		namespace Sound
		{
			/**
			* Sound output context.
			*/
			class Output
			{
				struct Locker;
				struct Unlocker;

			public:

				enum
				{
					MAX_LENGTH = 0x8000
				};

				/**
				* Pointer to sound memory to be written to.
				*
				* Assign NULL to samples[1] if circular buffers aren't needed.
				*/
				void* samples[2];

				/**
				* Length in <b>number of</b> samples for one frame.
				*
				* Assign 0 to length[1] if circular buffers aren't needed.
				* Length doesn't neccesarily need to be the same value for every frame as long
				* as they eventually add up in relation to the emulation speed. The requested
				* number of samples will always be written even if the length is greater
				* than what the sound engine normally produces. Non-written samples for one frame will
				* be carried over to the next through an internal buffer.
				*/
				uint length[2];

				Output(void* s0=0,uint l0=0,void* s1=0,uint l1=0)
				{
					samples[0] = s0;
					samples[1] = s1;
					length[0] = l0;
					length[1] = l1;
				}

				/**
				* Sound lock callback prototype.
				*
				* Called right before the core is about to render sound for one frame. Non-written
				* samples will be saved for the next frame.
				*
				* @param userData optional user data
				* @param output object to this class
				* @return true if output memory is valid and samples can be written to it
				*/
				typedef bool (NST_CALLBACK *LockCallback) (void* userData,Output& output);

				/**
				* Sound unlock callback prototype.
				*
				* Called when the core has finished rendering sound for one frame and a previously lock was made.
				*
				* @param userData optional user data
				* @param output object to this class
				*/
				typedef void (NST_CALLBACK *UnlockCallback) (void* userData,Output& output);

				/**
				* Sound lock callback manager.
				*
				* Static object used for adding the user defined callback.
				*/
				static Locker lockCallback;

				/**
				* Sound unlock callback manager.
				*
				* Static object used for adding the user defined callback.
				*/
				static Unlocker unlockCallback;
			};

			/**
			* Sound lock callback invoker.
			*
			* Used internally by the core.
			*/
			struct Output::Locker : UserCallback<Output::LockCallback>
			{
				bool operator () (Output& output) const
				{
					return (!function || function( userdata, output ));
				}
			};

			/**
			* Sound unlock callback invoker.
			*
			* Used internally by the core.
			*/
			struct Output::Unlocker : UserCallback<Output::UnlockCallback>
			{
				void operator () (Output& output) const
				{
					if (function)
						function( userdata, output );
				}
			};
		}
	}

	namespace Api
	{
		/**
		* Sound interface.
		*/
		class Sound : public Base
		{
		public:

			/**
			* Interface constructor.
			*
			* @param instance emulator instance
			*/
			template<typename T>
			Sound(T& instance)
			: Base(instance) {}

			/**
			* Sound channel types.
			*/
			enum Channel
			{
				/**
				* First square channel.
				*/
				CHANNEL_SQUARE1 = 0x001,
				/**
				* Second square channel.
				*/
				CHANNEL_SQUARE2 = 0x002,
				/**
				* Triangle channel.
				*/
				CHANNEL_TRIANGLE = 0x004,
				/**
				* Noise channel.
				*/
				CHANNEL_NOISE = 0x008,
				/**
				* DPCM channel.
				*/
				CHANNEL_DPCM = 0x010,
				/**
				* FDS sound chip channel.
				*/
				CHANNEL_FDS = 0x020,
				/**
				* MMC5 sound chip channel.
				*/
				CHANNEL_MMC5 = 0x040,
				/**
				* Konami VRC6 sound chip channel.
				*/
				CHANNEL_VRC6 = 0x080,
				/**
				* Konami VRC7 sound chip channel.
				*/
				CHANNEL_VRC7 = 0x100,
				/**
				* Namcot 163 sound chip channel.
				*/
				CHANNEL_N163 = 0x200,
				/**
				* Sunsoft 5B sound chip channel.
				*/
				CHANNEL_S5B = 0x400,
				/**
				* All NES APU channels.
				*/
				APU_CHANNELS = CHANNEL_SQUARE1|CHANNEL_SQUARE2|CHANNEL_TRIANGLE|CHANNEL_NOISE|CHANNEL_DPCM,
				/**
				* All external sound chip channels.
				*/
				EXT_CHANNELS = CHANNEL_FDS|CHANNEL_MMC5|CHANNEL_VRC6|CHANNEL_VRC7|CHANNEL_N163|CHANNEL_S5B,
				/**
				* All channels.
				*/
				ALL_CHANNELS = APU_CHANNELS|EXT_CHANNELS
			};

			/**
			* Speaker type.
			*/
			enum Speaker
			{
				/**
				* Mono sound (default).
				*/
				SPEAKER_MONO,
				/**
				* Pseudo stereo sound.
				*/
				SPEAKER_STEREO
			};

			enum
			{
				DEFAULT_VOLUME = 85,
				MAX_VOLUME = 100,
				DEFAULT_SPEED = 0,
				MIN_SPEED = 30,
				MAX_SPEED = 240
			};

			/**
			* Sets the sample rate.
			*
			* @param rate value in the range 11025 to 96000, default is 44100
			* @return result code
			*/
			Result SetSampleRate(ulong rate) throw();

			/**
			* Returns the sample rate.
			*
			* @return sample rate
			*/
			ulong GetSampleRate() const throw();

			/**
			* Sets the sample bits.
			*
			* @param bits value of 8 or 16, default is 16
			* @return result code
			*/
			Result SetSampleBits(uint bits) throw();

			/**
			* Returns the sample bits.
			*
			* @return number
			*/
			uint GetSampleBits() const throw();

			/**
			* Sets the speaker type.
			*
			* @param speaker speaker type, default is SPEAKER_MONO
			*/
			void SetSpeaker(Speaker speaker) throw();

			/**
			* Returns the speaker type.
			*
			* @return speaker type
			*/
			Speaker GetSpeaker() const throw();

			/**
			* Sets a channel volume.
			*
			* @param channel channel
			* @param volume volume in the range 0 to 100, default is 85
			* @return result code
			*/
			Result SetVolume(uint channel,uint volume) throw();

			/**
			* Returns the volume of a channel.
			*
			* @param channel channel
			* @return volume
			*/
			uint GetVolume(uint channel) const throw();

			/**
			* Sets the speed.
			*
			* @param speed speed in the range 30 to 240, set to DEFAULT_SPEED for automatic adjustment
			* @return result code
			*/
			Result SetSpeed(uint speed) throw();

			/**
			* Returns the current speed.
			*
			* @return speed
			*/
			uint GetSpeed() const throw();

			/**
			* Enables automatic transposition.
			*
			* @param state true to enable
			*/
			void SetAutoTranspose(bool state) throw();

			/**
			* Checks if automatic transposing is enabled.
			*
			* @return true if enabled
			*/
			bool IsAutoTransposing() const throw();

			/**
			* Checks if sound is audible at all.
			*
			* @return true if audible
			*/
			bool IsAudible() const throw();

			/**
			* Empties the internal sound buffer.
			*/
			void EmptyBuffer() throw();

			/**
			* Sound output context.
			*/
			typedef Core::Sound::Output Output;
		};
	}
}

#if NST_MSVC >= 1200 || NST_ICC >= 810
#pragma warning( pop )
#endif

#endif
