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

#ifndef COMMON_RENDERMODE_H
#define COMMON_RENDERMODE_H

#include "common/scummsys.h"

namespace Common {

class String;

/**
 * List of render modes. It specifies which original graphics mode
 * to use. Some targets used postprocessing dithering routines for
 * reducing color depth of final image which let it to be rendered on
 * such low-level adapters as CGA or Hercules.
 */
enum RenderMode {
	kRenderDefault = 0,
	kRenderVGA = 1,
	kRenderEGA = 2,
	kRenderCGA = 3,
	kRenderHercG = 4,
	kRenderHercA = 5,
	kRenderAmiga = 6,
	kRenderFMTowns = 7,
	kRenderPC9821 = 8,
	kRenderPC9801 = 9
};

struct RenderModeDescription {
	const char *code;
	const char *description;
	RenderMode id;
};

extern const RenderModeDescription g_renderModes[];

/** Convert a string containing a render mode name into a RenderingMode enum value. */
extern RenderMode parseRenderMode(const String &str);
extern const char *getRenderModeCode(RenderMode id);
extern const char *getRenderModeDescription(RenderMode id);

// TODO: Rename the following to something better; also, document it
extern String renderMode2GUIO(RenderMode id);

// TODO: Rename the following to something better; also, document it
extern String allRenderModesGUIOs();


} // End of namespace Common

#endif
