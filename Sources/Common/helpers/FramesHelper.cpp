#include "stdafx.h"
#include <helpers/FramesHelper.h>
#include <..\..\gadgets\common/imageproc/seekspots.h>
#include <..\..\gadgets\common\imageproc\clusters\Segmentation.h>
#include <Math/FRegression.h>

CFigureFrame * CreateFigureFrame( cmplx * Pts , int iLen ,
  double dTime , const char * pColor , const char * pLabel , DWORD dwId )
{
  CFigureFrame * pFigure = CFigureFrame::Create() ;
  if (pFigure)
  {
    pFigure->SetSize( iLen ) ;
    memcpy_s( pFigure->GetData() , pFigure->GetCount() * sizeof( CDPoint ) ,
      Pts , iLen * sizeof( cmplx ) ) ;
    //     for ( int i = 0 ; i < iLen ; i++ )
    //       pFigure->Add( CDPoint( Pts[i].real() , Pts[i].imag() ) ) ;
    if (pColor)
      pFigure->Attributes()->WriteString( "color" , pColor ) ;
    if (pLabel)
      pFigure->SetLabel( pLabel ) ;
    pFigure->SetTime( dTime ) ;
    pFigure->ChangeId( dwId ) ;
  }
  return pFigure ;
}

CFigureFrame * CreateFigureFrame( cmplx * Pts , int iLen ,
  DWORD dwColor , const char * pLabel , DWORD dwId )
{
  CFigureFrame * pFigure = CFigureFrame::Create() ;
  if (pFigure)
  {
    pFigure->SetSize( iLen ) ;
    memcpy_s( pFigure->GetData() , pFigure->GetCount() * sizeof( CDPoint ) ,
      Pts , iLen * sizeof( cmplx ) ) ;
    FXString Color ;
    Color.Format( "0x%X" , dwColor ) ;
    pFigure->Attributes()->WriteString( "color" , Color ) ;
    if (pLabel)
      pFigure->SetLabel( pLabel ) ;
    pFigure->SetTime( GetHRTickCount() ) ;
    pFigure->ChangeId( dwId ) ;
  }
  return pFigure ;
}

CFigureFrame * CreateFigureFrame(CRect& Rect ,
  DWORD dwColor , const char * pLabel ,
  DWORD dwId , int iThickness ) 
{
  CFigureFrame * pFigFrame = CFigureFrame::Create();
  if (pFigFrame)
  {
    pFigFrame->AddPoint(CDPoint(Rect.left , Rect.top));
    pFigFrame->AddPoint(CDPoint(Rect.right , Rect.top));
    pFigFrame->AddPoint(CDPoint(Rect.right , Rect.bottom));
    pFigFrame->AddPoint(CDPoint(Rect.left , Rect.bottom));
    pFigFrame->AddPoint(CDPoint(Rect.left , Rect.top));
    pFigFrame->Attributes()->WriteLong("color" , (long) dwColor);
    if (iThickness)
      pFigFrame->Attributes()->WriteLong("thickness" , (long) iThickness);
    if (pLabel)
      pFigFrame->SetLabel(pLabel);
    if (dwId)
      pFigFrame->ChangeId(dwId);
  }
  return pFigFrame;
}

CFigureFrame * CreateCircleView( cmplx& cCenter , 
  double dRadius , DWORD dwCircColor )
{
  CFigureFrame * pCircle = CFigureFrame::Create() ;
  pCircle->SetSize( 361 ) ;
  for (int i = 0 ; i < 360 ; i++)
  {
    cmplx cPt = cCenter + dRadius * polar( 1. , i * M_PI / 180. ) ;
    pCircle->SetAt( i , CmplxToCDPoint( cPt ) ) ;
  }
  pCircle->SetAt( 360 , pCircle->GetAt( 0 ) ) ;

  FXString AsHex ;
  AsHex.Format( "color=0x%08X;" , dwCircColor ) ;
  *(pCircle->Attributes()) += AsHex ;
  return pCircle ;
}

bool CheckContainer( const CContainerFrame * pContainer ,
  int& iFirst , int& iSecond )
{
  CFramesIterator * Iter = pContainer->CreateFramesIterator( transparent ) ;
  if ( Iter )
  {
    FXPtrArray FramePtrs ;
    const CDataFrame* pNextFrame = Iter->Next( DEFAULT_LABEL );
    while ( pNextFrame )
    {
      FramePtrs.Add( (void*) pNextFrame ) ;
      pNextFrame = Iter->Next( DEFAULT_LABEL );
    }
    delete Iter;
    for ( int i = 0 ; i < FramePtrs.GetCount() ; i++ )
    {
      void * ptr = FramePtrs[ i ] ;
      for ( int j = i + 1 ; j < FramePtrs.GetCount() ; j++ )
      {
        if ( FramePtrs[ j ] == ptr )
        {
          iFirst = i ;
          iSecond = j ;
          return false ;
        }
      }
    }
  }
  return true ;
}

int FormFrameTextView(
  const CDataFrame * pDataFrame , FXString& outp ,
  FXString& Prefix , int& iNFrames , FXPtrArray& FramePtrs )
{
  if ( !pDataFrame )
    return 0 ;
  int iNScanned = 1 , iNChilds = 0 ;
  bool bContainer = pDataFrame->IsContainer() ;
  FXString tmpS , Addition ;
  tmpS.Format( "%s %d %s(%d) [%u-%u" , (LPCTSTR) Prefix , 
    iNFrames , bContainer ?
    _T( "Cont." ) : Tvdb400_TypeToStr( pDataFrame->GetDataType() ) ,
    pDataFrame->IsContainer() ?
    ((CContainerFrame*) pDataFrame)->GetFramesCount() : 1 ,
    pDataFrame->GetId() , pDataFrame->GetUserCnt() );
  Addition += tmpS;

  tmpS.Format( ":'%s'-p=0x%p]" , pDataFrame->GetLabel() , pDataFrame );
  Addition += tmpS;
  iNFrames++ ;
  FramePtrs.Add( (void*) pDataFrame ) ;
  if ( pDataFrame->IsContainer() )
  {
    Addition += _T( "\n" ) ;
    FXString OldPrefix = Prefix ;
    Prefix += _T( "  " ) ;
    CFramesIterator* Iterator = pDataFrame->CreateFramesIterator( transparent );
    if ( Iterator )
    {
      const CDataFrame* pNextFrame = Iterator->NextChild( DEFAULT_LABEL );
      while ( pNextFrame )
      {
        iNScanned++ ;
        iNChilds = FormFrameTextView( pNextFrame , Addition ,
          Prefix , iNFrames , FramePtrs );
        pNextFrame = Iterator->NextChild( DEFAULT_LABEL );
      }
      delete Iterator;
    }
    Prefix = OldPrefix ;
  }
  else
  {
    switch ( pDataFrame->GetDataType() )
    {
    case text:
      {
        const CTextFrame* TextFrame = pDataFrame->GetTextFrame();
        FXString Text = TextFrame->GetString();
        int iTextLen = (int) Text.GetLength();
        bool bIsCRLF = iTextLen
          && (Text[ iTextLen - 1 ] == '\r' || Text[ iTextLen - 1 ] == '\n');
        tmpS.Format( "%s%s" ,
          (LPCTSTR) Text , (bIsCRLF) ? "" : "\r\n" );
      }
      break;
    case quantity:
      {
        const CQuantityFrame* QuantityFrame =
          pDataFrame->GetQuantityFrame();
        tmpS.Format( "%s\r\n" ,
          QuantityFrame->ToString() );
      }
      break;
    case logical:
      {
        const CBooleanFrame* BooleanFrame =
          pDataFrame->GetBooleanFrame();
        tmpS.Format( "%s\r\n" ,
          (BooleanFrame->operator bool() ? "true" : "false") );
      }
      break;
    case rectangle:
      {
        const CRectFrame* RectFrame = pDataFrame->GetRectFrame();
        CRect rc = (LPRECT) RectFrame;
        tmpS.Format( "(%d,%d,%d,%d)\r\n" ,
          rc.left , rc.top , rc.right , rc.bottom );
      }
      break;
    case figure:
      {
        const CFigureFrame * pFig = pDataFrame->GetFigureFrame() ;
        tmpS.Format( "\t%d pts\r\n" , pFig->GetCount() ) ;
      }
      break ;
    case vframe:
      {
        const CVideoFrame * pFr = pDataFrame->GetVideoFrame() ;
        if ( pFr && pFr->lpBMIH )
        {
          tmpS.Format( "\t[%dx%d]\r\n" ,
            pFr->lpBMIH->biWidth , pFr->lpBMIH->biHeight ) ;
        }
      }
      break ;
    default:
      tmpS = "\r\n";
      break;
    }
    Addition += Prefix + tmpS ;
  }
  for ( int i = 0 ; i < FramePtrs.GetCount() ; i++ )
  {
    void * ptr = FramePtrs[ i ] ;
    for ( int j = i+ 1 ; j < FramePtrs.GetCount() ; j++ )
    {
      ASSERT ( FramePtrs[ j ] != ptr ) ;
    }
  }
  outp += Addition ;
  return iNScanned + iNChilds ;
}

bool IsInContainer( const CContainerFrame * pContainer ,
  const CDataFrame * pFrame )
{
  CFramesIterator * Iter = pContainer->CreateFramesIterator( transparent ) ;
  if ( Iter )
  {
    FXPtrArray FramePtrs ;
    const CDataFrame* pNextFrame = Iter->Next( DEFAULT_LABEL );
    while ( pNextFrame )
    {
      if ( (void*) pNextFrame == (void*) pFrame )
      {
        delete Iter ;
        return true ;
      }
      pNextFrame = Iter->Next( DEFAULT_LABEL );
    }
    delete Iter;
  }
  return false ;
}

double GetCrossPosOnStripWithView(
  cmplx& cBegin , // strip corner
  cmplx& cDirAlongStep , // strip direction
  cmplx& cDirCrossStep ,  // for viewing only
  int    iNAlongSteps ,
  int    iNCrossSteps , // Every cross step will be 90 degrees to the left
                        // with the same length
  double dThres ,
  double * pSignal , // average value of cross direction will be saved
                      // in this array for every step in along direction
  int iMaxSignalLen , 
  const pTVFrame pTV ,
  CContainerFrame * pDiagOut ) 
{
  if ( iNAlongSteps <= 0. )
    return 0. ;
  double dAver = 0. , dVal ;
  cmplx cAlongIter = cBegin ;
  //cmplx cCrossStep = GetOrthoRight( cDirAlongStep ) ;
  double dMin = DBL_MAX , dMax = -DBL_MAX ;
  for ( int iAlong = 0 ; iAlong < iNAlongSteps ; 
    cAlongIter += cDirAlongStep , iAlong++ )
  {
    dAver += (dVal = ( pSignal[ iAlong ] = GetAverageOnLineSafe(
      cAlongIter , cDirCrossStep , iNCrossSteps , pTV ) ) );
    SetMinMax( dVal , dMin , dMax ) ;
  }
  dAver /= iNAlongSteps ;
//   double dPos = find_border_forw( pSignal , iNAlongSteps , dThres ) ;
  // Variant with constant threshold 50%
  if ( dThres < 1. )
    dThres = dMin + (dMax - dMin) * dThres ;
  double dPos = find_border_forw( pSignal , iNAlongSteps , dThres ) ;
  if ( pDiagOut )
  {
    CFigureFrame * pStrip = CreateFigureFrame( &cBegin , 1 , (DWORD) 0xff0000 , "Strip" ) ;
    cmplx NextCorner = cBegin + cDirCrossStep * (double) iNCrossSteps ;
    pStrip->AddPoint( CmplxToCDPoint( NextCorner ) ) ;
    NextCorner += cDirAlongStep * (double) iNAlongSteps ;
    pStrip->AddPoint( CmplxToCDPoint( NextCorner ) ) ;
    NextCorner -= cDirCrossStep * (double) iNCrossSteps ;
    pStrip->AddPoint( CmplxToCDPoint( NextCorner ) ) ;
    pStrip->AddPoint( CmplxToCDPoint( cBegin ) ) ;
    pDiagOut->AddFrame( pStrip ) ;
  }
  return dPos ;
}

CFigureFrame * CreateGraphic( double * Pts , int iLen ,
  cmplx Origin , cmplx Step , cmplx Range , double dMin , double dMax ,
  const char * pColor , double dK , double dMaxAmpl )
{
  CFigureFrame * pFigure = CFigureFrame::Create() ;
  if ( pFigure )
  {
    // Draw axis
    cmplx XEnd = Origin + Step * (double) iLen ;
    cmplx YEnd = Origin + Range ;
    pFigure->Add( CDPoint( XEnd.real() , XEnd.imag() ) ) ;
    pFigure->Add( CDPoint( Origin.real() , Origin.imag() ) ) ;
    pFigure->Add( CDPoint( YEnd.real() , YEnd.imag() ) ) ;

    cmplx CurPos = Origin ;
    for ( int i = 0 ; i < iLen ; i++ )
    {
      cmplx cVect = dK * ( Range * ( Pts[ i ] - dMin ) / ( dMax - dMin ) ) ;
      double dAmpl = abs( cVect ) ;
      if (dMaxAmpl < dAmpl)
        cVect *= dMaxAmpl / dAmpl ;
      cmplx NextPt = CurPos + cVect ;
      pFigure->Add( CDPoint( NextPt.real() , NextPt.imag() ) ) ;
      CurPos += Step ;
    }
    if ( pColor )
      pFigure->Attributes()->WriteString( "color" , pColor ) ;
    pFigure->SetLabel( _T( "Graphic" ) ) ;
    pFigure->SetTime( GetHRTickCount() ) ;
    pFigure->ChangeId( 0 ) ;
  }
  return pFigure ;
}

void CreateFullFrameCross( cmplx& Pt , CRect& ROI ,
  CContainerFrame * POut , COLORREF Color )
{
  CFigureFrame * pVLine = CFigureFrame::Create() ;
  if ( pVLine )
  {
    pVLine->Add( CDPoint( Pt.real() , ROI.top ) ) ;
    pVLine->Add( CDPoint( Pt.real() , ROI.bottom ) ) ;
    ((FXPropKit2*) (pVLine->Attributes()))->WriteUIntNotDecimal( "color" , (UINT) Color ) ;
    pVLine->SetLabel( "PtBigCross" ) ;
    pVLine->SetTime( GetHRTickCount() ) ;
    pVLine->ChangeId( 0 ) ;
    POut->AddFrame( pVLine ) ;
  }
  CFigureFrame * pHLine = CFigureFrame::Create() ;
  if ( pHLine )
  {
    pHLine->Add( CDPoint( ROI.left , Pt.imag() ) ) ;
    pHLine->Add( CDPoint( ROI.right , Pt.imag() ) ) ;
    ((FXPropKit2*) (pHLine->Attributes()))->WriteUIntNotDecimal( "color" , (UINT) Color ) ;
    pHLine->SetLabel( "PtBigCross" ) ;
    pHLine->SetTime( GetHRTickCount() ) ;
    pHLine->ChangeId( 0 ) ;
    POut->AddFrame( pHLine ) ;
  }
}

bool ExtractCirclesByCenterAndRadius(
  const CVideoFrame * pVF ,
  cmplx& cOrigCenter , double dInitialRadius_pix , double dFinalRadius_pix ,
  double dThres , CircleExtractMode ExtractionMode , DiffThresMode DiffDirection ,
  CmplxVector& ResultPts ,
  CContainerFrame * pMarking ,
  int iNSegments , double dDeviation_pix )
{
  try
  {
#define MAX_SIGNAL_LEN 10000
    double Signal[MAX_SIGNAL_LEN];
    double dBufferForDiff[ MAX_SIGNAL_LEN ] ;
    double dScanStep_rad = M_2PI / iNSegments;
    double dScanDir_rad = 0.; // Direction for edge searching
    double dMinRad = min(dInitialRadius_pix, dFinalRadius_pix);
    double dAverRad = (dInitialRadius_pix + dFinalRadius_pix) * 0.5;
    double dDeviationAngle = acos(dAverRad / (dAverRad + 0.5));
    double dDeviationLen_pix = dAverRad * tan(dDeviationAngle);
    double dSegmentLength = (dAverRad * M_2PI) / iNSegments;
    if (dDeviationLen_pix > dSegmentLength * 0.5)
      dDeviationLen_pix = dSegmentLength * 0.5;
    int iDeviationLen_pix = (int)ceil(dDeviationLen_pix);
    if (iDeviationLen_pix < 1)
      iDeviationLen_pix = 1;
    int iScanRange_pix = (int)(dFinalRadius_pix - dInitialRadius_pix);
    double dScanRange_pix = (double)abs(iScanRange_pix);
    int iOrthoRange_pix = (iDeviationLen_pix * 2) + 1;
    int iScanStep = (iScanRange_pix < 0) ? -1 : 1;
    DWORD dwColor = 0x000000ff;
    ResultPts.clear();
    cmplx cCrossPt;
    CRect ViewArea(iOrthoRange_pix, iOrthoRange_pix, GetWidth(pVF) - iOrthoRange_pix, GetHeight(pVF) - iOrthoRange_pix);
    for (; dScanDir_rad < M_2PI; dScanDir_rad += dScanStep_rad)
    {
      cmplx cCurrDir = polar(1.0, dScanDir_rad);
      cmplx cScanStep = cCurrDir * (double)iScanStep;
      cmplx cOrthoStep = GetOrthoLeftOnVF(cScanStep);
      cmplx cInitialPt = cOrigCenter + cCurrDir * dInitialRadius_pix - cOrthoStep * dDeviationLen_pix;
      double dCrossPos = 0.;

      int iCurrentScanRange = abs(iScanRange_pix);
      bool bCorrected = false;
      // check that inspection area is inside image
      while (!ViewArea.PtInRect(CPoint(ROUND(cInitialPt.real()), ROUND(cInitialPt.imag())))
        && (iCurrentScanRange > 2))
      {
        cInitialPt += cScanStep;
        iCurrentScanRange--;
        bCorrected = true;
      }
      if (bCorrected) // indent for 2 pixels to inside of frame
      {
        cInitialPt -= cScanStep * 2.;
        iCurrentScanRange -= 2;
      }
      if (iCurrentScanRange <= 2)
        continue; // initial point could be out of image area

      if (!bCorrected)
      {
        cmplx cEndPt = cInitialPt + cScanStep * dScanRange_pix;
        while (!ViewArea.PtInRect(CPoint(ROUND(cInitialPt.real()), ROUND(cInitialPt.imag())))
          && (iCurrentScanRange > 2))
        {
          cEndPt -= cScanStep;
          iCurrentScanRange--;
          bCorrected = true;
        }
        if (bCorrected) // indent for 2 pixels to inside of frame
        {
          cEndPt -= cScanStep * 2.;
          iCurrentScanRange -= 2;
        }
        if (iCurrentScanRange <= 2)
          continue; // end point could be out of image area
      }

      double dAverSignal = GetAverageSignalOnStrip(cInitialPt, cScanStep,
        iCurrentScanRange, iOrthoRange_pix, Signal, MAX_SIGNAL_LEN, pVF);

      double dMin = DBL_MAX, dMax = -DBL_MAX;
      GetMinMaxDbl(Signal, iCurrentScanRange , dMin, dMax);
      double dAbsThres = 0., dRelThres = 0.;
      switch (ExtractionMode)
      {
      case CEM_FirstEdge:
      {
        dAbsThres = GetThresByMinMax(Signal, iCurrentScanRange , dThres);
        dCrossPos = find_slope_forw(Signal, iCurrentScanRange , dAbsThres, Signal[0] < dAbsThres);
      }
      break;
      case CEM_ExtremWithNeightbours:
      {
        dAbsThres = GetThresByMinMax(Signal, iCurrentScanRange , dThres);
        dCrossPos = find_extrem_pos_by3(Signal, iCurrentScanRange ,
          dMin, dMax, true);
      }
      break;
      case CEM_FirstDiff:
      {
        dRelThres = GetDiffThresByMinMax(Signal,
          iCurrentScanRange , dThres, DiffDirection , dBufferForDiff );
        dCrossPos = find_diff_border_forw(Signal, 
          iCurrentScanRange , dRelThres , dBufferForDiff );
      }
      break;
      case CEM_Extrem:
      {
        double dMax;
        dCrossPos = GetMaxPos(Signal, iCurrentScanRange , dMax);
      }
      break;
      }

      if (dCrossPos)
      {
        cCrossPt = cInitialPt + cOrthoStep * (iOrthoRange_pix * 0.5)
          + cScanStep * dCrossPos;
        ResultPts.push_back(cCrossPt);
      }
    }
    return true;
  }
  catch (...)
  {
    FxSendLogMsg( MSG_WARNING_LEVEL ,
      "ExtractCircleByCenterAndRadius" , 0 , "Memory Access exception" ) ;
    return false;
  }
  return false;
}

size_t GetContrastLine( cmplx cInitialPt , cmplx cInitalLineDirection , 
  double dStripRange , CmplxVector& PtsOnLine ,const CVideoFrame * pVF )
{
  int iStripRange = ROUND( dStripRange ) ;
  cmplx cLineDirection = GetNormalized( cInitalLineDirection ) ;
  cmplx cStripDir = GetOrthoRightOnVF( cLineDirection ) ;

  double dInitialContrast = 0. ;
  int iOmittedDistance = 0 ;
  // Is point in safe range from video frame edges?
  while ( IsPtInFrameSafe( cInitialPt , pVF , dStripRange + 1. ) )
  {  
    cmplx cInitialScanPt = cInitialPt - cStripDir * dStripRange ;
    cmplx cEndScanPt = cInitialScanPt + cStripDir * ( 2. * dStripRange ) ;
#define MAX_ORTHO_STRIP_LENGTH 500 
    double OrthoStrip[ MAX_ORTHO_STRIP_LENGTH ] ;
    int iLen = GetPixelsOnLine( cInitialScanPt , cStripDir ,
      2 * dStripRange , OrthoStrip , MAX_ORTHO_STRIP_LENGTH , pVF ) ;

    double dCentBr = OrthoStrip[ iStripRange ] ;
    double dLeftBr = OrthoStrip[0] ;
    double dRightBr = OrthoStrip[ iLen - 1 ] ;
    double dContrastToLeft = dCentBr - dLeftBr ;
    double dContastToRight = dCentBr - dRightBr ;
    double dContrast = min( fabs( dContastToRight ) , fabs( dContrastToLeft ) ) ;
    if ( PtsOnLine.size() == 0 )
      dInitialContrast = dContrast ;
    else if ( dContrast < dInitialContrast * 0.5 )
    {
      cInitialPt += cLineDirection ;
      if ( ++iOmittedDistance < iStripRange * 3 )
        continue ;
      else
        break ;
    }
    else
      iOmittedDistance = 0 ;

    bool bWhiteLine = ( (dContrastToLeft > 0.) && (dContastToRight > 0.) ) ;
    bool bBlackLine = ( ( dContrastToLeft < 0. ) && ( dContastToRight < 0. ) )  ;
    if ( !bBlackLine && !bWhiteLine )
      break ; // end of line on image

    
    double dThres = ( dCentBr + 0.5 * ( dLeftBr + dRightBr ) ) / 2. ;

    double * pdIter = OrthoStrip ;
    double * pdEnd = OrthoStrip + iLen ;

    double dLeftEdge = 0 , dRightEdge = 0. ;

    while ( pdIter < pdEnd )
    {
      if ( bBlackLine )
      {
        while ( ( *pdIter >= dThres ) && ( ++pdIter < pdEnd ) ) ;
      }
      else
      {
        while ( ( *pdIter < dThres ) && ( ++pdIter < pdEnd ) ) ;
      }
      if ( pdIter >= pdEnd )
        break ;
      dLeftEdge = GetThresPosition( *( pdIter - 1 ) , *pdIter , dThres ) ;
      dLeftEdge += ( pdIter - OrthoStrip - 1 ) ;


      if ( bBlackLine )
      {
        while ( ( *pdIter < dThres ) && ( ++pdIter < pdEnd ) ) ;
      }
      else
      {
        while ( ( *pdIter >= dThres ) && ( ++pdIter < pdEnd ) ) ;
      }
      if ( pdIter >= pdEnd )
        break ;
      dRightEdge = GetThresPosition( *( pdIter - 1 ) , *pdIter , dThres ) ;
      dRightEdge += ( pdIter - OrthoStrip - 1 ) ;
    }

    if ( ( dLeftEdge != 0. ) && ( dRightEdge != 0. ) )
    {
      double dLineCenter = 0.5 * ( dLeftEdge + dRightEdge ) ;
      cmplx cLineCenter = cInitialScanPt + cStripDir * dLineCenter ;
      PtsOnLine.push_back( cLineCenter ) ;
      if ( PtsOnLine.size() > 10 )
      {
        cLineDirection = (PtsOnLine.back() - PtsOnLine.at( PtsOnLine.size() - 11 )) * 0.1 ;
        cLineDirection = GetNormalized( cLineDirection ) ;
        cStripDir = GetOrthoRightOnVF( cLineDirection ) ;
      }
    }
    cInitialPt += cLineDirection ;
  }
  return PtsOnLine.size() ;
}

int FormQuestionWithRectOnRender( cmplx& ViewPt ,
  LPCTSTR pQuestion , DWORD iFontSize , DWORD dwColor , 
  vector<CRect>& Zones ,
  CContainerFrame * pMarking )
{
  CSize QuestionSize_pix = GetStringSizeOnRenderInPix( pQuestion , iFontSize ) ;
 
  QuestionSize_pix.cx += 6 ;
  QuestionSize_pix.cy += 6 ;
  CPoint LT( ROUND( ViewPt.real() -3 ), ROUND( ViewPt.imag() - 3 ) ) ;
  CRect QuestionRect( LT , QuestionSize_pix ) ;

  CFigureFrame * pRectView = CreateFigureFrame( QuestionRect , dwColor ) ;
  if ( pRectView )
  {
    *(pRectView->Attributes()) += "thickness=3;" ;
    pMarking->AddFrame( pRectView ) ;

    pMarking->AddFrame( CreateTextFrame( ViewPt , pQuestion , dwColor , iFontSize ) ) ;

    Zones.push_back( QuestionRect ) ;
    ViewPt += ( QuestionSize_pix.cx - 6.) ;
    return (int)Zones.size() ;
  }
  return 0 ;
}

int FormQuestionsOnRender( cmplx& ViewPt ,
  LPCTSTR pInfo , LPCTSTR * pQuestions , 
  DWORD iFontSize , DWORD dwColor , vector<CRect>& Zones ,
  CContainerFrame * pMarking )
{
  pMarking->AddFrame( CreateTextFrame( ViewPt , pInfo , dwColor , iFontSize ) ) ;

  CSize InfoSize_pix = GetStringSizeOnRenderInPix( pInfo , iFontSize ) ;

  cmplx cQuestionLT = ViewPt + cmplx( 0. , InfoSize_pix.cy ) ;

  while ( pQuestions )
  {
    if ( !FormQuestionWithRectOnRender( ViewPt , *pQuestions , iFontSize , dwColor , Zones , pMarking ) )
    {
      FxSendLogMsg( MSG_ERROR_LEVEL , "FormQuestionsOnRender" , 0 , 
        "ERROR QIndex=%d Q=%s" , (int)Zones.size() , *pQuestions ) ;
      break ;
    }

    pQuestions++ ;
  }

  return (int)Zones.size() ;
};

bool PutTextsSeparated(
  COutputConnector * pPin , const FXString& TextData ,
  LPCTSTR pSeparator , LPCTSTR pLabel , DWORD dwDelay_ms , DWORD * piFirstID ,
  const FXString& cmdWithDelayAfter , DWORD dwDelay4CmdWithDelayAfter_ms )
{
  FXSIZE iPos = 0;
  FXString NextToken = TextData.Tokenize( pSeparator , iPos );
  while ( iPos >= 0 )
  {
    NextToken += pSeparator;
    CTextFrame * pNextFrame = CreateTextFrame( NextToken ,
      pLabel , piFirstID ? ( *piFirstID )++ : 0 );
    if ( !pPin->Put( pNextFrame ) )
    {
      TRACE( "\nCan't send frame ""%s"" to pin ""%s"" " ,
        pLabel , pPin->GetName() );
      pNextFrame->Release();
      return false;
    }
    if ( NextToken.CompareNoCase( cmdWithDelayAfter ) == 0 && dwDelay4CmdWithDelayAfter_ms > 0 )
      Sleep( dwDelay4CmdWithDelayAfter_ms );
    else if ( dwDelay_ms )
      Sleep( dwDelay_ms );
    NextToken = TextData.Tokenize( pSeparator , iPos );
  }
  return true;
}

const CDataFrame * GetFrameWithLabel( const CDataFrame * pSrc ,
  datatype type , LPCTSTR pLabelPart , DWORD dwMask )
{
  WordPart InLabel = ( WordPart ) ( dwMask & ~0x8000000 ) ;
  bool bCaseInsensitive = ( dwMask & 0x80000000 ) != 0 ;
  FXString PatternLab( pLabelPart ) ;
  if ( bCaseInsensitive )
    PatternLab = PatternLab.MakeUpper() ;
  if ( ( InLabel == WP_Full ) )
  {
    if ( !bCaseInsensitive )
    {
      const CDataFrame * pFound = pSrc->GetDataFrame( pLabelPart ) ;
      if ( pFound && ( pFound->GetDataType() == type ) )
        return pFound ;
      else
        return NULL ;
    }
    else if ( !pSrc->IsContainer() && ( pSrc->GetDataType() == type ) )
    {
      FXString Label = pSrc->GetLabel() ;
      if ( Label.CompareNoCase( pLabelPart ) == 0 )
        return pSrc ;
      else
        return NULL ;
    }
  }
  CFramesIterator * Iter = pSrc->CreateFramesIterator( transparent ) ;
  const CDataFrame * pFinal = NULL ;
  if ( Iter )
  {
    const CDataFrame * pNext = NULL ;
    while ( pNext = Iter->Next() )
    {
      if ( pNext->GetDataType() != type )
        continue ;
      FXString Label = pNext->GetLabel() ;
      if ( bCaseInsensitive )
        Label = Label.MakeUpper() ;
      int iPos = ( int ) Label.Find( PatternLab ) ;
      if ( iPos < 0 )
        continue ;

      switch ( InLabel )
      {
        case WP_Begin:
          if ( iPos == 0 )
            pFinal = pNext ;
          break ;
        case WP_Any:
          if ( iPos >= 0 )
            pFinal = pNext ;
          break ;
        default:
          break;
      }
      if ( pFinal )
        break ;
    }
    delete Iter ;
  }
  return pFinal ;
}

size_t GetVerticalContrastLine( cmplx cInitialPt , int iStripRange ,
  CmplxVector& PtsOnLine , double dNormThres , int iMinAmpl , 
  const CVideoFrame * pVF , double& dLineWidth ,
  DoubleVector * pdAvoidPointsY )
{
  double dInitialContrast = 0. ;
  int iOmittedDistance = 0 ;
  bool bY16 = false ; // otherwise 8 bits intensity
  dLineWidth = 0. ;
  switch ( GetCompression( pVF ) )
  {
    case BI_Y8:
    case BI_Y800:
    case BI_YUV9:
    case BI_YUV12:    break ;
    case BI_Y16: bY16 = true ; break ;
    default: return 0 ;
  }
  int iNAvoidPoints = pdAvoidPointsY ? ( int ) pdAvoidPointsY->size() : 0 ;
  int iFrameWidth = ( int ) GetWidth( pVF ) ;
  int iFrameHeight = ( int ) GetHeight( pVF ) ;
  int iImageSize = iFrameHeight * iFrameWidth * ( ( bY16 ) ? 2 : 1 ) ;
  int iScanWidth = 2 * iStripRange ;

  int iXFrom = ROUND( cInitialPt.real() ) ;
  if ( iXFrom < iStripRange )
    return 0 ;
  if ( iXFrom + iStripRange > iFrameWidth - (iStripRange/3) )
    return 0 ;

  int iXSavedFrom = iXFrom ;

  const LPBYTE pImage = GetData( pVF ) ;
  const LPBYTE pImageEnd = pImage + iImageSize ;
  int iNOmitted = 0 ;
  double dWidth = 0 , dLinePos = 0. ;
  double dAccumWidth = 0. , dAverWidth = 0. ;
  int iYMin = 5 ;
  int iYMax = iFrameHeight - 5 ;
  int iYMinLimit = iYMin , iYMaxLimit = iYMax ;
  int iBiggerIndex = -1 ;
  bool bUseAvoidPts = iNAvoidPoints ;
  int iYFirstAvoid = bUseAvoidPts ? ROUND( pdAvoidPointsY->at( 0 ) ) : -1 ;
  int iYLastAvoid = bUseAvoidPts ? ROUND( pdAvoidPointsY->back() ) : -1 ;
  int iInitialY = ROUND( cInitialPt.imag() ) ;

  if ( bUseAvoidPts )
  {
    int iUpperY = ( iYFirstAvoid != iYLastAvoid ) ?
      iYFirstAvoid + iStripRange : iYMin ;
    if ( iUpperY > iYMin )
      iYMinLimit = iYMin = iUpperY ;
    int iLowerY = ( iYFirstAvoid != iYLastAvoid ) ?
      iYLastAvoid - iStripRange : iYMaxLimit ;
    if ( iLowerY < iYMax )
      iYMaxLimit = iYMax = iLowerY ;

    for ( int i = 0 ; i < iNAvoidPoints ; i++ )
    {

      if ( ( int ) ( pdAvoidPointsY->at( i ) ) > iInitialY )
      {
        iBiggerIndex = i ;
        break ;
      }
    }
  }
  int iCurrentIndex = iBiggerIndex ;
  if ( ( iCurrentIndex > 0 ) || !bUseAvoidPts )
  {
    int iMaxYTmp = ( bUseAvoidPts ) ?
      ROUND( pdAvoidPointsY->at( iCurrentIndex - 1 ) ) : iYMax ;
    if ( bUseAvoidPts )
    {
      if ( iInitialY > iMaxYTmp + iStripRange )
        iYMinLimit = iMaxYTmp + iStripRange ;
      else
      {
        iInitialY = iMaxYTmp - iStripRange ;
        if ( iBiggerIndex > 1 )
        {
          iYMinLimit = ROUND( pdAvoidPointsY->at( --iCurrentIndex - 1 ) )
            + iStripRange ;
        }
      }
    }
    do
    {
      for ( int iY = iInitialY ; iY >= iYMinLimit ; iY-- )
      {
        if ( bY16 )
        {
          const LPWORD pPixFragment = ( LPWORD ) pImage + iY * iFrameWidth + iXFrom ;
          dLinePos = find_line_pos_lr( pPixFragment , iStripRange ,
            dNormThres , iMinAmpl , dWidth ) ;
        }
        else
        {
          const LPBYTE pPixFragment = pImage + iY * iFrameWidth + iXFrom ;
          dLinePos = find_line_pos_lr( pPixFragment , iStripRange ,
            dNormThres , iMinAmpl , dWidth ) ;
        }
        if ( fabs( dLinePos ) < ( 1.3 * ( ( iNOmitted > 6 ) ? 5. : 1. ) ) )
        {
          dLinePos += iXFrom ;
          cmplx Pt( dLinePos , iY ) ;
          PtsOnLine.push_back( Pt ) ;
          iXFrom = ROUND( dLinePos ) ;
          dAccumWidth += dWidth ;
          dAverWidth = dAccumWidth / PtsOnLine.size() ;
          // To do: Check for inside image
          iNOmitted = 0 ;
        }
        else
        {
          if ( ++iNOmitted >= iScanWidth )
            break ;
        }
      }
      if ( iCurrentIndex <= 1 )
        break ;
      else if ( bUseAvoidPts )
      {
        iInitialY = iMaxYTmp = ROUND( pdAvoidPointsY->at( --iCurrentIndex ) )
          - iStripRange ;
        if ( iCurrentIndex > 0 )
        {
          iYMinLimit = ROUND( pdAvoidPointsY->at( iCurrentIndex - 1 ) )
            + iStripRange ;
        }
        else
          iYMinLimit = iYMin ;
      }
    } while ( iYMinLimit >= iYMin );
    std::reverse( PtsOnLine.begin() , PtsOnLine.end() ) ;
  }
  if ( !PtsOnLine.size() )
    return 0 ;

  cmplx cContinuePt = PtsOnLine.back() ;
  iInitialY = ROUND( cContinuePt.imag() ) + 1 ;
  int iXTo = iXFrom + 2 * iStripRange ;
  iNOmitted = 0 ;

  if ( !bUseAvoidPts || ( iBiggerIndex <= ( iNAvoidPoints - 1 ) ) )
  {
    int iYaxLimit = ( ( bUseAvoidPts ) ?
      ROUND( pdAvoidPointsY->at( iBiggerIndex ) ) - iStripRange : iYMax ) ;
    if ( bUseAvoidPts && iYMaxLimit <= iInitialY )
    {
      iInitialY = ROUND( pdAvoidPointsY->at( iBiggerIndex ) )
        + iStripRange ;
      iYMaxLimit = (( iBiggerIndex < ( iNAvoidPoints - 1 ) ) ?
        ROUND( pdAvoidPointsY->at( ++iBiggerIndex ) ) : iFrameHeight ) - iStripRange ;
    }

    do
    {
      for ( int iY = ROUND( cInitialPt.imag() ) + 1 ; iY < iYMaxLimit ; iY++ )
      {
        if ( bY16 )
        {
          const LPWORD pPixFragment = ( LPWORD ) pImage + iY * iFrameWidth + iXFrom ;
          dLinePos = find_line_pos_lr( pPixFragment , iScanWidth ,
            dNormThres , iMinAmpl , dWidth ) ;
        }
        else
        {
          const LPBYTE pPixFragment = pImage + iY * iFrameWidth + iXFrom ;
          dLinePos = find_line_pos_lr( pPixFragment , iScanWidth ,
            dNormThres , iMinAmpl , dWidth ) ;
        }
        if ( fabs( dLinePos ) < ( 1.3 * ( ( iNOmitted > 6 ) ? 5. : 1. ) ) )
        {
          dLinePos += iXFrom ;
          cmplx Pt( dLinePos , iY ) ;
          PtsOnLine.push_back( Pt ) ;
          iXFrom = ROUND( dLinePos ) ;
          dAccumWidth += dWidth ;
          // To do: Check for inside image
          iNOmitted = 0 ;
        }
        else
        {
          if ( ++iNOmitted >= iScanWidth )
            break ;
        }
      }
      if ( bUseAvoidPts
        && ( iBiggerIndex < ( iNAvoidPoints - 1 ) ) )
      {
        iInitialY = ROUND( pdAvoidPointsY->at( iBiggerIndex ) )
          + iStripRange ;
        iYMaxLimit = ROUND( pdAvoidPointsY->at( ++iBiggerIndex ) )
          - iStripRange ;
      }
      else
        break ;
    } while ( iYMaxLimit <= iYMax );
  }

  dLineWidth = PtsOnLine.size() ? (dAccumWidth /  PtsOnLine.size()) : 0. ;
  return PtsOnLine.size() ;
}

size_t GetPixelsOnVerticalLine8( CPoint ptBegin , int iStep ,
  LPBYTE pResult , size_t ResultLen , const CVideoFrame * pVF , int& iMin , int& iMax ) 
{
  LPBYTE pData = GetData( pVF ) ;
  LPBYTE pEnd = pData + GetImageSizeWH( pVF ) ;
  LPBYTE pPt = (LPBYTE)GetLine( pVF , ptBegin.y) ; // ptr to first pixel
  LPBYTE pBegin = pResult ;

  int iHeight = GetHeight( pVF ) ;
  int iWidth = GetWidth( pVF ) ;
  if ( iStep > 0 )
  {
    while ( ResultLen-- && (pPt < pEnd) )
    {
      int iVal = *(pResult++) = *pPt ;
      SetMinMax( iVal , iMin , iMax ) ;
      pPt += iWidth ;
    }
    return ( pResult - pBegin ) ;
  }
  else
  {
    while ( ResultLen-- && ( pPt > pData ) )
    {
      int iVal = *( pResult++ ) = *pPt ;
      SetMinMax( iVal , iMin , iMax ) ;
      pPt -= iWidth ;
    }
    return ( pResult - pBegin ) ;
  }
  return 0 ;
}

size_t GetHorizontalContrastLine( cmplx cInitialPt , int iStripRange ,
  CmplxVector& PtsOnLine , double dNormThres , int iMinAmpl ,
  const CVideoFrame * pVF , double& dLineWidth , 
  DoubleVector * pdAvoidPointsX )
{
  double dInitialContrast = 0. ;
  int iOmittedDistance = 0 ;
  dLineWidth = 0. ;
  bool bY16 = false ; // otherwise 8 bits intensity
  switch ( GetCompression( pVF ) )
  {
    case BI_Y8:
    case BI_Y800:
    case BI_YUV9:
    case BI_YUV12:    break ;
    case BI_Y16: bY16 = true ; break ;
    default: return 0 ;
  }
  int iYFrom = ROUND( cInitialPt.imag() ) ;
  if ( iYFrom < iStripRange )
    return 0 ;
  if ( iYFrom + iStripRange > ( int )GetHeight( pVF ) - (iStripRange/3) )
    return 0 ;

  int iNAvoidPoints = pdAvoidPointsX ? (int)pdAvoidPointsX->size() : 0 ;
  int iFrameWidth = ( int ) GetWidth( pVF ) ;
  int iFrameHeight = ( int ) GetHeight( pVF ) ;
  int iImageSize = iFrameHeight * iFrameWidth * (( bY16 ) ? 2 : 1) ;
  int iScanWidth = 2 * iStripRange ;

  const LPBYTE pImage = GetData( pVF ) ;
  const LPBYTE pImageEnd = pImage + iImageSize ;
  int iNOmitted = 0 ;
  double dWidth = 0 , dLinePos = 0. ;
  double dAccumWidth = 0. ;
  int iInitialX = ROUND( cInitialPt.real() ) ;
  int iXMin = 5 ;
  int iXMax = iFrameWidth - 5 ;
  int iXMinLimit = iXMin , iXMaxLimit = iXMax ;
  int iBiggerIndex = -1 ;
  bool bUseAvoidPts = iNAvoidPoints ;
  int iXFirstAvoid = bUseAvoidPts ? ROUND( pdAvoidPointsX->at( 0 ) ) : -1 ;
  int iXLastAvoid = bUseAvoidPts ? ROUND( pdAvoidPointsX->back() ) : -1 ;
  // Go to X- (to the left)
  if ( bUseAvoidPts )
  {
    int iLeftX = ( iXFirstAvoid != iXLastAvoid ) ?
      iXFirstAvoid + iStripRange : iXMin ;
    if ( iLeftX > iXMin )
      iXMinLimit = iXMin = iLeftX ;
    int iRightX = ( iXFirstAvoid != iXLastAvoid ) ? 
      iXLastAvoid - iStripRange : iXMaxLimit ;
    if ( iRightX < iXMax )
      iXMaxLimit = iXMax = iRightX ;

    for ( int i = 0 ; i < iNAvoidPoints ; i++ )
    {

      if ( (int)(pdAvoidPointsX->at(i)) > iInitialX )
      {
        iBiggerIndex = i ;
        break ;
      }
    }
  }
  int iCurrentIndex = iBiggerIndex ;
  if ( (iCurrentIndex > 0) || !bUseAvoidPts )
  {
    int iMaxXTmp = ( bUseAvoidPts ) ? 
      ROUND( pdAvoidPointsX->at(iCurrentIndex - 1 ) ) : iXMax ;
    if ( bUseAvoidPts )
    {
      if ( iInitialX > iMaxXTmp + iStripRange )
        iXMinLimit = iMaxXTmp + iStripRange ;
      else
      {
        iInitialX = iMaxXTmp - iStripRange ;
        if ( iBiggerIndex > 1 )
        {
          iXMinLimit = ROUND( pdAvoidPointsX->at( --iCurrentIndex - 1 ) )
            + iStripRange ;
        }
      }
    }
    do 
    {
      for ( int iX = iInitialX ; iX >= iXMinLimit ; iX-- )
      {
        if ( ( iYFrom > iStripRange ) && ( iYFrom < iFrameHeight - iStripRange ) )
        {
          int iRealScanWidth = iScanWidth ;
          if ( iYFrom <= ( iScanWidth >> 1 ) )
            iRealScanWidth = ( iYFrom - 1 ) * 2 ;
          else if ( ( iFrameHeight - ( iScanWidth >> 1 ) ) <= iYFrom )
            iRealScanWidth = ( iFrameHeight - iYFrom - 1 ) * 2 ;
          if ( bY16 )
          {
            const LPWORD pPixFragment = ( LPWORD ) pImage + iYFrom * iFrameWidth + iX ;
              dLinePos = find_line_pos_ud( pPixFragment , iRealScanWidth ,
                dNormThres , iMinAmpl , dWidth , iFrameWidth ) ;
          }
          else
          {
            const LPBYTE pPixFragment = pImage + iYFrom * iFrameWidth + iX ;
            dLinePos = find_line_pos_ud( pPixFragment , iRealScanWidth ,
              dNormThres , iMinAmpl , dWidth , iFrameWidth ) ;
          }
        }
        else
          dLinePos = iStripRange ;

        if ( fabs( dLinePos ) < ( 1.3 * ( ( iNOmitted > 6 ) ? 5. : 1. ) ) )
        {
          dLinePos += iYFrom ;
          cmplx Pt( iX , dLinePos ) ;
          PtsOnLine.push_back( Pt ) ;
          iYFrom = ROUND( dLinePos ) ;
          dAccumWidth += dWidth ;
          // To do: Check for inside image
          iNOmitted = 0 ;
        }
        else
        {
          if ( ++iNOmitted >= iScanWidth )
            break ;
        }
      }
      if ( iCurrentIndex <= 1 )
        break ;
      else if ( bUseAvoidPts )
      {
        iInitialX = iMaxXTmp = ROUND( pdAvoidPointsX->at( --iCurrentIndex ) ) 
          - iStripRange ;
        if ( iCurrentIndex > 0 )
        {
          iXMinLimit = ROUND( pdAvoidPointsX->at( iCurrentIndex - 1 ) )
            + iStripRange ;
        }
        else
          iXMinLimit = iXMin ;
      }
    } while ( iXMinLimit >= iXMin );
    std::reverse( PtsOnLine.begin() , PtsOnLine.end() ) ;

  }

  if ( !PtsOnLine.size() )
    return 0 ;
  cmplx cContinuePt = PtsOnLine.back() ;
  iYFrom = ROUND( cContinuePt.imag() );

  iNOmitted = 0 ;
  iInitialX = ROUND( cInitialPt.real() ) + 1 ;
  if ( !bUseAvoidPts || (iBiggerIndex <= (iNAvoidPoints - 1) ))
  {
    int iXMaxLimit = (
      ( bUseAvoidPts ) ?
            ROUND( pdAvoidPointsX->at( iBiggerIndex ) ) - iStripRange : iXMax)
       ;
    if ( bUseAvoidPts && iXMaxLimit <= iInitialX )
    {
      iInitialX = ROUND( pdAvoidPointsX->at( iBiggerIndex ) )
        + iStripRange ;
      iXMaxLimit = ( ( iBiggerIndex < ( iNAvoidPoints - 1 ) )?
        ROUND( pdAvoidPointsX->at( ++iBiggerIndex ) ) : iFrameWidth ) - iStripRange ;
    }
    do 
    {
      for ( int iX = iInitialX ; iX <= iXMaxLimit ; iX++ )
      {
        int iRealScanWidth = iScanWidth ;
        if ( iYFrom <= ( iScanWidth >> 1 ) )
          iRealScanWidth = ( iYFrom - 1 ) * 2 ;
        else if ( ( iFrameHeight - ( iScanWidth >> 1 ) ) <= iYFrom )
          iRealScanWidth = ( iFrameHeight - iYFrom - 1 ) * 2 ;
        if ( bY16 )
        {
          const LPWORD pPixFragment = ( LPWORD ) pImage + ( iYFrom * iFrameWidth + iX ) ;
          dLinePos = find_line_pos_ud( pPixFragment , iRealScanWidth ,
            dNormThres , iMinAmpl , dWidth , iFrameWidth ) ;
        }
        else
        {

          const LPBYTE pPixFragment = pImage + ( iYFrom * iFrameWidth + iX ) ;
          dLinePos = find_line_pos_ud( pPixFragment , iRealScanWidth ,
            dNormThres , iMinAmpl , dWidth , iFrameWidth ) ;
        }
        if ( fabs( dLinePos ) < ( 1.3 * ( ( iNOmitted > 6 ) ? 5. : 1. ) ) )
        {
          dLinePos += iYFrom ;
          cmplx Pt( iX , dLinePos ) ;
          PtsOnLine.push_back( Pt ) ;
          iYFrom = ROUND( dLinePos ) ;
          dAccumWidth += dWidth ;
          // To do: Check for inside image
          iNOmitted = 0 ;
        }
        else
        {
          if ( ++iNOmitted >= iScanWidth )
            break ;
        }
      }
      if ( bUseAvoidPts 
        && (iBiggerIndex < ( iNAvoidPoints - 1 )) )
      {
        iInitialX = ROUND( pdAvoidPointsX->at( iBiggerIndex ) )
          + iStripRange ;
        iXMaxLimit = ROUND( pdAvoidPointsX->at( ++iBiggerIndex ) )
          - iStripRange ;
      }
      else
        break ;
    } while ( iXMaxLimit <= iXMax );
  }

  // Restore original positions for original point

 
  dLineWidth = PtsOnLine.size() ? ( dAccumWidth / PtsOnLine.size() ) : 0. ;
  return PtsOnLine.size() ;
}

CFigureFrame * CreateHGraphForDraw( Profile& HorProfile , CRect HorProfileROI )
{
  double dAmplX = HorProfile.m_dMaxValue - HorProfile.m_dMinValue ;
  double dScaleY = ( dAmplX <= 1. ) ? 0. : HorProfileROI.bottom / dAmplX ;
  cmplx cOrigin( HorProfile.m_iProfOrigin , HorProfileROI.top + HorProfileROI.bottom ) ;
  cmplx cEnd( HorProfileROI.right , HorProfileROI.top + HorProfileROI.bottom ) ;
  cmplx cHorStepX( 1.0 , 0. ) ;
  CFigureFrame * pHorProfile = CreateFigureFrameEx(
    HorProfile.m_pProfData + HorProfileROI.left ,
    HorProfileROI.right , cOrigin , cHorStepX ,
    HorProfile.m_dMinValue , dScaleY , 0x000000ff ) ;
  return pHorProfile ;
}

CFigureFrame * CreateVGraphForDraw( Profile& VertProfile , CRect VertProfileROI )
{
  double dAmplY = VertProfile.m_dMaxValue - VertProfile.m_dMinValue ;
  double dScaleX = ( dAmplY <= 1. ) ? 0. : -VertProfileROI.right / dAmplY ;
  cmplx cOriginY( VertProfileROI.left + VertProfileROI.right ,
    VertProfileROI.top ) ;
  cmplx cEndY( cOriginY.real() , cOriginY.imag() + VertProfileROI.bottom ) ;
  cmplx cVertStepX( 0. , 1. ) ;
  CFigureFrame * pVertProfile = CreateFigureFrameEx(
    VertProfile.m_pProfData + VertProfileROI.top ,
    VertProfileROI.bottom , cOriginY , cVertStepX ,
    VertProfile.m_dMinValue , dScaleX , 0x000000ff ) ;
  return pVertProfile ;
}

