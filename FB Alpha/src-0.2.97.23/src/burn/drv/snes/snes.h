#pragma once

#include "tiles_generic.h"
#include "bitswap.h"

/* Useful definitions */
#define SNES_SCR_WIDTH        256		/* 32 characters 8 pixels wide */
#define SNES_SCR_HEIGHT_NTSC  224		/* Can be 224 or 240 height */
#define SNES_SCR_HEIGHT_PAL   274		/* ??? */
#define SNES_VTOTAL_NTSC      262		/* Maximum number of lines for NTSC systems */
#define SNES_VTOTAL_PAL       312		/* Maximum number of lines for PAL systems */
#define SNES_HTOTAL           341		/* Maximum number pixels per line (incl. hblank) */
#define SNES_DMA_BASE         0x4300	/* Base DMA register address */
#define SNES_MODE_20          0x1		/* Lo-ROM cart */
#define SNES_MODE_21          0x2		/* Hi-ROM cart */
#define SNES_MODE_22          0x4		/* Extended Lo-ROM cart - SDD-1 */
#define SNES_MODE_25          0x8		/* Extended Hi-ROM cart */
#define SNES_NTSC             0x00
#define SNES_PAL              0x10
#define SNES_VRAM_SIZE        0x20000	/* 128kb of video ram */
#define SNES_CGRAM_SIZE       0x202		/* 256 16-bit colours + 1 tacked on 16-bit colour for fixed colour */
#define SNES_OAM_SIZE         0x440		/* 1088 bytes of Object Attribute Memory */
#define SNES_SPCRAM_SIZE      0x10000	/* 64kb of spc700 ram */
#define SNES_EXROM_START      0x1000000
#define FIXED_COLOUR          256		/* Position in cgram for fixed colour */
/* Definitions for PPU Memory-Mapped registers */
#define INIDISP        0x2100
#define OBSEL          0x2101
#define OAMADDL        0x2102
#define OAMADDH        0x2103
#define OAMDATA        0x2104
#define BGMODE         0x2105	/* abcdefff = abcd: bg4-1 tile size | e: BG3 high priority | f: mode */
#define MOSAIC         0x2106	/* xxxxabcd = x: pixel size | abcd: affects bg 1-4 */
#define BG1SC          0x2107
#define BG2SC          0x2108
#define BG3SC          0x2109
#define BG4SC          0x210A
#define BG12NBA        0x210B
#define BG34NBA        0x210C
#define BG1HOFS        0x210D
#define BG1VOFS        0x210E
#define BG2HOFS        0x210F
#define BG2VOFS        0x2110
#define BG3HOFS        0x2111
#define BG3VOFS        0x2112
#define BG4HOFS        0x2113
#define BG4VOFS        0x2114
#define VMAIN          0x2115	/* i---ffrr = i: Increment timing | f: Full graphic | r: increment rate */
#define VMADDL         0x2116	/* aaaaaaaa = a: LSB of vram address */
#define VMADDH         0x2117	/* aaaaaaaa = a: MSB of vram address */
#define VMDATAL        0x2118	/* dddddddd = d: data to be written */
#define VMDATAH        0x2119	/* dddddddd = d: data to be written */
#define M7SEL          0x211A	/* ab----yx = a: screen over | y: vertical flip | x: horizontal flip */
#define M7A            0x211B	/* aaaaaaaa = a: COSINE rotate angle / X expansion */
#define M7B            0x211C	/* aaaaaaaa = a: SINE rotate angle / X expansion */
#define M7C            0x211D	/* aaaaaaaa = a: SINE rotate angle / Y expansion */
#define M7D            0x211E	/* aaaaaaaa = a: COSINE rotate angle / Y expansion */
#define M7X            0x211F
#define M7Y            0x2120
#define CGADD          0x2121
#define CGDATA         0x2122
#define W12SEL         0x2123
#define W34SEL         0x2124
#define WOBJSEL        0x2125
#define WH0            0x2126	/* pppppppp = p: Left position of window 1 */
#define WH1            0x2127	/* pppppppp = p: Right position of window 1 */
#define WH2            0x2128	/* pppppppp = p: Left position of window 2 */
#define WH3            0x2129	/* pppppppp = p: Right position of window 2 */
#define WBGLOG         0x212A	/* aabbccdd = a: BG4 params | b: BG3 params | c: BG2 params | d: BG1 params */
#define WOBJLOG        0x212B	/* ----ccoo = c: Colour window params | o: Object window params */
#define TM             0x212C
#define TS             0x212D
#define TMW            0x212E
#define TSW            0x212F
#define CGWSEL         0x2130
#define CGADSUB        0x2131
#define COLDATA        0x2132
#define SETINI         0x2133
#define MPYL           0x2134
#define MPYM           0x2135
#define MPYH           0x2136
#define SLHV           0x2137
#define ROAMDATA       0x2138
#define RVMDATAL       0x2139
#define RVMDATAH       0x213A
#define RCGDATA        0x213B
#define OPHCT          0x213C
#define OPVCT          0x213D
#define STAT77         0x213E
#define STAT78         0x213F
#define APU00          0x2140
#define APU01          0x2141
#define APU02          0x2142
#define APU03          0x2143
#define WMDATA         0x2180
#define WMADDL         0x2181
#define WMADDM         0x2182
#define WMADDH         0x2183
/* Definitions for CPU Memory-Mapped registers */

extern UINT16 snes_cgram[SNES_CGRAM_SIZE];
extern UINT32 snesPal[0x20000];
/*SPC700*/
extern double spccycles;
extern double spctotal2;
extern double spctotal3;
void execspc();

static inline void clockspc(int cyc)
{
	spccycles+=cyc;
	if (spccycles>0) execspc();
}

/*65816*/
/*Registers*/
typedef union
{
	UINT16 w;
	struct
	{
#ifndef BIG_ENDIAN
		UINT8 l,h;
#else
		UINT8 h,l;
#endif
	} b;
} reg;


extern unsigned long pbr,dbr;
extern UINT16 pc,dp;



extern INT32 ins,output;
extern INT32 timetolive;
extern INT32 inwai;
/*Opcode table*/
extern void (*opcodes[256][5])();

/*CPU modes : 0 = X1M1
1 = X1M0
2 = X0M1
3 = X0M0
4 = emulation*/
extern INT32 cpumode;

/*Current opcode*/
extern UINT8 global_opcode;
/*Global cycles count*/
extern INT32 cycles;

/*Memory*/
extern UINT8 *SNES_ram;
extern UINT8 *SNES_rom;
extern UINT8 *memlookup[2048];
extern UINT8 *memread;
extern UINT8 *memwrite;
extern UINT8 *accessspeed;

extern INT32 lorom;


unsigned char snes_readmem(unsigned long adress);
void snes_writemem(unsigned long ad, unsigned char v);

#define readmemw(a) (snes_readmem(a))|((snes_readmem((a)+1))<<8)
#define writememw(a,v)  snes_writemem(a,(v)&0xFF); snes_writemem((a)+1,(v)>>8)
#define writememw2(a,v) snes_writemem((a)+1,(v)>>8); snes_writemem(a,(v)&0xFF)

/*Video*/
extern INT32 nmi,vbl,joyscan;
extern INT32 nmienable;

extern INT32 ppumask;

extern INT32 yirq,xirq,irqenable,irq;
extern INT32 lines;



extern int global_pal;

/*DMA registers*/
extern unsigned short dmadest[8],dmasrc[8],dmalen[8];
extern unsigned long hdmaaddr[8],hdmaaddr2[8];
extern unsigned char dmabank[8],dmaibank[8],dmactrl[8],hdmastat[8],hdmadat[8];
extern int hdmacount[8];
extern unsigned char hdmaena;

// debugging
void snemlog( TCHAR *format, ...);

// ppu stuff
void resetppu();
void initppu();
void initspc();
void makeopcodetable();
unsigned char readppu(unsigned short addr);
void writeppu(unsigned short addr, unsigned char val);
void drawline(int line);

// io stuff
void readjoy();
unsigned char readio(unsigned short addr);
unsigned char readjoyold(unsigned short addr);
void writeio(unsigned short addr, unsigned char val);
void writejoyold(unsigned short addr, unsigned char val);

struct CPU_P
{
	int c,z,i,d,b,v,n,m,x,e;
};

extern CPU_P p;

// spc stuff
unsigned char readfromspc(unsigned short addr);
void writetospc(unsigned short addr, unsigned char val);
void resetspc();

extern unsigned char voiceon;

//snes.cpp stuff



// cpu stuff
void irq65816();
void nmi65816();
void reset65816();
extern int skipz,setzf;
//mem stuff

void allocmem();

void initsnem();
void resetsnem();
void execframe();
void loadrom(char *fn);

void snes_mapmem();
void freemem();
extern unsigned short srammask;
extern unsigned char *SNES_sram;
extern INT32 spctotal;

// snes_main.cpp
INT32 SnesInit();
INT32 SnesExit();
INT32 SnesFrame();
INT32 SnesScan(INT32 nAction,INT32 *pnMin);
extern unsigned char DoSnesReset;

extern unsigned char SnesJoy1[12];
