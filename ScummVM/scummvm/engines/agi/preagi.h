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

#ifndef AGI_PREAGI_H
#define AGI_PREAGI_H

#include "agi/agi.h"

#include "audio/softsynth/pcspk.h"

namespace Agi {

// default attributes
#define IDA_DEFAULT		0x0F
#define IDA_DEFAULT_REV	0xF0

#define IDI_SND_OSCILLATOR_FREQUENCY	1193180
#define IDI_SND_TIMER_RESOLUTION		0.0182

#define kColorDefault 0x1337

#define IDI_MAX_ROW_PIC	20

enum SelectionTypes {
	kSelYesNo,
	kSelNumber,
	kSelSpace,
	kSelAnyKey,
	kSelBackspace
};

class PreAgiEngine : public AgiBase {
	int _gameId;

protected:
	void initialize();

	void pollTimer() {}
	int getKeypress() { return 0; }
	bool isKeypress() { return false; }
	void clearKeyQueue() {}

	PreAgiEngine(OSystem *syst, const AGIGameDescription *gameDesc);
	virtual ~PreAgiEngine();
	int getGameId() {
		return _gameId;
	}

	PictureMgr *_picture;

	void clearImageStack() {}
	void recordImageStackCall(uint8 type, int16 p1, int16 p2, int16 p3,
		int16 p4, int16 p5, int16 p6, int16 p7) {}
	void replayImageStackCall(uint8 type, int16 p1, int16 p2, int16 p3,
		int16 p4, int16 p5, int16 p6, int16 p7) {}
	void releaseImageStack() {}
	int saveGame(const Common::String &fileName, const Common::String &saveName) { return -1; }
	int loadGame(const Common::String &fileName, bool checkId = true) { return -1; }

	// Game
	Common::String getTargetName() { return _targetName; }

	// Screen
	void clearScreen(int attr, bool overrideDefault = true);
	void clearGfxScreen(int attr);
	void setDefaultTextColor(int attr) { _defaultColor = attr; }

	// Keyboard
	int getSelection(SelectionTypes type);

	int rnd(int hi);

	// Text
	void drawStr(int row, int col, int attr, const char *buffer);
	void drawStrMiddle(int row, int attr, const char *buffer);
	void clearTextArea();
	void clearRow(int row);
	void XOR80(char *buffer);
	void printStr(const char *szMsg);
	void printStrXOR(char *szMsg);

	// Saved Games
	Common::SaveFileManager* getSaveFileMan() { return _saveFileMan; }

	void playNote(int16 frequency, int32 length);
	void waitForTimer(int msec_delay);

private:
	int _defaultColor;

	Audio::PCSpeaker *_speakerStream;
	Audio::SoundHandle _speakerHandle;
};

} // End of namespace Agi


#endif
