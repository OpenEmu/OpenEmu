// StringSet C++ class
#include "burner.h"

int __cdecl StringSet::Add(TCHAR* szFormat,...)
{
	TCHAR szAdd[256];
	int nAddLen = 0;
	TCHAR* NewMem;
	va_list Arg;

	va_start(Arg, szFormat);
	_vstprintf(szAdd, szFormat, Arg);

	nAddLen = _tcslen(szAdd);			// find out the length of the new text
	NewMem = (TCHAR*)realloc(szText, (nLen + nAddLen + 1) * sizeof(TCHAR));
	if (NewMem) {
		szText = NewMem;
		// copy the new text to the end
		_tcsncpy(szText + nLen, szAdd, nAddLen);
		nLen += nAddLen;
		szText[nLen] = 0;				// zero-terminate
	}

	va_end(Arg);

	return 0;
}

int StringSet::Reset()
{
	// Reset the text
	nLen = 0;
	szText= (TCHAR*)realloc(szText, sizeof(TCHAR));
	if (szText == NULL) {
		return 1;
	}
	szText[0] = 0;

	return 0;
}

StringSet::StringSet()
{
	szText = NULL;
	nLen = 0;
	Reset();							// reset string to nothing
}

StringSet::~StringSet()
{
	realloc(szText, 0);					// Free BZip text
}
