#include "stdafx.h"
#include "PhIKDriver.h"



//////////////////////////////////////////////////////////////////////////
//
// Phidget Kit callbacks handlers
//
int CCONV IFKDriver::AttachHandler( CPhidgetHandle hIFK, void *userptr )
{
	IFKDriver *pDrvr = (IFKDriver *)userptr;
	ASSERT(pDrvr);

	pDrvr->OnAttachEvent(hIFK);

	return 0;
}
int CCONV IFKDriver::DetachHandler( CPhidgetHandle hIFK, void *userptr )
{
	int serialNo;
	const char *name;

	CPhidget_getDeviceName (hIFK, &name);
	CPhidget_getSerialNumber(hIFK, &serialNo);

	IFKDriver *pDrvr = (IFKDriver *)userptr;
	ASSERT(pDrvr);

	pDrvr->OnDettachEvent(hIFK, serialNo, name);

	return 0;
}
int CCONV IFKDriver::ErrorHandler( CPhidgetHandle hIFK, void *userptr, int ErrorCode, const char *pErrDescr )
{
	IFKDriver *pDrvr = (IFKDriver *)userptr;
	ASSERT(pDrvr);

	pDrvr->OnErrorEvent(hIFK, ErrorCode, pErrDescr);

	return 0;
}

//callback that will run if an input changes.
//Index - Index of the input that generated the event, State - boolean (0 or 1) representing the input state (on or off)
int CCONV IFKDriver::DigitalsChangeHandler_Input( CPhidgetInterfaceKitHandle hIFK, void *userptr, int index, int state )
{
	IFKDriver *pDrvr = (IFKDriver *)userptr;
	ASSERT(pDrvr);

	pDrvr->OnDigitalsChangedEvent(DIGITAL_TYPE_IN, state, index);

	return 0;
}
//callback that will run if an output changes.
//Index - Index of the output that generated the event, State - boolean (0 or 1) representing the output state (on or off)
int CCONV IFKDriver::DigitalsChangeHandler_Output( CPhidgetInterfaceKitHandle hIFK, void *userptr, int index, int state )
{
	IFKDriver *pDrvr = (IFKDriver *)userptr;
	ASSERT(pDrvr);

	pDrvr->OnDigitalsChangedEvent(DIGITAL_TYPE_OUT, state, index);

	return 0;
}

int CCONV IFKDriver::AnalogsChangeHandler_Input( CPhidgetInterfaceKitHandle hIFK, void *userptr, int index, int value )
{
	IFKDriver *pDrvr = (IFKDriver *)userptr;
	ASSERT(pDrvr);

	pDrvr->OnAnalogInputsChangedEvent(value, index);
	return 0;
}


int IFKDriver::GetDigitalState(DIGITAL_TYPE iDigitalType, BYTE bitId )
{
	int bitVal = 0;

	if(!GetHandle())
		RaisePhidgetErrorEvent(GetInfo().GetSerialNum(), 0,"There is no handle yet!");
	else if(iDigitalType & DIGITAL_TYPE_IN)
		CPhidgetInterfaceKit_getInputState((CPhidgetInterfaceKitHandle)GetHandle(), bitId, &bitVal);
	else if(iDigitalType & DIGITAL_TYPE_OUT)
		CPhidgetInterfaceKit_getOutputState((CPhidgetInterfaceKitHandle)GetHandle(), bitId, &bitVal);

	return bitVal;
}
UINT IFKDriver::GetStatesMask( DIGITAL_TYPE iDigitalType )
{
	UINT resultStatesMask = 0;
	int iBitState = 0;
	if(!GetHandle())
		RaisePhidgetErrorEvent(GetInfo().GetSerialNum(), 0,"There is no handle yet!");
	else 
	{
		int numInOut = iDigitalType == DIGITAL_TYPE_IN ? GetInfo().GetNumInputs() : GetInfo().GetNumOutputs();
		for(int indx=numInOut-1; indx>=0; indx-- )
		{	
			iBitState = GetDigitalState(iDigitalType, indx);

			if(iBitState)
				resultStatesMask |= (iBitState<<indx);
		}
	}
	return resultStatesMask;
}

int IFKDriver::GetAnalogSensitivity(BYTE index )
{
	int indxSens = 0;

	if(!GetHandle())
		RaisePhidgetErrorEvent(GetInfo().GetSerialNum(), 0,"There is no handle yet!");
	else
		CPhidgetInterfaceKit_getSensorChangeTrigger((CPhidgetInterfaceKitHandle)GetHandle(), index, &indxSens);

	return indxSens;
}

int IFKDriver::GetAnalogInputRawValue(BYTE index )
{
	int indxSens = 0;

	if(!GetHandle())
		RaisePhidgetErrorEvent(GetInfo().GetSerialNum(), 0,"There is no handle yet!");
	else
		CPhidgetInterfaceKit_getSensorRawValue((CPhidgetInterfaceKitHandle)GetHandle(), index, &indxSens);

	return indxSens;
}
int IFKDriver::GetAnalogInputValue(BYTE index )
{
	int indxSens = 0;

	if(!GetHandle())
		RaisePhidgetErrorEvent(GetInfo().GetSerialNum(), 0,"There is no handle yet!");
	else
	{
		CPhidgetInterfaceKit_getSensorValue((CPhidgetInterfaceKitHandle)GetHandle(), index, &indxSens);
		OnAnalogInputsChangedEvent(indxSens, index);
	}

	return indxSens;
}


//void IFKDriver::AsyncStartController( int iSerNo, int iDelay )
//{
//	CWinThread *pWatchThread = NULL;
//	pWatchThread=AfxBeginThread(OnAsyncStartController, (LPVOID)this, THREAD_PRIORITY_ABOVE_NORMAL);
//	pWatchThread->m_bAutoDelete = TRUE;
//}
//
////bool CPhidgetDriver::StartController( int iSerNo, int iDelay )
////{
////	int isStarted = 0;
////	const char* errMsg;
////
////	if(!m_hIntrfaceKit)
////	{
//////	Logger_AddMessage(_T("Interface Kit did not created. Try Again!"));
////	}
////	else
////	{
//////		Logger_AddMessage("Opening the Interface-Kit connection.");
////
////		EnterCriticalSection( &m_ResourceLock ) ;
////
////		//open the interface-kit for device connections
////		int op = CPhidget_open((CPhidgetHandle)m_hIntrfaceKit, iSerNo);
////
////		//PostStatusMessage(_T(WAIT_MESSAGE));
////
////		//get the program to wait for an interface kit device to be attached
////		if(!(isStarted = CPhidget_waitForAttachment((CPhidgetHandle)m_hIntrfaceKit, iDelay)))
////		{
////			//Logger_AddMessage("The Interface-Kit connection succeed.");
////		}
////		else
////		{
////			CPhidget_getErrorDescription(isStarted, &errMsg);
////			//CString csErr = CString(errMsg);
////			//Logger_AddMessage(csErr);
////			///PostStatusMessage(csErr);
////		}
////
////		LeaveCriticalSection( &m_ResourceLock ) ;
////	}
////	return !isStarted;
////}
//
//UINT IFKDriver::OnAsyncStartController( LPVOID lpData )
//{
//	int result = FALSE;
//
//	IFKDriver * pSender = (IFKDriver*)lpData;
//
//	if(pSender)
//	{
//		result = (UINT)pSender->StartController(pSender->GetKitSerialNo(), pSender->GetStartupDelay());
//	}
//
//	return (UINT)result;
//}