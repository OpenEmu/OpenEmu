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
#include "opengl.h"
#include "shader.h"

glBindTexture_Func p_glBindTexture;
glColorTableEXT_Func p_glColorTableEXT;
glTexImage2D_Func p_glTexImage2D;
glBegin_Func p_glBegin;
glVertex2f_Func p_glVertex2f;
glTexCoord2f_Func p_glTexCoord2f;
glEnd_Func p_glEnd;
glEnable_Func p_glEnable;
glBlendFunc_Func p_glBlendFunc;
glGetString_Func p_glGetString;
glViewport_Func p_glViewport;
glGenTextures_Func p_glGenTextures;
glDeleteTextures_Func p_glDeleteTextures;
glTexParameteri_Func p_glTexParameteri;
glClearColor_Func p_glClearColor;
glLoadIdentity_Func p_glLoadIdentity;
glClear_Func p_glClear;
glMatrixMode_Func p_glMatrixMode;
glDisable_Func p_glDisable;
glPixelStorei_Func p_glPixelStorei;
glTexSubImage2D_Func p_glTexSubImage2D;
glFinish_Func p_glFinish;
glOrtho_Func p_glOrtho;
glPixelTransferf_Func p_glPixelTransferf;
glColorMask_Func p_glColorMask;
glTexEnvf_Func p_glTexEnvf;
glGetIntegerv_Func p_glGetIntegerv;
glTexGend_Func p_glTexGend;
glDrawPixels_Func p_glDrawPixels;
glRasterPos2i_Func p_glRasterPos2i;
glPixelZoom_Func p_glPixelZoom;
glGetTexLevelParameteriv_Func p_glGetTexLevelParameteriv;
glAccum_Func p_glAccum;
glClearAccum_Func p_glClearAccum;

#if MDFN_WANT_OPENGL_SHADERS
glCreateShaderObjectARB_Func p_glCreateShaderObjectARB;
glShaderSourceARB_Func p_glShaderSourceARB;
glCompileShaderARB_Func p_glCompileShaderARB;
glCreateProgramObjectARB_Func p_glCreateProgramObjectARB;
glAttachObjectARB_Func p_glAttachObjectARB;
glLinkProgramARB_Func p_glLinkProgramARB;
glUseProgramObjectARB_Func p_glUseProgramObjectARB;
glUniform1fARB_Func p_glUniform1fARB;
glUniform2fARB_Func p_glUniform2fARB;
glUniform3fARB_Func p_glUniform3fARB;
glUniform1iARB_Func p_glUniform1iARB;
glUniform2iARB_Func p_glUniform2iARB;
glUniform3iARB_Func p_glUniform3iARB;
glActiveTextureARB_Func p_glActiveTextureARB;
glGetInfoLogARB_Func p_glGetInfoLogARB;
glGetUniformLocationARB_Func p_glGetUniformLocationARB;
glDeleteObjectARB_Func p_glDeleteObjectARB;
glDetachObjectARB_Func p_glDetachObjectARB;
#endif

static uint32 MaxTextureSize; // Maximum power-of-2 texture width/height(we assume they're the same, and if they're not, this is set to the lower value of the two)
static bool SupportNPOT; // True if the OpenGL implementation supports non-power-of-2-sized textures

static SDL_Surface *gl_screen = NULL;
static GLuint textures[4] = {0, 0, 0, 0}; // emulated fb, scanlines, osd, raw(netplay)
static GLuint rgb_mask = 0; // TODO:  RGB mask texture for LCD RGB triad simulation

static bool using_scanlines = 0;
static unsigned int last_w, last_h;
static float twitchy;

extern uint32 uppow2(uint32);
static uint32 OSDLastWidth, OSDLastHeight;

static bool UsingShader = FALSE; // TRUE if we're using a pixel shader.
static bool UsingIP;	// True if bilinear interpolation is enabled.

static uint32 *DummyBlack; // Black/Zeroed image data for cleaning textures
static uint32 DummyBlackSize;

void BlitOpenGLRaw(SDL_Surface *surface, const SDL_Rect *rect, const SDL_Rect *dest_rect)
{
 unsigned int tmpwidth;
 unsigned int tmpheight;

 if(SupportNPOT)
 {
  tmpwidth = rect->w;
  tmpheight = rect->h;
 }
 else
 {
  tmpwidth = uppow2(rect->w);
  tmpheight = uppow2(rect->h);
 }

 if(tmpwidth > MaxTextureSize || tmpheight > MaxTextureSize)
 {
  SDL_Rect neo_rect;
  SDL_Rect neo_dest_rect;

  for(uint32 xseg = 0; xseg < rect->w; xseg += MaxTextureSize)
  {
   for(uint32 yseg = 0; yseg < rect->h; yseg += MaxTextureSize)
   {
    neo_rect.x = rect->x + xseg;
    neo_rect.w = rect->w - xseg;
    if(neo_rect.w > MaxTextureSize)
     neo_rect.w = MaxTextureSize;
    neo_rect.y = rect->y + yseg;
    neo_rect.h = rect->h - yseg;
    if(neo_rect.h > MaxTextureSize)
     neo_rect.h = MaxTextureSize;


    neo_dest_rect.x = dest_rect->x + xseg * dest_rect->w / rect->w;
    neo_dest_rect.y = dest_rect->y + yseg * dest_rect->h / rect->h;
    neo_dest_rect.w = neo_rect.w * dest_rect->w / rect->w;
    neo_dest_rect.h = neo_rect.h * dest_rect->h / rect->h;
    BlitOpenGLRaw(surface, &neo_rect, &neo_dest_rect);
   }
  }

 }
 else
 {
  p_glEnable(GL_BLEND);
  p_glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

  p_glBindTexture(GL_TEXTURE_2D, textures[3]);
  p_glPixelStorei(GL_UNPACK_ROW_LENGTH, surface->pitch >> 2);

  p_glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, tmpwidth, tmpheight, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
  p_glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, rect->w, rect->h, GL_RGBA, GL_UNSIGNED_BYTE, (uint32 *)surface->pixels + rect->x + rect->y * (surface->pitch >> 2));

  p_glBegin(GL_QUADS);

  p_glTexCoord2f(0.0f, 1.0f * rect->h / tmpheight);  // Bottom left of our picture.
  p_glVertex2f(dest_rect->x, dest_rect->y + dest_rect->h);

  p_glTexCoord2f((float)rect->w / tmpwidth, 1.0f * rect->h / tmpheight); // Bottom right of our picture.
  p_glVertex2f(dest_rect->x + dest_rect->w, dest_rect->y + dest_rect->h);

  p_glTexCoord2f((float)rect->w / tmpwidth, 0.0f);    // Top right of our picture.
  p_glVertex2f(dest_rect->x + dest_rect->w,  dest_rect->y);

  p_glTexCoord2f(0.0f, 0.0f);     // Top left of our picture.
  p_glVertex2f(dest_rect->x, dest_rect->y);

  p_glEnd();

  p_glDisable(GL_BLEND);
 }
}

void BlitOpenGL(SDL_Surface *src_surface, const SDL_Rect *src_rect, const SDL_Rect *dest_rect, const SDL_Rect *original_src_rect, bool alpha_blend)
{
 int left = src_rect->x;
 int right = src_rect->x + src_rect->w;	// First nonincluded pixel
 int top = src_rect->y;
 int bottom = src_rect->y + src_rect->h; // ditto

 int sl_bottom = /*original_src_rect->y + */ original_src_rect->h; // ditto

 p_glBindTexture(GL_TEXTURE_2D, textures[0]);

 unsigned int tmpwidth;
 unsigned int tmpheight;
 uint32 *src_pixies = (uint32 *)src_surface->pixels;

 src_pixies += left + top * (src_surface->pitch >> 2);
 right -= left;
 left = 0;
 bottom -= top;
 top = 0;

 if(SupportNPOT)
 {
  tmpwidth = src_rect->w;
  tmpheight = src_rect->h;

  if(tmpwidth != last_w || tmpheight != last_h)
  {
   p_glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, tmpwidth, tmpheight, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
   last_w = tmpwidth;
   last_h = tmpheight;
  }
 }
 else
 {
  bool ImageSizeChange = FALSE;

  tmpwidth = uppow2(src_rect->w);
  tmpheight = uppow2(src_rect->h);

  // If the required GL texture size has changed, resize the texture! :b
  if(tmpwidth != uppow2(last_w) || tmpheight != uppow2(last_h))
  {
   p_glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, tmpwidth, tmpheight, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
   ImageSizeChange = TRUE;
  }
 
  // If the dimensions of our image stored in the texture have changed...
  if(src_rect->w != last_w || src_rect->h != last_h)
   ImageSizeChange = TRUE;

  // Only clean up if we're using pixel shaders and/or bilinear interpolation
  if(ImageSizeChange && (UsingShader || UsingIP))
  {
   uint32 neo_dbs = DummyBlackSize;

   if(src_rect->w != tmpwidth && neo_dbs < src_rect->h)
    neo_dbs = src_rect->h;

   if(src_rect->h != tmpheight && neo_dbs < src_rect->w)
    neo_dbs = src_rect->w;

   if(neo_dbs != DummyBlackSize)
   {
    //printf("Realloc: %d\n", neo_dbs);
    if(DummyBlack)
     MDFN_free(DummyBlack);

    if((DummyBlack = (uint32 *)MDFN_calloc(neo_dbs, sizeof(uint32), _("OpenGL dummy black texture data"))))
     DummyBlackSize = neo_dbs;
    else
     DummyBlackSize = 0;
   }

   //printf("Cleanup: %d %d, %d %d\n", src_rect->w, src_rect->h, tmpwidth, tmpheight);

   if(DummyBlack) // If memory allocation failed for some reason, don't clean the texture. :(
   {
    if(src_rect->w < tmpwidth)
    {
     //puts("X");
     p_glPixelStorei(GL_UNPACK_ROW_LENGTH, 1);
     p_glTexSubImage2D(GL_TEXTURE_2D, 0, src_rect->w, 0, 1, src_rect->h, GL_RGBA, GL_UNSIGNED_BYTE, DummyBlack);
    }
    if(src_rect->h < tmpheight)
    {
     //puts("Y");
     p_glPixelStorei(GL_UNPACK_ROW_LENGTH, src_rect->w);
     p_glTexSubImage2D(GL_TEXTURE_2D, 0, 0, src_rect->h, src_rect->w, 1, GL_RGBA, GL_UNSIGNED_BYTE, DummyBlack);
    }
   } // end if(DummyBlack)

  }

  last_w = src_rect->w;
  last_h = src_rect->h;
 }

 #if MDFN_WANT_OPENGL_SHADERS
 if(UsingShader)
  ShaderBegin(src_surface, src_rect, tmpwidth, tmpheight);
 #endif

 if(alpha_blend)
 {
  p_glEnable(GL_BLEND);
  p_glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
 }

 p_glPixelStorei(GL_UNPACK_ROW_LENGTH, src_surface->pitch >> 2);
 p_glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, right, bottom, GL_RGBA, GL_UNSIGNED_BYTE, src_pixies);

 bool DoneAgain = 0;

 DoAgain:
 p_glBegin(GL_QUADS);
 if(CurGame->rotated != MDFN_ROTATE0)
 {
  if(CurGame->rotated == MDFN_ROTATE90)
  {
   p_glTexCoord2f(0, 0);
    p_glVertex2f(dest_rect->x, dest_rect->y + dest_rect->h);

   p_glTexCoord2f(0, (float)src_rect->h / tmpheight);
    p_glVertex2f(dest_rect->x + dest_rect->w, dest_rect->y + dest_rect->h);

   p_glTexCoord2f(1.0f * right / tmpwidth, (float)src_rect->h / tmpheight);
    p_glVertex2f(dest_rect->x + dest_rect->w,  dest_rect->y);

   p_glTexCoord2f(1.0f * right / tmpwidth, 0);
    p_glVertex2f(dest_rect->x,  dest_rect->y);
  }
  else
  {
   p_glTexCoord2f(1.0f * right / tmpwidth, (float)src_rect->h / tmpheight);
    p_glVertex2f(dest_rect->x, dest_rect->y + dest_rect->h);
   p_glTexCoord2f(1.0f * right / tmpwidth, 0);
    p_glVertex2f(dest_rect->x + dest_rect->w, dest_rect->y + dest_rect->h);
   p_glTexCoord2f(0, 0);
    p_glVertex2f(dest_rect->x + dest_rect->w,  dest_rect->y);
   p_glTexCoord2f(0, (float)src_rect->h / tmpheight);
    p_glVertex2f(dest_rect->x,  dest_rect->y);
  }
 }
 else
 {
  p_glTexCoord2f(0, (1.0f * src_rect->h) / tmpheight);
   p_glVertex2f(dest_rect->x, twitchy + dest_rect->y + dest_rect->h);
   p_glTexCoord2f(1.0f * right / tmpwidth, (1.0f * src_rect->h) / tmpheight);
  p_glVertex2f(dest_rect->x + dest_rect->w, twitchy + dest_rect->y + dest_rect->h);
  p_glTexCoord2f(1.0f * right / tmpwidth, 0);
   p_glVertex2f(dest_rect->x + dest_rect->w, twitchy + dest_rect->y);

  p_glTexCoord2f(0, 0);
   p_glVertex2f(dest_rect->x, twitchy + dest_rect->y);
 }

 p_glEnd();

 if(alpha_blend)
 {
  if(!DoneAgain)
  {
   DoneAgain = TRUE;
   p_glBlendFunc(GL_ONE_MINUS_SRC_ALPHA, GL_ONE);

   goto DoAgain;
  }
  else
  {
   p_glDisable(GL_BLEND);
  }
 }

 #if MDFN_WANT_OPENGL_SHADERS
 if(UsingShader)
  ShaderEnd();
 #endif

 if(using_scanlines)
 {
  p_glEnable(GL_BLEND);

  p_glBindTexture(GL_TEXTURE_2D, textures[1]);
  p_glBlendFunc(GL_DST_COLOR, GL_SRC_ALPHA);

  p_glBegin(GL_QUADS);

  p_glTexCoord2f(0.0f, 1.0f * sl_bottom/256);  // Bottom left of our picture.
  p_glVertex2f(dest_rect->x, dest_rect->y + dest_rect->h);

  p_glTexCoord2f(1.0f, 1.0f * sl_bottom/256); // Bottom right of our picture.
  p_glVertex2f(dest_rect->x + dest_rect->w, dest_rect->y + dest_rect->h);

  p_glTexCoord2f(1.0f, 0.0f);    // Top right of our picture.
  p_glVertex2f(dest_rect->x + dest_rect->w,  dest_rect->y);

  p_glTexCoord2f(0.0f, 0.0f);     // Top left of our picture.
  p_glVertex2f(dest_rect->x,  dest_rect->y);

  p_glEnd();
  p_glDisable(GL_BLEND);
 }

 //if(1)
 //{
 // p_glAccum(GL_MULT, 0.99);
 // p_glAccum(GL_ACCUM, 1 - 0.99);
 // p_glAccum(GL_RETURN, 1.0);
 //}
}

void FlipOpenGL(void)
{
 PumpWrap();
 SDL_GL_SwapBuffers();
}

void KillOpenGL(void)
{
 if(textures[0])
  p_glDeleteTextures(4, &textures[0]);

 textures[0] = textures[1] = textures[2] = textures[3] = 0;

 if(rgb_mask)
 {
  p_glDeleteTextures(1, &rgb_mask);
  rgb_mask = 0;
 }

 if(DummyBlack)
 {
  MDFN_free(DummyBlack);
  DummyBlack = NULL;
 }
 DummyBlackSize = 0;

 #if MDFN_WANT_OPENGL_SHADERS
 if(UsingShader)
 {
  KillShader();
  UsingShader = FALSE;
 }
 #endif
}

static bool CheckExtension(const char *extensions, const char *testval)
{
 const char *extparse = extensions;
 const size_t testval_len = strlen(testval);

 while((extparse = strstr(extparse, testval)))
 {
  if(extparse == extensions || *(extparse - 1) == ' ')
  {
   if(extparse[testval_len] == ' ' || extparse[testval_len] == 0)
   {
    return(TRUE);
   }
  }
  extparse += testval_len;
 }
 return(FALSE);
}

/* Rectangle, left, right(not inclusive), top, bottom(not inclusive). */

int InitOpenGL(int ipolate, int scanlines, std::string pixshader, SDL_Surface *screen)
{
 const char *extensions;

 #define LFG(x) if(!(p_##x = (x##_Func) SDL_GL_GetProcAddress(#x))) { MDFN_PrintError(_("Error getting proc address for: %s\n"), #x); return(0); }
 #define LFGN(x) p_##x = (x##_Func) SDL_GL_GetProcAddress(#x)

 LFG(glBindTexture);
 LFGN(glColorTableEXT);
 LFG(glTexImage2D);
 LFG(glBegin);
 LFG(glVertex2f);
 LFG(glTexCoord2f);
 LFG(glEnd);
 LFG(glEnable);
 LFG(glBlendFunc);
 LFG(glGetString);
 LFG(glViewport);
 LFG(glGenTextures);
 LFG(glDeleteTextures);
 LFG(glTexParameteri);
 LFG(glClearColor);
 LFG(glLoadIdentity);
 LFG(glClear);
 LFG(glMatrixMode);
 LFG(glDisable);
 LFG(glPixelStorei);
 LFG(glTexSubImage2D);
 LFG(glFinish);
 LFG(glOrtho);
 LFG(glPixelTransferf);
 LFG(glColorMask);
 LFG(glTexEnvf);
 LFG(glGetIntegerv);
 LFG(glTexGend);
 LFG(glRasterPos2i);
 LFG(glDrawPixels);
 LFG(glPixelZoom);
 LFG(glAccum);
 LFG(glClearAccum);
 LFG(glGetTexLevelParameteriv);

 gl_screen = screen;

 MDFN_printf(_("OpenGL Implementation: %s %s %s\n"), p_glGetString(GL_VENDOR), p_glGetString(GL_RENDERER), p_glGetString(GL_VERSION));
 extensions=(const char*)p_glGetString(GL_EXTENSIONS);

 MDFN_printf(_("Checking extensions:\n"));
 MDFN_indent(1);

 SupportNPOT = FALSE;

 if(CheckExtension(extensions, "GL_ARB_texture_non_power_of_two"))
 {
  MDFN_printf(_("GL_ARB_texture_non_power_of_two found.\n"));
  SupportNPOT = TRUE;
 }
 MDFN_indent(-1);

 p_glViewport(0, 0, screen->w, screen->h);

 p_glGenTextures(4, &textures[0]);
 p_glGenTextures(1, &rgb_mask);
 using_scanlines = 0;

 UsingShader = FALSE;

 #if MDFN_WANT_OPENGL_SHADERS
 if(strcasecmp(pixshader.c_str(), "none"))
 {
  ShaderType pixshadertype;

  if(!strcasecmp(pixshader.c_str(), "hqxx"))
   pixshadertype = SHADER_HQXX;
  else if(!strcasecmp(pixshader.c_str(), "scale2x"))
   pixshadertype = SHADER_SCALE2X;
  else if(!strcasecmp(pixshader.c_str(), "ipsharper"))
   pixshadertype = SHADER_IPSHARPER;
  else if(!strcasecmp(pixshader.c_str(), "ipxnoty"))
   pixshadertype = SHADER_IPXNOTY;
  else if(!strcasecmp(pixshader.c_str(), "ipxnotysharper"))
   pixshadertype = SHADER_IPXNOTYSHARPER;
  else if(!strcasecmp(pixshader.c_str(), "ipynotx"))
   pixshadertype = SHADER_IPYNOTX;
  else if(!strcasecmp(pixshader.c_str(), "ipynotxsharper"))
   pixshadertype = SHADER_IPYNOTXSHARPER;
  else
  {
   MDFN_PrintError(_("Invalid pixel shader type: %s\n"), pixshader.c_str());
   return(0);
  }
  LFG(glCreateShaderObjectARB);
  LFG(glShaderSourceARB);
  LFG(glCompileShaderARB);
  LFG(glCreateProgramObjectARB);
  LFG(glAttachObjectARB);
  LFG(glLinkProgramARB);
  LFG(glUseProgramObjectARB);
  LFG(glUniform1fARB);
  LFG(glUniform2fARB);
  LFG(glUniform3fARB);
  LFG(glUniform1iARB);
  LFG(glUniform2iARB);
  LFG(glUniform3iARB);
  LFG(glActiveTextureARB);
  LFG(glGetInfoLogARB);
  LFG(glGetUniformLocationARB);
  LFG(glDeleteObjectARB);
  LFG(glDetachObjectARB);

  if(!InitShader(pixshadertype))
  {
   return(0);
  }
  UsingShader = TRUE;
  ipolate = FALSE; // Disable texture interpolation, otherwise our pixel shaders will be extremely blurry.  
  SupportNPOT = 0; // Our pixel shaders don't work right with NPOT textures:  FIXME
  p_glActiveTextureARB(GL_TEXTURE0_ARB);
 }
 #endif

 // printf here because pixel shader code will set SupportNPOT to 0

 if(SupportNPOT)
  MDFN_printf(_("Using non-power-of-2 sized textures.\n"));
 else
  MDFN_printf(_("Using power-of-2 sized textures.\n"));

 if(scanlines)	// Check for scanlines, and disable them if vertical scaling isn't large enough.
 {
  int slcount;
  uint8 *buf;
  int x,y;

  using_scanlines = scanlines;

  p_glBindTexture(GL_TEXTURE_2D, textures[1]);
  p_glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_NEAREST);
  p_glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_NEAREST);

  buf=(uint8*)malloc(64 * (256 * 2) * 4);

  slcount = 0;
  for(y=0;y<(256 * 2);y++)
  {
   for(x=0;x<64;x++)
   {
    buf[y*64*4+x*4]=0;
    buf[y*64*4+x*4+1]=0;
    buf[y*64*4+x*4+2]=0;
    buf[y*64*4+x*4+3]=slcount?0xFF:0xFF - (0xFF / (scanlines));
    //buf[y*256+x]=(y&1)?0x00:0xFF;
   }
   slcount ^= 1;
  }

  p_glPixelStorei(GL_UNPACK_ROW_LENGTH, 64);
  p_glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, 64, 256 * 2, 0, GL_RGBA,GL_UNSIGNED_BYTE,buf);
  free(buf);
 }
 p_glBindTexture(GL_TEXTURE_2D, textures[3]);
 p_glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_NEAREST);
 p_glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_NEAREST);
 p_glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_S,GL_CLAMP);
 p_glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_T,GL_CLAMP);


 p_glBindTexture(GL_TEXTURE_2D, textures[0]);
     
 UsingIP = ipolate;

 p_glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,ipolate?GL_LINEAR:GL_NEAREST);
 p_glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,ipolate?GL_LINEAR:GL_NEAREST);
 p_glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_S,GL_CLAMP);
 p_glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_T,GL_CLAMP);

 p_glBindTexture(GL_TEXTURE_2D, textures[2]);

 p_glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_S,GL_CLAMP);
 p_glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_T,GL_CLAMP);

 p_glEnable(GL_TEXTURE_2D);
 p_glClearColor(0.0f, 0.0f, 0.0f, 0.0f);	// Background color to black.
 p_glMatrixMode(GL_MODELVIEW);

 p_glLoadIdentity();
 p_glFinish();

 p_glDisable(GL_TEXTURE_1D);
 p_glDisable(GL_FOG);
 p_glDisable(GL_LIGHTING);
 p_glDisable(GL_LOGIC_OP);
 p_glDisable(GL_DITHER);
 p_glDisable(GL_COLOR_MATERIAL);
 p_glDisable(GL_NORMALIZE);
 p_glDisable(GL_SCISSOR_TEST);
 p_glDisable(GL_STENCIL_TEST);
 p_glDisable(GL_ALPHA_TEST);
 p_glDisable(GL_DEPTH_TEST);

 p_glPixelTransferf(GL_RED_BIAS, 0);
 p_glPixelTransferf(GL_GREEN_BIAS, 0);
 p_glPixelTransferf(GL_BLUE_BIAS, 0);
 p_glPixelTransferf(GL_ALPHA_BIAS, 0);

 p_glPixelTransferf(GL_RED_SCALE, 1);
 p_glPixelTransferf(GL_GREEN_SCALE, 1);
 p_glPixelTransferf(GL_BLUE_SCALE, 1);
 p_glPixelTransferf(GL_ALPHA_SCALE, 1);

 p_glPixelTransferf(GL_MAP_COLOR, GL_FALSE);

 p_glOrtho(0.0, screen->w, screen->h, 0, -1.0, 1.0);

 if(scanlines && ipolate)
  twitchy = 0.5f;
 else
  twitchy = 0;

 last_w = 0;
 last_h = 0;

 OSDLastWidth = OSDLastHeight = 0;


 MDFN_printf(_("Checking maximum texture size...\n"));
 MDFN_indent(1);
 p_glBindTexture(GL_TEXTURE_2D, textures[0]);
 // Assume maximum texture width is the same as maximum texture height to greatly simplify things
 MaxTextureSize = 32768;
 
 while(MaxTextureSize)
 {
  GLint width_test = 0;

  p_glTexImage2D(GL_PROXY_TEXTURE_2D, 0, GL_RGBA, MaxTextureSize, MaxTextureSize, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
  p_glGetTexLevelParameteriv(GL_PROXY_TEXTURE_2D, 0, GL_TEXTURE_WIDTH, &width_test);

  if((unsigned int)width_test == MaxTextureSize)
   break;

  MaxTextureSize >>= 1;
 }
 MDFN_printf(_("Apparently it is at least: %d x %d\n"), MaxTextureSize, MaxTextureSize);

 if(MaxTextureSize < 256)
 {
  MDFN_printf(_("Warning:  Maximum texture size is reported as being less than 256, but we can't handle that.\n"));
  MaxTextureSize = 256;
 }

 DummyBlack = NULL;
 DummyBlackSize = 0;

 MDFN_indent(-1);

 return(1);
}

void ClearBackBufferOpenGL(SDL_Surface *screen)
{
 //if(1)
 //{
 // p_glClearAccum(0.0, 0.0, 0.0, 1.0);
 // p_glClear(GL_COLOR_BUFFER_BIT | GL_ACCUM_BUFFER_BIT);
 //}
 //else
 //{
  p_glClear(GL_COLOR_BUFFER_BIT);
 //}
}

