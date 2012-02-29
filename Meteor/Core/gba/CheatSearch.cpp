#include <stdlib.h>
#include <memory.h>

#include "CheatSearch.h"

CheatSearchBlock cheatSearchBlocks[4];

CheatSearchData cheatSearchData = {
  0,
  cheatSearchBlocks
};

static bool cheatSearchEQ(u32 a, u32 b)
{
  return a == b;
}

static bool cheatSearchNE(u32 a, u32 b)
{
  return a != b;
}

static bool cheatSearchLT(u32 a, u32 b)
{
  return a < b;
}

static bool cheatSearchLE(u32 a, u32 b)
{
  return a <= b;
}

static bool cheatSearchGT(u32 a, u32 b)
{
  return a > b;
}

static bool cheatSearchGE(u32 a, u32 b)
{
  return a >= b;
}

static bool cheatSearchSignedEQ(s32 a, s32 b)
{
  return a == b;
}

static bool cheatSearchSignedNE(s32 a, s32 b)
{
  return a != b;
}

static bool cheatSearchSignedLT(s32 a, s32 b)
{
  return a < b;
}

static bool cheatSearchSignedLE(s32 a, s32 b)
{
  return a <= b;
}

static bool cheatSearchSignedGT(s32 a, s32 b)
{
  return a > b;
}

static bool cheatSearchSignedGE(s32 a, s32 b)
{
  return a >= b;
}

static bool (*cheatSearchFunc[])(u32,u32) = {
  cheatSearchEQ,
  cheatSearchNE,
  cheatSearchLT,
  cheatSearchLE,
  cheatSearchGT,
  cheatSearchGE
};

static bool (*cheatSearchSignedFunc[])(s32,s32) = {
  cheatSearchSignedEQ,
  cheatSearchSignedNE,
  cheatSearchSignedLT,
  cheatSearchSignedLE,
  cheatSearchSignedGT,
  cheatSearchSignedGE
};

void cheatSearchCleanup(CheatSearchData *cs)
{
  int count = cs->count;

  for(int i = 0; i < count; i++) {
    free(cs->blocks[i].saved);
    free(cs->blocks[i].bits);
  }
  cs->count = 0;
}

void cheatSearchStart(const CheatSearchData *cs)
{
  int count = cs->count;

  for(int i = 0; i < count; i++) {
    CheatSearchBlock *block = &cs->blocks[i];

    memset(block->bits, 0xff, block->size >> 3);
    memcpy(block->saved, block->data, block->size);
  }
}

s32 cheatSearchSignedRead(u8 *data, int off, int size)
{
  u32 res = data[off++];

  switch(size) {
  case BITS_8:
    res <<= 24;
    return ((s32)res) >> 24;
  case BITS_16:
    res |= ((u32)data[off++])<<8;
    res <<= 16;
    return ((s32)res) >> 16;
  case BITS_32:
    res |= ((u32)data[off++])<<8;
    res |= ((u32)data[off++])<<16;
    res |= ((u32)data[off++])<<24;
    return (s32)res;
  }
  return (s32)res;
}

u32 cheatSearchRead(u8 *data, int off, int size)
{
  u32 res = data[off++];
  if(size == BITS_16)
    res |= ((u32)data[off++])<<8;
  else if(size == BITS_32) {
    res |= ((u32)data[off++])<<8;
    res |= ((u32)data[off++])<<16;
    res |= ((u32)data[off++])<<24;
  }
  return res;
}

void cheatSearch(const CheatSearchData *cs, int compare, int size,
                 bool isSigned)
{
  if(compare < 0 || compare > SEARCH_GE)
    return;
  int inc = 1;
  if(size == BITS_16)
    inc = 2;
  else if(size == BITS_32)
    inc = 4;

  if(isSigned) {
    bool (*func)(s32,s32) = cheatSearchSignedFunc[compare];

    for(int i = 0; i < cs->count; i++) {
      CheatSearchBlock *block = &cs->blocks[i];
      int size2 = block->size;
      u8 *bits = block->bits;
      u8 *data = block->data;
      u8 *saved = block->saved;

      for(int j = 0; j < size2; j += inc) {
	if(IS_BIT_SET(bits, j)) {
	  s32 a = cheatSearchSignedRead(data, j, size);
	  s32 b = cheatSearchSignedRead(saved,j, size);

	  if(!func(a, b)) {
	    CLEAR_BIT(bits, j);
	    if(size == BITS_16)
	      CLEAR_BIT(bits, j+1);
	    if(size == BITS_32) {
	      CLEAR_BIT(bits, j+2);
	      CLEAR_BIT(bits, j+3);
	    }
	  }
	}
      }
    }
  } else {
    bool (*func)(u32,u32) = cheatSearchFunc[compare];

    for(int i = 0; i < cs->count; i++) {
      CheatSearchBlock *block = &cs->blocks[i];
      int size2 = block->size;
      u8 *bits = block->bits;
      u8 *data = block->data;
      u8 *saved = block->saved;

      for(int j = 0; j < size2; j += inc) {
	if(IS_BIT_SET(bits, j)) {
	  u32 a = cheatSearchRead(data, j, size);
	  u32 b = cheatSearchRead(saved,j, size);

	  if(!func(a, b)) {
	    CLEAR_BIT(bits, j);
	    if(size == BITS_16)
	      CLEAR_BIT(bits, j+1);
	    if(size == BITS_32) {
	      CLEAR_BIT(bits, j+2);
	      CLEAR_BIT(bits, j+3);
	    }
	  }
	}
      }
    }
  }
}

void cheatSearchValue(const CheatSearchData *cs, int compare, int size,
		      bool isSigned, u32 value)
{
  if(compare < 0 || compare > SEARCH_GE)
    return;
  int inc = 1;
  if(size == BITS_16)
    inc = 2;
  else if(size == BITS_32)
    inc = 4;

  if(isSigned) {
    bool (*func)(s32,s32) = cheatSearchSignedFunc[compare];

    for(int i = 0; i < cs->count; i++) {
      CheatSearchBlock *block = &cs->blocks[i];
      int size2 = block->size;
      u8 *bits = block->bits;
      u8 *data = block->data;

      for(int j = 0; j < size2; j += inc) {
	if(IS_BIT_SET(bits, j)) {
	  s32 a = cheatSearchSignedRead(data, j, size);
	  s32 b = (s32)value;

	  if(!func(a, b)) {
	    CLEAR_BIT(bits, j);
	    if(size == BITS_16)
	      CLEAR_BIT(bits, j+1);
	    if(size == BITS_32) {
	      CLEAR_BIT(bits, j+2);
	      CLEAR_BIT(bits, j+3);
	    }
	  }
	}
      }
    }
  } else {
    bool (*func)(u32,u32) = cheatSearchFunc[compare];

    for(int i = 0; i < cs->count; i++) {
      CheatSearchBlock *block = &cs->blocks[i];
      int size2 = block->size;
      u8 *bits = block->bits;
      u8 *data = block->data;

      for(int j = 0; j < size2; j += inc) {
	if(IS_BIT_SET(bits, j)) {
	  u32 a = cheatSearchRead(data, j, size);

	  if(!func(a, value)) {
	    CLEAR_BIT(bits, j);
	    if(size == BITS_16)
	      CLEAR_BIT(bits, j+1);
	    if(size == BITS_32) {
	      CLEAR_BIT(bits, j+2);
	      CLEAR_BIT(bits, j+3);
	    }
	  }
	}
      }
    }
  }
}

int cheatSearchGetCount(const CheatSearchData *cs, int size)
{
  int res = 0;
  int inc = 1;
  if(size == BITS_16)
    inc = 2;
  else if(size == BITS_32)
    inc = 4;

  for(int i = 0; i < cs->count; i++) {
    CheatSearchBlock *block = &cs->blocks[i];

    int size2 = block->size;
    u8 *bits = block->bits;
    for(int j = 0; j < size2; j += inc) {
      if(IS_BIT_SET(bits, j))
	res++;
    }
  }
  return res;
}

void cheatSearchUpdateValues(const CheatSearchData *cs)
{
  for(int i = 0; i < cs->count; i++) {
    CheatSearchBlock *block = &cs->blocks[i];

    memcpy(block->saved, block->data, block->size);
  }
}

