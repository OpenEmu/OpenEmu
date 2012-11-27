#include <cxxtest/TestSuite.h>

#include "common/md5.h"
#include "common/stream.h"

/*
 * those are the standard RFC 1321 test vectors
 */
static const char *md5_test_string[] = {
	"",
	"a",
	"abc",
	"message digest",
	"abcdefghijklmnopqrstuvwxyz",
	"ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789",
	"12345678901234567890123456789012345678901234567890123456789012" \
		"345678901234567890"
};

static const char *md5_test_digest[] = {
	"d41d8cd98f00b204e9800998ecf8427e",
	"0cc175b9c0f1b6a831c399e269772661",
	"900150983cd24fb0d6963f7d28e17f72",
	"f96b697d7cb7938d525a2f31aaf161d0",
	"c3fcd3d76192e4007dfb496cca67e13b",
	"d174ab98d277d9f5a5611c2c9f419d9f",
	"57edf4a22be3c955ac49da2e2107b67a"
};

class MD5TestSuite : public CxxTest::TestSuite {
	public:
	void test_computeStreamMD5() {
		int i, j;
		char output[33];
		unsigned char md5sum[16];

		for (i = 0; i < 7; i++) {
			Common::MemoryReadStream stream((const byte *)md5_test_string[i], strlen(md5_test_string[i]));
			Common::computeStreamMD5(stream, md5sum);

			for (j = 0; j < 16; j++) {
				sprintf(output + j * 2, "%02x", md5sum[j]);
			}

			Common::String tmp(output);
			TS_ASSERT_EQUALS(tmp, md5_test_digest[i]);
		}
	}

};
