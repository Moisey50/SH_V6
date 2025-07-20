#include "stdafx.h"
#include "imageproc/statistics.h"
#include "SkewMeter.h"
#include "fxfc/FXRegistry.h"
#include <iostream>
#include <fstream>

USER_FILTER_RUNTIME_GADGET( SkewMeter , "Video.FileX_Specific" );

double   SkewMeter::m_dHorLineVertShiftFromLUCorner_um = 0. ;
double   SkewMeter::m_dVertLineHorShiftFromLUCorner_um = 0. ;
double   SkewMeter::m_dHorLineVertShiftFromRUCorner_um = 0. ;
double   SkewMeter::m_dVertLineHorShiftFromRUCorner_um = 0. ;
double   SkewMeter::m_dHorLineVertShiftFromLDCorner_um = 0. ;
double   SkewMeter::m_dVertLineHorShiftFromLDCorner_um = 0. ;
double   SkewMeter::m_dVertDistBetweenSquares_mm = 240. ;
double   SkewMeter::m_dHorDistBetweenSquares_mm = 240. ;
double   SkewMeter::m_dBaseHorDist_mm = 280. ;
double   SkewMeter::m_dBaseVertDist_mm = 300. ;
// 2200um - distance between pattern on internal film and image on paper
// 88000um - distance from lens to pattern
double   SkewMeter::m_dParallaxScale = ( 1. + ( 2200. / ( 75000. + 2200. ) ) ) ;
double   SkewMeter::m_dA = -3. ; // Y = ax + b, for lens distortion correction
double   SkewMeter::m_dB = 20. ;


int      SkewMeter::m_iViewMode = 5 ;
int      SkewMeter::m_iCaptureMode = -1 ;
bool     SkewMeter::m_bResultOK = false ;
bool     SkewMeter::m_bBatteriesStatusUpdated = false ;
bool     SkewMeter::m_bBatteriesAreOK = false ;

bool     SkewMeter::m_bLEL_OK = false ;
bool     SkewMeter::m_bLER_OK = false ;
bool     SkewMeter::m_bTEL_OK = false ;
bool     SkewMeter::m_bLEL_Stable = false ;
bool     SkewMeter::m_bLER_Stable = false ;
bool     SkewMeter::m_bTEL_Stable = false ;

cmplx   SkewMeter::m_cLeftTopRectCenter_pix ;
cmplx   SkewMeter::m_cRightRectCenter_pix ;
cmplx   SkewMeter::m_cBottomRectCenter_pix ;
cmplx   SkewMeter::m_cLeftTopLinesCross_pix ;
cmplx   SkewMeter::m_cRightLinesCross_pix ;
cmplx   SkewMeter::m_cBottomLinesCross_pix  ;

cmplx    SkewMeter::m_cLeftTopLineCrossToRectCent_um ;
cmplx    SkewMeter::m_cRightLineCrossToRectCent_um ;
cmplx    SkewMeter::m_cBottomLineCrossToRectCenter_um ;

int      SkewMeter::m_PowerStatus = PS_Unknown ;
int      SkewMeter::m_PowerStatusSaved = PS_Unknown ;


static bool g_bEndOfWork = false ;

VOID CALLBACK MainLoopTimerRoutine( LPVOID lpParam , BOOLEAN TimerOrWaitFired )
{
  if ( !g_bEndOfWork )
  {
    SkewMeter * pGadget = ( SkewMeter* ) lpParam;
//     if ( pGadget->m_pStatus->GetStatus() == CExecutionStatus::RUN )
//     {
    pGadget->ProcessTimer() ;
//     }
  }
}

void SkewMeter::ConfigParamChange( LPCTSTR pName , void* pObject , 
  bool& bInvalidate , bool& bInitRescan )
{
  SkewMeter * pGadget = ( SkewMeter* ) pObject;
  if ( pGadget )
  {
    if ( !_tcsicmp( pName , _T( "Thres-SkewSzVDist" ) ) )
    {
      FXSIZE iPos = 0 ;
      FXString Token = pGadget->m_sThresholdForLog.Tokenize( "," , iPos ) ;
      if ( ( !Token.IsEmpty() ) )
      {
        Token.Trim() ;
        if ( !Token.IsEmpty() )
          pGadget->m_dSkewThresForLog_um = atof( Token ) ;
        Token = pGadget->m_sThresholdForLog.Tokenize( "," , iPos ) ;
        if ( ( !Token.IsEmpty() ) )
        {
          Token.Trim() ;
          if ( !Token.IsEmpty() )
            pGadget->m_dSizeThresForLog_um = atof( Token ) ;
          Token = pGadget->m_sThresholdForLog.Tokenize( "," , iPos ) ;
          if ( ( !Token.IsEmpty() ) )
          {
            Token.Trim() ;
            if ( !Token.IsEmpty() )
              pGadget->m_dHLineDistThresForLog_um = atof( Token ) ;
          }
        }
      }
    }
  }
}


SkewMeter::SkewMeter()
{
  m_GadgetMode = SMGM_Unknown ;
  m_OutputMode = modeReplace ;
  m_dScale_um_per_pix ;
  m_dTolerance_um = 7. ;
  m_LastROI = CRect( 0 , 0 , 1440 , 1080 ) ;
  m_CrossCenters.resize( N_SAMPLES_FOR_STABILITY_CHECK ) ;
  m_RectCenters.resize( N_SAMPLES_FOR_STABILITY_CHECK ) ;
  memset( m_CrossCenters.data() , 0 , N_SAMPLES_FOR_STABILITY_CHECK * sizeof( cmplx ) ) ;
  memset( m_RectCenters.data() , 0 , N_SAMPLES_FOR_STABILITY_CHECK * sizeof( cmplx ) ) ;
  init() ;
}


SkewMeter::~SkewMeter()
{
  if ( m_pLastResultViewWithoutPower )
  {
    m_pLastResultViewWithoutPower->Release() ;
    m_pLastResultViewWithoutPower = NULL ;
  }
}

static const char * pGadgetMode = "Unknown;LeftTop;Right;Bottom" ;
static const char * pCalibrationCommand = "NoCalib;CalibSkewB;CalibSkewA;CalibSizes;" ;

void SkewMeter::PropertiesRegistration()
{
  addProperty( SProperty::COMBO , _T( "GadgetMode" ) , ( int * ) &m_GadgetMode ,
    SProperty::Int , pGadgetMode ) ;
  addProperty( SProperty::SPIN , "ViewMode" , &m_iViewMode , SProperty::Int , 0 , 100 ) ;
  addProperty( SProperty::EDITBOX , "InitialDirection_deg" , &m_dInitialDirection_deg , SProperty::Double ) ;
  addProperty( SProperty::EDITBOX , "SquareSize_um" , &m_dSquareSize_um , SProperty::Double ) ;
  addProperty( SProperty::EDITBOX , "VertDistBetweenRects_mm" , &m_dVertDistBetweenSquares_mm , SProperty::Double ) ;
  addProperty( SProperty::EDITBOX , "HorDistBetweenSquares_mm" , &m_dHorDistBetweenSquares_mm , SProperty::Double ) ;
  addProperty( SProperty::EDITBOX , "BaseHorDist_mm" , &m_dBaseHorDist_mm , SProperty::Double ) ;
  addProperty( SProperty::EDITBOX , "BaseVertDist_mm" , &m_dBaseVertDist_mm , SProperty::Double ) ;
  addProperty( SProperty::EDITBOX , "Scale_um/pix" , &m_dScale_um_per_pix , SProperty::Double ) ;
  addProperty( SProperty::EDITBOX , "Tolerance_um" , &m_dTolerance_um , SProperty::Double ) ;
  addProperty( SProperty::EDITBOX , "SearchWidth_pix" , &m_dSearchWidth_pix , SProperty::Double ) ;
  addProperty( SProperty::EDITBOX , "SquareSearchArea_perc" ,
    &m_dSquareSearchArea_perc , SProperty::Double ) ;
  addProperty( SProperty::EDITBOX , "MinimalContrast" ,
    &m_dMinimalContrast , SProperty::Double ) ;
  addProperty( SProperty::EDITBOX , "LineThreshold" ,
    &m_dLineSearchThres , SProperty::Double ) ;
  addProperty( SProperty::COMBO , _T( "Calibration" ) , ( int * ) &m_CalibCommand ,
    SProperty::Int , pCalibrationCommand ) ;
  addProperty( SProperty::SPIN , "RecordPeriod_sec" , &m_iRecordPeriod_sec , SProperty::Int , 0 , 3600 ) ;
  addProperty( SProperty::EDITBOX , "ParallaxScale" ,
    &m_dParallaxScale , SProperty::Double ) ;
  addProperty( SProperty::EDITBOX , "DistCorrectionA" ,
    &m_dA , SProperty::Double ) ;
  addProperty( SProperty::EDITBOX , "DistCorrectionB" ,
    &m_dB , SProperty::Double ) ;
  addProperty( SProperty::EDITBOX , "Thres-SkewSzVDist" ,
    &m_sThresholdForLog , SProperty::String ) ;
  SetChangeNotificationForLast( ConfigParamChange , this ) ;
};

void SkewMeter::ConnectorsRegistration()
{
  addInputConnector( transparent , "FigureInput" );
  addOutputConnector( transparent , "OutputView" );
  addOutputConnector( transparent , "ResultView" ) ;
  addOutputConnector( text , "CameraControl" ) ;
  GetInputConnector( 0 )->SetQueueSize( 10 ) ;
  GetInputConnector( 0 )->Send( CreateTextFrame( "Init" , "Init" ) ) ;
};

void SkewMeter::ShutDown()
{
//   if ( m_GadgetMode == SMGM_LeftTop)
//   {
  g_bEndOfWork = true ;
  DeleteAsyncTimer() ;
  Sleep( 150 ) ;
//   }
  CFilterGadget::ShutDown();
//    CGadget::ShutDown();
}

CDataFrame* SkewMeter::DoProcessing( const CDataFrame* pDataFrame )
{
  double dStart = GetHRTickCount() ;
  if ( !m_hMainTimer )
  {
    if ( !CreateTimerQueueTimer( &m_hMainTimer , NULL ,
      ( WAITORTIMERCALLBACK ) MainLoopTimerRoutine ,
      this , 100 , 100 , 0 ) )
    {
      SEND_GADGET_ERR( "Create Power Status timer is failed" );
    }
  }

  if ( !pDataFrame->IsContainer() )
  {
    if ( pDataFrame->GetDataType() == text )
    {
      if ( _tcsstr( pDataFrame->GetLabel() , _T( "Basler" ) ) )
      {
        int iRes = AnalyzeUPSStatus( pDataFrame->GetTextFrame() ) ;
      }
      else if ( _tcsicmp( pDataFrame->GetLabel() , _T( "CheckPower" ) ) == 0 )
      {
        LPCTSTR szpPowerStatus = AnalyzePower() ;
        CTextFrame * pRequestFromCameras = CTextFrame::Create( "get io_status" ) ;
        pRequestFromCameras->SetLabel( "GetIO" ) ;
        PutFrame( GetOutputConnector( OC_CamControl ) , pRequestFromCameras ) ;
        if ( ( ( GetHRTickCount() - m_dLastResultViewTime ) > 2000. )
          && m_pLastResultViewWithoutPower )
        {
          CContainerFrame * pOut = ( CContainerFrame * ) ( m_pLastResultViewWithoutPower->Copy() ) ;
          AddPowerStatusAndControls( pOut ) ;
          m_dLastResultViewTime = GetHRTickCount() ;
          PutFrame( GetOutputConnector( OC_ResultView ) , pOut ) ;
        }
      }
      else
      {
        FXPropertyKit pk( pDataFrame->GetTextFrame()->GetString() ) ;
        int iX = 0 , iY = 0 ;
        if ( pk.GetInt( "x" , iX ) && pk.GetInt( "y" , iY )
          && ( pk.Find( "selected" ) >= 0 ) )
        {
          int iButton = IsThereClickedButton( iX , iY ) ;
          switch ( iButton )
          {
            case CM_LiveProcess:
            {
              if ( m_iCaptureMode == CM_LiveProcess )
                m_iLastSelectedByUI = m_iCaptureMode = CM_Pause ;
              else
                m_iLastSelectedByUI = m_iCaptureMode = CM_LiveProcess ;
              break ;
            }
            case CM_LiveView:
            {
              if ( m_iCaptureMode == CM_LiveView )
                m_iLastSelectedByUI = m_iCaptureMode = CM_Pause ;
              else
                m_iLastSelectedByUI = m_iCaptureMode = CM_LiveView ;
              break ;
            }
            case CM_OneProcess:
            {
              m_iLastSelectedByUI = m_iCaptureMode = CM_OneProcess ;
              break ;
            }
            case CM_Quit:
            {
              DWORD dwProcessId = GetCurrentProcessId() ;
              HANDLE h = OpenProcess( PROCESS_TERMINATE , FALSE , dwProcessId );
              if ( NULL != h )
              {
                TerminateProcess( h , 0 );
                CloseHandle( h );
              }

              return NULL ;
            }
          }
          if ( m_iCaptureMode )
            DoOrderGrab() ;
        }
      }
      if ( ( !m_bBatteriesAreOK /*|| !m_bBatteriesStatusUpdated*/ )
        && m_GadgetMode == SMGM_LeftTop )
      {
        FormAndSendResultView() ;
      }

      return NULL ;
    }
  }
  CContainerFrame * pOut = CContainerFrame::Create() ;
  pOut->CopyAttributes( pDataFrame ) ;

  pOut->AddFrame( pDataFrame ) ;

  FXIntArray LineCnt , NSegments ;
  FXDblArray Times ;
  //#ifdef _DEBUG
  FXString DiagInfo ;
  //#endif
  m_pLastVideoFrame = pDataFrame->GetVideoFrame() ;
  if ( m_pLastVideoFrame )
  {
    m_dLastFrameTime = GetHRTickCount() ;
    m_LastROI.right = GetWidth( m_pLastVideoFrame ) ;
    m_LastROI.bottom = GetHeight( m_pLastVideoFrame ) ;

//     int iCutLevel = GetCutLevelByHisto8( m_pLastVideoFrame , 0.01 ) ;
//     if ( iCutLevel > 245 )
//     {
// 
//     }
  }
  m_cLastROICent_pix = cmplx( ( double ) m_LastROI.CenterPoint().x , ( double ) m_LastROI.CenterPoint().y ) ;

  int iRes = SimplifiedProcessing( pOut ) ;
  Tetragon Square ;
  cmplx cViewPt( m_cLastROICent_pix * 0.1 ) ;
  m_cHorCross = m_cVertCross = cmplx() ;
  bool bFormIsOK = false ;

  if ( FillTetragon( m_IntersectionsArea , m_Intersections , Square ) )
  {
    if ( Square.IsFilled() && Square.IsRomb( 1.0 ) )
    {
      for ( size_t i = 0 ; i < m_Intersections.size() ; i++ )
      {
        cmplx cCross = m_Intersections[ i ] ;

        if ( ( cCross != Square.m_c1st )
          && ( cCross != Square.m_c2nd )
          && ( cCross != Square.m_c3rd )
          && ( cCross != Square.m_c4th ) )
        {
          m_Square = Square ;
          double dDistToUpperLine = fabs( GetPtToLineDistance(
            cCross , Square.m_c1st , Square.m_c2nd ) ) ;
          double dDistToLeftLine = fabs( GetPtToLineDistance(
            cCross , Square.m_c1st , Square.m_c4th ) ) ;
          if ( dDistToUpperLine < 3 ) // on left vertical edge
            m_cHorCross = cCross ;
          else if ( dDistToLeftLine < 3 ) // on top horizontal edge
            m_cVertCross = cCross ;
        }
      }

      double dAverageSide = m_Square.Perimeter() / 4. ;
      m_dScale_um_per_pix = m_dSquareSize_um / dAverageSide ;
      CTextFrame * pScale = CTextFrame::Create() ;
      pScale->SetLabel( "Scale&Units" ) ;
      pScale->GetString().Format( "%.5f,um" , m_dScale_um_per_pix ) ;
      PutFrame( m_pOutput , pScale ) ;
      CalcDistsAndViewRectData( pOut ) ;
      bFormIsOK = true ;
    }
    else
      pOut->AddFrame( CreateTextFrame( cViewPt , "0x0000ff" , 14 ,
        NULL , 0 , "Error: Can't measure, not proper form, adjust system position" ) ) ;
  }
  else
    pOut->AddFrame( CreateTextFrame( cViewPt , "0x0000ff" , 14 ,
      NULL , 0 , "Error: Can't measure, adjust system position" ) ) ;

  if ( m_iCaptureMode >= CM_LiveProcess ) // Live process or single frame
  {
    if ( bFormIsOK )
    {
      m_cRectCenter_pix = Square.GetCenter() ;
      m_cRectCenterToFOVCenter_pix = m_cRectCenter_pix - m_cLastROICent_pix ;
      m_cRectCenterToFOVCenter_pix *= m_dParallaxScale /*- 1.0*/ ;
      m_cRectCenterToFOVCenter_um = m_cRectCenterToFOVCenter_pix * m_dScale_um_per_pix ;

      m_cAvgRectCenterToFOVCenter_um = AddCmplxValAndCalcAverageAndStd(
        m_cRectCenterToFOVCenter_um , m_RectCenters , m_dLastSquareStd ) ;

      m_cVHLinesCrossToFOVCenter_pix = m_cVHLinesCross_pix - m_cLastROICent_pix ;
      m_cVHLinesCrossToFOVCenter_pix *= m_dParallaxScale /*- 1.0*/ ;

      m_cVHLinesCrossToFOVCenter_um = m_cVHLinesCrossToFOVCenter_pix * m_dScale_um_per_pix ;
      m_cAvgVHLinesCrossToFOVCenter_um = AddCmplxValAndCalcAverageAndStd(
        m_cVHLinesCrossToFOVCenter_um , m_CrossCenters , m_dLastCrossStd ) ;

      bool bStable = ( m_dLastCrossStd < 1.0 ) && ( m_dLastSquareStd < 1.0 ) ;
      cmplx cStableViewPt( m_LastROI.right * 0.9 , m_LastROI.bottom * 0.15 ) ;
      pOut->AddFrame( CreateTextFrameEx( cStableViewPt ,
        bStable ? "Stable" : "Not Stable" , bStable ? 0x008000 : 0x0000ff , 16 ) ) ;

      m_cLineCrossOffsetToRectCenter_um =
        m_cAvgVHLinesCrossToFOVCenter_um - m_cAvgRectCenterToFOVCenter_um  ;

      cmplx cScaleViewPt( m_LastROI.right * 0.01 , m_LastROI.bottom * 0.95 ) ;
      pOut->AddFrame( CreateTextFrameEx( cScaleViewPt , 0x00ff0000 , 10 ,
        "Scale=%.5fum/pix  Roff=(%.2f,%.2f)um Ang=%.4fdeg Coff=(%.2f,%.2f)um" ,
        m_dScale_um_per_pix , m_cRectCenterToFOVCenter_pix ,
        RadToDeg( arg( Square.m_c2nd - Square.m_c1st ) ) ,
        m_cLineCrossOffsetToRectCenter_um ) ) ;

      switch ( m_GadgetMode )
      {
        case SMGM_LeftTop: ProcessLeftTopImage( pDataFrame , pOut ) ; break ;
        case SMGM_Right: ProcessRightImage( pDataFrame , pOut ) ; break ;
        case SMGM_Bottom: ProcessBottomImage( pDataFrame , pOut ) ; break ;
      }
    }
  }
  else if ( ( m_iCaptureMode == CM_LiveView ) && ( m_GadgetMode == SMGM_LeftTop ) )
    FormAndSendResultView() ;

  if ( bFormIsOK )
  {
    cmplx cRectCent = m_Square.GetCenter() ;
    cmplx cDiag1_4 = ( m_Square.m_c3rd - m_Square.m_c1st ) * 0.25 ;
    cmplx cDiag2_4 = ( m_Square.m_c4th - m_Square.m_c2nd ) * 0.25 ;
    cmplx WorkingArea[] = { m_Square.m_c1st + cDiag1_4 ,
      m_Square.m_c2nd + cDiag2_4 ,
      m_Square.m_c3rd - cDiag1_4 ,
      m_Square.m_c4th - cDiag2_4 ,
      m_Square.m_c1st + cDiag1_4 } ;

    FXString Attrib ;
    Attrib.Format( "color=0X%X; style=%d;thickness=%d;" ,
      m_TargetRectColor , m_TargetRectStyle , m_iTargetRectThickness ) ;
    pOut->AddFrame( CreateFigureFrameEx( WorkingArea , 5 , ( LPCTSTR ) Attrib ) ) ;
  }

  m_pLastVideoFrame = NULL ;

  return pOut ;
}

void SkewMeter::ProcessLeftTopImage( const CDataFrame * pDataFrame , CContainerFrame * pMarking )
{
//   FXRegistry Reg( "TheFileX\\SkewMeter" ) ;
  m_bLEL_OK = false ;
  cmplx cViewPt( m_cLastROICent_pix * 0.1 ) ;
  m_bResultOK = m_bSkewOK = m_bHScaleOK = m_bVScaleOK = false ;

  if ( m_Intersections.size() < 9 )
  {
    pMarking->AddFrame( CreateTextFrame( cViewPt , "0x0000ff" , 14 ,
      NULL , 0 , "Bad position:\nPut intersection inside square" ) ) ;
    return ;
  }

  if ( m_Square.IsFilled() && m_cVertCross.real() && m_cHorCross.real() )
  {
    double dHorShift_um = m_dScale_um_per_pix * abs( m_cHorCross - m_Square.m_c1st ) ;
    double dVertShift_um = m_dScale_um_per_pix * abs( m_cVertCross - m_Square.m_c1st ) ;

    m_dHorLineVertShiftFromLUCorner_um = dVertShift_um ;
    m_dVertLineHorShiftFromLUCorner_um = dHorShift_um ;
    if ( 1 /*m_bLER_OK && m_bTEL_OK*/ )
    {
      // Skew calculation by left top square corner and distances to upper and left sides
      double dY = m_dHorLineVertShiftFromRUCorner_um - m_dHorLineVertShiftFromLUCorner_um ;
      double dX = m_dVertLineHorShiftFromLDCorner_um - m_dVertLineHorShiftFromLUCorner_um ;
      double dHorAngle = atan2( -dY , m_dHorDistBetweenSquares_mm * 1000. ) ;
      double dVertAngle = atan2( dX , m_dVertDistBetweenSquares_mm * 1000. ) ;
      double dDiff = dHorAngle - dVertAngle ;
      m_dSkew_deg = RadToDeg( dDiff ) ;
      double dError_um = m_dSkew_um = m_dBaseHorDist_mm * dDiff * 1000. ;

      // Skew calculation by square center and lines cross
      m_cLeftTopRectCenter_pix = m_cRectCenterToFOVCenter_pix ;
      m_cLeftTopLinesCross_pix = m_cVHLinesCross_pix ;
      m_cLeftTopLineCrossToRectCent_um = m_cLineCrossOffsetToRectCenter_um  ;

      dY = m_cLeftTopLineCrossToRectCent_um.imag()
        - m_cRightLineCrossToRectCent_um.imag()  ;
      dX = m_cBottomLineCrossToRectCenter_um.real()
        - m_cLeftTopLineCrossToRectCent_um.real() ;
      dHorAngle = atan2( dY , m_dHorDistBetweenSquares_mm * 1000. ) ;
      dVertAngle = atan2( dX , m_dVertDistBetweenSquares_mm * 1000. ) ;
      dDiff = dHorAngle - dVertAngle ;
      m_dSkewByCent_deg = RadToDeg( dDiff ) ;
      // dError will be taken from measurements by center
      dError_um = m_dSkewByCent_um = m_dBaseHorDist_mm * dDiff * 1000. ;


      // Correction by Skew = Skew + dA * dNormHorLineHeight + dB ;
      // Correction is per system, dependent on light and optics
      double dCorrection_um = m_dB
        + m_dA * ( m_cLeftTopLineCrossToRectCent_um.imag() / ( m_Square.Perimeter() * 0.125 ) ) ;
      m_dCorrectedSkew_um = m_dSkew_um + dCorrection_um ;
      m_dCorrectedSkewByCent_um = m_dSkewByCent_um + dCorrection_um ;

      m_dHorErr_um = m_dVertLineHorShiftFromRUCorner_um
        - m_dVertLineHorShiftFromLUCorner_um
        + ( m_dHorDistBetweenSquares_mm - m_dBaseHorDist_mm ) * 1000. ;
      m_dVertErr_um = m_dHorLineVertShiftFromLDCorner_um
        - m_dHorLineVertShiftFromLUCorner_um
        + ( m_dVertDistBetweenSquares_mm - m_dBaseVertDist_mm ) * 1000. ;
      m_bLEL_OK = true ;

      if ( m_bLEL_OK )
      {
        m_bSkewOK = ( abs( m_dCorrectedSkewByCent_um ) < m_dTolerance_um ) ;
        m_bHScaleOK = ( abs( m_dHorErr_um ) <= m_dTolerance_um ) ;
        m_bVScaleOK = ( abs( m_dVertErr_um ) <= m_dTolerance_um ) ;
        if ( m_iViewMode > 7 )
        {
          CTextFrame * pInfo = CreateTextFrameEx( cViewPt , 0x00ffff , 14 ,
            "Positions [%.2f,%.2f]um, Deltas [%.2f,%.2f]um , Angle=%.2fmrad(%.4fdeg)" ,
            m_dVertLineHorShiftFromLUCorner_um , m_dHorLineVertShiftFromLUCorner_um ,
            dX , -dY ,
            dDiff * 1000. , m_dSkewByCent_deg ) ;

          *( pInfo->Attributes() ) += "back=0;" ;
          pMarking->AddFrame( pInfo ) ;

          DWORD dwColor = ( m_dTolerance_um == 0. ) ? 0x0000ffff :
            ( m_bSkewOK ? 0x00ff00 : 0x0000ff ) ;
          pInfo = CreateTextFrameEx( cViewPt ,
            dwColor , 14 , "\n\n\nSkew=%.2fum per %.2f mm" ,
            dError_um , m_dBaseHorDist_mm ) ;
          *( pInfo->Attributes() ) += "back=0;" ;
          pMarking->AddFrame( pInfo ) ;

          dwColor = ( m_dTolerance_um == 0. ) ? 0x0000ffff :
            ( m_bHScaleOK ? 0x00ff00 : 0x0000ff ) ;
          pInfo = CreateTextFrameEx( cViewPt ,
            dwColor , 14 , "\n\n\n\nHor Scale Error=%.2fum per %.2f mm (%.2fmm)" ,
            m_dHorErr_um , m_dBaseHorDist_mm ,
            m_dBaseHorDist_mm + m_dHorErr_um * 0.001 ) ;
          *( pInfo->Attributes() ) += "back=0;" ;
          pMarking->AddFrame( pInfo ) ;

          dwColor = ( m_dTolerance_um == 0. ) ? 0x0000ffff :
            ( m_bVScaleOK ? 0x00ff00 : 0x0000ff ) ;
          pInfo = CreateTextFrameEx( cViewPt , dwColor , 14 ,
            "\n\n\n\n\nVert Scale Error=%.2fum per %.2f mm (%.3fmm)" ,
            m_dVertErr_um , m_dBaseVertDist_mm ,
            m_dBaseVertDist_mm + m_dVertErr_um * 0.001 ) ;
          *( pInfo->Attributes() ) += "back=0;" ;
          pMarking->AddFrame( pInfo ) ;
        }
        else if ( m_iViewMode == 7 )
        {
          CTextFrame * pInfo = CreateTextFrameEx( cViewPt , 0x00ff00 , 14 ,
            "dX=%.2fum dY=%.2fum" , m_cLineCrossOffsetToRectCenter_um.real() ,
            m_cLineCrossOffsetToRectCenter_um.imag() ) ;
          *( pInfo->Attributes() ) += "back=0;" ;
          pMarking->AddFrame( pInfo ) ;
        }
        else if ( m_iViewMode == 6 )
        {
          cmplx cViewPt( m_cLastROICent_pix * 0.2 ) ;
          CTextFrame * pInfo = CreateTextFrameEx( cViewPt , 0x00ff00 , 14 ,
            "dX=%.2fum dY=%.2fum" ,
            ( m_cHorCross.real() != 0. ) ? m_dVertLineHorShiftFromLUCorner_um : 0. ,
            m_dHorLineVertShiftFromLUCorner_um ) ;
          *( pInfo->Attributes() ) += "back=0;" ;
          pMarking->AddFrame( pInfo ) ;
        }

        if ( ( m_dTolerance_um != 0. ) && ( m_iViewMode > 6 ) )
        {
          m_bResultOK = ( m_bVScaleOK && m_bHScaleOK && m_bSkewOK ) ;
          cmplx cResultViewPt( m_LastROI.Width() * 0.02 , m_LastROI.Height() * 0.75 ) ;
          CTextFrame * pResult = CreateTextFrameEx( cResultViewPt ,
            m_bResultOK ? 0xffffff : 0x00000 ,
            64 , m_bResultOK ? "  PASS  " : "  FAIL  " ) ;
          pResult->Attributes()->WriteString( "back" , m_bResultOK ? "0x00ff00" : "0x0000FF" ) ;
          pMarking->AddFrame( pResult );
        }
        FormAndSendResultView() ;
        return ;
      }
    }
    else
    {
      pMarking->AddFrame( CreateTextFrameEx( cViewPt , 0xffffff , 14 ,
        "dX=%.2fum\ndY=%.2fum\n"
        "Other FOVs are not measured" ,
        m_dVertLineHorShiftFromLUCorner_um ,
        m_dHorLineVertShiftFromLUCorner_um ) ) ;
      return ;
    }
  }
  pMarking->AddFrame( CreateTextFrame( cViewPt , "0x0000ff" , 14 ,
    NULL , 0 , "Error: Can't measure, adjust system position" ) ) ;
}

void SkewMeter::ProcessRightImage( const CDataFrame * pDataFrame , CContainerFrame * pMarking )
{
  m_bLER_OK = false ;
  cmplx cViewPt( m_cLastROICent_pix * 0.2 ) ;
  if ( ( m_cVertCross.real() == 0. ) || ( m_Intersections.size() < 6 ) )
  {
    pMarking->AddFrame( CreateTextFrame( cViewPt , "0x0000ff" , 14 ,
      NULL , 0 , "Bad position:\nPut Horizontal line inside square" ) ) ;
    m_dHorLineVertShiftFromRUCorner_um = 10000. ;
    return ;
  }

  if ( m_Square.IsFilled() && m_cVertCross.real() )
  {
    double dVertShift_um = ( m_cVertCross.real() - m_Square.m_c1st.real() )
      * m_dScale_um_per_pix ;
    m_dHorLineVertShiftFromRUCorner_um = dVertShift_um ;

    if ( m_cHorCross.real() )
    {
      m_cRightRectCenter_pix = ( m_Square.GetCenter() - m_cLastROICent_pix ) ;
      m_cRightLinesCross_pix = m_cVHLinesCross_pix ;
      m_cRightLineCrossToRectCent_um = m_cLineCrossOffsetToRectCenter_um ;

      double dHorShift = m_dScale_um_per_pix * abs( m_cHorCross - m_Square.m_c1st ) ;
      m_dVertLineHorShiftFromRUCorner_um = dHorShift ;
      m_bLER_OK = true ;
      if ( m_iViewMode > 5 )
      {
        CTextFrame * pInfo = CreateTextFrameEx( cViewPt , 0x00ff00 , 14 ,
          "dX=%.2fum dY=%.2fum" , m_cLineCrossOffsetToRectCenter_um.real() ,
          m_cLineCrossOffsetToRectCenter_um.imag() ) ;
        *( pInfo->Attributes() ) += "back=0;" ;
        pMarking->AddFrame( pInfo ) ;
      }
      return ;
    }
  }

  pMarking->AddFrame( CreateTextFrame( cViewPt , "0x0000ff" , 14 ,
    NULL , 0 , "Error: Can't measure" ) ) ;
  // 10000. is sign, that measurement is not OK
  m_dHorLineVertShiftFromRUCorner_um = 10000. ;
}

void SkewMeter::ProcessBottomImage( const CDataFrame * pDataFrame , CContainerFrame * pMarking )
{
  m_bTEL_OK = false ;
  cmplx cViewPt( m_cLastROICent_pix * 0.2 ) ;
  if ( ( m_cHorCross.real() == 0. ) || ( m_Intersections.size() < 6 ) )
  {
    pMarking->AddFrame( CreateTextFrame( cViewPt , "0x0000ff" , 14 ,
      NULL , 0 , "Bad position:\nPut Vertical line inside square" ) ) ;
    m_dVertLineHorShiftFromLDCorner_um = 10000. ;
    return ;
  }

  if ( m_Square.IsFilled() && m_cHorCross.real() )
  {
    double dHorShift_um = ( m_cHorCross.real() - m_Square.m_c1st.real() ) * m_dScale_um_per_pix ;

    m_dVertLineHorShiftFromLDCorner_um = dHorShift_um ;
    if ( m_cVertCross.real() )
    {
      m_cBottomRectCenter_pix = ( m_Square.GetCenter() - m_cLastROICent_pix ) ;
      m_cBottomLinesCross_pix = m_cVHLinesCross_pix ;
      m_cBottomLineCrossToRectCenter_um = m_cLineCrossOffsetToRectCenter_um ;

      double dVertShift = ( m_cVertCross.imag() - m_Square.m_c1st.imag() )
        * m_dScale_um_per_pix ;
      m_dHorLineVertShiftFromLDCorner_um = dVertShift ;
      m_bTEL_OK = true ;
      if ( m_iViewMode > 5 )
      {
        CTextFrame * pInfo = CreateTextFrameEx( cViewPt , 0x00ff00 , 14 ,
          "dX=%.2fum dY=%.2fum" , m_cLineCrossOffsetToRectCenter_um.real() ,
          m_cLineCrossOffsetToRectCenter_um.imag() ) ;
        *( pInfo->Attributes() ) += "back=0;" ;
        pMarking->AddFrame( pInfo ) ;
      }
      return ;
    }
  }

  pMarking->AddFrame( CreateTextFrame( cViewPt , "0x0000ff" , 14 ,
    NULL , 0 , "Error: Can't measure" ) ) ;
  m_dVertLineHorShiftFromLDCorner_um = 10000. ;
}

int SkewMeter::FindLineCrosses( cmplx cInitialPt_pix ,
  double dDirectionOnVideo_rad , bool bOrtho , CContainerFrame * pOut )
{
#define SIGNAL_MAX_LENGTH 10000

  double pSignal[ SIGNAL_MAX_LENGTH ] ;

  cmplx cDir = -polar( 1. , dDirectionOnVideo_rad ) ;

  cmplx cSearchVector = cInitialPt_pix - m_cLastROICent_pix ;
  int iNSearchSteps = ROUND( 1.8 * abs( cSearchVector ) ) ; // 0.9 of FOV
  cmplx cRightStep = GetOrthoLeftOnVF( cDir ) ;
  cmplx c1stSearchCenter = m_cLastROICent_pix
    + ( 0.9 * ( cSearchVector ) ) ;
  cmplx c1stSearchPt = c1stSearchCenter - cRightStep * m_dSearchWidth_pix * 0.5 ;

   // Get "signal" - values averaged by parallel to edge lines
  double dAverage = GetAverageSignalOnStrip(
    c1stSearchPt , cDir , iNSearchSteps , ROUND( m_dSearchWidth_pix ) ,
    pSignal , SIGNAL_MAX_LENGTH , m_pLastVideoFrame ) ;

  double dMin = DBL_MAX , dMax = -DBL_MAX ;

  GetMinMaxDbl( pSignal , iNSearchSteps , dMin , dMax ) ;
  double dThres = dMin + ( dMax - dMin ) * m_dLineSearchThres ;

  CmplxVector& Falls = ( bOrtho ) ? m_OrthoFalls : m_MainFalls ;
  CmplxVector& Rises = ( bOrtho ) ? m_OrthoRises : m_MainRises ;
  CmplxVector& Centers = ( bOrtho ) ? m_OrthoCenters : m_MainCenters ;
  CmplxVectors& Lines = ( bOrtho ) ? m_OrthoLines : m_MainLines ;
  LinesVector& Regressions = ( bOrtho ) ? m_OrthoLinesRegression : m_MainLinesRegression ;

  Falls.clear() , Rises.clear() , Centers.clear() , Lines.clear() , Regressions.clear() ;
  double * pdIter = pSignal ;
  double * pdEnd = pSignal + iNSearchSteps ;
  while ( pdIter < pdEnd )
  {
    while ( ( *pdIter >= dThres ) && ( ++pdIter < pdEnd ) ) ;
    if ( pdIter >= pdEnd )
      break ;
    double dFallPos = GetThresPosition( *( pdIter - 1 ) , *pdIter , dThres ) ;
    dFallPos += ( pdIter - pSignal - 1 ) ;
    Falls.push_back( c1stSearchCenter + cDir * dFallPos ) ;
    while ( ( *pdIter < dThres ) && ( ++pdIter < pdEnd ) ) ;
    if ( pdIter >= pdEnd )
      break ;
    double dRisePos = GetThresPosition( *( pdIter - 1 ) , *pdIter , dThres ) ;
    dRisePos += ( pdIter - pSignal - 1 ) ;
    Rises.push_back( c1stSearchCenter + cDir * dRisePos ) ;
    Centers.push_back( ( Falls.back() + Rises.back() ) * 0.5 ) ;
  }
  if ( m_iViewMode >= 10 )
  {
    for ( size_t i = 0 ; i < Falls.size() ; i++ )
      pOut->AddFrame( CreatePtFrame( Falls[ i ] , GetHRTickCount() , 0x000000ff ) ) ;
    for ( size_t i = 0 ; i < Rises.size() ; i++ )
      pOut->AddFrame( CreatePtFrame( Rises[ i ] , GetHRTickCount() , 0x0000ff00 ) ) ;
  }

  if ( Centers.size() )
  {
    for ( size_t iLine = 0 ; iLine < Centers.size() ; iLine++ )
    {
      CmplxVector HalfLine , SecondHalfLine ;

      int iRes1 = ( int ) GetContrastLine( Centers[ iLine ] ,
        cRightStep , m_dSearchWidth_pix * 0.5 , HalfLine , m_pLastVideoFrame ) ;

      if ( iRes1 )
      {
        int iRes2 = ( int ) GetContrastLine( Centers[ iLine ] , -cRightStep , m_dSearchWidth_pix * 0.5 ,
          SecondHalfLine , m_pLastVideoFrame ) ;
        if ( iRes2 )
        {
          CmplxVector FullLine ;
          FullLine.resize( HalfLine.size() + SecondHalfLine.size() - 1 ) ;
          cmplx * pFullData = FullLine.data() ;
          cmplx * pDst = pFullData ;
          cmplx * pFromOrig = SecondHalfLine.data() ;
          cmplx * pFrom = pFromOrig + SecondHalfLine.size() - 1 ;
          while ( pFrom > pFromOrig )
            *( pDst++ ) = *( pFrom-- ) ;
          memcpy( pDst , HalfLine.data() , HalfLine.size() * sizeof( cmplx ) ) ;

          if ( m_iViewMode > 4 )
          {
            pOut->AddFrame( CreateFigureFrame( FullLine.data() ,
              ( int ) FullLine.size() , ( DWORD ) ( bOrtho ? 0x0000ffff : 0x0000ff00 ) ) ) ;
          }
          Lines.push_back( FullLine ) ;

          CFRegression Line ;
          Line.AddPtsToRegression( FullLine , 0 , ( int ) FullLine.size() - 1 ) ;
          Line.Calculate() ;
          CLine2d NewLine = Line.GetCLine2d() ;
          LinesVector Vect ;

          Regressions.push_back( NewLine ) ;
        }
        else
          Centers.clear() ;
      }
      else
        Centers.clear() ;
    }
  }
  return ( int ) Centers.size() ;
}

int SkewMeter::FillTetragon( CDRect& IntersectionsArea , CmplxVector& Points , Tetragon& Result )
{
  cmplx cLT( m_IntersectionsArea.left , m_IntersectionsArea.top ) ;
  cmplx cRT( m_IntersectionsArea.right , m_IntersectionsArea.top ) ;
  cmplx cRB( m_IntersectionsArea.right , m_IntersectionsArea.bottom ) ;
  cmplx cLB( m_IntersectionsArea.left , m_IntersectionsArea.bottom ) ;
  double dMaxLT = 0. , dMaxRT = 0. , dMaxRB = 0. , dMaxLB = 0. ;
  for ( size_t i = 0 ; i < Points.size() ; i++ )
  {
    cmplx cCross = Points[ i ] ;

    double dDistToLT = abs( cLT - cCross ) ;
    if ( dDistToLT > dMaxLT )
    {
      dMaxLT = dDistToLT ;
      Result.m_c3rd = cCross ;
    }
    double dDistToRT = abs( cRT - cCross ) ;
    if ( dDistToRT > dMaxRT )
    {
      dMaxRT = dDistToRT ;
      Result.m_c4th = cCross ;
    }
    double dDistToRB = abs( cRB - cCross ) ;
    if ( dDistToRB > dMaxRB )
    {
      dMaxRB = dDistToRB ;
      Result.m_c1st = cCross ;
    }
    double dDistToLB = abs( cLB - cCross ) ;
    if ( dDistToLB > dMaxLB )
    {
      dMaxLB = dDistToLB ;
      Result.m_c2nd = cCross ;
    }
  }
  return 1;
}

int SkewMeter::SimplifiedProcessing( CContainerFrame * pMarking )
{
  double dClearences = ( 100. - m_dSquareSearchArea_perc ) / 200. ;
  CRect HorProfROI( ROUND( m_LastROI.right * dClearences ) ,
    ROUND( m_LastROI.bottom * 0.45 ) ,
    ROUND( m_LastROI.right * ( 1. - ( 2. * dClearences ) ) ) ,
    ROUND( m_LastROI.bottom * 0.1 ) ) ;
  CRect VertProfROI( ROUND( m_LastROI.right * 0.45 ) ,
    ROUND( m_LastROI.bottom * dClearences ) ,
    ROUND( m_LastROI.right * 0.1 ) ,
    ROUND( m_LastROI.bottom * ( 1. - ( 2. * dClearences ) ) ) ) ;

  Profile HorProfX , HorProfY , VertProfX , VertProfY ;

  calc_profiles( m_pLastVideoFrame , &HorProfX , &HorProfY , &HorProfROI ) ;
  HorProfX.m_iProfOrigin = HorProfROI.left ;
  HorProfX.m_iActiveLen = HorProfROI.right ; // right holds width, not right edge
  calc_profiles( m_pLastVideoFrame , &VertProfX , &VertProfY , &VertProfROI ) ;
  VertProfY.m_iProfOrigin = VertProfROI.top ;
  VertProfY.m_iActiveLen = VertProfROI.bottom ; // bottom holds height, not bottom edge
  double dCenterYForHorProfile = HorProfROI.top + HorProfROI.bottom * 0.5 ;
  double dCenterXForVertProfile = VertProfROI.left + VertProfROI.right * 0.5 ;


  CmplxVector VertBases , HorBases ;
  DoubleVector FoundVert , FoundHor ;
  Profile HorProfXNorm( HorProfX ) , VertProfYNorm( VertProfY ) ;
  // find vertical lines
  int iNVertLines = ProcessProfile( HorProfX , FoundVert , HorProfXNorm ) ;
  // find horizontal lines
  int iNHorLines = ProcessProfile( VertProfY , FoundHor , VertProfYNorm ) ;
  double dHorAmpl = HorProfX.m_dMaxValue - HorProfX.m_dMinValue ;
  double dVertAmpl = VertProfY.m_dMaxValue - VertProfY.m_dMinValue ;
  if ( ( 12 <= m_iViewMode ) && ( m_iViewMode <= 13 ) )
  {
    CFigureFrame * pHorProfile = CreateHGraphForDraw(
      ( m_iViewMode == 12 ) ? HorProfX : HorProfXNorm , HorProfROI ) ;
    pMarking->AddFrame( pHorProfile ) ;
    CFigureFrame * pVertProfile = CreateVGraphForDraw(
      ( m_iViewMode == 12 ) ? VertProfY : VertProfYNorm , VertProfROI ) ;
    pMarking->AddFrame( pVertProfile ) ;

    HorProfROI.right += HorProfROI.left ;
    HorProfROI.bottom += HorProfROI.top ;
    pMarking->AddFrame( CreateFigureFrameEx( HorProfROI , 0x000000ff ) ) ;
    VertProfROI.right += VertProfROI.left ;
    VertProfROI.bottom += VertProfROI.top ;
    pMarking->AddFrame( CreateFigureFrameEx( VertProfROI , 0x00000ff ) ) ;
  }

  m_HorLines.clear() ;
  m_VertLines.clear() ;
  m_HorLinesRegression.clear() ;
  m_VertLinesRegression.clear() ;
  m_VWidths.clear() ;
  m_HWidths.clear() ;
  for ( int i = 0 ; i < iNVertLines ; i++ )
  {
    cmplx cLinePos( FoundVert[ i ] , dCenterYForHorProfile ) ;
    VertBases.push_back( cLinePos ) ;
    if ( m_iViewMode >= 7 )
      pMarking->AddFrame( CreatePtFrameEx( cLinePos , 0x0000ff00 ) ) ;
    CmplxVector PtsOnLine ;
    double dWidth = 0. ;
    int iRes = ( int ) GetVerticalContrastLine( cLinePos ,
      ROUND( m_dMaxLineWidth * 1.3 / 2. ) , PtsOnLine ,
      m_dLineSearchThres , ROUND( m_dMinimalContrast * dHorAmpl ) ,
      m_pLastVideoFrame , dWidth , &FoundHor ) ;
    if ( PtsOnLine.size() > ( m_cLastROICent_pix.imag() * 0.4 ) )
    {
      if ( iRes )
      {
        if ( m_iViewMode >= 5 )
          pMarking->AddFrame( CreateFigureFrameEx( PtsOnLine.data() , ( int ) PtsOnLine.size() , 0x0000ffff ) ) ;

        if ( PtsOnLine.size() > 15 )
        {
          CFRegression Line ;
          Line.AddPtsToRegression( PtsOnLine , 5 , ( int ) PtsOnLine.size() - 5 ) ;
          Line.Calculate() ;
          CLine2d NewLine = Line.GetCLine2d() ;

          m_VertLinesRegression.push_back( NewLine ) ;
          m_VWidths.push_back( dWidth ) ;
        }
      }
    }
  }
  for ( int i = 0 ; i < iNHorLines ; i++ )
  {
    cmplx cLinePos( dCenterXForVertProfile , FoundHor[ i ] ) ;
    HorBases.push_back( cLinePos ) ;
    if ( m_iViewMode >= 7 )
      pMarking->AddFrame( CreatePtFrameEx( cLinePos , 0x0000ff00 ) ) ;
    CmplxVector PtsOnLine ;
    double dWidth = 0. ;
    int iRes = ( int ) GetHorizontalContrastLine( cLinePos ,
      ROUND( m_dMaxLineWidth * 1.3 / 2. ) , PtsOnLine ,
      m_dLineSearchThres , ROUND( m_dMinimalContrast * dVertAmpl ) ,
      m_pLastVideoFrame , dWidth , &FoundVert ) ;
    if ( iRes )
    {
      if ( m_iViewMode >= 5 )
        pMarking->AddFrame( CreateFigureFrameEx( PtsOnLine.data() , ( int ) PtsOnLine.size() , 0x0000ffff ) ) ;

      if ( PtsOnLine.size() > 15 )
      {
        CFRegression Line ;
        Line.AddPtsToRegression( PtsOnLine , 5 , ( int ) PtsOnLine.size() - 5 ) ;
        Line.Calculate() ;
        CLine2d NewLine = Line.GetCLine2d() ;

        m_HorLinesRegression.push_back( NewLine ) ;
        m_HWidths.push_back( dWidth ) ;
      }
    }
  }

      // Find and draw cross points
  m_Intersections.clear() ;
  m_cCrossesAverage = cmplx( 0. , 0. ) ;
  m_IntersectionsArea.left = DBL_MAX ;
  m_IntersectionsArea.right = -DBL_MAX ;
  m_IntersectionsArea.top = DBL_MAX ;
  m_IntersectionsArea.bottom = -DBL_MAX ;

  for ( size_t i = 0 ; i < m_HorLinesRegression.size() ; i++ )
  {
    for ( size_t j = 0 ; j < m_VertLinesRegression.size() ; j++ )
    {
      cmplx Cross ;
      if ( m_HorLinesRegression[ i ].intersect( m_VertLinesRegression[ j ] , Cross ) )
      {
        m_Intersections.push_back( Cross ) ;
        if ( m_iViewMode >= 3 )
          pMarking->AddFrame( CreatePtFrame( Cross , "color=0x0000ff; Sz=5;" ) ) ;
        m_cCrossesAverage += Cross ;
        m_IntersectionsArea.Union( CmplxToCDPoint( Cross ) ) ;
        if ( ( i == 1 ) && ( j == 1 ) )
        {
          m_cVHLinesCross_pix = Cross ;
        }
      }
    }
  }
  return 0 ;
}

int SkewMeter::ProcessProfile( Profile& Prof , DoubleVector& Results
  , Profile& OutProfile )
{
  double dFoundPos ;
  int iSearchIndex = Prof.m_iProfOrigin ;
  double dFoundWidth = 0. ;
  m_dMaxLineWidth = m_dSearchWidth_pix ;

  ProfileLocalNormalize( Prof , OutProfile ,
    m_dMinimalContrast / 2. , ROUND( m_dSearchWidth_pix * 1.3 ) ,
    ( GetCompression( m_pLastVideoFrame ) == BI_Y16 ) ? 65535. : 255. ) ;

  while ( ( dFoundPos = FindProfileCrossesPosition( OutProfile ,
    iSearchIndex , m_dLineSearchThres , dFoundWidth ) ) != 0. )
  {
    if ( m_dMinLineWidth <= dFoundWidth && dFoundWidth <= m_dMaxLineWidth )
      Results.push_back( dFoundPos ) ;
  }
  return ( int ) Results.size() ;
}


int SkewMeter::CalcDistsAndViewRectData( CContainerFrame * pMarking )
{
  m_cUpperVect = m_Square.IsFilled() ? m_Square.m_c2nd - m_Square.m_c1st : 0. ;
  m_cRigthVect = m_Square.IsFilled() ? m_Square.m_c3rd - m_Square.m_c2nd : 0. ;
  m_cBottomVect = m_Square.IsFilled() ? m_Square.m_c4th - m_Square.m_c3rd : 0. ;
  m_cLeftVect = m_Square.IsFilled() ? m_Square.m_c1st - m_Square.m_c4th : 0. ;
  m_cLTtoRBVect = m_Square.IsFilled() ? m_Square.m_c3rd - m_Square.m_c1st : 0. ;
  m_cRTtoLBVect = m_Square.IsFilled() ? m_Square.m_c4th - m_Square.m_c2nd : 0. ;
  if ( ( m_iViewMode >= 10 ) && ( m_HWidths.size() == 3 ) && ( m_VWidths.size() == 3 ) )
  {
    cmplx cViewPt = m_Square.m_c1st + ( m_cUpperVect / 2. ) + cmplx( 0. , -100. ) ;
    pMarking->AddFrame( CreateTextFrameEx( cViewPt , 0x00ffff , 12 ,
      "L=%.2fum W=%.2fum" , abs( m_cUpperVect ) * m_dScale_um_per_pix ,
      m_HWidths[ 0 ] * m_dScale_um_per_pix ) ) ;
    cViewPt = m_Square.m_c2nd + ( m_cRigthVect / 2. ) + cmplx( 10. , 0. ) ;
    pMarking->AddFrame( CreateTextFrameEx( cViewPt , 0x00ffff , 12 ,
      "L=%.2f\nW=%.2fum" , abs( m_cRigthVect ) * m_dScale_um_per_pix ,
      m_VWidths[ 2 ] * m_dScale_um_per_pix ) ) ;
    cViewPt = m_Square.m_c3rd + ( m_cBottomVect / 2. ) + cmplx( 0. , 10. ) ;
    pMarking->AddFrame( CreateTextFrameEx( cViewPt , 0x00ffff , 12 ,
      "L=%.2f\nW=%.2fum" , abs( m_cBottomVect ) * m_dScale_um_per_pix ,
      m_HWidths[ 2 ] * m_dScale_um_per_pix ) ) ;
    cViewPt = m_Square.m_c4th + ( m_cLeftVect / 2. ) + cmplx( -150. , 0. ) ;
    pMarking->AddFrame( CreateTextFrameEx( cViewPt , 0x00ffff , 12 ,
      "L=%.2f\nW=%.2fum" , abs( m_cLeftVect ) * m_dScale_um_per_pix ,
      m_VWidths[ 0 ] * m_dScale_um_per_pix ) ) ;

    cViewPt = m_cLastROICent_pix * 1.8 + cmplx( -4. , -30. );
    pMarking->AddFrame( CreateTextFrameEx( cViewPt , 0x000000 , 12 ,
      "Wh=%.2f\nWv=%.2fum" , m_HWidths[ 1 ] * m_dScale_um_per_pix ,
      m_VWidths[ 1 ] * m_dScale_um_per_pix ) ) ;
    if ( m_iViewMode >= 11 )
    {
      pMarking->AddFrame( CreateLineFrameEx( m_Square.m_c1st , m_Square.m_c3rd , 0x800080 ) ) ;
      pMarking->AddFrame( CreateLineFrameEx( m_Square.m_c2nd , m_Square.m_c4th , 0xff0000 ) ) ;
      cViewPt = m_Square.m_c1st + ( m_cLTtoRBVect * .250 ) + cmplx( 20. , 0. ) ;
      pMarking->AddFrame( CreateTextFrameEx( cViewPt , 0x800080 , 12 ,
        "%.2f" , abs( m_cLTtoRBVect ) * m_dScale_um_per_pix ) ) ;
      cViewPt = m_Square.m_c2nd + ( m_cRTtoLBVect * 0.750 ) + cmplx( 10. , 0. ) ;
      pMarking->AddFrame( CreateTextFrameEx( cViewPt , 0xff0000 , 12 ,
        "%.2f" , abs( m_cRTtoLBVect ) * m_dScale_um_per_pix ) ) ;
    }
  }
  return 0;
}


int SkewMeter::FormAndSendResultView()
{
  pTVFrame pTv = makeNewY8Frame( m_LastROI.right , m_LastROI.bottom ) ;
  CVideoFrame * pResultView = CVideoFrame::Create( pTv ) ;
  m_cLastROICent_pix = cmplx( m_LastROI.right / 2. , m_LastROI.bottom / 2. ) ;
  int iWidth = GetWidth( pResultView ) ;
  int iHeight = GetHeight( pResultView ) ;
  memset( GetData( pResultView ) , 128 , iWidth * iHeight ) ;

  CContainerFrame * pOut = CContainerFrame::Create() ;
  pOut->AddFrame( pResultView ) ;
  if ( m_bBatteriesAreOK
    || m_SystemPowerStatus.ACLineStatus
    || ( m_iCaptureMode == CM_OneProcess ) ) // Problem with battery, but single frame mode will work
  {
    if ( ( fabs( m_dHorErr_um ) > 3500. )
      || ( fabs( m_dVertErr_um ) > 3500. )
      || ( fabs( m_dSkewByCent_um ) > 3500. )
      || !m_bLER_OK || !m_bTEL_OK )
    { // too big variation
      cmplx cBatteryMsgPt( m_cLastROICent_pix * 0.7 ) ;
      pOut->AddFrame( CreateTextFrameEx( cBatteryMsgPt , 0x8080ff , 14 ,
        "Bad Measurement result\nDo adjust Skew Meter position\n%s" ,
        m_bLER_OK ? ( m_bTEL_OK ? "" : "May be Trail Edge Camera is not working" )
        : "May be Leading Edge Right Camera is not working" ) ) ;
    }
    else
    {
      if ( m_iCaptureMode >= CM_LiveProcess )
      {
        cmplx cSkewCent( iWidth * 0.2 , iHeight * 0.12 ) ;
        cmplx cIdealUpperEdgeVect( iWidth * 0.6 , 0. ) ;
        cmplx cIdealLeftSideVect( 0. , iHeight * 0.65 ) ;
        cmplx cPageCorners[] = {
          cSkewCent ,                                             // LT
          cSkewCent + cIdealUpperEdgeVect ,                       // RT
          cSkewCent + cIdealUpperEdgeVect + cIdealLeftSideVect ,  // RB
          cSkewCent + cIdealLeftSideVect                          // LB
        } ;
        CRectFrame * pPage = CRectFrame::Create() ;
        pPage->left = ROUND( cSkewCent.real() ) ;
        pPage->right = ROUND( iWidth * 0.8 ) ;
        pPage->top = ROUND( cSkewCent.imag() ) ;
        pPage->bottom = ROUND( cSkewCent.imag() + cIdealLeftSideVect.imag() ) ;

        cSkewCent = cmplx( pPage->left - 0.5 , pPage->top - 0.5 ) ;
        pPage->Attributes()->Format(
          _T( "color=0X%X; thickness=%d; style=%d; back=0X%X;" ) ,
          m_BorderResultColor , m_iPageBorderThickness , m_PageBorderStyle ,
          m_PageColor ) ;
        pPage->SetLabel( "Page" ) ;
        pOut->AddFrame( pPage ) ;
#define RelativeTolerance_pix (0.1)

        double dYTol_pix = RelativeTolerance_pix * iHeight ; // 1/10 of frame height
        double dXTol_pix = RelativeTolerance_pix * iWidth ;
        cmplx cSkewRangeLeftUpper( cPageCorners[ 1 ]
          + cmplx( -dXTol_pix , -dYTol_pix ) ) ;
        cmplx cSkewRangeRightUpper( cSkewRangeLeftUpper
          + cmplx( 2. * dXTol_pix , 0. ) ) ;
        cmplx cSkewRangeLeftLower( cSkewRangeLeftUpper
          + cmplx( 0. , 2. * dYTol_pix ) ) ;
        cmplx cSkewRangeRightLower( cSkewRangeRightUpper
          + cmplx( 0. , 2. * dYTol_pix ) ) ;

        CFigureFrame * pRangeView = CreateLineFrameEx( cSkewRangeLeftUpper , cSkewRangeRightUpper , 0xff00ff ) ;
        pOut->AddFrame( pRangeView ) ;
        pRangeView = CreateLineFrameEx( cSkewRangeLeftLower , cSkewRangeRightLower , 0xff00ff ) ;
        pOut->AddFrame( pRangeView ) ;

  //       double dSkewRangeViewLen = iWidth * 0.7 ;
  //       double dAngleRange_rad = atan2( dYTol_pix , iWidth * 0.6 ) ;
  //       double dSkewViewAngle = dYTol_pix * m_dCorrectedSkewByCent_um / m_dTolerance_um ;
  //       cmplx cRangePlusVect = polar( dSkewRangeViewLen , -dAngleRange_rad ) ;
  //       cmplx cRangeMinusVect = conj( cRangePlusVect ) ;
  //       cmplx cUpperRangePt = cSkewCent + cRangePlusVect ;
  //       cmplx cLowerRangePt = cSkewCent + cRangeMinusVect ;
  //       CFigureFrame * pRangeView = CreateLineFrameEx( cUpperRangePt , cSkewCent , 0xff00ff ) ;
  //       pRangeView->AddPoint( CmplxToCDPoint( cLowerRangePt ) ) ;
  //       pOut->AddFrame( pRangeView ) ;


        cmplx cHorRangeLeftUpper( cPageCorners[ 2 ]
          + cmplx( -dXTol_pix , -dYTol_pix ) ) ;
        cmplx cHorRangeRightUpper( cHorRangeLeftUpper
          + cmplx( 2. * dXTol_pix , 0. ) ) ;
        cmplx cHorRangeLeftLower( cHorRangeLeftUpper
          + cmplx( 0. , 2. * dYTol_pix ) ) ;
        cmplx cHorRangeRightLower( cHorRangeRightUpper
          + cmplx( 0. , 2. * dYTol_pix ) ) ;


        pRangeView = CreateLineFrameEx( cHorRangeLeftUpper , cHorRangeLeftLower , 0xff00ff ) ;
        pOut->AddFrame( pRangeView ) ;
        pRangeView = CreateLineFrameEx( cHorRangeRightUpper , cHorRangeRightLower , 0xff00ff ) ;
        pOut->AddFrame( pRangeView ) ;

        cmplx cVertRangeLeftUpper( cPageCorners[ 3 ]
          + cmplx( -dXTol_pix , -dYTol_pix ) ) ;
        cmplx cVertRangeRightUpper( cVertRangeLeftUpper
          + cmplx( 2.0 * dXTol_pix , 0. ) ) ;
        cmplx cVertRangeLeftLower( cVertRangeLeftUpper
          + cmplx( 0. , 2. * dYTol_pix ) ) ;
        cmplx cVertRangeRightLower( cVertRangeRightUpper
          + cmplx( 0. , 2. * dYTol_pix ) ) ;
        pRangeView = CreateLineFrameEx( cVertRangeLeftUpper , cVertRangeRightUpper , 0xff00ff ) ;
        pOut->AddFrame( pRangeView ) ;
        pRangeView = CreateLineFrameEx( cVertRangeLeftLower , cVertRangeRightLower , 0xff00ff ) ;
        pOut->AddFrame( pRangeView ) ;


        double dSkewViewErr_um = ( fabs( m_dCorrectedSkewByCent_um ) > ( m_dTolerance_um * 1.1 ) ) ?
          sign( m_dCorrectedSkewByCent_um ) * m_dTolerance_um * 1.1 : m_dCorrectedSkewByCent_um ;
        double dHorViewErr_um = ( fabs( m_dHorErr_um ) > ( m_dTolerance_um * 1.1 ) ) ?
          sign( m_dHorErr_um ) * m_dTolerance_um * 1.1 : m_dHorErr_um ;
        double dVertViewErr_um = ( fabs( m_dVertErr_um ) > ( m_dTolerance_um * 1.1 ) ) ?
          sign( m_dVertErr_um ) * m_dTolerance_um * 1.1 : m_dVertErr_um ;

        cmplx cRealHorSizeVect = cIdealUpperEdgeVect
          + ( dXTol_pix * dHorViewErr_um / m_dTolerance_um ) ;
        cmplx cRealVertSide = cIdealLeftSideVect
          + cmplx( 0. , dYTol_pix * dVertViewErr_um / m_dTolerance_um ) ;
      //   cmplx cRealUpperEdgeVect = cRealHorSizeVect
      //     + cmplx( 0 , -dYTol_pix * m_dSkew_um / m_dTolerance_um ) ;

        cmplx cRealUpperEdgeVect = cRealHorSizeVect
          + cmplx( 0 , -dYTol_pix * dSkewViewErr_um / m_dTolerance_um ) ;
        cmplx cRTPt = cSkewCent + cRealUpperEdgeVect /*+ ( dXTol_pix * dHorViewErr_um / m_dTolerance_um )*/ ;

        CFigureFrame * pUpperView = CreateLineFrameEx( cSkewCent , cRTPt ,
          m_bSkewOK ? 0x00ff00 : 0x8080ff , 3 ) ;
        pOut->AddFrame( pUpperView ) ;


        CFigureFrame * pRealPageView = CreateLineFrameEx( cSkewCent , cRTPt ,
          0xff0000 ) ;
        cmplx cRBPt = cRTPt + cRealVertSide ;
        cmplx cLBPt( cSkewCent + cRealVertSide ) ;
        pRealPageView->AddPoint( CmplxToCDPoint( cRBPt ) ) ;
        pRealPageView->AddPoint( CmplxToCDPoint( cLBPt ) ) ;
        pRealPageView->AddPoint( CmplxToCDPoint( cSkewCent ) ) ;
        pRealPageView->Attributes()->WriteInt( "thickness" , 3 ) ;
        pOut->AddFrame( pRealPageView ) ;

        cmplx cSkewMarkUpper( cPageCorners[ 1 ].real() + 50. , cRTPt.imag() ) ;
        CFigureFrame * pSkewMark = CreateLineFrameEx(
          cSkewMarkUpper ,
          cPageCorners[ 1 ] + 50. , m_bSkewOK ? 0x00ff00 : 0x8080ff , 3 ) ;
        pOut->AddFrame( pSkewMark ) ;

        cmplx cSkewTextView( cPageCorners[ 1 ] + 55. ) ;
        CTextFrame * pSkewVal = CreateTextFrameEx(
          cSkewTextView ,
      //     m_bSkewOK ? 0x80ff80 : 0x8080ff , 12 , "Skew=%.2f(%.2f)um\n%s" ,
          m_bSkewOK ? 0xa0ffa0 : 0xa0a0ff , 12 , "Skew\n%.2f um\n%s" ,
          m_dCorrectedSkewByCent_um ,
          ( m_dCorrectedSkewByCent_um > 0 ) ? "CCW" : "CW" ) ;
        pOut->AddFrame( pSkewVal ) ;

        cSkewTextView += cmplx( 0. , 200. ) ;
        if ( m_iViewMode >= 10 )
        {
          pSkewVal = CreateTextFrameEx(
            cSkewTextView , 0xffffff , 12 , "Corner\n Skew\n%.2f um\n%s" ,
            m_dCorrectedSkew_um ,/* m_dSkew_um ,*/
            ( m_dCorrectedSkewByCent_um > 0 ) ? "CCW" : "CW" ) ;
          pOut->AddFrame( pSkewVal ) ;
        }

        cmplx cHSizeErrMarkEx( cRBPt.real() , cPageCorners[ 2 ].imag() + 50. ) ;
        cmplx cHSizeErrMark( cPageCorners[ 2 ].real() , cHSizeErrMarkEx.imag() ) ;
        CFigureFrame * pHSizeMark = CreateLineFrameEx(
          cHSizeErrMarkEx , cHSizeErrMark ,
          m_bHScaleOK ? 0xa0ffa0 : 0x8080ff , 3 ) ;
        pOut->AddFrame( pHSizeMark ) ;

        CTextFrame * pHSizeVal = CreateTextFrameEx(
          cHSizeErrMark + cmplx( -100. , 20. ) ,
          m_bHScaleOK ? 0x80ff80 : 0xa0a0ff , 12 , "HSize Err=%.2fum" , m_dHorErr_um ) ;
        pOut->AddFrame( pHSizeVal ) ;

        cmplx cVSizeErrMarkEx( cLBPt - 20. ) ;
        cmplx cVSizeErrMark( cPageCorners[ 3 ] - 20. ) ;
        CFigureFrame * pVSizeMark = CreateLineFrameEx(
          cVSizeErrMarkEx , cVSizeErrMark ,
          m_bVScaleOK ? 0xa0ffa0 : 0x0000ff , 3 ) ;
        pOut->AddFrame( pVSizeMark ) ;

        cmplx cVErrViewPt( cVSizeErrMark - 200. ) ;
        CTextFrame * pVSizeVal = CreateTextFrameEx(
          cVErrViewPt ,
          m_bVScaleOK ? 0xa0ffa0 : 0xa0a0ff , 12 , "VSize Err\n%.2fum" , m_dVertErr_um ) ;
        pOut->AddFrame( pVSizeVal ) ;

        // Form results summary
        double dHorDist_mm = m_dBaseHorDist_mm + m_dHorErr_um * 0.001 ;
  //         ( m_cRightLineCrossToRectCent_um.real() - m_cLeftTopLineCrossToRectCent_um.real() ) * 0.001
  //         + m_dHorDistBetweenSquares_mm ;
        double dVertDist_mm = m_dBaseVertDist_mm + m_dVertErr_um * 0.001 ;
  //         ( m_cBottomLineCrossToRectCenter_um.imag() - m_cLeftTopLineCrossToRectCent_um.imag() ) * 0.001
  //         + m_dVertDistBetweenSquares_mm ;
        double dSkewByCent_mrad = m_dCorrectedSkewByCent_um / dHorDist_mm ;
        double m_dSkewByCent_deg = RadToDeg( dSkewByCent_mrad * 0.001 );
        cmplx cSummaryPt( cPageCorners[ 0 ] + cmplx( 50. , 50. ) ) ;
        CTextFrame * pSummary = CreateTextFrameEx( cSummaryPt ,
          /*m_bResultOK ? 0x00ff00 : 0x0000ff*/0x00ffff , 16 ,
          "Skew            %.2fmm %s\n"
          "Width error   %.2fmm\n"
          "Height error  %.2fmm\n"
          , m_dCorrectedSkewByCent_um / 1000.
          , ( m_dCorrectedSkewByCent_um > 0 ) ? "CCW" : "CW"
          , m_dHorErr_um / 1000. , m_dVertErr_um / 1000.
        ) ;
        *( pSummary->Attributes() ) += "back=0;" ;

        pOut->AddFrame( pSummary ) ;

        cSummaryPt += cmplx( 0. , 260. ) ;
        pSummary = CreateTextFrameEx( cSummaryPt ,
          /*m_bResultOK ? 0x00ff00 : 0x0000ff*/0x00ffff , 12 ,
          "Skew Angle       %.3fdeg (%.2fmrad)\n"
          "Printed Width    %.2fmm\n"
          "Printed Height   %.2fmm\n"
          "Printed W=%.2fmm H=%.2fmm"
          , m_dSkewByCent_deg , dSkewByCent_mrad
          , dHorDist_mm , dVertDist_mm
          , m_dBaseHorDist_mm , m_dBaseVertDist_mm
        ) ;
        *( pSummary->Attributes() ) += "back=0;" ;
        pOut->AddFrame( pSummary ) ;

        if ( m_iRecordPeriod_sec )
        {
          FXString AddInfo ;
          double dLeftVertDistFromCornerToHLine_pix =
            m_Intersections[ 3 ].imag() - m_Intersections[ 0 ].imag() ;
          double dLeftDist_um = dLeftVertDistFromCornerToHLine_pix * m_dScale_um_per_pix ;
          double dRightVertDistFromCornerToHLine_pix =
            m_Intersections[ 5 ].imag() - m_Intersections[ 2 ].imag() ;
          double dRightDist_um = dRightVertDistFromCornerToHLine_pix * m_dScale_um_per_pix ;
          double dDistFromRectTopToHorLine_um =
            0.5 * ( dLeftDist_um + dRightDist_um ) ;
          FXString Add ;
          if ( !m_LogFileName.IsEmpty() )
          {
            double dSkewDelta_um = m_dCorrectedSkewByCent_um - m_dPrevSkew_um ;
            if ( ( m_dSkewThresForLog_um != 0. )
            && ( fabs( dSkewDelta_um ) > m_dSkewThresForLog_um ) )
            {
              AddInfo.Format( ",dSkew=%.2f" , dSkewDelta_um ) ;
            }
            m_dPrevSkew_um = m_dCorrectedSkewByCent_um ;
            double dHErrorDelta_um = m_dHorErr_um - m_dPrevHError_um ;
            if ( ( m_dSizeThresForLog_um != 0. )
              && ( fabs( dHErrorDelta_um ) > m_dSizeThresForLog_um ) )
            {
              Add.Format( ",dHErr=%.2f" , dHErrorDelta_um ) ;
              AddInfo += Add ;
            }
            m_dPrevHError_um = m_dHorErr_um ;
            double dVErrorDelta_um = m_dVertErr_um - m_dPrevVError_um ;
            if ( ( m_dSizeThresForLog_um != 0. )
              && ( fabs( dVErrorDelta_um ) > m_dSizeThresForLog_um ) )
            {
              Add.Format( ",dVErr=%.2f" , dHErrorDelta_um ) ;
              AddInfo += Add ;
            }
            m_dPrevVError_um = m_dVertErr_um ;
            double dHLineDistDelta_um = dDistFromRectTopToHorLine_um - m_dPrevHLineDist_um ;
            if ( ( m_dHLineDistThresForLog_um != 0. )
              && ( fabs( dHLineDistDelta_um ) > m_dHLineDistThresForLog_um ) )
            {
              Add.Format( ",dHLinePos=%.2f" , dHLineDistDelta_um ) ;
              AddInfo += Add ;
            }
            m_dPrevHLineDist_um = dDistFromRectTopToHorLine_um ;
          }
          if ( !AddInfo.IsEmpty() 
            || ( ( GetHRTickCount() - m_dLastLoggedTime_ms ) > ( m_iRecordPeriod_sec * 1000) ) )
          {
            if ( m_LogFileName.IsEmpty() )
            {
              m_LogFileName = "C:\\SkewMeterLog" ;
              int iRes = SHCreateDirectoryExA( NULL , m_LogFileName , NULL ) ;
              if ( ( iRes != ERROR_SUCCESS ) && ( iRes != ERROR_ALREADY_EXISTS ) )
              {
                ASSERT( 0 ) ;
                m_LogFileName.Empty() ;
              }
              else
              {
                m_LogFileName += ( LPCTSTR ) GetTimeStamp( "\\Log_" , ".csv" ) ;
                ifstream myfile ;
                myfile.open( ( LPCTSTR ) m_LogFileName ,
                  std::ifstream::binary | std::ifstream::in );
                if ( myfile.is_open() )
                  myfile.close() ;
                else
                {
                  ofstream myfile;
                  myfile.open( ( LPCTSTR ) m_LogFileName ,
                    std::ofstream::binary | std::ofstream::app );
                  if ( myfile.is_open() )
                  {
                    // Do caption
                    myfile << "   Time Stamp          , Skew um , HErr um , VErr um ,HlineToUp_um\n" ;
                    myfile.close() ;
                    m_dPrevSkew_um = m_dCorrectedSkewByCent_um ;
                    m_dPrevHError_um = m_dHorErr_um ;
                    m_dPrevVError_um = m_dVertErr_um ;
                    m_dPrevHLineDist_um = dDistFromRectTopToHorLine_um ;
                    SEND_GADGET_INFO( "Log file %s is opened" , ( LPCTSTR ) m_LogFileName ) ;
                  }
                }
              }
            }
            if ( !m_LogFileName.IsEmpty() )
            {
              ofstream myfile;
              myfile.open( ( LPCTSTR ) m_LogFileName ,
                std::ofstream::binary | std::ofstream::app );
              if ( myfile.is_open() )
              {
                FXString ForLog ;
                ForLog.Format( "%s, %8.2f, %8.2f, %8.2f, %8.2f%s\n" ,
                  ( LPCTSTR ) GetTimeAsString_ms() ,
                  m_dCorrectedSkewByCent_um , m_dHorErr_um , m_dVertErr_um ,
                  dDistFromRectTopToHorLine_um , (LPCTSTR)AddInfo ) ;
                myfile.write( ( LPCTSTR ) ForLog , ForLog.GetLength() ) ;
                myfile.close() ;
                m_dLastLoggedTime_ms = GetHRTickCount() ;
                cmplx cViewPt( cSkewCent.real() , iHeight * 0.05 ) ;
                pOut->AddFrame( CreateTextFrameEx( cViewPt , ForLog , 
                  AddInfo.IsEmpty() ? 0x80ff80 : 0x8080ff , 14 ) ) ;
              }
            }
          }
          else if ( m_iViewMode == 9 )
          {
            cmplx cViewPt( m_cLastROICent_pix.real() * 1.7 , m_cLastROICent_pix.real() * 0.4 ) ;
            pOut->AddFrame( CreateTextFrameEx( cViewPt , 0x000ffff , 12 ,
              "Log Par.\nPer=%ds\nThr.Sk.=%.2f\n"
              "Thr.Sz=%.2f\nThr.V=%.2f" ,
              m_iRecordPeriod_sec ,
              m_dSkewThresForLog_um ,
              m_dSizeThresForLog_um ,
              m_dHLineDistThresForLog_um ) ) ;
            ;
            pOut->AddFrame( CreateTextFrameEx(
              cmplx( cSkewCent.real() , iHeight * 0.05 ) ,
              AddInfo , 14 ) ) ;
          }

        }
        else
          m_LogFileName.Empty() ;

        m_bLER_OK = m_bTEL_OK = false ;
      }
    }
  }
  else
  {
    cmplx cBatteryMsgPt( m_cLastROICent_pix * 0.7 ) ;
    FXString Msg( "Not enough battery charge or info\nConnect external power\n"
      "Batt=%d AC=%d PowStat=0x%04X\n" ) ;
    Msg += m_PowerStatusAsString ;
    pOut->AddFrame( CreateTextFrameEx( cBatteryMsgPt , 0x8080ff , 14 , Msg ,
      m_bBatteriesAreOK ? 1 : 0 , m_SystemPowerStatus.ACLineStatus ,
      m_PowerStatus ) ) ;
  }

  if ( m_pLastResultViewWithoutPower )
    m_pLastResultViewWithoutPower->Release() ;

  m_pLastResultViewWithoutPower = ( CContainerFrame* ) ( pOut->Copy() ) ;
  m_dLastResultViewTime = GetHRTickCount() ;

  AddPowerStatusAndControls( pOut ) ;

  return ( PutFrame( GetOutputConnector( OC_ResultView ) , pOut ) != false ) ;
}

double SkewMeter::FindProfileCrossesPosition( Profile& Prof , // profile data
  int& iStartIndex ,   // the first point for line search
  double dNormThres ,   // normalized threshold (0.,1.) range
  double& dWidth )
{
  double * p = Prof.m_pProfData + iStartIndex ;
  if ( iStartIndex == Prof.m_iProfOrigin )
  {
    m_dCurrentMax = Prof.m_dMinValue ;
    m_dCurrentMin = Prof.m_dMaxValue ;

  }
  double * pEnd = Prof.m_pProfData +
    ( ( Prof.m_iActiveLen ) ? Prof.m_iActiveLen : Prof.m_iProfLen ) ;
  double dThres = Prof.m_dMinValue
    + dNormThres * ( Prof.m_dMaxValue - Prof.m_dMinValue ) ;
  bool bPositive = ( *p >= dThres ) ;
  double * pFront = p ;
  if ( bPositive )
  {
    while ( ( *( ++p ) >= dThres ) && ( p < pEnd ) ) ;
    if ( p >= pEnd )
      return 0. ;
    pFront = p  ;
    while ( ( *( ++p ) < dThres ) && ( p < pEnd ) ) ;
    if ( p >= pEnd )
      return 0. ;
  }
  else
  {
    while ( ( *( ++p ) < dThres ) && ( p < pEnd ) ) ;
    if ( p >= pEnd )
      return 0. ;
    pFront = p  ;
    while ( ( *( ++p ) >= dThres ) && ( p < pEnd ) ) ;
    if ( p >= pEnd )
      return 0. ;
  }
  double d1stPos = GetThresPosition( *( pFront - 1 ) , *pFront , dThres ) ;
  d1stPos += pFront - Prof.m_pProfData - 1 ;
  double d2ndPos = GetThresPosition( *( p - 1 ) , *p , dThres ) ;
  d2ndPos += p - Prof.m_pProfData - 1 ;
  double dPos = 0.5 * ( d1stPos + d2ndPos ) ;
  dWidth = d2ndPos - d1stPos ;
  iStartIndex = ( int ) ( p - Prof.m_pProfData ) ;
  return dPos ;
}


LPCTSTR SkewMeter::AnalyzePower()
{
  m_PowerStatusAsString.Empty() ;
  GetSystemPowerStatus( &m_SystemPowerStatus );


  m_PowerStatus &= ~( PS_ComputerBattIsCharging | PS_ComputerExtPower ) ;
  switch ( m_SystemPowerStatus.ACLineStatus )
  {
    case 0: m_PowerStatus &= ~PS_ComputerExtPower ; break ;
    case 1: m_PowerStatus |= PS_ComputerExtPower ; break ;
    default: m_PowerStatusAsString += "Computer power status problem\n" ;
  }

  m_PowerStatus &= ~PS_ComputerBattIsCharging ;
  switch ( m_SystemPowerStatus.BatteryFlag )
  {
    case 128: m_PowerStatusAsString += "No Computer battery\n" ;
    case 255: m_PowerStatusAsString += "Can't read Computer battery status\n" ;
    default:
      if ( m_SystemPowerStatus.BatteryFlag & 8 )
        m_PowerStatus |= PS_ComputerBattIsCharging ;
  }

  bool bComputerBatteryOK =
    ( ( m_SystemPowerStatus.BatteryLifePercent <= 100 )
      && ( m_SystemPowerStatus.BatteryLifePercent > 50 ) ) ;
  bool bUPSBatteryOK = ( ( m_PowerStatus & PS_UPSBattMoreThanHalf ) != 0 ) ;

  m_bBatteriesAreOK = ( m_SystemPowerStatus.ACLineStatus == 1 )
    || ( bComputerBatteryOK && bUPSBatteryOK ) ;


  m_PowerStatusSaved = m_PowerStatus ;

  if ( ( m_PowerStatus & PS_UPSDataUpdated ) == PS_UPSDataUpdated )
  {
    m_bBatteriesStatusUpdated = true ;
    m_dwPowerStatusCheckTicks = 20 ;
  }

  m_PowerStatus &= ~PS_UPSDataUpdated ;

  if ( !m_bBatteriesAreOK )
  {
    if ( ( m_iCaptureMode == CM_LiveProcess )
      || ( m_iCaptureMode == CM_LiveView ) )
    {
      m_iCaptureMode = CM_OneProcess ;
    }
  }
  return ( LPCTSTR ) m_PowerStatusAsString ;
}


int SkewMeter::AnalyzeUPSStatus( const CTextFrame * pData )
{
  if ( pData )
  {
    FXString Data = pData->GetString();
    Data = Data.MakeLower() ;
    int iPos = ( int ) Data.Find( "io_status=" ) ;
    if ( iPos >= 0 )
    {
      iPos = ( int ) Data.Find( '=' ) ;
      Data = Data.Mid( iPos + 3 ) ;
      int iStatus = 0 ;
      if ( sscanf( ( LPCTSTR ) Data , "%x" , &iStatus ) )
      {
        FXString Label = pData->GetLabel() ;
        if ( Label.Find( "LEL" ) > 0 )
        {
          m_PowerStatus &= ~PS_UPSBattMoreThanHalf ;
          m_PowerStatus |= ( ( iStatus & 1 ) ? PS_UPSBattMoreThanHalf : 0 )
            | PS_UPSBattMoreThanHalfUpdated ;
          return PS_UPSBattMoreThanHalfUpdated;
        }
        else if ( Label.Find( "LER" ) > 0 )
        {
          m_PowerStatus &= ~PS_UPSBatteryIsCharging ;
          m_PowerStatus |= ( ( iStatus & 1 ) ? PS_UPSBatteryIsCharging : 0 )
            | PS_UPSBatteryIsChargingUpdated ;
          return PS_UPSBatteryIsChargingUpdated ;
        }
        else if ( Label.Find( "TEL" ) > 0 )
        {
          m_PowerStatus &= ~PS_UPSExtPowerSupp ;
          m_PowerStatus |= ( ( iStatus & 1 ) ? PS_UPSExtPowerSupp : 0 )
            | PS_UPSExtPowerUpdated ;
          return PS_UPSExtPowerUpdated ;
        }
      }
    }
  }
  return 0;
}


int SkewMeter::AddPowerStatusAndControls( CContainerFrame * pView )
{
  int iWidth = m_LastROI.Width() ;
  int iHeight = m_LastROI.Height() ;
  // Power status view
  // Battery external power
  cmplx cBattExtPowerPt( iWidth * 0.02 , iHeight * 0.95 ) ;
  pView->AddFrame( CreateTextFrameEx( cBattExtPowerPt ,
    ( m_PowerStatusSaved & PS_UPSExtPowerSupp ) ? 0x00ff00 : 0 ,
    12 , "Batt. Ext. Pwr" ) ) ;

  cmplx cHalfBattPt( cBattExtPowerPt + cmplx( iWidth * 0.24 , 0. ) ) ;
  pView->AddFrame( CreateTextFrameEx( cHalfBattPt ,
    ( m_PowerStatusSaved & PS_UPSBattMoreThanHalf ) ? 0x00ff00 : 0x0000ff ,
    12 , ( m_PowerStatusSaved & PS_UPSBattMoreThanHalf ) ?
    "> 1/2 Battery" : "< 1/2 Battery" ) ) ;

  cmplx cIsChargingPt( cHalfBattPt + ( iWidth * 0.25 ) ) ;
  pView->AddFrame( CreateTextFrameEx( cIsChargingPt ,
    ( m_PowerStatusSaved & PS_UPSBatteryIsCharging ) ? 0x00ffff : 0 ,
    12 , ( m_PowerStatusSaved & PS_UPSBatteryIsCharging ) ?
    "Batt. charging" : "Batt. not charging" ) ) ;

  cmplx cBattToCompDiff( 0. , -0.05 * iHeight ) ;
  cmplx cComputerExtPowerPt( cBattExtPowerPt + cBattToCompDiff ) ;
  pView->AddFrame( CreateTextFrameEx( cComputerExtPowerPt ,
    ( m_PowerStatusSaved & PS_ComputerExtPower ) ? 0x00ff00 : 0 ,
    12 , "Comp. Ext. Pwr" ) ) ;

  cmplx cComputerCapacityPt( cHalfBattPt + cBattToCompDiff ) ;
  pView->AddFrame( CreateTextFrameEx( cComputerCapacityPt ,
    ( m_SystemPowerStatus.BatteryLifePercent >= 50 ) ? 0x00ff00 : 0x0000ff ,
    12 , ( m_SystemPowerStatus.BatteryLifePercent < 255 ) ?
    "Comp.Bat.=%d%%" : "No Battery" , m_SystemPowerStatus.BatteryLifePercent ) ) ;

  if ( m_SystemPowerStatus.BatteryLifePercent < 255 )
  {
    cmplx cCompChargingPt( cIsChargingPt + cBattToCompDiff ) ;
    pView->AddFrame( CreateTextFrameEx( cCompChargingPt ,
      ( m_PowerStatusSaved & PS_ComputerBattIsCharging ) ? 0x00ffff : 0 ,
      12 , ( m_PowerStatusSaved & PS_ComputerBattIsCharging ) ?
      "Comp.charging" : "Comp. not charging" ) ) ;
  }

  if ( m_iViewMode > 15 )
  {
    cmplx cPowerStatusRawData( iWidth / 4. , 0.01 * iHeight ) ;
    pView->AddFrame( CreateTextFrameEx( cPowerStatusRawData ,
      0x00ffff , 12 , "AC=%d Batt.Flag=0x%X Batt=%d%% SysStat=%d\n"
      "Batt.Remain=%dsec Batt.ToFull=%dsec. BattOK=%d" ,
      m_SystemPowerStatus.ACLineStatus , m_SystemPowerStatus.BatteryFlag ,
      m_SystemPowerStatus.BatteryLifePercent , m_SystemPowerStatus.SystemStatusFlag ,
      m_SystemPowerStatus.BatteryLifeTime , m_SystemPowerStatus.BatteryFullLifeTime ,
      m_bBatteriesAreOK ? 1 : 0 ) ) ;
  }


  // Buttons view

  cmplx cViewPt( 0.02 * iWidth , 0.05 * iHeight ) ;
  CRect Back( CPoint( ROUND( cViewPt.real() ) - 5 , ROUND( cViewPt.imag() ) - 5 ) ,
    CSize( ROUND( iWidth * 0.154 ) , iHeight / 10 ) ) ;
  m_LiveProcessingRect = Back ;
  pView->AddFrame( CreateRectFrameEx( Back ,
    ( m_iCaptureMode == CM_LiveProcess ) ?
    "color=0xffffff;back=0xffffff;" : "color=0x000000;back=0x000000" ) ) ;
  pView->AddFrame( CreateTextFrameEx( cViewPt ,
    ( m_iCaptureMode == CM_LiveProcess ) ? 0x00ff00 : 0x008080 ,
    12 , "     Live\nProcessing" ) ) ;

  cViewPt += cmplx( 0. , 0.12 * iHeight ) ;
  Back.OffsetRect( 0 , ROUND( 0.12 * iHeight ) ) ;
  m_LiveViewRect = Back ;
  CRectFrame * pRect = CreateRectFrameEx( Back ,
    ( m_iCaptureMode == CM_LiveView ) ?
    "color=0xffffff;back=0xffffff;" : "color=0x010101;back=0x010101;" ) ;
  pView->AddFrame( pRect ) ;
  pView->AddFrame( CreateTextFrameEx( cViewPt ,
    ( m_iCaptureMode == CM_LiveView ) ? 0x00ff00 : 0x008080 ,
    12 , "     Live\n     View" ) ) ;

  cViewPt += cmplx( 0. , 0.12 * iHeight ) ;
  Back.OffsetRect( 0 , ROUND( 0.12 * iHeight ) ) ;
  m_SingleFrameProcessingRect = Back ;
  pRect = CreateRectFrameEx( Back , ( m_iCaptureMode > 0 ) ?
    "color=0xffffff;back=0xffffff;" : "color=0x020202;back=0x020202;" ) ;
  pView->AddFrame( pRect ) ;
  pView->AddFrame( CreateTextFrameEx( cViewPt ,
    ( m_iCaptureMode > 0 ) ? 0x00ff00 : 0x00ff00 ,
    12 , "One Frame\nProcessing" ) ) ;

  cViewPt = cmplx( 0.8 * iWidth , 0.9 * iHeight ) ;
  Back = CRect( CPoint( ROUND( cViewPt.real() ) - 5 , ROUND( cViewPt.imag() ) - 25 ) ,
    CSize( ROUND( iWidth * 0.14 ) , iHeight / 10 ) ) ;
  m_QuitRect = Back ;
  pRect = CreateRectFrameEx( Back , "color=0;back=0x0c0c0c;" ) ;
  pView->AddFrame( pRect ) ;
  pView->AddFrame( CreateTextFrameEx( cViewPt , 0xffffff ,
    18 , "  Quit" ) ) ;

  return 0;
}

bool SkewMeter::DeleteAsyncTimer()
{
  FXAutolock al( m_LockTimer );
  if ( m_hMainTimer != NULL )
  {
    BOOL bDeleteTimerQueueTimer = DeleteTimerQueueTimer(
      NULL , m_hMainTimer , INVALID_HANDLE_VALUE );
    if ( !bDeleteTimerQueueTimer )
    {
      if ( GetLastError() != ERROR_IO_PENDING )
      {
        SEND_GADGET_ERR( _T( "Can't delete Main Loop Timer" ) );
        return false;
      }
    }
    else
      m_hMainTimer = NULL;

  }
  return true;
}

void SkewMeter::ProcessTimer()
{
  m_iTimeCount++ ;
  if ( m_pStatus && ( m_pStatus->GetStatus() == CExecutionStatus::RUN ) )
  {
    if ( ( m_iTimeCount % m_dwPowerStatusCheckTicks ) == 0 )
    {
      CTextFrame * pTimeoutNotification = CreateTextFrame( "CheckPower" , "CheckPower" );
      GetInputConnector( 0 )->Send( pTimeoutNotification );
    }
    if (
      ( ( m_bBatteriesAreOK ) || m_SystemPowerStatus.ACLineStatus )
      && m_iCaptureMode && ( m_GadgetMode == SMGM_Bottom ) )
    {
      bool bTakeFrame =
        ( m_iCaptureMode == CM_LiveProcess ) || ( m_iCaptureMode == CM_LiveView );
      if ( !bTakeFrame )
      {
        if ( m_iCaptureMode > 0 )
        {
          bTakeFrame = true ;
          m_iCaptureMode-- ;
        }
        double dTimeFromOrderToFrameArriving = m_dLastFrameTime - m_dLastFrameOrderTime ;
        if ( ( dTimeFromOrderToFrameArriving < 0. )
          && ( ( GetHRTickCount() - m_dLastFrameOrderTime ) > 180. ) )
        {
          bTakeFrame = true ;
        } ;
      }
      if ( bTakeFrame )
        DoOrderGrab() ;
    }
  }
}


int SkewMeter::IsThereClickedButton( int iXCursor , int iYCursor )
{
  CPoint Pt( iXCursor , iYCursor ) ;
  if ( m_LiveProcessingRect.PtInRect( Pt ) )
    return CM_LiveProcess ;
  if ( m_LiveViewRect.PtInRect( Pt ) )
    return CM_LiveView ;
  if ( m_SingleFrameProcessingRect.PtInRect( Pt ) )
    return CM_OneProcess ;
  if ( m_QuitRect.PtInRect( Pt ) )
    return CM_Quit ;
  return 0;
}


int SkewMeter::DoOrderGrab()
{
//   PutFrame( GetOutputConnector( OC_CamControl ) , CreateTextFrame( "Grab" , "Grab" ) ) ;
  PutFrame( GetOutputConnector( OC_CamControl ) ,
    CreateTextFrame( "FireSoftTrigger" , "FireSoftTrigger" ) ) ;
  m_dLastFrameOrderTime = GetHRTickCount() ;
  return 0;
}


int SkewMeter::SetCameraExposure( int iExposure_us )
{
  FXString CameraCommand;
  CameraCommand.Format( "shutter_us=%d;" , iExposure_us );
  return CameraControl( CameraCommand );
}

BOOL SkewMeter::CameraControl( LPCTSTR pCommand )
{
  FXString Command;
  Command.Format( "set properties(%s;);" , pCommand );
  CTextFrame * pOut = CreateTextFrame( Command , "CameraCommand" );
  return PutFrame( GetOutputConnector( OC_CamControl ) , pOut , 200 );
}
