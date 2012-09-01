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
 * PCI.h
 * 
 * Header file for PCI bus and device emulation. Defines the CPCIDevice and
 * CPCIBus classes.
 *
 * References
 * ----------
 * 1. "MPC106 PCI/Bridge/Memory Controller User's Manual" (MPC106UM/D Rev.1) 
 *    Sec.7.4.5 describes the PCI configuration space header, which CPCIDevice-
 *    derived classes are supposed to implement.
 */

#ifndef INCLUDED_PCI_H
#define INCLUDED_PCI_H

#include <vector>

using namespace std;


/*
 * CPCIDevice:
 *
 * An abstract base class for PCI devices. Devices can inherit this class to 
 * provide PCI configuration space access handlers.
 */
class CPCIDevice
{
public:
	/*
	 * ReadPCIConfigSpace(reg, bits, offset):
	 *
	 * Reads a PCI configuration space register. Returns data in big endian
	 * form (little endian devices must byte reverse it).
	 *
	 * Parameters:
	 *		device	The device number. This is system-specific and ought to be
	 *				ignored in most cases (devices ought to know what they 
	 *				are) but is provided in case multiple devices are handled
	 *				by the same object.
	 *		reg		Register number (32-bit offset).
	 *		bits	Width of access (8, 16, or 32 only).
	 *		offset	Byte offset within register, aligned to bit width of
	 *				access. For bytes, can be 0, 1, 2, or 3, but for 16-bit
	 *				words can only be 0 or 2. Offsets are relative to the base
	 *				of the 32-bit aligned register address, so register must be
	 *				ANDed with ~3 first by caller.
	 *
	 * Returns:
	 *		Register data. Bit width will be the same as specified by 'bits'.
	 */
	virtual UINT32 ReadPCIConfigSpace(unsigned device, unsigned reg, unsigned bits, unsigned offset) = 0;
	
	/*
	 * WritePCIConfigSpace(reg, bits, offset, data):
	 *
	 * Writes to a PCI configuration space register.
	 *
	 * Parameters:
	 *		device	Device number.
	 *		reg		Register number (32-bit offset).
	 *		bits	Width of access (8, 16, or 32 only).
	 *		offset	Byte offset within register, aligned to bit width of 
	 *				access.
	 *		data	Data. Interpreted according to specified bit width.
	 */
	virtual void WritePCIConfigSpace(unsigned device, unsigned reg, unsigned bits, unsigned offset, UINT32 data) = 0;
};

/*
 * CPCIBus:
 *
 * A PCI bus. Dispatches commands to various attached devices. Because PCI
 * configuration space access is not a critical or frequently-used function,
 * dedicated 8-, 16-, and 32-bit handlers are not provided. Rather, the access
 * size is passed to the handler to respond accordingly.
 */
class CPCIBus
{
public:

	/*
	 * ReadConfigSpace(device, reg, bits, offset):
	 *
	 * Reads a PCI configuration space register belonging to a particular 
	 * device.
	 *
	 * Parameters:
	 *		device	PCI device ID.
	 *		reg		Register number.
	 *		bits	Width of access (8, 16, or 32 only).
	 *		offset	Byte offset within register, aligned to bit width of 
	 *				access. Offsets are relative to the base of the 32-bit 
	 *				aligned register address, so register must be ANDed with ~3
	 *				first by caller.
	 *
	 * Returns:
	 *		Register data. Bit width will be the same as specified by 'bits'.
	 */
	UINT32 ReadConfigSpace(unsigned device, unsigned reg, unsigned bits, unsigned offset);
	
	/*
	 * WriteConfigSpace(device, reg, bits, offset, data):
	 *
	 * Writes a PCI configuration space register.
	 *
	 * Parameters:
	 *		device	PCI device ID.
	 *		reg		Register number.
	 *		bits	Width of access (8, 16, or 32 only).
	 *		offset	Byte offset within register, aligned to bit width of 
	 *				access.
	 *		data	Data being written. Interpreted according to bit width.
	 */
	void WriteConfigSpace(unsigned device, unsigned reg, unsigned bits, unsigned offset, UINT32 data);
	
	/*
	 * Reset(void):
	 *
	 * Resets the PCI bus emulation. Does not reset or access any devices 
	 * attached to the PCI bus. These must be reset manually.
	 */
	void Reset(void);
	
	/*
 	 * AttachDevice(device, DeviceObjectPtr):
 	 *
 	 * Attaches a device to the PCI bus. This means it is added to a list of 
 	 * devices to scan when handling PCI configuration space requests.
 	 *
 	 * Parameters:
 	 *		device				PCI ID of the device being attached.
 	 *		DeviceObjectPtr		Pointer to the device object.
 	 */
 	void AttachDevice(unsigned device, CPCIDevice *DeviceObjectPtr);
 	 
	/*
	 * Init(void):
	 *
	 * One-time initialization of the context. Must be called prior to all
	 * other members.
	 */
	void Init(void);
	 
	/*
	 * CPCIBus(void):
	 * ~CPCIBus(void):
	 *
	 * Constructor and destructor.
	 */
	CPCIBus(void);
	~CPCIBus(void);
	
private:
	
	// Map device IDs to objects
	struct DeviceObjectLink
	{
		unsigned	device;
		CPCIDevice	*DeviceObject;
	};
	
	// An array of device objects
	vector<struct DeviceObjectLink>	DeviceVector;
};


#endif	// INCLUDED_PCI_H
