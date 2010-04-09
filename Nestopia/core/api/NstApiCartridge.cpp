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
#include "../NstMachine.hpp"
#include "../NstStream.hpp"
#include "../NstChecksum.hpp"
#include "../NstCartridge.hpp"
#include "../NstImageDatabase.hpp"
#include "../NstCartridgeInes.hpp"
#include "NstApiMachine.hpp"

namespace Nes
{
	namespace Api
	{
		Cartridge::ChooseProfileCaller Cartridge::chooseProfileCallback;

		#ifdef NST_MSVC_OPTIMIZE
		#pragma optimize("s", on)
		#endif

		Cartridge::Profile::Hash::Hash() throw()
		{
			Clear();
		}

		Cartridge::Profile::Hash::Hash(const char* sha1,const char* crc) throw()
		{
			Assign( sha1, crc );
		}

		Cartridge::Profile::Hash::Hash(const wchar_t* sha1,const wchar_t* crc) throw()
		{
			Assign( sha1, crc );
		}

		Cartridge::Profile::Hash::Hash(const dword* sha1,dword crc) throw()
		{
			Assign( sha1, crc );
		}

		void Cartridge::Profile::Hash::Clear() throw()
		{
			Assign( NULL, dword(0) );
		}

		template<typename T>
		bool Cartridge::Profile::Hash::Set(dword& dst,const T* NST_RESTRICT src)
		{
			dword v = 0;
			uint i = 32;

			do
			{
				i -= 4;

				const T c = *src++;

				if (c >= '0' && c <= '9')
				{
					v |= dword(c - '0') << i;
				}
				else if (c >= 'A' && c <= 'F')
				{
					v |= dword(c - 'A' + 0xA) << i;
				}
				else if (c >= 'a' && c <= 'f')
				{
					v |= dword(c - 'a' + 0xA) << i;
				}
				else
				{
					return false;
				}
			}
			while (i);

			dst = v;
			return true;
		}

		template<typename T>
		void Cartridge::Profile::Hash::Import(const T* sha1,const T* crc)
		{
			Clear();

			if (crc && *crc)
				Set( data[0], crc );

			if (sha1 && *sha1)
			{
				for (uint i=1; i < 1+SHA1_WORD_LENGTH; ++i, sha1 += 8)
				{
					if (!Set( data[i], sha1 ))
					{
						for (uint j=1; j < i; ++j)
							data[j] = 0;

						break;
					}
				}
			}
		}

		void Cartridge::Profile::Hash::Assign(const char* sha1,const char* crc) throw()
		{
			Import( sha1, crc );
		}

		void Cartridge::Profile::Hash::Assign(const wchar_t* sha1,const wchar_t* crc) throw()
		{
			Import( sha1, crc );
		}

		void Cartridge::Profile::Hash::Assign(const dword* sha1,dword crc) throw()
		{
			data[0] = crc & 0xFFFFFFFF;

			for (uint i=0; i < SHA1_WORD_LENGTH; ++i)
				data[1+i] = (sha1 ? sha1[i] & 0xFFFFFFFF : 0);
		}

		void Cartridge::Profile::Hash::Get(char* sha1,char* crc) const throw()
		{
			if (crc)
			{
				for (dword v=data[0], j=32; j; )
				{
					j -= 4;
					uint c = v >> j & 0xF;
					*crc++ = (c < 0xA ? '0' + c : 'A' + (c - 0xA) );
				}
			}

			if (sha1)
			{
				for (uint i=1; i < 1+SHA1_WORD_LENGTH; ++i)
				{
					for (dword v=data[i], j=32; j; )
					{
						j -= 4;
						uint c = v >> j & 0xF;
						*sha1++ = (c < 0xA ? '0' + c : 'A' + (c - 0xA) );
					}
				}
			}
		}

		bool Cartridge::Profile::Hash::operator < (const Hash& c) const throw()
		{
			for (const dword *a=data, *b=c.data, *end=data+(1+SHA1_WORD_LENGTH); a != end; ++a, ++b)
			{
				if (*a < *b)
					return true;

				if (*a > *b)
					return false;
			}

			return false;
		}

		bool Cartridge::Profile::Hash::operator == (const Hash& c) const throw()
		{
			for (const dword *a=data, *b=c.data, *end=data+(1+SHA1_WORD_LENGTH); a != end; ++a, ++b)
			{
				if (*a != *b)
					return false;
			}

			return true;
		}

		bool Cartridge::Profile::Hash::operator ! () const throw()
		{
			for (uint i=0; i < 1+SHA1_WORD_LENGTH; ++i)
			{
				if (data[i])
					return false;
			}

			return true;
		}

		void Cartridge::Profile::Hash::Compute(const void* mem,ulong size) throw()
		{
			const Core::Checksum checksum( static_cast<const byte*>(mem), size );
			Assign( checksum.GetSha1(), checksum.GetCrc() );
		}

		const dword* Cartridge::Profile::Hash::GetSha1() const throw()
		{
			return data+1;
		}

		dword Cartridge::Profile::Hash::GetCrc32() const throw()
		{
			return data[0];
		}

		Cartridge::Profile::System::System() throw()
		: type(NES_NTSC), cpu(CPU_RP2A03), ppu(PPU_RP2C02) {}

		Cartridge::Profile::Dump::Dump() throw()
		: state(UNKNOWN) {}

		Cartridge::Profile::Game::Game() throw()
		: adapter(Input::ADAPTER_NES), players(0)
		{
			controllers[0] = Input::PAD1;
			controllers[1] = Input::PAD2;
			controllers[2] = Input::UNCONNECTED;
			controllers[3] = Input::UNCONNECTED;
			controllers[4] = Input::UNCONNECTED;
		}

		Cartridge::Profile::Board::Pin::Pin() throw()
		: number(0) {}

		Cartridge::Profile::Board::Sample::Sample() throw()
		: id(0) {}

		Cartridge::Profile::Board::Rom::Rom() throw()
		: id(0), size(0) {}

		Cartridge::Profile::Board::Ram::Ram() throw()
		: id(0), size(0), battery(false) {}

		Cartridge::Profile::Board::Chip::Chip() throw()
		: battery(false) {}

		Cartridge::Profile::Board::Board() throw()
		: solderPads(0), mapper(NO_MAPPER) {}

		Cartridge::Profile::Board::~Board() throw()
		{
		}

		Cartridge::Profile::Profile() throw()
		: multiRegion(false), patched(false)
		{
		}

		Cartridge::Profile::~Profile() throw()
		{
		}

		template<typename T>
		dword Cartridge::Profile::Board::GetComponentSize(const T& components) const
		{
			dword size = 0;

			for (typename T::const_iterator it(components.begin()), end(components.end()); it != end; ++it)
				size += it->size;

			return size;
		}

		dword Cartridge::Profile::Board::GetPrg() const throw()
		{
			return GetComponentSize( prg );
		}

		dword Cartridge::Profile::Board::GetChr() const throw()
		{
			return GetComponentSize( chr );
		}

		dword Cartridge::Profile::Board::GetWram() const throw()
		{
			return GetComponentSize( wram );
		}

		dword Cartridge::Profile::Board::GetVram() const throw()
		{
			return GetComponentSize( vram );
		}

		template<typename T>
		bool Cartridge::Profile::Board::HasComponentBattery(const T& components) const
		{
			for (typename T::const_iterator it(components.begin()), end(components.end()); it != end; ++it)
			{
				if (it->battery)
					return true;
			}

			return false;
		}

		bool Cartridge::Profile::Board::HasWramBattery() const throw()
		{
			return HasComponentBattery( wram );
		}

		bool Cartridge::Profile::Board::HasMmcBattery() const throw()
		{
			return HasComponentBattery( chips );
		}

		bool Cartridge::Profile::Board::HasBattery() const throw()
		{
			return HasWramBattery() || HasMmcBattery();
		}

		Cartridge::NesHeader::NesHeader() throw()
		{
			Clear();
		}

		void Cartridge::NesHeader::Clear() throw()
		{
			system = SYSTEM_CONSOLE;
			region = REGION_NTSC;
			prgRom = 0;
			prgRam = 0;
			prgNvRam = 0;
			chrRom = 0;
			chrRam = 0;
			chrNvRam = 0;
			ppu = PPU_RP2C02;
			mirroring = MIRRORING_VERTICAL;
			mapper = 0;
			subMapper = 0;
			security = 0;
			version = 0;
			trainer = false;
		}

		Result Cartridge::NesHeader::Import(const void* const data,const ulong length) throw()
		{
			return Core::Cartridge::Ines::ReadHeader( *this, static_cast<const byte*>(data), length );
		}

		Result Cartridge::NesHeader::Export(void* data,ulong length) const throw()
		{
			return Core::Cartridge::Ines::WriteHeader( *this, static_cast<byte*>(data), length );
		}

		bool Cartridge::Database::IsLoaded() const throw()
		{
			return emulator.imageDatabase;
		}

		bool Cartridge::Database::IsEnabled() const throw()
		{
			return emulator.imageDatabase && emulator.imageDatabase->Enabled();
		}

		bool Cartridge::Database::Create()
		{
			if (emulator.imageDatabase == NULL)
				emulator.imageDatabase = new (std::nothrow) Core::ImageDatabase;

			return emulator.imageDatabase;
		}

		Result Cartridge::Database::Load(std::istream& stream) throw()
		{
			return Create() ? emulator.imageDatabase->Load( stream ) : RESULT_ERR_OUT_OF_MEMORY;
		}

		Result Cartridge::Database::Load(std::istream& baseStream,std::istream& overloadStream) throw()
		{
			return Create() ? emulator.imageDatabase->Load( baseStream, overloadStream ) : RESULT_ERR_OUT_OF_MEMORY;
		}

		void Cartridge::Database::Unload() throw()
		{
			if (emulator.imageDatabase)
				emulator.imageDatabase->Unload();
		}

		Result Cartridge::Database::Enable(bool state) throw()
		{
			if (Create())
			{
				if (emulator.imageDatabase->Enabled() != state)
				{
					emulator.imageDatabase->Enable( state );
					return RESULT_OK;
				}

				return RESULT_NOP;
			}

			return RESULT_ERR_OUT_OF_MEMORY;
		}

		Result Cartridge::ReadRomset(std::istream& stream,Machine::FavoredSystem system,bool askProfile,Profile& profile) throw()
		{
			try
			{
				Core::Cartridge::ReadRomset( stream, static_cast<Core::FavoredSystem>(system), askProfile, profile );
			}
			catch (Result result)
			{
				return result;
			}
			catch (...)
			{
				return RESULT_ERR_GENERIC;
			}

			return RESULT_OK;
		}

		Result Cartridge::ReadInes(std::istream& stream,Machine::FavoredSystem system,Profile& profile) throw()
		{
			try
			{
				Core::Cartridge::ReadInes( stream, static_cast<Core::FavoredSystem>(system), profile );
			}
			catch (Result result)
			{
				return result;
			}
			catch (...)
			{
				return RESULT_ERR_GENERIC;
			}

			return RESULT_OK;
		}

		Result Cartridge::ReadUnif(std::istream& stream,Machine::FavoredSystem system,Profile& profile) throw()
		{
			try
			{
				Core::Cartridge::ReadUnif( stream, static_cast<Core::FavoredSystem>(system), profile );
			}
			catch (Result result)
			{
				return result;
			}
			catch (...)
			{
				return RESULT_ERR_GENERIC;
			}

			return RESULT_OK;
		}

		const Cartridge::Profile* Cartridge::GetProfile() const throw()
		{
			if (emulator.Is(Machine::CARTRIDGE))
				return &static_cast<const Core::Cartridge*>(emulator.image)->GetProfile();

			return NULL;
		}

		Cartridge::Database::Entry Cartridge::Database::FindEntry(const Profile::Hash& hash,Machine::FavoredSystem system) const throw()
		{
			return emulator.imageDatabase ? emulator.imageDatabase->Search( hash, static_cast<Core::FavoredSystem>(system) ).Reference() : NULL;
		}

		Cartridge::Database::Entry Cartridge::Database::FindEntry(const void* file,ulong length,Machine::FavoredSystem system) const throw()
		{
			if (emulator.imageDatabase)
			{
				Profile::Hash hash;
				hash.Compute( file, length );
				return emulator.imageDatabase->Search( hash, static_cast<Core::FavoredSystem>(system) ).Reference();
			}
			else
			{
				return NULL;
			}
		}

		Result Cartridge::Database::Entry::GetProfile(Profile& profile) const throw()
		{
			Core::ImageDatabase::Entry entry(ref);

			if (!entry)
				return RESULT_ERR_NOT_READY;

			try
			{
				entry.Fill(profile);
			}
			catch (const std::bad_alloc&)
			{
				return RESULT_ERR_OUT_OF_MEMORY;
			}
			catch (...)
			{
				return RESULT_ERR_GENERIC;
			}

			return RESULT_OK;
		}

		const wchar_t* Cartridge::Database::Entry::GetTitle() const throw()
		{
			return Core::ImageDatabase::Entry(ref).GetTitle();
		}

		const Cartridge::Profile::Hash* Cartridge::Database::Entry::GetHash() const throw()
		{
			return Core::ImageDatabase::Entry(ref).GetHash();
		}

		const wchar_t* Cartridge::Database::Entry::GetRegion() const throw()
		{
			return Core::ImageDatabase::Entry(ref).GetRegion();
		}

		Cartridge::Profile::System::Type Cartridge::Database::Entry::GetSystem() const throw()
		{
			return Core::ImageDatabase::Entry(ref).GetSystem();
		}

		bool Cartridge::Database::Entry::IsMultiRegion() const throw()
		{
			return Core::ImageDatabase::Entry(ref).IsMultiRegion();
		}

		dword Cartridge::Database::Entry::GetPrgRom() const throw()
		{
			return Core::ImageDatabase::Entry(ref).GetPrg();
		}

		dword Cartridge::Database::Entry::GetChrRom() const throw()
		{
			return Core::ImageDatabase::Entry(ref).GetChr();
		}

		uint Cartridge::Database::Entry::GetWram() const throw()
		{
			return Core::ImageDatabase::Entry(ref).GetWram();
		}

		uint Cartridge::Database::Entry::GetVram() const throw()
		{
			return Core::ImageDatabase::Entry(ref).GetVram();
		}

		bool Cartridge::Database::Entry::HasBattery() const throw()
		{
			return Core::ImageDatabase::Entry(ref).HasBattery();
		}

		uint Cartridge::Database::Entry::GetMapper() const throw()
		{
			return Core::ImageDatabase::Entry(ref).GetMapper();
		}

		Cartridge::Profile::Dump::State Cartridge::Database::Entry::GetDumpState() const throw()
		{
			return Core::ImageDatabase::Entry(ref).GetDumpState();
		}

		#ifdef NST_MSVC_OPTIMIZE
		#pragma optimize("", on)
		#endif
	}
}
