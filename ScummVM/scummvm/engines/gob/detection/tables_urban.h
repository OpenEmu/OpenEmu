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

/* Detection tables for Urban Runner. */

#ifndef GOB_DETECTION_TABLES_URBAN_H
#define GOB_DETECTION_TABLES_URBAN_H

// -- Windows --

{
	{
		"urban",
		"",
		AD_ENTRY1s("intro.stk", "3ab2c542bd9216ae5d02cc6f45701ae1", 1252436),
		EN_USA,
		kPlatformPC,
		ADGF_NO_FLAGS,
		GUIO3(GUIO_NOSUBTITLES, GUIO_NOSPEECH, GUIO_NOASPECT)
	},
	kGameTypeUrban,
	kFeatures640x480 | kFeaturesTrueColor,
	0, 0, 0
},
{ // Supplied by Collector9 in bug report #3228040
	{
		"urban",
		"",
		AD_ENTRY1s("intro.stk", "6ce3d878178932053267237ec4843ce1", 1252518),
		EN_USA,
		kPlatformPC,
		ADGF_NO_FLAGS,
		GUIO3(GUIO_NOSUBTITLES, GUIO_NOSPEECH, GUIO_NOASPECT)
	},
	kGameTypeUrban,
	kFeatures640x480 | kFeaturesTrueColor,
	0, 0, 0
},
{ // Supplied by gamin in the forums
	{
		"urban",
		"",
		AD_ENTRY1s("intro.stk", "b991ed1d31c793e560edefdb349882ef", 1276408),
		FR_FRA,
		kPlatformPC,
		ADGF_NO_FLAGS,
		GUIO3(GUIO_NOSUBTITLES, GUIO_NOSPEECH, GUIO_NOASPECT)
	},
	kGameTypeUrban,
	kFeatures640x480 | kFeaturesTrueColor,
	0, 0, 0
},
{ // Supplied by jvprat on #scummvm
	{
		"urban",
		"",
		AD_ENTRY1s("intro.stk", "4ec3c0864e2b54c5b4ccf9f6ad96528d", 1253328),
		ES_ESP,
		kPlatformPC,
		ADGF_NO_FLAGS,
		GUIO3(GUIO_NOSUBTITLES, GUIO_NOSPEECH, GUIO_NOASPECT)
	},
	kGameTypeUrban,
	kFeatures640x480 | kFeaturesTrueColor,
	0, 0, 0
},
{ // Supplied by Alex on the gobsmacked blog
	{
		"urban",
		"",
		AD_ENTRY1s("intro.stk", "9ea647085a16dd0fb9ecd84cd8778ec9", 1253436),
		IT_ITA,
		kPlatformPC,
		ADGF_NO_FLAGS,
		GUIO3(GUIO_NOSUBTITLES, GUIO_NOSPEECH, GUIO_NOASPECT)
	},
	kGameTypeUrban,
	kFeatures640x480 | kFeaturesTrueColor,
	0, 0, 0
},
{ // Supplied by alex86r in bug report #3297602
	{
		"urban",
		"",
		AD_ENTRY1s("intro.stk", "4e4a3c017fe5475353bf94c455fe3efd", 1253448),
		IT_ITA,
		kPlatformPC,
		ADGF_NO_FLAGS,
		GUIO3(GUIO_NOSUBTITLES, GUIO_NOSPEECH, GUIO_NOASPECT)
	},
	kGameTypeUrban,
	kFeatures640x480 | kFeaturesTrueColor,
	0, 0, 0
},
{ // Supplied by goodoldgeorg in bug report #2770340
	{
		"urban",
		"",
		AD_ENTRY1s("intro.stk", "4bd31979ea3d77a58a358c09000a85ed", 1253018),
		DE_DEU,
		kPlatformPC,
		ADGF_NO_FLAGS,
		GUIO3(GUIO_NOSUBTITLES, GUIO_NOSPEECH, GUIO_NOASPECT)
	},
	kGameTypeUrban,
	kFeatures640x480 | kFeaturesTrueColor,
	0, 0, 0
},

// -- Demos --

{
	{
		"urban",
		"Non-Interactive Demo",
		{
			{"wdemo.s24", 0, "14ac9bd51db7a075d69ddb144904b271", 87},
			{"demo.vmd", 0, "65d04715d871c292518b56dd160b0161", 9091237},
			{"urband.vmd", 0, "60343891868c91854dd5c82766c70ecc", 922461},
			{0, 0, 0, 0}
		},
		EN_ANY,
		kPlatformPC,
		ADGF_NO_FLAGS,
		GUIO1(GUIO_NOASPECT)
	},
	kGameTypeUrban,
	kFeatures640x480 | kFeaturesTrueColor | kFeaturesSCNDemo,
	0, 0, 2
},

#endif // GOB_DETECTION_TABLES_URBAN_H
