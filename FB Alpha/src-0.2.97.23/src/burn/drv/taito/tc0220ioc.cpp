// TC0220IOC

#include "burnint.h"
#include "taito_ic.h"

UINT8 TC0220IOCInputPort0[8] = { 0, 0, 0, 0, 0, 0, 0, 0 };
UINT8 TC0220IOCInputPort1[8] = { 0, 0, 0, 0, 0, 0, 0, 0 };
UINT8 TC0220IOCInputPort2[8] = { 0, 0, 0, 0, 0, 0, 0, 0 };
UINT8 TC0220IOCDip[2]        = { 0, 0 };
UINT8 TC0220IOCInput[3]      = { 0, 0, 0 };

static UINT8 TC0220IOCRegs[8];
static UINT8 TC0220IOCPort;

extern UINT8 TaitoCoinLockout[4];

UINT8 TC0220IOCPortRead()
{
	return TC0220IOCPort;
}

UINT8 TC0220IOCHalfWordPortRead()
{
	return TC0220IOCPortRead();
}

UINT8 TC0220IOCRead(UINT8 Port)
{
	switch (Port) {
		case 0x00: {
			return TC0220IOCDip[0];
		}
		
		case 0x01: {
			return TC0220IOCDip[1];
		}
		
		case 0x02: {
			return TC0220IOCInput[0];
		}
		
		case 0x03: {
			return TC0220IOCInput[1];
		}
		
		case 0x04: {
			return TC0220IOCRegs[4];
		}
		
		case 0x07: {
			return TC0220IOCInput[2];
		}
	}
	
	bprintf(PRINT_IMPORTANT, _T("Unmapped TC0220IOC Read at %x\n"), Port);
	
	return 0;
}

UINT8 TC0220IOCPortRegRead()
{
	return TC0220IOCRead(TC0220IOCPort);
}

UINT8 TC0220IOCHalfWordRead(INT32 Offset)
{
	return TC0220IOCRead(Offset);
}

void TC0220IOCWrite(UINT8 Port, UINT8 Data)
{
	TC0220IOCRegs[Port] = Data;

	if (Port == 0) {
		TaitoWatchdog = 0;
	}

	if (Port == 4) {
		TaitoCoinLockout[0] = ~Data & 0x01;
		TaitoCoinLockout[1] = ~Data & 0x02;
		// coin counters 0x04 & 0x08
	}
}

void TC0220IOCHalfWordPortRegWrite(UINT16 Data)
{
	TC0220IOCWrite(TC0220IOCPort, Data & 0xff);
}

static void TC0220IOCPortWrite(UINT8 Data)
{
	TC0220IOCPort = Data;
}

void TC0220IOCHalfWordPortWrite(UINT16 Data)
{
	TC0220IOCPortWrite(Data & 0xff);
}

void TC0220IOCHalfWordWrite(INT32 Offset, UINT16 Data)
{
	TC0220IOCWrite(Offset, Data & 0xff);
}

void TC0220IOCReset()
{
	memset(TC0220IOCRegs, 0, 8);
	TC0220IOCPort = 0;
}

void TC0220IOCInit()
{
	TaitoIC_TC0220IOCInUse = 1;
}

void TC0220IOCExit()
{
	memset(TC0220IOCRegs, 0, 8);
	TC0220IOCPort = 0;
}

void TC0220IOCScan(INT32 nAction)
{
	if (nAction & ACB_DRIVER_DATA) {
		SCAN_VAR(TC0220IOCInputPort0);
		SCAN_VAR(TC0220IOCInputPort1);
		SCAN_VAR(TC0220IOCInputPort2);
		SCAN_VAR(TC0220IOCDip);
		SCAN_VAR(TC0220IOCInput);
		SCAN_VAR(TC0220IOCRegs);
		SCAN_VAR(TC0220IOCPort);
	}
}
