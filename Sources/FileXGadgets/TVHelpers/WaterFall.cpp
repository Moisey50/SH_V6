// WaterFall.h : Implementation of the WaterFall class


#include "StdAfx.h"
#include "WaterFall.h"

inline BOOL GetSpectrumAmp( const CArrayFrame & Data , FXDblArray& Result , 
                           double& dMin , double& dMax , int iLen , bool bReset = true )
{
  if ( !Data.GetCount() )
    return FALSE ;

  if ( bReset )
  {
    dMin = DBL_MAX ;
    dMax = DBL_MIN ;
    Result.RemoveAll() ;
  }
  DPOINT * pData = (DPOINT*) Data.GetData() ;
  if ( iLen > Data.GetCount() )
    iLen = Data.GetCount() ;
  for ( int i = 0 ; i < iLen ; i++ )
  {
    double dAmpl ;
    dAmpl = pData[i].y ;
    Result.Add( dAmpl ) ;
    if ( dAmpl > dMax )
      dMax = dAmpl ;
    if ( dAmpl < dMin )
      dMin = dAmpl ;
  }
  return true ;
}

inline BOOL GetSpectrumAmp( const CArrayFrame * pDataRe , const CArrayFrame * pDataIm , 
                           FXDblArray& Result , 
                           double& dMin , double& dMax , int iLen , bool bReset = true )
{
  if ( !pDataRe || !pDataRe->GetCount() )
    return FALSE ;
    // check lengths are equal
  if ( pDataIm && (pDataRe->GetCount() != pDataIm->GetCount()))
    return FALSE ;

  if ( bReset )
  {
    dMin = DBL_MAX ;
    dMax = DBL_MIN ;
    Result.RemoveAll() ;
  }
  DPOINT * pRePts = (DPOINT*) pDataRe->GetData() ;
  DPOINT * pImPts = (pDataIm) ? (DPOINT*) pDataIm->GetData() : NULL ;
  if ( iLen > pDataRe->GetCount() )
    iLen = pDataRe->GetCount() ;
  for ( int i = 0 ; i < iLen ; i++ )
  {
    double dAmpl = ( pDataIm == NULL ) ? pRePts[i].y 
      : sqrt(pRePts[i].y * pRePts[i].y + pImPts[i].y * pImPts[i].y) ;
    Result.Add( dAmpl ) ;
    if ( dAmpl > dMax )
      dMax = dAmpl ;
    if ( dAmpl < dMin )
      dMin = dAmpl ;
  }
  return true ;
}

// Do replace "TODO_Unknown" to necessary gadget folder name in gadgets tree
IMPLEMENT_RUNTIME_GADGET_EX(WaterFall, CFilterGadget, "Wave", TVDB400_PLUGIN_NAME);

WaterFall::WaterFall(void)
{
    m_pInput = new CInputConnector( arraytype );
    m_pOutput = new COutputConnector(vframe);
    m_pDuplexConnector = new CDuplexConnector( this , transparent, transparent);
    m_iImageWidth = 512 ;
    m_iImageHeight = 1024 ;
    m_pCurrentState = NULL ;
    m_bAutoGain = FALSE ;
    m_iDecimationInterval = 30 ;
    m_iDecimationCounter = 0 ;
    m_dGain_dB = 0. ;
    m_Law = 1 ;
    m_bSizeChanged = false ;
    Resume();
}

void WaterFall::ShutDown()
{
  //TODO: Add all destruction code here
  CFilterGadget::ShutDown();
	delete m_pInput;
	m_pInput = NULL;
	delete m_pOutput;
	m_pOutput = NULL;
	delete m_pDuplexConnector ;
	m_pDuplexConnector = NULL ;
  if ( m_pCurrentState )
    freeTVFrame( m_pCurrentState ) ;
  m_pCurrentState = NULL ;
}

CDataFrame* WaterFall::DoProcessing(const CDataFrame* pDataFrame)
{
  if ( ! pDataFrame )
    return NULL ;
  if ( (m_iDecimationCounter++ % m_iDecimationInterval) != 0 )
    return NULL ;

  FXDblArray ResultSpectrum ;
  double dMin , dMax ;
  int iHalfLen ;
  CFramesIterator * Iterator = pDataFrame->CreateFramesIterator(arraytype);
  if (Iterator)
  {
    const CArrayFrame* ReFrame = NULL;
    const CArrayFrame* ImFrame = NULL;
    const CArrayFrame* AmpFrame = NULL;
    const CArrayFrame* PhFrame = NULL;

    CDataFrame* pFrame = Iterator->Next(DEFAULT_LABEL);
    while (pFrame)
    {
      if (!ReFrame)
      {
        if ( ReFrame = pFrame->GetArrayFrame("Re") )
        {
          pFrame = Iterator->Next(DEFAULT_LABEL);
          continue ;
        }
      }
      if (!ImFrame)
      {
        if ( ImFrame = pFrame->GetArrayFrame("Im") )
        {
          pFrame = Iterator->Next(DEFAULT_LABEL);
          continue ;
        }
      }
      if (!AmpFrame)
      {
        if ( AmpFrame = pFrame->GetArrayFrame("Amp") )
        {
          pFrame = Iterator->Next(DEFAULT_LABEL);
          continue ;
        }
      }
      if (!PhFrame)
      {
        if ( PhFrame = pFrame->GetArrayFrame("Ph") )
        {
          pFrame = Iterator->Next(DEFAULT_LABEL);
          continue ;
        }
      }
      pFrame = Iterator->Next(DEFAULT_LABEL);
    }
    delete Iterator ;
    if ( ReFrame && ReFrame->GetCount() )
    {
      iHalfLen = (ReFrame->GetCount() / 2) ;
      if ( iHalfLen > m_iImageWidth )
        iHalfLen = m_iImageWidth ;
      GetSpectrumAmp( ReFrame , ImFrame , ResultSpectrum , dMin , dMax , iHalfLen , true ) ;
    }
    else if ( AmpFrame && AmpFrame->GetCount() )
    {
      iHalfLen = (AmpFrame->GetCount() / 2) ;
      if ( iHalfLen > m_iImageWidth )
        iHalfLen = m_iImageWidth ;
      GetSpectrumAmp( AmpFrame , NULL , ResultSpectrum , dMin , dMax , iHalfLen , true ) ;
    }
  }
  if ( !ResultSpectrum.GetCount() )
    return NULL ;

  if ( m_bSizeChanged  )
  {
    if ( m_pCurrentState )
    {
      freeTVFrame( m_pCurrentState ) ;
      m_pCurrentState = NULL ;
    }
    m_bSizeChanged = false ;
  }
  if ( !m_pCurrentState && m_iImageWidth && m_iImageHeight )
  {
    m_pCurrentState = makeNewY8Frame( m_iImageWidth , m_iImageHeight ) ;
    if ( !m_pCurrentState )
      return NULL ;
    memset( GetData( m_pCurrentState) , 0 , GetImageSize( m_pCurrentState) ) ;
  }

    // Do shift for one string in old data
  memcpy( GetData(m_pCurrentState) + m_iImageWidth , GetData( m_pCurrentState) ,
    GetImageSize( m_pCurrentState ) - m_iImageWidth ) ;
    // Form first row in old data
  LPBYTE pPixel = (LPBYTE)GetData( m_pCurrentState ) ;
  if ( m_bAutoGain )
  {
    if ( (dMax - dMin) > 0)
    {
        // Amplitude normalization to range 0..255
      for ( int i = 0 ; i < iHalfLen ; i++ )
      {
        *(pPixel++) = (BYTE)( 255. * (ResultSpectrum[i] - dMin)/(dMax - dMin) ) ;
      }
    }
    else
      memset( pPixel , 0 , m_iImageWidth ) ;
  }
  else // fixed gain
  {
    double dGain = pow( 10. , m_dGain_dB/20. ) ;
    // Amplitude gain with  to range 0..255
    for ( int i = 0 ; i < iHalfLen ; i++ )
    {
      double dValue ;
      switch( m_Law )
      {
      case 0: // Linear
        dValue = 1.e-3 * ((ResultSpectrum[i] - dMin) * dGain) ;
        break ;
      case 1: // Logarithmic
        dValue = 32 * log10((ResultSpectrum[i] - dMin) * dGain) ;
        break ;
      }
      int iValue = (int)dValue ; 
      *(pPixel++) = (BYTE)( (iValue <= m_iCutLevel)? 0 : (( iValue > 255 )? 255 : iValue) ) ;
    }
  }
  
  //Create new TVframe for output
  pTVFrame pOutFrame = makecopyTVFrame( m_pCurrentState ) ;
  if ( pOutFrame )
  {
    CVideoFrame * pOutVideoFrame = CVideoFrame::Create( pOutFrame ) ;
    pOutVideoFrame->CopyAttributes( pDataFrame ) ;
    return pOutVideoFrame ;
  }
  return NULL ;
}

bool WaterFall::ScanProperties(LPCTSTR text, bool& Invalidate)
{
  
  FXPropertyKit pk(text);
  if ( pk.GetInt( "ViewWidth" , m_iImageWidth ) )
    m_bSizeChanged = true ;
  if( pk.GetInt( "ViewHeight" , m_iImageHeight ) )
    m_bSizeChanged = true ;
  pk.GetInt( "Decimation" , m_iDecimationInterval ) ;
  pk.GetDouble( "Gain_dB" , m_dGain_dB ) ;
  pk.GetInt( "Law" , m_Law ) ;
  pk.GetInt( "AutoGain" , m_bAutoGain ) ;
  pk.GetInt( "CutLevel" , m_iCutLevel ) ;
  return true;
}

bool WaterFall::PrintProperties(FXString& text)
{
    FXPropertyKit pk;
    CFilterGadget::PrintProperties(text);
    pk.WriteInt( "ViewWidth" , m_iImageWidth ) ;
    pk.WriteInt( "ViewHeight" , m_iImageHeight ) ;
    pk.WriteInt( "Decimation" , m_iDecimationInterval ) ;
    pk.WriteDouble( "Gain_dB" , m_dGain_dB ) ;
    pk.WriteInt( "AutoGain" , m_bAutoGain ) ;
    pk.WriteInt( "Law" , m_Law ) ;
    pk.WriteInt( "CutLevel" , m_iCutLevel ) ;
    text+=pk;
    return true;

}

bool WaterFall::ScanSettings(FXString& text)
{
   text = "template(Spin(ViewWidth,0,2000),"
     "Spin(ViewHeight,100,1000),"
     "Spin(Decimation,1,100),"
     "EditBox(Gain_dB),"
     "ComboBox(Law(Linear(0),Log10(1))),"
     "Spin(CutLevel,0,255),"
     "ComboBox(AutoGain(No(0),Yes(1)))"
     ")";
  return true;
}


int WaterFall::GetDuplexCount()
{
  return CFilterGadget::GetDuplexCount();
}

CDuplexConnector* WaterFall::GetDuplexConnector(int n)
{
  return CFilterGadget::GetDuplexConnector(n);
}

void WaterFall::AsyncTransaction(CDuplexConnector* pConnector, CDataFrame* pParamFrame)
{
  pParamFrame->Release( pParamFrame );
}
