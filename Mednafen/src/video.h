#include "video-driver.h"

void MDFN_ResetMessages(void);
void MDFN_InitFontData(void);
void MDFN_DispMessage(const char *format, ...) __attribute__ ((format (printf, 1, 2)));

int MDFN_InitVirtualVideo(void);
void MDFN_KillVirtualVideo(void);
int MDFN_SavePNGSnapshot(const char *fname, uint32 *fb, const MDFN_Rect *rect, uint32 pitch);

static inline void DECOMP_COLOR(int c, int &r, int &g, int &b)
{
 r = (c >> FSettings.rshift) & 0xFF;
 g = (c >> FSettings.gshift) & 0xFF;
 b = (c >> FSettings.bshift) & 0xFF;
}

#define MK_COLOR(r,g,b) ( ((r)<<FSettings.rshift) | ((g) << FSettings.gshift) | ((b) << FSettings.bshift))
#define MK_COLORA(r,g,b, a) ( ((r)<<FSettings.rshift) | ((g) << FSettings.gshift) | ((b) << FSettings.bshift) | ((a) << FSettings.ashift))

