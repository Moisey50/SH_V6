// AnchorNormalization.cpp: implementation of the AnchorNormalization class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "SmartNorma.h"
#include "AnchorNormalization.h"
#include <Gadgets\vftempl.h>
#include <imageproc\simpleip.h>

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

IMPLEMENT_RUNTIME_GADGET_EX(AnchorNormalization, CFilterGadget, "Video.color&brightness", TVDB400_PLUGIN_NAME);

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

AnchorNormalization::AnchorNormalization():
m_pInputWhiteRef(NULL),
m_pInputBlackRef(NULL),
m_Black(NULL), 
m_White(NULL) ,
m_iOffset(0)
{
    m_pInput         = new CInputConnector( transparent );
    m_pOutput        = new COutputConnector(vframe);
    m_pInputWhiteRef = new CInputConnector(vframe,AnchorNormalization_OnWhiteImage,this);
    m_pInputWhiteRef->SetName("WhiteRefInput");
    m_pInputBlackRef = new CInputConnector(vframe,AnchorNormalization_OnBlackImage,this);
    m_pInputBlackRef->SetName("BlackRefInput");
    Resume();
}

void AnchorNormalization::ShutDown()
{
    CFilterGadget::ShutDown(); 
    if (m_pInput)
    {
        delete m_pInput; m_pInput = NULL;
    }
    if (m_pInputWhiteRef)
    {
        delete m_pInputWhiteRef; m_pInputWhiteRef = NULL;
    }
    if (m_pInputBlackRef)
    {
        delete m_pInputBlackRef; m_pInputBlackRef = NULL;
    }
    if (m_pOutput)
    {
        delete m_pOutput; m_pOutput = NULL;
    }
    if (m_Black) freeTVFrame(m_Black); m_Black=NULL;
    if (m_White) freeTVFrame(m_White); m_White=NULL;
}

int AnchorNormalization::GetInputsCount()
{
    return CFilterGadget::GetInputsCount()+2;
}

CInputConnector*    AnchorNormalization::GetInputConnector(int n)
{
    switch (n)
    {
    case 0:
        return CFilterGadget::GetInputConnector(n);
    case 1:
        return m_pInputWhiteRef;
    case 2:
        return m_pInputBlackRef;
    default:
        return NULL;
    }
}

CDataFrame* AnchorNormalization::DoProcessing(const CDataFrame* pDataFrame)
{
    FXAutolock lock(m_Lock);
    const CVideoFrame* VideoFrame = pDataFrame->GetVideoFrame(DEFAULT_LABEL);
    if ( !VideoFrame || VideoFrame->IsNullFrame() )
    {
      if ( pDataFrame->GetTextFrame( _T( "ResetBlack" ) ) )
      {
        if ( m_Black )
        {
          freeTVFrame( m_Black ) ;
          m_Black = NULL ;
        }
      }
      if ( pDataFrame->GetTextFrame( _T( "ResetWhite" ) ) )
      {
        if ( m_White )
        {
          freeTVFrame( m_White ) ;
          m_White = NULL ;
        }
      }
      return NULL ;
    }
    pTVFrame frame = NULL ;    
    if (m_White)
    {
        frame = makecopyTVFrame(VideoFrame);
        if (m_Black)
            _normalize_bwt(frame, m_Black, m_White);
        else
            _normalize_wt(frame, m_White);
    }
    else
    {
        if (m_Black)
        {
            frame = makecopyTVFrame(VideoFrame);
            //             _normalize_b(frame, m_Black);
            _minus_black( frame, m_Black , m_iOffset ) ;
        }
        else
        {
            //             _normalize(frame);
            frame = makecopyTVFrame(VideoFrame);
        }
    }
    CVideoFrame* retVal=CVideoFrame::Create(frame);
    retVal->CopyAttributes( VideoFrame );
    retVal->SetTime( GetGraphTime() * 1.e-3 ) ;
    return retVal;
}

void AnchorNormalization::OnWhiteImage(CDataFrame* lpData)
{
    FXAutolock lock(m_Lock);
    CVideoFrame* VideoFrame = lpData->GetVideoFrame(DEFAULT_LABEL);
    if ( m_White )
    {
      freeTVFrame( m_White );
      m_White = NULL ;
    }
    if (VideoFrame)
    {
      m_White = makecopyTVFrame( VideoFrame );
    }
    lpData->RELEASE(lpData);
}

void AnchorNormalization::OnBlackImage(CDataFrame* lpData)
{
    FXAutolock lock(m_Lock);
    CVideoFrame* VideoFrame = lpData->GetVideoFrame(DEFAULT_LABEL);
    if (m_Black) 
    {
      freeTVFrame( m_Black );
      m_Black = NULL;
    }
    if (VideoFrame)
        m_Black= makecopyTVFrame(VideoFrame);
    else
        m_Black= NULL;
    lpData->RELEASE(lpData);
}
bool AnchorNormalization::PrintProperties( FXString& text )
{
  FXPropertyKit pc;
  pc.WriteInt( "Offset" , m_iOffset );
  text = pc ;
  return true;
}

bool AnchorNormalization::ScanProperties( LPCTSTR text , bool& Invalidate )
{
  FXPropertyKit pc( text );
  pc.GetInt( "Offset" , m_iOffset ) ;
  return true;
}

bool AnchorNormalization::ScanSettings( FXString& text )
{
  text = "template(Spin(Offset,-10000,10000))";
  return true;
}
