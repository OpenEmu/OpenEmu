#include "burnint.h"
#include "8255ppi.h"

#define MAX_PPIS		3

PPIPortRead PPI0PortReadA;
PPIPortRead PPI0PortReadB;
PPIPortRead PPI0PortReadC;
PPIPortWrite PPI0PortWriteA;
PPIPortWrite PPI0PortWriteB;
PPIPortWrite PPI0PortWriteC;
PPIPortRead PPI1PortReadA;
PPIPortRead PPI1PortReadB;
PPIPortRead PPI1PortReadC;
PPIPortWrite PPI1PortWriteA;
PPIPortWrite PPI1PortWriteB;
PPIPortWrite PPI1PortWriteC;
PPIPortRead PPI2PortReadA;
PPIPortRead PPI2PortReadB;
PPIPortRead PPI2PortReadC;
PPIPortWrite PPI2PortWriteA;
PPIPortWrite PPI2PortWriteB;
PPIPortWrite PPI2PortWriteC;

typedef struct
{
	/* mode flags */
	UINT8 groupA_mode;
	UINT8 groupB_mode;
	UINT8 portA_dir;
	UINT8 portB_dir;
	UINT8 portCH_dir;
	UINT8 portCL_dir;

	/* handshake signals (1=asserted; 0=non-asserted) */
	UINT8 obf_a;
	UINT8 obf_b;
	UINT8 ibf_a;
	UINT8 ibf_b;
	UINT8 inte_a;
	UINT8 inte_b;

	UINT8 in_mask[3];		/* input mask */
	UINT8 out_mask[3];	/* output mask */
	UINT8 read[3];		/* data read from ports */
	UINT8 latch[3];		/* data written to ports */
	UINT8 output[3];	/* actual output data */
} ppi8255;

static ppi8255 chips[MAX_PPIS];
static INT32 nNumChips = 0;

static void ppi8255_get_handshake_signals(ppi8255 *chip, UINT8 *result)
{
	UINT8 handshake = 0x00;
	UINT8 mask = 0x00;

	/* group A */
	if (chip->groupA_mode == 1)
	{
		if (chip->portA_dir)
		{
			handshake |= chip->ibf_a ? 0x20 : 0x00;
			handshake |= (chip->ibf_a && chip->inte_a) ? 0x08 : 0x00;
			mask |= 0x28;
		}
		else
		{
			handshake |= chip->obf_a ? 0x00 : 0x80;
			handshake |= (chip->obf_a && chip->inte_a) ? 0x08 : 0x00;
			mask |= 0x88;
		}
	}
	else if (chip->groupA_mode == 2)
  	{
		handshake |= chip->inte_a ? 0x08 : 0x00;
		handshake |= chip->obf_a ? 0x00 : 0x80;
		handshake |= chip->ibf_a ? 0x20 : 0x00;
		mask |= 0xA8;
	}

	/* group B */
	if (chip->groupB_mode == 1)
  	{
		if (chip->portA_dir)
		{
			handshake |= chip->ibf_b ? 0x02 : 0x00;
			handshake |= (chip->ibf_b && chip->inte_b) ? 0x01 : 0x00;
			mask |= 0x03;
  		}
		else
  		{
			handshake |= chip->obf_b ? 0x00 : 0x02;
			handshake |= (chip->obf_b && chip->inte_b) ? 0x01 : 0x00;
			mask |= 0x03;
		}
  	}

	*result &= ~mask;
	*result |= handshake & mask;
}

static void ppi8255_write_port(ppi8255 *chip, INT32 port, INT32 chipnum)
{
	UINT8 write_data;

	write_data = chip->latch[port] & chip->out_mask[port];
	write_data |= 0xFF & ~chip->out_mask[port];

	/* write out special port 2 signals */
	if (port == 2)
		ppi8255_get_handshake_signals(chip, &write_data);

	chip->output[port] = write_data;
	
	if (chipnum == 0 && port == 0) {
		if (PPI0PortWriteA) PPI0PortWriteA(write_data);
	}
	
	if (chipnum == 0 && port == 1) {
		if (PPI0PortWriteB) PPI0PortWriteB(write_data);
	}
	
	if (chipnum == 0 && port == 2) {
		if (PPI0PortWriteC) PPI0PortWriteC(write_data);
	}
	
	if (chipnum == 1 && port == 0) {
		if (PPI1PortWriteA) PPI1PortWriteA(write_data);
	}
	
	if (chipnum == 1 && port == 1) {
		if (PPI1PortWriteB) PPI1PortWriteB(write_data);
	}
	
	if (chipnum == 1 && port == 2) {
		if (PPI1PortWriteC) PPI1PortWriteC(write_data);
	}
	
	if (chipnum == 2 && port == 0) {
		if (PPI2PortWriteA) PPI2PortWriteA(write_data);
	}
	
	if (chipnum == 2 && port == 1) {
		if (PPI2PortWriteB) PPI2PortWriteB(write_data);
	}
	
	if (chipnum == 2 && port == 2) {
		if (PPI2PortWriteC) PPI2PortWriteC(write_data);
	}
}

static void ppi8255_input(ppi8255 *chip, INT32 port, UINT8 data, INT32 which)
{
	INT32 changed = 0;

	chip->read[port] = data;

	/* port C is special */
	if (port == 2)
	{
		if (((chip->groupA_mode == 1) && (chip->portA_dir == 0)) || (chip->groupA_mode == 2))
		{
			/* is !ACKA asserted? */
			if (chip->obf_a && !(data & 0x40))
			{
				chip->obf_a = 0;
				changed = 1;
			}
		}

		if ((chip->groupB_mode == 1) && (chip->portB_dir == 0))
		{
			/* is !ACKB asserted? */
			if (chip->obf_b && !(data & 0x04))
			{
				chip->obf_b = 0;
				changed = 1;
			}
		}

		if (changed)
			ppi8255_write_port(chip, 2, which);
	}
}

static UINT8 ppi8255_read_port(ppi8255 *chip, INT32 port, INT32 chipnum)
{
	UINT8 result = 0x00;

	if (chip->in_mask[port])
	{
		if (chipnum == 0 && port == 0) {
			ppi8255_input(chip, port, (PPI0PortReadA) ? PPI0PortReadA() : 0, chipnum);
		}
		
		if (chipnum == 0 && port == 1) {
			ppi8255_input(chip, port, (PPI0PortReadB) ? PPI0PortReadB() : 0, chipnum);
		}
		
		if (chipnum == 0 && port == 2) {
			ppi8255_input(chip, port, (PPI0PortReadC) ? PPI0PortReadC() : 0, chipnum);
		}
		
		if (chipnum == 1 && port == 0) {
			ppi8255_input(chip, port, (PPI1PortReadA) ? PPI1PortReadA() : 0, chipnum);
		}
		
		if (chipnum == 1 && port == 1) {
			ppi8255_input(chip, port, (PPI1PortReadB) ? PPI1PortReadB() : 0, chipnum);
		}
		
		if (chipnum == 1 && port == 2) {
			ppi8255_input(chip, port, (PPI1PortReadC) ? PPI1PortReadC() : 0, chipnum);
		}
		
		if (chipnum == 2 && port == 0) {
			ppi8255_input(chip, port, (PPI2PortReadA) ? PPI2PortReadA() : 0, chipnum);
		}
		
		if (chipnum == 2 && port == 1) {
			ppi8255_input(chip, port, (PPI2PortReadB) ? PPI2PortReadB() : 0, chipnum);
		}
		
		if (chipnum == 2 && port == 2) {
			ppi8255_input(chip, port, (PPI2PortReadC) ? PPI2PortReadC() : 0, chipnum);
		}

		result |= chip->read[port] & chip->in_mask[port];
	}
	result |= chip->latch[port] & chip->out_mask[port];

	/* read special port 2 signals */
	if (port == 2)
		ppi8255_get_handshake_signals(chip, &result);

	return result;
}

UINT8 ppi8255_r(INT32 which, INT32 offset)
{
#if defined FBA_DEBUG
	if (!DebugDev_8255PPIInitted) bprintf(PRINT_ERROR, _T("ppi8255_r called without init\n"));
	if (which > nNumChips) bprintf(PRINT_ERROR, _T("ppi8255_r called with invalid chip %x\n"), which);
#endif

	ppi8255 *chip = &chips[which];
	UINT8 result = 0;

	offset %= 4;

	switch(offset)
	{
		case 0: /* Port A read */
		case 1: /* Port B read */
		case 2: /* Port C read */
			result = ppi8255_read_port(chip, offset, which);
			break;

		case 3: /* Control word */
			result = 0xFF;
			break;
	}

	return result;
}

static void set_mode(INT32 which, INT32 data, INT32 call_handlers)
{
	ppi8255 *chip = &chips[which];
	INT32 i;

	/* parse out mode */
	chip->groupA_mode = (data >> 5) & 3;
	chip->groupB_mode = (data >> 2) & 1;
	chip->portA_dir = (data >> 4) & 1;
	chip->portB_dir = (data >> 1) & 1;
	chip->portCH_dir = (data >> 3) & 1;
	chip->portCL_dir = (data >> 0) & 1;
	
	/* normalize groupA_mode */
	if (chip->groupA_mode == 3)
		chip->groupA_mode = 2;

  	/* Port A direction */
	if (chip->portA_dir)
		chip->in_mask[0] = 0xFF, chip->out_mask[0] = 0x00;	/* input */
    else
		chip->in_mask[0] = 0x00, chip->out_mask[0] = 0xFF; 	/* output */

  	/* Port B direction */
	if (chip->portB_dir)
		chip->in_mask[1] = 0xFF, chip->out_mask[1] = 0x00;	/* input */
	else
		chip->in_mask[1] = 0x00, chip->out_mask[1] = 0xFF; 	/* output */

	/* Port C upper direction */
	if (chip->portCH_dir)
		chip->in_mask[2] = 0xF0, chip->out_mask[2] = 0x00;	/* input */
	else
		chip->in_mask[2] = 0x00, chip->out_mask[2] = 0xF0;	/* output */

  	/* Port C lower direction */
	if (chip->portCL_dir)
		chip->in_mask[2] |= 0x0F;	/* input */
	else
		chip->out_mask[2] |= 0x0F;	/* output */

	/* now depending on the group modes, certain Port C lines may be replaced
     * with varying control signals */
	switch(chip->groupA_mode)
	{
		case 0:	/* Group A mode 0 */
			/* no changes */
			break;

		case 1:	/* Group A mode 1 */
			/* bits 5-3 are reserved by Group A mode 1 */
			chip->in_mask[2] &= ~0x38;
			chip->out_mask[2] &= ~0x38;
			break;

		case 2: /* Group A mode 2 */
			/* bits 7-3 are reserved by Group A mode 2 */
			chip->in_mask[2] &= ~0xF8;
			chip->out_mask[2] &= ~0xF8;
			break;
	}

	switch(chip->groupB_mode)
	{
		case 0:	/* Group B mode 0 */
			/* no changes */
			break;

		case 1:	/* Group B mode 1 */
			/* bits 2-0 are reserved by Group B mode 1 */
			chip->in_mask[2] &= ~0x07;
			chip->out_mask[2] &= ~0x07;
			break;
	}
	
	/* KT: 25-Dec-99 - 8255 resets latches when mode set */
	chip->latch[0] = chip->latch[1] = chip->latch[2] = 0;

	if (call_handlers)
	{
		for (i = 0; i < 3; i++)
			ppi8255_write_port(chip, i, which);
	}
}

void ppi8255_w(INT32 which, INT32 offset, UINT8 data)
{
#if defined FBA_DEBUG
	if (!DebugDev_8255PPIInitted) bprintf(PRINT_ERROR, _T("ppi8255_w called without init\n"));
	if (which > nNumChips) bprintf(PRINT_ERROR, _T("ppi8255_w called with invalid chip %x\n"), which);
#endif

	ppi8255	*chip = &chips[which];
	
	offset %= 4;
	
  	switch( offset )
  	{
	  	case 0: /* Port A write */
		case 1: /* Port B write */
  		case 2: /* Port C write */
			chip->latch[offset] = data;
			ppi8255_write_port(chip, offset, which);

			switch(offset)
			{
				case 0:
					if (!chip->portA_dir && (chip->groupA_mode != 0))
					{
						chip->obf_a = 1;
						ppi8255_write_port(chip, 2, which);
					}
					break;

				case 1:
					if (!chip->portB_dir && (chip->groupB_mode != 0))
					{
						chip->obf_b = 1;
						ppi8255_write_port(chip, 2, which);
					}
					break;
			}
		  	break;

		case 3: /* Control word */
			if (data & 0x80)
			{
				set_mode(which, data & 0x7f, 1);
			}
			else
			{
	  			/* bit set/reset */
	  			INT32 bit;

	  			bit = (data >> 1) & 0x07;

	  			if (data & 1)
					chip->latch[2] |= (1<<bit);		/* set bit */
	  			else
					chip->latch[2] &= ~(1<<bit);	/* reset bit */

				ppi8255_write_port(chip, 2, which);
			}
			break;
	}
}

void ppi8255_init(INT32 num)
{
	DebugDev_8255PPIInitted = 1;
	
	for (INT32 i = 0; i < num; i++) {
		ppi8255 *chip = &chips[i];
	
		memset(chip, 0, sizeof(*chip));
	
		set_mode(i, 0x1b, 0);
	}
	
	nNumChips = num;
}

void ppi8255_exit()
{
#if defined FBA_DEBUG
	if (!DebugDev_8255PPIInitted) bprintf(PRINT_ERROR, _T("ppi8255_exit called without init\n"));
#endif

	for (INT32 i = 0; i < MAX_PPIS; i++) {
		ppi8255 *chip = &chips[i];
	
		memset(chip, 0, sizeof(*chip));
	}
	
 	PPI0PortReadA = NULL;
	PPI0PortReadB = NULL;
	PPI0PortReadC = NULL;
	PPI0PortWriteA = NULL;
	PPI0PortWriteB = NULL;
	PPI0PortWriteC = NULL;
	PPI1PortReadA = NULL;
	PPI1PortReadB = NULL;
	PPI1PortReadC = NULL;
	PPI1PortWriteA = NULL;
	PPI1PortWriteB = NULL;
	PPI1PortWriteC = NULL;
	PPI2PortReadA = NULL;
	PPI2PortReadB = NULL;
	PPI2PortReadC = NULL;
	PPI2PortWriteA = NULL;
	PPI2PortWriteB = NULL;
	PPI2PortWriteC = NULL;
	
	DebugDev_8255PPIInitted = 0;
	nNumChips = 0;
}

void ppi8255_scan()
{
#if defined FBA_DEBUG
	if (!DebugDev_8255PPIInitted) bprintf(PRINT_ERROR, _T("ppi8255_scan called without init\n"));
#endif

	SCAN_VAR(chips);
}

void ppi8255_set_portC( INT32 which, UINT8 data )
{
#if defined FBA_DEBUG
	if (!DebugDev_8255PPIInitted) bprintf(PRINT_ERROR, _T("ppi8255_set_portC called without init\n"));
	if (which > nNumChips) bprintf(PRINT_ERROR, _T("ppi8255_set_portC called with invalid chip %x\n"), which);
#endif

	ppi8255_input(&chips[which], 2, data, which);
}

#undef MAX_PPIS
