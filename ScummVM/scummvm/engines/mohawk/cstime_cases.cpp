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

#include "mohawk/cstime_cases.h"
#include "mohawk/cstime_ui.h"

#include "common/textconsole.h"

namespace Mohawk {

CSTimeCase1::CSTimeCase1(MohawkEngine_CSTime *vm) : CSTimeCase(vm, 1) {
}

CSTimeCase1::~CSTimeCase1() {
}

bool CSTimeCase1::checkConvCondition(uint16 conditionId) {
	const Common::Array<CSTimeHotspot> &hotspots = getCurrScene()->getHotspots();

	bool gotTorch = _vm->_haveInvItem[1];

	// These are all for conversations in the first scene (with the boatman).
	switch (conditionId) {
	case 0:
		// Got the torch?
		return (gotTorch);
	case 1:
		// Is the bag still on land?
		return (hotspots[5].state == 1);
	case 2:
		// Is the bag on the boat, but player hasn't taken the torch?
		return (hotspots[5].state != 1 && !gotTorch);
	}

	return false;
}

bool CSTimeCase1::checkAmbientCondition(uint16 charId, uint16 ambientId) {
	return true;
}

bool CSTimeCase1::checkObjectCondition(uint16 objectId) {
	const Common::Array<CSTimeHotspot> &hotspots = getCurrScene()->getHotspots();

	switch (_currScene) {
	case 1:
		switch (objectId) {
		case 1:
			// Hide bag on boat if it's not there.
			return (hotspots[5].state == 1);
		case 2:
			// Hide bag on land if it's not there.
			return (hotspots[5].state != 1);
		case 3:
			// Hide torch if it's been picked up.
			return (hotspots[4].state == 1);
		}
		break;

	case 2:
		// The first note piece.
		return !_vm->getInterface()->getCarmenNote()->havePiece(0);

	case 3:
		// The features representing different stages in the body sequence.
		if (objectId == 6 && _vm->_caseVariable[3] != 0)
			return false;
		else if (objectId == 7 && _vm->_caseVariable[3] != 1)
			return false;
		else if (objectId == 8 && _vm->_caseVariable[3] != 2)
			return false;
		else if (objectId > 8)
			return false;
		break;

	case 4:
		// The second note piece?
		if (objectId == 0)
			return (hotspots[0].state > 0);
		break;

	case 5:
		// The third note piece.
		if (objectId == 1)
			return !_vm->getInterface()->getCarmenNote()->havePiece(2);
	}

	return true;
}

void CSTimeCase1::selectHelpStrings() {
	CSTimeHelp *help = _vm->getInterface()->getHelp();

	if (_currScene == 1) {
		if (_vm->_haveInvItem[1]) {
			// Got the torch, ready to leave.
			help->addQaR(15, 55);
		} else if (getCurrScene()->getHotspot(6).state == 1) {
			// Should get the torch.
			help->addQaR(13, 53);
		} else if (_conversations[0]->getAsked(2, 1)) {
			// Should move the bag.
			help->addQaR(12, 52);
		} else if (_conversations[0]->getAsked(0, 0) || _conversations[0]->getAsked(1, 0) || _conversations[0]->getAsked(2, 0)) {
			// Should keep talking to the boatman.
			help->addQaR(11, 51);
		} else {
			// Should talk to the boatman.
			help->addQaR(10, 50);
		}
	} else {
		if (!_conversations[1]->getAsked(2, 0)) {
			if (!_conversations[1]->getAsked(0, 0) && !_conversations[1]->getAsked(1, 0)) {
				// Should talk to Hatshepsut.
				help->addQaR(16, 56);
			} else {
				// Should ask Hatshepsut what to do.
				help->addQaR(17, 57);
			}
		} else if (!_conversations[2]->getAsked(0, 2) && !_vm->_caseVariable[1]) {
			// Haven't asked the head priest what to do, and we haven't dealt with the body.
			help->addQaR(18, 58);
		}
		if (!_vm->getInterface()->getCarmenNote()->havePiece(0)) {
			// Should get that note piece.
			help->addQaR(14, 54);
		}
		if (_currScene == 5) {
			if (!_vm->getInterface()->getCarmenNote()->havePiece(2)) {
				// Should get the last note piece.
				help->addQaR(28, 68);
			} else {
				// Should work out where that henchman is.
				help->addQaR(29, 69);
			}
		} else {
			if (_conversations[2]->getAsked(0, 2)) {
				if (!_vm->_caseVariable[1]) {
					// We haven't dealt with the body yet.
					help->addQaR(21, 61);
				}
				if (!_scenes[3]->_visitCount) {
					// We haven't checked out that dark tomb.
					help->addQaR(22, 62);
				}
			} else {
				if (_conversations[2]->getAsked(0, 0) || _conversations[2]->getAsked(1, 0) || _conversations[2]->getAsked(2, 0)) {
					// We should keep talking to the head priest.
					help->addQaR(20, 60);
					if (!_scenes[3]->_visitCount) {
						// We haven't checked out that dark tomb.
						help->addQaR(23, 63);
					}
				} else if (_scenes[2]->_visitCount) {
					// We've visited the tomb, but not talked to the head priest.
					help->addQaR(19, 59);
				}
			}
			if (_vm->_caseVariable[2] && !_vm->_caseVariable[1]) {
				// We're in the middle of the body sequence.
				help->addQaR(24, 64);
			}
			if (_scenes[3]->_visitCount) {
				if (_vm->_haveInvItem[1]) {
					// Need to light that torch.
					help->addQaR(25, 65);
				} else if (_vm->_haveInvItem[2] && !_vm->_caseVariable[2]) {
					// Need to start the body sequence.
					help->addQaR(27, 67);
				}
			}
		}
	}

	help->addQaR(99, 0);
}

void CSTimeCase1::handleConditionalEvent(const CSTimeEvent &event) {
	CSTimeEvent newEvent;

	switch (event.param2) {
	case 0:
		// Trying to enter the first room of the tomb.
		if (!_conversations[1]->getAsked(2, 0)) {
			// We need a plan first.
			_vm->addEvent(CSTimeEvent(kCSTimeEventCharStartFlapping, getCurrScene()->getHelperId(), 12352));
		} else if (!_vm->getInterface()->getCarmenNote()->havePiece(0)) {
			// Shouldn't we take a look at that note?
			_vm->addEvent(CSTimeEvent(kCSTimeEventCharStartFlapping, getCurrScene()->getHelperId(), 10355));
		} else {
			// Onward!
			_vm->addEvent(CSTimeEvent(kCSTimeEventNewScene, event.param1, 3));
			_vm->addEvent(CSTimeEvent(kCSTimeEventCharStartFlapping, 1, 12551));
		}
		break;

	case 1:
		// Poking at the jars. The response depends on whether the hieroglyphs on the tomb wall
		// have been seen yet or not.
		_vm->addEvent(CSTimeEvent(kCSTimeEventCharStartFlapping, getCurrScene()->getHelperId(),
			_vm->_caseVariable[2] ? 14304 : 14303));
		break;

	case 2:
		// Leaving the dark tomb.
		if (!_vm->getInterface()->getCarmenNote()->havePiece(1)) {
			// Should probably get that note.
			_vm->insertEventAtFront(CSTimeEvent(kCSTimeEventCharStartFlapping, 0, 16356));
		} else {
			bool doAfter = true;
			if (_vm->_haveInvItem[1]) {
				// Still have the unlit torch.
				_vm->insertEventAtFront(CSTimeEvent(kCSTimeEventCharStartFlapping, 1, 14553));
				_vm->insertEventAtFront(CSTimeEvent(kCSTimeEventSetInsertBefore, 0, 0xffff));
				_vm->insertEventAtFront(CSTimeEvent(kCSTimeEventCharStartFlapping, getCurrScene()->getHelperId(), 14356));
			} else if (_vm->_caseVariable[4]) {
				// Let's start the mummy-making again!
				_vm->_caseVariable[4] = 0;
				_vm->insertEventAtFront(CSTimeEvent(kCSTimeEventCharStartFlapping, 1, 14565));
				_vm->insertEventAtFront(CSTimeEvent(kCSTimeEventSetInsertBefore, 0, 0xffff));
			} else if (_vm->_caseVariable[2] && !_vm->_caseVariable[6]) {
				// One-time remark after we saw the hieroglyphs.
				_vm->_caseVariable[6] = 1;
				_vm->insertEventAtFront(CSTimeEvent(kCSTimeEventCharStartFlapping, getCurrScene()->getHelperId(), 14355));
				_vm->insertEventAtFront(CSTimeEvent(kCSTimeEventCharStartFlapping, 1, 14563));
				_vm->insertEventAtFront(CSTimeEvent(kCSTimeEventSetInsertBefore, 0, 0xffff));
			} else
				doAfter = false;

			if (doAfter) {
				_vm->insertEventAtFront(CSTimeEvent(kCSTimeEventInitScene, 0xffff, 0xffff));
				_vm->insertEventAtFront(CSTimeEvent(kCSTimeEventCharSetupRestPos, 1, 0xffff));
				_vm->insertEventAtFront(CSTimeEvent(kCSTimeEventSetInsertBefore, 0, 0xffff));
			}

			// Move back to the main room.
			_vm->insertEventAtFront(CSTimeEvent(kCSTimeEventNewScene, 0xffff, 3));
		}
		break;

	case 3:
		// Body sequence logic.
		if (!_vm->_caseVariable[2]) {
			// We haven't seen the hieroglyphs yet. No guessing!
			_vm->insertEventAtFront(CSTimeEvent(kCSTimeEventCharStartFlapping, getCurrScene()->getHelperId(), 14354));
			break;
		}

		switch (event.param1) {
		case 1:
			// Second part.
			if (_vm->_caseVariable[3] == 1) {
				_vm->_caseVariable[3] = 2;
				// Yes, that was the right thing.
				_vm->insertEventAtFront(CSTimeEvent(kCSTimeEventCharStartFlapping, getCurrScene()->getHelperId(), 14361));
				_vm->insertEventAtFront(CSTimeEvent(kCSTimeEventCharStartFlapping, 1, 14555));
				_vm->insertEventAtFront(CSTimeEvent(kCSTimeEventSetInsertBefore, 0, 0xffff));
				// Update the features.
				_vm->insertEventAtFront(CSTimeEvent(kCSTimeEventAddFeature, 0xffff, 8));
				_vm->insertEventAtFront(CSTimeEvent(kCSTimeEventAddFeature, 0xffff, event.param1));
				_vm->insertEventAtFront(CSTimeEvent(kCSTimeEventCharPlayNIS, 2, 1));
				_vm->insertEventAtFront(CSTimeEvent(kCSTimeEventDisableFeature, 2, 6));
			} else if (_vm->_caseVariable[3] == 2) {
				// We've already done that part!
				_vm->insertEventAtFront(CSTimeEvent(kCSTimeEventCharStartFlapping, getCurrScene()->getHelperId(), 14357));
				_vm->insertEventAtFront(CSTimeEvent(kCSTimeEventAddFeature, 0xffff, event.param1));
			} else {
				incorrectBodySequence(4, 14559, event.param1);
			}
			break;

		case 2:
			// Third/final part.
			if (_vm->_caseVariable[3] == 2) {
				_vm->_caseVariable[1] = 1;
				// Move to the final room (we can't get here without having all the notes so far).
				_vm->insertEventAtFront(CSTimeEvent(kCSTimeEventNewScene, 0xffff, 5));
				// Yes, that was the right thing. We'll move onward now.
				_vm->insertEventAtFront(CSTimeEvent(kCSTimeEventCharStartFlapping, 1, 14558));
				_vm->insertEventAtFront(CSTimeEvent(kCSTimeEventSetInsertBefore, 0, 0xffff));
				_vm->insertEventAtFront(CSTimeEvent(kCSTimeEventCharStartFlapping, 1, 14556));
				_vm->insertEventAtFront(CSTimeEvent(kCSTimeEventSetInsertBefore, 0, 0xffff));
				// Update the features.
				_vm->insertEventAtFront(CSTimeEvent(kCSTimeEventAddFeature, 0xffff, 9));
				_vm->insertEventAtFront(CSTimeEvent(kCSTimeEventAddFeature, 0xffff, event.param1));
			} else {
				incorrectBodySequence(9, 14560, event.param1);
			}
			break;

		case 3:
			// First part.
			if (_vm->_caseVariable[3]) {
				// We've already done that part!
				_vm->insertEventAtFront(CSTimeEvent(kCSTimeEventCharStartFlapping, getCurrScene()->getHelperId(), 14357));
				_vm->insertEventAtFront(CSTimeEvent(kCSTimeEventAddFeature, 0xffff, event.param1));
			} else {
				_vm->_caseVariable[3] = 1;
				// Yes, that was the right thing.
				_vm->insertEventAtFront(CSTimeEvent(kCSTimeEventCharStartFlapping, getCurrScene()->getHelperId(), 14360));
				_vm->insertEventAtFront(CSTimeEvent(kCSTimeEventCharStartFlapping, 1, 14554));
				_vm->insertEventAtFront(CSTimeEvent(kCSTimeEventSetInsertBefore, 0, 0xffff));
				// Update the features.
				_vm->insertEventAtFront(CSTimeEvent(kCSTimeEventAddFeature, 0xffff, event.param1));
				_vm->insertEventAtFront(CSTimeEvent(kCSTimeEventAddFeature, 0xffff, 7));
				_vm->insertEventAtFront(CSTimeEvent(kCSTimeEventCharPlayNIS, 2, 0));
				_vm->insertEventAtFront(CSTimeEvent(kCSTimeEventDisableFeature, 2, 6));
			}
			break;

		case 4:
			if (_vm->_caseVariable[3] == 2)
				incorrectBodySequence(5, 14561, event.param1);
			else
				incorrectBodySequence(2, 14561, event.param1);
			break;

		case 5:
			if (_vm->_caseVariable[3] == 2)
				incorrectBodySequence(6, 14562, event.param1);
			else
				incorrectBodySequence(3, 14562, event.param1);
			break;
		}

		_vm->insertEventAtFront(CSTimeEvent(kCSTimeEventDisableFeature, 0xffff, event.param1));
		break;

	case 4:
		// Woven bag dragged.
		if (_conversations[0]->getAsked(2, 1)) {
			// We were asked to move it.
			if (event.param1 == 5) {
				// Yay, the player got it on the boat!
				// Congratulate the player and enable the torch.
				_vm->insertEventAtFront(CSTimeEvent(kCSTimeEventCharStartFlapping, 2, 10551));
				getCurrScene()->getHotspot(4).invObjId = 1;
				_vm->insertEventAtFront(CSTimeEvent(kCSTimeEventEnableHotspot, 0xffff, 6));
				_vm->insertEventAtFront(CSTimeEvent(kCSTimeEventAddFeature, 0xffff, 2));
			} else {
				assert(event.param1 < 7);
				// It didn't get dropped onto the boat, so we complain about it.
				newEvent.type = kCSTimeEventCharStartFlapping;
				// Does the Good Guide complain (if we gave it to her, or put it in the inventory)?
				newEvent.param1 = (event.param1 == 1 || event.param1 == 6) ? getCurrScene()->getHelperId() : 2;
				// Which string?
				static const uint16 strings[7] = { 30201, 30103, 30202, 30203, 30203, 0, 10352};
				newEvent.param2 = strings[event.param1];
				_vm->insertEventAtFront(newEvent);
			}
		} else {
			// We're just randomly moving the woven bag!
			_vm->addEvent(CSTimeEvent(kCSTimeEventCharStartFlapping, getCurrScene()->getHelperId(), 10351));

			if (event.param1 == 5) {
				// This went onto the boat hotspot, so the bag was removed; put it back.
				_vm->insertEventAtFront(CSTimeEvent(kCSTimeEventEnableHotspot, 0xffff, 5));
				_vm->insertEventAtFront(CSTimeEvent(kCSTimeEventAddFeature, 0xffff, 1));
			}
		}
		break;

	case 5:
		// We're ready to shove off!
		_vm->insertEventAtFront(CSTimeEvent(kCSTimeEventCharStartFlapping, getCurrScene()->getHelperId(), 10356));
		break;

	case 6:
		// Trying to leave the first scene by walking.
		if (_vm->_haveInvItem[1]) {
			// If you have the torch, the Good Guide prods you to use the boat.
			_vm->addEvent(CSTimeEvent(kCSTimeEventCharStartFlapping, getCurrScene()->getHelperId(), 10305));
		} else {
			// Otherwise, the boatman tells you that you can't leave yet.
			_vm->addEvent(CSTimeEvent(kCSTimeEventCharStartFlapping, 2, 10506));
		}
		break;

	case 7:
		// Clicking on the woven bag.
		if (_conversations[0]->getAsked(2, 0)) {
			// If we were asked to move it, the Good Guide prods us to try dragging.
			_vm->addEvent(CSTimeEvent(kCSTimeEventCharStartFlapping, getCurrScene()->getHelperId(), 10306));
		} else {
			// Otherwise, the boatman tells us what it is.
			_vm->addEvent(CSTimeEvent(kCSTimeEventCharStartFlapping, 2, 10502));
		}
		break;

	case 8:
		// One-time-only reminder that you can re-ask questions.
		if (_vm->_caseVariable[7])
			break;
		_vm->_caseVariable[7] = 1;
		_vm->addEvent(CSTimeEvent(kCSTimeEventCharStartFlapping, 0, 12359));
		break;

	case 9:
		// Trying to give the torch to the Good Guide; you get a different message
		// depending on whether it's already in your inventory or not.
		_vm->addEvent(CSTimeEvent(kCSTimeEventCharStartFlapping, 0, _vm->_haveInvItem[1] ? 9906 : 30119));
		break;

	default:
		error("unknown Conditional Event type %d for case 1", event.param2);
	}
}

void CSTimeCase1::incorrectBodySequence(uint16 stage, uint16 speech, uint16 feature) {
	// (This adds events backwards, so read from the bottom up.)

	// You did it wrong! Set the case variable for leaving the dark tomb, and get sent there.
	_vm->_caseVariable[4] = 1;
	_vm->insertEventAtFront(CSTimeEvent(kCSTimeEventNewScene, 0xffff, 4));

	CSTimeEvent event;
	event.type = kCSTimeEventCharStartFlapping;
	event.param1 = getCurrScene()->getHelperId();
	if (!_vm->_caseVariable[5]) {
		// The first time, you get a nice "let's have another look" suggestion.
		event.param2 = 14353;
		_vm->_caseVariable[5] = 1;
	} else if (_vm->_rnd->getRandomBit()) {
		// Randomly repeat that suggestion...
		event.param2 = 14353;
	} else {
		// Or else something a little grumpier.
		event.param2 = 14358 + _vm->_rnd->getRandomBit();
	}
	_vm->insertEventAtFront(event);

	// "What a mess!"
	_vm->insertEventAtFront(CSTimeEvent(kCSTimeEventCharStartFlapping, 1, 14557));
	_vm->insertEventAtFront(CSTimeEvent(kCSTimeEventSetInsertBefore, 0, 0xffff));
	_vm->insertEventAtFront(CSTimeEvent(kCSTimeEventCharPlayNIS, 1, 1));
	_vm->insertEventAtFront(CSTimeEvent(kCSTimeEventSetInsertBefore, 0, 0xffff));

	// Enable the head priest(?!).
	_vm->insertEventAtFront(CSTimeEvent(kCSTimeEventCharSetState, 1, 1));

	// Explain what went wrong.
	_vm->insertEventAtFront(CSTimeEvent(kCSTimeEventCharStartFlapping, 1, speech));
	_vm->insertEventAtFront(CSTimeEvent(kCSTimeEventSetInsertBefore, 0, 0xffff));
	_vm->insertEventAtFront(CSTimeEvent(kCSTimeEventCharPlayNIS, 1, 0));

	// Reset the features, with an animation if necessary.
	if (stage == 9) {
		_vm->insertEventAtFront(CSTimeEvent(kCSTimeEventAddFeature, 0xffff, 9));
		_vm->insertEventAtFront(CSTimeEvent(kCSTimeEventAddFeature, 0xffff, feature));
	} else {
		_vm->insertEventAtFront(CSTimeEvent(kCSTimeEventAddFeature, 0xffff, stage + 8));
		_vm->insertEventAtFront(CSTimeEvent(kCSTimeEventAddFeature, 0xffff, feature));
		_vm->insertEventAtFront(CSTimeEvent(kCSTimeEventCharPlayNIS, 2, 1));
		event.type = kCSTimeEventDisableFeature;
		event.param1 = 2;
		if (_vm->_caseVariable[3] == 0)
			event.param2 = 6;
		else if (_vm->_caseVariable[3] == 1)
			event.param2 = 7;
		else
			event.param2 = 8;
		_vm->insertEventAtFront(event);
	}

	// And so, we're back to the beginning of the body sequence.
	_vm->_caseVariable[3] = 0;
}

} // End of namespace Mohawk
