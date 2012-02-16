
#include "shared.h"
#include "pcecrc.h"
#include <zlib.h>

//static int gzsize(gzFile *gd);

/* Name of the loaded file */
char game_name[0x100];

/* split : 1= Split image (only needed for 512k versions of 384k images)
   flip  : 1= Bit-flip image (only for some TurboGrafx-16 images) */
int load_rom(char *filename, int split, int flip)
{
    #include "bitflip.h"
    uint8 header[0x200];
    uint8 *ptr = NULL, *buf = NULL;
    uint32 crc;
    int size, n;

    /* Default */
    strcpy(game_name, filename);

    {
        gzFile *gd = NULL;

        /* Open file */
        gd = gzopen(filename, "rb");
        if(!gd) return (0);

        /* Get file size */
        size = gzsize(gd);

        /* Allocate file data buffer */
        buf = malloc(size);
        if(!buf) {
            gzclose(gd);
            return (0);
        }

        /* Read file data */
        gzread(gd, buf, size);

        /* Close file */
        gzclose(gd);
    }

    /* Check for 512-byte header */
    ptr = buf;
    if((size / 512) & 1)
    {
        memcpy(header, buf, 0x200);
        size -= 0x200;
        buf += 0x200;
    }

    /* Generate CRC and print information */
    crc = crc32(0, buf, size);

    /* Look up game CRC in the CRC database, and set up flip and
       split options accordingly */
    for(n = 0; n < (sizeof(pcecrc_list) / sizeof(t_pcecrc)); n += 1)
    {
        if(crc == pcecrc_list[n].crc)
        {
            if(pcecrc_list[n].flag & FLAG_BITFLIP) flip = 1;
            if(pcecrc_list[n].flag & FLAG_SPLIT) split = 1;
        }
    }

    /* Bit-flip image */
    if(flip)
    {
        uint8 temp;
        int count;

        for(count = 0; count < size; count += 1)
        {
            temp = buf[count];
            buf[count] = bitflip[temp];
        }
    }

    /* Always split 384K images */
    if(size == 0x60000)
    {
        memcpy(rom + 0x00000, buf + 0x00000, 0x40000);
        memcpy(rom + 0x80000, buf + 0x40000, 0x20000);
    }
    else /* Split 512K images if requested */
    if(split && (size == 0x80000))
    {
        memcpy(rom + 0x00000, buf + 0x00000, 0x40000);
        memcpy(rom + 0x80000, buf + 0x40000, 0x40000);
    }
    else
    {
        memcpy(rom, buf, (size > 0x100000) ? 0x100000 : size);
    }

    /* Free allocated memory and exit */
    free(ptr);

#ifdef DOS
    /* I need Allegro to handle this... */
    //strcpy(game_name, get_filename(game_name));
#endif

    return (1);
}


int file_exist(char *filename)
{
    FILE *fd = fopen(filename, "rb");
    if(!fd) return (0);
    fclose(fd);
    return (1);
}


int load_file(char *filename, char *buf, int size)
{
    FILE *fd = fopen(filename, "rb");
    if(!fd) return (0);
    fread(buf, size, 1, fd);
    fclose(fd);
    return (1);
}


int save_file(char *filename, char *buf, int size)
{
    FILE *fd = NULL;
    if(!(fd = fopen(filename, "wb"))) return (0);
    fwrite(buf, size, 1, fd);
    fclose(fd);
    return (1);
}

/* Because gzio.c doesn't work */
int gzsize(gzFile *gd)
{
    #define CHUNKSIZE   0x10000
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

