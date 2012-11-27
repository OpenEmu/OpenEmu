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

// ITE puzzle scene

#ifndef SAGA_PUZZLE_H
#define SAGA_PUZZLE_H

namespace Saga {


#define PUZZLE_SOUNDS         3622
#define PUZZLE_TOOL_SOUNDS    (PUZZLE_SOUNDS + 0)
#define PUZZLE_HINT_SOUNDS    (PUZZLE_SOUNDS + 45)
#define PUZZLE_SOLICIT_SOUNDS (PUZZLE_SOUNDS + 57)
#define PUZZLE_WHINE_SOUNDS   (PUZZLE_SOUNDS + 72)
#define PUZZLE_SAKKA_SOUNDS   (PUZZLE_SOUNDS + 87)

class Puzzle {
private:
	enum kRQStates {
		kRQNoHint = 0,
		kRQHintRequested = 1,
		kRQHintRequestedStage2 = 2,
		kRQSakkaDenies = 3,
		kRQSkipEverything = 4,
		kRQSpeaking = 5
	};

	SagaEngine *_vm;

	bool _solved;
	bool _active;
	bool _newPuzzle;
	bool _sliding;

	kRQStates _hintRqState;
	kRQStates _hintNextRqState;
	int _hintGiver;
	int _hintSpeaker;
	int _hintOffer;
	int _hintCount;
	int _helpCount;

	int _puzzlePiece;
	int _piecePriority[PUZZLE_PIECES];

	int _lang;

public:
	Puzzle(SagaEngine *vm);

	void execute();
	void exitPuzzle();

	bool isSolved() { return _solved; }
	bool isActive() { return _active; }

	void handleReply(int reply);
	void handleClick(Point mousePt);

	void movePiece(Point mousePt);

private:
	void initPieceInfo(int i, int16 curX, int16 curY, byte offX, byte offY, int16 trgX,
					   int16 trgY, uint8 flag, uint8 count, Point point0, Point point1,
					   Point point2, Point point3, Point point4, Point point5);

	static void hintTimerCallback(void *refCon);

	void solicitHint();

	void initPieces();
	void showPieces();
	void slidePiece(int x1, int y1, int x2, int y2);
	void dropPiece(Point mousePt);
	void alterPiecePriority();
	void drawCurrentPiece();

	void giveHint();
	void clearHint();

private:
	struct PieceInfo {
		int16 curX;
		int16 curY;
		byte offX;
		byte offY;
		int16 trgX;
		int16 trgY;
		uint8 flag;
		uint8 count;
		Point point[6];
	};

	PieceInfo _pieceInfo[PUZZLE_PIECES];
	int _slidePointX, _slidePointY;
	Rect _hintBox;
};

} // End of namespace Saga

#endif
