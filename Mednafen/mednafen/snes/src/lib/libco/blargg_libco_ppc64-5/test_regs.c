/* Verifies that correct registers are saved and restored by co_switch(),
that it doesn't unnecessarily preserve volatile registers, and that it
never touches system registers. MUST be compiled with --omit-frame-pointer */

#include "libco/libco.h"
#include "ppc_regs.h"

#include <assert.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include "testing.h"

enum {
	spr_cr		= 1 << 0,
	spr_xer		= 1 << 8,
	spr_ctr		= 1 << 9,
	spr_fpscr	= 1 << 10,
	spr_vrsave	= 1 << 11,
};

typedef struct {
	unsigned spr;
	unsigned gpr;
	unsigned fpr;
	unsigned vr;
} regmask_t;

enum { fpscr_mask = ~0x60000800 };

/* Single register and range of registers (inclusive) */
#define REG( b )				(1 << (b))
#define RANGE( start, end )		((~0 << (start)) & ~(~0 << (end) << 1))

/* Error if modified. FPRs and VRs included here since they shouldn't be
modified or needed during switching. */
static const regmask_t system_regs = {
	spr_fpscr | (spr_vrsave*!LIBCO_PPC_ALTIVEC),
	REG(2) | (REG(13)*LIBCO_PPC32),
	RANGE(0,LIBCO_PPC_FP ? 12 : 31),
	RANGE(0,LIBCO_PPC_ALTIVEC ? 19 : 31)
};

/* Error if not saved and restored */
static const regmask_t switched_regs = {
	0x7E | (spr_vrsave*LIBCO_PPC_ALTIVEC),
	REG(1) | RANGE(14-LIBCO_PPC32,31),
	RANGE(14,31)*LIBCO_PPC_FP,
	RANGE(20,31)*LIBCO_PPC_ALTIVEC
};

/* Error if saved and restored; fine if modified */
static const regmask_t volatile_regs = {
	spr_xer | spr_ctr,
	RANGE(3,13-LIBCO_PPC32) | REG(0),
	0,
	0
};

#undef RANGE
#undef REG

/* Everything is broken into a separate function to avoid the compiler loading
addresses into registers between operations, since we clear all registers. */

static int zero_enabled;

static ppc_regs_t regs_saved;
static ppc_regs_t regs_in;
static ppc_regs_t regs_zero;
static ppc_regs_t regs_out;

static void save_regs_saved()
{
	save_ppc_regs( &regs_saved );
	
	/* We must not trash these when restoring registers */
	regs_in.gpr [1] = regs_saved.gpr [1];
	regs_in.gpr [2] = regs_saved.gpr [2];
	
	regs_out = regs_in;
}

static void load_regs_saved()		{ load_ppc_regs( &regs_saved ); }
static void load_regs_in()			{ load_ppc_regs( &regs_in ); }
static void load_regs_zero()		{ load_ppc_regs( &regs_zero ); }
static void save_regs_out()			{ save_ppc_regs( &regs_out ); }

static cothread_t main_thread;
static cothread_t test_thread;
static cothread_t zero_thread;

static void switch_main_thread()	{ co_switch( main_thread ); }
static void switch_test_thread()	{ co_switch( test_thread ); }
static void switch_zero_thread()	{ if ( zero_enabled ) co_switch( zero_thread ); }

static void test_thread_func()
{
	for ( ;; )
	{
		/* Save current registers, load random registers, switch to another
		thread which then loads zero into registers and switches back to us,
		save our registers to see whether they match, then restore originals. */
		save_regs_saved();
		load_regs_in();
		switch_zero_thread();
		save_regs_out();
		load_regs_saved();
		switch_main_thread();
	}
}

static void zero_thread_func()
{
	for ( ;; )
	{
		load_regs_zero();
		switch_test_thread();
	}
}

/* Determines which registers are modified by co_switch(). If enable_zero_fill
is true, clears registers between two co_switch() calls. ORs result to
*out. */
static void find_modified( regmask_t* out, int enable_zero_fill )
{
	int i;
	
	zero_enabled = enable_zero_fill;
	
	for ( i = 0; i < sizeof regs_in; i++ )
		((unsigned char*) &regs_in) [i] = rand() >> 4;
	
	switch_test_thread();
	
	regs_out.fpscr = (regs_out.fpscr & fpscr_mask) | (regs_in.fpscr & ~fpscr_mask);
	
	#define SPR( name ) \
		if ( regs_out.name != regs_in.name )\
			out->spr |= spr_##name;
	
	SPR( xer );
	SPR( ctr );
	SPR( fpscr );
	SPR( vrsave );
	
	#undef SPR
	
	for ( i = 0; i < 8; i++ )
		if ( (regs_out.cr ^ regs_in.cr) >> (i*4) & 0x0F )
			out->spr |= spr_cr << i;
	
	#define REG( name ) \
	{\
		for ( i = 0; i < 32; i++ )\
			if ( memcmp( &regs_out.name [i], &regs_in.name [i], sizeof regs_in.name [i] ) )\
				out->name |= 1 << i;\
	}
	
	REG( gpr );
	REG( fpr );
	REG( vr );
	
	#undef REG
}

static void print_diffs( unsigned diff, const char name [] )
{
	if ( diff )
	{
		int i;
		
		printf( "%s: ", name );
		
		for ( i = 0; i < 32; i++ )
			if ( diff >> i & 1 )
				printf( "%d ", i );
		
		printf( "\n" );
	}
}

static unsigned check_regs( regmask_t regs, regmask_t mask, regmask_t cmp )
{
	unsigned any_diffs = 0;
	unsigned diff;
	
	diff = (regs.spr & mask.spr) ^ cmp.spr;
	any_diffs |= diff;
	if ( diff )
	{
		int i;
		for ( i = 0; i < 8; i++ )
			if ( diff >> i & 1 )
				printf( "CR%d ", 7 - i );
		
		#define SPR( name ) \
			if ( diff & spr_##name )\
				printf( #name " " );
		
		SPR( xer );
		SPR( ctr );
		SPR( fpscr );
		SPR( vrsave );
	
		#undef SPR
		
		printf( "\n" );
	}
	
	diff = (regs.gpr & mask.gpr) ^ cmp.gpr;
	any_diffs |= diff;
	print_diffs( diff, "GPR" );
	
	diff = (regs.fpr & mask.fpr) ^ cmp.fpr;
	any_diffs |= diff;
	print_diffs( diff, "FPR" );
	
	diff = (regs.vr & mask.vr) ^ cmp.vr;
	any_diffs |= diff;
	print_diffs( diff, "VR " );
	
	return any_diffs;
}

static void test( void )
{
	/* Run a bunch of trials and accumulate which registers are modified by co_switch(),
	and which aren't swapped by it. */
	regmask_t modified   = { 0 };
	regmask_t unswitched = { 0 };
	int n;
	
	for ( n = 2000; n--; )
	{
		find_modified( &modified,   0 );
		find_modified( &unswitched, 1 );
	}
	
	{
		regmask_t zero = { 0 };
		unsigned error = 0;
		
		printf( "Checking for modified system registers\n" );
		error |= check_regs( modified, system_regs, zero );
	
		printf( "Checking for unpreserved non-volatiles\n" );
		error |= check_regs( unswitched, switched_regs, zero );
	
		printf( "Checking for unnecessarily preserved volatiles\n" );
		error |= check_regs( unswitched, volatile_regs, volatile_regs );
		
		if ( error )
		{
			printf( "Failed\n" );
			exit( EXIT_FAILURE );
		}
	}
}

int main( void )
{
	printf( "Register test\n" );
	print_libco_opts();
	
	main_thread = co_active();
	test_thread = co_create( 32 * 1024, test_thread_func );
	zero_thread = co_create( 32 * 1024, zero_thread_func );
	
	test();
	
	printf( "Passed\n\n" );
	return 0;
}
