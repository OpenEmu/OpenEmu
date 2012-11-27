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
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */

#ifndef GLOBAL_DIALOGS_H
#define GLOBAL_DIALOGS_H

#include "gui/dialog.h"
#include "common/str.h"
#include "common/stream.h"

namespace GUI {

class EditTextWidget;
class ButtonWidget;
class PicButtonWidget;

enum ButtonId {
	kBtn1Act   = 0,
	kBtn2Act   = 1,
	kBtn3Act   = 2,
	kBtn4Act   = 3,
	kBtn5Act   = 4,
	kBtn6Act   = 5,
	kBtn7Act   = 6,
	kBtn8Act   = 7,
	kBtn9Act   = 8,
	kNextAct   = 9,
	kAddAct    = 10,
	kDelAct    = 11,
	kCancelAct = 12,
	kOkAct     = 13,
	kModeAct   = 14,
	kBtn0Act   = 15,
	kNoAct     = -1
};

enum {
	kRepeatDelay = 500
};

enum {
	kMaxLineLen = 80,
	kMaxWordLen = 24,
	kMaxWord = 50
};

class PredictiveDialog : public GUI::Dialog {
public:
	PredictiveDialog();
	~PredictiveDialog();

	virtual void handleCommand(GUI::CommandSender *sender, uint32 cmd, uint32 data);
	virtual void handleKeyUp(Common::KeyState state);
	virtual void handleKeyDown(Common::KeyState state);
	virtual void handleTickle();

	const char *getResult() const { return _predictiveResult; }
private:
	struct Dict {
		char **dictLine;
		char *dictText;
		char *dictActLine; // using only for united dict...
		int32 dictLineCount;
		int32 dictTextSize;
		Common::String nameDict;
		Common::String fnameDict;
	};

	uint8 countWordsInString(const char *const str);
	void bringWordtoTop(char *str, int wordnum);
	void loadDictionary(Common::SeekableReadStream *in, Dict &dict);
	void loadAllDictionary(Dict &dict);
	void addWordToDict();
	void addWord(Dict &dict, const Common::String &word, const Common::String &code);
	bool searchWord(const char *const where, const Common::String &whatCode);
	int binarySearch(const char *const *const dictLine, const Common::String &code, const int dictLineCount);
	bool matchWord();
	void processBtnActive(ButtonId active);
	void pressEditText();

	void saveUserDictToFile();

	void mergeDicts();

	void updateHighLightedButton(ButtonId active);
private:
	Dict _unitedDict;
	Dict _predictiveDict;
	Dict _userDict;

	int _mode;
	ButtonId _lastbutton;

	bool _userDictHasChanged;

	int _wordNumber;
	uint8 _numMatchingWords;
	char _predictiveResult[40];

	Common::String _currentCode;
	Common::String _currentWord;
	Common::String _prefix;

	uint32 _curTime, _lastTime;
	ButtonId _lastPressBtn;
	ButtonId _currBtn;

	char _temp[kMaxWordLen + 1];
	int _repeatcount[kMaxWordLen];

	char *_memoryList[kMaxWord];
	int _numMemory;

	Common::String _search;

	bool _navigationwithkeys;
	bool _needRefresh;
private:
	EditTextWidget *_edittext;
	ButtonWidget   **_btns;
};

} // namespace GUI

#endif
