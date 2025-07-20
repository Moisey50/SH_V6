#pragma once

#include "PhIKDriver.h"
#include <set>
#include <iostream>

class Locker
{
	CRITICAL_SECTION m_ResourceLocker;

	Locker(const Locker&);
	Locker& operator=(const Locker&);
public:
	Locker()
		: m_ResourceLocker()
	{
		InitializeCriticalSection(&m_ResourceLocker);		
	}

	void Lock()
	{
		EnterCriticalSection(&m_ResourceLocker);
	}

	void UnLock()
	{
		LeaveCriticalSection(&m_ResourceLocker);
	}

	~Locker()
	{
		DeleteCriticalSection(&m_ResourceLocker);
	}
};

struct IResetable
{
	virtual void OnReset() = 0;
};

template<class T = IControlable, class ARG_T=T*>
class PhidgetsBusBase //Threads Safe controllers bus
{
#pragma region | Fields |
private:
	IResetable             &m_Listener;
	vector<ARG_T>           m_DevicesRefs;
	map<int, ARG_T>         m_BusBySerialNum;

	bool                    m_bStopRequested;      // used to control thread lifetime
	HANDLE                  m_hEvent;
	Locker                  m_Locker;
#pragma endregion | Fields |

#pragma region | Constructors |
private:
	PhidgetsBusBase(const PhidgetsBusBase &);
	const PhidgetsBusBase& operator= (const PhidgetsBusBase &);
public:
	PhidgetsBusBase(IResetable &busResetedListener)
		: m_Listener(busResetedListener)
		, m_DevicesRefs()
		, m_BusBySerialNum()
		, m_bStopRequested(false)
		, m_hEvent(NULL)
		, m_Locker()
	{
		// Create the auto-reset event.
		m_hEvent = CreateEvent( NULL,     // no security attributes
								FALSE,    // auto-reset event
								FALSE,    // initial state is non-signaled
								"PhidgetsBusResetEvent" );    // lpName

		if (m_hEvent == NULL) 
		{
			std::cout << "CreateEvent() failed in MessageBuffer ctor.\n";
		}
	}
	virtual ~PhidgetsBusBase()
	{
		m_Locker.Lock();
		DestroyBus();
		m_Locker.UnLock();
		m_bStopRequested = true;
		CloseHandle( m_hEvent );
	}
#pragma endregion | Constructors |

#pragma region | Methods Private |
private:
	static int ExtractSerialNumber( CPhidgetHandle handle )
	{
		int serNum = 0;
		CPhidget_getSerialNumber (handle, &serNum);
		return serNum;
	}

	void RaiseReseted()
	{
		while (!m_bStopRequested)
		{
			DWORD   dwWaitResult = WaitForSingleObject( m_hEvent, 2000 );

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
	void ResetBus(CPhidgetManagerHandle hManager)
	{
		CPhidgetHandle *phDevices;
		int             numDevices;
		
		//m_DevicesRefs.clear();
		vector<map<int, ARG_T>::const_iterator> toRemove;
		map<int, CPhidgetHandle> toAdd;

		CPhidgetManager_getAttachedDevices (hManager, &phDevices, &numDevices);

		for(int i = 0; i < numDevices; i++)
			toAdd[ExtractSerialNumber(phDevices[i])] = phDevices[i];

		map<int, ARG_T>::const_iterator ci = m_BusBySerialNum.begin();
		if(toAdd.empty())
			DestroyBus();
		else
		{
			
			for(; ci!=m_BusBySerialNum.end(); ci++)
			{
				ARG_T pDevice = ci->second;
				if(!pDevice)
					toRemove.push_back(ci);
				else
				{
					int currentSN = pDevice->GetInfo().GetSerialNum();
					
					map<int, CPhidgetHandle>::const_iterator cii = toAdd.find(currentSN);
					
					if(cii!=toAdd.end())
						toAdd.erase(cii);
					else
						toRemove.push_back(ci);
				}
			}

			if(!toRemove.empty())
				RemoveDevices(toRemove);

			if(!toAdd.empty())
				AddDevices(toAdd);
		}

		m_DevicesRefs.clear();

		ci = m_BusBySerialNum.begin();
		for(; ci!=m_BusBySerialNum.end(); ci++)
		{
			if(ci->second)
				m_DevicesRefs.push_back(ci->second);
		}
		CPhidgetManager_freeAttachedDevicesArray(phDevices);
	}
	
	//Not Thread safe operation
	void AddDevices( const map<int, CPhidgetHandle>& toAdd )
	{
		map<int, CPhidgetHandle>::const_iterator ci = toAdd.begin();

		for( ; ci!= toAdd.end(); ci++)
		{
			if(ci->second)
			{
				m_BusBySerialNum[ci->first] = new T();
				IFKInfo::Init(ci->second, (IFKInfo*)&m_BusBySerialNum[ci->first]->GetInfo());
			}
		}
	}

	//Not Thread safe operation
	void RemoveDevices(const vector<typename map<int, ARG_T>::const_iterator>& toRemove)
	{
		vector<map<int, ARG_T>::const_iterator>::const_iterator ci = toRemove.begin();
		for( ; ci!= toRemove.end(); ci++)
		{
			if((*ci)->second)
			{
				(*ci)->second->Destroy();
			    delete ((*ci)->second);
			}
			m_BusBySerialNum.erase(*ci);
		}
	}
	
	//Not Thread safe operation
	void DestroyBus()
	{
		map<int, ARG_T>::iterator ii = m_BusBySerialNum.begin();
		for(; ii!=m_BusBySerialNum.end(); ii++)
		{
			if(ii->second)
			{
				ii->second->Destroy();
				delete ii->second;
			}
		}
		m_BusBySerialNum.clear();
		m_DevicesRefs.clear();
	}
#pragma endregion | Methods Private |
#pragma region | Methods Protected |
protected:
#pragma endregion | Methods Protected |
#pragma region | Methods Public |
public:
	static unsigned __stdcall StaticEntryPoint_ResetBus(LPVOID arg)
	{
		PhidgetsBusBase *pBus = reinterpret_cast<PhidgetsBusBase*>(arg);

		if(pBus)
			pBus->RaiseReseted();

		return 1;
	}
	
	void GetDevices(vector<ARG_T>& devsRefs)
	{
		m_Locker.Lock();
		devsRefs = m_DevicesRefs;
		m_Locker.UnLock();
	}

	const ARG_T GetDeviceBySerialNum(int deviceSN)
	{
		ARG_T pDevice = NULL;
		m_Locker.Lock();
		map<int, ARG_T>::const_iterator ci = m_BusBySerialNum.find(deviceSN);
		if(ci!= m_BusBySerialNum.end())
			pDevice = ci->second;
		else
		{
			ostringstream oss;
			oss << "Wrong Device SN '" << deviceSN << "'.";
			//RaisePhidgetErrorEvent(-1, 0, oss.str());
		}
		m_Locker.UnLock();
		return pDevice;
	}

	void Init(CPhidgetManagerHandle hManager)
	{
		m_Locker.Lock();
		ResetBus(hManager);
	  	m_Locker.UnLock();

		if ( ! SetEvent( m_hEvent ) )
		{
			std::cout << "SetEvent() failed in  Init().\n";
		}
	}
	void StopBus()
	{
		m_Locker.Lock();
		map<int, ARG_T>::iterator ii = m_BusBySerialNum.begin();
		for(; ii!=m_BusBySerialNum.end(); ii++)
		{
			if(ii->second)
				ii->second->Stop();
		}
		m_bStopRequested = true;
		m_Locker.UnLock();
	}

	virtual string ToString() = 0;
#pragma endregion | Methods Public |
};

class IFKitsBus
	: public PhidgetsBusBase<IFKDriver>
{
#pragma region | Constructors |
private:
	IFKitsBus(const IFKitsBus &);
	const IFKitsBus& operator= (const IFKitsBus &);
public:
	IFKitsBus(IResetable &busResetedListener)
		: PhidgetsBusBase(busResetedListener)
	{ }
#pragma endregion | Constructors |

#pragma region | Methods Public |
public:
	virtual string ToString()
	{
		string asTxt;
		vector<IFKDriver*> devRefs;
		GetDevices(devRefs);
		vector<IFKDriver*>::const_iterator ci = devRefs.begin();
		for ( ; ci!=devRefs.end(); ci++)
		{
			if(!asTxt.empty())
				asTxt.append(";");
			if((*ci)->IsInUse())
				asTxt.append("[x] ");
			asTxt.append((*ci)->ToString());
		}
		return asTxt;
	}
#pragma endregion | Methods Public |
};

typedef struct IPhidgetManagerListener : public IPhidgetBaseListener
{
	virtual void OnPhidgetBusChanged( /*const vector<IFKDriver*> & devices*/ ) =0;
}*LPIPhidgetManagerListener;

class PhidgetManagerEventHandler
	: public PhidgetEventHandler<IPhidgetManagerListener>
{
protected:
	virtual void RaisePhidgetErrorEvent(int deviceSN, int errCode, const string & errMsg )
	{
		EnterCriticalSection(GetCriticalSection());
		for(PhidgetBaseListenersMap::const_iterator ci = GetListeners().begin(); ci != GetListeners().end(); ci++)
		{
			if(ci->second)
				ci->second->OnPhidgetError(deviceSN, errCode, errMsg);
		}
		LeaveCriticalSection(GetCriticalSection());
	}
	virtual void RaisePhidgetBusChangedEvent( /*const vector<IFKDriver*> & devices*/ )
	{
		EnterCriticalSection(GetCriticalSection());
		for(PhidgetBaseListenersMap::const_iterator ci = GetListeners().begin(); ci != GetListeners().end(); ci++)
		{
			if(ci->second)
				ci->second->OnPhidgetBusChanged(/*devices*/);
		}
		LeaveCriticalSection(GetCriticalSection());
	}
};

typedef class CPhidgetsDriver
	: public PhidgetManagerEventHandler
	, public IPhIFKDriverListener
	, public IControlable
	, public IResetable
{
#pragma region | Fields |
	//static CPhidgetsDriver  m_pInstance;
	CPhidgetManagerHandle   m_hManager;
	IFKitsBus               m_Bus;
	//vector<IFKDriver*>      m_DevicesRefs;
	HANDLE                  m_hBusResetHandler;	
#pragma endregion | Fields |

#pragma region | Constructors |
private:

	CPhidgetsDriver(const CPhidgetsDriver&);
	const CPhidgetsDriver& operator=(const CPhidgetsDriver&);
protected:
public:
	CPhidgetsDriver(void)
		: m_hManager(0)
		, m_Bus(*this)
		, m_hBusResetHandler(NULL)
	{
		unsigned uiConsumerThreadID = 0;
		DWORD   dwExitCode;

		m_hBusResetHandler = (HANDLE)_beginthreadex
			( NULL
			, 0
			, PhidgetsBusBase<IFKDriver>::StaticEntryPoint_ResetBus
			, &m_Bus
			, CREATE_SUSPENDED // so we can later call ResumeThread()
			, &uiConsumerThreadID );

		if ( m_hBusResetHandler == 0 )
			std::cout << "Failed to create consumer thread\n";

		GetExitCodeThread( m_hBusResetHandler, &dwExitCode );  // should be STILL_ACTIVE = 0x00000103 = 259

		std::cout << "initial Consumer thread exit code = " << dwExitCode << std::endl;

		ResumeThread( m_hBusResetHandler );
		SetIsRunning(false);
		CPhidget_enableLogging(PHIDGET_LOG_VERBOSE, NULL);
	}
	virtual ~CPhidgetsDriver(void)
	{		
		Destroy();
		WaitForSingleObject(m_hBusResetHandler, INFINITE);
		CloseHandle(m_hBusResetHandler);
	}
#pragma endregion | Constructors |

#pragma region | Method Private |
private:
	static int CCONV AttachHandler(CPhidgetHandle hIFK, void *userptr)
	{
		CPhidgetsDriver *pDrvr = (CPhidgetsDriver *)userptr;
		ASSERT(pDrvr);

		pDrvr->OnAttachEvent();

		return 0;
	}
	static int CCONV DetachHandler(CPhidgetHandle hIFK, void *userptr)
	{
		CPhidgetsDriver *pDrvr = (CPhidgetsDriver *)userptr;
		ASSERT(pDrvr);

		pDrvr->OnDettachEvent();

		return 0;
	}
	static int CCONV ErrorHandler(CPhidgetManagerHandle hManager, void *userptr, int ErrorCode, const char *pErrDescr)
	{
		CPhidgetsDriver *pDrvr = (CPhidgetsDriver *)userptr;
		ASSERT(pDrvr);

		pDrvr->OnErrorEvent(/*hManager,*/ ErrorCode, pErrDescr);

		return 0;
	}

	void OnAttachEvent()
	{
		m_Bus.Init(m_hManager);
	}
	void OnDettachEvent()
	{
		m_Bus.Init(m_hManager);
	}
	void OnErrorEvent( int iErrorCode, const char * pErrDescr )
	{
		RaisePhidgetErrorEvent(-1, iErrorCode, pErrDescr);
	}

	void OnDigitalsChangedEvent(DIGITAL_TYPE dgtlType, int iState, int iIndex );

	//const vector<IFKDriver*>& GetDevices()
	//{
	//	m_Bus.GetDevices(m_DevicesRefs);
	//	return m_DevicesRefs;
	//}

	void StopBus()
	{
		m_Bus.StopBus();
	}

	int GetDigitalState(int deviceSN, DIGITAL_TYPE digitalType, BYTE bitId)
	{
		int state = -1;
		IFKDriver* pDevice = GetDeviceBySerialNum(deviceSN);

		if(pDevice!=NULL)
			state = digitalType & DIGITAL_TYPE_OUT ? pDevice->GetDigitalStateOutput(bitId) : pDevice->GetDigitalStateInput(bitId);

		return state;
	}
	UINT GetStatesMask(int deviceSN, DIGITAL_TYPE digitalType)
	{
		int state = -1;
		IFKDriver* pDevice = GetDeviceBySerialNum(deviceSN);

		if(pDevice!=NULL)
			state = digitalType & DIGITAL_TYPE_OUT ? pDevice->GetAllStatesOutputs() : pDevice->GetAllStatesInputs();

		return state;
	}
	//void OnDigitalOutputChangeRequested( int iState, int iIndex );
#pragma endregion | Method Private |

#pragma region | Metods Protected |
protected:
#pragma endregion | Method Private |

#pragma region | Metods Public |
public:
	//static CPhidgetsDriver& GetInstance()
	//{
	//	//if(!m_pInstance)
	//	//	m_pInstance = new CPhidgetsDriver();
	//	return m_pInstance;
	//}
	
	bool IsInUse() const
	{
		return GetListeners().size() > 0;
	}
	 
	virtual void /*IResetable::*/OnReset()
	{
		RaisePhidgetBusChangedEvent(/*GetDevices()*/);
	}
	virtual string ToString()
	{
		return m_Bus.ToString();
	}

	IFKDriver* GetDeviceBySerialNum(int deviceSN)
	{		
		return m_Bus.GetDeviceBySerialNum(deviceSN);
	}
	
	int  GetOutputState(int deviceSN, BYTE bitId )
	{
		return GetDigitalState(deviceSN, DIGITAL_TYPE_OUT, bitId);
	}
	int  GetInputState(int deviceSN, BYTE bitId )
	{
		return GetDigitalState(deviceSN, DIGITAL_TYPE_IN, bitId);
	}
	UINT GetAllInputsStates(int deviceSN)
	{
		return GetStatesMask(deviceSN, DIGITAL_TYPE_IN);
	}
	UINT GetAllOutputsStates(int deviceSN)
	{
		return GetStatesMask(deviceSN, DIGITAL_TYPE_OUT);
	}

	//void SetOutput(int deviceSN, BYTE bitId, BYTE bitVal )
	//{
	//	CPhidgetInterfaceKit_setOutputState(m_hIntrfaceKit, bitId, bitVal);
	//	OnDigitalOutputChangeRequested(bitVal, bitId);
	//}

	void ToggelOutput( int iOutputIndx );

	
	virtual void /*IPhIFKListener::*/OnPhidgetError(int deviceSN, int errCode, const string& errMsg ) 
	{
		RaisePhidgetErrorEvent(deviceSN, errCode, errMsg);
	}
	virtual void /*IPhIFKListener::*/OnPhIFKAttachmentChanged(int deviceSN, bool isAttached )
	{

	}
	virtual void /*IPhIFKListener::*/OnPhIFKChanged_Digital(int deviceSN, PHIDGET_EVENT eventId, DWORD dwStateMask, DWORD dwFeet )
	{

	}
	virtual void /*IPhIFKListener::*/OnPhIFKChanged_Analog(int deviceSN, PHIDGET_EVENT eventId, double dValue, DWORD dwFeet )
	{

	}

	virtual bool /*IControlable::*/Init(LPIPhidgetBaseListener pListener = NULL )
	{
		LPIPhidgetManagerListener pListenerManager = NULL;

		if(pListener)
			pListenerManager = (LPIPhidgetManagerListener)pListener;
		AttachPhidgetLisener(pListenerManager);
		
		//create the Manager object
		if(!m_hManager)
			CPhidgetManager_create(&m_hManager);

		//Set the handlers to be run when the device is plugged in or opened from software,
		// unplugged or closed from software, or generates an error.
		CPhidgetManager_set_OnAttach_Handler(m_hManager, AttachHandler, this);
		CPhidgetManager_set_OnDetach_Handler(m_hManager, DetachHandler, this);
		CPhidgetManager_set_OnError_Handler(m_hManager, ErrorHandler, this);

		return true;
	}

	virtual bool /*IControlable::*/Start()
	{
		if(m_hManager && !IsRunning())
		{
			//open the Manager for device connections
			CPhidgetManager_open(m_hManager);
			SetIsRunning(true);
		}
		return true;
	}

	virtual void /*IControlable::*/Stop()
	{
		if(m_hManager && IsRunning())
		{
			CPhidgetManager_close(m_hManager);
		}		
	}

	virtual bool /*IControlable::*/Destroy()
	{
		StopBus();

		Stop();

		if(m_hManager && IsRunning())
		{
			CPhidgetManager_delete(m_hManager),	m_hManager=NULL;
			SetIsRunning(false);
		}
		return true;
	}

#pragma endregion | Metods Public |
}*LPCPhidgetsDriver;

extern LPCPhidgetsDriver g_pPhidgetDrvr;