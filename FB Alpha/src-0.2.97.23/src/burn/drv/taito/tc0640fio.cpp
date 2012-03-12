// TC0640FIO

#include "burnint.h"
#include "taito_ic.h"

UINT8 TC0640FIOInputPort0[8] = { 0, 0, 0, 0, 0, 0, 0, 0 };
UINT8 TC0640FIOInputPort1[8] = { 0, 0, 0, 0, 0, 0, 0, 0 };
UINT8 TC0640FIOInputPort2[8] = { 0, 0, 0, 0, 0, 0, 0, 0 };
UINT8 TC0640FIOInputPort3[8] = { 0, 0, 0, 0, 0, 0, 0, 0 };
UINT8 TC0640FIOInputPort4[8] = { 0, 0, 0, 0, 0, 0, 0, 0 };
UINT8 TC0640FIOInput[5]      = { 0, 0, 0, 0, 0 };

static UINT8 TC0640FIORegs[8];

UINT8 TC0640FIORead(UINT32 Offset)
{
	switch (Offset) {
		case 0x00: {
			return TC0640FIOInput[0];
		}

		case 0x01: {
			return TC0640FIOInput[1];
		}

		case 0x02: {
			return TC0640FIOInput[2];
		}

		case 0x03: {
			return TC0640FIOInput[3];
		}

		case 0x04: {
			return TC0640FIORegs[4];
		}

		case 0x07: {
			return TC0640FIOInput[4];
		}
	}
	
	return 0xff;
}

void TC0640FIOWrite(UINT32 Offset, UINT8 Data)
{
	TC0640FIORegs[Offset] = Data;
	
	switch (Offset) {
		case 0x00: {
			TaitoWatchdog = 0;
			return;
		}

		case 0x04: {
			// coin lockout and counters
			return;
		}
	}
}

UINT16 TC0640FIOHalfWordRead(UINT32 Offset)
{
	return TC0640FIORead(Offset);
}

void TC0640FIOHalfWordWrite(UINT32 Offset, UINT16 Data)
{
	if ((Data & 0xff00) == 0) {
		TC0640FIOWrite(Offset, Data & 0xff);
	} else {		
		TC0640FIOWrite(Offset, (Data >> 8) & 0xff);
	}
}

UINT16 TC0640FIOHalfWordByteswapRead(UINT32 Offset)
{
	return TC0640FIOHalfWordRead(Offset) << 8;
}

void TC0640FIOHalfWordByteswapWrite(UINT32 Offset, UINT16 Data)
{
	if ((Data & 0xff00) == 0) {
		TC0640FIOWrite(Offset, (Data >> 8) & 0xff);
	} else {
		TC0640FIOWrite(Offset, Data & 0xff);
	}
}

void TC0640FIOReset()
{
	memset(TC0640FIORegs, 0, 8);
}

void TC0640FIOInit()
{
	TaitoIC_TC0640FIOInUse = 1;
}

void TC0640FIOExit()
{
	memset(TC0640FIORegs, 0, 8);
}

void TC0640FIOScan(INT32 nAction)
{
	if (nAction & ACB_DRIVER_DATA) {
		SCAN_VAR(TC0640FIOInputPort0);
		SCAN_VAR(TC0640FIOInputPort1);
		SCAN_VAR(TC0640FIOInputPort2);
		SCAN_VAR(TC0640FIOInputPort3);
		SCAN_VAR(TC0640FIOInputPort4);
		SCAN_VAR(TC0640FIOInput);
		SCAN_VAR(TC0640FIORegs);
	}
}
