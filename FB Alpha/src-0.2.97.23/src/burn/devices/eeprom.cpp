#include "burnint.h"
#include "eeprom.h"

#define SERIAL_BUFFER_LENGTH 40
#define MEMORY_SIZE 1024

static const eeprom_interface *intf;

static INT32 serial_count;
static UINT8 serial_buffer[SERIAL_BUFFER_LENGTH];
static UINT8 eeprom_data[MEMORY_SIZE];
static INT32 eeprom_data_bits;
static INT32 eeprom_read_address;
static INT32 eeprom_clock_count;
static INT32 latch, reset_line, clock_line, sending;
static INT32 locked;
static INT32 reset_delay;

static INT32 neeprom_available = 0;

static INT32 eeprom_command_match(const char *buf, const char *cmd, INT32 len)
{
	if ( cmd == 0 )	return 0;
	if ( len == 0 )	return 0;

	for (;len>0;)
	{
		char b = *buf;
		char c = *cmd;

		if ((b==0) || (c==0))
			return (b==c);

		switch ( c )
		{
			case '0':
			case '1':
				if (b != c)	return 0;
			case 'X':
			case 'x':
				buf++;
				len--;
				cmd++;
				break;

			case '*':
				c = cmd[1];
				switch( c )
				{
					case '0':
					case '1':
					  	if (b == c)	{	cmd++;			}
						else		{	buf++;	len--;	}
						break;
					default:	return 0;
				}
		}
	}
	return (*cmd==0);
}

INT32 EEPROMAvailable()
{
#if defined FBA_DEBUG
	if (!DebugDev_EEPROMInitted) bprintf(PRINT_ERROR, _T("EEPROMAvailable called without init\n"));
#endif

	return neeprom_available;
}

void EEPROMInit(const eeprom_interface *interface)
{
	DebugDev_EEPROMInitted = 1;
	
	intf = interface;

	if ((1 << intf->address_bits) * intf->data_bits / 8 > MEMORY_SIZE)
	{
		bprintf(0, _T("EEPROM larger than eeprom allows"));
	}

	memset(eeprom_data,0xff,(1 << intf->address_bits) * intf->data_bits / 8);
	serial_count = 0;
	latch = 0;
	reset_line = EEPROM_ASSERT_LINE;
	clock_line = EEPROM_ASSERT_LINE;
	eeprom_read_address = 0;
	sending = 0;
	if (intf->cmd_unlock) locked = 1;
	else locked = 0;

	char output[128];
	sprintf (output, "config/games/%s.nv", BurnDrvGetTextA(DRV_NAME));

	neeprom_available = 0;

	INT32 len = ((1 << intf->address_bits) * (intf->data_bits >> 3)) & (MEMORY_SIZE-1);

	FILE *fz = fopen(output, "rb");
	if (fz != NULL) {
		neeprom_available = 1;
		fread (eeprom_data, len, 1, fz);
		fclose (fz);
	}
}

void EEPROMExit()
{
#if defined FBA_DEBUG
	if (!DebugDev_EEPROMInitted) bprintf(PRINT_ERROR, _T("EEPROMExit called without init\n"));
#endif

	char output[128];
	sprintf (output, "config/games/%s.nv", BurnDrvGetTextA(DRV_NAME));

	neeprom_available = 0;

	INT32 len = ((1 << intf->address_bits) * (intf->data_bits >> 3)) & (MEMORY_SIZE-1);

	FILE *fz = fopen(output, "wb");
	fwrite (eeprom_data, len, 1, fz);
	fclose (fz);
	
	DebugDev_EEPROMInitted = 0;
}

static void eeprom_write(INT32 bit)
{
	if (serial_count >= SERIAL_BUFFER_LENGTH-1)
	{
		bprintf(0, _T("error: EEPROM serial buffer overflow\n"));
		return;
	}

	serial_buffer[serial_count++] = (bit ? '1' : '0');
	serial_buffer[serial_count] = 0;

	if ( (serial_count > intf->address_bits) &&
	      eeprom_command_match((char*)serial_buffer,intf->cmd_read,strlen((char*)serial_buffer)-intf->address_bits) )
	{
		INT32 i,address;

		address = 0;
		for (i = serial_count-intf->address_bits;i < serial_count;i++)
		{
			address <<= 1;
			if (serial_buffer[i] == '1') address |= 1;
		}
		if (intf->data_bits == 16)
			eeprom_data_bits = (eeprom_data[2*address+0] << 8) + eeprom_data[2*address+1];
		else
			eeprom_data_bits = eeprom_data[address];
		eeprom_read_address = address;
		eeprom_clock_count = 0;
		sending = 1;
		serial_count = 0;
	}
	else if ( (serial_count > intf->address_bits) &&
	           eeprom_command_match((char*)serial_buffer,intf->cmd_erase,strlen((char*)serial_buffer)-intf->address_bits) )
	{
		INT32 i,address;

		address = 0;
		for (i = serial_count-intf->address_bits;i < serial_count;i++)
		{
			address <<= 1;
			if (serial_buffer[i] == '1') address |= 1;
		}

		if (locked == 0)
		{
			if (intf->data_bits == 16)
			{
				eeprom_data[2*address+0] = 0xff;
				eeprom_data[2*address+1] = 0xff;
			}
			else
				eeprom_data[address] = 0xff;
		}
		else
			serial_count = 0;
	}
	else if ( (serial_count > (intf->address_bits + intf->data_bits)) &&
	           eeprom_command_match((char*)serial_buffer,intf->cmd_write,strlen((char*)serial_buffer)-(intf->address_bits + intf->data_bits)) )
	{
		INT32 i,address,data;

		address = 0;
		for (i = serial_count-intf->data_bits-intf->address_bits;i < (serial_count-intf->data_bits);i++)
		{
			address <<= 1;
			if (serial_buffer[i] == '1') address |= 1;
		}
		data = 0;
		for (i = serial_count-intf->data_bits;i < serial_count;i++)
		{
			data <<= 1;
			if (serial_buffer[i] == '1') data |= 1;
		}

		if (locked == 0)
		{
			if (intf->data_bits == 16)
			{
				eeprom_data[2*address+0] = data >> 8;
				eeprom_data[2*address+1] = data & 0xff;
			}
			else
				eeprom_data[address] = data;
		}
		else
			serial_count = 0;
	}
	else if ( eeprom_command_match((char*)serial_buffer,intf->cmd_lock,strlen((char*)serial_buffer)) )
	{
		locked = 1;
		serial_count = 0;
	}
	else if ( eeprom_command_match((char*)serial_buffer,intf->cmd_unlock,strlen((char*)serial_buffer)) )
	{
		locked = 0;
		serial_count = 0;
	}
}

void EEPROMReset()
{
#if defined FBA_DEBUG
	if (!DebugDev_EEPROMInitted) bprintf(PRINT_ERROR, _T("EEPROMReset called without init\n"));
#endif

	serial_count = 0;
	sending = 0;
	reset_delay = intf->reset_delay;
}

void EEPROMWriteBit(INT32 bit)
{
#if defined FBA_DEBUG
	if (!DebugDev_EEPROMInitted) bprintf(PRINT_ERROR, _T("EEPROMWriteBit called without init\n"));
#endif

	latch = bit;
}

INT32 EEPROMRead()
{
#if defined FBA_DEBUG
	if (!DebugDev_EEPROMInitted) bprintf(PRINT_ERROR, _T("EEPROMRead called without init\n"));
#endif

	INT32 res;

	if (sending)
		res = (eeprom_data_bits >> intf->data_bits) & 1;
	else
	{
		if (reset_delay > 0)
		{
			/* this is needed by wbeachvl */
			reset_delay--;
			res = 0;
		}
		else
			res = 1;
	}

	return res;
}

void EEPROMSetCSLine(INT32 state)
{
#if defined FBA_DEBUG
	if (!DebugDev_EEPROMInitted) bprintf(PRINT_ERROR, _T("EEPROMSetCSLine called without init\n"));
#endif

	reset_line = state;

	if (reset_line != EEPROM_CLEAR_LINE)
		EEPROMReset();
}

void EEPROMSetClockLine(INT32 state)
{
#if defined FBA_DEBUG
	if (!DebugDev_EEPROMInitted) bprintf(PRINT_ERROR, _T("EEPROMSetClockLine called without init\n"));
#endif

	if (state == EEPROM_PULSE_LINE || (clock_line == EEPROM_CLEAR_LINE && state != EEPROM_CLEAR_LINE))
	{
		if (reset_line == EEPROM_CLEAR_LINE)
		{
			if (sending)
			{
				if (eeprom_clock_count == intf->data_bits && intf->enable_multi_read)
				{
					eeprom_read_address = (eeprom_read_address + 1) & ((1 << intf->address_bits) - 1);
					if (intf->data_bits == 16)
						eeprom_data_bits = (eeprom_data[2*eeprom_read_address+0] << 8) + eeprom_data[2*eeprom_read_address+1];
					else
						eeprom_data_bits = eeprom_data[eeprom_read_address];
					eeprom_clock_count = 0;
				}
				eeprom_data_bits = (eeprom_data_bits << 1) | 1;
				eeprom_clock_count++;
			}
			else
				eeprom_write(latch);
		}
	}

	clock_line = state;
}

void EEPROMFill(const UINT8 *data, INT32 offset, INT32 length)
{
#if defined FBA_DEBUG
	if (!DebugDev_EEPROMInitted) bprintf(PRINT_ERROR, _T("EEPROMFill called without init\n"));
#endif

	memcpy(eeprom_data + offset, data, length);
}

void EEPROMScan(INT32 nAction, INT32* pnMin)
{
#if defined FBA_DEBUG
	if (!DebugDev_EEPROMInitted) bprintf(PRINT_ERROR, _T("EEPROMScan called without init\n"));
#endif

	struct BurnArea ba;

	if (nAction & ACB_DRIVER_DATA) {

		if (pnMin && *pnMin < 0x020902) {
			*pnMin = 0x029705;
		}

		memset(&ba, 0, sizeof(ba));
    		ba.Data		= serial_buffer;
		ba.nLen		= SERIAL_BUFFER_LENGTH;
		ba.szName	= "Serial Buffer";
		BurnAcb(&ba);

		SCAN_VAR(serial_count);
		SCAN_VAR(eeprom_data_bits);
		SCAN_VAR(eeprom_read_address);
		SCAN_VAR(eeprom_clock_count);
		SCAN_VAR(latch);
		SCAN_VAR(reset_line);
		SCAN_VAR(clock_line);
		SCAN_VAR(sending);
		SCAN_VAR(locked);
		SCAN_VAR(reset_delay);
	}

//	if (nAction & ACB_NVRAM) {
//
//		if (pnMin && (nAction & ACB_TYPEMASK) == ACB_NVRAM) {
//			*pnMin = 0x02705;
//		}
//
//		memset(&ba, 0, sizeof(ba));
//  		ba.Data		= eeprom_data;
//		ba.nLen		= MEMORY_SIZE;
//		ba.szName	= "EEPROM memory";
//		BurnAcb(&ba);
//	}
}
