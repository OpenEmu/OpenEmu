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

#include "common/textconsole.h"

#include "parallaction/parallaction.h"
#include "parallaction/objects.h"
#include "parallaction/parser.h"

namespace Parallaction {


Command::Command() {
	_id = 0;
	_flagsOn = 0;
	_flagsOff = 0;
	_valid = false;

	_flags = 0;
	_string = 0;
	_callable = 0;
	_object = 0;
	_counterValue = 0;
	_zeta0 = 0;
	_zeta1 = 0;
	_zeta2 = 0;
	_characterId = 0;
	_string2 = 0;
	_musicCommand = 0;
	_musicParm = 0;
}

Command::~Command() {
	free(_string);
	free(_string2);
}


Animation::Animation() {
	gfxobj = NULL;
	_scriptName = 0;
	_frame = 0;
	_z = 0;
}

Animation::~Animation() {
	free(_scriptName);
	gfxobj->release();
}

void Animation::getFrameRect(Common::Rect &r) const {
	r.setWidth(0); r.setHeight(0);
	if (!gfxobj) {
		return;
	}
	gfxobj->getRect(_frame, r);
	r.translate(_left, _top);
}

bool Animation::hitFrameRect(int x, int y) const {
	if (!gfxobj) {
		return false;
	}
	Common::Rect r;
	getFrameRect(r);
	return r.contains(x, y);
}

int16 Animation::getBottom() const {
	int bottom = _top;
	if (gfxobj) {
		Common::Rect r;
		getFrameRect(r);
		bottom = r.bottom;
	}
	return bottom;
}

void Animation::resetZ() {
	setZ(getBottom());
}

uint16 Animation::getFrameNum() const {
	if (!gfxobj) return 0;
	return gfxobj->getNum();
}

byte* Animation::getFrameData() const {
	if (!gfxobj) return NULL;
	return gfxobj->getData(_frame);
}

void Animation::setF(int16 value) {
	int16 min = MIN(0, getFrameNum() - 1);
	int16 max = MAX(0, getFrameNum() - 1);
	_frame = CLIP(value, min, max);
}

void Animation::forceXYZF(int16 x, int16 y, int16 z, int16 f) {
	_left = x;
	_top = y;
	_z = z;
	_frame = f;
}

void Animation::getFoot(Common::Point &foot) {
	Common::Rect rect;
	gfxobj->getRect(_frame, rect);
	foot.x = getX() + (rect.left + rect.width() / 2);
	foot.y = getY() + (rect.top + rect.height());
}

void Animation::setFoot(const Common::Point &foot) {
	Common::Rect rect;
	gfxobj->getRect(_frame, rect);

	setX(foot.x - (rect.left + rect.width() / 2));
	setY(foot.y - (rect.top + rect.height()));
}

#define NUM_LOCALS	10
char	_localNames[NUM_LOCALS][10];

Program::Program() {
	_loopCounter = 0;
	_locals = new LocalVariable[NUM_LOCALS];
	_numLocals = 0;
	_status = kProgramIdle;
}

Program::~Program() {
	delete[] _locals;
}

int16 Program::findLocal(const char* name) {
	for (uint16 _si = 0; _si < NUM_LOCALS; _si++) {
		if (!scumm_stricmp(name, _localNames[_si]))
			return _si;
	}

	return -1;
}

int16 Program::addLocal(const char *name, int16 value, int16 min, int16 max) {
	assert(_numLocals < NUM_LOCALS);

	strcpy(_localNames[_numLocals], name);
	_locals[_numLocals].setRange(min, max);
	_locals[_numLocals].setValue(value);

	return _numLocals++;
}

void LocalVariable::setValue(int16 value) {
	if (value >= _max)
		value = _min;
	if (value < _min)
		value = _max - 1;

	_value = value;
}

void LocalVariable::setRange(int16 min, int16 max) {
	_max = max;
	_min = min;
}

int16 LocalVariable::getValue() const {
	return _value;
}


Zone::Zone() {
	_left = _top = _right = _bottom = 0;

	_type = 0;

	_flags = kFlagsNoName;
	_label = 0;

	// BRA specific
	_index = INVALID_ZONE_INDEX;
	_locationIndex = INVALID_LOCATION_INDEX;
}

Zone::~Zone() {
	g_vm->_gfx->unregisterLabel(_label);
	delete _label;
}

void Zone::translate(int16 x, int16 y) {
	_left += x;
	_right += x;
	_top += y;
	_bottom += y;
}

bool Zone::hitRect(int x, int y) const {
	// The scripts of Nippon Safes are full of invalid rectangles, used to
	// provide 'special' features.
	if (_right < _left || _bottom < _top) {
		return false;
	}

	Common::Rect r(_left, _top, _right + 1, _bottom + 1);
	r.grow(-1);

	return r.contains(x, y);
}

Dialogue::Dialogue() {
	memset(_questions, 0, sizeof(_questions));
	_numQuestions = 0;
}

Dialogue::~Dialogue() {
	for (int i = 0; i < NUM_QUESTIONS; i++) {
		delete _questions[i];
	}
}

Question *Dialogue::findQuestion(const Common::String &name) const {
	for (uint i = 0; _questions[i]; ++i) {
		if (_questions[i]->_name == name) {
			return _questions[i];
		}
	}
	return 0;
}

void Dialogue::addQuestion(Question *q) {
	assert(_numQuestions < NUM_QUESTIONS);
	assert(q);
	_questions[_numQuestions] = q;
	_numQuestions++;
}

Answer::Answer() {
	_mood = 0;
	_noFlags = 0;
	_yesFlags = 0;
	_hasCounterCondition = false;
}

bool Answer::textIsNull() {
	return (_text.equalsIgnoreCase("NULL"));
}

int Answer::speakerMood() {
	return _mood & 0xF;
}

Question::Question(const Common::String &name) : _name(name), _mood(0) {
	memset(_answers, 0, sizeof(_answers));
}

Question::~Question() {
	for (uint32 i = 0; i < NUM_ANSWERS; i++) {
		delete _answers[i];
	}
}

bool Question::textIsNull() {
	return (_text.equalsIgnoreCase("NULL"));
}

int Question::speakerMood() {
	return _mood & 0xF;
}

int Question::balloonWinding() {
	return _mood & 0x10;
}


Instruction::Instruction() {
	_index = 0;
	_flags = 0;

	// common
	_immediate = 0;

	// BRA specific
	_text = 0;
	_text2 = 0;
	_y = 0;
}

Instruction::~Instruction() {
	free(_text);
	free(_text2);
}

int16 ScriptVar::getValue() {

	if (_flags & kParaImmediate) {
		return _value;
	}

	if (_flags & kParaLocal) {
		return _local->getValue();
	}

	if (_flags & kParaField) {
		return _field->getValue();
	}

	if (_flags & kParaRandom) {
		return (g_vm->_rnd.getRandomNumber(65536) * _value) >> 16;
	}

	error("Parameter is not an r-value");
	return 0;	// for compilers that don't support NORETURN
}

void ScriptVar::setValue(int16 value) {
	if ((_flags & kParaLValue) == 0) {
		error("Only l-value can be set");
	}

	if (_flags & kParaLocal) {
		_local->setValue(value);
	}

	if (_flags & kParaField) {
		_field->setValue(value);
	}

}

void ScriptVar::setLocal(LocalVariable *local) {
	_local = local;
	_flags |= (kParaLocal | kParaLValue);
}

void ScriptVar::setField(Animation *anim, AnimationField::AccessorFunc accessor, AnimationField::MutatorFunc mutator) {
	_field = new AnimationField(anim, accessor, mutator);
	_flags |= (kParaField | kParaLValue);
}

void ScriptVar::setField(Animation *anim, AnimationField::AccessorFunc accessor) {
	_field = new AnimationField(anim, accessor);
	_flags |= kParaField;
}

void ScriptVar::setImmediate(int16 value) {
	_value = value;
	_flags |= kParaImmediate;
}

void ScriptVar::setRandom(int16 seed) {
	_value = seed;
	_flags |= kParaRandom;
}


ScriptVar::ScriptVar() {
	_flags = 0;
	_local = 0;
	_value = 0;
	_field = 0;
}

ScriptVar::~ScriptVar() {
	delete _field;
}


Table::Table(uint32 size) : _size(size), _used(0), _disposeMemory(true) {
	_data = (char**)calloc(size, sizeof(char *));
}

Table::Table(uint32 size, const char **data) : _size(size), _used(size), _disposeMemory(false) {
	_data = const_cast<char **>(data);
}

Table::~Table() {

	if (!_disposeMemory) return;

	clear();

	free(_data);

}

void Table::addData(const char* s) {

	if (!(_used < _size))
		error("Table overflow");

	_data[_used++] = strdup(s);

}

uint16 Table::lookup(const char* s) {

	for (uint16 i = 0; i < _used; i++) {
		if (!scumm_stricmp(_data[i], s)) return i + 1;
	}

	return notFound;
}

void Table::clear() {
	for (uint32 i = 0; i < _used; i++)
		free(_data[i]);

	_used = 0;
}

const char *Table::item(uint index) const {
	assert(index < _used);
	return _data[index];
}


FixedTable::FixedTable(uint32 size, uint32 fixed) : Table(size), _numFixed(fixed) {
}

void FixedTable::clear() {
	uint32 deleted = 0;
	for (uint32 i = _numFixed; i < _used; i++) {
		free(_data[i]);
		_data[i] = 0;
		deleted++;
	}

	_used -= deleted;
}

Table* createTableFromStream(uint32 size, Common::SeekableReadStream *stream) {
	assert(stream);

	Table *t = new Table(size);
	assert(t);

	Script s(stream, false);
	s.readLineToken();
	while (scumm_stricmp(_tokens[0], "ENDTABLE")) {
		t->addData(_tokens[0]);
		s.readLineToken();
	}
	delete stream;
	return t;
}


} // namespace Parallaction
