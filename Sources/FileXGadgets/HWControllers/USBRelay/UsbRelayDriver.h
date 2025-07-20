#pragma once

#include "usb_relay_device.h"
#pragma comment(lib, "usb_relay_device.lib")


#include <string>
#include <windows.h>
//0x051e2bc8 "\\\\?\\hid#vid_16c0&pid_05df#6&2b84c2c8&0&0000#{4d1e55b2-f16f-11cf-88cb-001111000030}"
#include "UsbRelayCallbackHandler.h"
#include "IControlable.h"
#include "UsbRelayModel.h"
#include "usb_relay.h"

using namespace std;


#define WAIT_MESSAGE ("Waiting for usb relay to be attached....")

class UsbRelayDriver
	: public UsbRelayCallbackHandler
	, public IControlable
{
#pragma region | Fields |
	HANDLE        m_hDevHandle;
	UsbRelayInfo  m_devInfo;
#pragma endregion | Fields |

#pragma region | Constructors |
private:
	UsbRelayDriver(const UsbRelayDriver&);
	UsbRelayDriver& operator=(const UsbRelayDriver&);

	UsbRelayDriver
		( HANDLE handle
		, const string& deviceSN
		, const string& devicePath
		, int channels )
		: m_hDevHandle(handle)
		, m_devInfo(deviceSN, devicePath, channels)
	{
	}

public:
	virtual ~UsbRelayDriver()
	{
		Destroy();
	}
#pragma endregion | Constructors |

#pragma region | Metods Private |
private:

	void OnDigitalsChangedEvent(int iState, int iIndex )
	{
		//ostringstream oss;
		//RELAY_EVENT phdgtEvnt = PHIDGET_EVENT_UNKNOWN;
		//oss << "Digital ";

		//int iBitStateMask = iState;
		//if(iBitStateMask)
		//	iBitStateMask <<= iIndex;

		//if(dgtlType & DIGITAL_TYPE_IN)
		//{
		//	oss << "Input:";
		//	phdgtEvnt = PHIDGET_EVENT_CHANGED_INPUT;
		//	//UpdateViewInputs(GetAllInputsStates(), iIndex);
		//}
		//else if(dgtlType & DIGITAL_TYPE_OUT)
		//{
		//	oss << "Output:";
		//	phdgtEvnt = PHIDGET_EVENT_CHANGED_OUTPUT;
		//	//UpdateViewOutputs(GetAllOutputsStates(), iIndex);
		//}
		//oss <<" #" << iIndex << " -> State: " << iState << endl; 

		//if(phdgtEvnt == PHIDGET_EVENT_UNKNOWN)
		//	RaisePhidgetErrorEvent(GetInfo().GetSerialNum(), 0, "Wrong digital change.");
		//else
		//	RaisePhIFKChangedEvent(GetInfo().GetSerialNum(), phdgtEvnt, iBitStateMask, iIndex);

		//TRACE(oss.str().c_str());
	}
#pragma endregion | Metods Private |

#pragma region | Methods Protected |
protected:
	void OnDigitalOutputChangeRequested( int iState, int iIndex );
#pragma endregion | Methods Protected |

#pragma region | Methods Public |
public:
	unsigned GetChannelState( BYTE channelId );
	UINT GetStatesMask();

	static UsbRelayDriver* Create(const struct usb_relay_device_info* pInfo)
	{
		UsbRelayDriver* pRes = NULL;
		if(pInfo)
		{
			HANDLE hDev = (HANDLE)(size_t)usb_relay_device_open((struct usb_relay_device_info *)pInfo);
			pRes = Create(hDev, (char*)pInfo->serial_number, pInfo->device_path, pInfo->type);
		}
		return pRes;
	}

	static UsbRelayDriver* Create
		( HANDLE handle
		, const string& deviceSN
		, const string& devicePath
		, int channels )
	{
		UsbRelayDriver* pRes = NULL;

		if(handle)
		{
			pRes = new UsbRelayDriver(handle, deviceSN, devicePath, channels);
		}

		return pRes;
	}

	int GetHandle() const
	{
		return (int)(size_t)m_hDevHandle;
	}

	bool IsInUse() const
	{
		return GetListeners().size()>0;
	}

	const UsbRelayInfo& GetInfo() const
	{
		return m_devInfo;
	}

	bool operator<(const UsbRelayDriver& other) const
	{
		return GetInfo() < other.GetInfo();
	}

	virtual bool /*IControlable::*/IsRunning()
	{
		return GetHandle() != NULL;
	}

	virtual bool /*IControlable::*/Init(LPIRelayListenerBase pListener = NULL)
	{
		LPIRelayDriverListener pDrvrListener = NULL;
		bool res = false;
		if(pListener)
			pDrvrListener = (LPIRelayDriverListener)pListener;
		res = AttachRelayLisener(pDrvrListener);

		return res;
	}

	virtual bool /*IControlable::*/Destroy()
	{
		Stop();

		//if(GetHandle())
		//{
		//	try
		//	{
		//		CPhidget_delete((CPhidgetHandle)GetHandle());
		//	}
		//	catch (CException* e)
		//	{
		//		char perrmsg[4096]={0};
		//		e->GetErrorMessage(perrmsg,4096);
		//	}
		//}
		//	
		//SetHandle(NULL);
		return GetHandle()==NULL;
	}

	virtual bool /*IControlable::*/Start()
	{
		int isStarted = 0;
		//const char* errMsg;

		//if(!GetHandle())
		//{
		//	CPhidgetHandle hIFK = NULL;
		//	
		//	CPhidgetInterfaceKit_create((CPhidgetInterfaceKitHandle*)&hIFK);
		//	
		//	//Set the handlers to be run when the device is plugged in
		//	// or opened from software, unplugged or closed from software,
		//	// or generates an error.
		//	CPhidget_set_OnAttach_Handler(hIFK, AttachHandler, this);
		//	CPhidget_set_OnDetach_Handler(hIFK, DetachHandler, this);
		//	CPhidget_set_OnError_Handler(hIFK, ErrorHandler, this);
		//	
		//	SetHandle(hIFK);
		//	
		//	//Registers a callback that will run if an input changes.
		//	//Requires the handle for the Phidget, the function that
		//	//will be called, and an arbitrary pointer that will be
		//	//supplied to the callback function (may be NULL).
		//	CPhidgetInterfaceKit_set_OnInputChange_Handler ((CPhidgetInterfaceKitHandle)GetHandle(), DigitalsChangeHandler_Input, this);
		//	
		//	//Registers a callback that will run if an output changes.
		//	//Requires the handle for the Phidget, the function that
		//	//will be called, and an arbitrary pointer that will be
		//	//supplied to the callback function (may be NULL).
		//	CPhidgetInterfaceKit_set_OnOutputChange_Handler ((CPhidgetInterfaceKitHandle)GetHandle(), DigitalsChangeHandler_Output, this);
		//}

		//if(GetHandle() && !IsRunning())
		//{
		//	//		Logger_AddMessage("Opening the Interface-Kit connection.");

		//	//EnterCriticalSection( &m_ResourceLock ) ;

		//	//open the interface-kit for device connections
		//	int op = CPhidget_open((CPhidgetHandle)GetHandle(), GetInfo().GetSerialNum());

		//	SetIsRunning(true);

		//	//get the program to wait for an interface kit device to be attached
		//	if(!(isStarted = CPhidget_waitForAttachment((CPhidgetHandle)GetHandle(), /*iDelay*/10000)))
		//	{
		//		//Logger_AddMessage("The Interface-Kit connection succeed.");
		//	}
		//	else
		//	{
		//		CPhidget_getErrorDescription(isStarted, &errMsg);
		//		//CString csErr = CString(errMsg);
		//		//Logger_AddMessage(csErr);
		//		///PostStatusMessage(csErr);
		//	}

		//}
		return !isStarted;
	}

	virtual void /*IControlable::*/Stop()
	{
		if(/*GetHandle() &&*/ IsRunning())
		{
			usb_relay_device_close(GetHandle());
			m_hDevHandle = NULL;
//			SetIsRunning(false);
		}
	}

	/*
	 * @parameters:
	 *		channelId -- 0 is All channels, 1 - 8 is specific channel index;
	 *		channelState -- 0 is Off, 1 is On;
	 */
	void SetChannel( BYTE channelId, BYTE channelVal )
	{
		int res = -1;

		if(channelId == 0) //channel id = 0, for all channels
		{
			if(channelVal == DIGITAL_COMMAND_VAL_ON)
				res = usb_relay_device_open_all_relay_channel(GetHandle());
			else
				res = usb_relay_device_close_all_relay_channel(GetHandle());
		}
		else if(channelId > 0)
		{
			if(channelVal == DIGITAL_COMMAND_VAL_ON)
				res = usb_relay_device_open_one_relay_channel(GetHandle(), channelId);
			else
				res = usb_relay_device_close_one_relay_channel(GetHandle(), channelId);
		}
		unsigned iChnlsStateMask = 0;
		
		switch (res)
		{
		case 0: //Success
			if((iChnlsStateMask = GetStatesMask()) < ~0)
			{
				int iCh = channelId;
				int endCh = channelId;
				if(channelId == 0)
				{
					iCh++;
					endCh = GetInfo().GetNumChannels();
				}
				for( ; iCh<=endCh; iCh++)
					RaiseUsbRelayChangedCallback(GetInfo().GetSerialNum(), iChnlsStateMask, iCh);
			}
			break;
		case 1: //Error general
		case 2: //Error - channel index is out of range
			break;
		default:
			break;
		}
		
		//if(iBitStateMask)
		//	iBitStateMask <<= bitId;		
	}

	void ToggelChannel( BYTE channelId )
	{
		if(!GetHandle())
			RaiseUsbRelayErrorCallback(GetInfo().GetSerialNum(), 0,"There is no handle yet!");
		else 
		{		
			if(!channelId)
			{
				BYTE cId = channelId;
				BYTE endId = GetInfo().GetNumChannels();
				cId++;
				
				for ( ; cId <= endId; cId++ )
					ToggelChannel(cId);
			}
			else
			{			
				unsigned channelState = GetChannelState(channelId);

				if(!channelState)
					channelState=1;
				else
					channelState=0;

				SetChannel(channelId, channelState);
			}
		}
	}

	string ToString() const
	{
		return GetInfo().ToString();
	}
	
#pragma endregion | Methods Public |
};