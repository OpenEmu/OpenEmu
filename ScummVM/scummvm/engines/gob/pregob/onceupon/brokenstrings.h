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

#ifndef GOB_PREGOB_ONCEUPON_BROKENSTRINGS_H
#define GOB_PREGOB_ONCEUPON_BROKENSTRINGS_H

struct BrokenString {
	const char *wrong;
	const char *correct;
};

struct BrokenStringLanguage {
	const BrokenString *strings;
	uint count;
};

static const BrokenString kBrokenStringsGerman[] = {
	{ "Zeichungen von Kaki,"         , "Zeichnungen von Kaki,"        },
	{ "die es in seine Wachtr\204ume", "die es in seine Tagtr\204ume" },
	{ "   Spielerfahrung"            , "    Spielerfahren"            },
	{ "  Fortgeschrittene"           , "  Fortgeschritten"            },
	{ "die Vespe"                    , "die Wespe"                    },
	{ "das Rhinoceros"               , "das Rhinozeros"               },
	{ "die Heusschrecke"             , "die Heuschrecke"              },
	{ "Das, von Drachen gebrachte"   , "Das vom Drachen gebrachte"    },
	{ "Am Waldesrand es sieht"       , "Am Waldesrand sieht es"       },
	{ " das Kind den Palast."        , "das Kind den Palast."         },
	{ "Am Waldessaum sieht"          , "Am Waldesrand sieht"          },
	{ "tipp auf ESC!"                , "dr\201cke ESC!"               },
	{ "Wohin fliegt der Drachen?"    , "Wohin fliegt der Drache?"     }
};

static const BrokenStringLanguage kBrokenStrings[kLanguageCount] = {
	{                    0,                               0 }, // French
	{ kBrokenStringsGerman, ARRAYSIZE(kBrokenStringsGerman) }, // German
	{                    0,                               0 }, // English
	{                    0,                               0 }, // Spanish
	{                    0,                               0 }, // Italian
};

#endif // GOB_PREGOB_ONCEUPON_BROKENSTRINGS_H
