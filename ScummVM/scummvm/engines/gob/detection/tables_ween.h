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

/* Detection tables for Ween: The Prophecy. */

#ifndef GOB_DETECTION_TABLES_WEEN_H
#define GOB_DETECTION_TABLES_WEEN_H

// -- DOS VGA Floppy --

{
	{
		"ween",
		"",
		AD_ENTRY1("intro.stk", "2bb8878a8042244dd2b96ff682381baa"),
		EN_GRB,
		kPlatformPC,
		ADGF_NO_FLAGS,
		GUIO2(GUIO_NOSUBTITLES, GUIO_NOSPEECH)
	},
	kGameTypeWeen,
	kFeaturesAdLib,
	0, 0, 0
},
{
	{
		"ween",
		"",
		AD_ENTRY1s("intro.stk", "de92e5c6a8c163007ffceebef6e67f7d", 7117568),
		EN_USA,
		kPlatformPC,
		ADGF_NO_FLAGS,
		GUIO2(GUIO_NOSUBTITLES, GUIO_NOSPEECH)
	},
	kGameTypeWeen,
	kFeaturesAdLib,
	0, 0, 0
},
{ // Supplied by cybot_tmin in bug report #1667743
	{
		"ween",
		"",
		AD_ENTRY1s("intro.stk", "6d60f9205ecfbd8735da2ee7823a70dc", 7014426),
		ES_ESP,
		kPlatformPC,
		ADGF_NO_FLAGS,
		GUIO2(GUIO_NOSUBTITLES, GUIO_NOSPEECH)
	},
	kGameTypeWeen,
	kFeaturesAdLib,
	0, 0, 0
},
{
	{
		"ween",
		"",
		AD_ENTRY1("intro.stk", "4b10525a3782aa7ecd9d833b5c1d308b"),
		FR_FRA,
		kPlatformPC,
		ADGF_NO_FLAGS,
		GUIO2(GUIO_NOSUBTITLES, GUIO_NOSPEECH)
	},
	kGameTypeWeen,
	kFeaturesAdLib,
	0, 0, 0
},
{ // Supplied by cartman_ on #scummvm
	{
		"ween",
		"",
		AD_ENTRY1("intro.stk", "63170e71f04faba88673b3f510f9c4c8"),
		DE_DEU,
		kPlatformPC,
		ADGF_NO_FLAGS,
		GUIO2(GUIO_NOSUBTITLES, GUIO_NOSPEECH)
	},
	kGameTypeWeen,
	kFeaturesAdLib,
	0, 0, 0
},
{ // Supplied by glorfindel in bugreport #1722142
	{
		"ween",
		"",
		AD_ENTRY1s("intro.stk", "8b57cd510da8a3bbd99e3a0297a8ebd1", 7018771),
		IT_ITA,
		kPlatformPC,
		ADGF_NO_FLAGS,
		GUIO2(GUIO_NOSUBTITLES, GUIO_NOSPEECH)
	},
	kGameTypeWeen,
	kFeaturesAdLib,
	0, 0, 0
},

// -- Amiga --

{ // Supplied by vampir_raziel in bug report #1658373
	{
		"ween",
		"",
		{
			{"intro.stk", 0, "bfd9d02faf3d8d60a2cf744f95eb48dd", 456570},
			{"ween.ins", 0, "d2cb24292c9ddafcad07e23382027218", 87800},
			{0, 0, 0, 0}
		},
		EN_GRB,
		kPlatformAmiga,
		ADGF_NO_FLAGS,
		GUIO2(GUIO_NOSUBTITLES, GUIO_NOSPEECH)
	},
	kGameTypeWeen,
	kFeaturesNone,
	0, 0, 0
},
{ // Supplied by vampir_raziel in bug report #1658373
	{
		"ween",
		"",
		AD_ENTRY1s("intro.stk", "257fe669705ac4971efdfd5656eef16a", 457719),
		FR_FRA,
		kPlatformAmiga,
		ADGF_NO_FLAGS,
		GUIO2(GUIO_NOSUBTITLES, GUIO_NOSPEECH)
	},
	kGameTypeWeen,
	kFeaturesNone,
	0, 0, 0
},
{ // Supplied by vampir_raziel in bug report #1658373
	{
		"ween",
		"",
		AD_ENTRY1s("intro.stk", "dffd1ab98fe76150d6933329ca6f4cc4", 459458),
		FR_FRA,
		kPlatformAmiga,
		ADGF_NO_FLAGS,
		GUIO2(GUIO_NOSUBTITLES, GUIO_NOSPEECH)
	},
	kGameTypeWeen,
	kFeaturesNone,
	0, 0, 0
},
{ // Supplied by vampir_raziel in bug report #1658373
	{
		"ween",
		"",
		AD_ENTRY1s("intro.stk", "af83debf2cbea21faa591c7b4608fe92", 458192),
		DE_DEU,
		kPlatformAmiga,
		ADGF_NO_FLAGS,
		GUIO2(GUIO_NOSUBTITLES, GUIO_NOSPEECH)
	},
	kGameTypeWeen,
	kFeaturesNone,
	0, 0, 0
},
{ // Supplied by goodoldgeorg in bug report #2563539
	{
		"ween",
		"",
		{
			{"intro.stk", 0, "dffd1ab98fe76150d6933329ca6f4cc4", 459458},
			{"ween.ins", 0, "d2cb24292c9ddafcad07e23382027218", 87800},
			{0, 0, 0, 0}
		},
		IT_ITA,
		kPlatformAmiga,
		ADGF_NO_FLAGS,
		GUIO2(GUIO_NOSUBTITLES, GUIO_NOSPEECH)
	},
	kGameTypeWeen,
	kFeaturesNone,
	0, 0, 0
},

// -- Atari ST --

{ // Supplied by pwigren in bug report #1764174
	{
		"ween",
		"",
		{
			{"intro.stk", 0, "bfd9d02faf3d8d60a2cf744f95eb48dd", 456570},
			{"music__5.snd", 0, "7d1819b9981ecddd53d3aacbc75f1cc8", 13446},
			{0, 0, 0, 0}
		},
		EN_GRB,
		kPlatformAtariST,
		ADGF_NO_FLAGS,
		GUIO2(GUIO_NOSUBTITLES, GUIO_NOSPEECH)
	},
	kGameTypeWeen,
	kFeaturesNone,
	0, 0, 0
},
{
	{
		"ween",
		"",
		AD_ENTRY1("intro.stk", "e6d13fb3b858cb4f78a8780d184d5b2c"),
		FR_FRA,
		kPlatformAtariST,
		ADGF_NO_FLAGS,
		GUIO2(GUIO_NOSUBTITLES, GUIO_NOSPEECH)
	},
	kGameTypeWeen,
	kFeaturesNone,
	0, 0, 0
},

// -- DOS VGA Floppy --

{
	{
		"ween",
		"",
		AD_ENTRY1("intro.stk", "2bb8878a8042244dd2b96ff682381baa"),
		EN_GRB,
		kPlatformPC,
		ADGF_NO_FLAGS,
		GUIO2(GUIO_NOSUBTITLES, GUIO_NOSPEECH)
	},
	kGameTypeWeen,
	kFeaturesAdLib,
	0, 0, 0
},
{
	{
		"ween",
		"",
		AD_ENTRY1s("intro.stk", "de92e5c6a8c163007ffceebef6e67f7d", 7117568),
		EN_USA,
		kPlatformPC,
		ADGF_NO_FLAGS,
		GUIO2(GUIO_NOSUBTITLES, GUIO_NOSPEECH)
	},
	kGameTypeWeen,
	kFeaturesAdLib,
	0, 0, 0
},
{ // Supplied by cybot_tmin in bug report #1667743
	{
		"ween",
		"",
		AD_ENTRY1s("intro.stk", "6d60f9205ecfbd8735da2ee7823a70dc", 7014426),
		ES_ESP,
		kPlatformPC,
		ADGF_NO_FLAGS,
		GUIO2(GUIO_NOSUBTITLES, GUIO_NOSPEECH)
	},
	kGameTypeWeen,
	kFeaturesAdLib,
	0, 0, 0
},
{
	{
		"ween",
		"",
		AD_ENTRY1("intro.stk", "4b10525a3782aa7ecd9d833b5c1d308b"),
		FR_FRA,
		kPlatformPC,
		ADGF_NO_FLAGS,
		GUIO2(GUIO_NOSUBTITLES, GUIO_NOSPEECH)
	},
	kGameTypeWeen,
	kFeaturesAdLib,
	0, 0, 0
},
{ // Supplied by cartman_ on #scummvm
	{
		"ween",
		"",
		AD_ENTRY1("intro.stk", "63170e71f04faba88673b3f510f9c4c8"),
		DE_DEU,
		kPlatformPC,
		ADGF_NO_FLAGS,
		GUIO2(GUIO_NOSUBTITLES, GUIO_NOSPEECH)
	},
	kGameTypeWeen,
	kFeaturesAdLib,
	0, 0, 0
},
{ // Supplied by glorfindel in bugreport #1722142
	{
		"ween",
		"",
		AD_ENTRY1s("intro.stk", "8b57cd510da8a3bbd99e3a0297a8ebd1", 7018771),
		IT_ITA,
		kPlatformPC,
		ADGF_NO_FLAGS,
		GUIO2(GUIO_NOSUBTITLES, GUIO_NOSPEECH)
	},
	kGameTypeWeen,
	kFeaturesAdLib,
	0, 0, 0
},

// -- Demos --

{
	{
		"ween",
		"Demo",
		AD_ENTRY1("intro.stk", "2e9c2898f6bf206ede801e3b2e7ee428"),
		UNK_LANG,
		kPlatformPC,
		ADGF_DEMO,
		GUIO2(GUIO_NOSUBTITLES, GUIO_NOSPEECH)
	},
	kGameTypeWeen,
	kFeaturesAdLib,
	0, "show.tot", 0
},
{
	{
		"ween",
		"Demo",
		AD_ENTRY1("intro.stk", "15fb91a1b9b09684b28ac75edf66e504"),
		EN_USA,
		kPlatformPC,
		ADGF_DEMO,
		GUIO2(GUIO_NOSUBTITLES, GUIO_NOSPEECH)
	},
	kGameTypeWeen,
	kFeaturesAdLib,
	0, "show.tot", 0
},

#endif // GOB_DETECTION_TABLES_WEEN_H
