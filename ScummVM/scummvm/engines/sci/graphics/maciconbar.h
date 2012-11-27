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

#ifndef SCI_GRAPHICS_MACICONBAR_H
#define SCI_GRAPHICS_MACICONBAR_H

#include "common/array.h"

#include "sci/engine/vm.h"

namespace Graphics {
struct Surface;
}

namespace Sci {

class GfxMacIconBar {
public:
	GfxMacIconBar();
	~GfxMacIconBar();

	void addIcon(reg_t obj);
	void drawIcons();
	void setIconEnabled(int16 index, bool enabled);
	void setInventoryIcon(int16 icon);
	reg_t handleEvents();

private:
	struct IconBarItem {
		reg_t object;
		Graphics::Surface *nonSelectedImage;
		Graphics::Surface *selectedImage;
		Common::Rect rect;
		bool enabled;
	};

	Common::Array<IconBarItem> _iconBarItems;
	uint32 _lastX;
	uint16 _inventoryIndex;
	Graphics::Surface *_inventoryIcon;
	bool _allDisabled;

	Graphics::Surface *loadPict(ResourceId id);
	Graphics::Surface *createImage(uint32 iconIndex, bool isSelected);
	void remapColors(Graphics::Surface *surf, const byte *palette);

	void drawIcon(uint16 index, bool selected);
	void drawSelectedImage(uint16 index);
	bool isIconEnabled(uint16 index) const;
	void drawEnabledImage(Graphics::Surface *surface, const Common::Rect &rect);
	void drawDisabledImage(Graphics::Surface *surface, const Common::Rect &rect);
	bool pointOnIcon(uint32 iconIndex, Common::Point point);
};

} // End of namespace Sci

#endif
