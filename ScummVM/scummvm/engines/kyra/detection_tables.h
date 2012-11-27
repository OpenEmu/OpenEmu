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
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */

namespace {

#define FLAGS(x, y, z, a, b, c, d, e, id) { Common::UNK_LANG, Common::UNK_LANG, Common::UNK_LANG, Common::kPlatformUnknown, x, y, z, a, b, c, d, e, id }
#define FLAGS_FAN(fanLang, repLang, x, y, z, a, b, c, d, e, id) { Common::UNK_LANG, fanLang, repLang, Common::kPlatformUnknown, x, y, z, a, b, c, d, e, id }

#define KYRA1_FLOPPY_FLAGS FLAGS(false, false, false, false, false, false, false, false, Kyra::GI_KYRA1)
#define KYRA1_FLOPPY_CMP_FLAGS FLAGS(false, false, false, false, false, false, false, true, Kyra::GI_KYRA1)
#define KYRA1_OLDFLOPPY_FLAGS FLAGS(false, false, false, true, false, false, false, false, Kyra::GI_KYRA1)
#define KYRA1_AMIGA_FLAGS FLAGS(false, false, false, false, false, false, false, false, Kyra::GI_KYRA1)
#define KYRA1_TOWNS_FLAGS FLAGS(false, true, false, false, false, false, false, false, Kyra::GI_KYRA1)
#define KYRA1_TOWNS_SJIS_FLAGS FLAGS(false, true, false, false, true, false, false, false, Kyra::GI_KYRA1)
#define KYRA1_CD_FLAGS FLAGS(false, true, true, false, false, false, false, false, Kyra::GI_KYRA1)
#define KYRA1_DEMO_FLAGS FLAGS(true, false, false, false, false, false, false, false, Kyra::GI_KYRA1)
#define KYRA1_DEMO_CD_FLAGS FLAGS(true, true, true, false, false, false, false, false, Kyra::GI_KYRA1)

#define KYRA2_FLOPPY_FLAGS FLAGS(false, false, false, false, false, false, false, false, Kyra::GI_KYRA2)
#define KYRA2_FLOPPY_CMP_FLAGS FLAGS(false, false, false, false, false, false, false, true, Kyra::GI_KYRA2)
#define KYRA2_FLOPPY_FAN_FLAGS(x, y) FLAGS_FAN(x, y, false, false, false, false, false, false, false, false, Kyra::GI_KYRA2)
#define KYRA2_CD_FLAGS FLAGS(false, false, true, false, false, false, false, false, Kyra::GI_KYRA2)
#define KYRA2_CD_FAN_FLAGS(x, y) FLAGS_FAN(x, y, false, false, true, false, false, false, false, false, Kyra::GI_KYRA2)
#define KYRA2_CD_DEMO_FLAGS FLAGS(true, false, true, false, false, false, false, false, Kyra::GI_KYRA2)
#define KYRA2_DEMO_FLAGS FLAGS(true, false, false, false, false, false, false, false, Kyra::GI_KYRA2)
#define KYRA2_TOWNS_FLAGS FLAGS(false, false, false, false, false, false, false, false, Kyra::GI_KYRA2)
#define KYRA2_TOWNS_SJIS_FLAGS FLAGS(false, false, false, false, true, false, false, false, Kyra::GI_KYRA2)

#define KYRA3_CD_FLAGS FLAGS(false, false, true, false, false, false, true, true, Kyra::GI_KYRA3)
#define KYRA3_CD_INS_FLAGS FLAGS(false, false, true, false, false, false, true, false, Kyra::GI_KYRA3)
#define KYRA3_CD_FAN_FLAGS(x, y) FLAGS_FAN(x, y, false, false, true, false, false, false, true, false, Kyra::GI_KYRA3)

#define LOL_CD_FLAGS FLAGS(false, false, true, false, false, false, false, false, Kyra::GI_LOL)
#define LOL_CD_FAN_FLAGS(x, y) FLAGS_FAN(x, y, false, false, true, false, false, false, false, false, Kyra::GI_LOL)
#define LOL_FLOPPY_FLAGS FLAGS(false, false, false, false, false, false, false, false, Kyra::GI_LOL)
#define LOL_FLOPPY_FAN_FLAGS(x, y) FLAGS_FAN(x, y, false, false, false, false, false, false, false, false, Kyra::GI_LOL)
#define LOL_FLOPPY_CMP_FLAGS FLAGS(false, false, false, false, false, false, false, true, Kyra::GI_LOL)
#define LOL_PC98_SJIS_FLAGS FLAGS(false, false, false, false, true, true, false, false, Kyra::GI_LOL)
#define LOL_DEMO_FLAGS FLAGS(true, true, false, false, false, false, false, false, Kyra::GI_LOL)
#define LOL_KYRA2_DEMO_FLAGS FLAGS(true, false, false, false, false, false, false, false, Kyra::GI_KYRA2)

#define EOB_FLAGS FLAGS(false, false, false, false, false, false, false, false, Kyra::GI_EOB1)
#define EOB2_FLAGS FLAGS(false, false, false, false, false, false, false, false, Kyra::GI_EOB2)

#define GAMEOPTION_KYRA3_AUDIENCE GUIO_GAMEOPTIONS1
#define GAMEOPTION_KYRA3_SKIP     GUIO_GAMEOPTIONS2
#define GAMEOPTION_KYRA3_HELIUM   GUIO_GAMEOPTIONS3

#define GAMEOPTION_LOL_SCROLLING  GUIO_GAMEOPTIONS4
#define GAMEOPTION_LOL_CURSORS    GUIO_GAMEOPTIONS5

#define GAMEOPTION_EOB_HPGRAPHS   GUIO_GAMEOPTIONS6

const KYRAGameDescription adGameDescs[] = {
	/* disable these targets until they get supported
	{
		{
			"kyra1",
			0,
			AD_ENTRY1("DISK1.EXE", "c8641d0414d6c966d0a3dad79db07bf4"),
			Common::EN_ANY,
			Common::kPlatformPC,
			ADGF_NO_FLAGS,
			GUIO4(GUIO_NOSPEECH, GUIO_MIDIADLIB, GUIO_MIDIMT32, GUIO_MIDIPCSPK)
		},
		KYRA1_FLOPPY_CMP_FLAGS
	},

	{
		{
			"kyra1",
			0,
			AD_ENTRY1("DISK1.EXE", "5d5cee4c3d0b68d586788b74243d254a"),
			Common::DE_DEU,
			Common::kPlatformPC,
			ADGF_NO_FLAGS,
			GUIO4(GUIO_NOSPEECH, GUIO_MIDIADLIB, GUIO_MIDIMT32, GUIO_MIDIPCSPK)
		},
		KYRA1_FLOPPY_CMP_FLAGS
	},
	*/

	{
		{
			"kyra1",
			"Extracted",
			AD_ENTRY1("GEMCUT.EMC", "3c244298395520bb62b5edfe41688879"),
			Common::EN_ANY,
			Common::kPlatformPC,
			ADGF_NO_FLAGS,
			GUIO5(GUIO_NOSPEECH, GUIO_MIDIADLIB, GUIO_MIDIMT32, GUIO_MIDIPCSPK, GUIO_RENDERVGA)
		},
		KYRA1_FLOPPY_FLAGS
	},

	{
		{
			"kyra1",
			"Extracted",
			AD_ENTRY1("GEMCUT.EMC", "796e44863dd22fa635b042df1bf16673"),
			Common::EN_ANY,
			Common::kPlatformPC,
			ADGF_NO_FLAGS,
			GUIO5(GUIO_NOSPEECH, GUIO_MIDIADLIB, GUIO_MIDIMT32, GUIO_MIDIPCSPK, GUIO_RENDERVGA)
		},
		KYRA1_FLOPPY_FLAGS
	},
	{
		{
			"kyra1",
			"Extracted",
			AD_ENTRY1("GEMCUT.EMC", "abf8eb360e79a6c2a837751fbd4d3d24"),
			Common::FR_FRA,
			Common::kPlatformPC,
			ADGF_NO_FLAGS,
			GUIO5(GUIO_NOSPEECH, GUIO_MIDIADLIB, GUIO_MIDIMT32, GUIO_MIDIPCSPK, GUIO_RENDERVGA)
		},
		KYRA1_FLOPPY_FLAGS
	},
	{
		{
			"kyra1",
			"Extracted",
			AD_ENTRY1("GEMCUT.EMC", "6018e1dfeaca7fe83f8d0b00eb0dd049"),
			Common::DE_DEU,
			Common::kPlatformPC,
			ADGF_NO_FLAGS,
			GUIO5(GUIO_NOSPEECH, GUIO_MIDIADLIB, GUIO_MIDIMT32, GUIO_MIDIPCSPK, GUIO_RENDERVGA)
		},
		KYRA1_FLOPPY_FLAGS
	},
	{ // from Arne.F
		{
			"kyra1",
			"Extracted",
			AD_ENTRY1("GEMCUT.EMC", "f0b276781f47c130f423ec9679fe9ed9"),
			Common::DE_DEU,
			Common::kPlatformPC,
			ADGF_NO_FLAGS,
			GUIO5(GUIO_NOSPEECH, GUIO_MIDIADLIB, GUIO_MIDIMT32, GUIO_MIDIPCSPK, GUIO_RENDERVGA)
		},
		KYRA1_FLOPPY_FLAGS
	},
	{ // russian fan translation
		{
			"kyra1",
			"Extracted",
			AD_ENTRY1("GEMCUT.EMC", "689b62b7519215c1b2571d466c95624c"),
			Common::RU_RUS,
			Common::kPlatformPC,
			ADGF_NO_FLAGS,
			GUIO5(GUIO_NOSPEECH, GUIO_MIDIADLIB, GUIO_MIDIMT32, GUIO_MIDIPCSPK, GUIO_RENDERVGA)
		},
		KYRA1_OLDFLOPPY_FLAGS
	},
	{ // from VooD
		{
			"kyra1",
			"Extracted",
			AD_ENTRY1("GEMCUT.EMC", "8909b41596913b3f5deaf3c9f1017b01"),
			Common::ES_ESP,
			Common::kPlatformPC,
			ADGF_NO_FLAGS,
			GUIO5(GUIO_NOSPEECH, GUIO_MIDIADLIB, GUIO_MIDIMT32, GUIO_MIDIPCSPK, GUIO_RENDERVGA)
		},
		KYRA1_FLOPPY_FLAGS
	},
	{ // floppy 1.8 from clemmy
		{
			"kyra1",
			"Extracted",
			AD_ENTRY1("GEMCUT.EMC", "747861d2a9c643c59fdab570df5b9093"),
			Common::ES_ESP,
			Common::kPlatformPC,
			ADGF_NO_FLAGS,
			GUIO5(GUIO_NOSPEECH, GUIO_MIDIADLIB, GUIO_MIDIMT32, GUIO_MIDIPCSPK, GUIO_RENDERVGA)
		},
		KYRA1_FLOPPY_FLAGS
	},
	{ // from gourry
		{
			"kyra1",
			"Extracted",
			AD_ENTRY1("GEMCUT.EMC", "ef08c8c237ee1473fd52578303fc36df"),
			Common::IT_ITA,
			Common::kPlatformPC,
			ADGF_NO_FLAGS,
			GUIO5(GUIO_NOSPEECH, GUIO_MIDIADLIB, GUIO_MIDIMT32, GUIO_MIDIPCSPK, GUIO_RENDERVGA)
		},
		KYRA1_FLOPPY_FLAGS
	},

	{
		{
			"kyra1",
			0,
			{
				{ "GEMCUT.PAK", 0, "2bd1da653eaefd691e050e4a9eb68a64", -1 },
				{ "GEMCUT.EMC", 0, "2a3f44e179f1e9f7643e90083c747571", -1 },
				{ NULL, 0, NULL, 0 }
			},
			Common::EN_ANY,
			Common::kPlatformAmiga,
			ADGF_NO_FLAGS,
			GUIO3(GUIO_NOSPEECH, GUIO_MIDIAMIGA, GUIO_RENDERAMIGA)
		},
		KYRA1_AMIGA_FLAGS
	},

	{
		{
			"kyra1",
			0,
			{
				{ "GEMCUT.PAK", 0, "2bd1da653eaefd691e050e4a9eb68a64", -1 },
				{ "GEMCUT.EMC", 0, "74f99e9ed99abf8d0429826d78485a2a", -1 },
				{ NULL, 0, NULL, 0 }
			},
			Common::DE_DEU,
			Common::kPlatformAmiga,
			ADGF_NO_FLAGS,
			GUIO3(GUIO_NOSPEECH, GUIO_MIDIAMIGA, GUIO_RENDERAMIGA)
		},
		KYRA1_AMIGA_FLAGS
	},

	{
		{
			"kyra1",
			0,
			{
				{ "GEMCUT.EMC", 0, "796e44863dd22fa635b042df1bf16673", -1 },
				{ "BEAD.CPS", 0, "3038466f65b7751451844707187aa401", -1 },
				{ NULL, 0, NULL, 0 }
			},
			Common::EN_ANY,
			Common::kPlatformMacintosh,
			ADGF_NO_FLAGS,
			GUIO3(GUIO_NOSPEECH, GUIO_MIDIGM, GUIO_RENDERVGA)
		},
		KYRA1_FLOPPY_FLAGS
	},

	{ // FM-TOWNS version
		{
			"kyra1",
			0,
			{
				{ "EMC.PAK", 0, "a046bb0b422061aab8e4c4689400343a", -1 },
				{ "TWMUSIC.PAK", 0, "e53bca3a3e3fb49107d59463ec387a59", -1 },
				{ NULL, 0, NULL, 0 }
			},
			Common::EN_ANY,
			Common::kPlatformFMTowns,
			ADGF_NO_FLAGS,
			GUIO3(GUIO_NOSPEECH, GUIO_MIDITOWNS, GUIO_RENDERFMTOWNS)
		},
		KYRA1_TOWNS_FLAGS
	},
	{
		{
			"kyra1",
			0,
			{
				{ "JMC.PAK", 0, "9c5707a2a478e8167e44283246612d2c", -1 },
				{ "TWMUSIC.PAK", 0, "e53bca3a3e3fb49107d59463ec387a59", -1 },
				{ NULL, 0, NULL, 0 }
			},
			Common::JA_JPN,
			Common::kPlatformFMTowns,
			ADGF_NO_FLAGS,
			GUIO3(GUIO_NOSPEECH, GUIO_MIDITOWNS, GUIO_RENDERFMTOWNS)
		},
		KYRA1_TOWNS_SJIS_FLAGS
	},

	// PC-9801 floppy + CD / PC-9821 floppy version are all using the same data files,
	// thus we will mark it as non CD game.
	{
		{
			"kyra1",
			"",
			{
				{ "JMC.PAK", 0, "9c5707a2a478e8167e44283246612d2c", -1 },
				{ "MUSIC98.PAK", 0, "02fc212f799331b769b274e33d87b37f", -1 },
				{ NULL, 0, NULL, 0 }
			},
			Common::JA_JPN,
			Common::kPlatformPC98,
			ADGF_NO_FLAGS,
			GUIO4(GUIO_NOSPEECH, GUIO_MIDIPC98, GUIO_RENDERPC9821, GUIO_RENDERPC9801)
		},
		KYRA1_TOWNS_SJIS_FLAGS
	},

	{
		{
			"kyra1",
			"CD",
			AD_ENTRY1("GEMCUT.PAK", "fac399fe62f98671e56a005c5e94e39f"),
			Common::EN_ANY,
			Common::kPlatformPC,
			ADGF_CD,
			GUIO4(GUIO_MIDIADLIB, GUIO_MIDIMT32, GUIO_MIDIPCSPK, GUIO_RENDERVGA)
		},
		KYRA1_CD_FLAGS
	},
	{
		{
			"kyra1",
			"CD",
			AD_ENTRY1("GEMCUT.PAK", "230f54e6afc007ab4117159181a1c722"),
			Common::DE_DEU,
			Common::kPlatformPC,
			ADGF_CD,
			GUIO4(GUIO_MIDIADLIB, GUIO_MIDIMT32, GUIO_MIDIPCSPK, GUIO_RENDERVGA)
		},
		KYRA1_CD_FLAGS
	},
	{
		{
			"kyra1",
			"CD",
			AD_ENTRY1("GEMCUT.PAK", "b037c41768b652a040360ffa3556fd2a"),
			Common::FR_FRA,
			Common::kPlatformPC,
			ADGF_CD,
			GUIO4(GUIO_MIDIADLIB, GUIO_MIDIMT32, GUIO_MIDIPCSPK, GUIO_RENDERVGA)
		},
		KYRA1_CD_FLAGS
	},

	{ // italian fan translation see fr#1727941 "KYRA: add Italian CD Version to kyra.dat"
		{
			"kyra1",
			"CD",
			AD_ENTRY1("GEMCUT.PAK", "d8327fc4b7a72b23c900fa13aef4093a"),
			Common::IT_ITA,
			Common::kPlatformPC,
			ADGF_CD,
			GUIO4(GUIO_MIDIADLIB, GUIO_MIDIMT32, GUIO_MIDIPCSPK, GUIO_RENDERVGA)
		},
		KYRA1_CD_FLAGS
	},

	{ // Kyra 1 Mac CD as mentioned in fr #2766454 "KYRA1: Add support for Macintosh CD" by nnooiissee
		{
			"kyra1",
			"CD",
			{
				{ "GEMCUT.PAK", 0, "d3d4b281cd357230aabcec46843d04bd", -1 },
				{ "BEAD.CPS", 0, "3038466f65b7751451844707187aa401", -1 },
				{ NULL, 0, NULL, 0 }
			},
			Common::EN_ANY,
			Common::kPlatformMacintosh,
			ADGF_CD,
			GUIO0()
		},
		KYRA1_CD_FLAGS
	},
	{
		{
			"kyra1",
			"CD",
			{
				{ "GEMCUT.PAK", 0, "4a0cb720e824295bcbccbd1407652110", -1 },
				{ "BEAD.CPS", 0, "3038466f65b7751451844707187aa401", -1 },
				{ NULL, 0, NULL, 0 }
			},
			Common::DE_DEU,
			Common::kPlatformMacintosh,
			ADGF_CD,
			GUIO0()
		},
		KYRA1_CD_FLAGS
	},
	{
		{
			"kyra1",
			"CD",
			{
				{ "GEMCUT.PAK", 0, "b71ee090aa12e80ed2ba068826d92bed", -1 },
				{ "BEAD.CPS", 0, "3038466f65b7751451844707187aa401", -1 },
				{ NULL, 0, NULL, 0 }
			},
			Common::FR_FRA,
			Common::kPlatformMacintosh,
			ADGF_CD,
			GUIO0()
		},
		KYRA1_CD_FLAGS
	},

	{
		{
			"kyra1",
			"Demo",
			AD_ENTRY1("DEMO1.WSA", "fb722947d94897512b13b50cc84fd648"),
			Common::EN_ANY,
			Common::kPlatformPC,
			ADGF_DEMO,
			GUIO5(GUIO_NOSPEECH, GUIO_MIDIADLIB, GUIO_MIDIMT32, GUIO_MIDIPCSPK, GUIO_RENDERVGA)
		},
		KYRA1_DEMO_FLAGS
	},

	{	// Special Kyrandia 1 CD demo
		{
			"kyra1",
			"Demo/CD",
			AD_ENTRY1("INTRO.VRM", "e3045fb69b8c29db84b8fda3ccbdac54"),
			Common::EN_ANY,
			Common::kPlatformPC,
			ADGF_DEMO | ADGF_CD,
			GUIO4(GUIO_MIDIADLIB, GUIO_MIDIMT32, GUIO_MIDIPCSPK, GUIO_RENDERVGA)
		},
		KYRA1_DEMO_CD_FLAGS
	},

	{ // Floppy version
		{
			"kyra2",
			0,
			AD_ENTRY1("WESTWOOD.001", "3f52dda68c4f7696c8309038be9f4151"),
			Common::EN_ANY,
			Common::kPlatformPC,
			ADGF_NO_FLAGS,
			GUIO6(GUIO_NOSPEECH, GUIO_MIDIADLIB, GUIO_MIDIMT32, GUIO_MIDIGM, GUIO_MIDIPCSPK, GUIO_RENDERVGA)
		},
		KYRA2_FLOPPY_CMP_FLAGS
	},

	{ // Floppy version
		{
			"kyra2",
			0,
			AD_ENTRY1("WESTWOOD.001", "d787b9559afddfe058b84c0b3a787224"),
			Common::DE_DEU,
			Common::kPlatformPC,
			ADGF_NO_FLAGS,
			GUIO6(GUIO_NOSPEECH, GUIO_MIDIADLIB, GUIO_MIDIMT32, GUIO_MIDIGM, GUIO_MIDIPCSPK, GUIO_RENDERVGA)
		},
		KYRA2_FLOPPY_CMP_FLAGS
	},

	{ // Floppy version extracted
		{
			"kyra2",
			"Extracted",
			AD_ENTRY1("FATE.PAK", "1ba18be685ad8e5a0ab5d46a0ce4d345"),
			Common::EN_ANY,
			Common::kPlatformPC,
			ADGF_NO_FLAGS,
			GUIO6(GUIO_NOSPEECH, GUIO_MIDIADLIB, GUIO_MIDIMT32, GUIO_MIDIGM, GUIO_MIDIPCSPK, GUIO_RENDERVGA)
		},
		KYRA2_FLOPPY_FLAGS
	},

	{ // Floppy version extracted
		{
			"kyra2",
			"Extracted",
			AD_ENTRY1("FATE.PAK", "262fb69dd8e52e596c7aefc6456f7c1b"),
			Common::DE_DEU,
			Common::kPlatformPC,
			ADGF_NO_FLAGS,
			GUIO6(GUIO_NOSPEECH, GUIO_MIDIADLIB, GUIO_MIDIMT32, GUIO_MIDIGM, GUIO_MIDIPCSPK, GUIO_RENDERVGA)
		},
		KYRA2_FLOPPY_FLAGS
	},

	{ // Floppy version extracted
		{
			"kyra2",
			"Extracted",
			AD_ENTRY1("FATE.PAK", "f7de11506b4c8fdf64bc763206c3e4e7"),
			Common::FR_FRA,
			Common::kPlatformPC,
			ADGF_NO_FLAGS,
			GUIO6(GUIO_NOSPEECH, GUIO_MIDIADLIB, GUIO_MIDIMT32, GUIO_MIDIGM, GUIO_MIDIPCSPK, GUIO_RENDERVGA)
		},
		KYRA2_FLOPPY_FLAGS
	},

	{ // Floppy version extracted
		{
			"kyra2",
			"Extracted",
			AD_ENTRY1("FATE.PAK", "e0a70c31b022cb4bb3061890020fc27c"),
			Common::IT_ITA,
			Common::kPlatformPC,
			ADGF_NO_FLAGS,
			GUIO6(GUIO_NOSPEECH, GUIO_MIDIADLIB, GUIO_MIDIMT32, GUIO_MIDIGM, GUIO_MIDIPCSPK, GUIO_RENDERVGA)
		},
		KYRA2_FLOPPY_FLAGS
	},

	{ // Floppy version extracted
		{
			"kyra2",
			"Extracted",
			AD_ENTRY1("CH01-S00.DLG", "54b7a5a94f6e1ec91f0fb1311eec09ab"),
			Common::RU_RUS,
			Common::kPlatformPC,
			ADGF_NO_FLAGS,
			GUIO6(GUIO_NOSPEECH, GUIO_MIDIADLIB, GUIO_MIDIMT32, GUIO_MIDIGM, GUIO_MIDIPCSPK, GUIO_RENDERVGA)
		},
		KYRA2_FLOPPY_FAN_FLAGS(Common::RU_RUS, Common::EN_ANY)
	},

	{ // Floppy version extracted
		{
			"kyra2",
			"Extracted",
			AD_ENTRY1("CH01-S00.DLG", "7c36c0e63ab8c81cbb3ea58681331366"),
			Common::RU_RUS,
			Common::kPlatformPC,
			ADGF_NO_FLAGS,
			GUIO6(GUIO_NOSPEECH, GUIO_MIDIADLIB, GUIO_MIDIMT32, GUIO_MIDIGM, GUIO_MIDIPCSPK, GUIO_RENDERVGA)
		},
		KYRA2_FLOPPY_FAN_FLAGS(Common::RU_RUS, Common::EN_ANY)
	},

	{ // CD version
		{
			"kyra2",
			"CD",
			AD_ENTRY1("FATE.PAK", "28cbad1c5bf06b2d3825ae57d760d032"),
			Common::EN_ANY,
			Common::kPlatformPC,
			ADGF_DROPLANGUAGE | ADGF_CD,
			GUIO5(GUIO_MIDIADLIB, GUIO_MIDIMT32, GUIO_MIDIGM, GUIO_MIDIPCSPK, GUIO_RENDERVGA)
		},
		KYRA2_CD_FLAGS
	},
	{
		{
			"kyra2",
			"CD",
			AD_ENTRY1("FATE.PAK", "28cbad1c5bf06b2d3825ae57d760d032"),
			Common::DE_DEU,
			Common::kPlatformPC,
			ADGF_DROPLANGUAGE | ADGF_CD,
			GUIO5(GUIO_MIDIADLIB, GUIO_MIDIMT32, GUIO_MIDIGM, GUIO_MIDIPCSPK, GUIO_RENDERVGA)
		},
		KYRA2_CD_FLAGS
	},
	{
		{
			"kyra2",
			"CD",
			AD_ENTRY1("FATE.PAK", "28cbad1c5bf06b2d3825ae57d760d032"),
			Common::FR_FRA,
			Common::kPlatformPC,
			ADGF_DROPLANGUAGE | ADGF_CD,
			GUIO5(GUIO_MIDIADLIB, GUIO_MIDIMT32, GUIO_MIDIGM, GUIO_MIDIPCSPK, GUIO_RENDERVGA)
		},
		KYRA2_CD_FLAGS
	},

	// Italian fan translation, see fr#2003504 "KYRA: add support for Italian version of Kyrandia 2&3"
	{ // CD version
		{
			"kyra2",
			"CD",
			AD_ENTRY1("FATE.PAK", "30487f3b8d7790c7857f4769ff2dd125"),
			Common::IT_ITA,
			Common::kPlatformPC,
			ADGF_DROPLANGUAGE | ADGF_CD,
			GUIO5(GUIO_MIDIADLIB, GUIO_MIDIMT32, GUIO_MIDIGM, GUIO_MIDIPCSPK, GUIO_RENDERVGA)
		},
		KYRA2_CD_FAN_FLAGS(Common::IT_ITA, Common::EN_ANY)
	},
	{
		{
			"kyra2",
			"CD",
			AD_ENTRY1("FATE.PAK", "30487f3b8d7790c7857f4769ff2dd125"),
			Common::DE_DEU,
			Common::kPlatformPC,
			ADGF_DROPLANGUAGE | ADGF_CD,
			GUIO5(GUIO_MIDIADLIB, GUIO_MIDIMT32, GUIO_MIDIGM, GUIO_MIDIPCSPK, GUIO_RENDERVGA)
		},
		KYRA2_CD_FAN_FLAGS(Common::IT_ITA, Common::EN_ANY)
	},
	{
		{
			"kyra2",
			"CD",
			AD_ENTRY1("FATE.PAK", "30487f3b8d7790c7857f4769ff2dd125"),
			Common::FR_FRA,
			Common::kPlatformPC,
			ADGF_DROPLANGUAGE | ADGF_CD,
			GUIO5(GUIO_MIDIADLIB, GUIO_MIDIMT32, GUIO_MIDIGM, GUIO_MIDIPCSPK, GUIO_RENDERVGA)
		},
		KYRA2_CD_FAN_FLAGS(Common::IT_ITA, Common::EN_ANY)
	},

	{
		{
			"kyra2",
			"CD",
			AD_ENTRY1("FATE.PAK", "39772ff82e42c4c520050518deb82e64"),
			Common::IT_ITA,
			Common::kPlatformPC,
			ADGF_DROPLANGUAGE | ADGF_CD,
			GUIO5(GUIO_MIDIADLIB, GUIO_MIDIMT32, GUIO_MIDIGM, GUIO_MIDIPCSPK, GUIO_RENDERVGA)
		},
		KYRA2_CD_FAN_FLAGS(Common::IT_ITA, Common::EN_ANY)
	},

	{
		{
			"kyra2",
			"CD",
			AD_ENTRY1("FATE.PAK", "39772ff82e42c4c520050518deb82e64"),
			Common::DE_DEU,
			Common::kPlatformPC,
			ADGF_DROPLANGUAGE | ADGF_CD,
			GUIO5(GUIO_MIDIADLIB, GUIO_MIDIMT32, GUIO_MIDIGM, GUIO_MIDIPCSPK, GUIO_RENDERVGA)
		},
		KYRA2_CD_FAN_FLAGS(Common::IT_ITA, Common::EN_ANY)
	},

	{
		{
			"kyra2",
			"CD",
			AD_ENTRY1("FATE.PAK", "39772ff82e42c4c520050518deb82e64"),
			Common::FR_FRA,
			Common::kPlatformPC,
			ADGF_DROPLANGUAGE | ADGF_CD,
			GUIO5(GUIO_MIDIADLIB, GUIO_MIDIMT32, GUIO_MIDIGM, GUIO_MIDIPCSPK, GUIO_RENDERVGA)
		},
		KYRA2_CD_FAN_FLAGS(Common::IT_ITA, Common::EN_ANY)
	},

	// Russian fan translation
	{ // CD version
		{
			"kyra2",
			"CD",
			AD_ENTRY1("FERRY.CPS", "763e2103858347d4ffffc329910d323f"),
			Common::RU_RUS,
			Common::kPlatformPC,
			ADGF_DROPLANGUAGE | ADGF_CD,
			GUIO5(GUIO_MIDIADLIB, GUIO_MIDIMT32, GUIO_MIDIGM, GUIO_MIDIPCSPK, GUIO_RENDERVGA)
		},
		KYRA2_CD_FAN_FLAGS(Common::RU_RUS, Common::EN_ANY)
	},

	{ // Interactive Demo
		{
			"kyra2",
			"CD/Demo",
			AD_ENTRY1("THANKS.CPS", "b1a78d990b120bb2234b7094f74e30a5"),
			Common::EN_ANY,
			Common::kPlatformPC,
			ADGF_DROPLANGUAGE | ADGF_CD | ADGF_DEMO,
			GUIO5(GUIO_MIDIADLIB, GUIO_MIDIMT32, GUIO_MIDIGM, GUIO_MIDIPCSPK, GUIO_RENDERVGA)
		},
		KYRA2_CD_DEMO_FLAGS
	},

	{ // Interactive Demo
		{
			"kyra2",
			"CD/Demo",
			AD_ENTRY1("THANKS.CPS", "b1a78d990b120bb2234b7094f74e30a5"),
			Common::DE_DEU,
			Common::kPlatformPC,
			ADGF_DROPLANGUAGE | ADGF_CD | ADGF_DEMO,
			GUIO5(GUIO_MIDIADLIB, GUIO_MIDIMT32, GUIO_MIDIGM, GUIO_MIDIPCSPK, GUIO_RENDERVGA)
		},
		KYRA2_CD_DEMO_FLAGS
	},

	{ // Interactive Demo
		{
			"kyra2",
			"CD/Demo",
			AD_ENTRY1("THANKS.CPS", "b1a78d990b120bb2234b7094f74e30a5"),
			Common::FR_FRA,
			Common::kPlatformPC,
			ADGF_DROPLANGUAGE | ADGF_CD | ADGF_DEMO,
			GUIO5(GUIO_MIDIADLIB, GUIO_MIDIMT32, GUIO_MIDIGM, GUIO_MIDIPCSPK, GUIO_RENDERVGA)
		},
		KYRA2_CD_DEMO_FLAGS
	},

	{ // Non-Interactive Demos
		{
			"kyra2",
			"Demo",
			AD_ENTRY1("VOC.PAK", "ecb3561b63749158172bf21528cf5f45"),
			Common::EN_ANY,
			Common::kPlatformPC,
			ADGF_DEMO,
			GUIO5(GUIO_MIDIADLIB, GUIO_MIDIMT32, GUIO_MIDIGM, GUIO_MIDIPCSPK, GUIO_RENDERVGA)
		},
		KYRA2_DEMO_FLAGS
	},

	{ // FM-TOWNS
		{
			"kyra2",
			0,
			AD_ENTRY1("WSCORE.PAK", "c44de1302b67f27d4707409987b7a685"),
			Common::EN_ANY,
			Common::kPlatformFMTowns,
			ADGF_NO_FLAGS,
			GUIO3(GUIO_NOSPEECH, GUIO_MIDITOWNS, GUIO_RENDERFMTOWNS)
		},
		KYRA2_TOWNS_FLAGS
	},
	{
		{
			"kyra2",
			0,
			AD_ENTRY1("WSCORE.PAK", "c44de1302b67f27d4707409987b7a685"),
			Common::JA_JPN,
			Common::kPlatformFMTowns,
			ADGF_NO_FLAGS,
			GUIO3(GUIO_NOSPEECH, GUIO_MIDITOWNS, GUIO_RENDERFMTOWNS)
		},
		KYRA2_TOWNS_SJIS_FLAGS
	},
	{ // PC-9821
		{
			"kyra2",
			"CD",
			AD_ENTRY1("WSCORE.PAK", "c44de1302b67f27d4707409987b7a685"),
			Common::EN_ANY,
			Common::kPlatformPC98,
			ADGF_CD,
			GUIO3(GUIO_NOSPEECH, GUIO_MIDIPC98, GUIO_RENDERPC9821)
		},
		KYRA2_TOWNS_FLAGS
	},
	{
		{
			"kyra2",
			"CD",
			AD_ENTRY1("WSCORE.PAK", "c44de1302b67f27d4707409987b7a685"),
			Common::JA_JPN,
			Common::kPlatformPC98,
			ADGF_CD,
			GUIO3(GUIO_NOSPEECH, GUIO_MIDIPC98, GUIO_RENDERPC9821)
		},
		KYRA2_TOWNS_SJIS_FLAGS
	},

	// Kyra3

	// non installed version
	{
		{
			"kyra3",
			0,
			{
				{ "ONETIME.PAK", 0, "3833ff312757b8e6147f464cca0a6587", -1 },
				{ "WESTWOOD.001", 0, 0, -1 },
				{ 0, 0, 0, 0 }
			},
			Common::EN_ANY,
			Common::kPlatformPC,
			ADGF_DROPLANGUAGE,
			GUIO5(GUIO_NOMIDI, GUIO_RENDERVGA, GAMEOPTION_KYRA3_AUDIENCE, GAMEOPTION_KYRA3_SKIP, GAMEOPTION_KYRA3_HELIUM)
		},
		KYRA3_CD_FLAGS
	},
	{
		{
			"kyra3",
			0,
			{
				{ "ONETIME.PAK", 0, "3833ff312757b8e6147f464cca0a6587", -1 },
				{ "WESTWOOD.001", 0, 0, -1 },
				{ 0, 0, 0, 0 }
			},
			Common::DE_DEU,
			Common::kPlatformPC,
			ADGF_DROPLANGUAGE,
			GUIO5(GUIO_NOMIDI, GUIO_RENDERVGA, GAMEOPTION_KYRA3_AUDIENCE, GAMEOPTION_KYRA3_SKIP, GAMEOPTION_KYRA3_HELIUM)
		},
		KYRA3_CD_FLAGS
	},
	{
		{
			"kyra3",
			0,
			{
				{ "ONETIME.PAK", 0, "3833ff312757b8e6147f464cca0a6587", -1 },
				{ "WESTWOOD.001", 0, 0, -1 },
				{ 0, 0, 0, 0 }
			},
			Common::FR_FRA,
			Common::kPlatformPC,
			ADGF_DROPLANGUAGE,
			GUIO5(GUIO_NOMIDI, GUIO_RENDERVGA, GAMEOPTION_KYRA3_AUDIENCE, GAMEOPTION_KYRA3_SKIP, GAMEOPTION_KYRA3_HELIUM)
		},
		KYRA3_CD_FLAGS
	},

	// installed version
	{
		{
			"kyra3",
			0,
			{
				{ "ONETIME.PAK", 0, "3833ff312757b8e6147f464cca0a6587", -1 },
				{ "AUD.PAK", 0, 0, -1 },
				{ 0, 0, 0, 0 }
			},
			Common::EN_ANY,
			Common::kPlatformPC,
			ADGF_DROPLANGUAGE,
			GUIO5(GUIO_NOMIDI, GUIO_RENDERVGA, GAMEOPTION_KYRA3_AUDIENCE, GAMEOPTION_KYRA3_SKIP, GAMEOPTION_KYRA3_HELIUM)
		},
		KYRA3_CD_INS_FLAGS
	},
	{
		{
			"kyra3",
			0,
			{
				{ "ONETIME.PAK", 0, "3833ff312757b8e6147f464cca0a6587", -1 },
				{ "AUD.PAK", 0, 0, -1 },
				{ 0, 0, 0, 0 }
			},
			Common::DE_DEU,
			Common::kPlatformPC,
			ADGF_DROPLANGUAGE,
			GUIO5(GUIO_NOMIDI, GUIO_RENDERVGA, GAMEOPTION_KYRA3_AUDIENCE, GAMEOPTION_KYRA3_SKIP, GAMEOPTION_KYRA3_HELIUM)
		},
		KYRA3_CD_INS_FLAGS
	},
	{
		{
			"kyra3",
			0,
			{
				{ "ONETIME.PAK", 0, "3833ff312757b8e6147f464cca0a6587", -1 },
				{ "AUD.PAK", 0, 0, -1 },
				{ 0, 0, 0, 0 }
			},
			Common::FR_FRA,
			Common::kPlatformPC,
			ADGF_DROPLANGUAGE,
			GUIO5(GUIO_NOMIDI, GUIO_RENDERVGA, GAMEOPTION_KYRA3_AUDIENCE, GAMEOPTION_KYRA3_SKIP, GAMEOPTION_KYRA3_HELIUM)
		},
		KYRA3_CD_INS_FLAGS
	},

	// Mac version
	{
		{
			"kyra3",
			0,
			{
				{ "ONETIME.PAK", 0, "3833ff312757b8e6147f464cca0a6587", -1 },
				{ "AUD.PAK", 0, 0, -1 },
				{ 0, 0, 0, 0 }
			},
			Common::EN_ANY,
			Common::kPlatformMacintosh,
			ADGF_DROPLANGUAGE,
			GUIO5(GUIO_NOMIDI, GUIO_RENDERVGA, GAMEOPTION_KYRA3_AUDIENCE, GAMEOPTION_KYRA3_SKIP, GAMEOPTION_KYRA3_HELIUM)
		},
		KYRA3_CD_INS_FLAGS
	},
	{
		{
			"kyra3",
			0,
			{
				{ "ONETIME.PAK", 0, "3833ff312757b8e6147f464cca0a6587", -1 },
				{ "AUD.PAK", 0, 0, -1 },
				{ 0, 0, 0, 0 }
			},
			Common::DE_DEU,
			Common::kPlatformMacintosh,
			ADGF_DROPLANGUAGE,
			GUIO5(GUIO_NOMIDI, GUIO_RENDERVGA, GAMEOPTION_KYRA3_AUDIENCE, GAMEOPTION_KYRA3_SKIP, GAMEOPTION_KYRA3_HELIUM)
		},
		KYRA3_CD_INS_FLAGS
	},
	{
		{
			"kyra3",
			0,
			{
				{ "ONETIME.PAK", 0, "3833ff312757b8e6147f464cca0a6587", -1 },
				{ "AUD.PAK", 0, 0, -1 },
				{ 0, 0, 0, 0 }
			},
			Common::FR_FRA,
			Common::kPlatformMacintosh,
			ADGF_DROPLANGUAGE,
			GUIO5(GUIO_NOMIDI, GUIO_RENDERVGA, GAMEOPTION_KYRA3_AUDIENCE, GAMEOPTION_KYRA3_SKIP, GAMEOPTION_KYRA3_HELIUM)
		},
		KYRA3_CD_INS_FLAGS
	},

	// Spanish fan translation, see fr#1994040 "KYRA3: Add support for Spanish fan translation"
	{
		{
			"kyra3",
			0,
			{
				{ "ONETIME.PAK", 0, "9aaca21d2a205ca02ec53132f2911794", -1 },
				{ "AUD.PAK", 0, 0, -1 },
				{ 0, 0, 0, 0 }
			},
			Common::ES_ESP,
			Common::kPlatformPC,
			ADGF_DROPLANGUAGE,
			GUIO5(GUIO_NOMIDI, GUIO_RENDERVGA, GAMEOPTION_KYRA3_AUDIENCE, GAMEOPTION_KYRA3_SKIP, GAMEOPTION_KYRA3_HELIUM)
		},
		KYRA3_CD_FAN_FLAGS(Common::ES_ESP, Common::EN_ANY)
	},
	{
		{
			"kyra3",
			0,
			{
				{ "ONETIME.PAK", 0, "9aaca21d2a205ca02ec53132f2911794", -1 },
				{ "AUD.PAK", 0, 0, -1 },
				{ 0, 0, 0, 0 }
			},
			Common::DE_DEU,
			Common::kPlatformPC,
			ADGF_DROPLANGUAGE,
			GUIO5(GUIO_NOMIDI, GUIO_RENDERVGA, GAMEOPTION_KYRA3_AUDIENCE, GAMEOPTION_KYRA3_SKIP, GAMEOPTION_KYRA3_HELIUM)
		},
		KYRA3_CD_FAN_FLAGS(Common::ES_ESP, Common::EN_ANY)
	},
	{
		{
			"kyra3",
			0,
			{
				{ "ONETIME.PAK", 0, "9aaca21d2a205ca02ec53132f2911794", -1 },
				{ "AUD.PAK", 0, 0, -1 },
				{ 0, 0, 0, 0 }
			},
			Common::FR_FRA,
			Common::kPlatformPC,
			ADGF_DROPLANGUAGE,
			GUIO5(GUIO_NOMIDI, GUIO_RENDERVGA, GAMEOPTION_KYRA3_AUDIENCE, GAMEOPTION_KYRA3_SKIP, GAMEOPTION_KYRA3_HELIUM)
		},
		KYRA3_CD_FAN_FLAGS(Common::ES_ESP, Common::EN_ANY)
	},

	// Italian fan translation, see fr#2003504 "KYRA: add support for Italian version of Kyrandia 2&3"
	{
		{
			"kyra3",
			0,
			{
				{ "ONETIME.PAK", 0, "ee2d4d056a5de5333a3c6bda055b3cb4", -1 },
				{ "AUD.PAK", 0, 0, -1 },
				{ 0, 0, 0, 0 }
			},
			Common::EN_ANY,
			Common::kPlatformPC,
			ADGF_DROPLANGUAGE,
			GUIO5(GUIO_NOMIDI, GUIO_RENDERVGA, GAMEOPTION_KYRA3_AUDIENCE, GAMEOPTION_KYRA3_SKIP, GAMEOPTION_KYRA3_HELIUM)
		},
		KYRA3_CD_FAN_FLAGS(Common::IT_ITA, Common::FR_FRA)
	},
	{
		{
			"kyra3",
			0,
			{
				{ "ONETIME.PAK", 0, "ee2d4d056a5de5333a3c6bda055b3cb4", -1 },
				{ "AUD.PAK", 0, 0, -1 },
				{ 0, 0, 0, 0 }
			},
			Common::DE_DEU,
			Common::kPlatformPC,
			ADGF_DROPLANGUAGE,
			GUIO5(GUIO_NOMIDI, GUIO_RENDERVGA, GAMEOPTION_KYRA3_AUDIENCE, GAMEOPTION_KYRA3_SKIP, GAMEOPTION_KYRA3_HELIUM)
		},
		KYRA3_CD_FAN_FLAGS(Common::IT_ITA, Common::FR_FRA)
	},
	{
		{
			"kyra3",
			0,
			{
				{ "ONETIME.PAK", 0, "ee2d4d056a5de5333a3c6bda055b3cb4", -1 },
				{ "AUD.PAK", 0, 0, -1 },
				{ 0, 0, 0, 0 }
			},
			Common::IT_ITA,
			Common::kPlatformPC,
			ADGF_DROPLANGUAGE,
			GUIO5(GUIO_NOMIDI, GUIO_RENDERVGA, GAMEOPTION_KYRA3_AUDIENCE, GAMEOPTION_KYRA3_SKIP, GAMEOPTION_KYRA3_HELIUM)
		},
		KYRA3_CD_FAN_FLAGS(Common::IT_ITA, Common::FR_FRA)
	},

#ifdef ENABLE_LOL
	// Lands of Lore CD
	{
		{
			"lol",
			"CD",
			{
				{ "GENERAL.PAK", 0, "05a4f588fb81dc9c0ef1f2ec20d89e24", -1 },
				{ "L01.PAK", 0, "759a0ac26808d77ea968bd392355ba1d", -1 },
				{ 0, 0, 0, 0 }
			},
			Common::EN_ANY,
			Common::kPlatformPC,
			ADGF_DROPLANGUAGE | ADGF_CD,
			GUIO7(GUIO_MIDIADLIB, GUIO_MIDIMT32, GUIO_MIDIGM, GUIO_MIDIPCSPK, GUIO_RENDERVGA, GAMEOPTION_LOL_SCROLLING, GAMEOPTION_LOL_CURSORS)
		},
		LOL_CD_FLAGS
	},

	{
		{
			"lol",
			"CD",
			{
				{ "GENERAL.PAK", 0, "05a4f588fb81dc9c0ef1f2ec20d89e24", -1 },
				{ "L01.PAK", 0, "759a0ac26808d77ea968bd392355ba1d", -1 },
				{ 0, 0, 0, 0 }
			},
			Common::DE_DEU,
			Common::kPlatformPC,
			ADGF_DROPLANGUAGE | ADGF_CD,
			GUIO7(GUIO_MIDIADLIB, GUIO_MIDIMT32, GUIO_MIDIGM, GUIO_MIDIPCSPK, GUIO_RENDERVGA, GAMEOPTION_LOL_SCROLLING, GAMEOPTION_LOL_CURSORS)
		},
		LOL_CD_FLAGS
	},

	{
		{
			"lol",
			"CD",
			{
				{ "GENERAL.PAK", 0, "05a4f588fb81dc9c0ef1f2ec20d89e24", -1 },
				{ "L01.PAK", 0, "759a0ac26808d77ea968bd392355ba1d", -1 },
				{ 0, 0, 0, 0 }
			},
			Common::FR_FRA,
			Common::kPlatformPC,
			ADGF_DROPLANGUAGE | ADGF_CD,
			GUIO7(GUIO_MIDIADLIB, GUIO_MIDIMT32, GUIO_MIDIGM, GUIO_MIDIPCSPK, GUIO_RENDERVGA, GAMEOPTION_LOL_SCROLLING, GAMEOPTION_LOL_CURSORS)
		},
		LOL_CD_FLAGS
	},

	{
		{
			"lol",
			"CD",
			{
				{ "GENERAL.PAK", 0, "9e4bab499b7ea9337b91ac29fcba6d13", -1 },
				{ "L01.PAK", 0, "759a0ac26808d77ea968bd392355ba1d", -1 },
				{ 0, 0, 0, 0 }
			},
			Common::EN_ANY,
			Common::kPlatformPC,
			ADGF_DROPLANGUAGE | ADGF_CD,
			GUIO7(GUIO_MIDIADLIB, GUIO_MIDIMT32, GUIO_MIDIGM, GUIO_MIDIPCSPK, GUIO_RENDERVGA, GAMEOPTION_LOL_SCROLLING, GAMEOPTION_LOL_CURSORS)
		},
		LOL_CD_FLAGS
	},

	{
		{
			"lol",
			"CD",
			{
				{ "GENERAL.PAK", 0, "9e4bab499b7ea9337b91ac29fcba6d13", -1 },
				{ "L01.PAK", 0, "759a0ac26808d77ea968bd392355ba1d", -1 },
				{ 0, 0, 0, 0 }
			},
			Common::DE_DEU,
			Common::kPlatformPC,
			ADGF_DROPLANGUAGE | ADGF_CD,
			GUIO7(GUIO_MIDIADLIB, GUIO_MIDIMT32, GUIO_MIDIGM, GUIO_MIDIPCSPK, GUIO_RENDERVGA, GAMEOPTION_LOL_SCROLLING, GAMEOPTION_LOL_CURSORS)
		},
		LOL_CD_FLAGS
	},

	{
		{
			"lol",
			"CD",
			{
				{ "GENERAL.PAK", 0, "9e4bab499b7ea9337b91ac29fcba6d13", -1 },
				{ "L01.PAK", 0, "759a0ac26808d77ea968bd392355ba1d", -1 },
				{ 0, 0, 0, 0 }
			},
			Common::FR_FRA,
			Common::kPlatformPC,
			ADGF_DROPLANGUAGE | ADGF_CD,
			GUIO7(GUIO_MIDIADLIB, GUIO_MIDIMT32, GUIO_MIDIGM, GUIO_MIDIPCSPK, GUIO_RENDERVGA, GAMEOPTION_LOL_SCROLLING, GAMEOPTION_LOL_CURSORS)
		},
		LOL_CD_FLAGS
	},

	// Russian fan translation
	{
		{
			"lol",
			"CD",
			{
				{ "GENERAL.PAK", 0, "19354b0f464295c38c801d30588df062", -1 },
				{ "L01.PAK", 0, "174d37f21e0336c5d91020f8c58717ef", -1 },
				{ 0, 0, 0, 0 }
			},
			Common::EN_ANY,
			Common::kPlatformPC,
			ADGF_DROPLANGUAGE | ADGF_CD,
			GUIO7(GUIO_MIDIADLIB, GUIO_MIDIMT32, GUIO_MIDIGM, GUIO_MIDIPCSPK, GUIO_RENDERVGA, GAMEOPTION_LOL_SCROLLING, GAMEOPTION_LOL_CURSORS)
		},
		LOL_CD_FAN_FLAGS(Common::RU_RUS, Common::DE_DEU)
	},

	{
		{
			"lol",
			"CD",
			{

				{ "GENERAL.PAK", 0, "19354b0f464295c38c801d30588df062", -1 },
				{ "L01.PAK", 0, "174d37f21e0336c5d91020f8c58717ef", -1 },
				{ 0, 0, 0, 0 }
			},
			Common::FR_FRA,
			Common::kPlatformPC,
			ADGF_DROPLANGUAGE | ADGF_CD,
			GUIO7(GUIO_MIDIADLIB, GUIO_MIDIMT32, GUIO_MIDIGM, GUIO_MIDIPCSPK, GUIO_RENDERVGA, GAMEOPTION_LOL_SCROLLING, GAMEOPTION_LOL_CURSORS)
		},
		LOL_CD_FAN_FLAGS(Common::RU_RUS, Common::DE_DEU)
	},

	{
		{
			"lol",
			"CD",
			{
				{ "GENERAL.PAK", 0, "19354b0f464295c38c801d30588df062", -1 },
				{ "L01.PAK", 0, "174d37f21e0336c5d91020f8c58717ef", -1 },
				{ 0, 0, 0, 0 }
			},
			Common::RU_RUS,
			Common::kPlatformPC,
			ADGF_DROPLANGUAGE | ADGF_CD,
			GUIO7(GUIO_MIDIADLIB, GUIO_MIDIMT32, GUIO_MIDIGM, GUIO_MIDIPCSPK, GUIO_RENDERVGA, GAMEOPTION_LOL_SCROLLING, GAMEOPTION_LOL_CURSORS)
		},
		LOL_CD_FAN_FLAGS(Common::RU_RUS, Common::DE_DEU)
	},

	// Italian fan translation
	{
		{
			"lol",
			"CD",
			{
				{ "GENERAL.PAK", 0, "05a4f588fb81dc9c0ef1f2ec20d89e24", -1 },
				{ "L01.PAK", 0, "898485c0eb7bb4403fdd63bf5191f37e", -1 },
				{ 0, 0, 0, 0 }
			},
			Common::IT_ITA,
			Common::kPlatformPC,
			ADGF_DROPLANGUAGE | ADGF_CD,
			GUIO7(GUIO_MIDIADLIB, GUIO_MIDIMT32, GUIO_MIDIGM, GUIO_MIDIPCSPK, GUIO_RENDERVGA, GAMEOPTION_LOL_SCROLLING, GAMEOPTION_LOL_CURSORS)
		},
		LOL_CD_FAN_FLAGS(Common::IT_ITA, Common::EN_ANY)
	},

	{
		{
			"lol",
			"CD",
			{
				{ "GENERAL.PAK", 0, "05a4f588fb81dc9c0ef1f2ec20d89e24", -1 },
				{ "L01.PAK", 0, "898485c0eb7bb4403fdd63bf5191f37e", -1 },
				{ 0, 0, 0, 0 }
			},
			Common::DE_DEU,
			Common::kPlatformPC,
			ADGF_DROPLANGUAGE | ADGF_CD,
			GUIO7(GUIO_MIDIADLIB, GUIO_MIDIMT32, GUIO_MIDIGM, GUIO_MIDIPCSPK, GUIO_RENDERVGA, GAMEOPTION_LOL_SCROLLING, GAMEOPTION_LOL_CURSORS)
		},
		LOL_CD_FAN_FLAGS(Common::IT_ITA, Common::EN_ANY)
	},

	{
		{
			"lol",
			"CD",
			{
				{ "GENERAL.PAK", 0, "05a4f588fb81dc9c0ef1f2ec20d89e24", -1 },
				{ "L01.PAK", 0, "898485c0eb7bb4403fdd63bf5191f37e", -1 },
				{ 0, 0, 0, 0 }
			},
			Common::FR_FRA,
			Common::kPlatformPC,
			ADGF_DROPLANGUAGE | ADGF_CD,
			GUIO7(GUIO_MIDIADLIB, GUIO_MIDIMT32, GUIO_MIDIGM, GUIO_MIDIPCSPK, GUIO_RENDERVGA, GAMEOPTION_LOL_SCROLLING, GAMEOPTION_LOL_CURSORS)
		},
		LOL_CD_FAN_FLAGS(Common::IT_ITA, Common::EN_ANY)
	},

	{
		{
			"lol",
			"CD",
			{
				{ "GENERAL.PAK", 0, "9e4bab499b7ea9337b91ac29fcba6d13", -1 },
				{ "L01.PAK", 0, "898485c0eb7bb4403fdd63bf5191f37e", -1 },
				{ 0, 0, 0, 0 }
			},
			Common::IT_ITA,
			Common::kPlatformPC,
			ADGF_DROPLANGUAGE | ADGF_CD,
			GUIO7(GUIO_MIDIADLIB, GUIO_MIDIMT32, GUIO_MIDIGM, GUIO_MIDIPCSPK, GUIO_RENDERVGA, GAMEOPTION_LOL_SCROLLING, GAMEOPTION_LOL_CURSORS)
		},
		LOL_CD_FAN_FLAGS(Common::IT_ITA, Common::EN_ANY)
	},

	{
		{
			"lol",
			"CD",
			{
				{ "GENERAL.PAK", 0, "9e4bab499b7ea9337b91ac29fcba6d13", -1 },
				{ "L01.PAK", 0, "898485c0eb7bb4403fdd63bf5191f37e", -1 },
				{ 0, 0, 0, 0 }
			},
			Common::DE_DEU,
			Common::kPlatformPC,
			ADGF_DROPLANGUAGE | ADGF_CD,
			GUIO7(GUIO_MIDIADLIB, GUIO_MIDIMT32, GUIO_MIDIGM, GUIO_MIDIPCSPK, GUIO_RENDERVGA, GAMEOPTION_LOL_SCROLLING, GAMEOPTION_LOL_CURSORS)
		},
		LOL_CD_FAN_FLAGS(Common::IT_ITA, Common::EN_ANY)
	},

	{
		{
			"lol",
			"CD",
			{
				{ "GENERAL.PAK", 0, "9e4bab499b7ea9337b91ac29fcba6d13", -1 },
				{ "L01.PAK", 0, "898485c0eb7bb4403fdd63bf5191f37e", -1 },
				{ 0, 0, 0, 0 }
			},
			Common::FR_FRA,
			Common::kPlatformPC,
			ADGF_DROPLANGUAGE | ADGF_CD,
			GUIO7(GUIO_MIDIADLIB, GUIO_MIDIMT32, GUIO_MIDIGM, GUIO_MIDIPCSPK, GUIO_RENDERVGA, GAMEOPTION_LOL_SCROLLING, GAMEOPTION_LOL_CURSORS)
		},
		LOL_CD_FAN_FLAGS(Common::IT_ITA, Common::EN_ANY)
	},

	{
		{
			"lol",
			0,
			{
				{ "WESTWOOD.1", 0, "c656aa9a2b4032d341e3dc8e3525b917", -1 },
				{ 0, 0, 0, 0 }
			},
			Common::EN_ANY,
			Common::kPlatformPC,
			ADGF_NO_FLAGS,
			GUIO8(GUIO_NOSPEECH, GUIO_MIDIADLIB, GUIO_MIDIMT32, GUIO_MIDIGM, GUIO_MIDIPCSPK, GUIO_RENDERVGA, GAMEOPTION_LOL_SCROLLING, GAMEOPTION_LOL_CURSORS)
		},
		LOL_FLOPPY_CMP_FLAGS
	},

	{
		{
			"lol",
			0,
			{
				{ "WESTWOOD.1", 0, "320b2828be595c491903f467094f05eb", -1 },
				{ 0, 0, 0, 0 }
			},
			Common::EN_ANY,
			Common::kPlatformPC,
			ADGF_NO_FLAGS,
			GUIO8(GUIO_NOSPEECH, GUIO_MIDIADLIB, GUIO_MIDIMT32, GUIO_MIDIGM, GUIO_MIDIPCSPK, GUIO_RENDERVGA, GAMEOPTION_LOL_SCROLLING, GAMEOPTION_LOL_CURSORS)
		},
		LOL_FLOPPY_CMP_FLAGS
	},

	{
		{
			"lol",
			0,
			{
				{ "WESTWOOD.1", 0, "3c61cb7de5b2ec452f5851f5075207ee", -1 },
				{ 0, 0, 0, 0 }
			},
			Common::DE_DEU,
			Common::kPlatformPC,
			ADGF_NO_FLAGS,
			GUIO8(GUIO_NOSPEECH, GUIO_MIDIADLIB, GUIO_MIDIMT32, GUIO_MIDIGM, GUIO_MIDIPCSPK, GUIO_RENDERVGA, GAMEOPTION_LOL_SCROLLING, GAMEOPTION_LOL_CURSORS)
		},
		LOL_FLOPPY_CMP_FLAGS
	},

	{ // French floppy version 1.20, bug #3552534 "KYRA: LOL Floppy FR version unknown"
		{
			"lol",
			0,
			{
				{ "WESTWOOD.1", 0, "43857e24d1fc6731f3b13d9ed6db8c3a", -1 },
				{ 0, 0, 0, 0 }
			},
			Common::FR_FRA,
			Common::kPlatformPC,
			ADGF_NO_FLAGS,
			GUIO8(GUIO_NOSPEECH, GUIO_MIDIADLIB, GUIO_MIDIMT32, GUIO_MIDIGM, GUIO_MIDIPCSPK, GUIO_RENDERVGA, GAMEOPTION_LOL_SCROLLING, GAMEOPTION_LOL_CURSORS)
		},
		LOL_FLOPPY_CMP_FLAGS
	},

	{
		{
			"lol",
			"Extracted",
			{
				{ "GENERAL.PAK", 0, "2aaa30e120c08af87196820e9dd4bf73", -1 },
				{ "CHAPTER7.PAK", 0, "eb92bf7ebb4e890add1233a6b0c810ff", -1 },
				{ 0, 0, 0, 0 }
			},
			Common::EN_ANY,
			Common::kPlatformPC,
			ADGF_NO_FLAGS,
			GUIO8(GUIO_NOSPEECH, GUIO_MIDIADLIB, GUIO_MIDIMT32, GUIO_MIDIGM, GUIO_MIDIPCSPK, GUIO_RENDERVGA, GAMEOPTION_LOL_SCROLLING, GAMEOPTION_LOL_CURSORS)
		},
		LOL_FLOPPY_FLAGS
	},

	{
		{
			"lol",
			"Extracted",
			{
				{ "GENERAL.PAK", 0, "0f1fabc1f67b772a30d8e05ece720ac5", -1 },
				{ "CHAPTER7.PAK", 0, "482308aba1c40ee32449b91b0c63b990", -1 },
				{ 0, 0, 0, 0 }
			},
			Common::EN_ANY,
			Common::kPlatformPC,
			ADGF_NO_FLAGS,
			GUIO8(GUIO_NOSPEECH, GUIO_MIDIADLIB, GUIO_MIDIMT32, GUIO_MIDIGM, GUIO_MIDIPCSPK, GUIO_RENDERVGA, GAMEOPTION_LOL_SCROLLING, GAMEOPTION_LOL_CURSORS)
		},
		LOL_FLOPPY_FLAGS
	},

	{
		{
			"lol",
			"Extracted",
			{
				{ "GENERAL.PAK", 0, "d119e3b57f8e5edcbb90980ca6f4215a", -1 },
				{ "CHAPTER7.PAK", 0, "71a3d3cb1554294646a389e5c345cf28", -1 },
				{ 0, 0, 0, 0 }
			},
			Common::EN_ANY,
			Common::kPlatformPC,
			ADGF_NO_FLAGS,
			GUIO8(GUIO_NOSPEECH, GUIO_MIDIADLIB, GUIO_MIDIMT32, GUIO_MIDIGM, GUIO_MIDIPCSPK, GUIO_RENDERVGA, GAMEOPTION_LOL_SCROLLING, GAMEOPTION_LOL_CURSORS)
		},
		LOL_FLOPPY_FLAGS
	},

	{
		{
			"lol",
			"Extracted",
			{
				{ "GENERAL.PAK", 0, "996e66e81054d36249907a1d8158da3d", -1 },
				{ "CHAPTER7.PAK", 0, "cabee57f00d6d84b65a732b6868a4959", -1 },
				{ 0, 0, 0, 0 }
			},
			Common::DE_DEU,
			Common::kPlatformPC,
			ADGF_NO_FLAGS,
			GUIO8(GUIO_NOSPEECH, GUIO_MIDIADLIB, GUIO_MIDIMT32, GUIO_MIDIGM, GUIO_MIDIPCSPK, GUIO_RENDERVGA, GAMEOPTION_LOL_SCROLLING, GAMEOPTION_LOL_CURSORS)
		},
		LOL_FLOPPY_FLAGS
	},

	{ // French floppy version 1.23, bug #3552534 "KYRA: LOL Floppy FR version unknown"
		{
			"lol",
			"Extracted",
			{
				{ "GENERAL.PAK", 0, "f4fd14f244bd7c7fa08d026fafe44cc5", -1 },
				{ "CHAPTER7.PAK", 0, "733e33c8444c93843dac3b683c283eaa", -1 },
				{ 0, 0, 0, 0 }
			},
			Common::FR_FRA,
			Common::kPlatformPC,
			ADGF_NO_FLAGS,
			GUIO8(GUIO_NOSPEECH, GUIO_MIDIADLIB, GUIO_MIDIMT32, GUIO_MIDIGM, GUIO_MIDIPCSPK, GUIO_RENDERVGA, GAMEOPTION_LOL_SCROLLING, GAMEOPTION_LOL_CURSORS)
		},
		LOL_FLOPPY_FLAGS
	},

	// Russian fan translation
	{
		{
			"lol",
			"Extracted",
			{
				{ "GENERAL.PAK", 0, "d8f4c1153aed2418f41f886c3fb27543", -1 },
				{ "CHAPTER7.PAK", 0, "f0b8a2fdff951738834fadc12248ac1f", -1 },
				{ 0, 0, 0, 0 }
			},
			Common::RU_RUS,
			Common::kPlatformPC,
			ADGF_NO_FLAGS,
			GUIO8(GUIO_NOSPEECH, GUIO_MIDIADLIB, GUIO_MIDIMT32, GUIO_MIDIGM, GUIO_MIDIPCSPK, GUIO_RENDERVGA, GAMEOPTION_LOL_SCROLLING, GAMEOPTION_LOL_CURSORS)
		},
		LOL_FLOPPY_FAN_FLAGS(Common::RU_RUS, Common::EN_ANY)
	},

	{
		{
			"lol",
			0,
			{
				{ "GENERAL.PAK", 0, "3fe6539b9b09084c0984eaf7170464e9", -1 },
				{ "MUS.PAK", 0, "008dc69d8cbcdb6bae30e270fab26e76", -1 },
				{ 0, 0, 0, 0 }
			},
			Common::JA_JPN,
			Common::kPlatformPC98,
			ADGF_NO_FLAGS,
			GUIO5(GUIO_NOSPEECH, GUIO_MIDIPC98, GUIO_RENDERPC9801, GAMEOPTION_LOL_SCROLLING, GAMEOPTION_LOL_CURSORS)
		},
		LOL_PC98_SJIS_FLAGS
	},

	{
		{
			"lol",
			"Demo",
			{
				{ "INTRO.PAK", 0, "4bc22a3b57f19a49212c5de58ab014d6", -1 },
				{ "INTROVOC.PAK", 0, "7e578e4f1da31c1f294e14a8e8f3cc44", -1 },
				{ 0, 0, 0, 0 }
			},
			Common::EN_ANY,
			Common::kPlatformPC,
			ADGF_DEMO,
			GUIO5(GUIO_MIDIADLIB, GUIO_MIDIMT32, GUIO_MIDIGM, GUIO_MIDIPCSPK, GUIO_RENDERVGA)
		},
		LOL_DEMO_FLAGS
	},

	{
		{
			"lol",
			"Demo",
			{
				{ "GENERAL.PAK", 0, "e94863d86c4597a2d581d05481c152ba", -1 },
				{ 0, 0, 0, 0 }
			},
			Common::EN_ANY,
			Common::kPlatformPC,
			ADGF_DEMO,
			GUIO5(GUIO_MIDIADLIB, GUIO_MIDIMT32, GUIO_MIDIGM, GUIO_MIDIPCSPK, GUIO_RENDERVGA)
		},
		LOL_DEMO_FLAGS
	},
#endif // ENABLE_LOL
#ifdef ENABLE_EOB

	{
		{
			"eob",
			0,
			{
				{ "EOBDATA3.PAK", 0, "61aff543131bd61a8b7d7dc901a8278b", -1 },
				{ 0, 0, 0, 0 }
			},
			Common::EN_ANY,
			Common::kPlatformPC,
			ADGF_TESTING,
			GUIO7(GUIO_NOSPEECH, GUIO_MIDIADLIB, GUIO_MIDIPCSPK, GUIO_RENDERVGA, GUIO_RENDEREGA, GUIO_RENDERCGA, GAMEOPTION_EOB_HPGRAPHS)
		},
		EOB_FLAGS
	},

	{
		{
			"eob",
			0,
			{
				{ "TEXT.DAT", 0, "fb59b50f97fd1806756911d986b9b2b5", -1 },
				{ 0, 0, 0, 0 }
			},
			Common::DE_DEU,
			Common::kPlatformPC,
			ADGF_TESTING,
			GUIO7(GUIO_NOSPEECH, GUIO_MIDIADLIB, GUIO_MIDIPCSPK, GUIO_RENDERVGA, GUIO_RENDEREGA, GUIO_RENDERCGA, GAMEOPTION_EOB_HPGRAPHS)
		},
		EOB_FLAGS
	},

	{
		{
			"eob2",
			0,
			{
				{ "LEVEL15.INF", 0, "10f19eab75c73d0476dc58bcf70fff7a", -1 },
				{ 0, 0, 0, 0 }
			},
			Common::EN_ANY,
			Common::kPlatformPC,
			ADGF_TESTING,
			GUIO6(GUIO_NOSPEECH, GUIO_MIDIADLIB, GUIO_MIDIPCSPK, GUIO_RENDERVGA, GUIO_RENDEREGA, GAMEOPTION_EOB_HPGRAPHS)
		},
		EOB2_FLAGS
	},

	{
		{
			"eob2",
			0,
			{
				{ "LEVEL15.INF", 0, "ce54243ad1ca4447f521340428da2c91", -1 },
				{ 0, 0, 0, 0 }
			},
			Common::DE_DEU,
			Common::kPlatformPC,
			ADGF_TESTING,
			GUIO6(GUIO_NOSPEECH, GUIO_MIDIADLIB, GUIO_MIDIPCSPK, GUIO_RENDERVGA, GUIO_RENDEREGA, GAMEOPTION_EOB_HPGRAPHS)
		},
		EOB2_FLAGS
	},
#endif // ENABLE_EOB

	{ AD_TABLE_END_MARKER, FLAGS(0, 0, 0, 0, 0, 0, 0, 0, 0) }
};

const PlainGameDescriptor gameList[] = {
	{ "kyra1", "The Legend of Kyrandia" },
	{ "kyra2", "The Legend of Kyrandia: The Hand of Fate" },
	{ "kyra3", "The Legend of Kyrandia: Malcolm's Revenge" },
#ifdef ENABLE_LOL
	{ "lol", "Lands of Lore: The Throne of Chaos" },
#endif // ENABLE_LOL
#ifdef ENABLE_EOB
	{ "eob", "Eye of the Beholder" },
	{ "eob2", "Eye of the Beholder II: The Legend of Darkmoon" },
#endif // ENABLE_EOB
	{ 0, 0 }
};

} // End of anonymous namespace
