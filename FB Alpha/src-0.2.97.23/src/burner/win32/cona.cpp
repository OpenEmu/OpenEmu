// Burner Config file module
#include "burner.h"

#ifdef _UNICODE
 #include <locale.h>
#endif

int nIniVersion = 0;

struct VidPresetData VidPreset[4] = {
	{ 400, 300},
	{ 640, 480},
	{ 1024, 768},
	{ 1280, 960},
};

struct VidPresetDataVer VidPresetVer[4] = {
	{ 400, 300},
	{ 640, 480},
	{ 1024, 768},
	{ 1280, 960},
};

static void CreateConfigName(TCHAR* szConfig)
{
	_stprintf(szConfig, _T("config/%s.ini"), szAppExeName);
	return;
}

// Read in the config file for the whole application
int ConfigAppLoad()
{
	TCHAR szConfig[MAX_PATH];
	TCHAR szLine[1024];
	FILE* h;
	
#ifdef _UNICODE
	setlocale(LC_ALL, "");
#endif

	CreateConfigName(szConfig);

	if ((h = _tfopen(szConfig, _T("rt"))) == NULL) {
		return 1;
	}

	// Go through each line of the config file
	while (_fgetts(szLine, sizeof(szLine), h)) {
		int nLen = _tcslen(szLine);

		// Get rid of the linefeed at the end
		if (szLine[nLen - 1] == 10) {
			szLine[nLen - 1] = 0;
			nLen--;
		}

#define VAR(x) { TCHAR* szValue = LabelCheck(szLine,_T(#x));			\
  if (szValue) x = _tcstol(szValue, NULL, 0); }
#define VAR64(x) { TCHAR* szValue = LabelCheck(szLine,_T(#x));			\
  if (szValue) x = (long long)_tcstod(szValue, NULL); }
#define FLT(x) { TCHAR* szValue = LabelCheck(szLine,_T(#x));			\
  if (szValue) x = _tcstod(szValue, NULL); }
#define STR(x) { TCHAR* szValue = LabelCheck(szLine,_T(#x) _T(" "));	\
  if (szValue) _tcscpy(x,szValue); }
#define PAT(x) { TCHAR* szValue = LabelCheck(szLine,_T(#x) _T(" "));	\
	if (szValue) { _tcscpy(x, szValue); UpdatePath(x); } }
#define DRV(x) { TCHAR* szValue = LabelCheck(szLine,_T(#x) _T(" "));	\
  if (szValue) x = NameToDriver(szValue); }

		VAR(nIniVersion);

		// Emulation
		VAR(bBurnUseASMCPUEmulation);

		// Video
		VAR(nVidDepth); VAR(nVidRefresh);
		VAR(nVidRotationAdjust);

		// horizontal oriented
		VAR(nVidHorWidth); VAR(nVidHorHeight);
		VAR(bVidArcaderesHor);
		VAR(VidPreset[0].nWidth); VAR(VidPreset[0].nHeight);
		VAR(VidPreset[1].nWidth); VAR(VidPreset[1].nHeight);
		VAR(VidPreset[2].nWidth); VAR(VidPreset[2].nHeight);
		VAR(VidPreset[3].nWidth); VAR(VidPreset[3].nHeight);
		VAR(nScreenSizeHor);

		// vertical oriented
		VAR(nVidVerWidth); VAR(nVidVerHeight);
		VAR(bVidArcaderesVer);
		VAR(VidPresetVer[0].nWidth); VAR(VidPresetVer[0].nHeight);
		VAR(VidPresetVer[1].nWidth); VAR(VidPresetVer[1].nHeight);
		VAR(VidPresetVer[2].nWidth); VAR(VidPresetVer[2].nHeight);
		VAR(VidPresetVer[3].nWidth); VAR(VidPresetVer[3].nHeight);
		VAR(nScreenSizeVer);

		VAR(nWindowSize);
		VAR(nWindowPosX); VAR(nWindowPosY);
		VAR(bDoGamma);
		VAR(bVidUseHardwareGamma);
		VAR(bHardwareGammaOnly);
		FLT(nGamma);
		VAR(bVidFullStretch);
		VAR(bVidCorrectAspect);
		
		VAR(bVidAutoSwitchFull);

		VAR(bVidTripleBuffer);
		VAR(bVidVSync);
		VAR(bVidDWMCore);

		VAR(bVidScanlines);
		VAR(nVidScanIntensity);
		VAR(bMonitorAutoCheck);
		VAR(nVidScrnAspectX);
		VAR(nVidScrnAspectY);
		VAR(bForce60Hz);
		VAR(bAlwaysDrawFrames);

		VAR(nVidSelect);
		VAR(nVidBlitterOpt[0]);
		VAR64(nVidBlitterOpt[1]);
		VAR(nVidBlitterOpt[2]);
		VAR(nVidBlitterOpt[3]);
		VAR(nVidBlitterOpt[4]);

		// DirectDraw blitter
		VAR(bVidScanHalf);
		VAR(bVidForceFlip);

		// Direct3D blitter
		VAR(bVidBilinear);
		VAR(bVidScanDelay);
		VAR(bVidScanRotate);
		VAR(bVidScanBilinear);
		VAR(nVidFeedbackIntensity);
		VAR(nVidFeedbackOverSaturation);
		FLT(fVidScreenAngle);
		FLT(fVidScreenCurvature);
		VAR(bVidForce16bit);
		VAR(nVidTransferMethod);

		// DirectX Graphics blitter
		FLT(dVidCubicB);
		FLT(dVidCubicC);
		
		// DirectX Graphics 9 Alt blitter
		VAR(bVidDX9Bilinear);
		VAR(bVidHardwareVertex);
		VAR(bVidMotionBlur);

		// Sound
		VAR(nAudSelect);
		VAR(nAudSegCount);
		VAR(nInterpolation);
		VAR(nFMInterpolation);
		VAR(nAudSampleRate[0]);
		VAR(nAudDSPModule[0]);
		VAR(nAudSampleRate[1]);
		VAR(nAudDSPModule[1]);

		// Other
		STR(szLocalisationTemplate);
		STR(szGamelistLocalisationTemplate);
		VAR(nGamelistLocalisationActive);

		VAR(nVidSDisplayStatus);
		VAR(nMinChatFontSize);
		VAR(nMaxChatFontSize);

		VAR(bModelessMenu);

		VAR(nSplashTime);

		VAR(bDrvSaveAll);
		VAR(nAppThreadPriority);
		VAR(bAlwaysProcessKeyboardInput);
		VAR(bAutoPause);
		VAR(bSaveInputs);
		
		VAR(nCDEmuSelect);
		PAT(CDEmuImage);

		VAR(nSelDlgWidth);
		VAR(nSelDlgHeight);
		VAR(nLoadMenuShowX);
		VAR(nLoadMenuBoardTypeFilter);
		VAR(nLoadMenuGenreFilter);
		VAR(nLoadMenuFamilyFilter);

		STR(szAppRomPaths[0]);
		STR(szAppRomPaths[1]);
		STR(szAppRomPaths[2]);
		STR(szAppRomPaths[3]);
		STR(szAppRomPaths[4]);
		STR(szAppRomPaths[5]);
		STR(szAppRomPaths[6]);
		STR(szAppRomPaths[7]);
		STR(szAppRomPaths[8]);
		STR(szAppRomPaths[9]);
		STR(szAppRomPaths[10]);
		STR(szAppRomPaths[11]);
		STR(szAppRomPaths[12]);
		STR(szAppRomPaths[13]);
		STR(szAppRomPaths[14]);
		STR(szAppRomPaths[15]);
		STR(szAppRomPaths[16]);
		STR(szAppRomPaths[17]);
		STR(szAppRomPaths[18]);
		STR(szAppRomPaths[19]);
		
		STR(szNeoCDGamesDir);
		
		STR(szAppPreviewsPath);
		STR(szAppTitlesPath);
		STR(szAppCheatsPath);
		STR(szAppHiscorePath);
		STR(szAppSamplesPath);
		STR(szAppIpsPath);
		STR(szAppIconsPath);
		STR(szNeoCDCoverDir);

		VAR(bNoChangeNumLock);
		
		VAR(EnableHiscores);
		
		VAR(nIpsSelectedLanguage);
		
		VAR(bEnableIcons);
		VAR(nIconsSize);
		
		STR(szPrevGames[0]);
		STR(szPrevGames[1]);
		STR(szPrevGames[2]);
		STR(szPrevGames[3]);
		STR(szPrevGames[4]);
		STR(szPrevGames[5]);
		STR(szPrevGames[6]);
		STR(szPrevGames[7]);
		STR(szPrevGames[8]);
		STR(szPrevGames[9]);
		
		// MVS cartridges
		DRV(nBurnDrvSelect[0]);
		DRV(nBurnDrvSelect[1]);
		DRV(nBurnDrvSelect[2]);
		DRV(nBurnDrvSelect[3]);
		DRV(nBurnDrvSelect[4]);
		DRV(nBurnDrvSelect[5]);
		
		VAR(bNeoCDListScanSub);
		VAR(bNeoCDListScanOnlyISO);

		// Default Controls
		VAR(nPlayerDefaultControls[0]);
		STR(szPlayerDefaultIni[0]);
		VAR(nPlayerDefaultControls[1]);
		STR(szPlayerDefaultIni[1]);
		VAR(nPlayerDefaultControls[2]);
		STR(szPlayerDefaultIni[2]);
		VAR(nPlayerDefaultControls[3]);
		STR(szPlayerDefaultIni[3]);

#undef DRV
#undef PAT
#undef STR
#undef FLT
#undef VAR
#undef VAR64
	}

	fclose(h);
	return 0;
}

// Write out the config file for the whole application
int ConfigAppSave()
{
	TCHAR szConfig[MAX_PATH];
	FILE *h;

	if (bCmdOptUsed) {
		return 1;
	}
	
#ifdef _UNICODE
	setlocale(LC_ALL, "");
#endif

	CreateConfigName(szConfig);

	if ((h = _tfopen(szConfig, _T("wt"))) == NULL) {
		return 1;
	}

	// Write title
	_ftprintf(h, _T("// ") _T(APP_TITLE) _T(" v%s --- Main Config File\n\n"), szAppBurnVer);
	_ftprintf(h, _T("// Don't edit this file manually unless you know what you're doing\n"));
	_ftprintf(h, _T("// ") _T(APP_TITLE) _T(" will restore default settings when this file is deleted\n"));

#define VAR(x) _ftprintf(h, _T(#x) _T(" %d\n"),  x)
#define VAR64(x) _ftprintf(h, _T(#x) _T(" %lf\n"),  (float)x)
#define FLT(x) _ftprintf(h, _T(#x) _T(" %lf\n"), x)
#define STR(x) _ftprintf(h, _T(#x) _T(" %s\n"),  x)
#define DRV(x) _ftprintf(h, _T(#x) _T(" %s\n"),  DriverToName(x))

	_ftprintf(h, _T("\n// The application version this file was saved from\n"));
	// We can't use the macros for this!
	_ftprintf(h, _T("nIniVersion 0x%06X"), nBurnVer);

	_ftprintf(h, _T("\n\n\n"));
	_ftprintf(h, _T("// --- emulation --------------------------------------------------------------\n"));

	_ftprintf(h, _T("\n// If non-zero, use A68K for MC68000 emulation\n"));
	VAR(bBurnUseASMCPUEmulation);

	_ftprintf(h, _T("\n\n\n"));
	_ftprintf(h, _T("// --- Video ------------------------------------------------------------------\n"));

	// Horizontal oriented
	_ftprintf(h, _T("\n// (Horizontal Oriented) The display mode to use for fullscreen\n"));
	VAR(nVidHorWidth); VAR(nVidHorHeight);	
	_ftprintf(h, _T("\n// (Horizontal Oriented) If non-zero, use the same fullscreen resolution as the original arcade game\n"));
	VAR(bVidArcaderesHor);
	_ftprintf(h, _T("\n// (Horizontal Oriented) The preset resolutions appearing in the menu\n"));
	VAR(VidPreset[0].nWidth); VAR(VidPreset[0].nHeight);
	VAR(VidPreset[1].nWidth); VAR(VidPreset[1].nHeight);
	VAR(VidPreset[2].nWidth); VAR(VidPreset[2].nHeight);
	VAR(VidPreset[3].nWidth); VAR(VidPreset[3].nHeight);
	_ftprintf(h, _T("\n// (Horizontal Oriented) Full-screen size (0 = use display mode variables)\n"));
	VAR(nScreenSizeHor);

	// Vertical oriented
	_ftprintf(h, _T("\n// (Vertical Oriented) The display mode to use for fullscreen\n"));
	VAR(nVidVerWidth); VAR(nVidVerHeight);
	_ftprintf(h, _T("\n// (Vertical Oriented) If non-zero, use the same fullscreen resolution as the original arcade game\n"));
	VAR(bVidArcaderesVer);
	_ftprintf(h, _T("\n// (Vertical Oriented) The preset resolutions appearing in the menu\n"));
	VAR(VidPresetVer[0].nWidth); VAR(VidPresetVer[0].nHeight);
	VAR(VidPresetVer[1].nWidth); VAR(VidPresetVer[1].nHeight);
	VAR(VidPresetVer[2].nWidth); VAR(VidPresetVer[2].nHeight);
	VAR(VidPresetVer[3].nWidth); VAR(VidPresetVer[3].nHeight);
	_ftprintf(h, _T("\n// (Vertical Oriented) Full-screen size (0 = use display mode variables)\n"));
	VAR(nScreenSizeVer);

	_ftprintf(h, _T("\n// Full-screen bit depth\n"));
	VAR(nVidDepth);
	_ftprintf(h, _T("\n// Specify the refresh rate, 0 = default (changing this will not work with many video cards)\n"));
	VAR(nVidRefresh);
	_ftprintf(h, _T("\n// If non-zero, do not rotate the graphics for vertical games\n"));
	VAR(nVidRotationAdjust);
	_ftprintf(h, _T("\n// Initial window size (0 = autosize)\n"));
	VAR(nWindowSize);
	_ftprintf(h, _T("\n// Window position\n"));
	VAR(nWindowPosX); VAR(nWindowPosY);
	_ftprintf(h, _T("\n// If non-zero, perform gamma correction\n"));
	VAR(bDoGamma);
	_ftprintf(h, _T("\n// If non-zero, use the video hardware to correct gamma\n"));
	VAR(bVidUseHardwareGamma);
	_ftprintf(h, _T("\n// If non-zero, don't fall back on software gamma correction\n"));
	VAR(bHardwareGammaOnly);
	_ftprintf(h, _T("\n// Gamma to correct with\n"));
	FLT(nGamma);
	_ftprintf(h, _T("\n// If non-zero, auto-switch to fullscreen after loading game\n"));
	VAR(bVidAutoSwitchFull);
	_ftprintf(h, _T("\n// If non-zero, allow stretching of the image to any size\n"));
	VAR(bVidFullStretch);
	_ftprintf(h, _T("\n// If non-zero, stretch the image to the largest size preserving aspect ratio\n"));
	VAR(bVidCorrectAspect);
	_ftprintf(h, _T("\n// If non-zero, try to use a triple buffer in fullscreen\n"));
	VAR(bVidTripleBuffer);
	_ftprintf(h, _T("\n// If non-zero, try to synchronise blits with the display\n"));
	VAR(bVidVSync);
	_ftprintf(h, _T("\n// If non-zero, try to enable custom DWM parameters on Windows 7, this fixes frame stuttering problems.\n"));
	VAR(bVidDWMCore);
	_ftprintf(h, _T("\n// Transfer method:  0 = blit from system memory / use driver/DirectX texture management;\n"));
	_ftprintf(h, _T("//                   1 = copy to a video memory surface, then use bltfast();\n"));
	_ftprintf(h, _T("//                  -1 = autodetect for DirectDraw, equals 1 for Direct3D\n"));
	VAR(nVidTransferMethod);
	_ftprintf(h, _T("\n// If non-zero, draw scanlines to simulate a low-res monitor\n"));
	VAR(bVidScanlines);
	_ftprintf(h, _T("\n// Maximum scanline intensity\n"));
	VAR(nVidScanIntensity);
	_ftprintf(h, _T("\n// If non-zero, rotate scanlines and RGB effects for rotated games\n"));
	VAR(bVidScanRotate);
	_ftprintf(h, _T("\n// The selected blitter module\n"));
	VAR(nVidSelect);
	_ftprintf(h, _T("\n// Options for the blitter modules\n"));
	VAR(nVidBlitterOpt[0]);
	VAR64(nVidBlitterOpt[1]);
	VAR(nVidBlitterOpt[2]);
	VAR(nVidBlitterOpt[3]);
	VAR(nVidBlitterOpt[4]);
	_ftprintf(h, _T("\n// If non-zero, attempt to auto-detect the monitor aspect ratio\n"));
	VAR(bMonitorAutoCheck);
	_ftprintf(h, _T("\n// The aspect ratio of the monitor\n"));
	VAR(nVidScrnAspectX);
	VAR(nVidScrnAspectY);
	_ftprintf(h, _T("\n// If non-zero, force all games to use a 60Hz refresh rate\n"));
	VAR(bForce60Hz);
	_ftprintf(h, _T("\n// If non-zero, skip frames when needed to keep the emulation running at full speed\n"));
	VAR(bAlwaysDrawFrames);

	_ftprintf(h, _T("\n"));
	_ftprintf(h, _T("// --- DirectDraw blitter module settings -------------------------------------\n"));
	_ftprintf(h, _T("\n// If non-zero, draw scanlines at 50%% intensity\n"));
	VAR(bVidScanHalf);
	_ftprintf(h, _T("\n// If non-zero, force flipping for games that need it\n"));
	VAR(bVidForceFlip);
	_ftprintf(h, _T("\n"));
	_ftprintf(h, _T("// --- Direct3D 7 blitter module settings -------------------------------------\n"));
	_ftprintf(h, _T("\n// If non-zero, use bi-linear filtering to display the image\n"));
	VAR(bVidBilinear);
	_ftprintf(h, _T("\n// If non-zero, simulate slow phosphors (feedback)\n"));
	VAR(bVidScanDelay);
	_ftprintf(h, _T("\n// If non-zero, use bi-linear filtering for the scanlines\n"));
	VAR(bVidScanBilinear);
	_ftprintf(h, _T("\n// Feedback amount for slow phosphor simulation\n"));
	VAR(nVidFeedbackIntensity);
	_ftprintf(h, _T("\n// Oversaturation amount for slow phosphor simulation\n"));
	VAR(nVidFeedbackOverSaturation);
	_ftprintf(h, _T("\n// Angle at wich the emulated screen is tilted (in radians)\n"));
	FLT(fVidScreenAngle);
	_ftprintf(h, _T("\n// Angle of the sphere segment used for the 3D screen (in radians)\n"));
	FLT(fVidScreenCurvature);
	_ftprintf(h, _T("\n// If non-zero, force 16 bit emulation even in 32-bit screenmodes\n"));
	VAR(bVidForce16bit);
	_ftprintf(h, _T("\n"));
	_ftprintf(h, _T("// --- DirectX Graphics 9 blitter module settings -----------------------------\n"));
	_ftprintf(h, _T("\n// The filter parameters for the cubic filter\n"));
	FLT(dVidCubicB);
	FLT(dVidCubicC);
	_ftprintf(h, _T("\n"));
	_ftprintf(h, _T("// --- DirectX Graphics 9 Alt blitter module settings -------------------------\n"));
	_ftprintf(h, _T("\n// If non-zero, use bi-linear filtering to display the image\n"));
	VAR(bVidDX9Bilinear);
	_ftprintf(h, _T("\n// If non-zero, use hardware vertex to display the image\n"));
	VAR(bVidHardwareVertex);
	_ftprintf(h, _T("\n// If non-zero, use motion blur to display the image\n"));
	VAR(bVidMotionBlur);

	_ftprintf(h, _T("\n\n\n"));
	_ftprintf(h, _T("// --- Sound ------------------------------------------------------------------\n"));
	_ftprintf(h, _T("\n// The selected audio plugin\n"));
	VAR(nAudSelect);
	_ftprintf(h, _T("\n// Number of frames in sound buffer (= sound lag)\n"));
	VAR(nAudSegCount);
	_ftprintf(h, _T("\n// The order of PCM/ADPCM interpolation\n"));
	VAR(nInterpolation);
	_ftprintf(h, _T("\n// The order of FM interpolation\n"));
	VAR(nFMInterpolation);
	_ftprintf(h, _T("\n"));
	_ftprintf(h, _T("// --- DirectSound plugin settings --------------------------------------------\n"));
	_ftprintf(h, _T("\n// Sample rate\n"));
	VAR(nAudSampleRate[0]);
	_ftprintf(h, _T("\n// DSP module to use for sound enhancement: 0 = none, 1 = low-pass filter\n"));
	VAR(nAudDSPModule[0]);
	_ftprintf(h, _T("\n"));
	_ftprintf(h, _T("// --- XAudio2 plugin settings ------------------------------------------------\n"));
	_ftprintf(h, _T("\n// Sample rate\n"));
	VAR(nAudSampleRate[1]);
	_ftprintf(h, _T("\n// DSP module to use for sound enhancement: 0 = none, 1 = low-pass filter, 2 = reverb\n"));
	VAR(nAudDSPModule[1]);

	_ftprintf(h, _T("\n\n\n"));
	_ftprintf(h, _T("// --- UI ---------------------------------------------------------------------\n"));

	_ftprintf(h, _T("\n// Filename of the active UI translation template\n"));
	STR(szLocalisationTemplate);
	
	_ftprintf(h, _T("\n// Filename of the active gamelist translation template\n"));
	STR(szGamelistLocalisationTemplate);
	
	_ftprintf(h, _T("\n// If non-zero, enable gamelist localisation\n"));
	VAR(nGamelistLocalisationActive);
	
	_ftprintf(h, _T("\n// 1 = display pause/record/replay/kaillera icons in the upper right corner of the display\n"));
	VAR(nVidSDisplayStatus);
	_ftprintf(h, _T("\n// Minimum height (in pixels) of the font used for the Kaillera chat function (used for arcade resolution)\n"));
	VAR(nMinChatFontSize);
	_ftprintf(h, _T("\n// Maximum height (in pixels) of the font used for the Kaillera chat function (used for 1280x960 or higher).\n"));
	VAR(nMaxChatFontSize);

	_ftprintf(h, _T("\n// Make the menu modeless\n"));
	VAR(bModelessMenu);

	_ftprintf(h, _T("\n// Minimum length of time to display the splash screen (in milliseconds)\n"));
	VAR(nSplashTime);

	_ftprintf(h, _T("\n// If non-zero, load and save all ram (the state)\n"));
	VAR(bDrvSaveAll);
	_ftprintf(h, _T("\n// The thread priority for the application. Do *NOT* edit this manually\n"));
	VAR(nAppThreadPriority);
	_ftprintf(h, _T("\n// If non-zero, process keyboard input even when the application loses focus\n"));
	VAR(bAlwaysProcessKeyboardInput);
	_ftprintf(h, _T("\n// If non-zero, pause when the application loses focus\n"));
	VAR(bAutoPause);
	_ftprintf(h, _T("\n// If non-zero, save the inputs for each game\n"));
	VAR(bSaveInputs);
	
	_ftprintf(h, _T("\n\n\n"));
	_ftprintf(h, _T("// --- CD emulation -----------------------------------------------------------\n"));
	_ftprintf(h, _T("\n // The selected CD emulation module\n"));
	VAR(nCDEmuSelect);
	_ftprintf(h, _T("\n // The path to the CD image to use (.cue or .iso)\n"));
	STR(CDEmuImage);
	
	_ftprintf(h, _T("\n\n\n"));
	_ftprintf(h, _T("// --- Load Game Dialogs ------------------------------------------------------\n"));
	_ftprintf(h, _T("\n// Load game dialog dimensions (in win32 client co-ordinates)\n"));
	VAR(nSelDlgWidth);
	VAR(nSelDlgHeight);
	
	_ftprintf(h, _T("\n// Load game dialog options\n"));
	VAR(nLoadMenuShowX);
	
	_ftprintf(h, _T("\n// Load game dialog board type filter options\n"));
	VAR(nLoadMenuBoardTypeFilter);
	
	_ftprintf(h, _T("\n// Load game dialog genre filter options\n"));
	VAR(nLoadMenuGenreFilter);
	
	_ftprintf(h, _T("\n// Load game dialog family filter options\n"));
	VAR(nLoadMenuFamilyFilter);

	_ftprintf(h, _T("\n// The paths to search for rom zips (include trailing backslash)\n"));
	STR(szAppRomPaths[0]);
	STR(szAppRomPaths[1]);
	STR(szAppRomPaths[2]);
	STR(szAppRomPaths[3]);
	STR(szAppRomPaths[4]);
	STR(szAppRomPaths[5]);
	STR(szAppRomPaths[6]);
	STR(szAppRomPaths[7]);
	STR(szAppRomPaths[8]);
	STR(szAppRomPaths[9]);
	STR(szAppRomPaths[10]);
	STR(szAppRomPaths[11]);
	STR(szAppRomPaths[12]);
	STR(szAppRomPaths[13]);
	STR(szAppRomPaths[14]);
	STR(szAppRomPaths[15]);
	STR(szAppRomPaths[16]);
	STR(szAppRomPaths[17]);
	STR(szAppRomPaths[18]);
	STR(szAppRomPaths[19]);
	
	_ftprintf(h, _T("\n// The path to search for Neo Geo CDZ isos\n"));
	STR(szNeoCDGamesDir);
	
	_ftprintf(h, _T("\n// The paths to search for support files (include trailing backslash)\n"));
	STR(szAppPreviewsPath);
	STR(szAppTitlesPath);
	STR(szAppCheatsPath);
	STR(szAppHiscorePath);
	STR(szAppSamplesPath);
	STR(szAppIpsPath);
	STR(szAppIconsPath);
	STR(szNeoCDCoverDir);
	
	_ftprintf(h, _T("\n// The cartridges to use for emulation of an MVS system\n"));
	DRV(nBurnDrvSelect[0]);
	DRV(nBurnDrvSelect[1]);
	DRV(nBurnDrvSelect[2]);
	DRV(nBurnDrvSelect[3]);
	DRV(nBurnDrvSelect[4]);
	DRV(nBurnDrvSelect[5]);
	
	_ftprintf(h, _T("\n// Neo Geo CD Load Game Dialog options\n"));
	VAR(bNeoCDListScanSub);
	VAR(bNeoCDListScanOnlyISO);

	_ftprintf(h, _T("\n\n\n"));
	_ftprintf(h, _T("// --- miscellaneous ---------------------------------------------------------\n"));

	_ftprintf(h, _T("\n// If non-zero, don't change the status of the Num Lock key.\n"));
	VAR(bNoChangeNumLock);
	
	_ftprintf(h, _T("\n// If non-zero, enable high score saving support.\n"));
	VAR(EnableHiscores);
	
	_ftprintf(h, _T("\n// The language index to use for the IPS Patch Manager dialog.\n"));
	VAR(nIpsSelectedLanguage);
	
	_ftprintf(h, _T("\n// If non-zero, display drivers icons.\n"));
	VAR(bEnableIcons);

	_ftprintf(h, _T("\n// Specify icons display size, 0 = 16x16 , 1 = 24x24, 2 = 32x32.\n"));
	VAR(nIconsSize);
	
	_ftprintf(h, _T("\n// Previous games list.\n"));
	STR(szPrevGames[0]);
	STR(szPrevGames[1]);
	STR(szPrevGames[2]);
	STR(szPrevGames[3]);
	STR(szPrevGames[4]);
	STR(szPrevGames[5]);
	STR(szPrevGames[6]);
	STR(szPrevGames[7]);
	STR(szPrevGames[8]);
	STR(szPrevGames[9]);
	
	_ftprintf(h, _T("\n// Player default controls, number is the index of the configuration in the input dialog\n"));
	VAR(nPlayerDefaultControls[0]);
	STR(szPlayerDefaultIni[0]);
	VAR(nPlayerDefaultControls[1]);
	STR(szPlayerDefaultIni[1]);
	VAR(nPlayerDefaultControls[2]);
	STR(szPlayerDefaultIni[2]);
	VAR(nPlayerDefaultControls[3]);
	STR(szPlayerDefaultIni[3]);

	_ftprintf(h, _T("\n\n\n"));

#undef DRV
#undef STR
#undef FLT
#undef VAR
#undef VAR64

	fclose(h);
	return 0;
}

