#include <stdio.h>

int main(int /*argc*/, char** /*argv*/)
{
	printf("#define BUILD_TIME %s\n", __TIME__);
	printf("#define BUILD_DATE %s\n", __DATE__);

#ifdef _UNICODE
	printf("#define BUILD_CHAR Unicode\n");
#else
	printf("#define BUILD_CHAR ANSI\n");
#endif

#if !defined BUILD_X64_EXE
	printf("#define BUILD_CPU  X86\n");
#else
	printf("#define BUILD_CPU  X64\n");
#endif

#if defined __GNUC__
	printf("#define BUILD_COMP GCC %i.%i.%i\n", __GNUC__, __GNUC_MINOR__, __GNUC_PATCHLEVEL__);
#elif defined _MSC_VER
 #if _MSC_VER >= 1300 && _MSC_VER < 1310
	printf("#define BUILD_COMP Visual C++ 2002\n");
 #elif _MSC_VER >= 1310 && _MSC_VER < 1320
	printf("#define BUILD_COMP Visual C++ 2003\n");
 #else
	printf("#define BUILD_COMP Visual C++ %i.%i\n", _MSC_VER / 100 - 6, _MSC_VER % 100 / 10);
 #endif
#else
	printf("#define BUILD_COMP Unknown compiler\n");
#endif

// Visual C's resource compiler doesn't define _MSC_VER, but we need it for VERSION resources
#ifdef _MSC_VER
	printf("#ifndef _MSC_VER\n");
	printf(" #define _MSC_VER  %i\n", _MSC_VER);
	printf("#endif\n");
#endif

	return 0;
}
