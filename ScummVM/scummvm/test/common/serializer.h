#include <cxxtest/TestSuite.h>

#include "common/serializer.h"
#include "common/stream.h"

class SerializerTestSuite : public CxxTest::TestSuite {
	Common::SeekableReadStream *_inStreamV1;
	Common::SeekableReadStream *_inStreamV2;
public:
	void setUp() {
		// Our pseudo data format is as follows:
		// * magic id - string "MAGI"
		// * Version - uint32, LE
		// * uint32, LE (available in v2 onward)
		// * uint16, BE (available in v1 onward)
		// * sint16, LE (available only in v1)
		// * byte       (always available)
		static const byte contents_v1[] = {
			'M', 'A', 'G', 'I',		// magic id
			0x01, 0x00, 0x00, 0x00,	// Version
			0x06, 0x07, 			// uint16, BE (available in v1 onward)
			0xfe, 0xff, 			// sint16, LE (available only in v1)
			0x0a					// byte       (always available)
		};
		static const byte contents_v2[] = {
			'M', 'A', 'G', 'I',		// magic id
			0x02, 0x00, 0x00, 0x00,	// Version
			0x02, 0x03, 0x04, 0x05,	// uint32, LE (available in v2 onward)
			0x06, 0x07, 			// uint16, BE (available in v1 onward)
			0x0a					// byte       (always available)
		};

		_inStreamV1 = new Common::MemoryReadStream(contents_v1, sizeof(contents_v1));
		_inStreamV2 = new Common::MemoryReadStream(contents_v2, sizeof(contents_v2));
	}

	void tearDown() {
		delete _inStreamV1;
		delete _inStreamV2;
	}

	// A method which reads a v1 file
	void readVersioned_v1(Common::SeekableReadStream *stream, Common::Serializer::Version version) {
		Common::Serializer  ser(stream, 0);

		TS_ASSERT(ser.matchBytes("MAGI", 4));

		TS_ASSERT(ser.syncVersion(1));
		TS_ASSERT_EQUALS(ser.getVersion(), version);

		uint32 tmp = 0;

		ser.syncAsUint16BE(tmp, Common::Serializer::Version(1));
		TS_ASSERT_EQUALS(tmp, (uint16)0x0607);

		ser.syncAsSint16LE(tmp, Common::Serializer::Version(1));
		TS_ASSERT_EQUALS((int16)tmp, -2);

		ser.syncAsByte(tmp);
		TS_ASSERT_EQUALS(tmp, (uint8)0x0a);
	}

	// A method which reads a v2 file
	void readVersioned_v2(Common::SeekableReadStream *stream, Common::Serializer::Version version) {
		Common::Serializer  ser(stream, 0);

		TS_ASSERT(ser.matchBytes("MAGI", 4));

		TS_ASSERT(ser.syncVersion(2));
		TS_ASSERT_EQUALS(ser.getVersion(), version);

		uint32 tmp;

		// Read a value only available starting with v2.
		// Thus if we load an old save, it must be
		// manually set. To simplify that, no sync method should
		// modify the value passed to it if nothing was read!
		tmp = 0x12345678;
		ser.syncAsUint32LE(tmp, Common::Serializer::Version(2));
		if (ser.getVersion() < 2) {
			TS_ASSERT_EQUALS(tmp, (uint32)0x12345678);
		} else {
			TS_ASSERT_EQUALS(tmp, (uint32)0x05040302);
		}

		ser.syncAsUint16BE(tmp, Common::Serializer::Version(1));
		TS_ASSERT_EQUALS(tmp, (uint32)0x0607);

		// Skip over obsolete data
		ser.skip(2, Common::Serializer::Version(1), Common::Serializer::Version(1));

		ser.syncAsByte(tmp);
		TS_ASSERT_EQUALS(tmp, (uint8)0x0a);
	}

	void test_read_v1_as_v1() {
		readVersioned_v1(_inStreamV1, 1);
	}

	// There is no test_read_v2_as_v1() because a v1 parser cannot possibly
	// read v2 data correctly. It should instead error out if it
	// detects a version newer than its current version.

	void test_read_v1_as_v2() {
		readVersioned_v2(_inStreamV1, 1);
	}

	void test_read_v2_as_v2() {
		readVersioned_v2(_inStreamV2, 2);
	}
};
