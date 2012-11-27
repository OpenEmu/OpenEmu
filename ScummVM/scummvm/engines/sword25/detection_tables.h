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

namespace Sword25 {

static const ADGameDescription gameDescriptions[] = {
	{
		"sword25",
		"",
		AD_ENTRY1s("data.b25c", "f8b6e03ada2d2f6cf27fbc11ad1572e9", 654310588),
		Common::EN_ANY,
		Common::kPlatformUnknown,
		ADGF_UNSTABLE,
		GUIO1(GUIO_NOASPECT)
	},
	{
		"sword25",
		"",
		AD_ENTRY1s("lang_fr.b25c", "690caf157387e06d2c3d1ca53c43f428", 1006043),
		Common::FR_FRA,
		Common::kPlatformUnknown,
		ADGF_UNSTABLE,
		GUIO1(GUIO_NOASPECT)
	},
	{
		"sword25",
		"",
		AD_ENTRY1s("data.b25c", "f8b6e03ada2d2f6cf27fbc11ad1572e9", 654310588),
		Common::DE_DEU,
		Common::kPlatformUnknown,
		ADGF_UNSTABLE,
		GUIO1(GUIO_NOASPECT)
	},
	{
		"sword25",
		"",
		AD_ENTRY1s("lang_hr.b25c", "e881054d1f8ec1e527422fc521c25405", 1273217),
		Common::HR_HRV,
		Common::kPlatformUnknown,
		ADGF_UNSTABLE,
		GUIO1(GUIO_NOASPECT)
	},
	{
		"sword25",
		"",
		AD_ENTRY1s("lang_it.b25c", "f3325666da0515cc2b42062e953c0889", 996197),
		Common::IT_ITA,
		Common::kPlatformUnknown,
		ADGF_UNSTABLE,
		GUIO1(GUIO_NOASPECT)
	},
	{
		"sword25",
		"",
		AD_ENTRY1s("lang_pl.b25c", "49dc1a20f95391a808e475c49be2bac0", 1281799),
		Common::PL_POL,
		Common::kPlatformUnknown,
		ADGF_UNSTABLE,
		GUIO1(GUIO_NOASPECT)
	},
	{
		"sword25",
		"",
		AD_ENTRY1s("lang_pt.b25c", "1df701432f9e13dcefe1adeb890b9c69", 993812),
		Common::PT_BRA,
		Common::kPlatformUnknown,
		ADGF_UNSTABLE,
		GUIO1(GUIO_NOASPECT)
	},
	{
		"sword25",
		"",
		AD_ENTRY1s("lang_ru.b25c", "deb33dd2f90a71ff60181918a8ce5063", 1235378),
		Common::RU_RUS,
		Common::kPlatformUnknown,
		ADGF_UNSTABLE,
		GUIO1(GUIO_NOASPECT)
	},
	{
		"sword25",
		"",
		AD_ENTRY1s("lang_es.b25c", "384c19072d83725f351bb9ecb4d3f02b", 987965),
		Common::ES_ESP,
		Common::kPlatformUnknown,
		ADGF_UNSTABLE,
		GUIO1(GUIO_NOASPECT)
	},
	// Hungarian "psylog" version.
	// Submitted by goodoldgeorg in bug report #3428644.
	{
		"sword25",
		"psylog version",
		AD_ENTRY1s("lang_hu.b25c", "7de51a3b4926a192549e75b1a7d81667", 1864915),
		Common::HU_HUN,
		Common::kPlatformUnknown,
		ADGF_UNSTABLE,
		GUIO1(GUIO_NOASPECT)
	},

	// Extracted version
	{
		"sword25",
		"Extracted",
		{{"_includes.lua", 0, 0, -1},
		 {"boot.lua", 0, 0, -1},
		 {"kernel.lua", 0, 0, -1},
		 AD_LISTEND},
		Common::EN_ANY,
		Common::kPlatformUnknown,
		GF_EXTRACTED | ADGF_UNSTABLE,
		GUIO1(GUIO_NOASPECT)
	},
	AD_TABLE_END_MARKER
};

} // End of namespace Sword25
