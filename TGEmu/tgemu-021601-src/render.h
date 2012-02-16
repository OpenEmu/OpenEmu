
#ifndef _RENDER_H_
#define _RENDER_H_

#define FLAG_ENABLE         0x80    /* 1= Sprite is enabled */
#define FLAG_PRIORITY       0x01    /* 1= Sprite behind background */
#define FLAG_CGX            0x02    /* 1= Sprite is two patterns wide */
#define FLAG_YFLIP          0x04    /* 1= Sprite is vertically flipped */

typedef struct
{
    int32 top;              /* 0x00 */
    int32 bottom;           /* 0x04 */
    int32 xpos;             /* 0x08 */
    uint32 name_left;       /* 0x0C */
    uint32 name_right;      /* 0x10 */
    uint32 height;          /* 0x14 */
    uint8 palette;          /* 0x18 */
    uint8 flags;            /* 0x19 */
    uint8 filler[6];        /* 0x1A */           
} t_sprite;

/* Global data */
extern int plane_enable;
extern uint8 *xlat[2];
extern uint16 pixel[2][0x100];
extern uint16 pixel_lut[0x200];
extern void (*render_line)(int line);
extern t_sprite sprite_list[0x40];
extern uint32 bp_lut[0x10000];
extern uint8 used_sprite_list[0x40];
extern uint8 used_sprite_index;

/* Function prototypes */
int render_init(void);
void render_reset(void);
void render_shutdown(void);
int make_sprite_list(void);
void update_bg_pattern_cache(void);
void update_obj_pattern_cache(void);
void render_line_8(int line);
void render_line_16(int line);
void render_bg_8(int line);
void render_bg_16(int line);
void render_obj_8(int line);
void render_obj_16(int line);

#endif /* _RENDER_H_ */

