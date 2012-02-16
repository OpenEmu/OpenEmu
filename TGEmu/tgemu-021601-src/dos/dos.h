
#ifndef _DOS_H_
#define _DOS_H_

/* Function prototypes */
int init_machine(void);
void trash_machine(void);
void make_332_palette(void);
int check_key(int code);
void dump_sprites(void);
void tile_view(void);
void sprite_view(void);
void disp_view(void);
void update_input(void);
void dos_update_audio(void);
void dos_update_video(void);
void tick_handler(void);
void change_display(int w, int h);

#endif /* _DOS_H_ */
