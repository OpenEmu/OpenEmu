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

#ifndef AGI_PREAGI_MICKEY_H
#define AGI_PREAGI_MICKEY_H

namespace Agi {

#define MSA_SAVEGAME_VERSION			2

// strings
#define IDS_MSA_PATH_DAT	"dat/%s"
#define IDS_MSA_PATH_OBJ	"obj/%s.ooo"
#define IDS_MSA_PATH_PIC	"%d.pic"
#define IDS_MSA_PATH_LOGO	"logos.bcg"

#define IDS_MSA_INVENTORY	"MICKEY IS CARRYING THE FOLLOWING:"
#define IDS_MSA_CRYSTALS	"%s CRYSTALS"

const char IDS_MSA_CRYSTAL_NO[][3] = {
	"NO", " 1", " 2", " 3", " 4", " 5", " 6", " 7", " 8", " 9"
};
const char IDS_MSA_TEMP_C[][5] = {
	" 20 ", " 480", "-200", " 430", "-185", "-230", "-130", "-150", "-215"
};
const char IDS_MSA_TEMP_F[][5] = {
	" 68 ", " 897", "-328", " 807", "-301", "-382", "-202", "-238", "-355"
};
const char IDS_MSA_PLANETS[][10] = {
	"EARTH.  ", "VENUS.  ", "TRITON. ", "MERCURY.", "TITAN.  ",
	"PLUTO.  ", "IO.     ", "MARS.   ", "OBERON. "
};

const char IDS_MSA_ERRORS[][40] = {
	"THAT CANNOT BE UNDERSTOOD",
	"TRY GOING THERE INSTEAD",
	"THAT CAN'T BE DONE",
	"MICKEY WOULDN'T WANT TO DO THAT!",
	"WHICH DIRECTION?",
	"THAT DOESN'T MAKE SENSE!",
	"MICKEY WOULDN'T WANT TO DO THAT!"
};

// patch Mickey.exe offset 0x21E to value 0x01 to enable debug mode

const char IDS_MSA_INSERT_DISK[][40] = {
	"Please insert disk 1 and press any key", "Please insert disk 2 and press any key"
};

// max values

#define IDI_MSA_MAX_PLANET				9
#define IDI_MSA_MAX_DAT					10
#define IDI_MSA_MAX_PIC_ROOM			224
#define IDI_MSA_MAX_ROOM				160

#define IDI_MSA_MAX_BUTTON				6
#define IDI_MSA_MAX_ITEM				11

#define IDI_MSA_ANIM_DELAY				25

#define IDI_MSA_LEN_STORY				1372

// rows

#define IDI_MSA_ROW_MENU_0				20
#define IDI_MSA_ROW_MENU_1				21
#define IDI_MSA_ROW_INV_TITLE			2
#define IDI_MSA_ROW_INV_CRYSTALS		4
#define IDI_MSA_ROW_INV_ITEMS			5
#define IDI_MSA_ROW_TEMPERATURE			21
#define IDI_MSA_ROW_PLANET				22
#define IDI_MSA_ROW_INSERT_DISK			23

#define IDI_MSA_COL_INV_TITLE			4
#define IDI_MSA_COL_INV_ITEMS			15
#define IDI_MSA_COL_PLANET				28
#define IDI_MSA_COL_INSERT_DISK			1

// screen

#define IDI_MSA_PIC_WIDTH	140
#define IDI_MSA_PIC_HEIGHT	159

// pictures

#define IDI_MSA_PIC_EARTH_TIRE_SWING	1
#define IDI_MSA_PIC_EARTH_TIRE_SWING_1	200		// rope taken, swing on ground
#define IDI_MSA_PIC_EARTH_DOGHOUSE		2
#define IDI_MSA_PIC_EARTH_IN_DOGHOUSE	154
#define IDI_MSA_PIC_EARTH_TREE			3
#define IDI_MSA_PIC_EARTH_GARDEN		4
#define IDI_MSA_PIC_EARTH_FRONT_HOUSE	5
#define IDI_MSA_PIC_EARTH_HAMMOCK		6
#define IDI_MSA_PIC_EARTH_BUTTERFLY		7
#define IDI_MSA_PIC_EARTH_MAILBOX		8
#define IDI_MSA_PIC_EARTH_ROAD_0		9
#define IDI_MSA_PIC_EARTH_ROAD_1		10
#define IDI_MSA_PIC_EARTH_ROAD_2		11
#define IDI_MSA_PIC_EARTH_ROAD_3		12
#define IDI_MSA_PIC_EARTH_ROAD_4		13		// starting room
#define IDI_MSA_PIC_EARTH_ROAD_5		14
#define IDI_MSA_PIC_EARTH_ROAD_6		15
#define IDI_MSA_PIC_EARTH_ROAD_7		18
#define IDI_MSA_PIC_EARTH_UNDER_TREE	16
#define IDI_MSA_PIC_EARTH_UP_IN_TREE	155		// CRYSTAL
#define IDI_MSA_PIC_EARTH_SHIP			17
#define IDI_MSA_PIC_EARTH_LIVING_ROOM	19
#define IDI_MSA_PIC_EARTH_KITCHEN		20
#define IDI_MSA_PIC_EARTH_KITCHEN_1		159		// cupboard open
#define IDI_MSA_PIC_EARTH_GARAGE		21
#define IDI_MSA_PIC_EARTH_GARAGE_1		160		// cabinet open
#define IDI_MSA_PIC_EARTH_BEDROOM		22
#define IDI_MSA_PIC_EARTH_BEDROOM_1		161		// closet open
#define IDI_MSA_PIC_EARTH_BATHROOM		23		// WEIGH MICKEY
#define IDI_MSA_PIC_EARTH_SHIP_LEAVING	24
#define IDI_MSA_PIC_EARTH_MINNIE		25

#define IDI_MSA_PIC_SHIP_AIRLOCK		25
#define IDI_MSA_PIC_SHIP_AIRLOCK_0		201		// door closed
#define IDI_MSA_PIC_SHIP_AIRLOCK_1		202		// door open
#define IDI_MSA_PIC_SHIP_AIRLOCK_2		203		// door closed, spacesuits on
#define IDI_MSA_PIC_SHIP_AIRLOCK_3		204		// door open, spacesuits on
#define IDI_MSA_PIC_SHIP_BEDROOM		29
#define IDI_MSA_PIC_SHIP_CONTROLS		26
#define IDI_MSA_PIC_SHIP_CORRIDOR		27
#define IDI_MSA_PIC_SHIP_KITCHEN		28
#define IDI_MSA_PIC_SHIP_KITCHEN_1		172		// cabinet open

#define IDI_MSA_PIC_SHIP_VENUS			146
#define IDI_MSA_PIC_SHIP_NEPTUNE		147
#define IDI_MSA_PIC_SHIP_MERCURY		148
#define IDI_MSA_PIC_SHIP_SATURN			149
#define IDI_MSA_PIC_SHIP_PLUTO			150
#define IDI_MSA_PIC_SHIP_JUPITER		151
#define IDI_MSA_PIC_SHIP_MARS			152
#define IDI_MSA_PIC_SHIP_URANUS			153

#define IDI_MSA_PIC_VENUS_0				30
#define IDI_MSA_PIC_VENUS_1				31
#define IDI_MSA_PIC_VENUS_2				32
#define IDI_MSA_PIC_VENUS_3				34
#define IDI_MSA_PIC_VENUS_4				36
#define IDI_MSA_PIC_VENUS_5				38
#define IDI_MSA_PIC_VENUS_CHASM			35
#define IDI_MSA_PIC_VENUS_CHASM_1		183		// rope lowered
#define IDI_MSA_PIC_VENUS_PROBE			39		// CRYSTAL, USE WRENCH
#define IDI_MSA_PIC_VENUS_PROBE_1		184		// hatch open
#define IDI_MSA_PIC_VENUS_SHIP			33
#define IDI_MSA_PIC_VENUS_WEIGH			37		// WEIGH MICKEY

#define IDI_MSA_PIC_NEPTUNE_0			40
#define IDI_MSA_PIC_NEPTUNE_1			42
#define IDI_MSA_PIC_NEPTUNE_2			43
#define IDI_MSA_PIC_NEPTUNE_3			44
#define IDI_MSA_PIC_NEPTUNE_4			45
#define IDI_MSA_PIC_NEPTUNE_5			48
#define IDI_MSA_PIC_NEPTUNE_6			50
#define IDI_MSA_PIC_NEPTUNE_7			52
#define IDI_MSA_PIC_NEPTUNE_8			53
#define IDI_MSA_PIC_NEPTUNE_9			54
#define IDI_MSA_PIC_NEPTUNE_10			55
#define IDI_MSA_PIC_NEPTUNE_11			56
#define IDI_MSA_PIC_NEPTUNE_BABIES		61
#define IDI_MSA_PIC_NEPTUNE_CASTLE_0	46
#define IDI_MSA_PIC_NEPTUNE_CASTLE_1	51
#define IDI_MSA_PIC_NEPTUNE_CASTLE_2	57
#define IDI_MSA_PIC_NEPTUNE_CASTLE_3	58
#define IDI_MSA_PIC_NEPTUNE_CASTLE_4	59
#define IDI_MSA_PIC_NEPTUNE_CASTLE_5	60
#define IDI_MSA_PIC_NEPTUNE_CASTLE_6	66
#define IDI_MSA_PIC_NEPTUNE_CASTLE_7	67
#define IDI_MSA_PIC_NEPTUNE_CASTLE_8	68
#define IDI_MSA_PIC_NEPTUNE_EATING_AREA	62
#define IDI_MSA_PIC_NEPTUNE_ENTRANCE	47
#define IDI_MSA_PIC_NEPTUNE_ENTRANCE_1	185		// entrance open
#define IDI_MSA_PIC_NEPTUNE_ENTRYWAY	63
#define IDI_MSA_PIC_NEPTUNE_GUARD		69
#define IDI_MSA_PIC_NEPTUNE_LEADER		64		// CRYSTAL, GIVE SCARF
#define IDI_MSA_PIC_NEPTUNE_SHIP		49
#define IDI_MSA_PIC_NEPTUNE_SLEEP_AREA	65
#define IDI_MSA_PIC_NEPTUNE_WEIGH		41

#define IDI_MSA_PIC_MERCURY_0			71
#define IDI_MSA_PIC_MERCURY_1			73
#define IDI_MSA_PIC_MERCURY_2			75
#define IDI_MSA_PIC_MERCURY_3			77
#define IDI_MSA_PIC_MERCURY_4			80
#define IDI_MSA_PIC_MERCURY_ALIEN_0		72		// CRYSTAL, GIVE SUNGLASSES
#define IDI_MSA_PIC_MERCURY_ALIEN_1		74
#define IDI_MSA_PIC_MERCURY_ALIEN_2		81
#define IDI_MSA_PIC_MERCURY_CAVE_0		70		// hidden feature, press '2' here
#define IDI_MSA_PIC_MERCURY_CAVE_1		78
#define IDI_MSA_PIC_MERCURY_CAVE_2		79
#define IDI_MSA_PIC_MERCURY_SHIP		76
#define IDI_MSA_PIC_MERCURY_WEIGH		82

#define IDI_MSA_PIC_SATURN_0			84
#define IDI_MSA_PIC_SATURN_1			86
#define IDI_MSA_PIC_SATURN_2			90
#define IDI_MSA_PIC_SATURN_3			91
#define IDI_MSA_PIC_SATURN_ISLAND		89		// CRYSTAL
#define IDI_MSA_PIC_SATURN_LAKE_0		85		// USE MATTRESS
#define IDI_MSA_PIC_SATURN_LAKE_1		88		// USE MATTRESS
#define IDI_MSA_PIC_SATURN_LAKE_2		92		// USE MATTRESS
#define IDI_MSA_PIC_SATURN_SHIP			87
#define IDI_MSA_PIC_SATURN_WEIGH		83		// WEIGH MICKEY

#define IDI_MSA_PIC_PLUTO_0				93
#define IDI_MSA_PIC_PLUTO_1				96
#define IDI_MSA_PIC_PLUTO_2				97
#define IDI_MSA_PIC_PLUTO_3				98
#define IDI_MSA_PIC_PLUTO_4				101
#define IDI_MSA_PIC_PLUTO_ALIENS		100		// CRYSTAL, GIVE BONE
#define IDI_MSA_PIC_PLUTO_CAVE_0		99
#define IDI_MSA_PIC_PLUTO_CAVE_1		103
#define IDI_MSA_PIC_PLUTO_CRATER		102
#define IDI_MSA_PIC_PLUTO_SHIP			95
#define IDI_MSA_PIC_PLUTO_WEIGH			94		// WEIGH MICKEY

#define IDI_MSA_PIC_JUPITER_0			106
#define IDI_MSA_PIC_JUPITER_1			107
#define IDI_MSA_PIC_JUPITER_2			108
#define IDI_MSA_PIC_JUPITER_3			109
#define IDI_MSA_PIC_JUPITER_4			113
#define IDI_MSA_PIC_JUPITER_5			116
#define IDI_MSA_PIC_JUPITER_6			117
#define IDI_MSA_PIC_JUPITER_7			120
#define IDI_MSA_PIC_JUPITER_CRACK		114
#define IDI_MSA_PIC_JUPITER_LAVA		110		// CRYSTAL, THROW ROCK
#define IDI_MSA_PIC_JUPITER_ROCK_0		112		// GET ROCK
#define IDI_MSA_PIC_JUPITER_ROCK_1		119		// GET ROCK
#define IDI_MSA_PIC_JUPITER_SHIP		115
#define IDI_MSA_PIC_JUPITER_WEIGH		118		// WEIGH MICKEY

#define IDI_MSA_PIC_MARS_0				121
#define IDI_MSA_PIC_MARS_1				124
#define IDI_MSA_PIC_MARS_2				125
#define IDI_MSA_PIC_MARS_3				126
#define IDI_MSA_PIC_MARS_4				127
#define IDI_MSA_PIC_MARS_5				128
#define IDI_MSA_PIC_MARS_6				130
#define IDI_MSA_PIC_MARS_SHIP			123
#define IDI_MSA_PIC_MARS_TUBE_0			129
#define IDI_MSA_PIC_MARS_TUBE_1			131
#define IDI_MSA_PIC_MARS_VOLCANO		132		// CRYSTAL, DIG PLUTO
#define IDI_MSA_PIC_MARS_WEIGH			122		// WEIGH MICKEY

#define IDI_MSA_PIC_URANUS_0			133
#define IDI_MSA_PIC_URANUS_1			134
#define IDI_MSA_PIC_URANUS_2			135
#define IDI_MSA_PIC_URANUS_3			138
#define IDI_MSA_PIC_URANUS_4			139
#define IDI_MSA_PIC_URANUS_5			140
#define IDI_MSA_PIC_URANUS_6			142
#define IDI_MSA_PIC_URANUS_CHAMBER		145		// CRYSTAL, USE CROWBAR
#define IDI_MSA_PIC_URANUS_SHIP			137
#define IDI_MSA_PIC_URANUS_STEPS		144
#define IDI_MSA_PIC_URANUS_ENTRANCE		141		// ENTER TEMPLE
#define IDI_MSA_PIC_URANUS_TEMPLE		143		// USE CRYSTAL, ENTER DOOR
#define IDI_MSA_PIC_URANUS_TEMPLE_1		206		// crystal used
#define IDI_MSA_PIC_URANUS_TEMPLE_2		207		// door open
#define IDI_MSA_PIC_URANUS_WEIGH		136		// WEIGH MICKEY

#define IDI_MSA_PIC_STAR_MAP			165
#define IDI_MSA_PIC_TITLE				240

// objects

enum ENUM_MSA_OBJECT {
	IDI_MSA_OBJECT_NONE = -1,
	IDI_MSA_OBJECT_ROCK_0,
	IDI_MSA_OBJECT_WRENCH,
	IDI_MSA_OBJECT_SCALE,
	IDI_MSA_OBJECT_CROWBAR,
	IDI_MSA_OBJECT_BONE,
	IDI_MSA_OBJECT_SUNGLASSES,
	IDI_MSA_OBJECT_DESK_STUFF,
	IDI_MSA_OBJECT_MATTRESS,
	IDI_MSA_OBJECT_SCARF,
	IDI_MSA_OBJECT_FLASHLIGHT,
	IDI_MSA_OBJECT_ROPE,
	IDI_MSA_OBJECT_ROCK_1,
	IDI_MSA_OBJECT_SCARF_C64,
	IDI_MSA_OBJECT_ROCK_2,
	IDI_MSA_OBJECT_ROCK_3,
	IDI_MSA_OBJECT_W_EARTH,
	IDI_MSA_OBJECT_W_VENUS,
	IDI_MSA_OBJECT_W_TRITON,
	IDI_MSA_OBJECT_W_MERCURY,
	IDI_MSA_OBJECT_W_TITAN,
	IDI_MSA_OBJECT_W_PLUTO,
	IDI_MSA_OBJECT_W_IO,
	IDI_MSA_OBJECT_W_MARS,
	IDI_MSA_OBJECT_W_OBERON,
	IDI_MSA_OBJECT_W_SPACE,
	IDI_MSA_OBJECT_XL31,
	IDI_MSA_OBJECT_XL31E,
	IDI_MSA_OBJECT_XL32,
	IDI_MSA_OBJECT_XL32E,
	IDI_MSA_OBJECT_XL33,
	IDI_MSA_OBJECT_XL33E,
	IDI_MSA_OBJECT_CRYSTAL
};

const char IDS_MSA_NAME_OBJ[][9] = {
	"rok1", "wrench", "scale", "cbar", "bone", "glasses", "deskstuf", "raft",
	"scarf", "flashlit", "rope", "rok1", "scarfc64", "rok2", "rock35", "earthw",
	"venw", "trw", "merw", "titw", "plw", "iow", "mrw", "obw", "spw", "xl31",
	"xl31e", "xl32", "xl32e", "xl33", "xl33e", "crys1"
};

const int IDI_MSA_XTAL_ROOM_XY[IDI_MSA_MAX_PLANET][3] = {
	// room							x		y
	{IDI_MSA_PIC_EARTH_UP_IN_TREE,	14,		76},
	{IDI_MSA_PIC_VENUS_PROBE,		74,		80},
	{IDI_MSA_PIC_NEPTUNE_LEADER,	70,		27},
	{IDI_MSA_PIC_MERCURY_ALIEN_0,	123,	64},
	{IDI_MSA_PIC_SATURN_ISLAND,		110,	115},
	{IDI_MSA_PIC_PLUTO_ALIENS,		60,		104},
	{IDI_MSA_PIC_JUPITER_LAVA,		56,		54},
	{IDI_MSA_PIC_MARS_VOLCANO,		107,	100},
	{IDI_MSA_PIC_URANUS_CHAMBER,	90,		4}
};

// planets

enum ENUM_MSA_PLANET {
	IDI_MSA_PLANET_EARTH = 0,
	IDI_MSA_PLANET_VENUS,
	IDI_MSA_PLANET_NEPTUNE,
	IDI_MSA_PLANET_MERCURY,
	IDI_MSA_PLANET_SATURN,
	IDI_MSA_PLANET_PLUTO,
	IDI_MSA_PLANET_JUPITER,
	IDI_MSA_PLANET_MARS,
	IDI_MSA_PLANET_URANUS,
	IDI_MSA_PLANET_SPACESHIP
};

const char IDS_MSA_NAME_DAT[][13] = {
	"earth.dat", "venus.dat", "neptune.dat", "mercury.dat", "saturn.dat",
	"pluto.dat", "jupiter.dat", "mars.dat", "uranus.dat", "spacship.dat"
};

const char IDS_MSA_NAME_PLANET[][10] = {
	"EARTH", "VENUS", "TRITON", "MERCURY", "TITAN",
	"PLUTO", "IO", "MARS", "OBERON"
};

const char IDS_MSA_NAME_PLANET_2[][10] = {
	"EARTH", "VENUS", "NEPTUNE", "MERCURY", "SATURN",
	"PLUTO", "JUPITER", "MARS", "URANUS"
};

const char IDS_MSA_ADDR_PLANET[][7] = {
	"OB", "B", "OOBBB", "O", "OOBB",
	"OOOBBB", "OBB", "OOB", "OOOBB"
};

const int IDI_MSA_HOME_PLANET[] = {
	IDI_MSA_PIC_EARTH_SHIP, IDI_MSA_PIC_VENUS_SHIP, IDI_MSA_PIC_NEPTUNE_SHIP, IDI_MSA_PIC_MERCURY_SHIP,
	IDI_MSA_PIC_SATURN_SHIP, IDI_MSA_PIC_PLUTO_SHIP, IDI_MSA_PIC_JUPITER_SHIP, IDI_MSA_PIC_MARS_SHIP,
	IDI_MSA_PIC_URANUS_SHIP
};

const int IDI_MSA_SHIP_PLANET[] = {
	0, IDI_MSA_PIC_SHIP_VENUS, IDI_MSA_PIC_SHIP_NEPTUNE, IDI_MSA_PIC_SHIP_MERCURY, IDI_MSA_PIC_SHIP_SATURN,
	IDI_MSA_PIC_SHIP_PLUTO, IDI_MSA_PIC_SHIP_JUPITER, IDI_MSA_PIC_SHIP_MARS, IDI_MSA_PIC_SHIP_URANUS
};

// items

enum ENUM_MSA_ITEM {
	IDI_MSA_ITEM_FLASHLIGHT = 0,
	IDI_MSA_ITEM_ROPE,
	IDI_MSA_ITEM_BONE,
	IDI_MSA_ITEM_LETTER,
	IDI_MSA_ITEM_CROWBAR,
	IDI_MSA_ITEM_WRENCH,
	IDI_MSA_ITEM_MATTRESS,
	IDI_MSA_ITEM_SCARF,
	IDI_MSA_ITEM_SUNGLASSES,
	IDI_MSA_ITEM_SCALE,
	IDI_MSA_ITEM_ROCK
};

const char IDS_MSA_NAME_ITEM[][15] = {
	"A FLASHLIGHT", "A ROPE ", "A BONE ", "A LETTER", "A CROWBAR", "A WRENCH",
	"A MATTRESS", "A SCARF", "SUNGLASSES", "A SCALE ", "A ROCK "
};

// buttons

#define IDI_MSA_BUTTON_ORANGE		0x4F	// 'O'
#define IDI_MSA_BUTTON_BLUE			0x42	// 'B'

// file structures

struct MSA_TEXT_ENTRY {
	uint8	x0;
	uint8	szText[11];
};

struct MSA_TEXT_BLOCK {
	uint8	count;
	MSA_TEXT_ENTRY	entry[5];
};

struct MSA_MSG_BLOCK {
	uint8	data[5];
};

struct MSA_MENU {
	MSA_TEXT_BLOCK row[2];
	MSA_MSG_BLOCK cmd[5];
	MSA_MSG_BLOCK arg[5];
};

struct MSA_DAT_HEADER {
	uint16	filelen;
	uint16	ofsRoom[IDI_MSA_MAX_ROOM];
	uint16	ofsDesc[IDI_MSA_MAX_ROOM];
	uint16	ofsStr[IDI_MSA_MAX_ROOM];
};

struct MSA_SND_NOTE {
	uint16	counter;	// freq = 1193180 / counter
	uint8	length;		// msec = length / 0.0182
};

// file offset modifiers

#define IDI_MSA_OFS_DAT					0x0002
#define IDI_MSA_OFS_EXE					0x35C0

// actions

#define IDI_MSA_ACTION_GOTO_ROOM		0x00
#define IDI_MSA_ACTION_SHOW_INT_STR		0x01
#define IDI_MSA_ACTION_UNUSED			0x02
#define IDI_MSA_ACTION_SHOW_DAT_STR		0x03

#define IDI_MSA_ACTION_GET_ROPE			0x7F
#define IDI_MSA_ACTION_UNTIE_ROPE		0x80
#define IDI_MSA_ACTION_GET_BONE			0x81
#define IDI_MSA_ACTION_GET_XTAL_EARTH	0x82
#define IDI_MSA_ACTION_LOOK_DESK		0x83
#define IDI_MSA_ACTION_WRITE_LETTER		0x84
#define IDI_MSA_ACTION_MAIL_LETTER		0x85
#define IDI_MSA_ACTION_OPEN_CUPBOARD	0x86
#define IDI_MSA_ACTION_GET_FLASHLIGHT	0x87
#define IDI_MSA_ACTION_OPEN_CABINET		0x88
#define IDI_MSA_ACTION_GET_CROWBAR		0x89
#define IDI_MSA_ACTION_GET_WRENCH		0x8A
#define IDI_MSA_ACTION_OPEN_CLOSET		0x8B
#define IDI_MSA_ACTION_GET_MATTRESS		0x8C
#define IDI_MSA_ACTION_GET_SCARF		0x8D
#define IDI_MSA_ACTION_GET_SUNGLASSES	0x8E
#define IDI_MSA_ACTION_GET_SCALE		0x8F
#define IDI_MSA_ACTION_GOTO_SPACESHIP	0x90

#define IDI_MSA_ACTION_DOWN_CHASM		0x91
#define IDI_MSA_ACTION_DOWN_ROPE		0x92
#define IDI_MSA_ACTION_USE_ROPE			0x93
#define IDI_MSA_ACTION_OPEN_HATCH		0x94
#define IDI_MSA_ACTION_USE_WRENCH		0x95
#define IDI_MSA_ACTION_GET_XTAL_VENUS	0x96

#define IDI_MSA_ACTION_LOOK_CASTLE		0x97
#define IDI_MSA_ACTION_ENTER_OPENING	0x98
#define IDI_MSA_ACTION_USE_CROWBAR		0x99
#define IDI_MSA_ACTION_GET_XTAL_NEPTUNE	0x9A
#define IDI_MSA_ACTION_TALK_LEADER		0x9B
#define IDI_MSA_ACTION_GIVE_SCARF		0x9C

#define IDI_MSA_ACTION_GET_XTAL_MERCURY	0x9D
#define IDI_MSA_ACTION_GIVE_SUNGLASSES	0x9E
#define IDI_MSA_ACTION_CROSS_LAKE		0x9F
#define IDI_MSA_ACTION_USE_MATTRESS		0xA0
#define IDI_MSA_ACTION_GET_XTAL_SATURN	0xA1
#define IDI_MSA_ACTION_LEAVE_ISLAND		0xA2

#define IDI_MSA_ACTION_GET_XTAL_PLUTO	0xA3
#define IDI_MSA_ACTION_GIVE_BONE		0xA4

#define IDI_MSA_ACTION_GET_ROCK_0		0xA5
#define IDI_MSA_ACTION_GET_ROCK_1		0xA6
#define IDI_MSA_ACTION_GET_XTAL_JUPITER	0xA7
#define IDI_MSA_ACTION_THROW_ROCK		0xA8

#define IDI_MSA_ACTION_GO_TUBE			0xA9
#define IDI_MSA_ACTION_USE_FLASHLIGHT	0xAA
#define IDI_MSA_ACTION_PLUTO_DIG		0xAB
#define IDI_MSA_ACTION_GET_XTAL_MARS	0xAC

#define IDI_MSA_ACTION_USE_CRYSTAL		0xAD
#define IDI_MSA_ACTION_OPEN_DOOR		0xAE
#define IDI_MSA_ACTION_ENTER_DOOR		0xAF
#define IDI_MSA_ACTION_GET_XTAL_URANUS	0xB0
#define IDI_MSA_ACTION_USE_CROWBAR_1	0xB1

#define IDI_MSA_ACTION_GO_NORTH			0xB2
#define IDI_MSA_ACTION_GO_PLANET		0xB3
#define IDI_MSA_ACTION_PRESS_BUTTON		0xB4
#define IDI_MSA_ACTION_WEAR_SPACESUIT	0xB5
#define IDI_MSA_ACTION_READ_GAUGE		0xB6
#define IDI_MSA_ACTION_PRESS_ORANGE		0xB7
#define IDI_MSA_ACTION_PRESS_BLUE		0xB8
#define IDI_MSA_ACTION_FLIP_SWITCH		0xB9
#define IDI_MSA_ACTION_PUSH_THROTTLE	0xBA
#define IDI_MSA_ACTION_PULL_THROTTLE	0xBB
#define IDI_MSA_ACTION_LEAVE_ROOM		0xBC
#define IDI_MSA_ACTION_OPEN_CABINET_1	0xBD
#define IDI_MSA_ACTION_READ_MAP			0xBE
#define IDI_MSA_ACTION_GO_WEST			0xBF

#define IDI_MSA_ACTION_PLANET_INFO		0xC0
#define IDI_MSA_ACTION_ENTER_TEMPLE		0xC1
#define IDI_MSA_ACTION_OPEN_MAILBOX		0xC2
#define IDI_MSA_ACTION_SAVE_GAME		0xC3
#define IDI_MSA_ACTION_LOOK_MICKEY		0xC4

// sounds

enum ENUM_MSA_SOUND {
	IDI_MSA_SND_THEME,
	IDI_MSA_SND_CRYSTAL,
	IDI_MSA_SND_TAKE,
	IDI_MSA_SND_GAME_OVER,
	IDI_MSA_SND_PRESS_BLUE,
	IDI_MSA_SND_PRESS_ORANGE,
	IDI_MSA_SND_SHIP_LAND,
	IDI_MSA_SND_XL30
};

// message offsets within mickey.exe

const int IDO_MSA_HIDDEN_MSG[] = {
	0x8C44, 0x8C83, 0x8D23, 0x8D97, 0x8E2A
};

const int IDO_MSA_GAME_OVER[] = {
	0x7914, 0x7978, 0x7A17, 0x7A94, 0x7B04, 0x7B8F, 0x7BEB, 0x7C79
};

const int IDO_MSA_SAVE_GAME[] = {
	0x73FA, 0x7436, 0x746C, 0x74E9, 0x75F6, 0x766A, 0x758B
	// do you have a formatted disk, insert disk, insert disk 2, save by number
	// everything will be lost, previous game will be lost, game saved
};

const int IDO_MSA_LOAD_GAME[] = {
	0x76CE, 0x770B, 0x7777
	// do you want to load game, insert game save disk, game restored
};

const int IDO_MSA_AIR_SUPPLY[] = {
	0x7D10, 0x7D31, 0x7D51, 0x7D9B
	// be aware, low, dangerously low, out of air
};

const int IDI_MSA_AIR_SUPPLY[] = { 30, 20, 10, 0 };

// planet information

const int IDO_MSA_PLANET_INFO[IDI_MSA_MAX_PLANET][4] = {
	{0x6313, 0x63B2, 0x6449, 0},		// EARTH
	{0x61EB, 0x6288, 0,		 0},		// VENUS
	{0x6B64, 0x6C06, 0x6CA3, 0},		// NEPTUNE
	{0x609B, 0x612C, 0x61CA, 0},		// MERCURY
	{0x6879, 0x6916, 0x6984, 0},		// SATURN
	{0x6CCF, 0x6D72, 0x6E10, 0},		// PLUTO
	{0x667C, 0x6714, 0x67B1, 0x684E},	// JUPITER
	{0x6471, 0x650F, 0x65AD, 0x6651},	// MARS
	{0x69C3, 0x6A62, 0x6B00, 0}			// URANUS
};

// next crystal piece hints

const int IDO_MSA_NEXT_PIECE[IDI_MSA_MAX_PLANET][5] = {
	{0,		0,		0,		0,		0},				// earth
	{0x4DCC,	0x4E20,	0x4E64,	0x4E9E,	0x4F0B},	// venus
	{0x5900,	0x599B,	0x5A07,	0x5A8E,	0x5B07},	// neptune
	{0x4F57,	0x4FA3,	0x4FF1,	0x5056,	0x50BD},	// mercury
	{0x5471,	0x54DF,	0x5548,	0x55C2,	0x562A},	// saturn
	{0x5B78,	0x5BB6,	0x5C29,	0x5C76,	0x5CE1},	// pluto
	{0x526B,	0x52DA,	0x5340,	0x53A1,	0x540C},	// jupiter
	{0x50F6,	0x512C,	0x5170,	0x51D5,	0x5228},	// mars
	{0x56AA,	0x571C,	0x579E,	0x5807,	0x5875}		// uranus
};

// message offsets

#define IDO_MSA_COPYRIGHT						0x7801
#define IDO_MSA_INTRO							0x4679
#define IDO_MSA_GAME_STORY						0x6E9C

#define IDO_MSA_PRESS_1_TO_9					0x7530
#define IDO_MSA_PRESS_YES_OR_NO					0x480D
#define IDO_MSA_TOO_MANY_BUTTONS_PRESSED		0x5DF7

#define IDO_MSA_XL30_SPEAKING					0x4725
#define IDO_MSA_CRYSTAL_PIECE_FOUND				0x600C

#define IDO_MSA_ROOM_TEXT_OFFSETS				0x8B01
#define IDO_MSA_ROOM_OBJECT_XY_OFFSETS			0x8EA8
#define IDO_MSA_ROOM_MENU_FIX					0x4a27

// offsets to offset arrays

#define IDOFS_MSA_MENU_PATCHES					0x5e7a
#define IDOFS_MSA_SOUND_DATA					0x9deb

// game structure

struct MSA_GAME {
	uint8 iRoom;
	uint8 iPlanet;
	uint8 iDisk;

	uint8 nAir;
	uint8 nButtons;
	uint8 nRocks;

	uint8 nXtals;
	uint8 iPlanetXtal[IDI_MSA_MAX_DAT];
	uint16 iClue[IDI_MSA_MAX_PLANET];
	char szAddr[IDI_MSA_MAX_BUTTON + 1];

	// Flags
	bool fHasXtal;
	bool fIntro;
	bool fSuit;
	bool fShipDoorOpen;
	bool fFlying;
	bool fStoryShown;
	bool fPlanetsInitialized;
	bool fTempleDoorOpen;
	bool fAnimXL30;
	bool fItem[IDI_MSA_MAX_ITEM];
	bool fItemUsed[IDI_MSA_MAX_ITEM];

	int8 iItem[IDI_MSA_MAX_ITEM];
	uint8 nItems;

	//uint8 fRmTxt[IDI_MSA_MAX_ROOM];
	int8 iRmObj[IDI_MSA_MAX_ROOM];
	uint8 iRmPic[IDI_MSA_MAX_ROOM];
	uint16 oRmTxt[IDI_MSA_MAX_ROOM];

	uint8 iRmMenu[IDI_MSA_MAX_ROOM];
	uint8 nRmMenu[IDI_MSA_MAX_ROOM];

	int8 nFrame;
};

class PreAgiEngine;

class MickeyEngine : public PreAgiEngine {
public:
	MickeyEngine(OSystem *syst, const AGIGameDescription *gameDesc);
	~MickeyEngine();

	void init();
	Common::Error go();

	void debugCurRoom();
	void debugGotoRoom(int);
	void drawPic(int);
	void drawObj(ENUM_MSA_OBJECT, int, int);

	GUI::Debugger *getDebugger() { return _console; }

protected:
	MickeyConsole *_console;

	MSA_GAME _gameStateMickey;
	bool _clickToMove;

	int getDat(int);
	void readExe(int, uint8*, long);
	void getDatFileName(int, char*);
	void readDatHdr(char*, MSA_DAT_HEADER*);
	void readOfsData(int, int, uint8*, long);
	bool chooseY_N(int, bool);
	int choose1to9(int);
	void printStr(char *);
	void printLine(const char*);
	void printExeStr(int);
	void printExeMsg(int);
	void printDesc(int);
	bool checkMenu();
	void drawMenu(MSA_MENU, int, int);
	void getMouseMenuSelRow(MSA_MENU, int*, int*, int, int, int);
	bool getMenuSelRow(MSA_MENU, int*, int*, int);
	void getMenuSel(char*, int*, int*);
	void centerMenu(MSA_MENU*);
	void patchMenu(MSA_MENU*);
	void printDatString(int);
	void printDatMessage(int);
	void playNote(MSA_SND_NOTE);
	void playSound(ENUM_MSA_SOUND);
	void drawRoomAnimation();
	void drawRoom();
	void drawLogo();
	void animate();
	void printRoomDesc();
	bool loadGame();
	void saveGame();
	void showPlanetInfo();
	void printStory();
	int getPlanet();
	void pressOB(int);
	void insertDisk(int);
	void gameOver();
	void inventory();
	void intro();
	void getItem(ENUM_MSA_ITEM);
	void getXtal(int);
	bool parse(int, int);
	void flipSwitch();
	void waitAnyKey(bool anim = false);

	bool planetIsAlreadyAssigned(int planet) {
		for (int j = 0; j < IDI_MSA_MAX_PLANET; j++) {
			if (_gameStateMickey.iPlanetXtal[j] == planet)
				return true;
		}
		return false;
	}

	bool mickeyHasItem(int item) {
		if (_gameStateMickey.fItem[item]) {
			printDatMessage(90);	// Mickey already has item
			return true;
		} else {
			return false;
		}
	}
};

} // End of namespace Agi

#endif
