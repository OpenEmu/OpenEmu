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

// Game detection information and MD5s

namespace Saga {

static const GameResourceDescription ITE_Resources = {
	1806,	// Scene lookup table RN
	216,	// Script lookup table RN
	3,		// Main panel
	4,		// Converse panel
	5,		// Option panel
	6,		// Main sprites
	7,		// Main panel sprites
	35,		// Main strings
	// ITE specific resources
	36,		// Actor names
	125,	// Default portraits
	// IHNM specific resources
	0,		// Option panel sprites
	0,		// Warning panel
	0,		// Warning panel sprites
	0		// Psychic profile background
};

// FIXME: Option panel should be 4 but it is an empty resource.
// Proper fix would be to not load the options panel when the demo is running
static const GameResourceDescription ITEDemo_Resources = {
	318,	// Scene lookup table RN
	146,	// Script lookup table RN
	2,		// Main panel
	3,		// Converse panel
	3,		// Option panel
	5,		// Main sprites
	6,		// Main panel sprites
	8,		// Main strings
	// ITE specific resources
	9,		// Actor names
	80,		// Default portraits
	// IHNM specific resources
	0,		// Option panel sprites
	0,		// Warning panel
	0,		// Warning panel sprites
	0		// Psychic profile background
};

static const GameResourceDescription IHNM_Resources = {
	1272,	// Scene lookup table RN
	29,		// Script lookup table RN
	9,		// Main panel
	10,		// Converse panel
	15,		// Option panel
	12,		// Main sprites
	12,		// Main panel sprites
	21,		// Main strings
	// ITE specific resources
	0,		// Actor names
	0,		// Default portraits
	// IHNM specific resources
	16,		// Option panel sprites
	17,		// Warning panel
	18,		// Warning panel sprites
	20		// Psychic profile background
};

static const GameResourceDescription IHNMDEMO_Resources = {
	286,	// Scene lookup table RN
	18,		// Script lookup table RN
	5,		// Main panel
	6,		// Converse panel
	10,		// Option panel
	7,		// Main sprites
	7,		// Main panel sprites
	16,		// Main strings
	// ITE specific resources
	0,		// Actor names
	0,		// Default portraits
	// IHNM specific resources
	11,		// Option panel sprites
	12,		// Warning panel
	13,		// Warning panel sprites
	15		// Psychic profile background
};

static const GameFontDescription ITEDEMO_GameFonts[]    = { {0}, {1} };
static const GameFontDescription ITEWINDEMO_GameFonts[] = { {2}, {0} };
static const GameFontDescription ITE_GameFonts[]        = { {2}, {0}, {1} };
static const GameFontDescription IHNMDEMO_GameFonts[]   = { {2}, {3}, {4} };
// Font 6 is kIHNMFont8, font 8 is kIHNMMainFont
static const GameFontDescription IHNMCD_GameFonts[]     = { {2}, {3}, {4}, {5}, {6}, {7}, {8} };

// Patch files. Files not found will be ignored
static const GamePatchDescription ITEPatch_Files[] = {
	{       "cave.mid", GAME_RESOURCEFILE,    9},
	{      "intro.mid", GAME_RESOURCEFILE,   10},
	{   "fvillage.mid", GAME_RESOURCEFILE,   11},
	{    "elkhall.mid", GAME_RESOURCEFILE,   12},
	{      "mouse.mid", GAME_RESOURCEFILE,   13},
	{   "darkclaw.mid", GAME_RESOURCEFILE,   14},
	{   "birdchrp.mid", GAME_RESOURCEFILE,   15},
	{   "orbtempl.mid", GAME_RESOURCEFILE,   16},
	{     "spooky.mid", GAME_RESOURCEFILE,   17},
	{    "catfest.mid", GAME_RESOURCEFILE,   18},
	{ "elkfanfare.mid", GAME_RESOURCEFILE,   19},
	{     "bcexpl.mid", GAME_RESOURCEFILE,   20},
	{   "boargtnt.mid", GAME_RESOURCEFILE,   21},
	{   "boarking.mid", GAME_RESOURCEFILE,   22},
	{   "explorea.mid", GAME_RESOURCEFILE,   23},
	{   "exploreb.mid", GAME_RESOURCEFILE,   24},
	{   "explorec.mid", GAME_RESOURCEFILE,   25},
	{   "sunstatm.mid", GAME_RESOURCEFILE,   26},
	{   "nitstrlm.mid", GAME_RESOURCEFILE,   27},
	{   "humruinm.mid", GAME_RESOURCEFILE,   28},
	{   "damexplm.mid", GAME_RESOURCEFILE,   29},
	{     "tychom.mid", GAME_RESOURCEFILE,   30},
	{     "kitten.mid", GAME_RESOURCEFILE,   31},
	{      "sweet.mid", GAME_RESOURCEFILE,   32},
	{   "brutalmt.mid", GAME_RESOURCEFILE,   33},
	{     "shiala.mid", GAME_RESOURCEFILE,   34},

	{       "wyrm.pak", GAME_RESOURCEFILE, 1529},
	{      "wyrm1.dlt", GAME_RESOURCEFILE, 1530},
	{      "wyrm2.dlt", GAME_RESOURCEFILE, 1531},
	{      "wyrm3.dlt", GAME_RESOURCEFILE, 1532},
	{      "wyrm4.dlt", GAME_RESOURCEFILE, 1533},
	{   "credit3n.dlt", GAME_RESOURCEFILE, 1796}, // PC
	{   "credit3m.dlt", GAME_RESOURCEFILE, 1796}, // Macintosh
	{   "credit4n.dlt", GAME_RESOURCEFILE, 1797}, // PC
	{   "credit4m.dlt", GAME_RESOURCEFILE, 1797}, // Macintosh
	{       "p2_a.voc", GAME_VOICEFILE,       4},
	{       "p2_a.iaf", GAME_VOICEFILE,       4},
	{             NULL,              0,       0}
};

static const GamePatchDescription ITEMacPatch_Files[] = {
	{       "wyrm.pak", GAME_RESOURCEFILE, 1529},
	{      "wyrm1.dlt", GAME_RESOURCEFILE, 1530},
	{      "wyrm2.dlt", GAME_RESOURCEFILE, 1531},
	{      "wyrm3.dlt", GAME_RESOURCEFILE, 1532},
	{      "wyrm4.dlt", GAME_RESOURCEFILE, 1533},
	{   "credit3m.dlt", GAME_RESOURCEFILE, 1796},
	{   "credit4m.dlt", GAME_RESOURCEFILE, 1797},
	{       "p2_a.iaf", GAME_VOICEFILE,       4},
	{             NULL,              0,       0}
};

static const SAGAGameDescription gameDescriptions[] = {
	// ITE Section ////////////////////////////////////////////////////////////////////////////////////////////


	// ITE Demos //////////////////////////////////////////////////////////////////////////////////////////////

#if 0
	// Note: This version is NOT supported yet
	// Based on a very early version of the engine

	// Inherit the earth - DOS Demo version
	// sound unchecked
	{
		{
			"ite",
			"Demo", // Game title
			{
	{"ite.rsc",		GAME_RESOURCEFILE,					"986c79c4d2939dbe555576529fd37932", -1},
	//{"ite.dmo",	GAME_DEMOFILE},						"0b9a70eb4e120b6f00579b46c8cae29e"
	{"scripts.rsc", GAME_SCRIPTFILE,					"d5697dd3240a3ceaddaa986c47e1a2d7", -1},
	//{"voices.rsc",	GAME_SOUNDFILE | GAME_VOICEFILE,	"c58e67c506af4ffa03fd0aac2079deb0", -1},
	{ NULL, 0, NULL, 0}
			},
			Common::EN_ANY,
			Common::kPlatformPC,
			ADGF_DEMO,
			GUIO1(GUIO_NOSPEECH)
		},
		GID_ITE, // Game id
		GF_OLD_ITE_DOS, // features
		ITE_DEFAULT_SCENE, // Starting scene number
		&ITEDemo_Resources,
		ARRAYSIZE(ITEDEMO_GameFonts),
		ITEDEMO_GameFonts,
		NULL,
	},
#endif

	// Inherit the earth - MAC Demo version
	{
		{
			"ite",
			"Demo 2",
			{
	{"ited.rsc",		GAME_RESOURCEFILE,	"addfc9d82bc2fa1f4cab23743c652c08", 1865461},
	{"scriptsd.rsc",	GAME_SCRIPTFILE,	"fded5c59b8b7c5976229f960d21e6b0b", 70083},
	{ NULL, 0, NULL, 0}
			},
			Common::EN_ANY,
			Common::kPlatformMacintosh,
			ADGF_DEMO,
			GUIO1(GUIO_NOSPEECH)
		},
		GID_ITE,
		0,
		ITE_DEFAULT_SCENE,
		&ITE_Resources,
		ARRAYSIZE(ITEWINDEMO_GameFonts),
		ITEWINDEMO_GameFonts,
		ITEMacPatch_Files,
	},


	// Inherit the earth - MAC Demo version 1
	// Non-interactive demo
	{
		{
			"ite",
			"Demo 1",
			{
	{"ited.rsc",		GAME_RESOURCEFILE,	"addfc9d82bc2fa1f4cab23743c652c08", 1131098},
	{"scriptsd.rsc",	GAME_SCRIPTFILE,	"fded5c59b8b7c5976229f960d21e6b0b", 38613},
	{ NULL, 0, NULL, 0}
			},
			Common::EN_ANY,
			Common::kPlatformMacintosh,
			ADGF_DEMO,
			GUIO1(GUIO_NOSPEECH)
		},
		GID_ITE,
		0,
		ITE_DEFAULT_SCENE,
		&ITE_Resources,
		ARRAYSIZE(ITEWINDEMO_GameFonts),
		ITEWINDEMO_GameFonts,
		ITEMacPatch_Files,
	},


	// Inherit the earth - Win32 Demo version 2/3, Linux Demo version
	// Win32 Version 3 and Linux Demo version have digital music, Win32 version 2 has MIDI music
	{
		{
			"ite",
			"Win Demo 2/3, Linux Demo",
			{
	{"ited.rsc",		GAME_RESOURCEFILE,	"3a450852cbf3c80773984d565647e6ac", 1951395},
	{"scriptsd.rsc",	GAME_SCRIPTFILE,	"3f12b67fa93e56e1a6be39d2921d80bb", 70051},
	{ NULL, 0, NULL, 0}
			},
			Common::EN_ANY,
			Common::kPlatformWindows,
			ADGF_DEMO,
			GUIO0()
		},
		GID_ITE,
		0,
		ITE_DEFAULT_SCENE,
		&ITE_Resources,
		ARRAYSIZE(ITEWINDEMO_GameFonts),
		ITEWINDEMO_GameFonts,
		ITEPatch_Files,
	},


	// Inherit the earth - Win32 Demo version 1
	// Non-interactive demo
	{
		{
			"ite",
			"Demo 1",
			{
	{"ited.rsc",		GAME_RESOURCEFILE,	"3a450852cbf3c80773984d565647e6ac", 1327323},
	{"scriptsd.rsc",	GAME_SCRIPTFILE,	"3f12b67fa93e56e1a6be39d2921d80bb", 38613},
	{ NULL, 0, NULL, 0}
			},
			Common::EN_ANY,
			Common::kPlatformWindows,
			ADGF_DEMO,
			GUIO0()
		},
		GID_ITE,
		GF_8BIT_UNSIGNED_PCM,
		ITE_DEFAULT_SCENE,
		&ITE_Resources,
		ARRAYSIZE(ITEWINDEMO_GameFonts),
		ITEWINDEMO_GameFonts,
		ITEPatch_Files,
	},


	// TODO: Add Amiga demos here (not supported yet)


	// ITE Mac versions ///////////////////////////////////////////////////////////////////////////////////////

	// Inherit the earth - MAC CD Guild version
	{
		{
			"ite",
			"CD",
			{
	{"ite resources.bin",	GAME_RESOURCEFILE | GAME_MACBINARY,	"0bd506aa887bfc7965f695c6bd28237d", -1},
	{"ite scripts.bin",		GAME_SCRIPTFILE | GAME_MACBINARY,	"af0d7a2588e09ad3ecbc5b474ea238bf", -1},
	{"ite sounds.bin",		GAME_SOUNDFILE | GAME_MACBINARY,	"441426c6bb2a517f65c7e49b57f7a345", -1},
	{"ite music.bin",		GAME_MUSICFILE_GM | GAME_MACBINARY,	"c1d20324b7cdf1650e67061b8a93251c", -1},
	//{"ite voices.bin",		GAME_VOICEFILE | GAME_MACBINARY,	"dba92ae7d57e942250fe135609708369", -1},
	{ NULL, 0, NULL, 0}
			},
			Common::EN_ANY,
			Common::kPlatformMacintosh,
			ADGF_CD,
			GUIO0()
		},
		GID_ITE,
		GF_8BIT_UNSIGNED_PCM,
		ITE_DEFAULT_SCENE,
		&ITE_Resources,
		ARRAYSIZE(ITEWINDEMO_GameFonts),
		ITEWINDEMO_GameFonts,
		NULL,
	},

	// Inherit the earth - MAC CD Wyrmkeep version
	{
		{
			"ite",
			"Wyrmkeep CD",
			{
	{"ite.rsc",						GAME_RESOURCEFILE,	"4f7fa11c5175980ed593392838523060", -1},
	{"scripts.rsc",					GAME_SCRIPTFILE,	"adf1f46c1d0589083996a7060c798ad0", -1},
	{ NULL, 0, NULL, 0}
			},
			Common::EN_ANY,
			Common::kPlatformMacintosh,
			ADGF_CD,
			GUIO0()
		},
		GID_ITE,
		0,
		ITE_DEFAULT_SCENE,
		&ITE_Resources,
		ARRAYSIZE(ITEWINDEMO_GameFonts),
		ITEWINDEMO_GameFonts,
		ITEMacPatch_Files,
	},


	// ITE PC CD versions //////////////////////////////////////////////////////////////////////////////////////

	// Inherit the earth - Wyrmkeep combined Windows/Mac/Linux CD

	// version is different from the other Wyrmkeep re-releases in that it does
	// not have any substitute files. Presumably the ite.rsc file has been
	// modified to include the Wyrmkeep changes. The resource files are little-
	// endian, except for the voice file which is big-endian.
	{
		{
			"ite",
			"Multi-OS CD Version",
			{
	{"ite.rsc",						GAME_RESOURCEFILE,					"a6433e34b97b15e64fe8214651012db9", 8927165},
	{"scripts.rsc",					GAME_SCRIPTFILE,					"a891405405edefc69c9d6c420c868b84", -1},
	{ NULL, 0, NULL, 0}
			},
			Common::EN_ANY,
			Common::kPlatformUnknown,
			ADGF_CD,
			GUIO0()
		},
		GID_ITE,
		0,
		ITE_DEFAULT_SCENE,
		&ITE_Resources,
		ARRAYSIZE(ITE_GameFonts),
		ITE_GameFonts,
		NULL,
	},

	// Inherit the earth - Italian Wyrmkeep combined Windows/Mac/Linux CD (fan translation)

	// version is different from the other Wyrmkeep re-releases in that it does
	// not have any substitute files. Presumably the ite.rsc file has been
	// modified to include the Wyrmkeep changes. The resource files are little-
	// endian, except for the voice file which is big-endian.
	{
		{
			"ite",
			"Multi-OS CD Version",
			{
	// TODO: add size for ite.rsc
	{"ite.rsc",						GAME_RESOURCEFILE,					"a6433e34b97b15e64fe8214651012db9", -1},
	{"scripts.rsc",					GAME_SCRIPTFILE,					"a891405405edefc69c9d6c420c868b84", -1},
	{ NULL, 0, NULL, 0}
			},
			Common::IT_ITA,
			Common::kPlatformUnknown,
			ADGF_CD,
			GUIO0()
		},
		GID_ITE,
		0,
		ITE_DEFAULT_SCENE,
		&ITE_Resources,
		ARRAYSIZE(ITE_GameFonts),
		ITE_GameFonts,
		NULL,
	},

	// Inherit the earth - Windows/Linux/DOS CD version
	{
		{
			"ite",
			"Windows/Linux/DOS CD Version",
			{
	{"ite.rsc",		GAME_RESOURCEFILE,	"8f4315a9bb10ec839253108a032c8b54", 8928678},
	{"scripts.rsc",	GAME_SCRIPTFILE,	"a891405405edefc69c9d6c420c868b84", -1},
	{ NULL, 0, NULL, 0}
			},
			Common::EN_ANY,
			Common::kPlatformPC,
			ADGF_CD,
			GUIO0()
		},
		GID_ITE,
		GF_EXTRA_ITE_CREDITS,
		ITE_DEFAULT_SCENE,
		&ITE_Resources,
		ARRAYSIZE(ITE_GameFonts),
		ITE_GameFonts,
		ITEPatch_Files,
	},

	// Inherit the earth - Italian Windows/Linux/DOS CD version (fan translation)
	{
		{
			"ite",
			"Windows/Linux/DOS CD Version",
			{
	{"ite.rsc",		GAME_RESOURCEFILE,	"8f4315a9bb10ec839253108a032c8b54", 8929956},
	{"scripts.rsc",	GAME_SCRIPTFILE,	"a891405405edefc69c9d6c420c868b84", 350243},
	{ NULL, 0, NULL, 0}
			},
			Common::IT_ITA,
			Common::kPlatformPC,
			ADGF_CD,
			GUIO0()
		},
		GID_ITE,
		0,
		ITE_DEFAULT_SCENE,
		&ITE_Resources,
		ARRAYSIZE(ITE_GameFonts),
		ITE_GameFonts,
		ITEPatch_Files,
	},

	// Inherit the earth - DOS CD version
	{
		{
			"ite",
			"CD Version",
			{
	{"ite.rsc",		GAME_RESOURCEFILE,	"8f4315a9bb10ec839253108a032c8b54", -1},
	{"scripts.rsc",	GAME_SCRIPTFILE,	"50a0d2d7003c926a3832d503c8534e90", -1},
	{ NULL, 0, NULL, 0}
			},
			Common::EN_ANY,
			Common::kPlatformPC,
			ADGF_CD,
			GUIO0()
		},
		GID_ITE,
		0,
		ITE_DEFAULT_SCENE,
		&ITE_Resources,
		ARRAYSIZE(ITE_GameFonts),
		ITE_GameFonts,
		ITEPatch_Files,
	},

	// Inherit the earth - DOS CD German version
	// reported by mld. Bestsellergamers cover disk
	{
		{
			"ite",
			"CD Version",
			{
	{"ite.rsc",		GAME_RESOURCEFILE,	"869fc23c8f38f575979ec67152914fee", -1},
	{"scripts.rsc",	GAME_SCRIPTFILE,	"a891405405edefc69c9d6c420c868b84", -1},
	{ NULL, 0, NULL, 0}
			},
			Common::DE_DEU,
			Common::kPlatformPC,
			ADGF_CD,
			GUIO0()
		},
		GID_ITE,
		0,
		ITE_DEFAULT_SCENE,
		&ITE_Resources,
		ARRAYSIZE(ITE_GameFonts),
		ITE_GameFonts,
		NULL,
	},


	// ITE floppy versions ////////////////////////////////////////////////////////////////////////////////////

	// Inherit the earth - German Floppy version
	{
		{
			"ite",
			"Floppy",
			{
	{"ite.rsc",		GAME_RESOURCEFILE,					"869fc23c8f38f575979ec67152914fee", -1},
	{"scripts.rsc",	GAME_SCRIPTFILE,					"516f7330f8410057b834424ea719d1ef", -1},
	{ NULL, 0, NULL, 0}
			},
			Common::DE_DEU,
			Common::kPlatformPC,
			ADGF_NO_FLAGS,
			GUIO1(GUIO_NOSPEECH)
		},
		GID_ITE,
		GF_ITE_FLOPPY,
		ITE_DEFAULT_SCENE,
		&ITE_Resources,
		ARRAYSIZE(ITE_GameFonts),
		ITE_GameFonts,
		NULL,
	},

	// Inherit the earth - Disk version
	{
		{
			"ite",
			"Floppy",
			{
	{"ite.rsc",		GAME_RESOURCEFILE,					"8f4315a9bb10ec839253108a032c8b54", 8903828},
	{"scripts.rsc",	GAME_SCRIPTFILE,					"516f7330f8410057b834424ea719d1ef", -1},
	{ NULL, 0, NULL, 0}
			},
			Common::EN_ANY,
			Common::kPlatformPC,
			ADGF_NO_FLAGS,
			GUIO1(GUIO_NOSPEECH)
		},
		GID_ITE,
		GF_ITE_FLOPPY,
		ITE_DEFAULT_SCENE,
		&ITE_Resources,
		ARRAYSIZE(ITE_GameFonts),
		ITE_GameFonts,
		ITEPatch_Files,
	},

	// Inherit the earth - Italian Disk version (fan translation)
	{
		{
			"ite",
			"Floppy",
			{
	{"ite.rsc",		GAME_RESOURCEFILE,					"8f4315a9bb10ec839253108a032c8b54", 8905106},
	{"scripts.rsc",	GAME_SCRIPTFILE,					"516f7330f8410057b834424ea719d1ef", 340726},
	{ NULL, 0, NULL, 0}
			},
			Common::IT_ITA,
			Common::kPlatformPC,
			ADGF_NO_FLAGS,
			GUIO1(GUIO_NOSPEECH)
		},
		GID_ITE,
		GF_ITE_FLOPPY,
		ITE_DEFAULT_SCENE,
		&ITE_Resources,
		ARRAYSIZE(ITE_GameFonts),
		ITE_GameFonts,
		ITEPatch_Files,
	},

	// ITE Amiga versions /////////////////////////////////////////////////////////////////////////////////////

	// TODO: Add the Amiga versions here (not supported yet)

#ifdef ENABLE_IHNM
	// IHNM Section ///////////////////////////////////////////////////////////////////////////////////////////

	// I Have No Mouth And I Must Scream - Demo version
	{
		{
			"ihnm",
			"Demo",
			{
	{"music.res",		GAME_MUSICFILE_FM,	"0439083e3dfdc51b486071d45872ae52", -1},
	{"scream.res",		GAME_RESOURCEFILE,	"46bbdc65d164ba7e89836a0935eec8e6", 16687179},
	{"scripts.res",		GAME_SCRIPTFILE,	"9626bda8978094ff9b29198bc1ed5f9a", -1},
	{"sfx.res",			GAME_SOUNDFILE,		"1c610d543f32ec8b525e3f652536f269", -1},
	{ NULL, 0, NULL, 0}
			},
			Common::EN_ANY,
			Common::kPlatformPC,
			ADGF_DEMO,
			GUIO1(GUIO_NOASPECT)
		},
		GID_IHNM,
		0,
		IHNMDEMO_DEFAULT_SCENE,
		&IHNMDEMO_Resources,
		ARRAYSIZE(IHNMDEMO_GameFonts),
		IHNMDEMO_GameFonts,
		NULL,
	},

	// I Have No Mouth And I Must Scream - English CD version
	{
		{
			"ihnm",
			"",
			{
	{"musicfm.res",	GAME_MUSICFILE_FM,					"0439083e3dfdc51b486071d45872ae52", -1},
	{"musicgm.res",	GAME_MUSICFILE_GM,					"80f875a1fb384160d1f4b27166eef583", -1},
	{"scream.res",	GAME_RESOURCEFILE,					"46bbdc65d164ba7e89836a0935eec8e6", 79211140},
	{"patch.re_",	GAME_PATCHFILE | GAME_RESOURCEFILE,	"58b79e61594779513c7f2d35509fa89e", -1},
	{"scripts.res",	GAME_SCRIPTFILE,					"be38bbc5a26be809dbf39f13befebd01", -1},
	//{"sfx.res",		GAME_SOUNDFILE,						"1c610d543f32ec8b525e3f652536f269", -1},
	// There are two English versions of the game, each one with a different sfx.res file
	// Known MD5 checksums for sfx.res in the English version of the game are
	// 1c610d543f32ec8b525e3f652536f269 and 45a9a9f5d37740be24fd2ae2edf36573
	{"sfx.res",		GAME_SOUNDFILE,						NULL, -1},
	{ NULL, 0, NULL, 0}
			},
			Common::EN_ANY,
			Common::kPlatformPC,
			ADGF_NO_FLAGS,
			GUIO1(GUIO_NOASPECT)
		},
		GID_IHNM,
		0,
		IHNM_DEFAULT_SCENE,
		&IHNM_Resources,
		ARRAYSIZE(IHNMCD_GameFonts),
		IHNMCD_GameFonts,
		NULL,
	},

	// I Have No Mouth And I Must Scream - De CD version
	// Censored CD version (without Nimdok)
	// Reported by mld. German Retail
	{
		{
			"ihnm",
			"",
			{
	{"musicfm.res",	GAME_MUSICFILE_FM,					"0439083e3dfdc51b486071d45872ae52", -1},
	{"musicgm.res",	GAME_MUSICFILE_GM,					"80f875a1fb384160d1f4b27166eef583", -1},
	{"scream.res",	GAME_RESOURCEFILE,					"c92370d400e6f2a3fc411c3729d09224", 78517884},
	{"scripts.res",	GAME_SCRIPTFILE,					"32aa01a89937520fe0ea513950117292", -1},
	{"patch.re_",	GAME_PATCHFILE | GAME_RESOURCEFILE,	"58b79e61594779513c7f2d35509fa89e", -1},
	{"sfx.res",		GAME_SOUNDFILE,						"1c610d543f32ec8b525e3f652536f269", -1},
	{ NULL, 0, NULL, 0}
			},
			Common::DE_DEU,
			Common::kPlatformPC,
			ADGF_NO_FLAGS,
			GUIO1(GUIO_NOASPECT)
		},
		GID_IHNM,
		0,
		IHNM_DEFAULT_SCENE,
		&IHNM_Resources,
		ARRAYSIZE(IHNMCD_GameFonts),
		IHNMCD_GameFonts,
		NULL,
	},

	// I Have No Mouth And I Must Scream - Sp CD version
	{
		{
			"ihnm",
			"",
			{
	{"musicfm.res",	GAME_MUSICFILE_FM,					"0439083e3dfdc51b486071d45872ae52", -1},
	{"musicgm.res",	GAME_MUSICFILE_GM,					"80f875a1fb384160d1f4b27166eef583", -1},
	{"scream.res",	GAME_RESOURCEFILE,					"c92370d400e6f2a3fc411c3729d09224", 78560025},
	{"patch.re_",	GAME_PATCHFILE | GAME_RESOURCEFILE,	"58b79e61594779513c7f2d35509fa89e", -1},
	{"scripts.res",	GAME_SCRIPTFILE,					"be38bbc5a26be809dbf39f13befebd01", -1},
	{"sfx.res",		GAME_SOUNDFILE,						"1c610d543f32ec8b525e3f652536f269", -1},
	{ NULL, 0, NULL, 0}
			},
			Common::ES_ESP,
			Common::kPlatformPC,
			ADGF_NO_FLAGS,
			GUIO1(GUIO_NOASPECT)
		},
		GID_IHNM,
		0,
		IHNM_DEFAULT_SCENE,
		&IHNM_Resources,
		ARRAYSIZE(IHNMCD_GameFonts),
		IHNMCD_GameFonts,
		NULL,
	},

	// I Have No Mouth And I Must Scream - Fr CD version
	// Censored CD version (without Nimdok)
	{
		{
			"ihnm",
			"",
			{
	{"musicfm.res",	GAME_MUSICFILE_FM,					"0439083e3dfdc51b486071d45872ae52", -1},
	{"musicgm.res",	GAME_MUSICFILE_GM,					"80f875a1fb384160d1f4b27166eef583", -1},
	{"scream.res",	GAME_RESOURCEFILE,					"c92370d400e6f2a3fc411c3729d09224", 78519324},
	{"scripts.res",	GAME_SCRIPTFILE,					"32aa01a89937520fe0ea513950117292", -1},
	{"patch.re_",	GAME_PATCHFILE | GAME_RESOURCEFILE,	"58b79e61594779513c7f2d35509fa89e", -1},
	{"sfx.res",		GAME_SOUNDFILE,						"1c610d543f32ec8b525e3f652536f269", -1},
	{ NULL, 0, NULL, 0}
			},
			Common::FR_FRA,
			Common::kPlatformPC,
			ADGF_NO_FLAGS,
			GUIO1(GUIO_NOASPECT)
		},
		GID_IHNM,
		0,
		IHNM_DEFAULT_SCENE,
		&IHNM_Resources,
		ARRAYSIZE(IHNMCD_GameFonts),
		IHNMCD_GameFonts,
		NULL,
	},

	// I Have No Mouth And I Must Scream - Italian fan CD translation
	{
		{
			"ihnm",
			"",
			{
	{"musicfm.res",	GAME_MUSICFILE_FM,					"0439083e3dfdc51b486071d45872ae52", -1},
	{"musicgm.res",	GAME_MUSICFILE_GM,					"80f875a1fb384160d1f4b27166eef583", -1},
	{"scream.res",	GAME_RESOURCEFILE,					"46bbdc65d164ba7e89836a0935eec8e6", 79211498},
	{"patch.re_",	GAME_PATCHFILE | GAME_RESOURCEFILE,	"58b79e61594779513c7f2d35509fa89e", -1},
	{"scripts.res",	GAME_SCRIPTFILE,					"be38bbc5a26be809dbf39f13befebd01", -1},
	{"sfx.res",		GAME_SOUNDFILE,						"1c610d543f32ec8b525e3f652536f269", -1},
	{ NULL, 0, NULL, 0}
			},
			Common::IT_ITA,
			Common::kPlatformPC,
			ADGF_NO_FLAGS,
			GUIO1(GUIO_NOASPECT)
		},
		GID_IHNM,
		0,
		IHNM_DEFAULT_SCENE,
		&IHNM_Resources,
		ARRAYSIZE(IHNMCD_GameFonts),
		IHNMCD_GameFonts,
		NULL,
	},
	// I Have No Mouth And I Must Scream - Mac English CD
	{
		{
			"ihnm",
			"",
			{
	{"scream.res",	GAME_RESOURCEFILE,					"46bbdc65d164ba7e89836a0935eec8e6", 79211140},
	{"patch.res",	GAME_PATCHFILE | GAME_RESOURCEFILE,	"58b79e61594779513c7f2d35509fa89e", 5038599},
	{"scripts.res",	GAME_SCRIPTFILE,					"be38bbc5a26be809dbf39f13befebd01", 493597},
	{"sfx.res",		GAME_SOUNDFILE,						"0c24f75c30c3c4cffc7bf0e6d997898b", 3082},
	{ NULL, 0, NULL, 0}
			},
			Common::EN_ANY,
			Common::kPlatformMacintosh,
			ADGF_NO_FLAGS,
			GUIO1(GUIO_NOASPECT)
		},
		GID_IHNM,
		0,
		IHNM_DEFAULT_SCENE,
		&IHNM_Resources,
		ARRAYSIZE(IHNMCD_GameFonts),
		IHNMCD_GameFonts,
		NULL,
	},
#endif

#ifdef ENABLE_SAGA2

	// Dinotopia Section //////////////////////////////////////////////////////////////////////////////////////
	{
		{
			"dino",
			"",
			{
	{"dinodata.hrs", GAME_RESOURCEFILE,				    "45aa7026d441dd69957385c25d2fd33e", 2698},
	{"dino.hrs",     GAME_OBJRESOURCEFILE,				"7cf3665887a4a18e2fff6938a954c050", 66018},
	{"scripts.hrs",	 GAME_SCRIPTFILE,					"d405841a249a365cf92fc65dd52fb953", 164181},
	{"dinosnd.hrs",  GAME_SOUNDFILE | GAME_VOICEFILE,	"dcf4ade416614b8a64f99dacfd3bd071", 199163997},
	{"dinoimag.hrs", GAME_IMAGEFILE,					"d24d80676f7afcfaca0b61c95056044f", 42342931},
	{ NULL, 0, NULL, 0}
			},
			Common::EN_ANY,
			Common::kPlatformPC,
			ADGF_UNSTABLE,
			GUIO1(GUIO_NOASPECT)
		},
		GID_DINO,
		0,
		// TODO: fill out the data below
		-1,		// default scene
		NULL,	// game resources
		0,		// number of fonts
		NULL,	// font array
		NULL,
	},

	// Faery Tale Adventure II: Halls of the Dead Section /////////////////////////////////////////////////////

	// Faery Tale Adventure II: Halls of the Dead - English CD version
	{
		{
			"fta2",
			"",
			{
	{"ftadata.hrs",	 GAME_RESOURCEFILE,					"6dc3cbed8df2ddd9f060a7dba4d33ef8", 15042},
	{"fta.hrs",	     GAME_OBJRESOURCEFILE,				"c34a5ee5f1a14a87712a35c4a209a033", 213376},
	{"scripts.hrs",	 GAME_SCRIPTFILE,					"95f33928f6c4f02ee04d2ec5c3314c30", 1041948},
	{"ftasound.hrs", GAME_SOUNDFILE,					"ce930cb38922e6a03461f55d51b4e165", 12403350},
	{"ftaimage.hrs", GAME_IMAGEFILE,					"09bb003733b20f924e2e373d2ddcd394", 21127397},
	{ NULL, 0, NULL, 0}
			},
			Common::EN_ANY,
			Common::kPlatformPC,
			ADGF_UNSTABLE,
			GUIO1(GUIO_NOASPECT)
		},
		GID_FTA2,
		0,
		// TODO: fill out the data below
		-1,		// default scene
		NULL,	// game resources
		0,		// number of fonts
		NULL,	// font array
		NULL,
	},

#endif

	{ AD_TABLE_END_MARKER, 0, 0, 0, NULL, 0, NULL, NULL }
};

} // End of namespace Saga
