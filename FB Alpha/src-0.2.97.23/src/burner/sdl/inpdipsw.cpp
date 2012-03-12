// Burner DipSwitches Dialog module
#include "burner.h"

static unsigned char nPrevDIPSettings[4];

static int nDIPOffset;

static bool bOK;

static void InpDIPSWGetOffset()
{
	BurnDIPInfo bdi;

	nDIPOffset = 0;
	for (int i = 0; BurnDrvGetDIPInfo(&bdi, i) == 0; i++) {
		if (bdi.nFlags == 0xF0) {
			nDIPOffset = bdi.nInput;
			break;
		}
	}
}

void InpDIPSWResetDIPs()
{
	int i = 0;
	BurnDIPInfo bdi;
	struct GameInp* pgi;

	InpDIPSWGetOffset();

	while (BurnDrvGetDIPInfo(&bdi, i) == 0) {
		if (bdi.nFlags == 0xFF) {
			pgi = GameInp + bdi.nInput + nDIPOffset;
			pgi->Input.Constant.nConst = (pgi->Input.Constant.nConst & ~bdi.nMask) | (bdi.nSetting & bdi.nMask);
		}
		i++;
	}
}

static int InpDIPSWListBegin()
{




	return 0;
}

static bool CheckSetting(int i)
{
	BurnDIPInfo bdi;
	BurnDrvGetDIPInfo(&bdi, i);
	struct GameInp* pgi = GameInp + bdi.nInput + nDIPOffset;

	if ((pgi->Input.Constant.nConst & bdi.nMask) == bdi.nSetting) {
		unsigned char nFlags = bdi.nFlags;
		if ((nFlags & 0x0F) <= 1) {
			return true;
		} else {
			for (int j = 1; j < (nFlags & 0x0F); j++) {
				BurnDrvGetDIPInfo(&bdi, i + j);
				pgi = GameInp + bdi.nInput + nDIPOffset;
				if (nFlags & 0x80) {
					if ((pgi->Input.Constant.nConst & bdi.nMask) == bdi.nSetting) {
						return false;
					}
				} else {
					if ((pgi->Input.Constant.nConst & bdi.nMask) != bdi.nSetting) {
						return false;
					}
				}
			}
			return true;
		}
	}
	return false;
}

// Make a list view of the DIPswitches
static int InpDIPSWListMake()
{

	return 0;
}

static int InpDIPSWInit()
{
	BurnDIPInfo bdi;
	struct GameInp *pgi;

	InpDIPSWGetOffset();

	InpDIPSWListBegin();
	InpDIPSWListMake();

	for (int i = 0, j = 0; BurnDrvGetDIPInfo(&bdi, i) == 0; i++) {
		if (bdi.nInput >= 0  && bdi.nFlags == 0xFF) {
			pgi = GameInp + bdi.nInput + nDIPOffset;
			nPrevDIPSettings[j] = pgi->Input.Constant.nConst;
			j++;
		}
	}

	return 0;
}

static int InpDIPSWExit()
{
	GameInpCheckMouse();
	return 0;
}

static void InpDIPSWCancel()
{
	if (!bOK) {
		int i = 0, j = 0;
		BurnDIPInfo bdi;
		struct GameInp *pgi;
		while (BurnDrvGetDIPInfo(&bdi, i) == 0) {
			if (bdi.nInput >= 0 && bdi.nFlags == 0xFF) {
				pgi = GameInp + bdi.nInput + nDIPOffset;
				pgi->Input.Constant.nConst = nPrevDIPSettings[j];
				j++;
			}
			i++;
		}
	}
}

// Create the list of possible values for a DIPswitch
static void InpDIPSWSelect()
{
}

int InpDIPSWCreate()
{
	if (bDrvOkay == 0) {									// No game is loaded
		return 1;
	}

	bOK = false;

	return 0;
}
