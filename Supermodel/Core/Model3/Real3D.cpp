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
 * Real3D.cpp
 * 
 * The Model 3's Real3D-based graphics hardware. Based on the Real3D Pro-1000
 * family of image generators.
 *
 * PCI IDs
 * -------
 * It appears that Step 2.0 returns a different PCI ID depending on whether
 * the PCI configuration space or DMA register are accessed. For example,
 * Virtual On 2 expects 0x178611DB from the PCI configuration header but 
 * 0x16C311DB from the DMA device. 
 *
 * To-Do List
 * ----------
 * - For consistency, the status registers should probably be byte reversed (this is a
 *   little endian device), forcing the Model3 Read32/Write32 handlers to
 *   manually reverse the data. This keeps with the convention for VRAM.
 * - Keep an eye out for games writing non-mipmap textures to the mipmap area.
 *   The render currently cannot cope with this.
 */

#include <cstring>
#include "Supermodel.h"

// Offsets of memory regions within Real3D memory pool
#define OFFSET_8C			0			// 4 MB, culling RAM low (at 0x8C000000)
#define OFFSET_8E			0x400000	// 1 MB, culling RAM high (at 0x8E000000)
#define OFFSET_98			0x500000	// 4 MB, polygon RAM (at 0x98000000)
#define OFFSET_TEXRAM		0x900000	// 8 MB, texture RAM
#define OFFSET_TEXFIFO		0x1100000	// 1 MB, texture FIFO
#define MEMORY_POOL_SIZE	(0x400000+0x100000+0x400000+0x800000+0x100000)


/******************************************************************************
 Save States
******************************************************************************/

void CReal3D::SaveState(CBlockFile *SaveState)
{
	SaveState->NewBlock("Real3D", __FILE__);
	
	SaveState->Write(memoryPool, MEMORY_POOL_SIZE);
	SaveState->Write(&fifoIdx, sizeof(fifoIdx));
	SaveState->Write(&vromTextureAddr, sizeof(vromTextureAddr));
	SaveState->Write(&vromTextureHeader, sizeof(vromTextureHeader));
	
	SaveState->Write(&dmaSrc, sizeof(dmaSrc));
	SaveState->Write(&dmaDest, sizeof(dmaDest));
	SaveState->Write(&dmaLength, sizeof(dmaLength));
	SaveState->Write(&dmaData, sizeof(dmaData));
	SaveState->Write(&dmaUnknownReg, sizeof(dmaUnknownReg));
	SaveState->Write(&dmaStatus, sizeof(dmaStatus));
	SaveState->Write(&dmaConfig, sizeof(dmaConfig));
	
	SaveState->Write(&tapCurrentInstruction, sizeof(tapCurrentInstruction));
	SaveState->Write(&tapIR, sizeof(tapIR));
	SaveState->Write(tapID, sizeof(tapID));
	SaveState->Write(&tapIDSize, sizeof(tapIDSize));
	SaveState->Write(&tapTDO, sizeof(tapTDO));
	SaveState->Write(&tapState, sizeof(tapState));
}

void CReal3D::LoadState(CBlockFile *SaveState)
{
	if (OKAY != SaveState->FindBlock("Real3D"))
	{
		ErrorLog("Unable to load Real3D GPU state. Save state file is corrupt.");
		return;
	}
	
	SaveState->Read(memoryPool, MEMORY_POOL_SIZE);
	Render3D->UploadTextures(0,0,2048,2048);
	SaveState->Read(&fifoIdx, sizeof(fifoIdx));
	SaveState->Read(&vromTextureAddr, sizeof(vromTextureAddr));
	SaveState->Read(&vromTextureHeader, sizeof(vromTextureHeader));
	
	SaveState->Read(&dmaSrc, sizeof(dmaSrc));
	SaveState->Read(&dmaDest, sizeof(dmaDest));
	SaveState->Read(&dmaLength, sizeof(dmaLength));
	SaveState->Read(&dmaData, sizeof(dmaData));
	SaveState->Read(&dmaUnknownReg, sizeof(dmaUnknownReg));
	SaveState->Read(&dmaStatus, sizeof(dmaStatus));
	SaveState->Read(&dmaConfig, sizeof(dmaConfig));
	
	SaveState->Read(&tapCurrentInstruction, sizeof(tapCurrentInstruction));
	SaveState->Read(&tapIR, sizeof(tapIR));
	SaveState->Read(tapID, sizeof(tapID));
	SaveState->Read(&tapIDSize, sizeof(tapIDSize));
	SaveState->Read(&tapTDO, sizeof(tapTDO));
	SaveState->Read(&tapState, sizeof(tapState));
}


/******************************************************************************
 Rendering
******************************************************************************/

void CReal3D::RenderFrame(void)
{
	//if (commandPortWritten)
		Render3D->RenderFrame();
	commandPortWritten = false;
}

void CReal3D::BeginFrame(void)
{
	status |= 2;	// VBlank bit
	Render3D->BeginFrame();
}

void CReal3D::EndFrame(void)
{
	error = false;	// clear error (just needs to be done once per frame)
	status &= ~2;
	Render3D->EndFrame();
}

/******************************************************************************
 DMA Device
 
 Register 0xC:
 -------------
 +---+---+---+---+---+---+---+---+
 |BUS|???|???|???|???|???|???|IRQ|
 +---+---+---+---+---+---+---+---+
 	BUS:	Busy (see von2 0x18A104) if 1.
 	IRQ:	IRQ pending.
******************************************************************************/

void CReal3D::DMACopy(void)
{
	DebugLog("Real3D DMA copy (PC=%08X, LR=%08X): %08X -> %08X, %X %s\n", ppc_get_pc(), ppc_get_lr(), dmaSrc, dmaDest, dmaLength*4, (dmaConfig&0x80)?"(byte reversed)":"");
	//printf("Real3D DMA copy (PC=%08X, LR=%08X): %08X -> %08X, %X %s\n", ppc_get_pc(), ppc_get_lr(), dmaSrc, dmaDest, dmaLength*4, (dmaConfig&0x80)?"(byte reversed)":"");	
	if ((dmaConfig&0x80))	// reverse bytes
    {
        while (dmaLength != 0)
        {
        	UINT32	data = Bus->Read32(dmaSrc);
            Bus->Write32(dmaDest, FLIPENDIAN32(data));
            dmaSrc += 4;
            dmaDest += 4;
            --dmaLength;
        }
    }
    else
    {
        while (dmaLength != 0)
        {
            Bus->Write32(dmaDest, Bus->Read32(dmaSrc));
            dmaSrc += 4;
            dmaDest += 4;
            --dmaLength;
        }
    }
}

UINT8 CReal3D::ReadDMARegister8(unsigned reg)
{
	switch (reg)
	{
	case 0xC:	// status
		return dmaStatus;
	case 0xE:	// configuration
		return	dmaConfig;
	default:
		break;
	}
	
	DebugLog("Real3D: ReadDMARegister8: reg=%X\n", reg);
	return 0;
}

void CReal3D::WriteDMARegister8(unsigned reg, UINT8 data)
{
	switch (reg)
	{
	case 0xD:	// IRQ acknowledge
		if ((data&1))
		{
			dmaStatus &= ~1;
			IRQ->Deassert(dmaIRQ);
		}
		break;
	case 0xE:	// configuration
		dmaConfig = data;
		break;
	default:
		DebugLog("Real3D: WriteDMARegister8: reg=%X, data=%02X\n", reg, data);
		break;
	}
	//DebugLog("Real3D: WriteDMARegister8: reg=%X, data=%02X\n", reg, data);
}

UINT32 CReal3D::ReadDMARegister32(unsigned reg)
{
	switch (reg)
	{
	case 0x14:	// command result
		return dmaData;
	default:
		break;
	}
	
	DebugLog("Real3D: ReadDMARegister32: reg=%X\n", reg);
	return 0;
}

void CReal3D::WriteDMARegister32(unsigned reg, UINT32 data)
{
	switch (reg)
	{
	case 0x00:	// DMA source address
		dmaSrc = data;
		break;
	case 0x04:	// DMA destination address
		dmaDest = data;
		break;
	case 0x08:	// DMA length
		dmaLength = data;
		DMACopy();
		dmaStatus |= 1;
		IRQ->Assert(dmaIRQ);
		break;
	case 0x10:	// command register
		if ((data&0x20000000))
		{
			dmaData = 0x16C311DB;	// Virtual On 2 expects this from DMA
			DebugLog("Real3D: DMA ID command issued (ATTENTION: make sure we're returning the correct value), PC=%08X, LR=%08X\n", ppc_get_pc(), ppc_get_lr());
		}
		else if ((data&0x80000000))
		{
			dmaUnknownReg ^= 0xFFFFFFFF;
			dmaData = dmaUnknownReg;
		}
		break;
	case 0x14:	// ?
		dmaData = 0xFFFFFFFF;
		break;
	default:
		DebugLog("Real3D: WriteDMARegister32: reg=%X, data=%08X\n", reg, data);
		break;
	}
	//DebugLog("Real3D: WriteDMARegister32: reg=%X, data=%08X\n", reg, data);
}

/******************************************************************************
 JTAG Test Access Port Simulation
 
 What I term as "IDs" here are really boundary scan values.
******************************************************************************/

static const int	tapFSM[][2] = 	// finite state machine, each state can lead to 2 next states
					{
						{  1,  0 },  // 0  Test-Logic/Reset
						{  1,  2 },  // 1  Run-Test/Idle
						{  3,  9 },  // 2  Select-DR-Scan
						{  4,  5 },  // 3  Capture-DR
						{  4,  5 },  // 4  Shift-DR
						{  6,  8 },  // 5  Exit1-DR
						{  6,  7 },  // 6  Pause-DR
						{  4,  8 },  // 7  Exit2-DR
						{  1,  2 },  // 8  Update-DR
						{ 10,  0 },  // 9  Select-IR-Scan
						{ 11, 12 },  // 10 Capture-IR
						{ 11, 12 },  // 11 Shift-IR
						{ 13, 15 },  // 12 Exit1-IR
						{ 13, 14 },  // 13 Pause-IR
						{ 11, 15 },  // 14 Exit2-IR
						{  1,  2 }   // 15 Update-IR
					};
					
/*
 * InsertBit():
 *
 * Inserts a bit into an arbitrarily long bit field. Bit 0 is assumed to be
 * the MSB of the first byte in the buffer.
 */
void CReal3D::InsertBit(UINT8 *buf, unsigned bitNum, unsigned bit)
{
    unsigned	bitInByte;

    bitInByte = 7 - (bitNum & 7);

    buf[bitNum / 8] &= ~(1 << bitInByte);
    buf[bitNum / 8] |= (bit << bitInByte);
}

/*
 * InsertID():
 *
 * Inserts a 32-bit ID code into the ID bit field.
 */
void CReal3D::InsertID(UINT32 id, unsigned startBit)
{
    int	i;

    for (i = 31; i >= 0; i--)
        InsertBit(tapID, startBit++, (id >> i) & 1);
}

/*
 * Shift():
 *
 * Shifts the data buffer right (towards LSB at byte 0) by 1 bit. The size of
 * the number of bits must be specified. The bit shifted out of the LSB is
 * returned.
 */
unsigned CReal3D::Shift(UINT8 *data, unsigned numBits)
{
    unsigned	i;
    unsigned	shiftOut, shiftIn;

	// This loop takes care of all the fully-filled bytes
    shiftIn = 0;
	shiftOut = 0;
    for (i = 0; i < numBits / 8; i++)
    {
        shiftOut = data[i] & 1;
        data[i] >>= 1;
        data[i] |= (shiftIn << 7);
        shiftIn = shiftOut;   // carry over to next element's MSB
    }

	// Take care of the last partial byte (if there is one)
    if ((numBits & 7) != 0)
    {
        shiftOut = (data[i] >> (8 - (numBits & 7))) & 1;
        data[i] >>= 1;
        data[i] |= (shiftIn << 7);
    }

    return shiftOut;
}

unsigned CReal3D::ReadTAP(void)
{
    return tapTDO;
}

void CReal3D::WriteTAP(unsigned tck, unsigned tms, unsigned tdi, unsigned trst)
{
    if (!tck)
        return;

    // Go to next state
    tapState = tapFSM[tapState][tms];

    switch (tapState)
    {
    case 3:     // Capture-DR
    	//printf("TAP: Capture-DR\n");
    	//bit = 0;

        /*
         * Read ASIC IDs.
         *
         * The ID Sequence is:
         *  - Jupiter
         *  - Mercury
         *  - Venus
         *  - Earth
         *  - Mars
         *  - Mars (again)
         *
         * Note that different Model 3 steps have different chip
         * revisions, hence the different IDs returned below.
         *
         * On Step 1.5 and 1.0, instruction 0x0C631F8C7FFE is used to retrieve
         * the ID codes but Step 2.0 is a little weirder. It seems to use this
         * and either the state of the TAP after reset or other instructions
         * to read the IDs as well. This can be emulated in one of 2 ways:
         * Ignore the instruction and always load up the data or load the
         * data on TAP reset and when the instruction is issued.
         */
        if (step == 0x10)
        {
            InsertID(0x116C7057, 1 + 0 * 32);
            InsertID(0x216C3057, 1 + 1 * 32);
            InsertID(0x116C4057, 1 + 2 * 32);
            InsertID(0x216C5057, 1 + 3 * 32);
            InsertID(0x116C6057, 1 + 4 * 32 + 1);
            InsertID(0x116C6057, 1 + 5 * 32 + 1);
        }
        else if (step == 0x15)
        {
            InsertID(0x316C7057, 1 + 0 * 32);
            InsertID(0x316C3057, 1 + 1 * 32);
            InsertID(0x216C4057, 1 + 2 * 32);	// Lost World may to use 0x016C4057
            InsertID(0x316C5057, 1 + 3 * 32);
            InsertID(0x216C6057, 1 + 4 * 32 + 1);
            InsertID(0x216C6057, 1 + 5 * 32 + 1);
        }
        else if (step >= 0x20)
        {
            InsertID(0x416C7057, 1 + 0 * 32);
            InsertID(0x416C3057, 1 + 1 * 32);
            InsertID(0x316C4057, 1 + 2 * 32);	// skichamp at PC=A89F4, this value causes "NO DAUGHTER BOARD" message
            InsertID(0x416C5057, 1 + 3 * 32);
            InsertID(0x316C6057, 1 + 4 * 32 + 1);
            InsertID(0x316C6057, 1 + 5 * 32 + 1);
        }

        break;

    case 4:     // Shift-DR

        tapTDO = Shift(tapID, tapIDSize);
        //printf("TAP: Shift-DR Bit %d\n", bit++);
        break;

    case 10:    // Capture-IR

		// Load lower 2 bits with 01 as per IEEE 1149.1-1990
        tapIR = 1;
        break;

    case 11:    // Shift-IR

		// Shift IR towards output and load in new data from TDI
        tapTDO = tapIR & 1;   // shift LSB to output
        tapIR >>= 1;
        tapIR |= ((UINT64) tdi << 45);
        break;

    case 15:    // Update-IR

		/*
         * Latch IR (technically, this should occur on the falling edge of
         * TCK)
         */
        tapIR &= 0x3FFFFFFFFFFFULL;
        tapCurrentInstruction = tapIR;
        //printf("TAP: Update-IR %XLL\n", tapCurrentInstruction);
        break;

    default:
        break;
    }
}


/******************************************************************************
 Texture Uploading and Decoding
******************************************************************************/

// Mipmap coordinates for each reduction level (within a single 2048x1024 page)
static const int mipXBase[11] =
{ 
	1024,	// 1024/2
	1536, 	// 512/2
	1792, 	// 256/2
	1920, 	// ...
	1984, 
	2016, 
	2032, 
	2040, 
	2044, 
	2046, 
	2047 
};

static const int mipYBase[11] =
{
	512, 
	768, 
	896, 
	960, 
	992, 
	1008,
	1016, 
	1020, 
	1022, 
	1023, 
	0 
};

// Mipmap reduction factors
static const int mipDivisor[9] = { 2, 4, 8, 16, 32, 64, 128, 256, 512 };

// Table of texel offsets corresponding to an 8x8 texel texture tile
static const unsigned decode[64] =
{
	 0, 1, 4, 5, 8, 9,12,13,
	 2, 3, 6, 7,10,11,14,15,
	16,17,20,21,24,25,28,29,
	18,19,22,23,26,27,30,31,
	32,33,36,37,40,41,44,45,
	34,35,38,39,42,43,46,47,
	48,49,52,53,56,57,60,61,
	50,51,54,55,58,59,62,63
};

void CReal3D::StoreTexture(unsigned xPos, unsigned yPos, unsigned width, unsigned height, UINT16 *texData, unsigned bytesPerTexel)
{
	unsigned	x, y, xx, yy, destOffset;
	
	if (bytesPerTexel == 2)	// 16-bit textures
	{
		// Outer 2 loops: 8x8 tiles
		for (y = yPos; y < (yPos+height); y += 8)
		{
			for (x = xPos; x < (xPos+width); x += 8)
			{
				// Inner 2 loops: 8x8 texels for the current tile
				destOffset = y*2048+x;
				for (yy = 0; yy < 8; yy++)
				{
					for (xx = 0; xx < 8; xx++)
						textureRAM[destOffset++] = texData[decode[(yy*8+xx)^1]];
					destOffset += 2048-8;	// next line
				}
				texData += 8*8;	// next tile
			}
		}
	}
	else					// 8-bit textures
	{
		/*
		 * 8-bit textures appear to be unpacked into 16-bit words in the
		 * texture RAM. Oddly, the rows of the decoding table seem to be
		 * swapped.
		 */
		 
		// Outer 2 loops: 8x8 tiles
		for (y = yPos; y < (yPos+height); y += 8)
		{
			for (x = xPos; x < (xPos+width); x += 8)
			{
				// Inner 2 loops: 8x8 texels for the current tile
				destOffset = y*2048+x;
				for (yy = 0; yy < 8; yy++)
				{
					for (xx = 0; xx < 8; xx += 2)
					{
						textureRAM[destOffset++] = texData[decode[(yy^1)*8+((xx+0)^1)]/2]>>8;
						textureRAM[destOffset++] = texData[decode[(yy^1)*8+((xx+1)^1)]/2]&0xFF;

					}
					destOffset += 2048-8;
				}
				texData += 8*8/2;	// next tile
			}
		}
	}
}

// Texture data will be in little endian format
void CReal3D::UploadTexture(UINT32 header, UINT16 *texData)
{
	unsigned	x, y, page, width, height, bytesPerTexel,
				mipYPos, mipWidth, mipHeight, mipNum, mipX, mipY;
	
	
	// Position: texture RAM is arranged as 2 2048x1024 texel sheets
	x = 32*(header&0x3F);
	y = 32*((header>>7)&0x1F);
	page = (header>>20)&1;
	y += page*1024;	// treat page as additional Y bit (one 2048x2048 sheet)
	
	// Texture size and bit depth
	width	= 32<<((header>>14)&7);
	height	= 32<<((header>>17)&7);
	if ((header&0x00800000))	// 16 bits per texel
		bytesPerTexel = 2;
	else						// 8 bits
	{
		bytesPerTexel = 1;
		//printf("8-bit textures!\n");
	}
	
	// Mipmaps
	mipYPos = 32*((header>>7)&0x1F);
	
	// Process texture data
	DebugLog("Real3D: Texture upload: pos=(%d,%d) size=(%d,%d), %d-bit\n", x, y, width, height, bytesPerTexel*8);
	//printf("Real3D: Texture upload: pos=(%d,%d) size=(%d,%d), %d-bit\n", x, y, width, height, bytesPerTexel*8);
	switch ((header>>24)&0x0F)
	{
	case 0x00:	// texture w/ mipmaps
		StoreTexture(x, y, width, height, texData, bytesPerTexel);
		mipWidth = width;
		mipHeight = height;
		mipNum = 0;

		while((mipHeight>8) && (mipWidth>8))
		{
			if (bytesPerTexel == 1)
				texData += (mipWidth*mipHeight)/2;
			else
				texData += (mipWidth*mipHeight);
			mipWidth /= 2;
			mipHeight /= 2;
			mipX = mipXBase[mipNum] + (x / mipDivisor[mipNum]);
			mipY = mipYBase[mipNum] + (mipYPos / mipDivisor[mipNum]);
			if(page)
				mipY += 1024;
			mipNum++;
			StoreTexture(mipX, mipY, mipWidth, mipHeight, (UINT16 *) texData, bytesPerTexel);
		}

		break;
	case 0x01:	// texture w/out mipmaps
		StoreTexture(x, y, width, height, texData, bytesPerTexel);
		break;
	case 0x02:	// mipmaps only
		mipWidth = width;
		mipHeight = height;
		mipNum = 0;
		while((mipHeight>8) && (mipWidth>8))
		{
			mipWidth /= 2;
			mipHeight /= 2;
			mipX = mipXBase[mipNum] + (x / mipDivisor[mipNum]);
			mipY = mipYBase[mipNum] + (mipYPos / mipDivisor[mipNum]);
			if(page)
				mipY += 1024;
			mipNum++;
			StoreTexture(mipX, mipY, mipWidth, mipHeight, texData, bytesPerTexel);
			if (bytesPerTexel == 1)
				texData += (mipWidth*mipHeight)/2;
			else
				texData += (mipWidth*mipHeight);
		}
		break;
	case 0x80:	// MAME thinks these might be a gamma table
		//break;
	default:	// unknown
		DebugLog("Unknown texture format %02X\n", header>>24);
		//printf("unknown texture format %02X\n", header>>24);
		break;
	}
	
	// Signal to renderer that textures have changed
	// TO-DO: mipmaps? What if a game writes non-mipmap textures to mipmap area?
	//Render3D->UploadTextures(x,y,width,height);
	Render3D->UploadTextures(0,0,2048,2048);	// TO-DO: should not have to upload all 2048x2048 texels
}


/******************************************************************************
 Basic Emulation Functions, Registers, Memory, and Texture FIFO
******************************************************************************/

void CReal3D::Flush(void)
{
	unsigned	i, size;
	UINT32		header;
	
	commandPortWritten = true;
	
	DebugLog("Real3D 88000000 written @ PC=%08X\n", ppc_get_pc());
	
	// Upload textures (if any)
	if (fifoIdx > 0)
	{
		for (i = 0; i < fifoIdx; )
		{
			size = 2+textureFIFO[i+0]/2;
			size /= 4;
			header = textureFIFO[i+1];		// texture information header
			
			// Spikeout seems to be uploading 0 length textures
			if (0 == size)
			{
				DebugLog("Real3D: 0-length texture upload @ PC=%08X (%08X %08X %08X)\n", ppc_get_pc(), textureFIFO[i+0], textureFIFO[i+1], textureFIFO[i+2]);
				break;
			}
			
			UploadTexture(header,(UINT16 *)&textureFIFO[i+2]);
			DebugLog("Real3D: Texture upload completed: %X bytes (%X)\n", size*4, textureFIFO[i+0]);
			i += size;
		}
	}
	
	// Reset texture FIFO
	fifoIdx = 0;
}

void CReal3D::WriteTextureFIFO(UINT32 data)
{
	if (fifoIdx >= (0x100000/4))
	{
		if (!error)
			ErrorLog("Overflow in Real3D texture FIFO!");
		error = true;
	}
	else
		textureFIFO[fifoIdx++] = data;
}

void CReal3D::WriteTexturePort(unsigned reg, UINT32 data)
{
	//printf("Texture Port: %X=%08X\n", reg, data);
	switch (reg)
	{
	case 0x0:	// VROM texture address
	case 0xC:
		vromTextureAddr = data;
		break;
	case 0x4:	// VROM texture header
	case 0x10:
		vromTextureHeader = data;
		break;
	case 0x8:	// VROM texture length (also used to trigger uploads)
	case 0x14:
		UploadTexture(vromTextureHeader,(UINT16 *)&vrom[vromTextureAddr&0xFFFFFF]);
		//printf("texture upload: addr=%08X\n", vromTextureAddr);
		break;
	default:
		DebugLog("Real3D texture port write: %X=%08X\n", reg, data);
		break;
	}
}

void CReal3D::WriteLowCullingRAM(UINT32 addr, UINT32 data)
{
	cullingRAMLo[addr/4] = data;
}

void CReal3D::WriteHighCullingRAM(UINT32 addr, UINT32 data)
{
	cullingRAMHi[addr/4] = data;
}

void CReal3D::WritePolygonRAM(UINT32 addr, UINT32 data)
{
	polyRAM[addr/4] = data;
}

// Registers seem to range from 0x00 to around 0x3C but they are not understood
UINT32 CReal3D::ReadRegister(unsigned reg)
{
	DebugLog("Real3D: Read reg %X\n", reg);
	if (reg == 0)
		return 0xFFFFFFFD|status;
	else
		return 0xFFFFFFFF;
}

UINT32 CReal3D::ReadPCIConfigSpace(unsigned device, unsigned reg, unsigned bits, unsigned offset)
{
	UINT32	d;
	
	if ((bits==8))
	{
		DebugLog("Real3D: %d-bit PCI read request for reg=%02X\n", bits, reg);
		return 0;
	}
	
	// This is a little endian device, must return little endian words
	switch (reg)
	{
	case 0x00:	// Device ID and Vendor ID
		d = FLIPENDIAN32(pciID);
		switch (bits)
		{
		case 8:
			d >>= (3-offset)*8;	// offset will be 0-3; select appropriate byte
			d &= 0xFF;
			break;
		case 16:
			d >>= (2-offset)*8;	// offset will be 0 or 2 only; select either high or low word
			d &= 0xFFFF;
			break;
		default:
			break;
		}
		DebugLog("Real3D: PCI ID read. Returning %X (%d-bits). PC=%08X, LR=%08X\n", d, bits, ppc_get_pc(), ppc_get_lr());
		return d;
	default:
		DebugLog("Real3D: PCI read request for reg=%02X (%d-bit)\n", reg, bits);
		break;
	}

	return 0;
}
	
void CReal3D::WritePCIConfigSpace(unsigned device, unsigned reg, unsigned bits, unsigned offset, UINT32 data)
{
	DebugLog("Real3D: PCI %d-bit write request for reg=%02X, data=%08X\n", bits, reg, data);
}
	
void CReal3D::Reset(void)
{
	error = false;
	
	commandPortWritten = false;
	
	fifoIdx = 0;
	status = 0;
	vromTextureAddr = 0;
	vromTextureHeader = 0;
	tapState = 0;
	tapIDSize = 197;
	dmaStatus = 0;
	dmaUnknownReg = 0;
	
	memset(memoryPool, 0, MEMORY_POOL_SIZE);
	
	DebugLog("Real3D reset\n");
}


/******************************************************************************
 Configuration, Initialization, and Shutdown
******************************************************************************/

void CReal3D::AttachRenderer(CRender3D *Render3DPtr)
{
	Render3D = Render3DPtr;
	Render3D->AttachMemory(cullingRAMLo,cullingRAMHi,polyRAM,vrom,textureRAM);
	Render3D->SetStep(step);
	DebugLog("Real3D attached a Render3D object\n");
}

void CReal3D::SetStep(int stepID)
{
	step = stepID;
	if ((step!=0x10) && (step!=0x15) && (step!=0x20) && (step!=0x21))
	{
		DebugLog("Real3D: Unrecognized stepping: %d.%d\n", (step>>4)&0xF, step&0xF);
		step = 0x10;
	}
	
	// Set PCI ID
	if (step < 0x20)			
		pciID = 0x16C311DB;	// vendor 0x11DB = Sega
	else
		pciID = 0x178611DB;
		
	// Pass to renderer
	if (Render3D != NULL)
		Render3D->SetStep(step);
		
	DebugLog("Real3D set to Step %d.%d\n", (step>>4)&0xF, step&0xF);
}

bool CReal3D::Init(const UINT8 *vromPtr, CBus *BusObjectPtr, CIRQ *IRQObjectPtr, unsigned dmaIRQBit)
{
	float	memSizeMB = (float)MEMORY_POOL_SIZE/(float)0x100000;

	// IRQ and bus objects
	Bus = BusObjectPtr; 
	IRQ = IRQObjectPtr;
	dmaIRQ = dmaIRQBit;
		
	// Allocate all Real3D RAM regions
	memoryPool = new(std::nothrow) UINT8[MEMORY_POOL_SIZE];
	if (NULL == memoryPool)
		return ErrorLog("Insufficient memory for Real3D object (needs %1.1f MB).", memSizeMB);
	
	// Set up pointers
	cullingRAMLo = (UINT32 *) &memoryPool[OFFSET_8C];
	cullingRAMHi = (UINT32 *) &memoryPool[OFFSET_8E];
	polyRAM = (UINT32 *) &memoryPool[OFFSET_98];
	textureRAM = (UINT16 *) &memoryPool[OFFSET_TEXRAM];
	textureFIFO = (UINT32 *) &memoryPool[OFFSET_TEXFIFO];

	// VROM pointer passed to us
	vrom = (UINT32 *) vromPtr;
	
	return OKAY;
}

CReal3D::CReal3D(void)
{	
	Render3D = NULL;
	memoryPool = NULL;
	cullingRAMLo = NULL;
	cullingRAMHi = NULL;
	polyRAM = NULL;
	textureRAM = NULL;
	textureFIFO = NULL;
	vrom = NULL;
	error = false;
	fifoIdx = 0;
	vromTextureAddr = 0;
	vromTextureHeader = 0;
	tapState = 0;
	tapIDSize = 197;
	DebugLog("Built Real3D\n");
}

/*
 * CReal3D::~CReal3D(void):
 *
 * Destructor.
 */
CReal3D::~CReal3D(void)
{	
	// Dump memory
#if 0
	FILE	*fp;
	fp = fopen("8c000000", "wb");
	if (NULL != fp)
	{
		fwrite(cullingRAMLo, sizeof(UINT8), 0x400000, fp);
		fclose(fp);
		printf("dumped %s\n", "8c000000");
	}
	else
		printf("unable to dump %s\n", "8c000000");
	fp = fopen("8e000000", "wb");
	if (NULL != fp)
	{
		fwrite(cullingRAMHi, sizeof(UINT8), 0x100000, fp);
		fclose(fp);
		printf("dumped %s\n", "8e000000");
	}
	else
		printf("unable to dump %s\n", "8e000000");
	fp = fopen("98000000", "wb");
	if (NULL != fp)
	{
		fwrite(polyRAM, sizeof(UINT8), 0x400000, fp);
		fclose(fp);
		printf("dumped %s\n", "98000000");
	}
	else
		printf("unable to dump %s\n", "98000000");
	fp = fopen("texram", "wb");
	if (NULL != fp)
	{
		fwrite(textureRAM, sizeof(UINT8), 0x800000, fp);
		fclose(fp);
		printf("dumped %s\n", "texram");
	}
	else
		printf("unable to dump %s\n", "texram");
#endif

	Render3D = NULL;
	if (memoryPool != NULL)
	{
		delete [] memoryPool;
		memoryPool = NULL;
	}
	cullingRAMLo = NULL;
	cullingRAMHi = NULL;
	polyRAM = NULL;
	textureRAM = NULL;
	textureFIFO = NULL;
	vrom = NULL;
	DebugLog("Destroyed Real3D\n");
}
