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

/* Detection tables for the Playtoons series. */

#ifndef GOB_DETECTION_TABLES_PLAYTOONS_H
#define GOB_DETECTION_TABLES_PLAYTOONS_H

// -- Playtoons 1: Uncle Archibald --

{
	{
		"playtoons1",
		"",
		{
			{"playtoon.stk", 0, "8c98e9a11be9bb203a55e8c6e68e519b", 25574338},
			{"archi.stk", 0, "8d44b2a0d4e3139471213f9f0ed21e81", 5524674},
			{0, 0, 0, 0}
		},
		FR_FRA,
		kPlatformPC,
		ADGF_NO_FLAGS,
		GUIO3(GUIO_NOSUBTITLES, GUIO_NOSPEECH, GUIO_NOASPECT)
	},
	kGameTypePlaytoons,
	kFeatures640x480,
	"intro2.stk", 0, 0
},
{
	{
		"playtoons1",
		"Pack mes histoires anim\xE9""es",
		{
			{"playtoon.stk", 0, "55f0293202963854192e39474e214f5f", 30448474},
			{"archi.stk", 0, "8d44b2a0d4e3139471213f9f0ed21e81", 5524674},
			{0, 0, 0, 0}
		},
		FR_FRA,
		kPlatformPC,
		ADGF_NO_FLAGS,
		GUIO3(GUIO_NOSUBTITLES, GUIO_NOSPEECH, GUIO_NOASPECT)
	},
	kGameTypePlaytoons,
	kFeatures640x480,
	"intro2.stk", 0, 0
},
{
	{
		"playtoons1",
		"",
		{
			{"playtoon.stk", 0, "c5ca2a288cdaefca9556cd9ae4b579cf", 25158926},
			{"archi.stk", 0, "8d44b2a0d4e3139471213f9f0ed21e81", 5524674},
			{0, 0, 0, 0}
		},
		DE_DEU,
		kPlatformPC,
		ADGF_NO_FLAGS,
		GUIO3(GUIO_NOSUBTITLES, GUIO_NOSPEECH, GUIO_NOASPECT)
	},
	kGameTypePlaytoons,
	kFeatures640x480,
	"intro2.stk", 0, 0
},
{ // Supplied by scoriae in the forums
	{
		"playtoons1",
		"",
		{
			{"playtoon.stk", 0, "9e513e993a5b0e2496add3f50c08764b", 30448506},
			{"archi.stk", 0, "00d8274519dfcf8a0d8ae3099daea0f8", 5532135},
			{0, 0, 0, 0}
		},
		EN_ANY,
		kPlatformPC,
		ADGF_NO_FLAGS,
		GUIO3(GUIO_NOSUBTITLES, GUIO_NOSPEECH, GUIO_NOASPECT)
	},
	kGameTypePlaytoons,
	kFeatures640x480,
	"intro2.stk", 0, 0
},
{
	{
		"playtoons1",
		"Non-Interactive Demo",
		{
			{"play123.scn", 0, "4689a31f543915e488c3bc46ea358add", 258},
			{"archi.vmd", 0, "a410fcc8116bc173f038100f5857191c", 5617210},
			{"chato.vmd", 0, "5a10e39cb66c396f2f9d8fb35e9ac016", 5445937},
			{"genedeb.vmd", 0, "3bb4a45585f88f4d839efdda6a1b582b", 1244228},
			{"generik.vmd", 0, "b46bdd64b063e86927fb2826500ad512", 603242},
			{"genespi.vmd", 0, "b7611916f32a370ae9832962fc17ef72", 758719},
			{"spirou.vmd", 0, "8513dbf7ac51c057b21d371d6b217b47", 2550788},
			{0, 0, 0, 0}
		},
		EN_ANY,
		kPlatformPC,
		ADGF_NO_FLAGS,
		GUIO3(GUIO_NOSUBTITLES, GUIO_NOSPEECH, GUIO_NOASPECT)
	},
	kGameTypePlaytoons,
	kFeatures640x480 | kFeaturesSCNDemo,
	0, 0, 3
},
{
	{
		"playtoons1",
		"Non-Interactive Demo",
		{
			{"e.scn", 0, "8a0db733c3f77be86e74e8242e5caa61", 124},
			{"demarchg.vmd", 0, "d14a95da7d8792faf5503f649ffcbc12", 5619415},
			{0, 0, 0, 0}
		},
		EN_ANY,
		kPlatformPC,
		ADGF_NO_FLAGS,
		GUIO3(GUIO_NOSUBTITLES, GUIO_NOSPEECH, GUIO_NOASPECT)
	},
	kGameTypePlaytoons,
	kFeatures640x480 | kFeaturesSCNDemo,
	0, 0, 4
},
{
	{
		"playtoons1",
		"Non-Interactive Demo",
		{
			{"i.scn", 0, "8b3294474d39970463663edd22341730", 285},
			{"demarita.vmd", 0, "84c8672b91c7312462603446e224bfec", 5742533},
			{"dembouit.vmd", 0, "7a5fdf0a4dbdfe72e31dd489ea0f8aa2", 3536786},
			{"demo5.vmd", 0, "2abb7b6a26406c984f389f0b24b5e28e", 13290970},
			{"demoita.vmd", 0, "b4c0622d14c8749965cd0f5dfca4cf4b", 1183566},
			{"wooddem3.vmd", 0, "a1700596172c2d4e264760030c3a3d47", 8994250},
			{0, 0, 0, 0}
		},
		IT_ITA,
		kPlatformPC,
		ADGF_NO_FLAGS,
		GUIO3(GUIO_NOSUBTITLES, GUIO_NOSPEECH, GUIO_NOASPECT)
	},
	kGameTypePlaytoons,
	kFeatures640x480 | kFeaturesSCNDemo,
	0, 0, 5
},
{
	{
		"playtoons1",
		"Non-Interactive Demo",
		{
			{"s.scn", 0, "1f527010626b5490761f16ba7a6f639a", 251},
			{"demaresp.vmd", 0, "3f860f944056842b35a5fd05416f208e", 5720619},
			{"demboues.vmd", 0, "3a0caa10c98ef92a15942f8274075b43", 3535838},
			{"demo5.vmd", 0, "2abb7b6a26406c984f389f0b24b5e28e", 13290970},
			{"wooddem3.vmd", 0, "a1700596172c2d4e264760030c3a3d47", 8994250},
			{0, 0, 0, 0}
		},
		ES_ESP,
		kPlatformPC,
		ADGF_NO_FLAGS,
		GUIO3(GUIO_NOSUBTITLES, GUIO_NOSPEECH, GUIO_NOASPECT)
	},
	kGameTypePlaytoons,
	kFeatures640x480 | kFeaturesSCNDemo,
	0, 0, 6
},

// -- Playtoons 2: The Case of the Counterfeit Collaborator (Spirou) --

{
	{
		"playtoons2",
		"",
		{
			{"playtoon.stk", 0, "4772c96be88a57f0561519e4a1526c62", 24406262},
			{"spirou.stk", 0, "5d9c7644d0c47840169b4d016765cc1a", 9816201},
			{0, 0, 0, 0}
		},
		EN_ANY,
		kPlatformPC,
		ADGF_NO_FLAGS,
		GUIO3(GUIO_NOSUBTITLES, GUIO_NOSPEECH, GUIO_NOASPECT)
	},
	kGameTypePlaytoons,
	kFeatures640x480,
	"intro2.stk", 0, 0
},
{
	{
		"playtoons2",
		"",
		{
			{"playtoon.stk", 0, "55a85036dd93cce93532d8f743d90074", 17467154},
			{"spirou.stk", 0, "e3e1b6148dd72fafc3637f1a8e5764f5", 9812043},
			{0, 0, 0, 0}
		},
		FR_FRA,
		kPlatformPC,
		ADGF_NO_FLAGS,
		GUIO3(GUIO_NOSUBTITLES, GUIO_NOSPEECH, GUIO_NOASPECT)
	},
	kGameTypePlaytoons,
	kFeatures640x480,
	"intro2.stk", 0, 0
},
{
	{
		"playtoons2",
		"",
		{
			{"playtoon.stk", 0, "c5ca2a288cdaefca9556cd9ae4b579cf", 25158926},
			{"spirou.stk", 0, "91080dc148de1bbd6a97321c1a1facf3", 9817086},
			{0, 0, 0, 0}
		},
		DE_DEU,
		kPlatformPC,
		ADGF_NO_FLAGS,
		GUIO3(GUIO_NOSUBTITLES, GUIO_NOSPEECH, GUIO_NOASPECT)
	},
	kGameTypePlaytoons,
	kFeatures640x480,
	"intro2.stk", 0, 0
},
{ // Supplied by Hkz
	{
		"playtoons2",
		"",
		{
			{"playtoon.stk", 0, "2572685400852d12759a2fbf09ec88eb", 9698780},
			{"spirou.stk", 0, "d3cfeff920b6343a2ece55088f530dba", 7076608},
			{0, 0, 0, 0}
		},
		IT_ITA,
		kPlatformPC,
		ADGF_NO_FLAGS,
		GUIO3(GUIO_NOSUBTITLES, GUIO_NOSPEECH, GUIO_NOASPECT)
	},
	kGameTypePlaytoons,
	kFeatures640x480,
	"intro2.stk", 0, 0
},
{ // Supplied by scoriae in the forums
	{
		"playtoons2",
		"",
		{
			{"playtoon.stk", 0, "9e513e993a5b0e2496add3f50c08764b", 30448506},
			{"spirou.stk", 0, "993737f112ca6a9b33c814273280d832", 9825760},
			{0, 0, 0, 0}
		},
		EN_ANY,
		kPlatformPC,
		ADGF_NO_FLAGS,
		GUIO3(GUIO_NOSUBTITLES, GUIO_NOSPEECH, GUIO_NOASPECT)
	},
	kGameTypePlaytoons,
	kFeatures640x480,
	"intro2.stk", 0, 0
},

// -- Playtoons 3: The Secret of the Castle --

{
	{
		"playtoons3",
		"",
		{
			{"playtoon.stk", 0, "8c98e9a11be9bb203a55e8c6e68e519b", 25574338},
			{"chato.stk", 0, "4fa4ed96a427c344e9f916f9f236598d", 6033793},
			{0, 0, 0, 0}
		},
		FR_FRA,
		kPlatformPC,
		ADGF_NO_FLAGS,
		GUIO3(GUIO_NOSUBTITLES, GUIO_NOSPEECH, GUIO_NOASPECT)
	},
	kGameTypePlaytoons,
	kFeatures640x480,
	"intro2.stk", 0, 0
},
{
	{
		"playtoons3",
		"",
		{
			{"playtoon.stk", 0, "9e513e993a5b0e2496add3f50c08764b", 30448506},
			{"chato.stk", 0, "8fc8d0da5b3e758908d1d7298d497d0b", 6041026},
			{0, 0, 0, 0}
		},
		EN_ANY,
		kPlatformPC,
		ADGF_NO_FLAGS,
		GUIO3(GUIO_NOSUBTITLES, GUIO_NOSPEECH, GUIO_NOASPECT)
	},
	kGameTypePlaytoons,
	kFeatures640x480,
	"intro2.stk", 0, 0
},
{
	{
		"playtoons3",
		"Pack mes histoires anim\xE9""es",
		{
			{"playtoon.stk", 0, "55f0293202963854192e39474e214f5f", 30448474},
			{"chato.stk", 0, "4fa4ed96a427c344e9f916f9f236598d", 6033793},
			{0, 0, 0, 0}
		},
		FR_FRA,
		kPlatformPC,
		ADGF_NO_FLAGS,
		GUIO3(GUIO_NOSUBTITLES, GUIO_NOSPEECH, GUIO_NOASPECT)
	},
	kGameTypePlaytoons,
	kFeatures640x480,
	"intro2.stk", 0, 0
},
{
	{
		"playtoons3",
		"",
		{
			{"playtoon.stk", 0, "c5ca2a288cdaefca9556cd9ae4b579cf", 25158926},
			{"chato.stk", 0, "3c6cb3ac8a5a7cf681a19971a92a748d", 6033791},
			{0, 0, 0, 0}
		},
		DE_DEU,
		kPlatformPC,
		ADGF_NO_FLAGS,
		GUIO3(GUIO_NOSUBTITLES, GUIO_NOSPEECH, GUIO_NOASPECT)
	},
	kGameTypePlaytoons,
	kFeatures640x480,
	"intro2.stk", 0, 0
},
{ // Supplied by Hkz on #scummvm
	{
		"playtoons3",
		"",
		{
			{"playtoon.stk", 0, "4772c96be88a57f0561519e4a1526c62", 24406262},
			{"chato.stk", 0, "bdef407387112bfcee90e664865ac3af", 6033867},
			{0, 0, 0, 0}
		},
		EN_ANY,
		kPlatformPC,
		ADGF_NO_FLAGS,
		GUIO3(GUIO_NOSUBTITLES, GUIO_NOSPEECH, GUIO_NOASPECT)
	},
	kGameTypePlaytoons,
	kFeatures640x480,
	"intro2.stk", 0, 0
},

// -- Playtoons 4: The Mandarin Prince --

{
	{
		"playtoons4",
		"",
		{
			{"playtoon.stk", 0, "b7f5afa2dc1b0f75970b7c07d175db1b", 24340406},
			{"manda.stk", 0, "92529e0b927191d9898a34c2892e9a3a", 6485072},
			{0, 0, 0, 0}
		},
		FR_FRA,
		kPlatformPC,
		ADGF_NO_FLAGS,
		GUIO3(GUIO_NOSUBTITLES, GUIO_NOSPEECH, GUIO_NOASPECT)
	},
	kGameTypePlaytoons,
	kFeatures640x480,
	"intro2.stk", 0, 0
},
{ //Supplied by goodoldgeorg in bug report #2820006
	{
		"playtoons4",
		"",
		{
			{"playtoon.stk", 0, "9e513e993a5b0e2496add3f50c08764b", 30448506},
			{"manda.stk", 0, "69a79c9f61b2618e482726f2ff68078d", 6499208},
			{0, 0, 0, 0}
		},
		EN_ANY,
		kPlatformPC,
		ADGF_NO_FLAGS,
		GUIO3(GUIO_NOSUBTITLES, GUIO_NOSPEECH, GUIO_NOASPECT)
	},
	kGameTypePlaytoons,
	kFeatures640x480,
	"intro2.stk", 0, 0
},

// -- Playtoons 5: The Stone of Wakan --

{
	{
		"playtoons5",
		"",
		{
			{"playtoon.stk", 0, "55f0293202963854192e39474e214f5f", 30448474},
			{"wakan.stk", 0, "f493bf82851bc5ba74d57de6b7e88df8", 5520153},
			{0, 0, 0, 0}
		},
		FR_FRA,
		kPlatformPC,
		ADGF_NO_FLAGS,
		GUIO3(GUIO_NOSUBTITLES, GUIO_NOSPEECH, GUIO_NOASPECT)
	},
	kGameTypePlaytoons,
	kFeatures640x480,
	"intro2.stk", 0, 0
},

// -- Playtoons Construction Kit 1: Monsters --

{
	{
		"playtnck1",
		"",
		{
			{"playtoon.stk", 0, "5f9aae29265f1f105ad8ec195dff81de", 68382024},
			{"dan.itk", 0, "906d67b3e438d5e95ec7ea9e781a94f3", 3000320},
			{0, 0, 0, 0}
		},
		FR_FRA,
		kPlatformPC,
		ADGF_NO_FLAGS,
		GUIO3(GUIO_NOSUBTITLES, GUIO_NOSPEECH, GUIO_NOASPECT)
	},
	kGameTypePlaytoons,
	kFeatures640x480,
	"intro2.stk", 0, 0
},

// -- Playtoons Construction Kit 2: Knights --

{
	{
		"playtnck2",
		"",
		{
			{"playtoon.stk", 0, "5f9aae29265f1f105ad8ec195dff81de", 68382024},
			{"dan.itk", 0, "74eeb075bd2cb47b243349730264af01", 3213312},
			{0, 0, 0, 0}
		},
		FR_FRA,
		kPlatformPC,
		ADGF_NO_FLAGS,
		GUIO3(GUIO_NOSUBTITLES, GUIO_NOSPEECH, GUIO_NOASPECT)
	},
	kGameTypePlaytoons,
	kFeatures640x480,
	"intro2.stk", 0, 0
},

// -- Playtoons Construction Kit 3: Far West --

{
	{
		"playtnck3",
		"",
		{
			{"playtoon.stk", 0, "5f9aae29265f1f105ad8ec195dff81de", 68382024},
			{"dan.itk", 0, "9a8f62809eca5a52f429b5b6a8e70f8f", 2861056},
			{0, 0, 0, 0}
		},
		FR_FRA,
		kPlatformPC,
		ADGF_NO_FLAGS,
		GUIO3(GUIO_NOSUBTITLES, GUIO_NOSPEECH, GUIO_NOASPECT)
	},
	kGameTypePlaytoons,
	kFeatures640x480,
	"intro2.stk", 0, 0
},

// -- Bambou le sauveur de la jungle --

{
	{
		"bambou",
		"",
		{
			{"intro.stk", 0, "2f8db6963ff8d72a8331627ebda918f4", 3613238},
			{"bambou.itk", 0, "0875914d31126d0749313428f10c7768", 114440192},
			{0, 0, 0, 0}
		},
		FR_FRA,
		kPlatformPC,
		ADGF_NO_FLAGS,
		GUIO3(GUIO_NOSUBTITLES, GUIO_NOSPEECH, GUIO_NOASPECT)
	},
	kGameTypeBambou,
	kFeatures640x480,
	"intro.stk", "intro.tot", 0
},

#endif // GOB_DETECTION_TABLES_PLAYTOONS_H
