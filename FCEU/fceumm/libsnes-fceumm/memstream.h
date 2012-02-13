#ifndef __MEMSTREAM_H
#define __MEMSTREAM_H

#include <stddef.h>
#include <stdint.h>
#ifndef _MSC_VER
#include <stdbool.h>
#else
#define TRUE 1
#define FALSE 0
typedef unsigned char bool;
#endif

typedef struct memstream memstream_t;

memstream_t *memstream_open(bool writing);
void memstream_close(memstream_t *stream);

size_t memstream_read(memstream_t *stream, void *data, size_t bytes);
size_t memstream_write(memstream_t *stream, const void *data, size_t bytes);
int memstream_getc(memstream_t *stream);
void memstream_putc(memstream_t *stream, int c);
char *memstream_gets(memstream_t *stream, char *buffer, size_t len);
size_t memstream_pos(memstream_t *stream);
int memstream_seek(memstream_t *stream, int offset, int whence);

void memstream_set_buffer(uint8_t *buffer, size_t size);
size_t memstream_get_last_size(void);

#endif
