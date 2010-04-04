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

////////////////////////////////////////////////////////////////////////////////////////
//
// Based on public domain code written by Steve Reid
//
////////////////////////////////////////////////////////////////////////////////////////

#include <cstring>
#include "NstAssert.hpp"
#include "NstSha1.hpp"

namespace Nes
{
	namespace Core
	{
		namespace Sha1
		{
			#define NST_ROL(v,b) (((v) << (b) & 0xFFFFFFFF) | ((v) >> (32 - (b))))
			#define NST_BLK(p,i) (p[i&15U] = NST_ROL(p[(i+13)&15U] ^ p[(i+8)&15U] ^ p[(i+2)&15U] ^ p[i&15U],1))
			#define NST_R0(p,v,w,x,y,z,i) z = (z + ((w & (x ^ y)) ^ y)       + p[i]         + 0x5A827999 + NST_ROL(v,5)) & 0xFFFFFFFF; w = NST_ROL(w,30)
			#define NST_R1(p,v,w,x,y,z,i) z = (z + ((w & (x ^ y)) ^ y)       + NST_BLK(p,i) + 0x5A827999 + NST_ROL(v,5)) & 0xFFFFFFFF; w = NST_ROL(w,30)
			#define NST_R2(p,v,w,x,y,z,i) z = (z + (w ^ x ^ y)               + NST_BLK(p,i) + 0x6ED9EBA1 + NST_ROL(v,5)) & 0xFFFFFFFF; w = NST_ROL(w,30)
			#define NST_R3(p,v,w,x,y,z,i) z = (z + (((w | x) & y) | (w & x)) + NST_BLK(p,i) + 0x8F1BBCDC + NST_ROL(v,5)) & 0xFFFFFFFF; w = NST_ROL(w,30)
			#define NST_R4(p,v,w,x,y,z,i) z = (z + (w ^ x ^ y)               + NST_BLK(p,i) + 0xCA62C1D6 + NST_ROL(v,5)) & 0xFFFFFFFF; w = NST_ROL(w,30)

			static void Transform(dword* const NST_RESTRICT state,const byte* const NST_RESTRICT buffer)
			{
				dword p[16];

				for (uint i=0; i < 16; ++i)
					p[i] = dword(buffer[i*4+0]) << 24 | dword(buffer[i*4+1]) << 16 | uint(buffer[i*4+2]) << 8 | buffer[i*4+3];

				dword a = state[0];
				dword b = state[1];
				dword c = state[2];
				dword d = state[3];
				dword e = state[4];

				NST_R0(p,a,b,c,d,e, 0); NST_R0(p,e,a,b,c,d, 1); NST_R0(p,d,e,a,b,c, 2); NST_R0(p,c,d,e,a,b, 3);
				NST_R0(p,b,c,d,e,a, 4); NST_R0(p,a,b,c,d,e, 5); NST_R0(p,e,a,b,c,d, 6); NST_R0(p,d,e,a,b,c, 7);
				NST_R0(p,c,d,e,a,b, 8); NST_R0(p,b,c,d,e,a, 9); NST_R0(p,a,b,c,d,e,10); NST_R0(p,e,a,b,c,d,11);
				NST_R0(p,d,e,a,b,c,12); NST_R0(p,c,d,e,a,b,13); NST_R0(p,b,c,d,e,a,14); NST_R0(p,a,b,c,d,e,15);
				NST_R1(p,e,a,b,c,d,16); NST_R1(p,d,e,a,b,c,17); NST_R1(p,c,d,e,a,b,18); NST_R1(p,b,c,d,e,a,19);
				NST_R2(p,a,b,c,d,e,20); NST_R2(p,e,a,b,c,d,21); NST_R2(p,d,e,a,b,c,22); NST_R2(p,c,d,e,a,b,23);
				NST_R2(p,b,c,d,e,a,24); NST_R2(p,a,b,c,d,e,25); NST_R2(p,e,a,b,c,d,26); NST_R2(p,d,e,a,b,c,27);
				NST_R2(p,c,d,e,a,b,28); NST_R2(p,b,c,d,e,a,29); NST_R2(p,a,b,c,d,e,30); NST_R2(p,e,a,b,c,d,31);
				NST_R2(p,d,e,a,b,c,32); NST_R2(p,c,d,e,a,b,33); NST_R2(p,b,c,d,e,a,34); NST_R2(p,a,b,c,d,e,35);
				NST_R2(p,e,a,b,c,d,36); NST_R2(p,d,e,a,b,c,37); NST_R2(p,c,d,e,a,b,38); NST_R2(p,b,c,d,e,a,39);
				NST_R3(p,a,b,c,d,e,40); NST_R3(p,e,a,b,c,d,41); NST_R3(p,d,e,a,b,c,42); NST_R3(p,c,d,e,a,b,43);
				NST_R3(p,b,c,d,e,a,44); NST_R3(p,a,b,c,d,e,45); NST_R3(p,e,a,b,c,d,46); NST_R3(p,d,e,a,b,c,47);
				NST_R3(p,c,d,e,a,b,48); NST_R3(p,b,c,d,e,a,49); NST_R3(p,a,b,c,d,e,50); NST_R3(p,e,a,b,c,d,51);
				NST_R3(p,d,e,a,b,c,52); NST_R3(p,c,d,e,a,b,53); NST_R3(p,b,c,d,e,a,54); NST_R3(p,a,b,c,d,e,55);
				NST_R3(p,e,a,b,c,d,56); NST_R3(p,d,e,a,b,c,57); NST_R3(p,c,d,e,a,b,58); NST_R3(p,b,c,d,e,a,59);
				NST_R4(p,a,b,c,d,e,60); NST_R4(p,e,a,b,c,d,61); NST_R4(p,d,e,a,b,c,62); NST_R4(p,c,d,e,a,b,63);
				NST_R4(p,b,c,d,e,a,64); NST_R4(p,a,b,c,d,e,65); NST_R4(p,e,a,b,c,d,66); NST_R4(p,d,e,a,b,c,67);
				NST_R4(p,c,d,e,a,b,68); NST_R4(p,b,c,d,e,a,69); NST_R4(p,a,b,c,d,e,70); NST_R4(p,e,a,b,c,d,71);
				NST_R4(p,d,e,a,b,c,72); NST_R4(p,c,d,e,a,b,73); NST_R4(p,b,c,d,e,a,74); NST_R4(p,a,b,c,d,e,75);
				NST_R4(p,e,a,b,c,d,76); NST_R4(p,d,e,a,b,c,77); NST_R4(p,c,d,e,a,b,78); NST_R4(p,b,c,d,e,a,79);

				state[0] = (state[0] + a) & 0xFFFFFFFF;
				state[1] = (state[1] + b) & 0xFFFFFFFF;
				state[2] = (state[2] + c) & 0xFFFFFFFF;
				state[3] = (state[3] + d) & 0xFFFFFFFF;
				state[4] = (state[4] + e) & 0xFFFFFFFF;
			}

			#undef NST_ROL
			#undef NST_BLK
			#undef NST_R0
			#undef NST_R1
			#undef NST_R2
			#undef NST_R3
			#undef NST_R4

			void NST_CALL Compute(Key& key,const byte* data,dword length)
			{
				if (length)
					key.Compute( data, length );
			}

			#ifdef NST_MSVC_OPTIMIZE
			#pragma optimize("s", on)
			#endif

			Key::Key()
			{
				Clear();
			}

			void Key::Clear()
			{
				count = 0;
				state[0] = 0x67452301;
				state[1] = 0xEFCDAB89;
				state[2] = 0x98BADCFE;
				state[3] = 0x10325476;
				state[4] = 0xC3D2E1F0;
				finalized = false;
			}

			#ifdef NST_MSVC_OPTIMIZE
			#pragma optimize("", on)
			#endif

			inline void Key::Update() const
			{
				if (!finalized)
					Finalize();
			}

			bool Key::operator == (const Key& key) const
			{
				Update();
				key.Update();

				for (uint i=0; i < 5; ++i)
				{
					if (final[i] != key.final[i])
						return false;
				}

				return true;
			}

			void Key::Compute(const byte* const data,const dword length)
			{
				NST_ASSERT( data && length );

				finalized = false;

				dword i = 0, j = count & 63;

				count += length;

				if (length + j > 63)
				{
					i = 64 - j;

					std::memcpy( buffer+j, data, i );
					Transform( state, buffer );

					for (; i + 63 < length; i += 64)
						Transform( state, data+i );

					j = 0;
				}

				std::memcpy( buffer+j, data+i, length-i );
			}

			void Key::Finalize() const
			{
				NST_ASSERT( !finalized && sizeof(final) == sizeof(state) );

				finalized = true;
				std::memcpy( final, state, sizeof(final) );

				byte end[64+64];

				const uint length = count & 63;
				std::memcpy( end, buffer, length );

				end[length] = 0x80;

				const uint page = (length >= 56 ? 64 : 0);
				std::memset( end+length+1, 0x00, page + (64-1) - length );

				end[page+56] = count >> (56 - 3) & 0xFF;
				end[page+57] = count >> (48 - 3) & 0xFF;
				end[page+58] = count >> (40 - 3) & 0xFF;
				end[page+59] = count >> (32 - 3) & 0xFF;
				end[page+60] = count >> (24 - 3) & 0xFF;
				end[page+61] = count >> (16 - 3) & 0xFF;
				end[page+62] = count >> (8  - 3) & 0xFF;
				end[page+63] = count << (     3) & 0xFF;

				Transform( final, end );

				if (page)
					Transform( final, end+64 );
			}

			Key::Digest Key::GetDigest() const
			{
				Update();
				return final;
			}
		}
	}
}
