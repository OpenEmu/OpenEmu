#ifndef _FILEIO_H_
#define _FILEIO_H_



//#include <zlib.h>
/* Global variables */
extern int cart_size;
extern char cart_name[0x100];

/* Function prototypes */
int load_archive(char *filename, unsigned char *buffer, int maxsize);
uint8 *load_archive(char *filename, int *file_size);
int load_cart(char *filename);

#endif /* _FILEIO_H_ */
