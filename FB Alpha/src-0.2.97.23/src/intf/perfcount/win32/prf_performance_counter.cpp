// Profiling support
#include "burner.h"

static double dcntTimerFreq;

static struct { LARGE_INTEGER start; LARGE_INTEGER count[256]; LARGE_INTEGER tally; int index; int max_count; } cntSubsysInfo[16];

int cntExit()
{
	return 0;
}

int cntInit()
{
	LARGE_INTEGER f;

//	cntExit();

	QueryPerformanceFrequency(&f);

	// Use milliseconds when reporting profile information
	dcntTimerFreq = (double)f.QuadPart / 1000.0;

	for (int i = 0; i < 16; i++) {
		for (int j = 0; j < 256; j++) {
			cntSubsysInfo[i].count[j].QuadPart = 0;
		}
		cntSubsysInfo[i].start.QuadPart = 0;
		cntSubsysInfo[i].tally.QuadPart = 0;
		cntSubsysInfo[i].index = 0;
		cntSubsysInfo[i].max_count = 32;
	}

#ifdef PRINT_DEBUG_INFO
   	dprintf(_T("*** PerformanceCounter profiler initialised (timer frequency is %.2lf MHz).\n"), (double)f.QuadPart / 1000000.0);
#endif

	return 0;
}

int cntProfileStart(int nSubSystem)
{
	QueryPerformanceCounter(&cntSubsysInfo[nSubSystem].start);

	return 0;
}

int cntProfileEnd(int nSubSystem)
{
	cntSubsysInfo[nSubSystem].index++;
	if (cntSubsysInfo[nSubSystem].index >= cntSubsysInfo[nSubSystem].max_count) {
		cntSubsysInfo[nSubSystem].index = 0;
	}

	cntSubsysInfo[nSubSystem].tally.QuadPart -= cntSubsysInfo[nSubSystem].count[cntSubsysInfo[nSubSystem].index].QuadPart;

	QueryPerformanceCounter(&cntSubsysInfo[nSubSystem].count[cntSubsysInfo[nSubSystem].index]);
	cntSubsysInfo[nSubSystem].count[cntSubsysInfo[nSubSystem].index].QuadPart -= cntSubsysInfo[nSubSystem].start.QuadPart;

	cntSubsysInfo[nSubSystem].tally.QuadPart += cntSubsysInfo[nSubSystem].count[cntSubsysInfo[nSubSystem].index].QuadPart;

	return 0;
}

double cntProfileReadLast(int nSubSystem)
{
	return (double)cntSubsysInfo[nSubSystem].count[cntSubsysInfo[nSubSystem].index].QuadPart / dcntTimerFreq;
}

double cntProfileReadAverage(int nSubSystem)
{
	return (double)(cntSubsysInfo[nSubSystem].tally.QuadPart / (cntSubsysInfo[nSubSystem].max_count)) / dcntTimerFreq;
}

static int cntProfileGetSettings(InterfaceInfo* pInfo)
{
	TCHAR szString[MAX_PATH] = _T("");

	LARGE_INTEGER f;
	QueryPerformanceFrequency(&f);

	_sntprintf(szString, MAX_PATH, _T("timer frequency is %.2lfMHz\n"), (double)f.QuadPart / 1000000.0);
	IntInfoAddStringModule(pInfo, szString);

	return 0;
}

struct ProfileDo cntDoPerfCount = { cntExit, cntInit, cntProfileStart, cntProfileEnd, cntProfileReadLast, cntProfileReadAverage, cntProfileGetSettings, _T("PerformanceCounter profiler") };
