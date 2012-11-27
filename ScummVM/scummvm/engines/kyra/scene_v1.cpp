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

#include "kyra/kyra_v1.h"

namespace Kyra {

int KyraEngine_v1::findWay(int x, int y, int toX, int toY, int *moveTable, int moveTableSize) {
	x &= 0xFFFC; toX &= 0xFFFC;
	y &= 0xFFFE; toY &= 0xFFFE;
	x = (int16)x; y = (int16)y; toX = (int16)toX; toY = (int16)toY;

	if (x == toY && y == toY) {
		moveTable[0] = 8;
		return 0;
	}

	int curX = x;
	int curY = y;
	int tempValue = 0;
	int lastUsedEntry = 0;
	int *pathTable1 = new int[0x7D0];
	int *pathTable2 = new int[0x7D0];
	assert(pathTable1 && pathTable2);

	while (true) {
		int newFacing = getFacingFromPointToPoint(x, y, toX, toY);
		changePosTowardsFacing(curX, curY, newFacing);

		if (curX == toX && curY == toY) {
			if (!lineIsPassable(curX, curY))
				break;
			moveTable[lastUsedEntry++] = newFacing;
			break;
		}

		if (lineIsPassable(curX, curY)) {
			if (lastUsedEntry == moveTableSize) {
				delete[] pathTable1;
				delete[] pathTable2;
				return 0x7D00;
			}
			// debug drawing
			/*if (curX >= 0 && curY >= 0 && curX < 320 && curY < 200) {
				screen()->setPagePixel(0, curX, curY, 11);
				screen()->updateScreen();
				delayWithTicks(5);
			}*/
			moveTable[lastUsedEntry++] = newFacing;
			x = curX;
			y = curY;
			continue;
		}

		int temp = 0;
		while (true) {
			newFacing = getFacingFromPointToPoint(curX, curY, toX, toY);
			changePosTowardsFacing(curX, curY, newFacing);
			// debug drawing
			/*if (curX >= 0 && curY >= 0 && curX < 320 && curY < 200) {
				screen()->setPagePixel(0, curX, curY, 8);
				screen()->updateScreen();
				delayWithTicks(5);
			}*/

			if (!lineIsPassable(curX, curY)) {
				if (curX != toX || curY != toY)
					continue;
			}

			if (curX == toX && curY == toY) {
				if (!lineIsPassable(curX, curY)) {
					tempValue = 0;
					temp = 0;
					break;
				}
			}

			temp = findSubPath(x, y, curX, curY, pathTable1, 1, 0x7D0);
			tempValue = findSubPath(x, y, curX, curY, pathTable2, 0, 0x7D0);
			if (curX == toX && curY == toY) {
				if (temp == 0x7D00 && tempValue == 0x7D00) {
					delete[] pathTable1;
					delete[] pathTable2;
					return 0x7D00;
				}
			}

			if (temp != 0x7D00 || tempValue != 0x7D00)
				break;
		}

		if (temp < tempValue) {
			if (lastUsedEntry + temp > moveTableSize) {
				delete[] pathTable1;
				delete[] pathTable2;
				return 0x7D00;
			}
			memcpy(&moveTable[lastUsedEntry], pathTable1, temp * sizeof(int));
			lastUsedEntry += temp;
		} else {
			if (lastUsedEntry + tempValue > moveTableSize) {
				delete[] pathTable1;
				delete[] pathTable2;
				return 0x7D00;
			}
			memcpy(&moveTable[lastUsedEntry], pathTable2, tempValue * sizeof(int));
			lastUsedEntry += tempValue;
		}
		x = curX;
		y = curY;
		if (curX == toX && curY == toY)
			break;
	}

	delete[] pathTable1;
	delete[] pathTable2;
	moveTable[lastUsedEntry] = 8;
	return lastUsedEntry;
}

int KyraEngine_v1::findSubPath(int x, int y, int toX, int toY, int *moveTable, int start, int end) {
	// only used for debug specific code
	//static uint16 unkTable[] = { 8, 5 };
	static const int8 facingTable1[] = {  7,  0,  1,  2,  3,  4,  5,  6,  1,  2,  3,  4,  5,  6,  7,  0 };
	static const int8 facingTable2[] = { -1,  0, -1,  2, -1,  4, -1,  6, -1,  2, -1,  4, -1,  6, -1,  0 };
	static const int8 facingTable3[] = {  2,  4,  4,  6,  6,  0,  0,  2,  6,  6,  0,  0,  2,  2,  4,  4 };
	static const int8 addPosTableX[] = { -1,  0, -1,  4, -1,  0, -1, -4, -1, -4, -1,  0, -1,  4, -1,  0 };
	static const int8 addPosTableY[] = { -1,  2, -1,  0, -1, -2, -1,  0, -1,  0, -1,  2, -1,  0, -1, -2 };

	// debug specific
	/*++unkTable[start];
	while (screen()->getPalette(0)[unkTable[start]] != 0x0F) {
		++unkTable[start];
	}*/

	int xpos1 = x, xpos2 = x;
	int ypos1 = y, ypos2 = y;
	int newFacing = getFacingFromPointToPoint(x, y, toX, toY);
	int position = 0;

	while (position != end) {
		int newFacing2 = newFacing;
		while (true) {
			changePosTowardsFacing(xpos1, ypos1, facingTable1[start * 8 + newFacing2]);
			if (!lineIsPassable(xpos1, ypos1)) {
				if (facingTable1[start * 8 + newFacing2] == newFacing)
					return 0x7D00;
				newFacing2 = facingTable1[start * 8 + newFacing2];
				xpos1 = x;
				ypos1 = y;
				continue;
			}
			newFacing = facingTable1[start * 8 + newFacing2];
			break;
		}
		// debug drawing
		/*if (xpos1 >= 0 && ypos1 >= 0 && xpos1 < 320 && ypos1 < 200) {
			screen()->setPagePixel(0, xpos1, ypos1, unkTable[start]);
			screen()->updateScreen();
			delayWithTicks(5);
		}*/
		if (newFacing & 1) {
			int temp = xpos1 + addPosTableX[newFacing + start * 8];
			if (toX == temp) {
				temp = ypos1 + addPosTableY[newFacing + start * 8];
				if (toY == temp) {
					moveTable[position++] = facingTable2[newFacing + start * 8];
					return position;
				}
			}
		}

		moveTable[position++] = newFacing;
		x = xpos1;
		y = ypos1;

		if (x == toX && y == toY)
			return position;

		if (xpos1 == xpos2 && ypos1 == ypos2)
			break;

		newFacing = facingTable3[start * 8 + newFacing];
	}

	return 0x7D00;
}

int KyraEngine_v1::getFacingFromPointToPoint(int x, int y, int toX, int toY) {
	static const int facingTable[] = {
		1, 0, 1, 2, 3, 4, 3, 2, 7, 0, 7, 6, 5, 4, 5, 6
	};

	int facingEntry = 0;
	int ydiff = y - toY;
	if (ydiff < 0) {
		++facingEntry;
		ydiff = -ydiff;
	}
	facingEntry <<= 1;

	int xdiff = toX - x;
	if (xdiff < 0) {
		++facingEntry;
		xdiff = -xdiff;
	}

	if (xdiff >= ydiff) {
		int temp = ydiff;
		ydiff = xdiff;
		xdiff = temp;

		facingEntry <<= 1;
	} else {
		facingEntry <<= 1;
		facingEntry += 1;
	}
	int temp = (ydiff + 1) >> 1;

	if (xdiff < temp) {
		facingEntry <<= 1;
		facingEntry += 1;
	} else {
		facingEntry <<= 1;
	}

	assert(facingEntry < ARRAYSIZE(facingTable));
	return facingTable[facingEntry];
}


int KyraEngine_v1::getOppositeFacingDirection(int dir) {
	switch (dir) {
	case 0:
		return 2;
	case 1:
		return 1;
	case 3:
		return 7;
	case 4:
		return 6;
	case 5:
		return 5;
	case 6:
		return 4;
	case 7:
		return 3;
	default:
		break;
	}
	return 0;
}

void KyraEngine_v1::changePosTowardsFacing(int &x, int &y, int facing) {
	x += _addXPosTable[facing];
	y += _addYPosTable[facing];
}

int KyraEngine_v1::getMoveTableSize(int *moveTable) {
	int tableSize = 0;
	if (moveTable[0] == 8)
		return 0;

	static const int facingTable[] = {
		4, 5, 6, 7, 0, 1, 2, 3
	};
	static const int unkTable[] = {
		-1, -1,  1,  2, -1,  6,  7, -1,
		-1, -1, -1, -1,  2, -1,  0, -1,
		 1, -1, -1, -1,  3,  4, -1,  0,
		 2, -1, -1, -1, -1, -1,  4, -1,
		-1,  2,  3, -1, -1, -1,  5,  6,
		 6, -1,  4, -1, -1, -1, -1, -1,
		 7,  0, -1,  4,  5, -1, -1, -1,
		-1, -1,  0, -1,  6, -1, -1, -1
	};

	int *oldPosition = moveTable;
	int *tempPosition = moveTable;
	int *curPosition = moveTable + 1;
	tableSize = 1;

	while (*curPosition != 8) {
		if (*oldPosition == facingTable[*curPosition]) {
			tableSize -= 2;
			*oldPosition = 9;
			*curPosition = 9;

			while (tempPosition != moveTable) {
				--tempPosition;
				if (*tempPosition != 9)
					break;
			}

			if (tempPosition == moveTable && *tempPosition == 9) {
				while (*tempPosition == 9)
					++tempPosition;

				if (*tempPosition == 8)
					return 0;
			}

			oldPosition = tempPosition;
			curPosition = oldPosition + 1;

			while (*curPosition == 9)
				++curPosition;
		} else if (unkTable[*curPosition + *oldPosition * 8] != -1) {
			--tableSize;
			*oldPosition = unkTable[*curPosition + *oldPosition * 8];
			*curPosition = 9;

			if (tempPosition != oldPosition) {
				curPosition = oldPosition;
				oldPosition = tempPosition;
				while (tempPosition != moveTable) {
					--tempPosition;
					if (*tempPosition != 9)
						break;
				}
			} else {
				do {
					++curPosition;
				} while (*curPosition == 9);
			}
		} else {
			tempPosition = oldPosition;
			oldPosition = curPosition;
			++tableSize;

			do {
				++curPosition;
			} while (*curPosition == 9);
		}
	}

	return tableSize;
}

} // End of namespace Kyra
