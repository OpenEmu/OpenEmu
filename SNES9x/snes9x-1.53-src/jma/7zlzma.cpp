/*
Copyright (C) 2005-2006 NSRT Team ( http://nsrt.edgeemu.com )
Copyright (C) 2002 Andrea Mazzoleni ( http://advancemame.sf.net )

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

#include "7z.h"

#include "lzmadec.h"

bool decompress_lzma_7z(ISequentialInStream& in, unsigned in_size, ISequentialOutStream& out, unsigned out_size) throw ()
{
  try
  {
    NCompress::NLZMA::CDecoder cc;

    UINT64 in_size_l = in_size;
    UINT64 out_size_l = out_size;

    if (cc.ReadCoderProperties(&in) != S_OK)                 { return(false); }
    if (cc.Code(&in, &out, &in_size_l, &out_size_l) != S_OK) { return(false); }
    if (out.size_get() != out_size || out.overflow_get())    { return(false); }

    return(true);
  }
  catch (...)
  {
    return(false);
  }
}

bool decompress_lzma_7z(const unsigned char* in_data, unsigned int in_size, unsigned char* out_data, unsigned int out_size) throw ()
{
  ISequentialInStream_Array in(reinterpret_cast<const char*>(in_data), in_size);
  ISequentialOutStream_Array out(reinterpret_cast<char*>(out_data), out_size);

  return(decompress_lzma_7z(in, in_size, out, out_size));
}
