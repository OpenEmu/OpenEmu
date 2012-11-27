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

/* Detection tables for the ADI / Addy 2 series. */

#ifndef GOB_DETECTION_TABLES_ADI2_H
#define GOB_DETECTION_TABLES_ADI2_H

// -- French: Adi --

{
	{
		"adi2",
		"Adi 2.0 for Teachers",
		AD_ENTRY1s("adi2.stk", "da6f1fb68bff32260c5eecdf9286a2f5", 1533168),
		FR_FRA,
		kPlatformPC,
		ADGF_NO_FLAGS,
		GUIO0()
	},
	kGameTypeAdi2,
	kFeaturesNone,
	"adi2.stk", "ediintro.tot", 0
},
{ // Found in french ADI 2 Francais-Maths CM1. Exact version not specified.
	{
		"adi2",
		"Adi 2",
		AD_ENTRY1s("adi2.stk", "23f279615c736dc38320f1348e70c36e", 10817668),
		FR_FRA,
		kPlatformPC,
		ADGF_NO_FLAGS,
		GUIO1(GUIO_NOASPECT)
	},
	kGameTypeAdi2,
	kFeatures640x480,
	"adi2.stk", "ediintro.tot", 0
},
{ // Found in french ADI 2 Francais-Maths CE2. Exact version not specified.
	{
		"adi2",
		"Adi 2",
		AD_ENTRY1s("adi2.stk", "d4162c4298f9423ecc1fb04965557e90", 11531214),
		FR_FRA,
		kPlatformPC,
		ADGF_NO_FLAGS,
		GUIO1(GUIO_NOASPECT)
	},
	kGameTypeAdi2,
	kFeatures640x480,
	"adi2.stk", "ediintro.tot", 0
},
{
	{
		"adi2",
		"Adi 2.5",
		AD_ENTRY1s("adi2.stk", "fcac60e6627f37aee219575b60859de9", 16944268),
		FR_FRA,
		kPlatformPC,
		ADGF_NO_FLAGS,
		GUIO1(GUIO_NOASPECT)
	},
	kGameTypeAdi2,
	kFeatures640x480,
	"adi2.stk", "ediintro.tot", 0
},
{
	{
		"adi2",
		"Adi 2.5",
		AD_ENTRY1s("adi2.stk", "072d5e2d7826a7c055865568ebf918bb", 16934596),
		FR_FRA,
		kPlatformPC,
		ADGF_NO_FLAGS,
		GUIO1(GUIO_NOASPECT)
	},
	kGameTypeAdi2,
	kFeatures640x480,
	"adi2.stk", "ediintro.tot", 0
},
{
	{
		"adi2",
		"Adi 2.6",
		AD_ENTRY1s("adi2.stk", "2fb940eb8105b12871f6b88c8c4d1615", 16780058),
		FR_FRA,
		kPlatformPC,
		ADGF_NO_FLAGS,
		GUIO1(GUIO_NOASPECT)
	},
	kGameTypeAdi2,
	kFeatures640x480,
	"adi2.stk", "ediintro.tot", 0
},

// -- German: Addy --

{
	{
		"adi2",
		"Adi 2.6",
		AD_ENTRY1s("adi2.stk", "fde7d98a67dbf859423b6473796e932a", 18044780),
		DE_DEU,
		kPlatformPC,
		ADGF_NO_FLAGS,
		GUIO1(GUIO_NOASPECT)
	},
	kGameTypeAdi2,
	kFeatures640x480,
	"adi2.stk", "ediintro.tot", 0
},
{
	{
		"adi2",
		"Adi 2.7.1",
		AD_ENTRY1s("adi2.stk", "6fa5dffebf5c7243c6af6b8c188ee00a", 19278008),
		FR_FRA,
		kPlatformPC,
		ADGF_NO_FLAGS,
		GUIO1(GUIO_NOASPECT)
	},
	kGameTypeAdi2,
	kFeatures640x480,
	"adi2.stk", "ediintro.tot", 0
},

// -- Spanish: Adi --

{
	{
		"adi2",
		"Adi 2",
		AD_ENTRY1s("adi2.stk", "2a40bb48ccbd4e6fb3f7f0fc2f069d80", 17720132),
		ES_ESP,
		kPlatformPC,
		ADGF_NO_FLAGS,
		GUIO1(GUIO_NOASPECT)
	},
	kGameTypeAdi2,
	kFeatures640x480,
	"adi2.stk", "ediintro.tot", 0
},

// -- English: ADI (Amiga) --

{
	{
		"adi2",
		"Adi 2",
		AD_ENTRY1s("adi2.stk", "29694c5a649298a42f87ae731d6d6f6d", 311132),
		EN_ANY,
		kPlatformAmiga,
		ADGF_NO_FLAGS,
		GUIO0()
	},
	kGameTypeAdi2,
	kFeaturesNone,
	"adi2.stk", "ediintro.tot", 0
},

// -- Demos --

{
	{
		"adi2",
		"Non-Interactive Demo",
		{
			{"demo.scn", 0, "8b5ba359fd87d586ad39c1754bf6ea35", 168},
			{"demadi2t.vmd", 0, "08a1b18cfe2015d3b43270da35cc813d", 7250723},
			{"demarch.vmd", 0, "4c4a4616585d40ef3df209e3c3911062", 5622731},
			{"demobou.vmd", 0, "2208b9855775564d15c4a5a559da0aec", 3550511},
			{0, 0, 0, 0}
		},
		EN_ANY,
		kPlatformPC,
		ADGF_NO_FLAGS,
		GUIO3(GUIO_NOSUBTITLES, GUIO_NOSPEECH, GUIO_NOASPECT)
	},
	kGameTypeAdi2,
	kFeatures640x480 | kFeaturesSCNDemo,
	0, 0, 1
},

#endif // GOB_DETECTION_TABLES_ADI2_H
