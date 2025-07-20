#ifndef _AUTO_HANDLE_H_
#define _AUTO_HANDLE_H_

#include <stdexcept>
#include "Win32Types.h"
#include "Utilities.h"

class AutoHandleException
	: public std::runtime_error
{
public:
    typedef std::runtime_error base;

public:
    AutoHandleException(const std::string& message)
		: base(message)
    {
    }
};

class AutoHandle //non copyable
{
private:
    Handle m_handle;

	DISALLOW_COPY_AND_ASSIGN(AutoHandle);

public:
    struct Nothrow {};

public:
    AutoHandle(Handle handle);
    AutoHandle(Handle handle, Nothrow);
    virtual ~AutoHandle();

public:
    Handle GetHandle() const;
    operator Handle() const;

public:
	Handle Release();
    void Reset();
    void Reset(Handle newHandle);


};

#endif //_AUTO_HANDLE_H_