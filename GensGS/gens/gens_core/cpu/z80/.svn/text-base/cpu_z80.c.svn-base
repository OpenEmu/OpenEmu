#include <stdio.h>
#include <string.h>
#include "z80.h"
#include "cpu_z80.h"
#include "gens_core/mem/mem_m68k.h"
#include "gens_core/mem/mem_z80.h"


/**
 * Z80_Reset(): Reset the Z80 CPU.
 */
void Z80_Reset(void)
{
	memset(Ram_Z80, 0, 8 * 1024);
	
	//Bank_Z80 = 0x000000;
	Bank_Z80 = 0xFF8000;
	
	Z80_State &= 1;
	//Z80_State |= 2;         // RZWL needs it but Breath of Fire 3 don't want it...
	
	Last_BUS_REQ_Cnt = 0;
	Last_BUS_REQ_St = 0;
	
	z80_Reset(&M_Z80);
}


/**
 * Z80_Init(): Initialize the Z80 CPU.
 * @return 1 on success.
 */
int Z80_Init(void)
{
	z80_Init (&M_Z80);
	
	z80_Add_Fetch(&M_Z80, 0x00, 0x1F, &Ram_Z80[0]);
	z80_Add_Fetch(&M_Z80, 0x20, 0x3F, &Ram_Z80[0]);
	
	z80_Add_ReadB(&M_Z80, 0x00, 0xFF, Z80_ReadB_Bad);
	z80_Add_ReadB(&M_Z80, 0x00, 0x3F, Z80_ReadB_Ram);
	z80_Add_ReadB(&M_Z80, 0x40, 0x5F, Z80_ReadB_YM2612);
	z80_Add_ReadB(&M_Z80, 0x60, 0x6F, Z80_ReadB_Bank);
	z80_Add_ReadB(&M_Z80, 0x70, 0x7F, Z80_ReadB_PSG);
	z80_Add_ReadB(&M_Z80, 0x80, 0xFF, Z80_ReadB_68K_Ram);
	
	z80_Add_ReadW(&M_Z80, 0x00, 0xFF, Z80_ReadW_Bad);
	z80_Add_ReadW(&M_Z80, 0x00, 0x3F, Z80_ReadW_Ram);
	z80_Add_ReadW(&M_Z80, 0x40, 0x5F, Z80_ReadW_YM2612);
	z80_Add_ReadW(&M_Z80, 0x60, 0x6F, Z80_ReadW_Bank);
	z80_Add_ReadW(&M_Z80, 0x70, 0x7F, Z80_ReadW_PSG);
	z80_Add_ReadW(&M_Z80, 0x80, 0xFF, Z80_ReadW_68K_Ram);
	
	z80_Add_WriteB(&M_Z80, 0x00, 0xFF, Z80_WriteB_Bad);
	z80_Add_WriteB(&M_Z80, 0x00, 0x3F, Z80_WriteB_Ram);
	z80_Add_WriteB(&M_Z80, 0x40, 0x5F, Z80_WriteB_YM2612);
	z80_Add_WriteB(&M_Z80, 0x60, 0x6F, Z80_WriteB_Bank);
	z80_Add_WriteB(&M_Z80, 0x70, 0x7F, Z80_WriteB_PSG);
	z80_Add_WriteB(&M_Z80, 0x80, 0xFF, Z80_WriteB_68K_Ram);
	
	z80_Add_WriteW(&M_Z80, 0x00, 0xFF, Z80_WriteW_Bad);
	z80_Add_WriteW(&M_Z80, 0x00, 0x3F, Z80_WriteW_Ram);
	z80_Add_WriteW(&M_Z80, 0x40, 0x5F, Z80_WriteW_YM2612);
	z80_Add_WriteW(&M_Z80, 0x60, 0x6F, Z80_WriteW_Bank);
	z80_Add_WriteW(&M_Z80, 0x70, 0x7F, Z80_WriteW_PSG);
	z80_Add_WriteW(&M_Z80, 0x80, 0xFF, Z80_WriteW_68K_Ram);
	
	Z80_Reset();
	
	return 1;
}


/** Stub functions for SegaCD debugging. **/

#include "segacd/cd_sys.hpp"

void Write_To_Bank(int val)
{
#ifdef DEBUG_CD
	fprintf(debug_SCD_file, "Z80 write bank : %d     Bank = %.8X\n", val, Bank_Z80);
#endif
}

void Read_To_68K_Space(int adr)
{
#ifdef DEBUG_CD
	fprintf(debug_SCD_file, "Z80 read in 68K space : $(%.8X)\n", adr);
#endif
}

void Write_To_68K_Space (int adr, int data)
{
#ifdef DEBUG_CD
	fprintf(debug_SCD_file, "Z80 write in 68K space : $(%.8X) = %.8X\n", adr, data);
#endif
}
