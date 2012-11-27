#include <cxxtest/TestSuite.h>
#include "common/util.h"
#include "common/tokenizer.h"

class TokenizerTestSuite : public CxxTest::TestSuite {
public:
	void test_nextToken() {

		// test normal behavior
		Common::StringTokenizer strTokenizer("Now, this is a test!", " ,!");
		Common::String tokenArray[] = {"Now", "this", "is", "a", "test"};

		for (int i = 0; i < ARRAYSIZE(tokenArray); ++i) {
			// make sure nextToken works correctly
			TS_ASSERT_EQUALS(tokenArray[i], strTokenizer.nextToken());
		}
		TS_ASSERT(strTokenizer.empty());

		// Test edge cases:

		// empty string
		Common::StringTokenizer s1("");
		TS_ASSERT_EQUALS("", s1.nextToken());
		TS_ASSERT(s1.empty());

		// empty delimiter
		Common::StringTokenizer s2("test String", "");
		TS_ASSERT_EQUALS("test String", s2.nextToken());

		// string is the delimiter
		Common::StringTokenizer s3("abc", "abc");
		TS_ASSERT_EQUALS("", s3.nextToken());
		TS_ASSERT(s3.empty());

		// consecutive delimiters in the string
		Common::StringTokenizer s4("strstr,after all!!", "str, !");
		TS_ASSERT_EQUALS("af", s4.nextToken());
	}

	void test_resetAndEmpty() {
		Common::StringTokenizer strTokenizer("Just, another test!", " ,!");

		// test reset()
		Common::String token1 = strTokenizer.nextToken(); //Just
		TS_ASSERT_EQUALS(token1, "Just");
		strTokenizer.reset();
		Common::String token2 = strTokenizer.nextToken(); //Just
		TS_ASSERT_EQUALS(token2, "Just");

		// test empty()
		TS_ASSERT(!strTokenizer.empty());
		strTokenizer.nextToken(); //another
		strTokenizer.nextToken(); //test
		TS_ASSERT(strTokenizer.empty());
	}

};
