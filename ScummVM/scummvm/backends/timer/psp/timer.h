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

#ifndef PSP_TIMER_H
#define PSP_TIMER_H

class PspTimer {
public:
	typedef void (* CallbackFunc)(void);
	PspTimer() : _callback(0), _interval(0), _threadId(-1), _init(false) {}
	void stop() { _init = false; }
	bool start();
	~PspTimer() { stop(); }
	void setCallback(CallbackFunc cb) { _callback = cb; }
	void setIntervalMs(uint32 interval) { _interval = interval * 1000; }
	static int thread(SceSize, void *__this);		// static thread to use as bridge
	void timerThread();
private:
	CallbackFunc _callback;	// pointer to timer callback
	uint32 _interval;
	int _threadId;
	bool _init;
};

#endif // PSP_TIMER_H
