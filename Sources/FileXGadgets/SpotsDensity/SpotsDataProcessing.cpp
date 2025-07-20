#include "stdafx.h"

#include "SpotsDataProcessing.h"

SpotsDataProcessing::SpotsDataProcessing()
{
  m_TextObjectName = _T( "areaname" ) ;
  m_CalibDataName = _T( "spot_main" ) ;
}

size_t SpotsDataProcessing::ExtractConturs( const CDataFrame * pDataFrame )
{
  m_Conturs.clear() ;
  CFigureFrame* pFrame = NULL ;
  CFramesIterator * it = pDataFrame->CreateFramesIterator( figure ) ;
  while ( pFrame = ( CFigureFrame* ) it->Next() )
  {
    if ( pFrame->GetCount() > 4 )
    {
      LPCTSTR pLabel = pFrame->GetLabel() ;
      LPCTSTR pObjectName = _tcsstr( pLabel , m_CalibDataName ) ;
      if ( pObjectName )
      {
        int iConturNum = atoi( pObjectName + m_CalibDataName.GetLength() + 1 ) ;
        if ( ( int ) m_Conturs.size() < iConturNum + 1 )
          m_Conturs.resize( iConturNum + 1 ) ;
        m_Conturs[ iConturNum ] = pFrame ;
      }
    }
  }
  if ( it )
    delete it ;
  return m_Conturs.size() ;
}

bool SpotsDataProcessing::FormStatistics( const CDataFrame * pDataFrame ,
  FXString& ResultString )
{
  bool bRes = false ;

  CFStatistics Area_um2 , Diameter_um , Perimeter_um , BoxRatio ;
  CFStatistics XCentroid , YCentroid , XMidPoint , YMidPoint ;
  CFStatistics BoundLeft , BoundTop , BoundRight , BoundBottom ;
  CFStatistics BoundWidth , BoundHeight , Spacing , Ruling_lpcm ;
  CFStatistics Ruling_lpi , Dot_Percent ;

  size_t nConturs = ExtractConturs( pDataFrame ) ;

  Node1D * pNode = m_CalibGrid.m_Spots->GetNodeList()->GetHead() ;
  size_t iHorLineCnt = 0;
  Spot * pFirstInRow = NULL;
  // First cycle for statistics accumulation
  do
  {
    Axis& SpotHorAxis = *( m_CalibGrid.m_HorLines[ iHorLineCnt ] );

    Node1D * pCurrentNode = SpotHorAxis.GetHead() ;
    Spot& FirstInRow = *( pCurrentNode->m_Spot );
    do
    {
      Spot& CurrentSpot = *( pCurrentNode->m_Spot );
      size_t iIndex = CurrentSpot.GetGlobalIndex() ;
      if ( iIndex >= 0 && iIndex < m_Conturs.size() )
      {
        int iXIndexDiffToRowBegin = CurrentSpot.m_Indexes.x - FirstInRow.m_Indexes.x ;
        cmplx cSpotShiftRelToFirstInRow = FirstInRow.m_imgCoord
          + SpotHorAxis.GetMainVector() * ( double ) ( iXIndexDiffToRowBegin ) ;
        Area_um2.Add( CurrentSpot.m_dArea ) ;
        Diameter_um.Add( ( CurrentSpot.m_Sizes.getWidth() + CurrentSpot.m_Sizes.getHeight() ) / 2. ) ;
        Perimeter_um.Add( m_Conturs[ iIndex ]->GetConturLength() ) ;
        CmplxArray Extrems ;
        cmplx cSize ;
        cmplx cMid = FindExtrems( m_Conturs[ iIndex ] , Extrems , NULL , &cSize ) ;
        BoxRatio.Add( cSize.imag() / cSize.real() ) ;

        cmplx cCentroid = CurrentSpot.m_imgCoord - cSpotShiftRelToFirstInRow ;
        XCentroid.Add( cCentroid.real() ) ;
        YCentroid.Add( cCentroid.imag() ) ;

        cmplx cMidPoint = ( Extrems[ EXTREME_INDEX_RIGHT ] + Extrems[ EXTREME_INDEX_LEFT ] ) / 2.
          - cSpotShiftRelToFirstInRow ;
        XMidPoint.Add( cMidPoint.real() ) ;
        YMidPoint.Add( cMidPoint.imag() ) ;

        BoundLeft.Add( Extrems[ EXTREME_INDEX_LEFT ].real() - cSpotShiftRelToFirstInRow.real() ) ;
        BoundTop.Add( Extrems[ EXTREME_INDEX_TOP ].imag() - cSpotShiftRelToFirstInRow.imag() ) ;
        BoundRight.Add( Extrems[ EXTREME_INDEX_RIGHT ].real() - cSpotShiftRelToFirstInRow.real() ) ;
        BoundBottom.Add( Extrems[ EXTREME_INDEX_BOTTOM ].imag() - cSpotShiftRelToFirstInRow.imag() ) ;

        BoundWidth.Add( Extrems[ EXTREME_INDEX_RIGHT ].real() - Extrems[ EXTREME_INDEX_LEFT ].real() ) ;
        BoundHeight.Add( Extrems[ EXTREME_INDEX_BOTTOM ].imag() - Extrems[ EXTREME_INDEX_TOP ].imag() ) ;

        if ( CurrentSpot.m_DirE.real() || CurrentSpot.m_DirE.imag() )
          Spacing.Add( CurrentSpot.m_DirE.real() ) ;
        Ruling_lpcm.Add( 10000. / abs( SpotHorAxis.GetMainVector() ) * m_dScale_um_per_pixel ) ;
        Ruling_lpi.Add( 25400. / abs( SpotHorAxis.GetMainVector() ) * m_dScale_um_per_pixel ) ;
        //    double dSquare_pix2 = abs( SpotHorAxis.GetMainVector() ) * abs( SpotVertAxis.GetMainVector() ) ;
        double dSquare_pix2 = abs( SpotHorAxis.GetMainVector() ) * abs( SpotHorAxis.GetMainVector() ) ;
        double dDotPercent = CurrentSpot.m_dArea * 100. / dSquare_pix2 ;
        Dot_Percent.Add( dDotPercent ) ;
      }
//       if (pCurrentNode == SpotHorAxis.GetTail())
//         break;
      pCurrentNode = pCurrentNode->next;
    } while ( pCurrentNode );
  } while ( ++iHorLineCnt < m_CalibGrid.m_HorLines.size() );
  Area_um2.Calculate() ;
  Diameter_um.Calculate() ;
  Perimeter_um.Calculate() ;
  BoxRatio.Calculate() ;
  XCentroid.Calculate() ;
  YCentroid.Calculate() ;
  XMidPoint.Calculate() ;
  YMidPoint.Calculate() ;
  BoundLeft.Calculate() ;
  BoundTop.Calculate() ;
  BoundRight.Calculate() ;
  BoundBottom.Calculate() ;
  BoundWidth.Calculate() ;
  BoundHeight.Calculate() ;
  Spacing.Calculate() ;
  Ruling_lpcm.Calculate() ;
  Ruling_lpi.Calculate() ;
  Dot_Percent.Calculate() ;

  // Second cycle for histogram processing
  iHorLineCnt = 0;
  do
  {
    Axis& SpotHorAxis = *( m_CalibGrid.m_HorLines[ iHorLineCnt ] );
    Node1D * pCurrentNode = SpotHorAxis.GetHead();
    Spot& FirstInRow = *( pCurrentNode->m_Spot );
    do
    {
      Spot& CurrentSpot = *( pCurrentNode->m_Spot );
      size_t iIndex = CurrentSpot.GetGlobalIndex() ;
      if ( (iIndex >= 0) && (iIndex < m_Conturs.size()) )
      {
        Spot& FirstInRow = *( SpotHorAxis.GetHead()->m_Spot ) ;
        int iXIndexDiffToRowBegin = CurrentSpot.m_Indexes.x - FirstInRow.m_Indexes.x ;
        cmplx cSpotShiftRelToFirstInRow = FirstInRow.m_imgCoord
          + SpotHorAxis.GetMainVector() * ( double ) ( iXIndexDiffToRowBegin ) ;
        Area_um2.AddToHistogram( CurrentSpot.m_dArea ) ;
        Diameter_um.AddToHistogram( ( CurrentSpot.m_Sizes.getWidth() + CurrentSpot.m_Sizes.getHeight() ) / 2. ) ;
        Perimeter_um.AddToHistogram( m_Conturs[ iIndex ]->GetConturLength() ) ;
        CmplxArray Extrems ;
        cmplx cSize ;
        cmplx cMid = FindExtrems( m_Conturs[ iIndex ] , Extrems , NULL , &cSize ) ;
        BoxRatio.AddToHistogram( cSize.imag() / cSize.real() ) ;

        cmplx cCentroid = CurrentSpot.m_imgCoord - cSpotShiftRelToFirstInRow ;
        XCentroid.AddToHistogram( cCentroid.real() ) ;
        YCentroid.AddToHistogram( cCentroid.imag() ) ;

        cmplx cMidPoint = ( Extrems[ EXTREME_INDEX_RIGHT ] + Extrems[ EXTREME_INDEX_LEFT ] ) / 2.
          - cSpotShiftRelToFirstInRow ;
        XMidPoint.AddToHistogram( cMidPoint.real() ) ;
        YMidPoint.AddToHistogram( cMidPoint.imag() ) ;

        BoundLeft.AddToHistogram( Extrems[ EXTREME_INDEX_LEFT ].real() - cSpotShiftRelToFirstInRow.real() ) ;
        BoundTop.AddToHistogram( Extrems[ EXTREME_INDEX_TOP ].imag() - cSpotShiftRelToFirstInRow.imag() ) ;
        BoundRight.AddToHistogram( Extrems[ EXTREME_INDEX_RIGHT ].real() - cSpotShiftRelToFirstInRow.real() ) ;
        BoundBottom.AddToHistogram( Extrems[ EXTREME_INDEX_BOTTOM ].imag() - cSpotShiftRelToFirstInRow.imag() ) ;

        BoundWidth.AddToHistogram( Extrems[ EXTREME_INDEX_RIGHT ].real() - Extrems[ EXTREME_INDEX_LEFT ].real() ) ;
        BoundHeight.AddToHistogram( Extrems[ EXTREME_INDEX_BOTTOM ].imag() - Extrems[ EXTREME_INDEX_TOP ].imag() ) ;

        if ( CurrentSpot.m_DirE.real() || CurrentSpot.m_DirE.imag() )
          Spacing.AddToHistogram( CurrentSpot.m_DirE.real() ) ;
        Ruling_lpcm.AddToHistogram( 10000. / abs( SpotHorAxis.GetMainVector() ) * m_dScale_um_per_pixel ) ;
        Ruling_lpi.AddToHistogram( 25400. / abs( SpotHorAxis.GetMainVector() ) * m_dScale_um_per_pixel ) ;
        //    double dSquare_pix2 = abs( SpotHorAxis.GetMainVector() ) * abs( SpotVertAxis.GetMainVector() ) ;
        double dSquare_pix2 = abs( SpotHorAxis.GetMainVector() ) * abs( SpotHorAxis.GetMainVector() ) ;
        double dDotPercent = CurrentSpot.m_dArea * 100. / dSquare_pix2 ;
        Dot_Percent.AddToHistogram( dDotPercent ) ;
      }

      pCurrentNode = pCurrentNode->next;
    } while ( pCurrentNode );
  } while ( ++iHorLineCnt < m_CalibGrid.m_HorLines.size() );

  m_SpotsStatistics.Reset() ;
  m_SpotsStatistics.m_dArea = Area_um2.GetAverage() ;
  m_SpotsStatistics.m_dArea_Std = Area_um2.GetStd() ;
  Area_um2.GetValueForPercentOnHistogram( 75 , 
    m_SpotsStatistics.m_dArea_75percMin , m_SpotsStatistics.m_dArea_75percMax ) ;
  Area_um2.GetValueForPercentOnHistogram( 95 , 
    m_SpotsStatistics.m_dArea_95percMin , m_SpotsStatistics.m_dArea_95percMax ) ;
  m_SpotsStatistics.m_dDia = Diameter_um.GetAverage() ;
  m_SpotsStatistics.m_dDia_Std = Diameter_um.GetStd() ;
  Diameter_um.GetValueForPercentOnHistogram( 75 , 
    m_SpotsStatistics.m_dDia_75percMin , m_SpotsStatistics.m_dDia_75percMax ) ;
  Diameter_um.GetValueForPercentOnHistogram( 95 , 
    m_SpotsStatistics.m_dDia_95percMin , m_SpotsStatistics.m_dDia_95percMax ) ;
  m_SpotsStatistics.m_dPerimeter = Perimeter_um.GetAverage() ;
  m_SpotsStatistics.m_dPerimeter_Std = Perimeter_um.GetStd() ;
  Perimeter_um.GetValueForPercentOnHistogram( 75 , 
    m_SpotsStatistics.m_dPerimeter_75percMin , m_SpotsStatistics.m_dPerimeter_75percMax ) ;
  Perimeter_um.GetValueForPercentOnHistogram( 95 , 
    m_SpotsStatistics.m_dPerimeter_95percMin , m_SpotsStatistics.m_dPerimeter_95percMax ) ;
  m_SpotsStatistics.m_dBoxRatio = BoxRatio.GetAverage() ;
  m_SpotsStatistics.m_dBoxRatio_Std = BoxRatio.GetStd() ;
  BoxRatio.GetValueForPercentOnHistogram( 75 , 
    m_SpotsStatistics.m_dBoxRatio_75percMin , m_SpotsStatistics.m_dBoxRatio_75percMax ) ;
  BoxRatio.GetValueForPercentOnHistogram( 95 , 
    m_SpotsStatistics.m_dBoxRatio_95percMin , m_SpotsStatistics.m_dBoxRatio_95percMax ) ;
  m_SpotsStatistics.m_dXCentroid = XCentroid.GetAverage() ;
  m_SpotsStatistics.m_dXCentroid_Std = XCentroid.GetStd() ;
  XCentroid.GetValueForPercentOnHistogram( 75 , 
    m_SpotsStatistics.m_dXCentroid_75percMin , m_SpotsStatistics.m_dXCentroid_75percMax ) ;
  XCentroid.GetValueForPercentOnHistogram( 95 , 
    m_SpotsStatistics.m_dXCentroid_95percMin , m_SpotsStatistics.m_dXCentroid_95percMax ) ;
  m_SpotsStatistics.m_dYCentroid = YCentroid.GetAverage() ;
  m_SpotsStatistics.m_dYCentroid_Std = YCentroid.GetStd() ;
  YCentroid.GetValueForPercentOnHistogram( 75 , 
    m_SpotsStatistics.m_dYCentroid_75percMin , m_SpotsStatistics.m_dYCentroid_75percMax ) ;
  YCentroid.GetValueForPercentOnHistogram( 95 , 
    m_SpotsStatistics.m_dYCentroid_95percMin , m_SpotsStatistics.m_dYCentroid_95percMax ) ;
  m_SpotsStatistics.m_dXMid = XMidPoint.GetAverage() ;
  m_SpotsStatistics.m_dXMid_Std = XMidPoint.GetStd() ;
  XMidPoint.GetValueForPercentOnHistogram( 75 , 
    m_SpotsStatistics.m_dXMid_75percMin , m_SpotsStatistics.m_dXMid_75percMax ) ;
  XMidPoint.GetValueForPercentOnHistogram( 95 , 
    m_SpotsStatistics.m_dXMid_95percMin , m_SpotsStatistics.m_dXMid_95percMax ) ;
  m_SpotsStatistics.m_dYMid = YMidPoint.GetAverage() ;
  m_SpotsStatistics.m_dYMid_Std = YMidPoint.GetStd() ;
  YMidPoint.GetValueForPercentOnHistogram( 75 , 
    m_SpotsStatistics.m_dYMid_75percMin , m_SpotsStatistics.m_dYMid_75percMax ) ;
  YMidPoint.GetValueForPercentOnHistogram( 95 , 
    m_SpotsStatistics.m_dYMid_95percMin , m_SpotsStatistics.m_dYMid_95percMax ) ;
  m_SpotsStatistics.m_dBoundLeft = BoundLeft.GetAverage() ;
  m_SpotsStatistics.m_dBoundLeft_Std = BoundLeft.GetStd() ;
  m_SpotsStatistics.m_dBoundTop = BoundTop.GetAverage() ;
  m_SpotsStatistics.m_dBoundTop_Std = BoundTop.GetStd() ;
  m_SpotsStatistics.m_dBoundRight = BoundRight.GetAverage() ;
  m_SpotsStatistics.m_dBoundRight_Std = BoundRight.GetStd() ;
  m_SpotsStatistics.m_dBoundBottom = BoundBottom.GetAverage() ;
  m_SpotsStatistics.m_dBoundBottom_Std = BoundBottom.GetStd() ;
  m_SpotsStatistics.m_dBoundWidth = BoundWidth.GetAverage() ;
  m_SpotsStatistics.m_dBoundWidth_Std = BoundWidth.GetStd() ;
  BoundWidth.GetValueForPercentOnHistogram( 75 , 
    m_SpotsStatistics.m_dBoundWidth_75percMin , m_SpotsStatistics.m_dBoundWidth_75percMax ) ;
  BoundWidth.GetValueForPercentOnHistogram( 95 , 
    m_SpotsStatistics.m_dBoundWidth_95percMin , m_SpotsStatistics.m_dBoundWidth_95percMax ) ;
  m_SpotsStatistics.m_dBoundHeight = BoundHeight.GetAverage() ;
  m_SpotsStatistics.m_dBoundHeight_Std = BoundHeight.GetStd() ;
  BoundHeight.GetValueForPercentOnHistogram( 75 ,
    m_SpotsStatistics.m_dBoundHeight_75percMin , m_SpotsStatistics.m_dBoundHeight_75percMax ) ;
  BoundHeight.GetValueForPercentOnHistogram( 95 ,
    m_SpotsStatistics.m_dBoundHeight_95percMin , m_SpotsStatistics.m_dBoundHeight_95percMax ) ;
  m_SpotsStatistics.m_dSpacing = Spacing.GetAverage() ;
  m_SpotsStatistics.m_dSpacing_Std = Spacing.GetStd() ;
  Spacing.GetValueForPercentOnHistogram( 75 , 
    m_SpotsStatistics.m_dSpacing_75percMin , m_SpotsStatistics.m_dSpacing_75percMax ) ;
  Spacing.GetValueForPercentOnHistogram( 95 , 
    m_SpotsStatistics.m_dSpacing_95percMin , m_SpotsStatistics.m_dSpacing_95percMax ) ;
  m_SpotsStatistics.m_dRuling_lpcm = Ruling_lpcm.GetAverage() ;
  m_SpotsStatistics.m_dRuling_lpcm_Std = Ruling_lpcm.GetStd() ;
  Ruling_lpcm.GetValueForPercentOnHistogram( 75 ,
    m_SpotsStatistics.m_dRuling_lpcm_75percMin , m_SpotsStatistics.m_dRuling_lpcm_75percMax ) ;
  Ruling_lpcm.GetValueForPercentOnHistogram( 95 ,
    m_SpotsStatistics.m_dRuling_lpcm_95percMin , m_SpotsStatistics.m_dRuling_lpcm_95percMax ) ;
  m_SpotsStatistics.m_dRuling_lpi = Ruling_lpi.GetAverage() ;
  m_SpotsStatistics.m_dRuling_lpi_Std = Ruling_lpi.GetStd() ;
  Ruling_lpi.GetValueForPercentOnHistogram( 75 , 
    m_SpotsStatistics.m_dRuling_lpi_75percMin , m_SpotsStatistics.m_dRuling_lpi_75percMax ) ;
  Ruling_lpi.GetValueForPercentOnHistogram( 95 , 
    m_SpotsStatistics.m_dRuling_lpi_95percMin , m_SpotsStatistics.m_dRuling_lpi_95percMax ) ;
  m_SpotsStatistics.m_dDotPercent = Dot_Percent.GetAverage() ;
  m_SpotsStatistics.m_dDotPercent_Std = Dot_Percent.GetStd() ;
  Dot_Percent.GetValueForPercentOnHistogram( 75 ,
    m_SpotsStatistics.m_dDotPercent_75percMin , m_SpotsStatistics.m_dDotPercent_75percMax ) ;
  Dot_Percent.GetValueForPercentOnHistogram( 95 ,
    m_SpotsStatistics.m_dDotPercent_95percMin , m_SpotsStatistics.m_dDotPercent_95percMax ) ;

//   if ( m_iMeasurentCount == 0 || m_bGenerateCaption )
//   {
//     ResultString +=  m_SpotsStatistics.GetCaption() ;
// //     ResultString += _T( '\n' ) ;
//     m_bGenerateCaption = FALSE ;
//   }
  // Angle with minus because imaging coordinate system (Y is going down).
  FXString Out = m_SpotsStatistics.ToString( ++m_iMeasurentCount , 
    -RadToDeg( m_CalibGrid.m_HorLines[ 0 ]->GetArg() ) ) ;
  ResultString += Out ;
  m_Conturs.clear() ;
  bRes = true ;
  return bRes ;
}

bool SpotsDataProcessing::DoCalibrationProcess(
  const CDataFrame* pDataFrame , bool bUnconditional )
{
  const CTextFrame * pCalibData = NULL ;
  const CTextFrame * pCenterData = NULL ;

  double dBeginTime = GetHRTickCount() ;
  if ( !bUnconditional && pDataFrame->IsContainer() )
  {
    CFramesIterator * Iterator = pDataFrame->CreateFramesIterator( text ) ;
    if ( Iterator )
    {
      CTextFrame * pNext = ( CTextFrame* ) Iterator->Next() ;

      while ( pNext )
      {
        const CTextFrame * pText = pDataFrame->GetTextFrame() ;
        if ( !_tcsicmp( pText->GetLabel() , _T( "grid" ) ) )
        {
          if ( !pCalibData )
            pCalibData = pNext ;
          else
            ASSERT( 0 ) ; // several frames with calib data
        }
        else if ( !_tcsicmp( pText->GetLabel() , _T( "center" ) ) )
        {
          if ( !pCenterData )
            pCenterData = pNext ;
          else
            ASSERT( 0 ) ; // several frames with Center data
        }
        pNext = ( CTextFrame* ) Iterator->Next() ;
      }
      delete Iterator ;
    }
  }
  else
  {
    const CTextFrame * pText = pDataFrame->GetTextFrame() ;
    if ( bUnconditional || !_tcsicmp( pText->GetLabel() , _T( "grid" ) ) )
      pCalibData = pText ;
    else if ( !_tcsicmp( pText->GetLabel() , _T( "center" ) ) )
      pCenterData = pText ;
  }
  const CVideoFrame * pvf = pDataFrame->GetVideoFrame() ;
  if ( pvf )
  {
    m_CalibGrid.m_ImageSize = CSize( GetWidth( pvf ) , GetHeight( pvf ) ) ;
  }
  if ( pCalibData )
  {
    double dBeginInitTime = GetHRTickCount() ;
    FXString CalibData = pCalibData->GetString() ;
    m_bIsCalibrated = m_CalibGrid.init( CalibData , 0 , 0 , 
      m_CalibGrid.m_iInsertMissed , 0.15 , false );

    if ( m_CalibGrid.m_bIsCalibrated )
      SENDINFO( "HCalibration is OK, Center is (%.2f,%.2f)" , m_CalibGrid.m_CenterSpot.m_imgCoord ) ;
    else
      SENDINFO( "HCalibration FAILED. #Spots=%d" , 
        m_CalibGrid.m_Spots && m_CalibGrid.m_Spots->getCount() ? 
        m_CalibGrid.m_Spots->GetNodeList()->getCnt() : 0 ) ;
  }

  return m_bIsCalibrated ;
}

