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

#include "NstInpDevice.hpp"
#include "NstInpHoriTrack.hpp"

namespace Nes
{
	namespace Core
	{
		namespace Input
		{
			#ifdef NST_MSVC_OPTIMIZE
			#pragma optimize("s", on)
			#endif

			HoriTrack::HoriTrack(const Cpu& c)
			: Device(c,Api::Input::HORITRACK)
			{
				HoriTrack::Reset();
			}

			void HoriTrack::Reset()
			{
				strobe = 0;
				stream = 0;
				state = 0xFF00U|CONNECTED;
			}

			void HoriTrack::SaveState(State::Saver& saver,const byte id) const
			{
				saver.Begin( AsciiId<'H','T'>::R(0,0,id) ).Write8( strobe ).Write32( stream ).End();
			}

			void HoriTrack::LoadState(State::Loader& loader,const dword id)
			{
				if (id == AsciiId<'H','T'>::V)
				{
					strobe = loader.Read8() & 0x1;
					stream = loader.Read32();
				}
			}

			#ifdef NST_MSVC_OPTIMIZE
			#pragma optimize("", on)
			#endif

			uint HoriTrack::Peek(uint port)
			{
				if (port == 0)
				{
					port = stream;
					stream >>= 1;
					return port & 0x2;
				}
				else
				{
					return 0;
				}
			}

			void HoriTrack::Poke(uint data)
			{
				const uint prev = strobe;
				strobe = data & 0x1;

				if (prev > strobe)
				{
					if (input)
					{
						Controllers::HoriTrack& horiTrack = input->horiTrack;
						input = NULL;

						if (Controllers::HoriTrack::callback( horiTrack ))
						{
							dword bits = (horiTrack.buttons & 0xFF) | CONNECTED;

							if (horiTrack.mode & Controllers::HoriTrack::MODE_REVERSED)
								bits |= REVERSED;

							static const schar speeds[2][5] =
							{
								{0,4,8,16,24},
								{1,16,32,48,56}
							};

							const schar* NST_RESTRICT speed = speeds[0];

							if (horiTrack.mode & Controllers::HoriTrack::MODE_LOWSPEED)
							{
								speed = speeds[1];
								bits |= LOWSPEED;
							}

							int ox = x;
							int oy = y;

							x = NST_MIN(horiTrack.x,255);
							y = NST_MIN(horiTrack.y,239);

							ox -= x;
							oy -= y;

							if (ox > speed[0])
							{
								bits |=
								(
									(ox >= +speed[4]) ? (0x0000U|0x0100U) :
									(ox >= +speed[3]) ? (0x0800U|0x0100U) :
									(ox >= +speed[2]) ? (0x0400U|0x0100U) :
									(ox >= +speed[1]) ? (0x0200U|0x0100U) :
														(0x0600U|0x0100U)
								);
							}
							else if (ox < -speed[0])
							{
								bits |=
								(
									(ox <= -speed[4]) ? (0x0600U|0x0000U) :
									(ox <= -speed[3]) ? (0x0200U|0x0000U) :
									(ox <= -speed[2]) ? (0x0400U|0x0000U) :
									(ox <= -speed[1]) ? (0x0800U|0x0000U) :
														(0x0000U|0x0000U)
								);
							}
							else
							{
								bits |= 0x0F00;
							}

							if (oy > speed[0])
							{
								bits |=
								(
									(oy >= +speed[4]) ? (0x6000U|0x0000U) :
									(oy >= +speed[3]) ? (0x2000U|0x0000U) :
									(oy >= +speed[2]) ? (0x4000U|0x0000U) :
									(oy >= +speed[1]) ? (0x8000U|0x0000U) :
														(0x0000U|0x0000U)
								);
							}
							else if (oy < -speed[0])
							{
								bits |=
								(
									(oy <= -speed[4]) ? (0x0000U|0x1000U) :
									(oy <= -speed[3]) ? (0x8000U|0x1000U) :
									(oy <= -speed[2]) ? (0x4000U|0x1000U) :
									(oy <= -speed[1]) ? (0x2000U|0x1000U) :
														(0x6000U|0x1000U)
								);
							}
							else
							{
								bits |= 0xF000;
							}

							state = bits << 1;
						}
					}

					stream = state;
				}
			}
		}
	}
}
