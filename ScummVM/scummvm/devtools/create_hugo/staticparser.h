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

/*
 * This code is based on original Hugo Trilogy source code
 *
 * Copyright (c) 1989-1995 David P. Gray
 *
 */

#ifndef STATICPARSER_H
#define STATICPARSER_H

#define NUM_PARSER_TEXT 25
const char *textParser[NUM_PARSER_TEXT] = {
	"You should press ALT+F4 or click on Game/Exit.",
	"You are in a maze of\ntwisty little paths,\nwhich are all alike!",
	"There's no point!",
	"I don't fully understand.",
	"I don't quite understand.",
	"Eh?",
	"You see nothing\nunusual about it.",
	"You already have it.",
	"It is of no use to you.",
	"You don't have it.",
	"No! You'll be needing it.",
	"Ok.",
	"You don't have any!",
	"There aren't any!",
	"I don't see any here!",
	"You're not close enough!",
	"You are carrying:",
	"\nPress ESCAPE to continue",
	"I see nothing special about it",
	"You don't have it!",
	"I don't see it anywhere",
	"Are you sure you want to QUIT?",
	"Apparently our hero either doesn't\nunderstand what you mean or doesn't\nthink that would be very useful!",
	"I find that befuddling!",
	"I don't think that would\naccomplish much, somehow!"
};

#endif //STATICPARSER_H
