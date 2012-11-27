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

#ifndef MT32EMU_POLY_H
#define MT32EMU_POLY_H

namespace MT32Emu {

class Part;

enum PolyState {
	POLY_Playing,
	POLY_Held, // This marks keys that have been released on the keyboard, but are being held by the pedal
	POLY_Releasing,
	POLY_Inactive
};

class Poly {
private:
	Part *part;
	unsigned int key;
	unsigned int velocity;
	unsigned int activePartialCount;
	bool sustain;

	PolyState state;

	Partial *partials[4];

public:
	Poly(Part *part);
	void reset(unsigned int key, unsigned int velocity, bool sustain, Partial **partials);
	bool noteOff(bool pedalHeld);
	bool stopPedalHold();
	bool startDecay();
	bool startAbort();
	void terminate();

	void backupCacheToPartials(PatchCache cache[4]);

	unsigned int getKey() const;
	unsigned int getVelocity() const;
	bool canSustain() const;
	PolyState getState() const;
	unsigned int getActivePartialCount() const;
	bool isActive() const;

	void partialDeactivated(Partial *partial);
};

}

#endif /* POLY_H_ */
