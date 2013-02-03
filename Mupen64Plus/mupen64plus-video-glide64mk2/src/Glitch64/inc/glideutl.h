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
**
** $Header: /cvsroot/glide/glide3x/h5/glide3/src/glideutl.h,v 1.3.4.2 2003/06/05 08:23:53 koolsmoky Exp $
** $Log: 
**  3    3dfx      1.0.1.0.1.0 10/11/00 Brent           Forced check in to enforce
**       branching.
**  2    3dfx      1.0.1.0     06/20/00 Joseph Kain     Changes to support the
**       Napalm Glide open source release.  Changes include cleaned up offensive
**       comments and new legal headers.
**  1    3dfx      1.0         09/11/99 StarTeam VTS Administrator 
** $
** 
** 4     7/24/98 1:41p Hohn
** 
** 3     1/30/98 4:27p Atai
** gufog* prototype
** 
** 1     1/29/98 4:00p Atai
 * 
 * 1     1/16/98 4:29p Atai
 * create glide 3 src
 * 
 * 11    1/07/98 11:18a Atai
 * remove GrMipMapInfo and GrGC.mm_table in glide3
 * 
 * 10    1/06/98 6:47p Atai
 * undo grSplash and remove gu routines
 * 
 * 9     1/05/98 6:04p Atai
 * move 3df gu related data structure from glide.h to glideutl.h
 * 
 * 8     12/18/97 2:13p Peter
 * fogTable cataclysm
 * 
 * 7     12/15/97 5:52p Atai
 * disable obsolete glide2 api for glide3
 * 
 * 6     8/14/97 5:32p Pgj
 * remove dead code per GMT
 * 
 * 5     6/12/97 5:19p Pgj
 * Fix bug 578
 * 
 * 4     3/05/97 9:36p Jdt
 * Removed guFbWriteRegion added guEncodeRLE16
 * 
 * 3     1/16/97 3:45p Dow
 * Embedded fn protos in ifndef FX_GLIDE_NO_FUNC_PROTO 
*/

/* Glide Utility routines */

#ifndef __GLIDEUTL_H__
#define __GLIDEUTL_H__

#ifdef __cplusplus
extern "C" {
#endif

/*
** 3DF texture file structs
*/

typedef struct
{
  FxU32               width, height;
  int                 small_lod, large_lod;
  GrAspectRatio_t     aspect_ratio;
  GrTextureFormat_t   format;
} Gu3dfHeader;

typedef struct
{
  FxU8  yRGB[16];
  FxI16 iRGB[4][3];
  FxI16 qRGB[4][3];
  FxU32 packed_data[12];
} GuNccTable;

typedef struct {
    FxU32 data[256];
} GuTexPalette;

typedef union {
    GuNccTable   nccTable;
    GuTexPalette palette;
} GuTexTable;

typedef struct
{
  Gu3dfHeader  header;
  GuTexTable   table;
  void        *data;
  FxU32        mem_required;    /* memory required for mip map in bytes. */
} Gu3dfInfo;

#ifndef FX_GLIDE_NO_FUNC_PROTO
/*
** Gamma functions
*/

FX_ENTRY void FX_CALL 
guGammaCorrectionRGB( FxFloat red, FxFloat green, FxFloat blue );

/*
** fog stuff
*/
FX_ENTRY float FX_CALL
guFogTableIndexToW( int i );

FX_ENTRY void FX_CALL
guFogGenerateExp( GrFog_t *fogtable, float density );

FX_ENTRY void FX_CALL
guFogGenerateExp2( GrFog_t *fogtable, float density );

FX_ENTRY void FX_CALL
guFogGenerateLinear(GrFog_t *fogtable,
                    float nearZ, float farZ );

/*
** hi-level texture manipulation tools.
*/
FX_ENTRY FxBool FX_CALL
gu3dfGetInfo( const char *filename, Gu3dfInfo *info );

FX_ENTRY FxBool FX_CALL
gu3dfLoad( const char *filename, Gu3dfInfo *data );

#endif /* FX_GLIDE_NO_FUNC_PROTO */

#ifdef __cplusplus
}
#endif

#endif /* __GLIDEUTL_H__ */
