/***************************************************************************
 * Gens: Controller I/O.                                                   *
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

#include <stdio.h>

#include "io.h"
#include "io_teamplayer.h"

unsigned int Controller_1_State		= 0x40;
unsigned int Controller_1_COM		= 0;
unsigned int Controller_1_Counter	= 0;
unsigned int Controller_1_Delay		= 0;

unsigned int Controller_1_Type		= 0;
unsigned int Controller_1B_Type		= 0;
unsigned int Controller_1C_Type		= 0;
unsigned int Controller_1D_Type		= 0;

unsigned int Controller_1_Buttons	= 0xFFFFFFFF;
unsigned int Controller_1B_Buttons	= 0xFFFFFFFF;
unsigned int Controller_1C_Buttons	= 0xFFFFFFFF;
unsigned int Controller_1D_Buttons	= 0xFFFFFFFF;

unsigned int Controller_2_State		= 0x40;
unsigned int Controller_2_COM		= 0;
unsigned int Controller_2_Counter	= 0;
unsigned int Controller_2_Delay		= 0;

unsigned int Controller_2_Type		= 0;
unsigned int Controller_2B_Type		= 0;
unsigned int Controller_2C_Type		= 0;
unsigned int Controller_2D_Type		= 0;

unsigned int Controller_2_Buttons	= 0xFFFFFFFF;
unsigned int Controller_2B_Buttons	= 0xFFFFFFFF;
unsigned int Controller_2C_Buttons	= 0xFFFFFFFF;
unsigned int Controller_2D_Buttons	= 0xFFFFFFFF;

// These are only for io_old.asm compatibility.
#define CREATE_CONTROLLER_OLD_VARIABLES(player)			\
	unsigned int Controller_ ## player ## _Up	= 1;	\
	unsigned int Controller_ ## player ## _Down	= 1;	\
	unsigned int Controller_ ## player ## _Left	= 1;	\
	unsigned int Controller_ ## player ## _Right	= 1;	\
	unsigned int Controller_ ## player ## _Start	= 1;	\
	unsigned int Controller_ ## player ## _A	= 1;	\
	unsigned int Controller_ ## player ## _B	= 1;	\
	unsigned int Controller_ ## player ## _C	= 1;	\
	unsigned int Controller_ ## player ## _Mode	= 1;	\
	unsigned int Controller_ ## player ## _X	= 1;	\
	unsigned int Controller_ ## player ## _Y	= 1;	\
	unsigned int Controller_ ## player ## _Z	= 1;

CREATE_CONTROLLER_OLD_VARIABLES(1);
CREATE_CONTROLLER_OLD_VARIABLES(1B);
CREATE_CONTROLLER_OLD_VARIABLES(1C);
CREATE_CONTROLLER_OLD_VARIABLES(1D);

CREATE_CONTROLLER_OLD_VARIABLES(2);
CREATE_CONTROLLER_OLD_VARIABLES(2B);
CREATE_CONTROLLER_OLD_VARIABLES(2C);
CREATE_CONTROLLER_OLD_VARIABLES(2D);

// Select Line status
enum SelectLine
{
	First_Low	= 0x00,
	First_High	= 0x01,
	Second_Low	= 0x02,
	Second_High	= 0x03,
	Third_Low	= 0x04,
	Third_High	= 0x05,
	Fourth_Low	= 0x06,
	Fourth_High	= 0x07,
};


static unsigned char RD_Controller(unsigned int state,
				   unsigned int type,
				   unsigned int counter,
				   unsigned int buttons[4]);


#define CREATE_CONTROLLER_OLD(player)											\
{															\
	Controller_ ## player ## _Up		= (Controller_ ## player ## _Buttons & CONTROLLER_UP) ? 1 : 0;		\
	Controller_ ## player ## _Down		= (Controller_ ## player ## _Buttons & CONTROLLER_DOWN) ? 1 : 0;	\
	Controller_ ## player ## _Left		= (Controller_ ## player ## _Buttons & CONTROLLER_LEFT) ? 1 : 0;	\
	Controller_ ## player ## _Right		= (Controller_ ## player ## _Buttons & CONTROLLER_RIGHT) ? 1 : 0;	\
	Controller_ ## player ## _Start		= (Controller_ ## player ## _Buttons & CONTROLLER_START) ? 1 : 0;	\
	Controller_ ## player ## _A		= (Controller_ ## player ## _Buttons & CONTROLLER_A) ? 1 : 0;		\
	Controller_ ## player ## _B		= (Controller_ ## player ## _Buttons & CONTROLLER_B) ? 1 : 0;		\
	Controller_ ## player ## _C		= (Controller_ ## player ## _Buttons & CONTROLLER_C) ? 1 : 0;		\
	Controller_ ## player ## _Mode		= (Controller_ ## player ## _Buttons & CONTROLLER_MODE) ? 1 : 0;	\
	Controller_ ## player ## _X		= (Controller_ ## player ## _Buttons & CONTROLLER_X) ? 1 : 0;		\
	Controller_ ## player ## _Y		= (Controller_ ## player ## _Buttons & CONTROLLER_Y) ? 1 : 0;		\
	Controller_ ## player ## _Z		= (Controller_ ## player ## _Buttons & CONTROLLER_Z) ? 1 : 0;		\
}


unsigned char RD_Controller_1(void)
{
	// Read controller 1.
	
	if (Controller_1_Type & 0x10)
	{
		// TeamPlayer.
		CREATE_CONTROLLER_OLD(1);
		CREATE_CONTROLLER_OLD(1B);
		CREATE_CONTROLLER_OLD(1C);
		CREATE_CONTROLLER_OLD(1D);
		CREATE_CONTROLLER_OLD(2);
		CREATE_CONTROLLER_OLD(2B);
		CREATE_CONTROLLER_OLD(2C);
		CREATE_CONTROLLER_OLD(2D);
		return RD_Controller_1_TP();
	}
	
	// Create the bitfields.
	// TODO: This will be unnecessary when controllers are converted to bitfields.
	unsigned int buttons[4] =
	{
		Controller_1_Buttons,
		Controller_1B_Buttons,
		Controller_1C_Buttons,
		Controller_1D_Buttons,
	};
	
	// Read the controller data.
	return RD_Controller(Controller_1_State, Controller_1_Type,
			     Controller_1_Counter, buttons);
}


unsigned char RD_Controller_2(void)
{
	// Read controller 2.
	
	if (Controller_2_Type & 0x10)
	{
		// TeamPlayer.
		CREATE_CONTROLLER_OLD(1);
		CREATE_CONTROLLER_OLD(1B);
		CREATE_CONTROLLER_OLD(1C);
		CREATE_CONTROLLER_OLD(1D);
		CREATE_CONTROLLER_OLD(2);
		CREATE_CONTROLLER_OLD(2B);
		CREATE_CONTROLLER_OLD(2C);
		CREATE_CONTROLLER_OLD(2D);
		return RD_Controller_2_TP();
	}
	
	// Create the bitfields.
	// TODO: This will be unnecessary when controllers are converted to bitfields.
	unsigned int buttons[4] =
	{
		Controller_2_Buttons,
		Controller_2B_Buttons,
		Controller_2C_Buttons,
		Controller_2D_Buttons,
	};
	
	// Read the controller data.
	return RD_Controller(Controller_2_State, Controller_2_Type,
			     Controller_2_Counter, buttons);
}


static unsigned char RD_Controller(unsigned int state,
				   unsigned int type,
				   unsigned int counter,
				   unsigned int buttons[4])
{
	// Read the specified controller.
	
	int SelectLine_State = ((state >> 6) & 0x01);
	if (type & 0x01)
	{
		// 6-button controller. Add the counter value.
		SelectLine_State += ((counter & 0x03) * 2);
	}
	
	unsigned char out = 0;
	switch (SelectLine_State)
	{
		case First_High:
		case Second_High:
		case Third_High:
			// Format: 01CBRLDU
			if (buttons[0] & CONTROLLER_UP)
				out |= 0x01;
			if (buttons[0] & CONTROLLER_DOWN)
				out |= 0x02;
			if (buttons[0] & CONTROLLER_LEFT)
				out |= 0x04;
			if (buttons[0] & CONTROLLER_RIGHT)
				out |= 0x08;
			if (buttons[0] & CONTROLLER_B)
				out |= 0x10;
			if (buttons[0] & CONTROLLER_C)
				out |= 0x20;
			
			return (out | 0x40);
		
		case First_Low:
		case Second_Low:
			// Format: 00SA00DU
			if (buttons[0] & CONTROLLER_UP)
				out |= 0x01;
			if (buttons[0] & CONTROLLER_DOWN)
				out |= 0x02;
			if (buttons[0] & CONTROLLER_A)
				out |= 0x10;
			if (buttons[0] & CONTROLLER_START)
				out |= 0x20;
			
			return out;
		
		case Third_Low:
			// Format: 00SA0000
			if (buttons[0] & CONTROLLER_A)
				out |= 0x10;
			if (buttons[0] & CONTROLLER_START)
				out |= 0x20;
			
			return out;
		
		case Fourth_High:
			// Format: 01CBMXYZ
			if (buttons[0] & CONTROLLER_Z)
				out |= 0x01;
			if (buttons[0] & CONTROLLER_Y)
				out |= 0x02;
			if (buttons[0] & CONTROLLER_X)
				out |= 0x04;
			if (buttons[0] & CONTROLLER_MODE)
				out |= 0x08;
			if (buttons[0] & CONTROLLER_B)
				out |= 0x10;
			if (buttons[0] & CONTROLLER_C)
				out |= 0x20;
			
			return (out | 0x40);
		
		case Fourth_Low:
			// Format: 00SA1111
			if (buttons[0] & CONTROLLER_A)
				out |= 0x10;
			if (buttons[0] & CONTROLLER_START)
				out |= 0x20;
			
			return (out | 0x0F);
	}
	
	// Shouldn't happen...
	printf("%s: Invalid Select Line State: %d\n", __func__, SelectLine_State);
	return 0x00;
}


unsigned char WR_Controller_1(unsigned char data)
{
	if (!(Controller_1_State & 0x40) && (data & 0x40))
		Controller_1_Counter++;
	
	Controller_1_Delay = 0;
	
	if (!(Controller_1_Type & 0x10))
	{
		// Not Team Player, so skip the rest of the function.
		Controller_1_State = data;
		return data;
	}
	
	// Team Player adapter
	if (!(Controller_1_State & 0x20) && (data & 0x20))
		Controller_1_Counter += (1 << 16);
	
	Controller_1_State = data;
	return data;
}


unsigned char WR_Controller_2(unsigned char data)
{
	if (!(Controller_2_State & 0x40) && (data & 0x40))
		Controller_2_Counter++;
	
	// Team Player on Port 1
	// TODO: Why is this here?
	if ((Controller_1_Type & 0x10) && (data & 0x0C))
		Controller_1_Counter = 0;
	
	Controller_2_Delay = 0;
	
	if (!(Controller_2_Type & 0x10))
	{
		// Not Team Player, so skip the rest of the function.
		Controller_2_State = data;
		return data;
	}
	
	// Team Player adapter
	if (!(Controller_2_State & 0x20) && (data & 0x20))
		Controller_2_Counter += (1 << 16);
	
	Controller_2_State = data;
	return data;
}


void Fix_Controllers(void)
{
	if (Controller_1_Delay <= 25)
		Controller_1_Delay++;
	else
	{
		//Controller_1_Delay = 0; // commented out in asm
		Controller_1_Counter = 0;
	}
	
	if (Controller_2_Delay <= 25)
		Controller_2_Delay++;
	else
	{
		//Controller_2_Delay = 0; // commented out in asm
		Controller_2_Counter = 0;
	}
}


// Symbol aliases for cross-OS asm compatibility.
unsigned char _RD_Controller_1(void)
	__attribute__ ((weak, alias ("RD_Controller_1")));
unsigned char _RD_Controller_2(void)
	__attribute__ ((weak, alias ("RD_Controller_2")));
unsigned char _WR_Controller_1(void)
	__attribute__ ((weak, alias ("WR_Controller_1")));
unsigned char _WR_Controller_2(void)
	__attribute__ ((weak, alias ("WR_Controller_2")));
