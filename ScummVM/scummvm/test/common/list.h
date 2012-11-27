#include <cxxtest/TestSuite.h>

#include "common/list.h"

class ListTestSuite : public CxxTest::TestSuite
{
	public:
	void test_empty_clear() {
		Common::List<int> container;
		TS_ASSERT(container.empty());
		container.push_back(17);
		container.push_back(33);
		TS_ASSERT(!container.empty());
		container.clear();
		TS_ASSERT(container.empty());
	}

	public:
	void test_size() {
		Common::List<int> container;
		TS_ASSERT_EQUALS(container.size(), (unsigned int)0);
		container.push_back(17);
		TS_ASSERT_EQUALS(container.size(), (unsigned int)1);
		container.push_back(33);
		TS_ASSERT_EQUALS(container.size(), (unsigned int)2);
		container.clear();
		TS_ASSERT_EQUALS(container.size(), (unsigned int)0);
	}

	void test_iterator_begin_end() {
		Common::List<int> container;

		// The container is initially empty ...
		TS_ASSERT_EQUALS(container.begin(), container.end());

		// ... then non-empty ...
		container.push_back(33);
		TS_ASSERT_DIFFERS(container.begin(), container.end());

		// ... and again empty.
		container.clear();
		TS_ASSERT_EQUALS(container.begin(), container.end());
	}

	void test_iterator() {
		Common::List<int> container;
		Common::List<int>::iterator iter;
		Common::List<int>::const_iterator cIter;

		// Fill the container with some random data
		container.push_back(17);
		container.push_back(33);
		container.push_back(-11);

		// Iterate over the container and verify that we encounter the elements in
		// the order we expect them to be.

		iter = container.begin();
		cIter = container.begin();

		TS_ASSERT_EQUALS(iter, cIter);

		TS_ASSERT_EQUALS(*iter, 17);
		++iter;
		++cIter;
		TS_ASSERT_DIFFERS(iter, container.end());
		TS_ASSERT_DIFFERS(cIter, container.end());
		TS_ASSERT_EQUALS(iter, cIter);

		TS_ASSERT_EQUALS(*iter, 33);
		++iter;
		++cIter;
		TS_ASSERT_DIFFERS(iter, container.end());
		TS_ASSERT_DIFFERS(cIter, container.end());
		TS_ASSERT_EQUALS(iter, cIter);

		// Also test the postinc
		TS_ASSERT_EQUALS(*iter, -11);
		iter++;
		cIter++;
		TS_ASSERT_EQUALS(iter, container.end());
		TS_ASSERT_EQUALS(cIter, container.end());
		TS_ASSERT_EQUALS(iter, cIter);

		cIter = iter;
		TS_ASSERT_EQUALS(iter, cIter);
	}

	void test_insert() {
		Common::List<int> container;
		Common::List<int>::iterator iter;

		// Fill the container with some random data
		container.push_back(17);
		container.push_back(33);
		container.push_back(-11);

		// Iterate to after the second element
		iter = container.begin();
		++iter;
		++iter;

		// Now insert some values here
		container.insert(iter, 42);
		container.insert(iter, 43);

		// Verify contents are correct
		iter = container.begin();

		TS_ASSERT_EQUALS(*iter, 17);
		++iter;
		TS_ASSERT_DIFFERS(iter, container.end());

		TS_ASSERT_EQUALS(*iter, 33);
		++iter;
		TS_ASSERT_DIFFERS(iter, container.end());

		TS_ASSERT_EQUALS(*iter, 42);
		++iter;
		TS_ASSERT_DIFFERS(iter, container.end());

		TS_ASSERT_EQUALS(*iter, 43);
		++iter;
		TS_ASSERT_DIFFERS(iter, container.end());

		TS_ASSERT_EQUALS(*iter, -11);
		++iter;
		TS_ASSERT_EQUALS(iter, container.end());
	}

	void test_erase() {
		Common::List<int> container;
		Common::List<int>::iterator first, last;

		// Fill the container with some random data
		container.push_back(17);
		container.push_back(33);
		container.push_back(-11);
		container.push_back(42);
		container.push_back(43);

		// Iterate to after the second element
		first = container.begin();
		++first;
		++first;

		// Iterate to after the fourth element
		last = first;
		++last;
		++last;

		// Now erase that range
		container.erase(first, last);

		// Verify contents are correct
		Common::List<int>::iterator iter = container.begin();

		TS_ASSERT_EQUALS(*iter, 17);
		++iter;
		TS_ASSERT_DIFFERS(iter, container.end());

		TS_ASSERT_EQUALS(*iter, 33);
		++iter;
		TS_ASSERT_DIFFERS(iter, container.end());

		TS_ASSERT_EQUALS(*iter, 43);
		++iter;
		TS_ASSERT_EQUALS(iter, container.end());
	}

	void test_remove() {
		Common::List<int> container;
		Common::List<int>::iterator first, last;

		// Fill the container with some random data
		container.push_back(-11);
		container.push_back(17);
		container.push_back(33);
		container.push_back(42);
		container.push_back(-11);
		container.push_back(42);
		container.push_back(43);

		// Remove some stuff
		container.remove(42);
		container.remove(-11);

		// Now erase that range
		container.erase(first, last);

		// Verify contents are correct
		Common::List<int>::iterator iter = container.begin();

		TS_ASSERT_EQUALS(*iter, 17);
		++iter;
		TS_ASSERT_DIFFERS(iter, container.end());

		TS_ASSERT_EQUALS(*iter, 33);
		++iter;
		TS_ASSERT_DIFFERS(iter, container.end());

		TS_ASSERT_EQUALS(*iter, 43);
		++iter;
		TS_ASSERT_EQUALS(iter, container.end());
	}

	void test_reverse() {
		Common::List<int> container;
		Common::List<int>::iterator iter;

		// Fill the container with some random data
		container.push_back(17);
		container.push_back(33);
		container.push_back(-11);

		iter = container.reverse_begin();
		TS_ASSERT_DIFFERS(iter, container.end());


		TS_ASSERT_EQUALS(*iter, -11);
		--iter;
		TS_ASSERT_DIFFERS(iter, container.end());

		TS_ASSERT_EQUALS(*iter, 33);
		--iter;
		TS_ASSERT_DIFFERS(iter, container.end());

		TS_ASSERT_EQUALS(*iter, 17);
		--iter;
		TS_ASSERT_EQUALS(iter, container.end());

		iter = container.reverse_begin();

		iter = container.reverse_erase(iter);
		TS_ASSERT_DIFFERS(iter, container.end());
		TS_ASSERT_EQUALS(*iter, 33);

		iter = container.reverse_erase(iter);
		TS_ASSERT_DIFFERS(iter, container.end());
		TS_ASSERT_EQUALS(*iter, 17);

		iter = container.reverse_erase(iter);
		TS_ASSERT_EQUALS(iter, container.end());

		TS_ASSERT_EQUALS(container.begin(), container.end());
		TS_ASSERT_EQUALS(container.reverse_begin(), container.end());
	}

	void test_front_back_push_pop() {
		Common::List<int> container;

		container.push_back( 42);
		container.push_back(-23);

		TS_ASSERT_EQUALS(container.front(), 42);
		TS_ASSERT_EQUALS(container.back(), -23);

		container.front() = -17;
		container.back() = 163;
		TS_ASSERT_EQUALS(container.front(), -17);
		TS_ASSERT_EQUALS(container.back(),  163);

		container.pop_front();
		TS_ASSERT_EQUALS(container.front(), 163);
		TS_ASSERT_EQUALS(container.back(),  163);

		container.push_front(99);
		TS_ASSERT_EQUALS(container.front(), 99);
		TS_ASSERT_EQUALS(container.back(),  163);

		container.pop_back();
		TS_ASSERT_EQUALS(container.front(), 99);
		TS_ASSERT_EQUALS(container.back(),  99);
	}
};
