#ifndef __PCFX_PCFX_H
#define __PCFX_PCFX_H

#include "../types.h"
#include "../mednafen.h"
#include "../state.h"
#include "../general.h"
#include "../memory.h"

#if defined(HAVE_MMAP) && defined(HAVE_MUNMAP) && SIZEOF_SIZE_T > 4 && SIZEOF_VOID_P > 4
#define WANT_PCFX_MMAP
#else

#endif

#include "v810_cpu.h"
#include "vdc.h"

uint8 mem_rbyte(const uint32 A);
uint16 mem_rhword(const uint32 A);
uint32 mem_rword(const uint32 A);
void mem_wbyte(const uint32 A, const uint8 V);
void mem_whword(const uint32 A, const uint16 V);
void mem_wword(const uint32 A, const uint32 V);

uint8 port_rbyte(uint32 A);
uint16 port_rhword(uint32 A);
uint32 port_rword(uint32 A);
void port_wbyte(uint32 A, uint8 V);
void port_whword(uint32 A, uint16 V);
void port_wword(uint32 A, uint32 V);

extern fx_vdc_t *fx_vdc_chips[2];
extern int64 pcfx_timestamp_base;

#define REGSETHW(_reg, _data, _msh) { _reg &= 0xFFFF << (_msh ? 0 : 16); _reg |= _data << (_msh ? 16 : 0); }
#define REGGETHW(_reg, _msh) ((_reg >> (_msh ? 16 : 0)) & 0xFFFF)

#endif
