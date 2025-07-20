// This is the main DLL file.
#include "stdafx.h"
#include "shbase.h"
#include <gadgets\shkernel.h>
#include <gadgets\stdsetup.h>
#include <gadgets\textframe.h>
#include <gadgets\quantityframe.h>
#include <gadgets\FigureFrame.h>
#include <msclr/marshal.h>
#include <msclr\marshal_cppstd.h>
#include <typeinfo>

using namespace shbaseCLI;
using namespace msclr::interop;
using namespace System::Threading;

void __stdcall PrintMsg(int msgLevel, LPCSTR src, int msgId, LPCSTR msgText)
{
	if (GraphBuilder::m_pMsgLogCaller == nullptr)
		return;

	String^ source = gcnew String(src);
	String^ text = gcnew String(msgText);
	
	IntPtr lpSoucre = Marshal::StringToHGlobalAnsi(source);
	int srcLength = source->Length;
	IntPtr lpText = Marshal::StringToHGlobalAnsi(text);
	int textLength = text->Length;

	if (GraphBuilder::m_pMsgLogCaller->logStrDelegate)
		GraphBuilder::m_pMsgLogCaller->logStrDelegate(msgLevel, lpSoucre, srcLength, msgId, lpText, textLength, 0);
}


GraphBuilder::GraphBuilder(IntPtr^ fnDelegate)
{
	GraphBuilder::m_pMsgLogCaller = new Caller();
	GraphBuilder::m_pMsgLogCaller->logStrDelegate = static_cast<LogMsgDelegate>(fnDelegate->ToPointer());

	this->GraphBuilder::GraphBuilder();
}
GraphBuilder::GraphBuilder()
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState());
    m_pBuilder = NULL;
    
	m_caller = new Caller();
	m_callersCollection = new Callers();
	m_Lock = new FXLockObject();

	m_pBuilder = ::Tvdb400_CreateBuilder();
    ::FxInitMsgQueues(PrintMsg);
	IPluginLoader* PluginLoader=m_pBuilder->GetPluginLoader();
	PluginLoader->RegisterPlugins(m_pBuilder);


}
GraphBuilder::~GraphBuilder()
{	
    AFX_MANAGE_STATE(AfxGetStaticModuleState());
    Release();
    delete m_pMsgLogCaller;
}

UInt32  GraphBuilder::Release()
{
    AFX_MANAGE_STATE(AfxGetStaticModuleState());
    if (m_pBuilder)
    {
        m_pBuilder->Stop();
        Sleep(400);
        FxEnableMsgQueuesLog( false ) ;
        UInt32 retV=m_pBuilder->Release();
        ASSERT(retV==0);
        m_pBuilder = NULL;
        Sleep( 200 ) ;
        ::FxExitMsgQueues();
        return retV;
    }
    return 0;
}

MsgLevels  GraphBuilder::Load(String^ fileName, String^ script, Boolean bClearOld)
{
    AFX_MANAGE_STATE(AfxGetStaticModuleState());
    if (m_pBuilder)
    {
        char *fName=NULL, *Script=NULL;
        if (fileName!=nullptr)
        {
            CStringA a(fileName);
            fName = (char*)malloc(a.GetLength()+1);
            strcpy(fName,(LPCSTR)a);
        }
        if (script!=nullptr)
        {
            CStringA a(script);
            Script = (char*)malloc(a.GetLength()+1);
            strcpy(Script,(LPCSTR)a);
        }	
        MsgLevels retV=(MsgLevels)m_pBuilder->Load(fName,Script, bClearOld);
        if (fName)  free(fName);
        if (Script) free(Script);
        return retV;
    }
    return MsgLevels::MSG_CRITICAL_LEVEL;
}

void GraphBuilder::Start()
{
    AFX_MANAGE_STATE(AfxGetStaticModuleState());
    if (m_pBuilder)
        m_pBuilder->Start();
}

void GraphBuilder::Stop()
{
    AFX_MANAGE_STATE(AfxGetStaticModuleState());
    if (m_pBuilder)
        m_pBuilder->Stop();
}

void GraphBuilder::Pause()
{
    AFX_MANAGE_STATE(AfxGetStaticModuleState());
    if (m_pBuilder)
        m_pBuilder->Pause();
}

Boolean GraphBuilder::RunSetupDialog()
{
    AFX_MANAGE_STATE(AfxGetStaticModuleState());
    if (m_pBuilder)
        return (Tvdb400_RunSetupDialog(m_pBuilder)!=FALSE);
    return false;
}

Boolean GraphBuilder::ShowGadgetSetupDlg(String^ gadgetName , int x, int y)
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState());
	CStringA name(gadgetName);
	if (m_pBuilder)
		return (Tvdb400_ShowGadgetSetupDlg(m_pBuilder, name, CPoint(x, y)) != FALSE);
	return false;
}

Boolean GraphBuilder::ConnectRendererAndMonitor(String^ uid, IntPtr pParentWnd, String^ Monitor)
{
    AFX_MANAGE_STATE(AfxGetStaticModuleState());
    if (!m_pBuilder) return false;
    CStringA UID(uid);
    CStringA MONITOR(Monitor);
    CRenderGadget* RenderGadget;
    return (m_pBuilder->ConnectRendererAndMonitor(UID, CWnd::FromHandle((HWND)pParentWnd.ToInt64()), MONITOR, RenderGadget)!=FALSE);
}

//Boolean GraphBuilder::ResizeRenderer( IntPtr pParentWnd , int newWidth , int newHeight )
//{
//  AFX_MANAGE_STATE( AfxGetStaticModuleState() );
//
//  HWND hWindow = (HWND) pParentWnd.ToInt32();
//  return   ::SetWindowPos( hWindow , HWND_TOP , 0 , 0 , newWidth , newHeight , SWP_NOMOVE | SWP_NOZORDER | SWP_SHOWWINDOW );
//}
//
//bool shbaseCLI::GraphBuilder::ResizeRendererContent( IntPtr pContentView , int newWidth , int newHeight )
//{
//  AFX_MANAGE_STATE( AfxGetStaticModuleState() );
//
//  HWND hWindow = (HWND) pContentView.ToInt32();
//  return   ::SetWindowPos( hWindow , HWND_TOP , 0 , 0 , newWidth , newHeight , SWP_NOMOVE | SWP_NOZORDER /*| SWP_SHOWWINDOW*/ );
//}

Boolean GraphBuilder::GadgetScanProperties(String^ uid, String^ text)
{
    AFX_MANAGE_STATE(AfxGetStaticModuleState());
    bool Invalidate=false;
    if (!m_pBuilder) return false;
    CGadget* gdgt=m_pBuilder->GetGadget(CStringA(uid));
    if (!gdgt) return false;
    return gdgt->ScanProperties(CStringA(text),Invalidate);
}


Boolean GraphBuilder::SetWorkingMode(String^ GadgetName, int workingMode)
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState());
	if (!m_pBuilder) return false;


	CGadget* pGadget = m_pBuilder->GetGadget(CStringA(GadgetName));

	if (pGadget)
	{
		pGadget->SetMode(workingMode);
		return true;
	}

	return false;
}

Boolean GraphBuilder::SetProperties( String^ gadgetName , String^ propertiesWithValues )
{
  AFX_MANAGE_STATE( AfxGetStaticModuleState() );
  if ( !m_pBuilder || !gadgetName || !propertiesWithValues ) return false;

  CStringA strGadgetName( gadgetName );
  CStringA strPropertiesWithValues( propertiesWithValues );

  CGadget* pGadget = m_pBuilder->GetGadget( strGadgetName );

  if ( pGadget )
  {
    FXPropertyKit pk( (FXString) strPropertiesWithValues );

    bool Invalidate = false;
    if ( pGadget->ScanProperties( (LPCTSTR) (pk) , Invalidate ) )
      return true;
  }
  return false;
}

generic<typename T>
Boolean GraphBuilder::SetProperty(String^ gadgetName, String^ propertyName, T propertyValue)
{
  bool isDone = false;
  AFX_MANAGE_STATE(AfxGetStaticModuleState());
  if (!m_pBuilder) return false;

  CGadget* pGadget = m_pBuilder->GetGadget(CStringA(gadgetName));
  CStringA name(propertyName);

  if (pGadget)
  {
    FXPropertyKit pk;
    bool isWritten = false;

    if (T::typeid == Int32::typeid) isWritten = pk.WriteInt(name, (int)propertyValue);
    else if (T::typeid == Double::typeid) isWritten = pk.WriteDouble(name, (double)propertyValue);
    else if (T::typeid == String::typeid)
    {
      CStringA value((String^)propertyValue);
      isWritten = pk.WriteString(name, value);
    }

    if (isWritten)
    {
      bool Invalidate = false;
      isDone = pGadget->ScanProperties(pk, Invalidate);
    }
  }

  return isDone;
}

Boolean GraphBuilder::SetProperty(String^ gadgetName, String^ propertyName, String^ propertyValue)
{
  return SetProperty<String^>(gadgetName, propertyName, propertyValue);
}
Boolean GraphBuilder::SetProperty( String ^ gadgetName , String ^ propertyName , double propertyValue )
{
  return SetProperty<double>(gadgetName, propertyName, propertyValue);
}
Boolean GraphBuilder::SetProperty( String ^ gadgetName , String ^ propertyName , int propertyValue )
{
  return SetProperty<int>(gadgetName, propertyName, propertyValue);
}
Boolean GraphBuilder::SetProperty( String ^ gadgetName , String ^ propertyName , bool propertyValue )
{
  return SetProperty( gadgetName , propertyName , propertyValue ? 1 : 0 );
  //Boolean res = false;

  //AFX_MANAGE_STATE( AfxGetStaticModuleState() );
  //if ( m_pBuilder )
  //{
  //  CGadget* pGadget = m_pBuilder->GetGadget( CStringA( gadgetName ) );      

  //  CStringA name( propertyName );

  //  if ( pGadget )
  //  {
  //    FXPropertyKit pk;
  //    bool Invalidate = false;

  //    pGadget->PrintProperties( pk );

  //    if ( pk.WriteBool( name , propertyValue ) )
  //      res = true;

  //    if ( res && !pGadget->ScanProperties( pk , Invalidate ) )
  //      res = false;

  //  }
  //}

  //return res;
}

Boolean GraphBuilder::SendBoolean(String^ PinName, Boolean^ value, String^ Label)
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState());
	if (!m_pBuilder) return false;
	CStringA pinName(PinName);
	bool bValue = value->Equals(true) ? true : false;
	CStringA label(Label);
	CBooleanFrame * bf = CBooleanFrame::Create(bValue);
	if (bf)
	{
		if (label)
			bf->SetLabel(label);
		bf->ChangeId(0);
		bf->SetTime(GetHRTickCount());
		if ((m_pBuilder)->SendDataFrame(bf, pinName))
			return true;

		bf->Release(bf);
	}
	return false;
}
Boolean GraphBuilder::SendText(String^ PinName, String^ Data, String^ Label)
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState());
	if (!m_pBuilder) return false;
	CStringA pinName(PinName);
	CStringA data(Data);
	CStringA label(Label);
	CTextFrame* tf = CTextFrame::Create(data);
	if (tf)
	{
		if (Label)
			tf->SetLabel(label);
		if (m_pBuilder->SendDataFrame(tf, pinName))
			return true;

		tf->Release(tf);
	}

	return false;
}
Boolean GraphBuilder::SendQuantity(String^ PinName, double Data, String^ Label)
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState());
	if (!m_pBuilder) return false;

	CStringA pinName(PinName);
	double data = Data;
	CStringA label(Label);
	CQuantityFrame* qf = CQuantityFrame::Create(data);
	if (qf)
	{
		if (Label)
			qf->SetLabel(label);
		if (m_pBuilder->SendDataFrame(qf, pinName))
			return true;

		qf->Release(qf);
	}

	return false;
}


BOOL CALLBACK fnTextResultCallBack(CDataFrame*& pDataFrame, CString& PinName, void * lParam)
{
	Caller * pCaller = (Caller *)lParam;
	CTextFrame * pTextFrame = pDataFrame->GetTextFrame(DEFAULT_LABEL); 
	if (pTextFrame == 0)
	{
		pDataFrame->Release(pDataFrame);
		return true;
	}
	String^ text = gcnew String(pTextFrame->GetString());
	pDataFrame->Release(pDataFrame);
	if (text)
	{		
		DWORD h = pCaller->callerHandle;	
		String^ pinName = gcnew String(PinName.GetString());
		
		//StringThreadParams^ param = gcnew StringThreadParams(text, pinName, pCaller);
		ThreadParams^ param = gcnew ThreadParams(text, pinName, pCaller);

		SendHandler^ sender = gcnew SendHandler;
		Thread^ threads = gcnew Thread(gcnew ParameterizedThreadStart(sender, &SendHandler::Send));
		threads->Start(param);	
	}	
   
	return TRUE; // data frame is released
}
/*
BOOL CALLBACK fnFigureResultCallBack(CDataFrame*& pDataFrame, CString& PinName, void * lParam)
{
	Caller * pCaller = (Caller *)lParam;
	CFigureFrame *pFf = NULL;
	//CFramesIterator* Iterator = pDataFrame->CreateFramesIterator(figure);
	pDataFrame->Release(pDataFrame);

	pFf = CFigureFrame::Create();
	for (int i = 0; i < 100; i++)
	{
		CDPoint pt(i, 2 * i); //= new CDPoint(i, 2 * i);
		pFf->AddPoint(pt);
	}


	//do
	{

		array<double> ^Values = gcnew array<double>(pFf->GetCount() * 2);
		pin_ptr<double> array_start = &Values[0];
		pin_ptr<double> cdpoint_start = reinterpret_cast<double*>(pFf->GetData());
		memcpy(array_start, cdpoint_start, (pFf->GetCount() * 2)*sizeof(double));

		DWORD h = pCaller->callerHandle;
		String^ pinName = gcnew String(PinName.GetString());

		//FigureThreadParams^ param = gcnew FigureThreadParams(Values, pinName, pCaller);
		ThreadParams^ param = gcnew ThreadParams(Values, pinName, pCaller);

		SendHandler^ sender = gcnew SendHandler;
		Thread^ threads = gcnew Thread(gcnew ParameterizedThreadStart(sender, &SendHandler::Send));
		threads->Start(param);

	};

	pFf->Release(pFf);

	return TRUE; // data frame is released
}
*/

BOOL CALLBACK fnFigureResultCallBack(CDataFrame*& pDataFrame, CString& PinName, void * lParam)
{
	Caller * pCaller = (Caller *)lParam;
	CFigureFrame *pFf = NULL;
	CFramesIterator* Iterator = pDataFrame->CreateFramesIterator(figure);
	
	if (Iterator == NULL)
	{
		pDataFrame->Release(pDataFrame);
		return true;
	}	
	do
	{	
		pFf = (CFigureFrame*)Iterator->Next(DEFAULT_LABEL);
		if (pFf == NULL)
			break;
	
		array<double> ^Values = gcnew array<double>((int)pFf->GetCount()*2);
		pin_ptr<double> array_start = &Values[0];
		pin_ptr<double> cdpoint_start = reinterpret_cast<double*>(pFf->GetData());
		memcpy(array_start, cdpoint_start, (pFf->GetCount() * 2)*sizeof(double));
	
		DWORD h = pCaller->callerHandle;
		String^ pinName = gcnew String(PinName.GetString());

		ThreadParams^ param = gcnew ThreadParams(Values, pinName, pCaller);

		SendHandler^ sender = gcnew SendHandler;
		Thread^ threads = gcnew Thread(gcnew ParameterizedThreadStart(sender, &SendHandler::Send));
		threads->Start(param);

	} while (pFf != NULL);
	pDataFrame->Release(pDataFrame);
	pFf->Release(pFf);
	return TRUE; // data frame is released
}


Boolean GraphBuilder::SetCharIntPtrCallback(String^ PinName, IntPtr^ fnDelegate, DWORD fnHandle)
{
	FXAutolock alock(*m_Lock);
	Caller * pCaller = new Caller();
	pCaller->callerHandle = fnHandle;
	//GC::KeepAlive(fnDelegate);
	//pCaller->delegatePointer = /*(void*)*/(void(__stdcall *)(char*, int, char*,int,DWORD))Marshal::GetFunctionPointerForDelegate(fnDelegate).ToPointer();
	pCaller->dataDelegate = static_cast<IntPtrStrCallback>(fnDelegate->ToPointer());
	//GC::Collect();
	pCaller->SetLock(m_Lock);
	String^ PName = gcnew String(PinName);
	char* pPinName = (char*)(void*)Marshal::StringToHGlobalAnsi(PName);
	pin_ptr<char> pinPinName = pPinName;
	m_pBuilder->SetOutputCallback(pPinName, (OutputCallback)fnTextResultCallBack, pCaller);
	return true;
}


Boolean GraphBuilder::SetFigureIntPtrCallback(String^ PinName, IntPtr^ fnDelegate, DWORD fnHandle)
{
	FXAutolock alock(*m_Lock);
	Caller * pCaller = new Caller();
	pCaller->callerHandle = fnHandle;
	//GC::KeepAlive(fnDelegate);
	//pCaller->delegatePointer = /*(void*)*/(void(__stdcall *)(char*, int, char*,int,DWORD))Marshal::GetFunctionPointerForDelegate(fnDelegate).ToPointer();
	pCaller->dataDelegate = static_cast<IntPtrStrCallback>(fnDelegate->ToPointer());
	//GC::Collect();
	pCaller->SetLock(m_Lock);
	String^ PName = gcnew String(PinName);
	char* pPinName = (char*)(void*)Marshal::StringToHGlobalAnsi(PName);
	pin_ptr<char> pinPinName = pPinName;
	BOOL bret = m_pBuilder->SetOutputCallback(pPinName, (OutputCallback)fnFigureResultCallBack, pCaller);
	return true;
}



Boolean GraphBuilder::UnSetCharPtrCallback(String^ PinName)
{
	FXAutolock alock(*m_Lock);
	CStringA pinName(PinName);
	m_pBuilder->SetOutputCallback(pinName, NULL, NULL);
	return true;
}
Boolean GraphBuilder::UnSetDoublePtrCallback(String^ PinName)
{
	FXAutolock alock(*m_Lock);
	CStringA pinName(PinName);
	m_pBuilder->SetOutputCallback(pinName, NULL, NULL);
	return true;
}


String^ GraphBuilder::GetProperties( String^ gadgetName )
{
  AFX_MANAGE_STATE( AfxGetStaticModuleState() );

  String^ res;

  if ( !m_pBuilder && !gadgetName )
    return res;

  CStringA strGadgetName( gadgetName );

  CGadget * pGadget = ((IGraphbuilder*) m_pBuilder)->GetGadget( strGadgetName ) ;
  
  if ( pGadget )
  {
    FXPropertyKit pk ;

    if ( pGadget->PrintProperties( pk ) )
    {
      res = gcnew String( pk );
      GC::KeepAlive( res );
    }
  }
  return res;
}

Int32 GraphBuilder::GetIntProperty( String^ GadgetName, String^ PropertyName)
{
  AFX_MANAGE_STATE( AfxGetStaticModuleState() );

  if ( !m_pBuilder && !GadgetName && !PropertyName )
    return INT_MAX;
  
  CStringA gadgetName( GadgetName );
  CGadget * pGadget =
    ((IGraphbuilder*) m_pBuilder)->GetGadget( gadgetName ) ;
  if ( pGadget )
  {
    FXPropertyKit pk ;
    CStringA propertyName( PropertyName );
    INT32 iPropVal;
    if ( pGadget->PrintProperties( pk ) )
    {
      if ( pk.GetInt( propertyName , iPropVal ) )
        return iPropVal;
    }      
  }
  return INT_MAX;

}
Double GraphBuilder::GetDoubleProperty( String^ GadgetName , String^ PropertyName )
{
  AFX_MANAGE_STATE( AfxGetStaticModuleState() );
  if ( !m_pBuilder && !GadgetName && !PropertyName )
    return 0xcdcdcdcd;

  CStringA gadgetName( GadgetName );
  CGadget * pGadget =
    ((IGraphbuilder*) m_pBuilder)->GetGadget( gadgetName ) ;
  if ( pGadget )
  {
    FXPropertyKit pk ;
    CStringA propertyName( PropertyName );
    Double dPropVal;
    if ( pGadget->PrintProperties( pk ) )
    {
       if(pk.GetDouble( propertyName , dPropVal ))
         return dPropVal;
    }      
  }
  return 1e100;

}

Int32 GraphBuilder::GetBooleanProperty( String^ GadgetName , String^ PropertyName )
{
  AFX_MANAGE_STATE( AfxGetStaticModuleState() );
  if ( !m_pBuilder && !GadgetName && !PropertyName )
    return -1;

  CStringA gadgetName( GadgetName );
  CGadget * pGadget =
    ((IGraphbuilder*) m_pBuilder)->GetGadget( gadgetName ) ;
  if ( pGadget )
  {
    FXPropertyKit pk ;
    CStringA propertyName( PropertyName );
    if ( pGadget->PrintProperties( pk ) )
    {
      BOOL bPropVal ;
      if ( pk.GetBool( propertyName , bPropVal ) )
      {
        return ( bPropVal != FALSE) ;
      }
    }
   
  }
  return -1;
}
String^ GraphBuilder::GetStringProperty( String^ GadgetName , String^ PropertyName )
{
  AFX_MANAGE_STATE( AfxGetStaticModuleState() );
  if ( !m_pBuilder && !GadgetName && !PropertyName )
    return "error";

  CStringA gadgetName( GadgetName );
  CGadget * pGadget =
    ((IGraphbuilder*) m_pBuilder)->GetGadget( gadgetName ) ;
  if ( pGadget )
  {
    FXPropertyKit pk ;
    CStringA propertyName( PropertyName );
    FXString fxPropVal;
    if ( pGadget->PrintProperties( pk ) )
      if ( pk.GetString( propertyName , fxPropVal ) )
      {
        String ^ prop = gcnew String( fxPropVal.GetString());      
        GC::KeepAlive( prop );
        return prop;
      }
  }
  return "error";
}