#ifndef _VDP_H_
#define _VDP_H_

namespace MDFN_IEN_MD
{

/* Pack and unpack CRAM data */
#define PACK_CRAM(d)    ((((d)&0xE00)>>9)|(((d)&0x0E0)>>2)|(((d)&0x00E)<<5))
#define UNPACK_CRAM(d)  ((((d)&0x1C0)>>5)|((d)&0x038)<<2|(((d)&0x007)<<9))

/* Mark a pattern as dirty */
#define MARK_BG_DIRTY(addr)                                     \
{                                                               \
    int name = (addr >> 5) & 0x7FF;                             \
    if(bg_name_dirty[name] == 0)                                \
    {                                                           \
	assert(bg_list_index < 0x800);				\
        bg_name_list[bg_list_index] = name;                     \
        bg_list_index += 1;					\
    }                                                           \
    bg_name_dirty[name] |= (1 << ((addr >> 2) & 0x07));         \
}

/* Look-up pixel table information */
#define LUT_MAX         (5)
#define LUT_SIZE        (0x10000)

/* Clip structure */
typedef struct
{
    uint8 left;
    uint8 right;
    uint8 enable;
} clip_t;

typedef struct
{
    uint16 ypos;
    uint16 xpos;
    uint16 attr;
    uint8 size;
    uint8 index;
} object_info_t;

class MDVDP
{
 public:

 /* Function prototypes */
 MDVDP(void);
 ~MDVDP();

 void SetSettings(bool PAL, bool PAL_reported, bool auto_aspect);

 void Reset(void);
 void vdp_ctrl_w(uint16 data);
 uint16 vdp_ctrl_r(void);
 void vdp_data_w(uint16 data);
 uint16 vdp_data_r(void);
 void vdp_reg_w(uint8 r, uint8 d);
 uint16 vdp_hvc_r(void);
 void dma_copy(void);
 void dma_vbus(void);
 void vdp_test_w(uint16 value);

 void SyncColors(void);

 void SetPixelFormat(const MDFN_PixelFormat &format); //int rs, int gs, int bs);
 void SetSurface(MDFN_Surface *surface, MDFN_Rect *rect);

 bool ToggleLayer(int which);

 int StateAction(StateMem *sm, int load, int data_only);
 void ResetTS(void);

 inline int IntAckCallback(int int_level)
 {
 //    printf("Callback: %d; %d %d, %d\n", int_level, hint_pending, vint_pending, scanline);
    switch(int_level)
    {
        case 4:
            hint_pending = 0;
            status &= ~0x0080;
            vint_pending = 0;
            break;

        case 6:
            status &= ~0x0080;
            vint_pending = 0;
            break;
    }
 //    C68k_Set_IRQ(&Main68K, 0);

    return C68K_INT_ACK_AUTOVECTOR;
 }

 private:
 /* Tables that define the playfield layout */
 static const uint8 shift_table[4];
 static const uint8 col_mask_table[4];
 static const uint16 row_mask_table[4];
 static const uint32 y_mask_table[4];

 /* Attribute expansion table */
 static const uint32 atex_table[8];

 enum
 {
  //(Totally guesstimated and bsed VDP timing :b)

  // H-counter starts at H40: 0xE4, H32: 0xE9 from here
  // H-int is done here as well?
  // Increment scanline here as well?
  VDPLP_HRETRACE_0 = 0,	// H40: 56, H32: 46

  // H-counter starts at 0 from here.
  VDPLP_HRETRACE_1,	// H40: 16, H32: 13

  // Vertical interrupt happens here on the appropriate line
  VDPLP_VISIBLE_0,	// H40: 4, H32: 4
  VDPLP_VISIBLE_1,	// H40: 320 H32: 256

  // Vblank flag is set here
  VDPLP_VISIBLE_2,	// H40: 26  H32: 23
  VDPLP_TOTAL
 };


 uint8 sat[0x400];               /* Internal copy of sprite attribute table */
 uint8 vram[0x10000];            /* Video RAM (64Kx8) */
 uint16 cram[0x40];              /* On-chip color RAM (64x9) */
 uint16 vsram[0x40];             /* On-chip vertical scroll RAM (40x11) */
 uint8 reg[0x20];                /* Internal VDP registers (23x8) */

 uint16 addr;                    /* Address register */
 uint16 addr_latch;              /* Latched A15, A14 of address */
 uint8 code;                     /* Code register */
 uint8 pending;                  /* Pending write flag */
 uint16 buffer;                  /* Read buffer */
 uint16 status;                  /* VDP status flags */
 uint16 ntab;                    /* Name table A base address */
 uint16 ntbb;                    /* Name table B base address */
 uint16 ntwb;                    /* Name table W base address */
 uint16 satb;                    /* Sprite attribute table base address */
 uint16 hscb;                    /* Horizontal scroll table base address */
 uint16 sat_base_mask;           /* Base bits of SAT */
 uint16 sat_addr_mask;           /* Index bits of SAT */

 uint8 dma_fill_latch;
 uint32 DMASource;
 uint16 DMALength;

 uint8 border;                   /* Border color index */
 uint8 bg_name_dirty[0x800];     /* 1= This pattern is dirty */
 uint16 bg_name_list[0x800];     /* List of modified pattern indices */
 uint16 bg_list_index;           /* # of modified patterns in list */
 uint32 bg_pattern_cache[0x80000 / sizeof(uint32)];/* Cached and flipped patterns */
 uint8 playfield_shift;          /* Width of planes A, B (in bits) */
 uint8 playfield_col_mask;       /* Vertical scroll mask */
 uint16 playfield_row_mask;      /* Horizontal scroll mask */
 uint32 y_mask;                  /* Name table Y-index bits mask */

 bool hint_pending;               /* 0= Line interrupt is pending */
 bool vint_pending;               /* 1= Frame interrupt is pending */
 uint16 counter;                    /* Raster counter */
 bool dma_fill;                   /* 1= DMA fill has been requested */
 int im2_flag;                   /* 1= Interlace mode 2 is being used */
 int visible_frame_end;          /* End-of-frame (IRQ line) */
 int v_counter;                  /* VDP scan line counter */
 int v_update;                  /* 1= VC was updated by a ctrl or HV read */

 int32 vdp_cycle_counter;
 int32 vdp_last_ts;
 int32 vdp_line_phase;
 int32 vdp_hcounter_start_ts;

 int32 scanline;
 bool is_pal, report_pal;
 bool WantAutoAspect;


 void RedoViewport(void);
 void WriteCRAM(uint16);
 void MemoryWrite8(uint8);
 void MemoryWrite16(uint16);
 void CheckDMA(void);


 MDFN_Surface *surface;
 MDFN_Rect *rect;

 /* Clip data */
 clip_t clip[2];

 /* Sprite name look-up table */
 uint8 name_lut[0x400];

 /* Sprite line buffer data */
 uint8 object_index_count;

 object_info_t object_info[20];

 /* Pixel look-up tables and table base address */
 uint8 *lut[5];
 uint8 *lut_base;

 /* 32-bit pixel remapping data */
 uint32 pixel_32[0x100];
 uint32 pixel_32_lut[3][0x200];

 uint32 UserLE; // User layer enable;

/* Function prototypes */
 void render_line(int line);
 void render_obj(int line, uint8 *buf, uint8 *table);
 void render_obj_im2(int line, uint8 *buf, uint8 *table);
 void render_ntw(int line, uint8 *buf);
 void render_ntw_im2(int line, uint8 *buf);
 void render_ntx(int which, int line, uint8 *buf);
 void render_ntx_im2(int which, int line, uint8 *buf);
 void render_ntx_vs(int which, int line, uint8 *buf);
 void update_bg_pattern_cache(void);
 void get_hscroll(int line, uint16 *scrolla, uint16 *scrollb);
 void window_clip(int line);
 int make_lut_bg(int bx, int ax);
 int make_lut_obj(int bx, int sx);
 int make_lut_bg_ste(int bx, int ax);
 int make_lut_obj_ste(int bx, int sx);
 int make_lut_bgobj_ste(int bx, int sx);
 void remap_32(uint8 *src, uint32 *dst, uint32 *table, int length);
 void merge(uint8 *srca, uint8 *srcb, uint8 *dst, uint8 *table, int width);
 void color_update(int index, uint16 data);
 void make_name_lut(void);
 void parse_satb(int line);


 public:
 #if 0
 inline void Run(void)
 {
  #include "vdp_run.inc"
 }
 #else
 void Run(void);
 #endif
};

}

#endif /* _VDP_H_ */
