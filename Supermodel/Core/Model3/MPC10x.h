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
 * MPC10x.h
 * 
 * Header file defining the CMPC10x class (Motorola MPC105 and MPC106).
 */

#ifndef INCLUDED_MPC10X_H
#define INCLUDED_MPC10X_H


/*
 * CMPC10x:
 *
 * MPC105 or MPC106 PCI/bridge/memory controller device.
 */
class CMPC10x
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
	 * WritePCIConfigAddress(data):
	 *
	 * Writes to the PCI configuration space address (CONFIG_ADDR) register, 
	 * which selects a PCI device and register. For now, only 32-bit accesses 
	 * are supported.
	 *
	 * Parameters:
	 *		data	Data to write.
	 */
	void WritePCIConfigAddress(UINT32 data);
	
	/*
	 * ReadPCIConfigData(bits, offset):
	 *
	 * Reads from the PCI configuration space data (CONFIG_DATA) register, 
	 * which in turn calls upon the selected PCI device to return the data.
	 *
	 * Parameters:
	 *		bits	Width of access (8, 16, or 32 only).
	 *		offset	Byte offset within CONFIG_DATA, aligned to bit width of
	 *				access. For bytes, can be 0, 1, 2, or 3, but for 16-bit
	 *				words can only be 0 or 2. The offset is just the 
	 *				CONFIG_DATA address ANDed with 0, 2, or 3 depending on the
	 *				bit width (32, 16, or 8, respectively).
	 *
	 * Returns:
	 *		Data from the PCI conifguration space of the presently selected 
	 *		device. Bit width will be the same as specified by 'bits'.
	 */
	UINT32 ReadPCIConfigData(unsigned bits, unsigned offset);
	
	/*
	 * WritePCIConfigData(bits, offset, data):
	 *
	 * Writes data (CONFIG_DATA) to the presently selected PCI device and
	 * register. 
	 *
	 * Parameters:
	 *		bits	Width of access (8, 16, or 32 only).
	 *		offset	Byte offset within CONFIG_DATA, aligned to bit width of
	 *				access.
	 *		data	Data to write.
	 */
	void WritePCIConfigData(unsigned bits, unsigned offset, UINT32 data);
	
	/*
	 * WriteRegister(reg, data):
	 *
	 * Writes to the MPC10x's internal registers.
	 *
	 * Parameters:
	 *		reg		Register (0-255).
	 *		data	Data to write.
	 */
	void WriteRegister(unsigned reg, UINT8 data);
	
	/*
	 * Reset(void):
	 *
	 * Resets the device. 
	 */
	void Reset(void);
	
	/*
	 * AttachPCIBus(BusObjectPtr):
	 *
	 * Attaches a PCI bus object which will handle all PCI register accesses.
	 * For now, only one bus is supported. The MPCI10x will automatically
	 * handle any requests for device 0 (itself) without passing them on to the
	 * PCI bus.
	 *
	 * Parameters:
	 *		BusObjectPtr	A pointer to the PCI bus object.
	 */
	void AttachPCIBus(CPCIBus *BusObjectPtr);
	
	/*
	 * SetModel(modelNum):
	 *
	 * Selects either MPC105 or MPC106 behavior. This will also reset the
	 * device state by calling Reset(). This should be called prior to any
	 * other emulation functions and after Init().
	 *
	 * Parameters:
	 *		modelNum	Either 0x105 for MPC105 or 0x106 for MPC106. Defaults
	 *					to MPC105 if unrecognized.
	 */
	void SetModel(int modelNum);
	 
	/*
	 * Init(void):
	 *
	 * One-time initialization of the context. Must be called prior to all
	 * other members. By default, initializes to MPC105.
	 */
	void Init(void);
	 
	/*
	 * CMPC10x(void):
	 * ~CMPC10x(void):
	 *
	 * Constructor and destructor for MPC10x class.
	 */
	CMPC10x(void);
	~CMPC10x(void);
	
private:
	int			model;			// MPC105 = 0x105 (default), MPC106 = 0x106
	CPCIBus		*PCIBus;		// the PCI bus to which this MPC10x is attached
	UINT8		regs[256];		// internal registers
	unsigned	pciBus;			// PCI bus component of PCI address setting (8 bits)
	unsigned	pciDevice;		// PCI device component (5 bits)
	unsigned	pciFunction;	// PCI function component (3 bits)
	unsigned	pciReg;			// PCI register component (7 bits)
};


#endif	// INCLUDED_MPC10X_H
