#include <stdio.h>
#include "Dice.h"

int main()
{
    Dice dice;
    printf( "First roll: %u\n", dice.roll() );
    printf( "Second roll: %u\n", dice.roll() );

    return 0;
}
