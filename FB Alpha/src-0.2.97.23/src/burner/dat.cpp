// Burner data file module (for ROM managers)
// written    2001 LoqiqX
// updated 11/2003 by LvR -- essentially a rewrite

#include "burner.h"

static void ReplaceAmpersand(char *szBuffer, char *szGameName)
{
	UINT32 nStringPos = 0;
	
	for (UINT32 i = 0; i < strlen(szGameName); i++) {
		if (szGameName[i] == '&') {
			szBuffer[nStringPos + 0] = '&';
			szBuffer[nStringPos + 1] = 'a';
			szBuffer[nStringPos + 2] = 'm';
			szBuffer[nStringPos + 3] = 'p';
			szBuffer[nStringPos + 4] = ';';
			nStringPos += 5;
		} else {
			szBuffer[nStringPos] = szGameName[i];
			nStringPos++;
		}
	}
}

static void ReplaceLessThan(char *szBuffer, char *szGameName)
{
	UINT32 nStringPos = 0;
	
	for (UINT32 i = 0; i < strlen(szGameName); i++) {
		if (szGameName[i] == '<') {
			szBuffer[nStringPos + 0] = '&';
			szBuffer[nStringPos + 1] = 'l';
			szBuffer[nStringPos + 2] = 't';
			szBuffer[nStringPos + 3] = ';';
			nStringPos += 4;
		} else {
			szBuffer[nStringPos] = szGameName[i];
			nStringPos++;
		}
	}
}

static void ReplaceGreaterThan(char *szBuffer, char *szGameName)
{
	UINT32 nStringPos = 0;
	
	for (UINT32 i = 0; i < strlen(szGameName); i++) {
		if (szGameName[i] == '>') {
			szBuffer[nStringPos + 0] = '&';
			szBuffer[nStringPos + 1] = 'g';
			szBuffer[nStringPos + 2] = 't';
			szBuffer[nStringPos + 3] = ';';
			nStringPos += 4;
		} else {
			szBuffer[nStringPos] = szGameName[i];
			nStringPos++;
		}
	}
}

INT32 write_datfile(INT32 bType, FILE* fDat)
{
	INT32 nRet=0;
	UINT32 nOldSelect=0;
	UINT32 nGameSelect=0;
	UINT32 nParentSelect,nBoardROMSelect;
	
	fprintf(fDat, "<?xml version=\"1.0\"?>\n");
	fprintf(fDat, "<!DOCTYPE datafile PUBLIC \"-//FB Alpha//DTD ROM Management Datafile//EN\" \"http://www.logiqx.com/Dats/datafile.dtd\">\n\n");
	fprintf(fDat, "<datafile>\n");
	fprintf(fDat, "\t<header>\n");
	fprintf(fDat, "\t\t<name>" APP_TITLE "</name>\n");
	if (bType == DAT_ARCADE_ONLY) _ftprintf(fDat, _T("\t\t<description>") _T(APP_TITLE) _T(" v%s") _T(" Arcade Games</description>\n"), szAppBurnVer);
	if (bType == DAT_MEGADRIVE_ONLY) _ftprintf(fDat, _T("\t\t<description>") _T(APP_TITLE) _T(" v%s") _T(" Megadrive Games</description>\n"), szAppBurnVer);
	if (bType == DAT_PCENGINE_ONLY) _ftprintf(fDat, _T("\t\t<description>") _T(APP_TITLE) _T(" v%s") _T(" PC-Engine Games</description>\n"), szAppBurnVer);
	if (bType == DAT_TG16_ONLY) _ftprintf(fDat, _T("\t\t<description>") _T(APP_TITLE) _T(" v%s") _T(" TurboGrafx 16 Games</description>\n"), szAppBurnVer);
	if (bType == DAT_SGX_ONLY) _ftprintf(fDat, _T("\t\t<description>") _T(APP_TITLE) _T(" v%s") _T(" SuprGrafx Games</description>\n"), szAppBurnVer);
	fprintf(fDat, "\t\t<category>Standard DatFile</category>\n");
	_ftprintf(fDat, _T("\t\t<version>%s</version>\n"), szAppBurnVer);
	fprintf(fDat, "\t\t<author>" APP_TITLE "</author>\n");
	fprintf(fDat, "\t\t<homepage>http://www.barryharris.me.uk/</homepage>\n");
	fprintf(fDat, "\t\t<url>http://www.barryharris.me.uk/</url>\n");
	fprintf(fDat, "\t\t<clrmamepro forcenodump=\"ignore\"/>\n");		
	fprintf(fDat, "\t</header>\n");

	nOldSelect=nBurnDrvActive;										// preserve the currently selected driver

	// Go over each of the games
	for (nGameSelect=0;nGameSelect<nBurnDrvCount;nGameSelect++)
	{
		char sgName[32];
		char spName[32];
		char sbName[32];
		char ssName[32];
		UINT32 i=0;
		INT32 nPass=0;

		nBurnDrvActive=nGameSelect;									// Switch to driver nGameSelect

		if ((BurnDrvGetFlags() & BDF_BOARDROM) || !strcmp(BurnDrvGetTextA(DRV_NAME), "neogeo")) {
			continue;
		}
		
		if ((((BurnDrvGetHardwareCode() & HARDWARE_PUBLIC_MASK) == HARDWARE_SEGA_MEGADRIVE)
			|| ((BurnDrvGetHardwareCode() & HARDWARE_PUBLIC_MASK) == HARDWARE_PCENGINE_PCENGINE)
			|| ((BurnDrvGetHardwareCode() & HARDWARE_PUBLIC_MASK) == HARDWARE_PCENGINE_TG16)
			|| ((BurnDrvGetHardwareCode() & HARDWARE_PUBLIC_MASK) == HARDWARE_PCENGINE_SGX)
			) && (bType == DAT_ARCADE_ONLY)) {
			continue;
		}
		
		if (((BurnDrvGetHardwareCode() & HARDWARE_PUBLIC_MASK) != HARDWARE_SEGA_MEGADRIVE) && (bType == DAT_MEGADRIVE_ONLY)) {
			continue;
		}
		
		if (((BurnDrvGetHardwareCode() & HARDWARE_PUBLIC_MASK) != HARDWARE_PCENGINE_PCENGINE) && (bType == DAT_PCENGINE_ONLY)) {
			continue;
		}
		
		if (((BurnDrvGetHardwareCode() & HARDWARE_PUBLIC_MASK) != HARDWARE_PCENGINE_TG16) && (bType == DAT_TG16_ONLY)) {
			continue;
		}
		
		if (((BurnDrvGetHardwareCode() & HARDWARE_PUBLIC_MASK) != HARDWARE_PCENGINE_SGX) && (bType == DAT_SGX_ONLY)) {
			continue;
		}
		
		strcpy(sgName, BurnDrvGetTextA(DRV_NAME));
		strcpy(spName, "");											// make sure this string is empty before we start
		strcpy(sbName, "");											// make sure this string is empty before we start
		strcpy(ssName, "");											// make sure this string is empty before we start

		// Check to see if the game has a parent
		if (BurnDrvGetTextA(DRV_PARENT))
		{
			nParentSelect=-1U;
			while (BurnDrvGetTextA(DRV_PARENT))
			{
				strcpy(spName, BurnDrvGetTextA(DRV_PARENT));
				for (i=0;i<nBurnDrvCount;i++)
				{
					nBurnDrvActive=i;
					if (!strcmp(spName, BurnDrvGetTextA(DRV_NAME)))
					{
						nParentSelect=i;
						break;
					}
				}
			}

			nBurnDrvActive=nGameSelect;								// restore driver select
		}
		else
			nParentSelect=nGameSelect;

		// Check to see if the game has a BoardROM
		if (BurnDrvGetTextA(DRV_BOARDROM))
		{
			nBoardROMSelect=-1U;
			strcpy(sbName, BurnDrvGetTextA(DRV_BOARDROM));
			for (i=0;i<nBurnDrvCount;i++)
			{
				nBurnDrvActive=i;
				if (!strcmp(sbName, BurnDrvGetTextA(DRV_NAME)))
				{
					nBoardROMSelect=i;
					break;
				}
			}

			nBurnDrvActive=nGameSelect;								// restore driver select
		}
		else
			nBoardROMSelect=nGameSelect;
			
		if (BurnDrvGetTextA(DRV_SAMPLENAME)) {
			strcpy(ssName, BurnDrvGetTextA(DRV_SAMPLENAME));
		}
		
		if (((BurnDrvGetHardwareCode() & HARDWARE_PUBLIC_MASK) == HARDWARE_SEGA_MEGADRIVE)
			|| ((BurnDrvGetHardwareCode() & HARDWARE_PUBLIC_MASK) == HARDWARE_PCENGINE_TG16)
			) {
			// remove the md_ or tg_
			char Temp[35];
			INT32 Length;
			if (sgName[0]) {
				Length = strlen(sgName);
				memset(Temp, 0, 35);
				strcpy(Temp, sgName);
				memset(sgName, 0, 32);
				for (INT32 pos = 0; pos < Length; pos++) {
					sgName[pos] = Temp[pos + 3];
				}
			}
			if (spName[0]) {
				Length = strlen(spName);
				memset(Temp, 0, 35);
				strcpy(Temp, spName);
				memset(spName, 0, 32);
				for (INT32 pos = 0; pos < Length; pos++) {
					spName[pos] = Temp[pos + 3];
				}
			}
		}
		
		if (((BurnDrvGetHardwareCode() & HARDWARE_PUBLIC_MASK) == HARDWARE_PCENGINE_PCENGINE)
			|| ((BurnDrvGetHardwareCode() & HARDWARE_PUBLIC_MASK) == HARDWARE_PCENGINE_SGX)
			) {
			// remove the pce__ or sgx__
			char Temp[36];
			INT32 Length;
			if (sgName[0]) {
				Length = strlen(sgName);
				memset(Temp, 0, 36);
				strcpy(Temp, sgName);
				memset(sgName, 0, 32);
				for (INT32 pos = 0; pos < Length; pos++) {
					sgName[pos] = Temp[pos + 4];
				}
			}
			if (spName[0]) {
				Length = strlen(spName);
				memset(Temp, 0, 36);
				strcpy(Temp, spName);
				memset(spName, 0, 32);
				for (INT32 pos = 0; pos < Length; pos++) {
					spName[pos] = Temp[pos + 4];
				}
			}
		}

		// Report problems
		if (nParentSelect==-1U)
			fprintf(fDat, "# Missing parent %s. It needs to be added to " APP_TITLE "!\n\n", spName);
		if (nBoardROMSelect==-1U)
			fprintf(fDat, "# Missing boardROM %s. It needs to be added to " APP_TITLE "!\n\n", sbName);

		// Write the header
		if (nParentSelect!=nGameSelect && nParentSelect!=-1U)
		{
			if (!strcmp(ssName, "") || !strcmp(ssName, sgName)) {
				fprintf(fDat, "\t<game name=\"%s\" cloneof=\"%s\" romof=\"%s\">\n", sgName, spName, spName);
			} else {
				fprintf(fDat, "\t<game name=\"%s\" cloneof=\"%s\" romof=\"%s\" sampleof=\"%s\">\n", sgName, spName, spName, ssName);
			}
		}
		else
		{
			// Add "romof" (but not 'cloneof') line for games that have boardROMs
			if (nBoardROMSelect!=nGameSelect && nBoardROMSelect!=-1U)
			{
				fprintf(fDat, "\t<game name=\"%s\" romof=\"%s\">\n", sgName, sbName);
			} else {
				if (!strcmp(ssName, "") || !strcmp(ssName, sgName)) {
					fprintf(fDat, "\t<game name=\"%s\">\n", sgName);
				} else {
					fprintf(fDat, "\t<game name=\"%s\" sampleof=\"%s\">\n", sgName, ssName);
				}
			}
		}
		
		char szGameName[255];
		char szGameNameBuffer[255];
		char szManufacturer[255];
		char szManufacturerBuffer[255];
			
		memset(szGameName, 0, 255);
		memset(szGameNameBuffer, 0, 255);
		memset(szManufacturer, 0, 255);
		memset(szManufacturerBuffer, 0, 255);
		
		strcpy(szGameName, DecorateGameName(nBurnDrvActive));
		ReplaceAmpersand(szGameNameBuffer, szGameName);
		memset(szGameName, 0, 255);
		strcpy(szGameName, szGameNameBuffer);
		memset(szGameNameBuffer, 0, 255);
		ReplaceLessThan(szGameNameBuffer, szGameName);
		memset(szGameName, 0, 255);
		strcpy(szGameName, szGameNameBuffer);
		memset(szGameNameBuffer, 0, 255);
		ReplaceGreaterThan(szGameNameBuffer, szGameName);		
		
		strcpy(szManufacturer, BurnDrvGetTextA(DRV_MANUFACTURER));
		ReplaceAmpersand(szManufacturerBuffer, szManufacturer);
		memset(szManufacturer, 0, 255);
		strcpy(szManufacturer, szManufacturerBuffer);
		memset(szManufacturerBuffer, 0, 255);
		ReplaceLessThan(szManufacturerBuffer, szManufacturer);
		memset(szManufacturer, 0, 255);
		strcpy(szManufacturer, szManufacturerBuffer);
		memset(szManufacturerBuffer, 0, 255);
		ReplaceGreaterThan(szManufacturerBuffer, szManufacturer);
			
		fprintf(fDat, "\t\t<description>%s</description>\n", szGameNameBuffer);
		fprintf(fDat, "\t\t<year>%s</year>\n", BurnDrvGetTextA(DRV_DATE));
		fprintf(fDat, "\t\t<manufacturer>%s</manufacturer>\n", szManufacturerBuffer);
		
		// Write the individual ROM info
		for (nPass=0; nPass<2; nPass++)
		{
			nBurnDrvActive=nGameSelect;

			// Skip pass 0 if possible (pass 0 only needed for old-style clrMAME format)
			if (nPass==0 /*&& (nBoardROMSelect==nGameSelect || nBoardROMSelect==-1U)*/)
				continue;

			// Go over each of the files needed for this game (upto 0x0100)
			for (i=0, nRet=0; nRet==0 && i<0x100; i++)
			{
				INT32 nRetTmp=0;
				struct BurnRomInfo ri;
				INT32 nLen; UINT32 nCrc;
				char *szPossibleName=NULL;
				INT32 j, nMerged=0;

				memset(&ri,0,sizeof(ri));

				// Get info on this file
				nBurnDrvActive=nGameSelect;
				nRet=BurnDrvGetRomInfo(&ri,i);
				nRet+=BurnDrvGetRomName(&szPossibleName,i,0);

				if (ri.nLen==0) continue;

				if (nRet==0)
				{
					struct BurnRomInfo riTmp;
					char *szPossibleNameTmp;
					nLen=ri.nLen; nCrc=ri.nCrc;

					// Check for files from boardROMs
					if (nBoardROMSelect!=nGameSelect && nBoardROMSelect!=-1U) {
						nBurnDrvActive=nBoardROMSelect;
						nRetTmp=0;

						// Go over each of the files needed for this game (upto 0x0100)
						for (j=0; nRetTmp==0 && j<0x100; j++)
						{
							memset(&riTmp,0,sizeof(riTmp));

							nRetTmp+=BurnDrvGetRomInfo(&riTmp,j);
							nRetTmp+=BurnDrvGetRomName(&szPossibleNameTmp,j,0);

							if (nRetTmp==0)
							{
								if (riTmp.nLen && riTmp.nCrc==nCrc && !strcmp(szPossibleName, szPossibleNameTmp))
								{
									// This file is from a boardROM
									nMerged|=2;
									nRetTmp++;
								}
							}
						}
					}

					if (!nMerged && nParentSelect!=nGameSelect && nParentSelect!=-1U) {
						nBurnDrvActive=nParentSelect;
						nRetTmp=0;

						// Go over each of the files needed for this game (upto 0x0100)
						for (j=0; nRetTmp==0 && j<0x100; j++)
						{
							memset(&riTmp,0,sizeof(riTmp));

							nRetTmp+=BurnDrvGetRomInfo(&riTmp,j);
							nRetTmp+=BurnDrvGetRomName(&szPossibleNameTmp,j,0);

							if (nRetTmp==0)
							{
								if (riTmp.nLen && riTmp.nCrc==nCrc && !strcmp(szPossibleName, szPossibleNameTmp))
								{
									// This file is from a parent set
									nMerged|=1;
									nRetTmp++;
								}
							}
						}
					}

					nBurnDrvActive=nGameSelect;						// Switch back to game
				}

				char szPossibleNameBuffer[255];
				char szPossibleNameBuffer2[255];
			
				memset(szPossibleNameBuffer, 0, 255);
				memset(szPossibleNameBuffer2, 0, 255);
			
				ReplaceAmpersand(szPossibleNameBuffer, szPossibleName);
				strcpy(szPossibleNameBuffer2, szPossibleNameBuffer);
				memset(szPossibleNameBuffer, 0, 255);
				ReplaceLessThan(szPossibleNameBuffer, szPossibleNameBuffer2);
				memset(szPossibleNameBuffer2, 0, 255);
				strcpy(szPossibleNameBuffer2, szPossibleNameBuffer);
				memset(szPossibleNameBuffer, 0, 255);
				ReplaceGreaterThan(szPossibleNameBuffer, szPossibleNameBuffer2);
					
				// File info
				if (nPass==1 && !nMerged) {
					if (ri.nType & BRF_NODUMP) {
						fprintf(fDat, "\t\t<rom name=\"%s\" size=\"%d\" status=\"nodump\"/>\n", szPossibleNameBuffer, ri.nLen);
					} else {
						fprintf(fDat, "\t\t<rom name=\"%s\" size=\"%d\" crc=\"%08x\"/>\n", szPossibleNameBuffer, ri.nLen, ri.nCrc);
					}
				}
				if (nPass==1 && nMerged)
				{
					// Files from parent/boardROMs
					if (ri.nType & BRF_NODUMP) {
						fprintf(fDat, "\t\t<rom name=\"%s\" merge=\"%s\" size=\"%d\" status=\"nodump\"/>\n", szPossibleNameBuffer, szPossibleNameBuffer, ri.nLen);
					} else {
						fprintf(fDat, "\t\t<rom name=\"%s\" merge=\"%s\" size=\"%d\" crc=\"%08x\"/>\n", szPossibleNameBuffer, szPossibleNameBuffer, ri.nLen, ri.nCrc);
					}
				}
			}
			
			// samples
			if (strcmp(ssName, "")) {
				for (i=0, nRet=0; nRet==0 && i<0x100; i++)
				{
					struct BurnSampleInfo si;
					char *szPossibleName=NULL;

					memset(&si,0,sizeof(si));

					// Get info on this file
					nBurnDrvActive=nGameSelect;
					nRet=BurnDrvGetSampleInfo(&si,i);
					nRet+=BurnDrvGetSampleName(&szPossibleName,i,0);

					if (si.nFlags==0) continue;

					if (nPass == 1) {
						char szPossibleNameBuffer[255];
						char szPossibleNameBuffer2[255];
			
						memset(szPossibleNameBuffer, 0, 255);
						memset(szPossibleNameBuffer2, 0, 255);
			
						ReplaceAmpersand(szPossibleNameBuffer, szPossibleName);
						strcpy(szPossibleNameBuffer2, szPossibleNameBuffer);
						memset(szPossibleNameBuffer, 0, 255);
						ReplaceLessThan(szPossibleNameBuffer, szPossibleNameBuffer2);
						memset(szPossibleNameBuffer2, 0, 255);
						strcpy(szPossibleNameBuffer2, szPossibleNameBuffer);
						memset(szPossibleNameBuffer, 0, 255);
						ReplaceGreaterThan(szPossibleNameBuffer, szPossibleNameBuffer2);
					
						fprintf(fDat, "\t\t<sample name=\"%s\" />\n", szPossibleNameBuffer);
					}
				}
			}
		}

		fprintf(fDat, "\t</game>\n");
	}

	// Do another pass over each of the games to find boardROMs
	for (nBurnDrvActive=0; nBurnDrvActive<nBurnDrvCount; nBurnDrvActive++)
	{
		INT32 i, nPass;

		if (!(BurnDrvGetFlags() & BDF_BOARDROM)) {
			continue;
		}
		
		if (((BurnDrvGetHardwareCode() & HARDWARE_PUBLIC_MASK) != HARDWARE_SEGA_MEGADRIVE) && (bType == DAT_MEGADRIVE_ONLY)) {
			continue;
		}
		
		if (((BurnDrvGetHardwareCode() & HARDWARE_PUBLIC_MASK) != HARDWARE_PCENGINE_PCENGINE) && (bType == DAT_PCENGINE_ONLY)) {
			continue;
		}
		
		if (((BurnDrvGetHardwareCode() & HARDWARE_PUBLIC_MASK) != HARDWARE_PCENGINE_TG16) && (bType == DAT_TG16_ONLY)) {
			continue;
		}
		
		if (((BurnDrvGetHardwareCode() & HARDWARE_PUBLIC_MASK) != HARDWARE_PCENGINE_SGX) && (bType == DAT_SGX_ONLY)) {
			continue;
		}

		fprintf(fDat, "\t<game isbios=\"yes\" name=\"%s\">\n", BurnDrvGetTextA(DRV_NAME));
		fprintf(fDat, "\t\t<description>%s</description>\n", DecorateGameName(nBurnDrvActive));
		fprintf(fDat, "\t\t<year>%s</year>\n", BurnDrvGetTextA(DRV_DATE));
		fprintf(fDat, "\t\t<manufacturer>%s</manufacturer>\n", BurnDrvGetTextA(DRV_MANUFACTURER));		

		for (nPass=0; nPass<2; nPass++)
		{
			// No meta information needed (pass 0 only needed for old-style clrMAME format)
			if (nPass==0) continue;

			// Go over each of the individual files (upto 0x0100)
			for (i=0; i<0x100; i++)
			{
				struct BurnRomInfo ri;
				char *szPossibleName=NULL;

				memset(&ri,0,sizeof(ri));

				nRet=BurnDrvGetRomInfo(&ri,i);
				nRet+=BurnDrvGetRomName(&szPossibleName,i,0);

				if (ri.nLen==0) continue;

				if (nRet==0) {
					char szPossibleNameBuffer[255];
					char szPossibleNameBuffer2[255];
			
					memset(szPossibleNameBuffer, 0, 255);
					memset(szPossibleNameBuffer2, 0, 255);
			
					ReplaceAmpersand(szPossibleNameBuffer, szPossibleName);
					strcpy(szPossibleNameBuffer2, szPossibleNameBuffer);
					memset(szPossibleNameBuffer, 0, 255);
					ReplaceLessThan(szPossibleNameBuffer, szPossibleNameBuffer2);
					memset(szPossibleNameBuffer2, 0, 255);
					strcpy(szPossibleNameBuffer2, szPossibleNameBuffer);
					memset(szPossibleNameBuffer, 0, 255);
					ReplaceGreaterThan(szPossibleNameBuffer, szPossibleNameBuffer2);
				
					fprintf(fDat, "\t\t<rom name=\"%s\" size=\"%d\" crc=\"%08x\"/>\n", szPossibleNameBuffer, ri.nLen, ri.nCrc);
				}
			}
		}

		fprintf(fDat, "\t</game>\n");
	}

	// Restore current driver
	nBurnDrvActive=nOldSelect;
	
	fprintf(fDat, "</datafile>");

	return 0;
}

INT32 create_datfile(TCHAR* szFilename, INT32 bType)
{
	FILE *fDat=0;
	INT32 nRet=0;
	
	if ((fDat = _tfopen(szFilename, _T("wt")))==0)
		return -1;

	nRet =  write_datfile(bType, fDat);

	fclose(fDat);

	return nRet;
}
