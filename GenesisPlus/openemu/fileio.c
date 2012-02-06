#include "shared.h"
#include <zlib.h>

static int gzsize(gzFile *gd);

/*
    Load a normal file, or ZIP/GZ archive.
    Returns NULL if an error occured.
*/
uint8 *load_archive(char *filename, int *file_size)
{
    int size = 0;
    uint8 *buf = NULL;

    {
        gzFile *gd = NULL;

        /* Open file */
        gd = gzopen(filename, "rb");
        if(!gd) return (0);

        /* Get file size */
        size = gzsize(gd);

        /* Allocate file data buffer */
        buf = malloc(size);
        if(!buf)
        {
            gzclose(gd);
            return (0);
        }

        /* Read file data */
        gzread(gd, buf, size);

        /* Close file */
        gzclose(gd);

        /* Update file size and return pointer to file data */
        *file_size = size;
        return (buf);
    }
}

/*
    Returns the size of a GZ compressed file.
*/
int gzsize(gzFile *gd)
{
    #define CHUNKSIZE   (0x10000)
    int size = 0, length = 0;
    unsigned char buffer[CHUNKSIZE];
    gzrewind(gd);
    do {
        size = gzread(gd, buffer, CHUNKSIZE);
        if(size <= 0) break;
        length += size;
    } while (!gzeof(gd));
    gzrewind(gd);
    return (length);
    #undef CHUNKSIZE
}
