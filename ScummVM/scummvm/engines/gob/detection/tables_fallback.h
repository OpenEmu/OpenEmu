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

#ifndef GOB_DETECTION_TABLES_FALLBACK_H
#define GOB_DETECTION_TABLES_FALLBACK_H

// -- Tables for the filename-based fallback --

static const GOBGameDescription fallbackDescs[] = {
	{ //0
		{
			"gob1",
			"unknown",
			AD_ENTRY1(0, 0),
			UNK_LANG,
			kPlatformPC,
			ADGF_NO_FLAGS,
			GUIO2(GUIO_NOSUBTITLES, GUIO_NOSPEECH)
		},
		kGameTypeGob1,
		kFeaturesNone,
		0, 0, 0
	},
	{ //1
		{
			"gob1cd",
			"unknown",
			AD_ENTRY1(0, 0),
			UNK_LANG,
			kPlatformPC,
			ADGF_NO_FLAGS,
			GUIO2(GUIO_NOSUBTITLES, GUIO_NOSPEECH)
		},
		kGameTypeGob1,
		kFeaturesCD,
		0, 0, 0
	},
	{ //2
		{
			"gob2",
			"unknown",
			AD_ENTRY1(0, 0),
			UNK_LANG,
			kPlatformPC,
			ADGF_NO_FLAGS,
			GUIO2(GUIO_NOSUBTITLES, GUIO_NOSPEECH)
		},
		kGameTypeGob2,
		kFeaturesAdLib,
		0, 0, 0
	},
	{ //3
		{
			"gob2mac",
			"unknown",
			AD_ENTRY1(0, 0),
			UNK_LANG,
			kPlatformMacintosh,
			ADGF_NO_FLAGS,
			GUIO2(GUIO_NOSUBTITLES, GUIO_NOSPEECH)
		},
		kGameTypeGob2,
		kFeaturesAdLib,
		0, 0, 0
	},
	{ //4
		{
			"gob2cd",
			"unknown",
			AD_ENTRY1(0, 0),
			UNK_LANG,
			kPlatformPC,
			ADGF_NO_FLAGS,
			GUIO2(GUIO_NOSUBTITLES, GUIO_NOSPEECH)
		},
		kGameTypeGob2,
		kFeaturesCD,
		0, 0, 0
	},
	{ //5
		{
			"bargon",
			"",
			AD_ENTRY1(0, 0),
			UNK_LANG,
			kPlatformPC,
			ADGF_NO_FLAGS,
			GUIO2(GUIO_NOSUBTITLES, GUIO_NOSPEECH)
		},
		kGameTypeBargon,
		kFeaturesNone,
		0, 0, 0
	},
	{ //6
		{
			"gob3",
			"unknown",
			AD_ENTRY1(0, 0),
			UNK_LANG,
			kPlatformPC,
			ADGF_NO_FLAGS,
			GUIO2(GUIO_NOSUBTITLES, GUIO_NOSPEECH)
		},
		kGameTypeGob3,
		kFeaturesAdLib,
		0, 0, 0
	},
	{ //7
		{
			"gob3cd",
			"unknown",
			AD_ENTRY1(0, 0),
			UNK_LANG,
			kPlatformPC,
			ADGF_NO_FLAGS,
			GUIO2(GUIO_NOSUBTITLES, GUIO_NOSPEECH)
		},
		kGameTypeGob3,
		kFeaturesCD,
		0, 0, 0
	},
	{ //8
		{
			"woodruff",
			"unknown",
			AD_ENTRY1(0, 0),
			UNK_LANG,
			kPlatformPC,
			ADGF_NO_FLAGS,
			GUIO3(GUIO_NOSUBTITLES, GUIO_NOSPEECH, GUIO_NOASPECT)
		},
		kGameTypeWoodruff,
		kFeatures640x480,
		0, 0, 0
	},
	{ //9
		{
			"lostintime",
			"unknown",
			AD_ENTRY1(0, 0),
			UNK_LANG,
			kPlatformPC,
			ADGF_NO_FLAGS,
			GUIO2(GUIO_NOSUBTITLES, GUIO_NOSPEECH)
		},
		kGameTypeLostInTime,
		kFeaturesAdLib,
		0, 0, 0
	},
	{ //10
		{
			"lostintime",
			"unknown",
			AD_ENTRY1(0, 0),
			UNK_LANG,
			kPlatformMacintosh,
			ADGF_NO_FLAGS,
			GUIO2(GUIO_NOSUBTITLES, GUIO_NOSPEECH)
		},
		kGameTypeLostInTime,
		kFeaturesAdLib,
		0, 0, 0
	},
	{ //11
		{
			"lostintime",
			"unknown",
			AD_ENTRY1(0, 0),
			UNK_LANG,
			kPlatformPC,
			ADGF_NO_FLAGS,
			GUIO2(GUIO_NOSUBTITLES, GUIO_NOSPEECH)
		},
		kGameTypeLostInTime,
		kFeaturesCD,
		0, 0, 0
	},
	{ //12
		{
			"urban",
			"unknown",
			AD_ENTRY1(0, 0),
			UNK_LANG,
			kPlatformPC,
			ADGF_NO_FLAGS,
			GUIO3(GUIO_NOSUBTITLES, GUIO_NOSPEECH, GUIO_NOASPECT)
		},
		kGameTypeUrban,
		kFeatures640x480 | kFeaturesTrueColor,
		0, 0, 0
	},
	{ //13
		{
			"playtoons1",
			"unknown",
			AD_ENTRY1(0, 0),
			UNK_LANG,
			kPlatformPC,
			ADGF_NO_FLAGS,
			GUIO3(GUIO_NOSUBTITLES, GUIO_NOSPEECH, GUIO_NOASPECT)
		},
		kGameTypePlaytoons,
		kFeatures640x480,
		0, 0, 0
	},
	{ //14
		{
			"playtoons2",
			"unknown",
			AD_ENTRY1(0, 0),
			UNK_LANG,
			kPlatformPC,
			ADGF_NO_FLAGS,
			GUIO3(GUIO_NOSUBTITLES, GUIO_NOSPEECH, GUIO_NOASPECT)
		},
		kGameTypePlaytoons,
		kFeatures640x480,
		0, 0, 0
	},
	{ //15
		{
			"playtoons3",
			"unknown",
			AD_ENTRY1(0, 0),
			UNK_LANG,
			kPlatformPC,
			ADGF_NO_FLAGS,
			GUIO3(GUIO_NOSUBTITLES, GUIO_NOSPEECH, GUIO_NOASPECT)
		},
		kGameTypePlaytoons,
		kFeatures640x480,
		0, 0, 0
	},
	{ //16
		{
			"playtoons4",
			"unknown",
			AD_ENTRY1(0, 0),
			UNK_LANG,
			kPlatformPC,
			ADGF_NO_FLAGS,
			GUIO3(GUIO_NOSUBTITLES, GUIO_NOSPEECH, GUIO_NOASPECT)
		},
		kGameTypePlaytoons,
		kFeatures640x480,
		0, 0, 0
	},
	{ //17
		{
			"playtoons5",
			"unknown",
			AD_ENTRY1(0, 0),
			UNK_LANG,
			kPlatformPC,
			ADGF_NO_FLAGS,
			GUIO3(GUIO_NOSUBTITLES, GUIO_NOSPEECH, GUIO_NOASPECT)
		},
		kGameTypePlaytoons,
		kFeatures640x480,
		0, 0, 0
	},
	{ //18
		{
			"playtoons construction kit",
			"unknown",
			AD_ENTRY1(0, 0),
			UNK_LANG,
			kPlatformPC,
			ADGF_NO_FLAGS,
			GUIO3(GUIO_NOSUBTITLES, GUIO_NOSPEECH, GUIO_NOASPECT)
		},
		kGameTypePlaytoons,
		kFeatures640x480,
		0, 0, 0
	},
	{ //19
		{
			"bambou",
			"unknown",
			AD_ENTRY1(0, 0),
			UNK_LANG,
			kPlatformPC,
			ADGF_NO_FLAGS,
			GUIO3(GUIO_NOSUBTITLES, GUIO_NOSPEECH, GUIO_NOASPECT)
		},
		kGameTypeBambou,
		kFeatures640x480,
		0, 0, 0
	},
	{ //20
		{
			"fascination",
			"unknown",
			AD_ENTRY1(0, 0),
			UNK_LANG,
			kPlatformPC,
			ADGF_NO_FLAGS,
			GUIO2(GUIO_NOSUBTITLES, GUIO_NOSPEECH)
		},
		kGameTypeFascination,
		kFeaturesAdLib,
		"disk0.stk", 0, 0
	},
	{ //21
		{
			"geisha",
			"unknown",
			AD_ENTRY1(0, 0),
			UNK_LANG,
			kPlatformPC,
			ADGF_NO_FLAGS,
			GUIO2(GUIO_NOSUBTITLES, GUIO_NOSPEECH)
		},
		kGameTypeGeisha,
		kFeaturesEGA | kFeaturesAdLib,
		"disk1.stk", "intro.tot", 0
	},
	{ //22
		{
			"littlered",
			"unknown",
			AD_ENTRY1(0, 0),
			UNK_LANG,
			kPlatformPC,
			ADGF_NO_FLAGS,
			GUIO2(GUIO_NOSUBTITLES, GUIO_NOSPEECH)
		},
		kGameTypeLittleRed,
		kFeaturesAdLib | kFeaturesEGA,
		0, 0, 0
	},
	{ //23
		{
			"littlered",
			"unknown",
			AD_ENTRY1(0, 0),
			UNK_LANG,
			kPlatformAmiga,
			ADGF_NO_FLAGS,
			GUIO2(GUIO_NOSUBTITLES, GUIO_NOSPEECH)
		},
		kGameTypeLittleRed,
		kFeaturesNone,
		0, 0, 0
	},
	{ //24
		{
			"onceupon",
			"unknown",
			AD_ENTRY1(0, 0),
			UNK_LANG,
			kPlatformUnknown,
			ADGF_NO_FLAGS,
			GUIO2(GUIO_NOSUBTITLES, GUIO_NOSPEECH)
		},
		kGameTypeOnceUponATime,
		kFeaturesEGA,
		0, 0, 0
	},
	{ //25
		{
			"adi2",
			"",
			AD_ENTRY1(0, 0),
			UNK_LANG,
			kPlatformPC,
			ADGF_NO_FLAGS,
			GUIO3(GUIO_NOSUBTITLES, GUIO_NOSPEECH, GUIO_NOASPECT)
		},
		kGameTypeAdi2,
		kFeatures640x480,
		"adi2.stk", 0, 0
	},
	{ //26
		{
			"adi4",
			"",
			AD_ENTRY1(0, 0),
			UNK_LANG,
			kPlatformPC,
			ADGF_NO_FLAGS,
			GUIO3(GUIO_NOSUBTITLES, GUIO_NOSPEECH, GUIO_NOASPECT)
		},
		kGameTypeAdi4,
		kFeatures640x480,
		"adif41.stk", 0, 0
	},
	{ //27
		{
			"coktelplayer",
			"unknown",
			AD_ENTRY1(0, 0),
			UNK_LANG,
			kPlatformPC,
			ADGF_NO_FLAGS,
			GUIO1(GUIO_NOASPECT)
		},
		kGameTypeUrban,
		kFeaturesAdLib | kFeatures640x480 | kFeaturesSCNDemo,
		"", "", 8
	}
};

static const ADFileBasedFallback fileBased[] = {
	{ &fallbackDescs[ 0].desc, { "intro.stk", "disk1.stk", "disk2.stk", "disk3.stk", "disk4.stk", 0 } },
	{ &fallbackDescs[ 1].desc, { "intro.stk", "gob.lic", 0 } },
	{ &fallbackDescs[ 2].desc, { "intro.stk", 0 } },
	{ &fallbackDescs[ 2].desc, { "intro.stk", "disk2.stk", "disk3.stk", 0 } },
	{ &fallbackDescs[ 3].desc, { "intro.stk", "disk2.stk", "disk3.stk", "musmac1.mid", 0 } },
	{ &fallbackDescs[ 4].desc, { "intro.stk", "gobnew.lic", 0 } },
	{ &fallbackDescs[ 5].desc, { "intro.stk", "scaa.imd", "scba.imd", "scbf.imd", 0 } },
	{ &fallbackDescs[ 6].desc, { "intro.stk", "imd.itk", 0 } },
	{ &fallbackDescs[ 7].desc, { "intro.stk", "mus_gob3.lic", 0 } },
	{ &fallbackDescs[ 8].desc, { "intro.stk", "woodruff.itk", 0 } },
	{ &fallbackDescs[ 9].desc, { "intro.stk", "commun1.itk", 0 } },
	{ &fallbackDescs[10].desc, { "intro.stk", "commun1.itk", "musmac1.mid", 0 } },
	{ &fallbackDescs[11].desc, { "intro.stk", "commun1.itk", "lost.lic", 0 } },
	{ &fallbackDescs[12].desc, { "intro.stk", "cd1.itk", "objet1.itk", 0 } },
	{ &fallbackDescs[13].desc, { "playtoon.stk", "archi.stk", 0 } },
	{ &fallbackDescs[14].desc, { "playtoon.stk", "spirou.stk", 0 } },
	{ &fallbackDescs[15].desc, { "playtoon.stk", "chato.stk", 0 } },
	{ &fallbackDescs[16].desc, { "playtoon.stk", "manda.stk", 0 } },
	{ &fallbackDescs[17].desc, { "playtoon.stk", "wakan.stk", 0 } },
	{ &fallbackDescs[18].desc, { "playtoon.stk", "dan.itk" } },
	{ &fallbackDescs[19].desc, { "intro.stk", "bambou.itk", 0 } },
	{ &fallbackDescs[20].desc, { "disk0.stk", "disk1.stk", "disk2.stk", "disk3.stk", 0 } },
	{ &fallbackDescs[21].desc, { "disk1.stk", "disk2.stk", "disk3.stk", 0 } },
	{ &fallbackDescs[22].desc, { "intro.stk", "stk2.stk", "stk3.stk", 0 } },
	{ &fallbackDescs[23].desc, { "intro.stk", "stk2.stk", "stk3.stk", "mod.babayaga", 0 } },
	{ &fallbackDescs[24].desc, { "stk1.stk", "stk2.stk", "stk3.stk", 0 } },
	{ &fallbackDescs[25].desc, { "adi2.stk", 0 } },
	{ &fallbackDescs[26].desc, { "adif41.stk", "adim41.stk", 0 } },
	{ &fallbackDescs[27].desc, { "coktelplayer.scn", 0 } },
	{ 0, { 0 } }
};

// -- Tables for detecting the specific Once Upon A Time game --

enum OnceUponATime {
	kOnceUponATimeInvalid     = -1,
	kOnceUponATimeAbracadabra =  0,
	kOnceUponATimeBabaYaga    =  1,
	kOnceUponATimeMAX
};

enum OnceUponATimePlatform {
	kOnceUponATimePlatformInvalid = -1,
	kOnceUponATimePlatformDOS     =  0,
	kOnceUponATimePlatformAmiga   =  1,
	kOnceUponATimePlatformAtariST =  2,
	kOnceUponATimePlatformMAX
};

static const GOBGameDescription fallbackOnceUpon[kOnceUponATimeMAX][kOnceUponATimePlatformMAX] = {
	{ // kOnceUponATimeAbracadabra
		{ // kOnceUponATimePlatformDOS
			{
				"abracadabra",
				"",
				AD_ENTRY1(0, 0),
				UNK_LANG,
				kPlatformPC,
				ADGF_NO_FLAGS,
				GUIO2(GUIO_NOSUBTITLES, GUIO_NOSPEECH)
			},
			kGameTypeAbracadabra,
			kFeaturesAdLib | kFeaturesEGA,
			0, 0, 0
		},
		{ // kOnceUponATimePlatformAmiga
			{
				"abracadabra",
				"",
				AD_ENTRY1(0, 0),
				UNK_LANG,
				kPlatformAmiga,
				ADGF_NO_FLAGS,
				GUIO2(GUIO_NOSUBTITLES, GUIO_NOSPEECH)
			},
			kGameTypeAbracadabra,
			kFeaturesEGA,
			0, 0, 0
		},
		{ // kOnceUponATimePlatformAtariST
			{
				"abracadabra",
				"",
				AD_ENTRY1(0, 0),
				UNK_LANG,
				kPlatformAtariST,
				ADGF_NO_FLAGS,
				GUIO2(GUIO_NOSUBTITLES, GUIO_NOSPEECH)
			},
			kGameTypeAbracadabra,
			kFeaturesEGA,
			0, 0, 0
		}
	},
	{ // kOnceUponATimeBabaYaga
		{ // kOnceUponATimePlatformDOS
			{
				"babayaga",
				"",
				AD_ENTRY1(0, 0),
				UNK_LANG,
				kPlatformPC,
				ADGF_NO_FLAGS,
				GUIO2(GUIO_NOSUBTITLES, GUIO_NOSPEECH)
			},
			kGameTypeBabaYaga,
			kFeaturesAdLib | kFeaturesEGA,
			0, 0, 0
		},
		{ // kOnceUponATimePlatformAmiga
			{
				"babayaga",
				"",
				AD_ENTRY1(0, 0),
				UNK_LANG,
				kPlatformAmiga,
				ADGF_NO_FLAGS,
				GUIO2(GUIO_NOSUBTITLES, GUIO_NOSPEECH)
			},
			kGameTypeBabaYaga,
			kFeaturesEGA,
			0, 0, 0
		},
		{ // kOnceUponATimePlatformAtariST
			{
				"babayaga",
				"",
				AD_ENTRY1(0, 0),
				UNK_LANG,
				kPlatformAtariST,
				ADGF_NO_FLAGS,
				GUIO2(GUIO_NOSUBTITLES, GUIO_NOSPEECH)
			},
			kGameTypeBabaYaga,
			kFeaturesEGA,
			0, 0, 0
		}
	}
};

#endif // GOB_DETECTION_TABLES_FALLBACK_H
