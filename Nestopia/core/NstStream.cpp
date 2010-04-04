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

#include <iostream>
#include "NstVector.hpp"
#include "NstStream.hpp"

namespace Nes
{
	namespace Core
	{
		namespace Stream
		{
			void In::Clear()
			{
				std::istream& ref = *static_cast<std::istream*>(stream);

				if (!ref.bad())
					ref.clear();
			}

			void In::SafeRead(byte* data,dword size)
			{
				static_cast<std::istream*>(stream)->read( reinterpret_cast<char*>(data), size );
			}

			void In::Read(byte* data,dword size)
			{
				NST_ASSERT( data && size );

				SafeRead( data, size );

				if (!*static_cast<std::istream*>(stream))
					throw RESULT_ERR_CORRUPT_FILE;
			}

			uint In::Read8()
			{
				byte data;
				Read( &data, 1 );
				return data;
			}

			uint In::Read16()
			{
				byte data[2];
				Read( data, 2 );
				return data[0] | uint(data[1]) << 8;
			}

			dword In::Read32()
			{
				byte data[4];
				Read( data, 4 );
				return data[0] | uint(data[1]) << 8 | dword(data[2]) << 16 | dword(data[3]) << 24;
			}

			qword In::Read64()
			{
				byte data[8];
				Read( data, 8 );

				return
				(
					qword(data[4] | uint(data[5]) << 8 | dword(data[6]) << 16 | dword(data[7]) << 24) << 32 |
					dword(data[0] | uint(data[1]) << 8 | dword(data[2]) << 16 | dword(data[3]) << 24)
				);
			}

			uint In::SafeRead8()
			{
				byte data;
				SafeRead( &data, 1 );
				return *static_cast<std::istream*>(stream) ? data : ~0U;
			}

			#ifdef NST_MSVC_OPTIMIZE
			#pragma optimize("s", on)
			#endif

			void In::Seek(idword distance)
			{
				Clear();

				if (!static_cast<std::istream*>(stream)->seekg( distance, std::ios::cur ))
					throw RESULT_ERR_CORRUPT_FILE;
			}

			ulong In::Length()
			{
				Clear();

				std::istream& ref = *static_cast<std::istream*>(stream);

				const ulong pos = ref.tellg();

				if (!ref.seekg( 0, std::istream::end ))
					throw RESULT_ERR_CORRUPT_FILE;

				Clear();

				const ulong length = ulong(ref.tellg()) - pos;

				if (!ref.seekg( pos ))
					throw RESULT_ERR_CORRUPT_FILE;

				Clear();

				return length;
			}

			dword In::AsciiToC(char* NST_RESTRICT dst,const byte* NST_RESTRICT src,dword length)
			{
				const char* const dstEnd = dst + length;
				const byte* srcEnd = src;

				for (const byte* end=src+length; srcEnd != end && *srcEnd; )
					++srcEnd;

				while (srcEnd != src && srcEnd[-1] == Ascii<' '>::V)
					--srcEnd;

				while (src != srcEnd && *src && *src == Ascii<' '>::V)
					++src;

				while (src != srcEnd)
				{
					const byte b = *src++;
					char c;

					if (b >= Ascii<'a'>::V && b <= Ascii<'z'>::V)
					{
						c = b - Ascii<'a'>::V + 'a';
					}
					else if (b >= Ascii<'A'>::V && b <= Ascii<'Z'>::V)
					{
						c = b - Ascii<'A'>::V + 'A';
					}
					else if (b >= Ascii<'0'>::V && b <= Ascii<'9'>::V)
					{
						c = b - Ascii<'0'>::V + '0';
					}
					else switch (b)
					{
						case Ascii< '\0' >::V: c = '\0'; break;
						case Ascii< ' '  >::V: c = ' ' ; break;
						case Ascii< '!'  >::V: c = '!' ; break;
						case Ascii< '#'  >::V: c = '#' ; break;
						case Ascii< '%'  >::V: c = '%' ; break;
						case Ascii< '^'  >::V: c = '^' ; break;
						case Ascii< '&'  >::V: c = '&' ; break;
						case Ascii< '*'  >::V: c = '*' ; break;
						case Ascii< '('  >::V: c = '(' ; break;
						case Ascii< ')'  >::V: c = ')' ; break;
						case Ascii< '-'  >::V: c = '-' ; break;
						case Ascii< '_'  >::V: c = '_' ; break;
						case Ascii< '+'  >::V: c = '+' ; break;
						case Ascii< '='  >::V: c = '=' ; break;
						case Ascii< '~'  >::V: c = '~' ; break;
						case Ascii< '['  >::V: c = '[' ; break;
						case Ascii< ']'  >::V: c = ']' ; break;
						case Ascii< '\\' >::V: c = '\\'; break;
						case Ascii< '|'  >::V: c = '|' ; break;
						case Ascii< ';'  >::V: c = ';' ; break;
						case Ascii< ':'  >::V: c = ':' ; break;
						case Ascii< '\'' >::V: c = '\''; break;
						case Ascii< '\"' >::V: c = '\"'; break;
						case Ascii< '{'  >::V: c = '{' ; break;
						case Ascii< '}'  >::V: c = '}' ; break;
						case Ascii< ','  >::V: c = ',' ; break;
						case Ascii< '.'  >::V: c = '.' ; break;
						case Ascii< '<'  >::V: c = '<' ; break;
						case Ascii< '>'  >::V: c = '>' ; break;
						case Ascii< '/'  >::V: c = '/' ; break;
						case Ascii< '?'  >::V: c = '?' ; break;

						case Ascii< '\a' >::V:
						case Ascii< '\b' >::V:
						case Ascii< '\t' >::V:
						case Ascii< '\v' >::V:
						case Ascii< '\n' >::V:
						case Ascii< '\r' >::V:
						case Ascii< '\f' >::V:

							NST_DEBUG_MSG("invalid stream character!");
							continue;

						default:

							NST_DEBUG_MSG("unknown stream character!");

							c = b - (CHAR_MIN < 0 ? 0x100 : 0);
							break;
					}

					*dst++ = c;
				}

				length -= dstEnd - dst;

				while (dst != dstEnd)
					*dst++ = '\0';

				return length;
			}

			void In::Read(char* dst,dword size)
			{
				NST_ASSERT( dst && size );

				Vector<byte> buffer( size );
				Read( buffer.Begin(), size );
				AsciiToC( dst, buffer.Begin(), size );
			}

			dword In::Read(Vector<char>& string)
			{
				Vector<byte> buffer;
				buffer.Reserve( 32 );

				for (uint c; (c=Read8()) != '\0'; buffer.Append(c));

				string.Resize( buffer.Size() + 1 );
				string.SetTo( AsciiToC( string.Begin(), buffer.Begin(), buffer.Size() ) + 1 );
				string.Back() = '\0';

				return buffer.Size() + 1;
			}

			#ifdef NST_MSVC_OPTIMIZE
			#pragma optimize("", on)
			#endif

			uint In::Peek8()
			{
				const uint data = Read8();
				Seek( -1 );
				return data;
			}

			uint In::Peek16()
			{
				const uint data = Read16();
				Seek( -2 );
				return data;
			}

			dword In::Peek32()
			{
				const dword data = Read32();
				Seek( -4 );
				return data;
			}

			void In::Peek(byte* data,dword length)
			{
				Read( data, length );
				Seek( -idword(length) );
			}

			bool In::Eof()
			{
				std::istream& ref = *static_cast<std::istream*>(stream);
				return ref.eof() || (ref.peek(), ref.eof());
			}

			void Out::Write(const byte* data,dword size)
			{
				NST_VERIFY( data && size );

				if (!static_cast<std::ostream*>(stream)->write( reinterpret_cast<const char*>(data), size ))
					throw RESULT_ERR_CORRUPT_FILE;
			}

			void Out::Write8(const uint data)
			{
				NST_VERIFY( data <= 0xFF );

				const byte d = data & 0xFF;
				Write( &d, 1 );
			}

			void Out::Write16(const uint data)
			{
				NST_VERIFY( data <= 0xFFFF );

				const byte d[2] =
				{
					data >> 0 & 0xFF,
					data >> 8 & 0xFF
				};

				Write( d, 2 );
			}

			void Out::Write32(const dword data)
			{
				NST_VERIFY( data <= 0xFFFFFFFF );

				const byte d[4] =
				{
					data >>  0 & 0xFF,
					data >>  8 & 0xFF,
					data >> 16 & 0xFF,
					data >> 24 & 0xFF
				};

				Write( d, 4 );
			}

			void Out::Write64(const qword data)
			{
				const byte d[8] =
				{
					data >>  0 & 0xFF,
					data >>  8 & 0xFF,
					data >> 16 & 0xFF,
					data >> 24 & 0xFF,
					data >> 32 & 0xFF,
					data >> 40 & 0xFF,
					data >> 48 & 0xFF,
					data >> 56 & 0xFF
				};

				Write( d, 8 );
			}

			#ifdef NST_MSVC_OPTIMIZE
			#pragma optimize("s", on)
			#endif

			void Out::Clear()
			{
				std::ostream& ref = *static_cast<std::ostream*>(stream);

				if (!ref.bad())
					ref.clear();
			}

			void Out::Seek(idword distance)
			{
				Clear();

				if (!static_cast<std::ostream*>(stream)->seekp( distance, std::ios::cur ))
					throw RESULT_ERR_CORRUPT_FILE;
			}

			bool Out::SeekEnd()
			{
				Clear();

				std::ostream& ref = *static_cast<std::ostream*>(stream);

				const std::streampos pos( ref.tellp() );
				ref.seekp( 0, std::ios::end );
				const bool advanced = !(pos == ref.tellp());

				Clear();

				return advanced;
			}

			#ifdef NST_MSVC_OPTIMIZE
			#pragma optimize("", on)
			#endif
		}
	}
}
