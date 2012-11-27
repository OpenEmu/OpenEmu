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

#include "tony/mpal/mpal.h"
#include "tony/mpal/memory.h"
#include "tony/mpal/mpaldll.h"
#include "tony/tony.h"

namespace Tony {

namespace MPAL {

/**
 * Duplicate a mathematical expression.
 *
 * @param h				Handle to the original expression
 * @retruns		Pointer to the cloned expression
 */
static byte *duplicateExpression(MpalHandle h) {
	byte *orig, *clone;

	orig = (byte *)globalLock(h);

	int num = *(byte *)orig;
	LpExpression one = (LpExpression)(orig+1);

	clone = (byte *)globalAlloc(GMEM_FIXED, sizeof(Expression) * num + 1);
	LpExpression two = (LpExpression)(clone + 1);

	memcpy(clone, orig, sizeof(Expression) * num + 1);

	for (int i = 0; i < num; i++) {
		if (one->_type == ELT_PARENTH) {
			two->_type = ELT_PARENTH2;
			two->_val._pson = duplicateExpression(two->_val._son);
		}

		++one;
		++two;
	}

	globalUnlock(h);
	return clone;
}

static int Compute(int a, int b, byte symbol) {
	switch (symbol) {
	case OP_MUL:
		return a * b;
	case OP_DIV:
		return a / b;
	case OP_MODULE:
		return a % b;
	case OP_ADD:
		return a + b;
	case OP_SUB:
		return a - b;
	case OP_SHL:
		return a << b;
	case OP_SHR:
		return a >> b;
	case OP_MINOR:
		return a < b;
	case OP_MAJOR:
		return a > b;
	case OP_MINEQ:
		return a <= b;
	case OP_MAJEQ:
		return a >= b;
	case OP_EQUAL:
		return a == b;
	case OP_NOEQUAL:
		return a != b;
	case OP_BITAND:
		return a & b;
	case OP_BITXOR:
		return a ^ b;
	case OP_BITOR:
		return a | b;
	case OP_AND:
		return a && b;
	case OP_OR:
		return a || b;
	default:
		GLOBALS._mpalError = 1;
		break;
	}

	return 0;
}

static void solve(LpExpression one, int num) {
	LpExpression two, three;

	while (num > 1) {
		two = one + 1;
		if ((two->_symbol == 0) || (one->_symbol & 0xF0) <= (two->_symbol & 0xF0)) {
			two->_val._num = Compute(one->_val._num, two->_val._num, one->_symbol);
			memmove(one, two, (num - 1) * sizeof(Expression));
			--num;
		} else {
			int j = 1;
			three = two + 1;
			while ((three->_symbol != 0) && (two->_symbol & 0xF0) > (three->_symbol & 0xF0)) {
				++two;
				++three;
				++j;
			}

			three->_val._num = Compute(two->_val._num, three->_val._num, two->_symbol);
			memmove(two, three, (num - j - 1) * sizeof(Expression));
			--num;
		}
	}
}

/**
 * Calculates the result of a mathematical expression, replacing the current
 * value of any variable.
 *
 * @param expr				Pointer to an expression duplicated by DuplicateExpression
 * @returns		Value
 */
static int evaluateAndFreeExpression(byte *expr) {
	int num = *expr;
	LpExpression one = (LpExpression)(expr + 1);

	// 1) Substitutions of variables
	LpExpression cur = one;
	for (int i = 0; i < num; i++, cur++) {
		if (cur->_type == ELT_VAR) {
			cur->_type = ELT_NUMBER;
			cur->_val._num = varGetValue(cur->_val._name);
		}
	}

	// 2) Replacement of brackets (using recursive calls)
	cur = one;
	for (int i = 0; i < num; i++, cur++) {
		if (cur->_type == ELT_PARENTH2) {
			cur->_type = ELT_NUMBER;
			cur->_val._num = evaluateAndFreeExpression(cur->_val._pson);
		}
	}

	// 3) algebraic resolution
	solve(one, num);
	int val = one->_val._num;
	globalDestroy(expr);

	return val;
}

/**
 * Parses a mathematical expression from the MPC file
 *
 * @param buf				Buffer containing the expression to evaluate
 * @param h					Pointer to a handle that, at the end of execution,
 * will point to the area of memory containing the parsed expression
 * @returns		Pointer to the buffer immediately after the expression, or NULL if error.
 */
const byte *parseExpression(const byte *lpBuf, MpalHandle *h) {
	byte *start;

	uint32 num = *lpBuf;
	lpBuf++;

	if (num == 0)
		return NULL;

	*h = globalAllocate(GMEM_MOVEABLE | GMEM_ZEROINIT, num * sizeof(Expression) + 1);
	if (*h == NULL)
		return NULL;

	start = (byte *)globalLock(*h);
	*start = (byte)num;

	LpExpression cur = (LpExpression)(start + 1);

	for (uint32 i = 0;i < num; i++) {
		cur->_type = *(lpBuf);

		// *(lpBuf + 1) contains the unary operator, unused => skipped
		lpBuf += 2;

		switch (cur->_type) {
		case ELT_NUMBER:
			cur->_val._num = (int32)READ_LE_UINT32(lpBuf);
			lpBuf += 4;
			break;

		case ELT_VAR:
			cur->_val._name = (char *)globalAlloc(GMEM_FIXED | GMEM_ZEROINIT, (*lpBuf) + 1);
			if (cur->_val._name == NULL)
				return NULL;
			memcpy(cur->_val._name, lpBuf + 1, *lpBuf);
			lpBuf += *lpBuf + 1;
			break;

		case ELT_PARENTH:
			lpBuf = parseExpression(lpBuf, &cur->_val._son);
			if (lpBuf == NULL)
				return NULL;
			break;

		default:
			return NULL;
		}

		cur->_symbol = *lpBuf;
		lpBuf++;

		cur++;
	}

	if (*lpBuf != 0)
		return NULL;

	lpBuf++;

	return lpBuf;
}

/**
 * Calculate the value of a mathamatical expression
 *
 * @param h					Handle to the expression
 * @returns		Numeric value
 */
int evaluateExpression(MpalHandle h) {
	lockVar();
	int ret = evaluateAndFreeExpression(duplicateExpression(h));
	unlockVar();

	return ret;
}

/**
 * Compare two mathematical expressions together
 *
 * @param h1				Expression to be compared
 * @param h2				Expression to be compared
 */
bool compareExpressions(MpalHandle h1, MpalHandle h2) {
	byte *e1, *e2;

	e1 = (byte *)globalLock(h1);
	e2 = (byte *)globalLock(h2);

	int num1 = *(byte *)e1;
	int num2 = *(byte *)e2;

	if (num1 != num2) {
		globalUnlock(h1);
		globalUnlock(h2);
		return false;
	}

	LpExpression one = (LpExpression)(e1 + 1);
	LpExpression two = (LpExpression)(e2 + 1);

	for (int i = 0; i < num1; i++) {
		if (one->_type != two->_type || (i != num1 - 1 && one->_symbol != two->_symbol)) {
			globalUnlock(h1);
			globalUnlock(h2);
			return false;
		}

		switch (one->_type) {
		case ELT_NUMBER:
			if (one->_val._num != two->_val._num) {
				globalUnlock(h1);
				globalUnlock(h2);
				return false;
			}
			break;

		case ELT_VAR:
			if (strcmp(one->_val._name, two->_val._name) != 0) {
				globalUnlock(h1);
				globalUnlock(h2);
				return false;
			}
			break;

		case ELT_PARENTH:
			if (!compareExpressions(one->_val._son, two->_val._son)) {
				globalUnlock(h1);
				globalUnlock(h2);
				return false;
			}
			break;
		}

		++one;
		++two;
	}

	globalUnlock(h1);
	globalUnlock(h2);

	return true;
}

/**
 * Frees an expression that was previously parsed
 *
 * @param h					Handle for the expression
 */
void freeExpression(MpalHandle h) {
	byte *data = (byte *)globalLock(h);
	int num = *data;
	LpExpression cur = (LpExpression)(data + 1);

	for (int i = 0; i < num; ++i, ++cur) {
		switch (cur->_type) {
		case ELT_VAR:
			globalDestroy(cur->_val._name);
			break;

		case ELT_PARENTH:
			freeExpression(cur->_val._son);
			break;

		default:
			break;
		}
	}

	globalUnlock(h);
	globalFree(h);
}

} // end of namespace MPAL

} // end of namespace Tony
