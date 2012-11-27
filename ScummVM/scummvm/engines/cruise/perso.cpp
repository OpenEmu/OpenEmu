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

#include "cruise/cruise.h"
#include "cruise/cruise_main.h"
#include "common/util.h"

namespace Cruise {

persoStruct *persoTable[NUM_PERSONS];

int16 numPoly;

void freePerso(int persoIdx) {
	if (persoTable[persoIdx]) {
		MemFree(persoTable[persoIdx]);
		persoTable[persoIdx] = NULL;
	}
}

void freeCTP() {

	for (unsigned long int i = 0; i < NUM_PERSONS; i++) {
		freePerso(i);
	}

	if (_vm->_polyStruct) {
		_vm->_polyStructNorm.clear();
		_vm->_polyStructExp.clear();
		_vm->_polyStruct = NULL;
	}

	ctpVar17 = NULL;
	_vm->_polyStruct = NULL;

	strcpy((char *)currentCtpName, "");
}

int pathVar0;

unsigned int inc_jo;

int direction(int x1, int y1, int x2, int y2, int inc_jo1, int inc_jo2) {
	int h, v, h1, v1;

	h1 = x1 - x2;
	h = ABS(h1);
	v1 = y1 - y2;
	v = ABS(v1);

	if (v > h) {
		if (h > 30)
			inc_jo = inc_jo1 - inc_jo2;
		else
			inc_jo = inc_jo2;

		if (v1 < 0)
			return (2);
		else
			return (0);
	} else {
		inc_jo = inc_jo1;

		if (h1 < 0)
			return (1);
		else
			return (3);
	}
}

int cor_droite(int x1, int y1, int x2, int y2, point* outputTable) {
	int numOutput = 0;

	int dx;
	int dy;

	int mD0;
	int mD1;

	int mA0;
	int mA1;

	int bp;
	int cx;
	int si;

	int ax;
	int bx;

	outputTable[numOutput].x = x1;
	outputTable[numOutput].y = y1;
	numOutput++;

	dx = x2 - x1;
	dy = y2 - y1;

	mD0 = mD1 = 1;

	if (dx < 0) {
		dx = -dx;
		mD0 = -1;
	}

	if (dy < 0) {
		dy = -dy;
		mD1 = -1;
	}

	if (dx < dy) {
		mA0 = 0;
		bp = dx;
		cx = dy;

		mA1 = mD1;
	} else {
		mA1 = 0;
		bp = dy;
		cx = dx;

		mA0 = mD0;
	}

	bp = bp * 2;
	dx = bp - cx;
	si = dx - cx;

	ax = x1;
	bx = y1;

	while (--cx) {
		if (dx > 0) {
			ax += mD0;
			bx += mD1;
			dx += si;
		} else {
			ax += mA0;
			bx += mA1;
			dx += bp;
		}

		outputTable[numOutput].x = ax;
		outputTable[numOutput].y = bx;
		numOutput++;

	}

	flag_obstacle = 0;

	return numOutput;
}

void processActorWalk(MovementEntry &resx_y, int16 *inc_droite, int16 *inc_droite0,
                      int16 *inc_chemin, point* cor_joueur,
                      int16 solution0[NUM_NODES + 3][2], int16 *inc_jo1, int16 *inc_jo2,
                      int16 *dir_perso, int16 *inc_jo0, int16 num) {
	int x1, x2, y1, y2;
	int i, u;

	u = 0;
	inc_jo = *inc_jo0;

	i = *inc_chemin;

	if (!*inc_droite) {
		x1 = solution0[i][0];
		y1 = solution0[i][1];
		i++;
		if (solution0[i][0] != -1) {
			do {
				if (solution0[i][0] != -2) {
					x2 = solution0[i][0];
					y2 = solution0[i][1];
					if ((x1 == x2) && (y1 == y2)) {
						resx_y.x = -1;
						resx_y.y = -1;
						freePerso(num);

						return;
					}

					*inc_droite0 = cor_droite(x1, y1, x2, y2, cor_joueur);
					*dir_perso = resx_y.direction = direction(x1, y1, x2, y2, *inc_jo1, *inc_jo2);
					*inc_jo0 = inc_jo;
					u = 1;
				} else
					i++;

			} while (solution0[i][0] != -1 && !u);
		}
		if (!u) {
			resx_y.x = -1;
			resx_y.y = -1;
			freePerso(num);

			return;
		}
		*inc_chemin = i;
	}

	resx_y.x = cor_joueur[*inc_droite].x;
	resx_y.y = cor_joueur[*inc_droite].y;
	resx_y.direction = *dir_perso;
	resx_y.zoom = computeZoom(resx_y.y);

	getPixel(resx_y.x, resx_y.y);
	resx_y.poly = numPoly;

	u = subOp23(resx_y.zoom, inc_jo);
	if (!u)
		u = 1;
	*inc_droite += u;

	if ((*inc_droite) >= (*inc_droite0)) {
		*inc_droite = 0;
		resx_y.x = solution0[*inc_chemin][0];
		resx_y.y = solution0[*inc_chemin][1];
	}

}

void affiche_chemin(int16 persoIdx, MovementEntry &data) {
	persoStruct *pPerso = persoTable[persoIdx];

	ASSERT(pPerso);

	processActorWalk(data, &pPerso->inc_droite, &pPerso->inc_droite0,
	                 &pPerso->inc_chemin, pPerso->coordinates, pPerso->solution,
	                 &pPerso->inc_jo1, &pPerso->inc_jo2, &pPerso->dir_perso,
	                 &pPerso->inc_jo0, persoIdx);
}

} // End of namespace Cruise
