#include "stdafx.h"
#include "imageproc/statistics.h"
#include "RulTracker.h"
#include "fxfc/FXRegistry.h"
#include <iostream>
#include <fstream>

USER_FILTER_RUNTIME_GADGET( RulTracker , "Video.FileX_Specific" );

// static bool g_bEndOfWork = false ;

// VOID CALLBACK MainLoopTimerRoutine( LPVOID lpParam , BOOLEAN TimerOrWaitFired )
// {
//   if ( !g_bEndOfWork )
//   {
//     RulTracker * pGadget = ( RulTracker* ) lpParam;
// //     if ( pGadget->m_pStatus->GetStatus() == CExecutionStatus::RUN )
// //     {
//     pGadget->ProcessTimer() ;
// //     }
//   }
// }


static const char * msc_pGadgetModeNames = "Unknown;Side;Top;";

const int        RulTracker::msc_iDigitsPeriod_mm = 100;
const double     RulTracker::msc_dDigitsRegionWidth_mm = 12.;
const Segment1d  RulTracker::msc_DigitsRegionInPeriod_mm = Segment1d( RulTracker::msc_iDigitsPeriod_mm - ( RulTracker::msc_dDigitsRegionWidth_mm / 2. ) , RulTracker::msc_iDigitsPeriod_mm + ( RulTracker::msc_dDigitsRegionWidth_mm / 2. ) );
                 
int              RulTracker::ms_iViewMode = 5;
double           RulTracker::ms_dBaseHorDist_mm = 10.;
double           RulTracker::ms_dZeroPos_mm = 0.;
int              RulTracker::ms_iMaskToSetZeroPos = RulTrackerGadgetMode::RTGM_Unknown;
const LinesBatch RulTracker::msc_LinesBatchEmpty;



DWORD  RulTracker::ms_dLastVFIdPerGdgtMode[ 2 ] = { 0 , 0 };
double RulTracker::ms_dLastFrameTimePerGdgtMode_ms[ 2 ]={0.,0.};

// Global angle calculation data
double RulTracker::ms_dAngle_deg = 0.;
double RulTracker::ms_dSideAngle_deg = 0.;
double RulTracker::ms_dTopAngle_deg = 0.;
Segment1d RulTracker::ms_SideEdges_deg , RulTracker::m_OldSideEdges_deg  ;
Segment1d RulTracker::ms_TopEdges_deg , RulTracker::m_OldTopEdges_deg;
double RulTracker::ms_dAngleShift_deg = 0.;
// End of Global angle calculation data 


void RulTracker::ConfigParamChange( LPCTSTR pName , void* pObject ,
  bool& bInvalidate , bool& bInitRescan )
{
  RulTracker * pGadget = ( RulTracker* )pObject;
  if (pGadget)
  {
    //     if ( !_tcsicmp( pName , _T( "Thres-SkewSzVDist" ) ) )
    //     {
    //       FXSIZE iPos = 0 ;
    //       FXString Token = pGadget->m_sThresholdForLog.Tokenize( "," , iPos ) ;
    //       if ( ( !Token.IsEmpty() ) )
    //       {
    //         Token.Trim() ;
    //         if ( !Token.IsEmpty() )
    //           pGadget->m_dSkewThresForLog_um = atof( Token ) ;
    //         Token = pGadget->m_sThresholdForLog.Tokenize( "," , iPos ) ;
    //         if ( ( !Token.IsEmpty() ) )
    //         {
    //           Token.Trim() ;
    //           if ( !Token.IsEmpty() )
    //             pGadget->m_dSizeThresForLog_um = atof( Token ) ;
    //           Token = pGadget->m_sThresholdForLog.Tokenize( "," , iPos ) ;
    //           if ( ( !Token.IsEmpty() ) )
    //           {
    //             Token.Trim() ;
    //             if ( !Token.IsEmpty() )
    //               pGadget->m_dHLineDistThresForLog_um = atof( Token ) ;
    //           }
    //         }
    //       }
    //     }
  }
}


RulTracker::RulTracker( )
{

  m_sName = FXString( );

  m_GadgetMode = RulTrackerGadgetMode::RTGM_Unknown;
  m_OutputMode = modeReplace;
  m_dPixelSize_mm = 1.;
  m_dTolerance_um = 7.;
  m_LastROI_px = CRect( 0 , 0 , 1440 , 1080 );
  init( );
}


RulTracker::~RulTracker( )
{
  if (m_pLastResultViewWithoutPower)
  {
    m_pLastResultViewWithoutPower->Release( );
    m_pLastResultViewWithoutPower = NULL;
  }
}


void RulTracker::PropertiesRegistration( )
{
  addProperty( SProperty::COMBO , _T( "GadgetMode" ) , ( int * )&m_GadgetMode ,
    SProperty::Int , msc_pGadgetModeNames );
  addProperty( SProperty::SPIN , "ViewMode" , &ms_iViewMode , SProperty::Int , 0 , 100 );
  addProperty( SProperty::EDITBOX , "BaseHorDist_mm" , &ms_dBaseHorDist_mm , SProperty::Double );
  addProperty( SProperty::EDITBOX , "Scale_mm/pix" , &m_dPixelSize_mm , SProperty::Double );
  addProperty( SProperty::EDITBOX , "Tolerance_um" , &m_dTolerance_um , SProperty::Double );
  addProperty( SProperty::EDITBOX , "SearchWidth_pix" , &m_dSearchWidth_pix , SProperty::Double );
  addProperty( SProperty::EDITBOX , "MinimalContrast" ,
    &m_dMinimalContrast , SProperty::Double );
  addProperty( SProperty::EDITBOX , "LineThreshold" ,
    &m_dLineSearchThres , SProperty::Double );
  SetChangeNotificationForLast( ConfigParamChange , this );
};

void RulTracker::ConnectorsRegistration( )
{
  addInputConnector( transparent , "ImageWithMeasuredPoints" );
  addOutputConnector( transparent , "OutputView" );
  addOutputConnector( text , "ResultView" );
  addOutputConnector( text , "CamCtrl" );
  GetInputConnector( 0 )->SetQueueSize( 10 );
  GetInputConnector( 0 )->Send( CreateTextFrame( "Init" , "Init" ) );
};

void RulTracker::ShutDown( )
{
  //   if ( m_GadgetMode == SMGM_LeftTop)
  //   {
  //   g_bEndOfWork = true ;
  //   DeleteAsyncTimer() ;
  //   Sleep( 150 ) ;
  //   }
  CFilterGadget::ShutDown( );
  //    CGadget::ShutDown();
}

CDataFrame* RulTracker::DoProcessing( const CDataFrame* pDataFrame )
{
  double dStart = GetHRTickCount( );

  double dTmpZeroPos_mm = 0;
  if (TryExtractZeroPos_mm( pDataFrame , __out dTmpZeroPos_mm ))
  {
    ms_dZeroPos_mm = dTmpZeroPos_mm;
    m_dZeroPosOnImage_pix = 0;

    m_dMasterBaseHorDist_px = 0;
    m_Statistics.Reset( );

    ms_iMaskToSetZeroPos = (RulTrackerGadgetMode::RTGM_Top | RulTrackerGadgetMode::RTGM_Side); // for both cameras
    m_dPixelSize_mm = 0.;
    m_pPrevLines = NULL;
    m_PartOfDigitsRegionInROI_px_Predicted.m_dBegin = m_PartOfDigitsRegionInROI_px_Predicted.m_dEnd = 0;

    return NULL;
  }

  CContainerFrame * pOut = CContainerFrame::Create( );
  pOut->CopyAttributes( pDataFrame );
  pOut->AddFrame( pDataFrame );

  m_pLastVideoFrame = pDataFrame->GetVideoFrame( );
  if (m_pLastVideoFrame == NULL) //skip non-video frames
    return pOut;
  
  FXString sName = pDataFrame->GetLabel( );
  if (m_sName.IsEmpty() && sName.Find( "Cam" ) > 0)
    m_sName = sName;

  m_dTimeBetweenFrames_ms = GetFrameToFrameTime_ms( m_pLastVideoFrame->GetTime( ) );
  int dDeltaFrameIds = GetFrameToFrameIDs( m_pLastVideoFrame->GetId( ) );

  const double c_dTimeThreshold_ms = 100.;
  const double c_dMilliSecsPerSec = 1000.;

  if (m_dTimeBetweenFrames_ms >= c_dTimeThreshold_ms)
    m_dLastV_pps = 0.;
  
  m_LastROI_px.right = GetWidth( m_pLastVideoFrame );
  m_LastROI_px.bottom = GetHeight( m_pLastVideoFrame );

  m_cLastROI_Cntr_px = cmplx( ( double )m_LastROI_px.CenterPoint( ).x , ( double )m_LastROI_px.CenterPoint( ).y );

  cmplx cViewPt( 0. , m_cLastROI_Cntr_px.imag( ) * 0.18 ); //position of text in the viewer;

  if (IsZeroPosCalibrated( ) && m_pPrevLines!=NULL && !m_pPrevLines->IsEmpty( ))
  {
    ASSERT( m_pLastLines->HasAllAbsPos_mm( ) );
    ASSERT( m_pPrevLines->HasAllAbsPos_mm( ) );
  }

  //LineInfo li01( cmplx( 365.1 , 378.5 ) , 18.5 , 0 , 1 , 640.0 , 521.96000000000004 , 10 );
  //LineInfo li02( cmplx( 1137.2 , 378.5 ) , 18.5 , 0 , 1 , 640.0 , 521.96000000000004 , 10 );
  //
  //LinesBatch lb01( m_sName.GetString( ) , GetTimestamp_ms( ) - 25 , GetFrameId( ) - 1 , m_dPixelSize_mm );
  //lb01.AddLine( li01 , 389.30000000000001 , m_LastROI_px.Width() );
  //lb01.AddLine( li02 , 389.30000000000001 , m_LastROI_px.Width() );
  //
  //LineInfo li11( cmplx( 365.1 , 378.5 ) , 18.5 , 0 , 1 , 640.0 , 521.96000000000004 , 10 );
  //LinesBatch lb02( m_sName.GetString( ) , GetTimestamp_ms( ) , GetFrameId( ) , m_dPixelSize_mm );
  //lb02.AddLine( li11 , 386.03500000000003 , m_LastROI_px.Width() );
//
  //double dAvgShft_px = -1;
  //lb02.TryMatchByPrevBatch( lb01 , 5 , 10 , ms_dZeroPos_mm , __out dAvgShft_px );


  LinesBatchStatistics* pStatistics = NULL;

  if (ms_dZeroPos_mm > 0
    && ( ms_iMaskToSetZeroPos & m_GadgetMode ) == m_GadgetMode
    && !m_Statistics.TryGetMasterBaseInterval_px(10, m_dMasterBaseHorDist_px )
    && m_dMasterBaseHorDist_px==0)
      pStatistics = &m_Statistics;

  LinesBatch newBatch(m_sName.GetString( ), GetTimestamp_ms(), GetFrameId(), m_dPixelSize_mm );
  if (!TryExtractData(
    pDataFrame->CreateFramesIterator( text ) ,
    m_LastROI_px ,
    m_dMasterBaseHorDist_px ,
    m_dZeroPosOnImage_pix ,
    ms_dBaseHorDist_mm ,
    m_PartOfDigitsRegionInROI_px_Predicted ,
    __out newBatch ,
    __out pStatistics ))
  {
    pOut->AddFrame( CreateTextFrame( cViewPt , "0x0000ff" , 14 ,
      NULL , 0 , pStatistics == NULL ? "Error: Can't measure, adjust system position" : "Collecting data for calibration" ) );
  }
  else
  {
    const int c_iHistorySizeMax = 50;
    int iHistorySize = ( int ) m_LineBatches.size( );
    m_LineBatches.push_front( newBatch );
    
    if (iHistorySize >= c_iHistorySizeMax)
      m_LineBatches.pop_back( );

    if (m_pLastLines != NULL &&
      !m_pLastLines->IsEmpty( ) &&
      IsZeroPosCalibrated( ))
    {
      //m_PrevLines.Reset( );
      m_pPrevLines = m_pLastLines;
    }
    //m_LastLines.Reset( );
    
    m_pLL = &m_LineBatches[ 0 ];

    if (m_LineBatches.size( ) > 0)
      m_pLastLines = m_pLL;
//    m_LastLines = newBatch;
    ASSERT( m_pLastLines == m_pLL );

    double dZeroPosInROI_px;
    double dAbsShortestDist_px = 0;
    bool bIsShortestDistIsPrev = true;

    bool bIsReadyToCalibateZeroPos = IsReadyToCabrateZeroPosInROI_px( *m_pLastLines , ms_dZeroPos_mm , m_GadgetMode );
    
    if (bIsReadyToCalibateZeroPos)
    {
      if (!TryCalibrateZeroPosInROI_px(
        //m_LastLines ,
        *m_pLL,
        ms_dZeroPos_mm ,
        bIsReadyToCalibateZeroPos ,
        __out dZeroPosInROI_px ,
        __out dAbsShortestDist_px ,
        __out bIsShortestDistIsPrev ))
      {
        m_dZeroPosOnImage_pix = 0;
        m_dPixelSize_mm = 0;
        m_dLastX_mm = ms_dZeroPos_mm = 0;

        m_dMasterBaseHorDist_px = 0;

        m_ROIofZeroPos_mm.m_dBegin =
          m_ROIofZeroPos_mm.m_dEnd = 0;

        m_PartOfDigitsRegionInROI_px_Predicted.m_dBegin =
          m_PartOfDigitsRegionInROI_px_Predicted.m_dEnd = 0.;
      }
      else
      {
        m_dMasterBaseHorDist_px = m_pLastLines->GetBaseHorDist_px( );

        m_dZeroPosOnImage_pix = dZeroPosInROI_px;
        m_dPixelSize_mm = GetPixelSize_mm( ms_dBaseHorDist_mm , dAbsShortestDist_px );

        m_dLastV_pps = 0.;
        m_dLastX_mm = ms_dZeroPos_mm;

        double dROIBeginToZeroPosDist_mm = GetDistOfPixelsInMM( m_LastROI_px.left , m_dZeroPosOnImage_pix , m_dPixelSize_mm );
        double dZeroPosToROIEndDist_mm = GetDistOfPixelsInMM( m_dZeroPosOnImage_pix , m_LastROI_px.right , m_dPixelSize_mm );
        m_ROIofZeroPos_mm.m_dBegin = ms_dZeroPos_mm - dROIBeginToZeroPosDist_mm;
        m_ROIofZeroPos_mm.m_dEnd = ms_dZeroPos_mm + dZeroPosToROIEndDist_mm;


        ms_dAngleShift_deg = 0.;
      }
    }

    /*Print info on the frame*/
    double dUpperExtrem , dLowerExtrem;

    LineInfo* pLineNearest = m_pLastLines->GetLineNearestToROICntrX( );

    ASSERT( pLineNearest != NULL );

    GetMarkerExtrems( m_pLastVideoFrame , *pLineNearest , dUpperExtrem , dLowerExtrem , pOut );

    pOut->AddFrame( CreateTextFrame( cmplx( 300. , 20. ) , "0x008000" , 9 ,
      NULL , 0 , "Ids[%u,%u]=%d dT=%.1f" /*Aff=0x%03X"*/ ,
      ms_dLastVFIdPerGdgtMode[ 0 ] , ms_dLastVFIdPerGdgtMode[ 1 ] ,
      ( int )ms_dLastVFIdPerGdgtMode[ 0 ] - ( int )ms_dLastVFIdPerGdgtMode[ 1 ] ,
      ms_dLastFrameTimePerGdgtMode_ms[ 0 ] - ms_dLastFrameTimePerGdgtMode_ms[ 1 ] ,
      GetAffinity( ) ) );

    if (!IsZeroPosCalibrated( ))
    {
      pOut->AddFrame( CreateTextFrame( cViewPt , "0x0000ff" , 11 ,
        NULL , 0 , "N=%d Wp=%.1f dT=%.1f" ,
        m_pLastLines->GetSize( ) ,
        !m_pLastLines->HasThickLine( ) ? 0. : m_pLastLines->GetThickLineCntrPosX_px( ) /*m_dLastThickLinePos_pix */ ,
        m_dTimeBetweenFrames_ms ) );

      //m_sRecalibrateCause = "The thick line is missing";
      CTextFrame * pDoRecalibrate = CreateTextFrameEx(
        cmplx( 100. , m_cLastROI_Cntr_px.imag( ) ) ,
        0x0000ff , 18 , "Do Recalibrate\n%s" , (LPCTSTR)m_sRecalibrateCause );
      pOut->AddFrame( pDoRecalibrate );

    }
    else
    {

      if (m_dTimeBetweenFrames_ms >= c_dTimeThreshold_ms)
        m_dLastV_pps = 0.;

      // Expected shift because speed
      double dShiftExpected_px = m_dTimeBetweenFrames_ms * m_dLastV_pps * 1. / c_dMilliSecsPerSec; // times in msec
      double dShiftActual_px = 0;

      if (m_pPrevLines != NULL &&
        !m_pLastLines->TryMatchByPrevBatch(
          *m_pPrevLines ,
          dShiftExpected_px ,
          ms_dBaseHorDist_mm ,
          ms_dZeroPos_mm ,
          __out dShiftActual_px ))
      {
        m_dLastV_pps = 0.;
        m_dAcceleration_pps2 = 0.;

        //m_LastLines = m_PrevLines;
      }
      else
      {
        ASSERT( m_pLastLines->HasAllAbsPos_mm( ) );

        pLineNearest = m_pLastLines->GetLineNearestToROICntrX( );


        if (m_dTimeBetweenFrames_ms >= c_dTimeThreshold_ms)
        {
          m_dLastV_pps = 0.;
          m_dZeroPosOnImage_pix = 0;
          m_sRecalibrateCause.Format( "Timing Problem : dT = %.1f ms", 
            m_dTimeBetweenFrames_ms ); 
          CTextFrame * pDoRecalibrate = CreateTextFrameEx(
            cmplx( 100. , m_cLastROI_Cntr_px.imag() ) ,
            0x0000ff , 18 , "Do Recalibrate/n%s" , ( LPCTSTR )m_sRecalibrateCause );
          pOut->AddFrame( pDoRecalibrate );
        }
        else
        {
          double dActualV_pps = ( dShiftActual_px / m_dTimeBetweenFrames_ms ) * c_dMilliSecsPerSec;

          m_dAcceleration_pps2 = ( dActualV_pps - m_dLastV_pps ) * c_dMilliSecsPerSec / m_dTimeBetweenFrames_ms;

          //if (fabs( m_dLastV_pps ) > 0 && fabs( dActualV_pps - m_dLastV_pps ) > 20 * 40)
          //{
          //  m_sRecalibrateCause.Format( "Speed Problem:\n Va=%.1f mm/s \ndVa=%.1f mm/s (dVmax=%.1f mm/s)" ,
          //    dActualV_pps * m_dPixelSize_mm ,
          //    fabs( dActualV_pps - m_dLastV_pps ) * m_dPixelSize_mm ,
          //    20.0 );
          //  CTextFrame * pDoRecalibrate = CreateTextFrameEx(
          //    cmplx( 100. , m_cLastROI_Cntr_px.imag( ) ) ,
          //    0x0000ff , 18 , "Do Recalibrate/n%s" , ( LPCTSTR )m_sRecalibrateCause );
          //  pOut->AddFrame( pDoRecalibrate );
          //  m_dLastV_pps = 0.;
          //  m_dZeroPosOnImage_pix = 0;
          //}
          //else
          {
            m_dLastV_pps = dActualV_pps;
            //ASSERT( fabs( m_dAcceleration_mmps2 ) < 1000. );

            Segment1d partOfDigitsRgnInROI_px_forGraphics;
            double dCrntPosToZeroPosDist_mm = -1;
            //if (TryGetCurrentPosRelativeToZero_mm( pLineNearest , m_dZeroPosOnImage_pix , m_dPixelSize_mm , __out dCrntPosToZeroPosDist_mm )
            //  && dCrntPosToZeroPosDist_mm > 0)
            if (m_pLastLines->TryGetFinalPos_mm( dCrntPosToZeroPosDist_mm ))
            {
              m_dLastX_mm = dCrntPosToZeroPosDist_mm;

              partOfDigitsRgnInROI_px_forGraphics = GetPartOfDigitsRegionInROI_px( msc_DigitsRegionInPeriod_mm , m_dLastX_mm , m_dPixelSize_mm );

              double dPredictedAbsPos_mm = m_dLastX_mm - dShiftActual_px * m_dPixelSize_mm;

              m_PartOfDigitsRegionInROI_px_Predicted = GetPartOfDigitsRegionInROI_px( msc_DigitsRegionInPeriod_mm , dPredictedAbsPos_mm , m_dPixelSize_mm );
            }

            //
            if (/*IsDigitsInFOV( m_dLastX_mm , m_ROIofZeroPos_mm , m_DigitsRegionInROI_px )*/
              partOfDigitsRgnInROI_px_forGraphics.GetLength( ) > 0 && ms_iViewMode >= 5)
            {
              pOut->AddFrame( CreateLineFrameEx(
                cmplx( partOfDigitsRgnInROI_px_forGraphics.m_dBegin , m_LastROI_px.Height( ) - 90. ) ,
                cmplx( partOfDigitsRgnInROI_px_forGraphics.m_dEnd , m_LastROI_px.Height( ) - 90. ) ,
                0xff00ff , 3 ) );
            }
            // Print Zero position vertical line
            pOut->AddFrame( CreateLineFrame( cmplx( m_dZeroPosOnImage_pix , 100. ) ,
              cmplx( m_dZeroPosOnImage_pix , m_LastROI_px.Height( ) - 100. ) , 0xff00ff ) );
            // Print results in bottom
            CTextFrame * pXView = CreateTextFrameEx(
              cmplx( 100. , m_LastROI_px.Height( ) - 80. ) ,
              0x00ffff , 14 , "X=%.3fmm Angle=%.1fdeg" , m_dLastX_mm , ms_dAngle_deg );

            pXView->ChangeId( pDataFrame->GetId( ) );
            pXView->AddRef( );
            PutFrame( GetOutputConnector( 1 ) , pXView );
            pXView->Attributes( )->WriteString( "back" , "010101" );
            pOut->AddFrame( pXView );
          }
        }
      }

      if (fabs( m_dMaxV_pps ) < fabs( m_dLastV_pps ))
      {
        m_dMaxV_pps = m_dLastV_pps;
        m_dMaxSpeedSetTime_ms = GetHRTickCount( );
      }
      else if (GetHRTickCount( ) - m_dMaxSpeedSetTime_ms > 5000)
      {
        m_dMaxV_pps = 0.;
      }
      pOut->AddFrame( CreateTextFrame( cViewPt , "0x0000ff" , 10 ,
        NULL , 0 , "N=%d V=%.1f(%.1f)mm/s A=%.2fmm/s2 Sdeg=%.1f\ndT=%.1f Ang=%.1f Ash=%.1f Aby=%s Na=%d" ,
        m_pLastLines->GetSize( ) ,
        m_dMaxV_pps * m_dPixelSize_mm ,
        m_dLastV_pps * m_dPixelSize_mm ,
        m_dAcceleration_pps2 ,
        m_dLastSpaceSize_deg ,
        m_dTimeBetweenFrames_ms  ,
        ( m_GadgetMode == RTGM_Side ) ? ms_dSideAngle_deg : ms_dTopAngle_deg ,
        ms_dAngleShift_deg , m_pAngleSelectedBy , (int)m_AllSegments.size() ) );
    }
  }
  
  m_pLastVideoFrame = NULL;

  return pOut;
}


bool RulTracker::TryExtractData(
  CFramesIterator * pData ,
  const CRect& ROI_px ,
  double dMasterBaseHorDist_px ,
  double dZeroPosX_px ,
  double dBaseHorDist_mm ,
  const Segment1d& digitsRegionInROI_px ,
  __out LinesBatch& FoundLines ,
  __out LinesBatchStatistics* pStatistics )
{
  bool bIsDone = pData != NULL;

  const CTextFrame * pText = NULL;
  while (( pText = ( const CTextFrame * )pData->Next( ) ) && bIsDone)
  {
    FXString sLabel = pText->GetLabel( );
    FXPropertyKit pk( pText->GetString( ) );
    int iNLines , iNRemainLines = 0;
    int iSemicolonPos = ( int ) pk.Find( ':' );

    if (sLabel.Find( "Data_Line:" ) != 0
      || ( sLabel.Find( "markers" , 10 ) < 0
        && sLabel.Find( "body" , 10 ) < 0 )
      || !pk.GetInt( "Lines" , iNLines )
      || iNLines == 0
      || iSemicolonPos == 0)
      continue;

    iNRemainLines = iNLines;

    pk = pk.Mid( iSemicolonPos + 1 );

    FXSIZE iPos = 0;
    FXString Token = pk.Tokenize( ";" , iPos );

    if (sLabel.Find( "body" , 10 ) >= 0)
    {
      ASSERT( m_BodyInfo.FromString( Token ) );
      continue;
    }

    CLineResult NextLine;
    while (!Token.IsEmpty( ) && bIsDone)
    {
      iNRemainLines--;
      if (NextLine.FromString( Token ))
      {

        double dXCenterByRect = 0.5 * ( NextLine.m_DRect.left + NextLine.m_DRect.right );
        ASSERT( fabs( dXCenterByRect - NextLine.m_Center.x ) < 20. );

        //ASSERT( digitsRegionInROI_px.GetLength( ) <= 0. );
        if (digitsRegionInROI_px.GetLength( ) <= 0.
          || !( ( Segment1d& )digitsRegionInROI_px ).IsInSegment( NextLine.m_Center.x ))
        {
          LineInfo NewLine(
            CDPointToCmplx( NextLine.m_Center ) ,
            NextLine.m_DRect.Width( ) ,
            NextLine.m_iIndex ,
            FoundLines.GetTimeStamp_ms( ) ,
            ROI_px.CenterPoint( ).x ,
            dZeroPosX_px ,
            dBaseHorDist_mm );

          if (pStatistics != NULL)
          {
            pStatistics->Add( NewLine );
          }

          if (pStatistics == NULL)
          {
            bIsDone = FoundLines.AddLine(
              NewLine ,
              dMasterBaseHorDist_px ,
              ROI_px.right ,
              iNRemainLines == 0 ); //push_back( NewLine );

          }
        }
        else
        {
          TRACE( "\nPart of Digits Rgn is (%.3f-%.3f) and line center pos is %.3f."
            , digitsRegionInROI_px.m_dBegin
            , digitsRegionInROI_px.m_dEnd
            , NextLine.m_Center.x
          );
        }
      }


      Token = pk.Tokenize( ";" , iPos ).Trim( '\n' );
    }

    FoundLines.SetExpectedQty( iNLines );
    FoundLines.SetRawData( ( LPCSTR )( pText->GetString( ) ) );
  }

  return bIsDone && !FoundLines.IsEmpty( );
}

int RulTracker::GetMarkerExtrems( const CVideoFrame * pVF , LineInfo& Marker ,
  double& dUpperExtrem_pix , double& dLowerExtrem_pix , CContainerFrame * pMarking )
{
  int iSearchYShift_pix = ( int )Marker.GetWidth_px() * 3;
  CPoint Center = GetCPoint( Marker.GetCenterPos() );
  int iWidth = GetWidth( pVF );
  if (Center.x > iWidth / 2)
    iSearchYShift_pix = -iSearchYShift_pix;
  int iHalfRange = ( int )( m_BodyInfo.m_DRect.Height( ) * 0.7 );

  CPoint VertIter( Center.x + iSearchYShift_pix , Center.y - iHalfRange );

  Profile OnSide;
  CRect rcOnSide( Center.x + iSearchYShift_pix , Center.y - iHalfRange ,
    5 , 2 * iHalfRange );

  double dAverOnSide = calc_vprofile( pVF , &OnSide , &rcOnSide );

  double dMax = OnSide.m_dMaxValue;
  double dMin = OnSide.m_dMinValue;
  double dThres = ( dMin + dMax ) * 0.5;
  double * pProfData = OnSide.m_pProfData;
  cmplx UpperBodyEdge , LowerBodyEdge;
  int iYBegin = INT_MAX;
  int iYEnd = INT_MAX;

  m_AllSegments.clear();
  Segment1d NewSegment;

  while (( ( *pProfData > dThres ) || ( *( pProfData + 1 ) > dThres ) )
    && ( ( pProfData - OnSide.m_pProfData ) < OnSide.m_iProfLen ))
    pProfData++;
  if (( pProfData - OnSide.m_pProfData ) < OnSide.m_iProfLen - 1)
  {
    UpperBodyEdge._Val[ _RE ] = Center.x;
    UpperBodyEdge._Val[ _IM ] = iYBegin = rcOnSide.top + ( int ) ( pProfData - OnSide.m_pProfData );

    pMarking->AddFrame( CreatePtFrameEx( UpperBodyEdge , 0xff00ff , 3 ) );
    while (( ( *pProfData < dThres ) || ( *( pProfData + 1 ) < dThres ) )
      && ( ( pProfData - OnSide.m_pProfData ) < OnSide.m_iProfLen - 1 ))
      pProfData++;

    if (( pProfData - OnSide.m_pProfData ) < OnSide.m_iProfLen - 1)
    {
      LowerBodyEdge._Val[ _RE ] = Center.x;
      LowerBodyEdge._Val[ _IM ] = iYEnd = rcOnSide.top + ( int ) ( pProfData - OnSide.m_pProfData );
      pMarking->AddFrame( CreatePtFrameEx( LowerBodyEdge , 0xff00ff , 3 ) );

      int iXFrom = Center.x - 2 * ( int )Marker.GetWidth_px();
      int iRange = 2 * ( int )Marker.GetWidth_px();
      int iXTo = iXFrom + iRange;
      BYTE * pImage = GetData( pVF );
      double dWidth = 0.;
      bool bFound = false;
      cmplx cLastPt;

      for (int iY = iYBegin + 1; iY < iYEnd - 1; iY++)
      {
        const LPBYTE pPixFragment = pImage + iY * iWidth + Center.x;
        double dLinePos = find_line_pos_lr( pPixFragment , iRange ,
          0.5 , 20 , dWidth );
        cmplx cPos( Center.x + dLinePos , iY );
        if (dLinePos < iRange) // is found
        {
          if (!bFound)
          {
            bFound = true;
            NewSegment.m_dBegin = cPos.imag( );
            NewSegment.m_dEnd = 0.;
          }
          else
            cLastPt = cPos;
        }
        else //not found
        {
          if (bFound)
          {
            bFound = false;
            NewSegment.m_dEnd = cPos.imag( );
            m_AllSegments.push_back( NewSegment );
            NewSegment.m_dBegin = 0.;
          }
        }
      }
      if (NewSegment.m_dBegin > 0.)
      {
        NewSegment.m_dEnd = iYEnd - 1;
        m_AllSegments.push_back( NewSegment );
      }

    }
  }
  for (size_t i = 1; i < m_AllSegments.size( ); i++)
  {
    if (m_AllSegments[ i ].m_dBegin - m_AllSegments[ i - 1 ].m_dEnd < 10)
    {
      m_AllSegments[ i - 1 ].m_dEnd = m_AllSegments[ i ].m_dEnd;
      m_AllSegments.erase( m_AllSegments.begin( ) + (i--) );
    }
    else if ( m_AllSegments[i].GetLength() < 3 )
      m_AllSegments.erase( m_AllSegments.begin( ) + ( i-- ) );

  }
  int iNearestToCent = 0;
  bool bBegin = false;
  double dYDistB = DBL_MAX , dYDistE = DBL_MAX;
  double dYCenter = 0.5 * ( UpperBodyEdge.imag( ) + LowerBodyEdge.imag( ) );
  double dBodyRange = LowerBodyEdge.imag( ) - UpperBodyEdge.imag( );
  for (size_t i = 0; i < m_AllSegments.size( ); i++)
  {
    cmplx cBegin( Center.x , m_AllSegments[ i ].m_dBegin );
    cmplx cEnd( Center.x , m_AllSegments[ i ].m_dEnd );
    double dBegToCent = cBegin.imag( ) - dYCenter;
    if (abs( dBegToCent ) < abs( dYDistB ))
      dYDistB = dBegToCent;
    double dEndToCent = cEnd.imag( ) - dYCenter;
    if (abs( dEndToCent ) < abs( dYDistE ))
      dYDistE = dEndToCent;

    if (ms_iViewMode > 7)
    {
      pMarking->AddFrame( CreateLineFrameEx( cBegin , cEnd , 0x00ff00 , 1 ) );
    }
  }

  double dAngleB_deg = -RadToDeg( asin( 2. * dYDistB / dBodyRange ) );
  double dAngleE_deg = -RadToDeg( asin( 2. * dYDistE / dBodyRange ) );
  if (( dAngleB_deg < 0. ) && ( 0. < dAngleE_deg )) // two segments and space in the center
  {
    if (-dAngleB_deg < 60. && dAngleE_deg < 60.)
      m_dLastSpaceSize_deg = dAngleE_deg - dAngleB_deg;
  }
  if (fabs( dAngleB_deg ) <= 45.)
    m_dLastAngle_deg = dAngleB_deg;
  else if (fabs( dAngleE_deg ) <= 45.)
    m_dLastAngle_deg = dAngleE_deg - m_dLastSpaceSize_deg;

  if (m_GadgetMode == RTGM_Side)
  {
    ms_dSideAngle_deg = m_dLastAngle_deg;
    m_OldSideEdges_deg = ms_SideEdges_deg ;
    ms_SideEdges_deg.m_dBegin = dAngleB_deg ;
    ms_SideEdges_deg.m_dEnd = dAngleE_deg ;
    ms_dAngle_deg = GetAngle( );
  }
  else
  {
    ms_dTopAngle_deg = m_dLastAngle_deg;
    m_OldTopEdges_deg = ms_TopEdges_deg ;
    ms_TopEdges_deg.m_dBegin = dAngleB_deg ;
    ms_TopEdges_deg.m_dEnd = dAngleE_deg ;
  }

  return 0;
}


bool RulTracker::IsDigitsInFOV( double dCurrPos_mm , const Segment1d& FOVofZeroPos_mm, __out Segment1d& DigitsSegment_px )
{
  bool bRes = false;

  //Segment1d FOVofCurrentPos_mm = FOVofZeroPos_mm;
  //FOVofCurrentPos_mm.Offset( dCurrPos_mm - ms_dZeroPos_mm );

  //Segment1d FOVofCurrentPos_mm_Normalized;
  //FOVofCurrentPos_mm_Normalized.m_dBegin = fmod( FOVofCurrentPos_mm.m_dBegin , msc_iDigitsPeriod_mm );
  //FOVofCurrentPos_mm_Normalized.m_dEnd = FOVofCurrentPos_mm_Normalized.m_dBegin + FOVofZeroPos_mm.GetLength( );

  ////Segment1d InsideOrigin_mm( FOVofCurrentPos_mm_Normalized );
  ////DigitsSegment_px.m_dBegin = ms_dDigitsAreaShift_mm - ( msc_dDigitsAreaWidth_mm / 2. );
  ////DigitsSegment_px.m_dEnd = ms_dDigitsAreaShift_mm + ( msc_dDigitsAreaWidth_mm / 2. );
  //// Now DigitsSegment_px is in mm
  //if (FOVofCurrentPos_mm_Normalized.Intersection( msc_DigitsAreaPosInPmsc_DigitsAreaPosInPeriod_mmSegment_px.m_dBegin -= FOVofCurrentPos_mm_Normalized.m_dBegin; // InsideOrigin_mm.m_dBegin;
  //  DigitsSegment_px.m_dBegin /= m_dPixelSize_mm;
  //  DigitsSegment_px.m_dEnd -= FOVofCurrentPos_mm_Normalized.m_dBegin; // InsideOrigin_mm.m_dBegin;
  //  DigitsSegment_px.m_dEnd /= m_dPixelSize_mm;
  //  return true;
  //}
  return false;
}

//bool RulTracker::IsDigitsInFOV( double dCurrPos_mm , Segment1d& DigitsSegment_px )
//{
//  m_CurrentViewArea_mm = m_FOVofZeroPos_mm ;
//  m_CurrentViewArea_mm.Offset( dCurrPos_mm - ms_dZeroPos_mm );
//
//  Segment1d FOVofCurrentPos_mm_Normalized ; // What we see inside digits period
//  FOVofCurrentPos_mm_Normalized.m_dBegin = fmod( m_CurrentViewArea_mm.m_dBegin , msc_iDigitsPeriod_mm );
//  FOVofCurrentPos_mm_Normalized.m_dEnd = FOVofCurrentPos_mm_Normalized.m_dBegin + m_FOVofZeroPos_mm.GetLength() ;
//
//  bool bBeginIn = m_DigitsArea_mm.IsInSegment( FOVofCurrentPos_mm_Normalized.m_dBegin );
//  bool bEndIn = m_DigitsArea_mm.IsInSegment( FOVofCurrentPos_mm_Normalized.m_dBegin );
//
//  if (!bBeginIn && !bEndIn)
//    return false;
//  double dShift_mm = m_CurrentViewArea_mm.m_dBegin ;
//  if ( bBeginIn )
//    DigitsSegment_px.m_dBegin = FOVofCurrentPos_mm_Normalized.m_dBegin / m_dScale_mm_per_pix;
//  if (bEndIn)
//    DigitsSegment_px.m_dEnd = FOVofCurrentPos_mm_Normalized.m_dEnd / m_dScale_mm_per_pix;
//  if (!bBeginIn)
//    DigitsSegment_px.m_dBegin = m_DigitsArea_mm.m_dBegin / m_dScale_mm_per_pix;
//  if (!bEndIn)
//    DigitsSegment_px.m_dEnd = m_DigitsArea_mm.m_dEnd / m_dScale_mm_per_pix;
//
//
//  return true ;
//}


double RulTracker::GetAngle() 
{
  if (( m_AllSegments.size( ) == 1 )
    && ( 45 < ms_SideEdges_deg.m_dBegin )
    && ( m_OldSideEdges_deg.m_dBegin <= 45.0 ) 
    && ( 0. < m_OldSideEdges_deg.m_dBegin ))
  {
    ms_dAngleShift_deg += 180.;
  }
  else if (( m_AllSegments.size( ) == 1 )
    && (45. < m_OldSideEdges_deg.m_dBegin)
    && (ms_SideEdges_deg.m_dBegin <= 45.0) 
    && ( 0. < ms_SideEdges_deg.m_dBegin ))
  {
    ms_dAngleShift_deg -= 180. ;
  }
  if (-45. < ms_SideEdges_deg.m_dBegin && ms_SideEdges_deg.m_dBegin <= 45.)
  {
    ms_dAngle_deg = ms_SideEdges_deg.m_dBegin + ms_dAngleShift_deg;
    m_pAngleSelectedBy = "SB";
  }
  else if (-45. < ms_SideEdges_deg.m_dEnd && ms_SideEdges_deg.m_dEnd <= 45.)
  {
    ms_dAngle_deg = ms_SideEdges_deg.m_dEnd + ms_dAngleShift_deg - m_dLastSpaceSize_deg;
    m_pAngleSelectedBy = "SE";
  }
  else if (-45. < ms_TopEdges_deg.m_dBegin && ms_TopEdges_deg.m_dBegin <= 45.)
  {
    ms_dAngle_deg = ms_TopEdges_deg.m_dBegin + ms_dAngleShift_deg - 90.;
    m_pAngleSelectedBy = "TB";
  }
  else if (-45. < ms_TopEdges_deg.m_dEnd && ms_TopEdges_deg.m_dEnd <= 45.)
  {
    ms_dAngle_deg = ms_TopEdges_deg.m_dEnd + ms_dAngleShift_deg - m_dLastSpaceSize_deg - 90.;
    m_pAngleSelectedBy = "TE";
  }
  else
    ASSERT( 0 );
  return ms_dAngle_deg ;
}




int RulTracker::FormAndSendResultView( )
{
  pTVFrame pTv = makeNewY8Frame( m_LastROI_px.right , m_LastROI_px.bottom );
  CVideoFrame * pResultView = CVideoFrame::Create( pTv );
  m_cLastROI_Cntr_px = cmplx( m_LastROI_px.right / 2. , m_LastROI_px.bottom / 2. );
  int iWidth = GetWidth( pResultView );
  int iHeight = GetHeight( pResultView );
  memset( GetData( pResultView ) , 128 , iWidth * iHeight );

  CContainerFrame * pOut = CContainerFrame::Create( );
  pOut->AddFrame( pResultView );
  m_dLastResultViewTime = GetHRTickCount( );

  return ( PutFrame( GetOutputConnector( RTOC_ResultView ) , pOut ) != false );
}

int RulTracker::SetCameraExposure( int iExposure_us )
{
  FXString CameraCommand;
  CameraCommand.Format( "shutter_us=%d;" , iExposure_us );
  return CameraControl( CameraCommand );
}

BOOL RulTracker::CameraControl( LPCTSTR pCommand )
{
  FXString Command;
  Command.Format( "set properties(%s;);" , pCommand );
  CTextFrame * pOut = CreateTextFrame( Command , "CameraCommand" );
  return PutFrame( GetOutputConnector( RTOC_CamControl ) , pOut , 200 );
}