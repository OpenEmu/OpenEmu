#ifndef MONITOR_H_
#define MONITOR_H_

#include "config.h"
#include <stdio.h>

#include "atari.h"

int MONITOR_Run(void);

#ifdef MONITOR_TRACE
extern FILE *MONITOR_trace_file;
#endif

#ifdef MONITOR_BREAK
extern UWORD MONITOR_break_addr;
extern UBYTE MONITOR_break_step;
extern UBYTE MONITOR_break_ret;
extern UBYTE MONITOR_break_brk;
extern int MONITOR_ret_nesting;
#endif

extern const UBYTE MONITOR_optype6502[256];

void MONITOR_Exit(void);
void MONITOR_ShowState(FILE *fp, UWORD pc, UBYTE a, UBYTE x, UBYTE y, UBYTE s,
                char n, char v, char z, char c);

#ifdef MONITOR_BREAKPOINTS

/* Breakpoint conditions */

#define MONITOR_BREAKPOINT_OR          1
#define MONITOR_BREAKPOINT_FLAG_CLEAR  2
#define MONITOR_BREAKPOINT_FLAG_SET    3

/* these three may be ORed together and must be ORed with MONITOR_BREAKPOINT_PC .. MONITOR_BREAKPOINT_WRITE */
#define MONITOR_BREAKPOINT_LESS        1
#define MONITOR_BREAKPOINT_EQUAL       2
#define MONITOR_BREAKPOINT_GREATER     4

#define MONITOR_BREAKPOINT_PC          8
#define MONITOR_BREAKPOINT_A           16
#define MONITOR_BREAKPOINT_X           32
#define MONITOR_BREAKPOINT_Y           40
#define MONITOR_BREAKPOINT_S           48
#define MONITOR_BREAKPOINT_READ        64
#define MONITOR_BREAKPOINT_WRITE       128
#define MONITOR_BREAKPOINT_ACCESS      (MONITOR_BREAKPOINT_READ | MONITOR_BREAKPOINT_WRITE)

typedef struct {
	UBYTE enabled;
	UBYTE condition;
	UWORD value;
} MONITOR_breakpoint_cond;

#define MONITOR_BREAKPOINT_TABLE_MAX  20
extern MONITOR_breakpoint_cond MONITOR_breakpoint_table[MONITOR_BREAKPOINT_TABLE_MAX];
extern int MONITOR_breakpoint_table_size;
extern int MONITOR_breakpoints_enabled;

#endif /* MONITOR_BREAKPOINTS */

#endif /* MONITOR_H_ */
