#ifndef __FCEU_ENDIAN
#define __FCEU_ENDIAN

#include <iosfwd>
#include <stdio.h>
#include "../types.h"
#include "../emufile.h"

class EMUFILE;

inline uint64 double_to_u64(double d) {
	union {
		uint64 a;
		double b;
	} fuxor;
	fuxor.b = d;
	return fuxor.a;
}

inline double u64_to_double(uint64 u) {
	union {
		uint64 a;
		double b;
	} fuxor;
	fuxor.a = u;
	return fuxor.b;
}

inline uint32 float_to_u32(float f) {
	union {
		uint32 a;
		float b;
	} fuxor;
	fuxor.b = f;
	return fuxor.a;
}

inline float u32_to_float(uint32 u) {
	union {
		uint32 a;
		float b;
	} fuxor;
	fuxor.a = u;
	return fuxor.b;
}


int write16le(uint16 b, FILE *fp);
int write32le(uint32 b, FILE *fp);
int write32le(uint32 b, std::ostream* os);
int write64le(uint64 b, std::ostream* os);
int read64le(uint64 *Bufo, std::istream *is);
int read32le(uint32 *Bufo, std::istream *is);
int read32le(uint32 *Bufo, FILE *fp);
int read16le(uint16 *Bufo, std::istream *is);

void FlipByteOrder(uint8 *src, uint32 count);

void FCEU_en32lsb(uint8 *, uint32);
void FCEU_en16lsb(uint8* buf, uint16 val);
uint64 FCEU_de64lsb(uint8 *morp);
uint32 FCEU_de32lsb(uint8 *morp);
uint16 FCEU_de16lsb(uint8 *morp);

//well. just for the sake of consistency
int write8le(uint8 b, EMUFILE *fp);
inline int write8le(uint8* b, EMUFILE *fp) { return write8le(*b,fp); }
int write16le(uint16 b, EMUFILE* os);
int write32le(uint32 b, EMUFILE* os);
int write64le(uint64 b, EMUFILE* os);
inline int write_double_le(double b, EMUFILE*is) { uint64 temp = double_to_u64(b); int ret = write64le(temp,is); return ret; }

int read8le(uint8 *Bufo, EMUFILE*is);
int read16le(uint16 *Bufo, EMUFILE*is);
inline int read16le(int16 *Bufo, EMUFILE*is) { return read16le((uint16*)Bufo,is); }
int read32le(uint32 *Bufo, EMUFILE*is);
inline int read32le(int32 *Bufo, EMUFILE*is) { return read32le((uint32*)Bufo,is); }
int read64le(uint64 *Bufo, EMUFILE*is);
inline int read_double_le(double *Bufo, EMUFILE*is) { uint64 temp; int ret = read64le(&temp,is); *Bufo = u64_to_double(temp); return ret; }


template<typename T>
int readle(T *Bufo, EMUFILE *is)
{
	CTASSERT(sizeof(T)==1||sizeof(T)==2||sizeof(T)==4||sizeof(T)==8);
	switch(sizeof(T)) {
		case 1: return read8le((uint8*)Bufo,is);
		case 2: return read16le((uint16*)Bufo,is);
		case 4: return read32le((uint32*)Bufo,is);
		case 8: return read64le((uint64*)Bufo,is);
		default:
			return 0;
	}
}

template<typename T>
int writele(T *Bufo, EMUFILE *os)
{
	CTASSERT(sizeof(T)==1||sizeof(T)==2||sizeof(T)==4||sizeof(T)==8);
	switch(sizeof(T)) {
		case 1: return write8le(*(uint8*)Bufo,os);
		case 2: return write16le(*(uint16*)Bufo,os);
		case 4: return write32le(*(uint32*)Bufo,os);
		case 8: return write64le(*(uint64*)Bufo,os);
		default:
			return 0;
	}
}

#endif //__FCEU_ENDIAN

