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

#ifndef CRUISE_PERSO_H
#define CRUISE_PERSO_H

namespace Cruise {

enum {
	NUM_NODES = 20,
	NUM_PERSONS = 10
};

struct point {
	int16 x;
	int16 y;
};

struct persoStruct {
	int16 inc_droite;	// 2
	int16 inc_droite0;	// 2
	int16 inc_chemin;	// 2
	point coordinates[400];	// 1600
	int16 solution[NUM_NODES + 3][2];	//((20+3)*2*2)
	int16 inc_jo1;		// 2
	int16 inc_jo2;		// 2
	int16 dir_perso;	// 2
	int16 inc_jo0;		// 2
};

struct MovementEntry {
	int16 x;
	int16 y;
	int16 direction;
	int16 zoom;
	int16 poly;
};

extern persoStruct *persoTable[NUM_PERSONS];
extern int16 numPoly;

int cor_droite(int x1, int y1, int x2, int y2, point* outputTable);
void freePerso(int persoIdx);
void freeCTP();
void affiche_chemin(int16 persoIdx, MovementEntry &data);
int direction(int x1, int y1, int x2, int y2, int inc_jo1, int inc_jo2);

} // End of namespace Cruise

#endif
