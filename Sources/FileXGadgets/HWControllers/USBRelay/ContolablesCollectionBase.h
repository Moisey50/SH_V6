#pragma once

#include <vector>
#include <map>
#include <iostream>

#include "IControlable.h"
#include "Locker.h"
#include "IResetListener.h"


using namespace std;

template<class T = IControlable, class ARG_T=T*>
class ContolablesCollectionBase //Threads Safe controllers bus
{
#pragma region | Fields |
private:
	IResetListener         &m_Listener;
	vector<ARG_T>           m_RefsCollection;

	bool                    m_bStopRequested;      // used to control thread lifetime
	HANDLE                  m_hEvent;

protected:
	Locker                  m_Locker;
#pragma endregion | Fields |

#pragma region | Constructors |
private:
	ContolablesCollectionBase(const ContolablesCollectionBase &);
	const ContolablesCollectionBase& operator= (const ContolablesCollectionBase &);
public:
	ContolablesCollectionBase(IResetListener &busResetedListener)
		: m_Listener(busResetedListener)
		, m_RefsCollection()
		, m_bStopRequested(false)
		, m_hEvent(NULL)
		, m_Locker()
	{
		// Create the auto-reset event.
		m_hEvent = CreateEvent( NULL,     // no security attributes
			FALSE,    // auto-reset event
			FALSE,    // initial state is non-signaled
			"RelaysBusResetEvent" );    // lpName

		if (m_hEvent == NULL) 
		{
			std::cout << "CreateEvent() failed in MessageBuffer ctor.\n";
		}
	}

	virtual ~ContolablesCollectionBase()
	{
		m_Locker.Lock();
		DestroyCollection();
		m_Locker.UnLock();
		m_bStopRequested = true;
		CloseHandle( m_hEvent );
	}
#pragma endregion | Constructors |

#pragma region | Methods Private |
private:
#pragma endregion | Methods Private |

#pragma region | Methods Protected |
protected:
	void RaiseResetedCallback()
	{
		while (!m_bStopRequested)
		{
			DWORD dwWaitResult = WaitForSingleObject( m_hEvent, 2000 );

			switch(dwWaitResult)
			{
			case WAIT_TIMEOUT :   // WAIT_TIMEOUT = 258
				if(m_bStopRequested)
					return;    // we were told to die
				break;
			case WAIT_ABANDONED : // WAIT_ABANDONED = 80
				std::cout << "WaitForSingleObject(m_hEvent) failed in RaiseReseted().\n";
				return;
			case WAIT_OBJECT_0 :  // WAIT_OBJECT_0 = 0
				std::cout << "ProcessMessages() saw 'Event'.\n";
				m_Listener.OnReset();
				break;
			}			
		}
	}

	//Not Thread safe operation
	virtual void ResetAll() = 0;

	//Not Thread safe operation
	//virtual void Add( const map<string, /*CPhidgetHandle*/void*>& toAdd ) = 0;
	
	//Not Thread safe operation
	void Add( ARG_T pElement )
	{
		if(pElement)
			m_RefsCollection.push_back(pElement);
	}

	//Not Thread safe operation
	void Remove(ARG_T toRemove)
	{
		vector<ARG_T>::const_iterator ci = m_RefsCollection.begin();
		for ( ; ci != m_RefsCollection.end() && *ci != toRemove; ci++)
		{/*no thing*/}
		if(ci != m_RefsCollection.end())
			m_RefsCollection.erase(ci);
	}
	
	//Not Thread safe operation
	void RemoveAll()
	{
		m_RefsCollection.clear();
	}

	//Not Thread safe operation
	virtual void DestroyCollection()
	{
		vector<ARG_T>::iterator ii = m_RefsCollection.begin();
		for(; ii != m_RefsCollection.end(); ii++)
		{
			if(*ii)
			{
				(*ii)->Destroy();
				delete *ii;
			}
		}
		m_RefsCollection.clear();
	}
#pragma endregion | Methods Protected |
#pragma region | Methods Public |
public:
	static unsigned __stdcall StaticEntryPoint_ResetBus(LPVOID arg)
	{
		ContolablesCollectionBase *pBus = reinterpret_cast<ContolablesCollectionBase*>(arg);

		if(pBus)
			pBus->RaiseResetedCallback();

		return 1;
	}

	void GetCollection(vector<ARG_T>& refsCollection)
	{
		m_Locker.Lock();
		refsCollection = m_RefsCollection;
		m_Locker.UnLock();
	}

	void Init()
	{
		m_Locker.Lock();
		ResetAll();
		m_Locker.UnLock();

		if ( ! SetEvent( m_hEvent ) )
		{
			std::cout << "SetEvent() failed in Init().\n";
		}
	}
	
	void StopAll()
	{
		m_Locker.Lock();
		vector<ARG_T>::iterator ii = m_RefsCollection.begin();
		for(; ii != m_RefsCollection.end(); ii++)
		{
			if(*ii)
				(*ii)->Stop();
		}
		m_bStopRequested = true;
		m_Locker.UnLock();
	}

	virtual string ToString() = 0;

#pragma endregion | Methods Public |
};
