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

namespace AGOS {

static const AGOSGameDescription gameDescriptions[] = {
	// Personal Nightmare 1.1 - English Amiga
	{
		{
			"pn",
			"Floppy",

			{
	{ "icon.tmp",		GAME_ICONFILE,	"cd94091218ac2c46918fd3c0cbd81d5e", -1},
	{ "night.dbm",		GAME_BASEFILE,	"712c445d8e938956403a759978eab01b", -1},
	{ "night.txt",		GAME_TEXTFILE,	"52630ad100f473a2cdc7c699536d6730", -1},
	{ NULL, 0, NULL, 0}
			},
			Common::EN_ANY,
			Common::kPlatformAmiga,
			ADGF_NO_FLAGS,
			GUIO2(GUIO_NOSPEECH, GUIO_NOMIDI)
		},

		GType_PN,
		GID_PN,
		GF_OLD_BUNDLE | GF_CRUNCHED | GF_PLANAR
	},

	// Personal Nightmare - English Atari ST Floppy Demo
	{
		{
			"pn",
			"Non-Interactive Demo",

			{
	{ "01.IN",		GAME_VGAFILE,	"23a4c8c4c9ac460fee7281080b5274e3", 756},
	{ "02.IN",		GAME_VGAFILE,	"31be87808826538f0c0caebd5fedd48f", 73100},
	{ "03.IN",		GAME_VGAFILE,	"0e125f3df4e4b800936ebdcc8dc96060", 101664},
	{ NULL, 0, NULL, 0}
			},
			Common::EN_ANY,
			Common::kPlatformAtariST,
			ADGF_DEMO,
			GUIO2(GUIO_NOSPEECH, GUIO_NOMIDI)
		},

		GType_PN,
		GID_PN,
		GF_OLD_BUNDLE | GF_CRUNCHED | GF_PLANAR | GF_DEMO
	},

	// Personal Nightmare 1.1 - English AtariST Floppy
	{
		{
			"pn",
			"Floppy",

			{
	{ "night.dbm",		GAME_BASEFILE,	"712c445d8e938956403a759978eab01b", -1},
	{ "night.txt",		GAME_TEXTFILE,	"52630ad100f473a2cdc7c699536d6730", -1},
	{ "test.prg",		GAME_ICONFILE,	"0f316bbd7341a98c8d5f27d16fe50524", -1},
	{ NULL, 0, NULL, 0}
			},
			Common::EN_ANY,
			Common::kPlatformAtariST,
			ADGF_NO_FLAGS,
			GUIO2(GUIO_NOSPEECH, GUIO_NOMIDI)
		},

		GType_PN,
		GID_PN,
		GF_OLD_BUNDLE | GF_CRUNCHED | GF_PLANAR
	},

	// Personal Nightmare 1.1c - EGA English DOS Floppy
	{
		{
			"pn",
			"Floppy",

			{
	{ "icon.out",		GAME_ICONFILE,	"40d8347c3154bfa8b642d6860a4b9481", -1},
	{ "night.dbm",		GAME_BASEFILE,	"177311ae059243f6a2740e950585d786", -1},
	{ "night.txt",		GAME_TEXTFILE,	"861fc1fa0864eef585f5865dee52e325", -1},
	{ NULL, 0, NULL, 0}
			},
			Common::EN_ANY,
			Common::kPlatformPC,
			ADGF_NO_FLAGS,
			GUIO1(GUIO_NOSPEECH)
		},

		GType_PN,
		GID_PN,
		GF_OLD_BUNDLE | GF_CRUNCHED | GF_EGA | GF_PLANAR
	},

	// Elvira 1 - English Amiga Floppy Demo
	{
		{
			"elvira1",
			"Non-Interactive Demo",

			{
	{ "agos.mdf",		GAME_MENUFILE,	"825bc8eecd599f4c26732902ba2c2c77", 98},
	{ "englishdemo",	GAME_BASEFILE,	"7bbc2dfe8619ef579004ff57674c6e92", 21587},
	{ "icon.dat",		GAME_ICONFILE,	"68b329da9893e34099c7d8ad5cb9c940", 1},
	{ NULL, 0, NULL, 0}
			},
			Common::EN_ANY,
			Common::kPlatformAmiga,
			ADGF_DEMO,
			GUIO2(GUIO_NOSPEECH, GUIO_NOMIDI)
		},

		GType_ELVIRA1,
		GID_ELVIRA1,
		GF_OLD_BUNDLE | GF_CRUNCHED | GF_PLANAR | GF_DEMO
	},

	// Elvira 1 - English Amiga Floppy
	{
		{
			"elvira1",
			"Floppy",
			{
	{ "gameamiga",		GAME_BASEFILE,	"7bdaff4a118d8035047cf9b1393b3fa0", 218977},
	{ "icon.dat",		GAME_ICONFILE,	"2db931e84f1ca01f0816dddfae3f49e1", 36573},
	{ NULL, 0, NULL, 0}
			},
			Common::EN_ANY,
			Common::kPlatformAmiga,
			ADGF_NO_FLAGS,
			GUIO2(GUIO_NOSPEECH, GUIO_NOMIDI)
		},

		GType_ELVIRA1,
		GID_ELVIRA1,
		GF_OLD_BUNDLE | GF_CRUNCHED | GF_PLANAR
	},

	// Elvira 1 - French Amiga Floppy
	{
		{
			"elvira1",
			"Floppy",
			{
	{ "gameamiga",		GAME_BASEFILE,	"ab1a0798f74e71cc58a06e7e0db6f8a7", -1},
	{ "icon.dat",		GAME_ICONFILE,	"2db931e84f1ca01f0816dddfae3f49e1", -1},
	{ NULL, 0, NULL, 0}
			},
			Common::FR_FRA,
			Common::kPlatformAmiga,
			ADGF_NO_FLAGS,
			GUIO2(GUIO_NOSPEECH, GUIO_NOMIDI)
		},

		GType_ELVIRA1,
		GID_ELVIRA1,
		GF_OLD_BUNDLE | GF_CRUNCHED | GF_PLANAR
	},

	// Elvira 1 - German Amiga Floppy
	{
		{
			"elvira1",
			"Floppy",
			{
	{ "gameamiga",		GAME_BASEFILE,	"bde0334344c7b3a278ccc9a300f3085c", -1},
	{ "icon.dat",		GAME_ICONFILE,	"2db931e84f1ca01f0816dddfae3f49e1", -1},
	{ NULL, 0, NULL, 0}
			},
			Common::DE_DEU,
			Common::kPlatformAmiga,
			ADGF_NO_FLAGS,
			GUIO2(GUIO_NOSPEECH, GUIO_NOMIDI)
		},

		GType_ELVIRA1,
		GID_ELVIRA1,
		GF_OLD_BUNDLE | GF_CRUNCHED | GF_PLANAR
	},

	// Elvira 1 - English Atari ST Floppy Demo
	{
		{
			"elvira1",
			"Non-Interactive Demo",

			{
	{ "991.out",		GAME_VGAFILE,	"9238242d3274bb770cb4925d2b268f83", 1822},
	{ "992.out",		GAME_VGAFILE,	"5526cd64e515f1c5f9ff8f2fb569c4eb", 192236},
	{ "993.out",		GAME_VGAFILE,	"d41d8cd98f00b204e9800998ecf8427e", 0},
	{ NULL, 0, NULL, 0}
			},
			Common::EN_ANY,
			Common::kPlatformAtariST,
			ADGF_DEMO,
			GUIO2(GUIO_NOSPEECH, GUIO_NOMIDI)
		},

		GType_ELVIRA1,
		GID_ELVIRA1,
		GF_OLD_BUNDLE | GF_PLANAR | GF_DEMO
	},

	// Elvira 1 - English Atari ST Floppy
	{
		{
			"elvira1",
			"Floppy",

			{
	{ "gamest",		GAME_BASEFILE,	"8942859018fcfb2dbed13e83d974d1ab", -1},
	{ "icon.dat",		GAME_ICONFILE,	"2db931e84f1ca01f0816dddfae3f49e1", -1},
	{ "tbllist",		GAME_TBLFILE,	"5b6ff494bf7e24213758598ef4ac0a8b", -1},
	{ NULL, 0, NULL, 0}
			},
			Common::EN_ANY,
			Common::kPlatformAtariST,
			ADGF_NO_FLAGS,
			GUIO2(GUIO_NOSPEECH, GUIO_NOMIDI)
		},

		GType_ELVIRA1,
		GID_ELVIRA1,
		GF_OLD_BUNDLE | GF_CRUNCHED | GF_PLANAR
	},

	// Elvira 1 - English Atari ST Floppy alternative?
	{
		{
			"elvira1",
			"Floppy",

			{
	{ "gamest",		GAME_BASEFILE,	"ce2100ba71284f55ac302847d7f94747", 119851},
	{ "icon.dat",		GAME_ICONFILE,	"2db931e84f1ca01f0816dddfae3f49e1", 36573},
	{ "tbllist",		GAME_TBLFILE,	"5b6ff494bf7e24213758598ef4ac0a8b", 476},
	{ NULL, 0, NULL, 0}
			},
			Common::EN_ANY,
			Common::kPlatformAtariST,
			ADGF_NO_FLAGS,
			GUIO2(GUIO_NOSPEECH, GUIO_NOMIDI)
		},

		GType_ELVIRA1,
		GID_ELVIRA1,
		GF_OLD_BUNDLE | GF_CRUNCHED | GF_PLANAR
	},

	// Elvira 1 - English DOS Floppy Demo
	{
		{
			"elvira1",
			"Non-Interactive Demo",

			{
	{ "demo",		GAME_BASEFILE,	"54b43b6ab38964bd7fd17e9f1b41cc64", 2308},
	{ "icon.dat",		GAME_ICONFILE,	"55d8dd70c54340397ca518665274a477", 576},
	{ "tbllist",		GAME_TBLFILE,	"319f6b227c7822a551f57d24e70f8149", 368},
	{ NULL, 0, NULL, 0}
			},
			Common::EN_ANY,
			Common::kPlatformPC,
			ADGF_DEMO,
			GUIO1(GUIO_NOSPEECH)
		},

		GType_ELVIRA1,
		GID_ELVIRA1,
		GF_OLD_BUNDLE | GF_DEMO
	},

	// Elvira 1 - English DOS Floppy
	{
		{
			"elvira1",
			"Floppy",

			{
	{ "gamepc",		GAME_BASEFILE,	"a49e132a1f18306dd5d1ec2fe435e178", 135332},
	{ "icon.dat",		GAME_ICONFILE,	"fda48c9da7f3e72d0313e2f5f760fc45", 56448},
	{ "tbllist",		GAME_TBLFILE,	"319f6b227c7822a551f57d24e70f8149", 368},
	{ NULL, 0, NULL, 0}
			},
			Common::EN_ANY,
			Common::kPlatformPC,
			ADGF_NO_FLAGS,
			GUIO1(GUIO_NOSPEECH)
		},

		GType_ELVIRA1,
		GID_ELVIRA1,
		GF_OLD_BUNDLE
	},

	// Elvira 1 - English DOS Floppy, with Spanish patch
	{
		{
			"elvira1",
			"Floppy",

			{
	{ "gamepc",		GAME_BASEFILE,	"97d2f74db08845c43474312a87330cf6", 137361},
	{ "icon.dat",		GAME_ICONFILE,	"fda48c9da7f3e72d0313e2f5f760fc45", 56448},
	{ "tbllist",		GAME_TBLFILE,	"319f6b227c7822a551f57d24e70f8149", 368},
	{ NULL, 0, NULL, 0}
			},
			Common::ES_ESP,
			Common::kPlatformPC,
			ADGF_NO_FLAGS,
			GUIO1(GUIO_NOSPEECH)
		},

		GType_ELVIRA1,
		GID_ELVIRA1,
		GF_OLD_BUNDLE
	},

	// Elvira 1 - French DOS Floppy
	{
		{
			"elvira1",
			"Floppy",

			{
	{ "gamepc",		GAME_BASEFILE,	"9076d507d60cc454df662316438ec843", -1},
	{ "icon.dat",		GAME_ICONFILE,	"fda48c9da7f3e72d0313e2f5f760fc45", -1},
	{ "tbllist",		GAME_TBLFILE,	"319f6b227c7822a551f57d24e70f8149", -1},
	{ NULL, 0, NULL, 0}
			},
			Common::FR_FRA,
			Common::kPlatformPC,
			ADGF_NO_FLAGS,
			GUIO1(GUIO_NOSPEECH)
		},

		GType_ELVIRA1,
		GID_ELVIRA1,
		GF_OLD_BUNDLE
	},

	// Elvira 1 - German DOS Floppy
	{
		{
			"elvira1",
			"Floppy",

			{
	{ "gamepc",		GAME_BASEFILE,	"d0b593143e21fc150c044819df2c0b98", -1},
	{ "icon.dat",		GAME_ICONFILE,	"fda48c9da7f3e72d0313e2f5f760fc45", -1},
	{ "tbllist",		GAME_TBLFILE,	"319f6b227c7822a551f57d24e70f8149", -1},
	{ NULL, 0, NULL, 0}
			},
			Common::DE_DEU,
			Common::kPlatformPC,
			ADGF_NO_FLAGS,
			GUIO1(GUIO_NOSPEECH)
		},

		GType_ELVIRA1,
		GID_ELVIRA1,
		GF_OLD_BUNDLE
	},

	// Elvira 2 - English Amiga Floppy
	{
		{
			"elvira2",
			"Floppy",

			{
	{ "gameamiga",		GAME_BASEFILE,	"4aa163967f5d2bd319f8350d6af03186", 134799},
	{ "icon.dat",		GAME_ICONFILE,	"a88b1c02e13ab04dd790ec30502c323d", 69860},
	{ "menus.dat",		GAME_MENUFILE,	"a2fdc88a77c8bdffec6b36cbeda4d955", 108},
	{ "start",		GAME_RESTFILE,	"a9f876c6c66dfd011b971da3dc7b4ada", 27752},
	{ "stripped.txt",	GAME_STRFILE,	"41c975a9c1106cb5298a0bc3df0a266e", 72},
	{ "tbllist",		GAME_TBLFILE,	"177f5f2640e80ef92d1421d32de06a5e", 272},
	{ NULL, 0, NULL, 0}
			},
			Common::EN_ANY,
			Common::kPlatformAmiga,
			ADGF_NO_FLAGS,
			GUIO2(GUIO_NOSPEECH, GUIO_NOMIDI)
		},

		GType_ELVIRA2,
		GID_ELVIRA2,
		GF_OLD_BUNDLE | GF_CRUNCHED | GF_PLANAR
	},

	// Elvira 2 - French Amiga Floppy
	{
		{
			"elvira2",
			"Floppy",

			{
	{ "gameamiga",		GAME_BASEFILE,	"7bb91fd61a135243b18b74b51ebca6bf", -1},
	{ "icon.dat",		GAME_ICONFILE,	"a88b1c02e13ab04dd790ec30502c323d", -1},
	{ "menus.dat",		GAME_MENUFILE,	"a2fdc88a77c8bdffec6b36cbeda4d955", -1},
	{ "start",		GAME_RESTFILE,	"a9f876c6c66dfd011b971da3dc7b4ada", -1},
	{ "stripped.txt",	GAME_STRFILE,	"41c975a9c1106cb5298a0bc3df0a266e", -1},
	{ "tbllist",		GAME_TBLFILE,	"177f5f2640e80ef92d1421d32de06a5e", -1},
	{ NULL, 0, NULL, 0}
			},
			Common::FR_FRA,
			Common::kPlatformAmiga,
			ADGF_NO_FLAGS,
			GUIO2(GUIO_NOSPEECH, GUIO_NOMIDI)
		},

		GType_ELVIRA2,
		GID_ELVIRA2,
		GF_OLD_BUNDLE | GF_CRUNCHED | GF_PLANAR
	},

	// Elvira 2 - German Amiga Floppy
	{
		{
			"elvira2",
			"Floppy",

			{
	{ "menus.dat",		GAME_MENUFILE,	"a2fdc88a77c8bdffec6b36cbeda4d955", -1},
	{ "stripped.txt",	GAME_STRFILE,	"41c975a9c1106cb5298a0bc3df0a266e", -1},
	{ "gameamiga",		GAME_BASEFILE,	"7af80eb9759bcafcd8df21e61c5af200", -1},
	{ "icon.dat",		GAME_ICONFILE,	"a88b1c02e13ab04dd790ec30502c323d", -1},
	{ "tbllist",		GAME_TBLFILE,	"177f5f2640e80ef92d1421d32de06a5e", -1},
	{ "start",		GAME_RESTFILE,	"a9f876c6c66dfd011b971da3dc7b4ada", -1},
	{ NULL, 0, NULL, 0}
			},
			Common::DE_DEU,
			Common::kPlatformAmiga,
			ADGF_NO_FLAGS,
			GUIO2(GUIO_NOSPEECH, GUIO_NOMIDI)
		},

		GType_ELVIRA2,
		GID_ELVIRA2,
		GF_OLD_BUNDLE | GF_CRUNCHED | GF_PLANAR
	},

	// Elvira 2 - Italian Amiga Floppy
	{
		{
			"elvira2",
			"Floppy",

			{
	{ "gameamiga",		GAME_BASEFILE,	"3d4e0c8da4ebd222e50de2dffed92955", 139505},
	{ "icon.dat",		GAME_ICONFILE,	"a88b1c02e13ab04dd790ec30502c323d", 69860},
	{ "menus.dat",		GAME_MENUFILE,	"a2fdc88a77c8bdffec6b36cbeda4d955", 108},
	{ "start",		GAME_RESTFILE,	"a9f876c6c66dfd011b971da3dc7b4ada", 27752},
	{ "stripped.txt",	GAME_STRFILE,	"41c975a9c1106cb5298a0bc3df0a266e", 72},
	{ "tbllist",		GAME_TBLFILE,	"177f5f2640e80ef92d1421d32de06a5e", 272},
	{ NULL, 0, NULL, 0}
			},
			Common::IT_ITA,
			Common::kPlatformAmiga,
			ADGF_NO_FLAGS,
			GUIO2(GUIO_NOSPEECH, GUIO_NOMIDI)
		},

		GType_ELVIRA2,
		GID_ELVIRA2,
		GF_OLD_BUNDLE | GF_CRUNCHED | GF_PLANAR
	},

	// Elvira 2 - Spanish Amiga Floppy
	{
		{
			"elvira2",
			"Floppy",

			{
	{ "gameamiga",		GAME_BASEFILE,	"fddfac048a759c84ecf96e3d0cb368cc", 139126},
	{ "icon.dat",		GAME_ICONFILE,	"a88b1c02e13ab04dd790ec30502c323d", 69860},
	{ "menus.dat",		GAME_MENUFILE,	"a2fdc88a77c8bdffec6b36cbeda4d955", 108},
	{ "start",		GAME_RESTFILE,	"a9f876c6c66dfd011b971da3dc7b4ada", 27752},
	{ "stripped.txt",	GAME_STRFILE,	"41c975a9c1106cb5298a0bc3df0a266e", 72},
	{ "tbllist",		GAME_TBLFILE,	"177f5f2640e80ef92d1421d32de06a5e", 272},
	{ NULL, 0, NULL, 0}
			},
			Common::ES_ESP,
			Common::kPlatformAmiga,
			ADGF_NO_FLAGS,
			GUIO2(GUIO_NOSPEECH, GUIO_NOMIDI)
		},

		GType_ELVIRA2,
		GID_ELVIRA2,
		GF_OLD_BUNDLE | GF_CRUNCHED | GF_PLANAR
	},

	// Elvira 2 - English Atari ST Floppy
	{
		{
			"elvira2",
			"Floppy",

			{
	{ "gamest",		GAME_BASEFILE,	"1b1acd637d32bee79859b7cc9de070e7", -1},
	{ "icon.dat",		GAME_ICONFILE,	"9a4eaf4df0cdf5cc85a5134150f96589", -1},
	{ "menus.dat",		GAME_MENUFILE,	"a2fdc88a77c8bdffec6b36cbeda4d955", -1},
	{ "start",		GAME_RESTFILE,	"8cddf461f418ea12f711fda3d3dd62fe", -1},
	{ "stripped.txt",	GAME_STRFILE,	"41c975a9c1106cb5298a0bc3df0a266e", -1},
	{ "tbllist",		GAME_TBLFILE,	"177f5f2640e80ef92d1421d32de06a5e", -1},
	{ NULL, 0, NULL, 0}
			},
			Common::EN_ANY,
			Common::kPlatformAtariST,
			ADGF_NO_FLAGS,
			GUIO2(GUIO_NOSPEECH, GUIO_NOMIDI)
		},

		GType_ELVIRA2,
		GID_ELVIRA2,
		GF_OLD_BUNDLE | GF_CRUNCHED | GF_PLANAR
	},

	// Elvira 2 - French Atari ST Floppy
	{
		{
			"elvira2",
			"Floppy",

			{
	{ "gamest",		GAME_BASEFILE,	"7bb91fd61a135243b18b74b51ebca6bf", -1},
	{ "icon.dat",		GAME_ICONFILE,	"9a4eaf4df0cdf5cc85a5134150f96589", -1},
	{ "menus.dat",		GAME_MENUFILE,	"a2fdc88a77c8bdffec6b36cbeda4d955", -1},
	{ "start",		GAME_RESTFILE,	"8cddf461f418ea12f711fda3d3dd62fe", -1},
	{ "stripped.txt",	GAME_STRFILE,	"41c975a9c1106cb5298a0bc3df0a266e", -1},
	{ "tbllist",		GAME_TBLFILE,	"177f5f2640e80ef92d1421d32de06a5e", -1},
	{ NULL, 0, NULL, 0}
			},
			Common::FR_FRA,
			Common::kPlatformAtariST,
			ADGF_NO_FLAGS,
			GUIO2(GUIO_NOSPEECH, GUIO_NOMIDI)
		},

		GType_ELVIRA2,
		GID_ELVIRA2,
		GF_OLD_BUNDLE | GF_CRUNCHED | GF_PLANAR
	},


	// Elvira 2 - English DOS Floppy
	{
		{
			"elvira2",
			"Floppy",

			{
	{ "gamepc",		GAME_BASEFILE,	"3313254722031b22d833a2cf45a91fd7", 125702},
	{ "icon.dat",		GAME_ICONFILE,	"83a7278bff55c82fbb3aef92981866c9", 54471},
	{ "menus.dat",		GAME_MENUFILE,	"a2fdc88a77c8bdffec6b36cbeda4d955", 108},
	{ "start",		GAME_RESTFILE,	"4d380a35ba941d03ee5084c71d20055b", 27876},
	{ "stripped.txt",	GAME_STRFILE,	"c2533277b7ff11f5495967d55355ea17", 81},
	{ "tbllist",		GAME_TBLFILE,	"8252660df0edbdbc3e6377e155bbd0c5", 284},
	{ NULL, 0, NULL, 0}
			},
			Common::EN_ANY,
			Common::kPlatformPC,
			ADGF_NO_FLAGS,
			GUIO1(GUIO_NOSPEECH)
		},

		GType_ELVIRA2,
		GID_ELVIRA2,
		GF_OLD_BUNDLE
	},

	// Elvira 2 - English DOS Floppy, Alternate 1
	{
		{
			"elvira2",
			"Floppy",

			{
	{ "gamepc",		GAME_BASEFILE,	"1282fd5c520861ae2b73bf653afef547", -1},
	{ "icon.dat",		GAME_ICONFILE,	"83a7278bff55c82fbb3aef92981866c9", -1},
	{ "menus.dat",		GAME_MENUFILE,	"a2fdc88a77c8bdffec6b36cbeda4d955", -1},
	{ "start",		GAME_RESTFILE,	"4d380a35ba941d03ee5084c71d20055b", -1},
	{ "stripped.txt",	GAME_STRFILE,	"c2533277b7ff11f5495967d55355ea17", -1},
	{ "tbllist",		GAME_TBLFILE,	"8252660df0edbdbc3e6377e155bbd0c5", -1},
	{ NULL, 0, NULL, 0}
			},
			Common::EN_ANY,
			Common::kPlatformPC,
			ADGF_NO_FLAGS,
			GUIO1(GUIO_NOSPEECH)
		},

		GType_ELVIRA2,
		GID_ELVIRA2,
		GF_OLD_BUNDLE
	},

	// Elvira 2 - English DOS Floppy, Alternate 2
	{
		{
			"elvira2",
			"Floppy",

			{
	{ "gamepc",		GAME_BASEFILE,	"75d814739585b6fa89a025045885e3b9", 125665},
	{ "icon.dat",		GAME_ICONFILE,	"83a7278bff55c82fbb3aef92981866c9", 54471},
	{ "menus.dat",		GAME_MENUFILE,	"a2fdc88a77c8bdffec6b36cbeda4d955", 108},
	{ "start",		GAME_RESTFILE,	"016107aced82d0cc5d758a9fba716270", 27852},
	{ "stripped.txt",	GAME_STRFILE,	"c3a8f644551a27c8a2fec0f8070b46b7", 81},
	{ "tbllist",		GAME_TBLFILE,	"8252660df0edbdbc3e6377e155bbd0c5", 284},
	{ NULL, 0, NULL, 0}
			},
			Common::EN_ANY,
			Common::kPlatformPC,
			ADGF_NO_FLAGS,
			GUIO1(GUIO_NOSPEECH)
		},

		GType_ELVIRA2,
		GID_ELVIRA2,
		GF_OLD_BUNDLE
	},

	// Elvira 2 - French DOS Floppy
	{
		{
			"elvira2",
			"Floppy",

			{
	{ "gamepc",		GAME_BASEFILE,	"4bf28ab00f5324fd938e632595742382", 130980},
	{ "icon.dat",		GAME_ICONFILE,	"83a7278bff55c82fbb3aef92981866c9", 54471},
	{ "menus.dat",		GAME_MENUFILE,	"a2fdc88a77c8bdffec6b36cbeda4d955", 108},
	{ "start",		GAME_RESTFILE,	"016107aced82d0cc5d758a9fba716270", 27852},
	{ "stripped.txt",	GAME_STRFILE,	"c3a8f644551a27c8a2fec0f8070b46b7", 81},
	{ "tbllist",		GAME_TBLFILE,	"8252660df0edbdbc3e6377e155bbd0c5", 284},
	{ NULL, 0, NULL, 0}
			},
			Common::FR_FRA,
			Common::kPlatformPC,
			ADGF_NO_FLAGS,
			GUIO1(GUIO_NOSPEECH)
		},

		GType_ELVIRA2,
		GID_ELVIRA2,
		GF_OLD_BUNDLE
	},

	// Elvira 2 - German DOS Floppy
	{
		{
			"elvira2",
			"Floppy",

			{
	{ "gamepc",		GAME_BASEFILE,	"d1979d2fbc5fb5276563578ca55cbcec", -1},
	{ "icon.dat",		GAME_ICONFILE,	"83a7278bff55c82fbb3aef92981866c9", -1},
	{ "menus.dat",		GAME_MENUFILE,	"a2fdc88a77c8bdffec6b36cbeda4d955", -1},
	{ "start",		GAME_RESTFILE,	"016107aced82d0cc5d758a9fba716270", -1},
	{ "stripped.txt",	GAME_STRFILE,	"c3a8f644551a27c8a2fec0f8070b46b7", -1},
	{ "tbllist",		GAME_TBLFILE,	"8252660df0edbdbc3e6377e155bbd0c5", -1},
	{ NULL, 0, NULL, 0}
			},
			Common::DE_DEU,
			Common::kPlatformPC,
			ADGF_NO_FLAGS,
			GUIO1(GUIO_NOSPEECH)
		},

		GType_ELVIRA2,
		GID_ELVIRA2,
		GF_OLD_BUNDLE
	},

	// Elvira 2 - Italian DOS Floppy
	{
		{
			"elvira2",
			"Floppy",

			{
	{ "gamepc",		GAME_BASEFILE,	"09a3f1087f2977ff462ad2417bde0a5c", 129833},
	{ "icon.dat",		GAME_ICONFILE,	"83a7278bff55c82fbb3aef92981866c9", 54471},
	{ "menus.dat",		GAME_MENUFILE,	"a2fdc88a77c8bdffec6b36cbeda4d955", 108},
	{ "start",		GAME_RESTFILE,	"016107aced82d0cc5d758a9fba716270", 27852},
	{ "stripped.txt",	GAME_STRFILE,	"c3a8f644551a27c8a2fec0f8070b46b7", 81},
	{ "tbllist",		GAME_TBLFILE,	"8252660df0edbdbc3e6377e155bbd0c5", 284},
	{ NULL, 0, NULL, 0}
			},
			Common::IT_ITA,
			Common::kPlatformPC,
			ADGF_NO_FLAGS,
			GUIO1(GUIO_NOSPEECH)
		},

		GType_ELVIRA2,
		GID_ELVIRA2,
		GF_OLD_BUNDLE
	},

	// Elvira 2 - Spanish DOS Floppy
	{
		{
			"elvira2",
			"Floppy",

			{
	{ "gamepc",		GAME_BASEFILE,	"bfcd74d704ad481d75eb6ba5b828333a", 129577},
	{ "icon.dat",		GAME_ICONFILE,	"83a7278bff55c82fbb3aef92981866c9", 54471},
	{ "menus.dat",		GAME_MENUFILE,	"a2fdc88a77c8bdffec6b36cbeda4d955", 108},
	{ "start",		GAME_RESTFILE,	"016107aced82d0cc5d758a9fba716270", 27852},
	{ "stripped.txt",	GAME_STRFILE,	"c3a8f644551a27c8a2fec0f8070b46b7", 81},
	{ "tbllist",		GAME_TBLFILE,	"8252660df0edbdbc3e6377e155bbd0c5", 284},
	{ NULL, 0, NULL, 0}
			},
			Common::ES_ESP,
			Common::kPlatformPC,
			ADGF_NO_FLAGS,
			GUIO1(GUIO_NOSPEECH)
		},

		GType_ELVIRA2,
		GID_ELVIRA2,
		GF_OLD_BUNDLE
	},

	// Elvira 2 - Spanish DOS Floppy, Alternate
	{
		{
			"elvira2",
			"Floppy",

			{
	{ "gamepc",		GAME_BASEFILE,	"e84e1ac84f63d9a39270e517196c5ff9", 129577},
	{ "icon.dat",		GAME_ICONFILE,	"83a7278bff55c82fbb3aef92981866c9", 54471},
	{ "menus.dat",		GAME_MENUFILE,	"a2fdc88a77c8bdffec6b36cbeda4d955", 108},
	{ "start",		GAME_RESTFILE,	"016107aced82d0cc5d758a9fba716270", 27852},
	{ "stripped.txt",	GAME_STRFILE,	"c3a8f644551a27c8a2fec0f8070b46b7", 81},
	{ "tbllist",		GAME_TBLFILE,	"8252660df0edbdbc3e6377e155bbd0c5", 284},
	{ NULL, 0, NULL, 0}
			},
			Common::ES_ESP,
			Common::kPlatformPC,
			ADGF_NO_FLAGS,
			GUIO1(GUIO_NOSPEECH)
		},

		GType_ELVIRA2,
		GID_ELVIRA2,
		GF_OLD_BUNDLE
	},

	// Waxworks - English Amiga Floppy
	{
		{
			"waxworks",
			"Floppy",

			{
	{ "gameamiga",		GAME_BASEFILE,	"eca24fe7c3e005caca47cecac56f7245", 43392},
	{ "icon.pkd",		GAME_ICONFILE,	"4822a91c18b1b2005ac17fc617f7dcbe", 18940},
	{ "menus.dat",		GAME_MENUFILE,	"3409eeb8ca8b46fc04da99de67573f5e", 320},
	{ "start",		GAME_RESTFILE,	"b575b336e741dde1725edd4079d5ab67", 20902},
	{ "stripped.txt",	GAME_STRFILE,	"6faaebff2786216900061eeb978f10af", 225},
	{ "tbllist",		GAME_TBLFILE,	"95c44bfc380770a6b6dd0dfcc69e80a0", 309},
	{ "xtbllist",		GAME_XTBLFILE,	"6c7b3db345d46349a5226f695c03e20f", 88},
	{ NULL, 0, NULL, 0}
			},
			Common::EN_ANY,
			Common::kPlatformAmiga,
			ADGF_NO_FLAGS,
			GUIO2(GUIO_NOSPEECH, GUIO_NOMIDI)
		},

		GType_WW,
		GID_WAXWORKS,
		GF_OLD_BUNDLE | GF_CRUNCHED | GF_CRUNCHED_GAMEPC | GF_PLANAR
	},

	// Waxworks - German Amiga Floppy
	{
		{
			"waxworks",
			"Floppy",

			{
	{ "gameamiga",		GAME_BASEFILE,	"2938a17103de603c4c6f05e6a433b365", 44640},
	{ "icon.pkd",		GAME_ICONFILE,	"4822a91c18b1b2005ac17fc617f7dcbe", 18940},
	{ "menus.dat",		GAME_MENUFILE,	"3409eeb8ca8b46fc04da99de67573f5e", 320},
	{ "start",		GAME_RESTFILE,	"b575b336e741dde1725edd4079d5ab67", 20902},
	{ "stripped.txt",	GAME_STRFILE,	"6faaebff2786216900061eeb978f10af", 225},
	{ "tbllist",		GAME_TBLFILE,	"95c44bfc380770a6b6dd0dfcc69e80a0", 309},
	{ "xtbllist",		GAME_XTBLFILE,	"6c7b3db345d46349a5226f695c03e20f", 88},
	{ NULL, 0, NULL, 0}
			},
			Common::DE_DEU,
			Common::kPlatformAmiga,
			ADGF_NO_FLAGS,
			GUIO2(GUIO_NOSPEECH, GUIO_NOMIDI)
		},

		GType_WW,
		GID_WAXWORKS,
		GF_OLD_BUNDLE | GF_CRUNCHED | GF_CRUNCHED_GAMEPC | GF_PLANAR
	},

	// Waxworks - English DOS Floppy Demo
	{
		{
			"waxworks",
			"Non-Interactive Demo",

			{
	{ "demo",		GAME_BASEFILE,	"50704abde6c68a226001400461620129", 7238},
	{ "icon.dat",		GAME_ICONFILE,	"ef1b8ad3494cf103dc10a99fe152ef9a", 20901},
	{ "menus.dat",		GAME_MENUFILE,	"c4103f122d27677c9db144cae1394a66", 2},
	{ NULL, 0, NULL, 0}
			},
			Common::EN_ANY,
			Common::kPlatformPC,
			ADGF_DEMO,
			GUIO1(GUIO_NOSPEECH)
		},

		GType_WW,
		GID_WAXWORKS,
		GF_OLD_BUNDLE | GF_DEMO
	},

	// Waxworks - English DOS Floppy
	{
		{
			"waxworks",
			"Floppy",

			{
	{ "gamepc",		GAME_BASEFILE,	"7751e9358e894e32ef40ef3b3bae0f2a", 51327},
	{ "icon.dat",		GAME_ICONFILE,	"ef1b8ad3494cf103dc10a99fe152ef9a", 20901},
	{ "menus.dat",		GAME_MENUFILE,	"3409eeb8ca8b46fc04da99de67573f5e", 320},
	{ "roomslst",		GAME_RMSLFILE,	"e3758c46ab8f3c23a1ac012bd607108d", 128},
	{ "start",		GAME_RESTFILE,	"036b647973d6884cdfc2042a3d12df83", 15354},
	{ "statelst",		GAME_STATFILE,	"469e98c69f00928a8366ba415d91902d", 11104},
	{ "stripped.txt",	GAME_STRFILE,	"f259e3e07a1cde8d0404a767d815e12c", 225},
	{ "tbllist",		GAME_TBLFILE,	"95c44bfc380770a6b6dd0dfcc69e80a0", 309},
	{ "xtbllist",		GAME_XTBLFILE,	"6c7b3db345d46349a5226f695c03e20f", 88},
	{ NULL, 0, NULL, 0}
			},
			Common::EN_ANY,
			Common::kPlatformPC,
			ADGF_NO_FLAGS,
			GUIO1(GUIO_NOSPEECH)
		},

		GType_WW,
		GID_WAXWORKS,
		GF_OLD_BUNDLE
	},

	// Waxworks - French DOS Floppy
	{
		{
			"waxworks",
			"Floppy",

			{
	{ "gamepc",		GAME_BASEFILE,	"7edfdcccbf7627532882192c1a356150", 53681},
	{ "icon.dat",		GAME_ICONFILE,	"ef1b8ad3494cf103dc10a99fe152ef9a", 20901},
	{ "menus.dat",		GAME_MENUFILE,	"3409eeb8ca8b46fc04da99de67573f5e", 320},
	{ "roomslst",		GAME_RMSLFILE,	"e3758c46ab8f3c23a1ac012bd607108d", 128},
	{ "start",		GAME_RESTFILE,	"036b647973d6884cdfc2042a3d12df83", 15354},
	{ "statelst",		GAME_STATFILE,	"469e98c69f00928a8366ba415d91902d", 11104},
	{ "stripped.txt",	GAME_STRFILE,	"f259e3e07a1cde8d0404a767d815e12c", 225},
	{ "tbllist",		GAME_TBLFILE,	"95c44bfc380770a6b6dd0dfcc69e80a0", 309},
	{ "xtbllist",		GAME_XTBLFILE,	"6c7b3db345d46349a5226f695c03e20f", 88},
	{ NULL, 0, NULL, 0}
			},
			Common::FR_FRA,
			Common::kPlatformPC,
			ADGF_NO_FLAGS,
			GUIO1(GUIO_NOSPEECH)
		},

		GType_WW,
		GID_WAXWORKS,
		GF_OLD_BUNDLE
	},

	// Waxworks - German DOS Floppy
	{
		{
			"waxworks",
			"Floppy",

			{
	{ "gamepc",		GAME_BASEFILE,	"32ee34134422e286525c73e71bd0ea2d", 53523},
	{ "icon.dat",		GAME_ICONFILE,	"ef1b8ad3494cf103dc10a99fe152ef9a", 20901},
	{ "menus.dat",		GAME_MENUFILE,	"3409eeb8ca8b46fc04da99de67573f5e", 320},
	{ "roomslst",		GAME_RMSLFILE,	"e3758c46ab8f3c23a1ac012bd607108d", 128},
	{ "start",		GAME_RESTFILE,	"036b647973d6884cdfc2042a3d12df83", 15354},
	{ "statelst",		GAME_STATFILE,	"469e98c69f00928a8366ba415d91902d", 11104},
	{ "stripped.txt",	GAME_STRFILE,	"f259e3e07a1cde8d0404a767d815e12c", 225},
	{ "tbllist",		GAME_TBLFILE,	"95c44bfc380770a6b6dd0dfcc69e80a0", 309},
	{ "xtbllist",		GAME_XTBLFILE,	"6c7b3db345d46349a5226f695c03e20f", 88},
	{ NULL, 0, NULL, 0}
			},
			Common::DE_DEU,
			Common::kPlatformPC,
			ADGF_NO_FLAGS,
			GUIO1(GUIO_NOSPEECH)
		},

		GType_WW,
		GID_WAXWORKS,
		GF_OLD_BUNDLE
	},

	// Waxworks - Spanish DOS Floppy
	{
		{
			"waxworks",
			"Floppy",

			{
	{ "gamepc",		GAME_BASEFILE,	"b0d513069920a5e2eac2ea5d290692f1", 53307},
	{ "icon.dat",		GAME_ICONFILE,	"ef1b8ad3494cf103dc10a99fe152ef9a", 20901},
	{ "menus.dat",		GAME_MENUFILE,	"3409eeb8ca8b46fc04da99de67573f5e", 320},
	{ "roomslst",		GAME_RMSLFILE,	"e3758c46ab8f3c23a1ac012bd607108d", 128},
	{ "start",		GAME_RESTFILE,	"036b647973d6884cdfc2042a3d12df83", 15354},
	{ "statelst",		GAME_STATFILE,	"469e98c69f00928a8366ba415d91902d", -1},
	{ "stripped.txt",	GAME_STRFILE,	"f259e3e07a1cde8d0404a767d815e12c", 225},
	{ "tbllist",		GAME_TBLFILE,	"95c44bfc380770a6b6dd0dfcc69e80a0", 309},
	{ "xtbllist",		GAME_XTBLFILE,	"6c7b3db345d46349a5226f695c03e20f", 88},
	{ NULL, 0, NULL, 0}
			},
			Common::ES_ESP,
			Common::kPlatformPC,
			ADGF_NO_FLAGS,
			GUIO1(GUIO_NOSPEECH)
		},

		GType_WW,
		GID_WAXWORKS,
		GF_OLD_BUNDLE
	},

	// Simon the Sorcerer 1 - English Acorn Floppy
	{
		{
			"simon1",
			"Floppy",

			{
	{ "gamebase.dat",	GAME_BASEFILE,	"c392e494dcabed797b98cbcfc687b33a", 36980},
	{ "icondata.dat",	GAME_ICONFILE,	"22107c24dfb31b66ac503c28a6e20b19", 14361},
	{ "stripped.dat",	GAME_STRFILE,	"c95a0a1ee973e19c2a1c5d12026c139f", 252},
	{ "tbllist.dat",	GAME_TBLFILE,	"d198a80de2c59e4a0cd24b98814849e8", 711},
	{ NULL, 0, NULL, 0}
			},
			Common::EN_ANY,
			Common::kPlatformAcorn,
			ADGF_NO_FLAGS,
			GUIO2(GUIO_NOSPEECH, GUIO_NOMIDI)
		},

		GType_SIMON1,
		GID_SIMON1,
		GF_OLD_BUNDLE
	},

	// Simon the Sorcerer 1 - English Acorn CD Demo
	{
		{
			"simon1",
			"CD Demo",

			{
	{ "data",	GAME_GMEFILE,	"b4a7526ced425ba8ad0d548d0ec69900", 1237886},
	{ "gamebase",	GAME_BASEFILE,	"425c7d1957699d35abca7e12a08c7422", 30879},
	{ "icondata",	GAME_ICONFILE,	"22107c24dfb31b66ac503c28a6e20b19", 14361},
	{ "stripped",	GAME_STRFILE,	"d9de7542612d9f4e0819ad0df5eac56b", 252},
	{ "tbllist",	GAME_TBLFILE,	"d198a80de2c59e4a0cd24b98814849e8", 711},
	{ NULL, 0, NULL, 0}
			},
			Common::EN_ANY,
			Common::kPlatformAcorn,
			ADGF_DEMO,
			GUIO1(GUIO_NOSUBTITLES)
		},

		GType_SIMON1,
		GID_SIMON1,
		GF_TALKIE
	},

	// Simon the Sorcerer 1 - English Acorn CD
	{
		{
			"simon1",
			"CD",

			{
	{ "data",	GAME_GMEFILE,	"64958b3a38afdcb85da1eeed85169806", 6943110},
	{ "gamebase",	GAME_BASEFILE,	"28261b99cd9da1242189b4f6f2841bd6", 29176},
	{ "icondata",	GAME_ICONFILE,	"22107c24dfb31b66ac503c28a6e20b19", 14361},
	{ "stripped",	GAME_STRFILE,	"f3b27a3fbb45dcd323a48159496e45e8", 252},
	{ "tbllist",	GAME_TBLFILE,	"d198a80de2c59e4a0cd24b98814849e8", 711},
	{ NULL, 0, NULL, 0}
			},
			Common::EN_ANY,
			Common::kPlatformAcorn,
			ADGF_CD,
			GUIO1(GUIO_NOSUBTITLES)
		},

		GType_SIMON1,
		GID_SIMON1,
		GF_TALKIE
	},

	// Simon the Sorcerer 1 - English Amiga OCS Floppy
	{
		{
			"simon1",
			"OCS Floppy",

			{
	{ "gameamiga",		GAME_BASEFILE,	"bb94a688e247695d912cce9d0173d73a", 37991},
	{ "icon.pkd",		GAME_ICONFILE,	"565ef7a98dcc21ef526a2bb10b6f42ed", 18979},
	{ "stripped.txt",	GAME_STRFILE,	"f5fc67db3b8c5283cda51c43b98a74f8", 243},
	{ "tbllist",		GAME_TBLFILE,	"f9d5bf2ce09f82289c791c3ca26e1e4b", 696},
	{ NULL, 0, NULL, 0}
			},
			Common::EN_ANY,
			Common::kPlatformAmiga,
			ADGF_NO_FLAGS,
			GUIO2(GUIO_NOSPEECH, GUIO_NOMIDI)
		},

		GType_SIMON1,
		GID_SIMON1,
		GF_32COLOR | GF_CRUNCHED | GF_OLD_BUNDLE | GF_PLANAR
	},

	// Simon the Sorcerer 1 - English Amiga OCS Demo
	{
		{
			"simon1",
			"OCS Demo",

			{
	{ "gameamiga",		GAME_BASEFILE,	"4696309eed9d7335c62ebb87a0f006ad", 12764},
	{ "icon.pkd",		GAME_ICONFILE,	"ebc96af15bfaf75ba8210326b9260d2f", 9124},
	{ "stripped.txt",	GAME_STRFILE,	"8edde5b9498dc9f31da1093028da467c", 27},
	{ "tbllist",		GAME_TBLFILE,	"1247e024e1f13ca54c1e354120c7519c", 105},
	{ NULL, 0, NULL, 0}
			},
			Common::EN_ANY,
			Common::kPlatformAmiga,
			ADGF_DEMO,
			GUIO2(GUIO_NOSPEECH, GUIO_NOMIDI)
		},

		GType_SIMON1,
		GID_SIMON1,
		GF_32COLOR | GF_CRUNCHED | GF_CRUNCHED_GAMEPC | GF_OLD_BUNDLE | GF_PLANAR
	},

	// Simon the Sorcerer 1 - English Amiga AGA Floppy
	{
		{
			"simon1",
			"AGA Floppy",

			{
	{ "gameamiga",		GAME_BASEFILE,	"6c9ad2ff571d34a4cf0c696cf4e13500", 38057},
	{ "icon.pkd",		GAME_ICONFILE,	"565ef7a98dcc21ef526a2bb10b6f42ed", 18979},
	{ "stripped.txt",	GAME_STRFILE,	"c649fcc0439766810e5097ee7e81d4c8", 243},
	{ "tbllist",		GAME_TBLFILE,	"f9d5bf2ce09f82289c791c3ca26e1e4b", 696},
	{ NULL, 0, NULL, 0}
			},
			Common::EN_ANY,
			Common::kPlatformAmiga,
			ADGF_NO_FLAGS,
			GUIO2(GUIO_NOSPEECH, GUIO_NOMIDI)
		},

		GType_SIMON1,
		GID_SIMON1,
		GF_CRUNCHED | GF_OLD_BUNDLE | GF_PLANAR
	},

	// Simon the Sorcerer 1 - French Amiga AGA Floppy
	{
		{
			"simon1",
			"AGA Floppy",

			{
	{ "gameamiga",		GAME_BASEFILE,	"bd9828b9d4e5d89b50fe8c47a8e6bc07", -1},
	{ "icon.pkd",		GAME_ICONFILE,	"565ef7a98dcc21ef526a2bb10b6f42ed", -1},
	{ "stripped.txt",	GAME_STRFILE,	"2297baec985617d0d5612a0124bac359", -1},
	{ "tbllist",		GAME_TBLFILE,	"f9d5bf2ce09f82289c791c3ca26e1e4b", -1},
	{ NULL, 0, NULL, 0}
			},
			Common::FR_FRA,
			Common::kPlatformAmiga,
			ADGF_NO_FLAGS,
			GUIO2(GUIO_NOSPEECH, GUIO_NOMIDI)
		},

		GType_SIMON1,
		GID_SIMON1,
		GF_CRUNCHED | GF_OLD_BUNDLE | GF_PLANAR
	},

	// Simon the Sorcerer 1 - German Amiga AGA Floppy
	{
		{
			"simon1",
			"AGA Floppy",

			{
	{ "gameamiga",		GAME_BASEFILE,	"a2de9553f3b73064369948b5af38bb30", -1},
	{ "icon.pkd",		GAME_ICONFILE,	"565ef7a98dcc21ef526a2bb10b6f42ed", -1},
	{ "stripped.txt",	GAME_STRFILE,	"c649fcc0439766810e5097ee7e81d4c8", -1},
	{ "tbllist",		GAME_TBLFILE,	"f9d5bf2ce09f82289c791c3ca26e1e4b", -1},
	{ NULL, 0, NULL, 0}
			},
			Common::DE_DEU,
			Common::kPlatformAmiga,
			ADGF_NO_FLAGS,
			GUIO2(GUIO_NOSPEECH, GUIO_NOMIDI)
		},

		GType_SIMON1,
		GID_SIMON1,
		GF_CRUNCHED | GF_OLD_BUNDLE | GF_PLANAR
	},

	// Simon the Sorcerer 1 - Italian Amiga AGA Floppy
	{
		{
			"simon1",
			"AGA Floppy",

			{
	{ "gameamiga",		GAME_BASEFILE,	"55dc304e7d3f8ad518af3b7f69da02b6", 38941},
	{ "icon.pkd",		GAME_ICONFILE,	"565ef7a98dcc21ef526a2bb10b6f42ed", 18979},
	{ "stripped.txt",	GAME_STRFILE,	"c649fcc0439766810e5097ee7e81d4c8", 243},
	{ "tbllist",		GAME_TBLFILE,	"f9d5bf2ce09f82289c791c3ca26e1e4b", 696},
	{ NULL, 0, NULL, 0}
			},
			Common::IT_ITA,
			Common::kPlatformAmiga,
			ADGF_NO_FLAGS,
			GUIO2(GUIO_NOSPEECH, GUIO_NOMIDI)
		},

		GType_SIMON1,
		GID_SIMON1,
		GF_CRUNCHED | GF_OLD_BUNDLE | GF_PLANAR
	},

	// Simon the Sorcerer 1 - English Amiga CD32
	{
		{
			"simon1",
			"CD32",

			{
	{ "gameamiga",		GAME_BASEFILE,	"bab7f19237cf7d7619b6c73631da1854", 30139},
	{ "icon.pkd",		GAME_ICONFILE,	"565ef7a98dcc21ef526a2bb10b6f42ed", 18979},
	{ "stripped.txt",	GAME_STRFILE,	"59be788020441e21861e284236fd08c1", 243},
	{ "tbllist",		GAME_TBLFILE,	"f9d5bf2ce09f82289c791c3ca26e1e4b", 696},
	{ NULL, 0, NULL, 0}
			},
			Common::EN_ANY,
			Common::kPlatformAmiga,
			ADGF_CD,
			GUIO2(GUIO_NOSUBTITLES, GUIO_NOMIDI)
		},

		GType_SIMON1,
		GID_SIMON1CD32,
		GF_TALKIE | GF_OLD_BUNDLE | GF_PLANAR
	},

	// Simon the Sorcerer 1 - English Amiga CD32 alternative?
	{
		{
			"simon1",
			"CD32",

			{
	{ "gameamiga",		GAME_BASEFILE,	"ec5358680c117f29b128cbbb322111a4", -1},
	{ "icon.pkd",		GAME_ICONFILE,	"8ce5a46466a4f8f6d0f780b0ef00d5f5", -1},
	{ "stripped.txt",	GAME_STRFILE,	"59be788020441e21861e284236fd08c1", -1},
	{ "tbllist",		GAME_TBLFILE,	"f9d5bf2ce09f82289c791c3ca26e1e4b", -1},
	{ NULL, 0, NULL, 0}
			},
			Common::EN_ANY,
			Common::kPlatformAmiga,
			ADGF_CD,
			GUIO2(GUIO_NOSUBTITLES, GUIO_NOMIDI)
		},

		GType_SIMON1,
		GID_SIMON1CD32,
		GF_TALKIE | GF_OLD_BUNDLE | GF_PLANAR
	},

	// Simon the Sorcerer 1 - English DOS Floppy Demo
	{
		{
			"simon1",
			"Floppy Demo",

			{
	{ "gdemo",		GAME_BASEFILE,	"2be4a21bc76e2fdc071867c130651439", 25288},
	{ "icon.dat",		GAME_ICONFILE,	"55af3b4d93972bc58bfee38a86b76c3f", 11495},
	{ "stripped.txt",	GAME_STRFILE,	"33a2e329b97b2a349858d6a093159eb7", 27},
	{ "tbllist",		GAME_TBLFILE,	"1247e024e1f13ca54c1e354120c7519c", 105},
	{ NULL, 0, NULL, 0}
			},
			Common::EN_ANY,
			Common::kPlatformPC,
			ADGF_DEMO,
			GUIO1(GUIO_NOSPEECH)
		},

		GType_SIMON1,
		GID_SIMON1DOS,
		GF_OLD_BUNDLE | GF_DEMO
	},

	// Simon the Sorcerer 1 - English DOS Floppy
	{
		{
			"simon1",
			"Floppy",

			{
	{ "gamepc",		GAME_BASEFILE,	"c392e494dcabed797b98cbcfc687b33a", -1},
	{ "icon.dat",		GAME_ICONFILE,	"22107c24dfb31b66ac503c28a6e20b19", -1},
	{ "stripped.txt",	GAME_STRFILE,	"c95a0a1ee973e19c2a1c5d12026c139f", -1},
	{ "tbllist",		GAME_TBLFILE,	"d198a80de2c59e4a0cd24b98814849e8", -1},
	{ NULL, 0, NULL, 0}
			},
			Common::EN_ANY,
			Common::kPlatformPC,
			ADGF_NO_FLAGS,
			GUIO1(GUIO_NOSPEECH)
		},

		GType_SIMON1,
		GID_SIMON1DOS,
		GF_OLD_BUNDLE
	},

	// Simon the Sorcerer 1 - English DOS Floppy with Czech patch
	{
		{
			"simon1",
			"Floppy",

			{
	{ "gamepc",		GAME_BASEFILE,	"62de24fc579b94fac7d3d23201b65b14", -1},
	{ "icon.dat",		GAME_ICONFILE,	"22107c24dfb31b66ac503c28a6e20b19", -1},
	{ "stripped.txt",	GAME_STRFILE,	"c95a0a1ee973e19c2a1c5d12026c139f", -1},
	{ "tbllist",		GAME_TBLFILE,	"d198a80de2c59e4a0cd24b98814849e8", -1},
	{ NULL, 0, NULL, 0}
			},
			Common::CZ_CZE,
			Common::kPlatformPC,
			ADGF_NO_FLAGS,
			GUIO1(GUIO_NOSPEECH)
		},

		GType_SIMON1,
		GID_SIMON1DOS,
		GF_OLD_BUNDLE
	},

	// Simon the Sorcerer 1 - English DOS Floppy with Russian patch
	{
		{
			"simon1",
			"Floppy",

			{
	{ "gamepc",		GAME_BASEFILE,	"605fb866e03ec1c41b10c6a518ddfa49", -1},
	{ "icon.dat",		GAME_ICONFILE,	"22107c24dfb31b66ac503c28a6e20b19", -1},
	{ "stripped.txt",	GAME_STRFILE,	"c95a0a1ee973e19c2a1c5d12026c139f", -1},
	{ "tbllist",		GAME_TBLFILE,	"d198a80de2c59e4a0cd24b98814849e8", -1},
	{ NULL, 0, NULL, 0}
			},
			Common::RU_RUS,
			Common::kPlatformPC,
			ADGF_NO_FLAGS,
			GUIO1(GUIO_NOSPEECH)
		},

		GType_SIMON1,
		GID_SIMON1DOS,
		GF_OLD_BUNDLE
	},

	// Simon the Sorcerer 1 - English DOS Floppy (Infocom)
	{
		{
			"simon1",
			"Floppy",

			{
	{ "gamepc",		GAME_BASEFILE,	"9f93d27432ce44a787eef10adb640870", 37070},
	{ "icon.dat",		GAME_ICONFILE,	"22107c24dfb31b66ac503c28a6e20b19", 14361},
	{ "stripped.txt",	GAME_STRFILE,	"2af9affc5981eec44b90d4c556145cb8", 252},
	{ "tbllist",		GAME_TBLFILE,	"d198a80de2c59e4a0cd24b98814849e8", 711},
	{ NULL, 0, NULL, 0}
			},
			Common::EN_ANY,
			Common::kPlatformPC,
			ADGF_NO_FLAGS,
			GUIO1(GUIO_NOSPEECH)
		},

		GType_SIMON1,
		GID_SIMON1DOS,
		GF_OLD_BUNDLE
	},

	// Simon the Sorcerer 1 - English DOS Floppy (Infocom) with Czech patch
	{
		{
			"simon1",
			"Floppy",

			{
	{ "gamepc",		GAME_BASEFILE,	"62de24fc579b94fac7d3d23201b65b14", -1},
	{ "icon.dat",		GAME_ICONFILE,	"22107c24dfb31b66ac503c28a6e20b19", -1},
	{ "stripped.txt",	GAME_STRFILE,	"2af9affc5981eec44b90d4c556145cb8", -1},
	{ "tbllist",		GAME_TBLFILE,	"d198a80de2c59e4a0cd24b98814849e8", -1},
	{ NULL, 0, NULL, 0}
			},
			Common::CZ_CZE,
			Common::kPlatformPC,
			ADGF_NO_FLAGS,
			GUIO1(GUIO_NOSPEECH)
		},

		GType_SIMON1,
		GID_SIMON1DOS,
		GF_OLD_BUNDLE
	},

	// Simon the Sorcerer 1 - English DOS Floppy (Infocom) with Russian patch
	{
		{
			"simon1",
			"Floppy",

			{
	{ "gamepc",		GAME_BASEFILE,	"605fb866e03ec1c41b10c6a518ddfa49", -1},
	{ "icon.dat",		GAME_ICONFILE,	"22107c24dfb31b66ac503c28a6e20b19", -1},
	{ "stripped.txt",	GAME_STRFILE,	"2af9affc5981eec44b90d4c556145cb8", -1},
	{ "tbllist",		GAME_TBLFILE,	"d198a80de2c59e4a0cd24b98814849e8", -1},
	{ NULL, 0, NULL, 0}
			},
			Common::RU_RUS,
			Common::kPlatformPC,
			ADGF_NO_FLAGS,
			GUIO1(GUIO_NOSPEECH)
		},

		GType_SIMON1,
		GID_SIMON1DOS,
		GF_OLD_BUNDLE
	},

	// Simon the Sorcerer 1 - French DOS Floppy
	{
		{
			"simon1",
			"Floppy",

			{
	{ "gamepc",		GAME_BASEFILE,	"34759d0d4285a2f4b21b8e03b8fcefb3", -1},
	{ "icon.dat",		GAME_ICONFILE,	"22107c24dfb31b66ac503c28a6e20b19", -1},
	{ "stripped.txt",	GAME_STRFILE,	"aa01e7386057abc0c3e27dbaa9c4ba5b", -1},
	{ "tbllist",		GAME_TBLFILE,	"d198a80de2c59e4a0cd24b98814849e8", -1},
	{ NULL, 0, NULL, 0}
			},
			Common::FR_FRA,
			Common::kPlatformPC,
			ADGF_NO_FLAGS,
			GUIO1(GUIO_NOSPEECH)
		},

		GType_SIMON1,
		GID_SIMON1DOS,
		GF_OLD_BUNDLE
	},

	// Simon the Sorcerer 1 - German DOS Floppy
	{
		{
			"simon1",
			"Floppy",

			{
	{ "gamepc",		GAME_BASEFILE,	"063015e6ce7d90b570dbc21fe0c667b1", -1},
	{ "icon.dat",		GAME_ICONFILE,	"22107c24dfb31b66ac503c28a6e20b19", -1},
	{ "stripped.txt",	GAME_STRFILE,	"c95a0a1ee973e19c2a1c5d12026c139f", -1},
	{ "tbllist",		GAME_TBLFILE,	"d198a80de2c59e4a0cd24b98814849e8", -1},
	{ NULL, 0, NULL, 0}
			},
			Common::DE_DEU,
			Common::kPlatformPC,
			ADGF_NO_FLAGS,
			GUIO1(GUIO_NOSPEECH)
		},

		GType_SIMON1,
		GID_SIMON1DOS,
		GF_OLD_BUNDLE
	},

	// Simon the Sorcerer 1 - Italian DOS Floppy
	{
		{
			"simon1",
			"Floppy",

			{
	{ "gamepc",		GAME_BASEFILE,	"65c9b2dea57df84ef55d1eaf384ebd30", -1},
	{ "icon.dat",		GAME_ICONFILE,	"22107c24dfb31b66ac503c28a6e20b19", -1},
	{ "stripped.txt",	GAME_STRFILE,	"2af9affc5981eec44b90d4c556145cb8", -1},
	{ "tbllist",		GAME_TBLFILE,	"d198a80de2c59e4a0cd24b98814849e8", -1},
	{ NULL, 0, NULL, 0}
			},
			Common::IT_ITA,
			Common::kPlatformPC,
			ADGF_NO_FLAGS,
			GUIO1(GUIO_NOSPEECH)
		},

		GType_SIMON1,
		GID_SIMON1DOS,
		GF_OLD_BUNDLE
	},

	// Simon the Sorcerer 1 - Spanish DOS Floppy
	{
		{
			"simon1",
			"Floppy",

			{
	{ "gamepc",		GAME_BASEFILE,	"5374fafdea2068134f33deab225feed3", -1},
	{ "icon.dat",		GAME_ICONFILE,	"22107c24dfb31b66ac503c28a6e20b19", -1},
	{ "stripped.txt",	GAME_STRFILE,	"2af9affc5981eec44b90d4c556145cb8", -1},
	{ "tbllist",		GAME_TBLFILE,	"d198a80de2c59e4a0cd24b98814849e8", -1},
	{ NULL, 0, NULL, 0}
			},
			Common::ES_ESP,
			Common::kPlatformPC,
			ADGF_NO_FLAGS,
			GUIO1(GUIO_NOSPEECH)
		},

		GType_SIMON1,
		GID_SIMON1DOS,
		GF_OLD_BUNDLE
	},

	// Simon the Sorcerer 1 - English DOS CD Demo
	{
		{
			"simon1",
			"CD Demo",

			{
	{ "simon.gme",		GAME_GMEFILE,	"b4a7526ced425ba8ad0d548d0ec69900", 1237886},
	{ "gamepc"	,	GAME_BASEFILE,	"425c7d1957699d35abca7e12a08c7422", 30879},
	{ "icon.dat",		GAME_ICONFILE,	"22107c24dfb31b66ac503c28a6e20b19", 14361},
	{ "stripped.txt",	GAME_STRFILE,	"d9de7542612d9f4e0819ad0df5eac56b", 252},
	{ "tbllist",		GAME_TBLFILE,	"d198a80de2c59e4a0cd24b98814849e8", 711},
	{ NULL, 0, NULL, 0}
			},
			Common::EN_ANY,
			Common::kPlatformPC,
			ADGF_DEMO,
			GUIO1(GUIO_NOSUBTITLES)
		},

		GType_SIMON1,
		GID_SIMON1,
		GF_TALKIE
	},

	// Simon the Sorcerer 1 - English DOS CD
	{
		{
			"simon1",
			"CD",

			{
	{ "gamepc",		GAME_BASEFILE,	"28261b99cd9da1242189b4f6f2841bd6", 29176},
	{ "icon.dat",		GAME_ICONFILE,	"22107c24dfb31b66ac503c28a6e20b19", 14361},
	{ "simon.gme",		GAME_GMEFILE,	"64958b3a38afdcb85da1eeed85169806", 6943110},
	{ "stripped.txt",	GAME_STRFILE,	"f3b27a3fbb45dcd323a48159496e45e8", 252},
	{ "tbllist",		GAME_TBLFILE,	"d198a80de2c59e4a0cd24b98814849e8", 711},
	{ NULL, 0, NULL, 0}
			},
			Common::EN_ANY,
			Common::kPlatformPC,
			ADGF_CD,
			GUIO1(GUIO_NOSUBTITLES)
		},

		GType_SIMON1,
		GID_SIMON1,
		GF_TALKIE
	},

	// Simon the Sorcerer 1 - English DOS CD alternate?
	{
		{
			"simon1",
			"CD",

			{
	{ "gamepc",		GAME_BASEFILE,	"c0b948b6821d2140f8b977144f21027a", -1},
	{ "icon.dat",		GAME_ICONFILE,	"22107c24dfb31b66ac503c28a6e20b19", -1},
	{ "simon.gme",		GAME_GMEFILE,	"64f73e94639b63af846ac4a8a94a23d8", -1},
	{ "stripped.txt",	GAME_STRFILE,	"f3b27a3fbb45dcd323a48159496e45e8", -1},
	{ "tbllist",		GAME_TBLFILE,	"d198a80de2c59e4a0cd24b98814849e8", -1},
	{ NULL, 0, NULL, 0}
			},
			Common::EN_ANY,
			Common::kPlatformPC,
			ADGF_CD,
			GUIO1(GUIO_NOSUBTITLES)
		},

		GType_SIMON1,
		GID_SIMON1,
		GF_TALKIE
	},

	// Simon the Sorcerer 1 - English DOS CD with Russian patch
	{
		{
			"simon1",
			"CD",

			{
	{ "gamepc",		GAME_BASEFILE,	"3fac46064f69e5298f4f027f204c5aab", -1},
	{ "icon.dat",		GAME_ICONFILE,	"22107c24dfb31b66ac503c28a6e20b19", -1},
	{ "simon.gme",		GAME_GMEFILE,	"91321f0d806f8d9fef71a00e58581427", -1},
	{ "stripped.txt",	GAME_STRFILE,	"ef51ac74c946881ae4d7ca66cc7a0d1e", -1},
	{ "tbllist",		GAME_TBLFILE,	"d198a80de2c59e4a0cd24b98814849e8", -1},
	{ NULL, 0, NULL, 0}
			},
			Common::RU_RUS,
			Common::kPlatformPC,
			ADGF_CD,
			GUIO0()
		},

		GType_SIMON1,
		GID_SIMON1,
		GF_TALKIE
	},

	// Simon the Sorcerer 1 - French DOS CD
	{
		{
			"simon1",
			"CD",

			{
	{ "gamepc",		GAME_BASEFILE,	"3cfb9d1ff4ec725af9924140126cf69f", 39310},
	{ "icon.dat",		GAME_ICONFILE,	"22107c24dfb31b66ac503c28a6e20b19", 14361},
	{ "simon.gme",		GAME_GMEFILE,	"638049fa5d41b81fb6fb11671721b871", 7041803},
	{ "stripped.txt",	GAME_STRFILE,	"ef51ac74c946881ae4d7ca66cc7a0d1e", 252},
	{ "tbllist",		GAME_TBLFILE,	"d198a80de2c59e4a0cd24b98814849e8", 711},
	{ NULL, 0, NULL, 0}
			},
			Common::FR_FRA,
			Common::kPlatformPC,
			ADGF_CD,
			GUIO0()
		},

		GType_SIMON1,
		GID_SIMON1,
		GF_TALKIE
	},

	// Simon the Sorcerer 1 - German DOS CD
	{
		{
			"simon1",
			"CD",

			{
	{ "gamepc",		GAME_BASEFILE,	"48b1f3499e2e0d731047f4d481ff7817", -1},
	{ "icon.dat",		GAME_ICONFILE,	"22107c24dfb31b66ac503c28a6e20b19", -1},
	{ "simon.gme",		GAME_GMEFILE,	"7db9912acac4f1d965a64bdcfc370ba1", -1},
	{ "stripped.txt",	GAME_STRFILE,	"40d68bec54042ef930f084ad9a4342a1", -1},
	{ "tbllist",		GAME_TBLFILE,	"d198a80de2c59e4a0cd24b98814849e8", -1},
	{ NULL, 0, NULL, 0}
			},
			Common::DE_DEU,
			Common::kPlatformPC,
			ADGF_NO_FLAGS,
			GUIO1(GUIO_NOSUBTITLES)
		},

		GType_SIMON1,
		GID_SIMON1,
		GF_TALKIE
	},

	// Simon the Sorcerer 1 - Hebrew DOS CD
	{
		{
			"simon1",
			"CD",

			{
	{ "gamepc",		GAME_BASEFILE,	"bc66e9c0b296e1b155a246917133f71a", 34348},
	{ "icon.dat",		GAME_ICONFILE,	"22107c24dfb31b66ac503c28a6e20b19", 14361},
	{ "simon.gme",		GAME_GMEFILE,	"a34b2c8642f2e3676d7088b5c8b3e884", 6976948},
	{ "stripped.txt",	GAME_STRFILE,	"9d31bef42db1a8abe4e9f368014df1d5", 252},
	{ "tbllist",		GAME_TBLFILE,	"d198a80de2c59e4a0cd24b98814849e8", 711},
	{ NULL, 0, NULL, 0}
			},
			Common::HE_ISR,
			Common::kPlatformPC,
			ADGF_CD,
			GUIO0()
		},

		GType_SIMON1,
		GID_SIMON1,
		GF_TALKIE
	},

	// Simon the Sorcerer 1 - Italian DOS CD
	{
		{
			"simon1",
			"CD",

			{
	{ "gamepc",		GAME_BASEFILE,	"8d3ca654e158c91b860c7eae31d65312", 37807},
	{ "icon.dat",		GAME_ICONFILE,	"22107c24dfb31b66ac503c28a6e20b19", 14361},
	{ "simon.gme",		GAME_GMEFILE,	"52e315e0e02feca86d15cc82e3306b6c", 7035767},
	{ "stripped.txt",	GAME_STRFILE,	"9d31bef42db1a8abe4e9f368014df1d5", 252},
	{ "tbllist",		GAME_TBLFILE,	"d198a80de2c59e4a0cd24b98814849e8", 711},
	{ NULL, 0, NULL, 0}
			},
			Common::IT_ITA,
			Common::kPlatformPC,
			ADGF_CD,
			GUIO0()
		},

		GType_SIMON1,
		GID_SIMON1,
		GF_TALKIE
	},

	// Simon the Sorcerer 1 - Italian DOS CD alternate
	{
		{
			"simon1",
			"CD",

			{
	{ "gamepc",		GAME_BASEFILE,	"8d3ca654e158c91b860c7eae31d65312", -1},
	{ "icon.dat",		GAME_ICONFILE,	"22107c24dfb31b66ac503c28a6e20b19", -1},
	{ "simon.gme",		GAME_GMEFILE,	"104efd83c8f3edf545982e07d87f66ac", -1},
	{ "stripped.txt",	GAME_STRFILE,	"9d31bef42db1a8abe4e9f368014df1d5", -1},
	{ "tbllist",		GAME_TBLFILE,	"d198a80de2c59e4a0cd24b98814849e8", -1},
	{ NULL, 0, NULL, 0}
			},
			// FIXME: DOS version which uses WAV format
			Common::IT_ITA,
			Common::kPlatformWindows,
			ADGF_CD,
			GUIO0()
		},

		GType_SIMON1,
		GID_SIMON1,
		GF_TALKIE
	},

	// Simon the Sorcerer 1 - Spanish DOS CD
	{
		{
			"simon1",
			"CD",

			{
	{ "gamepc",		GAME_BASEFILE,	"439f801ba52c02c9d1844600d1ce0f5e", 37847},
	{ "icon.dat",		GAME_ICONFILE,	"22107c24dfb31b66ac503c28a6e20b19", 14361},
	{ "simon.gme",		GAME_GMEFILE,	"eff2774a73890b9eac533db90cd1afa1", 7030485},
	{ "stripped.txt",	GAME_STRFILE,	"9d31bef42db1a8abe4e9f368014df1d5", 252},
	{ "tbllist",		GAME_TBLFILE,	"d198a80de2c59e4a0cd24b98814849e8", 711},
	{ NULL, 0, NULL, 0}
			},
			Common::ES_ESP,
			Common::kPlatformPC,
			ADGF_CD,
			GUIO0()
		},

		GType_SIMON1,
		GID_SIMON1,
		GF_TALKIE
	},

	// Simon the Sorcerer 1 - English Windows CD
	{
		{
			"simon1",
			"CD",

			{
	{ "gamepc",		GAME_BASEFILE,	"c7c12fea7f6d0bfd22af5cdbc8166862", 36152},
	{ "icon.dat",		GAME_ICONFILE,	"22107c24dfb31b66ac503c28a6e20b19", 14361},
	{ "simon.gme",		GAME_GMEFILE,	"b1b18d0731b64c0738c5cc4a2ee792fc", 7030377},
	{ "stripped.txt",	GAME_STRFILE,	"a27e87a9ba21212d769804b3df47bfb2", 252},
	{ "tbllist",		GAME_TBLFILE,	"d198a80de2c59e4a0cd24b98814849e8", 711},
	{ NULL, 0, NULL, 0}
			},
			Common::EN_ANY,
			Common::kPlatformWindows,
			ADGF_CD,
			GUIO1(GUIO_NOSUBTITLES)
		},

		GType_SIMON1,
		GID_SIMON1,
		GF_TALKIE
	},

	// Simon the Sorcerer 1 - German Windows CD
	{
		{
			"simon1",
			"CD",

			{
	{ "gamepc",		GAME_BASEFILE,	"48b1f3499e2e0d731047f4d481ff7817", -1},
	{ "icon.dat",		GAME_ICONFILE,	"22107c24dfb31b66ac503c28a6e20b19", -1},
	{ "simon.gme",		GAME_GMEFILE,	"acd9cc438525b142d93b15c77a6f551b", -1},
	{ "stripped.txt",	GAME_STRFILE,	"40d68bec54042ef930f084ad9a4342a1", -1},
	{ "tbllist",		GAME_TBLFILE,	"d198a80de2c59e4a0cd24b98814849e8", -1},
	{ NULL, 0, NULL, 0}
			},
			Common::DE_DEU,
			Common::kPlatformWindows,
			ADGF_CD,
			GUIO1(GUIO_NOSUBTITLES)
		},

		GType_SIMON1,
		GID_SIMON1,
		GF_TALKIE
	},

	// Simon the Sorcerer 2 - English DOS Floppy
	{
		{
			"simon2",
			"Floppy",

			{
	{ "game32",		GAME_BASEFILE,	"604d04315935e77624bd356ac926e068", -1},
	{ "icon.dat",		GAME_ICONFILE,	"72096a62d36e6034ea9fecc13b2dbdab", -1},
	{ "simon2.gme",		GAME_GMEFILE,	"aa6840420899a31874204f90bb214108", -1},
	{ "stripped.txt",	GAME_STRFILE,	"e229f84d46fa83f99b4a7115679f3fb6", -1},
	{ "tbllist",		GAME_TBLFILE,	"2082f8d02075e590300478853a91ffd9", -1},
	{ NULL, 0, NULL, 0}
			},
			Common::EN_ANY,
			Common::kPlatformPC,
			ADGF_NO_FLAGS,
			GUIO1(GUIO_NOSPEECH)
		},

		GType_SIMON2,
		GID_SIMON2,
		0
	},

	// Simon the Sorcerer 2 - English DOS Floppy with Russian patch
	{
		{
			"simon2",
			"Floppy",

			{
	{ "game32",		GAME_BASEFILE,	"7edfc633dd50f8caa719c478443db70b", -1},
	{ "icon.dat",		GAME_ICONFILE,	"72096a62d36e6034ea9fecc13b2dbdab", -1},
	{ "simon2.gme",		GAME_GMEFILE,	"aa6840420899a31874204f90bb214108", -1},
	{ "stripped.txt",	GAME_STRFILE,	"e229f84d46fa83f99b4a7115679f3fb6", -1},
	{ "tbllist",		GAME_TBLFILE,	"2082f8d02075e590300478853a91ffd9", -1},
	{ NULL, 0, NULL, 0}
			},
			Common::RU_RUS,
			Common::kPlatformPC,
			ADGF_NO_FLAGS,
			GUIO1(GUIO_NOSPEECH)
		},

		GType_SIMON2,
		GID_SIMON2,
		0
	},

	// Simon the Sorcerer 2 - German DOS Floppy
	{
		{
			"simon2",
			"Floppy",

			{
	{ "game32",		GAME_BASEFILE,	"eb6e3e37fe52993f948d7e2d6b869828", -1},
	{ "icon.dat",		GAME_ICONFILE,	"72096a62d36e6034ea9fecc13b2dbdab", -1},
	{ "simon2.gme",		GAME_GMEFILE,	"5fa9d080b04c610f526bd685be1bf747", -1},
	{ "stripped.txt",	GAME_STRFILE,	"fd30df01cc248ecbaef302af855e0212", -1},
	{ "tbllist",		GAME_TBLFILE,	"2082f8d02075e590300478853a91ffd9", -1},
	{ NULL, 0, NULL, 0}
			},
			Common::DE_DEU,
			Common::kPlatformPC,
			ADGF_NO_FLAGS,
			GUIO1(GUIO_NOSPEECH)
		},

		GType_SIMON2,
		GID_SIMON2,
		0
	},

	// Simon the Sorcerer 2 - German DOS Floppy alternate?
	{
		{
			"simon2",
			"Floppy",

			{
	{ "game32",		GAME_BASEFILE,	"604d04315935e77624bd356ac926e068", 59111},
	{ "icon.dat",		GAME_ICONFILE,	"72096a62d36e6034ea9fecc13b2dbdab", 18089},
	{ "simon2.gme",		GAME_GMEFILE,	"ec9f0f24fd895e7ea72e3c8e448c0240", 19698755},
	{ "stripped.txt",	GAME_STRFILE,	"6de6292c9ac11bfb2e70fdb0f773ba85", 171},
	{ "tbllist",		GAME_TBLFILE,	"2082f8d02075e590300478853a91ffd9", 513},
	{ NULL, 0, NULL, 0}
			},
			Common::DE_DEU,
			Common::kPlatformPC,
			ADGF_NO_FLAGS,
			GUIO1(GUIO_NOSPEECH)
		},

		GType_SIMON2,
		GID_SIMON2,
		0
	},

	// Simon the Sorcerer 2 - Italian DOS Floppy
	{
		{
			"simon2",
			"Floppy",

			{
	{ "game32",		GAME_BASEFILE,	"3e11d400bea0638f360a724687005cd1", -1},
	{ "icon.dat",		GAME_ICONFILE,	"72096a62d36e6034ea9fecc13b2dbdab", -1},
	{ "simon2.gme",		GAME_GMEFILE,	"f306a397565d7f13bec7ecf14c723de7", -1},
	{ "stripped.txt",	GAME_STRFILE,	"bea6843fb9f3b2144fcb146d62db0b9a", -1},
	{ "tbllist",		GAME_TBLFILE,	"2082f8d02075e590300478853a91ffd9", -1},
	{ NULL, 0, NULL, 0}
			},
			Common::IT_ITA,
			Common::kPlatformPC,
			ADGF_NO_FLAGS,
			GUIO1(GUIO_NOSPEECH)
		},

		GType_SIMON2,
		GID_SIMON2,
		0
	},

	// Simon the Sorcerer 2 - English DOS CD Demo
	{
		{
			"simon2",
			"CD Demo",

			{
	{ "gsptr30",		GAME_BASEFILE,	"3794c15887539b8578bacab694ccf08a", 58757},
	{ "icon.dat",		GAME_ICONFILE,	"72096a62d36e6034ea9fecc13b2dbdab", 18089},
	{ "simon2.gme",		GAME_GMEFILE,	"f8c9e6df1e55923a749e115ba74210c4", 4187270},
	{ "stripped.txt",	GAME_STRFILE,	"e229f84d46fa83f99b4a7115679f3fb6", 171},
	{ "tbllist",		GAME_TBLFILE,	"a0d5a494b5d3d209d1a1d76cc8d76601", 513},
	{ NULL, 0, NULL, 0}
			},
			Common::EN_ANY,
			Common::kPlatformPC,
			ADGF_DEMO,
			GUIO0()
		},

		GType_SIMON2,
		GID_SIMON2,
		GF_TALKIE
	},

	// Simon the Sorcerer 2 - German DOS CD Demo
	{
		{
			"simon2",
			"CD Demo",

			{
	{ "gsptr30",		GAME_BASEFILE,	"7596ef2644fde56ee5fad0dcd483a01e", -1},
	{ "icon.dat",		GAME_ICONFILE,	"72096a62d36e6034ea9fecc13b2dbdab", -1},
	{ "simon2.gme",		GAME_GMEFILE,	"3f308f201f6b1ddf7c2704c1fc43a3e1", -1},
	{ "stripped.txt",	GAME_STRFILE,	"4c68cf64e581a9bd638a56c900b08bfe", -1},
	{ "tbllist",		GAME_TBLFILE,	"a0d5a494b5d3d209d1a1d76cc8d76601", -1},
	{ NULL, 0, NULL, 0}
			},
			Common::DE_DEU,
			Common::kPlatformPC,
			ADGF_DEMO,
			GUIO0()
		},

		GType_SIMON2,
		GID_SIMON2,
		GF_TALKIE
	},

	// Simon the Sorcerer 2 - German DOS CD Non-Interactive Demo
	{
		{
			"simon2",
			"CD Non-Interactive Demo",

			{
	{ "gsptr30",		GAME_BASEFILE,	"c45facd0605860684d464b6a62323567", -1},
	{ "icon.dat",		GAME_ICONFILE,	"72096a62d36e6034ea9fecc13b2dbdab", -1},
	{ "simon2.gme",		GAME_GMEFILE,	"30ea02d374327cab6c78444f3c198c1c", -1},
	{ "stripped.txt",	GAME_STRFILE,	"4c68cf64e581a9bd638a56c900b08bfe", -1},
	{ "tbllist",		GAME_TBLFILE,	"a0d5a494b5d3d209d1a1d76cc8d76601", -1},
	{ NULL, 0, NULL, 0}
			},
			Common::DE_DEU,
			Common::kPlatformPC,
			ADGF_DEMO,
			GUIO0()
		},

		GType_SIMON2,
		GID_SIMON2,
		GF_TALKIE
	},

	// Simon the Sorcerer 2 - English DOS CD
	{
		{
			"simon2",
			"CD",

			{
	{ "gsptr30",		GAME_BASEFILE,	"8c301fb9c4fcf119d2730ccd2a565eb3", 58652},
	{ "icon.dat",		GAME_ICONFILE,	"72096a62d36e6034ea9fecc13b2dbdab", 18089},
	{ "simon2.gme",		GAME_GMEFILE,	"9c535d403966750ae98bdaf698375a38", 19687892},
	{ "stripped.txt",	GAME_STRFILE,	"e229f84d46fa83f99b4a7115679f3fb6", 171},
	{ "tbllist",		GAME_TBLFILE,	"2082f8d02075e590300478853a91ffd9", 513},
	{ NULL, 0, NULL, 0}
			},
			Common::EN_ANY,
			Common::kPlatformPC,
			ADGF_CD,
			GUIO0()
		},

		GType_SIMON2,
		GID_SIMON2,
		GF_TALKIE
	},

	// Simon the Sorcerer 2 - English DOS CD alternate?
	{
		{
			"simon2",
			"CD",

			{
	{ "gsptr30",		GAME_BASEFILE,	"608e277904d87dd28725fa08eacc2c0d", -1},
	{ "icon.dat",		GAME_ICONFILE,	"72096a62d36e6034ea9fecc13b2dbdab", -1},
	{ "simon2.gme",		GAME_GMEFILE,	"8d6dcc65577e285dbca03ff6d7d9323c", -1},
	{ "stripped.txt",	GAME_STRFILE,	"e229f84d46fa83f99b4a7115679f3fb6", -1},
	{ "tbllist",		GAME_TBLFILE,	"a0d5a494b5d3d209d1a1d76cc8d76601", -1},
	{ NULL, 0, NULL, 0}
			},
			Common::EN_ANY,
			Common::kPlatformPC,
			ADGF_CD,
			GUIO0()
		},

		GType_SIMON2,
		GID_SIMON2,
		GF_TALKIE
	},

	// Simon the Sorcerer 2 - French DOS CD
	{
		{
			"simon2",
			"CD",

			{
	{ "gsptr30",		GAME_BASEFILE,	"43b3a04d2f0a0cbd1b024c814856561a", -1},
	{ "icon.dat",		GAME_ICONFILE,	"72096a62d36e6034ea9fecc13b2dbdab", -1},
	{ "simon2.gme",		GAME_GMEFILE,	"8af0e02c0c3344db64dffc12196eb59d", -1},
	{ "stripped.txt",	GAME_STRFILE,	"5ea27977b4d7dcfd50eb5074e162ebbf", -1},
	{ "tbllist",		GAME_TBLFILE,	"2082f8d02075e590300478853a91ffd9", -1},
	{ NULL, 0, NULL, 0}
			},
			Common::FR_FRA,
			Common::kPlatformPC,
			ADGF_CD,
			GUIO0()
		},

		GType_SIMON2,
		GID_SIMON2,
		GF_TALKIE
	},

	// Simon the Sorcerer 2 - German DOS CD
	{
		{
			"simon2",
			"CD",

			{
	{ "gsptr30",		GAME_BASEFILE,	"0d05c3f4c06c9a4ceb3d2f5bc0b18e11", -1},
	{ "icon.dat",		GAME_ICONFILE,	"72096a62d36e6034ea9fecc13b2dbdab", -1},
	{ "simon2.gme",		GAME_GMEFILE,	"6c5fdfdd0eab9038767c2d22858406b2", -1},
	{ "stripped.txt",	GAME_STRFILE,	"6de6292c9ac11bfb2e70fdb0f773ba85", -1},
	{ "tbllist",		GAME_TBLFILE,	"2082f8d02075e590300478853a91ffd9", -1},
	{ NULL, 0, NULL, 0}
			},
			Common::DE_DEU,
			Common::kPlatformPC,
			ADGF_CD,
			GUIO0()
		},

		GType_SIMON2,
		GID_SIMON2,
		GF_TALKIE
	},

	// Simon the Sorcerer 2 - German DOS CD alternate?
	{
		{
			"simon2",
			"CD",

			{
	{ "gsptr30",		GAME_BASEFILE,	"a76ea940076b5d9316796dea225a9b69", -1},
	{ "icon.dat",		GAME_ICONFILE,	"72096a62d36e6034ea9fecc13b2dbdab", -1},
	{ "simon2.gme",		GAME_GMEFILE,	"ec9f0f24fd895e7ea72e3c8e448c0240", -1},
	{ "stripped.txt",	GAME_STRFILE,	"6de6292c9ac11bfb2e70fdb0f773ba85", -1},
	{ "tbllist",		GAME_TBLFILE,	"2082f8d02075e590300478853a91ffd9", -1},
	{ NULL, 0, NULL, 0}
			},
			Common::DE_DEU,
			Common::kPlatformPC,
			ADGF_CD,
			GUIO0()
		},

		GType_SIMON2,
		GID_SIMON2,
		GF_TALKIE
	},

	// Simon the Sorcerer 2 - Hebrew DOS CD
	{
		{
			"simon2",
			"CD",

			{
	{ "gsptr30",		GAME_BASEFILE,	"952a2b1be23c3c609ba8d988a9a1627d", 53366},
	{ "icon.dat",		GAME_ICONFILE,	"72096a62d36e6034ea9fecc13b2dbdab", 18089},
	{ "simon2.gme",		GAME_GMEFILE,	"a2b249a82ea182af09789eb95fb6c5be", 19650639},
	{ "stripped.txt",	GAME_STRFILE,	"de9dbc24158660e153483fa0cf6c3172", 171},
	{ "tbllist",		GAME_TBLFILE,	"2082f8d02075e590300478853a91ffd9", 513},
	{ NULL, 0, NULL, 0}
			},
			Common::HE_ISR,
			Common::kPlatformPC,
			ADGF_CD,
			GUIO0()
		},

		GType_SIMON2,
		GID_SIMON2,
		GF_TALKIE
	},

	// Simon the Sorcerer 2 - Italian DOS CD
	{
		{
			"simon2",
			"CD",

			{
	{ "gsptr30",		GAME_BASEFILE,	"3e11d400bea0638f360a724687005cd1", -1},
	{ "icon.dat",		GAME_ICONFILE,	"72096a62d36e6034ea9fecc13b2dbdab", -1},
	{ "simon2.gme",		GAME_GMEFILE,	"344aca58e5ad5e25c517d5eb1d85c435", -1},
	{ "stripped.txt",	GAME_STRFILE,	"bea6843fb9f3b2144fcb146d62db0b9a", -1},
	{ "tbllist",		GAME_TBLFILE,	"2082f8d02075e590300478853a91ffd9", -1},
	{ NULL, 0, NULL, 0}
			},
			// FIXME: DOS version which uses WAV format
			Common::IT_ITA,
			Common::kPlatformWindows,
			ADGF_CD,
			GUIO0()
		},

		GType_SIMON2,
		GID_SIMON2,
		GF_TALKIE
	},

	// Simon the Sorcerer 2 - Spanish DOS CD
	{
		{
			"simon2",
			"CD",

			{
	{ "gsptr30",		GAME_BASEFILE,	"268dc322aa73bcf27bb016b8e8ceb889", -1},
	{ "icon.dat",		GAME_ICONFILE,	"72096a62d36e6034ea9fecc13b2dbdab", -1},
	{ "simon2.gme",		GAME_GMEFILE,	"4f43bd06b6cc78dbd25a7475ca964eb1", -1},
	{ "stripped.txt",	GAME_STRFILE,	"d13753796bd81bf313a2449f34d8b112", -1},
	{ "tbllist",		GAME_TBLFILE,	"2082f8d02075e590300478853a91ffd9", -1},
	{ NULL, 0, NULL, 0}
			},
			Common::ES_ESP,
			Common::kPlatformPC,
			ADGF_CD,
			GUIO0()
		},

		GType_SIMON2,
		GID_SIMON2,
		GF_TALKIE
	},

	// Simon the Sorcerer 2 - Czech Windows CD
	{
		{
			"simon2",
			"CD",

			{
	{ "gsptr30",		GAME_BASEFILE,	"eb8217f9ec4628d12ca606033146c48c", -1},
	{ "icon.dat",		GAME_ICONFILE,	"72096a62d36e6034ea9fecc13b2dbdab", -1},
	{ "simon2.gme",		GAME_GMEFILE,	"2d1074788501c55dcd9e59269ea0aaed", -1},
	{ "stripped.txt",	GAME_STRFILE,	"e229f84d46fa83f99b4a7115679f3fb6", -1},
	{ "tbllist",		GAME_TBLFILE,	"2082f8d02075e590300478853a91ffd9", -1},
	{ NULL, 0, NULL, 0}
			},
			Common::CZ_CZE,
			Common::kPlatformWindows,
			ADGF_CD,
			GUIO0()
		},

		GType_SIMON2,
		GID_SIMON2,
		GF_TALKIE
	},

	// Simon the Sorcerer 2 - English Windows CD
	{
		{
			"simon2",
			"CD",

			{
	{ "gsptr30",		GAME_BASEFILE,	"608e277904d87dd28725fa08eacc2c0d", 58652},
	{ "icon.dat",		GAME_ICONFILE,	"72096a62d36e6034ea9fecc13b2dbdab", 18089},
	{ "simon2.gme",		GAME_GMEFILE,	"e749c4c103d7e7d51b34620ed76c5a04", 20046789},
	{ "stripped.txt",	GAME_STRFILE,	"e229f84d46fa83f99b4a7115679f3fb6", 171},
	{ "tbllist",		GAME_TBLFILE,	"2082f8d02075e590300478853a91ffd9", 513},
	{ NULL, 0, NULL, 0}
			},
			Common::EN_ANY,
			Common::kPlatformWindows,
			ADGF_CD,
			GUIO0()
		},

		GType_SIMON2,
		GID_SIMON2,
		GF_TALKIE
	},

	// Simon the Sorcerer 2 - French Windows CD
	{
		{
			"simon2",
			"CD",

			{
	{ "gsptr30",		GAME_BASEFILE,	"6e217d75f4089b92270fad879e091e29", -1},
	{ "icon.dat",		GAME_ICONFILE,	"72096a62d36e6034ea9fecc13b2dbdab", -1},
	{ "simon2.gme",		GAME_GMEFILE,	"1d2f867a7eba818c85c1e4849821e812", -1},
	{ "stripped.txt",	GAME_STRFILE,	"5ea27977b4d7dcfd50eb5074e162ebbf", -1},
	{ "tbllist",		GAME_TBLFILE,	"2082f8d02075e590300478853a91ffd9", -1},
	{ NULL, 0, NULL, 0}
			},
			Common::FR_FRA,
			Common::kPlatformWindows,
			ADGF_CD,
			GUIO0()
		},

		GType_SIMON2,
		GID_SIMON2,
		GF_TALKIE
	},

	// Simon the Sorcerer 2 - German Windows CD
	{
		{
			"simon2",
			"CD",

			{
	{ "gsptr30",		GAME_BASEFILE,	"a76ea940076b5d9316796dea225a9b69", -1},
	{ "icon.dat",		GAME_ICONFILE,	"72096a62d36e6034ea9fecc13b2dbdab", -1},
	{ "simon2.gme",		GAME_GMEFILE,	"9609a933c541fed2e00c6c3479d7c181", -1},
	{ "stripped.txt",	GAME_STRFILE,	"6de6292c9ac11bfb2e70fdb0f773ba85", -1},
	{ "tbllist",		GAME_TBLFILE,	"2082f8d02075e590300478853a91ffd9", -1},
	{ NULL, 0, NULL, 0}
			},
			Common::DE_DEU,
			Common::kPlatformWindows,
			ADGF_CD,
			GUIO0()
		},

		GType_SIMON2,
		GID_SIMON2,
		GF_TALKIE
	},

	// Simon the Sorcerer 2 - Polish Windows CD
	{
		{
			"simon2",
			"CD",

			{
	{ "gsptr30",		GAME_BASEFILE,	"657fd873f5d0637097ee02315b447e6f", -1},
	{ "icon.dat",		GAME_ICONFILE,	"72096a62d36e6034ea9fecc13b2dbdab", -1},
	{ "simon2.gme",		GAME_GMEFILE,	"7b9afcf82a94722707e0d025c0192be8", -1},
	{ "stripped.txt",	GAME_STRFILE,	"e229f84d46fa83f99b4a7115679f3fb6", -1},
	{ "tbllist",		GAME_TBLFILE,	"2082f8d02075e590300478853a91ffd9", -1},
	{ NULL, 0, NULL, 0}
			},
			Common::PL_POL,
			Common::kPlatformWindows,
			ADGF_CD,
			GUIO0()
		},

		GType_SIMON2,
		GID_SIMON2,
		GF_TALKIE
	},

#ifdef ENABLE_AGOS2
	// The Feeble Files - English DOS Demo
	{
		{
			"feeble",
			"Demo",

			{
	{ "MAINMENU.SMK",		GAME_VGAFILE,	"b62df52fc36f514eb0464120853f22b6", 968808},
	{ NULL, 0, NULL, 0}
			},
			Common::EN_ANY,
			Common::kPlatformPC,
			ADGF_DEMO,
			GUIO3(GUIO_NOSUBTITLES, GUIO_NOMUSIC, GUIO_NOASPECT)
		},

		GType_FF,
		GID_FEEBLEFILES,
		GF_DEMO
	},

	// The Feeble Files - German DOS Demo
	{
		{
			"feeble",
			"Demo",

			{
	{ "MAINMENU.SMK",		GAME_VGAFILE,	"e18d365044eabea7352934917bbfd2e4", 976436},
	{ NULL, 0, NULL, 0}
			},
			Common::DE_DEU,
			Common::kPlatformPC,
			ADGF_DEMO,
			GUIO3(GUIO_NOSUBTITLES, GUIO_NOMUSIC, GUIO_NOASPECT)
		},

		GType_FF,
		GID_FEEBLEFILES,
		GF_DEMO
	},

	// The Feeble Files - English Amiga CD
	{
		{
			"feeble",
			"CD",

			{
	{ "game22",		GAME_BASEFILE,	"629762ea9ca9ee9ff85f4774d219f5c7", 259576},
	{ "gfxindex.dat",	GAME_GFXIDXFILE,"f550f7915c5ce3a68c9f870f507449c2", 48000},
	{ "setup",		GAME_RESTFILE,	"71512fc98501a8071a26b683a31dde78", 3360},
	{ "tbllist",		GAME_TBLFILE,	"0bbfee8e69739111eb36b0d138da8ddf", 668},
	{ NULL, 0, NULL, 0}
			},
			Common::EN_ANY,
			Common::kPlatformAmiga,
			ADGF_NO_FLAGS,
			GUIO3(GUIO_NOSUBTITLES, GUIO_NOMUSIC, GUIO_NOASPECT)
		},

		GType_FF,
		GID_FEEBLEFILES,
		GF_OLD_BUNDLE | GF_ZLIBCOMP | GF_TALKIE
	},

	// The Feeble Files - German Amiga CD
	{
		{
			"feeble",
			"CD",

			{
	{ "game22",		GAME_BASEFILE,	"bcd76ac080003eee3649df18db25b60e", 262517},
	{ "gfxindex.dat",	GAME_GFXIDXFILE,"f550f7915c5ce3a68c9f870f507449c2", 48000},
	{ "setup",		GAME_RESTFILE,	"71512fc98501a8071a26b683a31dde78", 3360},
	{ "tbllist",		GAME_TBLFILE,	"0bbfee8e69739111eb36b0d138da8ddf", 668},
	{ NULL, 0, NULL, 0}
			},
			Common::DE_DEU,
			Common::kPlatformAmiga,
			ADGF_NO_FLAGS,
			GUIO3(GUIO_NOSUBTITLES, GUIO_NOMUSIC, GUIO_NOASPECT)
		},

		GType_FF,
		GID_FEEBLEFILES,
		GF_OLD_BUNDLE | GF_ZLIBCOMP | GF_TALKIE
	},

	// The Feeble Files - English Macintosh CD
	{
		{
			"feeble",
			"CD",

			{
	{ "game22",		GAME_BASEFILE,	"629762ea9ca9ee9ff85f4774d219f5c7", 259576},
	{ "graphics.vga",	GAME_GFXIDXFILE,"11a4853cb35956846976e9473ee0e41e", 67456693},
	{ "setup",		GAME_RESTFILE,	"71512fc98501a8071a26b683a31dde78", 3360},
	{ "tbllist",		GAME_TBLFILE,	"0bbfee8e69739111eb36b0d138da8ddf", 668},
	{ NULL, 0, NULL, 0}
			},
			Common::EN_ANY,
			Common::kPlatformMacintosh,
			ADGF_NO_FLAGS,
			GUIO3(GUIO_NOSUBTITLES, GUIO_NOMUSIC, GUIO_NOASPECT)
		},

		GType_FF,
		GID_FEEBLEFILES,
		GF_OLD_BUNDLE | GF_ZLIBCOMP | GF_TALKIE
	},

	// The Feeble Files - French Macintosh CD
	{
		{
			"feeble",
			"CD",

			{
	{ "game22",		GAME_BASEFILE,	"ba90b40a47726039671d9e91630dd7ed", 259668},
	{ "graphics.vga",	GAME_GFXIDXFILE,"11a4853cb35956846976e9473ee0e41e", 67456693},
	{ "setup",		GAME_RESTFILE,	"71512fc98501a8071a26b683a31dde78", 3360},
	{ "tbllist",		GAME_TBLFILE,	"0bbfee8e69739111eb36b0d138da8ddf", 668},
	{ NULL, 0, NULL, 0}
			},
			Common::FR_FRA,
			Common::kPlatformMacintosh,
			ADGF_NO_FLAGS,
			GUIO3(GUIO_NOSUBTITLES, GUIO_NOMUSIC, GUIO_NOASPECT)
		},

		GType_FF,
		GID_FEEBLEFILES,
		GF_OLD_BUNDLE | GF_ZLIBCOMP | GF_TALKIE
	},

	// The Feeble Files - German Macintosh CD
	{
		{
			"feeble",
			"CD",

			{
	{ "game22",		GAME_BASEFILE,	"bcd76ac080003eee3649df18db25b60e", 262517},
	{ "graphics.vga",	GAME_GFXIDXFILE,"11a4853cb35956846976e9473ee0e41e", 67456693},
	{ "setup",		GAME_RESTFILE,	"71512fc98501a8071a26b683a31dde78", 3360},
	{ "tbllist",		GAME_TBLFILE,	"0bbfee8e69739111eb36b0d138da8ddf", 668},
	{ NULL, 0, NULL, 0}
			},
			Common::DE_DEU,
			Common::kPlatformMacintosh,
			ADGF_NO_FLAGS,
			GUIO3(GUIO_NOSUBTITLES, GUIO_NOMUSIC, GUIO_NOASPECT)
		},

		GType_FF,
		GID_FEEBLEFILES,
		GF_OLD_BUNDLE | GF_ZLIBCOMP | GF_TALKIE
	},

	// The Feeble Files - Spanish Macintosh CD
	{
		{
			"feeble",
			"CD",

			{
	{ "game22",		GAME_BASEFILE,	"71d7d2d5e479b053c5a9757f1702c9c3", 265629},
	{ "graphics.vga",	GAME_GFXIDXFILE,"11a4853cb35956846976e9473ee0e41e", 67456693},
	{ "setup",		GAME_RESTFILE,	"71512fc98501a8071a26b683a31dde78", 3360},
	{ "tbllist",		GAME_TBLFILE,	"0bbfee8e69739111eb36b0d138da8ddf", 668},
	{ NULL, 0, NULL, 0}
			},
			Common::ES_ESP,
			Common::kPlatformMacintosh,
			ADGF_NO_FLAGS,
			GUIO3(GUIO_NOSUBTITLES, GUIO_NOMUSIC, GUIO_NOASPECT)
		},

		GType_FF,
		GID_FEEBLEFILES,
		GF_OLD_BUNDLE | GF_ZLIBCOMP | GF_TALKIE
	},

	// The Feeble Files - English Windows 2CD (with InstallShield cab)
	{
		{
			"feeble",
			"2CD",

			{
	{ "data1.cab",		0,	"600db08891e7a21badc8215e604cd88f", 28845430},
	{ NULL, 0, NULL, 0}
			},
			Common::EN_ANY,
			Common::kPlatformWindows,
			ADGF_NO_FLAGS,
			GUIO3(GUIO_NOSUBTITLES, GUIO_NOMUSIC, GUIO_NOASPECT)
		},

		GType_FF,
		GID_FEEBLEFILES,
		GF_OLD_BUNDLE | GF_TALKIE | GF_PACKED
	},

	// The Feeble Files - English Windows 2CD
	{
		{
			"feeble",
			"2CD",

			{
	{ "game22",		GAME_BASEFILE,	"629762ea9ca9ee9ff85f4774d219f5c7", 259576},
	{ "save.999",		GAME_RESTFILE,	"71512fc98501a8071a26b683a31dde78", 3360},
	{ "tbllist",		GAME_TBLFILE,	"0bbfee8e69739111eb36b0d138da8ddf", 668},
	{ NULL, 0, NULL, 0}
			},
			Common::EN_ANY,
			Common::kPlatformWindows,
			ADGF_NO_FLAGS,
			GUIO3(GUIO_NOSUBTITLES, GUIO_NOMUSIC, GUIO_NOASPECT)
		},

		GType_FF,
		GID_FEEBLEFILES,
		GF_OLD_BUNDLE | GF_TALKIE
	},

	// The Feeble Files - Polish Windows 2CD
	{
		{
			"feeble",
			"2CD",

			{
	{ "game33",		GAME_BASEFILE,	"cda37c422c04dde8b8ab3405178b3ef9", 266565},
	{ "save.999",		GAME_RESTFILE,	"71512fc98501a8071a26b683a31dde78", 3360},
	{ "tbllist",		GAME_TBLFILE,	"0bbfee8e69739111eb36b0d138da8ddf", 668},
	{ NULL, 0, NULL, 0}
			},
			Common::PL_POL,
			Common::kPlatformWindows,
			ADGF_NO_FLAGS,
			GUIO3(GUIO_NOSUBTITLES, GUIO_NOMUSIC, GUIO_NOASPECT)
		},

		GType_FF,
		GID_FEEBLEFILES,
		GF_OLD_BUNDLE | GF_TALKIE
	},

	// The Feeble Files - English Windows 4CD (with InstallShield cab)
	{
		{
			"feeble",
			"4CD",

			{
	{ "data1.cab",		0,	"65804cbc9036ac4b1275d97e0de3be2f", 28943062},
	{ NULL, 0, NULL, 0}
			},
			Common::EN_ANY,
			Common::kPlatformWindows,
			ADGF_NO_FLAGS,
			GUIO3(GUIO_NOSUBTITLES, GUIO_NOMUSIC, GUIO_NOASPECT)
		},

		GType_FF,
		GID_FEEBLEFILES,
		GF_OLD_BUNDLE | GF_TALKIE | GF_PACKED
	},

	// The Feeble Files - English Windows 4CD
	{
		{
			"feeble",
			"4CD",

			{
	{ "game22",		GAME_BASEFILE,	"a8746407a5b20a7da0da0a14c380af1c", 251647},
	{ "save.999",		GAME_RESTFILE,	"71512fc98501a8071a26b683a31dde78", 3360},
	{ "tbllist",		GAME_TBLFILE,	"0bbfee8e69739111eb36b0d138da8ddf", 668},
	{ NULL, 0, NULL, 0}
			},
			Common::EN_ANY,
			Common::kPlatformWindows,
			ADGF_NO_FLAGS,
			GUIO3(GUIO_NOSUBTITLES, GUIO_NOMUSIC, GUIO_NOASPECT)
		},

		GType_FF,
		GID_FEEBLEFILES,
		GF_OLD_BUNDLE | GF_TALKIE
	},

	// The Feeble Files - French Windows 4CD
	{
		{
			"feeble",
			"4CD",

			{
	{ "game22",		GAME_BASEFILE,	"ba90b40a47726039671d9e91630dd7ed", -1},
	{ "save.999",		GAME_RESTFILE,	"71512fc98501a8071a26b683a31dde78", -1},
	{ "tbllist",		GAME_TBLFILE,	"0bbfee8e69739111eb36b0d138da8ddf", -1},
	{ NULL, 0, NULL, 0}
			},
			Common::FR_FRA,
			Common::kPlatformWindows,
			ADGF_NO_FLAGS,
			GUIO3(GUIO_NOSUBTITLES, GUIO_NOMUSIC, GUIO_NOASPECT)
		},

		GType_FF,
		GID_FEEBLEFILES,
		GF_OLD_BUNDLE | GF_TALKIE
	},

	// The Feeble Files - German Windows 4CD
	{
		{
			"feeble",
			"4CD",

			{
	{ "game22",		GAME_BASEFILE,	"bcd76ac080003eee3649df18db25b60e", -1},
	{ "save.999",		GAME_RESTFILE,	"71512fc98501a8071a26b683a31dde78", -1},
	{ "tbllist",		GAME_TBLFILE,	"0bbfee8e69739111eb36b0d138da8ddf", -1},
	{ NULL, 0, NULL, 0}
			},
			Common::DE_DEU,
			Common::kPlatformWindows,
			ADGF_NO_FLAGS,
			GUIO3(GUIO_NOSUBTITLES, GUIO_NOMUSIC, GUIO_NOASPECT)
		},

		GType_FF,
		GID_FEEBLEFILES,
		GF_OLD_BUNDLE | GF_TALKIE
	},

	// The Feeble Files - Italian Windows 4CD
	{
		{
			"feeble",
			"4CD",

			{
	{ "game22",		GAME_BASEFILE,	"80576f2e1ed4c912b63921fe77af313e", -1},
	{ "save.999",		GAME_RESTFILE,	"71512fc98501a8071a26b683a31dde78", -1},
	{ "tbllist",		GAME_TBLFILE,	"0bbfee8e69739111eb36b0d138da8ddf", -1},
	{ NULL, 0, NULL, 0}
			},
			Common::IT_ITA,
			Common::kPlatformWindows,
			ADGF_NO_FLAGS,
			GUIO3(GUIO_NOSUBTITLES, GUIO_NOMUSIC, GUIO_NOASPECT)
		},

		GType_FF,
		GID_FEEBLEFILES,
		GF_OLD_BUNDLE | GF_TALKIE
	},

	// The Feeble Files - Polish Windows 4CD
	{
		{
			"feeble",
			"4CD",

			{
	{ "game22",		GAME_BASEFILE,	"c498f892a5dbcbc968e0dbb8697884fc", 258660},
	{ "save.999",		GAME_RESTFILE,	"71512fc98501a8071a26b683a31dde78", 3360},
	{ "tbllist",		GAME_TBLFILE,	"0bbfee8e69739111eb36b0d138da8ddf", 668},
	{ NULL, 0, NULL, 0}
			},
			Common::PL_POL,
			Common::kPlatformWindows,
			ADGF_NO_FLAGS,
			GUIO3(GUIO_NOSUBTITLES, GUIO_NOMUSIC, GUIO_NOASPECT)
		},

		GType_FF,
		GID_FEEBLEFILES,
		GF_OLD_BUNDLE | GF_TALKIE
	},

	// The Feeble Files - Spanish Windows 4CD
	{
		{
			"feeble",
			"4CD",

			{
	{ "game22",		GAME_BASEFILE,	"71d7d2d5e479b053c5a9757f1702c9c3", -1},
	{ "save.999",		GAME_RESTFILE,	"71512fc98501a8071a26b683a31dde78", -1},
	{ "tbllist",		GAME_TBLFILE,	"0bbfee8e69739111eb36b0d138da8ddf", -1},
	{ NULL, 0, NULL, 0}
			},
			Common::ES_ESP,
			Common::kPlatformWindows,
			ADGF_NO_FLAGS,
			GUIO3(GUIO_NOSUBTITLES, GUIO_NOMUSIC, GUIO_NOASPECT)
		},

		GType_FF,
		GID_FEEBLEFILES,
		GF_OLD_BUNDLE | GF_TALKIE
	},

	// Simon the Sorcerer's Puzzle Pack - Demon in my Pocket (with InstallShield cab)
	{
		{
			"dimp",
			"CD",

			{
	{ "data1.cab", 0, "36dd86c1d872cea81ac1de7753dd684a", 40394693},
	{ NULL, 0, NULL, 0}
			},
			Common::EN_ANY,
			Common::kPlatformWindows,
			ADGF_NO_FLAGS,
			GUIO3(GUIO_NOSUBTITLES, GUIO_NOMUSIC, GUIO_NOASPECT)
		},

		GType_PP,
		GID_DIMP,
		GF_OLD_BUNDLE | GF_TALKIE | GF_PACKED
	},

	// Simon the Sorcerer's Puzzle Pack - Demon in my Pocket
	{
		{
			"dimp",
			"CD",

			{
	{ "Gdimp",		GAME_BASEFILE,	"0b1e89ae1dc2e012b7fa7a987b4ac42a", 6208},
	{ NULL, 0, NULL, 0}
			},
			Common::EN_ANY,
			Common::kPlatformWindows,
			ADGF_NO_FLAGS,
			GUIO3(GUIO_NOSUBTITLES, GUIO_NOMUSIC, GUIO_NOASPECT)
		},

		GType_PP,
		GID_DIMP,
		GF_OLD_BUNDLE | GF_TALKIE
	},

	// Simon the Sorcerer's Puzzle Pack - Jumble (with InstallShield cab)
	{
		{
			"jumble",
			"CD",

			{
	{ "data1.cab", 0, "36dd86c1d872cea81ac1de7753dd684a", 40394693},
	{ NULL, 0, NULL, 0}
			},
			Common::EN_ANY,
			Common::kPlatformWindows,
			ADGF_NO_FLAGS,
			GUIO2(GUIO_NOSUBTITLES, GUIO_NOASPECT)
		},

		GType_PP,
		GID_JUMBLE,
		GF_OLD_BUNDLE | GF_TALKIE | GF_PACKED
	},

	// Simon the Sorcerer's Puzzle Pack - Jumble
	{
		{
			"jumble",
			"CD",

			{
	{ "Gjumble",		GAME_BASEFILE,	"d54cce46d339038d1a6b74ea213655bc", 5953},
	{ NULL, 0, NULL, 0}
			},
			Common::EN_ANY,
			Common::kPlatformWindows,
			ADGF_NO_FLAGS,
			GUIO2(GUIO_NOSUBTITLES, GUIO_NOASPECT)
		},

		GType_PP,
		GID_JUMBLE,
		GF_OLD_BUNDLE | GF_TALKIE
	},

	// Simon the Sorcerer's Puzzle Pack - NoPatience (with InstallShield cab)
	{
		{
			"puzzle",
			"CD",

			{
	{ "data1.cab", 0, "36dd86c1d872cea81ac1de7753dd684a", 40394693},
	{ NULL, 0, NULL, 0}
			},
			Common::EN_ANY,
			Common::kPlatformWindows,
			ADGF_NO_FLAGS,
			GUIO2(GUIO_NOSUBTITLES, GUIO_NOASPECT)
		},

		GType_PP,
		GID_PUZZLE,
		GF_OLD_BUNDLE | GF_TALKIE | GF_PACKED
	},

	// Simon the Sorcerer's Puzzle Pack - NoPatience
	{
		{
			"puzzle",
			"CD",

			{
	{ "Gpuzzle",		GAME_BASEFILE,	"3f80dac8e0d85401a1058a560fe49ab6", 23218},
	{ NULL, 0, NULL, 0}
			},
			Common::EN_ANY,
			Common::kPlatformWindows,
			ADGF_NO_FLAGS,
			GUIO2(GUIO_NOSUBTITLES, GUIO_NOASPECT)
		},

		GType_PP,
		GID_PUZZLE,
		GF_OLD_BUNDLE | GF_TALKIE
	},

	// Simon the Sorcerer's Puzzle Pack - Swampy Adventures - English (with InstallShield cab)
	{
		{
			"swampy",
			"CD",

			{
	{ "data1.cab", 0, "36dd86c1d872cea81ac1de7753dd684a", 40394693},
	{ NULL, 0, NULL, 0}
			},
			Common::EN_ANY,
			Common::kPlatformWindows,
			ADGF_NO_FLAGS,
			GUIO2(GUIO_NOSUBTITLES, GUIO_NOASPECT)
		},

		GType_PP,
		GID_SWAMPY,
		GF_OLD_BUNDLE | GF_TALKIE | GF_PACKED
	},

	// Simon the Sorcerer's Puzzle Pack - Swampy Adventures - English
	{
		{
			"swampy",
			"CD",

			{
	{ "Gswampy",		GAME_BASEFILE,	"3a6d4d7b2433e660f2483f9396cc87a2", 30360},
	{ NULL, 0, NULL, 0}
			},
			Common::EN_ANY,
			Common::kPlatformWindows,
			ADGF_NO_FLAGS,
			GUIO2(GUIO_NOSUBTITLES, GUIO_NOASPECT)
		},

		GType_PP,
		GID_SWAMPY,
		GF_OLD_BUNDLE | GF_TALKIE
	},

	// Simon the Sorcerer's Puzzle Pack - Swampy Adventures - German
	{
		{
			"swampy",
			"CD",

			{
	{ "Gswampy",		GAME_BASEFILE,	"4b44f26f51c9c4a16210b4ff9875d24c", -1},
	{ NULL, 0, NULL, 0}
			},
			Common::DE_DEU,
			Common::kPlatformWindows,
			ADGF_NO_FLAGS,
			GUIO2(GUIO_NOSUBTITLES, GUIO_NOASPECT)
		},

		GType_PP,
		GID_SWAMPY,
		GF_OLD_BUNDLE | GF_TALKIE
	},

	// Simon the Sorcerer's Puzzle Pack - Swampy Adventures - Polish
	{
		{
			"swampy",
			"CD",

			{
	{ "Gswampy",		GAME_BASEFILE,	"31bfb5169b47ccc19177e61bd31d4391", -1},
	{ NULL, 0, NULL, 0}
			},
			Common::PL_POL,
			Common::kPlatformWindows,
			ADGF_NO_FLAGS,
			GUIO2(GUIO_NOSUBTITLES, GUIO_NOASPECT)
		},

		GType_PP,
		GID_SWAMPY,
		GF_OLD_BUNDLE | GF_TALKIE
	},

	// Simon the Sorcerer's Puzzle Pack - Swampy Adventures - Spanish
	{
		{
			"swampy",
			"CD",

			{
	{ "Gswampy",		GAME_BASEFILE,	"a534faabea5881d1f350544362edab87", -1},
	{ NULL, 0, NULL, 0}
			},
			Common::ES_ESP,
			Common::kPlatformWindows,
			ADGF_NO_FLAGS,
			GUIO2(GUIO_NOSUBTITLES, GUIO_NOASPECT)
		},

		GType_PP,
		GID_SWAMPY,
		GF_OLD_BUNDLE | GF_TALKIE
	},
#endif
	{ AD_TABLE_END_MARKER, 0, 0, 0 }
};

} // End of namespace AGOS
