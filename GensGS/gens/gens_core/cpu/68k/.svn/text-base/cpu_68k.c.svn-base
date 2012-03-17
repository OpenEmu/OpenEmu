#include <stdio.h>
#include <string.h>
#include "cpu_68k.h"

#include "gens_core/mem/mem_m68k.h"
#include "gens_core/mem/mem_m68k_cd.h"
#include "gens_core/mem/mem_m68k_32x.h"
#include "gens_core/mem/mem_s68k.h"
#include "gens_core/mem/mem_sh2.h"

#include "util/file/save.hpp"
#include "gens_core/sound/ym2612.h"
#include "gens_core/misc/misc.h"
#include "gens_core/gfx/gfx_cd.h"

#include "gens_core/io/io.h"
#include "segacd/cd_sys.hpp"

#define GENESIS 0
#define _32X    1
#define SEGACD  2


/** global variables **/

// TODO: This is not 64-bit clean!

struct S68000CONTEXT Context_68K;

/** Main 68000 **/

struct STARSCREAM_PROGRAMREGION M68K_Fetch[] =
{
	{0x000000, 0x3FFFFF, (unsigned int)0x000000},
	{0xFF0000, 0xFFFFFF, (unsigned int)&Ram_68k[0] - 0xFF0000},
	{0xF00000, 0xF0FFFF, (unsigned int)&Ram_68k[0] - 0xF00000},
	{0xEF0000, 0xEFFFFF, (unsigned int)&Ram_68k[0] - 0xEF0000},
	{-1, -1, (unsigned int)NULL},
	{-1, -1, (unsigned int)NULL},
	{-1, -1, (unsigned int)NULL}
};

struct STARSCREAM_DATAREGION M68K_Read_Byte[5] =
{
	{0x000000, 0x3FFFFF, NULL, NULL},
	{0xFF0000, 0xFFFFFF, NULL, &Ram_68k[0]},
	{0x400000, 0xFEFFFF, (void*)M68K_RB, NULL},
	{-1, -1, NULL, NULL}
};

struct STARSCREAM_DATAREGION M68K_Read_Word[5] =
{
	{0x000000, 0x3FFFFF, NULL, NULL},
	{0xFF0000, 0xFFFFFF, NULL, &Ram_68k[0]},
	{0x400000, 0xFEFFFF, (void*)M68K_RW, NULL},
	{-1, -1, NULL, NULL}
};

struct STARSCREAM_DATAREGION M68K_Write_Byte[] =
{
	{0xFF0000, 0xFFFFFF, NULL, &Ram_68k[0]},
	{0x000000, 0xFEFFFF, (void*)M68K_WB, NULL},
	{-1, -1, NULL, NULL}
};

struct STARSCREAM_DATAREGION M68K_Write_Word[] =
{
	{0xFF0000, 0xFFFFFF, NULL, &Ram_68k[0]},
	{0x000000, 0xFEFFFF, (void*)M68K_WW, NULL},
	{-1, -1, NULL, NULL}
};

/** Sub 68000 **/

struct STARSCREAM_PROGRAMREGION S68K_Fetch[] =
{
	{0x000000, 0x07FFFF, (unsigned int)&Ram_Prg[0]},
	{-1, -1, (unsigned int)NULL},
	{-1, -1, (unsigned int)NULL}
};

struct STARSCREAM_DATAREGION S68K_Read_Byte[] =
{
	{0x000000, 0x07FFFF, NULL, &Ram_Prg[0]},
	{0x080000, 0xFFFFFF, (void*)S68K_RB, NULL},
	{-1, -1, NULL, NULL}
};

struct STARSCREAM_DATAREGION S68K_Read_Word[] =
{
	{0x000000, 0x07FFFF, NULL, &Ram_Prg[0]},
	{0x080000, 0xFFFFFF, (void*)S68K_RW, NULL},
	{-1, -1, NULL, NULL}
};

struct STARSCREAM_DATAREGION S68K_Write_Byte[] =
{
	{0x000000, 0x07FFFF, NULL, &Ram_Prg[0]},
	{0x080000, 0xFFFFFF, (void*) S68K_WB, NULL},
	{-1, -1, NULL, NULL}
};

struct STARSCREAM_DATAREGION S68K_Write_Word[] =
{
	{0x000000, 0x07FFFF, NULL, &Ram_Prg[0]},
	{0x080000, 0xFFFFFF, (void*) S68K_WW, NULL},
	{-1, -1, NULL, NULL}
};


void M68K_Reset_Handler(void)
{
	//Init_Memory_M68K(GENESIS);
}


void S68K_Reset_Handler(void)
{
	//Init_Memory_M68K(SEGACD);
}


/**
 * M68K_Init(): Initialize the Main 68000.
 * @return 1 on success; 0 on error.
 */
int M68K_Init(void)
{
	memset(&Context_68K, 0, sizeof(Context_68K));
	
	Context_68K.s_fetch = Context_68K.u_fetch =
		Context_68K.fetch = M68K_Fetch;
	
	Context_68K.s_readbyte = Context_68K.u_readbyte =
		Context_68K.readbyte = M68K_Read_Byte;
	
	Context_68K.s_readword = Context_68K.u_readword =
		Context_68K.readword = M68K_Read_Word;
	
	Context_68K.s_writebyte = Context_68K.u_writebyte =
		Context_68K.writebyte = M68K_Write_Byte;
	
	Context_68K.s_writeword = Context_68K.u_writeword =
		Context_68K.writeword = M68K_Write_Word;
	
	Context_68K.resethandler = (void*)M68K_Reset_Handler;
	
	main68k_SetContext(&Context_68K);
	main68k_init();
	
	return 1;
}


/**
 * S68K_Init(): Initialize the Sub 68000.
 * @return 1 on success; 0 on error.
 */
int S68K_Init(void)
{
	memset(&Context_68K, 0, sizeof(Context_68K));
	
	Context_68K.s_fetch = Context_68K.u_fetch =
		Context_68K.fetch = S68K_Fetch;
	
	Context_68K.s_readbyte = Context_68K.u_readbyte =
		Context_68K.readbyte = S68K_Read_Byte;
	
	Context_68K.s_readword = Context_68K.u_readword =
		Context_68K.readword = S68K_Read_Word;
	
	Context_68K.s_writebyte = Context_68K.u_writebyte =
		Context_68K.writebyte = S68K_Write_Byte;
	
	Context_68K.s_writeword = Context_68K.u_writeword =
		Context_68K.writeword = S68K_Write_Word;
	
	Context_68K.resethandler = (void*)S68K_Reset_Handler;
	
	sub68k_SetContext(&Context_68K);
	sub68k_init();
	
	return 1;
}


/**
 * M68K_Reset(): General reset of the Main 68000 CPU.
 * @param System_ID System ID. [TODO: Make this an enum and/or a bitfield.]
 */
void M68K_Reset(int System_ID)
{
	// TODO: This is not 64-bit clean!
	
	memset(Ram_68k, 0, 64 * 1024);
	
	M68K_Fetch[0].lowaddr = 0x000000;
	M68K_Fetch[0].highaddr = Rom_Size - 1;
	M68K_Fetch[0].offset = (unsigned int)&Rom_Data[0] - 0x000000;
	
	M68K_Fetch[1].lowaddr = 0xFF0000;
	M68K_Fetch[1].highaddr = 0xFFFFFF;
	M68K_Fetch[1].offset = (unsigned int)&Ram_68k[0] - 0xFF0000;
	
	if (System_ID == GENESIS)
	{
		M68K_Fetch[2].lowaddr = 0xF00000;
		M68K_Fetch[2].highaddr = 0xF0FFFF;
		M68K_Fetch[2].offset = (unsigned int)&Ram_68k[0] - 0xF00000;
		
		M68K_Fetch[3].lowaddr = 0xEF0000;
		M68K_Fetch[3].highaddr = 0xEFFFFF;
		M68K_Fetch[3].offset = (unsigned int)&Ram_68k[0] - 0xEF0000;
		
		M68K_Fetch[4].lowaddr = -1;
		M68K_Fetch[4].highaddr = -1;
		M68K_Fetch[4].offset = (unsigned int)NULL;
	}
	else if (System_ID == _32X)
	{
		Bank_SH2 = 0;
		
		M68K_Fetch[2].lowaddr = 0xF00000;
		M68K_Fetch[2].highaddr = 0xF0FFFF;
		M68K_Fetch[2].offset = (unsigned int)&Ram_68k[0] - 0xF00000;
		
		M68K_Fetch[3].lowaddr = 0xEF0000;
		M68K_Fetch[3].highaddr = 0xEFFFFF;
		M68K_Fetch[3].offset = (unsigned int)&Ram_68k[0] - 0xEF0000;
		
		M68K_Fetch[4].lowaddr = -1;
		M68K_Fetch[4].highaddr = -1;
		M68K_Fetch[4].offset = (unsigned int)NULL;
	}
	else if (System_ID == SEGACD)
	{
		Bank_M68K = 0;
		
		MS68K_Set_Word_Ram();
		
		M68K_Fetch[3].lowaddr = 0x020000;
		M68K_Fetch[3].highaddr = 0x03FFFF;
		M68K_Set_Prg_Ram();
		
		M68K_Fetch[4].lowaddr = 0xF00000;
		M68K_Fetch[4].highaddr = 0xF0FFFF;
		M68K_Fetch[4].offset = (unsigned int)&Ram_68k[0] - 0xF00000;
		
		M68K_Fetch[5].lowaddr = 0xEF0000;
		M68K_Fetch[5].highaddr = 0xEFFFFF;
		M68K_Fetch[5].offset = (unsigned int)&Ram_68k[0] - 0xEF0000;
		
		M68K_Fetch[6].lowaddr = -1;
		M68K_Fetch[6].highaddr = -1;
		M68K_Fetch[6].offset = (unsigned int)NULL;
	}
	
	main68k_reset();
	Init_Memory_M68K(System_ID);
}


/**
 * M68K_Reset(): General reset of the Sub 68000 CPU.
 */
void S68K_Reset(void)
{
	memset(Ram_Prg,     0x00, 512 * 1024);
	memset(Ram_Word_2M, 0x00, 256 * 1024);
	memset(Ram_Word_1M, 0x00, 256 * 1024);
	
	memset(COMM.Command, 0x00, 8 * 5);
	memset(COMM.Status,  0x00, 8 * 5);
	
	LED_Status = S68K_State = S68K_Mem_WP = S68K_Mem_PM = Ram_Word_State = 0;
	COMM.Flag = Init_Timer_INT3 = Timer_INT3 = Int_Mask_S68K = 0;
	Font_COLOR = Font_BITS = 0;
	
	MS68K_Set_Word_Ram();
	sub68k_reset();
}


/**
 * M68K_32X_Mode(): Modify 32X mode.
 * This function is only called during 32X emulation.
 */
void M68K_32X_Mode(void)
{
	//if (_32X_ADEN && !_32X_RV) // 32X ON
	if (_32X_ADEN)
	{
		// 32X ON
		if (!_32X_RV)
		{
			// ROM MOVED
			M68K_Fetch[0].lowaddr = 0x880000;
			M68K_Fetch[0].highaddr = 0x8FFFFF;
			M68K_Fetch[0].offset = (unsigned int)&Rom_Data[0] - 0x880000;
			
			M68K_Fetch[1].lowaddr = 0x900000;
			M68K_Fetch[1].highaddr = 0x9FFFFF;
			M68K_Set_32X_Rom_Bank();
			
			M68K_Fetch[2].lowaddr = 0xFF0000;
			M68K_Fetch[2].highaddr = 0xFFFFFF;
			M68K_Fetch[2].offset = (unsigned int)&Ram_68k[0] - 0xFF0000;
			
			M68K_Fetch[3].lowaddr = 0x00;
			M68K_Fetch[3].highaddr = 0xFF;
			M68K_Fetch[3].offset = (unsigned int)&_32X_Genesis_Rom[0] - 0x000000;
			
			M68K_Fetch[4].lowaddr = 0xEF0000;
			M68K_Fetch[4].highaddr = 0xEFFFFF;
			M68K_Fetch[4].offset = (unsigned int)&Ram_68k[0] - 0xEF0000;
			
			M68K_Fetch[5].lowaddr = 0xF00000;
			M68K_Fetch[5].highaddr = 0xF0FFFF;
			M68K_Fetch[5].offset = (unsigned int)&Ram_68k[0] - 0xF00000;
			
			M68K_Fetch[6].lowaddr = -1;
			M68K_Fetch[6].highaddr = -1;
			M68K_Fetch[6].offset = (unsigned int)NULL;
			
			M68K_Read_Byte_Table[0] = _32X_M68K_Read_Byte_Table[4 * 2];
			M68K_Read_Word_Table[0] = _32X_M68K_Read_Word_Table[4 * 2];
		}
		else
		{
			// ROM NOT MOVED BUT BIOS PRESENT
			M68K_Fetch[0].lowaddr = 0x000100;
			M68K_Fetch[0].highaddr = Rom_Size - 1;
			M68K_Fetch[0].offset = (unsigned int)&Rom_Data[0] - 0x000000;
			
			M68K_Fetch[1].lowaddr = 0xFF0000;
			M68K_Fetch[1].highaddr = 0xFFFFFF;
			M68K_Fetch[1].offset = (unsigned int)&Ram_68k[0] - 0xFF0000;
			
			M68K_Fetch[2].lowaddr = 0x00;
			M68K_Fetch[2].highaddr = 0xFF;
			M68K_Fetch[2].offset = (unsigned int)&_32X_Genesis_Rom[0] - 0x000000;
			
			M68K_Fetch[3].lowaddr = 0xF00000;
			M68K_Fetch[3].highaddr = 0xF0FFFF;
			M68K_Fetch[3].offset = (unsigned int)&Ram_68k[0] - 0xF00000;
			
			M68K_Fetch[4].lowaddr = 0xEF0000;
			M68K_Fetch[4].highaddr = 0xEFFFFF;
			M68K_Fetch[4].offset = (unsigned int)&Ram_68k[0] - 0xEF0000;
			
			M68K_Fetch[5].lowaddr = -1;
			M68K_Fetch[5].highaddr = -1;
			M68K_Fetch[5].offset = (unsigned int)NULL;
			
			M68K_Read_Byte_Table[0] = _32X_M68K_Read_Byte_Table[4 * 2 + 1];
			M68K_Read_Word_Table[0] = _32X_M68K_Read_Word_Table[4 * 2 + 1];
		}
	}
	else
	{
		// 32X OFF
		M68K_Fetch[0].lowaddr = 0x000000;
		M68K_Fetch[0].highaddr = Rom_Size - 1;
		M68K_Fetch[0].offset = (unsigned int)&Rom_Data[0] - 0x000000;
		
		M68K_Fetch[1].lowaddr = 0xFF0000;
		M68K_Fetch[1].highaddr = 0xFFFFFF;
		M68K_Fetch[1].offset = (unsigned int)&Ram_68k[0] - 0xFF0000;
		
		M68K_Fetch[2].lowaddr = 0xF00000;
		M68K_Fetch[2].highaddr = 0xF0FFFF;
		M68K_Fetch[2].offset = (unsigned int)&Ram_68k[0] - 0xF00000;
		
		M68K_Fetch[3].lowaddr = 0xEF0000;
		M68K_Fetch[3].highaddr = 0xEFFFFF;
		M68K_Fetch[3].offset = (unsigned int)&Ram_68k[0] - 0xEF0000;
		
		M68K_Fetch[4].lowaddr = -1;
		M68K_Fetch[4].highaddr = -1;
		M68K_Fetch[4].offset = (unsigned int)NULL;
		
		M68K_Read_Byte_Table[0] = _32X_M68K_Read_Byte_Table[0];
		M68K_Read_Word_Table[0] = _32X_M68K_Read_Word_Table[0];
	}
}


/**
 * M68K_32X_Mode(): Modify 32X ROM bank.
 * This function is only called during 32X emulation.
 */
void M68K_Set_32X_Rom_Bank(void)
{
	if (!(_32X_ADEN && !_32X_RV))
		return;
		
	M68K_Fetch[1].offset = (unsigned int)&Rom_Data[Bank_SH2 << 20] - 0x900000;
	
	M68K_Read_Byte_Table[(9 * 2) + 0] =
		_32X_M68K_Read_Byte_Table[(Bank_SH2 << 1) + 0];
	M68K_Read_Byte_Table[(9 * 2) + 1] =
		_32X_M68K_Read_Byte_Table[(Bank_SH2 << 1) + 1];
	M68K_Read_Word_Table[(9 * 2) + 0] =
		_32X_M68K_Read_Word_Table[(Bank_SH2 << 1) + 0];
	M68K_Read_Word_Table[(9 * 2) + 1] =
		_32X_M68K_Read_Word_Table[(Bank_SH2 << 1) + 1];
}


/**
 * M68K_Set_Prg_Ram(): Modify bank Prg_Ram fetch.
 * This function is only called during SegaCD emulation.
 */
void M68K_Set_Prg_Ram(void)
{
	M68K_Fetch[3].offset = (unsigned int)&Ram_Prg[Bank_M68K] - 0x020000;
}


/**
 * MS68K_Set_Word_Ram(): Modify bank Word_Ram fetch.
 * This function is only called during SegaCD emulation.
 */
void MS68K_Set_Word_Ram(void)
{
	switch (Ram_Word_State)
	{
		case 0:
			// Mode 2M -> Assigned to Main CPU
			M68K_Fetch[2].lowaddr = 0x200000;
			M68K_Fetch[2].highaddr = 0x23FFFF;
			M68K_Fetch[2].offset = (unsigned int)&Ram_Word_2M[0] - 0x200000;
			
			//S68K_Fetch[1].lowaddr = -1;
			//S68K_Fetch[1].highaddr = -1;
			//S68K_Fetch[1].offset = (unsigned) NULL;
			
			S68K_Fetch[1].lowaddr = 0x080000;	// why not after all...
			S68K_Fetch[1].highaddr = 0x0BFFFF;
			S68K_Fetch[1].offset = (unsigned int)&Ram_Word_2M[0] - 0x080000;
			break;
		
		case 1:
			// Mode 2M -> Assigned to Sub CPU
			//M68K_Fetch[2].lowaddr = -1;
			//M68K_Fetch[2].highaddr = -1;
			//M68K_Fetch[2].offset = (unsigned) NULL;
			
			M68K_Fetch[2].lowaddr = 0x200000;	// why not after all...
			M68K_Fetch[2].highaddr = 0x23FFFF;
			M68K_Fetch[2].offset = (unsigned int)&Ram_Word_2M[0] - 0x200000;
			
			S68K_Fetch[1].lowaddr = 0x080000;
			S68K_Fetch[1].highaddr = 0x0BFFFF;
			S68K_Fetch[1].offset = (unsigned int)&Ram_Word_2M[0] - 0x080000;
			break;
		
		case 2:
			// Mode 1M -> Bank 0 to Main CPU
			M68K_Fetch[2].lowaddr = 0x200000;	// Bank 0
			M68K_Fetch[2].highaddr = 0x21FFFF;
			M68K_Fetch[2].offset = (unsigned int)&Ram_Word_1M[0] - 0x200000;
			
			S68K_Fetch[1].lowaddr = 0x0C0000;	// Bank 1
			S68K_Fetch[1].highaddr = 0x0DFFFF;
			S68K_Fetch[1].offset = (unsigned int)&Ram_Word_1M[0x20000] - 0x0C0000;
			break;
		
		case 3:
			// Mode 1M -> Bank 0 to Sub CPU
			M68K_Fetch[2].lowaddr = 0x200000;	// Bank 1
			M68K_Fetch[2].highaddr = 0x21FFFF;
			M68K_Fetch[2].offset = (unsigned int)&Ram_Word_1M[0x20000] - 0x200000;
			
			S68K_Fetch[1].lowaddr = 0x0C0000;	// Bank 0
			S68K_Fetch[1].highaddr = 0x0DFFFF;
			S68K_Fetch[1].offset = (unsigned int) &Ram_Word_1M[0] - 0x0C0000;
			break;
	}
}


/**
 * M68K_Reset_CPU(): Reset the Main 68000 CPU.
 */
void M68K_Reset_CPU(void)
{
	main68k_reset();
}


/**
 * S68K_Reset_CPU(): Reset the Sub 68000 CPU.
 */
void S68K_Reset_CPU(void)
{
	sub68k_reset();
}


// Symbol aliases for cross-OS asm compatibility.
void _M68K_Set_32X_Rom_Bank(void)
	__attribute__ ((weak, alias ("M68K_Set_32X_Rom_Bank")));
void _M68K_Set_Prg_Ram(void)
	__attribute__ ((weak, alias ("M68K_Set_Prg_Ram")));
void _M68K_32X_Mode(void)
	__attribute__ ((weak, alias ("M68K_32X_Mode")));
void _MS68K_Set_Word_Ram(void)
	__attribute__ ((weak, alias ("MS68K_Set_Word_Ram")));
