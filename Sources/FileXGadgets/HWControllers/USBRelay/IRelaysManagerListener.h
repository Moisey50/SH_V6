#pragma once

#include "IRelayListenerBase.h"

typedef struct IRelaysManagerListener : public IRelayListenerBase
{
	virtual void OnRelaysCollectionChanged( /*const vector<IFKDriver*> & devices*/ ) =0;
}*LPIRelaysManagerListener;