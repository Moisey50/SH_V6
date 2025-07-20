#pragma once

#include <set>
#include <iostream>
#include <process.h>

#include "IDeviceWatcherObserver.h"

#include "DeviceWatcher.h"

#include "usb_relay.h"
#include "RelaysManagerCallbackHandler.h"
#include "IResetListener.h"
#include "UsbRelayDriversCollection.h"

//#include "C_UsbEnumeratorHID.h"

using namespace DeviceDetectLibrary;
//using namespace DeviceDetectLibrary::Connection;

class UsbRelaysManager
	: public RelaysManagerCallbackHandler
	, public IResetListener
	, public IControlable
	, public IRelayDriverListener
	, public IDeviceWatcherObserver
{
#pragma region | Fields |
	DeviceWatcher_Ptr          m_pWatcher;

	UsbRelayDriversCollection  m_Bus;
	HANDLE                     m_hBusResetHandler;
	bool                       m_isRunning;
#pragma endregion | Fields |

#pragma region | Constructors |
private:

	UsbRelaysManager(const UsbRelaysManager&);
	const UsbRelaysManager& operator=(const UsbRelaysManager&);
protected:
public:
	UsbRelaysManager(void)
		: m_Bus(*this) 
		, m_hBusResetHandler(NULL)
		, m_isRunning(false)
	{
		m_pWatcher.reset(new DeviceWatcher(this));
		m_pWatcher->Start();

//		unsigned uiConsumerThreadID = 0;
//		DWORD   dwExitCode;
		if(usb_relay_exit()==0 && usb_relay_init()==0)
		{
			//m_hBusResetHandler = (HANDLE)_beginthreadex
			//	( NULL
			//	, 0
			//	, UsbRelayDriversCollection::StaticEntryPoint_ResetBus
			//	, &m_Bus
			//	, CREATE_SUSPENDED // so we can later call ResumeThread()
			//	, &uiConsumerThreadID );

			//if ( m_hBusResetHandler == 0 )
			//	std::cout << "Failed to create consumer thread\n";

			//GetExitCodeThread( m_hBusResetHandler, &dwExitCode );  // should be STILL_ACTIVE = 0x00000103 = 259

			//std::cout << "initial Consumer thread exit code = " ;//<< dwExitCode << std::endl;

//			ResumeThread( m_hBusResetHandler );
		}
	}
	virtual ~UsbRelaysManager(void)
	{
		m_pWatcher->Stop();
		Destroy();
		WaitForSingleObject(m_hBusResetHandler, INFINITE);
		CloseHandle(m_hBusResetHandler);
	}
#pragma endregion | Constructors |

#pragma region | Method Private |
private:

	void OnDigitalsChangedEvent(unsigned uState, BYTE channelId);

	//const vector<IFKDriver*>& GetDevices()
	//{
	//	m_Bus.GetDevices(m_DevicesRefs);
	//	return m_DevicesRefs;
	//}

	void StopBus()
	{
		m_Bus.StopAll();
		m_Bus.DestroyCollection();
	}

	//void OnDigitalOutputChangeRequested( int iState, int iIndex );
#pragma endregion | Method Private |

#pragma region | Metods Protected |
protected:
	void SetIsRunning(bool isRunning)
	{
		m_isRunning = isRunning;
	}
#pragma endregion | Method Private |

#pragma region | Metods Public |
public:
	unsigned GetDigitalState(const string& deviceSN, BYTE channelId)
	{
		int state = -1;
		UsbRelayDriver* pDevice = GetDeviceBySerialNum(deviceSN);

		if(pDevice!=NULL)
			state = pDevice->GetChannelState(channelId);

		return state;
	}
	unsigned GetStatesMask(const string& deviceSN)
	{
		int state = -1;
		UsbRelayDriver* pDevice = GetDeviceBySerialNum(deviceSN);

		if(pDevice!=NULL)
			state = pDevice->GetStatesMask();

		return state;
	}
	
	bool IsInUse() const
	{
		return GetListeners().size() > 0;
	}
	 
	virtual void /*IResetListener::*/OnReset()
	{
		RelaysManagerCallbackHandler::RaiseRelaysCollectionChangedCallback();
	}
	virtual string ToString()
	{
		return m_Bus.ToString();
	}

	UsbRelayDriver* GetDeviceBySerialNum(const string& deviceSN)
	{		
		return (UsbRelayDriver*)m_Bus.GetDeviceBySerialNum(deviceSN);
	}
	
	void SetOutput(const string& deviceSN, BYTE bitId, BYTE bitVal )
	{
		//CPhidgetInterfaceKit_setOutputState(m_hIntrfaceKit, bitId, bitVal);
		//OnDigitalOutputChangeRequested(bitVal, bitId);
	}

	void ToggelOutput( int iOutputIndx );

	virtual bool /*IControlable::*/IsRunning()
	{
		return m_isRunning;
	}

	virtual bool /*IControlable::*/Init(LPIRelayListenerBase pListener = NULL )
	{
		LPIRelaysManagerListener pListenerManager = NULL;

		if(pListener)
			pListenerManager = (LPIRelaysManagerListener)pListener;
		AttachRelayLisener(pListenerManager);

		return true;
	}

	virtual bool /*IControlable::*/Start()
	{		
		//if(/*m_hManager &&*/ !IsRunning())
		//{
		//	//open the Manager for device connections
		//	CPhidgetManager_open(m_hManager);
			m_Bus.Init();
//			SetIsRunning(true);
		//}
		return true;
	}

	virtual void /*IControlable::*/Stop()
	{
		if(IsRunning())
		{
			bool isClosed = usb_relay_exit() == 0;
			if(!isClosed)
			{
				RaiseRelaysErrorCallback(PROP_NAME_RELAY_DVC, -1, "usb_relay_exit() is failed on Stop() of UsbRelaysManager");
			}
		}
		SetIsRunning(false);
	}

	virtual bool /*IControlable::*/Destroy()
	{
		StopBus();

		Stop();

		return true;
	}


	virtual void /*IRelayDriverListener::*/OnRelayError(const string& deviceSN, int errCode, const string& errMsg ) 
	{
		RaiseRelaysErrorCallback(deviceSN, errCode, errMsg);
	}
	virtual void /*IRelayDriverListener::*/OnRelayAttachmentChanged(const string& deviceSN, bool isAttached )
	{

	}
	virtual void /*IRelayDriverListener::*/OnRelayChanged(const string& deviceSN, unsigned uStateMask, BYTE channelID )
	{

	}

	// IDeviceWatcherObserver interface
	void /*DeviceDetectLibrary::*/AppearedDevice(const DeviceInfo& deviceInfo)
	{
		m_Bus.Init();
	}
	void /*DeviceDetectLibrary::*/DisappearedDevice(const DeviceInfo& deviceInfo)
	{
		m_Bus.RemoveByPath(string(deviceInfo.GetConnectionInfo().DevicePath.begin(), deviceInfo.GetConnectionInfo().DevicePath.end()));
	}

#pragma endregion | Metods Public |
};

extern UsbRelaysManager *g_pUsbRelaysDrvr;