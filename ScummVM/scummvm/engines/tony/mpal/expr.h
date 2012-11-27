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
 *
 */
/*
 * This code is based on original Tony Tough source code
 *
 * Copyright (c) 1997-2003 Nayma Software
 */

#ifndef MPAL_EXPR_H
#define MPAL_EXPR_H

#include "tony/mpal/memory.h"

namespace Tony {

namespace MPAL {

/**
 * @defgroup Mathamatical operations
 */
//@{

#define OP_MUL     ((1  << 4) | 0)
#define OP_DIV     ((1  << 4) | 1)
#define OP_MODULE  ((1  << 4) | 2)
#define OP_ADD     ((2  << 4) | 0)
#define OP_SUB     ((2  << 4) | 1)
#define OP_SHL     ((3  << 4) | 0)
#define OP_SHR     ((3  << 4) | 1)
#define OP_MINOR   ((4  << 4) | 0)
#define OP_MAJOR   ((4  << 4) | 1)
#define OP_MINEQ   ((4  << 4) | 2)
#define OP_MAJEQ   ((4  << 4) | 3)
#define OP_EQUAL   ((5  << 4) | 0)
#define OP_NOEQUAL ((5  << 4) | 1)
#define OP_BITAND  ((6  << 4) | 0)
#define OP_BITXOR  ((7  << 4) | 0)
#define OP_BITOR   ((8  << 4) | 0)
#define OP_AND     ((9  << 4) | 0)
#define OP_OR      ((10 << 4) | 0)

//@}

/**
 * @defgroup Structures
 */

//@{
/**
 * Mathamatical framework to manage operations
 */
typedef struct {
	byte _type;          // Object Type (see enum ExprListTypes)

	union {
		int _num;        // Identifier (if type == ELT_NUMBER)
		char *_name;     // Variable name (if type == ELT_VAR)
		MpalHandle _son; // Handle expressions (if type == ELT_PARENTH)
		byte *_pson;     // Handle lockato (if type == ELT_PARENTH2)
	} _val;

	byte _symbol;        // Mathematic symbols (see #define OP_*)

} Expression;
typedef Expression *LpExpression;

//@}

/**
 * Object types that can be contained in an EXPRESSION structure
 */
enum ExprListTypes {
	ELT_NUMBER   = 1,
	ELT_VAR      = 2,
	ELT_PARENTH  = 3,
	ELT_PARENTH2 = 4
};

/****************************************************************************\
*       Function Prototypes
\****************************************************************************/

/**
 * Parses a mathematical expression from the MPC file
 *
 * @param buf				Buffer containing the expression to evaluate
 * @param h					Pointer to a handle that, at the end of execution,
 * will point to the area of memory containing the parsed expression
 * @returns		Pointer to the buffer immediately after the expression, or NULL if error.
 */
const byte *parseExpression(const byte *lpBuf, MpalHandle *h);

/**
 * Calculate the value of a mathamatical expression
 *
 * @param h					Handle to the expression
 * @returns		Numeric value
 */
int evaluateExpression(MpalHandle h);

/**
 * Compare two mathematical expressions together
 *
 * @param h1				Expression to be compared
 * @param h2				Expression to be compared
 */
bool compareExpressions(MpalHandle h1, MpalHandle h2);

/**
 * Frees an expression that was previously parsed
 *
 * @param h					Handle for the expression
 */
void freeExpression(MpalHandle h);

} // end of namespace MPAL

} // end of namespace Tony

#endif
