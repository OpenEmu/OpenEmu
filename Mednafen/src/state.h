#ifndef _STATE_H
#define _STATE_H

//#include <stdio.h>
#include <zlib.h>

#include "state-driver.h"

int MDFNSS_Save(const char *, const char *suffix, uint32 *fb = (uint32 *)NULL, MDFN_Rect *LineWidths = (MDFN_Rect *)NULL);
int MDFNSS_Load(const char *, const char *suffix);
int MDFNSS_SaveFP(gzFile fp, uint32 *fb = (uint32 *)NULL, MDFN_Rect *LineWidths = (MDFN_Rect *)NULL);
int MDFNSS_LoadFP(gzFile fp);

typedef struct
{
        uint8 *data;
        uint32 loc;
        uint32 len;

        uint32 malloced;

	uint32 initial_malloc; // A setting!
} StateMem;

// Eh, we abuse the smem_* in-memory stream code
// in a few other places. :)
int32 smem_read(StateMem *st, void *buffer, uint32 len);
int32 smem_write(StateMem *st, void *buffer, uint32 len);
int32 smem_putc(StateMem *st, int value);
int32 smem_tell(StateMem *st);
int32 smem_seek(StateMem *st, uint32 offset, int whence);
int smem_write32le(StateMem *st, uint32 b);
int smem_read32le(StateMem *st, uint32 *b);

int MDFNSS_SaveSM(StateMem *st, int wantpreview, int data_only, uint32 *fb = (uint32 *)NULL, MDFN_Rect *LineWidths = (MDFN_Rect *)NULL);
int MDFNSS_LoadSM(StateMem *st, int haspreview, int data_only);

void MDFNSS_CheckStates(void);

typedef struct {
           void *v;	// Value
           uint32 s;	// Length(if 0, the subchunk isn't saved)
	   const char *desc;	// Description
} SFORMAT;

#define MDFNSTATE_RLSB            0x80000000
#define MDFNSTATE_RLSB32          0x40000000
#define MDFNSTATE_RLSB16          0x20000000

#define SFVAR(x) { &x, sizeof(x) | MDFNSTATE_RLSB, #x }
#define SFVARN(x, n) { &x, sizeof(x) | MDFNSTATE_RLSB, n }

#define SFARRAY(x, l) { x, l, #x }
//TODO:#define SFARRAY(x, l) { x, (l * sizeof(x[0])) | ((sizeof(x[0]) == 2) ? MDFNSTATE_RLSB16) | ((sizeof(x[0]) == 4) ? MDFNSTATE_RLSB32), #x }

#define SFARRAYN(x, l, n) { x, l, n }

#define SFARRAY16(x, l) {x, MDFNSTATE_RLSB16 | (l * sizeof(uint16)), #x }
#define SFARRAY16N(x, l, n) {x, MDFNSTATE_RLSB16 | (l * sizeof(uint16)), n }

#define SFARRAY32(x, l) {x, MDFNSTATE_RLSB32 | (l * sizeof(uint32)), #x }
#define SFARRAY32N(x, l, n) {x, MDFNSTATE_RLSB32 | (l * sizeof(uint32)), n }

#define SFEND { 0, 0, 0 }

#include <vector>

// State-Section Descriptor
class SSDescriptor
{
 public:
 SSDescriptor(SFORMAT *n_sf, const char *n_name, bool n_optional = 0)
 {
  sf = n_sf;
  name = n_name;
  optional = n_optional;
 }
 ~SSDescriptor(void)
 {

 }

 SFORMAT *sf;
 const char *name;
 bool optional;
};

int MDFNSS_StateAction(StateMem *st, int load, int data_only, std::vector <SSDescriptor> &sections);
int MDFNSS_StateAction(StateMem *st, int load, int data_only, SFORMAT *sf, const char *name, bool optional = 0);

void MDFN_StateEvilFlushMovieLove(void);
bool MDFN_StateEvilIsRunning(void);
void MDFN_StateEvilBegin(void);
void MDFN_StateEvilEnd(void);
int MDFN_StateEvil(int);

#endif
