#include <string.h>
#include "sh2.h"
#include "cpu_sh2.h"

#include "gens_core/mem/mem_m68k.h"
#include "gens_core/mem/mem_m68k_32x.h"
#include "gens_core/mem/mem_sh2.h"
#include "gens_core/vdp/vdp_32x.h"

int MSH2_Speed;
int SSH2_Speed;


/**
 * MSH2_Init(): Initialize the Master SH2 CPU.
 * @return 0 on success.
 */
int MSH2_Init(void)
{
	SH2_Init (&M_SH2, 0);
	
	SH2_Set_Fetch_Reg(&M_SH2, 0, 0x06000000, 0x0603FFFF,
			  (UINT16*)&_32X_Ram[0]);
	SH2_Set_Fetch_Reg(&M_SH2, 1, 0x26000000, 0x2603FFFF,
			  (UINT16*)&_32X_Ram[0]);
	SH2_Set_Fetch_Reg(&M_SH2, 2, 0x02000000, 0x023FFFFF,
			  (UINT16*)&_32X_Rom[0]);
	SH2_Set_Fetch_Reg(&M_SH2, 3, 0x22000000, 0x223FFFFF,
			  (UINT16*)&_32X_Rom[0]);
	SH2_Set_Fetch_Reg(&M_SH2, 4, 0x00000000, 0x000003FF,
			  (UINT16*)&_32X_MSH2_Rom[0]);
	SH2_Set_Fetch_Reg(&M_SH2, 5, 0x20000000, 0x200003FF,
			  (UINT16*)&_32X_MSH2_Rom[0]);
	SH2_Set_Fetch_Reg(&M_SH2, 6, 0xC0000000, 0xC0000FFF,
			  (UINT16 *)&(M_SH2.Cache[0]));
	SH2_Set_Fetch_Reg(&M_SH2, 7, 0x00000000, 0x00000000, (UINT16*)(-1));
	
	SH2_Add_ReadB (&M_SH2, 0x00, 0x00, MSH2_Read_Byte_00);
	SH2_Add_ReadW (&M_SH2, 0x00, 0x00, MSH2_Read_Word_00);
	SH2_Add_ReadL (&M_SH2, 0x00, 0x00, MSH2_Read_Long_00);
	
	SH2_Add_ReadB (&M_SH2, 0x02, 0x02, SH2_Read_Byte_Rom);
	SH2_Add_ReadW (&M_SH2, 0x02, 0x02, SH2_Read_Word_Rom);
	SH2_Add_ReadL (&M_SH2, 0x02, 0x02, SH2_Read_Long_Rom);
	
	SH2_Add_ReadB (&M_SH2, 0x04, 0x04, SH2_Read_Byte_FB1);
	SH2_Add_ReadW (&M_SH2, 0x04, 0x04, SH2_Read_Word_FB1);
	SH2_Add_ReadL (&M_SH2, 0x04, 0x04, SH2_Read_Long_FB1);
	
	/*
	SH2_Add_ReadB(&M_SH2, 0x05, 0x05, SH2_Read_Byte_FB1);
	SH2_Add_ReadW(&M_SH2, 0x05, 0x05, SH2_Read_Word_FB1);
	SH2_Add_ReadL(&M_SH2, 0x05, 0x05, SH2_Read_Long_FB1);
	*/
	
	SH2_Add_ReadB(&M_SH2, 0x06, 0x06, SH2_Read_Byte_Ram);
	SH2_Add_ReadW(&M_SH2, 0x06, 0x06, SH2_Read_Word_Ram);
	SH2_Add_ReadL(&M_SH2, 0x06, 0x06, SH2_Read_Long_Ram);
	
	SH2_Add_WriteB(&M_SH2, 0x00, 0x00, MSH2_Write_Byte_00);
	SH2_Add_WriteW(&M_SH2, 0x00, 0x00, MSH2_Write_Word_00);
	SH2_Add_WriteL(&M_SH2, 0x00, 0x00, MSH2_Write_Long_00);
	
	SH2_Add_WriteB(&M_SH2, 0x04, 0x04, SH2_Write_Byte_FB1);
	SH2_Add_WriteW(&M_SH2, 0x04, 0x04, SH2_Write_Word_FB1);
	SH2_Add_WriteL(&M_SH2, 0x04, 0x04, SH2_Write_Long_FB1);
	
	/*
	SH2_Add_WriteB(&M_SH2, 0x05, 0x05, SH2_Write_Byte_FB1);
	SH2_Add_WriteW(&M_SH2, 0x05, 0x05, SH2_Write_Word_FB1);
	SH2_Add_WriteL(&M_SH2, 0x05, 0x05, SH2_Write_Long_FB1);
	*/
	
	SH2_Add_WriteB(&M_SH2, 0x06, 0x06, SH2_Write_Byte_Ram);
	SH2_Add_WriteW(&M_SH2, 0x06, 0x06, SH2_Write_Word_Ram);
	SH2_Add_WriteL(&M_SH2, 0x06, 0x06, SH2_Write_Long_Ram);
	
	SH2_Map_Cache_Trough(&M_SH2);
	
	SH2_Reset(&M_SH2, 0);
	return 0;
}


/**
 * SSH2_Init(): Initialize the Slave SH2 CPU.
 * @return 0 on success.
 */
int SSH2_Init(void)
{
	SH2_Init(&S_SH2, 1);
	
	SH2_Set_Fetch_Reg(&S_SH2, 0, 0x06000000, 0x0603FFFF,
			  (UINT16*)&_32X_Ram[0]);
	SH2_Set_Fetch_Reg(&S_SH2, 1, 0x26000000, 0x2603FFFF,
			  (UINT16*)&_32X_Ram[0]);
	SH2_Set_Fetch_Reg(&S_SH2, 2, 0x02000000, 0x023FFFFF,
			  (UINT16*)&_32X_Rom[0]);
	SH2_Set_Fetch_Reg(&S_SH2, 3, 0x22000000, 0x223FFFFF,
			  (UINT16*)&_32X_Rom[0]);
	SH2_Set_Fetch_Reg(&S_SH2, 4, 0x00000000, 0x000003FF,
			  (UINT16*)&_32X_SSH2_Rom[0]);
	SH2_Set_Fetch_Reg(&S_SH2, 5, 0x20000000, 0x200003FF,
			  (UINT16*)&_32X_SSH2_Rom[0]);
	SH2_Set_Fetch_Reg(&S_SH2, 6, 0xC0000000, 0xC0000FFF,
			  (UINT16*)&(S_SH2.Cache[0]));
	SH2_Set_Fetch_Reg(&S_SH2, 7, 0x00000000, 0x00000000, (UINT16*)(-1));
	
	/*
	SH2_Add_Fetch(&S_SH2, 0x06000000, 0x0603FFFF, (UINT16*)&_32X_Ram[0]);
	SH2_Add_Fetch(&S_SH2, 0x26000000, 0x2603FFFF, (UINT16*)&_32X_Ram[0]);
	SH2_Add_Fetch(&S_SH2, 0x02000000, 0x003FFFFF, (UINT16*)&_32X_Rom[0]);
	SH2_Add_Fetch(&S_SH2, 0x22000000, 0x203FFFFF, (UINT16*)&_32X_Rom[0]);
	SH2_Add_Fetch(&S_SH2, 0x00000000, 0x000003FF, (UINT16*)&_32X_SSH2_Rom[0]);
	SH2_Add_Fetch(&S_SH2, 0x20000000, 0x200003FF, (UINT16*)&_32X_SSH2_Rom[0]);
	SH2_Add_Fetch(&S_SH2, 0x00000000, 0x00000000, (UINT16*)(-1));
	*/
	
	SH2_Add_ReadB (&S_SH2, 0x00, 0x00, SSH2_Read_Byte_00);
	SH2_Add_ReadW (&S_SH2, 0x00, 0x00, SSH2_Read_Word_00);
	SH2_Add_ReadL (&S_SH2, 0x00, 0x00, SSH2_Read_Long_00);
	
	SH2_Add_ReadB (&S_SH2, 0x02, 0x02, SH2_Read_Byte_Rom);
	SH2_Add_ReadW (&S_SH2, 0x02, 0x02, SH2_Read_Word_Rom);
	SH2_Add_ReadL (&S_SH2, 0x02, 0x02, SH2_Read_Long_Rom);
	
	SH2_Add_ReadB (&S_SH2, 0x04, 0x04, SH2_Read_Byte_FB1);
	SH2_Add_ReadW (&S_SH2, 0x04, 0x04, SH2_Read_Word_FB1);
	SH2_Add_ReadL (&S_SH2, 0x04, 0x04, SH2_Read_Long_FB1);
	
	/*
	SH2_Add_ReadB(&S_SH2, 0x05, 0x05, SH2_Read_Byte_FB1);
	SH2_Add_ReadW(&S_SH2, 0x05, 0x05, SH2_Read_Word_FB1);
	SH2_Add_ReadL(&S_SH2, 0x05, 0x05, SH2_Read_Long_FB1);
	*/
	
	SH2_Add_ReadB (&S_SH2, 0x06, 0x06, SH2_Read_Byte_Ram);
	SH2_Add_ReadW (&S_SH2, 0x06, 0x06, SH2_Read_Word_Ram);
	SH2_Add_ReadL (&S_SH2, 0x06, 0x06, SH2_Read_Long_Ram);
	
	SH2_Add_WriteB (&S_SH2, 0x00, 0x00, SSH2_Write_Byte_00);
	SH2_Add_WriteW (&S_SH2, 0x00, 0x00, SSH2_Write_Word_00);
	SH2_Add_WriteL (&S_SH2, 0x00, 0x00, SSH2_Write_Long_00);
	
	SH2_Add_WriteB (&S_SH2, 0x04, 0x04, SH2_Write_Byte_FB1);
	SH2_Add_WriteW (&S_SH2, 0x04, 0x04, SH2_Write_Word_FB1);
	SH2_Add_WriteL (&S_SH2, 0x04, 0x04, SH2_Write_Long_FB1);
	
	/*
	SH2_Add_WriteB(&S_SH2, 0x05, 0x05, SH2_Write_Byte_FB1);
	SH2_Add_WriteW(&S_SH2, 0x05, 0x05, SH2_Write_Word_FB1);
	SH2_Add_WriteL(&S_SH2, 0x05, 0x05, SH2_Write_Long_FB1);
	*/
	
	SH2_Add_WriteB (&S_SH2, 0x06, 0x06, SH2_Write_Byte_Ram);
	SH2_Add_WriteW (&S_SH2, 0x06, 0x06, SH2_Write_Word_Ram);
	SH2_Add_WriteL (&S_SH2, 0x06, 0x06, SH2_Write_Long_Ram);
	
	SH2_Map_Cache_Trough(&S_SH2);
	
	SH2_Reset(&S_SH2, 0);
	return 0;
}


/**
 * MSH2_Reset(): Reset the Master SH2 (and stuff around).
 */
void MSH2_Reset(void)
{
	SH2_Reset(&M_SH2, 0);
	
	memset(_32X_Comm, 0, 0x10);
	memset(_32X_FIFO_A, 0, 4 * 2);
	memset(_32X_FIFO_B, 0, 4 * 2);
	_32X_ADEN = 0;
	_32X_FM = 0;
	_32X_RV = 0;
	_32X_DREQ_ST = 0;
	_32X_DREQ_SRC = 0;
	_32X_DREQ_DST = 0;
	_32X_DREQ_LEN = 0;
	_32X_FIFO_Block = 0;
	_32X_FIFO_Read = 0;
	_32X_FIFO_Write = 0;
	_32X_MINT = 0;
	_32X_HIC = 0;
}


/**
 * SSH2_Reset(): Reset the Slave SH2 (and stuff around).
 */
void SSH2_Reset(void)
{
	SH2_Reset(&S_SH2, 0);
	_32X_SINT = 0;
}


/**
 * MSH2_Reset_CPU(): Reset the Master SH2 CPU only.
 */
void MSH2_Reset_CPU(void)
{
	SH2_Reset(&M_SH2, 1);
}


/**
 * MSH2_Reset_CPU(): Reset the Slave SH2 CPU only.
 */
void SSH2_Reset_CPU(void)
{
	SH2_Reset(&S_SH2, 1);
}


/**
 * _32X_Set_FB(): Update Frame Buffer memory map.
 */
void _32X_Set_FB(void)
{
	if (_32X_VDP.State & 0x1)
	{
		// Frame Buffer 1 displayed
		if (_32X_FM)
		{
			SH2_Add_ReadB(&M_SH2, 0x04, 0x04, SH2_Read_Byte_FB0);
			SH2_Add_ReadW(&M_SH2, 0x04, 0x04, SH2_Read_Word_FB0);
			SH2_Add_ReadL(&M_SH2, 0x04, 0x04, SH2_Read_Long_FB0);
			SH2_Add_ReadB(&M_SH2, 0x24, 0x24, SH2_Read_Byte_FB0);
			SH2_Add_ReadW(&M_SH2, 0x24, 0x24, SH2_Read_Word_FB0);
			SH2_Add_ReadL(&M_SH2, 0x24, 0x24, SH2_Read_Long_FB0);
			
			SH2_Add_WriteB(&M_SH2, 0x04, 0x04, SH2_Write_Byte_FB0);
			SH2_Add_WriteW(&M_SH2, 0x04, 0x04, SH2_Write_Word_FB0);
			SH2_Add_WriteL(&M_SH2, 0x04, 0x04, SH2_Write_Long_FB0);
			SH2_Add_WriteB(&M_SH2, 0x24, 0x24, SH2_Write_Byte_FB0);
			SH2_Add_WriteW(&M_SH2, 0x24, 0x24, SH2_Write_Word_FB0);
			SH2_Add_WriteL(&M_SH2, 0x24, 0x24, SH2_Write_Long_FB0);
			
			SH2_Add_ReadB(&S_SH2, 0x04, 0x04, SH2_Read_Byte_FB0);
			SH2_Add_ReadW(&S_SH2, 0x04, 0x04, SH2_Read_Word_FB0);
			SH2_Add_ReadL(&S_SH2, 0x04, 0x04, SH2_Read_Long_FB0);
			SH2_Add_ReadB(&S_SH2, 0x24, 0x24, SH2_Read_Byte_FB0);
			SH2_Add_ReadW(&S_SH2, 0x24, 0x24, SH2_Read_Word_FB0);
			SH2_Add_ReadL(&S_SH2, 0x24, 0x24, SH2_Read_Long_FB0);
			
			SH2_Add_WriteB(&S_SH2, 0x04, 0x04, SH2_Write_Byte_FB0);
			SH2_Add_WriteW(&S_SH2, 0x04, 0x04, SH2_Write_Word_FB0);
			SH2_Add_WriteL(&S_SH2, 0x04, 0x04, SH2_Write_Long_FB0);
			SH2_Add_WriteB(&S_SH2, 0x24, 0x24, SH2_Write_Byte_FB0);
			SH2_Add_WriteW(&S_SH2, 0x24, 0x24, SH2_Write_Word_FB0);
			SH2_Add_WriteL(&S_SH2, 0x24, 0x24, SH2_Write_Long_FB0);
			
			M68K_Read_Byte_Table[8 * 2] = _32X_M68K_Read_Byte_Table[6 * 2];
			M68K_Read_Word_Table[8 * 2] = _32X_M68K_Read_Word_Table[6 * 2];
			M68K_Write_Byte_Table[8] = _32X_M68K_Write_Byte_Table[6];
			M68K_Write_Word_Table[8] = _32X_M68K_Write_Word_Table[6];
		}
		else
		{
			SH2_Add_ReadB(&M_SH2, 0x04, 0x04, Def_READB);
			SH2_Add_ReadW(&M_SH2, 0x04, 0x04, Def_READW);
			SH2_Add_ReadL(&M_SH2, 0x04, 0x04, Def_READL);
			SH2_Add_ReadB(&M_SH2, 0x24, 0x24, Def_READB);
			SH2_Add_ReadW(&M_SH2, 0x24, 0x24, Def_READW);
			SH2_Add_ReadL(&M_SH2, 0x24, 0x24, Def_READL);
			
			SH2_Add_WriteB(&M_SH2, 0x04, 0x04, Def_WRITEB);
			SH2_Add_WriteW(&M_SH2, 0x04, 0x04, Def_WRITEW);
			SH2_Add_WriteL(&M_SH2, 0x04, 0x04, Def_WRITEL);
			SH2_Add_WriteB(&M_SH2, 0x24, 0x24, Def_WRITEB);
			SH2_Add_WriteW(&M_SH2, 0x24, 0x24, Def_WRITEW);
			SH2_Add_WriteL(&M_SH2, 0x24, 0x24, Def_WRITEL);
			
			SH2_Add_ReadB(&S_SH2, 0x04, 0x04, Def_READB);
			SH2_Add_ReadW(&S_SH2, 0x04, 0x04, Def_READW);
			SH2_Add_ReadL(&S_SH2, 0x04, 0x04, Def_READL);
			SH2_Add_ReadB(&S_SH2, 0x24, 0x24, Def_READB);
			SH2_Add_ReadW(&S_SH2, 0x24, 0x24, Def_READW);
			SH2_Add_ReadL(&S_SH2, 0x24, 0x24, Def_READL);
			
			SH2_Add_WriteB(&S_SH2, 0x04, 0x04, Def_WRITEB);
			SH2_Add_WriteW(&S_SH2, 0x04, 0x04, Def_WRITEW);
			SH2_Add_WriteL(&S_SH2, 0x04, 0x04, Def_WRITEL);
			SH2_Add_WriteB(&S_SH2, 0x24, 0x24, Def_WRITEB);
			SH2_Add_WriteW(&S_SH2, 0x24, 0x24, Def_WRITEW);
			SH2_Add_WriteL(&S_SH2, 0x24, 0x24, Def_WRITEL);
			
			M68K_Read_Byte_Table[8 * 2] =
			_32X_M68K_Read_Byte_Table[(8 * 2) - 1];
			M68K_Read_Word_Table[8 * 2] =
			_32X_M68K_Read_Word_Table[(8 * 2) - 1];
			M68K_Write_Byte_Table[8] = _32X_M68K_Write_Byte_Table[8 - 1];
			M68K_Write_Word_Table[8] = _32X_M68K_Write_Word_Table[8 - 1];
		}
	}
	else
	{
		if (_32X_FM)
		{
			SH2_Add_ReadB(&M_SH2, 0x04, 0x04, SH2_Read_Byte_FB1);
			SH2_Add_ReadW(&M_SH2, 0x04, 0x04, SH2_Read_Word_FB1);
			SH2_Add_ReadL(&M_SH2, 0x04, 0x04, SH2_Read_Long_FB1);
			SH2_Add_ReadB(&M_SH2, 0x24, 0x24, SH2_Read_Byte_FB1);
			SH2_Add_ReadW(&M_SH2, 0x24, 0x24, SH2_Read_Word_FB1);
			SH2_Add_ReadL(&M_SH2, 0x24, 0x24, SH2_Read_Long_FB1);
			
			SH2_Add_WriteB(&M_SH2, 0x04, 0x04, SH2_Write_Byte_FB1);
			SH2_Add_WriteW(&M_SH2, 0x04, 0x04, SH2_Write_Word_FB1);
			SH2_Add_WriteL(&M_SH2, 0x04, 0x04, SH2_Write_Long_FB1);
			SH2_Add_WriteB(&M_SH2, 0x24, 0x24, SH2_Write_Byte_FB1);
			SH2_Add_WriteW(&M_SH2, 0x24, 0x24, SH2_Write_Word_FB1);
			SH2_Add_WriteL(&M_SH2, 0x24, 0x24, SH2_Write_Long_FB1);
			
			SH2_Add_ReadB(&S_SH2, 0x04, 0x04, SH2_Read_Byte_FB1);
			SH2_Add_ReadW(&S_SH2, 0x04, 0x04, SH2_Read_Word_FB1);
			SH2_Add_ReadL(&S_SH2, 0x04, 0x04, SH2_Read_Long_FB1);
			SH2_Add_ReadB(&S_SH2, 0x24, 0x24, SH2_Read_Byte_FB1);
			SH2_Add_ReadW(&S_SH2, 0x24, 0x24, SH2_Read_Word_FB1);
			SH2_Add_ReadL(&S_SH2, 0x24, 0x24, SH2_Read_Long_FB1);
			
			SH2_Add_WriteB(&S_SH2, 0x04, 0x04, SH2_Write_Byte_FB1);
			SH2_Add_WriteW(&S_SH2, 0x04, 0x04, SH2_Write_Word_FB1);
			SH2_Add_WriteL(&S_SH2, 0x04, 0x04, SH2_Write_Long_FB1);
			SH2_Add_WriteB(&S_SH2, 0x24, 0x24, SH2_Write_Byte_FB1);
			SH2_Add_WriteW(&S_SH2, 0x24, 0x24, SH2_Write_Word_FB1);
			SH2_Add_WriteL(&S_SH2, 0x24, 0x24, SH2_Write_Long_FB1);
			
			M68K_Read_Byte_Table[8 * 2] = _32X_M68K_Read_Byte_Table[6 * 2];
			M68K_Read_Word_Table[8 * 2] = _32X_M68K_Read_Word_Table[6 * 2];
			M68K_Write_Byte_Table[8] = _32X_M68K_Write_Byte_Table[6];
			M68K_Write_Word_Table[8] = _32X_M68K_Write_Word_Table[6];
		}
		else
		{
			SH2_Add_ReadB(&M_SH2, 0x04, 0x04, Def_READB);
			SH2_Add_ReadW(&M_SH2, 0x04, 0x04, Def_READW);
			SH2_Add_ReadL(&M_SH2, 0x04, 0x04, Def_READL);
			SH2_Add_ReadB(&M_SH2, 0x24, 0x24, Def_READB);
			SH2_Add_ReadW(&M_SH2, 0x24, 0x24, Def_READW);
			SH2_Add_ReadL(&M_SH2, 0x24, 0x24, Def_READL);
			
			SH2_Add_WriteB(&M_SH2, 0x04, 0x04, Def_WRITEB);
			SH2_Add_WriteW(&M_SH2, 0x04, 0x04, Def_WRITEW);
			SH2_Add_WriteL(&M_SH2, 0x04, 0x04, Def_WRITEL);
			SH2_Add_WriteB(&M_SH2, 0x24, 0x24, Def_WRITEB);
			SH2_Add_WriteW(&M_SH2, 0x24, 0x24, Def_WRITEW);
			SH2_Add_WriteL(&M_SH2, 0x24, 0x24, Def_WRITEL);
			
			SH2_Add_ReadB(&S_SH2, 0x04, 0x04, Def_READB);
			SH2_Add_ReadW(&S_SH2, 0x04, 0x04, Def_READW);
			SH2_Add_ReadL(&S_SH2, 0x04, 0x04, Def_READL);
			SH2_Add_ReadB(&S_SH2, 0x24, 0x24, Def_READB);
			SH2_Add_ReadW(&S_SH2, 0x24, 0x24, Def_READW);
			SH2_Add_ReadL(&S_SH2, 0x24, 0x24, Def_READL);
			
			SH2_Add_WriteB(&S_SH2, 0x04, 0x04, Def_WRITEB);
			SH2_Add_WriteW(&S_SH2, 0x04, 0x04, Def_WRITEW);
			SH2_Add_WriteL(&S_SH2, 0x04, 0x04, Def_WRITEL);
			SH2_Add_WriteB(&S_SH2, 0x24, 0x24, Def_WRITEB);
			SH2_Add_WriteW(&S_SH2, 0x24, 0x24, Def_WRITEW);
			SH2_Add_WriteL(&S_SH2, 0x24, 0x24, Def_WRITEL);
			
			M68K_Read_Byte_Table[8 * 2] = _32X_M68K_Read_Byte_Table[8 * 2];
			M68K_Read_Word_Table[8 * 2] = _32X_M68K_Read_Word_Table[8 * 2];
			M68K_Write_Byte_Table[8] = _32X_M68K_Write_Byte_Table[8];
			M68K_Write_Word_Table[8] = _32X_M68K_Write_Word_Table[8];
		}
	}
}


// Symbol aliases for cross-OS asm compatibility.
void __32X_Set_FB(void)
	__attribute__ ((weak, alias ("_32X_Set_FB")));
