/**
 ** Supermodel
 ** A Sega Model 3 Arcade Emulator.
 ** Copyright 2011 Bart Trzynadlowski, Nik Henson
 **
 ** This file is part of Supermodel.
 **
 ** Supermodel is free software: you can redistribute it and/or modify it under
 ** the terms of the GNU General Public License as published by the Free 
 ** Software Foundation, either version 3 of the License, or (at your option)
 ** any later version.
 **
 ** Supermodel is distributed in the hope that it will be useful, but WITHOUT
 ** ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 ** FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for
 ** more details.
 **
 ** You should have received a copy of the GNU General Public License along
 ** with Supermodel.  If not, see <http://www.gnu.org/licenses/>.
 **/

/*
 * Games.cpp
 * 
 * Model 3 game and ROM information.
 *
 * ROMs are loaded in their native orientation. For example, PowerPC and 68K
 * ROMs will be laid out in memory as they would appear to those processors.
 * Any byte swapping that is done for performance-enhancing reasons by the
 * emulator is handled elsewhere.
 */

#include "Supermodel.h"


const struct GameInfo	g_Model3GameList[] =
{
	// Sega Bass Fishing
	{
		"bass",
		"Sega Bass Fishing",
		"Sega",
		1997,
		0x10,
		0x200000,	// 2 MB of fixed CROM
		true,		// 48 MB of banked CROM (Mirror)
		0x2000000,	// 32 MB of VROM
		0x800000,	// 8 MB of sample ROMs
		GAME_INPUT_COMMON|GAME_INPUT_JOYSTICK1,
		0,			// no MPEG board
		false,		// no drive board

		{
			// Fixed CROM
			{ "CROM",	false,	"epr-20646.20",	0xD740AE06,	0x80000,	2,	0x0600000,	8,	true },
			{ "CROM",	false,	"epr-20645.19",	0x8EEFA2B0,	0x80000,	2,	0x0600002,	8,	true },
			{ "CROM",	false,	"epr-20644.18",	0xC28DB2B6,	0x80000,	2,	0x0600004,	8,	true },
			{ "CROM",	false,	"epr-20643.17",	0xDAF02716,	0x80000,	2,	0x0600006,	8,	true },

			// Banked CROM0
			{ "CROMxx",	false,	"mpr-20259.4",	0x40052562,	0x400000,	2,	0x0000000,	8,	true },
			{ "CROMxx",	false,	"mpr-20258.3",	0x7B78B071,	0x400000,	2,	0x0000002,	8,	true },
			{ "CROMxx",	false,	"mpr-20257.2",	0x025BC06D,	0x400000,	2,	0x0000004,	8,	true },
			{ "CROMxx",	false,	"mpr-20256.1",	0x115302AC,	0x400000,	2,	0x0000006,	8,	true },

			// Banked CROM1
			{ "CROMxx",	false,	"mpr-20263.8",	0x1CF4CBA9,	0x400000,	2,	0x1000000,	8,	true },
			{ "CROMxx",	false,	"mpr-20262.7",	0x52B0674D,	0x400000,	2,	0x1000002,	8,	true },
			{ "CROMxx",	false,	"mpr-20261.6",	0xB1E9D44A,	0x400000,	2,	0x1000004,	8,	true },
			{ "CROMxx",	false,	"mpr-20260.5",	0xC56B4C10,	0x400000,	2,	0x1000006,	8,	true },

			// Banked CROM2
			{ "CROMxx",	false,	"mpr-20267.12",	0x48989191,	0x400000,	2,	0x2000000,	8,	true },
			{ "CROMxx",	false,	"mpr-20266.11",	0xABD2DB85,	0x400000,	2,	0x2000002,	8,	true },
			{ "CROMxx",	false,	"mpr-20265.10",	0x28F76E3E,	0x400000,	2,	0x2000004,	8,	true },
			{ "CROMxx",	false,	"mpr-20264.9",	0x8D995196,	0x400000,	2,	0x2000006,	8,	true },

			// Video ROM
			{ "VROM",	false,	"mpr-20270.26",	0xDF68A7A7,	0x200000,	2,	0,		32,	false },
			{ "VROM",	false,	"mpr-20271.27",	0x4B01C3A4,	0x200000,	2,	2,		32,	false },
			{ "VROM",	false,	"mpr-20272.28",	0xA658DA23,	0x200000,	2,	4,		32,	false },
			{ "VROM",	false,	"mpr-20273.29",	0x577E9FFA,	0x200000,	2,	6,		32,	false },
			{ "VROM",	false,	"mpr-20274.30",	0x7C7056AE,	0x200000,	2,	8,		32,	false },
			{ "VROM",	false,	"mpr-20275.31",	0xE739F77A,	0x200000,	2,	10,		32,	false },
			{ "VROM",	false,	"mpr-20276.32",	0xCBF966C0,	0x200000,	2,	12,		32,	false },
			{ "VROM",	false,	"mpr-20277.33",	0x9C75200B,	0x200000,	2,	14,		32,	false },
			{ "VROM",	false,	"mpr-20278.34",	0xDB3991BA,	0x200000,	2,	16,		32,	false },
			{ "VROM",	false,	"mpr-20279.35",	0x995A11B8,	0x200000,	2,	18,		32,	false },
			{ "VROM",	false,	"mpr-20280.36",	0xC2C8F9F5,	0x200000,	2,	20,		32,	false },
			{ "VROM",	false,	"mpr-20281.37",	0xDA84B967,	0x200000,	2,	22,		32,	false },
			{ "VROM",	false,	"mpr-20282.38",	0x1869FF49,	0x200000,	2,	24,		32,	false },
			{ "VROM",	false,	"mpr-20283.39",	0x7D8FB469,	0x200000,	2,	26,		32,	false },
			{ "VROM",	false,	"mpr-20284.40",	0x5C7F3A6F,	0x200000,	2,	28,		32,	false },
			{ "VROM",	false,	"mpr-20285.41",	0x4AADC573,	0x200000,	2,	30,		32,	false },

			// Sound ROMs
			{ "SndProg",false,	"epr-20313.21",	0x863A7857,	0x80000,	2,	0,		2,	true },
			{ "Samples",false,	"mpr-20268.22",	0x3631E93E,	0x400000,	2,	0x000000,	2,	false },
			{ "Samples",false,	"mpr-20269.24",	0x105A3181,	0x400000,	2,	0x400000,	2,	false },

			{ NULL,	false, NULL, 0, 0, 0, 0, 0, false }
		}
	},

	// Daytona USA 2 (Revision A)
	{
		"daytona2",
		"Daytona USA 2 Battle on the Edge",
		"Sega",
		1998,
		0x21,
		0x800000,	// 8 MB of fixed CROM
		false,	// 96 MB of banked CROM (do not mirror)
		0x4000000,	// 64 MB of VROM
		0x1000000,	// 16 MB of sample ROMs
		GAME_INPUT_COMMON|GAME_INPUT_VEHICLE|GAME_INPUT_VR|GAME_INPUT_SHIFT4,
		2,		// DSB2 MPEG board
		true,	// drive board

		{
			// Fixed CROM
			{ "CROM",	false,	"epr-20864a.20",	0x5250F3A8,	0x200000,	2,	0x0000000,	8,	true },
			{ "CROM",	false,	"epr-20863a.19",	0x1DEB4686,	0x200000,	2,	0x0000002,	8,	true },
			{ "CROM",	false,	"epr-20862a.18",	0xE1B2CA61,	0x200000,	2,	0x0000004,	8,	true },
			{ "CROM",	false,	"epr-20861a.17",	0x89BA8E78,	0x200000,	2,	0x0000006,	8,	true },

			// Banked CROM0
			{ "CROMxx",	false,	"mpr-20848.ic4",	0x5B6C8B7D,	0x800000,	2,	0x0000000,	8,	true },
			{ "CROMxx",	false,	"mpr-20847.ic3",	0xEDA966EE,	0x800000,	2,	0x0000002,	8,	true },
			{ "CROMxx",	false,	"mpr-20846.ic2",	0xF44C5C7A,	0x800000,	2,	0x0000004,	8,	true },
			{ "CROMxx",	false,	"mpr-20845.ic1",	0x6037712C,	0x800000,	2,	0x0000006,	8,	true },

			// Banked CROM1
			{ "CROMxx",	false,	"mpr-20852.ic8",	0xD606AD38,	0x800000,	2,	0x2000000,	8,	true },
			{ "CROMxx",	false,	"mpr-20851.ic7",	0x6E7A64B7,	0x800000,	2,	0x2000002,	8,	true },
			{ "CROMxx",	false,	"mpr-20850.ic6",	0xCB73758A,	0x800000,	2,	0x2000004,	8,	true },
			{ "CROMxx",	false,	"mpr-20849.ic5",	0x50DEE4AF,	0x800000,	2,	0x2000006,	8,	true },

			// Banked CROM2
			{ "CROMxx",	false,	"mpr-20856.12",	0x0367A242,	0x400000,	2,	0x4000000,	8,	true },
			{ "CROMxx",	false,	"mpr-20855.11",	0xF1FF0794,	0x400000,	2,	0x4000002,	8,	true },
			{ "CROMxx",	false,	"mpr-20854.10",	0x68D94CDF,	0x400000,	2,	0x4000004,	8,	true },
			{ "CROMxx",	false,	"mpr-20853.9",	0x3245EE68,	0x400000,	2,	0x4000006,	8,	true },

			// Banked CROM3 (note: appears at offset 0x6000000 rather than 0x5000000 as expected)
			{ "CROMxx",	false,	"mpr-20860.16",	0xE5CE2939,	0x400000,	2,	0x6000000,	8,	true },
			{ "CROMxx",	false,	"mpr-20859.15",	0xE14F5C46,	0x400000,	2,	0x6000002,	8,	true },
			{ "CROMxx",	false,	"mpr-20858.14",	0x407FBAD5,	0x400000,	2,	0x6000004,	8,	true },
			{ "CROMxx",	false,	"mpr-20857.13",	0x1EAB9C62,	0x400000,	2,	0x6000006,	8,	true },

			// Video ROM
			{ "VROM",	false,	"mpr-20870.26",	0x7C9E573D,	0x400000,	2,	0,		32,	false },
			{ "VROM",	false,	"mpr-20871.27",	0x47A1B789,	0x400000,	2,	2,		32,	false },
			{ "VROM",	false,	"mpr-20872.28",	0x2F55B423,	0x400000,	2,	4,		32,	false },
			{ "VROM",	false,	"mpr-20873.29",	0xC9000E48,	0x400000,	2,	6,		32,	false },
			{ "VROM",	false,	"mpr-20874.30",	0x26A9CCA2,	0x400000,	2,	8,		32,	false },
			{ "VROM",	false,	"mpr-20875.31",	0xBFEFD21E,	0x400000,	2,	10,		32,	false },
			{ "VROM",	false,	"mpr-20876.32",	0xFA701B87,	0x400000,	2,	12,		32,	false },
			{ "VROM",	false,	"mpr-20877.33",	0x2CD072F1,	0x400000,	2,	14,		32,	false },
			{ "VROM",	false,	"mpr-20878.34",	0xE6D5BC01,	0x400000,	2,	16,		32,	false },
			{ "VROM",	false,	"mpr-20879.35",	0xF1D727EC,	0x400000,	2,	18,		32,	false },
			{ "VROM",	false,	"mpr-20880.36",	0x8B370602,	0x400000,	2,	20,		32,	false },
			{ "VROM",	false,	"mpr-20881.37",	0x397322E7,	0x400000,	2,	22,		32,	false },
			{ "VROM",	false,	"mpr-20882.38",	0x9185BE51,	0x400000,	2,	24,		32,	false },
			{ "VROM",	false,	"mpr-20883.39",	0xD1E39E83,	0x400000,	2,	26,		32,	false },
			{ "VROM",	false,	"mpr-20884.40",	0x63C4639A,	0x400000,	2,	28,		32,	false },
			{ "VROM",	false,	"mpr-20885.41",	0x61C292CA,	0x400000,	2,	30,		32,	false },

			// Sound ROMs
			{ "SndProg",false,	"epr-20865.21",		0xB70C2699,	0x20000,	2,	0,		2,	true },
			{ "Samples",false,	"mpr-20866.22",		0x91F40C1C,	0x400000,	2,	0x000000,	2,	false },
			{ "Samples",false,	"mpr-20868.24",		0xFA0C7EC0,	0x400000,	2,	0x400000,	2,	false },
			{ "Samples",false,	"mpr-20867.23",		0xA579C884,	0x400000,	2,	0x800000,	2,	false },
			{ "Samples",false,	"mpr-20869.25",		0x1F338832,	0x400000,	2,	0xC00000,	2,	false },
			{ "DSBProg",false,	"epr-20886.ic2",	0x65B05F98,	0x20000,	2,	0,		2,	true },
			{ "DSBMPEG",false,	"mpr-20887.ic18",	0xA0757684,	0x400000,	2,	0x000000,	2,	false },
			{ "DSBMPEG",false,	"mpr-20888.ic20",	0xB495FE65,	0x400000,	2,	0x400000,	2,	false },
			{ "DSBMPEG",false,	"mpr-20889.ic22",	0x18EEC79E,	0x400000,	2,	0x800000,	2,	false },
			{ "DSBMPEG",false,	"mpr-20890.ic24",	0xAAC96FA2,	0x400000,	2,	0xC00000,	2,	false },
			
			// Drive Board ROM
			{ "DriveBd",	true,	"epr-20985.bin", 0xB139481D, 0x10000, 2, 0, 2, false },

			{ NULL, false, NULL, 0, 0, 0, 0, 0, false }
		}
	},

	// Daytona USA 2 Power Edition
	{
		"dayto2pe",
		"Daytona USA 2 Power Edition",
		"Sega",
		1998,
		0x21,
		0x800000,	// 8 MB of fixed CROM
		false,	// 64 MB of banked CROM (do not mirror)
		0x4000000,	// 64 MB of VROM
		0x1000000,	// 16 MB of sample ROMs
		GAME_INPUT_COMMON|GAME_INPUT_VEHICLE|GAME_INPUT_VR|GAME_INPUT_SHIFT4,
		2,		// DSB2 MPEG board
		true,	// drive board

		{
			// Fixed CROM
			{ "CROM",	false,	"epr-21181.20",	0xBF0007ED,	0x200000,	2,	0x0000000,	8,	true },
			{ "CROM",	false,	"epr-21180.19",	0x6E7B98ED,	0x200000,	2,	0x0000002,	8,	true },
			{ "CROM",	false,	"epr-21179.18",	0xD5FFB4D6,	0x200000,	2,	0x0000004,	8,	true },
			{ "CROM",	false,	"epr-21178.17",	0x230BF8AC,	0x200000,	2,	0x0000006,	8,	true },

			// Banked CROM0
			{ "CROMxx",	false,	"mpr-21185.4",	0xB6D5D2A1,	0x400000,	2,	0x0000000,	8,	true },
			{ "CROMxx",	false,	"mpr-21184.3",	0x25616403,	0x400000,	2,	0x0000002,	8,	true },
			{ "CROMxx",	false,	"mpr-21183.2",	0xB4B44805,	0x400000,	2,	0x0000004,	8,	true },
			{ "CROMxx",	false,	"mpr-21182.1",	0xBA8E667F,	0x400000,	2,	0x0000006,	8,	true },

			// Banked CROM1
			{ "CROMxx",	false,	"mpr-21189.8",	0xCB439C45,	0x400000,	2,	0x2000000,	8,	true },
			{ "CROMxx",	false,	"mpr-21188.7",	0x753FC2A5,	0x400000,	2,	0x2000002,	8,	true },
			{ "CROMxx",	false,	"mpr-21187.6",	0x3BD14EE6,	0x400000,	2,	0x2000004,	8,	true },
			{ "CROMxx",	false,	"mpr-21186.5",	0xA6128662,	0x400000,	2,	0x2000006,	8,	true },

			// Banked CROM2
			{ "CROMxx",	false,	"mpr-21193.12",	0x4638FEF4,	0x400000,	2,	0x4000000,	8,	true },
			{ "CROMxx",	false,	"mpr-21192.11",	0x60CBB1FA,	0x400000,	2,	0x4000002,	8,	true },
			{ "CROMxx",	false,	"mpr-21191.10",	0xA2BDCFE0,	0x400000,	2,	0x4000004,	8,	true },
			{ "CROMxx",	false,	"mpr-21190.9",	0x984D56EB,	0x400000,	2,	0x4000006,	8,	true },

			// Banked CROM3
			{ "CROMxx",	false,	"mpr-21197.16",	0x04015247,	0x400000,	2,	0x6000000,	8,	true },
			{ "CROMxx",	false,	"mpr-21196.15",	0x0AB46DB5,	0x400000,	2,	0x6000002,	8,	true },
			{ "CROMxx",	false,	"mpr-21195.14",	0x7F39761C,	0x400000,	2,	0x6000004,	8,	true },
			{ "CROMxx",	false,	"mpr-21194.13",	0x12C7A414,	0x400000,	2,	0x6000006,	8,	true },

			// Video ROM
			{ "VROM",	false,	"mpr-21198.26",	0x42EC9ED4,	0x400000,	2,	0,		32,	false },
			{ "VROM",	false,	"mpr-21199.27",	0xFA28088C,	0x400000,	2,	2,		32,	false },
			{ "VROM",	false,	"mpr-21200.28",	0xFBB5AA1D,	0x400000,	2,	4,		32,	false },
			{ "VROM",	false,	"mpr-21201.29",	0xE6B13469,	0x400000,	2,	6,		32,	false },
			{ "VROM",	false,	"mpr-21202.30",	0xE6B4C2BE,	0x400000,	2,	8,		32,	false },
			{ "VROM",	false,	"mpr-21203.31",	0x32D08D33,	0x400000,	2,	10,		32,	false },
			{ "VROM",	false,	"mpr-21204.32",	0xEF18FE0A,	0x400000,	2,	12,		32,	false },
			{ "VROM",	false,	"mpr-21205.33",	0x4687BEA6,	0x400000,	2,	14,		32,	false },
			{ "VROM",	false,	"mpr-21206.34",	0xEC2D6884,	0x400000,	2,	16,		32,	false },
			{ "VROM",	false,	"mpr-21207.35",	0xEEAA510B,	0x400000,	2,	18,		32,	false },
			{ "VROM",	false,	"mpr-21208.36",	0xB222FEF0,	0x400000,	2,	20,		32,	false },
			{ "VROM",	false,	"mpr-21209.37",	0x170A28CE,	0x400000,	2,	22,		32,	false },
			{ "VROM",	false,	"mpr-21210.38",	0x460CEFE0,	0x400000,	2,	24,		32,	false },
			{ "VROM",	false,	"mpr-21211.39",	0xC84759CE,	0x400000,	2,	26,		32,	false },
			{ "VROM",	false,	"mpr-21212.40",	0x6F8A75E0,	0x400000,	2,	28,		32,	false },
			{ "VROM",	false,	"mpr-21213.41",	0xDE75BEC6,	0x400000,	2,	30,		32,	false },

			// Sound ROMs
			{ "SndProg",	false,"epr-21325.21",	0x004AD6AD,	0x20000,	2,	0,		2,	true },
			{ "Samples",	false,"mpr-21285.22",	0x7CDCA6AC,	0x400000,	2,	0x000000,	2,	false },
			{ "Samples",	false,"mpr-21287.24",	0x06B66F17,	0x400000,	2,	0x400000,	2,	false },
			{ "Samples",	false,"mpr-21286.23",	0x749DFEF0,	0x400000,	2,	0x800000,	2,	false },
			{ "Samples",	false,"mpr-21288.25",	0x14BEE38E,	0x400000,	2,	0xC00000,	2,	false },
			{ "DSBProg",	false,"epr-20886.ic2",	0x65B05F98,	0x20000,	2,	0,		2,	true },
			{ "DSBMPEG",	false,"mpr-20887.ic18",	0xA0757684,	0x400000,	2,	0x000000,	2,	false },
			{ "DSBMPEG",	false,"mpr-20888.ic20",	0xB495FE65,	0x400000,	2,	0x400000,	2,	false },
			{ "DSBMPEG",	false,"mpr-20889.ic22",	0x18EEC79E,	0x400000,	2,	0x800000,	2,	false },
			{ "DSBMPEG",	false,"mpr-20890.ic24",	0xAAC96FA2,	0x400000,	2,	0xC00000,	2,	false },
			
			// Drive Board ROM
			{ "DriveBd",	true,	"epr-20985.bin", 0xB139481D, 0x10000, 2, 0, 2, false },

			{ NULL,	false,	NULL, 0, 0, 0, 0, 0, false }
		}
	},

	// Dirt Devils (Revision A)
	{
		"dirtdvls",
		"Dirt Devils",
		"Sega",
		1998,
		0x21,
		0x800000,	// 8 MB of fixed CROM
		true,		// 32 MB of banked CROM (Mirror)
		0x4000000,	// 64 MB of VROM
		0xC00000,	// 12 MB of sample ROMs
		GAME_INPUT_COMMON|GAME_INPUT_VEHICLE|GAME_INPUT_SHIFT4|GAME_INPUT_RALLY,
		0,		// no MPEG board
		false,		// no drive board

		{
			// Fixed CROM
			{ "CROM",	false,	"epr-21065a.20",	0x3223DB1A,	0x200000,	2,	0x0000000,	8,	true },
			{ "CROM",	false,	"epr-21064a.19",	0x2A01F9AD,	0x200000,	2,	0x0000002,	8,	true },
			{ "CROM",	false,	"epr-21063a.18",	0x6AB7EB32,	0x200000,	2,	0x0000004,	8,	true },
			{ "CROM",	false,	"epr-21062a.17",	0x64B55254,	0x200000,	2,	0x0000006,	8,	true },

			// Banked CROM0
			{ "CROMxx",	false,	"mpr-21026.4",	0xF4937E3F,	0x400000,	2,	0x0000000,	8,	true },
			{ "CROMxx",	false,	"mpr-21025.3",	0x6591C66E,	0x400000,	2,	0x0000002,	8,	true },
			{ "CROMxx",	false,	"mpr-21024.2",	0xEDE859B0,	0x400000,	2,	0x0000004,	8,	true },
			{ "CROMxx",	false,	"mpr-21023.1",	0x932A3724,	0x400000,	2,	0x0000006,	8,	true },

			// Banked CROM1
			{ "CROMxx",	false,	"mpr-21030.8",	0xF8E51BEC,	0x400000,	2,	0x1000000,	8,	true },
			{ "CROMxx",	false,	"mpr-21029.7",	0x89867D8A,	0x400000,	2,	0x1000002,	8,	true },
			{ "CROMxx",	false,	"mpr-21028.6",	0xDB11F50A,	0x400000,	2,	0x1000004,	8,	true },
			{ "CROMxx",	false,	"mpr-21027.5",	0x74E1496A,	0x400000,	2,	0x1000006,	8,	true },

			// Video ROM
			{ "VROM",	false,	"mpr-21034.26",	0xACBA5CA6,	0x400000,	2,	0,		32,	false },
			{ "VROM",	false,	"mpr-21035.27",	0x618B7D6A,	0x400000,	2,	2,		32,	false },
			{ "VROM",	false,	"mpr-21036.28",	0x0E665BB2,	0x400000,	2,	4,		32,	false },
			{ "VROM",	false,	"mpr-21037.29",	0x90B98493,	0x400000,	2,	6,		32,	false },
			{ "VROM",	false,	"mpr-21038.30",	0x9B59D2C2,	0x400000,	2,	8,		32,	false },
			{ "VROM",	false,	"mpr-21039.31",	0x61407B07,	0x400000,	2,	10,		32,	false },
			{ "VROM",	false,	"mpr-21040.32",	0xB550C229,	0x400000,	2,	12,		32,	false },
			{ "VROM",	false,	"mpr-21041.33",	0x8F1AC988,	0x400000,	2,	14,		32,	false },
			{ "VROM",	false,	"mpr-21042.34",	0x1DAB621D,	0x400000,	2,	16,		32,	false },
			{ "VROM",	false,	"mpr-21043.35",	0x707015C8,	0x400000,	2,	18,		32,	false },
			{ "VROM",	false,	"mpr-21044.36",	0x776F9580,	0x400000,	2,	20,		32,	false },
			{ "VROM",	false,	"mpr-21045.37",	0xA28AD02F,	0x400000,	2,	22,		32,	false },
			{ "VROM",	false,	"mpr-21046.38",	0x05C995AE,	0x400000,	2,	24,		32,	false },
			{ "VROM",	false,	"mpr-21047.39",	0x06B7826F,	0x400000,	2,	26,		32,	false },
			{ "VROM",	false,	"mpr-21048.40",	0x96849974,	0x400000,	2,	28,		32,	false },
			{ "VROM",	false,	"mpr-21049.41",	0x91E8161A,	0x400000,	2,	30,		32,	false },

			// Sound ROMs
			{ "SndProg",	false,"epr-21066.21",	0xF7ED2582,	0x80000,	2,	0,		2,	true },
			{ "Samples",	false,"mpr-21031.22",	0x32F6B23A,	0x400000,	2,	0x000000,	2,	false },
			{ "Samples",	false,"mpr-21033.24",	0x253D3C70,	0x400000,	2,	0x400000,	2,	false },
			{ "Samples",	false,"mpr-21032.23",	0x3D3FF407,	0x400000,	2,	0x800000,	2,	false },

			{ NULL,	false,	NULL, 0, 0, 0, 0, 0, false }
		}
	},

	// Dirt Devils (Alt)(Revision A)
	{
		"dirtdvlsa",
		"Dirt Devils (Alt.)",
		"Sega",
		1998,
		0x21,
		0x800000,	// 8 MB of fixed CROM
		true,		// 32 MB of banked CROM (Mirror)
		0x4000000,	// 64 MB of VROM
		0xC00000,	// 12 MB of sample ROMs
		GAME_INPUT_COMMON|GAME_INPUT_VEHICLE|GAME_INPUT_SHIFT4|GAME_INPUT_RALLY,
		0,		// no MPEG board
		false,	// no drive board

		{
			// Fixed CROM
			{ "CROM",	false,	"epr-21061a",	0x755CA612,	0x200000,	2,	0x0000000,	8,	true },
			{ "CROM",	false,	"epr-21060a",	0x5EBE2816,	0x200000,	2,	0x0000002,	8,	true },
			{ "CROM",	false,	"epr-21059a",	0xF31A2AA4,	0x200000,	2,	0x0000004,	8,	true },
			{ "CROM",	false,	"epr-21058a",	0x4D7FDC8D,	0x200000,	2,	0x0000006,	8,	true },

			// Banked CROM0
			{ "CROMxx",	false,	"mpr-21026.4",	0xF4937E3F,	0x400000,	2,	0x0000000,	8,	true },
			{ "CROMxx",	false,	"mpr-21025.3",	0x6591C66E,	0x400000,	2,	0x0000002,	8,	true },
			{ "CROMxx",	false,	"mpr-21024.2",	0xEDE859B0,	0x400000,	2,	0x0000004,	8,	true },
			{ "CROMxx",	false,	"mpr-21023.1",	0x932A3724,	0x400000,	2,	0x0000006,	8,	true },

			// Banked CROM1
			{ "CROMxx",	false,	"mpr-21030.8",	0xF8E51BEC,	0x400000,	2,	0x1000000,	8,	true },
			{ "CROMxx",	false,	"mpr-21029.7",	0x89867D8A,	0x400000,	2,	0x1000002,	8,	true },
			{ "CROMxx",	false,	"mpr-21028.6",	0xDB11F50A,	0x400000,	2,	0x1000004,	8,	true },
			{ "CROMxx",	false,	"mpr-21027.5",	0x74E1496A,	0x400000,	2,	0x1000006,	8,	true },

			// Video ROM
			{ "VROM",	false,	"mpr-21034.26",	0xACBA5CA6,	0x400000,	2,	0,		32,	false },
			{ "VROM",	false,	"mpr-21035.27",	0x618B7D6A,	0x400000,	2,	2,		32,	false },
			{ "VROM",	false,	"mpr-21036.28",	0x0E665BB2,	0x400000,	2,	4,		32,	false },
			{ "VROM",	false,	"mpr-21037.29",	0x90B98493,	0x400000,	2,	6,		32,	false },
			{ "VROM",	false,	"mpr-21038.30",	0x9B59D2C2,	0x400000,	2,	8,		32,	false },
			{ "VROM",	false,	"mpr-21039.31",	0x61407B07,	0x400000,	2,	10,		32,	false },
			{ "VROM",	false,	"mpr-21040.32",	0xB550C229,	0x400000,	2,	12,		32,	false },
			{ "VROM",	false,	"mpr-21041.33",	0x8F1AC988,	0x400000,	2,	14,		32,	false },
			{ "VROM",	false,	"mpr-21042.34",	0x1DAB621D,	0x400000,	2,	16,		32,	false },
			{ "VROM",	false,	"mpr-21043.35",	0x707015C8,	0x400000,	2,	18,		32,	false },
			{ "VROM",	false,	"mpr-21044.36",	0x776F9580,	0x400000,	2,	20,		32,	false },
			{ "VROM",	false,	"mpr-21045.37",	0xA28AD02F,	0x400000,	2,	22,		32,	false },
			{ "VROM",	false,	"mpr-21046.38",	0x05C995AE,	0x400000,	2,	24,		32,	false },
			{ "VROM",	false,	"mpr-21047.39",	0x06B7826F,	0x400000,	2,	26,		32,	false },
			{ "VROM",	false,	"mpr-21048.40",	0x96849974,	0x400000,	2,	28,		32,	false },
			{ "VROM",	false,	"mpr-21049.41",	0x91E8161A,	0x400000,	2,	30,		32,	false },

			// Sound ROMs
			{ "SndProg",	false,"epr-21066.21",	0xF7ED2582,	0x80000,	2,	0,		2,	true },
			{ "Samples",	false,"mpr-21031.22",	0x32F6B23A,	0x400000,	2,	0x000000,	2,	false },
			{ "Samples",	false,"mpr-21033.24",	0x253D3C70,	0x400000,	2,	0x400000,	2,	false },
			{ "Samples",	false,"mpr-21032.23",	0x3D3FF407,	0x400000,	2,	0x800000,	2,	false },

			{ NULL, false, NULL, 0, 0, 0, 0, 0, false }
		}
	},

	// Emergency Call Ambulance
	{
		"eca",
		"Emergency Call Ambulance",
		"Sega",
		1999,
		0x21,
		0x800000,	// 8 MB of fixed CROM
		true,		// 48 MB of banked CROM (Mirror)
		0x4000000,	// 64 MB of VROM
		0x1000000,	// 16 MB of sample ROMs
		GAME_INPUT_COMMON|GAME_INPUT_VEHICLE|GAME_INPUT_SHIFT4|GAME_INPUT_RALLY,
		0,			// no MPEG board
		false,		// no drive board

		{
			// Fixed CROM
			{ "CROM",	false,	"epr22898.20",	0xEFB96701,	0x200000,	2,	0x0000000,	8,	true },
			{ "CROM",	false,	"epr22897.19",	0x9755DD8C,	0x200000,	2,	0x0000002,	8,	true },
			{ "CROM",	false,	"epr22896.18",	0x0FF828A8,	0x200000,	2,	0x0000004,	8,	true },
			{ "CROM",	false,	"epr22895.17",	0x07DF16A0,	0x200000,	2,	0x0000006,	8,	true },

			// Banked CROM0
			{ "CROMxx",	false,	"mpr22873.4",	0xDD406330,	0x400000,	2,	0x0000000,	8,	true },
			{ "CROMxx",	false,	"mpr22872.3",	0x4FDE63A1,	0x400000,	2,	0x0000002,	8,	true },
			{ "CROMxx",	false,	"mpr22871.2",	0xCF5BB5B5,	0x400000,	2,	0x0000004,	8,	true },
			{ "CROMxx",	false,	"mpr22870.1",	0x52054043,	0x400000,	2,	0x0000006,	8,	true },

			// Banked CROM1
			{ "CROMxx",	false,	"mpr22877.8",	0xE53B8764,	0x400000,	2,	0x1000000,	8,	true },
			{ "CROMxx",	false,	"mpr22876.7",	0xA7561249,	0x400000,	2,	0x1000002,	8,	true },
			{ "CROMxx",	false,	"mpr22875.6",	0x1BB5C018,	0x400000,	2,	0x1000004,	8,	true },
			{ "CROMxx",	false,	"mpr22874.5",	0x5E990497,	0x400000,	2,	0x1000006,	8,	true },

			// Banked CROM3
			{ "CROMxx",	false,	"mpr22885.16",	0x3525B46D,	0x400000,	2,	0x3000000,	8,	true },
			{ "CROMxx",	false,	"mpr22884.15",	0x254C3B63,	0x400000,	2,	0x3000002,	8,	true },
			{ "CROMxx",	false,	"mpr22883.14",	0x86D90148,	0x400000,	2,	0x3000004,	8,	true },
			{ "CROMxx",	false,	"mpr22882.13",	0xB161416F,	0x400000,	2,	0x3000006,	8,	true },

			// Video ROM
			{ "VROM",	false,	"mpr22854.26",	0x97A23D16,	0x400000,	2,	0,		32,	false },
			{ "VROM",	false,	"mpr22855.27",	0x7249CDC9,	0x400000,	2,	2,		32,	false },
			{ "VROM",	false,	"mpr22856.28",	0x9C0D1D1B,	0x400000,	2,	4,		32,	false },
			{ "VROM",	false,	"mpr22857.29",	0x44E6CE2B,	0x400000,	2,	6,		32,	false },
			{ "VROM",	false,	"mpr22858.30",	0x0AF40AAE,	0x400000,	2,	8,		32,	false },
			{ "VROM",	false,	"mpr22859.31",	0xC64F0158,	0x400000,	2,	10,		32,	false },
			{ "VROM",	false,	"mpr22860.32",	0x053AF14B,	0x400000,	2,	12,		32,	false },
			{ "VROM",	false,	"mpr22861.33",	0xD26343DA,	0x400000,	2,	14,		32,	false },
			{ "VROM",	false,	"mpr22862.34",	0x38347C14,	0x400000,	2,	16,		32,	false },
			{ "VROM",	false,	"mpr22863.35",	0x28B558E6,	0x400000,	2,	18,		32,	false },
			{ "VROM",	false,	"mpr22864.36",	0x31ED02F6,	0x400000,	2,	20,		32,	false },
			{ "VROM",	false,	"mpr22865.37",	0x3E3A211A,	0x400000,	2,	22,		32,	false },
			{ "VROM",	false,	"mpr22866.38",	0xA863A3C8,	0x400000,	2,	24,		32,	false },
			{ "VROM",	false,	"mpr22867.39",	0x1CE6C7B2,	0x400000,	2,	26,		32,	false },
			{ "VROM",	false,	"mpr22868.40",	0x2DB40CF8,	0x400000,	2,	28,		32,	false },
			{ "VROM",	false,	"mpr22869.41",	0xC6D62634,	0x400000,	2,	30,		32,	false },

			// Sound ROMs
			{ "SndProg",	false,"epr22886.21",	0x374EC1C6,	0x80000,	2,	0,		2,	true },
			{ "Samples",	false,"mpr22887.22",	0x7D04A867,	0x400000,	2,	0x000000,	2,	false },
			{ "Samples",	false,"mpr22889.24",	0x4F9BA45D,	0x400000,	2,	0x400000,	2,	false },
			{ "Samples",	false,"mpr22888.23",	0x018FCF22,	0x400000,	2,	0x800000,	2,	false },
			{ "Samples",	false,"mpr22890.25",	0xB638BD7C,	0x400000,	2,	0xC00000,	2,	false },

			{ NULL,	false,	NULL, 0, 0, 0, 0, 0, false }
		}
	},

	// Emergency Call Ambulance (Export)
	{
		"ecax",
		"Emergency Call Ambulance (Export)",
		"Sega",
		1999,
		0x21,
		0x800000,	// 8 MB of fixed CROM
		true,		// 48 MB of banked CROM (Mirror)
		0x4000000,	// 64 MB of VROM
		0x1000000,	// 16 MB of sample ROMs
		GAME_INPUT_COMMON|GAME_INPUT_VEHICLE|GAME_INPUT_SHIFT4|GAME_INPUT_RALLY,
		0,			// no MPEG board
		false,		// no drive board

		{
			// Fixed CROM
			{ "CROM",	false,	"epr22906.20",	0x7F6426FC,	0x200000,	2,	0x0000000,	8,	true },
			{ "CROM",	false,	"epr22905.19",	0x9755DD8C,	0x200000,	2,	0x0000002,	8,	true },
			{ "CROM",	false,	"epr22904.18",	0x0FF828A8,	0x200000,	2,	0x0000004,	8,	true },
			{ "CROM",	false,	"epr22903.17",	0x53882217,	0x200000,	2,	0x0000006,	8,	true },

			// Banked CROM0
			{ "CROMxx",	false,	"mpr22873.4",	0xDD406330,	0x400000,	2,	0x0000000,	8,	true },
			{ "CROMxx",	false,	"mpr22872.3",	0x4FDE63A1,	0x400000,	2,	0x0000002,	8,	true },
			{ "CROMxx",	false,	"mpr22871.2",	0xCF5BB5B5,	0x400000,	2,	0x0000004,	8,	true },
			{ "CROMxx",	false,	"mpr22870.1",	0x52054043,	0x400000,	2,	0x0000006,	8,	true },

			// Banked CROM1
			{ "CROMxx",	false,	"mpr22877.8",	0xE53B8764,	0x400000,	2,	0x1000000,	8,	true },
			{ "CROMxx",	false,	"mpr22876.7",	0xA7561249,	0x400000,	2,	0x1000002,	8,	true },
			{ "CROMxx",	false,	"mpr22875.6",	0x1BB5C018,	0x400000,	2,	0x1000004,	8,	true },
			{ "CROMxx",	false,	"mpr22874.5",	0x5E990497,	0x400000,	2,	0x1000006,	8,	true },

			// Banked CROM3
			{ "CROMxx",	false,	"mpr22885.16",	0x3525B46D,	0x400000,	2,	0x3000000,	8,	true },
			{ "CROMxx",	false,	"mpr22884.15",	0x254C3B63,	0x400000,	2,	0x3000002,	8,	true },
			{ "CROMxx",	false,	"mpr22883.14",	0x86D90148,	0x400000,	2,	0x3000004,	8,	true },
			{ "CROMxx",	false,	"mpr22882.13",	0xB161416F,	0x400000,	2,	0x3000006,	8,	true },

			// Video ROM
			{ "VROM",	false,	"mpr22854.26",	0x97A23D16,	0x400000,	2,	0,		32,	false },
			{ "VROM",	false,	"mpr22855.27",	0x7249CDC9,	0x400000,	2,	2,		32,	false },
			{ "VROM",	false,	"mpr22856.28",	0x9C0D1D1B,	0x400000,	2,	4,		32,	false },
			{ "VROM",	false,	"mpr22857.29",	0x44E6CE2B,	0x400000,	2,	6,		32,	false },
			{ "VROM",	false,	"mpr22858.30",	0x0AF40AAE,	0x400000,	2,	8,		32,	false },
			{ "VROM",	false,	"mpr22859.31",	0xC64F0158,	0x400000,	2,	10,		32,	false },
			{ "VROM",	false,	"mpr22860.32",	0x053AF14B,	0x400000,	2,	12,		32,	false },
			{ "VROM",	false,	"mpr22861.33",	0xD26343DA,	0x400000,	2,	14,		32,	false },
			{ "VROM",	false,	"mpr22862.34",	0x38347C14,	0x400000,	2,	16,		32,	false },
			{ "VROM",	false,	"mpr22863.35",	0x28B558E6,	0x400000,	2,	18,		32,	false },
			{ "VROM",	false,	"mpr22864.36",	0x31ED02F6,	0x400000,	2,	20,		32,	false },
			{ "VROM",	false,	"mpr22865.37",	0x3E3A211A,	0x400000,	2,	22,		32,	false },
			{ "VROM",	false,	"mpr22866.38",	0xA863A3C8,	0x400000,	2,	24,		32,	false },
			{ "VROM",	false,	"mpr22867.39",	0x1CE6C7B2,	0x400000,	2,	26,		32,	false },
			{ "VROM",	false,	"mpr22868.40",	0x2DB40CF8,	0x400000,	2,	28,		32,	false },
			{ "VROM",	false,	"mpr22869.41",	0xC6D62634,	0x400000,	2,	30,		32,	false },

			// Sound ROMs
			{ "SndProg",	false,	"epr22886.21",	0x374EC1C6,	0x80000,	2,	0,		2,	true },
			{ "Samples",	false,	"mpr22887.22",	0x7D04A867,	0x400000,	2,	0x000000,	2,	false },
			{ "Samples",	false,	"mpr22889.24",	0x4F9BA45D,	0x400000,	2,	0x400000,	2,	false },
			{ "Samples",	false,	"mpr22888.23",	0x018FCF22,	0x400000,	2,	0x800000,	2,	false },
			{ "Samples",	false,	"mpr22890.25",	0xB638BD7C,	0x400000,	2,	0xC00000,	2,	false },

			{ NULL, false, NULL, 0, 0, 0, 0, 0, false }
		}
	},

	// Fighting Vipers 2 (Revision A)
	{
		"fvipers2",
		"Fighting Vipers 2",
		"Sega",
		1998,
		0x20,
		0x800000,	// 8 MB of fixed CROM
		true,		// 64 MB of banked CROM (Mirror)
		0x4000000,	// 64 MB of VROM
		0x1000000,	// 16 MB of sample ROMs
		GAME_INPUT_COMMON|GAME_INPUT_JOYSTICK1|GAME_INPUT_JOYSTICK2|GAME_INPUT_FIGHTING,
		0,			// no MPEG board
		false,		// no drive board

		{
			// Fixed CROM
			{ "CROM",	false,	"epr-20599a.20",	0x9DF02AB9,	0x200000,	2,	0x0000000,	8,	true },
			{ "CROM",	false,	"epr-20598a.19",	0x87BD070F,	0x200000,	2,	0x0000002,	8,	true },
			{ "CROM",	false,	"epr-20597a.18",	0x6FCEE322,	0x200000,	2,	0x0000004,	8,	true },
			{ "CROM",	false,	"epr-20596a.17",	0x969AB801,	0x200000,	2,	0x0000006,	8,	true },

			// Banked CROM0
			{ "CROMxx",	false,	"mpr-20563.4",	0x999848AC,	0x400000,	2,	0x0000000,	8,	true },
			{ "CROMxx",	false,	"mpr-20562.3",	0x96E4942E,	0x400000,	2,	0x0000002,	8,	true },
			{ "CROMxx",	false,	"mpr-20561.2",	0x38A0F112,	0x400000,	2,	0x0000004,	8,	true },
			{ "CROMxx",	false,	"mpr-20560.1",	0xB0F6584D,	0x400000,	2,	0x0000006,	8,	true },

			// Banked CROM1
			{ "CROMxx",	false,	"mpr-20567.8",	0x80F4EBA7,	0x400000,	2,	0x1000000,	8,	true },
			{ "CROMxx",	false,	"mpr-20566.7",	0x2901883B,	0x400000,	2,	0x1000002,	8,	true },
			{ "CROMxx",	false,	"mpr-20565.6",	0xD6BBE638,	0x400000,	2,	0x1000004,	8,	true },
			{ "CROMxx",	false,	"mpr-20564.5",	0xBE69FCA0,	0x400000,	2,	0x1000006,	8,	true },

			// Banked CROM2
			{ "CROMxx",	false,	"mpr-20571.12",	0x40B459AF,	0x400000,	2,	0x2000000,	8,	true },
			{ "CROMxx",	false,	"mpr-20570.11",	0x2C0D91FC,	0x400000,	2,	0x2000002,	8,	true },
			{ "CROMxx",	false,	"mpr-20569.10",	0x136C014F,	0x400000,	2,	0x2000004,	8,	true },
			{ "CROMxx",	false,	"mpr-20568.9",	0xFF23CF1C,	0x400000,	2,	0x2000006,	8,	true },

			// Banked CROM3
			{ "CROMxx",	false,	"mpr-20575.16",	0xEBC99D8A,	0x400000,	2,	0x3000000,	8,	true },
			{ "CROMxx",	false,	"mpr-20574.15",	0x68567771,	0x400000,	2,	0x3000002,	8,	true },
			{ "CROMxx",	false,	"mpr-20573.14",	0xE0DEE793,	0x400000,	2,	0x3000004,	8,	true },
			{ "CROMxx",	false,	"mpr-20572.13",	0xD4A41A0B,	0x400000,	2,	0x3000006,	8,	true },

			// Video ROM
			{ "VROM",	false,	"mpr-20580.26",	0x6D42775E,	0x400000,	2,	0,		32,	false },
			{ "VROM",	false,	"mpr-20581.27",	0xAC9EEC04,	0x400000,	2,	2,		32,	false },
			{ "VROM",	false,	"mpr-20582.28",	0xB202F7BD,	0x400000,	2,	4,		32,	false },
			{ "VROM",	false,	"mpr-20583.29",	0x0D6D508A,	0x400000,	2,	6,		32,	false },
			{ "VROM",	false,	"mpr-20584.30",	0xECCF4DE6,	0x400000,	2,	8,		32,	false },
			{ "VROM",	false,	"mpr-20585.31",	0xB383F4E5,	0x400000,	2,	10,		32,	false },
			{ "VROM",	false,	"mpr-20586.32",	0xE7CD5DFB,	0x400000,	2,	12,		32,	false },
			{ "VROM",	false,	"mpr-20587.33",	0xE2B2ABE1,	0x400000,	2,	14,		32,	false },
			{ "VROM",	false,	"mpr-20588.34",	0x84F4162D,	0x400000,	2,	16,		32,	false },
			{ "VROM",	false,	"mpr-20589.35",	0x4E653D02,	0x400000,	2,	18,		32,	false },
			{ "VROM",	false,	"mpr-20590.36",	0x527049BE,	0x400000,	2,	20,		32,	false },
			{ "VROM",	false,	"mpr-20591.37",	0x3BE20243,	0x400000,	2,	22,		32,	false },
			{ "VROM",	false,	"mpr-20592.38",	0xD7985B28,	0x400000,	2,	24,		32,	false },
			{ "VROM",	false,	"mpr-20593.39",	0xE670C4D3,	0x400000,	2,	26,		32,	false },
			{ "VROM",	false,	"mpr-20594.40",	0x35578240,	0x400000,	2,	28,		32,	false },
			{ "VROM",	false,	"mpr-20595.41",	0x1D4A2CAD,	0x400000,	2,	30,		32,	false },

			// Sound ROMs
			{ "SndProg",	false,	"epr-20600a.21",	0xF0E7DB7E,	0x80000,	2,	0,		2,	true },
			{ "Samples",	false,	"mpr-20576",	0x1EEB540B, 0x400000,	2,	0x000000,	2,	false },
			{ "Samples",	false,	"mpr-20578",	0xD222F2D4, 0x400000,	2,	0x400000,	2,	false },
			{ "Samples",	false,	"mpr-20577",	0x3B236187, 0x400000,	2,	0x800000,	2,	false },
			{ "Samples",	false,	"mpr-20579",	0x08788436, 0x400000,	2,	0xC00000,	2,	false },

			{ NULL, false, NULL, 0, 0, 0, 0, 0, false }
		}
	},

	// Get Bass
	{
		"getbass",
		"Get Bass",
		"Sega",
		1997,
		0x10,
		0x200000,	// 2 MB of fixed CROM
		true,		// 48 MB of banked CROM (Mirror)
		0x2000000,	// 32 MB of VROM
		0x800000,	// 8 MB of sample ROMs
		GAME_INPUT_COMMON|GAME_INPUT_JOYSTICK1,
		0,			// no MPEG board
		false,		// no drive board

		{
			// Fixed CROM
			{ "CROM",	false,	"epr-20312.20",	0x9D8B8B58,	0x80000,	2,	0x0600000,	8,	true },
			{ "CROM",	false,	"epr-20311.19",	0xF721050D,	0x80000,	2,	0x0600002,	8,	true },
			{ "CROM",	false,	"epr-20310.18",	0x4EFCDDC9,	0x80000,	2,	0x0600004,	8,	true },
			{ "CROM",	false,	"epr-20309.17",	0xA42E1033,	0x80000,	2,	0x0600006,	8,	true },

			// Banked CROM0
			{ "CROMxx",	false,	"mpr-20259.4",	0x40052562,	0x400000,	2,	0x0000000,	8,	true },
			{ "CROMxx",	false,	"mpr-20258.3",	0x7B78B071,	0x400000,	2,	0x0000002,	8,	true },
			{ "CROMxx",	false,	"mpr-20257.2",	0x025BC06D,	0x400000,	2,	0x0000004,	8,	true },
			{ "CROMxx",	false,	"mpr-20256.1",	0x115302AC,	0x400000,	2,	0x0000006,	8,	true },

			// Banked CROM1
			{ "CROMxx",	false,	"mpr-20263.8",	0x1CF4CBA9,	0x400000,	2,	0x1000000,	8,	true },
			{ "CROMxx",	false,	"mpr-20262.7",	0x52B0674D,	0x400000,	2,	0x1000002,	8,	true },
			{ "CROMxx",	false,	"mpr-20261.6",	0xB1E9D44A,	0x400000,	2,	0x1000004,	8,	true },
			{ "CROMxx",	false,	"mpr-20260.5",	0xC56B4C10,	0x400000,	2,	0x1000006,	8,	true },

			// Banked CROM2
			{ "CROMxx",	false,	"mpr-20267.12",	0x48989191,	0x400000,	2,	0x2000000,	8,	true },
			{ "CROMxx",	false,	"mpr-20266.11",	0xABD2DB85,	0x400000,	2,	0x2000002,	8,	true },
			{ "CROMxx",	false,	"mpr-20265.10",	0x28F76E3E,	0x400000,	2,	0x2000004,	8,	true },
			{ "CROMxx",	false,	"mpr-20264.9",	0x8D995196,	0x400000,	2,	0x2000006,	8,	true },

			// Video ROM
			{ "VROM",	false,	"mpr-20270.26",	0xDF68A7A7,	0x200000,	2,	0,		32,	false },
			{ "VROM",	false,	"mpr-20271.27",	0x4B01C3A4,	0x200000,	2,	2,		32,	false },
			{ "VROM",	false,	"mpr-20272.28",	0xA658DA23,	0x200000,	2,	4,		32,	false },
			{ "VROM",	false,	"mpr-20273.29",	0x577E9FFA,	0x200000,	2,	6,		32,	false },
			{ "VROM",	false,	"mpr-20274.30",	0x7C7056AE,	0x200000,	2,	8,		32,	false },
			{ "VROM",	false,	"mpr-20275.31",	0xE739F77A,	0x200000,	2,	10,		32,	false },
			{ "VROM",	false,	"mpr-20276.32",	0xCBF966C0,	0x200000,	2,	12,		32,	false },
			{ "VROM",	false,	"mpr-20277.33",	0x9C75200B,	0x200000,	2,	14,		32,	false },
			{ "VROM",	false,	"mpr-20278.34",	0xDB3991BA,	0x200000,	2,	16,		32,	false },
			{ "VROM",	false,	"mpr-20279.35",	0x995A11B8,	0x200000,	2,	18,		32,	false },
			{ "VROM",	false,	"mpr-20280.36",	0xC2C8F9F5,	0x200000,	2,	20,		32,	false },
			{ "VROM",	false,	"mpr-20281.37",	0xDA84B967,	0x200000,	2,	22,		32,	false },
			{ "VROM",	false,	"mpr-20282.38",	0x1869FF49,	0x200000,	2,	24,		32,	false },
			{ "VROM",	false,	"mpr-20283.39",	0x7D8FB469,	0x200000,	2,	26,		32,	false },
			{ "VROM",	false,	"mpr-20284.40",	0x5C7F3A6F,	0x200000,	2,	28,		32,	false },
			{ "VROM",	false,	"mpr-20285.41",	0x4AADC573,	0x200000,	2,	30,		32,	false },

			// Sound ROMs
			{ "SndProg",	false,	"epr-20313.21",	0x863A7857,	0x80000,	2,	0,		2,	true },
			{ "Samples",	false,	"mpr-20268.22",	0x3631E93E,	0x400000,	2,	0x000000,	2,	false },
			{ "Samples",	false,	"mpr-20269.24",	0x105A3181,	0x400000,	2,	0x400000,	2,	false },

			{ NULL, false, NULL, 0, 0, 0, 0, 0, false }
		}
	},

	// Harley-Davidson & L.A. Riders (Revision A)
	{
		"harley",
		"Harley-Davidson & L.A. Riders",
		"Sega",
		1997,
		0x20,
		0x800000,	// 8 MB of fixed CROM
		true,		// 40 MB of banked CROM (Mirror)
		0x4000000,	// 64 MB of VROM
		0x1000000,	// 16 MB of sample ROMs
		GAME_INPUT_COMMON|GAME_INPUT_VEHICLE,
		0,			// no MPEG board
		false,		// no drive board

		{
			// Fixed CROM
			{ "CROM",	false,	"epr-20396a.20",	0x16B0106B,	0x200000,	2,	0x0000000,	8,	true },
			{ "CROM",	false,	"epr-20395a.19",	0x761F4976,	0x200000,	2,	0x0000002,	8,	true },
			{ "CROM",	false,	"epr-20394a.18",	0xCE29E2B6,	0x200000,	2,	0x0000004,	8,	true },
			{ "CROM",	false,	"epr-20393a.17",	0xB5646556,	0x200000,	2,	0x0000006,	8,	true },

			// Banked CROM0
			{ "CROMxx",	false,	"mpr-20364.4",	0xA2A68EF2,	0x400000,	2,	0x0000000,	8,	true },
			{ "CROMxx",	false,	"mpr-20363.3",	0x3E3CC6FF,	0x400000,	2,	0x0000002,	8,	true },
			{ "CROMxx",	false,	"mpr-20362.2",	0xF7E60DFD,	0x400000,	2,	0x0000004,	8,	true },
			{ "CROMxx",	false,	"mpr-20361.1",	0xDDB66C2F,	0x400000,	2,	0x0000006,	8,	true },

			// Banked CROM1
			{ "CROMxx",	false,	"mpr-20368.8",	0x100C9846,	0x400000,	2,	0x1000000,	8,	true },
			{ "CROMxx",	false,	"mpr-20367.7",	0x6C3F9748,	0x400000,	2,	0x1000002,	8,	true },
			{ "CROMxx",	false,	"mpr-20366.6",	0x45E3850E,	0x400000,	2,	0x1000004,	8,	true },
			{ "CROMxx",	false,	"mpr-20365.5",	0x7DD50361,	0x400000,	2,	0x1000006,	8,	true },

			// Banked CROM3
			{ "CROMxx",	false,	"epr-20412.16",	0x0D51BB34,	0x200000,	2,	0x3800000,	8,	true },
			{ "CROMxx",	false,	"epr-20411.15",	0x848DAAF7,	0x200000,	2,	0x3800002,	8,	true },
			{ "CROMxx",	false,	"epr-20410.14",	0x98B126F2,	0x200000,	2,	0x3800004,	8,	true },
			{ "CROMxx",	false,	"epr-20409.13",	0x58CAAA75,	0x200000,	2,	0x3800006,	8,	true },

			// Video ROM
			{ "VROM",	false,	"mpr-20377.26",	0x4D2887E5,	0x400000,	2,	0,		32,	false },
			{ "VROM",	false,	"mpr-20378.27",	0x5AD7C0EC,	0x400000,	2,	2,		32,	false },
			{ "VROM",	false,	"mpr-20379.28",	0x1E51C9F0,	0x400000,	2,	4,		32,	false },
			{ "VROM",	false,	"mpr-20380.29",	0xE10D35AE,	0x400000,	2,	6,		32,	false },
			{ "VROM",	false,	"mpr-20381.30",	0x76CD36A2,	0x400000,	2,	8,		32,	false },
			{ "VROM",	false,	"mpr-20382.31",	0xF089AE37,	0x400000,	2,	10,		32,	false },
			{ "VROM",	false,	"mpr-20383.32",	0x9E96D3BE,	0x400000,	2,	12,		32,	false },
			{ "VROM",	false,	"mpr-20384.33",	0x5BDFBB52,	0x400000,	2,	14,		32,	false },
			{ "VROM",	false,	"mpr-20385.34",	0x12DB1729,	0x400000,	2,	16,		32,	false },
			{ "VROM",	false,	"mpr-20386.35",	0xDB2CCAF8,	0x400000,	2,	18,		32,	false },
			{ "VROM",	false,	"mpr-20387.36",	0xC5DDE91B,	0x400000,	2,	20,		32,	false },
			{ "VROM",	false,	"mpr-20388.37",	0xAEAA862E,	0x400000,	2,	22,		32,	false },
			{ "VROM",	false,	"mpr-20389.38",	0x49BB6593,	0x400000,	2,	24,		32,	false },
			{ "VROM",	false,	"mpr-20390.39",	0x1D4A8EFE,	0x400000,	2,	26,		32,	false },
			{ "VROM",	false,	"mpr-20391.40",	0x5DC452DC,	0x400000,	2,	28,		32,	false },
			{ "VROM",	false,	"mpr-20392.41",	0x892208CB,	0x400000,	2,	30,		32,	false },

			// Sound ROMs
			{ "SndProg",	false,	"epr-20397.21",	0x5B20B54A, 0x80000,	2,	0,		2,	true },
			{ "Samples",	false,	"mpr-20373.22",	0xC684E8A3, 0x400000,	2,	0x000000,	2,	false },
			{ "Samples",	false,	"mpr-20375.24",	0x906ACE86, 0x400000,	2,	0x400000,	2,	false },
			{ "Samples",	false,	"mpr-20374.23",	0xFCF6EA21, 0x400000,	2,	0x800000,	2,	false },
			{ "Samples",	false,	"mpr-20376.25",	0xDEEED366, 0x400000,	2,	0xC00000,	2,	false },

			{ NULL, false, NULL, 0, 0, 0, 0, 0, false }
		}
	},

	// Harley-Davidson & L.A. Riders (Revision B)
	{
		"harleyb",
		"Harley-Davidson & L.A. Riders (Revision B)",
		"Sega",
		1997,
		0x20,
		0x800000,	// 8 MB of fixed CROM
		true,		// 40 MB of banked CROM (Mirror)
		0x4000000,	// 64 MB of VROM
		0x1000000,	// 16 MB of sample ROMs
		GAME_INPUT_COMMON|GAME_INPUT_VEHICLE,
		0,			// no MPEG board
		false,		// no drive board

		{
			// Fixed CROM
			{ "CROM",	false,	"epr-20396b.20",	0x9623DEA7,	0x200000,	2,	0x0000000,	8,	true },
			{ "CROM",	false,	"epr-20395b.19",	0x88F71D76,	0x200000,	2,	0x0000002,	8,	true },
			{ "CROM",	false,	"epr-20394b.18",	0xB4312135,	0x200000,	2,	0x0000004,	8,	true },
			{ "CROM",	false,	"epr-20393b.17",	0x7D712105,	0x200000,	2,	0x0000006,	8,	true },

			// Banked CROM0
			{ "CROMxx",	false,	"mpr-20364.4",	0xA2A68EF2,	0x400000,	2,	0x0000000,	8,	true },
			{ "CROMxx",	false,	"mpr-20363.3",	0x3E3CC6FF,	0x400000,	2,	0x0000002,	8,	true },
			{ "CROMxx",	false,	"mpr-20362.2",	0xF7E60DFD,	0x400000,	2,	0x0000004,	8,	true },
			{ "CROMxx",	false,	"mpr-20361.1",	0xDDB66C2F,	0x400000,	2,	0x0000006,	8,	true },

			// Banked CROM1
			{ "CROMxx",	false,	"mpr-20368.8",	0x100C9846,	0x400000,	2,	0x1000000,	8,	true },
			{ "CROMxx",	false,	"mpr-20367.7",	0x6C3F9748,	0x400000,	2,	0x1000002,	8,	true },
			{ "CROMxx",	false,	"mpr-20366.6",	0x45E3850E,	0x400000,	2,	0x1000004,	8,	true },
			{ "CROMxx",	false,	"mpr-20365.5",	0x7DD50361,	0x400000,	2,	0x1000006,	8,	true },

			// Banked CROM3
			{ "CROMxx",	false,	"epr-20412.16",	0x0D51BB34,	0x200000,	2,	0x3800000,	8,	true },
			{ "CROMxx",	false,	"epr-20411.15",	0x848DAAF7,	0x200000,	2,	0x3800002,	8,	true },
			{ "CROMxx",	false,	"epr-20410.14",	0x98B126F2,	0x200000,	2,	0x3800004,	8,	true },
			{ "CROMxx",	false,	"epr-20409.13",	0x58CAAA75,	0x200000,	2,	0x3800006,	8,	true },

			// Video ROM
			{ "VROM",	false,	"mpr-20377.26",	0x4D2887E5,	0x400000,	2,	0,		32,	false },
			{ "VROM",	false,	"mpr-20378.27",	0x5AD7C0EC,	0x400000,	2,	2,		32,	false },
			{ "VROM",	false,	"mpr-20379.28",	0x1E51C9F0,	0x400000,	2,	4,		32,	false },
			{ "VROM",	false,	"mpr-20380.29",	0xE10D35AE,	0x400000,	2,	6,		32,	false },
			{ "VROM",	false,	"mpr-20381.30",	0x76CD36A2,	0x400000,	2,	8,		32,	false },
			{ "VROM",	false,	"mpr-20382.31",	0xF089AE37,	0x400000,	2,	10,		32,	false },
			{ "VROM",	false,	"mpr-20383.32",	0x9E96D3BE,	0x400000,	2,	12,		32,	false },
			{ "VROM",	false,	"mpr-20384.33",	0x5BDFBB52,	0x400000,	2,	14,		32,	false },
			{ "VROM",	false,	"mpr-20385.34",	0x12DB1729,	0x400000,	2,	16,		32,	false },
			{ "VROM",	false,	"mpr-20386.35",	0xDB2CCAF8,	0x400000,	2,	18,		32,	false },
			{ "VROM",	false,	"mpr-20387.36",	0xC5DDE91B,	0x400000,	2,	20,		32,	false },
			{ "VROM",	false,	"mpr-20388.37",	0xAEAA862E,	0x400000,	2,	22,		32,	false },
			{ "VROM",	false,	"mpr-20389.38",	0x49BB6593,	0x400000,	2,	24,		32,	false },
			{ "VROM",	false,	"mpr-20390.39",	0x1D4A8EFE,	0x400000,	2,	26,		32,	false },
			{ "VROM",	false,	"mpr-20391.40",	0x5DC452DC,	0x400000,	2,	28,		32,	false },
			{ "VROM",	false,	"mpr-20392.41",	0x892208CB,	0x400000,	2,	30,		32,	false },

			// Sound ROMs
			{ "SndProg",	false,	"epr-20397.21",	0x5B20B54A, 0x80000,	2,	0,		2,	true },
			{ "Samples",	false,	"mpr-20373.22",	0xC684E8A3, 0x400000,	2,	0x000000,	2,	false },
			{ "Samples",	false,	"mpr-20375.24",	0x906ACE86, 0x400000,	2,	0x400000,	2,	false },
			{ "Samples",	false,	"mpr-20374.23",	0xFCF6EA21, 0x400000,	2,	0x800000,	2,	false },
			{ "Samples",	false,	"mpr-20376.25",	0xDEEED366, 0x400000,	2,	0xC00000,	2,	false },

			{ NULL, false, NULL, 0, 0, 0, 0, 0, false }
		}
	},

	// L.A. Machineguns
	{
		"lamachin",
		"L.A. Machineguns",
		"Sega",
		1998,
		0x21,
		0x800000,	// 8 MB of fixed CROM
		true,		// 48 MB of banked CROM (Mirror)
		0x4000000,	// 64 MB of VROM
		0x1000000,	// 16 MB of sample ROMs
		GAME_INPUT_COMMON|GAME_INPUT_ANALOG_JOYSTICK,
		0,			// no MPEG board
		false,		// no drive board

		{
			// Fixed CROM
			{ "CROM",	false,	"epr21483.17",	0x64DE433F,	0x200000,	2,	0x0000000,	8,	true },
			{ "CROM",	false,	"epr21484.18",	0xF68F7703,	0x200000,	2,	0x0000002,	8,	true },
			{ "CROM",	false,	"epr21485.19",	0x58102168,	0x200000,	2,	0x0000004,	8,	true },
			{ "CROM",	false,	"epr21486.20",	0x940637C2,	0x200000,	2,	0x0000006,	8,	true },

			// Banked CROM0
			{ "CROMxx",	false,	"mpr21451.1",	0x97FF94A7,	0x400000,	2,	0x0000000,	8,	true },
			{ "CROMxx",	false,	"mpr21452.2",	0x082D98AB,	0x400000,	2,	0x0000002,	8,	true },
			{ "CROMxx",	false,	"mpr21453.3",	0x01AC050C,	0x400000,	2,	0x0000004,	8,	true },
			{ "CROMxx",	false,	"mpr21454.4",	0x42BDC56C,	0x400000,	2,	0x0000006,	8,	true },

			// Banked CROM1
			{ "CROMxx",	false,	"mpr21458.8",	0xB748F5A1,	0x400000,	2,	0x1000000,	8,	true },
			{ "CROMxx",	false,	"mpr21457.7",	0x2034DBD4,	0x400000,	2,	0x1000002,	8,	true },
			{ "CROMxx",	false,	"mpr21456.6",	0x73A50547,	0x400000,	2,	0x1000004,	8,	true },
			{ "CROMxx",	false,	"mpr21455.5",	0x0B4A3CC5,	0x400000,	2,	0x1000006,	8,	true },

			// Banked CROM2
			{ "CROMxx",	false,	"mpr21462.12",	0x03D22EE8,	0x400000,	2,	0x2000000,	8,	true },
			{ "CROMxx",	false,	"mpr21462.11",	0x33D8F0DA,	0x400000,	2,	0x2000002,	8,	true },
			{ "CROMxx",	false,	"mpr21461.10",	0x02268361,	0x400000,	2,	0x2000004,	8,	true },
			{ "CROMxx",	false,	"mpr21460.9",	0x71A7B6B3,	0x400000,	2,	0x2000006,	8,	true },

			// Video ROM
			{ "VROM",	false,	"mpr21467.26",	0x73635100,	0x400000,	2,	0,		32,	false },
			{ "VROM",	false,	"mpr21468.27",	0x462E5C81,	0x400000,	2,	2,		32,	false },
			{ "VROM",	false,	"mpr21469.28",	0x4BA3F192,	0x400000,	2,	4,		32,	false },
			{ "VROM",	false,	"mpr21470.29",	0x670F0DF5,	0x400000,	2,	6,		32,	false },
			{ "VROM",	false,	"mpr21471.30",	0x1F07E6E3,	0x400000,	2,	8,		32,	false },
			{ "VROM",	false,	"mpr21472.31",	0xE6DC64A3,	0x400000,	2,	10,		32,	false },
			{ "VROM",	false,	"mpr21473.32",	0xD1C9B54A,	0x400000,	2,	12,		32,	false },
			{ "VROM",	false,	"mpr21474.33",	0xAA2F19AE,	0x400000,	2,	14,		32,	false },
			{ "VROM",	false,	"mpr21475.34",	0xBAE9B381,	0x400000,	2,	16,		32,	false },
			{ "VROM",	false,	"mpr21476.35",	0x3833DF51,	0x400000,	2,	18,		32,	false },
			{ "VROM",	false,	"mpr21477.36",	0x46032C35,	0x400000,	2,	20,		32,	false },
			{ "VROM",	false,	"mpr21478.37",	0x35EF75B8,	0x400000,	2,	22,		32,	false },
			{ "VROM",	false,	"mpr21479.38",	0x783E8ECE,	0x400000,	2,	24,		32,	false },
			{ "VROM",	false,	"mpr21480.39",	0xC947BCB8,	0x400000,	2,	26,		32,	false },
			{ "VROM",	false,	"mpr21481.40",	0x6CE566AC,	0x400000,	2,	28,		32,	false },
			{ "VROM",	false,	"mpr21482.41",	0xE995F554,	0x400000,	2,	30,		32,	false },

			// Sound ROMs
			{ "SndProg",	false,	"epr21487.21",	0xC2942448,	0x80000,	2,	0,		2,	true },
			{ "Samples",	false,	"mpr21463.22",	0x0E6d6C0E,	0x400000,	2,	0x000000,	2,	false },
			{ "Samples",	false,	"mpr21465.24",	0x1A62D925,	0x400000,	2,	0x400000,	2,	false },
			{ "Samples",	false,	"mpr21464.23",	0x8230C1DE,	0x400000,	2,	0x800000,	2,	false },
			{ "Samples",	false,	"mpr21466.25",	0xCA20359E,	0x400000,	2,	0xC00000,	2,	false },

			{ NULL, false, NULL, 0, 0, 0, 0, 0, false }
		}
	},

	// Le Mans 24
	{
		"lemans24",
		"Le Mans 24",
		"Sega",
		1997,
		0x15,
		0x200000,	// 2 MB of fixed CROM
		true,		// 48 MB of banked CROM (Mirror)
		0x2000000,	// 32 MB of VROM
		0x800000,	// 8 MB of sample ROMs
		GAME_INPUT_COMMON|GAME_INPUT_VEHICLE|GAME_INPUT_VR|GAME_INPUT_SHIFT4,	// for now, Shift Up/Down mapped to Shift 3/4
		0,			// no MPEG board
		false,		// no drive board

		{
			// Fixed CROM
			{ "CROM",	false,	"epr-19890.20",	0x9C16C3CC,	0x80000,	2,	0x0600000,	8,	true },
			{ "CROM",	false,	"epr-19889.19",	0xD1F7E44C,	0x80000,	2,	0x0600002,	8,	true },
			{ "CROM",	false,	"epr-19888.18",	0x800D763D,	0x80000,	2,	0x0600004,	8,	true },
			{ "CROM",	false,	"epr-19887.17",	0x2842BB87,	0x80000,	2,	0x0600006,	8,	true },

			// Banked CROM0
			{ "CROMxx",	false,	"mpr-19860.04",	0x19A1DDC7,	0x400000,	2,	0x0000000,	8,	true },
			{ "CROMxx",	false,	"mpr-19859.03",	0x15906869,	0x400000,	2,	0x0000002,	8,	true },
			{ "CROMxx",	false,	"mpr-19858.02",	0x993FA656,	0x400000,	2,	0x0000004,	8,	true },
			{ "CROMxx",	false,	"mpr-19857.01",	0x82C9FCFC,	0x400000,	2,	0x0000006,	8,	true },

			// Banked CROM1
			{ "CROMxx",	false,	"mpr-19864.08",	0xC7BAAB2B,	0x400000,	2,	0x1000000,	8,	true },
			{ "CROMxx",	false,	"mpr-19863.07",	0x2B2619D0,	0x400000,	2,	0x1000002,	8,	true },
			{ "CROMxx",	false,	"mpr-19862.06",	0xB0F69AE4,	0x400000,	2,	0x1000004,	8,	true },
			{ "CROMxx",	false,	"mpr-19861.05",	0x6DDF21B3,	0x400000,	2,	0x1000006,	8,	true },

			// Banked CROM2
			{ "CROMxx",	false,	"mpr-19868.12",	0x3C43D64F,	0x400000,	2,	0x2000000,	8,	true },
			{ "CROMxx",	false,	"mpr-19867.11",	0xAE610FC5,	0x400000,	2,	0x2000002,	8,	true },
			{ "CROMxx",	false,	"mpr-19866.10",	0xEDE5FC78,	0x400000,	2,	0x2000004,	8,	true },
			{ "CROMxx",	false,	"mpr-19865.09",	0xB2749D2B,	0x400000,	2,	0x2000006,	8,	true },

			// Video ROM
			{ "VROM",	false,	"mpr-19871.26",	0x5168E02B,	0x200000,	2,	0,		32,	false },
			{ "VROM",	false,	"mpr-19872.27",	0x9E65FC06,	0x200000,	2,	2,		32,	false },
			{ "VROM",	false,	"mpr-19873.28",	0x0B15D7AB,	0x200000,	2,	4,		32,	false },
			{ "VROM",	false,	"mpr-19874.29",	0x6A28EC89,	0x200000,	2,	6,		32,	false },
			{ "VROM",	false,	"mpr-19875.30",	0xA03E1173,	0x200000,	2,	8,		32,	false },
			{ "VROM",	false,	"mpr-19876.31",	0xC93BB036,	0x200000,	2,	10,		32,	false },
			{ "VROM",	false,	"mpr-19877.32",	0xB1E3DF56,	0x200000,	2,	12,		32,	false },
			{ "VROM",	false,	"mpr-19878.33",	0xA2ACC111,	0x200000,	2,	14,		32,	false },
			{ "VROM",	false,	"mpr-19879.34",	0x90C1553F,	0x200000,	2,	16,		32,	false },
			{ "VROM",	false,	"mpr-19880.35",	0x42504e63,	0x200000,	2,	18,		32,	false },
			{ "VROM",	false,	"mpr-19881.36",	0xD06985CF,	0x200000,	2,	20,		32,	false },
			{ "VROM",	false,	"mpr-19882.37",	0xA86F2E2F,	0x200000,	2,	22,		32,	false },
			{ "VROM",	false,	"mpr-19883.38",	0x12895D6E,	0x200000,	2,	24,		32,	false },
			{ "VROM",	false,	"mpr-19884.39",	0x711EEBFB,	0x200000,	2,	26,		32,	false },
			{ "VROM",	false,	"mpr-19885.40",	0xD1AE5473,	0x200000,	2,	28,		32,	false },
			{ "VROM",	false,	"mpr-19886.41",	0x278AAE0B,	0x200000,	2,	30,		32,	false },

			// Sound ROMs
			{ "SndProg",	false,	"epr-19891.21",	0xC3ECD448,	0x80000,	2,	0,		2,	true },
			{ "Samples",	false,	"mpr-19869.22",	0xEA1EF1CC,	0x400000,	2,	0x000000,	2,	false },
			{ "Samples",	false,	"mpr-19870.24",	0x49C70296,	0x400000,	2,	0x400000,	2,	false },

			{ NULL, false, NULL, 0, 0, 0, 0, 0, false }
		}
	},

	// The Lost World
	{
		"lostwsga",
		"The Lost World",
		"Sega",
		1997,
		0x15,
		0x200000,	// 2 MB of fixed CROM
		true,		// 64 MB of banked CROM (Mirror)
		0x2000000,	// 32 MB of VROM
		0x800000,	// 8 MB of sample ROMs
		GAME_INPUT_COMMON|GAME_INPUT_GUN1|GAME_INPUT_GUN2,
		0,			// no MPEG board
		false,		// no drive board

		{
			// Fixed CROM
			{ "CROM",	false,	"epr-19936.20",	0x2F1CA664,	0x80000,	2,	0x0600000,	8,	true },
			{ "CROM",	false,	"epr-19937.19",	0x9DBF5712,	0x80000,	2,	0x0600002,	8,	true },
			{ "CROM",	false,	"epr-19938.18",	0x38AFE27A,	0x80000,	2,	0x0600004,	8,	true },
			{ "CROM",	false,	"epr-19939.17",	0x8788B939,	0x80000,	2,	0x0600006,	8,	true },

			// Banked CROM0
			{ "CROMxx",	false,	"mpr-19921.4",	0x9AF3227F,	0x400000,	2,	0x0000000,	8,	true },
			{ "CROMxx",	false,	"mpr-19920.3",	0x8DF33574,	0x400000,	2,	0x0000002,	8,	true },
			{ "CROMxx",	false,	"mpr-19919.2",	0xFF119949,	0x400000,	2,	0x0000004,	8,	true },
			{ "CROMxx",	false,	"mpr-19918.1",	0x95B690E9,	0x400000,	2,	0x0000006,	8,	true },

			// Banked CROM1
			{ "CROMxx",	false,	"mpr-19925.8",	0xCFA4BB49,	0x400000,	2,	0x1000000,	8,	true },
			{ "CROMxx",	false,	"mpr-19924.7",	0x4EE3DDC5,	0x400000,	2,	0x1000002,	8,	true },
			{ "CROMxx",	false,	"mpr-19923.6",	0xED515CB2,	0x400000,	2,	0x1000004,	8,	true },
			{ "CROMxx",	false,	"mpr-19922.5",	0x4DFD7FC6,	0x400000,	2,	0x1000006,	8,	true },

			// Banked CROM2
			{ "CROMxx",	false,	"mpr-19929.12",	0x16491F63,	0x400000,	2,	0x2000000,	8,	true },
			{ "CROMxx",	false,	"mpr-19928.11",	0x9AFD5D4A,	0x400000,	2,	0x2000002,	8,	true },
			{ "CROMxx",	false,	"mpr-19927.10",	0x0C96EF11,	0x400000,	2,	0x2000004,	8,	true },
			{ "CROMxx",	false,	"mpr-19926.9",	0x05A232E0,	0x400000,	2,	0x2000006,	8,	true },

			// Banked CROM3
			{ "CROMxx",	false,	"mpr-19933.16",	0x8E2ACD3B,	0x400000,	2,	0x3000000,	8,	true },
			{ "CROMxx",	false,	"mpr-19932.15",	0x04389385,	0x400000,	2,	0x3000002,	8,	true },
			{ "CROMxx",	false,	"mpr-19931.14",	0x448A5007,	0x400000,	2,	0x3000004,	8,	true },
			{ "CROMxx",	false,	"mpr-19930.13",	0xB598C2F2,	0x400000,	2,	0x3000006,	8,	true },

			// Video ROM
			{ "VROM",	false,	"mpr-19902.26",	0x178BD471,	0x200000,	2,	0,		32,	false },
			{ "VROM",	false,	"mpr-19903.27",	0xFE575871,	0x200000,	2,	2,		32,	false },
			{ "VROM",	false,	"mpr-19904.28",	0x57971D7D,	0x200000,	2,	4,		32,	false },
			{ "VROM",	false,	"mpr-19905.29",	0x6FA122EE,	0x200000,	2,	6,		32,	false },
			{ "VROM",	false,	"mpr-19906.30",	0xA5B16DD9,	0x200000,	2,	8,		32,	false },
			{ "VROM",	false,	"mpr-19907.31",	0x84A425CD,	0x200000,	2,	10,		32,	false },
			{ "VROM",	false,	"mpr-19908.32",	0x7702AA7C,	0x200000,	2,	12,		32,	false },
			{ "VROM",	false,	"mpr-19909.33",	0x8FCA65F9,	0x200000,	2,	14,		32,	false },
			{ "VROM",	false,	"mpr-19910.34",	0x1EF585E2,	0x200000,	2,	16,		32,	false },
			{ "VROM",	false,	"mpr-19911.35",	0xCA26A48D,	0x200000,	2,	18,		32,	false },
			{ "VROM",	false,	"mpr-19912.36",	0xFFE000E0,	0x200000,	2,	20,		32,	false },
			{ "VROM",	false,	"mpr-19913.37",	0xC003049E,	0x200000,	2,	22,		32,	false },
			{ "VROM",	false,	"mpr-19914.38",	0x3C21A953,	0x200000,	2,	24,		32,	false },
			{ "VROM",	false,	"mpr-19915.39",	0xFD0F2A2B,	0x200000,	2,	26,		32,	false },
			{ "VROM",	false,	"mpr-19916.40",	0x10B0C52E,	0x200000,	2,	28,		32,	false },
			{ "VROM",	false,	"mpr-19917.41",	0x3035833B,	0x200000,	2,	30,		32,	false },

			// Sound ROMs
			{ "SndProg",	false,	"epr-19940.21",	0xB06FFE5F,	0x80000,	2,	0,		2,	true },
			{ "Samples",	false,	"mpr19934.22",	0xC7D8E194,	0x400000,	2,	0x000000,	2,	false },
			{ "Samples",	false,	"mpr19935.24",	0x91C1B618,	0x400000,	2,	0x400000,	2,	false },

			{ NULL, false, NULL, 0, 0, 0, 0, 0, false }
		}
	},

	// Magical Truck Adventure
	{
		"magtruck",
		"Magical Truck Adventure",
		"Sega",
		1998,
		0x21,
		0x800000,	// 8 MB of fixed CROM
		true,		// 16 MB of banked CROM (Mirror)
		0x4000000,	// 64 MB of VROM
		0x1000000,	// 16 MB of sample ROMs
		GAME_INPUT_COMMON|GAME_INPUT_VEHICLE,
		0,			// no MPEG board
		false,		// no drive board

		{
			// Fixed CROM
			{ "CROM",	false,	"epr21434.20",	0xE028D7CA,	0x200000,	2,	0x0000000,	8,	true },
			{ "CROM",	false,	"epr21436.19",	0x22BCBCA3,	0x200000,	2,	0x0000002,	8,	true },
			{ "CROM",	false,	"epr21433.18",	0x60AA9D76,	0x200000,	2,	0x0000004,	8,	true },
			{ "CROM",	false,	"epr21435.17",	0x9B169446,	0x200000,	2,	0x0000006,	8,	true },

			// Banked CROM0
			{ "CROMxx",	false,	"mpr21426.4",	0xCE77E26E,	0x400000,	2,	0x0000000,	8,	true },
			{ "CROMxx",	false,	"mpr21425.3",	0xAD235849,	0x400000,	2,	0x0000002,	8,	true },
			{ "CROMxx",	false,	"mpr21424.2",	0x25358FDF,	0x400000,	2,	0x0000004,	8,	true },
			{ "CROMxx",	false,	"mpr21423.1",	0x4EE0060A,	0x400000,	2,	0x0000006,	8,	true },

			// Video ROM
			{ "VROM",	false,	"mpr21407.26",	0x3FFB416C,	0x400000,	2,	0,		32,	false },
			{ "VROM",	false,	"mpr21408.27",	0x3E00A7EF,	0x400000,	2,	2,		32,	false },
			{ "VROM",	false,	"mpr21409.28",	0xA4673BBF,	0x400000,	2,	4,		32,	false },
			{ "VROM",	false,	"mpr21410.29",	0xC9F43B4A,	0x400000,	2,	6,		32,	false },
			{ "VROM",	false,	"mpr21411.30",	0xF14957C7,	0x400000,	2,	8,		32,	false },
			{ "VROM",	false,	"mpr21412.31",	0xEC24091F,	0x400000,	2,	10,		32,	false },
			{ "VROM",	false,	"mpr21413.32",	0xEA9049E0,	0x400000,	2,	12,		32,	false },
			{ "VROM",	false,	"mpr21414.33",	0x79BC5FFD,	0x400000,	2,	14,		32,	false },
			{ "VROM",	false,	"mpr21415.34",	0xF96FE7A2,	0x400000,	2,	16,		32,	false },
			{ "VROM",	false,	"mpr21416.35",	0x84A08B3E,	0x400000,	2,	18,		32,	false },
			{ "VROM",	false,	"mpr21417.36",	0x6094975C,	0x400000,	2,	20,		32,	false },
			{ "VROM",	false,	"mpr21418.37",	0x7BB868BA,	0x400000,	2,	22,		32,	false },
			{ "VROM",	false,	"mpr21419.38",	0xBE7325C2,	0x400000,	2,	24,		32,	false },
			{ "VROM",	false,	"mpr21420.39",	0x8B577E7B,	0x400000,	2,	26,		32,	false },
			{ "VROM",	false,	"mpr21421.40",	0x71E4E9FC,	0x400000,	2,	28,		32,	false },
			{ "VROM",	false,	"mpr21422.41",	0xFECA77A5,	0x400000,	2,	30,		32,	false },

			// Sound ROMs
			{ "SndProg",	false,	"epr21438.21",	0x6815AF9E,	0x80000,	2,	0,		2,	true },
			{ "Samples",	false,	"mpr21427.22",	0x884566F6,	0x400000,	2,	0x000000,	2,	false },
			{ "Samples",	false,	"mpr21428.24",	0x162D1E43,	0x400000,	2,	0x400000,	2,	false },
			{ "Samples",	false,	"mpr21431.23",	0x0EF8F7BB,	0x400000,	2,	0x800000,	2,	false },
			{ "Samples",	false,	"mpr21432.25",	0x59C0F6DF,	0x400000,	2,	0xC00000,	2,	false },

			{ NULL, false, NULL, 0, 0, 0, 0, 0, false }
		}
	},

	// The Ocean Hunter
	{
		"oceanhun",
		"The Ocean Hunter",
		"Sega",
		1998,
		0x21,
		0x800000,	// 8 MB of fixed CROM
		false,	// 96 MB of banked CROM (do not mirror)
		0x4000000,	// 64 MB of VROM
		0x1000000,	// 16 MB of sample ROMs
		GAME_INPUT_COMMON|GAME_INPUT_ANALOG_JOYSTICK,
		0,			// no MPEG board
		false,		// no drive board

		{
			// Fixed CROM
			{ "CROM",	false,	"epr21114.17",	0x58D985f1,	0x200000,	2,	0x0000000,	8,	true },
			{ "CROM",	false,	"epr21115.18",	0x69E31E85,	0x200000,	2,	0x0000002,	8,	true },
			{ "CROM",	false,	"epr21116.19",	0x0BB9C107,	0x200000,	2,	0x0000004,	8,	true },
			{ "CROM",	false,	"epr21117.20",	0x3ADFCB9D,	0x200000,	2,	0x0000006,	8,	true },

			// Banked CROM1
			{ "CROMxx",	false,	"mpr21082.5",	0x2B7224D3,	0x800000,	2,	0x2000000,	8,	true },
			{ "CROMxx",	false,	"mpr21083.6",	0xC1C6B554,	0x800000,	2,	0x2000002,	8,	true },
			{ "CROMxx",	false,	"mpr21084.7",	0xFDEC6A23,	0x800000,	2,	0x2000004,	8,	true },
			{ "CROMxx",	false,	"mpr21085.8",	0x5056AD33,	0x800000,	2,	0x2000006,	8,	true },

			// Banked CROM2
			{ "CROMxx",	false,	"mpr21089.12",	0x2E8F88BD,	0x800000,	2,	0x4000000,	8,	true },
			{ "CROMxx",	false,	"mpr21088.11",	0x7ED71C8C,	0x800000,	2,	0x4000002,	8,	true },
			{ "CROMxx",	false,	"mpr21087.10",	0xCFF28641,	0x800000,	2,	0x4000004,	8,	true },
			{ "CROMxx",	false,	"mpr21086.9",	0x3F12E1D0,	0x800000,	2,	0x4000006,	8,	true },

			// Banked CROM3
			{ "CROMxx",	false,	"mpr21093.16",	0xBDFBF357,	0x800000,	2,	0x6000000,	8,	true },
			{ "CROMxx",	false,	"mpr21092.15",	0x5B1CED40,	0x800000,	2,	0x6000002,	8,	true },
			{ "CROMxx",	false,	"mpr21091.14",	0x10671951,	0x800000,	2,	0x6000004,	8,	true },
			{ "CROMxx",	false,	"mpr21090.13",	0x749D7979,	0x800000,	2,	0x6000006,	8,	true },

			// Video ROM
			{ "VROM",	false,	"mpr21098.26",	0x91E71855,	0x400000,	2,	0,		32,	false },
			{ "VROM",	false,	"mpr21099.27",	0x308A2768,	0x400000,	2,	2,		32,	false },
			{ "VROM",	false,	"mpr21100.28",	0x5149B286,	0x400000,	2,	4,		32,	false },
			{ "VROM",	false,	"mpr21101.29",	0xE9ED4250,	0x400000,	2,	6,		32,	false },
			{ "VROM",	false,	"mpr21102.30",	0x06C6D4FC,	0x400000,	2,	8,		32,	false },
			{ "VROM",	false,	"mpr21103.31",	0x17C4B27A,	0x400000,	2,	10,		32,	false },
			{ "VROM",	false,	"mpr21104.32",	0xF6F80FFB,	0x400000,	2,	12,		32,	false },
			{ "VROM",	false,	"mpr21105.33",	0x99BDB52B,	0x400000,	2,	14,		32,	false },
			{ "VROM",	false,	"mpr21106.34",	0xAD2B7981,	0x400000,	2,	16,		32,	false },
			{ "VROM",	false,	"mpr21107.35",	0xE108FF62,	0x400000,	2,	18,		32,	false },
			{ "VROM",	false,	"mpr21108.36",	0xCDDC7A6E,	0x400000,	2,	20,		32,	false },
			{ "VROM",	false,	"mpr21109.37",	0x92D6141D,	0x400000,	2,	22,		32,	false },
			{ "VROM",	false,	"mpr21110.38",	0x4D6E3148,	0x400000,	2,	24,		32,	false },
			{ "VROM",	false,	"mpr21111.39",	0x0A046D7A,	0x400000,	2,	26,		32,	false },
			{ "VROM",	false,	"mpr21112.40",	0x9AFD9FEB,	0x400000,	2,	28,		32,	false },
			{ "VROM",	false,	"mpr21113.41",	0x864BF325,	0x400000,	2,	30,		32,	false },

			// Sound ROMs
			{ "SndProg",	false,	"epr21118.21",	0x598C00F0,	0x80000,	2,	0,		2,	true },
			{ "Samples",	false,	"mpr21094.22",	0xC262B80A,	0x400000,	2,	0x000000,	2,	false },
			{ "Samples",	false,	"mpr21096.24",	0x0A0021A0,	0x400000,	2,	0x400000,	2,	false },
			{ "Samples",	false,	"mpr21095.23",	0x16D27A0A,	0x400000,	2,	0x800000,	2,	false },
			{ "Samples",	false,	"mpr21097.25",	0x0D8033FC,	0x400000,	2,	0xC00000,	2,	false },

			{ NULL, false, NULL, 0, 0, 0, 0, 0, false }
		}
	},

	// Scud Race (Australia)
	{
		"scud",
		"Scud Race (Australia)",
		"Sega",
		1996,
		0x15,
		0x200000,	// 2 MB of fixed CROM
		true,		// 48 MB of banked CROM (Mirror)
		0x2000000,	// 32 MB of VROM
		0x800000,	// 8 MB of sample ROMs
		GAME_INPUT_COMMON|GAME_INPUT_VEHICLE|GAME_INPUT_VR|GAME_INPUT_SHIFT4,
		1,			// DSB1 MPEG board
		true,		// drive board

		{
			// Fixed CROM (mirroring behavior here is special and handled manually by CModel3)
			{ "CROM",	false,	"epr-19734.20",	0xBE897336,	0x80000,	2,	0x0600000,	8,	true },
			{ "CROM",	false,	"epr-19733.19",	0x6565E29A,	0x80000,	2,	0x0600002,	8,	true },
			{ "CROM",	false,	"epr-19732.18",	0x23E864BB,	0x80000,	2,	0x0600004,	8,	true },
			{ "CROM",	false,	"epr-19731.17",	0x3EE6447E,	0x80000,	2,	0x0600006,	8,	true },

			// Banked CROM0
			{ "CROMxx",	false,	"mpr-19661.04",	0x8E3FD241,	0x400000,	2,	0x0000000,	8,	true },
			{ "CROMxx",	false,	"mpr-19660.03",	0xD999C935,	0x400000,	2,	0x0000002,	8,	true },
			{ "CROMxx",	false,	"mpr-19659.02",	0xC47E7002,	0x400000,	2,	0x0000004,	8,	true },
			{ "CROMxx",	false,	"mpr-19658.01",	0xD523235C,	0x400000,	2,	0x0000006,	8,	true },

			// Banked CROM1
			{ "CROMxx",	false,	"mpr-19665.08",	0xF97C78F9,	0x400000,	2,	0x1000000,	8,	true },
			{ "CROMxx",	false,	"mpr-19664.07",	0xB9D11294,	0x400000,	2,	0x1000002,	8,	true },
			{ "CROMxx",	false,	"mpr-19663.06",	0xF6AF1CA4,	0x400000,	2,	0x1000004,	8,	true },
			{ "CROMxx",	false,	"mpr-19662.05",	0x3C700EFF,	0x400000,	2,	0x1000006,	8,	true },

			// Banked CROM2
			{ "CROMxx",	false,	"mpr-19669.12",	0xCDC43C61,	0x400000,	2,	0x2000000,	8,	true },
			{ "CROMxx",	false,	"mpr-19668.11",	0x0B4DD8D5,	0x400000,	2,	0x2000002,	8,	true },
			{ "CROMxx",	false,	"mpr-19667.10",	0xA8676799,	0x400000,	2,	0x2000004,	8,	true },
			{ "CROMxx",	false,	"mpr-19666.09",	0xB53DC97F,	0x400000,	2,	0x2000006,	8,	true },

			// Video ROM
			{ "VROM",	false,	"mpr-19672.26",	0x588C29FD,	0x200000,	2,	0,		32,	false },
			{ "VROM",	false,	"mpr-19673.27",	0x156ABAA9,	0x200000,	2,	2,		32,	false },
			{ "VROM",	false,	"mpr-19674.28",	0xC7B0F98C,	0x200000,	2,	4,		32,	false },
			{ "VROM",	false,	"mpr-19675.29",	0xFF113396,	0x200000,	2,	6,		32,	false },
			{ "VROM",	false,	"mpr-19676.30",	0xFD852EAD,	0x200000,	2,	8,		32,	false },
			{ "VROM",	false,	"mpr-19677.31",	0xC6AC0347,	0x200000,	2,	10,		32,	false },
			{ "VROM",	false,	"mpr-19678.32",	0xB8819CFE,	0x200000,	2,	12,		32,	false },
			{ "VROM",	false,	"mpr-19679.33",	0xE126C3E3,	0x200000,	2,	14,		32,	false },
			{ "VROM",	false,	"mpr-19680.34",	0x00EA5CEF,	0x200000,	2,	16,		32,	false },
			{ "VROM",	false,	"mpr-19681.35",	0xC949325F,	0x200000,	2,	18,		32,	false },
			{ "VROM",	false,	"mpr-19682.36",	0xCE5CA065,	0x200000,	2,	20,		32,	false },
			{ "VROM",	false,	"mpr-19683.37",	0xE5856419,	0x200000,	2,	22,		32,	false },
			{ "VROM",	false,	"mpr-19684.38",	0x56F6EC97,	0x200000,	2,	24,		32,	false },
			{ "VROM",	false,	"mpr-19685.39",	0x42B49304,	0x200000,	2,	26,		32,	false },
			{ "VROM",	false,	"mpr-19686.40",	0x84EED592,	0x200000,	2,	28,		32,	false },
			{ "VROM",	false,	"mpr-19687.41",	0x776CE694,	0x200000,	2,	30,		32,	false },

			// Sound ROMs
			{ "SndProg",	false,	"epr-19692.21",	0xA94F5521,	0x80000,	2,	0,		2,	true },
			{ "Samples",	false,	"mpr-19670.22",	0xBD31CC06,	0x400000,	2,	0x000000,	2,	false },
			{ "Samples",	false,	"mpr-19671.24",	0x8E8526AB,	0x400000,	2,	0x400000,	2,	false },
			{ "DSBProg",	false,	"epr-19612.2",	0x13978FD4,	0x20000,	2,	0,		2,	false },
			{ "DSBMPEG",	false,	"mpr-19603.57",	0xB1B1765F,	0x200000,	2,	0x000000,	2,	false },
			{ "DSBMPEG",	false,	"mpr-19604.58",	0x6AC85B49,	0x200000,	2,	0x200000,	2,	false },
			{ "DSBMPEG",	false,	"mpr-19605.59",	0xBEC891EB,	0x200000,	2,	0x400000,	2,	false },
			{ "DSBMPEG",	false,	"mpr-19606.60",	0xADAD46B2,	0x200000,	2,	0x600000,	2,	false },
			
			// Drive Board ROM
			{ "DriveBd",	true,	"epr-19338a.bin", 0xC9FAC464, 0x10000, 2, 0, 2, false },

			{ NULL, false, NULL, 0, 0, 0, 0, 0, false }
		}
	},

	// Scud Race (Export)
	{
		"scuda",
		"Scud Race (Export)",
		"Sega",
		1996,
		0x15,
		0x200000,	// 2 MB of fixed CROM
		true,		// 48 MB of banked CROM (Mirror)
		0x2000000,	// 32 MB of VROM
		0x800000,	// 8 MB of sample ROMs
		GAME_INPUT_COMMON|GAME_INPUT_VEHICLE|GAME_INPUT_VR|GAME_INPUT_SHIFT4,
		1,			// DSB1 MPEG board
		true,		// drive board

		{
			// Fixed CROM (mirroring behavior here is special and handled manually by CModel3)
			{ "CROM",	false,	"epr-19691.20",	0x83523B89,	0x80000,	2,	0x0600000,	8,	true },
			{ "CROM",	false,	"epr-19690.19",	0x25F007FE,	0x80000,	2,	0x0600002,	8,	true },
			{ "CROM",	false,	"epr-19689.18",	0xCBCE6D62,	0x80000,	2,	0x0600004,	8,	true },
			{ "CROM",	false,	"epr-19688.17",	0xA4C85103,	0x80000,	2,	0x0600006,	8,	true },

			// Banked CROM0
			{ "CROMxx",	false,	"mpr-19661.04",	0x8E3FD241,	0x400000,	2,	0x0000000,	8,	true },
			{ "CROMxx",	false,	"mpr-19660.03",	0xD999C935,	0x400000,	2,	0x0000002,	8,	true },
			{ "CROMxx",	false,	"mpr-19659.02",	0xC47E7002,	0x400000,	2,	0x0000004,	8,	true },
			{ "CROMxx",	false,	"mpr-19658.01",	0xD523235C,	0x400000,	2,	0x0000006,	8,	true },

			// Banked CROM1
			{ "CROMxx",	false,	"mpr-19665.08",	0xF97C78F9,	0x400000,	2,	0x1000000,	8,	true },
			{ "CROMxx",	false,	"mpr-19664.07",	0xB9D11294,	0x400000,	2,	0x1000002,	8,	true },
			{ "CROMxx",	false,	"mpr-19663.06",	0xF6AF1CA4,	0x400000,	2,	0x1000004,	8,	true },
			{ "CROMxx",	false,	"mpr-19662.05",	0x3C700EFF,	0x400000,	2,	0x1000006,	8,	true },

			// Banked CROM2
			{ "CROMxx",	false,	"mpr-19669.12",	0xCDC43C61,	0x400000,	2,	0x2000000,	8,	true },
			{ "CROMxx",	false,	"mpr-19668.11",	0x0B4DD8D5,	0x400000,	2,	0x2000002,	8,	true },
			{ "CROMxx",	false,	"mpr-19667.10",	0xA8676799,	0x400000,	2,	0x2000004,	8,	true },
			{ "CROMxx",	false,	"mpr-19666.09",	0xB53DC97F,	0x400000,	2,	0x2000006,	8,	true },

			// Video ROM
			{ "VROM",	false,	"mpr-19672.26",	0x588C29FD,	0x200000,	2,	0,		32,	false },
			{ "VROM",	false,	"mpr-19673.27",	0x156ABAA9,	0x200000,	2,	2,		32,	false },
			{ "VROM",	false,	"mpr-19674.28",	0xC7B0F98C,	0x200000,	2,	4,		32,	false },
			{ "VROM",	false,	"mpr-19675.29",	0xFF113396,	0x200000,	2,	6,		32,	false },
			{ "VROM",	false,	"mpr-19676.30",	0xFD852EAD,	0x200000,	2,	8,		32,	false },
			{ "VROM",	false,	"mpr-19677.31",	0xC6AC0347,	0x200000,	2,	10,		32,	false },
			{ "VROM",	false,	"mpr-19678.32",	0xB8819CFE,	0x200000,	2,	12,		32,	false },
			{ "VROM",	false,	"mpr-19679.33",	0xE126C3E3,	0x200000,	2,	14,		32,	false },
			{ "VROM",	false,	"mpr-19680.34",	0x00EA5CEF,	0x200000,	2,	16,		32,	false },
			{ "VROM",	false,	"mpr-19681.35",	0xC949325F,	0x200000,	2,	18,		32,	false },
			{ "VROM",	false,	"mpr-19682.36",	0xCE5CA065,	0x200000,	2,	20,		32,	false },
			{ "VROM",	false,	"mpr-19683.37",	0xE5856419,	0x200000,	2,	22,		32,	false },
			{ "VROM",	false,	"mpr-19684.38",	0x56F6EC97,	0x200000,	2,	24,		32,	false },
			{ "VROM",	false,	"mpr-19685.39",	0x42B49304,	0x200000,	2,	26,		32,	false },
			{ "VROM",	false,	"mpr-19686.40",	0x84EED592,	0x200000,	2,	28,		32,	false },
			{ "VROM",	false,	"mpr-19687.41",	0x776CE694,	0x200000,	2,	30,		32,	false },

			// Sound ROMs
			{ "SndProg",	false,	"epr-19692.21",	0xA94F5521,	0x80000,	2,	0,		2,	true },
			{ "Samples",	false,	"mpr-19670.22",	0xBD31CC06,	0x400000,	2,	0x000000,	2,	false },
			{ "Samples",	false,	"mpr-19671.24",	0x8E8526AB,	0x400000,	2,	0x400000,	2,	false },
			{ "DSBProg",	false,	"epr-19612.2",	0x13978FD4,	0x20000,	2,	0,		2,	false },
			{ "DSBMPEG",	false,	"mpr-19603.57",	0xB1B1765F,	0x200000,	2,	0x000000,	2,	false },
			{ "DSBMPEG",	false,	"mpr-19604.58",	0x6AC85B49,	0x200000,	2,	0x200000,	2,	false },
			{ "DSBMPEG",	false,	"mpr-19605.59",	0xBEC891EB,	0x200000,	2,	0x400000,	2,	false },
			{ "DSBMPEG",	false,	"mpr-19606.60",	0xADAD46B2,	0x200000,	2,	0x600000,	2,	false },
			
			// Drive Board ROM
			{ "DriveBd",	true,	"epr-19338a.bin", 0xC9FAC464, 0x10000, 2, 0, 2, false },

			{ NULL, false, NULL, 0, 0, 0, 0, 0, false }
		}
	},

	// Scud Race (Japan)
	{
		"scudj",
		"Scud Race (Japan)",
		"Sega",
		1996,
		0x15,
		0x200000,	// 2 MB of fixed CROM
		true,		// 48 MB of banked CROM (Mirror)
		0x2000000,	// 32 MB of VROM
		0x800000,	// 8 MB of sample ROMs
		GAME_INPUT_COMMON|GAME_INPUT_VEHICLE|GAME_INPUT_VR|GAME_INPUT_SHIFT4,
		1,			// DSB1 MPEG board
		true,		// drive board

		{
			// Fixed CROM (mirroring behavior here is special and handled manually by CModel3)
			{ "CROM",	false,	"epr-19607.20a",	0x24301A12,	0x80000,	2,	0x0600000,	8,	true },
			{ "CROM",	false,	"epr-19608.19a",	0x1426160E,	0x80000,	2,	0x0600002,	8,	true },
			{ "CROM",	false,	"epr-19609.18a",	0xEC418B68,	0x80000,	2,	0x0600004,	8,	true },
			{ "CROM",	false,	"epr-19610.17a",	0x53F5CD94,	0x80000,	2,	0x0600006,	8,	true },

			// Banked CROM0
			{ "CROMxx",	false,	"mpr-19589.4",	0x5482238F,	0x400000,	2,	0x0000000,	8,	true },
			{ "CROMxx",	false,	"mpr-19590.3",	0xA5CD4718,	0x400000,	2,	0x0000002,	8,	true },
			{ "CROMxx",	false,	"mpr-19591.2",	0x48E1AAFF,	0x400000,	2,	0x0000004,	8,	true },
			{ "CROMxx",	false,	"mpr-19592.1",	0xD9003B6F,	0x400000,	2,	0x0000006,	8,	true },

			// Banked CROM1
			{ "CROMxx",	false,	"mpr-19593.8",	0x21E48FF8,	0x400000,	2,	0x1000000,	8,	true },
			{ "CROMxx",	false,	"mpr-19594.7",	0x654C26B0,	0x400000,	2,	0x1000002,	8,	true },
			{ "CROMxx",	false,	"mpr-19595.6",	0xD06FD9D6,	0x400000,	2,	0x1000004,	8,	true },
			{ "CROMxx",	false,	"mpr-19596.5",	0x5672E3F4,	0x400000,	2,	0x1000006,	8,	true },

			// Banked CROM2
			{ "CROMxx",	false,	"mpr-19597.12",	0x4D0FFE60,	0x400000,	2,	0x2000000,	8,	true },
			{ "CROMxx",	false,	"mpr-19598.11",	0xA081592E,	0x400000,	2,	0x2000002,	8,	true },
			{ "CROMxx",	false,	"mpr-19599.10",	0x65C1D33C,	0x400000,	2,	0x2000004,	8,	true },
			{ "CROMxx",	false,	"mpr-19600.9",	0xA25DA127,	0x400000,	2,	0x2000006,	8,	true },

			// Video ROM
			{ "VROM",	false,	"mpr-19574.26",	0x9BE8F314,	0x200000,	2,	0,		32,	false },
			{ "VROM",	false,	"mpr-19573.27",	0x57B61D65,	0x200000,	2,	2,		32,	false },
			{ "VROM",	false,	"mpr-19576.28",	0x85F9B587,	0x200000,	2,	4,		32,	false },
			{ "VROM",	false,	"mpr-19575.29",	0xDAB11C34,	0x200000,	2,	6,		32,	false },
			{ "VROM",	false,	"mpr-19578.30",	0xAE882C42,	0x200000,	2,	8,		32,	false },
			{ "VROM",	false,	"mpr-19577.31",	0x36A1FE5D,	0x200000,	2,	10,		32,	false },
			{ "VROM",	false,	"mpr-19580.32",	0x62503CEE,	0x200000,	2,	12,		32,	false },
			{ "VROM",	false,	"mpr-19579.33",	0xAF9698D0,	0x200000,	2,	14,		32,	false },
			{ "VROM",	false,	"mpr-19582.34",	0xC8B9CF1A,	0x200000,	2,	16,		32,	false },
			{ "VROM",	false,	"mpr-19581.35",	0x8863C2D7,	0x200000,	2,	18,		32,	false },
			{ "VROM",	false,	"mpr-19584.36",	0x256B056C,	0x200000,	2,	20,		32,	false },
			{ "VROM",	false,	"mpr-19583.37",	0xC22CB5AA,	0x200000,	2,	22,		32,	false },
			{ "VROM",	false,	"mpr-19586.38",	0xAC37163E,	0x200000,	2,	24,		32,	false },
			{ "VROM",	false,	"mpr-19585.39",	0xE2598012,	0x200000,	2,	26,		32,	false },
			{ "VROM",	false,	"mpr-19588.40",	0x42E20AE9,	0x200000,	2,	28,		32,	false },
			{ "VROM",	false,	"mpr-19587.41",	0xC288C910,	0x200000,	2,	30,		32,	false },

			// Sound ROMs
			{ "SndProg",	false,	"epr-19611a.21",	0x9D4A34F6,	0x40000,	2,	0,		2,	true },
			{ "Samples",	false,	"mpr-19601.22",	0xBA350FCC,	0x400000,	2,	0x000000,	2,	false },
			{ "Samples",	false,	"mpr-19602.24",	0xA92231C1,	0x400000,	2,	0x400000,	2,	false },
			{ "DSBProg",	false,	"epr-19612.2",	0x13978FD4,	0x20000,	2,	0,		2,	false },
			{ "DSBMPEG",	false,	"mpr-19603.57",	0xB1B1765F,	0x200000,	2,	0x000000,	2,	false },
			{ "DSBMPEG",	false,	"mpr-19604.58",	0x6AC85B49,	0x200000,	2,	0x200000,	2,	false },
			{ "DSBMPEG",	false,	"mpr-19605.59",	0xBEC891EB,	0x200000,	2,	0x400000,	2,	false },
			{ "DSBMPEG",	false,	"mpr-19606.60",	0xADAD46B2,	0x200000,	2,	0x600000,	2,	false },
			
			// Drive Board ROM
			{ "DriveBd",	true,	"epr-19338a.bin", 0xC9FAC464, 0x10000, 2, 0, 2, false },

			{ NULL, false, NULL, 0, 0, 0, 0, 0, false }
		}
	},

	// Scud Race Plus (Revision A)
	{
		"scudp",
		"Scud Race Plus",
		"Sega",
		1997,
		0x15,
		0x200000,	// 2 MB of fixed CROM
		true,		// 64 MB of banked CROM (Mirror)
		0x2000000,	// 32 MB of VROM
		0x800000,	// 8 MB of sample ROMs
		GAME_INPUT_COMMON|GAME_INPUT_VEHICLE|GAME_INPUT_VR|GAME_INPUT_SHIFT4,
		1,			// DSB1 MPEG board
		true,		// drive board

		{
			// Fixed CROM (mirroring behavior here is special and handled manually by CModel3)
			{ "CROM",	false,	"epr-20095a.20",	0x58C7E393,	0x80000,	2,	0x0600000,	8,	true },
			{ "CROM",	false,	"epr-20094a.19",	0xDBF17A43,	0x80000,	2,	0x0600002,	8,	true },
			{ "CROM",	false,	"epr-20093a.18",	0x4ED2E35D,	0x80000,	2,	0x0600004,	8,	true },
			{ "CROM",	false,	"epr-20092a.17",	0xA94EC57E,	0x80000,	2,	0x0600006,	8,	true },

			// Banked CROM0
			{ "CROMxx",	false,	"mpr-19661.04",	0x8E3FD241,	0x400000,	2,	0x0000000,	8,	true },
			{ "CROMxx",	false,	"mpr-19660.03",	0xD999C935,	0x400000,	2,	0x0000002,	8,	true },
			{ "CROMxx",	false,	"mpr-19659.02",	0xC47E7002,	0x400000,	2,	0x0000004,	8,	true },
			{ "CROMxx",	false,	"mpr-19658.01",	0xD523235C,	0x400000,	2,	0x0000006,	8,	true },

			// Banked CROM1
			{ "CROMxx",	false,	"mpr-19665.08",	0xF97C78F9,	0x400000,	2,	0x1000000,	8,	true },
			{ "CROMxx",	false,	"mpr-19664.07",	0xB9D11294,	0x400000,	2,	0x1000002,	8,	true },
			{ "CROMxx",	false,	"mpr-19663.06",	0xF6AF1CA4,	0x400000,	2,	0x1000004,	8,	true },
			{ "CROMxx",	false,	"mpr-19662.05",	0x3C700EFF,	0x400000,	2,	0x1000006,	8,	true },

			// Banked CROM2
			{ "CROMxx",	false,	"mpr-19669.12",	0xCDC43C61,	0x400000,	2,	0x2000000,	8,	true },
			{ "CROMxx",	false,	"mpr-19668.11",	0x0B4DD8D5,	0x400000,	2,	0x2000002,	8,	true },
			{ "CROMxx",	false,	"mpr-19667.10",	0xA8676799,	0x400000,	2,	0x2000004,	8,	true },
			{ "CROMxx",	false,	"mpr-19666.09",	0xB53DC97F,	0x400000,	2,	0x2000006,	8,	true },

			// Banked CROM3
			{ "CROMxx",	false,	"mpr-20100.16",	0xC99E2C01,	0x400000,	2,	0x3000000,	8,	true },
			{ "CROMxx",	false,	"mpr-20099.15",	0xFC9BD7D9,	0x400000,	2,	0x3000002,	8,	true },
			{ "CROMxx",	false,	"mpr-20098.14",	0x8355FA41,	0x400000,	2,	0x3000004,	8,	true },
			{ "CROMxx",	false,	"mpr-20097.13",	0x269A9DBE,	0x400000,	2,	0x3000006,	8,	true },

			// Video ROM
			{ "VROM",	false,	"mpr-19672.26",	0x588C29FD,	0x200000,	2,	0,		32,	false },
			{ "VROM",	false,	"mpr-19673.27",	0x156ABAA9,	0x200000,	2,	2,		32,	false },
			{ "VROM",	false,	"mpr-19674.28",	0xC7B0F98C,	0x200000,	2,	4,		32,	false },
			{ "VROM",	false,	"mpr-19675.29",	0xFF113396,	0x200000,	2,	6,		32,	false },
			{ "VROM",	false,	"mpr-19676.30",	0xFD852EAD,	0x200000,	2,	8,		32,	false },
			{ "VROM",	false,	"mpr-19677.31",	0xC6AC0347,	0x200000,	2,	10,		32,	false },
			{ "VROM",	false,	"mpr-19678.32",	0xB8819CFE,	0x200000,	2,	12,		32,	false },
			{ "VROM",	false,	"mpr-19679.33",	0xE126C3E3,	0x200000,	2,	14,		32,	false },
			{ "VROM",	false,	"mpr-19680.34",	0x00EA5CEF,	0x200000,	2,	16,		32,	false },
			{ "VROM",	false,	"mpr-19681.35",	0xC949325F,	0x200000,	2,	18,		32,	false },
			{ "VROM",	false,	"mpr-19682.36",	0xCE5CA065,	0x200000,	2,	20,		32,	false },
			{ "VROM",	false,	"mpr-19683.37",	0xE5856419,	0x200000,	2,	22,		32,	false },
			{ "VROM",	false,	"mpr-19684.38",	0x56F6EC97,	0x200000,	2,	24,		32,	false },
			{ "VROM",	false,	"mpr-19685.39",	0x42B49304,	0x200000,	2,	26,		32,	false },
			{ "VROM",	false,	"mpr-19686.40",	0x84EED592,	0x200000,	2,	28,		32,	false },
			{ "VROM",	false,	"mpr-19687.41",	0x776CE694,	0x200000,	2,	30,		32,	false },

			// Sound ROMs
			{ "SndProg",	false,	"epr-20096a.21",	0x0FEF288B,	0x80000,	2,	0,		2,	true },
			{ "Samples",	false,	"mpr-19670.22",	0xBD31CC06,	0x400000,	2,	0x000000,	2,	false },
			{ "Samples",	false,	"mpr-20101.24",	0x66D1E31F,	0x400000,	2,	0x400000,	2,	false },
			{ "DSBProg",	false,	"epr-19612.2",	0x13978FD4,	0x20000,	2,	0,		2,	false },
			{ "DSBMPEG",	false,	"mpr-19603.57",	0xB1B1765F,	0x200000,	2,	0x000000,	2,	false },
			{ "DSBMPEG",	false,	"mpr-19604.58",	0x6AC85B49,	0x200000,	2,	0x200000,	2,	false },
			{ "DSBMPEG",	false,	"mpr-19605.59",	0xBEC891EB,	0x200000,	2,	0x400000,	2,	false },
			{ "DSBMPEG",	false,	"mpr-19606.60",	0xADAD46B2,	0x200000,	2,	0x600000,	2,	false },
			
			// Drive Board ROM
			{ "DriveBd",	true,	"epr-19338a.bin", 0xC9FAC464, 0x10000, 2, 0, 2, false },

			{ NULL, false, NULL, 0, 0, 0, 0, 0, false }
		}
	},

	// Ski Champ
	{
		"skichamp",
		"Ski Champ",
		"Sega",
		1998,
		0x20,
		0x800000,	// 8 MB of fixed CROM
		true,		// 64 MB of banked CROM (Mirror)
		0x4000000,	// 64 MB of VROM
		0x800000,	// 8 MB of sample ROMs
		GAME_INPUT_COMMON|GAME_INPUT_VEHICLE,
		0,			// no MPEG board
		false,		// no drive board

		{
			// Fixed CROM
			{ "CROM",	false,	"epr20355.20",	0x7A784E67,	0x200000,	2,	0x0000000,	8,	true },
			{ "CROM",	false,	"epr20354.18",	0xACA62BF8,	0x200000,	2,	0x0000002,	8,	true },
			{ "CROM",	false,	"epr20353.19",	0xBADF5F04,	0x200000,	2,	0x0000004,	8,	true },
			{ "CROM",	false,	"epr20352.17",	0xC92C2545,	0x200000,	2,	0x0000006,	8,	true },

			// Banked CROM0
			{ "CROMxx",	false,	"mpr20321.4",	0x698A97EE,	0x400000,	2,	0x0000000,	8,	true },
			{ "CROMxx",	false,	"mpr20320.3",	0xEDC9A9E5,	0x400000,	2,	0x0000002,	8,	true },
			{ "CROMxx",	false,	"mpr20319.2",	0x228047F3,	0x400000,	2,	0x0000004,	8,	true },
			{ "CROMxx",	false,	"mpr20318.1",	0xB0CAD2C8,	0x400000,	2,	0x0000006,	8,	true },

			// Banked CROM1
			{ "CROMxx",	false,	"mpr20325.8",	0xCB0EB133,	0x400000,	2,	0x1000000,	8,	true },
			{ "CROMxx",	false,	"mpr20324.7",	0x8F5848D0,	0x400000,	2,	0x1000002,	8,	true },
			{ "CROMxx",	false,	"mpr20323.6",	0x075DE2AE,	0x400000,	2,	0x1000004,	8,	true },
			{ "CROMxx",	false,	"mpr20322.5",	0x2F69B205,	0x400000,	2,	0x1000006,	8,	true },

			// Banked CROM2
			{ "CROMxx",	false,	"mpr20329.12",	0x0807EA33,	0x400000,	2,	0x2000000,	8,	true },
			{ "CROMxx",	false,	"mpr20328.11",	0x5FA5E9F5,	0x400000,	2,	0x2000002,	8,	true },
			{ "CROMxx",	false,	"mpr20327.10",	0xF55F51B2,	0x400000,	2,	0x2000004,	8,	true },
			{ "CROMxx",	false,	"mpr20326.9",	0xB63E1CB4,	0x400000,	2,	0x2000006,	8,	true },

			// Banked CROM3
			{ "CROMxx",	false,	"mpr20333.16",	0x76B8E0FA,	0x400000,	2,	0x3000000,	8,	true },
			{ "CROMxx",	false,	"mpr20332.15",	0x500DB1EE,	0x400000,	2,	0x3000002,	8,	true },
			{ "CROMxx",	false,	"mpr20331.14",	0xC4C45FB1,	0x400000,	2,	0x3000004,	8,	true },
			{ "CROMxx",	false,	"mpr20330.13",	0xFBC7BBD5,	0x400000,	2,	0x3000006,	8,	true },

			// Video ROM
			{ "VROM",	false,	"mpr20336.26",	0x261E3D39,	0x400000,	2,	0,		32,	false },
			{ "VROM",	false,	"mpr20337.27",	0x2C7E9EB8,	0x400000,	2,	2,		32,	false },
			{ "VROM",	false,	"mpr20338.28",	0x0AA626DF,	0x400000,	2,	4,		32,	false },
			{ "VROM",	false,	"mpr20339.29",	0x7AF05417,	0x400000,	2,	6,		32,	false },
			{ "VROM",	false,	"mpr20340.30",	0x82EF4A21,	0x400000,	2,	8,		32,	false },
			{ "VROM",	false,	"mpr20341.31",	0x9373096E,	0x400000,	2,	10,		32,	false },
			{ "VROM",	false,	"mpr20342.32",	0xEF98CD37,	0x400000,	2,	12,		32,	false },
			{ "VROM",	false,	"mpr20343.33",	0x9825A46B,	0x400000,	2,	14,		32,	false },
			{ "VROM",	false,	"mpr20344.34",	0xACBBCD68,	0x400000,	2,	16,		32,	false },
			{ "VROM",	false,	"mpr20345.35",	0x431E7585,	0x400000,	2,	18,		32,	false },
			{ "VROM",	false,	"mpr20346.36",	0x4F87F2D2,	0x400000,	2,	20,		32,	false },
			{ "VROM",	false,	"mpr20347.37",	0x389A2D98,	0x400000,	2,	22,		32,	false },
			{ "VROM",	false,	"mpr20348.38",	0x8BE8D4D2,	0x400000,	2,	24,		32,	false },
			{ "VROM",	false,	"mpr20349.39",	0xA3240428,	0x400000,	2,	26,		32,	false },
			{ "VROM",	false,	"mpr20350.40",	0xC48F9ACE,	0x400000,	2,	28,		32,	false },
			{ "VROM",	false,	"mpr20351.41",	0x1FBD3E10,	0x400000,	2,	30,		32,	false },

			// Sound ROMs
			{ "SndProg",	false,	"epr20356.21",	0x4E4015D0,	0x80000,	2,	0,		2,	true },
			{ "Samples",	false,	"mpr20334.22",	0xDE1D67CD,	0x400000,	2,	0x000000,	2,	false },
			{ "Samples",	false,	"mpr20335.24",	0x7300D0A2,	0x400000,	2,	0x400000,	2,	false },

			{ NULL, false, NULL, 0, 0, 0, 0, 0, false }
		}
	},

	// Spikeout Final Edition (disabled because this is a bad dump according to MAME)
	{
		"spikeofe",
		"Spikeout Final Edition",
		"Sega",
		1999,
		0x21,
		0x800000,	// 8 MB of fixed CROM
		false,		// 112 MB of banked CROM (do not mirror)
		0x4000000,	// 64 MB of VROM
		0x1000000,	// 16 MB of sample ROMs
		GAME_INPUT_COMMON|GAME_INPUT_JOYSTICK1|GAME_INPUT_SPIKEOUT,
		2,			// DSB2 MPEG board
		false,		// no drive board

		{
			// Fixed CROM
			{ "CROM",	false,	"epr21656.20",	0xBD2AAF64,	0x200000,	2,	0x0000000,	8,	true },
			{ "CROM",	false,	"epr21655.19",	0x68A9E417,	0x200000,	2,	0x0000002,	8,	true },
			{ "CROM",	false,	"epr21654.18",	0x5BE245A3,	0x200000,	2,	0x0000004,	8,	true },
			{ "CROM",	false,	"epr21653.17",	0xF4BD9C3C,	0x200000,	2,	0x0000006,	8,	true },

			// Banked CROM0
			{ "CROMxx",	false,	"mpr21616.4",	0x2900BDD8,	0x800000,	2,	0x0000000,	8,	true },
			{ "CROMxx",	false,	"mpr21615.3",	0x7727A6FC,	0x800000,	2,	0x0000002,	8,	true },
			{ "CROMxx",	false,	"mpr21614.2",	0xE21D619B,	0x800000,	2,	0x0000004,	8,	true },
			{ "CROMxx",	false,	"mpr21613.1",	0xD039E608,	0x800000,	2,	0x0000006,	8,	true },

			// Banked CROM1
			{ "CROMxx",	false,	"mpr21620.8",	0x476F027F,	0x800000,	2,	0x2000000,	8,	true },
			{ "CROMxx",	false,	"mpr21619.7",	0xE1076F47,	0x800000,	2,	0x2000002,	8,	true },
			{ "CROMxx",	false,	"mpr21618.6",	0x633530FA,	0x800000,	2,	0x2000004,	8,	true },
			{ "CROMxx",	false,	"mpr21617.5",	0xA08C6790,	0x800000,	2,	0x2000006,	8,	true },

			// Banked CROM2
			{ "CROMxx",	false,	"mpr21624.12",	0xA158B7DA,	0x800000,	2,	0x4000000,	8,	true },
			{ "CROMxx",	false,	"mpr21623.11",	0xD9301674,	0x800000,	2,	0x4000002,	8,	true },
			{ "CROMxx",	false,	"mpr21622.10",	0x5F5A1563,	0x800000,	2,	0x4000004,	8,	true },
			{ "CROMxx",	false,	"mpr21621.9",	0x551A444D,	0x800000,	2,	0x4000006,	8,	true },

			// Banked CROM3
			{ "CROMxx",	false,	"mpr21628.16",	0xDE3866EA,	0x400000,	2,	0x6000000,	8,	true },
			{ "CROMxx",	false,	"mpr21627.15",	0xEFE94608,	0x400000,	2,	0x6000002,	8,	true },
			{ "CROMxx",	false,	"mpr21626.14",	0x1861652E,	0x400000,	2,	0x6000004,	8,	true },
			{ "CROMxx",	false,	"mpr21625.13",	0x72A34707,	0x400000,	2,	0x6000006,	8,	true },

			// Video ROM
			{ "VROM",	false,	"mpr21633.26",	0x735FB67D,	0x400000,	2,	0,		32,	false },
			{ "VROM",	false,	"mpr21634.27",	0x876E6788,	0x400000,	2,	2,		32,	false },
			{ "VROM",	false,	"mpr21635.28",	0x093534A8,	0x400000,	2,	4,		32,	false },
			{ "VROM",	false,	"mpr21636.29",	0x2433F21C,	0x400000,	2,	6,		32,	false },
			{ "VROM",	false,	"mpr21637.30",	0xEDB8F2B8,	0x400000,	2,	8,		32,	false },
			{ "VROM",	false,	"mpr21638.31",	0x3773A215,	0x400000,	2,	10,		32,	false },
			{ "VROM",	false,	"mpr21639.32",	0x313D1872,	0x400000,	2,	12,		32,	false },
			{ "VROM",	false,	"mpr21640.33",	0x271366BE,	0x400000,	2,	14,		32,	false },
			{ "VROM",	false,	"mpr21641.34",	0x782147E4,	0x400000,	2,	16,		32,	false },
			{ "VROM",	false,	"mpr21642.35",	0x844732C9,	0x400000,	2,	18,		32,	false },
			{ "VROM",	false,	"mpr21643.36",	0x9E922E9D,	0x400000,	2,	20,		32,	false },
			{ "VROM",	false,	"mpr21644.37",	0x617AA65A,	0x400000,	2,	22,		32,	false },
			{ "VROM",	false,	"mpr21645.38",	0x71396F52,	0x400000,	2,	24,		32,	false },
			{ "VROM",	false,	"mpr21646.39",	0x90FD9C87,	0x400000,	2,	26,		32,	false },
			{ "VROM",	false,	"mpr21647.40",	0xCF87991F,	0x400000,	2,	28,		32,	false },
			{ "VROM",	false,	"mpr21648.41",	0x30F974A1,	0x400000,	2,	30,		32,	false },

			// Sound ROMs
			{ "SndProg",	false,	"epr21657.21",	0x7242E8FD,	0x80000,	2,	0,		2,	true },
			{ "Samples",	false,	"mpr21629.22",	0xBC9701C4,	0x400000,	2,	0x000000,	2,	false },
			{ "Samples",	false,	"mpr21630.24",	0x9F2DEADD,	0x400000,	2,	0x400000,	2,	false },
			{ "Samples",	false,	"mpr21631.23",	0x299036C5,	0x400000,	2,	0x800000,	2,	false },
			{ "Samples",	false,	"mpr21632.25",	0xFF162F0D,	0x400000,	2,	0xC00000,	2,	false },
			{ "DSBProg",	false,	"epr21658.ic2",	0x50BAD8CB,	0x20000,	2,	0,		2,	true },
			{ "DSBMPEG",	false,	"mpr21649.ic18",	0xDAC87F47,	0x400000,	2,	0x000000,	2,	false },
			{ "DSBMPEG",	false,	"mpr21650.ic20",	0x86D90123,	0x400000,	2,	0x400000,	2,	false },
			{ "DSBMPEG",	false,	"mpr21651.ic22",	0x81715565,	0x400000,	2,	0x800000,	2,	false },
			{ "DSBMPEG",	false,	"mpr21652.ic24",	0xE7C8C9BF,	0x400000,	2,	0xC00000,	2,	false },

			{ NULL, false, NULL, 0, 0, 0, 0, 0, false }
		}
	},

	// Spikeout (Revision C)
	{
		"spikeout",
		"Spikeout",
		"Sega",
		1998,
		0x21,
		0x800000,	// 8 MB of fixed CROM
		false,	// 96 MB of banked CROM (do not mirror)
		0x4000000,	// 64 MB of VROM
		0x1000000,	// 16 MB of sample ROMs
		GAME_INPUT_COMMON|GAME_INPUT_JOYSTICK1|GAME_INPUT_SPIKEOUT,
		2,			// DSB2 MPEG board
		false,		// no drive board

		{
			// Fixed CROM
			{ "CROM",	false, 	"epr21217c.20",	0xEA8C30CE,	0x200000,	2,	0x0000000,	8,	true },
			{ "CROM",	false,	"epr21216c.19",	0x867D3A0F,	0x200000,	2,	0x0000002,	8,	true },
			{ "CROM",	false,	"epr21215c.18",	0xE2878221,	0x200000,	2,	0x0000004,	8,	true },
			{ "CROM",	false,	"epr21214c.17",	0x8DC0A85C,	0x200000,	2,	0x0000006,	8,	true },

			// Banked CROM0
			{ "CROMxx",	false,	"mpr21137.4",	0x3572D417,	0x800000,	2,	0x0000000,	8,	true },
			{ "CROMxx",	false,	"mpr21136.3",	0xB730FE50,	0x800000,	2,	0x0000002,	8,	true },
			{ "CROMxx",	false,	"mpr21135.2",	0xF3FA7C50,	0x800000,	2,	0x0000004,	8,	true },
			{ "CROMxx",	false,	"mpr21134.1",	0x65399935,	0x800000,	2,	0x0000006,	8,	true },

			// Banked CROM1
			{ "CROMxx",	false,	"mpr21141.8",	0x1D0763CB,	0x800000,	2,	0x2000000,	8,	true },
			{ "CROMxx",	false,	"mpr21140.7",	0x1390746D,	0x800000,	2,	0x2000002,	8,	true },
			{ "CROMxx",	false,	"mpr21139.6",	0x06D441F5,	0x800000,	2,	0x2000004,	8,	true },
			{ "CROMxx",	false,	"mpr21138.5",	0xA9A2DE2C,	0x800000,	2,	0x2000006,	8,	true },

			// Banked CROM2
			{ "CROMxx",	false,	"mpr21145.12",	0x0E6A3AE3,	0x400000,	2,	0x4000000,	8,	true },
			{ "CROMxx",	false,	"mpr21144.11",	0xD93D778C,	0x400000,	2,	0x4000002,	8,	true },
			{ "CROMxx",	false,	"mpr21143.10",	0xDDCADA10,	0x400000,	2,	0x4000004,	8,	true },
			{ "CROMxx",	false,	"mpr21142.9",	0xDA35CD51,	0x400000,	2,	0x4000006,	8,	true },

			// Banked CROM3 (note: appears at offset 0x6000000 rather than 0x5000000 as expected)
			{ "CROMxx",	false,	"mpr21149.16",	0x9E4EBE58,	0x400000,	2,	0x6000000,	8,	true },
			{ "CROMxx",	false,	"mpr21148.15",	0x56D980AD,	0x400000,	2,	0x6000002,	8,	true },
			{ "CROMxx",	false,	"mpr21147.14",	0xA1F2B73F,	0x400000,	2,	0x6000004,	8,	true },
			{ "CROMxx",	false,	"mpr21146.13",	0x85F55311,	0x400000,	2,	0x6000006,	8,	true },

			// Video ROM
			{ "VROM",	false,	"mpr21154.26",	0x3B76F8E8,	0x400000,	2,	0,		32,	false },
			{ "VROM",	false,	"mpr21155.27",	0xACA19901,	0x400000,	2,	2,		32,	false },
			{ "VROM",	false,	"mpr21156.28",	0x5C9DF226,	0x400000,	2,	4,		32,	false },
			{ "VROM",	false,	"mpr21157.29",	0xF6FB1279,	0x400000,	2,	6,		32,	false },
			{ "VROM",	false,	"mpr21158.30",	0x61707554,	0x400000,	2,	8,		32,	false },
			{ "VROM",	false,	"mpr21159.31",	0xFCC791F5,	0x400000,	2,	10,		32,	false },
			{ "VROM",	false,	"mpr21160.32",	0xB40A38D3,	0x400000,	2,	12,		32,	false },
			{ "VROM",	false,	"mpr21161.33",	0x559063F0,	0x400000,	2,	14,		32,	false },
			{ "VROM",	false,	"mpr21162.34",	0xACC4B2E4,	0x400000,	2,	16,		32,	false },
			{ "VROM",	false,	"mpr21163.35",	0x653C54C7,	0x400000,	2,	18,		32,	false },
			{ "VROM",	false,	"mpr21164.36",	0x902FD1E0,	0x400000,	2,	20,		32,	false },
			{ "VROM",	false,	"mpr21165.37",	0x50B3BE05,	0x400000,	2,	22,		32,	false },
			{ "VROM",	false,	"mpr21166.38",	0x8F87A782,	0x400000,	2,	24,		32,	false },
			{ "VROM",	false,	"mpr21167.39",	0x0F3994D0,	0x400000,	2,	26,		32,	false },
			{ "VROM",	false,	"mpr21168.40",	0xC58BE980,	0x400000,	2,	28,		32,	false },
			{ "VROM",	false,	"mpr21169.41",	0xAA3B2CC0,	0x400000,	2,	30,		32,	false },

			// Sound ROMs
			{ "SndProg",	false,	"epr21218.21",	0x5821001A,	0x80000,	2,	0,		2,	true },
			{ "Samples",	false,	"mpr21150.22",	0x125201CE,	0x400000,	2,	0x000000,	2,	false },
			{ "Samples",	false,	"mpr21152.24",	0x0AFDEE87,	0x400000,	2,	0x400000,	2,	false },
			{ "Samples",	false,	"mpr21151.23",	0x599527B9,	0x400000,	2,	0x800000,	2,	false },
			{ "Samples",	false,	"mpr21153.25",	0x4155F307,	0x400000,	2,	0xC00000,	2,	false },
			{ "DSBProg",	false,	"epr21219.ic2",	0x4E042B21,	0x20000,	2,	0,		2,	true },
			{ "DSBMPEG",	false,	"mpr21170.ic18",	0xF51F7CE3,	0x400000,	2,	0x000000,	2,	false },
			{ "DSBMPEG",	false,	"mpr21171.ic20",	0x8D3BD5B6,	0x400000,	2,	0x400000,	2,	false },
			{ "DSBMPEG",	false,	"mpr21172.ic22",	0xBE221E27,	0x400000,	2,	0x800000,	2,	false },
			{ "DSBMPEG",	false,	"mpr21173.ic24",	0xCA7226D6,	0x400000,	2,	0xC00000,	2,	false },

			{ NULL, false, NULL, 0, 0, 0, 0, 0, false }
		}
	},

	// Sega Rally 2
	{
		"srally2",
		"Sega Rally 2",
		"Sega",
		1998,
		0x20,
		0x800000,	// 8 MB of fixed CROM
		true,		// 48 MB of banked CROM (Mirror)
		0x4000000,	// 64 MB of VROM
		0x800000,	// 8 MB of sample ROMs
		GAME_INPUT_COMMON|GAME_INPUT_VEHICLE|GAME_INPUT_RALLY|GAME_INPUT_SHIFT4,
		2,			// DSB2 MPEG board
		true,		// drive board

		{
			// Fixed CROM
			{ "CROM",	false,	"epr-20635.20",	0x7937473F,	0x200000,	2,	0x0000000,	8,	true },
			{ "CROM",	false,	"epr-20634.19",	0x45A09245,	0x200000,	2,	0x0000002,	8,	true },
			{ "CROM",	false,	"epr-20633.18",	0xF5A24F24,	0x200000,	2,	0x0000004,	8,	true },
			{ "CROM",	false,	"epr-20632.17",	0x6829A801,	0x200000,	2,	0x0000006,	8,	true },

			// Banked CROM0
			{ "CROMxx",	false,	"mpr-20605.4",	0x00513401,	0x400000,	2,	0x0000000,	8,	true },
			{ "CROMxx",	false,	"mpr-20605.3",	0x99C5F396,	0x400000,	2,	0x0000002,	8,	true },
			{ "CROMxx",	false,	"mpr-20603.2",	0xAD0D8EB8,	0x400000,	2,	0x0000004,	8,	true },
			{ "CROMxx",	false,	"mpr-20602.1",	0x60CFA72A,	0x400000,	2,	0x0000006,	8,	true },

			// Banked CROM1
			{ "CROMxx",	false,	"mpr-20609.8",	0xC03CC0E5,	0x400000,	2,	0x1000000,	8,	true },
			{ "CROMxx",	false,	"mpr-20608.7",	0x0C9B0571,	0x400000,	2,	0x1000002,	8,	true },
			{ "CROMxx",	false,	"mpr-20607.6",	0x6DA85AA3,	0x400000,	2,	0x1000004,	8,	true },
			{ "CROMxx",	false,	"mpr-20606.5",	0x072498FD,	0x400000,	2,	0x1000006,	8,	true },

			// Banked CROM2
			{ "CROMxx",	false,	"mpr-20613.12",	0x2938C0D9,	0x400000,	2,	0x2000000,	8,	true },
			{ "CROMxx",	false,	"mpr-20612.11",	0x721A44B6,	0x400000,	2,	0x2000002,	8,	true },
			{ "CROMxx",	false,	"mpr-20611.10",	0x5D9F8BA2,	0x400000,	2,	0x2000004,	8,	true },
			{ "CROMxx",	false,	"mpr-20610.9",	0xB6E0FF4E,	0x400000,	2,	0x2000006,	8,	true },

			// Video ROM
			{ "VROM",	false,	"mpr-20616.26",	0xE11DCF8B,	0x400000,	2,	0,		32,	false },
			{ "VROM",	false,	"mpr-20617.27",	0x96ACEF3F,	0x400000,	2,	2,		32,	false },
			{ "VROM",	false,	"mpr-20618.28",	0x6C281281,	0x400000,	2,	4,		32,	false },
			{ "VROM",	false,	"mpr-20619.29",	0x0FA65819,	0x400000,	2,	6,		32,	false },
			{ "VROM",	false,	"mpr-20620.30",	0xEE79585F,	0x400000,	2,	8,		32,	false },
			{ "VROM",	false,	"mpr-20621.31",	0x3A99148F,	0x400000,	2,	10,		32,	false },
			{ "VROM",	false,	"mpr-20622.32",	0x0618F056,	0x400000,	2,	12,		32,	false },
			{ "VROM",	false,	"mpr-20623.33",	0xCCF31B85,	0x400000,	2,	14,		32,	false },
			{ "VROM",	false,	"mpr-20624.34",	0x90F30936,	0x400000,	2,	16,		32,	false },
			{ "VROM",	false,	"mpr-20625.35",	0x04F804FA,	0x400000,	2,	18,		32,	false },
			{ "VROM",	false,	"mpr-20626.36",	0x2D6C97D6,	0x400000,	2,	20,		32,	false },
			{ "VROM",	false,	"mpr-20627.37",	0xA14EE871,	0x400000,	2,	22,		32,	false },
			{ "VROM",	false,	"mpr-20628.38",	0xBBA829A3,	0x400000,	2,	24,		32,	false },
			{ "VROM",	false,	"mpr-20629.39",	0xEAD2EB31,	0x400000,	2,	26,		32,	false },
			{ "VROM",	false,	"mpr-20630.40",	0xCC5881B8,	0x400000,	2,	28,		32,	false },
			{ "VROM",	false,	"mpr-20631.41",	0x5CB69FFD,	0x400000,	2,	30,		32,	false },

			// Sound ROMs
			{ "SndProg",	false,	"epr-20636.21",	0x7139EBF8,	0x80000,	2,	0,		2,	true },
			{ "Samples",	false,	"mpr-20614.22",	0xA3930E4A,	0x400000,	2,	0x000000,	2,	false },
			{ "Samples",	false,	"mpr-20615.24",	0x62E8A94A,	0x400000,	2,	0x400000,	2,	false },
			{ "DSBProg",	false,	"epr-20641.2",	0xC9B82035,	0x20000,	2,	0,		2,	true },
			{ "DSBMPEG",	false,	"mpr-20637.57",	0xD66E8A02,	0x400000,	2,	0x000000,	2,	false },
			{ "DSBMPEG",	false,	"mpr-20638.58",	0xD1513382,	0x400000,	2,	0x400000,	2,	false },
			{ "DSBMPEG",	false,	"mpr-20639.59",	0xF6603B7B,	0x400000,	2,	0x800000,	2,	false },
			{ "DSBMPEG",	false,	"mpr-20640.60",	0x9EEA07B7,	0x400000,	2,	0xC00000,	2,	false },
			
			// Drive Board ROM
			{ "DriveBd",	true,	"epr-20512.bin", 0xCF64350D, 0x10000, 2, 0, 2, false },

			{ NULL, false, NULL, 0, 0, 0, 0, 0, false }
		}
	},

	// Sega Rally 2 DX
	{
		"srally2x",
		"Sega Rally 2 DX",
		"Sega",
		1998,
		0x20,
		0x800000,	// 8 MB of fixed CROM
		true,		// 48 MB of banked CROM (Mirror)
		0x4000000,	// 64 MB of VROM
		0x800000,	// 8 MB of sample ROMs
		GAME_INPUT_COMMON|GAME_INPUT_VEHICLE|GAME_INPUT_RALLY|GAME_INPUT_SHIFT4,
		0,			// no MPEG board
		true,		// drive board

		{
			// Fixed CROM
			{ "CROM",	false,	"epr-20505.20",	0xC24A5097,	0x200000,	2,	0x0000000,	8,	true },
			{ "CROM",	false,	"epr-20504.19",	0x30BBC46D,	0x200000,	2,	0x0000002,	8,	true },
			{ "CROM",	false,	"epr-20503.18",	0x6E238B3D,	0x200000,	2,	0x0000004,	8,	true },
			{ "CROM",	false,	"epr-20502.17",	0xAF16846D,	0x200000,	2,	0x0000006,	8,	true },

			// Banked CROM0
			{ "CROMxx",	false,	"mpr-20475.4",	0xD0F059EE,	0x400000,	2,	0x0000000,	8,	true },
			{ "CROMxx",	false,	"mpr-20474.3",	0x66CB4C8E,	0x400000,	2,	0x0000002,	8,	true },
			{ "CROMxx",	false,	"mpr-20473.2",	0xDD8E3131,	0x400000,	2,	0x0000004,	8,	true },
			{ "CROMxx",	false,	"mpr-20472.1",	0xDB8D6A00,	0x400000,	2,	0x0000006,	8,	true },

			// Banked CROM1
			{ "CROMxx",	false,	"mpr-20479.8",	0x82EC5488,	0x400000,	2,	0x1000000,	8,	true },
			{ "CROMxx",	false,	"mpr-20478.7",	0x5DFD59F7,	0x400000,	2,	0x1000002,	8,	true },
			{ "CROMxx",	false,	"mpr-20477.6",	0x0B5AC3AD,	0x400000,	2,	0x1000004,	8,	true },
			{ "CROMxx",	false,	"mpr-20476.5",	0xCC97D758,	0x400000,	2,	0x1000006,	8,	true },

			// Banked CROM2
			{ "CROMxx",	false,	"mpr-20483.12",	0x7D487F3A,	0x400000,	2,	0x2000000,	8,	true },
			{ "CROMxx",	false,	"mpr-20482.11",	0xD21668D1,	0x400000,	2,	0x2000002,	8,	true },
			{ "CROMxx",	false,	"mpr-20481.10",	0x42ACC4F9,	0x400000,	2,	0x2000004,	8,	true },
			{ "CROMxx",	false,	"mpr-20480.9",	0x1E486A2E,	0x400000,	2,	0x2000006,	8,	true },

			// Video ROM
			{ "VROM",	false,	"mpr-20486.26",	0xDAB1F70F,	0x400000,	2,	0,		32,	false },
			{ "VROM",	false,	"mpr-20487.27",	0xFFB38774,	0x400000,	2,	2,		32,	false },
			{ "VROM",	false,	"mpr-20488.28",	0x0C25A1FB,	0x400000,	2,	4,		32,	false },
			{ "VROM",	false,	"mpr-20489.29",	0x6E8A911A,	0x400000,	2,	6,		32,	false },
			{ "VROM",	false,	"mpr-20490.30",	0x93DA0363,	0x400000,	2,	8,		32,	false },
			{ "VROM",	false,	"mpr-20491.31",	0xC4808E7A,	0x400000,	2,	10,		32,	false },
			{ "VROM",	false,	"mpr-20492.32",	0xD1B27B2B,	0x400000,	2,	12,		32,	false },
			{ "VROM",	false,	"mpr-20493.33",	0xE43CC6AF,	0x400000,	2,	14,		32,	false },
			{ "VROM",	false,	"mpr-20494.34",	0xB997B531,	0x400000,	2,	16,		32,	false },
			{ "VROM",	false,	"mpr-20495.35",	0x72480F09,	0x400000,	2,	18,		32,	false },
			{ "VROM",	false,	"mpr-20496.36",	0x96F6D3A8,	0x400000,	2,	20,		32,	false },
			{ "VROM",	false,	"mpr-20497.37",	0x7DC700A3,	0x400000,	2,	22,		32,	false },
			{ "VROM",	false,	"mpr-20498.38",	0x4E844081,	0x400000,	2,	24,		32,	false },
			{ "VROM",	false,	"mpr-20499.39",	0x09D9C7D1,	0x400000,	2,	26,		32,	false },
			{ "VROM",	false,	"mpr-20500.40",	0x3766FD87,	0x400000,	2,	28,		32,	false },
			{ "VROM",	false,	"mpr-20501.41",	0x741DA4AC,	0x400000,	2,	30,		32,	false },

			// Sound ROMs
			{ "SndProg",	false,	"epr-20506.21",	0x855AF67B,	0x80000,	2,	0,		2,	true },
			{ "Samples",	false,	"mpr-20484.22",	0x8AC3FBC4,	0x400000,	2,	0x000000,	2,	false },
			{ "Samples",	false,	"mpr-20485.24",	0xCFD8C19B,	0x400000,	2,	0x400000,	2,	false },
			
			// Drive Board ROM
			{ "DriveBd",	true,	"epr-20512.bin", 0xCF64350D, 0x10000, 2, 0, 2, false },

			{ NULL, false, NULL, 0, 0, 0, 0, 0, false }
		}
	},

	// Star Wars Trilogy (Revision A)
	{
		"swtrilgy",
		"Star Wars Trilogy (Revision A)",
		"Sega, LucasArts",
		1998,
		0x21,
		0x800000,	// 8 MB of fixed CROM
		true,		// 48 MB of banked CROM (Mirror)
		0x4000000,	// 64 MB of VROM
		0x800000,	// 8 MB of sample ROMs
		GAME_INPUT_COMMON|GAME_INPUT_ANALOG_JOYSTICK,
		2,			// DSB2 MPEG board
		false,		// no drive board

		{
			// Fixed CROM
			{ "CROM",	false,	"epr-21382a.20",	0x69BAF117,	0x200000,	2,	0x0000000,	8,	true },
			{ "CROM",	false,	"epr-21381a.19",	0x2DD34E28,	0x200000,	2,	0x0000002,	8,	true },
			{ "CROM",	false,	"epr-21380a.18",	0x780FB4E7,	0x200000,	2,	0x0000004,	8,	true },
			{ "CROM",	false,	"epr-21379a.17",	0x24DC1555,	0x200000,	2,	0x0000006,	8,	true },

			// Banked CROM0
			{ "CROMxx",	false,	"mpr-21342.04",	0x339525CE,	0x400000,	2,	0x0000000,	8,	true },
			{ "CROMxx",	false,	"mpr-21341.03",	0xB2A269E4,	0x400000,	2,	0x0000002,	8,	true },
			{ "CROMxx",	false,	"mpr-21340.02",	0xAD36040E,	0x400000,	2,	0x0000004,	8,	true },
			{ "CROMxx",	false,	"mpr-21339.01",	0xC0CE5037,	0x400000,	2,	0x0000006,	8,	true },

			// Banked CROM1
			{ "CROMxx",	false,	"mpr-21346.08",	0xC8733594,	0x400000,	2,	0x1000000,	8,	true },
			{ "CROMxx",	false,	"mpr-21345.07",	0x6C183A21,	0x400000,	2,	0x1000002,	8,	true },
			{ "CROMxx",	false,	"mpr-21344.06",	0x87453D76,	0x400000,	2,	0x1000004,	8,	true },
			{ "CROMxx",	false,	"mpr-21343.05",	0x12552D07,	0x400000,	2,	0x1000006,	8,	true },

			// Banked CROM2
			{ "CROMxx",	false,	"mpr-21350.12",	0x486195E7,	0x400000,	2,	0x2000000,	8,	true },
			{ "CROMxx",	false,	"mpr-21349.11",	0x3D39454B,	0x400000,	2,	0x2000002,	8,	true },
			{ "CROMxx",	false,	"mpr-21348.10",	0x1F7CC5F5,	0x400000,	2,	0x2000004,	8,	true },
			{ "CROMxx",	false,	"mpr-21347.09",	0xECB6B934,	0x400000,	2,	0x2000006,	8,	true },

			// Video ROM
			{ "VROM",	false,	"mpr-21359.26",	0x34EF4122,	0x400000,	2,	0,		32,	false },
			{ "VROM",	false,	"mpr-21360.27",	0x2882B95E,	0x400000,	2,	2,		32,	false },
			{ "VROM",	false,	"mpr-21361.28",	0x9B61C3C1,	0x400000,	2,	4,		32,	false },
			{ "VROM",	false,	"mpr-21362.29",	0x01A92169,	0x400000,	2,	6,		32,	false },
			{ "VROM",	false,	"mpr-21363.30",	0xE7D18FED,	0x400000,	2,	8,		32,	false },
			{ "VROM",	false,	"mpr-21364.31",	0xCB6A5468,	0x400000,	2,	10,		32,	false },
			{ "VROM",	false,	"mpr-21365.32",	0xAD5449D8,	0x400000,	2,	12,		32,	false },
			{ "VROM",	false,	"mpr-21366.33",	0xDEFB6B95,	0x400000,	2,	14,		32,	false },
			{ "VROM",	false,	"mpr-21367.34",	0xDFD51029,	0x400000,	2,	16,		32,	false },
			{ "VROM",	false,	"mpr-21368.35",	0xAE90FD21,	0x400000,	2,	18,		32,	false },
			{ "VROM",	false,	"mpr-21369.36",	0xBF17EEB4,	0x400000,	2,	20,		32,	false },
			{ "VROM",	false,	"mpr-21370.37",	0x2321592A,	0x400000,	2,	22,		32,	false },
			{ "VROM",	false,	"mpr-21371.38",	0xA68782FD,	0x400000,	2,	24,		32,	false },
			{ "VROM",	false,	"mpr-21372.39",	0xFC3F4E8B,	0x400000,	2,	26,		32,	false },
			{ "VROM",	false,	"mpr-21373.40",	0xB76AD261,	0x400000,	2,	28,		32,	false },
			{ "VROM",	false,	"mpr-21374.41",	0xAE6C4D28,	0x400000,	2,	30,		32,	false },

			// Sound ROMs
			{ "SndProg",	false,	"epr-21383.21",	0x544D1E28,	0x80000,	2,	0,		2,	true },
			{ "Samples",	false,	"mpr-21355.22",	0xC1B2D326,	0x400000,	2,	0x000000,	2,	false },
			{ "Samples",	false,	"mpr-21357.24",	0x02703FAB,	0x400000,	2,	0x400000,	2,	false },
			{ "DSBProg",	false,	"ep21384.2",	0x12FA4780,	0x20000,	2,	0,		2,	true },
			{ "DSBMPEG",	false,	"mp21375.18",	0x735157a9,	0x400000,	2,	0x000000,	2,	false },
			{ "DSBMPEG",	false,	"mp21376.20",	0xE635F81E,	0x400000,	2,	0x400000,	2,	false },
			{ "DSBMPEG",	false,	"mp21377.22",	0x720621F8,	0x400000,	2,	0x800000,	2,	false },
			{ "DSBMPEG",	false,	"mp21378.24",	0x1FCF715E,	0x400000,	2,	0xC00000,	2,	false },

			{ NULL, false, NULL, 0, 0, 0, 0, 0, false }
		}
	},

	// Star Wars Trilogy
	{
		"swtrilgya",
		"Star Wars Trilogy",
		"Sega, LucasArts",
		1998,
		0x21,
		0x800000,	// 8 MB of fixed CROM
		true,		// 48 MB of banked CROM (Mirror)
		0x4000000,	// 64 MB of VROM
		0x800000,	// 8 MB of sample ROMs
		GAME_INPUT_COMMON|GAME_INPUT_ANALOG_JOYSTICK,
		2,			// DSB2 MPEG board
		false,		// no drive board

		{
			// Fixed CROM
			{ "CROM",	false,	"ep21382.20",	0x0B9C44A0,	0x200000,	2,	0x0000000,	8,	true },
			{ "CROM",	false,	"ep21381.19",	0xBB5757BF,	0x200000,	2,	0x0000002,	8,	true },
			{ "CROM",	false,	"ep21380.18",	0x49B182F2,	0x200000,	2,	0x0000004,	8,	true },
			{ "CROM",	false,	"ep21379.17",	0x61AD51D9,	0x200000,	2,	0x0000006,	8,	true },

			// Banked CROM0
			{ "CROMxx",	false,	"mpr-21342.04",	0x339525CE,	0x400000,	2,	0x0000000,	8,	true },
			{ "CROMxx",	false,	"mpr-21341.03",	0xB2A269E4,	0x400000,	2,	0x0000002,	8,	true },
			{ "CROMxx",	false,	"mpr-21340.02",	0xAD36040E,	0x400000,	2,	0x0000004,	8,	true },
			{ "CROMxx",	false,	"mpr-21339.01",	0xC0CE5037,	0x400000,	2,	0x0000006,	8,	true },

			// Banked CROM1
			{ "CROMxx",	false,	"mpr-21346.08",	0xC8733594,	0x400000,	2,	0x1000000,	8,	true },
			{ "CROMxx",	false,	"mpr-21345.07",	0x6C183A21,	0x400000,	2,	0x1000002,	8,	true },
			{ "CROMxx",	false,	"mpr-21344.06",	0x87453D76,	0x400000,	2,	0x1000004,	8,	true },
			{ "CROMxx",	false,	"mpr-21343.05",	0x12552D07,	0x400000,	2,	0x1000006,	8,	true },

			// Banked CROM2
			{ "CROMxx",	false,	"mpr-21350.12",	0x486195E7,	0x400000,	2,	0x2000000,	8,	true },
			{ "CROMxx",	false,	"mpr-21349.11",	0x3D39454B,	0x400000,	2,	0x2000002,	8,	true },
			{ "CROMxx",	false,	"mpr-21348.10",	0x1F7CC5F5,	0x400000,	2,	0x2000004,	8,	true },
			{ "CROMxx",	false,	"mpr-21347.09",	0xECB6B934,	0x400000,	2,	0x2000006,	8,	true },

			// Video ROM
			{ "VROM",	false,	"mpr-21359.26",	0x34EF4122,	0x400000,	2,	0,		32,	false },
			{ "VROM",	false,	"mpr-21360.27",	0x2882B95E,	0x400000,	2,	2,		32,	false },
			{ "VROM",	false,	"mpr-21361.28",	0x9B61C3C1,	0x400000,	2,	4,		32,	false },
			{ "VROM",	false,	"mpr-21362.29",	0x01A92169,	0x400000,	2,	6,		32,	false },
			{ "VROM",	false,	"mpr-21363.30",	0xE7D18FED,	0x400000,	2,	8,		32,	false },
			{ "VROM",	false,	"mpr-21364.31",	0xCB6A5468,	0x400000,	2,	10,		32,	false },
			{ "VROM",	false,	"mpr-21365.32",	0xAD5449D8,	0x400000,	2,	12,		32,	false },
			{ "VROM",	false,	"mpr-21366.33",	0xDEFB6B95,	0x400000,	2,	14,		32,	false },
			{ "VROM",	false,	"mpr-21367.34",	0xDFD51029,	0x400000,	2,	16,		32,	false },
			{ "VROM",	false,	"mpr-21368.35",	0xAE90FD21,	0x400000,	2,	18,		32,	false },
			{ "VROM",	false,	"mpr-21369.36",	0xBF17EEB4,	0x400000,	2,	20,		32,	false },
			{ "VROM",	false,	"mpr-21370.37",	0x2321592A,	0x400000,	2,	22,		32,	false },
			{ "VROM",	false,	"mpr-21371.38",	0xA68782FD,	0x400000,	2,	24,		32,	false },
			{ "VROM",	false,	"mpr-21372.39",	0xFC3F4E8B,	0x400000,	2,	26,		32,	false },
			{ "VROM",	false,	"mpr-21373.40",	0xB76AD261,	0x400000,	2,	28,		32,	false },
			{ "VROM",	false,	"mpr-21374.41",	0xAE6C4D28,	0x400000,	2,	30,		32,	false },

			// Sound ROMs
			{ "SndProg",	false,	"epr-21383.21",	0x544D1E28,	0x80000,	2,	0,		2,	true },
			{ "Samples",	false,	"mpr-21355.22",	0xC1B2D326,	0x400000,	2,	0x000000,	2,	false },
			{ "Samples",	false,	"mpr-21357.24",	0x02703FAB,	0x400000,	2,	0x400000,	2,	false },
			{ "DSBProg",	false,	"ep21384.2",	0x12FA4780,	0x20000,	2,	0,		2,	true },
			{ "DSBMPEG",	false,	"mp21375.18",	0x735157a9,	0x400000,	2,	0x000000,	2,	false },
			{ "DSBMPEG",	false,	"mp21376.20",	0xE635F81E,	0x400000,	2,	0x400000,	2,	false },
			{ "DSBMPEG",	false,	"mp21377.22",	0x720621F8,	0x400000,	2,	0x800000,	2,	false },
			{ "DSBMPEG",	false,	"mp21378.24",	0x1FCF715E,	0x400000,	2,	0xC00000,	2,	false },

			{ NULL, false, NULL, 0, 0, 0, 0, 0, false }
		}
	},

	// Virtua Fighter 3 (Revision C)
	{
		"vf3",
		"Virtua Fighter 3",
		"Sega",
		1996,
		0x10,
		0x200000,	// 2 MB of fixed CROM
		true,		// 64 MB of banked CROM (Mirror)
		0x2000000,	// 32 MB of VROM
		0x800000,	// 8 MB of sample ROMs
		GAME_INPUT_COMMON|GAME_INPUT_JOYSTICK1|GAME_INPUT_JOYSTICK2|GAME_INPUT_FIGHTING,
		0,			// no MPEG board
		false,		// no drive board

		{
			// Fixed CROM
			{ "CROM",	false,	"epr-19230c.20",	0x736A9431,	0x80000,	2,	0x0600000,	8,	true },
			{ "CROM",	false,	"epr-19229c.19",	0x731B6B78,	0x80000,	2,	0x0600002,	8,	true },
			{ "CROM",	false,	"epr-19228c.18",	0x9C5727E2,	0x80000,	2,	0x0600004,	8,	true },
			{ "CROM",	false,	"epr-19227c.17",	0xA7DF4D75,	0x80000,	2,	0x0600006,	8,	true },

			// Banked CROM0
			{ "CROMxx",	false,	"mpr-19196.4",	0xF386B850,	0x400000,	2,	0x0000000,	8,	true },
			{ "CROMxx",	false,	"mpr-19195.3",	0xBD5E27A3,	0x400000,	2,	0x0000002,	8,	true },
			{ "CROMxx",	false,	"mpr-19194.2",	0x66254702,	0x400000,	2,	0x0000004,	8,	true },
			{ "CROMxx",	false,	"mpr-19193.1",	0x7BAB33D2,	0x400000,	2,	0x0000006,	8,	true },

			// Banked CROM1
			{ "CROMxx",	false,	"mpr-19200.8",	0x74941091,	0x400000,	2,	0x1000000,	8,	true },
			{ "CROMxx",	false,	"mpr-19199.7",	0x9F80D6FE,	0x400000,	2,	0x1000002,	8,	true },
			{ "CROMxx",	false,	"mpr-19198.6",	0xD8EE5032,	0x400000,	2,	0x1000004,	8,	true },
			{ "CROMxx",	false,	"mpr-19197.5",	0xA22D76C9,	0x400000,	2,	0x1000006,	8,	true },

			// Banked CROM2
			{ "CROMxx",	false,	"mpr-19204.12",	0x2F93310A,	0x400000,	2,	0x2000000,	8,	true },
			{ "CROMxx",	false,	"mpr-19203.11",	0x0AFA6334,	0x400000,	2,	0x2000002,	8,	true },
			{ "CROMxx",	false,	"mpr-19202.10",	0xAAA086C6,	0x400000,	2,	0x2000004,	8,	true },
			{ "CROMxx",	false,	"mpr-19201.9",	0x7C4A8C31,	0x400000,	2,	0x2000006,	8,	true },

			// Banked CROM3
			{ "CROMxx",	false,	"mpr-19208.16",	0x08F30F71,	0x400000,	2,	0x3000000,	8,	true },
			{ "CROMxx",	false,	"mpr-19207.15",	0x2CE1612D,	0x400000,	2,	0x3000002,	8,	true },
			{ "CROMxx",	false,	"mpr-19206.14",	0x71A98D73,	0x400000,	2,	0x3000004,	8,	true },
			{ "CROMxx",	false,	"mpr-19205.13",	0x199C328E,	0x400000,	2,	0x3000006,	8,	true },

			// Video ROM
			{ "VROM",	false,	"mpr-19211.26",	0x9C8F5DF1,	0x200000,	2,	0,		32,	false },
			{ "VROM",	false,	"mpr-19212.27",	0x75036234,	0x200000,	2,	2,		32,	false },
			{ "VROM",	false,	"mpr-19213.28",	0x67B123CF,	0x200000,	2,	4,		32,	false },
			{ "VROM",	false,	"mpr-19214.29",	0xA6F5576B,	0x200000,	2,	6,		32,	false },
			{ "VROM",	false,	"mpr-19215.30",	0xC6FD9F0D,	0x200000,	2,	8,		32,	false },
			{ "VROM",	false,	"mpr-19216.31",	0x201BB1ED,	0x200000,	2,	10,		32,	false },
			{ "VROM",	false,	"mpr-19217.32",	0x4DADD41A,	0x200000,	2,	12,		32,	false },
			{ "VROM",	false,	"mpr-19218.33",	0xCFF91953,	0x200000,	2,	14,		32,	false },
			{ "VROM",	false,	"mpr-19219.34",	0xC610D521,	0x200000,	2,	16,		32,	false },
			{ "VROM",	false,	"mpr-19220.35",	0xE62924D0,	0x200000,	2,	18,		32,	false },
			{ "VROM",	false,	"mpr-19221.36",	0x24F83E3C,	0x200000,	2,	20,		32,	false },
			{ "VROM",	false,	"mpr-19222.37",	0x61A6AA7D,	0x200000,	2,	22,		32,	false },
			{ "VROM",	false,	"mpr-19223.38",	0x1A8C1980,	0x200000,	2,	24,		32,	false },
			{ "VROM",	false,	"mpr-19224.39",	0x0A79A1BD,	0x200000,	2,	26,		32,	false },
			{ "VROM",	false,	"mpr-19225.40",	0x91A985EB,	0x200000,	2,	28,		32,	false },
			{ "VROM",	false,	"mpr-19226.41",	0x00091722,	0x200000,	2,	30,		32,	false },

			// Sound ROMs
			{ "SndProg",	false,	"epr19231.21",	0xB416FE96,	0x80000,	2,	0,		2,	true },
			{ "Samples",	false,	"mpr-19209.22",	0x3715E38C,	0x400000,	2,	0x000000,	2,	false },
			{ "Samples",	false,	"mpr-19210.24",	0xC03D6502,	0x400000,	2,	0x400000,	2,	false },

			{ NULL, false, NULL, 0, 0, 0, 0, 0, false }
		}
	},

	// Virtua Fighter 3 (Revision A)
	{
		"vf3a",
		"Virtua Fighter 3 (Revision A)",
		"Sega",
		1996,
		0x10,
		0x200000,	// 2 MB of fixed CROM
		true,		// 64 MB of banked CROM (Mirror)
		0x2000000,	// 32 MB of VROM
		0x800000,	// 8 MB of sample ROMs
		GAME_INPUT_COMMON|GAME_INPUT_JOYSTICK1|GAME_INPUT_JOYSTICK2|GAME_INPUT_FIGHTING,
		0,			// no MPEG board
		false,		// no drive board

		{
			// Fixed CROM
			{ "CROM",	false,	"epr19230a.20",	0x4DFF78ED,	0x80000,	2,	0x0600000,	8,	true },
			{ "CROM",	false,	"epr19229a.19",	0x5F1404B8,	0x80000,	2,	0x0600002,	8,	true },
			{ "CROM",	false,	"epr19228a.18",	0x82F17AB5,	0x80000,	2,	0x0600004,	8,	true },
			{ "CROM",	false,	"epr19227a.17",	0x7139931A,	0x80000,	2,	0x0600006,	8,	true },

			// Banked CROM0
			{ "CROMxx",	false,	"mpr-19196.4",	0xF386B850,	0x400000,	2,	0x0000000,	8,	true },
			{ "CROMxx",	false,	"mpr-19195.3",	0xBD5E27A3,	0x400000,	2,	0x0000002,	8,	true },
			{ "CROMxx",	false,	"mpr-19194.2",	0x66254702,	0x400000,	2,	0x0000004,	8,	true },
			{ "CROMxx",	false,	"mpr-19193.1",	0x7BAB33D2,	0x400000,	2,	0x0000006,	8,	true },

			// Banked CROM1
			{ "CROMxx",	false,	"mpr-19200.8",	0x74941091,	0x400000,	2,	0x1000000,	8,	true },
			{ "CROMxx",	false,	"mpr-19199.7",	0x9F80D6FE,	0x400000,	2,	0x1000002,	8,	true },
			{ "CROMxx",	false,	"mpr-19198.6",	0xD8EE5032,	0x400000,	2,	0x1000004,	8,	true },
			{ "CROMxx",	false,	"mpr-19197.5",	0xA22D76C9,	0x400000,	2,	0x1000006,	8,	true },

			// Banked CROM2
			{ "CROMxx",	false,	"mpr-19204.12",	0x2F93310A,	0x400000,	2,	0x2000000,	8,	true },
			{ "CROMxx",	false,	"mpr-19203.11",	0x0AFA6334,	0x400000,	2,	0x2000002,	8,	true },
			{ "CROMxx",	false,	"mpr-19202.10",	0xAAA086C6,	0x400000,	2,	0x2000004,	8,	true },
			{ "CROMxx",	false,	"mpr-19201.9",	0x7C4A8C31,	0x400000,	2,	0x2000006,	8,	true },

			// Banked CROM3
			{ "CROMxx",	false,	"mpr-19208.16",	0x08F30F71,	0x400000,	2,	0x3000000,	8,	true },
			{ "CROMxx",	false,	"mpr-19207.15",	0x2CE1612D,	0x400000,	2,	0x3000002,	8,	true },
			{ "CROMxx",	false,	"mpr-19206.14",	0x71A98D73,	0x400000,	2,	0x3000004,	8,	true },
			{ "CROMxx",	false,	"mpr-19205.13",	0x199C328E,	0x400000,	2,	0x3000006,	8,	true },

			// Video ROM
			{ "VROM",	false,	"mpr-19211.26",	0x9C8F5DF1,	0x200000,	2,	0,		32,	false },
			{ "VROM",	false,	"mpr-19212.27",	0x75036234,	0x200000,	2,	2,		32,	false },
			{ "VROM",	false,	"mpr-19213.28",	0x67B123CF,	0x200000,	2,	4,		32,	false },
			{ "VROM",	false,	"mpr-19214.29",	0xA6F5576B,	0x200000,	2,	6,		32,	false },
			{ "VROM",	false,	"mpr-19215.30",	0xC6FD9F0D,	0x200000,	2,	8,		32,	false },
			{ "VROM",	false,	"mpr-19216.31",	0x201BB1ED,	0x200000,	2,	10,		32,	false },
			{ "VROM",	false,	"mpr-19217.32",	0x4DADD41A,	0x200000,	2,	12,		32,	false },
			{ "VROM",	false,	"mpr-19218.33",	0xCFF91953,	0x200000,	2,	14,		32,	false },
			{ "VROM",	false,	"mpr-19219.34",	0xC610D521,	0x200000,	2,	16,		32,	false },
			{ "VROM",	false,	"mpr-19220.35",	0xE62924D0,	0x200000,	2,	18,		32,	false },
			{ "VROM",	false,	"mpr-19221.36",	0x24F83E3C,	0x200000,	2,	20,		32,	false },
			{ "VROM",	false,	"mpr-19222.37",	0x61A6AA7D,	0x200000,	2,	22,		32,	false },
			{ "VROM",	false,	"mpr-19223.38",	0x1A8C1980,	0x200000,	2,	24,		32,	false },
			{ "VROM",	false,	"mpr-19224.39",	0x0A79A1BD,	0x200000,	2,	26,		32,	false },
			{ "VROM",	false,	"mpr-19225.40",	0x91A985EB,	0x200000,	2,	28,		32,	false },
			{ "VROM",	false,	"mpr-19226.41",	0x00091722,	0x200000,	2,	30,		32,	false },

			// Sound ROMs
			{ "SndProg",	false,	"epr19231.21",	0xB416FE96,	0x80000,	2,	0,		2,	true },
			{ "Samples",	false,	"mpr-19209.22",	0x3715E38C,	0x400000,	2,	0x000000,	2,	false },
			{ "Samples",	false,	"mpr-19210.24",	0xC03D6502,	0x400000,	2,	0x400000,	2,	false },

			{ NULL, false, NULL, 0, 0, 0, 0, 0, false }
		}
	},

	// Virtua Fighter 3 Team Battle
	{
		"vf3tb",
		"Virtua Fighter 3 Team Battle",
		"Sega",
		1996,
		0x10,
		0x200000,	// 2 MB of fixed CROM
		true,		// 64 MB of banked CROM (Mirror)
		0x2000000,	// 32 MB of VROM
		0x800000,	// 8 MB of sample ROMs
		GAME_INPUT_COMMON|GAME_INPUT_JOYSTICK1|GAME_INPUT_JOYSTICK2|GAME_INPUT_FIGHTING,
		0,			// no MPEG board
		false,		// no drive board

		{
			// Fixed CROM
			{ "CROM",	false,	"epr-20129.20",	0x0DB897CE,	0x80000,	2,	0x0600000,	8,	true },
			{ "CROM",	false,	"epr-20128.19",	0xFFBDBDC5,	0x80000,	2,	0x0600002,	8,	true },
			{ "CROM",	false,	"epr-20127.18",	0x5C0F694B,	0x80000,	2,	0x0600004,	8,	true },
			{ "CROM",	false,	"epr-20126.17",	0x27ECD3B0,	0x80000,	2,	0x0600006,	8,	true },

			// Banked CROM0
			{ "CROMxx",	false,	"mpr-20133.4",	0x3D9B5171,	0x400000,	2,	0x0000000,	8,	true },
			{ "CROMxx",	false,	"mpr-20132.3",	0xF7557474,	0x400000,	2,	0x0000002,	8,	true },
			{ "CROMxx",	false,	"mpr-20131.2",	0x51FA69F1,	0x400000,	2,	0x0000004,	8,	true },
			{ "CROMxx",	false,	"mpr-20130.1",	0x40640446,	0x400000,	2,	0x0000006,	8,	true },

			// Banked CROM1
			{ "CROMxx",	false,	"mpr-19200.8",	0x74941091,	0x400000,	2,	0x1000000,	8,	true },
			{ "CROMxx",	false,	"mpr-19199.7",	0x9F80D6FE,	0x400000,	2,	0x1000002,	8,	true },
			{ "CROMxx",	false,	"mpr-19198.6",	0xD8EE5032,	0x400000,	2,	0x1000004,	8,	true },
			{ "CROMxx",	false,	"mpr-19197.5",	0xA22D76C9,	0x400000,	2,	0x1000006,	8,	true },

			// Banked CROM2
			{ "CROMxx",	false,	"mpr-19204.12",	0x2F93310A,	0x400000,	2,	0x2000000,	8,	true },
			{ "CROMxx",	false,	"mpr-19203.11",	0x0AFA6334,	0x400000,	2,	0x2000002,	8,	true },
			{ "CROMxx",	false,	"mpr-19202.10",	0xAAA086C6,	0x400000,	2,	0x2000004,	8,	true },
			{ "CROMxx",	false,	"mpr-19201.9",	0x7C4A8C31,	0x400000,	2,	0x2000006,	8,	true },

			// Banked CROM3
			{ "CROMxx",	false,	"mpr-19208.16",	0x08F30F71,	0x400000,	2,	0x3000000,	8,	true },
			{ "CROMxx",	false,	"mpr-19207.15",	0x2CE1612D,	0x400000,	2,	0x3000002,	8,	true },
			{ "CROMxx",	false,	"mpr-19206.14",	0x71A98D73,	0x400000,	2,	0x3000004,	8,	true },
			{ "CROMxx",	false,	"mpr-19205.13",	0x199C328E,	0x400000,	2,	0x3000006,	8,	true },

			// Video ROM
			{ "VROM",	false,	"mpr-19211.26",	0x9C8F5DF1,	0x200000,	2,	0,		32,	false },
			{ "VROM",	false,	"mpr-19212.27",	0x75036234,	0x200000,	2,	2,		32,	false },
			{ "VROM",	false,	"mpr-19213.28",	0x67B123CF,	0x200000,	2,	4,		32,	false },
			{ "VROM",	false,	"mpr-19214.29",	0xA6F5576B,	0x200000,	2,	6,		32,	false },
			{ "VROM",	false,	"mpr-19215.30",	0xC6FD9F0D,	0x200000,	2,	8,		32,	false },
			{ "VROM",	false,	"mpr-19216.31",	0x201BB1ED,	0x200000,	2,	10,		32,	false },
			{ "VROM",	false,	"mpr-19217.32",	0x4DADD41A,	0x200000,	2,	12,		32,	false },
			{ "VROM",	false,	"mpr-19218.33",	0xCFF91953,	0x200000,	2,	14,		32,	false },
			{ "VROM",	false,	"mpr-19219.34",	0xC610D521,	0x200000,	2,	16,		32,	false },
			{ "VROM",	false,	"mpr-19220.35",	0xE62924D0,	0x200000,	2,	18,		32,	false },
			{ "VROM",	false,	"mpr-19221.36",	0x24F83E3C,	0x200000,	2,	20,		32,	false },
			{ "VROM",	false,	"mpr-19222.37",	0x61A6AA7D,	0x200000,	2,	22,		32,	false },
			{ "VROM",	false,	"mpr-19223.38",	0x1A8C1980,	0x200000,	2,	24,		32,	false },
			{ "VROM",	false,	"mpr-19224.39",	0x0A79A1BD,	0x200000,	2,	26,		32,	false },
			{ "VROM",	false,	"mpr-19225.40",	0x91A985EB,	0x200000,	2,	28,		32,	false },
			{ "VROM",	false,	"mpr-19226.41",	0x00091722,	0x200000,	2,	30,		32,	false },

			// Sound ROMs
			{ "SndProg",	false,	"epr19231.21",	0xB416FE96,	0x80000,	2,	0,		2,	true },
			{ "Samples",	false,	"mpr-19209.22",	0x3715E38C,	0x400000,	2,	0x000000,	2,	false },
			{ "Samples",	false,	"mpr-19210.24",	0xC03D6502,	0x400000,	2,	0x400000,	2,	false },

			{ NULL, false, NULL, 0, 0, 0, 0, 0, false }
		}
	},

	// Virtual On: Oratorio Tangram (Revision B)
	{
		"von2",
		"Virtual On: Oratorio Tangram",
		"Sega",
		1998,
		0x20,
		0x800000,	// 8 MB of fixed CROM
		true,		// 64 MB of banked CROM (Mirror)
		0x4000000,	// 64 MB of VROM
		0x1000000,	// 16 MB of sample ROMs
		GAME_INPUT_COMMON|GAME_INPUT_TWIN_JOYSTICKS,
		0,			// no MPEG board
		false,		// no drive board

		{
			// Fixed CROM
			{ "CROM",	false,	"epr-20686b.20",	0x3EA4DE9F,	0x200000,	2,	0x0000000,	8,	true },
			{ "CROM",	false,	"epr-20685b.19",	0xAE82CB35,	0x200000,	2,	0x0000002,	8,	true },
			{ "CROM",	false,	"epr-20684b.18",	0x1FC15431,	0x200000,	2,	0x0000004,	8,	true },
			{ "CROM",	false,	"epr-20683b.17",	0x59D9C974,	0x200000,	2,	0x0000006,	8,	true },

			// Banked CROM0
			{ "CROMxx",	false,	"mpr-20650.4",	0x81F96649,	0x400000,	2,	0x0000000,	8,	true },
			{ "CROMxx",	false,	"mpr-20649.3",	0xB8FD56BA,	0x400000,	2,	0x0000002,	8,	true },
			{ "CROMxx",	false,	"mpr-20648.2",	0x107309E0,	0x400000,	2,	0x0000004,	8,	true },
			{ "CROMxx",	false,	"mpr-20647.1",	0xE8586380,	0x400000,	2,	0x0000006,	8,	true },

			// Banked CROM1
			{ "CROMxx",	false,	"mpr-20654.8",	0x763EF905,	0x400000,	2,	0x1000000,	8,	true },
			{ "CROMxx",	false,	"mpr-20653.7",	0x858E6BBA,	0x400000,	2,	0x1000002,	8,	true },
			{ "CROMxx",	false,	"mpr-20652.6",	0x64C6FBB6,	0x400000,	2,	0x1000004,	8,	true },
			{ "CROMxx",	false,	"mpr-20651.5",	0x8373CAB3,	0x400000,	2,	0x1000006,	8,	true },

			// Banked CROM2
			{ "CROMxx",	false,	"mpr-20658.12",	0xB80175B9,	0x400000,	2,	0x2000000,	8,	true },
			{ "CROMxx",	false,	"mpr-20657.11",	0x14BF8964,	0x400000,	2,	0x2000002,	8,	true },
			{ "CROMxx",	false,	"mpr-20656.10",	0x466BEE13,	0x400000,	2,	0x2000004,	8,	true },
			{ "CROMxx",	false,	"mpr-20655.9",	0xF0A471E9,	0x400000,	2,	0x2000006,	8,	true },

			// Banked CROM3
			{ "CROMxx",	false,	"mpr-20662.16",	0x7130CB61,	0x400000,	2,	0x3000000,	8,	true },
			{ "CROMxx",	false,	"mpr-20661.15",	0x50E6189E,	0x400000,	2,	0x3000002,	8,	true },
			{ "CROMxx",	false,	"mpr-20660.14",	0xD961D385,	0x400000,	2,	0x3000004,	8,	true },
			{ "CROMxx",	false,	"mpr-20659.13",	0xEDB63E7B,	0x400000,	2,	0x3000006,	8,	true },

			// Video ROM
			{ "VROM",	false,	"mpr-20667.26",	0x321E006F,	0x400000,	2,	0,		32,	false },
			{ "VROM",	false,	"mpr-20668.27",	0xC2DD8053,	0x400000,	2,	2,		32,	false },
			{ "VROM",	false,	"mpr-20669.28",	0x63432497,	0x400000,	2,	4,		32,	false },
			{ "VROM",	false,	"mpr-20670.29",	0xF7B554FD,	0x400000,	2,	6,		32,	false },
			{ "VROM",	false,	"mpr-20671.30",	0xFEE1A49B,	0x400000,	2,	8,		32,	false },
			{ "VROM",	false,	"mpr-20672.31",	0xE4B8C6E6,	0x400000,	2,	10,		32,	false },
			{ "VROM",	false,	"mpr-20673.32",	0xE7B6403B,	0x400000,	2,	12,		32,	false },
			{ "VROM",	false,	"mpr-20674.33",	0x9BE22E13,	0x400000,	2,	14,		32,	false },
			{ "VROM",	false,	"mpr-20675.34",	0x6A7C3862,	0x400000,	2,	16,		32,	false },
			{ "VROM",	false,	"mpr-20676.35",	0xDD299648,	0x400000,	2,	18,		32,	false },
			{ "VROM",	false,	"mpr-20677.36",	0x3FC5F330,	0x400000,	2,	20,		32,	false },
			{ "VROM",	false,	"mpr-20678.37",	0x62F794A1,	0x400000,	2,	22,		32,	false },
			{ "VROM",	false,	"mpr-20679.38",	0x35A37C53,	0x400000,	2,	24,		32,	false },
			{ "VROM",	false,	"mpr-20680.39",	0x81FEC46E,	0x400000,	2,	26,		32,	false },
			{ "VROM",	false,	"mpr-20681.40",	0xD517873B,	0x400000,	2,	28,		32,	false },
			{ "VROM",	false,	"mpr-20682.41",	0x5B43250C,	0x400000,	2,	30,		32,	false },

			// Sound ROMs
			{ "SndProg",	false,	"epr-20687.21",	0xFA084DE5,	0x80000,	2,	0,		2,	true },
			{ "Samples",	false,	"mpr-20663.22",	0x977EB6A4,	0x400000,	2,	0x000000,	2,	false },
			{ "Samples",	false,	"mpr-20665.24",	0x0EFC0CA8,	0x400000,	2,	0x400000,	2,	false },
			{ "Samples",	false,	"mpr-20664.23",	0x89220782,	0x400000,	2,	0x800000,	2,	false },
			{ "Samples",	false,	"mpr-20666.25",	0x3ECB2606,	0x400000,	2,	0xC00000,	2,	false },

			{ NULL, false, NULL, 0, 0, 0, 0, 0, false }
		}
	},

	// Virtual On: Oratorio Tangram (Version 5.4g)
	{
		"von254g",
		"Virtual On: Oratorio Tangram (Version 5.4g)",
		"Sega",
		1998,
		0x20,
		0x800000,	// 8 MB of fixed CROM
		true,		// 64 MB of banked CROM (Mirror)
		0x4000000,	// 64 MB of VROM
		0x1000000,	// 16 MB of sample ROMs
		GAME_INPUT_COMMON|GAME_INPUT_TWIN_JOYSTICKS,
		0,			// no MPEG board
		false,		// no drive board

		{
			// Fixed CROM
			{ "CROM",	false,	"epr-21791.20",	0xD0BB3CA3,	0x200000,	2,	0x0000000,	8,	true },
			{ "CROM",	false,	"epr-21790.19",	0x2AE1EFD3,	0x200000,	2,	0x0000002,	8,	true },
			{ "CROM",	false,	"epr-21789.18",	0x3069108F,	0x200000,	2,	0x0000004,	8,	true },
			{ "CROM",	false,	"epr-21788.17",	0x97066BCF,	0x200000,	2,	0x0000006,	8,	true },

			// Banked CROM0
			{ "CROMxx",	false,	"mpr-20650.4",	0x81F96649,	0x400000,	2,	0x0000000,	8,	true },
			{ "CROMxx",	false,	"mpr-20649.3",	0xB8FD56BA,	0x400000,	2,	0x0000002,	8,	true },
			{ "CROMxx",	false,	"mpr-20648.2",	0x107309E0,	0x400000,	2,	0x0000004,	8,	true },
			{ "CROMxx",	false,	"mpr-20647.1",	0xE8586380,	0x400000,	2,	0x0000006,	8,	true },

			// Banked CROM1
			{ "CROMxx",	false,	"mpr-20654.8",	0x763EF905,	0x400000,	2,	0x1000000,	8,	true },
			{ "CROMxx",	false,	"mpr-20653.7",	0x858E6BBA,	0x400000,	2,	0x1000002,	8,	true },
			{ "CROMxx",	false,	"mpr-20652.6",	0x64C6FBB6,	0x400000,	2,	0x1000004,	8,	true },
			{ "CROMxx",	false,	"mpr-20651.5",	0x8373CAB3,	0x400000,	2,	0x1000006,	8,	true },

			// Banked CROM2
			{ "CROMxx",	false,	"mpr-20658.12",	0xB80175B9,	0x400000,	2,	0x2000000,	8,	true },
			{ "CROMxx",	false,	"mpr-20657.11",	0x14BF8964,	0x400000,	2,	0x2000002,	8,	true },
			{ "CROMxx",	false,	"mpr-20656.10",	0x466BEE13,	0x400000,	2,	0x2000004,	8,	true },
			{ "CROMxx",	false,	"mpr-20655.9",	0xF0A471E9,	0x400000,	2,	0x2000006,	8,	true },

			// Banked CROM3
			{ "CROMxx",	false,	"mpr-20662.16",	0x7130CB61,	0x400000,	2,	0x3000000,	8,	true },
			{ "CROMxx",	false,	"mpr-20661.15",	0x50E6189E,	0x400000,	2,	0x3000002,	8,	true },
			{ "CROMxx",	false,	"mpr-20660.14",	0xD961D385,	0x400000,	2,	0x3000004,	8,	true },
			{ "CROMxx",	false,	"mpr-20659.13",	0xEDB63E7B,	0x400000,	2,	0x3000006,	8,	true },

			// Video ROM
			{ "VROM",	false,	"mpr-20667.26",	0x321E006F,	0x400000,	2,	0,		32,	false },
			{ "VROM",	false,	"mpr-20668.27",	0xC2DD8053,	0x400000,	2,	2,		32,	false },
			{ "VROM",	false,	"mpr-20669.28",	0x63432497,	0x400000,	2,	4,		32,	false },
			{ "VROM",	false,	"mpr-20670.29",	0xF7B554FD,	0x400000,	2,	6,		32,	false },
			{ "VROM",	false,	"mpr-20671.30",	0xFEE1A49B,	0x400000,	2,	8,		32,	false },
			{ "VROM",	false,	"mpr-20672.31",	0xE4B8C6E6,	0x400000,	2,	10,		32,	false },
			{ "VROM",	false,	"mpr-20673.32",	0xE7B6403B,	0x400000,	2,	12,		32,	false },
			{ "VROM",	false,	"mpr-20674.33",	0x9BE22E13,	0x400000,	2,	14,		32,	false },
			{ "VROM",	false,	"mpr-20675.34",	0x6A7C3862,	0x400000,	2,	16,		32,	false },
			{ "VROM",	false,	"mpr-20676.35",	0xDD299648,	0x400000,	2,	18,		32,	false },
			{ "VROM",	false,	"mpr-20677.36",	0x3FC5F330,	0x400000,	2,	20,		32,	false },
			{ "VROM",	false,	"mpr-20678.37",	0x62F794A1,	0x400000,	2,	22,		32,	false },
			{ "VROM",	false,	"mpr-20679.38",	0x35A37C53,	0x400000,	2,	24,		32,	false },
			{ "VROM",	false,	"mpr-20680.39",	0x81FEC46E,	0x400000,	2,	26,		32,	false },
			{ "VROM",	false,	"mpr-20681.40",	0xD517873B,	0x400000,	2,	28,		32,	false },
			{ "VROM",	false,	"mpr-20682.41",	0x5B43250C,	0x400000,	2,	30,		32,	false },

			// Sound ROMs
			{ "SndProg",	false,	"epr-20687.21",	0xFA084DE5,	0x80000,	2,	0,		2,	true },
			{ "Samples",	false,	"mpr-20663.22",	0x977EB6A4,	0x400000,	2,	0x000000,	2,	false },
			{ "Samples",	false,	"mpr-20665.24",	0x0EFC0CA8,	0x400000,	2,	0x400000,	2,	false },
			{ "Samples",	false,	"mpr-20664.23",	0x89220782,	0x400000,	2,	0x800000,	2,	false },
			{ "Samples",	false,	"mpr-20666.25",	0x3ECB2606,	0x400000,	2,	0xC00000,	2,	false },

			{ NULL, false, NULL, 0, 0, 0, 0, 0, false }
		}
	},

	// Virtua Striker 2 (Step 2.0)
	{
		"vs2",
		"Virtua Striker 2 (Step 2.0)",
		"Sega",
		1997,
		0x20,
		0x400000,	// 4 MB of fixed CROM
		true,		// 64 MB of banked CROM (Mirror)
		0x2000000,	// 32 MB of VROM
		0x800000,	// 8 MB of sample ROMs
		GAME_INPUT_COMMON|GAME_INPUT_JOYSTICK1|GAME_INPUT_JOYSTICK2|GAME_INPUT_SOCCER,
		0,			// no MPEG board
		false,		// no drive board

		{
			// Fixed CROM
			{ "CROM",	false,	"epr-20470.20",	0x2F62B292,	0x100000,	2,	0x0400000,	8,	true },
			{ "CROM",	false,	"epr-20469.19",	0x9D7521F6,	0x100000,	2,	0x0400002,	8,	true },
			{ "CROM",	false,	"epr-20468.18",	0xF0F0B6EA,	0x100000,	2,	0x0400004,	8,	true },
			{ "CROM",	false,	"epr-20467.17",	0x25D7AE73,	0x100000,	2,	0x0400006,	8,	true },

			// Banked CROM0
			{ "CROMxx",	false,	"mpr-19772.4",	0x6DB7B9D0,	0x400000,	2,	0x0000000,	8,	true },
			{ "CROMxx",	false,	"mpr-19771.3",	0x189C510F,	0x400000,	2,	0x0000002,	8,	true },
			{ "CROMxx",	false,	"mpr-19770.2",	0x91F690B0,	0x400000,	2,	0x0000004,	8,	true },
			{ "CROMxx",	false,	"mpr-19769.1",	0xDC020031,	0x400000,	2,	0x0000006,	8,	true },

			// Banked CROM1
			{ "CROMxx",	false,	"mpr-19776.8",	0x5B31C7C1,	0x400000,	2,	0x1000000,	8,	true },
			{ "CROMxx",	false,	"mpr-19775.7",	0xA6B32BD9,	0x400000,	2,	0x1000002,	8,	true },
			{ "CROMxx",	false,	"mpr-19774.6",	0x1D61D287,	0x400000,	2,	0x1000004,	8,	true },
			{ "CROMxx",	false,	"mpr-19773.5",	0x4E381AE7,	0x400000,	2,	0x1000006,	8,	true },

			// Banked CROM2
			{ "CROMxx",	false,	"mpr-19780.12",	0x38508791,	0x400000,	2,	0x2000000,	8,	true },
			{ "CROMxx",	false,	"mpr-19779.11",	0x2242B21B,	0x400000,	2,	0x2000002,	8,	true },
			{ "CROMxx",	false,	"mpr-19778.10",	0x2192B189,	0x400000,	2,	0x2000004,	8,	true },
			{ "CROMxx",	false,	"mpr-19777.9",	0xC8F216A6,	0x400000,	2,	0x2000006,	8,	true },

			// Banked CROM3
			{ "CROMxx",	false,	"mpr-19784.16",	0xA1CC70BE,	0x400000,	2,	0x3000000,	8,	true },
			{ "CROMxx",	false,	"mpr-19783.15",	0x47C3D726,	0x400000,	2,	0x3000002,	8,	true },
			{ "CROMxx",	false,	"mpr-19782.14",	0x43B43EEF,	0x400000,	2,	0x3000004,	8,	true },
			{ "CROMxx",	false,	"mpr-19781.13",	0x783213F4,	0x400000,	2,	0x3000006,	8,	true },

			// Video ROM
			{ "VROM",	false,	"mpr-19787.26",	0x856CC4AD,	0x200000,	2,	0,		32,	false },
			{ "VROM",	false,	"mpr-19788.27",	0x72EF970A,	0x200000,	2,	2,		32,	false },
			{ "VROM",	false,	"mpr-19789.28",	0x076ADD9A,	0x200000,	2,	4,		32,	false },
			{ "VROM",	false,	"mpr-19790.29",	0x74CE238C,	0x200000,	2,	6,		32,	false },
			{ "VROM",	false,	"mpr-19791.30",	0x75A98F96,	0x200000,	2,	8,		32,	false },
			{ "VROM",	false,	"mpr-19792.31",	0x85C81633,	0x200000,	2,	10,		32,	false },
			{ "VROM",	false,	"mpr-19793.32",	0x7F288CC4,	0x200000,	2,	12,		32,	false },
			{ "VROM",	false,	"mpr-19794.33",	0xE0C1C370,	0x200000,	2,	14,		32,	false },
			{ "VROM",	false,	"mpr-19795.34",	0x90989B20,	0x200000,	2,	16,		32,	false },
			{ "VROM",	false,	"mpr-19796.35",	0x5D1AAB8D,	0x200000,	2,	18,		32,	false },
			{ "VROM",	false,	"mpr-19797.36",	0xF5EDC891,	0x200000,	2,	20,		32,	false },
			{ "VROM",	false,	"mpr-19798.37",	0xAE2DA90F,	0x200000,	2,	22,		32,	false },
			{ "VROM",	false,	"mpr-19799.38",	0x92B18AD7,	0x200000,	2,	24,		32,	false },
			{ "VROM",	false,	"mpr-19800.39",	0x4A57B16C,	0x200000,	2,	26,		32,	false },
			{ "VROM",	false,	"mpr-19801.40",	0xBEB79A00,	0x200000,	2,	28,		32,	false },
			{ "VROM",	false,	"mpr-19802.41",	0xF2C3A7B7,	0x200000,	2,	30,		32,	false },

			// Sound ROMs
			{ "SndProg",	false,	"epr-19807.21",	0x9641CBAF,	0x80000,	2,	0,		2,	true },
			{ "Samples",	false,	"mpr-19785.22",	0xE7D190E3,	0x400000,	2,	0x000000,	2,	false },
			{ "Samples",	false,	"mpr-19786.24",	0xB08D889B,	0x400000,	2,	0x400000,	2,	false },

			{ NULL, false, NULL, 0, 0, 0, 0, 0, false }
		}
	},

	// Virtua Striker 2 (Step 1.5)
	{
		"vs215",
		"Virtua Striker 2 (Step 1.5)",
		"Sega",
		1997,
		0x15,
		0x200000,	// 2 MB of fixed CROM
		true,		// 64 MB of banked CROM (Mirror)
		0x2000000,	// 32 MB of VROM
		0x800000,	// 8 MB of sample ROMs
		GAME_INPUT_COMMON|GAME_INPUT_JOYSTICK1|GAME_INPUT_JOYSTICK2|GAME_INPUT_SOCCER,
		0,			// no MPEG board
		false,		// no drive board

		{
			// Fixed CROM
			{ "CROM",	false,	"epr-19897.20",	0x25A722A9,	0x80000,	2,	0x0600000,	8,	true },
			{ "CROM",	false,	"epr-19898.19",	0x4389D9CE,	0x80000,	2,	0x0600002,	8,	true },
			{ "CROM",	false,	"epr-19899.18",	0x8CC2BE9F,	0x80000,	2,	0x0600004,	8,	true },
			{ "CROM",	false,	"epr-19900.17",	0x8FB6045D,	0x80000,	2,	0x0600006,	8,	true },

			// Banked CROM0
			{ "CROMxx",	false,	"mpr-19772.4",	0x6DB7B9D0,	0x400000,	2,	0x0000000,	8,	true },
			{ "CROMxx",	false,	"mpr-19771.3",	0x189C510F,	0x400000,	2,	0x0000002,	8,	true },
			{ "CROMxx",	false,	"mpr-19770.2",	0x91F690B0,	0x400000,	2,	0x0000004,	8,	true },
			{ "CROMxx",	false,	"mpr-19769.1",	0xDC020031,	0x400000,	2,	0x0000006,	8,	true },

			// Banked CROM1
			{ "CROMxx",	false,	"mpr-19776.8",	0x5B31C7C1,	0x400000,	2,	0x1000000,	8,	true },
			{ "CROMxx",	false,	"mpr-19775.7",	0xA6B32BD9,	0x400000,	2,	0x1000002,	8,	true },
			{ "CROMxx",	false,	"mpr-19774.6",	0x1D61D287,	0x400000,	2,	0x1000004,	8,	true },
			{ "CROMxx",	false,	"mpr-19773.5",	0x4E381AE7,	0x400000,	2,	0x1000006,	8,	true },

			// Banked CROM2
			{ "CROMxx",	false,	"mpr-19780.12",	0x38508791,	0x400000,	2,	0x2000000,	8,	true },
			{ "CROMxx",	false,	"mpr-19779.11",	0x2242B21B,	0x400000,	2,	0x2000002,	8,	true },
			{ "CROMxx",	false,	"mpr-19778.10",	0x2192B189,	0x400000,	2,	0x2000004,	8,	true },
			{ "CROMxx",	false,	"mpr-19777.9",	0xC8F216A6,	0x400000,	2,	0x2000006,	8,	true },

			// Banked CROM3
			{ "CROMxx",	false,	"mpr-19784.16",	0xA1CC70BE,	0x400000,	2,	0x3000000,	8,	true },
			{ "CROMxx",	false,	"mpr-19783.15",	0x47C3D726,	0x400000,	2,	0x3000002,	8,	true },
			{ "CROMxx",	false,	"mpr-19782.14",	0x43B43EEF,	0x400000,	2,	0x3000004,	8,	true },
			{ "CROMxx",	false,	"mpr-19781.13",	0x783213F4,	0x400000,	2,	0x3000006,	8,	true },

			// Video ROM
			{ "VROM",	false,	"mpr-19787.26",	0x856CC4AD,	0x200000,	2,	0,		32,	false },
			{ "VROM",	false,	"mpr-19788.27",	0x72EF970A,	0x200000,	2,	2,		32,	false },
			{ "VROM",	false,	"mpr-19789.28",	0x076ADD9A,	0x200000,	2,	4,		32,	false },
			{ "VROM",	false,	"mpr-19790.29",	0x74CE238C,	0x200000,	2,	6,		32,	false },
			{ "VROM",	false,	"mpr-19791.30",	0x75A98F96,	0x200000,	2,	8,		32,	false },
			{ "VROM",	false,	"mpr-19792.31",	0x85C81633,	0x200000,	2,	10,		32,	false },
			{ "VROM",	false,	"mpr-19793.32",	0x7F288CC4,	0x200000,	2,	12,		32,	false },
			{ "VROM",	false,	"mpr-19794.33",	0xE0C1C370,	0x200000,	2,	14,		32,	false },
			{ "VROM",	false,	"mpr-19795.34",	0x90989B20,	0x200000,	2,	16,		32,	false },
			{ "VROM",	false,	"mpr-19796.35",	0x5D1AAB8D,	0x200000,	2,	18,		32,	false },
			{ "VROM",	false,	"mpr-19797.36",	0xF5EDC891,	0x200000,	2,	20,		32,	false },
			{ "VROM",	false,	"mpr-19798.37",	0xAE2DA90F,	0x200000,	2,	22,		32,	false },
			{ "VROM",	false,	"mpr-19799.38",	0x92B18AD7,	0x200000,	2,	24,		32,	false },
			{ "VROM",	false,	"mpr-19800.39",	0x4A57B16C,	0x200000,	2,	26,		32,	false },
			{ "VROM",	false,	"mpr-19801.40",	0xBEB79A00,	0x200000,	2,	28,		32,	false },
			{ "VROM",	false,	"mpr-19802.41",	0xF2C3A7B7,	0x200000,	2,	30,		32,	false },

			// Sound ROMs
			{ "SndProg",	false,	"epr-19807.21",	0x9641CBAF,	0x80000,	2,	0,		2,	true },
			{ "Samples",	false,	"mpr-19785.22",	0xE7D190E3,	0x400000,	2,	0x000000,	2,	false },
			{ "Samples",	false,	"mpr-19786.24",	0xB08D889B,	0x400000,	2,	0x400000,	2,	false },

			{ NULL, false, NULL, 0, 0, 0, 0, 0, false }
		}
	},

	// Virtua Striker 2 '98 (Step 2.0)
	{
		"vs298",
		"Virtua Striker 2 '98 (Step 2.0)",
		"Sega",
		1998,
		0x20,
		0x400000,	// 4 MB of fixed CROM
		true,		// 64 MB of banked CROM (Mirror)
		0x2000000,	// 32 MB of VROM
		0x800000,	// 8 MB of sample ROMs
		GAME_INPUT_COMMON|GAME_INPUT_JOYSTICK1|GAME_INPUT_JOYSTICK2|GAME_INPUT_SOCCER,
		0,			// no MPEG board
		false,		// no drive board

		{
			// Fixed CROM
			{ "CROM",	false,	"epr-20920.20",	0x428D05FC,	0x100000,	2,	0x0400000,	8,	true },
			{ "CROM",	false,	"epr-20919.19",	0x7A0713D2,	0x100000,	2,	0x0400002,	8,	true },
			{ "CROM",	false,	"epr-20918.18",	0x0E9CDC5B,	0x100000,	2,	0x0400004,	8,	true },
			{ "CROM",	false,	"epr-20917.17",	0xC3BBB270,	0x100000,	2,	0x0400006,	8,	true },

			// Banked CROM0
			{ "CROMxx",	false,	"mpr-19894.4",	0x09C065CC,	0x400000,	2,	0x0000000,	8,	true },
			{ "CROMxx",	false,	"mpr-19893.3",	0x5C83DCAA,	0x400000,	2,	0x0000002,	8,	true },
			{ "CROMxx",	false,	"mpr-19892.2",	0x8E5D3FE7,	0x400000,	2,	0x0000004,	8,	true },
			{ "CROMxx",	false,	"mpr-19891.1",	0x9ECB0B39,	0x400000,	2,	0x0000006,	8,	true },

			// Banked CROM1
			{ "CROMxx",	false,	"mpr-19776.8",	0x5B31C7C1,	0x400000,	2,	0x1000000,	8,	true },
			{ "CROMxx",	false,	"mpr-19775.7",	0xA6B32BD9,	0x400000,	2,	0x1000002,	8,	true },
			{ "CROMxx",	false,	"mpr-19774.6",	0x1D61D287,	0x400000,	2,	0x1000004,	8,	true },
			{ "CROMxx",	false,	"mpr-19773.5",	0x4E381AE7,	0x400000,	2,	0x1000006,	8,	true },

			// Banked CROM2
			{ "CROMxx",	false,	"mpr-20898.12",	0x94040D37,	0x400000,	2,	0x2000000,	8,	true },
			{ "CROMxx",	false,	"mpr-20897.11",	0xC5CF067A,	0x400000,	2,	0x2000002,	8,	true },
			{ "CROMxx",	false,	"mpr-20896.10",	0xBF1CBD5E,	0x400000,	2,	0x2000004,	8,	true },
			{ "CROMxx",	false,	"mpr-20895.9",	0x9B51CBF5,	0x400000,	2,	0x2000006,	8,	true },

			// Banked CROM3
			{ "CROMxx",	false,	"mpr-20902.16",	0xF4D3FF3A,	0x400000,	2,	0x3000000,	8,	true },
			{ "CROMxx",	false,	"mpr-20901.15",	0x3492DDC8,	0x400000,	2,	0x3000002,	8,	true },
			{ "CROMxx",	false,	"mpr-20900.14",	0x7A38B571,	0x400000,	2,	0x3000004,	8,	true },
			{ "CROMxx",	false,	"mpr-20899.13",	0x65422425,	0x400000,	2,	0x3000006,	8,	true },

			// Video ROM
			{ "VROM",	false,	"mpr-19787.26",	0x856CC4AD,	0x200000,	2,	0,		32,	false },
			{ "VROM",	false,	"mpr-19788.27",	0x72EF970A,	0x200000,	2,	2,		32,	false },
			{ "VROM",	false,	"mpr-19789.28",	0x076ADD9A,	0x200000,	2,	4,		32,	false },
			{ "VROM",	false,	"mpr-19790.29",	0x74CE238C,	0x200000,	2,	6,		32,	false },
			{ "VROM",	false,	"mpr-19791.30",	0x75A98F96,	0x200000,	2,	8,		32,	false },
			{ "VROM",	false,	"mpr-19792.31",	0x85C81633,	0x200000,	2,	10,		32,	false },
			{ "VROM",	false,	"mpr-19793.32",	0x7F288CC4,	0x200000,	2,	12,		32,	false },
			{ "VROM",	false,	"mpr-19794.33",	0xE0C1C370,	0x200000,	2,	14,		32,	false },
			{ "VROM",	false,	"mpr-19795.34",	0x90989B20,	0x200000,	2,	16,		32,	false },
			{ "VROM",	false,	"mpr-19796.35",	0x5D1AAB8D,	0x200000,	2,	18,		32,	false },
			{ "VROM",	false,	"mpr-19797.36",	0xF5EDC891,	0x200000,	2,	20,		32,	false },
			{ "VROM",	false,	"mpr-19798.37",	0xAE2DA90F,	0x200000,	2,	22,		32,	false },
			{ "VROM",	false,	"mpr-19799.38",	0x92B18AD7,	0x200000,	2,	24,		32,	false },
			{ "VROM",	false,	"mpr-19800.39",	0x4A57B16C,	0x200000,	2,	26,		32,	false },
			{ "VROM",	false,	"mpr-19801.40",	0xBEB79A00,	0x200000,	2,	28,		32,	false },
			{ "VROM",	false,	"mpr-19802.41",	0xF2C3A7B7,	0x200000,	2,	30,		32,	false },

			// Sound ROMs
			{ "SndProg",	false,	"epr-20921.21",	0x30F032A7,	0x80000,	2,	0,		2,	true },
			{ "Samples",	false,	"mpr-20903.22",	0xE343E131,	0x400000,	2,	0x000000,	2,	false },
			{ "Samples",	false,	"mpr-20904.24",	0x21A91B84,	0x400000,	2,	0x400000,	2,	false },

			{ NULL, false, NULL, 0, 0, 0, 0, 0, false }
		}
	},

	// Virtua Striker 2 '98 (Step 1.5)
	{
		"vs29815",
		"Virtua Striker 2 '98 (Step 1.5)",
		"Sega",
		1998,
		0x15,
		0x200000,	// 2 MB of fixed CROM
		true,		// 64 MB of banked CROM (Mirror)
		0x2000000,	// 32 MB of VROM
		0x800000,	// 8 MB of sample ROMs
		GAME_INPUT_COMMON|GAME_INPUT_JOYSTICK1|GAME_INPUT_JOYSTICK2|GAME_INPUT_SOCCER,
		0,			// no MPEG board
		false,		// no drive board

		{
			// Fixed CROM
			{ "CROM",	false,	"epr-20912.20",	0xCD2C0538,	0x80000,	2,	0x0600000,	8,	true },
			{ "CROM",	false,	"epr-20911.19",	0xACB8FD97,	0x80000,	2,	0x0600002,	8,	true },
			{ "CROM",	false,	"epr-20910.18",	0xDC75A2E3,	0x80000,	2,	0x0600004,	8,	true },
			{ "CROM",	false,	"epr-20909.17",	0x3DFF0D7E,	0x80000,	2,	0x0600006,	8,	true },

			// Banked CROM0
			{ "CROMxx",	false,	"mpr-19894.4",	0x09C065CC,	0x400000,	2,	0x0000000,	8,	true },
			{ "CROMxx",	false,	"mpr-19893.3",	0x5C83DCAA,	0x400000,	2,	0x0000002,	8,	true },
			{ "CROMxx",	false,	"mpr-19892.2",	0x8E5D3FE7,	0x400000,	2,	0x0000004,	8,	true },
			{ "CROMxx",	false,	"mpr-19891.1",	0x9ECB0B39,	0x400000,	2,	0x0000006,	8,	true },

			// Banked CROM1
			{ "CROMxx",	false,	"mpr-19776.8",	0x5B31C7C1,	0x400000,	2,	0x1000000,	8,	true },
			{ "CROMxx",	false,	"mpr-19775.7",	0xA6B32BD9,	0x400000,	2,	0x1000002,	8,	true },
			{ "CROMxx",	false,	"mpr-19774.6",	0x1D61D287,	0x400000,	2,	0x1000004,	8,	true },
			{ "CROMxx",	false,	"mpr-19773.5",	0x4E381AE7,	0x400000,	2,	0x1000006,	8,	true },

			// Banked CROM2
			{ "CROMxx",	false,	"mpr-20898.12",	0x94040D37,	0x400000,	2,	0x2000000,	8,	true },
			{ "CROMxx",	false,	"mpr-20897.11",	0xC5CF067A,	0x400000,	2,	0x2000002,	8,	true },
			{ "CROMxx",	false,	"mpr-20896.10",	0xBF1CBD5E,	0x400000,	2,	0x2000004,	8,	true },
			{ "CROMxx",	false,	"mpr-20895.9",	0x9B51CBF5,	0x400000,	2,	0x2000006,	8,	true },

			// Banked CROM3
			{ "CROMxx",	false,	"mpr-20902.16",	0xF4D3FF3A,	0x400000,	2,	0x3000000,	8,	true },
			{ "CROMxx",	false,	"mpr-20901.15",	0x3492DDC8,	0x400000,	2,	0x3000002,	8,	true },
			{ "CROMxx",	false,	"mpr-20900.14",	0x7A38B571,	0x400000,	2,	0x3000004,	8,	true },
			{ "CROMxx",	false,	"mpr-20899.13",	0x65422425,	0x400000,	2,	0x3000006,	8,	true },

			// Video ROM
			{ "VROM",	false,	"mpr-19787.26",	0x856CC4AD,	0x200000,	2,	0,		32,	false },
			{ "VROM",	false,	"mpr-19788.27",	0x72EF970A,	0x200000,	2,	2,		32,	false },
			{ "VROM",	false,	"mpr-19789.28",	0x076ADD9A,	0x200000,	2,	4,		32,	false },
			{ "VROM",	false,	"mpr-19790.29",	0x74CE238C,	0x200000,	2,	6,		32,	false },
			{ "VROM",	false,	"mpr-19791.30",	0x75A98F96,	0x200000,	2,	8,		32,	false },
			{ "VROM",	false,	"mpr-19792.31",	0x85C81633,	0x200000,	2,	10,		32,	false },
			{ "VROM",	false,	"mpr-19793.32",	0x7F288CC4,	0x200000,	2,	12,		32,	false },
			{ "VROM",	false,	"mpr-19794.33",	0xE0C1C370,	0x200000,	2,	14,		32,	false },
			{ "VROM",	false,	"mpr-19795.34",	0x90989B20,	0x200000,	2,	16,		32,	false },
			{ "VROM",	false,	"mpr-19796.35",	0x5D1AAB8D,	0x200000,	2,	18,		32,	false },
			{ "VROM",	false,	"mpr-19797.36",	0xF5EDC891,	0x200000,	2,	20,		32,	false },
			{ "VROM",	false,	"mpr-19798.37",	0xAE2DA90F,	0x200000,	2,	22,		32,	false },
			{ "VROM",	false,	"mpr-19799.38",	0x92B18AD7,	0x200000,	2,	24,		32,	false },
			{ "VROM",	false,	"mpr-19800.39",	0x4A57B16C,	0x200000,	2,	26,		32,	false },
			{ "VROM",	false,	"mpr-19801.40",	0xBEB79A00,	0x200000,	2,	28,		32,	false },
			{ "VROM",	false,	"mpr-19802.41",	0xF2C3A7B7,	0x200000,	2,	30,		32,	false },

			// Sound ROMs
			{ "SndProg",	false,	"epr-20921.21",	0x30F032A7,	0x80000,	2,	0,		2,	true },
			{ "Samples",	false,	"mpr-20903.22",	0xE343E131,	0x400000,	2,	0x000000,	2,	false },
			{ "Samples",	false,	"mpr-20904.24",	0x21A91B84,	0x400000,	2,	0x400000,	2,	false },

			{ NULL, false, NULL, 0, 0, 0, 0, 0, false }
		}
	},

	// Virtua Striker 2 '99
	{
		"vs299",
		"Virtua Striker 2 '99",
		"Sega",
		1999,
		0x21,
		0x400000,	// 4 MB of fixed CROM
		true,		// 64 MB of banked CROM (Mirror)
		0x2000000,	// 32 MB of VROM
		0x800000,	// 8 MB of sample ROMs
		GAME_INPUT_COMMON|GAME_INPUT_JOYSTICK1|GAME_INPUT_JOYSTICK2|GAME_INPUT_SOCCER,
		0,			// no MPEG board
		false,		// no drive board

		{
			// Fixed CROM
			{ "CROM",	false,	"epr-21538.20",	0x02DF6AC8,	0x100000,	2,	0x0400000,	8,	true },
			{ "CROM",	false,	"epr-21537.19",	0xFB37DC16,	0x100000,	2,	0x0400002,	8,	true },
			{ "CROM",	false,	"epr-21536.18",	0x9AF2B0D5,	0x100000,	2,	0x0400004,	8,	true },
			{ "CROM",	false,	"epr-21535.17",	0x976A00BF,	0x100000,	2,	0x0400006,	8,	true },

			// Banked CROM0
			{ "CROMxx",	false,	"mpr-21500.4",	0x8C43964B,	0x400000,	2,	0x0000000,	8,	true },
			{ "CROMxx",	false,	"mpr-21499.3",	0x2CC4C1F1,	0x400000,	2,	0x0000002,	8,	true },
			{ "CROMxx",	false,	"mpr-21498.2",	0x4F53D6E0,	0x400000,	2,	0x0000004,	8,	true },
			{ "CROMxx",	false,	"mpr-21497.1",	0x8EA759A1,	0x400000,	2,	0x0000006,	8,	true },

			// Banked CROM1
			{ "CROMxx",	false,	"mpr-21504.8",	0x7AAE557E,	0x400000,	2,	0x1000000,	8,	true },
			{ "CROMxx",	false,	"mpr-21503.7",	0xC9E1DE6B,	0x400000,	2,	0x1000002,	8,	true },
			{ "CROMxx",	false,	"mpr-21502.6",	0x921486BE,	0x400000,	2,	0x1000004,	8,	true },
			{ "CROMxx",	false,	"mpr-21501.5",	0x08BC2185,	0x400000,	2,	0x1000006,	8,	true },

			// Banked CROM2
			{ "CROMxx",	false,	"mpr-21508.12",	0x2E8F798E,	0x400000,	2,	0x2000000,	8,	true },
			{ "CROMxx",	false,	"mpr-21507.11",	0x1D8EB68B,	0x400000,	2,	0x2000002,	8,	true },
			{ "CROMxx",	false,	"mpr-21506.10",	0x2C1477C7,	0x400000,	2,	0x2000004,	8,	true },
			{ "CROMxx",	false,	"mpr-21505.9",	0xE169FF72,	0x400000,	2,	0x2000006,	8,	true },

			// Banked CROM3
			{ "CROMxx",	false,	"mpr-21512.16",	0x7CB2B05C,	0x400000,	2,	0x3000000,	8,	true },
			{ "CROMxx",	false,	"mpr-21511.15",	0x5AD9660C,	0x400000,	2,	0x3000002,	8,	true },
			{ "CROMxx",	false,	"mpr-21510.14",	0xF47489A4,	0x400000,	2,	0x3000004,	8,	true },
			{ "CROMxx",	false,	"mpr-21509.13",	0x9A65E6B4,	0x400000,	2,	0x3000006,	8,	true },

			// Video ROM
			{ "VROM",	false,	"mpr-21515.26",	0x8CE9910B,	0x200000,	2,	0,		32,	false },
			{ "VROM",	false,	"mpr-21516.27",	0x8971A753,	0x200000,	2,	2,		32,	false },
			{ "VROM",	false,	"mpr-21517.28",	0x55A4533B,	0x200000,	2,	4,		32,	false },
			{ "VROM",	false,	"mpr-21518.29",	0x4134026C,	0x200000,	2,	6,		32,	false },
			{ "VROM",	false,	"mpr-21519.30",	0xEF6757DE,	0x200000,	2,	8,		32,	false },
			{ "VROM",	false,	"mpr-21520.31",	0xC53BE8CC,	0x200000,	2,	10,		32,	false },
			{ "VROM",	false,	"mpr-21521.32",	0xABB501DC,	0x200000,	2,	12,		32,	false },
			{ "VROM",	false,	"mpr-21522.33",	0xE3B79973,	0x200000,	2,	14,		32,	false },
			{ "VROM",	false,	"mpr-21523.34",	0xFE4D1EAC,	0x200000,	2,	16,		32,	false },
			{ "VROM",	false,	"mpr-21524.35",	0x8633B6E9,	0x200000,	2,	18,		32,	false },
			{ "VROM",	false,	"mpr-21525.36",	0x3C490167,	0x200000,	2,	20,		32,	false },
			{ "VROM",	false,	"mpr-21526.37",	0x5FE5F9B0,	0x200000,	2,	22,		32,	false },
			{ "VROM",	false,	"mpr-21527.38",	0x10D0FE7E,	0x200000,	2,	24,		32,	false },
			{ "VROM",	false,	"mpr-21528.39",	0x4E346A6C,	0x200000,	2,	26,		32,	false },
			{ "VROM",	false,	"mpr-21529.40",	0x9A731A00,	0x200000,	2,	28,		32,	false },
			{ "VROM",	false,	"mpr-21530.41",	0x78400D5E,	0x200000,	2,	30,		32,	false },

			// Sound ROMs
			{ "SndProg",	false,	"epr-21539.21",	0xA1D3E00E,	0x80000,	2,	0,		2,	true },
			{ "Samples",	false,	"mpr-21513.22",	0xCCA1CC00,	0x400000,	2,	0x000000,	2,	false },
			{ "Samples",	false,	"mpr-21514.24",	0x6CEDD292,	0x400000,	2,	0x400000,	2,	false },

			{ NULL, false, NULL, 0, 0, 0, 0, 0, false }
		}
	},

	// Virtua Striker 2 '99 (Revision A)
	{
		"vs299a",
		"Virtua Striker 2 '99 (Revision A)",
		"Sega",
		1999,
		0x21,
		0x400000,	// 4 MB of fixed CROM
		true,		// 64 MB of banked CROM (Mirror)
		0x2000000,	// 32 MB of VROM
		0x800000,	// 8 MB of sample ROMs
		GAME_INPUT_COMMON|GAME_INPUT_JOYSTICK1|GAME_INPUT_JOYSTICK2|GAME_INPUT_SOCCER,
		0,			// no MPEG board
		false,		// no drive board

		{
			// Fixed CROM
			{ "CROM",	false,	"epr-21538a.20",	0x42BEBA70,	0x100000,	2,	0x0400000,	8,	true },
			{ "CROM",	false,	"epr-21537a.19",	0xF72A8F2F,	0x100000,	2,	0x0400002,	8,	true },
			{ "CROM",	false,	"epr-21536a.18",	0x95D49D6E,	0x100000,	2,	0x0400004,	8,	true },
			{ "CROM",	false,	"epr-21535a.17",	0x8E4EC341,	0x100000,	2,	0x0400006,	8,	true },

			// Banked CROM0
			{ "CROMxx",	false,	"mpr-21500.4",	0x8C43964B,	0x400000,	2,	0x0000000,	8,	true },
			{ "CROMxx",	false,	"mpr-21499.3",	0x2CC4C1F1,	0x400000,	2,	0x0000002,	8,	true },
			{ "CROMxx",	false,	"mpr-21498.2",	0x4F53D6E0,	0x400000,	2,	0x0000004,	8,	true },
			{ "CROMxx",	false,	"mpr-21497.1",	0x8EA759A1,	0x400000,	2,	0x0000006,	8,	true },

			// Banked CROM1
			{ "CROMxx",	false,	"mpr-21504.8",	0x7AAE557E,	0x400000,	2,	0x1000000,	8,	true },
			{ "CROMxx",	false,	"mpr-21503.7",	0xC9E1DE6B,	0x400000,	2,	0x1000002,	8,	true },
			{ "CROMxx",	false,	"mpr-21502.6",	0x921486BE,	0x400000,	2,	0x1000004,	8,	true },
			{ "CROMxx",	false,	"mpr-21501.5",	0x08BC2185,	0x400000,	2,	0x1000006,	8,	true },

			// Banked CROM2
			{ "CROMxx",	false,	"mpr-21508.12",	0x2E8F798E,	0x400000,	2,	0x2000000,	8,	true },
			{ "CROMxx",	false,	"mpr-21507.11",	0x1D8EB68B,	0x400000,	2,	0x2000002,	8,	true },
			{ "CROMxx",	false,	"mpr-21506.10",	0x2C1477C7,	0x400000,	2,	0x2000004,	8,	true },
			{ "CROMxx",	false,	"mpr-21505.9",	0xE169FF72,	0x400000,	2,	0x2000006,	8,	true },

			// Banked CROM3
			{ "CROMxx",	false,	"mpr-21512.16",	0x7CB2B05C,	0x400000,	2,	0x3000000,	8,	true },
			{ "CROMxx",	false,	"mpr-21511.15",	0x5AD9660C,	0x400000,	2,	0x3000002,	8,	true },
			{ "CROMxx",	false,	"mpr-21510.14",	0xF47489A4,	0x400000,	2,	0x3000004,	8,	true },
			{ "CROMxx",	false,	"mpr-21509.13",	0x9A65E6B4,	0x400000,	2,	0x3000006,	8,	true },

			// Video ROM
			{ "VROM",	false,	"mpr-21515.26",	0x8CE9910B,	0x200000,	2,	0,		32,	false },
			{ "VROM",	false,	"mpr-21516.27",	0x8971A753,	0x200000,	2,	2,		32,	false },
			{ "VROM",	false,	"mpr-21517.28",	0x55A4533B,	0x200000,	2,	4,		32,	false },
			{ "VROM",	false,	"mpr-21518.29",	0x4134026C,	0x200000,	2,	6,		32,	false },
			{ "VROM",	false,	"mpr-21519.30",	0xEF6757DE,	0x200000,	2,	8,		32,	false },
			{ "VROM",	false,	"mpr-21520.31",	0xC53BE8CC,	0x200000,	2,	10,		32,	false },
			{ "VROM",	false,	"mpr-21521.32",	0xABB501DC,	0x200000,	2,	12,		32,	false },
			{ "VROM",	false,	"mpr-21522.33",	0xE3B79973,	0x200000,	2,	14,		32,	false },
			{ "VROM",	false,	"mpr-21523.34",	0xFE4D1EAC,	0x200000,	2,	16,		32,	false },
			{ "VROM",	false,	"mpr-21524.35",	0x8633B6E9,	0x200000,	2,	18,		32,	false },
			{ "VROM",	false,	"mpr-21525.36",	0x3C490167,	0x200000,	2,	20,		32,	false },
			{ "VROM",	false,	"mpr-21526.37",	0x5FE5F9B0,	0x200000,	2,	22,		32,	false },
			{ "VROM",	false,	"mpr-21527.38",	0x10D0FE7E,	0x200000,	2,	24,		32,	false },
			{ "VROM",	false,	"mpr-21528.39",	0x4E346A6C,	0x200000,	2,	26,		32,	false },
			{ "VROM",	false,	"mpr-21529.40",	0x9A731A00,	0x200000,	2,	28,		32,	false },
			{ "VROM",	false,	"mpr-21530.41",	0x78400D5E,	0x200000,	2,	30,		32,	false },

			// Sound ROMs
			{ "SndProg",	false,	"epr-21539.21",	0xA1D3E00E,	0x80000,	2,	0,		2,	true },
			{ "Samples",	false,	"mpr-21513.22",	0xCCA1CC00,	0x400000,	2,	0x000000,	2,	false },
			{ "Samples",	false,	"mpr-21514.24",	0x6CEDD292,	0x400000,	2,	0x400000,	2,	false },

			{ NULL, false, NULL, 0, 0, 0, 0, 0, false }
		}
	},

	// Virtua Striker 2 '99 (Revision B)
	{
		"vs299b",
		"Virtua Striker 2 '99 (Revision B)",
		"Sega",
		1999,
		0x21,
		0x400000,	// 4 MB of fixed CROM
		true,		// 64 MB of banked CROM (Mirror)
		0x2000000,	// 32 MB of VROM
		0x800000,	// 8 MB of sample ROMs
		GAME_INPUT_COMMON|GAME_INPUT_JOYSTICK1|GAME_INPUT_JOYSTICK2|GAME_INPUT_SOCCER,
		0,			// no MPEG board
		false,		// no drive board

		{
			// Fixed CROM
			{ "CROM",	false,	"epr-21553b.20",	0x4F280A56,	0x100000,	2,	0x0400000,	8,	true },
			{ "CROM",	false,	"epr-21552b.19",	0xDB31EAF6,	0x100000,	2,	0x0400002,	8,	true },
			{ "CROM",	false,	"epr-21551b.18",	0x0BBC40F7,	0x100000,	2,	0x0400004,	8,	true },
			{ "CROM",	false,	"epr-21550b.17",	0xC508E488,	0x100000,	2,	0x0400006,	8,	true },

			// Banked CROM0
			{ "CROMxx",	false,	"mpr-21500.4",	0x8C43964B,	0x400000,	2,	0x0000000,	8,	true },
			{ "CROMxx",	false,	"mpr-21499.3",	0x2CC4C1F1,	0x400000,	2,	0x0000002,	8,	true },
			{ "CROMxx",	false,	"mpr-21498.2",	0x4F53D6E0,	0x400000,	2,	0x0000004,	8,	true },
			{ "CROMxx",	false,	"mpr-21497.1",	0x8EA759A1,	0x400000,	2,	0x0000006,	8,	true },

			// Banked CROM1
			{ "CROMxx",	false,	"mpr-21504.8",	0x7AAE557E,	0x400000,	2,	0x1000000,	8,	true },
			{ "CROMxx",	false,	"mpr-21503.7",	0xC9E1DE6B,	0x400000,	2,	0x1000002,	8,	true },
			{ "CROMxx",	false,	"mpr-21502.6",	0x921486BE,	0x400000,	2,	0x1000004,	8,	true },
			{ "CROMxx",	false,	"mpr-21501.5",	0x08BC2185,	0x400000,	2,	0x1000006,	8,	true },

			// Banked CROM2
			{ "CROMxx",	false,	"mpr-21508.12",	0x2E8F798E,	0x400000,	2,	0x2000000,	8,	true },
			{ "CROMxx",	false,	"mpr-21507.11",	0x1D8EB68B,	0x400000,	2,	0x2000002,	8,	true },
			{ "CROMxx",	false,	"mpr-21506.10",	0x2C1477C7,	0x400000,	2,	0x2000004,	8,	true },
			{ "CROMxx",	false,	"mpr-21505.9",	0xE169FF72,	0x400000,	2,	0x2000006,	8,	true },

			// Banked CROM3
			{ "CROMxx",	false,	"mpr-21512.16",	0x7CB2B05C,	0x400000,	2,	0x3000000,	8,	true },
			{ "CROMxx",	false,	"mpr-21511.15",	0x5AD9660C,	0x400000,	2,	0x3000002,	8,	true },
			{ "CROMxx",	false,	"mpr-21510.14",	0xF47489A4,	0x400000,	2,	0x3000004,	8,	true },
			{ "CROMxx",	false,	"mpr-21509.13",	0x9A65E6B4,	0x400000,	2,	0x3000006,	8,	true },

			// Video ROM
			{ "VROM",	false,	"mpr-21515.26",	0x8CE9910B,	0x200000,	2,	0,		32,	false },
			{ "VROM",	false,	"mpr-21516.27",	0x8971A753,	0x200000,	2,	2,		32,	false },
			{ "VROM",	false,	"mpr-21517.28",	0x55A4533B,	0x200000,	2,	4,		32,	false },
			{ "VROM",	false,	"mpr-21518.29",	0x4134026C,	0x200000,	2,	6,		32,	false },
			{ "VROM",	false,	"mpr-21519.30",	0xEF6757DE,	0x200000,	2,	8,		32,	false },
			{ "VROM",	false,	"mpr-21520.31",	0xC53BE8CC,	0x200000,	2,	10,		32,	false },
			{ "VROM",	false,	"mpr-21521.32",	0xABB501DC,	0x200000,	2,	12,		32,	false },
			{ "VROM",	false,	"mpr-21522.33",	0xE3B79973,	0x200000,	2,	14,		32,	false },
			{ "VROM",	false,	"mpr-21523.34",	0xFE4D1EAC,	0x200000,	2,	16,		32,	false },
			{ "VROM",	false,	"mpr-21524.35",	0x8633B6E9,	0x200000,	2,	18,		32,	false },
			{ "VROM",	false,	"mpr-21525.36",	0x3C490167,	0x200000,	2,	20,		32,	false },
			{ "VROM",	false,	"mpr-21526.37",	0x5FE5F9B0,	0x200000,	2,	22,		32,	false },
			{ "VROM",	false,	"mpr-21527.38",	0x10D0FE7E,	0x200000,	2,	24,		32,	false },
			{ "VROM",	false,	"mpr-21528.39",	0x4E346A6C,	0x200000,	2,	26,		32,	false },
			{ "VROM",	false,	"mpr-21529.40",	0x9A731A00,	0x200000,	2,	28,		32,	false },
			{ "VROM",	false,	"mpr-21530.41",	0x78400D5E,	0x200000,	2,	30,		32,	false },

			// Sound ROMs
			{ "SndProg",	false,	"epr-21539.21",	0xA1D3E00E,	0x80000,	2,	0,		2,	true },
			{ "Samples",	false,	"mpr-21513.22",	0xCCA1CC00,	0x400000,	2,	0x000000,	2,	false },
			{ "Samples",	false,	"mpr-21514.24",	0x6CEDD292,	0x400000,	2,	0x400000,	2,	false },

			{ NULL, false, NULL, 0, 0, 0, 0, 0, false }
		}
	},

	// Virtua Striker 2 '99.1 (Revision B)
	{
		"vs2v991",
		"Virtua Striker 2 '99.1",
		"Sega",
		1999,
		0x21,
		0x400000,	// 4 MB of fixed CROM
		true,		// 64 MB of banked CROM (Mirror)
		0x2000000,	// 32 MB of VROM
		0x800000,	// 8 MB of sample ROMs
		GAME_INPUT_COMMON|GAME_INPUT_JOYSTICK1|GAME_INPUT_JOYSTICK2|GAME_INPUT_SOCCER,
		0,			// no MPEG board
		false,		// no drive board

		{
			// Fixed CROM
			{ "CROM",	false,	"epr-21538b.20",	0xB3F0CE2A,	0x100000,	2,	0x0400000,	8,	true },
			{ "CROM",	false,	"epr-21537b.19",	0xA8B3FA5C,	0x100000,	2,	0x0400002,	8,	true },
			{ "CROM",	false,	"epr-21536b.18",	0x1F2BD190,	0x100000,	2,	0x0400004,	8,	true },
			{ "CROM",	false,	"epr-21535b.17",	0x76C5FA8E,	0x100000,	2,	0x0400006,	8,	true },

			// Banked CROM0
			{ "CROMxx",	false,	"mpr-21500.4",	0x8C43964B,	0x400000,	2,	0x0000000,	8,	true },
			{ "CROMxx",	false,	"mpr-21499.3",	0x2CC4C1F1,	0x400000,	2,	0x0000002,	8,	true },
			{ "CROMxx",	false,	"mpr-21498.2",	0x4F53D6E0,	0x400000,	2,	0x0000004,	8,	true },
			{ "CROMxx",	false,	"mpr-21497.1",	0x8EA759A1,	0x400000,	2,	0x0000006,	8,	true },

			// Banked CROM1
			{ "CROMxx",	false,	"mpr-21504.8",	0x7AAE557E,	0x400000,	2,	0x1000000,	8,	true },
			{ "CROMxx",	false,	"mpr-21503.7",	0xC9E1DE6B,	0x400000,	2,	0x1000002,	8,	true },
			{ "CROMxx",	false,	"mpr-21502.6",	0x921486BE,	0x400000,	2,	0x1000004,	8,	true },
			{ "CROMxx",	false,	"mpr-21501.5",	0x08BC2185,	0x400000,	2,	0x1000006,	8,	true },

			// Banked CROM2
			{ "CROMxx",	false,	"mpr-21508.12",	0x2E8F798E,	0x400000,	2,	0x2000000,	8,	true },
			{ "CROMxx",	false,	"mpr-21507.11",	0x1D8EB68B,	0x400000,	2,	0x2000002,	8,	true },
			{ "CROMxx",	false,	"mpr-21506.10",	0x2C1477C7,	0x400000,	2,	0x2000004,	8,	true },
			{ "CROMxx",	false,	"mpr-21505.9",	0xE169FF72,	0x400000,	2,	0x2000006,	8,	true },

			// Banked CROM3
			{ "CROMxx",	false,	"mpr-21512.16",	0x7CB2B05C,	0x400000,	2,	0x3000000,	8,	true },
			{ "CROMxx",	false,	"mpr-21511.15",	0x5AD9660C,	0x400000,	2,	0x3000002,	8,	true },
			{ "CROMxx",	false,	"mpr-21510.14",	0xF47489A4,	0x400000,	2,	0x3000004,	8,	true },
			{ "CROMxx",	false,	"mpr-21509.13",	0x9A65E6B4,	0x400000,	2,	0x3000006,	8,	true },

			// Video ROM
			{ "VROM",	false,	"mpr-21515.26",	0x8CE9910B,	0x200000,	2,	0,		32,	false },
			{ "VROM",	false,	"mpr-21516.27",	0x8971A753,	0x200000,	2,	2,		32,	false },
			{ "VROM",	false,	"mpr-21517.28",	0x55A4533B,	0x200000,	2,	4,		32,	false },
			{ "VROM",	false,	"mpr-21518.29",	0x4134026C,	0x200000,	2,	6,		32,	false },
			{ "VROM",	false,	"mpr-21519.30",	0xEF6757DE,	0x200000,	2,	8,		32,	false },
			{ "VROM",	false,	"mpr-21520.31",	0xC53BE8CC,	0x200000,	2,	10,		32,	false },
			{ "VROM",	false,	"mpr-21521.32",	0xABB501DC,	0x200000,	2,	12,		32,	false },
			{ "VROM",	false,	"mpr-21522.33",	0xE3B79973,	0x200000,	2,	14,		32,	false },
			{ "VROM",	false,	"mpr-21523.34",	0xFE4D1EAC,	0x200000,	2,	16,		32,	false },
			{ "VROM",	false,	"mpr-21524.35",	0x8633B6E9,	0x200000,	2,	18,		32,	false },
			{ "VROM",	false,	"mpr-21525.36",	0x3C490167,	0x200000,	2,	20,		32,	false },
			{ "VROM",	false,	"mpr-21526.37",	0x5FE5F9B0,	0x200000,	2,	22,		32,	false },
			{ "VROM",	false,	"mpr-21527.38",	0x10D0FE7E,	0x200000,	2,	24,		32,	false },
			{ "VROM",	false,	"mpr-21528.39",	0x4E346A6C,	0x200000,	2,	26,		32,	false },
			{ "VROM",	false,	"mpr-21529.40",	0x9A731A00,	0x200000,	2,	28,		32,	false },
			{ "VROM",	false,	"mpr-21530.41",	0x78400D5E,	0x200000,	2,	30,		32,	false },

			// Sound ROMs
			{ "SndProg",	false,	"epr-21539.21",	0xA1D3E00E,	0x80000,	2,	0,		2,	true },
			{ "Samples",	false,	"mpr-21513.22",	0xCCA1CC00,	0x400000,	2,	0x000000,	2,	false },
			{ "Samples",	false,	"mpr-21514.24",	0x6CEDD292,	0x400000,	2,	0x400000,	2,	false },

			{ NULL, false, NULL, 0, 0, 0, 0, 0, false }
		}
	},

	// Terminate list
	{
		"",
		NULL,
		NULL,
		0,
		0,
		0,
		0,
		0,
		0,
		0,
		0,
		false,

		{
			{ NULL, false, NULL, 0, 0, 0, 0, 0, false },
			{ NULL, false, NULL, 0, 0, 0, 0, 0, false },
			{ NULL, false, NULL, 0, 0, 0, 0, 0, false },
			{ NULL, false, NULL, 0, 0, 0, 0, 0, false }
		}
	}
};
