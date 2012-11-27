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

#ifndef MOHAWK_MYST_GRAPHICS_H
#define MOHAWK_MYST_GRAPHICS_H

#include "mohawk/graphics.h"

#include "common/file.h"

namespace Mohawk {

class MystBitmap;
class MohawkEngine_Myst;

enum RectState {
	kRectEnabled,
	kRectDisabled,
	kRectUnreachable
};

class MystGraphics : public GraphicsManager {
public:
	MystGraphics(MohawkEngine_Myst*);
	~MystGraphics();

	void copyImageSectionToScreen(uint16 image, Common::Rect src, Common::Rect dest);
	void copyImageSectionToBackBuffer(uint16 image, Common::Rect src, Common::Rect dest);
	void copyImageToScreen(uint16 image, Common::Rect dest);
	void copyImageToBackBuffer(uint16 image, Common::Rect dest);
	void copyBackBufferToScreen(Common::Rect r);
	void runTransition(uint16 type, Common::Rect rect, uint16 steps, uint16 delay);
	void drawRect(Common::Rect rect, RectState state);
	void drawLine(const Common::Point &p1, const Common::Point &p2, uint32 color);
	void enableDrawingTimeSimulation(bool enable);
	void fadeToBlack();
	void fadeFromBlack();

protected:
	MohawkSurface *decodeImage(uint16 id);
	MohawkEngine *getVM() { return (MohawkEngine *)_vm; }
	void simulatePreviousDrawDelay(const Common::Rect &dest);
	void copyBackBufferToScreenWithSaturation(int16 saturation);

private:
	MohawkEngine_Myst *_vm;
	MystBitmap *_bmpDecoder;

	Graphics::Surface *_backBuffer;
	Graphics::PixelFormat _pixelFormat;
	Common::Rect _viewport;

	int _enableDrawingTimeSimulation;
	uint32 _nextAllowedDrawTime;
	static const uint _constantDrawDelay = 10; // ms
	static const uint _proportionalDrawDelay = 500; // pixels per ms
};

} // End of namespace Mohawk

#endif
