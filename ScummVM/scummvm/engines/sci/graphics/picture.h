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

#ifndef SCI_GRAPHICS_PICTURE_H
#define SCI_GRAPHICS_PICTURE_H

namespace Sci {

#define SCI_PATTERN_CODE_RECTANGLE 0x10
#define SCI_PATTERN_CODE_USE_TEXTURE 0x20
#define SCI_PATTERN_CODE_PENSIZE 0x07

enum {
	SCI_PICTURE_TYPE_REGULAR		= 0,
	SCI_PICTURE_TYPE_SCI11		= 1,
	SCI_PICTURE_TYPE_SCI32		= 2
};

class GfxPorts;
class GfxScreen;
class GfxPalette;

/**
 * Picture class, handles loading and displaying of picture resources
 *  every picture resource has its own instance of this class
 */
class GfxPicture {
public:
	GfxPicture(ResourceManager *resMan, GfxCoordAdjuster *coordAdjuster, GfxPorts *ports, GfxScreen *screen, GfxPalette *palette, GuiResourceId resourceId, bool EGAdrawingVisualize = false);
	~GfxPicture();

	GuiResourceId getResourceId();
	void draw(int16 animationNr, bool mirroredFlag, bool addToFlag, int16 EGApaletteNo);

#ifdef ENABLE_SCI32
	int16 getSci32celCount();
	int16 getSci32celY(int16 celNo);
	int16 getSci32celX(int16 celNo);
	int16 getSci32celWidth(int16 celNo);
	int16 getSci32celHeight(int16 celNo);
	int16 getSci32celPriority(int16 celNo);
	void drawSci32Vga(int16 celNo, int16 callerX, int16 callerY, int16 pictureX, int16 pictureY, bool mirrored);
#endif

private:
	void initData(GuiResourceId resourceId);
	void reset();
	void drawSci11Vga();
	void drawCelData(byte *inbuffer, int size, int headerPos, int rlePos, int literalPos, int16 drawX, int16 drawY, int16 pictureX, int16 pictureY);
	void drawVectorData(byte *data, int size);
	bool vectorIsNonOpcode(byte pixel);
	void vectorGetAbsCoords(byte *data, int &curPos, int16 &x, int16 &y);
	void vectorGetAbsCoordsNoMirror(byte *data, int &curPos, int16 &x, int16 &y);
	void vectorGetRelCoords(byte *data, int &curPos, int16 &x, int16 &y);
	void vectorGetRelCoordsMed(byte *data, int &curPos, int16 &x, int16 &y);
	void vectorGetPatternTexture(byte *data, int &curPos, int16 pattern_Code, int16 &pattern_Texture);
	void vectorFloodFill(int16 x, int16 y, byte color, byte prio, byte control);
	void vectorPattern(int16 x, int16 y, byte pic_color, byte pic_priority, byte pic_control, byte code, byte texture);
	void vectorPatternBox(Common::Rect box, byte color, byte prio, byte control);
	void vectorPatternTexturedBox(Common::Rect box, byte color, byte prio, byte control, byte texture);
	void vectorPatternCircle(Common::Rect box, byte size, byte color, byte prio, byte control);
	void vectorPatternTexturedCircle(Common::Rect box, byte size, byte color, byte prio, byte control, byte texture);

	ResourceManager *_resMan;
	GfxCoordAdjuster *_coordAdjuster;
	GfxPorts *_ports;
	GfxScreen *_screen;
	GfxPalette *_palette;

	int16 _resourceId;
	Resource *_resource;
	int _resourceType;

	int16 _animationNr;
	bool _mirroredFlag;
	bool _addToFlag;
	int16 _EGApaletteNo;
	byte _priority;

	// If true, we will show the whole EGA drawing process...
	bool _EGAdrawingVisualize;
};

} // End of namespace Sci

#endif
