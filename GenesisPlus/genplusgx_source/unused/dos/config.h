
#ifndef _CONFIG_H_
#define _CONFIG_H_

#define PATH_SIZE       (0x100)
#define TOKEN_LIST_SIZE (0x20)

typedef struct
{
    char *token;
    int value;
}t_strint;

typedef struct
{
    int video_width;
    int video_height;
    int video_depth;
    int video_driver;

    int autores;
    int autores_w;
    int autores_h;

    int remap;
    int blur;
    int scanlines;
    int scale;

    int vsync;
    int throttle;
    int fps;
    int skip;

    int sound;
    int sndcard;
    int sndrate;
    int swap;

    int wave;
    char wavpath[PATH_SIZE];

    int joy_driver;

    int split;              /* 1= Split image at 2 megabit boundary */
    int flip;               /* 1= Bit-flip image */
    int usa;                /* 1= Set D6 of $1000 to indicate US machine */
    int softres;            /* 1= Allow RUN+SELECT */
} t_option;

/****************************************************************************
 * Config Option 
 *
 ****************************************************************************/
typedef struct 
{
  uint8 padtype;
} t_input_c;

typedef struct 
{
  uint8 hq_fm;
  uint8 psgBoostNoise;
  int32 psg_preamp;
  int32 fm_preamp;
  uint8 filter;
  uint16 low_freq;
  uint16 high_freq;
  uint8 lp_range;
  float lg;
  float mg;
  float hg;
  float rolloff;
  uint8 region_detect;
  uint8 force_dtack;
  uint8 addr_error;
  uint8 tmss;
  uint8 lock_on;
  uint8 romtype;
  uint8 overscan;
  uint8 render;
  uint8 ntsc;
  t_input_c input[MAX_INPUTS];
  uint8 gun_cursor[2];
  uint8 invert_mouse;
} t_config;

/* Global variables */
extern t_option option;
extern t_config config;
extern t_strint video_driver_table[];
extern t_strint joy_driver_table[];

/* Function prototypes */
void do_config(char *file);
int parse_file(char *filename, int *argc, char **argv);
void parse_args(int argc, char **argv);
void print_options(void);
int check_bool(char *token);
void set_option_defaults(void);
void set_config_defaults(void);

#endif /* _CONFIG_H_ */

