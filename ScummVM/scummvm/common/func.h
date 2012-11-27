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

#ifndef COMMON_FUNC_H
#define COMMON_FUNC_H

#include "common/scummsys.h"

namespace Common {

/**
 * Generic unary function.
 */
template<class Arg, class Result>
struct UnaryFunction {
	typedef Arg ArgumenType;
	typedef Result ResultType;
};

/**
 * Generic binary function.
 */
template<class Arg1, class Arg2, class Result>
struct BinaryFunction {
	typedef Arg1 FirstArgumentType;
	typedef Arg2 SecondArgumentType;
	typedef Result ResultType;
};

/**
 * Predicate to check for equallity of two data elements.
 */
template<class T>
struct EqualTo : public BinaryFunction<T, T, bool> {
	bool operator()(const T &x, const T &y) const { return x == y; }
};

/**
 * Predicate to check for x being less than y.
 */
template<class T>
struct Less : public BinaryFunction<T, T, bool> {
	bool operator()(const T &x, const T &y) const { return x < y; }
};

/**
 * Predicate to check for x being greater than y.
 */
template<class T>
struct Greater : public BinaryFunction<T, T, bool> {
	bool operator()(const T &x, const T &y) const { return x > y; }
};

template<class Op>
class Binder1st : public UnaryFunction<typename Op::SecondArgumentType, typename Op::ResultType> {
private:
	Op _op;
	typename Op::FirstArgumentType _arg1;
public:
	Binder1st(const Op &op, typename Op::FirstArgumentType arg1) : _op(op), _arg1(arg1) {}

	typename Op::ResultType operator()(typename Op::SecondArgumentType v) const {
		return _op(_arg1, v);
	}
};

/**
 * Transforms a binary function object into an unary function object.
 * To achieve that the first parameter is bound to the passed value t.
 */
template<class Op>
inline Binder1st<Op> bind1st(const Op &op, typename Op::FirstArgumentType t) {
	return Binder1st<Op>(op, t);
}

template<class Op>
class Binder2nd : public UnaryFunction<typename Op::FirstArgumentType, typename Op::ResultType> {
private:
	Op _op;
	typename Op::SecondArgumentType _arg2;
public:
	Binder2nd(const Op &op, typename Op::SecondArgumentType arg2) : _op(op), _arg2(arg2) {}

	typename Op::ResultType operator()(typename Op::FirstArgumentType v) const {
		return _op(v, _arg2);
	}
};

/**
 * Transforms a binary function object into an unary function object.
 * To achieve that the first parameter is bound to the passed value t.
 */
template<class Op>
inline Binder2nd<Op> bind2nd(const Op &op, typename Op::SecondArgumentType t) {
	return Binder2nd<Op>(op, t);
}

template<class Arg, class Result>
class PointerToUnaryFunc : public UnaryFunction<Arg, Result> {
private:
	Result (*_func)(Arg);
public:
	typedef Result (*FuncType)(Arg);

	PointerToUnaryFunc(const FuncType &func) : _func(func) {}
	Result operator()(Arg v) const {
		return _func(v);
	}
};

template<class Arg1, class Arg2, class Result>
class PointerToBinaryFunc : public BinaryFunction<Arg1, Arg2, Result> {
private:
	Result (*_func)(Arg1, Arg2);
public:
	typedef Result (*FuncType)(Arg1, Arg2);

	PointerToBinaryFunc(const FuncType &func) : _func(func) {}
	Result operator()(Arg1 v1, Arg2 v2) const {
		return _func(v1, v2);
	}
};

/**
 * Creates an unary function object from a function pointer.
 */
template<class Arg, class Result>
inline PointerToUnaryFunc<Arg, Result> ptr_fun(Result (*func)(Arg)) {
	return PointerToUnaryFunc<Arg, Result>(func);
}

/**
 * Creates an binary function object from a function pointer.
 */
template<class Arg1, class Arg2, class Result>
inline PointerToBinaryFunc<Arg1, Arg2, Result> ptr_fun(Result (*func)(Arg1, Arg2)) {
	return PointerToBinaryFunc<Arg1, Arg2, Result>(func);
}

template<class Result, class T>
class MemFunc0 : public UnaryFunction<T *, Result> {
private:
	Result (T::*_func)();
public:
	typedef Result (T::*FuncType)();

	MemFunc0(const FuncType &func) : _func(func) {}
	Result operator()(T *v) const {
		return (v->*_func)();
	}
};

template<class Result, class T>
class ConstMemFunc0 : public UnaryFunction<T *, Result> {
private:
	Result (T::*_func)() const;
public:
	typedef Result (T::*FuncType)() const;

	ConstMemFunc0(const FuncType &func) : _func(func) {}
	Result operator()(const T *v) const {
		return (v->*_func)();
	}
};

template<class Result, class Arg, class T>
class MemFunc1 : public BinaryFunction<T *, Arg, Result> {
private:
	Result (T::*_func)(Arg);
public:
	typedef Result (T::*FuncType)(Arg);

	MemFunc1(const FuncType &func) : _func(func) {}
	Result operator()(T *v1, Arg v2) const {
		return (v1->*_func)(v2);
	}
};

template<class Result, class Arg, class T>
class ConstMemFunc1 : public BinaryFunction<T *, Arg, Result> {
private:
	Result (T::*_func)(Arg) const;
public:
	typedef Result (T::*FuncType)(Arg) const;

	ConstMemFunc1(const FuncType &func) : _func(func) {}
	Result operator()(const T *v1, Arg v2) const {
		return (v1->*_func)(v2);
	}
};

/**
 * Creates a unary function object from a class member function pointer.
 * The parameter passed to the function object is the 'this' pointer to
 * be used for the function call.
 */
template<class Result, class T>
inline MemFunc0<Result, T> mem_fun(Result (T::*f)()) {
	return MemFunc0<Result, T>(f);
}

/**
 * Creates a unary function object from a class member function pointer.
 * The parameter passed to the function object is the 'this' pointer to
 * be used for the function call.
 */
template<class Result, class T>
inline ConstMemFunc0<Result, T> mem_fun(Result (T::*f)() const) {
	return ConstMemFunc0<Result, T>(f);
}

/**
 * Creates a binary function object from a class member function pointer.
 * The first parameter passed to the function object is the 'this' pointer to
 * be used for the function call.
 * The second one is the parameter passed to the member function.
 */
template<class Result, class Arg, class T>
inline MemFunc1<Result, Arg, T> mem_fun(Result (T::*f)(Arg)) {
	return MemFunc1<Result, Arg, T>(f);
}

/**
 * Creates a binary function object from a class member function pointer.
 * The first parameter passed to the function object is the 'this' pointer to
 * be used for the function call.
 * The second one is the parameter passed to the member function.
 */
template<class Result, class Arg, class T>
inline ConstMemFunc1<Result, Arg, T> mem_fun(Result (T::*f)(Arg) const) {
	return ConstMemFunc1<Result, Arg, T>(f);
}

template<class Result, class T>
class MemFuncRef0 : public UnaryFunction<T &, Result> {
private:
	Result (T::*_func)();
public:
	typedef Result (T::*FuncType)();

	MemFuncRef0(const FuncType &func) : _func(func) {}
	Result operator()(T &v) const {
		return (v.*_func)();
	}
};

template<class Result, class T>
class ConstMemFuncRef0 : public UnaryFunction<T &, Result> {
private:
	Result (T::*_func)() const;
public:
	typedef Result (T::*FuncType)() const;

	ConstMemFuncRef0(const FuncType &func) : _func(func) {}
	Result operator()(const T &v) const {
		return (v.*_func)();
	}
};

template<class Result, class Arg, class T>
class MemFuncRef1 : public BinaryFunction<T &, Arg, Result> {
private:
	Result (T::*_func)(Arg);
public:
	typedef Result (T::*FuncType)(Arg);

	MemFuncRef1(const FuncType &func) : _func(func) {}
	Result operator()(T &v1, Arg v2) const {
		return (v1.*_func)(v2);
	}
};

template<class Result, class Arg, class T>
class ConstMemFuncRef1 : public BinaryFunction<T &, Arg, Result> {
private:
	Result (T::*_func)(Arg) const;
public:
	typedef Result (T::*FuncType)(Arg) const;

	ConstMemFuncRef1(const FuncType &func) : _func(func) {}
	Result operator()(const T &v1, Arg v2) const {
		return (v1.*_func)(v2);
	}
};

/**
 * Creates a unary function object from a class member function pointer.
 * The parameter passed to the function object is the object instance to
 * be used for the function call. Note unlike mem_fun, it takes a reference
 * as parameter. Note unlike mem_fun, it takes a reference
 * as parameter.
 */
template<class Result, class T>
inline MemFuncRef0<Result, T> mem_fun_ref(Result (T::*f)()) {
	return MemFuncRef0<Result, T>(f);
}

/**
 * Creates a unary function object from a class member function pointer.
 * The parameter passed to the function object is the object instance to
 * be used for the function call. Note unlike mem_fun, it takes a reference
 * as parameter.
 */
template<class Result, class T>
inline ConstMemFuncRef0<Result, T> mem_fun_Ref(Result (T::*f)() const) {
	return ConstMemFuncRef0<Result, T>(f);
}

/**
 * Creates a binary function object from a class member function pointer.
 * The first parameter passed to the function object is the object instance to
 * be used for the function call. Note unlike mem_fun, it takes a reference
 * as parameter.
 * The second one is the parameter passed to the member function.
 */
template<class Result, class Arg, class T>
inline MemFuncRef1<Result, Arg, T> mem_fun_ref(Result (T::*f)(Arg)) {
	return MemFuncRef1<Result, Arg, T>(f);
}

/**
 * Creates a binary function object from a class member function pointer.
 * The first parameter passed to the function object is the object instance to
 * be used for the function call. Note unlike mem_fun, it takes a reference
 * as parameter.
 * The second one is the parameter passed to the member function.
 */
template<class Result, class Arg, class T>
inline ConstMemFuncRef1<Result, Arg, T> mem_fun_ref(Result (T::*f)(Arg) const) {
	return ConstMemFuncRef1<Result, Arg, T>(f);
}

// functor code

/**
 * Generic functor object for function objects without parameters.
 *
 * @see Functor1
 */
template<class Res>
struct Functor0 {
	virtual ~Functor0() {}

	virtual bool isValid() const = 0;
	virtual Res operator()() const = 0;
};

/**
 * Functor object for a class member function without parameter.
 *
 * Example creation:
 *
 * Foo bar;
 * Functor0Mem<void, Foo> myFunctor(&bar, &Foo::myFunc);
 *
 * Example usage:
 *
 * myFunctor();
 */
template<class Res, class T>
class Functor0Mem : public Functor0<Res> {
public:
	typedef Res (T::*FuncType)();

	Functor0Mem(T *t, const FuncType &func) : _t(t), _func(func) {}

	bool isValid() const { return _func != 0 && _t != 0; }
	Res operator()() const {
		return (_t->*_func)();
	}
private:
	mutable T *_t;
	const FuncType _func;
};

/**
 * Generic functor object for unary function objects.
 *
 * A typical usage for an unary function object is for executing opcodes
 * in a script interpreter. To achieve that one can create an Common::Array
 * object with 'Functor1<Arg, Res> *' as type. Now after the right engine version
 * has been determined and the opcode table to use is found one could easily
 * add the opcode implementations like this:
 *
 * Common::Array<Functor1<ScriptState, void> *> opcodeTable;
 * opcodeTable[0] = new Functor1Mem<ScriptState, void, MyEngine_v1>(&myEngine, &MyEngine_v1::o1_foo);
 * opcodeTable[1] = new Functor1Mem<ScriptState, void, MyEngine_v2>(&myEngine, &MyEngine_v2::o2_foo);
 * // unimplemented/unused opcode
 * opcodeTable[2] = 0;
 * etc.
 *
 * This makes it easy to add member functions of different classes as
 * opcode functions to the function table. Since with the generic
 * Functor1<ScriptState, void> object the only requirement for an
 * function to be used is 'ScriptState' as argument and 'void' as return
 * value.
 *
 * Now for calling the opcodes one has simple to do:
 * if (opcodeTable[opcodeNum] && opcodeTable[opcodeNum]->isValid())
 *     (*opcodeTable[opcodeNum])(scriptState);
 * else
 *     warning("Unimplemented opcode %d", opcodeNum);
 *
 * If you want to see an real world example check the kyra engine.
 * Files: engines/kyra/script.cpp and .h and engines/kyra/script_*.cpp
 * are interesting for that matter.
 */
template<class Arg, class Res>
struct Functor1 : public UnaryFunction<Arg, Res> {
	virtual ~Functor1() {}

	virtual bool isValid() const = 0;
	virtual Res operator()(Arg) const = 0;
};

/**
 * Functor object for an unary class member function.
 * Usage is like with Functor0Mem. The resulting functor object
 * will take one parameter though.
 *
 * @see Functor0Mem
 */
template<class Arg, class Res, class T>
class Functor1Mem : public Functor1<Arg, Res> {
public:
	typedef Res (T::*FuncType)(Arg);

	Functor1Mem(T *t, const FuncType &func) : _t(t), _func(func) {}

	bool isValid() const { return _func != 0 && _t != 0; }
	Res operator()(Arg v1) const {
		return (_t->*_func)(v1);
	}
private:
	mutable T *_t;
	const FuncType _func;
};

/**
 * Generic functor object for binary function objects.
 *
 * @see Functor1
 */
template<class Arg1, class Arg2, class Res>
struct Functor2 : public BinaryFunction<Arg1, Arg2, Res> {
	virtual ~Functor2() {}

	virtual bool isValid() const = 0;
	virtual Res operator()(Arg1, Arg2) const = 0;
};

/**
 * Functor object for a binary function.
 *
 * @see Functor2Mem
 */
template<class Arg1, class Arg2, class Res>
class Functor2Fun : public Functor2<Arg1, Arg2, Res> {
public:
	typedef Res (*FuncType)(Arg1, Arg2);

	Functor2Fun(const FuncType func) : _func(func) {}

	bool isValid() const { return _func != 0; }
	Res operator()(Arg1 v1, Arg2 v2) const {
		return (*_func)(v1, v2);
	}
private:
	const FuncType _func;
};

/**
 * Functor object for a binary class member function.
 * Usage is like with Functor0Mem. The resulting functor object
 * will take two parameter though.
 *
 * @see Functor0Mem
 */
template<class Arg1, class Arg2, class Res, class T>
class Functor2Mem : public Functor2<Arg1, Arg2, Res> {
public:
	typedef Res (T::*FuncType)(Arg1, Arg2);

	Functor2Mem(T *t, const FuncType &func) : _t(t), _func(func) {}

	bool isValid() const { return _func != 0 && _t != 0; }
	Res operator()(Arg1 v1, Arg2 v2) const {
		return (_t->*_func)(v1, v2);
	}
private:
	mutable T *_t;
	const FuncType _func;
};

/**
 * Base template for hash functor objects, used by HashMap.
 * This needs to be specialized for every type that you need to hash.
 */
template<typename T> struct Hash;


#define GENERATE_TRIVIAL_HASH_FUNCTOR(T) \
	template<> struct Hash<T> : public UnaryFunction<T, uint> { \
		uint operator()(T val) const { return (uint)val; } \
	}

GENERATE_TRIVIAL_HASH_FUNCTOR(bool);
GENERATE_TRIVIAL_HASH_FUNCTOR(char);
GENERATE_TRIVIAL_HASH_FUNCTOR(signed char);
GENERATE_TRIVIAL_HASH_FUNCTOR(unsigned char);
GENERATE_TRIVIAL_HASH_FUNCTOR(short);
GENERATE_TRIVIAL_HASH_FUNCTOR(int);
GENERATE_TRIVIAL_HASH_FUNCTOR(long);
GENERATE_TRIVIAL_HASH_FUNCTOR(unsigned short);
GENERATE_TRIVIAL_HASH_FUNCTOR(unsigned int);
GENERATE_TRIVIAL_HASH_FUNCTOR(unsigned long);

#undef GENERATE_TRIVIAL_HASH_FUNCTOR

}	// End of namespace Common

#endif
