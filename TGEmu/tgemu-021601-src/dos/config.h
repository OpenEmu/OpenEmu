
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

/* Global variables */
extern t_option option;
extern t_strint video_driver_table[];
extern t_strint joy_driver_table[];

/* Function prototypes */
void do_config(char *file);
int parse_file(char *filename, int *argc, char **argv);
void parse_args(int argc, char **argv);
void print_options(void);
int check_bool(char *token);
void set_option_defaults(void);

#endif /* _CONFIG_H_ */

