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

#ifndef NST_API_CARTRIDGE_H
#define NST_API_CARTRIDGE_H

#include <iosfwd>
#include <string>
#include <vector>
#include "NstApiInput.hpp"
#include "NstApiMachine.hpp"

#ifdef NST_PRAGMA_ONCE
#pragma once
#endif

#if NST_MSVC >= 1200
#pragma warning( push )
#pragma warning( disable : 4512 )
#endif

namespace Nes
{
	namespace Api
	{
		/**
		* Cartridge interface.
		*/
		class Cartridge : public Base
		{
			struct ChooseProfileCaller;

		public:

			/**
			* Interface constructor.
			*
			* @param instance emulator instance
			*/
			template<typename T>
			Cartridge(T& instance)
			: Base(instance) {}

			/**
			* Cartridge profile context.
			**/
			struct Profile
			{
				Profile() throw();
				~Profile() throw();

				/**
				* Hash checksum.
				*
				* Stores SHA-1 and CRC-32 combined or just one of the two.
				*/
				class Hash : public Core::ImplicitBool<Hash>
				{
				public:

					enum
					{
						SHA1_LENGTH = 40,
						SHA1_WORD_LENGTH = SHA1_LENGTH / 8,
						CRC32_LENGTH = 8,
						CRC32_WORD_LENGTH = CRC32_LENGTH / 8
					};

					/**
					* Default constructor.
					*/
					Hash() throw();

					/**
					* Constructs new checksum from null-terminated strings.
					*
					* @param sha SHA-1 string, set to NULL if values shouldn't be used
					* @param crc CRC-32 string, set to NULL if value shouldn't be used
					*/
					Hash(const char* sha,const char* crc) throw();

					/**
					* Constructs new checksum from null-terminated wide-strings.
					*
					* @param sha SHA-1 string, set to NULL if values shouldn't be used
					* @param crc CRC-32 string, set to NULL if value shouldn't be used
					*/
					Hash(const wchar_t* sha,const wchar_t* crc) throw();

					/**
					* Constructs new checksum from input values.
					*
					* @param sha SHA-1 value, set to NULL if values shouldn't be used
					* @param crc CRC-32 value, set to 0 if value shouldn't be used
					*/
					Hash(const dword* sha,dword crc) throw();

					/**
					* Tests for less-than.
					*
					* @param hash hash to compare with
					* @return true if input hash is less than this
					*/
					bool operator < (const Hash& hash) const throw();

					/**
					* Tests for equality.
					*
					* @param hash hash to compare with
					* @return true if hashes are equal
					*/
					bool operator == (const Hash& hash) const throw();

					/**
					* Checks if checksum is cleared.
					*
					* @return true if cleared
					*/
					bool operator ! () const throw();

					/**
					* Computes and updates checksum from input.
					*
					* @param mem pointer to memory
					* @param length length of memory in bytes
					*/
					void Compute(const void* mem,ulong length) throw();

					/**
					* Assigns new checksum from null-terminated strings.
					*
					* @param sha SHA-1 string, set to NULL if values shouldn't be used
					* @param crc CRC-32 string, set to NULL if value shouldn't be used
					*/
					void Assign(const char* sha,const char* crc) throw();

					/**
					* Assigns new checksum from null-terminated wide-strings.
					*
					* @param sha SHA-1 string, set to NULL if values shouldn't be used
					* @param crc CRC-32 string, set to NULL if value shouldn't be used
					*/
					void Assign(const wchar_t* sha,const wchar_t* crc) throw();

					/**
					* Assigns new checksum from input values.
					*
					* @param sha SHA-1 value, set to NULL if values shouldn't be used
					* @param crc CRC-32 value, set to 0 if value shouldn't be used
					*/
					void Assign(const dword* sha,dword crc) throw();

					/**
					* Returns the current checksum.
					*
					* @param sha SHA-1 string to be filled, set to to NULL if not needed
					* @param crc CRC-32 string to be filled, set to NULL if not needed
					*/
					void Get(char* sha,char* crc) const throw();

					/**
					* Clears the current checksum.
					*/
					void Clear() throw();

					/**
					* Returns the current SHA-1 values.
					*
					* @return SHA-1 values, zero-filled if unused
					*/
					const dword* GetSha1() const throw();

					/**
					* Returns the current CRC-32 value.
					*
					* @return CRC-32 value, 0 if unused
					*/
					dword GetCrc32() const throw();

				private:

					template<typename T>
					void Import(const T*,const T*);

					template<typename T>
					static bool Set(dword&,const T* NST_RESTRICT);

					dword data[CRC32_WORD_LENGTH+SHA1_WORD_LENGTH];
				};

				/**
				* Game context.
				*/
				struct Game
				{
					Game() throw();

					/**
					* Game title.
					*/
					std::wstring title;
					/**
					* Alternative game title.
					*/
					std::wstring altTitle;
					/**
					* Class.
					*/
					std::wstring clss;
					/**
					* Sub-class.
					*/
					std::wstring subClss;
					/**
					* Catalog.
					*/
					std::wstring catalog;
					/**
					* Publisher.
					*/
					std::wstring publisher;
					/**
					* Developer.
					*/
					std::wstring developer;
					/**
					* Port Developer.
					*/
					std::wstring portDeveloper;
					/**
					* Region.
					*/
					std::wstring region;
					/**
					* Revision.
					*/
					std::wstring revision;
					/**
					* Utilized controller adapter.
					*/
					Input::Adapter adapter;
					/**
					* Utilized controllers.
					*/
					Input::Type controllers[5];
					/**
					* Number of players.
					*/
					uint players;
				};

				/**
				* Dump context.
				*/
				struct Dump
				{
					Dump() throw();

					/**
					* Dump state type.
					*/
					enum State
					{
						/**
						* Good dump.
						*/
						OK,
						/**
						* Bad dump.
						*/
						BAD,
						/**
						* Unknown dump.
						*/
						UNKNOWN
					};

					/**
					* Dumped by.
					*/
					std::wstring by;
					/**
					* Dump date.
					*/
					std::wstring date;
					/**
					* Dump state.
					*/
					State state;
				};

				/**
				* System context.
				*/
				struct System
				{
					System() throw();

					/**
					* System Type.
					*/
					enum Type
					{
						/**
						* NES NTSC console.
						*/
						NES_NTSC = Core::SYSTEM_NES_NTSC,
						/**
						* NES PAL console.
						*/
						NES_PAL = Core::SYSTEM_NES_PAL,
						/**
						* NES PAL-A console.
						*/
						NES_PAL_A = Core::SYSTEM_NES_PAL_A,
						/**
						* NES PAL-B console.
						*/
						NES_PAL_B = Core::SYSTEM_NES_PAL_B,
						/**
						* Famicom console.
						*/
						FAMICOM = Core::SYSTEM_FAMICOM,
						/**
						* Vs UniSystem arcade.
						*/
						VS_UNISYSTEM = Core::SYSTEM_VS_UNISYSTEM,
						/**
						* Vs DualSystem arcade.
						*/
						VS_DUALSYSTEM = Core::SYSTEM_VS_DUALSYSTEM,
						/**
						* PlayChoice-10 arcade.
						*/
						PLAYCHOICE_10 = Core::SYSTEM_PLAYCHOICE_10
					};

					/**
					* CPU type.
					*/
					enum Cpu
					{
						/**
						* RP2A03 NTSC CPU.
						*/
						CPU_RP2A03 = Core::CPU_RP2A03,
						/**
						* RP2A07 PAL CPU.
						*/
						CPU_RP2A07 = Core::CPU_RP2A07
					};

					/**
					* PPU type.
					*/
					enum Ppu
					{
						/**
						* RP2C02 NTSC PPU.
						*/
						PPU_RP2C02 = Core::PPU_RP2C02,
						/**
						* RP2C03B RGB PPU.
						*/
						PPU_RP2C03B = Core::PPU_RP2C03B,
						/**
						* RP2C03G RGB PPU.
						*/
						PPU_RP2C03G = Core::PPU_RP2C03G,
						/**
						* RP2C04-0001 RGB PPU.
						*/
						PPU_RP2C04_0001 = Core::PPU_RP2C04_0001,
						/**
						* RP2C04-0002 RGB PPU.
						*/
						PPU_RP2C04_0002 = Core::PPU_RP2C04_0002,
						/**
						* RP2C04-0003 RGB PPU.
						*/
						PPU_RP2C04_0003 = Core::PPU_RP2C04_0003,
						/**
						* RP2C04-0004 RGB PPU.
						*/
						PPU_RP2C04_0004 = Core::PPU_RP2C04_0004,
						/**
						* RC2C03B RGB PPU.
						*/
						PPU_RC2C03B = Core::PPU_RC2C03B,
						/**
						* RC2C03C RGB PPU.
						*/
						PPU_RC2C03C = Core::PPU_RC2C03C,
						/**
						* RC2C05-01 RGB PPU.
						*/
						PPU_RC2C05_01 = Core::PPU_RC2C05_01,
						/**
						* RC2C05-02 RGB PPU.
						*/
						PPU_RC2C05_02 = Core::PPU_RC2C05_02,
						/**
						* RC2C05-03 RGB PPU.
						*/
						PPU_RC2C05_03 = Core::PPU_RC2C05_03,
						/**
						* RC2C05-04 RGB PPU.
						*/
						PPU_RC2C05_04 = Core::PPU_RC2C05_04,
						/**
						* RC2C05-05 RGB PPU.
						*/
						PPU_RC2C05_05 = Core::PPU_RC2C05_05,
						/**
						* RP2C07 PAL PPU.
						*/
						PPU_RP2C07 = Core::PPU_RP2C07
					};

					/**
					* System type.
					*/
					Type type;
					/**
					* CPU type.
					*/
					Cpu cpu;
					/**
					* PPU type.
					*/
					Ppu ppu;
				};

				/**
				* Cartridge property.
				*/
				struct Property
				{
					/**
					* Name.
					*/
					std::wstring name;
					/**
					* Value.
					*/
					std::wstring value;
				};

				/**
				* Cartridge properties.
				*/
				typedef std::vector<Property> Properties;

				/**
				* Board context.
				*/
				class Board
				{
					template<typename T>
					dword GetComponentSize(const T&) const;

					template<typename T>
					bool HasComponentBattery(const T&) const;

				public:

					Board() throw();
					~Board() throw();

					/**
					* Returns total size of PRG-ROM.
					*
					* @return size
					*/
					dword GetPrg() const throw();

					/**
					* Returns total size of CHR-ROM.
					*
					* @return size
					*/
					dword GetChr() const throw();

					/**
					* Returns total size of W-RAM.
					*
					* @return size
					*/
					dword GetWram() const throw();

					/**
					* Returns total size of V-RAM.
					*
					* @return size
					*/
					dword GetVram() const throw();

					/**
					* Returns battery status.
					*
					* @return true if a battery is present
					*/
					bool HasBattery() const throw();

					/**
					* Returns W-RAM battery status.
					*
					* @return true if a battery is present and connected to W-RAM
					*/
					bool HasWramBattery() const throw();

					/**
					* Returns custom chip battery status.
					*
					* @return true if a battery is present and connected to a custom chip
					*/
					bool HasMmcBattery() const throw();

					enum
					{
						SOLDERPAD_H = 0x1,
						SOLDERPAD_V = 0x2,
						NO_MAPPER = 0xFFFF
					};

					/**
					* Pin context.
					*/
					struct Pin
					{
						Pin() throw();

						/**
						* Pin number.
						*/
						uint number;

						/**
						* Pin function.
						*/
						std::wstring function;
					};

					/**
					* Pins.
					*/
					typedef std::vector<Pin> Pins;

					/**
					* Analogue sound sample context.
					*/
					struct Sample
					{
						Sample() throw();

						/**
						* Sound sample id.
						*/
						uint id;

						/**
						* Sound sample file.
						*/
						std::wstring file;
					};

					/**
					* Analogue sound samples.
					*/
					typedef std::vector<Sample> Samples;

					/**
					* ROM chip.
					*/
					struct Rom
					{
						Rom() throw();

						/**
						* ROM chip ID.
						*/
						dword id;

						/**
						* ROM chip size.
						*/
						dword size;

						/**
						* ROM chip name.
						*/
						std::wstring name;

						/**
						* File pointing to ROM chip.
						*/
						std::wstring file;

						/**
						* ROM chip package method.
						*/
						std::wstring package;

						/**
						* ROM chip pins.
						*/
						Pins pins;

						/**
						* ROM chip checksum.
						*/
						Hash hash;
					};

					/**
					* RAM chip.
					*/
					struct Ram
					{
						Ram() throw();

						/**
						* RAM chip ID.
						*/
						dword id;

						/**
						* RAM chip size.
						*/
						dword size;

						/**
						* File pointing to RAM chip.
						*/
						std::wstring file;

						/**
						* RAM chip package method.
						*/
						std::wstring package;

						/**
						* RAM chip pins.
						*/
						Pins pins;

						/**
						* Battery connected to RAM chip.
						*/
						bool battery;
					};

					/**
					* Custom chip.
					*/
					struct Chip
					{
						Chip() throw();

						/**
						* Custom chip type.
						*/
						std::wstring type;

						/**
						* File pointing to custom chip.
						*/
						std::wstring file;

						/**
						* Custom chip package type.
						*/
						std::wstring package;

						/**
						* Custom chip pins.
						*/
						Pins pins;

						/**
						* Analogue sound samples for custom chip.
						*/
						Samples samples;

						/**
						* battery connected to custom chip.
						*/
						bool battery;
					};

					/**
					* ROM chips.
					*/
					typedef std::vector<Rom> Roms;

					/**
					* RAM chips.
					*/
					typedef std::vector<Ram> Rams;

					/**
					* Custom chips.
					*/
					typedef std::vector<Chip> Chips;

					/**
					* PRG-ROM chips.
					*/
					typedef Roms Prg;

					/**
					* CHR-ROM chips.
					*/
					typedef Roms Chr;

					/**
					* W-RAM chips.
					*/
					typedef Rams Wram;

					/**
					* V-RAM chips.
					*/
					typedef Rams Vram;

					/**
					* Board type.
					*/
					std::wstring type;

					/**
					* CIC type.
					*/
					std::wstring cic;

					/**
					* Board PCB name.
					*/
					std::wstring pcb;

					/**
					* PRG-ROM.
					*/
					Prg prg;

					/**
					* CHR-ROM.
					*/
					Chr chr;

					/**
					* W-RAM.
					*/
					Wram wram;

					/**
					* V-RAM.
					*/
					Vram vram;

					/**
					* Custom chips.
					*/
					Chips chips;

					/**
					* Solder pads.
					*/
					uint solderPads;

					/**
					* Mapper ID.
					*/
					uint mapper;
				};

				/**
				* Hash of ROM chips combined.
				*/
				Hash hash;

				/**
				* Dump context.
				*/
				Dump dump;

				/**
				* Game context.
				*/
				Game game;

				/**
				* System context.
				*/
				System system;

				/**
				* Board context.
				*/
				Board board;

				/**
				* Properties.
				*/
				Properties properties;

				/**
				* Multi-region game.
				*/
				bool multiRegion;

				/**
				* Soft-patching state.
				*/
				bool patched;
			};

			/**
			* Database interface
			*/
			class Database
			{
				Core::Machine& emulator;

				bool Create();

			public:

				/**
				* Interface constructor.
				*
				* @param instance emulator instance
				*/
				Database(Core::Machine& instance)
				: emulator(instance) {}

				/**
				* Database entry.
				*/
				class Entry : public Core::ImplicitBool<Entry>
				{
				public:

					/**
					* Returns the profile of this entry.
					*
					* @param profile object to be filled
					* @return result code
					*/
					Result GetProfile(Profile& profile) const throw();

					/**
					* Returns the game title.
					*
					* @return game title or empty string on invalid entry
					*/
					const wchar_t* GetTitle() const throw();

					/**
					* Returns the target region.
					*
					* @return target region or empty string on invalid entry
					*/
					const wchar_t* GetRegion() const throw();

					/**
					* Returns the target system.
					*
					* @return target system
					*/
					Profile::System::Type GetSystem() const throw();

					/**
					* Checks if the game targets multiple regions.
					*
					* @return true if targeting multiple regions
					*/
					bool IsMultiRegion () const throw();

					/**
					* Returns hash code of ROM chips combined.
					*
					* @return hash code or NULL on invalid entry
					*/
					const Profile::Hash* GetHash() const throw();

					/**
					* Returns total size of PRG-ROM.
					*
					* @return size or 0 on invalid entry
					*/
					dword GetPrgRom() const throw();

					/**
					* Returns total size of CHR-ROM.
					*
					* @return size or 0 on invalid entry
					*/
					dword GetChrRom() const throw();

					/**
					* Returns total size of W-RAM.
					*
					* @return size or 0 on invalid entry
					*/
					uint GetWram() const throw();

					/**
					* Returns total size of V-RAM.
					*
					* @return size or 0 on invalid entry
					*/
					uint GetVram() const throw();

					/**
					* Returns mapper ID.
					*
					* @return mapper ID or 0 on invalid entry
					*/
					uint GetMapper() const throw();

					/**
					* Returns battery status.
					*
					* @return true if a battery is present
					*/
					bool HasBattery() const throw();

					/**
					* Returns the dump state.
					*
					* @return dump state
					*/
					Profile::Dump::State GetDumpState() const throw();

				private:

					friend class Database;
					const void* ref;

					Entry(const void* r)
					: ref(r) {}

				public:

					/**
					* Default constructor.
					*/
					Entry()
					: ref(NULL) {}

					/**
					* Checks if entry is invalid.
					*
					* @return true if invalid
					*/
					bool operator ! () const
					{
						return !ref;
					}
				};

				/**
				* Resets and loads internal XML database.
				*
				* @param stream input stream
				* @return result code
				*/
				Result Load(std::istream& stream) throw();

				/**
				* Resets and loads internal <b>and</b> external XML databases.
				*
				* @param streamInternal input stream to internal XML database
				* @param streamExternal input stream to external XML database
				* @return result code
				*/
				Result Load(std::istream& streamInternal,std::istream& streamExternal) throw();

				/**
				* Removes all databases from the system.
				*/
				void Unload() throw();

				/**
				* Enables image corrections.
				*
				* @param state true to enable, default is true
				* @return result code
				*/
				Result Enable(bool state=true) throw();

				/**
				* Checks if image corrections are enabled.
				*
				* @return true if enabled
				*/
				bool IsEnabled() const throw();

				/**
				* Checks if any database has been loaded into the system.
				*
				* @return true if loaded
				*/
				bool IsLoaded() const throw();

				/**
				* Attempts to locate and return an entry from one of the databases.
				*
				* @param hash hash code of combined ROMs
				* @param system preferred system in case of multiple profiles
				* @return entry found
				*/
				Entry FindEntry(const Profile::Hash& hash,Machine::FavoredSystem system) const throw();

				/**
				* Attempts to locate and return an entry from one of the databases.
				*
				* @param mem pointer to memory of combined ROMs
				* @param size size of memory
				* @param system preferred system in case of multiple profiles
				* @return entry found
				*/
				Entry FindEntry(const void* mem,ulong size,Machine::FavoredSystem system) const throw();
			};

			/**
			* iNES header format context.
			*/
			struct NesHeader
			{
				NesHeader() throw();

				/**
				* Clears settings.
				*/
				void Clear() throw();

				/**
				* Imports settings from iNES file header in memory.
				*
				* @param pointer to iNES header at least 16 byte in size
				* @param size size of memory
				* @return result code
				*/
				Result Import(const void* mem,ulong size) throw();

				/**
				* Exports settings to iNES file header in memory.
				*
				* @param pointer to iNES header at least 16 byte in size
				* @param size size of memory
				* @return result code
				*/
				Result Export(void* mem,ulong size) const throw();

				enum
				{
					MAX_PRG_ROM = 0x4000 * 0xFFFUL,
					MAX_CHR_ROM = 0x2000 * 0xFFFUL
				};

				/**
				* Region type.
				*/
				enum Region
				{
					/**
					* NTSC only.
					*/
					REGION_NTSC = 1,
					/**
					* PAL only.
					*/
					REGION_PAL,
					/**
					* Both PAL and NTSC.
					*/
					REGION_BOTH
				};

				/**
				* System type.
				*/
				enum System
				{
					/**
					* Console.
					*/
					SYSTEM_CONSOLE,
					/**
					* Vs System
					*/
					SYSTEM_VS,
					/**
					* PlayChoice-10
					*/
					SYSTEM_PC10
				};

				/**
				* PPU type.
				*/
				enum Ppu
				{
					/**
					* RP2C02 NTSC PPU.
					*/
					PPU_RP2C02 = Core::PPU_RP2C02,
					/**
					* RP2C03B RGB PPU.
					*/
					PPU_RP2C03B = Core::PPU_RP2C03B,
					/**
					* RP2C03G RGB PPU.
					*/
					PPU_RP2C03G = Core::PPU_RP2C03G,
					/**
					* RP2C04-0001 RGB PPU.
					*/
					PPU_RP2C04_0001 = Core::PPU_RP2C04_0001,
					/**
					* RP2C04-0002 RGB PPU.
					*/
					PPU_RP2C04_0002 = Core::PPU_RP2C04_0002,
					/**
					* RP2C04-0003 RGB PPU.
					*/
					PPU_RP2C04_0003 = Core::PPU_RP2C04_0003,
					/**
					* RP2C04-0004 RGB PPU.
					*/
					PPU_RP2C04_0004 = Core::PPU_RP2C04_0004,
					/**
					* RC2C03B RGB PPU.
					*/
					PPU_RC2C03B = Core::PPU_RC2C03B,
					/**
					* RC2C03C RGB PPU.
					*/
					PPU_RC2C03C = Core::PPU_RC2C03C,
					/**
					* RC2C05-01 RGB PPU.
					*/
					PPU_RC2C05_01 = Core::PPU_RC2C05_01,
					/**
					* RC2C05-02 RGB PPU.
					*/
					PPU_RC2C05_02 = Core::PPU_RC2C05_02,
					/**
					* RC2C05-03 RGB PPU.
					*/
					PPU_RC2C05_03 = Core::PPU_RC2C05_03,
					/**
					* RC2C05-04 RGB PPU.
					*/
					PPU_RC2C05_04 = Core::PPU_RC2C05_04,
					/**
					* RC2C05-05 RGB PPU.
					*/
					PPU_RC2C05_05 = Core::PPU_RC2C05_05,
					/**
					* RP2C07 PAL PPU.
					*/
					PPU_RP2C07 = Core::PPU_RP2C07
				};

				/**
				* Name-table mirroring type.
				*/
				enum Mirroring
				{
					/**
					* Horizontal mirroring.
					*/
					MIRRORING_HORIZONTAL,
					/**
					* Vertical mirroring.
					*/
					MIRRORING_VERTICAL,
					/**
					* Four-screen mirroring.
					*/
					MIRRORING_FOURSCREEN,
					/**
					* Single-screen mirroring.
					*/
					MIRRORING_SINGLESCREEN,
					/**
					* Software-controlled mirroring.
					*/
					MIRRORING_CONTROLLED
				};

				/**
				* System.
				*/
				System system;

				/**
				* Region.
				*/
				Region region;

				/**
				* PRG-ROM size.
				*/
				dword prgRom;

				/**
				* volatile PRG-RAM (aka W-RAM) size.
				*/
				dword prgRam;

				/**
				* Non-volatile PRG-RAM (aka W-RAM) size.
				*/
				dword prgNvRam;

				/**
				* CHR-ROM size.
				*/
				dword chrRom;

				/**
				* volatile CHR-RAM (aka V-RAM) size.
				*/
				dword chrRam;

				/**
				* Non-volatile CHR-RAM (aka V-RAM) size.
				*/
				dword chrNvRam;

				/**
				* PPU.
				*/
				Ppu ppu;

				/**
				* Name-table mirroring.
				*/
				Mirroring mirroring;

				/**
				* Mapper ID.
				*/
				ushort mapper;

				/**
				* Sub-mapper ID.
				*/
				uchar subMapper;

				/**
				* iNES version number.
				*/
				uchar version;

				/**
				* Vs System security bits.
				*/
				uchar security;

				/**
				* Trainer.
				*/
				bool trainer;
			};

			/**
			* Returns the current cartridge profile.
			*
			* @return pointer to current profile, NULL if no cartridge has been loaded into the system
			*/
			const Profile* GetProfile() const throw();

			/**
			* Creates a profile of an XML ROM set file.
			*
			* @param stream input stream to XML file
			* @param system preferred system in case of multiple profiles
			* @param askProfile allow callback triggering for choosing between multiple profiles
			* @param profile object to be filled
			* @return result code
			*/
			static Result ReadRomset(std::istream& stream,Machine::FavoredSystem system,bool askProfile,Profile& profile) throw();

			/**
			* Creates a profile of an iNES file.
			*
			* @param stream input stream to iNES file
			* @param system preferred system in case of multiple profiles
			* @param profile object to be filled
			* @return result code
			*/
			static Result ReadInes(std::istream& stream,Machine::FavoredSystem system,Profile& profile) throw();

			/**
			* Creates a profile of a UNIF file.
			*
			* @param stream input stream to UNIF file
			* @param system preferred system in case of multiple profiles
			* @param profile object to be filled
			* @return result code
			*/
			static Result ReadUnif(std::istream& stream,Machine::FavoredSystem system,Profile& profile) throw();

			/**
			* Returns the database interface.
			*
			* @return database interface
			*/
			Database GetDatabase() throw()
			{
				return emulator;
			}

			enum
			{
				CHOOSE_DEFAULT_PROFILE = INT_MAX
			};

			/**
			* Cartridge profile chooser callback prototype.
			*
			* @param userData optional user data
			* @param profiles pointer to an array of profiles
			* @param profileNames pointer to a wide-string array of profile names
			* @param numProfiles number of profiles to choose between
			* @return array index of chosen profile
			*/
			typedef uint (NST_CALLBACK *ChooseProfileCallback) (UserData userData,const Profile* profiles,const std::wstring* profileNames,uint numProfiles);

			/**
			* Cartridge profile chooser callback manager.
			*
			* Static object used for adding the user defined callback.
			*/
			static ChooseProfileCaller chooseProfileCallback;
		};

		/**
		* Cartridge profile chooser callback invoker.
		*
		* Used internally by the core.
		*/
		struct Cartridge::ChooseProfileCaller : Core::UserCallback<Cartridge::ChooseProfileCallback>
		{
			uint operator () (const Profile* profiles,const std::wstring* names,uint count) const
			{
				return function ? function( userdata, profiles, names, count ) : CHOOSE_DEFAULT_PROFILE;
			}
		};
	}
}

#if NST_MSVC >= 1200
#pragma warning( pop )
#endif

#endif
