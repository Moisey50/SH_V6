#include "stdafx.h"
#include "MPPT_Dispens.h"
#include <math/PlaneGeometry.h>
#include <math/PlaneCircle.h>

// double MPPT_Dispens::GetEccentricity( int iNSamples , double& dDia )
// {
//   CmplxVector Centers ;
//   double dAvgDia = 0. ;
//   while (iNSamples--)
//   {
//     CVideoFrame * pVF = m_AccumulatedFrames.front()->GetVideoFrame() ;
//     m_AccumulatedFrames.erase( m_AccumulatedFrames.begin() ) ;
//     double dCurrDia ;
//     cmplx CurrCent = GetCircleCenter( Image , double& dCurrDia ) ;
//     Centers.push_back( CurrCent ) ;
//     dAvgDia += dCurrDia ;
//   }
//   dDia = dAvgDia / iNSamples ;
//   return GetEccentricity( Centers ) ; // function returns eccentricity as double
// }
#ifdef _DEBUG
static double dTimeoutTime = 240000;
#else
static double dTimeoutTime = 10000;
#endif



bool MPPT_Dispens::DoMove( MotionPhase Phase , double dDist , cmplx& cLastMotion )
{
  switch ( Phase )
  {
    case MF_XPlus:
      cLastMotion._Val[ _RE ] = dDist;
      DoMoveX( dDist );
      return true;
    case MF_YPlus:
      cLastMotion._Val[ _IM ] = dDist;
      DoMoveY( dDist );
      return true;
    default:
      break;
  }
  return false;
}
// bool MPPT_Dispens::FindPointWithMinimalEccentricity(MotionPhase& Phase , double& dEccentr , cmplx& cLastMotion )
// {
//   double dDia , dEcc1 , dEcc2 ;
// 
//   double dLastStep = dEccentr * MotionStep ;
//   DoMove( MotionPhase , dLastStep , cLastMotion ) ;
//   dEcc1 = GetEccentricity( N_Samples , dDia ) ;
//   if (dEcc1 < dEccentr)
//   {
//     dEccentr = dEcc1 ;
//     return ( dEccentr < EccThreshold ) ;
//   }
//   else
//   {
//     dLastStep = -dEccentr * MotionStep * 1.5 ;
//     DoMove( MotionPhase , dLastStep , cLastMotion ) ;
//     dEcc2 = GetEccentricity( N_Samples , dDia ) ;
//     if (dEcc2 < dInitEccentr)
//     {
//       dEccentr = dEcc2 ;
//       return ( dEccentr < EccThreshold ) ;
//     }
//   }
//   return false ;
// }
// 
// bool MPPT_Dispens::CenteringProcedure(  ) // command for centering is sent from control program
// {
//   // Initial Eccentricity, after Xmotion, after Ymotion
//   double dErrEccCenter , dErrEccDx , dErrEccDx2 , dErrEccdY , dErrEccdY ;
//   double dCurrentEccentr ;
//   cmplx cMotion ;
//   MotionPhase Phase = MF_XPlus , PrevPhase = MF_NoMotion ;
//   int iNSteps = 0 ;
//   do
//   {
//     double dDia , dLastStep ;
//     dCurrentEccentr = GetEccentricity( N_Samples , dDia ) ;
//     if ( dErrEccCenter > EccThreshold )
//     {
//       if (FindPointWithMinimalEccentricity( Phase , dCurrentEccentr , cMotion ))
//         break ;
//       Phase = MF_YPlus ;
//       if (FindPointWithMinimalEccentricity( Phase , dCurrentEccentr , cMotion ))
//         break ;
//       Phase = MF_XPlus ;
//     }
//   } while (dErrEccCenter > EccThreshold );
// }
// 

CenteringStatus MPPT_Dispens::CenteringWithSync(
  const CDataFrame * pDataFrame , CContainerFrame * pMarking )
{
  double dCaptureTime = 0.;
  if ( !pDataFrame ) // Init Centering
  {
    InitCentering();
    return m_Centering; // OK for adjustment start
  }
  datatype Type = pDataFrame->GetDataType();

  switch ( Type )
  {
    case text:// command or answer from motion
      switch ( m_Centering )
      {
        case CNTST_AdjustForWhite:
        case CNTST_AdjustForBlack:
          if ( m_dYMotionDist_pix != 0. ) // case when X motion is finished
          {
            DoMoveY( m_dYMotionDist_pix );
            m_Centering = ( !m_bIsBlackMeasurement ) ?
              CNTST_AdjustForWhite : CNTST_AdjustForBlack;
            m_dYMotionDist_pix = 0.; // Y motion is ordered
          }
          else // motion is finished, necessary to remeasure
          {
            if ( m_bIsSynchronized )
            {
              m_Centering = ( !m_bIsBlackMeasurement ) ?
                CNTST_MeasureByWhite : CNTST_MeasureByBlack;
            }
            else
            {
              m_Centering = CNTST_Fault;
              InitCentering();
            }
          }
          return m_Centering;
      }
      return CNTST_Unknown;
    default:
      if ( pDataFrame->IsContainer() )// imaging result
      {
        const CVideoFrame * pVF = pDataFrame->GetVideoFrame();

        dCaptureTime = GetFrameTSFromEmbeddedInfo( pVF );
        if ( dCaptureTime == 0. )
          pVF->GetTime();
        m_CaptureTimes.push_back( dCaptureTime );
        ASSERT( dCaptureTime > 0. );
        if ( m_CaptureTimes.size() > N_MEASUREMENTS_PER_TURN * 2 )
          m_CaptureTimes.erase( m_CaptureTimes.begin() );
        if ( m_bIsSynchronized )
        {
          if ( ++m_iCurrentFrameNumberInTurn >= N_MEASUREMENTS_PER_TURN )
          {
            double dPeriod = dCaptureTime - m_dLastSyncTime_ms;
            double dPeriodDiff = ( dPeriod - m_dLastCalculatedRotationPeriod_ms );
            double dNormPeriodDiff = dPeriodDiff / m_dLastCalculatedRotationPeriod_ms;
            if ( fabs( dNormPeriodDiff ) > 2. * m_dRotationPeriodTolerance )
              m_bIsSynchronized = false;
            else
              m_iCurrentFrameNumberInTurn = 0;
          }
        }
        switch ( m_Centering )
        {
          case CNTST_Syncing:
          {
            double dLastPeriod = GetCapturePeriod() ;
            ASSERT( 0 ) ;
          }
          return m_Centering;
          case CNTST_MeasureByBlack:
          case CNTST_MeasureByWhite:
          {
            if ( m_bIsSynchronized )
            {
              LPCTSTR pObjectName =
                ( m_Centering == CNTST_MeasureByWhite ) ? "whitespot" : "blank_int";
              if ( GetCircles( pObjectName , m_FoundCircles , pDataFrame , pMarking ) )
              {
                m_FoundCircles.rbegin()->m_iPositionInTurn = m_iCurrentFrameNumberInTurn;
                if ( ( m_FoundCircles.size() >= N_MEASUREMENTS_PER_TURN )
                  && ( m_FoundCircles[ 0 ].m_iPositionInTurn == 0 ) )
                {
                  ASSERT( m_FoundCircles[ CD_XPlus ].m_dTime );
                  CirclesVector Saved = m_FoundCircles;
                  m_FoundCircles.clear();
                  cmplx cXP = m_FoundCircles[ CD_XPlus ].m_cCenter;
                  cmplx cYP = m_FoundCircles[ CD_YPlus ].m_cCenter;
                  cmplx cXM = m_FoundCircles[ CD_XMinus ].m_cCenter;
                  cmplx cYM = m_FoundCircles[ CD_YMinus ].m_cCenter;
                  cmplx cCenter = ( cXP + cYP + cXM + cYM ) / 4.;

                  cmplx cDeviationFromCenter = cXP - cCenter;
                  double dX = ScalarMult( m_cXVector , cDeviationFromCenter );
                  double dY = ScalarMult( m_cYVector , cDeviationFromCenter );
                  if ( dX < m_dCenteringTolerance_pix && dY < m_dCenteringTolerance_pix )
                    m_Centering = CNTST_Finished;
                  else
                  {
                    if ( abs( dY ) >= m_dCenteringTolerance_pix )
                      m_dYMotionDist_pix = -dY * 100. / abs( m_cYVector );
                    else
                      m_dYMotionDist_pix = 0.;
                    if ( abs( dX ) >= m_dCenteringTolerance_pix )
                    { // X motion ordering, when will be finished the m_dYMotionTime_ms
                      // will be analyzed
                      DoMoveX( -dX * 100. / abs( m_cXVector ) );
                      m_Centering = ( m_Centering == CNTST_MeasureByWhite ) ?
                        CNTST_AdjustForWhite : CNTST_AdjustForBlack;
                    }
                    else if ( m_dYMotionDist_pix != 0. )
                    {
                      DoMoveY( m_dYMotionDist_pix );
                      m_Centering = ( m_Centering == CNTST_MeasureByWhite ) ?
                        CNTST_AdjustForWhite : CNTST_AdjustForBlack;
                      m_dYMotionDist_pix = 0.; // Y motion is ordered
                    }
                    else
                      m_Centering = CNTST_Finished; // no necessary motions
                  }
                  m_FoundCircles.erase( m_FoundCircles.begin() ,
                    m_FoundCircles.begin() + N_MEASUREMENTS_PER_TURN - 1 );
                }
                if ( m_FoundCircles.size() > 0
                  && ( m_FoundCircles[ 0 ].m_iPositionInTurn != 0 ) )
                {
                  m_FoundCircles.erase( m_FoundCircles.begin() );
                }
              }
            }
          }
          return m_Centering;
        }
      }
  }
  return CNTST_Unknown;
}

int MPPT_Dispens::InitCentering()
{
  switch ( m_Centering )
  {
    default:
    case CNTST_Idle:
    case CNTST_Fault:
    case CNTST_Finished:
      m_CaptureTimes.clear();
      m_Centering = CNTST_Syncing;
      m_bIsSynchronized = false;
      break;
  }
  FXRegistry Reg( "TheFileX\\MPP_Dispens" );
  m_dRotationPeriod_ms = Reg.GetRegiDouble( "Control" , "RotationPeriod_ms" , 113. ) ;
  m_dRotationPeriodTolerance = Reg.GetRegiDouble( "Control" , "RotationTolerance_perc" , 1. ) * 0.01;
  m_dCenteringTolerance_pix = Reg.GetRegiDouble( "Control" , "CenteringTolerance_pix" , 1. );

  m_cXVector = cmplx( 1. , 0. );
  m_cYVector = cmplx( 0. , -1. );
  Reg.GetRegiCmplx( "Control" , "XVector" , m_cXVector , m_cXVector );
  Reg.GetRegiCmplx( "Control" , "YVector" , m_cYVector , m_cYVector );
  return 0;
}

double MPPT_Dispens::GetFrameTSFromEmbeddedInfo( const CVideoFrame * pVF )
{
  if ( pVF )
  {
    VideoFrameEmbeddedInfo * pEmbedInfo = ( VideoFrameEmbeddedInfo* ) GetData( pVF );
    if ( pEmbedInfo->m_dwIdentificationPattern == EMBED_INFO_PATTERN )
    {
      m_CameraROI = pEmbedInfo->m_ROI ;
      return pEmbedInfo->m_dGraphTime;
    }
  }
  return 0.0;
}

double MPPT_Dispens::GetCapturePeriod()
{
  if ( m_CaptureTimes.size() >= 3 )
  {
    double dLast = m_CaptureTimes[ m_CaptureTimes.size() - 1 ];
    double dPrev = m_CaptureTimes[ m_CaptureTimes.size() - 2 ];
    double dPrevPrev = m_CaptureTimes[ m_CaptureTimes.size() - 3 ];
    double dLastDiff = dLast - dPrev;
    double dRelError = fabs( dLastDiff - m_dRotationPeriod_ms ) / m_dRotationPeriod_ms ;
    if ( dRelError > 1.5 )
      dRelError -= 1.0 ;
    bool bPeriodIsOK = dRelError < m_dRotationPeriodTolerance ;

    double dPrevDiff = dPrev - dPrevPrev;
    double dDiffDiff = dLastDiff - dPrevDiff;
    double dAverPeriod = 0.5 * ( dLastDiff + dPrevDiff );
    double dError = fabs( dDiffDiff / dAverPeriod );
    if ( bPeriodIsOK )
//       if ( dError < m_dRotationPeriodTolerance )
    {
      m_dLastSyncTime_ms = dLast;
      m_dLastCalculatedRotationPeriod_ms = dLastDiff ; // dAverPeriod;
      m_CaptureTimes.erase( m_CaptureTimes.begin() );
      return m_dLastCalculatedRotationPeriod_ms;
//       double dRequestedFramePeriod_ms = dAverPeriod / N_MEASUREMENTS_PER_TURN ;
//       double dFPS = 1000. / dRequestedFramePeriod_ms ;
//       SetBurstTriggerMode( dFPS ) ;
//       LightOn( LightMask_Back ) ; // only front back light will be used
//       m_FoundCircles.clear() ;
//       m_FoundExtCircles.clear() ;
//       m_FoundIntCircles.clear() ;
//       m_bIsSynchronized = true ;
//       m_iCurrentFrameNumberInTurn = 0 ;
//       m_Centering = (m_bIsBlackMeasurement) ? CNTST_MeasureByBlack : CNTST_MeasureByWhite ;
    }
  }
  return ( m_dLastSyncTime_ms = 0. );
}

int MPPT_Dispens::ProcessImageForCentering( cmplx& TargetForCenter ,
  cmplx& cMeasuredCenter , LPCTSTR pOKMsg , LPCTSTR pCommandToEngine ,
  MPPD_Stage NextWorkingStateOnOK , CContainerFrame * pMarking )
{
  if ( (m_bLastWhiteDIOK || m_bCenteringWithBlack ) && ( m_iFrameCntAfterDone >= 2 ) )
  {
    TargetForCenter = cMeasuredCenter;
    m_AdditionalMsgForManual = pOKMsg;
    pMarking->AddFrame( CreateTextFrame( m_cLastROICent_pix , "0x0000ff" , 14 ,
      NULL , 0 , "Synchronized, Period = %.5fms F=%.5fHz\n%s\n%s" ,
      m_dLastCalculatedRotationPeriod_ms ,
      1000.0 / m_dLastCalculatedRotationPeriod_ms ,
      pCommandToEngine ,
      ( LPCTSTR ) m_AdditionalMsgForManual ) );
    if ( pCommandToEngine )
      SendMessageToEngine( pCommandToEngine , "ToEngine" );
    if ( ( int ) NextWorkingStateOnOK >= 0 )
      m_WorkingStage = NextWorkingStateOnOK;
    m_bDone = false;
    m_iFrameCntAfterDone = 0;
    return 1;
  }
  else
  {
    if ( GetHRTickCount() - m_dLastCommandTime > dTimeoutTime )
    {
      cmplx cViewTimeout( m_cLastROICent_pix + cmplx( -200. , -300. ) );
      pMarking->AddFrame( CreateTextFrame( cViewTimeout ,
        "0x0000ff" , 24 , "Timeout" , 0 , "Timeout after centering motion" ) );
      m_WorkingStage = STG_Idle; // stop processing
      return -1;
    }
  }
  return 0;
}

void MPPT_Dispens::SetCameraTriggerParams( bool bTriggerOn , double dTriggerDelay_us )
{
  FXString CameraCommand;
  CameraCommand.Format( "set properties"
    "(Trig_Selector=FrameStart;Trigger_mode=%s;"
    "Trigger_source=Line1;Trigger_delay_us=%d;);" ,
    ( bTriggerOn ) ? "On" : "Off" ,
    ( dTriggerDelay_us >= 0. ) ? ROUND( dTriggerDelay_us ) : 0 );
  m_TriggerMode = ( bTriggerOn ) ? TM_OneFrame : TM_NoTrigger;
  m_dLastTriggerDelayTime_us = dTriggerDelay_us;
  ProgramCamera( CameraCommand );
}

void MPPT_Dispens::SetNoTriggerMode()
{
  SetCameraTriggerParams( false ); // reset trigger mode
}

bool MPPT_Dispens::SetRotationPeriodMeasurement()
{
  return SetSimpleTriggerMode();
}

BOOL MPPT_Dispens::CameraControl( LPCTSTR pCommand )
{
  FXString Command;
  Command.Format( "set properties(%s;);" , pCommand );
  CTextFrame * pOut = CreateTextFrame( Command , "CameraCommand" );
  return PutFrame( GetOutputConnector( MPPDO_CameraControl ) , pOut , 200 );
}

BOOL MPPT_Dispens::SetSimpleTriggerMode()
{
  FXString CameraCommand;
  CameraCommand.Format( "Trig_Selector=FrameStart;"
    "Trigger_mode=On;Trigger_Source=Line1;"
    "Trigger_delay_us=0;Trigger_activate=RisingEdge;"
    "LineSelect=Out1;LineSource=%s;" , 
    m_bUseFrontLight ? "TimerActive" : "UserOutput"
    );
  m_dLastTriggerDelayTime_us = 0.;
  m_TriggerMode = TM_OneFrame;
  m_CaptureTimes.clear();
  return CameraControl( CameraCommand );
}

BOOL MPPT_Dispens::SetBurstTriggerMode( double dFrameRate , int iNFrames )
{
  FXString CameraCommand;
  CameraCommand.Format( "Trig_Selector=FrameBurstStart;Trigger_mode=On;Trigger_Source=Line1;"
    "Trigger_delay_us=0;Trigger_activate=RisingEdge;Burst_Length=%d;Frame_Rate=%.3f;" ,
    iNFrames , dFrameRate );
  m_TriggerMode = ( MPPD_TriggerMode ) iNFrames;
  m_CaptureTimes.clear();
  return CameraControl( CameraCommand );
}


BOOL MPPT_Dispens::SetExposureAndGain( double dExposure_us , double dGain )
{
  FXString CameraCommand;
  CameraCommand.Format( "set properties(Shutter_us=%d;Gain_dBx10=%d;);" ,
    ROUND( dExposure_us ) , ROUND( dGain ) );
  return CameraControl( CameraCommand );
}

int MPPT_Dispens::SetExposure( int iExp )
{
  FXString CameraCommand;
  CameraCommand.Format( "set properties(shutter_us=%d;);" , iExp );
  return CameraControl( CameraCommand );
}

void MPPT_Dispens::DoAdjustForCentering( CenterMoving Axe , double dDist_pix )
{
  // Convert to ms
  double dTime_ms = 0.;

  FXString Command;
  if ( dDist_pix >= 0. )
  {
    if ( Axe == CM_MoveX )
      dTime_ms = 1000. * dDist_pix / abs( m_cXScalePlus_pix_per_sec );
    else
      dTime_ms = 1000. * dDist_pix / abs( m_cYScalePlus_pix_per_sec );
  }
  else
  {
    if ( Axe == CM_MoveX )
      dTime_ms = 1000. * dDist_pix / abs( m_cXScaleMinus_pix_per_sec );
    else
      dTime_ms = 1000. * dDist_pix / abs( m_cYScaleMinus_pix_per_sec );
  }
  Command.Format( Axe == CM_MoveX ? "MoveX=%d;" : "MoveY=%d;" , ROUND( dTime_ms ) );
  SendMessageToEngine( Command , "MoveCenter" );
}

BOOL MPPT_Dispens::SetFrontLight( bool bOn , bool bHighBrightness )
{
  FXRegistry Reg("TheFileX\\MPP_Dispens");
  BOOL bDCFrontLight = Reg.GetRegiInt("Control", "DCFrontLight", 1 );
  FXString LightCommand;

  if ( bDCFrontLight )
    LightCommand.Format("Chan=2;Value=%d;", bOn ? 1 : 0);
  else
  {
    LightCommand.Format( "LineSelect=Out1;LineSource=%s;"
      "Timer1=(TimerTriggerSource_=ExposureActive;TimerDuration_=%d;);",
      bOn ? "TimerActive" : "UserOutput" , m_CurrentPart.m_iFrontForBlackExposure_us );
  }
  m_bUseFrontLight = bOn;
  if ( bOn )
  {
    m_bHighBrightnessMode = bHighBrightness;
    int iExposure_us = m_CurrentPart.m_iFrontForBlackExposure_us * (( bHighBrightness ) ? 2 : 1) ;
    SetExposureAndGain( iExposure_us , m_CurrentPart.m_iGainForBlack_dBx10 );
    ProgramImaging( bHighBrightness && (m_WorkingMode == MPPD_Front)? "Task(-1)" : "Task(4);" );
    m_iCurrentLightMask |= LightMask_Front ;
  }
  else
  {
    m_iCurrentLightMask &= ~LightMask_Front;
    m_bHighBrightnessMode = false;
  }

  return CameraControl(LightCommand);
}

BOOL MPPT_Dispens::SetBackLight( bool bOn )
{
  FXString LightCommand;
  LightCommand.Format( "Chan=1;Value=%d;" , bOn ? 1 : 0 );
  m_bUseBackLight = bOn;
  if ( bOn )
  {
    SetExposureAndGain( m_CurrentPart.m_iFrontForWhiteExposure_us ,
      m_CurrentPart.m_iGainForWhite_dBx10 );
    ProgramImaging( "Task(4);" );
    m_iCurrentLightMask |= LightMask_Back ;
  }
  else
    m_iCurrentLightMask &= ~LightMask_Back ;
  return CameraControl( LightCommand );
}

// returns true, if N accumulated is equal or bigger than m_iAveragingC
bool MPPT_Dispens::AccumulateLastCmplxResult( cmplx cResult )
{
  m_cAccumulator.push_back( cResult );
  return ( int ) m_cAccumulator.size() >= m_iAveragingC;
}

cmplx MPPT_Dispens::FilterAndGetMainPt()
{
  if ( m_cAccumulator.size() <= 1 )
    return cmplx();
  cmplx cOrigin = m_cAccumulator[ 0 ];
  cmplx cAver;
  for ( int i = 1; i < ( int ) m_cAccumulator.size(); i++ )
  {

  }
  return cAver;
}

void MPPT_Dispens::SetGrabForCentering( MPPD_Stage Stage )
{
  int iPhase = ( Stage == STG_Get0DegImageForAverage ) ? 0 : Stage - STG_Get0DegImage;
  SetCameraTriggerParams( true , m_dLastCalculatedRotationPeriod_ms * 1000. * iPhase / 4. );
  if ( m_bCenteringWithBlack )
  {
    SetExposureAndGain( m_CurrentPart.m_iFrontForBlackExposure_us , m_CurrentPart.m_iGainForBlack_dBx10 );
    SetFrontLight( true );
    SetBackLight( false );
  }
  else
  {
    SetExposureAndGain( m_CurrentPart.m_iFrontForWhiteExposure_us , m_CurrentPart.m_iGainForWhite_dBx10 );
    SetFrontLight( false );
    SetBackLight( true );
  }
  if ( Stage == STG_Get0DegImageForAverage ) 
  {
    cmplx cWhiteSpotCenterShift_pix = m_cLastIntWhiteCenter_pix - m_cLastROICent_pix ;
    int iXOffset = ROUND( cWhiteSpotCenterShift_pix.real() ) ;
    int iYOffset = ROUND( cWhiteSpotCenterShift_pix.imag() ) ;
    if ( abs( iXOffset ) > 2 )
    {
      int iNXSteps = ( iXOffset / 4 ) + ( iXOffset > 0 ) ? 1 : -1 ;
      iXOffset = iNXSteps * 4 ;
    }
    else
      iXOffset = 0 ;
    if ( abs( iYOffset ) >= 2 )
    {
      int iNXSteps = ( iYOffset / 4 ) + ( iYOffset > 0 ) ? 1 : -1 ;
      iYOffset = iNXSteps * 4 ;
    }
    else
      iYOffset = 0 ;
    if ( iXOffset || iYOffset )
      OffsetCameraROI( iXOffset , iYOffset ) ;
  }

  m_WorkingStage = Stage;
  m_iFrameCntAfterDone = 0;
  m_bDone = true; // no motion necessary
  m_AccumulatedFrames.clear();
}

int MPPT_Dispens::ProgramCamera( LPCTSTR pCameraParams , LPCTSTR pLabel )
{
  CTextFrame * pCamCom = CreateTextFrame( pCameraParams ,
    pLabel ? pLabel : "CameraParamsSet" );

  m_LastCameraCommands.insert( m_LastCameraCommands.begin() , pCameraParams );
  if ( m_LastCameraCommands.size() > 40 )
    m_LastCameraCommands.erase( m_LastCameraCommands.end() - 1 );
  return PutFrame( GetOutputConnector( MPPDO_CameraControl ) , pCamCom );
}

int MPPT_Dispens::ProgramImaging( LPCTSTR pImagingControl , LPCTSTR pLabel )
{
  CTextFrame * pCamCom = CreateTextFrame( pImagingControl ,
    pLabel ? pLabel : "ImagingControl" );

  return PutFrame( GetOutputConnector( MPPDO_Measurement_Control ) , pCamCom );
}

void MPPT_Dispens::GrabImage()
{
  switch ( m_WorkingMode )
  {
    case MPPD_Side:
      //     ProgramExposureAndLightParameters( m_iSideExposure_us , m_iSideExposure_us , 10 ) ;
      break;
    case MPPD_Front:
      //     ProgramExposureAndLightParameters( m_iFrontExposure_us ,
      //       m_iFrontExposure_us , m_iFrontExposure_us ) ;
      break;

  }
  double dNow = GetHRTickCount();
  double dTimeAfterLastOrder = dNow - m_dLastGrabOrderTime;
  if ( dTimeAfterLastOrder < 16.66 )
    Sleep( ROUND( 16.66 - dTimeAfterLastOrder ) + 1 );
  m_dLastGrabOrderTime = GetHRTickCount();
  //   FXString GrabCommand;
  //   GrabCommand.Format( "set FireSoftTrigger(%d);" , m_iFrameCount );
  //   ProgramCamera( GrabCommand , "GetNextImage" );
  ProgramCamera( "1" , "GetNextImage" );
  SetWatchDog( 3000 );
}

bool MPPT_Dispens::SendMessageToEngine( LPCTSTR pMessage , LPCTSTR pLabel )
{
  CTextFrame * pResult = CreateTextFrame( pMessage , pLabel );
  bool bRes = PutFrame( GetOutputConnector( MPPDO_DataOut ) , pResult );
  LPCTSTR pFound = _tcsstr( pMessage , _T( "Result=" ) );

  FXString ForLog;
  int iCnt = -1;
  if ( pFound )
  {
    if ( _tcsstr( pMessage , _T( "Blank" ) ) )
      iCnt = m_iNProcessedParts;
    ;
  }
  ForLog.Format( "To Engine: Lab=%s Msg=%s Blank#=%d" , pLabel , pMessage , iCnt );
  SaveLogMsg( ( LPCTSTR ) ForLog );
  return bRes;
}

bool MPPT_Dispens::SendOKToEngine( LPCTSTR pNote )
{
  FXString FullMessage = _T( "OK" );
  if ( pNote )
    FullMessage += pNote;

  return SendMessageToEngine( FullMessage , "OKforEngine" );
}

int MPPT_Dispens::ResetIterations()
{
  m_CommandTimeStamp = GetTimeAsString_ms();
  ClearInputQueue( false ); // remove from input queue frames
                     // only single frames will be left in the queue
  return 0;
}

// Adjust object placement in imaging
bool MPPT_Dispens::SetObjectPlacement( LPCTSTR pVideoObjectName ,
  CRect&  ROIPosition )
{
  FXString ROIData;
  ROIData.Format( "name=%s;xoffset=%d;yoffset=%d;width=%d;height=%d;" ,
    pVideoObjectName ,
    ROIPosition.left , ROIPosition.top ,
    ROIPosition.right , ROIPosition.bottom ); // right is width, bottom is height
  return SetParametersToTVObject( ROIData );
}
// Adjust object parameters in imaging
bool MPPT_Dispens::SetObjectPlacementAndSize( LPCTSTR pVideoObjectName ,
  CSize& ObjSizeMin , CSize& ObjSizeMax ,
  CRect * pROIPosition )
{
  CSize Area( ROUND( M_PI * ObjSizeMin.cx * ObjSizeMin.cy / 4. ) ,
    ObjSizeMax.cx * ObjSizeMax.cy );
  bool bRes = SetObjectSize( pVideoObjectName , ObjSizeMin , ObjSizeMax , Area );
  if ( bRes && pROIPosition )
    bRes = SetObjectPlacement( pVideoObjectName , *pROIPosition );
  return bRes;
}
// Adjust object parameters in imaging
bool MPPT_Dispens::SetObjectSize( LPCTSTR pVideoObjectName ,
  CSize& ObjSizeMin , CSize& ObjSizeMax ,
  CSize& AreaMinMax_pix )
{
  FXString ObjectSize;
  ObjectSize.Format( "name=%s;width_min=%d;height_min=%d;"
    "width_max=%d;height_max=%d;area_min=%d;area_max=%d;" ,
    pVideoObjectName ,
    ObjSizeMin.cx , ObjSizeMin.cy , ObjSizeMax.cx , ObjSizeMax.cy ,
    AreaMinMax_pix.cx , AreaMinMax_pix.cy );
  bool bRes = SetParametersToTVObject( ObjectSize );
  return bRes;
}

BOOL MPPT_Dispens::SetROIForCamera( CRect ROI )
{
  FXString CameraCommand;
  // right content is ROI width, bottom content is ROI height
  CameraCommand.Format( "set properties(ROI=%d,%d,%d,%d;);" ,
    ROI.left & ~0x03 , ROI.top & ~0x03 , ROI.right & ~0x03 , ROI.bottom & ~0x03 );
  m_CameraROI = ROI;
  return CameraControl( CameraCommand );
}

// Adjust offset camera ROI
void MPPT_Dispens::OffsetCameraROI( int iXOffset , int iYOffset )
{
  FXRegistry Reg( "TheFileX\\MPP_Dispens" );
  cmplx cMaxImageSize_pix;
  Reg.GetRegiCmplx( "Measurements" , "MaxImageSize" ,
    cMaxImageSize_pix , cmplx( 1600. , 1200. ) );

  CRect NewCameraROI( m_CameraROI ) ;
  if ( iXOffset + NewCameraROI.left < 0 )
    iXOffset = -NewCameraROI.left ;
  if ( iYOffset + NewCameraROI.top < 0 )
    iYOffset = -NewCameraROI.top ;
  if ( iXOffset + NewCameraROI.right >= cMaxImageSize_pix.real() )
    iXOffset = ROUND( cMaxImageSize_pix.real() - NewCameraROI.right ) ;
  if ( iYOffset + NewCameraROI.bottom >= cMaxImageSize_pix.imag() )
    iYOffset = ROUND( cMaxImageSize_pix.imag() - NewCameraROI.bottom ) ;

  NewCameraROI.OffsetRect( iXOffset , iYOffset ) ;
  SetROIForCamera( NewCameraROI ) ;
}

bool MPPT_Dispens::SetParametersToTVObject( LPCTSTR pParams )
{
  CTextFrame * pCommand = CreateTextFrame( pParams ,
    "SetObjectProp" );
  return PutFrame( GetOutputConnector( MPPDO_Measurement_Control ) , pCommand );
}

void MPPT_Dispens::SetImagingParameters( bool bByPart , CRect * pROI )
{
  if (m_WorkingMode != MPPD_Front)
    return;
  if ( pROI )
  {
    SetObjectPlacement( _T( "circle_int" ) , *pROI );
    SetObjectPlacement( _T( "back_light" ) , *pROI );
  }
  else
  {
    cmplx cPartSize( m_CurrentPart.m_dODmax_um , m_CurrentPart.m_dODmax_um );
    cmplx cPartSize_pix = cPartSize / m_dScale_um_per_pix;
    cmplx cROI_pix = cPartSize_pix * 1.3;
    FXRegistry Reg( "TheFileX\\MPP_Dispens" );
    cmplx cMaxImageSize_pix;
    Reg.GetRegiCmplx( "Measurements" , "MaxImageSize" ,
      cMaxImageSize_pix , cmplx( 1600. , 1200. ) );

    if ( cROI_pix.real() > cMaxImageSize_pix.imag() )
      cROI_pix = cmplx( cMaxImageSize_pix.imag() , cMaxImageSize_pix.imag() );
    CSize ViewArea_pix( ROUND( cROI_pix.real() ) & ~0x03 , ROUND( cROI_pix.real() ) & ~0x03 );
    int iOffsetX = (( ROUND( cMaxImageSize_pix.real() )- ViewArea_pix.cx ) / 2 ) & ~0x03;
    int iOffsetY = (( ROUND( cMaxImageSize_pix.imag() ) - ViewArea_pix.cy ) / 2 ) & ~0x03;
    CRect CameraROI( iOffsetX , iOffsetY , ViewArea_pix.cx , ViewArea_pix.cy );
    SetROIForCamera( CameraROI );

    int iBlackHoleMinSize_pix = ROUND( m_CurrentPart.m_dBlankIDmin_um * 0.8 / m_dInternalBlackScale_um_per_pixel );
    int iBlackHoleMaxSize_pix = ROUND( m_CurrentPart.m_dIDmax_um * 1.1 / m_dInternalBlackScale_um_per_pixel );
    int iBlackHoleAreaMin_pix = ROUND( M_PI * iBlackHoleMinSize_pix  * iBlackHoleMinSize_pix / 4.0 );
    int iBlackHoleAreaMax_pix = ROUND( M_PI * iBlackHoleMaxSize_pix  * iBlackHoleMaxSize_pix / 4.0 );

    CSize BlackMins( iBlackHoleMinSize_pix , iBlackHoleMinSize_pix );
    CSize BlackMaxs( iBlackHoleMaxSize_pix  , iBlackHoleMaxSize_pix );
    CSize BlackAreas( iBlackHoleAreaMin_pix , iBlackHoleAreaMax_pix );

    int iWhiteHoleMinSize_pix = ROUND( m_CurrentPart.m_dBlankIDmin_um * 0.8 / m_dInternalWhiteScale_um_per_pixel );
    int iWhiteHoleMaxSize_pix = ROUND( m_CurrentPart.m_dIDmax_um * 1.3 / m_dInternalWhiteScale_um_per_pixel );
    int iWhiteHoleAreaMin_pix = ROUND( M_PI * iWhiteHoleMinSize_pix  * iWhiteHoleMinSize_pix / 4.0 );
    int iWhiteHoleAreaMax_pix = ROUND( M_PI * iWhiteHoleMaxSize_pix  * iWhiteHoleMaxSize_pix / 4.0 );

    CSize WhiteMins( iWhiteHoleMinSize_pix , iWhiteHoleMinSize_pix );
    CSize WhiteMaxs( iWhiteHoleMaxSize_pix , iWhiteHoleMaxSize_pix );
    CSize WhiteAreas( iWhiteHoleAreaMin_pix , iWhiteHoleAreaMax_pix );

    if ( !bByPart )
    {

      m_cMainCrossCenter = cmplx( ( double ) ViewArea_pix.cx , ( double ) ViewArea_pix.cy )/2. ;
      CRect SearchArea( 
        ROUND( m_cMainCrossCenter.real() - iWhiteHoleMaxSize_pix * 0.6 ) , 
        ROUND( m_cMainCrossCenter.imag() - iWhiteHoleMaxSize_pix * 0.6 ) ,
        ROUND( iWhiteHoleMaxSize_pix * 1.2 ) ,  // right is width, 
        ROUND( iWhiteHoleMaxSize_pix * 1.2 ) ); // bottom is height
      if ( SearchArea.left + SearchArea.right > ViewArea_pix.cx - 20 )
      {
        SearchArea.left = 10 ;
        SearchArea.right = ViewArea_pix.cx - 20 ;
      }
      if ( SearchArea.top + SearchArea.bottom > ViewArea_pix.cy - 20 )
      {
        SearchArea.top = 10 ;
        SearchArea.bottom = ViewArea_pix.cx - 20 ;
      }
      SetObjectPlacement( _T( "circle_int" ) , SearchArea );
      SetObjectPlacement( _T( "back_light" ) , SearchArea );

    }
    else
    {
      int iRadius_pix = ROUND( m_CurrentPart.m_dIDmax_um * 1.2 / m_dScale_um_per_pix );
      if ( iRadius_pix > m_cMainCrossCenter.real() - 10. )
        iRadius_pix = ROUND(m_cMainCrossCenter.real() - 10.) ;
      CRect ForKnownPos( iRadius_pix , iRadius_pix ,
        2 * iRadius_pix , 2 * iRadius_pix ); // right is width, bottom is height
      SetObjectPlacement( _T( "circle_int" ) , ForKnownPos );
      SetObjectPlacement( _T( "back_light" ) , ForKnownPos );
    }
    SetObjectSize( _T( "circle_int" ) , BlackMins , BlackMaxs , BlackAreas  );
    SetObjectSize( _T( "back_light" ) , WhiteMins , WhiteMaxs , WhiteAreas );
  }
}
int MPPT_Dispens::GetVideoIntensityRange( const CVideoFrame * pVF ,
  int& iMin , int& iMax , int iStep ) 
{
  iMin = INT_MAX ;
  iMax = -INT_MAX ;
  LPVOID pImage = GetData( pVF ) ;

  PlanarSize PixSize = GetPlanarPixelSize( pVF ) ;
  if ( PixSize == PSize_8Bits )
  {
    LPBYTE pImage = GetData( pVF ) ;
    size_t iImageLen = GetImageSize( pVF ) ;
    LPBYTE pEnd = pImage + iImageLen ;
    while ( pImage < pEnd )
    {
      int iVal = *pImage ;
      SetMinMax( iVal , iMin , iMax ) ;
      pImage += iStep ;
    }
    return iMax - iMin ;
  }
  else if ( PixSize = PSize_16Bits )
  {
    LPWORD pImage = GetData16( pVF ) ;
    size_t iImageLen = GetImageSize( pVF ) ;
    LPWORD pEnd = pImage + iImageLen ;
    while ( pImage < pEnd )
    {
      int iVal = *pImage ;
      SetMinMax( iVal , iMin , iMax ) ;
      pImage += iStep ;
    }
    return iMax - iMin ;
  }
  return -1 ;
}
bool MPPT_Dispens::ExtractCirclesForCalibration( 
  const CVideoFrame * pVF , CContainerFrame * pMarking )
{
  m_bWhiteWasUsed = ( m_bUseBackLight && !m_bUseFrontLight );
  m_iExtractedForCalibration = 0;
  CmplxVector Pts;
  double dIntRadius_um = m_dCalibIDia_um / 2.;
  double dExtRadius_um = m_dCalibODia_um / 2.;
  
  double dRadius_pix = dIntRadius_um / m_dInternalBlackScale_um_per_pixel  ;
  double dInitialRadius = dRadius_pix < 30 ? 5. : dRadius_pix - 25.; // begin from inside
  bool bResInt = ExtractCirclesByCenterAndRadius(
    pVF , m_cLastIntWhiteCenter_pix , dInitialRadius , dRadius_pix + 10. ,
    0.5 , CEM_Extrem , DTM_PositiveOnly , Pts ,
    ( m_iViewMode > 5 ) ? pMarking : NULL );
  if ( bResInt )
  {
    m_dLastDI_pix = dRadius_pix * 2.;
    m_dLastDI_um = m_dLastWhiteDI_um = m_dLastBlackDI_um = m_dCalibIDia_um;
    bResInt = CircleFitting( Pts.data() , ( int ) Pts.size() , m_cLastIntCenter_pix , m_dLastDI_pix );
    if ( bResInt )
    {
      double dIntRadius_pix = m_dLastDI_pix * 0.5;
      for ( size_t i = 0; i < Pts.size(); i++ )
      {
        double dDistToCenter = abs( m_cLastIntCenter_pix - Pts[ i ] );
        double dDist = fabs( dDistToCenter - dIntRadius_pix );
        if ( dDist > m_dMaxPtDeviation_pix )
          Pts.erase( Pts.begin() + i-- );
        else
        {
          if ( ( m_iViewMode >= 8 ) && pMarking )
            pMarking->AddFrame( CreatePtFrame( Pts[ i ] , "color=0xffff00;Sz=2;" ) );
        }
      }
      bResInt = Pts.size() > 5 ;
      if ( bResInt )
      {
        bResInt = CircleFitting( Pts.data() ,
          ( int ) Pts.size() , m_cLastIntCenter_pix , m_dLastDI_pix );
        if ( bResInt )
        {
          pMarking->AddFrame( CreateCircleView(
            m_cLastIntCenter_pix , m_dLastDI_pix * 0.5 , 0xff00ff ) );
//           dIntRadius_pix = m_dLastDI_pix / 2.0;
//           m_dLastInternalDia_um = m_dLastDI_pix * m_dInternalBlackScale_um_per_pixel;
//          dIntRadius_um = m_dLastInternalDia_um / 2.0;
          double dMaxRadius = 0. , dMinRadius = DBL_MAX;
          for ( size_t i = 0; i < Pts.size(); i++ )
          {
            double dDistToCenter = abs( m_cLastIntCenter_pix - Pts[ i ] );
            double dDist = fabs( dDistToCenter - dIntRadius_pix );
            SetMinMax( dDistToCenter , dMinRadius , dMaxRadius );
          }
          GetMinMaxDia( Pts.data() , ( int ) Pts.size() ,
               m_bUseFrontLight ? m_dLastMinBlackDI_pix : m_dLastMinWhiteDI_pix ,
               m_bUseFrontLight ? m_dLastMaxBlackDI_pix : m_dLastMaxWhiteDI_pix ,
            m_CurrentPart.m_iDiameterAverage );
          if ( m_bUseFrontLight )
            m_dLastBlackDI_pix = m_dLastDI_pix;
          else
            m_dLastWhiteDI_pix = m_dLastDI_pix;
        }
      }
    }
  }
  dRadius_pix = dExtRadius_um / m_dExternalScale_um_per_pixel;
  Pts.clear();
  m_cLastExtCenter_pix = m_cLastIntCenter_pix;
  bool bResExt = ExtractCirclesByCenterAndRadius(
    pVF ,
    m_cLastExtCenter_pix ,
    dRadius_pix - 40. , // begin from inside
    dRadius_pix + 40. ,
    m_dImagingDiffThreshold , CEM_FirstDiff , DTM_NegativeOnly ,
    Pts , ( m_iViewMode > 7 ) ? pMarking : NULL );
  if ( bResExt )
  {
    m_dLastExternalDia_pix = dRadius_pix * 2.;
    m_dLastExternalDia_um = m_dCalibIDia_um;
    bResExt = CircleFitting( Pts.data() , ( int ) Pts.size() , m_cLastExtCenter_pix , m_dLastExternalDia_pix );
    if ( bResExt )
    {
      double dExtRadius_pix = m_dLastExternalDia_pix * 0.5;
      for ( size_t i = 0; i < Pts.size(); i++ )
      {
        double dDistToCenter = abs( m_cLastExtCenter_pix - Pts[ i ] );
        double dDist = fabs( dDistToCenter - dExtRadius_pix );
        if ( dDist > m_dMaxPtDeviation_pix * 2.5 )
          Pts.erase( Pts.begin() + i-- );
        else
        {
          if ( ( m_iViewMode >= 8 ) && pMarking )
            pMarking->AddFrame( CreatePtFrame( Pts[ i ] , "color=0xffff00;Sz=2;" ) );
        }
      }
      bResExt = Pts.size() > 5;
      if ( bResExt )
      {
        bResExt = CircleFitting( Pts.data() ,
          ( int ) Pts.size() , m_cLastExtCenter_pix , m_dLastExternalDia_pix );
        if ( bResExt )
        {
          dExtRadius_pix = m_dLastExternalDia_pix / 2.0;
          pMarking->AddFrame( CreateCircleView(
            m_cLastExtCenter_pix , dExtRadius_pix , 0xff00ff ) );
          m_dLastExternalDia_um = m_dLastExternalDia_pix * m_dExternalScale_um_per_pixel;
          dExtRadius_um = m_dLastExternalDia_um / 2.0;
          double dMaxRadius = 0. , dMinRadius = DBL_MAX;
          for ( size_t i = 0; i < Pts.size(); i++ )
          {
            double dDistToCenter = abs( m_cLastExtCenter_pix - Pts[ i ] );
            double dDist = fabs( dDistToCenter - dExtRadius_um );
            SetMinMax( dDistToCenter , dMinRadius , dMaxRadius );
          }
          GetMinMaxDia( Pts.data() , ( int ) Pts.size() ,
            m_dLastMinExternalDia_pix , m_dLastMaxExternalDia_pix , m_CurrentPart.m_iDiameterAverage );
        }
      }
    }
  }
//   double dTimeFromScaling = GetHRTickCount() - m_dLastScalingTime;
//   if ( (900. < dTimeFromScaling)  && (dTimeFromScaling < 1200.) )
//   {
//     FXRegistry Reg( "TheFileX\\MPP_Dispens" );
//     double dFrontScaleMin = Reg.GetRegiDouble( "Calibrations" , "MinFrontScale_um_per_pixel" , 0.78 );
//     double dFrontScaleMax = Reg.GetRegiDouble( "Calibrations" , "MaxFrontScale_um_per_pixel" , 0.89 );
//     if ( bResInt )
//     {
//       double dScale = m_dCalibIDia_um / ( m_dLastDI_pix );
//       if ( IsInRange( dScale , dFrontScaleMin , dFrontScaleMax ) )
//         m_dInternalBlackScale_um_per_pixel = dScale;
//     }
//     if ( bResExt )
//     {
//       double dScale = m_dCalibODia_um / ( m_dLastExternalDia_pix );
//       if ( IsInRange( dScale , dFrontScaleMin , dFrontScaleMax ) )
//         m_dExternalScale_um_per_pixel = dScale;
//     }
//     if ( bResExt || bResInt )
//       SaveScales();
//   }
  m_iExtractedForCalibration = (bResInt ? 1 : 0 )+ ( bResExt ? 2 : 0 );
  return bResExt && bResInt ;
}