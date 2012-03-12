// Burner Config file module
#include "burner.h"
int nIniVersion = 0;

static void CreateConfigName(char* szConfig)
{
	memcpy(szConfig, "fba.ini", 12);
	return;
}

// Read in the config file for the whole application
int ConfigAppLoad()
{
	char szConfig[MAX_PATH];
	char szLine[256];
	FILE *h;

	CreateConfigName(szConfig);

	if ((h = fopen(szConfig,"rt")) == NULL) {
		return 1;
	}

	// Go through each line of the config file
	while (fgets(szLine, sizeof(szLine), h)) {
		int nLen;

		nLen = strlen(szLine);

		// Get rid of the linefeed at the end
		if (szLine[nLen - 1] == 10) {
			szLine[nLen - 1] = 0;
			nLen--;
		}

#define VAR(x) { char *szValue = LabelCheck(szLine,#x);		\
  if (szValue) x = strtol(szValue, NULL, 0); }
#define FLT(x) { char *szValue = LabelCheck(szLine,#x);		\
  if (szValue) x = atof(szValue); }
#define STR(x) { char *szValue = LabelCheck(szLine,#x " ");	\
  if (szValue) strcpy(x,szValue); }

		VAR(nIniVersion);


		// Other
		STR(szAppRomPaths[0]);
		STR(szAppRomPaths[1]);
		STR(szAppRomPaths[2]);
		STR(szAppRomPaths[3]);
		STR(szAppRomPaths[4]);
		STR(szAppRomPaths[5]);
		STR(szAppRomPaths[6]);
		STR(szAppRomPaths[7]);
#undef STR
#undef FLT
#undef VAR
	}

	fclose(h);
	return 0;
}

// Write out the config file for the whole application
int ConfigAppSave()
{
	char szConfig[MAX_PATH];
	FILE *h;

	CreateConfigName(szConfig);

	if ((h = fopen(szConfig, "wt")) == NULL) {
		return 1;
	}


#define VAR(x) fprintf(h, #x " %d\n", x)
#define FLT(x) fprintf(h, #x " %f\n", x)
#define STR(x) fprintf(h, #x " %s\n", x)

	fprintf(h,"\n// The application version this file was saved from\n");
	// We can't use the macros for this!
	fprintf(h, "nIniVersion 0x%06X", nBurnVer);

	fprintf(h,"\n// The paths to search for rom zips. (include trailing backslash)\n");
	STR(szAppRomPaths[0]);
	STR(szAppRomPaths[1]);
	STR(szAppRomPaths[2]);
	STR(szAppRomPaths[3]);
	STR(szAppRomPaths[4]);
	STR(szAppRomPaths[5]);
	STR(szAppRomPaths[6]);
	STR(szAppRomPaths[7]);

	fprintf(h,"\n\n\n");

#undef STR
#undef FLT
#undef VAR

	fclose(h);
	return 0;
}

char* LabelCheck(char *s,char *szLabel)
{
	int nLen;
	if (s == NULL) {
		return NULL;
	}
	if (szLabel == NULL) {
		return NULL;
	}
	nLen = strlen(szLabel);

	SKIP_WS(s);													// Skip whitespace

	if (strncmp(s, szLabel, nLen)){								// Doesn't match
		return NULL;
	}
	return s + nLen;
}

int QuoteRead(char** pszQuote, char** pszEnd, char* szSrc)		// Read a quoted string from szSrc and point to the end
{
	static char szQuote[QUOTE_MAX];
	char *s = szSrc;
	int i;

	SKIP_WS(s);													// Skip whitespace

	if (*s != '\"')	{											// Didn't start with a quote
		return 1;
	}
	s++;

	// Copy the characters into szQuote
	for (i = 0; i < QUOTE_MAX - 1; i++, s++) {
		if (*s == '\"') {
			s++;
			break;
		}
		if (*s == 0) {											// Stop copying if end of line or end of string
			break;
		}
		szQuote[i] = *s;										// Copy character to szQuote
	}
	szQuote[i] = 0;												// Zero-terminate szQuote

	if (pszQuote) {
		*pszQuote = szQuote;
	}
	if (pszEnd)	{
		*pszEnd = s;
	}

	return 0;
}

