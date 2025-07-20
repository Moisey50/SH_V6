// ConturAsm.h : Drawing and on screen selection methods implementation  of the ConturAsm class


#include "StdAfx.h"
#include "ConturAsm.h"
#include <imageproc\clusters\segmentation.h>
#include <imageproc\utilities.h>
#include <imageproc/simpleip.h>
#include <gadgets\vftempl.h>
#include <helpers\propertykitEx.h>
#include <helpers\FramesHelper.h>
#include "ConturData.h"
#include <files\imgfiles.h>

#define THIS_MODULENAME "ConturAsm"

static cmplx cManualShift ;

cmplx ConturAsm::FormMarker(int iIndexInConturData, DWORD dwColor, 
  bool bViewValue, CContainerFrame * pMarking , int iSaveEnds )
{
  ConturSample& Selected = m_ConturData.GetAt(iIndexInConturData);
  cmplx cPt1, cPt2 , Ortho , cSelected ;
  if ( (iSaveEnds == 1) || (iSaveEnds == 2) )
  {
    CPoint SelPt = Selected.m_PointOnMap;
    int iSelPlus = (m_iSelectedPtIndex + 5) % m_ConturData.Count();
    FXSIZE iSelMinus = (m_iSelectedPtIndex - 5 + m_ConturData.Count()) % m_ConturData.Count();
    CPoint SelPlus = m_ConturData[iSelPlus].m_PointOnMap;
    CPoint SelMinus = m_ConturData[iSelMinus].m_PointOnMap;
    cSelected = cmplx(SelPt.x, SelPt.y);
    cmplx cSelPLus(SelPlus.x, SelPlus.y);
    cmplx cSelMinus(SelMinus.x, SelMinus.y);

    cmplx Vect = cSelPLus - cSelMinus;
    Ortho = GetNormalized(-GetOrthoLeft(Vect));
    cPt1 = cSelected;
    cPt2 = cPt1 + Ortho * 30.;
    CFigureFrame * pPointer = CreateLineFrame(cPt1, cPt2, dwColor, "Pointer");
    pMarking->AddFrame(pPointer);
    if ( iSaveEnds == 1 ) // one of points
    {
      m_LastPointer[0] = cPt1;
      m_LastPointer[1] = cPt2;
    }
    else if ( iSaveEnds == 2 )
    {
      m_MaxGradPtr[0] = cPt1;
      m_MaxGradPtr[1] = cPt2;
    }
  }
  else if ( iSaveEnds == 3 ) // use old points for max grad
  {
    cPt1 = m_MaxGradPtr[0];
    cPt2 = m_MaxGradPtr[1];
    Ortho = GetNormalized(cPt2 - cPt1);
  }
  CFigureFrame * pPointer = CreateLineFrame(cPt1, cPt2, dwColor, "Pointer");
  pMarking->AddFrame(pPointer);
   ;
  if ( bViewValue )
  {
    cmplx cSelectedViewPt = cPt1 + Ortho * 50.;
    double dAngle = arg(Ortho);
    if (dAngle > M_PI_2)
      cSelectedViewPt += cmplx(-30, 15);
    else if (dAngle < -M_PI_2)
      cSelectedViewPt += cmplx(-10.,-30);

    FXString Color;
    Color.Format("0x%X", dwColor);
    pMarking->AddFrame(CreateTextFrame(cSelectedViewPt, (LPCTSTR)Color , 14,
      NULL, 0, "%.1f", Selected.m_dAveBurrWidth_um));
  }
  return cSelected; // return point on map
}


CDataFrame * ConturAsm::FormMapImage(
  const CVideoFrame * pImage , ResultViewMode Mode ,
  int iViewIndex , int iPercent )
{
  if ( !m_ConturData.GetCount() )
    return NULL ;

  CVideoFrame * pMapViewImage = CVideoFrame::Create() ;
  if ( !pMapViewImage )
  {
    ASSERT( 0 ) ;
    return NULL ;
  }

  int iSize = 800 * 600 * 3 ; // RGB BMP for map 
  int iFullSize = iSize + sizeof( BITMAPINFOHEADER ) ;
  pMapViewImage->lpBMIH = (LPBITMAPINFOHEADER) malloc( iFullSize ) ;
  if ( !pMapViewImage->lpBMIH )
  {
    pMapViewImage->Release() ;
    ASSERT( 0 ) ;
    return NULL ;
  }

  pMapViewImage->lpData = NULL ;
  LPBYTE pData = (LPBYTE) &pMapViewImage->lpBMIH[ 1 ] ;
  memset( pMapViewImage->lpBMIH , 0 , iFullSize ) ;
  pMapViewImage->lpBMIH->biSize = sizeof( BITMAPINFOHEADER ) ;
  pMapViewImage->lpBMIH->biSizeImage = iSize ;
  pMapViewImage->lpBMIH->biBitCount = 24 ;
  pMapViewImage->lpBMIH->biCompression = BI_RGB ;
  int iWidth = pMapViewImage->lpBMIH->biWidth = 800 ;
  int iHeight = pMapViewImage->lpBMIH->biHeight = 600 ;
  int iMapWidth = 600 ; // map is RGB BMP 600x600 pixels
  pMapViewImage->lpBMIH->biClrImportant =
    pMapViewImage->lpBMIH->biClrUsed = pMapViewImage->lpBMIH->biXPelsPerMeter =
    pMapViewImage->lpBMIH->biYPelsPerMeter = 0 ;
  pMapViewImage->lpBMIH->biPlanes = 1 ;
  double dMaxValue = m_dMaximumsForViewCsale[ (int) Mode ] ;
  FormScaleView( pMapViewImage , m_ScaleBarPos , dMaxValue , 10./dMaxValue ) ;

  double dXRange = 1. ;
  double dYRange = 1. ;
  double dViewScale = 1. ;
  cmplx cLeftPtOnView ;
  cmplx cExtrToCent_um ;
  ConturSample& LeftPt = m_ConturData.GetAt( 0 ) ;

  cmplx cMapImgCent( iMapWidth / 2. , iHeight / 2. ) ; // we will use for image only left square
  m_cMapImageCenter = cMapImgCent ;
  if ( m_pPartConturOnObservation )
  {
    cmplx cShift = (m_cPartCenterOnObservation - m_cObservFOVCenter) ;
    cmplx cShiftedCent = m_cMapImageCenter /*+ cShift*/ ;
    dXRange = m_ObservationExtremes[ 2 ].real() - m_ObservationExtremes[ 0 ].real() ;
    dYRange = m_ObservationExtremes[ 3 ].imag() - m_ObservationExtremes[ 1 ].imag() ;
    cmplx cRange( dXRange , dYRange ) ;
    cmplx cRange_um = cRange / m_dObservScale_pix_per_um ; // Range in um
    cmplx cObservCent(
      m_ObservationExtremes[ 2 ].real() + m_ObservationExtremes[ 0 ].real() ,
      m_ObservationExtremes[ 3 ].imag() + m_ObservationExtremes[ 1 ].imag() ) ;
    cObservCent *= 0.5 ;
#define CLEARANCE 0.1
    double dActiveCoeff = 1. + CLEARANCE * 2. ;
    cmplx cActiveRange = cRange_um * dActiveCoeff ; // spare spaces on sides
    double dMax = max( cActiveRange.real() , cActiveRange.imag() ) ;
    // picture square 600x600 on left side, scale is on right side
    dViewScale = dMax / iHeight ;
    m_dMapScale_pix_per_um = iHeight / dMax ;

    cmplx cObservViewCent = (m_cMapImageCenter / 4.) ; // left upper corner
    cmplx cLeftOnObserv = CDP2Cmplx( m_pPartConturOnObservation->GetAt( 0 ) ) ;

    for ( int i = 0 ; i < m_pPartConturOnObservation->GetCount() ; i++ )
    {
      cmplx cNext = CDP2Cmplx( m_pPartConturOnObservation->GetAt( i ) ) ;
      cmplx cPtToCentVect = cNext - cObservCent ;
      cmplx cScaledPtToCent = cPtToCentVect / m_dObservScale_pix_per_um ;
      cmplx cScaledPtToCentOnView = cScaledPtToCent / (dViewScale * 4);
      cmplx cOnView = cObservViewCent + cScaledPtToCentOnView ;
      LPBYTE pOnView = GetPixel24Ptr( pData , cOnView , iWidth , iHeight ) ;
      if ( (pData <= pOnView) && (pOnView < pData + iSize) )
      {
        *(pOnView++) = 50 ;
        *(pOnView++) = 50 ;
        *(pOnView++) = 50 ;
      }
    }
    cmplx cExtremToCenterOnObserv =
      m_ObservationExtremes[ 0 ] - cObservCent ;
    double dRatio = dYRange / dXRange ;
    if ( dRatio < 1. )
    {
      double dYActiveRange = iHeight * dRatio / dActiveCoeff ;
      double dScale = dYActiveRange / dYRange ;
      double dYOnObserv = m_ObservationExtremes[ 0 ].imag() - cObservCent.imag() ;
      double dYPos = (iHeight / 2.) + dYOnObserv * dScale ;
      cLeftPtOnView = cmplx( iMapWidth * CLEARANCE , dYPos ) ;
    }
    else
    {
      double dYOfLeftToUpper =
        (m_ObservationExtremes[ 0 ].imag() - m_ObservationExtremes[ 1 ].imag()) ;
      double dYRatio = dYOfLeftToUpper / dYRange ;
      double dYPos = (iHeight * CLEARANCE)
        + (iHeight / (1. + CLEARANCE * 2)) * dYRatio ;
      double dRealWidth = iMapWidth / (dActiveCoeff * dRatio);
      double dLeftShift = (iMapWidth - dRealWidth) * 0.5;
      double dXPos = (iMapWidth * CLEARANCE) + dLeftShift;
      cLeftPtOnView = cmplx( dXPos , dYPos ) ;
    }
    cExtrToCent_um = cExtremToCenterOnObserv / m_dObservScale_pix_per_um ;
  }
  else // when we did download data from file (no observation contur)
  {
    dXRange = m_MeasurementExtremes[ 2 ].real() - m_MeasurementExtremes[ 0 ].real() ;
    dYRange = m_MeasurementExtremes[ 3 ].imag() - m_MeasurementExtremes[ 1 ].imag() ;
    cmplx cRange_um( dXRange , dYRange ) ;
    cmplx cPartCenter(
      m_MeasurementExtremes[ 2 ].real() + m_MeasurementExtremes[ 0 ].real() ,
      m_MeasurementExtremes[ 3 ].imag() + m_MeasurementExtremes[ 1 ].imag() ) ;
    cPartCenter *= 0.5 ;
#define CLEARANCE 0.1
    double dActiveCoeff = 1. + CLEARANCE * 2. ;
    cmplx cActiveRange = cRange_um * dActiveCoeff ; // spare spaces on sides
    double dMax = max( cActiveRange.real() , cActiveRange.imag() ) ;
    // picture square 600x600, scale is on right side
    dViewScale = dMax / iHeight ;

    cmplx cExtremToCenterOnMeas =
      m_MeasurementExtremes[ 0 ] - cPartCenter ;
    cExtrToCent_um = cExtremToCenterOnMeas /*/ m_dScale_pix_per_um*/ ;
    double dRatio = dYRange / dXRange ;
    if ( dRatio < 1. )
    {
      double dYActiveRange = iHeight * dRatio / dActiveCoeff ;
      double dScale = dYActiveRange / dYRange ;
      double dYOnMeas = m_MeasurementExtremes[ 0 ].imag() - cPartCenter.imag() ;
      double dYPos = (iHeight / 2.) + dYOnMeas * dScale ;
      cLeftPtOnView = cmplx( iMapWidth * CLEARANCE , dYPos ) ;
    }
    else
    {
      double dYOfLeftToUpper =
        (m_MeasurementExtremes[ 0 ].imag() - m_MeasurementExtremes[ 1 ].imag()) ;
      double dYRatio = dYOfLeftToUpper / dYRange ;
      double dYPos = (iHeight * CLEARANCE)
        + (iHeight / (1. + CLEARANCE * 2)) * dYRatio ;
      double dRealWidth = iMapWidth / (dActiveCoeff * dRatio);
      double dLeftShift = (iMapWidth - dRealWidth) * 0.5;
      double dXPos = (iMapWidth * CLEARANCE) + dLeftShift;
      cLeftPtOnView = cmplx( dXPos , dYPos ) ;
    }
  }

  BYTE bBlue = 0 ;
  cmplx cInitialShift ;
  double dValue = 0. ;

  cmplx cExtrToCent_pix = cExtrToCent_um / dViewScale ;
  cmplx cShiftedCenter_pix = m_cPartCenterOnMap = cLeftPtOnView - cExtrToCent_pix ;
  int iMinX = INT_MAX, iMaxX = -INT_MAX, iMinY = INT_MAX, iMaxY = -INT_MAX;

  for ( int i = 0; i < m_ConturData.GetCount() ; i++ )
  {
    cmplx cNext = (cmplx) m_ConturData[ i ].m_MiddlePt ;
    cmplx cRelToLeftPt = cNext - LeftPt.m_MiddlePt ; ;
    cmplx cScaledPtToLeftPtOnView = cRelToLeftPt / dViewScale ; // in pixels
    cmplx cOnViewSh = cLeftPtOnView - cScaledPtToLeftPtOnView ;
    CPoint OnMap = m_ConturData[i].m_PointOnMap = CPoint(
      ROUND( cOnViewSh.real() ) , ROUND( cOnViewSh.imag() ) )  ;
    SetMinMax(OnMap.x, iMinX, iMaxX);
    SetMinMax(OnMap.y, iMinY, iMaxY);

    GetDataForIndex( Mode , i , dValue ) ;


    DWORD dwColor ;

    switch ( m_ConturData[ i ].m_iBadEdge )
    {
    case EQ_Defects: dwColor = 0x0000ff ; break ;
    case EQ_Invisible: dwColor = 0xffffff ; break ;
    default: dwColor = GetMarkColor( dValue , dMaxValue , 0. ) ;
      break ;
    }

    LPBYTE pOnView = GetPixel24Ptr( pData , cOnViewSh , iWidth , iHeight ) ;
    LPBYTE pSaved = pOnView ;
    if ( (pData <= pOnView) && (pOnView < (pData + iSize - iWidth * 12)) )
    {
      *(pOnView) = *((LPBYTE) &dwColor) ;
      *(pOnView + 1) = *(((LPBYTE) &dwColor) + 1) ;
      *(pOnView + 2) = *(((LPBYTE) &dwColor) + 2) ;
    }

    if ( m_ProcessingStage == Stage_ScanFinished
      || IsInactive() )
    {
      cmplx cVect = cOnViewSh - cMapImgCent ;
      if ( m_CurrentDetailedForm.IsInSector( cVect ) )
      {
        cmplx cSectorMarkRel = cVect * 1.05 ;
        cmplx cSectorMarkAbs = cSectorMarkRel + cMapImgCent ;
        LPBYTE pOnView = GetPixel24Ptr( pData , cSectorMarkAbs , iWidth , iHeight ) ;
        LPBYTE pSaved = pOnView ;
        if ( (pData <= pOnView) && (pOnView < (pData + iSize - iWidth * 12)) )
        {
          *(pOnView) = 0xc0 ;
          *(pOnView + 1) = 0xc0 ;
          *(pOnView + 2) = 0xc0 ;
        }

      }
    }
  }

  m_cPartOnMapCenterByExtrems = cmplx((iMinX + iMaxX) / 2., (iMinY + iMaxY) / 2.);

  m_LastScaleCaptionText.Format( _T( "%s\n    Max=%.1f" ) ,
    GetScaleCaption( Mode ) , m_dMaximumsForViewCsale[ (int) Mode ] ) ;
  m_LastScaleCaptionPt = CPoint( 600 , 3 ) ;
  if ( iViewIndex >= 0 && iViewIndex < m_ConturData.GetCount() )
  {
    ConturSample& Sample = m_ConturData.GetAt( iViewIndex ) ;
    if ( Sample.m_Algorithm == CS_Wall )
    {
      m_LastInfoAboutGrad.Format(
        _T( "#%d Wav=%6.2fum Wmax=%6.2fum\n"
        "Hav=%6.2fum Hmax=%6.2fum\n"
        "L=%6.2fum V=%6.0fqum\n"
        "Alg=Wall" ) , iViewIndex ,
        Sample.m_dAveBurrWidth_um , Sample.m_dMaxBurrWidth_um ,
        Sample.m_dAveBurrHeight_um , Sample.m_dMaxBurrHeight_um ,
        Sample.m_dEdgeWithBurrLen_um , Sample.m_dBurrVolume_qum ) ;

    }
    else
    {
      m_LastInfoAboutGrad.Format(
        _T( "#%d Wpt=%.1fum Wav=%.1fum\n Wmax=%.1fum "
        "Stat=%s" ) , iViewIndex ,
        Sample.m_dAveBurrWidth_um , GetAverageOfSegmentForPt( iViewIndex ) ,
        Sample.m_dMaxBurrWidth_um ,
        Sample.m_iBadEdge == EQ_Normal ? "Normal" :
        Sample.m_iBadEdge == EQ_Defects ? "Defect" : "Invisible"
      ) ;
    }
    m_LastInfoAboutGradPt.y = (Sample.m_PointOnMap.y > iHeight / 2) ? 30 : (iHeight * 2) / 3 ;
    m_LastInfoAboutGradPt.y = 10 ;
  }

  if ( m_ProcessingStage == Stage_ScanFinished )
  {
    m_iSelectedPtIndex = iViewIndex;
    m_iMaxGradPtIndex = iViewIndex;
    m_bFormMaxGradMarker = true;
    CalcAveragesForRegions( m_AverData ) ;
    
    FXString MapFileName = (m_ResultDirectory
      + m_PartFile + _T( '_' )) + m_MeasurementTS + _T( "_Map.bmp" ) ;
    saveSH2BMP( (LPCTSTR) MapFileName , pMapViewImage->lpBMIH ) ;
  }


  CContainerFrame * pCont = CContainerFrame::Create() ;
  if ( pCont )
  {
    pCont->SetLabel( "MapImage" );
    pCont->AddFrame( pMapViewImage ) ;

    DrawMouseModeSelectMenu( pCont , CPoint( 5 , 400 ) ) ;

    CTextFrame * pViewScale = CreateTextFrame( (LPCTSTR) m_LastScaleCaptionText , _T( "OneSampleAsText" ) ) ;
    pViewScale->Attributes()->Format( _T( "x=%d;y=%d;Sz=11;color=0xffffff;" ) ,
      m_LastScaleCaptionPt.x , m_LastScaleCaptionPt.y );
    pCont->AddFrame( pViewScale ) ;
    if ( iViewIndex >= 0 && iViewIndex < m_ConturData.GetCount() )
    {
      ConturSample& Sample = m_ConturData.GetAt( iViewIndex ) ;
      if ( iPercent != -2 )
      {
        CTextFrame * pViewData = CreateTextFrame( (LPCTSTR) m_LastInfoAboutGrad , _T( "OneSampleAsText" ) ) ;
        pViewData->Attributes()->Format( _T( "x=%d;y=%d;Sz=11;color=0xffffff;" ) ,
          m_LastInfoAboutGradPt.x , m_LastInfoAboutGradPt.y );
        pCont->AddFrame( pViewData ) ;
      }
    }

    FXString Caption ;
    switch ( m_SectorsMarkingMode )
    {
    case SMM_Marking: Caption = "Sectors\nMarking" ; break ;
    case SMM_Delete: Caption = "Sectors\nDelete" ; break ;
    }
    if ( !Caption.IsEmpty() )
    {
      CTextFrame * pCaption = CreateTextFrame( Caption , _T( "SMM Caption" ) ) ;
      *(pCaption->Attributes()) = _T( "x=550;y=3; Sz=12; color=0xff00ff;" ) ;
      pCont->AddFrame( pCaption ) ;
    }
    if ( iPercent >= 0 )
    {
      cmplx cPercPos = cMapImgCent - cmplx( 30. , 0. ) ;
      CTextFrame * pPercShow = CreateTextFrame( cPercPos ,
        _T( "" ) , _T( "0xffffff" ) ) ;
      pPercShow->GetString().Format( _T( "%d%%" ) , iPercent ) ;
      pCont->AddFrame( pPercShow ) ;
    }
    if ( m_AverData.Size() )
    {
      for ( int i = 0 ; i < m_AverData.Count() ; i++ )
      {
        cmplx cBeginSegm = m_AverData[ i ].m_cSegmentBegin ;
        cmplx cEndSegm = m_AverData[ (i + 1) % m_AverData.size() ].m_cSegmentBegin ;
        cmplx VectFromCenter = cBeginSegm - m_cPartOnMapCenterByExtrems;
        cmplx Pt2 = m_AverData[ i ].m_cSegmentBegin - VectFromCenter * 0.2 ;
        CFigureFrame * pBeginMark = CreateLineFrame(
          m_AverData[ i ].m_cSegmentBegin , Pt2 , 0xffffff ) ;
        pCont->AddFrame( pBeginMark ) ;
        cmplx cVectFromBeginToEnd = cEndSegm - cBeginSegm ;
        cmplx cToLeft = -GetOrthoLeft( GetNormalized( cVectFromBeginToEnd ) ) ;
        cmplx cTextPt = 0.5 * (cBeginSegm + cEndSegm) + cToLeft * 25. - cmplx( 20. , 7. ) ;
        pCont->AddFrame( CreateTextFrame( cTextPt , "0xffffff" , 12 , 
          NULL , 0 , "A%.1f\nM%.1f" , m_AverData[ i ].GetAverGrad_um() , m_AverData[i].GetMaxGrad_um() ) ) ;
      }
    }
    // View max grad point on map (March 20, 2023)
    if ( m_iMaxGradPtIndex >= 0 )
    {
      FormMarker(m_iMaxGradPtIndex, 0x000000ff, true, pCont, m_bFormMaxGradMarker ? 2 : 3 );
      m_bFormMaxGradMarker = false;
    }

    // View selected point on map (March 20, 2023)
    if ( m_iSelectedPtIndex >= 0 )
    {
      cmplx cSelected = FormMarker(m_iSelectedPtIndex, 0x00ff00ff, true, pCont, 1);

      // Form marking on observation view
      if ( m_ObservationExtremes.Count() == 4 )
      {
        cmplx cVectFromCentToSelected = cSelected - m_cPartOnMapCenterByExtrems/*cMapImgCent*/;
        cmplx cVectCentToSelectedOnObservation =
          cVectFromCentToSelected * m_dObservScale_pix_per_um / m_dMapScale_pix_per_um ;
        cmplx cFirstOnObserv = m_ObservationExtremes[ 0 ] ;

        CContainerFrame * pObservationView = CContainerFrame::Create() ;
        m_pObservationImage->lpBMIH->biCompression = 0x20203859 ;
        m_pObservationImage->AddRef() ;
        pObservationView->AddFrame( m_pObservationImage ) ;
        m_VectorsToVertices->AddRef() ;
        pObservationView->AddFrame( m_VectorsToVertices ) ;

        cmplx cSelectedPointerOnObserv = m_cPartCenterOnObservation + cVectCentToSelectedOnObservation * 1.3 ;
        CFigureFrame * pPointer = CreateLineFrame(m_cPartCenterOnObservation, cSelectedPointerOnObserv,
          0x0000ff, "VectToSelectedPt");
        pObservationView->AddFrame(pPointer);

//        cmplx cRectCentOnAbserv = m_cPartCenterOnObservation + cVectCentToSelectedOnObservation;
//         cmplx cPtOnRect = cRectCentOnAbserv + cmplx( -8. , 8. ) ;
//         CFigureFrame * pMarkRect = CreateFigureFrame( &cPtOnRect , 1 , 0x00ff00 ,
//           "VectToSelectedPt" , m_pObservationImage->GetId() ) ;
//         cPtOnRect += 16. ;
//         pMarkRect->AddPoint( CmplxToCDPoint( cPtOnRect ) ) ;
//         cPtOnRect._Val[ _IM ] += 16. ;
//         pMarkRect->AddPoint( CmplxToCDPoint( cPtOnRect ) ) ;
//         cPtOnRect -= 16. ;
//         pMarkRect->AddPoint( CmplxToCDPoint( cPtOnRect ) ) ;
//         cPtOnRect._Val[ _IM ] -= 16. ;
//         pMarkRect->AddPoint( CmplxToCDPoint( cPtOnRect ) ) ;
//         pObservationView->AddFrame( pMarkRect ) ;

        pObservationView->SetLabel( "MarkedObservation" ) ;
        PutAndRegisterFrame( m_pOutput , pObservationView );
      }
    }
    for ( int i = 0 ; i < m_AveragedSelectedPoints.Count() ; i++ )
    {
      AveragedDataAroundPoint& Pt = m_AveragedSelectedPoints.GetAt( i ) ;
      pCont->AddFrame( CreateLineFrame( Pt.m_cBackwardPt , Pt.m_cForwardPt , Pt.m_Color ) ) ;
      pCont->AddFrame( CreateTextFrame( Pt.m_cTextPt , Pt.m_AsText , Pt.m_Color ) ) ;
    }
    // View scale image (March 20, 2023)
    double dMaxNormal = m_dMaximumsForViewCsale[(int)m_ResultViewMode] ;
    cmplx cRedLimitPos(m_ScaleBarPos.left - 30. , m_ScaleBarPos.top - 15. ) ;
    pCont->AddFrame(CreateTextFrame(cRedLimitPos, "ffffff", 10,
      "RedLimit", 0, "%d", ROUND( dMaxNormal * 1.12)));
    cmplx cRedThresPos(cRedLimitPos + cmplx( 0. , (m_ScaleBarPos.Height() * 0.12)));
    pCont->AddFrame(CreateTextFrame(cRedThresPos, "ffffff", 10,
      "RedLimit", 0, "%d", ROUND(dMaxNormal)));
    cmplx cWhiteThresPos( m_ScaleBarPos.left - 30., 
      m_ScaleBarPos.bottom - (m_ScaleBarPos.Height() * 10. /dMaxNormal ));
    pCont->AddFrame(CreateTextFrame(cWhiteThresPos, "ffffff", 10,
      "RedLimit", 0, "%d", 10 ));
    return pCont ;
  }
  else if ( pMapViewImage )
    return pMapViewImage ;
  else
    return NULL ;
}
bool ConturAsm::FormScaleView(
  CVideoFrame * pTarget , CRect Placement , double dMax , double dMin )
{
  int iWidth = Placement.Width() ;
  int iHeight = Placement.Height() ;
  int iImageWidth = GetWidth( pTarget ) ;
  int iImageHeight = GetHeight( pTarget ) ;
  if ( Placement.left < 0 || Placement.right >= iImageWidth
    || Placement.top < 0 || Placement.bottom >= (LONG) GetHeight( pTarget ) )
    return false ;

  double dK = 255. * 2. / iHeight ;
  LPBYTE pImage = GetData( pTarget ) ;
  int iInterlace = iImageWidth * 3 ;  // For RGB image only
  int iShift = Placement.left * 3 ;
  int iBarWidth = iWidth * 3 ;
  int iYCent = iHeight / 2 ;

  BYTE bRed = 0 ;
  int iRedGrows = (Placement.Height() / 5);
  for ( int iY = Placement.top ; iY < Placement.bottom ; iY++ )
  {
    double dRelative = 1.12 * (double) (Placement.bottom - iY) / (double) iHeight ;
//     BYTE bBlue = (dRelative > 0.5) ? 255 : (BYTE) ROUND( dRelative * 511. ) ;
//     BYTE bGreen = (dRelative < 0.5) ? 255 : (BYTE) ROUND( (1. - dRelative) * 511. ) ;
//     int iYPos = iY - Placement.top;
//     if (iYPos < iRedGrows)
//     {
//       if (iYPos <= 1 )
//       {
//         bRed = 255;
//         bGreen = bBlue = 0;
//       }
//       else
//         bRed = (iRedGrows - iYPos) * 255 / iRedGrows;
//     }
//     else
//       bRed = 0;
    DWORD dwColor = GetMarkColor(dRelative, 1.0 , dMin );
    BYTE bRed = (dwColor >> 16) & 0xff;
    BYTE bGreen = (dwColor >> 8) & 0xff;
    BYTE bBlue = dwColor & 0xff;
    LPBYTE pInImage = pImage + ((iImageHeight - iY) * iInterlace) + iShift ;
    LPBYTE pEnd = pInImage + iBarWidth ;
    while ( pInImage < pEnd )
    {
      *(pInImage++) = bBlue ;
      *(pInImage++) = bGreen ;
      *(pInImage++) = bRed ;
    }
  }
  return true ;
}

DWORD ConturAsm::GetMarkColor(double dValue, double dMaxValue , double dMinValue )
{
  if (dValue == 0.)
    return 0x000000ff;
  if (dValue <= dMinValue)
    return 0x00ffffff;
  if (dValue > dMaxValue * 1.1)
    dValue = dMaxValue * 1.1 ;
  double dRelative = dValue / dMaxValue;
  // following for 
  BYTE bRed = ( dRelative >= 1.0 ) ? 255 : 0;
  BYTE bGreen = (dRelative < 1.1) ? 255 : 0;
  BYTE bBlue = 0;
  //BYTE bBlue = (dRelative >= 0.5) ? 255 : (BYTE)ROUND(511. * dRelative);
//   BYTE bBlue = 0;
//   BYTE bGreen = (dRelative <= 0.75) ? 255 : (BYTE)ROUND((1. - dRelative) * 1023.);
//   if (dRelative >= 0.995 )
//   {
//     bRed = 255;
//     bGreen = bBlue = 0;
//   }
//   else if (dRelative > 0.75)
//   {
//     bRed = (BYTE)ROUND( (dRelative - 0.75) * 1024.);
//   }

  DWORD dwResult = (DWORD)bBlue + (((DWORD)bGreen) << 8) + (((DWORD)bRed) << 16);
  return dwResult;
}


static CPoint gs_LastPt( 0 , 0 ) ;
static double gs_dFromMapLastTime = 0. ;

bool ConturAsm::ProcessMsgFromMap( const CTextFrame * pMsgFromMap )
{
  if ( !IsInactive() )
  {
    SENDERR( "Scanning is ON, Operations with map are disabled" ) ;
    return false;
  }
  if ( !m_ConturData.Count() )
  {
    SENDERR( "There is no saved data about part" ) ;
    return false;
  }
  if ( m_DataFrom == ADF_NoData )
  {
    SENDERR( "Saved data about part are not valid" ) ;
    return false;
  }

  double dNow = GetHRTickCount() ;
  if ( dNow - gs_dFromMapLastTime < 500. )
    return false ;
  gs_dFromMapLastTime = GetHRTickCount();
  m_bScanFinished = m_bPrepareToEnd = false;
  FXPropertyKit pk = pMsgFromMap->GetString() ;
  CPoint MapPt ;
  if ( pk.GetInt( _T( "x" ) , (int&) MapPt.x )
    && pk.GetInt( _T( "y" ) , (int&) MapPt.y ) )
  {

    bool bCntl = m_bCTRLKeyIsDown ;
    bool bShift = m_bShiftKeyIsDown ;
    if ( MapPt == m_LastMapPt && bShift == m_bLastShiftPressedForMap )
      return false ; // not show second time the same;

    bool bModeProcessed = false ;
    MapMouseClickMode Mode = IsNewModeSelected( MapPt ) ;
    if ( Mode != MCM_NoReaction )
    {
      switch ( Mode )
      {
      case MCM_NoReaction:
        break;
      case MCM_DeleteAverage:
      case MCM_GetAverage:
      case MCM_MoveAndFullMeasurement:
      case MCM_MoveAndMeasureOnce:
      case MCM_MoveAndView:
      case MCM_ViewSaved:
        m_MapMouseClickMode = Mode ;
        bModeProcessed = true ;
        break;
      case MCM_RangePlus:
        ChangeAverageRange( true ) ;
        bModeProcessed = true ;
        break;
      case MCM_RangeMinus:
        ChangeAverageRange( false ) ;
        bModeProcessed = true ;
        break;
      case MCM_DoMapOutput:
      case MCM_PrintMap:
        bModeProcessed = true;
      case MCM_MeasureManual:
        break;
      case MCM_DeleteManual:
        break;
      default:
        break;
      }
      if ( bModeProcessed )
      {
        CDataFrame * pMapView = FormMapImage( NULL , m_ResultViewMode , m_iIndexForMapView , -1 ) ;
        if ( pMapView )
          pMapView->Release() ;
        pMapView = FormMapImage( NULL , m_ResultViewMode , m_iIndexForMapView , -1 ) ;
        if ( pMapView )
        {
          if ( Mode == MCM_DoMapOutput || Mode == MCM_PrintMap )
            PrepareDataForCombinedMap(pMapView , Mode == MCM_PrintMap );
          return PutFrame(m_pOutput, pMapView);
        }
        return false ;
      }
    }
    
    m_LastMapPt = MapPt ;
    m_bLastShiftPressedForMap = bShift ;
    CCoordinate Target ;
    bool bPtFound = GetTargetForMapPoint( MapPt , Target , m_iIndexForMapView ) ;
    int iPtNumForImage = GetFirstSampleInSegment( m_iIndexForMapView ) ;

    if ( bPtFound && (iPtNumForImage >= 0) )
    {
      cmplx cPt( MapPt.x , MapPt.y ) ;
      cmplx cMapCenter( 300. , 300. ) ;
      cmplx cVect = cPt - cMapCenter ;
      cVect /= m_dMapScale_pix_per_um ;
      double dAngle = arg( cVect ) ;
      switch ( m_SectorsMarkingMode )
      {
      case SMM_NoMarking:
        {
          m_iSelectedPtIndex = m_iIndexForMapView ;
          CDataFrame * pMapView = FormMapImage( NULL , m_ResultViewMode , m_iIndexForMapView , -1 ) ;
          ConturSample& Sample = m_ConturData.GetAt( m_iIndexForMapView ) ;
          switch ( m_MapMouseClickMode )
          {
          case MCM_GetAverage:
            {
              CPoint PtForward , PtBackward ;
              double dVal = GetAverageAroundPt( m_iSelectedPtIndex , m_iAveragingRange_um ,
                PtBackward , PtForward ) ;
              if ( dVal != 0. )
              {
                double dMaxValue = m_dMaximumsForViewCsale[ m_ResultViewMode ] ;

                AveragedDataAroundPoint NewPt( Sample.m_PointOnMap , PtBackward , PtForward ,
                  cMapCenter , 0xffffff , dVal );
                m_AveragedSelectedPoints.Add( NewPt ) ;
                CDataFrame * pMapView = FormMapImage( NULL , m_ResultViewMode , m_iIndexForMapView , -1 ) ;
                if ( pMapView )
                  pMapView->Release() ;
                pMapView = FormMapImage( NULL , m_ResultViewMode , m_iIndexForMapView , -1 ) ;
                return (pMapView) ? PutFrame( m_pOutput , pMapView ) : false ;
              }
            }
            break ;
          case MCM_MoveAndFullMeasurement:
          case MCM_MoveAndMeasureOnce:
          case MCM_MoveAndView:
            {
              if ( pMapView )
                PutFrame( m_pOutput , pMapView ) ;
              CDataFrame * pOut = FormFrameForSavedImageView(m_iIndexForMapView);
              if (pOut)
                PutFrame((pOut->IsContainer()) ? m_pMainView : m_pMarking, pOut);
              if (m_DataFrom == ADF_MeasuredNow)
              {
                double dAngle = -arg(Sample.m_cSegmentVect);// Y is going down
                SendEdgeAngleToTVObject(dAngle);
                m_dLastEdgePredictedAngle_deg = RadToDeg( dAngle ) ;
                if ( !Sample.m_RobotPos.Compare( m_CurrentPos , 10. ) )
                {
                  if ( m_MapMouseClickMode == MCM_MoveAndFullMeasurement )
                    m_ProcessingStage = Stage_OneDarkMeasBeginToMax ;
                  else
                    m_ProcessingStage = Stage_GetExtImage ; 
                  SetLightForDarkEdge() ;
                }
                else
                {
                  OrderAbsMotion( Sample.m_RobotPos ) ;
                  m_DelayedOperations.push( 
                    (m_MapMouseClickMode == MCM_MoveAndFullMeasurement) ? 
                       Stage_OneDarkMeasBeginToMax : Stage_GetExtImage ) ;
                }
                return true ;
              }
              else
                SENDERR( "There is no data for part on platform, motion disabled" ) ;

            }
            break ;
          case MCM_ViewSaved:
            if ( iPtNumForImage >= 0 )
            {
              CDataFrame * pOut = FormFrameForSavedImageView(m_iIndexForMapView);
              if ( pMapView )
                PutFrame( m_pOutput , pMapView ) ;

              if ( pOut )
              {
//                 FXString ContainerAsText , Prefix ;
//                 int iNFrames = 0 ;
//                 FXPtrArray Ptrs ;
//                 int iRes = FormFrameTextView( pOut , ContainerAsText ,
//                   Prefix , iNFrames , Ptrs ) ;
                if ( PutFrame( (pOut->IsContainer()) ?
                  m_pMainView : m_pMarking , pOut ) )
                {
                  FXString Report ;
                  Report.Format( "SelectedOnMap: ArchivePt=%d" , m_iIndexForMapView ) ;
                  SendReport( Report ) ;
                  return true ;
                }
              }
            }
            break ;
          default:
            break;
          }
        }
        break ;
      case SMM_Delete:
        {
          m_CurrentDetailedForm.DeleteSector( cVect ) ;
          m_CurrentDetailedForm.SaveToFile( (LPCTSTR) m_PartDetailsFileName ) ;
        }
        break ;
      case SMM_Marking:
        {
          m_CurrentDetailedForm.AddSector( cVect , m_dSectorRange_um ) ;
          m_CurrentDetailedForm.SaveToFile( (LPCTSTR) m_PartDetailsFileName ) ;
        }
        break ;
      }

    }
    else
    {
       if ( m_MapMouseClickMode == MCM_DeleteAverage)
       {
         for (int i = 0; i < m_AveragedSelectedPoints.Count(); i++)
         {
           if ( Dist( MapPt , m_AveragedSelectedPoints[i].m_cTextPt) < 20.)
           {
             m_AveragedSelectedPoints.RemoveAt(i);
             CDataFrame * pMapView = FormMapImage(NULL, m_ResultViewMode, m_iIndexForMapView, -1);
             if (pMapView)
               pMapView->Release();
             pMapView = FormMapImage(NULL, m_ResultViewMode, m_iIndexForMapView, -1);
             return (pMapView) ? PutFrame(m_pOutput, pMapView) : false;
           }
         }
       }
       SENDERR("Can't find result for map Pt(%d,%d)", MapPt.x, MapPt.y);
    }
  }
  else
    SENDERR( "There is no data about coordinates in '%s'" , (LPCTSTR) pk ) ;

  return false;
}


CDataFrame * ConturAsm::FormFrameForSavedImageView( int iPtNumInContur )
{
  int iPtNumForImage = GetFirstSampleInSegment( iPtNumInContur ) ;

  if ( iPtNumForImage < 0 || iPtNumForImage >= m_ConturData.Count() )
    return NULL ;

  ConturSample& Sample = m_ConturData.GetAt( iPtNumInContur ) ;

  FXString PureFileName ;
  PureFileName.Format( "Pt%d_X%d_Y%d.bmp" , iPtNumForImage ,
    ROUND( Sample.m_RobotPos.m_x ) , ROUND( Sample.m_RobotPos.m_y ) ) ;

  FXString FilePath( m_ImagesDirectory + PureFileName ) ;
  CRect SavedFragment ;
  CVideoFrame * pFrame = GetImageFromFile( FilePath , SavedFragment ) ;
  if ( !pFrame )
  {
    SENDERR( "No saved image for Pt#%d(%d) %s" ,
      iPtNumInContur , iPtNumForImage , (LPCTSTR) FilePath ) ;
    return NULL ;
  }
  else
    SENDINFO( "File %s found" , (LPCTSTR) PureFileName ) ;
  // if there is no info about fragment
  if ( SavedFragment.IsRectEmpty() )
    return pFrame ;  // simple do fragment output

  m_LastSavedFragment = SavedFragment ;
  // Otherwise create container with fragment and graphical info
  CContainerFrame * pOut = CContainerFrame::Create() ;

  int iWidth = GetWidth( pFrame ) ;
  int iHeight = GetHeight( pFrame ) ;

  cmplx cEdge = Sample.m_cEdgePt ;
  cmplx cTopLeft( SavedFragment.left , SavedFragment.top ) ;
  cEdge -= cTopLeft ;
  CFigureFrame * pEdge = CreateFigureFrame(
    &cEdge , 1 , (DWORD) 0x00ff00ff ) ;
  pEdge->Attributes()->WriteInt( "thickness" , 3 ) ;
  pOut->AddFrame( pEdge ) ;


  cmplx cWidthDir = GetOrthoLeft( Sample.m_cSegmentVect ) ;
  cmplx cNormWidthDir = GetNormalized( cWidthDir ) ;
  cWidthDir = cNormWidthDir * Sample.m_dAveBurrWidth_um ;
  cWidthDir *= m_dScale_pix_per_um ;
  CDPoint cdpWidth = CmplxToCDPoint( cEdge + cWidthDir ) ;
  pEdge->AddPoint( cdpWidth ) ;

  // Do image centering to selected point
  FXString CenteringMsg ;
  CenteringMsg.Format( "Xc=%d;Yc=%d;" ,
    ROUND( cEdge.real() ) , ROUND( cEdge.imag() ) ) ;
  CTextFrame * pCenteringFrame = CreateTextFrame(
    CenteringMsg , _T( "SetCenter" ) ) ;
  pOut->AddFrame( pCenteringFrame ) ;

  double dAng = arg( cWidthDir ) ;
  cmplx cCent( SavedFragment.Width() * 0.5 , SavedFragment.Height() * 0.5 ) ;
  double dTextShift = 120. ;
  if ( fabs( dAng ) <= M_PI_4 )
    dTextShift = 200. ;
  else if ( fabs( dAng ) > 3. * M_PI_4 )
    dTextShift = 100. ;
  cmplx cTextViewPt = cCent + polar( dTextShift , NormRad( dAng + M_PI ) ) ;
  //   if ( -M_PI_4 <= dAng && dAng <= M_PI_4 )
  //     cTextViewPt = -cNormWidthDir * 100. - cmplx( 0. , 20. ) ;
  //   else if ( (-(3. * M_PI_4) <= dAng) && (dAng < -M_PI_4) )
  //     cTextViewPt = -cNormWidthDir * 50. - cmplx( 20. , 0. ) ;
  //   else if ( (M_PI_4 <= dAng) && (dAng < (3. * M_PI_4)) )
  //     cTextViewPt = -cNormWidthDir * 90. - cmplx( 20. , 20. )  ;
  //   else
  //     cTextViewPt = -cNormWidthDir * 50. - cmplx( 0. , 20. ) ;

  cmplx cTextToEdgeVect = cEdge - cTextViewPt ;

  m_LastTextViewPtOnSavedImage = GetCPoint( cTextViewPt ) ;
  m_LastPtDescription.Format( "Pt%d\nW=%.1f um" ,
    iPtNumInContur , Sample.m_dAveBurrWidth_um ) ;
  CTextFrame * pDescr = CreateTextFrame(
    cTextViewPt , (LPCTSTR) m_LastPtDescription , "0xff00ff" ) ;
  pOut->AddFrame( pDescr ) ;

  m_LastViewPtr1st = cTextViewPt + cTextToEdgeVect * ((dTextShift == 200.) ? 0.5 : 0.5) ;
  m_LastViewPtr2nd = cTextViewPt + cTextToEdgeVect * 0.99 ;
  CFigureFrame * pPtr = CreateFigureFrame(
    &m_LastViewPtr1st , 1 , (DWORD) 0x00ff00ff ) ;
  pPtr->AddPoint( CmplxToCDPoint( m_LastViewPtr2nd ) ) ;
  pOut->AddFrame( pPtr ) ;

  //   CTextFrame * pMiddlePoint = CreateTextFrame( "SetCenter" , (DWORD) 0 ) ;
  //   pMiddlePoint->GetString().Format( "Xc=%d;Yc=%d;" ,
  //     SavedFragment.Width() / 2 , SavedFragment.Height() / 2 ) ;
  //   pOut->AddFrame( pMiddlePoint ) ; // once
  pFrame->lpBMIH->biCompression = BI_Y8 ;
  //   LPBYTE pImage = GetData( pFrame ) ;
  //   for ( int iy = 0 ; iy < iHeight / 2 ; iy++ )
  //   {
  //     BYTE Tmp[ 1024 ] ;
  //     LPBYTE pLow = pImage + iy * iWidth ;
  //     LPBYTE pHigh = pImage + (iHeight - iy - 1) * iWidth ;
  //     memcpy( Tmp , pLow , iWidth ) ;
  //     memcpy( pLow , pHigh , iWidth ) ;
  //     memcpy( pHigh , Tmp , iWidth ) ;
  //   }
  pOut->AddFrame( pFrame ) ;

  return pOut ;
}


int ConturAsm::CombineImagesAndPutOverlay( 
  pTVFrame pFrame1 , pTVFrame pFrame2 , Directions Orient , 
  GraphicsData& Overlay , bool bInvertBW , bool bPrint ,
  const pTVFrame pOverviewFrame )
{
  LPBITMAPINFOHEADER pBMIH1RGB = (GetCompression( pFrame1 ) == BI_RGB) ?
    pFrame1->lpBMIH : _decompress2rgb( pFrame1 , false ) ;
  LPBYTE pData1 = GetData( pFrame1 ) ;
  int iWidth1 = GetWidth( pFrame1 ) ;
  int iHeight1 = GetHeight( pFrame1 ) ;
  LPBYTE pTmpImage = NULL ;
  if ( bInvertBW )
  {  // Do inverse BW pixels only
    // Create separate image plane
    int iSize1 = iWidth1 * iHeight1 * 3 ;
    pTmpImage = new BYTE[ iSize1 ] ;
    memcpy_s( pTmpImage , iSize1 , pData1 , iSize1 ) ;
    pData1 = pTmpImage ;
    LPBYTE pIter = pData1 ;
    LPBYTE pEnd = pIter + iSize1 ;
    do
    {
      if ( (*pIter == 0) && (*(pIter + 1) == 0) && (*(pIter + 2) == 0) )
        *(pIter + 2) = *(pIter + 1) = *(pIter) = 0xff ;
      else if ( (*pIter == 0xff) && (*(pIter + 1) == 0xff) && (*(pIter + 2) == 0xff) )
        *(pIter + 2) = *(pIter + 1) = *(pIter) = 0 ;
    } while ( (pIter += 3) < pEnd );
  }

  int iWidth2 = GetWidth( pFrame2 ) ;
  int iHeight2 = GetHeight( pFrame2 ) ;
  int iOverviewWidth = pOverviewFrame ? GetWidth( pOverviewFrame ) : 0 ;
  int iOverviewHeight = pOverviewFrame ? GetHeight( pOverviewFrame ) : 0 ;
  // default orientation is pFrame1 on the left and pFrame2 on the right 
  int iCombineWidth = iWidth1 + iWidth2 + (pOverviewFrame ?iOverviewWidth : 0) ;
  int iCombineHeight = max( iHeight1 , iHeight2 ) ;
  if ( pOverviewFrame && (iOverviewHeight > iCombineHeight) )
    iCombineHeight = iOverviewHeight ;

  switch ( Orient ) // from Frame 1 to Frame 2
  {
  case H12_00:
  case H06_00:
    {
      iCombineWidth = max( iWidth1 , iWidth2 ) ;
      if ( pOverviewFrame && (iOverviewWidth > iCombineWidth) )
        iCombineHeight = iOverviewWidth ;

      iCombineHeight = iHeight1 + iHeight2 + (pOverviewFrame ? iOverviewHeight : 0);
    }
    break ;
  }

  LPBITMAPINFOHEADER pBMIH2RGB = (GetCompression( pFrame2 ) == BI_RGB) ?
    pFrame2->lpBMIH : _decompress2rgb( pFrame2 , false ) ;
  LPBYTE pData2 = GetData( pFrame2 ) ;

  LPBITMAPINFOHEADER  pBMIHOverviewRGB = NULL ;
  LPBYTE pDataOverview = NULL ;
  if ( pOverviewFrame )
  {
    pBMIHOverviewRGB =   (GetCompression( pOverviewFrame ) == BI_RGB) ?
    pOverviewFrame->lpBMIH : _decompress2rgb( pOverviewFrame , false ) ;
    pDataOverview = GetData( pOverviewFrame ) ;
  }

  int iCombineSize = iCombineHeight * iCombineWidth * 3 ; //24 bits BMP
  HDC hMemDC = CreateCompatibleDC( NULL ) ;
  if ( hMemDC )
  {
    BITMAPINFO bmpInfo ;
    OBJECT_ZERO( bmpInfo ) ;
    bmpInfo.bmiHeader.biSize = sizeof( BITMAPINFOHEADER );
    bmpInfo.bmiHeader.biWidth = iCombineWidth;
    bmpInfo.bmiHeader.biHeight = -iCombineHeight;
    bmpInfo.bmiHeader.biPlanes = 1;
    bmpInfo.bmiHeader.biBitCount = 24 ;
    bmpInfo.bmiHeader.biCompression = BI_RGB;
    LPBYTE pImage ;
    HBITMAP hMemBitmap = CreateDIBSection( hMemDC , &bmpInfo ,
      DIB_RGB_COLORS , (LPVOID*)&pImage , 0 , 0 );
    CRect NoNegativeBW ;
//     HBITMAP hMemBitmap = CreateCompatibleBitmap(
//       hMemDC , iCombineWidth , iCombineHeight ) ;
    if ( hMemBitmap )
    {
      HGDIOBJ hOldBitMap = SelectObject( hMemDC , hMemBitmap ) ;
      SetStretchBltMode( hMemDC , STRETCH_DELETESCANS );
      int iNLines1 = 0 , iNLines2 = 0 ;
      FXString OrderType ;
      OrderType.Format( "Order: %s , Type: %s\n"
        "Worst grad %.1f microns (Pt%d)" ,
        (LPCTSTR) m_PO , (LPCTSTR) m_InternalCatalogID ,
        m_dLastScanMaxWidth_um , m_iIndexWithMaxWidth ) ;
      if ( bInvertBW )
      {
        HGDIOBJ hOldBrush = SelectObject( hMemDC , GetStockObject( WHITE_BRUSH ) ) ;
        Rectangle( hMemDC , 0 , 0 , iCombineWidth , iCombineHeight ) ;
        SelectObject( hMemDC , hOldBrush ) ;
      }
      switch ( Orient )
      {
      case H12_00:
        {
          if ( pOverviewFrame )
          {
            iNLines1 = SetDIBitsToDevice( hMemDC , 0 , 0 , iOverviewWidth , iOverviewHeight ,
              0 , iOverviewHeight , 0 , iOverviewHeight , pDataOverview ,
              (BITMAPINFO*) (pBMIHOverviewRGB) , DIB_RGB_COLORS ) ;
          }
          int iY2Origin = iOverviewHeight ? iOverviewHeight : 0 ;
          int iY1Origin = iY2Origin + iHeight2 ;
          iNLines1 = SetDIBitsToDevice( hMemDC , 0 , iY2Origin , iWidth2 , iHeight2 ,
            0 , iHeight2 , 0 , iHeight2 , pData2 ,
            (BITMAPINFO*) (pBMIH2RGB) , DIB_RGB_COLORS ) ;
          iNLines2 = SetDIBitsToDevice( hMemDC , 0 , iY1Origin , iWidth1 , iHeight1 ,
            0 , iHeight1 , 0 , iHeight1 , pData1 ,
            (BITMAPINFO*) (pBMIH1RGB) , DIB_RGB_COLORS ) ;
          NoNegativeBW = CRect( 0 , 0 , max( iWidth2 , iOverviewWidth) , iOverviewHeight + iHeight2 ) ;
        }
        break ;
      case H06_00:
        {
          iNLines1 = SetDIBitsToDevice( hMemDC , 0 , 0 , iWidth1 , iHeight1 ,
            0 , iHeight1 , 0 , iHeight1 , pData1 ,
            (BITMAPINFO*) (pBMIH1RGB) , DIB_RGB_COLORS ) ;
          iNLines2 = SetDIBitsToDevice( hMemDC , 0 , iHeight1 , iWidth2 , iHeight2 ,
            0 , iHeight2 , 0 , iHeight2 , pData2 ,
            (BITMAPINFO*) (pBMIH2RGB) , DIB_RGB_COLORS ) ;
          if ( pOverviewFrame )
          {
            iNLines1 = SetDIBitsToDevice( hMemDC , 0 , iHeight1 + iHeight2 , 
              iOverviewWidth , iOverviewHeight ,
              0 , iOverviewHeight , 0 , iOverviewHeight , pDataOverview ,
              (BITMAPINFO*) (pBMIHOverviewRGB) , DIB_RGB_COLORS ) ;
          }
          NoNegativeBW = CRect( 0 , iHeight1 , max( iWidth2 , iOverviewWidth ) , iOverviewHeight + iHeight2 ) ;
        }
        break ;
      case H09_00:
        {
          if ( pOverviewFrame )
          {
            iNLines1 = SetDIBitsToDevice( hMemDC , 0 , 0 , iOverviewWidth , iOverviewHeight ,
              0 , iOverviewHeight , 0 , iOverviewHeight , pDataOverview ,
              (BITMAPINFO*) (pBMIHOverviewRGB) , DIB_RGB_COLORS ) ;
          }
          int iX2Origin = iOverviewHeight ? iOverviewWidth : 0 ;
          int iX1Origin = iX2Origin + iWidth2 ;
          iNLines1 = SetDIBitsToDevice( hMemDC , iX2Origin , 0 , iWidth2 , iHeight2 ,
            0 , iHeight2 , 0 , iHeight2 , pData2 ,
            (BITMAPINFO*) (pBMIH2RGB) , DIB_RGB_COLORS ) ;
          iNLines2 = SetDIBitsToDevice( hMemDC , iX1Origin , 0 , iWidth1 , iHeight1 ,
            0 , iHeight1 , 0 , iHeight1 , pData1 ,
            (BITMAPINFO*) (pBMIH1RGB) , DIB_RGB_COLORS ) ;
          NoNegativeBW = CRect( 0 , 0 , iWidth2 + iOverviewWidth , max( iHeight2 , iOverviewHeight) ) ;
        }
        break ;

      default:
      case H03_00:
        {
          iNLines1 = SetDIBitsToDevice( hMemDC , 0 , 0 , iWidth1 , iHeight1 ,
            0 , 0 , 0 , iHeight1 , pData1 ,
            (BITMAPINFO*) (pBMIH1RGB) , DIB_RGB_COLORS ) ;
          pBMIH2RGB->biHeight = -pBMIH2RGB->biHeight ;
          iNLines2 = SetDIBitsToDevice( hMemDC , iWidth1 , 
            iHeight1 - iHeight2 , 
            iWidth2 , iHeight2 , 0 , 0 , 0 , iHeight2 , pData2 ,
            (BITMAPINFO*) (pBMIH2RGB) , DIB_RGB_COLORS ) ;
          if ( pOverviewFrame )
          {
            pBMIHOverviewRGB->biHeight = -pBMIHOverviewRGB->biHeight ;
            iNLines1 = SetDIBitsToDevice( hMemDC , iWidth1 + iWidth2 , 0 ,
              iOverviewWidth , iOverviewHeight ,
              0 , 0 , 0 , iOverviewHeight , pDataOverview ,
              (BITMAPINFO*) (pBMIHOverviewRGB) , DIB_RGB_COLORS ) ;
          }
          NoNegativeBW = CRect( iWidth1 ,
            iCombineHeight - iHeight2 , iCombineWidth , iCombineHeight ) ;
        }
        break ;
      }

      if ( pTmpImage )
        delete[] pTmpImage ;

      Overlay.DrawTexts( hMemDC , bInvertBW ) ;
      Overlay.DrawRectangles( hMemDC , bInvertBW ) ;
      Overlay.DrawFigures( hMemDC , bInvertBW ) ;

      BITMAPINFO bmih ;
      memset( &bmih , 0 , sizeof( bmih ) ) ;
      bmih.bmiHeader.biSize = sizeof( BITMAPINFOHEADER ) ;
      int iRes = GetDIBits( hMemDC , hMemBitmap ,
        0 , iCombineHeight , NULL , &bmih , DIB_RGB_COLORS ) ;
      if ( iRes )
      {
        CVideoFrame * pCombined = CVideoFrame::Create() ;
        pCombined->lpBMIH = (BITMAPINFOHEADER*)malloc( bmih.bmiHeader.biSize + bmih.bmiHeader.biSizeImage ) ;
        memcpy( pCombined->lpBMIH , &bmih.bmiHeader , bmih.bmiHeader.biSize ) ;
        LPBYTE pCombineData = (LPBYTE) (pCombined->lpBMIH + 1) ;
        iRes = GetDIBits( hMemDC , hMemBitmap ,
          0 , iCombineHeight , pCombineData , &bmih , DIB_RGB_COLORS ) ;
        FXString CombineFileName = m_ResultDirectory + m_PureFileName + _T( "_CombineMap.bmp" ) ;
        saveSH2BMP( (LPCTSTR) CombineFileName , pCombined->lpBMIH ) ;
        
        PutFrame( m_pMarking , pCombined ) ;

        if ( bPrint && (GetHRTickCount() - m_dLastPrintTime) > 5000. )
        {
          FXString CommandLine("mspaint ");
          CommandLine += CombineFileName + " /p";
          system((LPCTSTR)CommandLine);
          m_dLastPrintTime = GetHRTickCount() ;
        }
      }

      SelectObject( hMemDC , hOldBitMap ) ;
      DeleteObject( hMemBitmap ) ;
      if ( pBMIH1RGB != pFrame1->lpBMIH )
        free( pBMIH1RGB ) ;
      if ( pBMIH2RGB != pFrame2->lpBMIH )
        free( pBMIH2RGB ) ;
    }
    DeleteDC( hMemDC ) ;
  }

  return 0;
}


int ConturAsm::CalcAveragesForRegions( AverData& Result )
{
  if ( m_ConturData.Count() < 20 )
    return 0 ;

  m_AverData.RemoveAll() ;
  FXRegistry Reg( "TheFileX\\ConturASM" );
  int iNSegments = Reg.GetRegiInt( "Parameters" , "NAverageSegments" , 12 ) ;
  CFigure AsFig ;
  for ( int i = 0 ; i < m_ConturData.Count() ; i++ )
    AsFig.Add( CmplxToCDPoint( m_ConturData[ i ].m_MiddlePt ) ) ;

  CmplxArray Vertices ;
  cmplx cCenter ;
  double dPerimeter ;
  FXIntArray Indexes ;
  int iNVertices = GetNFilteredVertices( AsFig , 
    cCenter , &Vertices , 0.15 , &dPerimeter , &Indexes ) ;
  if ( iNVertices <= 1 )
  {  // ring
  }
  else
  {
    double dNSegmentsPerVertice = (double)iNSegments / (double)iNVertices ;
    iNSegments = ROUND(dNSegmentsPerVertice * iNVertices) ;
    int iNSamplesBetweenVertices = (Indexes[0] > Indexes[1]) ?
      (Indexes[1] + (int) m_ConturData.Count() - Indexes[0]) : (Indexes[1] - Indexes[0]);
    double dNSamplesPerSegment = iNSamplesBetweenVertices / dNSegmentsPerVertice ;
    double dLengthPerSegm = dPerimeter / iNSegments ;
    int iIter = Indexes[ 0 ] ;
    int iNewIter = (iIter + 1) % m_ConturData.Count() ;
    int iCurrentIndex = 0 ;
    int iNPassed = 0;
    int iNextIndex = 1 ;
    int iNextStop = Indexes[ 1 ] ;
    AveragedData NewSegm ;
    while ( iNewIter != Indexes[0] && (iNPassed++ < m_ConturData.Count()) )
    {
      NewSegm.Add( &m_ConturData[ iIter ] , &m_ConturData[ iNewIter ] ) ;
      iIter = iNewIter ;
      iNewIter = (iIter + 1) % m_ConturData.Count() ;
      if ( iNewIter == iNextStop )
      {
        iCurrentIndex = iNextIndex ;
        iNextIndex = (++iNextIndex) % iNVertices ;
        iNextStop = Indexes[ iNextIndex ] ;
        if ( Indexes[ iNextIndex ] < Indexes[ iCurrentIndex ] ) // full cycle 
          dNSamplesPerSegment = (m_ConturData.size() - Indexes[ iCurrentIndex ] + Indexes[ iNextIndex] ) / dNSegmentsPerVertice ;
        else
          dNSamplesPerSegment = (Indexes[ iNextIndex ] - Indexes[ iCurrentIndex ]) / dNSegmentsPerVertice ;
        if (NewSegm.m_iNSegments > 5)
        {
          Result.Add(NewSegm);
          NewSegm.Reset();
        }
      }
      else if ( NewSegm.m_iNSegments >= ROUND(dNSamplesPerSegment) )
      {
        Result.Add( NewSegm ) ;
        NewSegm.Reset() ;
      }
    }
                    if ( NewSegm.m_iNSegments > 30 )
      Result.Add( NewSegm ) ;
  }
  return 0;
}


int ConturAsm::SetAveragedValuesToOverlay( GraphicsData& Overlay )
{
  for ( int i = 0 ; i < m_AverData.size() ; i++ )
  {
    cmplx cBeginSegm = m_AverData[ i ].m_cSegmentBegin ;
    cmplx cEndSegm = m_AverData[ (i + 1) % m_AverData.size() ].m_cSegmentBegin ;
    cmplx VectFromCenter = cBeginSegm - m_cMapImageCenter ;
    cmplx Pt2 = m_AverData[ i ].m_cSegmentBegin - VectFromCenter * 0.2 ;
    FXGFigure BeginMark( 0xffffff , 1 ) ;
    BeginMark.AddPoint( m_AverData[ i ].m_cSegmentBegin ) ;
    BeginMark.AddPoint( Pt2 ) ;
    Overlay.m_Figures.Add( BeginMark ) ;
    FXString AsText ;
    AsText.Format( "A%.1f\nM%.1f" , m_AverData[ i ].GetAverGrad_um(), m_AverData[i].GetMaxGrad_um()) ;
    cmplx cVectFromBeginToEnd = cEndSegm - cBeginSegm ;
    cmplx cToLeft = -GetOrthoLeft( GetNormalized( cVectFromBeginToEnd ) ) ;
    cmplx cTextPt = 0.5 * (cBeginSegm + cEndSegm) + cToLeft * 25. ;
    CPoint Pt = GetCPoint( cTextPt ) ;
    FXGText NewText( AsText , Pt , 10 , 0xffffff ) ;
    Overlay.m_Texts.Add( NewText ) ;
  }
  for ( int i = 0 ; i < m_AveragedSelectedPoints.Count() ; i++ )
  {
    FXGFigure Marker(m_AveragedSelectedPoints[i].m_Color, 1);
    Marker.AddPoint(m_AveragedSelectedPoints[i].m_cBackwardPt);
    Marker.AddPoint(m_AveragedSelectedPoints[i].m_cForwardPt);
    Overlay.m_Figures.Add(Marker);
    FXString Str(m_AveragedSelectedPoints[i].m_AsText);
    FXGText Value( Str , m_AveragedSelectedPoints[i].m_cTextPt , 
      12 , m_AveragedSelectedPoints[i].m_Color );
    Overlay.m_Texts.Add(Value);

  }
  return 0;
}


int ConturAsm::PrepareDataForCombinedMap( CDataFrame * pMapView , bool bPrint )
{
  CDataFrame * pWorstView = FormFrameForSavedImageView( m_iIndexWithMaxWidth ) ;

  if ( pWorstView )
  {
    CVideoFrame * pMap = pMapView->GetVideoFrame() ;
    CVideoFrame * pFragment = pWorstView->GetVideoFrame() ;
    CPoint SavedImageShift( GetWidth( pMap ) , GetHeight( pMap ) - GetHeight( pFragment ) ) ;
    cmplx cSavedImageShift( GetWidth( pMap ) , GetHeight( pMap ) - GetHeight( pFragment ) ) ;
    GraphicsData Overlay ;
    Overlay.m_ScrScale = 1.0 ;
    ConturSample& Sample = m_ConturData.GetAt( m_iIndexWithMaxWidth ) ;
    CPoint MarkPt( Sample.m_PointOnMap.x , Sample.m_PointOnMap.y ) ;
    MarkPt.Offset( GetWidth( pMap ) - 10 , GetHeight( pMap ) - GetHeight( pFragment ) - 10 ) ;
    //               CRect MarkRect( MarkPt , CSize( 20 , 20) );
    //               FXRectangle MarkGrad( RGB( 255 , 0 , 0 ) , 3 ) ;
    //               MarkGrad = MarkRect ;
    //               Overlay.m_Rects.Add( MarkGrad ) ;
    FXGText MapScale( m_LastScaleCaptionText , m_LastScaleCaptionPt , 12 , RGB( 255 , 255 , 255 ) ) ;
    Overlay.m_Texts.Add( MapScale ) ;
    FXGText GradValue( m_LastInfoAboutGrad , m_LastInfoAboutGradPt , 12 , RGB( 255 , 255 , 255 ) ) ;
    Overlay.m_Texts.Add( GradValue ) ;
    FXGText PtOnImageDescr( m_LastPtDescription ,
      m_LastTextViewPtOnSavedImage + SavedImageShift , 10 , RGB( 255 , 0 , 255 ) ) ;
    Overlay.m_Texts.Add( PtOnImageDescr ) ;
    FXString OrderType ;
    OrderType.Format( "Order: %s , Type: %s\nCatalogName: %s"
      "\nScan TS %s\nScan Time %.1f sec"
      /*"Worst grad %.1f microns (Pt%d)"*/ ,
      (LPCTSTR) m_PO , (LPCTSTR) m_InternalCatalogID , (LPCTSTR) m_PartCatalogName ,
      m_MeasurementTS , m_dLastScanTime_ms * 0.001 ) ;
    FXGText OrderAndType( OrderType , CPoint( GetWidth( pMap ) + 10 , 10 ) , 14 , RGB( 255 , 255 , 255 ) ) ;
    Overlay.m_Texts.Add( OrderAndType ) ;

    cmplx cRedLimitPos(m_ScaleBarPos.left - 30., m_ScaleBarPos.top - 15.);
    FXString ScaleRed , ScaleGreen ;
    ScaleRed.Format("%d", ROUND(m_dMaximumsForViewCsale[(int)m_ResultViewMode] * 1.12));
    ScaleGreen.Format("%d", ROUND(m_dMaximumsForViewCsale[(int)m_ResultViewMode]));
    FXGText ScaleRedValue(ScaleRed, 
      CPoint(ROUND(cRedLimitPos.real()), ROUND(cRedLimitPos.imag() + m_ScaleBarPos.Height() * 0.12) ), 
      10, RGB(0,0,0));
    Overlay.m_Texts.Add(ScaleRedValue);
    FXGText ScaleGreenMax(ScaleRed,
      CPoint(ROUND(cRedLimitPos.real()), ROUND(cRedLimitPos.imag() + m_ScaleBarPos.Height())),
      10, RGB(0, 0, 0));
    Overlay.m_Texts.Add(ScaleGreenMax);
    FXGFigure Ptr( RGB( 255 , 0 , 255 ) ) ;
    cmplx cShifted1 = m_LastViewPtr1st + cSavedImageShift ;
    cmplx cShifted2 = m_LastViewPtr2nd + cSavedImageShift ;
    Ptr.AddPoint( cShifted1 ) ;
    Ptr.AddPoint( cShifted2 ) ;
    Overlay.m_Figures.Add( Ptr ) ;
    FXGFigure MapPtr( RGB( 255 , 0 , 255 ) ) ;
    MapPtr.AddPoint( m_LastPointer[ 0 ] ) ;
    MapPtr.AddPoint( m_LastPointer[ 1 ] ) ;
    Overlay.m_Figures.Add( MapPtr ) ;

    SetAveragedValuesToOverlay( Overlay ) ;
    
    CVideoFrame * pPartViewImage = GetSavedImage( _T( "_partview.bmp" ) , _T( "bmp" ) ) ;
    CombineImagesAndPutOverlay( pMap , pFragment , H03_00 , Overlay , true , bPrint , pPartViewImage) ;
    PutFrame( m_pMainView , pWorstView )  ;
  }
  return 0;
}



CVideoFrame * ConturAsm::GetSavedImage( 
  LPCTSTR pFileNameFragmentWithExt , LPCTSTR pExt )
{
  FXFileFind ff ;
  FXString FindPath = (m_ResultDirectory + _T("/*.")) + (pExt ? pExt : _T( "bmp" )) ;
  bool bFound = ff.FindFile( FindPath ) ;
  while ( bFound )
  {
    bFound = ff.FindNextFile();
    if ( ff.IsDots() || ff.IsDirectory() )
      continue;
    FindPath = ff.GetFilePath() ;
    if ( FindPath.MakeLower().Find( _T( pFileNameFragmentWithExt ) ) > 0 )
    {
      CRect SavedFragment ;
      CVideoFrame * pImage = GetImageFromFile( FindPath , SavedFragment ) ;
      if ( pImage )
        return pImage ;
    }
  }
  SENDERR( "No saved image *%s in directory %s" , pFileNameFragmentWithExt ,
    m_ResultDirectory ) ;

  return NULL;
}

int ConturAsm::ViewSavedObservationImage()
{
  CVideoFrame * pObservationImage = GetSavedImage( _T( "_partview.bmp" ) , _T( "bmp" ) ) ;
      if ( pObservationImage )
      {
        pObservationImage->lpBMIH->biCompression = BI_Y8 ;
        LPBYTE pImage = GetData( pObservationImage ) ;

        pObservationImage->SetLabel( _T( "Observation" ) ) ;
        ReplaceFrame( (const CDataFrame**) &m_pObservationImage , pObservationImage ) ;
        return (int) PutFrame( m_pOutput , pObservationImage ) ;
      }
  SENDERR( "No saved observation image '*_partview.bmp'in directory %s" ,
    m_ResultDirectory ) ;

  return 0;
}


CVideoFrame * ConturAsm::FormImageFor3dView( const CVideoFrame * pImage )
{
  // Form image for 3d presentation
  CVideoFrame * p3DContur = (CVideoFrame*) pImage->Copy() ;
  LPBYTE pIm = GetData( p3DContur ) ;
  int iWidth = GetWidth( p3DContur ) ;
  int iHeight = GetHeight( p3DContur ) ;
  memset( pIm , 0 , GetImageSize( p3DContur ) ) ;
  cmplx * pData = (cmplx*) m_pPartExternalEdge->GetData() ;
  cmplx Old ;
  if ( IsPtInFrame( *pData , p3DContur ) )
    Old = *pData ;

  for ( int i = 0 ; i < m_ConturData.GetCount() ; i++ )
  {
    cmplx cNext = pData[ i ] ;
    if ( IsPtInFrame( cNext , p3DContur ) )
    {
      double dHeight = m_ConturData[ i ].m_dAveBurrWidth_um * m_dWidthViewScale ;
      int iValue = ROUND( dHeight ) ;
      if ( iValue > 255 )
        iValue = 255 ;
      SetPixelsOnLine( Old , cNext , &iValue , 1 , p3DContur ) ;
      Old = cNext ;
    }
  }
  return p3DContur ;
}

MCM_Name MCM_ModeNames[] =
{
  { MCM_NoReaction , WMCA_Unavailable , NULL } ,
  { MCM_ViewSaved , WMCA_LiveAndSaved , _T( "Saved" ) } ,
  { MCM_GetAverage , WMCA_LiveAndSaved , _T( "GetAverage" ) } ,
  { MCM_DeleteAverage , WMCA_LiveAndSaved , _T( "DelAverage" ) } ,
  { MCM_RangePlus , WMCA_LiveAndSaved , _T( "RangeUp" ) } ,
  { MCM_RangeMinus , WMCA_LiveAndSaved , _T( "RangeDown" ) } ,
  { MCM_MoveAndView , WMCA_LiveOnly , _T( "Move&View" ) } ,
  { MCM_MoveAndMeasureOnce , WMCA_LiveOnly , _T( "Move&Meas" ) } ,
  { MCM_MoveAndFullMeasurement , WMCA_LiveOnly , _T( "Move&Analyze" ) } ,
  { MCM_DoMapOutput, WMCA_LiveAndSaved, _T("DoMapFile") },
  { MCM_PrintMap, WMCA_LiveAndSaved, _T("PrintMap") }
//   { MCM_MeasureManual , WMCA_LiveOnly , _T( "GetManual" ) } ,
//   { MCM_DeleteManual , WMCA_LiveOnly , _T( "DelManual" ) }
} ;

#define DIST_BETWEEN_ROWS_IN_SELECTOR_AREA 16

int ConturAsm::DrawMouseModeSelectMenu( CContainerFrame * pOutData , CPoint LeftTopPt )
{
  LPCTSTR pColor ;
  if ( m_DataFrom == ADF_NoData )
  {
    m_SelectorsAreaOnMap.SetRectEmpty() ;
    return 0 ;
  }
  int iItemsCnt = 0 ;
  for ( int i = 0 ; i < ARRSZ(MCM_ModeNames ) ; i++ )
  {
    if ( MCM_ModeNames[i].m_pName )
    {

      cmplx Pt( LeftTopPt.x , LeftTopPt.y + i * DIST_BETWEEN_ROWS_IN_SELECTOR_AREA ) ;

      switch ( m_DataFrom )
      {
      case ADF_MeasuredNow: 
        pColor = (m_MapMouseClickMode == MCM_ModeNames[ i ].m_Mode) ?  
          "0x00ff00" : "0x0c0c0c0" ;
        break;
      case ADF_MeasuredInThePast:
        if ( MCM_ModeNames[ i ].m_When == WMCA_LiveOnly )
          pColor = "0x0606060" ;
        else
        {
          pColor = (m_MapMouseClickMode == MCM_ModeNames[ i ].m_Mode) ?
            "0x00ff00" : "0x0c0c0c0" ;
        }
        break;
      }
      CTextFrame * pText = CreateTextFrame( Pt , MCM_ModeNames[ i ].m_pName ,
        pColor , 10 ) ;
      pOutData->AddFrame( pText ) ;
      iItemsCnt++ ;
      if ( MCM_ModeNames[i].m_Mode == MCM_RangePlus )
      {
        Pt += cmplx( 105. , 1. + DIST_BETWEEN_ROWS_IN_SELECTOR_AREA / 2. ) ;
        CTextFrame * pRangeText = CreateTextFrame( Pt , "c0c0c0" , 10 , NULL , 0 ,
          "R=%d" , m_iAveragingRange_um ) ;
        pOutData->AddFrame( pRangeText ) ;
      }
    }
  }
  m_SelectorsAreaOnMap = CRect( LeftTopPt , CSize( 60 ,
    ARRSZ(MCM_ModeNames) * DIST_BETWEEN_ROWS_IN_SELECTOR_AREA ) ) ;
  FXString ContList , Prefix ;
  int iLevel = 0 ;
  FXPtrArray FramePtrs ;
  int iNFrames = FormFrameTextView(
    pOutData , ContList , Prefix , iLevel , FramePtrs ) ;
  return iItemsCnt ;
}



MapMouseClickMode ConturAsm::IsNewModeSelected( CPoint MouseCoordsOnImage )
{
  MapMouseClickMode SelectedMode = MCM_NoReaction ;
  if ( m_SelectorsAreaOnMap.PtInRect( MouseCoordsOnImage) )
  {
    MouseCoordsOnImage -= m_SelectorsAreaOnMap.TopLeft() ;
    int iSelectedItem = (int)((double) MouseCoordsOnImage.y / (double) DIST_BETWEEN_ROWS_IN_SELECTOR_AREA) ;
    SelectedMode = (MapMouseClickMode) iSelectedItem ; //item 0 is for empty action, but area includes it
    if ( (MCM_ModeNames[SelectedMode].m_When == WMCA_LiveOnly) 
      && (m_DataFrom != ADF_MeasuredNow) )
    {
      SelectedMode = m_MapMouseClickMode ; // Disabled for now
    }
  }
  return SelectedMode ;
}

