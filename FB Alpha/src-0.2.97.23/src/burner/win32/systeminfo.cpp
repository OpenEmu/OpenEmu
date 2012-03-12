#include "burner.h"
#include "build_details.h"
#include <tlhelp32.h>
#include <setupapi.h>
#include <psapi.h>

// these are not defined in the Cygwin/MinGW winapi package
#ifndef DISPLAY_DEVICE_ACTIVE
 #define DISPLAY_DEVICE_ACTIVE              0x00000001
#endif
#ifndef DISPLAY_DEVICE_ATTACHED
 #define DISPLAY_DEVICE_ATTACHED            0x00000002
#endif

static _EXCEPTION_POINTERS* pExceptionPointers;

static TCHAR* pszTextBuffer = NULL;
static int nTextBufferSize = 0;

static int nRecursion = 0;

static HFONT hLogFont, hCodeFont;
static HBRUSH hCodeBGBrush;

static const bool bPrintDriverInfo = false;

static int AddLine(TCHAR* pszFormat, ...)
{
	TCHAR szString[128] = _T("");

	va_list vaFormat;
	va_start(vaFormat, pszFormat);

	int nLen = _vsntprintf(szString, 70, pszFormat, vaFormat);
	nLen = (nLen >= 0 && nLen < 70) ? nLen : 70;
	nLen += _stprintf(szString + nLen, _T("\r\n"));
	TCHAR* pszNewBuffer = (TCHAR*)realloc(pszTextBuffer, (nLen + nTextBufferSize + 1) * sizeof(TCHAR));
	if (pszNewBuffer) {
		pszTextBuffer = pszNewBuffer;
		_tcsncpy(pszTextBuffer + nTextBufferSize, szString, nLen);
		nTextBufferSize += nLen;
		pszTextBuffer[nTextBufferSize] = 0;
	}

	va_end(vaFormat);

	return 0;
}

static int AddText(TCHAR* pszFormat, ...)
{
	TCHAR szString[128] = _T("");

	va_list vaFormat;
	va_start(vaFormat, pszFormat);

	int nLen = _vsntprintf(szString, 70, pszFormat, vaFormat);
	nLen = (nLen >= 0 && nLen < 70) ? nLen : 70;
	TCHAR* pszNewBuffer = (TCHAR*)realloc(pszTextBuffer, (nLen + nTextBufferSize + 1) * sizeof(TCHAR));
	if (pszNewBuffer) {
		pszTextBuffer = pszNewBuffer;
		_tcsncpy(pszTextBuffer + nTextBufferSize, szString, nLen);
		nTextBufferSize += nLen;
		pszTextBuffer[nTextBufferSize] = 0;
	}

	va_end(vaFormat);

	return 0;
}

static int PrintInterfaceInfo(InterfaceInfo* pInfo)
{
	if (pInfo == NULL) {
		return 1;
	}

	if (pInfo->pszModuleName) {
		AddLine(_T("    Selected module:    %s"), pInfo->pszModuleName);
	}
	for (int i = 0; pInfo->ppszInterfaceSettings[i]; i++) {
		AddLine(_T("    %s%s"), (i == 0) ? _T("Interface settings: ") : _T("                    "), pInfo->ppszInterfaceSettings[i]);
	}
	for (int i = 0; pInfo->ppszModuleSettings[i]; i++) {
		AddLine(_T("    %s%s"), (i == 0) ? _T("Module settings:    ") : _T("                    "), pInfo->ppszModuleSettings[i]);
	}

	return 0;
}

// Print information about the exception
int PrintExceptionInfo()
{
	static const struct { DWORD ExceptionCode; const TCHAR* szString; } ExceptionString[] = {

#define EXCEPTION_LIST_ENTRY(exception) { exception, _T(#exception) }

		EXCEPTION_LIST_ENTRY(EXCEPTION_ACCESS_VIOLATION),
		EXCEPTION_LIST_ENTRY(EXCEPTION_ARRAY_BOUNDS_EXCEEDED),
		EXCEPTION_LIST_ENTRY(EXCEPTION_BREAKPOINT),
		EXCEPTION_LIST_ENTRY(EXCEPTION_DATATYPE_MISALIGNMENT),
		EXCEPTION_LIST_ENTRY(EXCEPTION_FLT_DENORMAL_OPERAND),
		EXCEPTION_LIST_ENTRY(EXCEPTION_FLT_DIVIDE_BY_ZERO),
		EXCEPTION_LIST_ENTRY(EXCEPTION_FLT_INEXACT_RESULT),
		EXCEPTION_LIST_ENTRY(EXCEPTION_FLT_INVALID_OPERATION),
		EXCEPTION_LIST_ENTRY(EXCEPTION_FLT_OVERFLOW),
		EXCEPTION_LIST_ENTRY(EXCEPTION_FLT_STACK_CHECK),
		EXCEPTION_LIST_ENTRY(EXCEPTION_FLT_UNDERFLOW),
		EXCEPTION_LIST_ENTRY(EXCEPTION_ILLEGAL_INSTRUCTION),
		EXCEPTION_LIST_ENTRY(EXCEPTION_IN_PAGE_ERROR),
		EXCEPTION_LIST_ENTRY(EXCEPTION_INVALID_DISPOSITION),
		EXCEPTION_LIST_ENTRY(EXCEPTION_INT_DIVIDE_BY_ZERO),
		EXCEPTION_LIST_ENTRY(EXCEPTION_INT_OVERFLOW),
		EXCEPTION_LIST_ENTRY(EXCEPTION_INVALID_HANDLE),
		EXCEPTION_LIST_ENTRY(EXCEPTION_GUARD_PAGE),
		EXCEPTION_LIST_ENTRY(EXCEPTION_NONCONTINUABLE_EXCEPTION),
		EXCEPTION_LIST_ENTRY(EXCEPTION_PRIV_INSTRUCTION),
		EXCEPTION_LIST_ENTRY(EXCEPTION_SINGLE_STEP),
		EXCEPTION_LIST_ENTRY(EXCEPTION_STACK_OVERFLOW),
		{ 0, _T("unspecified exception") }

#undef EXCEPTION_LIST_ENTRY

	};

	int i;

	for (i = 0; ExceptionString[i].ExceptionCode; i++) {
		if (ExceptionString[i].ExceptionCode == pExceptionPointers->ExceptionRecord->ExceptionCode) {
			break;
		}
	}

	AddLine(_T("Exception 0x%08X (%s) thrown.\r\nEIP: 0x%p"), pExceptionPointers->ExceptionRecord->ExceptionCode, ExceptionString[i].szString, pExceptionPointers->ExceptionRecord->ExceptionAddress);
	if (pExceptionPointers->ExceptionRecord->ExceptionCode ==  EXCEPTION_ACCESS_VIOLATION) {
		AddLine(_T(" (attempting to %s address 0x%p)"), pExceptionPointers->ExceptionRecord->ExceptionInformation[0] ? _T("write") : _T("read"), pExceptionPointers->ExceptionRecord->ExceptionInformation[1]);
	}
	AddLine(_T(""));

#ifdef BUILD_X86_ASM
	AddLine(_T("EAX: 0x%08X, EBX: 0x%08X, ECX: 0x%08X, EDX: 0x%08X"), (unsigned int)pExceptionPointers->ContextRecord->Eax, (unsigned int)pExceptionPointers->ContextRecord->Ebx, (unsigned int)pExceptionPointers->ContextRecord->Ecx, (unsigned int)pExceptionPointers->ContextRecord->Edx);
	AddLine(_T("ESI: 0x%08X, EDI: 0x%08X, ESP: 0x%08X, EBP: 0x%08X"), (unsigned int)pExceptionPointers->ContextRecord->Esi, (unsigned int)pExceptionPointers->ContextRecord->Edi, (unsigned int)pExceptionPointers->ContextRecord->Esp, (unsigned int)pExceptionPointers->ContextRecord->Ebp);
#endif

	return 0;
}

// Print OS information
int PrintOSInfo()
{
	OSVERSIONINFOEX osvi;

	memset(&osvi, 0, sizeof(OSVERSIONINFOEX));
	osvi.dwOSVersionInfoSize = sizeof(OSVERSIONINFOEX);
	if (!GetVersionEx((OSVERSIONINFO*)&osvi)) {
		osvi.dwOSVersionInfoSize = sizeof(OSVERSIONINFO);
		GetVersionEx((OSVERSIONINFO*)&osvi);
	}

	AddText(_T("OS:  "));
	{
		if (osvi.dwPlatformId == VER_PLATFORM_WIN32_NT) {
			// http://msdn.microsoft.com/en-us/library/ms724833(VS.85).aspx
			if (osvi.dwMajorVersion == 5 && osvi.dwMinorVersion == 0) {
				AddText(_T("Microsoft Windows 2000 "));
			}
			if (osvi.dwMajorVersion == 5 && osvi.dwMinorVersion == 1) {
				AddText(_T("Microsoft Windows XP "));
			}
			if (osvi.dwMajorVersion == 5 && osvi.dwMinorVersion == 2) {
				AddText(_T("Microsoft Windows 2003 "));
			}
			if (osvi.dwMajorVersion < 5 || osvi.dwMinorVersion > 2) {
				AddText(_T("Microsoft Windows NT %d.%d "), osvi.dwMajorVersion, osvi.dwMinorVersion);
			}
			if (osvi.dwMajorVersion == 6 && osvi.dwMinorVersion == 0 && osvi.wProductType == VER_NT_WORKSTATION) {
				AddText(_T("Microsoft Windows Vista "));
			}
			if (osvi.dwMajorVersion == 6 && osvi.dwMinorVersion == 0 && osvi.wProductType != VER_NT_WORKSTATION) {
				AddText(_T("Microsoft Server 2008 "));
			}
			if (osvi.dwMajorVersion == 6 && osvi.dwMinorVersion == 1 && osvi.wProductType != VER_NT_WORKSTATION) {
				AddText(_T("Microsoft Server 2008 R2 "));
			}
			if (osvi.dwMajorVersion == 6 && osvi.dwMinorVersion == 1 && osvi.wProductType == VER_NT_WORKSTATION) {
				AddText(_T("Microsoft Windows 7 "));
			}

			if (osvi.dwMajorVersion == 5) {
				if (osvi.wProductType == VER_NT_WORKSTATION) {
					if (osvi.wSuiteMask & VER_SUITE_PERSONAL) {
						AddText(_T("Personal "));
					} else {
						AddText(_T("Professional "));
					}
				}
				if (osvi.wProductType == VER_NT_SERVER) {
					if (osvi.wSuiteMask & VER_SUITE_DATACENTER) {
						AddText(_T("DataCenter Server "));
					} else {
						if (osvi.wSuiteMask & VER_SUITE_DATACENTER) {
							AddText(_T("Advanced Server "));
						} else {
							AddText(_T("Server "));
						}
					}
				}
			}
			AddText(_T("%s (build %i)\r\n"), osvi.szCSDVersion, osvi.dwBuildNumber & 0xFFFF);
		}
		if (osvi.dwPlatformId == VER_PLATFORM_WIN32_WINDOWS) {
			if (osvi.dwMajorVersion == 4 && osvi.dwMinorVersion < 10) {
				AddText(_T("Microsoft Windows 95"));
				if (osvi.szCSDVersion[1] == _T('B') || osvi.szCSDVersion[1] == _T('C')) {
					AddText(_T(" OSR2"));
				}
			}
			if (osvi.dwMajorVersion == 4 && osvi.dwMinorVersion == 10) {
				AddText(_T("Microsoft Windows 98"));
				if (osvi.szCSDVersion[1] == _T('A')) {
					AddText(_T(" SE"));
				}
			}
			if (osvi.dwMajorVersion == 4 && osvi.dwMinorVersion == 90) {
				AddText(_T("Microsoft Windows Me"));
			}
			if (osvi.dwMajorVersion != 4 && osvi.dwMinorVersion < 10) {
				AddText(_T("Microsoft Windows %d.%d "), osvi.dwMajorVersion, osvi.dwMinorVersion);
			}
			AddText(_T("\r\n"));
		}
	}

	return 0;
}

int PrintCPUInfo()
{
	// Determine the processor type using the CPUID instruction
	SYSTEM_INFO si;
	unsigned int nSignatureEAX = 0, nSignatureEBX = 0, nVendorEBX = 0, nVendorECX = 0, nVendorEDX = 0;
#if !defined BUILD_X64_EXE
	unsigned int nSignatureECX = 0, nSignatureEDX = 0, nVendorEAX = 0;
#endif
	bool bMMX = false, bXMMI = false, bXMMI64 = false;
	char szCPUText[13] = { 0,0,0,0, 0,0,0,0, 0,0,0,0, 0 };
	TCHAR* szModel = NULL;
	DWORD nSpeed = 0;
	HKEY hKey;

	AddText(_T("CPU: "));

	CPUID(0, nVendorEAX, nVendorEBX, nVendorECX, nVendorEDX);
	CPUID(0, nVendorEAX, nVendorEBX, nVendorECX, nVendorEDX);

	for (int n = 0; n < 4; n++) {
		szCPUText[n + 0] = (nVendorEBX >> (n << 3)) & 255;
		szCPUText[n + 4] = (nVendorEDX >> (n << 3)) & 255;
		szCPUText[n + 8] = (nVendorECX >> (n << 3)) & 255;
	}

	CPUID(1, nSignatureEAX, nSignatureEBX, nSignatureECX, nSignatureEDX);

	if (strcmp("GenuineIntel", szCPUText) == 0) {
		switch (nSignatureEAX & 0x0F00) {
			case 0x0500:
				switch (nSignatureEAX & 0x00F0) {
					case 0x040:
					case 0x080: {
						szModel = _T("Pentium MMX");
							break;
						}
						default: {
							szModel = _T("Pentium");
						}
				}
				break;
			case 0x0600: {
				switch (nSignatureEAX & 0x00F0) {
					case 0x010: {
						szModel = _T("Pentium Pro");
						break;
					}
					case 0x030: {
						szModel = _T("Pentium II \"Klamath\"");
						break;
					}
					case 0x050: {
						if (nSignatureEAX & 0x2000) {
							szModel = _T("Pentium II Xeon \"Drake\"");
							} else {
								szModel = _T("Celeron \"Covington\" / Pentium II \"Deschutes\"");
							}
							break;
						}
					case 0x060: {
						szModel = _T("Celeron A \"Mendochino\" / Mobile Pentium II \"Dixon\"");
						break;
					}
					case 0x070: {
						if (nSignatureEAX & 0x2000) {
							szModel = _T("Pentium III Xeon \"Tanner\"");
						} else {
							szModel = _T("Pentium III \"Katmai\"");
						}
						break;
					}
					case 0x080: {
						switch (nSignatureEBX & 0x0F) {
							case 0x01:
								szModel = _T("Celeron 2 \"Coppermine\"");
								break;
							case 0x03:
								szModel = _T("Pentium III Xeon \"Coppermine\"");
								break;
							case 0x06:
								szModel = _T("Mobile Pentium III \"Geyserville\"");
								break;
							case 0x07:
								szModel = _T("Mobile Celeron 2 \"Coppermine\"");
								break;
							default:
								szModel = _T("Pentium III \"Coppermine\"");
						}
						break;
					}
					case 0x090: {
						szModel = _T("Pentium-M \"Banias\"");
						break;
					}
					case 0x0A0: {
						szModel = _T("Pentium III Xeon \"Cascades\"");
						break;
					}
					case 0x0B0: {
						switch (nSignatureEBX & 0x0F) {
							case 0x03:
								szModel = _T("Celeron 3 \"Tualatin\"");
								break;
							default:
								szModel = _T("Pentium III \"Tualatin\"");
						}
						break;
					}
					default: {
						szModel = _T("P6 family");
					}
				}
				break;
			}
			case 0x0F00: {
				switch (nSignatureEAX & 0x00F0) {
					case 0x000:
					case 0x010: {
						switch (nSignatureEBX & 0x0F) {
							case 0x0A:
								szModel = _T("Celeron 4 \"Willamette\"");
								break;
							case 0x0B:
								if ((nSignatureEAX & 0xFF) < 0x13) {
									szModel = _T("Pentium 4 Xeon MP \"Foster\"");
								} else {
									szModel = _T("Pentium 4 Xeon \"Foster\"");
								}
								break;
							case 0x0C:
								szModel = _T("Pentium 4 Xeon MP \"Foster\"");
								break;
							case 0x0E:
								szModel = _T("Pentium 4 Xeon \"Foster\"");
								break;
							case 0x0F:
								szModel = _T("Mobile Celeron 4 \"Willamette\"");
								break;
							default:
								szModel = _T("Pentium 4 \"Willamette\"");
						}
						break;
					}
					case 0x020: {
						switch (nSignatureEBX & 0x0F) {
							case 0x0A:
								szModel = _T("Celeron 4 \"Northwood\"");
								break;
							case 0x0B:
								szModel = _T("Pentium 4 Xeon \"Prestonia\"");
								break;
							case 0x0C:
								szModel = _T("Pentium 4 Xeon MP \"Prestonia\"");
								break;
							case 0x0E:
								szModel = _T("Mobile Pentium 4 M \"Northwood\"");
								break;
							case 0x0F:
								szModel = _T("Mobile Celeron 4 \"Northwood\"");
								break;
							default:
								szModel = _T("Pentium 4 \"Northwood\"");
						}
						break;
					}
					case 0x030: {
						switch (nSignatureEBX & 0x0F) {
							case 0x02:
								szModel = _T("Celeron 4 \"Prescott\"");
								break;
							case 0x03:
								szModel = _T("Pentium 4 Xeon \"Nocona\"");
								break;
							case 0x04:
								szModel = _T("Pentium 4 Xeon MP \"Nocona\"");
								break;
							case 0x06:
								szModel = _T("Mobile Pentium 4 M \"Prescott\"");
								break;
							case 0x07:
								szModel = _T("Mobile Celeron 4 \"Prescott\"");
								break;
							default:
								szModel = _T("Pentium 4 \"Prescott\"");
						}
						break;
					}
					default: {
						szModel = _T("Pentium 4 family");
					}
				}
				break;
			}
		}
	}

	if (strcmp("AuthenticAMD", szCPUText) == 0) {
		switch (nSignatureEAX & 0x0F00) {
			case 0x0500: {
				switch (nSignatureEAX & 0x00F0) {
					case 0x000:
					case 0x010:
					case 0x020:
					case 0x030:
						szModel = _T("K5");
						break;
					case 0x060:
						szModel = _T("K6");
						break;
					case 0x070:
						szModel = _T("K6-2 \"Little Foot\"");
						break;
					case 0x080:
						szModel = _T("K6-2 \"Chomper\"");
						break;
					case 0x090:
						szModel = _T("K6-III \"Sharptooth\"");
						break;
					case 0x0D0:
						szModel = _T("K6-2+ / K6-III+ \"Sharptooth\"");
						break;
					default:
						szModel = _T("K5 / K6 family");
				}
				break;
			}
			case 0x0600: {
				switch (nSignatureEAX & 0x00F0) {
					case 0x010:
						szModel = _T("Athlon \"Pluto\"");
						break;
					case 0x020:
						szModel = _T("Athlon \"Orion\"");
						break;
					case 0x030:
						szModel = _T("Duron \"Spitfire\"");
						break;
					case 0x040:
						szModel = _T("Athon \"Thunderbird\"");
						break;
					case 0x060:
						if (nSignatureEAX & 0x2000) {
							szModel = _T("Athlon MP \"Palomino\"");
						} else {
							szModel = _T("Mobile Athon 4 / Athon XP \"Palomino\"");
						}
						break;
					case 0x070:
						szModel = _T("Mobile Duron \"Camaro\" / Duron \"Morgan\"");
						break;
					case 0x080:
						if (nSignatureEAX & 0x2000) {
							szModel = _T("Sempron / Athlon MP \"Thoroughbred\"");
						} else {
							szModel = _T("Duron \"Applebred\" / Athon XP \"Thoroughbred\"");
						}
						break;
					case 0x0A0:
						if (nSignatureEAX & 0x2000) {
							szModel = _T("Sempron \"Thorton\" / Athlon MP \"Barton\"");
						} else {
							// Thorton has 256KB cache, Barton has 512KB
							szModel = _T("Athlon XP \"Thorton\" / Athon XP \"Barton\"");
						}
						break;
					default:
						szModel = _T("K7 family");
				}
				break;
			}
			case 0x0F00: {
				switch (nSignatureEAX & 0x00F0) {
					case 0x040:	// 1024KB l2 cache, socket 754
					case 0x080:	//  256KB l2 cache, socket 754
					case 0x0B0:	//  256KB l2 cache, socket 939
					case 0x0C0:	//  512KB l2 cache, socket 754
						szModel = _T("Athlon 64 / Athlon 64 Mobile");
						break;
					case 0x070:	// 1024KB l2 cache, socket 939
					case 0x0F0:	//  512KB l2 cache, socket 939
						szModel = _T("Athlon 64 FX / Athlon 64 / Athlon 64 Mobile");
						break;
					case 0x050:	// 1024KB l2 cache, socket 940
						szModel = _T("Athlon 64 FX / Opteron / Opteron FX");
						break;
					default:
						szModel = _T("x86-64 family");
				}
				break;
			}
		}
	}

	// Get the OS hardware info
	GetSystemInfo(&si);

	// Determine speed (read from the registry as there's no reliable way to determine it on modern systems)
	if (RegOpenKeyEx(HKEY_LOCAL_MACHINE, _T("HARDWARE\\DESCRIPTION\\System\\CentralProcessor\\0"), 0, KEY_EXECUTE, &hKey) == ERROR_SUCCESS) {
		DWORD nType = REG_DWORD;
		DWORD nSize = sizeof(nSpeed);

		RegQueryValueEx(hKey, _T("~MHz"), NULL, &nType, (BYTE*)&nSpeed, &nSize);
		RegCloseKey(hKey);
	}

	// Determine extensions supported (MMX, etc.)
	{
		BOOL (WINAPI* pIsProcessorFeaturePresent)(DWORD) = NULL;
		HINSTANCE hKernel32DLL;

		hKernel32DLL = LoadLibrary(_T("kernel32.dll"));
		if (hKernel32DLL) {
			pIsProcessorFeaturePresent = (BOOL (WINAPI*)(DWORD))GetProcAddress(hKernel32DLL, "IsProcessorFeaturePresent");
			if (pIsProcessorFeaturePresent) {
				if (pIsProcessorFeaturePresent(PF_MMX_INSTRUCTIONS_AVAILABLE)) {
					bMMX = true;
				}
				if (pIsProcessorFeaturePresent(PF_XMMI_INSTRUCTIONS_AVAILABLE)) {
					bXMMI = true;
				}
				if (pIsProcessorFeaturePresent(PF_XMMI64_INSTRUCTIONS_AVAILABLE)) {
					bXMMI64 = true;
				}
			}
			FreeLibrary(hKernel32DLL);
		}
	}

	// Finally, print out the info
	if (szModel) {
		AddLine(_T("%12hs, %s"), szCPUText, szModel);
	} else {
		AddLine(_T("%12hs, Unknown CPU (CPUID 0x%08X:%08X)"), szCPUText, nSignatureEAX, nSignatureEBX);
	}

	if (nSpeed) {
		AddText(_T("     %i MHz"), nSpeed);
	} else {
		AddText(_T("     unknown speed"));
	}
	if (bMMX || bXMMI || bXMMI64) {
		AddText(_T("%s%s%s"), bMMX ? _T(", MMX") : _T(""), bXMMI ? _T(", SSE") : _T(""), bXMMI64 ? _T(", SSE2") : _T(""));
	}
	AddText(_T(" (%i system processor%s)\r\n"), si.dwNumberOfProcessors, si.dwNumberOfProcessors > 1 ? _T("s") : _T(""));

	return 0;
}

// Print global memory information
int PrintGlobalMemoryInfo()
{
	MEMORYSTATUS stat;
	GlobalMemoryStatus(&stat);

	AddLine(_T("Physical RAM: %7i KB (%4i MB) total, %7i KB (%4i MB) avail"), stat.dwTotalPhys / 1024, stat.dwTotalPhys / (1024 * 1024), stat.dwAvailPhys / 1024, stat.dwAvailPhys / (1024 * 1024));
	AddLine(_T("Total RAM:    %7i KB (%4i MB) total, %7i KB (%4i MB) avail"), stat.dwTotalPageFile / 1024, stat.dwTotalPageFile / (1024 * 1024), stat.dwAvailPageFile / 1024, stat.dwAvailPageFile / (1024 * 1024));

	// Information on FB Alpha memory usage
	BOOL (WINAPI* pGetProcessMemoryInfo)(HANDLE, PPROCESS_MEMORY_COUNTERS, DWORD) = NULL;
	HINSTANCE hPsapiDLL;

	hPsapiDLL = LoadLibrary(_T("psapi.dll"));
	if (hPsapiDLL) {
		pGetProcessMemoryInfo = (BOOL (WINAPI*)(HANDLE, PPROCESS_MEMORY_COUNTERS, DWORD))GetProcAddress(hPsapiDLL, "GetProcessMemoryInfo");
		if (pGetProcessMemoryInfo) {
			PROCESS_MEMORY_COUNTERS pmc;

			if (pGetProcessMemoryInfo(GetCurrentProcess(), &pmc, sizeof(pmc))) {
				TCHAR szLine[1024] = _T("");
				int length = _tcslen(_T(APP_TITLE));
				if (length > 12) {
					length = 12;
				}
				_sntprintf(szLine, 12, _T(APP_TITLE));
				_sntprintf(szLine + length, 14 - length, _T(":                 "));
				AddLine(_T("%s%7i KB in use (%i KB peak, %i KB virtual)"), szLine, pmc.WorkingSetSize / 1024, pmc.PeakWorkingSetSize / 1024, pmc.PagefileUsage / 1024);
			}
		}
		FreeLibrary(hPsapiDLL);
	}

	return 0;
}

// Find the registry key for this display in the hardware area of the registry
// szHardwareID is the PNP ID
// szDriver     is the GUID/instance of the driver
HKEY FindMonitor(TCHAR* szHardwareID, TCHAR* szDriver)
{
	TCHAR szName[1024] = _T("");  DWORD nNameSize = sizeof(szName);
	TCHAR szClass[1024] = _T(""); DWORD nClassSize = sizeof(szClass);
	FILETIME ftLastWriteTime;
	HKEY hKey = NULL;
	HKEY hMonitorKey = NULL;

	bool bFound = false;

	// We need to enumerate all displays and all instances to check the values inside them
	RegOpenKeyEx(HKEY_LOCAL_MACHINE, _T("SYSTEM\\CurrentControlSet\\Enum\\DISPLAY\\"), 0, KEY_READ, &hKey);
	for (int i = 0; !bFound && RegEnumKeyEx(hKey, i, szName, &nNameSize, NULL, szClass, &nClassSize, &ftLastWriteTime) != ERROR_NO_MORE_ITEMS; i++) {
		TCHAR szSubName[1024] = _T("");  DWORD nSubNameSize = sizeof(szSubName);
		TCHAR szSubClass[1024] = _T(""); DWORD nSubClassSize = sizeof(szSubClass);
		FILETIME ftSubLastWriteTime;
		HKEY hSubKey = NULL;

		nNameSize = sizeof(szName);
		nClassSize = sizeof(szClass);

		RegOpenKeyEx(hKey, szName, 0, KEY_READ, &hSubKey);
		for (int j = 0; !bFound && RegEnumKeyEx(hSubKey, j, szSubName, &nSubNameSize, NULL, szSubClass, &nSubClassSize, &ftSubLastWriteTime) != ERROR_NO_MORE_ITEMS; j++) {
			HKEY hMonitorInfoKey = NULL;
			TCHAR szKeyValue[1024]; DWORD nSize = sizeof(szKeyValue);
			DWORD nType;

			nSubNameSize = sizeof(szSubName);
			nSubClassSize = sizeof(szSubClass);

			RegOpenKeyEx(hSubKey, szSubName, 0, KEY_READ, &hMonitorKey);

			// Check if this instance is the one we're looking for
			nType = REG_SZ; nSize = sizeof(szKeyValue);
			RegQueryValueEx(hMonitorKey, _T("HardwareID"), NULL, &nType, (BYTE*)szKeyValue, &nSize);
			if (_tcsicmp(szKeyValue, szHardwareID)) {
				continue;
			}
			nType = REG_SZ; nSize = sizeof(szKeyValue);
			RegQueryValueEx(hMonitorKey, _T("Driver"), NULL, &nType, (BYTE*)szKeyValue, &nSize);
			if (_tcsicmp(szKeyValue, szDriver)) {
				continue;
			}

			// Make sure the "Device Parameters" key which contains any EDID data is present
			RegOpenKeyEx(hMonitorKey, _T("Device Parameters"), 0, KEY_READ, &hMonitorInfoKey);
			if (!hMonitorInfoKey) {
				continue;
			}

			// We've found the display we're looking for
			bFound = true;

			RegCloseKey(hMonitorInfoKey);
		}
		RegCloseKey(hSubKey);
	}
	RegCloseKey(hKey);

	if (!bFound) {
		return NULL;
	}

	return hMonitorKey;
}

int ProcessEDID(HKEY hMonitorKey)
{
	HKEY hMonitorInfoKey = NULL;
	BYTE EDIDData[1024]; DWORD nEDIDSize = sizeof(EDIDData);
	DWORD nType;

	if (hMonitorKey == NULL) {
		return 1;
	}

	RegOpenKeyEx(hMonitorKey, _T("Device Parameters"), 0, KEY_READ, &hMonitorInfoKey);
	if (hMonitorInfoKey == NULL) {
		return 1;
	}

	// When Windows can't get valid EDID data from a Display, it creates a BAD_EDID value instead of EDID
	// Thus we can forego ensuring the validity of the EDID data ourselves
	nType = REG_BINARY; nEDIDSize = sizeof(EDIDData);
	if (RegQueryValueEx(hMonitorInfoKey, _T("BAD_EDID"), NULL, &nType, EDIDData, &nEDIDSize) == 0) {
		AddLine(_T("        No EDID data present for this device"));
	}
	nType = REG_BINARY; nEDIDSize = sizeof(EDIDData);
	if (RegQueryValueEx(hMonitorInfoKey, _T("EDID"), NULL, &nType, EDIDData, &nEDIDSize) == 0) {

		// Print some basic information about this display
		AddLine(_T("        Display size ~%dx%dcm, Gamma %1.2lf"), EDIDData[0x15], EDIDData[0x16], ((double)EDIDData[0x17] + 100.0) / 100.0);

		// Print the preferred mode for this display
		if (EDIDData[0x18] & 2) {
			int nPixelClock = ((EDIDData[0x36 + 0x01] << 8) | EDIDData[0x36 + 0x00]) * 10000;

			// Size of the display image in pixels (including blanking and sync)
			int nActiveH = ((EDIDData[0x36 + 0x04] & 0xF0) << 4) | EDIDData[0x36 + 0x02];
			int nBlankH  = ((EDIDData[0x36 + 0x04] & 0x0F) << 8) | EDIDData[0x36 + 0x03];
			int nActiveV = ((EDIDData[0x36 + 0x07] & 0xF0) << 4) | EDIDData[0x36 + 0x05];
			int nBlankV  = ((EDIDData[0x36 + 0x07] & 0x0F) << 8) | EDIDData[0x36 + 0x06];

			// Size of the display image in mm
			int nSizeH = ((EDIDData[0x36 + 0x0E] & 0xF0) << 4) | EDIDData[0x36 + 0x0C];
			int nSizeV = ((EDIDData[0x36 + 0x0E] & 0x0F) << 8) | EDIDData[0x36 + 0x0D];

			// We need to calculate the refresh rate ourselves based on the other numbers
			double dRefresh = 1.0 / ((double)(nActiveH + nBlankH) * (nActiveV + nBlankV) / nPixelClock);

			AddLine(_T("        Preferred mode %dx%d, %1.3lf Hz (%dx%dmm, %1.3lf MHz)"), nActiveH, nActiveV, dRefresh, nSizeH, nSizeV, nPixelClock / 1000000.0);
		}

		{
			// Print the signal limits for this display

			int nLimitsOffset = 0;

			// Find the data block containing the limits
			for (int i = 0; i < 4; i++) {
				if (EDIDData[0x36 + i * 0x12 + 0] == 0x00 && EDIDData[0x36 + i * 0x12 + 1] == 0x00 && EDIDData[0x36 + i * 0x12 + 2] == 0x00 && EDIDData[0x36 + i * 0x12 + 3] == 0xFD) {
					nLimitsOffset = 0x36 + i * 0x12;
					break;
				}
			}

			if (nLimitsOffset) {
				AddLine(_T("        Max. bandwidth %d MHz, H sync %d-%d KHz, V sync %d-%d Hz"), EDIDData[nLimitsOffset + 0x09] * 10, EDIDData[nLimitsOffset + 0x07], EDIDData[nLimitsOffset + 0x08], EDIDData[nLimitsOffset + 0x05], EDIDData[nLimitsOffset + 0x06]);
			}
		}

#if 0
		{

			// Print the raw data block
			BYTE* pData = EDIDData;

			AddText(_T("\r\n        Raw EDID data\r\n"));
			for (int n = 0; n < 8; n++) {
				AddText(_T("            "));

				for (int m = 0; m < 16; m++) {
					AddText(_T("%02X "), *pData & 255);
					pData++;
				}
				AddText(_T(""));
			}
			AddText(_T(""));
		}
#endif

	}

	RegCloseKey(hMonitorInfoKey);

	return 0;
}

// Print info about displays and display adapters
int PrintDisplayInfo()
{
	BOOL (WINAPI* pEnumDisplayDevices)(LPCTSTR, DWORD, PDISPLAY_DEVICE, DWORD) = NULL;
	HINSTANCE hUser32DLL;

	// The EnumDisplayDevices() function is only available on NT based OSes
	hUser32DLL = LoadLibrary(_T("user32.dll"));
	if (hUser32DLL) {
#if defined (UNICODE)
		pEnumDisplayDevices = (BOOL (WINAPI*)(LPCTSTR, DWORD, PDISPLAY_DEVICE, DWORD))GetProcAddress(hUser32DLL, "EnumDisplayDevicesW");
#else
		pEnumDisplayDevices = (BOOL (WINAPI*)(LPCTSTR, DWORD, PDISPLAY_DEVICE, DWORD))GetProcAddress(hUser32DLL, "EnumDisplayDevicesA");
#endif
		if (pEnumDisplayDevices) {
			DISPLAY_DEVICE ddAdapter;
			DISPLAY_DEVICE ddDisplay;

			ddAdapter.cb = sizeof(DISPLAY_DEVICE);
			ddDisplay.cb = sizeof(DISPLAY_DEVICE);

			// Now that we've ensured we can use the EnumDisplayDevices() function, use it to enumerate the connected displays
			AddLine(_T("Installed displays and display adapters:"));

			for (int i = 0; pEnumDisplayDevices(NULL, i, &ddAdapter, 0); i++) {

				// We're only interested in real display adapters
				if (!(ddAdapter.StateFlags & DISPLAY_DEVICE_MIRRORING_DRIVER)) {
					for (int j = 0; pEnumDisplayDevices(ddAdapter.DeviceName, j, &ddDisplay, 0); j++) {

						HKEY hKey = NULL;
						HKEY hMonitorKey = NULL;

						TCHAR szMfg[1024] = _T("");		   DWORD nMfgSize = sizeof(szMfg);
						TCHAR szDeviceDesc[1024] = _T(""); DWORD nDeviceDescSize = sizeof(szDeviceDesc);

						// If the display is active, get the data about it
						if (ddDisplay.StateFlags & DISPLAY_DEVICE_ACTIVE) {
							if (!_tcsnicmp(ddDisplay.DeviceKey, _T("\\Registry\\Machine\\"), 18)) {
								TCHAR szDriver[1024]; DWORD nSize = sizeof(szDriver);
								DWORD nType = REG_SZ;
								RegOpenKeyEx(HKEY_LOCAL_MACHINE, ddDisplay.DeviceKey + 18, 0, KEY_READ, &hKey);

								// Find the registry key for this display in the hardware area of the registry
								RegQueryValueEx(hKey, _T("MatchingDeviceId"), NULL, &nType, (BYTE*)szDriver, &nSize);
								hMonitorKey = FindMonitor(szDriver, ddDisplay.DeviceKey + 57);

								RegQueryValueEx(hMonitorKey, _T("DeviceDesc"), NULL, &nType, (BYTE*)szDeviceDesc, &nDeviceDescSize);
								RegQueryValueEx(hMonitorKey, _T("Mfg"), NULL, &nType, (BYTE*)szMfg, &nMfgSize);
							}

							// Print the information we've got so far
							TCHAR* pszStatus = _T("");
							if (ddAdapter.StateFlags & DISPLAY_DEVICE_PRIMARY_DEVICE) {
								pszStatus = _T(" (primary)");
							}
							if (!(ddAdapter.StateFlags & DISPLAY_DEVICE_ATTACHED_TO_DESKTOP)) {
								pszStatus = _T(" (disabled)");
							}
							if (szMfg[0] && szDeviceDesc[0]) {
								AddLine(_T("    %s %s on %s%s"), szMfg, szDeviceDesc, ddAdapter.DeviceString, pszStatus);
							} else {
								AddLine(_T("    %s on %s%s"), ddDisplay.DeviceString, ddAdapter.DeviceString, pszStatus);
							}

							ProcessEDID(hMonitorKey);

							if (hMonitorKey) {
								RegCloseKey(hMonitorKey);
							}
							if (hKey) {
								RegCloseKey(hKey);
							}
						}
					}
				}
			}
		}
		FreeLibrary(hUser32DLL);
	}

	return 0;
}

// Print FB Alpha settings
int PrintFBAInfo()
{
	InterfaceInfo* pInfo;

	AddLine(_T(APP_TITLE) _T(" information:"));
	AddLine(_T(""));

	AddLine(_T("Built on ") _T(MAKE_STRING(BUILD_DATE)) _T(", ") _T(MAKE_STRING(BUILD_TIME))  _T(", using ") _T(MAKE_STRING(BUILD_COMP)) _T("."));
	AddLine(_T("    Optimised for ") _T(MAKE_STRING(BUILD_CPU)) _T(" CPUs."));
#if defined (_UNICODE)
	AddLine(_T("    Using Unicode for all text."));
#else
	AddLine(_T("    Using multi-byte characters for all text, active codepage is %d."), GetACP());
#endif
#if defined (FBA_DEBUG)
	AddLine(_T("    Debug functionality present."));
#else
	AddLine(_T("    Debug functionality absent."));
#endif
	AddLine(_T(""));

	AddLine(_T("MMX optimisations %s."), bBurnUseMMX ? _T("enabled") : _T("disabled"));
	if (bBurnUseASMCPUEmulation) {
		AddLine(_T("A68K emulation core enabled for MC68000 emulation."));
		AddLine(_T("Musashi emulation core enabled for MC68010/MC68EC020 emulation."));
	} else {
		AddLine(_T("Musashi emulation core enabled for MC680x0 family emulation."));
	}
	AddLine(_T(""));

	if (bDrvOkay) {
		TCHAR szName[1024];

		int n = _stprintf(szName, _T("Emulating %s (%hs)"), BurnDrvGetText(DRV_NAME), DecorateGameName(nBurnDrvActive));
		if (n >= 70) {
			_tcscpy(szName + 66, _T("...)"));
		}
		AddLine(_T("%s"), szName);
		AddLine(_T("    Vertical refresh is %.2lf Hz."), (double)nBurnFPS / 100);
		AddLine(_T("    CPU running at %i%% of normal frequency."), nBurnCPUSpeedAdjust * 100 / 256);
		AddLine(_T(""));
	} else {
		AddLine(_T("Not emulating any game."));
		AddLine(_T(""));
	}

	if ((pInfo = VidGetInfo()) != NULL) {
		AddLine(_T("Video settings:"));
		PrintInterfaceInfo(pInfo);
	}
	AddLine(_T(""));

	if ((pInfo = AudGetInfo()) != NULL) {
		AddLine(_T("Audio settings:"));
		PrintInterfaceInfo(pInfo);
	}
	AddLine(_T(""));

	if ((pInfo = InputGetInfo()) != NULL) {
		AddLine(_T("Input settings:"));
		PrintInterfaceInfo(pInfo);
	}
	AddLine(_T(""));

	if ((pInfo = ProfileGetInfo()) != NULL) {
		AddLine(_T("Profiling settings:"));
		PrintInterfaceInfo(pInfo);
	}

	return 0;
}

// Print process information
int PrintProcessInfo()
{
#if 1

	HANDLE hModuleSnap;
	MODULEENTRY32 me32;

	hModuleSnap = CreateToolhelp32Snapshot(TH32CS_SNAPMODULE, GetCurrentProcessId());
	if (hModuleSnap == INVALID_HANDLE_VALUE) {
		AddLine(_T("Unable to retrieve detailed process information."));
	} else {
		me32.dwSize = sizeof(MODULEENTRY32);
		if(!Module32First(hModuleSnap, &me32)) {
			AddLine(_T("Unable to retrieve detailed process information."));
		} else {
			AddLine(_T("Detailed process information about %s:"), me32.szModule);
			AddLine(_T(""));
			AddLine(_T("%s (base address 0x%p, size %i KB)"), me32.szModule, me32.modBaseAddr, me32.modBaseSize / 1024);
			AddLine(_T(""));

			if (pExceptionPointers) {
				bool bFound = false;
				do {
					if (me32.modBaseAddr <= pExceptionPointers->ExceptionRecord->ExceptionAddress && (me32.modBaseAddr + me32.modBaseSize) > pExceptionPointers->ExceptionRecord->ExceptionAddress) {
						bFound = true;
						break;
					}
				} while (Module32Next(hModuleSnap, &me32));

				if (bFound) {
					AddLine(_T("Exception occurred in module %s:"), me32.szModule);
					AddLine(_T("    %20s (base address 0x%p, size %6i KB)"), me32.szModule, me32.modBaseAddr, me32.modBaseSize / 1024);
					AddLine(_T(""));
				} else {
					AddLine(_T("Unable to locate module in which exception occurred"));
					AddLine(_T(""));
				}
			}

			Module32First(hModuleSnap, &me32);
			AddLine(_T("Modules loaded by %s:"), me32.szModule);
			while (Module32Next(hModuleSnap, &me32)){
				AddLine(_T("    %20s (base address 0x%p, size %6i KB)"), me32.szModule, me32.modBaseAddr, me32.modBaseSize / 1024);
			}
		}

		CloseHandle(hModuleSnap);
	}

#endif

	return 0;
}

// Print information about installed devices
int PrintDeviceInfo()
{
	// Get a list of all devices that are present and enabled
	HDEVINFO hDevInfo = SetupDiGetClassDevs(NULL, NULL, NULL, DIGCF_ALLCLASSES | DIGCF_PRESENT | DIGCF_PROFILE);
	if (hDevInfo != INVALID_HANDLE_VALUE) {
		SC_HANDLE (WINAPI* pOpenSCManager)(LPCTSTR, LPCTSTR, DWORD) = NULL;
		SC_HANDLE (WINAPI* pOpenService)(SC_HANDLE, LPCTSTR, DWORD) = NULL;
		BOOL (WINAPI* pQueryServiceConfig)(SC_HANDLE, LPQUERY_SERVICE_CONFIG, DWORD, LPDWORD) = NULL;
		BOOL (WINAPI* pCloseServiceHandle)(SC_HANDLE) = NULL;
		HINSTANCE hAdvapi32DLL;

		SP_DEVINFO_DATA did;

		hAdvapi32DLL = LoadLibrary(_T("advapi32.dll"));
		if (hAdvapi32DLL) {
#if defined (_UNICODE)
			pOpenSCManager = (SC_HANDLE (WINAPI*)(LPCTSTR, LPCTSTR, DWORD))GetProcAddress(hAdvapi32DLL, "OpenSCManagerW");
			pOpenService = (SC_HANDLE (WINAPI*)(SC_HANDLE, LPCTSTR, DWORD))GetProcAddress(hAdvapi32DLL, "OpenServiceW");
			pQueryServiceConfig = (BOOL (WINAPI*)(SC_HANDLE, LPQUERY_SERVICE_CONFIG, DWORD, LPDWORD))GetProcAddress(hAdvapi32DLL, "QueryServiceConfigW");
#else
			pOpenSCManager = (SC_HANDLE (WINAPI*)(LPCTSTR, LPCTSTR, DWORD))GetProcAddress(hAdvapi32DLL, "OpenSCManagerA");
			pOpenService = (SC_HANDLE (WINAPI*)(SC_HANDLE, LPCTSTR, DWORD))GetProcAddress(hAdvapi32DLL, "OpenServiceA");
			pQueryServiceConfig = (BOOL (WINAPI*)(SC_HANDLE, LPQUERY_SERVICE_CONFIG, DWORD, LPDWORD))GetProcAddress(hAdvapi32DLL, "QueryServiceConfigA");
#endif
			pCloseServiceHandle = (BOOL (WINAPI*)(SC_HANDLE))GetProcAddress(hAdvapi32DLL, "CloseServiceHandle");
		}

		AddLine(_T("Installed devices (partial list):"));
		AddLine(_T(""));

		// Enumerate all devices in the list
		did.cbSize = sizeof(SP_DEVICE_INTERFACE_DATA);
		for (int i = 0; SetupDiEnumDeviceInfo(hDevInfo, i, &did); i++) {
			TCHAR* list[] = { _T("display"), _T("media"), _T("hid"), _T("hidclass"), _T("mouse"), _T("system"), NULL };
			TCHAR szClass[1024] = _T("");

			// Get the installer class of a device
			SetupDiGetDeviceRegistryProperty(hDevInfo, &did, SPDRP_CLASS, NULL, (BYTE*)szClass, 1024, NULL);

			// determine if the device is of a class we're interested in
			for (int j = 0; list[j]; j++) {
				if (!_tcsicmp(list[j], szClass)) {
					TCHAR szName[1024] = _T("");
					TCHAR szService[1024] = _T("");
					TCHAR szDriverDate[1024] = _T("");
					TCHAR szDriverVersion[1024] = _T("");
					TCHAR szImagePath[1024] = _T("");
					HKEY hKey;

					// Get the device name
					SetupDiGetDeviceRegistryProperty(hDevInfo, &did, SPDRP_DEVICEDESC, NULL, (BYTE*)szName, 1024, NULL);

					// Check if there are any lower filter drivers
					SetupDiGetDeviceRegistryProperty(hDevInfo, &did, SPDRP_LOWERFILTERS, NULL, (BYTE*)szService, 1024, NULL);
					if (szService[0] == _T('\0')) {
						// If we haven't got any lower filter drivers, just use the service key
						SetupDiGetDeviceRegistryProperty(hDevInfo, &did, SPDRP_SERVICE, NULL, (BYTE*)szService, 1024, NULL);
					}

					// Get driver info
					hKey = SetupDiOpenDevRegKey(hDevInfo, &did, DICS_FLAG_GLOBAL, 0, DIREG_DRV, KEY_READ);
					if (hKey != INVALID_HANDLE_VALUE) {
						DWORD nType = REG_SZ;
						DWORD nSize = 1024;

						RegQueryValueEx(hKey, _T("DriverVersion"), NULL, &nType, (BYTE*)szDriverVersion, &nSize);
						RegQueryValueEx(hKey, _T("DriverDate"), NULL, &nType, (BYTE*)szDriverDate, &nSize);
						RegCloseKey(hKey);
					}

					// If we have a driver, get the filename
					if (szService[0] && pOpenSCManager && pOpenService && pQueryServiceConfig && pCloseServiceHandle) {
						SC_HANDLE hSCManager, hService;
						QUERY_SERVICE_CONFIG* pSC = NULL;
						DWORD nSize;

						hSCManager = OpenSCManager(NULL, NULL, GENERIC_READ);
						hService = OpenService(hSCManager, szService, GENERIC_READ);

						QueryServiceConfig(hService, NULL, 0, &nSize);
						pSC = (QUERY_SERVICE_CONFIG*)malloc(nSize);

						if (QueryServiceConfig(hService, pSC, nSize, &nSize)) {
							_tcscpy(szImagePath, pSC->lpBinaryPathName);
						}

						if (pSC) {
							free(pSC);
							pSC = NULL;
						}

						CloseServiceHandle(hService);
						CloseServiceHandle(hSCManager);
					}

#if 0
					if (szImagePath[0] == '\0') {
						if (osvi.dwPlatformId == VER_PLATFORM_WIN32_WINDOWS) {
							// Officially, we shouldn't be doing this, but there's no API for getting this info on Win9x
							const TCHAR* pszServices = "SYSTEM\\CurrentControlSet\\Services\\";
							char szFullService[1024] = "";

							_tcscpy(szFullService, pszServices);
							_tcscat(szFullService, szService);
							if (RegOpenKeyEx(HKEY_LOCAL_MACHINE, szFullService, 0, KEY_EXECUTE, &hKey) == ERROR_SUCCESS) {
								DWORD nType = REG_SZ;
								DWORD nSize = 1024;

								RegQueryValueEx(hKey, _T("ImagePath"), NULL, &nType, szImagePath, &nSize);
								RegCloseKey(hKey);
							}
						}
					}
#endif

					// Print the information
					if (j < 2 || szImagePath[0]) {
						AddLine(_T("    %s"), szName);
						AddLine(_T("        %s"), szImagePath[0] ? szImagePath : _T("no driver needed"));

						if (szDriverVersion[0]) {
							AddLine(_T("        version %s (%s)"), szDriverVersion, szDriverDate[0] ? szDriverDate : _T("no date"));
						}
					}

					break;
				}
			}
		}

		FreeLibrary(hAdvapi32DLL);

		SetupDiDestroyDeviceInfoList(hDevInfo);
	}

	return 0;
}

static INT_PTR CALLBACK SysInfoProc(HWND hDlg, UINT Msg, WPARAM wParam, LPARAM lParam)
{
	static int nReturnCode = 0;

	switch (Msg) {
		case WM_INITDIALOG: {
		    time_t nTime;
			tm* tmTime;

			nReturnCode = 0;

			pszTextBuffer = NULL;
			nTextBufferSize = 0;

			time(&nTime);
			tmTime = localtime(&nTime);

			hLogFont = CreateFont(14, 0, 0, 0, FW_NORMAL, 0, 0, 0, 0, 0, 0, DEFAULT_QUALITY, FF_MODERN, _T(""));

			hCodeBGBrush = NULL;
			hCodeFont = NULL;

			if (pExceptionPointers) {
				TCHAR szText[1024] = _T("");
				hCodeBGBrush = CreateSolidBrush(RGB(0,0,0));
				hCodeFont = CreateFont(22, 0, 0, 0, FW_BOLD, 0, 0, 0, 0, 0, 0, ANTIALIASED_QUALITY, FF_MODERN, _T("Lucida Console"));
				SendDlgItemMessage(hDlg, IDC_SYSINFO_CODE, WM_SETFONT, (WPARAM)hCodeFont, (LPARAM)0);

				_stprintf(szText, _T("Guru Meditation #%08X.%08X"), pExceptionPointers->ExceptionRecord->ExceptionCode, (INT_PTR)pExceptionPointers->ExceptionRecord->ExceptionAddress);
				SendDlgItemMessage(hDlg, IDC_SYSINFO_CODE, WM_SETTEXT, (WPARAM)0, (LPARAM)szText);

				AddLine(_T(APP_TITLE) _T(" v%.20s fatal exception report (%s)."), szAppBurnVer, _tasctime(tmTime));

				AddLine(_T(""));
				AddLine(_T("----------------------------------------------------------------------"));

				// Exception
				PrintExceptionInfo();
			} else {
				AddLine(_T(APP_TITLE) _T(" v%.20s system information (%s)."), szAppBurnVer, _tasctime(tmTime));
			}

			AddLine(_T(""));
			AddLine(_T("----------------------------------------------------------------------"));

			AddLine(_T("System information:"));
			AddLine(_T(""));

			// OS information
			PrintOSInfo();

			// CPU information
			PrintCPUInfo();

			AddLine(_T(""));

			// Global memory information
			PrintGlobalMemoryInfo();

			AddLine(_T(""));

			// Displays and display adapters
			PrintDisplayInfo();

			AddLine(_T(""));
			AddLine(_T("----------------------------------------------------------------------"));

			PrintFBAInfo();

			AddLine(_T(""));
			AddLine(_T("----------------------------------------------------------------------"));

			// Process information
			PrintProcessInfo();

			AddLine(_T(""));
			AddLine(_T("----------------------------------------------------------------------"));

			if (bPrintDriverInfo) {
				// Device information
				PrintDeviceInfo();

				AddLine(_T(""));
				AddLine(_T("----------------------------------------------------------------------"));
			}

			SendDlgItemMessage(hDlg, IDC_SYSINFO_EDIT, WM_SETFONT, (WPARAM)hLogFont, (LPARAM)0);
			SendDlgItemMessage(hDlg, IDC_SYSINFO_EDIT, EM_SETMARGINS, (WPARAM)EC_LEFTMARGIN, (LPARAM)3);
			SendDlgItemMessage(hDlg, IDC_SYSINFO_EDIT, WM_SETTEXT, (WPARAM)0, (LPARAM)pszTextBuffer);

			WndInMid(hDlg, hScrnWnd);

			return TRUE;
		}

		case WM_CLOSE: {
			if (pszTextBuffer) {
				free(pszTextBuffer);
				pszTextBuffer = NULL;
			}
			nTextBufferSize = 0;

			DeleteObject(hLogFont);
			if (hCodeFont) {
				DeleteObject(hCodeFont);
			}
			if (hCodeBGBrush) {
				DeleteObject(hCodeBGBrush);
			}
			EndDialog(hDlg, nReturnCode);
			break;
		}

		case WM_CTLCOLORSTATIC: {
			if ((HWND)lParam == GetDlgItem(hDlg, IDC_SYSINFO_EDIT)) {
				return (INT_PTR)GetSysColorBrush(15);
			}
			if ((HWND)lParam == GetDlgItem(hDlg, IDC_SYSINFO_CODE)) {
				SetTextColor((HDC)wParam, RGB(255, 0, 0));
				SetBkMode((HDC)wParam, TRANSPARENT);
				return (INT_PTR)hCodeBGBrush;
			}
			break;
		}

		case WM_COMMAND: {
			if (HIWORD(wParam) == BN_CLICKED) {
				if (LOWORD(wParam) == IDOK) {
					SendMessage(hDlg, WM_CLOSE, 0, 0);
				}
				if (LOWORD(wParam) == IDC_SYSINFO_DEBUG) {
					nReturnCode = 1;
					SendMessage(hDlg, WM_CLOSE, 0, 0);
				}
				if (LOWORD(wParam) == IDC_SYSINFO_SHOW && pExceptionPointers) {
					RECT rect = { 0, 0, 0, 125 };

					SendMessage(hDlg, WM_NEXTDLGCTL, (WPARAM)GetDlgItem(hDlg, IDOK), TRUE);
					EnableWindow(GetDlgItem(hDlg, IDC_SYSINFO_SHOW), FALSE);

					MapDialogRect(hDlg, &rect);
					int nSize = rect.bottom;
					GetWindowRect(hDlg, &rect);
					MoveWindow(hDlg, rect.left, rect.top, rect.right - rect.left, rect.bottom - rect.top + nSize, TRUE);

					WndInMid(hDlg, hScrnWnd);
				}
				if (LOWORD(wParam) == IDC_SYSINFO_LOG) {
					FILE* fp = NULL;

					SendMessage(hDlg, WM_NEXTDLGCTL, (WPARAM)GetDlgItem(hDlg, IDOK), TRUE);

					if (pExceptionPointers) {
						TCHAR szLogName[MAX_PATH];
						_stprintf(szLogName, _T("config/%s.error.log"), szAppExeName);
						fp = _tfopen(szLogName, _T("ab"));

						EnableWindow(GetDlgItem(hDlg, IDC_SYSINFO_LOG), FALSE);
					} else {
						TCHAR szFilter[1024];

						_stprintf(szFilter, FBALoadStringEx(hAppInst, IDS_DISK_TEXT, true));
						memcpy(szFilter + _tcslen(szFilter), _T(" (*.txt)\0*.txt\0\0"), 16 * sizeof(TCHAR));

						_stprintf(szChoice, _T("%s system information.txt"), szAppExeName);

						memset(&ofn, 0, sizeof(ofn));
						ofn.lStructSize = sizeof(ofn);
						ofn.hwndOwner = hDlg;
						ofn.lpstrFilter = szFilter;
						ofn.lpstrFile = szChoice;
						ofn.nMaxFile = sizeof(szChoice) / sizeof(TCHAR);
						ofn.lpstrInitialDir = _T(".");
						ofn.Flags = OFN_NOCHANGEDIR | OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT;
						ofn.lpstrDefExt = _T("txt");
						ofn.lpstrTitle = FBALoadStringEx(hAppInst, IDS_DISK_SAVEREPORT, true);

						if (GetSaveFileName(&ofn)) {
							fp = _tfopen(szChoice, _T("wb"));
						}
					}

					if (fp) {
						TCHAR* szText;
						int nSize = SendDlgItemMessage(hDlg, IDC_SYSINFO_EDIT, WM_GETTEXTLENGTH, 0, 0);
						szText = (TCHAR*)malloc((nSize + 1) * sizeof(TCHAR));

#if defined (_UNICODE)
						// write a Unicode Byte Order Mark if needed
						if (ftell(fp) == 0) {
							WRITE_UNICODE_BOM(fp);
						}
#endif

						if (nSize && szText) {
							SendDlgItemMessage(hDlg, IDC_SYSINFO_EDIT, WM_GETTEXT, (WPARAM)(nSize + 1) * sizeof(TCHAR), (LPARAM)szText);
							fwrite(szText, sizeof(TCHAR), nSize, fp);
							_ftprintf(fp, _T(""));
						}
						if (szText) {
							free(szText);
							szText = NULL;
						}
						fclose(fp);
					}
				}
			}
		}
	}

	return 0;
}

LONG CALLBACK ExceptionFilter(_EXCEPTION_POINTERS* pExceptionInfo)
{
	int nRet;

	// If we're getting recursive calls to this function, bail out
	if (nRecursion++) {
		if (nRecursion <= 2) {
			MessageBox(hScrnWnd, _T(APP_TITLE) _T(" will now be terminated."), _T(APP_TITLE) _T(" Fatal exception"), MB_OK | MB_SETFOREGROUND);
			AppCleanup();
		}
#ifdef _DEBUG
		return EXCEPTION_CONTINUE_SEARCH;
#else
		return EXCEPTION_EXECUTE_HANDLER;
#endif
	}

	SplashDestroy(1);
	AudSoundStop();

	pExceptionPointers = pExceptionInfo;

	nRet = FBADialogBox(hAppInst, MAKEINTRESOURCE(IDD_EXCEPTION), hScrnWnd, (DLGPROC)SysInfoProc);

	switch (nRet) {
		case 1:
			return EXCEPTION_CONTINUE_SEARCH;
		default:
			AppCleanup();
			return EXCEPTION_EXECUTE_HANDLER;
	}
}

int SystemInfoCreate()
{
	pExceptionPointers = NULL;

	FBADialogBox(hAppInst, MAKEINTRESOURCE(IDD_SYSINFO), hScrnWnd, (DLGPROC)SysInfoProc);

	return 0;
}
