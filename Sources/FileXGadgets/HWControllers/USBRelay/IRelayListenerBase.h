#pragma once

#include <string>

using namespace std;

typedef struct IRelayListenerBase
{
	virtual void OnRelayError(const string& deviceSN, int errCode, const string& errMsg ) = 0;
}*LPIRelayListenerBase;