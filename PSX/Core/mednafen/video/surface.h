#ifndef __MDFN_SURFACE_H
#define __MDFN_SURFACE_H

typedef struct
{
 int32 x, y, w, h;
} MDFN_Rect;

enum
{
 MDFN_COLORSPACE_RGB = 0,
 MDFN_COLORSPACE_YCbCr = 1,
 //MDFN_COLORSPACE_YUV = 2, // TODO, maybe.
};

class MDFN_PixelFormat
{
 public:

 MDFN_PixelFormat();
 MDFN_PixelFormat(const unsigned int p_colorspace, const uint8 p_rs, const uint8 p_gs, const uint8 p_bs, const uint8 p_as);

 unsigned int bpp;	// 32 only for now(16 wip)
 unsigned int colorspace;

 union
 {
  uint8 Rshift;  // Bit position of the lowest bit of the red component
  uint8 Yshift;
 };

 union
 {
  uint8 Gshift;  // [...] green component
  uint8 Ushift;
  uint8 Cbshift;
 };

 union
 {
  uint8 Bshift;  // [...] blue component
  uint8 Vshift;
  uint8 Crshift;
 };

 uint8 Ashift;  // [...] alpha component.

 // For 16bpp, WIP
 uint8 Rprec;
 uint8 Gprec;
 uint8 Bprec;
 uint8 Aprec;

 // Creates a color value for the surface corresponding to the 8-bit R/G/B/A color passed.
 INLINE uint32 MakeColor(uint8 r, uint8 g, uint8 b, uint8 a = 0) const
 {
  if(colorspace == MDFN_COLORSPACE_YCbCr)
  {
   uint32 y, u, v;

   y = 16 + ((r * 16842 + g * 33030 + b * 6422) >> 16);
   u = 128 + ((r * -9699 + g * -19071 + b * 28770) >> 16);
   v = 128 + ((r * 28770 + g * -24117 + b * -4653) >> 16);

   return((y << Yshift) | (u << Ushift) | (v << Vshift) | (a << Ashift));
  }
  else
  {
   if(bpp == 16)
   {
    uint32 ret = 0;
/*
    ret |= std::min(((r * ((1 << Rprec) - 1) + 127) / 255), 255) << Rshift;
    ret |= std::min(((g * ((1 << Gprec) - 1) + 127) / 255), 255) << Gshift;
    ret |= std::min(((b * ((1 << Bprec) - 1) + 127) / 255), 255) << Bshift;
    ret |= std::min(((a * ((1 << Aprec) - 1) + 127) / 255), 255) << Ashift;
*/
    ret |= ((r * ((1 << Rprec) - 1) + 127) / 255) << Rshift;
    ret |= ((g * ((1 << Gprec) - 1) + 127) / 255) << Gshift;
    ret |= ((b * ((1 << Bprec) - 1) + 127) / 255) << Bshift;
    ret |= ((a * ((1 << Aprec) - 1) + 127) / 255) << Ashift;
    return(ret);
   }
   else
    return((r << Rshift) | (g << Gshift) | (b << Bshift) | (a << Ashift));
  }
 }

 // Gets the R/G/B/A values for the passed 32-bit surface pixel value
 INLINE void DecodeColor(uint32 value, int &r, int &g, int &b, int &a) const
 {
  if(colorspace == MDFN_COLORSPACE_YCbCr)
  {
   int32 y = (value >> Yshift) & 0xFF;
   int32 cb = (value >> Cbshift) & 0xFF;
   int32 cr = (value >> Crshift) & 0xFF;

   int32 r_tmp, g_tmp, b_tmp;

   r_tmp = g_tmp = b_tmp = 76284 * (y - 16);
   
   r_tmp = r_tmp + 104595 * (cr - 128);
   g_tmp = g_tmp - 53281 * (cr - 128) - 25690 * (cb - 128);
   b_tmp = b_tmp + 132186 * (cb - 128);

   r_tmp >>= 16;
   g_tmp >>= 16;
   b_tmp >>= 16;

   if(r_tmp < 0) r_tmp = 0;
   if(r_tmp > 255) r_tmp = 255;

   if(g_tmp < 0) g_tmp = 0;
   if(g_tmp > 255) g_tmp = 255;

   if(b_tmp < 0) b_tmp = 0;
   if(b_tmp > 255) b_tmp = 255;

   r = r_tmp;
   g = g_tmp;
   b = b_tmp;

   a = (value >> Ashift) & 0xFF;
  }
  else
  {
   if(bpp == 16)
   {
    r = ((value >> Rshift) & ((1 << Rprec) - 1)) * 255 / ((1 << Rprec) - 1);
    g = ((value >> Gshift) & ((1 << Gprec) - 1)) * 255 / ((1 << Gprec) - 1);
    b = ((value >> Bshift) & ((1 << Bprec) - 1)) * 255 / ((1 << Bprec) - 1);
    a = ((value >> Ashift) & ((1 << Aprec) - 1)) * 255 / ((1 << Aprec) - 1);
   }
   else
   {
    r = (value >> Rshift) & 0xFF;
    g = (value >> Gshift) & 0xFF;
    b = (value >> Bshift) & 0xFF;
    a = (value >> Ashift) & 0xFF;
   }
  }
 }

}; // MDFN_PixelFormat;

struct MDFN_PaletteEntry
{
 uint8 r, g, b;
};

#include <vector>
typedef std::vector<MDFN_PaletteEntry> MDFN_Palette;

// Supports 32-bit RGBA
//  16-bit is WIP
class MDFN_Surface //typedef struct
{
 public:

 MDFN_Surface();
 MDFN_Surface(void *const p_pixels, const uint32 p_width, const uint32 p_height, const uint32 p_pitchinpix, const MDFN_PixelFormat &nf);

 ~MDFN_Surface();

 uint16 *pixels16;
 uint32 *pixels;

 bool pixels_is_external;

 // w, h, and pitch32 should always be > 0
 int32 w;
 int32 h;

 union
 {
  int32 pitch32; // In pixels, not in bytes.
  int32 pitchinpix;	// New name, new code should use this.
 };

 MDFN_PixelFormat format;

 void Fill(uint8 r, uint8 g, uint8 b, uint8 a);
 void SetFormat(const MDFN_PixelFormat &new_format, bool convert);

 // Creates a 32-bit value for the surface corresponding to the R/G/B/A color passed.
 INLINE uint32 MakeColor(uint8 r, uint8 g, uint8 b, uint8 a = 0) const
 {
  return(format.MakeColor(r, g, b, a));
 }

 // Gets the R/G/B/A values for the passed 32-bit surface pixel value
 INLINE void DecodeColor(uint32 value, int &r, int &g, int &b, int &a) const
 {
  format.DecodeColor(value, r, g, b, a);
 }

 INLINE void DecodeColor(uint32 value, int &r, int &g, int &b) const
 {
  int dummy_a;

  DecodeColor(value, r, g, b, dummy_a);
 }
 private:
 void Init(void *const p_pixels, const uint32 p_width, const uint32 p_height, const uint32 p_pitchinpix, const MDFN_PixelFormat &nf);
};

#endif
