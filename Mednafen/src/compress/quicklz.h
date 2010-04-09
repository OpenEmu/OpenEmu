#ifndef __QUICKLZ_H
#define __QUICKLZ_H

#include <inttypes.h>

#ifdef __cplusplus
extern "C" {
#endif

uint32 qlz_compress(const void *source, char *destination, uint32 size);
uint32 qlz_decompress(const char *source, void *destination);

#ifdef __cplusplus
}
#endif

#endif
