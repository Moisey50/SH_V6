#include "stdafx.h"
#include "Win_AutoHandle.h"


AutoHandle::AutoHandle(Handle handle)
	:m_handle(handle)
{
    if (handle == INVALID_HANDLE_VALUE)
    {
        throw AutoHandleException("[AutoHandle]: Handle is invalid");
    }
}

AutoHandle::AutoHandle(Handle handle, Nothrow)
	:m_handle(handle)
{
}

AutoHandle::~AutoHandle()
{
    Reset();
}

AutoHandle::operator Handle() const 
{
    return m_handle;
}

Handle AutoHandle::GetHandle() const
{
    return m_handle;
}

Handle AutoHandle::Release()
{
    Handle result = m_handle;
    m_handle = INVALID_HANDLE_VALUE;
    return result;
}

void AutoHandle::Reset()
{
    Reset(INVALID_HANDLE_VALUE);
}

void AutoHandle::Reset(Handle newHandle/* = INVALID_HANDLE_VALUE*/)
{
    if (m_handle != INVALID_HANDLE_VALUE)
    {
        ::CloseHandle(m_handle);
    }
    m_handle = newHandle;
}
