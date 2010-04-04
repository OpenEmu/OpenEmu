/* Mednafen - Multi-system Emulator
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
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

#include <string.h>

#include "main.h"
#include "video.h"
#include "opengl.h"
#include "nongl.h"

#include "icon.h"
#include "netplay.h"
#include "cheat.h"

#include "scalebit.h"
#include "hqxx-common.h"
#include "nnx.h"
#include "debugger.h"
#include "fps.h"
#include "help.h"
#include "video-state.h"
#include "../video/selblur.h"

typedef struct
{
        int xres;
        int yres;
        double xscale, xscalefs;
        double yscale, yscalefs;
        int videoip;
        int stretch;
        int special;
        int scanlines;
	std::string pixshader;
} CommonVS;

static CommonVS _video;
static int _fullscreen;

typedef struct
{
	const char *name;
	int id;
	int xscale;
	int yscale;
} ScalerDefinition;

static ScalerDefinition Scalers[] = 
{

	{"hq2x", NTVB_HQ2X, 2, 2 },
	{"hq3x", NTVB_HQ3X, 3, 3 },
	{"hq4x", NTVB_HQ4X, 4, 4 },

	{"scale2x", NTVB_SCALE2X, 2, 2 },
	{"scale3x", NTVB_SCALE3X, 3, 3 },
	{"scale4x", NTVB_SCALE4X, 4, 4 },

	{"nn2x", NTVB_NN2X, 2, 2 },
        {"nn3x", NTVB_NN3X, 3, 3 },
        {"nn4x", NTVB_NN4X, 4, 4 },

	{"nny2x", NTVB_NNY2X, 1, 2 },
	{"nny3x", NTVB_NNY3X, 1, 3 },
	{"nny4x", NTVB_NNY4X, 1, 4 },
	{ 0 }
};

static MDFNGI *VideoGI;

static int cur_xres, cur_yres, cur_flags;

static ScalerDefinition *CurrentScaler = NULL;

static SDL_Surface *source_surface;
static SDL_Surface *screen = NULL;
static SDL_Surface *IconSurface=NULL;

static SDL_Rect src_rect;
static SDL_Rect dest_rect;

static SDL_Surface *DebuggerSurface;
static SDL_Rect DebuggerRect;

static SDL_Surface *NetSurface;
static SDL_Rect NetRect;

static SDL_Surface *CheatSurface;
static SDL_Rect CheatRect;

static SDL_Surface *HelpSurface;
static SDL_Rect HelpRect;

static SDL_Surface *SMSurface;
static SDL_Rect SMRect, SMDRect;


static int curbpp;

static double exs,eys;
static int exres, eyres;
static int evideoip;

static int NeedClear = 0;

void ClearBackBuffer(void)
{
 if(cur_flags & SDL_OPENGL)
 {
  ClearBackBufferOpenGL(screen);
 }
 else
 {
  SDL_FillRect(screen, NULL, 0);
 }
}

/* Return 1 if video was killed, 0 otherwise(video wasn't initialized). */
void KillVideo(void)
{
 if(IconSurface)
 {
  SDL_FreeSurface(IconSurface);
  IconSurface = NULL;
 }

 if(source_surface)
 {
  source_surface->pixels = NULL;
  SDL_FreeSurface(source_surface);
  source_surface = NULL;
 }

 if(DebuggerSurface)
 {
  SDL_FreeSurface(DebuggerSurface);
  DebuggerSurface = NULL;
 }

 if(SMSurface)
 {
  SDL_FreeSurface(SMSurface);
  SMSurface = NULL;
 }

 if(CheatSurface)
 {
  SDL_FreeSurface(CheatSurface);
  CheatSurface = NULL;
 }

 if(HelpSurface)
 {
  SDL_FreeSurface(HelpSurface);
  HelpSurface = NULL;
 }

 if(cur_flags & SDL_OPENGL)
  KillOpenGL();

 VideoGI = NULL;
 cur_flags = 0;
}

static void GenerateRects(MDFN_Rect &DisplayRect)
{
 src_rect.x = DisplayRect.x;
 src_rect.w = DisplayRect.w;

 src_rect.y = DisplayRect.y;
 src_rect.h = DisplayRect.h;

 if(_video.stretch && _fullscreen)
 {
  src_rect.x = DisplayRect.x;
  src_rect.w = DisplayRect.w;

  src_rect.y = DisplayRect.y;
  src_rect.h = DisplayRect.h;

  dest_rect.x = 0;
  dest_rect.w = exres;
  exs = (double)exres / src_rect.w;

  dest_rect.y = 0;
  dest_rect.h = eyres;
  eys = (double)eyres / src_rect.h;
 }
 else
 {
  if(VideoGI->rotated)
  {
   int ny = (int)((eyres - DisplayRect.w * exs) / 2);
   int nx = (int)((exres - src_rect.h * eys) / 2);

   if(ny < 0) ny = 0;
   if(nx < 0) nx = 0;

   dest_rect.x = _fullscreen ? nx : 0;
   dest_rect.y = _fullscreen ? ny : 0;
   dest_rect.w = (Uint16)(src_rect.h * eys);
   dest_rect.h = (Uint16)(DisplayRect.w * exs);
  }
  else
  {
   if(_fullscreen)
    dest_rect.x = (Uint16)((exres - VideoGI->width * exs) / 2);
   else
    dest_rect.x = 0;

   dest_rect.w = (Uint16)(VideoGI->width * exs);

   int ny = (int)((eyres - src_rect.h * eys) / 2);

   if(ny < 0) ny = 0;

   dest_rect.y = _fullscreen ? ny : 0;
   dest_rect.h = (Uint16)(src_rect.h * eys);
  }
 }
}

int VideoResize(int nw, int nh)
{
 return(1);
}

int GetSpecialScalerID(const std::string &special_string)
{
 int ret = -1;

 if(special_string == "" || !strcasecmp(special_string.c_str(), "none") || special_string == "0")
  ret = 0;
 else
 {
  ScalerDefinition *scaler = Scalers;

  while(scaler->name)
  {
   char tmpstr[16];

   sprintf(tmpstr, "%d", scaler->id);

   if(!strcasecmp(scaler->name, special_string.c_str()) || tmpstr == special_string)
   {
    ret = scaler->id;
    break;
   }
   scaler++;
  }
 }
 return(ret);
}

bool MDFND_ValidateVideoSetting(const char *name, const char *value)
{
 if(!strcasecmp(name, "vdriver"))
 {
  if(strcasecmp(value, "0") && strcasecmp(value, "1") && strcasecmp(value, "opengl") && strcasecmp(value, "sdl"))
   return(FALSE);
 }
 return(TRUE);
}


bool MDFND_ValidateSpecialScalerSetting(const char *name, const char *value)
{
 if(GetSpecialScalerID(value) < 0)
  return(0);

 return(1);
}


static uint32 real_rs, real_gs, real_bs, real_as;

int InitVideo(MDFNGI *gi)
{
 const SDL_VideoInfo *vinf;
 int flags=0;
 int desbpp;

 VideoGI = gi;

 MDFNI_printf(_("Initializing video...\n"));
 MDFN_indent(1);

 std::string sn = std::string(gi->shortname);

 if(gi->GameType == GMT_PLAYER)
  sn = "player";

 std::string special_string = MDFN_GetSettingS(std::string(sn + "." + std::string("special")).c_str());

 _fullscreen = MDFN_GetSettingB("fs");
 _video.xres = MDFN_GetSettingUI(std::string(sn + "." + std::string("xres")).c_str());
 _video.yres = MDFN_GetSettingUI(std::string(sn + "." + std::string("yres")).c_str());
 _video.xscale = MDFN_GetSettingF(std::string(sn + "." + std::string("xscale")).c_str());
 _video.yscale = MDFN_GetSettingF(std::string(sn + "." + std::string("yscale")).c_str());
 _video.xscalefs = MDFN_GetSettingF(std::string(sn + "." + std::string("xscalefs")).c_str());
 _video.yscalefs = MDFN_GetSettingF(std::string(sn + "." + std::string("yscalefs")).c_str());
 _video.videoip = MDFN_GetSettingB(std::string(sn + "." + std::string("videoip")).c_str());
 _video.stretch = MDFN_GetSettingB(std::string(sn + "." + std::string("stretch")).c_str());
 _video.scanlines = MDFN_GetSettingUI(std::string(sn + "." + std::string("scanlines")).c_str());

 _video.special = GetSpecialScalerID(special_string);

 #ifdef MDFN_WANT_OPENGL_SHADERS
 _video.pixshader = MDFN_GetSettingS(std::string(sn + "." + std::string("pixshader")).c_str());
 #else
 _video.pixshader = "none";
 #endif

 CurrentScaler = _video.special ? &Scalers[_video.special - 1] : NULL;

 vinf=SDL_GetVideoInfo();

 if(vinf->hw_available)
  flags|=SDL_HWSURFACE;

 if(_fullscreen)
  flags|=SDL_FULLSCREEN;

 const std::string vdriverstr = MDFN_GetSettingS("vdriver");
 unsigned int vdriver = 0;

 if(!strcasecmp(vdriverstr.c_str(), "0") || !strcasecmp(vdriverstr.c_str(), "opengl"))
  vdriver = 0;
 else if(!strcasecmp(vdriverstr.c_str(), "1") || !strcasecmp(vdriverstr.c_str(), "sdl"))
  vdriver = 1;
 else
 {
  puts("Inconceivable!");
  exit(-1);
 }

 if(vdriver == 0)
 {
  if(!sdlhaveogl)
  {
   // SDL_GL_LoadLibrary returns 0 on success, -1 on failure
   if(SDL_GL_LoadLibrary(NULL) == 0)
    sdlhaveogl = 1;
   else
    sdlhaveogl = 0;
  }

  if(sdlhaveogl)
   flags |= SDL_OPENGL;
  else
  {
   MDFN_PrintError(_("Could not load OpenGL library, disabling OpenGL usage!"));
   vdriver = 0;
  }
 }

 if(vdriver == 0)
 {
  SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1 );

  if(MDFN_GetSettingB("glvsync"))
  {
   #if SDL_VERSION_ATLEAST(1, 2, 10)
   SDL_GL_SetAttribute(SDL_GL_SWAP_CONTROL, 1);
   #endif
  }
 }
 else
  flags |= SDL_DOUBLEBUF;

 exres = _video.xres;
 eyres = _video.yres;
 exs = _fullscreen ? _video.xscalefs : _video.xscale;
 eys = _fullscreen ? _video.yscalefs : _video.yscale;
 evideoip = _video.videoip;

 desbpp = 32;

 GenerateRects(VideoGI->DisplayRect);

 if(exs > 20)
 {
  MDFND_PrintError(_("Eep!  Effective X scale setting is way too high.  Correcting."));
  exs = 20;
  GenerateRects(VideoGI->DisplayRect);
 }
 
 if(eys > 20)
 {
  MDFND_PrintError(_("Eep!  Effective Y scale setting is way too high.  Correcting."));
  eys = 20;
  GenerateRects(VideoGI->DisplayRect);
 }

 if(_fullscreen)
 {
  if(!screen || cur_xres != exres || cur_yres != eyres || cur_flags != flags || curbpp != desbpp)
  {
   //MDFNI_DispMessage("%d %d %d %d %d %d",cur_xres, _xres, cur_yres, _yres, curbpp, desbpp);
   if(!(screen = SDL_SetVideoMode(exres, eyres, desbpp, flags)))
   {
    MDFND_PrintError(SDL_GetError()); 

    /* A last-ditch effort. */
    if(exres != 640 || eyres != 480)
    {
     if(exs > 2) exs = 2;
     if(eys > 2) eys = 2;
     exres = 640;
     eyres = 480;
     GenerateRects(VideoGI->DisplayRect);
    }
    MDFN_indent(-1);
    return(0);
   }
  }
  cur_xres = exres;
  cur_yres = eyres;
 }
 else
 {
  if(!screen || cur_xres != dest_rect.w || cur_yres != dest_rect.h || cur_flags != flags || curbpp != desbpp)
   screen = SDL_SetVideoMode(dest_rect.w, dest_rect.h, desbpp, flags);
  cur_xres = dest_rect.w;
  cur_yres = dest_rect.h;
 }

 if(!screen)
 {
  MDFND_PrintError(SDL_GetError());
  KillVideo();
  MDFN_indent(-1);
  return(0);
 }
 cur_flags = flags;
 curbpp = screen->format->BitsPerPixel;

 MDFN_printf(_("Video Mode: %d x %d x %d bpp\n"),screen->w,screen->h,screen->format->BitsPerPixel);
 if(curbpp!=16 && curbpp!=24 && curbpp!=32)
 {
  MDFN_printf(_("Sorry, %dbpp modes are not supported by Mednafen.  Supported bit depths are 16bpp, 24bpp, and 32bpp.\n"),curbpp);
  KillVideo();
  MDFN_indent(-1);
  return(0);
 }
 MDFN_printf(_("OpenGL: %s\n"), (cur_flags & SDL_OPENGL) ? _("Yes") : _("No"));

 if(cur_flags & SDL_OPENGL)
 {
  MDFN_indent(1);
  MDFN_printf(_("Pixel shader: %s\n"), _video.pixshader.c_str());
  MDFN_indent(-1);
 }

 MDFN_printf(_("Fullscreen: %s\n"), _fullscreen ? _("Yes") : _("No"));
 MDFN_printf(_("Special Scaler: %s\n"), _video.special ? Scalers[_video.special - 1].name : _("None"));
 if(!_video.scanlines)
  MDFN_printf(_("Scanlines: Off\n"));
 else
  MDFN_printf(_("Scanlines: %f%% transparency\n"), 100.0f - (100.0f / _video.scanlines));

 MDFN_printf(_("Destination Rectangle: X=%d, Y=%d, W=%d, H=%d\n"), dest_rect.x, dest_rect.y, dest_rect.w, dest_rect.h);
 if(dest_rect.x < 0 || dest_rect.y < 0 || (dest_rect.x + dest_rect.w) > screen->w || (dest_rect.y + dest_rect.h) > screen->h)
 {
  MDFN_indent(1);
  if(cur_flags & SDL_OPENGL)   
   MDFN_printf(_("Warning:  Destination rectangle exceeds screen dimensions.  This is ok if you really do want the clipping...\n"));
  else // NonOpenGL mode doesn't support this kind of screen overflow clipping
   MDFN_printf(_("Warning:  Destination rectangle exceeds screen dimensions.  The rectangle will be adjusted to fit within the screen area.\n"));
  MDFN_indent(-1);
 }
 if(gi && gi->name)
  SDL_WM_SetCaption((char *)gi->name,(char *)gi->name);
 else
  SDL_WM_SetCaption("Mednafen","Mednafen");

 #ifdef WIN32
  #ifdef LSB_FIRST
  IconSurface=SDL_CreateRGBSurfaceFrom((void *)mednafen_playicon.pixel_data,32,32,32,32*4,0xFF,0xFF00,0xFF0000,0xFF000000);
  #else
  IconSurface=SDL_CreateRGBSurfaceFrom((void *)mednafen_playicon.pixel_data,32,32,32,32*4,0xFF000000,0xFF0000,0xFF00,0xFF);
  #endif
 #else
  #ifdef LSB_FIRST
  IconSurface=SDL_CreateRGBSurfaceFrom((void *)mednafen_playicon128.pixel_data,128,128,32,128*4,0xFF,0xFF00,0xFF0000,0xFF000000);
  #else
  IconSurface=SDL_CreateRGBSurfaceFrom((void *)mednafen_playicon128.pixel_data,128,128,32,128*4,0xFF000000,0xFF0000,0xFF00,0xFF);
  #endif
 #endif
 SDL_WM_SetIcon(IconSurface,0);

 if(cur_flags & SDL_OPENGL)
 {
  if(!InitOpenGL(evideoip, _video.scanlines, _video.pixshader, screen))
  {
   KillVideo();
   MDFN_indent(-1);
   return(0);
  }
 }
 else
 {



 }

 MDFN_indent(-1);
 SDL_ShowCursor(0);

 int rs, gs, bs, as;

 if(cur_flags & SDL_OPENGL)
 {
  #ifdef LSB_FIRST
  rs = 0;
  gs = 8;
  bs = 16;
  as = 24;
  #else
  rs = 24;
  gs = 16;
  bs = 8;
  as = 0;
  #endif
 }
 else
 {
  rs = screen->format->Rshift;
  gs = screen->format->Gshift;
  bs = screen->format->Bshift;

  as = 0;
  while(as == rs || as == gs || as == bs) // Find unused 8-bits to use as our alpha channel
   as += 8;
 }

 real_rs = rs;
 real_gs = gs;
 real_bs = bs;
 real_as = as;

 /* HQXX only supports this pixel format, sadly, and other pixel formats
    can't be easily supported without rewriting the filters.
    We do conversion to the real screen format in the blitting function. 
 */
 if(CurrentScaler)
  if(CurrentScaler->id == NTVB_HQ2X || CurrentScaler->id == NTVB_HQ3X || CurrentScaler->id == NTVB_HQ4X)
  {
   rs = 0;
   gs = 8;
   bs = 16;
   as = 24;
  }
 source_surface = SDL_CreateRGBSurfaceFrom(NULL, gi->pitch / sizeof(uint32), 256, 32, gi->pitch, 0xFF << rs, 0xFF << gs, 0xFF << bs, 0);

 NetSurface = SDL_CreateRGBSurface(SDL_SWSURFACE | SDL_SRCALPHA, screen->w, 18 * 5, 32, 0xFF << real_rs, 0xFF << real_gs, 0xFF << real_bs, 0xFF << real_as);
 SDL_SetColorKey(NetSurface, SDL_SRCCOLORKEY, 0);
 SDL_SetAlpha(NetSurface, SDL_SRCALPHA, 0);

 NetRect.w = screen->w;
 NetRect.h = 18 * 5;
 NetRect.x = 0;
 NetRect.y = 0;


 {
  int xmu = 1;
  int ymu = 1;

  if(screen->w >= 768)
   xmu = screen->w / 384;
  if(screen->h >= 576)
   ymu = screen->h / 288;

  SMRect.h = 18 + 2;
  SMRect.x = 0;
  SMRect.y = 0;
  SMRect.w = screen->w;

  SMDRect.w = SMRect.w * xmu;
  SMDRect.h = SMRect.h * ymu;
  SMDRect.x = (screen->w - SMDRect.w) / 2;
  SMDRect.y = screen->h - SMDRect.h;

  if(SMDRect.x < 0)
  {
   SMRect.w += SMDRect.x * 2 / xmu;
   SMDRect.w = SMRect.w * xmu;
   SMDRect.x = 0;
  }
  SMSurface = SDL_CreateRGBSurface(SDL_SWSURFACE | SDL_SRCALPHA, SMRect.w, SMRect.h, 32, 0xFF << real_rs, 0xFF << real_gs, 0xFF << real_bs, 0xFF << real_as);
  SDL_SetColorKey(SMSurface, SDL_SRCCOLORKEY, 0);
 }

 MDFNI_SetPixelFormat(rs, gs, bs, as);

 for(int i = 0; i < 2; i++)
 {
  ClearBackBuffer();

  if(cur_flags & SDL_OPENGL)
   FlipOpenGL();
  else
   SDL_Flip(screen);
 }

 return 1;
}

static uint32 howlong = 0;
static UTF8 *CurrentMessage = NULL;

void VideoShowMessage(UTF8 *text)
{
 if(text)
  howlong = MDFND_GetTime() + 2500;
 else
  howlong = 0;
 CurrentMessage = text;
}

#define MK_COLOR_A(tmp_surface, r,g,b,a) ( ((a)<<tmp_surface->format->Ashift) | ((r)<<tmp_surface->format->Rshift) | ((g) << tmp_surface->format->Gshift) | ((b) << tmp_surface->format->Bshift))

void BlitRaw(SDL_Surface *src, SDL_Rect *src_rect, SDL_Rect *dest_rect)
{
 if(cur_flags & SDL_OPENGL)
  BlitOpenGLRaw(src, src_rect, dest_rect);
 else 
 {
  BlitNonGL(src, src_rect, src_rect, dest_rect, screen, 0, 0);

  //SDL_BlitSurface(src, src_rect, screen, dest_rect);
 }
 NeedClear = 2;
}


static bool BlitInternalMessage(void)
{
 if(howlong < MDFND_GetTime())
 {
  if(CurrentMessage)
  {
   free(CurrentMessage);
   CurrentMessage = NULL;
  }
  return(0);
 }

 if(CurrentMessage)
 {
  SDL_FillRect(SMSurface, NULL, MK_COLOR_A(SMSurface, 0x00, 0x00, 0x00, 0xC0));
  DrawTextTransShadow((uint32 *)((uint8 *)SMSurface->pixels + SMSurface->pitch * 1), SMSurface->pitch, SMRect.w, CurrentMessage, 
	MK_COLOR_A(SMSurface, 0xFF, 0xFF, 0xFF, 0xFF), MK_COLOR_A(SMSurface, 0x00, 0x00, 0x00, 0xFF), TRUE);
  free(CurrentMessage);
  CurrentMessage = NULL;
 }

 BlitRaw(SMSurface, &SMRect, &SMDRect);
 return(1);
}


static void SubBlit(bool alpha_blend)
{
 SDL_Surface *eff_source_surface = source_surface;
 SDL_Rect eff_src_rect = src_rect;
 SDL_Surface *tmp_blur_surface = NULL;

 // Handle selective blur first
 if(0)
 {
  SelBlurImage sb_spec;

  tmp_blur_surface = SDL_CreateRGBSurface(SDL_SWSURFACE, src_rect.w, src_rect.h, source_surface->format->BitsPerPixel, source_surface->format->Rmask, source_surface->format->Gmask, source_surface->format->Bmask, source_surface->format->Amask);
  SDL_SetAlpha(tmp_blur_surface, 0, SDL_ALPHA_OPAQUE);

  sb_spec.red_threshold = 8;
  sb_spec.green_threshold = 7;
  sb_spec.blue_threshold = 10;
  sb_spec.radius = 3;
  sb_spec.source = (uint32 *)source_surface->pixels + eff_src_rect.x + eff_src_rect.y * (source_surface->pitch / 4);
  sb_spec.source_pitch32 = source_surface->pitch / 4;
  sb_spec.dest = (uint32 *)tmp_blur_surface->pixels;
  sb_spec.dest_pitch32 = tmp_blur_surface->pitch / 4;
  sb_spec.width = eff_src_rect.w;
  sb_spec.height = eff_src_rect.h;
  sb_spec.red_shift = source_surface->format->Rshift;
  sb_spec.green_shift = source_surface->format->Gshift;
  sb_spec.blue_shift = source_surface->format->Bshift;

  MDFN_SelBlur(&sb_spec);

  eff_source_surface = tmp_blur_surface;
  eff_src_rect.x = 0;
  eff_src_rect.y = 0;
 }

   if(CurrentScaler)
   {
    uint8 *screen_pixies;
    uint32 screen_pitch;
    SDL_Surface *bah_surface = NULL;
    SDL_Rect boohoo_rect = eff_src_rect;

    boohoo_rect.x = boohoo_rect.y = 0;
    boohoo_rect.w *= CurrentScaler->xscale;
    boohoo_rect.h *= CurrentScaler->yscale;

    bah_surface = SDL_CreateRGBSurface(SDL_SWSURFACE, boohoo_rect.w, boohoo_rect.h, eff_source_surface->format->BitsPerPixel, eff_source_surface->format->Rmask, eff_source_surface->format->Gmask, eff_source_surface->format->Bmask, eff_source_surface->format->Amask);

    screen_pixies = (uint8 *)bah_surface->pixels;
    screen_pitch = bah_surface->pitch;

    if(CurrentScaler->id == NTVB_SCALE4X || CurrentScaler->id == NTVB_SCALE3X || CurrentScaler->id == NTVB_SCALE2X)
    {
     uint8 *source_pixies = (uint8 *)eff_source_surface->pixels + eff_src_rect.x * eff_source_surface->format->BytesPerPixel + eff_src_rect.y * eff_source_surface->pitch;
     scale((CurrentScaler->id ==  NTVB_SCALE2X)?2:(CurrentScaler->id == NTVB_SCALE4X)?4:3, screen_pixies, screen_pitch, source_pixies, eff_source_surface->pitch, eff_source_surface->format->BytesPerPixel, eff_src_rect.w, eff_src_rect.h);
    }
    else if(CurrentScaler->id == NTVB_NN2X || CurrentScaler->id == NTVB_NN3X || CurrentScaler->id == NTVB_NN4X)
    {
     nnx(CurrentScaler->id - NTVB_NN2X + 2, eff_source_surface, &eff_src_rect, bah_surface, &boohoo_rect);
    }
    else if(CurrentScaler->id == NTVB_NNY2X || CurrentScaler->id == NTVB_NNY3X || CurrentScaler->id == NTVB_NNY4X)
    {
     nnyx(CurrentScaler->id - NTVB_NNY2X + 2, eff_source_surface, &eff_src_rect, bah_surface, &boohoo_rect);
    }
    else 
    {
     uint8 *source_pixies = (uint8 *)eff_source_surface->pixels + eff_src_rect.x * eff_source_surface->format->BytesPerPixel + eff_src_rect.y * eff_source_surface->pitch;

     if(CurrentScaler->id == NTVB_HQ2X)
      hq2x_32(source_pixies, screen_pixies, eff_src_rect.w, eff_src_rect.h, eff_source_surface->pitch, screen_pitch);
     else if(CurrentScaler->id == NTVB_HQ3X)
      hq3x_32(source_pixies, screen_pixies, eff_src_rect.w, eff_src_rect.h, eff_source_surface->pitch, screen_pitch);
     else if(CurrentScaler->id == NTVB_HQ4X)
      hq4x_32(source_pixies, screen_pixies, eff_src_rect.w, eff_src_rect.h, eff_source_surface->pitch, screen_pitch);

     if(bah_surface->format->Rshift != real_rs || bah_surface->format->Gshift != real_gs ||
  	bah_surface->format->Bshift != real_bs)
     {
      uint32 *lineptr = (uint32 *)bah_surface->pixels;
      unsigned int srs = bah_surface->format->Rshift;
      unsigned int sgs = bah_surface->format->Gshift;
      unsigned int sbs = bah_surface->format->Bshift;
      unsigned int drs = real_rs;
      unsigned int dgs = real_gs;
      unsigned int dbs = real_bs;

      for(int y = 0; y < boohoo_rect.h; y++)
      {
       for(int x = 0; x < boohoo_rect.w; x++)
       {
        uint32 pixel = lineptr[x];
        lineptr[x] = (((pixel >> srs) & 0xFF) << drs) | (((pixel >> sgs) & 0xFF) << dgs) | (((pixel >> sbs) & 0xFF) << dbs);
       }
       lineptr += bah_surface->pitch >> 2;
      }
     }
    }

    if(cur_flags & SDL_OPENGL)
     BlitOpenGL(bah_surface, &boohoo_rect, &dest_rect, &eff_src_rect, alpha_blend);
    else
     BlitNonGL(bah_surface, &boohoo_rect, &eff_src_rect, &dest_rect, screen, _video.scanlines, CurGame->rotated);

    SDL_FreeSurface(bah_surface);
   }
   else // No special scaler:
   {
    if(cur_flags & SDL_OPENGL)
     BlitOpenGL(eff_source_surface, &eff_src_rect, &dest_rect, &eff_src_rect, alpha_blend);
    else
     BlitNonGL(eff_source_surface, &eff_src_rect, &eff_src_rect, &dest_rect, screen, _video.scanlines, CurGame->rotated);
   }

 if(tmp_blur_surface)
 {
  SDL_FreeSurface(tmp_blur_surface);
 } 
}

void BlitScreen(uint32 *XBuf, MDFN_Rect *DisplayRect, MDFN_Rect *LineWidths)
{
 if(!screen) return;

 if(NeedClear)
 {
  NeedClear--;
  ClearBackBuffer();
 }

 GenerateRects(*DisplayRect);
 source_surface->pixels = XBuf;

 if(LineWidths[0].w == ~0) // Skip multi line widths code?
 {
  SubBlit(0);
 }
 else
 {
  int y;
  int last_y = src_rect.y;
  int last_x = LineWidths[src_rect.y].x;
  int last_width = LineWidths[src_rect.y].w;
  SDL_Rect src_rect_save;
  SDL_Rect dest_rect_save;

  for(y = src_rect.y; y < (src_rect.y + src_rect.h + 1); y++)
  {
   if(y == (src_rect.y + src_rect.h) || LineWidths[y].x != last_x || LineWidths[y].w != last_width)
   {
    src_rect_save = src_rect;
    dest_rect_save = dest_rect;
    src_rect.x = last_x;
    src_rect.w = last_width;
    src_rect.y = last_y;
    src_rect.h = y - last_y;

    dest_rect.y = dest_rect_save.y + (last_y - src_rect_save.y) * dest_rect.h / src_rect_save.h;
    dest_rect.h = src_rect.h * dest_rect.h / src_rect_save.h;

    // Blit here!
    SubBlit(0);

    src_rect = src_rect_save;
    dest_rect = dest_rect_save;
    last_y = y;
    last_width = LineWidths[y].w;
    last_x =  LineWidths[y].x;
   }
  }
 }

 unsigned int debw, debh;

 if(Debugger_IsActive(&debw, &debh))
 {
  if(!DebuggerSurface)
  {
   DebuggerSurface = SDL_CreateRGBSurface(SDL_SWSURFACE | SDL_SRCALPHA, 640, 480, 32, 0xFF << real_rs, 0xFF << real_gs, 0xFF << real_bs, 0xFF << real_as);
  }
  DebuggerRect.w = debw;
  DebuggerRect.h = debh;
  DebuggerRect.x = 0;
  DebuggerRect.y = 0;

  SDL_Rect zederect;

  int xm = screen->w / DebuggerRect.w;
  int ym = screen->h / DebuggerRect.h;

  if(xm < 1) xm = 1;
  if(ym < 1) ym = 1;

  //if(xm > ym) xm = ym;
  //if(ym > xm) ym = xm;

  // Allow it to be compacted horizontally, but don't stretch it out, as it's hard(IMHO) to read.
  if(xm > ym) xm = ym;
  if(ym > (2 * xm)) ym = 2 * xm;

  zederect.w = DebuggerRect.w * xm;
  zederect.h = DebuggerRect.h * ym;

  zederect.x = (screen->w - zederect.w) / 2;
  zederect.y = (screen->h - zederect.h) / 2;

  Debugger_Draw(DebuggerSurface, &DebuggerRect, &zederect);

  BlitRaw(DebuggerSurface, &DebuggerRect, &zederect);
 }

 if(Help_IsActive())
 {
  if(!HelpSurface)
  {
   HelpRect.w = screen->w;
   HelpRect.h = screen->h;

   if(HelpRect.w >= 512)
   {
    HelpRect.w /= (HelpRect.w / 512);
    if(HelpRect.w > 512)
     HelpRect.w = 512;
   }

   if(HelpRect.h >= 384)
   {
    HelpRect.h /= (HelpRect.h / 384);
    if(HelpRect.h > 384)
     HelpRect.h = 384;
   }

   HelpSurface = SDL_CreateRGBSurface(SDL_SWSURFACE | SDL_SRCALPHA, HelpRect.w, HelpRect.h, 32, 0xFF << real_rs, 0xFF << real_gs, 0xFF << real_bs, 0xFF << real_as);
   SDL_SetColorKey(HelpSurface, SDL_SRCCOLORKEY, 0);
   SDL_SetAlpha(HelpSurface, SDL_SRCALPHA, 0);
   Help_Draw(HelpSurface, &HelpRect);
  }
  SDL_Rect zederect;

  zederect.w = HelpRect.w * (screen->w / HelpRect.w);
  zederect.h = HelpRect.h * (screen->h / HelpRect.h);

  zederect.x = (screen->w - zederect.w) / 2;
  zederect.y = (screen->h - zederect.h) / 2;
  BlitRaw(HelpSurface, &HelpRect, &zederect);
 }
 else if(HelpSurface)
 {
  SDL_FreeSurface(HelpSurface);
  HelpSurface = NULL;
 }

 DrawSaveStates(screen, exs, eys, real_rs, real_gs, real_bs, real_as);

 if(IsConsoleCheatConfigActive())
 {
  if(!CheatSurface)
  {
   CheatRect.w = screen->w;
   CheatRect.h = screen->h;

   CheatSurface = SDL_CreateRGBSurface(SDL_SWSURFACE | SDL_SRCALPHA, CheatRect.w, CheatRect.h, 32, 0xFF << real_rs, 0xFF << real_gs, 0xFF << real_bs, 0xFF << real_as);
   SDL_SetColorKey(CheatSurface, SDL_SRCCOLORKEY, 0);
   SDL_SetAlpha(CheatSurface, SDL_SRCALPHA, 0);
  }
  SDL_Rect zederect = CheatRect;
  DrawCheatConsole(CheatSurface, &CheatRect);
  BlitRaw(CheatSurface, &CheatRect, &zederect);
 }
 else if(CheatSurface)
 {
  SDL_FreeSurface(CheatSurface);
  CheatSurface = NULL;
 }

 if(Netplay_GetTextView())
 {
  if(SDL_MUSTLOCK(NetSurface))
   SDL_LockSurface(NetSurface);

  DrawNetplayTextBuffer(NetSurface, &NetRect);

  {
   SDL_Rect zederect;

   zederect.x = 0;
   zederect.y = screen->h - NetRect.h;
   zederect.w = NetRect.w;
   zederect.h = NetRect.h;

   BlitRaw(NetSurface, &NetRect, &zederect);
  }
  if(SDL_MUSTLOCK(NetSurface))
   SDL_UnlockSurface(NetSurface);
 }

 BlitInternalMessage();

 FPS_Draw(screen, real_rs, real_gs, real_bs, real_as);

 if(!(cur_flags & SDL_OPENGL))
  SDL_Flip(screen);
 else
  FlipOpenGL();
}

void PtoV(int *x, int *y)
{
 *y=(int)((double)*y/eys);
 *x=(int)((double)*x/exs);
}

