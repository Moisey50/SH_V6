#pragma once

#include <map>

#include "IRelayListenerBase.h"

using namespace std;


template <class T = IRelayListenerBase, class ARG_T = T*>
class RelayCallbackHandlerBase
{
protected:
	typedef map<ARG_T, ARG_T> RelayBaseListenersMap;

#pragma region | Fields |
private:
	RelayBaseListenersMap  m_Listeners;
	CRITICAL_SECTION       m_ResourceLock;
#pragma endregion | Fields |

#pragma region | Constructors |
private:
	RelayCallbackHandlerBase(const RelayCallbackHandlerBase&);
	const RelayCallbackHandlerBase& operator =(const RelayCallbackHandlerBase&);
public:
	RelayCallbackHandlerBase()
	{
		InitializeCriticalSection( &m_ResourceLock ) ;
	}

	virtual ~RelayCallbackHandlerBase()
	{
		DeleteCriticalSection( &m_ResourceLock ) ;
	}
#pragma endregion | Constructors |

#pragma region | Method Private |
private:
#pragma endregion | Method Private |

#pragma region | Metods Protected |
protected:
	LPCRITICAL_SECTION GetCriticalSection()
	{
		return &m_ResourceLock;
	}
#pragma endregion | Method Private |

#pragma region | Metods Public |
public:
	const RelayBaseListenersMap& GetListeners() const
	{
		return m_Listeners;
	}
	bool AttachRelayLisener( ARG_T pRelayLisener )
	{
		if(pRelayLisener)
		{
			EnterCriticalSection(GetCriticalSection());
			m_Listeners[pRelayLisener] = pRelayLisener;
			LeaveCriticalSection(GetCriticalSection());
		}

		return m_Listeners[pRelayLisener] ? true : false;
	}

	void DetachRelayLisener( ARG_T pRelayLisener )
	{
		RelayBaseListenersMap::const_iterator ci;
		if(pRelayLisener)
		{
			EnterCriticalSection(GetCriticalSection());
			ci = m_Listeners.find(pRelayLisener);
			if(ci != m_Listeners.end())
				m_Listeners.erase(ci);
			LeaveCriticalSection(GetCriticalSection());
		}
	}
#pragma endregion | Metods Public |
};

