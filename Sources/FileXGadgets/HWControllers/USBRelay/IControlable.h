#pragma once
#include <windows.h>
#include "IRelayListenerBase.h"


typedef struct IControlable
{
private:
protected:
public:
	virtual bool IsRunning() = 0;
	virtual bool Init(LPIRelayListenerBase pListener = NULL) = 0; 
	virtual bool Start() = 0;
	virtual void Stop() = 0;
	virtual bool Destroy() = 0;
}*LPIControlable;