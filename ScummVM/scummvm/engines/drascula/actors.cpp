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

#include "drascula/drascula.h"

namespace Drascula {

void DrasculaEngine::placeIgor() {
	int igY = 0;

	if (currentChapter == 4) {
		igY = 138;
	} else {
		if (trackIgor == 3)
			igY = 138;
		else if (trackIgor == 1)
			igY = 76;
	}

	copyRect(1, igY, igorX, igorY, 54, 61, frontSurface, screenSurface);
}

void DrasculaEngine::placeDrascula() {
	int drX = 0;

	if (trackDrascula == 1)
		drX = 47;
	else if (trackDrascula == 0)
		drX = 1;
	else if (trackDrascula == 3 && currentChapter == 1)
		drX = 93;

	if (currentChapter == 6)
		copyRect(drX, 122, drasculaX, drasculaY, 45, 77, drawSurface2, screenSurface);
	else
		copyRect(drX, 122, drasculaX, drasculaY, 45, 77, backSurface, screenSurface);
}

void DrasculaEngine::placeBJ() {
	int bX = 0;

	if (trackBJ == 3)
		bX = 10;
	else if (trackBJ == 0)
		bX = 37;

	copyRect(bX, 99, bjX, bjY, 26, 76, drawSurface3, screenSurface);
}

void DrasculaEngine::hiccup(int counter) {
	int y = 0, trackCharacter = 0;
	if (currentChapter == 3)
		y = -1;

	do {
		counter--;

		updateEvents();
		updateRoom();
		if (currentChapter == 3)
			updateScreen(0, 0, 0, y, 320, 200, screenSurface);
		else
			updateScreen(0, 1, 0, y, 320, 198, screenSurface);

		if (trackCharacter == 0)
			y++;
		else
			y--;

		if (currentChapter == 3) {
			if (y == 1)
				trackCharacter = 1;
			if (y == -1)
				trackCharacter = 0;
		} else {
			if (y == 2)
				trackCharacter = 1;
			if (y == 0)
				trackCharacter = 0;
		}
		pause(3);
	} while (counter > 0);

	updateRoom();
	updateScreen();
}

void DrasculaEngine::startWalking() {
	characterMoved = 1;

	stepX = STEP_X;
	stepY = STEP_Y;

	if (currentChapter == 2) {
		if ((roomX < curX) && (roomY <= (curY + curHeight)))
			quadrant_1();
		else if ((roomX < curX) && (roomY > (curY + curHeight)))
			quadrant_3();
		else if ((roomX > curX + curWidth) && (roomY <= (curY + curHeight)))
			quadrant_2();
		else if ((roomX > curX + curWidth) && (roomY > (curY + curHeight)))
			quadrant_4();
		else if (roomY < curY + curHeight)
			walkUp();
		else if (roomY > curY + curHeight)
			walkDown();
	} else {
		if ((roomX < curX + curWidth / 2 ) && (roomY <= (curY + curHeight)))
			quadrant_1();
		else if ((roomX < curX + curWidth / 2) && (roomY > (curY + curHeight)))
			quadrant_3();
		else if ((roomX > curX + curWidth / 2) && (roomY <= (curY + curHeight)))
			quadrant_2();
		else if ((roomX > curX + curWidth / 2) && (roomY > (curY + curHeight)))
			quadrant_4();
		else
			characterMoved = 0;
	}
	startTime = getTime();
}

void DrasculaEngine::moveCharacters() {
	int curPos[6];
	int r;

	if (characterMoved == 1 && stepX == STEP_X) {
		for (r = 0; r < stepX; r++) {
			if (currentChapter != 2) {
				if (trackProtagonist == 0 && roomX - r == curX + curWidth / 2) {
					characterMoved = 0;
					stepX = STEP_X;
					stepY = STEP_Y;
				}
				if (trackProtagonist == 1 && roomX + r == curX + curWidth / 2) {
					characterMoved = 0;
					stepX = STEP_X;
					stepY = STEP_Y;
					curX = roomX - curWidth / 2;
					curY = roomY - curHeight;
				}
			} else if (currentChapter == 2) {
				if (trackProtagonist == 0 && roomX - r == curX) {
					characterMoved = 0;
					stepX = STEP_X;
					stepY = STEP_Y;
				}
				if (trackProtagonist == 1 && roomX + r == curX + curWidth) {
					characterMoved = 0;
					stepX = STEP_X;
					stepY = STEP_Y;
					curX = roomX - curWidth + 4;
					curY = roomY - curHeight;
				}
			}
		}
	}
	if (characterMoved == 1 && stepY == STEP_Y) {
		for (r = 0; r < stepY; r++) {
			if (trackProtagonist == 2 && roomY - r == curY + curHeight) {
				characterMoved = 0;
				stepX = STEP_X;
				stepY = STEP_Y;
			}
			if (trackProtagonist == 3 && roomY + r == curY + curHeight) {
				characterMoved = 0;
				stepX = STEP_X;
				stepY = STEP_Y;
			}
		}
	}

	if (currentChapter != 2 && currentChapter != 3) {
		if (hare_se_ve == 0) {
			increaseFrameNum();
			return;
		}
	}

	if (characterMoved == 0) {
		curPos[0] = 0;
		curPos[1] = DIF_MASK_HARE;
		curPos[2] = curX;
		curPos[3] = curY;
		if (currentChapter == 2) {
			curPos[4] = curWidth;
			curPos[5] = curHeight;
		} else {
			curPos[4] = CHARACTER_WIDTH;
			curPos[5] = CHARACTER_HEIGHT;
		}

		if (trackProtagonist == 0) {
			curPos[1] = 0;
			if (currentChapter == 2)
				copyRect(curPos[0], curPos[1], curPos[2], curPos[3], curPos[4], curPos[5],
						 extraSurface, screenSurface);
			else
				reduce_hare_chico(curPos[0], curPos[1], curPos[2], curPos[3], curPos[4], curPos[5],
									factor_red[curY + curHeight], extraSurface, screenSurface);
		} else if (trackProtagonist == 1) {
			if (currentChapter == 2)
				copyRect(curPos[0], curPos[1], curPos[2], curPos[3], curPos[4], curPos[5],
						 extraSurface, screenSurface);
			else
				reduce_hare_chico(curPos[0], curPos[1], curPos[2], curPos[3], curPos[4], curPos[5],
									factor_red[curY + curHeight], extraSurface, screenSurface);
		} else if (trackProtagonist == 2) {
			if (currentChapter == 2)
				copyRect(curPos[0], curPos[1], curPos[2], curPos[3], curPos[4], curPos[5],
						 backSurface, screenSurface);
			else
				reduce_hare_chico(curPos[0], curPos[1], curPos[2], curPos[3], curPos[4], curPos[5],
									factor_red[curY + curHeight], backSurface, screenSurface);
		} else {
			if (currentChapter == 2)
				copyRect(curPos[0], curPos[1], curPos[2], curPos[3], curPos[4], curPos[5],
						 frontSurface, screenSurface);
			else
				reduce_hare_chico(curPos[0], curPos[1], curPos[2], curPos[3], curPos[4], curPos[5],
									factor_red[curY + curHeight], frontSurface, screenSurface);
		}
	} else if (characterMoved == 1) {
		curPos[0] = _frameX[num_frame];
		curPos[1] = frame_y + DIF_MASK_HARE;
		curPos[2] = curX;
		curPos[3] = curY;
		if (currentChapter == 2) {
			curPos[4] = curWidth;
			curPos[5] = curHeight;
		} else {
			curPos[4] = CHARACTER_WIDTH;
			curPos[5] = CHARACTER_HEIGHT;
		}
		if (trackProtagonist == 0) {
			curPos[1] = 0;
			if (currentChapter == 2)
				copyRect(curPos[0], curPos[1], curPos[2], curPos[3], curPos[4], curPos[5],
						 extraSurface, screenSurface);
			else
				reduce_hare_chico(curPos[0], curPos[1], curPos[2], curPos[3], curPos[4], curPos[5],
									factor_red[curY + curHeight], extraSurface, screenSurface);
		} else if (trackProtagonist == 1) {
			if (currentChapter == 2)
				copyRect(curPos[0], curPos[1], curPos[2], curPos[3], curPos[4], curPos[5],
						 extraSurface, screenSurface);
			else
				reduce_hare_chico(curPos[0], curPos[1], curPos[2], curPos[3], curPos[4], curPos[5],
									factor_red[curY + curHeight], extraSurface, screenSurface);
		} else if (trackProtagonist == 2) {
			if (currentChapter == 2)
				copyRect(curPos[0], curPos[1], curPos[2], curPos[3], curPos[4], curPos[5],
						 backSurface, screenSurface);
			else
				reduce_hare_chico(curPos[0], curPos[1], curPos[2], curPos[3], curPos[4], curPos[5],
									factor_red[curY + curHeight], backSurface, screenSurface);
		} else {
			if (currentChapter == 2)
				copyRect(curPos[0], curPos[1], curPos[2], curPos[3], curPos[4], curPos[5],
						 frontSurface, screenSurface);
			else
				reduce_hare_chico(curPos[0], curPos[1], curPos[2], curPos[3], curPos[4], curPos[5],
									factor_red[curY + curHeight], frontSurface, screenSurface);
		}
		increaseFrameNum();
	}
}

void DrasculaEngine::quadrant_1() {
	float distanceX, distanceY;

	if (currentChapter == 2)
		distanceX = curX - roomX;
	else
		distanceX = curX + curWidth / 2 - roomX;

	distanceY = (curY + curHeight) - roomY;

	if (distanceX < distanceY) {
		curDirection = kDirectionUp;
		trackProtagonist = 2;
		stepX = (int)(distanceX / (distanceY / STEP_Y));
	} else {
		curDirection = kDirectionUp;
		trackProtagonist = 0;
		stepY = (int)(distanceY / (distanceX / STEP_X));
	}
}

void DrasculaEngine::quadrant_2() {
	float distanceX, distanceY;

	if (currentChapter == 2)
		distanceX = ABS(curX + curWidth - roomX);
	else
		distanceX = ABS(curX + curWidth / 2 - roomX);

	distanceY = (curY + curHeight) - roomY;

	if (distanceX < distanceY) {
		curDirection = kDirectionRight;
		trackProtagonist = 2;
		stepX = (int)(distanceX / (distanceY / STEP_Y));
	} else {
		curDirection = kDirectionRight;
		trackProtagonist = 1;
		stepY = (int)(distanceY / (distanceX / STEP_X));
	}
}

void DrasculaEngine::quadrant_3() {
	float distanceX, distanceY;

	if (currentChapter == 2)
		distanceX = curX - roomX;
	else
		distanceX = curX + curWidth / 2 - roomX;

	distanceY = roomY - (curY + curHeight);

	if (distanceX < distanceY) {
		curDirection = kDirectionLeft;
		trackProtagonist = 3;
		stepX = (int)(distanceX / (distanceY / STEP_Y));
	} else {
		curDirection = kDirectionLeft;
		trackProtagonist = 0;
		stepY = (int)(distanceY / (distanceX / STEP_X));
	}
}

void DrasculaEngine::quadrant_4() {
	float distanceX, distanceY;

	if (currentChapter == 2)
		distanceX = ABS(curX + curWidth - roomX);
	else
		distanceX = ABS(curX + curWidth / 2 - roomX);

	distanceY = roomY - (curY + curHeight);

	if (distanceX < distanceY) {
		curDirection = kDirectionDown;
		trackProtagonist = 3;
		stepX = (int)(distanceX / (distanceY / STEP_Y));
	} else {
		curDirection = kDirectionDown;
		trackProtagonist = 1;
		stepY = (int)(distanceY / (distanceX / STEP_X));
	}
}

void DrasculaEngine::increaseFrameNum() {
	timeDiff = getTime() - startTime;

	if (timeDiff >= 6) {
		startTime = getTime();
		num_frame++;
		if (num_frame == 6)
			num_frame = 0;

		if (curDirection == kDirectionUp) {
			curX -= stepX;
			curY -= stepY;
		} else if (curDirection == kDirectionRight) {
			curX += stepX;
			curY -= stepY;
		} else if (curDirection == kDirectionDown) {
			curX += stepX;
			curY += stepY;
		} else if (curDirection == kDirectionLeft) {
			curX -= stepX;
			curY += stepY;
		}
	}

	if (currentChapter != 2) {
		curY += (int)(curHeight - newHeight);
		curX += (int)(curWidth - newWidth);
		curHeight = (int)newHeight;
		curWidth = (int)newWidth;
	}
}

void DrasculaEngine::walkDown() {
	curDirection = kDirectionDown;
	trackProtagonist = 3;
	stepX = 0;
}

void DrasculaEngine::walkUp() {
	curDirection = kDirectionUp;
	trackProtagonist = 2;
	stepX = 0;
}

void DrasculaEngine::moveVonBraun() {
	int pos_vb[6];

	if (vonBraunHasMoved == 0) {
		pos_vb[0] = 256;
		pos_vb[1] = 129;
		pos_vb[2] = vonBraunX;
		pos_vb[3] = 66;
		pos_vb[4] = 33;
		pos_vb[5] = 69;
		if (trackVonBraun == 0)
			pos_vb[0] = 222;
		else if (trackVonBraun == 1)
			pos_vb[0] = 188;
	} else {
		pos_vb[0] = actorFrames[kFrameVonBraun];
		pos_vb[1] = (trackVonBraun == 0) ? 62 : 131;
		pos_vb[2] = vonBraunX;
		pos_vb[3] = 66;
		pos_vb[4] = 28;
		pos_vb[5] = 68;

		actorFrames[kFrameVonBraun] += 29;
		if (actorFrames[kFrameVonBraun] > 146)
			actorFrames[kFrameVonBraun] = 1;
	}

	copyRect(pos_vb[0], pos_vb[1], pos_vb[2], pos_vb[3], pos_vb[4], pos_vb[5],
			 frontSurface, screenSurface);
}

void DrasculaEngine::placeVonBraun(int pointX) {
	trackVonBraun = (pointX < vonBraunX) ? 0 : 1;
	vonBraunHasMoved = 1;

	while (!shouldQuit()) {
		updateEvents();
		updateRoom();
		updateScreen();
		if (trackVonBraun == 0) {
			vonBraunX = vonBraunX - 5;
			if (vonBraunX <= pointX)
				break;
		} else {
			vonBraunX = vonBraunX + 5;
			if (vonBraunX >= pointX)
				break;
		}
		pause(5);
	}

	vonBraunHasMoved = 0;
}



} // End of namespace Drascula
