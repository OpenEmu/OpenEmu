/**********************************************************************************
  Snes9x - Portable Super Nintendo Entertainment System (TM) emulator.

  (c) Copyright 1996 - 2002  Gary Henderson (gary.henderson@ntlworld.com),
                             Jerremy Koot (jkoot@snes9x.com)

  (c) Copyright 2002 - 2004  Matthew Kendora

  (c) Copyright 2002 - 2005  Peter Bortas (peter@bortas.org)

  (c) Copyright 2004 - 2005  Joel Yliluoma (http://iki.fi/bisqwit/)

  (c) Copyright 2001 - 2006  John Weidman (jweidman@slip.net)

  (c) Copyright 2002 - 2006  funkyass (funkyass@spam.shaw.ca),
                             Kris Bleakley (codeviolation@hotmail.com)

  (c) Copyright 2002 - 2007  Brad Jorsch (anomie@users.sourceforge.net),
                             Nach (n-a-c-h@users.sourceforge.net),
                             zones (kasumitokoduck@yahoo.com)

  (c) Copyright 2006 - 2007  nitsuja


  BS-X C emulator code
  (c) Copyright 2005 - 2006  Dreamer Nom,
                             zones

  C4 x86 assembler and some C emulation code
  (c) Copyright 2000 - 2003  _Demo_ (_demo_@zsnes.com),
                             Nach,
                             zsKnight (zsknight@zsnes.com)

  C4 C++ code
  (c) Copyright 2003 - 2006  Brad Jorsch,
                             Nach

  DSP-1 emulator code
  (c) Copyright 1998 - 2006  _Demo_,
                             Andreas Naive (andreasnaive@gmail.com)
                             Gary Henderson,
                             Ivar (ivar@snes9x.com),
                             John Weidman,
                             Kris Bleakley,
                             Matthew Kendora,
                             Nach,
                             neviksti (neviksti@hotmail.com)

  DSP-2 emulator code
  (c) Copyright 2003         John Weidman,
                             Kris Bleakley,
                             Lord Nightmare (lord_nightmare@users.sourceforge.net),
                             Matthew Kendora,
                             neviksti


  DSP-3 emulator code
  (c) Copyright 2003 - 2006  John Weidman,
                             Kris Bleakley,
                             Lancer,
                             z80 gaiden

  DSP-4 emulator code
  (c) Copyright 2004 - 2006  Dreamer Nom,
                             John Weidman,
                             Kris Bleakley,
                             Nach,
                             z80 gaiden

  OBC1 emulator code
  (c) Copyright 2001 - 2004  zsKnight,
                             pagefault (pagefault@zsnes.com),
                             Kris Bleakley,
                             Ported from x86 assembler to C by sanmaiwashi

  SPC7110 and RTC C++ emulator code
  (c) Copyright 2002         Matthew Kendora with research by
                             zsKnight,
                             John Weidman,
                             Dark Force

  S-DD1 C emulator code
  (c) Copyright 2003         Brad Jorsch with research by
                             Andreas Naive,
                             John Weidman

  S-RTC C emulator code
  (c) Copyright 2001-2006    byuu,
                             John Weidman

  ST010 C++ emulator code
  (c) Copyright 2003         Feather,
                             John Weidman,
                             Kris Bleakley,
                             Matthew Kendora

  Super FX x86 assembler emulator code
  (c) Copyright 1998 - 2003  _Demo_,
                             pagefault,
                             zsKnight,

  Super FX C emulator code
  (c) Copyright 1997 - 1999  Ivar,
                             Gary Henderson,
                             John Weidman

  Sound DSP emulator code is derived from SNEeSe and OpenSPC:
  (c) Copyright 1998 - 2003  Brad Martin
  (c) Copyright 1998 - 2006  Charles Bilyue'

  SH assembler code partly based on x86 assembler code
  (c) Copyright 2002 - 2004  Marcus Comstedt (marcus@mc.pp.se)

  2xSaI filter
  (c) Copyright 1999 - 2001  Derek Liauw Kie Fa

  HQ2x, HQ3x, HQ4x filters
  (c) Copyright 2003         Maxim Stepin (maxim@hiend3d.com)

  Win32 GUI code
  (c) Copyright 2003 - 2006  blip,
                             funkyass,
                             Matthew Kendora,
                             Nach,
                             nitsuja

  Mac OS GUI code
  (c) Copyright 1998 - 2001  John Stiles
  (c) Copyright 2001 - 2007  zones


  Specific ports contains the works of other authors. See headers in
  individual files.


  Snes9x homepage: http://www.snes9x.com

  Permission to use, copy, modify and/or distribute Snes9x in both binary
  and source form, for non-commercial purposes, is hereby granted without
  fee, providing that this license information and copyright notice appear
  with all copies and any derived work.

  This software is provided 'as-is', without any express or implied
  warranty. In no event shall the authors be held liable for any damages
  arising from the use of this software or it's derivatives.

  Snes9x is freeware for PERSONAL USE only. Commercial users should
  seek permission of the copyright holders first. Commercial use includes,
  but is not limited to, charging money for Snes9x or software derived from
  Snes9x, including Snes9x or derivatives in commercial game bundles, and/or
  using Snes9x as a promotion for your commercial product.

  The copyright holders request that bug fixes and improvements to the code
  should be forwarded to them so everyone can benefit from the modifications
  in future versions.

  Super NES and Super Nintendo Entertainment System are trademarks of
  Nintendo Co., Limited and its subsidiary companies.
**********************************************************************************/



#ifdef HAVE_CONFIG_H
        #include <config.h>
#endif
#include <stdio.h>
#include <stdlib.h>

#ifdef HAVE_LIBPNG
#include <png.h>
#endif

#include "port.h"
#include "crosshairs.h"

static char *crosshairs[32]={
    "`              "  /* Crosshair 0 (no image) */
    "               "
    "               "
    "               "
    "               "
    "               "
    "               "
    "               "
    "               "
    "               "
    "               "
    "               "
    "               "
    "               "
    "               ",

    "`              "  /* Crosshair 1 (the classic small dot) */
    "               "
    "               "
    "               "
    "               "
    "               "
    "               "
    "       #.      "
    "               "
    "               "
    "               "
    "               "
    "               "
    "               "
    "               ",

    "`              "  /* Crosshair 2 (a standard cross) */
    "               "
    "               "
    "               "
    "      .#.      "
    "      .#.      "
    "    ...#...    "
    "    #######    "
    "    ...#...    "
    "      .#.      "
    "      .#.      "
    "               "
    "               "
    "               "
    "               ",

    "`     .#.      "  /* Crosshair 3 (a standard cross) */
    "      .#.      "
    "      .#.      "
    "      .#.      "
    "      .#.      "
    "      .#.      "
    ".......#......."
    "###############"
    ".......#......."
    "      .#.      "
    "      .#.      "
    "      .#.      "
    "      .#.      "
    "      .#.      "
    "      .#.      ",

    "`              "  /* Crosshair 4 (an X) */
    "               "
    "               "
    "    .     .    "
    "   .#.   .#.   "
    "    .#. .#.    "
    "     .#.#.     "
    "      .#.      "
    "     .#.#.     "
    "    .#. .#.    "
    "   .#.   .#.   "
    "    .     .    "
    "               "
    "               "
    "               ",

    "`.           . "  /* Crosshair 5 (an X) */
    ".#.         .#."
    " .#.       .#. "
    "  .#.     .#.  "
    "   .#.   .#.   "
    "    .#. .#.    "
    "     .#.#.     "
    "      .#.      "
    "     .#.#.     "
    "    .#. .#.    "
    "   .#.   .#.   "
    "  .#.     .#.  "
    " .#.       .#. "
    ".#.         .#."
    " .           . ",

    "`              "  /* Crosshair 6 (a combo) */
    "               "
    "               "
    "               "
    "    #  .  #    "
    "     # . #     "
    "      #.#      "
    "    ...#...    "
    "      #.#      "
    "     # . #     "
    "    #  .  #    "
    "               "
    "               "
    "               "
    "               ",

    "`      .       "  /* Crosshair 7 (a combo) */
    " #     .     # "
    "  #    .    #  "
    "   #   .   #   "
    "    #  .  #    "
    "     # . #     "
    "      #.#      "
    ".......#......."
    "      #.#      "
    "     # . #     "
    "    #  .  #    "
    "   #   .   #   "
    "  #    .    #  "
    " #     .     # "
    "       .       ",

    "`      #       "  /* Crosshair 8 (a diamond cross) */
    "      #.#      "
    "     # . #     "
    "    #  .  #    "
    "   #   .   #   "
    "  #    .    #  "
    " #     .     # "
    "#......#......#"
    " #     .     # "
    "  #    .    #  "
    "   #   .   #   "
    "    #  .  #    "
    "     # . #     "
    "      #.#      "
    "       #       ",

    "`     ###      "  /* Crosshair 9 (a circle cross) */
    "    ## . ##    "
    "   #   .   #   "
    "  #    .    #  "
    " #     .     # "
    " #     .     # "
    "#      .      #"
    "#......#......#"
    "#      .      #"
    " #     .     # "
    " #     .     # "
    "  #    .    #  "
    "   #   .   #   "
    "    ## . ##    "
    "      ###      ",

    "`     .#.      "  /* Crosshair 10 (a square cross) */
    "      .#.      "
    "      .#.      "
    "   ....#....   "
    "   .#######.   "
    "   .#     #.   "
    "....#     #...."
    "#####     #####"
    "....#     #...."
    "   .#     #.   "
    "   .#######.   "
    "   ....#....   "
    "      .#.      "
    "      .#.      "
    "      .#.      ",

    "`     .#.      "  /* Crosshair 11 (an interrupted cross) */
    "      .#.      "
    "      .#.      "
    "      .#.      "
    "      .#.      "
    "               "
    ".....     ....."
    "#####     #####"
    ".....     ....."
    "               "
    "      .#.      "
    "      .#.      "
    "      .#.      "
    "      .#.      "
    "      .#.      ",

    "`.           . "  /* Crosshair 12 (an interrupted X) */
    ".#.         .#."
    " .#.       .#. "
    "  .#.     .#.  "
    "   .#.   .#.   "
    "               "
    "               "
    "               "
    "               "
    "               "
    "   .#.   .#.   "
    "  .#.     .#.  "
    " .#.       .#. "
    ".#.         .#."
    " .           . ",

    "`      .       "  /* Crosshair 13 (an interrupted combo) */
    " #     .     # "
    "  #    .    #  "
    "   #   .   #   "
    "    #  .  #    "
    "               "
    "               "
    ".....     ....."
    "               "
    "               "
    "    #  .  #    "
    "   #   .   #   "
    "  #    .    #  "
    " #     .     # "
    "       .       ",

    "`####     #### "  /* Crosshair 14 */
    "#....     ....#"
    "#.           .#"
    "#.           .#"
    "#.           .#"
    "       #       "
    "       #       "
    "     #####     "
    "       #       "
    "       #       "
    "#.           .#"
    "#.           .#"
    "#.           .#"
    "#....     ....#"
    " ####     #### ",

    "`  .#     #.   "  /* Crosshair 15 */
    "   .#     #.   "
    "   .#     #.   "
    "....#     #...."
    "#####     #####"
    "               "
    "               "
    "               "
    "               "
    "               "
    "#####     #####"
    "....#     #...."
    "   .#     #.   "
    "   .#     #.   "
    "   .#     #.   ",

    "`      #       "  /* Crosshair 16 */
    "       #       "
    "       #       "
    "   ....#....   "
    "   .   #   .   "
    "   .   #   .   "
    "   .   #   .   "
    "###############"
    "   .   #   .   "
    "   .   #   .   "
    "   .   #   .   "
    "   ....#....   "
    "       #       "
    "       #       "
    "       #       ",

    NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
    NULL, NULL, NULL
};

bool S9xLoadCrosshairFile(int idx, const char *filename){
    int i, r;
    char *s;
    FILE *fp;

    if(idx<1 || idx>31) return false;
    if((s=(char *)calloc(15*15+1, sizeof(char)))==NULL){
        fprintf(stderr, "S9xLoadCrosshairFile: malloc error while reading ");
        perror(filename);
        return false;
    }

    if((fp=fopen(filename, "rb"))==NULL){
        fprintf(stderr, "S9xLoadCrosshairFile: Couldn't open ");
        perror(filename);
        free(s);
        return false;
    }

    i=fread(s, 1, 8, fp);
    if(i!=8){
        fprintf(stderr, "S9xLoadCrosshairFile: File is too short!\n");
        free(s);
        fclose(fp);
        return false;
    }
#ifdef HAVE_LIBPNG
    png_structp png_ptr;
    png_infop info_ptr;

    if(!png_sig_cmp((png_byte *)s, 0, 8)){
        png_ptr=png_create_read_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
        if (!png_ptr){
            free(s);
            fclose(fp);
            return false;
        }
        info_ptr=png_create_info_struct(png_ptr);
        if(!info_ptr){
            png_destroy_read_struct(&png_ptr, (png_infopp)NULL, (png_infopp)NULL);
            free(s);
            fclose(fp);
            return false;
        }

        png_init_io(png_ptr, fp);
        png_set_sig_bytes(png_ptr, 8);
        png_read_info(png_ptr, info_ptr);

        png_uint_32 width, height;
        int bit_depth, color_type;

        png_get_IHDR(png_ptr, info_ptr, &width, &height, &bit_depth, &color_type, NULL, NULL, NULL);
        if(color_type!=PNG_COLOR_TYPE_PALETTE){
            fprintf(stderr, "S9xLoadCrosshairFile: Input PNG is not a palettized image!\n");
            png_destroy_read_struct(&png_ptr, &info_ptr, (png_infopp)NULL);
            free(s);
            fclose(fp);
            return false;
        }
        if(bit_depth==16)
            png_set_strip_16(png_ptr);

        if(width!=15 || height!=15){
            fprintf(stderr, "S9xLoadCrosshairFile: Expecting a 15x15 PNG\n");
            png_destroy_read_struct(&png_ptr, &info_ptr, (png_infopp)NULL);
            free(s);
            fclose(fp);
            return false;
        }

        int num_palette=0, num_trans=0;
        int fgcol=-1, bgcol=-1, transcol=-1;
        png_color *pngpal;
        png_byte *trans;

        png_get_PLTE(png_ptr, info_ptr, &pngpal, &num_palette);
        png_get_tRNS(png_ptr, info_ptr, &trans, &num_trans, NULL);

        if(num_palette!=3 || num_trans!=1){
            fprintf(stderr, "S9xLoadCrosshairFile: Expecting a 3-color PNG with 1 trasnparent color\n");
            png_destroy_read_struct(&png_ptr, &info_ptr, (png_infopp)NULL);
            free(s);
            fclose(fp);
            return false;
        }

        for(i=0; i<3; i++){
            if(trans[0]==i){
                transcol=i;
            } else if(pngpal[i].red==0 && pngpal[i].green==0 && pngpal[i].blue==0){
                bgcol=i;
            } else if(pngpal[i].red==255 && pngpal[i].green==255 && pngpal[i].blue==255){
                fgcol=i;
            }
        }

        if(transcol<0 || fgcol<0 || bgcol<0){
            fprintf(stderr, "S9xLoadCrosshairFile: PNG must have 3 colors: white (fg), black (bg), and transparent.\n");
            png_destroy_read_struct(&png_ptr, &info_ptr, (png_infopp)NULL);
            free(s);
            fclose(fp);
            return false;
        }

        png_set_packing(png_ptr);
        png_read_update_info(png_ptr, info_ptr);
        png_byte *row_pointer = new png_byte [png_get_rowbytes(png_ptr, info_ptr)];

        for(r=0; r<15*15; r+=15){
            png_read_row(png_ptr, row_pointer, NULL);
            for(i=0; i<15; i++){
                if(row_pointer[i]==transcol) s[r+i]=' ';
                else if(row_pointer[i]==fgcol) s[r+i]='#';
                else if(row_pointer[i]==bgcol) s[r+i]='.';
                else {
                    fprintf(stderr, "S9xLoadCrosshairFile: WTF? This was supposed to be a 3-color PNG!\n");
                    png_destroy_read_struct(&png_ptr, &info_ptr, (png_infopp)NULL);
                    free(s);
                    fclose(fp);
                    return false;
                }
            }
        }
        s[15*15]=0;
        png_destroy_read_struct(&png_ptr, &info_ptr, (png_infopp)NULL);
    } else
#endif
    {
        i=fread(s+8, 1, 15-8, fp);
        if(i!=15-8){
            fprintf(stderr, "S9xLoadCrosshairFile: File is too short!\n");
            free(s);
            fclose(fp);
            return false;
        }
        if(getc(fp)!='\n'){
            fprintf(stderr, "S9xLoadCrosshairFile: Invalid file format!");
#ifndef HAVE_LIBPNG
            fprintf(stderr, " (note: PNG support is not available)");
#endif
            fprintf(stderr, "\n");
            free(s);
            fclose(fp);
            return false;
        }
        for(r=1; r<15; r++){
            i=fread(s+r*15, 1, 15, fp);
            if(i!=15){
                fprintf(stderr, "S9xLoadCrosshairFile: File is too short!");
#ifndef HAVE_LIBPNG
                fprintf(stderr, " (note: PNG support is not available)");
#endif
                fprintf(stderr, "\n");
                free(s);
                fclose(fp);
                return false;
            }
            if(getc(fp)!='\n'){
                fprintf(stderr, "S9xLoadCrosshairFile: Invalid file format!");
#ifndef HAVE_LIBPNG
                fprintf(stderr, " (note: PNG support is not available)");
#endif
                fprintf(stderr, "\n");
                free(s);
                fclose(fp);
                return false;
            }
        }
        for(i=0; i<15*15; i++){
            if(s[i]!=' ' && s[i]!='#' && s[i]!='.'){
                fprintf(stderr, "S9xLoadCrosshairFile: Invalid file format!");
#ifndef HAVE_LIBPNG
                fprintf(stderr, " (note: PNG support is not available)");
#endif
                fprintf(stderr, "\n");
                free(s);
                fclose(fp);
                return false;
            }
        }
    }
    fclose(fp);

    if(crosshairs[idx]!=NULL && crosshairs[idx][0]!='`') free(crosshairs[idx]);
    crosshairs[idx]=s;
    return true;
}

const char *S9xGetCrosshair(int idx){
    if(idx<0 || idx>31) return NULL;
    return crosshairs[idx];
}
