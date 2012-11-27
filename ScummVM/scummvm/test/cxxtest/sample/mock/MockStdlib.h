#include <T/stdlib.h>

class MockStdlib :
    public T::Base_srand,
    public T::Base_rand,
    public T::Base_time
{
public:
    unsigned lastSeed;

    void srand( unsigned seed )
    {
        lastSeed = seed;
    }

    int nextRand;

    int rand()
    {
        return nextRand;
    }

    time_t nextTime;

    time_t time( time_t *t )
    {
        if ( t )
            *t = nextTime;
        return nextTime;
    }
};
