
#ifndef _DOS_H_
#define _DOS_H_

#define LOGERROR 1

/* Function prototypes */
int load_file(char *filename, char *buf, int size);
int save_file(char *filename, char *buf, int size);
void dos_update_input(void);
void dos_update_audio(void);
void dos_update_palette(void);
void dos_update_video(void);
void init_machine(void);
void trash_machine(void);
void make_vdp_palette(void);
void dos_change_mode(void);
int check_key(int code);

#define MAX_INPUTS 8

extern uint8 debug_on;
extern uint8 log_error;

#endif /* _DOS_H_ */

