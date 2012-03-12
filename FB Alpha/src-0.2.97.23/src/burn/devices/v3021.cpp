/* v3021 Calendar Emulation */

#include "burnint.h"

static UINT8 CalVal, CalMask, CalCom=0, CalCnt=0;

static UINT8 bcd(UINT8 data)
{
	return ((data / 10) << 4) | (data % 10);
}

UINT8 v3021Read()
{
	UINT8 calr;
	calr = (CalVal & CalMask) ? 1 : 0;
	CalMask <<= 1;
	return calr;
}

void v3021Write(UINT16 data)
{
	time_t nLocalTime = time(NULL);
	tm* tmLocalTime = localtime(&nLocalTime);

	CalCom <<= 1;
	CalCom |= data & 1;
	++CalCnt;
	if(CalCnt==4)
	{
		CalMask = 1;
		CalVal = 1;
		CalCnt = 0;
		
		switch(CalCom & 0xf)
		{
			case 0x1: case 0x3: case 0x5: case 0x7: case 0x9: case 0xb: case 0xd:
				CalVal++;
				break;

			case 0x0: // Day
				CalVal=bcd(tmLocalTime->tm_wday);
				break;

			case 0x2:  // Hours
				CalVal=bcd(tmLocalTime->tm_hour);
				break;

			case 0x4:  // Seconds
				CalVal=bcd(tmLocalTime->tm_sec);
				break;

			case 0x6:  // Month
				CalVal=bcd(tmLocalTime->tm_mon + 1); // not bcd in MVS
				break;

			case 0x8: // Milliseconds?
				CalVal=0; 
				break;

			case 0xa: // Day
				CalVal=bcd(tmLocalTime->tm_mday);
				break;

			case 0xc: // Minute
				CalVal=bcd(tmLocalTime->tm_min);
				break;

			case 0xe: // Year
				CalVal=bcd(tmLocalTime->tm_year % 100);
				break;

			case 0xf: // Load Date
				tmLocalTime = localtime(&nLocalTime);
				break;
		}
	}
}

INT32 v3021Scan()
{
	SCAN_VAR(CalVal);
	SCAN_VAR(CalMask);
	SCAN_VAR(CalCom);
	SCAN_VAR(CalCnt);

 	return 0;
}
