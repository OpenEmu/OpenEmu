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

#ifndef NST_INPUT_FAMILYKEYBOARD_H
#define NST_INPUT_FAMILYKEYBOARD_H

#ifdef NST_PRAGMA_ONCE
#pragma once
#endif

namespace Nes
{
	namespace Core
	{
		namespace Input
		{
			class FamilyKeyboard : public Device
			{
			public:

				explicit FamilyKeyboard(Cpu&,bool);

				Result PlayTape();
				Result RecordTape();
				Result StopTape();

				bool IsTapeRecording() const;
				bool IsTapePlaying() const;
				bool IsTapePlayable() const;
				bool IsTapeStopped() const;

			private:

				class DataRecorder;

				~FamilyKeyboard();

				void Reset();
				void Poke(uint);
				uint Peek(uint);
				void EndFrame();
				void LoadState(State::Loader&,dword);
				void SaveState(State::Saver&,byte) const;

				enum
				{
					COMMAND_RESET = 0x01,
					COMMAND_SCAN  = 0x02,
					COMMAND_KEY   = 0x04
				};

				uint mode;
				uint scan;
				DataRecorder* const dataRecorder;
			};
		}
	}
}

#endif
