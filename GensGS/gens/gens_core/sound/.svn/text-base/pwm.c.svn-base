/***************************************************************************
 * Gens: PWM audio emulator.                                               *
 *                                                                         *
 * Copyright (c) 1999-2002 by Stéphane Dallongeville                       *
 * Copyright (c) 2003-2004 by Stéphane Akhoun                              *
 * Copyright (c) 2008 by David Korth                                       *
 *                                                                         *
 * This program is free software; you can redistribute it and/or modify it *
 * under the terms of the GNU General Public License as published by the   *
 * Free Software Foundation; either version 2 of the License, or (at your  *
 * option) any later version.                                              *
 *                                                                         *
 * This program is distributed in the hope that it will be useful, but     *
 * WITHOUT ANY WARRANTY; without even the implied warranty of              *
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the           *
 * GNU General Public License for more details.                            *
 *                                                                         *
 * You should have received a copy of the GNU General Public License along *
 * with this program; if not, write to the Free Software Foundation, Inc., *
 * 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.           *
 ***************************************************************************/

#include "pwm.h"

#include <string.h>

#include "gens_core/mem/mem_sh2.h"

#if PWM_BUF_SIZE == 8
unsigned char PWM_FULL_TAB[PWM_BUF_SIZE * PWM_BUF_SIZE] =
{
	0x40, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x80,
	0x80, 0x40, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x80, 0x40, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x80, 0x40, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x80, 0x40, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x80, 0x40, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x80, 0x40, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x80, 0x40,
};
#elif PWM_BUF_SIZE == 4
unsigned char PWM_FULL_TAB[PWM_BUF_SIZE * PWM_BUF_SIZE] =
{
	0x40, 0x00, 0x00, 0x80,
	0x80, 0x40, 0x00, 0x00,
	0x00, 0x80, 0x40, 0x00,
	0x00, 0x00, 0x80, 0x40,
};
#else
#error PWM_BUF_SIZE must equal 4 or 8.
#endif /* PWM_BUF_SIZE */

unsigned short PWM_FIFO_R[8];
unsigned short PWM_FIFO_L[8];
unsigned int PWM_RP_R;
unsigned int PWM_WP_R;
unsigned int PWM_RP_L;
unsigned int PWM_WP_L;
unsigned int PWM_Cycles;
unsigned int PWM_Cycle;
unsigned int PWM_Cycle_Cnt;
unsigned int PWM_Int;
unsigned int PWM_Int_Cnt;
unsigned int PWM_Mode;
unsigned int PWM_Enable;
unsigned int PWM_Out_R;
unsigned int PWM_Out_L;

unsigned int PWM_Cycle_Tmp;
unsigned int PWM_Cycles_Tmp;
unsigned int PWM_Int_Tmp;
unsigned int PWM_FIFO_L_Tmp;
unsigned int PWM_FIFO_R_Tmp;


/**
 * PWM_Init(): Initialize the PWM audio emulator.
 */
void PWM_Init(void)
{
	PWM_Mode = 0;
	PWM_Out_R = 0;
	PWM_Out_L = 0;
	
	memset(PWM_FIFO_R, 0x00, sizeof(PWM_FIFO_R));
	memset(PWM_FIFO_L, 0x00, sizeof(PWM_FIFO_L));
	
	PWM_RP_R = 0;
	PWM_WP_R = 0;
	PWM_RP_L = 0;
	PWM_WP_L = 0;
	PWM_Cycle_Tmp = 0;
	PWM_Int_Tmp = 0;
	PWM_FIFO_L_Tmp = 0;
	PWM_FIFO_R_Tmp = 0;
	
	PWM_Set_Cycle(0);
	PWM_Set_Int(0);
}


void PWM_Set_Cycle(unsigned int cycle)
{
	cycle--;
	PWM_Cycle = (cycle & 0xFFF);
	PWM_Cycle_Cnt = PWM_Cycles;
}


void PWM_Set_Int(unsigned int int_time)
{
	int_time &= 0x0F;
	if (int_time)
		PWM_Int = PWM_Int_Cnt = int_time;
	else
		PWM_Int = PWM_Int_Cnt = 16;
}


void PWM_Clear_Timer(void)
{
	PWM_Cycle_Cnt = 0;
}


/**
 * PWM_SHIFT(): Shift PWM data.
 * @param src: Channel (L or R) with the source data.
 * @param dest Channel (L or R) for the destination.
 */
#define PWM_SHIFT(src, dest)										\
{													\
	/* Make sure the source FIFO isn't empty. */							\
	if (PWM_RP_##src != PWM_WP_##src)								\
	{												\
		/* Get destination channel output from the source channel FIFO. */			\
		PWM_Out_##dest = PWM_FIFO_##src[PWM_RP_##src];						\
													\
		/* Increment the source channel read pointer, resetting to 0 if it overflows. */	\
		PWM_RP_##src = (PWM_RP_##src + 1) & (PWM_BUF_SIZE - 1);					\
	}												\
}


static void PWM_Shift_Data(void)
{
	switch (PWM_Mode & 0x0F)
	{
		case 0x01:
		case 0x0D:
			// Rx_LL: Right -> Ignore, Left -> Left
			PWM_SHIFT(L, L);
			break;
		
		case 0x02:
		case 0x0E:
			// Rx_LR: Right -> Ignore, Left -> Right
			PWM_SHIFT(L, R);
			break;
		
		case 0x04:
		case 0x07:
			// RL_Lx: Right -> Left, Left -> Ignore
			PWM_SHIFT(R, L);
			break;
		
		case 0x05:
		case 0x09:
			// RR_LL: Right -> Right, Left -> Left
			PWM_SHIFT(L, L);
			PWM_SHIFT(R, R);
			break;
		
		case 0x06:
		case 0x0A:
			// RL_LR: Right -> Left, Left -> Right
			PWM_SHIFT(L, R);
			PWM_SHIFT(R, L);
			break;
		
		case 0x08:
		case 0x0B:
			// RR_Lx: Right -> Right, Left -> Ignore
			PWM_SHIFT(R, R);
			break;
		
		case 0x00:
		case 0x03:
		case 0x0C:
		case 0x0F:
		default:
			// Rx_Lx: Right -> Ignore, Left -> Ignore
			break;
	}
}


void PWM_Update_Timer(unsigned int cycle)
{
	// Don't do anything if PWM is disabled in the Sound menu.
	if (!PWM_Enable)
		return;
	
	// Don't do anything if PWM isn't active.
	if ((PWM_Mode & 0x0F) == 0x00)
		return;
	
	if (PWM_Cycle == 0x00 || (PWM_Cycle_Cnt > cycle))
		return;
	
	PWM_Shift_Data();
	
	PWM_Cycle_Cnt += PWM_Cycle;
	
	PWM_Int_Cnt--;
	if (PWM_Int_Cnt == 0)
	{
		PWM_Int_Cnt = PWM_Int;
		
		if (_32X_MINT & 1)
			SH2_Interrupt(&M_SH2, 6);
		if (_32X_SINT & 1)
			SH2_Interrupt(&S_SH2, 6);
	}
}


void PWM_Update(int **buf, int length)
{
	if (!PWM_Enable)
		return;
	
	int tmpOutL = ((PWM_Out_L & 0xFFFF) << 5) - 0x4000;
	int tmpOutR = ((PWM_Out_R & 0xFFFF) << 5) - 0x4000;
	
	// Multiply PWM by 4 so it's audible.
	tmpOutL <<= 2;
	tmpOutR <<= 2;
	
	while (length > 0)
	{
		if (PWM_Out_L)
			buf[0][length-1] += tmpOutL;
		if (PWM_Out_R)
			buf[1][length-1] += tmpOutR;
		
		length--;
	}
}


// Symbol aliases for cross-OS asm compatibility.
void _PWM_Init(void)
		__attribute__ ((weak, alias ("PWM_Init")));
void _PWM_Set_Cycle(unsigned int cycle)
		__attribute__ ((weak, alias ("PWM_Set_Cycle")));
void _PWM_Set_Int(unsigned int int_time)
		__attribute__ ((weak, alias ("PWM_Set_Int")));
void _PWM_Clear_Timer(void)
		__attribute__ ((weak, alias ("PWM_Clear_Timer")));
void _PWM_Update_Timer(void)
		__attribute__ ((weak, alias ("PWM_Update_Timer")));
void _PWM_Update(void)
		__attribute__ ((weak, alias ("PWM_Update")));
