/*
Copyright (C) 2002 Andrea Mazzoleni ( http://advancemame.sf.net )
Copyright (C) 2001-4 Igor Pavlov ( http://www.7-zip.org )

This library is free software; you can redistribute it and/or
modify it under the terms of the GNU Lesser General Public
License version 2.1 as published by the Free Software Foundation.

This library is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public
License along with this library; if not, write to the Free Software
Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
*/

#include "lzma.h"

namespace NCompress {
namespace NLZMA {

UINT32 kDistStart[kDistTableSizeMax];

static class CConstInit
{
public:
  CConstInit()
  {
    UINT32 aStartValue = 0;
    int i;
    for (i = 0; i < kDistTableSizeMax; i++)
    {
      kDistStart[i] = aStartValue;
      aStartValue += (1 << kDistDirectBits[i]);
    }
  }
} g_ConstInit;

}}
