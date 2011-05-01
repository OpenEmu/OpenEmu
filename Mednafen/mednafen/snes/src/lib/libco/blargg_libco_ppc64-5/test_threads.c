/* Stress tests cothreads

Randomly creates, deletes, switches threads, and randomly
shuffles random data between locals and shared globals,
and randomly calls deeper and returns shallower in call
chain of each thread. After running for a while, prints
XOR of all the global data, and verifies that this matches
correct value.

STRESS_EXCEPTIONS uses C++ exceptions rather than longjmp.
*/

#include "libco/libco.h"

#include <assert.h>
#include <stdlib.h>
#include <string.h>
#include <setjmp.h>
#include <stdio.h>
#include "testing.h"

#if BLARGG_DEV
	int const iter = 300;
	unsigned const final_data = 0x4E914B5A;
#else
	int const iter = 100000;
	unsigned const final_data = 0x1FED4BE2;
#endif

int const stack_size = 256 * 1024;

enum { max_threads = 16 };
static cothread_t threads [max_threads];

static unsigned shared [16];

static unsigned rnd()
{
	static unsigned n = 1;
	return n = (n >> 1) ^ (0xEDB88320 & -(n & 1));
}

static void run( jmp_buf, int depth );

static void entry( void )
{
	run( NULL, 0 );
}

static int max_depth = 0;

static void run( jmp_buf jb, int depth )
{
	unsigned local [16];
	memcpy( local, shared, sizeof local );
	
	if ( depth > max_depth )
		max_depth = depth;
	
	while ( 1 )
	{
		co_switch( threads [0] );
		
		switch ( rnd() & 7 )
		{
		case 0:
			if ( depth > 0 )
			{
				if ( rnd() & 1 )
					#if STRESS_EXCEPTIONS
						throw 0;
					#else
						longjmp( jb, 1 );
					#endif
				goto ret;
			}
			break;
		
		case 1:
			if ( depth < 50 )
			{
				#if STRESS_EXCEPTIONS
				{
					try {
						run( jb, depth + 1 );
					}
					catch ( ... )
					{ }
				}
				#else
				{
					jmp_buf jb2;
					if ( !setjmp( jb2 ) )
						run( jb2, depth + 1 );
				}
				#endif
			}
			break;
		
		case 2: {
			int i;
			for ( i = 0; i < max_threads; i++ )
			{
				if ( !threads [i] )
				{
					threads [i] = co_create( stack_size, entry );
					assert( threads [i] );
					break;
				}
			}
			break;
		}
		
		case 3:
		{
			int i = rnd() & (max_threads - 1);
			if ( i > 0 && threads [i] && threads [i] != co_active() )
			{
				co_delete( threads [i] );
				threads [i] = 0;
			}
			break;
		}
		
		case 4:
		case 5:
		case 6:
		case 7: {
			int n;
			for ( n = 10; n--; )
			{
				unsigned r;
				r = rnd(); local [r & 15]  += rnd();
				r = rnd(); shared [r & 15] += rnd();
				r = rnd(); local [r & 15]  ^= shared [rnd() & 15];
				r = rnd(); shared [r & 15] ^= local [rnd() & 15];
			}
			break;
		}
		
		}
		
		{
			int i = rnd() & (max_threads - 1);
			if ( threads [i] && threads [i] != co_active() )
				co_switch( threads [i] );
		}
	}
ret:;
}

int main( void )
{
	int i, n;
	
	printf( "Thread test\n" );
	print_libco_opts();
	
	threads [0] = co_active();
	threads [1] = co_create( stack_size, entry );
	assert( threads [1] );
	for ( n = 0; n < iter; n++ )
	{
		/*
		if ( !(n & (n - 1)) )
			printf( "%d\n", n );*/
		
		for ( i = 1; i < max_threads; i++ )
			if ( threads [i] )
				co_switch( threads [i] );
	}
	
	{
		unsigned all = 0;
		for ( i = 0; i < 16; i++ )
			all ^= shared [i];
		
		if ( all != final_data )
		{
			printf( "0x%08X\n", all );
			printf( "Incorrect CRC\n" );
			return EXIT_FAILURE;
		}
	}
	
	printf( "Passed\n\n" );
	return 0;
}
