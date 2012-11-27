/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * Additional copyright for this file:
 * Copyright (C) 1994-1998 Revolution Software Ltd.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */

#ifndef SWORD2_INTERPRETER_H
#define SWORD2_INTERPRETER_H

#include "common/endian.h"

namespace Sword2 {

// Interpreter return codes

enum {
	IR_STOP		= 0,	// Quit for a cycle
	IR_CONT		= 1,	// Continue as normal
	IR_TERMINATE	= 2,	// Return without updating the offset
	IR_REPEAT	= 3,	// Return; offset at start of function call
	IR_GOSUB	= 4	// Return with updated offset
};

// Get parameter fix so that the playstation version can handle words not on
// word boundaries

#define Read8ip(var)		{ var = code[ip]; ip++; }
#define Read16ip(var)		{ var = (int16)READ_LE_UINT16(code + ip); ip += 2; }
#define Read32ip(var)		{ var = (int32)READ_LE_UINT32(code + ip); ip += 4; }
#define Read32ipLeaveip(var)	{ var = (int32)READ_LE_UINT32(code + ip); }

enum {
	// Compiled tokens

	CP_END_SCRIPT			= 0,
	CP_PUSH_LOCAL_VAR32		= 1,	// Push a local variable on to the stack
	CP_PUSH_GLOBAL_VAR32		= 2,	// Push a global variable
	CP_POP_LOCAL_VAR32		= 3,	// Pop a local variable from the stack
	CP_CALL_MCODE			= 4,	// Call a machine code function
	CP_PUSH_LOCAL_ADDR		= 5,	// Push the address of a local variable
	CP_PUSH_INT32			= 6,	// Adjust the stack after calling an fn function
	CP_SKIPONFALSE			= 7,	// Skip if the bottom value on the stack is false
	CP_SKIPALWAYS			= 8,	// Skip a block of code
	CP_SWITCH			= 9,	// Switch on last stack value
	CP_ADDNPOP_LOCAL_VAR32		= 10,	// Add to a local varible
	CP_SUBNPOP_LOCAL_VAR32		= 11,	// Subtract from a local variable
	CP_SKIPONTRUE			= 12,	// Skip if the bottom value on the stack is true
	CP_POP_GLOBAL_VAR32		= 13,	// Pop a global variable
	CP_ADDNPOP_GLOBAL_VAR32		= 14,	// Add to a global variable
	CP_SUBNPOP_GLOBAL_VAR32		= 15,	// Subtract from a global variable
	CP_DEBUGON			= 16,	// Turn debugging on
	CP_DEBUGOFF			= 17,	// Turn debugging off
	CP_QUIT				= 18,	// Quit for a cycle
	CP_TERMINATE			= 19,	// Quit script completely

	// Operators

	OP_ISEQUAL			= 20,	// '=='
	OP_PLUS				= 21,	// '+'
	OP_MINUS			= 22,	// '-'
	OP_TIMES			= 23,	// '*'
	OP_DIVIDE			= 24,	// '/'
	OP_NOTEQUAL			= 25,	// '=='
	OP_ANDAND			= 26,	// '&&'
	OP_GTTHAN			= 27,	// '>'
	OP_LSTHAN			= 28,	// '<'

	// More tokens, mixed types

	CP_JUMP_ON_RETURNED		= 29,	// Use table of jumps with value returned from fn_mcode
	CP_TEMP_TEXT_PROCESS		= 30,	// A dummy text process command for me
	CP_SAVE_MCODE_START		= 31,	// Save the mcode code start for restarting when necessary
	CP_RESTART_SCRIPT		= 32,	// Start the script from the beginning
	CP_PUSH_STRING			= 33,	// Push a pointer to a string on the stack
	CP_PUSH_DEREFERENCED_STRUCTURE	= 34,	// Push the address of a structure thing
	OP_GTTHANE			= 35,	// >=
	OP_LSTHANE			= 36,	// <=
	OP_OROR				= 37	// || or OR
};

} // End of namespace Sword2

#endif
