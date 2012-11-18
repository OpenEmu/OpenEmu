#ifndef NALL_BMP_HPP
#define NALL_BMP_HPP

#include <nall/file.hpp>

//BMP reader / writer
//author: byuu
//note: only 24-bit RGB and 32-bit ARGB uncompressed images supported

namespace nall {

struct bmp {
  inline static bool read(const string &filename, uint32_t *&data, unsigned &width, unsigned &height);
  inline static bool write(const string &filename, const uint32_t *data, unsigned width, unsigned height, unsigned pitch, bool alpha = false);
};

bool bmp::read(const string &filename, uint32_t *&data, unsigned &width, unsigned &height) {
  file fp;
  if(fp.open(filename, file::mode::read) == false) return false;
  if(fp.size() < 0x36) return false;

  if(fp.readm(2) != 0x424d) return false;
  fp.seek(0x000a);
  unsigned offset = fp.readl(4);
  unsigned dibsize = fp.readl(4);
  if(dibsize != 40) return false;
  signed headerWidth = fp.readl(4);
  if(headerWidth < 0) return false;
  signed headerHeight = fp.readl(4);
  fp.readl(2);
  unsigned bitsPerPixel = fp.readl(2);
  if(bitsPerPixel != 24 && bitsPerPixel != 32) return false;
  unsigned compression = fp.readl(4);
  if(compression != 0) return false;
  fp.seek(offset);

  bool noFlip = headerHeight < 0;
  width = headerWidth, height = abs(headerHeight);
  data = new uint32_t[width * height];

  unsigned bytesPerPixel = bitsPerPixel / 8;
  unsigned alignedWidth  = width * bytesPerPixel;
  unsigned paddingLength = 0;
  while(alignedWidth % 4) alignedWidth++, paddingLength++;

  for(unsigned y = 0; y < height; y++) {
    uint32_t *p = noFlip ? data + y * width : data + (height - 1 - y) * width;
    for(unsigned x = 0; x < width; x++, p++) {
      *p = fp.readl(bytesPerPixel);
      if(bytesPerPixel == 3) *p |= 255 << 24;
    }
    if(paddingLength) fp.readl(paddingLength);
  }

  fp.close();
  return true;
}

bool bmp::write(const string &filename, const uint32_t *data, unsigned width, unsigned height, unsigned pitch, bool alpha) {
  file fp;
  if(fp.open(filename, file::mode::write) == false) return false;

  unsigned bitsPerPixel  = alpha ? 32 : 24;
  unsigned bytesPerPixel = bitsPerPixel / 8;
  unsigned alignedWidth  = width * bytesPerPixel;
  unsigned paddingLength = 0;
  unsigned imageSize     = alignedWidth * height;
  unsigned fileSize      = 0x36 + imageSize;
  while(alignedWidth % 4) alignedWidth++, paddingLength++;

  fp.writem(0x424d, 2);        //signature
  fp.writel(fileSize, 4);      //file size
  fp.writel(0, 2);             //reserved
  fp.writel(0, 2);             //reserved
  fp.writel(0x36, 4);          //offset

  fp.writel(40, 4);            //DIB size
  fp.writel(width, 4);         //width
  fp.writel(-height, 4);       //height
  fp.writel(1, 2);             //color planes
  fp.writel(bitsPerPixel, 2);  //bits per pixel
  fp.writel(0, 4);             //compression method (BI_RGB)
  fp.writel(imageSize, 4);     //image data size
  fp.writel(3780, 4);          //horizontal resolution
  fp.writel(3780, 4);          //vertical resolution
  fp.writel(0, 4);             //palette size
  fp.writel(0, 4);             //important color count

  for(unsigned y = 0; y < height; y++) {
    const uint32_t *p = (const uint32_t*)((const uint8_t*)data + y * pitch);
    for(unsigned x = 0; x < width; x++) fp.writel(*p++, bytesPerPixel);
    if(paddingLength) fp.writel(0, paddingLength);
  }

  fp.close();
  return true;
}

}

#endif
