/*
* Glide64 - Glide video plugin for Nintendo 64 emulators.
* Copyright (c) 2002  Dave2001
* Copyright (c) 2003-2009  Sergey 'Gonetz' Lipski
*
* This program is free software; you can redistribute it and/or modify
* it under the terms of the GNU General Public License as published by
* the Free Software Foundation; either version 2 of the License, or
* any later version.
*
* This program is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
* GNU General Public License for more details.
*
* You should have received a copy of the GNU General Public License
* along with this program; if not, write to the Free Software
* Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
*/

//****************************************************************
//
// Glide64 - Glide Plugin for Nintendo 64 emulators
// Project started on December 29th, 2001
//
// Authors:
// Dave2001, original author, founded the project in 2001, left it in 2002
// Gugaman, joined the project in 2002, left it in 2002
// Sergey 'Gonetz' Lipski, joined the project in 2002, main author since fall of 2002
// Hiroshi 'KoolSmoky' Morii, joined the project in 2007
//
//****************************************************************
//
// To modify Glide64:
// * Write your name and (optional)email, commented by your work, so I know who did it, and so that you can find which parts you modified when it comes time to send it to me.
// * Do NOT send me the whole project or file that you modified.  Take out your modified code sections, and tell me where to put them.  If people sent the whole thing, I would have many different versions, but no idea how to combine them all.
//
//****************************************************************

#ifndef RDP_H
#define RDP_H

extern char out_buf[2048];

extern wxUint32 frame_count; // frame counter

//GlideHQ support
//#define TEXTURE_FILTER
#ifdef TEXTURE_FILTER
#include "../GlideHQ/Ext_TxFilter.h"
#endif

#define MAX_CACHE   1024*4
#define MAX_TRI_CACHE 768 // this is actually # of vertices, not triangles
#define MAX_VTX     256

#define MAX_TMU     2

#define TEXMEM_2MB_EDGE 2097152

// Supported flags
#define SUP_TEXMIRROR 0x00000001

// Clipping flags
#define CLIP_XMAX 0x00000001
#define CLIP_XMIN 0x00000002
#define CLIP_YMAX 0x00000004
#define CLIP_YMIN 0x00000008
#define CLIP_WMIN 0x00000010
#define CLIP_ZMAX 0x00000020
#define CLIP_ZMIN 0x00000040

// Flags
#define ZBUF_ENABLED  0x00000001
#define ZBUF_DECAL    0x00000002
#define ZBUF_COMPARE  0x00000004
#define ZBUF_UPDATE   0x00000008
#define ALPHA_COMPARE 0x00000010
#define FORCE_BL      0x00000020
#define CULL_FRONT    0x00001000  // * must be here
#define CULL_BACK     0x00002000  // * must be here
#define FOG_ENABLED   0x00010000

#define CULLMASK    0x00003000
#define CULLSHIFT   12

// Update flags
#define UPDATE_ZBUF_ENABLED 0x00000001

#define UPDATE_TEXTURE    0x00000002  // \ Same thing!
#define UPDATE_COMBINE    0x00000002  // /

#define UPDATE_CULL_MODE  0x00000004
#define UPDATE_LIGHTS     0x00000010
#define UPDATE_BIASLEVEL  0x00000020
#define UPDATE_ALPHA_COMPARE  0x00000040
#define UPDATE_VIEWPORT   0x00000080
#define UPDATE_MULT_MAT   0x00000100
#define UPDATE_SCISSOR    0x00000200
#define UPDATE_FOG_ENABLED  0x00010000

#define CMB_MULT    0x00000001
#define CMB_SET     0x00000002
#define CMB_SUB     0x00000004
#define CMB_ADD     0x00000008
#define CMB_A_MULT  0x00000010
#define CMB_A_SET   0x00000020
#define CMB_A_SUB   0x00000040
#define CMB_A_ADD   0x00000080
#define CMB_SETSHADE_SHADEALPHA 0x00000100
#define CMB_INTER   0x00000200
#define CMB_MULT_OWN_ALPHA  0x00000400
#define CMB_COL_SUB_OWN  0x00000800

#define uc(x) coord[x<<1]
#define vc(x) coord[(x<<1)+1]

#if defined __VISUALC__
#define DECLAREALIGN16VAR(var) __declspec(align(16)) float (var)
#elif defined __GNUG__
#define DECLAREALIGN16VAR(var) float (var) __attribute__ ((aligned(16)))
#endif

// Vertex structure
typedef struct
{
  float x, y, z, q;
  float u0, v0, u1, v1;
  float coord[4];
  float w;
  wxUint16  flags;

  wxUint8  b;  // These values are arranged like this so that *(wxUint32*)(VERTEX+?) is
  wxUint8  g;  // ARGB format that glide can use.
  wxUint8  r;
  wxUint8  a;

  float f; //fog

  float vec[3]; // normal vector

  float sx, sy, sz;
  float x_w, y_w, z_w, u0_w, v0_w, u1_w, v1_w, oow;
  wxUint8  not_zclipped;
  wxUint8  screen_translated;
  wxUint8  uv_scaled;
  wxUint32 uv_calculated;  // like crc
  wxUint32 shade_mod;
  wxUint32 color_backup;

  float ou, ov;

  int   number;   // way to identify it
  int   scr_off, z_off; // off the screen?
} VERTEX;

// Clipping (scissors)
typedef struct {
  wxUint32 ul_x;
  wxUint32 ul_y;
  wxUint32 lr_x;
  wxUint32 lr_y;
} SCISSOR;

#ifdef TEXTURE_FILTER
extern wxUint32 texfltr[];
extern wxUint32 texenht[];
extern wxUint32 texcmpr[];
extern wxUint32 texhirs[];

typedef struct {
  wxUint16 tile_ul_s;
  wxUint16 tile_ul_t;
  wxUint16 tile_width;
  wxUint16 tile_height;
  wxUint16 tex_width;
  wxUint16 tex_size;
  wxUint32 dxt;
} LOAD_TILE_INFO;
#endif

// #warning no screenshot support
/*
typedef struct {
  const wxChar * format;
  const wxChar * extension;
  wxBitmapType type;
} SCREEN_SHOT_FORMAT;

extern const int NumOfFormats;
extern SCREEN_SHOT_FORMAT ScreenShotFormats[];
*/
typedef struct {
  int card_id;
  int lang_id;

  wxUint32 res_x, scr_res_x;
  wxUint32 res_y, scr_res_y;
  wxUint32 res_data, res_data_org;

  int advanced_options;
  int texenh_options;
  int ssformat;
  int vsync;

  int show_fps;
  int clock;
  int clock_24_hr;

  int filtering;
  int fog;
  int buff_clear;
  int swapmode;
  int lodmode;
  int aspectmode;
  int use_hotkeys;

  //Frame buffer emulation options
  #define  fb_emulation            (1<<0)   //frame buffer emulation
  #define  fb_hwfbe                (1<<1)   //hardware frame buffer emualtion
  #define  fb_motionblur           (1<<2)   //emulate motion blur
  #define  fb_ref                  (1<<3)   //read every frame
  #define  fb_read_alpha           (1<<4)   //read alpha
  #define  fb_hwfbe_buf_clear      (1<<5)   //clear auxiliary texture frame buffers
  #define  fb_depth_render         (1<<6)   //enable software depth render
  #define  fb_optimize_texrect     (1<<7)   //fast texrect rendering with hwfbe
  #define  fb_ignore_aux_copy      (1<<8)   //do not copy auxiliary frame buffers
  #define  fb_useless_is_useless   (1<<10)  //
  #define  fb_get_info             (1<<11)  //get frame buffer info
  #define  fb_read_back_to_screen  (1<<12)  //render N64 frame buffer to screen
  #define  fb_read_back_to_screen2 (1<<13)  //render N64 frame buffer to screen
  #define  fb_cpu_write_hack       (1<<14)  //show images writed directly by CPU

  #define fb_emulation_enabled ((settings.frame_buffer&fb_emulation)>0)
  #define fb_hwfbe_enabled ((settings.frame_buffer&(fb_emulation|fb_hwfbe))==(fb_emulation|fb_hwfbe))
  #define fb_depth_render_enabled ((settings.frame_buffer&fb_depth_render)>0)

  wxUint32 frame_buffer;
  enum FBCRCMODE {
    fbcrcNone = 0,
    fbcrcFast = 1,
    fbcrcSafe = 2} fb_crc_mode;

#ifdef TEXTURE_FILTER
  //Texture filtering options
  int ghq_fltr;
  int ghq_enht;
  int ghq_cmpr;
  int ghq_hirs;
  int ghq_use;
  int ghq_enht_cmpr;
  int ghq_enht_tile;
  int ghq_enht_f16bpp;
  int ghq_enht_gz;
  int ghq_enht_nobg;
  int ghq_hirs_cmpr;
  int ghq_hirs_tile;
  int ghq_hirs_f16bpp;
  int ghq_hirs_gz;
  int ghq_hirs_altcrc;
  int ghq_cache_save;
  int ghq_cache_size;
  int ghq_hirs_let_texartists_fly;
  int ghq_hirs_dump;
#endif

  //Debug
  int autodetect_ucode;
  int ucode;
  int logging;
  int elogging;
  int log_clear;
  int run_in_window;
  int filter_cache;
  int unk_as_red;
  int log_unk;
  int unk_clear;
  int wireframe;
  int wfmode;

  // Special fixes
  int offset_x, offset_y;
  int scale_x, scale_y;
  int fast_crc;
  int alt_tex_size;
  int use_sts1_only;
  int flame_corona; //hack for zeldas flame's corona
  int increase_texrect_edge; // add 1 to lower right corner coordinates of texrect
  int decrease_fillrect_edge; // sub 1 from lower right corner coordinates of fillrect
  int texture_correction; // enable perspective texture correction emulation. is on by default
  int stipple_mode;  //used for dithered alpha emulation
  wxUint32 stipple_pattern; //used for dithered alpha emulation
  int force_microcheck; //check microcode each frame, for mixed F3DEX-S2DEX games
  int force_quad3d; //force 0xb5 command to be quad, not line 3d
  int clip_zmin; //enable near z clipping
  int clip_zmax; //enable far plane clipping;
  int adjust_aspect; //adjust screen aspect for wide screen mode
  int force_calc_sphere; //use spheric mapping only, Ridge Racer 64
  int pal230;    //set special scale for PAL games
  int correct_viewport; //correct viewport values
  int zmode_compare_less; //force GR_CMP_LESS for zmode=0 (opaque)and zmode=1 (interpenetrating)
  int old_style_adither; //apply alpha dither regardless of alpha_dither_mode
  int n64_z_scale; //scale vertex z value before writing to depth buffer, as N64 does.

  //Special game hacks
  #define  hack_ASB         (1<<0)   //All-Star Baseball games
  #define  hack_Banjo2      (1<<1)   //Banjo Tooie
  #define  hack_BAR         (1<<2)   //Beetle Adventure Racing
  #define  hack_Chopper     (1<<3)   //Chopper Attack
  #define  hack_Diddy       (1<<4)   //diddy kong racing
  #define  hack_Fifa98      (1<<5)   //FIFA - Road to World Cup 98
  #define  hack_Fzero       (1<<6)   //F-Zero
  #define  hack_GoldenEye   (1<<7)   //Golden Eye
  #define  hack_Hyperbike   (1<<8)   //Top Gear Hyper Bike
  #define  hack_ISS64       (1<<9)   //International Superstar Soccer 64
  #define  hack_KI          (1<<10)  //Killer Instinct
  #define  hack_Knockout    (1<<11)  //Knockout Kings 2000
  #define  hack_Lego        (1<<12)  //LEGO Racers
  #define  hack_MK64        (1<<13)  //Mario Kart
  #define  hack_Megaman     (1<<14)  //Megaman64
  #define  hack_Makers      (1<<15)  //Mischief-makers
  #define  hack_WCWnitro    (1<<16)  //WCW Nitro
  #define  hack_Ogre64      (1<<17)  //Ogre Battle 64
  #define  hack_Pilotwings  (1<<18)  //Pilotwings
  #define  hack_PMario      (1<<19)  //Paper Mario
  #define  hack_PPL         (1<<20)  //pokemon puzzle league requires many special fixes
  #define  hack_RE2         (1<<21)  //Resident Evil 2
  #define  hack_Starcraft   (1<<22)  //StarCraft64
  #define  hack_Supercross  (1<<23)  //Supercross 2000
  #define  hack_TGR         (1<<24)  //Top Gear Rally
  #define  hack_TGR2        (1<<25)  //Top Gear Rally 2
  #define  hack_Tonic       (1<<26)  //tonic trouble
  #define  hack_Yoshi       (1<<27)  //Yoshi Story
  #define  hack_Zelda       (1<<28)  //zeldas hacks
  wxUint32 hacks;

  //wrapper settings
  int wrpResolution;
  int wrpVRAM;
  int wrpFBO;
  int wrpAnisotropic;

} SETTINGS;

typedef struct
{
  wxUint8 hk_ref;
  wxUint8 hk_motionblur;
  wxUint8 hk_filtering;
} HOTKEY_INFO;

typedef struct
{
  int num_tmu;
  int max_tex_size;
  int sup_large_tex;
  int sup_mirroring;
  int sup_32bit_tex;
  int has_2mb_tex_boundary;
  int tex_UMA;
  int gamma_correction;
  FxI32 gamma_table_size;
  FxU32 *gamma_table_r;
  FxU32 *gamma_table_g;
  FxU32 *gamma_table_b;
  wxUint32 tmem_ptr[MAX_TMU];
  wxUint32 tex_min_addr[MAX_TMU];
  wxUint32 tex_max_addr[MAX_TMU];
} VOODOO;

// This structure is what is passed in by rdp:settextureimage
typedef struct {
  wxUint8 format;  // format: ARGB, IA, ...
  wxUint8 size;    // size: 4,8,16, or 32 bit
  wxUint16 width;   // used in settextureimage
  wxUint32 addr;   // address in RDRAM to load the texture from
  int set_by;  // 0-loadblock 1-loadtile
} TEXTURE_IMAGE;

// This structure is a tile descriptor (as used by rdp:settile and rdp:settilesize)
typedef struct
{
  // rdp:settile
  wxUint8 format;  // format: ARGB, IA, ...
  wxUint8 size;    // size: 4,8,16, or 32 bit
  wxUint16 line;    // size of one row (x axis) in 64 bit words
  wxUint16 t_mem;   // location in texture memory (in 64 bit words, max 512 (4MB))
  wxUint8 palette; // palette # to use
  wxUint8 clamp_t; // clamp or wrap (y axis)?
  wxUint8 mirror_t;  // mirroring on (y axis)?
  wxUint8 mask_t;  // mask to wrap around (ex: 5 would wrap around 32) (y axis)
  wxUint8 shift_t; // ??? (scaling)
  wxUint8 clamp_s; // clamp or wrap (x axis)?
  wxUint8 mirror_s;  // mirroring on (x axis)?
  wxUint8 mask_s;  // mask to wrap around (x axis)
  wxUint8 shift_s; // ??? (scaling)

    // rdp:settilesize
  wxUint16 ul_s;    // upper left s coordinate
  wxUint16 ul_t;    // upper left t coordinate
  wxUint16 lr_s;    // lower right s coordinate
  wxUint16 lr_t;    // lower right t coordinate

  float f_ul_s;
  float f_ul_t;

  // these are set by loadtile
  wxUint16 t_ul_s;    // upper left s coordinate
  wxUint16 t_ul_t;    // upper left t coordinate
  wxUint16 t_lr_s;    // lower right s coordinate
  wxUint16 t_lr_t;    // lower right t coordinate

  wxUint32 width;
  wxUint32 height;

  // uc0:texture
  wxUint8 on;
  float s_scale;
  float t_scale;

  wxUint16 org_s_scale;
  wxUint16 org_t_scale;
} TILE;

// This structure forms the lookup table for cached textures
typedef struct {
  wxUint32 addr;     // address in RDRAM
  wxUint32 crc;      // CRC check
  wxUint32 palette;    // Palette #
  wxUint32 width;    // width
  wxUint32 height;   // height
  wxUint32 format;   // format
  wxUint32 size;     // size
  wxUint32 last_used;  // what frame # was this texture last used (used for replacing)

  wxUint32 line;

  wxUint32 flags;    // clamp/wrap/mirror flags

  wxUint32 realwidth;  // width of actual texture
  wxUint32 realheight; // height of actual texture
  wxUint32 lod;
  wxUint32 aspect;

  wxUint8 set_by;
  wxUint8 texrecting;

  int f_mirror_s;
  int f_mirror_t;
  int f_wrap_s;
  int f_wrap_t;

  float scale_x;    // texture scaling
  float scale_y;
  float scale;    // general scale to 256

  GrTexInfo t_info; // texture info (glide)
  wxUint32 tmem_addr;  // addres in texture memory (glide)

  int uses;   // 1 triangle that uses this texture

  int splits;   // number of splits
  int splitheight;

  float c_off;  // ul center texel offset (both x and y)
  float c_scl_x;  // scale to lower-right center-texel x
  float c_scl_y;  // scale to lower-right center-texel y

  wxUint32 mod, mod_color, mod_color1, mod_color2, mod_factor;
#ifdef TEXTURE_FILTER
  uint64 ricecrc;
  int is_hires_tex;
#endif
} CACHE_LUT;

// Lights
typedef struct {
  float r, g, b, a;       // color
  float dir_x, dir_y, dir_z;  // direction towards the light source
  float x, y, z, w;  // light position
  float ca, la, qa;
  wxUint32 nonblack;
  wxUint32 nonzero;
} LIGHT;

typedef enum {
  ci_main,      //0, main color image
  ci_zimg,      //1, depth image
  ci_unknown,   //2, status is unknown
  ci_useless,   //3, status is unclear
  ci_old_copy,  //4, auxiliary color image, copy of last color image from previous frame
  ci_copy,      //5, auxiliary color image, copy of previous color image
  ci_copy_self, //6, main color image, it's content will be used to draw into itself
  ci_zcopy,     //7, auxiliary color image, copy of depth image
  ci_aux,       //8, auxiliary color image
  ci_aux_copy   //9, auxiliary color image, partial copy of previous color image
} CI_STATUS;

// Frame buffers
typedef struct
{
	wxUint32 addr;   //color image address
	wxUint8 format;
	wxUint8 size;
	wxUint16 width;
	wxUint16 height;
	CI_STATUS status;
	int   changed;
} COLOR_IMAGE;

typedef struct
{
  GrChipID_t tmu;
	wxUint32 addr;  //address of color image
	wxUint32 end_addr;
	wxUint32 tex_addr; //address in video memory
	wxUint32 width;    //width of color image
	wxUint32 height;   //height of color image
	wxUint8  format;   //format of color image
	wxUint8  size;   //format of color image
	wxUint8  clear;  //flag. texture buffer must be cleared
	wxUint8  drawn;  //flag. if equal to 1, this image was already drawn in current frame
	wxUint32 crc; //checksum of the color image
	float scr_width; //width of rendered image
	float scr_height; //height of rendered image
	wxUint32 tex_width;  //width of texture buffer
	wxUint32 tex_height; //height of texture buffer
	int   tile;     //
	wxUint16  tile_uls; //shift from left bound of the texture
	wxUint16  tile_ult; //shift from top of the texture
	wxUint32 v_shift; //shift from top of the texture
	wxUint32 u_shift; //shift from left of the texture
	float lr_u;
	float lr_v;
	float u_scale; //used to map vertex u,v coordinates into hires texture
	float v_scale; //used to map vertex u,v coordinates into hires texture
	CACHE_LUT * cache; //pointer to texture cache item
	GrTexInfo info;
  wxUint16 t_mem;
} TBUFF_COLOR_IMAGE;

typedef struct
{
	GrChipID_t tmu;
	wxUint32 begin; //start of the block in video memory
	wxUint32 end;   //end of the block in video memory
	wxUint8 count;  //number of allocated texture buffers
	int clear_allowed; //stack of buffers can be cleared
	TBUFF_COLOR_IMAGE images[256];
} TEXTURE_BUFFER;

#define NUMTEXBUF 92

struct RDP_Base{
  float vi_width;
  float vi_height;

  int window_changed;

  float offset_x, offset_y, offset_x_bak, offset_y_bak;

  float scale_x, scale_1024, scale_x_bak;
  float scale_y, scale_768, scale_y_bak;

  float view_scale[3];
  float view_trans[3];
  float clip_min_x, clip_max_x, clip_min_y, clip_max_y;
  float clip_ratio;

  int updatescreen;

  wxUint32 tri_n;  // triangle counter
  wxUint32 debug_n;

  // Program counter
  wxUint32 pc[10]; // DList PC stack
  wxUint32 pc_i;   // current PC index in the stack
  int dl_count; // number of instructions before returning
  int LLE;

  // Segments
  wxUint32 segment[16];  // Segment pointer

  // Marks the end of DList execution (done in uc?:enddl)
  int halt;

  // Next command
  wxUint32 cmd0;
  wxUint32 cmd1;
  wxUint32 cmd2;
  wxUint32 cmd3;

  // Clipping
  SCISSOR scissor_o;
  SCISSOR scissor;
  int scissor_set;

  // Colors
  wxUint32 fog_color;
  wxUint32 fill_color;
  wxUint32 prim_color;
  wxUint32 blend_color;
  wxUint32 env_color;
  wxUint32 SCALE;
  wxUint32 CENTER;
  wxUint32 prim_lodmin, prim_lodfrac;
  wxUint16 prim_depth;
  wxUint16 prim_dz;
  wxUint8 K4;
  wxUint8 K5;
  enum {
  noise_none,
  noise_combine,
  noise_texture
  } noise;

  float col[4];   // color multiplier
  float coladd[4];  // color add/subtract
  float shade_factor;

  float col_2[4];

  wxUint32 cmb_flags, cmb_flags_2;

  // othermode_l flags
  int acmp; // 0 = none, 1 = threshold, 2 = dither
  int zsrc; // 0 = pixel, 1 = prim
  wxUint8 alpha_dither_mode;

  // Matrices
  DECLAREALIGN16VAR(model[4][4]);
  DECLAREALIGN16VAR(proj[4][4]);
  DECLAREALIGN16VAR(combined[4][4]);
  DECLAREALIGN16VAR(dkrproj[3][4][4]);

  DECLAREALIGN16VAR(model_stack[32][4][4]);  // 32 deep, will warn if overflow
  int model_i;          // index in the model matrix stack
  int model_stack_size;

  // Textures
  TEXTURE_IMAGE timg;       // 1 for each tmem address
  TILE tiles[8];          // 8 tile descriptors
  wxUint8 tmem[4096];        // 4k tmem
  wxUint32 addr[512];        // 512 addresses (used to determine address loaded from)
#ifdef TEXTURE_FILTER
  LOAD_TILE_INFO load_info[512];    // 512 addresses. inforamation about tile loading.
#endif

  int     cur_tile;   // current tile
  int     mipmap_level;
  int     last_tile;   // last tile set
  int     last_tile_size;   // last tile size set

  int     t0, t1;
  int     best_tex; // if no 2-tmus, which texture? (0 or 1)
  int     tex;
  int     filter_mode;

  // Texture palette
  wxUint16 pal_8[256];
  wxUint32 pal_8_crc[16];
  wxUint32 pal_256_crc;
  wxUint8 tlut_mode;
  int LOD_en;
  int Persp_en;
  int persp_supported;
  int force_wrap;
#ifdef TEXTURE_FILTER
  wxUint16 pal_8_rice[512];
#endif

  // Lighting
  wxUint32 num_lights;
  LIGHT light[12];
  float light_vector[12][3];
  float lookat[2][3];
  int  use_lookat;

  // Combine modes
  wxUint32 cycle1, cycle2, cycle_mode;
  wxUint8 c_a0, c_b0, c_c0, c_d0, c_Aa0, c_Ab0, c_Ac0, c_Ad0;
  wxUint8 c_a1, c_b1, c_c1, c_d1, c_Aa1, c_Ab1, c_Ac1, c_Ad1;

  wxUint8 fbl_a0, fbl_b0, fbl_c0, fbl_d0;
  wxUint8 fbl_a1, fbl_b1, fbl_c1, fbl_d1;

  wxUint8 uncombined;  // which is uncombined: 0x01=color 0x02=alpha 0x03=both

//  float YUV_C0, YUV_C1, YUV_C2, YUV_C3, YUV_C4; //YUV textures conversion coefficients

  // What needs updating
  wxUint32 update;
  wxUint32 flags;

  int first;

  wxUint32 tex_ctr;    // incremented every time textures are updated

  int allow_combine; // allow combine updating?

  int s2dex_tex_loaded;
  wxUint16 bg_image_height;

  // Debug stuff
  wxUint32 rm; // use othermode_l instead, this just as a check for changes
  wxUint32 render_mode_changed;
  wxUint32 geom_mode;

  wxUint32 othermode_h;
  wxUint32 othermode_l;

  // used to check if in texrect while loading texture
  wxUint8 texrecting;

  //frame buffer related slots. Added by Gonetz
  wxUint32 cimg, ocimg, zimg, tmpzimg, vi_org_reg;
  COLOR_IMAGE maincimg[2];
  wxUint32 last_drawn_ci_addr;
  wxUint32 main_ci, main_ci_end, main_ci_bg, main_ci_last_tex_addr, zimg_end, last_bg;
  wxUint32 ci_width, ci_height, ci_size, ci_end;
  wxUint32 zi_width;
  int zi_lrx, zi_lry;
  wxUint8  ci_count, num_of_ci, main_ci_index, copy_ci_index, copy_zi_index;
  int swap_ci_index, black_ci_index;
  wxUint32 ci_upper_bound, ci_lower_bound;
  int  motionblur, fb_drawn, fb_drawn_front, read_previous_ci, read_whole_frame;
  CI_STATUS ci_status;
  TBUFF_COLOR_IMAGE * cur_image;  //image currently being drawn
  TBUFF_COLOR_IMAGE * tbuff_tex;  //image, which corresponds to currently selected texture
  TBUFF_COLOR_IMAGE * aTBuffTex[2]; 
  wxUint8  cur_tex_buf;
  wxUint8  acc_tex_buf;
  int skip_drawing; //rendering is not required. used for frame buffer emulation

  //fog related slots. Added by Gonetz
  float fog_multiplier, fog_offset;
  enum {
    fog_disabled,
    fog_enabled,
    fog_blend,
    fog_blend_inverse
    }
  fog_mode;
};

struct RDP : public RDP_Base
{
  // Clipping
  int clip;     // clipping flags
  VERTEX *vtx1; //[256] copy vertex buffer #1 (used for clipping)
  VERTEX *vtx2; //[256] copy vertex buffer #2
  VERTEX *vtxbuf;   // current vertex buffer (reset to vtx, used to determine current vertex buffer)
  VERTEX *vtxbuf2;
  int n_global;   // Used to pass the number of vertices from clip_z to clip_tri
  int vtx_buffer;

  CACHE_LUT *cache[MAX_TMU]; //[MAX_CACHE]
  CACHE_LUT *cur_cache[MAX_TMU];
  wxUint32   cur_cache_n[MAX_TMU];
  int     n_cached[MAX_TMU];

  // Vertices
  VERTEX *vtx; //[MAX_VTX]
  int v0, vn;

  COLOR_IMAGE *frame_buffers; //[NUMTEXBUF+2]
  TEXTURE_BUFFER texbufs[2];

  char RomName[21];

  RDP();
  ~RDP();
  void Reset();
};


void SetWireframeCol ();
void ChangeSize ();
void GoToFullScreen();

extern RDP rdp;
extern SETTINGS settings;
extern HOTKEY_INFO hotkey_info;
extern VOODOO voodoo;

extern GrTexInfo  fontTex;
extern GrTexInfo  cursorTex;
extern wxUint32   offset_font;
extern wxUint32   offset_cursor;
extern wxUint32   offset_textures;
extern wxUint32   offset_texbuf1;

extern int	ucode_error_report;

/*
extern wxString pluginPath;
extern wxString iniPath;
*/
// RDP functions
void rdp_reset ();

extern const char *ACmp[];
extern const char *Mode0[];
extern const char *Mode1[];
extern const char *Mode2[];
extern const char *Mode3[];
extern const char *Alpha0[];
#define Alpha1 Alpha0
extern const char *Alpha2[];
#define Alpha3 Alpha0
extern const char *FBLa[];
extern const char *FBLb[];
extern const char *FBLc[];
extern const char *FBLd[];
extern const char *str_zs[];
extern const char *str_yn[];
extern const char *str_offon[];
extern const char *str_cull[];
// I=intensity probably
extern const char *str_format[];
extern const char *str_size[];
extern const char *str_cm[];
extern const char *str_lod[];
extern const char *str_aspect[];
extern const char *str_filter[];
extern const char *str_tlut[];
extern const char *CIStatus[];

#define FBL_D_1 2
#define FBL_D_0 3

#ifndef max
#define max(a,b)            (((a) > (b)) ? (a) : (b))
#endif
#ifndef min
#define min(a,b)            (((a) < (b)) ? (a) : (b))
#endif
#ifndef TRUE
#define TRUE 1
#endif
#ifndef FALSE
#define FALSE 0
#endif
#ifndef HIWORD
#define HIWORD(a) ((unsigned int)(a) >> 16)
#endif
#ifndef LOWORD
#define LOWORD(a) ((a) & 0xFFFF)
#endif

// Convert from u0/v0/u1/v1 to the real coordinates without regard to tmu
__inline void ConvertCoordsKeep (VERTEX *v, int n)
{
  for (int i=0; i<n; i++)
  {
    v[i].uc(0) = v[i].u0;
    v[i].vc(0) = v[i].v0;
    v[i].uc(1) = v[i].u1;
    v[i].vc(1) = v[i].v1;
  }
}

// Convert from u0/v0/u1/v1 to the real coordinates based on the tmu they are on
__inline void ConvertCoordsConvert (VERTEX *v, int n)
{
  for (int i=0; i<n; i++)
  {
    v[i].uc(rdp.t0) = v[i].u0;
    v[i].vc(rdp.t0) = v[i].v0;
    v[i].uc(rdp.t1) = v[i].u1;
    v[i].vc(rdp.t1) = v[i].v1;
  }
}

__inline void AllowShadeMods (VERTEX *v, int n)
{
  for (int i=0; i<n; i++)
  {
    v[i].shade_mod = 0;
  }
}

__inline void AddOffset (VERTEX *v, int n)
{
  for (int i=0; i<n; i++)
  {
    v[i].x += rdp.offset_x;
    v[i].y += rdp.offset_y;
  }
}

__inline void CalculateFog (VERTEX *v)
{
  if (rdp.flags & FOG_ENABLED)
  {
    if (v->w < 0.0f)
      v->f = 0.0f;
    else
      v->f = min(255.0f, max(0.0f, v->z_w * rdp.fog_multiplier + rdp.fog_offset));
    v->a = (wxUint8)v->f;
  }
  else
  {
    v->f = 1.0f;
  }
}

void newSwapBuffers();
extern int SwapOK;

// ** utility functions
void load_palette (wxUint32 addr, wxUint16 start, wxUint16 count);
void setTBufTex(wxUint16 t_mem, wxUint32 cnt);

#endif  // ifndef RDP_H
