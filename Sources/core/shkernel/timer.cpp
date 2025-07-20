// Timer.cpp: implementation of the CHPTimer class.
// 21 January 2025
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "Timer.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

#define TIMER_OFF		(-1)
#define TIMER_NOT_SET	0
//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CHPTimer::CHPTimer()
{
	if (!QueryPerformanceFrequency(&m_PerfFreq))
		m_PerfFreq.QuadPart = TIMER_NOT_SET;
	m_StartTick.QuadPart = TIMER_NOT_SET;
	m_PauseTick.QuadPart = TIMER_NOT_SET;
}

CHPTimer::~CHPTimer()
{
	Stop();
}

BOOL CHPTimer::Start()
{
	LARGE_INTEGER CurTick;
	if ((m_PerfFreq.QuadPart != TIMER_NOT_SET) && QueryPerformanceCounter(&CurTick))
	{
		m_StartTick.QuadPart += (CurTick.QuadPart - m_PauseTick.QuadPart);
		m_PauseTick.QuadPart = TIMER_NOT_SET;
		return TRUE;
	}
	m_StartTick.QuadPart = TIMER_NOT_SET;
	return FALSE;
}

BOOL CHPTimer::Pause()
{
	if ((m_PerfFreq.QuadPart == TIMER_NOT_SET) || (m_StartTick.QuadPart == TIMER_NOT_SET))
		return FALSE;
	return QueryPerformanceCounter(&m_PauseTick);
}

void CHPTimer::Stop()
{
	m_StartTick.QuadPart = TIMER_NOT_SET;
	m_PauseTick.QuadPart = TIMER_NOT_SET;
}

double	CHPTimer::GetTime() // Gives time in micro seconds
{
	if ((m_PerfFreq.QuadPart == TIMER_NOT_SET) || (m_StartTick.QuadPart == TIMER_NOT_SET))
		return TIMER_OFF;
	LARGE_INTEGER CurTick;
	if (m_PauseTick.QuadPart != TIMER_NOT_SET)
		CurTick.QuadPart = m_PauseTick.QuadPart;
	else if (!QueryPerformanceCounter(&CurTick))
		return TIMER_OFF;
    // Frequency is number of ticks in one second => nominator is multiplied by 1e6 (to microseconds)
	return ((double)(CurTick.QuadPart - m_StartTick.QuadPart) * 1e6 / (double)m_PerfFreq.QuadPart); 
}
