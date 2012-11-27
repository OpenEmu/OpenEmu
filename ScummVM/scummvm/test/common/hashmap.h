#include <cxxtest/TestSuite.h>

#include "common/hashmap.h"
#include "common/hash-str.h"

class HashMapTestSuite : public CxxTest::TestSuite
{
	public:
	void test_empty_clear() {
		Common::HashMap<int, int> container;
		TS_ASSERT(container.empty());
		container[0] = 17;
		container[1] = 33;
		TS_ASSERT(!container.empty());
		container.clear();
		TS_ASSERT(container.empty());

		Common::StringMap container2;
		TS_ASSERT(container2.empty());
		container2["foo"] = "bar";
		container2["quux"] = "blub";
		TS_ASSERT(!container2.empty());
		container2.clear();
		TS_ASSERT(container2.empty());
	}

	void test_contains() {
		Common::HashMap<int, int> container;
		container[0] = 17;
		container[1] = 33;
		TS_ASSERT(container.contains(0));
		TS_ASSERT(container.contains(1));
		TS_ASSERT(!container.contains(17));
		TS_ASSERT(!container.contains(-1));

		Common::StringMap container2;
		container2["foo"] = "bar";
		container2["quux"] = "blub";
		TS_ASSERT(container2.contains("foo"));
		TS_ASSERT(container2.contains("quux"));
		TS_ASSERT(!container2.contains("bar"));
		TS_ASSERT(!container2.contains("asdf"));
	}

	void test_add_remove() {
		Common::HashMap<int, int> container;
		container[0] = 17;
		container[1] = 33;
		container[2] = 45;
		container[3] = 12;
		container[4] = 96;
		TS_ASSERT(container.contains(1));
		container.erase(1);
		TS_ASSERT(!container.contains(1));
		container[1] = 42;
		TS_ASSERT(container.contains(1));
		container.erase(0);
		TS_ASSERT(!container.empty());
		container.erase(1);
		TS_ASSERT(!container.empty());
		container.erase(2);
		TS_ASSERT(!container.empty());
		container.erase(3);
		TS_ASSERT(!container.empty());
		container.erase(4);
		TS_ASSERT(container.empty());
		container[1] = 33;
		TS_ASSERT(container.contains(1));
		TS_ASSERT(!container.empty());
		container.erase(1);
		TS_ASSERT(container.empty());
	}

	void test_add_remove_iterator() {
		Common::HashMap<int, int> container;
		container[0] = 17;
		container[1] = 33;
		container[2] = 45;
		container[3] = 12;
		container[4] = 96;
		TS_ASSERT(container.contains(1));
		container.erase(container.find(1));
		TS_ASSERT(!container.contains(1));
		container[1] = 42;
		TS_ASSERT(container.contains(1));
		container.erase(container.find(0));
		TS_ASSERT(!container.empty());
		container.erase(container.find(1));
		TS_ASSERT(!container.empty());
		container.erase(container.find(2));
		TS_ASSERT(!container.empty());
		container.erase(container.find(3));
		TS_ASSERT(!container.empty());
		container.erase(container.find(4));
		TS_ASSERT(container.empty());
		container[1] = 33;
		TS_ASSERT(container.contains(1));
		TS_ASSERT(!container.empty());
		container.erase(container.find(1));
		TS_ASSERT(container.empty());
	}

	void test_lookup() {
		Common::HashMap<int, int> container;
		container[0] = 17;
		container[1] = -1;
		container[2] = 45;
		container[3] = 12;
		container[4] = 96;

		TS_ASSERT_EQUALS(container[0], 17);
		TS_ASSERT_EQUALS(container[1], -1);
		TS_ASSERT_EQUALS(container[2], 45);
		TS_ASSERT_EQUALS(container[3], 12);
		TS_ASSERT_EQUALS(container[4], 96);
	}

	void test_lookup_with_default() {
		Common::HashMap<int, int> container;
		container[0] = 17;
		container[1] = -1;
		container[2] = 45;
		container[3] = 12;
		container[4] = 96;

		// We take a const ref now to ensure that the map
		// is not modified by getVal.
		const Common::HashMap<int, int> &containerRef = container;

		TS_ASSERT_EQUALS(containerRef.getVal(0), 17);
		TS_ASSERT_EQUALS(containerRef.getVal(17), 0);
		TS_ASSERT_EQUALS(containerRef.getVal(0, -10), 17);
		TS_ASSERT_EQUALS(containerRef.getVal(17, -10), -10);
	}

	void test_iterator_begin_end() {
		Common::HashMap<int, int> container;

		// The container is initially empty ...
		TS_ASSERT_EQUALS(container.begin(), container.end());

		// ... then non-empty ...
		container[324] = 33;
		TS_ASSERT_DIFFERS(container.begin(), container.end());

		// ... and again empty.
		container.clear();
		TS_ASSERT_EQUALS(container.begin(), container.end());
	}

	void test_hash_map_copy() {
		Common::HashMap<int, int> map1, container2;
		map1[323] = 32;
		container2 = map1;
		TS_ASSERT_EQUALS(container2[323], 32);
	}

    void test_collision() {
		// NB: The usefulness of this example depends strongly on the
		// specific hashmap implementation.
		// It is constructed to insert multiple colliding elements.
		Common::HashMap<int, int> h;
		h[5] = 1;
		h[32+5] = 1;
		h[64+5] = 1;
		h[128+5] = 1;
		TS_ASSERT(h.contains(5));
		TS_ASSERT(h.contains(32+5));
		TS_ASSERT(h.contains(64+5));
		TS_ASSERT(h.contains(128+5));
		h.erase(32+5);
		TS_ASSERT(h.contains(5));
		TS_ASSERT(h.contains(64+5));
		TS_ASSERT(h.contains(128+5));
		h.erase(5);
		TS_ASSERT(h.contains(64+5));
		TS_ASSERT(h.contains(128+5));
		h[32+5] = 1;
		TS_ASSERT(h.contains(32+5));
		TS_ASSERT(h.contains(64+5));
		TS_ASSERT(h.contains(128+5));
		h[5] = 1;
		TS_ASSERT(h.contains(5));
		TS_ASSERT(h.contains(32+5));
		TS_ASSERT(h.contains(64+5));
		TS_ASSERT(h.contains(128+5));
		h.erase(5);
		TS_ASSERT(h.contains(32+5));
		TS_ASSERT(h.contains(64+5));
		TS_ASSERT(h.contains(128+5));
		h.erase(64+5);
		TS_ASSERT(h.contains(32+5));
		TS_ASSERT(h.contains(128+5));
		h.erase(128+5);
		TS_ASSERT(h.contains(32+5));
		h.erase(32+5);
		TS_ASSERT(h.empty());
    }

	void test_iterator() {
		Common::HashMap<int, int> container;
		container[0] = 17;
		container[1] = 33;
		container[2] = 45;
		container[3] = 12;
		container[4] = 96;
		container.erase(1);
		container[1] = 42;
		container.erase(0);
		container.erase(1);

		int found = 0;
		Common::HashMap<int, int>::iterator i;
		for (i = container.begin(); i != container.end(); ++i) {
			int key = i->_key;
			TS_ASSERT(key >= 0 && key <= 4);
			TS_ASSERT(!(found & (1 << key)));
			found |= 1 << key;
		}
		TS_ASSERT(found == 16+8+4);

		found = 0;
		Common::HashMap<int, int>::const_iterator j;
		for (j = container.begin(); j != container.end(); ++j) {
			int key = j->_key;
			TS_ASSERT(key >= 0 && key <= 4);
			TS_ASSERT(!(found & (1 << key)));
			found |= 1 << key;
		}
		TS_ASSERT(found == 16+8+4);
}

	// TODO: Add test cases for iterators, find, ...
};
