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

#include "video.hpp"
#include "ameteor.hpp"
#include "libsnes.hpp"

extern snes_video_refresh_t psnes_refresh;

void Video::InitAMeteor()
{
	AMeteor::_lcd.GetScreen().GetRenderer().SetFrameSlot(
			syg::mem_fun(*this, &Video::ShowFrame));
}

void Video::ShowFrame (const uint16_t* frame)
{
	// Convert from native XBGR1555 to libsnes XRGB1555.
	for (unsigned i = 0; i < sizeof(conv_buf) / sizeof(uint16_t); i++)
	{
		uint16_t col = frame[i];
		uint16_t b = (col >> 10) & 0x1f;
		uint16_t g = (col >>  5) & 0x1f;
		uint16_t r = (col >>  0) & 0x1f;
		conv_buf[i] = (r << 10) | (g << 5) | (b << 0);
	}

	AMeteor::Stop();
	psnes_refresh(conv_buf, 240, 160);
}
