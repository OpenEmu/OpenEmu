/*
 * Texture Filtering
 * Version:  1.0
 *
 * Copyright (C) 2007  Hiroshi Morii   All Rights Reserved.
 * Email koolsmoky(at)users.sourceforge.net
 * Web   http://www.3dfxzone.it/koolsmoky
 *
 * this is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2, or (at your option)
 * any later version.
 *
 * this is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with GNU Make; see the file COPYING.  If not, write to
 * the Free Software Foundation, 675 Mass Ave, Cambridge, MA 02139, USA.
 */

/* 2007 Gonetz <gonetz(at)ngs.ru>
 * Added callback to display hires texture info. */

#ifdef __MSC__
#pragma warning(disable: 4786)
#endif

/* dump processed hirestextures to disk 
 * (0:disable, 1:enable) */
#define DUMP_CACHE 1

/* handle oversized textures by
 *   0: minification
 *   1: Glide64 style tiling
 */
#define TEXTURE_TILING 1

/* use power of 2 texture size
 * (0:disable, 1:enable, 2:3dfx) */
#define POW2_TEXTURES 2

#if TEXTURE_TILING
#undef POW2_TEXTURES
#define POW2_TEXTURES 2
#endif

/* hack to reduce texture footprint to achieve
 * better performace on midrange gfx cards.
 * (0:disable, 1:enable) */
#define REDUCE_TEXTURE_FOOTPRINT 0

/* use aggressive format assumption for quantization
 * (0:disable, 1:enable, 2:extreme) */
#define AGGRESSIVE_QUANTIZATION 1

#include "TxHiResCache.h"
#include "TxDbg.h"
#include <zlib.h>
#include <string>
#include <SDL.h>

TxHiResCache::~TxHiResCache()
{
#if DUMP_CACHE
  if ((_options & DUMP_HIRESTEXCACHE) && !_haveCache && !_abortLoad) {
    /* dump cache to disk */
    std::wstring filename = _ident + L"_HIRESTEXTURES.dat";
    boost::filesystem::wpath cachepath(_path);
    cachepath /= boost::filesystem::wpath(L"cache");
    int config = _options & (HIRESTEXTURES_MASK|COMPRESS_HIRESTEX|COMPRESSION_MASK|TILE_HIRESTEX|FORCE16BPP_HIRESTEX|GZ_HIRESTEXCACHE|LET_TEXARTISTS_FLY);

    TxCache::save(cachepath.wstring().c_str(), filename.c_str(), config);
  }
#endif

  delete _txImage;
  delete _txQuantize;
  delete _txReSample;
}

TxHiResCache::TxHiResCache(int maxwidth, int maxheight, int maxbpp, int options,
                           const wchar_t *path, const wchar_t *ident,
                           dispInfoFuncExt callback
                           ) : TxCache((options & ~GZ_TEXCACHE), 0, path, ident, callback)
{
  _txImage = new TxImage();
  _txQuantize  = new TxQuantize();
  _txReSample = new TxReSample();

  _maxwidth  = maxwidth;
  _maxheight = maxheight;
  _maxbpp    = maxbpp;
  _abortLoad = 0;
  _haveCache = 0;

  /* assert local options */
  if (!(_options & COMPRESS_HIRESTEX))
    _options &= ~COMPRESSION_MASK;

  if (_path.empty() || _ident.empty()) {
    _options &= ~DUMP_HIRESTEXCACHE;
    return;
  }

#if DUMP_CACHE
  /* read in hires texture cache */
  if (_options & DUMP_HIRESTEXCACHE) {
    /* find it on disk */
    std::wstring filename = _ident + L"_HIRESTEXTURES.dat";
    boost::filesystem::wpath cachepath(_path);
    cachepath /= boost::filesystem::wpath(L"cache");
    int config = _options & (HIRESTEXTURES_MASK|COMPRESS_HIRESTEX|COMPRESSION_MASK|TILE_HIRESTEX|FORCE16BPP_HIRESTEX|GZ_HIRESTEXCACHE|LET_TEXARTISTS_FLY);

    _haveCache = TxCache::load(cachepath.wstring().c_str(), filename.c_str(), config);
  }
#endif

  /* read in hires textures */
  if (!_haveCache) TxHiResCache::load(0);
}

boolean
TxHiResCache::empty()
{
  return _cache.empty();
}

boolean
TxHiResCache::load(boolean replace) /* 0 : reload, 1 : replace partial */
{
  if (!_path.empty() && !_ident.empty()) {

    if (!replace) TxCache::clear();

    boost::filesystem::wpath dir_path(_path);

    switch (_options & HIRESTEXTURES_MASK) {
    case GHQ_HIRESTEXTURES:
      break;
    case RICE_HIRESTEXTURES:
      INFO(80, L"-----\n");
      INFO(80, L"using Rice hires texture format...\n");
      INFO(80, L"  must be one of the following;\n");
      INFO(80, L"    1) *_rgb.png + *_a.png\n");
      INFO(80, L"    2) *_all.png\n");
      INFO(80, L"    3) *_ciByRGBA.png\n");
      INFO(80, L"    4) *_allciByRGBA.png\n");
      INFO(80, L"    5) *_ci.bmp\n");
      INFO(80, L"  usage of only 2) and 3) highly recommended!\n");
      INFO(80, L"  folder names must be in US-ASCII characters!\n");

      dir_path /= boost::filesystem::wpath(L"hires_texture");
      dir_path /= boost::filesystem::wpath(_ident);
      loadHiResTextures(dir_path, replace);
      break;
    case JABO_HIRESTEXTURES:
      ;
    }

    return 1;
  }

  return 0;
}

boolean
TxHiResCache::loadHiResTextures(boost::filesystem::wpath dir_path, boolean replace)
{
  uint32_t last, now, diff;
  DBG_INFO(80, L"-----\n");
  DBG_INFO(80, L"path: %ls\n", dir_path.string().c_str());
  last = SDL_GetTicks();

  /* find it on disk */
  if (!boost::filesystem::exists(dir_path)) {
    INFO(80, L"Error: path not found!\n");
    return 0;
  }

  /* XXX: deal with UNICODE fiasco!
   * stupidity flows forth beneath this...
   *
   * I opted to use chdir in order to use fopen() for windows 9x.
   */
#ifdef WIN32
  wchar_t curpath[MAX_PATH];
  GETCWD(MAX_PATH, curpath);
  CHDIR(dir_path.wstring().c_str());
#else
  char curpath[MAX_PATH];
  char cbuf[MAX_PATH];
  wcstombs(cbuf, dir_path.wstring().c_str(), MAX_PATH);
  GETCWD(MAX_PATH, curpath);
  CHDIR(cbuf);
#endif

  /* NOTE: I could use the boost::wdirectory_iterator and boost::wpath
   * to resolve UNICODE file names and paths. But then, _wfopen() is
   * required to get the file descriptor for MS Windows to pass into
   * libpng, which is incompatible with Win9x. Win9x's fopen() cannot
   * handle UNICODE names. UNICODE capable boost::filesystem is available
   * with Boost1.34.1 built with VC8.0 (bjam --toolset=msvc-8.0 stage).
   *
   * RULE OF THUMB: NEVER save texture packs in NON-ASCII names!!
   */
  boost::filesystem::directory_iterator it(dir_path);
  boost::filesystem::directory_iterator end_it; /* default construction yields past-the-end */

  for (; it != end_it; ++it) {

    if (KBHIT(0x1B)) {
      _abortLoad = 1;
      if (_callback) (*_callback)(L"Aborted loading hiresolution texture!\n");
      INFO(80, L"Error: aborted loading hiresolution texture!\n");
    }
    if (_abortLoad) break;

    /* recursive read into sub-directory */
    if (boost::filesystem::is_directory(it->status())) {
      loadHiResTextures(it->path(), replace);
      continue;
    }

    DBG_INFO(80, L"-----\n");
    DBG_INFO(80, L"file: %ls\n", it->path().leaf().c_str());

    int width = 0, height = 0;
    uint16 format = 0;
    uint8 *tex = NULL;
    int tmpwidth = 0, tmpheight = 0;
    uint16 tmpformat = 0;
    uint8 *tmptex= NULL;
    int untiled_width = 0, untiled_height = 0;
    uint16 destformat = 0;

    /* Rice hi-res textures: begin
     */
    uint32 chksum = 0, fmt = 0, siz = 0, palchksum = 0;
    char *pfname = NULL, fname[MAX_PATH];
    std::string ident;
    FILE *fp = NULL;

    wcstombs(fname, _ident.c_str(), MAX_PATH);
    /* XXX case sensitivity fiasco!
     * files must use _a, _rgb, _all, _allciByRGBA, _ciByRGBA, _ci
     * and file extensions must be in lower case letters! */
#ifdef WIN32
    {
      unsigned int i;
      for (i = 0; i < strlen(fname); i++) fname[i] = tolower(fname[i]);
    }
#endif
    ident.assign(fname);

    /* read in Rice's file naming convention */
#define CRCFMTSIZ_LEN 13
#define PALCRC_LEN 9
    //wcstombs(fname, it->path().leaf().c_str(), MAX_PATH);
    strncpy(fname, it->path().leaf().string().c_str(), sizeof(fname));
    fname[sizeof(fname) - 1] = '\0';
    /* XXX case sensitivity fiasco!
     * files must use _a, _rgb, _all, _allciByRGBA, _ciByRGBA, _ci
     * and file extensions must be in lower case letters! */
#ifdef WIN32
    {
      unsigned int i;
      for (i = 0; i < strlen(fname); i++) fname[i] = tolower(fname[i]);
    }
#endif
    pfname = fname + strlen(fname) - 4;
    if (!(pfname == strstr(fname, ".png") ||
          pfname == strstr(fname, ".bmp") ||
          pfname == strstr(fname, ".dds"))) {
#if !DEBUG
      INFO(80, L"-----\n");
      INFO(80, L"path: %ls\n", dir_path.string().c_str());
      INFO(80, L"file: %ls\n", it->path().leaf().c_str());
#endif
      INFO(80, L"Error: not png or bmp or dds!\n");
      continue;
    }
    pfname = strstr(fname, ident.c_str());
    if (pfname != fname) pfname = 0;
    if (pfname) {
      if (sscanf(pfname + ident.size(), "#%08X#%01X#%01X#%08X", &chksum, &fmt, &siz, &palchksum) == 4)
        pfname += (ident.size() + CRCFMTSIZ_LEN + PALCRC_LEN);
      else if (sscanf(pfname + ident.size(), "#%08X#%01X#%01X", &chksum, &fmt, &siz) == 3)
        pfname += (ident.size() + CRCFMTSIZ_LEN);
      else
        pfname = 0;
    }
    if (!pfname) {
#if !DEBUG
      INFO(80, L"-----\n");
      INFO(80, L"path: %ls\n", dir_path.string().c_str());
      INFO(80, L"file: %ls\n", it->path().leaf().c_str());
#endif
      INFO(80, L"Error: not Rice texture naming convention!\n");
      continue;
    }
    if (!chksum) {
#if !DEBUG
      INFO(80, L"-----\n");
      INFO(80, L"path: %ls\n", dir_path.string().c_str());
      INFO(80, L"file: %ls\n", it->path().leaf().c_str());
#endif
      INFO(80, L"Error: crc32 = 0!\n");
      continue;
    }

    /* check if we already have it in hires texture cache */
    if (!replace) {
      uint64 chksum64 = (uint64)palchksum;
      chksum64 <<= 32;
      chksum64 |= (uint64)chksum;
      if (TxCache::is_cached(chksum64)) {
#if !DEBUG
        INFO(80, L"-----\n");
        INFO(80, L"path: %ls\n", dir_path.string().c_str());
        INFO(80, L"file: %ls\n", it->path().leaf().c_str());
#endif
        INFO(80, L"Error: already cached! duplicate texture!\n");
        continue;
      }
    }

    DBG_INFO(80, L"rom: %ls chksum:%08X %08X fmt:%x size:%x\n", _ident.c_str(), chksum, palchksum, fmt, siz);

    /* Deal with the wackiness some texture packs utilize Rice format.
     * Read in the following order: _a.* + _rgb.*, _all.png _ciByRGBA.png,
     * _allciByRGBA.png, and _ci.bmp. PNG are prefered over BMP.
     *
     * For some reason there are texture packs that include them all. Some
     * even have RGB textures named as _all.* and ARGB textures named as
     * _rgb.*... Someone pleeeez write a GOOD guideline for the texture
     * designers!!!
     *
     * We allow hires textures to have higher bpp than the N64 originals.
     */
    /* N64 formats
     * Format: 0 - RGBA, 1 - YUV, 2 - CI, 3 - IA, 4 - I
     * Size:   0 - 4bit, 1 - 8bit, 2 - 16bit, 3 - 32 bit
     */

    /*
     * read in _rgb.* and _a.*
     */
    if (pfname == strstr(fname, "_rgb.") || pfname == strstr(fname, "_a.")) {
      strcpy(pfname, "_rgb.png");
      if (!boost::filesystem::exists(fname)) {
        strcpy(pfname, "_rgb.bmp");
        if (!boost::filesystem::exists(fname)) {
#if !DEBUG
          INFO(80, L"-----\n");
          INFO(80, L"path: %ls\n", dir_path.string().c_str());
          INFO(80, L"file: %ls\n", it->path().leaf().c_str());
#endif
          INFO(80, L"Error: missing _rgb.*! _a.* must be paired with _rgb.*!\n");
          continue;
        }
      }
      /* _a.png */
      strcpy(pfname, "_a.png");
      if ((fp = fopen(fname, "rb")) != NULL) {
        tmptex = _txImage->readPNG(fp, &tmpwidth, &tmpheight, &tmpformat);
        fclose(fp);
      }
      if (!tmptex) {
        /* _a.bmp */
        strcpy(pfname, "_a.bmp");
        if ((fp = fopen(fname, "rb")) != NULL) {
          tmptex = _txImage->readBMP(fp, &tmpwidth, &tmpheight, &tmpformat);
          fclose(fp);
        }
      }
      /* _rgb.png */
      strcpy(pfname, "_rgb.png");
      if ((fp = fopen(fname, "rb")) != NULL) {
        tex = _txImage->readPNG(fp, &width, &height, &format);
        fclose(fp);
      }
      if (!tex) {
        /* _rgb.bmp */
        strcpy(pfname, "_rgb.bmp");
        if ((fp = fopen(fname, "rb")) != NULL) {
          tex = _txImage->readBMP(fp, &width, &height, &format);
          fclose(fp);
        }
      }
      if (tmptex) {
        /* check if _rgb.* and _a.* have matching size and format. */
        if (!tex || width != tmpwidth || height != tmpheight ||
            format != GR_TEXFMT_ARGB_8888 || tmpformat != GR_TEXFMT_ARGB_8888) {
#if !DEBUG
          INFO(80, L"-----\n");
          INFO(80, L"path: %ls\n", dir_path.string().c_str());
          INFO(80, L"file: %ls\n", it->path().leaf().c_str());
#endif
          if (!tex) {
            INFO(80, L"Error: missing _rgb.*!\n");
          } else if (width != tmpwidth || height != tmpheight) {
            INFO(80, L"Error: _rgb.* and _a.* have mismatched width or height!\n");
          } else if (format != GR_TEXFMT_ARGB_8888 || tmpformat != GR_TEXFMT_ARGB_8888) {
            INFO(80, L"Error: _rgb.* or _a.* not in 32bit color!\n");
          }
          if (tex) free(tex);
          if (tmptex) free(tmptex);
          tex = NULL;
          tmptex = NULL;
          continue;
        }
      }
      /* make adjustments */
      if (tex) {
        if (tmptex) {
          /* merge (A)RGB and A comp */
          DBG_INFO(80, L"merge (A)RGB and A comp\n");
          int i;
          for (i = 0; i < height * width; i++) {
#if 1
            /* use R comp for alpha. this is what Rice uses. sigh... */
            ((uint32*)tex)[i] &= 0x00ffffff;
            ((uint32*)tex)[i] |= ((((uint32*)tmptex)[i] & 0x00ff0000) << 8);
#endif
#if 0
            /* use libpng style grayscale conversion */
            uint32 texel = ((uint32*)tmptex)[i];
            uint32 acomp = (((texel >> 16) & 0xff) * 6969 +
                            ((texel >>  8) & 0xff) * 23434 +
                            ((texel      ) & 0xff) * 2365) / 32768;
            ((uint32*)tex)[i] = (acomp << 24) | (((uint32*)tex)[i] & 0x00ffffff);
#endif
#if 0
            /* use the standard NTSC gray scale conversion */
            uint32 texel = ((uint32*)tmptex)[i];
            uint32 acomp = (((texel >> 16) & 0xff) * 299 +
                            ((texel >>  8) & 0xff) * 587 +
                            ((texel      ) & 0xff) * 114) / 1000;
            ((uint32*)tex)[i] = (acomp << 24) | (((uint32*)tex)[i] & 0x00ffffff);
#endif
          }
          free(tmptex);
          tmptex = NULL;
        } else {
          /* clobber A comp. never a question of alpha. only RGB used. */
#if !DEBUG
          INFO(80, L"-----\n");
          INFO(80, L"path: %ls\n", dir_path.string().c_str());
          INFO(80, L"file: %ls\n", it->path().leaf().c_str());
#endif
          INFO(80, L"Warning: missing _a.*! only using _rgb.*. treat as opaque texture.\n");
          int i;
          for (i = 0; i < height * width; i++) {
            ((uint32*)tex)[i] |= 0xff000000;
          }
        }
      }
    } else

    /*
     * read in _all.png, _all.dds, _allciByRGBA.png, _allciByRGBA.dds
     * _ciByRGBA.png, _ciByRGBA.dds, _ci.bmp
     */
    if (pfname == strstr(fname, "_all.png") ||
        pfname == strstr(fname, "_all.dds") ||
#ifdef WIN32
        pfname == strstr(fname, "_allcibyrgba.png") ||
        pfname == strstr(fname, "_allcibyrgba.dds") ||
        pfname == strstr(fname, "_cibyrgba.png") ||
        pfname == strstr(fname, "_cibyrgba.dds") ||
#else
        pfname == strstr(fname, "_allciByRGBA.png") ||
        pfname == strstr(fname, "_allciByRGBA.dds") ||
        pfname == strstr(fname, "_ciByRGBA.png") ||
        pfname == strstr(fname, "_ciByRGBA.dds") ||
#endif
        pfname == strstr(fname, "_ci.bmp")) {
      if ((fp = fopen(fname, "rb")) != NULL) {
        if      (strstr(fname, ".png")) tex = _txImage->readPNG(fp, &width, &height, &format);
        else if (strstr(fname, ".dds")) tex = _txImage->readDDS(fp, &width, &height, &format);
        else                            tex = _txImage->readBMP(fp, &width, &height, &format);
        fclose(fp);
      }
      /* XXX: auto-adjustment of dxt dds textures unsupported for now */
      if (tex && strstr(fname, ".dds")) {
        const float aspectratio = (width > height) ? (float)width/(float)height : (float)height/(float)width;
        if (!(aspectratio == 1.0 ||
              aspectratio == 2.0 ||
              aspectratio == 4.0 ||
              aspectratio == 8.0)) {
          free(tex);
          tex = NULL;
#if !DEBUG
          INFO(80, L"-----\n");
          INFO(80, L"path: %ls\n", dir_path.string().c_str());
          INFO(80, L"file: %ls\n", it->path().leaf().c_str());
#endif
          INFO(80, L"Error: W:H aspect ratio range not 8:1 - 1:8!\n");
          continue;
        }
        if (width  != _txReSample->nextPow2(width) ||
            height != _txReSample->nextPow2(height)) {
          free(tex);
          tex = NULL;
#if !DEBUG
          INFO(80, L"-----\n");
          INFO(80, L"path: %ls\n", dir_path.string().c_str());
          INFO(80, L"file: %ls\n", it->path().leaf().c_str());
#endif
          INFO(80, L"Error: not power of 2 size!\n");
          continue;
        }
      }
    }

    /* if we do not have a texture at this point we are screwed */
    if (!tex) {
#if !DEBUG
      INFO(80, L"-----\n");
      INFO(80, L"path: %ls\n", dir_path.string().c_str());
      INFO(80, L"file: %ls\n", it->path().leaf().c_str());
#endif
      INFO(80, L"Error: load failed!\n");
      continue;
    }
    DBG_INFO(80, L"read in as %d x %d gfmt:%x\n", tmpwidth, tmpheight, tmpformat);

    /* check if size and format are OK */
    if (!(format == GR_TEXFMT_ARGB_8888     ||
          format == GR_TEXFMT_P_8           ||
          format == GR_TEXFMT_ARGB_CMP_DXT1 ||
          format == GR_TEXFMT_ARGB_CMP_DXT3 ||
          format == GR_TEXFMT_ARGB_CMP_DXT5) ||
        (width * height) < 4) { /* TxQuantize requirement: width * height must be 4 or larger. */
      free(tex);
      tex = NULL;
#if !DEBUG
      INFO(80, L"-----\n");
      INFO(80, L"path: %ls\n", dir_path.string().c_str());
      INFO(80, L"file: %ls\n", it->path().leaf().c_str());
#endif
      INFO(80, L"Error: not width * height > 4 or 8bit palette color or 32bpp or dxt1 or dxt3 or dxt5!\n");
      continue;
    }

    /* analyze and determine best format to quantize */
    if (format == GR_TEXFMT_ARGB_8888) {
      int i;
      int alphabits = 0;
      int fullalpha = 0;
      boolean intensity = 1;

      if (!(_options & LET_TEXARTISTS_FLY)) {
        /* HACK ALERT! */
        /* Account for Rice's weirdness with fmt:0 siz:2 textures.
         * Although the conditions are relaxed with other formats,
         * the D3D RGBA5551 surface is used for this format in certain
         * cases. See Nintemod's SuperMario64 life gauge and power
         * meter. The same goes for fmt:2 textures. See Mollymutt's
         * PaperMario text. */
        if ((fmt == 0 && siz == 2) || fmt == 2) {
          DBG_INFO(80, L"Remove black, white, etc borders along the alpha edges.\n");
          /* round A comp */
          for (i = 0; i < height * width; i++) {
            uint32 texel = ((uint32*)tex)[i];
            ((uint32*)tex)[i] = ((texel & 0xff000000) == 0xff000000 ? 0xff000000 : 0) |
                                (texel & 0x00ffffff);
          }
          /* Substitute texel color with the average of the surrounding
           * opaque texels. This removes borders regardless of hardware
           * texture filtering (bilinear, etc). */
          int j;
          for (i = 0; i < height; i++) {
            for (j = 0; j < width; j++) {
              uint32 texel = ((uint32*)tex)[i * width + j];
              if ((texel & 0xff000000) != 0xff000000) {
                uint32 tmptexel[8];
                uint32 k, numtexel, r, g, b;
                numtexel = r = g = b = 0;
                memset(&tmptexel, 0, sizeof(tmptexel));
                if (i > 0) {
                  tmptexel[0] = ((uint32*)tex)[(i - 1) * width + j];                        /* north */
                  if (j > 0)         tmptexel[1] = ((uint32*)tex)[(i - 1) * width + j - 1]; /* north-west */
                  if (j < width - 1) tmptexel[2] = ((uint32*)tex)[(i - 1) * width + j + 1]; /* north-east */
                }
                if (i < height - 1) {
                  tmptexel[3] = ((uint32*)tex)[(i + 1) * width + j];                        /* south */
                  if (j > 0)         tmptexel[4] = ((uint32*)tex)[(i + 1) * width + j - 1]; /* south-west */
                  if (j < width - 1) tmptexel[5] = ((uint32*)tex)[(i + 1) * width + j + 1]; /* south-east */
                }
                if (j > 0)         tmptexel[6] = ((uint32*)tex)[i * width + j - 1]; /* west */
                if (j < width - 1) tmptexel[7] = ((uint32*)tex)[i * width + j + 1]; /* east */
                for (k = 0; k < 8; k++) {
                  if ((tmptexel[k] & 0xff000000) == 0xff000000) {
                    r += ((tmptexel[k] & 0x00ff0000) >> 16);
                    g += ((tmptexel[k] & 0x0000ff00) >>  8);
                    b += ((tmptexel[k] & 0x000000ff)      );
                    numtexel++;
                  }
                }
                if (numtexel) {
                  ((uint32*)tex)[i * width + j] = ((r / numtexel) << 16) |
                                                  ((g / numtexel) <<  8) |
                                                  ((b / numtexel)      );
                } else {
                  ((uint32*)tex)[i * width + j] = texel & 0x00ffffff;
                }
              }
            }
          }
        }
      }

      /* simple analysis of texture */
      for (i = 0; i < height * width; i++) {
        uint32 texel = ((uint32*)tex)[i];
        if (alphabits != 8) {
#if AGGRESSIVE_QUANTIZATION
          if ((texel & 0xff000000) < 0x00000003) {
            alphabits = 1;
            fullalpha++;
          } else if ((texel & 0xff000000) < 0xfe000000) {
            alphabits = 8;
          }
#else
          if ((texel & 0xff000000) == 0x00000000) {
            alphabits = 1;
            fullalpha++;
          } else if ((texel & 0xff000000) != 0xff000000) {
            alphabits = 8;
          }
#endif
        }
        if (intensity) {
          int rcomp = (texel >> 16) & 0xff;
          int gcomp = (texel >>  8) & 0xff;
          int bcomp = (texel      ) & 0xff;
#if AGGRESSIVE_QUANTIZATION
          if (abs(rcomp - gcomp) > 8 || abs(rcomp - bcomp) > 8 || abs(gcomp - bcomp) > 8) intensity = 0;
#else
          if (rcomp != gcomp || rcomp != bcomp || gcomp != bcomp) intensity = 0;
#endif
        }
        if (!intensity && alphabits == 8) break;
      }
      DBG_INFO(80, L"required alpha bits:%d zero acomp texels:%d rgb as intensity:%d\n", alphabits, fullalpha, intensity);

      /* preparations based on above analysis */
#if !REDUCE_TEXTURE_FOOTPRINT
      if (_maxbpp < 32 || _options & (FORCE16BPP_HIRESTEX|COMPRESSION_MASK)) {
#endif
        if      (alphabits == 0) destformat = GR_TEXFMT_RGB_565;
        else if (alphabits == 1) destformat = GR_TEXFMT_ARGB_1555;
        else                     destformat = GR_TEXFMT_ARGB_8888;
#if !REDUCE_TEXTURE_FOOTPRINT
      } else {
        destformat = GR_TEXFMT_ARGB_8888;
      }
#endif
      if (fmt == 4 && alphabits == 0) {
        destformat = GR_TEXFMT_ARGB_8888;
        /* Rice I format; I = (R + G + B) / 3 */
        for (i = 0; i < height * width; i++) {
          uint32 texel = ((uint32*)tex)[i];
          uint32 icomp = (((texel >> 16) & 0xff) +
                          ((texel >>  8) & 0xff) +
                          ((texel      ) & 0xff)) / 3;
          ((uint32*)tex)[i] = (icomp << 24) | (texel & 0x00ffffff);
        }
      }
      if (intensity) {
        if (alphabits == 0) {
          if (fmt == 4) destformat = GR_TEXFMT_ALPHA_8;
          else          destformat = GR_TEXFMT_INTENSITY_8;
        } else {
          destformat = GR_TEXFMT_ALPHA_INTENSITY_88;
        }
      }

      DBG_INFO(80, L"best gfmt:%x\n", destformat);
    }
    /*
     * Rice hi-res textures: end */


    /* XXX: only ARGB8888 for now. comeback to this later... */
    if (format == GR_TEXFMT_ARGB_8888) {

#if TEXTURE_TILING

      /* Glide64 style texture tiling */
      /* NOTE: narrow wide textures can be tiled into 256x256 size textures */

      /* adjust texture size to allow tiling for V1, Rush, V2, Banshee, V3 */
      /* NOTE: we skip this for palette textures that need minification
       * becasue it will look ugly. */

      /* minification */
      {
        int ratio = 1;

        /* minification to enable glide64 style texture tiling */
        /* determine the minification ratio to tile the texture into 256x256 size */
        if ((_options & TILE_HIRESTEX) && _maxwidth >= 256 && _maxheight >= 256) {
          DBG_INFO(80, L"determine minification ratio to tile\n");
          tmpwidth = width;
          tmpheight = height;
          if (height > 256) {
            ratio = ((height - 1) >> 8) + 1;
            tmpwidth = width / ratio;
            tmpheight = height / ratio;
            DBG_INFO(80, L"height > 256, minification ratio:%d %d x %d -> %d x %d\n",
                     ratio, width, height, tmpwidth, tmpheight);
          }
          if (tmpwidth > 256 && (((tmpwidth - 1) >> 8) + 1) * tmpheight > 256) {
            ratio *= ((((((tmpwidth - 1) >> 8) + 1) * tmpheight) - 1) >> 8) + 1;
            DBG_INFO(80, L"width > 256, minification ratio:%d %d x %d -> %d x %d\n",
                     ratio, width, height, width / ratio, height / ratio);
          }
        } else {
          /* normal minification to fit max texture size */
          if (width > _maxwidth || height > _maxheight) {
            DBG_INFO(80, L"determine minification ratio to fit max texture size\n");
            tmpwidth = width;
            tmpheight = height;
            while (tmpwidth > _maxwidth) {
              tmpheight >>= 1;
              tmpwidth >>= 1;
              ratio <<= 1;
            }
            while (tmpheight > _maxheight) {
              tmpheight >>= 1;
              tmpwidth >>= 1;
              ratio <<= 1;
            }
            DBG_INFO(80, L"minification ratio:%d %d x %d -> %d x %d\n",
                     ratio, width, height, tmpwidth, tmpheight);
          }
        }

        if (ratio > 1) {
          if (!_txReSample->minify(&tex, &width, &height, ratio)) {
            free(tex);
            tex = NULL;
            DBG_INFO(80, L"Error: minification failed!\n");
            continue;
          }
        }
      }

      /* tiling */
      if ((_options & TILE_HIRESTEX) && _maxwidth >= 256 && _maxheight >= 256) {
        boolean usetile = 0;

        /* to tile or not to tile, that is the question */
        if (width > 256 && height <= 128 && (((width - 1) >> 8) + 1) * height <= 256) {

          if (width > _maxwidth) usetile = 1;
          else {
            /* tile if the tiled texture memory footprint is smaller */
            int tilewidth  = 256;
            int tileheight = _txReSample->nextPow2((((width - 1) >> 8) + 1) * height);
            tmpwidth  = width;
            tmpheight = height;

            /* 3dfx Glide3 tmpheight, W:H aspect ratio range (8:1 - 1:8) */
            if (tilewidth > (tileheight << 3)) tileheight = tilewidth >> 3;

            /* HACKALERT: see TxReSample::pow2(); */
            if      (tmpwidth  > 64) tmpwidth  -= 4;
            else if (tmpwidth  > 16) tmpwidth  -= 2;
            else if (tmpwidth  >  4) tmpwidth  -= 1;

            if      (tmpheight > 64) tmpheight -= 4;
            else if (tmpheight > 16) tmpheight -= 2;
            else if (tmpheight >  4) tmpheight -= 1;

            tmpwidth  = _txReSample->nextPow2(tmpwidth);
            tmpheight = _txReSample->nextPow2(tmpheight);

            /* 3dfx Glide3 tmpheight, W:H aspect ratio range (8:1 - 1:8) */
            if (tmpwidth > tmpheight) {
              if (tmpwidth  > (tmpheight << 3)) tmpheight = tmpwidth  >> 3;
            } else {
              if (tmpheight > (tmpwidth  << 3)) tmpwidth  = tmpheight >> 3;
            }

            usetile = (tilewidth * tileheight < tmpwidth * tmpheight);
          }

        }

        /* tile it! do the actual tiling into 256x256 size */
        if (usetile) {
          DBG_INFO(80, L"Glide64 style texture tiling\n");

          int x, y, z, ratio, offset;
          offset = 0;
          ratio = ((width - 1) >> 8) + 1;
          tmptex = (uint8 *)malloc(_txUtil->sizeofTx(256, height * ratio, format));
          if (tmptex) {
            for (x = 0; x < ratio; x++) {
              for (y = 0; y < height; y++) {
                if (x < ratio - 1) {
                  memcpy(&tmptex[offset << 2], &tex[(x * 256 + y * width) << 2], 256 << 2);
                } else {
                  for (z = 0; z < width - 256 * (ratio - 1); z++) {
                    ((uint32*)tmptex)[offset + z] = ((uint32*)tex)[x * 256 + y * width + z];
                  }
                  for (; z < 256; z++) {
                    ((uint32*)tmptex)[offset + z] = ((uint32*)tmptex)[offset + z - 1];
                  }
                }
                offset += 256;
              }
            }
            free(tex);
            tex = tmptex;
            untiled_width = width;
            untiled_height = height;
            width = 256;
            height *= ratio;
            DBG_INFO(80, L"Tiled: %d x %d -> %d x %d\n", untiled_width, untiled_height, width, height);
          }
        }
      }

#else  /* TEXTURE_TILING */

      /* minification */
      if (width > _maxwidth || height > _maxheight) {
        int ratio = 1;
        if (width / _maxwidth > height / _maxheight) {
          ratio = (int)ceil((double)width / _maxwidth);
        } else {
          ratio = (int)ceil((double)height / _maxheight);
        }
        if (!_txReSample->minify(&tex, &width, &height, ratio)) {
          free(tex);
          tex = NULL;
          DBG_INFO(80, L"Error: minification failed!\n");
          continue;
        }
      }

#endif /* TEXTURE_TILING */

      /* texture compression */
      if ((_options & COMPRESSION_MASK) &&
          (width >= 64 && height >= 64) /* Texture compression is not suitable for low pixel coarse detail
                                         * textures. The assumption here is that textures larger than 64x64
                                         * have enough detail to produce decent quality when compressed. The
                                         * down side is that narrow stripped textures that the N64 often use
                                         * for large background textures are also ignored. It would be more
                                         * reasonable if decisions are made based on fourier-transform
                                         * spectrum or RMS error.
                                         *
                                         * NOTE: texture size must be checked before expanding to pow2 size.
                                         */
          ) {
        uint32 alpha = 0;
        int dataSize = 0;
        int compressionType = _options & COMPRESSION_MASK;

#if POW2_TEXTURES
#if (POW2_TEXTURES == 2)
        /* 3dfx Glide3x aspect ratio (8:1 - 1:8) */
        if (!_txReSample->nextPow2(&tex, &width , &height, 32, 1)) {
#else
        /* normal pow2 expansion */
        if (!_txReSample->nextPow2(&tex, &width , &height, 32, 0)) {
#endif
          free(tex);
          tex = NULL;
          DBG_INFO(80, L"Error: aspect ratio adjustment failed!\n");
          continue;
        }
#endif

        switch (_options & COMPRESSION_MASK) {
        case S3TC_COMPRESSION:
          switch (destformat) {
          case GR_TEXFMT_ARGB_8888:
#if GLIDE64_DXTN
          case GR_TEXFMT_ARGB_1555: /* for ARGB1555 use DXT5 instead of DXT1 */
#endif
          case GR_TEXFMT_ALPHA_INTENSITY_88:
            dataSize = width * height;
            break;
#if !GLIDE64_DXTN
          case GR_TEXFMT_ARGB_1555:
#endif
          case GR_TEXFMT_RGB_565:
          case GR_TEXFMT_INTENSITY_8:
            dataSize = (width * height) >> 1;
            break;
          case GR_TEXFMT_ALPHA_8: /* no size benefit with dxtn */
            ;
          }
          break;
        case FXT1_COMPRESSION:
          switch (destformat) {
          case GR_TEXFMT_ARGB_1555:
          case GR_TEXFMT_RGB_565:
          case GR_TEXFMT_INTENSITY_8:
            dataSize = (width * height) >> 1;
            break;
            /* XXX: textures that use 8bit alpha channel look bad with the current
             * fxt1 library, so we substitute it with dxtn for now. afaik all gfx
             * cards that support fxt1 also support dxtn. (3dfx and Intel) */
          case GR_TEXFMT_ALPHA_INTENSITY_88:
          case GR_TEXFMT_ARGB_8888:
            compressionType = S3TC_COMPRESSION;
            dataSize = width * height;
            break;
          case GR_TEXFMT_ALPHA_8: /* no size benefit with dxtn */
            ;
          }
        }
        /* compress it! */
        if (dataSize) {
#if 0 /* TEST: dither before compression for better results with gradients */
          tmptex = (uint8 *)malloc(_txUtil->sizeofTx(width, height, destformat));
          if (tmptex) {
            if (_txQuantize->quantize(tex, tmptex, width, height, GR_TEXFMT_ARGB_8888, destformat, 0))
              _txQuantize->quantize(tmptex, tex, width, height, destformat, GR_TEXFMT_ARGB_8888, 0);
            free(tmptex);
          }
#endif
          tmptex = (uint8 *)malloc(dataSize);
          if (tmptex) {
            if (_txQuantize->compress(tex, tmptex,
                                      width, height, destformat,
                                      &tmpwidth, &tmpheight, &tmpformat,
                                      compressionType)) {
              free(tex);
              tex = tmptex;
              width = tmpwidth;
              height = tmpheight;
              format = destformat = tmpformat;
            } else {
              free(tmptex);
            }
          }
        }

      } else {

#if POW2_TEXTURES
#if (POW2_TEXTURES == 2)
        /* 3dfx Glide3x aspect ratio (8:1 - 1:8) */
        if (!_txReSample->nextPow2(&tex, &width , &height, 32, 1)) {
#else
        /* normal pow2 expansion */
        if (!_txReSample->nextPow2(&tex, &width , &height, 32, 0)) {
#endif
          free(tex);
          tex = NULL;
          DBG_INFO(80, L"Error: aspect ratio adjustment failed!\n");
          continue;
        }
#endif
      }

      /* quantize */
      {
        tmptex = (uint8 *)malloc(_txUtil->sizeofTx(width, height, destformat));
        if (tmptex) {
          switch (destformat) {
          case GR_TEXFMT_ARGB_8888:
          case GR_TEXFMT_ARGB_4444:
#if !REDUCE_TEXTURE_FOOTPRINT
            if (_maxbpp < 32 || _options & FORCE16BPP_HIRESTEX)
#endif
              destformat = GR_TEXFMT_ARGB_4444;
            break;
          case GR_TEXFMT_ARGB_1555:
#if !REDUCE_TEXTURE_FOOTPRINT
            if (_maxbpp < 32 || _options & FORCE16BPP_HIRESTEX)
#endif
              destformat = GR_TEXFMT_ARGB_1555;
            break;
          case GR_TEXFMT_RGB_565:
#if !REDUCE_TEXTURE_FOOTPRINT
            if (_maxbpp < 32 || _options & FORCE16BPP_HIRESTEX)
#endif
              destformat = GR_TEXFMT_RGB_565;
            break;
          case GR_TEXFMT_ALPHA_INTENSITY_88:
          case GR_TEXFMT_ALPHA_INTENSITY_44:
#if !REDUCE_TEXTURE_FOOTPRINT
            destformat = GR_TEXFMT_ALPHA_INTENSITY_88;
#else
            destformat = GR_TEXFMT_ALPHA_INTENSITY_44;
#endif
            break;
          case GR_TEXFMT_ALPHA_8:
            destformat = GR_TEXFMT_ALPHA_8; /* yes, this is correct. ALPHA_8 instead of INTENSITY_8 */
            break;
          case GR_TEXFMT_INTENSITY_8:
            destformat = GR_TEXFMT_INTENSITY_8;
          }
          if (_txQuantize->quantize(tex, tmptex, width, height, GR_TEXFMT_ARGB_8888, destformat, 0)) {
            format = destformat;
            free(tex);
            tex = tmptex;
          }
        }
      }

    }


    /* last minute validations */
    if (!tex || !chksum || !width || !height || !format || width > _maxwidth || height > _maxheight) {
#if !DEBUG
      INFO(80, L"-----\n");
      INFO(80, L"path: %ls\n", dir_path.string().c_str());
      INFO(80, L"file: %ls\n", it->path().leaf().c_str());
#endif
      if (tex) {
        free(tex);
        tex = NULL;
        INFO(80, L"Error: bad format or size! %d x %d gfmt:%x\n", width, height, format);
      } else {
        INFO(80, L"Error: load failed!!\n");
      }
      continue;
    }

    /* load it into hires texture cache. */
    {
      uint64 chksum64 = (uint64)palchksum;
      chksum64 <<= 32;
      chksum64 |= (uint64)chksum;

      GHQTexInfo tmpInfo;
      memset(&tmpInfo, 0, sizeof(GHQTexInfo));

      tmpInfo.data = tex;
      tmpInfo.width = width;
      tmpInfo.height = height;
      tmpInfo.format = format;
      tmpInfo.largeLodLog2 = _txUtil->grLodLog2(width, height);
      tmpInfo.smallLodLog2 = tmpInfo.largeLodLog2;
      tmpInfo.aspectRatioLog2 = _txUtil->grAspectRatioLog2(width, height);
      tmpInfo.is_hires_tex = 1;

#if TEXTURE_TILING
      /* Glide64 style texture tiling. */
      if (untiled_width && untiled_height) {
        tmpInfo.tiles = ((untiled_width - 1) >> 8) + 1;
        tmpInfo.untiled_width = untiled_width;
        tmpInfo.untiled_height = untiled_height;
      }
#endif

      /* remove redundant in cache */
      if (replace && TxCache::del(chksum64)) {
        DBG_INFO(80, L"removed duplicate old cache.\n");
      }

      /* add to cache */
      if (TxCache::add(chksum64, &tmpInfo)) {
        now = SDL_GetTicks();
        diff = now - last;

        /* Callback to display hires texture info.
         * Gonetz <gonetz(at)ngs.ru> */
        if (_callback && diff > 250) {
          wchar_t tmpbuf[MAX_PATH];
          mbstowcs(tmpbuf, fname, MAX_PATH);
          (*_callback)(L"[%d] total mem:%.2fmb - %ls\n", _cache.size(), (float)_totalSize/1000000, tmpbuf);
          last = now;
        }
        DBG_INFO(80, L"texture loaded!\n");
      }
      free(tex);
    }

  }

  CHDIR(curpath);

  return 1;
}
