// tvdbaseCLI.h

#pragma once

#include <gadgets\gadbase.h>
#include <string>
using namespace System;
using namespace System::Runtime::InteropServices;
class IGraphbuilder;


namespace shbaseCLI 
{	


	typedef void(__stdcall *IntPtrStrCallback)(IntPtr msg, int length, IntPtr pinName, int pinNameLength, DWORD handle);
	typedef void(__stdcall *LogMsgDelegate)(int msgLevel, IntPtr src, int srcLength, int msgId, IntPtr msgText, int msgTextLength, DWORD h);

    public enum class MsgLevels
    {
        MSG_INFO_LEVEL      = MSG_INFO_LEVEL,
	    MSG_DEBUG_LEVEL		= MSG_DEBUG_LEVEL,
        MSG_WARNING_LEVEL	= MSG_WARNING_LEVEL,
        MSG_ERROR_LEVEL		= MSG_ERROR_LEVEL,
        MSG_CRITICAL_LEVEL	= MSG_CRITICAL_LEVEL,
        MSG_SYSTEM_LEVEL	= MSG_SYSTEM_LEVEL
    };

    class Caller
	{
	private:
		FXLockObject * m_LockRef;
	public:

		LogMsgDelegate logStrDelegate;
		IntPtrStrCallback dataDelegate;

		DWORD callerHandle;
		void SetLock(FXLockObject * m_Lock)
		{
			m_LockRef = m_Lock;
		}
		void LockCBs()
		{
			m_LockRef->Lock();
		}
		void UnlockCBs()
		{
			m_LockRef->Unlock();
		}
	};
	class Callers
	{
	public:
		CArray<Caller*> m_mycallers;
	};
	
	public ref class GraphBuilder
	{
	private:
		IGraphbuilder* m_pBuilder;
		Caller *   m_caller;
		Callers *  m_callersCollection;
		FXLockObject * m_Lock;


		
	public:	
		GraphBuilder(IntPtr^ fnDelegate);
		GraphBuilder();
		~GraphBuilder();
	public:
		UInt32      Release();
		MsgLevels   Load(String^ fileName, String^ script, Boolean bClearOld);
		MsgLevels   Load(String^ fileName, String^ script)
		{
			return Load(fileName, script, true);
		}
		MsgLevels   Load(String^ fileName)
		{
			return Load(fileName, nullptr, true);
		}
		void        Start();
		void        Stop();
		void        Pause();
		Boolean     RunSetupDialog();
		Boolean		 ShowGadgetSetupDlg(String^ gadgetName, int x, int y);
		Boolean     ConnectRendererAndMonitor(String^ uid, IntPtr pParentWnd, String^ Monitor);
//    Boolean     ResizeRenderer( IntPtr pParentWnd , int newWidth, int newHeight );
//    static bool  ResizeRendererContent( IntPtr  pContentView , int newWidth , int newHeight );   
		Boolean     GadgetScanProperties(String^ uid, String^ text);
		Boolean     SendText(String^ PinName, String^ Data, String^ Label);
		Boolean     SendBoolean(String^ PinName, Boolean^ value, String^ Label);
		Boolean     SendQuantity(String^ PinName,double value, String^ Label);
		Boolean     SetWorkingMode(String^ GadgetName,int workingMode );

    Boolean    SetProperties( String ^ gadgetName , String ^ propertiesWithValues );

    generic<typename T>
    Boolean   SetProperty( String^ gadgetName , String^ propertyName , T propertyValue);
    Boolean		SetProperty( String^ gadgetName , String^ propertyName , String^ propertyValue );
    Boolean		SetProperty( String^ gadgetName , String^ propertyName , double propertyValue );
    Boolean		SetProperty( String^ gadgetName , String^ propertyName , int propertyValue );
    Boolean		SetProperty( String^ gadgetName , String^ propertyName , bool propertyValue );

		Boolean   SetCharIntPtrCallback(String^ PinName, IntPtr^ fnDelegate, DWORD fnHandle);
		Boolean		SetFigureIntPtrCallback(String^ PinName, IntPtr^ fnDelegate, DWORD fnHandle);

		Boolean		UnSetCharPtrCallback(String^ PinName);
		Boolean		UnSetDoublePtrCallback(String^ PinName);


    
    String ^ GetProperties( String ^ gadgetName );

    Int32 GetIntProperty( String^ GadgetName , String^ PropertyName );
    Double GetDoubleProperty( String^ GadgetName , String^ PropertyName );
    Int32 GetBooleanProperty( String^ GadgetName , String^ PropertyName );
    String^ GetStringProperty( String^ GadgetName , String^ PropertyName );

		static Caller * m_pMsgLogCaller;
		static String^ m_msgData;
    };
	
	public enum class DataType { StringType, FigureType };

	public ref class ThreadParams
	{
	public:
		array<double> ^m_ArrData;
		String^ m_StrData;
		String^ m_Pin;
		Caller *  pCaller;
		DataType m_type;
		ThreadParams(array <double>^ data, String^ pin, Caller* c)
		{
			m_type = DataType::FigureType;
			m_ArrData = gcnew array<double>(data->Length);
			pin_ptr<double> sourceArrStart = &data[0];
			pin_ptr<double> destArrstart = &m_ArrData[0];
			memcpy(destArrstart, sourceArrStart, data->Length*sizeof(double));
			m_Pin = gcnew String(pin);
			pCaller = c;
		}

		ThreadParams(String^ data, String^ pin, Caller* c)
		{
			m_type = DataType::StringType;
			m_StrData = gcnew String(data);
			m_Pin = gcnew String(pin);
			pCaller = c;
		}
	};

	/*
	public ref class SendHandler
	{
	public:
		void Send(Object^ obj)
		{	
			StringThreadParams^ params = safe_cast<StringThreadParams^>(obj);
			

			IntPtrStrCallback dataCallback = (IntPtrStrCallback)params->pCaller->strDelegate;
			DWORD h = params->pCaller->callerHandle;

			IntPtr lpData = Marshal::StringToHGlobalAnsi(params->mData);
			int    length = params->mData->Length;

			IntPtr lpPinName = Marshal::StringToHGlobalAnsi(params->mPin);
			int    pinNamelength = params->mPin->Length;

			dataCallback(lpData, length, lpPinName, pinNamelength, h);
		}
	};

	public ref class FigureSendHandler
	{
	public:
		void Send(Object^ obj)
		{
			ThreadParams^ params = safe_cast<ThreadParams^>(obj);

			IntPtrStrCallback dataCallback = (IntPtrStrCallback)params->pCaller->strDelegate;
			DWORD h = params->pCaller->callerHandle;

			int size = Marshal::SizeOf(params->mData[0]) * params->mData->Length;

			int i1 = params->mData->Length;
			int i2 = Marshal::SizeOf(params->mData[0]);

			int i3 = Marshal::SizeOf(params->mData[1]);
			int i4 = sizeof(double);

			IntPtr pnt = Marshal::AllocHGlobal(size);
			Marshal::Copy(params->mData, 0, pnt, params->mData->Length);

			IntPtr lpPinName = Marshal::StringToHGlobalAnsi(params->mPin);
			int    pinNamelength = params->mPin->Length;

			int length = params->mData->Length;
			dataCallback(pnt, length, lpPinName, pinNamelength, h);

		}
	};
	*/
	public ref class SendHandler
	{
	public:
		void Send(Object^ obj)
		{
			ThreadParams^ params = safe_cast<ThreadParams^>(obj);
			IntPtrStrCallback dataCallback = (IntPtrStrCallback)params->pCaller->dataDelegate;
			DWORD h = params->pCaller->callerHandle;
			switch (params->m_type)
			{
				case DataType::FigureType:
				{
					int size = Marshal::SizeOf(params->m_ArrData[0]) * params->m_ArrData->Length;

					int i1 = params->m_ArrData->Length;
					int i2 = Marshal::SizeOf(params->m_ArrData[0]);
					int i3 = Marshal::SizeOf(params->m_ArrData[1]);
					int i4 = sizeof(double);

					IntPtr pnt = Marshal::AllocHGlobal(size);
					Marshal::Copy(params->m_ArrData, 0, pnt, params->m_ArrData->Length);

					IntPtr lpPinName = Marshal::StringToHGlobalAnsi(params->m_Pin);
					int    pinNamelength = params->m_Pin->Length;
					int length = params->m_ArrData->Length;

					dataCallback(pnt, length, lpPinName, pinNamelength, h);
				}
				break;
				case DataType::StringType:
				{
					IntPtr lpData = Marshal::StringToHGlobalAnsi(params->m_StrData);
					int    length = params->m_StrData->Length;

					IntPtr lpPinName = Marshal::StringToHGlobalAnsi(params->m_Pin);
					int    pinNamelength = params->m_Pin->Length;
					
					dataCallback(lpData, length, lpPinName, pinNamelength, h);
				}
				break;
				default:
					break;
			}			
		}
	};
}
