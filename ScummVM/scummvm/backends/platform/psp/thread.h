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

#ifndef PSP_THREAD_H
#define PSP_THREAD_H

#include <pspthreadman.h>
#include "common/scummsys.h"

// class to inherit for creating threads
class PspThreadable {
protected:
	int _threadId;
	virtual void threadFunction() = 0;	// this function will be called when the thread starts
public:
	PspThreadable() : _threadId(-1) {}					// constructor
	virtual ~PspThreadable() {}							// destructor
	static int __threadCallback(SceSize, void *__this);	// used to get called by sceKernelStartThread() Don't override
	bool threadCreateAndStart(const char *threadName, int priority, int stackSize, bool useVfpu = false);
};

// class for thread utils
class PspThread {
public:
	// static functions
	static void delayMillis(uint32 ms);	// delay the current thread
	static void delayMicros(uint32 us);
};

class PspSemaphore {
private:
	uint32 _handle;
public:
	PspSemaphore(int initialValue, int maxValue=255);
	~PspSemaphore();
	bool take() { return takeWithTimeOut(0); }
	bool takeWithTimeOut(uint32 timeOut);
	bool give(int num=1);
	bool pollForValue(int value);	// check for a certain value
	int numOfWaitingThreads();
	int getValue();
};

class PspMutex {
private:
	PspSemaphore _semaphore;
	int _recursiveCount;
	int _ownerId;
public:
	PspMutex(bool initialValue) : _semaphore(initialValue ? 1 : 0, 255), _recursiveCount(0), _ownerId(0) {}	// initial, max value
	bool lock();
	bool unlock();
	bool poll() { return _semaphore.pollForValue(1); }
	int numOfWaitingThreads() { return _semaphore.numOfWaitingThreads(); }
	bool getValue() { return (bool)_semaphore.getValue(); }
};

class PspCondition {
private:
	PspMutex _mutex;
	int _waitingThreads;
	int _signaledThreads;
	PspSemaphore _waitSem;
	PspSemaphore _doneSem;
public:
	PspCondition() : _mutex(true), _waitingThreads(0), _signaledThreads(0),
								_waitSem(0), _doneSem(0) {}
	void wait(PspMutex &externalMutex);
	void releaseAll();
};

enum ThreadPriority {
	PRIORITY_MAIN_THREAD = 36,
	PRIORITY_TIMER_THREAD = 30,
	PRIORITY_AUDIO_THREAD = 25,		// must be higher than timer or we get stuttering
	PRIORITY_POWER_THREAD = 20,		// quite a light thread
	PRIORITY_DISPLAY_THREAD = 17	// very light thread for callbacks only
};

enum StackSizes {
	STACK_DEFAULT = 4 * 1024,
	STACK_AUDIO_THREAD = 16 * 1024,
	STACK_TIMER_THREAD = 32 * 1024,
	STACK_DISPLAY_THREAD = 2 * 1024,
	STACK_POWER_THREAD = 4 * 1024
};

#endif /* PSP_THREADS_H */
