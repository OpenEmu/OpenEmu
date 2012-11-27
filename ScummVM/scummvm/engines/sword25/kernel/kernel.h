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

/*
 * BS_Kernel
 * ---------
 * This is the main class of the engine.
 * This class creates and manages all other Engine elements: the sound engine, graphics engine ...
 * It is not necessary to release all the items individually, this is performed by the Kernel class.
 *
 * Autor: Malte Thiesen
 */

#ifndef SWORD25_KERNEL_H
#define SWORD25_KERNEL_H

#include "common/scummsys.h"
#include "common/random.h"
#include "common/stack.h"
#include "common/textconsole.h"
#include "common/util.h"
#include "engines/engine.h"

#include "sword25/kernel/common.h"
#include "sword25/kernel/resmanager.h"

namespace Sword25 {

// Class definitions
class Service;
class Geometry;
class GraphicEngine;
class ScriptEngine;
class SoundEngine;
class InputEngine;
class PackageManager;
class MoviePlayer;

/**
 * This is the main engine class
 *
 * This class creates and manages all other engine components such as sound engine, graphics engine ...
 * It is not necessary to release all the items individually, this is performed by the Kernel class.
*/
class Kernel {
public:

	/**
	 * Returns the elapsed time since startup in milliseconds
	 */
	uint getMilliTicks();

	/**
	 * Specifies whether the kernel was successfully initialized
	 */
	bool getInitSuccess() const {
		return _initSuccess;
	}
	/**
	 * Returns a pointer to the BS_ResourceManager
	 */
	ResourceManager *getResourceManager() {
		return _resourceManager;
	}
	/**
	 * Returns a random number
	 * @param Min       The minimum allowed value
	 * @param Max       The maximum allowed value
	 */
	int getRandomNumber(int min, int max);
	/**
	 * Returns a pointer to the active Gfx Service, or NULL if no Gfx service is active
	 */
	GraphicEngine *getGfx();
	/**
	 * Returns a pointer to the active Sfx Service, or NULL if no Sfx service is active
	 */
	SoundEngine *getSfx();
	/**
	 * Returns a pointer to the active input service, or NULL if no input service is active
	 */
	InputEngine *getInput();
	/**
	 * Returns a pointer to the active package manager, or NULL if no manager is active
	 */
	PackageManager *getPackage();
	/**
	 * Returns a pointer to the script engine, or NULL if it is not active
	 */
	ScriptEngine *getScript();

	/**
	 * Returns a pointer to the movie player, or NULL if it is not active
	 */
	MoviePlayer *getFMV();

	/**
	 * Pauses for the specified amount of time
	 * @param Msecs     The amount of time in milliseconds
	 */
	void sleep(uint msecs) const;

	/**
	 * Returns the singleton instance for the kernel
	 */
	static Kernel *getInstance() {
		if (!_instance)
			_instance = new Kernel();
		return _instance;
	}

	/**
	 * Destroys the kernel instance
	 * This method should only be called when the game is ended. No subsequent calls to any kernel
	 * methods should be done after calling this method.
	 */
	static void deleteInstance() {
		if (_instance) {
			delete _instance;
			_instance = NULL;
		}
	}

	/**
	 * Raises an error. This method is used in crashing testing.
	 */
	void crash() const {
		error("Kernel::Crash");
	}

private:
	// -----------------------------------------------------------------------------
	// Constructor / destructor
	// Private singleton methods
	// -----------------------------------------------------------------------------

	Kernel();
	virtual ~Kernel();

	// -----------------------------------------------------------------------------
	// Singleton instance
	// -----------------------------------------------------------------------------
	static Kernel *_instance;

	bool _initSuccess; // Specifies whether the engine was set up correctly

	// Random number generator
	// -----------------------
	Common::RandomSource _rnd;

	// Resourcemanager
	// ---------------
	ResourceManager *_resourceManager;

	GraphicEngine *_gfx;
	SoundEngine *_sfx;
	InputEngine *_input;
	PackageManager *_package;
	ScriptEngine *_script;
	Geometry *_geometry;
	MoviePlayer *_fmv;

	bool registerScriptBindings();
};

} // End of namespace Sword25

#endif
