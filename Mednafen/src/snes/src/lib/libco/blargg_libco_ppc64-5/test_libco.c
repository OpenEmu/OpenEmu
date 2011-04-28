#include "libco/libco.h"

#include <assert.h>
#include <stdlib.h>
#include <stdio.h>
#include "testing.h"

/* Verifies co_active() initial behavior */
static void test_initial( void )
{
	int n;
	
	if ( !co_active() )
		assert( 0 );
	
	for ( n = 1000; n--; )
		co_switch( co_active() );
}

static void thread_func( void )
{ }

/* Creates and deletes threads many times to ensure nothing is leaked */
static void test_leaks( void )
{
	enum { thread_count = 10 };
	cothread_t threads [thread_count] = { 0 };
	int i = 0;
	int n;
	
	for ( n = 10000; n--; )
	{
		if ( threads [i] )
			co_delete( threads [i] );
		
		threads [i] = co_create( 256 * 1024, thread_func );
		assert( threads [i] );
		
		i = (i + 1) % thread_count;
	}
	
	for ( i = 0; i < thread_count; i++ )
		if ( threads [i] )
			co_delete( threads [i] );
}

static cothread_t threads [10];
static int next;

static void next_thread( void )
{
	co_switch( threads [next++] );
}

static void preserved_func( void )
{
	int seed = next;
	
	#define INIT(type,n) \
		register type type##n = rand();
	
	#define CHECK(type,n) \
		assert( type##n == rand() );
	
	#define REGS_(type,op,base) \
		op(type,base##0)\
		op(type,base##1)\
		op(type,base##2)\
		op(type,base##3)\
		op(type,base##4)\
		op(type,base##5)\
		op(type,base##6)\
		op(type,base##7)\
		op(type,base##8)\
		op(type,base##9)
	
	/* Invokes op(type,n), with n going from 00 through 49 */
	#define REGS(type,op) \
		REGS_(type,op,0)\
		REGS_(type,op,1)\
		REGS_(type,op,2)\
		REGS_(type,op,3)\
		REGS_(type,op,4)
	
	srand(seed);
	{
		/* Fill int and FP registers with random values */
		REGS(size_t,INIT)
		
		#if LIBCO_PPC_FP
			REGS(double,INIT)
		#endif
		
		/* Run second copy of this func that puts different random values
		in registers, and then switches back to us */
		next_thread();
		
		/* Verify that registers match random values */
		srand(seed);
		
		REGS(size_t,CHECK)
		
		#if LIBCO_PPC_FP
			REGS(double,CHECK)
		#endif
	}
	
	next_thread();
}

/* Verifies that registers are saved and restored by co_switch() */
static void test_preserved( void )
{
	threads [0] = co_create( 64 * 1024, preserved_func );
	threads [1] = co_create( 64 * 1024, preserved_func );
	threads [2] = threads [0];
	threads [3] = co_active();
	
	next = 0;
	next_thread();
}

int main( void )
{
	printf( "Basic test\n" );
	print_libco_opts();
	
	test_initial();
	test_leaks();
	test_preserved();
	
	printf( "Passed\n\n" );
	return 0;
}
