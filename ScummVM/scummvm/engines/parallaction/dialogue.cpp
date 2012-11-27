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

#include "common/events.h"
#include "common/debug-channels.h"
#include "common/textconsole.h"
#include "parallaction/exec.h"
#include "parallaction/input.h"
#include "parallaction/parallaction.h"



namespace Parallaction {

#define MAX_PASSWORD_LENGTH			 7
/*
#define QUESTION_BALLOON_X			140
#define QUESTION_BALLOON_Y			10
#define QUESTION_CHARACTER_X		  190
#define QUESTION_CHARACTER_Y		  80

#define ANSWER_CHARACTER_X			10
#define ANSWER_CHARACTER_Y			80
*/
struct BalloonPositions {
	Common::Point	_questionBalloon;
	Common::Point	_questionChar;

	Common::Point	_answerChar;
};




class DialogueManager {

	Parallaction	*_vm;
	Dialogue		*_dialogue;

	bool			isNpc;
	GfxObj			*_questioner;
	GfxObj			*_answerer;
	int				_faceId;

	Question		*_q;

	int			_answerId;

	int		_selection, _oldSelection;

	uint32			_mouseButtons;
	Common::Point	_mousePos;

protected:
	BalloonPositions	_ballonPos;
	struct VisibleAnswer {
		Answer	*_a;
		int		_index;		// index into Question::_answers[]
		int		_balloon;
	} _visAnswers[5];
	int			_numVisAnswers;
	bool			_isKeyDown;
	uint16			_downKey;

protected:
	Gfx				*_gfx;
	BalloonManager  *_balloonMan;

public:
	DialogueManager(Parallaction *vm, ZonePtr z);
	virtual ~DialogueManager();

	void start();

	bool isOver() {
		return _state == DIALOGUE_OVER;
	}
	void run();

	ZonePtr			_z;
	CommandList *_cmdList;

protected:
	enum DialogueState {
		DIALOGUE_START,
		RUN_QUESTION,
		RUN_ANSWER,
		NEXT_QUESTION,
		NEXT_ANSWER,
		DIALOGUE_OVER
	} _state;

	static const int NO_ANSWER_SELECTED = -1;

	void transitionToState(DialogueState newState);

	bool displayQuestion();
	void displayAnswers();
	bool testAnswerFlags(Answer *a);
	virtual void addVisibleAnswers(Question *q) = 0;
	virtual int16 selectAnswer() = 0;

	int16 selectAnswer1();
	int16 selectAnswerN();
	int16 getHoverAnswer(int16 x, int16 y);

	void runQuestion();
	void runAnswer();
	void nextQuestion();
	void nextAnswer();
};

DialogueManager::DialogueManager(Parallaction *vm, ZonePtr z) : _vm(vm), _z(z) {
	_gfx = _vm->_gfx;
	_balloonMan = _vm->_balloonMan;

	_dialogue = _z->u._speakDialogue;
	isNpc = !_z->u._filename.empty() && _z->u._filename.compareToIgnoreCase("yourself");
	_questioner = isNpc ? _vm->_disk->loadTalk(_z->u._filename.c_str()) : _vm->_char._talk;
	_answerer = _vm->_char._talk;

	_cmdList = 0;
	_answerId = 0;
}

void DialogueManager::start() {
	assert(_dialogue);
	_q = _dialogue->_questions[0];
	_state = DIALOGUE_START;
	transitionToState(displayQuestion() ? RUN_QUESTION : NEXT_ANSWER);
}


DialogueManager::~DialogueManager() {
	if (isNpc) {
		delete _questioner;
	}
	_z.reset();
}

void DialogueManager::transitionToState(DialogueState newState) {
	static const char *dialogueStates[] = {
		"start",
		"runquestion",
		"runanswer",
		"nextquestion",
		"nextanswer",
		"over"
	};

	if (_state != newState) {
		debugC(3, kDebugDialogue, "DialogueManager moved to state '%s'", dialogueStates[newState]);

		if (DebugMan.isDebugChannelEnabled(kDebugDialogue) && gDebugLevel == 9) {
			switch (newState) {
				case RUN_QUESTION:
					debug("  Q  : %s", _q->_text.c_str());
					break;
				case RUN_ANSWER:
					for (int i = 0; i < _numVisAnswers; ++i) {
						debug("  A%02i: %s", i, _visAnswers[i]._a->_text.c_str());
					}
					break;
				default:
					break;
			}
		}
	}

	_state = newState;
}

bool DialogueManager::testAnswerFlags(Answer *a) {
	uint32 flags = _vm->getLocationFlags();
	if (a->_yesFlags & kFlagsGlobal)
		flags = g_globalFlags | kFlagsGlobal;
	return ((a->_yesFlags & flags) == a->_yesFlags) && ((a->_noFlags & ~flags) == a->_noFlags);
}

void DialogueManager::displayAnswers() {

	// create balloons
	int id;
	for (int i = 0; i < _numVisAnswers; ++i) {
		id = _balloonMan->setDialogueBalloon(_visAnswers[i]._a->_text, 1, BalloonManager::kUnselectedColor);
		assert(id >= 0);
		_visAnswers[i]._balloon = id;

	}

	int mood = 0;
	if (_numVisAnswers == 1) {
		mood = _visAnswers[0]._a->speakerMood();
		_balloonMan->setBalloonText(_visAnswers[0]._balloon, _visAnswers[0]._a->_text, BalloonManager::kNormalColor);
	} else
	if (_numVisAnswers > 1) {
		mood = _visAnswers[0]._a->speakerMood();
		_oldSelection = NO_ANSWER_SELECTED;
		_selection = 0;
	}

	_faceId = _gfx->setItem(_answerer, _ballonPos._answerChar.x, _ballonPos._answerChar.y);
	_gfx->setItemFrame(_faceId, mood);
}

int16 DialogueManager::selectAnswer1() {
	if (_visAnswers[0]._a->textIsNull()) {
		return _visAnswers[0]._index;
	}

	if (_mouseButtons == kMouseLeftUp) {
		return _visAnswers[0]._index;
	}

	return NO_ANSWER_SELECTED;
}

int16 DialogueManager::selectAnswerN() {

	_selection = _balloonMan->hitTestDialogueBalloon(_mousePos.x, _mousePos.y);

	VisibleAnswer *oldAnswer = &_visAnswers[_oldSelection];
	VisibleAnswer *answer = &_visAnswers[_selection];

	if (_selection != _oldSelection) {
		if (_oldSelection != NO_ANSWER_SELECTED) {
			_balloonMan->setBalloonText(oldAnswer->_balloon, oldAnswer->_a->_text, BalloonManager::kUnselectedColor);
		}

		if (_selection != NO_ANSWER_SELECTED) {
			_balloonMan->setBalloonText(answer->_balloon, answer->_a->_text, BalloonManager::kSelectedColor);
			_gfx->setItemFrame(_faceId, answer->_a->speakerMood());
		}
	}

	_oldSelection = _selection;

	if ((_mouseButtons == kMouseLeftUp) && (_selection != NO_ANSWER_SELECTED)) {
		return _visAnswers[_selection]._index;
	}

	return NO_ANSWER_SELECTED;
}

bool DialogueManager::displayQuestion() {
	if (_q->textIsNull()) return false;

	_balloonMan->setSingleBalloon(_q->_text, _ballonPos._questionBalloon.x, _ballonPos._questionBalloon.y, _q->balloonWinding(), BalloonManager::kNormalColor);
	_faceId = _gfx->setItem(_questioner, _ballonPos._questionChar.x, _ballonPos._questionChar.y);
	_gfx->setItemFrame(_faceId, _q->speakerMood());

	return true;
}

void DialogueManager::runQuestion() {
	if (_mouseButtons == kMouseLeftUp) {
		_gfx->freeDialogueObjects();
		transitionToState(NEXT_ANSWER);
	}

}


void DialogueManager::nextAnswer() {
	if (_q->_answers[0] == NULL) {
		transitionToState(DIALOGUE_OVER);
		return;
	}

	// try and check if there are any suitable answers,
	// given the current game state.
	addVisibleAnswers(_q);
	if (!_numVisAnswers) {
		// if there are no answers, then chicken out
		transitionToState(DIALOGUE_OVER);
		return;
	}

	if (_visAnswers[0]._a->textIsNull()) {
		// if the first answer is null (it's implied that it's the
		// only one because we already called addVisibleAnswers),
		// then jump to the next question
		_answerId = _visAnswers[0]._index;
		transitionToState(NEXT_QUESTION);
	} else {
		// at this point we are sure there are non-null answers to show
		displayAnswers();
		transitionToState(RUN_ANSWER);
	}
}

void DialogueManager::runAnswer() {
	_answerId = selectAnswer();
	if (_answerId != NO_ANSWER_SELECTED) {
		_cmdList = &_q->_answers[_answerId]->_commands;
		_gfx->freeDialogueObjects();
		transitionToState(NEXT_QUESTION);
	}
}

void DialogueManager::nextQuestion() {
	_q = _dialogue->findQuestion(_q->_answers[_answerId]->_followingName);
	if (_q == 0) {
		transitionToState(DIALOGUE_OVER);
	} else {
		transitionToState(displayQuestion() ? RUN_QUESTION : NEXT_ANSWER);
	}
}


void DialogueManager::run() {

	// cache event data
	_mouseButtons = _vm->_input->getLastButtonEvent();
	_vm->_input->getCursorPos(_mousePos);
	_isKeyDown = _vm->_input->getLastKeyDown(_downKey);

	switch (_state) {
	case RUN_QUESTION:
		runQuestion();
		break;

	case NEXT_ANSWER:
		nextAnswer();
		break;

	case NEXT_QUESTION:
		nextQuestion();
		break;

	case RUN_ANSWER:
		runAnswer();
		break;

	case DIALOGUE_OVER:
		break;

	default:
		error("unknown state in DialogueManager");

	}

}



class DialogueManager_ns : public DialogueManager {
protected:
	Parallaction_ns *_vm;
	bool			_passwordChanged;
	bool			_askPassword;

	bool checkPassword() {
		return ((!scumm_stricmp(_vm->_char.getBaseName(), g_doughName) && _vm->_password.hasPrefix("1732461")) ||
			   (!scumm_stricmp(_vm->_char.getBaseName(), g_donnaName) && _vm->_password.hasPrefix("1622")) ||
			   (!scumm_stricmp(_vm->_char.getBaseName(), g_dinoName) && _vm->_password.hasPrefix("179")));
	}

	void resetPassword() {
		_vm->_password.clear();
		_passwordChanged = true;
	}

	void accumPassword(uint16 ascii) {
		if (!Common::isDigit(ascii)) {
			return;
		}

		_vm->_password += ascii;
		_passwordChanged = true;
	}

	int16 askPassword() {

		if (_isKeyDown) {
			accumPassword(_downKey);
		}

		if (_passwordChanged) {
			_balloonMan->setBalloonText(_visAnswers[0]._balloon, _visAnswers[0]._a->_text, BalloonManager::kNormalColor);
			_passwordChanged = false;
		}

		if ((_vm->_password.size() == MAX_PASSWORD_LENGTH) || ((_isKeyDown) && (_downKey == Common::KEYCODE_RETURN))) {
			if (checkPassword()) {
				return 0;
			} else {
				resetPassword();
			}
		}

		return NO_ANSWER_SELECTED;
	}

public:
	DialogueManager_ns(Parallaction_ns *vm, ZonePtr z) : DialogueManager(vm, z), _vm(vm) {
		_ballonPos._questionBalloon = Common::Point(140, 10);
		_ballonPos._questionChar = Common::Point(190, 80);
		_ballonPos._answerChar = Common::Point(10, 80);
	}

	bool canDisplayAnswer(Answer *a) {
		return testAnswerFlags(a);
	}

	virtual void addVisibleAnswers(Question *q) {
		_askPassword = false;
		_numVisAnswers = 0;
		for (int i = 0; i < NUM_ANSWERS && q->_answers[i]; i++) {
			Answer *a = q->_answers[i];
			if (!canDisplayAnswer(a)) {
				continue;
			}

			if (a->_text.contains("%P")) {
				_askPassword = true;
			}

			_visAnswers[_numVisAnswers]._a = a;
			_visAnswers[_numVisAnswers]._index = i;
			_numVisAnswers++;
		}

		resetPassword();
	}

	virtual int16 selectAnswer() {
		int ans = NO_ANSWER_SELECTED;
		if (_askPassword) {
			ans = askPassword();
		} else
		if (_numVisAnswers == 1) {
			ans = selectAnswer1();
		} else {
			ans = selectAnswerN();
		}
		return ans;
	}
};

class DialogueManager_br : public DialogueManager {
	Parallaction_br *_vm;

public:
	DialogueManager_br(Parallaction_br *vm, ZonePtr z) : DialogueManager(vm, z), _vm(vm) {
		_ballonPos._questionBalloon = Common::Point(0, 0);
		_ballonPos._questionChar = Common::Point(380, 80);
		_ballonPos._answerChar = Common::Point(10, 80);
	}

	bool canDisplayAnswer(Answer *a) {
		if (!a)
			return false;

		if (a->_hasCounterCondition) {
			_vm->testCounterCondition(a->_counterName, a->_counterOp, a->_counterValue);
			return (_vm->getLocationFlags() & kFlagsTestTrue) != 0;
		}

		return testAnswerFlags(a);
	}

	virtual void addVisibleAnswers(Question *q) {
		_numVisAnswers = 0;
		for (int i = 0; i < NUM_ANSWERS && q->_answers[i]; i++) {
			Answer *a = q->_answers[i];
			if (!canDisplayAnswer(a)) {
				continue;
			}
			_visAnswers[_numVisAnswers]._a = a;
			_visAnswers[_numVisAnswers]._index = i;
			_numVisAnswers++;
		}
	}

	virtual int16 selectAnswer() {
		int16 ans = NO_ANSWER_SELECTED;
		if (_numVisAnswers == 1) {
			ans = selectAnswer1();
		} else {
			ans = selectAnswerN();
		}
		return ans;
	}
};


void Parallaction::enterDialogueMode(ZonePtr z) {
	if (!z->u._speakDialogue) {
		return;
	}

	debugC(1, kDebugDialogue, "Parallaction::enterDialogueMode(%s)", z->u._filename.c_str());
	_dialogueMan = createDialogueManager(z);
	assert(_dialogueMan);
	_dialogueMan->start();
	_input->_inputMode = Input::kInputModeDialogue;
}

void Parallaction::exitDialogueMode() {
	debugC(1, kDebugDialogue, "Parallaction::exitDialogueMode()");
	_input->_inputMode = Input::kInputModeGame;

	/* Since the current instance of _dialogueMan must be destroyed before the
	   zone commands are executed, as they may create a new instance of _dialogueMan that
	   would overwrite the current, we need to save the references to the command lists.
	*/
	CommandList *_cmdList = _dialogueMan->_cmdList;
	ZonePtr z = _dialogueMan->_z;

	// destroy the _dialogueMan here
	destroyDialogueManager();

	// run the lists saved
	if (_cmdList) {
		_cmdExec->run(*_cmdList);
	}
	_cmdExec->run(z->_commands, z);
}

void Parallaction::destroyDialogueManager() {
	// destroy the _dialogueMan here
	delete _dialogueMan;
	_dialogueMan = 0;
}

void Parallaction::runDialogueFrame() {
	if (_input->_inputMode != Input::kInputModeDialogue) {
		return;
	}

	_dialogueMan->run();

	if (_dialogueMan->isOver()) {
		exitDialogueMode();
	}

	return;
}

DialogueManager *Parallaction_ns::createDialogueManager(ZonePtr z) {
	return new DialogueManager_ns(this, z);
}

DialogueManager *Parallaction_br::createDialogueManager(ZonePtr z) {
	return new DialogueManager_br(this, z);
}

} // namespace Parallaction
