// Burner Game Input
#include "burner.h"

// Player Default Controls
INT32 nPlayerDefaultControls[4] = {0, 1, 2, 3};
TCHAR szPlayerDefaultIni[4][MAX_PATH] = { _T(""), _T(""), _T(""), _T("") };

// Mapping of PC inputs to game inputs
struct GameInp* GameInp = NULL;
UINT32 nGameInpCount = 0;
UINT32 nMacroCount = 0;
UINT32 nMaxMacro = 0;

INT32 nAnalogSpeed;

INT32 nFireButtons = 0;

bool bStreetFighterLayout = false;
bool bLeftAltkeyMapped = false;

// ---------------------------------------------------------------------------

// Check if the left alt (menu) key is mapped
void GameInpCheckLeftAlt()
{
	struct GameInp* pgi;
	UINT32 i;

	bLeftAltkeyMapped = false;

	for (i = 0, pgi = GameInp; i < (nGameInpCount + nMacroCount); i++, pgi++) {

		if (bLeftAltkeyMapped) {
			break;
		}

		switch (pgi->nInput) {
			case GIT_SWITCH:
				if (pgi->Input.Switch.nCode == FBK_LALT) {
					bLeftAltkeyMapped = true;
				}
				break;
			case GIT_MACRO_AUTO:
			case GIT_MACRO_CUSTOM:
				if (pgi->Macro.nMode) {
					if (pgi->Macro.Switch.nCode == FBK_LALT) {
						bLeftAltkeyMapped = true;
					}
				}
				break;

			default:
				continue;
		}
	}
}

// Check if the sytem mouse is mapped and set the cooperative level apropriately
void GameInpCheckMouse()
{
	bool bMouseMapped = false;
	struct GameInp* pgi;
	UINT32 i;

	for (i = 0, pgi = GameInp; i < (nGameInpCount + nMacroCount); i++, pgi++) {

		if (bMouseMapped) {
			break;
		}

		switch (pgi->nInput) {
			case GIT_SWITCH:
				if ((pgi->Input.Switch.nCode & 0xFF00) == 0x8000) {
					bMouseMapped = true;
				}
				break;
			case GIT_MOUSEAXIS:
				if (pgi->Input.MouseAxis.nMouse == 0) {
					bMouseMapped = true;
				}
				break;
			case GIT_MACRO_AUTO:
			case GIT_MACRO_CUSTOM:
				if (pgi->Macro.nMode) {
					if ((pgi->Macro.Switch.nCode & 0xFF00) == 0x8000) {
						bMouseMapped = true;
					}
				}
				break;

			default:
				continue;
		}
	}

	if (bDrvOkay) {
		if (!bRunPause) {
			InputSetCooperativeLevel(bMouseMapped, bAlwaysProcessKeyboardInput);
		} else {
			InputSetCooperativeLevel(false, bAlwaysProcessKeyboardInput);
		}
	} else {
		InputSetCooperativeLevel(false, false);
	}
}

// ---------------------------------------------------------------------------

INT32 GameInpBlank(INT32 bDipSwitch)
{
	UINT32 i = 0;
	struct GameInp* pgi = NULL;

	// Reset all inputs to undefined (even dip switches, if bDipSwitch==1)
	if (GameInp == NULL) {
		return 1;
	}

	// Get the targets in the library for the Input Values
	for (i = 0, pgi = GameInp; i < nGameInpCount; i++, pgi++) {
		struct BurnInputInfo bii;
		memset(&bii, 0, sizeof(bii));
		BurnDrvGetInputInfo(&bii, i);
		if (bDipSwitch == 0 && (bii.nType & BIT_GROUP_CONSTANT)) {		// Don't blank the dip switches
			continue;
		}

		memset(pgi, 0, sizeof(*pgi));									// Clear input

		pgi->nType = bii.nType;											// store input type
		pgi->Input.pVal = bii.pVal;										// store input pointer to value

		if (bii.nType & BIT_GROUP_CONSTANT) {							// Further initialisation for constants/DIPs
			pgi->nInput = GIT_CONSTANT;
			pgi->Input.Constant.nConst = *bii.pVal;
		}
	}

	for (i = 0; i < nMacroCount; i++, pgi++) {
		pgi->Macro.nMode = 0;
		if (pgi->nInput == GIT_MACRO_CUSTOM) {
			pgi->nInput = 0;
		}
	}

	bLeftAltkeyMapped = false;

	return 0;
}

static void GameInpInitMacros()
{
	struct GameInp* pgi;
	struct BurnInputInfo bii;

	INT32 nPunchx3[4] = {0, 0, 0, 0};
	INT32 nPunchInputs[4][3];
	INT32 nKickx3[4] = {0, 0, 0, 0};
	INT32 nKickInputs[4][3];

	INT32 nNeogeoButtons[4][4];
	INT32 nPgmButtons[4][4];

	bStreetFighterLayout = false;
	nMacroCount = 0;

	nFireButtons = 0;

	for (UINT32 i = 0; i < nGameInpCount; i++) {
		bii.szName = NULL;
		BurnDrvGetInputInfo(&bii, i);
		if (bii.szName == NULL) {
			bii.szName = "";
		}
		if (bii.szName[0] == 'P' && bii.szName[1] >= '1' && bii.szName[1] <= '4') {
			INT32 nPlayer = bii.szName[1] - '1';

			if (nPlayer == 0) {
				if (strncmp(" fire", bii.szInfo + 2, 5) == 0) {
					nFireButtons++;
				}
			}

			if (_stricmp(" Weak Punch", bii.szName + 2) == 0) {
				nPunchx3[nPlayer] |= 1;
				nPunchInputs[nPlayer][0] = i;
			}
			if (_stricmp(" Medium Punch", bii.szName + 2) == 0) {
				nPunchx3[nPlayer] |= 2;
				nPunchInputs[nPlayer][1] = i;
			}
			if (_stricmp(" Strong Punch", bii.szName + 2) == 0) {
				nPunchx3[nPlayer] |= 4;
				nPunchInputs[nPlayer][2] = i;
			}
			if (_stricmp(" Weak Kick", bii.szName + 2) == 0) {
				nKickx3[nPlayer] |= 1;
				nKickInputs[nPlayer][0] = i;
			}
			if (_stricmp(" Medium Kick", bii.szName + 2) == 0) {
				nKickx3[nPlayer] |= 2;
				nKickInputs[nPlayer][1] = i;
			}
			if (_stricmp(" Strong Kick", bii.szName + 2) == 0) {
				nKickx3[nPlayer] |= 4;
				nKickInputs[nPlayer][2] = i;
			}
			
			if ((BurnDrvGetHardwareCode() & HARDWARE_PUBLIC_MASK) == HARDWARE_SNK_NEOGEO) {
				if (_stricmp(" Button A", bii.szName + 2) == 0) {
					nNeogeoButtons[nPlayer][0] = i;
				}
				if (_stricmp(" Button B", bii.szName + 2) == 0) {
					nNeogeoButtons[nPlayer][1] = i;
				}
				if (_stricmp(" Button C", bii.szName + 2) == 0) {
					nNeogeoButtons[nPlayer][2] = i;
				}
				if (_stricmp(" Button D", bii.szName + 2) == 0) {
					nNeogeoButtons[nPlayer][3] = i;
				}
			}
			
			if ((BurnDrvGetHardwareCode() & HARDWARE_PUBLIC_MASK) == HARDWARE_IGS_PGM) {
				if (_stricmp(" Button 1", bii.szName + 2) == 0) {
					nPgmButtons[nPlayer][0] = i;
				}
				if (_stricmp(" Button 2", bii.szName + 2) == 0) {
					nPgmButtons[nPlayer][1] = i;
				}
				if (_stricmp(" Button 3", bii.szName + 2) == 0) {
					nPgmButtons[nPlayer][2] = i;
				}
				if (_stricmp(" Button 4", bii.szName + 2) == 0) {
					nPgmButtons[nPlayer][3] = i;
				}
			}
		}
	}

	pgi = GameInp + nGameInpCount;

	for (INT32 nPlayer = 0; nPlayer < nMaxPlayers; nPlayer++) {
		if (nPunchx3[nPlayer] == 7) {		// Create a 3x punch macro
			pgi->nInput = GIT_MACRO_AUTO;
			pgi->nType = BIT_DIGITAL;
			pgi->Macro.nMode = 0;

			sprintf(pgi->Macro.szName, "P%i 3× Punch", nPlayer + 1);
			for (INT32 j = 0; j < 3; j++) {
				BurnDrvGetInputInfo(&bii, nPunchInputs[nPlayer][j]);
				pgi->Macro.pVal[j] = bii.pVal;
				pgi->Macro.nVal[j] = 1;
			}

			nMacroCount++;
			pgi++;
		}

		if (nKickx3[nPlayer] == 7) {		// Create a 3x kick macro
			pgi->nInput = GIT_MACRO_AUTO;
			pgi->nType = BIT_DIGITAL;
			pgi->Macro.nMode = 0;

			sprintf(pgi->Macro.szName, "P%i 3× Kick", nPlayer + 1);
			for (INT32 j = 0; j < 3; j++) {
				BurnDrvGetInputInfo(&bii, nKickInputs[nPlayer][j]);
				pgi->Macro.pVal[j] = bii.pVal;
				pgi->Macro.nVal[j] = 1;
			}

			nMacroCount++;
			pgi++;
		}

		if (nFireButtons == 4 && (BurnDrvGetHardwareCode() & HARDWARE_PUBLIC_MASK) == HARDWARE_SNK_NEOGEO) {
			pgi->nInput = GIT_MACRO_AUTO;
			pgi->nType = BIT_DIGITAL;
			pgi->Macro.nMode = 0;
			sprintf(pgi->Macro.szName, "P%i Buttons AB", nPlayer + 1);
			BurnDrvGetInputInfo(&bii, nNeogeoButtons[nPlayer][0]);
			pgi->Macro.pVal[0] = bii.pVal;
			pgi->Macro.nVal[0] = 1;
			BurnDrvGetInputInfo(&bii, nNeogeoButtons[nPlayer][1]);
			pgi->Macro.pVal[1] = bii.pVal;
			pgi->Macro.nVal[1] = 1;
			nMacroCount++;
			pgi++;
			
			pgi->nInput = GIT_MACRO_AUTO;
			pgi->nType = BIT_DIGITAL;
			pgi->Macro.nMode = 0;
			sprintf(pgi->Macro.szName, "P%i Buttons AC", nPlayer + 1);
			BurnDrvGetInputInfo(&bii, nNeogeoButtons[nPlayer][0]);
			pgi->Macro.pVal[0] = bii.pVal;
			pgi->Macro.nVal[0] = 1;
			BurnDrvGetInputInfo(&bii, nNeogeoButtons[nPlayer][2]);
			pgi->Macro.pVal[1] = bii.pVal;
			pgi->Macro.nVal[1] = 1;
			nMacroCount++;
			pgi++;
			
			pgi->nInput = GIT_MACRO_AUTO;
			pgi->nType = BIT_DIGITAL;
			pgi->Macro.nMode = 0;
			sprintf(pgi->Macro.szName, "P%i Buttons AD", nPlayer + 1);
			BurnDrvGetInputInfo(&bii, nNeogeoButtons[nPlayer][0]);
			pgi->Macro.pVal[0] = bii.pVal;
			pgi->Macro.nVal[0] = 1;
			BurnDrvGetInputInfo(&bii, nNeogeoButtons[nPlayer][3]);
			pgi->Macro.pVal[1] = bii.pVal;
			pgi->Macro.nVal[1] = 1;
			nMacroCount++;
			pgi++;
			
			pgi->nInput = GIT_MACRO_AUTO;
			pgi->nType = BIT_DIGITAL;
			pgi->Macro.nMode = 0;
			sprintf(pgi->Macro.szName, "P%i Buttons BC", nPlayer + 1);
			BurnDrvGetInputInfo(&bii, nNeogeoButtons[nPlayer][1]);
			pgi->Macro.pVal[0] = bii.pVal;
			pgi->Macro.nVal[0] = 1;
			BurnDrvGetInputInfo(&bii, nNeogeoButtons[nPlayer][2]);
			pgi->Macro.pVal[1] = bii.pVal;
			pgi->Macro.nVal[1] = 1;
			nMacroCount++;
			pgi++;
			
			pgi->nInput = GIT_MACRO_AUTO;
			pgi->nType = BIT_DIGITAL;
			pgi->Macro.nMode = 0;
			sprintf(pgi->Macro.szName, "P%i Buttons BD", nPlayer + 1);
			BurnDrvGetInputInfo(&bii, nNeogeoButtons[nPlayer][1]);
			pgi->Macro.pVal[0] = bii.pVal;
			pgi->Macro.nVal[0] = 1;
			BurnDrvGetInputInfo(&bii, nNeogeoButtons[nPlayer][3]);
			pgi->Macro.pVal[1] = bii.pVal;
			pgi->Macro.nVal[1] = 1;
			nMacroCount++;
			pgi++;
			
			pgi->nInput = GIT_MACRO_AUTO;
			pgi->nType = BIT_DIGITAL;
			pgi->Macro.nMode = 0;
			sprintf(pgi->Macro.szName, "P%i Buttons CD", nPlayer + 1);
			BurnDrvGetInputInfo(&bii, nNeogeoButtons[nPlayer][2]);
			pgi->Macro.pVal[0] = bii.pVal;
			pgi->Macro.nVal[0] = 1;
			BurnDrvGetInputInfo(&bii, nNeogeoButtons[nPlayer][3]);
			pgi->Macro.pVal[1] = bii.pVal;
			pgi->Macro.nVal[1] = 1;
			nMacroCount++;
			pgi++;
			
			pgi->nInput = GIT_MACRO_AUTO;
			pgi->nType = BIT_DIGITAL;
			pgi->Macro.nMode = 0;
			sprintf(pgi->Macro.szName, "P%i Buttons ABC", nPlayer + 1);
			BurnDrvGetInputInfo(&bii, nNeogeoButtons[nPlayer][0]);
			pgi->Macro.pVal[0] = bii.pVal;
			pgi->Macro.nVal[0] = 1;
			BurnDrvGetInputInfo(&bii, nNeogeoButtons[nPlayer][1]);
			pgi->Macro.pVal[1] = bii.pVal;
			pgi->Macro.nVal[1] = 1;
			BurnDrvGetInputInfo(&bii, nNeogeoButtons[nPlayer][2]);
			pgi->Macro.pVal[2] = bii.pVal;
			pgi->Macro.nVal[2] = 1;
			nMacroCount++;
			pgi++;
			
			pgi->nInput = GIT_MACRO_AUTO;
			pgi->nType = BIT_DIGITAL;
			pgi->Macro.nMode = 0;
			sprintf(pgi->Macro.szName, "P%i Buttons ABD", nPlayer + 1);
			BurnDrvGetInputInfo(&bii, nNeogeoButtons[nPlayer][0]);
			pgi->Macro.pVal[0] = bii.pVal;
			pgi->Macro.nVal[0] = 1;
			BurnDrvGetInputInfo(&bii, nNeogeoButtons[nPlayer][1]);
			pgi->Macro.pVal[1] = bii.pVal;
			pgi->Macro.nVal[1] = 1;
			BurnDrvGetInputInfo(&bii, nNeogeoButtons[nPlayer][3]);
			pgi->Macro.pVal[2] = bii.pVal;
			pgi->Macro.nVal[2] = 1;
			nMacroCount++;
			pgi++;
			
			pgi->nInput = GIT_MACRO_AUTO;
			pgi->nType = BIT_DIGITAL;
			pgi->Macro.nMode = 0;
			sprintf(pgi->Macro.szName, "P%i Buttons ACD", nPlayer + 1);
			BurnDrvGetInputInfo(&bii, nNeogeoButtons[nPlayer][0]);
			pgi->Macro.pVal[0] = bii.pVal;
			pgi->Macro.nVal[0] = 1;
			BurnDrvGetInputInfo(&bii, nNeogeoButtons[nPlayer][2]);
			pgi->Macro.pVal[1] = bii.pVal;
			pgi->Macro.nVal[1] = 1;
			BurnDrvGetInputInfo(&bii, nNeogeoButtons[nPlayer][3]);
			pgi->Macro.pVal[2] = bii.pVal;
			pgi->Macro.nVal[2] = 1;
			nMacroCount++;
			pgi++;

			pgi->nInput = GIT_MACRO_AUTO;
			pgi->nType = BIT_DIGITAL;
			pgi->Macro.nMode = 0;
			sprintf(pgi->Macro.szName, "P%i Buttons BCD", nPlayer + 1);
			BurnDrvGetInputInfo(&bii, nNeogeoButtons[nPlayer][1]);
			pgi->Macro.pVal[0] = bii.pVal;
			pgi->Macro.nVal[0] = 1;
			BurnDrvGetInputInfo(&bii, nNeogeoButtons[nPlayer][2]);
			pgi->Macro.pVal[1] = bii.pVal;
			pgi->Macro.nVal[1] = 1;
			BurnDrvGetInputInfo(&bii, nNeogeoButtons[nPlayer][3]);
			pgi->Macro.pVal[2] = bii.pVal;
			pgi->Macro.nVal[2] = 1;
			nMacroCount++;
			pgi++;

			pgi->nInput = GIT_MACRO_AUTO;
			pgi->nType = BIT_DIGITAL;
			pgi->Macro.nMode = 0;
			sprintf(pgi->Macro.szName, "P%i Buttons ABCD", nPlayer + 1);
			BurnDrvGetInputInfo(&bii, nNeogeoButtons[nPlayer][0]);
			pgi->Macro.pVal[0] = bii.pVal;
			pgi->Macro.nVal[0] = 1;
			BurnDrvGetInputInfo(&bii, nNeogeoButtons[nPlayer][1]);
			pgi->Macro.pVal[1] = bii.pVal;
			pgi->Macro.nVal[1] = 1;
			BurnDrvGetInputInfo(&bii, nNeogeoButtons[nPlayer][2]);
			pgi->Macro.pVal[2] = bii.pVal;
			pgi->Macro.nVal[2] = 1;
			BurnDrvGetInputInfo(&bii, nNeogeoButtons[nPlayer][3]);
			pgi->Macro.pVal[3] = bii.pVal;
			pgi->Macro.nVal[3] = 1;
			nMacroCount++;
			pgi++;
		}
		
		if (nFireButtons == 4 && (BurnDrvGetHardwareCode() & HARDWARE_PUBLIC_MASK) == HARDWARE_IGS_PGM) {
			pgi->nInput = GIT_MACRO_AUTO;
			pgi->nType = BIT_DIGITAL;
			pgi->Macro.nMode = 0;
			sprintf(pgi->Macro.szName, "P%i Buttons 12", nPlayer + 1);
			BurnDrvGetInputInfo(&bii, nPgmButtons[nPlayer][0]);
			pgi->Macro.pVal[0] = bii.pVal;
			pgi->Macro.nVal[0] = 1;
			BurnDrvGetInputInfo(&bii, nPgmButtons[nPlayer][1]);
			pgi->Macro.pVal[1] = bii.pVal;
			pgi->Macro.nVal[1] = 1;
			nMacroCount++;
			pgi++;
			
			pgi->nInput = GIT_MACRO_AUTO;
			pgi->nType = BIT_DIGITAL;
			pgi->Macro.nMode = 0;
			sprintf(pgi->Macro.szName, "P%i Buttons 13", nPlayer + 1);
			BurnDrvGetInputInfo(&bii, nPgmButtons[nPlayer][0]);
			pgi->Macro.pVal[0] = bii.pVal;
			pgi->Macro.nVal[0] = 1;
			BurnDrvGetInputInfo(&bii, nPgmButtons[nPlayer][2]);
			pgi->Macro.pVal[1] = bii.pVal;
			pgi->Macro.nVal[1] = 1;
			nMacroCount++;
			pgi++;
			
			pgi->nInput = GIT_MACRO_AUTO;
			pgi->nType = BIT_DIGITAL;
			pgi->Macro.nMode = 0;
			sprintf(pgi->Macro.szName, "P%i Buttons 14", nPlayer + 1);
			BurnDrvGetInputInfo(&bii, nPgmButtons[nPlayer][0]);
			pgi->Macro.pVal[0] = bii.pVal;
			pgi->Macro.nVal[0] = 1;
			BurnDrvGetInputInfo(&bii, nPgmButtons[nPlayer][3]);
			pgi->Macro.pVal[1] = bii.pVal;
			pgi->Macro.nVal[1] = 1;
			nMacroCount++;
			pgi++;
			
			pgi->nInput = GIT_MACRO_AUTO;
			pgi->nType = BIT_DIGITAL;
			pgi->Macro.nMode = 0;
			sprintf(pgi->Macro.szName, "P%i Buttons 23", nPlayer + 1);
			BurnDrvGetInputInfo(&bii, nPgmButtons[nPlayer][1]);
			pgi->Macro.pVal[0] = bii.pVal;
			pgi->Macro.nVal[0] = 1;
			BurnDrvGetInputInfo(&bii, nPgmButtons[nPlayer][2]);
			pgi->Macro.pVal[1] = bii.pVal;
			pgi->Macro.nVal[1] = 1;
			nMacroCount++;
			pgi++;
			
			pgi->nInput = GIT_MACRO_AUTO;
			pgi->nType = BIT_DIGITAL;
			pgi->Macro.nMode = 0;
			sprintf(pgi->Macro.szName, "P%i Buttons 24", nPlayer + 1);
			BurnDrvGetInputInfo(&bii, nPgmButtons[nPlayer][1]);
			pgi->Macro.pVal[0] = bii.pVal;
			pgi->Macro.nVal[0] = 1;
			BurnDrvGetInputInfo(&bii, nPgmButtons[nPlayer][3]);
			pgi->Macro.pVal[1] = bii.pVal;
			pgi->Macro.nVal[1] = 1;
			nMacroCount++;
			pgi++;
			
			pgi->nInput = GIT_MACRO_AUTO;
			pgi->nType = BIT_DIGITAL;
			pgi->Macro.nMode = 0;
			sprintf(pgi->Macro.szName, "P%i Buttons 34", nPlayer + 1);
			BurnDrvGetInputInfo(&bii, nPgmButtons[nPlayer][2]);
			pgi->Macro.pVal[0] = bii.pVal;
			pgi->Macro.nVal[0] = 1;
			BurnDrvGetInputInfo(&bii, nPgmButtons[nPlayer][3]);
			pgi->Macro.pVal[1] = bii.pVal;
			pgi->Macro.nVal[1] = 1;
			nMacroCount++;
			pgi++;
			
			pgi->nInput = GIT_MACRO_AUTO;
			pgi->nType = BIT_DIGITAL;
			pgi->Macro.nMode = 0;
			sprintf(pgi->Macro.szName, "P%i Buttons 123", nPlayer + 1);
			BurnDrvGetInputInfo(&bii, nPgmButtons[nPlayer][0]);
			pgi->Macro.pVal[0] = bii.pVal;
			pgi->Macro.nVal[0] = 1;
			BurnDrvGetInputInfo(&bii, nPgmButtons[nPlayer][1]);
			pgi->Macro.pVal[1] = bii.pVal;
			pgi->Macro.nVal[1] = 1;
			BurnDrvGetInputInfo(&bii, nPgmButtons[nPlayer][2]);
			pgi->Macro.pVal[2] = bii.pVal;
			pgi->Macro.nVal[2] = 1;
			nMacroCount++;
			pgi++;
			
			pgi->nInput = GIT_MACRO_AUTO;
			pgi->nType = BIT_DIGITAL;
			pgi->Macro.nMode = 0;
			sprintf(pgi->Macro.szName, "P%i Buttons 124", nPlayer + 1);
			BurnDrvGetInputInfo(&bii, nPgmButtons[nPlayer][0]);
			pgi->Macro.pVal[0] = bii.pVal;
			pgi->Macro.nVal[0] = 1;
			BurnDrvGetInputInfo(&bii, nPgmButtons[nPlayer][1]);
			pgi->Macro.pVal[1] = bii.pVal;
			pgi->Macro.nVal[1] = 1;
			BurnDrvGetInputInfo(&bii, nPgmButtons[nPlayer][3]);
			pgi->Macro.pVal[2] = bii.pVal;
			pgi->Macro.nVal[2] = 1;
			nMacroCount++;
			pgi++;
			
			pgi->nInput = GIT_MACRO_AUTO;
			pgi->nType = BIT_DIGITAL;
			pgi->Macro.nMode = 0;
			sprintf(pgi->Macro.szName, "P%i Buttons 134", nPlayer + 1);
			BurnDrvGetInputInfo(&bii, nPgmButtons[nPlayer][0]);
			pgi->Macro.pVal[0] = bii.pVal;
			pgi->Macro.nVal[0] = 1;
			BurnDrvGetInputInfo(&bii, nPgmButtons[nPlayer][2]);
			pgi->Macro.pVal[1] = bii.pVal;
			pgi->Macro.nVal[1] = 1;
			BurnDrvGetInputInfo(&bii, nPgmButtons[nPlayer][3]);
			pgi->Macro.pVal[2] = bii.pVal;
			pgi->Macro.nVal[2] = 1;
			nMacroCount++;
			pgi++;

			pgi->nInput = GIT_MACRO_AUTO;
			pgi->nType = BIT_DIGITAL;
			pgi->Macro.nMode = 0;
			sprintf(pgi->Macro.szName, "P%i Buttons 234", nPlayer + 1);
			BurnDrvGetInputInfo(&bii, nPgmButtons[nPlayer][1]);
			pgi->Macro.pVal[0] = bii.pVal;
			pgi->Macro.nVal[0] = 1;
			BurnDrvGetInputInfo(&bii, nPgmButtons[nPlayer][2]);
			pgi->Macro.pVal[1] = bii.pVal;
			pgi->Macro.nVal[1] = 1;
			BurnDrvGetInputInfo(&bii, nPgmButtons[nPlayer][3]);
			pgi->Macro.pVal[2] = bii.pVal;
			pgi->Macro.nVal[2] = 1;
			nMacroCount++;
			pgi++;

			pgi->nInput = GIT_MACRO_AUTO;
			pgi->nType = BIT_DIGITAL;
			pgi->Macro.nMode = 0;
			sprintf(pgi->Macro.szName, "P%i Buttons 1234", nPlayer + 1);
			BurnDrvGetInputInfo(&bii, nPgmButtons[nPlayer][0]);
			pgi->Macro.pVal[0] = bii.pVal;
			pgi->Macro.nVal[0] = 1;
			BurnDrvGetInputInfo(&bii, nPgmButtons[nPlayer][1]);
			pgi->Macro.pVal[1] = bii.pVal;
			pgi->Macro.nVal[1] = 1;
			BurnDrvGetInputInfo(&bii, nPgmButtons[nPlayer][2]);
			pgi->Macro.pVal[2] = bii.pVal;
			pgi->Macro.nVal[2] = 1;
			BurnDrvGetInputInfo(&bii, nPgmButtons[nPlayer][3]);
			pgi->Macro.pVal[3] = bii.pVal;
			pgi->Macro.nVal[3] = 1;
			nMacroCount++;
			pgi++;
		}
	}

	if ((nPunchx3[0] == 7) && (nKickx3[0] == 7)) {
		bStreetFighterLayout = true;
	}
	if (nFireButtons >= 5 && (BurnDrvGetHardwareCode() & HARDWARE_PUBLIC_MASK) == HARDWARE_CAPCOM_CPS2) {
		bStreetFighterLayout = true;
	}
}

INT32 GameInpInit()
{
	INT32 nRet = 0;
	// Count the number of inputs
	nGameInpCount = 0;
	nMacroCount = 0;
	nMaxMacro = nMaxPlayers * 12;

	for (UINT32 i = 0; i < 0x1000; i++) {
		nRet = BurnDrvGetInputInfo(NULL,i);
		if (nRet) {														// end of input list
			nGameInpCount = i;
			break;
		}
	}

	// Allocate space for all the inputs
	INT32 nSize = (nGameInpCount + nMaxMacro) * sizeof(struct GameInp);
	GameInp = (struct GameInp*)malloc(nSize);
	if (GameInp == NULL) {
		return 1;
	}
	memset(GameInp, 0, nSize);

	GameInpBlank(1);

	InpDIPSWResetDIPs();

	GameInpInitMacros();

	nAnalogSpeed = 0x0100;

	return 0;
}

INT32 GameInpExit()
{
	if (GameInp) {
		free(GameInp);
		GameInp = NULL;
	}

	nGameInpCount = 0;
	nMacroCount = 0;

	nFireButtons = 0;

	bStreetFighterLayout = false;
	bLeftAltkeyMapped = false;

	return 0;
}

// ---------------------------------------------------------------------------
// Convert a string from a config file to an input

static TCHAR* SliderInfo(struct GameInp* pgi, TCHAR* s)
{
	TCHAR* szRet = NULL;
	pgi->Input.Slider.nSliderSpeed = 0x700;				// defaults
	pgi->Input.Slider.nSliderCenter = 0;
	pgi->Input.Slider.nSliderValue = 0x8000;

	szRet = LabelCheck(s, _T("speed"));
	s = szRet;
	if (s == NULL) {
		return s;
	}
	pgi->Input.Slider.nSliderSpeed = (INT16)_tcstol(s, &szRet, 0);
	s = szRet;
	if (s==NULL) {
		return s;
	}
	szRet = LabelCheck(s, _T("center"));
	s = szRet;
	if (s == NULL) {
		return s;
	}
	pgi->Input.Slider.nSliderCenter = (INT16)_tcstol(s, &szRet, 0);
	s = szRet;
	if (s == NULL) {
		return s;
	}

	return szRet;
}

static INT32 StringToJoyAxis(struct GameInp* pgi, TCHAR* s)
{
	TCHAR* szRet = s;

	pgi->Input.JoyAxis.nJoy = (UINT8)_tcstol(s, &szRet, 0);
	if (szRet == NULL) {
		return 1;
	}
	s = szRet;
	pgi->Input.JoyAxis.nAxis = (UINT8)_tcstol(s, &szRet, 0);
	if (szRet == NULL) {
		return 1;
	}

	return 0;
}

static INT32 StringToMouseAxis(struct GameInp* pgi, TCHAR* s)
{
	TCHAR* szRet = s;

	pgi->Input.MouseAxis.nAxis = (UINT8)_tcstol(s, &szRet, 0);
	if (szRet == NULL) {
		return 1;
	}

	return 0;
}

static INT32 StringToMacro(struct GameInp* pgi, TCHAR* s)
{
	TCHAR* szRet = NULL;

	szRet = LabelCheck(s, _T("switch"));
	if (szRet) {
		s = szRet;
		pgi->Macro.nMode = 0x01;
		pgi->Macro.Switch.nCode = (UINT16)_tcstol(s, &szRet, 0);
		return 0;
	}

	return 1;
}

static INT32 StringToInp(struct GameInp* pgi, TCHAR* s)
{
	TCHAR* szRet = NULL;

	SKIP_WS(s);											// skip whitespace
	szRet = LabelCheck(s, _T("undefined"));
	if (szRet) {
		pgi->nInput = 0;
		return 0;
	}

	szRet = LabelCheck(s, _T("constant"));
	if (szRet) {
		pgi->nInput = GIT_CONSTANT;
		s = szRet;
		pgi->Input.Constant.nConst=(UINT8)_tcstol(s, &szRet, 0);
		*(pgi->Input.pVal) = pgi->Input.Constant.nConst;
		return 0;
	}

	szRet = LabelCheck(s, _T("switch"));
	if (szRet) {
		pgi->nInput = GIT_SWITCH;
		s = szRet;
		pgi->Input.Switch.nCode = (UINT16)_tcstol(s, &szRet, 0);
		return 0;
	}

	// Analog using mouse axis:
	szRet = LabelCheck(s, _T("mouseaxis"));
	if (szRet) {
		pgi->nInput = GIT_MOUSEAXIS;
		return StringToMouseAxis(pgi, szRet);
	}
	// Analog using joystick axis:
	szRet = LabelCheck(s, _T("joyaxis-neg"));
	if (szRet) {
		pgi->nInput = GIT_JOYAXIS_NEG;
		return StringToJoyAxis(pgi, szRet);
	}
	szRet = LabelCheck(s, _T("joyaxis-pos"));
	if (szRet) {
		pgi->nInput = GIT_JOYAXIS_POS;
		return StringToJoyAxis(pgi, szRet);
	}
	szRet = LabelCheck(s, _T("joyaxis"));
	if (szRet) {
		pgi->nInput = GIT_JOYAXIS_FULL;
		return StringToJoyAxis(pgi, szRet);
	}

	// Analog using keyboard slider
	szRet = LabelCheck(s, _T("slider"));
	if (szRet) {
		s = szRet;
		pgi->nInput = GIT_KEYSLIDER;
		pgi->Input.Slider.SliderAxis.nSlider[0] = 0;	// defaults
		pgi->Input.Slider.SliderAxis.nSlider[1] = 0;	//

		pgi->Input.Slider.SliderAxis.nSlider[0] = (UINT16)_tcstol(s, &szRet, 0);
		s = szRet;
		if (s == NULL) {
			return 1;
		}
		pgi->Input.Slider.SliderAxis.nSlider[1] = (UINT16)_tcstol(s, &szRet, 0);
		s = szRet;
		if (s == NULL) {
			return 1;
		}
		szRet = SliderInfo(pgi, s);
		s = szRet;
		if (s == NULL) {								// Get remaining slider info
			return 1;
		}
		return 0;
	}

	// Analog using joystick slider
	szRet = LabelCheck(s, _T("joyslider"));
	if (szRet) {
		s = szRet;
		pgi->nInput = GIT_JOYSLIDER;
		pgi->Input.Slider.JoyAxis.nJoy = 0;				// defaults
		pgi->Input.Slider.JoyAxis.nAxis = 0;			//

		pgi->Input.Slider.JoyAxis.nJoy = (UINT8)_tcstol(s, &szRet, 0);
		s = szRet;
		if (s == NULL) {
			return 1;
		}
		pgi->Input.Slider.JoyAxis.nAxis = (UINT8)_tcstol(s, &szRet, 0);
		s = szRet;
		if (s == NULL) {
			return 1;
		}
		szRet = SliderInfo(pgi, s);						// Get remaining slider info
		s = szRet;
		if (s == NULL) {
			return 1;
		}
		return 0;
	}

	return 1;
}

// ---------------------------------------------------------------------------
// Convert an input to a string for config files

static TCHAR* InpToString(struct GameInp* pgi)
{
	static TCHAR szString[80];

	if (pgi->nInput == 0) {
		return _T("undefined");
	}
	if (pgi->nInput == GIT_CONSTANT) {
		_stprintf(szString, _T("constant 0x%.2X"), pgi->Input.Constant.nConst);
		return szString;
	}
	if (pgi->nInput == GIT_SWITCH) {
		_stprintf(szString, _T("switch 0x%.2X"), pgi->Input.Switch.nCode);
		return szString;
	}
	if (pgi->nInput == GIT_KEYSLIDER) {
		_stprintf(szString, _T("slider 0x%.2x 0x%.2x speed 0x%x center %d"), pgi->Input.Slider.SliderAxis.nSlider[0], pgi->Input.Slider.SliderAxis.nSlider[1], pgi->Input.Slider.nSliderSpeed, pgi->Input.Slider.nSliderCenter);
		return szString;
	}
	if (pgi->nInput == GIT_JOYSLIDER) {
		_stprintf(szString, _T("joyslider %d %d speed 0x%x center %d"), pgi->Input.Slider.JoyAxis.nJoy, pgi->Input.Slider.JoyAxis.nAxis, pgi->Input.Slider.nSliderSpeed, pgi->Input.Slider.nSliderCenter);
		return szString;
	}
	if (pgi->nInput == GIT_MOUSEAXIS) {
		_stprintf(szString, _T("mouseaxis %d"), pgi->Input.MouseAxis.nAxis);
		return szString;
	}
	if (pgi->nInput == GIT_JOYAXIS_FULL) {
		_stprintf(szString, _T("joyaxis %d %d"), pgi->Input.JoyAxis.nJoy, pgi->Input.JoyAxis.nAxis);
		return szString;
	}
	if (pgi->nInput == GIT_JOYAXIS_NEG) {
		_stprintf(szString, _T("joyaxis-neg %d %d"), pgi->Input.JoyAxis.nJoy, pgi->Input.JoyAxis.nAxis);
		return szString;
	}
	if (pgi->nInput == GIT_JOYAXIS_POS) {
		_stprintf(szString, _T("joyaxis-pos %d %d"), pgi->Input.JoyAxis.nJoy, pgi->Input.JoyAxis.nAxis);
		return szString;
	}

	return _T("unknown");
}

static TCHAR* InpMacroToString(struct GameInp* pgi)
{
	static TCHAR szString[256];

	if (pgi->nInput == GIT_MACRO_AUTO) {
		if (pgi->Macro.nMode) {
			_stprintf(szString, _T("switch 0x%.2X"), pgi->Macro.Switch.nCode);
			return szString;
		}
	}

	if (pgi->nInput == GIT_MACRO_CUSTOM) {
		struct BurnInputInfo bii;

		if (pgi->Macro.nMode) {
			_stprintf(szString, _T("switch 0x%.2X"), pgi->Macro.Switch.nCode);
		} else {
			_stprintf(szString, _T("undefined"));
		}

		for (INT32 i = 0; i < 4; i++) {
			if (pgi->Macro.pVal[i]) {
				BurnDrvGetInputInfo(&bii, pgi->Macro.nInput[i]);
				_stprintf(szString + _tcslen(szString), _T(" \"%hs\" 0x%02X"), bii.szName, pgi->Macro.nVal[i]);
			}
		}

		return szString;
	}

	return _T("undefined");
}

// ---------------------------------------------------------------------------
// Generate a user-friendly name for a control (PC-side)

static struct { INT32 nCode; TCHAR* szName; } KeyNames[] = {

#define FBK_DEFNAME(k) k, _T(#k)

	{ FBK_ESCAPE,				_T("ESCAPE") },
	{ FBK_1,					_T("1") },
	{ FBK_2,					_T("2") },
	{ FBK_3,					_T("3") },
	{ FBK_4,					_T("4") },
	{ FBK_5,					_T("5") },
	{ FBK_6,					_T("6") },
	{ FBK_7,					_T("7") },
	{ FBK_8,					_T("8") },
	{ FBK_9,					_T("9") },
	{ FBK_0,					_T("0") },
	{ FBK_MINUS,				_T("MINUS") },
	{ FBK_EQUALS,				_T("EQUALS") },
	{ FBK_BACK,					_T("BACKSPACE") },
	{ FBK_TAB,					_T("TAB") },
	{ FBK_Q,					_T("Q") },
	{ FBK_W,					_T("W") },
	{ FBK_E,					_T("E") },
	{ FBK_R,					_T("R") },
	{ FBK_T,					_T("T") },
	{ FBK_Y,					_T("Y") },
	{ FBK_U,					_T("U") },
	{ FBK_I,					_T("I") },
	{ FBK_O,					_T("O") },
	{ FBK_P,					_T("P") },
	{ FBK_LBRACKET,				_T("OPENING BRACKET") },
	{ FBK_RBRACKET,				_T("CLOSING BRACKET") },
	{ FBK_RETURN,				_T("ENTER") },
	{ FBK_LCONTROL,				_T("LEFT CONTROL") },
	{ FBK_A,					_T("A") },
	{ FBK_S,					_T("S") },
	{ FBK_D,					_T("D") },
	{ FBK_F,					_T("F") },
	{ FBK_G,					_T("G") },
	{ FBK_H,					_T("H") },
	{ FBK_J,					_T("J") },
	{ FBK_K,					_T("K") },
	{ FBK_L,					_T("L") },
	{ FBK_SEMICOLON,			_T("SEMICOLON") },
	{ FBK_APOSTROPHE,			_T("APOSTROPHE") },
	{ FBK_GRAVE,				_T("ACCENT GRAVE") },
	{ FBK_LSHIFT,				_T("LEFT SHIFT") },
	{ FBK_BACKSLASH,			_T("BACKSLASH") },
	{ FBK_Z,					_T("Z") },
	{ FBK_X,					_T("X") },
	{ FBK_C,					_T("C") },
	{ FBK_V,					_T("V") },
	{ FBK_B,					_T("B") },
	{ FBK_N,					_T("N") },
	{ FBK_M,					_T("M") },
	{ FBK_COMMA,				_T("COMMA") },
	{ FBK_PERIOD,				_T("PERIOD") },
	{ FBK_SLASH,				_T("SLASH") },
	{ FBK_RSHIFT,				_T("RIGHT SHIFT") },
	{ FBK_MULTIPLY,				_T("NUMPAD MULTIPLY") },
	{ FBK_LALT,					_T("LEFT MENU") },
	{ FBK_SPACE,				_T("SPACE") },
	{ FBK_CAPITAL,				_T("CAPSLOCK") },
	{ FBK_F1,					_T("F1") },
	{ FBK_F2,					_T("F2") },
	{ FBK_F3,					_T("F3") },
	{ FBK_F4,					_T("F4") },
	{ FBK_F5,					_T("F5") },
	{ FBK_F6,					_T("F6") },
	{ FBK_F7,					_T("F7") },
	{ FBK_F8,					_T("F8") },
	{ FBK_F9,					_T("F9") },
	{ FBK_F10,					_T("F10") },
	{ FBK_NUMLOCK,				_T("NUMLOCK") },
	{ FBK_SCROLL,				_T("SCROLLLOCK") },
	{ FBK_NUMPAD7,				_T("NUMPAD 7") },
	{ FBK_NUMPAD8,				_T("NUMPAD 8") },
	{ FBK_NUMPAD9,				_T("NUMPAD 9") },
	{ FBK_SUBTRACT,				_T("NUMPAD SUBTRACT") },
	{ FBK_NUMPAD4,				_T("NUMPAD 4") },
	{ FBK_NUMPAD5,				_T("NUMPAD 5") },
	{ FBK_NUMPAD6,				_T("NUMPAD 6") },
	{ FBK_ADD,					_T("NUMPAD ADD") },
	{ FBK_NUMPAD1,				_T("NUMPAD 1") },
	{ FBK_NUMPAD2,				_T("NUMPAD 2") },
	{ FBK_NUMPAD3,				_T("NUMPAD 3") },
	{ FBK_NUMPAD0,				_T("NUMPAD 0") },
	{ FBK_DECIMAL,				_T("NUMPAD DECIMAL POINT") },
	{ FBK_DEFNAME(FBK_OEM_102) },
	{ FBK_F11,					_T("F11") },
	{ FBK_F12,					_T("F12") },
	{ FBK_F13,					_T("F13") },
	{ FBK_F14,					_T("F14") },
	{ FBK_F15,					_T("F15") },
	{ FBK_DEFNAME(FBK_KANA) },
	{ FBK_DEFNAME(FBK_ABNT_C1) },
	{ FBK_DEFNAME(FBK_CONVERT) },
	{ FBK_DEFNAME(FBK_NOCONVERT) },
	{ FBK_DEFNAME(FBK_YEN) },
	{ FBK_DEFNAME(FBK_ABNT_C2) },
	{ FBK_NUMPADEQUALS,			_T("NUMPAD EQUALS") },
	{ FBK_DEFNAME(FBK_PREVTRACK) },
	{ FBK_DEFNAME(FBK_AT) },
	{ FBK_COLON,				_T("COLON") },
	{ FBK_UNDERLINE,			_T("UNDERSCORE") },
	{ FBK_DEFNAME(FBK_KANJI) },
	{ FBK_DEFNAME(FBK_STOP) },
	{ FBK_DEFNAME(FBK_AX) },
	{ FBK_DEFNAME(FBK_UNLABELED) },
	{ FBK_DEFNAME(FBK_NEXTTRACK) },
	{ FBK_NUMPADENTER,			_T("NUMPAD ENTER") },
	{ FBK_RCONTROL,				_T("RIGHT CONTROL") },
	{ FBK_DEFNAME(FBK_MUTE) },
	{ FBK_DEFNAME(FBK_CALCULATOR) },
	{ FBK_DEFNAME(FBK_PLAYPAUSE) },
	{ FBK_DEFNAME(FBK_MEDIASTOP) },
	{ FBK_DEFNAME(FBK_VOLUMEDOWN) },
	{ FBK_DEFNAME(FBK_VOLUMEUP) },
	{ FBK_DEFNAME(FBK_WEBHOME) },
	{ FBK_DEFNAME(FBK_NUMPADCOMMA) },
	{ FBK_DIVIDE,				_T("NUMPAD DIVIDE") },
	{ FBK_SYSRQ,				_T("PRINTSCREEN") },
	{ FBK_RALT,					_T("RIGHT MENU") },
	{ FBK_PAUSE,				_T("PAUSE") },
	{ FBK_HOME,					_T("HOME") },
	{ FBK_UPARROW,				_T("ARROW UP") },
	{ FBK_PRIOR,				_T("PAGE UP") },
	{ FBK_LEFTARROW,			_T("ARROW LEFT") },
	{ FBK_RIGHTARROW,			_T("ARROW RIGHT") },
	{ FBK_END,					_T("END") },
	{ FBK_DOWNARROW,			_T("ARROW DOWN") },
	{ FBK_NEXT,					_T("PAGE DOWN") },
	{ FBK_INSERT,				_T("INSERT") },
	{ FBK_DELETE,				_T("DELETE") },
	{ FBK_LWIN,					_T("LEFT WINDOWS") },
	{ FBK_RWIN,					_T("RIGHT WINDOWS") },
	{ FBK_DEFNAME(FBK_APPS) },
	{ FBK_DEFNAME(FBK_POWER) },
	{ FBK_DEFNAME(FBK_SLEEP) },
	{ FBK_DEFNAME(FBK_WAKE) },
	{ FBK_DEFNAME(FBK_WEBSEARCH) },
	{ FBK_DEFNAME(FBK_WEBFAVORITES) },
	{ FBK_DEFNAME(FBK_WEBREFRESH) },
	{ FBK_DEFNAME(FBK_WEBSTOP) },
	{ FBK_DEFNAME(FBK_WEBFORWARD) },
	{ FBK_DEFNAME(FBK_WEBBACK) },
	{ FBK_DEFNAME(FBK_MYCOMPUTER) },
	{ FBK_DEFNAME(FBK_MAIL) },
	{ FBK_DEFNAME(FBK_MEDIASELECT) },

#undef FBK_DEFNAME

	{ 0,				NULL }
};

TCHAR* InputCodeDesc(INT32 c)
{
	static TCHAR szString[64];
	TCHAR* szName = _T("");

	// Mouse
	if (c >= 0x8000) {
		INT32 nMouse = (c >> 8) & 0x3F;
		INT32 nCode = c & 0xFF;
		if (nCode >= 0x80) {
			_stprintf(szString, _T("Mouse %d Button %d"), nMouse, nCode & 0x7F);
			return szString;
		}
		if (nCode < 0x06) {
			TCHAR szAxis[3][3] = { _T("X"), _T("Y"), _T("Z") };
			TCHAR szDir[6][16] = { _T("negative"), _T("positive"), _T("Left"), _T("Right"), _T("Up"), _T("Down") };
			if (nCode < 4) {
				_stprintf(szString, _T("Mouse %d %s (%s %s)"), nMouse, szDir[nCode + 2], szAxis[nCode >> 1], szDir[nCode & 1]);
			} else {
				_stprintf(szString, _T("Mouse %d %s %s"), nMouse, szAxis[nCode >> 1], szDir[nCode & 1]);
			}
			return szString;
		}
	}

	// Joystick
	if (c >= 0x4000 && c < 0x8000) {
		INT32 nJoy = (c >> 8) & 0x3F;
		INT32 nCode = c & 0xFF;
		if (nCode >= 0x80) {
			_stprintf(szString, _T("Joy %d Button %d"), nJoy, nCode & 0x7F);
			return szString;
		}
		if (nCode < 0x10) {
			TCHAR szAxis[8][3] = { _T("X"), _T("Y"), _T("Z"), _T("rX"), _T("rY"), _T("rZ"), _T("s0"), _T("s1") };
			TCHAR szDir[6][16] = { _T("negative"), _T("positive"), _T("Left"), _T("Right"), _T("Up"), _T("Down") };
			if (nCode < 4) {
				_stprintf(szString, _T("Joy %d %s (%s %s)"), nJoy, szDir[nCode + 2], szAxis[nCode >> 1], szDir[nCode & 1]);
			} else {
				_stprintf(szString, _T("Joy %d %s %s"), nJoy, szAxis[nCode >> 1], szDir[nCode & 1]);
			}
			return szString;
		}
		if (nCode < 0x20) {
			TCHAR szDir[4][16] = { _T("Left"), _T("Right"), _T("Up"), _T("Down") };
			_stprintf(szString, _T("Joy %d POV-hat %d %s"), nJoy, (nCode & 0x0F) >> 2, szDir[nCode & 3]);
			return szString;
		}
	}

	for (INT32 i = 0; KeyNames[i].nCode; i++) {
		if (c == KeyNames[i].nCode) {
			if (KeyNames[i].szName) {
				szName = KeyNames[i].szName;
			}
			break;
		}
	}

	if (szName[0]) {
		_stprintf(szString, _T("%s"), szName);
	} else {
		_stprintf(szString, _T("code 0x%.2X"), c);
	}

	return szString;
}

TCHAR* InpToDesc(struct GameInp* pgi)
{
	static TCHAR szInputName[64] = _T("");

	if (pgi->nInput == 0) {
		return _T("");
	}
	if (pgi->nInput == GIT_CONSTANT) {
		if (pgi->nType & BIT_GROUP_CONSTANT) {
			for (INT32 i = 0; i < 8; i++) {
				szInputName[7 - i] = pgi->Input.Constant.nConst & (1 << i) ? _T('1') : _T('0');
			}
			szInputName[8] = 0;

			return szInputName;
		}

		if (pgi->Input.Constant.nConst == 0) {
			return _T("-");
		}
	}
	if (pgi->nInput == GIT_SWITCH) {
		return InputCodeDesc(pgi->Input.Switch.nCode);
	}
	if (pgi->nInput == GIT_MOUSEAXIS) {
		TCHAR nAxis = _T('?');
		switch (pgi->Input.MouseAxis.nAxis) {
			case 0:
				nAxis = _T('X');
				break;
			case 1:
				nAxis = _T('Y');
				break;
			case 2:
				nAxis = _T('Z');
				break;
		}
		_stprintf(szInputName, _T("Mouse %i %c axis"), pgi->Input.MouseAxis.nMouse, nAxis);
		return szInputName;
	}
	if (pgi->nInput & GIT_GROUP_JOYSTICK) {
		TCHAR szAxis[8][3] = { _T("X"), _T("Y"), _T("Z"), _T("rX"), _T("rY"), _T("rZ"), _T("s0"), _T("s1") };
		TCHAR szRange[4][16] = { _T("unknown"), _T("full"), _T("negative"), _T("positive") };
		INT32 nRange = 0;
		switch (pgi->nInput) {
			case GIT_JOYAXIS_FULL:
				nRange = 1;
				break;
			case GIT_JOYAXIS_NEG:
				nRange = 2;
				break;
			case GIT_JOYAXIS_POS:
				nRange = 3;
				break;
		}

		_stprintf(szInputName, _T("Joy %d %s axis (%s range)"), pgi->Input.JoyAxis.nJoy, szAxis[pgi->Input.JoyAxis.nAxis], szRange[nRange]);
		return szInputName;
	}

	return InpToString(pgi);							// Just do the rest as they are in the config file
}

TCHAR* InpMacroToDesc(struct GameInp* pgi)
{
	if (pgi->nInput & GIT_GROUP_MACRO) {
		if (pgi->Macro.nMode) {
			return InputCodeDesc(pgi->Macro.Switch.nCode);
		}
	}

	return _T("");
}

// ---------------------------------------------------------------------------

// Find the input number by info
static UINT32 InputInfoToNum(TCHAR* szName)
{
	for (UINT32 i = 0; i < nGameInpCount; i++) {
		struct BurnInputInfo bii;
		BurnDrvGetInputInfo(&bii, i);
		if (bii.pVal == NULL) {
			continue;
		}

		if (_tcsicmp(szName, ANSIToTCHAR(bii.szInfo, NULL, 0)) == 0) {
			return i;
		}
	}
	return ~0U;
}

// Find the input number by name
static UINT32 InputNameToNum(TCHAR* szName)
{
	for (UINT32 i = 0; i < nGameInpCount; i++) {
		struct BurnInputInfo bii;
		BurnDrvGetInputInfo(&bii, i);
		if (bii.pVal == NULL) {
			continue;
		}

		if (_tcsicmp(szName, ANSIToTCHAR(bii.szName, NULL, 0)) == 0) {
			return i;
		}
	}
	return ~0U;
}

static TCHAR* InputNumToName(UINT32 i)
{
	struct BurnInputInfo bii;
	bii.szName = NULL;
	BurnDrvGetInputInfo(&bii, i);
	if (bii.szName == NULL) {
		return _T("unknown");
	}
	return ANSIToTCHAR(bii.szName, NULL, 0);
}

static UINT32 MacroNameToNum(TCHAR* szName)
{
	struct GameInp* pgi = GameInp + nGameInpCount;
	for (UINT32 i = 0; i < nMacroCount; i++, pgi++) {
		if (pgi->nInput & GIT_GROUP_MACRO) {
			if (_tcsicmp(szName, ANSIToTCHAR(pgi->Macro.szName, NULL, 0)) == 0) {
				return i;
			}
		}
	}
	return ~0U;
}

// ---------------------------------------------------------------------------

static INT32 GameInpAutoOne(struct GameInp* pgi, char* szi)
{
	for (INT32 i = 0; i < nMaxPlayers; i++) {
		INT32 nSlide = nPlayerDefaultControls[i] >> 4;
		switch (nPlayerDefaultControls[i] & 0x0F) {
			case 0:										// Keyboard
				GamcAnalogKey(pgi, szi, i, nSlide);
				GamcPlayer(pgi, szi, i, -1);
				GamcMisc(pgi, szi, i);
				break;
			case 1:										// Joystick 1
				GamcAnalogJoy(pgi, szi, i, 0, nSlide);
				GamcPlayer(pgi, szi, i, 0);
				GamcMisc(pgi, szi, i);
				break;
			case 2:										// Joystick 2
				GamcAnalogJoy(pgi, szi, i, 1, nSlide);
				GamcPlayer(pgi, szi, i, 1);
				GamcMisc(pgi, szi, i);
				break;
			case 3:										// Joystick 3
				GamcAnalogJoy(pgi, szi, i, 2, nSlide);
				GamcPlayer(pgi, szi, i, 2);
				GamcMisc(pgi, szi, i);
				break;
			case 4:										// X-Arcade left side
				GamcMisc(pgi, szi, i);
				GamcPlayerHotRod(pgi, szi, i, 0x10, nSlide);
				break;
			case 5:										// X-Arcade right side
				GamcMisc(pgi, szi, i);
				GamcPlayerHotRod(pgi, szi, i, 0x11, nSlide);
				break;
			case 6:										// Hot Rod left side
				GamcMisc(pgi, szi, i);
				GamcPlayerHotRod(pgi, szi, i, 0x00, nSlide);
				break;
			case 7:										// Hot Rod right side
				GamcMisc(pgi, szi, i);
				GamcPlayerHotRod(pgi, szi, i, 0x01, nSlide);
				break;
			default:
				GamcMisc(pgi, szi, i);
		}
	}

	return 0;
}

static INT32 AddCustomMacro(TCHAR* szValue, bool bOverWrite)
{
	TCHAR* szQuote = NULL;
	TCHAR* szEnd = NULL;

	if (QuoteRead(&szQuote, &szEnd, szValue)) {
		return 1;
	}

	INT32 nMode = -1;
	INT32 nInput = -1;
	bool bCreateNew = false;
	struct BurnInputInfo bii;

	for (UINT32 j = nGameInpCount; j < nGameInpCount + nMacroCount; j++) {
		if (GameInp[j].nInput == GIT_MACRO_CUSTOM) {
			if (LabelCheck(szQuote, ANSIToTCHAR(GameInp[j].Macro.szName, NULL, 0))) {
				nInput = j;
				break;
			}
		}
	}

	if (nInput == -1) {
		if (nMacroCount + 1 == nMaxMacro) {
			return 1;
		}
		nInput = nGameInpCount + nMacroCount;
		bCreateNew = true;
	}

	_tcscpy(szQuote, ANSIToTCHAR(GameInp[nInput].Macro.szName, NULL, 0));

	if ((szValue = LabelCheck(szEnd, _T("undefined"))) != NULL) {
		nMode = 0;
	} else {
		if ((szValue = LabelCheck(szEnd, _T("switch"))) != NULL) {

			if (bOverWrite || GameInp[nInput].Macro.nMode == 0) {
				GameInp[nInput].Macro.Switch.nCode = (UINT16)_tcstol(szValue, &szValue, 0);
			}

			nMode = 1;
		}
	}

	if (nMode >= 0) {
		INT32 nFound = 0;

		for (INT32 i = 0; i < 4; i++) {
			GameInp[nInput].Macro.pVal[i] = NULL;
			GameInp[nInput].Macro.nVal[i] = 0;
			GameInp[nInput].Macro.nInput[i] = 0;

			if (szValue == NULL) {
				break;
			}

			if (QuoteRead(&szQuote, &szEnd, szValue)) {
				break;
			}

			for (UINT32 j = 0; j < nGameInpCount; j++) {
				bii.szName = NULL;
				BurnDrvGetInputInfo(&bii, j);
				if (bii.pVal == NULL) {
					continue;
				}

				TCHAR* szString = LabelCheck(szQuote, ANSIToTCHAR(bii.szName, NULL, 0));
				if (szString && szEnd) {
					GameInp[nInput].Macro.pVal[i] = bii.pVal;
					GameInp[nInput].Macro.nInput[i] = j;

					GameInp[nInput].Macro.nVal[i] = (UINT8)_tcstol(szEnd, &szValue, 0);

					nFound++;

					break;
				}
			}
		}

		if (nFound) {
			if (GameInp[nInput].Macro.pVal[nFound - 1]) {
				GameInp[nInput].nInput = GIT_MACRO_CUSTOM;
				GameInp[nInput].Macro.nMode = nMode;
				if (bCreateNew) {
					nMacroCount++;
				}
				return 0;
			}
		}
	}

	return 1;
}

INT32 GameInputAutoIni(INT32 nPlayer, TCHAR* lpszFile, bool bOverWrite)
{
	TCHAR szLine[1024];
	INT32 nFileVersion = 0;
	UINT32 i;

	nAnalogSpeed = 0x0100;
	
	FILE* h = _tfopen(lpszFile, _T("rt"));
	if (h == NULL) {
		return 1;
	}

	// Go through each line of the config file and process inputs
	while (_fgetts(szLine, sizeof(szLine), h)) {
		TCHAR* szValue;
		INT32 nLen = _tcslen(szLine);

		// Get rid of the linefeed at the end
		if (szLine[nLen - 1] == 10) {
			szLine[nLen - 1] = 0;
			nLen--;
		}

		szValue = LabelCheck(szLine, _T("version"));
		if (szValue) {
			nFileVersion = _tcstol(szValue, NULL, 0);
		}
		szValue = LabelCheck(szLine, _T("analog"));
		if (szValue) {
			nAnalogSpeed = _tcstol(szValue, NULL, 0);
		}

		if (nConfigMinVersion <= nFileVersion && nFileVersion <= nBurnVer) {
			szValue = LabelCheck(szLine, _T("input"));
			if (szValue) {
				TCHAR* szQuote = NULL;
				TCHAR* szEnd = NULL;
				if (QuoteRead(&szQuote, &szEnd, szValue)) {
					continue;
				}

				if ((szQuote[0] == _T('p') || szQuote[0] == _T('P')) && szQuote[1] >= _T('1') && szQuote[1] <= _T('0') + nMaxPlayers && szQuote[2] == _T(' ')) {
					if (szQuote[1] != _T('1') + nPlayer) {
						continue;
					}
				} else {
					if (nPlayer != 0) {
						continue;
					}
				}

				// Find which input number this refers to
				i = InputNameToNum(szQuote);
				if (i == ~0U) {
					i = InputInfoToNum(szQuote);
					if (i == ~0U) {
						continue;
					}
				}

				if (GameInp[i].nInput == 0 || bOverWrite) {				// Undefined - assign mapping
					StringToInp(GameInp + i, szEnd);
				}
			}

			szValue = LabelCheck(szLine, _T("macro"));
			if (szValue) {
				TCHAR* szQuote = NULL;
				TCHAR* szEnd = NULL;
				if (QuoteRead(&szQuote, &szEnd, szValue)) {
					continue;
				}

				i = MacroNameToNum(szQuote);
				if (i != ~0U) {
					i += nGameInpCount;
					if (GameInp[i].Macro.nMode == 0 || bOverWrite) {	// Undefined - assign mapping
						StringToMacro(GameInp + i, szEnd);
					}
				}
			}

			szValue = LabelCheck(szLine, _T("custom"));
			if (szValue) {
				AddCustomMacro(szValue, bOverWrite);
			}
		}
	}

	fclose(h);

	return 0;
}

INT32 ConfigGameLoadHardwareDefaults()
{
	TCHAR *szDefaultCpsFile = _T("config/presets/cps.ini");
	TCHAR *szDefaultNeogeoFile = _T("config/presets/neogeo.ini");
	TCHAR *szDefaultPgmFile = _T("config/presets/pgm.ini");
	TCHAR *szFileName = _T("");
	INT32 nApplyHardwareDefaults = 0;
	
	INT32 nHardwareFlag = (BurnDrvGetHardwareCode() & HARDWARE_PUBLIC_MASK);

	if (nHardwareFlag == HARDWARE_CAPCOM_CPS1 || nHardwareFlag == HARDWARE_CAPCOM_CPS1_QSOUND || nHardwareFlag == HARDWARE_CAPCOM_CPS1_GENERIC || nHardwareFlag == HARDWARE_CAPCOM_CPSCHANGER || nHardwareFlag == HARDWARE_CAPCOM_CPS2 || nHardwareFlag == HARDWARE_CAPCOM_CPS3) {
		szFileName = szDefaultCpsFile;
		nApplyHardwareDefaults = 1;
	}
	
	if (nHardwareFlag == HARDWARE_SNK_NEOGEO) {
		szFileName = szDefaultNeogeoFile;
		nApplyHardwareDefaults = 1;
	}
	
	if (nHardwareFlag == HARDWARE_IGS_PGM) {
		szFileName = szDefaultPgmFile;
		nApplyHardwareDefaults = 1;
	}
	
	if (nApplyHardwareDefaults) {
		for (INT32 nPlayer = 0; nPlayer < nMaxPlayers; nPlayer++) {
			GameInputAutoIni(nPlayer, szFileName, true);
		}
	}

	return 0;
}

// Auto-configure any undefined inputs to defaults
INT32 GameInpDefault()
{
	struct GameInp* pgi;
	struct BurnInputInfo bii;
	UINT32 i;

	for (INT32 nPlayer = 0; nPlayer < nMaxPlayers; nPlayer++) {

		if ((nPlayerDefaultControls[nPlayer] & 0x0F) != 0x0F) {
			continue;
		}

		GameInputAutoIni(nPlayer, szPlayerDefaultIni[nPlayer], false);
	}

	// Fill all inputs still undefined
	for (i = 0, pgi = GameInp; i < nGameInpCount; i++, pgi++) {
		if (pgi->nInput) {											// Already defined - leave it alone
			continue;
		}

		// Get the extra info about the input
		bii.szInfo = NULL;
		BurnDrvGetInputInfo(&bii, i);
		if (bii.pVal == NULL) {
			continue;
		}
		if (bii.szInfo == NULL) {
			bii.szInfo = "";
		}

		// Dip switches - set to constant
		if (bii.nType & BIT_GROUP_CONSTANT) {
			pgi->nInput = GIT_CONSTANT;
			continue;
		}

		GameInpAutoOne(pgi, bii.szInfo);
	}

	// Fill in macros still undefined
	for (i = 0; i < nMacroCount; i++, pgi++) {
		if (pgi->nInput != GIT_MACRO_AUTO || pgi->Macro.nMode) {	// Already defined - leave it alone
			continue;
		}

		GameInpAutoOne(pgi, pgi->Macro.szName);
	}

	return 0;
}

// ---------------------------------------------------------------------------
// Write all the GameInps out to config file 'h'

INT32 GameInpWrite(FILE* h)
{
	// Write input types
	for (UINT32 i = 0; i < nGameInpCount; i++) {
		TCHAR* szName = NULL;
		INT32 nPad = 0;
		szName = InputNumToName(i);
		_ftprintf(h, _T("input  \"%s\" "), szName);
		nPad = 16 - _tcslen(szName);
		for (INT32 j = 0; j < nPad; j++) {
			_ftprintf(h, _T(" "));
		}
		_ftprintf(h, _T("%s\n"), InpToString(GameInp + i));
	}

	_ftprintf(h, _T("\n"));

	struct GameInp* pgi = GameInp + nGameInpCount;
	for (UINT32 i = 0; i < nMacroCount; i++, pgi++) {
		INT32 nPad = 0;

		if (pgi->nInput & GIT_GROUP_MACRO) {
			switch (pgi->nInput) {
				case GIT_MACRO_AUTO:									// Auto-assigned macros
					_ftprintf(h, _T("macro  \"%hs\" "), pgi->Macro.szName);
					break;
				case GIT_MACRO_CUSTOM:									// Custom macros
					_ftprintf(h, _T("custom \"%hs\" "), pgi->Macro.szName);
					break;
				default:												// Unknown -- ignore
					continue;
			}

			nPad = 16 - strlen(pgi->Macro.szName);
			for (INT32 j = 0; j < nPad; j++) {
				_ftprintf(h, _T(" "));
			}
			_ftprintf(h, _T("%s\n"), InpMacroToString(pgi));
		}
	}

	return 0;
}

// ---------------------------------------------------------------------------

// Read a GameInp in
INT32 GameInpRead(TCHAR* szVal, bool bOverWrite)
{
	INT32 nRet;
	TCHAR* szQuote = NULL;
	TCHAR* szEnd = NULL;
	UINT32 i = 0;

	nRet = QuoteRead(&szQuote, &szEnd, szVal);
	if (nRet) {
		return 1;
	}

	// Find which input number this refers to
	i = InputNameToNum(szQuote);
	if (i == ~0U) {
		return 1;
	}

	if (bOverWrite || GameInp[i].nInput == 0) {
		// Parse the input description into the GameInp structure
		StringToInp(GameInp + i, szEnd);
	}

	return 0;
}

INT32 GameInpMacroRead(TCHAR* szVal, bool bOverWrite)
{
	INT32 nRet;
	TCHAR* szQuote = NULL;
	TCHAR* szEnd = NULL;
	UINT32 i = 0;

	nRet = QuoteRead(&szQuote, &szEnd, szVal);
	if (nRet) {
		return 1;
	}

	i = MacroNameToNum(szQuote);
	if (i != ~0U) {
		i += nGameInpCount;
		if (GameInp[i].Macro.nMode == 0 || bOverWrite) {
			StringToMacro(GameInp + i, szEnd);
		}
	}

	return 0;
}

INT32 GameInpCustomRead(TCHAR* szVal, bool bOverWrite)
{
	return AddCustomMacro(szVal, bOverWrite);
}

