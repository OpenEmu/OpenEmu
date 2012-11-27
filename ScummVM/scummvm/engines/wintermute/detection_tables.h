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

namespace Wintermute {

static const PlainGameDescriptor wintermuteGames[] = {
	{"5ld",             "Five Lethal Demons"},
	{"5ma",             "Five Magical Amulets"},
	{"actualdest",      "Actual Destination"},
	{"chivalry",        "Chivalry is Not Dead"},
	{"deadcity",        "Dead City"},
	{"dirtysplit",      "Dirty Split"},
	{"eastside",        "East Side Story"},
	{"ghostsheet",      "Ghost in the Sheet"},
	{"hamlet",          "Hamlet or the last game without MMORPS features, shaders and product placement"},
	{"julia",           "J.U.L.I.A."},
	{"mirage",          "Mirage"},
	{"pigeons",         "Pigeons in the Park"},
	{"reversion",       "Reversion"},
	{"rosemary",        "Rosemary"},
	{"thebox",          "The Box"},
	{"twc",             "the white chamber"},
	{"wintermute",      "Wintermute engine game"},
	{0, 0}
};

static const ADGameDescription gameDescriptions[] = {
	// Five Lethal Demons
	{
		"5ld",
		"",
		AD_ENTRY1s("data.dcp", "1037a77cbd001e0644898addc022322c", 15407750),
		Common::EN_ANY,
		Common::kPlatformWindows,
		ADGF_UNSTABLE,
		GUIO0()
	},
	// Five Magical Amulets
	{
		"5ma",
		"",
		AD_ENTRY1s("data.dcp", "0134e92bcd5fd2837df3971087e96067", 163316498),
		Common::EN_ANY,
		Common::kPlatformWindows,
		ADGF_UNSTABLE,
		GUIO0()
	},
	// Actual Destination
	{
		"actualdest",
		"",
		AD_ENTRY1s("data.dcp", "6926f44b26f21ceb1d840eaab9aeb510", 9081740),
		Common::EN_ANY,
		Common::kPlatformWindows,
		ADGF_UNSTABLE,
		GUIO0()
	},
	// Chivalry is Not Dead
	{
		"chivalry",
		"",
		AD_ENTRY1s("data.dcp", "ebd0915d9a12df5224be22f53bb23eb6", 7278306),
		Common::EN_ANY,
		Common::kPlatformWindows,
		ADGF_UNSTABLE,
		GUIO0()
	},
	// Dead City (English)
	{
		"deadcity",
		"",
		{
			{"english.dcp", 0, "c591046d6de7e381d76f70e0787b2b1f", 415935},
			{"data.dcp", 0, "7ebfd50d1a22370ed7b079bcaa631d62", 9070205},
			AD_LISTEND
		},
		Common::EN_ANY,
		Common::kPlatformWindows,
		ADGF_UNSTABLE,
		GUIO0()
	},
	// Dead City (Italian)
	{
		"deadcity",
		"",
		{
			{"italian.dcp", 0, "92d8efb94436bec7bd1b7fe0b548192e", 454037},
			{"data.dcp", 0, "7ebfd50d1a22370ed7b079bcaa631d62", 9070205},
			AD_LISTEND
		},
		Common::IT_ITA,
		Common::kPlatformWindows,
		ADGF_UNSTABLE,
		GUIO0()
	},
	// Dead City (Russian)
	{
		"deadcity",
		"",
		{
			{"russian.dcp", 0, "a0ae71e9e1185596fffb07ad2c951eb9", 653317},
			{"data.dcp", 0, "7ebfd50d1a22370ed7b079bcaa631d62", 9070205},
			AD_LISTEND
		},
		Common::RU_RUS,
		Common::kPlatformWindows,
		ADGF_UNSTABLE,
		GUIO0()
	},
	// Dirty Split (English)
	{
		"dirtysplit",
		"",
		AD_ENTRY1s("data.dcp", "8f3dae199361ece0f59fb20cfff6eed3", 88577621),
		Common::EN_ANY,
		Common::kPlatformWindows,
		ADGF_UNSTABLE,
		GUIO0()
	},
	// Dirty Split (German)
	{
		"dirtysplit",
		"",
		AD_ENTRY1s("data.dcp", "139d8a25579e969f8b37d20e6e3de5f9", 92668291),
		Common::DE_DEU,
		Common::kPlatformWindows,
		ADGF_UNSTABLE,
		GUIO0()
	},
	// East Side Story (Demo)
	{
		"eastside",
		"Demo",
		AD_ENTRY1s("data.dcp", "b3f8b09bb4b05ee3e9d14697525257f9", 59296246),
		Common::EN_ANY,
		Common::kPlatformWindows,
		ADGF_UNSTABLE |
		ADGF_DEMO,
		GUIO0()
	},
	// Ghosts in the Sheet
	{
		"ghostsheet",
		"Demo",
		AD_ENTRY1s("data.dcp", "dc1f6595f412ac25a52eaf47dad4ab81", 169083),
		Common::EN_ANY,
		Common::kPlatformWindows,
		ADGF_UNSTABLE |
		ADGF_DEMO,
		GUIO0()
	},
	// Hamlet or the last game without MMORPS features, shaders and product placement
	{
		"hamlet",
		"",
		AD_ENTRY1s("data.dcp", "f624add957a77c9930529fb28cc2450f", 88183022),
		Common::EN_ANY,
		Common::kPlatformWindows,
		ADGF_UNSTABLE,
		GUIO0()
	},
	// J.U.L.I.A. (English)
	{
		"julia",
		"",
		AD_ENTRY1s("data.dcp", "c2264b4f8fcd132d2913ff5b6076a24f", 10109741),
		Common::EN_ANY,
		Common::kPlatformWindows,
		ADGF_UNSTABLE,
		GUIO0()
	},
	// J.U.L.I.A. (English) (Demo)
	{
		"julia",
		"Demo",
		AD_ENTRY1s("data.dcp", "f0bbc3394555a9811f6050dae428cab6", 7655237),
		Common::EN_ANY,
		Common::kPlatformWindows,
		ADGF_UNSTABLE |
		ADGF_DEMO,
		GUIO0()
	},
	// Mirage
	{
		"mirage",
		"",
		AD_ENTRY1s("data.dcp", "d230b0b99c0aa77b9ecd094d8ee5573b", 17844056),
		Common::EN_ANY,
		Common::kPlatformWindows,
		ADGF_UNSTABLE,
		GUIO0()
	},
	// Pigeons in the Park
	{
		"pigeons",
		"",
		AD_ENTRY1s("data.dcp", "9143a5b6ff8206aefe3c4c643add3ec7", 2611100),
		Common::EN_ANY,
		Common::kPlatformWindows,
		ADGF_UNSTABLE,
		GUIO0()
	},
	// Reversion
	{
		"reversion",
		"",
		AD_ENTRY1s("data.dcp", "cd616f98ebfd047e0c540b50b4b70761", 254384531),
		Common::EN_ANY,
		Common::kPlatformWindows,
		ADGF_UNSTABLE,
		GUIO0()
	},
	// Rosemary
	{
		"rosemary",
		"",
		AD_ENTRY1s("data.dcp", "4f2631138bd4d27587d9043f8aeff3df", 29483643),
		Common::EN_ANY,
		Common::kPlatformWindows,
		ADGF_UNSTABLE,
		GUIO0()
	},
	// The Box
	{
		"thebox",
		"",
		AD_ENTRY1s("data.dcp", "ec5f0c7e8174e307701447b53afe7e2f", 108372483),
		Common::EN_ANY,
		Common::kPlatformWindows,
		ADGF_UNSTABLE,
		GUIO0()
	},
	// the white chamber (multi-language)
	{
		"twc",
		"",
		AD_ENTRY1s("data.dcp", "0011d01142547c61e51ba24dc42b579e", 186451273),
		Common::UNK_LANG,
		Common::kPlatformWindows,
		ADGF_UNSTABLE,
		GUIO0()
	},
	AD_TABLE_END_MARKER
};

} // End of namespace Wintermute
