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

#ifndef KYRA_TEXT_MR_H
#define KYRA_TEXT_MR_H

#include "kyra/text.h"

#include "kyra/kyra_mr.h"

namespace Kyra {

class TextDisplayer_MR : public TextDisplayer {
friend class KyraEngine_MR;
public:
	TextDisplayer_MR(KyraEngine_MR *vm, Screen_MR *screen);

	char *preprocessString(const char *str);
	int dropCRIntoString(char *str, int minOffs, int maxOffs);

	void printText(const char *str, int x, int y, uint8 c0, uint8 c1, uint8 c2);

	void restoreScreen();

	void calcWidestLineBounds(int &x1, int &x2, int w, int x);
protected:
	KyraEngine_MR *_vm;
	Screen_MR *_screen;
};

} // End of namespace Kyra

#endif
