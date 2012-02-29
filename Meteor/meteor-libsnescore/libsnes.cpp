// Meteor - A Nintendo Gameboy Advance emulator
// Copyright (C) 2009-2011 Philippe Daouadi
// Copyright (C) 2011 Hans-Kristian Arntzen
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program.  If not, see <http://www.gnu.org/licenses/>.


#include "libsnes.hpp"
#include "video.hpp"
#include "audio.hpp"
#include "input.hpp"
#include "ameteor.hpp"
#include "cartmem.hpp"
#include <sstream>
#include <stdio.h>
#include <cstring>
#include <assert.h>

Video am_video;
Audio am_audio;
Input am_input;

const char* snes_library_id(void) { return "Meteor GBA"; }

unsigned snes_library_revision_major(void) { return 1; }
unsigned snes_library_revision_minor(void) { return 3; }

snes_video_refresh_t psnes_refresh;
snes_audio_sample_t psnes_sample;
snes_input_poll_t psnes_poll;
snes_input_state_t psnes_input;
static snes_environment_t psnes_environment;

void snes_set_video_refresh(snes_video_refresh_t video_refresh) { psnes_refresh = video_refresh; }
void snes_set_audio_sample(snes_audio_sample_t audio_sample) { psnes_sample = audio_sample; }
void snes_set_input_poll(snes_input_poll_t input_poll) { psnes_poll = input_poll; }
void snes_set_input_state(snes_input_state_t input_state) { psnes_input = input_state; }
void snes_set_environment(snes_environment_t environment) { psnes_environment = environment; }

void snes_set_controller_port_device(bool, unsigned) {}

void snes_set_cartridge_basename(const char *) {}

void snes_init(void)
{
	if (psnes_environment)
	{
		unsigned pitch = 240 * 2;
		psnes_environment(SNES_ENVIRONMENT_SET_PITCH, &pitch);
		snes_geometry geom = { 240, 160, 240, 160 };
		psnes_environment(SNES_ENVIRONMENT_SET_GEOMETRY, &geom);

		snes_system_timing timing = { 16777216.0 / 280896.0, 44100.0 };
		psnes_environment(SNES_ENVIRONMENT_SET_TIMING, &timing);
	}
}

void snes_power(void) { AMeteor::Reset(AMeteor::UNIT_ALL ^ AMeteor::UNIT_MEMORY_BIOS); }
void snes_reset(void) { snes_power(); }
void snes_term(void) { snes_power(); }

static void init_first_run()
{
	static bool first_run = true;
	if (first_run)
	{
		AMeteor::_memory.LoadCartInferred();
		am_video.InitAMeteor();
		am_audio.InitAMeteor();
		am_input.InitAMeteor();
		first_run = false;
	}
}

void snes_run(void)
{
	init_first_run();
	psnes_poll();
	AMeteor::Run(10000000); // We emulate until VBlank.
}

unsigned snes_serialize_size(void)
{
	init_first_run();
	std::ostringstream stream;
	AMeteor::SaveState(stream);
	unsigned serialize_size = stream.str().size();

	// If we can rewind,
	// we need to make sure that we never report a size that can potentially increase behind our backs.
	bool has_rewind = false;
	if (psnes_environment && psnes_environment(SNES_ENVIRONMENT_GET_CAN_REWIND, &has_rewind) && has_rewind)
	{
		// We want constant sized streams, and thus we have to pad.
		uint32_t current_size = *(uint32_t*)(AMeteor::CartMemData + AMeteor::CartMem::MAX_SIZE);

		// Battery is not installed (yet!),
		// accomodate for the largest possible battery size if it does get installed after this check.
		// Flash 128kB + 4 byte state variable.
		if (!current_size)
			serialize_size += AMeteor::CartMem::MAX_SIZE + sizeof(uint32_t);
	}

	return serialize_size;
}

bool snes_serialize(uint8_t *data, unsigned size)
{
	std::ostringstream stream;
	AMeteor::SaveState(stream);

	std::string s = stream.str();
	if (s.size() > size)
		return false;

	std::memcpy(data, s.data(), s.size());
	return true;
}

bool snes_unserialize(const uint8_t *data, unsigned size)
{
	std::istringstream stream;
	stream.str(std::string((char*)data, size));
	AMeteor::LoadState(stream);

	return true;
}

void snes_cheat_reset(void) {}
void snes_cheat_set(unsigned, bool, const char *) {}

bool snes_load_cartridge_normal(
		const char *, const uint8_t *data, unsigned size)
{
	AMeteor::_memory.LoadRom(data, size);

	return true;
}

bool snes_load_cartridge_bsx_slotted(
		const char *, const uint8_t *, unsigned,
		const char *, const uint8_t *, unsigned)
{
	return false;
}

bool snes_load_cartridge_bsx(
		const char *, const uint8_t *, unsigned,
		const char *, const uint8_t *, unsigned)
{
	return false;
}

bool snes_load_cartridge_sufami_turbo(
		const char *, const uint8_t *, unsigned,
		const char *, const uint8_t *, unsigned,
		const char *, const uint8_t *, unsigned)
{
	return false;
}

bool snes_load_cartridge_super_game_boy(
		const char *, const uint8_t *, unsigned,
		const char *, const uint8_t *, unsigned)
{
	return false;
}

void snes_unload_cartridge(void) {}

bool snes_get_region(void) { return SNES_REGION_NTSC; }

uint8_t* snes_get_memory_data(unsigned id)
{
	if (id != SNES_MEMORY_CARTRIDGE_RAM)
		return 0;

	return AMeteor::CartMemData;
}

unsigned snes_get_memory_size(unsigned id)
{
	if (id != SNES_MEMORY_CARTRIDGE_RAM)
		return 0;

	return AMeteor::CartMem::MAX_SIZE+4;
}
