// Meteor - A Nintendo Gameboy Advance emulator
// Copyright (C) 2009-2011 Philippe Daouadi
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

#ifndef __SWAP_H__
#define __SWAP_H__

namespace AMeteor
{
	inline uint16_t Swap16(uint16_t val)
	{
		return (val << 8) | (val >> 8);
	}

	inline uint32_t Swap32(uint32_t val)
	{
		return (val << 24) |
			((val & 0x0000FF00) << 8) |
			((val & 0x00FF0000) >> 8) |
			(val >> 24);
	}
}

#endif
