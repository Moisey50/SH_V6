// ScanByCamera.h : Implementation of the ScanByCamera class


#include "StdAfx.h"
#include "ScanByCamera.h"

// Do replace "TODO_Unknown" to necessary gadget folder name in gadgets tree
IMPLEMENT_RUNTIME_GADGET_EX(ScanByCamera, CFilterGadget, "Video", TVDB400_PLUGIN_NAME);

ScanByCamera::ScanByCamera(void)
{
    m_pInput = new CInputConnector(vframe);
    m_pOutput = new COutputConnector(vframe);
    m_pDuplexConnector = new CDuplexConnector( this , transparent, transparent);
    m_iLineNumber = 240 ;
    m_iImageHeight = 800 ;
    m_pOutputTVFrame = NULL ;
    Resume();
}

void ScanByCamera::ShutDown()
{
  //TODO: Add all destruction code here
  CFilterGadget::ShutDown();
	delete m_pInput;
	m_pInput = NULL;
	delete m_pOutput;
	m_pOutput = NULL;
	delete m_pDuplexConnector ;
	m_pDuplexConnector = NULL ;
  freeTVFrame( m_pOutputTVFrame ) ;
  m_pOutputTVFrame = NULL ;
}

CDataFrame* ScanByCamera::DoProcessing(const CDataFrame* pDataFrame)
{
  if ( ! pDataFrame )
    return NULL ;
  const CVideoFrame * pFrame = pDataFrame->GetVideoFrame() ;
  if ( pFrame )
  {
    const pTVFrame ptv = (pTVFrame) pFrame ;
    if ( is16bit( ptv) )
      return NULL ; // now we can work with 8 bits images only
    if ( GetHeight( ptv ) > (DWORD)m_iLineNumber )
    {
      DWORD dwOutWidth = GetWidth( ptv ) ;
      if ( m_pOutputTVFrame  && ( (dwOutWidth != GetWidth( m_pOutputTVFrame)) 
        || (GetHeight(m_pOutputTVFrame) != m_iImageHeight) ) )
      {
        freeTVFrame( m_pOutputTVFrame ) ;
        m_pOutputTVFrame = NULL ;
      }
      if ( !m_pOutputTVFrame  && m_iImageHeight )
      {
        m_pOutputTVFrame = makeNewY8Frame( dwOutWidth , m_iImageHeight ) ;
        if ( !m_pOutputTVFrame )
          return NULL ;
        memset( GetData( m_pOutputTVFrame ) , 128 , GetImageSize( m_pOutputTVFrame) ) ;
        m_iLastLine = 0 ;
      }
      if ( m_pOutputTVFrame )
      {
        BYTE * pOutImageData = GetData(m_pOutputTVFrame) ;
        if ( m_iLastLine == m_iImageHeight - 1 ) // last line
        {
          memcpy( pOutImageData , pOutImageData + dwOutWidth ,
            GetImageSize( m_pOutputTVFrame) - dwOutWidth ) ;
        }
        memcpy( GetLine8(pOutImageData , m_iLastLine , dwOutWidth ) ,
          GetLine(ptv , m_iLineNumber ) , 
          dwOutWidth ) ;
        if ( m_iLastLine < m_iImageHeight - 1 )
          ++m_iLastLine ;
        pTVFrame pOutFrame = makecopyTVFrame( m_pOutputTVFrame ) ;
        CVideoFrame * pOutVideoFrame = CVideoFrame::Create( pOutFrame ) ;
        return pOutVideoFrame ;
      }
    }
  }
  return NULL;
}

bool ScanByCamera::ScanProperties(LPCTSTR text, bool& Invalidate)
{
  
  FXPropertyKit pk(text);
  pk.GetInt( "ScanLine" , m_iLineNumber ) ;
  pk.GetInt( "OutputHeight" , m_iImageHeight ) ;
  return true;
}

bool ScanByCamera::PrintProperties(FXString& text)
{
    FXPropertyKit pk;
    CFilterGadget::PrintProperties(text);
    pk.WriteInt( "ScanLine" , m_iLineNumber ) ;
    pk.WriteInt( "OutputHeight" , m_iImageHeight ) ;
    text+=pk;
    return true;

}

bool ScanByCamera::ScanSettings(FXString& text)
{
   text = "template(Spin(ScanLine,0,2000),"
     "Spin(OutputHeight,100,1000))";
  return true;
}


int ScanByCamera::GetDuplexCount()
{
  return CFilterGadget::GetDuplexCount();
}

CDuplexConnector* ScanByCamera::GetDuplexConnector(int n)
{
  return CFilterGadget::GetDuplexConnector(n);
}

void ScanByCamera::AsyncTransaction(CDuplexConnector* pConnector, CDataFrame* pParamFrame)
{
  pParamFrame->Release( pParamFrame );
}
