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

// Verb and hitarea handling

#include "common/system.h"

#include "graphics/surface.h"

#include "agos/agos.h"
#include "agos/intern.h"

namespace AGOS {

static const char *const russian_verb_names[] = {
	"Ietj _",
	"Qnotrft< pa",
	"Nt_r[t<",
	"Ecjdat<",
	"Q=fst<",
	"C^]t<",
	"Ha_r[t<",
	"Isqom<^ocat<",
	"Docorjt<",
	"Qp]t<",
	"Neft<",
	"Eat<"
};

static const char *const hebrew_verb_names[] = {
	"LJ @L",
	"DQZKL RL",
	"TZG",
	"DFF",
	"@KEL",
	"DXM",
	"QBEX",
	"DYZNY",
	"CAX @L",
	"DQX",
	"LAY",
	"ZO"
};

static const char *const spanish_verb_names[] = {
	"Caminar",
	"Mirar",
	"Abrir",
	"Mover",
	"Consumir",
	"Coger",
	"Cerrar",
	"Usar",
	"Hablar",
	"Quitar",
	"Llevar",
	"Dar"
};

static const char *const italian_verb_names[] = {
	"Vai verso",
	"Osserva",
	"Apri",
	"Sposta",
	"Mangia",
	"Raccogli",
	"Chiudi",
	"Usa",
	"Parla a",
	"Togli",
	"Indossa",
	"Dai"
};

static const char *const french_verb_names[] = {
	"Aller vers",
	"Regarder",
	"Ouvrir",
	"D/placer",
	"Consommer",
	"Prendre",
	"Fermer",
	"Utiliser",
	"Parler ;",
	"Enlever",
	"Mettre",
	"Donner"
};

static const char *const german_verb_names[] = {
	"Gehe zu",
	"Schau an",
	";ffne",
	"Bewege",
	"Verzehre",
	"Nimm",
	"Schlie+e",
	"Benutze",
	"Rede mit",
	"Entferne",
	"Trage",
	"Gib"
};

static const char *const english_verb_names[] = {
	"Walk to",
	"Look at",
	"Open",
	"Move",
	"Consume",
	"Pick up",
	"Close",
	"Use",
	"Talk to",
	"Remove",
	"Wear",
	"Give"
};

static const char *const czech_verb_names[] = {
	"Jit",
	"Podivat se",
	"Otevrit",
	"Pohnout s",
	"Snist",
	"Sebrat",
	"Zavrit",
	"Pouzit",
	"Mluvit s",
	"Odstranit",
	"Oblect",
	"Dat"
};

static const char *const russian_verb_prep_names[] = {
	"", "", "", "",
	"", "", "", "s yfn?",
	"", "", "", "_onu ?"
};

static const char *const hebrew_verb_prep_names[] = {
	"", "", "", "",
	"", "", "", "RM ND ?",
	"", "", "", "LNI ?"
};

static const char *const spanish_verb_prep_names[] = {
	"", "", "", "",
	"", "", "", "^con qu/?",
	"", "", "", "^a qui/n?"
};

static const char *const italian_verb_prep_names[] = {
	"", "", "", "",
	"", "", "", "con cosa ?",
	"", "", "", "a chi ?"
};

static const char *const french_verb_prep_names[] = {
	"", "", "", "",
	"", "", "", "avec quoi ?",
	"", "", "", "; qui ?"
};

static const char *const german_verb_prep_names[] = {
	"", "", "", "",
	"", "", "", "mit was ?",
	"", "", "", "zu wem ?"
};

static const char *const english_verb_prep_names[] = {
	"", "", "", "",
	"", "", "", "with what ?",
	"", "", "", "to whom ?"
};

static const char *const czech_verb_prep_names[] = {
	"", "", "", "",
	"", "", "", "s cim ?",
	"", "", "", "komu ?"
};

#ifdef ENABLE_AGOS2
void AGOSEngine_Feeble::clearName() {
	stopAnimateSimon2(2, 6);
	_lastNameOn = NULL;
	_animatePointer = false;
	_mouseAnim = 1;
	return;
}
#endif

void AGOSEngine_Simon2::clearName() {
	if (getBitFlag(79)) {
		sendSync(202);
		_lastNameOn = NULL;
		return;
	}

	AGOSEngine_Simon1::clearName();
}

void AGOSEngine_Simon1::clearName() {
	HitArea *ha;

	if (_currentVerbBox == _lastVerbOn)
		return;

	resetNameWindow();
	_lastVerbOn = _currentVerbBox;

	if (_currentVerbBox != NULL && (ha = findBox(200)) && (ha->flags & kBFBoxDead) && !(_currentVerbBox->flags & kBFBoxDead))
		printVerbOf(_currentVerbBox->id);
}

void AGOSEngine::clearName() {
	if (getGameType() == GType_ELVIRA1 || getGameType() == GType_ELVIRA2)
		return;

	if (_nameLocked || !_lastNameOn)
		return;

	resetNameWindow();
}

void AGOSEngine::printVerbOf(uint hitarea_id) {
	const char *txt;
	const char * const *verb_names;
	const char * const *verb_prep_names;

	hitarea_id -= 101;

	if (_showPreposition) {
		switch (_language) {
		case Common::RU_RUS:
			verb_prep_names = russian_verb_prep_names;
			break;
		case Common::HE_ISR:
			verb_prep_names = hebrew_verb_prep_names;
			break;
		case Common::ES_ESP:
			verb_prep_names = spanish_verb_prep_names;
			break;
		case Common::IT_ITA:
			verb_prep_names = italian_verb_prep_names;
			break;
		case Common::FR_FRA:
			verb_prep_names = french_verb_prep_names;
			break;
		case Common::DE_DEU:
			verb_prep_names = german_verb_prep_names;
			break;
		case Common::CZ_CZE:
			verb_prep_names = czech_verb_prep_names;
			break;
		default:
			verb_prep_names = english_verb_prep_names;
			break;
		}
		CHECK_BOUNDS(hitarea_id, english_verb_prep_names);
		txt = verb_prep_names[hitarea_id];
	} else {
		switch (_language) {
		case Common::RU_RUS:
			verb_names = russian_verb_names;
			break;
		case Common::HE_ISR:
			verb_names = hebrew_verb_names;
			break;
		case Common::ES_ESP:
			verb_names = spanish_verb_names;
			break;
		case Common::IT_ITA:
			verb_names = italian_verb_names;
			break;
		case Common::FR_FRA:
			verb_names = french_verb_names;
			break;
		case Common::DE_DEU:
			verb_names = german_verb_names;
			break;
		case Common::CZ_CZE:
			verb_names = czech_verb_names;
			break;
		default:
			verb_names = english_verb_names;
			break;
		}
		CHECK_BOUNDS(hitarea_id, english_verb_names);
		txt = verb_names[hitarea_id];
	}
	showActionString((const byte *)txt);
}

void AGOSEngine::showActionString(const byte *string) {
	WindowBlock *window;
	uint x;
	const uint len = (getGameType() == GType_WW) ? 29 : 53;

	window = _windowArray[1];
	if (window == NULL || window->textColor == 0)
		return;

	// Arisme : hack for long strings in the French version
	if ((strlen((const char*)string) - 1) <= len)
		x = (len - (strlen((const char *)string) - 1)) * 3;
	else
		x = 0;

	window->textColumn = x / 8;
	window->textColumnOffset = x & 7;
	if (_language == Common::HE_ISR && window->textColumnOffset != 0) {
		window->textColumnOffset = 8 - window->textColumnOffset;
		window->textColumn++;
	}

	for (; *string; string++)
		windowPutChar(window, *string);
}

void AGOSEngine::handleVerbClicked(uint verb) {
	Subroutine *sub;
	int result;

	if (shouldQuit())
		return;

	_objectItem = _hitAreaObjectItem;
	if (_objectItem == _dummyItem2) {
		_objectItem = me();
	}
	if (_objectItem == _dummyItem3) {
		_objectItem = derefItem(me()->parent);
	}

	_subjectItem = _hitAreaSubjectItem;
	if (_subjectItem == _dummyItem2) {
		_subjectItem = me();
	}
	if (_subjectItem == _dummyItem3) {
		_subjectItem = derefItem(me()->parent);
	}

	if (_subjectItem) {
		_scriptNoun1 = _subjectItem->noun;
		_scriptAdj1 = _subjectItem->adjective;
	} else {
		_scriptNoun1 = -1;
		_scriptAdj1 = -1;
	}

	if (_objectItem) {
		_scriptNoun2 = _objectItem->noun;
		_scriptAdj2 = _objectItem->adjective;
	} else {
		_scriptNoun2 = -1;
		_scriptAdj2 = -1;
	}

	_scriptVerb = _verbHitArea;

	sub = getSubroutineByID(0);
	if (sub == NULL)
		return;

	result = startSubroutine(sub);
	if (result == -1)
		showMessageFormat("I don't understand");

	_runScriptReturn1 = false;

	sub = getSubroutineByID(100);
	if (sub)
		startSubroutine(sub);

	if (getGameType() == GType_SIMON2 || getGameType() == GType_FF || getGameType() == GType_PP)
		_runScriptReturn1 = false;

	permitInput();
}

void AGOSEngine::resetNameWindow() {
	WindowBlock *window;

	if (getGameType() == GType_SIMON2 && getBitFlag(79))
		return;

	window = _windowArray[1];
	if (window != NULL && window->textColor != 0)
		clearWindow(window);

	_lastNameOn = NULL;
	_lastVerbOn = NULL;
}

HitArea *AGOSEngine::findBox(uint hitarea_id) {
	HitArea *ha = _hitAreas;
	uint count = ARRAYSIZE(_hitAreas);

	do {
		if (getGameType() == GType_FF || getGameType() == GType_PP) {
			if (ha->id == hitarea_id && ha->flags != 0)
				return ha;
		} else {
			if (ha->id == hitarea_id)
				return ha;
		}
	} while (ha++, --count);
	return NULL;
}

HitArea *AGOSEngine::findEmptyHitArea() {
	HitArea *ha = _hitAreas;
	uint count = ARRAYSIZE(_hitAreas) - 1;

	do {
		if (ha->flags == 0)
			return ha;
	} while (ha++, --count);

	// The last box is overwritten, if too many boxes are allocated.
	return ha;
}

void AGOSEngine::freeBox(uint index) {
	CHECK_BOUNDS(index, _hitAreas);
	_hitAreas[index].flags = 0;
}

void AGOSEngine::enableBox(uint hitarea) {
	HitArea *ha = findBox(hitarea);
	if (ha != NULL)
		ha->flags &= ~kBFBoxDead;
}

void AGOSEngine::disableBox(uint hitarea) {
	HitArea *ha = findBox(hitarea);
	if (ha != NULL) {
		ha->flags |= kBFBoxDead;
		ha->flags &= ~kBFBoxSelected;
		if ((getGameType() == GType_SIMON1 || getGameType() == GType_SIMON2) &&
			hitarea == 102) {
			resetVerbs();
		}
	}
}

void AGOSEngine::moveBox(uint hitarea, int x, int y) {
	HitArea *ha = findBox(hitarea);
	if (ha != NULL) {
		if (getGameType() == GType_FF || getGameType() == GType_PP) {
			ha->x += x;
			ha->y += y;
		} else {
			ha->x = x;
			ha->y = y;
		}
	}
}

void AGOSEngine::undefineBox(uint hitarea) {
	HitArea *ha = findBox(hitarea);
	if (ha != NULL) {
		ha->flags = 0;
		if (ha == _lastNameOn)
			clearName();
		_needHitAreaRecalc++;
	}
}

bool AGOSEngine::isBoxDead(uint hitarea) {
	HitArea *ha = findBox(hitarea);
	if (ha == NULL)
		return false;
	return (ha->flags & kBFBoxDead) == 0;
}

void AGOSEngine::defineBox(uint16 id, uint16 x, uint16 y, uint16 height, uint16 width, uint16 msg1, uint16 msg2, uint16 flags) {
	HitArea *ha = _hitAreaList + id;
	ha->x = x;
	ha->y = y;
	ha->width = width;
	ha->height = height;
	ha->msg1 = msg1;
	ha->msg2 = msg2;
	ha->flags = flags;
	ha->id = ha->priority = id;
}

void AGOSEngine::defineBox(int id, int x, int y, int width, int height, int flags, int verb, Item *itemPtr) {
	HitArea *ha;
	undefineBox(id);

	ha = findEmptyHitArea();
	ha->x = x;
	ha->y = y;
	ha->width = width;
	ha->height = height;
	ha->flags = flags | kBFBoxInUse;
	ha->id = ha->priority = id;
	ha->verb = verb;
	ha->itemPtr = itemPtr;

	if (getGameType() == GType_FF && (ha->flags & kBFHyperBox)) {
		ha->data = _hyperLink;
		ha->priority = 50;
	}

	_needHitAreaRecalc++;
}

#ifdef ENABLE_AGOS2
void AGOSEngine_PuzzlePack::resetVerbs() {
	_verbHitArea = 300;
}

void AGOSEngine_Feeble::resetVerbs() {
	_verbHitArea = 300;
	int cursor = 0;
	int animMax = 16;

	if (getBitFlag(203)) {
		cursor = 14;
		animMax = 9;
	} else if (getBitFlag(204)) {
		cursor = 15;
		animMax = 9;
	} else if (getBitFlag(207)) {
		cursor = 26;
		animMax = 2;
	}

	_mouseCursor = cursor;
	_mouseAnimMax = animMax;
	_mouseAnim = 1;
	_needHitAreaRecalc++;

	if (getBitFlag(99)) {
		setVerb(NULL);
	}
}
#endif

void AGOSEngine::resetVerbs() {
	if (getGameType() == GType_ELVIRA1 || getGameType() == GType_ELVIRA2)
		return;

	uint id;
	HitArea *ha;

	if (getGameType() == GType_SIMON2) {
		id = 2;
		if (!getBitFlag(79))
		id = (_mouse.y >= 136) ? 102 : 101;
	} else {
		id = (_mouse.y >= 136) ? 102 : 101;
	}

	_defaultVerb = id;

	ha = findBox(id);
	if (ha == NULL)
		return;

	if (ha->flags & kBFBoxDead) {
		_defaultVerb = 999;
		_currentVerbBox = NULL;
	} else {
		_verbHitArea = ha->verb;
		setVerb(ha);
	}
}

#ifdef ENABLE_AGOS2
void AGOSEngine_Feeble::setVerb(HitArea *ha) {
	int cursor = _mouseCursor;
	if (_noRightClick)
		return;

	if (cursor > 13)
		cursor = 0;
	cursor++;
	if (cursor == 5)
		cursor = 1;
	if (cursor == 4) {
		if (getBitFlag(72)) {
			cursor = 1;
		}
	} else if (cursor == 2) {
		if (getBitFlag(99)) {
			cursor = 3;
		}
	}

	_mouseCursor = cursor;
	_mouseAnimMax = (cursor == 4) ? 14: 16;
	_mouseAnim = 1;
	_needHitAreaRecalc++;
	_verbHitArea = cursor + 300;
}
#endif

void AGOSEngine::setVerb(HitArea *ha) {
	HitArea *tmp = _currentVerbBox;

	if (ha == tmp)
		return;

	if (getGameType() == GType_SIMON1) {
		if (tmp != NULL) {
			tmp->flags |= kBFInvertTouch;
			if (getFeatures() & GF_32COLOR)
				invertBox(tmp, 212, 208, 212, 8);
			else
				invertBox(tmp, 213, 208, 213, 10);
		}

		if (ha->flags & kBFBoxSelected) {
			if (getFeatures() & GF_32COLOR)
				invertBox(ha, 216, 212, 212, 4);
			else
				invertBox(ha, 218, 213, 213, 5);
		} else {
			if (getFeatures() & GF_32COLOR)
				invertBox(ha, 220, 216, 216, 8);
			else
				invertBox(ha, 223, 218, 218, 10);
		}

		ha->flags &= ~(kBFBoxSelected + kBFInvertTouch);
	} else {
		if (ha->id < 101)
			return;
		_mouseCursor = ha->id - 101;
		_needHitAreaRecalc++;
	}
	_currentVerbBox = ha;
}

#ifdef ENABLE_AGOS2
void AGOSEngine_Feeble::hitarea_leave(HitArea *ha, bool state) {
	invertBox(ha, state);
}
#endif

void AGOSEngine::hitarea_leave(HitArea *ha, bool state) {
	if (getGameType() == GType_SIMON2) {
		invertBox(ha, 231, 229, 230, 1);
	} else {
		if (getFeatures() & GF_32COLOR)
			invertBox(ha, 220, 212, 216, 4);
		else
			invertBox(ha, 223, 213, 218, 5);
	}
}

void AGOSEngine::leaveHitAreaById(uint hitarea_id) {
	HitArea *ha = findBox(hitarea_id);
	if (ha)
		hitarea_leave(ha);
}

void AGOSEngine::inventoryUp(WindowBlock *window) {
	if (window->iconPtr->line == 0)
		return;

	mouseOff();
	uint index = getWindowNum(window);
	drawIconArray(index, window->iconPtr->itemRef, window->iconPtr->line - 1, window->iconPtr->classMask);
	mouseOn();
}

void AGOSEngine::inventoryDown(WindowBlock *window) {
	mouseOff();
	uint index = getWindowNum(window);
	drawIconArray(index, window->iconPtr->itemRef, window->iconPtr->line + 1, window->iconPtr->classMask);
	mouseOn();
}

void AGOSEngine::boxController(uint x, uint y, uint mode) {
	HitArea *best_ha;
	HitArea *ha = _hitAreas;
	uint count = ARRAYSIZE(_hitAreas);
	uint16 priority = 0;

	best_ha = NULL;

	do {
		if (ha->flags & kBFBoxInUse) {
			if (!(ha->flags & kBFBoxDead)) {
				if (x >= ha->x && y >= ha->y &&
						x - ha->x < ha->width && y - ha->y < ha->height && priority <= ha->priority) {
					priority = ha->priority;
					best_ha = ha;
				} else {
					if (ha->flags & kBFBoxSelected) {
						hitarea_leave(ha , true);
						ha->flags &= ~kBFBoxSelected;
					}
				}
			} else {
				ha->flags &= ~kBFBoxSelected;
			}
		}
	} while (ha++, --count);

	_currentBoxNum = 0;
	_currentBox = best_ha;

	if (best_ha == NULL)
		return;

	_currentBoxNum = best_ha->id;

	if (mode != 0) {
		if (mode == 3) {
			if (best_ha->verb & 0x4000) {
				if (getGameType() == GType_ELVIRA1 && _variableArray[500] == 0) {
					_variableArray[500] = best_ha->verb & 0xBFFF;
				}

				if (_clickOnly && best_ha->id < 8) {
					uint id = best_ha->id;
					if (id >= 4)
						id -= 4;

					invertBox(findBox(id), 0, 0, 0, 0);
					_clickOnly = false;
					return;
				}
			}

			if (best_ha->flags & kBFDragBox)
				_lastClickRem = best_ha;
		} else {
			_lastHitArea = best_ha;
		}
	}

	if (_clickOnly)
		return;

	if (best_ha->flags & kBFInvertTouch) {
		if (!(best_ha->flags & kBFBoxSelected)) {
			hitarea_leave(best_ha, false);
			best_ha->flags |= kBFBoxSelected;
		}
	} else {
		if (mode == 0)
			return;

		if (!(best_ha->flags & kBFInvertSelect))
			return;

		if (best_ha->flags & kBFToggleBox) {
			hitarea_leave(best_ha, false);
			best_ha->flags ^= kBFInvertSelect;
		} else if (!(best_ha->flags & kBFBoxSelected)) {
			hitarea_leave(best_ha, false);
			best_ha->flags |= kBFBoxSelected;
		}
	}
}

void AGOSEngine_Waxworks::boxController(uint x, uint y, uint mode) {
	HitArea *best_ha;
	HitArea *ha = _hitAreas;
	uint count = ARRAYSIZE(_hitAreas);
	uint16 priority = 0;
	uint16 x_ = x;
	uint16 y_ = y;

	if (getGameType() == GType_FF || getGameType() == GType_PP) {
		x_ += _scrollX;
		y_ += _scrollY;
	} else if (getGameType() == GType_SIMON2) {
		if (getBitFlag(79) || y < 134) {
			x_ += _scrollX * 8;
		}
	}

	best_ha = NULL;

	do {
		if (ha->flags & kBFBoxInUse) {
			if (!(ha->flags & kBFBoxDead)) {
				if (x_ >= ha->x && y_ >= ha->y &&
						x_ - ha->x < ha->width && y_ - ha->y < ha->height && priority <= ha->priority) {
					priority = ha->priority;
					best_ha = ha;
				} else {
					if (ha->flags & kBFBoxSelected) {
						hitarea_leave(ha , true);
						ha->flags &= ~kBFBoxSelected;
					}
				}
			} else {
				ha->flags &= ~kBFBoxSelected;
			}
		}
	} while (ha++, --count);

	_currentBoxNum = 0;
	_currentBox = best_ha;

	if (best_ha == NULL) {
		clearName();
		if (getGameType() == GType_WW && _mouseCursor >= 4) {
			_mouseCursor = 0;
			_needHitAreaRecalc++;
		}
		return;
	}

	_currentBoxNum = best_ha->id;

	if (mode != 0) {
		if (mode == 3) {
			if (best_ha->flags & kBFDragBox) {
				_lastClickRem = best_ha;
			}
		} else {
			_lastHitArea = best_ha;
			if (getGameType() == GType_PP) {
				_variableArray[400] = x;
				_variableArray[401] = y;
			} else if (getGameType() == GType_SIMON1 || getGameType() == GType_SIMON2 ||
				getGameType() == GType_FF) {
				_variableArray[1] = x;
				_variableArray[2] = y;
			}
		}
	}

	if ((getGameType() == GType_WW) && (_mouseCursor == 0 || _mouseCursor >= 4)) {
		uint verb = best_ha->verb & 0x3FFF;
		if (verb >= 239 && verb <= 242) {
			uint cursor = verb - 235;
			if (_mouseCursor != cursor) {
				_mouseCursor = cursor;
				_needHitAreaRecalc++;
			}
		}
	}

	if (getGameType() != GType_WW || !_nameLocked) {
		if (best_ha->flags & kBFNoTouchName) {
			clearName();
		} else if (best_ha != _lastNameOn) {
			displayName(best_ha);
		}
	}

	if (best_ha->flags & kBFInvertTouch && !(best_ha->flags & kBFBoxSelected)) {
		hitarea_leave(best_ha, false);
		best_ha->flags |= kBFBoxSelected;
	}
}

void AGOSEngine::displayName(HitArea *ha) {
	if (getGameType() == GType_ELVIRA1 || getGameType() == GType_ELVIRA2 || getGameType() == GType_PP)
		return;

	bool result;
	int x = 0, y = 0;

	if (getGameType() == GType_FF) {
		if (ha->flags & kBFHyperBox) {
			_lastNameOn = ha;
			return;
		}
		if (findBox(50))
			return;

		if (getBitFlag(99))
			_animatePointer = ((ha->flags & kBFTextBox) == 0);
		else
			_animatePointer = true;

		if (!getBitFlag(73))
			return;

		y = ha->y;
		if (getBitFlag(99) && y > 288)
			y = 288;
		y -= 17;
		if (y < 0)
			y = 0;
		y += 2;
		x = ha->width / 2 + ha->x;
	} else {
		resetNameWindow();
	}

	if (ha->flags & kBFTextBox) {
		result = printTextOf(ha->flags / 256, x, y);
	} else {
		result = printNameOf(ha->itemPtr, x, y);
	}

	if (result)
		_lastNameOn = ha;
}

#ifdef ENABLE_AGOS2
void AGOSEngine_Feeble::invertBox(HitArea *ha, bool state) {
	if (getBitFlag(205) || getBitFlag(206)) {
		if (state != 0) {
			_mouseAnimMax = _oldMouseAnimMax;
			_mouseCursor = _oldMouseCursor;
		} else if (_mouseCursor != 18) {
			_oldMouseCursor = _mouseCursor;
			_animatePointer = false;
			_oldMouseAnimMax = _mouseAnimMax;
			_mouseAnimMax = 2;
			_mouseCursor = 18;
		}
	} else {
		if (getBitFlag(207)) {
			if (state != 0) {
				_noRightClick = 0;
				resetVerbs();
			} else {
				int cursor = ha->id + 9;
				if (cursor >= 23)
					cursor = 21;
				_mouseCursor = cursor;
				_mouseAnimMax = 8;
				_noRightClick = 1;
			}
		} else {
			VgaSprite *vsp = _vgaSprites;

			int id = ha->id - 43;
			while (vsp->id) {
				if (vsp->id == id && vsp->zoneNum == 2) {
					if (state == 0)
						vsp->flags |= kDFShaded;
					else
						vsp->flags &= ~kDFShaded;
					break;
				}
				vsp++;
			}
		}
	}
}
#endif

void AGOSEngine::invertBox(HitArea *ha, byte a, byte b, byte c, byte d) {
	byte *src, color;
	int w, h, i;

	_videoLockOut |= 0x8000;

	Graphics::Surface *screen = _system->lockScreen();
	src = (byte *)screen->pixels + ha->y * screen->pitch + ha->x;

	// WORKAROUND: Hitareas for saved game names aren't adjusted for scrolling locations
	if (getGameType() == GType_SIMON2 && ha->id >= 208 && ha->id <= 213) {
		src -= _scrollX * 8;
	}

	_litBoxFlag = true;

	w = ha->width;
	h = ha->height;

	do {
		for (i = 0; i != w; ++i) {
			color = src[i];
			if (getGameType() == GType_WW) {
				if (!(color & 0xF) || (color & 0xF) == 10) {
					color ^= 10;
					src[i] = color;
				}
			} else if (getGameType() == GType_ELVIRA2) {
				if (!(color & 1)) {
					color ^= 2;
					src[i] = color;
				}
			} else if (getGameType() == GType_ELVIRA1) {
				if (color & 1) {
					color ^= 2;
					src[i] = color;
				}
			} else if (getGameType() == GType_PN) {
				if (getPlatform() == Common::kPlatformPC) {
					if (color != 15) {
						color ^= 7;
						src[i] = color;
					}
				} else {
					if (color != 14) {
						color ^= 15;
						src[i] = color;
					}
				}
			} else {
				if (a >= color && b < color) {
					if (c >= color)
						color += d;
					else
						color -= d;
					src[i] = color;
				}
			}
		}
		src += screen->pitch;
	} while (--h);

	_system->unlockScreen();

	_videoLockOut &= ~0x8000;
}

} // End of namespace AGOS
