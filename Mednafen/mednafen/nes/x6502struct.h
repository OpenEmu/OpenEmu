#ifndef _X6502STRUCTH

typedef struct __X6502
{
        int32 tcount;           /* Temporary cycle counter */
        uint32 PC;
        uint8 A,X,Y,S,P,mooPI;
        uint8 jammed;

	int32 count;
        uint32 IRQlow;          /* Simulated IRQ pin held low(or is it high?).
                                   And other junk hooked on for speed reasons.*/
        uint8 DB;               /* Data bus "cache" for reads from certain areas */

        int preexec;            /* Pre-exec'ing for debug breakpoints. */
	int cpoint;		

	#ifdef WANT_DEBUGGER
        void (*CPUHook)(uint32);
        uint8 (*ReadHook)(struct __X6502 *, unsigned int);
        void (*WriteHook)(struct __X6502 *, unsigned int, uint8);
	#endif
} X6502;
#define _X6502STRUCTH
#endif
