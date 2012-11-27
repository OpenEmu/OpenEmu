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
    BS_RenderObjectManager
    ----------------------
    Diese Klasse ist f�r die Verwaltung von BS_RenderObjects zust�ndig.

    Sie sorgt z.B. daf�r, dass die BS_RenderObjects in der richtigen Reihenfolge gerendert werden.

    Autor: Malte Thiesen
*/

#ifndef SWORD25_RENDEROBJECTMANAGER_H
#define SWORD25_RENDEROBJECTMANAGER_H

#include "common/rect.h"
#include "sword25/kernel/common.h"
#include "sword25/gfx/renderobjectptr.h"
#include "sword25/kernel/persistable.h"

namespace Sword25 {

class Kernel;
class RenderObject;
class TimedRenderObject;

/**
    @brief Diese Klasse ist f�r die Verwaltung von BS_RenderObjects zust�ndig.

    Sie sorgt daf�r, dass die BS_RenderObjects in der richtigen Reihenfolge gerendert werden und erm�glicht den Zugriff auf die
    BS_RenderObjects �ber einen String.
*/
class RenderObjectManager : public Persistable {
public:
	/**
	    @brief Erzeugt ein neues BS_RenderObjectManager-Objekt.
	    @param Width die horizontale Bildschirmaufl�sung in Pixeln
	    @param Height die vertikale Bildschirmaufl�sung in Pixeln
	    @param Die Anzahl an Framebuffern, die eingesetzt wird (Backbuffer + Primary).
	*/
	RenderObjectManager(int width, int height, int framebufferCount);
	virtual ~RenderObjectManager();

	// Interface
	// ---------
	/**
	    @brief Initialisiert den Manager f�r einen neuen Frame.
	    @remark Alle Ver�nderungen an Objekten m�ssen nach einem Aufruf dieser Methode geschehen, damit sichergestellt ist, dass diese
	            visuell umgesetzt werden.<br>
	            Mit dem Aufruf dieser Methode werden die R�ckgabewerte von GetUpdateRects() und GetUpdateRectCount() auf ihre Startwerte
	            zur�ckgesetzt. Wenn man also mit diesen Werten arbeiten m�chten, muss man dies nach einem Aufruf von Render() und vor
	            einem Aufruf von StartFrame() tun.
	 */
	void startFrame();
	/**
	    @brief Rendert alle Objekte die sich w�hrend des letzten Aufrufes von Render() ver�ndert haben.
	    @return Gibt false zur�ck, falls das Rendern fehlgeschlagen ist.
	 */
	bool render();
	/**
	    @brief Gibt einen Pointer auf die Wurzel des Objektbaumes zur�ck.
	 */
	RenderObjectPtr<RenderObject> getTreeRoot() {
		return _rootPtr;
	}
	/**
	    @brief F�gt ein BS_TimedRenderObject in die Liste der zeitabh�ngigen Render-Objekte.

	    Alle Objekte die sich in dieser Liste befinden werden vor jedem Frame �ber die seit dem letzten Frame
	    vergangene Zeit informiert, so dass sich ihren Zustand zeitabh�ngig ver�ndern k�nnen.

	    @param RenderObject das einzuf�gende BS_TimedRenderObject
	*/
	void attatchTimedRenderObject(RenderObjectPtr<TimedRenderObject> pRenderObject);
	/**
	    @brief Entfernt ein BS_TimedRenderObject aus der Liste f�r zeitabh�ngige Render-Objekte.
	*/
	void detatchTimedRenderObject(RenderObjectPtr<TimedRenderObject> pRenderObject);

	virtual bool persist(OutputPersistenceBlock &writer);
	virtual bool unpersist(InputPersistenceBlock &reader);

private:
	bool _frameStarted;
	typedef Common::Array<RenderObjectPtr<TimedRenderObject> > RenderObjectList;
	RenderObjectList _timedRenderObjects;

	// RenderObject-Tree Variablen
	// ---------------------------
	// Der Baum legt die hierachische Ordnung der BS_RenderObjects fest.
	// Zu weiteren Informationen siehe: "renderobject.h"
	RenderObjectPtr<RenderObject>     _rootPtr;      // Die Wurzel der Baumes
};

} // End of namespace Sword25

#endif
