/*
**
** File: ym2151.h - header file for software implementation of YM2151
**                                            FM Operator Type-M(OPM)
**
** (c) 1997-2002 Jarek Burczynski (s0246@poczta.onet.pl, bujar@mame.net)
** Some of the optimizing ideas by Tatsuyuki Satoh
**
** Version 2.150 final beta May, 11th 2002
**
**
** I would like to thank following people for making this project possible:
**
** Beauty Planets - for making a lot of real YM2151 samples and providing
** additional informations about the chip. Also for the time spent making
** the samples and the speed of replying to my endless requests.
**
** Shigeharu Isoda - for general help, for taking time to scan his YM2151
** Japanese Manual first of all, and answering MANY of my questions.
**
** Nao - for giving me some info about YM2151 and pointing me to Shigeharu.
** Also for creating fmemu (which I still use to test the emulator).
**
** Aaron Giles and Chris Hardy - they made some samples of one of my favourite
** arcade games so I could compare it to my emulator.
**
** Bryan McPhail and Tim (powerjaw) - for making some samples.
**
** Ishmair - for the datasheet and motivation.
*/

#ifndef _H_YM2151_
#define _H_YM2151_

/* 16- and 8-bit samples (signed) are supported*/
#define SAMPLE_BITS 16

#if (SAMPLE_BITS==16)
	typedef INT16 SAMP;
#endif
#if (SAMPLE_BITS==8)
	typedef signed char SAMP;
#endif

/*
** Initialize YM2151 emulator(s).
**
** 'num' is the number of virtual YM2151's to allocate
** 'clock' is the chip clock in Hz
** 'rate' is sampling rate
*/
int YM2151Init(int num, int clock, int rate);

/* shutdown the YM2151 emulators*/
void YM2151Shutdown(void);

/* reset all chip registers for YM2151 number 'num'*/
void YM2151ResetChip(int num);

/*
** Generate samples for one of the YM2151's
**
** 'num' is the number of virtual YM2151
** '**buffers' is table of pointers to the buffers: left and right
** 'length' is the number of samples that should be generated
*/
void YM2151UpdateOne(int num, INT16 **buffers, int length);

/* write 'v' to register 'r' on YM2151 chip number 'n'*/
void YM2151WriteReg(int n, int r, int v);

/* read status register on YM2151 chip number 'n'*/
int YM2151ReadStatus(int n);

/* set interrupt handler on YM2151 chip number 'n'*/
void YM2151SetIrqHandler(int n, void (*handler)(int irq));

/* set port write handler on YM2151 chip number 'n'*/
void YM2151SetPortWriteHandler(int n, write8_handler handler);

#endif /*_H_YM2151_*/
