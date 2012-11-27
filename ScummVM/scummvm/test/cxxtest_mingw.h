
#ifndef CXXTEST_MINGW
#define CXXTEST_MINGW

// HACK to allow building with the SDL backend on MinGW
// see bug #1800764 "TOOLS: MinGW tools building broken"
#ifdef main
#undef main
#endif // main

#endif // CXXTEST_MINGW