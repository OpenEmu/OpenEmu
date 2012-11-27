/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */

#ifndef COMMON_PTR_H
#define COMMON_PTR_H

#include "common/scummsys.h"
#include "common/noncopyable.h"
#include "common/types.h"

namespace Common {

class SharedPtrDeletionInternal {
public:
	virtual ~SharedPtrDeletionInternal() {}
};

template<class T>
class SharedPtrDeletionImpl : public SharedPtrDeletionInternal {
public:
	SharedPtrDeletionImpl(T *ptr) : _ptr(ptr) {}
	~SharedPtrDeletionImpl() {
		// Checks if the supplied type is not just a plain
		// forward definition, taken from boost::checked_delete
		// This makes the user really aware what he tries to do
		// when using this with an incomplete type.
		typedef char completeCheck[sizeof(T) ? 1 : -1];
		(void)sizeof(completeCheck);
		delete _ptr;
	}
private:
	T *_ptr;
};

template<class T, class D>
class SharedPtrDeletionDeleterImpl : public SharedPtrDeletionInternal {
public:
	SharedPtrDeletionDeleterImpl(T *ptr, D d) : _ptr(ptr), _deleter(d) {}
	~SharedPtrDeletionDeleterImpl() { _deleter(_ptr); }
private:
	T *_ptr;
	D _deleter;
};

/**
 * A simple shared pointer implementation modelled after boost.
 *
 * This object keeps track of the assigned pointer and automatically
 * frees it when no more SharedPtr references to it exist.
 *
 * To achieve that the object implements an internal reference counting.
 * Thus you should try to avoid using the plain pointer after assigning
 * it to a SharedPtr object for the first time. If you still use the
 * plain pointer be sure you do not delete it on your own. You may also
 * not use the plain pointer to create a new SharedPtr object, since that
 * would result in a double deletion of the pointer sooner or later.
 *
 * Example creation:
 * Common::SharedPtr<int> pointer(new int(1));
 * would create a pointer to int. Later on usage via *pointer is the same
 * as for a normal pointer. If you need to access the plain pointer value
 * itself later on use the get method. The class also supplies a operator
 * ->, which does the same as the -> operator on a normal pointer.
 *
 * Be sure you are using new to initialize the pointer you want to manage.
 * If you do not use new for allocating, you have to supply a deleter as
 * second parameter when creating a SharedPtr object. The deleter has to
 * implement operator() which takes the pointer it should free as argument.
 *
 * Note that you have to specify the type itself not the pointer type as
 * template parameter.
 *
 * When creating a SharedPtr object from a normal pointer you need a real
 * definition of the type you want SharedPtr to manage, a simple forward
 * definition is not enough.
 *
 * The class has implicit upcast support, so if you got a class B derived
 * from class A, you can assign a pointer to B without any problems to a
 * SharedPtr object with template parameter A. The very same applies to
 * assignment of a SharedPtr<B> object to a SharedPtr<A> object.
 *
 * There are also operators != and == to compare two SharedPtr objects
 * with compatible pointers. Comparison between a SharedPtr object and
 * a plain pointer is only possible via SharedPtr::get.
 */
template<class T>
class SharedPtr {
#if !defined(__GNUC__) || GCC_ATLEAST(3, 0)
	template<class T2> friend class SharedPtr;
#endif
public:
	typedef int RefValue;
	typedef T ValueType;
	typedef T *PointerType;
	typedef T &ReferenceType;

	SharedPtr() : _refCount(0), _deletion(0), _pointer(0) {}

	template<class T2>
	explicit SharedPtr(T2 *p) : _refCount(new RefValue(1)), _deletion(new SharedPtrDeletionImpl<T2>(p)), _pointer(p) {}

	template<class T2, class D>
	SharedPtr(T2 *p, D d) : _refCount(new RefValue(1)), _deletion(new SharedPtrDeletionDeleterImpl<T2, D>(p, d)), _pointer(p) {}

	SharedPtr(const SharedPtr &r) : _refCount(r._refCount), _deletion(r._deletion), _pointer(r._pointer) { if (_refCount) ++(*_refCount); }
	template<class T2>
	SharedPtr(const SharedPtr<T2> &r) : _refCount(r._refCount), _deletion(r._deletion), _pointer(r._pointer) { if (_refCount) ++(*_refCount); }

	~SharedPtr() { decRef(); }

	SharedPtr &operator=(const SharedPtr &r) {
		if (r._refCount)
			++(*r._refCount);
		decRef();

		_refCount = r._refCount;
		_deletion = r._deletion;
		_pointer = r._pointer;

		return *this;
	}

	template<class T2>
	SharedPtr &operator=(const SharedPtr<T2> &r) {
		if (r._refCount)
			++(*r._refCount);
		decRef();

		_refCount = r._refCount;
		_deletion = r._deletion;
		_pointer = r._pointer;

		return *this;
	}

	ReferenceType operator*() const { assert(_pointer); return *_pointer; }
	PointerType operator->() const { assert(_pointer); return _pointer; }

	/**
	 * Returns the plain pointer value. Be sure you know what you
	 * do if you are continuing to use that pointer.
	 *
	 * @return the pointer the SharedPtr object manages
	 */
	PointerType get() const { return _pointer; }

	/**
	 * Implicit conversion operator to bool for convenience, to make
	 * checks like "if (sharedPtr) ..." possible.
	 */
	operator bool() const { return _pointer != 0; }

	/**
	 * Checks if the SharedPtr object is the only object refering
	 * to the assigned pointer. This should just be used for
	 * debugging purposes.
	 */
	bool unique() const { return refCount() == 1; }

	/**
	 * Resets the SharedPtr object to a NULL pointer.
	 */
	void reset() {
		decRef();
		_deletion = 0;
		_refCount = 0;
		_pointer = 0;
	}

	template<class T2>
	bool operator==(const SharedPtr<T2> &r) const {
		return _pointer == r.get();
	}

	template<class T2>
	bool operator!=(const SharedPtr<T2> &r) const {
		return _pointer != r.get();
	}

	/**
	 * Returns the number of references to the assigned pointer.
	 * This should just be used for debugging purposes.
	 */
	RefValue refCount() const { return _refCount ? *_refCount : 0; }
#if !defined(__GNUC__) || GCC_ATLEAST(3, 0)
private:
#endif
	void decRef() {
		if (_refCount) {
			--(*_refCount);
			if (!*_refCount) {
				delete _refCount;
				delete _deletion;
				_deletion = 0;
				_refCount = 0;
				_pointer = 0;
			}
		}
	}

	RefValue *_refCount;
	SharedPtrDeletionInternal *_deletion;
	PointerType _pointer;
};

template<typename T>
class ScopedPtr : NonCopyable {
public:
	typedef T ValueType;
	typedef T *PointerType;
	typedef T &ReferenceType;

	explicit ScopedPtr(PointerType o = 0) : _pointer(o) {}

	ReferenceType operator*() const { return *_pointer; }
	PointerType operator->() const { return _pointer; }

	/**
	 * Implicit conversion operator to bool for convenience, to make
	 * checks like "if (scopedPtr) ..." possible.
	 */
	operator bool() const { return _pointer != 0; }

	~ScopedPtr() {
		delete _pointer;
	}

	/**
	 * Resets the pointer with the new value. Old object will be destroyed
	 */
	void reset(PointerType o = 0) {
		delete _pointer;
		_pointer = o;
	}

	/**
	 * Returns the plain pointer value.
	 *
	 * @return the pointer the ScopedPtr manages
	 */
	PointerType get() const { return _pointer; }

	/**
	 * Returns the plain pointer value and releases ScopedPtr.
	 * After release() call you need to delete object yourself
	 *
	 * @return the pointer the ScopedPtr manages
	 */
	PointerType release() {
		PointerType r = _pointer;
		_pointer = 0;
		return r;
	}

private:
	PointerType _pointer;
};


template<typename T>
class DisposablePtr : NonCopyable {
public:
	typedef T  ValueType;
	typedef T *PointerType;
	typedef T &ReferenceType;

	explicit DisposablePtr(PointerType o, DisposeAfterUse::Flag dispose) : _pointer(o), _dispose(dispose) {}

	~DisposablePtr() {
		if (_dispose) delete _pointer;
	}

	ReferenceType operator*() const { return *_pointer; }
	PointerType operator->() const { return _pointer; }

	/**
	 * Implicit conversion operator to bool for convenience, to make
	 * checks like "if (scopedPtr) ..." possible.
	 */
	operator bool() const { return _pointer; }

	/**
	 * Returns the plain pointer value.
	 *
	 * @return the pointer the DisposablePtr manages
	 */
	PointerType get() const { return _pointer; }

private:
	PointerType           _pointer;
	DisposeAfterUse::Flag _dispose;
};

} // End of namespace Common

#endif
