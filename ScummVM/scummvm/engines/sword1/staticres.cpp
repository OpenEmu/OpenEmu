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


#include "sword1/sworddefs.h"
#include "sword1/swordres.h"
#include "sword1/screen.h"
#include "sword1/resman.h"
#include "sword1/objectman.h"
#include "sword1/menu.h"
#include "sword1/music.h"
#include "sword1/sound.h"
#include "sword1/sword1.h"
#include "sword1/logic.h"

namespace Sword1 {

const uint8 SwordEngine::_cdList[TOTAL_SECTIONS] = {
	0,		// 0		inventory

	1,		// 1		PARIS 1
	1,		// 2
	1,		// 3
	1,		// 4
	1,		// 5
	1,		// 6
	1,		// 7
	1,		// 8

	1,		// 9		PARIS 2
	1,		// 10
	1,		// 11
	1,		// 12
	1,		// 13
	1,		// 14
	1,		// 15
	1,		// 16
	1,		// 17
	1,		// 18

	2,		// 19		IRELAND
	2,		// 20
	2,		// 21
	2,		// 22
	2,		// 23
	2,		// 24
	2,		// 25
	2,		// 26

	1,		// 27		PARIS 3
	1,		// 28
	1,		// 29
	1,		// 30 - Heart Monitor
	1,		// 31
	1,		// 32
	1,		// 33
	1,		// 34
	1,		// 35

	1,		// 36		PARIS 4
	1,		// 37
	1,		// 38
	1,		// 39
	1,		// 40
	1,		// 41
	1,		// 42
	1,		// 43
	0,		// 44	<NOT USED>

	2,		// 45		SYRIA
	1,		// 46		PARIS 4
	2,		// 47
	1,		// 48		PARIS 4
	2,		// 49
	2,		// 50
	0,		// 51 <NOT USED>
	0,		// 52 <NOT USED>
	2,		// 53
	2,		// 54
	2,		// 55

	2,		// 56		SPAIN
	2,		// 57
	2,		// 58
	2,		// 59
	2,		// 60
	2,		// 61
	2,		// 62

	2,		// 63		NIGHT TRAIN
	0,		// 64 <NOT USED>
	2,		// 65
	2,		// 66
	2,		// 67
	0,		// 68 <NOT USED>
	2,		// 69
	0,		// 70 <NOT USED>

	2,		// 71		SCOTLAND
	2,		// 72
	2,		// 73
	2,		// 74		END SEQUENCE IN SECRET_CRYPT
	2,		// 75
	2,		// 76
	2,		// 77
	2,		// 78
	2,		// 79

	1,		// 80		PARIS MAP

	1,		// 81	Full-screen for "Asstair" in Paris2

	2,		// 82	Full-screen BRITMAP in sc55 (Syrian Cave)
	0,		// 83 <NOT USED>
	0,		// 84 <NOT USED>
	0,		// 85 <NOT USED>

	1,		// 86		EUROPE MAP
	1,		// 87		fudged in for normal window (sc48)
	1,		// 88		fudged in for filtered window (sc48)
	0,		// 89 <NOT USED>

	0,		// 90		PHONE SCREEN
	0,		// 91		ENVELOPE SCREEN
	1,		// 92		fudged in for George close-up surprised in sc17 wardrobe
	1,		// 93		fudged in for George close-up inquisitive in sc17 wardrobe
	1,		// 94		fudged in for George close-up in sc29 sarcophagus
	1,		// 95		fudged in for George close-up in sc29 sarcophagus
	1,		// 96		fudged in for chalice close-up from sc42
	0,		// 97 <NOT USED>
	0,		// 98 <NOT USED>
	0,		// 99		MESSAGE SCREEN (BLANK)

	0,		// 100
	0,		// 101
	0,		// 102
	0,		// 103
	0,		// 104
	0,		// 105
	0,		// 106
	0,		// 107
	0,		// 108
	0,		// 109

	0,		// 110
	0,		// 111
	0,		// 112
	0,		// 113
	0,		// 114
	0,		// 115
	0,		// 116
	0,		// 117
	0,		// 118
	0,		// 119

	0,		// 120
	0,		// 121
	0,		// 122
	0,		// 123
	0,		// 124
	0,		// 125
	0,		// 126
	0,		// 127
	0,		// 128  GEORGE'S GAME SECTION
	0,		// 129	NICO'S TEXT		- on both CD's

	0,		// 130
	1,		// 131	BENOIR'S TEXT - on CD1
	0,		// 132
	1,		// 133	ROSSO'S TEXT	- on CD1
	0,		// 134
	0,		// 135
	0,		// 136
	0,		// 137
	0,		// 138
	0,		// 139

	0,		// 140
	0,		// 141
	0,		// 142
	0,		// 143
	0,		// 144
	1,		// 145	MOUE'S TEXT		- on CD1
	1,		// 146	ALBERT'S TEXT	- on CD1
};

const MenuObject Menu::_objectDefs[TOTAL_pockets + 1] = {
	{	// 0 can't use
		0, 0, 0, 0, 0
	},
	{	// 1 NEWSPAPER
		menu_newspaper,							// text_desc
		ICON_NEWSPAPER,							// big_icon_res
		0,													// big_icon_frame
		LUGG_NEWSPAPER,							// luggage_icon_res
		SCR_icon_combine_script,		// use_script
	},
	{	// 2 HAZEL_WAND
		menu_hazel_wand,						// text_desc
		ICON_HAZEL_WAND,						// big_icon_res
		0,													// big_icon_frame
		LUGG_HAZEL_WAND,						// luggage_icon_res
		SCR_icon_combine_script,		// use_script
	},
	{	// 3 BEER_TOWEL
		0,													// text_desc - SEE MENU.SCR
		ICON_BEER_TOWEL,						// big_icon_res
		0,													// big_icon_frame
		LUGG_BEER_TOWEL,						// luggage_icon_res
		SCR_icon_combine_script,		// use_script
	},
	{	// 4 HOTEL_KEY
		menu_hotel_key,							// text_desc
		ICON_HOTEL_KEY,							// big_icon_res
		0,													// big_icon_frame
		LUGG_HOTEL_KEY,							// luggage_icon_res
		SCR_icon_combine_script,		// use_script
	},
	{	// 5 BALL
		menu_ball,									// text_desc
		ICON_BALL,									// big_icon_res
		0,													// big_icon_frame
		LUGG_BALL,									// luggage_icon_res
		SCR_icon_combine_script,		// use_script
	},
	{	// 6 STATUETTE
		menu_statuette,							// text_desc
		ICON_STATUETTE,							// big_icon_res
		0,													// big_icon_frame
		LUGG_STATUETTE,							// luggage_icon_res
		SCR_icon_combine_script,		// use_script
	},
	{	// 7 RED_NOSE
		0,													// text_desc - SEE MENU.SCR
		ICON_RED_NOSE,							// big_icon_res
		0,													// big_icon_frame
		LUGG_RED_NOSE,							// luggage_icon_res
		SCR_icon_combine_script,			// use_script
	},
	{	// 8 POLISHED_CHALICE
		menu_polished_chalice,			// text_desc
		ICON_POLISHED_CHALICE,			// big_icon_res
		0,													// big_icon_frame
		LUGG_POLISHED_CHALICE,			// luggage_icon_res
		SCR_icon_combine_script,			// use_script
	},
	{	// 9 DOLLAR_BILL
		menu_dollar_bill,						// text_desc
		ICON_DOLLAR_BILL,						// big_icon_res
		0,													// big_icon_frame
		LUGG_DOLLAR_BILL,						// luggage_icon_res
		SCR_icon_combine_script,			// use_script
	},
	{	// 10 PHOTO
		menu_photograph,						// text_desc
		ICON_PHOTOGRAPH,						// big_icon_res
		0,													// big_icon_frame
		LUGG_PHOTOGRAPH,						// luggage_icon_res
		SCR_icon_combine_script,			// use_script
	},
	{	// 11 FLASHLIGHT
		menu_flashlight,						// text_desc
		ICON_FLASHLIGHT,						// big_icon_res
		0,													// big_icon_frame
		LUGG_FLASHLIGHT,						// luggage_icon_res
		SCR_icon_combine_script,			// use_script
	},
	{	// 12 FUSE_WIRE
		menu_fuse_wire,							// text_desc
		ICON_FUSE_WIRE,							// big_icon_res
		0,													// big_icon_frame
		LUGG_FUSE_WIRE,							// luggage_icon_res
		SCR_icon_combine_script,			// use_script
	},
	{	// 13 GEM
		menu_gem,										// text_desc
		ICON_GEM,										// big_icon_res
		0,													// big_icon_frame
		LUGG_GEM,										// luggage_icon_res
		SCR_icon_combine_script,			// use_script
	},
	{	// 14 STATUETTE_PAINT
		menu_statuette_paint,				// text_desc
		ICON_STATUETTE_PAINT,				// big_icon_res
		0,													// big_icon_frame
		LUGG_STATUETTE_PAINT,				// luggage_icon_res
		SCR_icon_combine_script,			// use_script
	},
	{	// 15 STICK
		menu_stick,									// text_desc
		ICON_STICK,									// big_icon_res
		0,													// big_icon_frame
		LUGG_STICK,									// luggage_icon_res
		SCR_icon_combine_script,			// use_script
	},
	{	// 16 EXCAV_KEY
		menu_excav_key,							// text_desc
		ICON_EXCAV_KEY,							// big_icon_res
		0,													// big_icon_frame
		LUGG_EXCAV_KEY,							// luggage_icon_res
		SCR_icon_combine_script,			// use_script
	},
	{	// 17 LAB_PASS
		menu_lab_pass,							// text_desc
		ICON_LAB_PASS,							// big_icon_res
		0,													// big_icon_frame
		LUGG_LAB_PASS,							// luggage_icon_res
		SCR_icon_combine_script,			// use_script
	},
	{	// 18 LIFTING_KEYS
		menu_lifting_keys,					// text_desc
		ICON_LIFTING_KEYS,					// big_icon_res
		0,													// big_icon_frame
		LUGG_LIFTING_KEYS,					// luggage_icon_res
		SCR_icon_combine_script,			// use_script
	},
	{	// 19 MANUSCRIPT
		menu_manuscript,						// text_desc
		ICON_MANUSCRIPT,						// big_icon_res
		0,													// big_icon_frame
		LUGG_MANUSCRIPT,						// luggage_icon_res
		SCR_icon_combine_script,			// use_script
	},
	{	// 20 MATCH_BOOK
		menu_match_book,						// text_desc
		ICON_MATCHBOOK,							// big_icon_res
		0,													// big_icon_frame
		LUGG_MATCHBOOK,							// luggage_icon_res
		SCR_icon_combine_script,			// use_script
	},
	{	// 21 SUIT_MATERIAL
		menu_suit_material,					// text_desc
		ICON_SUIT_MATERIAL,					// big_icon_res
		0,													// big_icon_frame
		LUGG_SUIT_MATERIAL,					// luggage_icon_res
		SCR_icon_combine_script,			// use_script
	},
	{	// 22 STICK_TOWEL
		menu_stick_towel,						// text_desc
		ICON_STICK_TOWEL,						// big_icon_res
		0,													// big_icon_frame
		LUGG_STICK_TOWEL,						// luggage_icon_res
		SCR_icon_combine_script,			// use_script
	},
	{	// 23 PLASTER
		menu_plaster,								// text_desc
		ICON_PLASTER,								// big_icon_res
		0,													// big_icon_frame
		LUGG_PLASTER,								// luggage_icon_res
		SCR_icon_combine_script,			// use_script
	},
	{	// 24 PRESSURE_GAUGE
		menu_pressure_gauge,				// text_desc
		ICON_PRESSURE_GAUGE,				// big_icon_res
		0,													// big_icon_frame
		LUGG_PRESSURE_GAUGE,				// luggage_icon_res
		SCR_icon_combine_script,			// use_script
	},
	{	// 25 RAILWAY_TICKET
		menu_railway_ticket,				// text_desc
		ICON_RAILWAY_TICKET,				// big_icon_res
		0,													// big_icon_frame
		LUGG_RAILWAY_TICKET,				// luggage_icon_res
		SCR_icon_combine_script,			// use_script
	},
	{	// 26 BUZZER
		menu_buzzer,								// text_desc
		ICON_BUZZER,								// big_icon_res
		0,													// big_icon_frame
		LUGG_BUZZER,								// luggage_icon_res
		SCR_icon_combine_script,			// use_script
	},
	{	// 27 ROSSO_CARD
		menu_rosso_card,						// text_desc
		ICON_ROSSO_CARD,						// big_icon_res
		0,													// big_icon_frame
		LUGG_ROSSO_CARD,						// luggage_icon_res
		SCR_icon_combine_script,			// use_script
	},
	{	// 28 TOILET_KEY
		menu_toilet_key,						// text_desc
		ICON_TOILET_KEY,						// big_icon_res
		0,													// big_icon_frame
		LUGG_TOILET_KEY,						// luggage_icon_res
		SCR_icon_combine_script,			// use_script
	},
	{	// 29 SOAP
		menu_soap,									// text_desc
		ICON_SOAP,									// big_icon_res
		0,													// big_icon_frame
		LUGG_SOAP,									// luggage_icon_res
		SCR_icon_combine_script,			// use_script
	},
	{	// 30 STONE_KEY
		menu_stone_key,							// text_desc
		ICON_STONE_KEY,							// big_icon_res
		0,													// big_icon_frame
		LUGG_STONE_KEY,							// luggage_icon_res
		SCR_icon_combine_script,			// use_script
	},
	{	// 31 CHALICE
		menu_chalice,								// text_desc
		ICON_CHALICE,								// big_icon_res
		0,													// big_icon_frame
		LUGG_CHALICE,								// luggage_icon_res
		SCR_icon_combine_script,			// use_script
	},
	{	// 32 TISSUE
		menu_tissue,								// text_desc
		ICON_TISSUE,								// big_icon_res
		0,													// big_icon_frame
		LUGG_TISSUE,								// luggage_icon_res
		SCR_icon_combine_script,			// use_script
	},
	{	// 33 TOILET_BRUSH
		menu_toilet_brush,					// text_desc
		ICON_TOILET_BRUSH,					// big_icon_res
		0,													// big_icon_frame
		LUGG_TOILET_BRUSH,					// luggage_icon_res
		SCR_icon_combine_script,			// use_script
	},
	{	// 34 TOILET_CHAIN
		menu_toilet_chain,					// text_desc
		ICON_TOILET_CHAIN,					// big_icon_res
		0,													// big_icon_frame
		LUGG_TOILET_CHAIN,					// luggage_icon_res
		SCR_icon_combine_script,			// use_script
	},
	{	// 35 TOWEL
		menu_towel,									// text_desc
		ICON_TOWEL,									// big_icon_res
		0,													// big_icon_frame
		LUGG_TOWEL,									// luggage_icon_res
		SCR_icon_combine_script,			// use_script
	},
	{	// 36 TRIPOD
		menu_tripod,								// text_desc
		ICON_TRIPOD,								// big_icon_res
		0,													// big_icon_frame
		LUGG_TRIPOD,								// luggage_icon_res
		SCR_icon_combine_script,			// use_script
	},
	{	// 37 LENS
		menu_lens,									// text_desc
		ICON_LENS,									// big_icon_res
		0,													// big_icon_frame
		LUGG_LENS,									// luggage_icon_res
		SCR_icon_combine_script,			// use_script
	},
	{	// 38 MIRROR
		menu_mirror,								// text_desc
		ICON_MIRROR,								// big_icon_res
		0,													// big_icon_frame
		LUGG_MIRROR,								// luggage_icon_res
		SCR_icon_combine_script,			// use_script
	},
	{	// 39 TOWEL_CUT
		menu_towel_cut,							// text_desc
		ICON_TOWEL_CUT,							// big_icon_res
		0,													// big_icon_frame
		LUGG_TOWEL_CUT,							// luggage_icon_res
		SCR_icon_combine_script,			// use_script
	},
	{	// 40 BIBLE
		menu_bible,									// text_desc
		ICON_BIBLE,									// big_icon_res
		0,													// big_icon_frame
		LUGG_BIBLE,									// luggage_icon_res
		SCR_icon_combine_script,			// use_script
	},
	{	// 41 TISSUE_CHARRED
		menu_tissue_charred,				// text_desc
		ICON_TISSUE_CHARRED,				// big_icon_res
		0,													// big_icon_frame
		LUGG_TISSUE_CHARRED,				// luggage_icon_res
		SCR_icon_combine_script,			// use_script
	},
	{	// 42 FALSE_KEY
		menu_false_key,							// text_desc
		ICON_FALSE_KEY,							// big_icon_res
		0,													// big_icon_frame
		LUGG_FALSE_KEY,							// luggage_icon_res
		SCR_icon_combine_script,			// use_script
	},
	{	// 43 PAINTED_KEY - looks identical to excav key, so uses that icon & luggage
		menu_painted_key,						// text_desc
		ICON_EXCAV_KEY,							// big_icon_res
		0,													// big_icon_frame
		LUGG_EXCAV_KEY,							// luggage_icon_res
		SCR_icon_combine_script,			// use_script
	},
	{	// 44 KEYRING
		0,													// text_desc - SEE MENU.SCR
		ICON_KEYRING,								// big_icon_res
		0,													// big_icon_frame
		LUGG_KEYRING,								// luggage_icon_res
		SCR_icon_combine_script,			// use_script
	},
	{	// 45 SOAP_IMP
		menu_soap_imp,							// text_desc
		ICON_SOAP_IMP,							// big_icon_res
		0,													// big_icon_frame
		LUGG_SOAP_IMP,							// luggage_icon_res
		SCR_icon_combine_script,			// use_script
	},
	{	// 46 SOAP_PLAS
		menu_soap_plas,							// text_desc
		ICON_SOAP_PLAS,							// big_icon_res
		0,													// big_icon_frame
		LUGG_SOAP_PLAS,							// luggage_icon_res
		SCR_icon_combine_script,			// use_script
	},
	{	// 47 COG_1 - the larger cog with spindle attached
		menu_cog_1,									// text_desc
		ICON_COG_1,									// big_icon_res
		0,													// big_icon_frame
		LUGG_COG_1,									// luggage_icon_res
		SCR_icon_combine_script,			// use_script
	},
	{	// 48 COG_2 - the smaller cog, found in the rubble
		menu_cog_2,									// text_desc
		ICON_COG_2,									// big_icon_res
		0,													// big_icon_frame
		LUGG_COG_2,									// luggage_icon_res
		SCR_icon_combine_script,			// use_script
	},
	{	// 49 HANDLE
		menu_handle,								// text_desc
		ICON_HANDLE,								// big_icon_res
		0,													// big_icon_frame
		LUGG_HANDLE,								// luggage_icon_res
		SCR_icon_combine_script,			// use_script
	},
	{	// 50 COIN
		menu_coin,									// text_desc
		ICON_COIN,									// big_icon_res
		0,													// big_icon_frame
		LUGG_COIN,									// luggage_icon_res
		SCR_icon_combine_script,			// use_script
	},
	{	// 51 BIRO
		menu_biro,									// text_desc
		ICON_BIRO,									// big_icon_res
		0,													// big_icon_frame
		LUGG_BIRO,									// luggage_icon_res
		SCR_icon_combine_script,			// use_script
	},
	{	// 52 PIPE
		menu_pipe,									// text_desc
		ICON_PIPE,									// big_icon_res
		0,													// big_icon_frame
		LUGG_PIPE,									// luggage_icon_res
		SCR_icon_combine_script,			// use_script
	}
};

const Subject Menu::_subjectList[TOTAL_subjects] = {
	{	// 256
		0,								// subject_res
		0									// subject_frame
	},
	{	// 257
		ICON_BEER,				// subject_res
		0									// subject_frame
	},
	{	// 258
		ICON_CASTLE,			// subject_res
		0									// subject_frame
	},
	{ // 259
		ICON_YES,					// subject_res
		0									// subject_frame
	},
	{ // 260
		ICON_NO,					// subject_res
		0									// subject_frame
	},
	{ // 261
		ICON_PEAGRAM,			// subject_res
		0									// subject_frame
	},
	{ // 262
		ICON_DIG,					// subject_res
		0									// subject_frame
	},
	{ // 263
		ICON_SEAN,				// subject_res
		0									// subject_frame
	},
	{ // 264
		ICON_GEM,					// subject_res
		0									// subject_frame
	},
	{ // 265
		ICON_TEMPLARS,		// subject_res
		0									// subject_frame
	},
	{ // 266
		ICON_LEPRECHAUN,	// subject_res
		0									// subject_frame
	},
	{ // 267
		ICON_GOODBYE,			// subject_res
		0									// subject_frame
	},
	{ // 268
		ICON_GEORGE,			// subject_res
		0									// subject_frame
	},
	{ // 269
		ICON_ROSSO,				// subject_res
		0									// subject_frame
	},
	{ // 270
		ICON_GHOST,				// subject_res
		0									// subject_frame
	},
	{ // 271
		ICON_CLOWN,				// subject_res
		0									// subject_frame
	},
	{ // 272
		ICON_CAR,					// subject_res
		0									// subject_frame
	},
	{ // 273
		ICON_MOUE,				// subject_res
		0									// subject_frame
	},
	{ // 274
		ICON_NICO,				// subject_res
		0									// subject_frame
	},
	{ // 275
		ICON_MOB,					// subject_res
		0									// subject_frame
	},
	{ // 276
		ICON_CHANTELLE,		// subject_res
		0									// subject_frame
	},
	{ // 277
		ICON_PLANTARD,		// subject_res
		0									// subject_frame
	},
	{ // 278
		ICON_JACKET,			// subject_res
		0									// subject_frame
	},
	{ // 279
		ICON_BRIEFCASE,		// subject_res
		0									// subject_frame
	},
	{
		0, //ICON_GLASS,	// subject_res
		0									// subject_frame
	},
	{ // 281
		ICON_GLASS_EYE,		// subject_res
		0									// subject_frame
	},
	{ // 282
		ICON_BULL,				// subject_res
		0									// subject_frame
	},
	{ // 283
		ICON_KLAUSNER,		// subject_res
		0									// subject_frame
	},
	{ // 284
		0, //ICON_LOOM,		// subject_res
		0									// subject_frame
	},
	{ // 285
		ICON_ULTAR,				// subject_res
		0									// subject_frame
	},
	{ // 286
		ICON_PHONE,				// subject_res
		0									// subject_frame
	},
	{ // 287
		ICON_PHOTOGRAPH,	// subject_res
		0									// subject_frame
	},
	{ // 288
		ICON_CREST,				// subject_res
		0									// subject_frame
	},
	{ // 289
		ICON_LOBINEAU,		// subject_res
		0									// subject_frame
	},
	{ // 290
		ICON_BOOK,				// subject_res
		0									// subject_frame
	},
	{ // 291 (SNARE)
		ICON_FUSE_WIRE,		// subject_res
		0									// subject_frame
	},
	{ // 292
		ICON_LEARY,				// subject_res
		0									// subject_frame
	},
	{ // 293
		ICON_LADDER,			// subject_res
		0									// subject_frame
	},
	{ // 294
		ICON_GOAT,				// subject_res
		0									// subject_frame
	},
	{ // 295
		ICON_TOOLBOX,			// subject_res
		0									// subject_frame
	},
	{ // 296
		ICON_PACKAGE,			// subject_res
		0									// subject_frame
	},
	{ // 297
		ICON_FISH,				// subject_res
		0									// subject_frame
	},
	{ // 298
		ICON_NEJO,				// subject_res
		0									// subject_frame
	},
	{ // 299
		ICON_CAT,					// subject_res
		0									// subject_frame
	},
	{ // 300
		ICON_AYUB,				// subject_res
		0									// subject_frame
	},
	{ // 301
		ICON_STATUETTE,		// subject_res
		0									// subject_frame
	},
	{ // 302
		ICON_NEJO_STALL,	// subject_res
		0									// subject_frame
	},
	{ // 303
		ICON_TEMPLARS,		// subject_res
		0									// subject_frame
	},
	{ // 304
		ICON_ARTO,				// subject_res
		0									// subject_frame
	},
	{ // 305
		ICON_HENDERSONS,	// subject_res
		0									// subject_frame
	},
	{ // 306
		ICON_CLUB,				// subject_res
		0									// subject_frame
	},
	{ // 307
		ICON_SIGN,				// subject_res
		0									// subject_frame
	},
	{ // 308
		ICON_TAXI,				// subject_res
		0									// subject_frame
	},
	{ // 309
		ICON_BULLS_HEAD,	// subject_res
		0									// subject_frame
	},
	{ // 310
		ICON_DUANE,				// subject_res
		0									// subject_frame
	},
	{ // 311
		ICON_PEARL,				// subject_res
		0									// subject_frame
	},
	{ // 312
		ICON_TRUTH,				// subject_res
		0									// subject_frame
	},
	{ // 313
		ICON_LIE,					// subject_res
		0									// subject_frame
	},
	{ // 314
		0, //ICON_SUBJECT,// subject_res
		0									// subject_frame
	},
	{ // 315 KEY
		ICON_HOTEL_KEY,		// subject_res
		0									// subject_frame
	},
	{ // 316
		ICON_FLOWERS,			// subject_res
		0									// subject_frame
	},
	{ // 317
		ICON_BUST,				// subject_res
		0									// subject_frame
	},
	{ // 318
		ICON_MANUSCRIPT,	// subject_res
		0									// subject_frame
	},
	{ // 319
		ICON_WEASEL,			// subject_res
		0									// subject_frame
	},
	{ // 320
		ICON_BANANA,			// subject_res
		0									// subject_frame
	},
	{ // 321
		ICON_WEAVER,			// subject_res
		0									// subject_frame
	},
	{ // 322
		ICON_KNIGHT,			// subject_res
		0									// subject_frame
	},
	{ // 323
		ICON_QUEEN,				// subject_res
		0									// subject_frame
	},
	{ // 324
		ICON_PIERMONT,		// subject_res
		0									// subject_frame
	},
	{ // 325
		ICON_BALL,				// subject_res
		0									// subject_frame
	},
	{ // 326
		ICON_COUNTESS,		// subject_res
		0									// subject_frame
	},
	{ // 327
		ICON_MARQUET,			// subject_res
		0									// subject_frame
	},
	{ // 328
		ICON_SAFE,				// subject_res
		0									// subject_frame
	},
	{ // 329
		ICON_COINS,				// subject_res
		0									// subject_frame
	},
	{ // 330
		ICON_CHESS_SET,		// subject_res
		0									// subject_frame
	},
	{ // 331
		ICON_TOMB,				// subject_res
		0									// subject_frame
	},
	{ // 332
		ICON_CANDLE,			// subject_res
		0									// subject_frame
	},
	{ // 333
		ICON_MARY,				// subject_res
		0									// subject_frame
	},
	{ // 334
		ICON_CHESSBOARD,	// subject_res
		0									// subject_frame
	},
	{ // 335
		ICON_TRIPOD,			// subject_res
		0									// subject_frame
	},
	{ // 336
		ICON_POTS,				// subject_res
		0									// subject_frame
	},
	{ // 337
		ICON_ALARM,				// subject_res
		0									// subject_frame
	},
	{ // 338
		ICON_BAPHOMET,		// subject_res
		0									// subject_frame
	},
	{ // 339
		ICON_PHRASE,			// subject_res
		0									// subject_frame
	},
	{ // 340
		ICON_POLISHED_CHALICE,	// subject_res
		0												// subject_frame
	},
	{ // 341
		ICON_NURSE,				// subject_res
		0									// subject_frame
	},
	{ // 342
		ICON_WELL,				// subject_res
		0									// subject_frame
	},
	{ // 343
		ICON_WELL2,				// subject_res
		0									// subject_frame
	},
	{ // 344
		ICON_HAZEL_WAND,	// subject_res
		0									// subject_frame
	},
	{ // 345
		ICON_CHALICE,			// subject_res
		0									// subject_frame
	},
	{ // 346
		ICON_MR_SHINY,		// subject_res
		0									// subject_frame
	},
	{ // 347
		ICON_PHOTOGRAPH,	// subject_res
		0									// subject_frame
	},
	{ // 348
		ICON_PHILIPPE,		// subject_res
		0									// subject_frame
	},
	{ // 349
		ICON_ERIC,				// subject_res
		0									// subject_frame
	},
	{ // 350
		ICON_ROZZER,			// subject_res
		0									// subject_frame
	},
	{ // 351
		ICON_JUGGLER,			// subject_res
		0									// subject_frame
	},
	{ // 352
		ICON_PRIEST,			// subject_res
		0									// subject_frame
	},
	{ // 353
		ICON_WINDOW,			// subject_res
		0									// subject_frame
	},
	{ // 354
		ICON_SCROLL,			// subject_res
		0									// subject_frame
	},
	{ // 355
		ICON_PRESSURE_GAUGE,	// subject_res
		0											// subject_frame
	},
	{ // 356
		ICON_RENEE,				// subject_res
		0									// subject_frame
	},
	{ // 357
		ICON_CHURCH,			// subject_res
		0									// subject_frame
	},
	{ // 358
		ICON_EKLUND,			// subject_res
		0									// subject_frame
	},
	{ // 359
		ICON_FORTUNE,			// subject_res
		0									// subject_frame
	},
	{ // 360
		ICON_PAINTER,			// subject_res
		0									// subject_frame
	},
	{ // 361
		0, //ICON_SWORD,	// subject_res
		0									// subject_frame
	},
	{ // 362
		ICON_GUARD,				// subject_res
		0									// subject_frame
	},
	{ // 363
		ICON_THERMOSTAT,	// subject_res
		0									// subject_frame
	},
	{ // 364
		ICON_TOILET,			// subject_res
		0									// subject_frame
	},
	{ // 365
		ICON_MONTFAUCON,	// subject_res
		0									// subject_frame
	},
	{ // 366
		ICON_ASSASSIN,		// subject_res
		0									// subject_frame
	},
	{ // 367
		ICON_HASH,				// subject_res
		0									// subject_frame
	},
	{ // 368
		ICON_DOG,					// subject_res
		0									// subject_frame
	},
	{ // 369
		ICON_AYUB,				// subject_res
		0									// subject_frame
	},
	{ // 370
		ICON_LENS,				// subject_res
		0									// subject_frame
	},
	{ // 371
		ICON_RED_NOSE,		// subject_res
		0									// subject_frame
	},
	{ // 372
		0,								// subject_res
		0									// subject_frame
	},
	{ // 373
		0,								// subject_res
		0									// subject_frame
	},
	{ // 374
		0,								// subject_res
		0									// subject_frame
	}
};

const uint32 ResMan::_scriptList[TOTAL_SECTIONS] = { //a table of resource tags
	SCRIPT0,		// 0		STANDARD SCRIPTS

	SCRIPT1,		// 1		PARIS 1
	SCRIPT2,		// 2
	SCRIPT3,		// 3
	SCRIPT4,		// 4
	SCRIPT5,		// 5
	SCRIPT6,		// 6
	SCRIPT7,		// 7
	SCRIPT8,		// 8

	SCRIPT9,		// 9		PARIS 2
	SCRIPT10,		// 10
	SCRIPT11,		// 11
	SCRIPT12,		// 12
	SCRIPT13,		// 13
	SCRIPT14,		// 14
	SCRIPT15,		// 15
	SCRIPT16,		// 16
	SCRIPT17,		// 17
	SCRIPT18,		// 18

	SCRIPT19,		// 19		IRELAND
	SCRIPT20,		// 20
	SCRIPT21,		// 21
	SCRIPT22,		// 22
	SCRIPT23,		// 23
	SCRIPT24,		// 24
	SCRIPT25,		// 25
	SCRIPT26,		// 26

	SCRIPT27,		// 27		PARIS 3
	SCRIPT28,		// 28
	SCRIPT29,		// 29
	0,					// 30
	SCRIPT31,		// 31
	SCRIPT32,		// 32
	SCRIPT33,		// 33
	SCRIPT34,		// 34
	SCRIPT35,		// 35

	SCRIPT36,		// 36		PARIS 4
	SCRIPT37,		// 37
	SCRIPT38,		// 38
	SCRIPT39,		// 39
	SCRIPT40,		// 40
	SCRIPT41,		// 41
	SCRIPT42,		// 42
	SCRIPT43,		// 43
	0,					// 44

	SCRIPT45,		// 45		SYRIA
	SCRIPT46,		// 46		PARIS 4
	SCRIPT47,		// 47
	SCRIPT48,		// 48		PARIS 4
	SCRIPT49,		// 49
	SCRIPT50,		// 50
	0,					// 51
	0,					// 52
	0,					// 53
	SCRIPT54,		// 54
	SCRIPT55,		// 55

	SCRIPT56,		// 56		SPAIN
	SCRIPT57,		// 57
	SCRIPT58,		// 58
	SCRIPT59,		// 59
	SCRIPT60,		// 60
	SCRIPT61,		// 61
	SCRIPT62,		// 62

	SCRIPT63,		// 63		NIGHT TRAIN
	0,					// 64
	SCRIPT65,		// 65
	SCRIPT66,		// 66
	SCRIPT67,		// 67
	0,					// 68
	SCRIPT69,		// 69
	0,					// 70

	SCRIPT71,		// 71		SCOTLAND
	SCRIPT72,		// 72
	SCRIPT73,		// 73
	SCRIPT74,		// 74

	0,					// 75
	0,					// 76
	0,					// 77
	0,					// 78
	0,					// 79
	SCRIPT80,		// 80
	0,					// 81
	0,					// 82
	0,					// 83
	0,					// 84
	0,					// 85
	SCRIPT86,		// 86
	0,					// 87
	0,					// 88
	0,					// 89
	SCRIPT90,		// 90
	0,					// 91
	0,					// 92
	0,					// 93
	0,					// 94
	0,					// 95
	0,					// 96
	0,					// 97
	0,					// 98
	0,					// 99
	0,					// 100
	0,					// 101
	0,					// 102
	0,					// 103
	0,					// 104
	0,					// 105
	0,					// 106
	0,					// 107
	0,					// 108
	0,					// 109
	0,					// 110
	0,					// 111
	0,					// 112
	0,					// 113
	0,					// 114
	0,					// 115
	0,					// 116
	0,					// 117
	0,					// 118
	0,					// 119
	0,					// 120
	0,					// 121
	0,					// 122
	0,					// 123
	0,					// 124
	0,					// 125
	0,					// 126
	0,					// 127
	SCRIPT128,	// 128

	SCRIPT129,	// 129
	SCRIPT130,	// 130
	SCRIPT131,	// 131
	0,					// 132
	SCRIPT133,	// 133
	SCRIPT134,	// 134
	0,					// 135
	0,					// 136
	0,					// 137
	0,					// 138
	0,					// 139
	0,					// 140
	0,					// 141
	0,					// 142
	0,					// 143
	0,					// 144
	SCRIPT145,	// 145
	SCRIPT146,	// 146
	0,					// 147
	0,					// 148
	0,					// 149
};

const uint32 ObjectMan::_objectList[TOTAL_SECTIONS] = { //a table of pointers to object files
	0,			// 0

	COMP1,		// 1		PARIS 1
	COMP2,		// 2
	COMP3,		// 3
	COMP4,		// 4
	COMP5,		// 5
	COMP6,		// 6
	COMP7,		// 7
	COMP8,		// 8

	COMP9,		// 9		PARIS 2
	COMP10,		// 10
	COMP11,		// 11
	COMP12,		// 12
	COMP13,		// 13
	COMP14,		// 14
	COMP15,		// 15
	COMP16,		// 16
	COMP17,		// 17
	COMP18,		// 18

	COMP19,		// 19		IRELAND
	COMP20,		// 20
	COMP21,		// 21
	COMP22,		// 22
	COMP23,		// 23
	COMP24,		// 24
	COMP25,		// 25
	COMP26,		// 26

	COMP27,		// 27		PARIS 3
	COMP28,		// 28
	COMP29,		// 29
	COMP30,		// 30 - Heart Monitor
	COMP31,		// 31
	COMP32,		// 32
	COMP33,		// 33
	COMP34,		// 34
	COMP35,		// 35

	COMP36,		// 36		PARIS 4
	COMP37,		// 37
	COMP38,		// 38
	COMP39,		// 39
	COMP40,		// 40
	COMP41,		// 41
	COMP42,		// 42
	COMP43,		// 43
	0,				// 44

	COMP45,		// 45		SYRIA
	COMP46,		// 46		PARIS 4
	COMP47,		// 47
	COMP48,		// 48		PARIS 4
	COMP49,		// 49
	COMP50,		// 50
	0,				// 51
	0,				// 52
	COMP53,		// 53
	COMP54,		// 54
	COMP55,		// 55

	COMP56,		// 56		SPAIN
	COMP57,		// 57
	COMP58,		// 58
	COMP59,		// 59
	COMP60,		// 60
	COMP61,		// 61
	COMP62,		// 62

	COMP63,		// 63		NIGHT TRAIN
	0,				// 64
	COMP65,		// 65
	COMP66,		// 66
	COMP67,		// 67
	0,				// 68
	COMP69,		// 69
	0,				// 70

	COMP71,		// 71		SCOTLAND
	COMP72,		// 72
	COMP73,		// 73
	COMP74,		// 74		END SEQUENCE IN SECRET_CRYPT
	COMP75,		// 75
	COMP76,		// 76
	COMP77,		// 77
	COMP78,		// 78
	COMP79,		// 79

	COMP80,		// 80		PARIS MAP

	COMP81,		// 81	Full-screen for "Asstair" in Paris2

	COMP55,		// 82	Full-screen BRITMAP in sc55 (Syrian Cave)
	0,				// 83
	0,				// 84
	0,				// 85

	COMP86,		// 86		EUROPE MAP
	COMP48,		// 87		fudged in for normal window (sc48)
	COMP48,		// 88		fudged in for filtered window (sc48)
	0,				// 89

	COMP90,		// 90		PHONE SCREEN
	COMP91,		// 91		ENVELOPE SCREEN
	COMP17,		// 92		fudged in for George close-up surprised in sc17 wardrobe
	COMP17,		// 93		fudged in for George close-up inquisitive in sc17 wardrobe
	COMP29,		// 94		fudged in for George close-up in sc29 sarcophagus
	COMP38,		// 95		fudged in for George close-up in sc29 sarcophagus
	COMP42,		// 96		fudged in for chalice close-up from sc42
	0,				// 97
	0,				// 98
	COMP99,		// 99		MESSAGE SCREEN (BLANK)

	0,				// 100
	0,				// 101
	0,				// 102
	0,				// 103
	0,				// 104
	0,				// 105
	0,				// 106
	0,				// 107
	0,				// 108
	0,				// 109

	0,				// 110
	0,				// 111
	0,				// 112
	0,				// 113
	0,				// 114
	0,				// 115
	0,				// 116
	0,				// 117
	0,				// 118
	0,				// 119

	0,				// 120
	0,				// 121
	0,				// 122
	0,				// 123
	0,				// 124
	0,				// 125
	0,				// 126
	0,				// 127

//mega sections
	MEGA_GEO,		// 128 mega_one the player
	MEGA_NICO,		// 129 mega_two
	MEGA_MUS,		// 130
	MEGA_BENOIR,	// 131
	0,				// 132
	MEGA_ROSSO,		// 133
	MEGA_DUANE,		// 134
// james megas
	0,					// 135
	0,					// 136
	0,					// 137
	0,					// 138
	0,					// 139
	0,					// 140
	0,					// 141
	0,					// 142
	0,					// 143

// jeremy megas
	0,					// 144 mega_phone
	MEGA_MOUE,			// 145 mega_moue
	MEGA_ALBERT,		// 146 mega_albert
	0,					// 147
	0,					// 148
	TEXT_OBS,			// 149
};

const uint32 ObjectMan::_textList[TOTAL_SECTIONS][7] = {
	{ENGLISH0,		FRENCH0,		GERMAN0,		ITALIAN0,		SPANISH0,		CZECH0,		PORT0},		// 0		INVENTORY		BOTH CD'S - used in almost all locations
	{ENGLISH1,		FRENCH1,		GERMAN1,		ITALIAN1,		SPANISH1,		CZECH1,		PORT1},		// 1		PARIS 1			CD1
	{ENGLISH2,		FRENCH2,		GERMAN2,		ITALIAN2,		SPANISH2,		CZECH2,		PORT2},		// 2								CD1
	{ENGLISH3,		FRENCH3,		GERMAN3,		ITALIAN3,		SPANISH3,		CZECH3,		PORT3},		// 3								CD1
	{ENGLISH4,		FRENCH4,		GERMAN4,		ITALIAN4,		SPANISH4,		CZECH4,		PORT4},		// 4								CD1
	{ENGLISH5,		FRENCH5,		GERMAN5,		ITALIAN5,		SPANISH5,		CZECH5,		PORT5},		// 5								CD1
	{ENGLISH6,		FRENCH6,		GERMAN6,		ITALIAN6,		SPANISH6,		CZECH6,		PORT6},		// 6								CD1
	{ENGLISH7,		FRENCH7,		GERMAN7,		ITALIAN7,		SPANISH7,		CZECH7,		PORT7},		// 7								CD1
	{0},																																							// 8								-
	{ENGLISH9,		FRENCH9,		GERMAN9,		ITALIAN9,		SPANISH9,		CZECH9,		PORT9},		// 9		PARIS 2			CD1
	{0},																																							// 10								-
	{ENGLISH11,		FRENCH11,		GERMAN11,		ITALIAN11,	SPANISH11,	CZECH11,	PORT11},	// 11								CD1
	{ENGLISH12,		FRENCH12,		GERMAN12,		ITALIAN12,	SPANISH12,	CZECH12,	PORT12},	// 12								CD1
	{ENGLISH13,		FRENCH13,		GERMAN13,		ITALIAN13,	SPANISH13,	CZECH13,	PORT13},	// 13								CD1
	{ENGLISH14,		FRENCH14,		GERMAN14,		ITALIAN14,	SPANISH14,	CZECH14,	PORT14},	// 14								CD1
	{ENGLISH15,		FRENCH15,		GERMAN15,		ITALIAN15,	SPANISH15,	CZECH15,	PORT15},	// 15								CD1
	{ENGLISH16,		FRENCH16,		GERMAN16,		ITALIAN16,	SPANISH16,	CZECH16,	PORT16},	// 16								CD1
	{ENGLISH17,		FRENCH17,		GERMAN17,		ITALIAN17,	SPANISH17,	CZECH17,	PORT17},	// 17								CD1
	{ENGLISH18,		FRENCH18,		GERMAN18,		ITALIAN18,	SPANISH18,	CZECH18,	PORT18},	// 18								CD1
	{ENGLISH19,		FRENCH19,		GERMAN19,		ITALIAN19,	SPANISH19,	CZECH19,	PORT19},	// 19		IRELAND			CD2
	{ENGLISH20,		FRENCH20,		GERMAN20,		ITALIAN20,	SPANISH20,	CZECH20,	PORT20},	// 20								CD2
	{ENGLISH21,		FRENCH21,		GERMAN21,		ITALIAN21,	SPANISH21,	CZECH21,	PORT21},	// 21								CD2
	{ENGLISH22,		FRENCH22,		GERMAN22,		ITALIAN22,	SPANISH22,	CZECH22,	PORT22},	// 22								CD2
	{ENGLISH23,		FRENCH23,		GERMAN23,		ITALIAN23,	SPANISH23,	CZECH23,	PORT23},	// 23								CD2
	{ENGLISH24,		FRENCH24,		GERMAN24,		ITALIAN24,	SPANISH24,	CZECH24,	PORT24},	// 24								CD2
	{ENGLISH25,		FRENCH25,		GERMAN25,		ITALIAN25,	SPANISH25,	CZECH25,	PORT25},	// 25								CD2
	{0},																																							// 26								-
	{ENGLISH27,		FRENCH27,		GERMAN27,		ITALIAN27,	SPANISH27,	CZECH27,	PORT27},	// 27		PARIS 3			CD1
	{ENGLISH28,		FRENCH28,		GERMAN28,		ITALIAN28,	SPANISH28,	CZECH28,	PORT28},	// 28								CD1
	{ENGLISH29,		FRENCH29,		GERMAN29,		ITALIAN29,	SPANISH29,	CZECH29,	PORT29},	// 29								CD1
	{0},																																							// 30								-
	{ENGLISH31,		FRENCH31,		GERMAN31,		ITALIAN31,	SPANISH31,	CZECH31,	PORT31},	// 31								CD1
	{ENGLISH32,		FRENCH32,		GERMAN32,		ITALIAN32,	SPANISH32,	CZECH32,	PORT32},	// 32								CD1
	{ENGLISH33,		FRENCH33,		GERMAN33,		ITALIAN33,	SPANISH33,	CZECH33,	PORT33},	// 33								CD1
	{ENGLISH34,		FRENCH34,		GERMAN34,		ITALIAN34,	SPANISH34,	CZECH34,	PORT34},	// 34								CD1
	{ENGLISH35,		FRENCH35,		GERMAN35,		ITALIAN35,	SPANISH35,	CZECH35,	PORT35},	// 35								CD1
	{ENGLISH36,		FRENCH36,		GERMAN36,		ITALIAN36,	SPANISH36,	CZECH36,	PORT36},	// 36		PARIS 4			CD1
	{ENGLISH37,		FRENCH37,		GERMAN37,		ITALIAN37,	SPANISH37,	CZECH37,	PORT37},	// 37								CD1
	{ENGLISH38,		FRENCH38,		GERMAN38,		ITALIAN38,	SPANISH38,	CZECH38,	PORT38},	// 38								CD1
	{ENGLISH39,		FRENCH39,		GERMAN39,		ITALIAN39,	SPANISH39,	CZECH39,	PORT39},	// 39								CD1
	{ENGLISH40,		FRENCH40,		GERMAN40,		ITALIAN40,	SPANISH40,	CZECH40,	PORT40},	// 40								CD1
	{ENGLISH41,		FRENCH41,		GERMAN41,		ITALIAN41,	SPANISH41,	CZECH41,	PORT41},	// 41								CD1
	{ENGLISH42,		FRENCH42,		GERMAN42,		ITALIAN42,	SPANISH42,	CZECH42,	PORT42},	// 42								CD1
	{ENGLISH43,		FRENCH43,		GERMAN43,		ITALIAN43,	SPANISH43,	CZECH43,	PORT43},	// 43								CD1
	{0},																																							// 44								-
	{ENGLISH45,		FRENCH45,		GERMAN45,		ITALIAN45,	SPANISH45,	CZECH45,	PORT45},	// 45		SYRIA				CD2
	{0},																																							// 46		(PARIS 4)		-
	{ENGLISH47,		FRENCH47,		GERMAN47,		ITALIAN47,	SPANISH47,	CZECH47,	PORT47},	// 47								CD2
	{ENGLISH48,		FRENCH48,		GERMAN48,		ITALIAN48,	SPANISH48,	CZECH48,	PORT48},	// 48		(PARIS 4)		CD1
	{ENGLISH49,		FRENCH49,		GERMAN49,		ITALIAN49,	SPANISH49,	CZECH49,	PORT49},	// 49								CD2
	{ENGLISH50,		FRENCH50,		GERMAN50,		ITALIAN50,	SPANISH50,	CZECH50,	PORT50},	// 50								CD2
	{0},																																							// 51								-
	{0},																																							// 52								-
	{0},																																							// 53								-
	{ENGLISH54,		FRENCH54,		GERMAN54,		ITALIAN54,	SPANISH54,	CZECH54,	PORT54},	// 54								CD2
	{ENGLISH55,		FRENCH55,		GERMAN55,		ITALIAN55,	SPANISH55,	CZECH55,	PORT55},	// 55								CD2
	{ENGLISH56,		FRENCH56,		GERMAN56,		ITALIAN56,	SPANISH56,	CZECH56,	PORT56},	// 56		SPAIN				CD2
	{ENGLISH57,		FRENCH57,		GERMAN57,		ITALIAN57,	SPANISH57,	CZECH57,	PORT57},	// 57								CD2
	{ENGLISH58,		FRENCH58,		GERMAN58,		ITALIAN58,	SPANISH58,	CZECH58,	PORT58},	// 58								CD2
	{ENGLISH59,		FRENCH59,		GERMAN59,		ITALIAN59,	SPANISH59,	CZECH59,	PORT59},	// 59								CD2
	{ENGLISH60,		FRENCH60,		GERMAN60,		ITALIAN60,	SPANISH60,	CZECH60,	PORT60},	// 60								CD2
	{ENGLISH61,		FRENCH61,		GERMAN61,		ITALIAN61,	SPANISH61,	CZECH61,	PORT61},	// 61								CD2
	{0},																																							// 62								-
	{ENGLISH63,		FRENCH63,		GERMAN63,		ITALIAN63,	SPANISH63,	CZECH63,	PORT63},	// 63		TRAIN				CD2
	{0},																																							// 64								-
	{ENGLISH65,		FRENCH65,		GERMAN65,		ITALIAN65,	SPANISH65,	CZECH65,	PORT65},	// 65								CD2
	{ENGLISH66,		FRENCH66,		GERMAN66,		ITALIAN66,	SPANISH66,	CZECH66,	PORT66},	// 66								CD2
	{0},																																							// 67								-
	{0},																																							// 68								-
	{ENGLISH69,		FRENCH69,		GERMAN69,		ITALIAN69,	SPANISH69,	CZECH69,	PORT69},	// 69								CD2
	{0},																																							// 70								-
	{ENGLISH71,		FRENCH71,		GERMAN71,		ITALIAN71,	SPANISH71,	CZECH71,	PORT71},	// 71		SCOTLAND		CD2
	{ENGLISH72,		FRENCH72,		GERMAN72,		ITALIAN72,	SPANISH72,	CZECH72,	PORT72},	// 72								CD2
	{ENGLISH73,		FRENCH73,		GERMAN73,		ITALIAN73,	SPANISH73,	CZECH73,	PORT73},	// 73								CD2
	{ENGLISH74,		FRENCH74,		GERMAN74,		ITALIAN74,	SPANISH74,	CZECH74,	PORT74},	// 74								CD2
	{0},																																							// 75								-
	{0},																																							// 76								-
	{0},																																							// 77								-
	{0},																																							// 78								-
	{0},																																							// 79								-
	{0},																																							// 80								-
	{0},																																							// 81								-
	{0},																																							// 82								-
	{0},																																							// 83								-
	{0},																																							// 84								-
	{0},																																							// 85								-
	{0},																																							// 86								-
	{0},																																							// 87								-
	{0},																																							// 88								-
	{0},																																							// 89								-
	{ENGLISH90,		FRENCH90,		GERMAN90,		ITALIAN90,	SPANISH90,	CZECH90,	PORT90},	// 90		PHONE				BOTH CD'S (NICO & TODRYK PHONE TEXT - can phone nico from a number of sections
	{0},																																							// 91								-
	{0},																																							// 92								-
	{0},																																							// 93								-
	{0},																																							// 94								-
	{0},																																							// 95								-
	{0},																																							// 96								-
	{0},																																							// 97								-
	{0},																																							// 98								-
	{ENGLISH99,		FRENCH99,		GERMAN99,		ITALIAN99,	SPANISH99,	CZECH99,	PORT99},	// 99		MESSAGES		BOTH CD'S - contains general text, most not requiring samples, but includes demo samples
	{0},																																							// 100							-
	{0},																																							// 101							-
	{0},																																							// 102							-
	{0},																																							// 103							-
	{0},																																							// 104							-
	{0},																																							// 105							-
	{0},																																							// 106							-
	{0},																																							// 107							-
	{0},																																							// 108							-
	{0},																																							// 109							-
	{0},																																							// 110							-
	{0},																																							// 111							-
	{0},																																							// 112							-
	{0},																																							// 113							-
	{0},																																							// 114							-
	{0},																																							// 115							-
	{0},																																							// 116							-
	{0},																																							// 117							-
	{0},																																							// 118							-
	{0},																																							// 119							-
	{0},																																							// 120							-
	{0},																																							// 121							-
	{0},																																							// 122							-
	{0},																																							// 123							-
	{0},																																							// 124							-
	{0},																																							// 125							-
	{0},																																							// 126							-
	{0},																																							// 127							-
	{0},																																							// 128							-
	{ENGLISH129,	FRENCH129,	GERMAN129,	ITALIAN129,	SPANISH129,	CZECH129,	PORT129},	// 129	NICO				BOTH CD'S	- used in screens 1,10,71,72,73
	{0},																																							// 130							-
	{ENGLISH131,	FRENCH131,	GERMAN131,	ITALIAN131,	SPANISH131,	CZECH131,	PORT131},	// 131	BENOIR			CD1				- used in screens 31..35
	{0},																																							// 132							-
	{ENGLISH133,	FRENCH133,	GERMAN133,	ITALIAN133,	SPANISH133,	CZECH133,	PORT133},	// 133	ROSSO				CD1				- used in screen 18
	{0},																																							// 134							-
	{0},																																							// 135							-
	{0},																																							// 136							-
	{0},																																							// 137							-
	{0},																																							// 138							-
	{0},																																							// 139							-
	{0},																																							// 140							-
	{0},																																							// 141							-
	{0},																																							// 142							-
	{0},																																							// 143							-
	{0},																																							// 144							-
	{ENGLISH145,	FRENCH145,	GERMAN145,	ITALIAN145,	SPANISH145,	CZECH145,	PORT145},	// 145	MOUE				CD1				- used in screens 1 & 18
	{ENGLISH146,	FRENCH146,	GERMAN146,	ITALIAN146,	SPANISH146,	CZECH146,	PORT146},	// 146	ALBERT			CD1				- used in screens 4 & 5
	{0},																																							// 147							-
	{0},																																							// 148							-
	{0},																																							// 149							-
};


RoomDef Screen::_roomDefTable[TOTAL_ROOMS] = { // these are NOT const
	{
		0,						//total_layers  --- room 0 NOT USED
		0,						//size_x			= width
		0,						//size_y			= height
		0,						//grid_width	= width/16 + 16
		{0,0,0,0},				//layers
		{0,0,0},				//grids
		{0,0},					//palettes		{ background palette [0..183], sprite palette [184..255] }
		{0,0},					//parallax layers
	},

	//------------------------------------------------------------------------
	// PARIS 1

	{
		3,													//total_layers		//room 1
		784,												//size_x
		400,												//size_y
		65,													//grid_width
		{room1_l0,room1_l1,room1_l2},						//layers
		{room1_gd1,room1_gd2},								//grids
		{room1_PAL,PARIS1_PAL},								//palettes
		{room1_plx,0},												//parallax layers
	},
	{
		3,													//total_layers		//room 2
		640,												//size_x
		400,												//size_y
		56,													//grid_width
		{room2_l0,room2_l1,room2_l2,0},						//layers
		{room2_gd1,room2_gd2,0},							//grids
		{room2_PAL,PARIS1_PAL},								//palettes
		{0,0},												//parallax layers
	},
	{
		3,													//total_layers		//room 3
		640,												//size_x
		400,												//size_y
		56,													//grid_width
		{room3_l0,room3_l1,room3_l2,0},						//layers
		{room3_gd1,room3_gd2,0},							//grids
		{room3_PAL,PARIS1_PAL},								//palettes
		{0,0},												//parallax layers
	},
	{
		3,																			//total_layers		//room 4
		640,																		//size_x
		400,																		//size_y
		56,																			//grid_width
		{room4_l0,room4_l1,room4_l2,0},					//layers
		{room4_gd1,room4_gd2,0},								//grids
		{room4_PAL,PARIS1_PAL},									//palettes
		{0,0},																	//parallax layers
	},
	{
		3,																			//total_layers		//room 5
		640,																		//size_x
		400,																		//size_y
		56,																			//grid_width
		{room5_l0,room5_l1,room5_l2,0},					//layers
		{room5_gd1,room5_gd2,0},								//grids
		{room5_PAL,PARIS1_PAL},									//palettes
		{0,0},																	//parallax layers
	},
	{
		2,																			//total_layers		//room 6
		640,																		//size_x
		400,																		//size_y
		56,																			//grid_width
		{room6_l0,room6_l1,0,0},								//layers
		{room6_gd1,0,0},												//grids
		{room6_PAL,SEWER_PAL},									//palettes
		{0,0},																	//parallax layers
	},
	{
		3,																			//total_layers		//room 7
		640,																		//size_x
		400,																		//size_y
		56,																			//grid_width
		{room7_l0,room7_l1,room7_l2,0},					//layers
		{room7_gd1,room7_gd2,0},								//grids
		{room7_PAL,SEWER_PAL},									//palettes
		{0,0},																	//parallax layers
	},
	{
		3,																			//total_layers		//room 8
		784,																		//size_x
		400,																		//size_y
		65,																			//grid_width
		{room8_l0,room8_l1,room8_l2,0},					//layers
		{room8_gd1,room8_gd2,0},								//grids
		{room8_PAL,PARIS1_PAL},									//palettes
		{room8_plx,0},													//parallax layers
	},

	//------------------------------------------------------------------------
	// PARIS 2

	{
		3,																				//total_layers		//room 9
		640,																			//size_x
		400,																			//size_y
		56,																				//grid_width
		{room9_l0,room9_l1,room9_l2,0},						//layers
		{room9_gd1,room9_gd2,0},									//grids
		{room9_PAL,PARIS2_PAL},										//palettes
		{0,0},																		//parallax layers
	},
	{
		2,																				//total_layers		//room 10
		640,																			//size_x
		400,																			//size_y
		56,																				//grid_width
		{room10_l0,room10_l1,0,0},								//layers
		{room10_gd1,0,0},													//grids
		{room10_PAL,R10SPRPAL},										//palettes
		{0,0},																		//parallax layers
	},
	{
		3,																				//total_layers		//room 11
		640,																			//size_x
		400,																			//size_y
		56,																				//grid_width
		{room11_l0,room11_l1,room11_l2,0},				//layers
		{room11_gd1,room11_gd2,0},								//grids
		{room11_PAL,PARIS2_PAL},									//palettes
		{0,0},																		//parallax layers
	},
	{
		2,																				//total_layers		//room 12
		640,																			//size_x
		400,																			//size_y
		56,																				//grid_width
		{room12_l0,room12_l1,0,0},								//layers
		{room12_gd1,0,0},													//grids
		{room12_PAL,PARIS2_PAL},									//palettes
		{0,0},																		//parallax layers
	},
	{
		3,																				//total_layers		//room 13
		976,																			//size_x
		400,																			//size_y
		77,																				//grid_width
		{room13_l0,room13_l1,room13_l2,0},				//layers
		{room13_gd1,room13_gd2,0},								//grids
		{room13_PAL,R13SPRPAL},										//palettes
		{0,0},																		//parallax layers
	},
	{
		2,																				//total_layers		//room 14
		640,																			//size_x
		400,																			//size_y
		56,																				//grid_width
		{room14_l0,room14_l1,0,0},								//layers
		{room14_gd1,0,0},													//grids
		{room14_PAL,PARIS2_PAL},									//palettes
		{0,0},																		//parallax layers
	},
	{
		3,																				//total_layers		//room 15
		640,																			//size_x
		400,																			//size_y
		56,																				//grid_width
		{room15_l0,room15_l1,room15_l2,0},				//layers
		{room15_gd1,room15_gd2,0},								//grids
		{room15_PAL,PARIS2_PAL},									//palettes
		{0,0},																		//parallax layers
	},
	{
		2,																				//total_layers		//room 16
		640,																			//size_x
		400,																			//size_y
		56,																				//grid_width
		{R16L0,R16L1,0,0},												//layers
		{R16G1,0,0},															//grids
		{room16_PAL,PARIS2_PAL},									//palettes
		{0,0},																		//parallax layers
	},
	{
		3,																				//total_layers		//room 17
		640,																			//size_x
		400,																			//size_y
		56,																				//grid_width
		{room17_l0,room17_l1,room17_l2,0},				//layers
		{room17_gd1,room17_gd2,0},								//grids
		{room17_PAL,PARIS2_PAL},									//palettes
		{0,0},																		//parallax layers
	},
	{
		3,																				//total_layers		//room 18
		640,																			//size_x
		400,																			//size_y
		56,																				//grid_width
		{room18_l0,room18_l1,room18_l2,0},				//layers
		{room18_gd1,room18_gd2,0},								//grids
		{room18_PAL,R18SPRPAL},										//palettes
		{0,0},																		//parallax layers
	},

	//------------------------------------------------------------------------
	// IRELAND

	{
		3,													//total_layers		//room 19 - Ireland Street
		848,												//size_x
		864,												//size_y
		69,													//grid_width
		{R19L0,R19L1,R19L2,0},			//layers
		{R19G1,R19G2,0},						//grids
		{R19PAL,R19SPRPAL},					//palettes
		{0,0},											//parallax layers
	},
	{
		4,													//total_layers		//room 20 - Macdevitts
		640,												//size_x
		400,												//size_y
		56,													//grid_width
		{R20L0,R20L1,R20L2,R20L3},	//layers
		{R20G1,R20G2,R20G3},				//grids
		{R20PAL,R20SPRPAL},					//palettes
		{0,0},											//parallax layers
	},
	{
		3,													//total_layers		//room 21 - Pub Cellar
		640,												//size_x
		400,												//size_y
		56,													//grid_width
		{R21L0,R21L1,R21L2,0},			//layers
		{R21G1,R21G2,0},						//grids
		{R21PAL,SPRITE_PAL},				//palettes
		{0,0},											//parallax layers
	},
	{
		2,													//total_layers		//room 22 - Castle Gate
		784,												//size_x
		400,												//size_y
		65,													//grid_width
		{R22L0,R22L1,0,0},					//layers
		{R22G1,0,0},								//grids
		{R22PAL,R22SPRPAL},					//palettes
		{0,0},											//parallax layers
	},
	{
		1,													//total_layers		//room 23 - Castle Hay Top
		640,												//size_x
		400,												//size_y
		56,													//grid_width
		{R23L0,0,0,0},							//layers
		{0,0,0},										//grids
		{R23PAL,SPRITE_PAL},				//palettes
		{0,0},											//parallax layers
	},
	{
		2,													//total_layers		//room 24 - Castle Yard
		880,												//size_x
		400,												//size_y
		71,													//grid_width
		{R24L0,R24L1,0,0},					//layers
		{R24G1,0,0},								//grids
		{R24PAL,SPRITE_PAL},				//palettes
		{R24PLX,0},									//parallax layers
	},
	{
		2,													//total_layers		//room 25 - Castle Dig
		640,												//size_x
		400,												//size_y
		56,													//grid_width
		{R25L0,R25L1,0,0},					//layers
		{R25G1,0,0},								//grids
		{R25PAL,R25SPRPAL},					//palettes
		{0,0},											//parallax layers
	},
	{
		3,													//total_layers		//room 26 - Cellar Dark
		640,												//size_x
		400,												//size_y
		56,													//grid_width
		{R26L0,R26L1,R26L2,0},			//layers
		{R26G1,R26G2,0},						//grids
		{R26PAL,R26SPRPAL},					//palettes
		{0,0},											//parallax layers
	},

	//------------------------------------------------------------------------
	// PARIS 3

	{
		3,																					//total_layers		//room 27
		640,																				//size_x
		400,																				//size_y
		56,																					//grid_width
		{R27L0,R27L1,R27L2,0},											//layers
		{R27G1,R27G2,0},														//grids
		{room27_PAL,SPRITE_PAL},										//palettes
		{0,0},																			//parallax layers
	},
	{
		3,																					//total_layers		//room 28
		640,																				//size_x
		400,																				//size_y
		56,																					//grid_width
		{R28L0,R28L1,R28L2,0},											//layers
		{R28G1,R28G2,0},														//grids
		{R28PAL,R28SPRPAL},													//palettes
		{0,0},																			//parallax layers
	},
	{
		2,																					//total_layers		//room 29
		640,																				//size_x
		400,																				//size_y
		56,																					//grid_width
		{R29L0,R29L1,0,0},													//layers
		{R29G1,0,0},																//grids
		{R29PAL,R29SPRPAL},													//palettes
		{0,0},																			//parallax layers
	},
	{
		1,																					//total_layers		//room 30 - for MONITOR seen while player in rm34
		640,																				//size_x
		400,																				//size_y
		56,																					//grid_width
		{MONITOR,0,0,0},														//layers
		{0,0,0},																		//grids
		{MONITOR_PAL,PARIS3_PAL},										//palettes
		{0,0},																			//parallax layers
	},
	{
		1,																					//total_layers		//room 31
		640,																				//size_x
		400,																				//size_y
		56,																					//grid_width
		{room31_l0,0,0,0},													//layers
		{0,0,0},																		//grids
		{room31_PAL,PARIS3_PAL},										//palettes
		{0,0},																			//parallax layers
	},
	{
		3,																					//total_layers		//room 32
		640,																				//size_x
		400,																				//size_y
		56,																					//grid_width
		{room32_l0,room32_l1,room32_l2,0},					//layers
		{room32_gd1,room32_gd2,0},									//grids
		{room32_PAL,PARIS3_PAL},										//palettes
		{0,0},																			//parallax layers
	},
	{
		3,																					//total_layers		//room 33
		640,																				//size_x
		400,																				//size_y
		56,																					//grid_width
		{room33_l0,room33_l1,room33_l2,0},					//layers
		{room33_gd1,room33_gd2,0},									//grids
		{room33_PAL,PARIS3_PAL},										//palettes
		{0,0},																			//parallax layers
	},
	{
		4,																					//total_layers		//room 34
		1120,																				//size_x
		400,																				//size_y
		86,																					//grid_width
		{room34_l0,room34_l1,room34_l2,room34_l3},	//layers
		{room34_gd1,room34_gd2,room34_gd3},					//grids
		{room34_PAL,PARIS3_PAL},										//palettes
		{R34PLX,0},																	//parallax layers
	},
	{
		2,																					//total_layers		//room 35
		640,																				//size_x
		400,																				//size_y
		56,																					//grid_width
		{room35_l0,room35_l1,0},										//layers
		{room35_gd1,0},															//grids
		{room35_PAL,PARIS3_PAL},										//palettes
		{0,0},																			//parallax layers
	},

	//------------------------------------------------------------------------
	// PARIS 4

	{
		2,													//total_layers		//room 36
		960,												//size_x
		400,												//size_y
		76,													//grid_width
		{R36L0,R36L1,0,0},					//layers
		{R36G1,0,0},								//grids
		{room36_PAL,R36SPRPAL},			//palettes
		{R36PLX,0},									//parallax layers
	},
	{
		2,													//total_layers		//room 37
		640,												//size_x
		400,												//size_y
		56,													//grid_width
		{R37L0,R37L1,0,0},					//layers
		{R37G1,0,0},								//grids
		{room37_PAL,SPRITE_PAL},		//palettes
		{0,0},											//parallax layers
	},
	{
		2,													//total_layers		//room 38
		640,												//size_x
		400,												//size_y
		56,													//grid_width
		{R38L0,R38L1,0,0},					//layers
		{R38G1,0,0},								//grids
		{room38_PAL,R38SPRPAL},			//palettes
		{0,0},											//parallax layers
	},
	{
		2,													//total_layers		//room 39
		640,												//size_x
		400,												//size_y
		56,													//grid_width
		{R39L0,R39L1,0,0},					//layers
		{R39G1,0,0},								//grids
		{room39_PAL,R39SPRPAL},			//palettes
		{0,0},											//parallax layers
	},
	{
		1,													//total_layers		//room 40
		640,												//size_x
		400,												//size_y
		56,													//grid_width
		{R40L0,0,0,0},							//layers
		{0,0,0},										//grids
		{room40_PAL,SPRITE_PAL},		//palettes
		{0,0},											//parallax layers
	},
	{
		2,													//total_layers		//room 41
		640,												//size_x
		400,												//size_y
		56,													//grid_width
		{R41L0,R41L1,0,0},					//layers
		{R41G1,0,0},								//grids
		{room41_PAL,SPRITE_PAL},		//palettes
		{0,0},											//parallax layers
	},
	{
		3,													//total_layers		//room 42
		640,												//size_x
		400,												//size_y
		56,													//grid_width
		{R42L0,R42L1,R42L2,0},			//layers
		{R42G1,R42G2,0},						//grids
		{room42_PAL,SPRITE_PAL},		//palettes
		{0,0},											//parallax layers
	},
	{
		2,													//total_layers		//room 43
		640,												//size_x
		400,												//size_y
		56,													//grid_width
		{R43L0,R43L1,0,0},					//layers
		{R43G1,0,0},								//grids
		{room43_PAL,SPRITE_PAL},		//palettes
		{0,0},											//parallax layers
	},
	{
		0,													//total_layers		//room 44
		0,													//size_x
		0,													//size_y
		0,													//grid_width
		{0,0,0,0},									//layers
		{0,0,0},										//grids
		{0,0},											//palettes
		{0,0},											//parallax layers
	},

	//------------------------------------------------------------------------
	// SYRIA

	{
		2,													//total_layers		//room 45 - Syria Stall
		1152,												//size_x
		400,												//size_y
		88,													//grid_width
		{R45L0,R45L1,0,0},					//layers
		{R45G1,0,0},								//grids
		{R45PAL,R45SPRPAL},					//palettes
		{R45PLX,0},									//parallax layers
	},
	{
		3,																	//total_layers		//room 46 (Hotel Alley, Paris 2)
		640,																//size_x
		400,																//size_y
		56,																	//grid_width
		{room46_l0,room46_l1,room46_l2,0},	//layers
		{room46_gd1,room46_gd2,0},					//grids
		{room46_PAL,PARIS2_PAL},						//palettes
		{0,0},															//parallax layers
	},
	{
		3,													//total_layers		//room 47 - Syria Carpet
		640,												//size_x
		800,												//size_y
		56,													//grid_width
		{R47L0,R47L1,R47L2,0},			//layers
		{R47G1,R47G2,0},						//grids
		{R47PAL,SYRIA_PAL},					//palettes
		{0,0},											//parallax layers
	},
	{
		3,													//total_layers		//room 48 (Templar Church, Paris 4)
		1184,												//size_x
		400,												//size_y
		90,													//grid_width
		{R48L0,R48L1,R48L2,0},			//layers
		{R48G1,R48G2,0},						//grids
		{R48PAL,R48SPRPAL},					//palettes
		{R48PLX,0},									//parallax layers
	},
	{
		3,													//total_layers		//room 49 - Syria Club
		640,												//size_x
		400,												//size_y
		56,													//grid_width
		{R49L0,R49L1,R49L2,0},			//layers
		{R49G1,R49G2,0},						//grids
		{R49PAL,SYRIA_PAL},					//palettes
		{0,0},											//parallax layers
	},
	{
		4,													//total_layers		//room 50 - Syria Toilet
		640,												//size_x
		400,												//size_y
		56,													//grid_width
		{R50L0,R50L1,R50L2,R50L3},	//layers
		{R50G1,R50G2,R50G3},				//grids
		{R50PAL,SYRIA_PAL},					//palettes
		{0,0},											//parallax layers
	},
	{
		0,													//total_layers		//room 51 - NOT USED
		0,													//size_x
		0,													//size_y
		0,													//grid_width
		{0,0,0,0},									//layers
		{0,0,0},										//grids
		{0,0},											//palettes
		{0,0},											//parallax layers
	},
	{
		0,													//total_layers		//room 52 - NOT USED
		0,													//size_x
		0,													//size_y
		0,													//grid_width
		{0,0,0,0},									//layers
		{0,0,0},										//grids
		{0,0},											//palettes
		{0,0},											//parallax layers
	},
	{
		1,													//total_layers		//room 53 - Bull Head Pan
		880,												//size_x
		1736,												//size_y
		71,													//grid_width
		{R53L0,0,0,0},							//layers
		{0,0,0},										//grids
		{R53PAL,R53SPRPAL},					//palettes
		{FRONT53PLX,BACK53PLX},			//parallax layers
	},
	{
		2,													//total_layers		//room 54 - Bull Head
		896,												//size_x
		1112,												//size_y
		72,													//grid_width
		{R54L0,R54L1,0,0},					//layers
		{R54G1,0,0},								//grids
		{R54PAL,SYRIA_PAL},					//palettes
		{R54PLX,0},									//parallax layers - SPECIAL BACKGROUND PARALLAX - MUST GO IN FIRST SLOT
	},
	{
		1,													//total_layers		//room 55 - Bull Secret
		1040,												//size_x
		400,												//size_y
		81,													//grid_width
		{R55L0,0,0,0},							//layers
		{0,0,0},										//grids
		{R55PAL,R55SPRPAL},					//palettes
		{R55PLX,0},									//parallax layers
	},

	//------------------------------------------------------------------------
	// SPAIN

	{
		3,													//total_layers		//room 56 - Countess' room
		640,												//size_x
		400,												//size_y
		56,													//grid_width
		{R56L0,R56L1,R56L2,0},			//layers
		{R56G1,R56G2,0},						//grids
		{R56PAL,SPAIN_PAL},					//palettes
		{0,0},											//parallax layers
	},
	{
		2,													//total_layers		//room 57 - Spain Drive
		1760,												//size_x
		400,												//size_y
		126,												//grid_width
		{R57L0,R57L1,0,0},					//layers
		{R57G1,0,0},								//grids
		{R57PAL,SPAIN_PAL},					//palettes
		{R57PLX,0},									//parallax layers
	},
	{
		2,													//total_layers		//room 58 - Mausoleum Exterior
		864,												//size_x
		400,												//size_y
		70,													//grid_width
		{R58L0,R58L1,0,0},					//layers
		{R58G1,0,0},								//grids
		{R58PAL,SPAIN_PAL},					//palettes
		{R58PLX,0},									//parallax layers
	},
	{
		3,													//total_layers		//room 59 - Mausoleum Interior
		640,												//size_x
		400,												//size_y
		56,													//grid_width
		{R59L0,R59L1,R59L2,0},			//layers
		{R59G1,R59G2,0},						//grids
		{R59PAL,SPAIN_PAL},					//palettes
		{0,0},											//parallax layers
	},
	{
		3,													//total_layers		//room 60 - Spain Reception
		640,												//size_x
		400,												//size_y
		56,													//grid_width
		{R60L0,R60L1,R60L2,0},			//layers
		{R60G1,R60G2,0},						//grids
		{R60PAL,SPAIN_PAL},					//palettes
		{0,0},											//parallax layers
	},
	{
		2,													//total_layers		//room 61 - Spain Well
		640,												//size_x
		400,												//size_y
		56,													//grid_width
		{R61L0,R61L1,0,0},					//layers
		{R61G1,0,0},								//grids
		{R61PAL,SPAIN_PAL},					//palettes
		{0,0},											//parallax layers
	},
	{
		1,													//total_layers		//room 62 - CHESS PUZZLE
		640,												//size_x
		400,												//size_y
		56,													//grid_width
		{R62L0,0,0,0},							//layers
		{0,0,0},										//grids
		{R62PAL,SPRITE_PAL},				//palettes
		{0,0},											//parallax layers
	},

	//------------------------------------------------------------------------
	// NIGHT TRAIN

	{
		2,													//total_layers		//room 63 - train_one
		2160,												//size_x
		400,												//size_y
		151,												//grid_width
		{R63L0,R63L1,0,0},					//layers
		{R63G1,0,0},								//grids
		{R63PAL,TRAIN_PAL},					//palettes
		{0,0},											//parallax layers
	},
	{
		0,													//total_layers		//room 64 - NOT USED
		0,													//size_x
		0,													//size_y
		0,													//grid_width
		{0,0,0,0},									//layers
		{0,0,0},										//grids
		{0,0},											//palettes
		{0,0},											//parallax layers
	},
	{
		2,													//total_layers		//room 65 - compt_one
		640,												//size_x
		400,												//size_y
		56,													//grid_width
		{R65L0,R65L1,0,0},					//layers
		{R65G1,0,0},								//grids
		{R65PAL,TRAIN_PAL},					//palettes
		{0,0},											//parallax layers
	},
	{
		1,													//total_layers		//room 66 - compt_two
		640,												//size_x
		400,												//size_y
		56,													//grid_width
		{R66L0,0,0,0},							//layers
		{0,0,0},										//grids
		{R66PAL,TRAIN_PAL},					//palettes
		{0,0},											//parallax layers
	},
	{
		2,													//total_layers		//room 67 - compt_three
		640,												//size_x
		400,												//size_y
		56,													//grid_width
		{R67L0,R67L1,0,0},					//layers
		{R67G1,0,0},								//grids
		{R67PAL,TRAIN_PAL},					//palettes
		{0,0},											//parallax layers
	},
	{
		0,													//total_layers		//room 68 - NOT USED
		0,													//size_x
		0,													//size_y
		0,													//grid_width
		{0,0,0,0},									//layers
		{0,0,0},										//grids
		{0,0},											//palettes
		{0,0},											//parallax layers
	},
	{
		2,													//total_layers		//room 69 - train_guard
		640,												//size_x
		400,												//size_y
		56,													//grid_width
		{R69L0,R69L1,0,0},					//layers
		{R69G1,0,0},								//grids
		{R69PAL,R69SPRPAL},					//palettes
		{0,0},											//parallax layers
	},
	{
		0,													//total_layers		//room 70 - NOT USED
		0,													//size_x
		0,													//size_y
		0,													//grid_width
		{0,0,0,0},									//layers
		{0,0,0},										//grids
		{0,0},											//palettes
		{0,0},											//parallax layers
	},

	//------------------------------------------------------------------------
	// SCOTLAND

	{
		2,													//total_layers		//room 71 - churchyard
		1760,												//size_x
		400,												//size_y
		126,												//grid_width
		{R71L0,R71L1,0,0},					//layers
		{R71G1,0,0},								//grids
		{R71PAL,SPRITE_PAL},				//palettes
		{R71PLX,0},									//parallax layers
	},
	{
		2,													//total_layers		//room 72 - church_tower
		640,												//size_x
		400,												//size_y
		56,													//grid_width
		{R72L0,R72L1,0,0},					//layers
		{R72G1,0,0},								//grids
		{R72PAL,SPRITE_PAL},				//palettes
		{0,0},											//parallax layers
	},
	{
		3,													//total_layers		//room 73 - crypt
		640,												//size_x
		400,												//size_y
		56,													//grid_width
		{R73L0,R73L1,R73L2,0},			//layers
		{R73G1,R73G2,0},						//grids
		{R73PAL,R73SPRPAL},					//palettes
		{0,0},											//parallax layers
	},
	{
		2,													//total_layers		//room 74 - secret_crypt
		1136,												//size_x
		400,												//size_y
		87,													//grid_width
		{R74L0,R74L1,0,0},					//layers
		{R74G1,0,0},								//grids
		{R74PAL,ENDSPRPAL},					//palettes
		{0,0},											//parallax layers
	},
	{
		1,													//total_layers		//room 75 - secret_crypt
		640,												//size_x
		400,												//size_y
		56,													//grid_width
		{R75L0,0,0,0},							//layers
		{0,0,0},										//grids
		{R75PAL,ENDSPRPAL},					//palettes
		{0,0},											//parallax layers
	},
	{
		1,													//total_layers		//room 76 - secret_crypt
		640,												//size_x
		400,												//size_y
		56,													//grid_width
		{R76L0,0,0,0},							//layers
		{0,0,0},										//grids
		{R76PAL,ENDSPRPAL},					//palettes
		{0,0},											//parallax layers
	},
	{
		1,													//total_layers		//room 77 - secret_crypt
		640,												//size_x
		400,												//size_y
		56,													//grid_width
		{R77L0,0,0,0},							//layers
		{0,0,0},										//grids
		{R77PAL,ENDSPRPAL},					//palettes
		{0,0},											//parallax layers
	},
	{
		1,													//total_layers		//room 78 - secret_crypt
		640,												//size_x
		400,												//size_y
		56,													//grid_width
		{R78L0,0,0,0},							//layers
		{0,0,0},										//grids
		{R78PAL,ENDSPRPAL},					//palettes
		{0,0},											//parallax layers
	},
	{
		1,													//total_layers		//room 79 - secret_crypt
		640,												//size_x
		400,												//size_y
		56,													//grid_width
		{R79L0,0,0,0},							//layers
		{0,0,0},										//grids
		{R79PAL,ENDSPRPAL},					//palettes
		{0,0},											//parallax layers
	},

	//------------------------------------------------------------------------
	// MAPS

	{
		1,													//total_layers		//room 80 - paris map
		640,												//size_x
		400,												//size_y
		56,													//grid_width
		{room80_l0,0,0,0},					//layers
		{0,0,0},										//grids
		{room80_PAL,SPRITE_PAL},		//palettes
		{0,0},											//parallax layers
	},
	{
		1,													//total_layers		//room 81 - for sequence of Assassin coming up stairs to rm17
		640,												//size_x
		400,												//size_y
		56,													//grid_width
		{ASSTAIR2,0,0,0},						//layers
		{0,0,0},										//grids
		{ASSTAIR2_PAL,SPRITE_PAL},	//palettes
		{0,0},											//parallax layers
	},
	{
		1,													//total_layers		//room 82 - Map of Britain, viewed frrom sc55 (Syria Cave)
		640,												//size_x
		400,												//size_y
		56,													//grid_width
		{BRITMAP,0,0,0},						//layers
		{0,0,0},										//grids
		{BRITMAP_PAL,SPRITE_PAL},		//palettes
		{0,0},											//parallax layers
	},
	{
		0,													//total_layers		//room 83 - NOT USED
		0,													//size_x
		0,													//size_y
		0,													//grid_width
		{0,0,0,0},									//layers
		{0,0,0},										//grids
		{0,0},											//palettes
		{0,0},											//parallax layers
	},
	{
		0,													//total_layers		//room 84 - NOT USED
		0,													//size_x
		0,													//size_y
		0,													//grid_width
		{0,0,0,0},									//layers
		{0,0,0},										//grids
		{0,0},											//palettes
		{0,0},											//parallax layers
	},
	{
		0,													//total_layers		//room 85 - NOT USED
		0,													//size_x
		0,													//size_y
		0,													//grid_width
		{0,0,0,0},									//layers
		{0,0,0},										//grids
		{0,0},											//palettes
		{0,0},											//parallax layers
	},
	{
		1,													//total_layers		//room 86 - europe map
		640,												//size_x
		400,												//size_y
		56,													//grid_width
		{room86_l0,0,0,0},					//layers
		{0,0,0},										//grids
		{room86_PAL,SPRITE_PAL},		//palettes
		{0,0},											//parallax layers
	},
	{
		1,													//total_layers		//room 87 - normal window in sc48
		640,												//size_x
		400,												//size_y
		56,													//grid_width
		{WINDOW1,0,0,0},						//layers
		{0,0,0},										//grids
		{WINDOW1_PAL,SPRITE_PAL},		//palettes
		{0,0},											//parallax layers
	},
	{
		1,													//total_layers		//room 88 - filtered window in sc48
		640,												//size_x
		400,												//size_y
		56,													//grid_width
		{WINDOW2,0,0,0},						//layers
		{0,0,0},										//grids
		{WINDOW2_PAL,SPRITE_PAL},		//palettes
		{0,0},											//parallax layers
	},
	{
		0,													//total_layers		//room 89 - NOT USED
		0,													//size_x
		0,													//size_y
		0,													//grid_width
		{0,0,0,0},									//layers
		{0,0,0},										//grids
		{0,0},											//palettes
		{0,0},											//parallax layers
	},
	{
		1,													//total_layers		//room 90 - phone screen
		640,												//size_x
		400,												//size_y
		56,													//grid_width
		{R90L0,0,0,0},							//layers
		{0,0,0},										//grids
		{R90PAL,PHONE_PAL},					//palettes
		{0,0},											//parallax layers
	},
	{
		1,													//total_layers		//room 91 - envelope screen
		640,												//size_x
		400,												//size_y
		56,													//grid_width
		{R91L0,0,0,0},							//layers
		{0,0,0},										//grids
		{R91PAL,SPRITE_PAL},					//palettes
		{0,0},											//parallax layers
	},
	{
		1,													//total_layers		//room 92 - for close up of George surprised in wardrobe in sc17
		640,												//size_x
		400,												//size_y
		56,													//grid_width
		{SBACK17,0,0,0},						//layers
		{0,0,0},										//grids
		{SBACK17PAL,SPRITE_PAL},		//palettes
		{0,0},											//parallax layers
	},
	{
		1,													//total_layers		//room 93 - for close up of George inquisitive in wardrobe in sc17
		640,												//size_x
		400,												//size_y
		56,													//grid_width
		{IBACK17,0,0,0},						//layers
		{0,0,0},										//grids
		{IBACK17PAL,SPRITE_PAL},		//palettes
		{0,0},											//parallax layers
	},
	{
		1,													//total_layers		//room 94 - for close up of George in sarcophagus in sc29
		640,												//size_x
		400,												//size_y
		56,													//grid_width
		{BBACK29,0,0,0},						//layers
		{0,0,0},										//grids
		{BBACK29PAL,BBACK29SPRPAL},	//palettes
		{0,0},											//parallax layers
	},
	{
		1,													//total_layers		//room 95 - for close up of George during templar meeting, in sc38
		640,												//size_x
		400,												//size_y
		56,													//grid_width
		{BBACK38,0,0,0},						//layers
		{0,0,0},										//grids
		{BBACK38PAL,SPRITE_PAL},		//palettes
		{0,0},											//parallax layers
	},
	{
		1,													//total_layers		//room 96 - close up of chalice projection
		640,												//size_x
		400,												//size_y
		56,													//grid_width
		{CHALICE42,0,0,0},					//layers
		{0,0,0},										//grids
		{CHALICE42_PAL,SPRITE_PAL},	//palettes
		{0,0},											//parallax layers
	},
	{
		0,													//total_layers		//room 97 - NOT USED
		0,													//size_x
		0,													//size_y
		0,													//grid_width
		{0,0,0,0},									//layers
		{0,0,0},										//grids
		{0,0},											//palettes
		{0,0},											//parallax layers
	},
	{
		0,													//total_layers		//room 98 - NOT USED
		0,													//size_x
		0,													//size_y
		0,													//grid_width
		{0,0,0,0},									//layers
		{0,0,0},										//grids
		{0,0},											//palettes
		{0,0},											//parallax layers
	},
	{
		1,													//total_layers		//room 99 - blank screen
		640,												//size_x
		400,												//size_y
		56,													//grid_width
		{room99_l0,0,0,0},					//layers
		{0,0,0},										//grids
		{room99_PAL,SPRITE_PAL},		//palettes
		{0,0},											//parallax layers
	}
};

const char Music::_tuneList[TOTAL_TUNES][8] = {
	"",				// 0	SPARE
	"1m2",		// DONE 1	George picks up the newspaper
	"1m3",		// DONE 2	In the alley for the first time
	"1m4",		// DONE 3	Alleycat surprises George
	"1m6",		// DONE 4	George fails to remove manhole cover. Even numbered attempts
	"1m7",		// !!!! 5	George fails to remove manhole cover. Odd numbered attempts
	"1m8",		// DONE 6	George leaves alley
	"1m9",		// DONE	7	George enters cafe for the first time
	"1m10",		// DONE 8	Waitress
	"1m11",		// DONE 9	Lying doctor

	"1m12",		// DONE 10	Truthful George
	"1m13",		// DONE 11	Yes, drink brandy
	"1m14",		// DONE 12	Yes, he's dead (Maybe 1m14a)
	"1m15",		// DONE 13	From, "...clown entered?"
	"1m16",		// DONE 14	From, "How did the old man behave?"
	"1m17",		// DONE 15	Salah-eh-Din
	"1m18",		// DONE 16	From, "Stay here, mademoiselle"
	"1m19",		// DONE 17	Leaving the cafe
	"1m20",		// DONE 18	Stick-up on Moue's gun
	"1m21",		// DONE 19	From, "Stop that, monsieur"

	"1m22",		// DONE 20	From, "If you can"
	"1m23",		// DONE 21	From, "Yeah,...clown"
	"1m24",		// DONE 22	From, he claimed to be a doctor
	"1m25",		// DONE 23	First time George meets Nico
	"1m26",		// DONE 24	From, "Oh God, him again." (Read notes on list)
	"1m27",		// DONE 25	From, "He's inside"
	"1m28",		// DONE 26	From, "You speak very good English"
	"1m29",		// DONE 27	From, "Why won't you tell me about this clown?"
	"1m28a",	// DONE 28	Costumed killers from, "How did Plantard get your name?"
	"1m30",		// DONE 29	From, "I really did see the clown" when talking to Moue at cafe doorway

	"1m31",		// DONE 30	From, "I found this (paper) in the street" (talking to Moue)
	"1m32",		// DONE 31	From, "What's the difference?"
	"1m34",		// DONE 32	Roadworker "Did you see a clown?"
	"1m35",		// DONE 33	Worker re: explosion, "I guess not"
	"2m1",		// DONE 34	From, "What about the waitress?"
	"2m2",		// DONE 35	From, "Did you see the old guy with the briefcase?"
	"2m4",		// DONE 36	"Would you like to read my newspaper?" (2M3 is at position 144)
	"2m5",		// DONE 37	From, "Ah, what's this Saleh-eh-Din?"
	"2m6",		// DONE 38	From, "It was a battered old tool box".
	"2m7",		// DONE 39	George "borrows" the lifting key

	"2m8",		// DONE 40	From 'phone page. Call Nico
	"2m9",		// DONE 41	Leaving the workman's tent
	"2m10",		// DONE 42	Use lifting keys on manhole
	"2m11",		// DONE 43	Into sewer no.1 from George on his knees (Alternative: 2m12)
	"2m12",		// DONE 44	Into sewer (alternative to 2m11)
	"2m13",		// DONE 45	George bends to pick up the red nose
	"2m14",		// DONE 46	Paper tissue, "It was a soggy..."
	"2m15",		// DONE 47	Cloth, as George picks it up. (Alternative: 2m16)
	"2m16",		// !!!! 48	Alternative cloth music
	"2m17",		// DONE 49	George climbs out of sewer.

	"2m18",		// DONE 50	From, "The man I chased..."
	"2m19",		// DONE 51	Spooky music for sewers.
	"2m20",		// DONE 52	"She isn't hurt, is she?"
	"2m21",		// DONE 53	Click on material icon
	"2m22",		// DONE 54	Spooky music for sewers.
	"2m23",		// DONE 55	From, "So you don't want to hear my experiences in the desert?"
	"2m24",		// DONE 56	On the material icon with Albert (suit icon instead, because material icon done)
	"2m25",		// DONE 57	After "What was on the label?" i.e. the 'phone number.
	"2m26",		// DONE 58	Leaving yard, after, "I hope you catch that killer soon." Also for the Musee Crune icon on the map (5M7).
	"2m27",		// DONE 59	As George starts to 'phone Todryk.  (Repeated every time he calls him). Also, when the aeroport is clicked on (5M21).

	"2m28",		// DONE 60	Todryk conversation after, "Truth and justice"
	"2m29",		// DONE 61	'Phoning Nico from the roadworks. Also, 'phoning her from Ireland, ideally looping and fading on finish (6M10).
	"2m30",		// DONE 62	First time on Paris map
	"2m31",		// DONE 63	Click on Rue d'Alfred Jarry
	"2m32",		// DONE 64	From, "Will you tell me my fortune?"
	"2m33",		// DONE 65	After "Can you really tell my future?"
	"1m28",		// DONE 66	"What about the tall yellow ones?" Copy from 1M28.
	"2m24",		// DONE 67	Material Icon. Copy from 2M24
	"2m6",		// DONE 68	Exit icon on "See you later". Copy from 2M6.
	"1m25",		// DONE 69	On opening the front foor to Nico's. Copy from 1M25. .

	"2m38",		// DONE 70	Victim 1: From, "Tell me more about the clown's previous victims."
	"2m39",		// DONE 71	Victim 2: After, "What about the clown's second victim?"
	"2m40",		// DONE 72	Victim 3: On clown icon for 3rd victim.
	"3m1",		// DONE 73	George passes Nico the nose.
	"3m2",		// DONE 74	With Nico. From, "I found a piece of material..."
	"3m3",		// DONE 75	After George says, "... or clowns?"
	"3m4",		// DONE 76	After, "Did you live with your father?"
	"1m28",		// DONE 77	After, "Do you have a boyfriend?". Copy from 1M28.
	"2m26",		// DONE 78	After, "Good idea" (about going to costumier's). Copy from 2M26.
	"3m7",		// DONE 79	On costumier's icon on map.

	"3m8",		// DONE 80	Costumier's, after, "Come in, welcome."
	"3m9",		// DONE 81	On entering costumier's on later visits
	"3m10",		// DONE 82	After, "A description, perhaps."
	"2m13",		// DONE 83	Red nose icon at costumier's. Copy 2M13.
	"3m12",		// DONE 84	Tissue icon. Also, after Nico's "No, I write it (the magazine) 5M19.
	"3m13",		// DONE 85	Photo icon over, "Do you recognise this man?"
	"3m14",		// DONE 86	Exit icon, over, "Thanks for your help, buddy."
	"2m9",		// DONE 87	Clicking on police station on the map.
	"3m17",		// DONE 88	Police station on, "I've tracked down the clown's movements."
	"3m18",		// DONE 89	"One moment, m'sieur," as Moue turns.

	"3m19",		// DONE 90	G. on Rosso. "If he was trying to impress me..."
	"3m20",		// DONE 91	G. thinks, "He looked at me as if I'd farted."
	"3m21",		// DONE 92	Over Rosso's, "I've washed my hands of the whole affair."
	"3m22",		// DONE 93	Played over, "So long, inspector."
	"3m24",		// DONE 94	Conversation with Todrk, "He bought a suit from you, remember?"
	"3m26",		// !!!! 95	This piece is a problem. Don't worry about it for present.
	"3m27",		// DONE 96	George to Nico (in the flat): "Have you found out anymore?" [about the murders? or about the templars? JEL]
	"2m26",		// DONE 97	After, "Don't worry, I will." on leaving Nico's.
	"3m29",		// DONE 98	Ubu icon on the map.
	"3m30",		// DONE 99	G and Flap. After, "I love the clowns. Don't you?" AND "after "Not if you see me first" (3M31)

	"3m32",		// DONE 100	Source music for Lady Piermont.
	"3m33",		// DONE 101	More music for Lady P.
	"2m13",		// DONE 102 Red Nose music Copy 2M13
	"4m3",		// DONE 103	On photo, "Do you recognise the man in this photograph"
	"4m4",		// DONE 104	With Lady P. After, "Hi there, ma'am."
	"4m5",		// DONE 105	After, "I think the word you're looking for is...dick"
	"4m6",		// DONE 106	After, "English arrogance might do the trick." Also for "More English arrogance" (4M27)
	"4m8",		// !!!! 107	As George grabs key.
	"4m9",		// DONE 108	Room 21, on "Maybe it wasn't the right room"
	"4m10",		// DONE 109	On coming into 21 on subsequent occasions.

	"4m11",		// DONE 110 As George steps upto window.
	"4m12",		// DONE 111	Alternative times he steps up to the window.
	"4m13",		// DONE 112	In Moerlin's room
	"4m14",		// DONE 113	Sees "Moerlin" on the Stairs
	"4m15",		// DONE 114	George closing wardrobe door aftre Moerlin's gone.
	"4m17",		// DONE 115	After, "take your mind off the inquest"
	"4m18",		// DONE 116	"It was just as I'd imagined."
	"4m19",		// DONE 117	Show photo to Lady P
	"4m20",		// DONE 118	Lady P is "shocked" after the name "Khan".
	"4m21",		// DONE 119	After, "A bundle of papers, perhaps".

	"4m22",		// DONE 120	After, "Plantard's briefcase"
	"4m24",		// DONE 121	On fade to black as George leaves the hotel (prior to being searched)
	"4m25",		// DONE 122	After, "I break your fingers"
	"4m28",		// DONE 123	After clerk says, "Voila, m'sieur. Le manuscript..."
	"4m29",		// DONE 124	Onto the window sill after getting the manuscript.
	"4m31",		// DONE 125	Searched after he's dumped the manuscript in the alleyway.
	"4m32",		// DONE 126	Recovering the manuscript in the alley, "If the manuscript was..."
	"5m1",		// DONE 127	The manuscript, just after, "It's worth enough to kill for."
	"5m2",		// !SMK 128 The Templars after, "...over 800 years ago."
	"5m3",		// DONE 129	After, "Let's take another look at that manuscript"

	"5m4",		// DONE 130	On "Knight with a crystal ball" icon
	"5m5",		// DONE 131	On Nico's, "Patience"
	"5m6",		// DONE 132	After "I'm sure it will come in useful" when George leaves. Also, George leaving Nico after, "Keep me informed if you find anything new" (5M20). + "just take care of yourself"
	"5m8",		// DONE 133	Entering the museum for the first time on the fade.
	"5m9",		// DONE 134	George with guard after, "park their cars." Guard saying "No, no, no"
	"5m10",		// DONE 135	Incidental looking around the museum music. + fading from map to museum street, when lobineau is in museum
	"5m11",		// DONE 136	From "In the case was a spindly tripod, blackened with age and pitted with rust...". George answers Tripod ((?)That's what the cue list says). Also 5M15 and 5M16)
	"5m12",		// DONE 137	More looking around music.
	"5m13",		// DONE 138	Opening the mummy case.
	"5m14",		// DONE 139	High above me was a window

	"5m17",		// DONE 140	"As I reached toward the display case" (5M18 is in slot 165)
	"5m22",		// !SMK 141	From Ireland on the Europe map.
	"5m23",		// !!!! 142	IN front of the pub, searching.
	"5m24",		// DONE 143	Cheeky Maguire, "Wait 'til I get back"
	"2m3",		// DONE 144	Before, "Did anybody at the village work at the dig?" Loop and fade.
	"6m1",		// DONE 145	After, "You know something ... not telling me, don't you?"
	"6m2",		// DONE 146	On, "Mister, I seen it with my own eyes."
	"6m3",		// DONE 147	After, "Did you get to see the ghost" + On George's, "As soon as I saw the flickering torches..." in SCR_SC73.txt.
	"6m4",		// DONE 148	"the bloody place is haunted", just after G's "rational explanation... the castle"
	"6m5",		// DONE 149	Pub fiddler 1. Please programme stops between numbers - about 20" and a longer one every four or five minutes.

	"6m6",		// DONE 150	Pub fiddler 2.
	"6m7",		// DONE 151	Pub fiddler 3.
	"6m8",		// DONE 152	Pub fiddler 4.
	"6m12",		// DONE 153	Exit pub (as door opens). Copy from 2M6.
	"2m6",		// DONE 154	Going to the castle, start on the path blackout.
	"5m1",		// DONE 155	On, "Where was the Templar preceptory?" Copy 5M1
	"6m15",		// DONE 156	"On, "Do you mind if I climb up your hay stack..."
	"7m1",		// DONE 157	On plastic box, "It was a featureless..."
	"7m2",		// DONE 158	On tapping the plastic box with the sewer key
	"7m4",		// !!!! 159	"Shame on you, Patrick!" Fitzgerald was at the dig

	"7m5",		// !!!! 160	On the icon that leads to, "Maguire says that he saw you at the dig"
	"7m6",		// !!!! 161	On "The man from Paris"
	"7m7",		// !!!! 162	On, "I wish I'd never heard of..."
	"7m8",		// DONE 163	Exit pub
	"7m11",		// DONE 164	George picks up gem.
	"7m14",		// DONE 165	On George's icon, "the driver of the Ferrari..."
	"7m15",		// DONE 166	After George, "His name is Sean Fitzgerald"
	"5m18",		// DONE 167	Leaving museum after discovering tripod.
	"6m11",		// !!!! 168	With Fitz. On G's, "Did you work at...?". This is triggered here and on each subsequent question, fading at the end.
	"7m17",		// DONE 169	"You don't have to demolish the haystack"

	"7m18",		// DONE 170	George begins to climb the haystack.
	"7m19",		// DONE 171	Alternative climbing haystack music. These two tracks can be rotated with an ascent with FX only).
	"7m20",		// DONE 172	Attempting to get over the wall.
	"7m21",		// DONE 173	Descending the haystack
	"7m22",		// !!!! 174	Useful general purpose walking about music.
	"7m23",		// DONE 175	"Plastic cover" The exposed box, LB and RB.
	"7m28",		// !!!! 176	"No return"
	"7m30",		// !!!! 177	Picking up drink music (This will definitely clash with the fiddle music. We'll use it for something else). *
	"7m31",		// !!!! 178	Showing the landlord the electrician's ID.
	"7m32",		// !!!! 179	Stealing the wire (Probable clash again) *

	"7m33",		// DONE 180	On fade to black before going down into dark cellar.
	"7m34",		// DONE 181	On opening the grate, as George says, "I lifted the..." Khan's entrance.
	"8m1",		// DONE 182	Going down into the light cellar, starting as he goes through bar door.
	"8m2",		// DONE 183	General cellar music on, "It was an empty carton".
	"8m4",		// !!!! 184	Trying to get the bar towel. On, "The man's arm lay across..." *
	"8m7",		// DONE 185	Squeeze towel into drain. On, "Silly boy..."
	"8m10",		// DONE 186	Entering the castle as he places his foot on the tool embedded into the wall.
	"8m11",		// DONE 187	On, "Hey, billy." Goat confrontation music.
	"8m12",		// DONE 188	First butt from goat at moment of impact.
	"8m13",		// DONE 189	On examining the plough.

	"8m14",		// DONE 190	Second butt from goat.
	"8m15",		// DONE 191	Third butt from goat.
	"8m16",		// DONE 192	All subsequent butts, alternating with no music.
	"8m18",		// DONE 193	Poking around in the excavation music. I'd trigger it as he starts to descend the ladder into the dig.
	"8m19",		// DONE 194	"There was a pattern..." The five holes.
	"8m20",		// DONE 195	George actually touches the stone. Cooling carving (?)
	"8m21",		// DONE 196	"As I swung the stone upright" coming in on "Upright"
	"8m22",		// DONE 197	"The sack contained"
	"8m24",		// DONE 198	Down wall. As screen goes black. George over wall to haystack.
	"8m26",		// DONE 199	Wetting the towel

	"8m28",		// DONE 200	Wetting plaster. As George reaches for the towel prior to wringing water onto the plaster.
	"8m29",		// DONE 201	Mould in "The hardened plaster..."
	"8m30",		// DONE 202	Entering castle. As George steps in.
	"8m31",		// DONE 203	After George, "Hardly - he was dead." in nico_scr.txt
	"8m37",		// !!!! 204	Talking to Lobineau about the Templars. 5M2Keep looping and fade at the end.
						//					The problem is that it's an enormous sample and will have to be reduced in volume.
						//					I suggest forgetting about this one for the time being.
						//					If there's room when the rest of the game's in, then I'll re-record it more quietly and call it 8M37, okay?
	"8m38",		// DONE 205	"A female friend"
	"8m39",		// DONE 206	"Public toilet"
	"8m40",		// DONE 207	When George asks, "Where was the site at Montfaucon?" (to Lobineau, I suppose)
	"8m41",		// DONE 208	On matchbox icon. "Does this matchbook mean anything to you?"
	"9m1",		// DONE 209	On George, "It was the king of France" in ross_scr.txt

	"9m2",		// DONE 210	George, "Why do you get wound up...?" in ross_scr.txt
	"9m3",		// DONE 211	Ever heard of a guy called Marquet? Jacques Marquet?
	"9m5",		// DONE 212	On fade up at the hospital drive
	"9m6",		// DONE 213	On fade up inside the hospital
	"9m7",		// DONE 214	With Eva talking about Marquet. Before, "I'm conducting a private investigation."
	"9m8",		// DONE 215	With Eva, showing her the ID card.
	"9m9",		// DONE 216	With Eva, second NURSE click, "If nurse Grendel is that bad..."
	"9m10",		// DONE 217	Saying goodbye to Eva on the conversation where he discovers Marquet's location + on fade up on Sam's screen after being kicked off the ward
	"9m11",		// DONE 218	Talking to Sam. On, "Oh - hiya!" + first click on MR_SHINY
	"9m13",		// DONE 219	When George drinks from the cooler.

	"9m14",		// DONE 220	To Grendel, third MARQUET click. On "Do you know who paid for Marquet's room?"
	"9m15",		// DONE 221	To Grendel on first CLOWN click, "Do you have any clowns on the ward?"
	"9m17",		// DONE 222	When George pulls Shiny's plug the first time, on "As I tugged the plug..."
	"9m18",		// DONE 223	On subsequent plug tuggings if George has failed to get the white coat.
	"9m19",		// DONE 224	With the consultant, on "Excuse me, sir..."
	"9m20",		// DONE 225	Talking to Grendel. Launch immediately after she gives him the long metal box and "a stunning smile"
	"9m21",		// DONE 226	On Eric's, "Doctor!" when George is trying to get by for the first time, i.e. ward_stop_flag==0.
	"9m22",		// DONE 227	On Eric's, "Oh, Doctor!" when George is trying to get by for the second time, i.e. ward_stop_flag==1.
	"9m23",		// DONE 228	On Eric's, "You haven't finished taking my blood pressure!" when George is trying to get by for the third+ time, i.e. ward_stop_flag>1.
	"9m24",		// DONE 229	Giving the pressure gauge to Benoir, on, "Here, take this pressure gauge."

	"9m25",		// DONE 230	With Benoir, suggesting he use the gauge on the nurse. On, "Use it on Nurse Grendel."
	"10m1",		// DONE 231	Immediately after Marquet's, "Well, what are you waiting for? Get it over with!"
	"10m2",		// DONE 232	When George pulls open the sarcophagus lid prior to his successful hiding before the raid.
	"10m3",		// DONE 233	On fade to black as George spies on the Neo-Templars.
	"10m4",		// DONE 234	On second peer through the hole at the "Templars"
	"11m1",		// DONE 235	On clicking on the Marib button.
	"11m3",		// DONE 236	Loop in the Club Alamut, alternating with...
	"11m4",		// DONE 237	Loop in the Club Alamut.
	"11m7",		// DONE 238	When the door in the Bull's Mouth closes on George.
	"11m8",		// DONE 239	When the door opens to reveal Khan, immediately after, "You!" in KHAN_55.TXT.

	"11m9",		// !SMK 240	Over the "Going to the Bull's Head" smacker. Probably.
	"12m1",		// DONE 241	Clicking on the Spain icon from the aeroport. (AFTER CHANGING CD!)
	"11m2",		// DONE 242	Loop in the marketplace of Marib.
	"spm2",		// DONE 243	On fade up in the Countess' room for the first time.
	"spm3",		// DONE 244	At the end of VAS1SC56, triggered immediately before the Countess says, "Senor Stobbart, if I find that you are wasting my time..."
	"spm4",		// DONE 245	Immediately before Lopez enters the mausoleum with the chess set.
	"spm5",		// DONE 246	(This is actually 5m2 copied for CD2) Played through the chess puzzle. Ideally, when it finsishes, give it a couple of seconds and then launch 12m1. When that finishes,  a couple of seconds and then back to this and so on and so forth.
	"spm6",		// DONE 247	On fade up from completing the chess puzzle. The climax is now "spm6b"
	"scm1",		// DONE 248	This is used whenever George goes out of a carriage and onto the corridor.
	"scm2",		// DONE 249	As George climbs out of the train window.

	"scm3",		// DONE 250	As George lands inside the guard's van.
	"scm4",		// DONE 251	On Khan's death speech, "A noble foe..."
	"scm5",		// DONE 252	George to Khan. On, "You're talking in riddles!"
	"scm6",		// DONE 253	Before, "He's dead"
	"scm7",		// DONE 254	Kissing Nico. After, "Where do you think you're going?"
	"scm8",		// DONE 255	In the churchyard after Nico's, "I rather hope it did"
	"scm11",	// DONE 256	Click on the opened secret door.
	"rm3a",		// DONE 257	Immediately they fade up in the great cave.
	"rm3b",		// DONE 258	The scene change immediately after Eklund says, "If you wish to live much longer..."
	"scm16",	// DONE 259	The big end sequence from when the torch hits the gunpowder. Cross fade with the shortened version that fits on the Smacker.

	"scm1b",	// DONE 260	When George passes the trigger point toward the back of the train and he sees Guido.
	"spm6b",	// DONE 261 The climax of "spm6", which should coincide with the Countess holding up the chalice.
	"marquet",	// DONE 262 Starts at the fade down when George is asked to leave Jacques' room
	"rm4",		// DONE 263 "On crystal stand icon. As George walks to the centre of the cavern." I'd do this on the first LMB on the stump.
	"rm5",		// DONE 264 "On icon. As George places the crystal on top of the stand." When the player places the gem luggage on the emplaced tripod.
	"rm6",		// DONE 265 "Chalice reflection. On icon as George places Chalice on floor of Church" i.e. the mosaic in the Baphomet dig. It's over thirty seconds long so it had best start running when the chalice luggage is placed on the mosaic so it runs through the big screen of the reflection.
	"rm7",		// DONE 266 "Burning candle. On icon as George sets about lighting candle." One minute forty-eight, this one. I've no idea how long the burning candle Smacker is but the cue description seems to indicate it should run from the moment the burning tissue successfully lights the candle, i.e. the window is shut.
	"rm8",		// DONE 267 "Down well. George descends into circus trap well." I think the circus reference refers to the lion. Run it from the moment that George gets off the rope and has a look around. Run it once only.

	"rm3c",		// DONE 268 On the scene change to the Grand Master standing between the pillars as the earth power whomps him.
	"rm3d",		// DONE 269 ONe the scene change after the Grand Master says, "George, we have watched you..." This one might need a bit of fiddling to get it to match to the fisticuffs.
};

const FxDef Sound::_fxList[312] = {
		// 0
	{
		{0,0,0},						// sampleId
		0,						// type				(FX_LOOP, FX_RANDOM or FX_SPOT)
		0,						// delay			(random chance for FX_RANDOM sound fx)
		{							// roomVolList
			{0,0,0},		// {roomNo,leftVol,rightVol}
		},
	},
	//------------------------
	// 1 Newton's cradle. Anim=NEWTON.
	{
		FX_NEWTON,		// sampleId
		FX_SPOT,			// type
		7,						// delay (or random chance)
		{							// roomVolList
			{45,4,2},		// {roomNo,leftVol,rightVol}
			{0,0,0},		// NULL-TERMINATOR
		},
	},
	//------------------------
	// 2
	{
		FX_TRAFFIC2,	// sampleId
		FX_LOOP,			// type
		0,						// delay (or random chance)
		{							// roomVolList
			{1,12,12},	// {roomNo,leftVol,rightVol}
			{2,1,1},
			{3,1,1},
			{4,13,13},
			{5,1,1},
			{8,7,7},
			{0,0,0},		// NULL-TERMINATOR
		},
	},
	//------------------------
	// 3
	{
		FX_HORN1,			// sampleId
		FX_RANDOM,		// type
		1200,					// delay (or random chance)
		{							// roomVolList
			{1,3,3},		// {roomNo,leftVol,rightVol}
			{3,1,1},
			{4,1,1},
			{5,2,2},
			{8,4,4},
			{18,2,3},
			{0,0,0},		// NULL-TERMINATOR
		},
	},
	//------------------------
	// 4
	{
		FX_HORN2,			// sampleId
		FX_RANDOM,		// type
		1200,					// delay (or random chance)
		{							// roomVolList
			{1,4,4},		// {roomNo,leftVol,rightVol}
			{3,2,2},
			{4,3,3},
			{5,2,2},
			{8,4,4},
			{18,1,1},
			{0,0,0},		// NULL-TERMINATOR
		},
	},
	//------------------------
	// 5
	{
		FX_HORN3,			// sampleId
		FX_RANDOM,		// type
		1200,					// delay (or random chance)
		{							// roomVolList
			{1,4,4},		// {roomNo,leftVol,rightVol}
			{2,4,4},
			{3,2,2},
			{4,3,3},
			{5,2,2},
			{8,4,4},
			{18,1,1},
		},
	},
	//------------------------
	// 6
	{
		FX_CAMERA1,		// sampleId
		FX_SPOT,			// type
		25,						// delay (or random chance)
		{							// roomVolList
			{1,8,8},		// {roomNo,leftVol,rightVol}
			{0,0,0},		// NULL-TERMINATOR
		},
	},
	//------------------------
	// 7
	{
		FX_CAMERA2,		// sampleId
		FX_SPOT,			// type
		25,						// delay (or random chance)
		{							// roomVolList
			{1,8,8},		// {roomNo,leftVol,rightVol}
			{0,0,0},		// NULL-TERMINATOR
		},
	},
	//------------------------
	// 8
	{
		FX_CAMERA3,		// sampleId
		FX_SPOT,			// type
		25,						// delay (or random chance)
		{							// roomVolList
			{1,8,8},		// {roomNo,leftVol,rightVol}
			{0,0,0},		// NULL-TERMINATOR
		},
	},
	//------------------------
	// 9
	{
		FX_SWATER1,		// sampleId
		FX_LOOP,			// type
		0,						// delay (or random chance)
		{							// roomVolList
			{7,12,12},	// {roomNo,leftVol,rightVol}
			{6,12,10},	// {roomNo,leftVol,rightVol}
			{0,0,0},		// NULL-TERMINATOR
		},
	},
	//------------------------
	// 10 Mad dogs in Spain, triggered by George going around the corner in the villa hall.
	// In 56 and 57, the dogs will continue barking after George has either been ejected or sneaked up stairs
	// for a couple of loops before stopping.
	{
		FX_DOGS56,		// sampleId
		FX_LOOP,			// type
		0,						// delay (or random chance)
		{							// roomVolList
			{60,16,16},	// {roomNo,leftVol,rightVol}
			{0,0,0}			// NULL-TERMINATOR
		},
	},
	//------------------------
	// 11
	{
		FX_DRIP1,			// sampleId
		FX_RANDOM,		// type
		20,						// delay (or random chance)
		{							// roomVolList
			{7,15,15},	// {roomNo,leftVol,rightVol}
			{6,8,8},		// {roomNo,leftVol,rightVol}
			{0,0,0},		// NULL-TERMINATOR
		},
	},
	//------------------------
	// 12
	{
		FX_DRIP2,			// sampleId
		FX_RANDOM,		// type
		30,						// delay (or random chance)
		{							// roomVolList
			{7,15,15},	// {roomNo,leftVol,rightVol}
			{6,8,8},		// {roomNo,leftVol,rightVol}
			{0,0,0},		// NULL-TERMINATOR
		},
	},
	//------------------------
	// 13
	{
		FX_DRIP3,			// sampleId
		FX_RANDOM,		// type
		40,						// delay (or random chance)
		{							// roomVolList
			{7,15,15},	// {roomNo,leftVol,rightVol}
			{6,8,8},		// {roomNo,leftVol,rightVol}
			{0,0,0},		// NULL-TERMINATOR
		},
	},
	//------------------------
	// 14
	{
		FX_TWEET1,		// sampleId
		FX_SPOT,			// type
		10,						// delay (or random chance)
		{							// roomVolList
			{1,5,5},		// {roomNo,leftVol,rightVol}
			{0,0,0},		// NULL-TERMINATOR
		},
	},
	//------------------------
	// 15
	{
		FX_TWEET2,		// sampleId
		FX_SPOT,			// type
		10,						// delay (or random chance)
		{							// roomVolList
			{1,5,5},		// {roomNo,leftVol,rightVol}
			{0,0,0},		// NULL-TERMINATOR
		},
	},
	//------------------------
	// 16
	{
		FX_TWEET3,		// sampleId
		FX_SPOT,			// type
		10,						// delay (or random chance)
		{							// roomVolList
			{1,5,5},		// {roomNo,leftVol,rightVol}
			{0,0,0},		// NULL-TERMINATOR
		},
	},
	//------------------------
	// 17
	{
		FX_TWEET4,		// sampleId
		FX_SPOT,			// type
		10,						// delay (or random chance)
		{							// roomVolList
			{1,5,5},		// {roomNo,leftVol,rightVol}
			{0,0,0},		// NULL-TERMINATOR
		},
	},
	//------------------------
	// 18
	{
		FX_TWEET5,		// sampleId
		FX_SPOT,			// type
		10,						// delay (or random chance)
		{							// roomVolList
			{1,5,5},		// {roomNo,leftVol,rightVol}
			{0,0,0},		// NULL-TERMINATOR
		},
	},
	//------------------------
	// 19 Tied to large bird flying up screen anim
	{
		FX_CAW1,			// sampleId
		FX_SPOT,			// type
		20,						// delay (or random chance)
		{							// roomVolList
			{1,5,5},		// {roomNo,leftVol,rightVol}
			{0,0,0},		// NULL-TERMINATOR
		},
	},
	//------------------------
	//------------------------
	// 20 George picking the canopy up: GEOCAN
	{
		FX_CANUP,			// sampleId
		FX_SPOT,			// type
		5,						// delay (or random chance) *
		{							// roomVolList
			{1,10,10},	// {roomNo,leftVol,rightVol}
			{0,0,0},		// NULL-TERMINATOR
		},
	},
	//------------------------
	// 21 George dropping the canopy: GEOCAN
	{
		FX_CANDO,			// sampleId
		FX_SPOT,			// type
		52,						// delay (or random chance) *
		{							// roomVolList
			{1,10,10},	// {roomNo,leftVol,rightVol}
			{0,0,0},		// NULL-TERMINATOR
		},
	},
	//------------------------
	// 22 George dusts himself down: GEOCAN
	{
		FX_DUST,			// sampleId
		FX_SPOT,			// type
		58,						// delay (or random chance) *
		{							// roomVolList
			{1,10,10},	// {roomNo,leftVol,rightVol}
			{0,0,0},		// NULL-TERMINATOR
		},
	},
	//------------------------
	// 23 George picks up the paper and opens it: GEOPAP
	{
		FX_PAP1,			// sampleId
		FX_SPOT,			// type
		23,						// delay (or random chance) *
		{							// roomVolList
			{1,10,10},	// {roomNo,leftVol,rightVol}
			{0,0,0},		// NULL-TERMINATOR
		},
	},
	//------------------------
	// 24 George puts the paper away: GEOPAP2
	{
		FX_PAP2,			// sampleId
		FX_SPOT,			// type
		3,						// delay (or random chance) *
		{							// roomVolList
			{1,10,10},	// {roomNo,leftVol,rightVol}
			{0,0,0},		// NULL-TERMINATOR
		},
	},
	//------------------------
	// 25 George gives the paper away: GEOWRK8
	{
		FX_PAP3,			// sampleId
		FX_SPOT,			// type
		13,						// delay (or random chance) *
		{							// roomVolList
			{4,14,12},	// {roomNo,leftVol,rightVol}
			{0,0,0},		// NULL-TERMINATOR
		},
	},
	//------------------------
	// 26 Workman examines paper: WRKOPN - it's now just WRKPPR
	{
		FX_PAP4,			// sampleId
		FX_SPOT,			// type
		15,						// delay (or random chance) *
		{							// roomVolList
			{4,14,12},	// {roomNo,leftVol,rightVol}
			{0,0,0},		// NULL-TERMINATOR
		},
	},
	//------------------------
	// 27 Workman puts paper down: WRKOPN (REVERSED) - now just WRKCLM
	{
		FX_PAP5,			// sampleId
		FX_SPOT,			// type
		2,						// delay (or random chance)*
		{							// roomVolList
			{4,14,12},	// {roomNo,leftVol,rightVol}
			{0,0,0},		// NULL-TERMINATOR
		},
	},
	//------------------------
	// 28 Pickaxe sound 1:, Screen 4 - WRKDIG
	{
		FX_PICK1,			// sampleId
		FX_SPOT,			// type
		11,						// delay (or random chance) *
		{							// roomVolList
			{4,10,10},
			{0,0,0}			// NULL-TERMINATOR
		},
	},
	//------------------------
	// 29 Pickaxe sound 2:, Screen 4 - WRKDIG
	{
		FX_PICK2,			// sampleId
		FX_SPOT,			// type
		11,						// delay (or random chance) *
		{							// roomVolList
			{4,10,10},
			{0,0,0},		// NULL-TERMINATOR
		},
	},
	//------------------------
	//------------------------
	// 30 Pickaxe sound 3:, Screen 4 - WRKDIG
	{
		FX_PICK3,			// sampleId
		FX_SPOT,			// type
		11,						// delay (or random chance) *
		{							// roomVolList
			{4,10,10},
			{0,0,0},		// NULL-TERMINATOR
		},
	},
	//------------------------
	// 31 Pickaxe sound 4:, Screen 4 - WRKDIG
	{
		FX_PICK4,			// sampleId
		FX_SPOT,			// type
		11,						// delay (or random chance) *
		{							// roomVolList
			{4,10,10},
			{0,0,0},		// NULL-TERMINATOR
		},
	},
	//------------------------
	// 32 Shorting light: FLICKER
	{
		FX_LIGHT,			// sampleId
		FX_SPOT,			// type
		1,						// delay (or random chance) *
		{							// roomVolList
			{3,15,15},	// {roomNo,leftVol,rightVol}
			{0,0,0},		// NULL-TERMINATOR
		},
	},
	//------------------------
	// 33 Cat leaps out of bin and runs: CATJMP!
	{
		FX_CAT,				// sampleId
		FX_SPOT,			// type
		20,						// delay (or random chance) *
		{							// roomVolList
			{2,10,10},	// {roomNo,leftVol,rightVol}
			{0,0,0},		// NULL-TERMINATOR
		},
	},
	//------------------------
	// 34 George rocks plastic crate: GEOCRT
	{
		FX_CRATE,			// sampleId
		FX_SPOT,			// type
		10,						// delay (or random chance) *
		{							// roomVolList
			{2,8,8},		// {roomNo,leftVol,rightVol}
			{0,0,0},		// NULL-TERMINATOR
		},
	},
	//------------------------
	// 35 George tries to climb drainpipe: GEOCLM02
	{
		FX_DRAIN,			// sampleId
		FX_SPOT,			// type
		9,						// delay (or random chance) *
		{							// roomVolList
			{2,10,10},	// {roomNo,leftVol,rightVol}
			{0,0,0},		// NULL-TERMINATOR
		},
	},
	//------------------------
	// 36 George removes manhole cover: GEOMAN8
	{
		FX_HOLE,			// sampleId
		FX_SPOT,			// type
		19,						// delay (or random chance) ?
		{							// roomVolList
			{2,12,11},	// {roomNo,leftVol,rightVol}
			{0,0,0},		// NULL-TERMINATOR
		},
	},
	//------------------------
	// 37 Brandy bottle put down: CHNDRN
	{
		FX_BOTDN,			// sampleId
		FX_SPOT,			// type
		43,						// delay (or random chance) *
		{							// roomVolList
			{3,8,8},		// {roomNo,leftVol,rightVol}
			{0,0,0},		// NULL-TERMINATOR
		},
	},
	//------------------------
	// 38 Brandy bottle picked up: GEOBOT3
	{
		FX_BOTUP,			// sampleId
		FX_SPOT,			// type
		9,						// delay (or random chance) *
		{							// roomVolList
			{3,10,10},	// {roomNo,leftVol,rightVol}
			{0,0,0},		// NULL-TERMINATOR
		},
	},
	//------------------------
	// 39 Chantelle gulps on brandy: CHNDRN
	{
		FX_GULP,			// sampleId
		FX_SPOT,			// type
		23,						// delay (or random chance) *
		{							// roomVolList
			{3,4,3},		// {roomNo,leftVol,rightVol}
			{0,0,0},		// NULL-TERMINATOR
		},
	},
	//------------------------
	// 40 Chantelle picked up off the floor: GEOCHN
	{
		FX_PIKUP,			// sampleId
		FX_SPOT,			// type
		28,						// delay (or random chance) *
		{							// roomVolList
			{3,11,10},	// {roomNo,leftVol,rightVol}
			{0,0,0},		// NULL-TERMINATOR
		},
	},
	//------------------------
	// 41 George searches Plantard's body: GEOCPS
	{
		FX_BODY,			// sampleId
		FX_SPOT,			// type
		10,						// delay (or random chance) *
		{							// roomVolList
			{3,10,10},	// {roomNo,leftVol,rightVol}
			{0,0,0},		// NULL-TERMINATOR
		},
	},
	//------------------------
	// 42 Moue cocks handgun. MOUENT
	{
		FX_PISTOL,		// sampleId
		FX_SPOT,			// type
		23,						// delay (or random chance) *
		{							// roomVolList
			{4,4,7},		// {roomNo,leftVol,rightVol}
			{0,0,0},		// NULL-TERMINATOR
		},
	},
	//------------------------
	// 43 George rummages in toolbox: GEOTBX
	{
		FX_TBOX,			// sampleId
		FX_SPOT,			// type
		12,						// delay (or random chance) *
		{							// roomVolList
			{4,12,10},	// {roomNo,leftVol,rightVol}
			{0,0,0},		// NULL-TERMINATOR
		},
	},
	//------------------------
	// 44 rat squeak 1
	{
		FX_RAT1,			// sampleId
		FX_RANDOM,		// type
		193,					// delay (or random chance)
		{							// roomVolList
			{6,5,7},		// {roomNo,leftVol,rightVol}
			{7,5,3},
			{0,0,0},		// NULL-TERMINATOR
		},
	},
	//------------------------
	// 45 rat squeak 2
	{
		FX_RAT2,			// sampleId
		FX_RANDOM,		// type
		201,					// delay (or random chance)
		{							// roomVolList
			{6,3,5},		// {roomNo,leftVol,rightVol}
			{7,4,6},
			{0,0,0},		// NULL-TERMINATOR
		},
	},
	//------------------------
	// 46 George climbs down ladder:
	{
		FX_LADD1,			// sampleId
		FX_SPOT,			// type
		10,						// delay (or random chance)
		{							// roomVolList
			{6,10,8},		// {roomNo,leftVol,rightVol}
			{0,0,0},		// NULL-TERMINATOR
		},
	},
	//------------------------
	// 47 Rushing water loop
	{
		FX_SWATER3,		// sampleId
		FX_LOOP,			// type
		0,						// delay (or random chance)
		{							// roomVolList
			{6,10,11},	// {roomNo,leftVol,rightVol}
			{7,12,11},
			{0,0,0},		// NULL-TERMINATOR
		},
	},
	//------------------------
	// 48 Left hand bin being opened: GEOCAT?
	{
		FX_BIN3,			// sampleId
		FX_SPOT,			// type
		12,						// delay (or random chance)
		{							// roomVolList
			{2,12,11},	// {roomNo,leftVol,rightVol}
			{0,0,0},		// NULL-TERMINATOR
		},
	},
	//------------------------
	// 49 Middle bin being opened: GEOBIN
	{
		FX_BIN2,			// sampleId
		FX_SPOT,			// type
		12,						// delay (or random chance)
		{							// roomVolList
			{2,11,11},	// {roomNo,leftVol,rightVol}
			{0,0,0},		// NULL-TERMINATOR
		},
	},
	//------------------------
	// 50 Right hand bin being opened: GEOLID?
	{
		FX_BIN1,			// sampleId
		FX_SPOT,			// type
		12,						// delay (or random chance)
		{							// roomVolList
			{2,10,11},	// {roomNo,leftVol,rightVol}
			{0,0,0},		// NULL-TERMINATOR
		},
	},
	//------------------------
	// 51 Passing car sound
	{
		FX_CARS,			// sampleId
		FX_RANDOM,		// type
		120,					// delay (or random chance)
		{							// roomVolList
			{10,8,1},
			{12,7,7},
			{0,0,0},		// NULL-TERMINATOR
		},
	},
	//------------------------
	// 52 Passing car sound
	{
		FX_FIESTA,		// sampleId
		FX_RANDOM,		// type
		127,					// delay (or random chance)
		{							// roomVolList
			{10,8,1},
			{12,7,7},
			{0,0,0},		// NULL-TERMINATOR
		},
	},
	//------------------------
	// 53 Passing car sound
	{
		FX_CARLTON ,	// sampleId
		FX_RANDOM,		// type
		119,					// delay (or random chance)
		{							// roomVolList
			{10,8,1},
			{0,0,0},		// NULL-TERMINATOR
		},
	},
	//------------------------
	// 54 Bird
	{
		FX_BIRD,			// sampleId
		FX_RANDOM,		// type
		500,					// delay (or random chance)
		{							// roomVolList
			{9,10,10},	// {roomNo,leftVol,rightVol}
			{10,2,1},
			{0,0,0},		// NULL-TERMINATOR
		},
	},
	//------------------------
	// 55 George tries the door: GEOTRY
	{
		FX_DOORTRY,		// sampleId
		FX_SPOT,			// type
		7,						// delay (or random chance)
		{							// roomVolList
			{9,9,9},		// {roomNo,leftVol,rightVol}
			{0,0,0},		// NULL-TERMINATOR
		},
	},
	//------------------------
	// 56 George opens the door: GEODOOR9
	{
		FX_FLATDOOR,	// sampleId
		FX_SPOT,			// type
		10,						// delay (or random chance)
		{							// roomVolList
			{9,9,9},		// {roomNo,leftVol,rightVol}
			{0,0,0},		// NULL-TERMINATOR
		},
	},
	//------------------------
	// 57 George picks the 'phone up: GEOPHN10
	{
		FX_FONEUP,		// sampleId
		FX_SPOT,			// type
		15,						// delay (or random chance)
		{							// roomVolList
			{10,9,9},		// {roomNo,leftVol,rightVol}
			{0,0,0},		// NULL-TERMINATOR
		},
	},
	//------------------------
	// 58 George puts the 'phone down: GEPDWN10
	{
		FX_FONEDN,		// sampleId
		FX_SPOT,			// type
		4,						// delay (or random chance)
		{							// roomVolList
			{10,9,9},		// {roomNo,leftVol,rightVol}
			{0,0,0},		// NULL-TERMINATOR
		},
	},
	//------------------------
	// 59 Albert opens the door: ALBOPEN
	{
		FX_ALBOP,			// sampleId
		FX_SPOT,			// type
		13,						// delay (or random chance)
		{							// roomVolList
			{5,10,10},	// {roomNo,leftVol,rightVol}
			{0,0,0},		// NULL-TERMINATOR
		},
	},
	//------------------------
	//------------------------
	// 60 Albert closes the door: ALBCLOSE
	{
		FX_ALBCLO,		// sampleId
		FX_SPOT,			// type
		20,						// delay (or random chance)
		{							// roomVolList
			{5,9,9},		// {roomNo,leftVol,rightVol}
			{0,0,0},		// NULL-TERMINATOR
		},
	},
	//------------------------
	// 61 George enter Nico's flat. GEOENT10
	{
		FX_NICOPEN,		// sampleId
		FX_SPOT,			// type
		1,						// delay (or random chance)
		{							// roomVolList
			{10,7,8},		// {roomNo,leftVol,rightVol}
			{0,0,0},		// NULL-TERMINATOR
		},
	},
	//------------------------
	// 62 George leaves Nico's. GEOLVS10
	{
		FX_NICLOSE,		// sampleId
		FX_SPOT,			// type
		13,						// delay (or random chance)
		{							// roomVolList
			{10,7,8},		// {roomNo,leftVol,rightVol}
			{0,0,0},		// NULL-TERMINATOR
		},
	},
	//------------------------
	// 63 Another bird for the street.
	{
		FX_BIRD2,			// sampleId
		FX_RANDOM,		// type
		500,					// WAS 15 (TOO LATE)
		{							// roomVolList
			{9,10,10},	// {roomNo,leftVol,rightVol}
			{10,2,1},
			{0,0,0},		// NULL-TERMINATOR
		},
	},
	//------------------------
	// 64 George sits in the chair: GEOCHR
	{
		FX_GEOCHAIR,	// sampleId
		FX_SPOT,			// type
		14,						// delay (or random chance)
		{							// roomVolList
			{10,5,5},		// {roomNo,leftVol,rightVol}
			{0,0,0},		// NULL-TERMINATOR
		},
	},
	//------------------------
	// 65 George sits on the couch: GEOCCH
	{
		FX_GEOCCH,		// sampleId
		FX_SPOT,			// type
		14,						// delay (or random chance)
		{							// roomVolList
			{10,5,5},		// {roomNo,leftVol,rightVol}
			{0,0,0},		// NULL-TERMINATOR
		},
	},
	//------------------------
	// 66 George gets up from the chair:  GEOCHR9
	{
		FX_GEOCHR9,		// sampleId
		FX_SPOT,			// type
		5,						// delay (or random chance)
		{							// roomVolList
			{10,3,3},		// {roomNo,leftVol,rightVol}
			{0,0,0},		// NULL-TERMINATOR
		},
	},
	//------------------------
	// 67 George is electrocuted: COSSHK
	{
		FX_SHOCK1,		// sampleId
		FX_SPOT,			// type
		19,						// delay (or random chance)
		{							// roomVolList
			{11,10,10},	// {roomNo,leftVol,rightVol}
			{0,0,0},		// NULL-TERMINATOR
		},
	},
	//------------------------
	// 68 George plays record: GEOWIND
	{
		FX_GRAMOFON,	// sampleId
		FX_SPOT,			// type
		0,						// delay (or random chance)
		{							// roomVolList
			{11,11,13},	// {roomNo,leftVol,rightVol}
			{0,0,0},		// NULL-TERMINATOR
		},
	},
	//------------------------
	// 69 George is frisked: GORFRK
	{
		FX_FRISK,			// sampleId
		FX_SPOT,			// type
		6,						// delay (or random chance)
		{							// roomVolList
			{12,8,8},		// {roomNo,leftVol,rightVol}
			{0,0,0},		// NULL-TERMINATOR
		},
	},
	//------------------------
	// 70 Traffic sound
	{
		FX_TRAFFIC3,	// sampleId
		FX_LOOP,			// type
		0,						// delay (or random chance)
		{							// roomVolList
			{11,5,4},
			{12,1,1},
			{16,4,4},
			{18,2,3},
			{46,4,3},
			{0,0,0},		// NULL-TERMINATOR
		}
	},
	//------------------------
	// 71 Latvian reading: LATRDS
	{
		FX_PAPER6,		// sampleId
		FX_SPOT,			// type
		8,						// delay (or random chance)
		{							// roomVolList
			{13,8,8},		// {roomNo,leftVol,rightVol}
			{0,0,0},		// NULL-TERMINATOR
		},
	},
	//------------------------
	// 72 Deskbell
	{
		FX_DESKBELL,	// sampleId
		FX_SPOT,			// type
		0,						// delay (or random chance)
		{							// roomVolList
			{13,10,8},	// {roomNo,leftVol,rightVol}
			{0,0,0},		// NULL-TERMINATOR
		},
	},
	//------------------------
	// 73 George picks up hotel 'phone: GEOTEL
	{
		FX_PHONEUP2,	// sampleId
		FX_SPOT,			// type
		10,						// delay (or random chance)
		{							// roomVolList
			{13,8,8},		// {roomNo,leftVol,rightVol}
			{0,0,0},		// NULL-TERMINATOR
		},
	},
	//------------------------
	// 74 George puts down hotel 'phone: GEOTEL9
	{
		FX_PHONEDN2,	// sampleId
		FX_SPOT,			// type
		10,						// delay (or random chance)
		{							// roomVolList
			{13,8,8},		// {roomNo,leftVol,rightVol}
			{0,0,0},		// NULL-TERMINATOR
		},
	},
	//------------------------
	// 75 George tries doors in corridor: GEODOR
	{
		FX_TRYDOR14,	// sampleId
		FX_SPOT,			// type
		10,						// delay (or random chance)
		{							// roomVolList
			{14,8,8},		// {roomNo,leftVol,rightVol}
			{0,0,0},		// NULL-TERMINATOR
		},
	},
	//------------------------
	// 76 George opens bedside cabinet: BEDDOR
	{
		FX_CABOPEN,		// sampleId
		FX_SPOT,			// type
		11,						// delay (or random chance)
		{							// roomVolList
			{15,10,14},	// {roomNo,leftVol,rightVol}
			{17,10,14},
			{0,0,0},		// NULL-TERMINATOR
		},
	},
	//------------------------
	// 77 George closes bedside cabinet: BEDDOR (reversed)
	{
		FX_CABCLOSE,	// sampleId
		FX_SPOT,			// type
		5,						// delay (or random chance)
		{							// roomVolList
			{15,10,14},	// {roomNo,leftVol,rightVol}
			{17,10,14},
			{0,0,0},		// NULL-TERMINATOR
		},
	},
	//------------------------
	// 78 George opens the window: WINDOW
	{
		FX_WINDOPEN,	// sampleId
		FX_SPOT,			// type
		19,						// delay (or random chance)
		{							// roomVolList
			{15,8,8},		// {roomNo,leftVol,rightVol}
			{0,0,0},		// NULL-TERMINATOR
		},
	},
	//------------------------
	// 79 George goes right along the ledge: GEOIRW
	{
		FX_LEDGE1,		// sampleId
		FX_SPOT,			// type
		1,						// delay (or random chance)
		{							// roomVolList
			{16,12,12},	// {roomNo,leftVol,rightVol}
			{0,0,0},		// NULL-TERMINATOR
		},
	},
	//------------------------
	//------------------------
	// 80 George goes left along the ledge: GEOILW
	{
		FX_LEDGE2,		// sampleId
		FX_SPOT,			// type
		1,						// delay (or random chance)
		{							// roomVolList
			{16,12,12},	// {roomNo,leftVol,rightVol}
			{0,0,0},		// NULL-TERMINATOR
		},
	},
	//------------------------
	// 81 Pigeon noises
	{
		FX_COO,				// sampleId
		FX_RANDOM,		// type
		80,						// delay (or random chance)
		{							// roomVolList
			{16,7,9},		// {roomNo,leftVol,rightVol}
			{46,5,4},
			{0,0,0},		// NULL-TERMINATOR
		},
	},
	//------------------------
	// 82 Pigeon noises
	{
		FX_COO2,			// sampleId
		FX_RANDOM,		// type
		60,						// delay (or random chance)
		{							// roomVolList
			{15,3,4},		// {roomNo,leftVol,rightVol}
			{16,8,5},		// {roomNo,leftVol,rightVol}
			{17,3,4},
			{0,0,0},		// NULL-TERMINATOR
		},
	},
	//------------------------
	// 83 George picks up and opens case: GEOBFC
	{
		FX_BRIEFON,		// sampleId
		FX_SPOT,			// type
		16,						// delay (or random chance)
		{							// roomVolList
			{17,12,12},	// {roomNo,leftVol,rightVol}
			{0,0,0},		// NULL-TERMINATOR
		},
	},
	//------------------------
	// 84 George closes and puts down case: GEOBFC (reversed)
	{
		FX_BRIEFOFF,	// sampleId
		FX_SPOT,			// type
		12,						// delay (or random chance)
		{							// roomVolList
			{17,12,12},	// {roomNo,leftVol,rightVol}
			{0,0,0},		// NULL-TERMINATOR
		},
	},
	//------------------------
	// 85 George gets into wardrobe. GEOWRB2 Attention, James. This is new as of 15/7/96
	{
		FX_WARDIN,		// sampleId
		FX_SPOT,			// type
		6,						// delay (or random chance)
		{							// roomVolList
			{17,4,4},		// {roomNo,leftVol,rightVol}
			{0,0,0},		// NULL-TERMINATOR
		},
	},
	//------------------------
	// 86 George gets out of wardrobe. GEOWRB2  (Reversed). Attention, James. This is new as of 15/7/96
	{
		FX_WARDOUT,		// sampleId
		FX_SPOT,			// type
		41,						// delay (or random chance)
		{							// roomVolList
			{17,4,4},		// {roomNo,leftVol,rightVol}
			{0,0,0},		// NULL-TERMINATOR
		},
	},
	//------------------------
	// 87 George jumps in through window: GEOWIN2
	{
		FX_JUMPIN,		// sampleId
		FX_SPOT,			// type
		7,						// delay (or random chance)
		{							// roomVolList
			{15,8,10},
			{0,0,0},		// NULL-TERMINATOR
		},
	},
	//------------------------
	// 88 George climbs in: GEOWIN2/GEOWIN8
	{
		FX_CLIMBIN,		// sampleId
		FX_SPOT,			// type
		0,						// delay (or random chance)
		{							// roomVolList
			{17,8,16},	// {roomNo,leftVol,rightVol}
			{15,8,16},
			{0,0,0},		// NULL-TERMINATOR
		},
	},
	//------------------------
	// 89 George climbs out: GEOWIN1/GEOWIN9
	{
		FX_CLIMBOUT,	// sampleId
		FX_SPOT,			// type
		17,						// delay (or random chance)
		{							// roomVolList
			{17,9,10},	// {roomNo,leftVol,rightVol}
			{15,9,10},
			{0,0,0},		// NULL-TERMINATOR
		},
	},
	//------------------------
	// 90 George picks the 'phone up: GEOTEL18
	{
		FX_FONEUP,		// sampleId
		FX_SPOT,			// type
		10,						// delay (or random chance)
		{							// roomVolList
			{18,4,3},		// {roomNo,leftVol,rightVol}
			{0,0,0},		// NULL-TERMINATOR
		},
	},
	//------------------------
	// 91 George puts the 'phone down: GEOTL18A
	{
		FX_FONEDN,		// sampleId
		FX_SPOT,			// type
		11,						// delay (or random chance)
		{							// roomVolList
			{18,4,3},		// {roomNo,leftVol,rightVol}
			{0,0,0},		// NULL-TERMINATOR
		},
	},
	//------------------------
	// 92 George tries to get keys. GEOKEY
	{
		FX_KEY13,			// sampleId
		FX_SPOT,			// type
		8,						// delay (or random chance)
		{							// roomVolList
			{13,3,2},		// {roomNo,leftVol,rightVol}
			{0,0,0},		// NULL-TERMINATOR
		},
	},
	//------------------------
	// 93 George manages to get keys. GEOKEY13
	{
		FX_KEY13,			// sampleId
		FX_SPOT,			// type
		9,						// delay (or random chance)
		{							// roomVolList
			{13,3,2},		// {roomNo,leftVol,rightVol}
			{0,0,0},		// NULL-TERMINATOR
		},
	},
	//------------------------
	// 94 George electrocutes Maguire: MAGSHK
	{
		FX_SHOCK2,		// sampleId
		FX_SPOT,			// type
		8,						// delay (or random chance)
		{							// roomVolList
			{19,9,10},	// {roomNo,leftVol,rightVol}
			{0,0,0},		// NULL-TERMINATOR
		},
	},
	//------------------------
	// 95 George opens dray door : GEOTRP8
	{
		FX_TRAPOPEN,	// sampleId
		FX_SPOT,			// type
		20,						// delay (or random chance)
		{							// roomVolList
			{19,10,10},	// {roomNo,leftVol,rightVol}
			{0,0,0},		// NULL-TERMINATOR
		},
	},
	//------------------------
	// 96 George breaks switch : Which anim?
	{
		FX_SWITCH19,	// sampleId
		FX_SPOT,			// type
		10,						// delay (or random chance)
		{							// roomVolList
			{19,10,10},	// {roomNo,leftVol,rightVol}
			{0,0,0},		// NULL-TERMINATOR
		},
	},
	//------------------------
	// 97 Leary pulls pint: LESPMP
	{
		FX_PULLPINT,	// sampleId
		FX_SPOT,			// type
		6,						// delay (or random chance)
		{							// roomVolList
			{20,10,8},	// {roomNo,leftVol,rightVol}
			{0,0,0},		// NULL-TERMINATOR
		},
	},
	//------------------------
	// 98 Glasswasher fuse blows (and the glass washer grinds to a halt)
	{
		FX_FUSE20,		// sampleId
		FX_SPOT,			// type
		0,						// delay (or random chance)
		{							// roomVolList
			{20,8,8},		// {roomNo,leftVol,rightVol}
			{0,0,0},		// NULL-TERMINATOR
		},
	},
	//------------------------
	// 99 Fitz leaps to his feet: FTZSTD
	{
		FX_FITZUP,		// sampleId
		FX_SPOT,			// type
		5,						// delay (or random chance)
		{							// roomVolList
			{20,8,8},		// {roomNo,leftVol,rightVol}
			{0,0,0},		// NULL-TERMINATOR
		},
	},
	//------------------------
	//------------------------
	// 100 Fitz runs for it: FTZRUN
	{
		FX_FITZRUN,		// sampleId
		FX_SPOT,			// type
		15,						// delay (or random chance)
		{							// roomVolList
			{20,12,10},	// {roomNo,leftVol,rightVol}
			{0,0,0},		// NULL-TERMINATOR
		},
	},
	//------------------------
	// 101 George pulls lever: GEOLVR & GEOLVR26
	{
		FX_LEVER,			// sampleId
		FX_SPOT,			// type
		26,						// delay (or random chance)
		{							// roomVolList
			{21,8,10},	// {roomNo,leftVol,rightVol}
			{26,8,10},
			{0,0,0},		// NULL-TERMINATOR
		},
	},
	//------------------------
	// 102 George pulls lever: GEOLVR8 & GEOLVR08
	{
		FX_LEVER2,		// sampleId
		FX_SPOT,			// type
		9,						// delay (or random chance)
		{							// roomVolList
			{21,8,10},	// {roomNo,leftVol,rightVol}
			{26,8,10},
			{0,0,0},		// NULL-TERMINATOR
		},
	},
	//------------------------
	// 103 George opens tap: No idea what the anim is
	{
		FX_TAP,				// sampleId
		FX_SPOT,			// type
		10,						// delay (or random chance)
		{							// roomVolList
			{21,8,8},		// {roomNo,leftVol,rightVol}
			{0,0,0},		// NULL-TERMINATOR
		},
	},
	//------------------------
	// 104 George closes tap: No idea what this anim is either
	{
		FX_TAP2,			// sampleId
		FX_SPOT,			// type
		10,						// delay (or random chance)
		{							// roomVolList
			{21,8,8},		// {roomNo,leftVol,rightVol}
			{0,0,0},		// NULL-TERMINATOR
		},
	},
	//------------------------
	// 105 Bar flap: FLPOPN
	{
		FX_BARFLAP,		// sampleId
		FX_SPOT,			// type
		1,						// delay (or random chance)
		{							// roomVolList
			{20,6,6},		// {roomNo,leftVol,rightVol}
			{0,0,0},		// NULL-TERMINATOR
		},
	},
	//------------------------
	// 106 Farmer leaves: FRMWLK
	{
		FX_FARMERGO,	// sampleId
		FX_SPOT,			// type
		11,						// delay (or random chance)
		{							// roomVolList
			{22,6,9},		// {roomNo,leftVol,rightVol}
			{0,0,0},		// NULL-TERMINATOR
		},
	},
	//------------------------
	// 107 George climbs haystack: GEOCLM
	{
		FX_CLIMBHAY,	// sampleId
		FX_SPOT,			// type
		11,						// delay (or random chance)
		{							// roomVolList
			{22,14,14},	// {roomNo,leftVol,rightVol}
			{0,0,0},		// NULL-TERMINATOR
		},
	},
	//------------------------
	// 108 George drives sewer key into wall: GEOKEY23
	{
		FX_KEYSTEP,		// sampleId
		FX_SPOT,			// type
		39,						// delay (or random chance)
		{							// roomVolList
			{23,8,10},	// {roomNo,leftVol,rightVol}
			{0,0,0},		// NULL-TERMINATOR
		},
	},
	//------------------------
	// 109 George climbs over wall: GEOCLM23
	{
		FX_CASTLWAL,	// sampleId
		FX_SPOT,			// type
		17,						// delay (or random chance)
		{							// roomVolList
			{23,8,8},		// {roomNo,leftVol,rightVol}
			{0,0,0},		// NULL-TERMINATOR
		},
	},
	//------------------------
	//------------------------
	// 110 George falls from wall: GEOTRY23
	{
		FX_CLIMBFAL,	// sampleId
		FX_SPOT,			// type
		43,						// delay (or random chance)
		{							// roomVolList
			{23,12,12},	// {roomNo,leftVol,rightVol}
			{0,0,0},		// NULL-TERMINATOR
		},
	},
	//------------------------
	// 111 Goat chewing: GOTEAT
	{
		FX_GOATCHEW,	// sampleId
		FX_SPOT,			// type
		1,						// delay (or random chance)
		{							// roomVolList
			{24,10,10},	// {roomNo,leftVol,rightVol}
			{0,0,0},		// NULL-TERMINATOR
		},
	},
	//------------------------
	// 112 George moves plough: GEOPLW
	{
		FX_PLOUGH,		// sampleId
		FX_SPOT,			// type
		8,						// delay (or random chance)
		{							// roomVolList
			{24,10,10},	// {roomNo,leftVol,rightVol}
			{0,0,0},		// NULL-TERMINATOR
		},
	},
	//------------------------
	// 113 George drops slab: STNFALL
	{
		FX_SLABFALL,	// sampleId
		FX_SPOT,			// type
		6,						// delay (or random chance)
		{							// roomVolList
			{25,10,10},	// {roomNo,leftVol,rightVol}
			{0,0,0},		// NULL-TERMINATOR
		},
	},
	//------------------------
	// 114 George picks up slab: GEOSTN8
	{
		FX_SLABUP,		// sampleId
		FX_SPOT,			// type
		29,						// delay (or random chance)
		{							// roomVolList
			{25,10,10},	// {roomNo,leftVol,rightVol}
			{0,0,0},		// NULL-TERMINATOR
		},
	},
	//------------------------
	// 115 Secret door opens: ALTOPN
	{
		FX_SECDOR25,	// sampleId
		FX_SPOT,			// type
		17,						// delay (or random chance)
		{							// roomVolList
			{25,10,10},	// {roomNo,leftVol,rightVol}
			{0,0,0},		// NULL-TERMINATOR
		},
	},
	//------------------------
	// 116 George wrings out cloth: GEOTWL25
	{
		FX_WRING,			// sampleId
		FX_SPOT,			// type
		24,						// delay (or random chance)
		{							// roomVolList
			{25,10,10},	// {roomNo,leftVol,rightVol}
			{0,0,0},		// NULL-TERMINATOR
		},
	},
	//------------------------
	// 117 Rat running across barrels: RATJMP
	{
		FX_RAT3A,			// sampleId
		FX_SPOT,			// type
		1,						// delay (or random chance)
		{							// roomVolList
			{26,8,5},		// {roomNo,leftVol,rightVol}
			{0,0,0},		// NULL-TERMINATOR
		},
	},
	//------------------------
	// 118 Rat running across barrels: RATJMP
	{
		FX_RAT3B,			// sampleId
		FX_SPOT,			// type
		8,						// delay (or random chance)
		{							// roomVolList
			{26,7,6},		// {roomNo,leftVol,rightVol}
			{0,0,0},		// NULL-TERMINATOR
		},
	},
	//------------------------
	// 119 Rat running across barrels: RATJMP
	{
		FX_RAT3C,			// sampleId
		FX_SPOT,			// type
		26,						// delay (or random chance)
		{							// roomVolList
			{26,8,8},		// {roomNo,leftVol,rightVol}
			{0,0,0},		// NULL-TERMINATOR
		},
	},
	//------------------------
	//------------------------
	// 120 Irish bird song 1:
	{
		FX_EIRBIRD1,	// sampleId
		FX_RANDOM,		// type
		720,					// delay (or random chance)
		{							// roomVolList
			{19,6,8},		// {roomNo,leftVol,rightVol}
			{21,2,3},
			{22,8,5},
			{23,6,5},
			{24,8,8},
			{0,0,0},		// NULL-TERMINATOR
		}
	},
	//------------------------
	// 121 Irish bird song 2:
	{
		FX_EIRBIRD2,	// sampleId
		FX_RANDOM,		// type
		720,					// delay (or random chance)
		{							// roomVolList
			{19,8,6},		// {roomNo,leftVol,rightVol}
			{21,2,3},
			{22,6,8},
			{23,5,5},
			{24,8,8},
			{0,0,0},		// NULL-TERMINATOR
		}
	},
	//------------------------
	// 122 Irish bird song 3:
	{
		FX_EIRBIRD3,	// sampleId
		FX_RANDOM,		// type
		720,					// delay (or random chance)
		{							// roomVolList
			{19,8,8},		// {roomNo,leftVol,rightVol}
			{21,3,4},
			{22,8,8},
			{23,5,6},
			{24,6,8},
			{0,0,0},		// NULL-TERMINATOR
		}
	},
	//------------------------
	// 123 Rat 3D:
	{
		FX_RAT3D,			// sampleId
		FX_RANDOM,		// type
		600,					// delay (or random chance)
		{							// roomVolList
			{26,2,3},		// {roomNo,leftVol,rightVol}
			{0,0,0},		// NULL-TERMINATOR
		}
	},
	//------------------------
	// 124 Wind atop the battlements
	{
		FX_WIND,			// sampleId
		FX_LOOP,			// type
		0,						// delay (or random chance)
		{							// roomVolList
			{23,6,6},		// {roomNo,leftVol,rightVol}
			{0,0,0},		// NULL-TERMINATOR
		}
	},
	//------------------------
	// 125 Glasswasher in the pub (Room 20) *JEL* Stops after fuse blows and starts when george fixes it.
	{
		FX_WASHER,		// sampleId
		FX_LOOP,			// type
		0,						// delay (or random chance)
		{							// roomVolList
			{20,4,4},		// {roomNo,leftVol,rightVol}
			{0,0,0},		// NULL-TERMINATOR
		}
	},
	//------------------------
	// 126 Running tap in the cellar: (Room 21) *JEL* Only when the tap is on.
	{
		FX_CELTAP,		// sampleId
		FX_LOOP,			// type
		0,						// delay (or random chance)
		{							// roomVolList
			{21,3,3},		// {roomNo,leftVol,rightVol}
			{0,0,0},		// NULL-TERMINATOR
		}
	},
	//------------------------
	// 127 Lopez's hose. Basically a loop but stops when George cuts the water supply. Replaces MUTTER1.
	{
		FX_HOSE57,		// sampleId
		FX_LOOP,			// type
		0,						// delay (or random chance)
		{							// roomVolList
			{57,3,1},		// {roomNo,leftVol,rightVol}
			{0,0,0},		// NULL-TERMINATOR
		}
	},
	//------------------------
	// 128 Lopez's hose being switched off. Anim GARD05. Replaces MUTTER2.
	{
		FX_HOSE57B,		// sampleId
		FX_SPOT,			// type
		1,						// delay (or random chance)
		{							// roomVolList
			{57,3,2},		// {roomNo,leftVol,rightVol}
			{0,0,0},		// NULL-TERMINATOR
		}
	},
	//------------------------
	// 129 Nejo bouncing the ball off the door. NEJ8
	{
		FX_BALLPLAY,	// sampleId
		FX_SPOT,			// type
		13,						// delay (or random chance)
		{							// roomVolList
			{45,5,1},		// {roomNo,leftVol,rightVol}
			{0,0,0},		// NULL-TERMINATOR
		}
	},
	//------------------------
	//------------------------
	// 130 Cricket loop for Syrian desert Only audible in 55 when the cave door is open.
	{
		FX_CRICKET,		// sampleId
		FX_LOOP,			// type
		0,						// delay (or random chance)
		{							// roomVolList
			{54,8,8},		// {roomNo,leftVol,rightVol}
			{55,3,5},
			{0,0,0},		// NULL-TERMINATOR
		}
	},
	//------------------------
	// 131 Display case shatters: GEOTOTB
	{
		FX_SMASHGLA,	// sampleId
		FX_SPOT,			// type
		35,						// delay (or random chance)
		{							// roomVolList
			{29,16,12},	// {roomNo,leftVol,rightVol}
			{0,0,0},		// NULL-TERMINATOR
		}
	},
	//------------------------
	// 132 Burglar alarm: Once the case is smashed (see 131)
	{
		FX_ALARM,			// sampleId
		FX_LOOP,			// type
		0,						// delay (or random chance)
		{							// roomVolList
			{28,12,12},	// {roomNo,leftVol,rightVol}
			{29,16,16},	// {roomNo,leftVol,rightVol}
			{0,0,0},		// NULL-TERMINATOR
		}
	},
	//------------------------
	// 133 Guido fires: GUIGUN
	{
		FX_GUN1,			// sampleId
		FX_SPOT,			// type
		7,						// delay (or random chance)
		{							// roomVolList
			{29,16,16},	// {roomNo,leftVol,rightVol}
			{0,0,0},		// NULL-TERMINATOR
		}
	},
	//------------------------
	// 134 Guido knocked down: NICPUS1
	{
		FX_GUI_HIT,		// sampleId
		FX_SPOT,			// type
		40,						// delay (or random chance)
		{							// roomVolList
			{29,10,10},	// {roomNo,leftVol,rightVol}
			{0,0,0},		// NULL-TERMINATOR
		}
	},
	//------------------------
	// 135 Museum exterior ambience
	{
		FX_MUESEXT,		// sampleId
		FX_LOOP,			// type
		0,						// delay (or random chance)
		{							// roomVolList
			{27,10,10},	// {roomNo,leftVol,rightVol}
			{0,0,0},		// NULL-TERMINATOR
		}
	},
	//------------------------
	// 136 Cat gets nowty: CAT3
	{
		FX_STALLCAT,	// sampleId
		FX_SPOT,			// type
		1,						// delay (or random chance)
		{							// roomVolList
			{45,10,6},	// {roomNo,leftVol,rightVol}
			{0,0,0},		// NULL-TERMINATOR
		}
	},
	//------------------------
	// 137 Cat gets very nowty: CAT5
	{
		FX_CATHIT,		// sampleId
		FX_SPOT,			// type
		4,						// delay (or random chance)
		{							// roomVolList
			{45,10,6},	// {roomNo,leftVol,rightVol}
			{0,0,0},		// NULL-TERMINATOR
		}
	},
	//------------------------
	// 138 Desert wind: Only audible in 55 when the cave door is open.
	{
		FX_SYRIWIND,	// sampleId
		FX_RANDOM,		// type
		720,					// delay (or random chance)
		{							// roomVolList
			{54,10,10},	// {roomNo,leftVol,rightVol}
			{55,5,7},
			{0,0,0},		// NULL-TERMINATOR
		}
	},
	//------------------------
	// 139 Bell on Nejo's stall: GEOSYR7
	{
		FX_STALLBEL,	// sampleId
		FX_SPOT,			// type
		7,						// delay (or random chance)
		{							// roomVolList
			{45,10,8},	// {roomNo,leftVol,rightVol}
			{0,0,0},		// NULL-TERMINATOR
		}
	},
	//------------------------
	//------------------------
	// 140 George electrocutes Khan: GEOSYR40
	{
		FX_SHOCK3,		// sampleId
		FX_SPOT,			// type
		6,						// delay (or random chance)
		{							// roomVolList
			{54,10,10},	// {roomNo,leftVol,rightVol}
			{0,0,0},		// NULL-TERMINATOR
		},
	},
	//------------------------
	// 141 George thumps Khan: GEOSYR40
	{
		FX_THUMP1,		// sampleId
		FX_SPOT,			// type
		22,						// delay (or random chance)
		{							// roomVolList
			{54,12,12},	// {roomNo,leftVol,rightVol}
			{0,0,0},		// NULL-TERMINATOR
		},
	},
	//------------------------
	// 142 Khan hits the floor: KHS9
	{
		FX_KHANDOWN,	// sampleId
		FX_SPOT,			// type
		24,						// delay (or random chance)
		{							// roomVolList
			{54,8,8},		// {roomNo,leftVol,rightVol}
			{0,0,0},		// NULL-TERMINATOR
		},
	},
	//------------------------
	// 143 Hospital ambience
	{
		FX_HOSPNOIS,	// sampleId
		FX_LOOP,			// type
		0,						// delay (or random chance)
		{							// roomVolList
			{32,6,4},		// {roomNo,leftVol,rightVol}
			{33,7,7},
			{34,3,4},
			{0,0,0},		// NULL-TERMINATOR
		},
	},
	//------------------------
	// 144 Mr Shiny switched on: DOMPLG (Start FX_SHINY)
	{
		FX_SHINYON,		// sampleId
		FX_SPOT,			// type
		0,						// delay (or random chance)
		{							// roomVolList
			{33,12,14},	// {roomNo,leftVol,rightVol}
			{0,0,0},		// NULL-TERMINATOR
		},
	},
	//------------------------
	// 145 Mr Shiny running
	{
		FX_SHINY,			// sampleId
		FX_LOOP,			// type
		0,						// delay (or random chance)
		{							// roomVolList
			{32,4,3},		// {roomNo,leftVol,rightVol}
			{33,12,14},
			{0,0,0},		// NULL-TERMINATOR
		},
	},
	//------------------------
	// 146 Mr Shiny switched off: GEOPLG33 (Turn off FX_SHINY at the same time)
	{
		FX_SHINYOFF,	// sampleId
		FX_SPOT,			// type
		0,						// delay (or random chance)
		{							// roomVolList
			{33,12,16},	// {roomNo,leftVol,rightVol}
			{0,0,0},		// NULL-TERMINATOR
		},
	},
	//------------------------
	// 147 Benoir takes blood pressure: BENBP1 or BENBP2
	{
		FX_BLOODPRE,	// sampleId
		FX_SPOT,			// type
		39,						// delay (or random chance)
		{							// roomVolList
			{34,14,16},	// {roomNo,leftVol,rightVol}
			{0,0,0},		// NULL-TERMINATOR
		},
	},
	//------------------------
	// 148 George takes blood pressure: GEOBP1 or GEOBP2
	{
		FX_BLOODPRE,	// sampleId
		FX_SPOT,			// type
		62,						// delay (or random chance)
		{							// roomVolList
			{34,14,16},	// {roomNo,leftVol,rightVol}
			{0,0,0},		// NULL-TERMINATOR
		},
	},
	//------------------------
	// 149 Goat baas as it attacks: GOTCR and GOTCL
	{
		FX_GOATBAA,		// sampleId
		FX_SPOT,			// type
		1,						// delay (or random chance)
		{							// roomVolList
			{24,12,12},		// {roomNo,leftVol,rightVol}
			{0,0,0},		// NULL-TERMINATOR
		},
	},
	//------------------------
	//------------------------
	// 150 Goat peeved at being trapped: GOTPLW (I'd advise triggering this anim randomly if you haven't done that)
	{
		FX_GOATDOH,		// sampleId
		FX_SPOT,			// type
		7,						// delay (or random chance)
		{							// roomVolList
			{24,7,6},		// {roomNo,leftVol,rightVol}
			{0,0,0},		// NULL-TERMINATOR
		},
	},
	//------------------------
	// 151 George triggers the Irish secret door: GEOPUT
	{
		FX_TRIGER25,	// sampleId
		FX_SPOT,			// type
		35,						// delay (or random chance)
		{							// roomVolList
			{25,6,5},		// {roomNo,leftVol,rightVol}
			{0,0,0},		// NULL-TERMINATOR
		},
	},
	//------------------------
	// 152 George winds up gramophone: GEOWIND
	{
		FX_WINDUP11,	// sampleId
		FX_SPOT,			// type
		16,						// delay (or random chance)
		{							// roomVolList
			{11,7,7},		// {roomNo,leftVol,rightVol}
			{0,0,0},		// NULL-TERMINATOR
		},
	},
	//------------------------
	// 153 Marib ambience
	{
		FX_MARIB,			// sampleId
		FX_LOOP,			// type
		0,						// delay (or random chance)
		{							// roomVolList
			{45,7,7},		// {roomNo,leftVol,rightVol}
			{47,5,5},
			{50,5,4},
			{0,0,0},		// NULL-TERMINATOR
		},
	},
	//------------------------
	// 154 Statuette breaks: STA2
	{
		FX_STATBREK,	// sampleId
		FX_SPOT,			// type
		10,						// delay (or random chance)
		{							// roomVolList
			{45,7,4},		// {roomNo,leftVol,rightVol}
			{0,0,0},		// NULL-TERMINATOR
		},
	},
	//------------------------
	// 155 George opens toilet door: CUBDOR50
	{
		FX_CUBDOR,		// sampleId
		FX_SPOT,			// type
		6,						// delay (or random chance)
		{							// roomVolList
			{50,6,6},		// {roomNo,leftVol,rightVol}
			{0,0,0},		// NULL-TERMINATOR
		},
	},
	//------------------------
	// 156 Crowd goes, "Ooh!": CRO36APP
	{
		FX_OOH,				// sampleId
		FX_SPOT,			// type
		1,						// delay (or random chance)
		{							// roomVolList
			{36,6,7},		// {roomNo,leftVol,rightVol}
			{0,0,0},		// NULL-TERMINATOR
		},
	},
	//------------------------
	// 157 Phone rings: When Nico calls back in room 41. Loops until the guard answers it.
	{
		FX_PHONCALL,	// sampleId
		FX_LOOP,			// type
		0,						// delay (or random chance)
		{							// roomVolList
			{41,16,16},	// {roomNo,leftVol,rightVol}
			{0,0,0},		// NULL-TERMINATOR
		},
	},
	//------------------------
	// 158 Phone picked up in 41: GUA41ANS
	{
		FX_FONEUP41,	// sampleId
		FX_SPOT,			// type
		18,						// delay (or random chance)
		{							// roomVolList
			{41,5,6},		// {roomNo,leftVol,rightVol}
			{0,0,0},		// NULL-TERMINATOR
		},
	},
	//------------------------
	// 159 George turns thermostat: GEO41THE (another dummy). Also used on the reverse.
	{
		FX_THERMO1,		// sampleId
		FX_SPOT,			// type
		10,						// delay (or random chance)
		{							// roomVolList
			{41,6,5},		// {roomNo,leftVol,rightVol}
			{0,0,0},		// NULL-TERMINATOR
		},
	},
	//------------------------
	// 160 Low echoing rumble of large church
	{
		FX_CHURCHFX,	// sampleId
		FX_LOOP,			// type
		0,						// delay (or random chance)
		{							// roomVolList
			{38,5,5},		// {roomNo,leftVol,rightVol}
			{48,5,5},		// {roomNo,leftVol,rightVol}
			{0,0,0},		// NULL-TERMINATOR
		},
	},
	//------------------------
	// 161 George drys hand: GEO43HAN
	{
		FX_DRIER1,		// sampleId
		FX_SPOT,			// type
		9,						// delay (or random chance)
		{							// roomVolList
			{43,16,16},	// {roomNo,leftVol,rightVol}
			{0,0,0},		// NULL-TERMINATOR
		},
	},
	//------------------------
	// 162 George jumps in through window: GEOWIN8
	{
		FX_JUMPIN,		// sampleId
		FX_SPOT,			// type
		49,						// delay (or random chance)
		{							// roomVolList
			{17,8,10},	// {roomNo,leftVol,rightVol}
			{0,0,0},		// NULL-TERMINATOR
		},
	},
	//------------------------
	// 163 Khan fires: KHS12
	{
		FX_SHOTKHAN,	// sampleId
		FX_SPOT,			// type
		30,						// delay (or random chance)
		{							// roomVolList
			{54,5,5},		// {roomNo,leftVol,rightVol}
			{0,0,0},		// NULL-TERMINATOR
		},
	},
	//------------------------
	// 164 Khan fires: KHS5
	{
		FX_SHOTKHAN,	// sampleId
		FX_SPOT,			// type
		5,						// delay (or random chance)
		{							// roomVolList
			{54,5,5},		// {roomNo,leftVol,rightVol}
			{0,0,0},		// NULL-TERMINATOR
		},
	},
	//------------------------
	// 165 George falls: GEOSYR37
	{
		FX_GEOFAL54,	// sampleId
		FX_SPOT,			// type
		25,						// delay (or random chance)
		{							// roomVolList
			{54,5,5},		// {roomNo,leftVol,rightVol}
			{0,0,0},		// NULL-TERMINATOR
		},
	},
	//------------------------
	// 166 George falls after going for the gun (GEOSYR42)
	{
		FX_GEOFAL54,	// sampleId
		FX_SPOT,			// type
		46,						// delay (or random chance)
		{							// roomVolList
			{54,5,5},		// {roomNo,leftVol,rightVol}
			{0,0,0},		// NULL-TERMINATOR
		},
	},
	//------------------------
	// 167 Pickaxe sound 5: Screen 1 - WRKDIG01
	{
		FX_PICK5,			// sampleId
		FX_SPOT,			// type
		9,						// delay (or random chance) *
		{							// roomVolList
			{1,3,3},		// {roomNo,leftVol,rightVol}
			{0,0,0},		// NULL-TERMINATOR
		},
	},
	//------------------------
	// 168 George climbs ladder in 7: GEOASC07
	{
		FX_SEWLADU7,	// sampleId
		FX_SPOT,			// type
		7,						// delay (or random chance) *
		{							// roomVolList
			{7,8,9},		// {roomNo,leftVol,rightVol}
			{0,0,0},		// NULL-TERMINATOR
		},
	},
	//------------------------
	// 169 George picks keys up in Alamut: GEOKEYS1
	{
		FX_KEYS49,		// sampleId
		FX_SPOT,			// type
		9,						// delay (or random chance) *
		{							// roomVolList
			{49,8,7},		// {roomNo,leftVol,rightVol}
			{0,0,0},		// NULL-TERMINATOR
		},
	},
	//------------------------
	//------------------------
	// 170 George puts down keys up in Alamut: GEOKEYS2
	{
		FX_KEYS49,		// sampleId
		FX_SPOT,			// type
		7,						// delay (or random chance) *
		{							// roomVolList
			{49,8,7},		// {roomNo,leftVol,rightVol}
			{0,0,0},		// NULL-TERMINATOR
		},
	},
	//------------------------
	// 171 George unlocks toilet door: GEOSYR43
	{
		FX_UNLOCK49,	// sampleId
		FX_SPOT,			// type
		16,						// delay (or random chance) *
		{							// roomVolList
			{49,6,8},		// {roomNo,leftVol,rightVol}
			{0,0,0},		// NULL-TERMINATOR
		},
	},
	//------------------------
	// 172 George breaks the toilet chain. GEOSYR48
	{
		FX_WCCHAIN,		// sampleId
		FX_SPOT,			// type
		10,						// delay (or random chance) *
		{							// roomVolList
			{50,6,7},		// {roomNo,leftVol,rightVol}
			{0,0,0},		// NULL-TERMINATOR
		},
	},
	//------------------------
	// 173 George breaks the branch of the cliff edge tree. GEOSYR20
	{
		FX_BREKSTIK,	// sampleId
		FX_SPOT,			// type
		16,						// delay (or random chance) *
		{							// roomVolList
			{54,6,6},		// {roomNo,leftVol,rightVol}
			{0,0,0},		// NULL-TERMINATOR
		},
	},
	//------------------------
	// 174 George climbs down the cliff face. GEOSYR23
	{
		FX_CLIMBDWN,	// sampleId
		FX_SPOT,			// type
		6,						// delay (or random chance) *
		{							// roomVolList
			{54,6,7},		// {roomNo,leftVol,rightVol}
			{0,0,0},		// NULL-TERMINATOR
		},
	},
	//------------------------
	// 175 George pulls ring:  GEOSYR26
	{
		FX_RINGPULL,	// sampleId
		FX_SPOT,			// type
		1,						// delay (or random chance) *
		{							// roomVolList
			{54,7,4},		// {roomNo,leftVol,rightVol}
			{0,0,0},		// NULL-TERMINATOR
		},
	},
	//------------------------
	// 176 Bull's Head door opens: SECDOR
	{
		FX_SECDOR54,	// sampleId
		FX_SPOT,			// type
		1,						// delay (or random chance) *
		{							// roomVolList
			{54,7,8},		// {roomNo,leftVol,rightVol}
			{0,0,0},		// NULL-TERMINATOR
		},
	},
	//------------------------
	// 177 Inside Bull's Head door opens: DOOR55 (and its reverse).
	{
		FX_SECDOR55,	// sampleId
		FX_SPOT,			// type
		1,						// delay (or random chance) *
		{							// roomVolList
			{55,4,12},	// {roomNo,leftVol,rightVol}
			{0,0,0},		// NULL-TERMINATOR
		},
	},
	//------------------------
	// 178 Ayub opens door. AYU1
	{
		FX_AYUBDOOR,	// sampleId
		FX_SPOT,			// type
		1,						// delay (or random chance) *
		{							// roomVolList
			{45,8,3},		// {roomNo,leftVol,rightVol}
			{0,0,0},		// NULL-TERMINATOR
		},
	},
	//------------------------
	// 179 George knocks at the door in location 4: GEONOK followed by reverse of GEONOK
	{
		FX_KNOKKNOK,	// sampleId
		FX_SPOT,			// type
		13,						// delay (or random chance) *
		{							// roomVolList
			{4,16,16},	// {roomNo,leftVol,rightVol}
			{0,0,0},		// NULL-TERMINATOR
		},
	},
	//------------------------
	// 180 George knocks at the door in location 5: GEONOK05
	{
		FX_KNOKKNOK,	// sampleId
		FX_SPOT,			// type
		8,						// delay (or random chance) *
		{							// roomVolList
			{5,8,8},		// {roomNo,leftVol,rightVol}
			{0,0,0},		// NULL-TERMINATOR
		},
	},
	//------------------------
	// 181 Those pesky Irish birds turn up in Spain, too.
	{
		FX_SPNBIRD1,	// sampleId
		FX_RANDOM,		// type
		720,					// delay (or random chance) *
		{							// roomVolList
			{57,1,4},
			{58,8,4},
			{0,0,0},		// NULL-TERMINATOR
		},
	},
	//------------------------
	// 182 Those pesky Irish birds turn up in Spain, too.
	{
		FX_SPNBIRD2,	// sampleId
		FX_RANDOM,		// type
		697,					// delay (or random chance) *
		{							// roomVolList
			{57,4,8},		// {roomNo,leftVol,rightVol}
			{58,4,1},
			{0,0,0},		// NULL-TERMINATOR
		},
	},
	//------------------------
	// 183 The secret door in the well: SECDOR61 anim
	{
		FX_SECDOR61,	// sampleId
		FX_SPOT,			// type
		1,						// delay (or random chance) *
		{							// roomVolList
			{61,4,8},		// {roomNo,leftVol,rightVol}
			{0,0,0},		// NULL-TERMINATOR
		},
	},
	//------------------------
	// 184 Spanish countryside ambience
	{
		FX_SPAIN,			// sampleId
		FX_LOOP,			// type
		0,						// delay (or random chance) *
		{							// roomVolList
			{57,1,2},		//
			{58,2,2},		//
			{60,1,1},		//
			{0,0,0},		// NULL-TERMINATOR
		},
	},
	//------------------------
	// 185 Spanish well ambience
	{
		FX_WELLDRIP,	// sampleId
		FX_LOOP,			// type
		0,						// delay (or random chance) *
		{							// roomVolList
			{61,4,4},		// {roomNo,leftVol,rightVol}
			{0,0,0},		// NULL-TERMINATOR
		},
	},
	//------------------------
	// 186 Fish falls on George's head: GEOTOT29
	{
		FX_FISHFALL,	// sampleId
		FX_SPOT,			// type
		60,						// delay (or random chance) *
		{							// roomVolList
			{29,10,16},	// {roomNo,leftVol,rightVol}
			{0,0,0},		// NULL-TERMINATOR
		},
	},
	//------------------------
	// 187 Hospital exterior ambience
	{
		FX_HOSPEXT,		// sampleId
		FX_LOOP,			// type
		0,						// delay (or random chance) *
		{							// roomVolList
			{31,3,4},		// {roomNo,leftVol,rightVol}
			{0,0,0},		// NULL-TERMINATOR
		},
	},
	//------------------------
	// 188 Hospital exterior gravel footstep #1
	{
		FX_GRAVEL1,		// sampleId
		FX_SPOT,			// type
		0,						// delay (or random chance) *
		{							// roomVolList
			{31,16,16},	// {roomNo,leftVol,rightVol}
			{0,0,0},		// NULL-TERMINATOR
		},
	},
	//------------------------
	// 189 Hospital exterior gravel footstep #2
	{
		FX_GRAVEL2,		// sampleId
		FX_SPOT,			// type
		0,						// delay (or random chance) *
		{							// roomVolList
			{31,16,16},	// {roomNo,leftVol,rightVol}
			{0,0,0},		// NULL-TERMINATOR
		},
	},
	//------------------------
	//------------------------
	// 190 George opens sarcophagus: GEOSAR
	{
		FX_SARCO28A,	// sampleId
		FX_SPOT,			// type
		26,						// delay (or random chance) *
		{							// roomVolList
			{28,6,6},		// {roomNo,leftVol,rightVol}
			{0,0,0},		// NULL-TERMINATOR
		},
	},
	//------------------------
	// 191 George closes sarcophagus: GEOSAR2
	{
		FX_SARCO28B,	// sampleId
		FX_SPOT,			// type
		24,						// delay (or random chance) *
		{							// roomVolList
			{28,3,3},		// {roomNo,leftVol,rightVol}
			{0,0,0},		// NULL-TERMINATOR
		},
	},
	//------------------------
	// 192 Guard opens sarcophagus: MUSOPN
	{
		FX_SARCO28C,	// sampleId
		FX_SPOT,			// type
		14,						// delay (or random chance) *
		{							// roomVolList
			{28,3,3},		// {roomNo,leftVol,rightVol}
			{0,0,0},		// NULL-TERMINATOR
		},
	},
	//------------------------
	// 193 George peeks out of sarcophagus: GEOPEEK
	{
		FX_SARCO29,		// sampleId
		FX_SPOT,			// type
		4,						// delay (or random chance) *
		{							// roomVolList
			{29,5,10},	// {roomNo,leftVol,rightVol}
			{0,0,0},		// NULL-TERMINATOR
		},
	},
	//------------------------
	// 194 The rope drops into the room: ROPE29
	{
		FX_ROPEDOWN,	// sampleId
		FX_SPOT,			// type
		3,						// delay (or random chance) *
		{							// roomVolList
			{29,3,3},		// {roomNo,leftVol,rightVol}
			{0,0,0},		// NULL-TERMINATOR
		},
	},
	//------------------------
	// 195 George pushes the totem pole: GEOTOT29
	{
		FX_TOTEM29A,	// sampleId
		FX_SPOT,			// type
		30,						// delay (or random chance) *
		{							// roomVolList
			{29,12,12},	// {roomNo,leftVol,rightVol}
			{0,0,0},		// NULL-TERMINATOR
		},
	},
	//------------------------
	// 196 George pushes the totem pole over: GEOTOTB
	{
		FX_TOTEM29B,	// sampleId
		FX_SPOT,			// type
		1,						// delay (or random chance) *
		{							// roomVolList
			{29,16,16},	// {roomNo,leftVol,rightVol}
			{0,0,0},		// NULL-TERMINATOR
		},
	},
	//------------------------
	// 197 George rocks the totem pole in museum hours: TOTEM28
	{
		FX_TOTEM28A,	// sampleId
		FX_SPOT,			// type
		8,						// delay (or random chance) *
		{							// roomVolList
			{28,4,4},		// {roomNo,leftVol,rightVol}
			{0,0,0},		// NULL-TERMINATOR
		},
	},
	//------------------------
	// 198 Ambient sound for Montfaucon Square
	{
		FX_MONTAMB,		// sampleId
		FX_LOOP,			// type
		0,						// delay (or random chance) *
		{							// roomVolList
			{36,6,6},		// {roomNo,leftVol,rightVol}
			{40,6,6},
			{0,0,0},		// NULL-TERMINATOR
		},
	},
	//------------------------
	// 199 Ambient sound churchyard.
	{
		FX_WIND71,		// sampleId
		FX_RANDOM,		// type
		720,					// delay (or random chance) *
		{							// roomVolList
			{71,10,10},	// {roomNo,leftVol,rightVol}
			{72,7,7},
			{0,0,0},		// NULL-TERMINATOR
		},
	},
	//------------------------
	// 200 Owl cry #1 in churchyard
	{
		FX_OWL71A,		// sampleId
		FX_RANDOM,		// type
		720,					// delay (or random chance) *
		{							// roomVolList
			{71,8,8},		// {roomNo,leftVol,rightVol}
			{72,6,4},
			{0,0,0},		// NULL-TERMINATOR
		},
	},
	//------------------------
	// 201 Owl cry #2 in churchyard
	{
		FX_OWL71B,		// sampleId
		FX_RANDOM,		// type
		1080,					// delay (or random chance) *
		{							// roomVolList
			{71,8,8},		// {roomNo,leftVol,rightVol}
			{72,7,6},
			{0,0,0},		// NULL-TERMINATOR
		},
	},
	//------------------------
	// 202 Air conditioner in the museum
	{
		FX_AIRCON28,	// sampleId
		FX_LOOP,			// type
		0,						// delay (or random chance) *
		{							// roomVolList
			{28,6,6},		// {roomNo,leftVol,rightVol}
			{29,3,3},
			{0,0,0},		// NULL-TERMINATOR
		},
	},
	//------------------------
	// 203 George breaks the handle off in the church tower. GEOWND72
	{
		FX_COG72A,		// sampleId
		FX_SPOT,			// type
		5,						// delay (or random chance) *
		{							// roomVolList
			{72,10,10},
			{0,0,0},		// NULL-TERMINATOR
		},
	},
	//------------------------
	// 204 Countess' room ambience
	{
		FX_AMBIEN56,	// sampleId
		FX_LOOP,			// type
		0,						// delay (or random chance) *
		{							// roomVolList
			{56,3,2},		// {roomNo,leftVol,rightVol}
			{0,0,0},		// NULL-TERMINATOR
		},
	},
	//------------------------
	// 205 Musical effect for George drinking beer. GEODRN20
	{
		FX_DRINK,			// sampleId
		FX_SPOT,			// type
		17,						// delay (or random chance) *
		{							// roomVolList
			{20,12,12},	// {roomNo,leftVol,rightVol}
			{0,0,0},		// NULL-TERMINATOR
		},
	},
	//------------------------
	// 206 Torch thrown through the air. GEOTHROW
	{
		FX_TORCH73,		// sampleId
		FX_SPOT,			// type
		14,						// delay (or random chance) *
		{							// roomVolList
			{73,16,16},	// {roomNo,leftVol,rightVol}
			{0,0,0},		// NULL-TERMINATOR
		},
	},
	//------------------------
	// 207 Internal train ambience.
	{
		FX_TRAININT,	// sampleId
		FX_LOOP,			// type
		0,						// delay (or random chance) *
		{							// roomVolList
			{63,3,3},		// {roomNo,leftVol,rightVol}
			{65,2,2},
			{66,2,2},
			{67,2,2},
			{69,2,2},
			{0,0,0},		// NULL-TERMINATOR
		},
	},
	//------------------------
	// 208 Countess' clock. PENDULUM. Note: Trigger the sound effect on alternate runs of the pendulum animation.
	{
		FX_PENDULUM,	// sampleId
		FX_SPOT,			// type
		6,						// delay (or random chance) *
		{							// roomVolList
			{56,2,2},		// {roomNo,leftVol,rightVol}
			{0,0,0},		// NULL-TERMINATOR
		},
	},
	//------------------------
	// 209 Compartment door.  DOOR65
	{
		FX_DOOR65,		// sampleId
		FX_SPOT,			// type
		1,						// delay (or random chance) *
		{							// roomVolList
			{65,3,3},		// {roomNo,leftVol,rightVol}
			{0,0,0},		// NULL-TERMINATOR
		},
	},
	//------------------------
	//------------------------
	// 210 Opening window. GEOOPN1
	{
		FX_WINDOW66,	// sampleId
		FX_SPOT,			// type
		8,						// delay (or random chance) *
		{							// roomVolList
			{66,5,5},		// {roomNo,leftVol,rightVol}
			{0,0,0},		// NULL-TERMINATOR
		},
	},
	//------------------------
	// 211 Wind rip by the open window. Triggered at the end of effect 210.
	{
		FX_WIND66,		// sampleId
		FX_LOOP,			// type
		0,						// delay (or random chance) *
		{							// roomVolList
			{66,5,5},		// {roomNo,leftVol,rightVol}
			{0,0,0},		// NULL-TERMINATOR
		},
	},
	//------------------------
	// 212 George electrocutes himself on the pantograph. Fool.  GEOSHK64
	{
		FX_SHOCK63,		// sampleId
		FX_SPOT,			// type
		11,						// delay (or random chance) *
		{							// roomVolList
			{63,12,14},	// {roomNo,leftVol,rightVol}
			{0,0,0},		// NULL-TERMINATOR
		},
	},
	//------------------------
	// 213 The train brakes violently. GEOSTP69
	{
		FX_BRAKES,		// sampleId
		FX_SPOT,			// type
		13,						// delay (or random chance) *
		{							// roomVolList
			{69,16,16},	// {roomNo,leftVol,rightVol}
			{0,0,0},		// NULL-TERMINATOR
		},
	},
	//------------------------
	// 214 The train ticks over. From the end of BRAKE.
	{
		FX_TICK69,		// sampleId
		FX_LOOP,			// type
		0,						// delay (or random chance) *
		{							// roomVolList
			{69,8,8},		// {roomNo,leftVol,rightVol}
			{0,0,0},		// NULL-TERMINATOR
		},
	},
	//------------------------
	// 215 Eklund shoot Khan.  FIGHT69
	{
		FX_EKSHOOT,		// sampleId
		FX_SPOT,			// type
		120,					// delay (or random chance) *
		{							// roomVolList
			{69,12,12},	// {roomNo,leftVol,rightVol}
			{0,0,0},		// NULL-TERMINATOR
		},
	},
	//------------------------
	// 216 Eklund shoots George. GEODIE69
	{
		FX_EKSHOOT,		// sampleId
		FX_SPOT,			// type
		21,						// delay (or random chance) *
		{							// roomVolList
			{69,12,12},	// {roomNo,leftVol,rightVol}
			{0,0,0},		// NULL-TERMINATOR
		},
	},
	//------------------------
	// 217 Khan pulls the door open. FIGHT69
	{
		FX_DOOR69,		// sampleId
		FX_SPOT,			// type
		42,						// delay (or random chance) *
		{							// roomVolList
			{69,8,12},	// {roomNo,leftVol,rightVol}
			{0,0,0},		// NULL-TERMINATOR
		},
	},
	//------------------------
	// 218 Wind shriek. Loops from the end of DOOR69 wav to the beginning of BRAKES.
	{
		FX_WIND66,		// sampleId
		FX_LOOP,			// type
		0,						// delay (or random chance) *
		{							// roomVolList
			{69,8,12},	// {roomNo,leftVol,rightVol}
			{0,0,0},		// NULL-TERMINATOR
		},
	},
	//------------------------
	// 219 Brakes releasing pressure. Only after BRAKE has been run.
	{
		FX_PNEUMO69,	// sampleId
		FX_RANDOM,		// type
		720,					// delay (or random chance) *
		{							// roomVolList
			{69,8,8},		// {roomNo,leftVol,rightVol}
			{0,0,0},		// NULL-TERMINATOR
		},
	},
	//------------------------
	// 220 External train sound. Played while George is on the top of the train.
	{
		FX_TRAINEXT,	// sampleId
		FX_LOOP,			// type
		0,						// delay (or random chance) *
		{							// roomVolList
			{63,8,8},		// {roomNo,leftVol,rightVol}
			{0,0,0},		// NULL-TERMINATOR
		},
	},
	//------------------------
	// 221 The passing train. FIGHT69
	{
		FX_TRNPASS,		// sampleId
		FX_SPOT,			// type
		102,					// delay (or random chance) *
		{							// roomVolList
			{69,4,16},	// {roomNo,leftVol,rightVol}
			{0,0,0},		// NULL-TERMINATOR
		},
	},
	//------------------------
	// 222 George descends into sewer. GEODESO6
	{
		FX_LADD2,			// sampleId
		FX_SPOT,			// type
		1,						// delay (or random chance) *
		{							// roomVolList
			{6,4,4},		// {roomNo,leftVol,rightVol}
			{0,0,0},		// NULL-TERMINATOR
		},
	},
	//------------------------
	// 223 George ascends into alley. GEOASC06
	{
		FX_LADD3,			// sampleId
		FX_SPOT,			// type
		10,						// delay (or random chance) *
		{							// roomVolList
			{6,4,4},		// {roomNo,leftVol,rightVol}
			{0,0,0},		// NULL-TERMINATOR
		},
	},
	//------------------------
	// 224 George replaces manhole cover. GEOMAN9
	{
		FX_COVERON2,	// sampleId
		FX_SPOT,			// type
		19,						// delay (or random chance) *
		{							// roomVolList
			{2,12,11},	// {roomNo,leftVol,rightVol}
			{0,0,0},		// NULL-TERMINATOR
		},
	},
	//------------------------
	// 225 Montfaucon sewer ambience.
	{
		FX_AMBIEN37,	// sampleId
		FX_LOOP,			// type
		0,						// delay (or random chance) *
		{							// roomVolList
			{37,5,5},		// {roomNo,leftVol,rightVol}
			{0,0,0},		// NULL-TERMINATOR
		},
	},
	//------------------------
	// 226 George's winning smile. GEOJMP72.
	{
		FX_PING,			// sampleId
		FX_SPOT,			// type
		26,						// delay (or random chance) *
		{							// roomVolList
			{72,10,14},	// {roomNo,leftVol,rightVol}
			{0,0,0},		// NULL-TERMINATOR
		},
	},
	//------------------------
	// 227 George starts to open the manhole. GEO36KNE
	{
		FX_MANOP36,		// sampleId
		FX_SPOT,			// type
		19,						// delay (or random chance) *
		{							// roomVolList
			{36,4,6},		// {roomNo,leftVol,rightVol}
			{0,0,0},		// NULL-TERMINATOR
		},
	},
	//------------------------
	// 228 George opens the manhole. GEO36OPE
	{
		FX_PULLUP36,	// sampleId
		FX_SPOT,			// type
		9,						// delay (or random chance) *
		{							// roomVolList
			{36,4,6},		// {roomNo,leftVol,rightVol}
			{0,0,0},		// NULL-TERMINATOR
		},
	},
	//------------------------
	// 229 George replaces the manhole cover. GEO36CLO
	{
		FX_REPLCE36,	// sampleId
		FX_SPOT,			// type
		20,						// delay (or random chance) *
		{							// roomVolList
			{36,4,6},		// {roomNo,leftVol,rightVol}
			{0,0,0},		// NULL-TERMINATOR
		},
	},
	//------------------------
	//------------------------
	// 230 George knocks at righthand arch. GEO37TA3
	{
		FX_KNOCK37,		// sampleId
		FX_SPOT,			// type
		20,						// delay (or random chance) *
		{							// roomVolList
			{37,6,12},	// {roomNo,leftVol,rightVol}
			{0,0,0},		// NULL-TERMINATOR
		},
	},
	//------------------------
	// 231 George knocks at middle or lefthand arch. GEO37TA1 or GEO37TA2.
	{
		FX_KNOCK37B,	// sampleId
		FX_SPOT,			// type
		20,						// delay (or random chance) *
		{							// roomVolList
			{37,4,8},		// {roomNo,leftVol,rightVol}
			{0,0,0},		// NULL-TERMINATOR
		},
	},
	//------------------------
	// 232 George winds the chain down  HOO37LBO
	{
		FX_CHAIN37,		// sampleId
		FX_SPOT,			// type
		14,						// delay (or random chance) *
		{							// roomVolList
			{37,6,10},	// {roomNo,leftVol,rightVol}
			{0,0,0},		// NULL-TERMINATOR
		},
	},
	//------------------------
	// 233 George winds the chain up.  HOO37LBO (In reverse)
	{
		FX_CHAIN37B,	// sampleId
		FX_SPOT,			// type
		1,						// delay (or random chance) *
		{							// roomVolList
			{37,6,10},	// {roomNo,leftVol,rightVol}
			{0,0,0},		// NULL-TERMINATOR
		},
	},
	//------------------------
	// 234 George breaks hole in door. GEO37TA4
	{
		FX_HOLE37,		// sampleId
		FX_SPOT,			// type
		11,						// delay (or random chance) *
		{							// roomVolList
			{37,6,12},	// {roomNo,leftVol,rightVol}
			{0,0,0},		// NULL-TERMINATOR
		},
	},
	//------------------------
	// 235 Plaster door collapses. DOR37COL
	{
		FX_DOOR37,		// sampleId
		FX_SPOT,			// type
		23,						// delay (or random chance) *
		{							// roomVolList
			{37,8,15},	// {roomNo,leftVol,rightVol}
			{0,0,0},		// NULL-TERMINATOR
		},
	},
	//------------------------
	// 236 Barge winch. GEO37TUL (If it runs more than once, trigger the effect on frame one. Incidentally, this is a reversible so the effect must launch on frame one of the .cdr version as well. )
	{
		FX_WINCH37,		// sampleId
		FX_SPOT,			// type
		1,						// delay (or random chance) *
		{							// roomVolList
			{37,16,16},	// {roomNo,leftVol,rightVol}
			{0,0,0},		// NULL-TERMINATOR
		},
	},
	//------------------------
	// 237 George places chess piece. GEOSPA17
	{
		FX_CHESS,			// sampleId
		FX_SPOT,			// type
		23,						// delay (or random chance) *
		{							// roomVolList
			{59,4,4},		// {roomNo,leftVol,rightVol}
			{0,0,0},		// NULL-TERMINATOR
		},
	},
	//------------------------
	// 238 Piano loop for the upstairs hotel corridor.
	{
		FX_PIANO14,		// sampleId
		FX_LOOP,			// type
		0,						// delay (or random chance) *
		{							// roomVolList
			{14,2,2},		// {roomNo,leftVol,rightVol}
			{0,0,0},		// NULL-TERMINATOR
		},
	},
	//------------------------
	// 239 Door opens in church tower. PANEL72
	{
		FX_SECDOR72,	// sampleId
		FX_SPOT,			// type
		1,						// delay (or random chance) *
		{							// roomVolList
			{72,8,10},	// {roomNo,leftVol,rightVol}
			{0,0,0},		// NULL-TERMINATOR
		},
	},
	//------------------------
	//------------------------
	// 240 George rummages through debris. Tied to the end of the whichever crouch is used. Use either this one or RUMMAGE2 alternatively or randomly. Same kind of schtick as the pick axe noises, I suppose.
	{
		FX_RUMMAGE1,	// sampleId
		FX_SPOT,			// type
		9,						// delay (or random chance) *
		{							// roomVolList
			{72,8,6},		// {roomNo,leftVol,rightVol}
			{0,0,0},		// NULL-TERMINATOR
		},
	},
	//------------------------
	// 241 George rummages through debris. See above for notes.
	{
		FX_RUMMAGE2,	// sampleId
		FX_SPOT,			// type
		9,						// delay (or random chance) *
		{							// roomVolList
			{72,8,6},		// {roomNo,leftVol,rightVol}
			{0,0,0},		// NULL-TERMINATOR
		},
	},
	//------------------------
	// 242 Gust of wind in the graveyard.
	{
		FX_GUST71,		// sampleId
		FX_RANDOM,		// type
		1080,					// delay (or random chance) *
		{							// roomVolList
			{71,3,3},		// {roomNo,leftVol,rightVol}
			{72,2,1},
			{0,0,0},		// NULL-TERMINATOR
		},
	},
	//------------------------
	// 243 Violin ambience for Ireland.
	{
		FX_VIOLIN19,	// sampleId
		FX_LOOP,			// type
		0,						// delay (or random chance) *
		{							// roomVolList
			{19,3,3},		// {roomNo,leftVol,rightVol}
			{21,2,2},
			{26,2,2},
			{0,0,0},		// NULL-TERMINATOR
		},
	},
	//------------------------
	// 244 Footstep #1 for underground locations. Same schtick as for 188 and 189.
	{
		FX_SEWSTEP1,	// sampleId
		FX_SPOT,			// type
		0,						// delay (or random chance) *
		{							// roomVolList
			{6,8,8},		// {roomNo,leftVol,rightVol}
			{7,8,8},
			{0,0,0},		// NULL-TERMINATOR
		},
	},
	//------------------------
	// 245 Footstep #2 for underground locations. Same schtick as for 188 and 189.
	{
		FX_SEWSTEP2,	// sampleId
		FX_SPOT,			// type
		0,						// delay (or random chance) *
		{							// roomVolList
			{6,16,16},	// {roomNo,leftVol,rightVol}
			{7,16,16},
			{0,0,0},		// NULL-TERMINATOR
		},
	},
	//------------------------
	// 246 Nico's carabiner as she descends into the museum. NICPUS1
	{
		FX_CARABINE,	// sampleId
		FX_SPOT,			// type
		4,						// delay (or random chance) *
		{							// roomVolList
			{29,8,8},		// {roomNo,leftVol,rightVol}
			{0,0,0},		// NULL-TERMINATOR
		},
	},
	//------------------------
	// 247 Rosso is shot (with a piece of field artillery).  ROSSHOT
	{
		FX_GUN79,			// sampleId
		FX_SPOT,			// type
		2,						// delay (or random chance) *
		{							// roomVolList
			{79,16,16},	// {roomNo,leftVol,rightVol}
			{0,0,0},		// NULL-TERMINATOR
		},
	},
	//------------------------
	// 248 George is hit by the thrown stilletto. GEODIE1
	{
		FX_DAGGER1,		// sampleId
		FX_SPOT,			// type
		2,						// delay (or random chance) *
		{							// roomVolList
			{73,12,12},	// {roomNo,leftVol,rightVol}
			{0,0,0},		// NULL-TERMINATOR
		},
	},
	//------------------------
	// 249 George is hit by the thrown stilletto after walking forward. GEODIE2
	{
		FX_DAGGER1,		// sampleId
		FX_SPOT,			// type
		9,						// delay (or random chance) *
		{							// roomVolList
			{73,12,12},	// {roomNo,leftVol,rightVol}
			{0,0,0},		// NULL-TERMINATOR
		},
	},
	//------------------------
	//------------------------
	// 250 Can hits the well water. The cue is in GAR2SC57.TXT immediately after the line, "over: Lopez threw the can away. It seemed to fall an awfully long way."
	{
		FX_CANFALL,		// sampleId
		FX_SPOT,			// type
		4,						// delay (or random chance) *
		{							// roomVolList
			{57,16,16},	// {roomNo,leftVol,rightVol}
			{0,0,0},		// NULL-TERMINATOR
		},
	},
	//------------------------
	// 251 Mad, fizzing damp and ancient gunpowder after the application of a torch.
	{
		FX_GUNPOWDR,	// sampleId
		FX_LOOP,			// type
		0,						// delay (or random chance) *
		{							// roomVolList
			{73,12,12},	// {roomNo,leftVol,rightVol}
			{0,0,0},		// NULL-TERMINATOR
		},
	},
	//------------------------
	// 252 Maguire whistling. MAGSLK. Plays while Maguire is idling, stops abruptly when he does something else.
	{
		FX_WHISTLE,		// sampleId
		FX_LOOP,			// type
		0,						// delay (or random chance) *
		{							// roomVolList
			{19,2,5},		// {roomNo,leftVol,rightVol}
			{0,0,0},		// NULL-TERMINATOR
		},
	},
	//------------------------
	// 253  George is hit by the goat. GEOHITR and GEOHITL.
	{
		FX_GEOGOAT,		// sampleId
		FX_SPOT,			// type
		11,						// delay (or random chance) *
		{							// roomVolList
			{24,10,10},	// {roomNo,leftVol,rightVol}
			{0,0,0},		// NULL-TERMINATOR
		},
	},
	//------------------------
	// 254 Manager says, "Hello". MAN2
	{
		FX_MANG1,			// sampleId
		FX_SPOT,			// type
		7,						// delay (or random chance) *
		{							// roomVolList
			{49,6,5},		// {roomNo,leftVol,rightVol}
			{0,0,0},		// NULL-TERMINATOR
		},
	},
	//------------------------
	// 255 Manager says, Don't go in there!" MAN3
	{
		FX_MANG2,			// sampleId
		FX_SPOT,			// type
		11,						// delay (or random chance) *
		{							// roomVolList
			{49,6,5},		// {roomNo,leftVol,rightVol}
			{0,0,0},		// NULL-TERMINATOR
		},
	},
	//------------------------
	// 256 Manager says, "Here are the keys." MAN4
	{
		FX_MANG3,			// sampleId
		FX_SPOT,			// type
		6,						// delay (or random chance) *
		{							// roomVolList
			{49,6,5},		// {roomNo,leftVol,rightVol}
			{0,0,0},		// NULL-TERMINATOR
		},
	},
	//------------------------
	// 257 George pulls the lion's tooth. GEOSPA26
	{
		FX_TOOTHPUL,	// sampleId
		FX_SPOT,			// type
		19,						// delay (or random chance) *
		{							// roomVolList
			{61,8,16},	// {roomNo,leftVol,rightVol}
			{0,0,0},		// NULL-TERMINATOR
		},
	},
	//------------------------
	// 258 George escapes the lion.  LION1
	{
		FX_LIONFALL,	// sampleId
		FX_SPOT,			// type
		7,						// delay (or random chance) *
		{							// roomVolList
			{61,8,16},	// {roomNo,leftVol,rightVol}
			{0,0,0},		// NULL-TERMINATOR
		},
	},
	//------------------------
	// 259 George gets flattened. LION2
	{
		FX_LIONFAL2,	// sampleId
		FX_SPOT,			// type
		4,						// delay (or random chance) *
		{							// roomVolList
			{61,8,16},	// {roomNo,leftVol,rightVol}
			{0,0,0},		// NULL-TERMINATOR
		},
	},
	//------------------------
	// 260 Rosso dies. ROSSFALL
	{
		FX_ROSSODIE,	// sampleId
		FX_SPOT,			// type
		1,						// delay (or random chance) *
		{							// roomVolList
			{74,16,16},	// {roomNo,leftVol,rightVol}
			{0,0,0},		// NULL-TERMINATOR
		},
	},
	//------------------------
	// 261 Eklund chokes George. FIGHT79
	{
		FX_CHOKE1,		// sampleId
		FX_SPOT,			// type
		9,						// delay (or random chance) *
		{							// roomVolList
			{79,16,16},	// {roomNo,leftVol,rightVol}
			{0,0,0},		// NULL-TERMINATOR
		},
	},
	//------------------------
	// 262 Eklund chokes George some more. FIGHT79
	{
		FX_CHOKE2,		// sampleId
		FX_SPOT,			// type
		54,						// delay (or random chance) *
		{							// roomVolList
			{79,16,16},	// {roomNo,leftVol,rightVol}
			{0,0,0},		// NULL-TERMINATOR
		},
	},
	//------------------------
	// 263 Eklund dies. FIGHT79
	{
		FX_FIGHT2,		// sampleId
		FX_SPOT,			// type
		44,					// delay (or random chance) *
		{							// roomVolList
			{79,16,16},	// {roomNo,leftVol,rightVol}
			{0,0,0},		// NULL-TERMINATOR
		},
	},
	//------------------------
	// 264 George hears museum break-in. GEOSUR29
	{
		FX_DOOR29,		// sampleId
		FX_SPOT,			// type
		10,						// delay (or random chance) *
		{							// roomVolList
			{94,14,10},	// {roomNo,leftVol,rightVol}
			{0,0,0},		// NULL-TERMINATOR
		},
	},
	//------------------------
	// 265 George hits the floor having been shot. GEODED.
	{
		FX_GDROP29,		// sampleId
		FX_SPOT,			// type
		27,						// delay (or random chance) *
		{							// roomVolList
			{29,10,16},	// {roomNo,leftVol,rightVol}
			{0,0,0},		// NULL-TERMINATOR
		},
	},
	//------------------------
	// 266 George hits the floor having been stunned. GEOFISH
	{
		FX_GDROP29,		// sampleId
		FX_SPOT,			// type
		27,						// delay (or random chance) *
		{							// roomVolList
			{29,10,16},	// {roomNo,leftVol,rightVol}
			{0,0,0},		// NULL-TERMINATOR
		},
	},
	//------------------------
	// 267 Fitz being knocked down as heard from inside the pub. Triggered from the script, I think. This is just a stopgap until Hackenbacker do the full version for the Smacker, then I'll sample the requisite bit and put it in here.
	{
		FX_FITZHIT,		// sampleId
		FX_SPOT,			// type
		1,						// delay (or random chance) *
		{							// roomVolList
			{20,16,10},	// {roomNo,leftVol,rightVol}
			{0,0,0},		// NULL-TERMINATOR
		},
	},
	//------------------------
	// 268 Gendarme shoots lock off. GENSHOT
	{
		FX_GUN34,			// sampleId
		FX_SPOT,			// type
		1,						// delay (or random chance) *
		{							// roomVolList
			{34,16,16},	// {roomNo,leftVol,rightVol}
			{0,0,0},		// NULL-TERMINATOR
		},
	},
	//------------------------
	// 269 ECG alarm, Marquet in trouble. Start looping imeediately before George says, "Thanks, Bunny".
	// Incidentally, James, please switch off Mr Shiney permanently when George first gets into Marquet's room. He gets in the way when they're figuring out that Eklund's an imposter.
	{
		FX_PULSE2,		// sampleId
		FX_LOOP,			// type
		1,						// delay (or random chance) *
		{							// roomVolList
			{30,16,16},	// {roomNo,leftVol,rightVol}
			{34,12,12},	// {roomNo,leftVol,rightVol}
			{0,0,0},		// NULL-TERMINATOR
		},
	},
	//------------------------
	//------------------------
	// 270 ECG alarm, Marquet dead. Switch off the previous effect and replace with this immediately before the gendarme says, "Stand back, messieurs."
	{
		FX_PULSE3,		// sampleId
		FX_LOOP,			// type
		1,						// delay (or random chance) *
		{							// roomVolList
			{30,16,16},	// {roomNo,leftVol,rightVol}
			{34,13,13},
			{35,13,13},
			{0,0,0},		// NULL-TERMINATOR
		},
	},
	//------------------------
	// 271 Door closing. GEOENT15
	{
		FX_DORCLOSE,	// sampleId
		FX_SPOT,			// type
		4,						// delay (or random chance) *
		{							// roomVolList
			{15,12,12},	// {roomNo,leftVol,rightVol}
			{0,0,0},		// NULL-TERMINATOR
		},
	},
	//------------------------
	// 272 Cupboard opening. GEOCOT
	{
		FX_CUPBOPEN,	// sampleId
		FX_SPOT,			// type
		8,						// delay (or random chance) *
		{							// roomVolList
			{33,12,12},	// {roomNo,leftVol,rightVol}
			{0,0,0},		// NULL-TERMINATOR
		},
	},
	//------------------------
	// 273 Cupboard closing. GEOCOT
	{
		FX_CUPBCLOS,	// sampleId
		FX_SPOT,			// type
		33,						// delay (or random chance) *
		{							// roomVolList
			{33,12,12},	// {roomNo,leftVol,rightVol}
			{0,0,0},		// NULL-TERMINATOR
		},
	},
	//------------------------
	// 274 Closing door when George leaves hotel room. GEOLVS15 and GEODOR17 (they're identical).
	{
		FX_DORCLOSE,	// sampleId
		FX_SPOT,			// type
		44,						// delay (or random chance) *
		{							// roomVolList
			{15,12,12},	// {roomNo,leftVol,rightVol}
			{17,12,12},
			{0,0,0},		// NULL-TERMINATOR
		},
	},
	//------------------------
	// 275 Closing door when George leaves the pub. DOROPN20 (Reversed)
	{
		FX_DORCLOSE20,// sampleId
		FX_SPOT,			// type
		11,						// delay (or random chance) *
		{							// roomVolList
			{20,12,12},	// {roomNo,leftVol,rightVol}
			{0,0,0},		// NULL-TERMINATOR
		},
	},
	//------------------------
	// 276 Nico call for a cab.  NICPHN10
	{
		FX_PHONICO1,	// sampleId
		FX_SPOT,			// type
		15,						// delay (or random chance) *
		{							// roomVolList
			{10,12,12},	// {roomNo,leftVol,rightVol}
			{0,0,0},		// NULL-TERMINATOR
		},
	},
	//------------------------
	// 277 Nico puts down the phone. NICDWN10
	{
		FX_FONEDN,		// sampleId
		FX_SPOT,			// type
		6,						// delay (or random chance) *
		{							// roomVolList
			{10,12,12},	// {roomNo,leftVol,rightVol}
			{0,0,0},		// NULL-TERMINATOR
		},
	},
	//------------------------
	// 278 Painter puts down the phone. PAI41HAN
	{
		FX_FONEDN41,	// sampleId
		FX_SPOT,			// type
		5,						// delay (or random chance) *
		{							// roomVolList
			{41,12,12},	// {roomNo,leftVol,rightVol}
			{0,0,0},		// NULL-TERMINATOR
		},
	},
	//------------------------
	// 279 Mechanical hum of heating system in the dig lobby.
	{
		FX_AIRCON41,	// sampleId
		FX_LOOP,			// type
		0,						// delay (or random chance) *
		{							// roomVolList
			{41,6,6},		// {roomNo,leftVol,rightVol}
			{43,8,8},
			{0,0,0},		// NULL-TERMINATOR
		},
	},
	//------------------------
	//------------------------
	// 280 The Sword is Reforged (Grandmaster gets zapped) GMPOWER
	{
		FX_REFORGE1,	// sampleId
		FX_SPOT,			// type
		1,						// delay (or random chance) *
		{							// roomVolList
			{78,16,16},	// {roomNo,leftVol,rightVol}
			{0,0,0},		// NULL-TERMINATOR
		},
	},
	//------------------------
	// 281 The Sword is Reforged (G&N gawp at the spectacle) There's no anim I know of to tie it to unless the flickering blue light is one.
	{
		FX_REFORGE2,	// sampleId
		FX_SPOT,			// type
		1,						// delay (or random chance) *
		{							// roomVolList
			{75,12,12},	// {roomNo,leftVol,rightVol}
			{0,0,0},		// NULL-TERMINATOR
		},
	},
	//------------------------
	// 282 The Sword is Reforged (We watch over G&N's heads as the Grandmaster gets zapped) GMWRIT74
	{
		FX_REFORGE2,	// sampleId
		FX_SPOT,			// type
		1,						// delay (or random chance) *
		{							// roomVolList
			{74,14,14},	// {roomNo,leftVol,rightVol}
			{0,0,0},		// NULL-TERMINATOR
		},
	},
	//------------------------
	// 283 The Sword is Reforged (Grandmaster finishes being zapped) GMWRITH
	{
		FX_REFORGE4,	// sampleId
		FX_SPOT,			// type
		1,						// delay (or random chance) *
		{							// roomVolList
			{78,16,16},	// {roomNo,leftVol,rightVol}
			{0,0,0},		// NULL-TERMINATOR
		},
	},
	//------------------------
	// 284 Baphomet Cavern Ambience
	{
		FX_BAPHAMB,		// sampleId
		FX_LOOP,			// type
		0,						// delay (or random chance) *
		{							// roomVolList
			{74,6,8},		// {roomNo,leftVol,rightVol}
			{75,7,8},		// {roomNo,leftVol,rightVol}
			{76,8,8},		// {roomNo,leftVol,rightVol}
			{77,8,8},		// {roomNo,leftVol,rightVol}
			{78,8,8},		// {roomNo,leftVol,rightVol}
			{79,7,8},		// {roomNo,leftVol,rightVol}
			{0,0,0},		// NULL-TERMINATOR
		},
	},
	//------------------------
	// 285 Duane's Happy-Snappy Camera. XDNEPHO3 and XDNEPHO5.
	{
		FX_CAMERA45,	// sampleId
		FX_SPOT,			// type
		30,						// delay (or random chance) *
		{							// roomVolList
			{45,4,4},		// {roomNo,leftVol,rightVol}
			{0,0,0},		// NULL-TERMINATOR
		},
	},
	//------------------------
	// 286 Grand Master strikes the floor with his cane. GMENTER
	{
		FX_STAFF,			// sampleId
		FX_SPOT,			// type
		28,						// delay (or random chance) *
		{							// roomVolList
			{73,8,8},		// {roomNo,leftVol,rightVol}
			{0,0,0},		// NULL-TERMINATOR
		},
	},
	//------------------------
	// 287 George descends ladder in 7: GEOASC07 (Reversed) This used to be handled by effect #46 but it didn't fit at all.
	{
		FX_SEWLADD7,	// sampleId
		FX_SPOT,			// type
		1,						// delay (or random chance) *
		{							// roomVolList
			{7,8,9},		// {roomNo,leftVol,rightVol}
			{0,0,0},		// NULL-TERMINATOR
		},
	},
	//------------------------
	// 288 Sam kicks the recalcitrant Mr. Shiny. DOMKIK
	{
		FX_KIKSHINY,	// sampleId
		FX_SPOT,			// type
		16,						// delay (or random chance) *
		{							// roomVolList
			{33,9,9},		// {roomNo,leftVol,rightVol}
			{0,0,0},		// NULL-TERMINATOR
		},
	},
	//------------------------
	// 289 Gust of wind outside bombed cafe. LVSFLY
	{
		FX_LVSFLY,		// sampleId
		FX_SPOT,			// type
		1,						// delay (or random chance) *
		{							// roomVolList
			{1,16,16},	// {roomNo,leftVol,rightVol}
			{0,0,0},		// NULL-TERMINATOR
		},
	},
	//------------------------
	// 290 Ron's disgusting No.1 Sneeze. Either this or the next effect (randomly chosen) is used for the following animations, RONSNZ & RONSNZ2
	{
		FX_SNEEZE1,		// sampleId
		FX_SPOT,			// type
		11,						// delay (or random chance) *
		{							// roomVolList
			{20,10,16},	// {roomNo,leftVol,rightVol}
			{0,0,0},		// NULL-TERMINATOR
		},
	},
	//------------------------
	// 291 Ron's disgusting No.2 Sneeze. Either this or the previous effect (randomly chosen) is used for the following animations, RONSNZ & RONSNZ2
	{
		FX_SNEEZE2,		// sampleId
		FX_SPOT,			// type
		11,						// delay (or random chance) *
		{							// roomVolList
			{20,10,16},	// {roomNo,leftVol,rightVol}
			{0,0,0},		// NULL-TERMINATOR
		},
	},
	//------------------------
	// 292 Dripping tap in the pub cellar. TAPDRP
	{
		FX_DRIPIRE,		// sampleId
		FX_SPOT,			// type
		9,						// delay (or random chance) *
		{							// roomVolList
			{21,4,4},		// {roomNo,leftVol,rightVol}
			{26,4,4},		// {roomNo,leftVol,rightVol}
			{0,0,0},		// NULL-TERMINATOR
		},
	},
	//------------------------
	// 293 Dripping tap in the pub cellar. TAPDRP
	{
		FX_DRIPIRE2,	// sampleId
		FX_SPOT,			// type
		9,						// delay (or random chance) *
		{							// roomVolList
			{21,4,4},		// {roomNo,leftVol,rightVol}
			{0,0,0},		// NULL-TERMINATOR
		},
	},
	//------------------------
	// 294 Dripping tap in the excavation toilet. (see WATER43 - but it's looped anyway, not triggered with anim)
	{
		FX_TAPDRIP,		// sampleId
		FX_SPOT,			// type
		6,						// delay (or random chance) *
		{							// roomVolList
			{43,8,8},		// {roomNo,leftVol,rightVol}
			{0,0,0},		// NULL-TERMINATOR
		},
	},
	//------------------------
	// 295 George closes the mausoleum window. GEOSPA23
	{
		FX_WINDOW59,	// sampleId
		FX_SPOT,			// type
		24,						// delay (or random chance) *
		{							// roomVolList
			{59,10,8},	// {roomNo,leftVol,rightVol}
			{0,0,0},		// NULL-TERMINATOR
		},
	},
	//------------------------
	// 296 George opens the mausoleum window, the feebleminded loon. GEOSPA23 reversed.
	{
		FX_WINDOW59,	// sampleId
		FX_SPOT,			// type
		14,						// delay (or random chance) *
		{							// roomVolList
			{59,10,8},	// {roomNo,leftVol,rightVol}
			{0,0,0},		// NULL-TERMINATOR
		},
	},
	//------------------------
	// 297	When George & Nico hear chanting from sc73
	{
		FX_CHANT,			// sampleId
		FX_SPOT,			// type
		10,						// delay (or random chance) *
		{							// roomVolList
			{73,2,4},		// {roomNo,leftVol,rightVol}
			{0,0,0},		// NULL-TERMINATOR
		},
	},
	//------------------------
	// 298	EKFIGHT
	{
		FX_FIGHT1,		// sampleId
		FX_SPOT,			// type
		31,						// delay (or random chance) *
		{							// roomVolList
			{74,16,16},	// {roomNo,leftVol,rightVol}
			{0,0,0},		// NULL-TERMINATOR
		},
	},
	//------------------------
	// 299 Small van passes, left to right. CARA9 and CARC9
	{
		FX_LITEVEHR,	// sampleId
		FX_SPOT,			// type
		1,						// delay (or random chance) *
		{							// roomVolList
			{9,16,10},	// {roomNo,leftVol,rightVol}
			{0,0,0},		// NULL-TERMINATOR
		},
	},
	//------------------------
	// 300 Small van passes, right to left to right. CARB9
	{
		FX_LITEVEHL,	// sampleId
		FX_SPOT,			// type
		1,						// delay (or random chance) *
		{							// roomVolList
			{9,16,10},	// {roomNo,leftVol,rightVol}
			{0,0,0},		// NULL-TERMINATOR
		},
	},
	//------------------------
	// 301 Truck passes, left to right. TRUCKA9 and TRUCKB9
	{
		FX_HVYVEHR,		// sampleId
		FX_SPOT,			// type
		1,						// delay (or random chance) *
		{							// roomVolList
			{9,14,10},	// {roomNo,leftVol,rightVol}
			{0,0,0},		// NULL-TERMINATOR
		},
	},
	//------------------------
	// 302 Truck passes, right to left. TRUCKC9
	{
		FX_HVYVEHL,		// sampleId
		FX_SPOT,			// type
		1,						// delay (or random chance) *
		{							// roomVolList
			{9,14,10},	// {roomNo,leftVol,rightVol}
			{0,0,0},		// NULL-TERMINATOR
		},
	},
	//------------------------
	// 303 With anim FIGHT69
	{
		FX_FIGHT69,		// sampleId
		FX_SPOT,			// type
		78,						// delay (or random chance) *
		{							// roomVolList
			{69,12,16},	// {roomNo,leftVol,rightVol}
			{0,0,0},		// NULL-TERMINATOR
		},
	},
	//------------------------
	// 304 With anim GEODIE1 in sc73
	{
		FX_GDROP73,		// sampleId
		FX_SPOT,			// type
		14,						// delay (or random chance) *
		{							// roomVolList
			{73,12,16},	// {roomNo,leftVol,rightVol}
			{0,0,0},		// NULL-TERMINATOR
		},
	},
	//------------------------
	// 305 With anim GEODIE2 in sc73
	{
		FX_GDROP73,		// sampleId
		FX_SPOT,			// type
		21,						// delay (or random chance) *
		{							// roomVolList
			{73,12,16},	// {roomNo,leftVol,rightVol}
			{0,0,0},		// NULL-TERMINATOR
		},
	},
	//------------------------
	// 306 With anim GEODES25
	{
		FX_LADDWN25,	// sampleId
		FX_SPOT,			// type
		0,						// delay (or random chance) *
		{							// roomVolList
			{25,12,8},	// {roomNo,leftVol,rightVol}
			{0,0,0},		// NULL-TERMINATOR
		},
	},
	//------------------------
	// 307 With anim GEOASC25
	{
		FX_LADDUP25,	// sampleId
		FX_SPOT,			// type
		8,						// delay (or random chance) *
		{							// roomVolList
			{25,12,8},	// {roomNo,leftVol,rightVol}
			{0,0,0},		// NULL-TERMINATOR
		},
	},
	//------------------------
	// 308 With anim GKSWORD in sc76
	{
		FX_GKSWORD,		// sampleId
		FX_SPOT,			// type
		9,						// delay (or random chance) *
		{							// roomVolList
			{76,10,10},	// {roomNo,leftVol,rightVol}
			{0,0,0},		// NULL-TERMINATOR
		},
	},
	//------------------------
	// 309 With anim GEO36KNE in sc36
	{
		FX_KEYIN,			// sampleId
		FX_SPOT,			// type
		18,						// delay (or random chance) *
		{							// roomVolList
			{36,14,14},	// {roomNo,leftVol,rightVol}
			{0,0,0},		// NULL-TERMINATOR
		},
	},
	//------------------------
	// 310 With anim GEO36ENT in sc36
	{
		FX_COVDWN,		// sampleId
		FX_SPOT,			// type
		85,						// delay (or random chance) *
		{							// roomVolList
			{36,14,14},	// {roomNo,leftVol,rightVol}
			{0,0,0},		// NULL-TERMINATOR
		},
	},
	//------------------------
	// 311 With anim SECDOR59 in sc59
	{
		FX_SECDOR59,	// sampleId
		FX_SPOT,			// type
		0,						// delay (or random chance) *
		{							// roomVolList
			{59,16,16},	// {roomNo,leftVol,rightVol}
			{0,0,0},		// NULL-TERMINATOR
		},
	},
	//------------------------
};

//--------------------------------------------------------------------------------------
// Continuous & random background sound effects for each location

// NB. There must be a list for each room number, even if location doesn't exist in game

const uint16 Sound::_roomsFixedFx[TOTAL_ROOMS][TOTAL_FX_PER_ROOM] =
{
	{0},											// 0

	// PARIS 1
	{2,3,4,5,0},							// 1
	{2,0},										// 2
	{2,3,4,5,32,0},						// 3
	{2,3,4,5,0},							// 4
	{2,3,4,5,0},							// 5
	{9,11,12,13,44,45,47},		// 6
	{9,11,12,13,44,45,47},		// 7
	{2,3,4,5,0},							// 8

	// PARIS 2
	{54,63,0},								// 9
	{51,52,53,54,63,0},				// 10
	{70,0},										// 11
	{51,52,70,0},							// 12
	{0},											// 13
	{238,0},									// 14
	{82,0},										// 15
	{70,81,82,0},							// 16
	{82,0},										// 17
	{3,4,5,70,0},							// 18

	// IRELAND
	{120,121,122,243,0},			// 19
	{0},											// 20 Violin makes the ambience..
	{120,121,122,243,0},			// 21
	{120,121,122,0},					// 22
	{120,121,122,124,0},			// 23
	{120,121,122,0},					// 24
	{0},											// 25
	{123,243,0},							// 26

	// PARIS 3
	{135,0},									// 27
	{202,0},									// 28
	{202,0},									// 29
	{0},											// 30
	{187,0},									// 31
	{143,145,0},							// 32
	{143,0},									// 33
	{143,0},									// 34
	{0},											// 35

	// PARIS 4
	{198,0},									// 36
	{225,0},									// 37
	{160,0},									// 38
	{0},											// 39
	{198,0},									// 40
	{279,0},									// 41
	{0},											// 42
	{279,0},									// 43
	{0},											// 44 Doesn't exist

	// SYRIA
	{153,0},									// 45
	{70,81,0},								// 46 - PARIS 2
	{153,0},									// 47
	{160,0},									// 48 - PARIS 4
	{0},											// 49
	{153,0},									// 50
	{0},											// 51
	{0},											// 52
	{0},											// 53
	{130,138,0},							// 54
	{0},											// 55

	// SPAIN
	{204,0},									// 56
	{181,182,184,0},					// 57
	{181,182,184,0},					// 58
	{0},											// 59
	{184,0},									// 60
	{185,0},									// 61
	{0},											// 62 Just music

	// NIGHT TRAIN
	{207,0,0},								// 63
	{0},											// 64 Doesn't exist
	{207,0},									// 65
	{207,0},									// 66
	{207,0},									// 67
	{0},											// 68 Disnae exist
	{0},											// 69

	// SCOTLAND + FINALE
	{0},											// 70 Disnae exist
	{199,200,201,242,0},			// 71
	{199,200,201,242,0},			// 72
	{0},											// 73
	{284,0},									// 74
	{284,0},									// 75
	{284,0},									// 76
	{284,0},									// 77
	{284,0},									// 78
	{284,0},									// 79
	{0},											// 80
	{0},											// 81
	{0},											// 82
	{0},											// 83
	{0},											// 84
	{0},											// 85
	{0},											// 86
	{0},											// 87
	{0},											// 88
	{0},											// 89
	{0},											// 90
	{0},											// 91
	{0},											// 92
	{0},											// 93
	{0},											// 94
	{0},											// 95
	{0},											// 96
	{0},											// 97
	{0},											// 98
	{0},											// 99
};

#define ENCODE8(VAL) \
	(uint8)(VAL & 0xFF)
#define ENCODE16(VAL) \
	(uint8)(VAL & 0xFF), (uint8)(VAL >> 8)
#define ENCODE24(VAL) \
	(uint8)(VAL & 0xFF), (uint8)((VAL >> 8) & 0xFF), (uint8)(VAL >> 16)
#define ENCODE32(VAL) \
	(uint8)(VAL & 0xFF), (uint8)((VAL >> 8) & 0xFF), (uint8)((VAL >> 16) & 0xFF), (uint8)(VAL >> 24)

#define LOGIC_CALL_FN(FN_ID, PARAM) \
	opcCallFn,     ENCODE8(FN_ID), ENCODE8(PARAM)
#define LOGIC_CALL_FN_LONG(FN_ID, PARAM1, PARAM2, PARAM3) \
	opcCallFnLong, ENCODE8(FN_ID), ENCODE32(PARAM1), ENCODE32(PARAM2), ENCODE32(PARAM3)
#define LOGIC_SET_VAR8(VAR_ID, VAL) \
	opcSetVar8,  ENCODE16(VAR_ID), ENCODE8(VAL)
#define LOGIC_SET_VAR16(VAR_ID, VAL) \
	opcSetVar16, ENCODE16(VAR_ID), ENCODE16(VAL)
#define LOGIC_SET_VAR32(VAR_ID, VAL) \
	opcSetVar32, ENCODE16(VAR_ID), ENCODE32(VAL)
#define GEORGE_POS(POS_X, POS_Y, DIR, PLACE) \
	opcGeorge,   ENCODE16(POS_X),  ENCODE16(POS_Y), DIR, ENCODE24(PLACE)

#define INIT_SEQ_END \
	opcSeqEnd
#define RUN_START_SCRIPT(SCR_ID) \
	opcRunStart, ENCODE8(SCR_ID)
#define RUN_HELPER_SCRIPT(SCR_ID) \
	opcRunHelper, ENCODE8(SCR_ID)

const uint8 g_startPos0[] = {				// Intro with sequence
	LOGIC_CALL_FN(opcPlaySequence, 4),
	GEORGE_POS(481, 413, DOWN, FLOOR_1),
	INIT_SEQ_END
};

const uint8 g_startPos1[] = {				// Intro without sequence
	GEORGE_POS(481, 413, DOWN, FLOOR_1),
	INIT_SEQ_END
};

const uint8 g_startPos2[] = {				// blind alley
	GEORGE_POS(480, 388, DOWN_LEFT, FLOOR_2),
	LOGIC_CALL_FN(opcAddObject, LIFTING_KEYS),
	LOGIC_CALL_FN(opcAddObject, ROSSO_CARD),
	INIT_SEQ_END
};

const uint8 g_startPos3[] = {				// cafe
	GEORGE_POS(660, 368, DOWN_LEFT, FLOOR_3),
	INIT_SEQ_END
};

const uint8 g_startPos4[] = {				// ready to use the phone
	GEORGE_POS(463, 391, DOWN, FLOOR_4),
	LOGIC_SET_VAR8(MOUE_TEXT,         1),
	LOGIC_SET_VAR8(MOUE_NICO_FLAG,    1),
	LOGIC_SET_VAR8(PARIS_FLAG,        5),
	LOGIC_SET_VAR8(NICO_PHONE_FLAG,   1),
	LOGIC_SET_VAR8(TAILOR_PHONE_FLAG, 1),
	LOGIC_SET_VAR8(WORKMAN_GONE_FLAG, 1),
	LOGIC_SET_VAR8(ALBERT_INFO_FLAG,  1),
	LOGIC_SET_VAR8(SEEN_SEWERS_FLAG,  1),
	// item stuff missing
	INIT_SEQ_END
};

const uint8 g_startPos5[] = {				// court yard
	GEORGE_POS(400, 400, DOWN_LEFT, FLOOR_5),
	INIT_SEQ_END
};

const uint8 g_startPos7[] = {				// sewer two
	GEORGE_POS(520, 310, DOWN_LEFT, FLOOR_7),
	INIT_SEQ_END
};

const uint8 g_startPos8[] = {				// cafe repaired
	GEORGE_POS(481, 413, DOWN, FLOOR_8),
	INIT_SEQ_END
};

const uint8 g_startPos11[] = {				// costumier
	GEORGE_POS(264, 436, DOWN_RIGHT, FLOOR_11),
	LOGIC_CALL_FN(opcAddObject, TISSUE),
	LOGIC_CALL_FN(opcAddObject, PHOTOGRAPH),
	INIT_SEQ_END
};

const uint8 g_startPos12[] = {				// hotel street
	GEORGE_POS(730, 460, LEFT, FLOOR_12),
	LOGIC_SET_VAR8(NICO_ADDRESS_FLAG,     1),
	LOGIC_SET_VAR8(NICO_PHONE_FLAG,       1),
	LOGIC_SET_VAR8(COSTUMES_ADDRESS_FLAG, 1),
	LOGIC_SET_VAR8(HOTEL_ADDRESS_FLAG,    1),
	LOGIC_SET_VAR8(AEROPORT_ADDRESS_FLAG, 1),
	LOGIC_SET_VAR8(TAILOR_PHONE_FLAG,     1),
	INIT_SEQ_END
};

const uint8 g_startPos14[] = {				// hotel corridor
	GEORGE_POS(528, 484, UP, FLOOR_14),
	LOGIC_CALL_FN(opcAddObject, HOTEL_KEY),
	LOGIC_CALL_FN(opcAddObject, MANUSCRIPT),
	INIT_SEQ_END
};

const uint8 g_startPos17[] = {				// hotel assassin
	GEORGE_POS(714, 484, LEFT, FLOOR_17),
	INIT_SEQ_END
};

const uint8 g_startPos18[] = {				// gendarmerie
	GEORGE_POS(446, 408, DOWN_LEFT, FLOOR_18),
	LOGIC_SET_VAR8(PARIS_FLAG, 5),
	INIT_SEQ_END
};

const uint8 g_startPos19[] = {				// ireland street
	GEORGE_POS(256, 966, UP_RIGHT, FLOOR_19),
	RUN_HELPER_SCRIPT(HELP_IRELAND)
};

const uint8 g_startPos20[] = {				// macdevitts
	GEORGE_POS(194, 417, DOWN_RIGHT, FLOOR_20),
	LOGIC_SET_VAR8(FARMER_MOVED_FLAG,  1),
	LOGIC_SET_VAR8(FARMER_SEAN_FLAG,   5),
	LOGIC_SET_VAR8(PUB_FLAP_FLAG,      1),
	LOGIC_SET_VAR8(PUB_TRAP_DOOR,      2),
	LOGIC_SET_VAR8(KNOWS_PEAGRAM_FLAG, 1),
	RUN_HELPER_SCRIPT(HELP_IRELAND)
};

const uint8 g_startPos21[] = {				// pub cellar
	GEORGE_POS(291, 444, DOWN_RIGHT, FLOOR_21),
	LOGIC_CALL_FN(opcAddObject, BEER_TOWEL),
	LOGIC_SET_VAR8(FARMER_MOVED_FLAG,       1),
	LOGIC_SET_VAR8(FLEECY_STUCK,            1),
	LOGIC_SET_VAR8(LIFTING_KEYS_IN_HOLE_23, 1),
	RUN_HELPER_SCRIPT(HELP_IRELAND)
};

const uint8 g_startPos22[] = {				// castle gate
	GEORGE_POS(547, 500, UP_LEFT, FLOOR_22),
	LOGIC_SET_VAR8(IRELAND_FLAG, 4),
	RUN_HELPER_SCRIPT(HELP_IRELAND)
};

const uint8 g_startPos23[] = {				// castle hay top
	GEORGE_POS(535, 510, UP, FLOOR_23),
	RUN_HELPER_SCRIPT(HELP_IRELAND)
};

const uint8 g_startPos24[] = {				// castle yard
	GEORGE_POS(815, 446, DOWN_LEFT, FLOOR_24),
	RUN_HELPER_SCRIPT(HELP_IRELAND)
};

const uint8 g_startPos25[] = {				// castle dig
	GEORGE_POS(369, 492, LEFT, FLOOR_25),
	LOGIC_CALL_FN(opcAddObject, BEER_TOWEL),
	LOGIC_SET_VAR8(BEER_TOWEL_BEEN_WET, 1),
	LOGIC_SET_VAR16(WET_BEER_TOWEL_TIMER, 1000),
	RUN_HELPER_SCRIPT(HELP_IRELAND)
};

const uint8 g_startPos26[] = {				// cellar dark
	GEORGE_POS(291, 444, DOWN_RIGHT, FLOOR_26),
	RUN_HELPER_SCRIPT(HELP_IRELAND)
};

const uint8 g_startPos27[] = {				// museum street
	GEORGE_POS(300, 510, UP_RIGHT, FLOOR_27),
	LOGIC_SET_VAR8(PARIS_FLAG,                  12),
	LOGIC_SET_VAR8(MANUSCRIPT_ON_TABLE_10_FLAG,  1),
	INIT_SEQ_END
};

const uint8 g_startPos31[] = {				// hospital street
	GEORGE_POS(400, 500, UP_RIGHT, FLOOR_31),
	LOGIC_SET_VAR8(PARIS_FLAG, 11),
	LOGIC_CALL_FN(opcAddObject, PHOTOGRAPH),
	LOGIC_CALL_FN(opcAddObject, LAB_PASS),
	INIT_SEQ_END
};

const uint8 g_startPos32[] = {				// hospital desk (after we've found out where Marquet is)
	GEORGE_POS(405, 446, UP_RIGHT, FLOOR_32),
	LOGIC_SET_VAR8(GOT_BENOIR_FLAG, 1),
	LOGIC_CALL_FN(opcAddObject, PHOTOGRAPH),
	LOGIC_CALL_FN(opcAddObject, LAB_PASS),
	RUN_HELPER_SCRIPT(HELP_WHITECOAT)
};

const uint8 g_startPos35[] = {				// hospital jacques
	GEORGE_POS(640, 500, LEFT, FLOOR_35),
	LOGIC_SET_VAR8(DOOR_34_OPEN,	 1),
	LOGIC_SET_VAR8(GOT_BENOIR_FLAG,  2),
	LOGIC_SET_VAR8(HOS_POS_FLAG,    26),
	LOGIC_SET_VAR8(BENOIR_FLAG,     24),
	RUN_HELPER_SCRIPT(HELP_WHITECOAT)
};

const uint8 g_startPos36[] = {				// montfaucon
	GEORGE_POS(300, 480, RIGHT, FLOOR_36),
	LOGIC_CALL_FN(opcAddObject, LENS),
	LOGIC_CALL_FN(opcAddObject, RED_NOSE),
	LOGIC_CALL_FN(opcAddObject, LIFTING_KEYS),
	LOGIC_SET_VAR8(MONTFAUCON_CONTROL_FLAG, 1),
	INIT_SEQ_END
};

const uint8 g_startPos37[] = {				// catacomb sewer
	GEORGE_POS(592, 386, RIGHT, FLOOR_37),
	LOGIC_CALL_FN(opcAddObject, LIFTING_KEYS),
	LOGIC_CALL_FN(opcAddObject, TRIPOD),
	LOGIC_CALL_FN(opcAddObject, GEM),
	INIT_SEQ_END
};

const uint8 g_startPos38[] = {				// catacomb room
	GEORGE_POS(200, 390, RIGHT, FLOOR_38),
	LOGIC_CALL_FN(opcAddObject, TRIPOD),
	LOGIC_CALL_FN(opcAddObject, GEM),
	INIT_SEQ_END
};

const uint8 g_startPos39[] = {				// catacomb meeting
	GEORGE_POS(636, 413, DOWN_LEFT, FLOOR_39),
	LOGIC_SET_VAR8(MEETING_FLAG, 3),	// meeting finished
	LOGIC_CALL_FN(opcAddObject, TRIPOD),
	LOGIC_CALL_FN(opcAddObject, GEM),
	INIT_SEQ_END
};

const uint8 g_startPos40[] = {				// excavation exterior
	GEORGE_POS(648, 492, LEFT, FLOOR_40),
	LOGIC_SET_VAR8(NICO_PHONE_FLAG,  1),
	LOGIC_SET_VAR8(PARIS_FLAG,      16),
	LOGIC_CALL_FN(opcAddObject, PLASTER),
	LOGIC_CALL_FN(opcAddObject, POLISHED_CHALICE),
	INIT_SEQ_END
};

const uint8 g_startPos45[] = {				// syria stall
	GEORGE_POS(410, 490, DOWN_RIGHT, FLOOR_45),
	RUN_HELPER_SCRIPT(HELP_SYRIA)
};

const uint8 g_startPos47[] = {				// syria carpet
	GEORGE_POS(225, 775, RIGHT, FLOOR_47),
	RUN_HELPER_SCRIPT(HELP_SYRIA)
};

const uint8 g_startPos48[] = {				// templar church
	GEORGE_POS(315, 392, DOWN, FLOOR_48),
	LOGIC_SET_VAR8(CHALICE_FLAG, 2),
	LOGIC_SET_VAR8(NEJO_TEXT,    1),
	LOGIC_CALL_FN(opcAddObject, CHALICE),
	LOGIC_CALL_FN(opcAddObject, LENS),
	INIT_SEQ_END
};

const uint8 g_startPos49[] = {				// syria club
	GEORGE_POS(438, 400, DOWN_RIGHT, FLOOR_49),
	LOGIC_CALL_FN(opcAddObject, TOILET_BRUSH),
	RUN_HELPER_SCRIPT(HELP_SYRIA)
};

const uint8 g_startPos50[] = {				// syria toilet
	GEORGE_POS(313, 440, DOWN_RIGHT, FLOOR_50),
	LOGIC_CALL_FN(opcAddObject, TOILET_KEY),
	RUN_HELPER_SCRIPT(HELP_SYRIA)
};

const uint8 g_startPos53[] = {				// bull's head pan
	LOGIC_SET_VAR32(CHANGE_PLACE, FLOOR_53),
	LOGIC_CALL_FN(opcAddObject, TOWEL_CUT),
	RUN_HELPER_SCRIPT(HELP_SYRIA)
};

const uint8 g_startPos54[] = {				// bull's head
	GEORGE_POS(680, 425, DOWN_LEFT, FLOOR_54),
	LOGIC_CALL_FN(opcAddObject, TOWEL_CUT),
	RUN_HELPER_SCRIPT(HELP_SYRIA)
};

const uint8 g_startPos55[] = {				// bull secret
	GEORGE_POS(825, 373, DOWN_LEFT, FLOOR_55),
	RUN_HELPER_SCRIPT(HELP_SYRIA)
};

const uint8 g_startPos56[] = {				// contess' room
	GEORGE_POS(572, 443, LEFT, FLOOR_56),
	RUN_HELPER_SCRIPT(HELP_SPAIN)
};

const uint8 g_startPos57[] = {				// Spain drive
	GEORGE_POS(1630, 460, DOWN_LEFT, FLOOR_57),
	RUN_HELPER_SCRIPT(HELP_SPAIN)
};

const uint8 g_startPos58[] = {				// Mausoleum Exterior
	GEORGE_POS(SC58_PATH_X, SC58_PATH_Y, UP_RIGHT, FLOOR_58),
	RUN_HELPER_SCRIPT(HELP_SPAIN)
};

const uint8 g_startPos59[] = {				// Mausoleum interior
	GEORGE_POS(750, 455, LEFT, FLOOR_59),
	RUN_HELPER_SCRIPT(HELP_SPAIN)
};

const uint8 g_startPos60[] = {				// Spain reception
	GEORGE_POS(750, 475, DOWN_LEFT, FLOOR_60),
	RUN_HELPER_SCRIPT(HELP_SPAIN)
};

const uint8 g_startPos61[] = {				// Spain well
	GEORGE_POS(400, 345, DOWN, LEFT_FLOOR_61),
	LOGIC_CALL_FN(opcAddObject, STONE_KEY),
	LOGIC_CALL_FN(opcAddObject, MIRROR),
	RUN_HELPER_SCRIPT(HELP_SPAIN)
};

const uint8 g_startPos62[] = {				// chess puzzle
	LOGIC_SET_VAR32(CHANGE_PLACE, FLOOR_62),
	LOGIC_SET_VAR8(TOP_MENU_DISABLED, 1),
	LOGIC_SET_VAR8(GEORGE_ALLOWED_REST_ANIMS, 0),
	LOGIC_CALL_FN_LONG(opcNoSprite, PLAYER, 0, 0),
	RUN_HELPER_SCRIPT(HELP_SPAIN)
};

const uint8 g_startPos63[] = {				// train one
	GEORGE_POS(710, 450, LEFT, FLOOR_63),
	LOGIC_SET_VAR8(DOOR_SC65_FLAG, 2),
	LOGIC_SET_VAR8(DOOR_ONE_63_OPEN, 0),
	LOGIC_SET_VAR8(DOOR_65_OPEN, 1),
	LOGIC_SET_VAR8(VAIL_TEXT, 1),
	RUN_HELPER_SCRIPT(HELP_NIGHTTRAIN)
};

const uint8 g_startPos65[] = {				// compt one
	GEORGE_POS(460, 430, DOWN, FLOOR_65),
	RUN_HELPER_SCRIPT(HELP_NIGHTTRAIN)
};

const uint8 g_startPos66[] = {				// compt two
	GEORGE_POS(460, 430, DOWN, FLOOR_66),
	RUN_HELPER_SCRIPT(HELP_NIGHTTRAIN)
};

const uint8 g_startPos67[] = {				// compt three
	GEORGE_POS(460, 430, DOWN, FLOOR_67),
	RUN_HELPER_SCRIPT(HELP_NIGHTTRAIN)
};

const uint8 g_startPos69[] = {				// train_guard
	GEORGE_POS(310, 430, DOWN, FLOOR_69),
	RUN_HELPER_SCRIPT(HELP_NIGHTTRAIN)
};

const uint8 g_startPos71[] = {				// churchyard
	GEORGE_POS(1638, 444, LEFT, RIGHT_FLOOR_71),
	LOGIC_SET_VAR8(NICO_SCOT_SCREEN, 71),
	LOGIC_SET_VAR8(NICO_POSITION_71, 1),
	RUN_HELPER_SCRIPT(HELP_SCOTLAND)
};

const uint8 g_startPos72[] = {				// church tower
	GEORGE_POS(150, 503, RIGHT, FLOOR_72),
	LOGIC_SET_VAR8(NICO_SCOT_SCREEN, 72),
	RUN_HELPER_SCRIPT(HELP_SCOTLAND)
};

const uint8 g_startPos73[] = {				// crypt
	GEORGE_POS(250, 390, DOWN_RIGHT, FLOOR_73),
	LOGIC_SET_VAR8(NICO_SCOT_SCREEN, 73),
	LOGIC_SET_VAR8(NICO_POSITION_73, 1)
};

const uint8 g_startPos80[] = {				// Paris map
	GEORGE_POS(645, 160, DOWN, FLOOR_80),
	LOGIC_SET_VAR8(PARIS_FLAG, 3),
	LOGIC_SET_VAR8(NICO_CLOWN_FLAG, 3),
	LOGIC_SET_VAR8(NICO_DOOR_FLAG, 2),

	LOGIC_CALL_FN(opcAddObject, RED_NOSE),
	LOGIC_CALL_FN(opcAddObject, PHOTOGRAPH),
	LOGIC_CALL_FN(opcAddObject, PLASTER),
	LOGIC_CALL_FN(opcAddObject, LAB_PASS),

	LOGIC_SET_VAR8(MANUSCRIPT_FLAG,       1),
	LOGIC_SET_VAR8(NICO_ADDRESS_FLAG,     1),
	LOGIC_SET_VAR8(NICO_PHONE_FLAG,       1),
	LOGIC_SET_VAR8(COSTUMES_ADDRESS_FLAG, 1),
	LOGIC_SET_VAR8(HOTEL_ADDRESS_FLAG,    1),
	LOGIC_SET_VAR8(MUSEUM_ADDRESS_FLAG,   1),
	LOGIC_SET_VAR8(HOSPITAL_ADDRESS_FLAG, 1),
	LOGIC_SET_VAR8(MONTFACN_ADDRESS_FLAG, 1),
	LOGIC_SET_VAR8(AEROPORT_ADDRESS_FLAG, 1),
	LOGIC_SET_VAR8(NERVAL_ADDRESS_FLAG,   1),

	LOGIC_SET_VAR8(IRELAND_MAP_FLAG,      1),
	LOGIC_SET_VAR8(SPAIN_MAP_FLAG,        1),
	LOGIC_SET_VAR8(SYRIA_FLAG,            2),

	LOGIC_SET_VAR8(TAILOR_PHONE_FLAG,     1),
	INIT_SEQ_END
};

const uint8 g_genIreland[] = {
	LOGIC_SET_VAR8(PARIS_FLAG, 9),
	LOGIC_CALL_FN(opcAddObject, RED_NOSE),
	LOGIC_CALL_FN(opcAddObject, PHOTOGRAPH),
	LOGIC_CALL_FN(opcAddObject, LAB_PASS),
	LOGIC_CALL_FN(opcAddObject, LIFTING_KEYS),
	LOGIC_CALL_FN(opcAddObject, MATCHBOOK),
	LOGIC_CALL_FN(opcAddObject, BUZZER),
	LOGIC_CALL_FN(opcAddObject, TISSUE),
	INIT_SEQ_END
};

const uint8 g_genSyria[] = {
	LOGIC_SET_VAR8(PARIS_FLAG, 1),
	LOGIC_CALL_FN(opcAddObject, BALL),
	LOGIC_CALL_FN(opcAddObject, RED_NOSE),
	LOGIC_CALL_FN(opcAddObject, PHOTOGRAPH),
	LOGIC_CALL_FN(opcAddObject, LIFTING_KEYS),
	LOGIC_CALL_FN(opcAddObject, MATCHBOOK),
	LOGIC_CALL_FN(opcAddObject, BUZZER),
	LOGIC_CALL_FN(opcAddObject, TISSUE),
	LOGIC_SET_VAR8(CHANGE_STANCE, STAND),
	INIT_SEQ_END
};

const uint8 g_genSpain[] = {
	LOGIC_SET_VAR8(PARIS_FLAG, 1),
	LOGIC_SET_VAR8(SPAIN_VISIT, 1),		// default to 1st spain visit, may get overwritten later
	LOGIC_CALL_FN(opcAddObject, RED_NOSE),
	LOGIC_CALL_FN(opcAddObject, PHOTOGRAPH),
	LOGIC_CALL_FN(opcAddObject, LAB_PASS),
	LOGIC_CALL_FN(opcAddObject, LIFTING_KEYS),
	LOGIC_CALL_FN(opcAddObject, BUZZER),
	LOGIC_CALL_FN(opcAddObject, TISSUE),
	LOGIC_CALL_FN(opcAddObject, BALL),
	LOGIC_CALL_FN(opcAddObject, MATCHBOOK),
	LOGIC_CALL_FN(opcAddObject, PRESSURE_GAUGE),
	INIT_SEQ_END
};

const uint8 g_genSpain2[] = {	// 2nd spain visit
	LOGIC_SET_VAR8(SPAIN_VISIT, 2),
	LOGIC_CALL_FN(opcRemoveObject, PRESSURE_GAUGE),
	LOGIC_CALL_FN(opcAddObject, POLISHED_CHALICE),
	INIT_SEQ_END
};

const uint8 g_genNightTrain[] = {
	LOGIC_SET_VAR8(PARIS_FLAG, 18),
	INIT_SEQ_END
};

const uint8 g_genScotland[] = {
	LOGIC_SET_VAR8(PARIS_FLAG, 1),
	LOGIC_CALL_FN(opcAddObject, RED_NOSE),
	LOGIC_CALL_FN(opcAddObject, PHOTOGRAPH),
	LOGIC_CALL_FN(opcAddObject, LAB_PASS),
	LOGIC_CALL_FN(opcAddObject, LIFTING_KEYS),
	LOGIC_CALL_FN(opcAddObject, BUZZER),
	INIT_SEQ_END
};

const uint8 g_genWhiteCoat[] = {
	LOGIC_SET_VAR8(PARIS_FLAG,       11),
	LOGIC_SET_VAR8(EVA_TEXT,          1),
	LOGIC_SET_VAR8(EVA_MARQUET_FLAG,  2),
	LOGIC_SET_VAR8(EVA_NURSE_FLAG,    4),
	LOGIC_SET_VAR8(FOUND_WARD_FLAG,   1),
	LOGIC_SET_VAR8(CONSULTANT_HERE,   1),

	LOGIC_CALL_FN_LONG(opcMegaSet, PLAYER, GEORGE_WLK, MEGA_WHITE),

	LOGIC_SET_VAR32(GEORGE_CDT_FLAG, WHT_TLK_TABLE),
	LOGIC_SET_VAR8(GEORGE_TALK_FLAG,          0),
	LOGIC_SET_VAR8(WHITE_COAT_FLAG,           1),
	LOGIC_SET_VAR8(GEORGE_ALLOWED_REST_ANIMS, 0),
	INIT_SEQ_END
};

const uint8 *const Logic::_startData[] = {
	g_startPos0,
	g_startPos1,
	g_startPos2,
	g_startPos3,
	g_startPos4,
	g_startPos5,
	NULL, //g_startPos6,
	g_startPos7,
	g_startPos8,
	NULL, //g_startPos9,
	NULL, //g_startPos10,
	g_startPos11,
	g_startPos12,
	NULL, //g_startPos13,
	g_startPos14,
	NULL, //g_startPos15,
	NULL, //g_startPos16,
	g_startPos17,
	g_startPos18,
	g_startPos19,
	g_startPos20,
	g_startPos21,
	g_startPos22,
	g_startPos23,
	g_startPos24,
	g_startPos25,
	g_startPos26,
	g_startPos27,
	NULL, //g_startPos28,
	NULL, //g_startPos29,
	NULL, //g_startPos30,
	g_startPos31,
	g_startPos32,
	NULL, //g_startPos33,
	NULL, //g_startPos34,
	g_startPos35,
	g_startPos36,
	g_startPos37,
	g_startPos38,
	g_startPos39,
	g_startPos40,
	NULL, //g_startPos41,
	NULL, //g_startPos42,
	NULL, //g_startPos43,
	NULL, //g_startPos44,
	g_startPos45,
	NULL, //g_startPos46,
	g_startPos47,
	g_startPos48,
	g_startPos49,
	g_startPos50,
	NULL, //g_startPos51,
	NULL, //g_startPos52,
	g_startPos53,
	g_startPos54,
	g_startPos55,
	g_startPos56,
	g_startPos57,
	g_startPos58,
	g_startPos59,
	g_startPos60,
	g_startPos61,
	g_startPos62,
	g_startPos63,
	NULL, //g_startPos64,
	g_startPos65,
	g_startPos66,
	g_startPos67,
	NULL, //g_startPos68,
	g_startPos69,
	NULL, //g_startPos70,
	g_startPos71,
	g_startPos72,
	g_startPos73,
	NULL, //g_startPos74,
	NULL, //g_startPos75,
	NULL, //g_startPos76
	NULL, //g_startPos77
	NULL, //g_startPos78
	NULL, //g_startPos79
	g_startPos80
};

const uint8 *const Logic::_helperData[] = {
	g_genIreland,
	g_genSyria,
	g_genSpain,
	g_genNightTrain,
	g_genScotland,
	g_genWhiteCoat,
	g_genSpain2
};

} // End of namespace Sword1
