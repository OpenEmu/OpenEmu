#include <stdio.h>
#include <string.h>

#include "emulator/g_main.hpp"
#include "emulator/g_md.hpp"
#include "emulator/g_32x.hpp"
#include "emulator/g_input.hpp"
#include "gens_core/misc/misc.h"
#include "ui/gens_ui.hpp"

#include "debugger.hpp"

// MC68000
#include "gens_core/cpu/68k/cpu_68k.h"
#include "gens_core/cpu/68k/star_68k.h"
#include "gens_core/mem/mem_m68k.h"
#include "gens_core/mem/mem_s68k.h"
#include "m68kd.h"

// Z80
#include "gens_core/cpu/z80/cpu_z80.h"
#include "gens_core/cpu/z80/z80.h"
#include "gens_core/mem/mem_z80.h"
#include "z80dis.h"

// SH2
#include "gens_core/cpu/sh2/cpu_sh2.h"
#include "gens_core/cpu/sh2/sh2.h"
#include "gens_core/mem/mem_sh2.h"
#include "sh2d.h"

// VDP
#include "gens_core/vdp/vdp_io.h"
#include "gens_core/vdp/vdp_rend.h"
#include "gens_core/vdp/vdp_32x.h"

// SegaCD
#include "segacd/lc89510.h"
#include "segacd/cd_sys.hpp"
#include "gens_core/gfx/gfx_cd.h"

static int Current_32X_FB = 0;
static int adr_mem = 0, pattern_adr = 0, cd_pattern_adr = 0, pattern_pal = 0;
static int Current_PC;

static char Dbg_Out_Str[GENS_PATH_MAX];

// Macro used to print a constant string.
// Print_Text() doesn't directly support string constants, and since it's
// written in assembly language, it's too cumbersome to fix.
#define Print_Text_Constant(text, size, x, y, color)		\
{								\
	strcpy(Dbg_Out_Str, (text));				\
	Print_Text(Dbg_Out_Str, (size), (x), (y), (color));	\
}


/**
 * Debug_Event(): Key pressed while debugging.
 * @param key Keycode.
 * @param mod Modifiers.
 */
void Debug_Event(int key, int mod)
{
	int i, steps;
	SH2_CONTEXT *sh;
	// Determine the appropriate SH2 context.
	if (Debug == DEBUG_MAIN_SH2 || Debug == DEBUG_32X_VDP)
		sh = &M_SH2;
	else if (Debug == DEBUG_SUB_SH2)
		sh = &S_SH2;
	else
		sh = NULL;
	
	switch (key)
	{
		case GENS_KEY_t:
		case GENS_KEY_y:
		case GENS_KEY_u:
		case GENS_KEY_i:
		case GENS_KEY_o:
		case GENS_KEY_p:
			// Trip Odometer (run code?)
			// T = 1; Y = 10; U = 100; I = 1,000; O = 10,000; P = 100,000
			steps = 0;
			switch (key)
			{
				case GENS_KEY_t:
					steps = 1;
					break;
				case GENS_KEY_y:
					steps = 10;
					break;
				case GENS_KEY_u:
					steps = 100;
					break;
				case GENS_KEY_i:
					steps = 1000;
					break;
				case GENS_KEY_o:
					steps = 10000;
					break;
				case GENS_KEY_p:
					steps = 100000;
					break;
			}
			
			for (i = 0; i < steps; i++)
			{
				if ((Debug == DEBUG_MAIN_68000) || (Debug == DEBUG_GENESIS_VDP))
				{
					main68k_tripOdometer();
					main68k_exec(1);
				}
				else if (Debug == DEBUG_Z80)
				{
					z80_Clear_Odo(&M_Z80);
					z80_Exec(&M_Z80, 1);
				}
				else if (Debug == DEBUG_SUB_68000_REG ||
					 Debug == DEBUG_SUB_68000_CDC ||
					 Debug == DEBUG_WORD_RAM_PATTERN)
				{
					sub68k_tripOdometer();
					sub68k_exec(1);
				}
				else if (Debug == DEBUG_MAIN_SH2 ||
					 Debug == DEBUG_SUB_SH2 ||
					 Debug == DEBUG_32X_VDP)
				{
					SH2_Clear_Odo(sh);
					SH2_Exec(sh, 1);
				}
			}
			
			break;
		
		case GENS_KEY_z:
			adr_mem -= 0xC * 0xC * 0xC;
			break;
		
		case GENS_KEY_s:
			adr_mem += 0xC * 0xC * 0xC;
			break;
		
		case GENS_KEY_e:
			adr_mem -= 0xC * 0xC;
			break;
		
		case GENS_KEY_d:
			adr_mem += 0xC * 0xC;
			break;
		
		case GENS_KEY_r:
			adr_mem -= 0xC;
			break;
		
		case GENS_KEY_f:
			adr_mem += 0xC;
			break;
		
		case GENS_KEY_h:
			if (Debug == DEBUG_MAIN_68000)
			{
				main68k_interrupt(4, -1);
			}
			else if (Debug == DEBUG_Z80)
			{
				z80_Interrupt(&M_Z80, 0xFF);
			}
			else if (Debug == DEBUG_SUB_68000_REG ||
				 Debug == DEBUG_SUB_68000_CDC ||
				 Debug == DEBUG_WORD_RAM_PATTERN)
			{
				sub68k_interrupt(5, -1);
			}
			else if (Debug == DEBUG_MAIN_SH2 ||
				 Debug == DEBUG_SUB_SH2 ||
				 Debug == DEBUG_32X_VDP)
			{
				SH2_Interrupt(sh, 8);
			}
			
			break;
		
		case GENS_KEY_j:
			if (Debug == DEBUG_MAIN_68000)
			{
				main68k_interrupt(6, -1);
			}
			else if (Debug == DEBUG_Z80)
			{
				z80_Interrupt(&M_Z80, 0xFF);
			}
			else if (Debug == DEBUG_SUB_68000_REG ||
				 Debug == DEBUG_SUB_68000_CDC ||
				 Debug == DEBUG_WORD_RAM_PATTERN)
			{
				sub68k_interrupt(4, -1);
			}
			else if (Debug == DEBUG_MAIN_SH2 ||
				 Debug == DEBUG_SUB_SH2 ||
				 Debug == DEBUG_32X_VDP)
			{
				SH2_Interrupt(sh, 12);
			}
			
			break;
		
		case GENS_KEY_l:
			// Go up one VDP line.
			if (VDP_Current_Line > 0)
				VDP_Current_Line--;
			break;
		
		case GENS_KEY_m:
			// Go down one VDP line.
			if (VDP_Current_Line < 319)
				VDP_Current_Line++;
			break;
		
		case GENS_KEY_x:
			Debug ^= 0x0100;
			break;
		
		case GENS_KEY_c:
			if (Debug == DEBUG_SUB_68000_REG || Debug == DEBUG_SUB_68000_CDC)
				SCD.Cur_LBA++;
			VDP_Status ^= 0x8;
			break;
		
		case GENS_KEY_v:
			VDP_Status ^= 0x4;
			Current_32X_FB ^= 1;
			break;
		
		case GENS_KEY_n:
			if (Debug == DEBUG_MAIN_SH2 || Debug == DEBUG_SUB_SH2)
			{
				sh->PC += 2;
				sh->Status &= 0xFFFFFFF0;
			}
			else if (Debug == DEBUG_SUB_68000_REG || Debug == DEBUG_SUB_68000_CDC)
			{
				sub68k_context.pc += 2;
			}
			else if (Debug == DEBUG_MAIN_68000)
			{
				main68k_context.pc += 2;
			}
			else if (Debug == DEBUG_Z80)
			{
				z80_Set_PC(&M_Z80, z80_Get_PC(&M_Z80) + 1);
			}
			
			break;
		
		case GENS_KEY_w:
			if (Debug == DEBUG_SUB_68000_REG || Debug == DEBUG_SUB_68000_CDC)
				Check_CD_Command();
			
			break;
		
		case GENS_KEY_SPACE:
			if (Debug)
			{
				Debug++;
				
				if (SegaCD_Started)
				{
					if (Debug > 6)
						Debug = 1;
				}
				else if (_32X_Started)
				{
					if ((Debug > 3) && (Debug < 7))
						Debug = 7;
					if (Debug > 9)
						Debug = 1;
				}
				else if (Debug > 3)
					Debug = 1;
			}
			
			break;
		
		case GENS_KEY_NUM_DIVIDE:
			// What does this do?
			VDP_Status &= ~2;
			for (i = 0; i < 16; i++)
			{
				// NOTE: Gens Rerecording uses [12 * 16 * i].
				// Original Gens uses [7 * 16 * i].
				// Which is it?!
				if (bppMD == 15)
				{
					MD_Palette[7 * 16 + i] =
						((2 * i) << 10) + ((2 * i) << 5) + (2 * i);
				}
				else if (bppMD == 16)
				{
					MD_Palette[7 * 16 + i] =
						((2 * i) << 11) + ((4 * i) << 5) + (2 * i);
				}
				else //if (bppMD == 32)
				{
					MD_Palette32[7 * 16 * i] =
						((16 * i) << 16) + ((16 * i) << 8) + (16 * i);
				}
			}
			
			break;
		
		case GENS_KEY_NUM_MULTIPLY:
			if (mod & GENS_KMOD_SHIFT)
				pattern_pal--;
			else
				pattern_pal++;
			
			pattern_pal &= 0xF;
			break;
		
		case GENS_KEY_NUM_PLUS:
			// Scroll down in pattern debugging.
			if (Debug == DEBUG_MAIN_68000 ||
			    Debug == DEBUG_Z80 ||
			    Debug == DEBUG_GENESIS_VDP)
			{
				if (pattern_adr < 0xDA00)
					pattern_adr = (pattern_adr + 0x200) & 0xFFFF;
				if (pattern_adr >= 0xDA00) // Make sure it doesn't go out of bounds.
					pattern_adr = 0xDA00 - 0x200;
			}
			else if (Debug == DEBUG_WORD_RAM_PATTERN)
			{
				if (cd_pattern_adr < 0x3D000)
					cd_pattern_adr = (pattern_adr + 0x800) & 0x3FFFF;
				if (pattern_adr >= 0x3D000) // Make sure it doesn't go out of bounds.
					pattern_adr = 0x3D000 - 0x800;
			}
			
			break;
		
		case GENS_KEY_NUM_MINUS:
			// Scroll up in pattern debugging.
			if (Debug == DEBUG_MAIN_68000 ||
			    Debug == DEBUG_Z80 ||
			    Debug == DEBUG_GENESIS_VDP)
			{
				if (pattern_adr > 0)
					pattern_adr = (pattern_adr - 0x200) & 0xFFFF;
				if (pattern_adr < 0) // Make sure it doesn't go out of bounds.
					pattern_adr = 0;
			}
			else if (Debug == DEBUG_WORD_RAM_PATTERN)
			{
				if (cd_pattern_adr > 0)
					cd_pattern_adr = cd_pattern_adr - 0x800;
				if (cd_pattern_adr < 0)	// Make sure it doesn't go out of bounds.
					cd_pattern_adr = 0;
			}
			
			break;
	}
}


/**
 * Next_Short(): Get the next short (16-bit) value.
 * @return Next short (16-bit) value.
 */
unsigned short Next_Word(void)
{
	unsigned short val = 0;
	
	if (Debug == DEBUG_MAIN_68000)
		val = M68K_RW(Current_PC);
	else if (Debug >= DEBUG_Z80)
		val = S68K_RW(Current_PC);
	
	Current_PC += 2;
	return val;
}


/**
 * Next_Long(): Get the next long (32-bit) value.
 * @return Next long (32-bit) value.
 */
unsigned int Next_Long(void)
{
	unsigned int val = 0;
	
	if (Debug == DEBUG_MAIN_68000)
	{
		val = M68K_RW(Current_PC);
		val <<= 16;
		val |= M68K_RW(Current_PC + 2);
	}
	else if (Debug >= DEBUG_Z80)
	{
		val = S68K_RW(Current_PC);
		val <<= 16;
		val |= S68K_RW(Current_PC + 2);
	}
	
	Current_PC += 4;
	return val;
}


/**
 * Refresh_M68k_Inst(): Refresh the Main 68000 instance.
 */
static void Refresh_M68k_Inst(void)
{
	Current_PC = main68k_context.pc;
	Print_Text_Constant("** MAIN 68000 DEBUG **", 22, 24, 1, VERT);
	
	for (unsigned int i = 1; i < 14; i++)
	{
		unsigned int PC = Current_PC;
		sprintf(Dbg_Out_Str, "%.4X   %-33s\n", PC, M68KDisasm(Next_Word, Next_Long));
		Print_Text(Dbg_Out_Str, 39, 1, (i << 3) + 5, (i == 1 ? ROUGE : BLANC));
	}
}


/**
 * Refresh_M68k_Inst(): Refresh the Sub 68000 instance.
 */
static void Refresh_S68k_Inst(void)
{
	Current_PC = sub68k_context.pc;
	Print_Text_Constant("** SUB 68000 DEBUG **", 22, 24, 1, VERT);
	
	for (unsigned int i = 1; i < 14; i++)
	{
		unsigned int PC = Current_PC;
		sprintf(Dbg_Out_Str, "%.4X   %-33s\n", PC, M68KDisasm(Next_Word, Next_Long));
		Print_Text(Dbg_Out_Str, 39, 1, (i << 3) + 5, (i == 1 ? ROUGE : BLANC));
	}
}


/**
 * Refresh_Z80_Inst(): Refresh the Z80 instance.
 */
static void Refresh_Z80_Inst(void)
{
	unsigned int PC = z80_Get_PC(&M_Z80);
	Print_Text_Constant("***** Z80 DEBUG *****", 22, 24, 1, VERT);
	
	for (unsigned int i = 1; i < 14; i++)
	{
		z80dis((unsigned char *) Ram_Z80, (int *) &PC, Dbg_Out_Str);
		Print_Text(Dbg_Out_Str, 39, 1, (i << 3) + 5, (i == 1 ? ROUGE : BLANC));
	}
}


/**
 * Refresh_SH2_Inst(): Refresh an SH2 instance.
 * @param num ID of SH2 CPU (0 or 1).
 */
static void Refresh_SH2_Inst(int num)
{
	SH2_CONTEXT *sh;
	
	if (num)
	{
		strcpy(Dbg_Out_Str, "** SLAVE SH2 DEBUG **");
		sh = &S_SH2;
	}
	else
	{
		strcpy(Dbg_Out_Str, "** MASTER SH2 DEBUG **");
		sh = &M_SH2;
	}
	Print_Text(Dbg_Out_Str, 22, 24, 1, VERT);
	
	unsigned int PC = (sh->PC - sh->Base_PC) - 4;
	
	for (unsigned int i = 1; i < 14; i++, PC += 2)
	{
		SH2Disasm(Dbg_Out_Str, PC, SH2_Read_Word (sh, PC), 0);
		Print_Text(Dbg_Out_Str, 39, 1, (i << 3) + 5, (i == 1 ? ROUGE : BLANC));
	}
}


/**
 * Refresh_M68k_Mem(): Refresh the Main 68000 memory dump.
 */
static void Refresh_M68k_Mem(void)
{
	unsigned int Adr = adr_mem >> 1;
	Print_Text_Constant("** MAIN 68000 MEM **", 20, 24, 130, VERT);
	
	for (unsigned int k = 0, j = Adr; k < 7; k++, j += 6)
	{
		unsigned int i = (j & 0x7FFF) << 1;
		sprintf(Dbg_Out_Str, "%.4X:%.4X %.4X %.4X %.4X %.4X %.4X\n", i,
				Ram_68k[i] + (Ram_68k[i + 1] << 8),
				Ram_68k[i + 2] + (Ram_68k[i + 3] << 8),
				Ram_68k[i + 4] + (Ram_68k[i + 5] << 8),
				Ram_68k[i + 6] + (Ram_68k[i + 7] << 8),
				Ram_68k[i + 8] + (Ram_68k[i + 9] << 8),
				Ram_68k[i + 10] + (Ram_68k[i + 11] << 8));
		Print_Text(Dbg_Out_Str, 34, 1, 146 + (k << 3), BLANC);
    }
}


/**
 * Refresh_M68k_Mem(): Refresh the Sub 68000 memory dump.
 */
static void Refresh_S68k_Mem(void)
{
	unsigned int Adr = adr_mem >> 1;
	Print_Text_Constant("** SUB 68000 MEM **", 19, 24, 130, VERT);
	
	for (unsigned int k = 0, j = Adr; k < 7; k++, j += 6)
	{
		unsigned int i = (j & 0x1FFFF) << 1;
		sprintf(Dbg_Out_Str, "%.5X:%.4X %.4X %.4X %.4X %.4X %.4X\n", i,
				Ram_Word_1M[i] + (Ram_Word_1M[i + 1] << 8),
				Ram_Word_1M[i + 2] + (Ram_Word_1M[i + 3] << 8),
				Ram_Word_1M[i + 4] + (Ram_Word_1M[i + 5] << 8),
				Ram_Word_1M[i + 6] + (Ram_Word_1M[i + 7] << 8),
				Ram_Word_1M[i + 8] + (Ram_Word_1M[i + 9] << 8),
				Ram_Word_1M[i + 10] + (Ram_Word_1M[i + 11] << 8));
		Print_Text(Dbg_Out_Str, 35, 1, 146 + (k << 3), BLANC);
	}
}


/**
 * Refresh_Z80_Mem(): Refresh the Z80 memory dump.
  */
static void Refresh_Z80_Mem(void)
{
	Print_Text_Constant("***** Z80 MEM *****", 19, 24, 130, VERT);
	
	for (unsigned int k = 0, j = adr_mem & 0xFFFF; k < 7; k++, j = (j + 12) & 0xFFFF)
	{
		sprintf(Dbg_Out_Str, "%.4X:%.2X %.2X %.2X %.2X %.2X %.2X %.2X %.2X %.2X %.2X %.2X %.2X\n", j,
				Z80_ReadB(j + 0), Z80_ReadB(j + 1), Z80_ReadB(j + 2),
				Z80_ReadB(j + 3), Z80_ReadB(j + 4), Z80_ReadB(j + 5),
				Z80_ReadB(j + 6), Z80_ReadB(j + 7), Z80_ReadB(j + 8),
				Z80_ReadB(j + 9), Z80_ReadB(j + 10), Z80_ReadB(j + 11));
		Print_Text(Dbg_Out_Str, 35, 1, 146 + (k << 3), BLANC);
	}
}


/**
 * Refresh_SH2_Mem(): Refresh the SH2 memory dump.
 */
static void Refresh_SH2_Mem(void)
{
	unsigned int i, j, k, Adr;
	Adr = adr_mem >> 1;
	Print_Text_Constant("** SH2 CPU MEM **", 19, 24, 130, VERT);
	
	for (k = 0, j = Adr; k < 7; k++, j += 6)
	{
		i = (j & 0x1FFFF) << 1;
		sprintf(Dbg_Out_Str, "%.5X:%.4X %.4X %.4X %.4X %.4X %.4X\n", i,
				_32X_Ram[i] + (_32X_Ram[i + 1] << 8),
				_32X_Ram[i + 2] + (_32X_Ram[i + 3] << 8),
				_32X_Ram[i + 4] + (_32X_Ram[i + 5] << 8),
				_32X_Ram[i + 6] + (_32X_Ram[i + 7] << 8),
				_32X_Ram[i + 8] + (_32X_Ram[i + 9] << 8),
				_32X_Ram[i + 10] + (_32X_Ram[i + 11] << 8));
		Print_Text(Dbg_Out_Str, 35, 1, 146 + (k << 3), BLANC);
	}
}


/**
 * Refresh_M68k_State(): Refresh the Main 68000 status display.
 */
static void Refresh_M68k_State(void)
{
	Print_Text_Constant("** MAIN 68000 STATUS **", 23, 196, 130, VERT);
	
	sprintf(Dbg_Out_Str, "A0=%.8X A1=%.8X A2=%.8X X=%d\n",
			main68k_context.areg[0], main68k_context.areg[1],
			main68k_context.areg[2], (main68k_context.sr & 0x10) ? 1 : 0);
	Print_Text(Dbg_Out_Str, strlen(Dbg_Out_Str) - 1, 162, 146, BLANC);
	sprintf(Dbg_Out_Str, "A3=%.8X A4=%.8X A5=%.8X N=%d\n",
			main68k_context.areg[3], main68k_context.areg[4],
			main68k_context.areg[5], (main68k_context.sr & 0x8) ? 1 : 0);
	Print_Text(Dbg_Out_Str, strlen(Dbg_Out_Str) - 1, 162, 154, BLANC);
	sprintf(Dbg_Out_Str, "A6=%.8X A7=%.8X D0=%.8X Z=%d\n",
			main68k_context.areg[6], main68k_context.areg[7],
			main68k_context.dreg[0], (main68k_context.sr & 0x4) ? 1 : 0);
	Print_Text(Dbg_Out_Str, strlen(Dbg_Out_Str) - 1, 162, 162, BLANC);
	sprintf(Dbg_Out_Str, "D1=%.8X D2=%.8X D3=%.8X V=%d\n",
			main68k_context.dreg[1], main68k_context.dreg[2],
			main68k_context.dreg[3], (main68k_context.sr & 0x2) ? 1 : 0);
	Print_Text(Dbg_Out_Str, strlen(Dbg_Out_Str) - 1, 162, 170, BLANC);
	sprintf(Dbg_Out_Str, "D4=%.8X D5=%.8X D6=%.8X C=%d\n",
			main68k_context.dreg[4], main68k_context.dreg[5],
			main68k_context.dreg[6], (main68k_context.sr & 0x1) ? 1 : 0);
	Print_Text(Dbg_Out_Str, strlen(Dbg_Out_Str) - 1, 162, 178, BLANC);
	sprintf(Dbg_Out_Str, "D7=%.8X PC=%.8X SR=%.4X\n", main68k_context.dreg[7],
			main68k_context.pc, main68k_context.sr);
	Print_Text(Dbg_Out_Str, strlen(Dbg_Out_Str) - 1, 162, 186, BLANC);
	sprintf(Dbg_Out_Str, "Cycles=%.10d \n", main68k_context.odometer);
	Print_Text(Dbg_Out_Str, strlen(Dbg_Out_Str) - 1, 162, 194, BLANC);
	sprintf(Dbg_Out_Str, "Bank for Z80 = %.8X\n", Bank_Z80);
	Print_Text(Dbg_Out_Str, strlen(Dbg_Out_Str) - 1, 162, 202, BLANC);
	/*
	sprintf(Dbg_Out_Str, "Bank = %.2X %.2X %.2X %.2X %.2X %.2X %.2X %.2X\n",
			Rom_Bank[0], Rom_Bank[1], Rom_Bank[2], Rom_Bank[3],
			Rom_Bank[4], Rom_Bank[5], Rom_Bank[6], Rom_Bank[7]);
	Print_Text(Dbg_Out_Str, strlen(Dbg_Out_Str) - 1, 162, 210, BLANC);
	*/
}


/**
 * Refresh_S68k_State(): Refresh the Sub 68000 status display.
 */
static void Refresh_S68k_State(void)
{
	Print_Text_Constant("** SUB 68000 STATUS **", 22, 196, 130, VERT);
	
	sprintf(Dbg_Out_Str, "A0=%.8X A1=%.8X A2=%.8X X=%d\n", sub68k_context.areg[0],
			sub68k_context.areg[1], sub68k_context.areg[2],
			(sub68k_context.sr & 0x10) ? 1 : 0);
	Print_Text(Dbg_Out_Str, strlen(Dbg_Out_Str) - 1, 162, 146, BLANC);
	sprintf(Dbg_Out_Str, "A3=%.8X A4=%.8X A5=%.8X N=%d\n", sub68k_context.areg[3],
			sub68k_context.areg[4], sub68k_context.areg[5],
			(sub68k_context.sr & 0x8) ? 1 : 0);
	Print_Text(Dbg_Out_Str, strlen(Dbg_Out_Str) - 1, 162, 154, BLANC);
	sprintf(Dbg_Out_Str, "A6=%.8X A7=%.8X D0=%.8X Z=%d\n", sub68k_context.areg[6],
			sub68k_context.areg[7], sub68k_context.dreg[0],
			(sub68k_context.sr & 0x4) ? 1 : 0);
	Print_Text(Dbg_Out_Str, strlen(Dbg_Out_Str) - 1, 162, 162, BLANC);
	sprintf(Dbg_Out_Str, "D1=%.8X D2=%.8X D3=%.8X V=%d\n", sub68k_context.dreg[1],
			sub68k_context.dreg[2], sub68k_context.dreg[3],
			(sub68k_context.sr & 0x2) ? 1 : 0);
	Print_Text(Dbg_Out_Str, strlen(Dbg_Out_Str) - 1, 162, 170, BLANC);
	sprintf(Dbg_Out_Str, "D4=%.8X D5=%.8X D6=%.8X C=%d\n", sub68k_context.dreg[4],
			sub68k_context.dreg[5], sub68k_context.dreg[6],
			(sub68k_context.sr & 0x1) ? 1 : 0);
	Print_Text(Dbg_Out_Str, strlen(Dbg_Out_Str) - 1, 162, 178, BLANC);
	sprintf(Dbg_Out_Str, "D7=%.8X PC=%.8X SR=%.4X\n", sub68k_context.dreg[7],
			sub68k_context.pc, sub68k_context.sr);
	Print_Text(Dbg_Out_Str, strlen(Dbg_Out_Str) - 1, 162, 186, BLANC);
	sprintf(Dbg_Out_Str, "Cycles=%.10d \n", sub68k_context.odometer);
	Print_Text(Dbg_Out_Str, strlen(Dbg_Out_Str) - 1, 162, 194, BLANC);
	sprintf(Dbg_Out_Str, "Bank for main 68K = %.8X\n", Bank_M68K);
	Print_Text(Dbg_Out_Str, strlen(Dbg_Out_Str) - 1, 162, 202, BLANC);
}


/**
 * Refresh_Z80_State(): Refresh the Z80 status display.
 */
static void Refresh_Z80_State(void)
{
	Print_Text_Constant("***** Z80 STATUS *****", 22, 196, 130, VERT);
	
	sprintf(Dbg_Out_Str, "AF =%.4X BC =%.4X DE =%.4X HL =%.4X\n",
			z80_Get_AF (&M_Z80), M_Z80.BC.w.BC,
			M_Z80.DE.w.DE, M_Z80.HL.w.HL);
	Print_Text(Dbg_Out_Str, strlen(Dbg_Out_Str) - 1, 162, 146, BLANC);
	sprintf(Dbg_Out_Str, "AF2=%.4X BC2=%.4X DE2=%.4X HL2=%.4X\n",
			z80_Get_AF2 (&M_Z80), M_Z80.BC2.w.BC2,
			M_Z80.DE2.w.DE2, M_Z80.HL2.w.HL2);
	Print_Text(Dbg_Out_Str, strlen(Dbg_Out_Str) - 1, 162, 154, BLANC);
	sprintf(Dbg_Out_Str, "IX =%.4X IY =%.4X SP =%.4X PC =%.4X\n",
			M_Z80.IX.w.IX, M_Z80.IY.w.IY,
			M_Z80.SP.w.SP, z80_Get_PC (&M_Z80));
	Print_Text(Dbg_Out_Str, strlen(Dbg_Out_Str) - 1, 162, 162, BLANC);
	sprintf(Dbg_Out_Str, "IFF1=%d IFF2=%d I=%.2X R=%.2X IM=%.2X\n",
			M_Z80.IFF.b.IFF1, M_Z80.IFF.b.IFF2, M_Z80.I,
			M_Z80.R.b.R1, M_Z80.IM);
	Print_Text(Dbg_Out_Str, strlen(Dbg_Out_Str) - 1, 162, 170, BLANC);
	sprintf(Dbg_Out_Str, "S=%d Z=%d Y=%d H=%d X=%d P=%d N=%d C=%d\n",
			(z80_Get_AF (&M_Z80) & 0x80) >> 7,
			(z80_Get_AF (&M_Z80) & 0x40) >> 6,
			(z80_Get_AF (&M_Z80) & 0x20) >> 5,
			(z80_Get_AF (&M_Z80) & 0x10) >> 4,
			(z80_Get_AF (&M_Z80) & 0x08) >> 3,
			(z80_Get_AF (&M_Z80) & 0x04) >> 2,
			(z80_Get_AF (&M_Z80) & 0x02) >> 1,
			(z80_Get_AF (&M_Z80) & 0x01) >> 0);
	Print_Text(Dbg_Out_Str, strlen(Dbg_Out_Str) - 1, 162, 178, BLANC);
	sprintf(Dbg_Out_Str, "Status=%.2X ILine=%.2X IVect=%.2X\n",
			M_Z80.Status & 0xFF, M_Z80.IntLine, M_Z80.IntVect);
	Print_Text(Dbg_Out_Str, strlen(Dbg_Out_Str) - 1, 162, 186, BLANC);
	sprintf(Dbg_Out_Str, "Bank68K=%.8X State=%.2X\n", Bank_M68K, Z80_State);
	Print_Text(Dbg_Out_Str, strlen(Dbg_Out_Str) - 1, 162, 194, BLANC);
}


/**
 * Refresh_Z80_State(): Refresh the Z80 status display.
 */
static void Refresh_SH2_State(int num)
{
	SH2_CONTEXT *sh;
	
	if (num)
	{
		strcpy(Dbg_Out_Str, "** SLAVE SH2 STATUS **");
		sh = &S_SH2;
	}
	else
	{
		strcpy(Dbg_Out_Str, "** MASTER SH2 STATUS **");
		sh = &M_SH2;
	}
	Print_Text(Dbg_Out_Str, 22, 196, 130, VERT);
	
	sprintf(Dbg_Out_Str, "R0=%.8X R1=%.8X R2=%.8X T=%d\n", SH2_Get_R(sh, 0),
			SH2_Get_R(sh, 1), SH2_Get_R(sh, 2), SH2_Get_SR(sh) & 1);
	Print_Text(Dbg_Out_Str, strlen(Dbg_Out_Str) - 1, 162, 146, BLANC);
	sprintf(Dbg_Out_Str, "R3=%.8X R4=%.8X R5=%.8X S=%d\n", SH2_Get_R(sh, 3),
			SH2_Get_R(sh, 4), SH2_Get_R(sh, 5), (SH2_Get_SR(sh) >> 1) & 1);
	Print_Text(Dbg_Out_Str, strlen(Dbg_Out_Str) - 1, 162, 154, BLANC);
	sprintf(Dbg_Out_Str, "R6=%.8X R7=%.8X R8=%.8X Q=%d\n", SH2_Get_R(sh, 6),
			SH2_Get_R(sh, 7), SH2_Get_R(sh, 8), (SH2_Get_SR(sh) >> 8) & 1);
	Print_Text(Dbg_Out_Str, strlen(Dbg_Out_Str) - 1, 162, 162, BLANC);
	sprintf(Dbg_Out_Str, "R9=%.8X RA=%.8X RB=%.8X M=%d\n", SH2_Get_R(sh, 9),
			SH2_Get_R(sh, 0xA), SH2_Get_R(sh, 0xB),
			(SH2_Get_SR(sh) >> 9) & 1);
	Print_Text(Dbg_Out_Str, strlen(Dbg_Out_Str) - 1, 162, 170, BLANC);
	sprintf(Dbg_Out_Str, "RC=%.8X RD=%.8X RE=%.8X I=%.1X\n", SH2_Get_R(sh, 0xC),
			SH2_Get_R(sh, 0xD), SH2_Get_R(sh, 0xE),
			(SH2_Get_SR(sh) >> 4) & 0xF);
	Print_Text(Dbg_Out_Str, strlen(Dbg_Out_Str) - 1, 162, 178, BLANC);
	sprintf(Dbg_Out_Str, "RF=%.8X PC=%.8X SR=%.4X St=%.4X\n", SH2_Get_R(sh, 0xF),
			SH2_Get_PC(sh), SH2_Get_SR(sh), sh->Status & 0xFFFF);
	Print_Text(Dbg_Out_Str, strlen(Dbg_Out_Str) - 1, 162, 186, BLANC);
	sprintf(Dbg_Out_Str, "GBR=%.8X VBR=%.8X PR=%.8X\n", SH2_Get_GBR(sh),
			SH2_Get_VBR(sh), SH2_Get_PR(sh));
	Print_Text(Dbg_Out_Str, strlen(Dbg_Out_Str) - 1, 162, 194, BLANC);
	sprintf(Dbg_Out_Str, "MACH=%.8X MACL=%.8X IL=%.2X IV=%.2X\n",
			SH2_Get_MACH(sh), SH2_Get_MACL(sh), sh->INT.Prio, sh->INT.Vect);
	Print_Text(Dbg_Out_Str, strlen(Dbg_Out_Str) - 1, 162, 202, BLANC);
}


/**
 * Refresh_VDP_State(): Refresh the VDP status display.
 */
static void Refresh_VDP_State(void)
{
	Print_Text_Constant("**** VDP STATUS ****", 20, 200, 1, VERT);
	
	sprintf(Dbg_Out_Str, "Setting register: 1=%.2X 2=%.2X 3=%.2X 4=%.2X",
			VDP_Reg.Set1, VDP_Reg.Set2, VDP_Reg.Set3, VDP_Reg.Set4);
	Print_Text(Dbg_Out_Str, strlen(Dbg_Out_Str), 162, 14, BLANC);
	sprintf(Dbg_Out_Str, "Pattern Adr: ScrA=%.2X ScrB=%.2X Win=%.2X",
			VDP_Reg.Pat_ScrA_Adr, VDP_Reg.Pat_ScrB_Adr, VDP_Reg.Pat_Win_Adr);
	Print_Text(Dbg_Out_Str, strlen(Dbg_Out_Str), 162, 22, BLANC);
	sprintf(Dbg_Out_Str, "Sprite Attribut Adr: Low=%.2X High=%.2X",
			VDP_Reg.Spr_Att_Adr, VDP_Reg.Reg6);
	Print_Text(Dbg_Out_Str, strlen(Dbg_Out_Str), 162, 30, BLANC);
	sprintf(Dbg_Out_Str, "H Scroll Adr: Low=%.2X High=%.2X",
			VDP_Reg.H_Scr_Adr, VDP_Reg.Reg14);
	Print_Text(Dbg_Out_Str, strlen(Dbg_Out_Str), 162, 38, BLANC);
	sprintf(Dbg_Out_Str, "H Interrupt=%.2X    Auto Inc=%.2X",
			VDP_Reg.H_Int, VDP_Reg.Auto_Inc);
	Print_Text(Dbg_Out_Str, strlen(Dbg_Out_Str), 162, 46, BLANC);
	sprintf(Dbg_Out_Str, "BG Color: Low=%.2X Med=%.2X High=%.2X",
			VDP_Reg.BG_Color, VDP_Reg.Reg8, VDP_Reg.Reg9);
	Print_Text(Dbg_Out_Str, strlen(Dbg_Out_Str), 162, 54, BLANC);
	sprintf(Dbg_Out_Str, "Scroll Size=%.2X    Window Pos: H=%.2X V=%.2X",
			VDP_Reg.Scr_Size, VDP_Reg.Win_H_Pos, VDP_Reg.Win_V_Pos);
	Print_Text(Dbg_Out_Str, strlen(Dbg_Out_Str), 162, 62, BLANC);
	sprintf(Dbg_Out_Str, "DMA Length: Low=%.2X High=%.2X",
		VDP_Reg.DMA_Length_L, VDP_Reg.DMA_Length_H);
	Print_Text(Dbg_Out_Str, strlen(Dbg_Out_Str), 162, 70, BLANC);
	sprintf(Dbg_Out_Str, "DMA Source Adr: Low=%.2X Med=%.2X High=%.2X",
			VDP_Reg.DMA_Src_Adr_L, VDP_Reg.DMA_Src_Adr_M,
			VDP_Reg.DMA_Src_Adr_H);
	Print_Text(Dbg_Out_Str, strlen(Dbg_Out_Str), 162, 78, BLANC);
	
	int tmp = Read_VDP_Status ();
	sprintf(Dbg_Out_Str, "V Int Happened %d  Sprite overflow %d", (tmp >> 7) & 1, (tmp >> 6) & 1);
	Print_Text(Dbg_Out_Str, strlen(Dbg_Out_Str), 162, 86, BLANC);
	sprintf(Dbg_Out_Str, "Collision Spr  %d  Odd Frame in IM %d", (tmp >> 5) & 1, (tmp >> 4) & 1);
	Print_Text(Dbg_Out_Str, strlen(Dbg_Out_Str), 162, 94, BLANC);
	sprintf(Dbg_Out_Str, "During V Blank %d  During H Blank  %d", (tmp >> 3) & 1, (tmp >> 2) & 1);
	Print_Text(Dbg_Out_Str, strlen(Dbg_Out_Str), 162, 102, BLANC);
	sprintf(Dbg_Out_Str, "DMA Busy %d  PAL Mode %d Line Num %d", (tmp >> 1) & 1, tmp & 1, VDP_Current_Line);
	Print_Text(Dbg_Out_Str, strlen(Dbg_Out_Str), 162, 110, BLANC);
	sprintf(Dbg_Out_Str, "VDP Int =%.2X DMA_Length=%.4X", VDP_Int, DMAT_Length);
	Print_Text(Dbg_Out_Str, strlen(Dbg_Out_Str), 162, 118, BLANC);
}


/**
 * Refresh_VDP_Pattern(): Refresh the VDP pattern display.
 */
static void Refresh_VDP_Pattern(void)
{
	Print_Text_Constant("******** VDP PATTERN ********", 29, 28, 0, VERT);
	
	for (unsigned int i = 0; i < 20; i++)
	{
		sprintf(Dbg_Out_Str, "%.4X", (pattern_adr & 0xFFFF) + 0x200 * i);
		Print_Text(Dbg_Out_Str, strlen(Dbg_Out_Str), 2, (i << 3) + 11, BLANC);
	}
	
	Cell_8x8_Dump(&VRam[pattern_adr & 0xFFFF], pattern_pal);
}


template<typename pixel>
static inline void Refresh_VDP_Palette_Colors(pixel *screen, pixel *palette, unsigned short numPalettes)
{
	for (unsigned int i = 0; i < 8; i++)
	{
		for (unsigned int j = 0; j < 16; j++)
		{
			for (unsigned int k = 0; k < 8; k++)
			{
				unsigned int col = (i * 336) + 180 + (j * 8) + k;
				for (unsigned int l = 0; l < numPalettes; l++)
				{
					screen[(336 * (10 + (l * 8))) + col] = palette[j + (16 * l)];
				}
			}
		}
	}
}

template<typename pixel>
static inline void Refresh_VDP_Palette_Outline(pixel *screen, unsigned short paletteMask, pixel outlineColor)
{
	// Outline the selected palette. Ported from Gens Rerecording.
	for (unsigned int i = 0; i < 16 * 8; i++)
	{
		screen[(336 * (9 + ((pattern_pal & paletteMask) * 8))) + 180 + i] = outlineColor;
		screen[(336 * (18 + ((pattern_pal & paletteMask) * 8))) + 180 + i] = outlineColor;
	}
}

/**
 * Refresh_VDP_Palette(): Refresh the VDP palette display.
 */
static void Refresh_VDP_Palette(void)
{
	Print_Text_Constant("******** VDP PALETTE ********", 29, 180, 0, ROUGE);
	
	if (bppMD == 32)
	{
		// 32-bit color palette update
		Refresh_VDP_Palette_Colors(MD_Screen32, MD_Palette32, 4);
		Refresh_VDP_Palette_Outline(MD_Screen32, 0x03, (unsigned int)0xFFFFFF);
	}
	else
	{
		// 15/16-bit color palette update
		Refresh_VDP_Palette_Colors(MD_Screen, MD_Palette, 4);
		Refresh_VDP_Palette_Outline(MD_Screen, 0x03, (unsigned short)0xFFFF);
	}
	
	Print_Text_Constant("******** VDP CONTROL ********", 29, 180, 60, BLANC);
	
	sprintf(Dbg_Out_Str, "Status : %.4X", Read_VDP_Status ());
	Print_Text(Dbg_Out_Str, strlen(Dbg_Out_Str), 176, 70, BLANC);
	sprintf(Dbg_Out_Str, "Flag : %.2X       Data : %.8X", Ctrl.Flag, Ctrl.Data);
	Print_Text(Dbg_Out_Str, strlen(Dbg_Out_Str), 176, 78, BLANC);
	sprintf(Dbg_Out_Str, "Write : %.2X      Access : %.2X", Ctrl.Write, Ctrl.Access);
	Print_Text(Dbg_Out_Str, strlen(Dbg_Out_Str), 176, 86, BLANC);
	sprintf(Dbg_Out_Str, "Address : %.4X  DMA_Mode : %.2X", Ctrl.Address, Ctrl.DMA_Mode);
	Print_Text(Dbg_Out_Str, strlen(Dbg_Out_Str), 176, 94, BLANC);
	sprintf(Dbg_Out_Str, "DMA adr: %.8X  DMA len: %.4X", VDP_Reg.DMA_Address, VDP_Reg.DMA_Length);
	Print_Text(Dbg_Out_Str, strlen(Dbg_Out_Str), 176, 102, BLANC);
	sprintf(Dbg_Out_Str, "DMA : %.2X", Ctrl.DMA);
	Print_Text(Dbg_Out_Str, strlen(Dbg_Out_Str), 176, 110, BLANC);
	
	Print_Text_Constant("Sprite List:", strlen(Dbg_Out_Str), 176, 126, BLANC);
	for (unsigned int i = 0; i < 10; i++)
	{
		sprintf(Dbg_Out_Str, "%d %d %d %d %d",
			Sprite_Struct[i].Pos_X, Sprite_Struct[i].Pos_Y,
			Sprite_Struct[i].Size_X, Sprite_Struct[i].Size_Y,
			Sprite_Struct[i].Num_Tile);
		Print_Text(Dbg_Out_Str, strlen(Dbg_Out_Str), 176, 134 + (i * 8), BLANC);
	}
}


/**
 * Refresh_SegaCD_State(): Refresh the Sega CD status display.
 */
static void Refresh_SegaCD_State(void)
{
	Print_Text_Constant("** SEGACD STATUS **", 20, 200, 1, VERT);
	
	sprintf(Dbg_Out_Str, "GE00=%.4X GE02=%.4X CD00=%.4X CD02=%.4X",
			M68K_RW (0xA12000), M68K_RW (0xA12002), S68K_RW (0xFF8000), S68K_RW (0xFF8002));
	Print_Text(Dbg_Out_Str, strlen(Dbg_Out_Str), 162, 14, BLANC);
	sprintf(Dbg_Out_Str, "GE04=%.4X GE06=%.4X CD04=%.4X CD06=%.4X",
			M68K_RW (0xA12004), M68K_RW (0xA12006), S68K_RW (0xFF8004), 0x0000);
	Print_Text(Dbg_Out_Str, strlen(Dbg_Out_Str), 162, 22, BLANC);
	sprintf(Dbg_Out_Str, "GE0A=%.4X GE0C=%.4X CD0A=%.4X CD0C=%.4X",
			M68K_RW (0xA1200A), M68K_RW (0xA1200C), S68K_RW (0xFF800A), S68K_RW (0xFF800C));
	Print_Text(Dbg_Out_Str, strlen(Dbg_Out_Str), 162, 30, BLANC);
	sprintf(Dbg_Out_Str, "GD0E=%.4X", S68K_RW (0xFF800E));
	Print_Text(Dbg_Out_Str, strlen(Dbg_Out_Str), 162, 38, BLANC);
	sprintf(Dbg_Out_Str, "GD10=%.4X GD12=%.4X GD14=%.4X GD16=%.4X",
			S68K_RW (0xFF8010), S68K_RW (0xFF8012), S68K_RW (0xFF8014), S68K_RW (0xFF8016));
	Print_Text(Dbg_Out_Str, strlen(Dbg_Out_Str), 162, 46, BLANC);
	sprintf(Dbg_Out_Str, "GD18=%.4X GD1A=%.4X GD1C=%.4X GD1E=%.4X",
			S68K_RW (0xFF8018), S68K_RW (0xFF801A), S68K_RW (0xFF801C), S68K_RW (0xFF801E));
	Print_Text(Dbg_Out_Str, strlen(Dbg_Out_Str), 162, 54, BLANC);
	sprintf(Dbg_Out_Str, "GD20=%.4X GD22=%.4X GD24=%.4X GD26=%.4X",
			S68K_RW (0xFF8020), S68K_RW (0xFF8022), S68K_RW (0xFF8024), S68K_RW (0xFF8026));
	Print_Text(Dbg_Out_Str, strlen(Dbg_Out_Str), 162, 62, BLANC);
	sprintf(Dbg_Out_Str, "GD28=%.4X GD2A=%.4X GD2C=%.4X GD2E=%.4X",
			S68K_RW (0xFF8028), S68K_RW (0xFF802A), S68K_RW (0xFF802C), S68K_RW (0xFF802E));
	Print_Text(Dbg_Out_Str, strlen(Dbg_Out_Str), 162, 70, BLANC);
	sprintf(Dbg_Out_Str, "CD30=%.4X CD32=%.4X CD34=%.4X CD36=%.4X",
			S68K_RW (0xFF8030), S68K_RW (0xFF8032), S68K_RW (0xFF8034), S68K_RW (0xFF8036));
	Print_Text(Dbg_Out_Str, strlen(Dbg_Out_Str), 162, 78, BLANC);
	sprintf(Dbg_Out_Str, "CD38=%.4X CD3A=%.4X CD3E=%.4X CD40=%.4X",
			S68K_RW (0xFF8038), S68K_RW (0xFF803A), S68K_RW (0xFF803E), S68K_RW (0xFF8040));
	Print_Text(Dbg_Out_Str, strlen(Dbg_Out_Str), 162, 86, BLANC);
	sprintf(Dbg_Out_Str, "CD42=%.4X CD44=%.4X CD48=%.4X CD4A=%.4X",
			S68K_RW (0xFF8042), S68K_RW (0xFF8044), S68K_RW (0xFF8048), S68K_RW (0xFF804A));
	Print_Text(Dbg_Out_Str, strlen(Dbg_Out_Str), 162, 94, BLANC);
	sprintf(Dbg_Out_Str, "CD4C=%.4X CD4E=%.4X CD50=%.4X CD52=%.4X",
			S68K_RW (0xFF804C), S68K_RW (0xFF804E), S68K_RW (0xFF8050), S68K_RW (0xFF8052));
	Print_Text(Dbg_Out_Str, strlen(Dbg_Out_Str), 162, 102, BLANC);
	sprintf(Dbg_Out_Str, "CD58=%.4X CD5A=%.4X CD5C=%.4X CD5E=%.4X",
			S68K_RW (0xFF8058), S68K_RW (0xFF805A), S68K_RW (0xFF805C), S68K_RW (0xFF805E));
	Print_Text(Dbg_Out_Str, strlen(Dbg_Out_Str), 162, 110, BLANC);
	sprintf(Dbg_Out_Str, "CD60=%.4X CD62=%.4X CD64=%.4X CD66=%.4X",
			S68K_RW (0xFF8060), S68K_RW (0xFF8062), S68K_RW (0xFF8064), S68K_RW (0xFF8066));
	Print_Text(Dbg_Out_Str, strlen(Dbg_Out_Str), 162, 118, BLANC);
}


/**
 * Refresh_32X_State(): Refresh the 32X status display.
 */
static void Refresh_32X_State(void)
{
	Print_Text_Constant("** 32X STATUS **", 20, 200, 1, VERT);
	
	sprintf(Dbg_Out_Str, "M000=%.4X S000=%.4X M004=%.4X M006=%.4X",
			SH2_Read_Word (&M_SH2, 0x4000), SH2_Read_Word (&S_SH2, 0x4000),
			SH2_Read_Word (&M_SH2, 0x4004), SH2_Read_Word (&M_SH2, 0x4006));
	Print_Text(Dbg_Out_Str, strlen(Dbg_Out_Str), 162, 14, BLANC);
	sprintf(Dbg_Out_Str, "M008=%.4X M00A=%.4X M00C=%.4X M00E=%.4X",
			SH2_Read_Word (&M_SH2, 0x4008), SH2_Read_Word (&M_SH2, 0x400A),
			SH2_Read_Word (&M_SH2, 0x400C), SH2_Read_Word (&M_SH2, 0x400E));
	Print_Text(Dbg_Out_Str, strlen(Dbg_Out_Str), 162, 22, BLANC);
	sprintf(Dbg_Out_Str, "M010=%.4X M012=%.4X M014=%.4X M016=%.4X",
			SH2_Read_Word (&M_SH2, 0x4010), SH2_Read_Word (&M_SH2, 0x4012),
			SH2_Read_Word (&M_SH2, 0x4014), SH2_Read_Word (&M_SH2, 0x4016));
	Print_Text(Dbg_Out_Str, strlen(Dbg_Out_Str), 162, 30, BLANC);
	sprintf(Dbg_Out_Str, "M020=%.4X M022=%.4X M024=%.4X M026=%.4X",
			SH2_Read_Word (&M_SH2, 0x4020), SH2_Read_Word (&M_SH2, 0x4022),
			SH2_Read_Word (&M_SH2, 0x4024), SH2_Read_Word (&M_SH2, 0x4026));
	Print_Text(Dbg_Out_Str, strlen(Dbg_Out_Str), 162, 38, BLANC);
	sprintf(Dbg_Out_Str, "M028=%.4X M02A=%.4X M02C=%.4X M02E=%.4X",
			SH2_Read_Word (&M_SH2, 0x4028), SH2_Read_Word (&M_SH2, 0x402A),
			SH2_Read_Word (&M_SH2, 0x402C), SH2_Read_Word (&M_SH2, 0x402E));
	Print_Text(Dbg_Out_Str, strlen(Dbg_Out_Str), 162, 46, BLANC);
	sprintf(Dbg_Out_Str, "M030=%.4X M032=%.4X M034=%.4X M036=%.4X",
			SH2_Read_Word (&M_SH2, 0x4030), SH2_Read_Word (&M_SH2, 0x4032),
			SH2_Read_Word (&M_SH2, 0x4034), SH2_Read_Word (&M_SH2, 0x4036));
	Print_Text(Dbg_Out_Str, strlen(Dbg_Out_Str), 162, 54, BLANC);
	sprintf(Dbg_Out_Str, "M100=%.4X M102=%.4X M104=%.4X M106=%.4X",
			SH2_Read_Word (&M_SH2, 0x4100), SH2_Read_Word (&M_SH2, 0x4102),
			SH2_Read_Word (&M_SH2, 0x4104), SH2_Read_Word (&M_SH2, 0x4106));
	Print_Text(Dbg_Out_Str, strlen(Dbg_Out_Str), 162, 62, BLANC);
	sprintf(Dbg_Out_Str, "M108=%.4X M10A=%.4X M10C=%.4X M10E=%.4X",
			SH2_Read_Word (&M_SH2, 0x4108), SH2_Read_Word (&M_SH2, 0x410A),
			SH2_Read_Word (&M_SH2, 0x410C), SH2_Read_Word (&M_SH2, 0x410E));
	Print_Text(Dbg_Out_Str, strlen(Dbg_Out_Str), 162, 70, BLANC);
}


/**
 * Refresh_CDC_State(): Refresh the CDC status display.
 */
static void Refresh_CDC_State(void)
{
	Print_Text_Constant("** CDC STATUS **", 16, 200, 1, VERT);
	
	sprintf(Dbg_Out_Str, "COMIN=%.2X IFSTAT=%.2X DBC=%.4X", CDC.COMIN, CDC.IFSTAT, CDC.DBC.N);
	Print_Text(Dbg_Out_Str, strlen(Dbg_Out_Str), 162, 14, BLANC);
	sprintf(Dbg_Out_Str, "HEAD=%.8X PT=%.4X WA=%.4X", CDC.HEAD.N, CDC.PT.N, CDC.WA.N);
	Print_Text(Dbg_Out_Str, strlen(Dbg_Out_Str), 162, 22, BLANC);
	sprintf(Dbg_Out_Str, "STAT=%.8X CTRL=%.8X", CDC.STAT.N, CDC.CTRL.N);
	Print_Text(Dbg_Out_Str, strlen(Dbg_Out_Str), 162, 30, BLANC);
	sprintf(Dbg_Out_Str, "DAC=%.4X IFCTRL=%.2X", CDC.DAC.N, CDC.IFCTRL);
	Print_Text(Dbg_Out_Str, strlen(Dbg_Out_Str), 162, 38, BLANC);
}


/**
 * Refresh_Word_RAM_Pattern(): Refresh the Word RAM pattern status display.
 */
static void Refresh_Word_RAM_Pattern(void)
{
	// Improved Word RAM pattern display function ported from Gens Rerecording.
	
	Print_Text_Constant("****** WORD RAM PATTERN ******", 29, 28, 0, VERT);
	
	for (unsigned int i = 0; i < 24; i++)
	{
		sprintf(Dbg_Out_Str, "%.4X", (cd_pattern_adr & 0x3FFFF) + 0x200 * i);
		Print_Text(Dbg_Out_Str, strlen(Dbg_Out_Str), 2, (i << 3) + 11, BLANC);
	}
	
	// Word RAM patterns can be either 16x16 or 32x32.
	if (Rot_Comp.Stamp_Size & 2)
		Cell_32x32_Dump(&Ram_Word_2M[cd_pattern_adr & 0x3FFFF], pattern_pal);
	else
		Cell_16x16_Dump(&Ram_Word_2M[cd_pattern_adr & 0x3FFFF], pattern_pal);
	
	Print_Text_Constant("******** VDP PALETTE ********", 29, 180, 0, ROUGE);
	
	if (bppMD == 32)
	{
		// 32-bit color palette update
		Refresh_VDP_Palette_Colors(MD_Screen32, MD_Palette32, 16);
		Refresh_VDP_Palette_Outline(MD_Screen32, 0x0F, (unsigned int)0xFFFFFF);
	}
	else
	{
		// 15/16-bit color palette update
		Refresh_VDP_Palette_Colors(MD_Screen, MD_Palette, 16);
		Refresh_VDP_Palette_Outline(MD_Screen, 0x0F, (unsigned short)0xFFFF);
	}
}


/**
 * Update_Debug_Screen(): Update the debug screen.
 */
void Update_Debug_Screen(void)
{
	// Clear the MD screen.
	if (bppMD == 32)
		memset(MD_Screen32, 0x00, sizeof(MD_Screen32));
	else
		memset(MD_Screen, 0x00, sizeof(MD_Screen));
	
	if (Debug & 0x100)
	{
		if (_32X_Started)
			Do_32X_VDP_Only();
		else
			Do_VDP_Only();
	}
	else
	{
		switch (Debug)
		{
			default:
			case DEBUG_MAIN_68000:
				// Main 68000
				Refresh_M68k_Mem();
				Refresh_M68k_Inst();
				Refresh_M68k_State();
				Refresh_VDP_State();
				break;
			
			case DEBUG_Z80:
				// Z80
				Refresh_Z80_Mem();
				Refresh_Z80_Inst();
				Refresh_Z80_State();
				break;
			
			case DEBUG_GENESIS_VDP:
				// Genesis VDP
				Refresh_VDP_Pattern();
				Refresh_VDP_Palette();
				break;
			
			case DEBUG_SUB_68000_REG:
				// Sub 68000 reg
				Refresh_S68k_Mem();
				Refresh_S68k_Inst();
				Refresh_S68k_State();
				Refresh_SegaCD_State();
				break;
			
			case DEBUG_SUB_68000_CDC:
				// Sub 68000 CDC
				Refresh_S68k_Mem();
				Refresh_S68k_Inst();
				Refresh_S68k_State();
				Refresh_CDC_State();
				break;
			
			case DEBUG_WORD_RAM_PATTERN:
				// Word RAM pattern
				Refresh_Word_RAM_Pattern();
				break;
			
			case DEBUG_MAIN_SH2:
				// Main SH2
				Refresh_SH2_Mem();
				Refresh_SH2_Inst(0);
				Refresh_SH2_State(0);
				Refresh_32X_State();
				break;
			
			case DEBUG_SUB_SH2:
				// Sub SH2
				Refresh_SH2_Mem();
				Refresh_SH2_Inst(1);
				Refresh_SH2_State(1);
				Refresh_32X_State();
				break;
			
			case DEBUG_32X_VDP:
				// 32X VDP
				_32X_VDP_Draw(Current_32X_FB);
				break;
		}
	}
	
	GensUI::sleep(10);
}
