#ifndef _EMU2413_H_
#define _EMU2413_H_

#define PI 3.14159265358979323846

enum {EMU2413_2413_TONE = 0};

/* voice data */
typedef struct {
  uint32 TL,FB,EG,ML,AR,DR,SL,RR,KR,KL,AM,PM,WF ;
} EMU2413_PATCH ;

/* slot */
typedef struct {

  EMU2413_PATCH *patch;  

  int32 type ;          /* 0 : modulator 1 : carrier */

  /* OUTPUT */
  int32 feedback ;
  int32 output[2] ;   /* Output value of slot */

  /* for Phase Generator (PG) */
  uint32 *sintbl ;    /* Wavetable */
  uint32 phase ;      /* Phase */
  uint32 dphase ;     /* Phase increment amount */
  uint32 pgout ;      /* output */

  /* for Envelope Generator (EG) */
  int32 fnum ;          /* F-Number */
  int32 block ;         /* Block */
  int32 volume ;        /* Current volume */
  int32 sustine ;       /* Sustine 1 = ON, 0 = OFF */
  uint32 tll ;	      /* Total Level + Key scale level*/
  uint32 rks ;        /* Key scale offset (Rks) */
  int32 eg_mode ;       /* Current state */
  uint32 eg_phase ;   /* Phase */
  uint32 eg_dphase ;  /* Phase increment amount */
  uint32 egout ;      /* output */

} EMU2413_SLOT ;

/* Mask */
#define EMU2413_MASK_CH(x) (1<<(x))
#define EMU2413_MASK_HH (1<<(9))
#define EMU2413_MASK_CYM (1<<(10))
#define EMU2413_MASK_TOM (1<<(11))
#define EMU2413_MASK_SD (1<<(12))
#define EMU2413_MASK_BD (1<<(13))
#define EMU2413_MASK_RHYTHM ( EMU2413_MASK_HH | EMU2413_MASK_CYM | EMU2413_MASK_TOM | EMU2413_MASK_SD | EMU2413_MASK_BD )

/* opll */
typedef struct
{

  uint32 adr ;
  int32 out ;

  uint32 pan[16];

  /* Register */
  uint8 reg[0x40] ; 
  int32 slot_on_flag[18] ;

  /* Pitch Modulator */
  uint32 pm_phase ;
  int32 lfo_pm ;

  /* Amp Modulator */
  int32 am_phase ;
  int32 lfo_am ;

  /* Noise Generator */
  uint32 noise_seed ;

  /* Channel Data */
  int32 patch_number[9];
  int32 key_status[9] ;

  /* Slot */
  EMU2413_SLOT slot[18] ;

  /* Voice Data */
  EMU2413_PATCH patch[19*2] ;
  int32 patch_update[2] ; /* flag for check patch update */

  uint32 mask ;

} EMU2413 ;

/* Create Object */
EMU2413 *EMU2413_new(uint32 clk);
void EMU2413_delete(EMU2413 *);

/* Setup */
void EMU2413_reset(EMU2413 *);
void EMU2413_reset_patch(EMU2413 *, int32);
void EMU2413_set_pan(EMU2413 *, uint32 ch, uint32 pan);

/* Port/Register access */
void EMU2413_writeIO(EMU2413 *, uint32 reg, uint32 val) ;
void EMU2413_writeReg(EMU2413 *, uint32 reg, uint32 val) ;

/* Synthsize */
int32 EMU2413_calc(EMU2413 *) ;
void EMU2413_calc_stereo(EMU2413 *, int32 out[2]) ;

/* Misc */
void EMU2413_setPatch(EMU2413 *, const uint8 *dump) ;
void EMU2413_copyPatch(EMU2413 *, int32, EMU2413_PATCH *) ;
void EMU2413_forceRefresh(EMU2413 *) ;

/* Utility */
void EMU2413_dump2patch(const uint8 *dump, EMU2413_PATCH *patch) ;
void EMU2413_patch2dump(const EMU2413_PATCH *patch, uint8 *dump) ;
void EMU2413_getDefaultPatch(int32 type, int32 num, EMU2413_PATCH *) ;

/* Channel Mask */
uint32 EMU2413_setMask(EMU2413 *, uint32 mask) ;
uint32 EMU2413_toggleMask(EMU2413 *, uint32 mask) ;

#endif
