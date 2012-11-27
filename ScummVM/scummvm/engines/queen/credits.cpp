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


#include "queen/credits.h"

#include "queen/display.h"
#include "queen/queen.h"
#include "queen/resource.h"

namespace Queen {

Credits::Credits(QueenEngine *vm, const char* filename) :
	_vm(vm), _running(true), _count(0), _pause(0), _justify(0), _fontSize(0), _color(0), _zone(0), _lineNum(0) {
	_vm->resource()->loadTextFile(filename, _credits);
}

void Credits::nextRoom() {
	if (-1 == _pause) {
		_pause = 0;
		_vm->display()->clearTexts(0, 199);
	}
}

void Credits::update() {
	if (!_running)
		return;

	if (_pause > 0) {
		_pause--;
		if (!_pause)
			_vm->display()->clearTexts(0, 199);
		return;
	}

	/* wait until next room */
	if (-1 == _pause)
		return;

	while (_lineNum < _credits.size()) {
		const char *line = _credits[_lineNum].c_str();
		++_lineNum;

		if (0 == memcmp(line, "EN", 2)) {
			_running = false;
			return;
		}

		if ('.' == line[0]) {
			int i;

			switch (tolower(line[1])) {
			case 'l' :
				_justify = 0;
				break;
			case 'c' :
				_justify = 1;
				break;
			case 'r' :
				_justify = 2;
				break;
			case 's' :
				_fontSize = 0;
				break;
			case 'b' :
				_fontSize = 1;
				break;
			case 'p' :
				_pause = atoi(&line[3]);
				_pause *= 10;
				/* wait until next room */
				if (0 == _pause)
					_pause = -1;
				for (i = 0; i < _count; i++)	{
					_vm->display()->textCurrentColor(_list[i].color);
					_vm->display()->setText(_list[i].x, _list[i].y, _list[i].text);
				}
				_count = 0;
				return;
			case 'i' :
				_color = atoi(&line[3]);
				if (_vm->resource()->getPlatform() == Common::kPlatformAmiga) {
					_color &= 31;
				}
				break;
			case '1' :
			case '2' :
			case '3' :
			case '4' :
			case '5' :
			case '6' :
			case '7' :
			case '8' :
			case '9' :
				_zone = line[1] - '1';
				break;
			}
		} else {
			assert(_count < ARRAYSIZE(_list));
			_list[_count].text = line;
			_list[_count].color = _color;
			_list[_count].fontSize = _fontSize;
			switch (_justify) {
			case 0:
				_list[_count].x = (_zone % 3) * (320 / 3) + 8;
				break;
			case 1:
				_list[_count].x = (_zone % 3) * (320 / 3) + 54 - _vm->display()->textWidth(line) / 2;
				if (_list[_count].x < 8)
					_list[_count].x = 8;
				break;
			case 2:
				_list[_count].x = (_zone % 3) * (320 / 3) + 100 - _vm->display()->textWidth(line);
				break;
			}
			_list[_count].y = (_zone / 3) * (200 / 3) + (_count * 10);
			_count++;
		}
	}
	_running = false;
}


} // End of namespace Queen
