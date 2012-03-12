/***************************************************************************

    z80daisy.h

    Z80/180 daisy chaining support functions.

***************************************************************************/


#ifndef Z80DAISY_H
#define Z80DAISY_H


/* daisy-chain link */
struct z80_irq_daisy_chain
{
	void (*reset)(int); 			/* reset callback */
	int (*irq_state)(int);			/* get interrupt state */
	int (*irq_ack)(int);			/* interrupt acknowledge callback */
	void (*irq_reti)(int);			/* reti callback */
	int param;						/* callback parameter (-1 ends list) */
};


/* these constants are returned from the irq_state function */
#define Z80_DAISY_INT 	0x01		/* interrupt request mask */
#define Z80_DAISY_IEO 	0x02		/* interrupt disable mask (IEO) */


/* prototypes */
void z80daisy_reset(const struct z80_irq_daisy_chain *daisy);
int z80daisy_update_irq_state(const struct z80_irq_daisy_chain *chain);
int z80daisy_call_ack_device(const struct z80_irq_daisy_chain *chain);
void z80daisy_call_reti_device(const struct z80_irq_daisy_chain *chain);

#endif
