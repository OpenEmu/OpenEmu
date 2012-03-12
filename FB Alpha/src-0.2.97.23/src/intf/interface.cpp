#include "burner.h"

#define INT_INFO_STRINGS (8)

INT32 IntInfoFree(InterfaceInfo* pInfo)
{
	if (pInfo->ppszInterfaceSettings) {
		for (INT32 i = 0; i < INT_INFO_STRINGS; i++) {
			if (pInfo->ppszInterfaceSettings[i]) {
				free(pInfo->ppszInterfaceSettings[i]);
				pInfo->ppszInterfaceSettings[i] = NULL;
			}
		}
	}
	if (pInfo->ppszInterfaceSettings) {
		free(pInfo->ppszInterfaceSettings);
		pInfo->ppszInterfaceSettings = NULL;
	}

	if (pInfo->ppszModuleSettings) {
		for (INT32 i = 0; i < INT_INFO_STRINGS; i++) {
			if (pInfo->ppszModuleSettings[i]) {
				free(pInfo->ppszModuleSettings[i]);
				pInfo->ppszModuleSettings[i] = NULL;
			}
		}
	}
	if (pInfo->ppszModuleSettings) {
		free(pInfo->ppszModuleSettings);
		pInfo->ppszModuleSettings = NULL;
	}

	memset(pInfo, 0, sizeof(InterfaceInfo));

	return 0;
}

INT32 IntInfoInit(InterfaceInfo* pInfo)
{
	IntInfoFree(pInfo);

	pInfo->ppszInterfaceSettings = (TCHAR**)malloc((INT_INFO_STRINGS + 1) * sizeof(TCHAR*));
	if (pInfo->ppszInterfaceSettings == NULL) {
		return 1;
	}
	memset(pInfo->ppszInterfaceSettings, 0, (INT_INFO_STRINGS + 1) * sizeof(TCHAR*));

	pInfo->ppszModuleSettings = (TCHAR**)malloc((INT_INFO_STRINGS + 1) * sizeof(TCHAR*));
	if (pInfo->ppszModuleSettings == NULL) {
		return 1;
	}
	memset(pInfo->ppszModuleSettings, 0, (INT_INFO_STRINGS + 1) * sizeof(TCHAR*));

	return 0;
}

INT32 IntInfoAddStringInterface(InterfaceInfo* pInfo, TCHAR* szString)
{
	INT32 i;

	for (i = 0; pInfo->ppszInterfaceSettings[i] && i < INT_INFO_STRINGS; i++) { }

	if (i >= INT_INFO_STRINGS) {
		return 1;
	}

	pInfo->ppszInterfaceSettings[i] = (TCHAR*)malloc(MAX_PATH * sizeof(TCHAR));
	if (pInfo->ppszInterfaceSettings[i] == NULL) {
		return 1;
	}

	_tcsncpy(pInfo->ppszInterfaceSettings[i], szString, MAX_PATH);

	return 0;
}

INT32 IntInfoAddStringModule(InterfaceInfo* pInfo, TCHAR* szString)
{
	INT32 i;

	for (i = 0; pInfo->ppszModuleSettings[i] && i < INT_INFO_STRINGS; i++) { }

	if (i >= INT_INFO_STRINGS) {
		return 1;
	}

	pInfo->ppszModuleSettings[i] = (TCHAR*)malloc(MAX_PATH * sizeof(TCHAR));
	if (pInfo->ppszModuleSettings[i] == NULL) {
		return 1;
	}

	_tcsncpy(pInfo->ppszModuleSettings[i], szString, MAX_PATH);

	return 0;
}

