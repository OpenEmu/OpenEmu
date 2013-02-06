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

#ifdef __MSC__
#pragma warning(disable: 4786)
#endif

/* NOTE: The codes are not optimized. They can be made faster. */

#include "TxQuantize.h"
#include <boost/thread.hpp>
#include <boost/bind.hpp>

TxQuantize::TxQuantize()
{
  _txUtil = new TxUtil();

  /* get number of CPU cores. */
  _numcore = _txUtil->getNumberofProcessors();

  /* get dxtn extensions */
  _tx_compress_fxt1 = TxLoadLib::getInstance()->getfxtCompressTexFuncExt();
  _tx_compress_dxtn = TxLoadLib::getInstance()->getdxtCompressTexFuncExt();
}


TxQuantize::~TxQuantize()
{
  delete _txUtil;
}

void
TxQuantize::ARGB1555_ARGB8888(uint32* src, uint32* dest, int width, int height)
{
#if 1
  int siz = (width * height) >> 1;
  int i;
  for (i = 0; i < siz; i++) {
    *dest = (((*src & 0x00008000) ? 0xff000000 : 0x00000000) |
            ((*src & 0x00007c00) << 9) | ((*src & 0x00007000) << 4) |
            ((*src & 0x000003e0) << 6) | ((*src & 0x00000380) << 1) |
            ((*src & 0x0000001f) << 3) | ((*src & 0x0000001c) >> 2));
    dest++;
    *dest = (((*src & 0x80000000) ? 0xff000000 : 0x00000000) |
            ((*src & 0x7c000000) >>  7) | ((*src & 0x70000000) >> 12) |
            ((*src & 0x03e00000) >> 10) | ((*src & 0x03800000) >> 15) |
            ((*src & 0x001f0000) >> 13) | ((*src & 0x001c0000) >> 18));
    dest++;
    src++;
  }
#else
  int siz = (width * height) >> 1;

  __asm {
    push ebx;
    push esi;
    push edi;

    mov esi, dword ptr [src];
    mov edi, dword ptr [dest];
    mov ecx, dword ptr [siz];

  tc1_loop:
    mov eax, dword ptr [esi];
    add esi, 4;

    // arrr rrgg gggb bbbb
    // aaaaaaaa rrrrrrrr gggggggg bbbbbbbb
    mov edx, eax;         // edx = arrrrrgg gggbbbbb arrrrrgg gggbbbbb
    mov ebx, 0x00000000;
    and eax, 0x00008000;  // eax = 00000000 00000000 a0000000 00000000
    jz  transparent1;
    mov ebx, 0xff000000;  // ebx = aaaaaaaa 00000000 00000000 00000000

  transparent1:
    mov eax, edx;         // eax = arrrrrgg gggbbbbb arrrrrgg gggbbbbb
    and edx, 0x00007c00;  // edx = 00000000 00000000 0rrrrr00 00000000
    shl edx, 4;           // edx = 00000000 00000rrr rr000000 00000000
    or  ebx, edx;         // ebx = aaaaaaaa 00000rrr rr000000 00000000
    shl edx, 5;           // edx = 00000000 rrrrr000 00000000 00000000
    or  ebx, edx;         // ebx = aaaaaaaa rrrrrrrr rr000000 00000000
    and ebx, 0xffff0000;  // ebx = aaaaaaaa rrrrrrrr 00000000 00000000
    mov edx, eax;
    and edx, 0x000003e0;  // edx = 00000000 00000000 000000gg ggg00000
    shl edx, 1;           // edx = 00000000 00000000 00000ggg gg000000
    or  ebx, edx;         // ebx = aaaaaaaa rrrrrrrr 00000ggg gg000000
    shl edx, 5;           // edx = 00000000 00000000 ggggg000 00000000
    or  ebx, edx;         // ebx = aaaaaaaa rrrrrrrr gggggggg gg000000
    and ebx, 0xffffff00;  // ebx = aaaaaaaa rrrrrrrr gggggggg 00000000
    mov edx, eax;
    and edx, 0x0000001f;  // edx = 00000000 00000000 00000000 000bbbbb
    shl edx, 3;           // edx = 00000000 00000000 00000000 bbbbb000
    or  ebx, edx;         // ebx = aaaaaaaa rrrrrrrr gggggggg bbbbb000
    shr edx, 5;           // edx = 00000000 00000000 00000000 00000bbb
    or  ebx, edx;         // ebx = aaaaaaaa rrrrrrrr gggggggg bbbbbbbb

    mov dword ptr [edi], ebx;
    add edi, 4;

    shr eax, 16;          // eax = 00000000 00000000 arrrrrgg gggbbbbb
    mov edx, eax;         // edx = 00000000 00000000 arrrrrgg gggbbbbb
    mov ebx, 0x00000000;
    and eax, 0x00008000;  // eax = 00000000 00000000 a0000000 00000000
    jz  transparent2;
    mov ebx, 0xff000000;  // ebx = aaaaaaaa 00000000 00000000 00000000

  transparent2:
    mov eax, edx;         // eax = 00000000 00000000 arrrrrgg gggbbbbb
    and edx, 0x00007c00;  // edx = 00000000 00000000 0rrrrr00 00000000
    shl edx, 4;           // edx = 00000000 00000rrr rr000000 00000000
    or  ebx, edx;         // ebx = aaaaaaaa 00000rrr rr000000 00000000
    shl edx, 5;           // edx = 00000000 rrrrr000 00000000 00000000
    or  ebx, edx;         // ebx = aaaaaaaa rrrrrrrr rr000000 00000000
    and ebx, 0xffff0000;  // ebx = aaaaaaaa rrrrrrrr 00000000 00000000
    mov edx, eax;
    and edx, 0x000003e0;  // edx = 00000000 00000000 000000gg ggg00000
    shl edx, 1;           // edx = 00000000 00000000 00000ggg gg000000
    or  ebx, edx;         // ebx = aaaaaaaa rrrrrrrr 00000ggg gg000000
    shl edx, 5;           // edx = 00000000 00000000 ggggg000 00000000
    or  ebx, edx;         // ebx = aaaaaaaa rrrrrrrr gggggggg gg000000
    and ebx, 0xffffff00;  // ebx = aaaaaaaa rrrrrrrr gggggggg 00000000
    mov edx, eax;
    and edx, 0x0000001f;  // edx = 00000000 00000000 00000000 000bbbbb
    shl edx, 3;           // edx = 00000000 00000000 00000000 bbbbb000
    or  ebx, edx;         // ebx = aaaaaaaa rrrrrrrr gggggggg bbbbb000
    shr edx, 5;           // edx = 00000000 00000000 00000000 00000bbb
    or  ebx, edx;         // ebx = aaaaaaaa rrrrrrrr gggggggg bbbbbbbb

    mov dword ptr [edi], ebx;
    add edi, 4;

    dec ecx;
    jnz tc1_loop;

    pop edi;
    pop esi;
    pop ebx;
  }
#endif
}

void
TxQuantize::ARGB4444_ARGB8888(uint32* src, uint32* dest, int width, int height)
{
#if 1
  int siz = (width * height) >> 1;
  int i;
  for (i = 0; i < siz; i++) {
    *dest = ((*src & 0x0000f000) << 12) |
            ((*src & 0x00000f00) << 8) |
            ((*src & 0x000000f0) << 4) |
             (*src & 0x0000000f);
    *dest |= (*dest << 4);
    dest++;
    *dest = ((*src & 0xf0000000) |
            ((*src & 0x0f000000) >> 4) |
            ((*src & 0x00f00000) >> 8) |
            ((*src & 0x000f0000) >> 12));
    *dest |= (*dest >> 4);
    dest++;
    src++;
  }
#else
  int siz = (width * height) >> 1;

  __asm {
    push ebx;
    push esi;
    push edi;

    mov esi, dword ptr [src];
    mov edi, dword ptr [dest];
    mov ecx, dword ptr [siz];

  tc1_loop:
    mov eax, dword ptr [esi];
    add esi, 4;

    // aaaa rrrr gggg bbbb
    // aaaaaaaa rrrrrrrr gggggggg bbbbbbbb
    mov edx, eax;
    and eax, 0x0000ffff;
    mov ebx, eax;        // 00000000 00000000 aaaarrrr ggggbbbb
    and ebx, 0x0000f000; // 00000000 00000000 aaaa0000 00000000
    shl ebx, 12;         // 0000aaaa 00000000 00000000 00000000
    or  eax, ebx;        // 0000aaaa 00000000 aaaarrrr ggggbbbb
    mov ebx, eax;
    and ebx, 0x00000f00; // 00000000 00000000 0000rrrr 00000000
    shl ebx, 8;          // 00000000 0000rrrr 00000000 00000000
    or  eax, ebx;        // 0000aaaa 0000rrrr aaaarrrr ggggbbbb
    mov ebx, eax;
    and ebx, 0x000000f0; // 00000000 00000000 00000000 gggg0000
    shl ebx, 4;          // 00000000 00000000 0000gggg 00000000
    and eax, 0x0f0f000f; // 0000aaaa 0000rrrr 00000000 0000bbbb
    or  eax, ebx;        // 0000aaaa 0000rrrr 0000gggg 0000bbbb
    mov ebx, eax;
    shl ebx, 4;          // aaaa0000 rrrr0000 gggg0000 bbbb0000
    or  eax, ebx;        // aaaaaaaa rrrrrrrr gggggggg bbbbbbbb

    mov dword ptr [edi], eax;
    add edi, 4;

    shr edx, 16;
    mov ebx, edx;        // 00000000 00000000 aaaarrrr ggggbbbb
    and ebx, 0x0000f000; // 00000000 00000000 aaaa0000 00000000
    shl ebx, 12;         // 0000aaaa 00000000 00000000 00000000
    or  edx, ebx;        // 0000aaaa 00000000 aaaarrrr ggggbbbb
    mov ebx, edx;
    and ebx, 0x00000f00; // 00000000 00000000 0000rrrr 00000000
    shl ebx, 8;          // 00000000 0000rrrr 00000000 00000000
    or  edx, ebx;        // 0000aaaa 0000rrrr aaaarrrr ggggbbbb
    mov ebx, edx;
    and ebx, 0x000000f0; // 00000000 00000000 00000000 gggg0000
    shl ebx, 4;          // 00000000 00000000 0000gggg 00000000
    and edx, 0x0f0f000f; // 0000aaaa 0000rrrr 00000000 0000bbbb
    or  edx, ebx;        // 0000aaaa 0000rrrr 0000gggg 0000bbbb
    mov ebx, edx;
    shl ebx, 4;          // aaaa0000 rrrr0000 gggg0000 bbbb0000
    or  edx, ebx;        // aaaaaaaa rrrrrrrr gggggggg bbbbbbbb

    mov dword ptr [edi], edx;
    add edi, 4;

    dec ecx;
    jnz tc1_loop;

    pop edi;
    pop esi;
    pop ebx;
  }
#endif
}

void
TxQuantize::RGB565_ARGB8888(uint32* src, uint32* dest, int width, int height)
{
#if 1
  int siz = (width * height) >> 1;
  int i;
  for (i = 0; i < siz; i++) {
    *dest = (0xff000000 |
            ((*src & 0x0000f800) << 8) | ((*src & 0x0000e000) << 3) |
            ((*src & 0x000007e0) << 5) | ((*src & 0x00000600) >> 1) |
            ((*src & 0x0000001f) << 3) | ((*src & 0x0000001c) >> 2));
    dest++;
    *dest = (0xff000000 |
            ((*src & 0xf8000000) >>  8) | ((*src & 0xe0000000) >> 13) |
            ((*src & 0x07e00000) >> 11) | ((*src & 0x06000000) >> 17) |
            ((*src & 0x001f0000) >> 13) | ((*src & 0x001c0000) >> 18));
    dest++;
    src++;
  }
#else
  int siz = (width * height) >> 1;

  __asm {
    push ebx;
    push esi;
    push edi;

    mov esi, dword ptr [src];
    mov edi, dword ptr [dest];
    mov ecx, dword ptr [siz];

  tc1_loop:
    mov eax, dword ptr [esi];
    add esi, 4;

    // rrrr rggg gggb bbbb
    // 11111111 rrrrrrrr gggggggg bbbbbbbb
    mov edx, eax;
    and eax, 0x0000ffff;
    mov ebx, eax;        // 00000000 00000000 rrrrrggg gggbbbbb
    and ebx, 0x0000f800; // 00000000 00000000 rrrrr000 00000000
    shl ebx, 5;          // 00000000 000rrrrr 00000000 00000000
    or  eax, ebx;        // 00000000 000rrrrr rrrrrggg gggbbbbb
    mov ebx, eax;
    and ebx, 0x000007e0; // 00000000 00000000 00000ggg ggg00000
    shl ebx, 5;          // 00000000 00000000 gggggg00 00000000
    and eax, 0x001F001F; // 00000000 000rrrrr 00000000 000bbbbb
    shl eax, 3;          // 00000000 rrrrr000 00000000 bbbbb000
    or  eax, ebx;        // 00000000 rrrrr000 gggggg00 bbbbb000
    mov ebx, eax;
    shr ebx, 5;          // 00000000 00000rrr rr000ggg ggg00bbb
    and ebx, 0x00070007; // 00000000 00000rrr 00000000 00000bbb
    or  eax, ebx;        // 00000000 rrrrrrrr gggggg00 bbbbbbbb
    mov ebx, eax;
    shr ebx, 6;
    and ebx, 0x00000300; // 00000000 00000000 000000gg 00000000
    or  eax, ebx         // 00000000 rrrrrrrr gggggggg bbbbbbbb
    or  eax, 0xff000000; // 11111111 rrrrrrrr gggggggg bbbbbbbb

    mov dword ptr [edi], eax;
    add edi, 4;

    shr edx, 16;
    mov eax, edx;        // 00000000 00000000 rrrrrggg gggbbbbb
    and eax, 0x0000ffff;
    mov ebx, eax;        // 00000000 00000000 rrrrrggg gggbbbbb
    and ebx, 0x0000f800; // 00000000 00000000 rrrrr000 00000000
    shl ebx, 5;          // 00000000 000rrrrr 00000000 00000000
    or  eax, ebx;        // 00000000 000rrrrr rrrrrggg gggbbbbb
    mov ebx, eax;
    and ebx, 0x000007e0; // 00000000 00000000 00000ggg ggg00000
    shl ebx, 5;          // 00000000 00000000 gggggg00 00000000
    and eax, 0x001F001F; // 00000000 000rrrrr 00000000 000bbbbb
    shl eax, 3;          // 00000000 rrrrr000 00000000 bbbbb000
    or  eax, ebx;        // 00000000 rrrrr000 gggggg00 bbbbb000
    mov ebx, eax;
    shr ebx, 5;          // 00000000 00000rrr rr000ggg ggg00bbb
    and ebx, 0x00070007; // 00000000 00000rrr 00000000 00000bbb
    or  eax, ebx;        // 00000000 rrrrrrrr gggggg00 bbbbbbbb
    mov ebx, eax;
    shr ebx, 6;
    and ebx, 0x00000300; // 00000000 00000000 000000gg 00000000
    or  eax, ebx         // 00000000 rrrrrrrr gggggggg bbbbbbbb
    or  eax, 0xff000000; // 11111111 rrrrrrrr gggggggg bbbbbbbb

    mov dword ptr [edi], eax;
    add edi, 4;

    dec ecx;
    jnz tc1_loop;

    pop edi;
    pop esi;
    pop ebx;
  }
#endif
}

void
TxQuantize::A8_ARGB8888(uint32* src, uint32* dest, int width, int height)
{
#if 1
  int siz = (width * height) >> 2;
  int i;
  for (i = 0; i < siz; i++) {
    *dest = (*src & 0x000000ff);
    *dest |= (*dest << 8);
    *dest |= (*dest << 16);
    dest++;
    *dest = (*src & 0x0000ff00);
    *dest |= (*dest >> 8);
    *dest |= (*dest << 16);
    dest++;
    *dest = (*src & 0x00ff0000);
    *dest |= (*dest << 8);
    *dest |= (*dest >> 16);
    dest++;
    *dest = (*src & 0xff000000);
    *dest |= (*dest >> 8);
    *dest |= (*dest >> 16);
    dest++;
    src++;
  }
#else
  int siz = (width * height) >> 2;

  __asm {
    push ebx;
    push esi;
    push edi;

    mov esi, dword ptr [src];
    mov edi, dword ptr [dest];
    mov ecx, dword ptr [siz];

  tc1_loop:
    mov eax, dword ptr [esi];
    add esi, 4;

    // aaaaaaaa
    // aaaaaaaa rrrrrrrr gggggggg bbbbbbbb
    mov edx, eax;
    and eax, 0x000000ff;
    mov ebx, eax;        // 00000000 00000000 00000000 aaaaaaaa
    shl ebx, 8;          // 00000000 00000000 aaaaaaaa 00000000
    or  eax, ebx;        // 00000000 00000000 aaaaaaaa aaaaaaaa
    mov ebx, eax;
    shl ebx, 16;         // aaaaaaaa aaaaaaaa 00000000 00000000
    or  eax, ebx;        // aaaaaaaa rrrrrrrr gggggggg bbbbbbbb

    mov dword ptr [edi], eax;
    add edi, 4;

    mov eax, edx;
    and eax, 0x0000ff00;
    mov ebx, eax;        // 00000000 00000000 aaaaaaaa 00000000
    shr ebx, 8;          // 00000000 00000000 00000000 aaaaaaaa
    or  eax, ebx;        // 00000000 00000000 aaaaaaaa aaaaaaaa
    mov ebx, eax;
    shl ebx, 16;         // aaaaaaaa aaaaaaaa 00000000 00000000
    or  eax, ebx;        // aaaaaaaa rrrrrrrr gggggggg bbbbbbbb

    mov dword ptr [edi], eax;
    add edi, 4;

    mov eax, edx;
    and eax, 0x00ff0000;
    mov ebx, eax;        // 00000000 aaaaaaaa 00000000 00000000
    shl ebx, 8;          // aaaaaaaa 00000000 00000000 00000000
    or  eax, ebx;        // aaaaaaaa aaaaaaaa 00000000 00000000
    mov ebx, eax;
    shr ebx, 16;         // 00000000 00000000 aaaaaaaa aaaaaaaa
    or  eax, ebx;        // aaaaaaaa rrrrrrrr gggggggg bbbbbbbb

    mov dword ptr [edi], eax;
    add edi, 4;

    mov eax, edx;
    and eax, 0xff000000;
    mov ebx, eax;        // aaaaaaaa 00000000 00000000 00000000
    shr ebx, 8;          // 00000000 aaaaaaaa 00000000 00000000
    or  eax, ebx;        // aaaaaaaa aaaaaaaa 00000000 00000000
    mov ebx, eax;
    shr ebx, 16;         // 00000000 00000000 aaaaaaaa aaaaaaaa
    or  eax, ebx;        // aaaaaaaa rrrrrrrr gggggggg bbbbbbbb

    mov dword ptr [edi], eax;
    add edi, 4;

    dec ecx;
    jnz tc1_loop;

    pop edi;
    pop esi;
    pop ebx;
  }
#endif
}

void
TxQuantize::AI44_ARGB8888(uint32* src, uint32* dest, int width, int height)
{
#if 1
  int siz = (width * height) >> 2;
  int i;
  for (i = 0; i < siz; i++) {
    *dest = (*src & 0x0000000f);
    *dest |= ((*dest << 8) | (*dest << 16));
    *dest |= ((*src & 0x000000f0) << 20);
    *dest |= (*dest << 4);
    dest++;
    *dest = (*src & 0x00000f00);
    *dest |= ((*dest << 8) | (*dest >> 8));
    *dest |= ((*src & 0x0000f000) << 12);
    *dest |= (*dest << 4);
    dest++;
    *dest = (*src & 0x000f0000);
    *dest |= ((*dest >> 8) | (*dest >> 16));
    *dest |= ((*src & 0x00f00000) << 4);
    *dest |= (*dest << 4);
    dest++;
    *dest = ((*src & 0x0f000000) >> 4);
    *dest |= ((*dest >> 8) | (*dest >> 16));
    *dest |= (*src & 0xf0000000);
    *dest |= (*dest >> 4);
    dest++;
    src++;
  }
#else
  int siz = (width * height) >> 2;

  __asm {
    push ebx;
    push esi;
    push edi;

    mov esi, dword ptr [src];
    mov edi, dword ptr [dest];
    mov ecx, dword ptr [siz];

  tc1_loop:
    mov eax, dword ptr [esi];
    add esi, 4;

    // aaaaiiii
    // aaaaaaaa iiiiiiii iiiiiiii iiiiiiii
    mov edx, eax;
    and eax, 0x000000f0; // 00000000 00000000 00000000 aaaa0000
    mov ebx, edx;
    shl eax, 20;         // 0000aaaa 00000000 00000000 00000000
    and ebx, 0x0000000f; // 00000000 00000000 00000000 0000iiii
    or  eax, ebx;        // 0000aaaa 00000000 00000000 0000iiii
    shl ebx, 8;          // 00000000 00000000 0000iiii 00000000
    or  eax, ebx;        // 0000aaaa 00000000 0000iiii 0000iiii
    shl ebx, 8;          // 00000000 0000iiii 00000000 00000000
    or  eax, ebx;        // 0000aaaa 0000iiii 0000iiii 0000iiii
    mov ebx, eax;
    shl ebx, 4;          // aaaa0000 iiii0000 iiii0000 iiii0000
    or  eax, ebx;        // aaaaaaaa iiiiiiii iiiiiiii iiiiiiii

    mov dword ptr [edi], eax;
    add edi, 4;

    mov eax, edx;
    and eax, 0x0000f000; // 00000000 00000000 aaaa0000 00000000
    mov ebx, edx;
    shl eax, 12;         // 0000aaaa 00000000 00000000 00000000
    and ebx, 0x00000f00; // 00000000 00000000 0000iiii 00000000
    or  eax, ebx;        // 0000aaaa 00000000 0000iiii 00000000
    shr ebx, 8;          // 00000000 00000000 00000000 0000iiii
    or  eax, ebx;        // 0000aaaa 00000000 0000iiii 0000iiii
    shl ebx, 16;         // 00000000 0000iiii 00000000 00000000
    or  eax, ebx;        // 0000aaaa 0000iiii 0000iiii 0000iiii
    mov ebx, eax;
    shl ebx, 4;          // aaaa0000 iiii0000 iiii0000 iiii0000
    or  eax, ebx;        // aaaaaaaa iiiiiiii iiiiiiii iiiiiiii

    mov dword ptr [edi], eax;
    add edi, 4;

    mov eax, edx;
    and eax, 0x00f00000; // 00000000 aaaa0000 00000000 00000000
    mov ebx, edx;
    shl eax, 4;          // 0000aaaa 00000000 00000000 00000000
    and ebx, 0x000f0000; // 00000000 0000iiii 00000000 00000000
    or  eax, ebx;        // 0000aaaa 0000iiii 00000000 00000000
    shr ebx, 8;          // 00000000 00000000 0000iiii 00000000
    or  eax, ebx;        // 0000aaaa 0000iiii 0000iiii 00000000
    shr ebx, 8;          // 00000000 00000000 00000000 0000iiii
    or  eax, ebx;        // 0000aaaa 0000iiii 0000iiii 0000iiii
    mov ebx, eax;
    shl ebx, 4;          // aaaa0000 iiii0000 iiii0000 iiii0000
    or  eax, ebx;        // aaaaaaaa iiiiiiii iiiiiiii iiiiiiii

    mov dword ptr [edi], eax;
    add edi, 4;

    mov eax, edx;
    and eax, 0xf0000000; // aaaa0000 00000000 00000000 00000000
    mov ebx, edx;
    and ebx, 0x0f000000; // 0000iiii 00000000 00000000 00000000
    shr ebx, 4;          // 00000000 iiii0000 00000000 00000000
    or  eax, ebx;        // aaaa0000 iiii0000 00000000 00000000
    shr ebx, 8;          // 00000000 00000000 iiii0000 00000000
    or  eax, ebx;        // aaaa0000 iiii0000 iiii0000 00000000
    shr ebx, 8;          // 00000000 00000000 00000000 iiii0000
    or  eax, ebx;        // aaaa0000 iiii0000 iiii0000 iiii0000
    mov ebx, eax;
    shr ebx, 4;          // 0000aaaa 0000iiii 0000iiii 0000iiii
    or  eax, ebx;        // aaaaaaaa iiiiiiii iiiiiiii iiiiiiii

    mov dword ptr [edi], eax;
    add edi, 4;

    dec ecx;
    jnz tc1_loop;

    pop edi;
    pop esi;
    pop ebx;
  }
#endif
}

void
TxQuantize::AI88_ARGB8888(uint32* src, uint32* dest, int width, int height)
{
#if 1
  int siz = (width * height) >> 1;
  int i;
  for (i = 0; i < siz; i++) {
    *dest = (*src & 0x000000ff);
    *dest |= ((*dest << 8) | (*dest << 16));
    *dest |= ((*src & 0x0000ff00) << 16);
    dest++;
    *dest = (*src & 0x00ff0000);
    *dest |= ((*dest >> 8) | (*dest >> 16));
    *dest |= (*src & 0xff000000);
    dest++;
    src++;
  }
#else
  int siz = (width * height) >> 1;

  __asm {
    push ebx;
    push esi;
    push edi;

    mov esi, dword ptr [src];
    mov edi, dword ptr [dest];
    mov ecx, dword ptr [siz];

  tc1_loop:
    mov eax, dword ptr [esi];
    add esi, 4;

    // aaaaaaaa iiiiiiii
    // aaaaaaaa iiiiiiii iiiiiiii iiiiiiii
    mov edx, eax;
    and eax, 0x0000ffff; // 00000000 00000000 aaaaaaaa iiiiiiii
    mov ebx, eax;        // 00000000 00000000 aaaaaaaa iiiiiiii
    shl eax, 16;         // aaaaaaaa iiiiiiii 00000000 00000000
    and ebx, 0x000000ff; // 00000000 00000000 00000000 iiiiiiii
    or  eax, ebx;        // aaaaaaaa iiiiiiii 00000000 iiiiiiii
    shl ebx, 8;          // 00000000 00000000 iiiiiiii 00000000
    or  eax, ebx;        // aaaaaaaa iiiiiiii iiiiiiii iiiiiiii

    mov dword ptr [edi], eax;
    add edi, 4;

    mov eax, edx;
    and eax, 0xffff0000; // aaaaaaaa iiiiiiii 00000000 00000000
    mov ebx, eax;        // aaaaaaaa iiiiiiii 00000000 00000000
    and ebx, 0x00ff0000; // 00000000 iiiiiiii 00000000 00000000
    shr ebx, 8;          // 00000000 00000000 iiiiiiii 00000000
    or  eax, ebx;        // aaaaaaaa iiiiiiii iiiiiiii 00000000
    shr ebx, 8;          // 00000000 00000000 00000000 iiiiiiii
    or  eax, ebx;        // aaaaaaaa iiiiiiii iiiiiiii iiiiiiii

    mov dword ptr [edi], eax;
    add edi, 4;

    dec ecx;
    jnz tc1_loop;

    pop edi;
    pop esi;
    pop ebx;
  }
#endif
}

void
TxQuantize::ARGB8888_ARGB1555(uint32* src, uint32* dest, int width, int height)
{
#if 1
  int siz = (width * height) >> 1;
  int i;
  for (i = 0; i < siz; i++) {
    *dest = ((*src & 0xff000000) ? 0x00008000 : 0x00000000);
    *dest |= (((*src & 0x00f80000) >> 9) |
              ((*src & 0x0000f800) >> 6) |
              ((*src & 0x000000f8) >> 3));
    src++;
    *dest |= ((*src & 0xff000000) ? 0x80000000 : 0x00000000);
    *dest |= (((*src & 0x00f80000) << 7) |
              ((*src & 0x0000f800) << 10) |
              ((*src & 0x000000f8) << 13));
    src++;
    dest++;
  }
#else
  int siz = (width * height) >> 1;

  __asm {
    push ebx;
    push esi;
    push edi;

    mov esi, dword ptr [src];
    mov edi, dword ptr [dest];
    mov ecx, dword ptr [siz];

  tc1_loop:
    mov eax, dword ptr [esi];
    add esi, 4;

#if 1
    mov edx, eax;
    and eax, 0xff000000;  // aaaa0000 00000000 00000000 00000000
    jz transparent1;
    mov eax, 0x00008000;  // 00000000 00000000 a0000000 00000000

  transparent1:
    mov ebx, edx;
    and ebx, 0x00f80000;  // 00000000 rrrrr000 00000000 00000000
    shr ebx, 9;           // 00000000 00000000 0rrrrr00 00000000
    or  eax, ebx;         // 00000000 00000000 arrrrr00 00000000
    mov ebx, edx;
    and ebx, 0x0000f800;  // 00000000 00000000 ggggg000 00000000
    shr ebx, 6;           // 00000000 00000000 000000gg ggg00000
    or  eax, ebx;         // 00000000 00000000 arrrrrgg ggg00000
    and edx, 0x000000f8;  // 00000000 00000000 00000000 bbbbb000
    shr edx, 3;           // 00000000 00000000 00000000 000bbbbb
    or  edx, eax;         // 00000000 00000000 arrrrrgg gggbbbbb

    mov eax, dword ptr [esi];
    add esi, 4;

    mov ebx, eax;
    and eax, 0xff000000;  // aaaa0000 00000000 00000000 00000000
    jz transparent2;
    or  edx, 0x80000000;  // a0000000 00000000 arrrrrgg gggbbbbb

  transparent2:
    mov eax, ebx;
    and ebx, 0x00f80000;  // 00000000 rrrrr000 00000000 00000000
    shl ebx, 7;           // 0rrrrr00 00000000 00000000 00000000
    or  edx, ebx;         // arrrrr00 00000000 arrrrrgg gggbbbbb
    mov ebx, eax;
    and ebx, 0x0000f800;  // 00000000 00000000 ggggg000 00000000
    shl ebx, 10;          // 000000gg ggg00000 00000000 00000000
    or  edx, ebx;         // arrrrrgg ggg00000 arrrrrgg gggbbbbb
    and eax, 0x000000f8;  // 00000000 00000000 00000000 bbbbb000
    shl eax, 13;          // 00000000 000bbbbb 00000000 00000000
    or  edx, eax;         // arrrrrgg gggbbbbb arrrrrgg gggbbbbb

    mov dword ptr [edi], edx;
    add edi, 4;
#else
    mov edx, eax;
    and edx, 0x01000000;  // 0000000a 00000000 00000000 00000000
    shr edx, 9;           // 00000000 00000000 a0000000 00000000
    mov ebx, eax;
    and ebx, 0x00f80000;  // 00000000 rrrrr000 00000000 00000000
    shr ebx, 9;           // 00000000 00000000 0rrrrr00 00000000
    or  edx, ebx;         // 00000000 00000000 arrrrr00 00000000
    mov ebx, eax;
    and ebx, 0x0000f800;  // 00000000 00000000 ggggg000 00000000
    shr ebx, 6;           // 00000000 00000000 000000gg ggg00000
    or  edx, ebx;         // 00000000 00000000 arrrrrgg ggg00000
    and eax, 0x000000f8;  // 00000000 00000000 00000000 bbbbb000
    shr eax, 3;           // 00000000 00000000 00000000 000bbbbb
    or  edx, eax;         // 00000000 00000000 arrrrrgg gggbbbbb

    mov eax, dword ptr [esi];
    add esi, 4;

    mov ebx, eax;
    and ebx, 0x80000000;  // a0000000 00000000 00000000 00000000
    or  edx, ebx;         // a0000000 00000000 arrrrrgg gggbbbbb
    mov ebx, eax;
    and ebx, 0x00f80000;  // 00000000 rrrrr000 00000000 00000000
    shl ebx, 7;           // 0rrrrr00 00000000 00000000 00000000
    or  edx, ebx;         // arrrrr00 00000000 arrrrrgg gggbbbbb
    mov ebx, eax;
    and ebx, 0x0000f800;  // 00000000 00000000 ggggg000 00000000
    shl ebx, 10;          // 000000gg ggg00000 00000000 00000000
    or  edx, ebx;         // arrrrrgg ggg00000 arrrrrgg gggbbbbb
    and eax, 0x000000f8;  // 00000000 00000000 00000000 bbbbb000
    shl eax, 13;          // 00000000 000bbbbb 00000000 00000000
    or  edx, eax;         // arrrrrgg gggbbbbb arrrrrgg gggbbbbb

    mov dword ptr [edi], edx;
    add edi, 4;
#endif
    dec ecx;
    jnz tc1_loop;

    pop edi;
    pop esi;
    pop ebx;
  }
#endif
}

void
TxQuantize::ARGB8888_ARGB4444(uint32* src, uint32* dest, int width, int height)
{
#if 1
  int siz = (width * height) >> 1;
  int i;
  for (i = 0; i < siz; i++) {
    *dest = (((*src & 0xf0000000) >> 16) |
             ((*src & 0x00f00000) >> 12) |
             ((*src & 0x0000f000) >> 8) |
             ((*src & 0x000000f0) >> 4));
    src++;
    *dest |= ((*src & 0xf0000000) |
              ((*src & 0x00f00000) << 4) |
              ((*src & 0x0000f000) << 8) |
              ((*src & 0x000000f0) << 12));
    src++;
    dest++;
  }
#else
  int siz = (width * height) >> 1;

  __asm {
    push ebx;
    push esi;
    push edi;

    mov esi, dword ptr [src];
    mov edi, dword ptr [dest];
    mov ecx, dword ptr [siz];

  tc1_loop:
    mov eax, dword ptr [esi];
    add esi, 4;

    mov edx, eax;
    and edx, 0xf0000000;  // aaaa0000 00000000 00000000 00000000
    shr edx, 16;          // 00000000 00000000 aaaa0000 00000000
    mov ebx, eax;
    and ebx, 0x00f00000;  // 00000000 rrrr0000 00000000 00000000
    shr ebx, 12;          // 00000000 00000000 0000rrrr 00000000
    or  edx, ebx;         // 00000000 00000000 aaaarrrr 00000000
    mov ebx, eax;
    and ebx, 0x0000f000;  // 00000000 00000000 gggg0000 00000000
    shr ebx, 8;           // 00000000 00000000 00000000 gggg0000
    or  edx, ebx;         // 00000000 00000000 aaaarrrr gggg0000
    and eax, 0x000000f0;  // 00000000 00000000 00000000 bbbb0000
    shr eax, 4;           // 00000000 00000000 00000000 0000bbbb
    or  edx, eax;         // 00000000 00000000 aaaarrrr ggggbbbb

    mov eax, dword ptr [esi];
    add esi, 4;

    mov ebx, eax;
    and ebx, 0xf0000000;  // aaaa0000 00000000 00000000 00000000
    or  edx, ebx;         // aaaa0000 00000000 aaaarrrr ggggbbbb
    mov ebx, eax;
    and ebx, 0x00f00000;  // 00000000 rrrr0000 00000000 00000000
    shl ebx, 4;           // 0000rrrr 00000000 00000000 00000000
    or  edx, ebx;         // aaaarrrr 00000000 aaaarrrr ggggbbbb
    mov ebx, eax;
    and ebx, 0x0000f000;  // 00000000 00000000 gggg0000 00000000
    shl ebx, 8;           // 00000000 gggg0000 00000000 00000000
    or  edx, ebx;         // aaaarrrr gggg0000 aaaarrrr ggggbbbb
    and eax, 0x000000f0;  // 00000000 00000000 00000000 bbbb0000
    shl eax, 12;          // 00000000 0000bbbb 00000000 00000000
    or  edx, eax;         // arrrrrgg ggggbbbb aaaarrrr ggggbbbb

    mov dword ptr [edi], edx;
    add edi, 4;

    dec ecx;
    jnz tc1_loop;

    pop edi;
    pop esi;
    pop ebx;
  }
#endif
}

void
TxQuantize::ARGB8888_RGB565(uint32* src, uint32* dest, int width, int height)
{
#if 1
  int siz = (width * height) >> 1;
  int i;
  for (i = 0; i < siz; i++) {
    *dest = (((*src & 0x000000f8) >> 3) |
             ((*src & 0x0000fc00) >> 5) |
             ((*src & 0x00f80000) >> 8));
    src++;
    *dest |= (((*src & 0x000000f8) << 13) |
              ((*src & 0x0000fc00) << 11) |
              ((*src & 0x00f80000) << 8));
    src++;
    dest++;
  }
#else
  int siz = (width * height) >> 1;

  __asm {
    push ebx;
    push esi;
    push edi;

    mov esi, dword ptr [src];
    mov edi, dword ptr [dest];
    mov ecx, dword ptr [siz];

  tc1_loop:
    mov eax, dword ptr [esi];
    add esi, 4;

    mov edx, eax;
    and edx, 0x000000F8;  // 00000000 00000000 00000000 bbbbb000
    shr edx, 3;           // 00000000 00000000 00000000 000bbbbb
    mov ebx, eax;
    and ebx, 0x0000FC00;  // 00000000 00000000 gggggg00 00000000
    shr ebx, 5;           // 00000000 00000000 00000ggg ggg00000
    or  edx, ebx;         // 00000000 00000000 00000ggg gggbbbbb
    mov ebx, eax;
    and ebx, 0x00F80000;  // 00000000 rrrrr000 00000000 00000000
    shr ebx, 8;           // 00000000 00000000 rrrrr000 00000000
    or  edx, ebx;         // 00000000 00000000 rrrrrggg gggbbbbb

    mov eax, dword ptr [esi];
    add esi, 4;

    mov ebx, eax;
    and ebx, 0x000000F8;  // 00000000 00000000 00000000 bbbbb000
    shl ebx, 13;          // 00000000 000bbbbb 00000000 00000000
    or  edx, ebx;         // 00000000 000bbbbb rrrrrggg gggbbbbb
    mov ebx, eax;
    and ebx, 0x0000FC00;  // 00000000 00000000 gggggg00 00000000
    shl ebx, 11;          // 00000ggg ggg00000 00000000 00000000
    or  edx, ebx;         // 00000ggg gggbbbbb rrrrrggg gggbbbbb
    mov ebx, eax;
    and ebx, 0x00F80000;  // 00000000 rrrrr000 00000000 00000000
    shl ebx, 8;           // rrrrr000 00000000 00000000 00000000
    or  edx, ebx;         // rrrrrggg gggbbbbb rrrrrggg gggbbbbb

    mov dword ptr [edi], edx;
    add edi, 4;

    dec ecx;
    jnz tc1_loop;

    pop edi;
    pop esi;
    pop ebx;
  }
#endif
}

void
TxQuantize::ARGB8888_A8(uint32* src, uint32* dest, int width, int height)
{
#if 1
  int siz = (width * height) >> 2;
  int i;
  for (i = 0; i < siz; i++) {
    *dest = (*src & 0x0000ff00) >> 8;
    src++;
    *dest |= (*src & 0x0000ff00);
    src++;
    *dest |= ((*src & 0x0000ff00) << 8);
    src++;
    *dest |= ((*src & 0x0000ff00) << 16);
    src++;
    dest++;
  }
#else
  int siz = (width * height) >> 2;

  __asm {
    push ebx;
    push esi;
    push edi;

    mov esi, dword ptr [src];
    mov edi, dword ptr [dest];
    mov ecx, dword ptr [siz];

  tc1_loop:
    mov eax, dword ptr [esi];
    add esi, 4;

#if 0
    mov edx, eax;         // we'll use A comp for every pixel
    and edx, 0xFF000000;  // aaaaaaaa 00000000 00000000 00000000
    shr edx, 24;          // 00000000 00000000 00000000 aaaaaaaa

    mov eax, dword ptr [esi];
    add esi, 4;

    and eax, 0xFF000000;  // aaaaaaaa 00000000 00000000 00000000
    shr eax, 16;          // 00000000 00000000 aaaaaaaa 00000000
    or  edx, eax;         // 00000000 00000000 aaaaaaaa aaaaaaaa

    mov eax, dword ptr [esi];
    add esi, 4;

    and eax, 0xFF000000;  // aaaaaaaa 00000000 00000000 00000000
    shr eax, 8;           // 00000000 aaaaaaaa 00000000 00000000
    or  edx, eax;         // 00000000 aaaaaaaa aaaaaaaa aaaaaaaa

    mov eax, dword ptr [esi];
    add esi, 4;

    and eax, 0xFF000000;  // aaaaaaaa 00000000 00000000 00000000
    or  edx, eax;         // aaaaaaaa aaaaaaaa aaaaaaaa aaaaaaaa
#endif

#if 1
    mov edx, eax;         // we'll use G comp for every pixel
    and edx, 0x0000FF00;  // 00000000 00000000 aaaaaaaa 00000000
    shr edx, 8;           // 00000000 00000000 00000000 aaaaaaaa

    mov eax, dword ptr [esi];
    add esi, 4;

    and eax, 0x0000FF00;  // 00000000 00000000 aaaaaaaa 00000000
    or  edx, eax;         // 00000000 00000000 aaaaaaaa aaaaaaaa

    mov eax, dword ptr [esi];
    add esi, 4;

    and eax, 0x0000FF00;  // 00000000 00000000 aaaaaaaa 00000000
    shl eax, 8;           // 00000000 aaaaaaaa 00000000 00000000
    or  edx, eax;         // 00000000 aaaaaaaa aaaaaaaa aaaaaaaa

    mov eax, dword ptr [esi];
    add esi, 4;

    and eax, 0x0000FF00;  // 00000000 00000000 aaaaaaaa 00000000
    shl eax, 16;          // aaaaaaaa 00000000 00000000 00000000
    or  edx, eax;         // aaaaaaaa aaaaaaaa aaaaaaaa aaaaaaaa
#endif

#if 0
    mov edx, eax;
    and edx, 0x000000FF;  // 00000000 00000000 00000000 aaaaaaaa

    mov eax, dword ptr [esi];
    add esi, 4;

    and eax, 0x0000FF00;  // 00000000 00000000 aaaaaaaa 00000000
    or  edx, eax;         // 00000000 00000000 aaaaaaaa aaaaaaaa

    mov eax, dword ptr [esi];
    add esi, 4;

    and eax, 0x00FF0000;  // 00000000 aaaaaaaa 00000000 00000000
    or  edx, eax;         // 00000000 aaaaaaaa aaaaaaaa aaaaaaaa

    mov eax, dword ptr [esi];
    add esi, 4;

    and eax, 0xFF000000;  // aaaaaaaa 00000000 00000000 00000000
    or  edx, eax;         // aaaaaaaa aaaaaaaa aaaaaaaa aaaaaaaa
#endif
    mov dword ptr [edi], edx;
    add edi, 4;

    dec ecx;
    jnz tc1_loop;

    pop edi;
    pop esi;
    pop ebx;
  }
#endif
}

void
TxQuantize::ARGB8888_AI44(uint32* src, uint32* dest, int width, int height)
{
#if 1
  int siz = (width * height) >> 2;
  int i;
  for (i = 0; i < siz; i++) {
    *dest = (((*src & 0xf0000000) >> 24) | ((*src & 0x0000f000) >> 12));
    src++;
    *dest |= (((*src & 0xf0000000) >> 16) | ((*src & 0x0000f000) >> 4));
    src++;
    *dest |= (((*src & 0xf0000000) >> 8) | ((*src & 0x0000f000) << 4));
    src++;
    *dest |= ((*src & 0xf0000000) | ((*src & 0x0000f000) << 12));
    src++;
    dest++;
  }
#else
  int siz = (width * height) >> 2;

  __asm {
    push ebx;
    push esi;
    push edi;

    mov esi, dword ptr [src];
    mov edi, dword ptr [dest];
    mov ecx, dword ptr [siz];

  tc1_loop:
    mov eax, dword ptr [esi];
    add esi, 4;

    mov edx, eax;         // use A and G comps MSB
    and edx, 0xF0000000;  // aaaa0000 00000000 00000000 00000000
    mov ebx, eax;
    shr edx, 24;          // 00000000 00000000 00000000 aaaa0000
    and ebx, 0x0000F000;  // 00000000 00000000 iiii0000 00000000
    shr ebx, 12;          // 00000000 00000000 00000000 0000iiii
    or  edx, ebx;         // 00000000 00000000 00000000 aaaaiiii

    mov eax, dword ptr [esi];
    add esi, 4;

    mov ebx, eax;
    and eax, 0xF0000000;  // aaaa0000 00000000 00000000 00000000
    shr eax, 16;          // 00000000 00000000 aaaa0000 00000000
    and ebx, 0x0000F000;  // 00000000 00000000 iiii0000 00000000
    shr ebx, 4;           // 00000000 00000000 0000iiii 00000000
    or  eax, ebx;         // 00000000 00000000 aaaaiiii 00000000
    or  edx, eax;         // 00000000 00000000 aaaaiiii aaaaiiii

    mov eax, dword ptr [esi];
    add esi, 4;

    mov ebx, eax;
    and eax, 0xF0000000;  // aaaa0000 00000000 00000000 00000000
    shr eax, 8;           // 00000000 aaaa0000 00000000 00000000
    and ebx, 0x0000F000;  // 00000000 00000000 iiii0000 00000000
    shl ebx, 4;           // 00000000 0000iiii 00000000 00000000
    or  eax, ebx;         // 00000000 aaaaiiii 00000000 00000000
    or  edx, eax;         // 00000000 aaaaiiii aaaaiiii aaaaiiii

    mov eax, dword ptr [esi];
    add esi, 4;

    mov ebx, eax;
    and eax, 0xF0000000;   // aaaa0000 00000000 00000000 00000000
    and ebx, 0x0000F000;   // 00000000 00000000 iiii0000 00000000
    shl ebx, 12;           // 0000iiii 00000000 00000000 00000000
    or  eax, ebx;          // aaaaiiii 00000000 00000000 00000000
    or  edx, eax;          // aaaaiiii aaaaiiii aaaaiiii aaaaiiii

    mov dword ptr [edi], edx;
    add edi, 4;

    dec ecx;
    jnz tc1_loop;

    pop edi;
    pop esi;
    pop ebx;
  }
#endif
}

void
TxQuantize::ARGB8888_AI88(uint32* src, uint32* dest, int width, int height)
{
#if 1
  int siz = (width * height) >> 1;
  int i;
  for (i = 0; i < siz; i++) {
    *dest = (((*src & 0xff000000) >> 16) | ((*src & 0x0000ff00) >> 8));
    src++;
    *dest |= ((*src & 0xff000000) | ((*src & 0x0000ff00) << 8));
    src++;
    dest++;
  }
#else
  int siz = (width * height) >> 1;

  __asm {
    push ebx;
    push esi;
    push edi;

    mov esi, dword ptr [src];
    mov edi, dword ptr [dest];
    mov ecx, dword ptr [siz];

  tc1_loop:
    mov eax, dword ptr [esi];
    add esi, 4;

    mov edx, eax;
    and edx, 0xFF000000;   // aaaaaaaa 00000000 00000000 00000000
    mov ebx, eax;
    shr edx, 16;           // 00000000 00000000 aaaaaaaa 00000000
    and ebx, 0x0000FF00;   // 00000000 00000000 iiiiiiii 00000000
    shr ebx, 8;            // 00000000 00000000 00000000 iiiiiiii
    or  edx, ebx;          // 00000000 00000000 aaaaaaaa iiiiiiii

    mov eax, dword ptr [esi];
    add esi, 4;

    mov ebx, eax;
    and eax, 0xFF000000;    // aaaaaaaa 00000000 00000000 00000000
    and ebx, 0x0000FF00;    // 00000000 00000000 iiiiiiii 00000000
    shl ebx, 8;             // 00000000 iiiiiiii 00000000 00000000
    or  eax, ebx;           // aaaaaaaa iiiiiiii 00000000 00000000
    or  edx, eax;           // aaaaaaaa iiiiiiii aaaaaaaa iiiiiiii

    mov dword ptr [edi], edx;
    add edi, 4;

    dec ecx;
    jnz tc1_loop;

    pop edi;
    pop esi;
    pop ebx;
  }
#endif
}

/* R.W. Floyd and L. Steinberg, An adaptive algorithm
 * for spatial grey scale, Proceedings of the Society
 * of Information Display 17, pp75-77, 1976
 */
void
TxQuantize::ARGB8888_RGB565_ErrD(uint32* src, uint32* dst, int width, int height)
{
  /* Floyd-Steinberg error-diffusion halftoning */

  int i, x, y;
  int qr, qg, qb; /* quantized incoming values */
  int ir, ig, ib; /* incoming values */
  int t;
  int *errR = new int[width];
  int *errG = new int[width];
  int *errB = new int[width];

  uint16 *dest = (uint16 *)dst;

  for (i = 0; i < width; i++) errR[i] = errG[i] = errB[i] = 0;

  for (y = 0; y < height; y++) {
    for (x = 0; x < width; x++) {
      /* incoming pixel values */
      ir = ((*src >> 16) & 0xFF) * 10000;
      ig = ((*src >>  8) & 0xFF) * 10000;
      ib = ((*src      ) & 0xFF) * 10000;

      if (x == 0) qr = qg = qb = 0;

      /* quantize pixel values. 
       * qr * 0.4375 is the error from the pixel to the left, 
       * errR is the error from the pixel to the top, top left, and top right */
      /* qr * 0.4375 is the error distribution to the EAST in 
       * the previous loop */
      ir += errR[x] + qr * 4375 / 10000;
      ig += errG[x] + qg * 4375 / 10000;
      ib += errB[x] + qb * 4375 / 10000;

      /* error distribution to the SOUTH-EAST in the previous loop 
       * can't calculate in the previous loop because it steps on 
       * the above quantization */
      errR[x] = qr * 625 / 10000;
      errG[x] = qg * 625 / 10000;
      errB[x] = qb * 625 / 10000;

      qr = ir;
      qg = ig;
      qb = ib;

      /* clamp */
      if (qr < 0) qr = 0; else if (qr > 2550000) qr = 2550000; 
      if (qg < 0) qg = 0; else if (qg > 2550000) qg = 2550000;
      if (qb < 0) qb = 0; else if (qb > 2550000) qb = 2550000;

      /* convert to RGB565 */
      qr = qr * 0x1F / 2550000;
      qg = qg * 0x3F / 2550000;
      qb = qb * 0x1F / 2550000;

      /* this is the dithered pixel */
      t  = (qr << 11) | (qg << 5) | qb;

      /* compute the errors */
      qr = ((qr << 3) | (qr >> 2)) * 10000;
      qg = ((qg << 2) | (qg >> 4)) * 10000;
      qb = ((qb << 3) | (qb >> 2)) * 10000;
      qr = ir - qr;
      qg = ig - qg;
      qb = ib - qb;

      /* compute the error distributions */
      /* Floyd-Steinberg filter
       * 7/16 (=0.4375) to the EAST 
       * 5/16 (=0.3125) to the SOUTH 
       * 1/16 (=0.0625) to the SOUTH-EAST 
       * 3/16 (=0.1875) to the SOUTH-WEST
       *
       *         x    7/16
       *  3/16  5/16  1/16
       */
      /* SOUTH-WEST */
      if (x > 1) {
        errR[x - 1] += qr * 1875 / 10000;
        errG[x - 1] += qg * 1875 / 10000;
        errB[x - 1] += qb * 1875 / 10000;
      }

      /* SOUTH */
      errR[x] += qr * 3125 / 10000;
      errG[x] += qg * 3125 / 10000;
      errB[x] += qb * 3125 / 10000;

      *dest = (t & 0xFFFF);

      dest++;
      src++;
    }
  }

  delete [] errR;
  delete [] errG;
  delete [] errB;
}


void
TxQuantize::ARGB8888_ARGB1555_ErrD(uint32* src, uint32* dst, int width, int height)
{
  /* Floyd-Steinberg error-diffusion halftoning */

  int i, x, y;
  int qr, qg, qb; /* quantized incoming values */
  int ir, ig, ib; /* incoming values */
  int t;
  int *errR = new int[width];
  int *errG = new int[width];
  int *errB = new int[width];

  uint16 *dest = (uint16 *)dst;

  for (i = 0; i < width; i++) errR[i] = errG[i] = errB[i] = 0;

  for (y = 0; y < height; y++) {
    for (x = 0; x < width; x++) {
      /* incoming pixel values */
      ir = ((*src >> 16) & 0xFF) * 10000;
      ig = ((*src >>  8) & 0xFF) * 10000;
      ib = ((*src      ) & 0xFF) * 10000;

      if (x == 0) qr = qg = qb = 0;

      /* quantize pixel values. 
       * qr * 0.4375 is the error from the pixel to the left, 
       * errR is the error from the pixel to the top, top left, and top right */
      /* qr * 0.4375 is the error distribution to the EAST in 
       * the previous loop */
      ir += errR[x] + qr * 4375 / 10000;
      ig += errG[x] + qg * 4375 / 10000;
      ib += errB[x] + qb * 4375 / 10000;

      /* error distribution to the SOUTH-EAST of the previous loop. 
       * cannot calculate in the previous loop because it steps on 
       * the above quantization */
      errR[x] = qr * 625 / 10000;
      errG[x] = qg * 625 / 10000;
      errB[x] = qb * 625 / 10000;

      qr = ir;
      qg = ig;
      qb = ib;

      /* clamp */
      if (qr < 0) qr = 0; else if (qr > 2550000) qr = 2550000;
      if (qg < 0) qg = 0; else if (qg > 2550000) qg = 2550000;
      if (qb < 0) qb = 0; else if (qb > 2550000) qb = 2550000;

      /* convert to RGB555 */
      qr = qr * 0x1F / 2550000;
      qg = qg * 0x1F / 2550000;
      qb = qb * 0x1F / 2550000;

      /* this is the dithered pixel */
      t  = (qr << 10) | (qg << 5) | qb;
      t |= ((*src >> 24) ? 0x8000 : 0);

      /* compute the errors */
      qr = ((qr << 3) | (qr >> 2)) * 10000;
      qg = ((qg << 3) | (qg >> 2)) * 10000;
      qb = ((qb << 3) | (qb >> 2)) * 10000;
      qr = ir - qr;
      qg = ig - qg;
      qb = ib - qb;

      /* compute the error distributions */
      /* Floyd-Steinberg filter
       * 7/16 (=0.4375) to the EAST 
       * 5/16 (=0.3125) to the SOUTH 
       * 1/16 (=0.0625) to the SOUTH-EAST 
       * 3/16 (=0.1875) to the SOUTH-WEST
       *
       *         x    7/16
       *  3/16  5/16  1/16
       */
      /* SOUTH-WEST */
      if (x > 1) {
        errR[x - 1] += qr * 1875 / 10000;
        errG[x - 1] += qg * 1875 / 10000;
        errB[x - 1] += qb * 1875 / 10000;
      }

      /* SOUTH */
      errR[x] += qr * 3125 / 10000;
      errG[x] += qg * 3125 / 10000;
      errB[x] += qb * 3125 / 10000;

      *dest = (t & 0xFFFF);

      dest++;
      src++;
    }
  }

  delete [] errR;
  delete [] errG;
  delete [] errB;
}

void
TxQuantize::ARGB8888_ARGB4444_ErrD(uint32* src, uint32* dst, int width, int height)
{
  /* Floyd-Steinberg error-diffusion halftoning */

  /* NOTE: alpha dithering looks better for alpha gradients, but are prone
   * to producing noisy speckles for constant or step level alpha. Output
   * results should always be checked.
   */
  boolean ditherAlpha = 0;

  int i, x, y;
  int qr, qg, qb, qa; /* quantized incoming values */
  int ir, ig, ib, ia; /* incoming values */
  int t;
  int *errR = new int[width];
  int *errG = new int[width];
  int *errB = new int[width];
  int *errA = new int[width];

  uint16 *dest = (uint16 *)dst;

  for (i = 0; i < width; i++) errR[i] = errG[i] = errB[i] = errA[i] = 0;

  for (y = 0; y < height; y++) {
    for (x = 0; x < width; x++) {
      /* incoming pixel values */
      ir = ((*src >> 16) & 0xFF) * 10000;
      ig = ((*src >>  8) & 0xFF) * 10000;
      ib = ((*src      ) & 0xFF) * 10000;
      ia = ((*src >> 24) & 0xFF) * 10000;

      if (x == 0) qr = qg = qb = qa = 0;

      /* quantize pixel values. 
       * qr * 0.4375 is the error from the pixel to the left, 
       * errR is the error from the pixel to the top, top left, and top right */
      /* qr * 0.4375 is the error distribution to the EAST in 
       * the previous loop */
      ir += errR[x] + qr * 4375 / 10000;
      ig += errG[x] + qg * 4375 / 10000;
      ib += errB[x] + qb * 4375 / 10000;
      ia += errA[x] + qa * 4375 / 10000;

      /* error distribution to the SOUTH-EAST of the previous loop. 
       * cannot calculate in the previous loop because it steps on 
       * the above quantization */
      errR[x] = qr * 625 / 10000;
      errG[x] = qg * 625 / 10000;
      errB[x] = qb * 625 / 10000;
      errA[x] = qa * 625 / 10000;

      qr = ir;
      qg = ig;
      qb = ib;
      qa = ia;

      /* clamp */
      if (qr < 0) qr = 0; else if (qr > 2550000) qr = 2550000;
      if (qg < 0) qg = 0; else if (qg > 2550000) qg = 2550000;
      if (qb < 0) qb = 0; else if (qb > 2550000) qb = 2550000;
      if (qa < 0) qa = 0; else if (qa > 2550000) qa = 2550000;

      /* convert to RGB444 */
      qr = qr * 0xF / 2550000;
      qg = qg * 0xF / 2550000;
      qb = qb * 0xF / 2550000;
      qa = qa * 0xF / 2550000;

      /* this is the value to be returned */
      if (ditherAlpha) {
        t = (qa << 12) | (qr <<  8) | (qg << 4) | qb;
      } else {
        t = (qr <<  8) | (qg << 4) | qb;
        t |= (*src >> 16) & 0xF000;
      }

      /* compute the errors */
      qr = ((qr << 4) | qr) * 10000;
      qg = ((qg << 4) | qg) * 10000;
      qb = ((qb << 4) | qb) * 10000;
      qa = ((qa << 4) | qa) * 10000;
      qr = ir - qr;
      qg = ig - qg;
      qb = ib - qb;
      qa = ia - qa;

      /* compute the error distributions */
      /* Floyd-Steinberg filter
       * 7/16 (=0.4375) to the EAST 
       * 5/16 (=0.3125) to the SOUTH 
       * 1/16 (=0.0625) to the SOUTH-EAST 
       * 3/16 (=0.1875) to the SOUTH-WEST
       *
       *         x    7/16
       *  3/16  5/16  1/16
       */
      /* SOUTH-WEST */
      if (x > 1) {
        errR[x - 1] += qr * 1875 / 10000;
        errG[x - 1] += qg * 1875 / 10000;
        errB[x - 1] += qb * 1875 / 10000;
        errA[x - 1] += qa * 1875 / 10000;
      }

      /* SOUTH */
      errR[x] += qr * 3125 / 10000;
      errG[x] += qg * 3125 / 10000;
      errB[x] += qb * 3125 / 10000;
      errA[x] += qa * 3125 / 10000;

      *dest = (t & 0xFFFF);

      dest++;
      src++;
    }
  }

  delete [] errR;
  delete [] errG;
  delete [] errB;
  delete [] errA;
}

void
TxQuantize::ARGB8888_AI44_ErrD(uint32* src, uint32* dst, int width, int height)
{
  /* Floyd-Steinberg error-diffusion halftoning */

  /* NOTE: alpha dithering looks better for alpha gradients, but are prone
   * to producing noisy speckles for constant or step level alpha. Output
   * results should always be checked.
   */
  boolean ditherAlpha = 0;

  int i, x, y;
  int qi, qa; /* quantized incoming values */
  int ii, ia; /* incoming values */
  int t;
  int *errI = new int[width];
  int *errA = new int[width];

  uint8 *dest = (uint8 *)dst;

  for (i = 0; i < width; i++) errI[i] = errA[i] = 0;

  for (y = 0; y < height; y++) {
    for (x = 0; x < width; x++) {
      /* 3dfx style Intensity = R * 0.299 + G * 0.587 + B * 0.114 */
      ii = ((*src >> 16) & 0xFF) * 2990 +
           ((*src >>  8) & 0xFF) * 5870 +
           ((*src      ) & 0xFF) * 1140;
      ia = ((*src >> 24) & 0xFF) * 10000;

      if (x == 0) qi = qa = 0;

      /* quantize pixel values. 
       * qi * 0.4375 is the error from the pixel to the left, 
       * errI is the error from the pixel to the top, top left, and top right */
      /* qi * 0.4375 is the error distrtibution to the EAST in
       * the previous loop */
      ii += errI[x] + qi * 4375 / 10000;
      ia += errA[x] + qa * 4375 / 10000;

      /* error distribution to the SOUTH-EAST in the previous loop. 
       * cannot calculate in the previous loop because it steps on 
       * the above quantization */
      errI[x] = qi * 625 / 10000;
      errA[x] = qa * 625 / 10000;

      qi = ii;
      qa = ia;

      /* clamp */
      if (qi < 0) qi = 0; else if (qi > 2550000) qi = 2550000;
      if (qa < 0) qa = 0; else if (qa > 2550000) qa = 2550000;

      /* convert to I4 */
      qi = qi * 0xF / 2550000;
      qa = qa * 0xF / 2550000;

      /* this is the value to be returned */
      if (ditherAlpha) {
        t = (qa << 4) | qi;
      } else {
        t = qi;
        t |= ((*src >> 24) & 0xF0);
      }

      /* compute the errors */
      qi = ((qi << 4) | qi) * 10000;
      qa = ((qa << 4) | qa) * 10000;
      qi = ii - qi;
      qa = ia - qa;

      /* compute the error distributions */
      /* Floyd-Steinberg filter
       * 7/16 (=0.4375) to the EAST 
       * 5/16 (=0.3125) to the SOUTH 
       * 1/16 (=0.0625) to the SOUTH-EAST 
       * 3/16 (=0.1875) to the SOUTH-WEST
       *
       *         x    7/16
       *  3/16  5/16  1/16
       */
      /* SOUTH-WEST */
      if (x > 1) {
        errI[x - 1] += qi * 1875 / 10000;
        errA[x - 1] += qa * 1875 / 10000;
      }

      /* SOUTH */
      errI[x] += qi * 3125 / 10000;
      errA[x] += qa * 3125 / 10000;

      *dest = t & 0xFF;

      dest++;
      src++;
    }
  }

  delete [] errI;
  delete [] errA;
}

void
TxQuantize::ARGB8888_AI88_Slow(uint32* src, uint32* dst, int width, int height)
{
  int x, y;
  uint16 *dest = (uint16 *)dst;
  for (y = 0; y < height; y++) {
    for (x = 0; x < width; x++) {
#if 1
      /* libpng style grayscale conversion.
       * Reduce RGB files to grayscale with or without alpha
       * using the equation given in Poynton's ColorFAQ at
       * <http://www.inforamp.net/~poynton/>
       * Copyright (c) 1998-01-04 Charles Poynton poynton at inforamp.net
       *
       *     Y = 0.212671 * R + 0.715160 * G + 0.072169 * B
       *
       *  We approximate this with
       *
       *     Y = 0.21268 * R    + 0.7151 * G    + 0.07217 * B
       *
       *  which can be expressed with integers as
       *
       *     Y = (6969 * R + 23434 * G + 2365 * B)/32768
       *
       *  The calculation is to be done in a linear colorspace.
       */
      *dest = (((int)((((*src >> 16) & 0xFF) * 6969 +
                       ((*src >>  8) & 0xFF) * 23434 +
                       ((*src      ) & 0xFF) * 2365) / 32768) & 0xFF) |
              (uint16)((*src >> 16) & 0xFF00));
#else
      /* 3dfx style Intensity = R * 0.299 + G * 0.587 + B * 0.114
       * this is same as the standard NTSC gray scale conversion. */
      *dest = (((int)((((*src >> 16) & 0xFF) * 299 +
                       ((*src >>  8) & 0xFF) * 587 +
                       ((*src      ) & 0xFF) * 114) / 1000) & 0xFF) |
              (uint16)((*src >> 16) & 0xFF00));
#endif
      dest++;
      src++;
    }
  }
}

void
TxQuantize::ARGB8888_I8_Slow(uint32* src, uint32* dst, int width, int height)
{
  int x, y;
  uint8 *dest = (uint8 *)dst;
  for (y = 0; y < height; y++) {
    for (x = 0; x < width; x++) {
#if 1
      /* libpng style Intensity = (6969 * R + 23434 * G + 2365 * B)/32768 */
      *dest = (int)((((*src >> 16) & 0xFF) * 6969 +
                     ((*src >>  8) & 0xFF) * 23434 +
                     ((*src      ) & 0xFF) * 2365) / 32768) & 0xFF;
#else
      /* 3dfx style Intensity = R * 0.299 + G * 0.587 + B * 0.114
       * this is same as the standard NTSC gray scale conversion. */
      *dest = (int)((((*src >>16) & 0xFF) * 299 +
                     ((*src >> 8) & 0xFF) * 587 +
                     ((*src     ) & 0xFF) * 114) / 1000) & 0xFF;
#endif
      dest++;
      src++;
    }
  }
}

void
TxQuantize::P8_16BPP(uint32* src, uint32* dest, int width, int height, uint32* palette)
{
  /* passed in palette is RGBA5551 format */
#if 1
  int i;
  int size = width * height;
  for (i = 0; i < size; i++) {
    ((uint16*)dest)[i] = ((uint16*)palette)[(int)(((uint8*)src)[i])];
    ((uint16*)dest)[i] = ((((uint16*)dest)[i] << 15) | (((uint16*)dest)[i] >> 1));
  }
#else

  /* not finished yet... */

  int siz = (width * height) >> 2;

  __asm {
    push ebx;
    push esi;
    push edi;

    mov esi, dword ptr [src];
    mov edi, dword ptr [dest];
    mov ecx, dword ptr [siz];
    mov edx, dword ptr [palette];

  tc1_loop:
    mov eax, dword ptr [esi];
    add esi, 4;

    dec ecx;
    jnz tc1_loop;

    pop edi;
    pop esi;
    pop ebx;
  }
#endif
}

boolean
TxQuantize::quantize(uint8* src, uint8* dest, int width, int height, uint16 srcformat, uint16 destformat, boolean fastQuantizer)
{
  typedef void (TxQuantize::*quantizerFunc)(uint32* src, uint32* dest, int width, int height);
  quantizerFunc quantizer;
  int bpp_shift = 0;

  if (destformat == GR_TEXFMT_ARGB_8888) {
    switch (srcformat) {
    case GR_TEXFMT_ARGB_1555:
      quantizer = &TxQuantize::ARGB1555_ARGB8888;
      bpp_shift = 1;
      break;
    case GR_TEXFMT_ARGB_4444:
      quantizer = &TxQuantize::ARGB4444_ARGB8888;
      bpp_shift = 1;
      break;
    case GR_TEXFMT_RGB_565:
      quantizer = &TxQuantize::RGB565_ARGB8888;
      bpp_shift = 1;
      break;
    case GR_TEXFMT_ALPHA_8:
      quantizer = &TxQuantize::A8_ARGB8888;
      bpp_shift = 2;
      break;
    case GR_TEXFMT_ALPHA_INTENSITY_44:
      quantizer = &TxQuantize::AI44_ARGB8888;
      bpp_shift = 2;
      break;
    case GR_TEXFMT_ALPHA_INTENSITY_88:
      quantizer = &TxQuantize::AI88_ARGB8888;
      bpp_shift = 1;
      break;
    default:
      return 0;
    }

    unsigned int numcore = _numcore;
    unsigned int blkrow = 0;
    while (numcore > 1 && blkrow == 0) {
      blkrow = (height >> 2) / numcore;
      numcore--;
    }
    if (blkrow > 0 && numcore > 1) {
      boost::thread *thrd[MAX_NUMCORE];
      unsigned int i;
      int blkheight = blkrow << 2;
      unsigned int srcStride = (width * blkheight) << (2 - bpp_shift);
      unsigned int destStride = srcStride << bpp_shift;
      for (i = 0; i < numcore - 1; i++) {
        thrd[i] = new boost::thread(boost::bind(quantizer,
                                                this,
                                                (uint32*)src,
                                                (uint32*)dest,
                                                width,
                                                blkheight));
        src  += srcStride;
        dest += destStride;
      }
      thrd[i] = new boost::thread(boost::bind(quantizer,
                                              this,
                                              (uint32*)src,
                                              (uint32*)dest,
                                              width,
                                              height - blkheight * i));
      for (i = 0; i < numcore; i++) {
        thrd[i]->join();
        delete thrd[i];
      }
    } else {
      (*this.*quantizer)((uint32*)src, (uint32*)dest, width, height);
    }

  } else if (srcformat == GR_TEXFMT_ARGB_8888) {
    switch (destformat) {
    case GR_TEXFMT_ARGB_1555:
      quantizer = fastQuantizer ? &TxQuantize::ARGB8888_ARGB1555 : &TxQuantize::ARGB8888_ARGB1555_ErrD;
      bpp_shift = 1;
      break;
    case GR_TEXFMT_ARGB_4444:
      quantizer = fastQuantizer ? &TxQuantize::ARGB8888_ARGB4444 : &TxQuantize::ARGB8888_ARGB4444_ErrD;
      bpp_shift = 1;
      break;
    case GR_TEXFMT_RGB_565:
      quantizer = fastQuantizer ? &TxQuantize::ARGB8888_RGB565 : &TxQuantize::ARGB8888_RGB565_ErrD;
      bpp_shift = 1;
      break;
    case GR_TEXFMT_ALPHA_8:
    case GR_TEXFMT_INTENSITY_8:
      quantizer = fastQuantizer ? &TxQuantize::ARGB8888_A8 : &TxQuantize::ARGB8888_I8_Slow;
      bpp_shift = 2;
      break;
    case GR_TEXFMT_ALPHA_INTENSITY_44:
      quantizer = fastQuantizer ? &TxQuantize::ARGB8888_AI44 : &TxQuantize::ARGB8888_AI44_ErrD;
      bpp_shift = 2;
      break;
    case GR_TEXFMT_ALPHA_INTENSITY_88:
      quantizer = fastQuantizer ? &TxQuantize::ARGB8888_AI88 : &TxQuantize::ARGB8888_AI88_Slow;
      bpp_shift = 1;
      break;
    default:
      return 0;
    }

    unsigned int numcore = _numcore;
    unsigned int blkrow = 0;
    while (numcore > 1 && blkrow == 0) {
      blkrow = (height >> 2) / numcore;
      numcore--;
    }
    if (blkrow > 0 && numcore > 1) {
      boost::thread *thrd[MAX_NUMCORE];
      unsigned int i;
      int blkheight = blkrow << 2;
      unsigned int srcStride = (width * blkheight) << 2;
      unsigned int destStride = srcStride >> bpp_shift;
      for (i = 0; i < numcore - 1; i++) {
        thrd[i] = new boost::thread(boost::bind(quantizer,
                                                this,
                                                (uint32*)src,
                                                (uint32*)dest,
                                                width,
                                                blkheight));
        src  += srcStride;
        dest += destStride;
      }
      thrd[i] = new boost::thread(boost::bind(quantizer,
                                              this,
                                              (uint32*)src,
                                              (uint32*)dest,
                                              width,
                                              height - blkheight * i));
      for (i = 0; i < numcore; i++) {
        thrd[i]->join();
        delete thrd[i];
      }
    } else {
      (*this.*quantizer)((uint32*)src, (uint32*)dest, width, height);
    }

  } else {
    return 0;
  }

  return 1;
}

boolean
TxQuantize::FXT1(uint8 *src, uint8 *dest,
             int srcwidth, int srcheight, uint16 srcformat,
             int *destwidth, int *destheight, uint16 *destformat)
{
  /*
   * NOTE: src must be in ARGB8888 format, srcformat describes
   * the closest 16bbp representation of src.
   *
   * NOTE: I have modified the dxtn library to use ARGB format
   * which originaly was ABGR format.
   */

  boolean bRet = 0;

  if (_tx_compress_fxt1 &&
      srcwidth >= 8 && srcheight >= 4) {
    /* compress to fxt1
     * width and height must be larger than 8 and 4 respectively
     */
    int dstRowStride = ((srcwidth + 7) & ~7) << 1;
    int srcRowStride = (srcwidth << 2);

    unsigned int numcore = _numcore;
    unsigned int blkrow = 0;
    while (numcore > 1 && blkrow == 0) {
      blkrow = (srcheight >> 2) / numcore;
      numcore--;
    }
    if (blkrow > 0 && numcore > 1) {
      boost::thread *thrd[MAX_NUMCORE];
      unsigned int i;
      int blkheight = blkrow << 2;
      unsigned int srcStride = (srcwidth * blkheight) << 2;
      unsigned int destStride = dstRowStride * blkrow;
      for (i = 0; i < numcore - 1; i++) {
        thrd[i] = new boost::thread(boost::bind(_tx_compress_fxt1,
                                                srcwidth,
                                                blkheight,
                                                4,
                                                src,
                                                srcRowStride,
                                                dest,
                                                dstRowStride));
        src  += srcStride;
        dest += destStride;
      }
      thrd[i] = new boost::thread(boost::bind(_tx_compress_fxt1,
                                              srcwidth,
                                              srcheight - blkheight * i,
                                              4,
                                              src,
                                              srcRowStride,
                                              dest,
                                              dstRowStride));
      for (i = 0; i < numcore; i++) {
        thrd[i]->join();
        delete thrd[i];
      }
    } else {
      (*_tx_compress_fxt1)(srcwidth,      /* width */
                           srcheight,     /* height */
                           4,             /* comps: ARGB8888=4, RGB888=3 */
                           src,           /* source */
                           srcRowStride,  /* width*comps */
                           dest,          /* destination */
                           dstRowStride); /* 16 bytes per 8x4 texel */
    }

    /* dxtn adjusts width and height to M8 and M4 respectively by replication */
    *destwidth  = (srcwidth  + 7) & ~7;
    *destheight = (srcheight + 3) & ~3;
    *destformat = GR_TEXFMT_ARGB_CMP_FXT1;

    bRet = 1;
  }
  
  return bRet;
}

boolean
TxQuantize::DXTn(uint8 *src, uint8 *dest,
             int srcwidth, int srcheight, uint16 srcformat,
             int *destwidth, int *destheight, uint16 *destformat)
{
  /*
   * NOTE: src must be in ARGB8888 format, srcformat describes
   * the closest 16bbp representation of src.
   *
   * NOTE: I have modified the dxtn library to use ARGB format
   * which originaly was ABGR format.
   */

  boolean bRet = 0;

  if (_tx_compress_dxtn &&
      srcwidth >= 4 && srcheight >= 4) {
    /* compress to dxtn
     * width and height must be larger than 4
     */

    /* skip formats that DXTn won't help in size. */
    if (srcformat == GR_TEXFMT_ALPHA_8 ||
        srcformat == GR_TEXFMT_ALPHA_INTENSITY_44) {
      ; /* shutup compiler */
    } else {
      int dstRowStride = ((srcwidth + 3) & ~3) << 2;
      int compression = GL_COMPRESSED_RGBA_S3TC_DXT5_EXT;

      *destformat = GR_TEXFMT_ARGB_CMP_DXT5;

#if !GLIDE64_DXTN
      /* okay... we are going to disable DXT1 with 1bit alpha
       * for Glide64. some textures have all 0 alpha values.
       * see "N64 Kobe Bryant in NBA Courtside"
       */
      if (srcformat == GR_TEXFMT_ARGB_1555) {
        dstRowStride >>= 1;
        compression = GL_COMPRESSED_RGBA_S3TC_DXT1_EXT;
        *destformat = GR_TEXFMT_ARGB_CMP_DXT1;
      } else
#endif
      if (srcformat == GR_TEXFMT_RGB_565 ||
          srcformat == GR_TEXFMT_INTENSITY_8) {
        dstRowStride >>= 1;
        compression = GL_COMPRESSED_RGB_S3TC_DXT1_EXT;
        *destformat = GR_TEXFMT_ARGB_CMP_DXT1;
      }

      unsigned int numcore = _numcore;
      unsigned int blkrow = 0;
      while (numcore > 1 && blkrow == 0) {
        blkrow = (srcheight >> 2) / numcore;
        numcore--;
      }
      if (blkrow > 0 && numcore > 1) {
        boost::thread *thrd[MAX_NUMCORE];
        unsigned int i;
        int blkheight = blkrow << 2;
        unsigned int srcStride = (srcwidth * blkheight) << 2;
        unsigned int destStride = dstRowStride * blkrow;
        for (i = 0; i < numcore - 1; i++) {
          thrd[i] = new boost::thread(boost::bind(_tx_compress_dxtn,
                                                  4,
                                                  srcwidth,
                                                  blkheight,
                                                  src,
                                                  compression,
                                                  dest,
                                                  dstRowStride));
          src  += srcStride;
          dest += destStride;
        }
        thrd[i] = new boost::thread(boost::bind(_tx_compress_dxtn,
                                                4,
                                                srcwidth,
                                                srcheight - blkheight * i,
                                                src,
                                                compression,
                                                dest,
                                                dstRowStride));
        for (i = 0; i < numcore; i++) {
          thrd[i]->join();
          delete thrd[i];
        }
      } else {
        (*_tx_compress_dxtn)(4,             /* comps: ARGB8888=4, RGB888=3 */
                             srcwidth,      /* width */
                             srcheight,     /* height */
                             src,           /* source */
                             compression,   /* format */
                             dest,          /* destination */
                             dstRowStride); /* DXT1 = 8 bytes per 4x4 texel
                                             * others = 16 bytes per 4x4 texel */
      }

      /* dxtn adjusts width and height to M4 by replication */
      *destwidth  = (srcwidth  + 3) & ~3;
      *destheight = (srcheight + 3) & ~3;

      bRet = 1;
    }
  }

  return bRet;
}

boolean
TxQuantize::compress(uint8 *src, uint8 *dest,
                    int srcwidth, int srcheight, uint16 srcformat,
                    int *destwidth, int *destheight, uint16 *destformat,
                    int compressionType)
{
  boolean bRet = 0;

  switch (compressionType) {
  case FXT1_COMPRESSION:
    bRet = FXT1(src, dest,
                srcwidth, srcheight, srcformat,
                destwidth, destheight, destformat);
    break;
  case S3TC_COMPRESSION:
    bRet = DXTn(src, dest,
                srcwidth, srcheight, srcformat,
                destwidth, destheight, destformat);
    break;
  case NCC_COMPRESSION:
    /* TODO: narrow channel compression */
    ;
  }

  return bRet;
}

#if 0 /* unused */
void
TxQuantize::I8_ARGB8888(uint32* src, uint32* dest, int width, int height)
{
  int siz = (width * height) >> 2;

  __asm {
    push ebx;
    push esi;
    push edi;

    mov esi, dword ptr [src];
    mov edi, dword ptr [dest];
    mov ecx, dword ptr [siz];

  tc1_loop:
    mov eax, dword ptr [esi];
    add esi, 4;

    // aaaaaaaa
    // 11111111 aaaaaaaa aaaaaaaa aaaaaaaa
    mov edx, eax;
    and eax, 0x000000ff;
    mov ebx, eax;        // 00000000 00000000 00000000 aaaaaaaa
    shl ebx, 8;          // 00000000 00000000 aaaaaaaa 00000000
    or  eax, ebx;        // 00000000 00000000 aaaaaaaa aaaaaaaa
    shl ebx, 8;         // 00000000 aaaaaaaa 00000000 00000000
    or  eax, ebx;        // 00000000 aaaaaaaa aaaaaaaa aaaaaaaa
    or  eax, 0xff000000; // 11111111 aaaaaaaa aaaaaaaa aaaaaaaa

    mov dword ptr [edi], eax;
    add edi, 4;

    mov eax, edx;
    and eax, 0x0000ff00;
    mov ebx, eax;        // 00000000 00000000 aaaaaaaa 00000000
    shr ebx, 8;          // 00000000 00000000 00000000 aaaaaaaa
    or  eax, ebx;        // 00000000 00000000 aaaaaaaa aaaaaaaa
    shl ebx, 16;         // 00000000 aaaaaaaa 00000000 00000000
    or  eax, ebx;        // 00000000 aaaaaaaa aaaaaaaa aaaaaaaa
    or  eax, 0xff000000; // 11111111 aaaaaaaa aaaaaaaa aaaaaaaa

    mov dword ptr [edi], eax;
    add edi, 4;

    mov eax, edx;
    and eax, 0x00ff0000;
    mov ebx, eax;        // 00000000 aaaaaaaa 00000000 00000000
    shr ebx, 8;          // 00000000 00000000 aaaaaaaa 00000000
    or  eax, ebx;        // 00000000 aaaaaaaa aaaaaaaa 00000000
    shr ebx, 8;         // 00000000 00000000 00000000 aaaaaaaa
    or  eax, ebx;        // 00000000 aaaaaaaa aaaaaaaa aaaaaaaa
    or  eax, 0xff000000; // 11111111 aaaaaaaa aaaaaaaa aaaaaaaa

    mov dword ptr [edi], eax;
    add edi, 4;

    mov eax, edx;
    and eax, 0xff000000;
    mov ebx, eax;        // aaaaaaaa 00000000 00000000 00000000
    shr ebx, 8;          // 00000000 aaaaaaaa 00000000 00000000
    or  eax, ebx;        // aaaaaaaa aaaaaaaa 00000000 00000000
    shr ebx, 8;         // 00000000 00000000 aaaaaaaa 00000000
    or  eax, ebx;        // aaaaaaaa aaaaaaaa aaaaaaaa 00000000
    shr eax, 8;         // 00000000 aaaaaaaa aaaaaaaa aaaaaaaa
    or  eax, 0xff000000; // 11111111 aaaaaaaa aaaaaaaa aaaaaaaa

    mov dword ptr [edi], eax;
    add edi, 4;

    dec ecx;
    jnz tc1_loop;

    pop edi;
    pop esi;
    pop ebx;
  }
}

void
TxQuantize::ARGB8888_I8(uint32* src, uint32* dest, int width, int height)
{
  ARGB8888_A8(src, dest, width, height);
}

void
TxQuantize::ARGB1555_ABGR8888(uint32* src, uint32* dest, int width, int height)
{
  int siz = (width * height) >> 1;

  __asm {
    push ebx;
    push esi;
    push edi;

    mov esi, dword ptr [src];
    mov edi, dword ptr [dest];
    mov ecx, dword ptr [siz];

  tc1_loop:
    mov eax, dword ptr [esi];
    add esi, 4;

    // arrr rrgg gggb bbbb
    // aaaaaaaa bbbbbbbb gggggggg rrrrrrrr
    mov edx, eax;         // edx = arrrrrgg gggbbbbb arrrrrgg gggbbbbb
    and ebx, 0x00000000;
    and eax, 0x00008000;  // eax = 00000000 00000000 a0000000 00000000
    jz  transparent1;
    or  ebx, 0xff000000;  // ebx = aaaaaaaa 00000000 00000000 00000000

  transparent1:
    mov eax, edx;         // eax = arrrrrgg gggbbbbb arrrrrgg gggbbbbb
    and edx, 0x0000001f;  // edx = 00000000 00000000 00000000 000bbbbb
    shl edx, 14;          // edx = 00000000 00000bbb bb000000 00000000
    or  ebx, edx;         // ebx = aaaaaaaa 00000bbb bb000000 00000000
    shl edx, 5;           // edx = 00000000 bbbbb000 00000000 00000000
    or  ebx, edx;         // ebx = aaaaaaaa bbbbbbbb bb000000 00000000
    and ebx, 0xffff0000;  // ebx = aaaaaaaa bbbbbbbb 00000000 00000000
    mov edx, eax;
    and edx, 0x000003e0;  // edx = 00000000 00000000 000000gg ggg00000
    shl edx, 1;           // edx = 00000000 00000000 00000ggg gg000000
    or  ebx, edx;         // ebx = aaaaaaaa bbbbbbbb 00000ggg gg000000
    shl edx, 5;           // edx = 00000000 00000000 ggggg000 00000000
    or  ebx, edx;         // ebx = aaaaaaaa bbbbbbbb gggggggg gg000000
    and ebx, 0xffffff00;  // ebx = aaaaaaaa bbbbbbbb gggggggg 00000000
    mov edx, eax;
    and edx, 0x00007c00;  // edx = 00000000 00000000 0rrrrr00 00000000
    shr edx, 7;           // edx = 00000000 00000000 00000000 rrrrr000
    or  ebx, edx;         // ebx = aaaaaaaa bbbbbbbb gggggggg rrrrr000
    shr edx, 5;           // edx = 00000000 00000000 00000000 00000rrr
    or  ebx, edx;         // ebx = aaaaaaaa bbbbbbbb gggggggg rrrrrrrr

    mov dword ptr [edi], ebx;
    add edi, 4;

    shr eax, 16;          // eax = 00000000 00000000 arrrrrgg gggbbbbb
    mov edx, eax;         // edx = 00000000 00000000 arrrrrgg gggbbbbb
    and ebx, 0x00000000;
    and eax, 0x00008000;  // eax = 00000000 00000000 a0000000 00000000
    jz  transparent2;
    or  ebx, 0xff000000;  // ebx = aaaaaaaa 00000000 00000000 00000000

  transparent2:
    mov eax, edx;         // eax = arrrrrgg gggbbbbb arrrrrgg gggbbbbb
    and edx, 0x0000001f;  // edx = 00000000 00000000 00000000 000bbbbb
    shl edx, 14;          // edx = 00000000 00000bbb bb000000 00000000
    or  ebx, edx;         // ebx = aaaaaaaa 00000bbb bb000000 00000000
    shl edx, 5;           // edx = 00000000 bbbbb000 00000000 00000000
    or  ebx, edx;         // ebx = aaaaaaaa bbbbbbbb bb000000 00000000
    and ebx, 0xffff0000;  // ebx = aaaaaaaa bbbbbbbb 00000000 00000000
    mov edx, eax;
    and edx, 0x000003e0;  // edx = 00000000 00000000 000000gg ggg00000
    shl edx, 1;           // edx = 00000000 00000000 00000ggg gg000000
    or  ebx, edx;         // ebx = aaaaaaaa bbbbbbbb 00000ggg gg000000
    shl edx, 5;           // edx = 00000000 00000000 ggggg000 00000000
    or  ebx, edx;         // ebx = aaaaaaaa bbbbbbbb gggggggg gg000000
    and ebx, 0xffffff00;  // ebx = aaaaaaaa bbbbbbbb gggggggg 00000000
    mov edx, eax;
    and edx, 0x00007c00;  // edx = 00000000 00000000 0rrrrr00 00000000
    shr edx, 7;           // edx = 00000000 00000000 00000000 rrrrr000
    or  ebx, edx;         // ebx = aaaaaaaa bbbbbbbb gggggggg rrrrr000
    shr edx, 5;           // edx = 00000000 00000000 00000000 00000rrr
    or  ebx, edx;         // ebx = aaaaaaaa bbbbbbbb gggggggg rrrrrrrr

    mov dword ptr [edi], ebx;
    add edi, 4;

    dec ecx;
    jnz tc1_loop;

    pop edi;
    pop esi;
    pop ebx;
  }
}

void
TxQuantize::ARGB4444_ABGR8888(uint32* src, uint32* dest, int width, int height)
{
  int siz = (width * height) >> 1;

  __asm {
    push ebx;
    push esi;
    push edi;

    mov esi, dword ptr [src];
    mov edi, dword ptr [dest];
    mov ecx, dword ptr [siz];

  tc1_loop:
    mov eax, dword ptr [esi];
    add esi, 4;

    // aaaa rrrr gggg bbbb
    // aaaaaaaa bbbbbbbb gggggggg rrrrrrrr
    mov edx, eax;
    and eax, 0x0000ffff;
    mov ebx, eax;        // 00000000 00000000 aaaarrrr ggggbbbb
    and ebx, 0x0000f000; // 00000000 00000000 aaaa0000 00000000
    shl ebx, 12;         // 0000aaaa 00000000 00000000 00000000
    or  eax, ebx;        // 0000aaaa 00000000 aaaarrrr ggggbbbb
    mov ebx, eax;
    and ebx, 0x0000000f; // 00000000 00000000 00000000 0000bbbb
    shl ebx, 16;         // 00000000 0000bbbb 00000000 00000000
    or  eax, ebx;        // 0000aaaa 0000bbbb aaaarrrr ggggbbbb
    mov ebx, eax;
    and ebx, 0x00000f00; // 00000000 00000000 0000rrrr 00000000
    shr ebx, 8;          // 00000000 00000000 00000000 0000rrrr
    and eax, 0xfffffff0;
    or  eax, ebx;        // 0000aaaa 0000bbbb aaaarrrr ggggrrrr
    mov ebx, eax;
    and ebx, 0x000000f0; // 00000000 00000000 00000000 gggg0000
    shl ebx, 4;          // 00000000 00000000 0000gggg 00000000
    and eax, 0x0f0f000f; // 0000aaaa 0000bbbb 00000000 0000rrrr
    or  eax, ebx;        // 0000aaaa 0000bbbb 0000gggg 0000rrrr
    mov ebx, eax;
    shl ebx, 4;          // aaaa0000 bbbb0000 gggg0000 rrrr0000
    or  eax, ebx;        // aaaaaaaa bbbbbbbb gggggggg rrrrrrrr

    mov dword ptr [edi], eax;

    add edi, 4;

    shr edx, 16;
    mov ebx, edx;        // 00000000 00000000 aaaarrrr ggggbbbb
    and ebx, 0x0000f000; // 00000000 00000000 aaaa0000 00000000
    shl ebx, 12;         // 0000aaaa 00000000 00000000 00000000
    or  edx, ebx;        // 0000aaaa 00000000 aaaarrrr ggggbbbb
    mov ebx, edx;
    and ebx, 0x0000000f; // 00000000 00000000 00000000 0000bbbb
    shl ebx, 16;         // 00000000 0000bbbb 00000000 00000000
    or  edx, ebx;        // 0000aaaa 0000bbbb aaaarrrr ggggbbbb
    mov ebx, edx;
    and ebx, 0x00000f00; // 00000000 00000000 0000rrrr 00000000
    shr ebx, 8;          // 00000000 00000000 00000000 0000rrrr
    and edx, 0xfffffff0;
    or  edx, ebx;        // 0000aaaa 0000bbbb aaaarrrr ggggrrrr
    mov ebx, edx;
    and ebx, 0x000000f0; // 00000000 00000000 00000000 gggg0000
    shl ebx, 4;          // 00000000 00000000 0000gggg 00000000
    and edx, 0x0f0f000f; // 0000aaaa 0000bbbb 00000000 0000rrrr
    or  edx, ebx;        // 0000aaaa 0000bbbb 0000gggg 0000rrrr
    mov ebx, edx;
    shl ebx, 4;          // aaaa0000 bbbb0000 gggg0000 rrrr0000
    or  edx, ebx;        // aaaaaaaa bbbbbbbb gggggggg rrrrrrrr

    mov dword ptr [edi], edx;
    add edi, 4;

    dec ecx;
    jnz tc1_loop;

    pop edi;
    pop esi;
    pop ebx;
  }
}

void
TxQuantize::ARGB8888_ABGR8888(uint32* src, uint32* dest, int width, int height)
{
  int siz = width * height;

  __asm {
    push ebx;
    push esi;
    push edi;

    mov esi, dword ptr [src];
    mov edi, dword ptr [dest];
    mov ecx, dword ptr [siz];

  tc1_loop:
    mov eax, dword ptr [esi];
    add esi, 4;

    // aaaaaaaa bbbbbbbb gggggggg rrrrrrrr
    mov edx, eax;
    bswap edx;
    shr edx, 8;
    and eax, 0xff000000;

    or eax, edx;

    mov dword ptr [edi], eax;
    add edi, 4;

    dec ecx;
    jnz tc1_loop;

    pop edi;
    pop esi;
    pop ebx;
  }
}
#endif
