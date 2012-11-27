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
 */

#include "tinsel/actors.h"
#include "tinsel/dw.h"
#include "tinsel/font.h"
#include "tinsel/handle.h"
#include "tinsel/object.h"
#include "tinsel/sysvar.h"
#include "tinsel/text.h"
#include "tinsel/tinsel.h"

namespace Tinsel {

//----------------- LOCAL GLOBAL DATA --------------------

// FIXME: Avoid non-const global vars

static char g_tBuffer[TBUFSZ];

static SCNHANDLE g_hTagFont = 0, g_hTalkFont = 0;
static SCNHANDLE g_hRegularTalkFont = 0, g_hRegularTagFont = 0;


/**
 * Return address of tBuffer
 */
char *TextBufferAddr() {
	return g_tBuffer;
}

/**
 * Return hTagFont handle.
 */
SCNHANDLE GetTagFontHandle() {
	return g_hTagFont;
}

/**
 * Return hTalkFont handle.
 */
SCNHANDLE GetTalkFontHandle() {
	return g_hTalkFont;
}

/**
 * Called from dec_tagfont() Glitter function. Store the tag font handle.
 */
void SetTagFontHandle(SCNHANDLE hFont) {
	g_hTagFont = g_hRegularTagFont = hFont;		// Store the font handle
}

/**
 * Called from dec_talkfont() Glitter function.
 * Store the talk font handle.
 */
void SetTalkFontHandle(SCNHANDLE hFont) {
	g_hTalkFont = g_hRegularTalkFont = hFont;		// Store the font handle
}

void SetTempTagFontHandle(SCNHANDLE hFont) {
	g_hTagFont = hFont;
}

void SetTempTalkFontHandle(SCNHANDLE hFont) {
	g_hTalkFont = hFont;
}

void ResetFontHandles() {
	g_hTagFont = g_hRegularTagFont;
	g_hTalkFont = g_hRegularTalkFont;
}


/**
 * Poke the background palette into character 0's images.
 */
void FettleFontPal(SCNHANDLE fontPal) {
	const FONT *pFont;
	IMAGE *pImg;

	assert(fontPal);
	assert(g_hTagFont); // Tag font not declared
	assert(g_hTalkFont); // Talk font not declared

	pFont = (const FONT *)LockMem(g_hTagFont);
	pImg = (IMAGE *)LockMem(FROM_LE_32(pFont->fontInit.hObjImg));	// get image for char 0
	if (!TinselV2)
		pImg->hImgPal = TO_LE_32(fontPal);
	else
		pImg->hImgPal = 0;

	pFont = (const FONT *)LockMem(g_hTalkFont);
	pImg = (IMAGE *)LockMem(FROM_LE_32(pFont->fontInit.hObjImg));	// get image for char 0
	if (!TinselV2)
		pImg->hImgPal = TO_LE_32(fontPal);
	else
		pImg->hImgPal = 0;

	if (TinselV2 && SysVar(SV_TAGCOLOR)) {
		const COLORREF c = GetActorRGB(-1);
		SetTagColorRef(c);
		UpdateDACqueue(SysVar(SV_TAGCOLOR), c);
	}
}

} // End of namespace Tinsel
