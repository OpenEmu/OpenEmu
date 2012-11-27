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

#ifndef WINTERMUTE_SCVALUE_H
#define WINTERMUTE_SCVALUE_H


#include "engines/wintermute/base/base.h"
#include "engines/wintermute/persistent.h"
#include "engines/wintermute/base/scriptables/dcscript.h"   // Added by ClassView
#include "common/str.h"

namespace Wintermute {

class ScScript;
class BaseScriptable;

class ScValue : public BaseClass {
public:
	static int compare(ScValue *val1, ScValue *val2);
	static int compareStrict(ScValue *val1, ScValue *val2);
	TValType getTypeTolerant();
	void cleanup(bool ignoreNatives = false);
	DECLARE_PERSISTENT(ScValue, BaseClass)

	bool _isConstVar;
	bool saveAsText(BaseDynamicBuffer *buffer, int indent);
	void setValue(ScValue *val);
	bool _persistent;
	bool propExists(const char *name);
	void copy(ScValue *orig, bool copyWhole = false);
	void setStringVal(const char *val);
	TValType getType();
	bool getBool(bool defaultVal = false);
	int getInt(int defaultVal = 0);
	double getFloat(double defaultVal = 0.0f);
	const char *getString();
	void *getMemBuffer();
	BaseScriptable *getNative();
	bool deleteProp(const char *name);
	void deleteProps();
	void CleanProps(bool includingNatives);
	void setBool(bool val);
	void setInt(int val);
	void setFloat(double val);
	void setString(const char *val);
	void setString(const Common::String &val);
	void setNULL();
	void setNative(BaseScriptable *val, bool persistent = false);
	void setObject();
	void setReference(ScValue *val);
	bool isNULL();
	bool isNative();
	bool isString();
	bool isBool();
	bool isFloat();
	bool isInt();
	bool isObject();
	bool setProp(const char *name, ScValue *val, bool copyWhole = false, bool setAsConst = false);
	ScValue *getProp(const char *name);
	BaseScriptable *_valNative;
	ScValue *_valRef;
private:
	bool _valBool;
	int _valInt;
	double _valFloat;
	char *_valString;
public:
	TValType _type;
	ScValue(BaseGame *inGame);
	ScValue(BaseGame *inGame, bool Val);
	ScValue(BaseGame *inGame, int Val);
	ScValue(BaseGame *inGame, double Val);
	ScValue(BaseGame *inGame, const char *Val);
	virtual ~ScValue();
	Common::HashMap<Common::String, ScValue *> _valObject;
	Common::HashMap<Common::String, ScValue *>::iterator _valIter;

	bool setProperty(const char *propName, int value);
	bool setProperty(const char *propName, const char *value);
	bool setProperty(const char *propName, double value);
	bool setProperty(const char *propName, bool value);
	bool setProperty(const char *propName);
};

} // end of namespace Wintermute

#endif
