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
 * PCI.cpp
 * 
 * PCI bus and device emulation. Implementation of the CPCIBus class.
 * CPCIDevice is just a base class from which others are derived. Only the 
 * transfer of PCI register data is emulated. Device functionality is 
 * handled by device objects (derived from CPCIDevice).
 *
 * TO-DO List:
 * -----------
 * - Exception handling for out-of-memory errors in STL? Right now, we assume
 *   the vectors do not run out of memory because so few PCI devices exist.
 */

#include "Supermodel.h"


/******************************************************************************
 Emulation Functions
******************************************************************************/

/*
 * CPCIBus::ReadConfigSpace(device, reg, bits offset):
 *
 * Reads a PCI register belonging to a particular device.
 */
UINT32 CPCIBus::ReadConfigSpace(unsigned device, unsigned reg, unsigned bits, unsigned offset)
{
	unsigned	i;
	
	// Alignment check
#ifdef DEBUG
	if (((bits==16)&&(offset&1)) || ((bits==32)&&(offset&3)))
		ErrorLog("Misaligned PCI read request (device=%d,reg=%X,offset=%d)\n", device, reg, offset);
#endif

	// Search device vector for a matching device
	for (i = 0; i < DeviceVector.size(); i++)
	{
		if (DeviceVector[i].device == device)
			return DeviceVector[i].DeviceObject->ReadPCIConfigSpace(device, reg, bits, offset);
	}
	
	DebugLog("PCI read request for unknown device (device=%d,reg=%X)\n", device, reg);
	return 0;
}

/*
 * CPCIBus::WriteConfigSpace(device, reg, bits, offset, data):
 *
 * Writes to the PCI configuration space register belonging to a particular
 * device.
 */
void CPCIBus::WriteConfigSpace(unsigned device, unsigned reg, unsigned bits, unsigned offset, UINT32 data)
{
	unsigned	i;
	
	// Search device vector for a matching device
	for (i = 0; i < DeviceVector.size(); i++)
	{
		if (DeviceVector[i].device == device)
		{
			DeviceVector[i].DeviceObject->WritePCIConfigSpace(device, reg, bits, offset, data);
			return;
		}
	}
	
//	printf("PCI write request for unknown device (device=%d, reg=%X, data=%X)\n", device, reg, data);
	DebugLog("PCI write request for unknown device (device=%d, reg=%X, data=%X)\n", device, reg, data);
}
	
/*
 * CPCIBus::Reset():
 *
 * Resets the PCI bus emulation only. The devices attached to the bus are NOT
 * reset or accessed.
 */
void CPCIBus::Reset(void)
{
	// this function really only exists for consistency with other device classes
}


/******************************************************************************
 Configuration, Initialization, and Shutdown
******************************************************************************/

/*
 * CPCIBus::AttachDevice(device, DeviceObjectPtr):
 *
 * Attaches a device to the PCI bus. This means it is added to a list of 
 * devices to scan when handling PCI configuration space requests.
 */
void CPCIBus::AttachDevice(unsigned device, CPCIDevice *DeviceObjectPtr)
{
	struct DeviceObjectLink	D;
	
	D.device = device;
	D.DeviceObject = DeviceObjectPtr;
	DeviceVector.push_back(D);
	
	DebugLog("Attached device %d to PCI bus\n", device);
}

/*
 * CPCIBus::Init():
 *
 * This must be called first and only once during the lifetime of the class.
 */
void CPCIBus::Init(void)
{
	DeviceVector.clear();
}

/*
 * CPCIBus::CPCIBus(void):
 *
 * Constructor.
 */
CPCIBus::CPCIBus(void)
{	
	DebugLog("Built PCI bus\n");
}

/*
 * CPCIBus::~CPCIBus(void):
 *
 * Destructor.
 */
CPCIBus::~CPCIBus(void)
{	
	DebugLog("Destroyed PCI bus\n");
}
