#ifndef __MDFN_ENDIAN_H
#define __MDFN_ENDIAN_H

int write16le(uint16 b, FILE *fp);
int write32le(uint32 b, FILE *fp);
int read32le(uint32 *Bufo, FILE *fp);

void Endian_A16_NE_to_LE(void *src, uint32 nelements);
void Endian_A32_NE_to_LE(void *src, uint32 nelements);
void Endian_A16_LE_to_NE(void *src, uint32 nelements);
void Endian_A16_BE_to_NE(void *src, uint32 nelements);
void Endian_A32_LE_to_NE(void *src, uint32 nelements);

void Endian_V_LE_to_NE(void *src, uint32 bytesize);
void Endian_V_NE_to_LE(void *src, uint32 bytesize);

void FlipByteOrder(uint8 *src, uint32 count);

void MDFN_en32lsb(uint8 *, uint32);
uint32 MDFN_de32lsb(uint8 *);
uint32 MDFN_de32msb(uint8 *);

uint16 MDFN_de16lsb(uint8 *);

#endif
