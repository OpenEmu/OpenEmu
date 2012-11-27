#include <cxxtest/TestSuite.h>

#include "common/func.h"

void myFunction1(int &dst, const int src) { dst = src; }
void myFunction2(const int src, int &dst) { dst = src; }

class FuncTestSuite : public CxxTest::TestSuite
{
	public:
	void test_bind1st() {
		int dst = 0;
		Common::bind1st(Common::ptr_fun(myFunction1), dst)(1);
		TS_ASSERT_EQUALS(dst, 1);
	}

	void test_bind2nd() {
		int dst = 0;
		Common::bind2nd(Common::ptr_fun(myFunction2), dst)(1);
		TS_ASSERT_EQUALS(dst, 1);
	}

	struct Foo {
		void fooAdd(int &foo) {
			++foo;
		}

		void fooSub(int &foo) const {
			--foo;
		}
	};

	void test_mem_fun_ref() {
		Foo myFoos[4];
		int counter = 0;

		Common::for_each(myFoos, myFoos+4, Common::bind2nd(Common::mem_fun_ref(&Foo::fooAdd), counter));
		TS_ASSERT_EQUALS(counter, 4);

		Common::for_each(myFoos, myFoos+4, Common::bind2nd(Common::mem_fun_ref(&Foo::fooSub), counter));
		TS_ASSERT_EQUALS(counter, 0);
	}

	void test_mem_fun() {
		Foo *myFoos[4];
		for (int i = 0; i < 4; ++i)
			myFoos[i] = new Foo;

		int counter = 0;

		Common::for_each(myFoos, myFoos+4, Common::bind2nd(Common::mem_fun(&Foo::fooAdd), counter));
		TS_ASSERT_EQUALS(counter, 4);

		Common::for_each(myFoos, myFoos+4, Common::bind2nd(Common::mem_fun(&Foo::fooSub), counter));
		TS_ASSERT_EQUALS(counter, 0);

		for (int i = 0; i < 4; ++i)
			delete myFoos[i];
	}
};
