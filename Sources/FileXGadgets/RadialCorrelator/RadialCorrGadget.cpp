// RadialCorrGadget.h : Implementation of the RadialCorr class


#include "StdAfx.h"
#include "RadialCorrGadget.h"

IMPLEMENT_RUNTIME_GADGET_EX(RadialCorr, CFilterGadget, "Radial", TVDB400_PLUGIN_NAME);

RadialCorr::RadialCorr(void)
{
  m_pInput = new CInputConnector(transparent);
  m_pInput->SetName( "Video&RectIn" ) ;
  m_pControl = new CDuplexConnector( this , transparent , transparent ) ;
  m_pOutput = new COutputConnector(transparent);
  m_pOutput->SetName( "Video&Results" ) ;
  m_pCutControl = new COutputConnector( text * quantity ) ;
  m_pCutControl->SetName( "PositionControl" ) ;
  m_pROIPositionPin = new COutputConnector( figure ) ;
  m_pROIPositionPin->SetName( "ROIPosView" ) ;

  m_MultyCoreAllowed = FALSE ; 
  m_iMinRadius = 15 ;
  m_iMaxRadius = 60 ;
  m_dGaussSigma_pix = 2 ;
  m_iGaussHalfWidth_pix = 8 ;
  m_dInitialAngle_rad = 0. ;
  m_dDiscoverThres = 20. ;
  m_iSecondaryMeasZone = 12 ;
  m_iMeasStep_deg = 5 ;
  m_iOrigImageSizeX = 1920 ;
  m_iOrigImageSizeY = 1080 ;
  m_dZeroDistThres = DegToRad(m_iMeasStep_deg) * 1.3  ;
  m_dLastDist = 0. ;
  m_dLastOverDist = 0. ;
  m_dScale_nm = 34.81 ;
  m_dMinSize_um = 4.0 ;
  m_dMaxSize_um = 5.5 ;
  m_dSignThres = 100. ;
  m_iViewMode = 1 ;
  m_PrevZone = CRect( 0 , 0 , 0 , 0 ) ;
  m_iCellNumber = 0 ;
  m_iMeasurementNumber = 0 ;
  m_PrevCenter = cmplx(0.,0.) ;
  m_iNConturSlices = 15 ;
  m_PatternConturAsString = _T(
    "2,23,0.0,5.03,3.18,56.1,0,0.375,15,"
    "0.81,0.94,1.12,1.14,1.30,1.30,1.43,1.37,1.55,1.45,1.57,1.56,"
    "1.61,1.57,1.59,1.55,1.60,1.54,1.52,1.53,1.43,1.43,1.26,1.31,"
    "1.06,1.16,0.84,0.88,0.56,0.55") ;
  RestoreContur( m_PatternConturAsString , m_PatternContur ) ;
  // Coordinate conversion test
//   CoordsCorresp Pt1 = { cmplx(0.0,0.0) , cmplx(0.0,0.0) } ;
//   CoordsCorresp Pt2 = { cmplx(1.0,0.0) , cmplx(1.0,0.0) } ;
//   CoordsCorresp Pt3 = { cmplx(0.0,1.0) , cmplx(0.0,1.0) } ;
//   CoordsCorresp Meas = { cmplx(0.5,0.5) , cmplx(0.0,0.0) } ;
// 
//   bool bRes = ConvertBy3Pts( Pt1 , Pt2 , Pt3 , Meas ) ;
// 
//   Pt1.FOV._Val[_IM] = 5. ;
// 
// 

  Resume();
}

void RadialCorr::ShutDown()
{
  CFilterGadget::ShutDown();
  delete m_pControl ;
  m_pControl = NULL ;
  delete m_pInput;
  m_pInput = NULL;
  delete m_pOutput;
  m_pOutput = NULL;
  delete m_pCutControl ;
  m_pCutControl = NULL ;
  delete m_pROIPositionPin ;
  m_pROIPositionPin = NULL ;
}

CDataFrame* RadialCorr::DoProcessing(const CDataFrame* pDataFrame)
{
  const CRectFrame * pCutZone = pDataFrame->GetRectFrame() ;
  bool bTheSameArea = false ;
  if ( pCutZone )
  {
    CRect OrigCutRect( *pCutZone ) ;
    CFigureFrame * pROIView = CFigureFrame::Create() ;
    pROIView->Add( CDPoint( pCutZone->left , pCutZone->top ) ) ;
    pROIView->Add( CDPoint( pCutZone->right , pCutZone->top ) ) ;
    pROIView->Add( CDPoint( pCutZone->right , pCutZone->bottom ) ) ;
    pROIView->Add( CDPoint( pCutZone->left , pCutZone->bottom ) ) ;
    pROIView->ChangeId( m_dwFrameId ) ;
    pROIView->SetTime( m_dFrameTime ) ;
    *(pROIView->Attributes())="color(ffff00)" ;
    pROIView->SetLabel("ROIView") ;
    if ( !m_pROIPositionPin->Put( pROIView ) )
      pROIView->Release( pROIView ) ;
    if ( m_PrevZone.right && m_PrevZone.bottom )
    {
      CPoint CentDiff = m_PrevZone.CenterPoint() - OrigCutRect.CenterPoint() ;
      if ( abs(CentDiff.x) > 10  || abs(CentDiff.y) > 10 )
      {
        m_iCellNumber++ ;
        m_iMeasurementNumber = 0 ;
      }
    }
    m_PrevZone = OrigCutRect ;
  }
  FXString ViewData ;
  const CVideoFrame * vf = pDataFrame->GetVideoFrame( /*NULL*/ ) ;
  if ( vf  &&  vf->lpBMIH && vf->lpBMIH->biSizeImage )
  {   
    int iWidth = Width( vf ) ;
    int iHeight = Height( vf ) ;
    m_iRealMaxRadius = m_iMaxRadius ;
    if ( m_iRealMaxRadius >= iWidth/2 )
      m_iRealMaxRadius = (iWidth/2) - 1 ;
    if ( m_iRealMaxRadius >= iHeight/2 )
      m_iRealMaxRadius = (iHeight/2) - 1 ;
    if ( m_iMinRadius > m_iRealMaxRadius )
      return NULL ;
    if ( iWidth && iHeight && GetData( vf ) )
    {
      int iBitDepth = 0 ;
      switch ( GetCompression( vf ) )
      {
      case BI_Y8:
      case BI_Y800:
      case BI_YUV9:
      case BI_YUV12: iBitDepth = 8 ; break ;
      case BI_Y16:   iBitDepth = 16 ; break ;
      default: return NULL ;
      }
      m_dwFrameId = pDataFrame->GetId() ;
      m_dFrameTime = m_dFrameTime ;
      m_pTxtInfo = NULL ;
      m_pSlopeContour = NULL ;
      m_pWhiteContour = NULL ;
      m_pBlackContour = NULL ;
      m_pResult = CContainerFrame::Create() ;
      m_pResult->ChangeId(m_dwFrameId);
      m_pResult->SetTime(m_dFrameTime);
      m_pResult->SetLabel( "RadialCorrResults" ) ;
      for ( int i = 1 ; i < 360 ; i++ )
        m_CorrData[i].m_Pt1.m_iZeroPt = 0 ;
      m_Cent = cmplx( (double)iWidth * 0.5 , (double)iHeight * 0.5 ) ;
      m_iNMeasuredPts = 0 ;

      m_iLastMinPos =  m_iLastMaxPos = m_iMinPos = m_iMaxPos = -1 ;
      m_iStartCorrelPos = m_iStopCorrelPos = 0 ;
      m_iLastCorrFuncLen = 0 ;
      m_dMaxAmpl = 0.0 ;
      m_dMaxSignSigma2 = 0. ;
      double dAngleStep = DegToRad( ((double)m_iMeasStep_deg) ) ; // 
      double dAngleNorm = fmod( m_dInitialAngle_rad + M_2PI, M_2PI ) ;
      double dAngledeg = RadToDeg( dAngleNorm ) ;
      int iFirstIndex = GetAngleIndex( 
        fmod( m_dInitialAngle_rad + M_2PI, M_2PI ) , dAngleStep ) ;


      double dLastMeasuredAngle = FindAndMeasureSector( 
        dAngleNorm , dAngleNorm + M_2PI , dAngleStep , vf ) ;
      if ( dAngleNorm + M_2PI - dLastMeasuredAngle > dAngleStep * 5 )
      {
        m_iLastMinPos = m_CorrData[iFirstIndex].m_Pt1.m_iNPos ;
        m_iLastMaxPos = m_CorrData[iFirstIndex].m_Pt1.m_iPPos ;
        dAngleNorm -= dAngleStep ;
        dAngleNorm = fmod( dAngleNorm + M_2PI , M_2PI ) ;
        double dLastAngleForSecondScan = dLastMeasuredAngle + dAngleStep ;
        dLastAngleForSecondScan = fmod( dLastAngleForSecondScan + M_2PI , M_2PI ) ;
        double dSecondLast = ScanSector( m_Cent , dAngleNorm , dLastAngleForSecondScan , 
          -dAngleStep , vf ) ;
      }
      if ( m_iViewMode & VIEW_CONTUR_AND_RESULTS )
      {
        if ( m_pSlopeContour == NULL )
        {
          m_pSlopeContour = CreatePtFrame( 
            m_CorrData[iFirstIndex].m_Pt1.m_ZeroPt , vf->GetTime() , 
            "0x0000ff" , "ZeroCont" ) ;
        }
        cmplx First( 0., 0.) ;
        cmplx Last( 0. , 0.) ;
        if ( m_pSlopeContour )
        {
          for ( int i = 0 ; i < 360 ; i++ )
          {
            if ( m_CorrData[i].m_Pt1.m_iZeroPt )
            {
              Last = m_CorrData[i].m_Pt1.m_ZeroPt ;
              if ( First.real() == 0. )
                First = Last ;
              m_pSlopeContour->Add( CDPoint( Last.real() , Last.imag() ) ) ;
            }
          }
          m_pSlopeContour->Add( CDPoint( First.real() , First.imag() ) ) ;
        }
      }
      if ( m_dMaxAmpl > 0.05  && (m_iViewMode & VIEW_INIT_VECT_MIN_MAX) )
      {
        CFigureFrame * pAmplGraph = CFigureFrame::Create() ;
        pAmplGraph->Attributes()->WriteString( "color" , "0x8080ff" ) ;
        int iLowEdge = iHeight - 2 ;
        int iLeftEdge = 10 ;
        double dGraphAmpl = 100. ;
        double dScale = dGraphAmpl / m_dMaxAmpl ;
        for ( int i = 0 ; i < 360 ; i++ )
        {
          if ( m_CorrData[i].m_Pt1.m_iZeroPt )
          {
            double iAmpl = m_CorrData[i].m_Pt1.m_dMaxPCorr 
              - m_CorrData[i].m_Pt1.m_dMaxNCorr ;
            if ( iAmpl > 0. )
            {
              if ( i % 90 == 0 )
                pAmplGraph->Add( CDPoint( iLeftEdge + i/2 , iLowEdge - 110. ) ) ;
              else
                pAmplGraph->Add( CDPoint( iLeftEdge + i/2 , iLowEdge - iAmpl * dScale ) ) ;
            }
          }
        }
        pAmplGraph->SetLabel( "CorrFuncGraph" ) ;
        pAmplGraph->SetTime( vf->GetTime() ) ;
        pAmplGraph->ChangeId( 0 ) ;
        m_pResult->AddFrame( pAmplGraph ) ;
      }

      if ( m_iNMeasuredPts > (360. * 0.8) / m_iMeasStep_deg)
      {
        //           double dFirstToLastDist = 
        //             abs( m_CorrData[iFirstMeasuredIndex].m_Pt1.m_ZeroPt 
        //             - m_CorrData[iLastMeasuredIndex].m_Pt1.m_ZeroPt ) ;
        //           double dThresInPixels = m_dZeroDistThres 
        //             * abs( m_CorrData[iFirstMeasuredIndex].m_Pt1.m_ZeroPt - m_Cent ) ;
        if ( 1 /*dFirstToLastDist < dThresInPixels*/ )
        {
          ImageMoments Moments ;
          if ( CalculateMomentsByPts( m_CorrData , 
            sizeof(m_CorrData)/sizeof(m_CorrData[0]) , Moments , m_Cent ) > 0 )
          {
            double dMass = Moments.m_dM00 ;
            cmplx MassCent = Moments.m_M1 / dMass ;

            double dMu11 = Moments.m_dM11 - MassCent.real() * Moments.m_M1.real() ;
            double dMu11byY = Moments.m_dM11 - MassCent.imag() * Moments.m_M1.imag() ;
            double dDmu = dMu11 - dMu11byY ;
            double dMu20 = Moments.m_dM20 - MassCent.real() * Moments.m_M1.imag() ;
            double dMu02 = Moments.m_dM02 - MassCent.imag() * Moments.m_M1.real() ;

            double dMuCov20 = (Moments.m_dM20/Moments.m_dM00) 
              -( MassCent.real() * MassCent.real()) ;
            double dMuCov02 = (Moments.m_dM02/Moments.m_dM00) 
              -( MassCent.imag() * MassCent.imag()) ;
            double dMuCov11 = (Moments.m_dM11/Moments.m_dM00) 
              -( MassCent.imag() * MassCent.real() ) ;
            // minus in next line is because Y axis is directed down
            double dCellAngle = - 0.5 * atan2( 2 * dMuCov11 , dMuCov20 - dMuCov02 ) ;
            double dCellAngle_deg = RadToDeg( dCellAngle ) ;
            cmplx ContourCent = /*Cent + */MassCent ;
            cmplx CellDir = polar( 50. , -dCellAngle ) ; // minus is also because
            // coordinate system
            cmplx DirEnd = ContourCent + CellDir ;

            double dPositiveAngle = NormTo2PI( dCellAngle_deg ) ;

            int iFirst = GetIndexForCenter( dCellAngle , dAngleStep , ContourCent );
            int iSecond = GetIndexForCenter( 
              dCellAngle + M_PI2 , dAngleStep , ContourCent ) ;
            int iThird = GetIndexForCenter( 
              dCellAngle + M_PI , dAngleStep , ContourCent ) ;
            int iFourth = GetIndexForCenter( 
              dCellAngle + M_PI + M_PI2 , dAngleStep , ContourCent ) ;

            if ( (0 <= iFirst) && (iFirst < 360) 
              && (0 <= iSecond) && (iSecond < 360)
              && (0 <= iThird) && (iThird < 360)
              && (0 <= iFourth) && (iFourth < 360) 
              && (abs(GetDeltaForIndexes( iFirst ,  iSecond ) - 90 ) < 20) 
              && (abs(GetDeltaForIndexes( iSecond , iThird ) - 90 ) < 20) 
              && (abs(GetDeltaForIndexes( iThird , iFourth ) - 90 ) < 20) 
              && (abs(GetDeltaForIndexes( iFourth , iFirst ) - 90 ) < 20) 
              && (abs(m_CorrData[iFirst].m_Pt1.m_ZeroPt) > 1.)
              && (abs(m_CorrData[iSecond].m_Pt1.m_ZeroPt) > 1.)
              && (abs(m_CorrData[iThird].m_Pt1.m_ZeroPt) > 1.)
              && (abs(m_CorrData[iFourth].m_Pt1.m_ZeroPt) > 1.)
              )
            {
              cmplx CellLen = m_CorrData[iFirst].m_Pt1.m_ZeroPt 
                - m_CorrData[iThird].m_Pt1.m_ZeroPt ;
              double dLen = abs(CellLen) ;
              cmplx CellWidth = m_CorrData[iSecond].m_Pt1.m_ZeroPt 
                - m_CorrData[iFourth].m_Pt1.m_ZeroPt ;
              double dWidth = abs(CellWidth) ;
              dLen *= m_dScale_nm * 0.001 ; // convert to microns
              dWidth *= m_dScale_nm * 0.001 ;
              ViewData.Format("Length=%7.3f Width=%7.3f " , dLen , dWidth ) ;


              bool bTailDetectPossible = true ;

              m_TailDetectAmpls[0] = BuildCorrFunction( 
                ContourCent , m_CorrData[iFirst].m_Pt1.m_ZeroPt , 
                vf , m_TailDetectSigmas2[0] , m_TailPoints[0] ,
                m_Ortho1[0] , m_Ortho2[0] ) ;
              m_TailDetectAmpls[1] = BuildCorrFunction( 
                ContourCent , m_CorrData[iSecond].m_Pt1.m_ZeroPt , 
                vf , m_TailDetectSigmas2[1] , m_TailPoints[1] ,
                m_Ortho1[1] , m_Ortho2[1] ) ;
              m_TailDetectAmpls[2] = BuildCorrFunction( 
                ContourCent , m_CorrData[iThird].m_Pt1.m_ZeroPt , 
                vf , m_TailDetectSigmas2[2] , m_TailPoints[2] ,
                m_Ortho1[2] , m_Ortho2[2] ) ;
              m_TailDetectAmpls[3] = BuildCorrFunction( 
                ContourCent , m_CorrData[iFourth].m_Pt1.m_ZeroPt , 
                vf , m_TailDetectSigmas2[3] , m_TailPoints[3] ,
                m_Ortho1[3] , m_Ortho2[3] ) ;

              for ( int i = 0 ; i < 4 ; i++ )
              {
                if ( m_TailDetectAmpls[i] == 0. )
                {
                  bTailDetectPossible = false ;
                  break ;
                }
              }
              //                 }
              if ( bTailDetectPossible )
              {
                if ( m_iViewMode & VIEW_TAIL_SEARCH_POINTS )
                {
                  for ( int i = 0 ; i < 4 ; i++ )
                  {
                    CFigureFrame * pMayBeTail =  CreateLineFrame( m_Ortho1[i] , 
                      m_Ortho2[i] , "0xffb0b0" , "TailPt" , m_dwFrameId, vf->GetTime()) ;
                    m_pResult->AddFrame( pMayBeTail ) ;
                    cmplx pt = m_TailPoints[i] + cmplx(2,0) ;
                    FXString Text ;
                    Text.Format( "A=%5.1f Sigm=%4.0f" , 100. * m_TailDetectAmpls[i],
                      m_TailDetectSigmas2[i] ) ;
                    CTextFrame * pText = CreateTextFrame( pt , Text , "0xffc0c0" , 10 , 
                      "TailPtSearch" , m_dwFrameId) ;
                    m_pResult->AddFrame( pText ) ;
                  }
                }

                bool bTailOnFirst = m_TailDetectSigmas2[0] > m_TailDetectSigmas2[2] ;

                if ( bTailOnFirst )
                {
                  int iTmp = iFirst ;
                  iFirst = iThird ;
                  iThird = iTmp ;
                  iTmp = iSecond ;
                  iSecond = iFourth ;
                  iFourth = iTmp ;
                }
                cmplx Pt1 = m_CorrData[iFirst].m_Pt1.m_ZeroPt ;
                cmplx Pt2 = m_CorrData[iThird].m_Pt1.m_ZeroPt ;
                cmplx Pt3 = m_CorrData[iSecond].m_Pt1.m_ZeroPt ;
                cmplx Pt4 = m_CorrData[iFourth].m_Pt1.m_ZeroPt ;
                cmplx FirstToThirdVect = Pt1 - Pt2 ;
                if ( m_iViewMode & VIEW_LONG_SHORT_DIRS )
                {
                  CFigureFrame * pLong = NULL ;
                  Pt2 -= FirstToThirdVect / 3. ;
                  pLong = CreateLineFrame( Pt1 , Pt2 ,
                    "0x8080ff" , "LongDir" , m_dwFrameId , vf->GetTime() ) ;
                  m_pResult->AddFrame( pLong ) ;
                  CFigureFrame * pShort = CreateLineFrame(
                    Pt3 , Pt4 , "0xff80ff" , "ShortDir" , m_dwFrameId, vf->GetTime()) ;
                  m_pResult->AddFrame( pShort ) ;
                }

                // Find point1 - farest from tail and from short dir
                cmplx Point1 = GetFarestPoint( Pt3 , Pt4 , iFirst - 15 , iFirst + 15 ) ;
                // Find point5 - nearest to tail and farest from short dir
                cmplx Point5 = GetFarestPoint( Pt3 , Pt4 , iThird - 15 , iThird + 15 ) ;
                // Find point3 - farest from long dir on one side
                cmplx Point3 = GetFarestPoint( Pt1 , Pt2 , iSecond - 15 , iSecond + 15 ) ;
                // Find point7 - farest from long dir on second side
                cmplx Point7 = GetFarestPoint( Pt1 , Pt2 , iFourth - 15 , iFourth + 15 ) ;

                if ( m_iViewMode & VIEW_SLICES )
                {
                  if ( abs(Point1) > 1.  &&  abs(Point3) > 1.
                    && abs(Point5) > 1.  &&  abs(Point7) > 1. )
                  {
                    CFigureFrame * pMainPoints = CreateLineFrame(
                      Point1 , Point3 , "0xffffff" , "MainPoints" , m_dwFrameId, vf->GetTime() ) ;
                    pMainPoints->Add( CDPoint( Point5.real() , Point5.imag() ) ) ;
                    pMainPoints->Add( CDPoint( Point7.real() , Point7.imag() ) ) ;
                    pMainPoints->Add( CDPoint( Point1.real() , Point1.imag() ) ) ;

                    m_pResult->AddFrame( pMainPoints ) ;
                  }
                }
                cmplx MainAxisDir = polar( 100. , -dCellAngle ) ;  // minus, because coord system
                cmplx Extr1 = ContourCent + MainAxisDir ;
                cmplx Extr2 = ContourCent - MainAxisDir ;
                CLine2d MainAxis( Extr1 , Extr2 ) ;
                m_pResult->AddFrame( CreateLineFrame( Extr1 , Extr2 , 
                  "0x0000ff" , "MainDir" , m_dwFrameId ) ) ;
                int iNSlices = GetSlices( ContourCent , dCellAngle , 
                  iFirst ,  m_iNConturSlices ) ;
                if ( iNSlices == m_iNConturSlices )
                {
                  for ( int i = 0 ; i < m_LastContur.iNConturSlices ; i++ )
                  {
                    if ( m_LastContur.Presentation[i] )
                      m_pResult->AddFrame( (CDataFrame*) m_LastContur.Presentation[i] ) ;
                  }
                  ViewData.Format("Length=%7.3f Ws=%7.3f " , dLen , m_LastContur.dWidth_um ) ;
                  m_LastContur.dTime = GetHRTickCount()/1000. ; // in ms
                  CTextFrame * pSimptomData = CreateTextFrame( "Symptoms" , m_dwFrameId ) ;
                  pSimptomData->GetString().Format( 
                    "%6d,%6d,%10.1f,%6.2f,%6.2f,%6.2f,%8.1f,%d" ,
                    m_iCellNumber , m_iMeasurementNumber++ , m_LastContur.dTime ,
                    m_LastContur.dLength_um , m_LastContur.dWidth_um , m_LastContur.dAngle_deg ,
                    0. /* Square */ , m_LastContur.iNConturSlices ) ;
                  for ( int i = 0 ; i < m_LastContur.iNConturSlices ; i++ )
                  {
                    double dLeftDist = m_LastContur.Slices[i].dLeftSide_um ;
                    double dRightDist = m_LastContur.Slices[i].dRightSide_um ;
                    FXString Addition ; 
                    Addition.Format(",%6.2f,%6.2f" , dLeftDist , dRightDist ) ;
                    pSimptomData->GetString() += Addition ;
                  }
                  m_pResult->AddFrame( pSimptomData ) ;

                  if ( m_iViewMode & VIEW_PATTERN_CONTUR )
                  {
                    CFigureFrame * pPatternContur = GetPatternOnFoundContur(
                      m_LastContur.Center , -dCellAngle , _T("0xff4040")) ;
                    if ( pPatternContur )
                      m_pResult->AddFrame( pPatternContur ) ;
                  }
                }
              }
            }

            CFigureFrame * pCenter =  CreatePtFrame( ContourCent , 
              vf->GetTime() , "0x0000ff" , "Center" , m_dwFrameId ) ;
            m_pResult->AddFrame( pCenter ) ;

            cmplx Shift = MassCent - m_Cent ;
            CSize intShift(ROUND(Shift.real()) , ROUND(Shift.imag())) ;
            if ( m_pTxtInfo )
            {
              FXString ProcessingTime ;
              ProcessingTime.Format( " Tp=%5.1f\nShift(%d,%d) N=%d" ,
                ((GetGraphTime() * 1.e-3)- vf->GetTime()) , 
                intShift.cx , intShift.cy , m_iNMeasuredPts ) ;
              ViewData += ProcessingTime ;
              m_pTxtInfo->Attributes()->WriteString("color" , "c0ffc0" ) ;
              m_pTxtInfo->Attributes()->WriteString("message" , ViewData) ;
            }

            if ( m_pCutControl && m_pCutControl->IsConnected() )
            {
              CTextFrame * pRectControl = CTextFrame::Create() ;
              CRect CurrentRect( *pCutZone ) ;

              // the next commented is for cut rectangle moving step 4
              //                 intShift.cx = (intShift.cx >>2) << 2;
              //                 intShift.cy = (intShift.cy >>2) << 2;
              // 
              //                 CurrentRect.OffsetRect( intShift ) ;
              //                 int iDistToLeft = CurrentRect.left ;
              //                 if ( iDistToLeft < 0 )
              //                   CurrentRect.OffsetRect( ((-iDistToLeft >> 2) << 2) , 0 ) ;
              //                 int iDistToTop = CurrentRect.top ;
              //                 if ( iDistToTop < 0 )
              //                   CurrentRect.OffsetRect( ((-iDistToTop >> 2) << 2) , 0 ) ;
              //                 int iDistToRight = m_iOrigImageSizeX - 4 - CurrentRect.right ;
              //                 if ( iDistToRight < 0 )
              //                   CurrentRect.OffsetRect( 0 , ((-iDistToRight >> 2) << 2) ) ;
              //                 int iDistToBottom = m_iOrigImageSizeY - 4 - CurrentRect.bottom ;
              //                 if ( iDistToBottom < 0 )
              //                   CurrentRect.OffsetRect( 0 , ((-iDistToBottom >> 2) << 2) ) ;

              // The next is cut rectangle position correction in agree with cell center position
              CurrentRect.OffsetRect( intShift ) ;
              int iWidthError = CurrentRect.Width() & 0x3 ;
              switch ( iWidthError )
              {
              case 0 : break;
              case 1 : CurrentRect.right-- ; break ;
              case 2 : CurrentRect.right++ ;
                CurrentRect.left-- ;
                break ;
              case 3: CurrentRect.right++ ; break ;
              }
              int iDistToLeft = CurrentRect.left ;
              if ( iDistToLeft < 0 )
                CurrentRect.OffsetRect( -iDistToLeft , 0 ) ;
              int iDistToTop = CurrentRect.top ;
              if ( iDistToTop < 0 )
                CurrentRect.OffsetRect( 0 , -iDistToTop ) ;
              int iDistToRight = m_iOrigImageSizeX - 2 - CurrentRect.right ;
              if ( iDistToRight < 0 )
                CurrentRect.OffsetRect( iDistToRight , 0 ) ;
              int iDistToBottom = m_iOrigImageSizeY - 2 - CurrentRect.bottom ;
              if ( iDistToBottom < 0 )
                CurrentRect.OffsetRect( 0 , iDistToBottom ) ;


              pRectControl->GetString().Format( "Rect=%d,%d,%d,%d;" ,
                CurrentRect.left , CurrentRect.top , 
                CurrentRect.right , CurrentRect.bottom ) ;
              pRectControl->ChangeId( 0 ) ;
              pRectControl->SetTime( vf->GetTime() ) ;
              if ( ! m_pCutControl->Put( pRectControl ) )
                pRectControl->Release( pRectControl ) ;
            }
          }
          else if ( m_pTxtInfo )
          {
            FXString AddInfo ;
            AddInfo.Format( " Tp=%7.3f N=%d\nCan't calculate moments" , 
              ((GetGraphTime() * 1.e-3)- vf->GetTime()) ,
              m_iNMeasuredPts ) ;
            ViewData += AddInfo ;
            m_pTxtInfo->Attributes()->WriteString("message" , ViewData) ;
            m_pTxtInfo->Attributes()->WriteString("color" , "0x8080ff" ) ;
          }
        }
        else if ( m_pTxtInfo )
        {
          //           FXString AddInfo ;
          //           AddInfo.Format( " Tp=%7.3f n=%d\nToo big breaking %d(%d)" , 
          //             (GetGraphTime() * 1.e-3)- vf->GetTime() , 
          //             m_iNMeasuredPts , (int)dFirstToLastDist , (int)dThresInPixels ) ;
          //           ViewData += AddInfo ;
          //           m_pTxtInfo->Attributes()->WriteString("message" , ViewData) ;
          //           m_pTxtInfo->Attributes()->WriteString("color" , "0x8080ff" ) ;
        }
      }
      else if ( m_pTxtInfo )
      {
        FXString AddInfo ;
        AddInfo.Format( " Tp=%7.3f \nToo few points %d(%d)" , (GetGraphTime() * 1.e-3)- vf->GetTime() , 
          m_iNMeasuredPts , (int)((360. * 0.8) / m_iMeasStep_deg) ) ;
        ViewData += AddInfo ;
        m_pTxtInfo->Attributes()->WriteString("message" , ViewData) ;
        m_pTxtInfo->Attributes()->WriteString("color" , "0x0000ff" ) ;
      }
      if ( m_pWhiteContour )
        m_pResult->AddFrame( m_pWhiteContour ) ;
      if ( m_pBlackContour )
        m_pResult->AddFrame( m_pBlackContour ) ;
      if ( m_pSlopeContour)
        m_pResult->AddFrame( m_pSlopeContour ) ;
      if ( m_pTxtInfo )
        m_pResult->AddFrame( m_pTxtInfo ) ;

//       if ( !pDataFrame->IsContainer() )
        m_pResult->AddFrame( vf ) ;

      return m_pResult ;
    }
  }
  return NULL ;
}

bool RadialCorr::ScanProperties(LPCTSTR text, bool& Invalidate)
{
  FXPropertyKit pk(text);
  pk.GetDouble("Sigma_Pix" , m_dGaussSigma_pix ) ;
  pk.GetInt( "HalfWidth" , m_iGaussHalfWidth_pix ) ;
  pk.GetInt( "MinRadius_pix" , m_iMinRadius ) ;
  pk.GetInt( "MaxRadius_pix" , m_iMaxRadius ) ;
  pk.GetInt( "SZone" , m_iSecondaryMeasZone ) ;
  double m_dInitialAngle_deg ;
  pk.GetDouble("Angle_Deg" , m_dInitialAngle_deg ) ;
  pk.GetInt( "AngleStep" , m_iMeasStep_deg ) ;
  pk.GetDouble("2ndThres" , m_dDiscoverThres ) ;
  pk.GetDouble("Scale_nm" , m_dScale_nm ) ;
  pk.GetInt( "#Slices" , m_iNConturSlices ) ;
  pk.GetDouble("MinSize_um" , m_dMinSize_um ) ;
  pk.GetDouble("MaxSize_um" , m_dMaxSize_um ) ;
  pk.GetDouble("SigSigma2" , m_dSignThres ) ;
  pk.GetInt( "ViewMode" , m_iViewMode ) ;
  pk.GetInt( "OrigSzX" , m_iOrigImageSizeX ) ;
  pk.GetInt( "OrigSzY" , m_iOrigImageSizeY ) ;
  if (   pk.GetString( "PatternContur" , m_PatternConturAsString ) 
    &&   !m_PatternConturAsString.IsEmpty() )
  {
    FXString Patt( m_PatternConturAsString ) ;
    Patt.Remove( _T('\t')) ;
    Patt.Remove( _T(' ')) ;
    RestoreContur( (LPCTSTR)Patt , m_PatternContur ) ;
  }
  m_dInitialAngle_deg = m_iMeasStep_deg * ROUND( m_dInitialAngle_deg / m_iMeasStep_deg ) ;
  m_dInitialAngle_rad = DegToRad( m_dInitialAngle_deg ) ;
  m_dInitialAngle_rad = fmod( m_dInitialAngle_rad , M_2PI ) ;
//   m_iPatternLen = CreateGaussPattern( 
//     m_dGaussSigma_pix , m_iGaussHalfWidth_pix , m_Pattern ) ;
  m_iPatternLen = CreateCosinusPattern( m_iGaussHalfWidth_pix , m_Pattern ) ;
  return true;
}

bool RadialCorr::PrintProperties(FXString& text)
{
  FXPropertyKit pk;
  CFilterGadget::PrintProperties(text);
  pk.WriteDouble("Sigma_Pix" , m_dGaussSigma_pix ) ;
  pk.WriteInt( "HalfWidth" , m_iGaussHalfWidth_pix ) ;
  pk.WriteInt( "MinRadius_pix" , m_iMinRadius ) ;
  pk.WriteInt( "MaxRadius_pix" , m_iMaxRadius ) ;
  pk.WriteInt( "SZone" , m_iSecondaryMeasZone ) ;
  pk.WriteDouble("Angle_Deg" , RadToDeg( m_dInitialAngle_rad ) ) ;
  pk.WriteInt( "AngleStep" , m_iMeasStep_deg ) ;
  pk.WriteDouble("2ndThres" , m_dDiscoverThres ) ;
  pk.WriteDouble("Scale_nm" , m_dScale_nm ) ;
  pk.WriteInt( "#Slices" , m_iNConturSlices ) ;
  pk.WriteDouble("MinSize_um" , m_dMinSize_um ) ;
  pk.WriteDouble("MaxSize_um" , m_dMaxSize_um ) ;
  pk.WriteDouble("SigSigma2" , m_dSignThres ) ;
  pk.WriteInt( "ViewMode" , m_iViewMode ) ;
  pk.WriteInt( "OrigSzX" , m_iOrigImageSizeX ) ;
  pk.WriteInt( "OrigSzY" , m_iOrigImageSizeY ) ;
  if ( BuildConturRecord( m_PatternContur , m_PatternConturAsString ) )
    pk.WriteString( "PatternContur" , m_PatternConturAsString ) ;
  text+=pk;
  return true;
}

bool RadialCorr::ScanSettings(FXString& text)
{
  text = "template(EditBox(Sigma_Pix),"
    "Spin(HalfWidth,3,40),"
    "Spin(MinRadius_pix,10,50),"
    "Spin(MaxRadius_pix,15,250),"
    "Spin(SZone,5,50),"
    "EditBox(Angle_Deg),"
    "Spin(AngleStep,1,10),"
    "EditBox(2ndThres),"
    "EditBox(Scale_nm),"
    "Spin(#Slices,7,50),"
    "EditBox(MinSize_um),"
    "EditBox(MaxSize_um)," 
    "EditBox(SigSigma2),"
    "Spin(ViewMode,0,127),"
    "Spin(OrigSzX,100,5000),"
    "Spin(OrigSzY,100,5000),"
    "EditBox(PatternContur)"
    ")";
  return true;
}

// int RadialCorr::GetOutputsCount() 
// {
//   return 2; 
// } ;
COutputConnector* RadialCorr::GetOutputConnector(int n) 
{
  switch ( n )
  {
  case 0: return m_pOutput ;
  case 1: return m_pCutControl ;
  case 2: return m_pROIPositionPin ;
  default: return NULL ;
  }
};
CDuplexConnector* RadialCorr::GetDuplexConnector(int n)
{
  return (n) ? NULL : m_pControl ;
}

void RadialCorr::AsyncTransaction(
  CDuplexConnector* pConnector, CDataFrame* pParamFrame)
{
  if (!pParamFrame)
    return;
  CTextFrame* tf=pParamFrame->GetTextFrame(DEFAULT_LABEL);
  if (tf)
  {
    FXParser pk=tf->GetString(); 
    FXString cmd;
    FXString param;
    FXSIZE pos=0;
    pk.GetWord(pos,cmd);

//     if (cmd.CompareNoCase("list")==0)
//     {
//       pk.Empty();
//       for (int i = 0 ; i<m_Properties.GetSize(); i++)
//       {
//         //FXParser
//         pk+=m_Properties[i].property; pk+="\r\n";
//       }
//     }
//     else if ((cmd.CompareNoCase("get")==0) && (pk.GetWord(pos,cmd)))
//     {
//       unsigned id=GetPropertyId(cmd);
//       int value;
//       bool bauto;
//       if ((id!=WRONG_PROPERTY) && GetCameraProperty(id,value,bauto))
//       {
//         if (IsDigitField(id))
//         {
//           if (bauto)
//             pk="auto";
//           else
//             pk.Format("%d",value);
//         }
//         else
//           pk=(LPCTSTR)value;
//       }
//       else
//         pk="error";
//     }
//     else if ((cmd.CompareNoCase("set")==0) 
//           && (pk.GetWord(pos,cmd)) 
//           && (pk.GetParamString(pos, param)))
//     {
//       unsigned id=GetPropertyId(cmd);
//       int value=0;
//       bool bauto=false, Invalidate=false;
//       if (IsDigitField(id))
//       {
//         if (param.CompareNoCase("auto")==0)
//           bauto=true;
//         else
//           value=atoi(param);
//       }
//       else
//         value=(int)(LPCTSTR)param;
//       if ((id!=WRONG_PROPERTY) && SetCameraProperty(id,value,bauto,Invalidate))
//       {
//         pk="OK";
//       }
//       else
//         pk="error";
//     }
//     else
//     {
//       pk="List of avalabale commands:\r\n"
//         "list - return list of properties\r\n"
//         "get <item name> - return current value of item\r\n"
//         "set <item name>(<value>) - change an item\r\n";
//     }
//     CTextFrame* retV=CTextFrame::Create(&pk);
//     retV->ChangeId(NOSYNC_FRAME);
//     if (!m_pControl->Put(retV))
//       retV->RELEASE(retV);

  }
  pParamFrame->Release(pParamFrame);
}

