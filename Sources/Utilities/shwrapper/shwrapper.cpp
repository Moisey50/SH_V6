// shwrapper.cpp : Defines the initialization routines for the DLL.
//

#include "stdafx.h"

#define SHWRAPPER_COMPILE

#include <gadgets\gadbase.h>
#include <gadgets\TextFrame.h>
#include <gadgets\QuantityFrame.h>
#include <gadgets\FigureFrame.h>
#include <gadgets\RectFrame.h>
#include <gadgets\VideoFrame.h>
#include <gadgets\tvinspect.h>
#include <gadgets\stdsetup.h>
#include <video\TVFrame.h>
#include <comutil.h>
#include "helpers\shwrapper.h"
#include <fxfc\lockobject.h>

#include "resource.h"		// main symbols

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

//
//TODO: If this DLL is dynamically linked against the MFC DLLs,
//		any functions exported from this DLL which call into
//		MFC must have the AFX_MANAGE_STATE macro added at the
//		very beginning of the function.
//
//		For example:
//
//		extern "C" BOOL PASCAL EXPORT ExportedFunction()
//		{
//			AFX_MANAGE_STATE(AfxGetStaticModuleState());
//			// normal function body here
//		}
//
//		It is very important that this macro appear in each
//		function, prior to any calls into MFC.  This means that
//		it must appear as the first statement within the 
//		function, even before any object variable declarations
//		as their constructors may generate calls into the MFC
//		DLL.
//
//		Please see MFC Technical Notes 33 and 58 for additional
//		details.
//


//***************************************************************************
// Utilities

__forceinline const char * w2a(const wchar_t * w, char * a , int iLen_a )
{
  int len = (int) wcslen( w );
  if ( len >= iLen_a )
    return NULL ;
  size_t ConvLen ;
  if ( wcstombs_s( &ConvLen , a , iLen_a , w , len ) )
    return NULL ;
  return a ;
}

__forceinline const char * w2a( const wchar_t * w )
{
  int len = (int) wcslen( w );
  char * pResult = new char[len+1] ;
  size_t ConvLen ;
  if ( wcstombs_s( &ConvLen , pResult , len+1 , w , len ) )
    return NULL ;
  return pResult ;
}

__forceinline const wchar_t * a2w(const char * a, wchar_t * w , int iLen_w )
{
  int len = (int) strlen(a);
  if ( len >= iLen_w )
    return NULL ;
  size_t ConvLen ;
  if ( mbstowcs_s( &ConvLen , w , len+1 , a , len ) )
    return NULL ;
  return w ;
}

__forceinline const wchar_t * a2w( const char * pTextA )
{
  int len = (int) strlen(pTextA) ;
  wchar_t * pWideString = new wchar_t[len + 1] ;

  if ( pWideString )
  {
    size_t ConvLen ;
    mbstowcs_s( &ConvLen , pWideString , len+1 , pTextA , len );
    pWideString[len] = 0 ;
  }
  return pWideString ;
}
// End of utilities
//*************************************************************************************
class TextCallBackSet 
{
public:
  TextCallBackSet( bool Multi = false ) 
  { m_FuncAddrW = NULL ; m_FuncAddrA = NULL ; m_bMulti = Multi ;} ;
  FXString      m_PinName ;
  TextCallbackW m_FuncAddrW ;
  TextCallbackA m_FuncAddrA ;
  void   *      m_pParam ;
  bool          m_bMulti ;

  TextCallBackSet& operator =(TextCallBackSet& Orig)
  {
    m_PinName = Orig.m_PinName ;
    m_FuncAddrW = Orig.m_FuncAddrW ;
    m_FuncAddrA = Orig.m_FuncAddrA ;
    m_pParam = Orig.m_pParam ;
    m_bMulti = Orig.m_bMulti ;
    return *this ;
  } ;
} ;


class DataCallBackSet 
{
public:
  DataCallBackSet( bool Multi = false ) 
  { m_FuncAddrW = NULL ; m_FuncAddrA = NULL ; m_bMulti = Multi ;} ;
  FXString      m_PinName ;
  DataCallbackW m_FuncAddrW ;
  DataCallbackA m_FuncAddrA ;
  void   *      m_pParam ;
  bool          m_bMulti ;

  DataCallBackSet& operator =(DataCallBackSet& Orig)
  {
    m_PinName = Orig.m_PinName ;
    m_FuncAddrW = Orig.m_FuncAddrW ;
    m_FuncAddrA = Orig.m_FuncAddrA ;
    m_pParam = Orig.m_pParam ;
    m_bMulti = Orig.m_bMulti ;
    return *this ;
  } ;
} ;

class WindowToRendererCorresponding 
{
public:
  WindowToRendererCorresponding() { m_pWindow = NULL ; m_pRenderer = NULL ; }
  WindowToRendererCorresponding( CWnd * pWnd , CRenderGadget * pRender ,
    FXString& RenderName , FXString& WindowCaption ) 
  { m_pWindow = pWnd ; m_pRenderer = pRender ; m_RenderName = RenderName ; }
  CWnd * m_pWindow ;
  CRenderGadget * m_pRenderer ;
  FXString m_RenderName ;
  FXString m_WindowCaption ;

  WindowToRendererCorresponding& operator = (const WindowToRendererCorresponding& Orig)
  {
    m_pWindow = Orig.m_pWindow ;
    m_pRenderer = Orig.m_pRenderer ;
    m_RenderName = Orig.m_RenderName ;
    m_WindowCaption = Orig.m_WindowCaption ;
    return *this ;
  }
};

typedef CArray <TextCallBackSet , TextCallBackSet&> TextCallBacks ;
typedef CArray <DataCallBackSet , DataCallBackSet&> DataCallBacks ;
typedef CArray<WindowToRendererCorresponding> WinToRendArray ;


static AFX_EXTENSION_MODULE SHWrapperDll = { NULL, NULL };
CDynLinkLibrary* pThisDll = NULL;

extern "C" int APIENTRY
  DllMain(HINSTANCE hInstance, DWORD dwReason, LPVOID lpReserved)
{
  // Remove this if you use lpReserved
  UNREFERENCED_PARAMETER(lpReserved);

  if (dwReason == DLL_PROCESS_ATTACH)
  {
    TRACE0("SHWrapper.DLL Initializing!\n");

    // Extension DLL one-time initialization
    if (!AfxInitExtensionModule(SHWrapperDll, hInstance))
      return 0;

    // Insert this DLL into the resource chain
    // NOTE: If this Extension DLL is being implicitly linked to by
    //  an MFC Regular DLL (such as an ActiveX Control)
    //  instead of an MFC application, then you will want to
    //  remove this line from DllMain and put it in a separate
    //  function exported from this Extension DLL.  The Regular DLL
    //  that uses this Extension DLL should then explicitly call that
    //  function to initialize this Extension DLL.  Otherwise,
    //  the CDynLinkLibrary object will not be attached to the
    //  Regular DLL's resource chain, and serious problems will
    //  result.

    pThisDll = new CDynLinkLibrary(SHWrapperDll);
    //     // Init plugin
    //     FXString fName=::getAppName(SHWrapperDll.hModule);
    //     ASSERT(fName.GetLength()<APP_NAME_MAXLENGTH);
  }
  else if (dwReason == DLL_PROCESS_DETACH)
  {
    TRACE0("SHWrapper.DLL Terminating!\n");
    // Terminate the library before destructors are called
    AfxTermExtensionModule(SHWrapperDll);
  }
  return 1;   // ok
}



ConnectGraph * pThisGraph = NULL ;



void __stdcall PrintGraphMsg(
  int msgLevel, const char * src, int msgId, const char * msgText)
{
  // we will be here only when wide chars are used
  FXString mes;
  if ( !pThisGraph || !pThisGraph->GetLoggerFunc() )
  {
    mes.Format(_T("+++ %d %s %d %s\n"),msgLevel, src, msgId, msgText);
    TRACE(mes);
  }
  else  
  {
    pThisGraph->a2wMessage( msgLevel , src , msgId , msgText ) ;
  }
}

BOOL __stdcall TextCallBackFunc(CDataFrame*& lpData, FXString& idPin, void* lpParam)
{
  BOOL bres = FALSE ;
  ConnectGraph * pConnect = (ConnectGraph*)lpParam ;

  pConnect->LockCBs() ;
  TextCallBacks * pCB = (TextCallBacks*)pConnect->GetCB() ; 
  for ( int i = 0 ; i < pCB->GetCount() ; i++ )
  {
    TextCallBackSet& CallBack = pCB->ElementAt(i) ;
    if ( (LPCTSTR)idPin == CallBack.m_PinName )
    {
      CTextFrame* tf = NULL ;
      CFramesIterator* Iterator = lpData->CreateFramesIterator(text) ;
      FXString data ;
      if ( Iterator )
      {
        int iCallBacksCounter = 0 ;
        while ( tf = (CTextFrame*) Iterator->Next(NULL) )
        {
          if ( !data.IsEmpty() )
            data += ':' ;
          data += tf->GetLabel() ;
          data += ':' ;
          data += tf->GetString() ;
          if ( !CallBack.m_bMulti )
            break ;
        }
        delete Iterator ;
      }
      else  // no container with texts
      {
        tf = lpData->GetTextFrame( NULL ) ;
        if ( tf )
        {
          data += tf->GetLabel() ;
          data += ':' ;
          data += tf->GetString() ;
        }
      }
      if ( !data.IsEmpty() )
      {
        if ( CallBack.m_FuncAddrW )
        {
          const wchar_t * res = a2w(data);
          bres = CallBack.m_FuncAddrW( res , CallBack.m_pParam );
          delete[] res ;
        }
        else if ( CallBack.m_FuncAddrA )
        {
          bres = CallBack.m_FuncAddrA( (LPCTSTR)data , CallBack.m_pParam );
        }
      }
    }
  }
  pConnect->UnlockCBs() ;
  lpData->Release(lpData);
  return TRUE ;
}

bool ProcessCallbackFrame( CDataFrame * df , CallBackDataA& DataA , CallBackDataW& DataW , 
                          ConnectGraph * pConnect , DataCallBackSet& CallBack )
{
  bool bres = false ;
  datatype Type = df->GetDataType() ;
  CallBackDataType cbType ;
  switch ( Type )
  {
  case nulltype: cbType = DATA_NULL ; break ;
  case text:     cbType = DATA_TEXT ; break ;
  case logical:  cbType = DATA_BOOL ; break ;
  case quantity: cbType = DATA_QUANTITY ; break ;
  case figure:   cbType = DATA_FIGURE ; break ;
  case vframe:   cbType = DATA_IMAGE ; break ;
  case rectangle:cbType = DATA_RECT ; break ;
  default:       cbType = DATA_UNKNOWN ; break ;
  }
  if ( CallBack.m_FuncAddrA )
  {
    DataA.Reset( cbType , df->GetTime() ) ;
    int iLabLen = (int) _tcslen( df->GetLabel() ) ;
    if ( iLabLen )
    {
      char * pLab = new char[iLabLen + 1] ;
      _tcscpy( pLab , df->GetLabel() ) ;
      DataA.m_Label = pLab ;
    }
    DataA.SetId( df->GetId() ) ;
  }
  if ( CallBack.m_FuncAddrW )
  {
    DataW.Reset( cbType , df->GetTime() ) ;
    DataW.m_Label = a2w( df->GetLabel() ) ;
    DataW.SetId( df->GetId() ) ;
  }
  switch ( Type )
  {
  case nulltype:
    {
      if ( CallBack.m_FuncAddrA )
        bres = CallBack.m_FuncAddrA( DataA , CallBack.m_pParam );
      if ( CallBack.m_FuncAddrW )
        bres = CallBack.m_FuncAddrW( DataW , CallBack.m_pParam );
    }
    break ;
  case text:
    {
      CTextFrame * tf = df->GetTextFrame(NULL) ;
      if ( tf )
      {
        if ( CallBack.m_FuncAddrA )
        {
          DataA.m_Par1.pString = (tf->GetString()) ;
          bres = CallBack.m_FuncAddrA( DataA , CallBack.m_pParam );
          DataA.m_Par1.pString = NULL ;
        }
        if ( CallBack.m_FuncAddrW )
        {
          DataW.m_Par1.pString = (LPCTSTR) a2w( tf->GetString() ) ;
          bres = CallBack.m_FuncAddrW( DataW , CallBack.m_pParam );
          delete[] DataW.m_Par1.pString ;
        }
      }
    }
    break ;
  case logical:
    {
      CBooleanFrame * bf = df->GetBooleanFrame(NULL) ;
      if ( bf )
      {
        if ( CallBack.m_FuncAddrA )
        {
          DataA.m_Par1.BoolVal = (bool)*bf ;
          bres = (bool)CallBack.m_FuncAddrA( DataA , CallBack.m_pParam );
        }
        if ( CallBack.m_FuncAddrW )
        {
          DataW.m_Par1.BoolVal = (bool)*bf ;
          bres = (bool)CallBack.m_FuncAddrW( DataW , CallBack.m_pParam );
        }
      }
    }
    break ;
  case quantity:
    {
      CQuantityFrame * qf = df->GetQuantityFrame(NULL) ;
      if ( qf )
      {
        if ( CallBack.m_FuncAddrA )
        {
          DataA.m_Par1.Quantity = (double)*qf ;
          bres = (bool)CallBack.m_FuncAddrA( DataA , CallBack.m_pParam );
        }
        if ( CallBack.m_FuncAddrW )
        {
          DataW.m_Par1.Quantity = (double)*qf ;
          bres = (bool)CallBack.m_FuncAddrW( DataW , CallBack.m_pParam );
        }
      }
    }
    break ;
  case figure:
    {
      CFigureFrame * ff = df->GetFigureFrame(NULL) ;
      if ( ff )
      {
        if ( CallBack.m_FuncAddrA )
        {
          DataA.m_Par1.pFigure = (CDPointArray*)ff ;
          bres = (bool)CallBack.m_FuncAddrA( DataA , CallBack.m_pParam );
          DataA.m_Par1.pFigure = NULL ;
        }
        if ( CallBack.m_FuncAddrW )
        {
          DataW.m_Par1.pFigure = (CDPointArray*)ff ;
          bres = (bool)CallBack.m_FuncAddrW( DataW , CallBack.m_pParam );
          DataW.m_Par1.pFigure = NULL;
        }
      }
    }
    break ;
  case rectangle:
    {
      CRectFrame * rf = df->GetRectFrame(NULL) ;
      if ( rf )
      {
        if ( CallBack.m_FuncAddrA )
        {
          DataA.m_Par1.Rectangle = (RECT)*rf ;
          bres = (bool)CallBack.m_FuncAddrA( DataA , CallBack.m_pParam );
        }
        if ( CallBack.m_FuncAddrW )
        {
          DataW.m_Par1.Rectangle = (RECT)*rf ;
          bres = (bool)CallBack.m_FuncAddrW( DataW , CallBack.m_pParam );
        }
      }
    }
    break ;
  case vframe:
    {
      CVideoFrame * vf = df->GetVideoFrame(NULL) ;
      if ( vf )
      {
//        vf->AddRef() ;
        if ( CallBack.m_FuncAddrA )
        {
          DataA.m_Par1.BmpInfo = vf->lpBMIH ;
          DataA.SetImageData( (vf->lpData)? vf->lpData : (LPBYTE) &vf->lpBMIH[1] ) ;
          bres = (bool)CallBack.m_FuncAddrA( DataA , CallBack.m_pParam );
          DataA.m_Par1.BmpInfo = NULL;
          DataA.m_Par2 = NULL;
        }
        if ( CallBack.m_FuncAddrW )
        {
          DataW.m_Par1.BmpInfo = vf->lpBMIH ;
          DataW.SetImageData( (vf->lpData)? vf->lpData : (LPBYTE) &vf->lpBMIH[1] ) ;
          bres =(bool) CallBack.m_FuncAddrW( DataW , CallBack.m_pParam );
          DataW.m_Par1.BmpInfo = NULL;
          DataW.m_Par2 = NULL;
        }
//        vf->Release() ;
      }
    }
    break ;
  default:
    {
      FXString TypeName( Tvdb400_TypeToStr(Type) ) ;
      if ( CallBack.m_FuncAddrA )
      {
        DataA.m_Par1.pString  = (LPCTSTR)TypeName ;
        bres = (bool)CallBack.m_FuncAddrA( DataA , CallBack.m_pParam );
      }
      if ( CallBack.m_FuncAddrW )
      {
        DataW.m_Par1.pString = (LPCTSTR) a2w( TypeName ) ;
        bres = (bool)CallBack.m_FuncAddrW( DataW , CallBack.m_pParam );
        delete[] DataW.m_Par1.pString ;
      }
    }
    break ;
  }
  if ( DataW.m_Label )
    delete[] DataW.m_Label ;

  return true ;
}

bool ProcessCallbackFrame( CDataFrame * df ,  
                          ConnectGraph * pConnect , DataCallBackSet& CallBack )
{
  bool bres = false ;
  datatype Type = df->GetDataType() ;
  CallBackDataType cbType ;
  switch ( Type )
  {
  case nulltype: cbType = DATA_NULL ; break ;
  case text:     cbType = DATA_TEXT ; break ;
  case logical:  cbType = DATA_BOOL ; break ;
  case quantity: cbType = DATA_QUANTITY ; break ;
  case figure:   cbType = DATA_FIGURE ; break ;
  case vframe:   cbType = DATA_IMAGE ; break ;
  case rectangle:cbType = DATA_RECT ; break ;
  default:       cbType = DATA_UNKNOWN ; break ;
  }
  CallBackDataA * pDataA = ( CallBack.m_FuncAddrA ) ? 
    new CallBackDataA( cbType , df->GetTime() ) : NULL ;
  CallBackDataW * pDataW = ( CallBack.m_FuncAddrW ) ? 
    new CallBackDataW( cbType , df->GetTime() ) : NULL ;
  if ( pDataA )
  {
    if ( df->GetLabel() )
      pDataA->m_Label = strdup( df->GetLabel() ) ;
    pDataA->SetId( df->GetId() ) ;
  }
  if ( pDataW )
  {
    if ( df->GetLabel() )
      pDataW->m_Label = a2w( df->GetLabel() ) ;
    pDataW->SetId( df->GetId() ) ;
  }

  switch ( Type )
  {
  case nulltype:
    {
      if ( CallBack.m_FuncAddrA )
        bres = CallBack.m_FuncAddrA( *pDataA , CallBack.m_pParam );
      if ( CallBack.m_FuncAddrW )
        bres = CallBack.m_FuncAddrW( *pDataW , CallBack.m_pParam );
    }
    break ;
  case text:
    {
      CTextFrame * tf = df->GetTextFrame(NULL) ;
      if ( tf )
      {
        if ( pDataA )
        {
          pDataA->m_Par1.pString = strdup(tf->GetString()) ;
          bres = CallBack.m_FuncAddrA( *pDataA , CallBack.m_pParam );
        }
        if ( pDataW )
        {
          pDataW->m_Par1.pString = (LPCTSTR) a2w( tf->GetString() ) ;
          bres = CallBack.m_FuncAddrW( *pDataW , CallBack.m_pParam );
        }
      }
    }
    break ;
  case logical:
    {
      CBooleanFrame * bf = df->GetBooleanFrame(NULL) ;
      if ( bf )
      {
        if ( pDataA )
        {
          pDataA->m_Par1.BoolVal = (bool)*bf ;
          bres = (bool)CallBack.m_FuncAddrA( *pDataA , CallBack.m_pParam );
        }
        if ( pDataW )
        {
          pDataW->m_Par1.BoolVal = (bool)*bf ;
          bres = (bool)CallBack.m_FuncAddrW( *pDataW , CallBack.m_pParam );
        }
      }
    }
    break ;
  case quantity:
    {
      CQuantityFrame * qf = df->GetQuantityFrame(NULL) ;
      if ( qf )
      {
        if ( CallBack.m_FuncAddrA )
        {
          pDataA->m_Par1.Quantity = (double)*qf ;
          bres = (bool)CallBack.m_FuncAddrA( *pDataA , CallBack.m_pParam );
        }
        if ( CallBack.m_FuncAddrW )
        {
          pDataW->m_Par1.Quantity = (double)*qf ;
          bres = (bool)CallBack.m_FuncAddrW( *pDataW , CallBack.m_pParam );
        }
      }
    }
    break ;
  case figure:
    {
      CFigureFrame * ff = df->GetFigureFrame(NULL) ;
      if ( ff )
      {
        if ( pDataA )
        {
          pDataA->m_Par1.pFigure = new CDPointArray ;
          pDataA->m_Par1.pFigure->Copy( *(CDPointArray*)ff ) ;
          bres = (bool)CallBack.m_FuncAddrA( *pDataA , CallBack.m_pParam );
        }
        if ( pDataW )
        {
          pDataW->m_Par1.pFigure = new CDPointArray ;
          pDataW->m_Par1.pFigure->Copy( *(CDPointArray*)ff ) ;
          bres = (bool)CallBack.m_FuncAddrW( *pDataW , CallBack.m_pParam );
        }
      }
    }
    break ;
  case rectangle:
    {
      CRectFrame * rf = df->GetRectFrame(NULL) ;
      if ( rf )
      {
        if ( pDataA )
        {
          pDataA->m_Par1.Rectangle = (RECT)*rf ;
          bres = (bool)CallBack.m_FuncAddrA( *pDataA , CallBack.m_pParam );
        }
        if ( pDataW )
        {
          pDataW->m_Par1.Rectangle = (RECT)*rf ;
          bres = (bool)CallBack.m_FuncAddrW( *pDataW , CallBack.m_pParam );
        }
      }
    }
    break ;
  case vframe:
    {
      CVideoFrame * vf = df->GetVideoFrame(NULL) ;
      if ( vf )
      {
        DWORD dwImageSize = GetImageSize( vf ) ;
        if ( pDataA )
        {
          pDataA->m_Par1.BmpInfo = new BITMAPINFOHEADER ;
          memcpy( pDataA->m_Par1.BmpInfo , vf->lpBMIH , sizeof(BITMAPINFOHEADER) ) ;
          LPBYTE pImageData = new UINT8[ dwImageSize ] ;
          memcpy( pImageData , ( vf->lpData ) ? vf->lpData : (LPVOID)&vf->lpBMIH[1] , dwImageSize ) ;
          pDataA->SetImageData( pImageData ) ;
          bres = (bool)CallBack.m_FuncAddrA( *pDataA , CallBack.m_pParam );
        }
        if ( pDataW )
        {
          pDataW->m_Par1.BmpInfo = new BITMAPINFOHEADER ;
          memcpy( pDataW->m_Par1.BmpInfo , vf->lpBMIH , sizeof(BITMAPINFOHEADER) ) ;
          LPBYTE pImageData = new UINT8[ dwImageSize ] ;
          memcpy( pImageData , ( vf->lpData ) ? vf->lpData : (LPVOID)&vf->lpBMIH[1] , dwImageSize ) ;
          pDataW->SetImageData( pImageData ) ;
          bres =(bool) CallBack.m_FuncAddrW( *pDataW , CallBack.m_pParam );
        }
      }
    }
    break ;
  default:
    {
      FXString TypeName( Tvdb400_TypeToStr(Type) ) ;
      if ( pDataA )
      {
        pDataA->m_Par1.pString  = (LPCTSTR)TypeName ;
        bres = (bool)CallBack.m_FuncAddrA( *pDataA , CallBack.m_pParam );
      }
      if ( pDataW )
      {
        pDataW->m_Par1.pString = (LPCTSTR) a2w( TypeName ) ;
        bres = (bool)CallBack.m_FuncAddrW( *pDataW , CallBack.m_pParam );
      }
    }
    break ;
  }

  return bres ;
}

BOOL __stdcall DataCallBackFunc(CDataFrame*& lpData, FXString& idPin, void* lpParam)
{
  BOOL bres = FALSE ;
  ConnectGraph * pConnect = (ConnectGraph*)lpParam ;
  CallBackDataA DataA ;
  CallBackDataW DataW ;

  pConnect->LockCBs() ;
  DataCallBacks * pCB = (DataCallBacks*)pConnect->GetDataCB() ;

  for ( int i = 0 ; i < pCB->GetCount() ; i++ )
  {
    DataCallBackSet& CallBack = pCB->ElementAt(i) ;
    if ( (LPCTSTR)idPin == CallBack.m_PinName )
    {
      CDataFrame * df = NULL ;
      CFramesIterator* Iterator = lpData->CreateFramesIterator(transparent) ;
      if ( Iterator )
      {
        while ( df = (CDataFrame*) Iterator->Next(NULL) )
        {
          ProcessCallbackFrame( df , DataA , DataW , pConnect , CallBack ) ;
        }
        delete Iterator ;
      }
      else
      {
        ProcessCallbackFrame( lpData , DataA , DataW , pConnect , CallBack ) ;
      }
    }
  }
  pConnect->UnlockCBs() ;
  lpData->Release(lpData);
  return TRUE ;
}

BOOL __stdcall DataCallBackFuncEx(CDataFrame*& lpData, FXString& idPin, void* lpParam)
{
  BOOL bres = FALSE ;
  ConnectGraph * pConnect = (ConnectGraph*)lpParam ;

  pConnect->LockCBs() ;
  DataCallBacks * pCB = (DataCallBacks*)pConnect->GetDataCB() ;

  for ( int i = 0 ; i < pCB->GetCount() ; i++ )
  {
    DataCallBackSet& CallBack = pCB->ElementAt(i) ;
    if ( (LPCTSTR)idPin == CallBack.m_PinName )
    {
      CDataFrame * df = NULL ;
      CFramesIterator* Iterator = lpData->CreateFramesIterator(transparent) ;
      if ( Iterator )
      {
        while ( df = (CDataFrame*) Iterator->Next(NULL) )
        {
          ProcessCallbackFrame( df , pConnect , CallBack ) ;
        }
        delete Iterator ;
      }
      else
      {
        ProcessCallbackFrame( lpData , pConnect , CallBack ) ;
      }
    }
  }
  pConnect->UnlockCBs() ;
  lpData->Release(lpData);
  return TRUE ;
}

// ConnectGraph::ConnectGraph(FXString * tvgName)
// {
//   m_pBuilder = NULL ;
//   m_pPluginLoader = NULL ;
//   if ( tvgName )
//     Init( *tvgName ) ;
// }
ConnectGraph::ConnectGraph(const char * tvgName)
	: m_pBuilder()
	, m_pPluginLoader()
	, m_CB()
	, m_DataCB()
	, m_pCorrespondence()
	, m_Lock()
	, m_pLogger()
	, m_bRunBeforeInspect()
	, m_bExternalConnect()
{
  m_pBuilder = NULL ;
  m_pPluginLoader = NULL ;
  m_pCorrespondence = NULL ;
  if ( tvgName )
    Init( tvgName ) ;
}

ConnectGraph::ConnectGraph(const wchar_t * tvgName)
	: ConnectGraph::ConnectGraph()
{
  if ( tvgName )
    Init( tvgName ) ;
}

bool ConnectGraph::Init(const char * tvgName , PrintLogMsgFuncA pLogger/* = NULL */, bool toStart/* = true */)
{
  m_EvaluationMsg.Empty() ;
  Disconnect() ;

  m_pBuilder = (void*)Tvdb400_CreateBuilder();
  FxInitMsgQueues( (pLogger)? pLogger : PrintGraphMsg );
  m_pPluginLoader = (void*)((IGraphbuilder*) m_pBuilder)->GetPluginLoader();
  ((IPluginLoader*)m_pPluginLoader)->RegisterPlugins( ((IGraphbuilder*) m_pBuilder) );

  int iRes = ( ( IGraphbuilder* ) m_pBuilder )->Load( tvgName ) ;
  if ( iRes == MSG_SYSTEM_LEVEL )
  {
    m_EvaluationMsg = "License Expired" ;
  }
  else if ( iRes == MSG_NOT_REGISTERED )
  {
    m_EvaluationMsg = "Software is not registered" ;
  }
  if ( iRes >= MSG_ERROR_LEVEL ) 
  { 
    Sleep( 50 ) ; // get time for FXGraphMsgQueue work.
    ((IGraphbuilder*) m_pBuilder)->Release(); 
    m_pBuilder = NULL;
    FxExitMsgQueues();
    return false;
  }

  OperateGraph(toStart);
  m_CB = (void*) new TextCallBacks ;
  m_DataCB = (void*) new DataCallBacks ;
  m_bExternalConnect = false ;
  m_pCorrespondence = (void*) new WinToRendArray ;
  m_GraphPath = tvgName ;

  return true;
}

bool ConnectGraph::Init(const wchar_t * tvgName ,  PrintLogMsgFuncW pLogger/* = NULL */, bool toStart/* = true */)
{
  const char * asciiStr = w2a(tvgName);
  m_pLogger = pLogger ;
  bool bRes = Init( asciiStr , NULL, toStart) ;
  delete[] asciiStr ;
  return bRes ;
}

bool ConnectGraph::Init(void * pBuilder , void * pLoader )
{
  Disconnect() ;

  m_pBuilder = pBuilder ;
  m_pPluginLoader = pLoader ;
  m_CB = (void*) new TextCallBacks ;
  m_DataCB = (void*) new DataCallBacks ;
  m_bExternalConnect = true ;
  m_pCorrespondence = (void*) new WinToRendArray ;
  return true;
}

bool ConnectGraph::SaveGraph( LPCTSTR pPath )
{
  LPCTSTR pGraphPath = ( pPath )? pPath : (LPCTSTR)m_GraphPath ;

  bool bRes = ((IGraphbuilder*)m_pBuilder)->Save( pGraphPath ) ;
  if ( bRes && ( m_GraphPath != pGraphPath ) )
    m_GraphPath = pGraphPath ;

  return bRes ;
}

ConnectGraph::~ConnectGraph()
{
  Disconnect() ;
}
void ConnectGraph::Disconnect( int iTimeout )
{
  if ( m_pBuilder )
  {
    RemoveCallbacks() ;
    //     WinToRendArray * pCorresp = 
    //       (WinToRendArray *)m_pCorrespondence ;
    //     for ( int i = 0 ; i < pCorresp->GetCount() ; i++ )
    //     {
    //       WindowToRendererCorresponding OneCorresp = pCorresp->GetAt(i) ;
    //       ((IGraphbuilder*) m_pBuilder)->ConnectRendererAndMonitor( 
    //         OneCorresp.m_RenderName , NULL , NULL , 
    //         OneCorresp.m_pRenderer ) ;  
    //     }

    if ( m_bExternalConnect )
      m_pBuilder = NULL ;
    else
    {
      ((IGraphbuilder*) m_pBuilder)->Stop();	
      Sleep( iTimeout ) ;
      ((IGraphbuilder*) m_pBuilder)->Release(); 
      m_pBuilder = NULL;
      FxExitMsgQueues();
    }
  }
}

void ConnectGraph::RemoveCallbacks()
{
  if ( m_pBuilder )
  {
    LockCBs() ;
    TextCallBacks * pCB = (TextCallBacks*) m_CB ;
    if(pCB)
    {
      for ( int i = 0 ; i < pCB->GetCount() ; i++ )
      {
        TextCallBackSet& CallBack = pCB->ElementAt(i) ; 
        CallBack.m_FuncAddrA = NULL ;
        CallBack.m_FuncAddrW = NULL ;
        ((IGraphbuilder*) m_pBuilder)->SetOutputCallback(
          CallBack.m_PinName , NULL, NULL);
      }
    }

    DataCallBacks * pDataCB = (DataCallBacks*) m_DataCB ;
    if(pDataCB)
    {
      for ( int i = 0 ; i < pDataCB->GetCount() ; i++ )
      {
        DataCallBackSet& CallBack = pDataCB->ElementAt(i) ;
        CallBack.m_FuncAddrA = NULL ;
        CallBack.m_FuncAddrW = NULL ;
        ((IGraphbuilder*) m_pBuilder)->SetOutputCallback(
          CallBack.m_PinName , NULL, NULL);
      }
    }
    Sleep(50) ;
    if(pCB)
      pCB->RemoveAll() ;
    if(pDataCB)
      pDataCB->RemoveAll() ;
    UnlockCBs() ;
    delete (TextCallBacks*) m_CB, m_CB = 0;
    delete (DataCallBacks*) m_DataCB, m_DataCB = 0;
    delete (WinToRendArray*) m_pCorrespondence, m_pCorrespondence = 0;
  }
}

void ConnectGraph::OperateGraph(bool toStart/* = false*/)
{
  IGraphbuilder* builder = (IGraphbuilder*) m_pBuilder;
  if(builder)
    toStart ? builder->Start() : builder->Stop();
}

void ConnectGraph::PauseGraph()
{
  IGraphbuilder* builder = (IGraphbuilder*) m_pBuilder;
  if(builder)
    builder->Pause();
}

void ConnectGraph::StepFrwrdGraph()
{
  IGraphbuilder* builder = (IGraphbuilder*) m_pBuilder;
  if(builder)
    builder->StepFwd();
}


void ConnectGraph::RunSetupDialog( const char * pGadgetName , CPoint * pPosition )
{
  if ( m_pBuilder )
  {
    if ( !pGadgetName )
      Tvdb400_RunSetupDialog( (IGraphbuilder*) m_pBuilder);
    else
    {
      FXString GadgetName( pGadgetName ) ;
      Tvdb400_ShowGadgetSetupDlg( (IGraphbuilder*) m_pBuilder , GadgetName , *pPosition ) ;
    }
  }
}

void ConnectGraph::RunSetupDialog( const wchar_t * pGadgetName , CPoint * pPosition )
{
  const char * asciiGadgetName = w2a( pGadgetName );
  RunSetupDialog( asciiGadgetName , pPosition ) ;
  delete[] asciiGadgetName ;
}

bool ConnectGraph::SetTextCallBack(
  const char * PinName , TextCallbackA oc, LPVOID lParam , bool bMulti )
{
  FXAutolock alock( m_Lock ) ;
  DataCallBacks * pDataCB = (DataCallBacks*) m_DataCB ;
  for ( int i = 0 ; i < pDataCB->GetCount() ; i++ )
  {
    if ( pDataCB->ElementAt(i).m_PinName == PinName )
    {
      ((IGraphbuilder*) m_pBuilder)->SetOutputCallback(
        pDataCB->ElementAt(i).m_PinName , NULL, NULL );
      pDataCB->RemoveAt( i ) ;
      i-- ;
    }
  }
  TextCallBacks * pCB = (TextCallBacks*) m_CB ;
  for ( int i = 0 ; i < pCB->GetCount() ; i++ )
  {
    if ( pCB->ElementAt(i).m_PinName == PinName )
    {
      ((IGraphbuilder*) m_pBuilder)->SetOutputCallback(
        pCB->ElementAt(i).m_PinName , NULL, NULL );
      pCB->RemoveAt( i ) ;
      i-- ;
    }
  }
  if ( oc == NULL )
    return true ;
  TextCallBackSet NewCallBack(bMulti) ;
  NewCallBack.m_PinName = PinName ;
  NewCallBack.m_FuncAddrA = oc ;
  NewCallBack.m_pParam = lParam ;
  if (!((IGraphbuilder*) m_pBuilder)->SetOutputCallback( 
    (LPCTSTR) NewCallBack.m_PinName , TextCallBackFunc, this) )
  {
    return false;
  }
  pCB->Add( NewCallBack ) ;
  return true ;
}

bool ConnectGraph::SetTextCallBack(
  const  wchar_t * PinName , TextCallbackW oc, LPVOID lParam , bool bMulti )
{
  const char * asciiPinName = w2a( PinName );

  FXAutolock alock( m_Lock ) ;

  DataCallBacks * pDataCB = (DataCallBacks*) m_DataCB ;
  for ( int i = 0 ; i < pDataCB->GetCount() ; i++ )
  {
    if ( pDataCB->ElementAt(i).m_PinName == asciiPinName )
    {
      ((IGraphbuilder*) m_pBuilder)->SetOutputCallback(
        pDataCB->ElementAt(i).m_PinName , NULL, NULL );
      pDataCB->RemoveAt( i ) ;
      i-- ;
    }
  }
  TextCallBacks * pCB = (TextCallBacks*) m_CB ;
  for ( int i = 0 ; i < pCB->GetCount() ; i++ )
  {
    if ( pCB->ElementAt(i).m_PinName == asciiPinName )
    {
      ((IGraphbuilder*) m_pBuilder)->SetOutputCallback(
        pCB->ElementAt(i).m_PinName , NULL, NULL );
      pCB->RemoveAt( i ) ;
      i-- ;
    }
  }

  if ( oc == NULL )
  {
    delete[] asciiPinName ;
    return true ;
  }
  TextCallBackSet NewCallBack(bMulti) ;
  NewCallBack.m_PinName = asciiPinName ;
  NewCallBack.m_FuncAddrW = oc ;
  NewCallBack.m_pParam = lParam ;
  delete[] asciiPinName ;
  if ( !((IGraphbuilder*) m_pBuilder)->SetOutputCallback( 
    (LPCTSTR) NewCallBack.m_PinName , TextCallBackFunc, this) )
  {
    return false;
  }
  pCB->Add( NewCallBack ) ;
  return true ;
}

bool ConnectGraph::SetDataCallBack(
  const char * PinName , DataCallbackA oc, LPVOID lParam , bool bMulti )
{
  FXAutolock alock( m_Lock ) ;

  DataCallBacks * pDataCB = (DataCallBacks*) m_DataCB ;
  for ( int i = 0 ; i < pDataCB->GetCount() ; i++ )
  {
    if ( pDataCB->ElementAt(i).m_PinName == PinName )
    {
      ((IGraphbuilder*) m_pBuilder)->SetOutputCallback(
        pDataCB->ElementAt(i).m_PinName , NULL, NULL );
      pDataCB->RemoveAt( i ) ;
      i-- ;
    }
  }
  TextCallBacks * pCB = (TextCallBacks*) m_CB ;
  for ( int i = 0 ; i < pCB->GetCount() ; i++ )
  {
    if ( pCB->ElementAt(i).m_PinName == PinName )
    {
      ((IGraphbuilder*) m_pBuilder)->SetOutputCallback(
        pCB->ElementAt(i).m_PinName , NULL, NULL );
      pCB->RemoveAt( i ) ;
      i-- ;
    }
  }

  if ( oc == NULL )
    return true ;


  DataCallBackSet NewCallBack(bMulti) ;
  NewCallBack.m_PinName = PinName ;
  NewCallBack.m_FuncAddrA = oc ;
  NewCallBack.m_pParam = lParam ;
  if (!((IGraphbuilder*) m_pBuilder)->SetOutputCallback( 
    (LPCTSTR) NewCallBack.m_PinName , DataCallBackFunc, this) )
  {
    return false;
  }
  pDataCB->Add( NewCallBack ) ;
  return true ;
}

bool ConnectGraph::SetDataCallBackW(
  const  wchar_t * PinName , DataCallbackW oc, LPVOID lParam , bool bMulti )
{
  const char * asciiPinName = w2a( PinName );

  FXAutolock alock( m_Lock ) ;

  DataCallBacks * pDataCB = (DataCallBacks*) m_DataCB ;
  for ( int i = 0 ; i < pDataCB->GetCount() ; i++ )
  {
    if ( pDataCB->ElementAt(i).m_PinName == asciiPinName )
    {
      ((IGraphbuilder*) m_pBuilder)->SetOutputCallback(
        pDataCB->ElementAt(i).m_PinName , NULL, NULL );
      pDataCB->RemoveAt( i ) ;
      i-- ;
    }
  }
  TextCallBacks * pCB = (TextCallBacks*) m_CB ;
  for ( int i = 0 ; i < pCB->GetCount() ; i++ )
  {
    if ( pCB->ElementAt(i).m_PinName == asciiPinName )
    {
      ((IGraphbuilder*) m_pBuilder)->SetOutputCallback(
        pCB->ElementAt(i).m_PinName , NULL, NULL );
      pCB->RemoveAt( i ) ;
      i-- ;
    }
  }

  if ( oc == NULL )
  {
    delete[] asciiPinName ;
    return true ;
  }
  DataCallBackSet NewCallBack(bMulti) ;
  NewCallBack.m_PinName = asciiPinName ;
  NewCallBack.m_FuncAddrW = oc ;
  NewCallBack.m_pParam = lParam ;
  delete[] asciiPinName ;
  if (!((IGraphbuilder*) m_pBuilder)->SetOutputCallback( 
    (LPCTSTR) NewCallBack.m_PinName , DataCallBackFunc, this) )
  {
    return false;
  }
  pDataCB->Add( NewCallBack ) ;
  return true ;
}
typedef bool (__stdcall * SetCallBackA )(const char * PinName , DataCallbackA oc , LPVOID lParam , bool bMulti) ;
typedef bool( __stdcall * SetCallBackW )(const wchar_t * PinName , DataCallbackW oc , LPVOID lParam , bool bMulti) ;


bool ConnectGraph::SetDataCallBackEx(
  const char * PinName , DataCallbackA oc, LPVOID lParam , bool bMulti )
{
  FXAutolock alock( m_Lock ) ;

  DataCallBacks * pDataCB = (DataCallBacks*) m_DataCB ;
  for ( int i = 0 ; i < pDataCB->GetCount() ; i++ )
  {
    if ( pDataCB->ElementAt(i).m_PinName == PinName )
    {
      ((IGraphbuilder*) m_pBuilder)->SetOutputCallback(
        pDataCB->ElementAt(i).m_PinName , NULL, NULL );
      pDataCB->RemoveAt( i ) ;
      i-- ;
    }
  }
  TextCallBacks * pCB = (TextCallBacks*) m_CB ;
  for ( int i = 0 ; i < pCB->GetCount() ; i++ )
  {
    if ( pCB->ElementAt(i).m_PinName == PinName )
    {
      ((IGraphbuilder*) m_pBuilder)->SetOutputCallback(
        pCB->ElementAt(i).m_PinName , NULL, NULL );
      pCB->RemoveAt( i ) ;
      i-- ;
    }
  }

  if ( oc == NULL )
    return true ;


  DataCallBackSet NewCallBack(bMulti) ;
  NewCallBack.m_PinName = PinName ;
  NewCallBack.m_FuncAddrA = oc ;
  NewCallBack.m_pParam = lParam ;
  if (!((IGraphbuilder*) m_pBuilder)->SetOutputCallback( 
    (LPCTSTR) NewCallBack.m_PinName , DataCallBackFuncEx, this) )
  {
    return false;
  }
  pDataCB->Add( NewCallBack ) ;
  return true ;
}

bool ConnectGraph::SetDataCallBackEx(
  const  wchar_t * PinName , DataCallbackW oc, LPVOID lParam , bool bMulti )
{
  const char * asciiPinName = w2a( PinName );

  FXAutolock alock( m_Lock ) ;

  DataCallBacks * pDataCB = (DataCallBacks*) m_DataCB ;
  for ( int i = 0 ; i < pDataCB->GetCount() ; i++ )
  {
    if ( pDataCB->ElementAt(i).m_PinName == asciiPinName )
    {
      ((IGraphbuilder*) m_pBuilder)->SetOutputCallback(
        pDataCB->ElementAt(i).m_PinName , NULL, NULL );
      pDataCB->RemoveAt( i ) ;
      i-- ;
    }
  }
  TextCallBacks * pCB = (TextCallBacks*) m_CB ;
  for ( int i = 0 ; i < pCB->GetCount() ; i++ )
  {
    if ( pCB->ElementAt(i).m_PinName == asciiPinName )
    {
      ((IGraphbuilder*) m_pBuilder)->SetOutputCallback(
        pCB->ElementAt(i).m_PinName , NULL, NULL );
      pCB->RemoveAt( i ) ;
      i-- ;
    }
  }

  if ( oc == NULL )
  {
    delete[] asciiPinName ;
    return true ;
  }
  DataCallBackSet NewCallBack(bMulti) ;
  NewCallBack.m_PinName = asciiPinName ;
  NewCallBack.m_FuncAddrW = oc ;
  NewCallBack.m_pParam = lParam ;
  delete[] asciiPinName ;
  if (!((IGraphbuilder*) m_pBuilder)->SetOutputCallback( 
    (LPCTSTR) NewCallBack.m_PinName , DataCallBackFuncEx, this) )
  {
    return false;
  }
  pDataCB->Add( NewCallBack ) ;
  return true ;
}

bool ConnectGraph::ConnectRendererToWindow(
  CWnd * pWnd , const char * WindowCaption , const char * RendererName  )
{
  if ( m_pBuilder )
  {
    CRenderGadget * pRenderer = 
      (CRenderGadget*) ((IGraphbuilder*) m_pBuilder)->GetGadget( RendererName ) ;
    if ( pRenderer )
    {
      WinToRendArray * pCorresp = 
        (WinToRendArray *)m_pCorrespondence ;
      for ( int i = 0 ; i < pCorresp->GetCount() ; i++ )
      {
        WindowToRendererCorresponding OneCorresp = pCorresp->GetAt(i) ;
        bool bTheSameWindow = ( OneCorresp.m_pWindow == pWnd ) ;
        bool bTheSameRenderer = ( OneCorresp.m_pRenderer == pRenderer ) ;
        if ( bTheSameRenderer || bTheSameWindow )
        {
          pCorresp->RemoveAt( i ) ;
          i-- ;
        }
        if ( bTheSameWindow && !bTheSameRenderer )
        {
          ((IGraphbuilder*) m_pBuilder)->ConnectRendererAndMonitor( 
            CString(OneCorresp.m_RenderName) , NULL , NULL , 
            OneCorresp.m_pRenderer ) ;
        }
      }
      BOOL bRes = ((IGraphbuilder*) m_pBuilder)->ConnectRendererAndMonitor( 
        CString(RendererName) , pWnd , WindowCaption , pRenderer ) ;
      if ( bRes )
      {
        WindowToRendererCorresponding NewCorresponding( 
          pWnd , pRenderer , FXString(RendererName) , FXString(WindowCaption) ) ;
        pCorresp->Add( NewCorresponding  );
      }
      return (bRes != FALSE)  ;
    }
  }
  return false ;
}

bool ConnectGraph::ConnectRendererToWindow(
  CWnd * pWnd , const  wchar_t * WindowCaption , const  wchar_t * RendererName  )
{
  if ( m_pBuilder )
  {
    const char * asciiWCaption = w2a( WindowCaption  );
    const char * asciiRendName = w2a( RendererName ) ;
    bool bRes = ConnectRendererToWindow( pWnd , asciiWCaption , asciiRendName ) ;
    delete[] asciiRendName ;
    delete[] asciiWCaption ;
  }
  return false ;
}

void ConnectGraph::ReconnectRenderersToWindows()
{
  WinToRendArray * pCorresp = 
    (WinToRendArray *)m_pCorrespondence ;
  for ( int i = 0 ; i < pCorresp->GetCount() ; i++ )
  {
    WindowToRendererCorresponding OneCorresp = pCorresp->GetAt(i) ;
    ((IGraphbuilder*) m_pBuilder)->ConnectRendererAndMonitor( 
      CString(OneCorresp.m_RenderName) , OneCorresp.m_pWindow , OneCorresp.m_WindowCaption , 
      OneCorresp.m_pRenderer ) ;  
  }
  ((IGraphbuilder*) m_pBuilder)->Start() ;
}

bool ConnectGraph::SendFrame( const char * PinName , // Send text frame to pin
  CDataFrame * pDataFrame , const char * pLabel )
{
  if ( m_pBuilder )
  {
      if ( pLabel )
        pDataFrame->SetLabel( pLabel ) ;
      pDataFrame->SetTime( GetHRTickCount() ) ;
      if ( ( ( IGraphbuilder* ) m_pBuilder )->SendDataFrame( pDataFrame , PinName ) )
        return true ;
  }
  pDataFrame->Release() ;
  return false ;
}


bool ConnectGraph::SendText(
  const char * PinName , const char * Data , const char * pLabel )
{
  if ( m_pBuilder )
  {
    CTextFrame * tf = CTextFrame::Create() ;
    if ( tf )
    {
      tf->GetString() = Data ;
      if ( pLabel )
        tf->SetLabel( pLabel ) ;
      tf->ChangeId( 0 ) ;
      tf->SetTime( GetHRTickCount() ) ;
      if ( ( ( IGraphbuilder* ) m_pBuilder )->SendDataFrame( tf , PinName ) )
        return true ;

      tf->Release( tf ) ;
    }
  }
  return false ;
}
int ConnectGraph::SendText(
  const char * PinNames[] , const char * Data , const char * pLabel )
{
  if ( m_pBuilder )
  {
    CTextFrame * tf = CTextFrame::Create() ;
    if ( tf )
    {
      tf->GetString() = Data ;
      if ( pLabel )
        tf->SetLabel( pLabel ) ;
      tf->ChangeId( 0 ) ;
      tf->SetTime( GetHRTickCount() ) ;
      int iPinCnt = 0 ;
      while ( PinNames[ iPinCnt++ ] ) ; // how many target pins
      if ( iPinCnt > 1 )
        tf->AddRef( iPinCnt - 2 ) ; // minus last NULL ptr and minus 1 of first target
      int iSentCnt = 0 ;
      iPinCnt = 0 ;
      do
      {
        if ( ( ( IGraphbuilder* ) m_pBuilder )->SendDataFrame( tf , PinNames[ iPinCnt++ ] ) )
          iSentCnt++ ;
      } while ( PinNames[ iPinCnt ] );
      while ( iSentCnt < iPinCnt-- )
        tf->Release( tf ) ;
      return iSentCnt ;
    }
  }
  return 0 ;
}

bool ConnectGraph::SendText( const wchar_t * PinName ,
  const wchar_t * Data , const wchar_t * pLabel )
{
  bool bRes = false ;
  if ( m_pBuilder && PinName && Data )
  {
    const char * asciiPinName = w2a( PinName ) ;
    const char * asciiData = w2a( Data ) ;
    if ( !pLabel )
      bRes = SendText( asciiPinName , asciiData ) ;
    else
    {
      const char * asciiLabel = w2a( pLabel ) ;
      bRes = SendText( asciiPinName , asciiData , asciiLabel ) ;
      delete[] asciiLabel ;
    }
    delete[] asciiData ;
    delete[] asciiPinName ;
  }
  return bRes ;
}

bool ConnectGraph::SendBoolean( const char * PinName , 
                               bool bValue , const char * pLabel )
{
  if ( m_pBuilder )
  {
    CBooleanFrame * bf = CBooleanFrame::Create( bValue ) ;
    if ( bf )
    {
      if ( pLabel )
        bf->SetLabel( pLabel ) ;
      bf->ChangeId(0) ;
      bf->SetTime( GetHRTickCount() ) ;
      if ( ((IGraphbuilder*)m_pBuilder)->SendDataFrame( bf , PinName ) )
        return true ;

      bf->Release(bf) ;
    }
  }
  return false ;
}
bool ConnectGraph::SendBoolean( const wchar_t * PinName , 
                               bool bValue , const wchar_t * pLabel )
{
  bool bRes = false ;
  if ( m_pBuilder && PinName )
  {
    const char * asciiPinName = w2a( PinName ) ;
    if ( !pLabel )
      bRes = SendBoolean( asciiPinName , bValue ) ;
    else
    {
      const char * asciiLabel = w2a( pLabel ) ;
      bRes = SendBoolean( asciiPinName , bValue , asciiLabel ) ;
      delete[] asciiLabel ;
    }
    delete[] asciiPinName ;
  }
  return bRes ;
}
bool ConnectGraph::SendQuantity( const char * PinName ,
  double dValue , const char * pLabel )
{
  if ( m_pBuilder )
  {
    CQuantityFrame * qf = CQuantityFrame::Create( dValue ) ;
    if ( qf )
    {
      if ( pLabel )
        qf->SetLabel( pLabel ) ;
      qf->ChangeId( 0 ) ;
      qf->SetTime( GetHRTickCount() ) ;
      if ( ( ( IGraphbuilder* ) m_pBuilder )->SendDataFrame( qf , PinName ) )
        return true ;

      qf->Release( qf ) ;
    }
  }
  return false ;
}
bool ConnectGraph::SendQuantity( const wchar_t * PinName ,
  double dValue , const wchar_t * pLabel )
{
  bool bRes = false ;
  if ( m_pBuilder && PinName )
  {
    const char * asciiPinName = w2a( PinName ) ;
    if ( !pLabel )
      bRes = SendQuantity( asciiPinName , dValue ) ;
    else
    {
      const char * asciiLabel = w2a( pLabel ) ;
      bRes = SendQuantity( asciiPinName , dValue , asciiLabel ) ;
      delete[] asciiLabel ;
    }
    delete[] asciiPinName ;
  }
  return bRes ;
}
bool ConnectGraph::SendQuantity( const char * PinName ,
  int iValue , const char * pLabel )
{
  if ( m_pBuilder )
  {
    CQuantityFrame * qf = CQuantityFrame::Create( iValue ) ;
    if ( qf )
    {
      if ( pLabel )
        qf->SetLabel( pLabel ) ;
      qf->ChangeId( 0 ) ;
      qf->SetTime( GetHRTickCount() ) ;
      if ( ( ( IGraphbuilder* ) m_pBuilder )->SendDataFrame( qf , PinName ) )
        return true ;

      qf->Release( qf ) ;
    }
  }
  return false ;
}
bool ConnectGraph::SendQuantity( const wchar_t * PinName ,
  int iValue , const wchar_t * pLabel )
{
  bool bRes = false ;
  if ( m_pBuilder && PinName )
  {
    const char * asciiPinName = w2a( PinName ) ;
    if ( !pLabel )
      bRes = SendQuantity( asciiPinName , iValue ) ;
    else
    {
      const char * asciiLabel = w2a( pLabel ) ;
      bRes = SendQuantity( asciiPinName , iValue , asciiLabel ) ;
      delete[] asciiLabel ;
    }
    delete[] asciiPinName ;
  }
  return bRes ;
}

bool ConnectGraph::SendFigure( const char * PinName , 
                              CDPointArray& Figure , const char * pLabel )
{
  if ( m_pBuilder )
  {
    CFigureFrame * ff = CFigureFrame::Create() ;
    if ( ff )
    {
      if ( pLabel )
        ff->SetLabel( pLabel ) ;
      ff->ChangeId(0) ;
      ff->SetTime( GetHRTickCount() ) ;
      ff->SetSize( Figure.GetCount() ) ;
      memcpy( ff->GetData() , Figure.GetData() , sizeof(CDPoint) * Figure.GetCount() ) ;
      if ( ((IGraphbuilder*)m_pBuilder)->SendDataFrame( ff , PinName ) )
        return true ;

      ff->Release(ff) ;
    }
  }
  return false ;
}
bool ConnectGraph::SendFigure( const wchar_t * PinName , 
                              CDPointArray& Figure , const wchar_t * pLabel )
{
  bool bRes = false ;
  if ( m_pBuilder && PinName )
  {
    const char * asciiPinName = w2a( PinName ) ;
    if ( !pLabel )
      bRes = SendFigure( asciiPinName , Figure ) ;
    else
    {
      const char * asciiLabel = w2a( pLabel ) ;
      bRes = SendFigure( asciiPinName , Figure , asciiLabel ) ;
      delete[] asciiLabel ;
    }
    delete[] asciiPinName ;
  }
  return bRes ;
}

bool ConnectGraph::SendImage( const char * PinName , void * pImage ,
                             CSize Size , int iBits , const char * pLabel )
{
  if ( (iBits != 8)   &&  (iBits != 16) )
    return false ;
  if ( m_pBuilder )
  {
    pTVFrame frame = pImage ? (pTVFrame)malloc(sizeof(TVFrame)) : NULL ;
    if ( frame )
    {
      ULONG ulSize = Size.cx * Size.cy * (iBits/8) ;
      frame->lpData=NULL;
      frame->lpBMIH = (BITMAPINFOHEADER*)malloc( sizeof(BITMAPINFOHEADER) + ulSize ) ;
      if ( frame->lpBMIH )
      {
        frame->lpBMIH->biSize = sizeof(BITMAPINFOHEADER) ;
        frame->lpBMIH->biWidth = Size.cx ;
        frame->lpBMIH->biHeight = Size.cy ;
        frame->lpBMIH->biSizeImage = ulSize ;
        frame->lpBMIH->biCompression = (iBits == 8)? BI_Y8 : BI_Y16 ;
        frame->lpBMIH->biPlanes = 1 ;
        frame->lpBMIH->biBitCount = iBits ;
        frame->lpBMIH->biXPelsPerMeter = 0 ;
        frame->lpBMIH->biYPelsPerMeter = 0 ;
        frame->lpBMIH->biClrUsed = 0 ;
        frame->lpBMIH->biClrImportant = 0 ;
        TRY
        {
          memcpy( frame->lpBMIH + 1 , pImage , ulSize ) ;
        }
        CATCH(CException, pEx)
        {
          free( frame->lpBMIH ) ;
          free( frame ) ;
          return false ;
        }
        END_CATCH
      }
    }
    CVideoFrame * vf = CVideoFrame::Create( frame ) ;
    if ( vf )
    {
      if ( pImage )
      {
        if ( pLabel )
          vf->SetLabel( pLabel ) ;
      }
      else
        Tvdb400_SetEOS( vf ) ;
      vf->ChangeId(0) ;
      vf->SetTime( GetHRTickCount() ) ;
      if ( ((IGraphbuilder*)m_pBuilder)->SendDataFrame( vf , PinName ) )
        return true;

      vf->Release(vf) ;
    }
    else if ( frame )
      free( frame ) ;
  }
  return false ;
}

bool ConnectGraph::SendImage( const wchar_t * PinName , void * pImage ,
                             CSize Size , int iBits , const wchar_t * pLabel )
{
  bool bRes = false ;
  if ( m_pBuilder && PinName )
  {
    const char * asciiPinName = w2a( PinName ) ;
    if ( !pLabel )
      bRes = SendImage( asciiPinName , pImage , Size , iBits ) ;
    else
    {
      const char * asciiLabel = w2a( pLabel ) ;
      bRes = SendImage( asciiPinName , pImage , Size , iBits , asciiLabel ) ;
      delete[] asciiLabel ;
    }
    delete[] asciiPinName ;
  }
  return bRes ;
}

void ConnectGraph::LockCBs()
{
  m_Lock.Lock() ;
}
void ConnectGraph::UnlockCBs()
{
  m_Lock.Unlock() ;
}

void ConnectGraph::a2wMessage( 
  int msgLevel, const char * src, int msgId, const char * msgText ) 
{
  WCHAR wsSrc[300] , wsMsgText[1000] ;
  if ( m_pLogger 
    && a2w( src , wsSrc , 300 ) 
    && a2w( msgText , wsMsgText , 1000 ) )
  {
    m_pLogger( msgLevel , wsSrc , msgId , wsMsgText ) ;
  }

}
void ConnectGraph::ViewGraph( CWnd * pHost ) 
{
  if (m_pBuilder)
  {
    m_bRunBeforeInspect = ( ((IGraphbuilder*)m_pBuilder)->IsRuning() != FALSE) ;
    StartInspect( ((IGraphbuilder*)m_pBuilder) , pHost );
  }
}

void ConnectGraph::ViewSetup( CWnd * pHost ) 
{
  if (m_pBuilder)
  {
    Tvdb400_RunSetupDialog( (IGraphbuilder*) m_pBuilder);
  }
}

bool ConnectGraph::SetProperty( 
  const char * GadgetName ,  const char * PropertyName , 
  const char * PropertyValue )
{
  if (m_pBuilder)
  {
    CGadget * pGadget = 
      ((IGraphbuilder*)m_pBuilder)->GetGadget( GadgetName ) ;
    if ( pGadget )
    {
      FXPropertyKit pk ;
      if ( pk.WriteString( PropertyName , PropertyValue ) )
      {
        bool Invalidate = false ;
        if ( pGadget->ScanProperties( pk , Invalidate ) )
          return true ;
      }
    }
  }
  return false ;
}

bool ConnectGraph::SetProperty( 
  const wchar_t * GadgetName ,  const wchar_t * PropertyName , 
  const wchar_t * PropertyValue )
{
  bool bRes = false ;
  if (m_pBuilder && GadgetName && PropertyName && PropertyValue )
  {
    const char * asciiGName = w2a( GadgetName ) ;
    const char * asciiPropName = w2a( PropertyName ) ;
    const char * asciiPropValue = w2a( PropertyValue ) ;
    if ( asciiGName && asciiPropName && asciiPropValue )
      bRes = SetProperty( GadgetName , PropertyName , PropertyValue ) ;
    if ( asciiPropValue )
      delete[] asciiPropName ;
    if ( asciiPropValue )
      delete[] asciiPropValue ;
    if ( asciiGName)
      delete[] asciiGName ;
  }
  return bRes ;
}

bool ConnectGraph::SetProperty( 
  const char * GadgetName ,  const char * PropertyName , 
  int PropertyValue )
{
  if (m_pBuilder)
  {
    FXString PropVal ;
    PropVal.Format( "%d" , PropertyValue ) ;
    return SetProperty( GadgetName , PropertyName , PropVal ) ;
  }
  return false ;
}

bool ConnectGraph::SetProperty( 
  const char * GadgetName ,  const char * PropertyName , 
  double dPropertyValue )
{
  if (m_pBuilder)
  {
    FXString PropVal ;
    PropVal.Format( "%g" , dPropertyValue ) ;
    return SetProperty( GadgetName , PropertyName , PropVal ) ;
  }
  return false ;
}

bool ConnectGraph::SetProperty( 
  const wchar_t * GadgetName ,  const wchar_t * PropertyName , 
  int iPropertyValue )
{
  bool bRes = false ;
  if (m_pBuilder && GadgetName && PropertyName)
  {
    const char * asciiGName = w2a( GadgetName ) ;
    if ( asciiGName )
    {
      const char * asciiPropName = w2a( PropertyName ) ;
      bRes = SetProperty( asciiGName , asciiPropName , iPropertyValue ) ;
      delete[] asciiPropName ;
    }
    delete[] asciiGName ;
  }
  return bRes ;
}

bool ConnectGraph::SetProperty( 
  const wchar_t * GadgetName ,  const wchar_t * PropertyName , 
  double dPropertyValue )
{
  bool bRes = false ;
  if (m_pBuilder && GadgetName && PropertyName)
  {
    const char * asciiGName = w2a( GadgetName ) ;
    if ( asciiGName )
    {
      const char * asciiPropName = w2a( PropertyName ) ;
      bRes = SetProperty( asciiGName , asciiPropName , dPropertyValue ) ;
      delete[] asciiPropName ;
    }
    delete[] asciiGName ;
  }
  return bRes ;
}

bool ConnectGraph::ScanProperties(
  const char * GadgetName , const char * pProperties )
{
  if ( m_pBuilder )
  {
    CGadget * pGadget =
      ((IGraphbuilder*) m_pBuilder)->GetGadget( GadgetName ) ;
    if ( pGadget )
    {
      bool Invalidate = false ;
      if ( pGadget->ScanProperties( pProperties , Invalidate ) )
        return true ;
    }
  }
  return false ;
}

bool ConnectGraph::PrintProperties( const char * GadgetName ,
  char * pBuffer , // if zero, ulPropValLen will return necessary length
  ULONG& ulPropValLen ) // In-Out: In - buffer length, Out - real string length
{
  if ( m_pBuilder && GadgetName )
  {
    CGadget * pGadget =
      ((IGraphbuilder*) m_pBuilder)->GetGadget( GadgetName ) ;
    if ( pGadget )
    {
      FXPropertyKit pk ;
      if ( pGadget->PrintProperties( pk ) )
      {
        size_t ulLength = pk.GetLength() ;
        if( pBuffer && (ulLength < ulPropValLen) )
        {
          strcpy_s( pBuffer , ulPropValLen , (LPCTSTR) pk ) ;
          ulPropValLen = (ULONG)ulLength ;
          return true ;
        }
        ulPropValLen = (ULONG)ulLength ; // no copy, because no buffer or small buffer length
      }
    }
    else
      ulPropValLen = 0 ;
  }
  return false ;
}

bool ConnectGraph::PrintProperties(
  const wchar_t *  GadgetName ,
  wchar_t * pBuffer , ULONG& ulPropValLen ) // In-Out: In - buffer length, Out - real string length
{
  if ( m_pBuilder && GadgetName )
  {
    const char * pGName = w2a( GadgetName ) ;
    if ( pGName )
    {
      CGadget * pGadget =
        ((IGraphbuilder*) m_pBuilder)->GetGadget( pGName ) ;
      delete[] pGName ;
      if ( pGadget )
      {
        FXPropertyKit pk ;
        if ( pGadget->PrintProperties( pk ) )
        {
          size_t ulLength = pk.GetLength() ;
          if ( pBuffer && (ulLength < ulPropValLen) )
          {
            a2w( (LPCTSTR) pk , pBuffer , (int&) ulPropValLen ) ;
            return true ;
          }
          ulPropValLen = (ULONG)ulLength ;
        }
      }
      else
        ulPropValLen = 0 ; // no gadget
    }
    else
      ulPropValLen = 0xffffffff ; // problem with conversion to ASCII
  }
  return false ;
}


bool ConnectGraph::GetProperty(
  const char * GadgetName , const char * PropertyName , 
  char * PropertyValue , ULONG ulPropValLen )
{
  if (m_pBuilder)
  {
    CGadget * pGadget = 
      ((IGraphbuilder*)m_pBuilder)->GetGadget( GadgetName ) ;
    if ( pGadget )
    {
      FXPropertyKit pk ;
      if ( pGadget->PrintProperties( pk ))
      {
        FXString Tmp ;
        if ( pk.GetString( PropertyName , Tmp ) )
        {
          if ( (ULONG) Tmp.GetLength() < ulPropValLen - 1 )
          {
            strcpy_s( PropertyValue , ulPropValLen , Tmp.GetString() ) ;
            return true ;
          }
        }
      }
    }
  }
  return false ;
}

bool ConnectGraph::GetProperty(
  const wchar_t *  GadgetName ,  const wchar_t * PropertyName ,  
  wchar_t * PropertyValue , ULONG ulValLen )
{
  bool bRes = false ;
  if (m_pBuilder && GadgetName && PropertyName && PropertyValue )
  {
    const char * pGName = w2a( GadgetName ) ;
    if ( pGName )
    {
      CGadget * pGadget = 
        ((IGraphbuilder*)m_pBuilder)->GetGadget( pGName ) ;
      if ( pGadget )
      {
        FXPropertyKit pk ;
        if ( pGadget->PrintProperties( pk ))
        {
          const char * pPName = w2a( PropertyName ) ;
          if ( pPName )
          {
            FXString PropVal ;
            if ( pk.GetString( pPName , PropVal ) )
            {
              if ( (ULONG)PropVal.GetLength() < ulValLen - 1 )
              {
                if ( a2w( PropVal , PropertyValue , (int) PropVal.GetLength() ) )
                {
                  PropertyValue[PropVal.GetLength()] = 0 ;
                  bRes = true ;
                }
              }
            }
            delete[] pPName ;
          }
        }
      }
      delete[] pGName ;
    }
  }
  return bRes ;
}
bool ConnectGraph::GetProperty(
  const char * GadgetName , const char * PropertyName , 
  int& iPropVal )
{
  if (m_pBuilder)
  {
    CGadget * pGadget = 
      ((IGraphbuilder*)m_pBuilder)->GetGadget( GadgetName ) ;
    if ( pGadget )
    {
      FXPropertyKit pk ;
      if ( pGadget->PrintProperties( pk ))
        return pk.GetInt( PropertyName , iPropVal ) ;
    }
  }
  return false ;
}

bool ConnectGraph::GetProperty(
  const wchar_t *  GadgetName ,  const wchar_t * PropertyName ,  
  int& iPropVal )
{
  bool bRes = false ;
  if (m_pBuilder && GadgetName && PropertyName  )
  {
    const char * pGName = w2a( GadgetName ) ;
    if ( pGName )
    {
      const char * pPName = w2a( PropertyName ) ;
      bRes = GetProperty( pGName , pPName , iPropVal ) ;
      delete[] pPName ;
    }
    delete[] pGName ;
  }
  return bRes ;
}

bool ConnectGraph::GetProperty(
  const char * GadgetName , const char * PropertyName , 
  double& dPropVal )
{
  if (m_pBuilder)
  {
    CGadget * pGadget = 
      ((IGraphbuilder*)m_pBuilder)->GetGadget( GadgetName ) ;
    if ( pGadget )
    {
      FXPropertyKit pk ;
      if ( pGadget->PrintProperties( pk ))
        return pk.GetDouble( PropertyName , dPropVal ) ;
    }
  }
  return false ;
}

bool ConnectGraph::GetProperty(
  const wchar_t *  GadgetName ,  const wchar_t * PropertyName ,  
  double& dPropVal )
{
  bool bRes = false ;
  if (m_pBuilder && GadgetName && PropertyName  )
  {
    const char * pGName = w2a( GadgetName ) ;
    if ( pGName )
    {
      const char * pPName = w2a( PropertyName ) ;
      bRes = GetProperty( pGName , pPName , dPropVal ) ;
      delete[] pPName ;
    }
    delete[] pGName ;
  }
  return bRes ;
}

bool ConnectGraph::SetWorkingMode(const char * GadgetName , int iWorkingMode )
{
  if (m_pBuilder)
  {
    CGadget * pGadget = 
      ((IGraphbuilder*)m_pBuilder)->GetGadget( GadgetName ) ;
    if ( pGadget )
    {
      pGadget->SetMode( iWorkingMode ) ;
      return true ;
    }
  }
  return false ;
}

bool ConnectGraph::SetWorkingMode( const wchar_t * GadgetName , int iWorkingMode )
{
  bool bRes = false ;
  if (m_pBuilder && GadgetName )
  {
    const char * pGName = w2a( GadgetName ) ;
    if ( pGName )
    {
      bRes = SetWorkingMode( pGName , iWorkingMode ) ;
      delete[] pGName ;
    }
  }
  return bRes ;
}

bool ConnectGraph::GetWorkingMode(const char * GadgetName , int& iWorkingMode )
{
  if (m_pBuilder)
  {
    CGadget * pGadget = 
      ((IGraphbuilder*)m_pBuilder)->GetGadget( GadgetName ) ;
    if ( pGadget )
    {
      pGadget->GetMode( iWorkingMode ) ;
      return true ;
    }
  }
  return false ;
}

bool ConnectGraph::GetWorkingMode( const wchar_t * GadgetName , int& iWorkingMode )
{
  bool bRes = false ;
  if (m_pBuilder && GadgetName )
  {
    const char * pGName = w2a( GadgetName ) ;
    if ( pGName )
    {
      bRes = GetWorkingMode( pGName , iWorkingMode ) ;
      delete[] pGName ;
    }
  }
  return bRes ;
}

map<string, list<const CGadgetInfo*> > ConnectGraph::getGadgetsByClassNames()
{
  CStringArray src, dst;

  if(m_pBuilder)
    ((IGraphbuilder*) m_pBuilder)->EnumGadgets(src, dst);

  dst.Append(src);
  CGabgetsDBFactory dbFactory(dst);

  m_gdgtsInfoDb.clean();

  m_gdgtsInfoDb = dbFactory.fillDB(m_gdgtsInfoDb, (IGraphbuilder*)m_pBuilder);

  return m_gdgtsInfoDb.getGadgetsByClassNames();
}

void ConnectGraph::_setRenderRect( const RECT& newRC, LPCTSTR renderName, void *pBuilder )
{
  CRenderGadget* pGadget = (CRenderGadget*)((IGraphbuilder*)pBuilder)->GetGadget(renderName);

  if(pGadget)
  {
    CWnd* wnd=pGadget->GetRenderWnd();

    if(wnd)
      wnd->MoveWindow(&newRC);
  }
}

bool ConnectGraph::IsRunning() const
{//!!! .h file is a contract only so implementation should be here!!!
  return m_pBuilder && ((IGraphbuilder*)m_pBuilder)->IsRuning();
}

bool ConnectGraph::IsPaused() const
{//!!! .h file is a contract only so implementation should be here!!!
  return m_pBuilder && ((IGraphbuilder*)m_pBuilder)->IsPaused();
}


//********************************************************************
//-Interface functions for connection to C# and .Net

void * GConnectGraph( const char * pGraphName )
{
  if ( pGraphName != NULL )
  {
    wchar_t * pWideCharName = ( wchar_t* ) pGraphName ;
    ConnectGraph * pGraph = new ConnectGraph( pWideCharName );
    return pGraph;
  }

  return NULL ;
}

bool GIsInitialized(void * GPointer)
{
  return (((ConnectGraph * )GPointer)->IsInitialized());
}

void * GRunSetupDialog(void * GPointer, void * pGadgetName  , CPoint * pPoint )
{
  if ( GPointer )
    ((ConnectGraph * )GPointer)->RunSetupDialog( (const wchar_t*)pGadgetName , pPoint );
  return GPointer ;
}

void * GViewSetup(void * GPointer, CWnd * PHost )
{
  if ( GPointer )
    ((ConnectGraph *)GPointer)->ViewGraph( PHost );
  return GPointer ;
}

bool GSetTextCallBack( void * GPointer, void * PinName ,
                      TextCallbackW oc, LPVOID lParam , bool bMulti )
{
  if ( GPointer )
  {
    return ((ConnectGraph *)GPointer)->SetTextCallBack( 
      (const wchar_t*)PinName , oc, lParam , bMulti );
  }
  return false ;
}

bool GSendText( void * GPointer, 
               void * PinName ,	void * Data , const char * pLabel)
{
  if ( GPointer )
  {
    return ((ConnectGraph *)GPointer)->SendText( 
      (const wchar_t*)PinName ,	(const wchar_t*)Data , (const wchar_t*) pLabel );
  }
  return false ;

}

bool GSetProperty( void * GPointer, 
                  void* GadgetName , void * PropertyName , 	void * PropertyValue )
{
  if ( GPointer )
  {
    return ((ConnectGraph *)GPointer)->SetProperty( 
      (const wchar_t*)GadgetName , (const wchar_t*)PropertyName , 
      (const wchar_t*)PropertyValue);
  }
  return false ;
}

bool GGetProperty( void * GPointer, 
                  void * GadgetName ,void * PropertyName , 
                  void * PropertyValue , ULONG ulPropValLen )
{
  if ( GPointer )
  {
    return ((ConnectGraph *)GPointer)->GetProperty( 
      (const wchar_t*)GadgetName , (const wchar_t*)PropertyName , 
      (wchar_t*)PropertyValue , ulPropValLen );
  }
  return false ;
}

//********************************************************************
//END-Interface functions for connection to C# and .Net



vector<CGadgetInfo> CGadgetsDB::_listPtrs2VectVals( const list<const CGadgetInfo*>& gdgtsPtrs ) const
{
  vector<CGadgetInfo> gdgtsVals(gdgtsPtrs.size());
  size_t indGadgets=0;
  list <const CGadgetInfo*>::const_iterator it,itEnd=gdgtsPtrs.end();
  for(it=gdgtsPtrs.begin(); it != itEnd; it++)
    gdgtsVals[indGadgets++]=**it;
  return gdgtsVals;
}

bool CGadgetsDB::addGadget( const CGadgetInfo& gdgt )
{
  bool res = false;
  pair<set<CGadgetInfo>::iterator, bool> pr = m_gdgts.insert(gdgt); //returns pair of iterator and insertion result;

  if(pr.second) //if insertion to the set was successful
  {
    _addToGadgetsByType(*pr.first);
    _addToGadgetsByClss(*pr.first);
    res = true;
  }
  return res;
}

bool CGadgetsDB::removeGadget( const string& name )
{
  bool res = false;
  set<CGadgetInfo>::iterator it = m_gdgts.find(CGadgetInfo(name));

  if(it != m_gdgts.end())
  {
    _removeFromGadgetsByType(*it);
    _removeFromGadgetsByClss(*it);
    res = true;
  }
  return res;
}

CGadgetsDB& CGabgetsDBFactory::fillDB( CGadgetsDB& db, void *pBuilder )
{
  if(pBuilder)
  {
    set<string>::const_iterator cit, cite = m_gadgetsNames.end();

    for(cit = m_gadgetsNames.begin(); cit != cite; cit++ )
    {
      CGadget* pGadget = ((IGraphbuilder*)pBuilder)->GetGadget(cit->c_str());

      if(pGadget)
      {
        FXString clName = pGadget->IsKindOf(RUNTIME_GADGET(CVirtualGadget)) ? ((CVirtualGadget*)pGadget)->GetClassName() : pGadget->GetRuntimeGadget()->m_lpszClassName;

        FXString uid(cit->c_str());
        size_t type = ((IGraphbuilder*)pBuilder)->KindOf(uid);
        db.addGadget(CGadgetInfo(uid.GetString(), clName.GetString(), type));
      }
    }
  }
  return db;
}
