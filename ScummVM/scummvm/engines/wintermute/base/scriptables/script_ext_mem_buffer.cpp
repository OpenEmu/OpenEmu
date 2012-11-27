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

/*
 * This file is based on WME Lite.
 * http://dead-code.org/redir.php?target=wmelite
 * Copyright (c) 2011 Jan Nedoma
 */

#include "engines/wintermute/base/base_scriptable.h"
#include "engines/wintermute/base/scriptables/script_stack.h"
#include "engines/wintermute/base/scriptables/script.h"
#include "engines/wintermute/base/scriptables/script_value.h"
#include "engines/wintermute/base/scriptables/script_ext_mem_buffer.h"
#include "common/file.h"

namespace Wintermute {

IMPLEMENT_PERSISTENT(SXMemBuffer, false)

BaseScriptable *makeSXMemBuffer(BaseGame *inGame, ScStack *stack) {
	return new SXMemBuffer(inGame, stack);
}

//////////////////////////////////////////////////////////////////////////
SXMemBuffer::SXMemBuffer(BaseGame *inGame, ScStack *stack) : BaseScriptable(inGame) {
	stack->correctParams(1);
	_buffer = NULL;
	_size = 0;

	int newSize = stack->pop()->getInt();
	resize(MAX(0, newSize));
}

//////////////////////////////////////////////////////////////////////////
SXMemBuffer::SXMemBuffer(BaseGame *inGame, void *buffer) : BaseScriptable(inGame) {
	_size = 0;
	_buffer = buffer;
}


//////////////////////////////////////////////////////////////////////////
SXMemBuffer::~SXMemBuffer() {
	cleanup();
}

//////////////////////////////////////////////////////////////////////////
void *SXMemBuffer::scToMemBuffer() {
	return _buffer;
}

//////////////////////////////////////////////////////////////////////////
void SXMemBuffer::cleanup() {
	if (_size) {
		free(_buffer);
	}
	_buffer = NULL;
	_size = 0;
}

//////////////////////////////////////////////////////////////////////////
bool SXMemBuffer::resize(int newSize) {
	int oldSize = _size;

	if (_size == 0) {
		_buffer = malloc(newSize);
		if (_buffer) {
			_size = newSize;
		}
	} else {
		void *newBuf = realloc(_buffer, newSize);
		if (!newBuf) {
			if (newSize == 0) {
				_buffer = newBuf;
				_size = newSize;
			} else {
				return STATUS_FAILED;
			}
		} else {
			_buffer = newBuf;
			_size = newSize;
		}
	}

	if (_buffer && _size > oldSize) {
		memset((byte *)_buffer + oldSize, 0, _size - oldSize);
	}
	return STATUS_OK;
}

//////////////////////////////////////////////////////////////////////////
bool SXMemBuffer::checkBounds(ScScript *script, int start, int length) {
	if (_buffer == NULL) {
		script->runtimeError("Cannot use Set/Get methods on an uninitialized memory buffer");
		return false;
	}
	if (_size == 0) {
		return true;
	}

	if (start < 0 || length == 0 || start + length > _size) {
		script->runtimeError("Set/Get method call is out of bounds");
		return false;
	} else {
		return true;
	}
}

//////////////////////////////////////////////////////////////////////////
const char *SXMemBuffer::scToString() {
	return "[membuffer object]";
}


//////////////////////////////////////////////////////////////////////////
bool SXMemBuffer::scCallMethod(ScScript *script, ScStack *stack, ScStack *thisStack, const char *name) {
	//////////////////////////////////////////////////////////////////////////
	// SetSize
	//////////////////////////////////////////////////////////////////////////
	if (strcmp(name, "SetSize") == 0) {
		stack->correctParams(1);
		int newSize = stack->pop()->getInt();
		newSize = MAX(0, newSize);
		if (DID_SUCCEED(resize(newSize))) {
			stack->pushBool(true);
		} else {
			stack->pushBool(false);
		}

		return STATUS_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// GetBool
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(name, "GetBool") == 0) {
		stack->correctParams(1);
		int start = stack->pop()->getInt();
		if (!checkBounds(script, start, sizeof(bool))) {
			stack->pushNULL();
		} else {
			stack->pushBool(*(bool *)((byte *)_buffer + start));
		}

		return STATUS_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// GetByte
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(name, "GetByte") == 0) {
		stack->correctParams(1);
		int start = stack->pop()->getInt();
		if (!checkBounds(script, start, sizeof(byte))) {
			stack->pushNULL();
		} else {
			stack->pushInt(*(byte *)((byte *)_buffer + start));
		}

		return STATUS_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// GetShort
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(name, "GetShort") == 0) {
		stack->correctParams(1);
		int start = stack->pop()->getInt();
		if (!checkBounds(script, start, sizeof(short))) {
			stack->pushNULL();
		} else {
			stack->pushInt(65536 + * (short *)((byte *)_buffer + start));
		}

		return STATUS_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// GetInt / GetLong
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(name, "GetInt") == 0 || strcmp(name, "GetLong") == 0) {
		stack->correctParams(1);
		int start = stack->pop()->getInt();
		if (!checkBounds(script, start, sizeof(int))) {
			stack->pushNULL();
		} else {
			stack->pushInt(*(int *)((byte *)_buffer + start));
		}

		return STATUS_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// GetFloat
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(name, "GetFloat") == 0) {
		stack->correctParams(1);
		int start = stack->pop()->getInt();
		if (!checkBounds(script, start, sizeof(float))) {
			stack->pushNULL();
		} else {
			stack->pushFloat(*(float *)((byte *)_buffer + start));
		}

		return STATUS_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// GetDouble
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(name, "GetDouble") == 0) {
		stack->correctParams(1);
		int start = stack->pop()->getInt();
		if (!checkBounds(script, start, sizeof(double))) {
			stack->pushNULL();
		} else {
			stack->pushFloat(*(double *)((byte *)_buffer + start));
		}

		return STATUS_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// GetString
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(name, "GetString") == 0) {
		stack->correctParams(2);
		int start = stack->pop()->getInt();
		int length = stack->pop()->getInt();

		// find end of string
		if (length == 0 && start >= 0 && start < _size) {
			for (int i = start; i < _size; i++) {
				if (((char *)_buffer)[i] == '\0') {
					length = i - start;
					break;
				}
			}
		}

		if (!checkBounds(script, start, length)) {
			stack->pushNULL();
		} else {
			char *str = new char[length + 1];
			Common::strlcpy(str, (const char *)_buffer + start, length + 1);
			stack->pushString(str);
			delete[] str;
		}
		return STATUS_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// GetPointer
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(name, "GetPointer") == 0) {
		stack->correctParams(1);
		int start = stack->pop()->getInt();
		if (!checkBounds(script, start, sizeof(void *))) {
			stack->pushNULL();
		} else {
			void *pointer = *(void **)((byte *)_buffer + start);
			SXMemBuffer *buf = new SXMemBuffer(_gameRef,  pointer);
			stack->pushNative(buf, false);
		}
		return STATUS_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// SetBool
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(name, "SetBool") == 0) {
		stack->correctParams(2);
		int start = stack->pop()->getInt();
		bool val = stack->pop()->getBool();

		if (!checkBounds(script, start, sizeof(bool))) {
			stack->pushBool(false);
		} else {
			*(bool *)((byte *)_buffer + start) = val;
			stack->pushBool(true);
		}
		return STATUS_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// SetByte
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(name, "SetByte") == 0) {
		stack->correctParams(2);
		int start = stack->pop()->getInt();
		byte val = (byte)stack->pop()->getInt();

		if (!checkBounds(script, start, sizeof(byte))) {
			stack->pushBool(false);
		} else {
			*(byte *)((byte *)_buffer + start) = val;
			stack->pushBool(true);
		}
		return STATUS_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// SetShort
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(name, "SetShort") == 0) {
		stack->correctParams(2);
		int start = stack->pop()->getInt();
		short val = (short)stack->pop()->getInt();

		if (!checkBounds(script, start, sizeof(short))) {
			stack->pushBool(false);
		} else {
			*(short *)((byte *)_buffer + start) = val;
			stack->pushBool(true);
		}
		return STATUS_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// SetInt / SetLong
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(name, "SetInt") == 0 || strcmp(name, "SetLong") == 0) {
		stack->correctParams(2);
		int start = stack->pop()->getInt();
		int val = stack->pop()->getInt();

		if (!checkBounds(script, start, sizeof(int))) {
			stack->pushBool(false);
		} else {
			*(int *)((byte *)_buffer + start) = val;
			stack->pushBool(true);
		}
		return STATUS_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// SetFloat
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(name, "SetFloat") == 0) {
		stack->correctParams(2);
		int start = stack->pop()->getInt();
		float val = (float)stack->pop()->getFloat();

		if (!checkBounds(script, start, sizeof(float))) {
			stack->pushBool(false);
		} else {
			*(float *)((byte *)_buffer + start) = val;
			stack->pushBool(true);
		}
		return STATUS_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// SetDouble
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(name, "SetDouble") == 0) {
		stack->correctParams(2);
		int start = stack->pop()->getInt();
		double val = stack->pop()->getFloat();

		if (!checkBounds(script, start, sizeof(double))) {
			stack->pushBool(false);
		} else {
			*(double *)((byte *)_buffer + start) = val;
			stack->pushBool(true);
		}
		return STATUS_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// SetString
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(name, "SetString") == 0) {
		stack->correctParams(2);
		int start = stack->pop()->getInt();
		const char *val = stack->pop()->getString();

		if (!checkBounds(script, start, strlen(val) + 1)) {
			stack->pushBool(false);
		} else {
			memcpy((byte *)_buffer + start, val, strlen(val) + 1);
			stack->pushBool(true);
		}
		return STATUS_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// SetPointer
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(name, "SetPointer") == 0) {
		stack->correctParams(2);
		int start = stack->pop()->getInt();
		/* ScValue *val = */ stack->pop();

		if (!checkBounds(script, start, sizeof(void *))) {
			stack->pushBool(false);
		} else {
			/*
			int pointer = (int)Val->getMemBuffer();
			memcpy((byte *)_buffer+Start, &Pointer, sizeof(void*));
			stack->pushBool(true);
			*/
			// TODO fix
			stack->pushBool(false);

		}
		return STATUS_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// DEBUG_Dump
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(name, "DEBUG_Dump") == 0) {
		stack->correctParams(0);
		if (_buffer && _size) {
			warning("SXMemBuffer::ScCallMethod - DEBUG_Dump");
			Common::DumpFile f;
			f.open("buffer.bin");
			f.write(_buffer, _size);
			f.close();
		}
		stack->pushNULL();
		return STATUS_OK;
	} else {
		return STATUS_FAILED;
	}
}


//////////////////////////////////////////////////////////////////////////
ScValue *SXMemBuffer::scGetProperty(const Common::String &name) {
	_scValue->setNULL();

	//////////////////////////////////////////////////////////////////////////
	// Type (RO)
	//////////////////////////////////////////////////////////////////////////
	if (name == "Type") {
		_scValue->setString("membuffer");
		return _scValue;
	}

	//////////////////////////////////////////////////////////////////////////
	// Size (RO)
	//////////////////////////////////////////////////////////////////////////
	if (name == "Size") {
		_scValue->setInt(_size);
		return _scValue;
	} else {
		return BaseScriptable::scGetProperty(name);
	}
}


//////////////////////////////////////////////////////////////////////////
bool SXMemBuffer::scSetProperty(const char *name, ScValue *value) {
	/*
	//////////////////////////////////////////////////////////////////////////
	// Length
	//////////////////////////////////////////////////////////////////////////
	if (strcmp(name, "Length")==0){
	    int origLength = _length;
	    _length = max(value->getInt(0), 0);

	    char propName[20];
	    if (_length < OrigLength){
	        for(int i=_length; i<OrigLength; i++){
	            sprintf(PropName, "%d", i);
	            _values->DeleteProp(PropName);
	        }
	    }
	    return STATUS_OK;
	}
	else*/ return BaseScriptable::scSetProperty(name, value);
}


//////////////////////////////////////////////////////////////////////////
bool SXMemBuffer::persist(BasePersistenceManager *persistMgr) {

	BaseScriptable::persist(persistMgr);

	persistMgr->transfer(TMEMBER(_size));

	if (persistMgr->getIsSaving()) {
		if (_size > 0) {
			persistMgr->putBytes((byte *)_buffer, _size);
		}
	} else {
		if (_size > 0) {
			_buffer = malloc(_size);
			persistMgr->getBytes((byte *)_buffer, _size);
		} else {
			_buffer = NULL;
		}
	}

	return STATUS_OK;
}


//////////////////////////////////////////////////////////////////////////
int SXMemBuffer::scCompare(BaseScriptable *val) {
	if (_buffer == val->scToMemBuffer()) {
		return 0;
	} else {
		return 1;
	}
}

} // end of namespace Wintermute
