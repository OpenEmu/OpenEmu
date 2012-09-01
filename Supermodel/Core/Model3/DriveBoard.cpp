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
 * DriveBoard.cpp
 *
 * Implementation of the CDriveBoard class: drive board (force feedback)
 * emulation.
 *
 * NOTE: Simulation does not yet work. Drive board ROMs are required.
 */

#include "Supermodel.h"

#include <cstdio>
#include <cmath>

#define RAM_SIZE 0x2000	// Z80 RAM

bool CDriveBoard::IsAttached(void)
{
	return m_attached && !m_tmpDisabled;
}

bool CDriveBoard::IsSimulated(void)
{
	return m_simulated;
}

void CDriveBoard::GetDIPSwitches(UINT8 &dip1, UINT8 &dip2)
{
	dip1 = m_dip1;
	dip2 = m_dip2;
}

void CDriveBoard::SetDIPSwitches(UINT8 dip1, UINT8 dip2)
{
	m_dip1 = dip1;
	m_dip2 = dip2;
}

unsigned CDriveBoard::GetSteeringStrength()
{
	return ((~(m_dip1>>2))&7) + 1;
}

void CDriveBoard::SetSteeringStrength(unsigned steeringStrength)
{
	m_dip1 = (m_dip1&0xE3) | (((~(steeringStrength - 1))&7)<<2);
}

void CDriveBoard::Get7SegDisplays(UINT8 &seg1Digit1, UINT8 &seg1Digit2, UINT8 &seg2Digit1, UINT8 &seg2Digit2)
{
	seg1Digit1 = m_seg1Digit1;
	seg1Digit2 = m_seg1Digit2;
	seg2Digit1 = m_seg2Digit1;
	seg2Digit2 = m_seg2Digit2;
}

CZ80 *CDriveBoard::GetZ80(void)
{
	return &m_z80;
}

void CDriveBoard::SaveState(CBlockFile *SaveState)
{
	SaveState->NewBlock("DriveBoard", __FILE__);
	
	// Check board is attached and not temporarily disabled
	bool attached = m_attached && !m_tmpDisabled;
	SaveState->Write(&attached, sizeof(attached));
	if (attached)
	{
		// Check if simulated
		SaveState->Write(&m_simulated, sizeof(m_simulated));
		if (m_simulated)
		{
			// TODO - save board simulation state
		}
		else
		{
			// Save DIP switches and digit displays
			SaveState->Write(&m_dip1, sizeof(m_dip1));	
			SaveState->Write(&m_dip2, sizeof(m_dip2));
			//SaveState->Write(&m_seg1Digit1, sizeof(m_seg1Digit1)); // No point in saving these
			//SaveState->Write(&m_seg1Digit2, sizeof(m_seg1Digit2));
			//SaveState->Write(&m_seg2Digit1, sizeof(m_seg2Digit1));
			//SaveState->Write(&m_seg2Digit2, sizeof(m_seg2Digit2));

			// Save RAM state
			SaveState->Write(m_ram, RAM_SIZE);
	
			// Save interrupt and input/output state
			SaveState->Write(&m_initialized, sizeof(m_initialized));
			SaveState->Write(&m_allowInterrupts, sizeof(m_allowInterrupts));
			SaveState->Write(&m_dataSent, sizeof(m_dataSent));
			SaveState->Write(&m_dataReceived, sizeof(m_dataReceived));
			SaveState->Write(&m_adcPortRead, sizeof(m_adcPortRead));
			SaveState->Write(&m_adcPortBit, sizeof(m_adcPortBit));
			SaveState->Write(&m_uncenterVal1, sizeof(m_uncenterVal1));
			SaveState->Write(&m_uncenterVal2, sizeof(m_uncenterVal2));
		
			// Save CPU state
			m_z80.SaveState(SaveState, "DriveBoard Z80");
		}
	}
}

void CDriveBoard::LoadState(CBlockFile *SaveState)
{
	if (SaveState->FindBlock("DriveBoard") != OKAY)
	{
		ErrorLog("Unable to load drive board state. Save state file is corrupt.");
		return;
	}
	
	// Check that board was attached in saved state
	bool wasAttached;
	SaveState->Read(&wasAttached, sizeof(wasAttached));
	if (wasAttached)
	{
		// Check that board configuration exactly matches current configuration
		bool wasSimulated;
		SaveState->Read(&wasSimulated, sizeof(wasSimulated));
		if (wasAttached == m_attached && wasSimulated == m_simulated)
		{
			// Check if board was simulated
			if (wasSimulated)
			{
				// TODO - load board simulation state
			}
			else
			{
				// Load DIP switches and digit displays
				SaveState->Read(&m_dip1, sizeof(m_dip1));
				SaveState->Read(&m_dip2, sizeof(m_dip2));
				//SaveState->Read(&m_seg1Digit1, sizeof(m_seg1Digit1));
				//SaveState->Read(&m_seg1Digit2, sizeof(m_seg1Digit2));
				//SaveState->Read(&m_seg2Digit1, sizeof(m_seg2Digit1));
				//SaveState->Read(&m_seg2Digit2, sizeof(m_seg2Digit2));

				// Load RAM state
				SaveState->Read(m_ram, RAM_SIZE);
				
				// Load interrupt and input/output state
				SaveState->Read(&m_initialized, sizeof(m_initialized));
				SaveState->Read(&m_allowInterrupts, sizeof(m_allowInterrupts));
				SaveState->Read(&m_dataSent, sizeof(m_dataSent));
				SaveState->Read(&m_dataReceived, sizeof(m_dataReceived));
				SaveState->Read(&m_adcPortRead, sizeof(m_adcPortRead));
				SaveState->Read(&m_adcPortBit, sizeof(m_adcPortBit));
				SaveState->Read(&m_uncenterVal1, sizeof(m_uncenterVal1));
				SaveState->Read(&m_uncenterVal2, sizeof(m_uncenterVal2));
			
				// Load CPU state
				m_z80.LoadState(SaveState, "DriveBoard Z80");
			}

			// Enable board
			m_tmpDisabled = false;
		}
		else
		{
			// Otherwise, disable board as it can't be used with a mismatching configuratin
			m_tmpDisabled = true;
		}
	}
	else
		// Disable board if it was not attached
		m_tmpDisabled = true;

	if (m_attached)
	{
		if (m_tmpDisabled)
			printf("Disabled drive board due to incompatible save state.\n");

		SendStopAll();
	}
}

bool CDriveBoard::Init(const UINT8 *romPtr)
{	
	// Assign ROM (note that the ROM data has not yet been loaded)
	m_rom = romPtr;

	// Check have a valid ROM
	m_attached = (m_rom != NULL);
	if (!m_attached)
		return OKAY;

	// Allocate memory for RAM
	m_ram = new (std::nothrow) UINT8[RAM_SIZE];
	if (NULL == m_ram)
	{
		float ramSizeMB = (float)RAM_SIZE/(float)0x100000;
		return ErrorLog("Insufficient memoy for drive board (needs %1.1f MB).", ramSizeMB);
	}
	memset(m_ram, 0, RAM_SIZE);

	// Initialize Z80
	m_z80.Init(this, NULL);

	return OKAY;
}

void CDriveBoard::AttachInputs(CInputs *InputsPtr, unsigned gameInputFlags)
{
	m_inputs = InputsPtr;
	m_inputFlags = gameInputFlags;

	DebugLog("DriveBoard attached inputs\n");
}

void CDriveBoard::Reset(void)
{
	m_tmpDisabled = false;

	m_initialized = false;
	m_allowInterrupts = false;

	m_seg1Digit1 = 0xFF;
	m_seg1Digit2 = 0xFF;
	m_seg2Digit1 = 0xFF;
	m_seg2Digit2 = 0xFF;

	m_dataSent = 0;
	m_dataReceived = 0;
	m_adcPortRead = 0;
	m_adcPortBit = 0;
	m_port42Out = 0;
	m_port46Out = 0;
	m_prev42Out = 0;
	m_prev46Out = 0;

	m_initState = 0;
	m_boardMode = 0;
	m_readMode = 0;
	m_wheelCenter = 0x80;
	m_uncenterVal1 = 0;
	m_uncenterVal2 = 0;

	m_lastConstForce = 0;
	m_lastSelfCenter = 0;
	m_lastFriction = 0;
	m_lastVibrate = 0;

	// Configure options (cannot be done in Init() because command line settings weren't yet parsed)
	m_simulated = g_Config.simulateDrvBoard;
	SetSteeringStrength(g_Config.steeringStrength);

	m_z80.Reset();	// always reset to provide a valid Z80 state
	
	if (!g_Config.forceFeedback)
		m_attached = false;

	// Stop any effects that may still be playing
	if (m_attached)
		SendStopAll();
}

UINT8 CDriveBoard::Read(void)
{
	// TODO - simulate initialization sequence even when emulating to get rid of long pause at boot up (drive board can
	// carry on booting whilst game starts)
	if (m_simulated)
		return SimulateRead();
	else
		return m_dataReceived;
}

void CDriveBoard::Write(UINT8 data)
{
	//if (data >= 0x01 && data <= 0x0F ||
	//	data >= 0x20 && data <= 0x2F || 
	//	data >= 0x30 && data <= 0x3F || 
	//	data >= 0x40 && data <= 0x4F || 
	//	data >= 0x70 && data <= 0x7F) 
	//	printf("DriveBoard.Write(%02X)\n", data);
	if (m_simulated)
		SimulateWrite(data);
	else
	{
		m_dataSent = data;
		if (data == 0xCB)
			m_initialized = false;
	}
}

UINT8 CDriveBoard::SimulateRead(void)
{
	if (m_initialized)
	{	
		switch (m_readMode)
		{
			case 0x0: return m_statusFlags;             // Status flags
			case 0x1: return m_dip1;                    // DIP switch 1 value
			case 0x2: return m_dip2;                    // DIP switch 2 value
			case 0x3: return m_wheelCenter;             // Wheel center
			case 0x4: return 0x80;                      // Cockpit banking center
			case 0x5: return m_inputs->steering->value; // Wheel position
			case 0x6: return 0x80;                      // Cockpit banking position
			case 0x7: return m_echoVal;                 // Init status/echo test
			default:  return 0xFF;
		}
	}
	else
	{
		switch (m_initState / 5)
		{
			case 0:  return 0xCF;  // Initiate start
			case 1:  return 0xCE;  
			case 2:  return 0xCD;
			case 3:  return 0xCC;  // Centering wheel
			default:
				m_initialized = true;
				return 0x80;
		}
	}
}

void CDriveBoard::SimulateWrite(UINT8 cmd)
{
	// Following are commands for Scud Race.  Daytona 2 has a compatible command set while Sega Rally 2 is completely different
	// TODO - finish for Scud Race and Daytona 2
	// TODO - implement for Sega Rally 2
	UINT8 type = cmd>>4;
	UINT8 val = cmd&0xF;
	switch (type)
	{
		case 0: // 0x00-0F Play sequence
			/* TODO */
			break;   
		case 1: // 0x10-1F Set centering strength
			if (val == 0)
				// Disable auto-centering
				// TODO - is 0x10 for disable?
				SendSelfCenter(0);
			else
				// Enable auto-centering (0x1 = weakest, 0xF = strongest)
				SendSelfCenter(val * 0x11);
			break;
		case 2: // 0x20-2F Friction strength
			if (val == 0)
				// Disable friction
				// TODO - is 0x20 for disable?
				SendFriction(0);
			else
				// Enable friction (0x1 = weakest, 0xF = strongest)
				SendFriction(val * 0x11);
			break;
		case 3: // 0x30-3F Uncentering (vibrate)
			if (val == 0)
				// Disable uncentering
				SendVibrate(0);
			else
				// Enable uncentering (0x1 = weakest, 0xF = strongest)
				SendVibrate(val * 0x11); 
			break;
		case 4: // 0x40-4F Play power-slide sequence
			/* TODO */
			break;
		case 5: // 0x50-5F Rotate wheel right
			SendConstantForce((val + 1) * 0x5);  
			break;   
		case 6: // 0x60-6F Rotate wheel left
			SendConstantForce(-(val + 1) * 0x5); 
			break;   
		case 7: // 0x70-7F Set steering parameters
			/* TODO */               
			break;
		case 8: // 0x80-8F Test Mode
			switch (val&0x7)
			{
				case 0:  SendStopAll();                             break;  // 0x80 Stop motor
				case 1:  SendConstantForce(20);                     break;  // 0x81 Roll wheel right
				case 2:  SendConstantForce(-20);                    break;  // 0x82 Roll wheel left
				case 3:  /* Ignore - no clutch */                   break;  // 0x83 Clutch on 
				case 4:  /* Ignore - no clutch */                   break;  // 0x84 Clutch off
				case 5:  m_wheelCenter = m_inputs->steering->value; break;  // 0x85 Set wheel center position
				case 6:  /* Ignore */                               break;  // 0x86 Set cockpit banking position
				case 7:  /* Ignore */                               break;  // 0x87 Lamp on/off
			}
		case 0x9: // 0x90-9F ??? Don't appear to have any effect with Scud Race ROM
			/* TODO */
			break;
		case 0xA: // 0xA0-AF ??? Don't appear to have any effect with Scud Race ROM
			/* TODO */
			break;
		case 0xB: // 0xB0-BF Invalid command (reserved for use by PPC to send cabinet type 0xB0 or 0xB1 during initialization)
			/* Ignore */
			break;
		case 0xC: // 0xC0-CF Set board mode (0xCB = reset board)
			SendStopAll();
			if (val >= 0xB)
			{
				// Reset board
				m_initialized = false;
				m_initState = 0;
			}
			else
				m_boardMode = val;      
			break;
		case 0xD: // 0xD0-DF Set read mode
			m_readMode = val&0x7;      
			break;
		case 0xE: // 0xE0-EF Invalid command
			/* Ignore */
			break;
		case 0xF: // 0xF0-FF Echo test
			m_echoVal = val;
			break;
	}	
}

void CDriveBoard::RunFrame(void)
{
	if (m_simulated)
		SimulateFrame();
	else
		EmulateFrame();
}

void CDriveBoard::SimulateFrame(void)
{
	if (!m_initialized)
		m_initState++;
	// TODO - update m_statusFlags and play preset scripts according to board mode
}	

void CDriveBoard::EmulateFrame(void)
{
	// Assuming Z80 runs @ 4.0MHz and NMI triggers @ 60.0KHz
	// TODO - find out if Z80 frequency is correct and exact frequency of NMI interrupts (just guesswork at the moment!)
	int cycles     = 4.0 * 1000000 / 60;
	int loopCycles = 10000;
	while (cycles > 0)
	{
		if (m_allowInterrupts)
			m_z80.TriggerNMI();
		cycles -= m_z80.Run(min<int>(loopCycles, cycles));
	}
}

UINT8 CDriveBoard::Read8(UINT32 addr)
{
	// TODO - shouldn't end of ROM be 0x7FFF not 0x8FFF?
	if (addr < 0x9000)        // ROM is 0x0000-0x8FFF
		return m_rom[addr];
	else if (addr >= 0xE000)  // RAM is 0xE000-0xFFFF
		return m_ram[(addr-0xE000)&0x1FFF];
	else
	{
		//printf("Unhandled Z80 read of %08X (at PC = %04X)\n", addr, m_z80.GetPC());
		return 0xFF;
	}
}

void CDriveBoard::Write8(UINT32 addr, UINT8 data)
{
	if (addr >= 0xE000)  // RAM is 0xE000-0xFFFF
		m_ram[(addr-0xE000)&0x1FFF] = data;
#ifdef DEBUG
	else
		printf("Unhandled Z80 write to %08X (at PC = %04X)\n", addr, m_z80.GetPC());
#endif
}

UINT8 CDriveBoard::IORead8(UINT32 portNum)
{
	UINT8 adcVal;
	switch (portNum)
	{
		case 32: // DIP 1 value
			return m_dip1;
		case 33: // DIP 2 value
			return m_dip2;
		case 36: // ADC channel 1 - not connected
		case 37: // ADC channel 2 - steering wheel position (0x00 = full left, 0x80 = center, 0xFF = full right)
		case 38: // ADC channel 3 - cockpit bank position (deluxe cabinets) (0x00 = full left, 0x80 = center, 0xFF = full right)
		case 39: // ADC channel 4 - not connected
			if (portNum == m_adcPortRead && m_adcPortBit-- > 0)
			{
				switch (portNum)
				{
					case 36: //	Not connected
						adcVal = 0x00;
						break;
					case 37: // Steering wheel for twin racing cabinets - TODO - check actual range of steering, suspect it is not really 0x00-0xFF
						if (m_initialized)
							adcVal = (UINT8)m_inputs->steering->value;
						else
							adcVal = 0x80; // If not initialized, return 0x80 so that wheel centering test does not fail
						break;
					case 38: // Cockpit bank position for deluxe racing cabinets
						adcVal = 0x80;
						break;
					case 39: // Not connected
						adcVal = 0x00;
						break;
					default:
#ifdef DEBUG						
						printf("Unhandled Z80 input on ADC port %u (at PC = %04X)\n", portNum, m_z80.GetPC());
#endif
						return 0xFF;
				}
				return (adcVal>>m_adcPortBit)&0x01;
			}
			else
			{
#ifdef DEBUG
				printf("Unhandled Z80 input on ADC port %u (at PC = %04X)\n", portNum, m_z80.GetPC());
#endif
				return 0xFF;
			}
		case 40: // PPC command
			return m_dataSent;
		case 44: // Encoder error reporting (kept at 0x00 for no error)
			// Bit 1 0	
			//     0 0 = encoder okay, no error
			//     0 1 = encoder error 1 - overcurrent error
			//     1 0 = encoder error 2 - overheat error
			//     1 1 = encoder error 3 - encoder error, reinitializes board
			return 0x00;
		default:
#ifdef DEBUG
			printf("Unhandled Z80 input on port %u (at PC = %04X)\n", portNum, m_z80.GetPC());
#endif
			return 0xFF;
	}
}

void CDriveBoard::IOWrite8(UINT32 portNum, UINT8 data)
{
	switch (portNum)
	{
		case 16: // Unsure? - single byte 0x03 sent at initialization, then occasionally writes 0x07 & 0xFA to port
			return;
		case 17: // Interrupt control
			if (data == 0x57)
				m_allowInterrupts = true;
			else if (data == 0x53) // Strictly speaking 0x53 then 0x04
				m_allowInterrupts = false;
			return;
		case 28: // Unsure? - two bytes 0xFF, 0xFF sent at initialization only
		case 29: // Unsure? - two bytes 0x0F, 0x17 sent at initialization only
		case 30: // Unsure? - same as port 28
		case 31: // Unsure? - same as port 31
			return;
		case 32: // Left digit of 7-segment display 1
			m_seg1Digit1 = data;
			return;
		case 33: // Right digit of 7-segment display 1
			m_seg1Digit2 = data;
			return;
		case 34: // Left digit of 7-segment display 2
			m_seg2Digit1 = data;
			return;
		case 35: // Right digit of 7-segment display 2
			m_seg2Digit2 = data;
			return;
		case 36: // ADC channel 1 control
		case 37: // ADC channel 2 control
		case 38: // ADC channel 3 control
		case 39: // ADC channel 4 control
			m_adcPortRead = portNum;
			m_adcPortBit = 8;
			return;
		case 41: // Reply for PPC
			m_dataReceived = data;
			if (data == 0xCC)
				m_initialized = true;
			return;
		case 42: // Encoder motor data
			m_port42Out = data;
			ProcessEncoderCmd();
			return;
		case 45: // Clutch/lamp control (deluxe cabinets)
			return;
		case 46: // Encoder motor control
			m_port46Out = data;
			return;
		case 240: // Unsure? - single byte 0xBB sent at initialization only
			return;
		case 241: // Unsure? - single byte 0x4E sent regularly - some sort of watchdog?
			return;
		default:
#ifdef DEBUG
			printf("Unhandled Z80 output on port %u (at PC = %04X)\n", portNum, m_z80.GetPC());
#endif
			return;
	}
}

void CDriveBoard::ProcessEncoderCmd(void)
{
	if (m_prev42Out != m_port42Out || m_prev46Out != m_port46Out)
	{
		//printf("46 [%02X] / 42 [%02X]\n", m_port46Out, m_port42Out);
		switch (m_port46Out)
		{
			case 0xFB:
				// TODO - friction?  Sent during power slide.  0xFF = strongest or 0x00?
				//SendFriction(m_port42Out);
				break;

			case 0xFC:
				// Centering / uncentering (vibrate)
				// Bit 2 = on for centering, off for uncentering
				if (m_port42Out&0x04)
				{
					// Centering
					// Bit 7 = on for disable, off for enable
					if (m_port42Out&0x80)
					{
						// Disable centering
						SendSelfCenter(0);
					}
					else
					{
						// Bits 3-6 = centering strength 0x0-0xF.  This is scaled to range 0x0F-0xFF
						UINT8 strength = ((m_port42Out&0x78)>>3) * 0x10 + 0xF;
						SendSelfCenter(strength); 
					}
				}
				else
				{
					// Uncentering
					// Bits 0-1 = data sequence number 0-3
					UINT8 seqNum = m_port42Out&0x03;
					// Bits 4-7 = data values
					UINT16 data = (m_port42Out&0xF0)>>4;
					switch (seqNum)
					{
						case 0: m_uncenterVal1 = data<<4; break;
						case 1: m_uncenterVal1 |= data;   break;
						case 2: m_uncenterVal2 = data<<4; break;
						case 3: m_uncenterVal2 |= data;   break;
					}
					if (seqNum == 0 && m_uncenterVal1 == 0)
					{
						// Disable uncentering
						SendVibrate(0);
					}
					else if (seqNum == 3 && m_uncenterVal1 > 0)
					{
						// Uncentering - unsure exactly how values sent map to strength or whether they specify some other attributes of effect
						// For now just attempting to map them to a sensible value in range 0x00-0xFF
						UINT8 strength = ((m_uncenterVal1>>1) - 7) * 0x50 + ((m_uncenterVal2>>1) - 5) * 0x10 + 0xF;
						SendVibrate(strength);
					}
				}
				break;
		
			case 0xFD:
				// TODO - unsure?  Sent as velocity changes, similar to self-centering
				break;
		
			case 0xFE:
				// Apply constant force to wheel
				// Value is: 0x80 = stop motor, 0x81-0xC0 = roll wheel left, 0x40-0x7F = roll wheel right, scale to range -0x80-0x7F
				// Note: seems to often output 0x7F or 0x81 for stop motor, so narrowing wheel ranges to 0x40-0x7E and 0x82-0xC0
				if (m_port42Out > 0x81)
				{
					if (m_port42Out <= 0xC0)
						SendConstantForce(2 * (0x81 - m_port42Out));	
					else
						SendConstantForce(-0x80);
				}
				else if (m_port42Out < 0x7F)
				{
					if (m_port42Out >= 0x40)
						SendConstantForce(2 * (0x7F - m_port42Out));
					else
						SendConstantForce(0x7F);
				}
				else
					SendConstantForce(0);
				break;
		
			case 0xFF:
				// Stop all effects
				if (m_port42Out == 0)
					SendStopAll();
				break;

			default:
				//printf("Unknown = 46 [%02X] / 42 [%02X]\n", m_port46Out, m_port42Out);
				break;
		}

		m_prev42Out = m_port42Out;
		m_prev46Out = m_port46Out;
	}
}

void CDriveBoard::SendStopAll(void)
{
	//printf(">> Stop All Effects\n");

	ForceFeedbackCmd ffCmd;
	ffCmd.id = FFStop;
	m_inputs->steering->SendForceFeedbackCmd(ffCmd);

	m_lastConstForce = 0;
	m_lastSelfCenter = 0;
	m_lastFriction   = 0;
	m_lastVibrate    = 0;
}

void CDriveBoard::SendConstantForce(INT8 val)
{
	if (val == m_lastConstForce)
		return;
	/*
	if (val > 0)
	{
		printf(">> Force Right %02X [%8s", val, "");
		for (unsigned i = 0; i < 8; i++)
			printf(i == 0 || i <= (val + 1) / 16 ? ">" : " ");
		printf("]\n");
	}
	else if (val < 0)
	{
		printf(">> Force Left  %02X [", -val);
		for (unsigned i = 0; i < 8; i++)
			printf(i == 7 || i >= (val + 128) / 16 ? "<" : " ");
		printf("%8s]\n", "");
	}
	else
		printf(">> Stop Force     [%16s]\n", "");
	*/
	
	ForceFeedbackCmd ffCmd;
	ffCmd.id = FFConstantForce;			
	ffCmd.force = (float)val / (val >= 0 ? 127.0f : 128.0f);
	m_inputs->steering->SendForceFeedbackCmd(ffCmd);

	m_lastConstForce = val;
}

void CDriveBoard::SendSelfCenter(UINT8 val)
{
	if (val == m_lastSelfCenter)
		return;
	/*
	if (val == 0)
		printf(">> Stop Self-Center\n");
	else
		printf(">> Self-Center %02X\n", val);
	*/
	
	ForceFeedbackCmd ffCmd;
	ffCmd.id = FFSelfCenter;
	ffCmd.force = (float)val / 255.0f;
	m_inputs->steering->SendForceFeedbackCmd(ffCmd);

	m_lastSelfCenter = val;
}

void CDriveBoard::SendFriction(UINT8 val)
{
	if (val == m_lastFriction)
		return;
	/*
	if (val == 0)
		printf(">> Stop Friction\n");
	else
		printf(">> Friction %02X\n", val);
	*/
	
	ForceFeedbackCmd ffCmd;
	ffCmd.id = FFFriction;
	ffCmd.force = (float)val / 255.0f;
	m_inputs->steering->SendForceFeedbackCmd(ffCmd);

	m_lastFriction = val;
}

void CDriveBoard::SendVibrate(UINT8 val)
{
	if (val == m_lastVibrate)
		return;
	/*
	if (val == 0)
		printf(">> Stop Vibrate\n");
	else
		printf(">> Vibrate %02X\n", val);
	*/

	ForceFeedbackCmd ffCmd;
	ffCmd.id = FFVibrate;
	ffCmd.force = (float)val / 255.0f;
	m_inputs->steering->SendForceFeedbackCmd(ffCmd);

	m_lastVibrate = val;
}

CDriveBoard::CDriveBoard() : m_attached(false), m_tmpDisabled(false), m_simulated(false),
	m_rom(NULL), m_ram(NULL), m_inputs(NULL), m_dip1(0xCF), m_dip2(0xFF)
{
	DebugLog("Built Drive Board\n");
}

CDriveBoard::~CDriveBoard(void)
{	
	if (m_ram != NULL)
	{
		delete[] m_ram;
		m_ram = NULL;
	}
	m_rom = NULL;
	m_inputs = NULL;

	DebugLog("Destroyed Drive Board\n");
}
