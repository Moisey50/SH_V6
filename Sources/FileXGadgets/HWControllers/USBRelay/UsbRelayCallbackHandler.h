#pragma once

#include <string>
#include <windows.h>

#include "RelayCallbackHandlerBase.h"
#include "IRelayDriverListener.h"

using namespace std;

class UsbRelayCallbackHandler
	: public RelayCallbackHandlerBase<IRelayDriverListener>
{
#pragma region | Constructors |
private:
	UsbRelayCallbackHandler(const UsbRelayCallbackHandler&);

public:
	UsbRelayCallbackHandler(){}
	virtual ~UsbRelayCallbackHandler(){}
#pragma endregion | Constructors |

#pragma region | Methods Protected |
protected:
	virtual void RaiseUsbRelayErrorCallback(const string& deviceSN, int errCode, const string& errMsg )
	{
		EnterCriticalSection(GetCriticalSection());
		for(RelayBaseListenersMap::const_iterator ci = GetListeners().begin(); ci != GetListeners().end(); ci++)
		{
			if(ci->second)
				ci->second->OnRelayError(deviceSN, errCode, errMsg);
		}
		LeaveCriticalSection(GetCriticalSection());
	}
	virtual void RaiseUsbRelayAttachmentChangedCallback(const string& deviceSN, bool isAttached )
	{
		EnterCriticalSection(GetCriticalSection());
		for(RelayBaseListenersMap::const_iterator ci = GetListeners().begin(); ci != GetListeners().end(); ci++)
		{
			if(ci->second)
				ci->second->OnRelayAttachmentChanged(deviceSN, isAttached);
		}
		LeaveCriticalSection(GetCriticalSection());
	}
	virtual void RaiseUsbRelayChangedCallback(const string& deviceSN, unsigned uStateMask, BYTE channelId )
	{
		EnterCriticalSection(GetCriticalSection());
		for(RelayBaseListenersMap::const_iterator ci = GetListeners().begin(); ci != GetListeners().end(); ci++)
		{
			if(ci->second)
				ci->second->OnRelayChanged(deviceSN, uStateMask, channelId);
		}
		LeaveCriticalSection(GetCriticalSection());
	}
#pragma endregion | Methods Protected |
};