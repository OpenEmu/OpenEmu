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

#include <new>
#include <cstring>
#include <iosfwd>
#include "NstStream.hpp"
#include "NstPatcherIps.hpp"

namespace Nes
{
	namespace Core
	{
		Ips::~Ips()
		{
			Destroy();
		}

		bool Ips::IsIps(std::istream& stream)
		{
			try
			{
				byte data[5];
				Stream::In(&stream).Peek( data, 5 );

				return
				(
					data[0] == Ascii<'P'>::V &&
					data[1] == Ascii<'A'>::V &&
					data[2] == Ascii<'T'>::V &&
					data[3] == Ascii<'C'>::V &&
					data[4] == Ascii<'H'>::V
				);
			}
			catch (...)
			{
				return false;
			}
		}

		Result Ips::Load(std::istream& stdStream)
		{
			Destroy();

			if (!IsIps(stdStream))
				return RESULT_ERR_INVALID_FILE;

			try
			{
				Stream::In stream( &stdStream );

				stream.Seek( 5 );

				while (!stream.Eof())
				{
					byte data[4];
					stream.Read( data, 3 );

					if
					(
						data[0] == Ascii<'E'>::V &&
						data[1] == Ascii<'O'>::V &&
						data[2] == Ascii<'F'>::V
					)
						break;

					blocks.push_back(Block());
					Block& block = blocks.back();

					block.data = NULL;
					block.offset = dword(data[0]) << 16 | uint(data[1]) << 8 | data[2];

					stream.Read( data, 2 );
					block.length = uint(data[0]) << 8 | data[1];

					if (block.length)
					{
						block.fill = NO_FILL;
						block.data = new byte [block.length];
						stream.Read( block.data, block.length );
					}
					else
					{
						stream.Read( data, 2 );
						block.length = uint(data[0]) << 8 | data[1];

						if (block.length)
							block.fill = stream.Read8();
						else
							throw RESULT_ERR_CORRUPT_FILE;
					}
				}
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
				return RESULT_ERR_GENERIC;
			}

			return RESULT_OK;
		}

		Result Ips::Save(std::ostream& stdStream) const
		{
			try
			{
				Stream::Out stream( &stdStream );

				byte data[8];

				data[0] = Ascii<'P'>::V;
				data[1] = Ascii<'A'>::V;
				data[2] = Ascii<'T'>::V;
				data[3] = Ascii<'C'>::V;
				data[4] = Ascii<'H'>::V;

				stream.Write( data, 5 );

				for (Blocks::const_iterator it(blocks.begin()), end(blocks.end()); it != end; ++it)
				{
					data[0] = it->offset >> 16 & 0xFF;
					data[1] = it->offset >>  8 & 0xFF;
					data[2] = it->offset >>  0 & 0xFF;

					stream.Write( data, 3 );

					if (it->fill != NO_FILL)
					{
						data[0] = 0;
						data[1] = 0;

						stream.Write( data, 2 );
					}

					data[0] = it->length >> 8 & 0xFF;
					data[1] = it->length >> 0 & 0xFF;

					stream.Write( data, 2 );

					if (it->fill == NO_FILL)
						stream.Write( it->data, it->length );
					else
						stream.Write8( it->fill );
				}

				data[0] = Ascii<'E'>::V;
				data[1] = Ascii<'O'>::V;
				data[2] = Ascii<'F'>::V;

				stream.Write( data, 3 );
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

		Result Ips::Test(std::istream&) const
		{
			return RESULT_OK;
		}

		Result Ips::Test(const byte*,dword) const
		{
			return RESULT_OK;
		}

		bool Ips::Patch(const byte* const src,byte* const dst,const dword length,const dword offset) const
		{
			NST_ASSERT( !length || (src && dst) );

			bool patched = false;

			if (length)
			{
				if (src != dst)
					std::memcpy( dst, src, length );

				for (Blocks::const_iterator it(blocks.begin()), end(blocks.end()); it != end; ++it)
				{
					NST_ASSERT( it->length );

					if (it->offset < offset)
						continue;

					if (it->offset >= offset + length)
						break;

					const dword pos = it->offset - offset;
					const dword part = NST_MIN(it->length,length-pos);

					if (it->fill == NO_FILL)
						std::memcpy( dst + pos, it->data, part );
					else
						std::memset( dst + pos, it->fill, part );

					patched = true;
				}
			}

			return patched;
		}

		Result Ips::Create(const byte* const src,const byte* const dst,const dword length)
		{
			NST_ASSERT( !length || (src && dst) );

			Destroy();

			try
			{
				for (dword i=0; i < length; )
				{
					dword j = i++;

					if (src[j] == dst[j])
						continue;

					for (dword k=0; i < length; ++i)
					{
						if (src[i] != dst[i])
						{
							k = 0;
						}
						else if (k++ == MIN_EQUAL)
						{
							i -= MIN_EQUAL;
							break;
						}
					}

					do
					{
						if (j == AsciiId<'F','O','E'>::V)
							--j;

						blocks.push_back(Block());
						Block& block = blocks.back();

						block.data = NULL;
						block.offset = j;

						uint c = dst[j];

						dword k = j;
						const dword stop = NST_MIN(j + MAX_BLOCK,i);

						while (++k != stop && c == dst[k]);

						if (k - j >= MIN_BEG_RUN)
						{
							block.fill = c;
							block.length = k - j;
						}
						else
						{
							dword l = k;

							if (k + 1 < stop)
							{
								c = dst[k];

								for (l=k++; k < stop; ++k)
								{
									if (c != dst[k])
									{
										c = dst[k];
										l = k;
									}
									else if (k - l == MIN_MID_RUN)
									{
										k = l;
										break;
									}
								}
							}

							if (k == stop && k - l >= MIN_END_RUN)
								k = l;

							if (k == AsciiId<'F','O','E'>::V)
								++k;

							block.fill = NO_FILL;
							block.length = k - j;

							block.data = new byte [block.length];
							std::memcpy( block.data, dst + j, block.length );
						}

						j = k;
					}
					while (j != i);
				}
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
				return RESULT_ERR_GENERIC;
			}

			return RESULT_OK;
		}

		void Ips::Destroy()
		{
			for (Blocks::iterator it(blocks.begin()), end(blocks.end()); it != end; ++it)
				delete [] it->data;

			blocks.clear();
		}
	}
}
