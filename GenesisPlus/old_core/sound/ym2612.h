/*
**
** File: ym2612.h -- header for ym2612.c
** software implementation of Yamaha FM sound generator
**
** Copyright (C) 2001, 2002, 2003 Jarek Burczynski (bujar at mame dot net)
** Copyright (C) 1998 Tatsuyuki Satoh , MultiArcadeMachineEmulator development
**
** Version 1.4 (final beta)
**
*/

#ifndef _H_FM_FM_
#define _H_FM_FM_

/* compiler dependence */
#ifndef INLINE
#define INLINE static __inline__
#endif


extern int YM2612Init(int baseclock, int rate);
extern int YM2612ResetChip(void);
extern void YM2612Update(int length);
extern void YM2612Write(unsigned int a, unsigned int v);
extern unsigned int YM2612Read(void);
extern unsigned char *YM2612GetContextPtr(void);
extern unsigned int YM2612GetContextSize(void);
extern void YM2612Restore(unsigned char *buffer);

#endif /* _H_FM_FM_ */
