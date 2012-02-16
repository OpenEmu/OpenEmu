
#ifndef _WAVE_H_
#define _WAVE_H_

/* Macros and defines */
typedef struct
{
    uint8 riff[4];
    uint32 file_size;
    uint8 wave[4];
    uint8 fmt[4];
    uint32 fmt_chunk_size;
    uint16 format_type;
    uint16 channel_count;
    uint32 samples_per_second;
    uint32 average_bytes;
    uint16 block_align;
    uint16 bits_per_sample;
    uint8 data[4];
    uint32 data_size;
} t_wave_header;

typedef struct
{
    FILE *handle;
    int enable;
    int sample_rate;
    int buffer_size;
    int wavebuf_size;
    int16 *buffer;
    t_wave_header header;
} t_wave;

/* Global variables */
extern t_wave_header wave_header;

/* Function prototypes */
int wave_init(t_wave *p, char *filename, int sample_rate, int fps);
void wave_shutdown(t_wave *p);
void wave_update(t_wave *p, int16 **buf);

#endif /* _WAVE_H_ */

