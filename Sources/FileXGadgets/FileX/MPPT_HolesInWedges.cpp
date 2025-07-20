#include "stdafx.h"
#include "TwoCamsGadget.h"
#include "fxfc/FXRegistry.h"
#include "imageproc/statistics.h"

void TwoChans::ProcessWedgeFront( const CDataFrame * pDataFrame , CContainerFrame * pMarking )
{
  const CVideoFrame * pVF = pDataFrame->GetVideoFrame( );
  BYTE * pImage = (pVF) ? GetData( pVF ) : NULL ;
  int iWidth = ( pVF ) ? GetWidth( pVF ) : 0 ;
  int iHeight = ( pVF ) ? GetHeight( pVF ) : 0 ;

//   const CFigureFrame * pCavityFig = ( const CFigureFrame * ) GetFrameWithLabel( pDataFrame ,
//     figure , "Contur[wedge_cavity" , WP_Any ) ;
//   if ( !pCavityFig )
//   {
//     m_iLeftCornerCntr = m_iTipCornerCntr = m_iBottomCornerCntr = 0 ;
//     return ;
//   }
//   CmplxVector Extrems ;
//   IntVector   ExtremIndexes ;
//   cmplx cCavitySize ;
//   cmplx cCenter = FindExtrems( pCavityFig , Extrems , &ExtremIndexes , &cCavitySize ) ;
//   if ( cCenter.real() == 0. )
//     return ;

//   double dElectrodeTarget = cCenter.real() /*- m_dElectrodeToCornerHorizDist_um / m_dScale*/ ;
//   cmplx cPt1( dElectrodeTarget , 0. ) , cPt2( dElectrodeTarget , m_LastROI.Height() ) ;
//   pMarking->AddFrame( CreateLineFrame( cPt1 , cPt2 , 0x00ffff ) ) ;
// 

  const CFigureFrame * pLeft = ( const CFigureFrame * ) GetFrameWithLabel( pDataFrame ,
    figure , "electrode_left_0" , WP_Full ) ;
  const CFigureFrame * pRight = ( const CFigureFrame * ) GetFrameWithLabel( pDataFrame ,
    figure , "electrode_right_0" , WP_Full ) ;
  if ( pLeft && pRight )
  {
    cmplx cLeft = CDPointToCmplx( *( pLeft->GetData() ) ) ;
    cmplx cRight = CDPointToCmplx( *( pRight->GetData() ) ) ;
    cmplx cElectrodeCenter = ( cLeft + cRight ) * 0.5 ;
    double dWidth_pix = cRight.real() - cLeft.real();
    pMarking->AddFrame( CreatePtFrame( cElectrodeCenter , "color=0x0000ff;Sz=5;" ) ) ;
    cmplx cUp( cElectrodeCenter.real() , 0. ) , cDown( cElectrodeCenter.real() , m_LastROI.Height() ) ;
    pMarking->AddFrame( CreateLineFrame( cUp , cDown , 0x00ffff ) ) ;

    if ( m_iVertLinesDist_tenth )
    {
      double dLineShift = m_iVertLinesDist_tenth * 2.54 / m_dScale_um_per_pix ;
      cmplx cPt1( cElectrodeCenter.real() - dLineShift , m_LastROI.Height() * 0.7 ) ;
      cmplx cPt2( cElectrodeCenter.real() - dLineShift , m_LastROI.Height() ) ;
      pMarking->AddFrame( CreateLineFrame( cPt1 , cPt2 , 0x00ffff ) ) ;
      cmplx cPt3( cPt1 + 2 * dLineShift ) ;
      cmplx cPt4( cPt2 + 2 * dLineShift ) ;
      pMarking->AddFrame( CreateLineFrame( cPt3 , cPt4 , 0x00ffff ) ) ;
    }

    cmplx cViewPt( 10. , m_LastROI.Height() * 0.97 ) ;
    pMarking->AddFrame( CreateTextFrame( cViewPt , "0x0000ff" ,
      11 , NULL , pDataFrame->GetId() , "Scale %.6f um/pix (%.6f T/pix)   %d   %s" ,
      m_dScale_um_per_pix , m_dScale_um_per_pix / 2.54 ,
      pDataFrame->GetId() , (LPCTSTR)GetTimeAsString_ms() ) ) ;

    cmplx ViewWidthPt(cRight + 100.);
    pMarking->AddFrame(CreateTextFrame(ViewWidthPt, "0x0000ff",
      15, NULL, pDataFrame->GetId(), "Width= %.2fum (%.2fT)",
      dWidth_pix * m_dScale_um_per_pix, dWidth_pix * m_dScale_um_per_pix / 2.54,
      pDataFrame->GetId(), (LPCTSTR)GetTimeAsString_ms()));

  }
  if ( (m_iMoveFragmentFrom_pix >= 0) && (m_iFragmentHeight_pix > 0) )
  {
    if ( pVF )
    {
      int iFrom = m_iMoveFragmentFrom_pix ;
      if ( iFrom + m_iFragmentHeight_pix >= iHeight )
        iFrom = iHeight - m_iFragmentHeight_pix ;
      int iTo = m_iMoveFragmentTo_pix ;
      if ( iTo + m_iFragmentHeight_pix >= iHeight )
        iTo = iHeight - m_iFragmentHeight_pix ;
      memcpy( pImage + ( iWidth * m_iMoveFragmentTo_pix ) ,
        pImage + ( iWidth * m_iMoveFragmentFrom_pix ) , m_iFragmentHeight_pix * iWidth ) ;
    }
  }
  cmplx UpperPt( m_iVertLinePosition_pix , 0. );
  cmplx LowerPt( m_iVertLinePosition_pix , ( double ) iHeight );
  pMarking->AddFrame( CreateLineFrame( UpperPt , LowerPt , 0x00c0c000 ) );

  cmplx LeftPt( 0. , m_iHorLinePosition_pix );
  cmplx RightPt( iWidth , m_iHorLinePosition_pix );
  pMarking->AddFrame( CreateLineFrame( LeftPt , RightPt , 0x00c0c000 ) );

}

void TwoChans::ProcessWedgeSide( const CDataFrame * pDataFrame , CContainerFrame * pMarking )
{
  double dMaxY = -DBL_MAX , dMinY = DBL_MAX ;

  const CRectFrame * pWedgeROI = pDataFrame->GetRectFrame( "ROI:wedge_side" ) ;
  if ( !pWedgeROI )
    return ;
  CRect WedgeROI( *( (RECT*)pWedgeROI ) );
  WedgeROI.DeflateRect( 2 , 2 ) ;
  FXSIZE iIndexOfMaxY = -1 , iIndexOfMinY = -1 ;
  const CFigureFrame * pEdgeFig = (const CFigureFrame * )GetFrameWithLabel( pDataFrame ,
    figure , "Contur[wedge_side" , WP_Any ) ;
  if ( !pEdgeFig )
  {
    m_iLeftCornerCntr = m_iTipCornerCntr = m_iBottomCornerCntr = 0 ;
    return ;
  }

  double dNow_ms = GetHRTickCount() ;

  const cmplx * pcEdge = ( const cmplx* ) pEdgeFig->GetData() ;
  const cmplx * pcEdgeEnd = pcEdge + pEdgeFig->Size() ;

  CmplxVector Extrems ;
  IntVector   ExtremIndexes ;
  cmplx cWedgeSize ;
  cmplx cCenter = FindExtrems( pEdgeFig , Extrems , &ExtremIndexes , &cWedgeSize ) ;
  if ( cCenter.real() == 0. )
    return ;

  cmplx cLeftCorner = Extrems[ EXTREME_INDEX_LEFT ] ;
  pMarking->AddFrame( CreatePtFrame( Extrems[ EXTREME_INDEX_LEFT ] , "color=0x0000ff;Sz=3;" ) ) ;
  pMarking->AddFrame( CreatePtFrame( Extrems[ EXTREME_INDEX_BOTTOM ] , "color=0x0000ff;Sz=3;" ) ) ;
  pMarking->AddFrame( CreatePtFrame( Extrems[ EXTREME_INDEX_TOP ] , "color=0x0000ff;Sz=3;" ) ) ;
  pMarking->AddFrame( CreatePtFrame( Extrems[ EXTREME_INDEX_RIGHT ] , "color=0x0000ff;Sz=3;" ) ) ;

  double dDeviation = 0. ;
  for ( size_t i = 0 ; i < m_LastLeftCorners.size() ; i++ )
  {
    double dDist = abs( cLeftCorner - m_LastLeftCorners[ i ] ) ;
    if ( dDist > dDeviation )
      dDeviation = dDist ;
  }
  m_LastLeftCorners.push_back( cLeftCorner ) ;
#define CHECK_LENGTH 16
  if ( m_LastLeftCorners.size() > CHECK_LENGTH )
    m_LastLeftCorners.erase( m_LastLeftCorners.begin() ) ;

  if ( dDeviation > 12. )
    m_dShutDownProcessingTime = dNow_ms ;

    cmplx cViewText( 30. , cLeftCorner.imag() ) ;
    pMarking->AddFrame( CreateTextFrame( cViewText , "0x0000ff" ,
      12 , NULL , 0 , "Deviation=%.1f" , dDeviation ) ) ;

  // 
//   int iMin = 255 , iMax = 0 ;
//   const CVideoFrame * pVF = pDataFrame->GetVideoFrame() ;
// 
//   cmplx cLeftDown( cLeftCorner + cmplx( -100. , 100. ) ) ;
//   cmplx cUp( cLeftCorner + cmplx( -30. , -100 ) ) ;
//   double dAverLeft = 0. , dAverUp = 1000. ;  
//   if ( pVF )
//   {
//     dAverLeft = _calc_average( pVF , cLeftDown , 20 ) ;
//     dAverUp = _calc_average( pVF , cUp , 20 ) ;
//     pMarking->AddFrame( CreateFigureFrameCR( cLeftDown , 20 , "0x0000ff" ) ) ;
//     pMarking->AddFrame( CreateFigureFrameCR( cUp , 20 , "0x0000ff" ) ) ;
//     cmplx cViewText( 30. , cLeftCorner.imag() ) ;
//     pMarking->AddFrame( CreateTextFrame( cViewText , "0x0000ff" ,
//       12 , NULL , 0 , "AverUp=%.1f\nAverLD=%.1f" , dAverUp , dAverLeft ) ) ;
//     if ( (fabs( dAverLeft - dAverUp ) > 20) )
//       m_dShutDownProcessingTime = dNow_ms ;
//   }

  if ( dNow_ms > m_dShutDownProcessingTime + 1000. )
  {
    const cmplx * pcSlopePt = pcEdge + ExtremIndexes[ EXTREME_INDEX_LEFT ] ;
    size_t iOffset = ((pEdgeFig->Size() - ExtremIndexes[ EXTREME_INDEX_LEFT ]) / 20) ; // 5% from begin

    CFRegression WedgeUpperSlope ;
    WedgeUpperSlope.AddPtsToRegression( pcEdge , (int) pEdgeFig->Size() ,
      ExtremIndexes[ EXTREME_INDEX_LEFT ] + (int) iOffset , (int) (pEdgeFig->Size() - iOffset) ) ;
    WedgeUpperSlope.Calculate() ;
    double dWedgeAngle = WedgeUpperSlope.GetAngle() ;
    cmplx cViewPt( m_LastROI.Width() * 0.5 , m_LastROI.Height() * 0.6 ) ;

    pMarking->AddFrame( CreateTextFrame( cViewPt , "0x00ffff" ,
      20 , NULL , pDataFrame->GetId() , "Wedge Angle=%.2f" ,
      180. + RadToDeg( dWedgeAngle ) ) );

    cmplx cXMin = Extrems[ EXTREME_INDEX_LEFT ] ;
    cmplx cXMinAv = SlidingWindow( cXMin , m_cAvLeftCorner , m_iLeftCornerCntr , m_iWindowWidth ) ;
    pMarking->AddFrame( CreatePtFrame( cXMinAv , "color=0xff00ff;Sz=3;" ) ) ;

    cmplx cBottomCorner = Extrems[ EXTREME_INDEX_BOTTOM ] ;
    cmplx cBottomCornerAv = SlidingWindow( cBottomCorner , m_cAvBottomCorner , m_iBottomCornerCntr , m_iWindowWidth ) ;
    pMarking->AddFrame( CreatePtFrame( cBottomCornerAv , "color=0xff00ff;Sz=5;" ) ) ;
    //   cmplx cBCavMarking( cBottomCornerAv + cmplx( -5. , -40. ) ) ;
    //   pMarking->AddFrame( CreateTextFrame( cBCavMarking , "0xff00ff" ,
    //     20 , NULL , pDataFrame->GetId() , "A") );



    int iCornerOnTipIndex = -1 ;
    double dFarestDist = 0. ;
    cmplx cCornerOnTip = FindFarthestPtFromFigSegment(
      ExtremIndexes[ EXTREME_INDEX_BOTTOM ] , ExtremIndexes[ EXTREME_INDEX_LEFT ] ,
      pEdgeFig , iCornerOnTipIndex , dFarestDist ) ;
    cmplx cTipCornerAv = SlidingWindow( cCornerOnTip , m_cAvTipCorner , m_iTipCornerCntr , m_iWindowWidth ) ;

    pMarking->AddFrame( CreatePtFrame( cTipCornerAv , "color=0xff00ff;Sz=3;" ) ) ;

    const cmplx * pFindRightVertEdge = pcEdge + ExtremIndexes[ EXTREME_INDEX_BOTTOM ] - 1 ;
    double dVertEdge = WedgeROI.right - 2. ;
    while ( (--pFindRightVertEdge > pcEdge) && (pFindRightVertEdge->real() < dVertEdge) ) ;

    if ( pFindRightVertEdge == pcEdge )
      return ;

    pMarking->AddFrame( CreatePtFrame( *pFindRightVertEdge , "color=0xff00ff;Sz=3;" ) ) ;

    int iCornerOnBottomRightIndex = -1 ;
    cmplx cBottomRightCorner = FindFarthestPtFromFigSegment(
      (int) (pFindRightVertEdge - pcEdge) , ExtremIndexes[ EXTREME_INDEX_BOTTOM ] ,
      pEdgeFig , iCornerOnBottomRightIndex , dFarestDist ) ;
    cmplx cBottomRightCornerAv = SlidingWindow( cBottomRightCorner ,
      m_cAvBottomRightCorner , m_iBottomRightCornerCntr , m_iWindowWidth ) ;
    cmplx cLowerOnRightEdge = CDPointToCmplx( pEdgeFig->GetAt( pFindRightVertEdge - pcEdge ) ) ;

    pMarking->AddFrame( CreatePtFrame( cBottomRightCornerAv , "color=0xff00ff;Sz=3;" ) ) ;

    CLine2d BottomLineToVertEdge( cBottomRightCorner , cLowerOnRightEdge ) ; // segment to right edge on bottom
    int iIndexOfRightPoint = iCornerOnBottomRightIndex
      + ROUND( (ExtremIndexes[ EXTREME_INDEX_BOTTOM ] - iCornerOnBottomRightIndex) * 0.8 ) ;
    cmplx cPtForBottomRightSegment = CDPointToCmplx( pEdgeFig->GetAt( iIndexOfRightPoint ) ) ;
    CLine2d BottomRightLine( cPtForBottomRightSegment , cBottomRightCorner ) ;
    int iIndexOfLeftPoint = iCornerOnTipIndex -
      ROUND( (iCornerOnTipIndex - ExtremIndexes[ EXTREME_INDEX_BOTTOM ]) * 0.8 ) ;
    cmplx cPtForBottomLeftSegment = CDPointToCmplx( pEdgeFig->GetAt( iIndexOfLeftPoint ) ) ;
    CLine2d BottomLeftLine( cCornerOnTip , cPtForBottomLeftSegment ) ;

    cmplx cPtB , cPtC ;
    bool bResB = BottomLeftLine.intersect( BottomRightLine , cPtB ) ;
    bool bResC = BottomLeftLine.intersect( BottomLineToVertEdge , cPtC ) ;

    cmplx cBottomSideVect = cBottomRightCorner - cLowerOnRightEdge ;

    cmplx cOrthoVector = GetOrthoRightOnVF( cBottomSideVect ) ;
    CLine2d OrthoLineThroughLowerOnRightEdge ;
    OrthoLineThroughLowerOnRightEdge.ByPointAndDir( cLowerOnRightEdge , cOrthoVector ) ;
    cmplx cSecondPtOnOrtho = cLowerOnRightEdge + cOrthoVector ;
    pMarking->AddFrame( CreateLineFrame( cSecondPtOnOrtho , cLowerOnRightEdge , "color=0x00ffff;" ) ) ;

    int iFarFromOrthoPt = -1 ;
    cmplx cFarestFromOrtho = OrthoLineThroughLowerOnRightEdge.GetFarestPtOfSegment(
      ExtremIndexes[ EXTREME_INDEX_BOTTOM ] , ExtremIndexes[ EXTREME_INDEX_LEFT ] ,
      (const cmplx*) (pEdgeFig->GetData()) , iCornerOnBottomRightIndex , dFarestDist ) ;
    pMarking->AddFrame( CreatePtFrame( cFarestFromOrtho , "color=0xffffff;Sz=5;" ) ) ;
    m_dElectrodeToCornerHorizDist_tenth = m_iShiftFromCorner_Tx10 * 0.1 ;
    double dECornerDist_um = m_dElectrodeToCornerHorizDist_tenth * 2.54 ;
    double dElectrodeTarget_pix = 100. ;
    cmplx cCrossPt ;
    if ( BottomLineToVertEdge.GetOrthoCross( cFarestFromOrtho , cCrossPt ) )
    {
      pMarking->AddFrame( CreateLineFrame( cLowerOnRightEdge , cCrossPt , 0x00ffff ) ) ;
      CLine2d FLine( cFarestFromOrtho , cCrossPt ) ;
      cmplx NearestToLowerCorner = FLine.GetNearestPt( cBottomCornerAv ) ;
      pMarking->AddFrame( CreateLineFrame( NearestToLowerCorner , cBottomCornerAv , 0xff0000 ) ) ;

      cmplx cOrthoVect( cFarestFromOrtho - cCrossPt ) ;
      cmplx cNormOrtho( GetNormalized( cOrthoVect ) ) ;

      cmplx cElectrodeTarget = NearestToLowerCorner + cNormOrtho * (dECornerDist_um / m_dScale_um_per_pix) ;
      cmplx cElectrodeTargetAv = SlidingWindow( cElectrodeTarget ,
        m_cAvElectrodeTarget , m_iElectrodeTargetCntr , m_iWindowWidth ) ;
      pMarking->AddFrame( CreateLineFrame( cElectrodeTargetAv , cCrossPt , 0x00ffff ) ) ;
      pMarking->AddFrame( CreatePtFrame( cElectrodeTargetAv , "color=0xffffff;Sz=5;" ) ) ;

      dElectrodeTarget_pix = cElectrodeTargetAv.real() ;
      CFigureFrame * pFLine = CreateLineFrame( cElectrodeTargetAv , NearestToLowerCorner , 0x0000ff ) ;
      pFLine->Attributes()->WriteInt( "thickness" , 3 ) ;
      pMarking->AddFrame( pFLine ) ;
    }

    //   if ( bResB )
    //   {
    //     pMarking->AddFrame( CreatePtFrame( cPtB , "color=0x000000;Sz=5;" ) ) ;
    //     pMarking->AddFrame( CreateLineFrame( cPtB , cBottomRightCorner , "color=0x00ffff;Sz=5;" ) ) ;
    //     cmplx cBMarking( cPtB + cmplx( -40. , -5. ) ) ;
    //     pMarking->AddFrame( CreateTextFrame( cBMarking , "0xff00ff" ,
    //       14 , NULL , pDataFrame->GetId() , "B" ) );
    //   }
    //   if ( bResC )
    //   {
    //     pMarking->AddFrame( CreatePtFrame( cPtC , "color=0xff0000;Sz=5;" ) ) ;
    //     pMarking->AddFrame( CreateLineFrame( cPtC , cBottomRightCorner , "color=0xff0000;Sz=5;" ) ) ;
    //     pMarking->AddFrame( CreateLineFrame( cPtC , cCornerOnTip , "color=0xff0000;Sz=5;" ) ) ;
    //     cmplx cBMarking( cPtC + cmplx( 30. , 0. ) ) ;
    //     pMarking->AddFrame( CreateTextFrame( cBMarking , "0xff0000" ,
    //       14 , NULL , pDataFrame->GetId() , "C" ) );
    //   }

    else
    {
      dElectrodeTarget_pix = cBottomCornerAv.real()
        - ((dECornerDist_um / sqrt( 2. )) / m_dScale_um_per_pix) ;
    }

    cmplx cPt1( dElectrodeTarget_pix , 0. ) , cPt2( dElectrodeTarget_pix , m_LastROI.Height() ) ;
    pMarking->AddFrame( CreateLineFrame( cPt1 , cPt2 , 0x0000ff ) ) ;


    const CFigureFrame * pRight = (const CFigureFrame *) GetFrameWithLabel( pDataFrame ,
      figure , "electrode_right_0" , WP_Full ) ;
    if ( pRight )
    {
      cmplx cRight = CDPointToCmplx( *(pRight->GetData()) ) ;
      pMarking->AddFrame( CreatePtFrame( cRight , "color=0xff0000;Sz=5;" ) ) ;

      double dShift = (cRight.real() - dElectrodeTarget_pix) * m_dScale_um_per_pix ;

      cmplx cViewPt( 100. , m_LastROI.Height() * 0.27 ) ;
      pMarking->AddFrame( CreateTextFrame( cViewPt , fabs( dShift ) > 2.54 ? "0x0000ff" : "0x00ff00" ,
        20 , NULL , pDataFrame->GetId() , "Shift %s for %.2fT (%.2fum)" ,
        dShift > 0. ? "Left" : "Right" , fabs( dShift / 2.54 ) , fabs( dShift ) ) ) ;

      const CFigureFrame * pLeft = (const CFigureFrame *)GetFrameWithLabel(pDataFrame,
        figure, "electrode_left", WP_Begin );
      if ( pLeft )
      {
        cmplx cLeft = CDPointToCmplx(*(pLeft->GetData()));
        double dWidth_pix = cRight.real() - cLeft.real();
        cmplx ViewWidthPt(cRight + 20.);
        pMarking->AddFrame(CreateTextFrame(ViewWidthPt, "0x0000ff",
          15, NULL, pDataFrame->GetId(), "Width= %.2fum (%.2fT)",
          dWidth_pix * m_dScale_um_per_pix, dWidth_pix * m_dScale_um_per_pix / 2.54,
          pDataFrame->GetId(), (LPCTSTR)GetTimeAsString_ms()));
      }

    }
    cmplx cViewPtInfo( 10. , m_LastROI.Height() * 0.97 ) ;
    pMarking->AddFrame( CreateTextFrame( cViewPtInfo , "0x0000ff" ,
      11 , NULL , pDataFrame->GetId() , "Scale %.6f um/pix (%.6f T/pix)   %d   %s" ,
      m_dScale_um_per_pix , m_dScale_um_per_pix / 2.54 ,
      pDataFrame->GetId() , (LPCTSTR) GetTimeAsString_ms() ) ) ;
  }
}

void TwoChans::ProcessForHolesInWedges( const CDataFrame * pDataFrame , CContainerFrame * pMarking )
{
  switch ( m_GadgetMode )
  {
  case GM_SIDE: ProcessWedgeSide( pDataFrame , pMarking ) ; break ;
  case GM_FRONT: ProcessWedgeFront( pDataFrame , pMarking ) ; break ;
  }
}


