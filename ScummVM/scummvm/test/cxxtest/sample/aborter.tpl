// -*- C++ -*-
// This template file demonstrates the use of CXXTEST_ABORT_TEST_ON_FAIL
//

#define CXXTEST_HAVE_STD
#define CXXTEST_ABORT_TEST_ON_FAIL
#include <cxxtest/ErrorPrinter.h>

int main()
{
    return CxxTest::ErrorPrinter().run();
}

// The CxxTest "world"
<CxxTest world>
