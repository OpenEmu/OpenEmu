/**
 ** Supermodel
 ** A Sega Model 3 Arcade Emulator.
 ** Copyright 2011 Bart Trzynadlowski, Nik Henson
 **
 ** This file is part of Supermodel.
 **
 ** Supermodel is free software: you can redistribute it and/or modify it under
 ** the terms of the GNU General Public License as published by the Free 
 ** Software Foundation, either version 3 of the License, or (at your option)
 ** any later version.
 **
 ** Supermodel is distributed in the hope that it will be useful, but WITHOUT
 ** ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 ** FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for
 ** more details.
 **
 ** You should have received a copy of the GNU General Public License along
 ** with Supermodel.  If not, see <http://www.gnu.org/licenses/>.
 **/
 
/*
 * Thread.h
 * 
 * Header file for threading primitives.
 */
 
#ifndef INCLUDED_THREADS_H
#define INCLUDED_THREADS_H

class CSemaphore;
class CMutex;
class CCondVar;

typedef int (*ThreadStart)(void *startParam);

/*
 * CThread
 * 
 * Class that represents an O/S thread.
 */
class CThread
{
private:
	void *m_impl;

	CThread(void *impl);

public:
	/*
     * CreateThread
	 *
	 * Creates a new thread with the given ThreadStart callback and start parameter.  The thread starts running immediately.
	 */
	static CThread *CreateThread(ThreadStart start, void *startParam);

	/* 
	 * CreateSemaphore
	 * 
	 * Creates a new semaphore with the given initial starting value.
	 */
	static CSemaphore *CreateSemaphore(UINT32 initVal);

	/* 
	 * CreateCondVar
	 *
	 * Creates a new condition variable.
	 */
	static CCondVar *CreateCondVar();

	/*
	 * CreateMutex
	 *
	 * Creates a new mutex.
	 */
	static CMutex *CreateMutex();
	
	/*
	 * GetLastError
	 *
	 * Returns the error message for the last error.
	 */
	static const char *GetLastError();

	/*
	 * Thread destructor.
	 */
	~CThread();

	/*
	 * GetId
	 *
	 * Returns the id of this thread.
	 */
	UINT32 GetId();

	/*
	 * Kill
	 *
	 * Kills this thread.
	 */
	void Kill();

	/*
	 * Wait
	 *
	 * Waits until this thread has exited.
	 */
	int Wait();
};

/*
 * CSemaphore
 *
 * Class that represents a semaphore.
 */
class CSemaphore
{
friend class CThread;

private:
	void *m_impl;
	
	CSemaphore(void *impl);

public:
	~CSemaphore();

	/*
	 * GetValue
	 *
	 * Returns the current value of this semaphore.
	 */
	UINT32 GetValue();

	/*
	 * Wait
	 *
	 * Locks this semaphore and suspends the calling thread if its value is zero.
	 */
	bool Wait();

	/*
	 * Post
	 *
	 * Unlocks this semaphore and resumes any threads that were blocked on it.
	 */
	bool Post();
};

/*
 * CCondVar
 *
 * Class that represents a condition variable.
 */
class CCondVar
{
friend class CThread;

private:
	void *m_impl;

	CCondVar(void *impl);

public:
	~CCondVar();

	/*
	 * Wait
	 *
	 * Waits on this condition variable and unlocks the provided mutex (which must be locked before calling this method).
	 */
	bool Wait(CMutex *mutex);

	/*
	 * Signal
	 *
	 * Restarts a single thread that is waiting on this condition variable.
	 */
	bool Signal();

	/*
	 * SignalAll
	 *
	 * Restarts all threads that are waiting on this condition variable.
	 */
	bool SignalAll();
};

/*
 * CSemaphore
 *
 * Class that represents a mutex.
 */
class CMutex
{
friend class CThread;
friend class CCondVar;

private:
	void *m_impl;

	CMutex(void *impl);

public:
	~CMutex();

	/*
	 * Lock
	 *
	 * Locks this mutex.  If it is already locked then the calling thread will suspend until it is unlocked.
	 */
	bool Lock();

	/*
	 * Unlock
	 *
     * Unlocks this mutex.  
	 */
	bool Unlock();
};

#endif	// INCLUDED_THREADS_H
