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
 * 53C810.h
 * 
 * Header file defining the C53C810 class (NCR 53C810 SCSI controller).
 */

#ifndef INCLUDED_53C810_H
#define INCLUDED_53C810_H


/*
 * struct NCR53C810Context:
 *
 * Context information for an NCR 53C810 device. Used internally by the C53C810
 * class (defined below).
 */
struct NCR53C810Context
{
	UINT8	regs[0x60];
	
	// Registers defined below should not be read from here, not regs[]
	UINT32	regTEMP;	// TEMP
	UINT32	regDSP;		// DSP: DMA SCRIPTS Pointer
	UINT32	regDSPS;	// DSPS: DMA SCRIPTS Pointer Save
	UINT32	regDBC;		// DBC: DMA Byte Counter (24 bits)
	UINT8	regDCMD;	// DCMD: DMA Command
	UINT8	regDCNTL;	// DCNTL: DMA Control
	UINT8	regDMODE;	// DMODE: DMA Mode
	UINT8	regDSTAT;	// DSTAT: DMA Status (read only)
	UINT8	regISTAT;	// ISTAT: Interrupt Status
	
	// Operational status
	bool	halt;		// set true if halted by interrupt instruction
	
	// Big endian bus object for DMA memory access and instruction fetching
	CBus	*Bus;
	
	// IRQ handling
	CIRQ		*IRQ;		// IRQ controller
	unsigned	scsiIRQ;	// IRQ bit to use when calling IRQ handler
};

/*
 * C53C810:
 *
 * NCR 53C810 SCSI controller device.
 */
class C53C810: public CPCIDevice
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
	 * ReadRegister(reg):
	 *
	 * Read from an operating register (8 bits at a time). 
	 *
	 * Parameters:
	 *		reg		Register offset ranging from 0x00 to 0x5F. Anything higher
	 *				is ignored and returns 0.
	 */
	UINT8 ReadRegister(unsigned reg);
	
	/*
	 * WriteRegister(reg, data):
	 *
	 * Write to an operating register (8 bits at a time). When breaking multi-
	 * byte words into individual bytes, make sure to write the lowest address
	 * first and the highest last. Some special functions registers, like DSP,
	 * will initiate processing only when the highest byte is written.
	 *
	 * Parameters:
	 *		reg		Register offset ranging from 0x00 to 0x5F. Anything higher
	 *				is ignored.
	 *		data	Data to write.
	 */
	void WriteRegister(unsigned reg, UINT8 data);
	
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
	 * Resets the device. 
	 */
	void Reset(void);
	
	/*
	 * Init(BusObjectPtr, IRQObjectPtr, scsiIRQBit):
	 *
	 * One-time initialization of the context. Must be called prior to all
	 * other members.
	 *
	 * Parameters:
	 *		BusObjectPtr	Pointer to the bus that the 53C810 has control
	 *						over. Used to read/write memory.
	 *		IRQObjectPtr	Pointer to the IRQ controller. Used to trigger SCSI
	 *						and DMA interrupts.
	 *		scsiIRQBit		IRQ identifier bit to pass along to IRQ controller
	 *						when asserting interrupts.
	 */
	void Init(CBus *BusObjectPtr, CIRQ *IRQObjectPtr, unsigned scsiIRQBit);
	 
	/*
	 * C53C810(void):
	 * ~C53C810(void):
	 *
	 * Constructor and destructor.
	 */
	C53C810(void);
	~C53C810(void);
	
private:
	// Private members
	void	Run(bool singleStep);
	void	BuildOpTable(void);
	void	Insert(UINT8 mask, UINT8 op, bool (*Handler)(struct NCR53C810Context *));
	bool	(*OpTable[256])(struct NCR53C810Context *);
	
	// Context (register file)
	struct NCR53C810Context	Ctx;
	
	// IRQ controller and IRQ identifier for this SCSI controller
	CIRQ		*IRQ;
	unsigned	scsiIRQ;
};


#endif	// INCLUDED_53C810_H
