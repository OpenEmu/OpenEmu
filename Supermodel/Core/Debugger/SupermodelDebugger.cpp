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
 * SupermodelDebugger.cpp
 */

#ifdef SUPERMODEL_DEBUGGER

#include "Supermodel.h"

#include "ConsoleDebugger.h"
#include "CPUDebug.h"
#include "Label.h"

#include <cstdio>
#include <string>

namespace Debugger
{
	static const char *s_giGroup = "Game Inputs";
	static const char *s_gmGroup = "Game Inputs (Mirrored)";
	static const char *s_sbGroup = "Sound Board";

	static char s_mSlotStr[32][20];
	static char s_sSlotStr[32][20];
		
	CCPUDebug *CSupermodelDebugger::CreateMainBoardCPUDebug(::CModel3 *model3)
	{
		CPPCDebug *cpu = new CPPCDebug("MainPPC");

		// Interrupts
		cpu->AddInterrupt("VD0", 0, "Unknown video-related");
		cpu->AddInterrupt("VBL", 1, "VBlank start");
		cpu->AddInterrupt("VD2", 2, "Unknown video-related");
		cpu->AddInterrupt("VD3", 3, "Unknown video-related");
		cpu->AddInterrupt("NET", 4, "Network");
		cpu->AddInterrupt("UN5", 5, "Unknown");
		cpu->AddInterrupt("SND", 6, "SCSP (sound)");
		cpu->AddInterrupt("UN7", 7, "Unknown");

		// Memory regions
		cpu->AddRegion(0x00000000, 0x007FFFFF, true,  false, "RAM");
		cpu->AddRegion(0x84000000, 0x8400003F, false, false, "Real3D Status Registers");
		cpu->AddRegion(0x88000000, 0x88000007, false, false, "Real3D Command Port");
		cpu->AddRegion(0x8C000000, 0x8C3FFFFF, false, false, "Real3D Culling RAM (Low)");
		cpu->AddRegion(0x8E000000, 0x8E0FFFFF, false, false, "Real3D Culling RAM (High)");
		cpu->AddRegion(0x90000000, 0x9000000B, false, false, "Real3D VROM Texture Port");
		cpu->AddRegion(0x94000000, 0x940FFFFF, false, false, "Real3D Texture FIFO");
		cpu->AddRegion(0x98000000, 0x980FFFFF, false, false, "Real3D Polygon RAM");
		cpu->AddRegion(0xC0000000, 0xC00000FF, false, false, "SCSI (Step 1.x)");
		cpu->AddRegion(0xC1000000, 0xC10000FF, false, false, "SCSI (Step 1.x) (Lost World expects it here)");
		cpu->AddRegion(0xC2000000, 0xC20000FF, false, false, "Real3D DMA (Step 2.x)");
		cpu->AddRegion(0xF0040000, 0xF004003F, false, false, "Input (Controls) Registers");
		cpu->AddRegion(0xF0080000, 0xF0080007, false, false, "Sound Board Registers");
		cpu->AddRegion(0xF00C0000, 0xF00DFFFF, false, false, "Backup RAM");
		cpu->AddRegion(0xF0100000, 0xF010003F, false, false, "System Registers");
		cpu->AddRegion(0xF0140000, 0xF014003F, false, false, "Real, 0xTime Clock");
		cpu->AddRegion(0xF0180000, 0xF019FFFF, false, false, "Security Board RAM");
		cpu->AddRegion(0xF01A0000, 0xF01A003F, false, false, "Security Board Registers");
		cpu->AddRegion(0xF0800CF8, 0xF0800CFF, false, false, "MPC105 CONFIG_cpu->AddR (Step 1.x)");
		cpu->AddRegion(0xF0C00CF8, 0xF0C00CFF, false, false, "MPC105 CONFIG_DATA (Step 1.x)");
		cpu->AddRegion(0xF1000000, 0xF10F7FFF, false, false, "Tile Generator Pattern Table");
		cpu->AddRegion(0xF10F8000, 0xF10FFFFF, false, false, "Tile Generator Name Table");
		cpu->AddRegion(0xF1100000, 0xF111FFFF, false, false, "Tile Generator Palette");
		cpu->AddRegion(0xF1180000, 0xF11800FF, false, false, "Tile Generator Registers");
		cpu->AddRegion(0xF8FFF000, 0xF8FFF0FF, false, false, "MPC105 (Step 1.x) or MPC106 (Step 2.x) Registers");
		cpu->AddRegion(0xF9000000, 0xF90000FF, false, false, "NCR 53C810 Registers (Step 1.x?)");
		cpu->AddRegion(0xFE040000, 0xFE04003F, false, false, "Mirrored Input Registers");
		cpu->AddRegion(0xFEC00000, 0xFEDFFFFF, false, false, "MPC106 CONFIG_cpu->AddR (Step 2.x)");
		cpu->AddRegion(0xFEE00000, 0xFEFFFFFF, false, false, "MPC106 CONFIG_DATA (Step 2.x)");
		cpu->AddRegion(0xFF000000, 0xFF7FFFFF, true,  true,  "Banked CROM (CROMxx)");
		cpu->AddRegion(0xFF800000, 0xFFFFFFFF, true,  true,  "Fixed CROM");

		// Memory-mapped IO
		cpu->AddMappedIO(0xF0040000, 1, "Input Bank Select",      s_giGroup);
		cpu->AddMappedIO(0xF0040004, 1, "Current Input Bank",     s_giGroup);
		cpu->AddMappedIO(0xF0040008, 1, "Game Specific Inputs 1", s_giGroup);
		cpu->AddMappedIO(0xF004000C, 1, "Game Specific Inputs 2", s_giGroup);
		cpu->AddMappedIO(0xF0040010, 1, "Drive Board",            s_giGroup);
		cpu->AddMappedIO(0xF0040014, 1, "LED Outputs?",           s_giGroup);
		cpu->AddMappedIO(0xF0040018, 1, "Unknown?",               s_giGroup);
		cpu->AddMappedIO(0xF0040024, 1, "Serial FIFO 1 Control",  s_giGroup);
		cpu->AddMappedIO(0xF0040028, 1, "Serial FIFO 2 Control",  s_giGroup);
		cpu->AddMappedIO(0xF004002C, 1, "Serial FIFO 1",          s_giGroup);
		cpu->AddMappedIO(0xF0040030, 1, "Serial FIFO 2",          s_giGroup);
		cpu->AddMappedIO(0xF0040034, 1, "Serial FIFO Flags",      s_giGroup);
		cpu->AddMappedIO(0xF004003C, 1, "ADC",                    s_giGroup);

		cpu->AddMappedIO(0xFE040000, 1, "Input Bank Select",      s_gmGroup);
		cpu->AddMappedIO(0xFE040004, 1, "Current Input Bank",     s_gmGroup);
		cpu->AddMappedIO(0xFE040008, 1, "Game Specific Inputs 1", s_gmGroup);
		cpu->AddMappedIO(0xFE04000C, 1, "Game Specific Inputs 2", s_gmGroup);
		cpu->AddMappedIO(0xFE040010, 1, "Drive Board",            s_gmGroup);
		cpu->AddMappedIO(0xFE040014, 1, "LED Outputs?",           s_gmGroup);
		cpu->AddMappedIO(0xFE040018, 1, "Unknown?",               s_gmGroup);
		cpu->AddMappedIO(0xFE040024, 1, "Serial FIFO 1 Control",  s_gmGroup);
		cpu->AddMappedIO(0xFE040028, 1, "Serial FIFO 2 Control",  s_gmGroup);
		cpu->AddMappedIO(0xFE04002C, 1, "Serial FIFO 1",          s_gmGroup);
		cpu->AddMappedIO(0xFE040030, 1, "Serial FIFO 2",          s_gmGroup);
		cpu->AddMappedIO(0xFE040034, 1, "Serial FIFO Flags",      s_gmGroup);
		cpu->AddMappedIO(0xFE04003C, 1, "ADC",                    s_gmGroup);

		cpu->AddMappedIO(0xF0080000, 1, "MIDI",    s_sbGroup);
		cpu->AddMappedIO(0xF0080004, 1, "Control", s_sbGroup);

		return cpu;
	}

	CCPUDebug *CSupermodelDebugger::CreateSoundBoardCPUDebug(::CModel3 *model3)
	{
		CSoundBoard *sndBrd = model3->GetSoundBoard();
		CMusashi68KDebug *cpu = new CMusashi68KDebug("Snd68K", sndBrd->GetM68K());

		// Regions
		cpu->AddRegion(0x000000, 0x0FFFFF, true,  false, "SCSP1 RAM");
		cpu->AddRegion(0x200000, 0x2FFFFF, true,  false, "SCSP2 RAM");
		cpu->AddRegion(0x600000, 0x67FFFF, true,  true,  "Program ROM");
		cpu->AddRegion(0x800000, 0x9FFFFF, false, true,  "Sample ROM (low 2 MB)");
		cpu->AddRegion(0xA00000, 0xDFFFFF, false, true,  "Sample ROM (bank)");
		cpu->AddRegion(0xE00000, 0xFFFFFF, false, true,  "Sample ROM (bank)");
		cpu->AddRegion(0x100000, 0x10FFFF, false, false, "SCSP Master");
		cpu->AddRegion(0x300000, 0x30FFFF, false, false, "SCSP Slave");

		// Mapped I/O

		// SCSP Master 32 slots
		for (unsigned slot = 0; slot < 32; slot++)
		{
			sprintf(s_mSlotStr[slot], "SCSP Master Slot %02X", slot);
			UINT32 addr = 0x100000 + slot * 0x20;
			cpu->AddMappedIO(addr + 0x00, 2, "KYONX,KYONB,SBCTL,SSCTL,LPCTL,PCM8B,SA", s_mSlotStr[slot]);
			cpu->AddMappedIO(addr + 0x02, 2, "SA",                                     s_mSlotStr[slot]);
			cpu->AddMappedIO(addr + 0x04, 2, "LSA",                                    s_mSlotStr[slot]);
			cpu->AddMappedIO(addr + 0x06, 2, "LEA",                                    s_mSlotStr[slot]);
			cpu->AddMappedIO(addr + 0x08, 2, "D2R,D1R,EGHOLD,AR",                      s_mSlotStr[slot]);
			cpu->AddMappedIO(addr + 0x0A, 2, "LPSLNK,KRS,DL,RR",                       s_mSlotStr[slot]);
			cpu->AddMappedIO(addr + 0x0C, 1, "STWINH",                                 s_mSlotStr[slot]);
			cpu->AddMappedIO(addr + 0x0D, 1, "SDIR,TL",                                s_mSlotStr[slot]);
			cpu->AddMappedIO(addr + 0x0E, 2, "MDL,MDXSL,MDYSL",                        s_mSlotStr[slot]);
			cpu->AddMappedIO(addr + 0x10, 2, "OCT,FNS",                                s_mSlotStr[slot]);
			cpu->AddMappedIO(addr + 0x12, 1, "LFORE,LFOF,PLFOWS",                      s_mSlotStr[slot]);
			cpu->AddMappedIO(addr + 0x13, 1, "PLFOS,ALFOWS,ALFOS",                     s_mSlotStr[slot]);
			cpu->AddMappedIO(addr + 0x15, 1, "ISEL,OMXL",                              s_mSlotStr[slot]);
			cpu->AddMappedIO(addr + 0x16, 1, "DISDL,DIPAN",                            s_mSlotStr[slot]);
			cpu->AddMappedIO(addr + 0x17, 1, "EFSDL,EFPAN",                            s_mSlotStr[slot]);
		}
		// SCSP Master control registers
		const char *masterCtl = "SCSP Master Control Registers";
		cpu->AddMappedIO(0x100400, 1, "MEM4MB,DAC18B",                   masterCtl);
		cpu->AddMappedIO(0x100401, 1, "VER,MVOL",                        masterCtl);
		cpu->AddMappedIO(0x100402, 2, "RBL,RBP",                         masterCtl);
		cpu->AddMappedIO(0x100404, 1, "MOFULL,MOEMP,MIOVF,MIFULL,MIEMP", masterCtl);
		cpu->AddMappedIO(0x100405, 1, "MIBUF",                           masterCtl);
		cpu->AddMappedIO(0x100407, 1, "MOBUF",                           masterCtl);
		cpu->AddMappedIO(0x100408, 2, "MSLC,CA",                         masterCtl);
		cpu->AddMappedIO(0x100412, 2, "DMEAL",                           masterCtl);
		cpu->AddMappedIO(0x100414, 2, "DMEAH,DRGA",                      masterCtl);
		cpu->AddMappedIO(0x100416, 2, "DGATE,DDIR,DEXE,DTLG",            masterCtl);
		cpu->AddMappedIO(0x100418, 1, "TACTL",                           masterCtl);
		cpu->AddMappedIO(0x100419, 1, "TIMA",                            masterCtl);
		cpu->AddMappedIO(0x10041A, 1, "TBCTL",                           masterCtl);
		cpu->AddMappedIO(0x10041B, 1, "TIMB",                            masterCtl);
		cpu->AddMappedIO(0x10041C, 1, "TCCTL",                           masterCtl);
		cpu->AddMappedIO(0x10041D, 1, "TIMC",                            masterCtl);
		cpu->AddMappedIO(0x10041E, 2, "SCIEB",                           masterCtl);
		cpu->AddMappedIO(0x100420, 2, "SCIPD",                           masterCtl);
		cpu->AddMappedIO(0x100422, 2, "SCIRE",                           masterCtl);
		cpu->AddMappedIO(0x100425, 1, "SCILV0",                          masterCtl);
		cpu->AddMappedIO(0x100427, 1, "SCILV1",                          masterCtl);
		cpu->AddMappedIO(0x100429, 1, "SCILV2",                          masterCtl);
		cpu->AddMappedIO(0x10042A, 2, "MCIEB",                           masterCtl);
		cpu->AddMappedIO(0x10042C, 2, "MCIPD",                           masterCtl);
		cpu->AddMappedIO(0x10042E, 2, "MCIRE",                           masterCtl);

		// SCSP Slave 32 slots
		for (unsigned slot = 0; slot < 32; slot++)
		{
			sprintf(s_sSlotStr[slot], "SCSP Slave Slot %02X", slot);
			UINT32 addr = 0x300000 + slot * 0x20;
			cpu->AddMappedIO(addr + 0x00, 2, "KYONX,KYONB,SBCTL,SSCTL,LPCTL,PCM8B,SA", s_sSlotStr[slot]);
			cpu->AddMappedIO(addr + 0x02, 2, "SA",                                     s_sSlotStr[slot]);
			cpu->AddMappedIO(addr + 0x04, 2, "LSA",                                    s_sSlotStr[slot]);
			cpu->AddMappedIO(addr + 0x06, 2, "LEA",                                    s_sSlotStr[slot]);
			cpu->AddMappedIO(addr + 0x08, 2, "D2R,D1R,EGHOLD,AR",                      s_sSlotStr[slot]);
			cpu->AddMappedIO(addr + 0x0A, 2, "LPSLNK,KRS,DL,RR",                       s_sSlotStr[slot]);
			cpu->AddMappedIO(addr + 0x0C, 1, "STWINH",                                 s_sSlotStr[slot]);
			cpu->AddMappedIO(addr + 0x0D, 1, "SDIR,TL",                                s_sSlotStr[slot]);
			cpu->AddMappedIO(addr + 0x0E, 2, "MDL,MDXSL,MDYSL",                        s_sSlotStr[slot]);
			cpu->AddMappedIO(addr + 0x10, 2, "OCT,FNS",                                s_sSlotStr[slot]);
			cpu->AddMappedIO(addr + 0x12, 1, "LFORE,LFOF,PLFOWS",                      s_sSlotStr[slot]);
			cpu->AddMappedIO(addr + 0x13, 1, "PLFOS,ALFOWS,ALFOS",                     s_sSlotStr[slot]);
			cpu->AddMappedIO(addr + 0x15, 1, "ISEL,OMXL",                              s_sSlotStr[slot]);
			cpu->AddMappedIO(addr + 0x16, 1, "DISDL,DIPAN",                            s_sSlotStr[slot]);
			cpu->AddMappedIO(addr + 0x17, 1, "EFSDL,EFPAN",                            s_sSlotStr[slot]);
		}

		// SCSP Master control registers
		const char *slaveCtl = "SCSP Slave Control Registers";
		cpu->AddMappedIO(0x300400, 1, "MEM4MB,DAC18B",                   slaveCtl);
		cpu->AddMappedIO(0x300401, 1, "VER,MVOL",                        slaveCtl);
		cpu->AddMappedIO(0x300402, 2, "RBL,RBP",                         slaveCtl);
		cpu->AddMappedIO(0x300404, 1, "MOFULL,MOEMP,MIOVF,MIFULL,MIEMP", slaveCtl);
		cpu->AddMappedIO(0x300405, 1, "MIBUF",                           slaveCtl);
		cpu->AddMappedIO(0x300407, 1, "MOBUF",                           slaveCtl);
		cpu->AddMappedIO(0x300408, 2, "MSLC,CA",                         slaveCtl);
		cpu->AddMappedIO(0x300412, 2, "DMEAL",                           slaveCtl);
		cpu->AddMappedIO(0x300414, 2, "DMEAH,DRGA",                      slaveCtl);
		cpu->AddMappedIO(0x300416, 2, "DGATE,DDIR,DEXE,DTLG",            slaveCtl);
		cpu->AddMappedIO(0x300418, 1, "TACTL",                           slaveCtl);
		cpu->AddMappedIO(0x300419, 1, "TIMA",                            slaveCtl);
		cpu->AddMappedIO(0x30041A, 1, "TBCTL",                           slaveCtl);
		cpu->AddMappedIO(0x30041B, 1, "TIMB",                            slaveCtl);
		cpu->AddMappedIO(0x30041C, 1, "TCCTL",                           slaveCtl);
		cpu->AddMappedIO(0x30041D, 1, "TIMC",                            slaveCtl);
		cpu->AddMappedIO(0x30041E, 2, "SCIEB",                           slaveCtl);
		cpu->AddMappedIO(0x300420, 2, "SCIPD",                           slaveCtl);
		cpu->AddMappedIO(0x300422, 2, "SCIRE",                           slaveCtl);
		cpu->AddMappedIO(0x300425, 1, "SCILV0",                          slaveCtl);
		cpu->AddMappedIO(0x300427, 1, "SCILV1",                          slaveCtl);
		cpu->AddMappedIO(0x300429, 1, "SCILV2",                          slaveCtl);
		cpu->AddMappedIO(0x30042A, 2, "MCIEB",                           slaveCtl);
		cpu->AddMappedIO(0x30042C, 2, "MCIPD",                           slaveCtl);
		cpu->AddMappedIO(0x30042E, 2, "MCIRE",                           slaveCtl);

		return cpu;
	}

	CCPUDebug *CSupermodelDebugger::CreateDSBCPUDebug(::CModel3 *model3)
	{
		CSoundBoard *sndBrd = model3->GetSoundBoard();
		CDSB *dsb = sndBrd->GetDSB();
		
		CDSB1 *dsb1 = dynamic_cast<CDSB1*>(dsb);
		if (dsb1 != NULL)
		{
			CZ80Debug *cpu = new CZ80Debug("DSBZ80", dsb1->GetZ80());
			
			// Regions
			cpu->AddRegion(0x0000, 0x7FFF, true,  true,  "ROM");
			cpu->AddRegion(0x8000, 0xFFFF, false, false, "RAM");

			// TODO - rename some I/O ports
			return cpu;
		}
		
		CDSB2 *dsb2 = dynamic_cast<CDSB2*>(dsb);
		if (dsb2 != NULL)
		{
			CMusashi68KDebug *cpu = new CMusashi68KDebug("DSB68K", dsb2->GetM68K());
			
			// Regions
			cpu->AddRegion(0x000000, 0x020000, true,  true,  "ROM");
			cpu->AddRegion(0xF00000, 0xF10000, false, false, "RAM");

			// TODO - memory mapped I/O
			return cpu;
		}

		return NULL;
	}

	CCPUDebug *CSupermodelDebugger::CreateDriveBoardCPUDebug(::CModel3 *model3)
	{
		CDriveBoard *drvBrd = model3->GetDriveBoard();
		if (!drvBrd->IsAttached())
			return NULL;
		CZ80Debug *cpu = new CZ80Debug("DrvZ80", drvBrd->GetZ80());
		
		// Regions
		cpu->AddRegion(0x0000, 0x7FFF, true,  true,  "ROM");
		cpu->AddRegion(0xE000, 0xFFFF, false, false, "RAM");

		// TODO - rename some I/O ports
		return cpu;
	}

	CSupermodelDebugger::CSupermodelDebugger(::CModel3 *model3, ::CInputs *inputs, ::CLogger *logger) : 
		CConsoleDebugger(), m_model3(model3), m_inputs(inputs), m_logger(logger), 
		m_loadEmuState(false), m_saveEmuState(false), m_resetEmu(false)
	{
		//
	}

	void CSupermodelDebugger::AddCPUs()
	{
		CCPUDebug *cpu;

		// Add main board CPU
		cpu = CreateMainBoardCPUDebug(m_model3);
		if (cpu) AddCPU(cpu);
		
		// Add sound board CPU (if attached)
		cpu = CreateSoundBoardCPUDebug(m_model3);
		if (cpu) AddCPU(cpu);

		// Add sound daughter board CPU (if attached)
		cpu = CreateDSBCPUDebug(m_model3);
		if (cpu) AddCPU(cpu);
		
		// Add drive board CPU (if attached)
		cpu = CreateDriveBoardCPUDebug(m_model3);
		if (cpu) AddCPU(cpu);
	}

	void CSupermodelDebugger::WaitCommand(CCPUDebug *cpu)
	{
		// Ungrab mouse and disable audio
		m_inputs->GetInputSystem()->UngrabMouse();
		SetAudioEnabled(false);

		CConsoleDebugger::WaitCommand(cpu);

		m_inputs->GetInputSystem()->GrabMouse();
		SetAudioEnabled(true);
	}

	bool CSupermodelDebugger::ProcessToken(const char *token, const char *cmd)
	{
		//
		// Emulator
		//
		if (CheckToken(token, "les", "loademustate"))				// loademustate <filename>
		{
			// Parse arguments
			token = strtok(NULL, " ");
			if (token == NULL)
			{
				Print("Missing filename.\n");
				return false;
			}

			strncpy(m_stateFile, token, 254);
			m_stateFile[254] = '\0';
			m_loadEmuState = true;
			return true;
		}
		else if (CheckToken(token, "ses", "saveemustate"))			// saveemustate  <filename>
		{
			// Parse arguments
			token = strtok(NULL, " ");
			if (token == NULL)
			{
				Print("Missing filename.\n");
				return false;
			}

			strncpy(m_stateFile, token, 254);
			m_stateFile[254] = '\0';
			m_saveEmuState = true;
			return true;
		}
		else if (CheckToken(token, "res", "resetemu"))				// resetemu
		{
			m_resetEmu = true;
			return true;
		}
		//
		// Inputs
		//
		else if (CheckToken(token, "lip", "listinputs"))			// listinputs
		{
			ListInputs();
			return false;
		}
		else if (CheckToken(token, "pip", "printinput"))			// printinput (<id>|<label>)
		{
			// Parse arguments
			token = strtok(NULL, " ");
			if (token == NULL)
			{
				Print("Mising input name.\n");
				return false;
			}
			::CInput *input = (*m_inputs)[token];
			if (input == NULL)
			{
				Print("No input with id or label '%s'.\n", token);
				return false;
			}
			if (!InputIsValid(input))
			{
				Print("Input '%s' is not valid for current game.\n", token);
				return false;
			}

			if (!input->IsVirtual())
			{
				char mapTrunc[41];
				Truncate(mapTrunc, 40, input->GetMapping());
				Print("Input %s (%s) [%s] = %04X (%d)\n", input->id, input->label, mapTrunc, input->value, input->value);
			}
			else
				Print("Input %s (%s) = %04X (%d)\n", input->id, input->label, input->value, input->value);
			return false;
		}
		else if (CheckToken(token, "sip", "setinput"))				// setinput (<id>|<label>) <mapping>
		{
			// Parse arguments
			token = strtok(NULL, " ");
			if (token == NULL)
			{
				Print("Mising input id or label.\n");
				return false;
			}
			::CInput *input = (*m_inputs)[token];
			if (input == NULL)
			{
				Print("No input with id or label '%s'.\n", token);
				return false;
			}
			if (!InputIsValid(input))
			{
				Print("Input '%s' is not valid for current game.\n", token);
				return false;
			}
			token = strtok(NULL, " ");
			if (token == NULL)
			{
				Print("Missing mapping to set.\n");
				return false;
			}

			input->SetMapping(token);

			Print("Set input %s (%s) to [%s]\n", input->id, input->label, input->GetMapping());
			return false;
		}
		else if (CheckToken(token, "rip", "resetinput"))            // resetinput (<id>|<label>)
		{
			// Parse arguments
			token = strtok(NULL, " ");
			if (token == NULL)
			{
				Print("Mising input id or label.\n");
				return false;
			}
			::CInput *input = (*m_inputs)[token];
			if (input == NULL)
			{
				Print("No input with id or label '%s'.\n", token);
				return false;
			}
			if (!InputIsValid(input))
			{
				Print("Input '%s' is not valid for current game.\n", token);
				return false;
			}

			input->ResetToDefaultMapping();

			Print("Reset input %s (%s) to [%s]\n", input->id, input->label, input->GetMapping());
			return false;
		}
		else if (CheckToken(token, "cip", "configinput"))			// configinput (<id>|<label>) [(s)et|(a)ppend]
		{
			// Parse arguments
			token = strtok(NULL, " ");
			if (token == NULL)
			{
				Print("Mising input id or label.\n");
				return false;
			}
			CInput *input = (*m_inputs)[token];
			if (input == NULL)
			{
				Print("No input with id or label '%s'.\n", token);
				return false;
			}
			if (!InputIsValid(input))
			{
				Print("Input '%s' is not valid for current game.\n", token);
				return false;
			}
			token = strtok(NULL, " ");
			bool append;
			if (token == NULL || CheckToken(token, "s", "set"))
				append = false;
			else if (CheckToken(token, "a", "append"))
				append = true;
			else 
			{
				Print("Enter a valid mode (s)et or (a)ppend.\n");
				return false;
			}

			Print("Configure input %s [%s]: %s...", input->label, input->GetMapping(), (append ? "Appending" : "Setting"));
			fflush(stdout);	// required on terminals that use buffering
	
			// Configure the input
			if (input->Configure(append, "KEY_ESCAPE"))
				Print(" %s\n", input->GetMapping());
			else
				Print(" [Cancelled]\n");
			return false;
		}
		else if (CheckToken(token, "caip", "configallinputs"))		// configallinputs
		{
			m_inputs->ConfigureInputs(m_model3->GetGameInfo());
			return false;
		}
		//
		// Help
		//
		else if (CheckToken(token, "h", "help"))
		{
			CConsoleDebugger::ProcessToken(token, cmd);

			const char *fmt = "  %-6s %-25s %s\n";
			Print(" Emulator:\n");
			Print(fmt, "les",    "loademustate",           "<filename>");
			Print(fmt, "ses",    "saveemustate",           "<filename>");
			Print(fmt, "res",    "resetemu",               "");
			Print(" Inputs:\n");
			Print(fmt, "lip",    "listinputs",             "");
			Print(fmt, "pip",    "printinput",             "(<id>|<label>)");
			Print(fmt, "sip",    "setinput",               "(<id>|<label>) <mapping>");
			Print(fmt, "rip",    "resetinput",             "(<id>|<label>)");
			Print(fmt, "cip",    "configinput",            "(<id>|<label>) [(s)et|(a)ppend]");
			Print(fmt, "caip",   "configallinputs",        "");   
			return false;
		}
		else
			return CConsoleDebugger::ProcessToken(token, cmd);		
	}

	bool CSupermodelDebugger::InputIsValid(::CInput *input)
	{
		return input->IsUIInput() || (input->gameFlags & m_model3->GetGameInfo()->inputFlags);
	}

	void CSupermodelDebugger::ListInputs()
	{
		Print("Inputs:\n");
		if (m_inputs->Count() == 0)
			Print(" None\n");

		// Get maximum id, label and mapping widths
		size_t idAndLabelWidth = 0;
		size_t mappingWidth    = 0;
		for (unsigned i = 0; i < m_inputs->Count(); i++)
		{
			::CInput *input = (*m_inputs)[i];
			if (!InputIsValid(input))
				continue;

			idAndLabelWidth = max<size_t>(idAndLabelWidth, strlen(input->id) + strlen(input->label) + 3);
			if (!input->IsVirtual())
				mappingWidth = max<size_t>(mappingWidth, strlen(input->GetMapping()));
		}
		mappingWidth = min<size_t>(mappingWidth, 20);
		
		// Print labels, mappings and values for each input
		const char *groupLabel = NULL;
		char idAndLabel[255];
		char mapping[21];
		for (unsigned i = 0; i < m_inputs->Count(); i++)
		{
			::CInput *input = (*m_inputs)[i];
			if (!InputIsValid(input))
				continue;

			if (groupLabel == NULL || stricmp(groupLabel, input->GetInputGroup()) != 0)
			{
				groupLabel = input->GetInputGroup();
				Print(" %s:\n", groupLabel);
			}

			sprintf(idAndLabel, "%s (%s)", input->id, input->label);
			Print("  %-*s", (int)idAndLabelWidth, idAndLabel);
			if (!input->IsVirtual())
				Truncate(mapping, 20, input->GetMapping());
			else
				mapping[0] = '\0';
			Print(" %-*s %04X (%d)\n", (int)mappingWidth, mapping, input->value, input->value);
		}
	}

	void CSupermodelDebugger::Attached()
	{
		CConsoleDebugger::Attached();

		char fileName[25];
		sprintf(fileName, "Debug/%s.ds", m_model3->GetGameInfo()->id);
		LoadState(fileName);
	}

	void CSupermodelDebugger::Detaching()
	{
		char fileName[25];
		sprintf(fileName, "Debug/%s.ds", m_model3->GetGameInfo()->id);
		SaveState(fileName);
		
		CConsoleDebugger::Detaching();
	}

	bool CSupermodelDebugger::LoadModel3State(const char *fileName)
	{
		// Open file and find header 
		CBlockFile state;
		if (state.Load(fileName) != OKAY)
			return false;
		if (state.FindBlock("Debugger Model3 State") != OKAY)
		{
			state.Close();
			return false;
		}

		// Check version in header matches
		unsigned version;
		state.Read(&version, sizeof(version));
		if (version != MODEL3_STATEFILE_VERSION)
		{
			state.Close();
			return false;
		}

		// Load Model3 state
		m_model3->LoadState(&state);
		state.Close();

		// Reset debugger
		Reset();
		return true;
	}

	bool CSupermodelDebugger::SaveModel3State(const char *fileName)
	{
		// Create file with header
		CBlockFile state;
		if (state.Create(fileName, "Debugger Model3 State", __FILE__) != OKAY)
			return false;

		// Write out version in header
		unsigned version = MODEL3_STATEFILE_VERSION;
		state.Write(&version, sizeof(version));

		// Save Model3 state
		m_model3->SaveState(&state);
		state.Close();
		return true;
	}

	void CSupermodelDebugger::ResetModel3()
	{
		// Reset Model3
		m_model3->Reset();

		// Reset debugger
		Reset();
	}

	void CSupermodelDebugger::DebugLog(const char *fmt, va_list vl)
	{
		// Use the supplied logger, if any
		if (m_logger != NULL)
			m_logger->DebugLog(fmt, vl);
		else
			CConsoleDebugger::DebugLog(fmt, vl);
	}
		
	void CSupermodelDebugger::InfoLog(const char *fmt, va_list vl)
	{
		// Use the supplied logger, if any
		if (m_logger != NULL)
			m_logger->InfoLog(fmt, vl);
		else
			CConsoleDebugger::InfoLog(fmt, vl);
	}
	
	void CSupermodelDebugger::ErrorLog(const char *fmt, va_list vl)
	{
		// Use the supplied logger, if any
		if (m_logger != NULL)
			m_logger->ErrorLog(fmt, vl);
		else
			CConsoleDebugger::ErrorLog(fmt, vl);
	}

	void CSupermodelDebugger::Poll()
	{
		CConsoleDebugger::Poll();

		// Load/saving of emulator state and resetting emulator must be done here
		if (m_loadEmuState)
		{
			LoadModel3State(m_stateFile);
			m_loadEmuState = false;
			ForceBreak(false);
		}
		else if (m_saveEmuState)
		{
			SaveModel3State(m_stateFile);
			m_saveEmuState = false;
			ForceBreak(false);
		}
		else if (m_resetEmu)
		{
			ResetModel3();
			m_resetEmu = false;
			ForceBreak(false);
		}
	}
}

#endif  // SUPERMODEL_DEBUGGER