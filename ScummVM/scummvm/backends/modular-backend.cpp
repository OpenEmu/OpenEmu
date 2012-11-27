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

#define FORBIDDEN_SYMBOL_EXCEPTION_exit

#include "backends/modular-backend.h"

#include "backends/graphics/graphics.h"
#include "backends/mutex/mutex.h"

#include "audio/mixer.h"
#include "graphics/pixelformat.h"

ModularBackend::ModularBackend()
	:
	_mutexManager(0),
	_graphicsManager(0),
	_mixer(0) {

}

ModularBackend::~ModularBackend() {
	delete _graphicsManager;
	_graphicsManager = 0;
	delete _mixer;
	_mixer = 0;
	delete _mutexManager;
	_mutexManager = 0;
}

bool ModularBackend::hasFeature(Feature f) {
	return _graphicsManager->hasFeature(f);
}

void ModularBackend::setFeatureState(Feature f, bool enable) {
	return _graphicsManager->setFeatureState(f, enable);
}

bool ModularBackend::getFeatureState(Feature f) {
	return _graphicsManager->getFeatureState(f);
}

GraphicsManager *ModularBackend::getGraphicsManager() {
	assert(_graphicsManager);
	return (GraphicsManager *)_graphicsManager;
}

const OSystem::GraphicsMode *ModularBackend::getSupportedGraphicsModes() const {
	return _graphicsManager->getSupportedGraphicsModes();
}

int ModularBackend::getDefaultGraphicsMode() const {
	return _graphicsManager->getDefaultGraphicsMode();
}

bool ModularBackend::setGraphicsMode(int mode) {
	return _graphicsManager->setGraphicsMode(mode);
}

int ModularBackend::getGraphicsMode() const {
	return _graphicsManager->getGraphicsMode();
}

void ModularBackend::resetGraphicsScale() {
	_graphicsManager->resetGraphicsScale();
}

#ifdef USE_RGB_COLOR

Graphics::PixelFormat ModularBackend::getScreenFormat() const {
	return _graphicsManager->getScreenFormat();
}

Common::List<Graphics::PixelFormat> ModularBackend::getSupportedFormats() const {
	return _graphicsManager->getSupportedFormats();
}

#endif

void ModularBackend::initSize(uint w, uint h, const Graphics::PixelFormat *format ) {
	_graphicsManager->initSize(w, h, format);
}

int ModularBackend::getScreenChangeID() const {
	return _graphicsManager->getScreenChangeID();
}

void ModularBackend::beginGFXTransaction() {
	_graphicsManager->beginGFXTransaction();
}

OSystem::TransactionError ModularBackend::endGFXTransaction() {
	return _graphicsManager->endGFXTransaction();
}

int16 ModularBackend::getHeight() {
	return _graphicsManager->getHeight();
}

int16 ModularBackend::getWidth() {
	return _graphicsManager->getWidth();
}

PaletteManager *ModularBackend::getPaletteManager() {
	return _graphicsManager;
}

void ModularBackend::copyRectToScreen(const void *buf, int pitch, int x, int y, int w, int h) {
	_graphicsManager->copyRectToScreen(buf, pitch, x, y, w, h);
}

Graphics::Surface *ModularBackend::lockScreen() {
	return _graphicsManager->lockScreen();
}

void ModularBackend::unlockScreen() {
	_graphicsManager->unlockScreen();
}

void ModularBackend::fillScreen(uint32 col) {
	_graphicsManager->fillScreen(col);
}

void ModularBackend::updateScreen() {
	_graphicsManager->updateScreen();
}

void ModularBackend::setShakePos(int shakeOffset) {
	_graphicsManager->setShakePos(shakeOffset);
}
void ModularBackend::setFocusRectangle(const Common::Rect& rect) {
	_graphicsManager->setFocusRectangle(rect);
}

void ModularBackend::clearFocusRectangle() {
	_graphicsManager->clearFocusRectangle();
}

void ModularBackend::showOverlay() {
	_graphicsManager->showOverlay();
}

void ModularBackend::hideOverlay() {
	_graphicsManager->hideOverlay();
}

Graphics::PixelFormat ModularBackend::getOverlayFormat() const {
	return _graphicsManager->getOverlayFormat();
}

void ModularBackend::clearOverlay() {
	_graphicsManager->clearOverlay();
}

void ModularBackend::grabOverlay(void *buf, int pitch) {
	_graphicsManager->grabOverlay(buf, pitch);
}

void ModularBackend::copyRectToOverlay(const void *buf, int pitch, int x, int y, int w, int h) {
	_graphicsManager->copyRectToOverlay(buf, pitch, x, y, w, h);
}

int16 ModularBackend::getOverlayHeight() {
	return _graphicsManager->getOverlayHeight();
}

int16 ModularBackend::getOverlayWidth() {
	return _graphicsManager->getOverlayWidth();
}

bool ModularBackend::showMouse(bool visible) {
	return _graphicsManager->showMouse(visible);
}

void ModularBackend::warpMouse(int x, int y) {
	_graphicsManager->warpMouse(x, y);
}

void ModularBackend::setMouseCursor(const void *buf, uint w, uint h, int hotspotX, int hotspotY, uint32 keycolor, bool dontScale, const Graphics::PixelFormat *format) {
	_graphicsManager->setMouseCursor(buf, w, h, hotspotX, hotspotY, keycolor, dontScale, format);
}

void ModularBackend::setCursorPalette(const byte *colors, uint start, uint num) {
	_graphicsManager->setCursorPalette(colors, start, num);
}

OSystem::MutexRef ModularBackend::createMutex() {
	assert(_mutexManager);
	return _mutexManager->createMutex();
}

void ModularBackend::lockMutex(MutexRef mutex) {
	assert(_mutexManager);
	_mutexManager->lockMutex(mutex);
}

void ModularBackend::unlockMutex(MutexRef mutex) {
	assert(_mutexManager);
	_mutexManager->unlockMutex(mutex);
}

void ModularBackend::deleteMutex(MutexRef mutex) {
	assert(_mutexManager);
	_mutexManager->deleteMutex(mutex);
}

Audio::Mixer *ModularBackend::getMixer() {
	assert(_mixer);
	return (Audio::Mixer *)_mixer;
}

void ModularBackend::displayMessageOnOSD(const char *msg) {
	_graphicsManager->displayMessageOnOSD(msg);
}

void ModularBackend::quit() {
	exit(0);
}
