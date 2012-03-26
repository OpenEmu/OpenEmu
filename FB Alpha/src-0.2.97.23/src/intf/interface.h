#ifdef FBA_DEBUG
 #define PRINT_DEBUG_INFO
#endif

// GameInp structure
#include "gameinp.h"
// Key codes
#include "inp_keys.h"

// Interface info (used for all modules)
struct InterfaceInfo {
	const TCHAR* pszModuleName;
	TCHAR** ppszInterfaceSettings;
	TCHAR** ppszModuleSettings;
};

INT32 IntInfoFree(InterfaceInfo* pInfo);
INT32 IntInfoInit(InterfaceInfo* pInfo);
INT32 IntInfoAddStringInterface(InterfaceInfo* pInfo, TCHAR* szString);
INT32 IntInfoAddStringModule(InterfaceInfo* pInfo, TCHAR* szString);

// Input plugin:
struct InputInOut {
	INT32   (*Init)();
	INT32   (*Exit)();
	INT32   (*SetCooperativeLevel)(bool bExclusive, bool bForeground);
	// Setup new frame
	INT32   (*NewFrame)();
	// Read digital
	INT32   (*ReadSwitch)(INT32 nCode);
	// Read analog
	INT32   (*ReadJoyAxis)(INT32 i, INT32 nAxis);
	INT32   (*ReadMouseAxis)(INT32 i, INT32 nAxis);
	// Find out which control is activated
	INT32   (*Find)(bool CreateBaseline);
	// Get the name of a control
	INT32   (*GetControlName)(INT32 nCode, TCHAR* pszDeviceName, TCHAR* pszControlName);
	// Get plugin info
	INT32   (*GetPluginSettings)(InterfaceInfo* pInfo);
	const TCHAR* szModuleName;
};

INT32 InputInit();
INT32 InputExit();
INT32 InputSetCooperativeLevel(const bool bExclusive, const bool bForeGround);
INT32 InputMake(bool bCopy);
INT32 InputFind(const INT32 nFlags);
INT32 InputGetControlName(INT32 nCode, TCHAR* pszDeviceName, TCHAR* pszControlName);
InterfaceInfo* InputGetInfo();

extern bool bInputOkay;
extern UINT32 nInputSelect;

// CD emulation module

struct CDEmuDo {
	INT32			   (*CDEmuExit)();
	INT32			   (*CDEmuInit)();
	INT32			   (*CDEmuStop)();
	INT32			   (*CDEmuPlay)(UINT8 M, UINT8 S, UINT8 F);
	INT32			   (*CDEmuLoadSector)(INT32 LBA, char* pBuffer);
	UINT8* (*CDEmuReadTOC)(INT32 track);
	UINT8* (*CDEmuReadQChannel)();
	INT32			   (*CDEmuGetSoundBuffer)(INT16* buffer, INT32 samples);
	// Get plugin info
	INT32			   (*GetPluginSettings)(InterfaceInfo* pInfo);
	const TCHAR*	 szModuleName;
};

#include "cd_interface.h"

InterfaceInfo* CDEmuGetInfo();

extern bool bCDEmuOkay;
extern UINT32 nCDEmuSelect;

extern CDEmuStatusValue CDEmuStatus;

// Profiling plugin
struct ProfileDo {
	INT32    (*ProfileExit)();
	INT32    (*ProfileInit)();
	INT32    (*ProfileStart)(INT32 nSubSystem);
	INT32    (*ProfileEnd)(INT32 nSubSystem);
	double (*ProfileReadLast)(INT32 nSubSystem);
	double (*ProfileReadAverage)(INT32 nSubSystem);
	// Get plugin info
	INT32    (*GetPluginSettings)(InterfaceInfo* pInfo);
	const  TCHAR* szModuleName;
};

extern bool bProfileOkay;
extern UINT32 nProfileSelect;

INT32 ProfileInit();
INT32 ProfileExit();
INT32 ProfileProfileStart(INT32 nSubSystem);
INT32 ProfileProfileEnd(INT32 nSubSustem);
double ProfileProfileReadLast(INT32 nSubSustem);
double ProfileProfileReadAverage(INT32 nSubSustem);
InterfaceInfo* ProfileGetInfo();

// Audio Output plugin
struct AudOut {
	INT32   (*BlankSound)();
	INT32   (*SoundCheck)();
	INT32   (*SoundInit)();
	INT32   (*SetCallback)(INT32 (*pCallback)(INT32));
	INT32   (*SoundPlay)();
	INT32   (*SoundStop)();
	INT32   (*SoundExit)();
	INT32   (*SoundSetVolume)();
	// Get plugin info
	INT32   (*GetPluginSettings)(InterfaceInfo* pInfo);
	const TCHAR* szModuleName;
};

INT32 AudSelect(UINT32 nPlugIn);
INT32 AudSoundInit();
INT32 AudSoundExit();
INT32 AudSetCallback(INT32 (*pCallback)(INT32));
INT32 AudSoundPlay();
INT32 AudSoundStop();
INT32 AudBlankSound();
INT32 AudSoundCheck();
INT32 AudSoundSetVolume();
InterfaceInfo* AudGetInfo();
void AudWriteSilence();

extern INT32 nAudSampleRate[8];          // sample rate
extern INT32 nAudVolume;				// Sound volume (% * 100)
extern INT32 nAudSegCount;          	// Segs in the pdsbLoop buffer
extern INT32 nAudSegLen;            	// Seg length in samples (calculated from Rate/Fps)
extern INT32 nAudAllocSegLen;
extern INT16 *nAudNextSound;       	// The next sound seg we will add to the sample loop
extern UINT8 bAudOkay;    	// True if DSound was initted okay
extern UINT8 bAudPlaying;	// True if the Loop buffer is playing
extern INT32 nAudDSPModule[8];			// DSP module to use: 0 = none, 1 = low-pass filter
extern UINT32 nAudSelect;

// Video Output plugin:
struct VidOut {
	INT32   (*Init)();
	INT32   (*Exit)();
	INT32   (*Frame)(bool bRedraw);
	INT32   (*Paint)(INT32 bValidate);
	INT32   (*ImageSize)(RECT* pRect, INT32 nGameWidth, INT32 nGameHeight);
	// Get plugin info
	INT32   (*GetPluginSettings)(InterfaceInfo* pInfo);
	const TCHAR* szModuleName;
};

INT32 VidSelect(UINT32 nPlugin);
INT32 VidInit();
INT32 VidExit();
INT32 VidReInitialise();
INT32 VidFrame();
INT32 VidRedraw();
INT32 VidRecalcPal();
INT32 VidPaint(INT32 bValidate);
INT32 VidImageSize(RECT* pRect, INT32 nGameWidth, INT32 nGameHeight);
const TCHAR* VidGetModuleName();
InterfaceInfo* VidGetInfo();

#ifdef BUILD_WIN32
 extern HWND hVidWnd;
#endif

#if defined (_XBOX)
 extern HWND hVidWnd;
#endif

extern bool bVidOkay;
extern UINT32 nVidSelect;
extern INT32 nVidWidth, nVidHeight, nVidDepth, nVidRefresh;

extern INT32 nVidHorWidth, nVidHorHeight;
extern INT32 nVidVerWidth, nVidVerHeight;

extern INT32 nVidFullscreen;
extern INT32 bVidBilinear;
extern INT32 bVidScanlines;
extern INT32 bVidScanRotate;
extern INT32 bVidScanBilinear;
extern INT32 nVidScanIntensity;
extern INT32 bVidScanHalf;
extern INT32 bVidScanDelay;
extern INT32 nVidFeedbackIntensity;
extern INT32 nVidFeedbackOverSaturation;
extern INT32 bVidCorrectAspect;
extern INT32 bVidArcaderes;

extern INT32 bVidArcaderesHor;
extern INT32 bVidArcaderesVer;

extern INT32 nVidRotationAdjust;
extern INT32 bVidUseHardwareGamma;
extern INT32 bVidAutoSwitchFull;
extern INT32 bVidForce16bit;
extern INT32 bVidForceFlip;
extern INT32 nVidTransferMethod;
extern float fVidScreenAngle;
extern float fVidScreenCurvature;
extern INT64 nVidBlitterOpt[];
extern INT32 bVidFullStretch;
extern INT32 bVidTripleBuffer;
extern INT32 bVidVSync;
extern double dVidCubicB;
extern double dVidCubicC;
extern INT32 bVidDX9Bilinear;
extern INT32 bVidHardwareVertex;
extern INT32 bVidMotionBlur;
extern INT32 nVidScrnWidth, nVidScrnHeight;
extern INT32 nVidScrnDepth;

extern INT32 nVidScrnAspectX, nVidScrnAspectY;

extern UINT8* pVidImage;
extern INT32 nVidImageWidth, nVidImageHeight;
extern INT32 nVidImageLeft, nVidImageTop;
extern INT32 nVidImagePitch, nVidImageBPP;
extern INT32 nVidImageDepth;

extern "C" UINT32 (__cdecl *VidHighCol) (INT32 r, INT32 g, INT32 b, INT32 i);

// vid_directx_support.cpp

INT32 VidSNewTinyMsg(const TCHAR* pText, INT32 nRGB = 0, INT32 nDuration = 0, INT32 nPiority = 5);

INT32 VidSNewShortMsg(const TCHAR* pText, INT32 nRGB = 0, INT32 nDuration = 0, INT32 nPiority = 5);
void VidSKillShortMsg();

INT32 VidSAddChatMsg(const TCHAR* pID, INT32 nIDRGB, const TCHAR* pMain, INT32 nMainRGB);

#define MAX_CHAT_SIZE (128)

extern INT32 nVidSDisplayStatus;
extern INT32 nMaxChatFontSize;
extern INT32 nMinChatFontSize;
extern bool bEditActive;
extern bool bEditTextChanged;
extern TCHAR EditText[MAX_CHAT_SIZE + 1];

// osd text display for dx9
extern TCHAR OSDMsg[MAX_PATH];
extern UINT32 nOSDTimer;
void VidSKillOSDMsg();
