/* Header is not present in Windows CE SDK */

/* This stuff will live here until port configuration file is in place */
#define strdup _strdup

#ifndef _FILE_DEFINED
typedef void FILE;
#define _FILE_DEFINED
#endif
FILE *wce_fopen(const char *fname, const char *fmode);
#define fopen wce_fopen
