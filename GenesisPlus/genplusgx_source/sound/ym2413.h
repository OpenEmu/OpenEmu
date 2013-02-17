/*
**
** File: ym2413.c - software implementation of YM2413
**                  FM sound generator type OPLL
**
** Copyright (C) 2002 Jarek Burczynski
**
** Version 1.0
**
*/

#ifndef _H_YM2413_
#define _H_YM2413_

extern void YM2413Init(void);
extern void YM2413ResetChip(void);
extern void YM2413Update(int *buffer, int length);
extern void YM2413Write(unsigned int a, unsigned int v);
extern unsigned int YM2413Read(unsigned int a);
extern unsigned char *YM2413GetContextPtr(void);
extern unsigned int YM2413GetContextSize(void);

#endif /*_H_YM2413_*/
