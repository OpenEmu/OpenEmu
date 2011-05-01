
/*============================================================================

This C source file is part of the SoftFloat IEC/IEEE Floating-point Arithmetic
Package, Release 2b.

Written by John R. Hauser.  This work was made possible in part by the
International Computer Science Institute, located at Suite 600, 1947 Center
Street, Berkeley, California 94704.  Funding was partially provided by the
National Science Foundation under grant MIP-9311980.  The original version
of this code was written as part of a project to build a fixed-point vector
processor in collaboration with the University of California at Berkeley,
overseen by Profs. Nelson Morgan and John Wawrzynek.  More information
is available through the Web page `http://www.cs.berkeley.edu/~jhauser/
arithmetic/SoftFloat.html'.

THIS SOFTWARE IS DISTRIBUTED AS IS, FOR FREE.  Although reasonable effort has
been made to avoid it, THIS SOFTWARE MAY CONTAIN FAULTS THAT WILL AT TIMES
RESULT IN INCORRECT BEHAVIOR.  USE OF THIS SOFTWARE IS RESTRICTED TO PERSONS
AND ORGANIZATIONS WHO CAN AND WILL TAKE FULL RESPONSIBILITY FOR ALL LOSSES,
COSTS, OR OTHER PROBLEMS THEY INCUR DUE TO THE SOFTWARE, AND WHO FURTHERMORE
EFFECTIVELY INDEMNIFY JOHN HAUSER AND THE INTERNATIONAL COMPUTER SCIENCE
INSTITUTE (possibly via similar legal warning) AGAINST ALL LOSSES, COSTS, OR
OTHER PROBLEMS INCURRED BY THEIR CUSTOMERS AND CLIENTS DUE TO THE SOFTWARE.

Derivative works are acceptable, even for commercial purposes, so long as
(1) the source code for the derivative work includes prominent notice that
the work is derivative, and (2) the source code includes prominent notice with
these four paragraphs for those parts of this code that are retained.

=============================================================================*/

#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <stdio.h>
#include <time.h>
#include "milieu.h"
#include "softfloat.h"

enum {
    minIterations = 1000
};

void fail( const char *message, ... )
{
    va_list varArgs;

    fputs( "timesoftfloat: ", stderr );
    va_start( varArgs, message );
    vfprintf( stderr, message, varArgs );
    va_end( varArgs );
    fputs( ".\n", stderr );
    exit( EXIT_FAILURE );

}

static char *functionName, *roundingModeName, *tininessModeName;

static void reportTime( int32 count, long clocks )
{

    printf(
        "%8.1f kops/s: %s",
        ( count / ( ( (float) clocks ) / CLOCKS_PER_SEC ) ) / 1000,
        functionName
    );
    if ( roundingModeName ) {
        fputs( ", rounding ", stdout );
        fputs( roundingModeName, stdout );
        if ( tininessModeName ) {
            fputs( ", tininess ", stdout );
            fputs( tininessModeName, stdout );
            fputs( " rounding", stdout );
        }
    }
    fputc( '\n', stdout );

}

enum {
    numInputs_int32 = 32
};

static const int32 inputs_int32[ numInputs_int32 ] = {
    0xFFFFBB79, 0x405CF80F, 0x00000000, 0xFFFFFD04,
    0xFFF20002, 0x0C8EF795, 0xF00011FF, 0x000006CA,
    0x00009BFE, 0xFF4862E3, 0x9FFFEFFE, 0xFFFFFFB7,
    0x0BFF7FFF, 0x0000F37A, 0x0011DFFE, 0x00000006,
    0xFFF02006, 0xFFFFF7D1, 0x10200003, 0xDE8DF765,
    0x00003E02, 0x000019E8, 0x0008FFFE, 0xFFFFFB5C,
    0xFFDF7FFE, 0x07C42FBF, 0x0FFFE3FF, 0x040B9F13,
    0xBFFFFFF8, 0x0001BF56, 0x000017F6, 0x000A908A
};

static void time_a_int32_z_float32( float32 function( int32 ) )
{
    clock_t startClock, endClock;
    int32 count, i;
    int8 inputNum;

    count = 0;
    inputNum = 0;
    startClock = clock();
    do {
        for ( i = minIterations; i; --i ) {
            function( inputs_int32[ inputNum ] );
            inputNum = ( inputNum + 1 ) & ( numInputs_int32 - 1 );
        }
        count += minIterations;
    } while ( clock() - startClock < CLOCKS_PER_SEC );
    inputNum = 0;
    startClock = clock();
    for ( i = count; i; --i ) {
        function( inputs_int32[ inputNum ] );
        inputNum = ( inputNum + 1 ) & ( numInputs_int32 - 1 );
    }
    endClock = clock();
    reportTime( count, endClock - startClock );

}

static void time_a_int32_z_float64( float64 function( int32 ) )
{
    clock_t startClock, endClock;
    int32 count, i;
    int8 inputNum;

    count = 0;
    inputNum = 0;
    startClock = clock();
    do {
        for ( i = minIterations; i; --i ) {
            function( inputs_int32[ inputNum ] );
            inputNum = ( inputNum + 1 ) & ( numInputs_int32 - 1 );
        }
        count += minIterations;
    } while ( clock() - startClock < CLOCKS_PER_SEC );
    inputNum = 0;
    startClock = clock();
    for ( i = count; i; --i ) {
        function( inputs_int32[ inputNum ] );
        inputNum = ( inputNum + 1 ) & ( numInputs_int32 - 1 );
    }
    endClock = clock();
    reportTime( count, endClock - startClock );

}

enum {
    numInputs_float32 = 32
};

static const float32 inputs_float32[ numInputs_float32 ] = {
    0x4EFA0000, 0xC1D0B328, 0x80000000, 0x3E69A31E,
    0xAF803EFF, 0x3F800000, 0x17BF8000, 0xE74A301A,
    0x4E010003, 0x7EE3C75D, 0xBD803FE0, 0xBFFEFF00,
    0x7981F800, 0x431FFFFC, 0xC100C000, 0x3D87EFFF,
    0x4103FEFE, 0xBC000007, 0xBF01F7FF, 0x4E6C6B5C,
    0xC187FFFE, 0xC58B9F13, 0x4F88007F, 0xDF004007,
    0xB7FFD7FE, 0x7E8001FB, 0x46EFFBFF, 0x31C10000,
    0xDB428661, 0x33F89B1F, 0xA3BFEFFF, 0x537BFFBE
};

static void time_a_float32_z_int32( int32 function( float32 ) )
{
    clock_t startClock, endClock;
    int32 count, i;
    int8 inputNum;

    count = 0;
    inputNum = 0;
    startClock = clock();
    do {
        for ( i = minIterations; i; --i ) {
            function( inputs_float32[ inputNum ] );
            inputNum = ( inputNum + 1 ) & ( numInputs_float32 - 1 );
        }
        count += minIterations;
    } while ( clock() - startClock < CLOCKS_PER_SEC );
    inputNum = 0;
    startClock = clock();
    for ( i = count; i; --i ) {
        function( inputs_float32[ inputNum ] );
        inputNum = ( inputNum + 1 ) & ( numInputs_float32 - 1 );
    }
    endClock = clock();
    reportTime( count, endClock - startClock );

}

static void time_a_float32_z_float64( float64 function( float32 ) )
{
    clock_t startClock, endClock;
    int32 count, i;
    int8 inputNum;

    count = 0;
    inputNum = 0;
    startClock = clock();
    do {
        for ( i = minIterations; i; --i ) {
            function( inputs_float32[ inputNum ] );
            inputNum = ( inputNum + 1 ) & ( numInputs_float32 - 1 );
        }
        count += minIterations;
    } while ( clock() - startClock < CLOCKS_PER_SEC );
    inputNum = 0;
    startClock = clock();
    for ( i = count; i; --i ) {
        function( inputs_float32[ inputNum ] );
        inputNum = ( inputNum + 1 ) & ( numInputs_float32 - 1 );
    }
    endClock = clock();
    reportTime( count, endClock - startClock );

}

static void time_az_float32( float32 function( float32 ) )
{
    clock_t startClock, endClock;
    int32 count, i;
    int8 inputNum;

    count = 0;
    inputNum = 0;
    startClock = clock();
    do {
        for ( i = minIterations; i; --i ) {
            function( inputs_float32[ inputNum ] );
            inputNum = ( inputNum + 1 ) & ( numInputs_float32 - 1 );
        }
        count += minIterations;
    } while ( clock() - startClock < CLOCKS_PER_SEC );
    inputNum = 0;
    startClock = clock();
    for ( i = count; i; --i ) {
        function( inputs_float32[ inputNum ] );
        inputNum = ( inputNum + 1 ) & ( numInputs_float32 - 1 );
    }
    endClock = clock();
    reportTime( count, endClock - startClock );

}

static void time_ab_float32_z_flag( flag function( float32, float32 ) )
{
    clock_t startClock, endClock;
    int32 count, i;
    int8 inputNumA, inputNumB;

    count = 0;
    inputNumA = 0;
    inputNumB = 0;
    startClock = clock();
    do {
        for ( i = minIterations; i; --i ) {
            function(
                inputs_float32[ inputNumA ], inputs_float32[ inputNumB ] );
            inputNumA = ( inputNumA + 1 ) & ( numInputs_float32 - 1 );
            if ( inputNumA == 0 ) ++inputNumB;
            inputNumB = ( inputNumB + 1 ) & ( numInputs_float32 - 1 );
        }
        count += minIterations;
    } while ( clock() - startClock < CLOCKS_PER_SEC );
    inputNumA = 0;
    inputNumB = 0;
    startClock = clock();
    for ( i = count; i; --i ) {
            function(
                inputs_float32[ inputNumA ], inputs_float32[ inputNumB ] );
        inputNumA = ( inputNumA + 1 ) & ( numInputs_float32 - 1 );
        if ( inputNumA == 0 ) ++inputNumB;
        inputNumB = ( inputNumB + 1 ) & ( numInputs_float32 - 1 );
    }
    endClock = clock();
    reportTime( count, endClock - startClock );

}

static void time_abz_float32( float32 function( float32, float32 ) )
{
    clock_t startClock, endClock;
    int32 count, i;
    int8 inputNumA, inputNumB;

    count = 0;
    inputNumA = 0;
    inputNumB = 0;
    startClock = clock();
    do {
        for ( i = minIterations; i; --i ) {
            function(
                inputs_float32[ inputNumA ], inputs_float32[ inputNumB ] );
            inputNumA = ( inputNumA + 1 ) & ( numInputs_float32 - 1 );
            if ( inputNumA == 0 ) ++inputNumB;
            inputNumB = ( inputNumB + 1 ) & ( numInputs_float32 - 1 );
        }
        count += minIterations;
    } while ( clock() - startClock < CLOCKS_PER_SEC );
    inputNumA = 0;
    inputNumB = 0;
    startClock = clock();
    for ( i = count; i; --i ) {
            function(
                inputs_float32[ inputNumA ], inputs_float32[ inputNumB ] );
        inputNumA = ( inputNumA + 1 ) & ( numInputs_float32 - 1 );
        if ( inputNumA == 0 ) ++inputNumB;
        inputNumB = ( inputNumB + 1 ) & ( numInputs_float32 - 1 );
    }
    endClock = clock();
    reportTime( count, endClock - startClock );

}

static const float32 inputs_float32_pos[ numInputs_float32 ] = {
    0x4EFA0000, 0x41D0B328, 0x00000000, 0x3E69A31E,
    0x2F803EFF, 0x3F800000, 0x17BF8000, 0x674A301A,
    0x4E010003, 0x7EE3C75D, 0x3D803FE0, 0x3FFEFF00,
    0x7981F800, 0x431FFFFC, 0x4100C000, 0x3D87EFFF,
    0x4103FEFE, 0x3C000007, 0x3F01F7FF, 0x4E6C6B5C,
    0x4187FFFE, 0x458B9F13, 0x4F88007F, 0x5F004007,
    0x37FFD7FE, 0x7E8001FB, 0x46EFFBFF, 0x31C10000,
    0x5B428661, 0x33F89B1F, 0x23BFEFFF, 0x537BFFBE
};

static void time_az_float32_pos( float32 function( float32 ) )
{
    clock_t startClock, endClock;
    int32 count, i;
    int8 inputNum;

    count = 0;
    inputNum = 0;
    startClock = clock();
    do {
        for ( i = minIterations; i; --i ) {
            function( inputs_float32_pos[ inputNum ] );
            inputNum = ( inputNum + 1 ) & ( numInputs_float32 - 1 );
        }
        count += minIterations;
    } while ( clock() - startClock < CLOCKS_PER_SEC );
    inputNum = 0;
    startClock = clock();
    for ( i = count; i; --i ) {
        function( inputs_float32_pos[ inputNum ] );
        inputNum = ( inputNum + 1 ) & ( numInputs_float32 - 1 );
    }
    endClock = clock();
    reportTime( count, endClock - startClock );

}

enum {
    numInputs_float64 = 32
};

static const struct {
    bits32 high, low;
} inputs_float64[ numInputs_float64 ] = {
    { 0x422FFFC0, 0x08000000 },
    { 0xB7E00004, 0x80000000 },
    { 0xF3FD2546, 0x120B7935 },
    { 0x3FF00000, 0x00000000 },
    { 0xCE07F766, 0xF09588D6 },
    { 0x80000000, 0x00000000 },
    { 0x3FCE0004, 0x00000000 },
    { 0x8313B60F, 0x0032BED8 },
    { 0xC1EFFFFF, 0xC0002000 },
    { 0x3FB3C75D, 0x224F2B0F },
    { 0x7FD00000, 0x004000FF },
    { 0xA12FFF80, 0x00001FFF },
    { 0x3EE00000, 0x00FE0000 },
    { 0x00100000, 0x80000004 },
    { 0x41CFFFFE, 0x00000020 },
    { 0x40303FFF, 0xFFFFFFFD },
    { 0x3FD00000, 0x3FEFFFFF },
    { 0xBFD00000, 0x10000000 },
    { 0xB7FC6B5C, 0x16CA55CF },
    { 0x413EEB94, 0x0B9D1301 },
    { 0xC7E00200, 0x001FFFFF },
    { 0x47F00021, 0xFFFFFFFE },
    { 0xBFFFFFFF, 0xF80000FF },
    { 0xC07FFFFF, 0xE00FFFFF },
    { 0x001497A6, 0x3740C5E8 },
    { 0xC4BFFFE0, 0x001FFFFF },
    { 0x96FFDFFE, 0xFFFFFFFF },
    { 0x403FC000, 0x000001FE },
    { 0xFFD00000, 0x000001F6 },
    { 0x06404000, 0x02000000 },
    { 0x479CEE1E, 0x4F789FE0 },
    { 0xC237FFFF, 0xFFFFFDFE }
};

static void time_a_float64_z_int32( int32 function( float64 ) )
{
    clock_t startClock, endClock;
    int32 count, i;
    int8 inputNum;
    float64 a;

    count = 0;
    inputNum = 0;
    startClock = clock();
    do {
        for ( i = minIterations; i; --i ) {
            a.low = inputs_float64[ inputNum ].low;
            a.high = inputs_float64[ inputNum ].high;
            function( a );
            inputNum = ( inputNum + 1 ) & ( numInputs_float64 - 1 );
        }
        count += minIterations;
    } while ( clock() - startClock < CLOCKS_PER_SEC );
    inputNum = 0;
    startClock = clock();
    for ( i = count; i; --i ) {
        a.low = inputs_float64[ inputNum ].low;
        a.high = inputs_float64[ inputNum ].high;
        function( a );
        inputNum = ( inputNum + 1 ) & ( numInputs_float64 - 1 );
    }
    endClock = clock();
    reportTime( count, endClock - startClock );

}

static void time_a_float64_z_float32( float32 function( float64 ) )
{
    clock_t startClock, endClock;
    int32 count, i;
    int8 inputNum;
    float64 a;

    count = 0;
    inputNum = 0;
    startClock = clock();
    do {
        for ( i = minIterations; i; --i ) {
            a.low = inputs_float64[ inputNum ].low;
            a.high = inputs_float64[ inputNum ].high;
            function( a );
            inputNum = ( inputNum + 1 ) & ( numInputs_float64 - 1 );
        }
        count += minIterations;
    } while ( clock() - startClock < CLOCKS_PER_SEC );
    inputNum = 0;
    startClock = clock();
    for ( i = count; i; --i ) {
        a.low = inputs_float64[ inputNum ].low;
        a.high = inputs_float64[ inputNum ].high;
        function( a );
        inputNum = ( inputNum + 1 ) & ( numInputs_float64 - 1 );
    }
    endClock = clock();
    reportTime( count, endClock - startClock );

}

static void time_az_float64( float64 function( float64 ) )
{
    clock_t startClock, endClock;
    int32 count, i;
    int8 inputNum;
    float64 a;

    count = 0;
    inputNum = 0;
    startClock = clock();
    do {
        for ( i = minIterations; i; --i ) {
            a.low = inputs_float64[ inputNum ].low;
            a.high = inputs_float64[ inputNum ].high;
            function( a );
            inputNum = ( inputNum + 1 ) & ( numInputs_float64 - 1 );
        }
        count += minIterations;
    } while ( clock() - startClock < CLOCKS_PER_SEC );
    inputNum = 0;
    startClock = clock();
    for ( i = count; i; --i ) {
        a.low = inputs_float64[ inputNum ].low;
        a.high = inputs_float64[ inputNum ].high;
        function( a );
        inputNum = ( inputNum + 1 ) & ( numInputs_float64 - 1 );
    }
    endClock = clock();
    reportTime( count, endClock - startClock );

}

static void time_ab_float64_z_flag( flag function( float64, float64 ) )
{
    clock_t startClock, endClock;
    int32 count, i;
    int8 inputNumA, inputNumB;
    float64 a, b;

    count = 0;
    inputNumA = 0;
    inputNumB = 0;
    startClock = clock();
    do {
        for ( i = minIterations; i; --i ) {
            a.low = inputs_float64[ inputNumA ].low;
            a.high = inputs_float64[ inputNumA ].high;
            b.low = inputs_float64[ inputNumB ].low;
            b.high = inputs_float64[ inputNumB ].high;
            function( a, b );
            inputNumA = ( inputNumA + 1 ) & ( numInputs_float64 - 1 );
            if ( inputNumA == 0 ) ++inputNumB;
            inputNumB = ( inputNumB + 1 ) & ( numInputs_float64 - 1 );
        }
        count += minIterations;
    } while ( clock() - startClock < CLOCKS_PER_SEC );
    inputNumA = 0;
    inputNumB = 0;
    startClock = clock();
    for ( i = count; i; --i ) {
        a.low = inputs_float64[ inputNumA ].low;
        a.high = inputs_float64[ inputNumA ].high;
        b.low = inputs_float64[ inputNumB ].low;
        b.high = inputs_float64[ inputNumB ].high;
        function( a, b );
        inputNumA = ( inputNumA + 1 ) & ( numInputs_float64 - 1 );
        if ( inputNumA == 0 ) ++inputNumB;
        inputNumB = ( inputNumB + 1 ) & ( numInputs_float64 - 1 );
    }
    endClock = clock();
    reportTime( count, endClock - startClock );

}

static void time_abz_float64( float64 function( float64, float64 ) )
{
    clock_t startClock, endClock;
    int32 count, i;
    int8 inputNumA, inputNumB;
    float64 a, b;

    count = 0;
    inputNumA = 0;
    inputNumB = 0;
    startClock = clock();
    do {
        for ( i = minIterations; i; --i ) {
            a.low = inputs_float64[ inputNumA ].low;
            a.high = inputs_float64[ inputNumA ].high;
            b.low = inputs_float64[ inputNumB ].low;
            b.high = inputs_float64[ inputNumB ].high;
            function( a, b );
            inputNumA = ( inputNumA + 1 ) & ( numInputs_float64 - 1 );
            if ( inputNumA == 0 ) ++inputNumB;
            inputNumB = ( inputNumB + 1 ) & ( numInputs_float64 - 1 );
        }
        count += minIterations;
    } while ( clock() - startClock < CLOCKS_PER_SEC );
    inputNumA = 0;
    inputNumB = 0;
    startClock = clock();
    for ( i = count; i; --i ) {
        a.low = inputs_float64[ inputNumA ].low;
        a.high = inputs_float64[ inputNumA ].high;
        b.low = inputs_float64[ inputNumB ].low;
        b.high = inputs_float64[ inputNumB ].high;
        function( a, b );
        inputNumA = ( inputNumA + 1 ) & ( numInputs_float64 - 1 );
        if ( inputNumA == 0 ) ++inputNumB;
        inputNumB = ( inputNumB + 1 ) & ( numInputs_float64 - 1 );
    }
    endClock = clock();
    reportTime( count, endClock - startClock );

}

static const struct {
    bits32 high, low;
} inputs_float64_pos[ numInputs_float64 ] = {
    { 0x422FFFC0, 0x08000000 },
    { 0x37E00004, 0x80000000 },
    { 0x73FD2546, 0x120B7935 },
    { 0x3FF00000, 0x00000000 },
    { 0x4E07F766, 0xF09588D6 },
    { 0x00000000, 0x00000000 },
    { 0x3FCE0004, 0x00000000 },
    { 0x0313B60F, 0x0032BED8 },
    { 0x41EFFFFF, 0xC0002000 },
    { 0x3FB3C75D, 0x224F2B0F },
    { 0x7FD00000, 0x004000FF },
    { 0x212FFF80, 0x00001FFF },
    { 0x3EE00000, 0x00FE0000 },
    { 0x00100000, 0x80000004 },
    { 0x41CFFFFE, 0x00000020 },
    { 0x40303FFF, 0xFFFFFFFD },
    { 0x3FD00000, 0x3FEFFFFF },
    { 0x3FD00000, 0x10000000 },
    { 0x37FC6B5C, 0x16CA55CF },
    { 0x413EEB94, 0x0B9D1301 },
    { 0x47E00200, 0x001FFFFF },
    { 0x47F00021, 0xFFFFFFFE },
    { 0x3FFFFFFF, 0xF80000FF },
    { 0x407FFFFF, 0xE00FFFFF },
    { 0x001497A6, 0x3740C5E8 },
    { 0x44BFFFE0, 0x001FFFFF },
    { 0x16FFDFFE, 0xFFFFFFFF },
    { 0x403FC000, 0x000001FE },
    { 0x7FD00000, 0x000001F6 },
    { 0x06404000, 0x02000000 },
    { 0x479CEE1E, 0x4F789FE0 },
    { 0x4237FFFF, 0xFFFFFDFE }
};

static void time_az_float64_pos( float64 function( float64 ) )
{
    clock_t startClock, endClock;
    int32 count, i;
    int8 inputNum;
    float64 a;

    count = 0;
    inputNum = 0;
    startClock = clock();
    do {
        for ( i = minIterations; i; --i ) {
            a.low = inputs_float64_pos[ inputNum ].low;
            a.high = inputs_float64_pos[ inputNum ].high;
            function( a );
            inputNum = ( inputNum + 1 ) & ( numInputs_float64 - 1 );
        }
        count += minIterations;
    } while ( clock() - startClock < CLOCKS_PER_SEC );
    inputNum = 0;
    startClock = clock();
    for ( i = count; i; --i ) {
        a.low = inputs_float64_pos[ inputNum ].low;
        a.high = inputs_float64_pos[ inputNum ].high;
        function( a );
        inputNum = ( inputNum + 1 ) & ( numInputs_float64 - 1 );
    }
    endClock = clock();
    reportTime( count, endClock - startClock );

}

enum {
    INT32_TO_FLOAT32 = 1,
    INT32_TO_FLOAT64,
    FLOAT32_TO_INT32,
    FLOAT32_TO_INT32_ROUND_TO_ZERO,
    FLOAT32_TO_FLOAT64,
    FLOAT32_ROUND_TO_INT,
    FLOAT32_ADD,
    FLOAT32_SUB,
    FLOAT32_MUL,
    FLOAT32_DIV,
    FLOAT32_REM,
    FLOAT32_SQRT,
    FLOAT32_EQ,
    FLOAT32_LE,
    FLOAT32_LT,
    FLOAT32_EQ_SIGNALING,
    FLOAT32_LE_QUIET,
    FLOAT32_LT_QUIET,
    FLOAT64_TO_INT32,
    FLOAT64_TO_INT32_ROUND_TO_ZERO,
    FLOAT64_TO_FLOAT32,
    FLOAT64_ROUND_TO_INT,
    FLOAT64_ADD,
    FLOAT64_SUB,
    FLOAT64_MUL,
    FLOAT64_DIV,
    FLOAT64_REM,
    FLOAT64_SQRT,
    FLOAT64_EQ,
    FLOAT64_LE,
    FLOAT64_LT,
    FLOAT64_EQ_SIGNALING,
    FLOAT64_LE_QUIET,
    FLOAT64_LT_QUIET,
    NUM_FUNCTIONS
};

static struct {
    char *name;
    int8 numInputs;
    flag roundingMode, tininessMode;
} functions[ NUM_FUNCTIONS ] = {
    { 0, 0, 0, 0 },
    { "int32_to_float32",                1, TRUE,  FALSE },
    { "int32_to_float64",                1, FALSE, FALSE },
    { "float32_to_int32",                1, TRUE,  FALSE },
    { "float32_to_int32_round_to_zero",  1, FALSE, FALSE },
    { "float32_to_float64",              1, FALSE, FALSE },
    { "float32_round_to_int",            1, TRUE,  FALSE },
    { "float32_add",                     2, TRUE,  FALSE },
    { "float32_sub",                     2, TRUE,  FALSE },
    { "float32_mul",                     2, TRUE,  TRUE, },
    { "float32_div",                     2, TRUE,  FALSE },
    { "float32_rem",                     2, FALSE, FALSE },
    { "float32_sqrt",                    1, TRUE,  FALSE },
    { "float32_eq",                      2, FALSE, FALSE },
    { "float32_le",                      2, FALSE, FALSE },
    { "float32_lt",                      2, FALSE, FALSE },
    { "float32_eq_signaling",            2, FALSE, FALSE },
    { "float32_le_quiet",                2, FALSE, FALSE },
    { "float32_lt_quiet",                2, FALSE, FALSE },
    { "float64_to_int32",                1, TRUE,  FALSE },
    { "float64_to_int32_round_to_zero",  1, FALSE, FALSE },
    { "float64_to_float32",              1, TRUE,  TRUE, },
    { "float64_round_to_int",            1, TRUE,  FALSE },
    { "float64_add",                     2, TRUE,  FALSE },
    { "float64_sub",                     2, TRUE,  FALSE },
    { "float64_mul",                     2, TRUE,  TRUE, },
    { "float64_div",                     2, TRUE,  FALSE },
    { "float64_rem",                     2, FALSE, FALSE },
    { "float64_sqrt",                    1, TRUE,  FALSE },
    { "float64_eq",                      2, FALSE, FALSE },
    { "float64_le",                      2, FALSE, FALSE },
    { "float64_lt",                      2, FALSE, FALSE },
    { "float64_eq_signaling",            2, FALSE, FALSE },
    { "float64_le_quiet",                2, FALSE, FALSE },
    { "float64_lt_quiet",                2, FALSE, FALSE }
};

enum {
    ROUND_NEAREST_EVEN = 1,
    ROUND_TO_ZERO,
    ROUND_DOWN,
    ROUND_UP,
    NUM_ROUNDINGMODES
};
enum {
    TININESS_BEFORE_ROUNDING = 1,
    TININESS_AFTER_ROUNDING,
    NUM_TININESSMODES
};

static void
 timeFunctionVariety(
     uint8 functionCode, int8 roundingMode, int8 tininessMode )
{
    uint8 roundingCode;
    int8 tininessCode;

    functionName = functions[ functionCode ].name;
    switch ( roundingMode ) {
     case 0:
        roundingModeName = 0;
        roundingCode = float_round_nearest_even;
        break;
     case ROUND_NEAREST_EVEN:
        roundingModeName = "nearest_even";
        roundingCode = float_round_nearest_even;
        break;
     case ROUND_TO_ZERO:
        roundingModeName = "to_zero";
        roundingCode = float_round_to_zero;
        break;
     case ROUND_DOWN:
        roundingModeName = "down";
        roundingCode = float_round_down;
        break;
     case ROUND_UP:
        roundingModeName = "up";
        roundingCode = float_round_up;
        break;
    }
    float_rounding_mode = roundingCode;
    switch ( tininessMode ) {
     case 0:
        tininessModeName = 0;
        tininessCode = float_tininess_after_rounding;
        break;
     case TININESS_BEFORE_ROUNDING:
        tininessModeName = "before";
        tininessCode = float_tininess_before_rounding;
        break;
     case TININESS_AFTER_ROUNDING:
        tininessModeName = "after";
        tininessCode = float_tininess_after_rounding;
        break;
    }
    float_detect_tininess = tininessCode;
    switch ( functionCode ) {
     case INT32_TO_FLOAT32:
        time_a_int32_z_float32( int32_to_float32 );
        break;
     case INT32_TO_FLOAT64:
        time_a_int32_z_float64( int32_to_float64 );
        break;
     case FLOAT32_TO_INT32:
        time_a_float32_z_int32( float32_to_int32 );
        break;
     case FLOAT32_TO_INT32_ROUND_TO_ZERO:
        time_a_float32_z_int32( float32_to_int32_round_to_zero );
        break;
     case FLOAT32_TO_FLOAT64:
        time_a_float32_z_float64( float32_to_float64 );
        break;
     case FLOAT32_ROUND_TO_INT:
        time_az_float32( float32_round_to_int );
        break;
     case FLOAT32_ADD:
        time_abz_float32( float32_add );
        break;
     case FLOAT32_SUB:
        time_abz_float32( float32_sub );
        break;
     case FLOAT32_MUL:
        time_abz_float32( float32_mul );
        break;
     case FLOAT32_DIV:
        time_abz_float32( float32_div );
        break;
     case FLOAT32_REM:
        time_abz_float32( float32_rem );
        break;
     case FLOAT32_SQRT:
        time_az_float32_pos( float32_sqrt );
        break;
     case FLOAT32_EQ:
        time_ab_float32_z_flag( float32_eq );
        break;
     case FLOAT32_LE:
        time_ab_float32_z_flag( float32_le );
        break;
     case FLOAT32_LT:
        time_ab_float32_z_flag( float32_lt );
        break;
     case FLOAT32_EQ_SIGNALING:
        time_ab_float32_z_flag( float32_eq_signaling );
        break;
     case FLOAT32_LE_QUIET:
        time_ab_float32_z_flag( float32_le_quiet );
        break;
     case FLOAT32_LT_QUIET:
        time_ab_float32_z_flag( float32_lt_quiet );
        break;
     case FLOAT64_TO_INT32:
        time_a_float64_z_int32( float64_to_int32 );
        break;
     case FLOAT64_TO_INT32_ROUND_TO_ZERO:
        time_a_float64_z_int32( float64_to_int32_round_to_zero );
        break;
     case FLOAT64_TO_FLOAT32:
        time_a_float64_z_float32( float64_to_float32 );
        break;
     case FLOAT64_ROUND_TO_INT:
        time_az_float64( float64_round_to_int );
        break;
     case FLOAT64_ADD:
        time_abz_float64( float64_add );
        break;
     case FLOAT64_SUB:
        time_abz_float64( float64_sub );
        break;
     case FLOAT64_MUL:
        time_abz_float64( float64_mul );
        break;
     case FLOAT64_DIV:
        time_abz_float64( float64_div );
        break;
     case FLOAT64_REM:
        time_abz_float64( float64_rem );
        break;
     case FLOAT64_SQRT:
        time_az_float64_pos( float64_sqrt );
        break;
     case FLOAT64_EQ:
        time_ab_float64_z_flag( float64_eq );
        break;
     case FLOAT64_LE:
        time_ab_float64_z_flag( float64_le );
        break;
     case FLOAT64_LT:
        time_ab_float64_z_flag( float64_lt );
        break;
     case FLOAT64_EQ_SIGNALING:
        time_ab_float64_z_flag( float64_eq_signaling );
        break;
     case FLOAT64_LE_QUIET:
        time_ab_float64_z_flag( float64_le_quiet );
        break;
     case FLOAT64_LT_QUIET:
        time_ab_float64_z_flag( float64_lt_quiet );
        break;
    }

}

static void
 timeFunction( uint8 functionCode, int8 roundingModeIn, int8 tininessModeIn )
{
    int8 roundingMode, tininessMode;

    for ( roundingMode = 1;
          roundingMode < NUM_ROUNDINGMODES;
          ++roundingMode
        ) {
        if ( ! functions[ functionCode ].roundingMode ) {
            roundingMode = 0;
        }
        else if ( roundingModeIn ) {
            roundingMode = roundingModeIn;
        }
        for ( tininessMode = 1;
              tininessMode < NUM_TININESSMODES;
              ++tininessMode
            ) {
            if ( ! functions[ functionCode ].tininessMode ) {
                tininessMode = 0;
            }
            else if ( tininessModeIn ) {
                tininessMode = tininessModeIn;
            }
            timeFunctionVariety( functionCode, roundingMode, tininessMode );
            if ( tininessModeIn || ! tininessMode ) break;
        }
        if ( roundingModeIn || ! roundingMode ) break;
    }

}

main( int argc, char **argv )
{
    char *argPtr;
    flag functionArgument;
    uint8 functionCode;
    int8 operands, roundingMode, tininessMode;

    if ( argc <= 1 ) goto writeHelpMessage;
    functionArgument = FALSE;
    functionCode = 0;
    operands = 0;
    roundingMode = 0;
    tininessMode = 0;
    --argc;
    ++argv;
    while ( argc && ( argPtr = argv[ 0 ] ) ) {
        if ( argPtr[ 0 ] == '-' ) ++argPtr;
        if ( strcmp( argPtr, "help" ) == 0 ) {
 writeHelpMessage:
            fputs(
"timesoftfloat [<option>...] <function>\n"
"  <option>:  (* is default)\n"
"    -help            --Write this message and exit.\n"
"    -nearesteven     --Only time rounding to nearest/even.\n"
"    -tozero          --Only time rounding to zero.\n"
"    -down            --Only time rounding down.\n"
"    -up              --Only time rounding up.\n"
"    -tininessbefore  --Only time underflow tininess before rounding.\n"
"    -tininessafter   --Only time underflow tininess after rounding.\n"
"  <function>:\n"
"    int32_to_<float>                 <float>_add   <float>_eq\n"
"    <float>_to_int32                 <float>_sub   <float>_le\n"
"    <float>_to_int32_round_to_zero   <float>_mul   <float>_lt\n"
"    <float>_to_<float>               <float>_div   <float>_eq_signaling\n"
"    <float>_round_to_int             <float>_rem   <float>_le_quiet\n"
"    <float>_sqrt                                   <float>_lt_quiet\n"
"    -all1            --All 1-operand functions.\n"
"    -all2            --All 2-operand functions.\n"
"    -all             --All functions.\n"
"  <float>:\n"
"    float32          --Single precision.\n"
"    float64          --Double precision.\n",
                stdout
            );
            return EXIT_SUCCESS;
        }
        else if (    ( strcmp( argPtr, "nearesteven" ) == 0 )
                  || ( strcmp( argPtr, "nearest_even" ) == 0 ) ) {
            roundingMode = ROUND_NEAREST_EVEN;
        }
        else if (    ( strcmp( argPtr, "tozero" ) == 0 )
                  || ( strcmp( argPtr, "to_zero" ) == 0 ) ) {
            roundingMode = ROUND_TO_ZERO;
        }
        else if ( strcmp( argPtr, "down" ) == 0 ) {
            roundingMode = ROUND_DOWN;
        }
        else if ( strcmp( argPtr, "up" ) == 0 ) {
            roundingMode = ROUND_UP;
        }
        else if ( strcmp( argPtr, "tininessbefore" ) == 0 ) {
            tininessMode = TININESS_BEFORE_ROUNDING;
        }
        else if ( strcmp( argPtr, "tininessafter" ) == 0 ) {
            tininessMode = TININESS_AFTER_ROUNDING;
        }
        else if ( strcmp( argPtr, "all1" ) == 0 ) {
            functionArgument = TRUE;
            functionCode = 0;
            operands = 1;
        }
        else if ( strcmp( argPtr, "all2" ) == 0 ) {
            functionArgument = TRUE;
            functionCode = 0;
            operands = 2;
        }
        else if ( strcmp( argPtr, "all" ) == 0 ) {
            functionArgument = TRUE;
            functionCode = 0;
            operands = 0;
        }
        else {
            for ( functionCode = 1;
                  functionCode < NUM_FUNCTIONS;
                  ++functionCode 
                ) {
                if ( strcmp( argPtr, functions[ functionCode ].name ) == 0 ) {
                    break;
                }
            }
            if ( functionCode == NUM_FUNCTIONS ) {
                fail( "Invalid option or function `%s'", argv[ 0 ] );
            }
            functionArgument = TRUE;
        }
        --argc;
        ++argv;
    }
    if ( ! functionArgument ) fail( "Function argument required" );
    if ( functionCode ) {
        timeFunction( functionCode, roundingMode, tininessMode );
    }
    else if ( operands == 1 ) {
        for ( functionCode = 1; functionCode < NUM_FUNCTIONS; ++functionCode
            ) {
            if ( functions[ functionCode ].numInputs == 1 ) {
                timeFunction( functionCode, roundingMode, tininessMode );
            }
        }
    }
    else if ( operands == 2 ) {
        for ( functionCode = 1; functionCode < NUM_FUNCTIONS; ++functionCode
            ) {
            if ( functions[ functionCode ].numInputs == 2 ) {
                timeFunction( functionCode, roundingMode, tininessMode );
            }
        }
    }
    else {
        for ( functionCode = 1; functionCode < NUM_FUNCTIONS; ++functionCode
            ) {
            timeFunction( functionCode, roundingMode, tininessMode );
        }
    }
    return EXIT_SUCCESS;

}

