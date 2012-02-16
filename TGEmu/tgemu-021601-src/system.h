
#ifndef _SYSTEM_H_
#define _SYSTEM_H_

/* Input devices */
#define MAX_DEVICES         (5)     /* 1, or 5 with the MultiTap */
#define DEVICE_2BUTTON      (0)     /* Standard 2-button pad */
#define DEVICE_3BUTTON      (1)     /* 3-button pad */
#define DEVICE_6BUTTON      (2)     /* 6-button pad */
#define DEVICE_MULTITAP     (3)     /* MultiTap device */

/* Input bitmasks */
#define INPUT_B6        (0x00000800)    /* 6 button pad only */
#define INPUT_B5        (0x00000400)    /* 6 button pad only */
#define INPUT_B4        (0x00000200)    /* 6 button pad only */
#define INPUT_B3        (0x00000100)    /* 3/6 button pad only */
#define INPUT_B2        (0x00000080)
#define INPUT_B1        (0x00000040)
#define INPUT_RUN       (0x00000020)
#define INPUT_SELECT    (0x00000010)
#define INPUT_LEFT      (0x00000008)
#define INPUT_RIGHT     (0x00000004)
#define INPUT_DOWN      (0x00000002)
#define INPUT_UP        (0x00000001)

/* System input bitmasks */
#define SYSTEM_TGX      (0x00000001)    /* System is 1=TGX, 0=PCE */

typedef struct
{
    uint8 *data;        /* Bitmap data */
    int width;          /* Bitmap width (32+512+32) */
    int height;         /* Bitmap height (256) */
    int depth;          /* Color depth (8 bits) */
    int pitch;          /* Width of bitmap in bytes */
    int granularity;    /* Size of each pixel in bytes */
    struct {
        int x;          /* X offset of viewport within bitmap */
        int y;          /* Y offset of viewport within bitmap */
        int w;          /* Width of viewport */
        int h;          /* Height of viewport */
        int ow;         /* Previous width of viewport */
        int oh;         /* Previous height of viewport */
        int changed;    /* 1= Viewport width or height have changed */
    }viewport;
}t_bitmap;  

typedef struct
{
    uint8 dev[5];       /* Can be any of the DEVICE_* values */
    uint32 pad[5];      /* Can be any of the INPUT_* bitmasks */
    uint32 system;      /* Can be any of the SYSTEM_* bitmasks */
}t_input;

typedef struct
{
    int sample_rate;    /* Sample rate (8000-44100) */
    int enabled;        /* 1= sound emulation is enabled */
    int buffer_size;    /* Size of sound buffer (in bytes) */
    int16 *buffer[2];   /* Signed 16-bit stereo sound data */
}t_snd;

/* Global variables */
extern t_bitmap bitmap;
extern t_input input;
extern t_snd snd;

/* Function prototypes */
int system_init(int sample_rate);
void audio_init(int rate);
void system_frame(int skip);
void system_reset(void);
void system_shutdown(void);

#endif /* _SYSTEM_H_ */

