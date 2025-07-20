#pragma once

#include <windows.h>
#include "IRelayListenerBase.h"
#include "Utils.h"


typedef struct IRelayDriverListener : public IRelayListenerBase
{
	virtual void OnRelayAttachmentChanged(const string& deviceSN, bool isAttached ) =0;
	virtual void OnRelayChanged(const string& deviceSN, unsigned uStateMask, BYTE channelID ) =0;
}*LPIRelayDriverListener;