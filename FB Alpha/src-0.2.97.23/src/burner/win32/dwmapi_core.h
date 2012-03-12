#include <uxtheme.h>

#define DWM_EC_DISABLECOMPOSITION         0
#define DWM_EC_ENABLECOMPOSITION          1
#ifndef DWMAPI
#define DWMAPI        STDAPI
#define DWMAPI_(type) STDAPI_(type)
#endif

typedef ULONGLONG DWM_FRAME_COUNT;

typedef  struct _UNSIGNED_RATIO
{
    UINT32 uiNumerator;
    UINT32 uiDenominator;
} UNSIGNED_RATIO;

typedef enum 
{
    // Use the first source frame that 
    // includes the first refresh of the output frame
    DWM_SOURCE_FRAME_SAMPLING_POINT,

    // use the source frame that includes the most 
    // refreshes of out the output frame
    // in case of multiple source frames with the 
    // same coverage the last will be used
    DWM_SOURCE_FRAME_SAMPLING_COVERAGE,

       // Sentinel value
    DWM_SOURCE_FRAME_SAMPLING_LAST
} DWM_SOURCE_FRAME_SAMPLING;

typedef struct _DWM_PRESENT_PARAMETERS {
  UINT32                    cbSize;
  BOOL                      fQueue;
  DWM_FRAME_COUNT           cRefreshStart;
  UINT                      cBuffer;
  BOOL                      fUseSourceRate;
  UNSIGNED_RATIO            rateSource;
  UINT                      cRefreshesPerFrame;
  DWM_SOURCE_FRAME_SAMPLING eSampling;
} DWM_PRESENT_PARAMETERS;

//#if defined (_MARGINS)
//#else
//typedef struct _MARGINS
//{
//    int cxLeftWidth;
//    int cxRightWidth;
//    int cyTopHeight;
//    int cyBottomHeight;
//} MARGINS;
//#endif

typedef enum _DWMWINDOWATTRIBUTE {
  DWMWA_NCRENDERING_ENABLED	= 1,
  DWMWA_NCRENDERING_POLICY,
  DWMWA_TRANSITIONS_FORCEDISABLED,
  DWMWA_ALLOW_NCPAINT,
  DWMWA_CAPTION_BUTTON_BOUNDS,
  DWMWA_NONCLIENT_RTL_LAYOUT,
  DWMWA_FORCE_ICONIC_REPRESENTATION,
  DWMWA_FLIP3D_POLICY,
  DWMWA_EXTENDED_FRAME_BOUNDS,
  DWMWA_HAS_ICONIC_BITMAP,
  DWMWA_DISALLOW_PEEK,
  DWMWA_EXCLUDED_FROM_PEEK,
  DWMWA_LAST 
} DWMWINDOWATTRIBUTE;

typedef enum _DWMNCRENDERINGPOLICY {
  DWMNCRP_USEWINDOWSTYLE,
  DWMNCRP_DISABLED,
  DWMNCRP_ENABLED,
  DWMNCRP_LAST 
} DWMNCRENDERINGPOLICY;

extern HRESULT (WINAPI *DwmEnableComposition)			(UINT uCompositionAction);
extern HRESULT (WINAPI *DwmSetDxFrameDuration)			(HWND hwnd, INT cRefreshes);
extern HRESULT (WINAPI *DwmSetPresentParameters)		(HWND hwnd, DWM_PRESENT_PARAMETERS *pPresentParams);
extern HRESULT (WINAPI *DwmExtendFrameIntoClientArea)	(HWND hwnd, const MARGINS* pMarInset);
extern HRESULT (WINAPI *DwmSetWindowAttribute)			(HWND hwnd, DWORD dwAttribute, LPCVOID pvAttribute, DWORD cbAttribute);
extern HRESULT (WINAPI *DwmIsCompositionEnabled)		(BOOL*);

void ExitDWMAPI();
void GetDllFunctions();
int InitDWMAPI();
void ExtendIntoClientAll(HWND);
BOOL IsWindows7();
void DWM_StutterFix();
BOOL IsCompositeOn();

extern bool bVidDWMCore;
