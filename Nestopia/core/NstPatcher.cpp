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
#include "NstVector.hpp"
#include "NstPatcher.hpp"
#include "NstPatcherIps.hpp"
#include "NstPatcherUps.hpp"

namespace Nes
{
	namespace Core
	{
		#ifdef NST_MSVC_OPTIMIZE
		#pragma optimize("s", on)
		#endif

		Patcher::Patcher(bool b)
		:
		ips            (NULL),
		ups            (NULL),
		bypassChecksum (b)
		{
		}

		Patcher::~Patcher()
		{
			Destroy();
		}

		void Patcher::Destroy()
		{
			delete ips;
			ips = NULL;

			delete ups;
			ups = NULL;
		}

		#ifdef NST_MSVC_OPTIMIZE
		#pragma optimize("", on)
		#endif

		Result Patcher::Load(std::istream& stream)
		{
			Destroy();

			if (Ips::IsIps( stream ))
			{
				ips = new (std::nothrow) Ips;
				return ips ? ips->Load( stream ) : RESULT_ERR_OUT_OF_MEMORY;
			}

			if (Ups::IsUps( stream ))
			{
				ups = new (std::nothrow) Ups;
				return ups ? ups->Load( stream, bypassChecksum ) : RESULT_ERR_OUT_OF_MEMORY;
			}

			return RESULT_ERR_INVALID_FILE;
		}

		Result Patcher::Load(std::istream& patchStream,std::istream& srcStream)
		{
			Result result = Load( patchStream );

			if (NES_SUCCEEDED(result))
			{
				result = Test( srcStream );

				if (NES_FAILED(result))
					Destroy();
			}

			return result;
		}

		Result Patcher::Save(std::ostream& stream) const
		{
			return
			(
				ips ? ips->Save( stream ) :
				ups ? ups->Save( stream ) : RESULT_ERR_NOT_READY
			);
		}

		Result Patcher::Test(std::istream& stream) const
		{
			return
			(
				ips ? ips->Test( stream ) :
				ups ? ups->Test( stream, bypassChecksum ) : RESULT_ERR_NOT_READY
			);
		}

		Result Patcher::Test(const Block* const blocks,const uint numBlocks) const
		{
			NST_ASSERT( blocks || !numBlocks );

			if (numBlocks > 1)
			{
				Vector<byte> buffer;

				try
				{
					dword size = 0;

					for (uint i=0; i < numBlocks; ++i)
						size += blocks[i].size;

					buffer.Reserve( size );
				}
				catch (...)
				{
					return RESULT_ERR_OUT_OF_MEMORY;
				}

				for (uint i=0; i < numBlocks; ++i)
					buffer.Append( blocks[i].data, blocks[i].size );

				return Test( buffer.Begin(), buffer.Size() );
			}
			else
			{
				return Test( blocks ? blocks->data : NULL, blocks ? blocks->size : 0 );
			}
		}

		Result Patcher::Test(const byte* const data,const dword size) const
		{
			return
			(
				ips ? ips->Test( data, size ) :
				ups ? ups->Test( data, size, bypassChecksum ) : RESULT_ERR_NOT_READY
			);
		}

		bool Patcher::Patch(const byte* const src,byte* const dst,const dword size,const dword offset) const
		{
			return
			(
				ips ? ips->Patch( src, dst, size, offset ) :
				ups ? ups->Patch( src, dst, size, offset ) : false
			);
		}

		Result Patcher::Create(const Type type,const byte* const src,const byte* const dst,const dword length)
		{
			Destroy();

			switch (type)
			{
				case IPS:

					ips = new (std::nothrow) Ips;
					return ips ? ips->Create( src, dst, length ) : RESULT_ERR_OUT_OF_MEMORY;

				case UPS:

					ups = new (std::nothrow) Ups;
					return ups ? ups->Create( src, dst, length ) : RESULT_ERR_OUT_OF_MEMORY;
			}

			return RESULT_ERR_UNSUPPORTED;
		}

		bool Patcher::Empty() const
		{
			return
			(
				ips ? ips->Empty() :
				ups ? ups->Empty() : true
			);
		}
	}
}
