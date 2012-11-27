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

#ifndef GROOVIE_CELL_H
#define GROOVIE_CELL_H

#include "common/textconsole.h"

#define BOARDSIZE 7
#define CELL_CLEAR 0
#define CELL_BLUE 1
#define CELL_GREEN 2

namespace Groovie {

class GroovieEngine;
class Script;

class CellGame {
public:
	CellGame();
	~CellGame();
	byte getStartX();
	byte getStartY();
	byte getEndX();
	byte getEndY();
	int playStauf(byte color, uint16 depth, byte *scriptBoard);

private:
	void copyToTempBoard();
	void copyFromTempBoard();
	void copyToShadowBoard();
	void pushBoard();
	void popBoard();
	void pushShadowBoard();
	void popShadowBoard();
	void clearMoves();
	void pushMove();
	void resetMove();
	bool canMoveFunc1(int8 color);
	bool canMoveFunc2(int8 color);
	bool canMoveFunc3(int8 color);
	void takeCells(uint16 whereTo, int8 color);
	void countAllCells();
	int countCellsOnTempBoard(int8 color);
	void makeMove(int8 color);
	int getBoardWeight(int8 color1, int8 color2);
	void chooseBestMove(int8 color);
	int8 calcBestWeight(int8 color1, int8 color2, uint16 depth, int bestWeight);
	int16 doGame(int8 color, int depth);
	int16 calcMove(int8 color, uint16 depth);

	byte _startX;
	byte _startY;
	byte _endX;
	byte _endY;

	int8 _board[57];
	int8 _tempBoard[58];
	int8 _shadowBoard[64];
	int8 _boardStack[570];
	int _boardStackPtr;

	int8 _boardSum[58];

	int8 _stack_startXY[128];
	int8 _stack_endXY[128];
	int8 _stack_pass[128];
	int _stack_index;

	int _coeff3;
	bool _flag1, _flag2, _flag4;
	int _moveCount;
};

} // End of Groovie namespace

#endif // GROOVIE_CELL_H
