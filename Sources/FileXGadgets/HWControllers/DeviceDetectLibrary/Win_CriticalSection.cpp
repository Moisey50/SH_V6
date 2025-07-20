#include "stdafx.h"
#include "Win_CriticalSection.h"

using namespace DeviceDetectLibrary;

CriticalSection::CriticalSection()
{
   ::InitializeCriticalSection(&m_criticalSection);
}

CriticalSection::~CriticalSection()
{
   ::DeleteCriticalSection(&m_criticalSection);
}

void CriticalSection::Enter()
{
   ::EnterCriticalSection(&m_criticalSection);
}

void CriticalSection::Leave()
{
   ::LeaveCriticalSection(&m_criticalSection);
}


