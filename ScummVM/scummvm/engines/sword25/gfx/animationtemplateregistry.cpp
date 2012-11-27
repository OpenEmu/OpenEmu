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

#include "sword25/kernel/outputpersistenceblock.h"
#include "sword25/kernel/inputpersistenceblock.h"
#include "sword25/gfx/animationtemplateregistry.h"
#include "sword25/gfx/animationtemplate.h"

namespace Common {
DECLARE_SINGLETON(Sword25::AnimationTemplateRegistry);
}

namespace Sword25 {

bool AnimationTemplateRegistry::persist(OutputPersistenceBlock &writer) {
	bool result = true;

	// Das nächste zu vergebene Handle schreiben.
	writer.write(_nextHandle);

	// Anzahl an BS_AnimationTemplates schreiben.
	writer.write(_handle2PtrMap.size());

	// Alle BS_AnimationTemplates persistieren.
	HANDLE2PTR_MAP::const_iterator iter = _handle2PtrMap.begin();
	while (iter != _handle2PtrMap.end()) {
		// Handle persistieren.
		writer.write(iter->_key);

		// Objekt persistieren.
		result &= iter->_value->persist(writer);

		++iter;
	}

	return result;
}

// -----------------------------------------------------------------------------

bool AnimationTemplateRegistry::unpersist(InputPersistenceBlock &reader) {
	bool result = true;

	// Das nächste zu vergebene Handle wieder herstellen.
	reader.read(_nextHandle);

	// Alle vorhandenen BS_AnimationTemplates zerstören.
	while (!_handle2PtrMap.empty())
		delete _handle2PtrMap.begin()->_value;

	// Anzahl an BS_AnimationTemplates einlesen.
	uint animationTemplateCount;
	reader.read(animationTemplateCount);

	// Alle gespeicherten BS_AnimationTemplates wieder herstellen.
	for (uint i = 0; i < animationTemplateCount; ++i) {
		// Handle lesen.
		uint handle;
		reader.read(handle);

		// BS_AnimationTemplate wieder herstellen.
		result &= (AnimationTemplate::create(reader, handle) != 0);
	}

	return reader.isGood() && result;
}

} // End of namespace Sword25
