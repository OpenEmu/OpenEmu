////////////////////////////////////////////////////////////////////////////////////////
//
// Nestopia - NES/Famicom emulator written in C++
//
// Copyright (C) 2003-2008 Martin Freij
//
// This file is part of Nestopia.
//
// Nestopia is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 2 of the License, or
// (at your option) any later version.
//
// Nestopia is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with Nestopia; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
//
////////////////////////////////////////////////////////////////////////////////////////

#include <cstring>
#include <new>
#include <iosfwd>
#include "NstVector.hpp"
#include "NstCrc32.hpp"
#include "NstStream.hpp"
#include "NstPatcherUps.hpp"

namespace Nes
{
	namespace Core
	{
		class Ups::Reader
		{
		public:

			uint Read();
			dword ReadInt();
			dword ReadCrc();

		private:

			Stream::In stream;
			dword remaining;
			dword crc;

		public:

			explicit Reader(std::istream& stdStream)
			:
			stream    (&stdStream),
			remaining (stream.Length()),
			crc       (0)
			{}

			void ReadSignature()
			{
				if
				(
					Read() != Ascii<'U'>::V ||
					Read() != Ascii<'P'>::V ||
					Read() != Ascii<'S'>::V ||
					Read() != Ascii<'1'>::V
				)
					throw RESULT_ERR_INVALID_FILE;
			}

			dword Remaining() const
			{
				return remaining;
			}

			dword Crc() const
			{
				return crc;
			}
		};

		class Ups::Writer
		{
		public:

			void Write(uint);
			void Write(const byte* NST_RESTRICT,dword);
			void WriteInt(dword);
			void WriteCrc(dword);

		private:

			Stream::Out stream;
			dword crc;

		public:

			explicit Writer(std::ostream& stdStream)
			:
			stream (&stdStream),
			crc    (0)
			{}

			void WriteSignature()
			{
				const byte data[] =
				{
					Ascii<'U'>::V,
					Ascii<'P'>::V,
					Ascii<'S'>::V,
					Ascii<'1'>::V
				};

				Write( data, sizeof(data) );
			}

			dword Crc() const
			{
				return crc;
			}
		};

		#ifdef NST_MSVC_OPTIMIZE
		#pragma optimize("s", on)
		#endif

		Ups::Ups()
		:
		srcSize (0),
		srcCrc  (0),
		dstSize (0),
		dstCrc  (0),
		patch   (NULL)
		{
		}

		Ups::~Ups()
		{
			Destroy();
		}

		void Ups::Destroy()
		{
			srcSize = 0;
			srcCrc  = 0;
			dstSize = 0;
			dstCrc  = 0;

			delete [] patch;
			patch = NULL;
		}

		#ifdef NST_MSVC_OPTIMIZE
		#pragma optimize("", on)
		#endif

		bool Ups::IsUps(std::istream& stream)
		{
			try
			{
				return Stream::In(&stream).Peek32() == AsciiId<'U','P','S','1'>::V;
			}
			catch (...)
			{
				return false;
			}
		}

		Result Ups::Load(std::istream& stdStream,const bool bypassChecksum)
		{
			Destroy();

			try
			{
				Reader reader( stdStream );

				reader.ReadSignature();

				srcSize = reader.ReadInt();
				dstSize = reader.ReadInt();

				const dword size = NST_MAX(srcSize,dstSize) + 1;

				patch = new byte [size];
				std::memset( patch, 0, size );

				for (dword i=0; reader.Remaining() > 4+4+4; ++i)
				{
					i += reader.ReadInt();

					if (i > MAX_OFFSET)
						throw RESULT_ERR_OUT_OF_MEMORY;

					while (const uint data = reader.Read())
					{
						if (i < dstSize)
							patch[i++] = data;
						else
							throw RESULT_ERR_CORRUPT_FILE;
					}
				}

				srcCrc = reader.ReadCrc();
				dstCrc = reader.ReadCrc();

				const dword crc = reader.Crc();
				const dword fileCrc = reader.ReadCrc();

				if (!bypassChecksum && crc != fileCrc)
					throw RESULT_ERR_INVALID_CRC;
			}
			catch (Result result)
			{
				Destroy();
				return result;
			}
			catch (const std::bad_alloc&)
			{
				Destroy();
				return RESULT_ERR_OUT_OF_MEMORY;
			}
			catch (...)
			{
				Destroy();
				return RESULT_ERR_CORRUPT_FILE;
			}

			return RESULT_OK;
		}

		Result Ups::Save(std::ostream& stdStream) const
		{
			try
			{
				Writer writer( stdStream );

				writer.WriteSignature();

				writer.WriteInt( srcSize );
				writer.WriteInt( dstSize );

				for (dword i=0, offset=0, n=dstSize; i < n; ++i)
				{
					if (patch[i])
					{
						writer.WriteInt( i - offset );
						offset = i;

						while (patch[++i]);

						NST_ASSERT( i <= n );

						writer.Write( patch + offset, i - offset );
						writer.Write( 0 );

						offset = i + 1;
					}
				}

				writer.WriteCrc( srcCrc );
				writer.WriteCrc( dstCrc );
				writer.WriteCrc( writer.Crc() );
			}
			catch (Result result)
			{
				return result;
			}
			catch (const std::bad_alloc&)
			{
				return RESULT_ERR_OUT_OF_MEMORY;
			}
			catch (...)
			{
				return RESULT_ERR_GENERIC;
			}

			return RESULT_OK;
		}

		Result Ups::Test(std::istream& stdStream,const bool bypassChecksum) const
		{
			Vector<byte> buffer( srcSize );

			try
			{
				Stream::In stream( &stdStream );

				if (stream.Length() < buffer.Size())
					return RESULT_ERR_CORRUPT_FILE;

				if (buffer.Size())
					stream.Peek( buffer.Begin(), buffer.Size() );
			}
			catch (Result result)
			{
				return result;
			}
			catch (const std::bad_alloc&)
			{
				return RESULT_ERR_OUT_OF_MEMORY;
			}
			catch (...)
			{
				return RESULT_ERR_CORRUPT_FILE;
			}

			return Test( buffer.Begin(), buffer.Size(), bypassChecksum );
		}

		Result Ups::Test(const byte* const NST_RESTRICT src,const dword size,const bool bypassChecksum) const
		{
			NST_ASSERT( !size || src );

			if (size < srcSize)
				return RESULT_ERR_CORRUPT_FILE;

			if (!bypassChecksum)
			{
				if (Crc32::Compute( src, srcSize ) != srcCrc)
					return RESULT_ERR_INVALID_CRC;

				dword crc = 0;

				for (dword i=0, n=dstSize; i < n; ++i)
					crc = Crc32::Compute( (i < size ? src[i] : 0U) ^ patch[i], crc );

				if (crc != dstCrc)
					return RESULT_ERR_INVALID_CRC;
			}

			return RESULT_OK;
		}

		bool Ups::Patch(const byte* const src,byte* const dst,const dword size,dword offset) const
		{
			NST_ASSERT( !size || (src && dst) );

			uint patched = 0;

			if (dstSize || src != dst)
			{
				for (dword i=0, end=dstSize; i < size; ++i)
					dst[i] = src[i] ^ (offset < end ? patched |= patch[offset], patch[offset++] : 0U);
			}

			return patched;
		}

		Result Ups::Create(const byte* const src,const byte* const dst,const dword size)
		{
			NST_ASSERT( !size || (src && dst) );

			Destroy();

			if (size)
			{
				patch = new (std::nothrow) byte [size];

				if (!patch)
					return RESULT_ERR_OUT_OF_MEMORY;

				srcSize = size;
				dstSize = size;

				srcCrc = Crc32::Compute( src, size );
				dstCrc = Crc32::Compute( dst, size );

				for (dword i=0; i < size; ++i)
					patch[i] = uint(src[i]) ^ uint(dst[i]);
			}

			return RESULT_OK;
		}

		uint Ups::Reader::Read()
		{
			if (remaining)
				remaining--;
			else
				throw 1;

			const uint data = stream.Read8();
			crc = Crc32::Compute( data, crc );

			return data;
		}

		dword Ups::Reader::ReadInt()
		{
			dword v = 0, s = 0;

			for (;;)
			{
				uint b = Read();
				v += (b & 0x7F) << s;

				if (v > MAX_SIZE)
					throw 1;

				if (b & 0x80)
					break;

				s += 7;
				v += 0x1UL << s;
			}

			return v;
		}

		dword Ups::Reader::ReadCrc()
		{
			dword crc = 0;

			for (uint i=0; i < 32; i += 8)
				crc |= dword(Read()) << i;

			return crc;
		}

		void Ups::Writer::Write(const uint data)
		{
			crc = Crc32::Compute( data, crc );
			stream.Write8( data );
		}

		void Ups::Writer::Write(const byte* const NST_RESTRICT data,const dword size)
		{
			crc = Crc32::Compute( data, size, crc );
			stream.Write( data, size );
		}

		void Ups::Writer::WriteCrc(const dword crc)
		{
			for (uint i=0; i < 32; i += 8)
				Write( crc >> i & 0xFF );
		}

		void Ups::Writer::WriteInt(dword v)
		{
			for (;;)
			{
				uint x = v & 0x7F;
				v >>= 7;

				if (!v)
				{
					Write( 0x80 | x );
					break;
				}

				Write( x );
				v--;
			}
		}
	}
}
