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

#ifndef COMMON_GUI_OPTIONS_H
#define COMMON_GUI_OPTIONS_H

#define GUIO_NONE			"\000"
#define GUIO_NOSUBTITLES	"\001"
#define GUIO_NOMUSIC		"\002"
#define GUIO_NOSPEECH		"\003"
#define GUIO_NOSFX			"\004"
#define GUIO_NOMIDI			"\005"
#define GUIO_NOLAUNCHLOAD	"\006"

#define GUIO_MIDIPCSPK		"\007"
#define GUIO_MIDICMS		"\010"
#define GUIO_MIDIPCJR		"\011"
#define GUIO_MIDIADLIB		"\012"
#define GUIO_MIDIC64        "\013"
#define GUIO_MIDIAMIGA      "\014"
#define GUIO_MIDIAPPLEIIGS  "\015"
#define GUIO_MIDITOWNS		"\016"
#define GUIO_MIDIPC98		"\017"
#define GUIO_MIDIMT32		"\020"
#define GUIO_MIDIGM			"\021"

#define GUIO_NOASPECT		"\022"

#define GUIO_RENDERHERCGREEN	"\030"
#define GUIO_RENDERHERCAMBER	"\031"
#define GUIO_RENDERCGA		"\032"
#define GUIO_RENDEREGA		"\033"
#define GUIO_RENDERVGA		"\034"
#define GUIO_RENDERAMIGA	"\035"
#define GUIO_RENDERFMTOWNS	"\036"
#define GUIO_RENDERPC9821	"\037"
#define GUIO_RENDERPC9801	"\040"

// Special GUIO flags for the AdvancedDetector's caching of game specific
// options.
#define GUIO_GAMEOPTIONS1	"\041"
#define GUIO_GAMEOPTIONS2	"\042"
#define GUIO_GAMEOPTIONS3	"\043"
#define GUIO_GAMEOPTIONS4	"\044"
#define GUIO_GAMEOPTIONS5	"\045"
#define GUIO_GAMEOPTIONS6	"\046"
#define GUIO_GAMEOPTIONS7	"\047"

#define GUIO0() (GUIO_NONE)
#define GUIO1(a) (a)
#define GUIO2(a,b) (a b)
#define GUIO3(a,b,c) (a b c)
#define GUIO4(a,b,c,d) (a b c d)
#define GUIO5(a,b,c,d,e) (a b c d e)
#define GUIO6(a,b,c,d,e,f) (a b c d e f)
#define GUIO7(a,b,c,d,e,f,g) (a b c d e f g)
#define GUIO8(a,b,c,d,e,f,g,h) (a b c d e f g h)

namespace Common {

class String;

bool checkGameGUIOption(const String &option, const String &str);
String parseGameGUIOptions(const String &str);
const String getGameGUIOptionsDescription(const String &options);

/**
 * Updates the GUI options of the current config manager
 * domain, when they differ to the ones passed as
 * parameter.
 */
void updateGameGUIOptions(const String &options, const String &langOption);


} // End of namespace Common

#endif
