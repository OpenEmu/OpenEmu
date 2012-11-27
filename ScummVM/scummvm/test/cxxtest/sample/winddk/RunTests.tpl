// -*- C++ -*-

//
// The DDK doesn't handle <iostream> too well
//
#include <cxxtest/StdioPrinter.h>

int __cdecl main()
{
    return CxxTest::StdioPrinter().run();
}

<CxxTest world>
