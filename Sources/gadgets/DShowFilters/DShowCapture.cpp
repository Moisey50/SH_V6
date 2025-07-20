// DShowCapture.cpp : Implementation of the DShowCapture class


#include "StdAfx.h"
#include "DShowCapture.h"
#include <video\shvideo.h>
//#include <imageproc/simpleip.h>
#include <Dvdmedia.h>
#include <video\mediasample2tvframe.h>
#include <Gadgets\TextFrame.h>

#ifdef _DEBUG
#pragma comment(lib,"strmbasd.lib")
#else
#pragma comment(lib,"STRMBASE.lib")
#endif

#ifndef SAFE_RELEASE
#define SAFE_RELEASE(x) { if (x) { x->Release(); x = NULL; } }
#endif

#ifndef SAFE_DELETE
#define SAFE_DELETE(x) { if (x) delete x; x = NULL; }
#endif

#ifdef THIS_MODULENAME
#undef THIS_MODULENAME
#endif

#define THIS_MODULENAME "DSHOW_CAPTURE_GADGET"

class CNtfWnd: public CWnd
{
protected:
  DShowCapture* m_Server;
public:
  virtual BOOL Create(DShowCapture* Server);
  //{{AFX_MSG(CNtfWnd)
  afx_msg LRESULT OnFGNotify(WPARAM wParam, LPARAM lParam);
  afx_msg void OnTimer(UINT_PTR nIDEvent);
  //}}AFX_MSG
  DECLARE_MESSAGE_MAP()
};

BEGIN_MESSAGE_MAP(CNtfWnd, CWnd)
  //{{AFX_MSG_MAP(CNtfWnd)
  ON_WM_TIMER()
  //}}AFX_MSG_MAP
  ON_MESSAGE (UM_FGNOTIFY, OnFGNotify)
END_MESSAGE_MAP()

#define BKGRND (RGB(0,0,0))

BOOL CNtfWnd::Create(DShowCapture* Server)
{
  m_Server=Server;
  RECT rect={4, 1, 6, 7};
  CWnd *MainWnd=CWnd::FindWindow("Progman",NULL);	
  LPCTSTR lpszNtfVClass =  AfxRegisterWndClass(CS_PARENTDC, LoadCursor(NULL, IDC_ARROW), NULL);

  if (CWnd::CreateEx(0,lpszNtfVClass, "Notify window", WS_POPUP, rect, MainWnd, 0, NULL))
    return(TRUE);
  return(FALSE);
}

LRESULT CNtfWnd::OnFGNotify(WPARAM wParam, LPARAM lParam)
{
  return (m_Server->OnFGNotify(wParam,lParam));
}

void CNtfWnd::OnTimer(UINT_PTR nIDEvent) 
{
  CWnd::OnTimer(nIDEvent);
}

IMPLEMENT_RUNTIME_GADGET_EX(DShowCapture, CCaptureGadget, "Files.Capture", TVDB400_PLUGIN_NAME);

DShowCapture::DShowCapture(void):
  m_NotifyWnd(NULL),
  m_state(STATE_CLOSED),
  m_pGraph(NULL),
  m_pSource(NULL),
  m_pControl(NULL),
 // m_pMediaPosition(NULL),
  m_pEvent(NULL),
  m_EventMsg(0),
  m_pSeek(NULL),
  m_pStep(NULL),
  m_VideoRenderer(NULL),
  m_pMediaType(NULL),
  m_OutputFormat(BI_YUV9),
  m_pInput(NULL),
  m_dSpeedFactor( 1.0 )
{
  CoInitialize(NULL);
  DbgInitialise(AfxGetApp()->m_hInstance);

  m_pControlPin = new CDuplexConnector(this, text, text);
  m_pOutput = new COutputConnector(vframe);

  m_NotifyWnd     = new CNtfWnd;
  VERIFY(((CNtfWnd*)m_NotifyWnd)->Create(this));

  Resume();
}

void DShowCapture::ShutDown()
{
  CCaptureGadget::ShutDown();
  TearDownGraph();
  if (m_NotifyWnd) 
  {
    if (m_NotifyWnd->GetSafeHwnd())
      m_NotifyWnd->DestroyWindow();
    delete m_NotifyWnd; m_NotifyWnd=NULL;
  }
  if (m_pInput) delete m_pInput;	m_pInput = NULL;
  if (m_pControlPin) delete m_pControlPin; m_pControlPin = NULL;
  delete m_pOutput;
  m_pOutput = NULL;
  DbgTerminate();
  CoUninitialize();
}

int     DShowCapture::GetInputsCount()
{
  return (m_pInput)?1:0;
}

CInputConnector* DShowCapture::GetInputConnector(int n)
{
  return (!n) ? m_pInput : NULL;
}

int DShowCapture::GetDuplexCount()
{
  return 1;
}

CDuplexConnector* DShowCapture::GetDuplexConnector(int n)
{
  if (!n)
    return m_pControlPin;
  return NULL;
}

bool DShowCapture::ScanSettings(FXString& text)
{
  text.Format("template("
              "EditBox(FileName),"
              "ComboBox(LoopClip(False(false),True(true))),"
              "ComboBox(SoftwareTrigger(False(false),True(true))),"
              "EditBox(SpeedFactor)"
              ")" );
  return true;
}

bool DShowCapture::OpenFile(const FXString& fName)
{
  HRESULT hr = S_OK;
  hr = InitializeGraph();

  if (SUCCEEDED(hr))
  {
    LPWSTR unicode=(LPWSTR)malloc((fName.GetLength()+1)*sizeof(WCHAR));
    MultiByteToWideChar(CP_ACP, 0, fName, -1, unicode, (int)fName.GetLength()+1);
    hr = m_pGraph->AddSourceFilter(unicode, NULL, &m_pSource);
    free(unicode);
  }
  if (SUCCEEDED(hr))
  {
    hr=RenderStream(m_pSource);
  }
  if (SUCCEEDED(hr))
  {
    m_state = STATE_STOPPED;
  }
  return (hr == S_OK);
}

void    DShowCapture::NotifyMediaType(CMediaType *pMediaType)
{
  m_pMediaType = pMediaType;
}

HRESULT DShowCapture::DoRenderSample(IMediaSample *pMediaSample)
{
  double ts=GetHRTickCount();

  pTVFrame result=mediasample2tvframe(m_pMediaType, pMediaSample, m_OutputFormat);
  if (result)
  {
    DoSend(result);
    free(result);
  }
  AddCPUUsage(GetHRTickCount()-ts);
  return NOERROR;
}

HRESULT DShowCapture::InitializeGraph()
{
  HRESULT hr = S_OK;

  TearDownGraph();

  hr = CoCreateInstance(
    CLSID_FilterGraph, 
    NULL, 
    CLSCTX_INPROC_SERVER,
    IID_IGraphBuilder,
    (void**)&m_pGraph
    );

  if (SUCCEEDED(hr))
  {
    hr = m_pGraph->QueryInterface(IID_IMediaControl, (void**)&m_pControl);
  }
  if (SUCCEEDED(hr))
  {
    hr = m_pGraph->QueryInterface(IID_IMediaEventEx, (void**)&m_pEvent);
  }
  if (SUCCEEDED(hr))
  {
    m_EventMsg=UM_FGNOTIFY;
    hr = m_pEvent->SetNotifyWindow((OAHWND)m_NotifyWnd->m_hWnd, m_EventMsg, NULL);
  }
  if (SUCCEEDED(hr))
  {
    hr = m_pGraph->QueryInterface(IID_IMediaSeeking, (void**)&m_pSeek);
  }
  if (SUCCEEDED(hr))
  {
    hr = m_pGraph->QueryInterface(IID_IVideoFrameStep, (void**)&m_pStep);
  }
//   if ( SUCCEEDED( hr ) )
//   {
//     hr = m_pGraph->QueryInterface( IID_IMediaPosition , (void**) &m_pMediaPosition );
//   }
  return hr;
}

void	DShowCapture::TearDownGraph()
{
  if (m_pEvent)
  {
    m_pEvent->SetNotifyWindow((OAHWND)NULL, NULL, NULL);
  }

  SAFE_RELEASE(m_pSource);
  SAFE_RELEASE(m_pGraph);
  SAFE_RELEASE(m_pControl);
  SAFE_RELEASE(m_pEvent);
  SAFE_RELEASE(m_pSeek);
  SAFE_RELEASE(m_pStep);
  SAFE_DELETE(m_VideoRenderer);
}

HRESULT	DShowCapture::RenderStream(IBaseFilter *pSource)
{
  HRESULT         hr = S_OK;
  IFilterGraph2   *pGraph2 = NULL;
  IEnumPins       *pEnum = NULL;
  IBaseFilter     *pRenderer=NULL;
  BOOL            bRenderedAnyPin = FALSE;

  hr = m_pGraph->QueryInterface(IID_IFilterGraph2, (void**)&pGraph2);

  if (SUCCEEDED(hr))  // create renderer
  {
    m_VideoRenderer = new CVFilter(NULL,&hr,this);
    hr=m_VideoRenderer->QueryInterface(IID_IBaseFilter, (void **) &pRenderer);
  }
  if (SUCCEEDED(hr))
  {
    hr=m_pGraph->AddFilter(pRenderer,NULL);
  }

  if (SUCCEEDED(hr))
  {
    hr = pSource->EnumPins(&pEnum);
  }

  if (SUCCEEDED(hr))
  {
    // Loop through all the pins
    IPin *pPin = NULL;

    while (S_OK == pEnum->Next(1, &pPin, NULL))
    {			
      // Try to render this pin. 
      // It's OK if we fail some pins, if at least one pin renders.
      HRESULT hr2 = pGraph2->RenderEx(pPin, AM_RENDEREX_RENDERTOEXISTINGRENDERERS, NULL);

      pPin->Release();

      if (SUCCEEDED(hr2))
      {
        bRenderedAnyPin = TRUE;
      }
    }
  }

  SAFE_RELEASE(pEnum);
  SAFE_RELEASE(pGraph2);

  if (!bRenderedAnyPin)
  {
    hr = VFW_E_CANNOT_RENDER;
  }
  DWORD dwCaps = 0 ;
  hr = m_pSeek->IsFormatSupported( &TIME_FORMAT_FRAME );
  hr = m_pSeek->SetTimeFormat( &TIME_FORMAT_FRAME );
  hr = m_pSeek->GetCapabilities( &dwCaps );

  return hr;
}

LRESULT DShowCapture::OnFGNotify(WPARAM wParam, LPARAM lParam)
{
  if (!m_pEvent) return E_UNEXPECTED;

  long evCode = 0;
  LONG_PTR param1 = 0, param2 = 0;

  HRESULT hr = S_OK;
  while (SUCCEEDED(m_pEvent->GetEvent(&evCode, &param1, &param2, 0)))
  {
    // Invoke the callback.
    //pCB->OnGraphEvent(evCode, param1, param2);
    switch (evCode)
    {
    case EC_COMPLETE:
      if (!m_pSeek) 
        hr=E_UNEXPECTED;
      else if (m_LoopClip)
        hr=SetPosition(REFERENCE_TIME(0));
      break;
    }
    // Free the event data.
    hr = m_pEvent->FreeEventParams(evCode, param1, param2);
    if (FAILED(hr))
    {
      break;
    }
  }
  return hr;
}

int DShowCapture::DoJob()
{
  CDataFrame* pDataFrame = NULL;
  while (m_pStatus==NULL) Sleep(0);
  ASSERT(m_pStatus!=NULL);
  switch (m_pStatus->GetStatus())
  {
  case CExecutionStatus::STOP:
    if (m_state==STATE_RUNNING) 
    {
      Stop();
      pDataFrame= CDataFrame::Create(transparent);
      Tvdb400_SetEOS(pDataFrame);
      m_FrameCounter = 0;
      break;
    }
    else
    {
      HANDLE pEvents[] = { m_evExit, m_pStatus->GetStartHandle() };
      DWORD cEvents = sizeof(pEvents) / sizeof(HANDLE);
      DWORD retVal=::WaitForMultipleObjects(cEvents, pEvents, FALSE, INFINITE);
      return WR_CONTINUE;
    }
  case CExecutionStatus::PAUSE:
    {
      if (m_state != STATE_PAUSED)
        Pause();
      ResetEvent(m_pStatus->GetStartHandle());
      HANDLE pEvents[] = { m_evExit, m_pStatus->GetStartHandle(),m_pStatus->GetStpFwdHandle() };
      DWORD cEvents = sizeof(pEvents) / sizeof(HANDLE);
      DWORD retVal=::WaitForMultipleObjects(cEvents, pEvents, FALSE, INFINITE);
      TRACE("+++ %d\n",retVal);
      if (retVal==2)
        if (MoveToNextFrame()!=S_OK)
          SENDERR("Can't move to next frame");
      return WR_CONTINUE;
    }
  case CExecutionStatus::RUN:
    {
      if (m_state!=STATE_RUNNING)
        Start();
      HANDLE pEvents[] = { m_evExit, m_pStatus->GetStopHandle(),m_pStatus->GetPauseHandle() };
      DWORD cEvents = sizeof(pEvents) / sizeof(HANDLE);
      DWORD retVal=::WaitForMultipleObjects(cEvents, pEvents, FALSE, INFINITE);
      break;
    }
  case CExecutionStatus::EXIT:
    return WR_EXIT;
  default:
    ASSERT(FALSE);
    return WR_CONTINUE;
  }
  if (pDataFrame) // send EOS
  {
    if ((!m_pOutput) || (!m_pOutput->Put(pDataFrame)))
      pDataFrame->Release();
  }
  return WR_CONTINUE;
}

HRESULT DShowCapture::Start()
{
  HRESULT         hr = S_OK;
  if (!m_pGraph)
    return VFW_E_WRONG_STATE;
  if (m_state != STATE_PAUSED && m_state != STATE_STOPPED)
    return VFW_E_WRONG_STATE;

  ASSERT(m_pGraph); // If state is correct, the graph should exist.

  if (m_pInput)
    hr = m_pControl->Pause();
  else
    hr = m_pControl->Run();

  if (SUCCEEDED(hr))
  {
    m_state = STATE_RUNNING;
  }

  return hr;
}

HRESULT DShowCapture::Stop()
{
  HRESULT         hr = S_OK;
  if (m_state != STATE_RUNNING && m_state != STATE_PAUSED)
  {
    return VFW_E_WRONG_STATE;
  }
  ASSERT(m_pGraph); 
  hr = m_pControl->Stop();
  //Rewind to start
  hr=SetPosition(REFERENCE_TIME(0));
  if (SUCCEEDED(hr))
  {
    m_state = STATE_STOPPED;
  }
  return hr;
}

HRESULT DShowCapture::Pause()
{
  if (m_state != STATE_RUNNING)
    return VFW_E_WRONG_STATE;
  ASSERT(m_pGraph); 
  HRESULT hr=S_OK;
  if (!m_pInput)
    hr = m_pControl->Pause();
  if (SUCCEEDED(hr))
    m_state = STATE_PAUSED;
  return hr;
}

void DShowCapture::DoSend(pTVFrame inFrame)
{
  pTVFrame pFrame = new TVFrame;
  pFrame->lpBMIH=inFrame->lpBMIH;
  pFrame->lpData=inFrame->lpData;
  inFrame->lpBMIH=NULL;
  inFrame->lpData=NULL;

  CString label;
  //w2a(s_wachFriendlyName,label);	
  CVideoFrame* vf=CVideoFrame::Create(pFrame);
  vf->SetLabel(label);
  vf->SetTime(GetGraphTime() * 1.e-3 );
  vf->ChangeId(m_FrameCounter++);
  if ((!m_pOutput) || (!m_pOutput->Put(vf)))
    vf->RELEASE(vf);
  return;
}

HRESULT DShowCapture::GetPosition(REFERENCE_TIME& pos)
{
  if (m_pControl == NULL || m_pSeek == NULL)
  {
    return E_UNEXPECTED;
  }

  REFERENCE_TIME pStop;
  HRESULT hr = S_OK;
  hr = m_pSeek->GetPositions(&pos, &pStop);
  return hr;
}

HRESULT DShowCapture::SetPosition(REFERENCE_TIME pos)
{
  if (m_pControl == NULL || m_pSeek == NULL)
    return E_UNEXPECTED;

  HRESULT hr = S_OK;
  REFERENCE_TIME duration;
  hr=GetDuration(duration);
  if (SUCCEEDED(hr))
  {
    if (pos>duration)
      hr=E_INVALIDARG;
  }
  if (SUCCEEDED(hr))
  {
    hr = m_pSeek->SetPositions(&pos, AM_SEEKING_AbsolutePositioning,
      NULL, AM_SEEKING_NoPositioning);
  }
  if (SUCCEEDED(hr))
  {
    if (m_state == STATE_STOPPED)
    {
      hr = m_pControl->StopWhenReady();
    }
  }

  return hr;
}

static bool bViewDebug = true ;
static int k = 2 ;

HRESULT DShowCapture::MoveToNextFrame()
{
  if (m_pControl == NULL || m_pSeek == NULL)
  {
    return E_UNEXPECTED;
  }
  HRESULT hr = S_OK;
  hr=m_pStep->CanStep(0,m_pSource);
  if (hr==S_OK)
  {
    hr=m_pStep->Step(1,m_pSource);
  }
  else
  {
    LONGLONG pos, duration , readpos ; 
    REFERENCE_TIME rt; 
    hr = m_pSeek->GetCurrentPosition(&pos);
    if (SUCCEEDED(hr))
    {
      rt=AvgTimePerFrame(m_pMediaType);
      hr=GetDuration(duration);
      if ( SUCCEEDED( hr ) )
      {
//        pos += rt * k ;
        pos += k ;
        if ( pos > duration )
        {
          pos = 0;
          hr = m_pSeek->SetPositions( &pos , AM_SEEKING_AbsolutePositioning ,
            NULL , AM_SEEKING_NoPositioning );
          hr = m_pSeek->GetCurrentPosition( &readpos );
          if ( bViewDebug )
          {
            TRACE( "+++ Loop init pos=%I64d, duration=%I64d hr=%d\n" , pos , duration ,hr );
          }
        }
        else
        {
//           rt *= k ;
//           hr = m_pSeek->SetPositions( &rt , AM_SEEKING_RelativePositioning ,
//             NULL , AM_SEEKING_NoPositioning );
          rt = k ;
          hr = m_pSeek->SetPositions( &rt , AM_SEEKING_RelativePositioning ,
            NULL , AM_SEEKING_NoPositioning );
          hr = m_pSeek->GetCurrentPosition( &readpos );
          if ( bViewDebug )
          {
            TRACE( "+++ Step=%I64d pos=%I64d, duration=%I64d hr=%d\n" , rt , pos , duration , hr );
          }
        }
      }
    }
    if (SUCCEEDED(hr))
    {
      if (m_state == STATE_STOPPED)
        hr = m_pControl->StopWhenReady();
    }
  }
  return hr;
}

HRESULT DShowCapture::GetDuration(LONGLONG& Duration)
{
  if (m_pSeek == NULL)
  {
    return E_UNEXPECTED;
  }
  return m_pSeek->GetDuration(&Duration);
}

bool DShowCapture::ScanProperties(LPCTSTR text, bool& Invalidate)
{
  CCaptureGadget::ScanProperties(text,Invalidate);
  FXPropertyKit pk(text);
  FXString fNewName;
  if (pk.GetString("FileName",fNewName))
  {
    if ((fNewName!=m_fName) && OpenFile(fNewName))
      m_fName=fNewName;
    else
    {
      m_fName=fNewName;
      Invalidate=true;
      SENDERR("Can't open file \"%s\"",fNewName);
    }
  }
  pk.GetBool("LoopClip",m_LoopClip);
  if ( pk.GetDouble( "SpeedFactor" , m_dSpeedFactor ) )
  {
    if ( -5.0 > m_dSpeedFactor )
    {
      m_dSpeedFactor = -5. ;
      Invalidate = true ;
    }
    if ( m_dSpeedFactor > 5.0 )
    {
      m_dSpeedFactor = 5. ;
      Invalidate = true ;
    }
    if ( m_pSeek )
    {
      if ( m_state == STATE_RUNNING )
        m_pControl->Stop() ;
      m_pSeek->SetRate( m_dSpeedFactor ) ;
      if ( m_state == STATE_RUNNING )
        m_pControl->Run() ;
    }
  }
  bool wantInput(false);
  if ( pk.GetBool("SoftwareTrigger",wantInput) )
  {
    if (wantInput && (m_pInput==NULL))
    {
      if (m_state == STATE_RUNNING)
        m_pControl->Pause();
      m_pInput = new CInputConnector(nulltype, _fn_capture_trigger, this);
      Status().WriteBool(STATUS_REDRAW, true);
    }
    else if ((!wantInput) && (m_pInput!=NULL))
    {
      if (m_state == STATE_RUNNING)
        m_pControl->Run();
      delete m_pInput;
      m_pInput=NULL;
      Status().WriteBool(STATUS_REDRAW, true);
    }
  }
  return true;
}

bool DShowCapture::PrintProperties(FXString& text)
{
  CCaptureGadget::PrintProperties(text);
  FXPropertyKit pc(text);
  pc.WriteString("FileName",m_fName);
  pc.WriteBool("LoopClip",m_LoopClip);
  pc.WriteBool("SoftwareTrigger",(m_pInput!=NULL));
  pc.WriteDouble( "SpeedFactor" , m_dSpeedFactor ) ;
  text=pc;
  return true;
}

void DShowCapture::AsyncTransaction(CDuplexConnector* pConnector, CDataFrame* pDataFrame)
{
  CTextFrame* TextFrame = pDataFrame->GetTextFrame(DEFAULT_LABEL);
  if (!TextFrame)
  {
    pDataFrame->Release();
    return;
  }
  FXParser pk=(LPCTSTR)TextFrame->GetString();
  pDataFrame->Release();
  FXString cmd,param;
  FXSIZE pos=0;
  pk.GetWord(pos,cmd);
  if ((cmd.CompareNoCase("list")==0) || (cmd.CompareNoCase("help")==0) || (cmd.CompareNoCase("?")==0))
  {
    pk="FileName\r\nSWTrigger\r\nLoopFilm\r\nLength\r\nPosition\r\n\r\nFor example:\r\n\tset loopfilm(false)";
  }
  else if ((cmd.CompareNoCase("get")==0) && (pk.GetWord(pos,cmd)))
  {
    if (cmd.CompareNoCase("FileName")==0)
    {
      FXPropertyKit fxpk;
      if ( (!PrintProperties(fxpk)) || (!fxpk.GetString("FileName",pk)) )
      {
        pk="Error";
      }
    }
    else if (cmd.CompareNoCase("SWTrigger")==0)
    {
      FXPropertyKit fxpk;
      if ( (!PrintProperties(fxpk)) || (!fxpk.GetString("SoftwareTrigger",pk)) )
      {
        pk="Error";
      }
    }
    else if (cmd.CompareNoCase("LoopFilm")==0)
    {
      FXPropertyKit fxpk;
      if ( (!PrintProperties(fxpk)) || (!fxpk.GetString("LoopClip",pk)) )
      {
        pk="Error";
      }
    }
    else if (cmd.CompareNoCase("Length")==0)
    {
      LONGLONG duration;
      if (GetDuration(duration)==S_OK)
        pk.Format("%.1f s",((double)duration)/10000000.0);
      else
        pk="Error";
    }
    else if (cmd.CompareNoCase("Position")==0)
    {
      LONGLONG position;
      if (GetPosition(position)==S_OK)
        pk.Format("%.1f s",((double)position)/10000000.0);
      else
        pk="Error";
    }
  }
  else if ((cmd.CompareNoCase("set")==0) && (pk.GetWord(pos,cmd)) && (pk.GetParamString(pos, param)))
  {
    if (cmd.CompareNoCase("FileName")==0)
    {
      if (param.IsEmpty())
        pk="Error";
      else 
      {
        FXPropertyKit fxpk;
        bool Invalidate;
        fxpk.WriteString("FileName",param);
        if (ScanProperties(fxpk,Invalidate))
          pk="OK";
        else 
          pk="Error";
      }
    }
    else if (cmd.CompareNoCase("SWTrigger")==0)
    {
      if (param.IsEmpty())
        pk="Error";
      else 
      {
        FXPropertyKit fxpk;
        bool Invalidate;
        bool val=(param.CompareNoCase("true")==0);
        fxpk.WriteBool("SoftwareTrigger",val);
        if (ScanProperties(fxpk,Invalidate))
          pk="OK";
        else 
          pk="Error";
      }
    }
    else if (cmd.CompareNoCase("LoopFilm")==0)
    {
      if (param.IsEmpty())
        pk="Error";
      else 
      {
        FXPropertyKit fxpk;
        bool Invalidate;
        bool val=(param.CompareNoCase("true")==0);
        fxpk.WriteBool("LoopClip",val);
        if (ScanProperties(fxpk,Invalidate))
          pk="OK";
        else 
          pk="Error";
      }
    }
    else if (cmd.CompareNoCase("Position")==0)
    {
      LONGLONG position;
      double dpos=atof(param);
      position=(LONGLONG)(dpos*10000000.0);
      if (SetPosition(position)==S_OK)
        pk="OK";
      else
        pk="Error";
    }
  }
  else
  {
    pk="Error";
  }
  CTextFrame* retV=CTextFrame::Create(pk);
  retV->ChangeId(NOSYNC_FRAME);
  if (!m_pControlPin->Put(retV))
    retV->RELEASE(retV);
}
