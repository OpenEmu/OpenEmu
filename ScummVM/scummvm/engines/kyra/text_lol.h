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

#ifndef KYRA_TEXT_LOL_H
#define KYRA_TEXT_LOL_H

#if defined(ENABLE_EOB) || defined(ENABLE_LOL)
#include "kyra/text_rpg.h"
#endif
#include "common/scummsys.h"

#ifdef ENABLE_LOL

namespace Kyra {

class Screen_LoL;
class LoLEngine;
struct EMCState;

class TextDisplayer_LoL : public TextDisplayer_rpg {
public:
	TextDisplayer_LoL(LoLEngine *engine, Screen_LoL *screenLoL);
	~TextDisplayer_LoL();

	void setupField(bool mode);
	void expandField();

	void printDialogueText(int dim, char *str, EMCState *script, const uint16 *paramList, int16 paramIndex);
	void printMessage(uint16 type, const char *str, ...) GCC_PRINTF(3, 4);

	int16 _scriptTextParameter;

private:
	virtual KyraRpgEngine *vm();
	virtual Screen *screen();

	void preprocessString(char *str, EMCState *script, const uint16 *paramList, int16 paramIndex);
	void textPageBreak();

	char *_stringParameters[15];
	char *_buffer;

	LoLEngine *_vm;
	Screen_LoL *_screen;
};

} // End of namespace Kyra

#endif // ENABLE_LOL

#endif
