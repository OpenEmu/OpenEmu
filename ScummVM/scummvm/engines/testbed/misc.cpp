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
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */

#include "testbed/misc.h"
#include "common/timer.h"

namespace Testbed {

Common::String MiscTests::getHumanReadableFormat(const TimeDate &td) {
	return Common::String::format("%d:%d:%d on %d/%d/%d (dd/mm/yyyy)", td.tm_hour, td.tm_min, td.tm_sec, td.tm_mday, td.tm_mon + 1, td.tm_year + 1900);
}

void MiscTests::timerCallback(void *arg) {
	// Increment arg which actually points to an int
	int &valToModify = *((int *) arg);
	valToModify = 999; // some arbitrary value
}

void MiscTests::criticalSection(void *arg) {
	SharedVars &sv = *((SharedVars *)arg);

	Testsuite::logDetailedPrintf("Before critical section: %d %d\n", sv.first, sv.second);
	g_system->lockMutex(sv.mutex);

	// In any case, the two vars must be equal at entry, if mutex works fine.
	// verify this here.
	if (sv.first != sv.second) {
		sv.resultSoFar = false;
	}

	sv.first++;
	g_system->delayMillis(1000);

	// This should bring no change as well in the difference between vars
	// verify this too.
	if (sv.second + 1 != sv.first) {
		sv.resultSoFar = false;
	}

	sv.second *= sv.first;
	Testsuite::logDetailedPrintf("After critical section: %d %d\n", sv.first, sv.second);
	g_system->unlockMutex(sv.mutex);

	g_system->getTimerManager()->removeTimerProc(criticalSection);
}

TestExitStatus MiscTests::testDateTime() {

	if (ConfParams.isSessionInteractive()) {
		if (Testsuite::handleInteractiveInput("Testing the date time API implementation", "Continue", "Skip", kOptionRight)) {
			Testsuite::logPrintf("Info! Date time tests skipped by the user.\n");
			return kTestSkipped;
		}

		Testsuite::writeOnScreen("Verifying Date-Time...", Common::Point(0, 100));
	}

	TimeDate t1, t2;
	g_system->getTimeAndDate(t1);
	Testsuite::logDetailedPrintf("Current Time and Date: ");
	Common::String dateTimeNow;
	dateTimeNow = getHumanReadableFormat(t1);

	if (ConfParams.isSessionInteractive()) {
		// Directly verify date
		dateTimeNow = "We expect the current date time to be " + dateTimeNow;
		if (Testsuite::handleInteractiveInput(dateTimeNow, "Correct!", "Wrong", kOptionRight)) {
			return kTestFailed;
		}
	}

	g_system->getTimeAndDate(t1);
	dateTimeNow = getHumanReadableFormat(t1);
	Testsuite::logDetailedPrintf("%s\n", dateTimeNow.c_str());
	// Now, Put some delay
	g_system->delayMillis(2000);
	g_system->getTimeAndDate(t2);
	Testsuite::logDetailedPrintf("Time and Date 2s later: ");
	dateTimeNow = getHumanReadableFormat(t2);
	Testsuite::logDetailedPrintf("%s\n", dateTimeNow.c_str());

	if (t1.tm_year == t2.tm_year && t1.tm_mon == t2.tm_mon && t1.tm_mday == t2.tm_mday) {
		if (t1.tm_mon == t2.tm_mon && t1.tm_year == t2.tm_year) {
			// Ignore lag due to processing time
			if (t1.tm_sec + 2 == t2.tm_sec) {
				return kTestPassed;
			}
		}
	}
	return kTestFailed;
}

TestExitStatus MiscTests::testTimers() {
	int valToModify = 0;
	if (g_system->getTimerManager()->installTimerProc(timerCallback, 100000, &valToModify, "testbedTimer")) {
		g_system->delayMillis(150);
		g_system->getTimerManager()->removeTimerProc(timerCallback);

		if (999 == valToModify) {
			return kTestPassed;
		}
	}
	return kTestFailed;
}

TestExitStatus MiscTests::testMutexes() {

	if (ConfParams.isSessionInteractive()) {
		if (Testsuite::handleInteractiveInput("Testing the Mutual Exclusion API implementation", "Continue", "Skip", kOptionRight)) {
			Testsuite::logPrintf("Info! Mutex tests skipped by the user.\n");
			return kTestSkipped;
		}
		Testsuite::writeOnScreen("Installing mutex", Common::Point(0, 100));
	}

	SharedVars sv = {1, 1, true, g_system->createMutex()};

	if (g_system->getTimerManager()->installTimerProc(criticalSection, 100000, &sv, "testbedMutex")) {
		g_system->delayMillis(150);
	}

	g_system->lockMutex(sv.mutex);
	sv.first++;
	g_system->delayMillis(1000);
	sv.second *= sv.first;
	g_system->unlockMutex(sv.mutex);

	// wait till timed process exits
	if (ConfParams.isSessionInteractive()) {
		Testsuite::writeOnScreen("Waiting for 3s so that timed processes finish", Common::Point(0, 100));
	}
	g_system->delayMillis(3000);

	Testsuite::logDetailedPrintf("Final Value: %d %d\n", sv.first, sv.second);
	g_system->deleteMutex(sv.mutex);

	if (sv.resultSoFar && 6 == sv.second) {
		return kTestPassed;
	}

	return kTestFailed;
}

MiscTestSuite::MiscTestSuite() {
	addTest("Datetime", &MiscTests::testDateTime, false);
	addTest("Timers", &MiscTests::testTimers, false);
	addTest("Mutexes", &MiscTests::testMutexes, false);
}

} // End of namespace Testbed
