// NEC uPD4990A module (real-time clock/calendar)

#include "neogeo.h"

static struct {
	// The current time
	UINT32 nSeconds; UINT32 nMinutes; UINT32 nHours;
	UINT32 nDay; UINT32 nMonth; UINT32 nYear;
	UINT32 nWeekDay;

	// Modes for both outputs
	INT32 nMode; INT32 nTPMode;

	// Shift register and command
	UINT32 nRegister[2]; UINT32 nCommand;

	// Counters
	UINT32 nCount; UINT32 nTPCount; UINT32 nInterval;

	// Outputs
	UINT8 TP; UINT8 nPrevCLK; UINT8 nPrevSTB;
} uPD4990A;

static UINT32 nOneSecond;

INT32 uPD4990AInit(UINT32 nTicksPerSecond)
{
	nOneSecond = nTicksPerSecond;

	uPD4990A.nRegister[0] = uPD4990A.nRegister[1] = 0;

	uPD4990A.nCommand = 0;
	uPD4990A.nMode = uPD4990A.nTPMode = 0;

	uPD4990A.nCount = uPD4990A.nTPCount = 0;
	uPD4990A.nInterval = nOneSecond / 64;

	uPD4990A.nPrevCLK = uPD4990A.nPrevSTB = 0;

	uPD4990A.TP = 0;

	// Set the time of the uPD4990A to the current local time
	time_t nLocalTime = time(NULL);
	tm* tmLocalTime = localtime(&nLocalTime);

	uPD4990A.nSeconds = tmLocalTime->tm_sec;
	uPD4990A.nMinutes = tmLocalTime->tm_min;
	uPD4990A.nHours   = tmLocalTime->tm_hour;
	uPD4990A.nDay     = tmLocalTime->tm_mday;
	uPD4990A.nWeekDay = tmLocalTime->tm_wday;
	uPD4990A.nMonth   = tmLocalTime->tm_mon + 1;
	uPD4990A.nYear    = tmLocalTime->tm_year % 100;

	return 0;
}

void uPD499ASetTicks(UINT32 nTicksPerSecond)
{
	uPD4990A.nCount    = (UINT32)((INT64)uPD4990A.nCount    * nTicksPerSecond / nOneSecond);
	uPD4990A.nTPCount  = (UINT32)((INT64)uPD4990A.nTPCount  * nTicksPerSecond / nOneSecond);
	uPD4990A.nInterval = (UINT32)((INT64)uPD4990A.nInterval * nTicksPerSecond / nOneSecond);

	nOneSecond = nTicksPerSecond;
}

void uPD4990AExit()
{
}

void uPD4990AUpdate(UINT32 nTicks)
{
	if (uPD4990A.nTPMode != 2) {
		uPD4990A.nTPCount += nTicks;

		if (uPD4990A.nTPMode == 1) {
			if (uPD4990A.nTPCount >= uPD4990A.nInterval) {
				uPD4990A.nTPMode = 0;
				uPD4990A.nTPCount %= uPD4990A.nInterval;
				uPD4990A.TP = (uPD4990A.nTPCount >= (uPD4990A.nInterval >> 1));
			}
		} else {
			if (uPD4990A.nTPCount >= uPD4990A.nInterval) {
				uPD4990A.nTPCount %= uPD4990A.nInterval;
			}
			uPD4990A.TP = (uPD4990A.nTPCount >= (uPD4990A.nInterval >> 1));
		}
	}

	uPD4990A.nCount += nTicks;
	if (uPD4990A.nCount >= nOneSecond) {
		uPD4990A.nCount %= uPD4990A.nInterval;

		uPD4990A.nSeconds++;
		if (uPD4990A.nSeconds >= 60) {
			uPD4990A.nSeconds = 0;

			uPD4990A.nMinutes++;
			if (uPD4990A.nMinutes >= 60) {
				uPD4990A.nMinutes = 0;

				uPD4990A.nHours++;
				if (uPD4990A.nHours >= 24) {
					uPD4990A.nHours = 0;

					uPD4990A.nWeekDay++;
					if (uPD4990A.nWeekDay >= 7) {
						uPD4990A.nWeekDay = 0;
					}

					UINT32 nMonthLength[12] = { 31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31 };
					if ((uPD4990A.nYear & 3) == 0) {
						nMonthLength[2]++;
					}

					uPD4990A.nDay++;
					if (uPD4990A.nDay > nMonthLength[uPD4990A.nMonth]) {
						uPD4990A.nDay = 1;

						uPD4990A.nMonth++;
						if (uPD4990A.nMonth > 12) {
							uPD4990A.nMonth = 1;

							uPD4990A.nYear++;
							if (uPD4990A.nYear >= 100) {
								uPD4990A.nYear = 0;
							}
						}
					}
				}
			}
		}
	}
}

void uPD4990AScan(INT32 nAction, INT32* pnMin)
{
	if (nAction & ACB_DRIVER_DATA) {							// Scan variables

		if (pnMin && *pnMin < 0x020902) {						// Return minimum compatible version
			*pnMin = 0x020902;
		}

		SCAN_VAR(uPD4990A);
	}
}

void uPD4990AWrite(UINT8 CLK, UINT8 STB, UINT8 DATA)
{
//	bprintf(PRINT_NORMAL, _T("  - uPD4990A written: CLK: %i, STB: %i DATA IN: %i (PC: %06X).\n"), CLK ? 1 : 0, STB ? 1 : 0, DATA ? 1 : 0, SekGetPC(-1));

	if (STB && uPD4990A.nPrevSTB == 0) {						// Process command

//		bprintf(PRINT_NORMAL, _T("  - Command sent: %02X.\n"), nCommand & 0x0F);

		switch (uPD4990A.nCommand & 0x0F) {
			case 0x00:											// Register hold
				uPD4990A.nMode = 0;

				uPD4990A.nTPMode = 0;
				uPD4990A.nInterval = nOneSecond / 64;
				uPD4990A.nTPCount %= uPD4990A.nInterval;
				break;
			case 0x01:											// Register shift
				uPD4990A.nMode = 1;
				break;
			case 0x02:											// Time set & counter hold
				uPD4990A.nMode = 2;

//				bprintf(PRINT_NORMAL, _T("Time set: %08X %08X.\n"), nRegister[0], nRegister[1]);

				// Convert BCD values to normal numbers
				uPD4990A.nSeconds  = ((uPD4990A.nRegister[0] >>  0) & 0x0F);
				uPD4990A.nSeconds += ((uPD4990A.nRegister[0] >>  4) & 0x0F) * 10;
				uPD4990A.nMinutes  = ((uPD4990A.nRegister[0] >>  8) & 0x0F);
				uPD4990A.nMinutes += ((uPD4990A.nRegister[0] >> 12) & 0x0F) * 10;
				uPD4990A.nHours    = ((uPD4990A.nRegister[0] >> 16) & 0x0F);
				uPD4990A.nHours   += ((uPD4990A.nRegister[0] >> 20) & 0x0F) * 10;
				uPD4990A.nDay      = ((uPD4990A.nRegister[0] >> 24) & 0x0F);
				uPD4990A.nDay     += ((uPD4990A.nRegister[0] >> 28) & 0x0F) * 10;
				uPD4990A.nWeekDay  = ((uPD4990A.nRegister[1] >>  0) & 0x0F);
				uPD4990A.nMonth    = ((uPD4990A.nRegister[1] >>  4) & 0x0F);
				uPD4990A.nYear     = ((uPD4990A.nRegister[1] >>  8) & 0x0F);
				uPD4990A.nYear    += ((uPD4990A.nRegister[1] >> 12) & 0x0F) * 10;
				break;
			case 0x03:											// Time read
				uPD4990A.nMode = 0;

				// Convert normal numbers to BCD values
				uPD4990A.nRegister[0]  = (uPD4990A.nSeconds % 10) <<  0;
				uPD4990A.nRegister[0] |= (uPD4990A.nSeconds / 10) <<  4;
				uPD4990A.nRegister[0] |= (uPD4990A.nMinutes % 10) <<  8;
				uPD4990A.nRegister[0] |= (uPD4990A.nMinutes / 10) << 12;
				uPD4990A.nRegister[0] |= (uPD4990A.nHours   % 10) << 16;
				uPD4990A.nRegister[0] |= (uPD4990A.nHours   / 10) << 20;
				uPD4990A.nRegister[0] |= (uPD4990A.nDay     % 10) << 24;
				uPD4990A.nRegister[0] |= (uPD4990A.nDay     / 10) << 28;
				uPD4990A.nRegister[1]  = (uPD4990A.nWeekDay     ) <<  0;
				uPD4990A.nRegister[1] |= (uPD4990A.nMonth       ) <<  4;
				uPD4990A.nRegister[1] |= (uPD4990A.nYear    % 10) <<  8;
				uPD4990A.nRegister[1] |= (uPD4990A.nYear    / 10) << 12;
				break;

			case 0x04:
			case 0x05:
			case 0x06:
			case 0x07: {										// TP = nn Hz
				INT32 n[4] = { 64, 256, 2048, 4096 };

				uPD4990A.nTPMode = 0;
				uPD4990A.nInterval = nOneSecond / n[uPD4990A.nCommand & 3];
				uPD4990A.nTPCount %= uPD4990A.nInterval;
				break;
			}

			case 0x08:
			case 0x09:
			case 0x0A:
			case 0x0B: {										// TP = nn s interval set (counter reset & start)
				INT32 n[4] = { 1, 10, 30, 60 };

				uPD4990A.nTPMode = 0;
				uPD4990A.nInterval = n[uPD4990A.nCommand & 3] * nOneSecond;
				uPD4990A.nTPCount = 0;
				break;
			}
			case 0x0C:											// Interval reset
				uPD4990A.nTPMode = 1;
				uPD4990A.TP = 1;
				break;
			case 0x0D:											// interval start
				uPD4990A.nTPMode = 0;
				break;
			case 0x0E:											// Interval stop
				uPD4990A.nTPMode = 2;
				break;

			case 0x0F:											// Test mode set (not implemented)
				break;
		}
	}

	if (STB == 0 && CLK && uPD4990A.nPrevCLK == 0) {

		// Shift a new bit into the register
		if (uPD4990A.nMode == 1) {
			uPD4990A.nRegister[0] >>= 1;
			if (uPD4990A.nRegister[1] & 1) {
				uPD4990A.nRegister[0] |= (1 << 31);
			}
			uPD4990A.nRegister[1] >>= 1;
			uPD4990A.nRegister[1] &= 0x7FFF;
			if (uPD4990A.nCommand & 1) {
				uPD4990A.nRegister[1] |= (1 << 15);
			}
		}

		// Shift a new bit into the command
		uPD4990A.nCommand >>= 1;
		uPD4990A.nCommand &= 7;
		if (DATA) {
			uPD4990A.nCommand |= 8;
		}
	}

	uPD4990A.nPrevCLK = CLK;
	uPD4990A.nPrevSTB = STB;
}

UINT8 uPD4990ARead(UINT32 nTicks)
{
	UINT8 OUT;

	uPD4990AUpdate(nTicks);

	if (uPD4990A.nMode == 0) {								// 1Hz pulse appears at output
		OUT = (uPD4990A.nCount >= (nOneSecond >> 1));
	} else {												// LSB of the shift register appears at output
		OUT = uPD4990A.nRegister[0] & 1;
	}

//	bprintf(PRINT_NORMAL, _T("  - uPD4990A read: OUT %i, TP %i.\n"), OUT, uPD4990A.TP);

	return (OUT << 1) | uPD4990A.TP;
}

