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

#ifndef LURE_SURFACE_H
#define LURE_SURFACE_H


#include "common/str.h"
#include "common/ptr.h"
#include "lure/disk.h"
#include "lure/luredefs.h"

namespace Lure {

#define DEFAULT_TEXT_COLOR -1

class Surface {
private:
	MemoryBlock *_data;
	uint16 _width, _height;

	void egaCreateDialog(bool blackFlag);
	void vgaCreateDialog(bool blackFlag);
public:
	Surface(MemoryBlock *src, uint16 width, uint16 height);
	Surface(uint16 width, uint16 height);
	~Surface();
	static uint16 textX();
	static uint16 textY();
	static void getDialogBounds(Common::Point &size, int charWidth, int numLines,
		bool squashedLines = true);

	static void initialize();
	static void deinitialize();

	uint16 width() { return _width; }
	uint16 height() { return _height; }
	MemoryBlock &data() { return *_data; }

	void loadScreen(uint16 resourceId);
	void loadScreen(MemoryBlock *data);
	int writeChar(uint16 x, uint16 y, uint8 ascii, bool transparent, int color);
	void writeString(uint16 x, uint16 y, Common::String line, bool transparent,
		int color = DEFAULT_TEXT_COLOR, bool varLength = true);
	void writeSubstring(uint16 x, uint16 y, Common::String line, int len,
		bool transparent, int color = DEFAULT_TEXT_COLOR, bool varLength = true);
	void transparentCopyTo(Surface *dest);
	void copyTo(Surface *dest);
	void copyTo(Surface *dest, uint16 x, uint16 y);
	void copyTo(Surface *dest, const Common::Rect &srcBounds, uint16 destX, uint16 destY,
		int transparentColor = -1);
	void copyFrom(MemoryBlock *src) { _data->copyFrom(src); }
	void copyFrom(MemoryBlock *src, uint32 destOffset);
	void empty() { _data->empty(); }
	void fillRect(const Common::Rect &r, uint8 color);
	void createDialog(bool blackFlag = false);
	void copyToScreen(uint16 x, uint16 y);
	void centerOnScreen();

	static uint16 textWidth(const char *s, int numChars = 0);
	static void wordWrap(char *text, uint16 width, char **&lines, uint8 &numLines);
	static Surface *newDialog(uint16 width, uint8 numLines, const char **lines, bool varLength = true,
		int color = DEFAULT_TEXT_COLOR, bool squashedLines = true);
	static Surface *newDialog(uint16 width, const char *lines, int color = DEFAULT_TEXT_COLOR);
	static Surface *getScreen(uint16 resourceId);
	bool getString(Common::String &line, int maxSize, bool isNumeric, bool varLength, int16 x, int16 y);
};

class Dialog {
public:
	static void show(const char *text);
	static void show(uint16 stringId, const char *hotspotName, const char *characterName);
	static void show(uint16 stringId);
};

class TalkDialog {
private:
	Surface *_surface;
	char _desc[MAX_DESC_SIZE];
	char **_lines;
	uint8 _numLines;
	int _endLine, _endIndex;
	int _wordCountdown;

	uint16 _characterId;
	uint16 _destCharacterId;
	uint16 _activeItemId;
	uint16 _descId;

	int getArticle(uint16 msgId, uint16 objId);
	void vgaTalkDialog(Surface *s);
public:
	TalkDialog(uint16 characterId, uint16 destCharacterId, uint16 activeItemId, uint16 descId);
	~TalkDialog();

	char *desc() { return _desc; }
	Surface &surface() { return *_surface; }
	void copyTo(Surface *dest, uint16 x, uint16 y);
	bool isBuilding() { return _endLine < _numLines; }

	void saveToStream(Common::WriteStream *stream);
	static TalkDialog *loadFromStream(Common::ReadStream *stream);
};

class SaveRestoreDialog {
private:
	static void toggleHightlight(int xs, int xe, int ys, int ye);
public:
	static bool show(bool saveDialog);
};

class RestartRestoreDialog {
public:
	static bool show();
};

class CopyProtectionDialog {
private:
	typedef Common::List<Common::SharedPtr<Hotspot> > HotspotsList;
	HotspotsList _hotspots;
	int _charIndex;

	void chooseCharacters();
public:
	CopyProtectionDialog();
	bool show();
};

} // End of namespace Lure

#endif
