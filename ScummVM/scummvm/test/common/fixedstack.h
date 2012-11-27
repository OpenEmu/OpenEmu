#include <cxxtest/TestSuite.h>

#include "common/stack.h"

class FixedStackTestSuite : public CxxTest::TestSuite {
public:
	void test_empty_clear() {
		Common::FixedStack<int> stack;
		TS_ASSERT(stack.empty());

		stack.push(1);
		stack.push(2);
		TS_ASSERT(!stack.empty());

		stack.clear();

		TS_ASSERT(stack.empty());
	}

	void test_size() {
		typedef Common::FixedStack<int> Stack;

		Stack stack;
		TS_ASSERT_EQUALS(stack.size(), (Stack::size_type)0);

		stack.push(5);
		TS_ASSERT_EQUALS(stack.size(), (Stack::size_type)1);

		stack.push(9);
		stack.push(0);
		TS_ASSERT_EQUALS(stack.size(), (Stack::size_type)3);

		stack.pop();
		TS_ASSERT_EQUALS(stack.size(), (Stack::size_type)2);
	}

	void test_top_pop() {
		Common::FixedStack<int> stack;

		stack.push( 42);
		stack.push(-23);

		TS_ASSERT_EQUALS(stack[0], 42);
		TS_ASSERT_EQUALS(stack.top(), -23);

		stack[0] = -23;
		stack.top() = 42;
		TS_ASSERT_EQUALS(stack[0], -23);
		TS_ASSERT_EQUALS(stack.top(), 42);

		stack.pop();
		TS_ASSERT_EQUALS(stack[0], -23);
	}

	void test_assign() {
		Common::FixedStack<int> q1, q2;

		for (int i = 0; i <= 4; ++i) {
			q1.push(4-i);
			q2.push(i);
		}

		Common::FixedStack<int> q3(q1);

		for (int i = 0; i < 5; ++i) {
			TS_ASSERT_EQUALS(q3.top(), i);
			q3.pop();
		}

		TS_ASSERT(q3.empty());

		q3 = q2;

		for (int i = 4; i >= 0; --i) {
			TS_ASSERT_EQUALS(q3.top(), i);
			q3.pop();
		}

		TS_ASSERT(q3.empty());
		TS_ASSERT(!q1.empty());
		TS_ASSERT(!q2.empty());
	}
};
