#include "stdafx.h"
#include "AutoCriticalSection.h"

using namespace DeviceDetectLibrary;

AutoCriticalSection::AutoCriticalSection(CriticalSection& section)
	: m_criticalSection(section)
{
   m_criticalSection.Enter();
}

AutoCriticalSection::~AutoCriticalSection() 
{
   m_criticalSection.Leave();
}

