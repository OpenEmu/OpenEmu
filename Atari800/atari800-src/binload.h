#ifndef BINLOAD_H_
#define BINLOAD_H_

#include <stdio.h> /* FILE */
#include "atari.h" /* UBYTE */

extern FILE *BINLOAD_bin_file;

int BINLOAD_Loader(const char *filename);
extern int BINLOAD_start_binloading;
extern int BINLOAD_loading_basic;
#define BINLOAD_LOADING_BASIC_SAVED              1
#define BINLOAD_LOADING_BASIC_LISTED             2
#define BINLOAD_LOADING_BASIC_LISTED_ATARI       3
#define BINLOAD_LOADING_BASIC_LISTED_CR          4
#define BINLOAD_LOADING_BASIC_LISTED_LF          5
#define BINLOAD_LOADING_BASIC_LISTED_CRLF        6
#define BINLOAD_LOADING_BASIC_LISTED_CR_OR_CRLF  7
#define BINLOAD_LOADING_BASIC_RUN                8
int BINLOAD_LoaderStart(UBYTE *buffer);

#endif /* BINLOAD_H_ */
