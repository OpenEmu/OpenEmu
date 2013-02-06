/*
** THIS SOFTWARE IS SUBJECT TO COPYRIGHT PROTECTION AND IS OFFERED ONLY
** PURSUANT TO THE 3DFX GLIDE GENERAL PUBLIC LICENSE. THERE IS NO RIGHT
** TO USE THE GLIDE TRADEMARK WITHOUT PRIOR WRITTEN PERMISSION OF 3DFX
** INTERACTIVE, INC. A COPY OF THIS LICENSE MAY BE OBTAINED FROM THE 
** DISTRIBUTOR OR BY CONTACTING 3DFX INTERACTIVE INC(info@3dfx.com). 
** THIS PROGRAM IS PROVIDED "AS IS" WITHOUT WARRANTY OF ANY KIND, EITHER 
** EXPRESSED OR IMPLIED. SEE THE 3DFX GLIDE GENERAL PUBLIC LICENSE FOR A
** FULL TEXT OF THE NON-WARRANTY PROVISIONS.  
** 
** USE, DUPLICATION OR DISCLOSURE BY THE GOVERNMENT IS SUBJECT TO
** RESTRICTIONS AS SET FORTH IN SUBDIVISION (C)(1)(II) OF THE RIGHTS IN
** TECHNICAL DATA AND COMPUTER SOFTWARE CLAUSE AT DFARS 252.227-7013,
** AND/OR IN SIMILAR OR SUCCESSOR CLAUSES IN THE FAR, DOD OR NASA FAR
** SUPPLEMENT. UNPUBLISHED RIGHTS RESERVED UNDER THE COPYRIGHT LAWS OF
** THE UNITED STATES.  
** 
** COPYRIGHT 3DFX INTERACTIVE, INC. 1999, ALL RIGHTS RESERVED
*/

/*
** H3EXT.H
**
** The following #defines are relevant when using Glide:
**
** One of the following "platform constants" must be defined during
** compilation:
**
**            __DOS__           Defined for 32-bit DOS applications
**            __WIN32__         Defined for 32-bit Windows applications
**            __sparc__         Defined for Sun Solaris/SunOS
**            __linux__         Defined for Linux applications
**            __FreeBSD__       Defined for FreeBSD applications
**            __NetBSD__        Defined for NetBSD applications
**            __OpenBSD__       Defined for OpenBSD applications
**            __IRIX__          Defined for SGI Irix applications
**
*/
#ifndef __H3EXT_H__
#define __H3EXT_H__

#include <3dfx.h>
#include <glidesys.h>
#include <sst1vid.h>

#ifdef __cplusplus
extern "C" {
#endif

/*
** -----------------------------------------------------------------------
** TYPE DEFINITIONS
** -----------------------------------------------------------------------
*/
/*
** -----------------------------------------------------------------------
** CONSTANTS AND TYPES
** -----------------------------------------------------------------------
*/

/*
 * gregk  5/3/99
 * Constants defined for SSTH3_ALPHADITHERMODE  registry key
 */
 
#define OPTIMAL                 1
#define SHARPER                 2
#define SMOOTHER                3

/* tbext */
#define GR_BUFFER_TEXTUREBUFFER_EXT 0x6
#define GR_BUFFER_TEXTUREAUXBUFFER_EXT 0x7

typedef FxU32 GrPixelFormat_t;

#define GR_PIXFMT_I_8                           0x0001
#define GR_PIXFMT_AI_88                         0x0002
#define GR_PIXFMT_RGB_565                       0x0003
#define GR_PIXFMT_ARGB_1555                     0x0004
#define GR_PIXFMT_ARGB_8888                     0x0005
#define GR_PIXFMT_AA_2_RGB_565                  0x0006
#define GR_PIXFMT_AA_2_ARGB_1555                0x0007
#define GR_PIXFMT_AA_2_ARGB_8888                0x0008
#define GR_PIXFMT_AA_4_RGB_565                  0x0009
#define GR_PIXFMT_AA_4_ARGB_1555                0x000a
#define GR_PIXFMT_AA_4_ARGB_8888                0x000b
#define GR_PIXFMT_AA_8_RGB_565                  0x000c 	/* 8xaa */
#define GR_PIXFMT_AA_8_ARGB_1555                0x000d
#define GR_PIXFMT_AA_8_ARGB_8888                0x000e


#define GR_LFBWRITEMODE_Z32                     0x0008

typedef FxU32 GrAAMode_t;

#define GR_AA_NONE                              0x0000
#define GR_AA_4SAMPLES                          0x0001

typedef FxU8 GrStencil_t;

typedef FxU32 GrStencilOp_t;
#define GR_STENCILOP_KEEP        0x00              /* keep current value */
#define GR_STENCILOP_ZERO        0x01              /* set to zero */
#define GR_STENCILOP_REPLACE     0x02              /* replace with reference value */
#define GR_STENCILOP_INCR_CLAMP  0x03              /* increment - clamp */
#define GR_STENCILOP_DECR_CLAMP  0x04              /* decrement - clamp */
#define GR_STENCILOP_INVERT      0x05              /* bitwise inversion */
#define GR_STENCILOP_INCR_WRAP   0x06              /* increment - wrap */
#define GR_STENCILOP_DECR_WRAP   0x07              /* decrement - wrap */

#define GR_TEXTURE_UMA_EXT       0x06
#define GR_STENCIL_MODE_EXT      0x07
#define GR_OPENGL_MODE_EXT       0x08

typedef FxU32 GrCCUColor_t;

typedef FxU32 GrACUColor_t;

typedef FxU32 GrTCCUColor_t;

typedef FxU32 GrTACUColor_t;

#define GR_CMBX_ZERO                      0x00
#define GR_CMBX_TEXTURE_ALPHA             0x01
#define GR_CMBX_ALOCAL                    0x02
#define GR_CMBX_AOTHER                    0x03
#define GR_CMBX_B                         0x04
#define GR_CMBX_CONSTANT_ALPHA            0x05
#define GR_CMBX_CONSTANT_COLOR            0x06
#define GR_CMBX_DETAIL_FACTOR             0x07
#define GR_CMBX_ITALPHA                   0x08
#define GR_CMBX_ITRGB                     0x09
#define GR_CMBX_LOCAL_TEXTURE_ALPHA       0x0a
#define GR_CMBX_LOCAL_TEXTURE_RGB         0x0b
#define GR_CMBX_LOD_FRAC                  0x0c
#define GR_CMBX_OTHER_TEXTURE_ALPHA       0x0d
#define GR_CMBX_OTHER_TEXTURE_RGB         0x0e
#define GR_CMBX_TEXTURE_RGB               0x0f
#define GR_CMBX_TMU_CALPHA                0x10
#define GR_CMBX_TMU_CCOLOR                0x11

typedef FxU32 GrCombineMode_t;
#define GR_FUNC_MODE_ZERO                 0x00
#define GR_FUNC_MODE_X                    0x01
#define GR_FUNC_MODE_ONE_MINUS_X          0x02
#define GR_FUNC_MODE_NEGATIVE_X           0x03
#define GR_FUNC_MODE_X_MINUS_HALF         0x04

typedef FxU32 GrAlphaBlendOp_t;
#define GR_BLEND_OP_ADD                   0x00
#define GR_BLEND_OP_SUB                   0x01
#define GR_BLEND_OP_REVSUB                0x02

#define GR_BLEND_SAME_COLOR_EXT           0x08
#define GR_BLEND_ONE_MINUS_SAME_COLOR_EXT 0x09

/* Napalm extensions to GrTextureFormat_t */
#define GR_TEXFMT_ARGB_CMP_FXT1           0x11
#define GR_TEXFMT_ARGB_8888               0x12
#define GR_TEXFMT_YUYV_422                0x13
#define GR_TEXFMT_UYVY_422                0x14
#define GR_TEXFMT_AYUV_444                0x15
#define GR_TEXFMT_ARGB_CMP_DXT1           0x16
#define GR_TEXFMT_ARGB_CMP_DXT2           0x17
#define GR_TEXFMT_ARGB_CMP_DXT3           0x18
#define GR_TEXFMT_ARGB_CMP_DXT4           0x19
#define GR_TEXFMT_ARGB_CMP_DXT5           0x1A
#define GR_TEXTFMT_RGB_888                0xFF

/* Napalm extensions to GrLOD_t */
#define GR_LOD_LOG2_2048        0xb
#define GR_LOD_LOG2_1024        0xa
#define GR_LOD_LOG2_512         0x9

/* Napalm extensions to GrTexBaseRange_t */
#define GR_TEXBASE_2048     0x7
#define GR_TEXBASE_1024     0x6
#define GR_TEXBASE_512      0x5
#define GR_TEXBASE_256_TO_1 0x4

#ifdef __cplusplus
}
#endif

#include <glideutl.h>

#endif /* __H3EXT_H__ */
