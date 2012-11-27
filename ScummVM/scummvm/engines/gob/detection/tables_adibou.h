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

/* Detection tables for Adibou / Addy Junior series. */

#ifndef GOB_DETECTION_TABLES_ADIBOU_H
#define GOB_DETECTION_TABLES_ADIBOU_H

// -- French: Adibou --

{
	{
		"adibou1",
		"ADIBOU 1 Environnement 4-7 ans",
		AD_ENTRY1s("intro.stk", "6db110188fcb7c5208d9721b5282682a", 4805104),
		FR_FRA,
		kPlatformPC,
		ADGF_NO_FLAGS,
		GUIO2(GUIO_NOSUBTITLES, GUIO_NOSPEECH)
	},
	kGameTypeAdibou1,
	kFeaturesAdLib,
	0, 0, 0
},
{
	{
		"adibou2",
		"ADIBOU 2",
		AD_ENTRY1s("intro.stk", "94ae7004348dc8bf99c23a9a6ef81827", 956162),
		FR_FRA,
		kPlatformPC,
		ADGF_NO_FLAGS,
		GUIO0()
	},
	kGameTypeAdibou2,
	kFeaturesNone,
	0, 0, 0
},
{
	{
		"adibou2",
		"Le Jardin Magique d'Adibou",
		AD_ENTRY1s("intro.stk", "a8ff86f3cc40dfe5898e0a741217ef27", 956328),
		FR_FRA,
		kPlatformPC,
		ADGF_NO_FLAGS,
		GUIO0()
	},
	kGameTypeAdibou2,
	kFeaturesNone,
	0, 0, 0
},
{
	{
		"adibou2",
		"ADIBOU Version Decouverte",
		AD_ENTRY1s("intro.stk", "558c14327b79ed39214b49d567a75e33", 8737856),
		FR_FRA,
		kPlatformPC,
		ADGF_NO_FLAGS,
		GUIO0()
	},
	kGameTypeAdibou2,
	kFeaturesNone,
	0, 0, 0
},
{
	{
		"adibou2",
		"ADIBOU 2.10 Environnement",
		AD_ENTRY1s("intro.stk", "f2b797819aeedee557e904b0b5ccd82e", 8736454),
		FR_FRA,
		kPlatformPC,
		ADGF_NO_FLAGS,
		GUIO0()
	},
	kGameTypeAdibou2,
	kFeaturesNone,
	0, 0, 0
},
{
	{
		"adibou2",
		"ADIBOU 2.11 Environnement",
		AD_ENTRY1s("intro.stk", "7b1f1f6f6477f54401e95d913f75e333", 8736904),
		FR_FRA,
		kPlatformPC,
		ADGF_NO_FLAGS,
		GUIO0()
	},
	kGameTypeAdibou2,
	kFeaturesNone,
	0, 0, 0
},
{
	{
		"adibou2",
		"ADIBOU 2.12 Environnement",
		AD_ENTRY1s("intro.stk", "1e49c39a4a3ce6032a84b712539c2d63", 8738134),
		FR_FRA,
		kPlatformPC,
		ADGF_NO_FLAGS,
		GUIO0()
	},
	kGameTypeAdibou2,
	kFeaturesNone,
	0, 0, 0
},
{
	{
		"adibou2",
		"ADIBOU 2.13s Environnement",
		AD_ENTRY1s("intro.stk", "092707829555f27706920e4cacf1fada", 8737958),
		FR_FRA,
		kPlatformPC,
		ADGF_NO_FLAGS,
		GUIO0()
	},
	kGameTypeAdibou2,
	kFeaturesNone,
	0, 0, 0
},
{
	{
		"adibou2",
		"ADIBOO 2.14 Environnement",
		AD_ENTRY1s("intro.stk", "ff63637e3cb7f0a457edf79457b1c6b3", 9333874),
		FR_FRA,
		kPlatformPC,
		ADGF_NO_FLAGS,
		GUIO0()
	},
	kGameTypeAdibou2,
	kFeaturesNone,
	0, 0, 0
},

// -- German: Addy Junior --

{
	{
		"adibou2",
		"ADIBOU 2",
		AD_ENTRY1s("intro.stk", "092707829555f27706920e4cacf1fada", 8737958),
		DE_DEU,
		kPlatformPC,
		ADGF_NO_FLAGS,
		GUIO0()
	},
	kGameTypeAdibou2,
	kFeaturesNone,
	0, 0, 0
},

// -- Italian: Adibù --
{
	{
		"adibou2",
		"ADIB\xD9 2",
		AD_ENTRY1s("intro.stk", "092707829555f27706920e4cacf1fada", 8737958),
		IT_ITA,
		kPlatformPC,
		ADGF_NO_FLAGS,
		GUIO0()
	},
	kGameTypeAdibou2,
	kFeaturesNone,
	0, 0, 0
},

// -- Demos --

{
	{
		"adibou2",
		"Non-Interactive Demo",
		{
			{"demogb.scn", 0, "9291455a908ac0e6aaaca686e532609b", 105},
			{"demogb.vmd", 0, "bc9c1db97db7bec8f566332444fa0090", 14320840},
			{0, 0, 0, 0}
		},
		EN_GRB,
		kPlatformPC,
		ADGF_DEMO,
		GUIO3(GUIO_NOSUBTITLES, GUIO_NOSPEECH, GUIO_NOASPECT)
	},
	kGameTypeAdibou2,
	kFeatures640x480 | kFeaturesSCNDemo,
	0, 0, 9
},
{
	{
		"adibou2",
		"Non-Interactive Demo",
		{
			{"demoall.scn", 0, "c8fd308c037b829800006332b2c32674", 106},
			{"demoall.vmd", 0, "4672b2deacc6fca97484840424b1921b", 14263433},
			{0, 0, 0, 0}
		},
		DE_DEU,
		kPlatformPC,
		ADGF_DEMO,
		GUIO3(GUIO_NOSUBTITLES, GUIO_NOSPEECH, GUIO_NOASPECT)
	},
	kGameTypeAdibou2,
	kFeatures640x480 | kFeaturesSCNDemo,
	0, 0, 10
},
{
	{
		"adibou2",
		"Non-Interactive Demo",
		{
			{"demofra.scn", 0, "d1b2b1618af384ea1120def8b986c02b", 106},
			{"demofra.vmd", 0, "b494cdec1aac7e54c3f2480512d2880e", 14297100},
			{0, 0, 0, 0}
		},
		FR_FRA,
		kPlatformPC,
		ADGF_DEMO,
		GUIO3(GUIO_NOSUBTITLES, GUIO_NOSPEECH, GUIO_NOASPECT)
	},
	kGameTypeAdibou2,
	kFeatures640x480 | kFeaturesSCNDemo,
	0, 0, 11
},

#endif // GOB_DETECTION_TABLES_ADIBOU_H
