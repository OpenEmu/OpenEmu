/*
Copyright (C) 2005-2006 NSRT Team ( http://nsrt.edgeemu.com )
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

#ifndef __IINOUTSTREAMS_H
#define __IINOUTSTREAMS_H

#include <string>
#include <fstream>

#include "portable.h"


class ISequentialInStream
{
public:
  virtual HRESULT Read(void *, UINT32, UINT32 *) = 0;

  virtual ~ISequentialInStream() {}
};


class ISequentialInStream_Array : public ISequentialInStream
{
  const char *data;
  unsigned int size;
public:
  ISequentialInStream_Array(const char *Adata, unsigned Asize) : data(Adata), size(Asize) { }

  HRESULT Read(void *aData, UINT32 aSize, UINT32 *aProcessedSize);

  virtual ~ISequentialInStream_Array() {}
};

class ISequentialInStream_String : public ISequentialInStream
{
  std::string& data;
public:
  ISequentialInStream_String(std::string& Adata) : data(Adata) { }

  HRESULT Read(void *aData, UINT32 aSize, UINT32 *aProcessedSize);

  virtual ~ISequentialInStream_String() {}
};

class ISequentialInStream_Istream : public ISequentialInStream
{
  std::istream& data;
public:
  ISequentialInStream_Istream(std::istream& Adata) : data(Adata) { }

  HRESULT Read(void *aData, UINT32 aSize, UINT32 *aProcessedSize);

  virtual ~ISequentialInStream_Istream() {}
};



class ISequentialOutStream
{
public:
  virtual bool overflow_get() const = 0;
  virtual unsigned int size_get() const = 0;

  virtual HRESULT Write(const void *, UINT32, UINT32 *) = 0;

  virtual ~ISequentialOutStream() {}
};


class ISequentialOutStream_Array : public ISequentialOutStream
{
  char *data;
  unsigned int size;
  bool overflow;
  unsigned int total;
public:
  ISequentialOutStream_Array(char *Adata, unsigned Asize) : data(Adata), size(Asize), overflow(false), total(0) { }

  bool overflow_get() const { return(overflow); }
  unsigned int size_get() const { return(total); }

  HRESULT Write(const void *aData, UINT32 aSize, UINT32 *aProcessedSize);

  virtual ~ISequentialOutStream_Array() {}
};

class ISequentialOutStream_String : public ISequentialOutStream
{
  std::string& data;
  unsigned int total;
public:
  ISequentialOutStream_String(std::string& Adata) : data(Adata), total(0) { }

  bool overflow_get() const { return(false); }
  unsigned int size_get() const { return(total); }

  HRESULT Write(const void *aData, UINT32 aSize, UINT32 *aProcessedSize);

  virtual ~ISequentialOutStream_String() {}
};


class ISequentialOutStream_Ostream : public ISequentialOutStream
{
  std::ostream& data;
  unsigned int total;
public:
  ISequentialOutStream_Ostream(std::ostream& Adata) : data(Adata), total(0) { }

  bool overflow_get() const { return(false); }
  unsigned int size_get() const { return(total); }

  HRESULT Write(const void *aData, UINT32 aSize, UINT32 *aProcessedSize);

  virtual ~ISequentialOutStream_Ostream() {}
};



class ISequentialStreamCRC32
{
protected:
  unsigned int crc32;
public:
  ISequentialStreamCRC32() : crc32(0) {}
  unsigned int crc32_get() const { return(crc32); }

  virtual ~ISequentialStreamCRC32() {}
};


class ISequentialInStreamCRC32_Array : public ISequentialInStream_Array, public ISequentialStreamCRC32
{
public:
  ISequentialInStreamCRC32_Array(const char *Adata, unsigned Asize) : ISequentialInStream_Array(Adata, Asize) { }

  HRESULT Read(void *aData, UINT32 aSize, UINT32 *aProcessedSize);

  virtual ~ISequentialInStreamCRC32_Array() {}
};

class ISequentialInStreamCRC32_String : public ISequentialInStream_String, public ISequentialStreamCRC32
{
public:
  ISequentialInStreamCRC32_String(std::string& Adata) : ISequentialInStream_String(Adata) { }

  HRESULT Read(void *aData, UINT32 aSize, UINT32 *aProcessedSize);

  virtual ~ISequentialInStreamCRC32_String() {}
};

class ISequentialInStreamCRC32_Istream : public ISequentialInStream_Istream, public ISequentialStreamCRC32
{
public:
  ISequentialInStreamCRC32_Istream(std::istream& Adata) : ISequentialInStream_Istream(Adata) { }

  HRESULT Read(void *aData, UINT32 aSize, UINT32 *aProcessedSize);

  virtual ~ISequentialInStreamCRC32_Istream() {}
};


class ISequentialOutStreamCRC32_Array : public ISequentialOutStream_Array, public ISequentialStreamCRC32
{
public:
  ISequentialOutStreamCRC32_Array(char *Adata, unsigned Asize) : ISequentialOutStream_Array(Adata, Asize) { }

  HRESULT Write(const void *aData, UINT32 aSize, UINT32 *aProcessedSize);

  virtual ~ISequentialOutStreamCRC32_Array() {}
};

class ISequentialOutStreamCRC32_String : public ISequentialOutStream_String, public ISequentialStreamCRC32
{
public:
  ISequentialOutStreamCRC32_String(std::string& Adata) : ISequentialOutStream_String(Adata) { }

  HRESULT Write(const void *aData, UINT32 aSize, UINT32 *aProcessedSize);

  virtual ~ISequentialOutStreamCRC32_String() {}
};


class ISequentialOutStreamCRC32_Ostream : public ISequentialOutStream_Ostream, public ISequentialStreamCRC32
{
public:
  ISequentialOutStreamCRC32_Ostream(std::ostream& Adata) : ISequentialOutStream_Ostream(Adata) { }

  HRESULT Write(const void *aData, UINT32 aSize, UINT32 *aProcessedSize);

  virtual ~ISequentialOutStreamCRC32_Ostream() {}
};

#endif
