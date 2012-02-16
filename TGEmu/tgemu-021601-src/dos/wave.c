
#include "osd.h"


/* Default header for 16-bit signed stereo 44100Hz sound */
t_wave_header wave_header = {
    "RIFF", 0, "WAVE", "fmt ", 16, 1, 2, 0, 0, 4, 16, "data", 0
};


int wave_init(t_wave *p, char *filename, int sample_rate, int fps)
{
    /* Initialize wave structure */
    memset(p, 0, sizeof(t_wave));

    /* Calculate buffer data */
    p->sample_rate = sample_rate;
    p->buffer_size = (sample_rate / fps);
    p->wavebuf_size = (p->buffer_size * 2) * sizeof(int16);

    /* Allocate wave buffer */
    p->buffer = malloc(p->wavebuf_size);
    if(!p->buffer) {
        error("Couldn't allocate %d bytes.\n", p->wavebuf_size);
        return (0);
    }

    /* Open wave log file */
    p->handle = fopen(filename, "wb");
    if(!p->handle) {
        error("Couldn't open `%s' for writing.\n", filename);
        free(p->buffer);
        return (0);
    }

    /* Initialize header */
    memcpy(&p->header, &wave_header, sizeof(t_wave_header));
    fwrite(&p->header, sizeof(t_wave_header), 1, p->handle);

    /* Mark wave logging as enabled */
    p->enable = 1;

    return (1);
}


void wave_shutdown(t_wave *p)
{
    if(!p->enable) return;

    /* Seek to start of file */
    fseek(p->handle, 0, SEEK_SET);
        
    /* Update and rewrite header */
    p->header.file_size = p->header.data_size + sizeof(t_wave_header);
    p->header.samples_per_second = p->sample_rate;
    p->header.average_bytes = p->wavebuf_size;
    fwrite(&p->header, sizeof(t_wave_header), 1, p->handle);

    /* Seek to end of file and close file */
    fseek(p->handle, 0, SEEK_END);
    fclose(p->handle);

    /* Free sample buffer */
    free(p->buffer);
}


void wave_update(t_wave *p, int16 **buf)
{
    int count;

    if(!p->enable) return;

    /* Interleave sound samples into one buffer */
    for(count = 0; count < p->buffer_size; count += 1)
    {
        p->buffer[(count << 1)] = buf[1][count];
        p->buffer[(count << 1) | (1)] = buf[0][count];
    }

    /* Write sample data to disk */
    fwrite(p->buffer, p->wavebuf_size, 1, p->handle);

    /* Update current data chunk size */
    p->header.data_size += p->wavebuf_size;
}

