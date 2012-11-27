#include <cxxtest/TestSuite.h>

#include "common/queue.h"

class QueueTestSuite : public CxxTest::TestSuite {
public:
	void test_empty_clear() {
		Common::Queue<int> queue;
		TS_ASSERT(queue.empty());

		queue.push(1);
		queue.push(2);
		TS_ASSERT(!queue.empty());

		queue.clear();

		TS_ASSERT(queue.empty());
	}

	void test_size() {
		Common::Queue<int> queue;
		TS_ASSERT_EQUALS(queue.size(), 0);

		queue.push(5);
		TS_ASSERT_EQUALS(queue.size(), 1);

		queue.push(9);
		queue.push(0);
		TS_ASSERT_EQUALS(queue.size(), 3);

		queue.pop();
		TS_ASSERT_EQUALS(queue.size(), 2);
	}

	void test_front_back_push_pop() {
		Common::Queue<int> container;

		container.push( 42);
		container.push(-23);

		TS_ASSERT_EQUALS(container.front(), 42);
		TS_ASSERT_EQUALS(container.back(), -23);

		container.front() = -17;
		container.back() = 163;
		TS_ASSERT_EQUALS(container.front(), -17);
		TS_ASSERT_EQUALS(container.back(),  163);

		container.pop();
		TS_ASSERT_EQUALS(container.front(), 163);
		TS_ASSERT_EQUALS(container.back(),  163);
	}

	void test_assign() {
		Common::Queue<int> q1, q2;

		for (int i = 0; i < 5; ++i) {
			q1.push(i);
			q2.push(4-i);
		}

		Common::Queue<int> q3(q1);

		for (int i = 0; i < 5; ++i) {
			TS_ASSERT_EQUALS(q3.front(), i);
			q3.pop();
		}

		TS_ASSERT(q3.empty());

		q3 = q2;

		for (int i = 4; i >= 0; --i) {
			TS_ASSERT_EQUALS(q3.front(), i);
			q3.pop();
		}

		TS_ASSERT(q3.empty());
		TS_ASSERT(!q1.empty());
		TS_ASSERT(!q2.empty());
	}
};
