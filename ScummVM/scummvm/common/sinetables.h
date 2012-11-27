/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

#ifndef COMMON_SINETABLES_H
#define COMMON_SINETABLES_H

namespace Common {

class SineTable {
public:
	/**
	 * Construct a sine table with the specified bit precision
	 *
	 * @param bitPrecision Precision of the table, which must be in range [4, 16]
	 */
	SineTable(int bitPrecision);
	~SineTable();

	/**
	 * Get pointer to table
	 */
	const float *getTable() { return _table; }

	/**
	 * Get pointer to table
	 */
	int getPrecision() { return _bitPrecision; }

private:
	float *_table;
	int _bitPrecision;
};

} // End of namespace Common

#endif // COMMON_SINETABLES_H
