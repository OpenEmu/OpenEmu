
#include "burnint.h"
#include "taito_m68705.h"

// taito m68705 protection

UINT8 portA_in;
UINT8 portA_out;
UINT8 ddrA;
UINT8 portB_in;
UINT8 portB_out;
UINT8 ddrB;
UINT8 portC_in;
UINT8 portC_out;
UINT8 ddrC;

UINT8 from_main;
UINT8 from_mcu;
INT32 mcu_sent;
INT32 main_sent;

static struct m68705_interface *ptr;

void m67805_mcu_write(UINT16 address, UINT8 data)
{
	switch (address & 0x7ff)
	{
		case 0x0000:
			if (ptr->portA_out) {
				ptr->portA_out(&data);
			}
			portA_out = data;
		return;
		
		case 0x0001:
			if (ptr->portB_out) {
				ptr->portB_out(&data);
			}
			portB_out = data;
		return;
		
		case 0x0002:
			if (ptr->portC_out) {
				ptr->portC_out(&data);
			}
			portC_out = data;
		return;
		
		case 0x0004:
			if (ptr->ddrA_out) {
				ptr->ddrA_out(&data);
			}
			ddrA = data;
		return;
		
		case 0x0005:
			if (ptr->ddrB_out) {
				ptr->ddrB_out(&data);
			}
			ddrB = data;
		return;
		
		case 0x0006:
			if (ptr->ddrC_out) {
				ptr->ddrC_out(&data);
			}
			ddrC = data;
		return;
	}
}

UINT8 m67805_mcu_read(UINT16 address)
{
	switch (address & 0x7ff)
	{
		case 0x0000:
			if (ptr->portA_in) {
				ptr->portA_in();
			}
			return (portA_out & ddrA) | (portA_in & ~ddrA);
		
		case 0x0001:
			if (ptr->portB_in) {
				ptr->portB_in();
			}
			return (portB_out & ddrB) | (portB_in & ~ddrB);
		
		case 0x0002:
			if (ptr->portC_in) {
				ptr->portC_in();
			}
			return (portC_out & ddrC) | (portC_in & ~ddrC);
	}

	return 0;
}

void m67805_taito_reset()
{
	m6805Open(0);
	m68705Reset();
	m6805Close();

	portA_in = 0;
	portA_out = 0;
	ddrA = 0;
	portB_in = 0;
	portB_out = 0;
	ddrB = 0;
	portC_in = 0;
	portC_out = 0;
	ddrC = 0;

	from_main = 0;
	from_mcu = 0;
	mcu_sent = 0;
	main_sent = 0;
}

void m67805_taito_init(UINT8 *rom, UINT8 *ram, m68705_interface *interface)
{
	ptr = interface;

	m6805Init(1, 0x800 /*max memory range - page size is max range / 0x100*/);
	m6805Open(0);
	m6805MapMemory(ram, 		0x0010, 0x007f, M6805_RAM);
	m6805MapMemory(rom + 0x80,	0x0080, 0x07ff, M6805_ROM);
	m6805SetWriteHandler(m67805_mcu_write);
	m6805SetReadHandler(m67805_mcu_read);
	m6805Close();
}

void m67805_taito_exit()
{
	portA_in = 0;
	portA_out = 0;
	ddrA = 0;
	portB_in = 0;
	portB_out = 0;
	ddrB = 0;
	portC_in = 0;
	portC_out = 0;
	ddrC = 0;

	from_main = 0;
	from_mcu = 0;
	mcu_sent = 0;
	main_sent = 0;

	ptr = NULL;

	m6805Exit();
}

INT32 m68705_taito_scan(INT32 nAction)
{
	if (nAction & ACB_VOLATILE) {		
		m6805Scan(nAction, 0);

		SCAN_VAR(portA_in);
		SCAN_VAR(portB_in);
		SCAN_VAR(portC_in);
		SCAN_VAR(ddrA);
		SCAN_VAR(ddrB);
		SCAN_VAR(ddrC);
		SCAN_VAR(portA_out);
		SCAN_VAR(portB_out);
		SCAN_VAR(portC_out);

		SCAN_VAR(from_main);
		SCAN_VAR(from_mcu);
		SCAN_VAR(mcu_sent);
		SCAN_VAR(main_sent);
	}

	return 0;
}





void standard_taito_mcu_write(INT32 data)
{
	from_main = data;
	main_sent = 1;
	m68705SetIrqLine(0, 1 /*ASSERT_LINE*/);
}

INT32 standard_taito_mcu_read()
{
	mcu_sent = 0;
	return from_mcu;
}

void standard_m68705_portB_out(UINT8 *data)
{
	if ((ddrB & 0x02) && (~*data & 0x02) && (portB_out & 0x02))
	{
		portA_in = from_main;
		if (main_sent)
			m68705SetIrqLine(0, 0 /*CLEAR_LINE*/);
		main_sent = 0;
	}
	if ((ddrB & 0x04) && (*data & 0x04) && (~portB_out & 0x04))
	{
		from_mcu = portA_out;
		mcu_sent = 1;
	}
}

void standard_m68705_portC_in()
{
	portC_in = 0;
	if (main_sent) portC_in |= 0x01;
	if (!mcu_sent) portC_in |= 0x02;
}

m68705_interface standard_m68705_interface = {
	NULL /* portA */, standard_m68705_portB_out /* portB */, NULL /* portC */,
	NULL /* ddrA  */, NULL                      /* ddrB  */, NULL /* ddrC  */,
	NULL /* portA */, NULL                      /* portB */, standard_m68705_portC_in  /* portC */
};

