#include <cxxtest/TestSuite.h>

#include "common/ptr.h"

class PtrTestSuite : public CxxTest::TestSuite {
	public:

	// A simple class which keeps track of all its instances
	class InstanceCountingClass {
	public:
		static int count;
		InstanceCountingClass() { count++; }
		InstanceCountingClass(const InstanceCountingClass&) { count++; }
		~InstanceCountingClass() { count--; }
	};

	void test_deletion() {
		TS_ASSERT_EQUALS(InstanceCountingClass::count, 0);
		{
			Common::SharedPtr<InstanceCountingClass> p1(new InstanceCountingClass());
			TS_ASSERT_EQUALS(InstanceCountingClass::count, 1);

			Common::ScopedPtr<InstanceCountingClass> p2(new InstanceCountingClass());
			TS_ASSERT_EQUALS(InstanceCountingClass::count, 2);
		}
		TS_ASSERT_EQUALS(InstanceCountingClass::count, 0);
	}

	void test_assign() {
		Common::SharedPtr<int> p1(new int(1));
		TS_ASSERT(p1.unique());
		TS_ASSERT_EQUALS(*p1, 1);

		{
			Common::SharedPtr<int> p2 = p1;
			TS_ASSERT(!p1.unique());
			TS_ASSERT_EQUALS(p1.refCount(), p2.refCount());
			TS_ASSERT_EQUALS(p1.refCount(), 2);
			TS_ASSERT_EQUALS(p1, p2);
			TS_ASSERT_EQUALS(*p2, 1);
			{
				Common::SharedPtr<int> p3;
				p3 = p2;
				TS_ASSERT_EQUALS(p3, p2);
				TS_ASSERT_EQUALS(p3, p1);
				TS_ASSERT_EQUALS(p1.refCount(), 3);
				TS_ASSERT_EQUALS(*p3, 1);
				*p3 = 0;
				TS_ASSERT_EQUALS(*p3, 0);
			}
			TS_ASSERT_EQUALS(*p2, 0);
			TS_ASSERT_EQUALS(p1.refCount(), 2);
		}

		TS_ASSERT_EQUALS(*p1, 0);
		TS_ASSERT(p1.unique());
	}

	template<class T>
	struct Deleter {
		bool *test;
		void operator()(T *ptr) { *test = true; delete ptr; }
	};

	void test_deleter() {
		Deleter<int> myDeleter;
		myDeleter.test = new bool(false);

		{
			Common::SharedPtr<int> p(new int(1), myDeleter);
		}

		TS_ASSERT_EQUALS(*myDeleter.test, true);
		delete myDeleter.test;
	}

	void test_compare() {
		Common::SharedPtr<int> p1(new int(1));
		Common::SharedPtr<int> p2;

		TS_ASSERT(p1);
		TS_ASSERT(!p2);

		TS_ASSERT(p1 != 0);
		TS_ASSERT(p2 == 0);

		p1.reset();
		TS_ASSERT(!p1);
	}

	struct A {
		int a;
	};

	struct B : public A {
		int b;
	};

	void test_cast() {
		Common::SharedPtr<B> b(new B);
		Common::SharedPtr<A> a(b);
		a = b;
	}
};

int PtrTestSuite::InstanceCountingClass::count = 0;
