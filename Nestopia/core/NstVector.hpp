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

#ifndef NST_VECTOR_H
#define NST_VECTOR_H

#ifndef NST_ASSERT_H
#include "NstAssert.hpp"
#endif

#ifdef NST_PRAGMA_ONCE
#pragma once
#endif

namespace Nes
{
	namespace Core
	{
		template<typename T>
		class Vector;

		template<>
		class Vector<void>
		{
		public:

			static void* Malloc(dword);
			static void* Realloc(void*,dword);
			static void  Free(void*);
			static void  Copy(void*,const void*,dword);
			static void  Move(void*,const void*,dword);

			template<typename T>
			static void Copy(T& dst,const T& src)
			{
				Copy( &dst, &src, sizeof(T) );
			}
		};

		template<> inline void Vector<void>::Copy(char&   dst,const char&   src) { dst = src; }
		template<> inline void Vector<void>::Copy(schar&  dst,const schar&  src) { dst = src; }
		template<> inline void Vector<void>::Copy(uchar&  dst,const uchar&  src) { dst = src; }
		template<> inline void Vector<void>::Copy(short&  dst,const short&  src) { dst = src; }
		template<> inline void Vector<void>::Copy(ushort& dst,const ushort& src) { dst = src; }
		template<> inline void Vector<void>::Copy(int&    dst,const int&    src) { dst = src; }
		template<> inline void Vector<void>::Copy(uint&   dst,const uint&   src) { dst = src; }
		template<> inline void Vector<void>::Copy(long&   dst,const long&   src) { dst = src; }
		template<> inline void Vector<void>::Copy(ulong&  dst,const ulong&  src) { dst = src; }

		template<typename T>
		class Vector
		{
			T* data;
			dword size;
			dword capacity;

			void MakeRoom(dword);

			typedef Vector<void> Allocator;

		public:

			typedef T Type;

			Vector();
			explicit Vector(dword);
			Vector(const Vector<T>&);
			Vector(const T*,dword);

			bool operator == (const Vector<T>&) const;

			void Reserve(dword);
			void Resize(dword);
			void Expand(dword);
			void Assign(const T*,dword);
			void Append(const T&);
			void Append(const T*,dword);
			T*   Insert(T*,const T&);
			void Erase(T*,dword=1);
			void Destroy();
			void Defrag();

			static void Swap(Vector<T>&,Vector<T>&);

			~Vector()
			{
				Allocator::Free( data );
			}

			void operator = (const Vector<T>& vector)
			{
				Assign( vector.data, vector.size );
			}

			void operator += (const Vector<T>& vector)
			{
				Append( vector.data, vector.size );
			}

			T& operator [] (dword i) const
			{
				NST_ASSERT( i < size );
				return data[i];
			}

			T* Begin() const
			{
				return data;
			}

			T* End() const
			{
				return data + size;
			}

			dword Size() const
			{
				return size;
			}

			dword Capacity() const
			{
				return capacity;
			}

			T& Front() const
			{
				NST_ASSERT( size );
				return data[0];
			}

			T& Back() const
			{
				NST_ASSERT( size );
				return data[size - 1];
			}

			const T& Pop()
			{
				NST_ASSERT( size );
				return data[--size];
			}

			void SetTo(dword count)
			{
				NST_ASSERT( count <= capacity );
				size = count;
			}

			void Clear()
			{
				size = 0;
			}
		};

		template<typename T>
		Vector<T>::Vector()
		: data(NULL), size(0), capacity(0) {}

		template<typename T>
		Vector<T>::Vector(const dword count)
		: data(count ? static_cast<T*>(Allocator::Malloc(count * sizeof(T))) : NULL), size(count), capacity(count)
		{
		}

		template<typename T>
		Vector<T>::Vector(const T* in,const dword count)
		: data(count ? static_cast<T*>(Allocator::Malloc(count * sizeof(T))) : NULL), size(count), capacity(count)
		{
			Allocator::Copy( data, in, count * sizeof(T) );
		}

		template<typename T>
		Vector<T>::Vector(const Vector<T>& v)
		: data(v.size ? static_cast<T*>(Allocator::Malloc(v.size * sizeof(T))) : NULL), size(v.size), capacity(v.size)
		{
			Allocator::Copy( data, v.data, v.size * sizeof(T) );
		}

		template<typename T>
		void Vector<T>::MakeRoom(const dword count)
		{
			NST_ASSERT( count );
			data = static_cast<T*>(Allocator::Realloc( data, count * sizeof(T) ));
			capacity = count;
		}

		template<typename T>
		void Vector<T>::Append(const T& value)
		{
			if (size == capacity)
				MakeRoom( (size + 1) * 2 );

			Allocator::Copy( data[size++], value );
		}

		template<typename T>
		void Vector<T>::Assign(const T* const NST_RESTRICT inData,const dword inSize)
		{
			if (capacity < inSize)
				MakeRoom( inSize );

			Allocator::Copy( data, inData, (size=inSize) * sizeof(T) );
		}

		template<typename T>
		void Vector<T>::Append(const T* const NST_RESTRICT inData,const dword inSize)
		{
			if (capacity < size + inSize)
				MakeRoom( (size * 2) + inSize );

			void* const tmp = data + size;
			size += inSize;

			Allocator::Copy( tmp, inData, inSize * sizeof(T) );
		}

		template<typename T>
		T* Vector<T>::Insert(T* it,const T& value)
		{
			const dword pos = it - data;

			if (size++ == capacity)
				MakeRoom( size * 2 );

			Allocator::Move( data+pos+1, data+pos, (size - (pos+1)) * sizeof(T) );
			Allocator::Copy( data[pos], value );

			return data+pos;
		}

		template<typename T>
		void Vector<T>::Erase(T* it,dword count)
		{
			NST_ASSERT( size >= count );

			const dword s = size;
			size -= count;
			Allocator::Move( it, it + count, (s - ((it-data) + count)) * sizeof(T) );
		}

		template<typename T>
		void Vector<T>::Reserve(dword count)
		{
			if (capacity < count)
				MakeRoom( count );
		}

		template<typename T>
		void Vector<T>::Resize(dword count)
		{
			Reserve( count );
			size = count;
		}

		template<typename T>
		void Vector<T>::Expand(dword count)
		{
			Reserve( size + count );
			size += count;
		}

		template<typename T>
		void Vector<T>::Defrag()
		{
			if (size)
			{
				MakeRoom( size );
			}
			else if (void* const tmp = data)
			{
				data = NULL;
				capacity = 0;
				Allocator::Free( tmp );
			}
		}

		template<typename T>
		bool Vector<T>::operator == (const Vector<T>& vector) const
		{
			if (size != vector.size)
				return false;

			for (const T *a=data, *b=vector.data, *const end=data+size; a != end; ++a, ++b)
			{
				if (!(*a == *b))
					return false;
			}

			return true;
		}

		template<typename T>
		void Vector<T>::Destroy()
		{
			if (void* const tmp = data)
			{
				data = NULL;
				size = 0;
				capacity = 0;
				Allocator::Free( tmp );
			}
		}

		template<typename T>
		void Vector<T>::Swap(Vector<T>& a,Vector<T>& b)
		{
			T* t = a.data;
			a.data = b.data;
			b.data = t;
			dword u = a.size;
			a.size = b.size;
			b.size = u;
			u = a.capacity;
			a.capacity = b.capacity;
			b.capacity = u;
		}
	}
}

#endif
