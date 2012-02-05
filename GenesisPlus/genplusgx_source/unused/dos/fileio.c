#include "shared.h"

/*
    Load a normal file, or ZIP/GZ archive.
    Returns NULL if an error occured.
*/
uint8 *load_archive(char *filename, int *file_size)
{
    int size = 0;
    uint8 *buf = NULL;

    if(check_zip(filename))
    {
        unzFile *fd = NULL;
        unz_file_info info;
        int ret = 0;

        /* Attempt to open the archive */
        fd = unzOpen(filename);
        if(!fd) return (NULL);

        /* Go to first file in archive */
        ret = unzGoToFirstFile(fd);
        if(ret != UNZ_OK)
        {
            unzClose(fd);
            return (NULL);
        }

        ret = unzGetCurrentFileInfo(fd, &info, filename, 128, NULL, 0, NULL, 0);
        if(ret != UNZ_OK)
        {
            unzClose(fd);
            return (NULL);
        }

        /* Open the file for reading */
        ret = unzOpenCurrentFile(fd);
        if(ret != UNZ_OK)
        {
            unzClose(fd);
            return (NULL);
        }

        /* Allocate file data buffer */
        size = info.uncompressed_size;
        buf = malloc(size);
        if(!buf)
        {
            unzClose(fd);
            return (NULL);
        }

        /* Read (decompress) the file */
        ret = unzReadCurrentFile(fd, buf, info.uncompressed_size);
        if(ret != info.uncompressed_size)
        {
            free(buf);
            unzCloseCurrentFile(fd);
            unzClose(fd);
            return (NULL);
        }

        /* Close the current file */
        ret = unzCloseCurrentFile(fd);
        if(ret != UNZ_OK)
        {
            free(buf);
            unzClose(fd);
            return (NULL);
        }

        /* Close the archive */
        ret = unzClose(fd);
        if(ret != UNZ_OK)
        {
            free(buf);
            return (NULL);
        }

        /* Update file size and return pointer to file data */
        *file_size = size;
        return (buf);
    }
    else
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
    Verifies if a file is a ZIP archive or not.
    Returns: 1= ZIP archive, 0= not a ZIP archive
*/
int check_zip(char *filename)
{
    uint8 buf[2];
    FILE *fd = NULL;
    fd = fopen(filename, "rb");
    if(!fd) return (0);
    fread(buf, 2, 1, fd);
    fclose(fd);
    if(memcmp(buf, "PK", 2) == 0) return (1);
    return (0);
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
