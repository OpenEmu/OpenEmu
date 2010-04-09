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

#include <new>
#include "NstCpu.hpp"
#include "NstChips.hpp"
#include "NstSoundPlayer.hpp"
#include "api/NstApiUser.hpp"

namespace Nes
{
	namespace Core
	{
		namespace Sound
		{
			#ifdef NST_MSVC_OPTIMIZE
			#pragma optimize("s", on)
			#endif

			Player::Slot::Slot()
			: data(NULL) {}

			Player::Slot::~Slot()
			{
				delete [] data;
			}

			Player::Player(Apu& a,uint n)
			: Pcm(a), slots(new Slot [n]), numSlots(n)
			{
				NST_ASSERT( n );
			}

			Player::~Player()
			{
				delete [] slots;
			}

			Player* Player::Create(Apu& apu,const Chips& chips,wcstring const chip,Game game,uint maxSamples)
			{
				if (!maxSamples)
					return NULL;

				if (chip && chips.Has(chip) && chips[chip].HasSamples())
				{
					game = GAME_UNKNOWN;
				}
				else if (game != GAME_UNKNOWN)
				{
					maxSamples = uint(game) >> GAME_NUM_SAMPLES_SHIFT;
					NST_ASSERT( maxSamples );
				}
				else
				{
					return NULL;
				}

				if (Player* const player = new (std::nothrow) Player(apu,maxSamples))
				{
					for (uint i=0; i < maxSamples; ++i)
					{
						class Loader : public Api::User::File
						{
							const Action action;
							Slot& slot;
							const uint id;
							wcstring const filename;

							Action GetAction() const throw()
							{
								return action;
							}

							wcstring GetName() const throw()
							{
								return filename;
							}

							uint GetId() const throw()
							{
								return id;
							}

							Result SetSampleContent(const void* data,ulong length,bool stereo,uint bits,ulong rate) throw()
							{
								if (!data || !length)
									return RESULT_ERR_INVALID_PARAM;

								if (!Pcm::CanDo( bits, rate ))
									return RESULT_ERR_UNSUPPORTED;

								iword* NST_RESTRICT dst = new (std::nothrow) iword [length];

								if (!dst)
									return RESULT_ERR_OUT_OF_MEMORY;

								slot.data = dst;
								slot.length = length;
								slot.rate = rate;

								if (bits == 8)
								{
									const byte* NST_RESTRICT src = static_cast<const byte*>(data);
									const byte* const end = src + length;

									if (stereo)
									{
										for (; src != end; src += 2)
										{
											const idword sample = (idword(uint(src[0]) << 8) - 32768) + (idword(uint(src[1]) << 8) - 32768);
											*dst++ = Clamp<Apu::Channel::OUTPUT_MIN,Apu::Channel::OUTPUT_MAX>(sample);
										}
									}
									else
									{
										for (; src != end; src += 1)
										{
											const idword sample = idword(uint(*src) << 8) - 32768;
											*dst++ = Clamp<Apu::Channel::OUTPUT_MIN,Apu::Channel::OUTPUT_MAX>(sample);
										}
									}
								}
								else
								{
									const iword* NST_RESTRICT src = static_cast<const iword*>(data);
									const iword* const end = src + length;

									if (stereo)
									{
										for (; src != end; src += 2)
										{
											const idword sample = src[0] + src[1];
											*dst++ = Clamp<Apu::Channel::OUTPUT_MIN,Apu::Channel::OUTPUT_MAX>(sample);
										}
									}
									else
									{
										for (; src != end; src += 1)
										{
											const idword sample = *src;
											*dst++ = Clamp<Apu::Channel::OUTPUT_MIN,Apu::Channel::OUTPUT_MAX>(sample);
										}
									}
								}

								return RESULT_OK;
							}

						public:

							Loader(Game g,Slot& s,uint i,wcstring f)
							:
							action
							(
								g == GAME_MOERO_PRO_YAKYUU         ? LOAD_SAMPLE_MOERO_PRO_YAKYUU :
								g == GAME_MOERO_PRO_YAKYUU_88      ? LOAD_SAMPLE_MOERO_PRO_YAKYUU_88 :
								g == GAME_MOERO_PRO_TENNIS         ? LOAD_SAMPLE_MOERO_PRO_TENNIS :
								g == GAME_TERAO_NO_DOSUKOI_OOZUMOU ? LOAD_SAMPLE_TERAO_NO_DOSUKOI_OOZUMOU :
								g == GAME_AEROBICS_STUDIO          ? LOAD_SAMPLE_AEROBICS_STUDIO :
                                                                     LOAD_SAMPLE
							),
							slot     (s),
							id       (i),
							filename (f)
							{
							}
						};

						wcstring filename = L"";

						if (game != GAME_UNKNOWN || *(filename = *chips[chip].Sample(i)))
						{
							Loader loader( game, player->slots[i], i, filename );

							try
							{
								Api::User::fileIoCallback( loader );
							}
							catch (...)
							{
								delete player;
								throw;
							}
						}
					}

					for (uint i=0; i < maxSamples; ++i)
					{
						if (player->slots[i].data)
							return player;
					}

					delete player;
				}

				return NULL;
			}

			void Player::Destroy(Player* player)
			{
				delete player;
			}

			#ifdef NST_MSVC_OPTIMIZE
			#pragma optimize("", on)
			#endif
		}
	}
}
