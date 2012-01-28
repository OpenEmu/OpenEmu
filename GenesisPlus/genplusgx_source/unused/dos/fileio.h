#ifndef _FILEIO_H_
#define _FILEIO_H_

/* Global variables */
extern int cart_size;
extern char cart_name[0x100];

/* Function prototypes */
uint8 *load_archive(char *filename, int *file_size);
int load_cart(char *filename);
int check_zip(char *filename);
int gzsize(gzFile *gd);

#endif /* _FILEIO_H_ */
