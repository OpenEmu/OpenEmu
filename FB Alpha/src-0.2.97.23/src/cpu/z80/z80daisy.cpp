/***************************************************************************

    z80daisy.c

    Z80/180 daisy chaining support functions.

***************************************************************************/

#include "z80daisy.h"

#define CLEAR_LINE	0
#define ASSERT_LINE	1


void z80daisy_reset(const struct z80_irq_daisy_chain *daisy)
{
	/* loop over all devices and call their reset function */
	for ( ; daisy->param != -1; daisy++)
		if (daisy->reset)
			(*daisy->reset)(daisy->param);
}


int z80daisy_update_irq_state(const struct z80_irq_daisy_chain *daisy)
{
	/* loop over all devices; dev[0] is highest priority */
	for ( ; daisy->param != -1; daisy++)
	{
		int state = (*daisy->irq_state)(daisy->param);

		/* if this device is asserting the INT line, that's the one we want */
		if (state & Z80_DAISY_INT)
			return ASSERT_LINE;

		/* if this device is asserting the IEO line, it blocks everyone else */
		if (state & Z80_DAISY_IEO)
			return CLEAR_LINE;
	}

	return CLEAR_LINE;
}


int z80daisy_call_ack_device(const struct z80_irq_daisy_chain *daisy)
{
	/* loop over all devices; dev[0] is the highest priority */
	for ( ; daisy->param != -1; daisy++)
	{
		int state = (*daisy->irq_state)(daisy->param);

		/* if this device is asserting the INT line, that's the one we want */
		if (state & Z80_DAISY_INT)
			return (*daisy->irq_ack)(daisy->param);
	}

//	logerror("z80daisy_call_ack_device: failed to find an device to ack!\n");
	return 0;
}


void z80daisy_call_reti_device(const struct z80_irq_daisy_chain *daisy)
{
	/* loop over all devices; dev[0] is the highest priority */
	for ( ; daisy->param != -1; daisy++)
	{
		int state = (*daisy->irq_state)(daisy->param);

		/* if this device is asserting the IEO line, that's the one we want */
		if (state & Z80_DAISY_IEO)
		{
			(*daisy->irq_reti)(daisy->param);
			return;
		}
	}

//	logerror("z80daisy_call_reti_device: failed to find an device to reti!\n");
}
