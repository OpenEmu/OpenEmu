#ifndef GENS_MEM_Z80_H
#define GENS_MEM_Z80_H

#ifdef __cplusplus
extern "C" {
#endif

#if defined(__GNUC__) && !defined(__fastcall)
#define __fastcall __attribute__ ((regparm(2)))
#endif

extern unsigned char Ram_Z80[8 * 1024];
extern int Bank_Z80;

unsigned char Z80_ReadB(unsigned int Adr);
unsigned short Z80_ReadW(unsigned int Adr);
void Z80_WriteB(unsigned int Adr, unsigned char Data);
void Z80_WriteW(unsigned int Adr, unsigned short Data);

unsigned char __fastcall Z80_ReadB_Bad(unsigned int Adr);
unsigned char __fastcall Z80_ReadB_Ram(unsigned int Adr);
unsigned char __fastcall Z80_ReadB_Bank(unsigned int Adr);
unsigned char __fastcall Z80_ReadB_YM2612(unsigned int Adr);
unsigned char __fastcall Z80_ReadB_PSG(unsigned int Adr);
unsigned char __fastcall Z80_ReadB_68K_Ram(unsigned int Adr);

unsigned short __fastcall Z80_ReadW_Bad(unsigned int Adr);
unsigned short __fastcall Z80_ReadW_Ram(unsigned int Adr);
unsigned short __fastcall Z80_ReadW_Bank(unsigned int Adr);
unsigned short __fastcall Z80_ReadW_YM2612(unsigned int Adr);
unsigned short __fastcall Z80_ReadW_PSG(unsigned int Adr);
unsigned short __fastcall Z80_ReadW_68K_Ram(unsigned int Adr);

void __fastcall Z80_WriteB_Bad(unsigned int Adr, unsigned char Data);
void __fastcall Z80_WriteB_Ram(unsigned int Adr, unsigned char Data);
void __fastcall Z80_WriteB_Bank(unsigned int Adr, unsigned char Data);
void __fastcall Z80_WriteB_YM2612(unsigned int Adr, unsigned char Data);
void __fastcall Z80_WriteB_PSG(unsigned int Adr, unsigned char Data);
void __fastcall Z80_WriteB_68K_Ram(unsigned int Adr, unsigned char Data);

void __fastcall Z80_WriteW_Bad(unsigned int Adr, unsigned short Data);
void __fastcall Z80_WriteW_Bank(unsigned int Adr, unsigned short Data);
void __fastcall Z80_WriteW_Ram(unsigned int Adr, unsigned short Data);
void __fastcall Z80_WriteW_YM2612(unsigned int Adr, unsigned short Data);
void __fastcall Z80_WriteW_PSG(unsigned int Adr, unsigned short Data);
void __fastcall Z80_WriteW_68K_Ram(unsigned int Adr, unsigned short Data);

#ifdef __cplusplus
}
#endif

#endif /* GENS_MEM_Z80_H */
