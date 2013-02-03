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
** $Header: /cvsroot/glide/glide3x/h5/incsrc/sst1vid.h,v 1.3.4.1 2003/04/06 18:23:10 koolsmoky Exp $
** $Log: 
**  7    3dfx      1.4.1.0.1.0 10/11/00 Brent           Forced check in to enforce
**       branching.
**  6    3dfx      1.4.1.0     06/20/00 Joseph Kain     Changes to support the
**       Napalm Glide open source release.  Changes include cleaned up offensive
**       comments and new legal headers.
**  5    3dfx      1.4         12/10/99 Leo Galway      Removed previous hi-res
**       mode information for Glide3. These modes were only necessary for
**       Cornerstone (or future hi-res) support in RT4.2 source branch and
**       proceeded to break the V3 and V2 builds (from 3dfx view), hence they have
**       been removed.
**  4    3dfx      1.3         12/08/99 Leo Galway      Added mode information for
**       1600x1280, 1792x1440, 1920x1080, 1920x1200, 2046x1536 (as a result of
**       glide being tested with Cornerstone modes). Although not all of these
**       modes are currently capable under Glide, their inclusion prevents Glide
**       apps from displaying in incorrect modes when these hi-res modes are
**       selected. Search for SUSTAINED_ENGINEERING_CHANGE_BEGIN. 
**  3    3dfx      1.2         09/17/99 Jeremy Zelsnack 
**  2    3dfx      1.1         09/17/99 Jeremy Zelsnack 
**  1    3dfx      1.0         09/11/99 StarTeam VTS Administrator 
** $
** 
** 8     3/04/99 1:19p Atai
** sync new res modes
** 
** 10    2/27/99 12:28p Dow
** new resolutions
** 
** 6     2/13/99 1:56p Dow
** Added new resolution constants
** 
** 5     7/24/98 1:38p Hohn
 * 
 * 4     9/09/97 7:35p Sellers
 * Added 400x300 resolution
 * 
 * 3     8/24/97 9:31a Sellers
 * moved new video timing to sst1vid.h
 * redefined 1600x1280 to be 1600x1200
 * 
 * 2     6/05/97 11:14p Pgj
 * 
 * 5     7/24/96 3:43p Sellers
 * added 512x384 @ 60 Hz for arcade monitors
 * added 512x256 @ 60 Hz for arcade monitors
 * 
 * 4     7/18/96 10:58a Sellers
 * fixed FT and TF clock delay values for lower frequencies with
 * .5/.5 combos
 * 
 * 3     6/18/96 6:54p Sellers
 * added sst1InitShutdownSli() to fix Glide Splash screen problems with
 * SLI
 * 
 * 2     6/13/96 7:45p Sellers
 * added "voodoo.ini" support
 * added DirectX support
 * misc cleanup
 * 
 * 2     6/11/96 1:43p Sellers
 * added support for 60, 75, 85, and 120 Hz refresh rates for "most"
 * resolutions
 * 
 * 1     5/08/96 5:43p Paik
 * Video definitions
*/
#ifndef __SST1VID_H__
#define __SST1VID_H__

#ifdef __cplusplus
extern "C" {
#endif

/* Video defines */

typedef FxI32 GrScreenRefresh_t;
#define GR_REFRESH_60Hz   0x0
#define GR_REFRESH_70Hz   0x1
#define GR_REFRESH_72Hz   0x2
#define GR_REFRESH_75Hz   0x3
#define GR_REFRESH_80Hz   0x4
#define GR_REFRESH_90Hz   0x5
#define GR_REFRESH_100Hz  0x6
#define GR_REFRESH_85Hz   0x7
#define GR_REFRESH_120Hz  0x8
#define GR_REFRESH_NONE   0xff

typedef FxI32 GrScreenResolution_t;
#define GR_RESOLUTION_320x200   0x0
#define GR_RESOLUTION_320x240   0x1
#define GR_RESOLUTION_400x256   0x2
#define GR_RESOLUTION_512x384   0x3
#define GR_RESOLUTION_640x200   0x4
#define GR_RESOLUTION_640x350   0x5
#define GR_RESOLUTION_640x400   0x6
#define GR_RESOLUTION_640x480   0x7
#define GR_RESOLUTION_800x600   0x8
#define GR_RESOLUTION_960x720   0x9
#define GR_RESOLUTION_856x480   0xa
#define GR_RESOLUTION_512x256   0xb
#define GR_RESOLUTION_1024x768  0xC
#define GR_RESOLUTION_1280x1024 0xD
#define GR_RESOLUTION_1600x1200 0xE
#define GR_RESOLUTION_400x300   0xF
#define GR_RESOLUTION_1152x864  0x10
#define GR_RESOLUTION_1280x960  0x11
#define GR_RESOLUTION_1600x1024 0x12
#define GR_RESOLUTION_1792x1344 0x13
#define GR_RESOLUTION_1856x1392 0x14
#define GR_RESOLUTION_1920x1440 0x15
#define GR_RESOLUTION_2048x1536 0x16
#define GR_RESOLUTION_2048x2048 0x17
#define GR_RESOLUTION_NONE      0xff

#ifdef GR_RESOLUTION_MAX
#undef GR_RESOLUTION_MAX
#endif
#ifdef GR_RESOLUTION_MIN
#undef GR_RESOLUTION_MIN
#endif
#define GR_RESOLUTION_MIN       GR_RESOLUTION_320x200
#define GR_RESOLUTION_MAX       GR_RESOLUTION_2048x2048

#ifdef __cplusplus
}
#endif

#endif /* __SST1VID_H__ */
