#ifndef _STATE_H
#define _STATE_H

#include <zlib.h>

#include "video.h"
#include "state-common.h"

void MDFNSS_GetStateInfo(const char *filename, StateStatusStruct *status);

int MDFNSS_Save(const char *, const char *suffix, const MDFN_Surface *surface = (MDFN_Surface *)NULL, const MDFN_Rect *DisplayRect = (MDFN_Rect*)NULL, const MDFN_Rect *LineWidths = (MDFN_Rect *)NULL);
int MDFNSS_Load(const char *, const char *suffix);
int MDFNSS_SaveFP(gzFile fp, const MDFN_Surface *surface = (MDFN_Surface *)NULL, const MDFN_Rect *DisplayRect = (MDFN_Rect*)NULL, const MDFN_Rect *LineWidths = (MDFN_Rect *)NULL);
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

int MDFNSS_SaveSM(StateMem *st, int wantpreview_and_ts, int data_only, const MDFN_Surface *surface = (MDFN_Surface *)NULL, const MDFN_Rect *DisplayRect = (MDFN_Rect*)NULL, const MDFN_Rect *LineWidths = (MDFN_Rect *)NULL);
int MDFNSS_LoadSM(StateMem *st, int haspreview, int data_only);

void MDFNSS_CheckStates(void);

// Flag for a single, >= 1 byte native-endian variable
#define MDFNSTATE_RLSB            0x80000000

// 32-bit native-endian elements
#define MDFNSTATE_RLSB32          0x40000000

// 16-bit native-endian elements
#define MDFNSTATE_RLSB16          0x20000000

// 64-bit native-endian elements
#define MDFNSTATE_RLSB64          0x10000000

#define MDFNSTATE_BOOL		  0x08000000


//// Array of structures
//#define MDFNSTATE_ARRAYOFS	  0x04000000

typedef struct {
           void *v;		// Pointer to the variable/array
           uint32 size;		// Length, in bytes, of the data to be saved EXCEPT:
				//  In the case of MDFNSTATE_BOOL, it is the number of bool elements to save(bool is not always 1-byte).
				// If 0, the subchunk isn't saved.
	   uint32 flags;	// Flags
	   const char *name;	// Name
	   //uint32 struct_size;	// Only used for MDFNSTATE_ARRAYOFS, sizeof(struct) that members of the linked SFORMAT struct are in.
} SFORMAT;

INLINE bool SF_IS_BOOL(bool *) { return(1); }
INLINE bool SF_IS_BOOL(void *) { return(0); }

INLINE uint32 SF_FORCE_AB(bool *) { return(0); }

INLINE uint32 SF_FORCE_A8(int8 *) { return(0); }
INLINE uint32 SF_FORCE_A8(uint8 *) { return(0); }

INLINE uint32 SF_FORCE_A16(int16 *) { return(0); }
INLINE uint32 SF_FORCE_A16(uint16 *) { return(0); }

INLINE uint32 SF_FORCE_A32(int32 *) { return(0); }
INLINE uint32 SF_FORCE_A32(uint32 *) { return(0); }

INLINE uint32 SF_FORCE_A64(int64 *) { return(0); }
INLINE uint32 SF_FORCE_A64(uint64 *) { return(0); }

INLINE uint32 SF_FORCE_D(double *) { return(0); }

#define SFVARN(x, n) { &(x), SF_IS_BOOL(&(x)) ? 1U : (uint32)sizeof(x), MDFNSTATE_RLSB | (SF_IS_BOOL(&(x)) ? MDFNSTATE_BOOL : 0), n }
#define SFVAR(x) SFVARN((x), #x)

#define SFARRAYN(x, l, n) { (x), (uint32)(l), 0 | SF_FORCE_A8(x), n }
#define SFARRAY(x, l) SFARRAYN((x), (l), #x)

#define SFARRAYBN(x, l, n) { (x), (uint32)(l), MDFNSTATE_BOOL | SF_FORCE_AB(x), n }
#define SFARRAYB(x, l) SFARRAYBN((x), (l), #x)

#define SFARRAY16N(x, l, n) { (x), (uint32)((l) * sizeof(uint16)), MDFNSTATE_RLSB16 | SF_FORCE_A16(x), n }
#define SFARRAY16(x, l) SFARRAY16N((x), (l), #x)

#define SFARRAY32N(x, l, n) { (x), (uint32)((l) * sizeof(uint32)), MDFNSTATE_RLSB32 | SF_FORCE_A32(x), n }
#define SFARRAY32(x, l) SFARRAY32N((x), (l), #x)

#define SFARRAY64N(x, l, n) { (x), (uint32)((l) * sizeof(uint64)), MDFNSTATE_RLSB64 | SF_FORCE_A64(x), n }
#define SFARRAY64(x, l) SFARRAY64N((x), (l), #x)

#if SIZEOF_DOUBLE != 8
#error "sizeof(double) != 8"
#endif

#define SFARRAYDN(x, l, n) { (x), (uint32)((l) * 8), MDFNSTATE_RLSB64 | SF_FORCE_D(x), n }
#define SFARRAYD(x, l) SFARRAYDN((x), (l), #x)

#define SFEND { 0, 0, 0, 0 }

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
