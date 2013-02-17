#include "shared.h"
#define CHUNKSIZE   (0x10000)

/*
    Load a normal file, or ZIP/GZ archive.
    Returns NULL if an error occured.
*/
int load_archive(char *filename, unsigned char *buffer, int maxsize)
{
    int size = 0;
    char in[CHUNKSIZE];
    char msg[64] = "Unable to open file";
    
    /* Open file */
    FILE *fd = fopen(filename, "rb");
    
    /* Mega CD BIOS are required files */
    if (!strcmp(filename,CD_BIOS_US) || !strcmp(filename,CD_BIOS_EU) || !strcmp(filename,CD_BIOS_JP)) 
    {
        sprintf(msg,"Unable to open CD BIOS");
    }
    
    if (!fd)
    {
        fprintf(stderr, "ERROR - %s.\n");
        return 0;
    }
    
    /* Read first chunk */
    fread(in, CHUNKSIZE, 1, fd);
    
    {
        int left;
        /* Get file size */
        fseek(fd, 0, SEEK_END);
        size = ftell(fd);
        fseek(fd, 0, SEEK_SET);
        
        /* size limit */
        if(size > maxsize)
        {
            fclose(fd);
            fprintf(stderr, "ERROR - File is too large.\n");
            return 0;
        }
        
        sprintf((char *)msg,"Loading %d bytes ...", size);
        fprintf(stderr, "INFORMATION - %s\n", msg);
        
        /* Read into buffer */
        left = size;
        while (left > CHUNKSIZE)
        {
            fread(buffer, CHUNKSIZE, 1, fd);
            buffer += CHUNKSIZE;
            left -= CHUNKSIZE;
        }
        
        /* Read remaining bytes */
        fread(buffer, left, 1, fd);
    }
    
    /* Close file */
    fclose(fd);
    
    /* Return loaded ROM size */
    return size;
}