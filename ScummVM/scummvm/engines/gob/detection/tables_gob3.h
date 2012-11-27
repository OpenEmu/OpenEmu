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

/* Detection tables for Goblins 3 / Goblins Quest 3. */

#ifndef GOB_DETECTION_TABLES_GOB3_H
#define GOB_DETECTION_TABLES_GOB3_H

// -- DOS VGA Floppy --

{
	{
		"gob3",
		"",
		AD_ENTRY1s("intro.stk", "32b0f57f5ae79a9ae97e8011df38af42", 157084),
		EN_GRB,
		kPlatformPC,
		ADGF_NO_FLAGS,
		GUIO2(GUIO_NOSUBTITLES, GUIO_NOSPEECH)
	},
	kGameTypeGob3,
	kFeaturesAdLib,
	0, 0, 0
},
{
	{
		"gob3",
		"",
		AD_ENTRY1s("intro.stk", "904fc32032295baa3efb3a41f17db611", 178582),
		HE_ISR,
		kPlatformPC,
		ADGF_NO_FLAGS,
		GUIO2(GUIO_NOSUBTITLES, GUIO_NOSPEECH)
	},
	kGameTypeGob3,
	kFeaturesAdLib,
	0, 0, 0
},
{ // Supplied by raziel_ in bug report #1891869
	{
		"gob3",
		"",
		AD_ENTRY1s("intro.stk", "16b014bf32dbd6ab4c5163c44f56fed1", 445104),
		EN_GRB,
		kPlatformPC,
		ADGF_NO_FLAGS,
		GUIO2(GUIO_NOSUBTITLES, GUIO_NOSPEECH)
	},
	kGameTypeGob3,
	kFeaturesAdLib,
	0, 0, 0
},
{
	{
		"gob3",
		"",
		AD_ENTRY1("intro.stk", "1e2f64ec8dfa89f42ee49936a27e66e7"),
		EN_USA,
		kPlatformPC,
		ADGF_NO_FLAGS,
		GUIO2(GUIO_NOSUBTITLES, GUIO_NOSPEECH)
	},
	kGameTypeGob3,
	kFeaturesAdLib,
	0, 0, 0
},
{ // Supplied by paul66 in bug report #1652352
	{
		"gob3",
		"",
		AD_ENTRY1("intro.stk", "f6d225b25a180606fa5dbe6405c97380"),
		DE_DEU,
		kPlatformPC,
		ADGF_NO_FLAGS,
		GUIO2(GUIO_NOSUBTITLES, GUIO_NOSPEECH)
	},
	kGameTypeGob3,
	kFeaturesAdLib,
	0, 0, 0
},
{
	{
		"gob3",
		"",
		AD_ENTRY1("intro.stk", "e42a4f2337d6549487a80864d7826972"),
		FR_FRA,
		kPlatformPC,
		ADGF_NO_FLAGS,
		GUIO2(GUIO_NOSUBTITLES, GUIO_NOSPEECH)
	},
	kGameTypeGob3,
	kFeaturesAdLib,
	0, 0, 0
},
{ // Supplied by Paranoimia on #scummvm
	{
		"gob3",
		"",
		AD_ENTRY1s("intro.stk", "fe8144daece35538085adb59c2d29613", 159402),
		IT_ITA,
		kPlatformPC,
		ADGF_NO_FLAGS,
		GUIO2(GUIO_NOSUBTITLES, GUIO_NOSPEECH)
	},
	kGameTypeGob3,
	kFeaturesAdLib,
	0, 0, 0
},
{
	{
		"gob3",
		"",
		AD_ENTRY1("intro.stk", "4e3af248a48a2321364736afab868527"),
		RU_RUS,
		kPlatformPC,
		ADGF_NO_FLAGS,
		GUIO2(GUIO_NOSUBTITLES, GUIO_NOSPEECH)
	},
	kGameTypeGob3,
	kFeaturesAdLib,
	0, 0, 0
},
{
	{
		"gob3",
		"",
		AD_ENTRY1("intro.stk", "8d28ce1591b0e9cc79bf41cad0fc4c9c"),
		UNK_LANG,
		kPlatformPC,
		ADGF_NO_FLAGS,
		GUIO2(GUIO_NOSUBTITLES, GUIO_NOSPEECH)
	},
	kGameTypeGob3,
	kFeaturesAdLib,
	0, 0, 0
},
{ // Supplied by SiRoCs in bug report #2098621
	{
		"gob3",
		"",
		AD_ENTRY1s("intro.stk", "d3b72938fbbc8159198088811f9e6d19", 160382),
		ES_ESP,
		kPlatformPC,
		ADGF_NO_FLAGS,
		GUIO2(GUIO_NOSUBTITLES, GUIO_NOSPEECH)
	},
	kGameTypeGob3,
	kFeaturesAdLib,
	0, 0, 0
},

// -- Windows --

{
	{
		"gob3",
		"",
		{
			{"intro.stk", 0, "16b014bf32dbd6ab4c5163c44f56fed1", 445104},
			{"musmac1.mid", 0, "948c546cad3a9de5bff3fe4107c82bf1", 6404},
			{0, 0, 0, 0}
		},
		DE_DEU,
		kPlatformWindows,
		ADGF_NO_FLAGS,
		GUIO2(GUIO_NOSUBTITLES, GUIO_NOSPEECH)
	},
	kGameTypeGob3,
	kFeaturesAdLib,
	0, 0, 0
},
{
	{
		"gob3",
		"",
		{
			{"intro.stk", 0, "16b014bf32dbd6ab4c5163c44f56fed1", 445104},
			{"musmac1.mid", 0, "948c546cad3a9de5bff3fe4107c82bf1", 6404},
			{0, 0, 0, 0}
		},
		FR_FRA,
		kPlatformWindows,
		ADGF_NO_FLAGS,
		GUIO2(GUIO_NOSUBTITLES, GUIO_NOSPEECH)
	},
	kGameTypeGob3,
	kFeaturesAdLib,
	0, 0, 0
},
{
	{
		"gob3",
		"",
		{
			{"intro.stk", 0, "16b014bf32dbd6ab4c5163c44f56fed1", 445104},
			{"musmac1.mid", 0, "948c546cad3a9de5bff3fe4107c82bf1", 6404},
			{0, 0, 0, 0}
		},
		EN_GRB,
		kPlatformWindows,
		ADGF_NO_FLAGS,
		GUIO2(GUIO_NOSUBTITLES, GUIO_NOSPEECH)
	},
	kGameTypeGob3,
	kFeaturesAdLib,
	0, 0, 0
},
{
	{
		"gob3",
		"",
		{
			{"intro.stk", 0, "edd7403e5dc2a14459d2665a4c17714d", 209534},
			{"musmac1.mid", 0, "948c546cad3a9de5bff3fe4107c82bf1", 6404},
			{0, 0, 0, 0}
		},
		FR_FRA,
		kPlatformWindows,
		ADGF_NO_FLAGS,
		GUIO2(GUIO_NOSUBTITLES, GUIO_NOSPEECH)
	},
	kGameTypeGob3,
	kFeaturesAdLib,
	0, 0, 0
},
{
	{
		"gob3",
		"",
		{
			{"intro.stk", 0, "428e2de130cf3b303c938924539dc50d", 324420},
			{"musmac1.mid", 0, "948c546cad3a9de5bff3fe4107c82bf1", 6404},
			{0, 0, 0, 0}
		},
		FR_FRA,
		kPlatformWindows,
		ADGF_NO_FLAGS,
		GUIO2(GUIO_NOSUBTITLES, GUIO_NOSPEECH)
	},
	kGameTypeGob3,
	kFeaturesAdLib,
	0, 0, 0
},
{
	{
		"gob3",
		"",
		{
			{"intro.stk", 0, "428e2de130cf3b303c938924539dc50d", 324420},
			{"musmac1.mid", 0, "948c546cad3a9de5bff3fe4107c82bf1", 6404},
			{0, 0, 0, 0}
		},
		EN_ANY,
		kPlatformWindows,
		ADGF_NO_FLAGS,
		GUIO2(GUIO_NOSUBTITLES, GUIO_NOSPEECH)
	},
	kGameTypeGob3,
	kFeaturesAdLib,
	0, 0, 0
},
{ // Found in Found in french ADI 2.5 Anglais Multimedia 5e
	{
		"gob3",
		"",
		AD_ENTRY1s("intro.stk", "edd7403e5dc2a14459d2665a4c17714d", 209534),
		FR_FRA,
		kPlatformWindows,
		ADGF_NO_FLAGS,
		GUIO2(GUIO_NOSUBTITLES, GUIO_NOSPEECH)
	},
	kGameTypeGob3,
	kFeaturesAdLib,
	0, 0, 0
},

// -- Mac --

{ // Supplied by fac76 in bug report #1742716
	{
		"gob3",
		"",
		{
			{"intro.stk", 0, "32b0f57f5ae79a9ae97e8011df38af42", 157084},
			{"musmac1.mid", 0, "834e55205b710d0af5f14a6f2320dd8e", 8661},
			{0, 0, 0, 0}
		},
		EN_GRB,
		kPlatformMacintosh,
		ADGF_NO_FLAGS,
		GUIO2(GUIO_NOSUBTITLES, GUIO_NOSPEECH)
	},
	kGameTypeGob3,
	kFeaturesAdLib,
	0, 0, 0
},

// -- Amiga --

{
	{
		"gob3",
		"",
		AD_ENTRY1("intro.stk", "bd679eafde2084d8011f247e51b5a805"),
		EN_GRB,
		kPlatformAmiga,
		ADGF_NO_FLAGS,
		GUIO2(GUIO_NOSUBTITLES, GUIO_NOSPEECH)
	},
	kGameTypeGob3,
	kFeaturesNone,
	0, "menu.tot", 0
},
{
	{
		"gob3",
		"",
		AD_ENTRY1("intro.stk", "bd679eafde2084d8011f247e51b5a805"),
		DE_DEU,
		kPlatformAmiga,
		ADGF_NO_FLAGS,
		GUIO2(GUIO_NOSUBTITLES, GUIO_NOSPEECH)
	},
	kGameTypeGob3,
	kFeaturesNone,
	0, "menu.tot", 0
},

// -- DOS VGA CD --

{
	{
		"gob3cd",
		"v1.000",
		AD_ENTRY1("intro.stk", "6f2c226c62dd7ab0ab6f850e89d3fc47"),
		EN_USA,
		kPlatformPC,
		ADGF_NO_FLAGS,
		GUIO2(GUIO_NOSUBTITLES, GUIO_NOSPEECH)
	},
	kGameTypeGob3,
	kFeaturesCD,
	0, 0, 0
},
{ // Supplied by pykman in bug report #3067489
	{
		"gob3cd",
		"v1.02 Polish",
		AD_ENTRY1s("intro.stk", "978afddcac81bb95a04757b61f78471c", 619825),
		UNK_LANG,
		kPlatformPC,
		ADGF_NO_FLAGS,
		GUIO2(GUIO_NOSUBTITLES, GUIO_NOSPEECH)
	},
	kGameTypeGob3,
	kFeaturesCD,
	0, 0, 0
},
{ // Supplied by paul66 and noizert in bug reports #1652352 and #1691230
	{
		"gob3cd",
		"v1.02",
		AD_ENTRY1("intro.stk", "c3e9132ea9dc0fb866b6d60dcda10261"),
		EN_ANY,
		kPlatformPC,
		ADGF_NO_FLAGS,
		GUIO2(GUIO_NOSUBTITLES, GUIO_NOSPEECH)
	},
	kGameTypeGob3,
	kFeaturesCD,
	0, 0, 0
},
{ // Supplied by paul66 and noizert in bug reports #1652352 and #1691230
	{
		"gob3cd",
		"v1.02",
		AD_ENTRY1("intro.stk", "c3e9132ea9dc0fb866b6d60dcda10261"),
		DE_DEU,
		kPlatformPC,
		ADGF_NO_FLAGS,
		GUIO2(GUIO_NOSUBTITLES, GUIO_NOSPEECH)
	},
	kGameTypeGob3,
	kFeaturesCD,
	0, 0, 0
},
{ // Supplied by paul66 and noizert in bug reports #1652352 and #1691230
	{
		"gob3cd",
		"v1.02",
		AD_ENTRY1("intro.stk", "c3e9132ea9dc0fb866b6d60dcda10261"),
		FR_FRA,
		kPlatformPC,
		ADGF_NO_FLAGS,
		GUIO2(GUIO_NOSUBTITLES, GUIO_NOSPEECH)
	},
	kGameTypeGob3,
	kFeaturesCD,
	0, 0, 0
},
{ // Supplied by paul66 and noizert in bug reports #1652352 and #1691230
	{
		"gob3cd",
		"v1.02",
		AD_ENTRY1("intro.stk", "c3e9132ea9dc0fb866b6d60dcda10261"),
		IT_ITA,
		kPlatformPC,
		ADGF_NO_FLAGS,
		GUIO2(GUIO_NOSUBTITLES, GUIO_NOSPEECH)
	},
	kGameTypeGob3,
	kFeaturesCD,
	0, 0, 0
},
{ // Supplied by paul66 and noizert in bug reports #1652352 and #1691230
	{
		"gob3cd",
		"v1.02",
		AD_ENTRY1("intro.stk", "c3e9132ea9dc0fb866b6d60dcda10261"),
		ES_ESP,
		kPlatformPC,
		ADGF_NO_FLAGS,
		GUIO2(GUIO_NOSUBTITLES, GUIO_NOSPEECH)
	},
	kGameTypeGob3,
	kFeaturesCD,
	0, 0, 0
},
{ // Supplied by goodoldgeorg in bug report #2810082
	{
		"gob3cd",
		"v1.02",
		AD_ENTRY1s("intro.stk", "bfd7d4c6fedeb2cfcc8baa4d5ddb1f74", 616220),
		HU_HUN,
		kPlatformPC,
		ADGF_NO_FLAGS,
		GUIO2(GUIO_NOSUBTITLES, GUIO_NOSPEECH)
	},
	kGameTypeGob3,
	kFeaturesCD,
	0, 0, 0
},
{ // Supplied by goodoldgeorg in bug report #2810082
	{
		"gob3cd",
		"v1.02",
		AD_ENTRY1s("intro.stk", "bfd7d4c6fedeb2cfcc8baa4d5ddb1f74", 616220),
		FR_FRA,
		kPlatformPC,
		ADGF_NO_FLAGS,
		GUIO2(GUIO_NOSUBTITLES, GUIO_NOSPEECH)
	},
	kGameTypeGob3,
	kFeaturesCD,
	0, 0, 0
},
{ // Supplied by goodoldgeorg in bug report #2810082
	{
		"gob3cd",
		"v1.02",
		AD_ENTRY1s("intro.stk", "bfd7d4c6fedeb2cfcc8baa4d5ddb1f74", 616220),
		DE_DEU,
		kPlatformPC,
		ADGF_NO_FLAGS,
		GUIO2(GUIO_NOSUBTITLES, GUIO_NOSPEECH)
	},
	kGameTypeGob3,
	kFeaturesCD,
	0, 0, 0
},
{ // Supplied by goodoldgeorg in bug report #2810082
	{
		"gob3cd",
		"v1.02",
		AD_ENTRY1s("intro.stk", "bfd7d4c6fedeb2cfcc8baa4d5ddb1f74", 616220),
		ES_ESP,
		kPlatformPC,
		ADGF_NO_FLAGS,
		GUIO2(GUIO_NOSUBTITLES, GUIO_NOSPEECH)
	},
	kGameTypeGob3,
	kFeaturesCD,
	0, 0, 0
},

// -- Demos --

{
	{
		"gob3",
		"Non-interactive Demo",
		AD_ENTRY1("intro.stk", "b9b898fccebe02b69c086052d5024a55"),
		UNK_LANG,
		kPlatformPC,
		ADGF_DEMO,
		GUIO2(GUIO_NOSUBTITLES, GUIO_NOSPEECH)
	},
	kGameTypeGob3,
	kFeaturesAdLib,
	0, 0, 0
},
{
	{
		"gob3",
		"Interactive Demo",
		AD_ENTRY1("intro.stk", "7aebd94e49c2c5c518c9e7b74f25de9d"),
		FR_FRA,
		kPlatformPC,
		ADGF_DEMO,
		GUIO2(GUIO_NOSUBTITLES, GUIO_NOSPEECH)
	},
	kGameTypeGob3,
	kFeaturesAdLib,
	0, 0, 0
},
{
	{
		"gob3",
		"Interactive Demo 2",
		AD_ENTRY1("intro.stk", "e5dcbc9f6658ebb1e8fe26bc4da0806d"),
		FR_FRA,
		kPlatformPC,
		ADGF_DEMO,
		GUIO2(GUIO_NOSUBTITLES, GUIO_NOSPEECH)
	},
	kGameTypeGob3,
	kFeaturesAdLib,
	0, 0, 0
},
{
	{
		"gob3",
		"Interactive Demo 3",
		AD_ENTRY1s("intro.stk", "9e20ad7b471b01f84db526da34eaf0a2", 395561),
		EN_ANY,
		kPlatformPC,
		ADGF_DEMO,
		GUIO2(GUIO_NOSUBTITLES, GUIO_NOSPEECH)
	},
	kGameTypeGob3,
	kFeaturesAdLib,
	0, 0, 0
},

#endif // GOB_DETECTION_TABLES_GOB3_H
