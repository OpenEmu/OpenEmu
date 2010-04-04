
#ifndef CPUINTRF_H
#define CPUINTRF_H

#include "osd_cpu.h"

/* Interrupt line constants */
enum
{
  /* line states */
  CLEAR_LINE = 0,     /* clear (a fired, held or pulsed) line */
  ASSERT_LINE,        /* assert an interrupt immediately */
  HOLD_LINE,          /* hold interrupt line until acknowledged */
  PULSE_LINE,         /* pulse interrupt line for one instruction */

  /* internal flags (not for use by drivers!) */
  INTERNAL_CLEAR_LINE = 100 + CLEAR_LINE,
  INTERNAL_ASSERT_LINE = 100 + ASSERT_LINE,

  /* input lines */
  MAX_INPUT_LINES = 32+3,
  INPUT_LINE_IRQ0 = 0,
  INPUT_LINE_IRQ1 = 1,
  INPUT_LINE_IRQ2 = 2,
  INPUT_LINE_IRQ3 = 3,
  INPUT_LINE_IRQ4 = 4,
  INPUT_LINE_IRQ5 = 5,
  INPUT_LINE_IRQ6 = 6,
  INPUT_LINE_IRQ7 = 7,
  INPUT_LINE_IRQ8 = 8,
  INPUT_LINE_IRQ9 = 9,
  INPUT_LINE_NMI = MAX_INPUT_LINES - 3,

  /* special input lines that are implemented in the core */
  INPUT_LINE_RESET = MAX_INPUT_LINES - 2,
  INPUT_LINE_HALT = MAX_INPUT_LINES - 1,

  /* output lines */
  MAX_OUTPUT_LINES = 32
};


/* daisy-chain link */
typedef struct {
  void (*reset)(int);             /* reset callback     */
  int  (*interrupt_entry)(int);   /* entry callback     */
  void (*interrupt_reti)(int);    /* reti callback      */
  int irq_param;                  /* callback paramater */
}  Z80_DaisyChain;

#define Z80_MAXDAISY  4    /* maximum of daisy chan device */

#define Z80_INT_REQ     0x01    /* interrupt request mask       */
#define Z80_INT_IEO     0x02    /* interrupt disable mask(IEO)  */

#define Z80_VECTOR(device,state) (((device)<<8)|(state))


#endif  /* CPUINTRF_H */
