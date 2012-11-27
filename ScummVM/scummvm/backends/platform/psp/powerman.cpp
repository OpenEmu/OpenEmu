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

#include <psppower.h>
#include <pspthreadman.h>

#include "backends/platform/psp/powerman.h"
#include "engine.h"

//#define __PSP_DEBUG_FUNCS__	/* can put this locally too */
//#define __PSP_DEBUG_PRINT__
#include "backends/platform/psp/trace.h"

namespace Common {
DECLARE_SINGLETON(PowerManager);
}

// Function to debug the Power Manager (we have no output to screen)
inline void PowerManager::debugPM() {
	PSP_DEBUG_PRINT("PM status[%d]. Listcount[%d]. CriticalCount[%d]. ThreadId[%x]. Error[%d]\n",
	                _PMStatus, _listCounter, _criticalCounter, sceKernelGetThreadId(), _error);
}


/*******************************************
*
*	Constructor
*
********************************************/
PowerManager::PowerManager() : _pauseFlag(false), _pauseFlagOld(false), _pauseClientState(UNPAUSED),
								_suspendFlag(false), _flagMutex(true), _listMutex(true),
								_criticalCounter(0), _listCounter(0), _error(0), _PMStatus(kInitDone) {}

/*******************************************
*
*	Function to register to be notified when suspend/resume time comes
*
********************************************/
bool PowerManager::registerForSuspend(Suspendable *item) {
	DEBUG_ENTER_FUNC();
	// Register in list
	debugPM();

	_listMutex.lock();

	_suspendList.push_front(item);
	_listCounter++;

	_listMutex.unlock();

	debugPM();

	return true;
}

/*******************************************
*
*	Function to unregister to be notified when suspend/resume time comes
*
********************************************/
bool PowerManager::unregisterForSuspend(Suspendable *item) {
	DEBUG_ENTER_FUNC();
	debugPM();

	// Unregister from stream list
	_listMutex.lock();

	_suspendList.remove(item);
	_listCounter--;

	_listMutex.unlock();

	debugPM();
	return true;
}

/*******************************************
*
*	Destructor
*
********************************************/
PowerManager::~PowerManager() {
	_PMStatus = kDestroyPM;
}

/*******************************************
*
*	Unsafe function to poll for a pause event (first stage of suspending)
*   Only for pausing the engine, which doesn't need high synchronization ie. we don't care if it misreads
*   the flag a couple of times since there is NO mutex protection (for performance reasons).
*   Polling the engine happens regularly.
*	On the other hand, we don't know if there will be ANY polling which prevents us from using proper events.
*
********************************************/
void PowerManager::pollPauseEngine() {
	DEBUG_ENTER_FUNC();


	bool pause = _pauseFlag;		// We copy so as not to have multiple values

	if (pause != _pauseFlagOld) {
		if (g_engine) { // Check to see if we have an engine
			if (pause && _pauseClientState == UNPAUSED) {
				_pauseClientState = PAUSING;		// Tell PM we're in the middle of pausing
				g_engine->pauseEngine(true);
				PSP_DEBUG_PRINT_FUNC("Pausing engine\n");
				_pauseClientState = PAUSED;			// Tell PM we're done pausing
			} else if (!pause && _pauseClientState == PAUSED) {
				g_engine->pauseEngine(false);
				PSP_DEBUG_PRINT_FUNC("Unpausing for resume\n");
				_pauseClientState = UNPAUSED;		// Tell PM we're unpaused
			}
		}
		_pauseFlagOld = pause;
	}
}

/*******************************************
*
*	Function to block on a suspend, then start a non-suspendable critical section
*   Use this for large or REALLY critical critical-sections.
*	Make sure to call endCriticalSection or the PSP won't suspend.
*   returns true if blocked, false if not blocked
********************************************/

bool PowerManager::beginCriticalSection() {
	DEBUG_ENTER_FUNC();

	bool ret = false;

	_flagMutex.lock();

	// Check the access flag
	if (_suspendFlag) {
		ret = true;

		PSP_DEBUG_PRINT("I got blocked. ThreadId[%x]\n", sceKernelGetThreadId());
		debugPM();

		_threadSleep.wait(_flagMutex);

		PSP_DEBUG_PRINT_FUNC("I got released. ThreadId[%x]\n", sceKernelGetThreadId());
		debugPM();
	}

	// Now prevent the PM from suspending until we're done
	_criticalCounter++;

	_flagMutex.unlock();

	return ret;
}

// returns success = true
void PowerManager::endCriticalSection() {
	DEBUG_ENTER_FUNC();

	_flagMutex.lock();

	// We're done with our critical section
	_criticalCounter--;

	if (_criticalCounter <= 0) {
		if (_suspendFlag) {		// If the PM is sleeping, this flag must be set
				PSP_DEBUG_PRINT_FUNC("PM is asleep. Waking it up.\n");
				debugPM();

				_pmSleep.releaseAll();

				PSP_DEBUG_PRINT_FUNC("Woke up the PM\n");

				debugPM();
		}

		if (_criticalCounter < 0) {	// Check for bad usage of critical sections
			PSP_ERROR("Critical counter[%d]!!!\n", _criticalCounter);
			debugPM();
		}
	}

	_flagMutex.unlock();
}

/*******************************************
*
*	Callback function to be called to put every Suspendable to suspend
*
********************************************/
void PowerManager::suspend() {
	DEBUG_ENTER_FUNC();

	if (_pauseFlag)
		return;					// Very important - make sure we only suspend once

	scePowerLock(0);			// Also critical to make sure PSP doesn't suspend before we're done

	// The first stage of suspend is pausing the engine if possible. We don't want to cause files
	// to block, or we might not get the engine to pause. On the other hand, we might wait for polling
	// and it'll never happen. We also want to do this w/o mutexes (for speed) which is ok in this case.
	_pauseFlag = true;

	_PMStatus = kWaitForClientPause;

	// Now we wait, giving the engine thread some time to find our flag.
	for (int i = 0; i < 10 && _pauseClientState == UNPAUSED; i++)
		PspThread::delayMicros(50000);	// We wait 50 msec x 10 times = 0.5 seconds

	if (_pauseClientState == PAUSING) {	// Our event has been acknowledged. Let's wait until the client is done.
		_PMStatus = kWaitForClientToFinishPausing;

		while (_pauseClientState != PAUSED)
			PspThread::delayMicros(50000);	// We wait 50 msec at a time
	}

	// It's possible that the polling thread missed our pause event, but there's
	// nothing we can do about that.
	// We can't know if there's polling going on or not.
	// It's usually not a critical thing anyway.

	_PMStatus = kGettingFlagMutexSuspend;

	// Now we set the suspend flag to true to cause reading threads to block
	_flagMutex.lock();

	_PMStatus = kGotFlagMutexSuspend;

	_suspendFlag = true;

	// Check if anyone is in a critical section. If so, we'll wait for them
	if (_criticalCounter > 0) {
		_PMStatus = kWaitCritSectionSuspend;

		_pmSleep.wait(_flagMutex);

		_PMStatus = kDoneWaitingCritSectionSuspend;
	}

	_flagMutex.unlock();

	_PMStatus = kGettingListMutexSuspend;

	// Loop over list, calling suspend()
	_listMutex.lock();

	_PMStatus = kIteratingListSuspend;
	// Iterate
	Common::List<Suspendable *>::iterator i;

	for (i = _suspendList.begin(); i != _suspendList.end(); ++i) {
		(*i)->suspend();
	}
	_PMStatus = kDoneIteratingListSuspend;

	_listMutex.unlock();
	_PMStatus = kDoneSuspend;

	scePowerUnlock(0);				// Allow the PSP to go to sleep now

	_PMStatus = kDonePowerUnlock;
}

/*******************************************
*
*	Callback function to resume every Suspendable
*
********************************************/
void PowerManager::resume() {
	DEBUG_ENTER_FUNC();

	_PMStatus = kBeginResume;

	// Make sure we can't get another suspend
	scePowerLock(0);

	_PMStatus = kCheckingPauseFlag;

	if (!_pauseFlag)
		return;						// Make sure we can only resume once

	_PMStatus = kGettingListMutexResume;

	// First we notify our Suspendables. Loop over list, calling resume()
	_listMutex.lock();

	_PMStatus = kIteratingListResume;

	// Iterate
	Common::List<Suspendable *>::iterator i = _suspendList.begin();

	for (; i != _suspendList.end(); ++i) {
		(*i)->resume();
	}

	_PMStatus = kDoneIteratingListResume;

	_listMutex.unlock();

	_PMStatus = kGettingFlagMutexResume;

	// Now we set the suspend flag to false
	_flagMutex.lock();

	_PMStatus = kGotFlagMutexResume;

	_suspendFlag = false;

	_PMStatus = kSignalSuspendedThreadsResume;

	// Signal the threads to wake up
	_threadSleep.releaseAll();

	_PMStatus = kDoneSignallingSuspendedThreadsResume;

	_flagMutex.unlock();

	_PMStatus = kDoneResume;

	_pauseFlag = false;	// Signal engine to unpause -- no mutex needed

	scePowerUnlock(0);	// Allow new suspends
}
