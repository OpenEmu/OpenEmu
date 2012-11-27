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

#include "common/scummsys.h"

#ifdef __SYMBIAN32__

#include "backends/graphics/symbiansdl/symbiansdl-graphics.h"
#include "backends/platform/symbian/src/SymbianActions.h"

SymbianSdlGraphicsManager::SymbianSdlGraphicsManager(SdlEventSource *sdlEventSource)
	: SurfaceSdlGraphicsManager(sdlEventSource) {
}

int SymbianSdlGraphicsManager::getDefaultGraphicsMode() const {
	return GFX_NORMAL;
}

static const OSystem::GraphicsMode s_supportedGraphicsModes[] = {
	{"1x", "Fullscreen", GFX_NORMAL},
	{0, 0, 0}
};

const OSystem::GraphicsMode *SymbianSdlGraphicsManager::getSupportedGraphicsModes() const {
	return s_supportedGraphicsModes;
}

// make sure we always go to normal, even if the string might be set wrong!
bool SymbianSdlGraphicsManager::setGraphicsMode(int /*name*/) {
	// let parent OSystem_SDL handle it
	return SurfaceSdlGraphicsManager::setGraphicsMode(getDefaultGraphicsMode());
}

bool SymbianSdlGraphicsManager::hasFeature(OSystem::Feature f) {
	switch (f) {
	case OSystem::kFeatureFullscreenMode:
	case OSystem::kFeatureAspectRatioCorrection:
	case OSystem::kFeatureCursorPalette:
#ifdef  USE_VIBRA_SE_PXXX
	case OSystem::kFeatureVibration:
#endif
		return true;
	default:
		return false;
	}
}

void SymbianSdlGraphicsManager::setFeatureState(OSystem::Feature f, bool enable) {
	switch (f) {
	case OSystem::kFeatureVirtualKeyboard:
		break;
	case OSystem::kFeatureDisableKeyFiltering:
		GUI::Actions::Instance()->beginMapping(enable);
		break;
	default:
		SurfaceSdlGraphicsManager::setFeatureState(f, enable);
	}
}

#endif
