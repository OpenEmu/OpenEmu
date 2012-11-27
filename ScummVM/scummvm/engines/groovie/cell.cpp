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

#include "groovie/cell.h"

namespace Groovie {

CellGame::CellGame() {
	_startX = _startY = _endX = _endY = 255;

	_stack_index = _boardStackPtr = 0;
	_flag4 = false;
	_flag2 = false;
	_coeff3 = 0;

	_moveCount = 0;
}

byte CellGame::getStartX() {
	if (_startX > BOARDSIZE) {
		warning ("CellGame::getStartX: not calculated yet (%d)!", _startX);
		return 0;
	} else {
		return _startX;
	}
}

byte CellGame::getStartY() {
	if (_startY > BOARDSIZE) {
		warning ("CellGame::getStartY: not calculated yet (%d)!", _startY);
		return 6;
	} else {
		return _startY;
	}
}

byte CellGame::getEndX() {
	if (_endX > BOARDSIZE) {
		warning ("CellGame::getEndX: not calculated yet (%d)!", _endX);
		return 1;
	} else {
		return _endX;
	}
}

byte CellGame::getEndY() {
	if (_endY > BOARDSIZE) {
		warning ("CellGame::getEndY: not calculated yet (%d)!", _endY);
		return 6;
	} else {
		return _endY;
	}
}

CellGame::~CellGame() {
}

const int8 possibleMoves[][9] = {
	{ 1, 7, 8, -1 },
	{ 0, 2, 7, 8, 9, -1 },
	{ 1, 3, 8, 9, 10, -1 },
	{ 2, 4, 9, 10, 11, -1 },
	{ 3, 5, 10, 11, 12, -1 },
	{ 4, 6, 11, 12, 13, -1 }, // 5
	{ 5, 12, 13, -1 },
	{ 0, 1, 8, 14, 15, -1 },
	{ 0, 1, 2, 7, 9, 14, 15, 16, -1 },
	{ 1, 2, 3, 8, 10, 15, 16, 17, -1 },
	{ 2, 3, 4, 9, 11, 16, 17, 18, -1 }, // 10
	{ 3, 4, 5, 10, 12, 17, 18, 19, -1 },
	{ 4, 5, 6, 11, 13, 18, 19, 20, -1 },
	{ 5, 6, 12, 19, 20, -1 },
	{ 7, 8, 15, 21, 22, -1 },
	{ 7, 8, 9, 14, 16, 21, 22, 23, -1 }, // 15
	{ 8, 9, 10, 15, 17, 22, 23, 24, -1 },
	{ 9, 10, 11, 16, 18, 23, 24, 25, -1 },
	{ 10, 11, 12, 17, 19, 24, 25, 26, -1 },
	{ 11, 12, 13, 18, 20, 25, 26, 27, -1 },
	{ 12, 13, 19, 26, 27, -1 }, // 20
	{ 14, 15, 22, 28, 29, -1 },
	{ 14, 15, 16, 21, 23, 28, 29, 30, -1 },
	{ 15, 16, 17, 22, 24, 29, 30, 31, -1 },
	{ 16, 17, 18, 23, 25, 30, 31, 32, -1 },
	{ 17, 18, 19, 24, 26, 31, 32, 33, -1 }, // 25
	{ 18, 19, 20, 25, 27, 32, 33, 34, -1 },
	{ 19, 20, 26, 33, 34, -1 },
	{ 21, 22, 29, 35, 36, -1 },
	{ 21, 22, 23, 28, 30, 35, 36, 37, -1 },
	{ 22, 23, 24, 29, 31, 36, 37, 38, -1 }, // 30
	{ 23, 24, 25, 30, 32, 37, 38, 39, -1 },
	{ 24, 25, 26, 31, 33, 38, 39, 40, -1 },
	{ 25, 26, 27, 32, 34, 39, 40, 41, -1 },
	{ 26, 27, 33, 40, 41, -1 },
	{ 28, 29, 36, 42, 43, -1 }, // 35
	{ 28, 29, 30, 35, 37, 42, 43, 44, -1 },
	{ 29, 30, 31, 36, 38, 43, 44, 45, -1 },
	{ 30, 31, 32, 37, 39, 44, 45, 46, -1 },
	{ 31, 32, 33, 38, 40, 45, 46, 47, -1 },
	{ 32, 33, 34, 39, 41, 46, 47, 48, -1 }, // 40
	{ 33, 34, 40, 47, 48, -1 },
	{ 35, 36, 43, -1 },
	{ 35, 36, 37, 42, 44, -1 },
	{ 36, 37, 38, 43, 45, -1 },
	{ 37, 38, 39, 44, 46, -1 }, // 45
	{ 38, 39, 40, 45, 47, -1 },
	{ 39, 40, 41, 46, 48, -1 },
	{ 40, 41, 47, -1 }
};


const int8 strategy2[][17] = {
	{ 2, 9, 14, 15, 16, -1 },
	{ 3, 10, 14, 15, 16, 17, -1 },
	{ 0, 4, 7, 11, 14, 15, 16, 17, 18, -1 },
	{ 1, 5, 8, 12, 15, 16, 17, 18, 19, -1 },
	{ 2, 6, 9, 13, 16, 17, 18, 19, 20, -1 },
	{ 3, 10, 17, 18, 19, 20, -1 }, // 5
	{ 4, 11, 18, 19, 20, -1 },
	{ 2, 9, 16, 21, 22, 23, -1 },
	{ 3, 10, 17, 21, 22, 23, 24, -1 },
	{ 0, 4, 7, 11, 14, 18, 21, 22, 23, 24, 25, -1 },
	{ 1, 5, 8, 12, 15, 19, 22, 23, 24, 25, 26, -1 }, // 10
	{ 2, 6, 9, 13, 16, 20, 23, 24, 25, 26, 27, -1 },
	{ 3, 10, 17, 24, 25, 26, 27, -1 },
	{ 4, 11, 18, 25, 26, 27, -1 },
	{ 0, 1, 2, 9, 16, 23, 28, 29, 30, -1 },
	{ 0, 1, 2, 3, 10, 17, 24, 28, 29, 30, 31, -1 }, // 15
	{ 0, 1, 2, 3, 4, 7, 11, 14, 18, 21, 25, 28, 29, 30, 31, 32, -1 },
	{ 1, 2, 3, 4, 5, 8, 12, 15, 19, 22, 26, 29, 30, 31, 32, 33, -1 },
	{ 2, 3, 4, 5, 6, 9, 13, 16, 20, 23, 27, 30, 31, 32, 33, 34, -1 },
	{ 3, 4, 5, 6, 10, 17, 24, 31, 32, 33, 34, -1 },
	{ 4, 5, 6, 11, 18, 25, 32, 33, 34, -1 }, // 20
	{ 7, 8, 9, 16, 23, 30, 35, 36, 37, -1 },
	{ 7, 8, 9, 10, 17, 24, 31, 35, 36, 37, 38, -1 },
	{ 7, 8, 9, 10, 11, 14, 18, 21, 25, 28, 32, 35, 36, 37, 38, 39, -1 },
	{ 8, 9, 10, 11, 12, 15, 19, 22, 26, 29, 33, 36, 37, 38, 39, 40, -1 },
	{ 9, 10, 11, 12, 13, 16, 20, 23, 27, 30, 34, 37, 38, 39, 40, 41, -1 }, // 25
	{ 10, 11, 12, 13, 17, 24, 31, 38, 39, 40, 41, -1 },
	{ 11, 12, 13, 18, 25, 32, 39, 40, 41, -1 },
	{ 14, 15, 16, 23, 30, 37, 42, 43, 44, -1 },
	{ 14, 15, 16, 17, 24, 31, 38, 42, 43, 44, 45, -1 },
	{ 14, 15, 16, 17, 18, 21, 25, 28, 32, 35, 39, 42, 43, 44, 45, 46, -1 }, // 30
	{ 15, 16, 17, 18, 19, 22, 26, 29, 33, 36, 40, 43, 44, 45, 46, 47, -1 },
	{ 16, 17, 18, 19, 20, 23, 27, 30, 34, 37, 41, 44, 45, 46, 47, 48, -1 },
	{ 17, 18, 19, 20, 24, 31, 38, 45, 46, 47, 48, -1 },
	{ 18, 19, 20, 25, 32, 39, 46, 47, 48, -1 },
	{ 21, 22, 23, 30, 37, 44, -1 }, // 35
	{ 21, 22, 23, 24, 31, 38, 45, -1 },
	{ 21, 22, 23, 24, 25, 28, 32, 35, 39, 42, 46, -1 },
	{ 22, 23, 24, 25, 26, 29, 33, 36, 40, 43, 47, -1 },
	{ 23, 24, 25, 26, 27, 30, 34, 37, 41, 44, 48, -1 },
	{ 24, 25, 26, 27, 31, 38, 45, -1 }, // 40
	{ 25, 26, 27, 32, 39, 46, -1 },
	{ 28, 29, 30, 37, 44, -1 },
	{ 28, 29, 30, 31, 38, 45, -1 },
	{ 28, 29, 30, 31, 32, 35, 39, 42, 46, -1 },
	{ 29, 30, 31, 32, 33, 36, 40, 43, 47, -1 }, // 45
	{ 30, 31, 32, 33, 34, 37, 41, 44, 48, -1 },
	{ 31, 32, 33, 34, 38, 45, -1 },
	{ 32, 33, 34, 39, 46, -1 }
};

void CellGame::copyToTempBoard() {
	for (int i = 0; i < 53; ++i) {
		_tempBoard[i] = _board[i];
	}
}

void CellGame::copyFromTempBoard() {
	for (int i = 0; i < 53; ++i) {
		_board[i] = _tempBoard[i];
	}
}

void CellGame::copyToShadowBoard() {
	_board[53] = 0;
	_board[55] = 1;
	_board[56] = 0;

	for (int i = 0; i < 49; ++i) {
		_shadowBoard[i] = _board[i];
	}
}

void CellGame::pushBoard() {
	assert(_boardStackPtr < 57 * 9);

	for (int i = 0; i < 57; ++i)
		_boardStack[_boardStackPtr + i] = _board[i];
	_boardStackPtr += 57;
}

void CellGame::popBoard() {
	assert(_boardStackPtr > 0);

	_boardStackPtr -= 57;
	for (int i = 0; i < 57; ++i) {
		_board[i] = _boardStack[_boardStackPtr + i];
	}
}

void CellGame::pushShadowBoard() {
	assert(_boardStackPtr < 57 * 9);

	for (int i = 0; i < 57; ++i)
		_boardStack[_boardStackPtr + i] = _shadowBoard[i];

	_boardStackPtr += 57;
}

void CellGame::popShadowBoard() {
	assert(_boardStackPtr > 0);

	_boardStackPtr -= 57;

	for (int i = 0; i < 57; ++i) {
		_shadowBoard[i] = _boardStack[_boardStackPtr + i];
	}
}

void CellGame::clearMoves() {
	_stack_startXY[0] = _board[53];
	_stack_endXY[0] = _board[54];
	_stack_pass[0] = _board[55];

	_stack_index = 1;
}

void CellGame::pushMove() {
	_stack_startXY[_stack_index] = _board[53];
	_stack_endXY[_stack_index] = _board[54];
	_stack_pass[_stack_index] = _board[55];

	_stack_index++;
}

void CellGame::resetMove() {
	_board[53] = 0;
	_board[54] = 0;
	_board[55] = 0;
}

void CellGame::takeCells(uint16 whereTo, int8 color) {
	int cellN;
	const int8 *str;

	str = possibleMoves[whereTo];
	while (1) {
		cellN = *str++;
		if (cellN < 0)
			break;
		if (_tempBoard[cellN] > 0) {
			--_tempBoard[_tempBoard[cellN] + 48];
			_tempBoard[cellN] = color;
			++_tempBoard[color + 48];
		}
	}
}

void CellGame::countAllCells() {
	_board[49] = 0;
	_board[50] = 0;
	_board[51] = 0;
	_board[52] = 0;

	for (int i = 0; i < 49; i++) {
		switch (_board[i]) {
		case 1:	// CELL_BLUE
			_board[49]++;
			break;
		case 2:	// CELL_GREEN
			_board[50]++;
			break;
		case 3:
			_board[51]++;
			break;
		case 4:
			_board[52]++;
			break;
		}
	}
}

int CellGame::countCellsOnTempBoard(int8 color) {
	const int8 *str;
	int res = 0;
	int i;

	for (i = 0; i < 49; i++)
		_boardSum[i] = 0;

	for (i = 0; i < 49; i++) {
		if (_tempBoard[i] == color) {
			for (str = possibleMoves[i]; *str > 0; str++) {
				if (!_tempBoard[*str])
					++_boardSum[*str];
			}
		}
	}

	for (i = 0; i < 49; i++)
		res += _boardSum[i];

	return res;
}

bool CellGame::canMoveFunc1(int8 color) {
	const int8 *str;

	if (_board[55] == 1) {
		for (; _board[53] < 49; _board[53]++) {
			if (_shadowBoard[_board[53]] == color) {
				str = &possibleMoves[_board[53]][_board[56]];
				for (;_board[56] < 8; _board[56]++) {
					_board[54] = *str++;
					if (_board[54] < 0)
						break;
					if (!_shadowBoard[_board[54]]) {
						_shadowBoard[_board[54]] = -1;
						++_board[56];
						return true;
					}
				}
				_board[56] = 0;
			}
		}
		_board[53] = 0;
		_board[55] = 2;
		_board[56] = 0;
	}
	if (_board[55] == 2) {
		for (; _board[53] < 49; _board[53]++) {
			if (_shadowBoard[_board[53]] == color) {
				str = &strategy2[_board[53]][_board[56]];
				for (;_board[56] < 16; _board[56]++) {
					_board[54] = *str++;
					if (_board[54] < 0)
						break;
					if (!_board[_board[54]]) {
						++_board[56];
						return true;
					}
				}
				_board[56] = 0;
			}
		}
	}

	return false;
}

bool CellGame::canMoveFunc3(int8 color) {
	const int8 *str;

	if (_board[55] == 1) {
		for (; _board[53] < 49; _board[53]++) {
			if (_shadowBoard[_board[53]] == color) {
				str = &possibleMoves[_board[53]][_board[56]];
				for (;_board[56] < 8; _board[56]++) {
					_board[54] = *str++;
					if (_board[54] < 0)
						break;
					if (!_shadowBoard[_board[54]]) {
						_shadowBoard[_board[54]] = -1;
						++_board[56];
						return true;
					}
				}
				_board[56] = 0;
			}
		}

		_board[53] = 0;
		_board[55] = 2;
		_board[56] = 0;
		for (int i = 0; i < 49; ++i)
			_shadowBoard[i] = _board[i];
	}
	if (_board[55] == 2) {
		for (; _board[53] < 49; _board[53]++) {
			if (_shadowBoard[_board[53]] == color) {
				str = &strategy2[_board[53]][_board[56]];
				for (;_board[56] < 16; _board[56]++) {
					_board[54] = *str++;
					if (_board[54] < 0)
						break;
					if (!_shadowBoard[_board[54]]) {
						_shadowBoard[_board[54]] = -1;
						++_board[56];
						return true;
					}
				}
				_board[56] = 0;
			}
		}
	}

	return false;
}

bool CellGame::canMoveFunc2(int8 color) {
	const int8 *str;

	while (1) {
		while (_board[_board[54]]) {
			++_board[54];
			if (_board[54] >= 49)
				return false;
		}
		if (!_board[55]) {
			str = possibleMoves[_board[54]];
			while (1) {
				_board[53] = *str++;
				if (_board[53] < 0)
					break;
				if (_board[_board[53]] == color) {
					_board[55] = 1;
					return true;
				}
			}
			_board[55] = 1;
		}
		if (_board[55] == 1) {
			_board[55] = 2;
			_board[56] = 0;
		}
		if (_board[55] == 2) {
			str = &strategy2[_board[54]][_board[56]];
			for (; _board[56] < 16; _board[56]++) {
				_board[53] = *str++;
				if (_board[53] < 0)
					break;
				if (_board[_board[53]] == color) {
					++_board[56];
					return true;
				}
			}
			++_board[54];
			_board[55] = 0;
			if (_board[54] >= 49)
				break;
		}
	}
	return false;
}

void CellGame::makeMove(int8 color) {
	copyToTempBoard();
	_tempBoard[_board[54]] = color;
	++_tempBoard[color + 48];
	if (_board[55] == 2) {
		_tempBoard[_board[53]] = 0;
		--_tempBoard[color + 48];
	}
	takeCells(_board[54], color);
}

int CellGame::getBoardWeight(int8 color1, int8 color2) {
	int8 celln;
	const int8 *str;
	byte cellCnt[8];

	str = possibleMoves[_board[54]];
	cellCnt[1] = _board[49];
	cellCnt[2] = _board[50];
	cellCnt[3] = _board[51];
	cellCnt[4] = _board[52];
	if (_board[55] != 2)
		++cellCnt[color2];
	celln = *str++;

	celln = _board[celln];
	if (celln > 0) {
		--cellCnt[celln];
		++cellCnt[color2];
	}
	celln = *str++;

	celln = _board[celln];
	if (celln > 0) {
		--cellCnt[celln];
		++cellCnt[color2];
	}
	celln = *str++;

	celln = _board[celln];
	if (celln > 0) {
		--cellCnt[celln];
		++cellCnt[color2];
	}
	while (1) {
		celln = *str++;
		if (celln < 0)
			break;
		celln = _board[celln];
		if (celln > 0) {
			--cellCnt[celln];
			++cellCnt[color2];
		}
	}
	return _coeff3 + 2 * (2 * cellCnt[color1] - cellCnt[1] - cellCnt[2] - cellCnt[3] - cellCnt[4]);
}

void CellGame::chooseBestMove(int8 color) {
	int moveIndex = 0;

	if (_flag2) {
		int bestWeight = 32767;
		for (int i = 0; i < _stack_index; ++i) {
			_board[53] = _stack_startXY[i];
			_board[54] = _stack_endXY[i];
			_board[55] = _stack_pass[i];
			makeMove(color);
			int curWeight = countCellsOnTempBoard(color);
			if (curWeight <= bestWeight) {
				if (curWeight < bestWeight)
					moveIndex = 0;
				bestWeight = curWeight;
				_stack_startXY[moveIndex] = _board[53];
				_stack_endXY[moveIndex] = _board[54];
				_stack_pass[moveIndex++] = _board[55];
			}
		}
		_stack_index = moveIndex;
	}

	_startX = _stack_startXY[0] % 7;
	_startY = _stack_startXY[0] / 7;
	_endX = _stack_endXY[0] % 7;
	_endY = _stack_endXY[0] / 7;
}

int8 CellGame::calcBestWeight(int8 color1, int8 color2, uint16 depth, int bestWeight) {
	int8 res;
	int8 curColor;
	bool canMove;
	int type;
	uint16 i;
	int8 currBoardWeight;
	int8 weight;

	pushBoard();
	copyFromTempBoard();
	curColor = color2;
	for (i = 0;; ++i) {
		if (i >= 4) {
			res = _coeff3 + 2 * (2 * _board[color1 + 48] - _board[49] - _board[50] - _board[51] - _board[52]);
			popBoard();
			return res;
		}
		++curColor;
		if (curColor > 4)
			curColor = 1;

		if (_board[curColor + 48]) {
			if (_board[curColor + 48] >= 49 - _board[49] - _board[50] - _board[51] - _board[52]) {
				resetMove();
				canMove = canMoveFunc2(curColor);
				type = 1;
			} else {
				copyToShadowBoard();
				if (depth == 1) {
					canMove = canMoveFunc3(curColor);
					type = 3;
				} else {
					canMove = canMoveFunc1(curColor);
					type = 2;
				}
			}
			if (canMove)
				break;
		}
	}
	if (_flag1) {
		popBoard();
		return bestWeight + 1;
	}

	depth -= 1;
	if (depth) {
		makeMove(curColor);
		if (type == 1) {
			res = calcBestWeight(color1, curColor, depth, bestWeight);
		} else {
			pushShadowBoard();
			res = calcBestWeight(color1, curColor, depth, bestWeight);
			popShadowBoard();
		}
	} else {
		res = getBoardWeight(color1, curColor);
	}

	if ((res < bestWeight && color1 != curColor) || _flag4) {
		popBoard();
		return res;
	}

	currBoardWeight = _coeff3 + 2 * (2 * _board[color1 + 48] - _board[49] - _board[50] - _board[51] - _board[52]);
	while (1) {
		if (type == 1) {
			canMove = canMoveFunc2(curColor);
		} else if (type == 2) {
			canMove = canMoveFunc1(curColor);
		} else {
			canMove = canMoveFunc3(curColor);
		}

		if (!canMove)
			break;
		if (_flag1) {
			popBoard();
			return bestWeight + 1;
		}
		if (_board[55] == 2) {
			if (getBoardWeight(color1, curColor) == currBoardWeight)
				continue;
		}
		if (!depth) {
			weight = getBoardWeight(color1, curColor);
			if (type == 1) {
				if (_board[55] == 2)
					_board[56] = 16;
			}
		} else {
			makeMove(curColor);
			if (type != 1) {
				pushShadowBoard();
				weight = calcBestWeight(color1, curColor, depth, bestWeight);
				popShadowBoard();
			} else {
				weight = calcBestWeight(color1, curColor, depth, bestWeight);
			}
		}
		if ((weight < res && color1 != curColor) || (weight > res && color1 == curColor))
			res = weight;

		if ((res < bestWeight && color1 != curColor) || _flag4)
			break;
	}
	popBoard();

	return res;
}

int16 CellGame::doGame(int8 color, int depth) {
	bool canMove;
	int type;

	countAllCells();
	if (_board[color + 48] >= 49 - _board[49] - _board[50] - _board[51] - _board[52]) {
		resetMove();
		canMove = canMoveFunc2(color);
		type = true;
	} else {
		copyToShadowBoard();
		canMove = canMoveFunc1(color);
		type = false;
	}

	if (canMove) {
		int8 w1, w2;
		if (_board[color + 48] - _board[49] - _board[50] - _board[51] - _board[52] == 0)
			depth = 0;
		_coeff3 = 0;
		if (_board[55] == 1)
			_coeff3 = 1;
		clearMoves();
		if (depth) {
			makeMove(color);
			_flag4 = false;
			if (type) {
				w2 = calcBestWeight(color, color, depth, -127);
			} else {
				pushShadowBoard();
				w2 = calcBestWeight(color, color, depth, -127);
				popShadowBoard();
			}
		} else {
			w2 = getBoardWeight(color, color);
		}
		int8 currBoardWeight = 2 * (2 * _board[color + 48] - _board[49] - _board[50] - _board[51] - _board[52]);
		while (1) {
			if (type)
				canMove = canMoveFunc2(color);
			else
				canMove = canMoveFunc1(color);

			if (!canMove)
				break;
			if (_flag1)
				break;
			_coeff3 = 0;
			if (_board[55] == 2) {
				if (getBoardWeight(color, color) == currBoardWeight)
					continue;
			}
			if (_board[55] == 1)
				_coeff3 = 1;
			if (depth) {
				makeMove(color);
				_flag4 = false;
				if (type) {
					w1 = calcBestWeight(color, color, depth, w2);
				} else {
					pushShadowBoard();
					w1 = calcBestWeight(color, color, depth, w2);
					popShadowBoard();
				}
			} else {
				w1 = getBoardWeight(color, color);
			}
			if (w1 == w2)
				pushMove();

			if (w1 > w2) {
				clearMoves();
				w2 = w1;
			}
		}
		chooseBestMove(color);
		return 1;
	}

	return 0;
}

const int8 depths[] = { 1, 1, 1, 2, 1, 1, 2, 2, 1, 2, 2, 2, 3, 2, 2, 3, 3, 2, 3, 3, 3 };

int16 CellGame::calcMove(int8 color, uint16 depth) {
	int result = 0;

	_flag1 = false;
	++_moveCount;
	if (depth) {
		if (depth == 1) {
			_flag2 = true;
			result = doGame(color, 0);
		} else {
			int newDepth;

			newDepth = depths[3 * (depth - 2) + _moveCount % 3];
			_flag2 = true;
			if (newDepth >= 20) {
				assert(0); // This branch is not implemented
			} else {
				result = doGame(color, newDepth);
			}
		}
	} else {
		_flag2 = false;
		result = doGame(color, depth);
	}
	return result;
}

int CellGame::playStauf(byte color, uint16 depth, byte *scriptBoard) {
	int i;

	for (i = 0; i < 49; i++, scriptBoard++) {
		_board[i] = 0;
		if (*scriptBoard == 50)
			_board[i] = 1;
		if (*scriptBoard == 66)
			_board[i] = 2;
	}
	for (i = 49; i < 57; i++)
		_board[i] = 0;

	return calcMove(color, depth);
}


} // End of Groovie namespace
