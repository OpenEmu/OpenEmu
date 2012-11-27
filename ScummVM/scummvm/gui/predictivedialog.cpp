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

#include "gui/predictivedialog.h"
#include "gui/widget.h"
#include "gui/widgets/edittext.h"
#include "gui/gui-manager.h"

#include "common/config-manager.h"
#include "common/translation.h"
#include "common/events.h"
#include "common/debug.h"
#include "common/system.h"
#include "common/keyboard.h"
#include "common/file.h"
#include "common/savefile.h"

#ifdef __DS__
#include "backends/platform/ds/arm9/source/wordcompletion.h"
#endif

namespace GUI {

enum {
	kCancelCmd = 'CNCL',
	kOkCmd     = '__OK',
	kBut1Cmd   = 'BUT1',
	kBut2Cmd   = 'BUT2',
	kBut3Cmd   = 'BUT3',
	kBut4Cmd   = 'BUT4',
	kBut5Cmd   = 'BUT5',
	kBut6Cmd   = 'BUT6',
	kBut7Cmd   = 'BUT7',
	kBut8Cmd   = 'BUT8',
	kBut9Cmd   = 'BUT9',
	kBut0Cmd   = 'BUT0',
	kNextCmd   = 'NEXT',
	kAddCmd    = '_ADD',
	kModeCmd   = 'MODE',
	kDelCmd    = '_DEL',
	kTestCmd   = 'TEST'
};

enum {
	kModePre = 0,
	kModeNum = 1,
	kModeAbc = 2
};

PredictiveDialog::PredictiveDialog() : Dialog("Predictive") {
	new StaticTextWidget(this, "Predictive.Headline", "Enter Text");

	_btns = (ButtonWidget **)calloc(1, sizeof(ButtonWidget *) * 16);

	_btns[kCancelAct] = new ButtonWidget(this, "Predictive.Cancel",  _("Cancel")   , 0, kCancelCmd);
	_btns[kOkAct] = 	new ButtonWidget(this, "Predictive.OK",      _("Ok")       , 0, kOkCmd);
	_btns[kBtn1Act] = 	new ButtonWidget(this, "Predictive.Button1", "1  `-.&" , 0, kBut1Cmd);
	_btns[kBtn2Act] = 	new ButtonWidget(this, "Predictive.Button2", "2  abc"     , 0, kBut2Cmd);
	_btns[kBtn3Act] = 	new ButtonWidget(this, "Predictive.Button3", "3  def"     , 0, kBut3Cmd);
	_btns[kBtn4Act] = 	new ButtonWidget(this, "Predictive.Button4", "4  ghi"     , 0, kBut4Cmd);
	_btns[kBtn5Act] = 	new ButtonWidget(this, "Predictive.Button5", "5  jkl"     , 0, kBut5Cmd);
	_btns[kBtn6Act] = 	new ButtonWidget(this, "Predictive.Button6", "6  mno"     , 0, kBut6Cmd);
	_btns[kBtn7Act] = 	new ButtonWidget(this, "Predictive.Button7", "7  pqrs"    , 0, kBut7Cmd);
	_btns[kBtn8Act] = 	new ButtonWidget(this, "Predictive.Button8", "8  tuv"     , 0, kBut8Cmd);
	_btns[kBtn9Act] = 	new ButtonWidget(this, "Predictive.Button9", "9  wxyz"    , 0, kBut9Cmd);
	_btns[kBtn0Act] = 	new ButtonWidget(this, "Predictive.Button0", "0"        , 0, kBut0Cmd);
  	// I18N: You must leave "#" as is, only word 'next' is translatable
	_btns[kNextAct] = 	new ButtonWidget(this, "Predictive.Next",    _("#  next") , 0, kNextCmd);
	_btns[kAddAct] = 	new ButtonWidget(this, "Predictive.Add",     _("add") , 0, kAddCmd);
	_btns[kAddAct]->setEnabled(false);

  #ifndef DISABLE_FANCY_THEMES
	_btns[kDelAct] = new PicButtonWidget(this, "Predictive.Delete", _("Delete char"), kDelCmd);
	((PicButtonWidget *)_btns[kDelAct])->useThemeTransparency(true);
	((PicButtonWidget *)_btns[kDelAct])->setGfx(g_gui.theme()->getImageSurface(ThemeEngine::kImageDelbtn));
  #endif
	_btns[kDelAct] = new ButtonWidget(this, "Predictive.Delete" , _("<") , 0, kDelCmd);
  	// I18N: Pre means 'Predictive', leave '*' as is
	_btns[kModeAct] = new ButtonWidget(this, "Predictive.Pre", _("*  Pre"), 0, kModeCmd);
	_edittext = new EditTextWidget(this, "Predictive.Word", _search, 0, 0, 0);

	_userDictHasChanged = false;

	_predictiveDict.nameDict = "predictive_dictionary";
	_predictiveDict.fnameDict = "pred.dic";
	_predictiveDict.dictActLine = NULL;

	_userDict.nameDict = "user_dictionary";
	_userDict.fnameDict = "user.dic";
	_userDict.dictActLine = NULL;

	_unitedDict.nameDict = "";
	_unitedDict.fnameDict = "";

	_predictiveDict.dictLine = NULL;
	_predictiveDict.dictText = NULL;
	_predictiveDict.dictLineCount = 0;

	if (!_predictiveDict.dictText) {
		loadAllDictionary(_predictiveDict);
		if (!_predictiveDict.dictText)
			debug("Predictive Dialog: pred.dic not loaded");
	}

	_userDict.dictLine = NULL;
	_userDict.dictText = NULL;
	_userDict.dictTextSize = 0;
	_userDict.dictLineCount = 0;

	if (!_userDict.dictText) {
		loadAllDictionary(_userDict);
		if (!_userDict.dictText)
			debug("Predictive Dialog: user.dic not loaded");
	}

	mergeDicts();

	_unitedDict.dictActLine = NULL;
	_unitedDict.dictText = NULL;

	memset(_repeatcount, 0, sizeof(_repeatcount));

	_prefix.clear();
	_currentCode.clear();
	_currentWord.clear();
	_wordNumber = 0;
	_numMatchingWords = 0;

	_lastbutton = kNoAct;
	_mode = kModePre;

	_lastTime = 0;
	_curTime = 0;
	_lastPressBtn = kNoAct;

	_memoryList[0] = _predictiveDict.dictText;
	_memoryList[1] = _userDict.dictText;
	_numMemory = 0;

	_navigationwithkeys = false;
}

PredictiveDialog::~PredictiveDialog() {
	for (int i = 0; i < _numMemory; i++) {
		free(_memoryList[i]);
	}
	free(_userDict.dictLine);
	free(_predictiveDict.dictLine);
	free(_unitedDict.dictLine);

	free(_btns);
}

void PredictiveDialog::saveUserDictToFile() {
	if (_userDictHasChanged) {
		ConfMan.registerDefault("user_dictionary", "user.dic");

		Common::OutSaveFile *file = g_system->getSavefileManager()->openForSaving(ConfMan.get("user_dictionary"));

		for (int i = 0; i < _userDict.dictLineCount; i++) {
			file->writeString(_userDict.dictLine[i]);
			file->writeString("\n");
		}

		file->finalize();
		delete file;
	}
}

void PredictiveDialog::handleKeyUp(Common::KeyState state) {
	if (_currBtn != kNoAct && !_needRefresh) {
		_btns[_currBtn]->startAnimatePressedState();
		processBtnActive(_currBtn);
	}
}

void PredictiveDialog::handleKeyDown(Common::KeyState state) {
	_currBtn = kNoAct;
	_needRefresh = false;

	if (getFocusWidget() == _edittext) {
		setFocusWidget(_btns[kAddAct]);
	}

	if (_lastbutton == kNoAct) {
		_lastbutton = kBtn5Act;
	}

	switch (state.keycode) {
	case Common::KEYCODE_ESCAPE:
		saveUserDictToFile();
		close();
		return;
	case Common::KEYCODE_LEFT:
		_navigationwithkeys = true;
		if (_lastbutton == kBtn1Act || _lastbutton == kBtn4Act || _lastbutton == kBtn7Act)
			_currBtn = ButtonId(_lastbutton + 2);
		else if (_lastbutton == kDelAct)
			_currBtn = kBtn1Act;
		else if (_lastbutton == kModeAct)
			_currBtn = kNextAct;
		else if (_lastbutton == kNextAct)
			_currBtn = kBtn0Act;
		else if (_lastbutton == kAddAct)
			_currBtn = kOkAct;
		else if (_lastbutton == kCancelAct)
			_currBtn = kAddAct;
		else
			_currBtn = ButtonId(_lastbutton - 1);


		if (_mode != kModeAbc && _lastbutton == kCancelAct)
			_currBtn = kOkAct;

		_needRefresh = true;
		break;
	case Common::KEYCODE_RIGHT:
		_navigationwithkeys = true;
		if (_lastbutton == kBtn3Act || _lastbutton == kBtn6Act || _lastbutton == kBtn9Act || _lastbutton == kOkAct)
			_currBtn = ButtonId(_lastbutton - 2);
		else if (_lastbutton == kDelAct)
			_currBtn = kBtn3Act;
		else if (_lastbutton == kBtn0Act)
			_currBtn = kNextAct;
		else if (_lastbutton == kNextAct)
			_currBtn = kModeAct;
		else if (_lastbutton == kAddAct)
			_currBtn = kCancelAct;
		else if (_lastbutton == kOkAct)
			_currBtn = kAddAct;
		else
			_currBtn = ButtonId(_lastbutton + 1);

		if (_mode != kModeAbc && _lastbutton == kOkAct)
			_currBtn = kCancelAct;
		_needRefresh = true;
		break;
	case Common::KEYCODE_UP:
		_navigationwithkeys = true;
		if (_lastbutton <= kBtn3Act)
			_currBtn = kDelAct;
		else if (_lastbutton == kDelAct)
			_currBtn = kOkAct;
		else if (_lastbutton == kModeAct)
			_currBtn = kBtn7Act;
		else if (_lastbutton == kBtn0Act)
			_currBtn = kBtn8Act;
		else if (_lastbutton == kNextAct)
			_currBtn = kBtn9Act;
		else if (_lastbutton == kAddAct)
			_currBtn = kModeAct;
		else if (_lastbutton == kCancelAct)
			_currBtn = kBtn0Act;
		else if (_lastbutton == kOkAct)
			_currBtn = kNextAct;
		else
			_currBtn = ButtonId(_lastbutton - 3);
		_needRefresh = true;
		break;
	case Common::KEYCODE_DOWN:
		_navigationwithkeys = true;
		if (_lastbutton == kDelAct)
			_currBtn = kBtn3Act;
		else if (_lastbutton == kBtn7Act)
			_currBtn = kModeAct;
		else if (_lastbutton == kBtn8Act)
			_currBtn = kBtn0Act;
		else if (_lastbutton == kBtn9Act)
			_currBtn = kNextAct;
		else if (_lastbutton == kModeAct)
			_currBtn = kAddAct;
		else if (_lastbutton == kBtn0Act)
			_currBtn = kCancelAct;
		else if (_lastbutton == kNextAct)
			_currBtn = kOkAct;
		else if (_lastbutton == kAddAct || _lastbutton == kCancelAct || _lastbutton == kOkAct)
			_currBtn = kDelAct;
		else
			_currBtn = ButtonId(_lastbutton + 3);

		if (_mode != kModeAbc && _lastbutton == kModeAct)
			_currBtn = kCancelAct;

		_needRefresh = true;
		break;
	case Common::KEYCODE_KP_ENTER:
	case Common::KEYCODE_RETURN:
		if (state.flags & Common::KBD_CTRL) {
			_currBtn = kOkAct;
			break;
		}
		if (_navigationwithkeys) {
			// when the user has utilized arrow key navigation,
			// interpret enter as 'click' on the _currBtn button
			_currBtn = _lastbutton;
			_needRefresh = false;
		} else {
			// else it is a shortcut for 'Ok'
			_currBtn = kOkAct;
		}
		break;
	case Common::KEYCODE_KP_PLUS:
		_currBtn = kAddAct;
		break;
	case Common::KEYCODE_BACKSPACE:
	case Common::KEYCODE_KP_MINUS:
		_currBtn = kDelAct;
		break;
	case Common::KEYCODE_KP_DIVIDE:
		_currBtn = kNextAct;
		break;
	case Common::KEYCODE_KP_MULTIPLY:
		_currBtn = kModeAct;
		break;
	case Common::KEYCODE_KP0:
		_currBtn = kBtn0Act;
		break;
	case Common::KEYCODE_KP1:
	case Common::KEYCODE_KP2:
	case Common::KEYCODE_KP3:
	case Common::KEYCODE_KP4:
	case Common::KEYCODE_KP5:
	case Common::KEYCODE_KP6:
	case Common::KEYCODE_KP7:
	case Common::KEYCODE_KP8:
	case Common::KEYCODE_KP9:
		_currBtn = ButtonId(state.keycode - Common::KEYCODE_KP1);
		break;
	default:
		Dialog::handleKeyDown(state);
	}

	if (_lastbutton != _currBtn)
		_btns[_lastbutton]->stopAnimatePressedState();

	if (_currBtn != kNoAct && !_needRefresh)
		_btns[_currBtn]->setPressedState();
	else
		updateHighLightedButton(_currBtn);
}

void PredictiveDialog::updateHighLightedButton(ButtonId act) {
	if (_currBtn != kNoAct) {
		_btns[_lastbutton]->setHighLighted(false);
		_lastbutton = act;
		_btns[_lastbutton]->setHighLighted(true);
	}
}

void PredictiveDialog::handleCommand(CommandSender *sender, uint32 cmd, uint32 data) {
	_currBtn = kNoAct;

	_navigationwithkeys = false;

	if (_lastbutton != kNoAct)
		_btns[_lastbutton]->setHighLighted(false);

	switch (cmd) {
	case kDelCmd:
		_currBtn = kDelAct;
		break;
	case kNextCmd:
		_currBtn = kNextAct;
		break;
	case kAddCmd:
		_currBtn = kAddAct;
		break;
	case kModeCmd:
		_currBtn = kModeAct;
		break;
	case kBut1Cmd:
		_currBtn = kBtn1Act;
		break;
	case kBut2Cmd:
		_currBtn = kBtn2Act;
		break;
	case kBut3Cmd:
		_currBtn = kBtn3Act;
		break;
	case kBut4Cmd:
		_currBtn = kBtn4Act;
		break;
	case kBut5Cmd:
		_currBtn = kBtn5Act;
		break;
	case kBut6Cmd:
		_currBtn = kBtn6Act;
		break;
	case kBut7Cmd:
		_currBtn = kBtn7Act;
		break;
	case kBut8Cmd:
		_currBtn = kBtn8Act;
		break;
	case kBut9Cmd:
		_currBtn = kBtn9Act;
		break;
	case kBut0Cmd:
		_currBtn = kBtn0Act;
		break;
	case kCancelCmd:
		saveUserDictToFile();
		close();
		return;
	case kOkCmd:
		_currBtn = kOkAct;
		break;
	default:
		Dialog::handleCommand(sender, cmd, data);
	}

	if (_currBtn != kNoAct) {
		processBtnActive(_currBtn);
	}
}

void PredictiveDialog::processBtnActive(ButtonId button) {
	uint8 x;
	static const char *const buttonStr[] = {
		"1", "2", "3",
		"4", "5", "6",
		"7", "8", "9",
		     "0"
	};

	static const char *const buttons[] = {
		"'-.&",  "abc", "def",
		"ghi",  "jkl", "mno",
		"pqrs", "tuv", "wxyz",
		"next",    "add",
		"<",
		"Cancel",  "OK",
		"Pre", "(0) ", NULL
	};

	if (_mode == kModeAbc) {
		if (button >= kBtn1Act && button <= kBtn9Act) {
			if (!_lastTime)
				_lastTime = g_system->getMillis();
			if (_lastPressBtn == button) {
				_curTime = g_system->getMillis();
				if ((_curTime - _lastTime) < kRepeatDelay) {
					button = kNextAct;
					_lastTime = _curTime;
				} else {
					_lastTime = 0;
				}
			} else {
				_lastPressBtn = button;
				_lastTime = g_system->getMillis();
			}
		}
	}

	if (button >= kBtn1Act) {
		_lastbutton = button;
		if (button == kBtn0Act && _mode != kModeNum) { // Space
			// bring MRU word at the top of the list when changing words
			if (_mode == kModePre && _unitedDict.dictActLine && _numMatchingWords > 1 && _wordNumber != 0)
				bringWordtoTop(_unitedDict.dictActLine, _wordNumber);

			strncpy(_temp, _currentWord.c_str(), _currentCode.size());
			_temp[_currentCode.size()] = 0;
			_prefix += _temp;
			_prefix += " ";
			_currentCode.clear();
			_currentWord.clear();
			_numMatchingWords = 0;
			memset(_repeatcount, 0, sizeof(_repeatcount));
			_lastTime = 0;
			_lastPressBtn = kNoAct;
			_curTime = 0;
		} else if (button < kNextAct || button == kDelAct || button == kBtn0Act) { // number or backspace
			if (button == kDelAct) { // backspace
				if (_currentCode.size()) {
					_repeatcount[_currentCode.size() - 1] = 0;
					_currentCode.deleteLastChar();
					if (_currentCode.empty())
						_currentWord.clear();
				} else {
					if (_prefix.size())
						_prefix.deleteLastChar();
				}
			} else if (_prefix.size() + _currentCode.size() < kMaxWordLen - 1) { // don't overflow the dialog line
				if (button == kBtn0Act) { // zero
					_currentCode += buttonStr[9];
				} else {
					_currentCode += buttonStr[button];
				}
			}

			switch (_mode) {
			case kModeNum:
				_currentWord = _currentCode;
				break;
			case kModePre:
				if (!matchWord() && _currentCode.size()) {
					_currentCode.deleteLastChar();
					matchWord();
				}
				_numMatchingWords = countWordsInString(_unitedDict.dictActLine);
				break;
			case kModeAbc:
				for (x = 0; x < _currentCode.size(); x++)
					if (_currentCode[x] >= '1')
						_temp[x] = buttons[_currentCode[x] - '1'][_repeatcount[x]];
				_temp[_currentCode.size()] = 0;
				_currentWord = _temp;
			}
		} else if (button == kNextAct) { // next
			if (_mode == kModePre) {
				if (_unitedDict.dictActLine && _numMatchingWords > 1) {
					_wordNumber = (_wordNumber + 1) % _numMatchingWords;
					char tmp[kMaxLineLen];
					strncpy(tmp, _unitedDict.dictActLine, kMaxLineLen);
					tmp[kMaxLineLen - 1] = 0;
					char *tok = strtok(tmp, " ");
					for (uint8 i = 0; i <= _wordNumber; i++)
						tok = strtok(NULL, " ");
					_currentWord = Common::String(tok, _currentCode.size());
				}
			} else if (_mode == kModeAbc) {
				x = _currentCode.size();
				if (x) {
					if (_currentCode.lastChar() == '1' || _currentCode.lastChar() == '7' || _currentCode.lastChar() == '9')
						_repeatcount[x - 1] = (_repeatcount[x - 1] + 1) % 4;
					else
						_repeatcount[x - 1] = (_repeatcount[x - 1] + 1) % 3;

					if (_currentCode.lastChar() >= '1')
						_currentWord.setChar(buttons[_currentCode[x - 1] - '1'][_repeatcount[x - 1]], x - 1);
				}
			}
		} else if (button == kAddAct) { // add
			if (_mode == kModeAbc)
				addWordToDict();
			else
				debug("Predictive Dialog: button Add doesn't work in this mode");
		} else if (button == kOkAct) { // Ok
			// bring MRU word at the top of the list when ok'ed out of the dialog
			if (_mode == kModePre && _unitedDict.dictActLine && _numMatchingWords > 1 && _wordNumber != 0)
				bringWordtoTop(_unitedDict.dictActLine, _wordNumber);
		} else if (button == kModeAct) { // Mode
			_mode++;
			_btns[kAddAct]->setEnabled(false);
			if (_mode > kModeAbc) {
				_mode = kModePre;
				// I18N: Pre means 'Predictive', leave '*' as is
				_btns[kModeAct]->setLabel("*  Pre");
			} else if (_mode == kModeNum) {
				// I18N: 'Num' means Numbers
				_btns[kModeAct]->setLabel("*  Num");
			} else {
				// I18N: 'Abc' means Latin alphabet input
				_btns[kModeAct]->setLabel("*  Abc");
				_btns[kAddAct]->setEnabled(true);
			}

			// truncate current input at mode change
			strncpy(_temp, _currentWord.c_str(), _currentCode.size());
			_temp[_currentCode.size()] = 0;
			_prefix += _temp;
			_currentCode.clear();
			_currentWord.clear();
			memset(_repeatcount, 0, sizeof(_repeatcount));

			_lastTime = 0;
			_lastPressBtn = kNoAct;
			_curTime = 0;
		}
	}

	pressEditText();

	if (button == kOkAct)
		close();

 	if (button == kCancelAct) {
 		saveUserDictToFile();
 		close();
 	}
}

void PredictiveDialog::handleTickle() {
	if (_lastTime) {
		if ((_curTime - _lastTime) > kRepeatDelay) {
			_lastTime = 0;
		}
	}

	if (getTickleWidget()) {
		getTickleWidget()->handleTickle();
	}
}

void PredictiveDialog::mergeDicts() {
	_unitedDict.dictLineCount  = _predictiveDict.dictLineCount + _userDict.dictLineCount;
	_unitedDict.dictLine = (char **)calloc(1, sizeof(char *) * _unitedDict.dictLineCount);

	if (!_unitedDict.dictLine) {
		debug("Predictive Dialog: cannot allocate memory for united dic");
		return;
	}

	int lenUserDictCode, lenPredictiveDictCode, lenCode;
	int i, j, k;
	i = j = k = 0;

	while ((i < _userDict.dictLineCount) && (j < _predictiveDict.dictLineCount)) {
		lenUserDictCode = strchr(_userDict.dictLine[i], ' ') - _userDict.dictLine[i];
		lenPredictiveDictCode = strchr(_predictiveDict.dictLine[j], ' ') - _predictiveDict.dictLine[j];
		lenCode = (lenUserDictCode >= lenPredictiveDictCode) ? lenUserDictCode : lenPredictiveDictCode;
		if (strncmp(_userDict.dictLine[i], _predictiveDict.dictLine[j], lenCode) >= 0) {
			_unitedDict.dictLine[k++] = _predictiveDict.dictLine[j++];
		} else {
			_unitedDict.dictLine[k++] = _userDict.dictLine[i++];
		}
	}

	while (i < _userDict.dictLineCount) {
		_unitedDict.dictLine[k++] = _userDict.dictLine[i++];
	}

	while (j < _predictiveDict.dictLineCount) {
		_unitedDict.dictLine[k++] = _predictiveDict.dictLine[j++];
	}
}

uint8 PredictiveDialog::countWordsInString(const char *const str) {
	// Count the number of (space separated) words in the given string.
	const char *ptr;

	if (!str)
		return 0;

	ptr = strchr(str, ' ');
	if (!ptr) {
		debug("Predictive Dialog: Invalid dictionary line");
		return 0;
	}

	uint8 num = 1;
	ptr++;
	while ((ptr = strchr(ptr, ' '))) {
		ptr++;
		num++;
	}
	return num;
}

void PredictiveDialog::bringWordtoTop(char *str, int wordnum) {
	// This function reorders the words on the given pred.dic line
	// by moving the word at position 'wordnum' to the front (that is, right behind
	// right behind the numerical code word at the start of the line).
	Common::Array<Common::String> words;
	char buf[kMaxLineLen];

	if (!str)
		return;
	strncpy(buf, str, kMaxLineLen);
	buf[kMaxLineLen - 1] = 0;
	char *word = strtok(buf, " ");
	if (!word) {
		debug("Predictive Dialog: Invalid dictionary line");
		return;
	}

	words.push_back(word);
	while ((word = strtok(NULL, " ")) != NULL)
		words.push_back(word);
	words.insert_at(1, words.remove_at(wordnum + 1));

	Common::String tmp;
	for (uint8 i = 0; i < words.size(); i++)
		tmp += words[i] + " ";
	tmp.deleteLastChar();
	memcpy(str, tmp.c_str(), strlen(str));
}

int PredictiveDialog::binarySearch(const char *const *const dictLine, const Common::String &code, const int dictLineCount) {
	int hi = dictLineCount - 1;
	int lo = 0;
	int line = 0;
	while (lo <= hi) {
		line = (lo + hi) / 2;
		int cmpVal = strncmp(dictLine[line], code.c_str(), code.size());
		if (cmpVal > 0)
			hi = line - 1;
		else if (cmpVal < 0)
			lo = line + 1;
		else {
			break;
		}
	}

	if (hi < lo) {
		return -(lo + 1);
	} else {
		return line;
	}
}

bool PredictiveDialog::matchWord() {
	// If no text has been entered, then there is no match.
	if (_currentCode.empty())
		return false;

	// If the currently entered text is too long, it cannot match anything.
	if (_currentCode.size() > kMaxWordLen)
		return false;

	// The entries in the dictionary consist of a code, a space, and then
	// a space-separated list of words matching this code.
	// To ex_currBtnly match a code, we therefore match the code plus the trailing
	// space in the dictionary.
	Common::String code = _currentCode + " ";

	int line = binarySearch(_unitedDict.dictLine, code, _unitedDict.dictLineCount);
	if (line < 0) {
		line = -(line + 1);
		_unitedDict.dictActLine = NULL;
	} else {
		_unitedDict.dictActLine = _unitedDict.dictLine[line];
	}

	_currentWord.clear();
	_wordNumber = 0;
	if (0 == strncmp(_unitedDict.dictLine[line], _currentCode.c_str(), _currentCode.size())) {
		char tmp[kMaxLineLen];
		strncpy(tmp, _unitedDict.dictLine[line], kMaxLineLen);
		tmp[kMaxLineLen - 1] = 0;
		char *tok = strtok(tmp, " ");
		tok = strtok(NULL, " ");
		_currentWord = Common::String(tok, _currentCode.size());
		return true;
	} else {
		return false;
	}
}

bool PredictiveDialog::searchWord(const char *const where, const Common::String &whatCode) {
	const char *ptr = where;
	ptr += whatCode.size();

	const char *newPtr;
	bool is = false;
	while ((newPtr = strchr(ptr, ' '))) {
		if (0 == strncmp(ptr, _currentWord.c_str(), newPtr - ptr)) {
			is = true;
			break;
		}
		ptr = newPtr + 1;
	}
	if (!is) {
		if (0 == strcmp(ptr, _currentWord.c_str())) {
			is = true;
		}
	}
	return is;
}

void PredictiveDialog::addWord(Dict &dict, const Common::String &word, const Common::String &code) {
	char *newLine;
	Common::String tmpCode = code + ' ';
	int line = binarySearch(dict.dictLine, tmpCode, dict.dictLineCount);
	if (line >= 0) {
		if (searchWord(dict.dictLine[line], tmpCode)) {
			// if we found code and word, we should not insert/expands any word
			return;
		} else {
			// if we found the code, but did not find a word, we must
			// EXPANDS the currnent line with new word
			int oldLineSize = strlen(dict.dictLine[line]);
			int newLineSize = oldLineSize + word.size() + 1;

			newLine = (char *)malloc(newLineSize + 1);

			char *ptr = newLine;
			strncpy(ptr, dict.dictLine[line], oldLineSize);
			ptr += oldLineSize;
			Common::String tmp = ' ' + word + '\0';
			strncpy(ptr, tmp.c_str(), tmp.size());

			dict.dictLine[line] = newLine;
			_memoryList[_numMemory++] = newLine;

			if (dict.nameDict == "user_dictionary")
				_userDictHasChanged = true;

			return;
		}
	} else { // if we didn't find the code, we need to INSERT new line with code and word
		if (dict.nameDict == "user_dictionary") {
			// if we must INSERT new line(code and word) to user_dictionary, we need to
			// check if there is a line that we want to INSERT in predictive dictionay
			int predictLine = binarySearch(_predictiveDict.dictLine, tmpCode, _predictiveDict.dictLineCount);
			if (predictLine >= 0) {
				if (searchWord(_predictiveDict.dictLine[predictLine], tmpCode)) {
					// if code and word is in predictive dictionary, we need to copy
					// this line to user dictionary
					int len = (predictLine == _predictiveDict.dictLineCount - 1) ? &_predictiveDict.dictText[_predictiveDict.dictTextSize] - _predictiveDict.dictLine[predictLine] :
					          _predictiveDict.dictLine[predictLine + 1] - _predictiveDict.dictLine[predictLine];
					newLine = (char *)malloc(len);
					strncpy(newLine, _predictiveDict.dictLine[predictLine], len);
				} else {
					// if there is no word in predictive dictionary, we need to copy to
					// user dictionary mathed line + new word.
					int len = (predictLine == _predictiveDict.dictLineCount - 1) ? &_predictiveDict.dictText[_predictiveDict.dictTextSize] - _predictiveDict.dictLine[predictLine] :
					          _predictiveDict.dictLine[predictLine + 1] - _predictiveDict.dictLine[predictLine];
					newLine = (char *)malloc(len + word.size() + 1);
					char *ptr = newLine;
					strncpy(ptr, _predictiveDict.dictLine[predictLine], len);
					ptr[len - 1] = ' ';
					ptr += len;
					strncpy(ptr, word.c_str(), word.size());
					ptr[len + word.size()] = '\0';
				}
			} else {
				// if we didnt find line in predictive dialog, we should copy to user dictionary
				// code + word
				Common::String tmp;
				tmp = tmpCode + word + '\0';
				newLine = (char *)malloc(tmp.size());
				strncpy(newLine, tmp.c_str(), tmp.size());
			}
		} else {
			// if want to insert line to different from user dictionary, we should copy to this
			// dictionary code + word
			Common::String tmp;
			tmp = tmpCode + word + '\0';
			newLine = (char *)malloc(tmp.size());
			strncpy(newLine, tmp.c_str(), tmp.size());
		}
	}

	// start from here are INSERTING new line to dictionaty ( dict )
	char **newDictLine = (char **)calloc(1, sizeof(char *) * (dict.dictLineCount + 1));
	if (!newDictLine) {
		warning("Predictive Dialog: cannot allocate memory for index buffer");
		return;
	}
	newDictLine[dict.dictLineCount] = '\0';

	int k = 0;
	bool inserted = false;
	for (int i = 0; i < dict.dictLineCount; i++) {
		uint lenPredictiveDictCode = strchr(dict.dictLine[i], ' ') - dict.dictLine[i];
		uint lenCode = (lenPredictiveDictCode >= (code.size() - 1)) ? lenPredictiveDictCode : code.size() - 1;
		if ((strncmp(dict.dictLine[i], code.c_str(), lenCode) > 0) && !inserted) {
			newDictLine[k++] = newLine;
			inserted = true;
		}
		if (k != (dict.dictLineCount + 1)) {
			newDictLine[k++] = dict.dictLine[i];
		}
	}
	if (!inserted)
		newDictLine[k] = newLine;

	_memoryList[_numMemory++] = newLine;

	free(dict.dictLine);
	dict.dictLineCount += 1;
	dict.dictLine = (char **)calloc(1, sizeof(char *) * dict.dictLineCount);
	if (!dict.dictLine) {
		warning("Predictive Dialog: cannot allocate memory for index buffer");
		free(newDictLine);
		return;
	}

	for (int i = 0; i < dict.dictLineCount; i++) {
		dict.dictLine[i] = newDictLine[i];
	}

	if (dict.nameDict == "user_dictionary")
		_userDictHasChanged = true;

	free(newDictLine);
}

void PredictiveDialog::addWordToDict() {
	if (_numMemory < kMaxWord) {
		addWord(_unitedDict, _currentWord, _currentCode);
		addWord(_userDict, _currentWord, _currentCode);
	} else {
		warning("Predictive Dialog: You cannot add word to user dictionary...");
	}
}

void PredictiveDialog::loadDictionary(Common::SeekableReadStream *in, Dict &dict) {
	int lines = 0;

	uint32 time1 = g_system->getMillis();

	dict.dictTextSize = in->size();
	dict.dictText = (char *)malloc(dict.dictTextSize + 1);

	if (!dict.dictText) {
		warning("Predictive Dialog: Not enough memory to load the file user.dic");
		return;
	}

	in->read(dict.dictText, dict.dictTextSize);
	dict.dictText[dict.dictTextSize] = 0;
	uint32 time2 = g_system->getMillis();
	debug("Predictive Dialog: Time to read %s: %d bytes, %d ms", ConfMan.get(dict.nameDict).c_str(), dict.dictTextSize, time2 - time1);
	delete in;

	char *ptr = dict.dictText;
	lines = 1;
	while ((ptr = strchr(ptr, '\n'))) {
		lines++;
		ptr++;
	}

	dict.dictLine = (char **)calloc(1, sizeof(char *) * lines);
	if (dict.dictLine == NULL) {
		warning("Predictive Dialog: Cannot allocate memory for line index buffer");
		return;
	}
	dict.dictLine[0] = dict.dictText;
	ptr = dict.dictText;
	int i = 1;
	while ((ptr = strchr(ptr, '\n'))) {
		*ptr = 0;
		ptr++;
#ifdef __DS__
		// Pass the line on to the DS word list
		DS::addAutoCompleteLine(dict.dictLine[i - 1]);
#endif
		dict.dictLine[i++] = ptr;
	}
	if (dict.dictLine[lines - 1][0] == 0)
		lines--;

	dict.dictLineCount = lines;
	debug("Predictive Dialog: Loaded %d lines", dict.dictLineCount);

	// FIXME: We use binary search on _predictiveDict.dictLine, yet we make no at_tempt
	// to ever sort this array (except for the DS port). That seems risky, doesn't it?

#ifdef __DS__
	// Sort the DS word completion list, to allow for a binary chop later (in the ds backend)
	DS::sortAutoCompleteWordList();
#endif

	uint32 time3 = g_system->getMillis();
	debug("Predictive Dialog: Time to parse %s: %d, total: %d", ConfMan.get(dict.nameDict).c_str(), time3 - time2, time3 - time1);
}

void PredictiveDialog::loadAllDictionary(Dict &dict) {
	ConfMan.registerDefault(dict.nameDict, dict.fnameDict);

	if (dict.nameDict == "predictive_dictionary") {
		Common::File *inFile = new Common::File();
		if (!inFile->open(ConfMan.get(dict.nameDict))) {
			warning("Predictive Dialog: cannot read file: %s", dict.fnameDict.c_str());
			return;
		}
		loadDictionary(inFile, dict);
	} else {
		Common::InSaveFile *inFile = g_system->getSavefileManager()->openForLoading(ConfMan.get(dict.nameDict));
		if (!inFile) {
			warning("Predictive Dialog: cannot read file: %s", dict.fnameDict.c_str());
			return;
		}
		loadDictionary(inFile, dict);
	}
}

void PredictiveDialog::pressEditText() {
	Common::strlcpy(_predictiveResult, _prefix.c_str(), sizeof(_predictiveResult));
	Common::strlcat(_predictiveResult, _currentWord.c_str(), sizeof(_predictiveResult));
	_edittext->setEditString(_predictiveResult);
	//_edittext->setCaretPos(_prefix.size() + _currentWord.size());
	_edittext->draw();
}

} // namespace GUI
