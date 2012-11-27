#include <cxxtest/TestSuite.h>

#include "audio/timestamp.h"

class TimestampTestSuite : public CxxTest::TestSuite
{
	public:
	void test_diff_add_frames() {
		const Audio::Timestamp a(10000, 1000);
		const Audio::Timestamp b(10001, 1000);
		const Audio::Timestamp c(10002, 1000);

		TS_ASSERT_EQUALS(a.frameDiff(b), -1);
		TS_ASSERT_EQUALS(b.frameDiff(a), 1);
		TS_ASSERT_EQUALS(c.frameDiff(a), 2);
		TS_ASSERT_EQUALS(b.addFrames(2000).frameDiff(a), 2001);
		TS_ASSERT_EQUALS(a.frameDiff(b), -1);
		TS_ASSERT_EQUALS(b.frameDiff(a), 1);
		TS_ASSERT_EQUALS(c.frameDiff(a), 2);
		TS_ASSERT_EQUALS(b.addFrames(2000).frameDiff(a.addFrames(-1000)), 3001);
		TS_ASSERT_EQUALS(a.frameDiff(b), -1);
		TS_ASSERT_EQUALS(b.frameDiff(a), 1);
		TS_ASSERT_EQUALS(c.frameDiff(a), 2);
	}

	void test_diff_add_msecs() {
		Audio::Timestamp ts0(3, 22050);
		Audio::Timestamp ts1(0, 22050);
		Audio::Timestamp ts2(0, 22050);

		TS_ASSERT_EQUALS(ts0.msecs(), 3);
		TS_ASSERT_EQUALS(ts0.totalNumberOfFrames(), 3 * 22050 / 1000);
		TS_ASSERT_EQUALS(ts0.numberOfFrames(), 3 * 22050 / 1000);

		ts1 = ts1.addFrames(53248);
		TS_ASSERT_EQUALS(ts1.secs(), 2);
		TS_ASSERT_EQUALS(ts1.msecs(), 53248 * 1000 / 22050);
		TS_ASSERT_EQUALS(ts1.totalNumberOfFrames(), 53248);
		TS_ASSERT_EQUALS(ts1.numberOfFrames(), 53248 - 2 * 22050);
		ts1 = ts1.addMsecs(47);
		TS_ASSERT_EQUALS(ts1.secs(), 2);
		TS_ASSERT_EQUALS(ts1.msecs(), 2414+47);
		TS_ASSERT_EQUALS(ts1.totalNumberOfFrames(), 47*22050 / 1000 + 53248);
		TS_ASSERT_EQUALS(ts1.numberOfFrames(), 47*22050 / 1000 + 53248 - 2 * 22050);

		ts2 = ts2.addMsecs(47);
		TS_ASSERT_EQUALS(ts2.secs(), 0);
		TS_ASSERT_EQUALS(ts2.msecs(), 47);
		TS_ASSERT_EQUALS(ts2.totalNumberOfFrames(), 47*22050 / 1000);
		TS_ASSERT_EQUALS(ts2.numberOfFrames(), 47*22050 / 1000);
		ts2 = ts2.addFrames(53248);
		TS_ASSERT_EQUALS(ts2.secs(), 2);
		TS_ASSERT_EQUALS(ts2.msecs(), 2414+47);
		TS_ASSERT_EQUALS(ts2.totalNumberOfFrames(), 47*22050 / 1000 + 53248);
		TS_ASSERT_EQUALS(ts2.numberOfFrames(), 47*22050 / 1000 + 53248 - 2 * 22050);
	}

	void test_ticks() {
		const Audio::Timestamp a(1234, 60);
		const Audio::Timestamp b(5678, 60);

		TS_ASSERT_EQUALS(a.msecs(), 1234);
		TS_ASSERT_EQUALS(b.msecs(), 5678);

		TS_ASSERT_EQUALS(a.secs(), 1);
		TS_ASSERT_EQUALS(b.secs(), 5);

		TS_ASSERT_EQUALS(a.msecsDiff(b), 1234 - 5678);
		TS_ASSERT_EQUALS(b.msecsDiff(a), 5678 - 1234);

		TS_ASSERT_EQUALS(a.frameDiff(b), (1234 - 5678) * 60 / 1000);
		TS_ASSERT_EQUALS(b.frameDiff(a), (5678 - 1234) * 60 / 1000);

		TS_ASSERT_EQUALS(a.addFrames(1).msecs(), (1234 + 1000 * 1/60));
		TS_ASSERT_EQUALS(a.addFrames(59).msecs(), (1234 + 1000 * 59/60));
		TS_ASSERT_EQUALS(a.addFrames(60).msecs(), (1234 + 1000 * 60/60));

		// As soon as we go back even by only one frame, the msec value
		// has to drop by at least one.
		TS_ASSERT_EQUALS(a.addFrames(-1).msecs(), (1234 - 1000 * 1/60 - 1));
		TS_ASSERT_EQUALS(a.addFrames(-59).msecs(), (1234 - 1000 * 59/60 - 1));
		TS_ASSERT_EQUALS(a.addFrames(-60).msecs(), (1234 - 1000 * 60/60));
	}

	void test_more_add_diff() {
		const Audio::Timestamp c(10002, 1000);

		for (int i = -10000; i < 10000; i++) {
			int v = c.addFrames(i).frameDiff(c);
			TS_ASSERT_EQUALS(v, i);
		}
	}

	void test_negate() {
		const Audio::Timestamp a = Audio::Timestamp(0, 60).addFrames(13);
		const Audio::Timestamp b = -a;

		TS_ASSERT_EQUALS(a.msecs() + 1, -b.msecs());
		TS_ASSERT_EQUALS(a.totalNumberOfFrames(), -b.totalNumberOfFrames());
		TS_ASSERT_EQUALS(a.numberOfFrames(), 60 - b.numberOfFrames());
	}

	void test_add_sub() {
		const Audio::Timestamp a = Audio::Timestamp(0, 60).addFrames(13);
		const Audio::Timestamp b = -a;
		const Audio::Timestamp c = Audio::Timestamp(0, 60).addFrames(20);

		TS_ASSERT_EQUALS((a+a).secs(), 0);
		TS_ASSERT_EQUALS((a+a).numberOfFrames(), 2*13);

		TS_ASSERT_EQUALS((a+b).secs(), 0);
		TS_ASSERT_EQUALS((a+b).numberOfFrames(), 0);

		TS_ASSERT_EQUALS((a+c).secs(), 0);
		TS_ASSERT_EQUALS((a+c).numberOfFrames(), 13+20);

		TS_ASSERT_EQUALS((a-a).secs(), 0);
		TS_ASSERT_EQUALS((a-a).numberOfFrames(), 0);

		TS_ASSERT_EQUALS((a-b).secs(), 0);
		TS_ASSERT_EQUALS((a-b).numberOfFrames(), 2*13);

		TS_ASSERT_EQUALS((a-c).secs(), -1);
		TS_ASSERT_EQUALS((a-c).numberOfFrames(), 60 + (13 - 20));
	}

	void test_diff_with_conversion() {
		const Audio::Timestamp a = Audio::Timestamp(10, 1000).addFrames(20);
		const Audio::Timestamp b = Audio::Timestamp(10, 1000/5).addFrames(20/5);
		const Audio::Timestamp c = Audio::Timestamp(10, 1000*2).addFrames(20*2);

		TS_ASSERT_EQUALS(a.frameDiff(a), 0);
		TS_ASSERT_EQUALS(a.frameDiff(b), 0);
		TS_ASSERT_EQUALS(a.frameDiff(c), 0);

		TS_ASSERT_EQUALS(b.frameDiff(a), 0);
		TS_ASSERT_EQUALS(b.frameDiff(b), 0);
		TS_ASSERT_EQUALS(b.frameDiff(c), 0);

		TS_ASSERT_EQUALS(c.frameDiff(a), 0);
		TS_ASSERT_EQUALS(c.frameDiff(b), 0);
		TS_ASSERT_EQUALS(c.frameDiff(c), 0);
	}


	void test_convert() {
		const Audio::Timestamp a = Audio::Timestamp(10, 1000).addFrames(20);
		const Audio::Timestamp b = Audio::Timestamp(10, 1000/5).addFrames(20/5);
		const Audio::Timestamp c = Audio::Timestamp(10, 1000*2).addFrames(20*2);

		TS_ASSERT_EQUALS(a.convertToFramerate(1000/5), b);
		TS_ASSERT_EQUALS(a.convertToFramerate(1000*2), c);
	}

	void test_equals() {
		const Audio::Timestamp a = Audio::Timestamp(500, 1000);
		Audio::Timestamp b = Audio::Timestamp(0, 1000);
		Audio::Timestamp c = Audio::Timestamp(0, 100);

		TS_ASSERT_EQUALS(a, Audio::Timestamp(0, 500, 1000));

		TS_ASSERT(a != b);
		TS_ASSERT(a != c);
		TS_ASSERT(b == c);

		b = b.addFrames(500);
		c = c.addFrames(50);

		TS_ASSERT(a == b);
		TS_ASSERT(a == c);
		TS_ASSERT(b == c);
	}


	void test_compare() {
		const Audio::Timestamp a = Audio::Timestamp(60, 1000);
		Audio::Timestamp b = Audio::Timestamp(60, 60);
		Audio::Timestamp c = Audio::Timestamp(60, 44100);

		TS_ASSERT(a <= b);
		TS_ASSERT(b <= c);
		TS_ASSERT(a <= c);

		TS_ASSERT(b >= a);
		TS_ASSERT(c >= b);
		TS_ASSERT(c >= a);

		b = b.addFrames(60 / 12);
		c = c.addFrames(44100 / 10);

		TS_ASSERT(a < b);
		TS_ASSERT(b < c);
		TS_ASSERT(a < c);

		TS_ASSERT(b > a);
		TS_ASSERT(c > b);
		TS_ASSERT(c > a);

		TS_ASSERT(a <= b);
		TS_ASSERT(b <= c);
		TS_ASSERT(a <= c);

		TS_ASSERT(b >= a);
		TS_ASSERT(c >= b);
		TS_ASSERT(c >= a);
	}


	void test_framerate() {
		const Audio::Timestamp a = Audio::Timestamp(500, 1000);
		const Audio::Timestamp b = Audio::Timestamp(500, 67);
		const Audio::Timestamp c = Audio::Timestamp(500, 100);
		const Audio::Timestamp d = Audio::Timestamp(500, 44100);

		TS_ASSERT_EQUALS(a.framerate(), (uint)1000);
		TS_ASSERT_EQUALS(b.framerate(), (uint)67);
		TS_ASSERT_EQUALS(c.framerate(), (uint)100);
		TS_ASSERT_EQUALS(d.framerate(), (uint)44100);
	}

	void test_direct_query() {
		const Audio::Timestamp a = Audio::Timestamp(0, 22050);
		const Audio::Timestamp b = a.addFrames(11025);
		const Audio::Timestamp c = Audio::Timestamp(1500, 22050);

		TS_ASSERT_EQUALS(a.secs(), 0);
		TS_ASSERT_EQUALS(a.msecs(), 0);
		TS_ASSERT_EQUALS(a.numberOfFrames(), 0);
		TS_ASSERT_EQUALS(a.totalNumberOfFrames(), 0);

		TS_ASSERT_EQUALS(b.secs(), 0);
		TS_ASSERT_EQUALS(b.msecs(), 500);
		TS_ASSERT_EQUALS(b.numberOfFrames(), 11025);
		TS_ASSERT_EQUALS(b.totalNumberOfFrames(), 11025);

		TS_ASSERT_EQUALS(c.secs(), 1);
		TS_ASSERT_EQUALS(c.msecs(), 1500);
		TS_ASSERT_EQUALS(c.numberOfFrames(), 11025);
		TS_ASSERT_EQUALS(c.totalNumberOfFrames(), 33075);
	}
};
