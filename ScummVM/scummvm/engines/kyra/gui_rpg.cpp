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

#if defined(ENABLE_EOB) || defined(ENABLE_LOL)

#include "kyra/kyra_rpg.h"

namespace Kyra {

void KyraRpgEngine::removeInputTop() {
	if (!_eventList.empty()) {
		if (_eventList.begin()->event.type == Common::EVENT_LBUTTONDOWN)
			_mouseClick = 1;
		else if (_eventList.begin()->event.type == Common::EVENT_RBUTTONDOWN)
			_mouseClick = 2;
		else
			_mouseClick = 0;

		_eventList.erase(_eventList.begin());
	}
}

void KyraRpgEngine::gui_drawBox(int x, int y, int w, int h, int frameColor1, int frameColor2, int fillColor) {
	w--;
	h--;
	if (fillColor != -1)
		screen()->fillRect(x + 1, y + 1, x + w - 1, y + h - 1, fillColor);

	screen()->drawClippedLine(x + 1, y, x + w, y, frameColor2);
	screen()->drawClippedLine(x + w, y, x + w, y + h - 1, frameColor2);
	screen()->drawClippedLine(x, y, x, y + h, frameColor1);
	screen()->drawClippedLine(x, y + h, x + w, y + h, frameColor1);
}

void KyraRpgEngine::gui_drawHorizontalBarGraph(int x, int y, int w, int h, int32 cur, int32 max, int col1, int col2) {
	if (max < 1)
		return;
	if (cur < 0)
		cur = 0;

	int32 e = MIN(cur, max);

	if (!--w)
		return;
	if (!--h)
		return;

	int32 t = (e * w) / max;

	if (!t && e)
		t++;

	if (t)
		screen()->fillRect(x, y, x + t - 1, y + h, col1);

	if (t < w && col2)
		screen()->fillRect(x + t, y, x + w - 1, y + h, col2);
}

void KyraRpgEngine::gui_initButtonsFromList(const int16 *list) {
	while (*list != -1)
		gui_initButton(*list++);
}

void KyraRpgEngine::gui_resetButtonList() {
	for (uint i = 0; i < ARRAYSIZE(_activeButtonData); ++i)
		_activeButtonData[i].nextButton = 0;

	gui_notifyButtonListChanged();
	_activeButtons = 0;
}

void KyraRpgEngine::gui_notifyButtonListChanged() {
	if (gui()) {
		if (!_buttonListChanged && !_preserveEvents)
			removeInputTop();
		_buttonListChanged = true;
	}
}

bool KyraRpgEngine::clickedShape(int shapeIndex) {
	if (_clickedSpecialFlag != 0x40)
		return true;

	for (; shapeIndex; shapeIndex = _levelDecorationProperties[shapeIndex].next) {
		if (_flags.gameID != GI_LOL)
			shapeIndex--;

		uint16 s = _levelDecorationProperties[shapeIndex].shapeIndex[1];

		if (s == 0xffff)
			continue;

		int w = _flags.gameID == GI_LOL ? _levelDecorationShapes[s][3] : (_levelDecorationShapes[s][2] << 3);
		int h = _levelDecorationShapes[s][_flags.gameID == GI_LOL ? 2 : 1];
		int x = _levelDecorationProperties[shapeIndex].shapeX[1] + _clickedShapeXOffs;
		int y = _levelDecorationProperties[shapeIndex].shapeY[1] + _clickedShapeYOffs;

		if (_levelDecorationProperties[shapeIndex].flags & 1) {
			if (_flags.gameID == GI_LOL)
				w <<= 1;
			else
				x = 176 - x - w;
		}

		if (posWithinRect(_mouseX, _mouseY, x - 4, y - 4, x + w + 8, y + h + 8))
			return true;
	}

	return false;
}

} // End of namespace Kyra

#endif // defined(ENABLE_EOB) || defined(ENABLE_LOL)
