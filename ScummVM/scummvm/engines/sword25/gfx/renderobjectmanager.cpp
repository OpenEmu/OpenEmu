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

#include "sword25/gfx/renderobjectmanager.h"

#include "sword25/kernel/kernel.h"
#include "sword25/kernel/inputpersistenceblock.h"
#include "sword25/kernel/outputpersistenceblock.h"
#include "sword25/gfx/graphicengine.h"
#include "sword25/gfx/animationtemplateregistry.h"
#include "common/rect.h"
#include "sword25/gfx/renderobject.h"
#include "sword25/gfx/timedrenderobject.h"
#include "sword25/gfx/rootrenderobject.h"

namespace Sword25 {

RenderObjectManager::RenderObjectManager(int width, int height, int framebufferCount) :
	_frameStarted(false) {
	// Wurzel des BS_RenderObject-Baumes erzeugen.
	_rootPtr = (new RootRenderObject(this, width, height))->getHandle();
}

RenderObjectManager::~RenderObjectManager() {
	// Die Wurzel des Baumes löschen, damit werden alle BS_RenderObjects mitgelöscht.
	_rootPtr.erase();
}

void RenderObjectManager::startFrame() {
	_frameStarted = true;

	// Verstrichene Zeit bestimmen
	int timeElapsed = Kernel::getInstance()->getGfx()->getLastFrameDurationMicro();

	// Alle BS_TimedRenderObject Objekte über den Framestart und die verstrichene Zeit in Kenntnis setzen
	RenderObjectList::iterator iter = _timedRenderObjects.begin();
	for (; iter != _timedRenderObjects.end(); ++iter)
		(*iter)->frameNotification(timeElapsed);
}

bool RenderObjectManager::render() {
	// Den Objekt-Status des Wurzelobjektes aktualisieren. Dadurch werden rekursiv alle Baumelemente aktualisiert.
	// Beim aktualisieren des Objekt-Status werden auch die Update-Rects gefunden, so dass feststeht, was neu gezeichnet
	// werden muss.
	if (!_rootPtr.isValid() || !_rootPtr->updateObjectState())
		return false;

	_frameStarted = false;

	// Die Render-Methode der Wurzel aufrufen. Dadurch wird das rekursive Rendern der Baumelemente angestoßen.
	return _rootPtr->render();
}

void RenderObjectManager::attatchTimedRenderObject(RenderObjectPtr<TimedRenderObject> renderObjectPtr) {
	_timedRenderObjects.push_back(renderObjectPtr);
}

void RenderObjectManager::detatchTimedRenderObject(RenderObjectPtr<TimedRenderObject> renderObjectPtr) {
	for (uint i = 0; i < _timedRenderObjects.size(); i++)
		if (_timedRenderObjects[i] == renderObjectPtr) {
			_timedRenderObjects.remove_at(i);
			break;
		}
}

bool RenderObjectManager::persist(OutputPersistenceBlock &writer) {
	bool result = true;

	// Alle Kinder des Wurzelknotens speichern. Dadurch werden alle BS_RenderObjects gespeichert rekursiv gespeichert.
	result &= _rootPtr->persistChildren(writer);

	writer.write(_frameStarted);

	// Referenzen auf die TimedRenderObjects persistieren.
	writer.write(_timedRenderObjects.size());
	RenderObjectList::const_iterator iter = _timedRenderObjects.begin();
	while (iter != _timedRenderObjects.end()) {
		writer.write((*iter)->getHandle());
		++iter;
	}

	// Alle BS_AnimationTemplates persistieren.
	result &= AnimationTemplateRegistry::instance().persist(writer);

	return result;
}

bool RenderObjectManager::unpersist(InputPersistenceBlock &reader) {
	bool result = true;

	// Alle Kinder des Wurzelknotens löschen. Damit werden alle BS_RenderObjects gelöscht.
	_rootPtr->deleteAllChildren();

	// Alle BS_RenderObjects wieder hestellen.
	if (!_rootPtr->unpersistChildren(reader))
		return false;

	reader.read(_frameStarted);

	// Momentan gespeicherte Referenzen auf TimedRenderObjects löschen.
	_timedRenderObjects.resize(0);

	// Referenzen auf die TimedRenderObjects wieder herstellen.
	uint timedObjectCount;
	reader.read(timedObjectCount);
	for (uint i = 0; i < timedObjectCount; ++i) {
		uint handle;
		reader.read(handle);
		_timedRenderObjects.push_back(handle);
	}

	// Alle BS_AnimationTemplates wieder herstellen.
	result &= AnimationTemplateRegistry::instance().unpersist(reader);

	return result;
}

} // End of namespace Sword25
