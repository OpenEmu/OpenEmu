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

#ifndef __TXRESAMPLE_H__
#define __TXRESAMPLE_H__

#include "TxInternal.h"

class TxReSample
{
private:
  double tent(double x);
  double gaussian(double x);
  double sinc(double x);
  double lanczos3(double x);
  double mitchell(double x);
  double besselI0(double x);
  double kaiser(double x);
public:
  boolean minify(uint8 **src, int *width, int *height, int ratio);
  boolean nextPow2(uint8** image, int* width, int* height, int bpp, boolean use_3dfx);
  int nextPow2(int num);
};

#endif /* __TXRESAMPLE_H__ */
