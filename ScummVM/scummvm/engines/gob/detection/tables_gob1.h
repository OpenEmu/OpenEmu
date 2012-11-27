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

/* Detection tables for Gobliiins. */

#ifndef GOB_DETECTION_TABLES_GOB1_H
#define GOB_DETECTION_TABLES_GOB1_H

// -- DOS EGA Floppy --

{ // Supplied by Florian Zeitz on scummvm-devel
	{
		"gob1",
		"EGA",
		AD_ENTRY1("intro.stk", "c65e9cc8ba23a38456242e1f2b1caad4"),
		UNK_LANG,
		kPlatformPC,
		ADGF_NO_FLAGS,
		GUIO2(GUIO_NOSUBTITLES, GUIO_NOSPEECH)
	},
	kGameTypeGob1,
	kFeaturesEGA | kFeaturesAdLib,
	0, 0, 0
},
{
	{
		"gob1",
		"EGA",
		AD_ENTRY1("intro.stk", "f9233283a0be2464248d83e14b95f09c"),
		RU_RUS,
		kPlatformPC,
		ADGF_NO_FLAGS,
		GUIO2(GUIO_NOSUBTITLES, GUIO_NOSPEECH)
	},
	kGameTypeGob1,
	kFeaturesEGA | kFeaturesAdLib,
	0, 0, 0
},

// -- DOS VGA Floppy --

{ // Supplied by Theruler76 in bug report #1201233
	{
		"gob1",
		"VGA",
		AD_ENTRY1("intro.stk", "26a9118c0770fa5ac93a9626761600b2"),
		UNK_LANG,
		kPlatformPC,
		ADGF_NO_FLAGS,
		GUIO2(GUIO_NOSUBTITLES, GUIO_NOSPEECH)
	},
	kGameTypeGob1,
	kFeaturesNone,
	0, 0, 0
},
{ // Supplied by raziel_ in bug report #1891864
	{
		"gob1",
		"VGA",
		AD_ENTRY1s("intro.stk", "e157cb59c6d330ca70d12ab0ef1dd12b", 288972),
		EN_GRB,
		kPlatformPC,
		ADGF_NO_FLAGS,
		GUIO2(GUIO_NOSUBTITLES, GUIO_NOSPEECH)
	},
	kGameTypeGob1,
	kFeaturesAdLib,
	0, 0, 0
},

// -- DOS VGA CD --

{ // Provided by pykman in the forums.
	{
		"gob1cd",
		"Polish",
		AD_ENTRY1s("intro.stk", "97d2443948b2e367cf567fe7e101f5f2", 4049267),
		UNK_LANG,
		kPlatformPC,
		ADGF_NO_FLAGS,
		GUIO2(GUIO_NOSUBTITLES, GUIO_NOSPEECH)
	},
	kGameTypeGob1,
	kFeaturesCD,
	0, 0, 0
},
{ // CD 1.000 version.
	{
		"gob1cd",
		"v1.000",
		AD_ENTRY1("intro.stk", "2fbf4b5b82bbaee87eb45d4404c28998"),
		EN_USA,
		kPlatformPC,
		ADGF_NO_FLAGS,
		GUIO2(GUIO_NOSUBTITLES, GUIO_NOSPEECH)
	},
	kGameTypeGob1,
	kFeaturesCD,
	0, 0, 0
},
{ // CD 1.000 version.
	{
		"gob1cd",
		"v1.000",
		AD_ENTRY1("intro.stk", "2fbf4b5b82bbaee87eb45d4404c28998"),
		DE_DEU,
		kPlatformPC,
		ADGF_NO_FLAGS,
		GUIO2(GUIO_NOSUBTITLES, GUIO_NOSPEECH)
	},
	kGameTypeGob1,
	kFeaturesCD,
	0, 0, 0
},
{ // CD 1.000 version.
	{
		"gob1cd",
		"v1.000",
		AD_ENTRY1("intro.stk", "2fbf4b5b82bbaee87eb45d4404c28998"),
		FR_FRA,
		kPlatformPC,
		ADGF_NO_FLAGS,
		GUIO2(GUIO_NOSUBTITLES, GUIO_NOSPEECH)
	},
	kGameTypeGob1,
	kFeaturesCD,
	0, 0, 0
},
{ // CD 1.000 version.
	{
		"gob1cd",
		"v1.000",
		AD_ENTRY1("intro.stk", "2fbf4b5b82bbaee87eb45d4404c28998"),
		IT_ITA,
		kPlatformPC,
		ADGF_NO_FLAGS,
		GUIO2(GUIO_NOSUBTITLES, GUIO_NOSPEECH)
	},
	kGameTypeGob1,
	kFeaturesCD,
	0, 0, 0
},
{ // CD 1.000 version.
	{
		"gob1cd",
		"v1.000",
		AD_ENTRY1("intro.stk", "2fbf4b5b82bbaee87eb45d4404c28998"),
		ES_ESP,
		kPlatformPC,
		ADGF_NO_FLAGS,
		GUIO2(GUIO_NOSUBTITLES, GUIO_NOSPEECH)
	},
	kGameTypeGob1,
	kFeaturesCD,
	0, 0, 0
},
{ // CD 1.02 version. Multilingual
	{
		"gob1cd",
		"v1.02",
		AD_ENTRY1("intro.stk", "8bd873137b6831c896ee8ad217a6a398"),
		EN_USA,
		kPlatformPC,
		ADGF_NO_FLAGS,
		GUIO2(GUIO_NOSUBTITLES, GUIO_NOSPEECH)
	},
	kGameTypeGob1,
	kFeaturesCD,
	0, 0, 0
},
{ // CD 1.02 version. Multilingual
	{
		"gob1cd",
		"v1.02",
		AD_ENTRY1("intro.stk", "8bd873137b6831c896ee8ad217a6a398"),
		DE_DEU,
		kPlatformPC,
		ADGF_NO_FLAGS,
		GUIO2(GUIO_NOSUBTITLES, GUIO_NOSPEECH)
	},
	kGameTypeGob1,
	kFeaturesCD,
	0, 0, 0
},
{ // CD 1.02 version. Multilingual
	{
		"gob1cd",
		"v1.02",
		AD_ENTRY1("intro.stk", "8bd873137b6831c896ee8ad217a6a398"),
		FR_FRA,
		kPlatformPC,
		ADGF_NO_FLAGS,
		GUIO2(GUIO_NOSUBTITLES, GUIO_NOSPEECH)
	},
	kGameTypeGob1,
	kFeaturesCD,
	0, 0, 0
},
{ // CD 1.02 version. Multilingual
	{
		"gob1cd",
		"v1.02",
		AD_ENTRY1("intro.stk", "8bd873137b6831c896ee8ad217a6a398"),
		IT_ITA,
		kPlatformPC,
		ADGF_NO_FLAGS,
		GUIO2(GUIO_NOSUBTITLES, GUIO_NOSPEECH)
	},
	kGameTypeGob1,
	kFeaturesCD,
	0, 0, 0
},
{ // CD 1.02 version. Multilingual
	{
		"gob1cd",
		"v1.02",
		AD_ENTRY1("intro.stk", "8bd873137b6831c896ee8ad217a6a398"),
		ES_ESP,
		kPlatformPC,
		ADGF_NO_FLAGS,
		GUIO2(GUIO_NOSUBTITLES, GUIO_NOSPEECH)
	},
	kGameTypeGob1,
	kFeaturesCD,
	0, 0, 0
},
{ // Supplied by goodoldgeorg in bug report #2810082
	{
		"gob1cd",
		"v1.02",
		AD_ENTRY1s("intro.stk", "40d4a53818f4fce3f5997d02c3fafe73", 4049248),
		HU_HUN,
		kPlatformPC,
		ADGF_NO_FLAGS,
		GUIO2(GUIO_NOSUBTITLES, GUIO_NOSPEECH)
	},
	kGameTypeGob1,
	kFeaturesCD,
	0, 0, 0
},
{ // Supplied by goodoldgeorg in bug report #2810082
	{
		"gob1cd",
		"v1.02",
		AD_ENTRY1s("intro.stk", "40d4a53818f4fce3f5997d02c3fafe73", 4049248),
		FR_FRA,
		kPlatformPC,
		ADGF_NO_FLAGS,
		GUIO2(GUIO_NOSUBTITLES, GUIO_NOSPEECH)
	},
	kGameTypeGob1,
	kFeaturesCD,
	0, 0, 0
},
{ // Supplied by goodoldgeorg in bug report #2810082
	{
		"gob1cd",
		"v1.02",
		AD_ENTRY1s("intro.stk", "40d4a53818f4fce3f5997d02c3fafe73", 4049248),
		ES_ESP,
		kPlatformPC,
		ADGF_NO_FLAGS,
		GUIO2(GUIO_NOSUBTITLES, GUIO_NOSPEECH)
	},
	kGameTypeGob1,
	kFeaturesCD,
	0, 0, 0
},
{ // Supplied by goodoldgeorg in bug report #2810082
	{
		"gob1cd",
		"v1.02",
		AD_ENTRY1s("intro.stk", "40d4a53818f4fce3f5997d02c3fafe73", 4049248),
		IT_ITA,
		kPlatformPC,
		ADGF_NO_FLAGS,
		GUIO2(GUIO_NOSUBTITLES, GUIO_NOSPEECH)
	},
	kGameTypeGob1,
	kFeaturesCD,
	0, 0, 0
},

// -- Mac --

{ // Supplied by raina in the forums
	{
		"gob1",
		"",
		AD_ENTRY1s("intro.stk", "6d837c6380d8f4d984c9f6cc0026df4f", 192712),
		EN_ANY,
		kPlatformMacintosh,
		ADGF_NO_FLAGS,
		GUIO2(GUIO_NOSUBTITLES, GUIO_NOSPEECH)
	},
	kGameTypeGob1,
	kFeaturesNone,
	0, 0, 0
},
{ // Supplied by paul66 in bug report #1652352
	{
		"gob1",
		"",
		AD_ENTRY1("intro.stk", "00a42a7d2d22e6b6ab1b8c673c4ed267"),
		EN_ANY,
		kPlatformMacintosh,
		ADGF_NO_FLAGS,
		GUIO2(GUIO_NOSUBTITLES, GUIO_NOSPEECH)
	},
	kGameTypeGob1,
	kFeaturesAdLib,
	0, 0, 0
},
{ // Supplied by paul66 in bug report #1652352
	{
		"gob1",
		"",
		AD_ENTRY1("intro.stk", "00a42a7d2d22e6b6ab1b8c673c4ed267"),
		DE_DEU,
		kPlatformMacintosh,
		ADGF_NO_FLAGS,
		GUIO2(GUIO_NOSUBTITLES, GUIO_NOSPEECH)
	},
	kGameTypeGob1,
	kFeaturesAdLib,
	0, 0, 0
},
{ // Supplied by paul66 in bug report #1652352
	{
		"gob1",
		"",
		AD_ENTRY1("intro.stk", "00a42a7d2d22e6b6ab1b8c673c4ed267"),
		FR_FRA,
		kPlatformMacintosh,
		ADGF_NO_FLAGS,
		GUIO2(GUIO_NOSUBTITLES, GUIO_NOSPEECH)
	},
	kGameTypeGob1,
	kFeaturesAdLib,
	0, 0, 0
},
{ // Supplied by paul66 in bug report #1652352
	{
		"gob1",
		"",
		AD_ENTRY1("intro.stk", "00a42a7d2d22e6b6ab1b8c673c4ed267"),
		IT_ITA,
		kPlatformMacintosh,
		ADGF_NO_FLAGS,
		GUIO2(GUIO_NOSUBTITLES, GUIO_NOSPEECH)
	},
	kGameTypeGob1,
	kFeaturesAdLib,
	0, 0, 0
},
{ // Supplied by paul66 in bug report #1652352
	{
		"gob1",
		"",
		AD_ENTRY1("intro.stk", "00a42a7d2d22e6b6ab1b8c673c4ed267"),
		ES_ESP,
		kPlatformMacintosh,
		ADGF_NO_FLAGS,
		GUIO2(GUIO_NOSUBTITLES, GUIO_NOSPEECH)
	},
	kGameTypeGob1,
	kFeaturesAdLib,
	0, 0, 0
},

// -- Windows --

{ // Supplied by Hkz on #scummvm
	{
		"gob1",
		"",
		{
			{"intro.stk", 0, "f5f028ee39c456fa51fa63b606583918", 313472},
			{"musmac1.mid", 0, "4f66903b33df8a20edd4c748809c0b56", 8161},
			{0, 0, 0, 0}
		},
		FR_FRA,
		kPlatformWindows,
		ADGF_NO_FLAGS,
		GUIO2(GUIO_NOSUBTITLES, GUIO_NOSPEECH)
	},
	kGameTypeGob1,
	kFeaturesAdLib,
	0, 0, 0
},
{ // Supplied by Hkz on #scummvm
	{
		"gob1",
		"",
		{
			{"intro.stk", 0, "f5f028ee39c456fa51fa63b606583918", 313472},
			{"musmac1.mid", 0, "4f66903b33df8a20edd4c748809c0b56", 8161},
			{0, 0, 0, 0}
		},
		IT_ITA,
		kPlatformWindows,
		ADGF_NO_FLAGS,
		GUIO2(GUIO_NOSUBTITLES, GUIO_NOSPEECH)
	},
	kGameTypeGob1,
	kFeaturesAdLib,
	0, 0, 0
},
{ // Supplied by Hkz on #scummvm
	{
		"gob1",
		"",
		{
			{"intro.stk", 0, "f5f028ee39c456fa51fa63b606583918", 313472},
			{"musmac1.mid", 0, "4f66903b33df8a20edd4c748809c0b56", 8161},
			{0, 0, 0, 0}
		},
		EN_GRB,
		kPlatformWindows,
		ADGF_NO_FLAGS,
		GUIO2(GUIO_NOSUBTITLES, GUIO_NOSPEECH)
	},
	kGameTypeGob1,
	kFeaturesAdLib,
	0, 0, 0
},
{ // Supplied by Hkz on #scummvm
	{
		"gob1",
		"",
		{
			{"intro.stk", 0, "f5f028ee39c456fa51fa63b606583918", 313472},
			{"musmac1.mid", 0, "4f66903b33df8a20edd4c748809c0b56", 8161},
			{0, 0, 0, 0}
		},
		DE_DEU,
		kPlatformWindows,
		ADGF_NO_FLAGS,
		GUIO2(GUIO_NOSUBTITLES, GUIO_NOSPEECH)
	},
	kGameTypeGob1,
	kFeaturesAdLib,
	0, 0, 0
},
{ // Supplied by Hkz on #scummvm
	{
		"gob1",
		"",
		{
			{"intro.stk", 0, "f5f028ee39c456fa51fa63b606583918", 313472},
			{"musmac1.mid", 0, "4f66903b33df8a20edd4c748809c0b56", 8161},
			{0, 0, 0, 0}
		},
		ES_ESP,
		kPlatformWindows,
		ADGF_NO_FLAGS,
		GUIO2(GUIO_NOSUBTITLES, GUIO_NOSPEECH)
	},
	kGameTypeGob1,
	kFeaturesAdLib,
	0, 0, 0
},
{
	{
		"gob1",
		"",
		{
			{"intro.stk", 0, "e157cb59c6d330ca70d12ab0ef1dd12b", 288972},
			{"musmac1.mid", 0, "4f66903b33df8a20edd4c748809c0b56", 8161},
			{0, 0, 0, 0}
		},
		EN_GRB,
		kPlatformWindows,
		ADGF_NO_FLAGS,
		GUIO2(GUIO_NOSUBTITLES, GUIO_NOSPEECH)
	},
	kGameTypeGob1,
	kFeaturesAdLib,
	0, 0, 0
},
{
	{
		"gob1",
		"",
		{
			{"intro.stk", 0, "e157cb59c6d330ca70d12ab0ef1dd12b", 288972},
			{"musmac1.mid", 0, "4f66903b33df8a20edd4c748809c0b56", 8161},
			{0, 0, 0, 0}
		},
		FR_FRA,
		kPlatformWindows,
		ADGF_NO_FLAGS,
		GUIO2(GUIO_NOSUBTITLES, GUIO_NOSPEECH)
	},
	kGameTypeGob1,
	kFeaturesAdLib,
	0, 0, 0
},
{
	{
		"gob1",
		"",
		{
			{"intro.stk", 0, "e157cb59c6d330ca70d12ab0ef1dd12b", 288972},
			{"musmac1.mid", 0, "4f66903b33df8a20edd4c748809c0b56", 8161},
			{0, 0, 0, 0}
		},
		ES_ESP,
		kPlatformWindows,
		ADGF_NO_FLAGS,
		GUIO2(GUIO_NOSUBTITLES, GUIO_NOSPEECH)
	},
	kGameTypeGob1,
	kFeaturesAdLib,
	0, 0, 0
},
{
	{
		"gob1",
		"",
		{
			{"intro.stk", 0, "e157cb59c6d330ca70d12ab0ef1dd12b", 288972},
			{"musmac1.mid", 0, "4f66903b33df8a20edd4c748809c0b56", 8161},
			{0, 0, 0, 0}
		},
		IT_ITA,
		kPlatformWindows,
		ADGF_NO_FLAGS,
		GUIO2(GUIO_NOSUBTITLES, GUIO_NOSPEECH)
	},
	kGameTypeGob1,
	kFeaturesAdLib,
	0, 0, 0
},
{
	{
		"gob1",
		"",
		{
			{"intro.stk", 0, "e157cb59c6d330ca70d12ab0ef1dd12b", 288972},
			{"musmac1.mid", 0, "4f66903b33df8a20edd4c748809c0b56", 8161},
			{0, 0, 0, 0}
		},
		DE_DEU,
		kPlatformWindows,
		ADGF_NO_FLAGS,
		GUIO2(GUIO_NOSUBTITLES, GUIO_NOSPEECH)
	},
	kGameTypeGob1,
	kFeaturesAdLib,
	0, 0, 0
},
{ // Found in french ADI 2.5 Anglais Multimedia 5e
	{
		"gob1",
		"",
		AD_ENTRY1s("intro.stk", "f5f028ee39c456fa51fa63b606583918", 313472),
		FR_FRA,
		kPlatformWindows,
		ADGF_NO_FLAGS,
		GUIO2(GUIO_NOSUBTITLES, GUIO_NOSPEECH)
	},
	kGameTypeGob1,
	kFeaturesAdLib,
	0, 0, 0
},
{ // Found in french ADI 2.5 Anglais Multimedia 5e
	{
		"gob1",
		"",
		AD_ENTRY1s("intro.stk", "f5f028ee39c456fa51fa63b606583918", 313472),
		EN_GRB,
		kPlatformWindows,
		ADGF_NO_FLAGS,
		GUIO2(GUIO_NOSUBTITLES, GUIO_NOSPEECH)
	},
	kGameTypeGob1,
	kFeaturesAdLib,
	0, 0, 0
},
{ // Found in french ADI 2.5 Anglais Multimedia 5e
	{
		"gob1",
		"",
		AD_ENTRY1s("intro.stk", "f5f028ee39c456fa51fa63b606583918", 313472),
		DE_DEU,
		kPlatformWindows,
		ADGF_NO_FLAGS,
		GUIO2(GUIO_NOSUBTITLES, GUIO_NOSPEECH)
	},
	kGameTypeGob1,
	kFeaturesAdLib,
	0, 0, 0
},
{ // Found in french ADI 2.5 Anglais Multimedia 5e
	{
		"gob1",
		"",
		AD_ENTRY1s("intro.stk", "f5f028ee39c456fa51fa63b606583918", 313472),
		IT_ITA,
		kPlatformWindows,
		ADGF_NO_FLAGS,
		GUIO2(GUIO_NOSUBTITLES, GUIO_NOSPEECH)
	},
	kGameTypeGob1,
	kFeaturesAdLib,
	0, 0, 0
},
{ // Found in french ADI 2.5 Anglais Multimedia 5e
	{
		"gob1",
		"",
		AD_ENTRY1s("intro.stk", "f5f028ee39c456fa51fa63b606583918", 313472),
		ES_ESP,
		kPlatformWindows,
		ADGF_NO_FLAGS,
		GUIO2(GUIO_NOSUBTITLES, GUIO_NOSPEECH)
	},
	kGameTypeGob1,
	kFeaturesAdLib,
	0, 0, 0
},

// -- Demos --

{
	{
		"gob1",
		"Demo",
		AD_ENTRY1("intro.stk", "972f22c6ff8144a6636423f0354ca549"),
		UNK_LANG,
		kPlatformAmiga,
		ADGF_DEMO,
		GUIO2(GUIO_NOSUBTITLES, GUIO_NOSPEECH)
	},
	kGameTypeGob1,
	kFeaturesNone,
	0, 0, 0
},
{
	{
		"gob1",
		"Interactive Demo",
		AD_ENTRY1("intro.stk", "e72bd1e3828c7dec4c8a3e58c48bdfdb"),
		UNK_LANG,
		kPlatformPC,
		ADGF_DEMO,
		GUIO2(GUIO_NOSUBTITLES, GUIO_NOSPEECH)
	},
	kGameTypeGob1,
	kFeaturesNone,
	0, 0, 0
},
{
	{
		"gob1",
		"Interactive Demo",
		AD_ENTRY1s("intro.stk", "a796096280d5efd48cf8e7dfbe426eb5", 193595),
		UNK_LANG,
		kPlatformPC,
		ADGF_DEMO,
		GUIO2(GUIO_NOSUBTITLES, GUIO_NOSPEECH)
	},
	kGameTypeGob1,
	kFeaturesNone,
	0, 0, 0
},
{ // Supplied by goodoldgeorg in bug report #2785958
	{
		"gob1",
		"Interactive Demo",
		AD_ENTRY1s("intro.stk", "35a098571af9a03c04e2303aec7c9249", 116582),
		FR_FRA,
		kPlatformPC,
		ADGF_DEMO,
		GUIO2(GUIO_NOSUBTITLES, GUIO_NOSPEECH)
	},
	kGameTypeGob1,
	kFeaturesNone,
	0, 0, 0
},
{
	{
		"gob1",
		"",
		AD_ENTRY1s("intro.stk", "0e022d3f2481b39e9175d37b2c6ad4c6", 2390121),
		FR_FRA,
		kPlatformCDi,
		ADGF_NO_FLAGS,
		GUIO2(GUIO_NOSUBTITLES, GUIO_NOSPEECH)
	},
	kGameTypeGob1,
	kFeaturesAdLib,
	0, "AVT003.TOT", 0
},

#endif // GOB_DETECTION_TABLES_GOB1_H
