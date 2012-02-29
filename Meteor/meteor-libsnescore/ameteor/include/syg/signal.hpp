#ifndef SYG_connection_HPP
#define SYG_connection_HPP

#include <list>

namespace syg
{

//------------------------------
// connections
//------------------------------

template <typename Tret>
class connection_base
{
	public:
		virtual Tret call() const = 0;
		virtual connection_base<Tret>* clone() const = 0;
		virtual ~connection_base() {}
};

template <typename Tret, typename Targ0>
class connection_base1
{
	public:
		virtual Tret call(Targ0) const = 0;
		virtual connection_base1<Tret, Targ0>* clone() const = 0;
		virtual ~connection_base1() {}
};

template <typename Tret>
class connection_func : public connection_base<Tret>
{
	public:
		typedef Tret (*FuncPtr)();

		connection_func(FuncPtr ptr):
			_func(ptr)
		{}

		Tret call() const
		{
			return _func();
		}

		connection_base<Tret>* clone() const
		{
			return new connection_func<Tret>(*this);
		}

	private:
		FuncPtr _func;
};

template <typename Tret, typename Targ0>
class connection_func1 : public connection_base1<Tret, Targ0>
{
	public:
		typedef Tret (*FuncPtr)(Targ0);

		connection_func1(FuncPtr ptr):
			_func(ptr)
		{}

		Tret call(Targ0 arg0) const
		{
			return _func(arg0);
		}

		connection_base1<Tret, Targ0>* clone() const
		{
			return new connection_func1<Tret, Targ0>(*this);
		}

	private:
		FuncPtr _func;
};

template <typename Tobj, typename Tret>
class connection_meth : public connection_base<Tret>
{
	public:
		typedef Tobj TypeObj;
		typedef Tret (Tobj::*FuncPtr)();

		connection_meth(TypeObj& obj, FuncPtr ptr):
			_obj(obj),
			_meth(ptr)
		{}

		Tret call() const
		{
			return (_obj.*_meth)();
		}

		connection_base<Tret>* clone() const
		{
			return new connection_meth<Tobj, Tret>(*this);
		}

	private:
		Tobj& _obj;
		FuncPtr _meth;
};

template <typename Tobj, typename Tret, typename Targ0>
class connection_meth1 : public connection_base1<Tret, Targ0>
{
	public:
		typedef Tobj TypeObj;
		typedef Tret (Tobj::*FuncPtr)(Targ0);

		connection_meth1(TypeObj& obj, FuncPtr ptr):
			_obj(obj),
			_meth(ptr)
		{}

		Tret call(Targ0 arg0) const
		{
			return (_obj.*_meth)(arg0);
		}

		connection_base1<Tret, Targ0>* clone() const
		{
			return new connection_meth1<Tobj, Tret, Targ0>(*this);
		}

	private:
		Tobj& _obj;
		FuncPtr _meth;
};

//------------------------------
// slots
//------------------------------

template <typename Tret>
class slot;

template <typename Tret, typename Targ0>
class slot1;

template <typename Tret>
slot<Tret> ptr_fun(Tret (*fun)());

template <typename Tobj, typename Tret>
slot<Tret> mem_fun(Tobj& obj, Tret (Tobj::*fun)());

template <typename Tret, typename Targ0>
slot1<Tret, Targ0> ptr_fun(Tret (*fun)(Targ0));

template <typename Tobj, typename Tret, typename Targ0>
slot1<Tret, Targ0> mem_fun(Tobj& obj, Tret (Tobj::*fun)(Targ0));

template <typename Tret>
class slot
{
	public:
		slot():
			_conn(0)
		{}
		slot(const slot<Tret>& s)
		{
			*this = s;
		}
		~slot()
		{
			delete _conn;
		}

		slot& operator=(const slot<Tret>& s)
		{
			if (s._conn)
				_conn = s._conn->clone();
			else
				_conn = 0;

			return *this;
		}

		Tret call() const
		{
			return _conn->call();
		}

		Tret operator()() const
		{
			return call();
		}

		operator bool() const
		{
			return _conn;
		}

	private:
		connection_base<Tret>* _conn;

		slot(const connection_base<Tret>& conn):
			_conn(conn.clone())
		{}

		friend slot<Tret> ptr_fun<Tret>(Tret (*fun)());

		template <typename Tobj2, typename Tret2>
		friend slot<Tret2> mem_fun(Tobj2& obj, Tret2 (Tobj2::*fun)());
};

template <typename Tret, typename Targ0>
class slot1
{
	public:
		typedef slot1<Tret, Targ0> SlotType;

		slot1():
			_conn(0)
		{}
		slot1(const SlotType& s)
		{
			*this = s;
		}
		~slot1()
		{
			delete _conn;
		}

		slot1& operator=(const SlotType& s)
		{
			if (s._conn)
				_conn = s._conn->clone();
			else
				_conn = 0;

			return *this;
		}

		Tret call(Targ0 arg0) const
		{
			return _conn->call(arg0);
		}

		Tret operator()(Targ0 arg0) const
		{
			return call(arg0);
		}

		operator bool() const
		{
			return _conn;
		}

	private:
		typedef connection_base1<Tret, Targ0> Connection;

		Connection* _conn;

		slot1(const Connection& conn):
			_conn(conn.clone())
		{}

		friend SlotType ptr_fun<Tret, Targ0>(Tret (*fun)(Targ0));

		template <typename Tobj2, typename Tret2, typename Targ02>
		friend slot1<Tret2, Targ02> mem_fun(Tobj2& obj,
				Tret2 (Tobj2::*fun)(Targ02));
};

template <typename Tret>
inline slot<Tret> ptr_fun(Tret (*fun)())
{
	return slot<Tret>(connection_func<Tret>(fun));
}

template <typename Tobj, typename Tret>
inline slot<Tret> mem_fun(Tobj& obj, Tret (Tobj::*fun)())
{
	return slot<Tret>(connection_meth<Tobj, Tret>(obj, fun));
}

template <typename Tret, typename Targ0>
inline slot1<Tret, Targ0> ptr_fun(Tret (*fun)(Targ0))
{
	return slot1<Tret, Targ0>(connection_func1<Tret, Targ0>(fun));
}

template <typename Tobj, typename Tret, typename Targ0>
inline slot1<Tret, Targ0> mem_fun(Tobj& obj, Tret (Tobj::*fun)(Targ0))
{
	return slot1<Tret, Targ0>(connection_meth1<Tobj, Tret, Targ0>(obj, fun));
}

//------------------------------
// signals
//------------------------------

template <typename Tret>
class signal;

template <typename Tret>
class connection
{
	public:
		connection():
			_list(0)
		{
		}

		void disconnect()
		{
			_list->erase(_iter);
		}

	private:
		typedef std::list<slot<Tret> > List;
		typedef typename List::iterator Iterator;

		List* _list;
		Iterator _iter;

		connection(List* list, Iterator iter):
			_list(list),
			_iter(iter)
		{}

		friend class signal<Tret>;
};

template <typename Tret>
class signal
{
	public:
		connection<Tret> connect(const slot<Tret> s)
		{
			_slots.push_back(s);
			return connection<Tret>(&_slots, (++_slots.rbegin()).base());
		}

		Tret emit() const
		{
			for (typename Slots::const_iterator iter = _slots.begin(),
					end = (++_slots.rbegin()).base();
					iter != end; ++iter)
				iter->call();

			return _slots.back().call();
		}

		Tret operator()() const
		{
			return emit();
		}

	private:
		typedef std::list<slot<Tret> > Slots;

		Slots _slots;
};

template <typename Tret, typename Targ0>
class signal1
{
	public:
		void connect(const slot<Tret> s)
		{
			_slots.push_back(s);
		}

		Tret emit(Targ0 arg0) const
		{
			for (typename Slots::const_iterator iter = _slots.begin(),
					end = (++_slots.rbegin()).base();
					iter != _slots.end()-1; ++iter)
				iter->call();

			return _slots.back().call(arg0);
		}

		Tret operator()(Targ0 arg0) const
		{
			return emit(arg0);
		}

	private:
		typedef std::list<slot<Tret> > Slots;

		Slots _slots;
};

} // namespace syg

#endif
