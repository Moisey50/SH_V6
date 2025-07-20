#include "stdafx.h"
#include "TiedDebugRender.h"
#include <gadgets\VideoFrame.h>

IMPLEMENT_RUNTIME_GADGET(CTiedDebugRender, CRenderGadget, LINEAGE_DEBUG);

CTiedDebugRender::CTiedDebugRender() :
  m_pView(NULL),
  m_pStaticView(NULL),
  m_pContainerView(NULL),
  m_DispMode(Default),
  FXTimer("TDRTimer"),
  m_FrCnt(0),
  m_LstTick(-1)
{
  m_pInput = new CInputConnector(transparent);
  Resume();
}

void CTiedDebugRender::ShutDown()
{
  AlarmOff();
  CRenderGadget::ShutDown();
  Detach();
  delete m_pInput;
  m_pInput = NULL;
  if (m_pView->GetSafeHwnd())
    m_pView->DestroyWindow();
  delete m_pView; m_pView = NULL;
  if (m_pStaticView->GetSafeHwnd())
    m_pStaticView->DestroyWindow();
  delete m_pStaticView; m_pStaticView = NULL;
  if (m_pContainerView->GetSafeHwnd())
    m_pContainerView->DestroyWindow();
  delete m_pContainerView; m_pContainerView = NULL;
}

void CTiedDebugRender::Attach(CWnd* pWnd)
{
  Detach();
  m_pParentWnd = pWnd;

  m_pView = new CDIBRender;
  m_pView->Create(pWnd);
  m_pView->SetScale(-1);

  m_pStaticView = new CTextView;
  m_pStaticView->Create(pWnd);
  m_pStaticView->ShowWindow(SW_HIDE);

  m_pContainerView = new CContainerView;
  m_pContainerView->Create(pWnd);
  m_pContainerView->ShowWindow(SW_HIDE);

  m_CurrentView = m_pView;
}



void CTiedDebugRender::Detach()
{
  VERIFY(m_Lock.LockAndProcMsgs());
  if (m_pView)
  {
    m_pView->DestroyWindow();
    delete m_pView; m_pView = NULL;
  }
  if (m_pStaticView)
  {
    m_pStaticView->DestroyWindow();
    delete m_pStaticView; m_pStaticView = NULL;
  }
  if (m_pContainerView)
  {
    m_pContainerView->DestroyWindow();
    delete m_pContainerView; m_pContainerView = NULL;
  }
  m_pParentWnd = NULL;
  m_Lock.Unlock();
}


void CTiedDebugRender::Render(const CDataFrame* pDataFrame)
{
  FXAutolock lock(m_Lock);
  if ((!m_pView) || (!m_pStaticView) || (!m_pContainerView))
    return;
  if (!Tvdb400_IsEOS(pDataFrame))
  {
    switch (m_DispMode)
    {
    case Default:
    {
      CFramesIterator* Iterator = pDataFrame->CreateFramesIterator(nulltype);
      if (Iterator)
      {
        delete Iterator;
        SetContainerView();
        m_pContainerView->Render(pDataFrame);
      }
      else if (pDataFrame->GetVideoFrame(DEFAULT_LABEL))
      {
        SetVideoView();
        m_pView->Render(pDataFrame);
      }
      else if (pDataFrame->GetVideoFrame(DEFAULT_LABEL) == NULL)
      {
        SetStaticView();
        m_pStaticView->Render(pDataFrame);
      }
      break;
    }
    case Text:
    {
      SetStaticView();
      m_pStaticView->Render(pDataFrame);
      break;
    }
    case Video:
    {
      SetVideoView();
      m_pView->Render(pDataFrame);
      break;
    }
    case Container:
    {
      SetContainerView();
      m_pContainerView->Render(pDataFrame);
      break;
    }
    case FrameRate:
    {
      SetStaticView();
      m_FrCnt++;
      break;
    }
    case FrameInfo:
    {
      SetStaticView();
      DispInfo(pDataFrame);
      break;
    }
    }
  }
}

void CTiedDebugRender::OnAlarm(DWORD TimeID)
{
  if (m_LstTick == -1)
  {
    m_LstTick = GetHRTickCount();
    m_FrCnt = 0;
  }
  else
  {
    double tick = GetHRTickCount();
    if (m_pStaticView->GetSafeHwnd())
    {
      CString text;

      double frRate = 0.0;
      frRate = ((double)m_FrCnt)*1000.0 / (tick - m_LstTick);
      text.Format("%.2f", frRate);
      m_pStaticView->SetText(text);
    }
    m_LstTick = tick; m_FrCnt = 0;
  }
}

void CTiedDebugRender::SetDispMode(Mode mode)
{
  m_DispMode = mode;
  if (m_DispMode == FrameRate)
  {
    m_FrCnt = 0;
    m_LstTick = -1;
    SetAlarmFreq(1.0);
    AlarmOn();
  }
  else
  {
    AlarmOff();
  }
}

void CTiedDebugRender::SetStaticView()
{
  if (m_CurrentView != m_pStaticView)
  {
    CRect rc;
    m_pParentWnd->GetClientRect(rc);
    m_pStaticView->BringWindowToTop();
    m_pStaticView->MoveWindow(rc);
    m_pStaticView->ShowWindow(SW_SHOW);
    m_pView->ShowWindow(SW_HIDE);
    m_pContainerView->ShowWindow(SW_HIDE);
    m_CurrentView = m_pStaticView;
    m_pStaticView->SetText("");
  }
}

void CTiedDebugRender::SetVideoView()
{
  if (m_CurrentView != m_pView)
  {
    CRect rc;
    m_pParentWnd->GetClientRect(rc);
    m_pView->BringWindowToTop();
    m_pView->MoveWindow(rc);
    m_pView->ShowWindow(SW_SHOW);
    m_pStaticView->ShowWindow(SW_HIDE);
    m_pContainerView->ShowWindow(SW_HIDE);
    m_CurrentView = m_pView;
  }
}

void CTiedDebugRender::SetContainerView()
{
  if (m_CurrentView != m_pContainerView)
  {
    CRect rc;
    m_pParentWnd->GetClientRect(rc);
    m_pContainerView->BringWindowToTop();
    m_pContainerView->MoveWindow(rc);
    m_pContainerView->ShowWindow(SW_SHOW);
    m_pStaticView->ShowWindow(SW_HIDE);
    m_pView->ShowWindow(SW_HIDE);
    m_CurrentView = m_pContainerView;
  }
}


void CTiedDebugRender::DispInfo(const CDataFrame *pDataFrame)
{
  CString text;
  double frTime = pDataFrame->GetTime();
  if (Tvdb400_IsEOS(pDataFrame))
  {
    if (frTime != -1)
      text.Format("ID: EOS\nTime:%.0f\nType:%s", frTime, Tvdb400_TypeToStr(pDataFrame->GetDataType()));
    else
      text.Format("ID: EOS\nTime: Undefined\nType:%s", Tvdb400_TypeToStr(pDataFrame->GetDataType()));
  }
  else
  {
    if (frTime != -1)
    {
      CString time;
      unsigned sec = (unsigned)(frTime / 1000000);
      int hours = sec / 3600;
      int mins = (sec % 3600) / 60;
      sec %= 60;
      int msec = ((unsigned)(frTime / 1000)) % 1000;
      time.Format("%02d:%02d:%02d.%03d", hours, mins, sec, msec);
      //text.Format("ID:%d\nTime:%.0f\nType:%s", pDataFrame->GetId(),frTime,Tvdb400_TypeToStr(pDataFrame->GetDataType()));
      text.Format("ID:%d\nLabel:'%s'\nRegistered:%s\nTime:%s\nType:%s\n", pDataFrame->GetId(), pDataFrame->GetLabel(), (pDataFrame->IsRegistered()) ? "true" : "false", time, Tvdb400_TypeToStr(pDataFrame->GetDataType()));
      if (pDataFrame->GetDataType() == vframe)
      {
        const CVideoFrame* vf = pDataFrame->GetVideoFrame(DEFAULT_LABEL);
        CString tmpS;
        if (vf->lpBMIH)
        {
          tmpS.Format("Video format:%s\nFrame size %dx%d\n",
            GetVideoFormatName(vf->lpBMIH->biCompression ), vf->lpBMIH->biWidth, vf->lpBMIH->biHeight);
        }
        else
        {
          tmpS = "Empty video frame";
        }
        text += tmpS;
      }
    }
    else
      text.Format("ID:%d\nTime: Undefined\nType:%s\n", pDataFrame->GetId(), Tvdb400_TypeToStr(pDataFrame->GetDataType()));
  }
  m_pStaticView->SetText(text);
}
