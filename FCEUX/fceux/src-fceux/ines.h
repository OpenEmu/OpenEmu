/* FCE Ultra - NES/Famicom Emulator
 *
 * Copyright notice for this file:
 *  Copyright (C) 1998 Bero
 *  Copyright (C) 2002 Xodnizel
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

#ifndef _INES_H_
#define _INES_H_
#include <stdlib.h>
#include <string.h>
#ifdef INESPRIV

void iNESStateRestore(int version);
extern uint32 iNESGameCRC32;

extern uint32 VROM_size;
extern uint32 ROM_size;

extern void (*MapStateRestore)(int version);
extern void (*MapClose)(void);
extern void (*MapperReset)(void);

/* This order is necessary */
#define WRAM    (GameMemBlock)
#define sizeofWRAM    8192

#define MapperExRAM   (GameMemBlock+sizeofWRAM)
#define sizeofMapperExRAM  32768
/* for the MMC5 code to work properly.  It might be fixed later... */

#define CHRRAM  (GameMemBlock+sizeofWRAM+sizeofMapperExRAM)
#define sizeofCHRRAM 8192

#define ExtraNTARAM   (GameMemBlock+sizeofWRAM+sizeofMapperExRAM+sizeofCHRRAM)
#define sizeofExtraNTARAM 2048

#define PRGBankList    (ExtraNTARAM+sizeofExtraNTARAM)

#define mapbyte1       (PRGBankList+4)
#define mapbyte2       (mapbyte1+8)
#define mapbyte3       (mapbyte2+8)
#define mapbyte4       (mapbyte3+8)
extern uint16 iNESCHRBankList[8];
extern int32 iNESIRQLatch,iNESIRQCount;
extern uint8 iNESMirroring;
extern uint8 iNESIRQa;

#define IRQa iNESIRQa
#define Mirroring iNESMirroring
#define IRQCount iNESIRQCount
#define IRQLatch iNESIRQLatch
#define CHRBankList iNESCHRBankList
#else
#endif

//mbg merge 6/29/06
extern uint8 *ROM;
extern uint8 *VROM;
extern uint32 VROM_size;
extern uint32 ROM_size;
extern int iNesSave(); //bbit Edited: line added
extern int iNesSaveAs(char* name);
extern char LoadedRomFName[2048]; //bbit Edited: line added

//mbg merge 7/19/06 changed to c++ decl format
struct iNES_HEADER {
    char ID[4]; /*NES^Z*/
    uint8 ROM_size;
    uint8 VROM_size;
    uint8 ROM_type;
    uint8 ROM_type2;
    uint8 reserve[8];

	void cleanup()
	{
		if(!memcmp((char *)(this)+0x7,"DiskDude",8))
		{
			memset((char *)(this)+0x7,0,0x9);
		}

		if(!memcmp((char *)(this)+0x7,"demiforce",9))
		{
			memset((char *)(this)+0x7,0,0x9);
		}

		if(!memcmp((char *)(this)+0xA,"Ni03",4))
		{
			if(!memcmp((char *)(this)+0x7,"Dis",3))
				memset((char *)(this)+0x7,0,0x9);
			else
				memset((char *)(this)+0xA,0,0x6);
		}
	}
};
extern struct iNES_HEADER head; //for mappers usage

void VRAM_BANK1(uint32 A, uint8 V);
void VRAM_BANK4(uint32 A,uint32 V);

void VROM_BANK1(uint32 A,uint32 V);
void VROM_BANK2(uint32 A,uint32 V);
void VROM_BANK4(uint32 A, uint32 V);
void VROM_BANK8(uint32 V);
void ROM_BANK8(uint32 A, uint32 V);
void ROM_BANK16(uint32 A, uint32 V);
void ROM_BANK32(uint32 V);

extern uint8 vmask;
extern uint32 vmask1;
extern uint32 vmask2;
extern uint32 vmask4;
extern uint32 pmask8;
extern uint8 pmask16;
extern uint8 pmask32;

void onemir(uint8 V);
void MIRROR_SET2(uint8 V);
void MIRROR_SET(uint8 V);

void Mapper0_init(void);
void Mapper1_init(void);
//void Mapper2_init(void);
//void Mapper3_init(void);
void Mapper6_init(void);
//void Mapper7_init(void);
void Mapper8_init(void);
void Mapper9_init(void);
void Mapper10_init(void);
//void Mapper11_init(void);
void Mapper12_init(void);
//void Mapper13_init(void);
void Mapper14_init(void);
//void Mapper15_init(void);
//void Mapper16_init(void);
void Mapper17_init(void);
void Mapper18_init(void);
void Mapper19_init(void);
void Mapper20_init(void);
void Mapper21_init(void);
void Mapper22_init(void);
//void Mapper23_init(void);
void Mapper24_init(void);
void Mapper25_init(void);
void Mapper26_init(void);
void Mapper27_init(void);
void Mapper28_init(void);
void Mapper29_init(void);
void Mapper30_init(void);
void Mapper31_init(void);
void Mapper32_init(void);
void Mapper33_init(void);
void Mapper34_init(void);
void Mapper35_init(void);
void Mapper36_init(void);
//void Mapper37_init(void);
//void Mapper38_init(void);
//void Mapper39_init(void);
void Mapper40_init(void);
void Mapper41_init(void);
void Mapper42_init(void);
//void Mapper43_init(void);
void Mapper44_init(void);
void Mapper45_init(void);
void Mapper46_init(void);
void Mapper47_init(void);
void Mapper48_init(void);
void Mapper49_init(void);
void Mapper50_init(void);
void Mapper51_init(void);
void Mapper53_init(void);
void Mapper54_init(void);
void Mapper55_init(void);
void Mapper56_init(void);
void Mapper59_init(void);
void Mapper60_init(void);
void Mapper61_init(void);
void Mapper62_init(void);
void Mapper63_init(void);
void Mapper64_init(void);
void Mapper65_init(void);
//void Mapper66_init(void);
void Mapper67_init(void);
//void Mapper68_init(void);
void Mapper69_init(void);
//void Mapper70_init(void);
void Mapper71_init(void);
void Mapper72_init(void);
void Mapper73_init(void);
void Mapper74_init(void);
void Mapper75_init(void);
void Mapper76_init(void);
void Mapper77_init(void);
//void Mapper78_init(void);
void Mapper79_init(void);
void Mapper80_init(void);
void Mapper81_init(void);
void Mapper82_init(void);
void Mapper83_init(void);
void Mapper84_init(void);
void Mapper85_init(void);
void Mapper86_init(void);
//void Mapper87_init(void);
void Mapper88_init(void);
void Mapper89_init(void);
void Mapper91_init(void);
void Mapper92_init(void);
//void Mapper93_init(void);
//void Mapper94_init(void);
void Mapper96_init(void);
void Mapper97_init(void);
void Mapper98_init(void);
void Mapper99_init(void);
void Mapper100_init(void);
//void Mapper101_init(void);
//void Mapper103_init(void);
void Mapper104_init(void);
//void Mapper106_init(void);
//void Mapper107_init(void);
//void Mapper108_init(void);
void Mapper109_init(void);
void Mapper110_init(void);
void Mapper111_init(void);
//void Mapper113_init(void);
void Mapper115_init(void);
void Mapper116_init(void);
//void Mapper117_init(void);
//void Mapper120_init(void);
//void Mapper121_init(void);
void Mapper122_init(void);
void Mapper123_init(void);
void Mapper124_init(void);
void Mapper126_init(void);
void Mapper127_init(void);
void Mapper128_init(void);
void Mapper129_init(void);
void Mapper130_init(void);
void Mapper131_init(void);
void Mapper132_init(void);
//void Mapper134_init(void);
void Mapper135_init(void);
void Mapper136_init(void);
void Mapper137_init(void);
void Mapper139_init(void);
//void Mapper140_init(void);
void Mapper141_init(void);
//void Mapper142_init(void);
void Mapper143_init(void);
//void Mapper144_init(void);
void Mapper150_init(void);
void Mapper151_init(void);
//void Mapper152_init(void);
//void Mapper153_init(void);
void Mapper154_init(void);
void Mapper156_init(void);
void Mapper157_init(void);
//void Mapper158_init(void);
void Mapper159_init(void);
void Mapper160_init(void);
void Mapper161_init(void);
void Mapper162_init(void);
void Mapper166_init(void);
void Mapper167_init(void);
void Mapper168_init(void);
//void Mapper169_init(void);
void Mapper170_init(void);
//void Mapper171_init(void);
//void Mapper172_init(void);
//void Mapper173_init(void);
void Mapper174_init(void);
void Mapper175_init(void);
void Mapper176_init(void);
//void Mapper177_init(void);
//void Mapper178_init(void);
//void Mapper179_init(void);
//void Mapper180_init(void);
//void Mapper181_init(void);
//void Mapper184_init(void);
//void Mapper185_init(void);
//void Mapper189_init(void);
//void Mapper192_init(void);
void Mapper193_init(void);
//void Mapper194_init(void);
//void Mapper195_init(void);
//void Mapper196_init(void);
//void Mapper197_init(void);
//void Mapper198_init(void);
void Mapper199_init(void);
//void Mapper200_init(void);
void Mapper201_init(void);
void Mapper202_init(void);
void Mapper203_init(void);
void Mapper204_init(void);
void Mapper207_init(void);
//void Mapper211_init(void);
void Mapper212_init(void);
void Mapper213_init(void);
void Mapper214_init(void);
//void Mapper218_init(void);
void Mapper219_init(void);
//void Mapper220_init(void);
void Mapper221_init(void);
//void Mapper222_init(void);
void Mapper223_init(void);
void Mapper224_init(void);
void Mapper225_init(void);
//void Mapper226_init(void);
void Mapper227_init(void);
void Mapper228_init(void);
void Mapper229_init(void);
void Mapper230_init(void);
void Mapper231_init(void);
void Mapper232_init(void);
void Mapper233_init(void);
void Mapper234_init(void);
//void Mapper235_init(void);
void Mapper236_init(void);
void Mapper237_init(void);
void Mapper238_init(void);
void Mapper239_init(void);
void Mapper240_init(void);
void Mapper241_init(void);
void Mapper242_init(void);
void Mapper244_init(void);
void Mapper245_init(void);
void Mapper246_init(void);
void Mapper247_init(void);
void Mapper249_init(void);
void Mapper251_init(void);
void Mapper252_init(void);
//void Mapper253_init(void);
void Mapper255_init(void);

void NSFVRC6_Init(void);
void NSFMMC5_Init(void);
void NSFAY_Init(void);
void NSFN106_Init(void);
void NSFVRC7_Init(void);
void Mapper19_ESI(void);

void Mapper1_Init(CartInfo *);
void Mapper4_Init(CartInfo *);
void Mapper5_Init(CartInfo *);
void Mapper11_Init(CartInfo *);
void Mapper12_Init(CartInfo *);
void Mapper15_Init(CartInfo *);
void Mapper16_Init(CartInfo *);
void Mapper19_Init(CartInfo *);
void Mapper23_Init(CartInfo *);
void Mapper36_Init(CartInfo *);
void Mapper37_Init(CartInfo *);
void Mapper38_Init(CartInfo *);
void Mapper43_Init(CartInfo *);
void Mapper44_Init(CartInfo *);
void Mapper45_Init(CartInfo *);
void Mapper47_Init(CartInfo *);
void Mapper49_Init(CartInfo *);
void Mapper52_Init(CartInfo *);
void Mapper57_Init(CartInfo *);
//void Mapper58_Init(CartInfo *);
void Mapper68_Init(CartInfo *);
void Mapper70_Init(CartInfo *);
void Mapper74_Init(CartInfo *);
void Mapper78_Init(CartInfo *);
void Mapper87_Init(CartInfo *);
void Mapper88_Init(CartInfo *);
void Mapper90_Init(CartInfo *);
void Mapper93_Init(CartInfo *);
void Mapper94_Init(CartInfo *);
void Mapper95_Init(CartInfo *);
void Mapper101_Init(CartInfo *);
void Mapper103_Init(CartInfo *);
void Mapper105_Init(CartInfo *);
void Mapper106_Init(CartInfo *);
void Mapper107_Init(CartInfo *);
void Mapper108_Init(CartInfo *);
void Mapper112_Init(CartInfo *);
void Mapper113_Init(CartInfo *);
void Mapper114_Init(CartInfo *);
void Mapper115_Init(CartInfo *);
void Mapper116_Init(CartInfo *);
void Mapper117_Init(CartInfo *);
void Mapper118_Init(CartInfo *);
void Mapper119_Init(CartInfo *);
void Mapper120_Init(CartInfo *);
void Mapper121_Init(CartInfo *);
void Mapper125_Init(CartInfo *);
void Mapper134_Init(CartInfo *);
void Mapper140_Init(CartInfo *);
void Mapper144_Init(CartInfo *);
void Mapper152_Init(CartInfo *);
void Mapper153_Init(CartInfo *);
void Mapper154_Init(CartInfo *);
void Mapper155_Init(CartInfo *);
void Mapper163_Init(CartInfo *);
void Mapper164_Init(CartInfo *);
void Mapper165_Init(CartInfo *);
//void Mapper169_Init(CartInfo *);
void Mapper171_Init(CartInfo *);
void Mapper172_Init(CartInfo *);
void Mapper173_Init(CartInfo *);
void Mapper175_Init(CartInfo *);
void Mapper177_Init(CartInfo *);
void Mapper178_Init(CartInfo *);
void Mapper180_Init(CartInfo *);
void Mapper181_Init(CartInfo *);
void Mapper182_Init(CartInfo *);
void Mapper183_Init(CartInfo *);
void Mapper184_Init(CartInfo *);
void Mapper185_Init(CartInfo *);
void Mapper186_Init(CartInfo *);
void Mapper187_Init(CartInfo *);
void Mapper188_Init(CartInfo *);
void Mapper189_Init(CartInfo *);
void Mapper191_Init(CartInfo *);
void Mapper192_Init(CartInfo *);
void Mapper194_Init(CartInfo *);
void Mapper195_Init(CartInfo *);
void Mapper196_Init(CartInfo *);
void Mapper197_Init(CartInfo *);
void Mapper198_Init(CartInfo *);
void Mapper199_Init(CartInfo *);
void Mapper200_Init(CartInfo *);
void Mapper205_Init(CartInfo *);
void Mapper208_Init(CartInfo *);
void Mapper209_Init(CartInfo *);
void Mapper210_Init(CartInfo *);
void Mapper211_Init(CartInfo *);
void Mapper215_Init(CartInfo *);
void Mapper216_Init(CartInfo *);
void Mapper217_Init(CartInfo *);
void Mapper220_Init(CartInfo *);
void Mapper222_Init(CartInfo *);
void Mapper226_Init(CartInfo *);
void Mapper235_Init(CartInfo *);
void Mapper236_Init(CartInfo *);
void Mapper237_Init(CartInfo *);
void Mapper240_Init(CartInfo *);
void Mapper245_Init(CartInfo *);
void Mapper249_Init(CartInfo *);
void Mapper250_Init(CartInfo *);
void Mapper253_Init(CartInfo *);
void Mapper254_Init(CartInfo *);

#endif
