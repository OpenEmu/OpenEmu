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

#ifndef DREAMWEB_DETECTION_TABLES_H
#define DREAMWEB_DETECTION_TABLES_H

namespace DreamWeb {

#define GAMEOPTION_ORIGINAL_SAVELOAD GUIO_GAMEOPTIONS1
#define GAMEOPTION_BRIGHTPALETTE     GUIO_GAMEOPTIONS2

struct DreamWebGameDescription {
	ADGameDescription desc;
};

static const DreamWebGameDescription gameDescriptions[] = {
	// International floppy release
	{
		{
			"dreamweb",
			"",
			{
				{"dreamweb.r00", 0, "3b5c87717fc40cc5a5ae19c155662ee3", 152918},
				{"dreamweb.r02", 0, "28458718167a040d7e988cf7d2298eae", 210466},
				{"dreamweb.exe", 0, "56b1d73aa56e964b45872ff552402341", 64985},
				AD_LISTEND
			},
			Common::EN_ANY,
			Common::kPlatformPC,
			0,
			GUIO2(GAMEOPTION_ORIGINAL_SAVELOAD, GAMEOPTION_BRIGHTPALETTE)
		},
	},

	// International CD release
	{
		{
			"dreamweb",
			"CD",
			{
				{"dreamweb.r00", 0, "3b5c87717fc40cc5a5ae19c155662ee3", 152918},
				{"dreamweb.r02", 0, "d6fe5e3590ec1eea42ff65c10b023e0f", 198681},
				AD_LISTEND
			},
			Common::EN_ANY,
			Common::kPlatformPC,
			ADGF_CD,
			GUIO2(GAMEOPTION_ORIGINAL_SAVELOAD, GAMEOPTION_BRIGHTPALETTE)
		},
	},

	// UK-V (Early UK) CD Release - From bug #3526483
	// Note: r00 and r02 files are identical to international floppy release
	//       so was misidentified as floppy, resulting in disabled CD speech.
	//       Added executable to detection to avoid this.
	{
		{
			"dreamweb",
			"CD",
			{
				{"dreamweb.r00", 0, "3b5c87717fc40cc5a5ae19c155662ee3", 152918},
				{"dreamweb.r02", 0, "28458718167a040d7e988cf7d2298eae", 210466},
				{"dreamweb.exe", 0, "dd1c7793b151489e67b83cd1ecab51cd", -1},
				AD_LISTEND
			},
			Common::EN_GRB,
			Common::kPlatformPC,
			ADGF_CD,
			GUIO2(GAMEOPTION_ORIGINAL_SAVELOAD, GAMEOPTION_BRIGHTPALETTE)
		},
	},

	// US CD release
	{
		{
			"dreamweb",
			"CD",
			{
				{"dreamweb.r00", 0, "8acafd7f4418d08d0e16b65b8b10bc50", 152983},
				{"dreamweb.r02", 0, "c0c363715ddf14ab54f2379906a3aa01", 198707},
				AD_LISTEND
			},
			Common::EN_USA,
			Common::kPlatformPC,
			ADGF_CD,
			GUIO2(GAMEOPTION_ORIGINAL_SAVELOAD, GAMEOPTION_BRIGHTPALETTE)
		},
	},

	// French CD release
	{
		{
			"dreamweb",
			"CD",
			{
				{"dreamwfr.r00", 0, "e354582a8564faf5c515df92f207e8d1", 154657},
				{"dreamwfr.r02", 0, "57f3f08d5aefd04184eac76927eced80", 200575},
				AD_LISTEND
			},
			Common::FR_FRA,
			Common::kPlatformPC,
			ADGF_CD,
			GUIO2(GAMEOPTION_ORIGINAL_SAVELOAD, GAMEOPTION_BRIGHTPALETTE)
		},
	},

	// French CD release
	// From bug #3524362
	{
		{
			"dreamweb",
			"CD",
			{
				{"dreamwfr.r00", 0, "e354582a8564faf5c515df92f207e8d1", 154657},
				{"dreamwfr.r02", 0, "cb99f08d5aefd04184eac76927eced80", 200575},
				AD_LISTEND
			},
			Common::FR_FRA,
			Common::kPlatformPC,
			ADGF_CD,
			GUIO2(GAMEOPTION_ORIGINAL_SAVELOAD, GAMEOPTION_BRIGHTPALETTE)
		},
	},

	// German floppy release
	{
		{
			"dreamweb",
			"",
			{
				{"dreamweb.r00", 0, "9960dc3baddabc6ad2a6fd75292b149c", 155886},
				{"dreamweb.r02", 0, "48e1f42a53402f963ca2d1ed969f4084", 212823},
				AD_LISTEND
			},
			Common::DE_DEU,
			Common::kPlatformPC,
			0,
			GUIO2(GAMEOPTION_ORIGINAL_SAVELOAD, GAMEOPTION_BRIGHTPALETTE)
		},
	},

	// German CD release
	{
		{
			"dreamweb",
			"CD",
			{
				{"dreamweb.r00", 0, "9960dc3baddabc6ad2a6fd75292b149c", 155886},
				{"dreamweb.r02", 0, "076ca7cd326cb2abfb2091c6cf46ae08", 201038},
				AD_LISTEND
			},
			Common::DE_DEU,
			Common::kPlatformPC,
			ADGF_CD,
			GUIO2(GAMEOPTION_ORIGINAL_SAVELOAD, GAMEOPTION_BRIGHTPALETTE)
		},
	},

	// Spanish floppy release
	{
		{
			"dreamweb",
			"",
			{
				{"dreamweb.r00", 0, "2df07174321de39c4f17c9ff654b268a", 153608},
				{"dreamweb.r02", 0, "f97d435ad5da08fb1bcf6ea3dd6e0b9e", 199499},
				AD_LISTEND
			},
			Common::ES_ESP,
			Common::kPlatformPC,
			0,
			GUIO2(GAMEOPTION_ORIGINAL_SAVELOAD, GAMEOPTION_BRIGHTPALETTE)
		},
	},

	// Spanish CD release
	{
		{
			"dreamweb",
			"CD",
			{
				{"dreamwsp.r00", 0, "2df07174321de39c4f17c9ff654b268a", 153608},
				{"dreamwsp.r02", 0, "577d435ad5da08fb1bcf6ea3dd6e0b9e", 199499},
				AD_LISTEND
			},
			Common::ES_ESP,
			Common::kPlatformPC,
			ADGF_CD,
			GUIO2(GAMEOPTION_ORIGINAL_SAVELOAD, GAMEOPTION_BRIGHTPALETTE)
		},
	},

	// Spanish CD release
	// From bug #3524362
	{
		{
			"dreamweb",
			"CD",
			{
				{"dreamwsp.r00", 0, "2df07174321de39c4f17c9ff654b268a", 153608},
				{"dreamwsp.r02", 0, "f97d435ad5da08fb1bcf6ea3dd6e0b9e", 199499},
				AD_LISTEND
			},
			Common::ES_ESP,
			Common::kPlatformPC,
			ADGF_CD,
			GUIO2(GAMEOPTION_ORIGINAL_SAVELOAD, GAMEOPTION_BRIGHTPALETTE)
		},
	},

	// Italian floppy release
	{
		{
			"dreamweb",
			"",
			{
				{"dreamweb.r00", 0, "66dcab08354232f423c590156335f819", 155448},
				{"dreamweb.r02", 0, "87a026e9f80ed4f94169381f871ee305", 199676},
				AD_LISTEND
			},
			Common::IT_ITA,
			Common::kPlatformPC,
			0,
			GUIO2(GAMEOPTION_ORIGINAL_SAVELOAD, GAMEOPTION_BRIGHTPALETTE)
		},
	},

	{ AD_TABLE_END_MARKER }
};

} // End of namespace DreamWeb

#endif
