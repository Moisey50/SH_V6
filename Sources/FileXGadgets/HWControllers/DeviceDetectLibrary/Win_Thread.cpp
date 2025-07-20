#include "StdAfx.h"
#include "Win_Thread.h"

using namespace DeviceDetectLibrary;

Thread::Thread( LPThreadWorker pWorkerRoutine )
    : WaitObject(INVALID_HANDLE_VALUE, AutoHandle::Nothrow())
    , m_pWorkerRoutine(pWorkerRoutine)
    , m_pCurrentState(NULL)
{
}

Thread::~Thread()
{
    Wait(INFINITE);    
}

void Thread::Start( void * pState )
{
    if ((HANDLE)(*this) != INVALID_HANDLE_VALUE)
    {
        throw AutoHandleException("Thread is already started"); 
    }

    m_pCurrentState = pState;
    Reset((HANDLE)_beginthreadex(NULL, 0, Thread::Routine, this, 0, NULL));
    if (((HANDLE)(*this) == INVALID_HANDLE_VALUE) || ((HANDLE)(*this) == 0))
    {
        Reset();
        throw AutoHandleException("Cannot create thread");
    }
}

unsigned int __stdcall Thread::Routine( void * pState )
{
    Thread* pThis = static_cast<Thread*>(pState);
    pThis->m_pWorkerRoutine(pThis->m_pCurrentState);
    return 0;
}

bool Thread::Wait( unsigned int milliseconds )
{
    bool result = WaitObject::Wait(milliseconds);
    if (result)
    {
        Reset();
    }
    return result;
}