/**
 ** Supermodel
 ** A Sega Model 3 Arcade Emulator.
 ** Copyright 2011 Bart Trzynadlowski, Nik Henson 
 **
 ** This file is part of Supermodel.
 **
 ** Supermodel is free software: you can redistribute it and/or modify it under
 ** the terms of the GNU General Public License as published by the Free 
 ** Software Foundation, either version 3 of the License, or (at your option)
 ** any later version.
 **
 ** Supermodel is distributed in the hope that it will be useful, but WITHOUT
 ** ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 ** FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for
 ** more details.
 **
 ** You should have received a copy of the GNU General Public License along
 ** with Supermodel.  If not, see <http://www.gnu.org/licenses/>.
 **/
 
/*
 * Model3.cpp
 * 
 * Implementation of the CModel3 class: a complete Model 3 machine.
 *
 * To-Do List
 * ----------
 *	- ROM sets should probably be handled with a class that manages ROM
 *	  loading, the game list, as well as ROM patching
 *	- Wrap up CPU emulation inside a class (hah!)
 *	- Update the to-do list! I forgot lots of other stuff here :)
 *
 * PowerPC Address Map (may be slightly out of date/incomplete)
 * -------------------
 * 00000000-007FFFFF	RAM
 * 84000000-8400003F	Real3D Status Registers
 * 88000000-88000007	Real3D Command Port
 * 8C000000-8C3FFFFF	Real3D Culling RAM (Low)
 * 8E000000-8E0FFFFF	Real3D Culling RAM (High)
 * 90000000-9000000B	Real3D VROM Texture Port
 * 94000000-940FFFFF	Real3D Texture FIFO
 * 98000000-980FFFFF	Real3D Polygon RAM
 * C0000000-C00000FF	SCSI (Step 1.x)
 * C1000000-C10000FF	SCSI (Step 1.x) (Lost World expects it here)
 * C2000000-C20000FF	Real3D DMA (Step 2.x)
 * F0040000-F004003F	Input (Controls) Registers
 * F0080000-F0080007	Sound Board Registers
 * F00C0000-F00DFFFF	Backup RAM
 * F0100000-F010003F	System Registers
 * F0140000-F014003F	Real-Time Clock
 * F0180000-F019FFFF	Security Board RAM
 * F01A0000-F01A003F	Security Board Registers
 * F0800CF8-F0800CFF	MPC105 CONFIG_ADDR (Step 1.x)
 * F0C00CF8-F0800CFF	MPC105 CONFIG_DATA (Step 1.x)
 * F1000000-F10F7FFF	Tile Generator Pattern Table
 * F10F8000-F10FFFFF	Tile Generator Name Table
 * F1100000-F111FFFF	Tile Generator Palette
 * F1180000-F11800FF	Tile Generator Registers
 * F8FFF000-F8FFF0FF	MPC105 (Step 1.x) or MPC106 (Step 2.x) Registers
 * F9000000-F90000FF	NCR 53C810 Registers (Step 1.x?)
 * FEC00000-FEDFFFFF	MPC106 CONFIG_ADDR (Step 2.x)
 * FEE00000-FEFFFFFF	MPC106 CONFIG_DATA (Step 2.x)
 * FF000000-FF7FFFFF	Banked CROM (CROMxx)
 * FF800000-FFFFFFFF	Fixed CROM
 *
 * Endianness
 * ----------
 * We assume a little endian machine and so for speed, PowerPC RAM and ROM
 * regions are byte reversed, which means that aligned words can be read and
 * written without any conversion. Problems arise when the PowerPC accesses 
 * little endian devices, like the tile generator, MPC10x, or Real3D. Then, the
 * access must be carried out carefully one byte at a time or by manually byte
 * reversing first (because the PowerPC will already have byte reversed it).
 *
 * System Registers
 * ----------------
 *
 * F0100014: IRQ Enable
 *   7   6   5   4   3   2   1   0
 * +---+---+---+---+---+---+---+---+
 * | ? |SND| ? |NET|VD3|VD2|VBL|VD0|
 * +---+---+---+---+---+---+---+---+
 *		SND		SCSP (sound)
 *		NET		Network
 *		VD3		Unknown video-related
 *		VD2		Unknown video-related
 *		VBL		VBlank start
 *		VD0		Unknown video-related (?)
 *		0 = Disable, 1 = Enable
 *
 * Game Buttons
 * ------------
 *
 * For further information, see ReadInputs().
 *
 * Offset 0x04, bank 0:
 *
 *	  7   6   5   4   3   2   1   0
 *	+---+---+---+---+---+---+---+---+
 *  | ? | ? |ST2|ST1|SVA|TSA|CN2|CN1|
 *	+---+---+---+---+---+---+---+---+
 *		CNx		Coin 1, Coin 2
 *		TSA		Test Button A
 *		SVA		Service Button A
 *		STx		Start 1, Start 2
 *
 * Offset 0x04, bank 1:
 *
 *	  7   6   5   4   3   2   1   0
 *	+---+---+---+---+---+---+---+---+
 *  |TSB|SVB|EEP| ? | ? | ? | ? | ? |
 *	+---+---+---+---+---+---+---+---+
 *		EEP		Mapped to EEPROM (values written here are ignored)
 *		SVB		Service Button B
 *		TSB		Test Button B
 *
 * Offset 0x08:
 *
 *	  7   6   5   4   3   2   1   0
 *	+---+---+---+---+---+---+---+---+
 *  |G27|G26|G25|G24|G23|G22|G21|G20|
 *	+---+---+---+---+---+---+---+---+
 *		G2x		Game-specific
 *
 * Offset 0x0C:
 *
 *	  7   6   5   4   3   2   1   0
 *	+---+---+---+---+---+---+---+---+
 *  |G37|G36|G35|G34|G33|G32|G31|G30|
 *	+---+---+---+---+---+---+---+---+
 *		G3x		Game-specific
 *
 * Game-specific buttons:
 *
 *	Scud Race:
 *		G27		---
 *		G26		Shift 2 when combined w/ G25, Shift 1 when combined w/ G24
 *		G25		Shift 4
 *		G24		Shift 3
 *		G23		VR4 Green
 *		G22		VR3 Yellow 
 *		G21		VR2 Blue 
 *		G20		VR1 Red
 *
 *	Virtua Fighter 3, Fighting Vipers 2:
 *		G27		Right
 *		G26		Left
 *		G25		Down
 *		G24		Up
 *		G23		Punch
 *		G22		Kick
 *		G21		Guard
 *		G20		Escape (VF3 only)
 *
 *	Sega Rally 2:
 *		G21		Hand Brake
 *		G20		View Change
 *
 *	Lost World, LA Machineguns:
 *		G20		Gun trigger
 *
 *	Star Wars Trilogy:
 *		G20		Event button
 *		G25		Trigger
 *
 *	Virtual On 2:
 *		G27		Left Lever Left
 *		G26		Left Lever Right
 *		G25		Left Lever Up
 *		G24		Left Lever Down
 *		G23		---
 *		G22		---
 *		G21		Left Turbo 
 *		G20		Left Shot Trigger
 *		G37		Right Lever Left
 *		G36		Right Lever Right
 *		G35		Right Lever Up
 *		G34		Right Lever Down
 *		G33		---
 *		G32		---
 *		G31		Right Turbo 
 *		G30		Right Shot Trigger
 */

#include <new>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include "Supermodel.h"

/******************************************************************************
 Model 3 Inputs
 
 Game controls. The EEPROM is mapped here as well.
******************************************************************************/

UINT8 CModel3::ReadInputs(unsigned reg)
{
	UINT8	adc[8];
	UINT8	data;
	
	reg &= 0x3F;
	switch (reg)
	{
	case 0x00:	// input bank
		return inputBank;

	case 0x04:	// current input bank

		data = 0xFF;

		if ((inputBank&1) == 0)
		{
			data &= ~(Inputs->coin[0]->value);          // Coin 1
			data &= ~(Inputs->coin[1]->value<<1);       // Coin 2
			data &= ~(Inputs->test[0]->value<<2);	    // Test A
			data &= ~(Inputs->service[0]->value<<3);    // Service A
			data &= ~(Inputs->start[0]->value<<4);	    // Start 1
			data &= ~(Inputs->start[1]->value<<5);	    // Start 2
		}
		else
		{
			data &= ~(Inputs->service[1]->value<<6);    // Service B
			data &= ~(Inputs->test[1]->value<<7);	    // Test B
			data = (data&0xDF)|(EEPROM.Read()<<5);	// bank 1 contains EEPROM data bit
		}
		return data;

	case 0x08:	// game-specific inputs

		data = 0xFF;

		if ((Game->inputFlags&GAME_INPUT_JOYSTICK1))
		{
			data &= ~(Inputs->up[0]->value<<5);		        // P1 Up
			data &= ~(Inputs->down[0]->value<<4);	        // P1 Down
			data &= ~(Inputs->left[0]->value<<7);	        // P1 Left
			data &= ~(Inputs->right[0]->value<<6);	        // P1 Right
		}

		if ((Game->inputFlags&GAME_INPUT_FIGHTING))
		{
			data &= ~(Inputs->escape[0]->value<<3);	        // P1 Escape
			data &= ~(Inputs->guard[0]->value<<2);		    // P1 Guard
			data &= ~(Inputs->kick[0]->value<<1);		    // P1 Kick
			data &= ~(Inputs->punch[0]->value<<0);		    // P1 Punch
		}
		
		if ((Game->inputFlags&GAME_INPUT_SPIKEOUT))
		{
			data &= ~(Inputs->shift->value<<2);	        	// Shift
			data &= ~(Inputs->beat->value<<0);		    	// Beat
			data &= ~(Inputs->charge->value<<1);		    // Charge
			data &= ~(Inputs->jump->value<<3);		    	// Jump
		}
		
		if ((Game->inputFlags&GAME_INPUT_SOCCER))
		{
			data &= ~(Inputs->shortPass[0]->value<<2);	    // P1 Short Pass
			data &= ~(Inputs->longPass[0]->value<<0);	    // P1 Long Pass
			data &= ~(Inputs->shoot[0]->value<<1);		    // P1 Shoot
		}

		if ((Game->inputFlags&GAME_INPUT_VR))
		{
			data &= ~(Inputs->vr[0]->value<<0);		        // VR1 Red
			data &= ~(Inputs->vr[1]->value<<1);		        // VR2 Blue
			data &= ~(Inputs->vr[2]->value<<2);		        // VR3 Yellow
			data &= ~(Inputs->vr[3]->value<<3);		        // VR4 Green
		}
		
		if ((Game->inputFlags&GAME_INPUT_SHIFT4))
		{
			if (Inputs->gearShift4->value == 2)             // Shift 2
				data &= ~0x60;
			else if (Inputs->gearShift4->value == 4)        // Shift 4
				data &= ~0x20;
			if (Inputs->gearShift4->value == 1)			    // Shift 1
				data &= ~0x50;
			else if (Inputs->gearShift4->value == 3)	    // Shift 3
				data &= ~0x10;
		}
		
		if ((Game->inputFlags&GAME_INPUT_RALLY))
		{
			data &= ~(Inputs->viewChange->value<<0);	    // View change
			data &= ~(Inputs->handBrake->value<<1);	        // Hand brake
		}
		
		if ((Game->inputFlags&GAME_INPUT_GUN1))
			data &= ~(Inputs->trigger[0]->value<<0);		// P1 Trigger
		
		if ((Game->inputFlags&GAME_INPUT_ANALOG_JOYSTICK))
		{
			data &= ~(Inputs->analogJoyTrigger->value<<5);	// Trigger
			data &= ~(Inputs->analogJoyEvent->value<<0);	// Event Button
		}
		
		if ((Game->inputFlags&GAME_INPUT_TWIN_JOYSTICKS))	// First twin joystick
		{
			/*
			 * Process left joystick inputs first
			 */
			 
			// Shot trigger and Turbo
			data &= ~(Inputs->twinJoyShot1->value<<0);
			data &= ~(Inputs->twinJoyTurbo1->value<<1);
			
			// Stick
			data &= ~(Inputs->twinJoyLeft1->value<<7);
			data &= ~(Inputs->twinJoyRight1->value<<6);
			data &= ~(Inputs->twinJoyUp1->value<<5);
			data &= ~(Inputs->twinJoyDown1->value<<4);
			
			/*
			 * Next, process twin joystick macro inputs (higher level inputs
			 * that map to actions on both joysticks simultaneously).
			 */
			 
			/*
			 * Forward/reverse/turn are mutually exclusive.
			 *
			 * Turn Left: 	1D 2U
			 * Turn Right: 	1U 2D
			 * Forward: 	1U 2U
			 * Reverse: 	1D 2D
			 */
			if (Inputs->twinJoyTurnLeft->value)
				data &= ~0x10;
			else if (Inputs->twinJoyTurnRight->value)
				data &= ~0x20;
			else if (Inputs->twinJoyForward->value)
				data &= ~0x20;
			else if (Inputs->twinJoyReverse->value)
				data &= ~0x10;
				
			/*
			 * Strafe/crouch/jump are mutually exclusive.
			 *
			 * Strafe Left:		1L 2L
			 * Strafe Right: 	1R 2R
			 * Jump: 			1L 2R
			 * Crouch: 			1R 2L
			 */
			if (Inputs->twinJoyStrafeLeft->value)
				data &= ~0x80;
			else if (Inputs->twinJoyStrafeRight->value)
				data &= ~0x40;
			else if (Inputs->twinJoyJump->value)
				data &= ~0x80;
			else if (Inputs->twinJoyCrouch->value)
				data &= ~0x40;
		}

		return data;

	case 0x0C:	// game-specific inputs

		data = 0xFF;

		if (DriveBoard.IsAttached())
			data = DriveBoard.Read();
		
		if ((Game->inputFlags&GAME_INPUT_JOYSTICK2))
		{
			data &= ~(Inputs->up[1]->value<<5);		        // P2 Up
			data &= ~(Inputs->down[1]->value<<4);	        // P2 Down
			data &= ~(Inputs->left[1]->value<<7);	        // P2 Left
			data &= ~(Inputs->right[1]->value<<6);	        // P2 Right
		}

		if ((Game->inputFlags&GAME_INPUT_FIGHTING))
		{
			data &= ~(Inputs->escape[1]->value<<3);	        // P2 Escape
			data &= ~(Inputs->guard[1]->value<<2);		    // P2 Guard
			data &= ~(Inputs->kick[1]->value<<1);		    // P2 Kick
			data &= ~(Inputs->punch[1]->value<<0);		    // P2 Punch
		}
		
		if ((Game->inputFlags&GAME_INPUT_SOCCER))
		{
			data &= ~(Inputs->shortPass[1]->value<<2);	    // P2 Short Pass
			data &= ~(Inputs->longPass[1]->value<<0);	    // P2 Long Pass
			data &= ~(Inputs->shoot[1]->value<<1);		    // P2 Shoot
		}
		
		if ((Game->inputFlags&GAME_INPUT_TWIN_JOYSTICKS))	// Second twin joystick (see register 0x08 for comments)
		{
						
			data &= ~(Inputs->twinJoyShot2->value<<0);
			data &= ~(Inputs->twinJoyTurbo2->value<<1);
			
			data &= ~(Inputs->twinJoyLeft2->value<<7);
			data &= ~(Inputs->twinJoyRight2->value<<6);
			data &= ~(Inputs->twinJoyUp2->value<<5);
			data &= ~(Inputs->twinJoyDown2->value<<4);

			if (Inputs->twinJoyTurnLeft->value)
				data &= ~0x20;
			else if (Inputs->twinJoyTurnRight->value)
				data &= ~0x10;
			else if (Inputs->twinJoyForward->value)
				data &= ~0x20;
			else if (Inputs->twinJoyReverse->value)
				data &= ~0x10;
			
			if (Inputs->twinJoyStrafeLeft->value)
				data &= ~0x80;
			else if (Inputs->twinJoyStrafeRight->value)
				data &= ~0x40;
			else if (Inputs->twinJoyJump->value)
				data &= ~0x40;
			else if (Inputs->twinJoyCrouch->value)
				data &= ~0x80;
		}
		
		if ((Game->inputFlags&GAME_INPUT_GUN2))
			data &= ~(Inputs->trigger[1]->value<<0);        // P2 Trigger
		
		return data;

	case 0x2C:	// Serial FIFO 1
		return serialFIFO1;
		
	case 0x30:	// Serial FIFO 2
		return serialFIFO2;
		
	case 0x34:	// Serial FIFO full/empty flags
		return 0x0C;	
		
	case 0x3C:	// ADC

		// Load ADC channels with input data
		memset(adc, 0, sizeof(adc));
		if ((Game->inputFlags&GAME_INPUT_VEHICLE))
		{
			adc[0] = (UINT8)Inputs->steering->value;
			adc[1] = (UINT8)Inputs->accelerator->value;
			adc[2] = (UINT8)Inputs->brake->value;
		}
		if ((Game->inputFlags&GAME_INPUT_ANALOG_JOYSTICK))
		{
			adc[0] = (UINT8)Inputs->analogJoyY->value;
			adc[1] = (UINT8)Inputs->analogJoyX->value;
		}
			
		// Read out appropriate channel
		data = adc[adcChannel&7];
		++adcChannel;
		return data;
	
	default:
		break;
	}
	
	return 0xFF;	// controls are active low
}

void CModel3::WriteInputs(unsigned reg, UINT8 data)
{
	switch (reg&0x3F)
	{
	case 0:
		EEPROM.Write((data>>6)&1,(data>>7)&1,(data>>5)&1);
		inputBank = data;
		break;

	case 0x10:  // Drive board
		if (DriveBoard.IsAttached())
			DriveBoard.Write(data);
		break;

	case 0x24:	// Serial FIFO 1
		switch (data)	// Command
		{
		case 0x00:		// Light gun register select
			gunReg = serialFIFO2;
			break;
		case 0x87:		// Read light gun register
			serialFIFO1 = 0;	// clear serial FIFO 1
			serialFIFO2 = 0;
			if ((Game->inputFlags&GAME_INPUT_GUN1||Game->inputFlags&GAME_INPUT_GUN2))
			{
				switch (gunReg)
				{
				case 0:	// Player 1 gun Y (low 8 bits)
					serialFIFO2 = Inputs->gunY[0]->value&0xFF;
					break;
				case 1:	// Player 1 gun Y (high 2 bits)
					serialFIFO2 = (Inputs->gunY[0]->value>>8)&3;
					break;
				case 2:	// Player 1 gun X (low 8 bits)
					serialFIFO2 = Inputs->gunX[0]->value&0xFF;
					break;
				case 3:	// Player 1 gun X (high 2 bits)
					serialFIFO2 = (Inputs->gunX[0]->value>>8)&3;
					break;
				case 4:	// Player 2 gun Y (low 8 bits)
					serialFIFO2 = Inputs->gunY[1]->value&0xFF;
					break;
				case 5:	// Player 2 gun Y (high 2 bits)
					serialFIFO2 = (Inputs->gunY[1]->value>>8)&3;
					break;
				case 6:	// Player 2 gun X (low 8 bits)
					serialFIFO2 = Inputs->gunX[1]->value&0xFF;
					break;
				case 7:	// Player 2 gun X (high 2 bits)
					serialFIFO2 = (Inputs->gunX[1]->value>>8)&3;
					break;
				case 8:	// Off-screen indicator (bit 0 = player 1, bit 1 = player 2, set indicates off screen)
					serialFIFO2 = (Inputs->trigger[1]->offscreenValue<<1)|Inputs->trigger[0]->offscreenValue;
					break;
				default:
					DebugLog("Unknown gun register: %X\n", gunReg);
					break;
				}
			}
			break;
		default:
			DebugLog("Uknown command to serial FIFO: %02X\n", data);
			break;
		}
		break;
	case 0x28:	// Serial FIFO 2
		serialFIFO2 = data;
		break;
	case 0x3C:
		adcChannel = data&7;
		break;
	default:
		break;
	}
	//printf("Controls: %X=%02X\n", reg, data);
}


/******************************************************************************
 Model 3 Security Device
 
 The security device is present in some games. Virtual On and Dirt Devils read
 tile pattern data from it. Spikeout calls a routine at PC=0x6FAC8 that writes/
 reads the security device and, if the return value in R3 is 0, prints "ILLEGAL
 ROM" and locks the game. Our habit of returning all 1's for unknown reads 
 seems to help avoid this.
******************************************************************************/

static const UINT16 spikeoutSecurity[] =
{
	0x0000,
	0x4f4d, 0x4544, 0x2d4c, 0x2033, 0x7953, 0x7473, 0x6d65, 0x5020,
	0x6f72, 0x7267, 0x6d61, 0x4320, 0x706f, 0x7279, 0x6769, 0x7468,
	0x2820, 0x2943, 0x3120, 0x3939, 0x2035, 0x4553, 0x4147, 0x4520,
	0x746e, 0x7265, 0x7270, 0x7369, 0x7365, 0x4c2c, 0x4454, 0x202e,
	0x6c41, 0x206c, 0x6972, 0x6867, 0x2074, 0x6572, 0x6573, 0x7672,
	0x6465, 0x202e, 0x2020, 0x0020
};

static const UINT16 vs298Security[] =
{
	0x0000,	// dummy read
	0x4A20, 0x5041, 0x4E41, 0x4920, 0x4154, 0x594C, 0x4220, 0x4152, 0x4953, 0x204C,
	0x5241, 0x4547, 0x544E, 0x4E49, 0x2041, 0x4547, 0x4D52, 0x4E41, 0x2059, 0x4E45,
	0x4C47, 0x4E41, 0x2044, 0x454E, 0x4854, 0x5245, 0x414C, 0x444E, 0x2053, 0x5246,
	0x4E41, 0x4543, 0x4320, 0x4C4F, 0x4D4F, 0x4942, 0x2041, 0x4150, 0x4152, 0x5547,
	0x5941, 0x4220, 0x4C55, 0x4147, 0x4952, 0x2041, 0x5053, 0x4941, 0x204E, 0x5243,
	0x414F, 0x4954, 0x2041, 0x4542, 0x474C, 0x5549, 0x204D, 0x494E, 0x4547, 0x4952,
	0x2041, 0x4153, 0x4455, 0x2049, 0x4F4B, 0x4552, 0x2041, 0x4544, 0x4D4E, 0x5241,
	0x204B, 0x4F52, 0x414D, 0x494E, 0x2041, 0x4353, 0x544F, 0x414C, 0x444E, 0x5520,
	0x4153, 0x5320, 0x554F, 0x4854, 0x4641, 0x4952, 0x4143, 0x4D20, 0x5845, 0x4349,
	0x204F, 0x5559, 0x4F47, 0x4C53, 0x5641, 0x4149, 0x4620, 0x5F43, 0x4553, 0x4147
};

static const UINT16 ecaSecurity[] =
{
	0x0000,
    0x2d2f, 0x202d, 0x4d45, 0x5245, 0x4547, 0x434e, 0x2059, 0x4143,
    0x4c4c, 0x4120, 0x424d, 0x4c55, 0x4e41, 0x4543, 0x2d20, 0x0a2d,
    0x6f43, 0x7970, 0x6952, 0x6867, 0x2074, 0x4553, 0x4147, 0x4520,
    0x746e, 0x7265, 0x7270, 0x7369, 0x7365, 0x202c, 0x744c, 0x2e64,
    0x530a, 0x666f, 0x7774, 0x7261, 0x2065, 0x2652, 0x2044, 0x6544,
    0x7470, 0x202e, 0x3123, 0x660a, 0x726f, 0x7420, 0x7365, 0x0a74,
};

static const UINT16 oceanhunSecurity[57] =
{
    0x0000,    // dummy read

    0x3d3d, 0x203d, 0x434f, 0x4145, 0x204e, 0x5548, 0x544e, 0x5245,
    0x3d20, 0x3d3d, 0x430a, 0x706f, 0x5279, 0x6769, 0x7468, 0x5320,
    0x4745, 0x2041, 0x6e45, 0x6574, 0x7072, 0x6972, 0x6573, 0x2c73,
    0x4c20, 0x6474, 0x0a2e, 0x6d41, 0x7375, 0x6d65, 0x6e65, 0x2074,
    0x2652, 0x2044, 0x6544, 0x7470, 0x202e, 0x3123, 0x4b0a, 0x7a61,
    0x6e75, 0x7261, 0x2069, 0x7354, 0x6b75, 0x6d61, 0x746f, 0x206f,
    0x6553, 0x7463, 0x6f69, 0x206e, 0x614d, 0x616e, 0x6567, 0x0a72
};

static const UINT16 swtrilgySecurity[57] =
{
	0xffff,
	0x3d3d, 0x3d3d, 0x203d, 0x5453, 0x5241, 0x5720, 0x5241, 0x2053,
	0x3d3d, 0x3d3d, 0x0a3d, 0x6f43, 0x7970, 0x6952, 0x6867, 0x2074,
	0x4553, 0x4147, 0x4520, 0x746e, 0x7265, 0x7270, 0x7369, 0x7365,
	0x202c, 0x744c, 0x2e64, 0x410a, 0x756d, 0x6573, 0x656d, 0x746e,
	0x5220, 0x4426, 0x4420, 0x7065, 0x2e74, 0x2320, 0x3231, 0x4b0a,
	0x7461, 0x7573, 0x6179, 0x7573, 0x4120, 0x646e, 0x206f, 0x2026,
	0x614b, 0x6f79, 0x6f6b, 0x5920, 0x6d61, 0x6d61, 0x746f, 0x0a6f
};

static const UINT16 fvipers2Security[65] =
{
	0x2a2a,
	0x2a2a, 0x2a2a, 0x2a2a, 0x2a2a, 0x2a2a, 0x2a2a, 0x202a, 0x5b5b,
	0x4620, 0x6769, 0x7468, 0x6e69, 0x2067, 0x6956, 0x6570, 0x7372,
	0x3220, 0x5d20, 0x205d, 0x6e69, 0x3c20, 0x4d3c, 0x444f, 0x4c45,
	0x332d, 0x3e3e, 0x4320, 0x706f, 0x7279, 0x6769, 0x7468, 0x2820,
	0x2943, 0x3931, 0x3839, 0x5320, 0x4745, 0x2041, 0x6e45, 0x6574,
	0x7072, 0x6972, 0x6573, 0x2c73, 0x544c, 0x2e44, 0x2020, 0x4120,
	0x6c6c, 0x7220, 0x6769, 0x7468, 0x7220, 0x7365, 0x7265, 0x6576,
	0x2e64, 0x2a20, 0x2a2a, 0x2a2a, 0x2a2a, 0x2a2a, 0x2a2a, 0x2a2a
};

UINT32 CModel3::ReadSecurity(unsigned reg)
{
	UINT32	data;
	
	switch (reg)
	{
	case 0x00:	// Status
		return 0;
	case 0x1C:	// Data
		
		if (!strcmp(Game->id, "spikeout") || !strcmp(Game->id, "spikeofe"))
		{
			data = (spikeoutSecurity[securityPtr++] << 16);
			securityPtr %= (sizeof(spikeoutSecurity)/sizeof(UINT16));
		}
		else if (!strcmp(Game->id, "vs298"))
		{
			data = (vs298Security[securityPtr++] << 16);
			securityPtr %= (sizeof(vs298Security)/sizeof(UINT16));
		}
		else if (!strcmp(Game->id, "eca") || !strcmp(Game->id, "ecax"))
		{
			data = (ecaSecurity[securityPtr++] << 16);
			securityPtr %= (sizeof(ecaSecurity)/sizeof(UINT16));
		}
		else if (!strcmp(Game->id, "oceanhun"))
		{
			data = (oceanhunSecurity[securityPtr++] << 16);
			securityPtr %= (sizeof(oceanhunSecurity)/sizeof(UINT16));
		}
		else if (!strcmp(Game->id, "swtrilgy") || !strcmp(Game->id, "swtrilgya"))
		{
			data = (swtrilgySecurity[securityPtr++] << 16);
			securityPtr %= (sizeof(swtrilgySecurity)/sizeof(UINT16));
		}
		else if (!strcmp(Game->id, "fvipers2"))
		{
			data = (fvipers2Security[securityPtr++] << 16);
			securityPtr %= (sizeof(fvipers2Security)/sizeof(UINT16));
		}
		else if (!strcmp(Game->id, "von2"))
			data = 0xFFFFFFFF;
		else
		{
			data = 0xFFFFFFFF;
			DebugLog("Security read: reg=%X, PC=%08X, LR=%08X\n", reg, ppc_get_pc(), ppc_get_lr());
		}
		return data;
	default:
		DebugLog("Security read: reg=%X\n", reg);
		break;
	}
	
	return 0xFFFFFFFF;
}

void CModel3::WriteSecurity(unsigned reg, UINT32 data)
{
	DebugLog("Security write: reg=%X, data=%08X (PC=%08X, LR=%08X)\n", reg, data, ppc_get_pc(), ppc_get_lr());
}


/******************************************************************************
 PCI Devices
 
 Unknown PCI devices are handled here. 
******************************************************************************/

UINT32 CModel3::ReadPCIConfigSpace(unsigned device, unsigned reg, unsigned bits, unsigned offset)
{	
	if ((bits==8) || (bits==16))
	{
		DebugLog("Model 3: %d-bit PCI read request for reg=%02X\n", bits, reg);
		return 0;
	}
	
	switch (device)
	{
	case 16:	// Used by Daytona 2
		switch (reg)
		{
		case 0:	// PCI vendor and device ID
			return 0x182711DB;
		default:
			break;
		}
	default:
		break;
	}

	DebugLog("Model 3: PCI %d-bit write request for device=%d, reg=%02X\n", bits, device, reg);
	return 0;
}
	
void CModel3::WritePCIConfigSpace(unsigned device, unsigned reg, unsigned bits, unsigned offset, UINT32 data)
{
	DebugLog("Model 3: PCI %d-bit write request for device=%d, reg=%02X, data=%08X\n", bits, device, reg, data);
}


/******************************************************************************
 Model 3 System Registers
 
 NOTE: Proper IRQ handling requires a "deassert" function in the PowerPC core,
 which the interpreter presently lacks. This is because different modules that
 generate IRQs, like the tilegen, Real3D, and SCSP, should each call
 IRQ.Assert() on their own, which will assert the CPU IRQ line. Right now,
 the CPU processes an interrupt and clears the line by itself, which means that
 if multiple interrupts are asserted simultaneously, depending on the IRQ
 handler code, only one may be processed. Keep an eye on this!
******************************************************************************/

// Set the CROM bank index (active low logic)
void CModel3::SetCROMBank(unsigned idx)
{
	cromBankReg = idx;
	idx = (~idx) & 0xF;
	cromBank = &crom[0x800000 + (idx*0x800000)];
	DebugLog("CROM bank setting: %d (%02X), PC=%08X, LR=%08X\n", idx, cromBankReg, ppc_get_pc(), ppc_get_lr());
	//printf("CROM bank setting: %d (%02X), PC=%08X, LR=%08X\n", idx, cromBankReg, ppc_get_pc(), ppc_get_lr());
}

UINT8 CModel3::ReadSystemRegister(unsigned reg)
{
	switch (reg&0x3F)
	{
	case 0x08:	// CROM bank
		return cromBankReg;
	case 0x14:	// IRQ enable
		return IRQ.ReadIRQEnable();
	case 0x18:	// IRQ pending
		return IRQ.ReadIRQState();
	case 0x1C:	// unknown (apparently expects some or all bits set)
		//DebugLog("System register %02X read\n", reg);
		return 0xFF;
	case 0x10:	// JTAG Test Access Port
		return (GPU.ReadTAP()<< 5);
	default:		
		//DebugLog("System register %02X read\n", reg);
		break;
	}
	
	return 0xFF;
}

void CModel3::WriteSystemRegister(unsigned reg, UINT8 data)
{
	switch (reg&0x3F)
	{
	case 0x08:	// CROM bank
		SetCROMBank(data);
		break;
	case 0x14:	// IRQ enable
		IRQ.WriteIRQEnable(data);
		DebugLog("IRQ ENABLE=%02X\n", data);
		break;
	case 0x18:	// IRQ acknowledge
		DebugLog("IRQ SETTING! %02X=%02X\n", reg, data);
		break;
	case 0x0C:	// JTAG Test Access Port
		GPU.WriteTAP((data>>6)&1,(data>>2)&1,(data>>5)&1,(data>>7)&1);	// TCK, TMS, TDI, TRST
		break;
	case 0x0D:
	case 0x0E:
	case 0x0F:
	case 0x1C:	// LED control?
		break;
	default:
		//DebugLog("System register %02X=%02X\n", reg, data);
		break;
	}
}


/******************************************************************************
 Optimized Address Space Access Handlers
 
 Although I have not yet profiled the code, these ought to be more optimal,
 especially if the compiler can generate jump tables.

 NOTE: Testing of some of the address ranges is not strict enough, especially
 for the MPC10x. Write32() handles the MPC10x most correctly. For now, 
 accesses outside of the handled ranges have not been observed. Use the DEBUG
 version of these handlers for validation of new games.
******************************************************************************/

#ifndef DEBUG

/*
 * CModel3::Read8(addr):
 * CModel3::Read16(addr):
 * CModel3::Read32(addr):
 * CModel3::Read64(addr):
 *
 * Read handlers.
 */
UINT8 CModel3::Read8(UINT32 addr)
{
	// RAM (most frequently accessed)
	if (addr<0x00800000)
		return ram[addr^3];
	
	// Other
	switch ((addr>>24))
	{
	// CROM
	case 0xFF:
		if (addr < 0xFF800000)
			return cromBank[(addr&0x7FFFFF)^3];
		else
			return crom[(addr&0x7FFFFF)^3];

	// Real3D DMA
	case 0xC2:
		return GPU.ReadDMARegister8(addr&0xFF);

	// Various
	case 0xF0:
	case 0xFE:	// mirror
		
		switch ((addr>>16)&0xFF)
		{
		// Inputs
		case 0x04:
			return ReadInputs(addr&0x3F);
		
		// Sound Board
		case 0x08:
			if ((addr&0xF) == 4)	// MIDI control port
				return 0xFF;		// one of these bits (0x80?) indicates "ready"
			break;

		// System registers
		case 0x10:		
			return ReadSystemRegister(addr&0x3F);

		// RTC		
		case 0x14:
			if ((addr&3)==1)	// battery voltage test
				return 0x03;
			else if ((addr&3)==0)
				return RTC.ReadRegister((addr>>2)&0xF);
			return 0;

		// Unknown
		default:
			break;
		}

		break;

	// 53C810 SCSI
	case 0xC0:	// only on Step 1.0
		if (Game->step != 0x10)	// check for Step 1.0
			break;
	case 0xF9:
	case 0xC1:
		return SCSI.ReadRegister(addr&0xFF);

	// Unknown	
	default:
		break;
	}

	DebugLog("PC=%08X\tread8 : %08X\n", ppc_get_pc(), addr);
	return 0xFF;
}

UINT16 CModel3::Read16(UINT32 addr)
{
	UINT16	data;
	
	if ((addr&1))
	{
		data = 	Read8(addr+0)<<8;
		data |=	Read8(addr+1);
		return data;
	}

	// RAM (most frequently accessed)	
	if (addr<0x00800000)
		return *(UINT16 *) &ram[addr^2];

	// Other
	switch ((addr>>24))
	{
	// CROM
	case 0xFF:
		if (addr < 0xFF800000)
			return *(UINT16 *) &cromBank[(addr&0x7FFFFF)^2];
		else
			return *(UINT16 *) &crom[(addr&0x7FFFFF)^2];
	
	// Various
	case 0xF0:
	case 0xFE:	// mirror
		
		switch ((addr>>16)&0xFF)
		{
		// Backup RAM
		case 0x0C:
		case 0x0D:
			return *(UINT16 *) &backupRAM[(addr&0x1FFFF)^2];
			
		// Sound Board
		case 0x08:
			//printf("PPC: Read16 %08X\n", addr);
			break;

		// MPC105
		case 0xC0:	// F0C00CF8
			return PCIBridge.ReadPCIConfigData(16,addr&3);

		// MPC106
		case 0xE0:
		case 0xE1:
		case 0xE2:
		case 0xE3:
		case 0xE4:
		case 0xE5:
		case 0xE6:
		case 0xE7:
		case 0xE8:
		case 0xE9:
		case 0xEA:
		case 0xEB:
		case 0xEC:
		case 0xED:
		case 0xEE:
		case 0xEF:
			return PCIBridge.ReadPCIConfigData(16,addr&3);

		// Unknown
		default:
			break;
		}

		break;

	// Unknown
	default:
		break;
	}

	DebugLog("PC=%08X\tread16: %08X\n", ppc_get_pc(), addr);
	return 0xFFFF;
}

UINT32 CModel3::Read32(UINT32 addr)
{
	UINT32	data;

	if ((addr&3))
	{
		data = 	Read16(addr+0)<<16;
		data |=	Read16(addr+2);
		return data;
	}

	// RAM (most frequently accessed)
	if (addr < 0x00800000)
		return *(UINT32 *) &ram[addr];

	// Other
	switch ((addr>>24))
	{
	// CROM
	case 0xFF:		
		if (addr < 0xFF800000)
			return *(UINT32 *) &cromBank[(addr&0x7FFFFF)];
		else
			return *(UINT32 *) &crom[(addr&0x7FFFFF)];

	// Real3D registers
	case 0x84:
		return GPU.ReadRegister(addr&0x3F);
		
	// Real3D DMA
	case 0xC2:
		data = GPU.ReadDMARegister32(addr&0xFF);
		return FLIPENDIAN32(data);

	// Various
	case 0xF0:
	case 0xFE:	// mirror
		
		switch ((addr>>16)&0xFF)
		{
		// Inputs		
		case 0x04:
			data = 	ReadInputs((addr&0x3F)+0) << 24;
			data |=	ReadInputs((addr&0x3F)+1) << 16;
			data |=	ReadInputs((addr&0x3F)+2) << 8;
			data |=	ReadInputs((addr&0x3F)+3) << 0;
			return data;
		
		// Sound Board
		case 0x08:
			//printf("PPC: Read32 %08X\n", addr);
			break;

		// Backup RAM
		case 0x0C:
		case 0x0D:
			return *(UINT32 *) &backupRAM[(addr&0x1FFFF)];

		// System registers
		case 0x10:
			data = 	ReadSystemRegister((addr&0x3F)+0) << 24;
			data |=	ReadSystemRegister((addr&0x3F)+1) << 16;
			data |=	ReadSystemRegister((addr&0x3F)+2) << 8;
			data |=	ReadSystemRegister((addr&0x3F)+3) << 0;
			return data;

		// MPC105
		case 0xC0:	// F0C00CF8
			return PCIBridge.ReadPCIConfigData(32,0);

		// MPC106
		case 0xE0:
		case 0xE1:
		case 0xE2:
		case 0xE3:
		case 0xE4:
		case 0xE5:
		case 0xE6:
		case 0xE7:
		case 0xE8:
		case 0xE9:
		case 0xEA:
		case 0xEB:
		case 0xEC:
		case 0xED:
		case 0xEE:
		case 0xEF:
			return PCIBridge.ReadPCIConfigData(32,0);

		// RTC
		case 0x14:
			data = (RTC.ReadRegister((addr>>2)&0xF) << 24);
			data |= 0x00030000;	// set these bits to pass battery voltage test
			return data;

		// Security board RAM
		case 0x18:
		case 0x19:
			return *(UINT32 *) &securityRAM[(addr&0x1FFFF)];	// so far, only 32-bit access observed, so we use little endian access

		// Security board registers
		case 0x1A:
			return ReadSecurity(addr&0x3F);
		
		// Unknown
		default:
			break;
		}

		break;

	// Tile generator
	case 0xF1:
		if (addr==0xF1180000)	// fixes 2D graphics (TO-DO: integrate register reads into TileGen.cpp)
			return 0;		

		// Tile generator accesses its RAM as little endian, must flip for big endian PowerPC
		if (addr < 0xF1120000)
		{
			data = TileGen.ReadRAM(addr&0x1FFFFF);
			return FLIPENDIAN32(data);
		}

		break;

	// 53C810 SCSI
	case 0xC0:	// only on Step 1.0
		if (Game->step != 0x10)	// check for Step 1.0
			break;
	case 0xF9:
	case 0xC1:
		data = 	(SCSI.ReadRegister((addr+0)&0xFF) << 24);
		data |=	(SCSI.ReadRegister((addr+1)&0xFF) << 16);
		data |=	(SCSI.ReadRegister((addr+2)&0xFF) << 8);
		data |=	(SCSI.ReadRegister((addr+3)&0xFF) << 0);
		return data;

	// Unknown
	default:
		break;
	}

	DebugLog("PC=%08X\tread32: %08X\n", ppc_get_pc(), addr);
	return 0xFFFFFFFF;
}

UINT64 CModel3::Read64(UINT32 addr)
{
    UINT64  data;

	data = Read32(addr+0);
	data <<= 32;
	data |= Read32(addr+4);

	return data;
}

/*
 * CModel3::Write8(addr, data):
 * CModel3::Write16(addr, data):
 * CModel3::Write32(addr, data):
 * CModel3::Write64(addr, data):
 *
 * Write handlers.
 */
void CModel3::Write8(UINT32 addr, UINT8 data)
{
	// RAM (most frequently accessed)
	if (addr < 0x00800000)
	{
		ram[addr^3] = data;
		return;
	}

	// Other
	switch ((addr>>24))
	{
	// Real3D DMA
	case 0xC2:
		GPU.WriteDMARegister8(addr&0xFF,data);
		break;

	// Various
	case 0xF0:
	case 0xFE:	// mirror
		
		switch ((addr>>16)&0xFF)
		{
		// Inputs		
		case 0x04:	
			WriteInputs(addr&0x3F,data);
			break;
			
		// Sound Board
		case 0x08:
			//printf("PPC: %08X=%02X * (PC=%08X, LR=%08X)\n", addr, data, ppc_get_pc(), ppc_get_lr());
			if ((addr&0xF) == 0)		// MIDI data port
				SoundBoard.WriteMIDIPort(data);
			else if ((addr&0xF) == 4)	// MIDI control port
				midiCtrlPort = data;
			break;
	
		// Backup RAM
		case 0x0C:
		case 0x0D:
			backupRAM[(addr&0x1FFFF)^3] = data;
			break;

		// System registers
		case 0x10:
			WriteSystemRegister(addr&0x3F,data);
			break;

		// RTC
		case 0x14:
			if ((addr&3)==0)
				RTC.WriteRegister((addr>>2)&0xF,data);
			break;
		
		// Unknown
		default:
			break;
		}

		DebugLog("PC=%08X\twrite8 : %08X=%02X\n", ppc_get_pc(), addr, data);		
		break;

	// MPC105/106
	case 0xF8:
		PCIBridge.WriteRegister(addr&0xFF,data);
		break;

	// 53C810 SCSI
	case 0xC0:	// only on Step 1.0
		if (Game->step != 0x10)
			goto Unknown8;
	case 0xF9:
	case 0xC1:
		SCSI.WriteRegister(addr&0xFF,data);
		break;

	// Unknown:
	default:
	Unknown8:
		DebugLog("PC=%08X\twrite8 : %08X=%02X\n", ppc_get_pc(), addr, data);
		break;
	}
}

void CModel3::Write16(UINT32 addr, UINT16 data)
{
	if ((addr&1))
	{
		Write8(addr+0,data>>8);
		Write8(addr+1,data&0xFF);
		return;
	}

	// RAM (most frequently accessed)
	if (addr < 0x00800000)
	{
		*(UINT16 *) &ram[addr^2] = data;
		return;
	}

	// Other
	switch ((addr>>24))
	{
	// Various
	case 0xF0:
	case 0xFE:	// mirror
		
		switch ((addr>>16)&0xFF)
		{
		// Sound Board
		case 0x08:
			//printf("%08X=%04X\n", addr, data);
			break;
			
		// Backup RAM
		case 0x0C:
		case 0x0D:
			*(UINT16 *) &backupRAM[(addr&0x1FFFF)^2] = data;
			break;
		
		// MPC105
		case 0xC0:	// F0C00CF8
			PCIBridge.WritePCIConfigData(16,addr&2,data);
			break;

		// Unknown
		default:
			break;
		}

		DebugLog("PC=%08X\twrite16 : %08X=%04X\n", ppc_get_pc(), addr, data);		
		break;

	// MPC105/106
	case 0xF8:
		// Write in big endian order, like a real PowerPC
		PCIBridge.WriteRegister((addr&0xFF)+0,data>>8);
		PCIBridge.WriteRegister((addr&0xFF)+1,data&0xFF);
		break;

	// Unknown
	default:
		DebugLog("PC=%08X\twrite16: %08X=%04X\n", ppc_get_pc(), addr, data);
		break;
	}
}	

void CModel3::Write32(UINT32 addr, UINT32 data)
{
	if ((addr&3))
	{
		Write16(addr+0,data>>16);
		Write16(addr+2,data);
		return;
	}

	// RAM (most frequently accessed)
	if (addr<0x00800000)
	{
		*(UINT32 *) &ram[addr] = data;
		return;
	}

	// Other
	switch ((addr>>24))
	{
	// Real3D trigger
	case 0x88:	// 88000000
		GPU.Flush();
		break;
	
	// Real3D low culling RAM
	case 0x8C:	// 8C000000-8C400000
		GPU.WriteLowCullingRAM(addr&0x3FFFFF,FLIPENDIAN32(data));
		break;

	// Real3D high culling RAM
	case 0x8E:	// 8E000000-8E100000
		GPU.WriteHighCullingRAM(addr&0xFFFFF,FLIPENDIAN32(data));
		break;

	// Real3D texture port
	case 0x90:	// 90000000-90000018
		GPU.WriteTexturePort(addr&0xFF,FLIPENDIAN32(data));
		break;

	// Real3D texture FIFO
	case 0x94:	// 94000000-94100000
		GPU.WriteTextureFIFO(FLIPENDIAN32(data));
		break;

	// Real3D polygon RAM
	case 0x98:	// 98000000-98400000
		GPU.WritePolygonRAM(addr&0x3FFFFF,FLIPENDIAN32(data));
		break;

	// Real3D DMA
	case 0xC2:	// C2000000-C2000100
		GPU.WriteDMARegister32(addr&0xFF,FLIPENDIAN32(data));
		break;

	// Various
	case 0xF0:
	case 0xFE:	// mirror
		
		switch ((addr>>16)&0xFF)
		{
		// Inputs		
		case 0x04:
			WriteInputs((addr&0x3F)+0,(data>>24)&0xFF);
			WriteInputs((addr&0x3F)+1,(data>>16)&0xFF);
			WriteInputs((addr&0x3F)+2,(data>>8)&0xFF);
			WriteInputs((addr&0x3F)+3,(data>>0)&0xFF);
			break;

		// Sound Board
		case 0x08:
			//printf("PPC: %08X=%08X\n", addr, data);
			break;
			
		// Backup RAM
		case 0x0C:
		case 0x0D:
			*(UINT32 *) &backupRAM[(addr&0x1FFFF)] = data;
			break;

		// MPC105
		case 0x80:	// F0800CF8 (never observed at 0xFExxxxxx)
			PCIBridge.WritePCIConfigAddress(data);
			break;

		// MPC105/106
		case 0xC0: case 0xD0: case 0xE0: 
		case 0xC1: case 0xD1: case 0xE1: 
		case 0xC2: case 0xD2: case 0xE2: 
		case 0xC3: case 0xD3: case 0xE3: 
		case 0xC4: case 0xD4: case 0xE4: 
		case 0xC5: case 0xD5: case 0xE5: 
		case 0xC6: case 0xD6: case 0xE6: 
		case 0xC7: case 0xD7: case 0xE7: 
		case 0xC8: case 0xD8: case 0xE8: 
		case 0xC9: case 0xD9: case 0xE9: 
		case 0xCA: case 0xDA: case 0xEA: 
		case 0xCB: case 0xDB: case 0xEB: 
		case 0xCC: case 0xDC: case 0xEC: 
		case 0xCD: case 0xDD: case 0xED: 
		case 0xCE: case 0xDE: case 0xEE: 
		case 0xCF: case 0xDF: case 0xEF: 
			if ((addr>=0xF0C00CF8) && (addr<0xF0C00D00))		// MPC105
				PCIBridge.WritePCIConfigData(32,0,data);
			else if ((addr>=0xFEC00000) && (addr<0xFEE00000))	// MPC106
				PCIBridge.WritePCIConfigAddress(data);
			else if ((addr>=0xFEE00000) && (addr<0xFEF00000))	// MPC106
				PCIBridge.WritePCIConfigData(32,0,data);
			break;

		// System registers
		case 0x10:
			WriteSystemRegister((addr&0x3F)+0,(data>>24)&0xFF);
			WriteSystemRegister((addr&0x3F)+1,(data>>16)&0xFF);
			WriteSystemRegister((addr&0x3F)+2,(data>>8)&0xFF);
			WriteSystemRegister((addr&0x3F)+3,(data>>0)&0xFF);
			break;
		
		// RTC
		case 0x14:
			RTC.WriteRegister((addr>>2)&0xF,data);
			break;

		// Security board RAM
		case 0x18:
			*(UINT32 *) &securityRAM[(addr&0x1FFFF)] = data;
			break;

		// Security board registers
		case 0x1A:
			WriteSecurity(addr&0x3F,data);
			break;
		
		// Unknown
		default:
			break;
		}

		DebugLog("PC=%08X\twrite32: %08X=%08X\n", ppc_get_pc(), addr, data);		
		break;

	// Tile generator
	case 0xF1:
		if (addr < 0xF1120000)
		{
			// Tile generator accesses its RAM as little endian, must flip for big endian PowerPC
			data = FLIPENDIAN32(data);
			TileGen.WriteRAM(addr&0x1FFFFF,data);
			break;
		}
		else if ((addr>=0xF1180000) && (addr<0xF1180100))
		{
			TileGen.WriteRegister(addr&0xFF,FLIPENDIAN32(data));
			break;
		}

		goto Unknown32;

	// MPC105/106
	case 0xF8:	// F8FFF000-F8FFF100
		// Write in big endian order, like a real PowerPC
		PCIBridge.WriteRegister((addr&0xFF)+0,(data>>24)&0xFF);
		PCIBridge.WriteRegister((addr&0xFF)+1,(data>>16)&0xFF);
		PCIBridge.WriteRegister((addr&0xFF)+2,(data>>8)&0xFF);
		PCIBridge.WriteRegister((addr&0xFF)+3,data&0xFF);
		break;

	// 53C810 SCSI
	case 0xC0:	// step 1.0 only
		if (Game->step != 0x10)
			goto Unknown32;
	case 0xF9:
	case 0xC1:
		SCSI.WriteRegister((addr&0xFF)+0,(data>>24)&0xFF);
		SCSI.WriteRegister((addr&0xFF)+1,(data>>16)&0xFF);
		SCSI.WriteRegister((addr&0xFF)+2,(data>>8)&0xFF);
		SCSI.WriteRegister((addr&0xFF)+3,data&0xFF);
		break;

	// Unknown
	default:
	Unknown32:
		DebugLog("PC=%08X\twrite32: %08X=%08X\n", ppc_get_pc(), addr, data);
		break;
	}
}

void CModel3::Write64(UINT32 addr, UINT64 data)
{
    Write32(addr+0, (UINT32) (data>>32));
    Write32(addr+4, (UINT32) data);
}

#endif


/******************************************************************************
 Debug Mode (Strict) Address Space Access Handlers
 
 Enabled only if DEBUG is defined. These perform stricter checks than the
 "optimized" handlers but may be slower.
******************************************************************************/

#ifdef DEBUG

/*
 * CModel3::Read8(addr):
 * CModel3::Read16(addr):
 * CModel3::Read32(addr):
 * CModel3::Read64(addr):
 *
 * Read handlers.
 */
UINT8 CModel3::Read8(UINT32 addr)
{
	if (addr<0x00800000)
		return ram[addr^3];
	else if ((addr>=0xFF000000) && (addr<0xFF800000))
		return cromBank[(addr&0x7FFFFF)^3];
	else if (addr>=0xFF800000)
		return crom[(addr&0x7FFFFF)^3];
	else if ((addr>=0xC2000000) && (addr<0xC2000100))
		return GPU.ReadDMARegister8(addr&0xFF);
	else if (((addr>=0xF0040000) && (addr<0xF0040040)) || ((addr>=0xFE040000) && (addr<0xFE040040)))
		return ReadInputs(addr&0x3F);
	else if (((addr>=0xF0080000) && (addr<=0xF0080007)) || ((addr>=0xFE080000) && (addr<=0xFE080007)))
	{
		if ((addr&0xF) == 4)	// MIDI control port
			return 0xFF;
	}
	else if (((addr>=0xF00C0000) && (addr<0xF00DFFFF)) || ((addr>=0xFE0C0000) && (addr<0xFE0DFFFF)))
		return backupRAM[(addr&0x1FFFF)^3];
	else if (((addr>=0xF0100000) && (addr<0xF0100040)) || ((addr>=0xFE100000) && (addr<0xFE100040)))
		return ReadSystemRegister(addr&0x3F);
	else if (((addr>=0xF0140000) && (addr<0xF0140040)) || ((addr>=0xFE140000) && (addr<0xFE140040)))
	{
		if ((addr&3)==1)	// battery voltage test
			return 0x03;
		else if ((addr&3)==0)
			return RTC.ReadRegister((addr>>2)&0xF);
		return 0;
	}
	else if (((addr>=0xF9000000) && (addr<0xF9000100)) || ((addr>=0xC1000000) && (addr<0xC1000100)) || ((Game->step==0x10) && ((addr>=0xC0000000) && (addr<0xC0000100))))
		return SCSI.ReadRegister(addr&0xFF);
		
	DebugLog("PC=%08X\tread8 : %08X\n", ppc_get_pc(), addr);
	return 0xFF;
}

UINT16 CModel3::Read16(UINT32 addr)
{
	UINT16	data;
	
	if ((addr&1))
	{
		data = 	Read8(addr+0)<<8;
		data |=	Read8(addr+1);
		return data;
	}
		
	if (addr<0x00800000)
		return *(UINT16 *) &ram[addr^2];
	else if ((addr>=0xFF000000) && (addr<0xFF800000))
		return *(UINT16 *) &cromBank[(addr&0x7FFFFF)^2];
	else if (addr>=0xFF800000)
		return *(UINT16 *) &crom[(addr&0x7FFFFF)^2];
	else if (((addr>=0xF00C0000) && (addr<0xF00DFFFF)) || ((addr>=0xFE0C0000) && (addr<0xFE0DFFFF)))
		return *(UINT16 *) &backupRAM[(addr&0x1FFFF)^2];
	else if ((addr>=0xF0C00CF8) && (addr<0xF0C00D00))	// MPC105
		return PCIBridge.ReadPCIConfigData(16,addr&3);
	else if ((addr>=0xFEE00000) && (addr<0xFEF00000))	// MPC106
		return PCIBridge.ReadPCIConfigData(16,addr&3);

	DebugLog("PC=%08X\tread16: %08X\n", ppc_get_pc(), addr);
	return 0xFFFF;
}

UINT32 CModel3::Read32(UINT32 addr)
{
	UINT32	data;
			
	if ((addr&3))
	{
		data = 	Read16(addr+0)<<16;
		data |=	Read16(addr+2);
		return data;
	}

	if (addr<0x00800000)
		return *(UINT32 *) &ram[addr];
	else if ((addr>=0xFF000000) && (addr<0xFF800000))
		return *(UINT32 *) &cromBank[(addr&0x7FFFFF)];
	else if (addr>=0xFF800000)
		return *(UINT32 *) &crom[(addr&0x7FFFFF)];
	else if ((addr>=0x84000000) && (addr<0x8400003F))
		return GPU.ReadRegister(addr&0x3F);
	else if ((addr>=0xC2000000) && (addr<0xC2000100))
	{
		data = GPU.ReadDMARegister32(addr&0xFF);
		return FLIPENDIAN32(data);
	}
	else if (((addr>=0xF0040000) && (addr<0xF0040040)) || ((addr>=0xFE040000) && (addr<0xFE040040)))
	{
		data = 	ReadInputs((addr&0x3F)+0) << 24;
		data |=	ReadInputs((addr&0x3F)+1) << 16;
		data |=	ReadInputs((addr&0x3F)+2) << 8;
		data |=	ReadInputs((addr&0x3F)+3) << 0;
		return data;
	}
	else if (((addr>=0xF00C0000) && (addr<0xF00DFFFF)) || ((addr>=0xFE0C0000) && (addr<0xFE0DFFFF)))
		return *(UINT32 *) &backupRAM[(addr&0x1FFFF)];
	else if (((addr>=0xF0100000) && (addr<0xF0100040)) || ((addr>=0xFE100000) && (addr<0xFE100040)))
	{
		data = 	ReadSystemRegister((addr&0x3F)+0) << 24;
		data |=	ReadSystemRegister((addr&0x3F)+1) << 16;
		data |=	ReadSystemRegister((addr&0x3F)+2) << 8;
		data |=	ReadSystemRegister((addr&0x3F)+3) << 0;
		return data;
	}
	else if ((addr>=0xF0C00CF8) && (addr<0xF0C00D00))	// MPC105
		return PCIBridge.ReadPCIConfigData(32,0);
	else if ((addr>=0xFEE00000) && (addr<0xFEF00000))	// MPC106
		return PCIBridge.ReadPCIConfigData(32,0);
	else if (((addr>=0xF0140000) && (addr<0xF0140040)) || ((addr>=0xFE140000) && (addr<0xFE140040)))
	{
		data = (RTC.ReadRegister((addr>>2)&0xF) << 24);
		data |= 0x00030000;	// set these bits to pass battery voltage test
		return data;
	}
	else if (((addr>=0xF0180000) && (addr<0xF019FFFF)) || ((addr>=0xFE180000) && (addr<0xFE19FFFF)))
		return *(UINT32 *) &securityRAM[(addr&0x1FFFF)];	// so far, only 32-bit access observed, so we use little endian access
	else if (((addr>=0xF01A0000) && (addr<0xF01A003F)) || ((addr>=0xFE1A0000) && (addr<0xFE1A003F)))
		return ReadSecurity(addr&0x3F);
	else if ((addr>=0xF1000000) && (addr<0xF1120000))
	{
		// Tile generator accesses its RAM as little endian, must flip for big endian PowerPC
		data = TileGen.ReadRAM(addr&0x1FFFFF);
		return FLIPENDIAN32(data);
	}
	else if (((addr>=0xF9000000) && (addr<0xF9000100)) || ((addr>=0xC1000000) && (addr<0xC1000100)) || ((Game->step==0x10) && ((addr>=0xC0000000) && (addr<0xC0000100))))
	{
		data = 	(SCSI.ReadRegister((addr+0)&0xFF) << 24);
		data |=	(SCSI.ReadRegister((addr+1)&0xFF) << 16);
		data |=	(SCSI.ReadRegister((addr+2)&0xFF) << 8);
		data |=	(SCSI.ReadRegister((addr+3)&0xFF) << 0);
		return data;
	}
	
	// FIXES 2D GRAPHICS (to-do: integrate this into tilegen.cpp)
	if (addr==0xF1180000)
		return 0;
		
	DebugLog("PC=%08X\tread32: %08X\n", ppc_get_pc(), addr);
	return 0xFFFFFFFF;
}

UINT64 CModel3::Read64(UINT32 addr)
{
    UINT64  data;

	data = Read32(addr+0);
	data <<= 32;
	data |= Read32(addr+4);

	return data;
}

/*
 * CModel3::Write8(addr, data):
 * CModel3::Write16(addr, data):
 * CModel3::Write32(addr, data):
 * CModel3::Write64(addr, data):
 *
 * Write handlers.
 */
void CModel3::Write8(UINT32 addr, UINT8 data)
{		
	if (addr<0x00800000)
		ram[addr^3] = data;
	else if ((addr>=0xC2000000) && (addr<0xC2000100))
		GPU.WriteDMARegister8(addr&0xFF,data);
	else if (((addr>=0xF0040000) && (addr<0xF0040040)) || ((addr>=0xFE040000) && (addr<0xFE040040)))
		WriteInputs(addr&0x3F,data);
	else if (((addr>=0xF0080000) && (addr<=0xF0080007)) || ((addr>=0xFE080000) && (addr<=0xFE080007)))
	{
			if ((addr&0xF) == 0)		// MIDI data port
				SoundBoard.WriteMIDIPort(data);
			else if ((addr&0xF) == 4)	// MIDI control port
				midiCtrlPort = data;
	}
	else if (((addr>=0xF00C0000) && (addr<0xF00E0000)) || ((addr>=0xFE0C0000) && (addr<0xFE0E0000)))
		backupRAM[(addr&0x1FFFF)^3] = data;
	else if (((addr>=0xF0100000) && (addr<0xF0100040)) || ((addr>=0xFE100000) && (addr<0xFE100040)))
		WriteSystemRegister(addr&0x3F,data);
	else if (((addr>=0xF0140000) && (addr<0xF0140040)) || ((addr>=0xFE140000) && (addr<0xFE140040)))
	{
		if ((addr&3)==0)
			RTC.WriteRegister((addr>>2)&0xF,data);
	}
	else if ((addr>=0xF8FFF000) && (addr<0xF8FFF100))
		PCIBridge.WriteRegister(addr&0xFF,data);
	else if (((addr>=0xF9000000) && (addr<0xF9000100)) || ((addr>=0xC1000000) && (addr<0xC1000100)) || ((Game->step==0x10) && ((addr>=0xC0000000) && (addr<0xC0000100))))
		SCSI.WriteRegister(addr&0xFF,data);
	else
	{
		DebugLog("PC=%08X\twrite8 : %08X=%02X\n", ppc_get_pc(), addr, data);
		//printf("PC=%08X\twrite8 : %08X=%02X\n", ppc_get_pc(), addr, data);
	}
}

void CModel3::Write16(UINT32 addr, UINT16 data)
{
	if ((addr&1))
	{
		Write8(addr+0,data>>8);
		Write8(addr+1,data&0xFF);
		return;
	}
	
	if (addr<0x00800000)
		*(UINT16 *) &ram[addr^2] = data;
	else if (((addr>=0xF00C0000) && (addr<0xF00E0000)) || ((addr>=0xFE0C0000) && (addr<0xFE0E0000)))
		*(UINT16 *) &backupRAM[(addr&0x1FFFF)^2] = data;
	else if ((addr>=0xF0C00CF8) && (addr<0xF0C00D00))
		PCIBridge.WritePCIConfigData(16,addr&2,data);
	else if ((addr>=0xF8FFF000) && (addr<0xF8FFF100))
	{
		// Write in big endian order, like a real PowerPC
		PCIBridge.WriteRegister((addr&0xFF)+0,data>>8);
		PCIBridge.WriteRegister((addr&0xFF)+1,data&0xFF);
	}
	else
	{
		DebugLog("PC=%08X\twrite16: %08X=%04X\n", ppc_get_pc(), addr, data);
		//printf("PC=%08X\twrite16: %08X=%04X\n", ppc_get_pc(), addr, data);
	}
}

void CModel3::Write32(UINT32 addr, UINT32 data)
{		
	if ((addr&3))
	{
		Write16(addr+0,data>>16);
		Write16(addr+2,data);
		return;
	}

	if (addr<0x00800000)
		*(UINT32 *) &ram[addr] = data;
	else if ((addr>=0x88000000) && (addr<0x88000008))
		GPU.Flush();
	else if ((addr>=0x8C000000) && (addr<0x8C400000))
		GPU.WriteLowCullingRAM(addr&0x3FFFFF,FLIPENDIAN32(data));
	else if ((addr>=0x8E000000) && (addr<0x8E100000))
		GPU.WriteHighCullingRAM(addr&0xFFFFF,FLIPENDIAN32(data));
	else if ((addr>=0x90000000) && (addr<0x90000018))
		GPU.WriteTexturePort(addr&0xFF,FLIPENDIAN32(data));
	else if ((addr>=0x94000000) && (addr<0x94100000))
		GPU.WriteTextureFIFO(FLIPENDIAN32(data));
	else if ((addr>=0x98000000) && (addr<0x98400000))
		GPU.WritePolygonRAM(addr&0x3FFFFF,FLIPENDIAN32(data));
	else if ((addr>=0xC2000000) && (addr<0xC2000100))
		GPU.WriteDMARegister32(addr&0xFF,FLIPENDIAN32(data));
	else if (((addr>=0xF0040000) && (addr<0xF0040040)) || ((addr>=0xFE040000) && (addr<0xFE040040)))
	{
		WriteInputs((addr&0x3F)+0,(data>>24)&0xFF);
		WriteInputs((addr&0x3F)+1,(data>>16)&0xFF);
		WriteInputs((addr&0x3F)+2,(data>>8)&0xFF);
		WriteInputs((addr&0x3F)+3,(data>>0)&0xFF);
	}
	else if (((addr>=0xF00C0000) && (addr<0xF00E0000)) || ((addr>=0xFE0C0000) && (addr<0xFE0E0000)))
		*(UINT32 *) &backupRAM[(addr&0x1FFFF)] = data;
	else if ((addr>=0xF0800CF8) && (addr<0xF0800D00))	// MPC105
		PCIBridge.WritePCIConfigAddress(data);
	else if ((addr>=0xF0C00CF8) && (addr<0xF0C00D00))	// MPC105
		PCIBridge.WritePCIConfigData(32,0,data);
	else if ((addr>=0xFEC00000) && (addr<0xFEE00000))	// MPC106
		PCIBridge.WritePCIConfigAddress(data);
	else if ((addr>=0xFEE00000) && (addr<0xFEF00000))	// MPC106
		PCIBridge.WritePCIConfigData(32,0,data);
	else if (((addr>=0xF0100000) && (addr<0xF0100040)) || ((addr>=0xFE100000) && (addr<0xFE100040)))
	{
		WriteSystemRegister((addr&0x3F)+0,(data>>24)&0xFF);
		WriteSystemRegister((addr&0x3F)+1,(data>>16)&0xFF);
		WriteSystemRegister((addr&0x3F)+2,(data>>8)&0xFF);
		WriteSystemRegister((addr&0x3F)+3,(data>>0)&0xFF);
	}
	else if (((addr>=0xF0140000) && (addr<0xF0140040)) || ((addr>=0xFE140000) && (addr<0xFE140040)))
		RTC.WriteRegister((addr>>2)&0xF,data);
	else if (((addr>=0xF0180000) && (addr<0xF019FFFF)) || ((addr>=0xFE180000) && (addr<0xFE19FFFF)))
		*(UINT32 *) &securityRAM[(addr&0x1FFFF)] = data;	// so far, only 32-bit access observed, so just store little endian
	else if (((addr>=0xF01A0000) && (addr<0xF01A003F)) || ((addr>=0xFE1A0000) && (addr<0xFE1A003F)))
		WriteSecurity(addr&0x3F,data);
	else if ((addr>=0xF1000000) && (addr<0xF1120000))
	{
		// Tile generator accesses its RAM as little endian, must flip for big endian PowerPC
		data = FLIPENDIAN32(data);
		TileGen.WriteRAM(addr&0x1FFFFF,data);
	}
	else if ((addr>=0xF1180000) && (addr<0xF1180100))
		TileGen.WriteRegister(addr&0xFF,FLIPENDIAN32(data));
	else if ((addr>=0xF8FFF000) && (addr<0xF8FFF100))
	{
		// Write in big endian order, like a real PowerPC
		PCIBridge.WriteRegister((addr&0xFF)+0,(data>>24)&0xFF);
		PCIBridge.WriteRegister((addr&0xFF)+1,(data>>16)&0xFF);
		PCIBridge.WriteRegister((addr&0xFF)+2,(data>>8)&0xFF);
		PCIBridge.WriteRegister((addr&0xFF)+3,data&0xFF);
	}
	else if (((addr>=0xF9000000) && (addr<0xF9000100)) || ((addr>=0xC1000000) && (addr<0xC1000100)) || ((Game->step==0x10) && ((addr>=0xC0000000) && (addr<0xC0000100))))
	{	
		SCSI.WriteRegister((addr&0xFF)+0,(data>>24)&0xFF);
		SCSI.WriteRegister((addr&0xFF)+1,(data>>16)&0xFF);
		SCSI.WriteRegister((addr&0xFF)+2,(data>>8)&0xFF);
		SCSI.WriteRegister((addr&0xFF)+3,data&0xFF);
	}
	else
	{
		//printf("%08X=%08X\n", addr, data);
		DebugLog("PC=%08X\twrite32: %08X=%08X\n", ppc_get_pc(), addr, data);
	}
}

void CModel3::Write64(UINT32 addr, UINT64 data)
{
    Write32(addr+0, (UINT32) (data>>32));
    Write32(addr+4, (UINT32) data);
}


#endif

 
/******************************************************************************
 Emulation and Interface Functions
******************************************************************************/

void CModel3::SaveState(CBlockFile *SaveState)
{
	// Write Model 3 state
	SaveState->NewBlock("Model 3", __FILE__);
	SaveState->Write(&inputBank, sizeof(inputBank));
	SaveState->Write(&serialFIFO1, sizeof(serialFIFO1));
	SaveState->Write(&serialFIFO2, sizeof(serialFIFO2));
	SaveState->Write(&gunReg, sizeof(gunReg));
	SaveState->Write(&adcChannel, sizeof(adcChannel));
	SaveState->Write(&cromBankReg, sizeof(cromBankReg));
	SaveState->Write(&securityPtr, sizeof(securityPtr));
	SaveState->Write(ram, 0x800000);
	SaveState->Write(backupRAM, 0x20000);
	SaveState->Write(securityRAM, 0x20000);
	SaveState->Write(&midiCtrlPort, sizeof(midiCtrlPort));
	
	// All devices...
	ppc_save_state(SaveState);
	IRQ.SaveState(SaveState);
	PCIBridge.SaveState(SaveState);
	SCSI.SaveState(SaveState);
	EEPROM.SaveState(SaveState);
	TileGen.SaveState(SaveState);
	GPU.SaveState(SaveState);
	SoundBoard.SaveState(SaveState);	// also saves DSB state
	DriveBoard.SaveState(SaveState);
}

void CModel3::LoadState(CBlockFile *SaveState)
{
	// Load Model 3 state
	if (OKAY != SaveState->FindBlock("Model 3"))
	{
		ErrorLog("Unable to load Model 3 core state. Save state file is corrupt.");
		return;
	}
	
	SaveState->Read(&inputBank, sizeof(inputBank));
	SaveState->Read(&serialFIFO1, sizeof(serialFIFO1));
	SaveState->Read(&serialFIFO2, sizeof(serialFIFO2));
	SaveState->Read(&gunReg, sizeof(gunReg));
	SaveState->Read(&adcChannel, sizeof(adcChannel));
	SaveState->Read(&cromBankReg, sizeof(cromBankReg));
	SetCROMBank(cromBankReg);	// update CROM bank
	SaveState->Read(&securityPtr, sizeof(securityPtr));
	SaveState->Read(ram, 0x800000);
	SaveState->Read(backupRAM, 0x20000);
	SaveState->Read(securityRAM, 0x20000);
	SaveState->Read(&midiCtrlPort, sizeof(midiCtrlPort));
	
	// All devices...
	GPU.LoadState(SaveState);
	TileGen.LoadState(SaveState);
	EEPROM.LoadState(SaveState);
	SCSI.LoadState(SaveState);
	PCIBridge.LoadState(SaveState);
	IRQ.LoadState(SaveState);
	ppc_load_state(SaveState);
	SoundBoard.LoadState(SaveState);
	DriveBoard.LoadState(SaveState);
}

void CModel3::SaveNVRAM(CBlockFile *NVRAM)
{
	// Load EEPROM
	EEPROM.SaveState(NVRAM);

	// Save backup RAM
	NVRAM->NewBlock("Backup RAM", __FILE__);
	NVRAM->Write(backupRAM, 0x20000);
}

void CModel3::LoadNVRAM(CBlockFile *NVRAM)
{
	// Load EEPROM
	EEPROM.LoadState(NVRAM);
	
	// Load backup RAM
	if (OKAY != NVRAM->FindBlock("Backup RAM"))
	{
		ErrorLog("Unable to load Model 3 backup RAM. NVRAM file is corrupt.");
		return;
	}
	NVRAM->Read(backupRAM, 0x20000);
}

void CModel3::ClearNVRAM(void)
{
	memset(backupRAM, 0, 0x20000);
	EEPROM.Clear();
}

void CModel3::RunFrame(void)
{
	// See if currently running multi-threaded
	if (g_Config.multiThreaded)
	{
		// If so, check all threads are up and running
		if (!StartThreads())
			goto ThreadError;

		// Wake threads for sound board (if sync'd) and drive board (if attached) so they can process a frame
		if (syncSndBrdThread && !sndBrdThreadSync->Post() || DriveBoard.IsAttached() && !drvBrdThreadSync->Post())
			goto ThreadError;

		// At the same time, process a single frame for main board (PPC) in this thread
		RunMainBoardFrame();

		// Enter notify wait critical section
		if (!notifyLock->Lock())
			goto ThreadError;

		// Wait for sound board and drive board threads to finish their work (if they haven't done so already)
		while (syncSndBrdThread && !sndBrdThreadDone || DriveBoard.IsAttached() && !drvBrdThreadDone)
		{
			if (!notifySync->Wait(notifyLock))
				goto ThreadError;
		}
		sndBrdThreadDone = false;
		drvBrdThreadDone = false;
		
		// Leave notify wait critical section
		if (!notifyLock->Unlock())
			goto ThreadError;
	}
	else
	{
		// If not multi-threaded, then just process a single frame for main board, sound board and drive board in turn in this thread
		RunMainBoardFrame();
		SoundBoard.RunFrame();
		if (DriveBoard.IsAttached())
			DriveBoard.RunFrame();
	}
	
	return;

ThreadError:
	ErrorLog("Threading error in CModel3::RunFrame: %s\nSwitching back to single-threaded mode.\n", CThread::GetLastError());
	g_Config.multiThreaded = false;
}

bool CModel3::StartThreads(void)
{
	if (startedThreads)
		return true;
			
	// Create synchronization objects
	sndBrdThreadSync = CThread::CreateSemaphore(1);
	if (sndBrdThreadSync == NULL)
		goto ThreadError;
	sndBrdNotifyLock = CThread::CreateMutex();
	if (sndBrdNotifyLock == NULL)
		goto ThreadError;
	sndBrdNotifySync = CThread::CreateCondVar();
	if (sndBrdNotifySync == NULL)
		goto ThreadError;
	if (DriveBoard.IsAttached())
	{
		drvBrdThreadSync = CThread::CreateSemaphore(1);
		if (drvBrdThreadSync == NULL)
			goto ThreadError;
	}
	notifyLock = CThread::CreateMutex();
	if (notifyLock == NULL)
		goto ThreadError;
	notifySync = CThread::CreateCondVar();
	if (notifySync == NULL)
		goto ThreadError;

	// Create sound board thread (sync'd or unsync'd)
	if (syncSndBrdThread)
		sndBrdThread = CThread::CreateThread(StartSoundBoardThreadSyncd, this);
	else
		sndBrdThread = CThread::CreateThread(StartSoundBoardThread, this);
	if (sndBrdThread == NULL)
		goto ThreadError;

	// Create drive board thread (sync'd), if drive board is attached
	if (DriveBoard.IsAttached())
	{
		drvBrdThread = CThread::CreateThread(StartDriveBoardThreadSyncd, this);
		if (drvBrdThread == NULL)
			goto ThreadError;
	}

	// Set audio callback if unsync'd
	if (!syncSndBrdThread)
		SetAudioCallback(AudioCallback, this);
	
	startedThreads = true;
	return true;

ThreadError:
	ErrorLog("Unable to create threads and/or synchronization objects: %s\nSwitching back to single-threaded mode.\n", CThread::GetLastError());
	DeleteThreadObjects();
	g_Config.multiThreaded = false;
	return false;
}

bool CModel3::PauseThreads(void)
{
	if (!startedThreads)
		return true;
	
	// Enter notify critical section
	if (!notifyLock->Lock())
		goto ThreadError;

	// Wait for all threads to finish their processing
	pausedThreads = true;
	while (sndBrdThreadRunning || drvBrdThreadRunning)
	{
		if (!notifySync->Wait(notifyLock))
			goto ThreadError;
	}

	// Leave notify critical section
	if (!notifyLock->Unlock())
		goto ThreadError;
	return true;

ThreadError:
	ErrorLog("Threading error in CModel3::PauseThreads: %s\nSwitching back to single-threaded mode.\n", CThread::GetLastError());
	g_Config.multiThreaded = false;
	return false;
}

void CModel3::ResumeThreads(void)
{	
	// No need to use any locking here
	pausedThreads = false;
	return;
}

void CModel3::StopThreads(void)
{
	if (!startedThreads)
		return;

	// If sound board not sync'd then remove callback
	if (!syncSndBrdThread)
		SetAudioCallback(NULL, NULL);
	
	// Pause threads so that can safely delete thread objects
	PauseThreads();
	
	DeleteThreadObjects();
	startedThreads = false;
}

void CModel3::DeleteThreadObjects(void)
{
	// Delete (which in turn kills) sound board and drive board threads
	// Note that can do so here safely because threads will always be waiting on their semaphores when this method is called
	if (sndBrdThread != NULL)
	{
		delete sndBrdThread;
		sndBrdThread = NULL;
	}
	if (drvBrdThread != NULL)
	{
		delete drvBrdThread;
		drvBrdThread = NULL;
	}

	// Delete synchronization objects
	if (sndBrdThreadSync != NULL)
	{
		delete sndBrdThreadSync;
		sndBrdThreadSync = NULL;
	}
	if (drvBrdThreadSync != NULL)
	{
		delete drvBrdThreadSync;
		drvBrdThreadSync = NULL;
	}
	if (sndBrdNotifyLock != NULL)
	{
		delete sndBrdNotifyLock;
		sndBrdNotifyLock = NULL;
	}
	if (sndBrdNotifySync != NULL)
	{
		delete sndBrdNotifySync;
		sndBrdNotifySync = NULL;
	}
	if (notifyLock != NULL)
	{
		delete notifyLock;
		notifyLock = NULL;
	}
	if (notifySync != NULL)
	{
		delete notifySync;
		notifySync = NULL;
	}
}

int CModel3::StartSoundBoardThread(void *data)
{
	// Call method on CModel3 to run unsync'd sound board thread
	CModel3 *model3 = (CModel3*)data;
	model3->RunSoundBoardThread();
	return 0;
}

int CModel3::StartSoundBoardThreadSyncd(void *data)
{
	// Call method on CModel3 to run sync'd sound board thread
	CModel3 *model3 = (CModel3*)data;
	model3->RunSoundBoardThreadSyncd();
	return 0;
}

int CModel3::StartDriveBoardThreadSyncd(void *data)
{
	// Call method on CModel3 to run sync'd drive board thread
	CModel3 *model3 = (CModel3*)data;
	model3->RunDriveBoardThreadSyncd();
	return 0;
}

void CModel3::AudioCallback(void *data)
{
	// Call method on CModel3 to wake sound board thread
	CModel3 *model3 = (CModel3*)data;
	model3->WakeSoundBoardThread();
}

void CModel3::WakeSoundBoardThread(void)
{
	// Enter sound board notify critical section
	if (!sndBrdNotifyLock->Lock())
		goto ThreadError;

	// Signal to sound board that it should start processing again
	if (!sndBrdNotifySync->Signal())
		goto ThreadError;

	// Exit sound board notify critical section
	if (!sndBrdNotifyLock->Unlock())
		goto ThreadError;
	return;

ThreadError:
	ErrorLog("Threading error in WakeSoundBoardThread: %s\nSwitching back to single-threaded mode.\n", CThread::GetLastError());
	g_Config.multiThreaded = false;
}

void CModel3::RunSoundBoardThread(void)
{
	for (;;)
	{
		bool wait = true;
		while (wait)
		{
			// Enter sound board notify critical section
			if (!sndBrdNotifyLock->Lock())
				goto ThreadError;

			// Wait for notification from audio callback
			if (!sndBrdNotifySync->Wait(sndBrdNotifyLock))
				goto ThreadError;

			// Exit sound board notify critical section
			if (!sndBrdNotifyLock->Unlock())
				goto ThreadError;
	
			// Enter main notify critical section
			if (!notifyLock->Lock())
				goto ThreadError;

			// Check threads not paused
			if (!pausedThreads)
			{
				wait = false;
				sndBrdThreadRunning = true;
			}

			// Leave main notify critical section
			if (!notifyLock->Unlock())
				goto ThreadError;
		}

		// Keep processing frames until audio buffer is full
		bool repeat = true;
		// NOTE - performs an unlocked read of pausedThreads here, but this is okay
		while (!pausedThreads && !SoundBoard.RunFrame())
		{
			//printf("Rerunning sound board\n");
		}

		// Enter main notify critical section
		if (!notifyLock->Lock())
			goto ThreadError;

		// Let other threads know processing has finished
		sndBrdThreadRunning = false;
		sndBrdThreadDone = true;
		if (!notifySync->SignalAll())
			goto ThreadError;

		// Leave main notify critical section
		if (!notifyLock->Unlock())
			goto ThreadError;
	}

ThreadError:
	ErrorLog("Threading error in RunSoundBoardThread: %s\nSwitching back to single-threaded mode.\n", CThread::GetLastError());
	g_Config.multiThreaded = false;
}

void CModel3::RunSoundBoardThreadSyncd(void)
{
	for (;;)
	{
		bool wait = true;
		while (wait)
		{
			// Wait on sound board thread semaphore
			if (!sndBrdThreadSync->Wait())
				goto ThreadError;
	
			// Enter notify critical section
			if (!notifyLock->Lock())
				goto ThreadError;

			// Check threads not paused
			if (!pausedThreads)
			{
				wait = false;
				sndBrdThreadRunning = true;
			}

			// Leave notify critical section
			if (!notifyLock->Unlock())
				goto ThreadError;
		}

		// Process a single frame for sound board
		SoundBoard.RunFrame();

		// Enter notify critical section
		if (!notifyLock->Lock())
			goto ThreadError;

		// Let other threads know processing has finished
		sndBrdThreadRunning = false;
		sndBrdThreadDone = true;
		if (!notifySync->SignalAll())
			goto ThreadError;

		// Leave notify critical section
		if (!notifyLock->Unlock())
			goto ThreadError;
	}

ThreadError:
	ErrorLog("Threading error in RunSoundBoardThreadSyncd: %s\nSwitching back to single-threaded mode.\n", CThread::GetLastError());
	g_Config.multiThreaded = false;
}

void CModel3::RunDriveBoardThreadSyncd(void)
{
	for (;;)
	{
		bool wait = true;
		while (wait)
		{
			// Wait on drive board thread semaphore
			if (!drvBrdThreadSync->Wait())
				goto ThreadError;

			// Enter notify critical section
			if (!notifyLock->Lock())
				goto ThreadError;

			// Check threads not paused
			if (!pausedThreads)
			{
				wait = false;
				drvBrdThreadRunning = true;
			}
	
			// Leave notify critical section
			if (!notifyLock->Unlock())
				goto ThreadError;
		}

		// Process a single frame for drive board
		DriveBoard.RunFrame();

		// Enter notify critical section
		if (!notifyLock->Lock())
			goto ThreadError;

		// Let other threads know processing has finished
		drvBrdThreadRunning = false;
		drvBrdThreadDone = true;
		if (!notifySync->SignalAll())
			goto ThreadError;

		// Leave notify critical section
		if (!notifyLock->Unlock())
			goto ThreadError;
	}

ThreadError:
	ErrorLog("Threading error in RunDriveBoardThreadSyncd: %s\nSwitching back to single-threaded mode.\n", CThread::GetLastError());
	g_Config.multiThreaded = false;
}

void CModel3::RunMainBoardFrame(void)
{
	// Compute display and VBlank timings
	unsigned	frameCycles = g_Config.GetPowerPCFrequency()*1000000/60;
	unsigned	vblCycles = (unsigned) ((float) frameCycles * 2.5f/100.0f);	// 2.5% vblank (ridiculously short and wrong but bigger values cause flicker in Daytona)
	unsigned	dispCycles = frameCycles - vblCycles;
	
	// Run the PowerPC for the active display part of the frame
	ppc_execute(dispCycles);
	//printf("PC=%08X LR=%08X\n", ppc_get_pc(), ppc_get_lr());
	
	// VBlank
	TileGen.BeginFrame();
	GPU.BeginFrame();
	GPU.RenderFrame();
	IRQ.Assert(0x02);
	ppc_execute(vblCycles);
	//printf("PC=%08X LR=%08X\n", ppc_get_pc(), ppc_get_lr());	
	
	/*
	 * Sound:
	 *
	 * Bit 0x20 of the MIDI control port appears to enable periodic interrupts,
	 * which are used to send MIDI commands. Often games will write 0x27, send
	 * a series of commands, and write 0x06 to stop. Other games, like Star
	 * Wars Trilogy and Sega Rally 2, will enable interrupts at the beginning
	 * by writing 0x37 and will disable/enable interrupts to control command
	 * output.
	 */
	//printf("\t-- BEGIN (Ctrl=%02X, IRQEn=%02X, IRQPend=%02X) --\n", midiCtrlPort, IRQ.ReadIRQEnable()&0x40, IRQ.ReadIRQState());
	int irqCount = 0;
	while ((midiCtrlPort&0x20))
	//while (midiCtrlPort == 0x27)	// 27 triggers IRQ sequence, 06 stops it
	{
		// Don't waste time firing MIDI interrupts if game has disabled them
		if ((IRQ.ReadIRQEnable()&0x40) == 0)
			break;
			
		// Process MIDI interrupt
		IRQ.Assert(0x40);
		ppc_execute(200);	// give PowerPC time to acknowledge IRQ
		IRQ.Deassert(0x40);
		ppc_execute(200);	// acknowledge that IRQ was deasserted (TODO: is this really needed?)
		
		++irqCount;
		if (irqCount > 128)
		{
			//printf("\tMIDI FIFO OVERFLOW! (IRQEn=%02X, IRQPend=%02X)\n", IRQ.ReadIRQEnable()&0x40, IRQ.ReadIRQState());
			break;
		}
	}
	//printf("\t-- END --\n");
	//printf("PC=%08X LR=%08X\n", ppc_get_pc(), ppc_get_lr());

	// End frame
	GPU.EndFrame();
	TileGen.EndFrame();
	IRQ.Assert(0x0D);
}

void CModel3::Reset(void)
{
	// Clear memory (but do not modify backup RAM!)
	memset(ram, 0, 0x800000);
	
	// Initial bank is bank 0
	SetCROMBank(0xFF);
	
	// Reset security device
	securityPtr = 0;
	
	// Reset inputs
	inputBank = 0;
	serialFIFO1 = 0;
	serialFIFO2 = 0;
	adcChannel = 0;
	
	// MIDI
	midiCtrlPort = 0;
	
	// Reset all devices
	ppc_reset();
	IRQ.Reset();
	PCIBridge.Reset();
	PCIBus.Reset();
	SCSI.Reset();
	RTC.Reset();
	EEPROM.Reset();
	TileGen.Reset();
	GPU.Reset();
	SoundBoard.Reset();

	if (DriveBoard.IsAttached())
		DriveBoard.Reset();
	
	DebugLog("Model 3 reset\n");
}


/******************************************************************************
 Initialization, Shutdown, and ROM Management
******************************************************************************/

// Apply patches to games
void CModel3::Patch(void)
{
	if (!strcmp(Game->id, "vf3") || !strcmp(Game->id, "vf3a"))
	{
		*(UINT32 *) &crom[0x713C7C] = 0x60000000;
		*(UINT32 *) &crom[0x713E54] = 0x60000000;
		*(UINT32 *) &crom[0x7125B0] = 0x60000000;
		*(UINT32 *) &crom[0x7125D0] = 0x60000000;
	}
	else if (!strcmp(Game->id, "lemans24"))
	{
		// Base offset of program in CROM: 6473C0
		*(UINT32 *) &crom[0x6D8C4C] = 0x00000002;	// comm. mode: 00=master, 01=slave, 02=satellite
		*(UINT32 *) &crom[0x73FE38] = 0x38840004;	// an actual bug in the game code
		*(UINT32 *) &crom[0x73EB5C] = 0x60000000;
		*(UINT32 *) &crom[0x73EDD0] = 0x60000000;
		*(UINT32 *) &crom[0x73EDC4] = 0x60000000;
		//*(UINT32 *) &crom[0x6473C0+0xF8BD0] = 0x60000000;	// waiting for something from network card, called at F8CD8
		//*(UINT32 *) &crom[0x6473C0+0xF8B80] = 0x60000000;	// "", called at 0xF8D90		
	}
	else if (!strcmp(Game->id, "scud"))
	{
		// Base offset of program in CROM: 0x710000
		*(UINT32 *) &crom[0x712734] = 0x60000000;	// skips some ridiculously slow delay loop during boot-up
		*(UINT32 *) &crom[0x71AEBC] = 0x60000000;	// waiting for some flag in RAM that never gets modified (IRQ problem? try emulating VBL on Real3D)
		*(UINT32 *) &crom[0x712268] = 0x60000000;	// this corrects the boot-up menu (but why?)
		crom[0x787B36^3] = 0x00;          			// Link ID: 00=single, 01=master, 02=slave (can bypass network board error)
		*(UINT32 *) &crom[0x71277C] = 0x60000000;	// seems to allow the game to start
		*(UINT32 *) &crom[0x74072C] = 0x60000000; 	// ... ditto
		
		//*(UINT32 *)&crom[0x799DE8] = 0x00050208;   // debug menu
	}
	else if (!strcmp(Game->id, "scuda"))
	{
		*(UINT32 *) &crom[0x712734] = 0x60000000;	// skips some ridiculously slow delay loop during boot-up
	}
	else if (!strcmp(Game->id, "scudj"))
	{
		*(UINT32 *) &crom[0x7126C8] = 0x60000000;	// skips some ridiculously slow delay loop during boot-up
	}
	else if (!strcmp(Game->id, "scudp"))
	{
		/*
		 * RAM program structure:
		 * 
		 * 1540: 	Reset vector transfers control here. Effective start of
		 *			program. On error, game often resets here.
		 * 14844:	Appears to be beginning of the actual boot-up process.
		 */
		
		// Base offset of program in CROM: 710000
		// *(UINT32 *) &crom[0x713724] = 0x60000000;
		// *(UINT32 *) &crom[0x713744] = 0x60000000;
		// *(UINT32 *) &crom[0x741f48] = 0x60000000;
		
		*(UINT32 *) &crom[0x741f68] = 0x60000000;
		*(UINT32 *) &crom[0x7126B8] = 0x60000000;	// waits for something in RAM

		crom[0x7C62B2^3] = 0x00;	// link ID is copied to 0x10011E, set it to single
	}
	else if (!strcmp(Game->id, "von2"))
	{
        *(UINT32 *) &crom[0x1B0] = 0x7C631A78;		// eliminate annoyingly long delay loop
        *(UINT32 *) &crom[0x1B4] = 0x60000000;		// ""
  	}
  	else if (!strcmp(Game->id, "lostwsga"))
  	{
  		*(UINT32 *) &crom[0x7374f4] = 0x38840004;	// an actual bug in the game code
  	}
  	else if (!strcmp(Game->id, "vs215"))
  	{
  		// VS215 is a modification of VS2 that runs on Step 1.5 hardware. I
  		// suspect the code here is trying to detect the system type but am too
  		// lazy to figure it out right now.
  		*(UINT32 *) &crom[0x7001A8] = 0x48000000+(0xFFF01630-0xFFF001A8);	// force a jump to FFF01630
  	}
  	else if (!strcmp(Game->id, "vs298"))
  	{
  		// Base offset of program in CROM: 600000
  		// Inexplicably, at PC=AFC1C, a call is made to FC78, which is right in the middle of some
		// totally unrelated initialization code (ASIC checks). This causes an invalid pointer to be fetched. 
		// Perhaps FC78 should be overwritten with other program data by then? Why is it not?
		// Or, 300138 needs to be written with a non-zero value, it is loaded from EEPROM but is 0.
		*(UINT32 *) &crom[0x6AFC1C] = 0x60000000;
	}
  	else if (!strcmp(Game->id, "srally2"))
  	{
  		*(UINT32 *) &crom[0x7C0C4] = 0x60000000;
		*(UINT32 *) &crom[0x7C0C8] = 0x60000000;
		*(UINT32 *) &crom[0x7C0CC] = 0x60000000;
  	}
  	else if (!strcmp(Game->id, "daytona2"))
  	{
  		// Base address of program in CROM: 0x600000
		// 0x10019E is the location in RAM which contains link type.
		// Region menu can be accessed by entering test mode, holding start,
		// and pressing: green, green, blue, yellow, red, yellow, blue (VR4,4,2,3,1,3,2)
  		*(UINT32 *) &crom[0x68468c] = 0x60000000;	// protection device
  		*(UINT32 *) &crom[0x6063c4] = 0x60000000;	// needed to allow levels to load
		*(UINT32 *) &crom[0x616434] = 0x60000000;	// prevents PPC from executing invalid code (MMU?)
		*(UINT32 *) &crom[0x69f4e4] = 0x60000000;	// ""
		*(UINT32 *) &crom[0x600000+0x4C744] = 0x60000000;	// decrementer loop?
	}
	else if (!strcmp(Game->id, "dayto2pe"))
	{
		*(UINT32 *) &crom[0x606784] = 0x60000000;
		*(UINT32 *) &crom[0x69A3FC] = 0x60000000;		// MAME says: jump to encrypted code
		*(UINT32 *) &crom[0x618B28] = 0x60000000;		// MAME says: jump to encrypted code
		*(UINT32 *) &crom[0x64CA34] = 0x60000000;		// decrementer 
	}
	else if (!strcmp(Game->id, "fvipers2"))
  	{
		/*
		 * Game code is copied to RAM in a non-trivial fashion (it may be 
		 * compressed) just prior to the following sequence of code, which then
		 * transfers control to the RAM program:
		 *
  		 * FFF0153C: 3C200070 li   r1,0x00700000
		 * FFF01540: 3C60FF80 li   r3,0xFF800000
		 * FFF01544: 38800000 li   r4,0x00000000
		 * FFF01548: 48000751 bl   0xFFF01C98
		 * FFF0154C: 7C7F42A6 mfspr        r3,pvr
		 * FFF01550: 90600080 stw  r3,0x00000080
		 * FFF01554: 92E00084 stw  r23,0x00000084
		 * FFF01558: 38600100 li   r3,0x00000100
		 * FFF0155C: 7C6803A6 mtspr        lr,r3
		 * FFF01560: 4E800020 bclr 0x14,0
		 *
		 * In order to patch the necessary portions of the RAM program, we must
		 * insert a routine that executes after the program is loaded. There is
		 * ample room in the vector table between 0xFFF00004 and 0xFFF000FC.
		 * The patching routine must terminate with a "bclr 0x14,0" to jump to
		 * the RAM program.
		 */
		*(UINT32 *) &crom[0xFFF01560-0xFF800000] = 0x4BF00006;					// ba 		0xFFF00004
		*(UINT32 *) &crom[0xFFF00004-0xFF800000] = (31<<26)|(316<<1);			// xor 		r0,r0,r0		; R0 = 0
		*(UINT32 *) &crom[0xFFF00008-0xFF800000] = (15<<26)|(2<<21)|0x6000;		// addis	r2,0,0x6000		; R2 = nop
		*(UINT32 *) &crom[0xFFF0000C-0xFF800000] = (24<<26)|(1<<16)|0xA2E8;		// ori 		r1,r0,0xA2E8	; [A2E8] <- nop	(decrementer loop)
		*(UINT32 *) &crom[0xFFF00010-0xFF800000] = (36<<26)|(2<<21)|(1<<16);	// stw 		r2,0(r1)
		*(UINT32 *) &crom[0xFFF00014-0xFF800000] = 0x4E800020;					// bclr		0x14,0			; return to RAM code
		
		// NOTE: At 32714, a test is made that determines the message: ONE PROCESSOR DETECTED, TWO "", etc.
  	}
  	else if (!strcmp(Game->id, "harley"))
  	{
  		*(UINT32 *) &crom[0x50E8D4] = 0x60000000;
		*(UINT32 *) &crom[0x50E8F4] = 0x60000000;
		*(UINT32 *) &crom[0x50FB84] = 0x60000000;
  		
  		*(UINT32 *) &crom[0x4F736C] = 0x60000000;
		*(UINT32 *) &crom[0x4F738C] = 0x60000000;
  	}
  	else if (!strcmp(Game->id, "harleyb"))
  	{
  		*(UINT32 *) &crom[0x50ECB4] = 0x60000000;
		*(UINT32 *) &crom[0x50ECD4] = 0x60000000;
		*(UINT32 *) &crom[0x50FF64] = 0x60000000;
  		
  		*(UINT32 *) &crom[0x4F774C] = 0x60000000;
		*(UINT32 *) &crom[0x4F776C] = 0x60000000;
  	}
  	else if (!strcmp(Game->id, "oceanhun"))
  	{
  		// Base address of program in CROM: 588FD8-108FD8=480000
  		//*(UINT32 *) &crom[0x480000+0x108FE0] = 0x60000000;	// bad DMA copies from CROM
  		//*(UINT32 *) &crom[0x480000+0x112020] = 0x60000000;	// reads from invalid addresses (due to CROM?)
  		*(UINT32 *) &crom[0x480000+0xF995C] = 0x60000000;	// decrementer
  	}
  	else if (!strcmp(Game->id, "swtrilgy"))
  	{
  		*(UINT32 *)	&crom[0xF0E48] = 0x60000000;
		*(UINT32 *)	&crom[0x043DC] = 0x48000090;
		*(UINT32 *)	&crom[0x029A0] = 0x60000000;
		*(UINT32 *)	&crom[0x02A0C] = 0x60000000;
  	}
  	else if (!strcmp(Game->id, "swtrilgya"))
  	{
  		*(UINT32 *) &crom[0xF6DD0] = 0x60000000;	// from MAME
  		
  		//*(UINT32 *) &crom[0xF1128] = 0x60000000;	// these bypass required delay loops and break game timing
  		//*(UINT32 *) &crom[0xF10E0] = 0x60000000;
  	}
  	else if (!strcmp(Game->id, "eca") || !strcmp(Game->id, "ecax"))
  	{
		*(UINT32 *) &crom[0x535580] = 0x60000000;
		*(UINT32 *) &crom[0x5023B4] = 0x60000000;
		*(UINT32 *) &crom[0x5023D4] = 0x60000000;
		
		*(UINT32 *) &crom[0x535560] = 0x60000000;	// decrementer loop
	}
	else if (!strcmp(Game->id, "spikeout"))
	{
		/*
		 * Decrementer loop at 0x31994 seems to work until a few frames into
		 * the attract mode and game, at which point a very large value is
		 * loaded into the decrementer and locks up the CPU (the usual
		 * decrementer problem). "Insert Coin" keeps flashing because it is
		 * managed via an IRQ, evidently.
		 *
		 * 0x00031994: 0x7C9602A6	mfspr	r4,dec
		 * 0x00031998: 0x2C040000	cmpi	cr0,0,r4,0x0000
		 * 0x0003199C: 0x41A0FFF8	bt	cr0[lt],0x00031994
		 */
		
		*(UINT32 *) &crom[0x600000+0x3199C] = 0x60000000;
	}
	else if (!strcmp(Game->id, "spikeofe"))
	{
		*(UINT32 *) &crom[0x600000+0x36F2C] = 0x60000000;	// decrementer loop (see Spikeout)
	}
	else if (!strcmp(Game->id, "skichamp"))
	{
		// Base address of program in CROM: 0x480000
		*(UINT32 *) &crom[0x480000+0x96B9C] = 0x60000000;	// decrementer loop
	}
}

// Reverses all aligned 16-bit words, thereby switching their endianness (assumes buffer size is divisible by 2)
static void Reverse16(UINT8 *buf, unsigned size)
{
	unsigned	i;
	UINT8		tmp;
	
	for (i = 0; i < size; i += 2)
	{
		tmp = buf[i+0];
		buf[i+0] = buf[i+1];
		buf[i+1] = tmp;
	}
}

// Reverses all aligned 32-bit words, thereby switching their endianness (assumes buffer size is divisible by 4)
static void Reverse32(UINT8 *buf, unsigned size)
{
	unsigned	i;
	UINT8		tmp1, tmp2;
	
	for (i = 0; i < size; i += 4)
	{
		tmp1 = buf[i+0];
		tmp2 = buf[i+1];
		buf[i+0] = buf[i+3];
		buf[i+1] = buf[i+2];
		buf[i+2] = tmp2;
		buf[i+3] = tmp1;
	}
}

// Dumps a memory region to a file for debugging purposes
static void Dump(const char *file, UINT8 *buf, unsigned size, bool reverse32, bool reverse16)
{
	FILE	*fp;
	
	fp = fopen(file, "wb");
	if (NULL != fp)
	{
		if (reverse32)
			Reverse32(buf, size);
		else if (reverse16)
			Reverse16(buf, size);
		fwrite(buf, sizeof(UINT8), size, fp);
		fclose(fp);
		printf("dumped %s\n", file);
	}
	else
		printf("unable to dump %s\n", file);
}

// Offsets of memory regions within Model 3's pool
#define OFFSET_RAM			0			// 8 MB
#define OFFSET_CROM			0x800000	// 8 MB (fixed CROM)
#define OFFSET_CROMxx		0x1000000	// 128 MB (banked CROM0-3 must follow fixed CROM)
#define OFFSET_VROM			0x9000000	// 64 MB
#define OFFSET_BACKUPRAM	0xD000000	// 128 KB
#define OFFSET_SECURITYRAM	0xD020000	// 128 KB
#define OFFSET_SOUNDROM		0xD040000	// 512 KB (68K sound board program)
#define OFFSET_SAMPLEROM	0xD0C0000	// 16 MB (sound board samples)
#define OFFSET_DSBPROGROM	0xE0C0000	// 128 KB (DSB program)
#define OFFSET_DSBMPEGROM	0xE0E0000	// 16 MB (DSB MPEG data -- Z80 version only uses 8MB)
#define OFFSET_DRIVEROM		0xF0E0000	// 64 KB
#define MEMORY_POOL_SIZE	(0x800000 + 0x800000 + 0x8000000 + 0x4000000 + 0x20000 + 0x20000 + 0x80000 + 0x1000000 + 0x20000 + 0x1000000 + 0x10000)

// 64-bit magic number to use detect if ROM was loaded
#define MAGIC_NUMBER	0x4C444D5245505553ULL

const struct GameInfo * CModel3::GetGameInfo(void)
{
	return Game;
}
	
// Stepping-dependent parameters (MPC10x type, etc.) are initialized here
bool CModel3::LoadROMSet(const struct GameInfo *GameList, const char *zipFile)
{
	struct ROMMap Map[] =
	{
		{ "CROM", 		crom },
		{ "CROMxx",		&crom[0x800000] },	
		{ "VROM", 		vrom },
		{ "SndProg",	soundROM },
		{ "Samples",	sampleROM },
		{ "DSBProg",	dsbROM },
		{ "DSBMPEG",	mpegROM },
		{ "DriveBd",	driveROM },
		{ NULL, NULL }
	};
	PPC_CONFIG	PPCConfig;
	
	// Magic numbers to detect if optional ROMs are loaded
	*(UINT64 *) driveROM = MAGIC_NUMBER;
	
	// Load game
	Game = LoadROMSetFromZIPFile(Map, GameList, zipFile, true);
	if (NULL == Game)
		return ErrorLog("Failed to load ROM set.");	
	
	// Perform mirroring as necessary 
	if (Game->vromSize < 0x4000000)		// VROM is actually 64 MB
		CopyRegion(vrom, Game->vromSize, 0x4000000, vrom, Game->vromSize);
	if (Game->cromSize < 0x800000)		// low part of fixed CROM region contains CROM0
		CopyRegion(crom, 0, 0x800000-Game->cromSize, &crom[0x800000], 0x800000);
	if (Game->mirrorLow64MB)			// for games w/ 64 MB or less banked CROM, mirror to upper 128 MB
		CopyRegion(&crom[0x800000], 0x4000000, 0x8000000, &crom[0x800000], 0x4000000);
	if (Game->sampleSize < 0x1000000)	// if less than 16 MB of sample ROMs, mirror
		CopyRegion(sampleROM, 0x800000, 0x1000000, sampleROM, 0x800000);
		
	// Byte reverse the PowerPC ROMs (convert to little endian words)
	Reverse32(crom, 0x800000+0x8000000);
	
	// Byte swap sound board 68K ROMs
	Reverse16(soundROM, 0x80000);
	Reverse16(sampleROM, 0x1000000);
		
	// Initialize CPU and configure hardware (CPU speed is set in Init())
	if (Game->step >= 0x20)			// Step 2.0+
	{
		PPCConfig.pvr = PPC_MODEL_603R;	// 166 MHz
		PPCConfig.bus_frequency = BUS_FREQUENCY_66MHZ;
		PPCConfig.bus_frequency_multiplier = 0x25;	// 2.5X multiplier
		PCIBridge.SetModel(0x106);		// MPC106
	} 
	else if (Game->step == 0x15)	// Step 1.5
	{
		PPCConfig.pvr = PPC_MODEL_603E;	// 100 MHz
		PPCConfig.bus_frequency = BUS_FREQUENCY_66MHZ;
		PPCConfig.bus_frequency_multiplier = 0x15;	// 1.5X multiplier
		PCIBridge.SetModel(0x105);		// MPC105
	}
	else if (Game->step == 0x10)	// Step 1.0
	{
		PPCConfig.pvr = PPC_MODEL_603R;	// 66 MHz
		PPCConfig.bus_frequency = BUS_FREQUENCY_66MHZ;
		PPCConfig.bus_frequency_multiplier = 0x10;	// 1X multiplier
		if (!strcmp(Game->id, "bass") || !strcmp(Game->id, "getbass"))	// some Step 1.0 games use MPC106
			PCIBridge.SetModel(0x106);
		else
			PCIBridge.SetModel(0x105);	// MPC105
	}
	else
		return ErrorLog("Game uses an unrecognized stepping (%d.%d), cannot configure Model 3.", (Game->step>>4)&0xF, Game->step&0xF);

	GPU.SetStep(Game->step);

	ppc_init(&PPCConfig);
	ppc_attach_bus(this);
	
	PPCFetchRegions[0].start = 0;	
	PPCFetchRegions[0].end = 0x007FFFFF;
	PPCFetchRegions[0].ptr = (UINT32 *) ram;
	PPCFetchRegions[1].start = 0xFF800000;	
	PPCFetchRegions[1].end = 0xFFFFFFFF;
	PPCFetchRegions[1].ptr = (UINT32 *) crom;
	PPCFetchRegions[2].start = 0;
	PPCFetchRegions[2].end = 0;
	PPCFetchRegions[2].ptr = NULL;
	
	ppc_set_fetch(PPCFetchRegions);
	
	// DSB board (if present)
	if (Game->mpegBoard == 1)		// Z80 board, do not byte swap program ROM
	{
		DSB = new(std::nothrow) CDSB1();
		if (NULL == DSB)
			return ErrorLog("Insufficient memory for Digital Sound Board object.");
		if (OKAY != DSB->Init(dsbROM,mpegROM))
			return FAIL;
	}
	else if (Game->mpegBoard == 2)	// 68K board
	{
		Reverse16(dsbROM, 0x20000);	// byte swap program ROM
		DSB = new(std::nothrow) CDSB2();
		if (NULL == DSB)
			return ErrorLog("Insufficient memory for Digital Sound Board object.");
		if (OKAY != DSB->Init(dsbROM,mpegROM))
			return FAIL;
	}
	SoundBoard.AttachDSB(DSB);
	
	// Drive board (if present)
	if (Game->driveBoard)
	{
		// Was the optional drive board ROM loaded?
		if (MAGIC_NUMBER != *(UINT64 *) driveROM)	// magic number overwritten by ROM
		{
			if (DriveBoard.Init(driveROM))
				return FAIL;
		}
		else
			DriveBoard.Init(NULL);
	}
	else
		DriveBoard.Init(NULL);	// disable
	
	// Apply ROM patches
	Patch();
	
	// Print game information
	printf("    Title:          %s\n", Game->title);
	printf("    ROM Set:        %s\n", Game->id);
	printf("    Developer:      %s\n", Game->mfgName);
	printf("    Year:           %d\n", Game->year);
	printf("    Step:           %d.%d\n", (Game->step>>4)&0xF, Game->step&0xF);
	if (Game->mpegBoard)
	{
		printf("    Extra Hardware: Digital Sound Board (Type %d)", Game->mpegBoard);
		if (Game->driveBoard)
			printf(", Drive Board");
		printf("\n");
	}
	else if (Game->driveBoard)
		printf("    Extra Hardware: Drive Board\n");
	printf("\n");
		
	return OKAY;
}

void CModel3::AttachRenderers(CRender2D *Render2DPtr, CRender3D *Render3DPtr)
{
	TileGen.AttachRenderer(Render2DPtr);
	GPU.AttachRenderer(Render3DPtr);
}

void CModel3::AttachInputs(CInputs *InputsPtr)
{
	Inputs = InputsPtr;

	if (DriveBoard.IsAttached())
		DriveBoard.AttachInputs(InputsPtr, Game->inputFlags);

	DebugLog("Model 3 attached inputs\n");
}

// Model 3 initialization. Some initialization is deferred until ROMs are loaded in LoadROMSet()
bool CModel3::Init(void)
{
	float	memSizeMB = (float)MEMORY_POOL_SIZE/(float)0x100000;
	
	// Allocate all memory for ROMs and PPC RAM
	memoryPool = new(std::nothrow) UINT8[MEMORY_POOL_SIZE];
	if (NULL == memoryPool)
		return ErrorLog("Insufficient memory for Model 3 object (needs %1.1f MB).", memSizeMB);
		
	// Set up pointers
	ram = &memoryPool[OFFSET_RAM];
	crom = &memoryPool[OFFSET_CROM];
	vrom = &memoryPool[OFFSET_VROM];
	soundROM = &memoryPool[OFFSET_SOUNDROM];
	sampleROM = &memoryPool[OFFSET_SAMPLEROM];
	dsbROM = &memoryPool[OFFSET_DSBPROGROM];
	mpegROM = &memoryPool[OFFSET_DSBMPEGROM];
	backupRAM = &memoryPool[OFFSET_BACKUPRAM];
	securityRAM = &memoryPool[OFFSET_SECURITYRAM];
	driveROM = &memoryPool[OFFSET_DRIVEROM];
	SetCROMBank(0xFF);
	
	// Initialize other devices (PowerPC and DSB initialized after ROMs loaded)
	IRQ.Init();
	PCIBridge.Init();
	PCIBus.Init();
	SCSI.Init(this,&IRQ,0x100);	// SCSI is actually a non-maskable interrupt, so we give it a bit number outside of 8-bit range
	RTC.Init();
	EEPROM.Init();
	if (OKAY != TileGen.Init(&IRQ))
		return FAIL;
	if (OKAY != GPU.Init(vrom,this,&IRQ,0x100))	// same for Real3D DMA interrupt
		return FAIL;
	if (OKAY != SoundBoard.Init(soundROM,sampleROM))
		return FAIL;
		
	PCIBridge.AttachPCIBus(&PCIBus);
	PCIBus.AttachDevice(13,&GPU);
	PCIBus.AttachDevice(14,&SCSI);
	PCIBus.AttachDevice(16,this);
	
	DebugLog("Initialized Model 3 (allocated %1.1f MB)\n", memSizeMB);
	
	return OKAY;
}

CSoundBoard *CModel3::GetSoundBoard(void)
{
	return &SoundBoard;
}
 
CDriveBoard *CModel3::GetDriveBoard(void)
{
	return &DriveBoard;
}

CModel3::CModel3(void)
{
	// Initialize pointers so dtor can know whether to free them
	memoryPool = NULL;
	
	// Various uninitialized pointers
	Game = NULL;
	ram = NULL;
	crom = NULL;
	vrom = NULL;
	soundROM = NULL;
	sampleROM = NULL;
	dsbROM = NULL;
	mpegROM = NULL;
	cromBank = NULL;
	backupRAM = NULL;
	securityRAM = NULL;
	
	DSB = NULL;
	
	securityPtr = 0;
	
	startedThreads = false;
	pausedThreads = false;
	sndBrdThread = NULL; 
	drvBrdThread = NULL;
	sndBrdThreadRunning = false;
	sndBrdThreadDone = false;
	drvBrdThreadRunning = false;
	drvBrdThreadDone = false;
	syncSndBrdThread = false;
	sndBrdThreadSync = NULL;
	drvBrdThreadSync = NULL;
	notifyLock = NULL;
	notifySync = NULL;
	
	DebugLog("Built Model 3\n");
}

CModel3::~CModel3(void)
{
	// Debug: dump some files
#if 0
	Dump("ram", ram, 0x800000, true, false);
	//Dump("vrom", vrom, 0x4000000, true, false);
	//Dump("crom", crom, 0x800000, true, false);
	//Dump("bankedCrom", &crom[0x800000], 0x7000000, true, false);
	//Dump("soundROM", soundROM, 0x80000, false, true);
	//Dump("sampleROM", sampleROM, 0x800000, false, true);
#endif
	
	// Stop all threads
	StopThreads();
	
	// Free memory
	if (memoryPool != NULL)
	{
		delete [] memoryPool;
		memoryPool = NULL;
	}
	
	if (DSB != NULL)
	{
		delete DSB;
		DSB = NULL;
	}
	
	Game = NULL;
	ram = NULL;
	crom = NULL;
	vrom = NULL;
	soundROM = NULL;
	sampleROM = NULL;
	dsbROM = NULL;
	mpegROM = NULL;
	cromBank = NULL;
	backupRAM = NULL;
	securityRAM = NULL;
	
	DebugLog("Destroyed Model 3\n");
}