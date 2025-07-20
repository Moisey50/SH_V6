#pragma once

#include <windows.h>

#include "RelayCallbackHandlerBase.h"
#include "IRelaysManagerListener.h"

class RelaysManagerCallbackHandler
	: public RelayCallbackHandlerBase<IRelaysManagerListener>
{
protected:
	virtual void RaiseRelaysErrorCallback(const string& deviceSN, int errCode, const string & errMsg )
	{
		EnterCriticalSection(GetCriticalSection());
		for(RelayBaseListenersMap::const_iterator ci = GetListeners().begin(); ci != GetListeners().end(); ci++)
		{
			if(ci->second)
				ci->second->OnRelayError(deviceSN, errCode, errMsg);
		}
		LeaveCriticalSection(GetCriticalSection());
	}

	virtual void RaiseRelaysCollectionChangedCallback()
	{
		EnterCriticalSection(GetCriticalSection());
		for(RelayBaseListenersMap::const_iterator ci = GetListeners().begin(); ci != GetListeners().end(); ci++)
		{
			if(ci->second)
				ci->second->OnRelaysCollectionChanged();
		}
		LeaveCriticalSection(GetCriticalSection());
	}
};