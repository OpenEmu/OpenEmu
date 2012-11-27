/* Copyright (C) 2003, 2004, 2005, 2006, 2008, 2009 Dean Beeler, Jerome Fisher
 * Copyright (C) 2011 Dean Beeler, Jerome Fisher, Sergey V. Mikayev
 *
 *  This program is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU Lesser General Public License as published by
 *  the Free Software Foundation, either version 2.1 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public License
 *  along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef MT32EMU_LA32RAMP_H
#define MT32EMU_LA32RAMP_H

namespace MT32Emu {

class LA32Ramp {
private:
	Bit32u current;
	unsigned int largeTarget;
	unsigned int largeIncrement;
	bool descending;

	int interruptCountdown;
	bool interruptRaised;

public:
	LA32Ramp();
	void startRamp(Bit8u target, Bit8u increment);
	Bit32u nextValue();
	bool checkInterrupt();
	void reset();
};

}

#endif /* TVA_H_ */
