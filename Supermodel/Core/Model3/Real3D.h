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
 * Real3D.h
 * 
 * Header file defining the CReal3D class: the Model 3's Real3D-based graphics
 * hardware.
 */

#ifndef INCLUDED_REAL3D_H
#define INCLUDED_REAL3D_H


/*
 * CReal3D:
 *
 * Model 3 Real3D-based graphics hardware. This class manages the hardware
 * state and drives the rendering process (scene database traversal). Actual
 * rasterization and matrix transformations are carried out by the graphics
 * engine.
 */
class CReal3D: public CPCIDevice
{
public:
	/*
	 * SaveState(SaveState):
	 *
	 * Saves an image of the current device state.
	 *
	 * Parameters:
	 *		SaveState	Block file to save state information to.
	 */
	void SaveState(CBlockFile *SaveState);

	/*
	 * LoadState(SaveState):
	 *
	 * Loads and a state image.
	 *
	 * Parameters:
	 *		SaveState	Block file to load state information from.
	 */
	void LoadState(CBlockFile *SaveState);
	
	/*
	 * RenderFrame(void):
	 *
	 * Traverses the scene database and renders a frame. Must be called after
	 * BeginFrame() but before EndFrame().
	 */
	void RenderFrame(void);
	
	/*
	 * BeginFrame(void):
	 *
	 * Prepare to render a new frame. Must be called once per frame prior to
	 * drawing anything.
	 */
	void BeginFrame(void);
	
	/*
	 * EndFrame(void):
	 *
	 * Signals the end of rendering for this frame. Must be called last during
	 * the frame.
	 */
	void EndFrame(void);
	
	/*
	 * Flush(void):
	 *
	 * Triggers the beginning of a new frame. All textures in the texture FIFO
	 * are uploaded and the FIFO is reset. On the real device, this seems to 
	 * cause a frame to be rendered as well but this is not performed here.
	 *
	 * This should be called when the command port is written.
	 */
	void Flush(void);
	 
	/*
	 * ReadDMARegister8(reg):
	 * ReadDMARegister32(reg):
	 *
	 * Reads from a DMA register. Multi-byte reads are returned as little
	 * endian and must be flipped if called by a big endian device.
	 *
	 * Parameters:
	 *		reg		Register number to read from (0-0xFF only).
	 *
	 * Returns:
	 *		Data of the requested size, in little endian.
	 */
	UINT8	ReadDMARegister8(unsigned reg);
	UINT32	ReadDMARegister32(unsigned reg);
	
	/*
	 * WriteDMARegister8(reg, data):
	 * WriteDMARegister32(reg, data);
	 *
	 * Write to a DMA register. Multi-byte writes by big endian devices must be
	 * byte reversed (this is a little endian device).
	 *
	 * Parameters:
	 *		reg		Register number to read from (0-0xFF only).
	 *		data	Data to write.
	 */
	void WriteDMARegister8(unsigned reg, UINT8 data);
	void WriteDMARegister32(unsigned reg, UINT32 data);
	
	/*
	 * WriteLowCullingRAM(addr, data):
	 *
	 * Writes the low culling RAM region. Because this is a little endian
	 * device, big endian devices/buses have to take care to manually reverse
	 * the data before writing. 
	 * 
	 * Parameters:
	 *		addr	Word (32-bit) aligned address ranging from 0 to 0x3FFFFC.
	 *				User must ensure address is properly clamped.
	 *		data	Data to write.
	 */
	void WriteLowCullingRAM(UINT32 addr, UINT32 data);
	
	/*
	 * WriteHighCullingRAM(addr, data):
	 *
	 * Writes the high culling RAM region. Because this is a little endian
	 * device, big endian devices/buses have to take care to manually reverse
	 * the data before writing. 
	 * 
	 * Parameters:
	 *		addr	Word (32-bit) aligned address ranging from 0 to 0xFFFFC.
	 *				User must ensure address is properly clamped.
	 *		data	Data to write.
	 */
	void WriteHighCullingRAM(UINT32 addr, UINT32 data);
	
	/*
	 * WriteTextureFIFO(data):
	 *
	 * Writes to the 1MB texture FIFO. Because this is a little endian device,
	 * big endian devices/buses have to take care to manually reverse the data
	 * before writing.
	 *
	 * Parameters:
	 *		data	Data to write.
	 */
	void WriteTextureFIFO(UINT32 data);
	
	/*
	 * WriteTexturePort(reg, data):
	 *
	 * Writes to the VROM texture ports. Register 0 is the word-granular VROM
	 * address of the texture, register 4 is the texture information header,
	 * and register 8 is the size of the texture in words.
	 *
	 * Parameters:
	 *		reg		Register number: must be 0, 4, 8, 0xC, 0x10, or 0x14 only.
	 *		data	The 32-bit word to write to the register. A write to
	 *				register 8 triggers the upload.
	 */
	void WriteTexturePort(unsigned reg, UINT32 data);
	
	/*
	 * WritePolygonRAM(addr, data):
	 *
	 * Writes the polygon RAM region. Because this is a little endian device,
	 * big endian devices/buses have to take care to manually reverse the data
	 * before writing. 
	 * 
	 * Parameters:
	 *		addr	Word (32-bit) aligned address ranging from 0 to 0x3FFFFC.
	 *				User must ensure address is properly clamped.
	 *		data	Data to write.
	 */
	void WritePolygonRAM(UINT32 addr, UINT32 data);
	
	/*
	 * ReadTAP(void):
	 *
	 * Reads the JTAG Test Access Port.
	 *
	 * Returns:
	 *		The TDO bit (either 1 or 0).
	 */
	unsigned ReadTAP(void);
	
	/*
	 * void WriteTAP(tck, tms, tdi, trst):
	 *
	 * Writes to the JTAG TAP. State changes only occur on the rising edge of 
	 * the clock (tck = 1). Each of the inputs is a single bit only and must be
	 * either 0 or 1, or the code will fail.
	 *
	 * Parameters:
	 *      tck		Clock.
	 *      tms		Test mode select.
	 *      tdi		Serial data input. Must be 0 or 1 only!
	 *      trst	Reset.
	 */
	void WriteTAP(unsigned tck, unsigned tms, unsigned tdi, unsigned trst);
		
	/*
	 * ReadRegister(reg):
	 *
	 * Reads one of the status registers.
	 *
	 * Parameters:
	 *		reg		Register offset (32-bit aligned). From 0x00 to 0x3C.
	 *
	 * Returns:
	 *		The 32-bit status register.
	 */
	UINT32 ReadRegister(unsigned reg);
	
	/*
	 * ReadPCIConfigSpace(device, reg, bits, offset):
	 *
	 * Reads a PCI configuration space register. See CPCIDevice definition for
	 * more details.
	 *
	 * Parameters:
	 *		device	Device number (ignored, not needed).
	 *		reg		Register number.
	 *		bits	Bit width of access (8, 16, or 32 only).;
	 *		offset	Byte offset within register, aligned to the specified bit
	 *				width, and offset from the 32-bit aligned base of the
	 * 				register number.
	 *
	 * Returns:
	 *		Register data.
	 */
	UINT32 ReadPCIConfigSpace(unsigned device, unsigned reg, unsigned bits, unsigned width);
	
	/*
	 * WritePCIConfigSpace(device, reg, bits, offset, data):
	 *
	 * Writes to a PCI configuration space register. See CPCIDevice definition
	 * for more details.
	 *
	 * Parameters:
	 *		device	Device number (ignored, not needed).
	 *		reg		Register number.
	 *		bits	Bit width of access (8, 16, or 32 only).
	 *		offset	Byte offset within register, aligned to the specified bit
	 *				width, and offset from the 32-bit aligned base of the
	 * 				register number.
	 *		data	Data.
	 */
	void WritePCIConfigSpace(unsigned device, unsigned reg, unsigned bits, unsigned width, UINT32 data);
	
	/*
	 * Reset(void):
	 *
	 * Resets the Real3D device. Must be called before reading/writing the
	 * device.
	 */
	void Reset(void);
	
	/*
	 * AttachRenderer(render2DPtr):
	 *
	 * Attaches a 3D renderer for the Real3D to use. This function will
	 * immediately pass along the information that a CRender3D object needs to
	 * work with.
	 *
	 * Parameters:
	 *		Render3DPtr		Pointer to a 3D renderer object.
	 */
	void AttachRenderer(CRender3D *Render3DPtr);
	
	/*
	 * SetStep(stepID):
	 *
	 * Sets the Model 3 hardware stepping, which also determines the Real3D
	 * functionality. The default is Step 1.0. This should be called prior to 
	 * any other emulation functions and after Init().
	 *
	 * Parameters:
	 *		stepID	0x10 for Step 1.0, 0x15 for Step 1.5, 0x20 for Step 2.0,
	 *				or 0x21 for Step 2.1. Anything else defaults to 1.0.
	 */
	void SetStep(int stepID);
	
	/*
	 * Init(vromPtr, BusObjectPtr, IRQObjectPtr, dmaIRQBit):
	 *
	 * One-time initialization of the context. Must be called prior to all
	 * other members. Connects the Real3D device to its video ROM and allocates
	 * memory for RAM regions.
	 *
	 * Parameters:
	 *		vromPtr			A pointer to video ROM (with each 32-bit word in
	 *						its native little endian format).
	 *		BusObjectPtr	Pointer to the bus that the 53C810 has control
	 *						over. Used to read/write memory.
	 *		IRQObjectPtr	Pointer to the IRQ controller. Used to trigger SCSI
	 *						and DMA interrupts.
	 *		dmaIRQBit		IRQ identifier bit to pass along to IRQ controller
	 *						when asserting interrupts.

	 *
	 * Returns:
	 *		OKAY if successful otherwise FAIL (not enough memory). Prints own
	 *		errors.
	 */
	bool Init(const UINT8 *vromPtr, CBus *BusObjectPtr, CIRQ *IRQObjectPtr, unsigned dmaIRQBit);
	 
	/*
	 * CReal3D(void):
	 * ~CReal3D(void):
	 *
	 * Constructor and destructor.
	 */
	CReal3D(void);
	~CReal3D(void);
	
private:
	// Private member functions
	void		DMACopy(void);
	void		InsertBit(UINT8 *buf, unsigned bitNum, unsigned bit);
	void 		InsertID(UINT32 id, unsigned startBit);
	unsigned	Shift(UINT8 *data, unsigned numBits);
	void		StoreTexture(unsigned xPos, unsigned yPos, unsigned width, unsigned height, UINT16 *texData, unsigned bytesPerTexel);
	void		UploadTexture(UINT32 header, UINT16 *texData);
			
	// Renderer attached to the Real3D
	CRender3D	*Render3D;
	
	// Data passed from Model 3 object
	const UINT32	*vrom;	// Video ROM
	int				step;	// hardware stepping (as in GameInfo structure)
	UINT32			pciID;	// PCI vendor and device ID
	
	// Error flag (to limit errors to once per frame)
	bool		error;		// true if an error occurred this frame
	
	// Real3D memory
	UINT8		*memoryPool;	// all memory allocated here
	UINT32		*cullingRAMLo;	// 4MB of culling RAM at 8C000000
	UINT32		*cullingRAMHi;	// 1MB of culling RAM at 8E000000
	UINT32		*polyRAM;		// 4MB of polygon RAM at 98000000
	UINT16		*textureRAM;	// 8MB of internal texture RAM
	UINT32		*textureFIFO;	// 1MB texture FIFO at 0x94000000
	unsigned	fifoIdx;		// index into texture FIFO
	UINT32		vromTextureAddr;	// VROM texture port address data
	UINT32		vromTextureHeader;	// VROM texture port header data
	
	// Big endian bus object for DMA memory access
	CBus	*Bus;
	
	// IRQ handling
	CIRQ		*IRQ;	// IRQ controller
	unsigned	dmaIRQ;	// IRQ bit to use when calling IRQ handler
	
	// DMA device
	UINT32	dmaSrc;
	UINT32	dmaDest;
	UINT32	dmaLength;
	UINT32	dmaData;
	UINT32	dmaUnknownReg;
	UINT8	dmaStatus;
	UINT8	dmaConfig;
	
	// Command port
	bool	commandPortWritten;
	
	// Status and command registers
	UINT32	status;
	
	// JTAG Test Access Port
	UINT64		tapCurrentInstruction;	// latched IR (not always equal to IR)
	UINT64		tapIR;					// instruction register (46 bits)
	UINT8		tapID[32];				// ASIC ID code data buffer
	unsigned	tapIDSize;				// size of ID data in bits
	unsigned	tapTDO;					// bit shifted out to TDO
	int			tapState;				// current state

};


#endif	// INCLUDED_REAL3D_H
