
typedef struct _eeprom_interface eeprom_interface;
struct _eeprom_interface
{
	INT32 address_bits;	/* EEPROM has 2^address_bits cells */
	INT32 data_bits;		/* every cell has this many bits (8 or 16) */
	const char *cmd_read;	/*   read command string, e.g. "0110" */
	const char *cmd_write;	/*  write command string, e.g. "0111" */
	const char *cmd_erase;	/*  erase command string, or 0 if n/a */
	const char *cmd_lock;	/*   lock command string, or 0 if n/a */
	const char *cmd_unlock;	/* unlock command string, or 0 if n/a */
	INT32 enable_multi_read;  /* set to 1 to enable multiple values to be read from one read command */
	INT32 reset_delay;	/* number of times eeprom_read_bit() should return 0 after a reset, */
				/* before starting to return 1. */
};

// default for most in fba
const eeprom_interface eeprom_interface_93C46 =
{
	6,		// address bits 6
	16,		// data bits    16
	"*110",		// read         1 10 aaaaaa
	"*101",		// write        1 01 aaaaaa dddddddddddddddd
	"*111",		// erase        1 11 aaaaaa
	"*10000xxxx",	// lock         1 00 00xxxx
	"*10011xxxx",	// unlock       1 00 11xxxx
	1,
	0
};

#define EEPROM_CLEAR_LINE	0
#define EEPROM_ASSERT_LINE	1
#define EEPROM_PULSE_LINE	2

void EEPROMInit(const eeprom_interface *interface);
void EEPROMReset();
void EEPROMExit();

INT32 EEPROMAvailable(); // are we loading an eeprom file?

INT32 EEPROMRead();

// Write each individually
void EEPROMWriteBit(INT32 bit);
void EEPROMSetCSLine(INT32 state);
void EEPROMSetClockLine(INT32 state);

// Or all at once
#define EEPROMWrite(clock, cs, bit)		\
	EEPROMWriteBit(bit);		\
	EEPROMSetCSLine(cs ? EEPROM_CLEAR_LINE : EEPROM_ASSERT_LINE);	\
	EEPROMSetClockLine(clock ? EEPROM_ASSERT_LINE : EEPROM_CLEAR_LINE)

void EEPROMFill(const UINT8 *data, INT32 offset, INT32 length);

void EEPROMScan(INT32 nAction, INT32* pnMin);
