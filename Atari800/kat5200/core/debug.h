/******************************************************************************
*
* FILENAME: debug.h
*
* DESCRIPTION:  This contains function declarations for debugger functions
*
* AUTHORS:  bberlin
*
* CHANGE LOG
*
*  VER     DATE     CHANGED BY   DESCRIPTION OF CHANGE
* ------  --------  ----------   ------------------------------------
* 0.5.0   02/11/07  bberlin      Creation, breakout from util
******************************************************************************/
#ifndef debugger_h
#define debugger_h

#include "boom6502.h"

/* Debug commands  */
#define PRINT 'p'
#define DEBUG_QUIT 'q'
#define TRACE 't'
#define STOPTRACE 'x'
#define BREAKPOINT 'b'
#define WATCH 'w'
#define DELETE_BK 'd'
#define STEP 's'
#define CONT 'c'
#define HELP 'h'

/* Debug arguments */
#define ARG_MODELINE "modeline"
#define ARG_SCANLINE "scanline"
#define ARG_VBLANK   "vblank"

/* Debug processor */
#define DEBUG_CPU "cpu"
#define DEBUG_ANTIC "antic"

#define DEBUG_MAX_BK 10

/* Debug constants */
typedef enum {
	BK_NONE,
	BK_SCANLINE,
	BK_MEM,
	BK_WATCH,
	BK_VBLANK
} e_debug_breakpoint;

struct debug {
	int flag;
	int trace_cpu_flag;
	char trace_cpu_file[257];
	char trace_antic_file[257];
	int trace_antic_flag;
	int single_step_flag;
	int breakpoint_flag;
	e_debug_breakpoint breakpoint[DEBUG_MAX_BK];
	int breakpoint_place[DEBUG_MAX_BK];
};

void debug_init ( void );
void print_addr_val ( int access_type, WORD addr, BYTE val );
int debug_crash_to_gui ( void );
void debug_get_input (void);
void debug_print_value ( char *arg );
void debug_print_cpu_op ( int point_of_debug );
void debug_search_for_breakpoint (e_debug_breakpoint break_type, int value );
void debug_break_for_watch (int addr, int new_value, int old_value );
int debug_is_page_in_debug (int page);
void debug_print_string_to_trace (char *string);
void debug_open_default_trace (void);
void debug_close_default_trace (void);

#endif
