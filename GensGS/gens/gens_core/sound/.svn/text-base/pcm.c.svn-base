/***********************************************************/
/*                                                         */
/* PCM.C : PCM RF5C164 emulator                            */
/*                                                         */
/* This source is a part of Gens project                   */
/* Written by Stéphane Dallongeville (gens@consolemul.com) */
/* Copyright (c) 2002 by Stéphane Dallongeville            */
/*                                                         */
/***********************************************************/

#include <stdio.h>
#include <string.h>
#include "gens_core/sound/pcm.h"
#include "segacd/cd_sys.hpp"
#include "gens_core/cpu/68k/star_68k.h"
#include "gens_core/mem/mem_m68k.h"

#define PCM_STEP_SHIFT 11

struct pcm_chip_ PCM_Chip;

static int PCM_Volume_Tab[256 * 256];

unsigned char Ram_PCM[64 * 1024];
int PCM_Enable;

/**
 * Init_PCM(): Initialize the PCM chip.
 * @param Rate Sample rate.
 * @return 0 if successful.
 */
int Init_PCM(int Rate)
{
	int i, j, out;
	
	for (i = 0; i < 0x100; i++)
	{
		for (j = 0; j < 0x100; j++)
		{
			if (i & 0x80)
			{
				out = -(i & 0x7F);
				out *= j;
				PCM_Volume_Tab[(j << 8) + i] = out;
			}
			else
			{
				out = i * j;
				PCM_Volume_Tab[(j << 8) + i] = out;
			}
		}
	}
	
	Reset_PCM();
	Set_Rate_PCM(Rate);
	
	return 0;
}

/**
 * Reset_PCM(): Reset the PCM chip.
 */
void Reset_PCM(void)
{
	int i;
	
	// Clear the PCM memory.
	memset(Ram_PCM, 0, 64 * 1024);
	
	PCM_Chip.Enable = 0;
	
	/* clear channel registers */
	for (i = 0; i < 8; i++)
	{
		PCM_Chip.Channel[i].Enable = 0;
		PCM_Chip.Channel[i].ENV = 0;
		PCM_Chip.Channel[i].PAN = 0;
		PCM_Chip.Channel[i].St_Addr = 0;
		PCM_Chip.Channel[i].Addr = 0;
		PCM_Chip.Channel[i].Loop_Addr = 0;
		PCM_Chip.Channel[i].Step = 0;
		PCM_Chip.Channel[i].Step_B = 0;
		PCM_Chip.Channel[i].Data = 0;
	}
}

/**
 * Set_Rate_PCM(): Change the PCM sample rate.
 * @param Rate New sample rate.
 */
void Set_Rate_PCM(int Rate)
{
	int i;
	
	if (Rate == 0)
		return;
	
	//PCM_Chip.Rate = (float) (32 * 1024) / (float) Rate;
	PCM_Chip.Rate = (float) (31.8 * 1024) / (float) Rate;
	
	for (i = 0; i < 8; i++)
	{
		PCM_Chip.Channel[i].Step =
			(int) ((float) PCM_Chip.Channel[i].Step_B * PCM_Chip.Rate);
	}
}

/* write pcm register */
/**
 * Write_PCM_Reg(): Write to a PCM register.
 * @param Reg Register ID.
 * @param Data Data to write.
 */
void Write_PCM_Reg(unsigned int Reg, unsigned int Data)
{
	int i;
	
	Data &= 0xFF;
	
	switch (Reg)
	{
		case 0x00:
			/* evelope register */
			PCM_Chip.Channel[PCM_Chip.Cur_Chan].ENV = Data;
			PCM_Chip.Channel[PCM_Chip.Cur_Chan].MUL_L =
				(Data * (PCM_Chip.Channel[PCM_Chip.Cur_Chan].PAN & 0x0F)) >> 5;
			PCM_Chip.Channel[PCM_Chip.Cur_Chan].MUL_R =
				(Data * (PCM_Chip.Channel[PCM_Chip.Cur_Chan].PAN >> 4)) >> 5;
		break;
		
		case 0x01:
			/* pan register */
			PCM_Chip.Channel[PCM_Chip.Cur_Chan].PAN = Data;
			PCM_Chip.Channel[PCM_Chip.Cur_Chan].MUL_L =
				((Data & 0x0F) * PCM_Chip.Channel[PCM_Chip.Cur_Chan].ENV) >> 5;
			PCM_Chip.Channel[PCM_Chip.Cur_Chan].MUL_R =
				((Data >> 4) * PCM_Chip.Channel[PCM_Chip.Cur_Chan].ENV) >> 5;
			break;
		
		case 0x02:
			/* frequency step (LB) registers */
			PCM_Chip.Channel[PCM_Chip.Cur_Chan].Step_B &= 0xFF00;
			PCM_Chip.Channel[PCM_Chip.Cur_Chan].Step_B += Data;
			PCM_Chip.Channel[PCM_Chip.Cur_Chan].Step =
				(int)((float)PCM_Chip.Channel[PCM_Chip.Cur_Chan].Step_B * PCM_Chip.Rate);
#ifdef DEBUG_CD
			fprintf(debug_SCD_file,
				 "PCM : Step low = %.2X   Step calculated = %.8X\n", Data,
				 PCM_Chip.Channel[PCM_Chip.Cur_Chan].Step);
#endif
			break;
		
		case 0x03:
			/* frequency step (HB) registers */
			PCM_Chip.Channel[PCM_Chip.Cur_Chan].Step_B &= 0x00FF;
			PCM_Chip.Channel[PCM_Chip.Cur_Chan].Step_B += Data << 8;
			PCM_Chip.Channel[PCM_Chip.Cur_Chan].Step =
				(int)((float)PCM_Chip.Channel[PCM_Chip.Cur_Chan].Step_B * PCM_Chip.Rate);
#ifdef DEBUG_CD
			fprintf(debug_SCD_file,
				 "PCM : Step high = %.2X   Step calculated = %.8X\n", Data,
				 PCM_Chip.Channel[PCM_Chip.Cur_Chan].Step);
#endif
			break;
		
		case 0x04:
			PCM_Chip.Channel[PCM_Chip.Cur_Chan].Loop_Addr &= 0xFF00;
			PCM_Chip.Channel[PCM_Chip.Cur_Chan].Loop_Addr += Data;			
#ifdef DEBUG_CD
			fprintf(debug_SCD_file, "PCM : Loop low = %.2X   Loop = %.8X\n", Data,
				 PCM_Chip.Channel[PCM_Chip.Cur_Chan].Loop_Addr);
#endif
			break;
		
		case 0x05:
			/* loop address registers */
			PCM_Chip.Channel[PCM_Chip.Cur_Chan].Loop_Addr &= 0x00FF;
			PCM_Chip.Channel[PCM_Chip.Cur_Chan].Loop_Addr += Data << 8;
#ifdef DEBUG_CD
			fprintf(debug_SCD_file, "PCM : Loop high = %.2X   Loop = %.8X\n", Data,
				PCM_Chip.Channel[PCM_Chip.Cur_Chan].Loop_Addr);
#endif
			break;
		
		case 0x06:
			/* start address registers */
			PCM_Chip.Channel[PCM_Chip.Cur_Chan].St_Addr =
				Data << (PCM_STEP_SHIFT + 8);
			//PCM_Chip.Channel[PCM_Chip.Cur_Chan].Addr = PCM_Chip.Channel[PCM_Chip.Cur_Chan].St_Addr;
#ifdef DEBUG_CD
			fprintf(debug_SCD_file, "PCM : Start addr = %.2X   New Addr = %.8X\n",
				 Data, PCM_Chip.Channel[PCM_Chip.Cur_Chan].Addr);
#endif
			break;
		
		case 0x07:
			/* control register */
			/* mod is H */
			if (Data & 0x40)
			{
				/* select channel */
				PCM_Chip.Cur_Chan = Data & 0x07;
			}
			/* mod is L */
			else
			{
				/* pcm ram bank select */
				PCM_Chip.Bank = (Data & 0x0F) << 12;
			}
			
			/* sounding bit */
			if (Data & 0x80)
				PCM_Chip.Enable = 0xFF;	// Used as mask
			else
				PCM_Chip.Enable = 0;
			
#ifdef DEBUG_CD
			fprintf(debug_SCD_file, "PCM : General Enable = %.2X\n", Data);
#endif
			break;
		
		case 0x08:
			/* sound on/off register */
			Data ^= 0xFF;

#ifdef DEBUG_CD
			fprintf(debug_SCD_file, "PCM : Channel Enable = %.2X\n", Data);
#endif
			
			for (i = 0; i < 8; i++)
			{
				if (!PCM_Chip.Channel[i].Enable)
					PCM_Chip.Channel[i].Addr = PCM_Chip.Channel[i].St_Addr;
			}
			
			for (i = 0; i < 8; i++)
			{
				PCM_Chip.Channel[i].Enable = Data & (1 << i);
			}
	}
}


/**
 * Update_PCM(): Update the PCM buffer.
 * @param buf PCM buffer.
 * @param Length Buffer length.
 */
int Update_PCM(int **buf, int Length)
{
	int i, j;
	int *bufL, *bufR;		//, *volL, *volR;
	unsigned int Addr, k;
	struct pcm_chan_ *CH;
	
	// if PCM disable, no sound
	if (!PCM_Chip.Enable)
		return 1;
	
	bufL = buf[0];
	bufR = buf[1];
	
#if 0
	// faster for short update
	for (j = 0; j < Length; j++)
	{
		for (i = 0; i < 8; i++)
		{
			CH = &(PCM_Chip.Channel[i]);

			// only loop when sounding and on
			if (CH->Enable)
			{
				Addr = CH->Addr >> PCM_STEP_SHIFT;

				if (Addr & 0x10000)
				{
					for(k = CH->Old_Addr; k < 0x10000; k++)
					{
						if (Ram_PCM[k] == 0xFF)
						{
							CH->Old_Addr = Addr = CH->Loop_Addr;
							CH->Addr = Addr << PCM_STEP_SHIFT;
							break;
						}
					}

					if (Addr & 0x10000)
					{
						//CH->Addr -= CH->Step;
						CH->Enable = 0;
						break;
					}
				}
				else
				{
					for(k = CH->Old_Addr; k <= Addr; k++)
					{
						if (Ram_PCM[k] == 0xFF)
						{
							CH->Old_Addr = Addr = CH->Loop_Addr;
							CH->Addr = Addr << PCM_STEP_SHIFT;
							break;
						}
					}
				}

				// test for loop signal
				if (Ram_PCM[Addr] == 0xFF)
				{
					Addr = CH->Loop_Addr;
					CH->Addr = Addr << PCM_STEP_SHIFT;
				}

				if (Ram_PCM[Addr] & 0x80)
				{
					CH->Data = Ram_PCM[Addr] & 0x7F;
					bufL[j] -= CH->Data * CH->MUL_L;
					bufR[j] -= CH->Data * CH->MUL_R;
				}
				else
				{
					CH->Data = Ram_PCM[Addr];
					bufL[j] += CH->Data * CH->MUL_L;
					bufR[j] += CH->Data * CH->MUL_R;
				}

				// update address register
//				CH->Addr = (CH->Addr + CH->Step) & 0x7FFFFFF;
				CH->Addr += CH->Step;
				CH->Old_Addr = Addr + 1;
			}
		}
	}
#endif
	
	// for long update
	for (i = 0; i < 8; i++)
	{
		CH = &(PCM_Chip.Channel[i]);
		
		// only loop when sounding and on
		if (CH->Enable)
		{
			Addr = CH->Addr >> PCM_STEP_SHIFT;
			//volL = &(PCM_Volume_Tab[CH->MUL_L << 8]);
			//volR = &(PCM_Volume_Tab[CH->MUL_R << 8]);
			
			for (j = 0; j < Length; j++)
			{
				// test for loop signal
				if (Ram_PCM[Addr] == 0xFF)
				{
					CH->Addr = (Addr = CH->Loop_Addr) << PCM_STEP_SHIFT;
					if (Ram_PCM[Addr] == 0xFF)
						break;
					else
						j--;
				}
				else
				{
					/*
					CH->Data = Ram_PCM[Addr];
					
					bufL[j] += volL[CH->Data];
					bufR[j] += volR[CH->Data];
					*/
					if (Ram_PCM[Addr] & 0x80)
					{
						CH->Data = Ram_PCM[Addr] & 0x7F;
						bufL[j] -= CH->Data * CH->MUL_L;
						bufR[j] -= CH->Data * CH->MUL_R;
					}
					else
					{
						CH->Data = Ram_PCM[Addr];
						bufL[j] += CH->Data * CH->MUL_L;
						bufR[j] += CH->Data * CH->MUL_R;
					}
					
					// update address register
					k = Addr + 1;
					CH->Addr = (CH->Addr + CH->Step) & 0x7FFFFFF;
					Addr = CH->Addr >> PCM_STEP_SHIFT;
					
					for (; k < Addr; k++)
					{
						if (Ram_PCM[k] == 0xFF)
						{
							CH->Addr = (Addr = CH->Loop_Addr) << PCM_STEP_SHIFT;
							break;
						}
					}
				}
			}
			
			if (Ram_PCM[Addr] == 0xFF)
			{
				CH->Addr = CH->Loop_Addr << PCM_STEP_SHIFT;
			}
		}
	}
	
	return 0;
}


// Symbol aliases for cross-OS asm compatibility.
void _Write_PCM_Reg(unsigned int Reg, unsigned int Data)
	__attribute__ ((weak, alias ("Write_PCM_Reg")));
