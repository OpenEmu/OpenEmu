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


#include "queen/sound.h"

namespace Queen {

const SongData Sound::_songDemo[] = {
	/* 1 - Hotel Gangsters */
	{ { 1, 0 }, 128, 128, 128, 1, 0 },

	/* 2 - Hotel General */
	{ { 2, 0 }, 128, 128, 128, 1, 0 },

	/* 3 - Jungle */
	{ { 3, 4, 5, 6, 0 }, 128, 128, 128, 1, 0 },

	/* 4 - Waterfall On */
	{ { 7, 0 }, 128, 128, 128, 0, 0 },

	/* 5 - Vnormal */
	{ { 8, 0 }, 128, 128, 128, 2, 0 },

	/* 6 - Trader Bob */
	{ { 9, 0 }, 120, 128, 128, 1, 0 },

	/* 7 - Jetty Music */
	{ { 10, 0 }, 128, 128, 128, 1, 0 },

	/* 8 - Ferry Music */
	{ { 11, 0 }, 128, 128, 128, 1, 0 },

	/* 9 - Temple Upstairs */
	{ { 12, 0 }, 128, 128, 128, 1, 0 },

	/* 10 - Temple Downstairs */
	{ { 13, 0 }, 128, 128, 128, 1, 0 },

	/* 11 - Temple Maze */
	{ { 14, 0 }, 128, 128, 128, 1, 0 },

	/* 12 - Temple Skull */
	{ { 15, 0 }, 128, 128, 128, 1, 0 },

	/* 13 - Johns Theme (Love Story) */
	{ { 16, 0 }, 128, 128, 128, 1, 0 },

	/* 14 - Vmedium */
	{ { 17, 0 }, 128, 128, 0, 2, 0 },

	/* 15 - Vsoft */
	{ { 18, 0 }, 128, 128, 0, 2, 0 },

	/* 16 - Floda Upstairs */
	{ { 19, 0 }, 128, 128, 0, 1, 0 },

	/* 17 - Floda General */
	{ { 20, 0 }, 128, 128, 128, 1, 0 },

	/* 18 - Klunks Room */
	{ { 21, 0 }, 110, 128, 128, 1, 0 },

	/* 19 - Hotel Lola */
	{ { 22, 0 }, 120, 18128, 128, 1, 0 },

	/* 20 - Hotel Escape 1 */
	{ { 23, 0 }, 128, 18128, 128, 1, 0 },

	/* 21 - Amazon Fortress */
	{ { 24, 0 }, 128, 128, 128, 1, 0 },

	/* 22 - Waterfall Off */
	{ { 25, 0 }, 128, 128, 128, 0, 0 },

	/* 23 - Wave Torch */
	{ { 26, 0 }, 128, 128, 128, 0, 1 },

	/* 24 - Zombies Rez Out */
	{ { 27, 0 }, 128, 128, 128, 0, 1 },

	/* 25 - Open Door (standard) */
	{ { 28, 0 }, 128, 128, 128, 0, 1 },

	/* 26 - Close Door (standard) */
	{ { 29, 0 }, 128, 128, 128, 0, 1 },

	/* 27 - Cloth Unrolls */
	{ { 30, 0 }, 128, 128, 128, 0, 1 },

	/* 28 - Snake Slithers Off */
	{ { 31, 0 }, 128, 128, 128, 0, 1 },

	/* 29 - Hotel Fanfare 2 */
	{ { 32, 0 }, 128, 128, 128, 1, 1 },

	/* 30 - Floda Secret */
	{ { 33, 0 }, 120, 128, 128, 1, 0 },

	/* 31 - Temple Fanfare 1 */
	{ { 34, 0 }, 128, 128, 128, 1, 1 },
};

const SongData Sound::_song[] = {
	/* 1 - Hotel Gangsters */
	{ { 1, 0 }, 128, 180, 0, 1, 0 },

	/* 2 - Hotel General */
	{ { 2, 0 }, 128, 180, 0, 1, 0 },

	/* 3 - Jungle */
	{ { 3, 4, 5, 6, 0 }, 128, 0, 0, 1, 0 },

	/* 4 - Waterfall On */
	{ { 7, 0 }, 128, 0, 0, 0, 0 },

	/* 5 - Vnormal */
	{ { 8, 0 }, 128, 0, 0, 2, 0 },

	/* 6 - Trader Bob */
	{ { 9, 0 }, 120, 0, 0, 1, 0 },

	/* 7 - Jetty Music */
	{ { 10, 0 }, 128, 0, 0, 1, 0 },

	/* 8 - Ferry Music */
	{ { 11, 0 }, 128, 0, 0, 1, 0 },

	/* 9 - Temple Upstairs */
	{ { 12, 0 }, 128, 0, 0, 1, 0 },

	/* 10 - Temple Downstairs */
	{ { 13, 0 }, 128, 0, 0, 1, 0 },

	/* 11 - Temple Maze */
	{ { 14, 0 }, 128, 0, 0, 1, 0 },

	/* 12 - Temple Skull */
	{ { 15, 0 }, 128, 0, 0, 1, 0 },

	/* 13 - Johns Theme (Love Story) */
	{ { 16, 0 }, 128, 0, 0, 1, 0 },

	/* 14 - Vmedium */
	{ { 17, 0 }, 120, 0, 0, 2, 0 },

	/* 15 - Vsoft */
	{ { 18, 0 }, 110, 0, 0, 2, 0 },

	/* 16 - Floda Upstairs */
	{ { 19, 0 }, 110, 0, 0, 1, 0 },

	/* 17 - Floda General */
	{ { 20, 0 }, 128, 0, 0, 1, 0 },

	/* 18 - Klunks Room */
	{ { 21, 0 }, 128, 0, 0, 1, 0 },

	/* 19 - Hotel Lola */
	{ { 22, 0 }, 120, 180, 0, 1, 0 },

	/* 20 - Hotel Escape 1 */
	{ { 23, 0 }, 128, 180, 0, 1, 0 },

	/* 21 - Amazon Fortress */
	{ { 24, 0 }, 128, 0, 0, 1, 0 },

	/* 22 - Waterfall Off */
	{ { 25, 0 }, 128, 0, 0, 0, 0 },

	/* 23 - Wave Torch */
	{ { 26, 0 }, 128, 0, 0, 0, 1 },

	/* 24 - Zombies Rez Out */
	{ { 27, 0 }, 128, 0, 0, 0, 1 },

	/* 25 - Open Door (standard) */
	{ { 28, 0 }, 128, 0, 0, 0, 1 },

	/* 26 - Close Door (standard) */
	{ { 29, 0 }, 128, 0, 0, 0, 1 },

	/* 27 - Cloth Unrolls */
	{ { 30, 0 }, 128, 0, 0, 0, 1 },

	/* 28 - Snake Slithers Off */
	{ { 31, 0 }, 128, 0, 0, 0, 1 },

	/* 29 - Hotel Fanfare 2 */
	{ { 32, 0 }, 128, 0, 0, 1, 1 },

	/* 30 - Floda Secret */
	{ { 33, 0 }, 128, 0, 0, 1, 0 },

	/* 31 - Temple Fanfare 1 */
	{ { 34, 0 }, 128, 0, 0, 1, 1 },

	/* 32 - Commander Rocket 1 */
	{ { 35, 0 }, 128, 0, 0, 1, 0 },

	/* 33 - Hotel Escape 2 */
	{ { 36, 0 }, 128, 180, 0, 1, 0 },

	/* 34 - Back of Truck */
	{ { 37, 0 }, 128, 180, 0, 1, 0 },

	/* 35 - Hotel Fanfare 1 */
	{ { 38, 0 }, 128, 0, 0, 1, 1 },

	/* 36 - Truck Fanfare */
	{ { 39, 0 }, 128, 0, 0, 1, 1 },

	/* 37 - Airport */
	{ { 40, 0 }, 128, 0, 0, 1, 0 },

	/* 38 - Plane Leaves */
	{ { 41, 0 }, 128, 0, 0, 1, 1 },

	/* 39 - Arrive Hotel */
	{ { 42, 0 }, 128, 0, 0, 1, 0 },

	/* 40 - Jungle Fanfare */
	{ { 43, 0 }, 128, 0, 0, 1, 1 },

	/* 41 - General Fanfare */
	{ { 44, 0 }, 128, 0, 0, 1, 1 },

	/* 42 - Johns Room */
	{ { 45, 0 }, 128, 0, 0, 1, 0 },

	/* 43 - Floda Lab */
	{ { 46, 0 }, 128, 0, 0, 1, 0 },

	/* 44 - Azura's Theme */
	{ { 47, 0 }, 128, 0, 0, 1, 0 },

	/* 45 - Use Record */
	{ { 48, 0 }, 128, 0, 0, 1, 1 },

	/* 46 - Franks Theme */
	{ { 49, 0 }, 128, 0, 0, 1, 0 },

	/* 47 - Anderson Doubts */
	{ { 50, 0 }, 128, 0, 0, 1, 0 },

	/* 48 - Bud and Lou Theme */
	{ { 51, 0 }, 128, 0, 0, 1, 1 },

	/* 49 - Gorilla Theme */
	{ { 52, 0 }, 128, 0, 0, 1, 1 },

	/* 50 - Missionaries Theme */
	{ { 53, 0 }, 128, 0, 0, 1, 1 },

	/* 51 - Sloth Theme */
	{ { 54, 0 }, 128, 0, 0, 1, 1 },

	/* 52 - Amazon Dungeon */
	{ { 55, 0 }, 128, 0, 0, 1, 0 },

	/* 53 - Throne Room */
	{ { 56, 0 }, 128, 0, 0, 1, 0 },

	/* 54 - Temple Puzzle */
	{ { 57, 0 }, 128, 0, 0, 1, 0 },

	/* 55 - Temple Fountain Room */
	{ { 58, 0 }, 128, 0, 0, 1, 0 },

	/* 56 - Light Switch */
	{ { 59, 0 }, 128, 0, 0, 0, 1 },

	/* 57 - Hydraulic Open */
	{ { 60, 0 }, 128, 0, 0, 0, 1 },

	/* 58 - Hydraulic Close */
	{ { 61, 0 }, 128, 0, 0, 0, 1 },

	/* 59 - Close Door (metal) */
	{ { 62, 0 }, 128, 0, 0, 0, 1 },

	/* 60 - Small Hatch Close */
	{ { 63, 0 }, 128, 0, 0, 0, 1 },

	/* 61 - Scissors Snip */
	{ { 64, 0 }, 128, 0, 0, 0, 1 },

	/* 62 - Pick up Sticky */
	{ { 65, 0 }, 128, 0, 0, 0, 1 },

	/* 63 - Oracle Rezzes In */
	{ { 66, 0 }, 128, 0, 0, 0, 1 },

	/* 64 - Sparkle SFX */
	{ { 67, 0 }, 128, 0, 0, 0, 1 },

	/* 65 - Splorch! */
	{ { 68, 0 }, 128, 0, 0, 0, 1 },

	/* 66 - Pour Liquid */
	{ { 69, 0 }, 128, 0, 0, 0, 1 },

	/* 67 - End Credit Medley */
	{ { 70, 0 }, 128, 0, 0, 1, 0 },

	/* 68 - Dino Ray */
	{ { 71, 0 }, 128, 0, 0, 0, 1 },

	/* 69 - Squish! */
	{ { 72, 0 }, 128, 0, 0, 0, 1 },

	/* 70 - Robot Laser */
	{ { 73, 0 }, 128, 0, 0, 0, 1 },

	/* 71 - Thud wood light */
	{ { 74, 0 }, 128, 0, 0, 0, 1 },

	/* 72 - Thud wood deep */
	{ { 75, 0 }, 128, 0, 0, 0, 1 },

	/* 73 - Thud metallic */
	{ { 76, 0 }, 128, 0, 0, 0, 1 },

	/* 74 - Cut Coconut */
	{ { 77, 0 }, 128, 0, 0, 0, 1 },

	/* 75 - Thud Stone */
	{ { 78, 0 }, 128, 0, 0, 0, 1 },

	/* 76 - Cloth Slide 1 */
	{ { 79, 0 }, 128, 0, 0, 0, 1 },

	/* 77 - Open Chest */
	{ { 80, 0 }, 128, 0, 0, 0, 1 },

	/* 78 - Close Chest */
	{ { 81, 0 }, 128, 0, 0, 0, 1 },

	/* 79 - Open Drawer */
	{ { 82, 0 }, 128, 0, 0, 0, 1 },

	/* 80 - Truck door closes */
	{ { 83, 0 }, 128, 0, 0, 0, 1 },

	/* 81 - Truck Starts */
	{ { 84, 0 }, 128, 0, 0, 0, 1 },

	/* 82 - Truck Drives Off */
	{ { 85, 0 }, 128, 0, 0, 0, 1 },

	/* 83 - Fish Splash */
	{ { 86, 0 }, 128, 0, 0, 0, 1 },

	/* 84 - Close Drawer/Push Ladder */
	{ { 87, 0 }, 128, 0, 0, 0, 1 },

	/* 85 - Agression Enhancer */
	{ { 88, 0 }, 128, 0, 0, 0, 1 },

	/* 86 - Stone Door Grind 1 */
	{ { 89, 0 }, 128, 0, 0, 0, 1 },

	/* 87 - Prequel 1 */
	{ { 90, 0 }, 128, 0, 0, 1, 0 },

	/* 88 - Intro Credits */
	{ { 91, 0 }, 128, 0, 0, 1, 0 },

	/* 89 - Valley 1 */
	{ { 92, 0 }, 128, 0, 0, 1, 0 },

	/* 90 - Valley 3 */
	{ { 93, 0 }, 128, 0, 0, 1, 0 },

	/* 91 - Fight Music */
	{ { 94, 0 }, 128, 0, 0, 1, 0 },

	/* 92 - Confrontation 1 */
	{ { 95, 0 }, 128, 0, 0, 1, 0 },

	/* 93 - Confrontation 2 */
	{ { 96, 0 }, 128, 0, 0, 1, 0 },

	/* 94 - Plane Hatch Open */
	{ { 97, 0 }, 128, 128, 128, 0, 1 },

	/* 95 - Plane Hatch Close */
	{ { 98, 0 }, 128, 128, 128, 0, 1 },

	/* 96 - Tie Vines */
	{ { 99, 0 }, 128, 128, 128, 0, 1 },

	/* 97 - Pterodactyl */
	{ { 100, 0 }, 128, 128, 128, 0, 1 },

	/* 98 - Beef Jerky Splash */
	{ { 101, 0 }, 128, 128, 128, 0, 1 },

	/* 99 - Piranha Burp */
	{ { 102, 0 }, 128, 128, 128, 0, 1 },

	/* 100 - Falling Vine */
	{ { 103, 0 }, 128, 128, 128, 0, 1 },

	/* 101 - Stone Door Grind 2 */
	{ { 104, 0 }, 128, 128, 128, 0, 1 },

	/* 102 - Stone Grind (light) */
	{ { 105, 0 }, 128, 128, 128, 0, 1 },

	/* 103 - Ape Takes Off Mask */
	{ { 106, 0 }, 128, 128, 128, 0, 1 },

	/* 104 - Bark Breaks */
	{ { 107, 0 }, 128, 128, 128, 0, 1 },

	/* 105 - Stone Click */
	{ { 108, 0 }, 128, 128, 128, 0, 1 },

	/* 106 - Sproing! */
	{ { 109, 0 }, 128, 128, 128, 0, 1 },

	/* 107 - Cash Register */
	{ { 110, 0 }, 128, 128, 128, 0, 1 },

	/* 108 - Squeaky Toy */
	{ { 111, 0 }, 128, 128, 128, 0, 1 },

	/* 109 - Falling Chains */
	{ { 112, 0 }, 128, 128, 128, 0, 1 },

	/* 110 - Open Locker Door */
	{ { 113, 0 }, 128, 128, 128, 0, 1 },

	/* 111 - Close Locker Door */
	{ { 114, 0 }, 128, 128, 128, 0, 1 },

	/* 112 - Rub Pencil */
	{ { 115, 0 }, 128, 128, 128, 0, 1 },

	/* 113 - Open Safe */
	{ { 116, 0 }, 128, 128, 128, 0, 1 },

	/* 114 - Close Safe */
	{ { 117, 0 }, 128, 128, 128, 0, 1 },

	/* 115 - Push Chair */
	{ { 118, 0 }, 128, 128, 128, 0, 1 },

	/* 116 - Snake Hiss */
	{ { 119, 0 }, 128, 128, 128, 0, 1 },

	/* 117 - Oracle Rezzes Out */
	{ { 120, 0 }, 128, 128, 128, 0, 1 },

	/* 118 - Wall Crumbles */
	{ { 121, 0 }, 128, 128, 128, 0, 1 },

	/* 119 - Crypt Crumbles */
	{ { 122, 0 }, 128, 128, 128, 0, 1 },

	/* 120 - Joe Sucked Up */
	{ { 123, 0 }, 128, 128, 128, 0, 1 },

	/* 121 - Rocket Pack Zoom */
	{ { 124, 0 }, 128, 128, 128, 0, 1 },

	/* 122 - Piranha Splash */
	{ { 125, 0 }, 128, 128, 128, 0, 1 },

	/* 123 - Snap Branch */
	{ { 126, 0 }, 128, 128, 128, 0, 1 },

	/* 124 - Dino Horn */
	{ { 127, 0 }, 128, 128, 128, 0, 1 },

	/* 125 - Tire Screech */
	{ { 128, 0 }, 128, 128, 128, 0, 1 },

	/* 126 - Oil Splat */
	{ { 129, 0 }, 128, 128, 128, 0, 1 },

	/* 127 - Punch */
	{ { 130, 0 }, 128, 128, 128, 0, 1 },

	/* 128 - Body Hits Ground */
	{ { 131, 0 }, 128, 128, 128, 0, 1 },

	/* 129 - Chicken */
	{ { 132, 0 }, 128, 128, 128, 0, 1 },

	/* 130 - Open Sarcophagus */
	{ { 133, 0 }, 128, 128, 128, 0, 1 },

	/* 131 - Close Sarcophagus */
	{ { 134, 0 }, 128, 128, 128, 0, 1 },

	/* 132 - Creaking Stick */
	{ { 135, 0 }, 128, 128, 128, 0, 1 },

	/* 133 - Pick Hits Stone */
	{ { 136, 0 }, 128, 128, 128, 0, 1 },

	/* 134 - Stalactite Crumbles */
	{ { 137, 0 }, 128, 128, 128, 0, 1 },

	/* 135 - Tic-Toc */
	{ { 138, 0 }, 128, 128, 128, 0, 1 },

	/* 136 - Stone Grind (heavy) */
	{ { 139, 0 }, 128, 128, 128, 0, 1 },

	/* 137 - Explosion */
	{ { 140, 0 }, 128, 128, 128, 0, 1 },

	/* 138 - Cloth Slide 2 */
	{ { 141, 0 }, 128, 128, 128, 0, 1 },

	/* 139 - Temple Laser */
	{ { 142, 0 }, 128, 128, 128, 0, 1 },

	/* 140 - Dino Transformation */
	{ { 143, 0 }, 128, 128, 128, 0, 1 },

	/* 141 - Experimental Laser */
	{ { 144, 0 }, 128, 128, 128, 0, 1 },

	/* 142 - Stone Grind (medium) */
	{ { 145, 0 }, 128, 128, 128, 0, 1 },

	/* 143 - Weeping God Grind */
	{ { 146, 0 }, 128, 128, 128, 0, 1 },

	/* 144 - Alien Hum */
	{ { 147, 0 }, 128, 128, 128, 0, 1 },

	/* 145 - Alien Puzzle */
	{ { 148, 0 }, 128, 128, 128, 0, 1 },

	/* 146 - Vacuum On */
	{ { 149, 0 }, 128, 128, 128, 0, 1 },

	/* 147 - Vacuum Off */
	{ { 150, 0 }, 128, 128, 128, 0, 1 },

	/* 148 - Elevator Starts */
	{ { 151, 0 }, 128, 128, 128, 0, 1 },

	/* 149 - Mummy Crumbles */
	{ { 152, 0 }, 128, 128, 128, 0, 1 },

	/* 150 - Temple Green Circle */
	{ { 153, 0 }, 128, 128, 128, 0, 1 },

	/* 151 - Rattle Bars */
	{ { 154, 0 }, 128, 128, 128, 0, 1 },

	/* 152 - Door Dissolves */
	{ { 155, 0 }, 128, 128, 128, 0, 1 },

	/* 153 - Altar Slides */
	{ { 156, 0 }, 128, 128, 128, 0, 1 },

	/* 154 - Light Torch */
	{ { 157, 0 }, 128, 128, 128, 0, 1 },

	/* 155 - Stamp Sound */
	{ { 158, 0 }, 128, 128, 128, 0, 1 },

	/* 156 - Plaster Loud */
	{ { 159, 0 }, 128, 128, 128, 0, 1 },

	/* 157 - Sparky Bathtub */
	{ { 160, 0 }, 128, 128, 128, 0, 1 },

	/* 158 - Ape Rezzes Out */
	{ { 161, 0 }, 128, 128, 128, 0, 1 },

	/* 159 - Song  159 */
	{ { 162, 0 }, 128, 128, 128, 1, 0 },

	/* 160 - Song  160 */
	{ { 163, 0 }, 128, 128, 128, 1, 0 },

	/* 161 - Song  161 */
	{ { 164, 0 }, 128, 128, 128, 1, 0 },

	/* 162 - Piranhas Swim */
	{ { 165, 0 }, 128, 128, 128, 0, 1 },

	/* 163 - Prison/Dungeon Door */
	{ { 166, 0 }, 128, 128, 128, 0, 1 },

	/* 164 - Fight Explosion */
	{ { 167, 0 }, 128, 128, 128, 0, 1 },

	/* 165 - Press Button  */
	{ { 168, 0 }, 128, 128, 128, 2, 1 },

	/* 166 - Pull Lever */
	{ { 169, 0 }, 128, 128, 128, 0, 1 },

	/* 167 - Wrong Code */
	{ { 170, 0 }, 128, 128, 128, 0, 1 },

	/* 168 - Correct Code */
	{ { 171, 0 }, 128, 128, 128, 0, 1 },

	/* 169 - Sizzle */
	{ { 172, 0 }, 128, 128, 128, 0, 1 },

	/* 170 - Money In Slot */
	{ { 173, 0 }, 128, 128, 128, 0, 1 },

	/* 171 - Lightning Crack */
	{ { 174, 0 }, 128, 128, 128, 0, 1 },

	/* 172 - Machine Gun Fire */
	{ { 175, 0 }, 128, 128, 128, 0, 1 },

	/* 173 - Cage Descends */
	{ { 176, 0 }, 128, 128, 128, 0, 1 },

	/* 174 - Chair Activates */
	{ { 177, 0 }, 128, 128, 128, 0, 1 },

	/* 175 - Robot Powers On */
	{ { 178, 0 }, 128, 128, 128, 0, 1 },

	/* 176 - Grow Big */
	{ { 179, 0 }, 128, 128, 128, 0, 1 },

	/* 177 - Eat Food */
	{ { 180, 0 }, 128, 128, 128, 0, 1 },

	/* 178 - Head Shrink */
	{ { 181, 0 }, 128, 128, 128, 0, 1 },

	/* 179 - Grinding Gears */
	{ { 182, 0 }, 128, 128, 128, 0, 1 },

	/* 180 - Chair Splash */
	{ { 183, 0 }, 128, 128, 128, 0, 1 },

	/* 181 - Deflect Laser */
	{ { 184, 0 }, 128, 128, 128, 0, 1 },

	/* 182 - Zap Frank */
	{ { 185, 0 }, 128, 128, 128, 0, 1 },

	/* 183 - Frank Transforms */
	{ { 186, 0 }, 128, 128, 128, 0, 1 },

	/* 184 - Alarm Clock */
	{ { 187, 0 }, 128, 128, 128, 0, 1 },

	/* 185 - Slide Chute */
	{ { 188, 0 }, 128, 128, 128, 0, 1 },

	/* 186 - Puff */
	{ { 189, 0 }, 128, 128, 128, 0, 1 },

	/* 187 - Bite */
	{ { 190, 0 }, 128, 128, 128, 0, 0 },

	/* 188 - Stone Door Grind 2 */
	{ { 191, 0 }, 128, 128, 128, 0, 1 },

	/* 189 - Prequel 2 */
	{ { 192, 0 }, 128, 128, 128, 1, 0 },

	/* 190 - Prequel 3 */
	{ { 193, 0 }, 128, 128, 128, 1, 0 },

	/* 191 - Prequel 4 */
	{ { 194, 0 }, 128, 128, 128, 1, 0 },

	/* 192 - Stop Music */
	{ { 195, 0 }, 128, 128, 128, 1, 0 },

	/* 193 - Plane Flyby */
	{ { 196, 0 }, 128, 128, 128, 0, 1 },

	/* 194 - Commander Rocket 2 */
	{ { 197, 0 }, 128, 128, 128, 1, 0 },

	/* 195 - Commander Rocket 3 */
	{ { 198, 0 }, 128, 128, 128, 1, 0 },

	/* 196 - Rescue */
	{ { 199, 0 }, 128, 128, 128, 1, 0 },

	/* 197 - Slow Fanfare */
	{ { 200, 0 }, 128, 128, 128, 1, 0 },

	/* 198 - Plane Crash */
	{ { 201, 0 }, 128, 128, 128, 1, 0 },

	/* 199 - Plane Engine 1 */
	{ { 202, 0 }, 128, 128, 128, 0, 1 },

	/* 200 - Plane Engine 2 */
	{ { 203, 0 }, 128, 128, 128, 0, 1 },

	/* 201 - Boat In */
	{ { 204, 0 }, 128, 128, 128, 0, 1 },

	/* 202 - Boat Out */
	{ { 205, 0 }, 128, 128, 128, 0, 1 },

	/* 203 - Final Fanfare! */
	{ { 206, 0 }, 128, 128, 128, 1, 0 },

	/* 204 - Frank Destroyed */
	{ { 207, 0 }, 128, 128, 128, 1, 0 },

	/* 205 - Jaspar Eats */
	{ { 208, 0 }, 128, 128, 128, 0, 1 },

	/* 206 - Compy Scream 1 */
	{ { 209, 0 }, 128, 128, 128, 0, 1 },

	/* 207 - Compy Scream 2 */
	{ { 210, 0 }, 128, 128, 128, 0, 1 },

	/* 208 - Punch Klunk Fanfare */
	{ { 211, 0 }, 128, 128, 128, 1, 0 },

	/* 209 - Talk Frank */
	{ { 212, 0 }, 128, 128, 128, 1, 0 }
};

const TuneData Sound::_tuneDemo[] = {
	/* 1 - Hotel Gangsters */
	{ { 32, 0 }, { 0, 0 }, 1, 0 },

	/* 2 - Hotel General */
	{ { 26, 0 }, { 0, 0 }, 1, 0 },

	/* 3 - Jungle */
	{ { 15, 0 }, { 0, 0 }, 1, 0 },

	/* 4 - Jungle */
	{ { 17, 0 }, { 0, 0 }, 1, 0 },

	/* 5 - Jungle */
	{ { 18, 0 }, { 0, 0 }, 1, 0 },

	/* 6 - Jungle */
	{ { 7, 8, 9, 10, 11, 12, 13, 14, 0 }, { 0, 0 }, 0, 0 },

	/* 7 - Waterfall On */
	{ { 3, 0 }, { 0, 0 }, 1, 0 },

	/* 8 - Vnormal */
	{ { 1, 0 }, { 0, 0 }, 1, 0 },

	/* 9 - Trader Bob */
	{ { 1, 0 }, { 0, 0 }, 1, 0 },

	/* 10 - Jetty Music */
	{ { 37, 0 }, { 0, 0 }, 1, 0 },

	/* 11 - Ferry Music */
	{ { 38, 0 }, { 0, 0 }, 1, 0 },

	/* 12 - Temple Upstairs */
	{ { 30, 0 }, { 0, 0 }, 1, 0 },

	/* 13 - Temple Downstairs */
	{ { 34, 0 }, { 0, 0 }, 1, 0 },

	/* 14 - Temple Maze */
	{ { 35, 0 }, { 0, 0 }, 1, 0 },

	/* 15 - Temple Skull */
	{ { 36, 0 }, { 0, 0 }, 1, 0 },

	/* 16 - Johns Theme (Love Story) */
	{ { 43, 0 }, { 0, 0 }, 1, 0 },

	/* 17 - Vmedium */
	{ { 28, 0 }, { 0, 0 }, 1, 0 },

	/* 18 - Vsoft */
	{ { 28, 0 }, { 0, 0 }, 1, 0 },

	/* 19 - Floda Upstairs */
	{ { 28, 0 }, { 0, 0 }, 1, 0 },

	/* 20 - Floda General */
	{ { 29, 0 }, { 0, 0 }, 1, 0 },

	/* 21 - Klunks Room */
	{ { 39, 0 }, { 0, 0 }, 1, 0 },

	/* 22 - Hotel Lola */
	{ { 31, 0 }, { 0, 0 }, 1, 0 },

	/* 23 - Hotel Escape 1 */
	{ { 33, 0 }, { 0, 0 }, 1, 0 },

	/* 24 - Amazon Fortress */
	{ { 40, 0 }, { 0, 0 }, 1, 0 },

	/* 25 - Waterfall Off */
	{ { -3, 0 }, { 0, 0 }, 1, 0 },

	/* 26 - Wave Torch */
	{ { 22, 0 }, { 121, 0 }, 2, 0 },

	/* 27 - Zombies Rez Out */
	{ { 25, 0 }, { 20, 0 }, 2, 0 },

	/* 28 - Open Door (standard) */
	{ { 20, 0 }, { 1, 0 }, 2, 0 },

	/* 29 - Close Door (standard) */
	{ { 21, 0 }, { 2, 0 }, 2, 0 },

	/* 30 - Cloth Unrolls */
	{ { 23, 0 }, { 51, 0 }, 2, 0 },

	/* 31 - Snake Slithers Off */
	{ { 24, 0 }, { 122, 0 }, 2, 0 },

	/* 32 - Hotel Fanfare 2 */
	{ { 69, 1003, 0 }, { 0, 0 }, 2, 0 },

	/* 33 - Floda Secret */
	{ { 28, 0 }, { 0, 0 }, 1, 0 },

	/* 34 - Temple Fanfare 1 */
	{ { 60, 162, 0 }, { 0, 0 }, 2, 0 },

	/* 35 - Commander Rocket 1 */
	{ { 46, 0 }, { 0, 0 }, 1, 0 },
};

const TuneData Sound::_tune[] = {
	/* 1 - Hotel Gangsters */
	{ { 32, 0 }, { 0, 0 }, 1, 0 },

	/* 2 - Hotel General */
	{ { 41, 0 }, { 0, 0 }, 1, 0 },

	/* 3 - Jungle */
	{ { 15, 16, 0 }, { 0, 0 }, 1, 0 },

	/* 4 - Jungle */
	{ { 17, 0 }, { 0, 0 }, 1, 0 },

	/* 5 - Jungle */
	{ { 18, 0 }, { 0, 0 }, 1, 0 },

	/* 6 - Jungle */
	{ { 7, 8, 9, 10, 11, 12, 13, 14, 0 }, { 0, 0 }, 0, -10 },

	/* 7 - Waterfall On */
	{ { 3, 0 }, { 0, 0 }, 1, 0 },

	/* 8 - Vnormal */
	{ { 23, 0 }, { 0, 0 }, 1, 0 },

	/* 9 - Trader Bob */
	{ { 23, 0 }, { 0, 0 }, 1, 0 },

	/* 10 - Jetty Music */
	{ { 37, 0 }, { 0, 0 }, 1, 0 },

	/* 11 - Ferry Music */
	{ { 38, 0 }, { 0, 0 }, 1, 0 },

	/* 12 - Temple Upstairs */
	{ { 30, 0 }, { 0, 0 }, 1, 0 },

	/* 13 - Temple Downstairs */
	{ { 34, 36, 56, 0 }, { 0, 0 }, 0, 0 },

	/* 14 - Temple Maze */
	{ { 87, 35, 0 }, { 0, 0 }, 1, 0 },

	/* 15 - Temple Skull */
	{ { 76, 0 }, { 0, 0 }, 1, 0 },

	/* 16 - Johns Theme (Love Story) */
	{ { 44, 0 }, { 0, 0 }, 1, 0 },

	/* 17 - Vmedium */
	{ { 28, 0 }, { 0, 0 }, 1, 0 },

	/* 18 - Vsoft */
	{ { 28, 0 }, { 0, 0 }, 1, 0 },

	/* 19 - Floda Upstairs */
	{ { 28, 39, 0 }, { 0, 0 }, 0, 0 },

	/* 20 - Floda General */
	{ { 89, 63, 64, 65, 0 }, { 0, 0 }, 0, 0 },

	/* 21 - Klunks Room */
	{ { 43, 0 }, { 0, 0 }, 1, 0 },

	/* 22 - Hotel Lola */
	{ { 31, 0 }, { 0, 0 }, 1, 0 },

	/* 23 - Hotel Escape 1 */
	{ { 52, 0 }, { 0, 0 }, 1, 0 },

	/* 24 - Amazon Fortress */
	{ { 40, 0 }, { 0, 0 }, 1, 0 },

	/* 25 - Waterfall Off */
	{ {-3, 0 }, { 0, 0 }, 1, 0 },

	/* 26 - Wave Torch */
	{ { 0, 0 }, { 121, 0 }, 2, 0 },

	/* 27 - Zombies Rez Out */
	{ { 0, 0 }, { 20, 0 }, 2, 0 },

	/* 28 - Open Door (standard) */
	{ { 0, 0 }, { 1, 0 }, 2, 0 },

	/* 29 - Close Door (standard) */
	{ { 0, 0 }, { 2, 0 }, 2, 0 },

	/* 30 - Cloth Unrolls */
	{ { 0, 0 }, { 51, 0 }, 2, 0 },

	/* 31 - Snake Slithers Off */
	{ { 0, 0 }, { 122, 0 }, 2, 0 },

	/* 32 - Hotel Fanfare 2 */
	{ { 69, 2000, 0 }, { 0, 0 }, 2, 0 },

	/* 33 - Floda Secret */
	{ { 29, 42, 0 }, { 0, 0 }, 1, 0 },

	/* 34 - Temple Fanfare 1 */
	{ { 70, 2000, 0 }, { 0, 0 }, 2, 0 },

	/* 35 - Commander Rocket 1 */
	{ { 45, 0 }, { 0, 0 }, 2, 0 },

	/* 36 - Hotel Escape 2 */
	{ { 52, 0 }, { 0, 0 }, 1, 0 },

	/* 37 - Back of Truck */
	{ { 51, 48, 33, 54, 52, 0 }, { 0, 0 }, 1, 0 },

	/* 38 - Hotel Fanfare 1 */
	{ { 67, 2000, 0 }, { 0, 0 }, 2, 0 },

	/* 39 - Truck Fanfare */
	{ { 67, 177, 0 }, { 0, 0 }, 2, 0 },

	/* 40 - Airport */
	{ { 81, 0 }, { 0, 0 }, 1, 0 },

	/* 41 - Plane Leaves */
	{ { 68, 1198, 0 }, { 0, 0 }, 2, 0 },

	/* 42 - Arrive Hotel */
	{ { 26, 0 }, { 0, 0 }, 1, 0 },

	/* 43 - Jungle Fanfare */
	{ { 68, 2000, 0 }, { 0, 0 }, 2, 0 },

	/* 44 - General Fanfare */
	{ { 57, 2000, 0 }, { 0, 0 }, 2, 0 },

	/* 45 - Johns Room */
	{ { 90, 0 }, { 0, 0 }, 1, 0 },

	/* 46 - Floda Lab */
	{ { 92, 0 }, { 0, 0 }, 1, 0 },

	/* 47 - Azura's Theme */
	{ { 80, 0 }, { 0, 0 }, 1, 0 },

	/* 48 - Use Record */
	{ { 91, 2000, 0 }, { 0, 0 }, 2, 0 },

	/* 49 - Franks Theme */
	{ { 77, 0 }, { 0, 0 }, 1, 0 },

	/* 50 - Anderson Doubts */
	{ { 75, 0 }, { 0, 0 }, 1, 0 },

	/* 51 - Bud and Lou Theme */
	{ { 94, 1003, 0 }, { 0, 0 }, 1, 0 },

	/* 52 - Gorilla Theme */
	{ { 97, 1003, 0 }, { 0, 0 }, 1, 0 },

	/* 53 - Missionaries Theme */
	{ { 98, 1003, 0 }, { 0, 0 }, 1, 0 },

	/* 54 - Sloth Theme */
	{ { 100, 1003, 0 }, { 0, 0 }, 1, 0 },

	/* 55 - Amazon Dungeon */
	{ { 96, 0 }, { 0, 0 }, 1, 0 },

	/* 56 - Throne Room */
	{ { 78, 0 }, { 0, 0 }, 1, 0 },

	/* 57 - Temple Puzzle */
	{ { 88, 0 }, { 0, 0 }, 1, 0 },

	/* 58 - Temple Fountain Room */
	{ { 55, 0 }, { 0, 0 }, 1, 0 },

	/* 59 - Light Switch */
	{ { 0, 0 }, { 4, 0 }, 2, 0 },

	/* 60 - Hydraulic Open */
	{ { 0, 0 }, { 7, 0 }, 2, 0 },

	/* 61 - Hydraulic Close */
	{ { 0, 0 }, { 8, 0 }, 2, 0 },

	/* 62 - Close Door (metal) */
	{ { 0, 0 }, { 9, 0 }, 2, 0 },

	/* 63 - Small Hatch Close */
	{ { 0, 0 }, { 10, 0 }, 2, 0 },

	/* 64 - Scissors Snip */
	{ { 0, 0 }, { 5, 0 }, 2, 0 },

	/* 65 - Pick up Sticky */
	{ { 0, 0 }, { 6, 0 }, 2, 0 },

	/* 66 - Oracle Rezzes In */
	{ { 0, 0 }, { 11, 0 }, 2, 0 },

	/* 67 - Sparkle SFX */
	{ { 0, 0 }, { 12, 0 }, 2, 0 },

	/* 68 - Splorch! */
	{ { 0, 0 }, { 13, 0 }, 2, 0 },

	/* 69 - Pour Liquid */
	{ { 0, 0 }, { 3, 0 }, 2, 0 },

	/* 70 - End Credit Medley */
	{ { 95, 0 }, { 0, 0 }, 1, 0 },

	/* 71 - Dino Ray */
	{ { 0, 0 }, { 14, 0 }, 2, 0 },

	/* 72 - Squish! */
	{ { 0, 0 }, { 15, 0 }, 2, 0 },

	/* 73 - Robot Laser */
	{ { 0, 0 }, { 16, 0 }, 2, 0 },

	/* 74 - Thud wood light */
	{ { 0, 0 }, { 17, 0 }, 2, 0 },

	/* 75 - Thud wood deep */
	{ { 0, 0 }, { 18, 0 }, 2, 0 },

	/* 76 - Thud metallic */
	{ { 0, 0 }, { 19, 0 }, 2, 0 },

	/* 77 - Cut Coconut */
	{ { 0, 0 }, { 22, 0 }, 2, 0 },

	/* 78 - Thud Stone */
	{ { 0, 0 }, { 23, 0 }, 2, 0 },

	/* 79 - Cloth Slide 1 */
	{ { 0, 0 }, { 24, 0 }, 2, 0 },

	/* 80 - Open Chest */
	{ { 0, 0 }, { 25, 0 }, 2, 0 },

	/* 81 - Close Chest */
	{ { 0, 0 }, { 26, 0 }, 2, 0 },

	/* 82 - Open Drawer */
	{ { 0, 0 }, { 27, 0 }, 2, 0 },

	/* 83 - Truck door closes */
	{ { 0, 0 }, { 28, 0 }, 2, 0 },

	/* 84 - Truck Starts */
	{ { 0, 0 }, { 29, 0 }, 2, 0 },

	/* 85 - Truck Drives Off */
	{ { 0, 0 }, { 30, 0 }, 2, 0 },

	/* 86 - Fish Splash */
	{ { 0, 0 }, { 31, 0 }, 2, 0 },

	/* 87 - Close Drawer/Push Ladder */
	{ { 0, 0 }, { 33, 0 }, 2, 0 },

	/* 88 - Agression Enhancer */
	{ { 0, 0 }, { 32, 0 }, 2, 0 },

	/* 89 - Stone Door Grind 1 */
	{ { 0, 0 }, { 78, 0 }, 2, 0 },

	/* 90 - Prequel 1 */
	{ { 20, 0 }, { 0, 0 }, 1, 0 },

	/* 91 - Intro Credits */
	{ { 21, 0 }, { 0, 0 }, 2, 0 },

	/* 92 - Valley 1 */
	{ { 71, 0 }, { 0, 0 }, 1, 0 },

	/* 93 - Valley 3 */
	{ { 73, 0 }, { 0, 0 }, 1, 0 },

	/* 94 - Fight Music */
	{ { 72, 0 }, { 0, 0 }, 1, 0 },

	/* 95 - Confrontation 1 */
	{ { 93, 0 }, { 0, 0 }, 1, 0 },

	/* 96 - Confrontation 2 */
	{ { 74, 0 }, { 0, 0 }, 1, 0 },

	/* 97 - Plane Hatch Open */
	{ { 0, 0 }, { 35, 0 }, 2, 0 },

	/* 98 - Plane Hatch Close */
	{ { 0, 0 }, { 36, 0 }, 2, 0 },

	/* 99 - Tie Vines */
	{ { 0, 0 }, { 37, 0 }, 2, 0 },

	/* 100 - Pterodactyl */
	{ { 0, 0 }, { 38, 0 }, 2, 0 },

	/* 101 - Beef Jerky Splash */
	{ { 0, 0 }, { 39, 0 }, 2, 0 },

	/* 102 - Piranha Burp */
	{ { 0, 0 }, { 40, 0 }, 2, 0 },

	/* 103 - Falling Vine */
	{ { 0, 0 }, { 41, 0 }, 2, 0 },

	/* 104 - Stone Door Grind 2 */
	{ { 0, 0 }, { 79, 0 }, 2, 0 },

	/* 105 - Stone Grind (light) */
	{ { 0, 0 }, { 82, 0 }, 2, 0 },

	/* 106 - Ape Takes Off Mask */
	{ { 0, 0 }, { 44, 0 }, 2, 0 },

	/* 107 - Bark Breaks */
	{ { 0, 0 }, { 45, 0 }, 2, 0 },

	/* 108 - Stone Click */
	{ { 0, 0 }, { 46, 0 }, 2, 0 },

	/* 109 - Sproing! */
	{ { 0, 0 }, { 42, 0 }, 2, 0 },

	/* 110 - Cash Register */
	{ { 0, 0 }, { 48, 0 }, 2, 0 },

	/* 111 - Squeaky Toy */
	{ { 0, 0 }, { 49, 0 }, 2, 0 },

	/* 112 - Falling Chains */
	{ { 0, 0 }, { 50, 0 }, 2, 0 },

	/* 113 - Open Locker Door */
	{ { 0, 0 }, { 52, 0 }, 2, 0 },

	/* 114 - Close Locker Door */
	{ { 0, 0 }, { 53, 0 }, 2, 0 },

	/* 115 - Rub Pencil */
	{ { 0, 0 }, { 54, 0 }, 2, 0 },

	/* 116 - Open Safe */
	{ { 0, 0 }, { 55, 0 }, 2, 0 },

	/* 117 - Close Safe */
	{ { 0, 0 }, { 56, 0 }, 2, 0 },

	/* 118 - Push Chair */
	{ { 0, 0 }, { 57, 0 }, 2, 0 },

	/* 119 - Snake Hiss */
	{ { 0, 0 }, { 58, 0 }, 2, 0 },

	/* 120 - Oracle Rezzes Out */
	{ { 0, 0 }, { 59, 0 }, 2, 0 },

	/* 121 - Wall Crumbles */
	{ { 0, 0 }, { 60, 0 }, 2, 0 },

	/* 122 - Crypt Crumbles */
	{ { 0, 0 }, { 61, 0 }, 2, 0 },

	/* 123 - Joe Sucked Up */
	{ { 0, 0 }, { 63, 0 }, 2, 0 },

	/* 124 - Rocket Pack Zoom */
	{ { 0, 0 }, { 47, 0 }, 2, 0 },

	/* 125 - Piranha Splash */
	{ { 0, 0 }, { 83, 0 }, 2, 0 },

	/* 126 - Snap Branch */
	{ { 0, 0 }, { 66, 0 }, 2, 0 },

	/* 127 - Dino Horn */
	{ { 0, 0 }, { 67, 0 }, 2, 0 },

	/* 128 - Tire Screech */
	{ { 0, 0 }, { 68, 0 }, 2, 0 },

	/* 129 - Oil Splat */
	{ { 0, 0 }, { 70, 0 }, 2, 0 },

	/* 130 - Punch */
	{ { 0, 0 }, { 71, 0 }, 2, 0 },

	/* 131 - Body Hits Ground */
	{ { 0, 0 }, { 72, 0 }, 2, 0 },

	/* 132 - Chicken */
	{ { 0, 0 }, { 69, 0 }, 2, 0 },

	/* 133 - Open Sarcophagus */
	{ { 0, 0 }, { 21, 0 }, 2, 0 },

	/* 134 - Close Sarcophagus */
	{ { 0, 0 }, { 21, 0 }, 2, 0 },

	/* 135 - Creaking Stick */
	{ { 0, 0 }, { 62, 0 }, 2, 0 },

	/* 136 - Pick Hits Stone */
	{ { 0, 0 }, { 73, 0 }, 2, 0 },

	/* 137 - Stalactite Crumbles */
	{ { 0, 0 }, { 74, 0 }, 2, 0 },

	/* 138 - Tic-Toc */
	{ { 0, 0 }, { 76, 0 }, 2, 0 },

	/* 139 - Stone Grind (heavy) */
	{ { 0, 0 }, { 81, 0 }, 2, 0 },

	/* 140 - Explosion */
	{ { 0, 0 }, { 77, 0 }, 2, 0 },

	/* 141 - Cloth Slide 2 */
	{ { 0, 0 }, { 84, 0 }, 2, 0 },

	/* 142 - Temple Laser */
	{ { 0, 0 }, { 85, 0 }, 2, 0 },

	/* 143 - Dino Transformation */
	{ { 0, 0 }, { 86, 0 }, 2, 0 },

	/* 144 - Experimental Laser */
	{ { 0, 0 }, { 87, 0 }, 2, 0 },

	/* 145 - Stone Grind (medium) */
	{ { 0, 0 }, { 88, 0 }, 2, 0 },

	/* 146 - Weeping God Grind */
	{ { 0, 0 }, { 89, 0 }, 2, 0 },

	/* 147 - Alien Hum */
	{ { 0, 0 }, { 90, 0 }, 2, 0 },

	/* 148 - Alien Puzzle */
	{ { 0, 0 }, { 91, 0 }, 2, 0 },

	/* 149 - Vacuum On */
	{ { 0, 0 }, { 92, 0 }, 2, 0 },

	/* 150 - Vacuum Off */
	{ { 0, 0 }, { 93, 0 }, 2, 0 },

	/* 151 - Elevator Starts */
	{ { 0, 0 }, { 94, 0 }, 2, 0 },

	/* 152 - Mummy Crumbles */
	{ { 0, 0 }, { 95, 0 }, 2, 0 },

	/* 153 - Temple Green Circle */
	{ { 0, 0 }, { 96, 0 }, 2, 0 },

	/* 154 - Rattle Bars */
	{ { 0, 0 }, { 97, 0 }, 2, 0 },

	/* 155 - Door Dissolves */
	{ { 0, 0 }, { 98, 0 }, 2, 0 },

	/* 156 - Altar Slides */
	{ { 0, 0 }, { 99, 0 }, 2, 0 },

	/* 157 - Light Torch */
	{ { 0, 0 }, { 100, 0 }, 2, 0 },

	/* 158 - Stamp Sound */
	{ { 0, 0 }, { 34, 0 }, 2, 0 },

	/* 159 - Plaster Loud */
	{ { 0, 0 }, { 102, 0 }, 2, 0 },

	/* 160 - Sparky Bathtub */
	{ { 0, 0 }, { 103, 0 }, 2, 0 },

	/* 161 - Ape Rezzes Out */
	{ { 0, 0 }, { 104, 0 }, 2, 0 },

	/* 162 - Song  159 */
	{ { 0, 0 }, { 0, 0 }, 2, 0 },

	/* 163 - Song  160 */
	{ { 0, 0 }, { 0, 0 }, 2, 0 },

	/* 164 - Song  161 */
	{ { 0, 0 }, { 0, 0 }, 2, 0 },

	/* 165 - Piranhas Swim */
	{ { 0, 0 }, { 105, 0 }, 2, 0 },

	/* 166 - Prison/Dungeon Door */
	{ { 0, 0 }, { 43, 0 }, 2, 0 },

	/* 167 - Fight Explosion */
	{ { 0, 0 }, { 80, 0 }, 2, 0 },

	/* 168 - Press Button  */
	{ { 0, 0 }, { 65, 0 }, 2, 0 },

	/* 169 - Pull Lever */
	{ { 0, 0 }, { 107, 0 }, 2, 0 },

	/* 170 - Wrong Code */
	{ { 0, 0 }, { 108, 0 }, 2, 0 },

	/* 171 - Correct Code */
	{ { 0, 0 }, { 109, 0 }, 2, 0 },

	/* 172 - Sizzle */
	{ { 0, 0 }, { 110, 0 }, 2, 0 },

	/* 173 - Money In Slot */
	{ { 0, 0 }, { 111, 0 }, 2, 0 },

	/* 174 - Lightning Crack */
	{ { 0, 0 }, { 112, 0 }, 2, 0 },

	/* 175 - Machine Gun Fire */
	{ { 0, 0 }, { 113, 0 }, 2, 0 },

	/* 176 - Cage Descends */
	{ { 0, 0 }, { 114, 0 }, 2, 0 },

	/* 177 - Chair Activates */
	{ { 0, 0 }, { 115, 0 }, 2, 0 },

	/* 178 - Robot Powers On */
	{ { 0, 0 }, { 116, 0 }, 2, 0 },

	/* 179 - Grow Big */
	{ { 0, 0 }, { 117, 0 }, 2, 0 },

	/* 180 - Eat Food */
	{ { 0, 0 }, { 118, 0 }, 2, 0 },

	/* 181 - Head Shrink */
	{ { 0, 0 }, { 119, 0 }, 2, 0 },

	/* 182 - Grinding Gears */
	{ { 0, 0 }, { 120, 0 }, 2, 0 },

	/* 183 - Chair Splash */
	{ { 0, 0 }, { 123, 0 }, 2, 0 },

	/* 184 - Deflect Laser */
	{ { 0, 0 }, { 124, 0 }, 2, 0 },

	/* 185 - Zap Frank */
	{ { 0, 0 }, { 125, 0 }, 2, 0 },

	/* 186 - Frank Transforms */
	{ { 0, 0 }, { 126, 0 }, 2, 0 },

	/* 187 - Alarm Clock */
	{ { 0, 0 }, { 127, 0 }, 2, 0 },

	/* 188 - Slide Chute */
	{ { 0, 0 }, { 64, 0 }, 2, 0 },

	/* 189 - Puff */
	{ { 0, 0 }, { 128, 0 }, 2, 0 },

	/* 190 - Bite */
	{ { 0, 0 }, { 129, 0 }, 2, 0 },

	/* 191 - Stone Door Grind 2 */
	{ { 0, 0 }, { 79, 0 }, 2, 0 },

	/* 192 - Prequel 2 */
	{ { 22, 0 }, { 0, 0 }, 1, 0 },

	/* 193 - Prequel 3 */
	{ { 24, 0 }, { 0, 0 }, 1, 0 },

	/* 194 - Prequel 4 */
	{ { 25, 0 }, { 0, 0 }, 1, 0 },

	/* 195 - Stop Music */
	{ { 1, 0 }, { 0, 0 }, 1, 0 },

	/* 196 - Plane Flyby */
	{ { 0, 0 }, { 101, 0 }, 2, 0 },

	/* 197 - Commander Rocket 2 */
	{ { 46, 0 }, { 0, 0 }, 1, 0 },

	/* 198 - Commander Rocket 3 */
	{ { 47, 0 }, { 0, 0 }, 1, 0 },

	/* 199 - Rescue */
	{ { 99, 0 }, { 0, 0 }, 1, 0 },

	/* 200 - Slow Fanfare */
	{ { 0, 0 }, { 0, 0 }, 1, 0 },

	/* 201 - Plane Crash */
	{ { 93, 0 }, { 0, 0 }, 1, 0 },

	/* 202 - Plane Engine 1 */
	{ { 0, 0 }, { 130, 0 }, 2, 0 },

	/* 203 - Plane Engine 2 */
	{ { 0, 0 }, { 131, 0 }, 2, 0 },

	/* 204 - Boat In */
	{ { 0, 0 }, { 132, 0 }, 2, 0 },

	/* 205 - Boat Out */
	{ { 0, 0 }, { 133, 0 }, 2, 0 },

	/* 206 - Final Fanfare! */
	{ { 21, 95, 21, 0 }, { 0, 0 }, 1, 0 },

	/* 207 - Frank Destroyed */
	{ { 25, 1044, 0 }, { 0, 0 }, 1, 0 },


	/* 208 - Jaspar Eats */
	{ { 0, 0 }, { 134, 0 }, 2, 0 },

	/* 209 - Compy Scream 1 */
	{ { 0, 0 }, { 135, 0 }, 2, 0 },

	/* 210 - Compy Scream 2 */
	{ { 0, 0 }, { 136, 0 }, 2, 0 },

	/* 211 - Punch Klunk Fanfare */
	{ { 69, 1017, 0 }, { 0, 0 }, 1, 0 },

	/* 212 - Talk Frank */
	{ { 77, 1017, 0 }, { 0, 0 }, 1, 0 }
};

const char *const Sound::_sfxName[] = {
	/* 1 - Door Open (standard) */
	"116Bssss",

	/* 2 - Door Close (standard) */
	"105assss",

	/* 3 - Pour Liquid */
	"133sssss",

	/* 4 - Light Switch */
	"27ssssss",

	/* 5 - Scissor Snip */
	"15ssssss",

	/* 6 - Pick up Sticky */
	"79ssssss",

	/* 7 - Hydraulic Doors Open */
	"96ssssss",

	/* 8 - Hydraulic Doors Close */
	"97ssssss",

	/* 9 - Metallic Door Slams */
	"105sssss",

	/* 10 - Small Hatch Close */
	"106sssss",

	/* 11 - Oracle Rezzes In */
	"132sssss",

	/* 12 - Polish Sparkle */
	"132Cssss",

	/* 13 - Splorch! */
	"137Bssss",

	/* 14 - Dino Ray Gun */
	"138sssss",

	/* 15 - Squish! */
	"137Assss",

	/* 16 - Robot Laser */
	"61ssssss",

	/* 17 - Thud wood light */
	"109sssss",

	/* 18 - Thud wood deep */
	"110sssss",

	/* 19 - Thud metallic */
	"111sssss",

	/* 20 - Zombies Rez Out */
	"77ssssss",

	/* 21 - Sarc Door Closes  */
	"58ssssss",

	/* 22 - Thud breadboard fruit/Coconut */
	"131Assss",

	/* 23 - Thud stone */
	"75ssssss",

	/* 24 - Cloth Slide 1 */
	"135sssss",

	/* 25 - Open Chest */
	"112sssss",

	/* 26 - Close Chest */
	"121sssss",

	/* 27 - Open Drawer */
	"120sssss",

	/* 28 - Truck door closes */
	"122sssss",

	/* 29 - Truck Starts */
	"123Assss",

	/* 30 - Truck Drives Off */
	"123Bssss",

	/* 31 - Fish Splash */
	"18ssssss",

	/* 32 - Agression Enhancer */
	"138Bssss",

	/* 33 - Close Drawer/Push Ladder */
	"113sssss",

	/* 34 - *Stamp Sound */
	"40ssssss",

	/* 35 - plane hatch open */
	"3sssssss",

	/* 36 - plane hatch close */
	"4sssssss",

	/* 37 - tie vines */
	"11ssssss",

	/* 38 - Pterodactyl */
	"10ssssss",

	/* 39 - Beef Jerky Splash */
	"6sssssss",

	/* 40 - Piranha Burp */
	"7sssssss",

	/* 41 - Falling Vine */
	"13ssssss",

	/* 42 - Sproing! */
	"29ssssss",

	/* 43 - Prison/Dungeon Door */
	"33ssssss",

	/* 44 - Ape takes off mask */
	"24ssssss",

	/* 45 - Bark breaks */
	"25ssssss",

	/* 46 - Stone Click */
	"136sssss",

	/* 47 - Rocket Pack Zoom */
	"1006ssss",

	/* 48 - Cash Register */
	"36ssssss",

	/* 49 - Squeaky Toy */
	"37ssssss",

	/* 50 - Falling Chains */
	"38ssssss",

	/* 51 - Cloth Unravels */
	"64ssssss",

	/* 52 - Open Locker Door */
	"48ssssss",

	/* 53 - Close Locker Door */
	"49ssssss",

	/* 54 - Rub Pencil on Pad */
	"50ssssss",

	/* 55 - Open Safe */
	"51ssssss",

	/* 56 - Close Safe */
	"52ssssss",

	/* 57 - Push Chair */
	"59ssssss",

	/* 58 - Snake Hiss */
	"83ssssss",

	/* 59 - Oracle Rezzes Out */
	"70ssssss",

	/* 60 - Wall Crumbles */
	"73Asssss",

	/* 61 - Crypt Crumbles */
	"76ssssss",

	/* 62 - Creaking Stick */
	"74Asssss",

	/* 63 - Joe Sucked Up */
	"80ssssss",

	/* 64 - Slide Chute */
	"114assss",

	/* 65 - Press Button */
	"1007ssss",

	/* 66 - Snap Branch */
	"101sssss",

	/* 67 - Dino Horn */
	"103sssss",

	/* 68 - Tire Screech */
	"125sssss",

	/* 69 - Chicken */
	"126sssss",

	/* 70 - Oil Splat */
	"127sssss",

	/* 71 - Punch */
	"128sssss",

	/* 72 - Body Hits Ground */
	"129sssss",

	/* 73 - Pick Hits Stone */
	"71ssssss",

	/* 74 - Stalactite Crumbles */
	"119sssss",

	/* 75 - *Drip */
	"93ssssss",

	/* 76 - Tic-Toc */
	"42Bsssss",

	/* 77 - Explosion */
	"88ssssss",

	/* 78 - Stone Door Grind 1 */
	"1001ssss",

	/* 79 - Stone Door Grind 2 */
	"1002ssss",

	/* 80 - *Fight Explosion */
	"1000ssss",

	/* 81 - Stone Grind (heavy) */
	"1003ssss",

	/* 82 - Stone Grind (light) */
	"89ssssss",

	/* 83 - Piranha Splash */
	"5sssssss",

	/* 84 - Cloth Slide 2 */
	"1005ssss",

	/* 85 - Temple Laser */
	"87ssssss",

	/* 86 - Dino Transformation */
	"55Bsssss",

	/* 87 - Experimental Laser */
	"55ssssss",

	/* 88 - Stone Grind (medium) */
	"134sssss",

	/* 89 - Weeping God Grind */
	"94ssssss",

	/* 90 - Alien Hum */
	"95ssssss",

	/* 91 - Alien Puzzle */
	"103Assss",

	/* 92 - Vacuum On */
	"21ssssss",

	/* 93 - Vacuum Off */
	"21Csssss",

	/* 94 - Elevator Starts */
	"44ssssss",

	/* 95 - Mummy Crumbles */
	"68ssssss",

	/* 96 - Temple Green Circle */
	"60Bsssss",

	/* 97 - Rattle Bars */
	"115sssss",

	/* 98 - Door Dissolves */
	"56ssssss",

	/* 99 - Altar Slides */
	"85ssssss",

	/* 100 - Light Torch */
	"81ssssss",

	/* 101 - Plane Flyby */
	"1027ssss",

	/* 102 - Plaster Loud */
	"41Bsssss",

	/* 103 - Sparky Bathtub */
	"73ssssss",

	/* 104 - Ape Rezzes Out */
	"14ssssss",

	/* 105 - Piranhas Swim */
	"17ssssss",

	/* 106 - *Gun Shot */
	"1004ssss",

	/* 107 - Pull Lever */
	"1008ssss",

	/* 108 - Wrong Code */
	"1009ssss",

	/* 109 - Correct Code */
	"1010ssss",

	/* 110 - Sizzle */
	"1011ssss",

	/* 111 - Money In Slot */
	"1012ssss",

	/* 112 - Lightning */
	"1013ssss",

	/* 113 - Machine Gun Fire */
	"1014ssss",

	/* 114 - Cage Descends */
	"1015ssss",

	/* 115 - Temple Chair Activates */
	"1016ssss",

	/* 116 - Robot Powers On */
	"1017ssss",

	/* 117 - Grow Big */
	"1018ssss",

	/* 118 - Eat Food */
	"1019ssss",

	/* 119 - Head Shrink */
	"1020ssss",

	/* 120 - Grinding Gears */
	"84ssssss",

	/* 121 - Wave Torch */
	"1021ssss",

	/* 122 - Snake Slithers Off */
	"1022ssss",

	/* 123 - Chair Splash */
	"26ssssss",

	/* 124 - Deflect Laser */
	"60ssssss",

	/* 125 - Zap Frank */
	"1023ssss",

	/* 126 - Frank Transforms */
	"1024ssss",

	/* 127 - Alarm Clock */
	"1025ssss",

	/* 128 - Puff */
	"35ssssss",

	/* 129 - Bite */
	"1026ssss",

	/* 130 - Plane Engine 1 */
	"1028ssss",

	/* 131 - Plane Engine 2 */
	"1029ssss",

	/* 132 - Boat In */
	"1030ssss",

	/* 133 - Boat Out */
	"1031ssss",

	/* 134 - Jaspar Eats */
	"1032ssss",

	/* 135 - Compy Scream 1 */
	"1033ssss",

	/* 136 - Compy Scream 2 */
	"1034ssss"
};

const int16 Sound::_jungleList[] = { 15, 16, 17, 18, 7, 8, 9, 10, 11, 12, 13, 14, 0 };


} // End of namespace Queen
