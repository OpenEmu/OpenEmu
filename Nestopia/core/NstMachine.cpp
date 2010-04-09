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

#include "NstMachine.hpp"
#include "NstCartridge.hpp"
#include "NstCheats.hpp"
#include "NstNsf.hpp"
#include "NstImageDatabase.hpp"
#include "input/NstInpDevice.hpp"
#include "input/NstInpAdapter.hpp"
#include "input/NstInpPad.hpp"
#include "api/NstApiMachine.hpp"
#include "api/NstApiUser.hpp"

namespace Nes
{
	namespace Core
	{
		#ifdef NST_MSVC_OPTIMIZE
		#pragma optimize("s", on)
		#endif

		Machine::Machine()
		:
		state         (Api::Machine::NTSC),
		frame         (0),
		extPort       (new Input::AdapterTwo( *new Input::Pad(cpu,0), *new Input::Pad(cpu,1) )),
		expPort       (new Input::Device( cpu )),
		image         (NULL),
		cheats        (NULL),
		imageDatabase (NULL),
		ppu           (cpu)
		{
		}

		Machine::~Machine()
		{
			Unload();

			delete imageDatabase;
			delete cheats;
			delete expPort;

			for (uint ports=extPort->NumPorts(), i=0; i < ports; ++i)
				delete &extPort->GetDevice(i);

			delete extPort;
		}

		Result Machine::Load
		(
			std::istream& imageStream,
			FavoredSystem system,
			bool ask,
			std::istream* const patchStream,
			bool patchBypassChecksum,
			Result* patchResult,
			uint type
		)
		{
			Unload();

			Image::Context context
			(
				static_cast<Image::Type>(type),
				cpu,
				cpu.GetApu(),
				ppu,
				imageStream,
				patchStream,
				patchBypassChecksum,
				patchResult,
				system,
				ask,
				imageDatabase
			);

			image = Image::Load( context );

			switch (image->GetType())
			{
				case Image::CARTRIDGE:

					state |= Api::Machine::CARTRIDGE;

					switch (static_cast<const Cartridge*>(image)->GetProfile().system.type)
					{
						case Api::Cartridge::Profile::System::VS_UNISYSTEM:

							state |= Api::Machine::VS;
							break;

						case Api::Cartridge::Profile::System::PLAYCHOICE_10:

							state |= Api::Machine::PC10;
							break;
					}
					break;

				case Image::DISK:

					state |= Api::Machine::DISK;
					break;

				case Image::SOUND:

					state |= Api::Machine::SOUND;
					break;
			}

			UpdateModels();
			UpdateColorMode();

			Api::Machine::eventCallback( Api::Machine::EVENT_LOAD, context.result );

			return context.result;
		}

		Result Machine::Unload()
		{
			if (!image)
				return RESULT_OK;

			const Result result = PowerOff();

			tracker.Unload();

			Image::Unload( image );
			image = NULL;

			state &= (Api::Machine::NTSC|Api::Machine::PAL);

			Api::Machine::eventCallback( Api::Machine::EVENT_UNLOAD, result );

			return result;
		}

		void Machine::UpdateModels()
		{
			const Region region = (state & Api::Machine::NTSC) ? REGION_NTSC : REGION_PAL;

			CpuModel cpuModel;
			PpuModel ppuModel;

			if (image)
			{
				image->GetDesiredSystem( region, &cpuModel, &ppuModel );
			}
			else
			{
				cpuModel = (region == REGION_NTSC ? CPU_RP2A03 : CPU_RP2A07);
				ppuModel = (region == REGION_NTSC ? PPU_RP2C02 : PPU_RP2C07);
			}

			cpu.SetModel( cpuModel );
			ppu.SetModel( ppuModel, renderer.GetPaletteType() == Video::Renderer::PALETTE_YUV );

			renderer.EnableForcedFieldMerging( ppuModel != PPU_RP2C02 );
		}

		Result Machine::UpdateColorMode()
		{
			return UpdateColorMode
			(
				renderer.GetPaletteType() == Video::Renderer::PALETTE_YUV    ? COLORMODE_YUV :
				renderer.GetPaletteType() == Video::Renderer::PALETTE_CUSTOM ? COLORMODE_CUSTOM :
                                                                               COLORMODE_RGB
			);
		}

		Result Machine::UpdateColorMode(const ColorMode mode)
		{
			ppu.SetModel( ppu.GetModel(), mode == COLORMODE_YUV );

			Video::Renderer::PaletteType palette;

			switch (mode)
			{
				case COLORMODE_RGB:

					switch (ppu.GetModel())
					{
						case PPU_RP2C04_0001: palette = Video::Renderer::PALETTE_VS1;  break;
						case PPU_RP2C04_0002: palette = Video::Renderer::PALETTE_VS2;  break;
						case PPU_RP2C04_0003: palette = Video::Renderer::PALETTE_VS3;  break;
						case PPU_RP2C04_0004: palette = Video::Renderer::PALETTE_VS4;  break;
						default:              palette = Video::Renderer::PALETTE_PC10; break;
					}
					break;

				case COLORMODE_CUSTOM:

					palette = Video::Renderer::PALETTE_CUSTOM;
					break;

				default:

					palette = Video::Renderer::PALETTE_YUV;
					break;
			}

			return renderer.SetPaletteType( palette );
		}

		Result Machine::PowerOff(Result result)
		{
			if (state & Api::Machine::ON)
			{
				tracker.PowerOff();

				if (image && !image->PowerOff() && NES_SUCCEEDED(result))
					result = RESULT_WARN_SAVEDATA_LOST;

				ppu.PowerOff();
				cpu.PowerOff();

				state &= ~uint(Api::Machine::ON);
				frame = 0;

				Api::Machine::eventCallback( Api::Machine::EVENT_POWER_OFF, result );
			}

			return result;
		}

		void Machine::Reset(bool hard)
		{
			if (state & Api::Machine::SOUND)
				hard = true;

			try
			{
				frame = 0;
				cpu.Reset( hard );

				if (!(state & Api::Machine::SOUND))
				{
					InitializeInputDevices();

					cpu.Map( 0x4016 ).Set( this, &Machine::Peek_4016, &Machine::Poke_4016 );
					cpu.Map( 0x4017 ).Set( this, &Machine::Peek_4017, &Machine::Poke_4017 );

					extPort->Reset();
					expPort->Reset();

					ppu.Reset
					(
						hard,
						image ? image->GetDesiredSystem((state & Api::Machine::NTSC) ? REGION_NTSC : REGION_PAL) != SYSTEM_FAMICOM : true
					);

					if (image)
						image->Reset( hard );

					if (cheats)
						cheats->Reset();

					tracker.Reset();
				}
				else
				{
					image->Reset( true );
				}

				cpu.Boot( hard );

				if (state & Api::Machine::ON)
				{
					Api::Machine::eventCallback( hard ? Api::Machine::EVENT_RESET_HARD : Api::Machine::EVENT_RESET_SOFT );
				}
				else
				{
					state |= Api::Machine::ON;
					Api::Machine::eventCallback( Api::Machine::EVENT_POWER_ON );
				}
			}
			catch (...)
			{
				PowerOff();
				throw;
			}
		}

		void Machine::SwitchMode()
		{
			NST_ASSERT( !(state & Api::Machine::ON) );

			if (state & Api::Machine::NTSC)
				state = (state & ~uint(Api::Machine::NTSC)) | Api::Machine::PAL;
			else
				state = (state & ~uint(Api::Machine::PAL)) | Api::Machine::NTSC;

			UpdateModels();

			Api::Machine::eventCallback( (state & Api::Machine::NTSC) ? Api::Machine::EVENT_MODE_NTSC : Api::Machine::EVENT_MODE_PAL );
		}

		void Machine::InitializeInputDevices() const
		{
			if (state & Api::Machine::GAME)
			{
				const bool arcade = state & Api::Machine::VS;

				extPort->Initialize( arcade );
				expPort->Initialize( arcade );
			}
		}

		void Machine::SaveState(State::Saver& saver) const
		{
			NST_ASSERT( (state & (Api::Machine::GAME|Api::Machine::ON)) > Api::Machine::ON );

			saver.Begin( AsciiId<'N','S','T'>::V | 0x1AUL << 24 );

			saver.Begin( AsciiId<'N','F','O'>::V ).Write32( image->GetPrgCrc() ).Write32( frame ).End();

			cpu.SaveState( saver, AsciiId<'C','P','U'>::V, AsciiId<'A','P','U'>::V );
			ppu.SaveState( saver, AsciiId<'P','P','U'>::V );
			image->SaveState( saver, AsciiId<'I','M','G'>::V );

			saver.Begin( AsciiId<'P','R','T'>::V );

			if (extPort->NumPorts() == 4)
			{
				static_cast<const Input::AdapterFour*>(extPort)->SaveState
				(
					saver, AsciiId<'4','S','C'>::V
				);
			}

			for (uint i=0; i < extPort->NumPorts(); ++i)
				extPort->GetDevice( i ).SaveState( saver, Ascii<'0'>::V + i );

			expPort->SaveState( saver, Ascii<'X'>::V );

			saver.End();

			saver.End();
		}

		bool Machine::LoadState(State::Loader& loader,const bool resetOnError)
		{
			NST_ASSERT( (state & (Api::Machine::GAME|Api::Machine::ON)) > Api::Machine::ON );

			try
			{
				if (loader.Begin() != (AsciiId<'N','S','T'>::V | 0x1AUL << 24))
					throw RESULT_ERR_INVALID_FILE;

				while (const dword chunk = loader.Begin())
				{
					switch (chunk)
					{
						case AsciiId<'N','F','O'>::V:
						{
							const dword crc = loader.Read32();

							if
							(
								loader.CheckCrc() && !(state & Api::Machine::DISK) &&
								crc && crc != image->GetPrgCrc() &&
								Api::User::questionCallback( Api::User::QUESTION_NST_PRG_CRC_FAIL_CONTINUE ) == Api::User::ANSWER_NO
							)
							{
								for (uint i=0; i < 2; ++i)
									loader.End();

								return false;
							}

							frame = loader.Read32();
							break;
						}

						case AsciiId<'C','P','U'>::V:
						case AsciiId<'A','P','U'>::V:

							cpu.LoadState( loader, AsciiId<'C','P','U'>::V, AsciiId<'A','P','U'>::V, chunk );
							break;

						case AsciiId<'P','P','U'>::V:

							ppu.LoadState( loader );
							break;

						case AsciiId<'I','M','G'>::V:

							image->LoadState( loader );
							break;

						case AsciiId<'P','R','T'>::V:

							extPort->Reset();
							expPort->Reset();

							while (const dword subId = loader.Begin())
							{
								if (subId == AsciiId<'4','S','C'>::V)
								{
									if (extPort->NumPorts() == 4)
										static_cast<Input::AdapterFour*>(extPort)->LoadState( loader );
								}
								else switch (const uint index = (subId >> 16 & 0xFF))
								{
									case Ascii<'2'>::V:
									case Ascii<'3'>::V:

										if (extPort->NumPorts() != 4)
											break;

									case Ascii<'0'>::V:
									case Ascii<'1'>::V:

										extPort->GetDevice( index - Ascii<'0'>::V ).LoadState( loader, subId & 0xFF00FFFF );
										break;

									case Ascii<'X'>::V:

										expPort->LoadState( loader, subId & 0xFF00FFFF );
										break;
								}

								loader.End();
							}
							break;
					}

					loader.End();
				}

				loader.End();
			}
			catch (...)
			{
				if (resetOnError)
					Reset( true );

				throw;
			}

			return true;
		}

		#ifdef NST_MSVC_OPTIMIZE
		#pragma optimize("", on)
		#endif

		void Machine::Execute
		(
			Video::Output* const video,
			Sound::Output* const sound,
			Input::Controllers* const input
		)
		{
			NST_ASSERT( state & Api::Machine::ON );

			if (!(state & Api::Machine::SOUND))
			{
				if (state & Api::Machine::CARTRIDGE)
					static_cast<Cartridge*>(image)->BeginFrame( Api::Input(*this), input );

				extPort->BeginFrame( input );
				expPort->BeginFrame( input );

				ppu.BeginFrame( tracker.IsFrameLocked() );

				if (cheats)
					cheats->BeginFrame( tracker.IsFrameLocked() );

				cpu.ExecuteFrame( sound );
				ppu.EndFrame();

				if (video)
					renderer.Blit( *video, ppu.GetScreen(), ppu.GetBurstPhase() );

				cpu.EndFrame();

				if (image)
					image->VSync();

				extPort->EndFrame();
				expPort->EndFrame();

				frame++;
			}
			else
			{
				static_cast<Nsf*>(image)->BeginFrame();

				cpu.ExecuteFrame( sound );
				cpu.EndFrame();

				image->VSync();
			}
		}

		NES_POKE_D(Machine,4016)
		{
			extPort->Poke( data );
			expPort->Poke( data );
		}

		NES_PEEK(Machine,4016)
		{
			return OPEN_BUS | extPort->Peek(0) | expPort->Peek(0);
		}

		NES_POKE_D(Machine,4017)
		{
			cpu.GetApu().WriteFrameCtrl( data );
		}

		NES_PEEK(Machine,4017)
		{
			return OPEN_BUS | extPort->Peek(1) | expPort->Peek(1);
		}
	}
}
