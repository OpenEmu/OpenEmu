
#ifndef CPUINTRF_H
#define CPUINTRF_H

#include "osd_cpu.h"

#define CPU_16BIT_PORT          0x4000
#define CPU_FLAGS_MASK          0xff00
#define CLEAR_LINE              0
#define ASSERT_LINE             1
#define REG_PREVIOUSPC          -1
#define REG_SP_CONTENTS         -2

/* daisy-chain link */
typedef struct {
	void (*reset)(int);             /* reset callback     */
	int  (*interrupt_entry)(int);   /* entry callback     */
	void (*interrupt_reti)(int);    /* reti callback      */
	int irq_param;                  /* callback paramater */
}	Z80_DaisyChain;

#define Z80_MAXDAISY	4		/* maximum of daisy chan device */

#define Z80_INT_REQ     0x01    /* interrupt request mask       */
#define Z80_INT_IEO     0x02    /* interrupt disable mask(IEO)  */

#define Z80_VECTOR(device,state) (((device)<<8)|(state))


#endif	/* CPUINTRF_H */
