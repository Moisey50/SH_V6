#include "stdafx.h"
#include "MPPT.h"
#include <files/imgfiles.h>
#include <fxfc/FXRegistry.h>
#include <imageproc/statistics.h>


#define THIS_MODULENAME "MPPT"

USER_FILTER_RUNTIME_GADGET( MPPT , "Video.recognition" );

CRect g_ROI( 910 , 550 , 1010 , 650 );

CCoor3t MPPT::m_ShiftsUL;
int     MPPT::m_iNProcessedBlanks;
double  MPPT::m_dZAfterShift_um;
double  MPPT::m_dZStdAfterShift_um;



cmplx CalibTable[] =
{
  cmplx( 0. , 0. ) ,
  cmplx( 100. , 0. ) ,
  cmplx( 0. , 100. ) ,
  cmplx( -100. , 0. ) ,
  cmplx( -100. , 0. ) ,
  cmplx( 0. , -100. ) ,
  cmplx( 0. , -100. ) ,
  cmplx( 100. , 0. ) ,
  cmplx( 100. , 0. )
};

VOID CALLBACK MPPTWatchdogTimerRoutine( LPVOID lpParam , BOOLEAN TimerOrWaitFired )
{
  MPPT * pGadget = ( MPPT* ) lpParam;
  //   if ( pGadget->m_pStatus->GetStatus() == CExecutionStatus::RUN )
  //   {
  CTextFrame * pTimeoutNotification = CreateTextFrame( "Timeout" , "Timeout" );
  try
  {
    pGadget->GetInputConnector( 0 )->Send( pTimeoutNotification );
    pGadget->DeleteWatchDog();
    return ;
  }
  catch (CMemoryException* e)
  {
    TCHAR Msg[ 2000 ] ;
    e->GetErrorMessage( Msg , 2000 ) ;
    TRACE( "\n MPPTWatchdogTimerRoutine memory exception %s" , Msg ) ;
  }
  catch (CException* e)
  {
    TCHAR Msg[ 2000 ] ;
    e->GetErrorMessage( Msg , 2000 ) ;
    TRACE( "\n MPPTWatchdogTimerRoutine exception %s" , Msg ) ;
  }
  pTimeoutNotification->Release() ;
}


LPCTSTR MPPT::GetWorkingModeName()
{
  switch (m_WorkingMode)
  {
    case MPPTWM_NotSet: return _T( "NotSet" );
    case MPPTWM_Down:    return _T( "DownLook" );
    case MPPTWM_UpFront:return _T( "UpFront" );
    case MPPTWM_UpSide:return _T( "UpSide" );
    case MPPTWM_FinalSide:return _T( "FinalSide" );
    case MPPTWM_FinalFront:return _T( "FinalFront" );
    case MPPTWM_Common: return _T( "Common" );
  }
  return "Unknown";
}
LPCTSTR MPPT::GetShortWorkingModeName()
{
  switch (m_WorkingMode)
  {
    case MPPTWM_NotSet: return _T( "NotSet" );
    case MPPTWM_Down:    return _T( "DL_" );
    case MPPTWM_UpFront:return _T( "ULF_" );
    case MPPTWM_UpSide:return _T( "ULS_" );
    case MPPTWM_FinalSide:return _T( "FS_" );
    case MPPTWM_FinalFront:return _T( "FF_" );
    case MPPTWM_Common: return _T( "CMN_" );
  }
  return "Unknown";
}

LPCTSTR MPPT::GetWorkingStateName()
{
  switch (m_WorkingState)
  {
    case State_Idle: return _T( "STUnk_" );
    case State_ShortCommand: return _T( "Short_" );
    case State_Finished: return _T( "End_" );
    case State_Stopped: return _T( "Stop_" );
    case DL_LiveVideoPinFocusing: return _T( "LvPinF_" );
    case DL_LiveVideoCavity: return _T( "LvCav_" );
    case DL_LiveVideoLaser: return _T( "LvLas_" );
    case DL_ScaleCalib: return _T( "ScaleCalib_" );
    case DL_LaserCalib: return _T( "LaserCalib_" );
    case DL_PinNorthSide:
    case DL_PinEastSide:
    case DL_PinSouthSide:
    case DL_PinWestSide: return _T( "PinSide_" );
    case DL_MeasCavity: return _T( "Cavity_" );
    case DL_CaptureCavityFinalImage: return _T( "CavXYFin" );
    case DL_MeasCavityXY: return _T( "CavXY_" );
    case DL_MoveCavityForFinalImage: return _T( "CavMovFin" );
    case DL_CaptureZWithCorrectHeigth: return _T( "CavZCorr" );
    case DL_MeasCavityZ: return _T( "CavZ_" );
    case DL_FinalImageOverPin: return _T( "FinalPin_" );
    case DL_LongSweep:
    case DL_ShortSweep: return _T( "ZSweep_" );
    case ULS_ZCorrection:        return _T( "ZCoor_" ) ;
    case ULS_ZCorrection2:        return _T( "ZCoor2_" ) ;
    case ULS_FirstZForBlank:      return _T( "FirstZForBlank" );
    case ULS_ZCorrectionForMeas:  return _T( "ZMeasCorr_" ) ;
    case ULS_ZCorrectionForMeas2:      return _T( "ZMeasCorr2_" );
    case ULS_ZMeasurement:      return _T( "ZMeas_" );
    case ULS_GrabFinal:      return _T( "ZGrabFinal" );
    case ULF_MoveAfterMeasurement:return _T( "XYCorrAfterMeas_" ) ;
    case ULF_MeasureAndCorrect:  return _T( "XYMeas&Corr_" );
    case ULF_WaitForExternalReady:  return _T( "WaitForReady_" );
    case ULF_RightSideCorrection:  return _T( "RightSideCorr_" );
    case ULF_LiveVideo:  return _T( "LiveVideo_" );

  }
  m_Info2.Format( "Unknown_Id=%d_" , m_WorkingState );
  return m_Info2 ;
}


MPPT::MPPT() :
  m_CurrentPart( "Default" , NULL , true )
  , m_LastROI( 0 , 0 , 1920 , 1200 )
  , m_LastROICenter( 960 , 600 )
  , m_cLastROICenter( 960. , 600. )
{
  m_OldWorkingMode = m_WorkingMode = MPPTWM_NotSet;
  m_SaveMode = SaveMode_No;
  m_iCentralZoneWidthForX = 100;
  m_ViewDetails = 1;
  m_dScale_um_per_pix = 1.7209;
  m_dZScale_um_per_pix = 1.95;
  m_iLaserExposure = 20;
  m_iLaserExposureOnPin = 30;
  m_iLaserExposureOnCavity = 140;
  m_iNewCavityExposure = m_iCavityExposure = 1750;
  m_iNewFocusExposure = m_iCavityFocusExposure = 300 ;
  m_iPinExposure = 4000;
  m_dLineFilterThres_pix = 0.7;
  m_dMaxDistCut_pix = 2.0;
  m_CavityEdge = CavEdge_LowerAndUpper;
  m_UsedBlankEdge = SQREdge_Right;
  m_dYCenterRelativelyToLowerEdge = 0.5;
  m_iSaveDecimator = 5;
  m_iFrameCount = 0;
  m_iCntForSave = 0;
  m_iAfterCommandSaved = 0;
  m_iAverager = 10;
  m_iAverageCnt = 0;
  m_iDistToCenterForVertLines = m_iPinHorizBaseLine
    = m_iDLCavityHorizBaseLine = m_iULSPartHorizBaseLine = 0;
  m_dDLCavityHorizBaseLine = 0. ;
  m_iDistToFocusingArea = 600;
  m_iFocusingRadius = 100;
  m_dPinDiam_um = 6000.;
  m_MatrixParamAsString = _T( "5,9,100." );
  m_CalibMatrixSize = CSize( 5 , 9 );
  m_bRestoreScales = true;
  m_dLastGrabOrderTime = 0.;
  m_bPinOnlyCalib = false;
  m_iNProcessedCavities = 0;
  m_iNProcessedBlanks = 0;
  m_iNCavityZMeasurements = 1;
  m_iNZMeasured = 0;
  m_ZMethod = AnalyzeAndGetDLMeasurementZMethod();
  m_iPassNumber = 0;
  m_cPartPatternPt = cmplx( DBL_MAX , DBL_MAX );
  m_bAdditionalShift = false ;
  m_PreviousState = State_Idle ;
  m_bUpdatePartParameters = true;
  m_iFocusLogPeriod_samples = 0;
  m_iSamplesCnt = 0;
  m_SaveFinalImageWithGraphics = 0 ;
  m_bMeasureZ = TRUE;
  m_pSideBlankContur = NULL ;
  m_iFrontExposure_us = 1000;
  m_PlotSampleCntr = 0 ;
  m_bStabilizationDone = false ;
  m_hWatchDogTimer = NULL;
  m_WorkingState = State_Unknown;


  m_OutputMode = modeReplace;
  FXRegistry Reg( "TheFileX\\Micropoint" );
  FXString SelectedPartName = Reg.GetRegiString( "PartsData" , "SelectedPart" , "" ) ;
  RestoreKnownParts() ;
  if (m_SelectedPartName.IsEmpty() && m_KnownParts.size())
    m_SelectedPartName = SelectedPartName ;
  LoadAndUpdatePartParameters();
  SelectCurrentPart( SelectedPartName ); // override written in graph
  Reg.WriteRegiString( "PartsData" , "SelectedPart" , SelectedPartName );
  CheckCreateDataDir();
  init();
};

void MPPT::ShutDown()
{
  UserGadgetBase::ShutDown();
  ClearConturData();
}

static const char * pWorkingMode = "Unknown;DownLook;UpLookStraight;UpLookSide;FinalSide;FinalFront;";
static const char * pSaveMode = "NoSave;SaveFinal;SaveBad;SaveAll;OnePerSerie;Decimate;";
static const char * pCavityEdgeSelect = "Unknown;Lower;Upper;Upper&Lower;Lower_Xc;Upper_Xc;";
static const char * pBlankEdgeSelect = "BlankCenter;LeftEdge;UpperEdge;RightEdge;LowerEdge;";
static const char * pFiltration = "Off;On;";



void MPPT::PropertiesRegistration()
{
  addProperty( SProperty::COMBO , _T( "WorkingMode" ) ,
    ( int * ) &m_WorkingMode , SProperty::Int , pWorkingMode );
  SetChangeNotification( _T( "WorkingMode" ) , ConfigParamChange , this );
  addProperty( SProperty::SPIN , "ViewDetails" , &m_ViewDetails , SProperty::Long , 0 , 10 );
  addProperty( SProperty::COMBO , _T( "AutoSave" ) ,
    ( int * ) &m_SaveMode , SProperty::Int , pSaveMode );
  addProperty( SProperty::COMBO , _T( "SaveFinalGraphics" ) ,
    ( int * ) &m_SaveFinalImageWithGraphics , SProperty::Int , pFiltration );
  addProperty( SProperty::EDITBOX , _T( "PartName" ) ,
    &m_PartName , SProperty::String );
  //SetChangeNotification( _T( "PartName" ) , ConfigParamChange , this );

  m_PartListForStdDialog[ 0 ] = 0 ;

  for (size_t i = 0 ; i < m_KnownParts.size() ; i++) // form list for std dialog
  {
    _tcscat_s( m_PartListForStdDialog , ( m_KnownParts[ i ].m_Name + ';' ).c_str() ) ;
    if (m_SelectedPartName == m_KnownParts[ i ].m_Name.c_str())
      m_iSelectedPart = ( int ) i ;
  }

  addProperty( SProperty::COMBO , _T( "PartSelect" ) , &m_iSelectedPart , SProperty::Int , m_PartListForStdDialog ) ;
  SetChangeNotification( _T( "PartSelect" ) , ConfigParamChange , this );
  switch (m_WorkingMode)
  {
    case MPPTWM_Down:
      addProperty( SProperty::SPIN , _T( "CentralZoneWidth_pix" ) ,
        &m_iCentralZoneWidthForX , SProperty::Long , 3 , 500 );
      addProperty( SProperty::SPIN , _T( "LaserExpOnPin_us" ) ,
        &m_iLaserExposureOnPin , SProperty::Long , 20 , 500 );
      addProperty( SProperty::SPIN , _T( "LaserExpOnCavity_us" ) ,
        &m_iLaserExposureOnCavity , SProperty::Long , 20 , 500 );
      addProperty( SProperty::SPIN , _T( "CavityExposure_us" ) ,
        &m_iCavityExposure , SProperty::Long , 20 , 5000 );
      SetChangeNotification( _T( "CavityExposure_us" ) ,
        ConfigParamChange , this );
      addProperty( SProperty::SPIN , _T( "PinExposure_us" ) ,
        &m_iPinExposure , SProperty::Long , 20 , 5000 );
      addProperty( SProperty::SPIN , _T( "CavityFocusExp_us" ) ,
        &m_iCavityFocusExposure , SProperty::Long , 20 , 5000 );
      addProperty( SProperty::COMBO , _T( "EdgeSelect" ) ,
        ( int * ) &m_CavityEdge , SProperty::Int , pCavityEdgeSelect );
      SetChangeNotification( _T( "CavityEdgeSelect" ) , ConfigParamChange , this );
      if (m_CavityEdge == CavEdge_LowerAndUpper)
      {
        addProperty( SProperty::EDITBOX , _T( "RelativeY" ) ,
          &m_dYCenterRelativelyToLowerEdge , SProperty::Double );
      }
      addProperty( SProperty::SPIN , _T( "NumberOfZMeas" ) ,
        ( int * ) &m_iNCavityZMeasurements , SProperty::Long , 1 , 100 );

      addProperty( SProperty::EDITBOX , _T( "LineFilterThres_pix" ) ,
        &m_dLineFilterThres_pix , SProperty::Double );
      addProperty( SProperty::EDITBOX , _T( "MaxDistCut_pix" ) ,
        &m_dMaxDistCut_pix , SProperty::Double );
      addProperty( SProperty::SPIN , _T( "VertLinesPos_um" ) ,
        &m_iDistToCenterForVertLines , SProperty::Long , 0 , 400 );
      addProperty( SProperty::SPIN , _T( "HorisLineShift_um" ) ,
        &m_iDLCavityHorizBaseLine , SProperty::Long , -1000 , 1000 );
      addProperty( SProperty::EDITBOX , _T( "PinDiameter_um" ) ,
        &m_dPinDiam_um , SProperty::Double );
      addProperty( SProperty::SPIN , _T( "DistToFocusingArea_pix" ) ,
        &m_iDistToFocusingArea , SProperty::Long , 1 , 5000 );
      break;
    case MPPTWM_UpFront:
      addProperty( SProperty::SPIN , _T( "FrontExposure_us" ) ,
        &m_iFrontExposure_us , SProperty::Long , 20 , 10000 );
      addProperty( SProperty::COMBO , _T( "BlankEdgeSelect" ) ,
        ( int * ) &m_UsedBlankEdge , SProperty::Int , pBlankEdgeSelect );
      break;
    case MPPTWM_UpSide:
      addProperty( SProperty::SPIN , _T( "SideExposure_us" ) ,
        &m_iSideExposure_us , SProperty::Long , 20 , 10000 );
      break;
  }
  addProperty( SProperty::EDITBOX , _T( "CalibMatrixParams" ) ,
    &m_MatrixParamAsString , SProperty::String );
  SetChangeNotification( _T( "CalibMatrixParams" ) , ConfigParamChange , this );
  addProperty( SProperty::EDITBOX , _T( "Scale_um_per_pix" ) ,
    &m_dScale_um_per_pix , SProperty::Double );
  addProperty( SProperty::SPIN , _T( "Averaging" ) ,
    &m_iAverager , SProperty::Long , 1 , 100 );

  //   addProperty( SProperty::EDITBOX , _T( "FinalTol_mrad" ) ,
//     &m_dFinalTolerance , SProperty::Double );

};

void MPPT::ConnectorsRegistration()
{
  addInputConnector( transparent , "InputForAll" );
  addOutputConnector( transparent , "OutVideo" );
  addOutputConnector( transparent , "DataOut" );
  addOutputConnector( text , "CameraControl" );
  addOutputConnector( text , "MeasurementControl" );
  addOutputConnector( text , "LPFControl" );
};

void MPPT::ConfigParamChange( LPCTSTR pName , void* pObject , bool& bInvalidate , bool& bInitRescan )
{
  MPPT * pGadget = ( MPPT* ) pObject;
  if (pGadget)
  {
    if (!_tcsicmp( pName , _T( "WorkingMode" ) ))
    {
      if (pGadget->m_OldWorkingMode != pGadget->m_WorkingMode)
      {
        pGadget->m_OldWorkingMode = pGadget->m_WorkingMode;
        pGadget->m_bRestoreScales = true;
        bInitRescan = true;
      }

      if (pGadget->m_bRestoreScales)
        pGadget->RestoreKnownParts();
      //       pGadget->PropertiesReregistration() ;
      bInvalidate = true;
      pGadget->m_CurrentLogFilePath = pGadget->CheckCreateCurrentLogs();
      pGadget->SaveLogMsg( "\nLog Initialized at %s\n" ,
        ( LPCTSTR ) GetTimeAsString_ms() );
      pGadget->SaveCSVLogMsg( "\nLog Initialized at %s\n" ,
        ( LPCTSTR ) GetTimeAsString_ms() );
      if (pGadget->m_WorkingMode == MPPTWM_Down)
      {
        FXRegistry Reg( "TheFileX\\Micropoint" );
        pGadget->m_dZStepForHighResolutionDefocusing_um = Reg.GetRegiDouble(
          "Parameters" , "ZStepForHighResolutionDefocusing_um" , 10. );
        pGadget->m_dForFastZBigStep_um = Reg.GetRegiDouble(
          "Parameters" , "FastZBigStep_um" , 50. );
        pGadget->m_dForFastZSmallStep_um = Reg.GetRegiDouble(
          "Parameters" , "FastZSmallStep_um" , 20. );
        pGadget->m_iNZShouldBeMeasuredWithHighResolution = Reg.GetRegiInt(
          "Parameters" , "NZHighResolutionMeasurements" , 10 );
        pGadget->m_dDefocusThreshold = Reg.GetRegiDouble(
          "Parameters" , "DefocusingThreshold" , 0.8 );
      }
    }
    else if (!_tcsicmp( pName , _T( "CalibMatrixParams" ) ))
    {
      int iLx , iLy;
      double dCalibStep;
      int iNSCanned = sscanf( ( LPCTSTR ) pGadget->m_MatrixParamAsString , _T( "%d,%d,%lf" ) ,
        &iLx , &iLy , &dCalibStep );
      if (iNSCanned == 3)
      {
        if (( iLx & 1 ) && ( iLy & 1 ) && ( dCalibStep > 50. )
          && ( iLx > 0 ) && ( iLy > 0 ))
        {
          pGadget->m_CalibMatrixSize = CSize( iLx , iLy );
        }
        else
        {
          FxSendLogMsg( MSG_ERROR_LEVEL , _T( "MPPT Prop Scan" ) , 0 ,
            _T( "Matrix sizes should be ODD (%d,%d)" ) , iLx , iLy );
        }
      }
      else
        FxSendLogMsg( MSG_ERROR_LEVEL , _T( "MPPT Prop Scan" ) , 0 ,
          _T( "Bad Calib Matrix params '%s'" ) , ( LPCTSTR ) pGadget->m_MatrixParamAsString );
    }
    else if (!_tcsicmp( pName , _T( "EdgeSelect" ) ))
    {
      bInvalidate = true;
    }
    else if (!_tcsicmp( pName , _T( "CavityExposure_us" ) ))
    {
      if (pGadget->m_WorkingState == DL_MeasCavityXY
        || pGadget->m_WorkingState == DL_MeasCavity
        || pGadget->m_WorkingState == DL_LiveVideoCavity)
      {
        pGadget->m_iNewCavityExposure = pGadget->m_CurrentPart.m_Cavity.m_iCavityExp_us =
          pGadget->m_iCavityExposure;
      }
    }
    else if (!_tcsicmp( pName , _T( "PartSelect" ) ))
    {
      FXRegistry Reg( "TheFileX\\Micropoint" );
      if (pGadget->m_iSelectedPart >= 0   // protection against selected part number from another
        && pGadget->m_iSelectedPart < ( int ) pGadget->m_KnownParts.size()) // computer, where is more parts
      {                                        // are saved in registry
        FXString PartName( pGadget->m_KnownParts[ pGadget->m_iSelectedPart ].m_Name.c_str() ) ;
        pGadget->SelectPartByName( PartName , false ) ;
        pGadget->m_SelectedPartName = pGadget->m_PartName = PartName;
        bInvalidate = true ;
      }
    }
    else if (!_tcsicmp( pName , _T( "PartName" ) ))
    {
      pGadget->SelectPartByName( pGadget->m_PartName , true );
      bInvalidate = true;
      return;
    }
  }
}

CDataFrame * MPPT::ProcessCavity( const CDataFrame * pDataFrame )
{
  m_bLastCavityResult = false;
  CContainerFrame * pPartialOut = CContainerFrame::Create();
  if (m_CavitiesCenters.Count() == 0
    || ( m_CavitiesCenters.Count() != 2 && m_CavitiesConturs.Count() != 2 ))
  {
    return pPartialOut;
  }

  cmplx FilterStatViewPt( 1350 , 250 ) ;
  pPartialOut->AddFrame( CreateTextFrame( FilterStatViewPt , _T( "0x00ffff" ) ,
    14 , _T( "FiltersShow" ) , pDataFrame->GetId() , _T( "Filters Enabling:\n"
      "Additional Angle Filter: %s\n"
      "Max diff between angles=%.2fdeg\nY difference: %s (%.2fum)\nProfile Using: %s\n"
      "Compare Cont and Profile: %s(%.2fum)\n" ) ,
      ( m_CurrentPart.m_Cavity.m_bAngleFiltrationOn ) ? _T( "Yes" ) : _T( "No" ) ,
    m_CurrentPart.m_Cavity.m_dMaxAngleDiffBetweenInternalEdges_deg ,
    ( m_CurrentPart.m_Cavity.m_bCavCheckYdiffer ) ? _T( "Yes" ) : _T( "No" ) ,
    m_CurrentPart.m_Cavity.m_dAllowedYdiffer_um ,
    ( m_CurrentPart.m_Cavity.m_bCavEnableProfileUsing ) ? _T( "Yes" ) : _T( "No" ) ,
    ( m_CurrentPart.m_Cavity.m_bCavCompareContAndProfResults ) ? _T( "Yes" ) : _T( "No" ) ,
    m_CurrentPart.m_Cavity.m_dMaxDiffBetweenXsOnConturAndProfile_um ) ) ;

  const CFigureFrame *pCentFig1 = ( const CFigureFrame * ) m_CavitiesCenters.GetFrame( 0 );
  cmplx cCentPt1 = CDPointToCmplx( pCentFig1->GetAt( 0 ) );
  const CFigureFrame *pCentFig2 = ( const CFigureFrame * ) m_CavitiesCenters.GetFrame( 1 );
  cmplx cCentPt2 = CDPointToCmplx( pCentFig2->GetAt( 0 ) );
  const CFigureFrame *pFig1 = ( const CFigureFrame * ) m_CavitiesConturs.GetFrame( 0 );
  const CFigureFrame *pFig2 = ( const CFigureFrame * ) m_CavitiesConturs.GetFrame( 1 );

  // pFig1 is left figure, pFig2 is right figure
  if (cCentPt1.real() > cCentPt2.real())
  {
    swap( cCentPt1 , cCentPt2 );
    swap( pFig1 , pFig2 );
  }
  CmplxArray ExtremesLeft , ExtremesRight;
  FXIntArray IndexesLeft , IndexesRight ;
  cmplx cCentFigLeft = FindExtrems( pFig1 ,
    ExtremesLeft , &IndexesLeft , &m_cLastLeftCavitySize_pix );
  cmplx cCentFigRight = FindExtrems( pFig2 ,
    ExtremesRight , &IndexesRight , &m_cLastRightCavitySize_pix );

  double dLeftUpYCorr = 0. , dLeftDownYCorr = 0. , dRightUpYCorr = 0. , dRightDownYCorr = 0.;
  if (m_CurrentPart.m_Cavity.m_dYCorrectionWidth_um > 0.)
  {
    double dExtremWidthThres = m_CurrentPart.m_Cavity.m_dYCorrectionWidth_um / m_dScale_um_per_pix ;
    CDataFrame * pAddView = CavityYCorrection(
      pFig1 , IndexesLeft , dExtremWidthThres , dLeftUpYCorr , dLeftDownYCorr ) ;
    if (pAddView)
      pPartialOut->AddFrame( pAddView ) ;
    pAddView = CavityYCorrection(
      pFig2 , IndexesRight , dExtremWidthThres , dRightUpYCorr , dRightDownYCorr ) ;
    if (pAddView)
      pPartialOut->AddFrame( pAddView ) ;
  }

  double dMaxNormErr = m_CurrentPart.m_Cavity.m_dCavitySizeTolerance_perc * 0.01 ;
  double dMaxAbsErrorForDist_um = m_CurrentPart.m_Cavity.m_dDistBetweenAreas_um * dMaxNormErr ;
  double dYRange = m_CurrentPart.m_Cavity.m_iCentralZoneWidth_pix / 2. ;

  cmplx cHorizLeftUpper = ExtremesLeft[ EXTREME_INDEX_TOP ] + cmplx( 0. , dLeftUpYCorr );
  cmplx cHorizRightUpper = ExtremesRight[ EXTREME_INDEX_TOP ] + cmplx( 0. , dRightUpYCorr );
  cmplx cHorizLeftLower = ExtremesLeft[ EXTREME_INDEX_BOTTOM ] + cmplx( 0. , dLeftDownYCorr );
  cmplx cHorizRightLower = ExtremesRight[ EXTREME_INDEX_BOTTOM ] + cmplx( 0. , dRightDownYCorr );

  m_cLastLeftCavitySize_um = m_cLastLeftCavitySize_pix * m_dScale_um_per_pix;
  m_cLastRightCavitySize_um = m_cLastRightCavitySize_pix * m_dScale_um_per_pix;
  cmplx cAvCavitySize_um = ( m_cLastRightCavitySize_um + m_cLastLeftCavitySize_um ) / 2. ;

  cCentPt1 = cCentFigLeft;
  cCentPt2 = cCentFigRight;
  CLine2d CenterLine( cCentPt1 , cCentPt2 );
  if (m_ViewDetails > 3)
  {
    CFigureFrame * pCentLine = CreateLineFrame( cCentPt1 , cCentPt2 , 0xff0000 , "CenterLine" );
    pPartialOut->AddFrame( pCentLine );
  }

  cmplx * pcFig1 = ( cmplx* ) pFig1->GetData();
  cmplx * pcFig2 = ( cmplx* ) pFig2->GetData();

  double dHalfWidth = ( double ) m_iCentralZoneWidthForX / 2. ;
  StraightLineRegression SRegrLeft , SRegrRight;
  CLine2d SLeftLine = GetRegressionNearPoint( *pFig1 , cCentFigLeft , true ,
    dHalfWidth , pPartialOut , SRegrLeft ) ;
  CLine2d SRightLine = GetRegressionNearPoint( *pFig2 , cCentFigRight , false ,
    dHalfWidth , pPartialOut , SRegrRight ) ;

  cmplx cCalcLeftUpper = SLeftLine.GetPtForY( cCentFigLeft.imag() - dHalfWidth );
  cmplx cCalcLeftLower = SLeftLine.GetPtForY( cCentFigLeft.imag() + dHalfWidth );
  cmplx cCalcRightUpper = SRightLine.GetPtForY( cCentFigRight.imag() - dHalfWidth );
  cmplx cCalcRightLower = SRightLine.GetPtForY( cCentFigRight.imag() + dHalfWidth );

  if (m_ViewDetails >= 2)
  {
    CFigureFrame * pLeftLine = CreateLineFrame( cCalcLeftUpper ,
      cCalcLeftLower , 0x00ff00 , "LeftEdge" );
    CFigureFrame * pRightLine = CreateLineFrame( cCalcRightUpper ,
      cCalcRightLower , 0x00ff00 , "RightEdge" );
    pPartialOut->AddFrame( pLeftLine );
    pPartialOut->AddFrame( pRightLine );
  }

  double dUpperDiff_um = ( cCalcRightUpper.real() - cCalcLeftUpper.real() ) * m_dScale_um_per_pix ;
  double dLowerDiff_um = ( cCalcRightLower.real() - cCalcLeftLower.real() ) * m_dScale_um_per_pix ;
  double dGW_um = ( dUpperDiff_um + dLowerDiff_um ) / 2. ;
  double dUpperError = ( dUpperDiff_um - m_CurrentPart.m_Cavity.m_dDistBetweenAreas_um ) ;
  double dLowerError = ( dLowerDiff_um - m_CurrentPart.m_Cavity.m_dDistBetweenAreas_um ) ;
  double dNormUpperError = dUpperError / m_CurrentPart.m_Cavity.m_dDistBetweenAreas_um ;
  double dNormLowerError = dLowerError / m_CurrentPart.m_Cavity.m_dDistBetweenAreas_um ;
  FXString DistErr , AngleErr ;
  double dNormAvError = ( dNormUpperError + dNormLowerError ) * 0.5;
  double dNormAvErrorAbs = ( fabs( dNormUpperError ) + fabs( dNormLowerError ) ) * 0.5;
  bool bDistErr = dNormAvErrorAbs > dMaxNormErr ;
  if (dNormAvErrorAbs > m_CurrentPart.m_Cavity.m_dCavitySizeTolerance_perc * 0.01)
  {
    DistErr.Format( "UpEr=%.1f DownEr=%.1f\n AvErr=%.2f [%.2f] um\n"
      "\nFL=%.1fum\nW=%.1fum\nGW=%.1fum" ,
      dUpperError , dLowerError , dNormAvErrorAbs * m_CurrentPart.m_Cavity.m_dDistBetweenAreas_um ,
      dMaxAbsErrorForDist_um ,
      cAvCavitySize_um.imag() , 0. , dGW_um ) ;
  }
  double dAngleDiff_deg = RadToDeg( SLeftLine.GetAngleDiff( SRightLine ) );
  double dAbsDiff_deg = fabs( dAngleDiff_deg );
  bool bAnglesDeltaIsOK = true ;
  if (dAbsDiff_deg > m_CurrentPart.m_Cavity.m_dMaxAngleDiffBetweenInternalEdges_deg)
  {
    bAnglesDeltaIsOK = false ;
    if (m_CurrentPart.m_Cavity.m_bAngleFiltrationOn)
    {
      CFilterRegression LRegrFiltered , RRegrFiltered ;
      SRegrLeft.DoOneSideFiltering( true , 7 , LRegrFiltered ) ;
      SRegrRight.DoOneSideFiltering( true , 7 , RRegrFiltered ) ;

      CLine2d LeftFiltLine = LRegrFiltered.GetCLine2d() ;
      CLine2d RightFiltLine = RRegrFiltered.GetCLine2d() ;
      double dFiltAngleDiff = RadToDeg( LeftFiltLine.GetAngleDiff( RightFiltLine ) ) ;
      double dAbsFiltDiff_deg = fabs( dFiltAngleDiff ) ;
      cmplx cCalcFiltLeftUpper = LeftFiltLine.GetPtForY( cCentFigLeft.imag() - dHalfWidth );
      cmplx cCalcFiltLeftLower = LeftFiltLine.GetPtForY( cCentFigLeft.imag() + dHalfWidth );
      cmplx cCalcFiltRightUpper = RightFiltLine.GetPtForY( cCentFigRight.imag() - dHalfWidth );
      cmplx cCalcFiltRightLower = RightFiltLine.GetPtForY( cCentFigRight.imag() + dHalfWidth );
      if (m_ViewDetails >= 2)
      {
        CFigureFrame * pLeftFiltLine = CreateLineFrame( cCalcFiltLeftUpper ,
          cCalcFiltLeftLower , 0xff0000 , "LeftFiltEdge" );
        CFigureFrame * pRightFiltLine = CreateLineFrame( cCalcFiltRightUpper ,
          cCalcFiltRightLower , 0xff0000 , "RightFiltEdge" );
        pPartialOut->AddFrame( pLeftFiltLine );
        pPartialOut->AddFrame( pRightFiltLine );

        CFigureFrame * LPts = CreateFigureFrame( LRegrFiltered.m_Pts.data() ,
          ( int ) LRegrFiltered.m_Pts.size() , ( DWORD ) 0x00ffff ) ;
        CFigureFrame * RPts = CreateFigureFrame( RRegrFiltered.m_Pts.data() ,
          ( int ) RRegrFiltered.m_Pts.size() , ( DWORD ) 0x00ffff ) ;

        pPartialOut->AddFrame( LPts );
        pPartialOut->AddFrame( RPts );
      }

      if (dAbsFiltDiff_deg > m_CurrentPart.m_Cavity.m_dMaxAngleDiffBetweenInternalEdges_deg)
      {
        AngleErr.Format( "Left and Right edges\nare not parallel\n dAngle=%.2f deg\n"
          "FiltAng=%.2f deg" , dAngleDiff_deg , dFiltAngleDiff ) ;
      }
      else
      {
        bAnglesDeltaIsOK = true ;
        cCalcLeftUpper = cCalcFiltLeftUpper ;
        cCalcLeftLower = cCalcFiltLeftLower ;
        cCalcRightUpper = cCalcFiltRightUpper ;
        cCalcRightLower = cCalcFiltRightLower ;
      }
    }
    else
    {
      AngleErr.Format( "\nLeft and Right edges\nare not parallel\n"
        "dAngle=%.2f deg, Limit=%.2f deg\n" ,
        dAngleDiff_deg , m_CurrentPart.m_Cavity.m_dMaxAngleDiffBetweenInternalEdges_deg ) ;
    }
  }

  cmplx cLeftCent = ( cCalcLeftUpper + cCalcLeftLower ) / 2. ;
  cmplx cRightCent = ( cCalcRightUpper + cCalcRightLower ) / 2. ;
  cmplx cCavCent = ( cLeftCent + cRightCent ) / 2. ;
  double dDistBycontours = ( cRightCent.real() - cLeftCent.real() ) * m_dScale_um_per_pix ;

  bool bDeltaYIsOK = true ;
  FXString DeltaYMsg ;
  if (m_CurrentPart.m_Cavity.m_bCavCheckYdiffer)
  {
    // Check for delta Y on left and right sides
    double dDeltaYUpper = m_dScale_um_per_pix * fabs( cHorizLeftUpper.imag() - cHorizRightUpper.imag() ) ;
    double dDeltaYLower = m_dScale_um_per_pix * fabs( cHorizLeftLower.imag() - cHorizRightLower.imag() ) ;
    switch (m_CurrentPart.m_Cavity.m_CavityEdge)
    {
      case CavEdge_Lower:
      case CavEdge_Lower_Xc:
        bDeltaYIsOK = ( dDeltaYLower <= m_CurrentPart.m_Cavity.m_dAllowedYdiffer_um ) ;
        if (!bDeltaYIsOK)
          DeltaYMsg.Format( "\nToo big Y difference %.2fum" , dDeltaYLower ) ;
        break;
      case CavEdge_Upper:
      case CavEdge_Upper_Xc:
        bDeltaYIsOK = ( dDeltaYUpper <= m_CurrentPart.m_Cavity.m_dAllowedYdiffer_um ) ;
        if (!bDeltaYIsOK)
          DeltaYMsg.Format( "\nToo big Y difference %.2fum" , dDeltaYUpper ) ;
        break;
      case CavEdge_LowerAndUpper:
        bDeltaYIsOK = ( dDeltaYUpper <= m_CurrentPart.m_Cavity.m_dAllowedYdiffer_um )
          && ( dDeltaYLower <= m_CurrentPart.m_Cavity.m_dAllowedYdiffer_um );
        if (!bDeltaYIsOK)
          DeltaYMsg.Format( "\nToo big Y diff %.2f-%.2f um" ,
            dDeltaYLower , dDeltaYUpper ) ;
        break;

    }
  }

  cmplx cCavCentByProfiles ;

  cmplx CentPtView( cLeftCent + cmplx( 0. , -0.68 * m_CurrentPart.m_Cavity.m_dPlaneHeight_um / m_dScale_um_per_pix ) ) ;
  pPartialOut->AddFrame( CreateTextFrame( CentPtView , "0xFFFFFF" , 10 ,
    NULL , 0 , "ByCont(%.2f,%.2f) GW=%.2fum" ,
    cCavCent.real() , cCavCent.imag() , dDistBycontours ) ) ;

  cmplx cViewPt = cmplx(
    m_cLastROICenter.real() * 0.1 , m_cLastROICenter.imag() * 0.8 );

  cmplx cLeftEdgeByProfiles , cRightEdgeByProfiles ;
  bool bFoundByProfiles = FindCavityInternalEdges(
    cCentPt1 , cCentPt2 , cLeftEdgeByProfiles , cRightEdgeByProfiles , pPartialOut ) ;
  double dDistByProfiles = 0. , dDistByProfilesErr = DBL_MAX ;
  if (bFoundByProfiles && m_CurrentPart.m_Cavity.m_bCavEnableProfileUsing)
  {
    cCavCentByProfiles = ( cLeftEdgeByProfiles + cRightEdgeByProfiles ) / 2. ;
    dDistByProfiles = ( cRightEdgeByProfiles.real() - cLeftEdgeByProfiles.real() ) * m_dScale_um_per_pix ;
    dDistByProfiles -= 10. ;
    dDistByProfilesErr = dDistByProfiles - m_CurrentPart.m_Cavity.m_dDistBetweenAreas_um ;
    bFoundByProfiles = ( fabs( dDistByProfilesErr ) < dMaxAbsErrorForDist_um ) ;
    cmplx LeftSideByProfiles[ 2 ] = {
      cmplx( cLeftEdgeByProfiles ) - cmplx( 0. , dYRange ) ,
      cmplx( cLeftEdgeByProfiles ) + cmplx( 0. , dYRange ) } ;
    cmplx RightSideByProfiles[ 2 ] = {
      cmplx( cRightEdgeByProfiles ) - cmplx( 0. , dYRange ) ,
      cmplx( cRightEdgeByProfiles ) + cmplx( 0. , dYRange ) } ;
    pPartialOut->AddFrame( CreateLineFrame(
      LeftSideByProfiles[ 0 ] , LeftSideByProfiles[ 1 ] , ( COLORREF ) 0 ) ) ;
    pPartialOut->AddFrame( CreateLineFrame(
      RightSideByProfiles[ 0 ] , RightSideByProfiles[ 1 ] , ( COLORREF ) 0 ) ) ;
    cmplx CentPtView( cLeftCent + cmplx( 0. , -0.6 * m_CurrentPart.m_Cavity.m_dPlaneHeight_um / m_dScale_um_per_pix ) ) ;
    pPartialOut->AddFrame( CreateTextFrame( CentPtView , "0xFFFFFF" , 10 ,
      NULL , 0 , "ByProf(%.2f,%.2f) GW=%.2fum" ,
      cCavCentByProfiles.real() , cCavCentByProfiles.imag() , dDistByProfiles ) ) ;
  }

  if (!DistErr.IsEmpty() || !AngleErr.IsEmpty() || !DeltaYMsg.IsEmpty())
  {
    if (!( bFoundByProfiles && m_CurrentPart.m_Cavity.m_bCavEnableProfileUsing ) || !DeltaYMsg.IsEmpty())
    {
      pPartialOut->AddFrame( FormTextFrameForView(
        cViewPt , 22 , "0x0000ff" ,
        "%s%s%s" , ( LPCTSTR ) DistErr , ( LPCTSTR ) AngleErr , ( LPCTSTR ) DeltaYMsg ) );
      return pPartialOut; // side lines are not parallel
    }
  }

  StraightLineRegression SRegrExtLeft , SRegrExtRight;

  CLine2d SLeftExtLine = GetRegressionNearPoint( *pFig1 , cCentFigLeft , false ,
    dHalfWidth , pPartialOut , SRegrExtLeft ) ;
  CLine2d SRightExtLine = GetRegressionNearPoint( *pFig2 , cCentFigRight , true ,
    dHalfWidth , pPartialOut , SRegrExtRight ) ;
  cmplx cCalcExtLeft = SLeftExtLine.GetPtForY( cCentFigLeft.imag() );
  cmplx cCalcExtRight = SRightExtLine.GetPtForY( cCentFigRight.imag() );
  double dW_pix = cCalcExtRight.real() - cCalcExtLeft.real() ;
  double dW_um = dW_pix * m_dScale_um_per_pix ;

  bool bOK = !bDistErr ;
  if (bDistErr && bFoundByProfiles && m_CurrentPart.m_Cavity.m_bCavEnableProfileUsing)
  {
    dGW_um = dDistByProfiles ;
    dNormAvError = ( dDistByProfiles - m_CurrentPart.m_Cavity.m_dDistBetweenAreas_um )
      / m_CurrentPart.m_Cavity.m_dDistBetweenAreas_um ;
    cCavCent = cCavCentByProfiles ;

    bOK = true ;
  }
  DistErr.Format( "UpEr=%.1f DownEr=%.1f\n AvErr=%.2f [%.2f] um\n"
    "\nFL=%.1fum\nW=%.1fum\nGW=%.1fum\n\n%s\n" ,
    dUpperError , dLowerError , dNormAvError * m_CurrentPart.m_Cavity.m_dDistBetweenAreas_um ,
    dMaxAbsErrorForDist_um , cAvCavitySize_um.imag() , dW_um , dGW_um ,
    bOK ? ( ( bDistErr ) ? "By Profiles" : "By Contours" ) : "No Success" ) ;
  pPartialOut->AddFrame( FormTextFrameForView(
    cViewPt , 18 , bOK ? "0x00ff00" : "0x0000ff" ,
    "%s%s" , ( LPCTSTR ) DistErr , ( LPCTSTR ) AngleErr ) );

  cmplx cLeftVect = cCalcLeftLower - cCalcLeftUpper;
  cmplx cRightVect = cCalcRightLower - cCalcRightUpper;

  cmplx cAverVect = ( cLeftVect + cRightVect ) / 2.;
  if (abs( cAverVect ) / abs( cLeftVect ) < 0.5)
    cAverVect = ( cLeftVect - cRightVect ) / 2.;

  cmplx cLeftXSelectionPt = cHorizLeftUpper
    + ( cHorizLeftLower - cHorizLeftUpper ) * ( 0.5 + m_CurrentPart.m_Cavity.m_dRelativeHeightForXSampling ) ;
  cmplx cRightXSelectionPt = cHorizRightUpper
    + ( cHorizRightLower - cHorizRightUpper ) * ( 0.5 + m_CurrentPart.m_Cavity.m_dRelativeHeightForXSampling ) ;
  CLine2d XSamplingLine( cLeftXSelectionPt , cRightXSelectionPt ) ;
  CLine2d LeftVertInternalEdge( cCalcLeftLower , cCalcLeftUpper ) ;
  CLine2d RightVertInternalEdge( cCalcRightLower , cCalcRightUpper ) ;
  cmplx cLeftPointSampling , cRightPointSampling ;
  bool bCrossLeft = XSamplingLine.intersect( LeftVertInternalEdge , cLeftPointSampling ) ;
  bool bCrossRight = XSamplingLine.intersect( RightVertInternalEdge , cRightPointSampling ) ;
  cmplx cSamplingX = ( !bDistErr ) ?
    ( cLeftPointSampling + cRightPointSampling ) * 0.5 : cCavCent ;


  cmplx cUpperPt = cCavCent + cAverVect * 1.0;
  cmplx cLowerPt = cCavCent - cAverVect * 1.0;
  CLine2d XLine( cUpperPt , cLowerPt );
  CLine2d YLineLower( cHorizLeftLower , cHorizRightLower );
  CLine2d YLineUpper( cHorizLeftUpper , cHorizRightUpper );
  cmplx cCrossXYUpper , cCrossXYLower;
  cmplx cCrossXY;

  CLine2d CentLine( cLeftCent , cRightCent );
  cmplx cCrossCent;
  bool bCrossU = XLine.intersect( YLineUpper , cCrossXYUpper );
  bool bCrossL = XLine.intersect( YLineLower , cCrossXYLower );
  bool bCrossC = XLine.intersect( CentLine , cCrossCent ) ;
  m_cLastCent_um = cCrossCent;
  m_cLastUpper_um = cCrossXYUpper;
  m_cLastLower_um = cCrossXYLower;

  switch (m_CurrentPart.m_Cavity.m_CavityEdge)
  {
    case CavEdge_Lower:
      {
        cCrossXY = cCrossXYLower;
        cCrossXY._Val[ _RE ] = cSamplingX.real();
        m_bLastCavityResult = bOK;
        if (m_ViewDetails >= 2)
        {
          CFigureFrame * pBottomLine = CreateLineFrame( cHorizLeftLower , cHorizRightLower , 0x0000ff );
          pPartialOut->AddFrame( pBottomLine );
        }
      }
      break;
    case CavEdge_Lower_Xc:
      {
        cCrossXY = cCrossXYLower;
        cCrossXY._Val[ _RE ] = cSamplingX.real();
        m_bLastCavityResult = bOK;
        //       if ( bCrossC )
        //       {
        //         cCrossXY._Val[ _RE ] = cCrossCent.real();
        //       }
        if (m_ViewDetails >= 2)
        {
          CFigureFrame * pBottomLine = CreateLineFrame( cHorizLeftLower , cHorizRightLower , 0x0000ff );
          pPartialOut->AddFrame( pBottomLine );
          CFigureFrame * pCrossPt = CreateFigureFrame( &cCrossCent , 1 , ( DWORD ) 0xffff00 );
          *( pCrossPt->Attributes() ) += _T( "Sz=2;thickness=3;" );
          pPartialOut->AddFrame( pCrossPt );
        }
      }
      break;
    case CavEdge_Upper:
      {
        cCrossXY = cCrossXYUpper;
        m_bLastCavityResult = bOK;
        cCrossXY._Val[ _RE ] = cSamplingX.real() ;
        if (m_ViewDetails >= 2)
        {
          CFigureFrame * pUpperLine = CreateLineFrame( cHorizLeftUpper , cHorizRightUpper , 0x0000ff );
          pPartialOut->AddFrame( pUpperLine );
        }
      }
      break;
    case CavEdge_Upper_Xc:
      {
        cCrossXY = cCrossXYUpper;
        cCrossXY._Val[ _RE ] = cSamplingX.real();
        m_bLastCavityResult = bOK;
        //       if ( bCrossC )
        //       {
        //         cCrossXY._Val[ _RE ] = cCrossCent.real();
        //       }
        if (m_ViewDetails >= 2)
        {
          CFigureFrame * pUpperLine = CreateLineFrame( cHorizLeftUpper , cHorizRightUpper , 0x0000ff );
          pPartialOut->AddFrame( pUpperLine );
          CFigureFrame * pCrossPt = CreateFigureFrame( &cCrossCent , 1 , ( DWORD ) 0xffff00 );
          *( pCrossPt->Attributes() ) += _T( "Sz=2;thickness=3;" );
          pPartialOut->AddFrame( pCrossPt );
        }
      }
      break;
    case CavEdge_LowerAndUpper:
      //     if ( bCrossL && bCrossU )
      {
        cmplx cVectLtoU = cCrossXYUpper - cCrossXYLower;
        cCrossXY = cCrossXYLower + cVectLtoU * m_dYCenterRelativelyToLowerEdge;
        m_bLastCavityResult = bOK;
        if (m_ViewDetails >= 2)
        {
          CFigureFrame * pBottomLine = CreateLineFrame( cHorizLeftLower , cHorizRightLower , 0x0000ff );
          pPartialOut->AddFrame( pBottomLine );
          CFigureFrame * pUpperLine = CreateLineFrame( cHorizLeftUpper , cHorizRightUpper , 0x0000ff );
          pPartialOut->AddFrame( pUpperLine );
        }
      }
      break;
  }

  m_cLastCentAver_um += cmplx( m_cLastCent_um.real() , cCrossXY.imag() ) - m_cLastMeasCenter;
  m_cLastUpperAver_um += cmplx( m_cLastUpper_um.real() , cCrossXY.imag() ) - m_cLastMeasCenter;
  m_cLastLowerAver_um += cmplx( m_cLastLower_um.real() , cCrossXY.imag() ) - m_cLastMeasCenter;

  if (m_ViewDetails >= 2)
  {
    CFigureFrame * pCenterLine = CreateLineFrame( cUpperPt , cLowerPt , 0x0000ff );
    pPartialOut->AddFrame( pCenterLine );
    CFigureFrame * pCrossPt = CreateFigureFrame( &cCrossXY , 1 , ( DWORD ) 0xffffff );
    *( pCrossPt->Attributes() ) += _T( "Sz=3;thickness=3;" );
    pPartialOut->AddFrame( pCrossPt );
  }
  FXString XAsString;
  cmplx cRelToCenter = cCrossXY - /* m_cLastROICenter*/ m_cLastMeasCenter;
  m_cLastExtractedResult_pix = cRelToCenter;
  cmplx cRelToCenter_um = ConvertCoordsRelativeToCenter( cRelToCenter );
  m_cLastCavityXYResult_um = cRelToCenter_um;
  if (m_ViewDetails >= 3)
  {
    XAsString.Format( "Ang=%.2f Deg " , RadToDeg( -arg( cAverVect ) ) );
    cmplx cXTextPt = m_cLastROICenter + cmplx( 300. , -m_cLastROICenter.imag() * 0.9 );
    CTextFrame * pXShow = CreateTextFrame( cXTextPt , ( LPCTSTR ) XAsString ,
      "0xffc000" , 20 );
    pPartialOut->AddFrame( pXShow );
    pPartialOut->AddFrame( CreatePtFrame( cCalcExtLeft , GetHRTickCount() , 0xff0000 ) ) ;
    pPartialOut->AddFrame( CreatePtFrame( cCalcExtRight , GetHRTickCount() , 0xff0000 ) ) ;
    CFigureFrame * pCrossPt = CreatePtFrame( cSamplingX , GetHRTickCount() , 0x00ff00 );
    *( pCrossPt->Attributes() ) += _T( "Sz=10;" );

    pPartialOut->AddFrame( pCrossPt );
  }
  return pPartialOut;
}


CDataFrame * MPPT::ProcessUplook( const CDataFrame * pDataFrame )
{

  return NULL;
}

CDataFrame * MPPT::ProcessUplookSide( const CDataFrame * pDataFrame )
{

  return NULL;
}

// Command list:
//   All received strings will be converted to low case
//   and will be analysed after such convertion.
//   When necessary, some strings will be converted to upper case
//   For example names of parts
//
// Common commands:
//   restart - 
//            1. Counters reset, 
//            2. Caption strings out to log files, 
//            3. Send "OK" to Engine
//            4. Stop current sequences
//   stop    - simple stop all current sequences
//   reset_counters - the same with restart
//   grab    - works when gadget in state IDLE, LONGSWEEP, SHORTSWEEP;
//             send to camera command for image grabbing
//   takeshot - the same with grab command
//   wedge   - select wedge parameters for future processing
//             command format "wedge=<wedge name>"
//             if wedge is known the gadget sends "OK" to Engine 
//             if wedge is unknown the gadget sends to Engine message 
//                "Error; No such part"
//    done        - there is external notification about ordered operation finishing
//                  Usually this is notification about machne motion finishing
//                  Gadget continues processing
//   continue_process - continue process after stabilization waiting. 
//            if ( m_bWaitForContinueCommand != 0)
//              m_bWaitForContinueCommand = 2 ;
//
//  Commands for Down looking mode (DL gadget working mode or MPPTWM_Down):
//    setlasermode - switch on laser and set proper exposures for pn and cavity
//    setcavitymode - switch off laser, set gadget for cavity XY measurements,
//                    select proper exposure, switch on low pass filter
//    domeasure   - initiate cavity measurement sequence
//                  Command format "domeasure [partname=<part name>;]
//                  if substring "partname=<par name>;" exists, correspondent 
//                  part will be selected for processing,
//                  If there is no such part, message "Error; No such part" will be sent to Engine.
//                  Part parameters will be loaded
//                  If processed cavities counter is zero, caption will be written to log file
//                  gadget will be switched to cavity XY measurement mode and image grab initiated
//    docalibration - Down looking camera calibration process will be initiated
//                  Current part parameters will be loaded
//                  For height measurements with laser: 
//                    Laser will be switched on, exposure will be settled for pin measurements
//                    Gadget will begin XY calibration which will be followed by laser calibration
//                  For Height measurement by defocus:
//                    Pin Z measurement will be initiated
//                    If "Parameters->DoPinXYCalibration" flag is 1, the pin XY calibration will be done after Pin Z measurement
//                    If "Parameters->DoPinXYCalibration" flag is 0, the pin XY calibration will be omitted
//                  Pin diameter measurement will be done after calibrations
//                  Pin contact point (usually "lower" pin edge on image) will be measured and process will be finished
//                  The last machine position will be "contact point".
//    dozmeasure  - for internal code debugging
//    dopinonlycalib - Pin diameter measurement will be done after calibrations
//                  Pin contact point (usually "lower" pin edge on image) will be measured and process will be finished
//                  The last machine position will be "contact point".
//    addcavity   - current cavity parameters will be saved under current name in setup dialig
//                  if cavity exists, saved before parameters will be overwritten
//                  If word "reset" following command, cavity parameters width, length and area will be reset
//                  If sequence "partname=<Part Name>;" following, part with correcpondent name 
//                    will be created or overwritten
//    liveview    - switch on live view in ordered mode
//                  Format is "liveview <mode>"
//          the modes are:
//        tip        - pin center will be measured ("OK" will be sent to Engine")
//        tipnorth   - pin upper extreme will be measured ("OK" will be sent to Engine")
//        tipeast    - pin right extreme will be measured ("OK" will be sent to Engine")
//        tipsouth   - pin lower extreme will be measured ("OK" will be sent to Engine")
//        tipwest    - pin left extreme will be measured ("OK" will be sent to Engine")
//        cavity     - cavity XY position will be measured ("OK" will be sent to Engine")
//        cavityapex - cavity apex focus will be measured ("OK" will be sent to Engine")
//        laser      - cavity focus value will be measured 
//        focusmeasure - cavity focus value will be measured ("OK" will be sent to Engine")
//        pinfocus    - pin focus value will be measured ("OK" will be sent to Engine")
//   lighton      - constant light will be switched on
//                  format "lighton[=<mask>]
//                  If there is no mask, all lights will be on (straight and ring)
//                  Is bit 0 of mask is 1, ring light will be switched on
//                  Is bit 1 of mask is 1, straight light will be switched on
//   lightoff     - constant light will be switched off
//                  format "lightoff[=<mask>]
//                  If there is no mask, all lights will be off (straight and ring)
//                  Is bit 0 of mask is 1, ring light will be switched off
//                  Is bit 1 of mask is 1, straight light will be switched off
//
//  Commands for Up front mode (blank tip front measurement; MPPTWM_UpFront gadget mode):
//  correct_right_side - put blank right edge to side camera focal plane
//  measure_and_correct - measure blank position and put blank into predefined point.
//                     This is main command for blank position measurement and position correction
//  do_xycalibration - initiate XY calibration for front looking camera
//  domeasure - ULF and ULS are using the same LAN server. This command is going to ULS for processing.
//              ULF will do simple image grabbing and showing without processing.
//    liveview    - switch on live view in ordered mode
//                  Format is "liveview <mode>"
//          the modes are:
//        bottom     - blank bottom will be measured ("OK" will be sent to Engine")
//
//  Commands for Side measurement (blank tip side measurement; MPPTWM_UpSide gadget mode):
//  correct_right_side - put blank right edge to side camera focal plane (used for side looking 
//                       camera calibration and sent for this purpose)
//  measure_and_correct - measure blank position and put blank into predefined point.
//                     This is main command for blank position measurement and position correction
//  do_xycalibration - initiate XY calibration for front looking camera
//  domeasure - Initiate blank measurement. First of all blank will be adjusted to necessary height
//              and after Up Looking camera will do measure blank XY position ( command "measure_and_correct"
//              will be sent to ULF.
//  docalibration - Vertical scale calibration (Z) will be done and command for 
//                  up looking camera XY calibration will be sent to ULF
//  calib_zscale  - command for Z scale calibration. This command sent after up looking camera does blank X adjust to side looking camera focal plane.
//    liveview    - switch on live view in ordered mode
//                  Format is "liveview <mode>"
//          the modes are:
//        side     - blank side will be measured ("OK" will be sent to Engine")




bool MPPT::ProcessMPPTCommand( const CTextFrame * pCommand )
{
  if (pCommand)
  {
    FXString Label = pCommand->GetLabel();
    FXPropertyKit Command( pCommand->GetString() );
    FXString ForLog;
    ForLog.Format( "ProcessMPPTCommand entry: Lab=%s Command=%s" ,
      ( LPCTSTR ) Label , ( LPCTSTR ) Command );
    SaveLogMsg( ForLog ) ;

    if (Label.Find( _T( "Timeout" ) ) == 0)// Check for timeout in scan process
    {
      SEND_GADGET_ERR( "Timeout on image grab, New grab initiated" );
      GrabImage();
      return true;
    }

    Command.MakeLower();
    Command.Trim( " \t\n\r" );
    int iSeparatorIndex = ( int ) Command.Find( ':' );
    if (iSeparatorIndex > 0)
    {
      Label = Command.Left( iSeparatorIndex );
      Command.Delete( 0 , iSeparatorIndex + 1 );
    }
    MPPT_State NewState = State_Idle;

    if (Command == _T( "restart" ))
    {
      m_iNProcessedCavities = 0;
      m_iNProcessedBlanks = 0;
      m_iNZMeasured = 0;
      m_PlotSampleCntr = 0 ;
      m_bStabilizationDone = false ;
      GetPatternPoint();
      SaveLogMsg( "Restart" );
      switch (m_WorkingMode)
      {
        case MPPTWM_Down:
          SaveCSVLogMsg( "Restart\nTime Stamp              ,   Cav#, dX um,  dY um, dZ um, dZAver , dZleft , dZRight , Stat" );
          break;
        case MPPTWM_UpFront:
          SaveCSVLogMsg( "Restart\nTime Stamp              , Blank#, dX um, dY um , dZ um, Stat" );
          break;
      }
      SendMessageToEngine( "OK" , "RestartReaction" );
      m_WorkingState = State_Idle;
      NewState = State_Stopped;
      return true;
    }
    else if (Command == _T( "stop" ))
    {
      switch (m_WorkingMode)
      {
        case MPPTWM_Down: NewState = m_WorkingState = State_Idle; break;
        case MPPTWM_UpFront:
          NewState = m_WorkingState = State_Idle;
          break;
        case MPPTWM_UpSide: NewState = m_WorkingState = State_Idle; break;
      }
      DeleteWatchDog();
      //SwitchOffConstantLight(true, true);
      return true;
    }
    else if (Command == _T( "reset_counters" ))
    {
      m_iNProcessedCavities = 0;
      m_iNProcessedBlanks = 0;
      m_iNZMeasured = 0;
      m_PlotSampleCntr = 0 ;
      m_bStabilizationDone = false ;
      GetPatternPoint();
      SaveLogMsg( "Reset Counters" );
      switch (m_WorkingMode)
      {
        case MPPTWM_Down:
          SaveCSVLogMsg( "Restart\nTime Stamp              ,  Cav#, dX um,  dY um, dZ um, dZAver , dZleft , dZRight , Stat" );
          break;
        case MPPTWM_UpFront:
          SaveCSVLogMsg( "Reset Counters\nTime Stamp              , Blank#, dX um, dY um , dZ um, Stat" );
          break;
      }
      SendMessageToEngine( "OK" , "ResetCountersReaction" );
      return true;
    }
    else if (Command == "grab" || Command == "takeshot")
    {
      if (m_WorkingState == State_Idle
        || m_WorkingState == DL_LongSweep
        || m_WorkingState == DL_ShortSweep
        || m_WorkingState == ULS_Unknown
        || m_WorkingState == ULF_Unknown)
      {
        GrabImage();
        NewState = State_ShortCommand ;
        return true;
      }
    }
    else if (Command.Find( "wedge" ) == 0)
    {
      FXSIZE iPos = Command.Find( '=' );
      if (iPos > 0)
      {
        FXString PartName = Command.Mid( iPos + 1 ).Trim( " \t;," ) ;
        RestoreKnownParts( PartName , true );
        return true;
      }
      SendMessageToEngine( "Error; Bad format (expected 'wedge=<wedge type>')" ,
        ( LPCTSTR ) ( m_GadgetName + _T( "_AnswerForWedge" ) ) );
      SEND_GADGET_ERR( "Bad format in string %s  (expected 'wedge=<wedge type>')" , ( LPCTSTR ) Command );
      return false;
    }
    else if (Command.Find( "continue_process" ) == 0)
    {
      ProcessContinueCommand() ;
      return true ;
    }
    else
    {
      m_bMeasureFocus = false;
      switch (m_WorkingMode)
      {
        case MPPTWM_Down:
          {
            if (Command == _T( "setlasermode" ))
            {
              NewState = State_ShortCommand;
              SetLaserMode( LEM_Unknown );
            }
            else if (Command == _T( "setcavitymode" ))
            {
              NewState = State_ShortCommand;
              SetCavityMode();
            }
            else if (Command.Find( _T( "domeasure" ) ) == 0)
            {
              m_PreviousState = State_Idle;
              FXString PartName;
              if (Command.GetString( "partname" , PartName ))
              {
                if (!SelectCurrentPart( PartName ))
                {
                  SendMessageToEngine( "Error; No such part" , "ReactionForWedge" );
                  SEND_GADGET_ERR( "Unknown part name %" , ( LPCTSTR ) PartName );
                }
              }
              LoadAndUpdatePartParameters();

              //             int iMinWidth = ROUND(m_CurrentPart.m_dPlaneWidth_um * 0.75) ;
              //             int iMaxWidth = ROUND( m_CurrentPart.m_dPlaneWidth_um * 1.25 ) ;
              //             int iMinHeight = ROUND( m_CurrentPart.m_dPlaneHeight_um * 0.80 ) ;
              //             int iMaxHeight = ROUND( m_CurrentPart.m_dPlaneHeight_um * 1.15 ) ;
              //             FXString Params ;
              //             Params.Format( "name=cavity_bottom;width_min=%d;height_min=%d;"
              //               "width_max=%d;height_max=%d;" , iMinWidth ,
              //               iMinHeight , iMaxWidth , iMaxHeight ) ;
              //             SetParametersToTVObject( Params ) ;

              NewState = m_WorkingState = DL_MeasCavityXY;
              FXRegistry Reg( "TheFileX\\Micropoint" );
              BOOL bWaitStabilitzationOrder = Reg.GetRegiInt( "Parameters" , "DL_WaitForContinueCommand" , 1 );
              if (bWaitStabilitzationOrder && !m_bStabilizationDone)
              {
                SaveCSVLogMsg( "Restart\nTime Stamp              ,  Cav#, dX um,  dY um, dZ um, dZAver , dZleft , dZRight , Stat" );
                m_bWaitForContinueCommand = 1 ;
                m_dLastPlotTime = 0.;
                m_iWaitCounter = 0 ;
              }
              m_iNProcessedCavities++;
              ResetIterations();
              SetCavityMode();
              GrabImage();
            }
            else if (Command == _T( "docalibration" ))
            {
              LoadAndUpdatePartParameters();
              m_PreviousState = State_Idle;

              m_ZMethod = AnalyzeAndGetDLMeasurementZMethod();
              switch (m_ZMethod)
              {
                case DLZ_Laser:
                  ResetIterations();
                  m_iCalibCntr = -1;
                  m_bPinOnlyCalib = false;
                  m_NewCalibData.clear();
                  m_NewCalibMatrixSize = m_CalibMatrixSize;
                  NewState = m_WorkingState = DL_ScaleCalib;
                  SetPinMode();
                  GrabImage();
                  break;
                case DLZ_NoZMeasurement:
                case DLZ_Defocusing:
                case DLZ_LongSweep:
                case DLZ_ShortSweep:
                  NewState = m_WorkingState = DL_PinToCenterForZ;
                  SetPinMode();
                  GrabImage();
                  break;
                default:
                  {
                    SendMessageToEngine( "Result=Error; "
                      "Unknown Z measurement method" , "EndOfCalibration" ) ;
                    m_WorkingState = State_Idle;
                  }
                  break;
              }
            }
            else if (Command == _T( "dozmeasure" ))
            {
              m_PreviousState = State_Idle;
              NewState = m_WorkingState = ( m_iNCavityZMeasurements < 2 ) ?
                DL_MeasCavityZ : DL_MeasCavityZMultiple;
              ResetIterations();
              SetLaserMode( LEM_ForCavity );
              GrabImage();
            }
            else if (Command == _T( "dopinonlycalib" ))
            {
              m_PreviousState = State_Idle;
              LoadAndUpdatePartParameters();
              NewState = m_WorkingState = DL_PinNorthSide;
              ResetIterations();
              m_iCalibCntr = -1;
              m_bPinOnlyCalib = true;
              m_NewCalibData.clear();
              SendDisplaceCommand( 0. , m_dPinDiam_um / 2. , 0. );
              m_ShiftsDL.m_x = 0.;
              m_ShiftsDL.m_y = GetBoundToTenth( m_dPinDiam_um / 2. ) ;
            }
            else if (Command == _T( "takeshot" ))
            {
              if (m_WorkingState == DL_LongSweep
                || m_WorkingState == DL_ShortSweep)
              {
                GrabImage();
                NewState = m_WorkingState;
              }
              else
              {
                SEND_GADGET_ERR( "TakeShot command is NOT EXPECTED. State = %s" ,
                  GetWorkingStateName() );
                SendMessageToEngine( "Result=Error; TakeShot command is NOT EXPECTED" ,
                  "ErrorMsg" );
              }
            }
            else if (Command.Find( _T( "addcavity" ) ) == 0)
            {
              FXString PartName;
              if (Command.Find( _T( "reset" ) ) > 0)
              {
                m_CurrentPart.m_Cavity.m_iCentralZoneWidth_pix = m_iCentralZoneWidthForX;
                m_CurrentPart.m_Cavity.m_dPlaneArea_um2 =
                  m_CurrentPart.m_Cavity.m_dPlaneHeight_um =
                  m_CurrentPart.m_Cavity.m_dPlaneWidth_um = 0;
              }
              else
              {
                if (Command.GetString( "partname" , PartName ))
                  m_PartName = PartName;
                CheckAndAddCavity();
              }
            }
            else if (Command == _T( "done" ))
            {
              m_DoneTimeStamp = GetTimeAsString_ms();
              FXString sOldState = GetWorkingStateName() ;
              NewState = m_WorkingState; // protection against log messages
              switch (m_WorkingState)
              {
                case State_Idle: NewState = State_ShortCommand; break;
                case DL_ScaleCalib:
                  GrabImage();
                  break;
                case DL_PinNorthSide:
                  SetPinEdgeMode( ED_UP );
                  GrabImage();
                  break;
                case DL_PinEastSide:
                  SetPinEdgeMode( ED_RIGHT );
                  GrabImage();
                  break;
                case DL_PinSouthSide:
                  SetPinEdgeMode( ED_DOWN );
                  GrabImage();
                  break;
                case DL_PinWestSide:
                  SetPinEdgeMode( ED_LEFT );
                  GrabImage();
                  break;
                case DL_LaserCalib:
                  SetLaserMode( LEM_ForPin );
                  GrabImage();
                  break;
                case DL_CavityJumpForZ:
                  m_cCavityZMeasurementShift_um = 0.;
                  m_ZMeasurements.clear();
                  if (m_iNCavityZMeasurements < 2)
                    m_WorkingState = DL_MeasCavityZ;
                  else
                  {
                    m_WorkingState = DL_MeasCavityZMultiple;
                    m_cCavityZMeasurementStep_um =
                      2. * m_cLastRightCavitySize_um / ( 3. * m_iNCavityZMeasurements );
                  }
                  GrabImage();
                  break;
                case DL_FinalImageOverPin:
                case DL_CorrectAfterFinalVision:
                case DL_LaserCalibFinished:
                  {
                    EDGE_DIR Dir = ( ( m_CurrentPart.m_Cavity.m_CavityEdge == CavEdge_Upper_Xc )
                      || ( m_CurrentPart.m_Cavity.m_CavityEdge == CavEdge_Upper ) ) ?
                      ED_DOWN : ED_UP ;
                    SetPinEdgeMode( Dir );
                    GrabImage();
                    //NewState = m_WorkingState = DL_FinalImageOverPin ;
                  }
                  break;
                case DL_MoveToCorrectZ:
                  {
                    NewState = m_WorkingState = DL_CaptureZWithCorrectHeigth;
                    SetLaserMode( LEM_ForCavity );
                    GrabImage();
                  }
                  break;
                case DL_MoveCavityForFinalImage:
                  {
                    NewState = m_WorkingState = DL_CaptureCavityFinalImage;
                    SetCavityMode();
                    GrabImage();
                    ResetIterations();
                    SaveCSVLogMsg( "  %3d,%6.2f,%6.2f,%6.2f,%6.2f,%6.2f,%6.2f,OK %s" , m_iNProcessedCavities ,
                      m_cLastCavityXYAveraged_um.real() ,
                      m_cLastCavityXYAveraged_um.imag() ,
                      m_dZUsedAsResult ,
                      m_dAverage_dZ , m_dAverageLeft_dZ ,
                      m_dAverageRight_dZ , ( LPCTSTR ) m_Info1 );
                    SaveLogMsg( "Cavity %d final result - %.2f,%.2f,%.2f,%6.2f,%6.2f,%6.2f,OK %s" ,
                      m_iNProcessedCavities ,
                      m_cLastCavityXYAveraged_um.real() ,
                      m_cLastCavityXYAveraged_um.imag() ,
                      m_dZUsedAsResult ,
                      m_dAverage_dZ , m_dAverageLeft_dZ ,
                      m_dAverageRight_dZ , ( LPCTSTR ) m_Info1 );
                    if (m_bWaitForContinueCommand == 2)
                    {
                      m_bStabilizationDone = true ;
                      m_bWaitForContinueCommand = 0;
                    }
                  }
                  break;
                case DL_PinToCenterForZ:
                case DL_PinZMeasureByDefocus:
                case DL_PinZDefocusToPlus:
                case DL_PinZDefocusToMinus:
                case DL_DefocusToMinus:
                case DL_DefocusToPlus:
                case DL_MeasZByDefocus: // for sweep mode, first of all measure defocus value
                                        // and then tell about readiness for sweep
                case DL_MeasZAfterExpAdjust:
                  {
                    GrabImage();
                    break;
                  }
                case State_AddMotion:
                  SendDisplaceCommand( m_TheRestShift ) ;
                  break ;
                case DL_BadCavityOnDefocusing:
                  SendMessageToEngine( "Result=BadCavity; Bad defocus" , "ZbyDefocus" );
                  m_WorkingState = State_Idle;
                  break;
                default:
                  NewState = State_Idle;  // Motion finished
              }
              SaveOperativeLogMsg( "%s: DONE Msg, OldState=%s NewState=%s" ,
                GetShortWorkingModeName() , ( LPCTSTR ) sOldState , GetWorkingStateName() );
            }
            else if (Command.Find( "viewhandle" ) == 0)
            {
              FXPropertyKit pk( Command ) ;
              FXString TargetAsString ;
              if (pk.GetString( "h" , TargetAsString ))
              {
                CTextFrame * pText = CreateTextFrame( ( LPCTSTR ) TargetAsString , "SetWndHandle" ) ;
                PutFrame( m_pOutput , pText ) ;
                Sleep( 100 );
                SendMessageToEngine( "OK; Handle for DL image is received" , "DL_ViewHandle" );
              }
              return NULL ;
            }
            else if (Command.Find( "liveview" ) == 0)
            {
              Command.Delete( 0 , 9 );
              Command.Trim();
              FXString ViewMode;
              int iExpChange_perc = 0 ;
              FXPropertyKit pk( Command );
              if (pk.GetString( "viewmode" , ViewMode ))
              {
                if (ViewMode == "tip")
                {
                  FXRegistry Reg( "TheFileX\\Micropoint" );
                  LoadAndUpdatePartParameters();
                  NewState = m_WorkingState = DL_LiveVideoPinMeasure;
                  SetPinMode( true );
                  m_WhatSideToUse = m_WhatSideToUseForCavity;
                  ResetIterations();
                  SendMessageToEngine( "OK" , "Tip View" );
                  GrabImage();
                  return true;
                }
                else if (ViewMode == "tipnorth")
                {
                  NewState = m_WorkingState = DL_LiveVideoPinNorth;
                  SetPinEdgeMode( ED_UP , true );
                  ResetIterations();
                  SendMessageToEngine( "OK" , "Tip View" );
                  GrabImage();
                  return true;
                }
                else if (ViewMode == "tipeast")
                {
                  NewState = m_WorkingState = DL_LiveVideoPinEast;
                  SetPinEdgeMode( ED_RIGHT , true );
                  ResetIterations();
                  SendMessageToEngine( "OK" , "Tip View" );
                  GrabImage();
                  return true;
                }
                else if (ViewMode == "tipsouth")
                {
                  NewState = m_WorkingState = DL_LiveVideoPinSouth;
                  SetPinEdgeMode( ED_DOWN , true );
                  ResetIterations();
                  SendMessageToEngine( "OK" , "Tip View" );
                  GrabImage();
                  return true;
                }
                else if (ViewMode == "tipwest")
                {
                  NewState = m_WorkingState = DL_LiveVideoPinWest;
                  SetPinEdgeMode( ED_LEFT , true );
                  ResetIterations();
                  SendMessageToEngine( "OK" , "Tip View" );
                  GrabImage();
                  return true;
                }
                else if (ViewMode == "cavity")
                {
                  LoadAndUpdatePartParameters();
                  m_bMeasureFocus = false;
                  NewState = m_WorkingState = DL_LiveVideoCavity;
                  SetCavityMode( true );
                  m_WhatSideToUse = m_WhatSideToUseForCavity;
                  ResetIterations();
                  SendMessageToEngine( "OK" , "Cavity View" );
                  GrabImage();
                  return true;
                }
                else if (ViewMode == "cavityapex")
                {
                  FXRegistry Reg( "TheFileX\\Micropoint" );
                  FXString ApexAsText = Reg.GetRegiString(
                    "Parameters" , "ApexZArea" , "920,500,1000,700" );
                  CRect ApexRect;
                  int iNLen = Reg.GetRegiIntSerie( "Parameters" , "ApexZArea" ,
                    ( int* ) &ApexRect , 4 );
                  m_LeftFocusMeasRect = ApexRect;
                  m_RightFocusMeasRect = CRect( 0 , 0 , 0 , 0 );
                  m_bMeasureFocus = false;
                  SetCavityMode( false );
                  NewState = m_WorkingState = DL_LiveApexZView;
                  ResetIterations();
                  SendMessageToEngine( "OK" , "Cavity Apex Z View" );
                  GrabImage();
                  return true;
                }
                else if (ViewMode == "laser")
                {
                  LoadAndUpdatePartParameters();
                  NewState = m_WorkingState = DL_LiveVideoLaser;
                  SetLaserMode( LEM_Unknown );
                  ResetIterations();
                  SendMessageToEngine( "OK" , "Laser View" );
                  GrabImage();
                  return true;
                }
                else if (ViewMode == "laseron")
                {
                  SwitchLaser( true );
                  SendMessageToEngine( "OK" , "LaserOn" );
                  return true;
                }
                else if (ViewMode == "laseroff")
                {
                  SwitchLaser( false );
                  SendMessageToEngine( "OK" , "LaserOff" );
                  return true;
                }
                else if (ViewMode == "focusmeasure"
                  || ViewMode == "focus")
                {
                  LoadAndUpdatePartParameters();
                  NewState = m_WorkingState = DL_LiveVideoCavityFocus ;
                  SetCavityMode( LEM_Unknown );
                  m_Exposures.clear();
                  m_Averages.clear();
                  m_Threshs.clear();
                  m_bMeasureFocus = true;
                  m_WhatSideToUse = m_WhatSideToUseForCavity;
                  m_iNAttempts = 0;
                  ResetIterations();
                  SendMessageToEngine( "OK" , "Focus Measure View" );
                  CBooleanFrame * pLPFSwitch = CBooleanFrame::Create( true );
                  PutFrame( GetOutputConnector( 4 ) , pLPFSwitch );
                  Sleep( 30 );
                  GrabImage();
                  return true;
                }
                else if (ViewMode == "pinfocus")
                {
                  Sleep( 100 );
                  LoadAndUpdatePartParameters();
                  NewState = m_WorkingState = DL_LiveVideoPinFocusing;
                  SetPinMode();
                  m_iNAttempts = 0;
                  m_LeftFocusMeasRect = GetRectangle(
                    cmplx( -750. ) , cmplx( 300. , 400. ) );
                  m_RightFocusMeasRect = GetRectangle(
                    cmplx( 750. ) , cmplx( 300. , 400. ) );
                  m_WhatSideToUse = m_WhatSideToUseForPin;
                  m_bMeasureFocus = false;
                  ResetIterations();
                  SendMessageToEngine( "OK" , "Focus Measure View" );
                  CBooleanFrame * pLPFSwitch = CBooleanFrame::Create( false );
                  PutFrame( GetOutputConnector( 4 ) , pLPFSwitch );
                  GrabImage();
                  return true;
                }
                else
                {
                  SendMessageToEngine( "Error: unknown live view mode" ,
                    "ErrorOnView" );
                }
              }
              else if (pk.GetInt( "exposurechange" , iExpChange_perc ))
              {
                if (iExpChange_perc != 0)
                {
                  FXString PartFolder( "TheFileX\\Micropoint\\PartsData" ) ;
                  PartFolder = ( PartFolder + "\\" ) + m_PartName ;
                  m_CurrentPart.RestorePartDataFromRegistry( "TheFileX\\Micropoint\\PartsData" , m_PartName );
                  m_CurrentPart.m_Cavity.m_dNormBrightnessForCavity *= ( 1. + ( iExpChange_perc * 0.01 ) ) ;
                  Bound( m_CurrentPart.m_Cavity.m_dNormBrightnessForCavity , 0.4 , 0.995 ) ;
                  m_CurrentPart.m_Cavity.SaveCavityDataToRegistry( "TheFileX\\Micropoint\\PartsData" , m_PartName ) ;
                }
                m_bMeasureFocus = false;
                NewState = m_WorkingState = DL_LiveVideoCavity;
                SetCavityMode( true );
                m_WhatSideToUse = m_WhatSideToUseForCavity;
                ResetIterations();
                FXString ExpAsString ;
                ExpAsString.Format( "NormBrightness=%.4f" , m_CurrentPart.m_Cavity.m_dNormBrightnessForCavity ) ;
                SendMessageToEngine( "OK" , ExpAsString );
                GrabImage();
                return true ;
              }
            }
            else if (Command.Find( "lighton" ) == 0)
            {
              FXSIZE iNumPos = Command.Find( '=' , 7 );
              int iLightMask = 3;
              if (iNumPos > 0)
              {
                FXString Part = Command.Mid( iNumPos + 1 );
                iLightMask = atoi( Part.Trim() );
              }
              SwitchOnConstantLight( iLightMask & 1 , iLightMask & 2 );
              return true;
            }
            else if (Command.Find( "lightoff" ) == 0)
            {
              FXSIZE iNumPos = Command.Find( '=' , 7 );
              int iLightMask = 3;
              if (iNumPos > 0)
              {
                FXString Part = Command.Mid( iNumPos + 1 );
                iLightMask = atoi( Part.Trim() );
              }
              SwitchOffConstantLight( iLightMask & 1 , iLightMask & 2 );
              return true;
            }
            break; // not found command
          }
          if (NewState == State_Idle)
          {
            FXString Result;
            Result.Format( "Result= Unprocessed Command %s" ,
              ( LPCTSTR ) pCommand->GetString() );
            SendMessageToEngine( Result , "ToLAN" );
          }
          else
          {
          }
          return true;
        case MPPTWM_UpFront:
          {
            if (Command.Find( _T( "correct_right_side" ) ) == 0)
            {
              SetBlankMode( m_CurrentPart.m_Blank.m_iBlankExp_us );
              NewState = m_WorkingState = ULF_RightSideCorrection;
              ResetIterations();
              GrabImage();
            }
            else if (Command.Find( _T( "corr_gauge_right_side" ) ) == 0)
            {
              SetBlankMode( m_CurrentPart.m_Gauge.m_iBlankExp_us );
              NewState = m_WorkingState = ULF_RightSideCorrection;
              ResetIterations();
              GrabImage();
            }
            else if (Command.Find( _T( "measure_and_correct" ) ) == 0)
            {
              LoadAndUpdatePartParameters() ;
              if (m_WorkingState != ULF_WaitForExternalReady)
                NewState = m_WorkingState = ULF_MeasureAndCorrect;
              SetBlankMode( ( m_iNProcessedBlanks <= 0 ) ?
                m_CurrentPart.m_Gauge.m_iBlankExp_us : m_CurrentPart.m_Blank.m_iBlankExp_us );
              FXPropertyKit pk( Command );
              double dTmp = 0.;
              if (pk.GetDouble( "dz" , dTmp ))
                m_dExtdZ = dTmp;
              else
                m_dExtdZ = 0.;

              ResetIterations();
              GrabImage();
              InitZGrabForULF() ;
            }
            else if (Command == _T( "do_xycalibration" ))
            {
              NewState = m_WorkingState = ULF_MoveForScaleCalib;
              SetBlankMode( m_CurrentPart.m_Gauge.m_iBlankExp_us );
              m_ShiftsUL.Reset();
              ResetIterations();
              m_iCalibCntr = -1;
              m_NewCalibData.clear();
              m_NewCalibMatrixSize = m_CalibMatrixSize;
              GetPatternPoint();
              GrabImage();
            }
            else if (( Command == _T( "docalibrationnoxy" ) )
              || ( Command == _T( "docalibration" ) ))
            {
              LoadAndUpdatePartParameters();
              FXRegistry Reg( "TheFileX\\Micropoint" );
              m_dXY_PlotPeriod = Reg.GetRegiDouble( "Parameters" , "XY_PlotPeriod" , 1000. );
              m_dZ_PlotPeriod = Reg.GetRegiDouble( "Parameters" , "Z_PlotPeriod" , 1000. );
              m_bWaitForContinueCommand = Reg.GetRegiInt( "Parameters" , "WaitForContinueCommand" , 1 );
              m_iWaitCounter = 0 ;
              GetPatternPoint();
              m_iNProcessedBlanks = 0;
              m_cPrevMeasuredXY = cmplx( 0. , 0. ) ;
              m_dPrevMeasuredZ = 0. ;
              m_bNoXYMeasurement = ( Command == _T( "docalibrationnoxy" ) ) ;
//           if (m_bNoXYMeasurement)
//             m_bWaitForContinueCommand = FALSE;
            }
            else if (( Command == _T( "domeasurenoxy" ) )
              || ( Command == _T( "domeasure" ) ))
            {
              LoadAndUpdatePartParameters();
              SetBlankMode( m_CurrentPart.m_Gauge.m_iBlankExp_us );
          m_bNoXYMeasurement = (Command == _T("domeasurenoxy"));
              Sleep( 20 );
              GrabImage(); // Nothing to do - side camera does work
            }
            else if (Command == _T( "addjig" ))
            {
              PartParams Jig = m_CurrentPart ;
              Jig.m_Name = m_PartName ;
              Jig.m_Gauge.m_dBlankHeight_um = m_LastBlankSize.imag() ;
              Jig.m_Gauge.m_dBlankWidth_um = m_LastBlankSize.real() ;
              SavePartDataToRegistry( Jig ) ;
              CheckAndAddPart( Jig.m_Name.c_str() , Jig ) ;
              SelectCurrentPart( Jig.m_Name.c_str() ) ;
              Sleep( 20 );
              GrabImage(); // Nothing to do - side camera does work
            }
            else if (Command == _T( "done" ))
            {
              FXString sOldState = GetWorkingStateName();
              switch (m_WorkingState)
              {
                case State_Idle: NewState = State_ShortCommand; break;
                case ULF_MoveForScaleCalib:
                  m_WorkingState = ULF_ScaleCalib;
                case ULF_ScaleCalib:
                case ULF_MeasureAfterCalib:
                case ULF_MoveAfterCalib:
                case ULF_MoveAfterMeasurement:
                  {
                    SetBlankMode( ( m_WorkingState != ULF_MoveAfterMeasurement ) ?
                      m_CurrentPart.m_Gauge.m_iBlankExp_us : m_CurrentPart.m_Blank.m_iBlankExp_us );
                    ResetIterations();
                    GrabImage();
                    InitZGrabForULF() ;
                  }
                  break;
                case ULF_AfterSideCorrection:
                  {
                    //                 SetBlankMode(m_CurrentPart.m_iBlankExp_us);
                    CTextFrame * pULSOrder = CreateTextFrame( "Calib_ZScale" , "FromULF" );
                    PutFrame( GetOutputConnector( 4 ) , pULSOrder );
                    m_WorkingState = State_Idle;
                    Sleep( 10 );
                    GrabImage();
                  }
                  break;
              }
              SaveOperativeLogMsg( "%s: DONE Msg, OldState=%s NewState=%s" ,
                GetShortWorkingModeName() , ( LPCTSTR ) sOldState , GetWorkingStateName() );
            }
            else if (Command.Find( "liveview" ) == 0)
            {
              Command.Delete( 0 , 9 );
              Command.Trim();
              FXString ViewMode;
              FXPropertyKit pk( Command );
              int iExpChange_perc = 0 ;
              if (pk.GetString( "viewmode" , ViewMode ))
              {
                if (ViewMode == "bottom" || ViewMode == "gauge")
                {
                  LoadAndUpdatePartParameters();
                  NewState = m_WorkingState = ULF_LiveVideo;
                  SetBlankMode( ViewMode == "bottom" ?
                    m_CurrentPart.m_Blank.m_iBlankExp_us : m_CurrentPart.m_Gauge.m_iBlankExp_us );
                  ResetIterations();
                  SendMessageToEngine( "OK" , "Bottom View" );
                  //                 ProgramExposureAndLightParameters( m_iFrontExposure_us ,
                  //                   m_iFrontExposure_us , m_iFrontExposure_us );
                  m_PlotSampleCntr = 0 ;
                  GetPatternPoint() ;
                  GrabImage();
                }
              }
              else if (pk.GetInt( "exposurechange" , iExpChange_perc ))
              {
                if (iExpChange_perc != 0)
                {
                  FXString PartFolder( "TheFileX\\Micropoint\\PartsData" ) ;
                  PartFolder = ( PartFolder + "\\" ) + m_PartName ;
                  m_CurrentPart.RestorePartDataFromRegistry(
                    "TheFileX\\Micropoint\\PartsData" , m_PartName );
                  double dNewExp = m_CurrentPart.m_Blank.m_iBlankExp_us *
                    ( 1. + ( iExpChange_perc * 0.01 ) ) ;
                  if (fabs( m_CurrentPart.m_Blank.m_iBlankExp_us - dNewExp ) < 1.)
                  {
                    if (iExpChange_perc > 0)
                      m_CurrentPart.m_Blank.m_iBlankExp_us++ ;
                    else if (iExpChange_perc < 0)
                      m_CurrentPart.m_Blank.m_iBlankExp_us-- ;
                  }
                  else
                    m_CurrentPart.m_Blank.m_iBlankExp_us = ROUND( dNewExp ) ;

                  SavePartDataToRegistry( m_PartName , true ) ;
                }
                m_bMeasureFocus = false;
                NewState = m_WorkingState = ULF_LiveVideo;
                SetBlankMode( m_CurrentPart.m_Blank.m_iBlankExp_us );
                ResetIterations();
                FXString ExpAsString ;
                ExpAsString.Format( "Exposure=%d us" , m_CurrentPart.m_Blank.m_iBlankExp_us ) ;
                SendMessageToEngine( "OK" , ExpAsString );
                GrabImage();
                return true ;
              }
            }
            else if (Command.Find( "viewhandle" ) == 0)
            {
              FXPropertyKit pk( Command ) ;
              FXString TargetAsString ;
              if (pk.GetString( "hf" , TargetAsString ))
              {
                CTextFrame * pText = CreateTextFrame( ( LPCTSTR ) TargetAsString , "SetWndHandle" ) ;
                PutFrame( m_pOutput , pText ) ;
                Sleep( 100 );
                SendMessageToEngine( "OK; Handle for Front image is received" , "ULF_ViewHandle" );
              }
              if (pk.GetString( "hg" , TargetAsString ))
              {
                CTextFrame * pText = CreateTextFrame( ( LPCTSTR ) TargetAsString , "SetWndHandle" ) ;
                PutFrame( GetOutputConnector( 2 ) , pText );
                Sleep( 100 );
                SendMessageToEngine( "OK; Handle for Graphics is received" , "Graphics_ViewHandle" );
              }

              return NULL ;
            }
            else
              break;
          }
          return true;
        case MPPTWM_UpSide:
          {
            FXRegistry Reg( "TheFileX\\Micropoint" );
            m_dTargetZ_pix = Reg.GetRegiDouble( "Data" , "TargetZ_pix" , 717. );
            m_iSideExposure_us = Reg.GetRegiInt( "Parameters" , "SideExposure_us" , 1200 );

            if (( Command == _T( "domeasurenoxy" ) )
          || (Command == _T("domeasure")))
            {
              m_CurrentPart.RestorePartDataFromRegistry(
                "TheFileX\\Micropoint\\PartsData" , m_PartName );
              NewState = m_WorkingState = ULS_FirstZForBlank;
              m_iNProcessedBlanks++;
              ResetIterations();
              //             ProgramExposureAndLightParameters(
              //               m_iSideExposure_us , m_iSideExposure_us , 10 );
              SwitchOnConstantLight( true , false );
              ProgramExposure( m_iSideExposure_us );
              m_bNoXYMeasurement = Command == _T( "domeasurenoxy" ) ;
              GrabImage();
            }
            else if (( Command == _T( "docalibrationnoxy" ) )
              || ( Command == _T( "docalibration" ) ))
            {
              LoadAndUpdatePartParameters();
              NewState = m_WorkingState = ULS_ZCorrection;
              ResetIterations();
              //             ProgramExposureAndLightParameters(m_iSideExposure_us,
              //               m_iSideExposure_us, m_iSideExposure_us);
              SwitchOnConstantLight( true , false );
              ProgramExposure( m_iSideExposure_us );
              m_iCalibCntr = -1;
              m_bNoXYMeasurement = ( Command == _T( "docalibrationnoxy" ) ) ;
              GrabImage();
            }
            else if (Command == _T( "calib_zscale" )) // Command from ULF
            {
              FXRegistry Reg( "TheFileX\\Micropoint" );
              int iDoZCalib = Reg.GetRegiInt( "Data" , "DoULSZCalibration" , 1 );
              ResetIterations();
              GrabImage();
              if (iDoZCalib)
              {
                NewState = m_WorkingState = ULS_ScaleCalib;
                m_iCalibCntr = -1;
              }
              else
                NewState = m_WorkingState = ULS_ZCorrNoZCalib;
            }
            else if (Command == _T( "done" ))
            {
              FXString sOldState = GetWorkingStateName();
              switch (m_WorkingState)
              {
                case State_Idle: NewState = State_ShortCommand; break;
                case ULS_ScaleCalib:
                case ULS_ZCorrection:
                case ULS_ZCorrection2:
                case ULS_ZCorrectionForMeas:
                case ULS_FirstZForBlank:
                  ResetIterations();
                  GrabImage();
                  break;
                case ULS_MoveAfterCalib:
                  {
                    CTextFrame * pULFOrder = CreateTextFrame( "do_xycalibration" , "FromULS" );
                    PutFrame( GetOutputConnector( 4 ) , pULFOrder );
                    m_WorkingState = State_Idle;
                  }
                  break;
                case State_AddMotion:
                  SendDisplaceCommand( m_TheRestShift ) ;
                  break ;
              }
              SaveOperativeLogMsg( "%s: DONE Msg, OldState=%s NewState=%s" ,
                GetShortWorkingModeName() , ( LPCTSTR ) sOldState , GetWorkingStateName() );
            }
            else if (Command == _T( "grab_final" ))
            {
              m_WorkingState = ULS_GrabFinal ;
              m_dZAfterShift_um = DBL_MAX;
              m_dZStdAfterShift_um = 0.;
              ResetIterations() ;
              GrabImage() ;
            }
            else if (Command.Find( "liveview" ) == 0)
            {
              Command.Delete( 0 , 9 );
              Command.Trim();
              FXString ViewMode;
              FXPropertyKit pk( Command );
              int iExpChange_perc = 0 ;
              if (pk.GetString( "viewmode" , ViewMode ))
              {
                if (ViewMode == "side")
                {
                  LoadAndUpdatePartParameters();
                  FXRegistry Reg( "TheFileX\\Micropoint" );
                  m_dTargetZ_pix = Reg.GetRegiDouble( "Data" , "TargetZ_pix" , 717. );
                  NewState = m_WorkingState = ULS_LiveVideo;
                  ResetIterations();
                  SendMessageToEngine( "OK" , "Side View" );
                  //                 ProgramExposureAndLightParameters( m_iSideExposure_us ,
                  //                   m_iSideExposure_us , m_iSideExposure_us );
                  SwitchOnConstantLight( true , false );
                  ProgramExposure( m_iSideExposure_us );
                  GrabImage();
                }
              }
              else if (pk.GetInt( "exposurechange" , iExpChange_perc ))
              {
                //               if ( iExpChange_perc != 0 )
                //               {
                //                 FXString PartFolder( "TheFileX\\Micropoint\\PartsData" ) ;
                //                 PartFolder = (PartFolder + "\\") + m_PartName ;
                //                 m_CurrentPart.RestorePartDataFromRegistry(
                //                   "TheFileX\\Micropoint\\PartsData" , m_PartName );
                //                 double dNewExp = m_iSideExposure_us * (1. + (iExpChange_perc * 0.01)) ;
                //                 if ( fabs( m_iSideExposure_us - dNewExp ) < 1. )
                //                 {
                //                   if ( iExpChange_perc > 0 )
                //                     m_iSideExposure_us++ ;
                //                   else if ( iExpChange_perc < 0 )
                //                     m_iSideExposure_us-- ;
                //                 }
                //                 else
                //                   m_iSideExposure_us = ROUND( dNewExp ) ;
                // 
                //                 SavePartDataToRegistry( m_PartName , true ) ;
                //               }
                //               m_bMeasureFocus = false;
                NewState = m_WorkingState = ULS_LiveVideo;
                ProgramExposure( m_iSideExposure_us );
                ResetIterations();
                FXString ExpAsString ;
                ExpAsString.Format( "Side Exposure=%d us" , m_iSideExposure_us ) ;
                SendMessageToEngine( "OK" , ExpAsString );
                GrabImage();
                return true ;
              }

            }
            else if (Command.Find( "viewhandle" ) == 0)
            {
              FXPropertyKit pk( Command ) ;
              FXString TargetAsString ;
              if (pk.GetString( "hs" , TargetAsString ))
              {
                CTextFrame * pText = CreateTextFrame( ( LPCTSTR ) TargetAsString , "SetWndHandle" ) ;
                PutFrame( m_pOutput , pText ) ;
                Sleep( 100 );
                SendMessageToEngine( "OK; Handle for Side image is received" , "ULF_ViewHandle" );
              }
              return NULL ;
            }
            else
              break;
          }
          return true;
      }
    }
    SENDWARN( "Unprocessed command %s for working mode %s gadget %s" ,
      ( LPCTSTR ) Command , GetWorkingModeName() , ( LPCTSTR ) m_GadgetName );
  }
  return false;
}

CDataFrame * MPPT::DoProcessing( const CDataFrame * pDataFrame )
{
  if (m_GadgetName.IsEmpty())
    GetGadgetName( m_GadgetName );
  if (m_bUpdatePartParameters)
  {
    LoadAndUpdatePartParameters();
    m_bUpdatePartParameters = false;
  }
  if (m_bRestoreScales)
  {
    if (m_WorkingMode != MPPTWM_UpSide)
    {
      RestoreXYCalibData();
      FXString Statistics;
      CalculateScaling( &Statistics );
      SaveXYCalibData( &Statistics );
    }
    if (m_WorkingMode == MPPTWM_UpSide
      || m_WorkingMode == MPPTWM_Down)
    {
      RestoreZCalibData();
    }
    m_bRestoreScales = false;
  }

  if (!pDataFrame->IsContainer())
  {
    const CTextFrame * pCommand = pDataFrame->GetTextFrame();
    if (pCommand)
    {
      ProcessMPPTCommand( pCommand );
      return NULL;
    }
  }

  m_pCurrentImage = pDataFrame->GetVideoFrame();
  if (!m_pCurrentImage || !IsExposureAndTimingOK( m_pCurrentImage ))
    return NULL;
  DeleteWatchDog();
  m_LastROI = CRect( 0 , 0 , Width( m_pCurrentImage ) , Height( m_pCurrentImage ) );
  m_LastROICenter = m_LastROI.CenterPoint();
  m_cLastROICenter = cmplx( m_LastROICenter.x , m_LastROICenter.y );

  switch (m_WorkingMode)
  {
    case MPPTWM_Down:
    case MPPTWM_UpFront:
    case MPPTWM_UpSide:
      {
        CheckAndSaveImage( m_pCurrentImage );
      }
      break;
  }
  CContainerFrame * pOutFrame = CContainerFrame::Create();

  pOutFrame->ChangeId( pDataFrame->GetId() ) ;
  DrawStandardGraphics( pOutFrame );
  ClearConturData();

  bool bExtracted = ExtractResultAndCheckForIterationFinish( pDataFrame );
  int iNConturs = SetDataAboutContours( pDataFrame );
  int iNSpots = GetStatisticsAboutContursAsSpots( pDataFrame );


  SaveOperativeLogMsg( "%s-%s: IsExtracted=%d #Cont=%d #Spots=%d IsFocus=%d" ,
    GetShortWorkingModeName() , GetWorkingStateName() ,
    ( int ) bExtracted , iNConturs , iNSpots , m_bMeasureFocus );
  switch (m_WorkingState)
  {
    case DL_LiveVideoCavity:
    case DL_MeasCavityXY:
    case DL_CaptureCavityFinalImage:
      {
        bool bThereAreConturs = ( iNSpots && ( iNSpots == iNConturs )
          && !m_bMeasureFocus ) ? ProcessContours() : false;
        if (m_bWaitForContinueCommand && !m_bStabilizationDone)
        {
          pOutFrame->AddFrame( CreateTextFrame( cmplx( 700 , 1000 ) ,
            "0x00ffff" , 20 , "StabilizationMsg" , pDataFrame->GetId() ,
            "Stabilization is ON" ) ) ;
        }

        if (( m_WorkingState != DL_CaptureCavityFinalImage )
          && ( m_iNAttempts++ < m_iNMaxAttempts ))
        {
          if (AdjustExposureForCavity( m_pCurrentImage , pOutFrame , m_CurrentPart.m_Cavity.m_dNormBrightnessForCavity ))
          {
            //           ProgramExposureAndLightParameters( m_iNewCavityExposure , 10 , m_iNewCavityExposure );
            ProgramExposure( m_iNewCavityExposure );
            GrabImage();
            pOutFrame->AddFrame( pDataFrame );
            return pOutFrame;
          }
          m_iNAttempts = m_iNMaxAttempts;
          m_iNRestSamples = m_iAverager;
          m_IterationResults.clear();
          m_PartMeasResult.clear();
          m_PartMeasSizes.clear();
          m_IterResultLeft.clear();
          m_IterResultRight.clear();
          m_LeftCornerOnSide.clear();
          m_RightCornerOnSide.clear();
        }
        if (bThereAreConturs)
        {
          bExtracted = FilterCavitiesContours( pOutFrame );
          iNConturs = ( int ) m_CavitiesConturs.Count();
        }
        else
          iNConturs = 0;
      }
      break;
    case DL_LiveVideoCavityFocus:
      {
        if (m_iNAttempts++ < m_iNMaxAttempts)
        {
          if (AdjustExposureForFocus( m_pCurrentImage , pOutFrame ,
            m_CurrentPart.m_Cavity.m_dTargetForFocusExpAdjust ))
          {
            ProgramExposure( m_iNewFocusExposure );
            GrabImage();
            pOutFrame->AddFrame( pDataFrame );
            return pOutFrame;
          }
          m_Exposures.clear();
          m_Averages.clear();
          m_Threshs.clear();
          m_iNAttempts = m_iNMaxAttempts;
        }
      }
      break;
    case ULS_ZCorrection:
    case ULS_ZCorrection2:
    case ULS_ZCorrectionForMeas:
    case ULS_ZMeasurement:
    case ULS_FirstZForBlank:
    case ULS_LiveVideo:
    case ULS_GrabFinal:
      {
        CFigureFrame * pBlankBaseCenter =
          ( CFigureFrame* ) pDataFrame->GetFigureFrame( "blank_side_pos" );
        CFigureFrame * pBlankTip =
          ( CFigureFrame* ) pDataFrame->GetFigureFrame( "part_tip" );

        if (!pBlankTip)
        {
          if (pBlankBaseCenter)
          {
            SendDisplaceCommand( 0. , 0. , 500. );
            pOutFrame->AddFrame( CreateTextFrame( cmplx( 400. , 400. ) ,
              "Blank is inserted too deep\nI do move out for 500 microns" , "0x0000ff" , 36 ) );
          }
          else
          {
            SendMessageToEngine( "Result=BadBlank; Don't see blank from side;" , "InitialZ" );
            pOutFrame->AddFrame( CreateTextFrame( m_cLastROICenter ,
              "Blank is not measured from side" , "0x0000ff" , 20 ) );
            CheckAndSaveFinalImages( pOutFrame , true , "NotSeen" );
            m_WorkingState = State_Idle;
          }
          break;
        }

        if (m_PartConturs.Count())
        {
          cmplx LeftCorner , RightCorner ;
          int iRes = ProcessSideContours( pOutFrame , LeftCorner , RightCorner ) ;
          if (iRes > 0)
          {
            m_LeftCornerOnSide.push_back( LeftCorner ) ;
            m_RightCornerOnSide.push_back( RightCorner ) ;
          }
          else
          {
            SendMessageToEngine( "Result=BadBlank; Dirty blank(?);" , "InitialZ" );
            pOutFrame->AddFrame( CreateTextFrame( m_cLastROICenter ,
              "Blank is not measured from side" , "0x0000ff" , 20 ) );
            CheckAndSaveFinalImages( pOutFrame , true , "DirtyBlank" );
            m_WorkingState = State_Idle;
          }
        }
      }
      break ;
  }
  if (( !iNConturs && !bExtracted && !m_bMeasureFocus )
    || ( m_WorkingState == DL_MeasZByDefocus ) || ( m_WorkingState == DL_MeasZAfterExpAdjust )
    || ( m_WorkingState == DL_LiveVideoCavityFocus ))
  {
    if (m_WorkingState == DL_LiveVideoCavityFocus)
    {
      CRect Left , Right;
      bool bPartKnown = GetFocusRectanglesForCavity( Left , Right );
      if (bPartKnown)
        CalcAndAddFocusIndication( pOutFrame , &Left , &Right );
      else
        CalcAndAddFocusIndication( pOutFrame );
      if (m_iFocusLogPeriod_samples && !m_FocusLogAccumulator.IsEmpty())
      {
        SaveFocusLog( "%s" , ( LPCTSTR ) m_FocusLogAccumulator );
        m_FocusLogAccumulator.Empty();
      }
    }
    if (m_WorkingState != State_Idle)
      ProcessNoContursNoPoints( pDataFrame , pOutFrame );
    pOutFrame->AddFrame( pDataFrame );

    return pOutFrame;
  }
  LPCTSTR pDisplacementViewColor = "0x002000";
  cmplx cAverage , cStd;
  switch (m_WorkingMode)
  {
    case MPPTWM_Down:
      {
        switch (m_WorkingState)
        {
          case DL_PinToCenterForZ:
            {
              if (bExtracted)
              {
                cmplx cCentDisplacement = -ConvertCoordsRelativeToCenter(
                  m_cLastExtractedResult_pix );
                m_ShiftsDL.m_x = GetBoundToTenth( cCentDisplacement.real() );
                m_ShiftsDL.m_y = GetBoundToTenth( cCentDisplacement.imag() );
                m_ShiftsDL.m_z = 0.;
                m_cNormZMeasArea = cmplx( 1. , 1. );
                m_LeftFocusMeasRect = GetNormFocusRectangle(
                  cmplx( -750. ) , cmplx( 300. , 400. ) );
                m_RightFocusMeasRect = GetNormFocusRectangle(
                  cmplx( 750. ) , cmplx( 300. , 400. ) );
                InitZDefocusingMeasurement();
                SendDisplaceCommand( m_ShiftsDL );
                SetPinEdgeMode( ED_DEFOCUS ); // switch to pin mode with defocusing exposure
                FXRegistry Reg( "TheFileX\\Micropoint" );
                m_dZStepForHighResolutionDefocusing_um = Reg.GetRegiDouble(
                  "Parameters" , "ZStepForPinDefocusing_um" , 10. );
                m_dDefocusThreshold = Reg.GetRegiDouble(
                  "Parameters" , "DefocusingThreshold" , 0.8 );
                m_WhatSideToUse = m_WhatSideToUseForPin ;
                m_iPassCount = 0;
                m_WorkingState = DL_PinZMeasureByDefocus;
              }
            }
            break;
          case DL_ScaleCalib:
            if (bExtracted)
            {
              if (XYCalibrationProcess( pOutFrame , m_ShiftsDL ))
              {  // XY calibration is not finished
              }
              else // XY calibration finished
              {
                m_WorkingState = DL_PinNorthSide;
                SendDisplaceCommand( GetBoundToTenth( -m_ShiftsDL.m_x ) ,
                  GetBoundToTenth( -m_ShiftsDL.m_y + ( m_dPinDiam_um / 2. ) ) , 0. );
                m_ShiftsDL.m_x = 0.;
                m_ShiftsDL.m_y = GetBoundToTenth( m_dPinDiam_um / 2. ) ;
              }
            }
            else
            {
              SENDERR( "Downlook Scale Calibration: Don't see target WM=%s Step=%d" ,
                GetWorkingModeName() , m_iCalibCntr );
              SendMessageToEngine(
                "Result=Error: Can't recognize target;" , "Scaling result" );
              if (m_SaveMode == Save_Final)
                CheckAndSaveFinalImages( pOutFrame , true , "NoTarget" ) ;
              m_WorkingState = State_Idle;
            }
            break;
          case DL_PinNorthSide:
            {
              if (bExtracted)
              {
                if (m_iNRestSamples <= 0)
                {
                  if (GetAverageAndStd( m_IterationResults , cAverage , cStd ))
                  {
                    cmplx World( m_ShiftsDL.m_x , m_ShiftsDL.m_y );
                    m_NorthPinSide.FOV = cAverage;
                    m_NorthPinSide.World = World ;

                    cmplx cViewPt = m_cLastROICenter
                      + cmplx( 300. , -m_cLastROICenter.imag() * 0.8 );
                    pOutFrame->AddFrame( FormTextFrameForView(
                      cViewPt , 20 , "0x000080" , "Pos pix\n%.2f,%.2f\nStd=(%.3f,%.3f)" ,
                      cAverage.real() , cAverage.imag() , cStd.real() , cStd.imag() ) );

                    m_WorkingState = DL_PinEastSide;
                    SendDisplaceCommand( GetBoundToTenth( m_dPinDiam_um / 2. ) , GetBoundToTenth( -m_ShiftsDL.m_y ) , 0. );
                    m_ShiftsDL.m_x = GetBoundToTenth( m_dPinDiam_um / 2. ) ;
                    m_ShiftsDL.m_y = 0.;
                    ResetIterations();
                  }
                }
                else
                  GrabImage();
              }
              else
              {
                SENDERR( "Pin measurement: Don't see North Pin Edge " );
                SendMessageToEngine( "Result=Error: Don't see North Pin Edge;" , "Pin Meas Result" );
                if (m_SaveMode == Save_Final)
                  CheckAndSaveFinalImages( pOutFrame , true , "NoEdge" ) ;

                m_WorkingState = State_Idle;
              }
            }
            break;
          case DL_PinEastSide:
            {
              if (bExtracted)
              {
                if (m_iNRestSamples <= 0)
                {
                  if (GetAverageAndStd( m_IterationResults , cAverage , cStd ))
                  {
                    cmplx World( m_ShiftsDL.m_x , m_ShiftsDL.m_y );
                    CoordsCorresp NewPair( cAverage , World );
                    m_EastPinSide = NewPair;

                    cmplx cViewPt = m_cLastROICenter
                      + cmplx( 300. , -m_cLastROICenter.imag() * 0.8 );
                    pOutFrame->AddFrame( FormTextFrameForView(
                      cViewPt , 20 , "0x000080" , "Pos pix\n%.2f,%.2f\nStd=(%.3f,%.3f)" ,
                      cAverage.real() , cAverage.imag() , cStd.real() , cStd.imag() ) );

                    m_WorkingState = DL_PinSouthSide;
                    SendDisplaceCommand( GetBoundToTenth( -m_dPinDiam_um / 2. ) , GetBoundToTenth( -m_dPinDiam_um / 2. ) , 0. );
                    m_ShiftsDL.m_x = 0.;
                    m_ShiftsDL.m_y = GetBoundToTenth( -m_dPinDiam_um / 2. ) ;
                    ResetIterations();
                  }
                }
                else
                  GrabImage();
              }
              else
              {
                SENDERR( "Pin measurement: Don't see East Pin Edge " );
                SendMessageToEngine( "Result=Error: Don't see East Pin Edge;" , "Pin Meas Result" );
                if (m_SaveMode == Save_Final)
                  CheckAndSaveFinalImages( pOutFrame , true , "NoEdge" ) ;
                m_WorkingState = State_Idle;
              }
            }
            break;
          case DL_PinSouthSide:
            {
              if (bExtracted)
              {
                if (m_iNRestSamples <= 0)
                {
                  if (GetAverageAndStd( m_IterationResults , cAverage , cStd ))
                  {
                    cmplx World( m_ShiftsDL.m_x , m_ShiftsDL.m_y );
                    CoordsCorresp NewPair( cAverage , World );
                    m_SouthPinSide = NewPair;

                    cmplx cViewPt = m_cLastROICenter
                      + cmplx( 300. , -m_cLastROICenter.imag() * 0.8 );
                    pOutFrame->AddFrame( FormTextFrameForView(
                      cViewPt , 20 , "0x000080" , "Pos pix\n%.2f,%.2f\nStd=(%.3f,%.3f)" ,
                      cAverage.real() , cAverage.imag() , cStd.real() , cStd.imag() ) );

                    m_WorkingState = DL_PinWestSide;
                    SendDisplaceCommand( GetBoundToTenth( -m_dPinDiam_um / 2. ) ,
                      GetBoundToTenth( m_dPinDiam_um / 2. ) , 0. );
                    m_ShiftsDL.m_x = GetBoundToTenth( -m_dPinDiam_um / 2. ) ;
                    m_ShiftsDL.m_y = 0.;
                    ResetIterations();
                  }
                }
                else
                  GrabImage();
              }
              else
              {
                SENDERR( "Pin measurement: Don't see South Pin Edge " );
                SendMessageToEngine( "Result=Error: Don't see South Pin Edge;" , "Pin Meas Result" );
                if (m_SaveMode == Save_Final)
                  CheckAndSaveFinalImages( pOutFrame , true , "NoEdge" ) ;
                m_WorkingState = State_Idle;
              }
            }
            break;
          case DL_PinWestSide:
            {
              if (bExtracted)
              {
                if (m_iNRestSamples <= 0)
                {
                  if (GetAverageAndStd( m_IterationResults , cAverage , cStd ))
                  {
                    cmplx World( m_ShiftsDL.m_x , m_ShiftsDL.m_y );
                    CoordsCorresp NewPair( cAverage , World );
                    m_WestPinSide = NewPair;

                    cmplx cViewPt = m_cLastROICenter
                      + cmplx( 300. , -m_cLastROICenter.imag() * 0.8 );
                    pOutFrame->AddFrame( FormTextFrameForView(
                      cViewPt , 20 , "0x000080" , "Pos pix\n%.2f,%.2f\nStd=(%.3f,%.3f)" ,
                      cAverage.real() , cAverage.imag() , cStd.real() , cStd.imag() ) );

                    bool bRes = CalcPinCenter();
                    ASSERT( bRes );
                    FXRegistry Reg( "TheFileX\\Micropoint" );
                    FXString FileName = Reg.GetRegiString(
                      "Parameters" , "DebugLog" , "MPPDebug.log" );
                    FXString Dir = GetMainDir();
                    FXString Path = Dir + GetShortWorkingModeName() + FileName;
                    ofstream myfile( ( LPCTSTR ) Path , ios_base::app );
                    if (myfile.is_open())
                    {
                      FXString Out;
                      CoordsCorresp Val = m_NorthPinSide;
                      Out.Format( "\nPin measurement results %s\n"
                        "m_NorthPinSide=FOV(%.2f,%.2f)-World(%.2f,%.2f)" ,
                        ( LPCTSTR ) GetTimeAsString() ,
                        Val.FOV.real() , Val.FOV.imag() ,
                        Val.World.real() , Val.World.imag() );
                      myfile << ( LPCTSTR ) Out;
                      Val = m_EastPinSide;
                      Out.Format( "\n m_EastPinSide=FOV(%.2f,%.2f)-World(%.2f,%.2f)" ,
                        Val.FOV.real() , Val.FOV.imag() ,
                        Val.World.real() , Val.World.imag() );
                      myfile << ( LPCTSTR ) Out;
                      Val = m_SouthPinSide;
                      Out.Format( "\n m_SouthPinSide=FOV(%.2f,%.2f)-World(%.2f,%.2f)" ,
                        Val.FOV.real() , Val.FOV.imag() ,
                        Val.World.real() , Val.World.imag() );
                      myfile << ( LPCTSTR ) Out;
                      Val = m_WestPinSide;
                      Out.Format( "\n m_WestPinSide=FOV(%.2f,%.2f)-World(%.2f,%.2f)" ,
                        Val.FOV.real() , Val.FOV.imag() ,
                        Val.World.real() , Val.World.imag() );
                      myfile << ( LPCTSTR ) Out;
                      Out.Format( "\n PinPos=(%.2f,%.2f) Diam=%.2f\n" ,
                        m_CalculatedPinPosition.real() ,
                        m_CalculatedPinPosition.imag() ,
                        m_dPinDiam_um );
                      myfile.close();
                    }
                    else
                    {
                      SENDERR( "MPPT::DoProcessing open file ERROR: %s for file %s" ,
                        strerror( GetLastError() ) , ( LPCTSTR ) Path );
                    }

                    cmplx cDiff = CalcShiftForFinalPinPosition();
                    if (cDiff.real() > 1.e308)
                    {
                      if (m_SaveMode == Save_Final)
                        CheckAndSaveFinalImages( pOutFrame , true , "NoEdge" ) ;

                      m_WorkingState = State_Idle;
                    }
                    else
                    {
                      m_ShiftsDL.m_x = GetBoundToTenth( cDiff.real() ) ;
                      m_ShiftsDL.m_y = GetBoundToTenth( cDiff.imag() ) ;
                      if (( m_ZMethod == DLZ_Defocusing )
                        || ( m_ZMethod == DLZ_LongSweep )
                        || ( m_ZMethod == DLZ_ShortSweep ) || m_bPinOnlyCalib)
                      {
                        m_WorkingState = DL_FinalImageOverPin;
                      }
                      else if (m_ZMethod = DLZ_Laser)
                      {
                        m_WorkingState = DL_LaserCalib;
                        m_iCalibCntr = -1;
                        ResetIterations();
                      }
                      SendDisplaceCommand( m_ShiftsDL.m_x , m_ShiftsDL.m_y , 0. );
                    }
                  }
                }
                else
                  GrabImage();
              }
              else
              {
                SENDERR( "Pin measurement: Don't see West Pin Edge " );
                SendMessageToEngine( "Result=Error: Don't see West Pin Edge;" , "Pin Meas Result" );
                if (m_SaveMode == Save_Final)
                  CheckAndSaveFinalImages( pOutFrame , true , "NoEdge" ) ;
                m_WorkingState = State_Idle;
              }
            }
            break;
          case DL_LaserCalib:
            {
              if (bExtracted)
              {
                if (ZCalibrationProcess( pOutFrame , m_ShiftsDL ) == 0)
                {  // Calibration finished
                  m_cFinalTargetOverPin = m_SouthPinSide.World;
                  cmplx cCorrection = ConvertCoordsRelativeToCenter(
                    m_SouthPinSide.FOV );
                  m_cFinalTargetOverPin += cCorrection;
                  m_cFinalTargetOverPin._Val[ _RE ] = m_CalculatedPinPosition.real();
                  cmplx cCurrentPt( m_ShiftsDL.m_x , m_ShiftsDL.m_y );
                  cmplx cDiff = m_cFinalTargetOverPin - cCurrentPt;
                  m_ShiftsDL.m_x = GetBoundToTenth( cDiff.real() ) ;
                  m_ShiftsDL.m_y = GetBoundToTenth( cDiff.imag() ) ;
                  m_WorkingState = DL_LaserCalibFinished;
                  SendDisplaceCommand( m_ShiftsDL.m_x , m_ShiftsDL.m_y , GetBoundToTenth( -m_ShiftsDL.m_z ) );
                  m_ShiftsDL.m_z = 0.;
                  m_ShiftsDL.m_x = GetBoundToTenth( m_cFinalTargetOverPin.real() ) ;
                  m_ShiftsDL.m_y = GetBoundToTenth( m_cFinalTargetOverPin.imag() ) ;

                  SwitchLaser( false );
                }
                //             else
                //               GrabImage() ;
              }
            }
            break;
          case DL_FinalImageOverPin:
            {
              cmplx cDisplace = m_cLastExtractedResult_pix
                - ( m_cLastMeasCenter - m_cLastROICenter );
              cmplx cAbsDisplace = ConvertCoordsRelativeToCenter( cDisplace );
              m_WorkingState = DL_CorrectAfterFinalVision;
              SendDisplaceCommand( cAbsDisplace );
              m_ShiftsDL.m_x = m_EffectiveShift.m_x ;
              m_ShiftsDL.m_y = m_EffectiveShift.m_y ;
              //           CTextFrame * pResult = CreateTextFrame( "Result=CalibrationDone;" , "EndOfCalibration" ) ;
              //           PutFrame( GetOutputConnector( 1 ) , pResult ) ;
              //           m_WorkingState = State_Idle ;
              //           m_iNProcessedCavities = 0 ;
              //           m_iNProcessedBlanks = 0;
              //           m_iNZMeasured = 0 ;
            }
            break;
          case DL_CorrectAfterFinalVision:
            {
              SendMessageToEngine( "Result=CalibrationDone;" , "EndOfCalibration" );
              m_WorkingState = State_Idle;
              m_iNProcessedCavities = 0;
              m_iNProcessedBlanks = 0;
              m_iNZMeasured = 0;
            }
            break;
          case DL_MeasCavityXY:
          case DL_CaptureCavityFinalImage:
            {
              m_ShiftsDL.Reset();
              pOutFrame->AddFrame( ProcessCavity( pDataFrame ) );
              if (m_bLastCavityResult)
              {
                m_IterationResults.push_back( m_cLastCavityXYResult_um );
                //m_cLastCavityXYAveraged_um += m_cLastCavityXYResult_um ;
                bExtracted = true;
                pDisplacementViewColor = "0x00ffff";
              }
              else
              {
                if (( m_bWaitForContinueCommand == 1 )
                  && !m_bStabilizationDone
                  && ( m_iWaitCounter >= 0 )   // ENgine is waiting for stabilization
                  && ( m_IterationResults.size() // if something measured
                    || ( --m_iNAllowedBadMeasurementsOnStabilization > 0 ) )
                  && m_iNAllowedErrors > 0)
                {
                  GrabImage();
                  break;
                }
                if (--m_iNAllowedErrors <= 0)
                {
                  SendMessageToEngine( "Result=BadCavity;" , "EndOfMeasurement" );
                  cmplx Pt = m_cLastMeasCenter - cmplx( 300. , 100. );
                  pOutFrame->AddFrame( CreateTextFrame( Pt ,
                    "Cavity is not recognized" , "0x0000ff" , 20 ) );
                  SaveCSVLogMsg( " %4d,     0,     0,    0,bad XY contours" , m_iNProcessedCavities );
                  SaveLogMsg( "Cavity %d - XY Result=Error: "
                    "Can't recognize cavity (bad contours);!!!!!!!!!!!!!!!!!!!!!!!!!" ,
                    m_iNProcessedCavities );
                  //               FXString FileName;
                  //               FileName.Format( "%s_%sBadCavXY_%d.bmp" , (LPCTSTR) GetTimeStamp() ,
                  //                 GetShortWorkingModeName() , m_iNProcessedCavities );
                  //               SaveImage( m_pCurrentImage , FileName );
                  if (m_SaveMode == Save_Final)
                  {
                    CheckAndSaveFinalImages( pOutFrame , true ,
                      ( m_WorkingState == DL_MeasCavityXY ) ? "BadCavXY" : "BadCavXYShifted" ) ;
                  }
                  SaveCavityResultLogMsg( "%s" , _T( "Error, Bad Contours" ) ) ;

                  m_WorkingState = State_Idle;
                }
                else // not measured, next try
                  GrabImage();
                break;
              }

              if (m_iNRestSamples <= 0)
              {
                if (GetAverageAndStd( m_IterationResults , cAverage , cStd ))
                {
                  //m_cLastCavityXYAveraged_um /= m_iAverager ;
                  m_cLastCavityXYAveraged_um = cAverage;
                  cmplx cViewPt = m_cLastROICenter
                    + cmplx( 100. , -m_cLastROICenter.imag() * 0.8 );
                  CDataFrame * pAveraged = FormTextFrameForView( cViewPt , 20 ,
                    "0xffff00" , "Shift_um(%.2f,%.2f) Std(%.3f,%.3f)" ,
                    m_cLastCavityXYAveraged_um.real() ,
                    m_cLastCavityXYAveraged_um.imag() ,
                    cStd.real() , cStd.imag() );
                  pAveraged->SetLabel( _T( "Averaged" ) );
                  pOutFrame->AddFrame( pAveraged );
                  SaveLogMsg( "Cavity %d XY measured, "
                    "Shift(%.2f,%.2f) um, Std(%.3f,%.3f) um" ,
                    m_iNProcessedCavities ,
                    m_cLastCavityXYAveraged_um.real() ,
                    m_cLastCavityXYAveraged_um.imag() ,
                    cStd.real() , cStd.imag() );
                  if (m_iSaveCavityPreliminaryData & 1)
                    CheckAndSaveFinalImages( pOutFrame , false , "PrelimCav" );
                  if (m_iSaveCavityPreliminaryData & 2)
                  {
                    FXString ForLogX , ForLogY , Add;
                    for (size_t i = 0; i < m_IterationResults.size(); i++)
                    {
                      Add.Format( "%.2f%s" , m_IterationResults[ i ].real() ,
                        ( i == ( m_PartMeasResult.size() - 1 ) ) ? "" : "," );
                      ForLogX += Add;
                      Add.Format( "%.2f%s" , m_IterationResults[ i ].imag() ,
                        ( i == ( m_PartMeasResult.size() - 1 ) ) ? "" : "," );
                      ForLogY += Add;
                    }
                    SaveLogMsg( "Cavity %d %s Coords: Avg(%.2f,%.2f) Std(%.2f,%.2f) X(%s)Y(%s)" ,
                      m_iNProcessedCavities ,
                      ( m_WorkingState == DL_MeasCavityXY ) ? "Prelim." : "Final" ,
                      m_cLastCavityXYAveraged_um.real() , m_cLastCavityXYAveraged_um.imag() ,
                      cStd.real() , cStd.imag() ,
                      ( LPCTSTR ) ForLogX , ( LPCTSTR ) ForLogY );
                  }
                }
                m_cLastCentAver_um /= ( double ) m_IterationResults.size();
                m_cLastCentAver_um = ConvertCoordsRelativeToCenter( m_cLastCentAver_um );

                m_cLastUpperAver_um /= ( double ) m_IterationResults.size();
                m_cLastUpperAver_um = ConvertCoordsRelativeToCenter( m_cLastUpperAver_um );
                m_cLastLowerAver_um /= ( double ) m_IterationResults.size();
                m_cLastLowerAver_um = ConvertCoordsRelativeToCenter( m_cLastLowerAver_um );

                ResetIterations();
                m_dZUsedAsResult = m_dAverage_dZ = 0.;
                if (m_cLastRightCavityCenter_um.real() != 0.)
                {
                  m_ZMethod = AnalyzeAndGetDLMeasurementZMethod();
                  switch (m_ZMethod)
                  {
                    case DLZ_Laser:
                      // Move right cavity area for Z measurements by laser
                      m_ShiftsDL.m_x = GetBoundToTenth( m_cLastRightCavityCenter_um.real() );
                      m_ShiftsDL.m_y = GetBoundToTenth( m_cLastRightCavityCenter_um.imag() );
                      m_ShiftsDL.m_z = 0.;
                      m_WorkingState = DL_CavityJumpForZ;
                      SendDisplaceCommand( m_ShiftsDL );
                      SetLaserMode( LEM_ForCavity );
                      break;
                    case DLZ_Defocusing:
                    case DLZ_LongSweep:
                    case DLZ_ShortSweep:
                      {
                        if (( m_bWaitForContinueCommand == 1 )
                          && !m_bStabilizationDone
                          && ( m_iWaitCounter >= 0 ))
                        {
                          ResetIterations();
                          GrabImage();
                          cmplx Pt( 30. , m_LastROI.Height() * 0.8 );
                          pOutFrame->AddFrame( CreateTextFrame( Pt , "0xffff00" , 20 ,
                            "WaitForContinue" , pDataFrame->GetId() , "Wait For Continue" ) );
                          if (++m_iWaitCounter == 1)
                          {
                            CTextFrame * pClearFrame = CreateTextFrame( "Clear Graph" , "Clear" , m_PlotSampleCntr );
                            PutFrame( GetOutputConnector( 2 ) , pClearFrame );
                            SendMessageToEngine( "Result=StartTempDetection;" , NULL );
                          }
                          CheckAndSendCavityPlot();
                        }
                        else
                        {
                          InitZDefocusingMeasurement();
                          m_WhatSideToUse = m_WhatSideToUseForCavity;
                        }
                      }
                      break;
                    case DLZ_NoZMeasurement:
                      {
                        if (m_WorkingState == DL_MeasCavityXY) // Initial measurement
                        {
                          FXString ForLog;
                          ForLog.Format( _T( "OK, Prelim, Selected(%.1f,%.1f), Cent(%.1f,%.1f),"
                            " Up(%.1f,%.1f), Dn(%.1f,%.1f)" ) ,
                            m_cLastCavityXYAveraged_um.real() , m_cLastCavityXYAveraged_um.imag() ,
                            m_cLastCentAver_um.real() , m_cLastCentAver_um.imag() ,
                            m_cLastUpperAver_um.real() , m_cLastUpperAver_um.imag() ,
                            m_cLastLowerAver_um.real() , m_cLastLowerAver_um.imag() );
                          SaveCavityResultLogMsg( "%s" , ( LPCTSTR ) ForLog );

                          FXString MsgToEngine;
                          if (abs( m_cLastCavityXYAveraged_um ) < 70.) // microns
                          {
                            SendDisplaceCommand( m_cLastCavityXYAveraged_um );
                            m_WorkingState = DL_MoveCavityForFinalImage;
                            m_dAverage_dZ = m_dAverageLeft_dZ = m_dAverageRight_dZ = 0. ;
                            m_iNZMeasured++;
                            //                     if ( (m_bWaitForContinueCommand == 1) 
                            //                       && (m_iNProcessedCavities == 1) 
                            //                       && m_iWaitCounter > 0 )
                            //                     {
                            //                       ResetIterations();
                            //                       GrabImage();
                            //                       cmplx Pt( 30. , m_LastROI.Height() * 0.8 ) ;
                            //                       pOutFrame->AddFrame( CreateTextFrame( Pt , "0xffff00" , 20 ,
                            //                         "WaitForContinue" , pDataFrame->GetId() , "Wait For Continue" ) ) ;
                            //                       CheckAndSendCavityPlot() ;
                            //                     }
                            //                     else
                            //                     {
                            //                       if ( m_bWaitForContinueCommand == 2 )
                            //                         m_bWaitForContinueCommand = 0 ;
                            //                     }
                          }
                          else
                          {
                            MsgToEngine.Format( "Result=BadCavity; Too Big Deviation=(%.2f,%.2f);" ,
                              m_cLastCavityXYAveraged_um.real() ,
                              m_cLastCavityXYAveraged_um.imag() );
                            SendMessageToEngine( MsgToEngine , "EndOfMeasurement" );
                            m_WorkingState = State_Idle;
                          }
                        }
                        else // final image data to Engine
                        {
                          pDisplacementViewColor = "0x00ffff";
                          // Nothing to do: image with graphics will be on display
                          pOutFrame->AddFrame( ProcessCavity( pDataFrame ) );
                          cmplx cViewPt = m_cLastROICenter
                            + cmplx( 100. , -m_cLastROICenter.imag() * 0.8 );
                          CDataFrame * pAveraged = FormTextFrameForView( cViewPt , 20 ,
                            "0xffff00" , "Shift_um(%.2f,%.2f) Std(%.3f,%.3f)" ,
                            m_cLastCavityXYAveraged_um.real() ,
                            m_cLastCavityXYAveraged_um.imag() ,
                            cStd.real() , cStd.imag() );
                          pAveraged->SetLabel( _T( "Averaged" ) );

                          FXString ForLog;
                          ForLog.Format( _T( "OK, Final, Selected(%.1f,%.1f), Cent(%.1f,%.1f),"
                            " Up(%.1f,%.1f), Dn(%.1f,%.1f)" ) ,
                            m_cLastCavityXYAveraged_um.real() , m_cLastCavityXYAveraged_um.imag() ,
                            m_cLastCentAver_um.real() , m_cLastCentAver_um.imag() ,
                            m_cLastUpperAver_um.real() , m_cLastUpperAver_um.imag() ,
                            m_cLastLowerAver_um.real() , m_cLastLowerAver_um.imag() );
                          SaveCavityResultLogMsg( "%s" , ( LPCTSTR ) ForLog );

                          FXString MsgToEngine;
                          if (abs( m_cLastCavityXYAveraged_um ) < 70.) // microns
                          {
                            if (( m_bWaitForContinueCommand == 1 ) && !m_bStabilizationDone)
                            {
                              ResetIterations();
                              GrabImage();
                              cmplx Pt( 30. , m_LastROI.Height() * 0.8 ) ;
                              pOutFrame->AddFrame( CreateTextFrame( Pt , "0xffff00" , 20 ,
                                "WaitForContinue" , pDataFrame->GetId() , "Wait For Continue" ) ) ;
                              if (++m_iWaitCounter == 1)
                                SendMessageToEngine( "Result=StartTempDetection;" , NULL ) ;
                              CheckAndSendCavityPlot() ;
                            }
                            else
                            {
                              MsgToEngine.Format( "Result=Cavity;dX=%.2f;dY=%.2f;dZ=0;" ,
                                m_cLastCavityXYAveraged_um.real() ,
                                m_cLastCavityXYAveraged_um.imag() );
                              SendMessageToEngine( MsgToEngine , "EndOfMeasurement" );
                              m_bWaitForContinueCommand = 0 ;
                            }
                          }
                          else
                          {
                            MsgToEngine.Format( "Result=BadCavity; Too Big Deviation=(%.2f,%.2f);" ,
                              m_cLastCavityXYAveraged_um.real() ,
                              m_cLastCavityXYAveraged_um.imag() );
                            SendMessageToEngine( MsgToEngine , "EndOfMeasurement" );
                          }
                          pOutFrame->AddFrame( pAveraged );
                          CheckAndSaveFinalImages( pOutFrame ) ;

                          m_WorkingState = State_Idle;
                        }
                        break;
                      }
                    default:
                      SendMessageToEngine( "Result=BadCavity; Unknown Z "
                        "measurement method" , "EndOfMeasurement" );
                      cmplx Pt = m_cLastMeasCenter - cmplx( 300. , 100. );
                      pOutFrame->AddFrame( CreateTextFrame( Pt ,
                        "Cavity is not measured: Unknown Z "
                        "measurement method" , "0x0000ff" , 20 ) );
                      SaveCSVLogMsg( " %4d,     0,     0,    0,Unknown Z Method" , m_iNProcessedCavities );
                      SaveLogMsg( "Cavity %d - XY Result=Error: Unknown Z Method;" ,
                        m_iNProcessedCavities );
                      m_WorkingState = State_Idle;

                  }
                }
              }
              else
                GrabImage();
            }
            break;
          case DL_MeasCavityZMultiple:
          case DL_MeasCavityZ:
            {
              pDisplacementViewColor = "0x00ffff";

              double dZ = SelectLaserSpot( m_cLastExtractedResult_pix );

              if (m_IterationResults.size())
                m_IterationResults.pop_back();
              m_IterationResults.push_back( m_cLastExtractedResult_pix );

              cmplx cViewPt = m_cLastROICenter
                + cmplx( 300. , -m_cLastROICenter.imag() * 0.5 );
              pOutFrame->AddFrame( FormTextFrameForView( cViewPt , 16 ,
                "0xffff00" , "dZ um = %.2f" , dZ ) );
              if (m_iNRestSamples <= 0)
              {
                if (GetAverageAndStd( m_IterationResults , cAverage , cStd ))
                {
                  m_dAverage_dZ = GetdZ( cAverage );
                  cmplx cViewPt = m_cLastROICenter
                    + cmplx( 300. , -m_cLastROICenter.imag() * 0.8 );
                  pOutFrame->AddFrame( FormTextFrameForView( cViewPt , 20 ,
                    "0xffff00" , "Avr dZ um = %.2f\nStd = %.3f" , m_dAverage_dZ ,
                    abs( cStd ) / m_dZSensitivity_pix_per_um ) );

                  CCoor3t NewZResult( cAverage.real() , cAverage.imag() ,
                    m_dAverage_dZ , m_CurrentPart.m_Cavity.m_dPlaneArea_um2 );
                  m_ZMeasurements.push_back( NewZResult );
                  m_Info1.Empty(); m_Info2.Empty(); m_Info3.Empty();
                  if (m_WorkingState == DL_MeasCavityZMultiple)
                  {
                    cmplx cStep = GetShiftForNextZMeasurement();
                    m_WorkingState = DL_MoveToCorrectZ;
                    if (cStep != 0.)
                    {
                      SendDisplaceCommand( cStep );
                      ResetIterations();
                      break;
                    }
                    else
                    { // process all Z results
                      double dAver = 0.;
                      double dSquares = 0.;
                      for (auto it = m_ZMeasurements.begin(); it < m_ZMeasurements.end(); it++)
                      {
                        dAver += it->m_z;
                        dSquares += ( it->m_z * it->m_z );
                        FXString Tmp;
                        Tmp.Format( ",%.2f" , it->m_z );
                        m_Info1 += Tmp;
                      }
                      dAver /= m_ZMeasurements.size();
                      dSquares /= m_ZMeasurements.size();
                      double dStd = sqrt( dSquares - dAver * dAver );

                      m_dAverage_dZ = dAver;
                    }
                    SendDisplaceCommand( -m_cCavityZMeasurementShift_um.real() ,
                      -m_cCavityZMeasurementShift_um.imag() , m_dAverage_dZ );
                  }
                  else
                    SendDisplaceCommand( 0. , 0. , m_dAverage_dZ );
                }
              }
              else
                GrabImage();
            }
            break;
          case DL_CaptureZWithCorrectHeigth:
            {
              pDisplacementViewColor = "0x00ffff";
              m_dAverage_dZ = GetdZ( m_cLastExtractedResult_pix );

              cmplx cViewPt = m_cLastROICenter
                + cmplx( 300. , -m_cLastROICenter.imag() * 0.5 );
              pOutFrame->AddFrame( FormTextFrameForView( cViewPt , 16 ,
                "0xffff00" , "dZ um = %.2f" , m_dAverage_dZ ) );
              m_WorkingState = DL_MoveCavityForFinalImage;
              //          CheckAndSaveFinalImages(pOutFrame, false , "Prelim1st" );

              SetCavityMode();

              SendDisplaceCommand(
                -m_ShiftsDL.m_x + m_cLastCavityXYAveraged_um.real() ,
                -m_ShiftsDL.m_y + m_cLastCavityXYAveraged_um.imag() ,
                0. );
            }
            break;
            //       case DL_CaptureCavityFinalImage:
            //         {
            //           pDisplacementViewColor = "0x00ffff";
            //           // Nothing to do: image with graphics will be on display
            //           pOutFrame->AddFrame( ProcessCavity( pDataFrame ) );
            //           cmplx cViewPt = m_cLastROICenter
            //             + cmplx( 100. , -m_cLastROICenter.imag() * 0.8 );
            //           CDataFrame * pAveraged = FormTextFrameForView( cViewPt , 20 ,
            //             "0xffff00" , "Shift_um(%.2f,%.2f) Std(%.3f,%.3f)" ,
            //             m_cLastCavityXYAveraged_um.real() ,
            //             m_cLastCavityXYAveraged_um.imag() ,
            //             cStd.real() , cStd.imag() );
            //           pAveraged->SetLabel( _T( "Averaged" ) );
            //           SendMessageToEngine(
            //             "Result=Cavity;dX=0;dY=0;dZ=0;", "EndOfMeasurement");
            //           pOutFrame->AddFrame( pAveraged );
            //           CheckAndSaveFinalImages( pOutFrame ) ;
            // //           FXString FileName;
            // //           FileName.Format( "%s_%sLastCavXY_%d.bmp" , (LPCTSTR) GetTimeStamp() ,
            // //             GetShortWorkingModeName() , m_iNProcessedCavities );
            // //           SaveImage( m_pCurrentImage , FileName );
            // 
            //           m_WorkingState = State_Idle;
            //         }
            //         break;
          case DL_LiveVideoCavity:
            {
              if (m_bMeasureFocus)
              {
                if (abs( m_cLastLeftCavitySize_pix ) != 0.)
                {
                  m_LeftFocusMeasRect = GetNormFocusRectangle(
                    m_cLastLeftCavityCenter_pix , m_cLastLeftCavitySize_pix );
                  m_RightFocusMeasRect = GetNormFocusRectangle(
                    m_cLastRightCavityCenter_pix , m_cLastRightCavitySize_pix );
                  CalcAndAddFocusIndication( pOutFrame ,
                    &m_LeftFocusMeasRect , &m_RightFocusMeasRect );
                }
                else
                  CalcAndAddFocusIndication( pOutFrame );
              }
              pDisplacementViewColor = "0x00ffff";
              if (bExtracted && m_CavitiesConturs.Count() == 2)
              {
                pOutFrame->AddFrame( ProcessCavity( pDataFrame ) );
                if (m_bLastCavityResult)
                {
                  m_cLastCavityXYAveraged_um += m_cLastCavityXYResult_um;
                  if (m_iNRestSamples <= 0)
                  {
                    if (m_iAverager > 1)
                    {
                      m_cLastCavityXYAveraged_um /= m_iAverager;
                      m_AverageViewPt = m_cLastROICenter
                        + cmplx( 100. , -m_cLastROICenter.imag() * 0.8 );
                      m_AverageViewText.Format( "Shift_um(%.2f,%.2f) Std(%.3f,%.3f)" ,
                        m_cLastCavityXYAveraged_um.real() ,
                        m_cLastCavityXYAveraged_um.imag() ,
                        cStd.real() , cStd.imag() ) ;
                      CheckAndSendCavityPlot() ;
                    }
                    ResetIterations();
                    m_cLastCavityXYAveraged_um = cmplx( 0. , 0. );
                  }
                }
                else
                  m_iNRestSamples++;
                CDataFrame * pAveraged = FormTextFrameForView( m_AverageViewPt , 20 ,
                  "0xffff00" , ( LPCTSTR ) m_AverageViewText );
                pAveraged->SetLabel( _T( "Averaged" ) );
                pOutFrame->AddFrame( pAveraged );

              }
              else
              {
                cmplx Pt = m_cLastMeasCenter - cmplx( 300. , 100. );
                pOutFrame->AddFrame( CreateTextFrame( Pt ,
                  "Cavity is not recognized" , "0x0000ff" , 20 ) );
              }
              pDisplacementViewColor = "0x00ffff";
              GrabImage();
            }
            break;
          case DL_LiveVideoPinMeasure:
            {
              //         if (m_iNAttempts++ < m_iNMaxAttempts)
              //         {
              //           if (AdjustExposureForFocus(m_pCurrentImage,
              //             pOutFrame, m_dTargetForFocusExpAdjust))
              //           {
              //             ProgramExposure(m_iNewFocusExposure);
              //             GrabImage();
              //             break;
              //           }
              //         }
              //         m_iNAttempts = m_iNMaxAttempts;
              //         m_Exposures.clear();
              //         m_Averages.clear();
              //         m_Threshs.clear();
              CalcAndAddFocusIndication( pOutFrame );
            }
          case DL_LiveVideoPinNorth:
          case DL_LiveVideoPinEast:
          case DL_LiveVideoPinSouth:
          case DL_LiveVideoPinWest:
            {
              m_cLastCalibTargetXYAveraged_pix += m_cLastExtractedResult_pix;
              if (m_iNRestSamples <= 0)
              {
                if (m_iAverager > 1)
                {
                  m_cLastCalibTargetXYAveraged_pix /= m_iAverager;
                  m_cLastCalibTargetXYAveraged_pix *= m_dScale_um_per_pix;
                  m_PinAverageInfo.Format( "Shift_um(%.2f,%.2f)" ,
                    m_cLastCalibTargetXYAveraged_pix.real() ,
                    m_cLastCalibTargetXYAveraged_pix.imag() );
                }
                ResetIterations();
                m_cLastCalibTargetXYAveraged_pix = cmplx( 0. , 0. );
              }
              cmplx cViewPt = m_cLastROICenter
                + cmplx( 100. , -m_cLastROICenter.imag() * 0.8 );
              CTextFrame * pAveraged = CreateTextFrame( cViewPt , m_PinAverageInfo ,
                "0x000000" , 20 , _T( "Averaged" ) );
              pOutFrame->AddFrame( pAveraged );

              GrabImage();
            }
            break;
          case DL_LiveVideoLaser:
            {
              if (bExtracted)
              {
                const CFigureFrame * pCenterPtFrame = pDataFrame->GetFigureFrame( "laser" );
                cmplx cLasCentPos = ( pCenterPtFrame ) ?
                  CDPointToCmplx( pCenterPtFrame->GetAt( 0 ) ) - m_cLastROICenter : m_cNominalZZero;
                pDisplacementViewColor = "0xffff00";

                double dZ = GetdZ( cLasCentPos );

                cmplx cViewPt( m_cLastROICenter + cmplx( -700 , -100 ) );
                pOutFrame->AddFrame( FormTextFrameForView( cViewPt , 20 ,
                  "0xffff00" , "dZ=%.2f um" , dZ ) );
                m_cLastLaserXYAveraged_pix += cLasCentPos;
                m_dAverage_dZ += dZ;
                if (m_iNRestSamples <= 0)
                {
                  if (m_iAverager > 1)
                  {
                    m_cLastLaserXYAveraged_pix /= m_iAverager;
                    m_dAverage_dZ /= m_iAverager;
                    cmplx cViewPt = m_cLastROICenter
                      + cmplx( 300. , -m_cLastROICenter.imag() * 0.8 );
                    cmplx cConverted_um = ConvertCoordsRelativeToCenter( m_cLastLaserXYAveraged_pix );
                    pOutFrame->AddFrame( FormTextFrameForView( cViewPt , 20 ,
                      "0xffff00" , "  Shift\nPix: %.2f,%.2f\num: %.2f,%.2f\ndZ=%.2f um" ,
                      m_cLastLaserXYAveraged_pix.real() ,
                      m_cLastLaserXYAveraged_pix.imag() ,
                      cConverted_um.real() , cConverted_um.imag() ,
                      m_dAverage_dZ ) );
                  }
                  ResetIterations();
                  m_cLastLaserXYAveraged_pix = cmplx( 0. , 0. );
                  m_dAverage_dZ = 0.;
                }
              }
              else
                pOutFrame->AddFrame( CreateTextFrame( m_cLastROICenter ,
                  "No Laser Spot" , "0x0000ff" , 20 ) );
              GrabImage();
            }
            break;
        }
      }
      break;
    case MPPTWM_UpFront:
      {
        pDisplacementViewColor = "0x00ffff";
        switch (m_WorkingState)
        {
          case ULF_LiveVideo:
            {
              if (m_PartConturs.Count())
              {
                cmplx NearestCenter( m_cLastROICenter * 2. );
                ImgMoments Moments ;
                double dNearestDist = abs( m_cLastROICenter );
                const CFigureFrame * pSelectedFig = NULL ;
                cmplx cCent;
                cmplx cConturSize;
                CmplxArray Extrems;

                for (int i = 0 ; i < m_PartConturs.Count() ; i++)
                {
                  const CFigureFrame * pFig = ( const CFigureFrame* ) m_PartConturs.GetFrame( i );
                  cmplx cSize;
                  cmplx cCenter = FindExtrems( pFig , Extrems , NULL , &cSize ) ;
                  double dDistToCenter = abs( cCenter - m_cLastROICenter );
                  if (dDistToCenter < dNearestDist)
                  {
                    dNearestDist = dDistToCenter;
                    pSelectedFig = pFig ;
                    cCent = cCenter;
                    cConturSize = cSize;
                  }
                }
                if (MeasureAndFilterHorizAndVertEdges( pSelectedFig , cCent ,
                  cConturSize , pOutFrame , &m_BlankCentersAndCorners , m_dGoodStdThres_pix )
                  && m_BlankCentersAndCorners.size())
                {
                  FXString ErrorMsg ;
                  CheckBlankSize( m_BlankCentersAndCorners , pOutFrame , ErrorMsg ) ;
                  double dNow = GetHRTickCount();
                  if (( m_dXY_PlotPeriod != 0. ) && ( dNow - m_dLastPlotTime > m_dXY_PlotPeriod ))
                  {
                    if (m_PlotSampleCntr == 0)
                    {
                      FXString FileName ;
                      FileName = m_CurrentReportsDir + "BlankPosDataLive_" + GetTimeAsString_ms() + ".csv" ;
                      PutFrame( GetOutputConnector( 2 ) , CreateTextFrame( FileName , "SaveFileName" , 0 ) ) ;
                    }
                    m_PlotSampleCntr++ ;

                    cmplx cMoveDist = m_BlankCentersAndCorners[ m_CurrentPart.m_Blank.m_UsedBlankEdge ]
                      - ( m_cPartPatternPt + m_cLastROICenter ) ;
                    cmplx cInMicrons = ConvertCoordsRelativeToCenter( cMoveDist ) ;
                    CQuantityFrame * pX = CreateQuantityFrame( cInMicrons.real() , "Plot_Append:dX" , m_PlotSampleCntr );
                    PutFrame( GetOutputConnector( 2 ) , pX );
                    CQuantityFrame * pY = CreateQuantityFrame( cInMicrons.imag() , "Plot_Append:dY" , m_PlotSampleCntr );
                    PutFrame( GetOutputConnector( 2 ) , pY );
                    m_dLastPlotTime = dNow;
                  }
                }

              }
              GrabImage();
            }
            break;
          case ULF_MoveForScaleCalib:
            {
              if (bExtracted)
              {
                if (abs( m_cScale_um_pix ) != 0.)
                {
                  FXRegistry Reg( "TheFileX\\Micropoint" );
                  int iDoXYCalibration = Reg.GetRegiInt( "Data" , "DoULFCalibration" , 1 );
                  if (iDoXYCalibration)
                  {
                    cmplx cCentDisplacement = ConvertCoordsRelativeToCenter(
                      m_cLastExtractedResult_pix );
                    SendDisplaceCommand( cCentDisplacement );
                    break;
                  }
                  // No need for calibration, go to simple blank measurement
                  // and reference point saving

    //               GetPatternPoint();
    //               cmplx cPatternShift = m_BlankCentersAndCorners[m_CurrentPart.m_Blank.m_UsedBlankEdge]
    //                 - m_cLastROICenter - m_cPartPatternPt;
    //               cmplx cMove = ConvertCoordsRelativeToCenter(cPatternShift);
    //               SendDisplaceCommand(cMove);
                  m_WorkingState = ULF_MoveAfterCalib;
                  GrabImage();
                }
                else // no scale, begin calibration from current point
                {
                  m_WorkingState = ULF_ScaleCalib;
                  GrabImage();
                }
              }
              else
              {
                SENDERR( "Uplook Scale Calibration: Don't see blank before calibration" ,
                  GetWorkingModeName() );
                SendMessageToEngine(
                  "Result=BadBlank: Can't recognize target;" , "Scaling result" );
                if (m_SaveMode == Save_Final)
                  CheckAndSaveFinalImages( pOutFrame , true , "No1stView" ) ;
                m_WorkingState = State_Idle;
              }
            }
            break;
          case ULF_ScaleCalib:
            if (bExtracted)
            {
              if (XYCalibrationProcess( pOutFrame , m_ShiftsUL ))
              {  // XY calibration is not finished
              }
              else // XY calibration finished
              {
                cmplx cCentDisplacement = ConvertCoordsRelativeToCenter(
                  m_cLastExtractedResult_pix );
                m_WorkingState = ULF_MeasureAfterCalib;
                SendDisplaceCommand( cCentDisplacement );
              }
            }
            else
            {
              SENDERR( "Uplook Scale Calibration: Don't see blank WM=%s Step=%d" ,
                GetWorkingModeName() , m_iCalibCntr );
              SendMessageToEngine(
                "Result=Error: Can't recognize target;" , "Scaling result" );
              if (m_SaveMode == Save_Final)
                CheckAndSaveFinalImages( pOutFrame , true , "NoBlank" ) ;
              m_WorkingState = State_Idle;
            }
            break;
          case ULF_MeasureAfterCalib:
            {
              if (bExtracted)
              {
                cmplx cMove = ConvertCoordsRelativeToCenter( m_cLastExtractedResult_pix );
                m_WorkingState = ULF_MoveAfterCalib;
                SendDisplaceCommand( cMove );
              }
              else
              {
                SENDERR( "Uplook Scale Calibration: Don't see blank WM=%s Step=%d" ,
                  GetWorkingModeName() , m_iCalibCntr );
                SendMessageToEngine(
                  "Result=Error: Can't measure blank after moving to center;" ,
                  "Scaling result" );
                m_WorkingState = State_Idle;
                SaveCSVLogMsg( "Don't see blank after calibration, Cntr Reset" ,
                  m_cPartPatternPt.real() , m_cPartPatternPt.imag() );
                SaveLogMsg( "Don't see blank after calibration" );
              }
            }
            break;
          case ULF_MoveAfterCalib:
          case ULF_WaitForExternalReady:
            {
              m_CurrentPart.m_Gauge.m_dBasePos_um =
                m_CurrentPart.m_Blank.m_dBasePos_um = ConvertCoordsRelativeToCenter( m_cPartPatternPt ).imag();
              if (AccumulateDataAboutBlank( pOutFrame ) == 0) // measurement finished
              {
                //m_cPartPatternPt = m_cLastPartMeasuredPt; // this is in pixels
                m_cPartPatternPt._Val[ _IM ] += m_CurrentPart.m_Blank.m_dYShiftForBlank_um / m_dScale_um_per_pix;
                switch (m_bWaitForContinueCommand)
                {
                  case 1:
                    {
                      m_iNProcessedBlanks = -1;
                      m_WorkingState = ULF_WaitForExternalReady;
                      InitZGrabForULF( "domeasure" ); // do z measure for next cycle
                      ResetIterations();
                      GrabImage();
                      cmplx Pt( 30. , m_LastROI.Height() * 0.8 ) ;
                      pOutFrame->AddFrame( CreateTextFrame( Pt , "0xffff00" , 20 ,
                        "WaitForContinue" , pDataFrame->GetId() , "Wait For Continue" ) ) ;
                      if (++m_iWaitCounter == 1)
                        SendMessageToEngine( "Result=StartTempDetection;" , NULL ) ;
                      cmplx cDistFOV = m_cLastPartMeasuredPt;
                      cmplx cMoveDist = ConvertCoordsRelativeToCenter( cDistFOV );
                      PlotULandCheckJumps( cMoveDist , m_dExtdZ );
                    }
                    break;
                  case 0:
                    {
                      if (m_WorkingState == ULF_MoveAfterCalib)
                      {
                        FXRegistry Reg( "TheFileX\\Micropoint" );
                        //Reg.WriteRegiCmplx( "Data" , "PartPatternPt" , m_cPartPatternPt );
                        SendMessageToEngine( "Result=CalibrationDone;" , "EndOfCalibration" );
                        SaveCSVLogMsg( "Calibration Finished,%.2f,%.2f,0,Cntr Reset" ,
                          m_cPartPatternPt.real() , m_cPartPatternPt.imag() );
                        SaveLogMsg( " Calibration Finished,Xfov=%.2f,Yfov%.2f,0,Cntr Reset" ,
                          m_cPartPatternPt.real() , m_cPartPatternPt.imag() );
                        CheckBlankSize( m_BlankCentersAndCorners , pOutFrame , m_Info3 );
                        CheckAndSaveFinalImages( pOutFrame , false , "AfterCalib" );
                        m_iNProcessedBlanks = 0;
                        m_WorkingState = State_Idle;
                        SwitchOffConstantLight( true , true );
                      }
                    }
                    break;
                  case 2:
                    {
                      m_dZAfterShift_um = DBL_MAX;
                      CTextFrame * pULSOrder = CreateTextFrame( "docalibration" , "FromULF" );
                      PutFrame( GetOutputConnector( 4 ) , pULSOrder );
                      m_WorkingState = State_Idle;
                      m_bWaitForContinueCommand = 0;
                    }
                    break;
                  default:
                    ASSERT( 0 );
                }
              }
            }
            break;
          case ULF_RightSideCorrection:
            {
              if (bExtracted)
              {
                if (m_iNRestSamples <= 0)
                {
                  if (GetAverageAndStd( m_IterationResults , cAverage , cStd ))
                  {
                    if (m_PartConturs.Count())
                    {
                      const CFigureFrame * pFig = ( const CFigureFrame* ) m_PartConturs.GetFrame( 0 );
                      //                   cmplx cCent = m_CurrentPart.m_Cavity.m_cCenterAsFigure = m_cLastConturCenter ;
                      //                   //cCent -= m_cLastROICenter;
                      //                   m_CurrentPart.m_Cavity.m_cSizeAsFigure = m_cLastConturSize ;
                      cmplx cCent = m_cLastConturCenter;
                      if (MeasureAndFilterHorizAndVertEdges( pFig , cCent ,
                        m_cLastConturSize , pOutFrame , &m_BlankCentersAndCorners , m_dGoodStdThres_pix ))
                      {
                        GetPatternPoint();
                        cmplx cPatternShift = m_BlankCentersAndCorners[ m_CurrentPart.m_Blank.m_UsedBlankEdge ]
                          - m_cLastROICenter - m_cPartPatternPt ;
                        //                     cmplx cBlankCenter = m_BlankCentersAndCorners[ 0 ] - m_cLastROICenter;
                        cmplx cMove = ConvertCoordsRelativeToCenter( cPatternShift );
                        m_WorkingState = ULF_AfterSideCorrection;
                        if (fabs( cMove.real() ) > m_CurrentPart.m_Blank.m_dBlankWidth_um
                          || fabs( cMove.imag() ) > m_CurrentPart.m_Blank.m_dBlankHeight_um)
                        {
                          ASSERT( 0 );
                        }
                        SendDisplaceCommand( cMove );
                      }
                    }
                  }
                }
                else
                  GrabImage();
              }

            }
            break;
          case ULF_MeasureAndCorrect:
            if (bExtracted)
            {
              if (m_PartConturs.Count() == 0)
              {
                SaveCSVLogMsg( "  %3d,0.,0.,0.,ERROR on Blank" , m_iNProcessedBlanks );
                SaveLogMsg( "Bad Blank %d final result - 0.,0.,0.," , m_iNProcessedBlanks );
                CheckAndSaveFinalImages( pOutFrame , true );
                SendMessageToEngine( "Result=BadBlank; Don't see blank from bottom;" , "BadImage" );
                pOutFrame->AddFrame( CreateTextFrame( m_cLastROICenter ,
                  "Blank is not measured from bottom" , "0x0000ff" , 20 ) );

                SENDERR( "No contours for ULF measure and correct" );
                m_WorkingState = State_Idle;
                break;
              }
              GetZResult() ;
              if (AccumulateDataAboutBlank( pOutFrame ) == 0) // measurement finished
              {
                cmplx cDistFOV = m_cLastPartMeasuredPt ;
                cmplx cMoveDist = ConvertCoordsRelativeToCenter( cDistFOV );
                PlotULandCheckJumps( cMoveDist , m_dExtdZ );

                //             cmplx TextPt = m_cLastROICenter + m_cPartPatternPt + cmplx( -100 , 20 ) ;
                //             pOutFrame->AddFrame( CreateTextFrame( TextPt , "0xff00ff" , 12 , "RefPt" , 0 , "(%.2f,%.2f)" ,
                //               cMoveDist.real() , cMoveDist.imag() ) ) ;
                // 
                if (m_bWaitForContinueCommand)
                {
                  m_iNProcessedBlanks = -1;
                  m_WorkingState = State_Idle;
                  InitZGrabForULF( "domeasure" ); // do z measure for next cycle
                  break;
                }
                else if (m_iNProcessedBlanks == -1)
                {  // stop graph view process
                  m_iNProcessedBlanks = 0 ;
                  m_dXY_PlotPeriod = 0.;
                }
                if (fabs( cMoveDist.real() ) >= 0.1
                  || fabs( cMoveDist.imag() ) >= 0.1
                  || fabs( m_dExtdZ ) >= 0.1)
                {
                  m_WorkingState = ( m_iNProcessedBlanks <= 0 ) ?
                    ULF_MoveAfterCalib : ULF_MoveAfterMeasurement;

                  if (fabs( cMoveDist.real() ) > m_CurrentPart.m_Blank.m_dBlankWidth_um
                    || fabs( cMoveDist.imag() ) > m_CurrentPart.m_Blank.m_dBlankHeight_um)
                  {
                    ASSERT( 0 );
                  }

                  SaveLogMsg( "Blank %d State=%s Displacement=(%.f,%.1f,%.1f)" ,
                    m_iNProcessedBlanks , GetWorkingStateName() ,
                    cMoveDist.real() , cMoveDist.imag() , m_dExtdZ );
                  SendDisplaceCommand( cMoveDist.real() ,
                    cMoveDist.imag() , m_dExtdZ );
                  m_dExtdZ = 0. ;
                  CheckBlankSize( m_BlankCentersAndCorners , pOutFrame , m_Info3 );
                }
                else
                  CheckBlankSizesAndReport( pOutFrame ) ;

                if (m_iSaveBlankPreliminaryData & 1)
                  CheckAndSaveFinalImages( pOutFrame , false , "PrelimBlank" );
                if (m_iSaveBlankPreliminaryData & 2)
                {
                  FXString ForLogX , ForLogY , Add ;
                  for (size_t i = 0 ; i < m_PartMeasResult.size() ; i++)
                  {
                    Add.Format( "%.2f%s" , m_PartMeasResult[ i ].real() ,
                      ( i == ( m_PartMeasResult.size() - 1 ) ) ? "" : "," ) ;
                    ForLogX += Add ;
                    Add.Format( "%.2f%s" , m_PartMeasResult[ i ].real() ,
                      ( i == ( m_PartMeasResult.size() - 1 ) ) ? "" : "," ) ;
                    ForLogY += Add ;
                  }
                  SaveLogMsg( "Blank %d Prelim.Coords: Avg(%.2f,%.2f) Std(%.2f,%.2f) X(%s)Y(%s)" ,
                    m_iNProcessedBlanks ,
                    m_cLastBlankAvPos_um.real() , m_cLastBlankAvPos_um.imag() ,
                    m_cLastBlankStd_um.real() , m_cLastBlankStd_um.imag() ,
                    ( LPCTSTR ) ForLogX , ( LPCTSTR ) ForLogY ) ;
                }
              }
            }
            break;
          case ULF_MoveAfterMeasurement:
            {
              if (bExtracted)
              {
                GetZResult() ;
                if (AccumulateDataAboutBlank( pOutFrame ) == 0) // measurement finished
                {
                  CheckBlankSizesAndReport( pOutFrame ) ;
                  SwitchOffConstantLight( true , true );
                }
              }
            }
            break;
        }
      }
      break;
    case MPPTWM_UpSide:
      {
        if (bExtracted && m_pSideBlankContur)
        {

        }
        switch (m_WorkingState)
        {
          case ULS_LiveVideo:
            {
              if (bExtracted)
              {
                if (m_iNRestSamples <= 0)
                {
                  if (GetAverageAndStd( m_IterationResults , cAverage , cStd ))
                  {
                    double dZ = cAverage.imag();
                    double dScaledZ = dZ * m_dZScale_um_per_pix;

                    cmplx cAverLCorner , cAverRCorner ;
                    GetAverageAndStd( m_LeftCornerOnSide , cAverLCorner , cStd ) ;
                    GetAverageAndStd( m_RightCornerOnSide , cAverRCorner , cStd ) ;
                    double dNow = GetHRTickCount();
                    if (( m_dZ_PlotPeriod != 0. ) && ( dNow - m_dLastPlotTime > m_dZ_PlotPeriod ))
                    {
                      CQuantityFrame * pZ = CreateQuantityFrame( dScaledZ , "Plot_Append:dZ" );
                      PutFrame( GetOutputConnector( 2 ) , pZ );
                      m_dLastPlotTime = dNow;
                    }
                  }
                  ClearConturData() ;
                  ResetIterations() ;
                }
              }
              GrabImage();
            }
            break;
          case ULS_FirstZForBlank: // Case of blank measurement
            if (bExtracted)
            {
              if (m_iNRestSamples <= 0)
              {
                if (GetAverageAndStd( m_IterationResults , cAverage , cStd ))
                {
                  double dZ = cAverage.imag();
                  double dScaledZ = dZ * m_dZScale_um_per_pix;

                  cmplx cAverLCorner , cAverRCorner ;
                  GetAverageAndStd( m_LeftCornerOnSide , cAverLCorner , cStd ) ;
                  GetAverageAndStd( m_RightCornerOnSide , cAverRCorner , cStd ) ;

                  if (fabs( dScaledZ ) >= 40.)
                  {
                    SaveLogMsg( "Blank %d First dZ=%.1f: Avg(%.2f) Std(%.2f) LCorner(%.2f,%.2f)pix RCorner(%.2f,%.2f)pix" ,
                      m_iNProcessedBlanks , dScaledZ ,
                      dScaledZ , cStd.imag() * m_dZScale_um_per_pix ,
                      cAverLCorner.real() , cAverLCorner.imag() , cAverRCorner.real() , cAverRCorner.imag() ) ;
                    m_WorkingState = ULS_ZCorrectionForMeas;
                    SendDisplaceCommand( 0. , 0. , dScaledZ );
                    CheckAndSaveFinalImages( pOutFrame , false , "Prelim" )  ;
                  }
                  else
                  {
                    SaveLogMsg( "Blank %d Small dZ=%.1f start ULF: Avg(%.2f) Std(%.2f) LCorner(%.2f,%.2f)pix RCorner(%.2f,%.2f)pix" ,
                      m_iNProcessedBlanks , dScaledZ ,
                      dScaledZ , cStd.imag() * m_dZScale_um_per_pix ,
                      cAverLCorner.real() , cAverLCorner.imag() , cAverRCorner.real() , cAverRCorner.imag() );
                    FXString Msg;
                    if (!m_bNoXYMeasurement)
                    {
                      Msg.Format( "measure_and_correct; dZ=%.2f; LC=(%.2f,%.2f); RC=(%.2f,%.2f);" , dScaledZ ,
                        cAverLCorner.real() , cAverLCorner.imag() , cAverRCorner.real() , cAverRCorner.imag() );
                      CTextFrame * pULFOrder = CreateTextFrame(
                        Msg , "FromULS" );
                      PutFrame( GetOutputConnector( 4 ) , pULFOrder );
                      m_WorkingState = State_Idle;
                      CheckAndSaveFinalImages( pOutFrame , false , "Prelim" ) ;
                    }
                    else  // No XY measurement for blank
                    {
                      FXString Result;
                      Result.Format( "Result=%s;dX=%.2f;dY=%.2f;dZ=%.2f;" ,
                        "Blank" , 0. , 0. , dScaledZ );
                      SaveCSVLogMsg( "  %3d,%6.2f,%6.2f,%6.2f,OK" , m_iNProcessedBlanks ,
                        0. , 0. , dScaledZ );
                      SaveLogMsg( "%s %d final result - %.2f,%.2f,%.2f,OK" ,
                  "Blank", -1, 0., 0., dScaledZ);
                      SendMessageToEngine( Result , "EndOfMeasurement" );
                      m_WorkingState = State_Idle ;
                    }
                  }
                  if (( m_iSaveBlankPreliminaryData & 1 ) && ( fabs( dScaledZ ) >= 40. ))
                    CheckAndSaveFinalImages( pOutFrame , false , "ZPrelimBlank" );
                  if (m_iSaveBlankPreliminaryData & 2)
                  {
                    FXString ForLogZ , Add ;
                    for (size_t i = 0 ; i < m_IterationResults.size() ; i++)
                    {
                      Add.Format( "%.2f%s" , m_IterationResults[ i ].real() ,
                        ( i == ( m_IterationResults.size() - 1 ) ) ? "" : "," ) ;
                      ForLogZ += Add ;
                    }
                    SaveLogMsg( "Blank %d Prelim.Z: Avg(%.2f) Std(%.2f) "
                      "LCorner(%.2f,%.2f)pix RCorner(%.2f,%.2f)pix ZValues(%s)" ,
                      m_iNProcessedBlanks ,
                      dScaledZ , cStd.imag() * m_dZScale_um_per_pix ,
                      cAverLCorner.real() , cAverLCorner.imag() , cAverRCorner.real() , cAverRCorner.imag() ,
                      ( LPCTSTR ) ForLogZ ) ;
                  }
                }
              }
              else
                GrabImage();
            }
            break;
          case ULS_ZCorrectionForMeas:
          case ULS_ZCorrection:
          case ULS_GrabFinal:
            if (bExtracted)
            {
              if (m_iNRestSamples <= 0)
              {
                if (GetAverageAndStd( m_IterationResults , cAverage , cStd ))
                {
                  double dZ = cAverage.imag();
                  double dScaledZ = dZ * m_dZScale_um_per_pix;
                  double dScaledZStd = cStd.imag() * m_dZScale_um_per_pix;

                  cmplx cAverLCorner , cAverRCorner ;
                  if (m_WorkingState == ULS_ZCorrectionForMeas)
                  {
                    GetAverageAndStd( m_LeftCornerOnSide , cAverLCorner , cStd ) ;
                    GetAverageAndStd( m_RightCornerOnSide , cAverRCorner , cStd ) ;
                  }

                  if (m_WorkingState == ULS_ZCorrection && ( fabs( dScaledZ ) >= 0.5 ))
                  {
                    m_WorkingState = ULS_ZCorrection2;
                    if (fabs( dScaledZ ) >= 0.5)
                    {
                      SaveLogMsg( "Blank %d dZ=%.1f Corr(%s): Avg(%.2f) Std(%.2f) LCorner(%.2f,%.2f)pix RCorner(%.2f,%.2f)pix" ,
                        m_iNProcessedBlanks , dScaledZ , GetWorkingStateName() ,
                        dScaledZ , cStd.imag() * m_dZScale_um_per_pix ,
                        cAverLCorner.real() , cAverLCorner.imag() , cAverRCorner.real() , cAverRCorner.imag() );
                      SendDisplaceCommand( 0. , 0. , dScaledZ );
                    }
                    break;
                  }
                  if (m_WorkingState != ULS_GrabFinal)
                  {
                    if (!m_bNoXYMeasurement)
                    {
                      SaveLogMsg( "Blank %d dZ=%.1f Activate ULF(%s): Avg(%.2f) Std(%.2f) LCorner(%.2f,%.2f)pix RCorner(%.2f,%.2f)pix" ,
                        m_iNProcessedBlanks , dScaledZ , GetWorkingStateName() ,
                        dScaledZ , cStd.imag() * m_dZScale_um_per_pix ,
                        cAverLCorner.real() , cAverLCorner.imag() , cAverRCorner.real() , cAverRCorner.imag() );
                      FXString Msg;
                      Msg.Format( "measure_and_correct; dZ=%.2f; SE=(%.2f,%.2f); RE=(%.2f,%.2f);" , dScaledZ ,
                        cAverLCorner.real() , cAverLCorner.imag() , cAverRCorner.real() , cAverRCorner.imag() );
                      CTextFrame * pULFOrder = CreateTextFrame( Msg , "FromULS" );
                      PutFrame( GetOutputConnector( 4 ) , pULFOrder );
                      CheckAndSaveFinalImages( pOutFrame , false , "ZPrelim" ) ;
                    }
            else if (m_WorkingState == ULS_ZCorrection  // calibration finished (no XY measurement)
              || m_WorkingState == ULS_ZCorrectionForMeas) 
                    {
                      FXString Result;
                      Result.Format( "Result=%s;dX=%.2f;dY=%.2f;dZ=%.2f;" ,
                        ( m_WorkingState == ULS_ZCorrection ) ? "CalibrationDone" : "Blank" ,
                        0. , 0. , dScaledZ );
                      SaveCSVLogMsg( "  %3d,%6.2f,%6.2f,%6.2f,OK" , m_iNProcessedBlanks ,
                        0. , 0. , dScaledZ );
                      SaveLogMsg( "%s %d final result - %.2f,%.2f,%.2f,OK" ,
                        "Calibration" , -1 , 0. , 0. , dScaledZ );
                      SendMessageToEngine( Result , "EndOfMeasurement" );
                      m_WorkingState = State_Idle ;
                    }
                  }
                  else
                  {
                    if (m_ViewDetails > 2)
                    {
                      pOutFrame->AddFrame( CreatePtFrame( cAverLCorner , GetHRTickCount() , 0x0000ff ) ) ;
                      pOutFrame->AddFrame( CreatePtFrame( cAverRCorner , GetHRTickCount() , 0x0000ff ) ) ;
                      cmplx LViewPt = cAverLCorner + cmplx( -150 , 10 ) ;
                      cmplx RViewPt = cAverRCorner + cmplx( 10 , 10 ) ;
                      pOutFrame->AddFrame( CreateTextFrame( LViewPt , "0x800000" , 12 , "SouthEdge" , 0 , "SE(%.2f,%.2f)pix" ,
                        cAverLCorner.real() , cAverLCorner.imag() ) ) ;
                      pOutFrame->AddFrame( CreateTextFrame( RViewPt , "0x800000" , 12 , "NorthEdge" , 0 , "NE(%.2f,%.2f)pix\nLav=%.2fum" ,
                        cAverRCorner.real() , cAverRCorner.imag() ,
                        ( cAverRCorner.real() - cAverLCorner.real() ) * m_dZScale_um_per_pix ) );
                    }
                    m_dZStdAfterShift_um = dScaledZStd;
                    m_dZAfterShift_um = dScaledZ;

                    CheckAndSaveFinalImages( pOutFrame , false , "ZFinal" ) ;
                  }
                  m_WorkingState = State_Idle;
                }
              }
              else
                GrabImage();
            }
            break;
          case ULS_ZCorrection2:
            if (bExtracted)
            {
              if (m_iNRestSamples <= 0)
              {
                if (GetAverageAndStd( m_IterationResults , cAverage , cStd ))
                {
                  {
                    double dZ = cAverage.imag();
                    double dScaledZ = dZ * m_dZScale_um_per_pix;
                    m_WorkingState = State_Idle;
                    if (!m_bNoXYMeasurement)
                    {
                      CTextFrame * pULFOrder = CreateTextFrame( "corr_gauge_right_side" , "FromULS" );
                      PutFrame( GetOutputConnector( 4 ) , pULFOrder );
                    }
                    else // calibration finished (no XY measurement)
                    {
                      FXString Result;
                      Result.Format( "Result=%s;dX=%.2f;dY=%.2f;dZ=%.2f;" ,
                        "CalibrationDone" ,
                        0. , 0. , dScaledZ );
                      SaveCSVLogMsg( "  %3d,%6.2f,%6.2f,%6.2f,OK" , m_iNProcessedBlanks ,
                        0. , 0. , dScaledZ );
                      SaveLogMsg( "%s %d final result - %.2f,%.2f,%.2f,OK" ,
                        "Calibration" , -1 , 0. , 0. , dScaledZ );
                      SendMessageToEngine( Result , "EndOfMeasurement" );
                      m_WorkingState = State_Idle ;
                    }

                    //               CTextFrame * pResult = CreateTextFrame(
                    //                 "Result=Blank;dX=0.1;dY=-0.1;dZ=10.;", "EndOfMeasurement");
                    //               PutFrame(GetOutputConnector(1), pResult);
                                  // Target position correction order for Z measurements
                  }
                }
              }
              else
                GrabImage();
            }
            break;
          case ULS_ScaleCalib:
            {
              if (bExtracted)
              {
                if (ZCalibrationProcess( pOutFrame , m_ShiftsUL ) == 0)
                {  // Calibration finished
                  double dZ = m_cLastExtractedResult_pix.imag() * m_dZScale_um_per_pix;
                  double dOrderedShift = m_ZNewCalibData[ 0 ].m_dHeight;
                  m_WorkingState = ULS_MoveAfterCalib;
                  SendDisplaceCommand( 0. , 0. , -dOrderedShift );
                  m_ShiftsUL.m_z = 0.;
                }
              }
              else
                SENDERR( "ULS: No results for Z Calibration" );
            }
            break;
          case ULS_ZCorrNoZCalib:
            {
              if (bExtracted)
              {
                double dRelToTarget_pix = ( m_cLastROICenter.imag() + m_cLastExtractedResult_pix.imag() )
                  - m_dTargetZ_pix;
                double dZScaled = dRelToTarget_pix * m_dZScale_um_per_pix;
                m_WorkingState = ULS_MoveAfterCalib;
                SendDisplaceCommand( 0. , 0. , -dZScaled );
                m_ShiftsUL.m_z = 0.;
              }
              else
                SENDERR( "ULS: No results for Z correction without calibration" );
            }
            break ;
        }
      }
      break;
  }
  if (bExtracted)
  {
    cmplx cViewPt = m_cLastROICenter + cmplx( -800. , -m_cLastROICenter.imag() * 0.9 );

    if (m_WorkingState != ULF_MoveAfterCalib
      && m_WorkingState != ULF_MoveAfterMeasurement
      && m_WorkingState != ULF_WaitForExternalReady)
    {
      pOutFrame->AddFrame( FormBigMsg( m_cLastExtractedResult_pix , cViewPt , 20 , pDisplacementViewColor ) );
    }
    else
    {
      cmplx cDistFOV = m_cLastPartMeasuredPt ;
      pOutFrame->AddFrame( FormBigMsg( cDistFOV , cViewPt , 20 , pDisplacementViewColor ) );
    }
  }
  cmplx cScaleViewPt = m_cLastROICenter
    + cmplx( -m_cLastROICenter.real() * 0.9 , m_cLastROICenter.imag() * 0.93 ) ;
  pOutFrame->AddFrame( CreateTextFrame( cScaleViewPt , "0x0000ff" , 12 , "Scale" , 0 ,
    "Scale=%.5f (um/pix)     Part=%s    Version=%s %s" ,
    m_WorkingMode == MPPTWM_UpSide ? m_dZScale_um_per_pix : m_dScale_um_per_pix ,
    ( LPCTSTR ) m_PartName ,
#ifdef _DEBUG
    "Debug"
#else
    "Release"
#endif
    , __TIMESTAMP__ ) ) ;
  // common data will be first and will be drawn first
  // Results will be drawn after and will be better visible
  pOutFrame->AddFrame( pDataFrame );
  return pOutFrame;
}

WhatMPPTFigureFound MPPT::SetDataAboutContour( const CFigureFrame * pFrame )
{
  LPCTSTR pLabel = pFrame->GetLabel();
  bool bIsContur = ( _tcsstr( pLabel , _T( "Contur" ) ) == pLabel );
  bool bIsSegment = ( _tcsstr( pLabel , _T( "Data_Edge" ) ) == pLabel );
  // conturs, lines and edges
  //   laser
  //   rulerv1mm
  //   cavity_bottom
  //   pin_center
  //   pin_edgeup
  //   pin_edgedown
  //   pin_edgeright
  //   pin_edgeleft

  if (bIsContur)
  {
    if (_tcsstr( pLabel , _T( "cavity_bottom" ) ))
    {
      m_CavitiesConturs.AddFrame( pFrame );
      return WFFMPPT_CavityBottom;
    }
    else if (_tcsstr( pLabel , _T( "laser" ) ))
    {
      m_LaserConturs.AddFrame( pFrame );
      return WFFMPPT_LaserSpot;
    }
    //order of calib and calib_observ is important
    else if (_tcsstr( pLabel , _T( "pin_center" ) ))
    {
      return WFFMPPT_CalibXYOnPin;
    }
    else if (_tcsstr( pLabel , _T( "pin_edgeup" ) ))
    {
      //m_LastExtConturs.AddFrame( pFrame ) ;
      return WFFMPPT_CalibXPinUpperEdge;
    }
    else if (_tcsstr( pLabel , _T( "pin_edgeright" ) ))
    {
      //m_LastExtConturs.AddFrame( pFrame ) ;
      return WFFMPPT_CalibXPinUpperEdge;
    }
    else if (_tcsstr( pLabel , _T( "pin_edgedown" ) ))
    {
      //m_LastExtConturs.AddFrame( pFrame ) ;
      return WFFMPPT_CalibXPinUpperEdge;
    }
    else if (_tcsstr( pLabel , _T( "pin_edgeleft" ) ))
    {
      //m_LastExtConturs.AddFrame( pFrame ) ;
      return WFFMPPT_CalibXPinUpperEdge;
    }
    else if (_tcsstr( pLabel , _T( "part_top_prelim" ) ))
      return WFFMPPT_NotFound;
    else if (_tcsstr( pLabel , _T( "part_top" ) ))
    {
      m_PartConturs.AddFrame( pFrame );
      m_cLastConturCenter = FindExtrems(
        pFrame , m_ConturExtremes , NULL , &m_cLastConturSize );
      return WFFMPPT_PartFrontOnLower;
    }
    else if (_tcsstr( pLabel , _T( "part_tip" ) ))
    {
      return WFFMPPT_PartSideOnLower;
    }
    else if (_tcsstr( pLabel , _T( "tip_as_contur" ) ))
    {
      m_PartConturs.AddFrame( pFrame ) ;
      return WFFMPPT_PartSideConturOnLower;
    }
  }
  else if (bIsSegment)
  {
    //     if ( _tcsstr( pLabel , _T( "ext_segment" ) ) )
    //     {
    //       m_LastExtConturs.AddFrame( pFrame ) ;
    //       return WFFMPPT_ExternalContur ;
    //     }
  }
  else
  {
    if (_tcsstr( pLabel , _T( "cavity_bottom" ) ))
    {
      m_CavitiesCenters.AddFrame( pFrame );
      return WFFMPPT_CavityPlaneCenter;
    }
    else if (_tcsstr( pLabel , _T( "pin_center" ) ))
    {
      m_cPinCenter = CDPointToCmplx( pFrame->GetAt( 0 ) );
      m_cLastExtractedResult_pix = m_cPinCenter - m_cLastROICenter;
      return WFFMPPT_CalibXYOnPin;
    }
    else if (_tcsstr( pLabel , _T( "pin_edgeup" ) ))
    {
      cmplx cPt = CDPointToCmplx( pFrame->GetAt( 0 ) );
      m_cLastExtractedResult_pix = cPt - m_cLastROICenter;

      return WFFMPPT_CalibXPinUpperEdge;
    }
    else if (_tcsstr( pLabel , _T( "pin_edgeright" ) ))
    {
      cmplx cPt = CDPointToCmplx( pFrame->GetAt( 0 ) );
      m_cLastExtractedResult_pix = cPt - m_cLastROICenter;
      return WFFMPPT_CalibXPinRightEdge;
    }
    else if (_tcsstr( pLabel , _T( "pin_edgedown" ) ))
    {
      cmplx cPt = CDPointToCmplx( pFrame->GetAt( 0 ) );
      m_cLastExtractedResult_pix = cPt - m_cLastROICenter;
      return WFFMPPT_CalibXPinDownEdge;
    }
    else if (_tcsstr( pLabel , _T( "pin_edgeleft" ) ))
    {
      cmplx cPt = CDPointToCmplx( pFrame->GetAt( 0 ) );
      m_cLastExtractedResult_pix = cPt - m_cLastROICenter;
      return WFFMPPT_CalibXPinLeftEdge;
    }
    else if (_tcsstr( pLabel , _T( "part_tip" ) ))
    {
      cmplx cPt = CDPointToCmplx( pFrame->GetAt( 0 ) ) ;
      m_cLastExtractedResult_pix = cPt - m_cLastROICenter ;
      return WFFMPPT_PartSideOnLower;
    }
    else if (_tcsstr( pLabel , _T( "part_top_prelim" ) ))
      return WFFMPPT_NotFound;
    else if (_tcsstr( pLabel , _T( "part_top" ) ))
    {
      return WFFMPPT_PartFrontOnLower;
    }
  }
  return WFFMPPT_NotFound;
}

int MPPT::SetDataAboutContours( CFramesIterator * pFiguresIterator )
{
  if (!pFiguresIterator)
    return 0;
  DWORD dwFoundMask = 0;
  CFigureFrame * pNewFigure = NULL;
  int iNFound = 0;
  while (pNewFigure = ( CFigureFrame* ) pFiguresIterator->Next())
  {
    //     if (pNewFigure->GetCount() < 4)
    //       continue;
    LogFigure( pNewFigure->GetLabel() , Fig_Touch , __LINE__ , pNewFigure->GetUserCnt() );
    WhatMPPTFigureFound Res = SetDataAboutContour( pNewFigure );
    dwFoundMask |= Res;
    iNFound += ( ( Res & WFFMPPT_Conturs ) != 0 );
    LogFigure( pNewFigure->GetLabel() , Fig_Touch , __LINE__ , pNewFigure->GetUserCnt() );
  }
  m_dwLastFilledConturs = dwFoundMask;
  return iNFound;
}

int MPPT::SetDataAboutContours( const CDataFrame * pDataFrame )
{
  int iNConturs = 0;
  CFramesIterator * pFigIter = pDataFrame->CreateFramesIterator( figure );
  if (pFigIter)
  {
    iNConturs = SetDataAboutContours( pFigIter );
    delete pFigIter;
  }
  else
  {
    const CFigureFrame * pOneFig = pDataFrame->GetFigureFrame();
    if (pOneFig)
      iNConturs = ( SetDataAboutContour( pOneFig ) != WFFMPPT_NotFound ) ? 1 : 0;
  }
  if (iNConturs)
  {
  }
  return iNConturs;
}

void MPPT::ClearConturData()
{
  m_CavitiesConturs.RemoveAll();
  m_LaserConturs.RemoveAll();
  m_CavitiesCenters.RemoveAll();
  m_PartConturs.RemoveAll();
  FR_RELEASE_DEL( m_pSideBlankContur ) ;
}

void MPPT::GrabImage()
{
  switch (m_WorkingMode)
  {
    case MPPTWM_UpSide:
      //     ProgramExposureAndLightParameters( m_iSideExposure_us , m_iSideExposure_us , 10 ) ;
      break;
    case MPPTWM_UpFront:
      //     ProgramExposureAndLightParameters( m_iFrontExposure_us ,
      //       m_iFrontExposure_us , m_iFrontExposure_us ) ;
      break;

  }
  double dNow = GetHRTickCount();
  double dTimeAfterLastOrder = dNow - m_dLastGrabOrderTime;
  if (dTimeAfterLastOrder < 16.66)
    Sleep( ROUND( 16.66 - dTimeAfterLastOrder ) + 1 );
  m_dLastGrabOrderTime = GetHRTickCount();
  //   FXString GrabCommand;
  //   GrabCommand.Format( "set FireSoftTrigger(%d);" , m_iFrameCount );
  //   ProgramCamera( GrabCommand , "GetNextImage" );
  ProgramCamera( "1" , "GetNextImage" );
  SetWatchDog( 3000 );
}

void MPPT::SetLaserMode( LaserExpMode ExpMode , bool bLive )
{
  int iExp = m_iLaserExposure;
  switch (ExpMode)
  {
    case LEM_ForPin: iExp = m_iLaserExposureOnPin; break;
    case LEM_ForCavity: iExp = m_iLaserExposureOnCavity; break;
  }
  //   ProgramExposureAndLightParameters( iExp , 10 , 10 );
  SwitchOnConstantLight( false , false );
  CTextFrame * pTVObjComm = CreateTextFrame( "Task(2)\n" , "SetTask" );
  PutFrame( GetOutputConnector( 3 ) , pTVObjComm );
  CBooleanFrame * pLPFSwitch = CBooleanFrame::Create( false );
  PutFrame( GetOutputConnector( 4 ) , pLPFSwitch );
  SwitchLaser( true );
}

void MPPT::SetCavityMode( bool bXYMeasure , bool bLive )
{
  m_IterationResults.clear();
  m_IterResultLeft.clear();
  m_IterResultRight.clear();
  int iExp = ( bXYMeasure ) ? m_iNewCavityExposure : m_iNewFocusExposure;
  //   if ( bXYMeasure )
  //   {
  //     SwitchOffConstantLight( true , true );                // don't use constant light
  //     ProgramExposureAndLightParameters( iExp , 10 , iExp ); // do use straight light only
  //   }
  CTextFrame * pTVObjComm = CreateTextFrame(
    ( ( bXYMeasure ) ? "Task(0)\n" : "Task(-1)" ) , "SetTask" );
  PutFrame( GetOutputConnector( 3 ) , pTVObjComm );
  CBooleanFrame * pLPFSwitch = CBooleanFrame::Create( !bXYMeasure );
  PutFrame( GetOutputConnector( 4 ) , pLPFSwitch );
  SwitchLaser( false );
  m_bMeasureFocus = !bXYMeasure;
  bool bRing = bXYMeasure ? m_CurrentPart.m_Cavity.m_bCavXYRingLightOn : m_CurrentPart.m_Cavity.m_bCavFocusRingLightOn ;
  bool bStaright = bXYMeasure ? m_CurrentPart.m_Cavity.m_bCavXYStraightLightOn : m_CurrentPart.m_Cavity.m_bCavFocusStraightLightOn ;
  FXString CameraComm;
  CameraComm.Format( "set properties(OutputSelector=0;OutputValue=%s;"
    "Line2=(LineMode_=Output;LineSource_=UserOutput1;);" ,
    ( bRing ) ? "true" : "false" );
  int iRes = ProgramCamera( CameraComm );

  CameraComm.Format( "set properties(OutputSelector=1;OutputValue=%s;"
    "Line3=(LineMode_=Output;LineSource_=UserOutput2;);)" ,
    ( bStaright ) ? "true" : "false" ) ;
  iRes = ProgramCamera( CameraComm );
  ProgramExposure( iExp );
}

void MPPT::SetPinMode( bool bLive , MPPT_State PinPart )
{
  m_IterationResults.clear();
  int iExp = m_iPinExposure;
  LPCTSTR pTask = _T( "Task(1);" );
  FXRegistry Reg( "TheFileX\\Micropoint" );
  if (m_WorkingState == DL_PinZMeasureByDefocus
    || m_WorkingState == DL_LiveVideoPinFocusing)
  {
    iExp = m_iPinDefocusExposure = Reg.GetRegiInt(
      "Parameters" , "PinDefocusingExposure_us" , 350 );
    pTask = _T( "Task(-1);" );
    m_iNewFocusExposure = iExp ;
    int iLightMask = Reg.GetRegiInt(
      "Parameters" , "PinDefocusLightMask" , 2 );
    SwitchOnConstantLight( ( iLightMask & 1 ) != 0 , ( iLightMask & 2 ) != 0 );
  }
  else
  {
    iExp = m_iPinExposure = Reg.GetRegiInt(
      "Parameters" , "PinExposure_us" , 2800 );
    int iLightMask = Reg.GetRegiInt(
      "Parameters" , "PinMeasLightMask" , 2 );
    SwitchOnConstantLight( ( iLightMask & 1 ) != 0 , ( iLightMask & 2 ) != 0 );
  }
  ProgramExposure( iExp );
  CTextFrame * pTVObjComm = CreateTextFrame( pTask , "SetTask" );
  PutFrame( GetOutputConnector( 3 ) , pTVObjComm );
  CBooleanFrame * pLPFSwitch = CBooleanFrame::Create( false );
  PutFrame( GetOutputConnector( 4 ) , pLPFSwitch );
  SwitchLaser( false );
}

void MPPT::SetPinEdgeMode( EDGE_DIR Edge , bool bLive )
{
  LPCTSTR pTask = NULL;
  int iExp = m_iPinExposure;
  FXRegistry Reg( "TheFileX\\Micropoint" );
  int iLightMask = Reg.GetRegiInt(
    "Parameters" , "PinMeasLightMask" , 2 );

  switch (Edge)
  {
    case ED_UP:
      {
        int iROILeft = m_LastROICenter.x - PIN_HORIZ_SIDE_ROI_WIDTH / 2;
        int iRefY = m_LastROICenter.y;
        if (( m_WorkingState != DL_PinNorthSide ) && ( m_WorkingState != DL_LiveVideoPinNorth ))
          iRefY -= ROUND( m_dDLCavityHorizBaseLine / m_dScale_um_per_pix );
        int iROITop = iRefY - PIN_HORIZ_SIDE_ROI_HEIGHT / 2;

        FXString EdgePosition;
        EdgePosition.Format( "name=pin_edgeup;xoffset=%d;yoffset=%d;width=%d;height=%d;" ,
          iROILeft , iROITop , PIN_HORIZ_SIDE_ROI_WIDTH , PIN_HORIZ_SIDE_ROI_HEIGHT );
        SetParametersToTVObject( EdgePosition );
        pTask = _T( "Task(3)\n" );
      }
      break;
    case ED_RIGHT: pTask = _T( "Task(4)\n" ); break;
    case ED_DOWN:
      {
        int iROILeft = m_LastROICenter.x - PIN_HORIZ_SIDE_ROI_WIDTH / 2;
        int iRefY = m_LastROICenter.y;
        if (( m_WorkingState != DL_PinSouthSide ) && ( m_WorkingState != DL_LiveVideoPinSouth ))
          iRefY -= ROUND( m_dDLCavityHorizBaseLine / m_dScale_um_per_pix );
        int iROITop = iRefY - PIN_HORIZ_SIDE_ROI_HEIGHT / 2;

        FXString EdgePosition;
        EdgePosition.Format( "name=pin_edgedown;xoffset=%d;yoffset=%d;width=%d;height=%d;" ,
          iROILeft , iROITop , PIN_HORIZ_SIDE_ROI_WIDTH , PIN_HORIZ_SIDE_ROI_HEIGHT );
        SetParametersToTVObject( EdgePosition );
        pTask = _T( "Task(5)\n" );
      }
      break;
    case ED_LEFT: pTask = _T( "Task(6)\n" ); break;
    case ED_ALL: pTask = _T( "Task(7)\n" ); break;
    case ED_DEFOCUS:
      {
        pTask = _T( "Task(-1)\n" );
        m_iPinDefocusExposure = iExp = Reg.GetRegiInt(
          "Parameters" , "PinDefocusingExposure_us" , 1000 );
        iLightMask = Reg.GetRegiInt(
          "Parameters" , "PinDefocusLightMask" , 2 );
      }
      break;
    default:
      {
        SENDERR( "MPPT::SetPinEdgeMode - unknown edge %d" , ( int ) Edge );
        return;
      }
  }
  m_IterationResults.clear();
  if (Edge != ED_DEFOCUS)
  {
    m_iPinExposure = iExp = Reg.GetRegiInt(
      "Parameters" , "PinExposure_us" , 1500 );
  }
  SwitchOnConstantLight( ( iLightMask & 1 ) != 0 , ( iLightMask & 2 ) != 0 );
  ProgramExposure( iExp );
  CTextFrame * pTVObjComm = CreateTextFrame( pTask , "SetTask" );
  PutFrame( GetOutputConnector( 3 ) , pTVObjComm );
  CBooleanFrame * pLPFSwitch = CBooleanFrame::Create( true );
  PutFrame( GetOutputConnector( 4 ) , pLPFSwitch );
  SwitchLaser( false );
}

bool MPPT::ExtractResultAndCheckForIterationFinish( const CDataFrame * pDataFrame )
{
  cmplx cExtracted( DBL_MAX , DBL_MAX );
  const CFigureFrame * pCenter = NULL;

  FXString CenterName;
  switch (m_WorkingMode)
  {
    case MPPTWM_Down:
      {
        switch (m_WorkingState)
        {
          case DL_PinToCenterForZ:
          case DL_PinZMeasureByDefocus:
          case DL_ScaleCalib: CenterName = ( _T( "pin_center" ) ); break;
          case DL_LiveVideoPinFocusing: CenterName = ( _T( "pin_center" ) ); break;
          case DL_CaptureZWithCorrectHeigth:
          case DL_MeasCavityZ:
          case DL_MeasCavityZMultiple:
          case DL_LaserCalib:
          case DL_LiveVideoLaser: CenterName = ( _T( "laser" ) ); break;
          case DL_LiveVideoPinNorth:
          case DL_PinNorthSide: CenterName = ( _T( "pin_edgeup" ) ); break;
          case DL_LiveVideoPinEast:
          case DL_PinEastSide: CenterName = ( _T( "pin_edgeright" ) ); break;
          case DL_CorrectAfterFinalVision:
          case DL_FinalImageOverPin:
            CenterName = ( ( m_CurrentPart.m_Cavity.m_CavityEdge == CavEdge_Upper_Xc )
              || ( m_CurrentPart.m_Cavity.m_CavityEdge == CavEdge_Upper ) ) ?
              _T( "pin_edgedown" ) : _T( "pin_edgeup" ) ; break ;
          case DL_LiveVideoPinSouth:
          case DL_PinSouthSide:
            {
              CenterName = ( _T( "pin_edgedown" ) ); break;
            }
          case DL_LiveVideoPinWest:
          case DL_PinWestSide: CenterName = ( _T( "pin_edgeleft" ) ); break;
          case DL_CaptureCavityFinalImage:
          case DL_MeasCavityXY:
          case DL_LiveVideoCavity:
            {  // XY measurement has two results and processed separately
              CFramesIterator * pIter = pDataFrame->CreateFramesIterator( figure );
              if (pIter)
              {
                m_LastCavityCenters.clear();
                CFigureFrame * pFigure = NULL;
                cmplx cRight , cLeft;

                while (pFigure = ( CFigureFrame* ) pIter->Next())
                {
                  if (pFigure->Count() == 1  // one point
                    && _tcscmp( pFigure->GetLabel() , _T( "cavity_bottom" ) ) == 0)
                  {
                    cmplx cCavityCenter_pix = CDPointToCmplx( pFigure->GetAt( 0 ) );
                    cCavityCenter_pix -= m_cLastROICenter;
                    m_LastCavityCenters.push_back( cCavityCenter_pix );
                    if (cCavityCenter_pix.real() > cRight.real())
                    {
                      cRight = cCavityCenter_pix;
                      m_IterResultRight.push_back( cRight );
                    }
                    if (cCavityCenter_pix.real() < cLeft.real())
                    {
                      cLeft = cCavityCenter_pix;
                      m_IterResultLeft.push_back( cLeft );
                    }
                  }
                }
                delete pIter;
                if (cLeft.real() && cRight.real())
                {
                  m_cLastLeftCavityCenter_pix = cLeft;
                  m_cLastRightCavityCenter_pix = cRight;
                  m_cLastRightCavityCenter_um = ConvertCoordsRelativeToCenter( cRight );
                  m_cLastLeftCavityCenter_um = ConvertCoordsRelativeToCenter( cLeft );

                  m_iNRestSamples--;
                  return true;
                }
                return false;
              }
            }
            break;
          default:
            break;
        }
      }
      break;
    case MPPTWM_UpFront:
      {
        CenterName = ( _T( "part_top" ) ); break;
      }
      break;
    case MPPTWM_UpSide:
      {
        CenterName = ( _T( "part_tip" ) ); break;
        m_pSideBlankContur = pDataFrame->GetFigureFrame( "tip_as_contur" ) ;
      }
      break;
  }
  if (!CenterName.IsEmpty())
  {
    const CFigureFrame * pCenter =
      ( const CFigureFrame * ) GetFrameWithLabel( pDataFrame ,
        figure , CenterName , WP_Begin ) ;

    if (pCenter)
    {
      cmplx cCent = CDPointToCmplx( pCenter->GetAt( 0 ) );
      if (( m_WorkingMode == MPPTWM_UpSide ) && ( m_WorkingState != ULS_GrabFinal ))
      {
        cmplx cSideCenter( m_cLastROICenter.real() , m_dTargetZ_pix );
        m_cLastExtractedResult_pix = cExtracted = cCent - cSideCenter;
        double dZ = cExtracted.imag() * m_dZScale_um_per_pix ;
        m_dZAfterShift_um = dZ ;
      }
      else
        m_cLastExtractedResult_pix = cExtracted = cCent - m_cLastROICenter;
    }
    if (m_WorkingState == DL_MeasCavityZ)
    {
    }
  }

  if (cExtracted.real() < m_LastROI.right)
  {  // for all cases except Cavity XY measurement
    m_IterationResults.push_back( cExtracted );
    m_iNRestSamples--;
    return true;
  }
  return false;
}


int MPPT::ResetIterations()
{
  m_CommandTimeStamp = GetTimeAsString_ms();
  ClearInputQueue( false ); // remove from input queue frames
                     // only single frames will be left in the queue
  //  m_iFrameCount = 0 ;
  m_iCntForSave = 0;
  m_iAfterCommandSaved = 0;
  m_iNRestSamples = m_iAverager;
  m_iNAllowedBadMeasurementsOnStabilization = m_iAverager;
  m_IterationResults.clear();
  m_PartMeasResult.clear();
  m_PartMeasSizes.clear() ;
  m_IterResultLeft.clear();
  m_IterResultRight.clear();
  m_LeftCornerOnSide.clear();
  m_RightCornerOnSide.clear();
  m_cLastUpperAver_um = m_cLastSelectedAver_um = m_cLastCentAver_um = 0. ;
  return 0;
}

bool MPPT::ProgramExposureAndLightParameters( int iExp_us , int iFirstLightTime_us_24V , int iSecondLightTime_us_5V )
{
  FXString CameraComm;
  CameraComm.Format( "set properties(Shutter_us=%d;"
    "Timer1=(TimerTriggerSource_=ExposureActive;TimerDuration_=%d;);"
    "Timer2=(TimerTriggerSource_=ExposureActive;TimerDuration_=%d;);"
    "Line2=(LineMode_=Output;LineSource_=Timer1Active;);"
    "Line3=(LineMode_=Output;LineSource_=Timer2Active;);" ,
    iExp_us , iFirstLightTime_us_24V , iSecondLightTime_us_5V );
  m_iLastSettledExposure = iExp_us;
  m_dLastExposureSetTime = GetHRTickCount();
  m_bWasGoodFrame = false;

  return ProgramCamera( CameraComm );
}

bool MPPT::ProgramExposure( int iExp_us )
{
  FXString CameraComm;
  CameraComm.Format( "set properties(Shutter_us=%d;);" , iExp_us );
  CTextFrame * pCamCom = CreateTextFrame( CameraComm , "SetExposure" );
  m_iLastSettledExposure = iExp_us;
  m_dLastExposureSetTime = GetHRTickCount();
  m_bWasGoodFrame = false;

  Sleep( 1 );
  return ProgramCamera( CameraComm );
}

bool MPPT::SendDisplaceCommand( double dX , double dY , double dZ )
{
  if (dX < -SHIFT_LIMIT)
  {
    m_TheRestShift.m_x = dX + SHIFT_LIMIT;
    dX = -SHIFT_LIMIT;
  }
  else if (dX > SHIFT_LIMIT)
  {
    m_TheRestShift.m_x = dX - SHIFT_LIMIT;
    dX = SHIFT_LIMIT;
  }
  else
    m_TheRestShift.m_x = 0. ;

  if (dY < -SHIFT_LIMIT)
  {
    m_TheRestShift.m_y = dY + SHIFT_LIMIT;
    dY = -SHIFT_LIMIT;
  }
  else if (dY > SHIFT_LIMIT)
  {
    m_TheRestShift.m_y = dY - SHIFT_LIMIT;
    dY = SHIFT_LIMIT;
  }
  else
    m_TheRestShift.m_y = 0. ;

  if (dZ < -SHIFT_LIMIT)
  {
    m_TheRestShift.m_z = dZ + SHIFT_LIMIT;
    dZ = -SHIFT_LIMIT;
  }
  else if (dZ > SHIFT_LIMIT_Z)
  {
    m_TheRestShift.m_z = dZ - SHIFT_LIMIT_Z;
    dZ = SHIFT_LIMIT_Z;
  }
  else
    m_TheRestShift.m_z = 0. ;

  m_bAdditionalShift = !m_TheRestShift.IsZero() ;
  if (m_bAdditionalShift)
  {
    if (m_WorkingState != State_AddMotion)
    {
      m_PreviousState = m_WorkingState ;
      m_WorkingState = State_AddMotion ;
    }
  }
  else if (m_PreviousState != State_AddMotion
    && m_PreviousState != State_Idle)
  {
    m_WorkingState = m_PreviousState ;
    m_PreviousState = State_Idle ;
  }

  m_EffectiveShift.m_x = dX = GetBoundToTenth( dX ) ;
  m_EffectiveShift.m_y = dY = GetBoundToTenth( dY ) ;
  m_EffectiveShift.m_z = dZ = GetBoundToTenth( dZ ) ;

  FXString ForLAN;
  ForLAN.Format( "Result=Displace;dX=%.1f;dY=%.1f;dZ=%.1f;" ,
    dX , dY , dZ );

  return SendMessageToEngine( ( LPCTSTR ) ForLAN , "ShiftMachine" );
}

bool MPPT::SendDisplaceCommand( cmplx& cMoveVect )
{
  return SendDisplaceCommand( cMoveVect.real() , cMoveVect.imag() , 0. );
}
bool MPPT::SendDisplaceCommand( CCoor3t Shift )
{
  return SendDisplaceCommand( Shift.m_x , Shift.m_y , Shift.m_z );
}
bool MPPT::SendDisplaceCommand( CCoord3 Shift )
{
  return SendDisplaceCommand( Shift.m_x , Shift.m_y , Shift.m_z );
}

cmplx MPPT::CalculateScaling( FXString * pStatistics )
{
  size_t iNPts = m_NewCalibMatrixSize.cx * m_NewCalibMatrixSize.cy;
  if (iNPts == 0 || m_NewCalibData.size() != iNPts)
    return 0.0;
  cmplx cHoriz[ 100 ] , cVert[ 100 ];
  double dHoriz[ 100 ] , dVert[ 100 ];
  double dAngHoriz[ 100 ] , dAngVert[ 100 ];
  OBJECT_ZERO( cHoriz );
  OBJECT_ZERO( cVert );
  OBJECT_ZERO( dHoriz );
  OBJECT_ZERO( dVert );
  OBJECT_ZERO( dAngHoriz );
  OBJECT_ZERO( dAngVert );
  size_t iLastToFirstLineIndexDiff = iNPts - m_NewCalibMatrixSize.cx;

  cmplx cVertAvg , cHorizAvg;
  double dHorizAvg = 0.0 , dVertAvg = 0.0 , dAngHorizAvg = 0.0 , dAngVertAvg = 0.0;

  if (pStatistics)
  {
    *pStatistics += "\n      Vertical Scales\n"
      "#V      Cmplx Scale Vert     dScale(um/pix)  dAng(mRad)\n";
  }
  for (LONG iX = 0; iX < m_NewCalibMatrixSize.cx; iX++)
  {
    cVertAvg += ( cVert[ iX ] = -m_NewCalibData[ iX ].GetConjCScale( m_NewCalibData[ iX + iLastToFirstLineIndexDiff ] ) );
    dVertAvg += ( dVert[ iX ] = abs( cVert[ iX ] ) );
    dAngVertAvg += ( dAngVert[ iX ] = arg( cVert[ iX ] ) );
    if (pStatistics)
    {
      FXString AsText;
      AsText.Format( "%2d  (%12.8f,%12.8f) %12.8f %12.8f\n" ,
        iX , cVert[ iX ].real() , cVert[ iX ].imag() , dVert[ iX ] , dAngVert[ iX ] * 1000. );
      *pStatistics += AsText;
    }
  }
  cVertAvg /= m_NewCalibMatrixSize.cx;
  dVertAvg /= m_NewCalibMatrixSize.cx;
  dAngVertAvg /= m_NewCalibMatrixSize.cx;

  if (pStatistics)
  {
    FXString AsText;
    AsText.Format( "\n cVScaleAvg(%12.8f,%12.8f) dVScaleAvg_um_per_pix=%12.8f dVAngAvg_mRad=%12.8f\n" ,
      cVertAvg.real() , cVertAvg.imag() , dVertAvg , dAngVertAvg * 1000. );
    *pStatistics += AsText;
    *pStatistics += "\n      Horizontal Scales\n"
      "#V      Cmplx Scale Horiz    dScale(um/pix)  dAng(mRad)\n";
  }


  for (LONG iY = 0; iY < m_NewCalibMatrixSize.cy; iY++)
  {
    LONG iFirstIndex = iY * m_NewCalibMatrixSize.cx;
    LONG iLastIndex = iFirstIndex + m_NewCalibMatrixSize.cx - 1;
    cHorizAvg += ( cHoriz[ iY ] = -m_NewCalibData[ iLastIndex ].GetConjCScale( m_NewCalibData[ iFirstIndex ] ) );
    dHorizAvg += ( dHoriz[ iY ] = abs( cHoriz[ iY ] ) );
    cmplx cReverse = -m_NewCalibData[ iFirstIndex ].GetConjCScale( m_NewCalibData[ iLastIndex ] );
    dAngHorizAvg += ( dAngHoriz[ iY ] = arg( cHoriz[ iY ] ) );
    if (pStatistics)
    {
      FXString AsText;
      AsText.Format( "%2d  (%12.8f,%12.8f) %12.8f %12.8f\n" ,
        iY , cHoriz[ iY ].real() , cHoriz[ iY ].imag() , dHoriz[ iY ] , dAngHoriz[ iY ] * 1000. );
      *pStatistics += AsText;
    }
  }
  cHorizAvg /= m_NewCalibMatrixSize.cy;
  dHorizAvg /= m_NewCalibMatrixSize.cy;
  dAngHorizAvg /= m_NewCalibMatrixSize.cy;

  cmplx cDiag1 = -m_NewCalibData.front().GetConjCScale( m_NewCalibData.back() );
  cmplx cDiag2 = -m_NewCalibData[ m_NewCalibMatrixSize.cx - 1 ].GetConjCScale( m_NewCalibData[ iLastToFirstLineIndexDiff ] );

  cmplx cAvgScale = ( cHorizAvg + cVertAvg ) / 2.;
  m_cScale_um_pix = cAvgScale;
  m_dScale_um_per_pix = ( dHorizAvg + dVertAvg ) / 2.;
  if (pStatistics)
  {
    FXString AsText;
    AsText.Format( "\n cHScaleAvg(%12.8f,%12.8f) dHScaleAvg_um_per_pix=%12.8f dHAngAvg_mRad=%12.8f\n"
      "\nAverage Scale is (%12.8f,%12.8f) dAbsScale=%12.8f um/pix    dAvgAngle=%12.8f mRad\n"
      "Diag1 cScale=(%12.8f,%12.8f) dScale=%12.8f um/pix dAngle=%12.8f mRad\n"
      "Diag2 cScale=(%12.8f,%12.8f) dScale=%12.8f um/pix dAngle=%12.8f mRad\n" ,
      cHorizAvg.real() , cHorizAvg.imag() , dHorizAvg , dAngHorizAvg * 1000. ,
      cAvgScale.real() , cAvgScale.imag() , m_dScale_um_per_pix , arg( cAvgScale ) * 1000. ,
      cDiag1.real() , cDiag1.imag() , abs( cDiag1 ) , arg( cDiag1 ) * 1000. ,
      cDiag2.real() , cDiag2.imag() , abs( cDiag2 ) , arg( cDiag2 ) * 1000. );
    *pStatistics += AsText;
  }
  m_CalibMatrixSize = m_NewCalibMatrixSize;
  m_CalibData = m_NewCalibData;
  SendScalesToRender();
  return cAvgScale;
}
bool MPPT::SendScalesToRender()
{
  FXString ForRendering , ScaleForTVObj;
  if (m_WorkingMode == MPPTWM_Down || m_WorkingMode == MPPTWM_UpFront)
  {
    ForRendering.Format( "%.6f,um,%.7f,%.7f,1" ,
      m_dScale_um_per_pix , m_cScale_um_pix.real() , m_cScale_um_pix.imag() );
    CTextFrame * pRenderScale = CreateTextFrame( ForRendering , "Scale&Units" );

    ScaleForTVObj.Format( "Scale(%.6f,um);" , m_dScale_um_per_pix );
    CTextFrame * pTVObjComm = CreateTextFrame( ScaleForTVObj , "SetScale" );
    return PutFrame( GetOutputConnector( 0 ) , pRenderScale )
      && PutFrame( GetOutputConnector( 3 ) , pTVObjComm ) ;
  }
  else if (m_WorkingMode == MPPTWM_UpSide)
  {
    ForRendering.Format( "%.6f,um" , m_dZScale_um_per_pix );
    CTextFrame * pRenderScale = CreateTextFrame( ForRendering , "Scale&Units" );

    ScaleForTVObj.Format( "Scale(%.6f,um);" , m_dZScale_um_per_pix );
    CTextFrame * pTVObjComm = CreateTextFrame( ScaleForTVObj , "SetScale" );
    return PutFrame( GetOutputConnector( 0 ) , pRenderScale )
      && PutFrame( GetOutputConnector( 3 ) , pTVObjComm );
  }
  return false;
}


cmplx MPPT::ConvertCoordsRelativeToCenter( cmplx& cRelToCenter )
{
  cmplx cConverted = conj( cRelToCenter * m_cScale_um_pix );

  return cConverted;
}


bool MPPT::CalcPinCenter()
{
  cmplx cInNorth = ConvertCoordsRelativeToCenter( m_NorthPinSide.FOV )
    + m_NorthPinSide.World;
  cmplx cInEast = ConvertCoordsRelativeToCenter( m_EastPinSide.FOV )
    + m_EastPinSide.World;
  cmplx cInSouth = ConvertCoordsRelativeToCenter( m_SouthPinSide.FOV )
    + m_SouthPinSide.World;
  cmplx cInWest = ConvertCoordsRelativeToCenter( m_WestPinSide.FOV )
    + m_WestPinSide.World;

  cmplx cPinPos( ( cInEast.real() + cInWest.real() ) / 2. ,
    ( cInNorth.imag() + cInSouth.imag() ) / 2. );
  m_CalculatedPinPosition = cPinPos;

  double dHorizPinDiam = abs( cInEast.real() - cInWest.real() );
  double dVertPinDiam = abs( cInNorth.imag() - cInSouth.imag() );

  m_dPinDiam_um = ( dHorizPinDiam + dVertPinDiam ) / 2.;

  return true;
}


bool MPPT::CalcAndAddFocusIndication(
  CContainerFrame * pOutputFrame , CRect * pROI , CRect * pROI2 )
{
  double dLeftLaplace = 0. , dLeftAverage = 0. ;
  double dRightLaplace = 0. , dRightAverage = 0. ;
  int iMinLeft = INT_MAX , iMinRight = INT_MAX , iMaxLeft = INT_MIN , iMaxRight = INT_MIN ;
  CRect LeftArea , RightArea;
  bool bTwoAreas = false;
  if (pROI && pROI2)
  {
    LeftArea = *pROI;
    RightArea = *pROI2;
    bTwoAreas = true;
  }
  else if (!pROI)
  {
    LeftArea = GetRectangle( cmplx( -750. ) , cmplx( 300. , 400. ) );
    RightArea = GetRectangle( cmplx( 750. ) , cmplx( 300. , 400. ) );
    bTwoAreas = true;
  }
  if (bTwoAreas)
  {
    dLeftLaplace = m_dLastLeftFocusIndicator = _calc_laplace( m_pCurrentImage , LeftArea );
    dRightLaplace = m_dLastRightFocusIndicator = _calc_laplace( m_pCurrentImage , RightArea );
    dLeftAverage = _calc_average( m_pCurrentImage , LeftArea , iMinLeft , iMaxLeft ) ;
    dRightAverage = _calc_average( m_pCurrentImage , RightArea , iMinRight , iMaxRight ) ;
    m_dLastFocusIndicator = dLeftLaplace + dRightLaplace;
    cmplx LeftRect[ 5 ] = { cmplx( LeftArea.left , LeftArea.top ) , cmplx( LeftArea.right , LeftArea.top ) ,
      cmplx( LeftArea.right , LeftArea.bottom ) , cmplx( LeftArea.left , LeftArea.bottom ) ,
      cmplx( LeftArea.left , LeftArea.top ) };
    cmplx RightRect[ 5 ] = { cmplx( RightArea.left , RightArea.top ) , cmplx( RightArea.right , RightArea.top ) ,
      cmplx( RightArea.right , RightArea.bottom ) , cmplx( RightArea.left , RightArea.bottom ) ,
      cmplx( RightArea.left , RightArea.top ) };
    CFigureFrame * pLeftAreaView = CreateFigureFrame(
      ( cmplx* ) &LeftRect , 5 , ( DWORD ) ( ( m_WhatSideToUse & 1 ) ? 0x00f000 : 0x0000ff ) );
    CFigureFrame * pRightAreaView = CreateFigureFrame(
      ( cmplx* ) &RightRect , 5 , ( DWORD ) ( ( m_WhatSideToUse & 2 ) ? 0x00f000 : 0x0000ff ) );
    pOutputFrame->AddFrame( pLeftAreaView );
    pOutputFrame->AddFrame( pRightAreaView );
  }
  else
  {
    m_dLastFocusIndicator = _calc_laplace( m_pCurrentImage , *pROI );
    dLeftAverage = _calc_average( m_pCurrentImage , *pROI ) ;
    cmplx ROIRect[ 5 ] = { cmplx( pROI->left , pROI->top ) , cmplx( pROI->right , pROI->top ) ,
      cmplx( pROI->right , pROI->bottom ) , cmplx( pROI->left , pROI->bottom ) ,
      cmplx( pROI->left , pROI->top ) };
    CFigureFrame * pROIAreaView = CreateFigureFrame( ( cmplx* ) &ROIRect , 5 , ( DWORD ) 0x0000ff );
    pOutputFrame->AddFrame( pROIAreaView );
  }
  FXString FocusingAsText , AverageAsText , LogMsg;
  if (bTwoAreas)
  {
    FocusingAsText.Format( _T( "Focus %.2f(L%.2f,R%.2f)" ) ,
      m_dLastFocusIndicator , dLeftLaplace , dRightLaplace );
    AverageAsText.Format( _T( "Aver %.1f(L%.1f,R%.1f)[%d,%d][%d,%d]" ) ,
      0.5 * ( dLeftAverage + dRightAverage ) , dLeftAverage , dRightAverage ,
      iMinLeft , iMaxLeft , iMinRight , iMaxRight ) ;
    if (m_iFocusLogPeriod_samples && ( ++m_iSamplesCnt >= m_iFocusLogPeriod_samples ))
    {
      LogMsg.Format( "Focus,%.2f,%.2f,%.2f,Aver,%.1f,%.1f,%.1f,MinMax,%d,%d,%d,%d;" ,
        m_dLastFocusIndicator , dLeftLaplace , dRightLaplace ,
        0.5 * ( dLeftAverage + dRightAverage ) , dLeftAverage , dRightAverage ,
        iMinLeft , iMaxLeft , iMinRight , iMaxRight );
      m_iSamplesCnt = 0;
    }
  }
  else
  {
    FocusingAsText.Format( _T( "Focus %.2f, Aver = %.2f" ) ,
      m_dLastFocusIndicator , dLeftAverage );
  }

  cmplx cTextPlacement( m_cLastROICenter + ( pROI ? cmplx( -200. , -460. ) : cmplx( -700. , 460. ) ) );
  CTextFrame * pFocusIndicator = CreateTextFrame( cTextPlacement , FocusingAsText ,
    pROI ? "0x00ffff" : ( m_bMeasureFocus ) ? "0x00ffff" : "0x00ffff" ,
    30 , "Focus Indicator" , m_pCurrentImage->GetId() );
  pOutputFrame->AddFrame( pFocusIndicator );
  if (bTwoAreas)
  {
    cTextPlacement._Val[ _IM ] += 80. ;
    CTextFrame * pAverIndicator = CreateTextFrame( cTextPlacement , AverageAsText ,
      pROI ? "0x00ffff" : ( m_bMeasureFocus ) ? "0x00ffff" : "0x00ffff" ,
      16 , "Average Indicator" , m_pCurrentImage->GetId() );
    pOutputFrame->AddFrame( pAverIndicator );
  }
  if (m_iFocusLogPeriod_samples && !LogMsg.IsEmpty())
  {
    m_FocusLogAccumulator = LogMsg ;
  }
  return true;
}


double MPPT::GetdZ( cmplx& cLaserSpotPos )
{
  cmplx cDistToZeroZ = cLaserSpotPos - m_cNominalZZero;
  double dZ = cDistToZeroZ.real() / m_dZSensitivity_pix_per_um;
  return dZ;
}

static TCHAR * Keys[] =
{
  _T( "=Bad" ) ,
  _T( "=Cavity" ) ,
  _T( "=Error" ) ,
  _T( "=Blank" )
} ;

bool MPPT::SendMessageToEngine( LPCTSTR pMessage , LPCTSTR pLabel )
{
  CTextFrame * pResult = CreateTextFrame( pMessage , pLabel );
  bool bRes = PutFrame( GetOutputConnector( 1 ) , pResult );
  LPCTSTR pFound = _tcsstr( pMessage , _T( "Result=" ) ) ;

  FXString ForLog;
  int iCnt = -1;
  if (pFound)
  {
    if (_tcsstr( pMessage , _T( "Cavity" ) ))
      iCnt = m_iNProcessedCavities ;
    else     if (_tcsstr( pMessage , _T( "Blank" ) ))
      iCnt = m_iNProcessedBlanks ;
  }
  ForLog.Format( "To Engine: Lab=%s Msg=%s Cnt=%d" , pLabel , pMessage , iCnt );
  SaveLogMsg( ( LPCTSTR ) ForLog );

  //   if ( pFound == pMessage )
  //   {
  //     for ( int i = 0 ; i < ARRSZ( Keys ) ; i++ )
  //     {
  //       if ( _tcsstr( pMessage , Keys[i] ) )
  //       {
  //         SwitchOffConstantLight( true , true ) ;
  //         break ;
  //       }
  //     }
  //   }
  return bRes ;
}



void MPPT::SwitchLaser( bool bOn )
{
  CTextFrame * pRelayOn = CreateTextFrame(
    bOn ? "Chan=0;Value=1;" : "Chan=0;Value=0;" , "RelayControl" );
  PutFrame( GetOutputConnector( 0 ) , pRelayOn );
  Sleep( 20 );

}



bool MPPT::GetAverageAndStd( CmplxVector& Data , cmplx& cAverage , cmplx& cCmplxStd )
{
  if (Data.empty())
  {
    SENDERR( "No data for averaging: %s-%s " ,
      GetWorkingModeName() , GetWorkingStateName() );
    return false;
  }
  cmplx cAver;
  cmplx cSquares;
  if (Data.size() == 1)
  {
    cAverage = Data[ 0 ];
    cCmplxStd = cmplx();
  }
  else
  {
    for (size_t i = 0; i < Data.size(); i++)
    {
      cmplx cVal = Data[ i ];
      cAver += cVal;
      cSquares += cmplx( cVal.real() * cVal.real() , cVal.imag() * cVal.imag() );
    }
    cAverage = cAver / ( double ) Data.size();
    cSquares /= ( double ) Data.size();
    cCmplxStd = cmplx( sqrt( cSquares.real() - cAverage.real() * cAverage.real() ) ,
      sqrt( cSquares.imag() - cAverage.imag() * cAverage.imag() ) );
  }
  return true;
}


int MPPT::ClearInputQueue( bool bSingle , bool bContainers , bool bVFrames )
{
  Sleep( 25 );
  int iNFrames = 0;
  CDataFrame * p = NULL;
  vector<CDataFrame*> ForResend ;
  while (m_pInput->GetNFramesInQueue())
  {
    if (m_pInput->Get( p ))
    {
      if (p->IsContainer() && !bContainers)
      {
        ForResend.push_back( p ) ;
        continue ;
      }
      else
      {
        datatype Type = p->GetDataType() ;
        if (Type == vframe)
        {
          if (!bVFrames)
          {
            ForResend.push_back( p ) ;
            continue ;
          }
        }
        else if (!bSingle)
        {
          ForResend.push_back( p ) ;
          continue ;
        }
      }
      //       if ( p->GetDataType() == text )
      //       {
      //         CTextFrame * pText = p->GetTextFrame() ;
      //         if ( pText->GetString().MakeLower().Find( "continue_process" ) == 0 )
      //           ProcessContinueCommand() ;
      //       }
      p->Release();
    }
    iNFrames++;
  }
  for (size_t i = 0 ; i < ForResend.size() ; i++)
    m_pInput->Send( ForResend[ i ] ) ;

  return iNFrames;
}


int MPPT::GetStatisticsAboutContursAsSpots( const CDataFrame * pDataFrame )
{
  CFramesIterator * pIt = pDataFrame->CreateFramesIterator( text );
  if (pIt)
  {
    CTextFrame * pNext = ( CTextFrame* ) pIt->Next();
    while (pNext)
    {
      LPCTSTR pLabel = pNext->GetLabel();
      if (_tcsstr( pLabel , _T( "Data_Spot" ) ))
      {
        FXString Data = pNext->GetString();
        FXSIZE iPos = 0;
        FXString Token = Data.Tokenize( "\n" , iPos );
        if (!Token.IsEmpty())
        {
          FXSIZE iNSpotsPos = Token.Find( '=' ) + 1;
          int iNSpots = atoi( ( LPCTSTR ) Token + iNSpotsPos );
          if (iNSpots)
            m_DataForConturs.clear();
          for (int i = 0; i < iNSpots; i++)
          {
            Token = Data.Tokenize( "\n" , iPos );
            if (!Token.IsEmpty())
            {
              CavityParams NewData( ( LPCTSTR ) Token );
              if (NewData.IsInitialized())
              {
                LPCTSTR pName = _tcschr( pLabel , _T( ':' ) );
                if (pName)
                  NewData.m_Name = pName;
                m_DataForConturs.push_back( NewData );
              }
            }
          }
        }
      }
      pNext = ( CTextFrame* ) pIt->Next();
    }

    delete pIt;
  }

  return ( int ) m_DataForConturs.size();
}

bool MPPT::FilterCavitiesContours( CContainerFrame * pOutFrame )
{
  bool bBadWidth = false , bBadHeight = false , bBadArea = false;

  int iNConturs = ( int ) m_CavitiesConturs.Count();
  size_t i = 0;
  for (; i < m_DataForConturs.size(); i++)
  {
    double dAreaDiff = m_DataForConturs[ i ].m_dPlaneArea_um2 - m_CurrentPart.m_Cavity.m_dPlaneArea_um2;
    double dWidthDIff = m_DataForConturs[ i ].m_dPlaneWidth_um - m_CurrentPart.m_Cavity.m_dPlaneWidth_um;
    double dHeightDiff = m_DataForConturs[ i ].m_dPlaneHeight_um - m_CurrentPart.m_Cavity.m_dPlaneHeight_um;
    if (abs( dAreaDiff / m_CurrentPart.m_Cavity.m_dPlaneArea_um2 ) > m_dArea_Tolerance
      || abs( dWidthDIff / m_CurrentPart.m_Cavity.m_dPlaneWidth_um ) > m_dWidth_Tolerance
      || abs( dHeightDiff / m_CurrentPart.m_Cavity.m_dPlaneHeight_um ) > m_dHeight_Tolerance)
    {
      if (--iNConturs < 2)
      {
        bBadArea = abs( dAreaDiff / m_CurrentPart.m_Cavity.m_dPlaneArea_um2 ) > m_dArea_Tolerance;
        bBadWidth = abs( dWidthDIff / m_CurrentPart.m_Cavity.m_dPlaneWidth_um ) > m_dWidth_Tolerance;
        bBadHeight = abs( dHeightDiff / m_CurrentPart.m_Cavity.m_dPlaneHeight_um ) > m_dHeight_Tolerance;
        break;
      }
      int iIndex = ( int ) ( iNConturs - i - 1 );
      m_CavitiesConturs.RemoveFrame( iIndex );
      m_CavitiesCenters.RemoveFrame( iIndex );
      m_DataForConturs.erase( m_DataForConturs.begin() + i );
      i--;
    }
  }
  if (m_CavitiesConturs.Count() == 2)
  {
    m_dLastCavityArea_um2 = 0.5
      *( m_DataForConturs[ 0 ].m_dPlaneArea_um2 + m_DataForConturs[ 1 ].m_dPlaneArea_um2 );
    return true;
  }
  cmplx Pt = m_cLastMeasCenter - cmplx( 300. , 50 );
  FXString Msg , Addition;
  if (bBadArea)
  {
    Addition.Format( "Area Error(need %.1f) " ,
      m_CurrentPart.m_Cavity.m_dPlaneArea_um2 / ( m_dScale_um_per_pix * m_dScale_um_per_pix ) );
    Msg += Addition;
  }
  if (bBadWidth)
  {
    Addition.Format( "Width Error(need %.1f) " ,
      m_CurrentPart.m_Cavity.m_dPlaneWidth_um );
    Msg += Addition;
  }
  if (bBadHeight)
  {
    Addition.Format( "Height Error(need %.1f) " ,
      m_CurrentPart.m_Cavity.m_dPlaneHeight_um );
    Msg += Addition;
  }
  pOutFrame->AddFrame( CreateTextFrame( Pt , ( LPCTSTR ) Msg , "0x0000ff" , 14 ) );
  if (m_WorkingState != State_Unknown)
  {
    SaveCSVLogMsg( " %4d,     0,     0,    0,bad XY contours" , m_iNProcessedCavities );
    SaveLogMsg( "Cavity %d - XY Result=Error: "
      "Can't recognize cavity (bad contours);!!!!!!!!!!!!!!!!!!!!!!!!!" ,
      m_iNProcessedCavities );
  }

  return false;
}


CDataFrame *  MPPT::FormBigMsg( cmplx& cComplex , cmplx cPtOnImage ,
  int iSize , LPCTSTR pColor , LPCTSTR pFormat )
{
  cmplx cConverted;
  switch (m_WorkingMode)
  {
    case MPPTWM_UpFront:
    case MPPTWM_Down:
      cConverted = ConvertCoordsRelativeToCenter( cComplex );
      break;
    case MPPTWM_UpSide:
      cConverted = cComplex * m_dZScale_um_per_pix;
      cConverted._Val[ _IM ] = -cConverted._Val[ _IM ];
      break;
  }
  FXString BigMsg;
  BigMsg.Format( "Dpix(%.2f,%.2f)\nDum(%.2f,%.2f)" ,
    cComplex.real() , cComplex.imag() , cConverted.real() , cConverted.imag() );
  if (cPtOnImage.real() == 0.)
    cPtOnImage = m_cLastROICenter + cmplx( -700. , -m_cLastROICenter.imag() * 0.9 );
  CTextFrame * pBigMsg = CreateTextFrame( cPtOnImage , BigMsg , pColor , iSize , "BigMsg" );
  return pBigMsg;
}

CDataFrame *  MPPT::FormTextFrameForView( cmplx& cViewPt ,
  int iFontSize , LPCTSTR pColor , LPCTSTR pFormat , ... )
{
  va_list argList;
  va_start( argList , pFormat );
  FXString Out;
  Out.FormatV( pFormat , argList );
  va_end( argList );
  CTextFrame * pViewText = CreateTextFrame( cViewPt , Out , pColor , iFontSize , "ViewText" );
  return pViewText;
}

int MPPT::DrawStandardGraphics( CContainerFrame * pOutFrame )
{
  m_LastROI.left = m_LastROI.top = 0;
  m_LastROI.right = GetWidth( m_pCurrentImage );
  m_LastROI.bottom = GetHeight( m_pCurrentImage );
  m_LastROICenter = CPoint( m_LastROI.right / 2 , m_LastROI.bottom / 2 );
  m_cLastROICenter = cmplx( m_LastROICenter.x , m_LastROICenter.y );
  // on following line the negative m_iHorizLinePosShift will move line to up direction
  m_cLastMeasCenter = m_cLastROICenter ;
  switch (WhatWeSee())
  {
    case MO_Pin:
      m_cLastMeasCenter -= cmplx( 0. , m_dDLCavityHorizBaseLine / m_dScale_um_per_pix );
      break ;
    case MO_Cavity:
      m_cLastMeasCenter -= cmplx( 0. , m_dDLCavityHorizBaseLine / m_dScale_um_per_pix );
      break ;
  }
  if (m_ViewDetails > 0)
  {
    // Draw center cross over full screen
    cmplx cCentUp( m_cLastROICenter + cmplx( 0. , -m_cLastROICenter.imag() ) );
    cmplx cCentDown( m_cLastROICenter + cmplx( 0. , m_cLastROICenter.imag() ) );
    cmplx cCentLeft( m_cLastROICenter - m_cLastROICenter.real() );
    cmplx cCentRight( m_cLastROICenter + m_cLastROICenter.real() );
    CFigureFrame * pVertical = CreateLineFrame( cCentUp , cCentDown , 0xc0c000 );
    CFigureFrame * pHorizontal = CreateLineFrame( cCentLeft , cCentRight , 0xc0c000 );
    pOutFrame->AddFrame( pVertical );
    pOutFrame->AddFrame( pHorizontal );
    switch (m_WorkingMode)
    {
      case MPPTWM_UpSide:
        {
          pOutFrame->AddFrame( CreateLineFrame( cmplx( 0. , m_dTargetZ_pix ) ,
            cmplx( m_cLastROICenter.real()*2. , m_dTargetZ_pix ) , 0xff00ff ) );
        }
        break;
      case MPPTWM_UpFront:
        {
          double dBasePos_pix = m_cPartPatternPt.imag() ;
          dBasePos_pix += m_cLastROICenter.imag();
          pOutFrame->AddFrame( CreateLineFrame( cmplx( 0. , dBasePos_pix ) ,
            cmplx( m_cLastROICenter.real()*2. , dBasePos_pix ) , 0xff00ff ) );
        }
        break;
    }
    if (m_ViewDetails > 5)
    {
      if (m_iDistToCenterForVertLines)
      {
        // Draw vertical lines for manual positioning over cavity
        double dHShift_pix = m_iDistToCenterForVertLines / m_dScale_um_per_pix;
        cmplx cLeftUp( m_cLastROICenter + cmplx( -dHShift_pix , -m_cLastROICenter.imag() ) );
        cmplx cLeftDown( m_cLastROICenter + cmplx( -dHShift_pix , m_cLastROICenter.imag() ) );
        CFigureFrame * pVLeft = CreateLineFrame( cLeftUp , cLeftDown , 0x0000ff );
        pOutFrame->AddFrame( pVLeft );
        cmplx cRightUp( m_cLastROICenter + cmplx( dHShift_pix , -m_cLastROICenter.imag() ) );
        cmplx cRightDown( m_cLastROICenter + cmplx( dHShift_pix , m_cLastROICenter.imag() ) );
        CFigureFrame * pVRight = CreateLineFrame( cRightUp , cRightDown , 0x0000ff );
        pOutFrame->AddFrame( pVRight );
      }
      // Draw Horizontal base line 
      cmplx cHLeft( 0. , m_cLastMeasCenter.imag() );
      cmplx cHRight( m_LastROI.Width() , m_cLastMeasCenter.imag() );
      CFigureFrame * pHorizontal = CreateLineFrame( cHLeft , cHRight , 0xff00ff );
      pOutFrame->AddFrame( pHorizontal );
    }
    pOutFrame->AddFrame( CreateTextFrame(
      cmplx( m_LastROI.right - 300. , m_LastROI.bottom - 100 ) , "0x0000ff" , 12 ,
      "TSView" , 0 , "%s" , ( LPCTSTR ) GetTimeAsString_ms() ) ) ;
  }
  return 0;
}

int MPPT::XYCalibrationProcess( CContainerFrame * pOutFrame , CCoor3t& Shifts )
{
  if (m_iNRestSamples <= 0)
  {
    if (m_iCalibCntr++ == -1) // first motion to the left corner position
    {
      FXRegistry Reg( "TheFileX\\Micropoint" );
      FXString Name( GetWorkingModeName() );
      Name += _T( "XYCalibStep_um" );
      m_dCalibStep = Reg.GetRegiDouble(
        "Data" , Name , 100. );
      Shifts.m_x = m_dCalibStep * ( m_CalibMatrixSize.cx / 2 );
      Shifts.m_y = -m_dCalibStep * ( m_CalibMatrixSize.cy / 2 );
      SendDisplaceCommand( Shifts.m_x , Shifts.m_y , 0. );
      m_IterationResults.clear();
      m_iNRestSamples = m_iAverager;
      return 1;
    }
    else
    {
      cmplx cAverage , cStd;

      if (GetAverageAndStd( m_IterationResults , cAverage , cStd ))
      {
        cmplx World( Shifts.m_x , Shifts.m_y );
        cmplx cDiff = cAverage - m_cLastExtractedResult_pix;
        CoordsCorresp NewPair( cAverage , World );
        m_NewCalibData.push_back( NewPair );

        cmplx cViewPt = m_cLastROICenter
          + cmplx( 300. , -m_cLastROICenter.imag() * 0.8 );
        pOutFrame->AddFrame( FormTextFrameForView(
          cViewPt , 20 , "0x000080" , "Pos pix\n%.2f,%.2f\nStd=(%.3f,%.3f)" ,
          cAverage.real() , cAverage.imag() , cStd.real() , cStd.imag() ) );

        ResetIterations();
        if (m_WorkingMode == MPPTWM_UpFront)
          InitZGrabForULF() ;


        if (m_iCalibCntr < m_CalibMatrixSize.cx * m_CalibMatrixSize.cy)
        {
          if (m_iCalibCntr % m_CalibMatrixSize.cx) // end of row?
          {
            Shifts.m_x -= m_dCalibStep; // no
            SendDisplaceCommand( -m_dCalibStep , 0. , 0. );
          }
          else // next row
          {
            double dXShift = m_dCalibStep * ( m_CalibMatrixSize.cx - 1 );
            Shifts.m_x += dXShift;
            Shifts.m_y += m_dCalibStep;
            SendDisplaceCommand( dXShift , m_dCalibStep , 0. );
          }
          return 1; // continue calibration
        }
        else
        {
          FXString sStatistics;
          CalculateScaling( &sStatistics );
          SaveXYCalibData( &sStatistics );
        }
      }
      else
      {
        FxSendLogMsg( MSG_ERROR_LEVEL , _T( "MPP XY calibration" ) , 0 ,
          _T( "Final statistics error" ) );

      }
    }
    return 0; // calibration is finished
  }
  else
    GrabImage();
  return 1; // continue calibration
}

int MPPT::ZCalibrationProcess( CContainerFrame * pOutFrame , CCoor3t& m_Shifts )
{
  {
    if (m_iNRestSamples <= 0)
    {
      bool bLaserCalib = ( m_WorkingState == DL_LaserCalib );
      cmplx cAverage , cStd;
      if (GetAverageAndStd( m_IterationResults , cAverage , cStd ))
      {
        if (m_iCalibCntr++ == -1) // first calibration step
        {                           // Init variables
          FXRegistry Reg( "TheFileX\\Micropoint" );
          FXString ModeName( GetWorkingModeName() );
          FXString Name = ModeName + _T( "ZCalibStep_um" );
          m_dZCalibStep = Reg.GetRegiDouble( "Parameters" , Name , bLaserCalib ? 10. : 25. );
          Name = ModeName + _T( "ZCalibRange_um" );
          m_dZCalibRange = Reg.GetRegiDouble( "Parameters" , Name , bLaserCalib ? 100. : 400. );
          m_ZNewCalibData.clear();

          if (!bLaserCalib) // Move to "Zero" height for side camera of up looking
            SendDisplaceCommand( 0. , 0. , cAverage.imag() );
          else
            GrabImage();

          m_Shifts.m_z = 0.; // now we are on zero height
          ResetIterations();
          return 1;
        }
        else
        {
          if (m_WorkingMode == MPPTWM_UpSide)
          {
            CTextFrame * pULFOrder = CreateTextFrame( "Grab" , "FromULS" );
            PutFrame( GetOutputConnector( 4 ) , pULFOrder );
          }

          //          pDisplacementViewColor = "0x00ff00" ;
          //           const CFigureFrame * pCenterPtFrame = pDataFrame->GetFigureFrame( "laser" ) ;
          HeightMeasResult NewZResult( m_Shifts.m_z , cAverage );
          if (m_Shifts.m_z >= 0)
            m_ZNewCalibData.push_back( NewZResult );
          else
            m_ZNewCalibData.insert( m_ZNewCalibData.begin() , NewZResult );

          if (m_Shifts.m_z == 0.)
            m_cNominalZZero = cAverage; // save position with zero Z delta

          cmplx cViewPt = m_cLastROICenter
            + cmplx( 300. , -m_cLastROICenter.imag() * 0.8 );
          pOutFrame->AddFrame( FormTextFrameForView(
            cViewPt , 20 , "0xffff00" , "Pos pix\n%.2f\nStd=%.3f" ,
            bLaserCalib ? cAverage.real() : cAverage.imag() ,
            bLaserCalib ? cStd.real() : cStd.imag() ) );

          ResetIterations();
          double dZStep = ( m_Shifts.m_z >= 0. ) ?
            ( ( m_Shifts.m_z <= m_dZCalibRange ) ? m_dZCalibStep : -m_Shifts.m_z - m_dZCalibStep )
            : ( m_Shifts.m_z >= -m_dZCalibRange ) ? -m_dZCalibStep : 0.;
          if (dZStep != 0.)
          {
            SendDisplaceCommand( 0. , 0. , dZStep );
            m_Shifts.m_z += dZStep;
            return 1; // continue calibration
          }
          else // End of calibration
          {
            m_LastLaserSpotsPositions.clear();
            double dMinDist = DBL_MAX;
            size_t iMinIndex = 0;
            for (size_t i = 0; i < m_ZNewCalibData.size(); i++)
            {
              double dDistToCenter = abs( m_ZNewCalibData[ i ].m_cFOV );
              if (dDistToCenter < dMinDist)
              {
                dMinDist = dDistToCenter;
                iMinIndex = i;
              }
              m_LastLaserSpotsPositions.push_back( m_ZNewCalibData[ i ].m_cFOV );
            }
            m_ZCalibData = m_ZNewCalibData;
            m_iZeroIndex = ( ( int ) m_ZNewCalibData.size() / 2 );
            m_cNominalZZero = m_ZCalibData[ m_iZeroIndex ].m_cFOV;
            CFRegression ZRegression;
            cmplx cFirst = m_ZNewCalibData.front().m_cFOV;
            for (size_t i = 1; i < m_ZCalibData.size(); i++)
            {
              double dDistToFirst = abs( m_ZNewCalibData[ i ].m_cFOV - cFirst );
              cmplx RegrPt( m_ZNewCalibData[ i ].m_dHeight , dDistToFirst );
              ZRegression.Add( RegrPt );
            }
            ZRegression.Calculate();
            m_dZSensitivity_pix_per_um = ZRegression.m_da;
            m_dZScale_um_per_pix = 1. / m_dZSensitivity_pix_per_um;
            double dNominalZ = m_ZNewCalibData[ m_iZeroIndex ].m_dHeight;
            double dZ = dNominalZ - m_Shifts.m_z;

            SaveZCalibData();

            return 0; // calibration finished
          }
        }
      }
      else
      {
        FxSendLogMsg( MSG_ERROR_LEVEL , _T( "MPP Z calibration" ) , 0 ,
          _T( "Error on averaging Z=%.1f, step %d" ) , m_Shifts.m_z , m_iCalibCntr );
      }
    }
    else
      GrabImage();
  }
  return 1; // continue calibration
}

bool MPPT::ProcessContours()
{
  bool bCavity =
    ( m_WorkingState == DL_MeasCavityXY )
    || ( m_WorkingState == DL_LiveVideoCavity )
    || ( m_WorkingState == DL_MeasCavity );
  FramesCollection & OriginContours =
    ( bCavity ) ? m_CavitiesConturs : m_LaserConturs;
  int iNConturs = ( int ) OriginContours.Count();
  int iNSpots = ( int ) m_DataForConturs.size();
  //  ASSERT( iNSpots == iNConturs ) ;
  if (iNSpots && iNConturs && ( iNSpots == iNConturs ))
  {
    CmplxArray Extremes;
    for (int i = 0; i < iNConturs; i++)
    {
      int iIndexForData = iNConturs - i - 1;
      const CFigureFrame * pFig = ( const CFigureFrame * ) OriginContours.GetFrame( 0 );
      m_DataForConturs[ iIndexForData ].m_cCenterAsFigure = FindExtrems( pFig ,
        Extremes , NULL , &( m_DataForConturs[ iIndexForData ].m_cSizeAsFigure ) );

      m_DataForConturs[ iIndexForData ].m_Extremes.resize( 4 );
      for (int j = 0; j < 4; j++)
        m_DataForConturs[ iIndexForData ].m_Extremes[ j ] = Extremes[ j ];

      // Shift coordinates to relatively to center
      m_DataForConturs[ iIndexForData ].m_cCenterAsFigure -= m_cLastROICenter;
      m_DataForConturs[ iIndexForData ].m_cCenterAsSpot -= m_cLastROICenter;
      m_DataForConturs[ iIndexForData ].m_cSizeAsFigure *= m_dScale_um_per_pix;
      m_DataForConturs[ iIndexForData ].m_dPlaneHeight_um *= m_dScale_um_per_pix;
      m_DataForConturs[ iIndexForData ].m_dPlaneWidth_um *= m_dScale_um_per_pix;
      // convert area to square microns
      m_DataForConturs[ iIndexForData ].m_dPlaneArea_um2 *= m_dScale_um_per_pix * m_dScale_um_per_pix;
    }
    return true;
  }
  return false;
}

int MPPT::AccumulateDataAboutBlank( CContainerFrame * pOutFrame )
{
  const CFigureFrame * pFig = ( const CFigureFrame* ) m_PartConturs.GetFrame( 0 );
  cmplx cCent = m_cLastExtractedResult_pix + m_cLastROICenter;
  if (!pFig)
    return -1 ;
  FXString Label = pFig->GetLabel();
  if (Label.Find( "part_top_prel" ) >= 0)
    return -1;

  bool bIsMeasured = MeasureAndFilterHorizAndVertEdges( pFig , cCent ,
    m_cLastConturSize , pOutFrame , &m_BlankCentersAndCorners , m_dGoodStdThres_pix );
  if (bIsMeasured)
  {
    cmplx SelectedPt;
    SelectedPt = m_BlankCentersAndCorners[ ( int ) m_UsedBlankEdge ];
    double dMaxDeviation = max( m_CurrentPart.m_Blank.m_dBlankWidth_um , m_CurrentPart.m_Blank.m_dBlankHeight_um );
    dMaxDeviation /= m_dScale_um_per_pix;
    if (abs( SelectedPt - m_cLastROICenter - m_cPartPatternPt ) > dMaxDeviation)
      return -2 ; // some strange result
    m_PartMeasResult.push_back( SelectedPt );

    if (m_iNRestSamples <= 0)
    {
      if (GetAverageAndStd( m_PartMeasResult , m_cLastBlankAvPos_pix , m_cLastBlankStd_pix ))
      {
        cmplx World( m_ShiftsUL.m_x , m_ShiftsUL.m_y );
        //       //                CoordsCorresp NewPair( m_cLastExtractedResult_pix , World ) ;
        //       CoordsCorresp NewPair( m_cLastBlankAvPos , World );
        //       m_NorthPinSide = NewPair;??

        m_cLastPartMeasuredPt = ( m_cLastBlankAvPos_pix - m_cLastROICenter ) - m_cPartPatternPt;
        cmplx cViewPt = m_cLastROICenter
          + cmplx( 400. , -m_cLastROICenter.imag() * 0.8 );
        m_cLastBlankStd_um = m_cLastBlankStd_pix * m_dScale_um_per_pix ;
        m_cLastBlankAvPos_um = ConvertCoordsRelativeToCenter( m_cLastPartMeasuredPt ) ;
        double dZ_um = GetZResult();
        pOutFrame->AddFrame( FormTextFrameForView(
          cViewPt , 14 , "0x00ff00" , "Pos um\n%.2f,%.2f,%.2f\nStd=(%.3f,%.3f,%.3f)" ,
          m_cLastBlankAvPos_um.real() , m_cLastBlankAvPos_um.imag() , dZ_um ,
          m_cLastBlankStd_um.real() , m_cLastBlankStd_um.imag() , m_dZStdAfterShift_um ) );
        CPoint Corner = GetCPoint( m_cLastBlankAvPos_pix ) - CPoint( 20 , 20 ) ;
        CRect MarkRect( Corner , CSize( 40 , 40 ) ) ;
        pOutFrame->AddFrame( CreateRectFrame( MarkRect , "0xff4040" , "SeletedPointMark" ) ) ;
        return 0;
      }
    }
  }
  GrabImage();
  return 1;
}


cmplx MPPT::GetShiftForNextZMeasurement()
{
  cmplx cStep;
  int iNMeasured = ( int ) m_ZMeasurements.size();
  if (iNMeasured < m_iNCavityZMeasurements)
  {
    double dStep = m_cCavityZMeasurementStep_um.real() * ( double ) iNMeasured;
    if (!( iNMeasured & 1 ))
      dStep = -dStep;
    cStep._Val[ _IM ] = dStep;
    m_cCavityZMeasurementShift_um += cStep;
  }

  return cStep;
}

// Select laser spot from measured spots and return dZ
double MPPT::SelectLaserSpot( cmplx& cSelectedPos )
{
  int iIndex = -1;
  double dNearestDist = DBL_MAX;
  auto it = m_DataForConturs.begin();
  for (; it != m_DataForConturs.end(); it++)
  {
    double dDistToZero = fabs( it->m_cCenterAsSpot.real() - m_cNominalZZero.real() );
    if (dDistToZero < dNearestDist)
    {
      iIndex = ( int ) ( it - m_DataForConturs.begin() );
      dNearestDist = dDistToZero;
    }
  }

  m_DataForConturs[ 0 ] = m_DataForConturs[ iIndex ];
  m_DataForConturs.resize( 1 );
  double dZ = GetdZ( m_DataForConturs[ 0 ].m_cCenterAsSpot );
  cSelectedPos = m_DataForConturs[ 0 ].m_cCenterAsSpot;

  return dZ;
}


DL_ZMeasurementMethod MPPT::AnalyzeAndGetDLMeasurementZMethod()
{
  FXRegistry Reg( "TheFileX\\Micropoint" );
  FXString ZMeasMethod = Reg.GetRegiString(
    "Parameters" , "ZMeasurementMethod" , "Laser" );
  m_bMeasureZ = TRUE;
  if (ZMeasMethod == "Laser")
    return DLZ_Laser;
  else if (ZMeasMethod == "Defocus")
  {
    int m_iZMeasurementPeriod = Reg.GetRegiInt(
      "Parameters" , "ZMeasurementPeriod(0-1stOnly)" , 0 );
    if (m_iZMeasurementPeriod == 0)
    {
      if (m_iNZMeasured >= 1)
      {
        m_bMeasureZ = FALSE;
        return DLZ_NoZMeasurement;
      }
      return DLZ_Defocusing;
    }
    else if (m_iZMeasurementPeriod == 1)
      return DLZ_Defocusing;
    else if (( m_iNZMeasured % m_iZMeasurementPeriod ) != 0)
    {
      m_bMeasureZ = FALSE;
      return DLZ_NoZMeasurement;
    }
    return DLZ_Defocusing;
  }
  else if (ZMeasMethod == "LongSweep")
    return DLZ_LongSweep;
  else if (ZMeasMethod == "ShortSweep")
    return DLZ_ShortSweep;
  else if (ZMeasMethod == "NoZMeasurement")
  {
    m_bMeasureZ = FALSE;
    return DLZ_NoZMeasurement;
  }


  SEND_GADGET_ERR( "Unknown Z measurement method '%s'"
    "('Laser' or 'Defocus' available)" ,
    ( LPCTSTR ) ZMeasMethod );
  return DLZ_Unknown;
}


CRect MPPT::GetNormFocusRectangle( cmplx cCenter , cmplx cSize )
{
  cmplx OnImage = cCenter + m_cLastROICenter;
  CRect Result( ROUND( OnImage.real() - 0.5 * m_cNormZMeasArea.real()  * cSize.real() ) ,
    ROUND( OnImage.imag() - 0.5 * m_cNormZMeasArea.imag() * cSize.imag() ) ,
    ROUND( OnImage.real() + 0.5 * m_cNormZMeasArea.real() * cSize.real() ) ,
    ROUND( OnImage.imag() + 0.5 * m_cNormZMeasArea.imag() * cSize.imag() ) );
  return Result;
}
CRect MPPT::GetRectangle( cmplx cCenter , cmplx cSize )
{
  cmplx OnImage = cCenter + m_cLastROICenter;
  CRect Result( ROUND( OnImage.real() - 0.5 * cSize.real() ) ,
    ROUND( OnImage.imag() - 0.5 * cSize.imag() ) ,
    ROUND( OnImage.real() + 0.5 * cSize.real() ) ,
    ROUND( OnImage.imag() + 0.5 * cSize.imag() ) );
  return Result;
}

bool MPPT::GetFocusRectanglesForCavity( CRect& AreaLeft , CRect& AreaRight )
{
  if (!m_CurrentPart.m_Name.empty())
  {
    double dPix_per_um = 1. / m_dScale_um_per_pix;
    int iWidth = ROUND( m_CurrentPart.m_Cavity.m_dPlaneWidth_um * dPix_per_um / 2. );
    int iHeight = ROUND( m_CurrentPart.m_Cavity.m_dPlaneHeight_um * dPix_per_um * 2. / 3. );
    int iDistFromCenter = ROUND( dPix_per_um * ( m_CurrentPart.m_Cavity.m_dDistBetweenAreas_um + m_CurrentPart.m_Cavity.m_dPlaneWidth_um ) / 2. );
    AreaLeft = CRect( m_LastROICenter.x - iDistFromCenter - ( iWidth / 2 ) ,
      m_LastROICenter.y - ( iHeight / 2 ) ,
      m_LastROICenter.x - iDistFromCenter + ( iWidth / 2 ) ,
      m_LastROICenter.y + ( iHeight / 2 ) );
    AreaRight = AreaLeft;
    AreaRight.OffsetRect( iDistFromCenter * 2 , 0 );
    return true;
  }

  return false;
}

int MPPT::InitZDefocusingMeasurement()
{
  FXRegistry Reg( "TheFileX\\Micropoint" );
  m_dZStepForHighResolutionDefocusing_um = Reg.GetRegiDouble(
    "Parameters" , "ZStepForHighResolutionDefocusing_um" , 10. );
  m_dForFastZBigStep_um = Reg.GetRegiDouble(
    "Parameters" , "FastZBigStep_um" , 50. );
  m_dForFastZSmallStep_um = Reg.GetRegiDouble(
    "Parameters" , "FastZSmallStep_um" , 20. );
  m_iNZShouldBeMeasuredWithHighResolution = Reg.GetRegiInt(
    "Parameters" , "NZHighResolutionMeasurements" , 10 );
  m_iZMeasurementPeriod = Reg.GetRegiInt(
    "Parameters" , "ZMeasurementPeriod(0-1stOnly)" , 0 );
  m_dDefocusThreshold = Reg.GetRegiDouble(
    "Parameters" , "DefocusingThreshold" , 0.8 );
  m_dTargetForFocusExpAdjust = Reg.GetRegiDouble(
    "Parameters" , "TargetForFocusExpAdjust" , 0.9 );
  m_dDLZLongRange_um = Reg.GetRegiDouble( "Parameters" , "DefocusingRange" , 170 );
  m_iDLZSweepCount = 0;
  m_WhatSideToUseForCavity = ( DL_WhatSideToUseForZ ) Reg.GetRegiInt( "Parameters" ,
    "WhatCavZUse(1-L,2-R,3-Both)" , ( int ) DLWSZ_Right );
  m_WhatSideToUseForPin = ( DL_WhatSideToUseForZ ) Reg.GetRegiInt( "Parameters" ,
    "WhatPinZUse(1-L,2-R,3-Both)" , ( int ) DLWSZ_Right );
  m_iPassCount = 0;
  m_dAverageFocusIndicator = m_dLeftAverageFocusIndicator =
    m_dRightAverageFocusIndicator = 0.;
  m_dMaxFocusValue = m_dMaxLeftFocusValue = m_dMaxRightFocusValue = -DBL_MAX;
  m_iNMaxAttempts = Reg.GetRegiInt( "Data" , "NExpAdjustAttempts" , 10 );
  m_iNAttempts = 0 ;
  if (m_WorkingState == DL_PinToCenterForZ)
  {
    m_iPassCount = 0;
    return 0; // machine will be moved by caller
  }
  else
  {
    m_WorkingState = DL_MeasZByDefocus;
    m_bMeasureFocus = true;
    m_LeftFocusMeasRect = GetNormFocusRectangle(
      m_cLastLeftCavityCenter_pix , m_cLastLeftCavitySize_pix );
    m_RightFocusMeasRect = GetNormFocusRectangle(
      m_cLastRightCavityCenter_pix , m_cLastRightCavitySize_pix );
    SetCavityMode( false );
    switch (m_ZMethod)
    {
      case DLZ_Defocusing:
        {
          m_iPassCount = 0;
          GrabImage();
        }
        return 1;
      case DLZ_LongSweep:
        {
          m_ShiftsDL.m_z = -m_dDLZLongRange_um;
          m_dZShiftMin_um = m_ShiftsDL.m_z;
          m_iDLZNSweepSteps = ROUND( m_dDLZLongRange_um * 2. ) + 1;
          FXString Msg;
          Msg.Format( "ReadySweep;Step=%d;NSteps=%d;" ,
            m_dZStepForHighResolutionDefocusing_um , m_iDLZNSweepSteps );
          SendMessageToEngine( Msg , "LongSweepReady" );
          m_WorkingState = DL_LongSweep;
          //        SendDisplaceCommand( m_ShiftsDL ) ;
        }
        return 0;
      case DLZ_ShortSweep:
        {
          m_ShiftsDL.m_z = -( m_dForFastZSmallStep_um + m_dForFastZBigStep_um );
          m_dZShiftMin_um = m_ShiftsDL.m_z;
          m_iDLZNSweepSteps = 4;
          //         SendDisplaceCommand( m_ShiftsDL ) ;
          FXString Msg;
          Msg.Format( "ReadySweep;BigStep=%d;SmallStep=%d;NSteps=%d;" ,
            m_dForFastZBigStep_um , m_dForFastZSmallStep_um , m_iDLZNSweepSteps );
          SendMessageToEngine( Msg , "ShortSweepReady" );
          m_WorkingState = DL_ShortSweep;
        }
        return 0;
    }

  }
  return -1;
}

double MPPT::CalculateDeltaZByDefocus( CmplxVector& Data ,
  bool bSendMessage , FXString * pDiagnostics )
{
  if (!Data.size())
  {
    FXString Msg;
    Msg.Format( "No data for dZ calculation" );
    SEND_GADGET_ERR( ( LPCTSTR ) Msg );
    if (bSendMessage)
    {
      Msg.Insert( 0 , "Result=BadCavity; " );
      SendMessageToEngine( ( LPCTSTR ) Msg , "ZbyDefocus" );
    }
    m_WorkingState = State_Idle;
    return DBL_MAX;
  }

  double dMaxValue = -DBL_MAX;
  for (auto it = Data.begin(); it != Data.end(); it++)
  {
    if (it->real() > dMaxValue)
      dMaxValue = it->real();
  }
  double dThres = dMaxValue * m_dDefocusThreshold;
  if (pDiagnostics)
  {
    FXString Diagnostics , Add;
    auto it = Data.begin();
    do
    {
      Add.Format( "%s%.2f(%.1f)" , ( it == Data.begin() ) ? "" : "," , it->real() , it->imag() );
      Diagnostics += Add;
    } while (++it != Data.end());
    *pDiagnostics = Diagnostics;
  }

  double dZ = GetCenterForThresholdAndStep( Data , dThres , 1 );

  if (dZ > 1000.)
  {
    FXString Msg;
    Msg.Format( "Bad Z calculation" );
    SEND_GADGET_ERR( ( LPCTSTR ) Msg );
    if (bSendMessage)
    {
      Msg.Insert( 0 , "Result=BadCavity; " );
      SendMessageToEngine( ( LPCTSTR ) Msg , "ZbyDefocus" );
    }
    m_WorkingState = State_Idle;
    return DBL_MAX;
  }
  return dZ;
}

double MPPT::CalculateDeltaZByDefocus()
{
  FXString Diagnostics;

  double dZ = CalculateDeltaZByDefocus( m_IterationResults , false , &Diagnostics );
  if (dZ < 1000.)
  {
    SaveLogMsg( "Z defocus %d values : %s" ,
      m_IterationResults.size() , ( LPCTSTR ) Diagnostics , dZ );
    Diagnostics.Empty();
    double dZLeft = CalculateDeltaZByDefocus( m_IterResultLeft , false , &Diagnostics );
    SaveLogMsg( "Z Left defocus %d values : %s" ,
      m_IterResultLeft.size() , ( LPCTSTR ) Diagnostics , dZ );
    Diagnostics.Empty();
    double dZRight = CalculateDeltaZByDefocus( m_IterResultRight , false , &Diagnostics );
    SaveLogMsg( "Z Right defocus %d values : %s" ,
      m_IterResultRight.size() , ( LPCTSTR ) Diagnostics , dZ );
    switch (m_WhatSideToUse)
    {
      case DLWSZ_Left: dZ = dZLeft; break;
      case DLWSZ_Right: dZ = dZRight; break;
    }
    return dZ;
  }
  FXString Msg;
  Msg.Format( "Bad Z calculation" );
  SEND_GADGET_ERR( ( LPCTSTR ) Msg );
  Msg.Insert( 0 , "Result=BadCavity; " );
  SendMessageToEngine( ( LPCTSTR ) Msg , "ZbyDefocus" );
  m_WorkingState = State_Idle;
  return dZ;
}


double MPPT::GetCenterForThresholdAndStep( CmplxVector& Data , double dAbsThres , int iStep )
{
  if (!Data.size())
    return DBL_MAX;

  auto it = Data.begin();
  cmplx cPrev = *it;
  while (it->real() < dAbsThres)
  {
    cPrev = *it;
    if (++it == Data.end())
      return DBL_MAX;
  }
  double dZPart = ( dAbsThres - cPrev.real() ) / ( it->real() - cPrev.real() );

  double dZMinus = cPrev.imag() + dZPart * ( it->imag() - cPrev.imag() );
  while (it->real() > dAbsThres)
  {
    cPrev = *it;
    if (++it == Data.end())
      return DBL_MAX;
  }
  dZPart = ( cPrev.real() - dAbsThres ) / ( cPrev.real() - it->real() );

  double dZPlus = cPrev.imag() + dZPart * ( it->imag() - cPrev.imag() );

  double dZ = ( dZMinus + dZPlus ) / 2.;
  return dZ;
}


int MPPT::AddFocusResults( bool bToPlus )
{
  m_dLastFocusIndicator =
    m_dAverageFocusIndicator / ( m_iPassCount - m_iPassNumber );
  m_dLastLeftFocusIndicator =
    m_dLeftAverageFocusIndicator / ( m_iPassCount - m_iPassNumber );
  m_dLastRightFocusIndicator =
    m_dRightAverageFocusIndicator / ( m_iPassCount - m_iPassNumber );
  if (m_dMaxFocusValue < m_dLastFocusIndicator)
    m_dMaxFocusValue = m_dLastFocusIndicator;
  if (m_dMaxLeftFocusValue < m_dLastLeftFocusIndicator)
    m_dMaxLeftFocusValue = m_dLastLeftFocusIndicator;
  if (m_dMaxRightFocusValue < m_dLastRightFocusIndicator)
    m_dMaxRightFocusValue = m_dLastRightFocusIndicator;
  m_iPassCount = 0;
  m_dRightAverageFocusIndicator = m_dLeftAverageFocusIndicator =
    m_dAverageFocusIndicator = 0.;
  double dThresFull = m_dMaxFocusValue * m_dDefocusThreshold;
  double dThresLeft = m_dMaxLeftFocusValue * m_dDefocusThreshold;
  double dThresRight = m_dMaxRightFocusValue * m_dDefocusThreshold;

  bool bIsNotFinished = ( ( m_dLastFocusIndicator >= dThresFull )
    || ( m_dLastLeftFocusIndicator >= dThresLeft )
    || ( m_dLastRightFocusIndicator >= dThresRight ) );

  if (m_IterationResults.empty())
  {
    m_IterationResults.push_back( cmplx( m_dLastFocusIndicator , m_ShiftsDL.m_z ) );
    m_IterResultLeft.push_back( cmplx( m_dLastLeftFocusIndicator , m_ShiftsDL.m_z ) );
    m_IterResultRight.push_back( cmplx( m_dLastRightFocusIndicator , m_ShiftsDL.m_z ) );
  }
  else
  {
    for (size_t i = 0 ; i <= m_IterationResults.size() ; i++)
    {
      if (i == m_IterationResults.size())
      {
        m_IterationResults.push_back( cmplx( m_dLastFocusIndicator , m_ShiftsDL.m_z ) );
        m_IterResultLeft.push_back( cmplx( m_dLastLeftFocusIndicator , m_ShiftsDL.m_z ) );
        m_IterResultRight.push_back( cmplx( m_dLastRightFocusIndicator , m_ShiftsDL.m_z ) );
        break;
      }
      if (m_IterationResults.empty()
        || ( m_IterationResults[ i ].imag() > m_ShiftsDL.m_z ))
      {
        m_IterationResults.insert( m_IterationResults.begin() + i ,
          cmplx( m_dLastFocusIndicator , m_ShiftsDL.m_z ) );
        m_IterResultLeft.insert( m_IterResultLeft.begin() + i ,
          cmplx( m_dLastLeftFocusIndicator , m_ShiftsDL.m_z ) );
        m_IterResultRight.insert( m_IterResultRight.begin() + i ,
          cmplx( m_dLastRightFocusIndicator , m_ShiftsDL.m_z ) );
        break;
      }
    } ;
  }

  if (!bIsNotFinished)
  {
    if (bToPlus)
    {
      if (m_IterationResults.size() < 2)
        return -1; // necessary to do step back (to minus) and measure
    }
    else
    {
      //  Analysis for enough values to minus side
      if (( m_IterationResults.size() - m_iNZFullPlusValues ) == 1)
        return -1; // necessary to do step back (to plus) and measure
    }
    return 0; // scan finished
  }
  return 1;
}


bool MPPT::UpdateAverageFocuses()
{
  m_dAverageFocusIndicator += m_dLastFocusIndicator;
  m_dLeftAverageFocusIndicator += m_dLastLeftFocusIndicator;
  m_dRightAverageFocusIndicator += m_dLastRightFocusIndicator;
  return ( m_iPassCount < ( m_iAverager + m_iPassNumber ) );
}


int MPPT::SwitchOnConstantLight( bool bOnRing , bool bOnStraight )
{
  int iRes = 0;
  FXString CameraComm;
  if (bOnRing)
  {
    CameraComm.Format( "set properties(OutputSelector=0;OutputValue=true;"
      "Line2=(LineMode_=Output;LineSource_=UserOutput1;);)" );
  }
  else
  {
    CameraComm.Format( "set properties(OutputSelector=0;OutputValue=false;"
      "Line2=(LineMode_=Output;LineSource_=UserOutput1;);)" );
  }
  iRes = ProgramCamera( CameraComm );

  if (bOnStraight)
  {
    CameraComm.Format( "set properties(OutputSelector=1;OutputValue=true;"
      "Line3=(LineMode_=Output;LineSource_=UserOutput2;);)" );
  }
  else
  {
    CameraComm.Format( "set properties(OutputSelector=1;OutputValue=false;"
      "Line3=(LineMode_=Output;LineSource_=UserOutput2;);)" );
  }
  return iRes + ProgramCamera( CameraComm );
}
int MPPT::SwitchOffConstantLight( bool bOnRing , bool bOnStraight )
{
  if (m_bDisableLightSwitchOff)
    return 0 ;
  int iRes = 0;
  FXString CameraComm;
  if (bOnRing)
  {
    CameraComm.Format( "set properties("
      "Line2=(LineMode_=Output;LineSource_=UserOutput1;);"
      "OutputSelector=0;OutputValue=false;)" );
    iRes = ProgramCamera( CameraComm );
  }
  if (bOnStraight)
  {
    CameraComm.Format( "set properties("
      "Line3=(LineMode_=Output;LineSource_=UserOutput2;);"
      "OutputSelector=1;OutputValue=false;)" );
    iRes += ProgramCamera( CameraComm );
  }

  return iRes;
}


int MPPT::AdjustExposureForCavity( const CVideoFrame * pVF ,
  CContainerFrame * pOutFrame , double dTarget )
{
  CRect AreaLeft , AreaRight;
  GetFocusRectanglesForCavity( AreaLeft , AreaRight );

  FXIntArray HistoLeft , HistoRight;
  double dAverage = 0.;;
  int iNLevels = GetHistogram( pVF , HistoLeft , AreaLeft , &dAverage );
  GetHistogram( pVF , HistoRight , AreaRight , &dAverage );
  dAverage *= 0.5;
  int iArea = AreaLeft.Width() * AreaLeft.Height() * 2;
  int iNBlack = 0;
  int iNewExp = 0;
  CTextFrame * pText = NULL;
  int i = 0;
  int iHalfPosition = -1;
  for (; i < iNLevels; i++)
  {
    iNBlack += ( HistoLeft[ i ] += HistoRight[ i ] );
    if (iNBlack > iArea / 2)
    {
      iHalfPosition = i;
      break;
    }
  }
  double dNormDiff = ( dAverage / iNLevels ) - dTarget;
  if (abs( dNormDiff ) < 0.04)
  {

  }
  else if (abs( dNormDiff ) < 0.5)
  {
    iNewExp = ROUND( m_iNewCavityExposure * ( 1. - dNormDiff ) );
    pText = CreateTextFrame( cmplx( 100 , 800 ) , "0x00ffff" , 16 , "Description" ,
      pVF->GetId() , "Too Low Intensity\n Old Exp %d\n New Exp %d i=%d" ,
      m_iNewCavityExposure , iNewExp , i );
  }
  else if (iHalfPosition < iNLevels * 0.8)
  {
    iNewExp = ROUND( m_iNewCavityExposure * 1.15 /* ((iHalfPosition < iNLevels * 0.5) ? 1.5 : 1.1)*/ );
    pText = CreateTextFrame( cmplx( 100 , 800 ) , "0x00ffff" , 16 , "Description" ,
      pVF->GetId() , "Too Low Intensity\n Old Exp %d\n New Exp %d i=%d" ,
      m_iNewCavityExposure , iNewExp , i );
  }
  else if (iHalfPosition > iNLevels * 0.95)
  {
    iNewExp = ROUND( m_iNewCavityExposure * 0.8 );
    pText = CreateTextFrame( cmplx( 100 , 800 ) , "0x00ffff" , 16 , "Description" ,
      pVF->GetId() , "Too High Intensity\n Old Exp %d\n New Exp %d i=%d" ,
      m_iNewCavityExposure , iNewExp , i );
  }
  if (iNewExp)
  {
    m_iNewCavityExposure = iNewExp;
    pOutFrame->AddFrame( CreateRectFrame( AreaLeft , "0x0000ff" , "LeftArea" ) );
    pOutFrame->AddFrame( CreateRectFrame( AreaRight , "0x0000ff" , "LeftAreaRightArea" ) );
    pOutFrame->AddFrame( pText );
    return 1;
  }


  return 0;
}


int MPPT::ProcessNoContursNoPoints( const CDataFrame * pDataFrame , CContainerFrame * pOutFrame )
{
  switch (m_WorkingMode)
  {
    case MPPTWM_Down:
      {
        switch (m_WorkingState)
        {
          case DL_PinToCenterForZ:
          case DL_ScaleCalib:
            SendMessageToEngine( "Result=Error: Don't recognize target;" , "Scaling result" );
            break;
          case DL_PinNorthSide:
            SendMessageToEngine( "Result=Error: Don't see pin North edge;" , "Pin Measurement" );
            break;
          case DL_PinEastSide:
            SendMessageToEngine( "Result=Error: Don't see pin East edge;" , "Pin Measurement" );
            break;
          case DL_PinSouthSide:
            SendMessageToEngine( "Result=Error: Don't see pin South edge;" , "Pin Measurement" );
            break;
          case DL_PinWestSide:
            SendMessageToEngine( "Result=Error: Don't see pin West edge;" , "Pin Measurement" );
            break;
          case DL_MeasCavityXY:
            {
              if (m_iNAttempts++ < m_iNMaxAttempts)
              {
                const CVideoFrame * pVF = pDataFrame->GetVideoFrame();
                if (pVF)
                  AdjustExposureForCavity( pVF , pOutFrame , m_CurrentPart.m_Cavity.m_dNormBrightnessForCavity );
              }
              else
              {
                SendMessageToEngine( "Result=BadCavity;" , "Cavity result" );
                cmplx Pt = m_cLastMeasCenter - cmplx( 300. , 100. );
                pOutFrame->AddFrame( CreateTextFrame( Pt ,
                  "Cavity is not recognized" , "0x0000ff" , 20 ) );
                SaveCSVLogMsg( " %4d,     0,     0,    0,bad XY" , m_iNProcessedCavities );
                SaveLogMsg( "Cavity %d - XY Result=Error: Can't recognize cavity;" , m_iNProcessedCavities );
                FXString FileName;
                FileName.Format( "%s_%sBadCavXY_%d.bmp" , ( LPCTSTR ) GetTimeStamp() ,
                  GetShortWorkingModeName() , m_iNProcessedCavities );
                SaveImage( m_pCurrentImage , FileName );
                SaveCavityResultLogMsg( "%s" , _T( "Error, Can't recognize cavity" ) ) ;
              }
            }
            break;
          case DL_MeasCavityZ:
          case DL_MeasCavityZMultiple:
            {
              SendMessageToEngine( "Result=BadCavity;" , "Cavity result M" );
              pOutFrame->AddFrame( CreateTextFrame( m_cLastROICenter ,
                "Cavity depth is not measured" , "0x0000ff" , 20 ) );
              SaveCSVLogMsg( " %4d,     0,     0,    0,bad Z" , m_iNProcessedCavities );
              SaveLogMsg( "Cavity %d - Z Result=Error: Can't measure depth;" , m_iNProcessedCavities );
              FXString FileName;
              FileName.Format( "%s_%sBadCavZ_%d.bmp" , ( LPCTSTR ) GetTimeStamp() ,
                GetShortWorkingModeName() , m_iNProcessedCavities );
              SaveImage( m_pCurrentImage , FileName );
            }
            break;
          case DL_LiveVideoCavity:
            {
              if (m_bMeasureFocus)
                CalcAndAddFocusIndication( pOutFrame );

              if (m_pCurrentImage)
              {
                if (m_iNAttempts++ < m_iNMaxAttempts)
                {
                  if (AdjustExposureForCavity( m_pCurrentImage , pOutFrame , m_CurrentPart.m_Cavity.m_iCavityExp_us ))
                    //               ProgramExposureAndLightParameters( m_iNewCavityExposure , 10 , m_iNewCavityExposure );
                    ProgramExposure( m_iNewCavityExposure );
                  else
                    m_iNAttempts = m_iNMaxAttempts;
                }
              }
              //           {
              //             cmplx Pt = m_cLastMeasCenter - cmplx( 300. , 100. );
              //             pOutFrame->AddFrame( CreateTextFrame( Pt ,
              //               "Cavity is not recognized" , "0x0000ff" , 20 ) );
              //           }

              GrabImage();
            }
            break;
          case DL_LiveVideoCavityFocus:
            {
              CRect Left , Right;
              bool bPartKnown = GetFocusRectanglesForCavity( Left , Right );
              if (bPartKnown)
                CalcAndAddFocusIndication( pOutFrame , &Left , &Right );
              else
                CalcAndAddFocusIndication( pOutFrame );

              GrabImage();
              //           if (m_iFocusLogPeriod_samples && !m_FocusLogAccumulator.IsEmpty())
              //           {
              //             SaveFocusLog("%s", (LPCTSTR)m_FocusLogAccumulator);
              //             m_FocusLogAccumulator.Empty();
              //           }
            }
            break;
          case DL_LiveVideoPinFocusing:
            {
              if (m_iNAttempts++ < m_iNMaxAttempts)
              {
                if (AdjustExposureForFocus( m_pCurrentImage ,
                  pOutFrame , m_dTargetForFocusExpAdjust ))
                {
                  ProgramExposure( m_iNewFocusExposure );
                  GrabImage();
                  break;
                }
                m_iNAttempts = m_iNMaxAttempts;
                m_Exposures.clear();
                m_Averages.clear();
                m_Threshs.clear();
              }
              CalcAndAddFocusIndication( pOutFrame , ( m_iDistToFocusingArea < 20 ) ? &g_ROI : NULL );
              if (m_iFocusLogPeriod_samples && !m_FocusLogAccumulator.IsEmpty())
              {
                SaveFocusLog( "%s" , ( LPCTSTR ) m_FocusLogAccumulator );
                m_FocusLogAccumulator.Empty();
              }
            }
          case DL_LiveVideoPinMeasure:
          case DL_LiveVideoPinNorth:
          case DL_LiveVideoPinEast:
          case DL_LiveVideoPinSouth:
          case DL_LiveVideoPinWest:
            //           if ( bExtracted )
            //           {
            //             cmplx cViewPt = m_cLastROICenter + cmplx( -800. , -m_cLastROICenter.imag() * 0.9 ) ;
            //             pOutFrame->AddFrame( FormBigMsg( m_cLastExtractedResult_pix , cViewPt , 24 , "0x000080" ) ) ;
            //           }
          case DL_LiveVideoLaser:
            {
              GrabImage();
            }
            break;
          case DL_CaptureZWithCorrectHeigth:
            {
              SendMessageToEngine( "Result=BadCavity;" , "Cavity result" );
              pOutFrame->AddFrame( CreateTextFrame( m_cLastROICenter ,
                "Cavity depth is not measured" , "0x0000ff" , 20 ) );
              SaveCSVLogMsg( " %4d,     0,     0,    0,Error Cent Z" , m_iNProcessedCavities );
              SaveLogMsg( "Cavity %d - Final Z Result=Error: Can't measure depth;" , m_iNProcessedCavities );
            }
            break;
          case DL_LiveApexZView:
            {
              CalcAndAddFocusIndication( pOutFrame ,
                &m_LeftFocusMeasRect , NULL );
              GrabImage();
            }
            break;
          case DL_PinZMeasureByDefocus: // first image for defocusing measurement
            {
              if (m_iNAttempts++ < m_iNMaxAttempts)
              {
                if (AdjustExposureForFocus( m_pCurrentImage ,
                  pOutFrame , m_dTargetForFocusExpAdjust ))
                {
                  ProgramExposure( m_iNewFocusExposure );
                  GrabImage();
                  break;
                }
              }
              m_iNAttempts = m_iNMaxAttempts;
              m_Exposures.clear();
              m_Averages.clear();
              m_Threshs.clear();
              CalcAndAddFocusIndication( pOutFrame ,
                &m_LeftFocusMeasRect , &m_RightFocusMeasRect );
              if (m_iPassCount++ < m_iPassNumber)
                GrabImage();
              else
              {
                if (UpdateAverageFocuses())
                  GrabImage();
                else
                {
                  m_ShiftsDL.Reset();
                  ResetIterations();
                  AddFocusResults( true ) ;
                  m_WorkingState = DL_PinZDefocusToPlus;
                  SendDisplaceCommand( 0. , 0. , m_dZStepForHighResolutionDefocusing_um );
                  m_ShiftsDL.m_z = m_dZStepForHighResolutionDefocusing_um;
                }
              }
            }
            break;
          case DL_PinZDefocusToPlus:
            {
              CalcAndAddFocusIndication( pOutFrame ,
                &m_LeftFocusMeasRect , &m_RightFocusMeasRect );
              if (m_iPassCount++ < m_iPassNumber)
                GrabImage();
              else
              {
                if (UpdateAverageFocuses())
                  GrabImage();
                else
                {
                  if (AddFocusResults( true ))
                  {
                    SendDisplaceCommand( 0. , 0. , m_dZStepForHighResolutionDefocusing_um );
                    m_ShiftsDL.m_z += m_dZStepForHighResolutionDefocusing_um;
                  }
                  else  // Go to minus direction
                  {
                    m_dZShiftMax_um = m_ShiftsDL.m_z;
                    m_WorkingState = DL_PinZDefocusToMinus;
                    SendDisplaceCommand( 0. , 0. , -m_dZShiftMax_um - m_dZStepForHighResolutionDefocusing_um );
                    m_ShiftsDL.m_z = -m_dZStepForHighResolutionDefocusing_um;
                  }
                }
              }
            }
            break;
          case DL_PinZDefocusToMinus:
            {
              CalcAndAddFocusIndication( pOutFrame ,
                &m_LeftFocusMeasRect , &m_RightFocusMeasRect );
              if (m_iPassCount++ < m_iPassNumber)
                GrabImage();
              else
              {
                if (UpdateAverageFocuses())
                  GrabImage();
                else
                {
                  if (AddFocusResults( false ))
                  {
                    SendDisplaceCommand( 0. , 0. , -m_dZStepForHighResolutionDefocusing_um );
                    m_ShiftsDL.m_z -= m_dZStepForHighResolutionDefocusing_um;
                  }
                  else  // Measurement finished, do processing
                  {
                    m_dZShiftMin_um = m_ShiftsDL.m_z;
                    double dDeltaZ = CalculateDeltaZByDefocus();
                    if (dDeltaZ < 1000.)
                    {
                      m_dAverage_dZ = dDeltaZ;
                      cmplx cViewPt = m_cLastROICenter
                        + cmplx( 300. , -m_cLastROICenter.imag() * 0.5 );
                      pOutFrame->AddFrame( FormTextFrameForView( cViewPt , 16 ,
                        "0xffff00" , "dZ um = %.2f" , dDeltaZ ) );
                      if (m_bDoPinXYCalibration)
                      {
                        m_WorkingState = DL_ScaleCalib;
                        ResetIterations();
                        m_iCalibCntr = -1;
                        m_bPinOnlyCalib = false;
                        m_NewCalibData.clear();
                        m_NewCalibMatrixSize = m_CalibMatrixSize;
                        SetPinMode();
                        SendDisplaceCommand(
                          -m_ShiftsDL.m_x ,
                          -m_ShiftsDL.m_y ,
                          dDeltaZ - m_dZShiftMin_um );
                      }
                      else // No pin XY calibration, go to pin sides measurement
                      {
                        m_WorkingState = DL_PinNorthSide;
                        ResetIterations();
                        m_iCalibCntr = -1;
                        m_bPinOnlyCalib = true;
                        m_NewCalibData.clear();
                        SendDisplaceCommand( 0. , m_dPinDiam_um / 2. , 0. );
                        m_ShiftsDL.m_x = 0.;
                        m_ShiftsDL.m_y = GetBoundToTenth( m_dPinDiam_um / 2. ) ;

                      }
                    }
                    else
                      m_WorkingState = State_Idle;
                  }
                }
              }
            }
            break;
          case DL_MeasZByDefocus:  // it's OK, that no conturs, we don't need
          case DL_MeasZAfterExpAdjust:
          case DL_LongSweep:
          case DL_ShortSweep:
            {
              CalcAndAddFocusIndication( pOutFrame ,
                &m_LeftFocusMeasRect , &m_RightFocusMeasRect );
              if (( m_WorkingState == DL_MeasZByDefocus ) && ( m_ZMethod == DLZ_Defocusing ))
              {
                if (m_iNAttempts++ < m_iNMaxAttempts)
                {
                  if (AdjustExposureForFocus( m_pCurrentImage ,
                    pOutFrame , m_CurrentPart.m_Cavity.m_dTargetForFocusExpAdjust ))
                  {
                    ProgramExposure( m_iNewFocusExposure );
                    GrabImage();
                    break;
                  }
                }
                m_iNAttempts = m_iNMaxAttempts;
                m_Exposures.clear();
                m_Averages.clear();
                m_Threshs.clear();

                m_WorkingState = DL_MeasZAfterExpAdjust ;
              }

              if (m_iPassCount++ < m_iPassNumber)
                GrabImage();
              else
              {
                if (UpdateAverageFocuses())
                  GrabImage();
                else
                {
                  if (m_iDLZSweepCount++ == 0)
                  {
                    m_dMaxFocusValue = m_dMaxLeftFocusValue =
                      m_dMaxRightFocusValue = -DBL_MAX;
                    ResetIterations();
                  }
                  AddFocusResults( true );
                  switch (m_ZMethod)
                  {
                    case DLZ_Defocusing:
                      {
                        if (m_iNZMeasured < m_iNZShouldBeMeasuredWithHighResolution)
                          m_ShiftsDL.m_z = m_dZStepForHighResolutionDefocusing_um;
                        else  // fast Z resolution measurements
                          m_ShiftsDL.m_z = m_dForFastZBigStep_um;
                        m_WorkingState = DL_DefocusToPlus;
                        SendDisplaceCommand( 0. , 0. , m_ShiftsDL.m_z );
                      }
                      break;
                    case DLZ_LongSweep:
                      {
                        if (++m_iDLZSweepCount >= m_iDLZNSweepSteps)
                        {
                          if (CalcFinalZPosition( pOutFrame ))
                          {
                            m_WorkingState = DL_MoveCavityForFinalImage;
                            SetCavityMode();

                            SendDisplaceCommand(
                              -m_ShiftsDL.m_x + m_cLastCavityXYAveraged_um.real() ,
                              -m_ShiftsDL.m_y + m_cLastCavityXYAveraged_um.imag() ,
                              m_dZUsedAsResult - m_dZShiftMin_um );
                          }

                        }
                        else
                        {
                          m_ShiftsDL.m_z += m_dZStepForHighResolutionDefocusing_um;
                        }
                      }
                      break;
                    case DLZ_ShortSweep:
                      {
                        if (++m_iDLZSweepCount >= m_iDLZNSweepSteps)
                        {
                          if (CalcFinalZPosition( pOutFrame ))
                          {
                            m_WorkingState = DL_MoveCavityForFinalImage;
                            SetCavityMode();

                            SendDisplaceCommand(
                              -m_ShiftsDL.m_x + m_cLastCavityXYAveraged_um.real() ,
                              -m_ShiftsDL.m_y + m_cLastCavityXYAveraged_um.imag() ,
                              m_dZUsedAsResult - m_ShiftsDL.m_z );
                          }
                        }
                        else
                        {
                          switch (m_iDLZSweepCount)
                          {
                            case 1: m_ShiftsDL.m_z += m_dForFastZSmallStep_um; break;
                            case 2: m_ShiftsDL.m_z += m_dForFastZBigStep_um * 2.; break;
                            case 3: m_ShiftsDL.m_z += m_dForFastZSmallStep_um; break;
                            default:
                              SEND_GADGET_ERR( "Unnecessary image % d on sweep" , m_iDLZSweepCount ); break;
                          }
                        }
                      }
                      break;
                  }
                }
              }
            }
            break;
          case DL_DefocusToPlus:
            {
              CalcAndAddFocusIndication( pOutFrame ,
                &m_LeftFocusMeasRect , &m_RightFocusMeasRect );
              if (m_iPassCount++ < m_iPassNumber)
                GrabImage();
              else
              {
                if (UpdateAverageFocuses())
                  GrabImage();
                else
                {
                  if (m_iNZMeasured < m_iNZShouldBeMeasuredWithHighResolution)
                  {
                    if (AddFocusResults( true ))
                    {
                      if (fabs( m_ShiftsDL.m_z ) > m_dDLZLongRange_um)
                      {
                        SendDisplaceCommand( 0. , 0. , -m_ShiftsDL.m_z );
                        m_WorkingState = DL_BadCavityOnDefocusing;
                      }
                      else
                      {
                        SendDisplaceCommand( 0. , 0. , m_dZStepForHighResolutionDefocusing_um );
                        m_ShiftsDL.m_z += m_dZStepForHighResolutionDefocusing_um;
                      }
                    }
                    else  // Go to minus direction
                    {
                      m_dZShiftMax_um = m_ShiftsDL.m_z;
                      m_WorkingState = DL_DefocusToMinus;
                      SendDisplaceCommand( 0. , 0. , -m_dZShiftMax_um - m_dZStepForHighResolutionDefocusing_um );
                      m_ShiftsDL.m_z = -m_dZStepForHighResolutionDefocusing_um;
                      m_iNZFullPlusValues = ( int ) m_IterationResults.size();
                      m_iNZLeftPlusValues = ( int ) m_IterResultLeft.size();
                      m_iNZRightPlusValues = ( int ) m_IterResultRight.size();
                    }
                  }
                  else  // low Z resolution measurements
                  {
                    int iAddResult = AddFocusResults( true );
                    if (fabs( m_ShiftsDL.m_z ) > m_dDLZLongRange_um)
                    {
                      SendDisplaceCommand( 0. , 0. , -m_ShiftsDL.m_z );
                      m_WorkingState = DL_BadCavityOnDefocusing;
                    }
                    else if (iAddResult) // 1 - go forward, -1 - do one step back
                    {
                      double dStep = m_dForFastZSmallStep_um * iAddResult;
                      SendDisplaceCommand( 0. , 0. , dStep );
                      m_ShiftsDL.m_z += dStep ;
                    }
                    else  // Go to minus direction
                    {
                      m_dZShiftMax_um = m_ShiftsDL.m_z;
                      m_WorkingState = DL_DefocusToMinus;
                      SendDisplaceCommand( 0. , 0. , -m_dZShiftMax_um - m_dForFastZBigStep_um );
                      m_ShiftsDL.m_z = -m_dForFastZBigStep_um;
                      m_iNZFullPlusValues = ( int ) m_IterationResults.size();
                      m_iNZLeftPlusValues = ( int ) m_IterResultLeft.size();
                      m_iNZRightPlusValues = ( int ) m_IterResultRight.size();
                    }
                  }
                }
              }
            }
            break;
          case DL_DefocusToMinus:
            {
              CalcAndAddFocusIndication( pOutFrame ,
                &m_LeftFocusMeasRect , &m_RightFocusMeasRect );
              if (m_iPassCount++ < m_iPassNumber)
                GrabImage();
              else
              {
                if (UpdateAverageFocuses())
                  GrabImage();
                else
                {
                  if (m_iNZMeasured < m_iNZShouldBeMeasuredWithHighResolution)
                  {
                    int iAddResult = AddFocusResults( false );
                    if (fabs( m_ShiftsDL.m_z ) > m_dDLZLongRange_um)
                    {
                      SendDisplaceCommand( 0. , 0. , -m_ShiftsDL.m_z );
                      m_WorkingState = DL_BadCavityOnDefocusing;
                    }
                    else if (iAddResult)
                    {
                      SendDisplaceCommand( 0. , 0. , -m_dZStepForHighResolutionDefocusing_um );
                      m_ShiftsDL.m_z -= m_dZStepForHighResolutionDefocusing_um;
                      break; // from DL_DefocusToMinus branch
                    }
                  }
                  else  // low Z resolution measurements
                  {
                    int iAddResult = AddFocusResults( false );
                    if (fabs( m_ShiftsDL.m_z ) > m_dDLZLongRange_um)
                    {
                      SendDisplaceCommand( 0. , 0. , -m_ShiftsDL.m_z );
                      m_WorkingState = DL_BadCavityOnDefocusing;
                    }
                    else if (iAddResult) // 1 - continue to go back, -1 - do one step forward
                    {
                      double dStep = -m_dForFastZSmallStep_um * iAddResult;
                      SendDisplaceCommand( 0. , 0. , dStep );
                      m_ShiftsDL.m_z += dStep;
                      break;// from DL_DefocusToMinus branch
                    }
                  }
                  // Measurement finished, do processing
                  m_dZShiftMin_um = m_ShiftsDL.m_z;
                  if (CalcFinalZPosition( pOutFrame ))
                  {
                    if (m_SaveMode == Save_Final)
                    {
                      FXString FileName ;
                      FileName.Format( "%s_%sCavZ_%d.bmp" , ( LPCTSTR ) GetTimeStamp() ,
                        GetShortWorkingModeName() , m_iNProcessedCavities );
                      SaveImage( m_pCurrentImage , FileName );
                    }
                    m_WorkingState = DL_MoveCavityForFinalImage;
                    SetCavityMode();
                    SendDisplaceCommand(
                      -m_ShiftsDL.m_x + m_cLastCavityXYAveraged_um.real() ,
                      -m_ShiftsDL.m_y + m_cLastCavityXYAveraged_um.imag() ,
                      m_dZUsedAsResult - m_dZShiftMin_um );
                    m_iNZMeasured++;
                  }
                  else
                    m_WorkingState = State_Idle;
                }
              }
            }
            break;
        }
      }
      break;
    case MPPTWM_UpSide:
      {
        switch (m_WorkingState)
        {
          case ULS_FirstZForBlank: // Initial Z measurement
          case ULS_ZCorrection:
            {
              CFigureFrame * pBlankBaseCenter =
                ( CFigureFrame* ) pDataFrame->GetFigureFrame( "blank_side_pos" );
              if (pBlankBaseCenter)
              {
                SendDisplaceCommand( 0. , 0. , 500. );
                pOutFrame->AddFrame( CreateTextFrame( cmplx( 400. , 400. ) ,
                  "Blank is inserted too deep\nI do move out for 500 microns" , "0x0000ff" , 36 ) );
                break;
              }
              SendMessageToEngine( "Result=BadBlank; Don't see blank from side;" , "InitialZ" );
              pOutFrame->AddFrame( CreateTextFrame( m_cLastROICenter ,
                "Blank is not measured from side" , "0x0000ff" , 20 ) );
              CheckAndSaveFinalImages( pOutFrame , true , "NotSeen" )  ;
              m_WorkingState = State_Idle;
            }
            break;
          case ULS_LiveVideo:
            GrabImage();
            break;
        }
      }
      break;
    case MPPTWM_UpFront:
      {
        switch (m_WorkingState)
        {
          case ULF_MoveAfterMeasurement: // final measurement after pos correction
          case ULF_MeasureAndCorrect: // Measurement before correction
          case ULF_RightSideCorrection: // Initial Z measurement
            {
              SendMessageToEngine( "Result=BadBlank; Don't see blank bottom;" , GetWorkingStateName() );
              pOutFrame->AddFrame( CreateTextFrame( m_cLastROICenter ,
                "Blank is not measured from down" , "0x0000ff" , 20 ) );
              CheckAndSaveFinalImages( pOutFrame , true , "NotMeas" )  ;
              SaveCSVLogMsg( " %4d,     0,     0,    0,bad XY" , m_iNProcessedBlanks );
              SaveLogMsg( "Blank %d - XY Result=Error: Bad Blank;" , m_iNProcessedBlanks-- );
              m_WorkingState = State_Idle;
            }
            break;
          case ULF_LiveVideo:
            {
              GrabImage();
            }
            break;
        }
      }
      break;
  }
  return 0;
}

cmplx MPPT::CalcShiftForFinalPinPosition()
{
  m_ZMethod = AnalyzeAndGetDLMeasurementZMethod();
  cmplx cDiff( DBL_MAX , DBL_MAX );
  cmplx cCurrentPt( m_ShiftsDL.m_x , m_ShiftsDL.m_y );
  if (( m_ZMethod == DLZ_Defocusing )
    || ( m_ZMethod == DLZ_LongSweep )
    || ( m_ZMethod == DLZ_ShortSweep ) || m_bPinOnlyCalib)
  {
    bool bUpper = ( m_CurrentPart.m_Cavity.m_CavityEdge == CavEdge_Upper_Xc )
      || ( m_CurrentPart.m_Cavity.m_CavityEdge == CavEdge_Upper ) ;
    CoordsCorresp FinalTarget = bUpper ? m_SouthPinSide : m_NorthPinSide ;
    m_cFinalTargetOverPin = FinalTarget.World ;
    cmplx cCorrection = ConvertCoordsRelativeToCenter(
      FinalTarget.FOV );
    m_cFinalTargetOverPin += cCorrection; // coordinate for edge in center viewing

    double dVertCorrectionInFOV_pix = m_CurrentPart.m_Cavity.m_dPlaneHeight_um * 0.5 / m_dScale_um_per_pix ;
    if (!bUpper)
      dVertCorrectionInFOV_pix = -dVertCorrectionInFOV_pix ;
    cmplx cVertCorr = cmplx( 0. , dVertCorrectionInFOV_pix ) ;
    cmplx cCenterCorrection = ConvertCoordsRelativeToCenter( cVertCorr );

    m_cFinalTargetOverPin += cCenterCorrection;
    m_cFinalTargetOverPin._Val[ _RE ] = m_CalculatedPinPosition.real();
    cDiff = m_cFinalTargetOverPin - cCurrentPt;
  }
  else if (m_ZMethod == DLZ_Laser)
  {
    // following is calculations for standard calibration (XY,Pin,Z)
    // Target pt is in the middle between target center and East edge
    cmplx cTargetPt = cmplx( m_dPinDiam_um / 4. , 0. );
    cTargetPt += m_CalculatedPinPosition;
    cDiff = cTargetPt - cCurrentPt;
  }
  else
  {
    SendMessageToEngine( "Result=Error; "
      "Unknown Z measurement method" , "EndOfCalibration" ) ;
  }
  return cDiff;
}


bool MPPT::IsShiftedCenter()
{
  return ( ( m_WorkingState == DL_MeasCavityXY )
    || ( m_WorkingState == DL_FinalImageOverPin )
    || ( m_WorkingState == DL_CorrectAfterFinalVision )
    || ( m_WorkingState == DL_CaptureZWithCorrectHeigth )
    || ( m_WorkingState == DL_MoveCavityForFinalImage )
    || ( m_WorkingState == DL_SendCavityResult )
    || ( m_WorkingState == DL_CaptureCavityFinalImage ) );
}


int MPPT::CalcFinalZPosition( CContainerFrame * pOutFrame )
{
  FXString Diagnostics , LDiagnostics , RDiagnostics;
  m_dAverage_dZ = CalculateDeltaZByDefocus(
    m_IterationResults , true , &Diagnostics );
  m_dAverageLeft_dZ = CalculateDeltaZByDefocus(
    m_IterResultLeft , false , &LDiagnostics );
  m_dAverageRight_dZ = CalculateDeltaZByDefocus(
    m_IterResultRight , false , &RDiagnostics );
  if (m_dAverage_dZ < 1000.)
  {
    m_dZUsedAsResult = m_dAverage_dZ;
    switch (m_WhatSideToUse)
    {
      case DLWSZ_Left: m_dZUsedAsResult = m_dAverageLeft_dZ; break;
      case DLWSZ_Right: m_dZUsedAsResult = m_dAverageRight_dZ; break;
    }

    cmplx cViewPt = m_cLastROICenter
      + cmplx( 300. , -m_cLastROICenter.imag() * 0.5 );
    pOutFrame->AddFrame( FormTextFrameForView( cViewPt , 16 ,
      "0xffff00" , "dZ um = %.2f(%.2f,%.2f) dZused=%.2f" ,
      m_dAverage_dZ , m_dAverageLeft_dZ , m_dAverageRight_dZ , m_dZUsedAsResult ) );
    SaveLogMsg( "Z FULL defocus %d values (cent %d): %s" ,
      m_IterationResults.size() , m_IterationResults.size() - m_iNZFullPlusValues ,
      ( LPCTSTR ) Diagnostics );
    SaveLogMsg( "Z LEFT defocus %d values (cent %d): %s" ,
      m_IterResultLeft.size() , m_IterResultLeft.size() - m_iNZLeftPlusValues ,
      ( LPCTSTR ) LDiagnostics );
    SaveLogMsg( "Z RIGHT defocus %d values (cent %d): %s" ,
      m_IterResultRight.size() , m_IterResultRight.size() - m_iNZRightPlusValues ,
      ( LPCTSTR ) RDiagnostics );
    return 1;
  }
  else
  {
    SEND_GADGET_ERR( "Can't calculate Z position" );
    m_WorkingState = State_Idle;
  }
  return 0;
}


int MPPT::AdjustExposureForFocus(
  const CVideoFrame * pVF , CContainerFrame * pOutFrame , double dTarget )
{
  CRect AreaLeft , AreaRight ;
  if (m_WorkingState == DL_LiveVideoPinFocusing
    || m_WorkingState == DL_PinZMeasureByDefocus)
  {
    AreaLeft = m_LeftFocusMeasRect;
    AreaRight = m_RightFocusMeasRect;
  }
  else
    GetFocusRectanglesForCavity( AreaLeft , AreaRight );

  FXIntArray HistoLeft , HistoRight;
  double dAverage = 0.;
  int iNLevels = GetHistogram( pVF , HistoLeft , AreaLeft , &dAverage );
  GetHistogram( pVF , HistoRight , AreaRight , &dAverage );
  int iMin = INT_MAX , iMax = INT_MIN ;
  _calc_average( pVF , AreaLeft , iMin , iMax );
  _calc_average( pVF , AreaRight , iMin , iMax );

  dAverage *= 0.5;
  int iArea = AreaLeft.Width() * AreaLeft.Height() * 2;
  int iNWhite = 0;
  int iNewExp = 0;
  CTextFrame * pText = NULL;
  int i = ( int ) HistoLeft.GetUpperBound();
  for (; 0 <= i ; i--)
  {
    iNWhite += ( HistoLeft[ i ] += HistoRight[ i ] );
    if (iNWhite > iArea * dTarget)
      break ;
  }
  int iThresPos = i ;
  m_Exposures.push_back( m_iNewFocusExposure );
  m_Averages.push_back( dAverage );
  m_Threshs.push_back( iMax );
  if (iMax > 253)
  {
    iNewExp = ( m_iNewFocusExposure * 90 ) / 100 ;
    pText = CreateTextFrame( cmplx( 100 , 800 ) , "0x00ffff" , 16 , "Description" ,
      pVF->GetId() , "Too High Intensity\n Old Exp %d\n New Exp %d iMax=%d" ,
      m_iNewFocusExposure , iNewExp , iMax );
  }
  //   else if ( iThresPos < HistoLeft.Count() / 2 )
  //   {
  //     iNewExp = ROUND(m_iNewFocusExposure * 1.05) ;
  // //     iNewExp = (m_iNewFocusExposure * 7) / 10 ;
  //     pText = CreateTextFrame( cmplx( 100 , 800 ) , "0x00ffff" , 16 , "Description" ,
  //       pVF->GetId() , "Too Low Intensity\n Old Exp %d\n New Exp %d Thres=%d" ,
  //       m_iNewFocusExposure , iNewExp , iThresPos );
  //   }
  //   else if ( iThresPos < 240 )
  else if (iMax < 240)
  {
    double dIntensRatio = 247. / iMax;
    if (dIntensRatio > 1.07)
      dIntensRatio = 1.07;
    iNewExp = ROUND( m_iNewFocusExposure * dIntensRatio ) ;
    pText = CreateTextFrame( cmplx( 100 , 800 ) , "0x00ffff" , 16 , "Description" ,
      pVF->GetId() , "Adjust Intensity\n Old Exp %d\n New Exp %d iMax=%d" ,
      m_iNewFocusExposure , iNewExp , iMax );
  }
  //   double dRatio = dAverage / iNLevels ;
  //   double dNormDiff = dRatio / dTarget;
  // 
  //   if ( abs( dNormDiff - 1. ) < 0.03 )
  //   {
  // 
  //   }
  //   else if ( (m_iNAttempts++ >= m_iNMaxAttempts) 
  //     && (dNormDiff > 1.) )
  //   {
  // 
  //   }
  //   else 
  //   {
  //     if (dRatio > 0.98)
  //       iNewExp = ROUND(m_iNewFocusExposure * 0.9);
  //     else
  //     {
  //       if ( dNormDiff > 1.2 )
  //         dNormDiff = 1.2 ;
  //       else if ( dNormDiff < 0.8 )
  //         dNormDiff = 0.8 ;
  //       iNewExp = ROUND( m_iNewFocusExposure * 1. / dNormDiff );
  //     }
  //     pText = CreateTextFrame( cmplx( 100 , 800 ) , "0x00ffff" , 16 , "Description" ,
  //       pVF->GetId() , "Too Low Intensity\n Old Exp %d\n New Exp %d i=%d" ,
  //       m_iNewFocusExposure , iNewExp , i );
  //   }
  //   else if (iHalfPosition < iNLevels / 3)
  //   {
  //     iNewExp = ROUND( m_iNewFocusExposure * 1.2 );
  //     pText = CreateTextFrame( cmplx( 100 , 800 ) , "0x00ffff" , 16 , "Description" ,
  //       pVF->GetId() , "Too Low Intensity\n Old Exp %d\n New Exp %d i=%d" ,
  //       m_iNewFocusExposure, iNewExp , i );
  //   }
  //   else if ( iHalfPosition > 2 * iNLevels / 3 )
  //   {
  //     iNewExp = ROUND(m_iNewFocusExposure * 0.8 );
  //     pText = CreateTextFrame( cmplx( 100 , 800 ) , "0x00ffff" , 16 , "Description" ,
  //       pVF->GetId() , "Too High Intensity\n Old Exp %d\n New Exp %d i=%d" ,
  //       m_iNewFocusExposure, iNewExp , i );
  //   }
  if (iNewExp)
  {
    m_iNewFocusExposure = iNewExp;
    pOutFrame->AddFrame( CreateRectFrame( AreaLeft , "0xff00ff" , "LeftArea" ) );
    pOutFrame->AddFrame( CreateRectFrame( AreaRight , "0xff00ff" , "RightArea" ) );
    pOutFrame->AddFrame( pText );
    return 1;
  }

  return 0;
}

int MPPT::ProgramCamera( LPCTSTR pCameraParams , LPCTSTR pLabel )
{
  CTextFrame * pCamCom = CreateTextFrame( pCameraParams ,
    pLabel ? pLabel : "CameraParamsSet" );

  return PutFrame( GetOutputConnector( 2 ) , pCamCom );
}


CDataFrame * MPPT::CavityYCorrection( const CFigureFrame * pContur ,
  FXIntArray& ExtremeIndexes ,
  double dWidthThres , double& dUpCorrection_pix , double& dDownCorrection_pix )
{
  CContainerFrame * pViewData = NULL ;
  cmplx * pPts = ( cmplx* ) pContur->GetData() ;
  int iFigLen = ( int ) pContur->Count() ;
  int iTopIndex = ExtremeIndexes[ EXTREME_INDEX_TOP ] ;
  int iBottomIndex = ExtremeIndexes[ EXTREME_INDEX_BOTTOM ] ;
  cmplx cUpper = pPts[ iTopIndex ] ;
  cmplx cLower = pPts[ iBottomIndex ] ;
  int iTopLeft = ( int ) GetIndexInRing( iTopIndex - 1 , iFigLen ) ;
  int iTopRight = ( int ) GetIndexInRing( iTopIndex + 1 , iFigLen ) ;
  do
  {
    while (pPts[ iTopRight ].imag() >= pPts[ iTopLeft ].imag())
      iTopLeft = ( int ) GetIndexInRing( --iTopLeft , iFigLen ) ;
    while (pPts[ iTopRight ].imag() <= pPts[ iTopLeft ].imag())
      iTopRight = ( int ) GetIndexInRing( ++iTopRight , iFigLen ) ;
  } while (abs( pPts[ iTopRight ] - pPts[ iTopLeft ] ) < dWidthThres);
  dUpCorrection_pix =
    ( pPts[ iTopRight ].imag() + pPts[ iTopLeft ].imag() ) * 0.5 - cUpper.imag() ;
  if (m_ViewDetails >= 2)
  {
    pViewData = CContainerFrame::Create() ;
    CFigureFrame * pUpperLine = CreateLineFrame( pPts[ iTopRight ] , pPts[ iTopLeft ] , 0x00ff00 );
    pViewData->AddFrame( pUpperLine );
  }
  int iBottomLeft = ( int ) GetIndexInRing( iBottomIndex + 1 , iFigLen ) ;
  int iBottomRight = ( int ) GetIndexInRing( iBottomIndex - 1 , iFigLen ) ;
  do
  {
    while (pPts[ iBottomRight ].imag() >= pPts[ iBottomLeft ].imag())
      iBottomRight = ( int ) GetIndexInRing( --iBottomRight , iFigLen ) ;
    while (pPts[ iBottomRight ].imag() <= pPts[ iBottomLeft ].imag())
      iBottomLeft = ( int ) GetIndexInRing( ++iBottomLeft , iFigLen ) ;
  } while (abs( pPts[ iBottomRight ] - pPts[ iBottomLeft ] ) < dWidthThres);
  dDownCorrection_pix = ( pPts[ iBottomRight ].imag() + pPts[ iBottomLeft ].imag() ) * 0.5
    - cLower.imag() ;
  if (m_ViewDetails >= 2)
  {
    if (!pViewData)
      pViewData = CContainerFrame::Create() ;
    CFigureFrame * pLowerLine = CreateLineFrame( pPts[ iBottomRight ] , pPts[ iBottomLeft ] , 0x00ff00 );
    pViewData->AddFrame( pLowerLine );
  }
  return pViewData ;
}

bool CheckErr( double dVal , double dPattern , double dTol_perc , LPCTSTR pName , FXString& ErrMsg )
{
  double dErr = ( dVal - dPattern ) * 100. / dPattern ;
  if (fabs( dErr ) > dTol_perc)
  {
    ErrMsg.Format( "%s(%.2f[%.2f,%.2f]um) " , pName , dVal ,
      dPattern * ( 1. - dTol_perc * 0.01 ) , dPattern * ( 1. + dTol_perc * 0.01 ) ) ;
    return false ;
  }
  return true ;
}

bool CheckAngErr( double dVal , double dPattern , double dTol_deg , LPCTSTR pName , FXString& ErrMsg )
{
  double dErr = ( dVal - dPattern ) ;
  if (fabs( dErr ) > dTol_deg)
  {
    ErrMsg.Format( "%s(%.2f[%.2f,%.2f]deg) " , pName , dVal ,
      dPattern - dTol_deg , dPattern + dTol_deg ) ;
    return false ;
  }
  return true ;
}

int MPPT::CheckBlankSize( CmplxVector& CentersAndCorners ,
  CContainerFrame * pOutInfo , FXString& ErrorMsg )
{
  if (CentersAndCorners.size() < 9)
    return 0 ;

  double dWidth = ( m_iNProcessedBlanks <= 0 ) ?
    m_CurrentPart.m_Gauge.m_dBlankWidth_um : m_CurrentPart.m_Blank.m_dBlankWidth_um ;
  double dHeight = ( m_iNProcessedBlanks <= 0 ) ?
    m_CurrentPart.m_Gauge.m_dBlankHeight_um : m_CurrentPart.m_Blank.m_dBlankHeight_um ;

  double dTopWidth = CentersAndCorners[ SQRCorner_TR ].real() - CentersAndCorners[ SQRCorner_LT ].real() ;
  dTopWidth = fabs( dTopWidth ) * m_dScale_um_per_pix ;
  double dBottomWidth = CentersAndCorners[ SQRCorner_RB ].real() - CentersAndCorners[ SQRCorner_BL ].real() ;
  dBottomWidth = fabs( dBottomWidth ) * m_dScale_um_per_pix ;
  double dLeftHeight = CentersAndCorners[ SQRCorner_LT ].imag() - CentersAndCorners[ SQRCorner_BL ].imag() ;
  dLeftHeight = fabs( dLeftHeight ) * m_dScale_um_per_pix ;
  double dRightHeight = CentersAndCorners[ SQRCorner_TR ].imag() - CentersAndCorners[ SQRCorner_RB ].imag() ;
  dRightHeight = fabs( dRightHeight ) * m_dScale_um_per_pix ;

  m_LastBlankSize._Val[ _RE ] = 0.5 * ( dTopWidth + dBottomWidth ) ;
  m_LastBlankSize._Val[ _IM ] = 0.5 * ( dLeftHeight + dRightHeight ) ;

  // Next vector is from up to down
  cmplx LeftSideVect = CentersAndCorners[ SQRCorner_BL ] - CentersAndCorners[ SQRCorner_LT ] ;
  // Next vector is from left to right
  cmplx TopSideVect = CentersAndCorners[ SQRCorner_TR ] - CentersAndCorners[ SQRCorner_LT ] ;
  // Next vector is from up to down
  cmplx RightSideVect = CentersAndCorners[ SQRCorner_RB ] - CentersAndCorners[ SQRCorner_TR ] ;
  // Next vector is from left to right
  cmplx BottomSideVect = CentersAndCorners[ SQRCorner_RB ] - CentersAndCorners[ SQRCorner_BL ] ;

  double dTopLeftAng = RadToDeg( arg( LeftSideVect / TopSideVect ) ) ;
  double dTopRightAng = 180. - RadToDeg( arg( RightSideVect / TopSideVect ) ) ;
  double dBottomLeftAng = 180. - RadToDeg( arg( LeftSideVect / BottomSideVect ) ) ;
  double dBottomRightAng = RadToDeg( arg( RightSideVect / BottomSideVect ) ) ;

  pOutInfo->AddFrame( CreateTextFrame( CentersAndCorners[ SQRCorner_LT ] + cmplx( -100. , -50. ) ,
    "0x0000ff" , 12 , NULL , 0 , "%.2f" , dTopLeftAng ) ) ;
  pOutInfo->AddFrame( CreateTextFrame( CentersAndCorners[ SQRCorner_TR ] + cmplx( 100. , -50. ) ,
    "0x0000ff" , 12 , NULL , 0 , "%.2f" , dTopRightAng ) ) ;
  pOutInfo->AddFrame( CreateTextFrame( CentersAndCorners[ SQRCorner_RB ] + cmplx( 100. , 50. ) ,
    "0x0000ff" , 12 , NULL , 0 , "%.2f" , dBottomRightAng ) ) ;
  pOutInfo->AddFrame( CreateTextFrame( CentersAndCorners[ SQRCorner_BL ] + cmplx( -100. , 50. ) ,
    "0x0000ff" , 12 , NULL , 0 , "%.2f" , dBottomLeftAng ) ) ;

  FXString ProblemMsg( "Size Error: " ) , ErrMsg , AngleErrMsg , ProblemAngle( "Angle Error:" ) ;

  if (dWidth == 0. || dHeight == 0.)
  {
    ProblemMsg += "No data about size" ;
  }
  else
  {
    if (!CheckErr( dTopWidth , dWidth , m_CurrentPart.m_Blank.m_dSizeTolerance_perc , "Top" , ErrMsg ))
      ProblemMsg += ErrMsg ;
    if (!CheckErr( dRightHeight , dHeight , m_CurrentPart.m_Blank.m_dSizeTolerance_perc , "Right" , ErrMsg ))
      ProblemMsg += ErrMsg ;
    if (!CheckErr( dBottomWidth , dWidth , m_CurrentPart.m_Blank.m_dSizeTolerance_perc , "Bottom" , ErrMsg ))
      ProblemMsg += ErrMsg ;
    if (!CheckErr( dLeftHeight , dHeight , m_CurrentPart.m_Blank.m_dSizeTolerance_perc , "Left" , ErrMsg ))
      ProblemMsg += ErrMsg ;
    if (m_CurrentPart.m_Blank.m_UsedBlankEdge == SQREdge_Upper)
    {
      if (!CheckAngErr( dTopLeftAng , 90. , m_CurrentPart.m_Blank.m_dParallelismTol_deg , "TL_Corner" , AngleErrMsg ))
        ProblemAngle += AngleErrMsg ;
      if (!CheckAngErr( dTopRightAng , 90. , m_CurrentPart.m_Blank.m_dParallelismTol_deg , "TR_Corner" , AngleErrMsg ))
        ProblemAngle += AngleErrMsg ;
    }
    else if (m_CurrentPart.m_Blank.m_UsedBlankEdge == SQREdge_Lower)
    {
      if (!CheckAngErr( dBottomLeftAng , 90. , m_CurrentPart.m_Blank.m_dParallelismTol_deg , "BL_Corner" , AngleErrMsg ))
        ProblemAngle += AngleErrMsg ;
      if (!CheckAngErr( dBottomRightAng , 90. , m_CurrentPart.m_Blank.m_dParallelismTol_deg , "BR_Corner" , AngleErrMsg ))
        ProblemAngle += AngleErrMsg ;
    }
  }
  bool bBadSize = ProblemMsg.GetLength() > 15 ;
  bool bBadAngle = AngleErrMsg.GetLength() > 15 ;
  cmplx cTextPt = 0.5 * ( CentersAndCorners[ SQRCorner_BL ] + CentersAndCorners[ SQRCorner_RB ] ) + cmplx( -200. , 70. );
  FXString BlankSizeAsText ;
  BlankSizeAsText.Format( "Size(%.1f,%.1f)um\n" , 0.5 * ( dTopWidth + dBottomWidth ) , 0.5 * ( dLeftHeight + dRightHeight ) ) ;
  if (bBadSize || bBadAngle)
  {
    if (bBadSize)
      ErrMsg = BlankSizeAsText + ProblemMsg ;
    if (bBadAngle)
      ErrMsg = ( bBadSize ? "\n" : BlankSizeAsText ) + ProblemAngle ;


    CTextFrame * pErrFrame = CreateTextFrame( cTextPt , ( LPCTSTR ) ErrMsg ,
      "0x0000ff" , 12 , "BlankSizeError" ) ;
    pOutInfo->AddFrame( pErrFrame ) ;
    return 0 ;
  }
  CTextFrame * pSizeFrame = CreateTextFrame( cTextPt , ( LPCTSTR ) BlankSizeAsText ,
    "0xff00ff" , 12 , "BlankOK" ) ;
  pOutInfo->AddFrame( pSizeFrame ) ;

  cmplx SelectedPt = m_BlankCentersAndCorners[ ( int ) m_UsedBlankEdge ]
    - m_cLastROICenter;
  //   if (m_cPartPatternPt.real() == DBL_MAX || m_cPartPatternPt.real() == 0.)
  //     m_cPartPatternPt = SelectedPt ;
  //   else
  {
    cmplx cOnImage = m_cPartPatternPt + m_cLastROICenter ;
    pOutInfo->AddFrame( CreatePtFrame( cOnImage , "Sz=10;color=0xffff00;" ) ) ;
    cmplx cTextViewPt = cOnImage + cmplx( 10. , -80 );
    pOutInfo->AddFrame( CreateTextFrame( cTextViewPt , "0xff4000" , 10 , "PattPt" , 0 ,
      "Patt(%.2f,%.2f)pix\nRes(%.2f,%.2f)pix" ,
      m_cPartPatternPt.real() , m_cPartPatternPt.imag() ,
      SelectedPt.real() , SelectedPt.imag() ) );
    cmplx cDistFOV = SelectedPt - m_cPartPatternPt;
    cmplx cMoveDist = ConvertCoordsRelativeToCenter( cDistFOV );
    cmplx TextPt = cOnImage + cmplx( -100 , 20 );
    pOutInfo->AddFrame( CreateTextFrame( TextPt , "0xff00ff" , 12 , "RefPt" , 0 , "(%.2f,%.2f)um" ,
      cMoveDist.real() , cMoveDist.imag() ) );
  }

  return 1;
}


int MPPT::CheckBlankSizesAndReport( CContainerFrame * pOutInfo )
{
  cmplx cDistFOV = m_cLastPartMeasuredPt /*- m_cPartPatternPt*/;
  cmplx cMoveDist = ConvertCoordsRelativeToCenter( cDistFOV );
  bool bMeasurement = ( m_WorkingState >= ULF_Measurement );
  FXString Result ;
  if (CheckBlankSize( m_BlankCentersAndCorners , pOutInfo , m_Info3 ))
  {
    Result.Format( "Result=%s;dX=%.2f;dY=%.2f;dZ=%.2f;" ,
      bMeasurement ? "Blank" : "CalibrationDone" ,
      cMoveDist.real() , cMoveDist.imag() , m_dZAfterShift_um );
    SaveCSVLogMsg( "  %3d,%6.2f,%6.2f,%6.2f,OK" , m_iNProcessedBlanks ,
      cMoveDist.real() , cMoveDist.imag() , m_dZAfterShift_um );
    SaveLogMsg( "%s %d final result - %.2f,%.2f,%.2f,OK" ,
      bMeasurement ? "Blank" : "Calibration" ,
      bMeasurement ? m_iNProcessedBlanks : -1 ,
      cMoveDist.real() , cMoveDist.imag() , m_dZAfterShift_um );
    CheckAndSaveFinalImages( pOutInfo , false , bMeasurement ? "Blank" : "Calib" ) ;
  }
  else
  {
    Result.Format( "Result=%s;" ,
      bMeasurement ? "BadBlank" : "BadCalibration" );
    SaveCSVLogMsg( "  %3d,%6.2f,%6.2f,%6.2f,ERROR %s" , m_iNProcessedBlanks ,
      cMoveDist.real() , cMoveDist.imag() , m_dZAfterShift_um ,
      bMeasurement ? "on Blank" : "on CalibrationDone"
    );
    SaveLogMsg( "Blank %d final result - %.2f,%.2f,%.2f,%s" , m_iNProcessedBlanks ,
      cMoveDist.real() , cMoveDist.imag() , m_dZAfterShift_um , ( LPCTSTR ) m_Info3 );
    CheckAndSaveFinalImages( pOutInfo , true , bMeasurement ? "Blank" : "Calib" ) ;
  }
  SendMessageToEngine( Result , "EndOfMeasurement" );
  SwitchOffConstantLight( true , true );
  m_WorkingState = State_Idle;
  return 0;
}


int MPPT::SetBlankMode( int iExposure )
{
  LoadAndUpdatePartParameters();
  m_iFrontExposure_us = iExposure;
  SwitchOnConstantLight( m_CurrentPart.m_Blank.m_bBlankXYRingLightOn ,
    m_CurrentPart.m_Blank.m_bBlankXYStraightLightOn );
  ProgramExposure( m_iFrontExposure_us );
  return 0;
}


double MPPT::GetZResult()
{
  double dWaitStart = GetHRTickCount() ;
  while (m_dZAfterShift_um > 1.e308)
  {
    Sleep( 3 ) ;
    if (( GetHRTickCount() - dWaitStart ) > 100.)
      break ;
  }
  if (m_dZAfterShift_um < 1.e308)
    return m_dZAfterShift_um;
  return ( m_dZAfterShift_um = 0. ) ;
}


void MPPT::InitZGrabForULF( LPCTSTR pCommand )
{
  Sleep( ROUND( 0.001 * m_iFrontExposure_us ) + 2 ) ;
  m_dZAfterShift_um = DBL_MAX ;
  CTextFrame * pULSOrder = CreateTextFrame( pCommand == NULL ? "Grab" : pCommand , "FromULF" );
  PutFrame( GetOutputConnector( 4 ) , pULSOrder );
}


MEAS_OBJECT MPPT::WhatWeSee()
{
  switch (m_WorkingState)
  {
    case DL_LiveVideoPinFocusing:
    case DL_LiveVideoPinMeasure:
    case DL_LiveVideoPinNorth:
    case DL_LiveVideoPinEast:
    case DL_LiveVideoPinSouth:
    case DL_LiveVideoPinWest:
    case DL_PinToCenterForZ:
    case DL_PinZMeasureByDefocus:
    case DL_PinZDefocusToPlus:
    case DL_PinZDefocusToMinus:
    case DL_FinalImageOverPin:
    case DL_CorrectAfterFinalVision:
    case DL_PinNorthSide:
    case DL_PinEastSide:
    case DL_PinSouthSide:
    case DL_PinWestSide:
      return MO_Pin ;
    case   DL_LiveVideoCavity:
    case DL_LiveVideoCavityFocus:
    case DL_MeasCavity:
    case DL_MeasCavityXY:
    case DL_CavityJumpForZ:
    case DL_MeasCavityZ:
    case DL_MeasCavityZMultiple:
    case DL_MoveToCorrectZ:
    case DL_CaptureZWithCorrectHeigth:
    case DL_MoveCavityForFinalImage:
    case DL_SendCavityResult:
    case DL_CaptureCavityFinalImage:
    case DL_MeasZByDefocus:
    case DL_MeasZAfterExpAdjust:
    case DL_DefocusToPlus:
    case DL_DefocusToMinus:
    case DL_BadCavityOnDefocusing:
    case DL_LongSweep:
    case DL_ShortSweep:
      return MO_Cavity ;
  }
  if (ULS_Unknown <= m_WorkingState
    && m_WorkingState <= ULS_ZMeasurement)
    return MO_Blank_Side ;
  if (ULF_Unknown <= m_WorkingState
    && m_WorkingState <= ULF_MoveAfterMeasurement)
    return MO_Blank_Front ;

  return MO_Unknown ;
}


int MPPT::ProcessSideContours( CContainerFrame * pOutFrame , cmplx& LeftPt , cmplx& RightPt )
{
  if (!m_PartConturs.GetCount())
    return 0 ;

  CFigureFrame * pFig = ( CFigureFrame* ) m_PartConturs.GetFrame( 0 ) ;
  if (!pFig->Count())
    return 0 ;

  int iNearestIndex = -1 ;
  cmplx cAbsPt = m_cLastExtractedResult_pix + m_cLastROICenter ;
  cmplx cNearest = FindNearest( pFig , cAbsPt , iNearestIndex ) ;
  cmplx * pData = ( cmplx* ) pFig->GetData() ;

  CmplxArray Extrems ;
  FXIntArray Indexes ;
  cmplx cSize ;
  FindExtrems( pFig , Extrems , &Indexes , &cSize ) ;
  cmplx cUpperLeft( Extrems[ EXTREME_INDEX_RIGHT ].real() , Extrems[ EXTREME_INDEX_TOP ].imag() ) ;
  cmplx cUpperRight( Extrems[ EXTREME_INDEX_LEFT ].real() , Extrems[ EXTREME_INDEX_TOP ].imag() ) ;

  cmplx * pIter = pData ;
  cmplx * pEnd = pIter + pFig->Count() ;
  int iIndexUpperLeft = -1 , iIndexUpperRight = -1 ;
  // Find left and right upper points
  do
  {
    if (pIter->imag() == cUpperLeft.imag())
    {
      if (pIter->real() < cUpperLeft.real())
      {
        cUpperLeft._Val[ _RE ] = pIter->real() ;
        iIndexUpperLeft = ( int ) ( pIter - pData ) ;
      }
      if (pIter->real() > cUpperRight.real())
      {
        cUpperRight._Val[ _RE ] = pIter->real() ;
        iIndexUpperRight = ( int ) ( pIter - pData ) ;
      }
    }
  } while (++pIter < pEnd) ;
  // find most farest from left upper point
  int iIndexMaxLeft = -1 , iIndexMaxRight = -1 ;
  cmplx cLowerLeft = FindFarthestPt( cUpperRight , pFig , iIndexMaxLeft ) ;
  cmplx cLowerRight = FindFarthestPt( cUpperLeft , pFig , iIndexMaxRight ) ;
  if (m_ViewDetails > 5)
  {
    pOutFrame->AddFrame( CreatePtFrame( cUpperLeft , "Sz=5;color=0xffff00;" ) ) ;
    pOutFrame->AddFrame( CreatePtFrame( cUpperRight , "Sz=5;color=0xff00ff;" ) ) ;
    pOutFrame->AddFrame( CreatePtFrame( cLowerLeft , "Sz=5;color=0xffff00;" ) ) ;
    pOutFrame->AddFrame( CreatePtFrame( cLowerRight , "Sz=5;color=0xff00ff;" ) ) ;
  }

  CFRegression Horiz , LeftSide , RightSide ;
  Horiz.AddPtsToRegression( pData , ( int ) pFig->GetCount() , iIndexMaxRight + 5 , iIndexMaxLeft - 5 ) ;
  LeftSide.AddPtsToRegression( pData , ( int ) pFig->GetCount() ,
    iIndexMaxLeft + 5 , ( iIndexUpperLeft - 5 + ( int ) pFig->Count() ) % ( int ) pFig->Count() ) ;
  RightSide.AddPtsToRegression( pData , ( int ) pFig->GetCount() , iIndexUpperRight + 5 , iIndexMaxRight - 5 ) ;

  Horiz.Calculate() ;
  LeftSide.Calculate() ;
  RightSide.Calculate() ;

  double dHorizStd = Horiz.GetStdFromLine( pData , ( int ) pFig->GetCount() ,
    iIndexMaxRight + 5 , iIndexMaxLeft - 5 ) ;
  double dLeftStd = LeftSide.GetStdFromLine( pData , ( int ) pFig->GetCount() ,
    iIndexMaxLeft + 5 , ( iIndexUpperLeft - 5 + ( int ) pFig->Count() ) % ( int ) pFig->Count() ) ;
  double dRightStd = RightSide.GetStdFromLine( pData , ( int ) pFig->GetCount() ,
    iIndexUpperRight + 5 , iIndexMaxRight - 5 ) ;

  pOutFrame->AddFrame( CreateTextFrame( cmplx( 100. , 600. ) , "0x0000ff" , 20 , "" , 0 ,
    "HorizStd=%.2f\nLeftStd=%.2f\nRightStd=%.2f" , dHorizStd , dLeftStd , dRightStd ) ) ;

  CLine2d LHoriz = Horiz.GetCLine2d() ;
  CLine2d LSideL = LeftSide.GetCLine2d() ;
  CLine2d LSideR = RightSide.GetCLine2d() ;

  if (LHoriz.intersect( LSideL , LeftPt )
    && LHoriz.intersect( LSideR , RightPt ))
  {
    if (m_ViewDetails > 5)
    {
      pOutFrame->AddFrame( CreatePtFrame( LeftPt , "Sz=5;color=0x00ffff;" ) ) ;
      pOutFrame->AddFrame( CreatePtFrame( RightPt , "Sz=5;color=0x00ff00;" ) ) ;
      cmplx LViewPt = LeftPt + cmplx( -150 , 10 ) ;
      cmplx RViewPt = RightPt + cmplx( 100 , 10 ) ;
      pOutFrame->AddFrame( CreateTextFrame( LViewPt , "0xff0000" , 12 , "SouthEdge" , 0 , "SE(%.2f,%.2f)pix" ,
        LeftPt.real() , LeftPt.imag() ) ) ;
      pOutFrame->AddFrame( CreateTextFrame( RViewPt , "0xff0000" , 12 , "NorthEdge" , 0 , "NE(%.2f,%.2f)pix" ,
        RightPt.real() , RightPt.imag() ) ) ;
    }
    // blank FL as distance between corners (in pixels) scaled to microns
    double dBlankFL = abs(RightPt - LeftPt) * m_dScale_um_per_pix;
    cmplx BlankFLViewPt = LeftPt + cmplx(10, 100);
    pOutFrame->AddFrame(CreateTextFrame(BlankFLViewPt, "0xff", 14, "BlankFLValue", 0, "FL=%.2f um"));
    if ( dHorizStd > 2. )
      return -1 ;
    if (m_CurrentPart.m_Blank.m_UsedBlankEdge == 2)
    {
      if (dRightStd > 2.)
        return -2 ;
    }
    else if (dLeftStd > 2.)
      return -3 ;
  }
  else
    return 0 ;

  return 1 ;
}

int MPPT::IsExposureAndTimingOK( const CVideoFrame * pVF )
{
  VideoFrameEmbeddedInfo * pEmbedInfo = ( VideoFrameEmbeddedInfo* ) GetData( pVF );
  if (pEmbedInfo->m_dwIdentificationPattern == EMBED_INFO_PATTERN)
  {
    if (!m_bWasGoodFrame)
    {
      m_bWasGoodFrame = ( ROUND( pEmbedInfo->m_dExposure_us ) == m_iLastSettledExposure
        && pEmbedInfo->m_dGraphTime > m_dLastExposureSetTime + 5. );

      if (_tcsstr( pVF->GetLabel() , "Data" ))
        m_bWasGoodFrame = true ;
    }
    return m_bWasGoodFrame ;
  }
  return ( -1 );
}
void MPPT::GetPatternPoint()
{
  double dYPatternPoint = ( m_CurrentPart.m_Blank.m_dBlankHeight_um * 0.5 / m_dScale_um_per_pix ) ;
  if (m_CurrentPart.m_Blank.m_UsedBlankEdge == SQREdge_Upper)
    dYPatternPoint = -dYPatternPoint ;
  m_cPartPatternPt = cmplx( 0. , dYPatternPoint );
  m_CurrentPart.m_Blank.m_dBasePos_um = m_cPartPatternPt.imag();
}



CLine2d MPPT::GetRegressionNearPoint( const CFigure& Fig , const cmplx& cCentPt ,
  bool bXtoPlus , double dHalfLength , CContainerFrame * pOutInfo ,
  StraightLineRegression& SRegr )
{
  cmplx * pcFig = ( cmplx* ) Fig.GetData();
  int iIndex = FindNearestToYIndex( &Fig , cCentPt , true );

  cmplx cSegmCent = pcFig[ iIndex ];
  int iSegmBeginIndex = FindFigurePtOnDist( &Fig , iIndex , 1 , dHalfLength );
  int iSegmEndIndex = FindFigurePtOnDist( &Fig , iIndex , -1 , dHalfLength );

  if (m_ViewDetails > 3)
  {
    CFigureFrame * pCent = CreateFigureFrame( &cSegmCent , 1 , ( DWORD ) 0xff0000 );
    pOutInfo->AddFrame( pCent );
  }

  if (iSegmBeginIndex < 0 || iSegmEndIndex < 0)
    return CLine2d(); // no normal measurement, may be contour too small

  double dStd = SRegr.GetAndCalc( &Fig , cCentPt ,
    bXtoPlus ? 0. : M_PI , dHalfLength ,
    bXtoPlus ? -M_PI_2 : M_PI_2 , NULL , NULL , true );
  CLine2d SLine = SRegr.GetCLine2d();
  if (dStd > m_dLineFilterThres_pix)
  {
    double dNewStd = SRegr.DoFiltering( m_dMaxDistCut_pix );
    SLine = SRegr.GetFilteredLine();
  }

  cmplx cCalcUpper = SLine.GetPtForY( Fig[ iSegmBeginIndex ].y );
  cmplx cCalcLower = SLine.GetPtForY( Fig[ iSegmEndIndex ].y );

  if (m_ViewDetails >= 2)
  {
    CFigureFrame * pLine = CreateLineFrame( cCalcUpper ,
      cCalcLower , 0x00ff00 , "RegrEdge" );
    pOutInfo->AddFrame( pLine );
  }

  return SLine ;
}


int MPPT::CheckAndSendCavityPlot()
{
  double dNow = GetHRTickCount();
  if (( m_dXY_PlotPeriod != 0. ) && ( dNow - m_dLastPlotTime > m_dXY_PlotPeriod ))
  {
    if (m_PlotSampleCntr == 0)
    {
      FXString FileName ;
      FileName = m_CurrentReportsDir + "CavityPosDataLive_" + GetTimeAsString_ms() + ".csv" ;
      PutFrame( GetOutputConnector( 2 ) , CreateTextFrame( FileName , "SaveFileName" , 0 ) ) ;
    }
    m_PlotSampleCntr++ ;

    CQuantityFrame * pAvX = CreateQuantityFrame(
      m_cLastCavityXYAveraged_um.real() , "Plot_Append:dCavAvX" , m_PlotSampleCntr );
    PutFrame( GetOutputConnector( 2 ) , pAvX );
    CQuantityFrame * pAvY = CreateQuantityFrame(
      m_cLastCavityXYAveraged_um.imag() , "Plot_Append:dCavAvY" , m_PlotSampleCntr );
    PutFrame( GetOutputConnector( 2 ) , pAvY );
    //     CQuantityFrame * pX = CreateQuantityFrame(
    //       m_cLastCavityXYResult_um.real() , "Plot_Append:dCavX" , m_PlotSampleCntr );
    //     PutFrame( GetOutputConnector( 2 ) , pX );
    //     CQuantityFrame * pY = CreateQuantityFrame(
    //       m_cLastCavityXYResult_um.imag() , "Plot_Append:dCavY" , m_PlotSampleCntr );
    //     PutFrame( GetOutputConnector( 2 ) , pY );
        //               CQuantityFrame * pZ = CreateQuantityFrame(m_dExtdZ, "Plot_Append:dZ");
        //               PutFrame(GetOutputConnector(2), pZ);
    m_dLastPlotTime = dNow;
  }
  return 0;
}

int MPPT::SetWatchDog( int iTime_ms )
{
  DeleteWatchDog();
  if (!CreateTimerQueueTimer( &m_hWatchDogTimer , NULL , ( WAITORTIMERCALLBACK ) MPPTWatchdogTimerRoutine ,
    this , iTime_ms , 0 , 0 ))
  {
    SEND_GADGET_ERR( "Create Watchdog timer failed" );
    return false;
  }
  return true;
}
bool MPPT::DeleteWatchDog()
{
  m_LockWatchdog.Lock();
  if (m_hWatchDogTimer != NULL)
  {
    BOOL bDeleteTimerQueueTimer = DeleteTimerQueueTimer( NULL , m_hWatchDogTimer , NULL );
    m_hWatchDogTimer = NULL;
    if (!bDeleteTimerQueueTimer)
    {
      if (GetLastError() != ERROR_IO_PENDING)
      {
        SEND_GADGET_ERR( _T( "Can't delete Watchdog Timer" ) );
        m_LockWatchdog.Unlock();
        return false;
      }
    }
  }
  m_LockWatchdog.Unlock();
  return true;
}



int MPPT::PlotULandCheckJumps( cmplx& cDistFOV_um , double dZVal_um )
{
  double dNow = GetHRTickCount();
  bool bCurrentOK = ( abs( cDistFOV_um ) < 1000. && fabs( dZVal_um ) < 1000. ) ;
  bool bPrevOK = ( abs( m_cPrevMeasuredXY ) < 1000. && fabs( m_dPrevMeasuredZ ) < 1000. );
  if (bCurrentOK)
  {
    bCurrentOK = true;
    if (( m_dXY_PlotPeriod != 0. ) && ( dNow - m_dLastPlotTime > m_dXY_PlotPeriod ))
    {
      CQuantityFrame * pX = CreateQuantityFrame( cDistFOV_um.real() , "Plot_Append:dX" , m_PlotSampleCntr );
      PutFrame( GetOutputConnector( 2 ) , pX );
      CQuantityFrame * pY = CreateQuantityFrame( cDistFOV_um.imag() , "Plot_Append:dY" , m_PlotSampleCntr );
      PutFrame( GetOutputConnector( 2 ) , pY );
      CQuantityFrame * pZ = CreateQuantityFrame( dZVal_um , "Plot_Append:dZ" , m_PlotSampleCntr );
      PutFrame( GetOutputConnector( 2 ) , pZ );
      m_dLastPlotTime = dNow;
      if (m_PlotSampleCntr++ == 0)
      {
        FXString FileName;
        FileName = m_CurrentReportsDir + "BlankPosData_" + GetTimeAsString_ms() + ".csv";
        PutFrame( GetOutputConnector( 2 ) , CreateTextFrame( FileName , "SaveFileName" , 0 ) );
      }
      //       else if ( m_iWaitCounter > 3 )
      //         ASSERT( !bPrevOK || 
      //           (( abs(m_cPrevMeasuredXY - cDistFOV_um) < 10. )
      //             && ( fabs(dZVal_um - m_dPrevMeasuredZ) < 10. )));
    }
  }
  if (bCurrentOK)
  {
    m_cPrevMeasuredXY = cDistFOV_um;
    m_dPrevMeasuredZ = m_dExtdZ;
  }
  return 0;
}


int MPPT::FindCavityInternalEdges( cmplx cLeftCenter , cmplx cRightCenter ,
  cmplx& cLeftEdge , cmplx& cRightEdge , CContainerFrame * pMarking )
{
  int iYDistFromCent_pix = m_CurrentPart.m_Cavity.m_iCentralZoneWidth_pix / 2 ;
  double dRectWidth_pix = m_CurrentPart.m_Cavity.m_dPlaneWidth_um / m_dScale_um_per_pix ;
  CRect LeftSearchRect( ROUND( cLeftCenter.real() ) ,
    ROUND( cLeftCenter.imag() - iYDistFromCent_pix ) ,
    ROUND( cLeftCenter.real() + dRectWidth_pix ) ,
    ROUND( cLeftCenter.imag() + iYDistFromCent_pix ) ) ;
  CRect RightSearchRect( ROUND( cRightCenter.real() - dRectWidth_pix ) ,
    ROUND( cRightCenter.imag() - iYDistFromCent_pix ) ,
    ROUND( cRightCenter.real() ) ,
    ROUND( cRightCenter.imag() + iYDistFromCent_pix ) ) ;

  Profile LeftProfile , RightProfile ;
  double dLeftProfAver = 0. , dRightProfAver = 0. ;
  if (!GetHProfile( m_pCurrentImage , LeftProfile , LeftSearchRect , &dLeftProfAver ))
    return 0 ;

  if (!GetHProfile( m_pCurrentImage , RightProfile , RightSearchRect , &dRightProfAver ))
    return 0 ;

  double dLeftProfileScale = 100. / ( LeftProfile.m_dMaxValue - LeftProfile.m_dMinValue ) ;
  cmplx cLeftOrigin( cLeftCenter - cmplx( 0. , iYDistFromCent_pix + 10. ) ) ;
  if (m_ViewDetails > 2)
  {
    CmplxArray LeftView ;
    for (int i = 0 ; i < LeftProfile.m_iProfLen ; i++)
    {
      cmplx Pt = cLeftOrigin + cmplx( i ,
        -( LeftProfile.m_pProfData[ i ] - LeftProfile.m_dMinValue )  * dLeftProfileScale ) ;
      LeftView.Add( Pt ) ;
    }
    CFigureFrame * pLeftProfileView = CreateFigureFrame( LeftView.GetData() , ( int ) LeftView.Size() ,
      ( DWORD ) 0xffff00 , "LeftProfile" ) ;
    pMarking->AddFrame( pLeftProfileView ) ;
    CRectFrame * pLeftZone = CRectFrame::Create( &LeftSearchRect ) ;
    *( pLeftZone->Attributes() ) = "color=0xffff00;" ;
    pMarking->AddFrame( pLeftZone ) ;
  }

  double dRightProfileScale = 100. / ( RightProfile.m_dMaxValue - RightProfile.m_dMinValue ) ;
  cmplx cRightOrigin( cRightCenter - cmplx( RightProfile.m_iProfLen , iYDistFromCent_pix + 10. ) ) ;
  if (m_ViewDetails > 2)
  {
    CmplxArray RightView ;
    for (int i = 0 ; i < RightProfile.m_iProfLen ; i++)
    {
      cmplx Pt = cRightOrigin + cmplx( i ,
        -( RightProfile.m_pProfData[ i ] - RightProfile.m_dMinValue ) * dRightProfileScale ) ;
      RightView.Add( Pt ) ;
    }
    CFigureFrame * pRightProfileView = CreateFigureFrame( RightView.GetData() , ( int ) RightView.Size() ,
      ( DWORD ) 0x00ffff , "RightProfile" ) ;
    pMarking->AddFrame( pRightProfileView ) ;
    pMarking->AddFrame( CreateRectFrame( RightSearchRect , 0x00ffff ) ) ;
  }

  int i = 0 ;
  for (; i < LeftProfile.m_iProfLen ; i++)
  {
    if (LeftProfile.m_pProfData[ i ] == LeftProfile.m_dMaxValue)
      break ;
  }
  double dThres = LeftProfile.m_dMinValue + 0.75 * ( LeftProfile.m_dMaxValue - LeftProfile.m_dMinValue ) ;
  double dPos = find_border_forw( LeftProfile.m_pProfData + i ,
    LeftProfile.m_iProfLen - i - 1 , dThres ) ;
  if (dPos > 0.)
    cLeftEdge = cLeftCenter + cmplx( dPos + i , 0. ) ;
  else
    return 0 ; // not found

  i = RightProfile.m_iProfLen - 1 ;
  for (; 0 < i ; i--)
  {
    if (RightProfile.m_pProfData[ i ] == RightProfile.m_dMaxValue)
      break ;
  }
  dThres = RightProfile.m_dMinValue + 0.75 * ( RightProfile.m_dMaxValue - RightProfile.m_dMinValue ) ;
  dPos = find_border_forw( RightProfile.m_pProfData , RightProfile.m_iProfLen - 1 , dThres ) ;
  if (dPos > 0.)
    cRightEdge = cRightCenter + cmplx( dPos - RightSearchRect.Width() , 0. ) ;
  else
    return 0 ; // not found

  CFigureFrame * pLeft = CreatePtFrame( cLeftEdge , "color=0;Sz=5;" ) ;
  CFigureFrame *pRight = CreatePtFrame( cRightEdge , "color=0;Sz=5;" ) ;
  pMarking->AddFrame( pLeft ) ;
  pMarking->AddFrame( pRight ) ;
  return 1 ; // OK
}

static int bBypassContinue = 0 ;
bool MPPT::ProcessContinueCommand()
{

  if (!bBypassContinue && m_bWaitForContinueCommand
    && ( ( m_WorkingMode == MPPTWM_UpFront ) || ( m_WorkingMode == MPPTWM_Down ) ))
  {
    m_bWaitForContinueCommand = 2 ;
    m_cPartPatternPt = m_cLastBlankAvPos_pix - m_cLastROICenter;
    if (m_WorkingMode == MPPTWM_UpFront)
    {
      m_WorkingState = ULF_WaitForExternalReady;
      GrabImage();
    }
    return true;
    //SendMessageToEngine("OK, waiting for continue finished" , "Waiting finished"); //EM: commented since it may be cause of error
  }
  return false ;
}

