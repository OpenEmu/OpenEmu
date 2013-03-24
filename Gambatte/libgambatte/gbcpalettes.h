/***************************************************************************
 *   Copyright (C) 2007 by Sindre Aamås                                    *
 *   sinamas@users.sourceforge.net                                         *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License version 2 as     *
 *   published by the Free Software Foundation.                            *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License version 2 for more details.                *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   version 2 along with this program; if not, write to the               *
 *   Free Software Foundation, Inc.,                                       *
 *   59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.             *
 ***************************************************************************/

#include <cstring>
#include <algorithm>

namespace {

#define TO5BIT(c8) (((c8) * 0x1F * 2 + 0xFF) / (0xFF*2))
#define PACK15_1(rgb24) (TO5BIT((rgb24) >> 16 & 0xFF) << 10 | TO5BIT((rgb24) >> 8 & 0xFF) << 5 | TO5BIT((rgb24) & 0xFF))
#define PACK15_4(c0, c1, c2, c3) \
	PACK15_1(c0), PACK15_1(c1), PACK15_1(c2), PACK15_1(c3)

static const unsigned short p005[] = {
	PACK15_4(0xFFFFFF, 0x52FF00, 0xFF4200, 0x000000),
	PACK15_4(0xFFFFFF, 0x52FF00, 0xFF4200, 0x000000),
	PACK15_4(0xFFFFFF, 0x52FF00, 0xFF4200, 0x000000)
};

static const unsigned short p006[] = {
	PACK15_4(0xFFFFFF, 0xFF9C00, 0xFF0000, 0x000000),
	PACK15_4(0xFFFFFF, 0xFF9C00, 0xFF0000, 0x000000),
	PACK15_4(0xFFFFFF, 0xFF9C00, 0xFF0000, 0x000000)
};

static const unsigned short p007[] = {
	PACK15_4(0xFFFFFF, 0xFFFF00, 0xFF0000, 0x000000),
	PACK15_4(0xFFFFFF, 0xFFFF00, 0xFF0000, 0x000000),
	PACK15_4(0xFFFFFF, 0xFFFF00, 0xFF0000, 0x000000)
};

static const unsigned short p008[] = {
	PACK15_4(0xA59CFF, 0xFFFF00, 0x006300, 0x000000),
	PACK15_4(0xA59CFF, 0xFFFF00, 0x006300, 0x000000),
	PACK15_4(0xA59CFF, 0xFFFF00, 0x006300, 0x000000)
};

static const unsigned short p012[] = {
	PACK15_4(0xFFFFFF, 0xFFAD63, 0x843100, 0x000000),
	PACK15_4(0xFFFFFF, 0xFFAD63, 0x843100, 0x000000),
	PACK15_4(0xFFFFFF, 0xFFAD63, 0x843100, 0x000000)
};

static const unsigned short p013[] = {
	PACK15_4(0x000000, 0x008484, 0xFFDE00, 0xFFFFFF),
	PACK15_4(0x000000, 0x008484, 0xFFDE00, 0xFFFFFF),
	PACK15_4(0x000000, 0x008484, 0xFFDE00, 0xFFFFFF)
};

static const unsigned short p016[] = {
	PACK15_4(0xFFFFFF, 0xA5A5A5, 0x525252, 0x000000),
	PACK15_4(0xFFFFFF, 0xA5A5A5, 0x525252, 0x000000),
	PACK15_4(0xFFFFFF, 0xA5A5A5, 0x525252, 0x000000)
};

static const unsigned short p017[] = {
	PACK15_4(0xFFFFA5, 0xFF9494, 0x9494FF, 0x000000),
	PACK15_4(0xFFFFA5, 0xFF9494, 0x9494FF, 0x000000),
	PACK15_4(0xFFFFA5, 0xFF9494, 0x9494FF, 0x000000)
};

static const unsigned short p01B[] = {
	PACK15_4(0xFFFFFF, 0xFFCE00, 0x9C6300, 0x000000),
	PACK15_4(0xFFFFFF, 0xFFCE00, 0x9C6300, 0x000000),
	PACK15_4(0xFFFFFF, 0xFFCE00, 0x9C6300, 0x000000)
};

static const unsigned short p100[] = {
	PACK15_4(0xFFFFFF, 0xADAD84, 0x42737B, 0x000000),
	PACK15_4(0xFFFFFF, 0xFF7300, 0x944200, 0x000000),
	PACK15_4(0xFFFFFF, 0xADAD84, 0x42737B, 0x000000)
};

static const unsigned short p10B[] = {
	PACK15_4(0xFFFFFF, 0x63A5FF, 0x0000FF, 0x000000),
	PACK15_4(0xFFFFFF, 0xFF8484, 0x943A3A, 0x000000),
	PACK15_4(0xFFFFFF, 0x63A5FF, 0x0000FF, 0x000000)
};

static const unsigned short p10D[] = {
	PACK15_4(0xFFFFFF, 0x8C8CDE, 0x52528C, 0x000000),
	PACK15_4(0xFFFFFF, 0xFF8484, 0x943A3A, 0x000000),
	PACK15_4(0xFFFFFF, 0x8C8CDE, 0x52528C, 0x000000)
};

static const unsigned short p110[] = {
	PACK15_4(0xFFFFFF, 0xFF8484, 0x943A3A, 0x000000),
	PACK15_4(0xFFFFFF, 0x7BFF31, 0x008400, 0x000000),
	PACK15_4(0xFFFFFF, 0xFF8484, 0x943A3A, 0x000000)
};

static const unsigned short p11C[] = {
	PACK15_4(0xFFFFFF, 0x7BFF31, 0x0063C5, 0x000000),
	PACK15_4(0xFFFFFF, 0xFF8484, 0x943A3A, 0x000000),
	PACK15_4(0xFFFFFF, 0x7BFF31, 0x0063C5, 0x000000)
};

static const unsigned short p20B[] = {
	PACK15_4(0xFFFFFF, 0x63A5FF, 0x0000FF, 0x000000),
	PACK15_4(0xFFFFFF, 0x63A5FF, 0x0000FF, 0x000000),
	PACK15_4(0xFFFFFF, 0xFF8484, 0x943A3A, 0x000000)
};

static const unsigned short p20C[] = {
	PACK15_4(0xFFFFFF, 0x8C8CDE, 0x52528C, 0x000000),
	PACK15_4(0xFFFFFF, 0x8C8CDE, 0x52528C, 0x000000),
	PACK15_4(0xFFC542, 0xFFD600, 0x943A00, 0x4A0000)
};

static const unsigned short p300[] = {
	PACK15_4(0xFFFFFF, 0xADAD84, 0x42737B, 0x000000),
	PACK15_4(0xFFFFFF, 0xFF7300, 0x944200, 0x000000),
	PACK15_4(0xFFFFFF, 0xFF7300, 0x944200, 0x000000)
};

static const unsigned short p304[] = {
	PACK15_4(0xFFFFFF, 0x7BFF00, 0xB57300, 0x000000),
	PACK15_4(0xFFFFFF, 0xFF8484, 0x943A3A, 0x000000),
	PACK15_4(0xFFFFFF, 0xFF8484, 0x943A3A, 0x000000)
};

static const unsigned short p305[] = {
	PACK15_4(0xFFFFFF, 0x52FF00, 0xFF4200, 0x000000),
	PACK15_4(0xFFFFFF, 0xFF8484, 0x943A3A, 0x000000),
	PACK15_4(0xFFFFFF, 0xFF8484, 0x943A3A, 0x000000)
};

static const unsigned short p306[] = {
	PACK15_4(0xFFFFFF, 0xFF9C00, 0xFF0000, 0x000000),
	PACK15_4(0xFFFFFF, 0xFF8484, 0x943A3A, 0x000000),
	PACK15_4(0xFFFFFF, 0xFF8484, 0x943A3A, 0x000000)
};

static const unsigned short p308[] = {
	PACK15_4(0xA59CFF, 0xFFFF00, 0x006300, 0x000000),
	PACK15_4(0xFF6352, 0xD60000, 0x630000, 0x000000),
	PACK15_4(0xFF6352, 0xD60000, 0x630000, 0x000000)
};

static const unsigned short p30A[] = {
	PACK15_4(0xB5B5FF, 0xFFFF94, 0xAD5A42, 0x000000),
	PACK15_4(0x000000, 0xFFFFFF, 0xFF8484, 0x943A3A),
	PACK15_4(0x000000, 0xFFFFFF, 0xFF8484, 0x943A3A)
};

static const unsigned short p30C[] = {
	PACK15_4(0xFFFFFF, 0x8C8CDE, 0x52528C, 0x000000),
	PACK15_4(0xFFC542, 0xFFD600, 0x943A00, 0x4A0000),
	PACK15_4(0xFFC542, 0xFFD600, 0x943A00, 0x4A0000)
};

static const unsigned short p30D[] = {
	PACK15_4(0xFFFFFF, 0x8C8CDE, 0x52528C, 0x000000),
	PACK15_4(0xFFFFFF, 0xFF8484, 0x943A3A, 0x000000),
	PACK15_4(0xFFFFFF, 0xFF8484, 0x943A3A, 0x000000)
};

static const unsigned short p30E[] = {
	PACK15_4(0xFFFFFF, 0x7BFF31, 0x008400, 0x000000),
	PACK15_4(0xFFFFFF, 0xFF8484, 0x943A3A, 0x000000),
	PACK15_4(0xFFFFFF, 0xFF8484, 0x943A3A, 0x000000)
};

static const unsigned short p30F[] = {
	PACK15_4(0xFFFFFF, 0xFFAD63, 0x843100, 0x000000),
	PACK15_4(0xFFFFFF, 0x63A5FF, 0x0000FF, 0x000000),
	PACK15_4(0xFFFFFF, 0x63A5FF, 0x0000FF, 0x000000)
};

static const unsigned short p312[] = {
	PACK15_4(0xFFFFFF, 0xFFAD63, 0x843100, 0x000000),
	PACK15_4(0xFFFFFF, 0x7BFF31, 0x008400, 0x000000),
	PACK15_4(0xFFFFFF, 0x7BFF31, 0x008400, 0x000000)
};

static const unsigned short p319[] = {
	PACK15_4(0xFFE6C5, 0xCE9C84, 0x846B29, 0x5A3108),
	PACK15_4(0xFFFFFF, 0xFFAD63, 0x843100, 0x000000),
	PACK15_4(0xFFFFFF, 0xFFAD63, 0x843100, 0x000000)
};

static const unsigned short p31C[] = {
	PACK15_4(0xFFFFFF, 0x7BFF31, 0x0063C5, 0x000000),
	PACK15_4(0xFFFFFF, 0xFF8484, 0x943A3A, 0x000000),
	PACK15_4(0xFFFFFF, 0xFF8484, 0x943A3A, 0x000000)
};

static const unsigned short p405[] = {
	PACK15_4(0xFFFFFF, 0x52FF00, 0xFF4200, 0x000000),
	PACK15_4(0xFFFFFF, 0x52FF00, 0xFF4200, 0x000000),
	PACK15_4(0xFFFFFF, 0x5ABDFF, 0xFF0000, 0x0000FF)
};

static const unsigned short p406[] = {
	PACK15_4(0xFFFFFF, 0xFF9C00, 0xFF0000, 0x000000),
	PACK15_4(0xFFFFFF, 0xFF9C00, 0xFF0000, 0x000000),
	PACK15_4(0xFFFFFF, 0x5ABDFF, 0xFF0000, 0x0000FF )
};

static const unsigned short p407[] = {
	PACK15_4(0xFFFFFF, 0xFFFF00, 0xFF0000, 0x000000),
	PACK15_4(0xFFFFFF, 0xFFFF00, 0xFF0000, 0x000000),
	PACK15_4(0xFFFFFF, 0x5ABDFF, 0xFF0000, 0x0000FF)
};

static const unsigned short p500[] = {
	PACK15_4(0xFFFFFF, 0xADAD84, 0x42737B, 0x000000),
	PACK15_4(0xFFFFFF, 0xFF7300, 0x944200, 0x000000),
	PACK15_4(0xFFFFFF, 0x5ABDFF, 0xFF0000, 0x0000FF)
};

static const unsigned short p501[] = {
	PACK15_4(0xFFFF9C, 0x94B5FF, 0x639473, 0x003A3A),
	PACK15_4(0xFFC542, 0xFFD600, 0x943A00, 0x4A0000),
	PACK15_4(0xFFFFFF, 0xFF8484, 0x943A3A, 0x000000)
};

static const unsigned short p502[] = {
	PACK15_4(0x6BFF00, 0xFFFFFF, 0xFF524A, 0x000000),
	PACK15_4(0xFFFFFF, 0xFFFFFF, 0x63A5FF, 0x0000FF),
	PACK15_4(0xFFFFFF, 0xFFAD63, 0x843100, 0x000000)
};

static const unsigned short p503[] = {
	PACK15_4(0x52DE00, 0xFF8400, 0xFFFF00, 0xFFFFFF),
	PACK15_4(0xFFFFFF, 0xFFFFFF, 0x63A5FF, 0x0000FF),
	PACK15_4(0xFFFFFF, 0xFF8484, 0x943A3A, 0x000000)
};

static const unsigned short p508[] = {
	PACK15_4(0xA59CFF, 0xFFFF00, 0x006300, 0x000000),
	PACK15_4(0xFF6352, 0xD60000, 0x630000, 0x000000),
	PACK15_4(0x0000FF, 0xFFFFFF, 0xFFFF7B, 0x0084FF)
};

static const unsigned short p509[] = {
	PACK15_4(0xFFFFCE, 0x63EFEF, 0x9C8431, 0x5A5A5A),
	PACK15_4(0xFFFFFF, 0xFF7300, 0x944200, 0x000000),
	PACK15_4(0xFFFFFF, 0x63A5FF, 0x0000FF, 0x000000)
};

static const unsigned short p50B[] = {
	PACK15_4(0xFFFFFF, 0x63A5FF, 0x0000FF, 0x000000),
	PACK15_4(0xFFFFFF, 0xFF8484, 0x943A3A, 0x000000),
	PACK15_4(0xFFFFFF, 0xFFFF7B, 0x0084FF, 0xFF0000)
};

static const unsigned short p50C[] = {
	PACK15_4(0xFFFFFF, 0x8C8CDE, 0x52528C, 0x000000),
	PACK15_4(0xFFC542, 0xFFD600, 0x943A00, 0x4A0000),
	PACK15_4(0xFFFFFF, 0x5ABDFF, 0xFF0000, 0x0000FF)
};

static const unsigned short p50D[] = {
	PACK15_4(0xFFFFFF, 0x8C8CDE, 0x52528C, 0x000000),
	PACK15_4(0xFFFFFF, 0xFF8484, 0x943A3A, 0x000000),
	PACK15_4(0xFFFFFF, 0xFFAD63, 0x843100, 0x000000)
};

static const unsigned short p50E[] = {
	PACK15_4(0xFFFFFF, 0x7BFF31, 0x008400, 0x000000),
	PACK15_4(0xFFFFFF, 0xFF8484, 0x943A3A, 0x000000),
	PACK15_4(0xFFFFFF, 0x63A5FF, 0x0000FF, 0x000000)
};

static const unsigned short p50F[] = {
	PACK15_4(0xFFFFFF, 0xFFAD63, 0x843100, 0x000000),
	PACK15_4(0xFFFFFF, 0x63A5FF, 0x0000FF, 0x000000),
	PACK15_4(0xFFFFFF, 0x7BFF31, 0x008400, 0x000000)
};

static const unsigned short p510[] = {
	PACK15_4(0xFFFFFF, 0xFF8484, 0x943A3A, 0x000000),
	PACK15_4(0xFFFFFF, 0x7BFF31, 0x008400, 0x000000),
	PACK15_4(0xFFFFFF, 0x63A5FF, 0x0000FF, 0x000000)
};

static const unsigned short p511[] = {
	PACK15_4(0xFFFFFF, 0xFF8484, 0x943A3A, 0x000000),
	PACK15_4(0xFFFFFF, 0x00FF00, 0x318400, 0x004A00),
	PACK15_4(0xFFFFFF, 0x63A5FF, 0x0000FF, 0x000000)
};

static const unsigned short p512[] = {
	PACK15_4(0xFFFFFF, 0xFFAD63, 0x843100, 0x000000),
	PACK15_4(0xFFFFFF, 0x7BFF31, 0x008400, 0x000000),
	PACK15_4(0xFFFFFF, 0x63A5FF, 0x0000FF, 0x000000)
};

static const unsigned short p514[] = {
	PACK15_4(0xFFFFFF, 0x63A5FF, 0x0000FF, 0x000000),
	PACK15_4(0xFFFF00, 0xFF0000, 0x630000, 0x000000),
	PACK15_4(0xFFFFFF, 0x7BFF31, 0x008400, 0x000000)
};

static const unsigned short p515[] = {
	PACK15_4(0xFFFFFF, 0xADAD84, 0x42737B, 0x000000),
	PACK15_4(0xFFFFFF, 0xFFAD63, 0x843100, 0x000000),
	PACK15_4(0xFFFFFF, 0x63A5FF, 0x0000FF, 0x000000)
};

static const unsigned short p518[] = {
	PACK15_4(0xFFFFFF, 0x63A5FF, 0x0000FF, 0x000000),
	PACK15_4(0xFFFFFF, 0xFF8484, 0x943A3A, 0x000000),
	PACK15_4(0xFFFFFF, 0x7BFF31, 0x008400, 0x000000)
};

static const unsigned short p51A[] = {
	PACK15_4(0xFFFFFF, 0xFFFF00, 0x7B4A00, 0x000000),
	PACK15_4(0xFFFFFF, 0x63A5FF, 0x0000FF, 0x000000),
	PACK15_4(0xFFFFFF, 0x7BFF31, 0x008400, 0x000000)
};

static const unsigned short p51C[] = {
	PACK15_4(0xFFFFFF, 0x7BFF31, 0x0063C5, 0x000000),
	PACK15_4(0xFFFFFF, 0xFF8484, 0x943A3A, 0x000000),
	PACK15_4(0xFFFFFF, 0x63A5FF, 0x0000FF, 0x000000)
};

#undef PACK15_4
#undef PACK15_1
#undef TO5BIT

struct GbcPaletteEntry { const char *title; const unsigned short *p; };

static const GbcPaletteEntry gbcDirPalettes[] = {
	{ "GBC - Blue", p518 },
	{ "GBC - Brown", p012 },
	{ "GBC - Dark Blue", p50D },
	{ "GBC - Dark Brown", p319 },
	{ "GBC - Dark Green", p31C },
	{ "GBC - Grayscale", p016 },
	{ "GBC - Green", p005 },
	{ "GBC - Inverted", p013 },
	{ "GBC - Orange", p007 },
	{ "GBC - Pastel Mix", p017 },
	{ "GBC - Red", p510 },
	{ "GBC - Yellow", p51A },
};

static const GbcPaletteEntry gbcTitlePalettes[] = {
	{ "ALLEY WAY", p008 },
	{ "ASTEROIDS/MISCMD", p30E },
	{ "ATOMIC PUNK", p30F }, // unofficial ("DYNABLASTER" alt.)
	{ "BA.TOSHINDEN", p50F },
	{ "BALLOON KID", p006 },
	{ "BASEBALL", p503 },
	{ "BOMBERMAN GB", p31C }, // unofficial ("WARIO BLAST" alt.)
	{ "BOY AND BLOB GB1", p512 },
	{ "BOY AND BLOB GB2", p512 },
	{ "BT2RAGNAROKWORLD", p312 },
	{ "DEFENDER/JOUST", p50F },
	{ "DMG FOOTBALL", p30E },
	{ "DONKEY KONG", p306 },
	{ "DONKEYKONGLAND", p50C },
	{ "DONKEYKONGLAND 2", p50C },
	{ "DONKEYKONGLAND 3", p50C },
	{ "DONKEYKONGLAND95", p501 },
	{ "DR.MARIO", p20B },
	{ "DYNABLASTER", p30F },
	{ "F1RACE", p012 },
	{ "FOOTBALL INT'L", p502 }, // unofficial ("SOCCER" alt.)
	{ "G&W GALLERY", p304 },
	{ "GALAGA&GALAXIAN", p013 },
	{ "GAME&WATCH", p012 },
	{ "GAMEBOY GALLERY", p304 },
	{ "GAMEBOY GALLERY2", p304 },
	{ "GBWARS", p500 },
	{ "GBWARST", p500 }, // unofficial ("GBWARS" alt.)
	{ "GOLF", p30E },
	{ "Game and Watch 2", p304 },
	{ "HOSHINOKA-BI", p508 },
	{ "JAMES  BOND  007", p11C },
	{ "KAERUNOTAMENI", p10D },
	{ "KEN GRIFFEY JR", p31C },
	{ "KID ICARUS", p30D },
	{ "KILLERINSTINCT95", p50D },
	{ "KINGOFTHEZOO", p30F },
	{ "KIRAKIRA KIDS", p012 },
	{ "KIRBY BLOCKBALL", p508 },
	{ "KIRBY DREAM LAND", p508 },
	{ "KIRBY'S PINBALL", p308 },
	{ "KIRBY2", p508 },
	{ "LOLO2", p50F },
	{ "MAGNETIC SOCCER", p50E },
	{ "MANSELL", p012 },
	{ "MARIO & YOSHI", p305 },
	{ "MARIO'S PICROSS", p012 },
	{ "MARIOLAND2", p509 },
	{ "MEGA MAN 2", p50F },
	{ "MEGAMAN", p50F },
	{ "MEGAMAN3", p50F },
	{ "METROID2", p514 },
	{ "MILLI/CENTI/PEDE", p31C },
	{ "MOGURANYA", p300 },
	{ "MYSTIC QUEST", p50E },
	{ "NETTOU KOF 95", p50F },
	{ "NEW CHESSMASTER", p30F },
	{ "OTHELLO", p50E },
	{ "PAC-IN-TIME", p51C },
	{ "PENGUIN WARS", p30F }, // unofficial ("KINGOFTHEZOO" alt.)
	{ "PENGUINKUNWARSVS", p30F }, // unofficial ("KINGOFTHEZOO" alt.)
	{ "PICROSS 2", p012 },
	{ "PINOCCHIO", p20C },
	{ "POKEBOM", p30C },
	{ "POKEMON BLUE", p10B },
	{ "POKEMON GREEN", p11C },
	{ "POKEMON RED", p110 },
	{ "POKEMON YELLOW", p007 },
	{ "QIX", p407 },
	{ "RADARMISSION", p100 },
	{ "ROCKMAN WORLD", p50F },
	{ "ROCKMAN WORLD2", p50F },
	{ "ROCKMANWORLD3", p50F },
	{ "SEIKEN DENSETSU", p50E },
	{ "SOCCER", p502 },
	{ "SOLARSTRIKER", p013 },
	{ "SPACE INVADERS", p013 },
	{ "STAR STACKER", p012 },
	{ "STAR WARS", p512 },
	{ "STAR WARS-NOA", p512 },
	{ "STREET FIGHTER 2", p50F },
	{ "SUPER BOMBLISS  ", p006 }, // unofficial ("TETRIS BLAST" alt.)
	{ "SUPER MARIOLAND", p30A },
	{ "SUPER RC PRO-AM", p50F },
	{ "SUPERDONKEYKONG", p501 },
	{ "SUPERMARIOLAND3", p500 },
	{ "TENNIS", p502 },
	{ "TETRIS", p007 },
	{ "TETRIS ATTACK", p405 },
	{ "TETRIS BLAST", p006 },
	{ "TETRIS FLASH", p407 },
	{ "TETRIS PLUS", p31C },
	{ "TETRIS2", p407 },
	{ "THE CHESSMASTER", p30F },
	{ "TOPRANKINGTENNIS", p502 },
	{ "TOPRANKTENNIS", p502 },
	{ "TOY STORY", p30E },
	//{ "TRIP WORLD", p500 }, // unofficial
	{ "VEGAS STAKES", p50E },
	{ "WARIO BLAST", p31C },
	{ "WARIOLAND2", p515 },
	{ "WAVERACE", p50B },
	{ "WORLD CUP", p30E },
	{ "X", p016 },
	{ "YAKUMAN", p012 },
	{ "YOSHI'S COOKIE", p406 },
	{ "YOSSY NO COOKIE", p406 },
	{ "YOSSY NO PANEPON", p405 },
	{ "YOSSY NO TAMAGO", p305 },
	{ "ZELDA", p511 },
};

static inline std::size_t gbcDirPalettesSize() { return (sizeof gbcDirPalettes) / (sizeof gbcDirPalettes[0]); }
static inline const struct GbcPaletteEntry * gbcDirPalettesEnd() { return gbcDirPalettes + gbcDirPalettesSize(); }
static inline std::size_t gbcTitlePalettesSize() { return (sizeof gbcTitlePalettes) / (sizeof gbcTitlePalettes[0]); }
static inline const struct GbcPaletteEntry * gbcTitlePalettesEnd() { return gbcTitlePalettes + gbcTitlePalettesSize(); }

struct GbcPaletteEntryLess {
	bool operator()(const GbcPaletteEntry &lhs, const char *const rhstitle) const {
		return std::strcmp(lhs.title, rhstitle) < 0;
	}
};

static const unsigned short * findGbcDirPal(const char *const title) {
	const GbcPaletteEntry *const r = std::lower_bound(gbcDirPalettes, gbcDirPalettesEnd(), title, GbcPaletteEntryLess());
	return r < gbcDirPalettesEnd() && !std::strcmp(r->title, title) ? r->p : 0;
}

static const unsigned short * findGbcTitlePal(const char *const title) {
	const GbcPaletteEntry *const r = std::lower_bound(gbcTitlePalettes, gbcTitlePalettesEnd(), title, GbcPaletteEntryLess());
	return r < gbcTitlePalettesEnd() && !std::strcmp(r->title, title) ? r->p : 0;
}

static const unsigned short * findGbcPal(const char *const title) {
	if (const unsigned short *const pal = findGbcDirPal(title))
		return pal;
	
	return findGbcTitlePal(title);
}

static unsigned long gbcToRgb32(const unsigned rgb15) {
	const unsigned long r = rgb15 >> 10 & 0x1F;
	const unsigned long g = rgb15 >>  5 & 0x1F;
	const unsigned long b = rgb15       & 0x1F;
	
	return ((r * 13 + g * 2 + b) >> 1) << 16 | (g * 3 + b) << 9 | (r * 3 + g * 2 + b * 11) >> 1;
}

}
