// VideoTools.cpp: implementation of the CVideoTools class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "TVVideo.h"
#include "VideoTools.h"
#include "VideoFilters.h"
#include <imageproc\cut.h>
#include <gadgets\quantityframe.h>
#include <imageproc\simpleip.h>
#include <gadgets\RectFrame.h>
#include <gadgets\ContainerFrame.h>

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

//////////////////////////////////////////////////////////////////////////////////////////////////

IMPLEMENT_RUNTIME_GADGET_EX(VideoAverage, CGadget, "Video.average", TVDB400_PLUGIN_NAME);

VideoAverage::VideoAverage()
{
  m_pInput =    new CInputConnector( transparent );
  m_pVideoOut = new COutputConnector(vframe);
  m_pValueOut = new COutputConnector(quantity);
  m_ComplementOutput = CO_AverageValue ;
  m_iFrameCounter = 0 ;

  Resume();
}

void VideoAverage::ShutDown()
{
  CGadget::ShutDown();

  delete m_pInput;
  m_pInput = NULL;
  delete m_pVideoOut;
  m_pVideoOut = NULL;
  delete m_pValueOut;
  m_pValueOut = NULL;
}

int VideoAverage::GetInputsCount()
{
  return 1;
}

int VideoAverage::GetOutputsCount()
{
  return 2;
}

CInputConnector* VideoAverage::GetInputConnector(int n)
{
  if (!n)
    return m_pInput;
  return NULL;
}

COutputConnector* VideoAverage::GetOutputConnector(int n)
{
  if (!n)
    return m_pVideoOut;
  else if (n == 1)
    return m_pValueOut;
  return NULL;
}

bool VideoAverage::PrintProperties(FXString& text)
{
  FXPropertyKit pc;
  pc.WriteInt("Mode",m_Averager.GetMode());
  switch ( m_Averager.GetMode() )
  {
    case CAverager::AVG_ADD_AND_NORMALIZE:
      pc.WriteInt("AllowedMax",m_Averager.GetAllowedMax()); 
      break ;
    case CAverager::AVG_SIMPLE_ADD:
      pc.WriteInt( "MaxCnt" , m_Averager.GetAllowedAddCnt() );
      break ;
    default:
      pc.WriteInt("FramesRange",m_Averager.GetFramesRange()); 
      break ;
  }
  pc.WriteInt( "ComplementOut" , (int) m_ComplementOutput ) ;
  text=pc;
  return true;
}

bool VideoAverage::ScanProperties(LPCTSTR text, bool& Invalidate)
{
  FXPropertyKit pc(text);
  int i;

  if (pc.GetInt("Mode",i))
  {
    Invalidate=(i!=m_Averager.GetMode());
    if (Invalidate)
      m_Averager.SetMode(i);
  }
  if (pc.GetInt("FramesRange",i))
    m_Averager.SetFramesRange(i);
  if (pc.GetInt("AllowedMax",i))
  {
    m_Averager.SetAllowedMax(i);
    m_Averager.AllowedMaxChanged() ;
  }
  if ( pc.GetInt( "MaxCnt" , i ) )
  {
    m_Averager.SetAllowedAddCnt( i );
    m_Averager.AllowedMaxChanged() ;
  }
  pc.GetInt( "ComplementOut" , (int&) m_ComplementOutput ) ;
  return true;
}

bool VideoAverage::ScanSettings(FXString& text)
{
  text="template(";
  FXString formats="ComboBox(Mode(";
  for (int i=CAverager::AVG_INFINITE_UNIFORM; i<CAverager::AVG_AFTER_LAST; i++)
  {
    FXString tmpS;
    if (i!=CAverager::AVG_INFINITE_UNIFORM)
      formats+=',';
    tmpS.Format("%s(%d)",m_Averager.GetModeName(i),i);
    formats+=tmpS;
  }
  text+=formats+="))";
  if (m_Averager.GetMode()==CAverager::AVG_SLIDEWINDOW)
    text+=",Spin(FramesRange,1,1000)";
  if ( m_Averager.GetMode() == CAverager::AVG_ADD_AND_NORMALIZE )
    text += ",Spin(AllowedMax,50,2000000000)";
  if ( m_Averager.GetMode() == CAverager::AVG_SIMPLE_ADD )
    text += ",Spin(MaxCnt,1,10000)";
  text += ",ComboBox(ComplementOut(Average(0),FrameNumber(1)))" ;
  text += ')';
  return true;
}

int VideoAverage::DoJob()
{
  CDataFrame* pDataFrame = NULL;
  while (m_pInput->Get(pDataFrame))
  {
    ASSERT(pDataFrame);
    if (Tvdb400_IsEOS(pDataFrame))
    {
      m_Averager.Reset();
      if ( !m_pValueOut->Put( pDataFrame ) )
        pDataFrame->Release();
    }
    else switch (m_Mode)
    {
    case mode_reject:
      pDataFrame->Release();
      break;
    case mode_transmit:
      if (!m_pVideoOut->Put(pDataFrame))
        pDataFrame->Release();
      break;
    case mode_process:
      {
        CDataFrame* Container = pDataFrame->CopyContainer();
        if (Container)
        {
          pDataFrame->Release();
          pDataFrame = Container;
        }
        LPCTSTR pLabel = pDataFrame->GetLabel() ;
        if ( pLabel && ( _tcscmp( pLabel , _T( "Reset" ) ) == 0 ) )
        {
          CQuantityFrame * pQuan = pDataFrame->GetQuantityFrame() ;
          int iNewAverageFactor = 0 ;
          if ( !Container && pQuan )
            iNewAverageFactor = (int) (*pQuan) ;
          m_Averager.Reset( iNewAverageFactor ) ;
          m_iFrameCounter = 0 ;
          if ( !pDataFrame->GetVideoFrame() )
          {
            pDataFrame->Release();
            break;
          }
        }
        
        CVideoFrame* VideoFrame = pDataFrame->GetVideoFrame(DEFAULT_LABEL);
        if (VideoFrame)
        {
          double ts=GetHRTickCount();
          m_Averager.AddFrame(VideoFrame);
          CVideoFrame* ResultFrame = CVideoFrame::Create(m_Averager.GetAvgFrame());
          AddCPUUsage(GetHRTickCount()-ts);
          ResultFrame->CopyAttributes(pDataFrame);
          CQuantityFrame* Complement = 
            (m_ComplementOutput == CO_FrameCounter )? 
            CreateQuantityFrame( ++m_iFrameCounter ) : CreateQuantityFrame( m_Averager.GetAvgValue() ) ;
          Complement->SetLabel( (m_ComplementOutput == CO_FrameCounter) ? "AverCnt" : "AverVal" ) ;
          Complement->ChangeId( pDataFrame->GetId() );
          if (VideoFrame != pDataFrame) // container
          {
            pDataFrame->Release(VideoFrame);
            ((CContainerFrame*)pDataFrame)->AddFrame(ResultFrame);
            ResultFrame = (CVideoFrame*)pDataFrame;
          }
          else
            pDataFrame->Release();
          PutFrame( m_pVideoOut , ResultFrame) ;
          PutFrame( m_pValueOut , Complement ) ;
        }
      }
    }
  }
  return WR_CONTINUE;
}
