
#ifndef _SYSTEM_H_
#define _SYSTEM_H_

namespace MDFN_IEN_SMS
{

#define APP_NAME            "SMS Plus"
#define APP_VERSION         "1.2"

#define PALETTE_SIZE        0x20

/* Mask for removing unused pixel data */
#define PIXEL_MASK          0x1F

/* These can be used for 'input.pad[]' */
#define INPUT_UP            0x00000001
#define INPUT_DOWN          0x00000002
#define INPUT_LEFT          0x00000004
#define INPUT_RIGHT         0x00000008
#define INPUT_BUTTON2       0x00000010
#define INPUT_BUTTON1       0x00000020

/* These can be used for 'input.system' */
#define INPUT_START         0x00000001  /* Game Gear only */    
#define INPUT_PAUSE         0x00000002  /* Master System only */
#define INPUT_RESET         0x00000004  /* Master System only */

enum {
    SRAM_SAVE   = 0,
    SRAM_LOAD   = 1
};


/* User input structure */
typedef struct
{
    uint32 pad[2];
    uint8 analog[2];
    uint32 system;
} input_t;

/* Bitmap structure */
typedef struct
{
    unsigned char *data;
    int width;
    int height;
    int pitch;
} bitmap_t;

/* Global variables */
extern bitmap_t bitmap;     /* Display bitmap */
extern input_t input;       /* Controller input */

/* Function prototypes */
void system_frame(int skip_render);
void system_init(void);
void system_reset(void);
void system_manage_sram(uint8 *sram, int slot, int mode);
void system_poweron(void);
void system_poweroff(void);

}

#endif /* _SYSTEM_H_ */


