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

#ifndef NST_API_VIDEO_H
#define NST_API_VIDEO_H

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
		namespace Video
		{
			/**
			* Video output context.
			*/
			class Output
			{
				struct Locker;
				struct Unlocker;

			public:

				enum
				{
					WIDTH = 256,
					HEIGHT = 240,
					NTSC_WIDTH = 602
				};

				/**
				* Pointer to surface memory to be written to. Size must be equal to
				* or greater than bitsPerPixel/8 * NES screen width * NES screen height.
				*/
				void* pixels;

				/**
				* Distance in bytes for each line in the surface memory.
				* Must be equal to or greater than the actual NES screen width.
				* Value is allowed to be negative.
				*/
				long pitch;

				Output(void* v=0,long p=0)
				: pixels(v), pitch(p) {}

				/**
				* Surface lock callback prototype.
				*
				* Called right before the core is about to render to the surface.
				*
				* @param userData optional user data
				* @param output object to this class
				* @return true if surface is valid and can be written to
				*/
				typedef bool (NST_CALLBACK *LockCallback) (void* userData,Output& output);

				/**
				* Surface unlock callback prototype.
				*
				* Called when the core has finished rendering to the surface and a previous locked was made.
				*
				* @param userData optional user data
				* @param output object to this class
				*/
				typedef void (NST_CALLBACK *UnlockCallback) (void* userData,Output& output);

				/**
				* Surface lock callback manager.
				*
				* Static object used for adding the user defined callback.
				*/
				static Locker lockCallback;

				/**
				* Surface unlock callback manager.
				*
				* Static object used for adding the user defined callback.
				*/
				static Unlocker unlockCallback;
			};

			/**
			* Surface lock callback invoker.
			*
			* Used internally by the core.
			*/
			struct Output::Locker : UserCallback<Output::LockCallback>
			{
				bool operator () (Output& output) const
				{
					return (!function || function( userdata, output )) && output.pixels && output.pitch;
				}
			};

			/**
			* Surface unlock callback invoker.
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
		* Video interface.
		*/
		class Video : public Base
		{
		public:

			/**
			* Interface constructor.
			*
			* @param instance emulator instance
			*/
			template<typename T>
			Video(T& instance)
			: Base(instance) {}

			/**
			* Video output context.
			*/
			typedef Core::Video::Output Output;

			enum
			{
				MIN_BRIGHTNESS                  = -100,
				DEFAULT_BRIGHTNESS              =    0,
				MAX_BRIGHTNESS                  = +100,
				MIN_SATURATION                  = -100,
				DEFAULT_SATURATION              =    0,
				MAX_SATURATION                  = +100,
				MIN_CONTRAST                    = -100,
				DEFAULT_CONTRAST                =    0,
				MAX_CONTRAST                    = +100,
				MIN_SHARPNESS                   = -100,
				DEFAULT_SHARPNESS_COMP          =    0,
				DEFAULT_SHARPNESS_SVIDEO        =   20,
				DEFAULT_SHARPNESS_RGB           =   20,
				MAX_SHARPNESS                   = +100,
				MIN_COLOR_RESOLUTION            = -100,
				DEFAULT_COLOR_RESOLUTION_COMP   =    0,
				DEFAULT_COLOR_RESOLUTION_SVIDEO =   20,
				DEFAULT_COLOR_RESOLUTION_RGB    =   70,
				MAX_COLOR_RESOLUTION            = +100,
				MIN_COLOR_BLEED                 = -100,
				DEFAULT_COLOR_BLEED_COMP        =    0,
				DEFAULT_COLOR_BLEED_SVIDEO      =    0,
				DEFAULT_COLOR_BLEED_RGB         = -100,
				MAX_COLOR_BLEED                 = +100,
				MIN_COLOR_ARTIFACTS             = -100,
				DEFAULT_COLOR_ARTIFACTS_COMP    =    0,
				DEFAULT_COLOR_ARTIFACTS_SVIDEO  = -100,
				DEFAULT_COLOR_ARTIFACTS_RGB     = -100,
				MAX_COLOR_ARTIFACTS             = +100,
				MIN_COLOR_FRINGING              = -100,
				DEFAULT_COLOR_FRINGING_COMP     =    0,
				DEFAULT_COLOR_FRINGING_SVIDEO   = -100,
				DEFAULT_COLOR_FRINGING_RGB      = -100,
				MAX_COLOR_FRINGING              = +100,
				MIN_HUE                         =  -45,
				DEFAULT_HUE                     =    0,
				MAX_HUE                         =  +45
			};

			/**
			* Allows the PPU to render more than eight sprites per line.
			*
			* @param state true to allow it, default is false
			* @return result code
			*/
			Result EnableUnlimSprites(bool state) throw();

			/**
			* Checks if the PPU sprite software extension is enabled.
			*
			* @return true if enabled
			*/
			bool AreUnlimSpritesEnabled() const throw();

			/**
			* Returns the current brightness.
			*
			* @return brightness value in the range -100 to 100
			*/
			int GetBrightness() const throw();

			/**
			* Returns the current saturation.
			*
			* @return saturation value in the range -100 to 100
			*/
			int GetSaturation() const throw();

			/**
			* Returns the current contrast.
			*
			* @return contrast value in the range -100 to 100
			*/
			int GetContrast() const throw();

			/**
			* Returns the current sharpness for the NTSC filter.
			*
			* @return sharpness value in the range -100 to 100
			*/
			int GetSharpness() const throw();

			/**
			* Returns the current color resolution for the NTSC filter.
			*
			* @return color resolution value in the range -100 to 100
			*/
			int GetColorResolution() const throw();

			/**
			* Returns the current color bleed for the NTSC filter.
			*
			* @return color bleed value in the range -100 to 100
			*/
			int GetColorBleed() const throw();

			/**
			* Returns the current color artifacts for the NTSC filter.
			*
			* @return color artifacts value in the range -100 to 100
			*/
			int GetColorArtifacts() const throw();

			/**
			* Returns the current color fringing for the NTSC filter.
			*
			* @return color fringing value in the range -100 to 100
			*/
			int GetColorFringing() const throw();

			/**
			* Returns the current hue.
			*
			* @return hue value in the range -45 to 45
			*/
			int GetHue() const throw();

			/**
			* Sets the brightness.
			*
			* @param value brightness in the range -100 to 100, default is 0
			* @return result code
			*/
			Result SetBrightness(int value) throw();

			/**
			* Sets the saturation.
			*
			* @param value saturation in the range -100 to 100, default is 0
			* @return result code
			*/
			Result SetSaturation(int value) throw();

			/**
			* Sets the contrast.
			*
			* @param value contrast in the range -100 to 100, default is 0
			* @return result code
			*/
			Result SetContrast(int value) throw();

			/**
			* Sets the sharpness for the NTSC filter.
			*
			* @param value sharpness in the range -100 to 100, default is 0
			* @return result code
			*/
			Result SetSharpness(int value) throw();

			/**
			* Sets the color resolution for the NTSC filter.
			*
			* @param value color resolution in the range -100 to 100, default is 0
			* @return result code
			*/
			Result SetColorResolution(int value) throw();

			/**
			* Sets the color bleed for the NTSC filter.
			*
			* @param value color bleed in the range -100 to 100, default is 0
			* @return result code
			*/
			Result SetColorBleed(int value) throw();

			/**
			* Sets the color artifacts for the NTSC filter.
			*
			* @param value color artifacts in the range -100 to 100, default is 0
			* @return result code
			*/
			Result SetColorArtifacts(int value) throw();

			/**
			* Sets the color fringing for the NTSC filter.
			*
			* @param value fringing in the range -100 to 100, default is 0
			* @return result code
			*/
			Result SetColorFringing(int value) throw();

			/**
			* Sets the hue.
			*
			* @param value hue in the range -45 to 45, default is 0
			* @return result code
			*/
			Result SetHue(int value) throw();

			/**
			* Enables field merging for the NTSC filter.
			*
			* @param state true to enable
			*/
			void EnableFieldMerging(bool state) throw();

			/**
			* Checks if NTSC filter field merging is enabled.
			*
			* @return true if enabled
			*/
			bool IsFieldMergingEnabled() const throw();

			/**
			* Performs a manual blit to the video output object.
			*
			* The core calls this method internally for each frame.
			*
			* @param output video output object to blit to
			* @return result code
			*/
			Result Blit(Output& output) throw();

			/**
			* YUV decoder presets.
			*/
			enum DecoderPreset
			{
				/**
				* Canonical (default)
				*/
				DECODER_CANONICAL,
				/**
				* Consumer
				*/
				DECODER_CONSUMER,
				/**
				* Alternative
				*/
				DECODER_ALTERNATIVE
			};

			/**
			* YUV decoder context.
			*/
			struct Decoder
			{
				/**
				* Constructor.
				*
				* @param preset preset, canonical by default
				*/
				Decoder(DecoderPreset preset=DECODER_CANONICAL) throw();

				/**
				* Tests for equality.
				*
				* @param decoder object to compare
				* @return true if equal
				*/
				bool operator == (const Decoder& decoder) const throw();

				/**
				* Tests for non-equality.
				*
				* @param decoder object to compare
				* @return true if non-equal
				*/
				bool operator != (const Decoder& decoder) const throw();

				enum
				{
					AXIS_RY,
					AXIS_GY,
					AXIS_BY,
					NUM_AXES
				};

				struct
				{
					float gain;
					uint angle;
				}   axes[NUM_AXES];

				bool boostYellow;
			};

			/**
			* Sets the YUV decoder.
			*
			* @param decoder decoder
			* @return result code
			*/
			Result SetDecoder(const Decoder& decoder) throw();

			/**
			* Returns the current YUV decoder.
			*
			* @return current decoder
			*/
			const Decoder& GetDecoder() const throw();

			/**
			* Palette interface.
			*/
			class Palette
			{
				Core::Machine& emulator;

			public:

				/**
				* Interface constructor
				*
				* @param instance emulator instance
				*/
				Palette(Core::Machine& instance)
				: emulator(instance) {}

				enum
				{
					NUM_ENTRIES = 64,
					NUM_ENTRIES_EXT = 512
				};

				/**
				* Custom palette types.
				*/
				enum CustomType
				{
					/**
					* Standard palette. 64 colors.
					*/
					STD_PALETTE = NUM_ENTRIES,
					/**
					* Extended palette. 512 colors with emphasis included in it.
					*/
					EXT_PALETTE = NUM_ENTRIES_EXT
				};

				/**
				* Palette modes
				*/
				enum Mode
				{
					/**
					* YUV (default)
					*/
					MODE_YUV,
					/**
					* RGB
					*/
					MODE_RGB,
					/**
					* Custom
					*/
					MODE_CUSTOM
				};

				/**
				* RGB colors.
				*/
				typedef const uchar (*Colors)[3];

				/**
				* Returns the current palette mode.
				*
				* @return current mode
				*/
				Mode GetMode() const throw();

				/**
				* Returns the default palette mode.
				*
				* @return default palette mode
				*/
				Mode GetDefaultMode() const throw();

				/**
				* Sets the custom palette.
				*
				* @param colors RGB color data
				* @param type custom palette type
				*/
				Result SetCustom(Colors colors,CustomType type=STD_PALETTE) throw();

				/**
				* Returns the custom palette.
				*
				* @param colors RGB colors to be filled
				* @param type custom palette type
				* @return number of colors written
				*/
				uint GetCustom(uchar (*colors)[3],CustomType type) const throw();

				/**
				* Resets the custom palette.
				*/
				void ResetCustom() throw();

				/**
				* Returns the custom palette type.
				*
				* @return custom palette type
				*/
				CustomType GetCustomType() const throw();

				/**
				* Return the current palette colors.
				*
				* @return palette colors
				*/
				Colors GetColors() const throw();

				/**
				* Sets the palette mode.
				*
				* @param mode palette mode
				* @return result code
				*/
				Result SetMode(Mode mode) throw();
			};

			/**
			* Returns the palette interface.
			*
			* @return palette interface
			*/
			Palette GetPalette()
			{
				return emulator;
			}

			/**
			* Render state context.
			*/
			struct RenderState
			{
				RenderState() throw();

				/**
				* Pixel context.
				*/
				struct Bits
				{
					/**
					* RGB bit mask.
					*/
					struct Mask
					{
						ulong r,g,b;
					};

					/**
					* RGB bit mask.
					*/
					Mask mask;

					/**
					* Bits per pixel.
					*/
					uint count;
				};

				/**
				* Pixel context.
				*/
				Bits bits;

				/**
				* Screen width.
				*/
				ushort width;

				/**
				* Screen height.
				*/
				ushort height;

				/**
				* Video Filter.
				*/
				enum Filter
				{
					/**
					* No filter (default).
					*/
					FILTER_NONE,
					/**
					* NTSC filter.
					*/
					FILTER_NTSC
				#ifndef NST_NO_SCALEX
					,
					/**
					* Scale2x filter.
					*/
					FILTER_SCALE2X,
					/**
					* Scale3x filter.
					*/
					FILTER_SCALE3X
				#endif
				#ifndef NST_NO_HQ2X
					,
					/**
					* Hq2x filter.
					*/
					FILTER_HQ2X,
					/**
					* Hq3x filter.
					*/
					FILTER_HQ3X,
					/**
					* Hq4x filter.
					*/
					FILTER_HQ4X
				#endif
				#ifndef NST_NO_2XSAI
					,
					/**
					* 2xSaI filter.
					*/
					FILTER_2XSAI
				#endif
				};

				/**
				* Scale factors.
				*/
				enum Scale
				{
					SCALE_NONE = 1
				#ifndef NST_NO_SCALEX
					,SCALE_SCALE2X = 2
					,SCALE_SCALE3X = 3
				#endif
				#ifndef NST_NO_HQ2X
					,SCALE_HQ2X = 2
					,SCALE_HQ3X = 3
				#endif
				#ifndef NST_NO_2XSAI
					,SCALE_2XSAI = 2
				#endif
				};

				/**
				* Filter.
				*/
				Filter filter;
			};

			/**
			* Sets the render state.
			*
			* @param state render state to be set
			* @return result code
			*/
			Result SetRenderState(const RenderState& state) throw();

			/**
			* Returns the current render state.
			*
			* @param state object to be filled
			* @return result code
			*/
			Result GetRenderState(RenderState& state) const throw();
		};
	}
}

#if NST_MSVC >= 1200 || NST_ICC >= 810
#pragma warning( pop )
#endif

#endif
