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
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	 See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

namespace TsAGE {

static const tSageGameDescription gameDescriptions[] = {

	// Ringworld English CD and First Wave versions
	{
		{
			"ringworld",
			"CD",
			AD_ENTRY1s("ring.rlb", "466f0e6492d9d0f34d35c5cd088de90f", 37847618),
			Common::EN_ANY,
			Common::kPlatformPC,
			ADGF_CD,
			GUIO2(GUIO_NOSPEECH, GUIO_NOSFX)
		},
		GType_Ringworld,
		GF_CD | GF_ALT_REGIONS
	},
	// Ringworld Spanish CD
	{
		{
			"ringworld",
			"CD",
			AD_ENTRY1s("ring.rlb", "cb8bba91b30cd172712371d7123bd763", 7427980),
			Common::ES_ESP,
			Common::kPlatformPC,
			ADGF_CD,
			GUIO2(GUIO_NOSPEECH, GUIO_NOSFX)
		},
		GType_Ringworld,
		GF_CD | GF_ALT_REGIONS
	},
	// Ringworld English Floppy version
	{
		{
			"ringworld",
			"Floppy",
			AD_ENTRY1s("ring.rlb", "7b7f0c5b37b58fa5ec06ebb2ca0d0d9d", 8438770),
			Common::EN_ANY,
			Common::kPlatformPC,
			ADGF_NO_FLAGS,
			GUIO2(GUIO_NOSPEECH, GUIO_NOSFX)
		},
		GType_Ringworld,
		GF_FLOPPY
	},
	// Ringworld English Floppy Demo #1 version
	{
		{
			"ringworld",
			"Floppy Demo",
			AD_ENTRY1s("tsage.rlb", "3b3604a97c06c91f3735d3e9d341f63f", 833453),
			Common::EN_ANY,
			Common::kPlatformPC,
			ADGF_DEMO,
			GUIO2(GUIO_NOSPEECH, GUIO_NOSFX)
		},
		GType_Ringworld,
		GF_FLOPPY | GF_DEMO
	},

	// Ringworld English Floppy Demo #2 version
	{
		{
			"ringworld",
			"Floppy Demo",
			AD_ENTRY1s("demoring.rlb", "64050e1806203b15bb03876140eb4f56", 832206),
			Common::EN_ANY,
			Common::kPlatformPC,
			ADGF_DEMO,
			GUIO2(GUIO_NOSPEECH, GUIO_NOSFX)
		},
		GType_Ringworld,
		GF_FLOPPY | GF_DEMO | GF_ALT_REGIONS
	},
#if 0
	// FIXME: Compute new MD5s based on 5000 bytes instead of 0 (unlimited)
	// Blue Force floppy
	{
		{
			"blueforce",
			"Floppy",
			AD_ENTRY1s("blue.rlb", "17c3993415e8a2cf93040eef7e88ec93", 1156508),
			Common::EN_ANY,
			Common::kPlatformPC,
			ADGF_UNSTABLE,
			GUIO2(GUIO_NOSPEECH, GUIO_NOSFX)
		},
		GType_BlueForce,
		GF_FLOPPY
	},
#endif
	// Blue Force
	{
		{
			"blueforce",
			"",
			AD_ENTRY1s("blue.rlb", "17eabb456cb1546c66baf1aff387ba6a", 10032614),
			Common::EN_ANY,
			Common::kPlatformPC,
			ADGF_NO_FLAGS,
			GUIO2(GUIO_NOSPEECH, GUIO_NOSFX)
		},
		GType_BlueForce,
		GF_FLOPPY | GF_ALT_REGIONS
	},
	// Blue Force CD and First Wave use the same files
	{
		{
			"blueforce",
			"CD",
			AD_ENTRY1s("blue.rlb", "99983f48cb218f1f3760cf2f9a7ef11d", 63863322),
			Common::EN_ANY,
			Common::kPlatformPC,
			ADGF_CD,
			GUIO2(GUIO_NOSPEECH, GUIO_NOSFX)
		},
		GType_BlueForce,
		GF_CD | GF_ALT_REGIONS
	},
#if 0
	// Blue Force Spanish doesn't yet work
	// Blue Force Spanish CD
	{
		{
			"blueforce",
			"CD",
			AD_ENTRY1s("blue.rlb", "5b2b35c51b62e82d82b0791540bfae2d", 10082565),
			Common::ES_ESP,
			Common::kPlatformPC,
			ADGF_CD | ADGF_UNSTABLE,
			GUIO2(GUIO_NOSPEECH, GUIO_NOSFX)
		},
		GType_BlueForce,
		GF_CD | GF_ALT_REGIONS
	},
#endif
	// Return to Ringworld
	{
		{
			"ringworld2",
			"CD",
			AD_ENTRY1s("r2rw.rlb", "df6c25622387007788ca36d99362c1f0", 47586928),
			Common::EN_ANY,
			Common::kPlatformPC,
			ADGF_CD | ADGF_UNSTABLE,
			GUIO0()
		},
		GType_Ringworld2,
		GF_CD | GF_ALT_REGIONS
	},

	{ AD_TABLE_END_MARKER, 0, 0 }
};

} // End of namespace TsAGE
