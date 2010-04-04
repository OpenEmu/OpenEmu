////////////////////////////////////////////////////////////////////////////////////////
// Josh Weinberg
////////////////////////////////////////////////////////////////////////////////////////

#ifndef NST_API_RAM_H
#define NST_API_RAM_H

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
		class Ram : public Base
			{
				
			public:
				
				/**
				 * Interface constructor.
				 *
				 * @param instance emulator instance
				 */
				template<typename T>
				Ram(T& instance)
				: Base(instance) {}
				
				/**
				 * Returns the current sharpness for the NTSC filter.
				 *
				 * @return sharpness value in the range -100 to 100
				 */
				void SetCHR(int page, int offset, int value)  throw();
				void SetRAM(int offset, int value) throw();
				void SetNmtRam(int page, int offset, int value) throw();
			};
		
	}
}

#if NST_MSVC >= 1200 || NST_ICC >= 810
#pragma warning( pop )
#endif

#endif
