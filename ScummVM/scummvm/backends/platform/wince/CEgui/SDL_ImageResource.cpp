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

#include "backends/platform/sdl/sdl-sys.h"
#include "SDL_ImageResource.h"

namespace CEGUI {

SDL_ImageResource::SDL_ImageResource() :
	_surface(0) {
}

SDL_Surface *SDL_ImageResource::load(WORD resourceID) {
	HRSRC resource;
	HGLOBAL resourceGlobal;
	LPVOID resourcePointer;
	DWORD resourceSize;
	SDL_RWops *surfaceData;
	HMODULE moduleHandle;

	moduleHandle = GetModuleHandle(NULL);
	resource = FindResource(moduleHandle, MAKEINTRESOURCE(resourceID), TEXT("BINARY"));
	if (!resource)
		return NULL;
	resourceSize = SizeofResource(moduleHandle, resource);
	if (!resourceSize)
		return NULL;
	resourceGlobal = LoadResource(moduleHandle, resource);
	if (!resourceGlobal)
		return NULL;
	resourcePointer = LockResource(resourceGlobal);
	if (!resourcePointer)
		return NULL;

	surfaceData = SDL_RWFromMem(resourcePointer, resourceSize);
	if (!surfaceData)
		return NULL;
	_surface = SDL_LoadBMP_RW(surfaceData, 1);

	return _surface;
}

SDL_Surface *SDL_ImageResource::get() {
	return _surface;
}

int SDL_ImageResource::height() {
	if (_surface)
		return _surface->h;
	return 0;
}

int SDL_ImageResource::width() {
	if (_surface)
		return _surface->w;
	return 0;
}

SDL_ImageResource::~SDL_ImageResource() {
	if (_surface)
		SDL_FreeSurface(_surface);
}

} // End of namespace CEGUI
