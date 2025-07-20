#include "stdafx.h"
#include "TwoCamsGadget.h"
#include "fxfc/FXRegistry.h"

void TwoChans::ProcessForGreenMachine( const CDataFrame * pDataFrame , CContainerFrame * pMarking )
{
  if ( m_WorkingMode == WM_Green_Manual )
  {
    cmplx cTarget( m_cLastROICent_pix.real() , ( double ) m_iHorLinePosition_pix ) ;
    CreateFullFrameCross( cTarget , m_LastROI , pMarking , 0xff00ff ) ;
    switch ( m_GadgetMode )
    {
    case GM_SIDE:
      {
        cTarget._Val[ _IM ] -= m_iZOffset_um / m_dScale_um_per_pix ;
        cTarget._Val[ _RE ] += m_iYOffset_um / m_dScale_um_per_pix ;
        CreateFullFrameCross( cTarget , m_LastROI , pMarking , 0x000000ff ) ;
      }
      break ;
    case GM_FRONT:
      {
        cTarget._Val[ _RE ] += m_iXOffset_um / m_dScale_um_per_pix ;
        CreateFullFrameCross( cTarget , m_LastROI , pMarking , 0x000000ff ) ;
      }
      break ;
    }
  }
  else // not manual mode
  {
    double dMaxY = -DBL_MAX ;

    const CRectFrame * pEdgeUpROI = pDataFrame->GetRectFrame( "ROI:edgeup" ) ;
    CRect EdgeROI( pEdgeUpROI ? ( *( RECT* ) ( pEdgeUpROI ) ) : CRect( 100 , 500 , 1100 , 1800 ) ) ;
    EdgeROI.DeflateRect( 5 , 5 ) ;
    FXSIZE iIndexOfMaxY = -1 ;
    const CFigureFrame * pEdgeFig = NULL ;

    CFramesIterator* Iterator = pDataFrame->CreateFramesIterator( figure );
    if ( Iterator != NULL )
    {
      const CFigureFrame * pFigure = NULL ;
      SLines Segments ;
      while ( pFigure = ( const CFigureFrame * ) Iterator->Next() )
      {
        FXString Label = pFigure->GetLabel() ;
        if ( ( pFigure->GetCount() == 2 ) && ( Label.Find( "Straight" ) == 0 ) )
        {
          straight_segment NewSegment( CDPointToCmplx( pFigure->GetAt( 0 ) ) , CDPointToCmplx( ( pFigure->GetAt( 1 ) ) ) ) ;
          Segments.push_back( NewSegment ) ;
        }
        else if ( Label.Find( "Contur[edgeup" ) == 0 )
        {
          pEdgeFig = pFigure ;
          cmplx *pPts = ( cmplx* ) ( pFigure->GetData() ) ;
          cmplx * pIter = pPts ;
          cmplx * pEnd = pPts + pFigure->GetCount() ;
          do
          {
            if ( PtInRect( EdgeROI , *pIter ) && ( pIter->imag() > dMaxY ) )
            {
              dMaxY = pIter->imag() ;
              iIndexOfMaxY = pIter - pPts ;
            }
          } while ( ++pIter < pEnd ) ;
        }
      }
      delete Iterator ;
      CreateFullFrameCross( m_cLastROICent_pix , m_LastROI , pMarking , 0xff0000 ) ;
      CmplxVector Crosses ;
      if ( dMaxY > -1.e308 )
      {
        straight_segment Horiz( cmplx( 0. , dMaxY ) , cmplx( m_LastROI.right , dMaxY ) ) ;
        switch ( m_BGMforBasePoint )
        {
        case BGM_Sraights:
          {
            for ( int i = 0 ; i < ( ( int ) Segments.size() ) ; i++ )
            {
              cmplx Cross ;
              if ( ( ( CLine2d ) Segments[ i ] ).intersect( ( ( CLine2d ) Horiz ) , Cross ) )
              {
                if ( m_LastROI.PtInRect( CPoint( ROUND( Cross.real() ) , ROUND( Cross.imag() ) ) ) )
                  Crosses.push_back( Cross ) ;
              };
            }
          }
          break;
        case BGM_Corners:
          {
            if ( pEdgeFig )
            {
              GetBasePoint( pEdgeFig , Crosses , EdgeROI , iIndexOfMaxY , pMarking ) ;
            }
          }
          break ;
        }
      }

      if ( Crosses.size() == 2 )
      {
        m_cLastTipCenter = ( Crosses[ 0 ] + Crosses[ 1 ] ) / 2. ;
        pMarking->AddFrame( CreatePtFrame( m_cLastTipCenter ,
          "color=0x0000ff; Sz=5;" , "MaxY" ) ) ;
        m_cLastTipCenter -= m_cLastROICent_pix ;
        if ( Crosses[ 0 ].real() > Crosses[ 1 ].real() )
          std::swap( Crosses[ 0 ] , Crosses[ 1 ] ) ;
        m_cLastRightPt = Crosses[ 1 ];
        m_cLastRightPt -= m_cLastROICent_pix ;
        switch ( m_WorkingMode )
        {
        case WM_Green_Measure:
          {
            switch ( m_GadgetMode )
            {
            case GM_SIDE:
              {
                cmplx cViewPt = m_cLastRightPt ;
                CTextFrame * pCoords = CreateTextFrame( cmplx( 100. , m_cLastROICent_pix.imag() + 100 ) ,
                  "0x00ff00" , 24 , "Tip Coords" , pMarking->GetId() ,
                  "Yr=%.2fum\nZr=%.2fum\nRight cross" ,
                  cViewPt.real() * m_dScale_um_per_pix , cViewPt.imag() * m_dScale_um_per_pix ) ;
                pMarking->AddFrame( pCoords ) ;
                m_cLastLocked = m_cLastRightPtLocked = m_cLastRightPt ;
                cmplx cGreenCrossPt = cViewPt + m_cLastROICent_pix
                  + cmplx( m_iYOffset_um , m_iZOffset_um ) / m_dScale_um_per_pix ;
                CreateFullFrameCross( cGreenCrossPt , m_LastROI , pMarking , 0x000000ff ) ;
                m_dY_um = m_cLastLocked.real() * m_dScale_um_per_pix ;
                m_dZ_um = m_cLastLocked.imag() * m_dScale_um_per_pix ;
              }
              break ;
            case GM_FRONT:
              {
                cmplx cViewPt = m_cLastTipCenter ;
                CTextFrame * pCoords = CreateTextFrame( cmplx( 100. , m_cLastROICent_pix.imag() + 100 ) ,
                  "0xff8000" , 24 , "Tip Coords" , pMarking->GetId() ,
                  "Xtip=%.2fum\nZtip=%.2fum\nTip Center" ,
                  cViewPt.real() * m_dScale_um_per_pix , cViewPt.imag() * m_dScale_um_per_pix ) ;
                pMarking->AddFrame( pCoords ) ;
                m_cLastLocked = m_cLastTipCenter ;
                cmplx cGreenCrossPt = cViewPt + m_cLastROICent_pix 
                  + cmplx( m_iXOffset_um , 0. ) / m_dScale_um_per_pix ;
                CreateFullFrameCross( cGreenCrossPt , m_LastROI , pMarking , 0x000000ff ) ;
                m_dX_um = m_cLastTipCenter.real() * m_dScale_um_per_pix ;
                m_dZ_um = m_cLastTipCenter.imag() * m_dScale_um_per_pix ;
              }
              break ;
            }
          }
          break ;
        case WM_Green_Lock:
          {
            switch ( m_GadgetMode )
            {
            case GM_SIDE:
              {
                cmplx cOffset_um = cmplx( m_iYOffset_um , m_iZOffset_um ) ;
                cmplx cOffset_pix = cOffset_um / m_dScale_um_per_pix ;
                cmplx cTargetWidthOffset = m_cLastLocked + cOffset_pix ;
                cmplx cTarget = cTargetWidthOffset + m_cLastROICent_pix ;
                CreateFullFrameCross( cTarget , m_LastROI , pMarking , 0x000000ff ) ;

                cmplx cError = ( m_cLastRightPt - cTargetWidthOffset ) * m_dScale_um_per_pix ;
                cmplx cXViewPt = m_cLastROICent_pix + cmplx( -500. , 700 );
                cmplx cZViewPt = m_cLastROICent_pix + cmplx( -500. , 770 );
                CTextFrame * pX = CreateTextFrame( cXViewPt ,
                  fabs( cError.real() ) <= m_dTolerance_um ? "0x00ff00" : "0x4040ff" , 24 ,
                  "X" , pMarking->GetId() ,
                  "dY=%.2fum  %s" ,
                  cError.real() , ( cError.real() > 1. ) ? "Move Y Forward" :
                  ( cError.real() < -1. ) ? "Move Y Backward" : "OK" ) ;
                pMarking->AddFrame( pX ) ;
                CTextFrame * pZ = CreateTextFrame( cZViewPt ,
                  fabs( cError.imag() ) <= m_dTolerance_um ? "0x00ff00" : "0x0000ff" ,
                  24 , "Z" , pMarking->GetId() ,
                  "dZ=%.2fum  %s" , cError.imag() , ( cError.imag() > 1. ) ? "Move Z Up" :
                  ( cError.imag() < -1. ) ? "Move Z Down" : "OK" ) ;
                pMarking->AddFrame( pZ ) ;
                m_dY_um = cError.real() ;
                m_dZ_um = cError.imag() ;

                cmplx DiagViewPt = m_cLastROICent_pix + cmplx( -550. , 880 );
                CTextFrame * pDiag = CreateTextFrame( DiagViewPt ,
                  "0x0000ff" , 12 , "Diag" , pMarking->GetId() ,
                  "Locked=(%.1f,%.1f) Right=(%.1f,%.1f) Targ=(%.1f,%.1f)" ,
                  m_cLastLocked.real() , m_cLastLocked.imag() ,
                  m_cLastRightPt.real() , m_cLastRightPt.imag() ,
                  cTarget.real() , cTarget.imag() ) ;
                pMarking->AddFrame( pDiag ) ;

              }
              break ;
            case GM_FRONT:
              {
                // Draw Target cross
                cmplx cOffset_um = cmplx( m_iXOffset_um , 0. ) ;
                cmplx cOffset_pix = cOffset_um / m_dScale_um_per_pix ;
                cmplx cTargetWidthOffset = m_cLastLocked + cOffset_pix ;
                cmplx cTarget = cTargetWidthOffset + m_cLastROICent_pix ;
                CreateFullFrameCross( cTarget , m_LastROI , pMarking , 0x000000ff ) ;

                cmplx cError = ( m_cLastTipCenter - cTargetWidthOffset ) * m_dScale_um_per_pix ;
                cmplx cXViewPt = m_cLastROICent_pix + cmplx( -500. , 700 );
                cmplx cZViewPt = m_cLastROICent_pix + cmplx( -500. , 770 );
                CTextFrame * pX = CreateTextFrame( cXViewPt ,
                  fabs( cError.real() ) <= m_dTolerance_um ? "0x00ff00" : "0x0000ff" , 24 ,
                  "X" , pMarking->GetId() , "dX=%.2fum  %s" ,
                  cError.real() , ( cError.real() > 1. ) ? ( "Move X Left" ) :
                  ( cError.real() < -1. ) ? "Move X Right" : "OK" ) ;
                pMarking->AddFrame( pX ) ;
                CTextFrame * pZ = CreateTextFrame( cZViewPt ,
                  fabs( cError.imag() ) <= m_dTolerance_um ? "0x00ff00" : "0x4040ff" , 24 , "Z" ,
                  pMarking->GetId() , "dZ=%.2fum  %s" , cError.imag() ,
                  ( cError.imag() > 1. ) ? "Move Z Up" :
                  ( cError.imag() < -1. ) ? "Move Z Down" : "OK" ) ;
                pMarking->AddFrame( pZ ) ;
                m_dX_um = cError.real() ;
                m_dZ_um = cError.imag() ;
              }
              break ;
            }
          }
        }
      }
      for ( int i = 0 ; i < ( int ) Crosses.size() ; i++ )
        pMarking->AddFrame( CreatePtFrame( Crosses[ i ] , "Sz=5;color=0xff00ff;" ) ) ;
    }
  }

}

bool TwoChans::GetBasePoint( const CFigureFrame * pInFig ,
  CmplxVector& Crosses , CRect& WorkingZone , FXSIZE iMaxYIndex ,
  CContainerFrame * pMarking )
{
  cmplx * pFig = (cmplx*) pInFig->GetData() ;
  FXSIZE iLen = pInFig->Size() ;
  cmplx cPtMaxY =pFig[ iMaxYIndex ] ;
//   pMarking->AddFrame( CreatePtFrame( cPtMaxY , "color=0x0000ff; Sz=5;" , "MaxY" ) ) ;

  FXSIZE iRightIndex = iMaxYIndex ;
  FXSIZE iRightPlus10 = GetIndexInRing( (int) iRightIndex + 10 , iLen ) ;

  FXSIZE iIncrementToRight = (pFig[ iRightIndex ].real() > pFig[ iRightPlus10 ].real()) ? -1 : 1 ;

  // Find right edge
  iRightIndex = GetIndexInRing( (int) iMaxYIndex + (int) iIncrementToRight , iLen ) ;
  int iNSteps = 0 ;
  while ( (cPtMaxY.imag() - pFig[iRightIndex].imag()) < 150.  && iNSteps++ < 600 )
  {
    iRightIndex = GetIndexInRing( (int) iRightIndex + (int) iIncrementToRight , iLen ) ;
  }
  if ( iNSteps >= 600 )
    return false ;

  FXSIZE iLeftIndex = GetIndexInRing( (int) iMaxYIndex - (int) iIncrementToRight , iLen ) ;
  iNSteps = 0 ;
  while ( (cPtMaxY.imag() - pFig[ iLeftIndex ].imag()) < 150.  && iNSteps++ < 600 )
  {
    iLeftIndex = GetIndexInRing( (int) iLeftIndex - (int) iIncrementToRight , iLen ) ;
  }
  if ( iNSteps >= 600 )
    return false ;

  cmplx cRightPt = pFig[ iRightIndex ] ;
  cmplx cLeftPt = pFig[ iLeftIndex ] ;
  cmplx cRightLineTarget = cRightPt + polar( 250. , arg( cPtMaxY - cRightPt ) + DegToRad( m_dLineAngle ) ) ;
  cmplx cLeftLineTarget = cLeftPt + polar( 250. , arg( cPtMaxY - cLeftPt ) - DegToRad( m_dLineAngle ) ) ;

//   pMarking->AddFrame( CreatePtFrame( cRightPt , "color=0x00ff00; Sz=5;" , "RightPt" ) ) ;
//   pMarking->AddFrame( CreatePtFrame( cLeftPt , "color=0x00ffff; Sz=5;" ,  "LeftPt" ) ) ;
//   pMarking->AddFrame( CreatePtFrame( cRightLineTarget , "color=0x00ff00; Sz=5;" , "RightTarget" ) ) ;
//   pMarking->AddFrame( CreatePtFrame( cLeftLineTarget , "color=0x00ffff; Sz=5;" , "LeftTarget" ) ) ;

  cmplx cRightExtremum , cLeftExtremum ;
  FXSIZE iRightExtremumIndex , iLeftExtremumIndex ;
  double dRightMaxDist = GetMostOutlyingPoint( cRightPt , cRightLineTarget ,
    pInFig , cRightExtremum , iRightExtremumIndex ,
    (iIncrementToRight > 0) ? iMaxYIndex - 30 : iRightIndex ,
    (iIncrementToRight > 0) ? iRightIndex : iMaxYIndex + 30 ) ;

  double dLeftMaxDist = GetMostOutlyingPoint( cLeftLineTarget , cLeftPt ,
    pInFig , cLeftExtremum , iLeftExtremumIndex ,
    (iIncrementToRight > 0) ? iLeftIndex : iMaxYIndex - 30 ,
    (iIncrementToRight > 0) ? iMaxYIndex + 30 : iLeftIndex ) ;

  Crosses.push_back( cLeftExtremum ) ;
  Crosses.push_back( cRightExtremum ) ;

  return true ;
}
