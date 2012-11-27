#include <cxxtest/TestSuite.h>

#include "common/str.h"

class StringTestSuite : public CxxTest::TestSuite
{
	public:
	void test_constructors() {
		Common::String str("test-string");
		TS_ASSERT_EQUALS(str, "test-string");
		str = Common::String(str.c_str()+5, 3);
		TS_ASSERT_EQUALS(str, "str");
		str = "test-string";
		TS_ASSERT_EQUALS(str, "test-string");
		str = Common::String(str.c_str()+5, str.c_str()+8);
		TS_ASSERT_EQUALS(str, "str");
	}

	void test_trim() {
		Common::String str("  This is a s tring with spaces  ");
		Common::String str2 = str;
		str.trim();
		TS_ASSERT_EQUALS(str, "This is a s tring with spaces");
		TS_ASSERT_EQUALS(str2, "  This is a s tring with spaces  ");
	}

	void test_empty_clear() {
		Common::String str("test");
		TS_ASSERT(!str.empty());
		str.clear();
		TS_ASSERT(str.empty());
	}

	void test_lastChar() {
		Common::String str;
		TS_ASSERT_EQUALS(str.lastChar(), '\0');
		str = "test";
		TS_ASSERT_EQUALS(str.lastChar(), 't');
		Common::String str2("bar");
		TS_ASSERT_EQUALS(str2.lastChar(), 'r');
	}

	void test_concat1() {
		Common::String str("foo");
		Common::String str2("bar");
		str += str2;
		TS_ASSERT_EQUALS(str, "foobar");
		TS_ASSERT_EQUALS(str2, "bar");
	}

	void test_concat2() {
		Common::String str("foo");
		str += "bar";
		TS_ASSERT_EQUALS(str, "foobar");
	}

	void test_concat3() {
		Common::String str("foo");
		str += 'X';
		TS_ASSERT_EQUALS(str, "fooX");
	}

	void test_refCount() {
		// using internal storage
		Common::String foo1("foo");
		Common::String foo2(foo1);
		Common::String foo3(foo2);
		foo3 += 'X';
		TS_ASSERT_EQUALS(foo1, "foo");
		TS_ASSERT_EQUALS(foo2, "foo");
		TS_ASSERT_EQUALS(foo3, "foo""X");
		foo2 = 'x';
		TS_ASSERT_EQUALS(foo1, "foo");
		TS_ASSERT_EQUALS(foo2, "x");
		TS_ASSERT_EQUALS(foo3, "foo""X");
	}

	void test_refCount2() {
		// using external storage
		Common::String foo1("fooasdkadklasdjklasdjlkasjdlkasjdklasjdlkjasdasd");
		Common::String foo2(foo1);
		Common::String foo3(foo2);
		foo3 += 'X';
		TS_ASSERT_EQUALS(foo1, "fooasdkadklasdjklasdjlkasjdlkasjdklasjdlkjasdasd");
		TS_ASSERT_EQUALS(foo2, "fooasdkadklasdjklasdjlkasjdlkasjdklasjdlkjasdasd");
		TS_ASSERT_EQUALS(foo3, "fooasdkadklasdjklasdjlkasjdlkasjdklasjdlkjasdasd""X");
		foo2 = 'x';
		TS_ASSERT_EQUALS(foo1, "fooasdkadklasdjklasdjlkasjdlkasjdklasjdlkjasdasd");
		TS_ASSERT_EQUALS(foo2, "x");
		TS_ASSERT_EQUALS(foo3, "fooasdkadklasdjklasdjlkasjdlkasjdklasjdlkjasdasd""X");
	}

	void test_refCount3() {
		Common::String foo1("0123456789abcdefghijk");
		Common::String foo2(foo1);
		Common::String foo3(foo2);
		foo3 += "0123456789abcdefghijk";
		TS_ASSERT_EQUALS(foo1, foo2);
		TS_ASSERT_EQUALS(foo2, "0123456789abcdefghijk");
		TS_ASSERT_EQUALS(foo3, "0123456789abcdefghijk""0123456789abcdefghijk");
		foo2 = 'x';
		TS_ASSERT_EQUALS(foo1, "0123456789abcdefghijk");
		TS_ASSERT_EQUALS(foo2, "x");
		TS_ASSERT_EQUALS(foo3, "0123456789abcdefghijk""0123456789abcdefghijk");
	}

	void test_refCount4() {
		Common::String foo1("fooasdkadklasdjklasdjlkasjdlkasjdklasjdlkjasdasd");
		Common::String foo2(foo1);
		Common::String foo3(foo2);
		foo3 += "fooasdkadklasdjklasdjlkasjdlkasjdklasjdlkjasdasd";
		TS_ASSERT_EQUALS(foo1, foo2);
		TS_ASSERT_EQUALS(foo2, "fooasdkadklasdjklasdjlkasjdlkasjdklasjdlkjasdasd");
		TS_ASSERT_EQUALS(foo3, "fooasdkadklasdjklasdjlkasjdlkasjdklasjdlkjasdasd""fooasdkadklasdjklasdjlkasjdlkasjdklasjdlkjasdasd");
		foo2 = 'x';
		TS_ASSERT_EQUALS(foo1, "fooasdkadklasdjklasdjlkasjdlkasjdklasjdlkjasdasd");
		TS_ASSERT_EQUALS(foo2, "x");
		TS_ASSERT_EQUALS(foo3, "fooasdkadklasdjklasdjlkasjdlkasjdklasjdlkjasdasd""fooasdkadklasdjklasdjlkasjdlkasjdklasjdlkjasdasd");
	}

	void test_refCount5() {
		// using external storage
		Common::String foo1("HelloHelloHelloHelloAndHi");
		Common::String foo2(foo1);

		for (Common::String::iterator i = foo2.begin(); i != foo2.end(); ++i)
			*i = 'h';

		TS_ASSERT_EQUALS(foo1, "HelloHelloHelloHelloAndHi");
		TS_ASSERT_EQUALS(foo2, "hhhhhhhhhhhhhhhhhhhhhhhhh");
	}

	void test_refCount6() {
		// using internal storage
		Common::String foo1("Hello");
		Common::String foo2(foo1);

		for (Common::String::iterator i = foo2.begin(); i != foo2.end(); ++i)
			*i = 'h';

		TS_ASSERT_EQUALS(foo1, "Hello");
		TS_ASSERT_EQUALS(foo2, "hhhhh");
	}

	void test_self_asignment() {
		Common::String foo1("12345678901234567890123456789012");
		foo1 = foo1.c_str() + 2;
		TS_ASSERT_EQUALS(foo1, "345678901234567890123456789012");

		Common::String foo2("123456789012");
		foo2 = foo2.c_str() + 2;
		TS_ASSERT_EQUALS(foo2, "3456789012");

		// "foo3" and "foo4" will be using allocated storage from construction on.
		Common::String foo3("12345678901234567890123456789012");
		foo3 += foo3.c_str();
		TS_ASSERT_EQUALS(foo3, "12345678901234567890123456789012""12345678901234567890123456789012");

		Common::String foo4("12345678901234567890123456789012");
		foo4 += foo4;
		TS_ASSERT_EQUALS(foo4, "12345678901234567890123456789012""12345678901234567890123456789012");

		// Based on our current Common::String implementation "foo5" and "foo6" will first use the internal storage,
		// and on "operator +=" they will change to allocated memory.
		Common::String foo5("123456789012");
		foo5 += foo5.c_str();
		TS_ASSERT_EQUALS(foo5, "123456789012""123456789012");

		Common::String foo6("123456789012");
		foo6 += foo6;
		TS_ASSERT_EQUALS(foo6, "123456789012""123456789012");

		// "foo7" and "foo8" will purely operate on internal storage.
		Common::String foo7("1234");
		foo7 += foo7.c_str();
		TS_ASSERT_EQUALS(foo7, "1234""1234");

		Common::String foo8("1234");
		foo8 += foo8;
		TS_ASSERT_EQUALS(foo8, "1234""1234");

		Common::String foo9("123456789012345678901234567889012");
		foo9 = foo9.c_str();
		TS_ASSERT_EQUALS(foo9, "123456789012345678901234567889012");
		foo9 = foo9;
		TS_ASSERT_EQUALS(foo9, "123456789012345678901234567889012");

		Common::String foo10("1234");
		foo10 = foo10.c_str();
		TS_ASSERT_EQUALS(foo10, "1234");
		foo10 = foo10;
		TS_ASSERT_EQUALS(foo10, "1234");
	}

	void test_hasPrefix() {
		Common::String str("this/is/a/test, haha");
		TS_ASSERT_EQUALS(str.hasPrefix(""), true);
		TS_ASSERT_EQUALS(str.hasPrefix("this"), true);
		TS_ASSERT_EQUALS(str.hasPrefix("thit"), false);
		TS_ASSERT_EQUALS(str.hasPrefix("foo"), false);
	}

	void test_hasSuffix() {
		Common::String str("this/is/a/test, haha");
		TS_ASSERT_EQUALS(str.hasSuffix(""), true);
		TS_ASSERT_EQUALS(str.hasSuffix("haha"), true);
		TS_ASSERT_EQUALS(str.hasSuffix("hahb"), false);
		TS_ASSERT_EQUALS(str.hasSuffix("hahah"), false);
	}

	void test_contains() {
		Common::String str("this/is/a/test, haha");
		TS_ASSERT_EQUALS(str.contains(""), true);
		TS_ASSERT_EQUALS(str.contains("haha"), true);
		TS_ASSERT_EQUALS(str.contains("hahb"), false);
		TS_ASSERT_EQUALS(str.contains("test"), true);

		TS_ASSERT_EQUALS(str.contains('/'), true);
		TS_ASSERT_EQUALS(str.contains('x'), false);
	}

	void test_toLowercase() {
		Common::String str("Test it, NOW! 42");
		Common::String str2 = str;
		str.toLowercase();
		TS_ASSERT_EQUALS(str, "test it, now! 42");
		TS_ASSERT_EQUALS(str2, "Test it, NOW! 42");
	}

	void test_toUppercase() {
		Common::String str("Test it, NOW! 42");
		Common::String str2 = str;
		str.toUppercase();
		TS_ASSERT_EQUALS(str, "TEST IT, NOW! 42");
		TS_ASSERT_EQUALS(str2, "Test it, NOW! 42");
	}

	void test_deleteChar() {
		Common::String str("01234567890123456789012345678901");
		str.deleteChar(10);
		TS_ASSERT_EQUALS(str, "0123456789123456789012345678901");
		str.deleteChar(10);
		TS_ASSERT_EQUALS(str, "012345678923456789012345678901");
	}

	void test_sharing() {
		Common::String str("01234567890123456789012345678901");
		Common::String str2(str);
		TS_ASSERT_EQUALS(str2, "01234567890123456789012345678901");
		str.deleteLastChar();
		TS_ASSERT_EQUALS(str, "0123456789012345678901234567890");
		TS_ASSERT_EQUALS(str2, "01234567890123456789012345678901");
	}

	void test_lastPathComponent() {
		TS_ASSERT_EQUALS(Common::lastPathComponent("/", '/'), "");
		TS_ASSERT_EQUALS(Common::lastPathComponent("/foo/bar", '/'), "bar");
		TS_ASSERT_EQUALS(Common::lastPathComponent("/foo//bar/", '/'), "bar");
		TS_ASSERT_EQUALS(Common::lastPathComponent("/foo/./bar", '/'), "bar");
		TS_ASSERT_EQUALS(Common::lastPathComponent("/foo//./bar//", '/'), "bar");
		TS_ASSERT_EQUALS(Common::lastPathComponent("/foo//.bar//", '/'), ".bar");

		TS_ASSERT_EQUALS(Common::lastPathComponent("", '/'), "");
		TS_ASSERT_EQUALS(Common::lastPathComponent("foo/bar", '/'), "bar");
		TS_ASSERT_EQUALS(Common::lastPathComponent("foo//bar/", '/'), "bar");
		TS_ASSERT_EQUALS(Common::lastPathComponent("foo/./bar", '/'), "bar");
		TS_ASSERT_EQUALS(Common::lastPathComponent("foo//./bar//", '/'), "bar");
		TS_ASSERT_EQUALS(Common::lastPathComponent("foo//.bar//", '/'), ".bar");

		TS_ASSERT_EQUALS(Common::lastPathComponent("foo", '/'), "foo");
	}

	void test_normalizePath() {
		TS_ASSERT_EQUALS(Common::normalizePath("/", '/'), "/");
		TS_ASSERT_EQUALS(Common::normalizePath("/foo/bar", '/'), "/foo/bar");
		TS_ASSERT_EQUALS(Common::normalizePath("/foo//bar/", '/'), "/foo/bar");
		TS_ASSERT_EQUALS(Common::normalizePath("/foo/./bar", '/'), "/foo/bar");
		TS_ASSERT_EQUALS(Common::normalizePath("/foo//./bar//", '/'), "/foo/bar");
		TS_ASSERT_EQUALS(Common::normalizePath("/foo//.bar//", '/'), "/foo/.bar");

		TS_ASSERT_EQUALS(Common::normalizePath("", '/'), "");
		TS_ASSERT_EQUALS(Common::normalizePath("foo/bar", '/'), "foo/bar");
		TS_ASSERT_EQUALS(Common::normalizePath("foo//bar/", '/'), "foo/bar");
		TS_ASSERT_EQUALS(Common::normalizePath("foo/./bar", '/'), "foo/bar");
		TS_ASSERT_EQUALS(Common::normalizePath("foo//./bar//", '/'), "foo/bar");
		TS_ASSERT_EQUALS(Common::normalizePath("foo//.bar//", '/'), "foo/.bar");

		TS_ASSERT_EQUALS(Common::normalizePath("..", '/'), "..");
		TS_ASSERT_EQUALS(Common::normalizePath("../", '/'), "..");
		TS_ASSERT_EQUALS(Common::normalizePath("/..", '/'), "/..");
		TS_ASSERT_EQUALS(Common::normalizePath("../bar", '/'), "../bar");
		TS_ASSERT_EQUALS(Common::normalizePath("foo//../", '/'), "");
		TS_ASSERT_EQUALS(Common::normalizePath("foo/../bar", '/'), "bar");
		TS_ASSERT_EQUALS(Common::normalizePath("foo//../bar//", '/'), "bar");
		TS_ASSERT_EQUALS(Common::normalizePath("foo//..bar//", '/'), "foo/..bar");

		TS_ASSERT_EQUALS(Common::normalizePath("foo/../../bar//", '/'), "../bar");
		TS_ASSERT_EQUALS(Common::normalizePath("../foo/../bar", '/'), "../bar");
		TS_ASSERT_EQUALS(Common::normalizePath("../../foo/bar/", '/'), "../../foo/bar");
	}

	void test_matchString() {
		TS_ASSERT(Common::matchString("",  "*"));
		TS_ASSERT(Common::matchString("a",  "*"));
		TS_ASSERT(Common::matchString("monkey.s01",  "*"));

		TS_ASSERT(!Common::matchString("",  "?"));
		TS_ASSERT(Common::matchString("a",  "?"));
		TS_ASSERT(!Common::matchString("monkey.s01",  "?"));

		TS_ASSERT(Common::matchString("monkey.s01",  "monkey.s??"));
		TS_ASSERT(Common::matchString("monkey.s99",  "monkey.s??"));
		TS_ASSERT(!Common::matchString("monkey.s101", "monkey.s??"));

		TS_ASSERT(Common::matchString("monkey.s01",  "monkey.s?1"));
		TS_ASSERT(!Common::matchString("monkey.s99",  "monkey.s?1"));
		TS_ASSERT(!Common::matchString("monkey.s101", "monkey.s?1"));

		TS_ASSERT(Common::matchString("monkey.s01",  "monkey.s*"));
		TS_ASSERT(Common::matchString("monkey.s99",  "monkey.s*"));
		TS_ASSERT(Common::matchString("monkey.s101", "monkey.s*"));

		TS_ASSERT(Common::matchString("monkey.s01",  "monkey.s*1"));
		TS_ASSERT(!Common::matchString("monkey.s99",  "monkey.s*1"));
		TS_ASSERT(Common::matchString("monkey.s101", "monkey.s*1"));

		TS_ASSERT(!Common::String("").matchString("*_"));
		TS_ASSERT(Common::String("a").matchString("a***"));
	}

	void test_string_printf() {
		TS_ASSERT_EQUALS( Common::String::format(""), "" );
		TS_ASSERT_EQUALS( Common::String::format("%s", "test"), "test" );
		TS_ASSERT_EQUALS( Common::String::format("%s.s%.02d", "monkey", 1), "monkey.s01" );
		TS_ASSERT_EQUALS( Common::String::format("Some %s to make this string longer than the default built-in %s %d", "text", "capacity", 123456), "Some text to make this string longer than the default built-in capacity 123456" );

		Common::String s = Common::String::format("%s%X", "test", 1234);
		TS_ASSERT_EQUALS(s, "test4D2");
		TS_ASSERT_EQUALS(s.size(), 7U);
	}

	void test_strlcpy() {
		static const char * const testString = "1234567890";

		char test1[4];
		TS_ASSERT_EQUALS(Common::strlcpy(test1, testString, 4), strlen(testString));
		TS_ASSERT_EQUALS(strcmp(test1, "123"), 0);

		char test2[12];
		test2[11] = 'X';
		TS_ASSERT_EQUALS(Common::strlcpy(test2, testString, 11), strlen(testString));
		TS_ASSERT_EQUALS(strcmp(test2, testString), 0);
		TS_ASSERT_EQUALS(test2[11], 'X');

		char test3[1] = { 'X' };
		TS_ASSERT_EQUALS(Common::strlcpy(test3, testString, 0), strlen(testString));
		TS_ASSERT_EQUALS(test3[0], 'X');

		char test4[12];
		TS_ASSERT_EQUALS(Common::strlcpy(test4, testString, 12), strlen(testString));
		TS_ASSERT_EQUALS(strcmp(test4, testString), 0);
	}

	void test_strlcat() {
		static const char * const initialString = "123";
		static const char * const appendString = "4567890";
		static const char * const resultString = "1234567890";

		char test1[4];
		TS_ASSERT_EQUALS(Common::strlcpy(test1, initialString, 4), strlen(initialString));
		TS_ASSERT_EQUALS(strcmp(test1, initialString), 0);
		TS_ASSERT_EQUALS(Common::strlcat(test1, appendString, 4), strlen(resultString));
		TS_ASSERT_EQUALS(strcmp(test1, initialString), 0);

		char test2[12];
		test2[11] = 'X';
		TS_ASSERT_EQUALS(Common::strlcpy(test2, initialString, 11), strlen(initialString));
		TS_ASSERT_EQUALS(strcmp(test2, initialString), 0);
		TS_ASSERT_EQUALS(Common::strlcat(test2, appendString, 11), strlen(resultString));
		TS_ASSERT_EQUALS(strcmp(test2, resultString), 0);
		TS_ASSERT_EQUALS(test2[11], 'X');

		char test3[1];
		test3[0] = 'X';
		TS_ASSERT_EQUALS(Common::strlcat(test3, appendString, 0), strlen(appendString));
		TS_ASSERT_EQUALS(test3[0], 'X');

		char test4[11];
		TS_ASSERT_EQUALS(Common::strlcpy(test4, initialString, 11), strlen(initialString));
		TS_ASSERT_EQUALS(strcmp(test4, initialString), 0);
		TS_ASSERT_EQUALS(Common::strlcat(test4, appendString, 11), strlen(resultString));
		TS_ASSERT_EQUALS(strcmp(test4, resultString), 0);
	}

	void test_scumm_stricmp() {
		TS_ASSERT_EQUALS(scumm_stricmp("abCd", "abCd"), 0);
		TS_ASSERT_EQUALS(scumm_stricmp("abCd", "ABCd"), 0);
		TS_ASSERT_LESS_THAN(scumm_stricmp("abCd", "ABCe"), 0);
		TS_ASSERT_LESS_THAN(scumm_stricmp("abCd", "ABCde"), 0);
	}

	void test_scumm_strnicmp() {
		TS_ASSERT_EQUALS(scumm_strnicmp("abCd", "abCd", 3), 0);
		TS_ASSERT_EQUALS(scumm_strnicmp("abCd", "ABCd", 4), 0);
		TS_ASSERT_EQUALS(scumm_strnicmp("abCd", "ABCd", 5), 0);
		TS_ASSERT_EQUALS(scumm_strnicmp("abCd", "ABCe", 3), 0);
		TS_ASSERT_LESS_THAN(scumm_strnicmp("abCd", "ABCe", 4), 0);
		TS_ASSERT_EQUALS(scumm_strnicmp("abCd", "ABCde", 4), 0);
		TS_ASSERT_LESS_THAN(scumm_strnicmp("abCd", "ABCde", 5), 0);
	}
};
