#pragma once

#include "phidget21.h"

#include <map>
#include <string>
#include <windows.h>
#include <vector>
#include <sstream>

using namespace std;


#define WAIT_MESSAGE ("Waiting for interface kit to be attached....")

typedef enum PHIDGET_EVENT
{
	PHIDGET_EVENT_UNKNOWN,
	PHIDGET_EVENT_CHANGED_INPUT,
	PHIDGET_EVENT_CHANGED_OUTPUT,
	PHIDGET_EVENT_REQUESTED_CHANGE_OUTPUT
} *LPPHIDGET_EVENT;
typedef enum DIGITAL_TYPE{
	DIGITAL_TYPE_UNKNOWN,
	DIGITAL_TYPE_IN = 0x1,
	DIGITAL_TYPE_OUT = DIGITAL_TYPE_IN << 1
}EDIGITAL_TYPE, *LPEDIGITAL_TYPE;

typedef struct IFKInfo
{
#pragma region | Fields |
private:
	int m_iSerialNum;
	int m_iVersion;
	int m_iStatus;
	int m_iNumInputs;
	int m_iNumOutputs;
	int m_iNumAnalogs;
	bool m_bIsRatiometric;
	string m_sName, m_sLabel, m_sDevType;
	CPhidget_DeviceClass m_dvcClass;
	CPhidget_DeviceID m_dvcId;
#pragma endregion | Fields |

#pragma region | Constructors |
public:
	struct IFKInfo()
		: /*m_hKitHandle(0)
		,*/ m_iSerialNum(0)
		, m_iVersion(0)
		, m_iStatus(0)
		, m_iNumInputs(0)
		, m_iNumOutputs(0)
		, m_iNumAnalogs(0)
		, m_bIsRatiometric(0)
		, m_sName()
		, m_sLabel()
		, m_sDevType()
		, m_dvcClass()
		, m_dvcId()
	{}
#pragma endregion | Constructors |

#pragma region | Methods Private |
private:
	void GetKitProperties(CPhidgetHandle hIFK)
	{
		if(!hIFK)
		{
			m_dvcClass = CPhidget_DeviceClass();
			m_dvcId = CPhidget_DeviceID();
			m_sLabel = m_sName = m_sDevType = string();
			m_iStatus = m_iVersion = m_iSerialNum = m_iNumOutputs = m_iNumInputs = m_iNumAnalogs = 0;
		}
		else
		{
			CPhidget_getDeviceClass  (hIFK, &m_dvcClass);
			CPhidget_getDeviceID     (hIFK, &m_dvcId);
			const char* pTxt=NULL;		
			CPhidget_getDeviceLabel  (hIFK, &pTxt);
			if(pTxt)					
				m_sLabel = pTxt;		
			pTxt = NULL;				
			CPhidget_getDeviceName   (hIFK, &pTxt);
			if(pTxt)					
				m_sName = pTxt;			
			pTxt = NULL;				
			CPhidget_getDeviceType   (hIFK, &pTxt);
			if(pTxt)					
				m_sDevType = pTxt;		
			pTxt = NULL;				
			CPhidget_getDeviceStatus (hIFK, &m_iStatus);
			CPhidget_getDeviceVersion(hIFK, &m_iVersion);
			CPhidget_getSerialNumber (hIFK, &m_iSerialNum);

			CPhidgetInterfaceKit_getInputCount ((CPhidgetInterfaceKitHandle)hIFK, &m_iNumInputs);
			CPhidgetInterfaceKit_getOutputCount((CPhidgetInterfaceKitHandle)hIFK, &m_iNumOutputs);

			CPhidgetInterfaceKit_getSensorCount((CPhidgetInterfaceKitHandle)hIFK, &m_iNumAnalogs);

			int iRatiometric=0;
			CPhidgetInterfaceKit_getRatiometric((CPhidgetInterfaceKitHandle)hIFK, &iRatiometric);
			m_bIsRatiometric = iRatiometric != 0;
		}
	}
#pragma endregion | Methods Private |
	
#pragma region | Methods Public |
public:
	static void Init( CPhidgetHandle hIFK, IFKInfo *pKitInfo )
	{
		if(pKitInfo)
			pKitInfo->GetKitProperties(hIFK);
	}

	bool IsValid() const
	{
		return m_iSerialNum > 0;
	}

	int GetSerialNum() const
	{
		return m_iSerialNum <= 0 ? -1 : m_iSerialNum;
	}
	int GetVersion() const{return m_iVersion;}
	int GetStatus() const{return m_iStatus;}
	int GetNumInputs() const{return m_iNumInputs;}
	int GetNumOutputs() const{return m_iNumOutputs;}
	int GetNumAnalogs() const{return m_iNumAnalogs;}
	bool GetIsRatiometric() const{return m_bIsRatiometric;}
	const string& GetName() const{return m_sName;}
	const string& GetLabel() const{return m_sLabel;}
	const string& GetDevType() const{return m_sDevType;}
	const CPhidget_DeviceClass& GetDevClass() const{return m_dvcClass;}
	const CPhidget_DeviceID& GetDevID() const{return m_dvcId;}

	bool operator<(const IFKInfo& other) const
	{
		return GetSerialNum()<other.GetSerialNum();
	}
	string ToString() const
	{
		ostringstream oss;
		oss << GetName() << "_SN" << GetSerialNum() << "(" << GetSerialNum() << ")";
		return oss.str();
	}
#pragma endregion | Methods Public |

}*LPIFKInfo;

#pragma region | Interfaces Common |
typedef struct IPhidgetBaseListener
{
	virtual void OnPhidgetError(int deviceSN, int errCode, const string& errMsg ) =0;
}*LPIPhidgetBaseListener;
typedef struct IControlable
{
private:
	bool m_isRunning;
protected:
	void SetIsRunning(bool isRunning)
	{
		m_isRunning = isRunning;
	}
public:
	bool IsRunning() const
	{
		return m_isRunning;
	}

	virtual bool Init(/*CPhidgetHandle handle, */LPIPhidgetBaseListener pListener = NULL) = 0; 
	virtual bool Start() = 0;
	virtual void Stop() = 0;
	virtual bool Destroy() = 0;
}*LPIControlable;
#pragma endregion | Interfaces Common |

template <class T = IPhidgetBaseListener, class ARG_T = T*>
class PhidgetEventHandler
{
protected:
	typedef map<ARG_T, ARG_T> PhidgetBaseListenersMap;

#pragma region | Fields |
private:
	PhidgetBaseListenersMap m_Listeners;
	CRITICAL_SECTION        m_ResourceLock;
#pragma endregion | Fields |

#pragma region | Constructors |
private:
	PhidgetEventHandler(const PhidgetEventHandler&);
	const PhidgetEventHandler& operator =(const PhidgetEventHandler&);
public:
	PhidgetEventHandler()
	{
		InitializeCriticalSection( &m_ResourceLock ) ;
	}

	virtual ~PhidgetEventHandler()
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
	const PhidgetBaseListenersMap& GetListeners() const
	{
		return m_Listeners;
	}
	bool AttachPhidgetLisener( ARG_T pPhidgetLisener )
	{
		if(pPhidgetLisener)
		{
			EnterCriticalSection(GetCriticalSection());
			m_Listeners[pPhidgetLisener] = pPhidgetLisener;
			LeaveCriticalSection(GetCriticalSection());
		}

		return m_Listeners[pPhidgetLisener] ? true : false;
	}

	void DetachPhidgetLisener( ARG_T pPhidgetLisener )
	{
		PhidgetBaseListenersMap::const_iterator ci;
		if(pPhidgetLisener)
		{
			EnterCriticalSection(GetCriticalSection());
			ci = m_Listeners.find(pPhidgetLisener);
			if(ci != m_Listeners.end())
				m_Listeners.erase(ci);
			LeaveCriticalSection(GetCriticalSection());
		}
	}
#pragma endregion | Metods Public |
};

typedef struct IPhIFKDriverListener : public IPhidgetBaseListener
{
	virtual void OnPhIFKAttachmentChanged(int deviceSN, bool isAttached ) =0;
	virtual void OnPhIFKChanged_Digital(int deviceSN, PHIDGET_EVENT eventId, DWORD dwStateMask, DWORD dwFeet ) =0;
	virtual void OnPhIFKChanged_Analog(int deviceSN, PHIDGET_EVENT eventId, double dValue, DWORD dwFeet ) =0;
}*LPIPhIFKListener;
class PhidgetIFKEventHandler : public PhidgetEventHandler<IPhIFKDriverListener>
{
private:
	PhidgetIFKEventHandler(const PhidgetIFKEventHandler&);

public:
	PhidgetIFKEventHandler(){}
	virtual ~PhidgetIFKEventHandler(){}

protected:
	virtual void RaisePhidgetErrorEvent(int deviceSN, int errCode, const string& errMsg )
	{
		EnterCriticalSection(GetCriticalSection());
		for(PhidgetBaseListenersMap::const_iterator ci = GetListeners().begin(); ci != GetListeners().end(); ci++)
		{
			if(ci->second)
				ci->second->OnPhidgetError(deviceSN, errCode, errMsg);
		}
		LeaveCriticalSection(GetCriticalSection());
	}
	virtual void RaisePhIFKAttachmentChangedEvent(int deviceSN, bool isAttached )
	{
		EnterCriticalSection(GetCriticalSection());
		for(PhidgetBaseListenersMap::const_iterator ci = GetListeners().begin(); ci != GetListeners().end(); ci++)
		{
			if(ci->second)
				ci->second->OnPhIFKAttachmentChanged(deviceSN, isAttached);
		}
		LeaveCriticalSection(GetCriticalSection());
	}
	virtual void RaisePhIFKChangedEvent_Digital(int deviceSN, PHIDGET_EVENT eventId, DWORD dwStateMask, DWORD dwFeet )
	{
		EnterCriticalSection(GetCriticalSection());
		for(PhidgetBaseListenersMap::const_iterator ci = GetListeners().begin(); ci != GetListeners().end(); ci++)
		{
			if(ci->second)
				ci->second->OnPhIFKChanged_Digital(deviceSN, eventId, dwStateMask, dwFeet);
		}
		LeaveCriticalSection(GetCriticalSection());
	}
	virtual void RaisePhIFKChangedEvent_Analog(int deviceSN, PHIDGET_EVENT eventId, double dValue, DWORD dwFeet )
	{
		EnterCriticalSection(GetCriticalSection());
		for(PhidgetBaseListenersMap::const_iterator ci = GetListeners().begin(); ci != GetListeners().end(); ci++)
		{
			if(ci->second)
				ci->second->OnPhIFKChanged_Analog(deviceSN, eventId, dValue, dwFeet);
		}
		LeaveCriticalSection(GetCriticalSection());
	}
};


class IFKDriver
	: public PhidgetIFKEventHandler
	, public IControlable
{
#pragma region | Fields |
	CPhidgetHandle m_hKitHandle;
	IFKInfo        m_kitInfo;
	//void          *m_pHost;
#pragma endregion | Fields |

#pragma region | Constructors |
private:
	IFKDriver(const IFKDriver&);
public:
	IFKDriver()
		: m_hKitHandle(NULL)
		, m_kitInfo()
		//, m_pHost(NULL)
	{
		SetIsRunning(false);
	}

	virtual ~IFKDriver()
	{
		//Destroy();
	}
#pragma endregion | Constructors |

#pragma region | Metods Private |
private:
	static int CCONV AttachHandler(CPhidgetHandle hIFK, void *userptr);
	static int CCONV DetachHandler(CPhidgetHandle hIFK, void *userptr);
	static int CCONV ErrorHandler(CPhidgetHandle hIFK, void *userptr, int ErrorCode, const char *pErrDescr);

	static int CCONV DigitalsChangeHandler_Input(CPhidgetInterfaceKitHandle hIFK, void *userptr, int index, int state);
	static int CCONV DigitalsChangeHandler_Output(CPhidgetInterfaceKitHandle hIFK, void *userptr, int index, int state);

	static int CCONV AnalogsChangeHandler_Input(CPhidgetInterfaceKitHandle hIFK, void *userptr, int index, int value);

	void OnAttachEvent( CPhidgetHandle hIFK )
	{
		SetHandle(hIFK);		
		if(!hIFK)
			RaisePhidgetErrorEvent(GetInfo().GetSerialNum(), 0, "There is no handle.");
		else			
		{
			RaisePhIFKAttachmentChangedEvent(GetInfo().GetSerialNum(), true);
			if(GetInfo().GetNumAnalogs())
				SetIsRatiometric(true);
		}
	}
	void OnDettachEvent( CPhidgetHandle hIFK, int serialNo, const string& name )
	{
		SetHandle(hIFK);
		if(!hIFK)
			RaisePhidgetErrorEvent(GetInfo().GetSerialNum(), 0, "There is no handle.");
		else			
			RaisePhIFKAttachmentChangedEvent(GetInfo().GetSerialNum(), false);
	}
	void OnErrorEvent( CPhidgetHandle hIFK, int iErrorCode, const char * pErrDescr )
	{
		RaisePhidgetErrorEvent(GetInfo().GetSerialNum(), iErrorCode, pErrDescr);
	}

	void OnDigitalsChangedEvent( DIGITAL_TYPE dgtlType, int iState, int iIndex )
	{
		ostringstream oss;
		PHIDGET_EVENT phdgtEvnt = PHIDGET_EVENT_UNKNOWN;
		oss << "Digital ";

		int iBitStateMask = iState;
		if(iBitStateMask)
			iBitStateMask <<= iIndex;

		if(dgtlType & DIGITAL_TYPE_IN)
		{
			oss << "Input:";
			phdgtEvnt = PHIDGET_EVENT_CHANGED_INPUT;
			//UpdateViewInputs(GetAllInputsStates(), iIndex);
		}
		else if(dgtlType & DIGITAL_TYPE_OUT)
		{
			oss << "Output:";
			phdgtEvnt = PHIDGET_EVENT_CHANGED_OUTPUT;
			//UpdateViewOutputs(GetAllOutputsStates(), iIndex);
		}
		oss <<" #" << iIndex << " -> State: " << iState << endl; 

		if(phdgtEvnt == PHIDGET_EVENT_UNKNOWN)
			RaisePhidgetErrorEvent(GetInfo().GetSerialNum(), 0, "Wrong digital change.");
		else
			RaisePhIFKChangedEvent_Digital(GetInfo().GetSerialNum(), phdgtEvnt, iBitStateMask, iIndex);

		TRACE(oss.str().c_str());
	}

	void OnAnalogInputsChangedEvent( int value, int index )
	{
		ostringstream oss;
		PHIDGET_EVENT phdgtEvnt = PHIDGET_EVENT_UNKNOWN;
		oss << "Analog ";

		//int iBitStateMask = iState;
		//if(iBitStateMask)
		//	iBitStateMask <<= iIndex;

		//if(dgtlType & DIGITAL_TYPE_IN)
		//{
			oss << "Input:";
			phdgtEvnt = PHIDGET_EVENT_CHANGED_INPUT;
		//	//UpdateViewInputs(GetAllInputsStates(), iIndex);
		//}
		//else if(dgtlType & DIGITAL_TYPE_OUT)
		//{
		//	oss << "Output:";
		//	phdgtEvnt = PHIDGET_EVENT_CHANGED_OUTPUT;
		//	//UpdateViewOutputs(GetAllOutputsStates(), iIndex);
		//}
		
		double tmpVal = value * 0.2222 - 61.111;
		oss <<" #" << index << " -> RawValue: " << value << " -> Value: " << tmpVal << endl; 

		if(phdgtEvnt == PHIDGET_EVENT_UNKNOWN)
			RaisePhidgetErrorEvent(GetInfo().GetSerialNum(), 0, "Wrong analog change.");
		else
		{
			
			RaisePhIFKChangedEvent_Analog(GetInfo().GetSerialNum(), phdgtEvnt, tmpVal, index);
		}

		TRACE(oss.str().c_str());
	}
#pragma endregion | Metods Private |

#pragma region | Methods Protected |
protected:
	int GetDigitalState(DIGITAL_TYPE iDigitalType, BYTE bitId );
	UINT GetStatesMask(DIGITAL_TYPE iDigitalType);
	void OnDigitalOutputChangeRequested( int iState, int iIndex );
#pragma endregion | Methods Protected |

#pragma region | Methods Public |
public:
	//LPVOID GetHost() const
	//{
	//	return m_pHost;
	//}

	//void SetHost(LPVOID theHost)
	//{
	//	m_pHost=m_pHost;
	//}

	bool IsInUse() const
	{
		return GetListeners().size()>0;
	}

	CPhidgetHandle GetHandle() const
	{
		return m_hKitHandle;
	}
	void SetHandle(CPhidgetHandle hIFK)
	{
		if(GetHandle()!=hIFK || !hIFK)
		{
			m_hKitHandle = hIFK;
			IFKInfo::Init(m_hKitHandle, &m_kitInfo);			
		}
	}

	const IFKInfo& GetInfo() const
	{
		return m_kitInfo;
	}

	bool operator<(const IFKDriver& other) const
	{
		return GetInfo()<other.GetInfo();
	}

	static IFKDriver Create(CPhidgetHandle hDevice)
	{
		IFKDriver* drvr = new IFKDriver();
		IFKInfo::Init(hDevice, &drvr->m_kitInfo);
		return *drvr;
	}

	virtual bool /*IControlable::*/Init(LPIPhidgetBaseListener pListener = NULL)
	{
		LPIPhIFKListener pKitListener = NULL;
		bool res = false;
		if(pListener)
			pKitListener = (LPIPhIFKListener)pListener;
		res = AttachPhidgetLisener(pKitListener);

		return res;
	}

	virtual bool /*IControlable::*/Destroy()
	{
		Stop();

		if(GetHandle())
		{
			try
			{
				CPhidget_delete((CPhidgetHandle)GetHandle());
			}
			catch (CException* e)
			{
				char perrmsg[4096]={0};
				e->GetErrorMessage(perrmsg,4096);
			}
		}
			
		SetHandle(NULL);
		return GetHandle()==NULL;
	}

	virtual bool /*IControlable::*/Start()
	{
		int isStarted = 0;
		const char* errMsg;

		if(!GetHandle())
		{
			CPhidgetHandle hIFK = NULL;
			
			CPhidgetInterfaceKit_create((CPhidgetInterfaceKitHandle*)&hIFK);
			
			//Set the handlers to be run when the device is plugged in
			// or opened from software, unplugged or closed from software,
			// or generates an error.
			CPhidget_set_OnAttach_Handler(hIFK, AttachHandler, this);
			CPhidget_set_OnDetach_Handler(hIFK, DetachHandler, this);
			CPhidget_set_OnError_Handler(hIFK, ErrorHandler, this);
			
			SetHandle(hIFK);
			
			//Registers a callback that will run if an input changes.
			//Requires the handle for the Phidget, the function that
			//will be called, and an arbitrary pointer that will be
			//supplied to the callback function (may be NULL).
			CPhidgetInterfaceKit_set_OnInputChange_Handler ((CPhidgetInterfaceKitHandle)GetHandle(), DigitalsChangeHandler_Input, this);
			
			//Registers a callback that will run if an output changes.
			//Requires the handle for the Phidget, the function that
			//will be called, and an arbitrary pointer that will be
			//supplied to the callback function (may be NULL).
			CPhidgetInterfaceKit_set_OnOutputChange_Handler ((CPhidgetInterfaceKitHandle)GetHandle(), DigitalsChangeHandler_Output, this);

			//Registers a callback that will run if the analog value
			//changes by more than the SensorChangeTrigger.
			//Requires the handle for the IntefaceKit, the function that
			//will be called, and an arbitrary pointer that will be supplied
			//to the callback function (may be NULL).
			CPhidgetInterfaceKit_set_OnSensorChange_Handler ((CPhidgetInterfaceKitHandle)GetHandle(), AnalogsChangeHandler_Input, this);
		}

		if(GetHandle() && !IsRunning())
		{
			//		Logger_AddMessage("Opening the Interface-Kit connection.");

			//EnterCriticalSection( &m_ResourceLock ) ;

			//open the interface-kit for device connections
			int op = CPhidget_open((CPhidgetHandle)GetHandle(), GetInfo().GetSerialNum());

			SetIsRunning(true);

			//get the program to wait for an interface kit device to be attached
			if(!(isStarted = CPhidget_waitForAttachment((CPhidgetHandle)GetHandle(), /*iDelay*/10000)))
			{
				//Logger_AddMessage("The Interface-Kit connection succeed.");
			}
			else
			{
				CPhidget_getErrorDescription(isStarted, &errMsg);
				//CString csErr = CString(errMsg);
				//Logger_AddMessage(csErr);
				///PostStatusMessage(csErr);
			}

			//LeaveCriticalSection( &m_ResourceLock ) ;
		}
		return !isStarted;
	}

	virtual void /*IControlable::*/Stop()
	{
		if(GetHandle() && IsRunning())
		{
			CPhidget_close((CPhidgetHandle)GetHandle());
			SetIsRunning(false);
		}
	}

	int GetDigitalStateOutput( BYTE bitId )
	{
		return GetDigitalState(DIGITAL_TYPE_OUT, bitId);
	}
	int GetDigitalStateInput( BYTE bitId )
	{
		return GetDigitalState(DIGITAL_TYPE_IN, bitId);
	}
	UINT GetAllStatesInputs()
	{
		return GetStatesMask(DIGITAL_TYPE_IN);
	}
	UINT GetAllStatesOutputs()
	{
		return GetStatesMask(DIGITAL_TYPE_OUT);
	}

	void SetOutput( BYTE bitId, BYTE bitVal )
	{
		CPhidgetInterfaceKit_setOutputState((CPhidgetInterfaceKitHandle)GetHandle(), bitId, bitVal);
		int iBitStateMask = bitVal;
		if(iBitStateMask)
			iBitStateMask <<= bitId;
		
		RaisePhIFKChangedEvent_Digital(GetInfo().GetSerialNum(), PHIDGET_EVENT_REQUESTED_CHANGE_OUTPUT, iBitStateMask, bitId);
	}

	void ToggelOutput( int iOutputIndx )
	{
		if(!GetHandle())
			RaisePhidgetErrorEvent(GetInfo().GetSerialNum(), 0,"There is no handle yet!");
		else 
		{
			int outputState = GetDigitalState(DIGITAL_TYPE_OUT, iOutputIndx);

			if(!outputState)
				outputState=1;
			else
				outputState=0;

			SetOutput(iOutputIndx, outputState);
		}
	}

	void SetIsRatiometric( bool isRatiometric )
	{
		CPhidgetInterfaceKit_setRatiometric((CPhidgetInterfaceKitHandle)GetHandle(), isRatiometric);

		//RaisePhIFKChangedEvent(GetInfo().GetSerialNum(), PHIDGET_EVENT_REQUESTED_CHANGE_OUTPUT, iBitStateMask, bitId);
	}

	int GetAnalogSensitivity( BYTE index );
	void SetAnalogSensitivity( BYTE index, int sensitivity )
	{
		CPhidgetInterfaceKit_setSensorChangeTrigger((CPhidgetInterfaceKitHandle)GetHandle(), index, sensitivity);

		//RaisePhIFKChangedEvent(GetInfo().GetSerialNum(), PHIDGET_EVENT_REQUESTED_CHANGE_OUTPUT, iBitStateMask, bitId);
	}
	int GetAnalogInputRawValue(BYTE index );
	int GetAnalogInputValue(BYTE index );

	string ToString() const
	{
		return GetInfo().ToString();
	}

#pragma endregion | Methods Public |
};