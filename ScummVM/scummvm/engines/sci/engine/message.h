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

#ifndef SCI_ENGINE_MESSAGE_H
#define SCI_ENGINE_MESSAGE_H

#include "sci/resource.h"
#include "sci/engine/vm_types.h"
#include "common/stack.h"

namespace Sci {

class SegManager;
struct MessageRecord;

struct MessageTuple {
	byte noun;
	byte verb;
	byte cond;
	byte seq;

	MessageTuple(byte noun_ = 0, byte verb_ = 0, byte cond_ = 0, byte seq_ = 1)
		: noun(noun_), verb(verb_), cond(cond_), seq(seq_) { }
};

class CursorStack : public Common::Stack<MessageTuple> {
public:
	void init(int module, MessageTuple t) {
		clear();
		push(t);
		_module = module;
	}

	int getModule() const { return _module; }

private:
	int _module;
};

typedef Common::Stack<CursorStack> CursorStackStack;

class MessageState {
public:
	MessageState(SegManager *segMan) : _segMan(segMan) { }
	int getMessage(int module, MessageTuple &t, reg_t buf);
	int nextMessage(reg_t buf);
	int messageSize(int module, MessageTuple &t);
	bool messageRef(int module, const MessageTuple &t, MessageTuple &ref);
	void lastQuery(int &module, MessageTuple &tuple);
	void pushCursorStack();
	void popCursorStack();

private:
	bool getRecord(CursorStack &stack, bool recurse, MessageRecord &record);
	void outputString(reg_t buf, const Common::String &str);
	Common::String processString(const char *s);
	int hexDigitToInt(char h);
	bool stringHex(Common::String &outStr, const Common::String &inStr, uint &index);
	bool stringLit(Common::String &outStr, const Common::String &inStr, uint &index);
	bool stringStage(Common::String &outStr, const Common::String &inStr, uint &index);

	CursorStack _cursorStack;
	CursorStackStack _cursorStackStack;
	MessageTuple _lastReturned;
	int _lastReturnedModule;

	SegManager *_segMan;
};

} // End of namespace Sci

#endif // SCI_ENGINE_MESSAGE_H
