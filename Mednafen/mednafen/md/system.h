
#ifndef _SYSTEM_H_
#define _SYSTEM_H_

namespace MDFN_IEN_MD
{

/* Input devices */
#define MAX_DEVICES         (8)     /* Unsure of maximum */
#define DEVICE_2BUTTON      (0)     /* 2-button gamepad */
#define DEVICE_3BUTTON      (1)     /* 3-button gamepad */
#define DEVICE_6BUTTON      (2)     /* 6-button gamepad */

/* Input bitmasks */
#define INPUT_MODE      (0x00000800)
#define INPUT_Z         (0x00000400)
#define INPUT_Y         (0x00000200)
#define INPUT_X         (0x00000100)
#define INPUT_START     (0x00000080)
#define INPUT_C         (0x00000040)
#define INPUT_B         (0x00000020)
#define INPUT_A         (0x00000010)
#define INPUT_LEFT      (0x00000008)
#define INPUT_RIGHT     (0x00000004)
#define INPUT_DOWN      (0x00000002)
#define INPUT_UP        (0x00000001)

extern int32 md_timestamp;

void MD_ExitCPULoop(void);
void MD_Suspend68K(bool state);
bool MD_Is68KSuspended(void);


extern bool MD_IsCD;
extern int MD_HackyHackyMode;

}

#endif /* _SYSTEM_H_ */

