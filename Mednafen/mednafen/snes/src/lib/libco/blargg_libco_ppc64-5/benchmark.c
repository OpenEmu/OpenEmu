/* Measures absolute and relative performance of co_switch()
versus function call. */

#include "libco/libco.h"

#include <assert.h>
#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include "testing.h"

/* Keep calling this until it returns non-zero value. Result
is number of calls made per second. */
double calls_per_sec( void );

/* Something to do in addition to func call/co_switch */
static volatile int counter;
#define DO_SOMETHING() counter++


void benchmark_func( void );

static double time_func( void )
{
	int const iter = 100000;
	double rate;
	while ( !(rate = calls_per_sec()) )
	{
		int n;
		for ( n = iter / 4; n--; )
		{
			benchmark_func();
			benchmark_func();
			benchmark_func();
			benchmark_func();
		}
	}
	return rate * iter;
}


/* co_switch timing */
static cothread_t main_thread;

static void thread_func( void )
{
	while ( 1 )
	{
		co_switch( main_thread ); DO_SOMETHING();
		co_switch( main_thread ); DO_SOMETHING();
		co_switch( main_thread ); DO_SOMETHING();
		co_switch( main_thread ); DO_SOMETHING();
	}
}

static double time_co_switch( void )
{
	int const iter = 1000;
	double rate;
	cothread_t thread;
	
	main_thread = co_active();
	thread = co_create( 16 * 1024, thread_func );
	assert( thread && main_thread );
	
	while ( !(rate = calls_per_sec()) )
	{
		int n;
		for ( n = iter / 4; n--; )
		{
			co_switch( thread );
			co_switch( thread );
			co_switch( thread );
			co_switch( thread );
		}
	}
	return rate * iter;
}


/* Time both and compare */
int main( void )
{
	double func_rate = time_func();
	double co_rate = time_co_switch();
	
	print_libco_opts();
	
	printf( "%6.2f M function calls per sec\n", func_rate / 1000000 );
	printf( "%6.2f M co_switch()x2  per sec\n", co_rate   / 1000000 );
	
	printf( "Function call is %.1fX faster than two co_switch() calls.\n\n", func_rate / co_rate );
	
	return 0;
}


/* Utility */
double calls_per_sec( void )
{
	#define DURATION CLOCKS_PER_SEC
	
	static clock_t s_end;
	static int s_iter;
	clock_t const present = clock();
	int const iter = s_iter++;
	if ( iter <= 1 ) /* throw away first iteration since it'll be slower due to setup */
	{
		if ( CLOCKS_PER_SEC < 10 )
		{
			fprintf( stderr, "Not enough clock() resolution\n" );
			exit( EXIT_FAILURE );
		}
		while ( clock() == present ) { }
		s_end = clock() + DURATION;
	}
	else if ( present >= s_end )
	{
		s_iter = 0;
		return (double) (iter - 1) * CLOCKS_PER_SEC / (present - s_end + DURATION);
	}
	return 0;
}
