#ifndef FCEU_ENDIAN_H
#define FCEU_ENDIAN_H

#include "types.h"

int write16le(uint16 b, FILE *fp);
int write32le(uint32 b, FILE *fp);

#ifdef __LIBSNES__
#include "../libsnes-fceumm/memstream.h"
int write32le_mem(uint32 b, memstream_t *mem);
int read32le_mem(uint32 *Bufo, memstream_t *mem);
#endif

int read32le(uint32 *Bufo, FILE *fp);
void FlipByteOrder(uint8 *src, uint32 count);

void FCEU_en32lsb(uint8 *, uint32);
uint32 FCEU_de32lsb(uint8 *);

#endif
