/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

// PSP speed and unit tests. Activate in tests.h
// You may also want to build without any engines.

#include "backends/platform/psp/tests.h"

#if defined(PSP_ENABLE_UNIT_TESTS) || defined(PSP_ENABLE_SPEED_TESTS)

#include "common/scummsys.h"
#include <pspiofilemgr_fcntl.h>
#include <pspiofilemgr_stat.h>
#include <pspiofilemgr.h>
#include <pspthreadman.h>
#include <pspsdk.h>
#include <psprtc.h>
#include <stdlib.h>
#include <stdio.h>
#include <psputils.h>
#include "backends/platform/psp/rtc.h"
#include "backends/platform/psp/thread.h"
#include "backends/platform/psp/memory.h"
#include "common/stream.h"
#include "common/file.h"
#include "common/fs.h"

#define UNCACHED(x)		((byte *)(((uint32)(x)) | 0x40000000))	/* make an uncached access */
#define CACHED(x)		((byte *)(((uint32)(x)) & 0xBFFFFFFF))	/* make an uncached access into a cached one */

//#define __PSP_DEBUG_FUNCS__
//#define __PSP_DEBUG_PRINT__

// Results: (333Mhz/222Mhz)
// Getting a tick: 1-2 us
// Getting a time structure: 9/14us
// ie. using a tick and just dividing by 1000 saves us time.

#include "backends/platform/psp/trace.h"

class PspSpeedTests {
public:
	void tickSpeed();
	void getMicrosSpeed();
	void seekSpeed();
	void msReadSpeed();
	void threadFunctionsSpeed();
	void semaphoreSpeed();
	static int threadFunc(SceSize args, void *argp);
	void semaphoreManyThreadSpeed();
	void fastCopySpeed();

private:
	enum {
		MEMCPY_BUFFER_SIZE = 8192
	};
	static PspSemaphore _sem;	// semaphore

	void readAndTime(uint32 bytes, char *buffer, FILE *file);
	void seekAndTime(int bytes, int origin, FILE *file);
	void fastCopySpecificSize(byte *dst, byte *src, uint32 bytes);
	void fastCopyDifferentSizes(byte *dst, byte *src);
	int getThreadIdSpeed();
	void getPrioritySpeed();
	void changePrioritySpeed(int id, int priority);
};

PspSemaphore PspSpeedTests::_sem(0);

void PspSpeedTests::tickSpeed() {
	uint32 ticksPerSecond = sceRtcGetTickResolution();
	PSP_INFO_PRINT("ticksPerSecond[%d]\n", ticksPerSecond);

	uint32 currentTicks1[2];
	uint32 currentTicks2[2];

	sceRtcGetCurrentTick((u64 *)currentTicks1);
	sceRtcGetCurrentTick((u64 *)currentTicks2);
	PSP_INFO_PRINT("current tick[%x %x][%u %u]\n", currentTicks1[0], currentTicks1[1], currentTicks1[0], currentTicks1[1]);
	PSP_INFO_PRINT("current tick[%x %x][%u %u]\n", currentTicks2[0], currentTicks2[1], currentTicks2[0], currentTicks2[1]);

	pspTime time;
	sceRtcSetTick(&time, (u64 *)currentTicks2);
	PSP_INFO_PRINT("current tick in time, year[%d] month[%d] day[%d] hour[%d] minutes[%d] seconds[%d] us[%d]\n", time.year, time.month, time.day, time.hour, time.minutes, time.seconds, time.microseconds);

	pspTime time1;
	pspTime time2;
	sceRtcGetCurrentClockLocalTime(&time1);
	sceRtcGetCurrentClockLocalTime(&time2);
	PSP_INFO_PRINT("time1, year[%d] month[%d] day[%d] hour[%d] minutes[%d] seconds[%d] us[%d]\n", time1.year, time1.month, time1.day, time1.hour, time1.minutes, time1.seconds, time1.microseconds);
	PSP_INFO_PRINT("time2, year[%d] month[%d] day[%d] hour[%d] minutes[%d] seconds[%d] us[%d]\n", time2.year, time2.month, time2.day, time2.hour, time2.minutes, time2.seconds, time2.microseconds);
}

void PspSpeedTests::getMicrosSpeed() {
	uint32 time1, time2, time3, time4;
	time1 = PspRtc::instance().getMicros();
	time2 = PspRtc::instance().getMicros();
	time3 = PspRtc::instance().getMicros();
	time4 = PspRtc::instance().getMicros();

	PSP_INFO_PRINT("getMicros() times: %d, %d, %d\n", time4-time3, time3-time2, time2-time1);
}

void PspSpeedTests::readAndTime(uint32 bytes, char *buffer, FILE *file) {
	uint32 time1 = PspRtc::instance().getMicros();
	// test minimal read
	fread(buffer, bytes, 1, file);
	uint32 time2 = PspRtc::instance().getMicros();

	PSP_INFO_PRINT("Reading %d byte takes %dus\n", bytes, time2-time1);
}

/*
	333MHz/222MHz
	Reading 1 byte takes 2590us / 3167
	Reading 10 byte takes 8us / 9
	Reading 50 byte takes 8us / 11
	Reading 100 byte takes 8us / 11
	Reading 1000 byte takes 915us / 1131
	Reading 2000 byte takes 1806us / 2,284
	Reading 3000 byte takes 2697us / 3,374
	Reading 5000 byte takes 4551us / 5,544
	Reading 6000 byte takes 5356us / 6,676
	Reading 7000 byte takes 6800us / 8,358
	Reading 8000 byte takes 6794us / 8,454
	Reading 9000 byte takes 6782us / 8,563
	Reading 10000 byte takes 8497us / 10,631
	Reading 30000 byte takes 25995us / 32,473
	Reading 80000 byte takes 68457us / 85,291
	Reading 100000 byte takes 85103us / 106,163
*/
// Function to test the impact of MS reads
// These tests can't be done from shell - the cache screws them up
void PspSpeedTests::msReadSpeed() {
	FILE *file;
	file = fopen("ms0:/psp/music/track1.mp3", "r");

	char *buffer = (char *)malloc(2 * 1024 * 1024);

	readAndTime(1, buffer, file);
	readAndTime(10, buffer, file);
	readAndTime(50, buffer, file);
	readAndTime(100, buffer, file);
	readAndTime(1000, buffer, file);
	readAndTime(2000, buffer, file);
	readAndTime(3000, buffer, file);
	readAndTime(5000, buffer, file);
	readAndTime(6000, buffer, file);
	readAndTime(7000, buffer, file);
	readAndTime(8000, buffer, file);
	readAndTime(9000, buffer, file);
	readAndTime(10000, buffer, file);
	readAndTime(30000, buffer, file);
	readAndTime(50000, buffer, file);
	readAndTime(80000, buffer, file);
	readAndTime(100000, buffer, file);

	fclose(file);
	free(buffer);
}

void PspSpeedTests::seekAndTime(int bytes, int origin, FILE *file) {
	char buffer[1000];

	uint32 time1 = PspRtc::instance().getMicros();
	// test minimal read
	fseek(file, bytes, origin);
	uint32 time2 = PspRtc::instance().getMicros();

	PSP_INFO_PRINT("Seeking %d byte from %d took %dus\n", bytes, origin, time2-time1);

	time1 = PspRtc::instance().getMicros();
	// test minimal read
	fread(buffer, 1000, 1, file);
	time2 = PspRtc::instance().getMicros();

	PSP_INFO_PRINT("Reading 1000 bytes took %dus\n", time2-time1);
}

/*
333MHz
Seeking 0 byte from 0 took 946us
Reading 1000 bytes took 1781us
Seeking 5 byte from 0 took 6us
Reading 1000 bytes took 19us
Seeking 1000 byte from 0 took 5us
Reading 1000 bytes took 913us
Seeking 100 byte from 0 took 955us
Reading 1000 bytes took 906us
Seeking 10000 byte from 0 took 963us
Reading 1000 bytes took 905us
Seeking -5 byte from 1 took 1022us
Reading 1000 bytes took 949us
Seeking -100 byte from 1 took 1040us
Reading 1000 bytes took 907us
Seeking 100 byte from 1 took 1044us
Reading 1000 bytes took 930us
Seeking 0 byte from 2 took 7211us
Reading 1000 bytes took 80us
Seeking 10000 byte from 2 took 3636us
Reading 1000 bytes took 110us
*/

void PspSpeedTests::seekSpeed() {
	FILE *file;
	file = fopen("ms0:/psp/music/track1.mp3", "r");

	seekAndTime(0, SEEK_SET, file);
	seekAndTime(5, SEEK_SET, file);
	seekAndTime(1000, SEEK_SET, file);
	seekAndTime(100, SEEK_SET, file);
	seekAndTime(10000, SEEK_SET, file);
	seekAndTime(-5, SEEK_CUR, file);
	seekAndTime(-100, SEEK_CUR, file);
	seekAndTime(100, SEEK_CUR, file);
	seekAndTime(0, SEEK_END, file);
	seekAndTime(-10000, SEEK_END, file);

	fclose(file);
}

// 222: 5-7us
int PspSpeedTests::getThreadIdSpeed() {
	uint32 time1 = PspRtc::instance().getMicros();
	int threadId = sceKernelGetThreadId();
	uint32 time2 = PspRtc::instance().getMicros();

	PSP_INFO_PRINT("Getting thread ID %d took %dus\n", threadId, time2-time1);

	return threadId;
}

// 222: 4-5us
void PspSpeedTests::getPrioritySpeed() {
	uint32 time1 = PspRtc::instance().getMicros();
	int priority = sceKernelGetThreadCurrentPriority();
	uint32 time2 = PspRtc::instance().getMicros();

	PSP_INFO_PRINT("Getting thread priority %d took %dus\n", priority, time2-time1);
}

// 222: 9-10us
void PspSpeedTests::changePrioritySpeed(int id, int priority) {
	uint32 time1 = PspRtc::instance().getMicros();
	sceKernelChangeThreadPriority(id, priority);
	uint32 time2 = PspRtc::instance().getMicros();

	PSP_INFO_PRINT("Changing thread priority to %d for id %d took %dus\n", priority, id, time2-time1);
}

void PspSpeedTests::threadFunctionsSpeed() {
	// very unscientific -- just ballpark
	int id;
	id = getThreadIdSpeed();
	getThreadIdSpeed();
	getPrioritySpeed();
	getPrioritySpeed();
	changePrioritySpeed(id, 30);
	changePrioritySpeed(id, 35);
	changePrioritySpeed(id, 25);

	// test context switch time
	for (int i=0; i<10; i++) {
		uint time1 = PspRtc::instance().getMicros();
		PspThread::delayMicros(0);
		uint time2 = PspRtc::instance().getMicros();
		PSP_INFO_PRINT("poll %d. context switch Time = %dus\n", i, time2-time1);	// 10-15us
	}
}

void PspSpeedTests::semaphoreSpeed() {
	PspSemaphore sem(1);

	uint32 time1 = PspRtc::instance().getMicros();

	sem.take();

	uint32 time2 = PspRtc::instance().getMicros();

	PSP_INFO_PRINT("taking semaphore took %d us\n", time2-time1);	// 10us

	uint32 time3 = PspRtc::instance().getMicros();

	sem.give();

	uint32 time4 = PspRtc::instance().getMicros();
	PSP_INFO_PRINT("releasing semaphore took %d us\n", time4-time3);	//10us-55us
}

int PspSpeedTests::threadFunc(SceSize args, void *argp) {
	PSP_INFO_PRINT("thread %x created.\n", sceKernelGetThreadId());

	_sem.take();

	PSP_INFO_PRINT("grabbed semaphore. Quitting thread\n");

	return 0;
}

void PspSpeedTests::semaphoreManyThreadSpeed() {

	// create 4 threads
	for (int i=0; i<4; i++) {
		int thid = sceKernelCreateThread("my_thread", PspSpeedTests::threadFunc, 0x18, 0x10000, THREAD_ATTR_USER, NULL);
		sceKernelStartThread(thid, 0, 0);
	}

	PSP_INFO_PRINT("main thread. created threads\n");

	uint32 threads = _sem.numOfWaitingThreads();
	while (threads < 4) {
		threads = _sem.numOfWaitingThreads();
		PSP_INFO_PRINT("main thread: waiting threads[%d]\n", threads);
	}

	PSP_INFO_PRINT("main: semaphore value[%d]\n", _sem.getValue());
	PSP_INFO_PRINT("main thread: waiting threads[%d]\n", _sem.numOfWaitingThreads());

	_sem.give(4);
}

void PspSpeedTests::fastCopySpecificSize(byte *dst, byte *src, uint32 bytes) {
	uint32 time1, time2;
	uint32 fastcopyTime, memcpyTime;
	const int iterations = 2000;
	int intc;

	intc = pspSdkDisableInterrupts();

	time1 = PspRtc::instance().getMicros();
	for (int i=0; i<iterations; i++) {
		PspMemory::fastCopy(dst, src, bytes);
	}
	time2 = PspRtc::instance().getMicros();

	pspSdkEnableInterrupts(intc);

	fastcopyTime = time2-time1;

	intc = pspSdkDisableInterrupts();

	time1 = PspRtc::instance().getMicros();
	for (int i=0; i<iterations; i++) {
		memcpy(dst, src, bytes);
	}
	time2 = PspRtc::instance().getMicros();

	pspSdkEnableInterrupts(intc);

	memcpyTime = time2-time1;

	PSP_INFO_PRINT("%d bytes. memcpy[%d], fastcopy[%d]\n", bytes, memcpyTime, fastcopyTime);
}

void PspSpeedTests::fastCopyDifferentSizes(byte *dst, byte *src) {
	PSP_INFO_PRINT("\nsrc[%p], dst[%p]\n", src, dst);
	fastCopySpecificSize(dst, src, 1);
	fastCopySpecificSize(dst, src, 2);
	fastCopySpecificSize(dst, src, 3);
	fastCopySpecificSize(dst, src, 4);
	fastCopySpecificSize(dst, src, 5);
	fastCopySpecificSize(dst, src, 8);
	fastCopySpecificSize(dst, src, 10);
	fastCopySpecificSize(dst, src, 16);
	fastCopySpecificSize(dst, src, 32);
	fastCopySpecificSize(dst, src, 50);
	fastCopySpecificSize(dst, src, 100);
	fastCopySpecificSize(dst, src, 500);
	fastCopySpecificSize(dst, src, 1024);
	fastCopySpecificSize(dst, src, 2048);
}

void PspSpeedTests::fastCopySpeed() {
	PSP_INFO_PRINT("running fastCopy speed test\n");

	uint32 *bufferSrc32 = (uint32 *)memalign(16, MEMCPY_BUFFER_SIZE);
	uint32 *bufferDst32 = (uint32 *)memalign(16, MEMCPY_BUFFER_SIZE);

	// fill buffer 1
	for (int i=0; i<MEMCPY_BUFFER_SIZE/4; i++)
		bufferSrc32[i] = i | (((MEMCPY_BUFFER_SIZE/4)-i)<<16);

	// print buffer
	for (int i=0; i<50; i++)
		PSP_INFO_PRINT("%x ", bufferSrc32[i]);
	PSP_INFO_PRINT("\n");

	byte *bufferSrc = ((byte *)bufferSrc32);
	byte *bufferDst = ((byte *)bufferDst32);

	PSP_INFO_PRINT("\n\ndst and src cached: -----------------\n");
	fastCopyDifferentSizes(bufferDst, bufferSrc);
	fastCopyDifferentSizes(bufferDst+1, bufferSrc+1);
	fastCopyDifferentSizes(bufferDst, bufferSrc+1);
	fastCopyDifferentSizes(bufferDst+1, bufferSrc);

	PSP_INFO_PRINT("\n\ndst cached, src uncached: -----------------\n");
	bufferSrc = UNCACHED(bufferSrc);
	fastCopyDifferentSizes(bufferDst, bufferSrc);
	fastCopyDifferentSizes(bufferDst+1, bufferSrc+1);
	fastCopyDifferentSizes(bufferDst, bufferSrc+1);
	fastCopyDifferentSizes(bufferDst+1, bufferSrc);

	PSP_INFO_PRINT("\n\ndst uncached, src uncached: --------------\n");
	bufferDst = UNCACHED(bufferDst);
	fastCopyDifferentSizes(bufferDst, bufferSrc);
	fastCopyDifferentSizes(bufferDst+1, bufferSrc+1);
	fastCopyDifferentSizes(bufferDst, bufferSrc+1);
	fastCopyDifferentSizes(bufferDst+1, bufferSrc);

	PSP_INFO_PRINT("\n\ndst uncached, src cached: -------------------\n");
	bufferSrc = CACHED(bufferSrc);
	fastCopyDifferentSizes(bufferDst, bufferSrc);
	fastCopyDifferentSizes(bufferDst+1, bufferSrc+1);
	fastCopyDifferentSizes(bufferDst, bufferSrc+1);
	fastCopyDifferentSizes(bufferDst+1, bufferSrc);


	free(bufferSrc32);
	free(bufferDst32);
}

//-------Unit Tests -------------------------------

class PspUnitTests {
public:
	void testFastCopy();
	bool testFileSystem();

private:
	enum {
		MEMCPY_BUFFER_SIZE = 8192
	};

	void fastCopySpecificSize(byte *dst, byte *src, uint32 bytes, bool swap = false);
	void fastCopyDifferentSizes(byte *dst, byte *src, bool swap = false);

};

void PspUnitTests::testFastCopy() {
	PSP_INFO_PRINT("running fastcopy unit test ***********\n");
	PSP_INFO_PRINT("this test requires the test flag to be on in fastCopy\n\n");

	uint32 *bufferSrc32 = (uint32 *)memalign(16, MEMCPY_BUFFER_SIZE);
	uint32 *bufferDst32 = (uint32 *)memalign(16, MEMCPY_BUFFER_SIZE);

	// fill buffer 1
	for (int i=0; i<MEMCPY_BUFFER_SIZE/4; i++)
		bufferSrc32[i] = i | (((MEMCPY_BUFFER_SIZE/4)-i)<<16);

	// print buffer
	for (int i=0; i<50; i++)
		PSP_INFO_PRINT("%x ", bufferSrc32[i]);
	PSP_INFO_PRINT("\n");

	byte *bufferSrc = ((byte *)bufferSrc32);
	byte *bufferDst = ((byte *)bufferDst32);

	fastCopyDifferentSizes(bufferDst, bufferSrc, true);
	fastCopyDifferentSizes(bufferDst+1, bufferSrc+1);
	fastCopyDifferentSizes(bufferDst+2, bufferSrc+2, true);
	fastCopyDifferentSizes(bufferDst+3, bufferSrc+3);
	fastCopyDifferentSizes(bufferDst, bufferSrc+1);
	fastCopyDifferentSizes(bufferDst, bufferSrc+2, true);
	fastCopyDifferentSizes(bufferDst+2, bufferSrc, true);
	fastCopyDifferentSizes(bufferDst, bufferSrc+3);
	fastCopyDifferentSizes(bufferDst+1, bufferSrc+2);
	fastCopyDifferentSizes(bufferDst+1, bufferSrc+3);
	fastCopyDifferentSizes(bufferDst+2, bufferSrc+1);
	fastCopyDifferentSizes(bufferDst+2, bufferSrc+3);
	fastCopyDifferentSizes(bufferDst+3, bufferSrc+1);
	fastCopyDifferentSizes(bufferDst+3, bufferSrc+2);

	free(bufferSrc32);
	free(bufferDst32);
}

void PspUnitTests::fastCopyDifferentSizes(byte *dst, byte *src, bool swap) {
	fastCopySpecificSize(dst, src, 1);
	fastCopySpecificSize(dst, src, 2, swap);
	fastCopySpecificSize(dst, src, 4, swap);
	fastCopySpecificSize(dst, src, 6, swap);
	fastCopySpecificSize(dst, src, 8, swap);
	fastCopySpecificSize(dst, src, 9);
	fastCopySpecificSize(dst, src, 10, swap);
	fastCopySpecificSize(dst, src, 11);
	fastCopySpecificSize(dst, src, 12, swap);
	fastCopySpecificSize(dst, src, 13);
	fastCopySpecificSize(dst, src, 14, swap);
	fastCopySpecificSize(dst, src, 15);
	fastCopySpecificSize(dst, src, 16, swap);
	fastCopySpecificSize(dst, src, 17);
	fastCopySpecificSize(dst, src, 18, swap);
	fastCopySpecificSize(dst, src, 19);
	fastCopySpecificSize(dst, src, 20, swap);
	fastCopySpecificSize(dst, src, 32, swap);
	fastCopySpecificSize(dst, src, 33);
	fastCopySpecificSize(dst, src, 34, swap);
	fastCopySpecificSize(dst, src, 35);
	fastCopySpecificSize(dst, src, 36, swap);
	fastCopySpecificSize(dst, src, 50, swap);
	fastCopySpecificSize(dst, src, 100, swap);
	fastCopySpecificSize(dst, src, 500, swap);
	fastCopySpecificSize(dst, src, 1000, swap);
}

void PspUnitTests::fastCopySpecificSize(byte *dst, byte *src, uint32 bytes, bool swap) {
	memset(dst, 0, bytes);
	PspMemory::fastCopy(dst, src, bytes);

	if (swap) {	// test swap also
		memset(dst, 0, bytes);

		// pixelformat for swap
		PSPPixelFormat format;
		format.set(PSPPixelFormat::Type_4444, true);

		PspMemory::fastSwap(dst, src, bytes, format);
	}
}

// This function leaks. For now I don't care
bool PspUnitTests::testFileSystem() {
	// create memory
	const uint32 BufSize = 32 * 1024;
	char* buffer = new char[BufSize];
	int i;
	Common::WriteStream *wrStream;
	Common::SeekableReadStream *rdStream;

	PSP_INFO_PRINT("testing fileSystem...\n");

	// fill buffer
	for (i=0; i<(int)BufSize; i += 4) {
		buffer[i] = 'A';
		buffer[i + 1] = 'B';
		buffer[i + 2] = 'C';
		buffer[i + 3] = 'D';
	}

	// create a file
	const char *path = "./file.test";
	Common::FSNode file(path);

	PSP_INFO_PRINT("creating write stream...\n");

	wrStream = file.createWriteStream();
	if (!wrStream) {
		PSP_ERROR("%s couldn't be created.\n", path);
		return false;
	}

	// write contents
	char* index = buffer;
	int32 totalLength = BufSize;
	int32 curLength = 50;

	PSP_INFO_PRINT("writing...\n");

	while(totalLength - curLength > 0) {
		if ((int)wrStream->write(index, curLength) != curLength) {
			PSP_ERROR("couldn't write %d bytes\n", curLength);
			return false;
		}
		totalLength -= curLength;
		index += curLength;
		//curLength *= 2;
		//PSP_INFO_PRINT("write\n");
	}

	// write the rest
	if ((int)wrStream->write(index, totalLength) != totalLength) {
		PSP_ERROR("couldn't write %d bytes\n", curLength);
		return false;
	}

	delete wrStream;

	PSP_INFO_PRINT("reading...\n");

	rdStream = file.createReadStream();
	if (!rdStream) {
		PSP_ERROR("%s couldn't be created.\n", path);
		return false;
	}

	// seek to beginning
	if (!rdStream->seek(0, SEEK_SET)) {
		PSP_ERROR("couldn't seek to the beginning after writing the file\n");
		return false;
	}

	// read the contents
	char *readBuffer = new char[BufSize + 4];
	memset(readBuffer, 0, (BufSize + 4));
	index = readBuffer;
	while (rdStream->read(index, 100) == 100) {
		index += 100;
	}

	if (!rdStream->eos()) {
		PSP_ERROR("didn't find EOS at end of stream\n");
		return false;
	}

	// compare
	for (i=0; i<(int)BufSize; i++)
		if (buffer[i] != readBuffer[i]) {
			PSP_ERROR("reading/writing mistake at %x. Got %x instead of %x\n", i, readBuffer[i], buffer[i]);
			return false;
		}

	// Check for exceeding limit
	for (i=0; i<4; i++) {
		if (readBuffer[BufSize + i]) {
			PSP_ERROR("read exceeded limits. %d = %x\n", BufSize + i, readBuffer[BufSize + i]);
		}
	}

	delete rdStream;

	PSP_INFO_PRINT("writing...\n");

	wrStream = file.createWriteStream();
	if (!wrStream) {
		PSP_ERROR("%s couldn't be created.\n", path);
		return false;
	}

	const char *phrase = "Jello is really fabulous";
	uint32 phraseLen = strlen(phrase);

	int ret;
	if ((ret = wrStream->write(phrase, phraseLen)) != (int)phraseLen) {
		PSP_ERROR("couldn't write phrase. Got %d instead of %d\n", ret, phraseLen);
		return false;
	}

	PSP_INFO_PRINT("reading...\n");

	delete wrStream;
	rdStream = file.createReadStream();
	if (!rdStream) {
		PSP_ERROR("%s couldn't be created.\n", path);
		return false;
	}

	char *readPhrase = new char[phraseLen + 2];
	memset(readPhrase, 0, phraseLen + 2);

	if ((ret = rdStream->read(readPhrase, phraseLen) != phraseLen)) {
		PSP_ERROR("read error on phrase. Got %d instead of %d\n", ret, phraseLen);
		return false;
	}

	for (i=0; i<(int)phraseLen; i++) {
		if (readPhrase[i] != phrase[i]) {
			PSP_ERROR("bad read/write in phrase. At %d, %x != %x\n", i, readPhrase[i], phrase[i]);
			return false;
		}
	}

	// check for exceeding
	if (readPhrase[i] != 0) {
		PSP_ERROR("found excessive copy in phrase. %c at %d\n", readPhrase[i], i);
		return false;
	}

	PSP_INFO_PRINT("trying to read end...\n");

	// seek to end
	if (!rdStream->seek(0, SEEK_END)) {
		PSP_ERROR("couldn't seek to end for append\n");
		return false;
	};

	// try to read
	if (rdStream->read(readPhrase, 2) || !rdStream->eos()) {
		PSP_ERROR("was able to read at end of file\n");
		return false;
	}

	PSP_INFO_PRINT("ok\n");
	return true;
}

void psp_tests() {
	PSP_INFO_PRINT("in tests\n");

#ifdef PSP_ENABLE_SPEED_TESTS
	// Speed tests
	PspSpeedTests speedTests;
	speedTests.tickSpeed();
	speedTests.getMicrosSpeed();
	speedTests.msReadSpeed();
	speedTests.seekSpeed();
	speedTests.msReadSpeed();
	speedTests.threadFunctionsSpeed();
	speedTests.semaphoreSpeed();
	speedTests.semaphoreManyThreadSpeed();
	speedTests.fastCopySpeed();
#endif

#ifdef PSP_ENABLE_UNIT_TESTS
	// Unit tests
	PspUnitTests unitTests;

	//unitTests.testFastCopy();
	unitTests.testFileSystem();
#endif
}

#endif /* (PSP_ENABLE_UNIT_TESTS) || defined(PSP_ENABLE_SPEED_TESTS) */
