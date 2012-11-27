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

/* Detection tables for The Last Dynasty. */

#ifndef GOB_DETECTION_TABLES_DYNASTY_H
#define GOB_DETECTION_TABLES_DYNASTY_H

// -- Windows --

{
	{
		"dynasty",
		"",
		AD_ENTRY1s("intro.stk", "6190e32404b672f4bbbc39cf76f41fda", 2511470),
		EN_USA,
		kPlatformPC,
		ADGF_NO_FLAGS,
		GUIO3(GUIO_NOSUBTITLES, GUIO_NOSPEECH, GUIO_NOASPECT)
	},
	kGameTypeDynasty,
	kFeatures640x480,
	0, 0, 0
},
{
	{
		"dynasty",
		"",
		AD_ENTRY1s("intro.stk", "61e4069c16e27775a6cc6d20f529fb36", 2511300),
		EN_USA,
		kPlatformPC,
		ADGF_NO_FLAGS,
		GUIO3(GUIO_NOSUBTITLES, GUIO_NOSPEECH, GUIO_NOASPECT)
	},
	kGameTypeDynasty,
	kFeatures640x480,
	0, 0, 0
},
{
	{
		"dynasty",
		"",
		AD_ENTRY1s("intro.stk", "61e4069c16e27775a6cc6d20f529fb36", 2511300),
		FR_FRA,
		kPlatformPC,
		ADGF_NO_FLAGS,
		GUIO3(GUIO_NOSUBTITLES, GUIO_NOSPEECH, GUIO_NOASPECT)
	},
	kGameTypeDynasty,
	kFeatures640x480,
	0, 0, 0
},
{
	{
		"dynasty",
		"",
		AD_ENTRY1s("intro.stk", "b3f8472484b7a1df94557b51e7b6fca0", 2322644),
		FR_FRA,
		kPlatformPC,
		ADGF_NO_FLAGS,
		GUIO3(GUIO_NOSUBTITLES, GUIO_NOSPEECH, GUIO_NOASPECT)
	},
	kGameTypeDynasty,
	kFeatures640x480,
	0, 0, 0
},
{
	{
		"dynasty",
		"",
		AD_ENTRY1s("intro.stk", "bdbdac8919200a5e71ffb9fb0709f704", 2446652),
		DE_DEU,
		kPlatformPC,
		ADGF_NO_FLAGS,
		GUIO3(GUIO_NOSUBTITLES, GUIO_NOSPEECH, GUIO_NOASPECT)
	},
	kGameTypeDynasty,
	kFeatures640x480,
	0, 0, 0
},

// -- Demos --

{
	{
		"dynasty",
		"Demo",
		AD_ENTRY1s("intro.stk", "464538a17ed39755d7f1ba9c751af1bd", 1847864),
		EN_USA,
		kPlatformPC,
		ADGF_DEMO,
		GUIO1(GUIO_NOASPECT)
	},
	kGameTypeDynasty,
	kFeatures640x480,
	0, 0, 0
},
{
	{
		"dynasty",
		"Demo",
		AD_ENTRY1s("lda1.stk", "0e56a899357cbc0bf503260fd2dd634e", 15032774),
		UNK_LANG,
		kPlatformWindows,
		ADGF_DEMO,
		GUIO1(GUIO_NOASPECT)
	},
	kGameTypeDynasty,
	kFeatures640x480,
	"lda1.stk", 0, 0
},
{
	{
		"dynasty",
		"Demo",
		AD_ENTRY1s("lda1.stk", "8669ea2e9a8239c070dc73958fbc8753", 15567724),
		DE_DEU,
		kPlatformWindows,
		ADGF_DEMO,
		GUIO1(GUIO_NOASPECT)
	},
	kGameTypeDynasty,
	kFeatures640x480,
	"lda1.stk", 0, 0
},

#endif // GOB_DETECTION_TABLES_DYNASTY_H
