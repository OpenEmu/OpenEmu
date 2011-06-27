#ifndef _EMU2413_H_
#define _EMU2413_H_

#include "emutypes.h"

/* Size of Sintable ( 8 -- 18 can be used. 9 recommended.)*/
#define PG_BITS 9              
#define PG_WIDTH (1<<PG_BITS)

/* Phase increment counter */
#define DP_BITS 18
#define DP_WIDTH (1<<DP_BITS)
#define DP_BASE_BITS (DP_BITS - PG_BITS)
                            
/* Dynamic range (Accuracy of sin table) */
#define DB_BITS 8
#define DB_STEP (48.0/(1<<DB_BITS))
#define DB_MUTE (1<<DB_BITS)

/* Dynamic range of envelope */
#define EG_STEP 0.375
#define EG_BITS 7
#define EG_MUTE (1<<EG_BITS)

/* Dynamic range of total level */
#define TL_STEP 0.75
#define TL_BITS 6   
#define TL_MUTE (1<<TL_BITS)

/* Dynamic range of sustine level */
#define SL_STEP 3.0
#define SL_BITS 4  
#define SL_MUTE (1<<SL_BITS)


/* Bits for Pitch and Amp modulator */
#define PM_PG_BITS 8
#define PM_PG_WIDTH (1<<PM_PG_BITS)
#define PM_DP_BITS 16
#define PM_DP_WIDTH (1<<PM_DP_BITS)
#define AM_PG_BITS 8
#define AM_PG_WIDTH (1<<AM_PG_BITS)
#define AM_DP_BITS 16
#define AM_DP_WIDTH (1<<AM_DP_BITS)


#ifdef EMU2413_DLL_EXPORTS
  #define EMU2413_API __declspec(dllexport)
#elif defined(EMU2413_DLL_IMPORTS)
  #define EMU2413_API __declspec(dllimport)
#else
  #define EMU2413_API
#endif

#ifdef __cplusplus
extern "C" {
#endif

#define PI 3.14159265358979323846

enum {OPLL_VRC7_TONE=0} ;

/* voice data */
typedef struct {
  e_uint32 TL,FB,EG,ML,AR,DR,SL,RR,KR,KL,AM,PM,WF ;
} OPLL_PATCH ;

/* slot */
typedef struct {

  OPLL_PATCH patch;  

  e_int32 type ;          /* 0 : modulator 1 : carrier */

  /* OUTPUT */
  e_int32 feedback ;
  e_int32 output[2] ;   /* Output value of slot */

  /* for Phase Generator (PG) */
  e_uint16 *sintbl ;    /* Wavetable */
  e_uint32 phase ;      /* Phase */
  e_uint32 dphase ;     /* Phase increment amount */
  e_uint32 pgout ;      /* output */

  /* for Envelope Generator (EG) */
  e_int32 fnum ;          /* F-Number */
  e_int32 block ;         /* Block */
  e_int32 volume ;        /* Current volume */
  e_int32 sustine ;       /* Sustine 1 = ON, 0 = OFF */
  e_uint32 tll ;	      /* Total Level + Key scale level*/
  e_uint32 rks ;        /* Key scale offset (Rks) */
  e_int32 eg_mode ;       /* Current state */
  e_uint32 eg_phase ;   /* Phase */
  e_uint32 eg_dphase ;  /* Phase increment amount */
  e_uint32 egout ;      /* output */

} OPLL_SLOT ;

/* Mask */
#define OPLL_MASK_CH(x) (1<<(x))

/* opll */
typedef struct {

  e_uint32 adr ;
  e_int32 out ;

#ifndef EMU2413_COMPACTION
  e_uint32 realstep ;
  e_uint32 oplltime ;
  e_uint32 opllstep ;
  e_int32 prev, next ;
#endif

  /* Register */
  e_uint8 LowFreq[6];
  e_uint8 HiFreq[6];
  e_uint8 InstVol[6];

  e_uint8 CustInst[8];

  e_int32 slot_on_flag[6 * 2] ;

  /* Pitch Modulator */
  e_uint32 pm_phase ;
  e_int32 lfo_pm ;

  /* Amp Modulator */
  e_int32 am_phase ;
  e_int32 lfo_am ;

  e_uint32 quality;

  /* Channel Data */
  e_int32 patch_number[6];
  e_int32 key_status[6] ;

  /* Slot */
  OPLL_SLOT slot[6 * 2] ;

  e_uint32 mask ;

  /* Input clock */
  e_uint32 clk;

  /* WaveTable for each envelope amp */
  e_uint16 fullsintable[PG_WIDTH];
  e_uint16 halfsintable[PG_WIDTH];

  e_uint16 *waveform[2];

  /* LFO Table */
  e_int32 pmtable[PM_PG_WIDTH];
  e_int32 amtable[AM_PG_WIDTH];
    
  /* Phase delta for LFO */
  e_uint32 pm_dphase;
  e_uint32 am_dphase;
 
  /* dB to Liner table */
  e_int16 DB2LIN_TABLE[(DB_MUTE + DB_MUTE) * 2];  

  /* Liner to Log curve conversion table (for Attack rate). */
  e_uint16 AR_ADJUST_TABLE[1 << EG_BITS];

  /* Phase incr table for Attack */
  e_uint32 dphaseARTable[16][16];

  /* Phase incr table for Decay and Release */
  e_uint32 dphaseDRTable[16][16];

  /* KSL + TL Table */
  e_uint32 tllTable[16][8][1 << TL_BITS][4];
  e_int32 rksTable[2][8][2];

  /* Phase incr table for PG */  
  e_uint32 dphaseTable[512][8][16];
} OPLL ;

/* Create Object */
EMU2413_API OPLL *OPLL_new(e_uint32 clk) ;
EMU2413_API void OPLL_delete(OPLL *) ;

/* Setup */
EMU2413_API void OPLL_reset(OPLL *) ;
EMU2413_API void OPLL_set_rate(OPLL *opll, e_uint32 r) ;
EMU2413_API void OPLL_set_quality(OPLL *opll, e_uint32 q) ;

/* Port/Register access */
EMU2413_API void OPLL_writeIO(OPLL *, e_uint32 reg, e_uint32 val) ;
EMU2413_API void OPLL_writeReg(OPLL *, e_uint32 reg, e_uint32 val) ;

/* Synthsize */
EMU2413_API e_int16 OPLL_calc(OPLL *) ;

/* Misc */
EMU2413_API void OPLL_forceRefresh(OPLL *) ;

/* Channel Mask */
EMU2413_API e_uint32 OPLL_setMask(OPLL *, e_uint32 mask) ;
EMU2413_API e_uint32 OPLL_toggleMask(OPLL *, e_uint32 mask) ;

#ifdef __cplusplus
}
#endif

#endif
