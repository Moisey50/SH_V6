// BlackArea.h.h : Implementation of the BlackArea class


#include "StdAfx.h"
#include "BlackAreaCalculator.h"
#include "helpers\FramesHelper.h"

USER_FILTER_RUNTIME_GADGET(BlackArea,"Statistics");

BlackArea::BlackArea()
{
  m_dProportionThres_Perc		= 75 ;
  iHistogramDensityPercents_Property	= 5;
  m_dHistDensityPercent = 1.0 ;
  bUseTresholdByValueOnly_Property		= false;
  m_bOutBinarized = FALSE ;
  m_iAlgorithm = HistAnalyze ;
  m_iDistForward = 10 ;
  m_pHist = NULL ;
  m_iAllocatedHistSize = 0 ;
  m_iSavedBlackLevel = 20 ;
  m_iSavedWhiteLevel = 220 ;
  init();
}

unsigned int BlackArea::defineTresholdByValue(
  LPBYTE pProcessingPicture, unsigned int uiProcessingPictureSize ,
  int iPixelSize )
{
  m_iMax = 0;
  if ( iPixelSize == 1 )
  {
    m_iMin = UINT8_MAX ;
    LPBYTE pEnd = pProcessingPicture + uiProcessingPictureSize ;
    //	Find Min & Max values
    do
    {
      int iVal = *pProcessingPicture ;
      if( iVal < m_iMin )        
        m_iMin = iVal ;
      if( iVal > m_iMax )
        m_iMax = iVal ;
    } while ( ++pProcessingPicture < pEnd ) ;
  }
  else if ( iPixelSize == 2 )
  {
    m_iMin = UINT16_MAX ;
    LPWORD p16 = (LPWORD)pProcessingPicture ;
    LPWORD pEnd = p16 + uiProcessingPictureSize ;
    //	Find Min & Max values
    do
    {
      int iVal = *p16 ;
      if( iVal < m_iMin )        
        m_iMin = iVal ;
      if( iVal > m_iMax )
        m_iMax = iVal ;
    } while ( ++p16 < pEnd ) ;
  }
  else
    return 0 ;
  // Calculate Threshold
  m_iMainThres = m_iMin + 
    ROUND( (m_iMax-m_iMin) * m_dProportionThres_Perc) ;

  return m_iMainThres ;
}

void BlackArea::calculateHistogramArray(
  LPBYTE pProcessingPicture, unsigned int uiProcessingPictureSize, 
  double* histogramArray_Percents , int iPixelSize )
{
  double dOnePixelWeight = 100./(double)uiProcessingPictureSize ;
  if ( iPixelSize == 1 )
  {
    LPBYTE pEnd = pProcessingPicture + uiProcessingPictureSize ;
    do 
    {
      histogramArray_Percents[ *pProcessingPicture ] += dOnePixelWeight ;
    } while ( ++pProcessingPicture < pEnd );
  }
  else
  {
    LPWORD p16 = (LPWORD)pProcessingPicture ;
    LPWORD pEnd = p16 + uiProcessingPictureSize ;
    do 
    {
      histogramArray_Percents[ *pProcessingPicture ] += dOnePixelWeight ;
    } while ( ++p16 < pEnd );
  }
}

void BlackArea::calculateHistogramArray(
  LPBYTE pProcessingPicture, unsigned int uiProcessingPictureSize, 
  DWORD* histogramArray_Pixels , int iPixelSize )
{
  m_iMax = 0;
  m_iFrameSize = uiProcessingPictureSize ;
  if ( iPixelSize == 1 )
  {
    m_iMin = UINT8_MAX ;
    LPBYTE pEnd = pProcessingPicture + uiProcessingPictureSize ;
    do 
    {
      int iVal = *pProcessingPicture ;
      if( iVal < m_iMin )        
        m_iMin = iVal ;
      if( iVal > m_iMax )
        m_iMax = iVal ;
      ++histogramArray_Pixels[ iVal ] ;
    } while ( ++pProcessingPicture < pEnd );
  }
  else
  {
    m_iMin = UINT16_MAX ;
    LPWORD p16 = (LPWORD)pProcessingPicture ;
    LPWORD pEnd = p16 + uiProcessingPictureSize ;
    do 
    {
      int iVal = *p16 ;
      if( iVal < m_iMin )        
        m_iMin = iVal ;
      if( iVal > m_iMax )
        m_iMax = iVal ;
      ++histogramArray_Pixels[ iVal ] ;
    } while ( ++p16 < pEnd );
  }
  DWORD * pHist = histogramArray_Pixels ;
  DWORD * pHistEnd = pHist + m_iAllocatedHistSize ;
  m_dwHistMax = 0 ;
  for ( ; pHist < pHistEnd ; pHist++ )
  {
    if ( *pHist > m_dwHistMax )
    {
      m_dwHistMax = *pHist ;
      m_dwHistMaxPos = (DWORD)(pHist - histogramArray_Pixels) ;
    }
  }
}
int BlackArea::GetLowerHistMax( int& iMaxPos , int& iIntegral ) 
{
  int iIntegral1 = 0 , iIntegral2 = 0 ; // temporary values
  int iMax = 0 ;
  int i = m_iMin ;
  m_iRealMinPos = 0 ;
  for (  ; i < m_iMax ; i++ )
  {
    int iVal = (int)m_pHist[i] ;
    if ( m_iRealMinPos == 0  &&  iVal > 3 )
      m_iRealMinPos = i ;
    iIntegral1 += iVal ;
    if ( (iMax < iVal)  ||  (iIntegral1 == 0) ) 
    {
      iMax = iVal ;
      iMaxPos = i ;
    }
    else
    {
      iIntegral2 = iIntegral1 ;
      int i2 = i + 1 ;
      int iMaxSearchIndex = i + m_iDistForward ;
      if ( iMaxSearchIndex >= m_iMax )
        iMaxSearchIndex = m_iMax ;
      for (  ; i2 < iMaxSearchIndex ; i2++ )
      {
        iVal = (int)m_pHist[i2] ;
        iIntegral2 += iVal ;
        if ( m_iRealMinPos == 0  &&  iVal > 3 )
          m_iRealMinPos = i ;
        if ( iVal >= iMax )
        {
          i = i2 + 1 ;
          iMax = iVal ;
          iMaxPos = i2 ;
          iIntegral1 = iIntegral2 ;
          break ;
        }
      }
      if ( i2 >= iMaxSearchIndex )
      {
        i = iMaxSearchIndex ;
        iIntegral1 = iIntegral2 ;
        if ( ((double)iIntegral1 * 100. / (double)m_iFrameSize) > 0.02 )
          break ; // max is found
      }
      // else continue search with i
    }
  }
  while ( m_pHist[i] > (DWORD)m_iHistLowMaxValue/10)
  {
    iIntegral1 += m_pHist[i] ;
    if ( m_pHist[i] > (DWORD)iMax )
    {
      iMax = m_pHist[i] ;
      iMaxPos = i ;
    }
    if ( ++i > m_iMax )
      break ;
  }
  m_iLowBegin = m_iRealMinPos ;
  m_iLowEnd = i ;

  iIntegral = iIntegral1 ;
  return iMax ;
};
int BlackArea::GetHighHistMax( int& iMaxPos , int& iIntegral ) 
{
  int iIntegral1 = 0 , iIntegral2 = 0 ; // temporary values
  int iMax = 0 ;
  m_iRealMaxPos = 0 ;
  int i = m_iMax ;
  for (  ; i > 0 ; i-- )
  {
    int iVal = (int)m_pHist[i] ;
    if ( m_iRealMaxPos == 0  && iVal > 3 )
      m_iRealMaxPos = i ;
    iIntegral1 += iVal ;
    if ( (iMax < iVal)  ||  (iIntegral1 == 0) ) 
    {
      iMax = iVal ;
      iMaxPos = i ;
    }
    else
    {
      iIntegral2 = iIntegral1 ;
      int i2 = i - 1 ;
      int iMaxSearchIndex = i - m_iDistForward ;
      if ( iMaxSearchIndex < m_iMin )
        iMaxSearchIndex = m_iMin ;
      for (  ; i2 >= iMaxSearchIndex ; i2-- )
      {
        iVal = (int)m_pHist[i2] ;
        iIntegral2 += iVal ;
        if ( m_iRealMaxPos == 0  && iVal > 3 )
          m_iRealMaxPos = i ;
        if ( iVal >= iMax )
        {
          i = i2 - 1 ;
          iMax = iVal ;
          iMaxPos = i2 ;
          iIntegral1 = iIntegral2 ;
          break ;
        }
      }
      if ( i2 <= iMaxSearchIndex )
      {
        i = iMaxSearchIndex ;
        iIntegral1 = iIntegral2 ;
        if ( ((double)iIntegral1 * 100. / (double)m_iFrameSize) < 0.02 )
          return 0 ; // too small integral
        break ;
      }
      // else continue search with i
    }
  }
  while ( m_pHist[i] > (DWORD)m_iHistHighMaxValue/10)
  {
    iIntegral1 += m_pHist[i] ;
    if ( m_pHist[i] > (DWORD)iMax )
    {
      iMax = m_pHist[i] ;
      iMaxPos = i ;
    }
    if ( --i < m_iMin )
      break ;
  }
  m_iHighBegin = i ;
  m_iHighEnd = m_iRealMaxPos ;
  iIntegral = iIntegral1 ;
  return iMax ;

};

unsigned int BlackArea::defineTresholdByHistogram(
  LPBYTE pProcessingPicture, unsigned int uiProcessingPictureSize ,
  int iPixelSize )
{
  m_iHistSize = ((iPixelSize == 1) ? UINT8_MAX : UINT16_MAX) + 1 ;
  if ( m_pHist  ||  ( m_iAllocatedHistSize != m_iHistSize ))
  {
    if ( m_pHist )
      delete[] m_pHist  ;
    m_pHist = new DWORD[m_iHistSize] ;
    m_iAllocatedHistSize = m_iHistSize ;
  }
  memset( m_pHist , 0 , m_iHistSize * sizeof(DWORD) ) ;

  calculateHistogramArray( pProcessingPicture , 
    uiProcessingPictureSize , m_pHist , iPixelSize );

  DWORD dwHist[256] , dwIntegral[256] , dwReverseIntegral[256] ;
  if ( m_iHistSize == 256 )
  {
    memcpy( dwHist , m_pHist , 1024 ) ;
    memset( dwIntegral , 0 , 1024 ) ;
    memset( dwReverseIntegral , 0 , 1024 ) ;
    DWORD dwSum = 0 ;
    for ( int i = 0 ; i < 256 ; i++ )
    {
      dwSum += dwHist[i] ;
      dwIntegral[i] = dwSum ;
      dwReverseIntegral[i] = m_iFrameSize - dwSum ;
    }
  }
  if ( m_iAlgorithm == HistAnalyze )
  { // analyze histogram for density threshold definition
    m_iHistLowMaxValue = GetLowerHistMax( m_iHistLowMaxPos , m_iHistLowIntegral ) ;
    m_iHistHighMaxValue = GetHighHistMax( m_iHistHighMaxPos , m_iHistHighIntegral ) ;
    if ( (m_iHistLowIntegral + m_iHistHighIntegral < m_iFrameSize) 
      && (m_iHistLowIntegral >= m_iFrameSize/40) 
      && (m_iHistHighIntegral >= m_iFrameSize/40)
      && (abs( m_iHistHighMaxPos - m_iHistLowMaxPos) > 30) 
      )
    {
      int iHighPos = (m_iHighBegin + m_iHighEnd)/2 ;
      int iLowPos = (m_iLowBegin + m_iLowEnd)/2 ;
      //m_iMainThres = m_iLowEnd + (m_iHighBegin - m_iLowEnd) * (m_iFrameSize - m_iHistLowIntegral)/m_iFrameSize ;
      m_iMainThres = m_iLowEnd + ROUND((m_iHighBegin - m_iLowEnd) * m_dProportionThres_Perc * 0.01) ;
    }
    else 
    {
      //m_dHistDensityPercent = 0.05 ;
      double	lowValDensity_Percents	= 0 ;
      int	lowValPosition	= m_iMin ;
      double	highValDensity_Percents	= 0 ;
      int iNActivPixels = ROUND((double)uiProcessingPictureSize * m_dHistDensityPercent * 0.01) ;
      //	Find black pixels threshold
      int lowValAccum = 0 ;
      while ( lowValAccum < iNActivPixels )
      {
        lowValAccum += m_pHist [ lowValPosition ];
        if ( ++lowValPosition >= m_iMax )
          break ;
      }	;


      //	Find white pixels threshold
      int iHighValAccum = 0 ;
      int	highValPosition = m_iMax ;
      while ( iHighValAccum < iNActivPixels )
      {
        iHighValAccum += m_pHist[highValPosition];
        if ( --highValPosition <= m_iMin )
          break ;
      }	;

      // Calculate Threshold
      m_iBlackThres = --lowValPosition ;
      m_iWhiteThres = ++highValPosition ;

      if ( m_iHighBegin < m_iLowEnd ) // High and low hist overlap
      {
        int iSupposedWhiteThres = m_iSavedBlackLevel 
          + ROUND((m_iSavedWhiteLevel - m_iSavedBlackLevel) * 0.8) ;
        if ( m_iHistLowMaxPos > iSupposedWhiteThres ) // big white, small black
        {
          m_iMainThres = m_iBlackThres 
            + ROUND( (m_iHighBegin - m_iBlackThres) * m_dProportionThres_Perc * 0.01 ) ;
        }
        else // big black and small white
        {
          m_iMainThres = m_iLowEnd 
            + ROUND( (m_iWhiteThres - m_iLowEnd) * m_dProportionThres_Perc * 0.01 ) ;
        }
//         m_iMainThres = m_iBlackThres 
//           + ROUND((m_iWhiteThres - m_iBlackThres) * m_dProportionThres_Perc * 0.01) ;
      }
      else if ( m_iHistLowIntegral > m_iFrameSize * 0.90 )
      {
        double dRange = m_iWhiteThres - m_iLowEnd ;
        if ( dRange >= 30.0 )
        {
          m_iMainThres = m_iLowEnd 
            + ROUND( dRange * m_dProportionThres_Perc * 0.01 ) ;
        }
        else
          m_iMainThres = m_iSavedBlackLevel 
            + ROUND(( m_iSavedWhiteLevel - m_iSavedBlackLevel ) * m_dProportionThres_Perc * 0.01) ;
      }
      else if ( m_iHistHighIntegral > m_iFrameSize * 0.90 )
      {
        double dRange = m_iHighBegin - m_iBlackThres ;
        if ( dRange >= 30.0 )
        {
          m_iMainThres = m_iBlackThres 
            + ROUND( dRange * m_dProportionThres_Perc * 0.01 ) ;
        }
        else
          m_iMainThres = (m_iBlackThres + m_iWhiteThres)/2 ;
      }
      else
        m_iMainThres = m_iSavedBlackLevel 
          + ROUND(( m_iSavedWhiteLevel - m_iSavedBlackLevel ) * m_dProportionThres_Perc * 0.01) ;
      ASSERT( m_iMainThres > 0 ) ;
    }
  }
  else if ( m_iAlgorithm == FixedThresholdRelativelyToHistDensityOnEdges )
  {
    double	lowValDensity_Percents	= 0 ;
    int	lowValPosition	= 0;
    double	highValDensity_Percents	= 0 ;
    int iNActivPixels = ROUND((double)uiProcessingPictureSize * m_dHistDensityPercent * 0.01) ;
    //	Find black pixels threshold
    int lowValAccum = 0 ;
    while ( lowValAccum < iNActivPixels )
    {
      lowValAccum += m_pHist [ lowValPosition ];
      if ( ++lowValPosition >= m_iHistSize )
        break ;
    }	;


    //	Find white pixels threshold
    int iHighValAccum = 0 ;
    int	highValPosition = m_iHistSize - 1 ;
    while ( iHighValAccum < iNActivPixels )
    {
      iHighValAccum += m_pHist[highValPosition];
      if ( --highValPosition <= 0 )
        break ;
    }	;

    // Calculate Threshold
    m_iBlackThres = --lowValPosition ;
    m_iWhiteThres = ++highValPosition ;

    double dRange = m_iWhiteThres-m_iBlackThres ;
    m_iMainThres = m_iBlackThres 
      + ROUND( dRange * m_dProportionThres_Perc * 0.01 ) ;
  }
  else // FixedThresholdRelativePresettledBlackWhite
  {
    m_iMainThres = m_iSavedBlackLevel 
      + ROUND( ( m_iSavedWhiteLevel - m_iSavedBlackLevel ) * m_dProportionThres_Perc * 0.01 ) ;
    m_dwSecondHistMaxPos = -1 ;
    m_dwSecondHistMax = 0 ;
    if ( m_dwHistMaxPos > 10 )
    {
//       int iLowLimit = m_dwHistMaxPos - 40 ;
      int i = m_dwHistMaxPos ;
      while ( --i > 0)  
      {
        if ( m_pHist[i] > m_pHist[i + 1] ) 
        {
          int iLimit = i - 15 ;
          if ( iLimit < 0 )
            iLimit = 0 ;
          int j = i - 1 ;
          for ( ; j > iLimit ; j-- )
          {
            if ( m_pHist[j] < m_pHist[i] )
            {
              i = j ;
              break ;
            }
          }
          if ( j <= iLimit )    // i is minimum
            break ; 
        }
      }
      for ( ; i > 0 ; i-- )
      {
        if ( m_pHist[i] > m_dwSecondHistMax )
        {
          m_dwSecondHistMax = m_pHist[i] ;
          m_dwSecondHistMaxPos = i ;
        }
      }
    }
    if ( m_dwHistMaxPos < (DWORD)m_iHistSize - 10 )
    {
      int i = m_dwHistMaxPos ;
      while ( ++i < m_iHistSize)  
      {
        if ( m_pHist[i] > m_pHist[i - 1] ) 
        {
          int iLimit = i + 15 ;
          if ( iLimit > m_iHistSize - 1 )
            iLimit = m_iHistSize - 1 ;
          int j = i + 1 ;
          for ( ; j < iLimit ; j++ )
          {
            if ( m_pHist[j] < m_pHist[i] )
            {
              i = j ;
              break ;
            }
          }
          if ( j >= iLimit )    // i is minimum
            break ; 
        }
      }
      for ( ; i < m_iHistSize ; i++ )
      {
        if ( m_pHist[i] > m_dwSecondHistMax )
        {
          m_dwSecondHistMax = m_pHist[i] ;
          m_dwSecondHistMaxPos = i ;
        }
      }
    }
  }
  return m_iMainThres ;
}

double BlackArea::countSpotsDensity(
  LPBYTE pProcessingPicture, unsigned int uiProcessingPictureSize ,
  int iPixelSize )
{
  unsigned int uiTreshold = 
    (bUseTresholdByValueOnly_Property) ? 
    defineTresholdByValue(pProcessingPicture, 
    uiProcessingPictureSize , iPixelSize ) 
    : 
  defineTresholdByHistogram(pProcessingPicture, 
    uiProcessingPictureSize , iPixelSize );

  unsigned int uiSpotsPixelsCnt = 0;
  if ( iPixelSize = 1 )
  {
    for(unsigned int uiCnt=0; uiCnt<uiProcessingPictureSize; uiCnt++)
    {
      if( pProcessingPicture[uiCnt] < uiTreshold )
        uiSpotsPixelsCnt++;
    }
  }
  else
  {
    LPWORD p16 = (LPWORD)pProcessingPicture ;
    for(unsigned int uiCnt=0; uiCnt<uiProcessingPictureSize; uiCnt++)
    {
      if( p16[uiCnt] < uiTreshold )
        uiSpotsPixelsCnt++;
    }
  }

  double dDensity_percents = 
    100.0 * ((double)uiSpotsPixelsCnt / uiProcessingPictureSize);
  return dDensity_percents;
}

CDataFrame* BlackArea::DoProcessing(const CDataFrame* pDataFrame) 
{
  if ((pDataFrame) && (!Tvdb400_IsEOS(pDataFrame)))
  {
    //	Get Picture Data from Input Package
    const CVideoFrame* pInputPictureData = pDataFrame->GetVideoFrame();
    if ( pInputPictureData )
    {
      LPBITMAPINFOHEADER pInputInfoHeader = pInputPictureData->lpBMIH;
      DWORD dwCompression = pInputInfoHeader->biCompression ;
      //	Get Picture Width
      int iWidth	= pInputInfoHeader->biWidth;
      //	Get Picture Height
      int iHeight	= pInputInfoHeader->biHeight;
      LPBYTE pPicture = GetData(pInputPictureData) ;
      int iPictureSize = (iHeight - 2) * iWidth ; // don't process first and last row
      double dDensity_percents = 0. ;
      switch( dwCompression )
      {
      case BI_Y8:
      case BI_Y800:
      case BI_YUV9:
      case BI_YUV12:
        {
          pPicture += iWidth ;
          //	Calculate Spots Density
          dDensity_percents = countSpotsDensity(
            pPicture, iPictureSize , 1 );
        }
        break ;
      case BI_Y16:
        {
          pPicture += iWidth * 2; // omit first row, 2 bytes per pixel
          //	Calculate Spots Density
          dDensity_percents = countSpotsDensity(
            pPicture, iPictureSize , 2 ); // 2 bytes per pixel
        }
      default: return NULL ;
      }

      // Create Output Frame
      CQuantityFrame* pOutputFrame = CQuantityFrame::Create(dDensity_percents);
      if ( pOutputFrame )
      {
        CopyIdAndTime( pOutputFrame , pDataFrame ) ;
        pOutputFrame->SetLabel( _T("BlackAreaPerc") ) ;

        CContainerFrame * pOutContainer = CContainerFrame::Create() ;
        if ( pOutContainer )
        {
          CopyIdAndTime( pOutContainer , pDataFrame ) ;
          if ( !m_bOutBinarized )
            pOutContainer->AddFrame( pDataFrame ) ;
          else
          {
            CVideoFrame * pBinarized = (CVideoFrame*)( pInputPictureData->Copy() );
            int iBinImageSize = iWidth * iHeight ;
            LPBYTE pBinImage = GetData( pBinarized ) ;
            if ( Is8BitsImage( pBinarized ) )
            {
              LPBYTE pBinImageEnd = pBinImage + iBinImageSize ;

              while ( pBinImage < pBinImageEnd )
              {
                *pBinImage = ( *pBinImage < m_iMainThres ) ? 0 : UINT8_MAX ;
                pBinImage++ ;  
              }
            }
            else if ( Is16BitsImage( pBinarized ) )
            {
              LPWORD pBinImage16 = (LPWORD)pBinImage ;
              LPWORD pBinImage16End = pBinImage16 + iBinImageSize ;
              while ( pBinImage16 < pBinImage16End )
              {
                *pBinImage = ( *pBinImage < m_iMainThres ) ? 0 : UINT16_MAX ;
                pBinImage++ ;  
              }
            }
            pOutContainer->AddFrame( pBinarized ) ;
          }
          pOutContainer->AddFrame( pOutputFrame ) ;
          CTextFrame * pViewResults = CTextFrame::Create() ;
          if ( pViewResults )
          {
            CopyIdAndTime( pViewResults , pDataFrame ) ;
            if ( m_iAlgorithm == FixedThresholdRelativelyToHistDensityOnEdges )
            {
              pViewResults->GetString().Format( 
                _T("black=%6.3f(%%)\nMin=%d Max=%d\n Th=%d Th_low=%d Th_high=%d") ,
                dDensity_percents , m_iMin , m_iMax , m_iMainThres , m_iBlackThres , m_iWhiteThres ) ;
            }
            else if ( m_iAlgorithm == HistAnalyze )
            {
              pViewResults->GetString().Format( 
                _T("black=%6.3f(%%)\nMin=%d Max=%d Thres=%d\nPOSl=%d POSh=%d\n"
                "INTGRLl=%d INTGRLh=%d\n"
                "MAXl=%d MAXh=%d\nWl=%d Wh=%d\n") ,
                dDensity_percents , m_iMin , m_iMax , m_iMainThres , 
                m_iHistLowMaxPos , m_iHistHighMaxPos ,
                m_iHistLowIntegral , m_iHistHighIntegral ,
                m_iHistLowMaxValue , m_iHistHighMaxValue ,
                m_iLowEnd - m_iLowBegin , m_iHighBegin - m_iHighEnd ) ;
            }
            else // FixedThresholdRelativePresettledBlackWhite
            {
              pViewResults->GetString().Format( 
                _T("black=%6.3f(%%)\nMin=%d Max=%d Thres=%d\n"
                "HistMaxPos=%d SecondMaxPos=%d\n"
                "HistMax1=%d HistMax2=%d") ,
                dDensity_percents , m_iMin , m_iMax , m_iMainThres , 
                m_dwHistMaxPos , m_dwSecondHistMaxPos ,
                m_dwHistMax , m_dwSecondHistMax ) ;
            }
            pViewResults->Attributes()->WriteInt( "x" , 10 ) ;
            pViewResults->Attributes()->WriteInt( "y" , 10 ) ;
            pOutContainer->AddFrame( pViewResults ) ;
          }
          if ( m_pHist && m_iAllocatedHistSize == 256 )
          {
            CFigureFrame * pHistShow = CFigureFrame::Create() ;
            if ( pHistShow )
            {
              for ( int i = 0 ; i < m_iAllocatedHistSize ; i++ )
              {
                CDPoint Pt( i + 10. , 210. - 200. * ((double)m_pHist[i]/(double)m_dwHistMax) ) ;
                pHistShow->AddPoint( Pt ) ;
                CopyIdAndTime( pHistShow , pDataFrame ) ;
                pHistShow->Attributes()->WriteString( "color" ,  "0xff00ff" ) ;
                pHistShow->SetLabel( "HistShow" ) ;
              }
            }
            pOutContainer->AddFrame( pHistShow ) ;
          }
          return pOutContainer ;
        }
        return  pOutputFrame;
      }
    }
    else
      return NULL ;
  }
  return NULL ;
}

void BlackArea::AsyncTransaction(CDuplexConnector* pConnector, CDataFrame* pParamFrame) 
{

};

void BlackArea::PropertiesRegistration() 
{
  addProperty(SProperty::EDITBOX,		_T("Thres_Perc"),
    &m_dProportionThres_Perc,	SProperty::Double	);
  addProperty(SProperty::EDITBOX,	_T("HistDensity_Perc"),
    &m_dHistDensityPercent	,	SProperty::Double );
  addProperty(SProperty::SPIN , _T("WhiteLevel") , 
    &m_iSavedWhiteLevel , SProperty::Long , 50 , 65535 ) ;
  addProperty(SProperty::SPIN , _T("BlackLevel") , 
    &m_iSavedBlackLevel , SProperty::Long , 0 , 48000 ) ;
  addProperty(SProperty::COMBO , _T("Algorithm") , 
    &m_iAlgorithm , SProperty::Long , _T("HistAnalyze;FixedThreshold;BlackWhitePreset") ) ;
  addProperty(SProperty::COMBO , _T("Out_Binarized") , 
    &m_bOutBinarized , SProperty::Long , _T("No;Yes") ) ;
};

void BlackArea::ConnectorsRegistration() 
{
  addInputConnector( vframe, "PicInput");
  addOutputConnector(	quantity * text * vframe ,	"AreaQuantity");
};




