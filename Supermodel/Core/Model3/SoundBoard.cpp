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
 * SoundBoard.cpp
 * 
 * Model 3 sound board. Implementation of the CSoundBoard class. This class can
 * only be instantiated once because it relies on global variables (the non-OOP
 * 68K core and an IRQ line).
 *
 * Bank Switching
 * --------------
 * Banking is not fully understood yet. It is presumed that the low 2MB of the 
 * sample ROMs are not banked (MAME), but this is not guaranteed. Below are 
 * examples of known situations where banking helps (sound names are as they
 * appear in the sound test menu).
 *
 *		sound name
 *			ROM Offsets -> Address
 * 			
 *		dayto2pe
 *		--------
 *		let's hope he does better
 *			A... -> A...	(400001=3E)
 *		doing good i'd say you have a ..
 *			E... -> E...	(400001=3D)
 *
 * From the above, it appears that when (400001)&10, then:
 *
 *		ROM A00000-DFFFFF -> A00000-DFFFFF
 *		ROM E00000-FFFFFF -> E00000-FFFFFF
 *
 * And when that bit is clear (just use default mapping, upper 6MB of ROM):
 *
 *		ROM 200000-5FFFFF -> A00000-DFFFFF
 *		ROM 600000-7FFFFF -> E00000-FFFFFF
 */

#include "Supermodel.h"

// DEBUG
//#define SUPERMODEL_LOG_AUDIO	// define this to log all audio to sound.bin
#ifdef SUPERMODEL_LOG_AUDIO
static FILE		*soundFP;
#endif


/******************************************************************************
 68K Address Space Handlers
******************************************************************************/

void CSoundBoard::UpdateROMBanks(void)
{
	if ((ctrlReg&0x10))
	{
		sampleBankLo = &sampleROM[0xA00000];
		sampleBankHi = &sampleROM[0xE00000];
	}
	else
	{
		sampleBankLo = &sampleROM[0x200000];
		sampleBankHi = &sampleROM[0x600000];
	}
}

UINT8 CSoundBoard::Read8(UINT32 a)
{ 
	switch ((a>>20)&0xF)
	{
	case 0x0:	// SCSP RAM 1 (master): 000000-0FFFFF
		return ram1[a^1];
		
	case 0x1:	// SCSP registers (master): 100000-10FFFF (unlike real hardware, we mirror up to 1FFFFF)
		return SCSP_Master_r8(a);
		
	case 0x2:	// SCSP RAM 2 (slave): 200000-2FFFFF
		return ram2[(a&0x0FFFFF)^1];
	
	case 0x3:	// SCSP registers (slave): 300000-30FFFF (unlike real hardware, we mirror up to 3FFFFF)
		return SCSP_Slave_r8(a);
		
	case 0x6:	// Program ROM: 600000-67FFFF (unlike real hardware, we mirror up to 6FFFFF here)	
		return soundROM[(a&0x07FFFF)^1];
	
	case 0x8:	// Sample ROM (low 2MB, fixed): 800000-9FFFFF
	case 0x9:
		return sampleROM[(a&0x1FFFFF)^1];
	
	case 0xA:	// Sample ROM (bank): A00000-DFFFFF
	case 0xB:
	case 0xC:
	case 0xD:
		return sampleBankLo[(a-0xA00000)^1];

	case 0xE:	// Sample ROM (bank): E00000-FFFFFF
	case 0xF:
		return sampleBankHi[(a&0x1FFFFF)^1];
		
	default:
		//printf("68K: Unknown read8 %06X\n", a);
		break;
	}
	
	return 0;
}

UINT16 CSoundBoard::Read16(UINT32 a) 
{ 
	switch ((a>>20)&0xF)
	{
	case 0x0:	// SCSP RAM 1 (master): 000000-0FFFFF
		return *(UINT16 *) &ram1[a];
		
	case 0x1:	// SCSP registers (master): 100000-10FFFF
		return SCSP_Master_r16(a);
		
	case 0x2:	// SCSP RAM 2 (slave): 200000-2FFFFF
		return *(UINT16 *) &ram2[a&0x0FFFFF];
	
	case 0x3:	// SCSP registers (slave): 300000-30FFFF
		return SCSP_Slave_r16(a);
		
	case 0x6:	// Program ROM: 600000-67FFFF
		return *(UINT16 *) &soundROM[a&0x07FFFF];
	
	case 0x8:	// Sample ROM (low 2MB, fixed): 800000-9FFFFF
	case 0x9:
		return *(UINT16 *) &sampleROM[a&0x1FFFFF];
	
	case 0xA:	// Sample ROM (bank): A00000-DFFFFF
	case 0xB:
	case 0xC:
	case 0xD:
		return *(UINT16 *) &sampleBankLo[a-0xA00000];

	case 0xE:	// Sample ROM (bank): E00000-FFFFFF
	case 0xF:
		return *(UINT16 *) &sampleBankHi[a&0x1FFFFF];
		
	default:
		//printf("68K: Unknown read16 %06X\n", a);
		break;
	}
	
	return 0;
}

UINT32 CSoundBoard::Read32(UINT32 a) 
{
	UINT32	hi, lo;
	
	switch ((a>>20)&0xF)
	{
	case 0x0:	// SCSP RAM 1 (master): 000000-0FFFFF
		hi = *(UINT16 *) &ram1[a];
		lo = *(UINT16 *) &ram1[a+2];	// TODO: clamp? Possible bounds hazard.
		return (hi<<16)|lo;
		
	case 0x1:	// SCSP registers (master): 100000-10FFFF
		return SCSP_Master_r32(a);
		
	case 0x2:	// SCSP RAM 2 (slave): 200000-2FFFFF
		hi = *(UINT16 *) &ram2[a&0x0FFFFF];
		lo = *(UINT16 *) &ram2[(a+2)&0x0FFFFF];
		return (hi<<16)|lo;
	
	case 0x3:	// SCSP registers (slave): 300000-30FFFF
		return SCSP_Slave_r32(a);
		
	case 0x6:	// Program ROM: 600000-67FFFF
		hi = *(UINT16 *) &soundROM[a&0x07FFFF];
		lo = *(UINT16 *) &soundROM[(a+2)&0x07FFFF];
		return (hi<<16)|lo;
	
	case 0x8:	// Sample ROM (low 2MB, fixed): 800000-9FFFFF
	case 0x9:
		hi = *(UINT16 *) &sampleROM[a&0x1FFFFF];
		lo = *(UINT16 *) &sampleROM[(a+2)&0x1FFFFF];
		return (hi<<16)|lo;
	
	case 0xA:	// Sample ROM (bank): A00000-DFFFFF
	case 0xB:
	case 0xC:
	case 0xD:
		hi = *(UINT16 *) &sampleBankLo[a-0xA00000];
		lo = *(UINT16 *) &sampleBankLo[a+2-0xA00000];
		return (hi<<16)|lo;

	case 0xE:	// Sample ROM (bank): E00000-FFFFFF
	case 0xF:
		hi = *(UINT16 *) &sampleBankHi[a&0x1FFFFF];
		lo = *(UINT16 *) &sampleBankHi[(a+2)&0x1FFFFF];
		return (hi<<16)|lo;
		
	default:
		//printf("68K: Unknown read32 %06X\n", a);
		break;
	}
	
	return 0;
}

void CSoundBoard::Write8(unsigned int a,unsigned char d)  
{ 
	switch ((a>>20)&0xF)
	{
	case 0x0:	// SCSP RAM 1 (master): 000000-0FFFFF
		ram1[a^1] = d;
		break;
		
	case 0x1:	// SCSP registers (master): 100000-10FFFF
		SCSP_Master_w8(a,d);
		break;
		
	case 0x2:	// SCSP RAM 2 (slave): 200000-2FFFFF
		ram2[(a&0x0FFFFF)^1] = d;
		break;
	
	case 0x3:	// SCSP registers (slave): 300000-30FFFF
		SCSP_Slave_w8(a,d);
		break;
	
	default:
		if (a == 0x400001)
		{
			ctrlReg = d;
			UpdateROMBanks();
		}
		//else
		//	printf("68K: Unknown write8 %06X=%02X\n", a, d);
		break;
	}
}

void CSoundBoard::Write16(unsigned int a,unsigned short d) 
{ 
	switch ((a>>20)&0xF)
	{
	case 0x0:	// SCSP RAM 1 (master): 000000-0FFFFF
		*(UINT16 *) &ram1[a] = d;
		break;
		
	case 0x1:	// SCSP registers (master): 100000-10FFFF
		SCSP_Master_w16(a,d);
		break;
		
	case 0x2:	// SCSP RAM 2 (slave): 200000-2FFFFF
		*(UINT16 *) &ram2[a&0x0FFFFF] = d;
		break;
	
	case 0x3:	// SCSP registers (slave): 300000-30FFFF
		SCSP_Slave_w16(a,d);
		break;
	
	default:
		//printf("68K: Unknown write16 %06X=%04X\n", a, d);
		break;
	}
}

void CSoundBoard::Write32(unsigned int a,unsigned int d)
{
	switch ((a>>20)&0xF)
	{
	case 0x0:	// SCSP RAM 1 (master): 000000-0FFFFF
		*(UINT16 *) &ram1[a] = (d>>16);
		*(UINT16 *) &ram1[a+2] = (d&0xFFFF);
		break;
		
	case 0x1:	// SCSP registers (master): 100000-10FFFF
		SCSP_Master_w32(a,d);
		break;
		
	case 0x2:	// SCSP RAM 2 (slave): 200000-2FFFFF
		*(UINT16 *) &ram2[a&0x0FFFFF] = (d>>16);
		*(UINT16 *) &ram2[(a+2)&0x0FFFFF] = (d&0xFFFF);
		break;
	
	case 0x3:	// SCSP registers (slave): 300000-30FFFF
		SCSP_Slave_w32(a,d);
		break;
	
	default:
		//printf("68K: Unknown write32 %06X=%08X\n", a, d);
		break;
	}
}


/******************************************************************************
 SCSP 68K Callbacks
 
 The SCSP emulator drives the 68K via callbacks. These have to live outside of
 the CSoundBoard object for now, unfortunately.
******************************************************************************/

// Status of IRQ pins (IPL2-0) on 68K
// TODO: can we get rid of this global variable altogether?
static int	irqLine = 0;

// Interrupt acknowledge callback (TODO: don't need this, default behavior in M68K.cpp should be fine)
int IRQAck(int irqLevel)
{
	M68KSetIRQ(0);
	irqLine = 0;
	return M68K_IRQ_AUTOVECTOR;
}

// SCSP callback for generating IRQs
void SCSP68KIRQCallback(int irqLevel)
{
	/*
	 * IRQ arbitration logic: only allow higher priority IRQs to be asserted or
	 * 0 to clear pending IRQ.
	 */
	if ((irqLevel>irqLine) || (0==irqLevel))
	{
		irqLine = irqLevel;	
		
	}
	M68KSetIRQ(irqLine);
}

// SCSP callback for running the 68K
int SCSP68KRunCallback(int numCycles)
{
	return M68KRun(numCycles) - numCycles;
}


/******************************************************************************
 Sound Board Interface
******************************************************************************/

void CSoundBoard::WriteMIDIPort(UINT8 data)
{
	SCSP_MidiIn(data);
	if (NULL != DSB)	// DSB receives all commands as well
		DSB->SendCommand(data);
}

bool CSoundBoard::RunFrame(void)
{
	// Run sound board first to generate SCSP audio
	if (g_Config.emulateSound)
	{
		M68KSetContext(&M68K);
		SCSP_Update();
		M68KGetContext(&M68K);
	}
	else
	{
		memset(audioL, 0, 44100/60*sizeof(INT16));
		memset(audioR, 0, 44100/60*sizeof(INT16));
	}
	
	// Run DSB and mix with existing audio
	if (NULL != DSB)
		DSB->RunFrame(audioL, audioR);

	// Output the audio buffers
	bool bufferFull = OutputAudio(44100/60, audioL, audioR);

#ifdef SUPERMODEL_LOG_AUDIO
	// Output to binary file
	INT16	s;
	for (int i = 0; i < 44100/60; i++)
	{	
		s = audioL[i];
		fwrite(&s, sizeof(INT16), 1, soundFP);	// left channel
		s = audioR[i];
		fwrite(&s, sizeof(INT16), 1, soundFP);	// right channel
	}
#endif // SUPERMODEL_LOG_AUDIO

	return bufferFull;
}

void CSoundBoard::Reset(void)
{
	// Even if SCSP emulation is disabled, we must reset to establish a valid 68K state
	memcpy(ram1, soundROM, 16);				// copy 68K vector table
	ctrlReg = 0;							// set default banks
	UpdateROMBanks();
	M68KSetContext(&M68K);
	M68KReset();
	//printf("SBrd PC=%06X\n", M68KGetPC());
	M68KGetContext(&M68K);
	if (NULL != DSB)
		DSB->Reset();
	DebugLog("Sound Board Reset\n");
	//printf("PC=%06X\n", M68KGetPC());
	//M68KSetContext(&M68K);
	//M68KGetContext(&M68K);
	//printf("PC=%06X\n", M68KGetPC());
}

void CSoundBoard::SaveState(CBlockFile *SaveState)
{
	SaveState->NewBlock("Sound Board", __FILE__);
	SaveState->Write(ram1, 0x100000);
	SaveState->Write(ram2, 0x100000);
	SaveState->Write(&ctrlReg, sizeof(ctrlReg));
	
	// All other devices...
	M68KSetContext(&M68K);
	M68KSaveState(SaveState, "Sound Board 68K");
	SCSP_SaveState(SaveState);
	if (NULL != DSB)
		DSB->SaveState(SaveState);
}

void CSoundBoard::LoadState(CBlockFile *SaveState)
{
	if (OKAY != SaveState->FindBlock("Sound Board"))
	{
		ErrorLog("Unable to load sound board state. Save state file is corrupt.");
		return;
	}
	
	SaveState->Read(ram1, 0x100000);
	SaveState->Read(ram2, 0x100000);
	SaveState->Read(&ctrlReg, sizeof(ctrlReg));
	UpdateROMBanks();
	
	// All other devices
	M68KSetContext(&M68K);	// so we don't lose callback pointers when copying context back
	M68KLoadState(SaveState, "Sound Board 68K");
	M68KGetContext(&M68K);
	SCSP_LoadState(SaveState);
	if (NULL != DSB)
		DSB->LoadState(SaveState);
}


/******************************************************************************
 Configuration, Initialization, and Shutdown
******************************************************************************/

void CSoundBoard::AttachDSB(CDSB *DSBPtr)
{
	DSB = DSBPtr;
	DebugLog("Sound Board connected to DSB\n");
}

// Offsets of memory regions within sound board's pool
#define OFFSET_RAM1			0			// 1 MB SCSP1 RAM
#define OFFSET_RAM2			0x100000	// 1 MB SCSP2 RAM
#define OFFSET_AUDIO_LEFT	0x200000	// 1470 bytes (16 bits, 44.1 KHz, 1/60th second) left audio channel
#define OFFSET_AUDIO_RIGHT	0x2005BE	// 1470 bytes right audio channel
#define MEMORY_POOL_SIZE	(0x100000 + 0x100000 + 0x5BE + 0x5BE)

bool CSoundBoard::Init(const UINT8 *soundROMPtr, const UINT8 *sampleROMPtr)
{
	float	memSizeMB = (float)MEMORY_POOL_SIZE/(float)0x100000;
	
	// Receive sound ROMs
	soundROM = soundROMPtr;
	sampleROM = sampleROMPtr;
	ctrlReg = 0;
	UpdateROMBanks();

	// Allocate all memory for RAM
	memoryPool = new(std::nothrow) UINT8[MEMORY_POOL_SIZE];
	if (NULL == memoryPool)
		return ErrorLog("Insufficient memory for sound board (needs %1.1f MB).", memSizeMB);
	memset(memoryPool, 0, MEMORY_POOL_SIZE);
	
	// Set up memory pointers
	ram1 = &memoryPool[OFFSET_RAM1];
	ram2 = &memoryPool[OFFSET_RAM2];
	audioL = (INT16 *) &memoryPool[OFFSET_AUDIO_LEFT];
	audioR = (INT16 *) &memoryPool[OFFSET_AUDIO_RIGHT];
	
	// Initialize 68K core
	M68KSetContext(&M68K);
	M68KInit();
	M68KAttachBus(this);
	M68KSetIRQCallback(IRQAck);
	M68KGetContext(&M68K);
		
	// Initialize SCSPs
	SCSP_SetBuffers(audioL, audioR, 44100/60);
	SCSP_SetCB(SCSP68KRunCallback, SCSP68KIRQCallback);
	if (OKAY != SCSP_Init(2))
		return FAIL;
	SCSP_SetRAM(0, ram1);
	SCSP_SetRAM(1, ram2);
	
	// Binary logging
#ifdef SUPERMODEL_LOG_AUDIO
	soundFP = fopen("sound.bin","wb");	// delete existing file
	fclose(soundFP);
	soundFP = fopen("sound.bin","ab");	// append mode
#endif

	return OKAY;
}

M68KCtx *CSoundBoard::GetM68K(void)
{
	return &M68K;
}

CDSB *CSoundBoard::GetDSB(void)
{
	return DSB;
}

CSoundBoard::CSoundBoard(void)
{
	DSB = NULL;
	memoryPool = NULL;
	ram1 = NULL;
	ram2 = NULL;
	audioL = NULL;
	audioR = NULL;
	soundROM = NULL;
	sampleROM = NULL;
	
	DebugLog("Built Sound Board\n");
}

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

CSoundBoard::~CSoundBoard(void)
{	
#ifdef SUPERMODEL_LOG_AUDIO
	// close binary log file
	fclose(soundFP);
#endif

	SCSP_Deinit();
	
	DSB = NULL;
	
	if (memoryPool != NULL)
	{
		delete [] memoryPool;
		memoryPool = NULL;
	}
	ram1 = NULL;
	ram2 = NULL;
	audioL = NULL;
	audioR = NULL;
	soundROM = NULL;
	sampleROM = NULL;
	
	DebugLog("Destroyed Sound Board\n");
}
