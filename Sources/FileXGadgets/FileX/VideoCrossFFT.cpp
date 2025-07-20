// UserExampleGadget.h.h : Implementation of the UserExampleGadget class


#include "StdAfx.h"
#include "VideoCrossFFT.h"

#include <iostream>
#include <iomanip>
#include <map>
#include <random>
#include <cmath>
#include <math\intf_sup.h>

//std::default_random_engine generator; 
//std::normal_distribution<double> distribution(/*mean=*/0.0, /*stddev=*/1.0); 

USER_FILTER_RUNTIME_GADGET(CrossFFT,"Math");	//	Mandatory

void _calc_AmpPh( void* Re , void* Im , int count , int size , DPOINT** ppAmp , DPOINT** ppPh )
{
  if ( size == sizeof( DPOINT ) )
  {
    DPOINT* pRe = (DPOINT*) Re;
    DPOINT* pIm = (DPOINT*) Im;
    *ppAmp = (DPOINT*) calloc( count , sizeof( DPOINT ) );
    *ppPh = (DPOINT*) calloc( count , sizeof( DPOINT ) );
    for ( int i = 0; i < count; i++ )
    {
      (*ppAmp)[ i ].x = pRe[ i ].x;
      (*ppAmp)[ i ].y = sqrt( pRe[ i ].y * pRe[ i ].y + pIm[ i ].y * pIm[ i ].y );
      (*ppPh)[ i ].x = pRe[ i ].x;
      (*ppPh)[ i ].y = atan2( pIm[ i ].y , pRe[ i ].y );
    }
  }
  else if ( size == sizeof( double ) )
  {
    double* pRe = (double*) Re;
    double* pIm = (double*) Im;
    *ppAmp = (DPOINT*) calloc( count , sizeof( DPOINT ) );
    *ppPh = (DPOINT*) calloc( count , sizeof( DPOINT ) );
    for ( int i = 0; i < count; i++ )
    {
      (*ppAmp)[ i ].x = (double) i;
      (*ppAmp)[ i ].y = sqrt( pRe[ i ] * pRe[ i ] + pIm[ i ] * pIm[ i ] );
      (*ppPh)[ i ].x = (double) i;
      (*ppPh)[ i ].y = atan2( pIm[ i ] , pRe[ i ] );
    }
  }
}

void VideoCrossParsUpdate( LPCTSTR pPropertyName , void * pObject , bool& bInvalidate , bool& bRescanProperties)
{
  CrossFFT * pGadget = (CrossFFT*) pObject ;
  FXAutolock al( pGadget->m_LockSettings , _T( "VideoCrossParsUpdate" ) ) ;

  if ( _tcscmp( pPropertyName , _T( "CrossPt" ) ) == 0 )
  {
    pGadget->m_sCrossPt.Trim( _T( " \t" ) ) ; 
    pGadget->m_sCrossPt.Remove( _T( ' ' ) ) ;

    sscanf_s( (LPCTSTR) (pGadget->m_sCrossPt) , _T( "%d,%d" ) ,
      &(pGadget->m_CrossPt.x) , &(pGadget->m_CrossPt.y) ) ;
  }
  else if ( _tcscmp( pPropertyName , _T( "RectHalfSize" ) ) == 0 )
  {
    pGadget->m_sActiveRectHalfSize.Trim( _T( " \t" ) ) ; 
    pGadget->m_sActiveRectHalfSize.Remove( _T( ' ' ) ) ;

    sscanf_s( (LPCTSTR) (pGadget->m_sActiveRectHalfSize) , _T( "%d,%d" ) ,
      &(pGadget->m_ActiveRectHalfSize.cx) ,
      &(pGadget->m_ActiveRectHalfSize.cy) ) ;
  }
  else if (_tcscmp( pPropertyName , _T( "Ranges" ) ) == 0)
  {
    pGadget->m_AccumulationRanges.Trim( _T( " \t" ) ) ;
    pGadget->m_AccumulationRanges.Remove( _T( ' ' ) ) ;
    pGadget->ScanRanges( pGadget->m_AccumulationRanges ) ;
  }
}

int CrossFFT::ScanRanges( FXString& AsString )
{
  m_Ranges.clear() ;
  FXSIZE iPos = 0 ;
  LPCTSTR pAsString = ( LPCTSTR ) AsString ;
  FXString Token = AsString.Tokenize( "({[" , iPos ) ;
  while ( !Token.IsEmpty() )
  {
    CRangeI NewRange( Token ) ;
    if (NewRange.IsValid())
      m_Ranges.push_back( NewRange ) ;
    else
      break ;
    Token = AsString.Tokenize( "({[" , iPos ) ;
  }
  return (int)m_Ranges.size() ;
}


CrossFFT::CrossFFT() :
  m_CrossPt( 400 , 300 )
  , m_sCrossPt( _T("400, 300") )
  , m_CrossShift( 0 , 0 )
  , m_ActiveRectHalfSize( 50 , 50 )
  , m_iFFT_Order(10)
  , m_iHalfUsedData( 50 )
  , m_dCameraResolution_um_per_pix( 1.43 )
  , m_Multiplier( 1 , 1 )
  , m_iSpectrumViewShift( 0 )
{
  m_pdHorIm = m_pdHorRe = m_pdVertIm = m_pdVertRe = NULL ;
  m_iAllocatedFFTSize = 0 ;
  m_pdVertAmpl = m_pdHorAmpl = NULL ;
  m_pdWindow = NULL ;
  m_iAllocatedWindowSize = 0 ;
  m_pdCoeffs = NULL ;
  init();
  m_OutputMode = modeAppend ;
}

CrossFFT::~CrossFFT()
{
  bCheckAndRealloc( &m_pdHorRe , 0 ) ;
  m_iAllocatedFFTSize = 0 ;
  if ( m_pdCoeffs )
    free( m_pdCoeffs ) ;
}

void CrossFFT::bCheckAndRealloc( 
  double ** pArray , int iNewSize )
{
  if ( *pArray )
    delete[] *pArray ;
  if ( iNewSize )
    *pArray = new double[ iNewSize ] ;
  else
    *pArray = NULL ;
}

void CrossFFT::InitEnvelope( double * pEnv , int iHalfSize )
{
  ASSERT( pEnv ) ;
  double dStep = M_PI / (double) iHalfSize ;
  double dAngle = dStep ;
  for ( int i = 1 ; i <= iHalfSize ; i++ )
  {
    double dAmpl = (1. + cos( dAngle )) * 0.5 ;
    pEnv[ iHalfSize - i ] = pEnv[ iHalfSize + i - 1 ] = dAmpl ;
  }
}

void CrossFFT::CalcSpectrums( const CVideoFrame * pData )
{
  DWORD iFFTSize = (1 << m_iFFT_Order);
  if ( m_iAllocatedFFTSize != iFFTSize )
  {
    bCheckAndRealloc( &m_pdHorRe , iFFTSize * 6 ) ;
    m_pdHorIm = m_pdHorRe + iFFTSize ;
    m_pdVertRe = m_pdHorIm + iFFTSize ;
    m_pdVertIm = m_pdVertRe + iFFTSize ;
    m_pdHorAmpl = m_pdVertIm + iFFTSize ;
    m_pdVertAmpl = m_pdHorAmpl + iFFTSize ;
    m_iAllocatedFFTSize = iFFTSize ;
    if ( m_pdCoeffs )
      free( m_pdCoeffs ) ;
    m_pdCoeffs = _prepare_fft_lut( m_iFFT_Order , 1 ) ;
  }
  if ( m_iAllocatedWindowSize != m_iHalfUsedData * 2 )
  {
    bCheckAndRealloc( &m_pdWindow , m_iHalfUsedData * 2 ) ;
    m_iAllocatedWindowSize = m_iHalfUsedData * 2 ;
    InitEnvelope( m_pdWindow , m_iHalfUsedData ) ;
  }
  memset( m_pdHorRe , 0 , m_iAllocatedFFTSize * 6 * sizeof( double ) );
  int iImageWidth = GetWidth( pData ) ;
  int iImageHeight = GetHeight( pData ) ;

  CPoint RealCrossPt = m_CrossPt + m_CrossShift;
  int iInitShiftXHor = RealCrossPt.x - m_iHalfUsedData ;
  if ( iInitShiftXHor < 0 )
    iInitShiftXHor = 0 ;
  int iEndShiftX = RealCrossPt.x + m_iHalfUsedData ;
  if ( iEndShiftX >= iImageWidth )
    iInitShiftXHor = iImageWidth - (2 * m_iHalfUsedData) - 1 ;
  m_LastWorkingRect.left = iInitShiftXHor ;
  m_LastWorkingRect.right = m_LastWorkingRect.left + 2 * m_iHalfUsedData ;
  int iInitShiftYVert = RealCrossPt.y - m_iHalfUsedData ;
  if ( iInitShiftYVert < 0 )
    iInitShiftYVert = 0 ;
  int iEndShiftYVert = RealCrossPt.y + m_iHalfUsedData ;
  if ( iEndShiftYVert >= iImageHeight )
    iInitShiftYVert = iImageHeight - (2 * m_iHalfUsedData) - 1 ;
  m_LastWorkingRect.top = iInitShiftYVert ;
  m_LastWorkingRect.bottom = m_LastWorkingRect.top + 2 * m_iHalfUsedData ;
  m_dLastHorDC = m_dLastVertDC = 0. ;
  double * pEnvelope = m_pdWindow ;
  double * pDst = m_pdHorRe + (m_iAllocatedFFTSize / 2) - m_iHalfUsedData ;
  double * pIterD = pDst ;

  if ( is16bit( pData ) )
  {
      // fill horizontal source data for 16 bits
    WORD *pSrc = GetData16( pData ) + iInitShiftXHor
      + iImageWidth * RealCrossPt.y ;
    WORD * p = pSrc ;
    WORD *pEnd = pSrc + 2 * m_iHalfUsedData ;
    while ( p < pEnd )
      m_dLastHorDC += (*(pIterD++) = ((double) *(p++)) * ( *(pEnvelope++) ));

    m_dLastHorDC /= (double) (2 * m_iHalfUsedData) ;
    if ( m_bRemoveDC )
    {
      double * pd = pDst ;
      double * pdEnd = pd + 2 * m_iHalfUsedData ;
      while ( pd < pdEnd )
        *(pd++) -= m_dLastHorDC ;
    }

    // fill vertical source data for 16 bits
    pSrc = GetData16( pData ) + RealCrossPt.x
      + iImageWidth * iInitShiftYVert ;
    p = pSrc ;
    // step will be one row
    pEnd = pSrc + (2 * m_iHalfUsedData) * iImageWidth ;
    pDst = m_pdVertRe + (m_iAllocatedFFTSize / 2) - m_iHalfUsedData ;
    pIterD = pDst ;
    pEnvelope = m_pdWindow ;
    while ( p < pEnd )
    {
      m_dLastVertDC += (*(pIterD++) = ((double) *p) * (*(pEnvelope++)));
      p += iImageWidth ; // go to the next point in vertical direction
    }
    m_dLastVertDC /= (double) (2 * m_iHalfUsedData) ;
    if ( m_bRemoveDC )
    {
      double * pd = pDst ;
      double * pdEnd = pd + 2 * m_iHalfUsedData ;
      while ( pd < pdEnd )
        *(pd++) -= m_dLastVertDC ;
    }
  }
  else 
  {
    // fill horizontal source data for 8 bits
    BYTE * pSrc = (BYTE*) GetData( pData ) + iInitShiftXHor
      + iImageWidth * RealCrossPt.y ;
    BYTE * p = pSrc ;
    BYTE *pEnd = pSrc + 2 * m_iHalfUsedData ;
    while ( p < pEnd )
      m_dLastHorDC += (*(pIterD++) = ((double) *(p++)) * (*(pEnvelope++)));

    m_dLastHorDC /= (double) (2 * m_iHalfUsedData) ;
    if ( m_bRemoveDC )
    {
      double * pd = pDst ;
      double * pdEnd = pd + 2 * m_iHalfUsedData ;
      while ( pd < pdEnd )
        *(pd++) -= m_dLastHorDC ;
    }
    
    // fill vertical source data for 16 bits
    pSrc = GetData( pData ) + RealCrossPt.x
      + iImageWidth * iInitShiftYVert ;
    p = pSrc ;
    // step will be one row
    pEnd = pSrc + (2 * m_iHalfUsedData) * iImageWidth ;
    pDst = m_pdVertRe + (m_iAllocatedFFTSize / 2) - m_iHalfUsedData ;
    pIterD = pDst ;
    pEnvelope = m_pdWindow ;
    while ( p < pEnd )
    {
      m_dLastVertDC += (*(pIterD++) = ((double) *p) * (*(pEnvelope++)));
      p += iImageWidth ; // go to the next point in vertical direction
    }
    m_dLastVertDC /= (double) (2 * m_iHalfUsedData) ;
    if ( m_bRemoveDC )
    {
      double * pd = pDst ;
      double * pdEnd = pd + 2 * m_iHalfUsedData ;
      while ( pd < pdEnd )
        *(pd++) -= m_dLastVertDC ;
    }
  }
  
  _cfft_1( m_pdHorRe , m_pdHorIm , m_iFFT_Order , m_pdCoeffs );
  _cfft_1( m_pdVertRe , m_pdVertIm , m_iFFT_Order , m_pdCoeffs );

}

CContainerFrame * CrossFFT::CreateMarkers(
  CFigureFrame * pData, cmplx& Origin, cmplx& Step,
  int iAmpl, bool bVert)
{
  if (pData->GetCount() < 3)
    return NULL;

  CContainerFrame * pResult = NULL;
  CDPoint * pBase = pData->GetData() + 2;
  CDPoint * pFirst = pBase + 1 ;
  CDPoint * pLast = &pFirst[pData->GetUpperBound()] - 5 ;
  cmplx GraphIterator(Origin - Step);
  FXIntArray Mins , Maxes;
  int iDownCnt = 0 , iUpCnt = 0 ;
  int iMaxIndex = 0, iMinIndex = 0;
  bool bMinSaved = true;
  double dBaseVal = (bVert) ? pBase->x : pBase->y;
  double dVal = (bVert) ? pFirst->x : pFirst->y;
  dVal = ((bVert)? Origin.real() : Origin.imag()) - dVal ;
  double dMax = dVal;
  double dMin = dMax;
  double dCurrent = dMax;
  double dNextVal = (bVert) ? (pFirst + 1)->x : (pFirst + 1)->y;
  dNextVal = ((bVert) ? Origin.real() : Origin.imag()) - dNextVal;
  int iLastMarked = 0 ;

  LPCTSTR pColor = (bVert) ? "0xffff00" : "0x0000ff";
  cmplx ConstShift = ((bVert) ? cmplx(-50., -20.) : cmplx(-10., -50.));
  cmplx AddShift = ((bVert) ? cmplx(-100., 0.) : cmplx(0., -50.));
  double dOverNextVal ;
  int iMarkCnt = 0 ;
  for (CDPoint * pSample = pFirst; pSample < pLast ; pSample++ )
  {
    dOverNextVal = (bVert) ? (pSample + 2)->x : (pSample + 2)->y;
    dOverNextVal = ((bVert) ? Origin.real() : Origin.imag()) - dOverNextVal;
    if (dVal > dCurrent)
    {
      if ( !bMinSaved 
        && (dNextVal > dCurrent) && (dOverNextVal > dCurrent) )
      {
        cmplx FirstEnd = GraphIterator ;
        cmplx SecondEnd = FirstEnd + 
          ((bVert)? cmplx (-50.,0.) : cmplx(0., -50.) ) ;
        CFigureFrame * pMarker = CreateLineFrame(FirstEnd, SecondEnd,
          pColor );
        if (!pResult)
          pResult = CContainerFrame::Create();
        pResult->AddFrame(pMarker);
        bMinSaved = true;
        Mins.Add( ( int )(pSample - pFirst) );
        if ( ++iMarkCnt < 12 )
        {
          FXString MinMax;
          int iPos = (int)((bVert) ? GraphIterator.imag() : GraphIterator.real());
          MinMax.Format("%.1f,%.1f\n%d-%d", dMax, dMin,
            iPos, abs(iPos - iLastMarked));
          cmplx Pt = SecondEnd + ConstShift;
          if (iMarkCnt & 1)
            Pt += AddShift;

          CTextFrame * pDataView = CreateTextFrame(Pt, MinMax,
            pColor, 8);
          pResult->AddFrame(pDataView);
          iLastMarked = iPos;
        }
      }
      dCurrent = dVal;
      if (++iUpCnt > 3)
        iDownCnt = 0;

      dMax = dCurrent;
      iMaxIndex = ( int )(pSample - pFirst);
    }
    else if ( dVal <= dCurrent )
    {
      dCurrent = dVal;
      if ( ++iDownCnt >= 3 )
      {
        iUpCnt = 0;
        bMinSaved = false;
      }
      else if ( iDownCnt == 1 )
      {
        Maxes.Add(iMaxIndex);
      }
      dMin = dCurrent;
      iMinIndex = ( int )(pSample - pFirst);
    }

    GraphIterator += Step;
    dVal = dNextVal;
    dNextVal = dOverNextVal;
  }
  return pResult;
}

CDataFrame* CrossFFT::DoProcessing(const CDataFrame* pDataFrame) 
{
  const CVideoFrame * pNewImage = pDataFrame->GetVideoFrame() ;
  if ( !pNewImage )
    return NULL ;

  CalcSpectrums( pNewImage ) ;
  double dHorAmpl = _ampl( m_pdHorRe , m_pdHorIm , m_pdHorAmpl , m_iAllocatedFFTSize / 2 ) ;
  double dVertAmpl = _ampl( m_pdVertRe , m_pdVertIm , m_pdVertAmpl , m_iAllocatedFFTSize / 2 ) ;

  CSize ImageSize( GetWidth( pNewImage ) , GetHeight( pNewImage ) ) ;
  cmplx HorOrigin( 5. , ImageSize.cy - 5. ) ;
  cmplx HorStep( 1024. / m_iHalfUsedData , 0. ) ;
  cmplx VertOrigin( ImageSize.cx - 5. , ImageSize.cy - 15. ) ;
  cmplx VertStep( 0. , -1024. / m_iHalfUsedData ) ;
  int iGraphShift = m_iSpectrumViewShift ;

  int iSpectrumLen = m_iAllocatedFFTSize / 2 ;
  int iGraphLen = iSpectrumLen / m_Multiplier.cx ;

  if ( m_Multiplier.cx > 1 )
  {
    if (( iGraphShift + iGraphLen ) > iSpectrumLen )
      iGraphShift = iSpectrumLen - iGraphLen - 1;
    HorStep *= (double)m_Multiplier.cx ;
    VertStep *= ( double )m_Multiplier.cx ;
  }

  CFigureFrame * pHorSpectrum = CreateGraphic( m_pdHorAmpl + iGraphShift ,
    iGraphLen , HorOrigin , HorStep , cmplx( 0. , -300. ) ,
    0. , dHorAmpl , "0x0000ff" , (double)m_Multiplier.cy , 600. ) ;

  CContainerFrame * pOut = CContainerFrame::Create() ;
  pOut->AddFrame( pHorSpectrum ) ;
  
  CFigureFrame * pVertSpectrum = CreateGraphic( m_pdVertAmpl + iGraphShift ,
    iGraphLen , VertOrigin , VertStep , cmplx( -300. , 0. ) ,
    0. , dVertAmpl , "0xffff00" , ( double ) m_Multiplier.cy , 600. ) ;
  pOut->AddFrame( pVertSpectrum ) ;

  cmplx cViewPt( ImageSize.cx / 3. , 100. ) ;
  m_LastPowers.clear() ;
  for ( auto it = m_Ranges.begin() ; it != m_Ranges.end() ; it++  )
  {
    double dX1st = ( it->m_iBegin - m_iSpectrumViewShift ) * m_Multiplier.cx ;
    double dX2nd = ( it->m_iEnd - m_iSpectrumViewShift ) * m_Multiplier.cx ;

    cmplx cMarkerLeft( HorOrigin.real() + dX1st , HorOrigin.imag() + 3 ) ;
    cmplx cMarkerRight( HorOrigin.real() + dX2nd , HorOrigin.imag() + 3 ) ;
    CFigureFrame * pMarker = CreateLineFrame( cMarkerLeft , cMarkerRight ,
      RGB( 255 , 0 , 0 ) ) ;
    pMarker->Attributes()->WriteInt( "thickness" , 3 ) ;
    pOut->AddFrame( pMarker ) ;

    double dSum = 0. ;
    bool bUseHoriz = (m_UseSpectrums == 0) || (m_UseSpectrums == 2) ;
    bool bUseVert = (m_UseSpectrums == 0) || (m_UseSpectrums == 1) ;
    if ( bUseHoriz )
    {
      if ( bUseVert )
      {
        for ( int i = it->m_iBegin ; i < it->m_iEnd ; i++ )
          dSum += m_pdHorAmpl[ i ] + m_pdVertAmpl[ i ] ;
      }
      else
      {
        for ( int i = it->m_iBegin ; i < it->m_iEnd ; i++ )
          dSum += m_pdHorAmpl[ i ];
      }
    }
    else
    {
      for ( int i = it->m_iBegin ; i < it->m_iEnd ; i++ )
        dSum += m_pdVertAmpl[ i ] ;
    }

    double dLastRatio = m_LastPowers.size() ? dSum / m_LastPowers[ 0 ] : 1. ;
    m_LastPowers.push_back( dSum ) ;
    pOut->AddFrame( CreateTextFrame( cViewPt , "0x0000ff" , 18 , NULL , 0 ,
      "R%d=%.5f(%.2f)" , ( it - m_Ranges.begin() + 1 ) , dLastRatio , dSum ) ) ;
    cViewPt._Val[ _IM ] += 50. ;
  }
//   CContainerFrame * pHorMarkers = CreateMarkers(pHorSpectrum,
//     HorOrigin, HorStep, 300, false);
//   if (pHorMarkers)
//     pOut->AddFrame(pHorMarkers);
//   CContainerFrame * pVertMarkers = CreateMarkers(pVertSpectrum,
//     VertOrigin, VertStep, 300, true);
//   if (pVertMarkers)
//     pOut->AddFrame(pVertMarkers);
  pOut->AddFrame( CreateTextFrame( cViewPt , "0x0000ff" , 12 , NULL , 0 ,
    "%.1f,%.1f,%.1f,%.1f,%.1f,%.1f,%.1f,%.1f,%.1f,%.1f\n"
    "%.1f,%.1f,%.1f,%.1f,%.1f,%.1f,%.1f,%.1f,%.1f,%.1f" , 
    m_pdVertAmpl[0], m_pdVertAmpl[1] , m_pdVertAmpl[ 2 ] , m_pdVertAmpl[ 3 ] , m_pdVertAmpl[ 4 ] , m_pdVertAmpl[ 5 ] ,
    m_pdVertAmpl[ 6 ] , m_pdVertAmpl[ 7 ] , m_pdVertAmpl[ 8 ] , m_pdVertAmpl[ 9 ] , 
    m_pdVertAmpl[ 511 ] , m_pdVertAmpl[ 510 ] , m_pdVertAmpl[ 509 ] , m_pdVertAmpl[ 508 ] ,
    m_pdVertAmpl[ 507 ] , m_pdVertAmpl[ 506 ] , m_pdVertAmpl[ 505 ] , m_pdVertAmpl[ 504 ] , 
    m_pdVertAmpl[ 503 ] , m_pdVertAmpl[ 502 ] ) ) ;

  cmplx cRealCross( m_CrossPt.x + m_CrossShift.x , m_CrossPt.y + m_CrossShift.y ) ;

  CFigureFrame * pHorizontalAxis  = CreateLineFrame(
    cmplx( 0. , cRealCross.imag() ) , cmplx( ImageSize.cx , cRealCross.imag() ) ,
    RGB( 255 , 0 , 255 ) ) ;
  CFigureFrame *pVerticalAxis  = CreateLineFrame(
    cmplx( cRealCross.real() , 0. ) , cmplx( cRealCross.real() , ImageSize.cy ) ,
    RGB( 255 , 0 , 255 ) ) ;
  pOut->AddFrame( pVerticalAxis ) ;
  pOut->AddFrame( pHorizontalAxis ) ;

  CRectFrame * pROIRect = CRectFrame::Create( &m_LastWorkingRect ) ;
  pROIRect->Attributes()->WriteString( _T( "color" ) , _T( "0xff0000" ) ) ;
  pROIRect->SetLabel( _T( "ROI" ) ) ;
  pOut->AddFrame( pROIRect ) ;

  return pOut ;
}

void CrossFFT::AsyncTransaction(CDuplexConnector* pConnector, CDataFrame* pParamFrame) 
{
  CTextFrame * ParamText = pParamFrame->GetTextFrame(DEFAULT_LABEL);
  if (!ParamText)
    return;
  bool bInvalidate = false;
  UserGadgetBase::ScanProperties(ParamText->GetString(), bInvalidate);
  pParamFrame->Release();
};


void CrossFFT::PropertiesRegistration() 
{
  addProperty(SProperty::EDITBOX ,	_T("CrossPt")	,	&m_sCrossPt ,
    SProperty::String );
  addProperty(SProperty::SPIN, _T("XCrossShift"), &m_CrossShift.x,
    SProperty::Int, -100, 100);
  addProperty(SProperty::SPIN, _T("YCrossShift"), &m_CrossShift.y,
    SProperty::Int, -100, 100);
  addProperty( SProperty::COMBO , _T( "UseSpectrums" ) , &m_UseSpectrums ,
    SProperty::Int , _T( "Both;Vertical;Horizontal;" ) ) ;
  addProperty( SProperty::SPIN , _T( "RemoveDC" ) , &m_bRemoveDC ,
    SProperty::Int , 0 , 1 );

  addProperty(SProperty::EDITBOX		,	_T("RectHalfSize")	,
    &m_sActiveRectHalfSize ,	SProperty::String		);
  addProperty( SProperty::SPIN , _T( "FFT_Order" ) , &m_iFFT_Order ,
    SProperty::Int , 5 , 10 );
  addProperty(SProperty::SPIN, _T("HalfUsed"), &m_iHalfUsedData,
    SProperty::Int, 32, 512);
  addProperty(SProperty::EDITBOX, _T("Resolution_um_per_pix"),
    &m_dCameraResolution_um_per_pix, SProperty::Double);
  addProperty( SProperty::SPIN , _T( "XMultiplier" ) , &m_Multiplier.cx ,
    SProperty::Int , 1 , 20 );
  addProperty( SProperty::SPIN , _T( "YMultiplier" ) , &m_Multiplier.cy ,
    SProperty::Int , 1 , 500 );
  addProperty( SProperty::SPIN , _T( "SpectrumShift" ) , &m_iSpectrumViewShift ,
    SProperty::Int , 0 , 511 );
  addProperty( SProperty::EDITBOX , _T( "Ranges" ) , &m_AccumulationRanges ,
    SProperty::String );

  SetChangeNotification( _T( "CrossPt" ) , VideoCrossParsUpdate , this ) ;
  SetChangeNotification( _T( "RectHalfSize" ) , VideoCrossParsUpdate , this ) ;
  SetChangeNotification( _T( "Ranges" ) , VideoCrossParsUpdate , this ) ;
};


void CrossFFT::ConnectorsRegistration() 
{
  addInputConnector( transparent, "VideoFrameIn");
  addOutputConnector( transparent , "VideoWithSpectrums");
  
  addDuplexConnector( transparent, transparent, "Control");
};

//bool CrossFFT::ScanProperties(LPCTSTR text, bool& Invalidate)
//{
//  bool bRes = UserGadgetBase::ScanProperties( text , Invalidate ) ;
//  double dData[10] ;
//  memset( dData , 0 , sizeof( dData ) ) ;
//  FXPropKit2 pk(text) ;
//  int iNNumbers = GetArray( pk , "Mean" , 'f' , 10 , dData ) ;
//  if ( iNNumbers )
//  {
//    if ( m_cmplxStd.imag() <= 0. )
//      m_cmplxStd._Val[1] = -DBL_MAX ;
//    if ( m_cmplxStd.real() <= 0. )
//      m_cmplxStd._Val[ 0 ] = -DBL_MAX ;
//    if ( m_dSTD <= 0. )
//      m_dSTD = -DBL_MAX ;
//    m_CurrentReal._Init( m_cmplxMean._Val[0] = dData[0] , m_cmplxStd.real() ) ;
//    m_dMean = dData[ 0 ] ;
//    
//    m_bComplex = ( iNNumbers >= 2 ) ;
//    if ( m_bComplex )
//    {
//      m_cmplxMean = cmplx( dData[0] , dData[1] ) ;
//      //m_ImagNormGen.SetParams( dData[1] , m_cmplxStd.imag() ) ;
//      m_CurrentImag._Init( m_cmplxMean.imag() , m_cmplxStd.imag() ) ;
//      m_sMean.Format( "%g,%g" , dData[ 0 ] , dData[ 1 ] ) ;
//    }
//    else
//      m_sMean.Format( "%g" , dData[ 0 ] ) ;
//
////     else
////       m_cmplxMean._Val[1] = m_cmplxMean._Val[0] ;
//  }
//  iNNumbers = GetArray( pk , "STD" , 'f' , 10 , dData ) ;
//  if ( iNNumbers )
//  {
//    //m_NormGen.SetParams( dMean , dSTD = dData[0] ) ;
//    m_CurrentReal._Init( m_cmplxMean.real() , m_cmplxStd._Val[1] = dData[0] ) ;
//    m_dSTD = dData[ 0 ] ;
//    if ( m_bComplex && (iNNumbers == 2) )
//    {
//      m_cmplxStd = cmplx( dData[0] , dData[1] ) ;
//      //m_ImagNormGen.SetParams( m_cmplxMean.real() , m_cmplxStd.imag() ) ;
//      m_CurrentImag._Init( m_cmplxMean.imag() , m_cmplxStd.imag() ) ;
//      m_sStdDev.Format( "%g,%g" , dData[ 0 ] , dData[ 1 ] ) ;
//    }
//    else
//    {
//      if ( dData[0] < 0. )
//        dData[0] = 0.0 ;
//      m_cmplxStd = cmplx( dData[0] , dData[0] ) ;
//      //m_ImagNormGen.SetParams( dMean , m_cmplxStd.imag() ) ;
//      m_CurrentImag._Init( m_cmplxMean.imag() , m_cmplxStd.imag() ) ;
//      m_sStdDev.Format( "%g" , dData[ 0 ] ) ;
//    }
//  }
//  m_Protect.Lock() ;
//  gDistributionReal.param( m_CurrentReal ) ;
//  gDistributionImag.param( m_CurrentImag ) ;
//  m_Protect.Unlock() ;
//  return bRes ;
//}

//bool CrossFFT::PrintProperties(FXString& text)
//{
//  FXPropKit2 pk(text) ;
//  if ( !m_bComplex )
//  {
//    pk.WriteDouble( "STD" , m_dSTD ) ;
//    pk.WriteDouble( "Mean" , m_dMean ) ;
//  }
//  else
//  {
//    pk.WriteCmplx( "Mean" , m_cmplxMean ) ;
//    pk.WriteCmplx( "STD" , m_cmplxStd ) ;
//  }
//  text = (LPCTSTR)pk ;
//  return true ;
//}




