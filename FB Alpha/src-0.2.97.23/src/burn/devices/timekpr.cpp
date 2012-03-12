#include "burnint.h"
#include "time.h"
#include "timekpr.h"

typedef struct
{
	UINT8 control;
	UINT8 seconds;
	UINT8 minutes;
	UINT8 hours;
	UINT8 day;
	UINT8 date;
	UINT8 month;
	UINT8 year;
	UINT8 century;
	UINT8 *data;
	INT32 type;
	INT32 size;
	INT32 offset_control;
	INT32 offset_seconds;
	INT32 offset_minutes;
	INT32 offset_hours;
	INT32 offset_day;
	INT32 offset_date;
	INT32 offset_month;
	INT32 offset_year;
	INT32 offset_century;
	INT32 offset_flags;
} timekeeper_chip;

static timekeeper_chip Chip;

static INT32 AllocatedOwnDataArea = 0;

#define MASK_SECONDS ( 0x7f )
#define MASK_MINUTES ( 0x7f )
#define MASK_HOURS ( 0x3f )
#define MASK_DAY ( 0x07 )
#define MASK_DATE ( 0x3f )
#define MASK_MONTH ( 0x1f )
#define MASK_YEAR ( 0xff )
#define MASK_CENTURY ( 0xff )

#define CONTROL_W ( 0x80 )
#define CONTROL_R ( 0x40 )
#define CONTROL_S ( 0x20 ) /* not emulated */
#define CONTROL_CALIBRATION ( 0x1f ) /* not emulated */

#define SECONDS_ST ( 0x80 )

#define DAY_FT ( 0x40 ) /* not emulated */
#define DAY_CEB ( 0x20 ) /* M48T58 */
#define DAY_CB ( 0x10 ) /* M48T58 */

#define DATE_BLE ( 0x80 ) /* M48T58: not emulated */
#define DATE_BL ( 0x40 ) /* M48T58: not emulated */

#define FLAGS_BL ( 0x10 ) /* MK48T08: not emulated */

static inline UINT8 make_bcd(UINT8 data)
{
	return ( ( ( data / 10 ) % 10 ) << 4 ) + ( data % 10 );
}

static inline UINT8 from_bcd(UINT8 data)
{
	return ( ( ( data >> 4 ) & 15 ) * 10 ) + ( data & 15 );
}

static INT32 inc_bcd( UINT8 *data, INT32 mask, INT32 min, INT32 max )
{
	INT32 bcd;
	INT32 carry;

	bcd = ( *( data ) + 1 ) & mask;
	carry = 0;

	if( ( bcd & 0x0f ) > 9 )
	{
		bcd &= 0xf0;
		bcd += 0x10;
		if( bcd > max )
		{
			bcd = min;
			carry = 1;
		}
	}

	*( data ) = ( *( data ) & ~mask ) | ( bcd & mask );
	return carry;
}

static void counter_to_ram(UINT8 *data, INT32 offset, INT32 counter)
{
	if( offset >= 0 )
	{
		data[ offset ] = counter;
	}
}

static void counters_to_ram()
{
	counter_to_ram( Chip.data, Chip.offset_control, Chip.control );
	counter_to_ram( Chip.data, Chip.offset_seconds, Chip.seconds );
	counter_to_ram( Chip.data, Chip.offset_minutes, Chip.minutes );
	counter_to_ram( Chip.data, Chip.offset_hours, Chip.hours );
	counter_to_ram( Chip.data, Chip.offset_day, Chip.day );
	counter_to_ram( Chip.data, Chip.offset_date, Chip.date );
	counter_to_ram( Chip.data, Chip.offset_month, Chip.month );
	counter_to_ram( Chip.data, Chip.offset_year, Chip.year );
	counter_to_ram( Chip.data, Chip.offset_century, Chip.century );
}

static INT32 counter_from_ram(UINT8 *data, INT32 offset)
{
	if( offset >= 0 )
	{
		return data[ offset ];
	}
	return 0;
}

static void counters_from_ram()
{
	Chip.control = counter_from_ram( Chip.data, Chip.offset_control );
	Chip.seconds = counter_from_ram( Chip.data, Chip.offset_seconds );
	Chip.minutes = counter_from_ram( Chip.data, Chip.offset_minutes );
	Chip.hours = counter_from_ram( Chip.data, Chip.offset_hours );
	Chip.day = counter_from_ram( Chip.data, Chip.offset_day );
	Chip.date = counter_from_ram( Chip.data, Chip.offset_date );
	Chip.month = counter_from_ram( Chip.data, Chip.offset_month );
	Chip.year = counter_from_ram( Chip.data, Chip.offset_year );
	Chip.century = counter_from_ram( Chip.data, Chip.offset_century );
}

UINT8 TimeKeeperRead(UINT32 offset)
{
#if defined FBA_DEBUG
	if (!DebugDev_TimeKprInitted) bprintf(PRINT_ERROR, _T("TimeKeeperRead called without init\n"));
#endif

	return Chip.data[offset];
}

void TimeKeeperWrite(INT32 offset, UINT8 data)
{
#if defined FBA_DEBUG
	if (!DebugDev_TimeKprInitted) bprintf(PRINT_ERROR, _T("TimeKeeperWrite called without init\n"));
#endif

	if( offset == Chip.offset_control )
	{
		if( ( Chip.control & CONTROL_W ) != 0 &&
			( data & CONTROL_W ) == 0 )
		{
			counters_from_ram();
		}
		Chip.control = data;
	}
	else if( Chip.type == TIMEKEEPER_M48T58 && offset == Chip.offset_day )
	{
		Chip.day = ( Chip.day & ~DAY_CEB ) | ( data & DAY_CEB );
	}
	else if( Chip.type == TIMEKEEPER_M48T58 && offset == Chip.offset_date )
	{
		data &= ~DATE_BL;
	}
	else if( Chip.type == TIMEKEEPER_MK48T08 && offset == Chip.offset_flags )
	{
		data &= ~FLAGS_BL;
	}

	Chip.data[ offset ] = data;
}

void TimeKeeperTick()
{
#if defined FBA_DEBUG
	if (!DebugDev_TimeKprInitted) bprintf(PRINT_ERROR, _T("TimeKeeperTick called without init\n"));
#endif

	INT32 carry;

	if( ( Chip.seconds & SECONDS_ST ) != 0 ||
		( Chip.control & CONTROL_W ) != 0 )
	{
		return;
	}

	carry = inc_bcd( &Chip.seconds, MASK_SECONDS, 0x00, 0x59 );
	if( carry )
	{
		carry = inc_bcd( &Chip.minutes, MASK_MINUTES, 0x00, 0x59 );
	}
	if( carry )
	{
		carry = inc_bcd( &Chip.hours, MASK_HOURS, 0x00, 0x23 );
	}

	if( carry )
	{
		UINT8 month;
		UINT8 year;
		UINT8 maxdays;
		static const UINT8 daysinmonth[] = { 0x31, 0x28, 0x31, 0x30, 0x31, 0x30, 0x31, 0x31, 0x30, 0x31, 0x30, 0x31 };

		inc_bcd( &Chip.day, MASK_DAY, 0x01, 0x07 );

		month = from_bcd( Chip.month );
		year = from_bcd( Chip.year );

		if( month == 2 && ( year % 4 ) == 0 )
		{
			maxdays = 0x29;
		}
		else if( month >= 1 && month <= 12 )
		{
			maxdays = daysinmonth[ month - 1 ];
		}
		else
		{
			maxdays = 0x31;
		}

		carry = inc_bcd( &Chip.date, MASK_DATE, 0x01, maxdays );
	}
	if( carry )
	{
		carry = inc_bcd( &Chip.month, MASK_MONTH, 0x01, 0x12 );
	}
	if( carry )
	{
		carry = inc_bcd( &Chip.year, MASK_YEAR, 0x00, 0x99 );
	}
	if( carry )
	{
		carry = inc_bcd( &Chip.century, MASK_CENTURY, 0x00, 0x99 );
		if( Chip.type == TIMEKEEPER_M48T58 && ( Chip.day & DAY_CEB ) != 0 )
		{
			Chip.day ^= DAY_CB;
		}
	}

	if( ( Chip.control & CONTROL_R ) == 0 )
	{
		counters_to_ram();
	}
}

void TimeKeeperInit(INT32 type, UINT8 *data)
{
	DebugDev_TimeKprInitted = 1;
	
	time_t rawtime;
	struct tm *timeinfo;
	
	Chip.type = type;

	switch( Chip.type )
	{
	case TIMEKEEPER_M48T02:
		Chip.offset_control = 0x7f8;
		Chip.offset_seconds = 0x7f9;
		Chip.offset_minutes = 0x7fa;
		Chip.offset_hours = 0x7fb;
		Chip.offset_day = 0x7fc;
		Chip.offset_date = 0x7fd;
		Chip.offset_month = 0x7fe;
		Chip.offset_year = 0x7ff;
		Chip.offset_century = -1;
		Chip.offset_flags = -1;
		Chip.size = 0x800;
		break;
	case TIMEKEEPER_M48T58:
		Chip.offset_control = 0x1ff8;
		Chip.offset_seconds = 0x1ff9;
		Chip.offset_minutes = 0x1ffa;
		Chip.offset_hours = 0x1ffb;
		Chip.offset_day = 0x1ffc;
		Chip.offset_date = 0x1ffd;
		Chip.offset_month = 0x1ffe;
		Chip.offset_year = 0x1fff;
		Chip.offset_century = -1;
		Chip.offset_flags = -1;
		Chip.size = 0x2000;
		break;
	case TIMEKEEPER_MK48T08:
		Chip.offset_control = 0x1ff8;
		Chip.offset_seconds = 0x1ff9;
		Chip.offset_minutes = 0x1ffa;
		Chip.offset_hours = 0x1ffb;
		Chip.offset_day = 0x1ffc;
		Chip.offset_date = 0x1ffd;
		Chip.offset_month = 0x1ffe;
		Chip.offset_year = 0x1fff;
		Chip.offset_century = 0x1ff1;
		Chip.offset_flags = 0x1ff0;
		Chip.size = 0x2000;
		break;
	}

	if( data == NULL )
	{
		data = (UINT8*)malloc(Chip.size);
		memset(data, 0xff, Chip.size );
		AllocatedOwnDataArea = 1;
	}
	Chip.data = data;

	time(&rawtime);
	timeinfo = localtime(&rawtime);
	
	Chip.control = 0;
	Chip.seconds = make_bcd(timeinfo->tm_sec);
	Chip.minutes = make_bcd(timeinfo->tm_min);
	Chip.hours = make_bcd(timeinfo->tm_hour);
	Chip.day = make_bcd(timeinfo->tm_wday + 1 );
	Chip.date = make_bcd(timeinfo->tm_mday );
	Chip.month = make_bcd(timeinfo->tm_mon + 1 );
	Chip.year = make_bcd(timeinfo->tm_year % 100 );
	Chip.century = make_bcd(timeinfo->tm_year / 100 );
}

void TimeKeeperExit()
{
#if defined FBA_DEBUG
	if (!DebugDev_TimeKprInitted) bprintf(PRINT_ERROR, _T("TimeKeeperExit called without init\n"));
#endif

	if (AllocatedOwnDataArea) {
		free (Chip.data);
		Chip.data = NULL;
	}
	AllocatedOwnDataArea = 0;
	memset(&Chip, 0, sizeof(Chip));
	
	DebugDev_TimeKprInitted = 0;
}

void TimeKeeperScan(INT32 nAction)
{
#if defined FBA_DEBUG
	if (!DebugDev_TimeKprInitted) bprintf(PRINT_ERROR, _T("TimeKeeperScan called without init\n"));
#endif

	struct BurnArea ba;
	
	if (nAction & ACB_NVRAM) {
		memset(&ba, 0, sizeof(ba));
		ba.Data = Chip.data;
		ba.nLen = Chip.size;
		ba.szName = "Time Keeper RAM";
		BurnAcb(&ba);
	}
}
