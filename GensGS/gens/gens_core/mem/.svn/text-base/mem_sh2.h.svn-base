#ifndef GENS_MEM_SH2_H
#define GENS_MEM_SH2_H

#ifdef __cplusplus
extern "C" {
#endif

#include "gens_core/cpu/sh2/sh2.h"

extern unsigned char _32X_Rom[4 * 1024 * 1024];
extern unsigned char _32X_Ram[256 * 1024];
extern unsigned char _32X_MSH2_Rom[2 * 1024];
extern unsigned char _32X_SSH2_Rom[1 * 1024];

extern unsigned char _MSH2_Reg[0x40];
extern unsigned char _SSH2_Reg[0x40];
extern unsigned char _SH2_VDP_Reg[0x10];

extern unsigned char _32X_Comm[0x10];
extern unsigned char _32X_ADEN;
extern unsigned char _32X_RES;
extern unsigned char _32X_FM;
extern unsigned char _32X_RV;

extern unsigned int _32X_DREQ_ST;
extern unsigned int _32X_DREQ_SRC;
extern unsigned int _32X_DREQ_DST;
extern unsigned int _32X_DREQ_LEN;
extern unsigned short _32X_FIFO_A[4];
extern unsigned short _32X_FIFO_B[4];
extern unsigned int _32X_FIFO_Block;
extern unsigned int _32X_FIFO_Read;
extern unsigned int _32X_FIFO_Write;

extern unsigned char _32X_MINT;
extern unsigned char _32X_SINT;
extern unsigned char _32X_HIC;

extern unsigned int CPL_SSH2;
extern unsigned int CPL_MSH2;
extern int Cycles_MSH2;
extern int Cycles_SSH2;


UINT8 FASTCALL MSH2_Read_Byte_00(UINT32 adr);
UINT8 FASTCALL SSH2_Read_Byte_00(UINT32 adr);
UINT16 FASTCALL MSH2_Read_Word_00(UINT32 adr);
UINT16 FASTCALL SSH2_Read_Word_00(UINT32 adr);
UINT32 FASTCALL MSH2_Read_Long_00(UINT32 adr);
UINT32 FASTCALL SSH2_Read_Long_00(UINT32 adr);

UINT8 FASTCALL SH2_Read_Byte_Rom(UINT32 adr);
UINT16 FASTCALL SH2_Read_Word_Rom(UINT32 adr);
UINT32 FASTCALL SH2_Read_Long_Rom(UINT32 adr);

UINT8 FASTCALL SH2_Read_Byte_FB0(UINT32 adr);
UINT8 FASTCALL SH2_Read_Byte_FB1(UINT32 adr);
UINT16 FASTCALL SH2_Read_Word_FB0(UINT32 adr);
UINT16 FASTCALL SH2_Read_Word_FB1(UINT32 adr);
UINT32 FASTCALL SH2_Read_Long_FB0(UINT32 adr);
UINT32 FASTCALL SH2_Read_Long_FB1(UINT32 adr);

UINT8 FASTCALL SH2_Read_Byte_Ram(UINT32 adr);
UINT16 FASTCALL SH2_Read_Word_Ram(UINT32 adr);
UINT32 FASTCALL SH2_Read_Long_Ram(UINT32 adr);


void FASTCALL MSH2_Write_Byte_00(UINT32 adr, UINT8 data);
void FASTCALL SSH2_Write_Byte_00(UINT32 adr, UINT8 data);
void FASTCALL MSH2_Write_Word_00(UINT32 adr, UINT16 data);
void FASTCALL SSH2_Write_Word_00(UINT32 adr, UINT16 data);
void FASTCALL MSH2_Write_Long_00(UINT32 adr, UINT32 data);
void FASTCALL SSH2_Write_Long_00(UINT32 adr, UINT32 data);

void FASTCALL SH2_Write_Byte_FB0(UINT32 adr, UINT8 data);
void FASTCALL SH2_Write_Byte_FB1(UINT32 adr, UINT8 data);
void FASTCALL SH2_Write_Word_FB0(UINT32 adr, UINT16 data);
void FASTCALL SH2_Write_Word_FB1(UINT32 adr, UINT16 data);
void FASTCALL SH2_Write_Long_FB0(UINT32 adr, UINT32 data);
void FASTCALL SH2_Write_Long_FB1(UINT32 adr, UINT32 data);

void FASTCALL SH2_Write_Byte_Ram(UINT32 adr, UINT8 data);
void FASTCALL SH2_Write_Word_Ram(UINT32 adr, UINT16 data);
void FASTCALL SH2_Write_Long_Ram(UINT32 adr, UINT32 data);

#ifdef __cplusplus
}
#endif

#endif /* GENS_MEM_SH2_H */
