// Burner Config file module
#include "burner.h"
int nIniVersion = 0;

static void CreateConfigName(char* szConfig)
{
	memcpy(szConfig, "fbasdl.ini", 12);
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
		VAR(nVidSelect); // video mode select
		VAR(bBurnUseASMCPUEmulation); // if you have a poor mans PC

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
	
	fprintf(h,"\n// video mode 0 = standard SDL 1= (very expiermental) opengl\n");
	VAR(nVidSelect); // video mode select

	fprintf(h,"\n// use asm cpu cores (i.e. you need to buy a new PC)\n");
	VAR(bBurnUseASMCPUEmulation);

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
