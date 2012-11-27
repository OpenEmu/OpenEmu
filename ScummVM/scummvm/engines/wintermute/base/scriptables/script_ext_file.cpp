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

#include "engines/wintermute/system/sys_class_registry.h"
#include "engines/wintermute/system/sys_class.h"
#include "engines/wintermute/base/scriptables/script_stack.h"
#include "engines/wintermute/base/scriptables/script_value.h"
#include "engines/wintermute/base/scriptables/script.h"
#include "engines/wintermute/utils/utils.h"
#include "engines/wintermute/base/base_game.h"
#include "engines/wintermute/base/base_file_manager.h"
#include "engines/wintermute/platform_osystem.h"
#include "engines/wintermute/base/scriptables/script_ext_file.h"

// Note: This code is completely untested, as I have yet to find a game that uses SXFile.

namespace Wintermute {

IMPLEMENT_PERSISTENT(SXFile, false)

BaseScriptable *makeSXFile(BaseGame *inGame, ScStack *stack) {
	return new SXFile(inGame, stack);
}

//////////////////////////////////////////////////////////////////////////
SXFile::SXFile(BaseGame *inGame, ScStack *stack) : BaseScriptable(inGame) {
	stack->correctParams(1);
	ScValue *val = stack->pop();

	_filename = NULL;
	if (!val->isNULL()) {
		BaseUtils::setString(&_filename, val->getString());
	}

	_readFile = NULL;
	_writeFile = NULL;

	_mode = 0;
	_textMode = false;
}


//////////////////////////////////////////////////////////////////////////
SXFile::~SXFile() {
	cleanup();
}

//////////////////////////////////////////////////////////////////////////
void SXFile::cleanup() {
	delete[] _filename;
	_filename = NULL;
	close();
}


//////////////////////////////////////////////////////////////////////////
void SXFile::close() {
	if (_readFile) {
		BaseFileManager::getEngineInstance()->closeFile(_readFile);
		_readFile = NULL;
	}
	if (_writeFile) {
		_writeFile->finalize();
		delete _writeFile;
		_writeFile = NULL;
	}
	_mode = 0;
	_textMode = false;
}

//////////////////////////////////////////////////////////////////////////
const char *SXFile::scToString() {
	if (_filename) {
		return _filename;
	} else {
		return "[file object]";
	}
}

#define FILE_BUFFER_SIZE 32768
//////////////////////////////////////////////////////////////////////////
bool SXFile::scCallMethod(ScScript *script, ScStack *stack, ScStack *thisStack, const char *name) {
	//////////////////////////////////////////////////////////////////////////
	// SetFilename
	//////////////////////////////////////////////////////////////////////////
	if (strcmp(name, "SetFilename") == 0) {
		stack->correctParams(1);
		const char *filename = stack->pop()->getString();
		cleanup();
		BaseUtils::setString(&_filename, filename);
		stack->pushNULL();
		return STATUS_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// OpenAsText / OpenAsBinary
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(name, "OpenAsText") == 0 || strcmp(name, "OpenAsBinary") == 0) {
		stack->correctParams(1);
		close();
		_mode = stack->pop()->getInt(1);
		if (_mode < 1 || _mode > 3) {
			script->runtimeError("File.%s: invalid access mode. Setting read mode.", name);
			_mode = 1;
		}
		if (_mode == 1) {
			_readFile = BaseFileManager::getEngineInstance()->openFile(_filename);
			if (!_readFile) {
				//script->runtimeError("File.%s: Error opening file '%s' for reading.", Name, _filename);
				close();
			} else {
				_textMode = strcmp(name, "OpenAsText") == 0;
			}
		} else {
			if (strcmp(name, "OpenAsText") == 0) {
				if (_mode == 2) {
					_writeFile = openForWrite(_filename, false);
				} else {
					_writeFile = openForAppend(_filename, false);
				}
			} else {
				if (_mode == 2) {
					_writeFile = openForWrite(_filename, true);
				} else {
					_writeFile = openForAppend(_filename, true);
				}
			}

			if (!_writeFile) {
				//script->runtimeError("File.%s: Error opening file '%s' for writing.", Name, _filename);
				close();
			} else {
				_textMode = strcmp(name, "OpenAsText") == 0;
			}
		}

		if (_readFile || _writeFile) {
			stack->pushBool(true);
		} else {
			stack->pushBool(false);
		}

		return STATUS_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// Close
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(name, "Close") == 0) {
		stack->correctParams(0);
		close();
		stack->pushNULL();
		return STATUS_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// SetPosition
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(name, "SetPosition") == 0) {
		stack->correctParams(1);
		if (_mode == 0) {
			script->runtimeError("File.%s: File is not open", name);
			stack->pushBool(false);
		} else {
			int pos = stack->pop()->getInt();
			stack->pushBool(setPos(pos));
		}
		return STATUS_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// Delete
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(name, "Delete") == 0) {
		stack->correctParams(0);
		close();
		error("SXFile-Method: \"Delete\" not supported");
		//stack->pushBool(BasePlatform::deleteFile(_filename) != false);
		stack->pushBool(false);
		return STATUS_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// Copy
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(name, "Copy") == 0) {
		stack->correctParams(2);
		/* const char *dest = */ stack->pop()->getString();
		/* bool overwrite = */ stack->pop()->getBool(true);

		close();
		error("SXFile-Method: Copy not supported");
		//stack->pushBool(BasePlatform::copyFile(_filename, Dest, !Overwrite) != false);
		stack->pushBool(false);
		return STATUS_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// ReadLine
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(name, "ReadLine") == 0) {
		stack->correctParams(0);
		if (!_textMode || !_readFile) {
			script->runtimeError("File.%s: File must be open in text mode.", name);
			stack->pushNULL();
			return STATUS_OK;
		}
		uint32 bufSize = FILE_BUFFER_SIZE;
		byte *buf = (byte *)malloc(bufSize);
		uint32 counter = 0;
		byte b;
		bool foundNewLine = false;
		bool ret = STATUS_FAILED;
		do {
			ret = _readFile->read(&b, 1);
			if (ret != 1) {
				break;
			}

			if (counter > bufSize) {
				buf = (byte *)realloc(buf, bufSize + FILE_BUFFER_SIZE);
				bufSize += FILE_BUFFER_SIZE;
			}
			if (b == '\n') {
				buf[counter] = '\0';
				foundNewLine = true;
				break;
			} else if (b == 0x0D) {
				continue;
			} else {
				buf[counter] = b;
				counter++;
			}
		} while (DID_SUCCEED(ret));

		if (counter > bufSize) {
			buf = (byte *)realloc(buf, bufSize + FILE_BUFFER_SIZE);
			bufSize += FILE_BUFFER_SIZE;
		}
		buf[counter] = '\0';

		if (!foundNewLine && counter == 0) {
			stack->pushNULL();
		} else {
			stack->pushString((char *)buf);
		}

		free(buf);

		return STATUS_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// ReadText
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(name, "ReadText") == 0) {
		stack->correctParams(1);
		int textLen = stack->pop()->getInt();

		if (!_textMode || !_readFile) {
			script->runtimeError("File.%s: File must be open in text mode.", name);
			stack->pushNULL();
			return STATUS_OK;
		}
		uint32 bufSize = FILE_BUFFER_SIZE;
		byte *buf = (byte *)malloc(bufSize);
		uint32 counter = 0;
		byte b;

		bool ret = STATUS_FAILED;
		while (counter < (uint32)textLen) {
			ret = _readFile->read(&b, 1);
			if (ret != 1) {
				break;
			}

			if (counter > bufSize) {
				buf = (byte *)realloc(buf, bufSize + FILE_BUFFER_SIZE);
				bufSize += FILE_BUFFER_SIZE;
			}
			if (b == 0x0D) {
				continue;
			} else {
				buf[counter] = b;
				counter++;
			}
		}

		if (counter > bufSize) {
			buf = (byte *)realloc(buf, bufSize + FILE_BUFFER_SIZE);
			bufSize += FILE_BUFFER_SIZE;
		}
		buf[counter] = '\0';

		if (textLen > 0 && counter == 0) {
			stack->pushNULL();
		} else {
			stack->pushString((char *)buf);
		}

		free(buf);

		return STATUS_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// WriteLine / WriteText
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(name, "WriteLine") == 0 || strcmp(name, "WriteText") == 0) {
		stack->correctParams(1);
		const char *line = stack->pop()->getString();
		if (!_textMode || !_writeFile) {
			script->runtimeError("File.%s: File must be open for writing in text mode.", name);
			stack->pushBool(false);
			return STATUS_OK;
		}
		Common::String writeLine;
		if (strcmp(name, "WriteLine") == 0) {
			writeLine = Common::String::format("%s\n", line);
		} else {
			writeLine = Common::String::format("%s", line);
		}
		_writeFile->writeString(writeLine);
		_writeFile->writeByte(0);
		stack->pushBool(true);

		return STATUS_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////
	// ReadBool
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(name, "ReadBool") == 0) {
		stack->correctParams(0);
		if (_textMode || !_readFile) {
			script->runtimeError("File.%s: File must be open for reading in binary mode.", name);
			stack->pushNULL();
			return STATUS_OK;
		}
		bool val;
		if (_readFile->read(&val, sizeof(bool)) == sizeof(bool)) {
			stack->pushBool(val);
		} else {
			stack->pushNULL();
		}

		return STATUS_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// ReadByte
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(name, "ReadByte") == 0) {
		stack->correctParams(0);
		if (_textMode || !_readFile) {
			script->runtimeError("File.%s: File must be open for reading in binary mode.", name);
			stack->pushNULL();
			return STATUS_OK;
		}
		byte val = _readFile->readByte();
		if (!_readFile->err()) {
			stack->pushInt(val);
		} else {
			stack->pushNULL();
		}

		return STATUS_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// ReadShort
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(name, "ReadShort") == 0) {
		stack->correctParams(0);
		if (_textMode || !_readFile) {
			script->runtimeError("File.%s: File must be open for reading in binary mode.", name);
			stack->pushNULL();
			return STATUS_OK;
		}
		int16 val = _readFile->readSint16LE();
		if (!_readFile->err()) {
			stack->pushInt(65536 + val);
		} else {
			stack->pushNULL();
		}
		return STATUS_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// ReadInt / ReadLong
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(name, "ReadInt") == 0 || strcmp(name, "ReadLong") == 0) {
		stack->correctParams(0);
		if (_textMode || !_readFile) {
			script->runtimeError("File.%s: File must be open for reading in binary mode.", name);
			stack->pushNULL();
			return STATUS_OK;
		}
		int32 val = _readFile->readSint32LE();
		if (!_readFile->err()) {
			stack->pushInt(val);
		} else {
			stack->pushNULL();
		}

		return STATUS_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// ReadFloat
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(name, "ReadFloat") == 0) {
		stack->correctParams(0);
		if (_textMode || !_readFile) {
			script->runtimeError("File.%s: File must be open for reading in binary mode.", name);
			stack->pushNULL();
			return STATUS_OK;
		}
		float val;
		(*(uint32 *)&val) = _readFile->readUint32LE();
		if (!_readFile->err()) {
			stack->pushFloat(val);
		} else {
			stack->pushNULL();
		}

		return STATUS_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// ReadDouble
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(name, "ReadDouble") == 0) { // TODO: Solve reading a 8 byte double.
		error("SXFile::ReadDouble - Not endian safe yet");
		stack->correctParams(0);
		if (_textMode || !_readFile) {
			script->runtimeError("File.%s: File must be open for reading in binary mode.", name);
			stack->pushNULL();
			return STATUS_OK;
		}
		double val;
		if (_readFile->read(&val, sizeof(double)) == sizeof(double)) {
			stack->pushFloat(val);
		} else {
			stack->pushNULL();
		}

		return STATUS_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// ReadString
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(name, "ReadString") == 0) {
		stack->correctParams(0);
		if (_textMode || !_readFile) {
			script->runtimeError("File.%s: File must be open for reading in binary mode.", name);
			stack->pushNULL();
			return STATUS_OK;
		}
		uint32 size = _readFile->readUint32LE();
		if (!_readFile->err()) {
			byte *str = new byte[size + 1];
			if (str) {
				if (_readFile->read(str, size) == size) {
					str[size] = '\0';
					stack->pushString((char *)str);
				}
				delete[] str;
			} else {
				stack->pushNULL();
			}
		} else {
			stack->pushNULL();
		}

		return STATUS_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// WriteBool
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(name, "WriteBool") == 0) {
		stack->correctParams(1);
		bool val = stack->pop()->getBool();

		if (_textMode || !_writeFile) {
			script->runtimeError("File.%s: File must be open for writing in binary mode.", name);
			stack->pushBool(false);
			return STATUS_OK;
		}
		_writeFile->writeByte(val);
		stack->pushBool(true);

		return STATUS_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// WriteByte
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(name, "WriteByte") == 0) {
		stack->correctParams(1);
		byte val = stack->pop()->getInt();

		if (_textMode || !_writeFile) {
			script->runtimeError("File.%s: File must be open for writing in binary mode.", name);
			stack->pushBool(false);
			return STATUS_OK;
		}
		_writeFile->writeByte(val);
		stack->pushBool(true);

		return STATUS_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// WriteShort
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(name, "WriteShort") == 0) {
		stack->correctParams(1);
		int16 val = stack->pop()->getInt();

		if (_textMode || !_writeFile) {
			script->runtimeError("File.%s: File must be open for writing in binary mode.", name);
			stack->pushBool(false);
			return STATUS_OK;
		}
		_writeFile->writeSint16LE(val);
		stack->pushBool(true);

		return STATUS_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// WriteInt / WriteLong
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(name, "WriteInt") == 0 || strcmp(name, "WriteLong") == 0) {
		stack->correctParams(1);
		int32 val = stack->pop()->getInt();

		if (_textMode || !_writeFile) {
			script->runtimeError("File.%s: File must be open for writing in binary mode.", name);
			stack->pushBool(false);
			return STATUS_OK;
		}
		_writeFile->writeSint32LE(val);
		stack->pushBool(true);

		return STATUS_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// WriteFloat
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(name, "WriteFloat") == 0) {
		stack->correctParams(1);
		float val = stack->pop()->getFloat();

		if (_textMode || !_writeFile) {
			script->runtimeError("File.%s: File must be open for writing in binary mode.", name);
			stack->pushBool(false);
			return STATUS_OK;
		}
		uint32 *ptr = (uint32 *)&val;
		_writeFile->writeUint32LE(*ptr);
		stack->pushBool(true);

		return STATUS_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// WriteDouble
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(name, "WriteDouble") == 0) {
		error("SXFile::WriteDouble - Not endian safe yet");
		stack->correctParams(1);
		/* double val = */ stack->pop()->getFloat();

		if (_textMode || !_writeFile) {
			script->runtimeError("File.%s: File must be open for writing in binary mode.", name);
			stack->pushBool(false);
			return STATUS_OK;
		}
		//fwrite(&val, sizeof(val), 1, (FILE *)_writeFile);
		stack->pushBool(true);

		return STATUS_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// WriteString
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(name, "WriteString") == 0) {
		stack->correctParams(1);
		const char *val = stack->pop()->getString();

		if (_textMode || !_writeFile) {
			script->runtimeError("File.%s: File must be open for writing in binary mode.", name);
			stack->pushBool(false);
			return STATUS_OK;
		}

		uint32 size = strlen(val);
		_writeFile->writeUint32LE(size);
		_writeFile->writeString(val);

		stack->pushBool(true);

		return STATUS_OK;
	} else {
		return BaseScriptable::scCallMethod(script, stack, thisStack, name);
	}
}


//////////////////////////////////////////////////////////////////////////
ScValue *SXFile::scGetProperty(const Common::String &name) {
	_scValue->setNULL();

	//////////////////////////////////////////////////////////////////////////
	// Type (RO)
	//////////////////////////////////////////////////////////////////////////
	if (name == "Type") {
		_scValue->setString("file");
		return _scValue;
	}

	//////////////////////////////////////////////////////////////////////////
	// Filename (RO)
	//////////////////////////////////////////////////////////////////////////
	if (name == "Filename") {
		_scValue->setString(_filename);
		return _scValue;
	}

	//////////////////////////////////////////////////////////////////////////
	// Position (RO)
	//////////////////////////////////////////////////////////////////////////
	else if (name == "Position") {
		_scValue->setInt(getPos());
		return _scValue;
	}

	//////////////////////////////////////////////////////////////////////////
	// Length (RO)
	//////////////////////////////////////////////////////////////////////////
	else if (name == "Length") {
		_scValue->setInt(getLength());
		return _scValue;
	}

	//////////////////////////////////////////////////////////////////////////
	// TextMode (RO)
	//////////////////////////////////////////////////////////////////////////
	else if (name == "TextMode") {
		_scValue->setBool(_textMode);
		return _scValue;
	}

	//////////////////////////////////////////////////////////////////////////
	// AccessMode (RO)
	//////////////////////////////////////////////////////////////////////////
	else if (name == "AccessMode") {
		_scValue->setInt(_mode);
		return _scValue;
	} else {
		return BaseScriptable::scGetProperty(name);
	}
}


//////////////////////////////////////////////////////////////////////////
bool SXFile::scSetProperty(const char *name, ScValue *value) {
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
uint32 SXFile::getPos() {
	if (_mode == 1 && _readFile) {
		return _readFile->pos();
	} else if ((_mode == 2 || _mode == 3) && _writeFile) {
		error("SXFile - getPos for WriteFile not supported");
		return 0;
//		return ftell((FILE *)_writeFile);
	} else {
		return 0;
	}
}

//////////////////////////////////////////////////////////////////////////
bool SXFile::setPos(uint32 pos, int whence) {
	if (_mode == 1 && _readFile) {
		return _readFile->seek(pos, whence);
	} else if ((_mode == 2 || _mode == 3) && _writeFile) {
		error("SXFile - seeking in WriteFile not supported");
		return false;
//		return fseek((FILE *)_writeFile, pos, (int)origin) == 0;
	} else {
		return false;
	}
}

//////////////////////////////////////////////////////////////////////////
uint32 SXFile::getLength() {
	if (_mode == 1 && _readFile) {
		return _readFile->size();
	} else if ((_mode == 2 || _mode == 3) && _writeFile) {
		error("SXFile - reading length for WriteFile not supported");
		return 0;
		/*
		        uint32 currentPos = ftell((FILE *)_writeFile);
		        fseek((FILE *)_writeFile, 0, SEEK_END);
		        int ret = ftell((FILE *)_writeFile);
		        fseek((FILE *)_writeFile, CurrentPos, SEEK_SET);
		        return Ret;*/
	} else {
		return 0;
	}
}

//////////////////////////////////////////////////////////////////////////
bool SXFile::persist(BasePersistenceManager *persistMgr) {

	BaseScriptable::persist(persistMgr);

	persistMgr->transfer(TMEMBER(_filename));
	persistMgr->transfer(TMEMBER(_mode));
	persistMgr->transfer(TMEMBER(_textMode));

	uint32 pos = 0;
	if (persistMgr->getIsSaving()) {
		pos = getPos();
		persistMgr->transfer(TMEMBER(pos));
	} else {
		persistMgr->transfer(TMEMBER(pos));

		// try to re-open file if needed
		_writeFile = NULL;
		_readFile = NULL;

		if (_mode != 0) {
			// open for reading
			if (_mode == 1) {
				_readFile = BaseFileManager::getEngineInstance()->openFile(_filename);
				if (!_readFile) {
					close();
				}
			}
			// open for writing / appending
			else {
				if (_textMode) {
					if (_mode == 2) {
						_writeFile = openForWrite(_filename, false);
					} else {
						_writeFile = openForAppend(_filename, false);
					}
				} else {
					if (_mode == 2) {
						_writeFile = openForWrite(_filename, true);
					} else {
						_writeFile = openForAppend(_filename, true);
					}
				}
				if (_writeFile) {
					close();
				}
			}
			setPos(pos);
		}
	}

	return STATUS_OK;
}

// Should replace fopen(..., "wb+") and fopen(..., "w+")
Common::WriteStream *SXFile::openForWrite(const Common::String &filename, bool binary) {
	error("SXFile::openForWrite - WriteFiles not supported");
}

// Should replace fopen(..., "ab+") and fopen(..., "a+")
Common::WriteStream *SXFile::openForAppend(const Common::String &filename, bool binary) {
	error("SXFile::openForAppend - WriteFiles not supported");
}

} // end of namespace Wintermute
