#pragma once

#include <windows.h>

class Locker
{
	CRITICAL_SECTION m_ResourceLocker;

	Locker(const Locker&);
	Locker& operator=(const Locker&);
public:
	Locker()
		: m_ResourceLocker()
	{
		InitializeCriticalSection(&m_ResourceLocker);		
	}

	void Lock()
	{
		EnterCriticalSection(&m_ResourceLocker);
	}

	void UnLock()
	{
		LeaveCriticalSection(&m_ResourceLocker);
	}

	~Locker()
	{
		DeleteCriticalSection(&m_ResourceLocker);
	}
};

