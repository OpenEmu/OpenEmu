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

#ifndef SWORD1_OBJECT_H
#define SWORD1_OBJECT_H

#include "common/scummsys.h"

namespace Sword1 {

#define O_TOTAL_EVENTS  5
#define O_WALKANIM_SIZE 600         //max number of nodes in router output
#define O_GRID_SIZE     200
#define EXTRA_GRID_SIZE 20

#include "common/pack-start.h"  // START STRUCT PACKING

struct OEventSlot {         //receiving event list in the compact -
	int32 o_event;        //array of these with O_TOTAL_EVENTS elements
	int32 o_event_script;
} PACKED_STRUCT;    // size = 2*int32 = 8 bytes

#define TOTAL_script_levels 5

struct ScriptTree {         //this is a logic tree, used by OBJECTs
	int32 o_script_level;                     //logic level
	int32 o_script_id[TOTAL_script_levels];   //script id's (are unique to each level)
	int32 o_script_pc[TOTAL_script_levels];   //pc of script for each (if script_manager)
} PACKED_STRUCT;    // size = 11*int32 = 44 bytes

struct TalkOffset {
	int32 x;
	int32 y;
} PACKED_STRUCT;    // size = 2*int32 = 8 bytes

struct WalkData {
	int32 frame;
	int32 x;
	int32 y;
	int32 step;
	int32 dir;
} PACKED_STRUCT;    // size = 5*int32 = 20 bytes

struct Object {
	int32 o_type;                     // 0  broad description of type - object, floor, etc.
	int32 o_status;                   // 4  bit flags for logic, graphics, mouse, etc.
	int32 o_logic;                    // 8  logic type
	int32 o_place;                    // 12 where is the mega character
	int32 o_down_flag;                // 16 pass back down with this - with C possibly both are unnecessary?
	int32 o_target;                   // 20 target object for the GTM         *these are linked to script
	int32 o_screen;                   // 24 physical screen/section
	int32 o_frame;                    // 28 frame number &
	int32 o_resource;                 // 32 id of spr file it comes from
	int32 o_sync;                     // 36 receive sync here
	int32 o_pause;                    // 40 logic_engine() pauses these cycles
	int32 o_xcoord;                   // 44
	int32 o_ycoord;                   // 48
	int32 o_mouse_x1;                 // 52 top-left of mouse area is (x1,y1)
	int32 o_mouse_y1;                 // 56
	int32 o_mouse_x2;                 // 60 bottom-right of area is (x2,y2)   (these coords are inclusive)
	int32 o_mouse_y2;                 // 64
	int32 o_priority;                 // 68
	int32 o_mouse_on;                 // 72
	int32 o_mouse_off;                // 76
	int32 o_mouse_click;              // 80
	int32 o_interact;                 // 84
	int32 o_get_to_script;            // 88
	int32 o_scale_a;                  // 92 used by floors
	int32 o_scale_b;                  // 96
	int32 o_anim_x;                   // 100
	int32 o_anim_y;                   // 104

	ScriptTree o_tree;                // 108  size = 44 bytes
	ScriptTree o_bookmark;            // 152  size = 44 bytes

	int32 o_dir;                      // 196
	int32 o_speech_pen;               // 200
	int32 o_speech_width;             // 204
	int32 o_speech_time;              // 208
	int32 o_text_id;                  // 212 working back from o_ins1
	int32 o_tag;                      // 216
	int32 o_anim_pc;                  // 220 position within an animation structure
	int32 o_anim_resource;            // 224 cdt or anim table

	int32 o_walk_pc;                  // 228

	TalkOffset talk_table[6];         // 232  size = 6*8 bytes = 48

	OEventSlot o_event_list[O_TOTAL_EVENTS];    // 280  size = 5*8 bytes = 40

	int32 o_ins1;                     // 320
	int32 o_ins2;                     // 324
	int32 o_ins3;                     // 328

	int32 o_mega_resource;            // 332
	int32 o_walk_resource;            // 336

	WalkData    o_route[O_WALKANIM_SIZE];   // 340  size = 600*20 bytes = 12000
	                                        // mega size = 12340 bytes (+ 8 byte offset table + 20 byte header = 12368)
} PACKED_STRUCT;

#include "common/pack-end.h"    // END STRUCT PACKING

} // End of namespace Sword1

#endif //BSOBJECT_H
