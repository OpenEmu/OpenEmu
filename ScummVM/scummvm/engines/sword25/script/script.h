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
 * This code is based on Broken Sword 2.5 engine
 *
 * Copyright (c) Malte Thiesen, Daniel Queteschiner and Michael Elsdoerfer
 *
 * Licensed under GNU GPL v2
 *
 */

#ifndef SWORD25_SCRIPT_H
#define SWORD25_SCRIPT_H

#include "common/array.h"
#include "common/str.h"
#include "sword25/kernel/common.h"
#include "sword25/kernel/service.h"
#include "sword25/kernel/persistable.h"

namespace Sword25 {

class Kernel;
class OutputPersistenceBlock;
class BS_InputPersistenceBlock;

class ScriptEngine : public Service, public Persistable {
public:
	ScriptEngine(Kernel *KernelPtr) : Service(KernelPtr) {}
	virtual ~ScriptEngine() {}

	// -----------------------------------------------------------------------------
	// This method must be implemented by the script engine
	// -----------------------------------------------------------------------------

	/**
	 * Initializes the scrip tengine. Returns true if successful, false otherwise.
	 */
	virtual bool init() = 0;

	/**
	 * Loads a script file and executes it.
	 * @param FileName      The script filename
	*/
	virtual bool executeFile(const Common::String &fileName) = 0;

	/**
	 * Executes a specified script fragment
	 * @param Code      String of script code
	 */
	virtual bool executeString(const Common::String &code) = 0;

	/**
	 * Returns a pointer to the main object of the script engine
	 * Note: Using this method breaks the encapsulation of the language from the rest of the engine.
	 */
	virtual void *getScriptObject() = 0;

	/**
	 * Makes the command line parameters for the script environment available
	 * Note: How the command line parameters will be used by scripts is dependant on the
	 * particular implementation.
	 * @param CommandLineParameters     List containing the command line parameters
	*/
	virtual void setCommandLine(const Common::Array<Common::String> &commandLineParameters) = 0;

	virtual bool persist(OutputPersistenceBlock &writer) = 0;
	virtual bool unpersist(InputPersistenceBlock &reader) = 0;
};

} // End of namespace Sword25

#endif
