//============================================================================
//
//   SSSS    tt          lll  lll       
//  SS  SS   tt           ll   ll        
//  SS     tttttt  eeee   ll   ll   aaaa 
//   SSSS    tt   ee  ee  ll   ll      aa
//      SS   tt   eeeeee  ll   ll   aaaaa  --  "An Atari 2600 VCS Emulator"
//  SS  SS   tt   ee      ll   ll  aa  aa
//   SSSS     ttt  eeeee llll llll  aaaaa
//
// Copyright (c) 1995-2011 by Bradford W. Mott, Stephen Anthony
// and the Stella Team
//
// See the file "License.txt" for information on usage and redistribution of
// this file, and for a DISCLAIMER OF ALL WARRANTIES.
//
// $Id: SharedPtr.hxx 2199 2011-01-01 16:04:32Z stephena $
//
//   Based on code from ScummVM - Scumm Interpreter
//   Copyright (C) 2002-2004 The ScummVM project
//============================================================================

#ifndef SHARED_PTR_HXX
#define SHARED_PTR_HXX

#include <cassert>

namespace Common {

class SharedPtrDeletionInternal
{
  public:
    virtual ~SharedPtrDeletionInternal() {}
};

template<class T>
class SharedPtrDeletionImpl : public SharedPtrDeletionInternal
{
  public:
    SharedPtrDeletionImpl(T *ptr) : _ptr(ptr) {}
    ~SharedPtrDeletionImpl()
    {
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
class SharedPtrDeletionDeleterImpl : public SharedPtrDeletionInternal
{
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
class SharedPtr
{
#if !((__GNUC__ == 2) && (__GNUC_MINOR__ >= 95))
  template<class T2> friend class SharedPtr;
#endif
  public:
    typedef int RefValue;
    typedef T ValueType;
    typedef T *Pointer;

    SharedPtr() : _refCount(0), _deletion(0), _pointer(0) {}
    template<class T2> explicit SharedPtr(T2 *p) : _refCount(new RefValue(1)), _deletion(new SharedPtrDeletionImpl<T2>(p)), _pointer(p) {}
    template<class T2, class D> SharedPtr(T2 *p, D d) : _refCount(new RefValue(1)), _deletion(new SharedPtrDeletionDeleterImpl<T2, D>(p, d)), _pointer(p) {}

    SharedPtr(const SharedPtr &r) : _refCount(r._refCount), _deletion(r._deletion), _pointer(r._pointer) { if (_refCount) ++(*_refCount); }
    template<class T2> SharedPtr(const SharedPtr<T2> &r) : _refCount(r._refCount), _deletion(r._deletion), _pointer(r._pointer) { if (_refCount) ++(*_refCount); }

    ~SharedPtr() { decRef(); }

    SharedPtr &operator =(const SharedPtr &r)
    {
      if (r._refCount)
        ++(*r._refCount);
      decRef();

      _refCount = r._refCount;
      _deletion = r._deletion;
      _pointer = r._pointer;

      return *this;
    }

    template<class T2>
    SharedPtr &operator =(const SharedPtr<T2> &r)
    {
      if (r._refCount)
        ++(*r._refCount);
      decRef();

      _refCount = r._refCount;
      _deletion = r._deletion;
      _pointer = r._pointer;

      return *this;
    }

    ValueType &operator *() const { assert(_pointer); return *_pointer; }
    Pointer operator ->() const { assert(_pointer); return _pointer; }

    /**
     * Returns the plain pointer value. Be sure you know what you
     * do if you are continuing to use that pointer.
     *
     * @return the pointer the SharedPtr object manages
     */
    Pointer get() const { return _pointer; }

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
     * Returns the number of references to the assigned pointer.
     * This should just be used for debugging purposes.
     */
    RefValue refCount() const { return _refCount ? *_refCount : 0; }
#if !((__GNUC__ == 2) && (__GNUC_MINOR__ >= 95))
  private:
#endif
    void decRef()
    {
      if (_refCount)
      {
        --(*_refCount);
        if (!*_refCount)
        {
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
    T *_pointer;
};

} // end of namespace Common

template<class T1, class T2>
bool operator ==(const Common::SharedPtr<T1> &l, const Common::SharedPtr<T2> &r) {
  return l.get() == r.get();
}

template<class T1, class T2>
bool operator !=(const Common::SharedPtr<T1> &l, const Common::SharedPtr<T2> &r) {
  return l.get() != r.get();
}

#endif
