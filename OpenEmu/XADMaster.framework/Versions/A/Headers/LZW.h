/*
 * LZW.h
 *
 * Copyright (c) 2017-present, MacPaw Inc. All rights reserved.
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston,
 * MA 02110-1301  USA
 */
#ifndef __LZW_H__
#define __LZW_H__

#include <stdint.h>
#include <stdbool.h>

#define LZWNoError 0
#define LZWInvalidCodeError 1
#define LZWTooManyCodesError 2

typedef struct LZWTreeNode
{
	uint8_t chr;
	int parent;
} LZWTreeNode;

typedef struct LZW
{
	int numsymbols,maxsymbols,reservedsymbols;
	int prevsymbol;
	int symbolsize;

	uint8_t *buffer;
	int buffersize;

	LZWTreeNode nodes[0];
} LZW;

LZW *AllocLZW(int maxsymbols,int reservedsymbols);
void FreeLZW(LZW *self);
void ClearLZWTable(LZW *self);

int NextLZWSymbol(LZW *self,int symbol);
int ReplaceLZWSymbol(LZW *self,int oldsymbol,int symbol);
int LZWOutputLength(LZW *self);
int LZWOutputToBuffer(LZW *self,uint8_t *buffer);
int LZWReverseOutputToBuffer(LZW *self,uint8_t *buffer);
int LZWOutputToInternalBuffer(LZW *self);

static inline int LZWSuggestedSymbolSize(LZW *self)
{
	return self->symbolsize;
}

static inline uint8_t *LZWInternalBuffer(LZW *self)
{
	return self->buffer;
}

static inline int LZWSymbolCount(LZW *self)
{
	return self->numsymbols;
}

static inline bool LZWSymbolListFull(LZW *self)
{
	return self->numsymbols==self->maxsymbols;
}

static inline LZWTreeNode *LZWSymbols(LZW *self)
{
	return self->nodes;
}

#endif

