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
 * BS_Service
 * -------------
 * This is the base class for all engine services.
 * A serivce is an essential part of the engine, ex. the graphics system.
 * This was intended to allow, for example, different plug in modules for
 * different kinds of hardware and/or systems.
 * The services are created at runtime via the kernel method NewService and NEVER with new.
 *
 * Autor: Malte Thiesen
*/

#ifndef SWORD25_SERVICE_H
#define SWORD25_SERVICE_H

// Includes
#include "sword25/kernel/common.h"

namespace Sword25 {

// Klassendefinition
class Kernel;

class Service {
private:
	Kernel  *_pKernel;

protected:
	Service(Kernel *pKernel) : _pKernel(pKernel) {}

	Kernel *GetKernel() const {
		return _pKernel;
	}

public:
	virtual ~Service() {}
};

} // End of namespace Sword25

#endif
