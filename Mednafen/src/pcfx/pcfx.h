#ifndef __PCFX_PCFX_H
#define __PCFX_PCFX_H

#include "../mednafen.h"
#include "../state.h"
#include "../general.h"

#define PCFX_MASTER_CLOCK	21477272.72

#if 0
 #define FXDBG(format, ...) MDFN_DebugPrint(format, ## __VA_ARGS__)
#else
 #define FXDBG(format, ...) ((void)0)
#endif

#include "v810/v810_cpu.h"
#include "huc6270/vdc.h"

extern V810 PCFX_V810;

uint8 MDFN_FASTCALL mem_peekbyte(const v810_timestamp_t timestamp, const uint32 A);
uint16 MDFN_FASTCALL mem_peekhword(const v810_timestamp_t timestamp, const uint32 A);

int32 MDFN_FASTCALL pcfx_event_handler(const v810_timestamp_t timestamp);

extern VDC *fx_vdc_chips[2];
extern int64 pcfx_timestamp_base;

#define REGSETHW(_reg, _data, _msh) { _reg &= 0xFFFF << (_msh ? 0 : 16); _reg |= _data << (_msh ? 16 : 0); }
#define REGGETHW(_reg, _msh) ((_reg >> (_msh ? 16 : 0)) & 0xFFFF)

enum
{
 PCFX_EVENT_PAD = 0,
 PCFX_EVENT_TIMER,
 PCFX_EVENT_KING,
 PCFX_EVENT_ADPCM
};

#define PCFX_EVENT_NONONO       0x7fffffff

void PCFX_SetEvent(const int type, const v810_timestamp_t next_timestamp);


#endif
