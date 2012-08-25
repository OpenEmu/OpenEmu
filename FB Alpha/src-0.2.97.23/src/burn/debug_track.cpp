// A module to track if various support devices, cpus, sound modules are in use

#include "burnint.h"

UINT8 Debug_BurnTransferInitted;
UINT8 Debug_BurnGunInitted;
UINT8 Debug_BurnLedInitted;
UINT8 Debug_HiscoreInitted;
UINT8 Debug_GenericTilesInitted;

UINT8 DebugDev_8255PPIInitted;
UINT8 DebugDev_EEPROMInitted;
UINT8 DebugDev_PandoraInitted;
UINT8 DebugDev_SeibuSndInitted;
UINT8 DebugDev_TimeKprInitted;

UINT8 DebugSnd_AY8910Initted;
UINT8 DebugSnd_Y8950Initted;
UINT8 DebugSnd_YM2151Initted;
UINT8 DebugSnd_YM2203Initted;
UINT8 DebugSnd_YM2413Initted;
UINT8 DebugSnd_YM2608Initted;
UINT8 DebugSnd_YM2610Initted;
UINT8 DebugSnd_YM2612Initted;
UINT8 DebugSnd_YM3526Initted;
UINT8 DebugSnd_YM3812Initted;
UINT8 DebugSnd_YMF278BInitted;
UINT8 DebugSnd_DACInitted;
UINT8 DebugSnd_ES5506Initted;
UINT8 DebugSnd_ES8712Initted;
UINT8 DebugSnd_FilterRCInitted;
UINT8 DebugSnd_ICS2115Initted;
UINT8 DebugSnd_IremGA20Initted;
UINT8 DebugSnd_K007232Initted;
UINT8 DebugSnd_K051649Initted;
UINT8 DebugSnd_K053260Initted;
UINT8 DebugSnd_K054539Initted;
UINT8 DebugSnd_MSM5205Initted;
UINT8 DebugSnd_MSM6295Initted;
UINT8 DebugSnd_NamcoSndInitted;
UINT8 DebugSnd_RF5C68Initted;
UINT8 DebugSnd_SAA1099Initted;
UINT8 DebugSnd_SamplesInitted;
UINT8 DebugSnd_SegaPCMInitted;
UINT8 DebugSnd_SN76496Initted;
UINT8 DebugSnd_UPD7759Initted;
UINT8 DebugSnd_X1010Initted;
UINT8 DebugSnd_YMZ280BInitted;

UINT8 DebugCPU_ARM7Initted;
UINT8 DebugCPU_ARMInitted;
UINT8 DebugCPU_H6280Initted;
UINT8 DebugCPU_HD6309Initted;
UINT8 DebugCPU_KonamiInitted;
UINT8 DebugCPU_M6502Initted;
UINT8 DebugCPU_M6800Initted;
UINT8 DebugCPU_M6805Initted;
UINT8 DebugCPU_M6809Initted;
UINT8 DebugCPU_S2650Initted;
UINT8 DebugCPU_SekInitted;
UINT8 DebugCPU_VezInitted;
UINT8 DebugCPU_ZetInitted;

UINT8 DebugCPU_I8039Initted;
UINT8 DebugCPU_SH2Initted;

void DebugTrackerExit()
{
	if (Debug_BurnTransferInitted) 		bprintf(PRINT_ERROR, _T("BurnTransfer Not Exited\n"));
	if (Debug_BurnGunInitted) 			bprintf(PRINT_ERROR, _T("BurnGun Not Exited\n"));
	if (Debug_BurnLedInitted) 			bprintf(PRINT_ERROR, _T("BurnLed Not Exited\n"));
	if (Debug_HiscoreInitted) 			bprintf(PRINT_ERROR, _T("Hiscore Not Exited\n"));
	if (Debug_GenericTilesInitted) 		bprintf(PRINT_ERROR, _T("GenericTiles Not Exited\n"));
	
	if (DebugDev_8255PPIInitted) 		bprintf(PRINT_ERROR, _T("Device 8255PPI Not Exited\n"));
	if (DebugDev_EEPROMInitted) 		bprintf(PRINT_ERROR, _T("Device EEPROM Not Exited\n"));
	if (DebugDev_PandoraInitted) 		bprintf(PRINT_ERROR, _T("Device Pandora Not Exited\n"));
	if (DebugDev_SeibuSndInitted) 		bprintf(PRINT_ERROR, _T("Device SeibuSnd Not Exited\n"));
	if (DebugDev_TimeKprInitted) 		bprintf(PRINT_ERROR, _T("Device TimeKpr Not Exited\n"));
	
	if (DebugSnd_AY8910Initted) 		bprintf(PRINT_ERROR, _T("Sound Module AY8910 Not Exited\n"));
	if (DebugSnd_Y8950Initted) 			bprintf(PRINT_ERROR, _T("Sound Module Y8950 Not Exited\n"));
	if (DebugSnd_YM2151Initted) 		bprintf(PRINT_ERROR, _T("Sound Module YM2151 Not Exited\n"));
	if (DebugSnd_YM2203Initted) 		bprintf(PRINT_ERROR, _T("Sound Module YM2203 Not Exited\n"));
	if (DebugSnd_YM2413Initted) 		bprintf(PRINT_ERROR, _T("Sound Module YM2413 Not Exited\n"));
	if (DebugSnd_YM2608Initted) 		bprintf(PRINT_ERROR, _T("Sound Module YM2608 Not Exited\n"));
	if (DebugSnd_YM2610Initted) 		bprintf(PRINT_ERROR, _T("Sound Module YM2610 Not Exited\n"));
	if (DebugSnd_YM2612Initted) 		bprintf(PRINT_ERROR, _T("Sound Module YM2612 Not Exited\n"));
	if (DebugSnd_YM3526Initted) 		bprintf(PRINT_ERROR, _T("Sound Module YM3526 Not Exited\n"));
	if (DebugSnd_YM3812Initted) 		bprintf(PRINT_ERROR, _T("Sound Module YM3812 Not Exited\n"));
	if (DebugSnd_YMF278BInitted) 		bprintf(PRINT_ERROR, _T("Sound Module YMF278B Not Exited\n"));
	if (DebugSnd_DACInitted) 			bprintf(PRINT_ERROR, _T("Sound Module DAC Not Exited\n"));
	if (DebugSnd_ES5506Initted) 		bprintf(PRINT_ERROR, _T("Sound Module ES5506 Not Exited\n"));
	if (DebugSnd_ES8712Initted) 		bprintf(PRINT_ERROR, _T("Sound Module ES8712 Not Exited\n"));
	if (DebugSnd_FilterRCInitted)		bprintf(PRINT_ERROR, _T("Sound Module Filter RC Not Exited\n"));
	if (DebugSnd_ICS2115Initted) 		bprintf(PRINT_ERROR, _T("Sound Module ICS2115 Not Exited\n"));
	if (DebugSnd_IremGA20Initted) 		bprintf(PRINT_ERROR, _T("Sound Module IremGA20 Not Exited\n"));
	if (DebugSnd_K007232Initted) 		bprintf(PRINT_ERROR, _T("Sound Module K007232 Not Exited\n"));
	if (DebugSnd_K051649Initted) 		bprintf(PRINT_ERROR, _T("Sound Module K051649 Not Exited\n"));
	if (DebugSnd_K053260Initted) 		bprintf(PRINT_ERROR, _T("Sound Module K053260 Not Exited\n"));
	if (DebugSnd_K054539Initted) 		bprintf(PRINT_ERROR, _T("Sound Module K054539 Not Exited\n"));
	if (DebugSnd_MSM5205Initted) 		bprintf(PRINT_ERROR, _T("Sound Module MSM5205 Not Exited\n"));
	if (DebugSnd_MSM6295Initted) 		bprintf(PRINT_ERROR, _T("Sound Module MSM6295 Not Exited\n"));
	if (DebugSnd_NamcoSndInitted) 		bprintf(PRINT_ERROR, _T("Sound Module NamcoSnd Not Exited\n"));
	if (DebugSnd_SAA1099Initted) 		bprintf(PRINT_ERROR, _T("Sound Module SAA1099 Not Exited\n"));
	if (DebugSnd_SamplesInitted) 		bprintf(PRINT_ERROR, _T("Sound Module Samples Not Exited\n"));
	if (DebugSnd_SegaPCMInitted) 		bprintf(PRINT_ERROR, _T("Sound Module SegaPCM Not Exited\n"));
	if (DebugSnd_SN76496Initted) 		bprintf(PRINT_ERROR, _T("Sound Module SN76496 Not Exited\n"));
	if (DebugSnd_UPD7759Initted) 		bprintf(PRINT_ERROR, _T("Sound Module UPD7759 Not Exited\n"));
	if (DebugSnd_X1010Initted) 			bprintf(PRINT_ERROR, _T("Sound Module X1010 Not Exited\n"));
	if (DebugSnd_YMZ280BInitted) 		bprintf(PRINT_ERROR, _T("Sound Module YMZ280B Not Exited\n"));
	
	if (DebugCPU_ARM7Initted) 			bprintf(PRINT_ERROR, _T("CPU ARM7 Not Exited\n"));
	if (DebugCPU_ARMInitted) 			bprintf(PRINT_ERROR, _T("CPU ARM Not Exited\n"));
	if (DebugCPU_H6280Initted) 			bprintf(PRINT_ERROR, _T("CPU H6280 Not Exited\n"));
	if (DebugCPU_HD6309Initted) 		bprintf(PRINT_ERROR, _T("CPU HD6309 Not Exited\n"));
	if (DebugCPU_KonamiInitted) 		bprintf(PRINT_ERROR, _T("CPU Konami Not Exited\n"));
	if (DebugCPU_M6502Initted) 			bprintf(PRINT_ERROR, _T("CPU M6502 Not Exited\n"));
	if (DebugCPU_M6800Initted) 			bprintf(PRINT_ERROR, _T("CPU M6800 Not Exited\n"));
	if (DebugCPU_M6805Initted) 			bprintf(PRINT_ERROR, _T("CPU M6805 Not Exited\n"));
	if (DebugCPU_M6809Initted) 			bprintf(PRINT_ERROR, _T("CPU M6809 Not Exited\n"));
	if (DebugCPU_S2650Initted) 			bprintf(PRINT_ERROR, _T("CPU S2650 Not Exited\n"));
	if (DebugCPU_SekInitted) 			bprintf(PRINT_ERROR, _T("CPU Sek Not Exited\n"));
	if (DebugCPU_VezInitted) 			bprintf(PRINT_ERROR, _T("CPU Vez Not Exited\n"));
	if (DebugCPU_ZetInitted) 			bprintf(PRINT_ERROR, _T("CPU Zet Not Exited\n"));
	
	if (DebugCPU_I8039Initted) 			bprintf(PRINT_ERROR, _T("CPU I8039 Not Exited\n"));
	if (DebugCPU_SH2Initted) 			bprintf(PRINT_ERROR, _T("CPU SH2 Not Exited\n"));
}
