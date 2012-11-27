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

#include <pspthreadman.h>

#include "backends/platform/psp/thread.h"
#include "backends/platform/psp/trace.h"

// Class PspThreadable --------------------------------------------------
// Inherit this to create C++ threads easily

bool PspThreadable::threadCreateAndStart(const char *threadName, int priority, int stackSize, bool useVfpu /*= false*/) {
	DEBUG_ENTER_FUNC();

	if (_threadId != -1) {
		PSP_ERROR("thread already created!\n");
		return false;
	}

	_threadId = sceKernelCreateThread(threadName, __threadCallback, priority, stackSize, THREAD_ATTR_USER, 0);	// add VFPU support

	if (_threadId < 0) {
		PSP_ERROR("failed to create %s thread. Error code %d\n", threadName, _threadId);
		return false;
	}

	// We want to pass the pointer to this, but we'll have to take address of this so use a little trick
	PspThreadable *_this = this;

	if (sceKernelStartThread(_threadId, sizeof(uint32 *), &_this) < 0) {
		PSP_ERROR("failed to start %s thread id[%d]\n", threadName, _threadId);
		return false;
	}

	PSP_DEBUG_PRINT("Started %s thread with id[%x]\n", threadName, _threadId);

	return true;
}

// Callback function to be called by PSP kernel
int PspThreadable::__threadCallback(SceSize, void *__this) {
	DEBUG_ENTER_FUNC();

	PspThreadable *_this = *(PspThreadable **)__this;	// Dereference the copied value which was 'this'

	_this->threadFunction();	// call the virtual function

	return 0;
}

// PspThread class
// Utilities to access general thread functions

void PspThread::delayMillis(uint32 ms) {
	sceKernelDelayThread(ms * 1000);
}

void PspThread::delayMicros(uint32 us) {
	sceKernelDelayThread(us);
}

// Class PspSemaphore ------------------------------------------------
//#define __PSP_DEBUG_FUNCS__	/* For debugging function calls */
//#define __PSP_DEBUG_PRINT__	/* For debug printouts */

#include "backends/platform/psp/trace.h"

PspSemaphore::PspSemaphore(int initialValue, int maxValue/*=255*/) {
	DEBUG_ENTER_FUNC();
	_handle = 0;
	_handle = (uint32)sceKernelCreateSema("ScummVM Sema", 0 /* attr */,
								  initialValue, maxValue,
								  0 /*option*/);
	if (!_handle)
		PSP_ERROR("failed to create semaphore.\n");
}

PspSemaphore::~PspSemaphore() {
	DEBUG_ENTER_FUNC();
	if (_handle)
		if (sceKernelDeleteSema((SceUID)_handle) < 0)
			PSP_ERROR("failed to delete semaphore.\n");
}

int PspSemaphore::numOfWaitingThreads() {
	DEBUG_ENTER_FUNC();
	SceKernelSemaInfo info;
	info.numWaitThreads = 0;

	if (sceKernelReferSemaStatus((SceUID)_handle, &info) < 0)
		PSP_ERROR("failed to retrieve semaphore info for handle %d\n", _handle);

	return info.numWaitThreads;
}

int PspSemaphore::getValue() {
	DEBUG_ENTER_FUNC();
	SceKernelSemaInfo info;
	info.currentCount = 0;

	if (sceKernelReferSemaStatus((SceUID)_handle, &info) < 0)
		PSP_ERROR("failed to retrieve semaphore info for handle %d\n", _handle);

	return info.currentCount;
}

bool PspSemaphore::pollForValue(int value) {
	DEBUG_ENTER_FUNC();
	if (sceKernelPollSema((SceUID)_handle, value) < 0)
		return false;

	return true;
}

// false: timeout or error
bool PspSemaphore::takeWithTimeOut(uint32 timeOut) {
	DEBUG_ENTER_FUNC();

	uint32 *pTimeOut = 0;
	if (timeOut)
		pTimeOut = &timeOut;

	if (sceKernelWaitSema(_handle, 1, pTimeOut) < 0)	// we always wait for 1
		return false;
	return true;
}

bool PspSemaphore::give(int num /*=1*/) {
	DEBUG_ENTER_FUNC();

	if (sceKernelSignalSema((SceUID)_handle, num) < 0)
		return false;
	return true;
}

// Class PspMutex ------------------------------------------------------------

bool PspMutex::lock() {
	DEBUG_ENTER_FUNC();
	int threadId = sceKernelGetThreadId();
	bool ret = true;

	if (_ownerId == threadId) {
		_recursiveCount++;
	} else {
		ret = _semaphore.take();
		_ownerId = threadId;
		_recursiveCount = 0;
	}
	return ret;
}

bool PspMutex::unlock() {
	DEBUG_ENTER_FUNC();
	int threadId = sceKernelGetThreadId();
	bool ret = true;

	if (_ownerId != threadId) {
		PSP_ERROR("attempt to unlock mutex by thread[%x] as opposed to owner[%x]\n",
			threadId, _ownerId);
		return false;
	}

	if (_recursiveCount) {
		_recursiveCount--;
	} else {
		_ownerId = 0;
		ret = _semaphore.give(1);
	}
	return ret;
}

// Class PspCondition -------------------------------------------------

// Release all threads waiting on the condition
void PspCondition::releaseAll() {
        _mutex.lock();
        if (_waitingThreads > _signaledThreads) {	// we have signals to issue
                int numWaiting = _waitingThreads - _signaledThreads;	// threads we haven't signaled
                _signaledThreads = _waitingThreads;

				_waitSem.give(numWaiting);
                _mutex.unlock();
                for (int i=0; i<numWaiting; i++)	// wait for threads to tell us they're awake
					_doneSem.take();
        } else {
                _mutex.unlock();
        }
}

// Mutex must be taken before entering wait
void PspCondition::wait(PspMutex &externalMutex) {
        _mutex.lock();
        _waitingThreads++;
        _mutex.unlock();

        externalMutex.unlock();	// must unlock external mutex

		_waitSem.take();	// sleep on the wait semaphore

		// let the signaling thread know we're done
		_mutex.lock();
        if (_signaledThreads > 0 ) {
                _doneSem.give();	// let the thread know
                _signaledThreads--;
        }
        _waitingThreads--;
        _mutex.unlock();

        externalMutex.lock();		// must lock external mutex here for continuation
}
