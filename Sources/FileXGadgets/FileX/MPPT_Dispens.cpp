#include "stdafx.h"
#include "..\..\gadgets\common\imageproc\clusters\Segmentation.h"
#include "..\..\gadgets\common\imageproc\seekspots.h"
#include "..\..\gadgets\common\imageproc\utilities.h"
#include "math/PlaneCircle.h"
#include "MPPT_Dispens.h"
#include "helpers/FramesHelper.h"


#define THIS_MODULENAME "MPPT_Dispens"

USER_FILTER_RUNTIME_GADGET(MPPT_Dispens, "Video.recognition");

static int iNFramesForThrowing = 4;

VOID CALLBACK MPPT_DispensWDTimerRoutine(LPVOID lpParam, BOOLEAN TimerOrWaitFired)
{
  MPPT_Dispens * pGadget = (MPPT_Dispens*)lpParam;
  //   if ( pGadget->m_pStatus->GetStatus() == CExecutionStatus::RUN )
  //   {
  CTextFrame * pTimeoutNotification = CreateTextFrame("Timeout", "Timeout");
  try
  {
    pGadget->GetInputConnector(0)->Send(pTimeoutNotification);
    pGadget->DeleteWatchDog();
    return;
  }
  catch (CMemoryException* e)
  {
    TCHAR Msg[2000];
    e->GetErrorMessage(Msg, 2000);
    TRACE("\n MPPT_DispensWDTimerRoutine memory exception %s", Msg);
  }
  catch (CException* e)
  {
    TCHAR Msg[2000];
    e->GetErrorMessage(Msg, 2000);
    TRACE("\n MPPT_DispensWDTimerRoutine exception %s", Msg);
  }
  pTimeoutNotification->Release();
}


NameIDPair NamesAndIDs[] =
{
  { "back_light" , WDFF_WhiteContur } ,
{ "blank_ext" , WDFF_ConeEdge } ,
{ "circle_ext" , WDFF_ConeEdge } ,
{ "blank_int" , WDFF_HoleEdge } ,
{ "circle_int" , WDFF_HoleEdge } ,
{ "side_view" , WDFF_SideEdge } ,
{ "side_stone" , WDFF_SideStone } ,
{ "ako_tek" , WDFF_AkoTek } ,
{ "" }
};

// Common for all gadgets properties
FXString MPPT_Dispens::m_PartName;               // It should be the same on all gadgets
Dispenser  MPPT_Dispens::m_CurrentPart;
Dispensers MPPT_Dispens::m_KnownParts;
DispenserProcessingResults MPPT_Dispens::m_LastResults;
FXLockObject MPPT_Dispens::m_MPPD_Lock;
double MPPT_Dispens::m_dCalibODia_um = 221.;
double MPPT_Dispens::m_dDistFromLastVertEdgeToMasterVertEdge_um = 0.;
double MPPT_Dispens::m_dFrontScaleCorrectionCoeff = 1.;
double MPPT_Dispens::m_dFrontLensWorkingDistance_um = 18000.;
bool MPPT_Dispens::m_bReadyToGrind = false;
double MPPT_Dispens::m_dLastFrontDistaceToStone_um = 0.;

MPPT_Dispens::MPPT_Dispens()
  : m_Centering(CNTST_Idle)
  , m_dRotationPeriodTolerance(0.01)
  , m_dCenteringTolerance_pix(1.)
  , m_dYMotionDist_pix(0.)
  , m_dXMotionDist_pix(0.)
  , m_dLastSyncTime_ms(0.)
  , m_dLastCalculatedRotationPeriod_ms(0.)
  , m_bIsSynchronized(false)
  , m_bIsBlackMeasurement(false)
  , m_dAccumulatedX(0.)
  , m_dAccumulatedY(0.)
  , m_iCurrentLightMask(0)
  , m_iViewMode(6)
  , m_ImagingMode(MDI_SimpleConturs)
  , m_dMaxPtDeviation_pix(1.0)
  , m_dInternalCorrection_pix(0.5)
  , m_dExternalCorrection_pix(1.5)
  , m_dImagingDiffThreshold(0.5)
  , m_iFrameCount(0)
  , m_iSaveDecimator(10)
  , m_iAfterCommandSaved(0)
  , m_iNProcessedParts(0)
  , m_WorkingMode(MPPD_Front)
  , m_WorkingStage(STG_Idle)
  , m_CircExtractMode(CEM_FirstEdge)
  , m_dInternalBlackScale_um_per_pixel(1.0)
  , m_dInternalWhiteScale_um_per_pixel(1.0)
  , m_dExternalScale_um_per_pixel(1.0)
  , m_dLastExternalDia_pix(0.)
  , m_ContinueRect(0, 0, 0, 0)
  , m_FinishRect(0, 0, 0, 0)
{
  m_OutputMode = modeReplace;
  m_dScale_um_per_pix = 0.0;
  LightOff(0xffff); // switch off all lights
  init();

  GetInputConnector(0)->SetQueueSize(10);
  FXRegistry Reg("TheFileX\\MPP_Dispens");
  Reg.GetRegiCmplx("Calibrations",
    "cXPlusScale_pix_per_second", m_cXScalePlus_pix_per_sec, cmplx(1., 0));
  Reg.GetRegiCmplx("Calibrations",
    "cXMinusScale_pix_per_second", m_cXScaleMinus_pix_per_sec, cmplx(-1., 0.));
  Reg.GetRegiCmplx("Calibrations",
    "cYPlusScale_pix_per_second", m_cYScalePlus_pix_per_sec, cmplx(0., 1.));
  Reg.GetRegiCmplx("Calibrations",
    "cYMinusScale_pix_per_second", m_cYScaleMinus_pix_per_sec, cmplx(0., -1.));
  m_dLastCalculatedRotationPeriod_ms = Reg.GetRegiDouble(
    "Calibrations", "MainRotationPeriod_ms", 113.7223);
  m_dInternalBlackScale_um_per_pixel = Reg.GetRegiDouble(
    "Calibrations", "FrontInternalScale_um_per_pixel", m_dInternalBlackScale_um_per_pixel);
  m_dInternalWhiteScale_um_per_pixel = Reg.GetRegiDouble(
    "Calibrations", "FrontInternalWhiteScale_um_per_pixel", m_dInternalWhiteScale_um_per_pixel);
  m_dExternalScale_um_per_pixel = Reg.GetRegiDouble(
    "Calibrations", "FrontExternalScale_um_per_pixel", m_dExternalScale_um_per_pixel);
  m_iAveragingC = Reg.GetRegiInt("Measurements", "PointsAveraging", 10);
  m_iNFramesForThrowing = iNFramesForThrowing = Reg.GetRegiInt("Measurements", "NotUsedFirstFrames", 4);
  m_PartName = Reg.GetRegiString("PartData", "LastName", "");

  Reg.GetRegiCmplx("Calibrations",
    "CalibSideUpperRightCorner", m_cCalibSideUpperRightCorner, m_cCalibSideUpperRightCorner);
  Reg.GetRegiCmplx("Calibrations",
    "CalibSideLowerRightCorner", m_cCalibSideLowerRightCorner, m_cCalibSideLowerRightCorner);
  Reg.GetRegiCmplx("Calibrations",
    "CalibSideUpperLeftCorner", m_cCalibSideUpperLeftCorner, m_cCalibSideUpperLeftCorner);
  Reg.GetRegiCmplx("Calibrations",
    "CalibSideLowerLeftCorner", m_cCalibSideLowerLeftCorner, m_cCalibSideLowerLeftCorner);
  Reg.GetRegiCmplx("Calibrations",
    "LeftTopStoneTargetMark", m_cLeftTopStoneTargetMark, m_cLeftTopStoneTargetMark);
  Reg.GetRegiCmplx("Calibrations",
    "RightTopStoneTargetMark", m_cRightTopStoneTargetMark, m_cRightTopStoneTargetMark);
  m_dStoneToMasterDistance_um = Reg.GetRegiDouble(
    "Calibrations", "StoneToEdgeDist_um", 100.);
  m_LastPartUpperSideLine.Update(m_cCalibSideUpperLeftCorner, m_cCalibSideUpperRightCorner);
  RestoreKnownParts(m_PartName);
  m_CurrentLogFilePath = CheckCreateCurrentLogs();
  SaveLogMsg("\nLog Initialized at %s\n",
    (LPCTSTR)GetTimeAsString_ms());
  SaveCSVLogMsg("\nLog Initialized at %s\n",
    (LPCTSTR)GetTimeAsString_ms());
  //   m_MPPD_Lock.Lock() ;
  //   m_ExistentGadgets.push_back( this ) ;
  //   m_MPPD_Lock.Unlock() ;
};


void MPPT_Dispens::ShutDown()
{
  //   m_MPPD_Lock.Lock() ;
  //   for ( size_t i = 0 ; i < m_ExistentGadgets.size() ; i++ )
  //   {
  //     if ( m_ExistentGadgets[i] == this )
  //     {
  //       m_ExistentGadgets.erase( m_ExistentGadgets.begin() + 1 ) ;
  //       break ;
  //     }
  //   }
  //   m_MPPD_Lock.Unlock() ;
};

void MPPT_Dispens::ConnectorsRegistration()
{
  addInputConnector(transparent, "InputForAll");
  addOutputConnector(transparent, "OutVideo");
  addOutputConnector(transparent, "DataOut");
  addOutputConnector(text, "CameraControl");
  addOutputConnector(text, "MeasurementControl");
};

CDataFrame * MPPT_Dispens::DoProcessing(const CDataFrame * pDataFrame)
{
  if (m_dScale_um_per_pix == 0.0)
  {
    GetGadgetName(m_GadgetName);

    FXRegistry Reg("TheFileX\\MPP_Dispens");
    m_dScale_um_per_pix = Reg.GetRegiDouble(
      "Calibrations", (m_WorkingMode == MPPD_Front) ?
      "FrontAverageScale_um_per_pixel" : "SideScale_um_per_pixel", m_dScale_um_per_pix);

    CTextFrame * pSetScaleToRender = CreateTextFrame("Scale&Units", pDataFrame->GetId());
    pSetScaleToRender->GetString().Format("%.7f,um", m_dScale_um_per_pix);
    PutFrame(GetOutputConnector(MPPDO_VideoOut), pSetScaleToRender);
    CTextFrame * pSetScaleToImaging = CreateTextFrame("Scale", pDataFrame->GetId());
    pSetScaleToImaging->GetString().Format("%.7f,um", m_dScale_um_per_pix);
    PutFrame(GetOutputConnector(MPPDO_Measurement_Control), pSetScaleToImaging);
  }

  if (!pDataFrame->IsContainer())
  { // there is not data for processing, it's some command or parameter
    datatype Type = pDataFrame->GetDataType();
    switch (Type)
    {
    case text:
      ProcessTextCommand((const CTextFrame*)pDataFrame);
      return NULL;
    }
    return NULL;
  }
  //   CDataFrame * pNextFrame = NULL ;
  //   int iIsAnotherVideoFrames = 0 ;
  //   // we take all frames, check for video frames availability
  //   // and put back into queue in the same order
  //   while ( GetInputConnector( 0 )->Get( pNextFrame ) )
  //   {
  //     if ( pNextFrame->IsContainer() && pNextFrame->GetVideoFrame() )
  //       iIsAnotherVideoFrames++ ;
  // 
  //     GetInputConnector( 0 )->GetInputPin()->Send( pNextFrame ) ;
  //   }
  //   if ( iIsAnotherVideoFrames ) // container with video frame is in queue
  //   {
  //     SEND_GADGET_INFO( "%d additional images in queue, image is discarded; %d frames in queue" ,
  //       iIsAnotherVideoFrames , GetInputConnector( 0 )->GetNFramesInQueue() ) ;
  //     return NULL ; // we don't need to process, there are another images in input queue
  //   }

  FXRegistry Reg("TheFileX\\MPP_Dispens");
  m_dStoneToMasterDistance_um = Reg.GetRegiDouble(
    "Calibrations", "StoneToEdgeDist_um", 100.);
  cmplx cNewNormCrossCenter;
  Reg.GetRegiCmplx("Measurements",
    (m_WorkingMode == MPPD_Front) ? "NormFrontMainCrossCenter" : "NormSideMainCrossCenter",
    cNewNormCrossCenter, cmplx(0.5, 0.5));
  const CVideoFrame * pVF = pDataFrame->GetVideoFrame();
  m_pLastVideoFrame = pVF;
  m_dLastCaptureTime_ms = 0.;
  CContainerFrame * pMarking = CContainerFrame::Create();
  bool bContursDataSettled = false;

  if (pVF)
  {
    if (m_WorkingMode == MPPD_Front && m_bHighBrightnessMode)
    {
      pMarking->AddFrame(CreateTextFrame(cmplx(m_cLastROICent_pix * 0.2),
        "High Brightness Mode", 0x00ffff, 14));
      pMarking->AddFrame(pDataFrame);
      return pMarking;
    }
    if (m_iCheckSideLight > 0)
    {
      if (--m_iCheckSideLight == 0)
      {
        int iMin, iMax;
        int iRange = GetVideoIntensityRange(pVF, iMin, iMax, 97);
        FXString MsgToEngine;
        MsgToEngine.Format("%s;//Min=%d Max=%d",
          (iMax > 100) ? "IsOn" : "IsOff", iMin, iMax);
        SendMessageToEngine(MsgToEngine, "AnswerForLightChecking");
      }
    }
    if (m_iBlankLengthCheck > 0)
    {
      if (--m_iBlankLengthCheck == 0)
      {
        m_dMaxTipDeviationFromMaster_um = Reg.GetRegiDouble(
          "Measurements", "MaxTipDeviationFromMaster_um", 70.);
        m_cMasterCenter_pix = 0.5 *(m_cCalibSideUpperRightCorner + m_cCalibSideLowerRightCorner);
        double dDist_pix = -1000.;
        double dDist_um = dDist_pix * m_dScale_um_per_pix;

        CColorSpot * pFrontDistData = GetSpotData("front_dist");
        if (pFrontDistData)
        {
          dDist_pix = pFrontDistData->m_dBlobWidth;
          dDist_um = dDist_pix * m_dScale_um_per_pix;
          m_dConesEdgePos_pix = pFrontDistData->m_SimpleCenter.x - dDist_pix / 2.;
          double dDistFromEdgeToMasterEdge_pix = m_dConesEdgePos_pix - m_cMasterCenter_pix.real();
          double dDistFromEdgeToMasterEdge_um = dDistFromEdgeToMasterEdge_pix * m_dScale_um_per_pix;
          bool bBlankLengthOK = (fabs(dDistFromEdgeToMasterEdge_um) <= m_dMaxTipDeviationFromMaster_um);
          FXString MsgToEngine;
          LPCTSTR Qualification = (bBlankLengthOK) ? "Good Length," :
            (dDistFromEdgeToMasterEdge_um > 0.) ? "Too Long" : "Too Short";
          MsgToEngine.Format("%s;// %s Blank Ldiff=%.2fum",
            bBlankLengthOK ? "BlankOk" : "Fail",
            Qualification, dDistFromEdgeToMasterEdge_um);
          SendMessageToEngine(MsgToEngine, "BlankLengthCheckResult");
        }
      }
    }
    int iNROIs = SetDataAboutROIs(pDataFrame);
    CFramesIterator * pFigIterator = pDataFrame->CreateFramesIterator(figure);
    m_LastFigures.clear();
    bContursDataSettled = (pFigIterator && SetDataAboutContours(pFigIterator));
    m_LastSpots.RemoveAll();
    ExtractDataAboutSpots(pDataFrame, m_LastSpots);

    if (!m_bDoMeasurement)
    {
      pMarking->AddFrame(CreateTextFrame(cmplx(100., m_cLastROICent_pix.imag()),
        "Measurements Disabled", 0x0000ff, 20));
      DrawStandardGraphics(pMarking);
      pMarking->AddFrame(pDataFrame);
      return pMarking;
    }

    bool bContinue = PreProcessVideoFrame(pMarking, pDataFrame, pVF);
    if (m_cNormMainCrossCenter != cNewNormCrossCenter)
    {
      m_cNormMainCrossCenter = cNewNormCrossCenter;
      m_cMainCrossCenter = 2. * cmplx(m_cLastROICent_pix.real() * m_cNormMainCrossCenter.real(),
        m_cLastROICent_pix.imag() * m_cNormMainCrossCenter.imag());
    }
    if (!bContinue)
    {
      if (m_iLastConstrast < m_dMinBrightnessAmplitude) // low contrast
      {
        pMarking->AddFrame(CreateTextFrame(cmplx(m_cLastROICent_pix) - m_cLastROICent_pix.real() / 2.,
          "0x0000ff", 24, "LowContrast", pDataFrame->GetId(), "LOW CONTRAST %d", m_iNLowContrastImages));
        DrawStandardGraphics(pMarking);
        if (++m_iNLowContrastImages >= m_iNMaxImagesWithLowContrast)
        {
          m_iNMaxImagesWithLowContrast = Reg.GetRegiInt("Measurements",
            (m_WorkingMode == MPPD_Front) ? "NMaxImagesWithLowContrastFront" : "NMaxImagesWithLowContrastSide",
            (m_WorkingMode == MPPD_Front) ? 30 : 30);
          m_dMinBrightnessAmplitude = Reg.GetRegiDouble("Measurements",
            (m_WorkingMode == MPPD_Front) ? "FrontMinAmplitude" : "SideMinAmplitude",
            (m_WorkingMode == MPPD_Front) ? 40. : 40.);

          if (IsProcessingStage())
          {
            if (++m_iFrameCntAfterDone > 5)
            {
              if (m_WorkingStage != STG_FrontPolishing)
              {
                FXString MsgToEngine;
                MsgToEngine.Format("Failed;// Low contrast %s", GetWorkingStateName());
                SendMessageToEngine(MsgToEngine, "Low contrast message");
                SetIdleStage();
              }
              else
              {
                TRACE("\nLow constrast on polishing");
              }
            }
          }
        }
      }
      pMarking->AddFrame(pDataFrame);
      return pMarking;
    }
  }

  if ( m_WorkingMode == Ako_Tek )
  {
    if ( bContursDataSettled
      && ( ( m_dwLastFilledConturs & WDFF_AkoTek ) != 0 ) )
    {
      ProcessAkoTekContour( pVF , pMarking );
    }
  }
  else if (m_WorkingMode == MPPD_Side)
  {
    if (bContursDataSettled
      && ((m_dwLastFilledConturs & (WDFF_SideEdge | WDFF_SideStone)) != 0))
    {
      ProcessSideContour(pMarking);
    }
    else // no contours, simple move side stone
    {
      switch (m_WorkingStage)
      {
      case STG_FineStoneMeas:
        if (m_bDone && ((GetHRTickCount() - m_dLastMotionFinishedTime) > 400.))
        {
          m_iLastStoneMovingTime_ms = Reg.GetRegiInt(
            "Motions", "FineLargeMotion_ms", 200);
          FXString LargeMsg;
          LargeMsg.Format("Large=%d;", m_iLastStoneMovingTime_ms);
          SendMessageToEngine(LargeMsg, NULL);
          m_bDone = false;
          m_dLastMotionFinishedTime = 0.;
        }
        break;
      case STG_CoarseStoneMeas:
        if (m_bDone && ((GetHRTickCount() - m_dLastMotionFinishedTime) > 1000.))
        {
          if ( pVF )
          {
            CRect CheckArea(m_LastROICent.x + 80, m_LastROICent.y - 80,
              m_LastROICent.x + 100, m_LastROICent.y + 100);
            double dAverBrightness = GetAverageForRect(pVF, CheckArea);
            ASSERT(dAverBrightness > 128);
          }
          m_iLastStoneMovingTime_ms = Reg.GetRegiInt(
            "Motions", "CoarseLargeMotion_ms", 1000);
          FXString LargeMsg;
          LargeMsg.Format("Large=%d;", m_iLastStoneMovingTime_ms);
          SendMessageToEngine(LargeMsg, NULL);
          m_bDone = false;
          m_dLastMotionFinishedTime = 0.;
        }
        break;
      }
    }
  }

  if (pVF)
  {
    DrawStandardGraphics(pMarking);
    m_iFrameCntAfterDone++;
    switch (m_WorkingMode)
    {
    case MPPD_Front:
      {
        int iRes = 0;
        bool bBreakFromFront = false;
        if (!m_bLastWhiteDIOK)
        {
          switch (m_WorkingStage)
          {
          case STG_FrontSynchronization:
            if (m_bMotorIsOn)
              m_iFrameCntAfterDone = 0;
            break;
          case STG_MoveXPlusCalibration:
          case STG_MoveXMinusCalibration:
          case STG_MoveYPlusCalibration:
          case STG_MoveYMinusCalibration:
          case STG_Get0DegImage:
          case STG_Get90DegImage:
          case STG_Get180DegImage:
          case STG_Get270DegImage:
            if (!(m_ForWhatSync == SS_SyncForCalibration))
              bBreakFromFront = true;
          }
          if (bBreakFromFront)
            break;
        }
        switch (m_WorkingStage)
        {
        case STG_FrontSynchronization:
          // m_dLastCalculatedRotationPeriod_ms is filled and "Done" received from Engine
//          if ( /*m_dLastSyncTime_ms != 0. &&*/ (m_iFrameCntAfterDone > 10) && m_bMotorIsOn)
          if ((m_iFrameCntAfterDone > 10) && m_bMotorIsOn)
          {
            m_cLastInitialPosition = m_cLastWhiteConturCenter_pix;
            m_bDone = true;

            FXRegistry Reg("TheFileX\\MPP_Dispens");
            FXString TS = Reg.GetRegiString("Calibrations",
              "LastCenteringScalesMeasTime", GetTimeStamp());
            TS = TS.Left(8);
            FXString Now = GetTimeStamp();
            if (TS != Now.Left(8))
            {
              if (PreProcessCircles(pMarking, pDataFrame, pVF))
              {
                iRes = ProcessImageForCentering(m_cLastInitialPosition,
                  m_cLastWhiteConturCenter_pix, "Do Move X+ for 1 second (to the left)",
                  "MoveX=1000;", STG_MoveXPlusCalibration, pMarking);
                pMarking->AddFrame(CreatePtFrame(m_cLastInitialPosition, "color=0xff;Sz=5;thickness=3;"));
                m_bDone = false;
              }
              else
                ASSERT(0);
            }
            else
            {
              GetCenteringParameters();
              SetGrabForCentering(STG_Get0DegImage);
            }
          };
          break;
        case STG_MoveXPlusCalibration:
          if ((m_iFrameCntAfterDone >= 2) && m_bDone)
          {
            m_cLastXMovedPlusPosition = m_cLastIntCenter_pix;
            iRes = ProcessImageForCentering(m_cLastXMovedPlusPosition,
              m_cLastIntCenter_pix, "Do Move X- for 1 second (to the right)",
              "MoveX=-1000;", STG_MoveXMinusCalibration, pMarking);
            pMarking->AddFrame(CreatePtFrame(m_cLastInitialPosition, "color=0xff;Sz=5;thickness=3;"));
            pMarking->AddFrame(CreateLineFrame(m_cLastInitialPosition,
              m_cLastXMovedPlusPosition, 0xff4000));
            m_bDone = false;
          }
          break;
        case STG_MoveXMinusCalibration:
          if ((m_iFrameCntAfterDone >= 2) && m_bDone)
          {
            m_cLastXReturnedPosition = m_cLastIntCenter_pix;
            iRes = ProcessImageForCentering(m_cLastXReturnedPosition,
              m_cLastIntCenter_pix, "Do Move Y+ for 1 second",
              "MoveY=1000;", STG_MoveYPlusCalibration, pMarking);
            pMarking->AddFrame(CreatePtFrame(m_cLastInitialPosition, "color=0xff;Sz=5;thickness=3;"));
            pMarking->AddFrame(CreateLineFrame(m_cLastInitialPosition,
              m_cLastXReturnedPosition, 0xff4000));
            m_bDone = false;
          }
          break;
        case STG_MoveYPlusCalibration:
          if ((m_iFrameCntAfterDone >= 2) && m_bDone)
          {
            m_cLastYMovedPlusPosition = m_cLastIntCenter_pix;
            iRes = ProcessImageForCentering(m_cLastYMovedPlusPosition,
              m_cLastIntCenter_pix, "Do Move Y- for 1 second",
              "MoveY=-1000;", STG_MoveYMinusCalibration, pMarking);
            pMarking->AddFrame(CreatePtFrame(m_cLastInitialPosition, "color=0xff;Sz=5;thickness=3;"));
            pMarking->AddFrame(CreateLineFrame(m_cLastInitialPosition,
              m_cLastYMovedPlusPosition, 0xff4000));
            m_bDone = false;
          }
          break;
        case STG_MoveYMinusCalibration: // Scales are measured, decentering also
          {
            if ((m_iFrameCntAfterDone >= 2) && m_bDone)
            {
              m_cLastYReturnedPosition = m_cLastIntCenter_pix;
              iRes = ProcessImageForCentering(m_cLastYReturnedPosition,
                m_cLastIntCenter_pix, "Scaling Measurement Finished",
                NULL, (MPPD_Stage)(-1), pMarking);
              pMarking->AddFrame(CreatePtFrame(m_cLastInitialPosition, "color=0xff;Sz=5;thickness=3;"));
              pMarking->AddFrame(CreateLineFrame(m_cLastInitialPosition,
                m_cLastYMovedPlusPosition, 0xff4000));

              CalculateAndSaveCenteringScales();
              m_bDone = false;
              switch (m_ForWhatSync)
              {
              default:
              case SS_SyncForCalibration:
                {
                  //                   SetIdleStage() ;
                  m_AdditionalMsgForManual = "Scaling for Centering is finished";
                }
                //                break ;
              case SS_SyncForProduction:
              case SS_SyncForCentering: // Init centering
                SetGrabForCentering(STG_Get0DegImage);
                break;
              }
            }
          }
          break;
        case STG_Get0DegImage:
          if ((m_iFrameCntAfterDone >= iNFramesForThrowing) && m_bDone)
          {
            m_cLastSpotCent_0Deg = m_cLastIntCenter_pix;
            SetGrabForCentering(m_cLastIntCenter_pix.real() == 0. ?
              STG_Get0DegImage : STG_Get90DegImage);

            pMarking->AddFrame(CreatePtFrame(m_cLastSpotCent_0Deg, "color=0xc0;Sz=5;thickness=3;"));
          }
          break;
        case STG_Get90DegImage:
          if (m_iFrameCntAfterDone >= iNFramesForThrowing)
          {
            if (m_cLastSpotCent_0Deg.real() == 0.)
              SetGrabForCentering(STG_Get0DegImage);
            else
            {
              m_cLastSpotCent_90Deg = m_cLastIntCenter_pix;
              SetGrabForCentering(m_cLastIntCenter_pix.real() == 0. ?
                STG_Get90DegImage : STG_Get180DegImage);
              pMarking->AddFrame(CreatePtFrame(m_cLastSpotCent_0Deg, "color=0xc0;Sz=5;thickness=3;"));
              pMarking->AddFrame(CreatePtFrame(m_cLastSpotCent_90Deg, "color=0xc000;Sz=5;thickness=3;"));
            }
          }
          break;
        case STG_Get180DegImage:
          if (m_iFrameCntAfterDone >= iNFramesForThrowing)
          {
            if (m_cLastSpotCent_90Deg.real() == 0.)
              SetGrabForCentering(STG_Get90DegImage);
            else
            {
              m_cLastSpotCent_180Deg = m_cLastIntCenter_pix;
              SetGrabForCentering(m_cLastIntCenter_pix.real() == 0. ?
                STG_Get180DegImage : STG_Get270DegImage);
              pMarking->AddFrame(CreatePtFrame(m_cLastSpotCent_0Deg, "color=0xc0;Sz=5;thickness=3;"));
              pMarking->AddFrame(CreatePtFrame(m_cLastSpotCent_90Deg, "color=0xc000;Sz=5;thickness=3;"));
              pMarking->AddFrame(CreatePtFrame(m_cLastSpotCent_180Deg, "color=0xc00000;Sz=5;thickness=3;"));
            }
          }
          break;
        case STG_Get270DegImage:
          {
            if (m_iFrameCntAfterDone >= iNFramesForThrowing)
            {
              if (m_cLastSpotCent_180Deg.real() == 0.)
                SetGrabForCentering(STG_Get90DegImage);
              else
              {
                m_cLastSpotCent_270Deg = m_cLastIntCenter_pix;
                if (m_cLastSpotCent_270Deg.real() == 0.)
                  SetGrabForCentering(STG_Get270DegImage);
                else
                {
                  cmplx cCenter = (m_cLastSpotCent_0Deg + m_cLastSpotCent_180Deg
                    + m_cLastSpotCent_90Deg + m_cLastSpotCent_270Deg) / 4.;

                  cmplx cDeviation = m_cLastSpotCent_0Deg - cCenter;
                  m_dLastXEccentricitet_um = cDeviation.real() * m_dScale_um_per_pix;
                  m_dLastYEccentricitet_um = cDeviation.imag() * m_dScale_um_per_pix;
                  bool bFinished = false;
                  pMarking->AddFrame(CreatePtFrame(m_cLastSpotCent_0Deg, "color=0xc0;Sz=5;"));
                  pMarking->AddFrame(CreatePtFrame(m_cLastSpotCent_90Deg, "color=0xc000;Sz=5;"));
                  pMarking->AddFrame(CreatePtFrame(m_cLastSpotCent_180Deg, "color=0xc00000;Sz=5;"));
                  pMarking->AddFrame(CreatePtFrame(m_cLastSpotCent_270Deg, "color=0xc000c0;Sz=5;"));
                  pMarking->AddFrame(CreatePtFrame(cCenter, "color=0x008080;Sz=5;"));
                  if ((fabs(cDeviation.real()) <= m_dCenteringTolerance_pix / 2.)
                    && (fabs(cDeviation.imag()) <= m_dCenteringTolerance_pix / 2.))
                  {
                    m_AdditionalInfo3.Format("Centering Finished Ecc=(%.2f,%.2f) um",
                      m_dLastXEccentricitet_um, m_dLastYEccentricitet_um);
                    cmplx cResultView = cmplx(10., m_cLastROICent_pix.imag() * 0.75);
                    pMarking->AddFrame(CreateTextFrame(cResultView, "0x00ff00", 24,
                      "CenteringResult", pDataFrame->GetId(), m_AdditionalInfo3));
                    bFinished = true;
                    SetGrabForCentering(STG_Get0DegImageForAverage);
                    m_cAccumulator.clear();
                    m_cLastCenter_pix = cCenter;
                    m_Centering = CNTST_Idle;
                  }
                  else
                  {
                    SetCameraTriggerParams(true, 0.);
                    SetExposure(m_CurrentPart.m_iFrontForWhiteExposure_us);
                    double dXMotion_pix = cDeviation.real();
                    //                    double dYMotion_pix = -cDeviation.imag(); // Sign changed 2023.04.04 Moisey
                    double dYMotion_pix = cDeviation.imag();
                    m_dXMotionDist_pix = dXMotion_pix;
                    DoAdjustForCentering(CM_MoveX, m_dXMotionDist_pix);
                    m_WorkingStage = STG_CorrectX;
                    m_dYMotionDist_pix = dYMotion_pix;
                    m_bDone = false;
                    m_AdditionalMsgForManual.Format("Cent. Meas Finished\n"
                      "MoveX=%.1f pix, MoveY=%.1f pix\n"
                      "cDev=(%.2f,%.2f)",
                      dXMotion_pix, dYMotion_pix,
                      cDeviation.real(), cDeviation.imag());
                    cmplx cResultView = cmplx(10., m_cLastROICent_pix.imag() * 0.75);
                    pMarking->AddFrame(CreateTextFrame(cResultView, "0x00ff00", 14,
                      "CentMeasResult", pDataFrame->GetId(), m_AdditionalMsgForManual));
                  }
                }
              }
            }
          }
          break;
        case STG_Get0DegImageForAverage:
          {
            if (++m_iFrameCntAfterDone > 2)
            {
              m_cAccumulator.push_back(m_cLastIntWhiteCenter_pix);
              if (m_cAccumulator.size() >= 12)
              {
                cmplx cAverage, cMins(DBL_MAX, DBL_MAX), cMaxes(-DBL_MAX, -DBL_MAX);
                for (auto It = m_cAccumulator.begin() + 2; It < m_cAccumulator.end(); It++)
                {
                  cAverage += *It;
                  CmplxSetMinMax(*It, cMins, cMaxes);
                }

                cAverage /= (double)(m_cAccumulator.size() - 2);
                m_cAverageCenterFor0Deg = cAverage;
                m_cLastCenterMins = cMins;
                m_cLastCenterMaxes = cMaxes;

                //                 m_WorkingStage = STG_WaitForMotorStop;
                //                 StopMotor();

                FXString MsgForEngine;
                SetIdleStage();
                cmplx cResultView = cmplx(10., m_cLastROICent_pix.imag() * 0.75);
                // If it's first centering, check max white hole diameter
                if (m_LastResults.m_dBlankInitialDI_um == 0.)
                {
                  m_LastResults.m_dBlankInitialDI_um = m_dLastWhiteDI_um;
                  m_LastResults.m_dBlankInitialMinDI_um = m_dLastMinWhiteDI_um;
                  m_LastResults.m_dBlankInitialMaxDI_um = m_dLastMaxWhiteDI_um;
                  if (m_dInternalWhiteScale_um_per_pixel != 1.0) // first time calibration owith computer
                  {
                    if (m_bCheckAsBlank)
                    {
                      if ((m_LastResults.m_dBlankInitialMinDI_um < m_CurrentPart.m_dBlankIDmin_um - 5.))
                      {
                        MsgForEngine.Format("Failed; "
                          "// Too small blank diameter %.2f(%.2f,%.2f);Ecc=%.2f(%.2f,%.2f) um;",
                          m_dLastWhiteDI_um, m_dLastMinWhiteDI_um, m_dLastMaxWhiteDI_um,
                          abs(cmplx(m_dLastXEccentricitet_um, m_dLastYEccentricitet_um)),
                          (cMaxes.real() - cMins.real()), (cMaxes.imag() - cMins.imag()));
                      }
                      else if (m_LastResults.m_dBlankInitialMaxDI_um > m_CurrentPart.m_dBlankIDmax_um)
                      {
                        MsgForEngine.Format("Failed; "
                          "// Too big blank diameter %.2f(%.2f,%.2f);Ecc=%.2f(%.2f,%.2f) um;",
                          m_dLastWhiteDI_um, m_dLastMinWhiteDI_um, m_dLastMaxWhiteDI_um,
                          abs(cmplx(m_dLastXEccentricitet_um, m_dLastYEccentricitet_um)),
                          (cMaxes.real() - cMins.real()), (cMaxes.imag() - cMins.imag()));
                      }
                    }
                  }
                }
                if (MsgForEngine.IsEmpty())
                {
                  MsgForEngine.Format("Finished; // Ecc=%.2f(%.2f,%.2f) um;",
                    abs(cmplx(m_dLastXEccentricitet_um, m_dLastYEccentricitet_um)),
                    (cMaxes.real() - cMins.real()), (cMaxes.imag() - cMins.imag()));
                  m_AdditionalInfo3.Format("Centering Finished \n"
                    "Cav(%.3f,%.3f)pix, Deviation(%.3f,%.3f)um",
                    m_cAverageCenterFor0Deg,
                    (cMaxes.real() - cMins.real()), (cMaxes.imag() - cMins.imag()));
                  pMarking->AddFrame(CreateTextFrame(cResultView, "0x008000", 24,
                    "CenteringResult", pDataFrame->GetId(), m_AdditionalInfo3));
                }
                else
                {
                  m_AdditionalInfo3.Format("Bad Blank DI=%.2f(%.2f,%.2f)um",
                    m_dLastWhiteDI_um, m_dLastMinWhiteDI_um, m_dLastMaxWhiteDI_um);
                  pMarking->AddFrame(CreateTextFrame(cResultView, "0x000000", 26,
                    "CenteringResult", pDataFrame->GetId(), m_AdditionalInfo3));
                  FXString ForCSVLog = m_LastResults.ToCSVString();
                  SaveCSVLogMsg("%s", (LPCTSTR)ForCSVLog);
                  m_LastResults.Reset();
                }
                SendMessageToEngine(MsgForEngine, "Centering Finished");

                if (m_cAverageCenterFor0Deg.real() == 0.)
                {
                  int iRadius = ROUND(m_dLastBlackDI_pix * 0.75);
                  CRect ForKnownPos(ROUND(m_cAverageCenterFor0Deg.real()) - iRadius,
                    ROUND(m_cAverageCenterFor0Deg.imag()) - iRadius,
                    ROUND(m_cAverageCenterFor0Deg.real()) + iRadius,
                    ROUND(m_cAverageCenterFor0Deg.imag()) + iRadius);
                  SetObjectPlacement(_T("circle_int"), ForKnownPos);
                  SetObjectPlacement(_T("back_light"), ForKnownPos);
                }
                SetCameraTriggerParams(false); // switch off trigger for image viewing
              }
            }
          }
          break;
        case STG_FrontWhiteCalibration:
          {
            if (++m_iFrameCntAfterDone >= 5)
            {
              ASSERT(m_bLastWhiteDIOK);
              double dFrontScaleMin = Reg.GetRegiDouble("Calibrations", "MinFrontScale_um_per_pixel", 0.78);
              double dFrontScaleMax = Reg.GetRegiDouble("Calibrations", "MaxFrontScale_um_per_pixel", 0.89);
              double dScale = m_dCalibIDia_um / (m_dLastWhiteDI_pix);
              if (IsInRange(dScale, dFrontScaleMin, dFrontScaleMax))
                m_dInternalWhiteScale_um_per_pixel = dScale;
              else
                ASSERT(0);
              Reg.WriteRegiString("Calibrations",
                "LastWhiteCalibrationTime", GetTimeStamp());
              Reg.WriteRegiCmplx("Calibrations", "HoleWhiteCenter_pix",
                m_cLastIntWhiteCenter_pix);
              Reg.WriteRegiCmplx("Calibrations", "WhiteConturCenter_pix",
                m_cLastWhiteConturCenter_pix);
              SaveScales();

              SetBackLight(false);
              SetFrontLight(true);
              SetCameraTriggerParams(false);
              ProgramImaging("Task(4);");
              m_WorkingStage = STG_FrontBlackCalibration;
              m_iFrameCntAfterDone = 0;
              m_dLastScalingTime = m_dLastMotionFinishedTime = GetHRTickCount();
            }
          }
          break;

        case STG_FrontWhiteMeasAndView:
        case STG_FrontBlackMeasAndView:
        case STG_FrontBlackCalibration:
        case STG_FrontGrinding:
        case STG_FrontPolishing:
        case STG_FrontWhiteFinalMeasurement:
        case STG_FrontBlackFinalMeasurement:
        case STG_FrontWhiteFinalWithRotation:
        case STG_Idle:
          {
            double dBigStepForFrontGrinding = Reg.GetRegiDouble("Motions",
              "BigStepForFrontGrinding_um", 2.);
            double dSmallStepForFrontGrinding = Reg.GetRegiDouble("Motions",
              "SmallStepForFrontGrinding_um", 1.);
            double dPolishStepForFrontGrinding = Reg.GetRegiDouble("Motions",
              "PolishStepForFrontGrinding_um", 0.5);
            double dDelayAfterMotion = Reg.GetRegiDouble("Motions",
              "DelayAfterLargeForFrontGrinding_ms", 1500.);
            double dTimeAfterMotion = m_bDone ? (GetHRTickCount() - m_dLastMotionFinishedTime) : 0.;
            FXString Comment;
            if (m_bLastBlackDIOK || m_bLastWhiteDIOK) // center was measured by imaging
            {
              if (m_bUseFrontLight && m_bLastBlackDIOK)
              {
                switch (m_WorkingStage)
                {
                case STG_FrontBlackCalibration:
                  {
                    if (GetHRTickCount() - m_dLastMotionFinishedTime > dDelayAfterMotion)
                    {
                      double dFrontScaleMin = Reg.GetRegiDouble("Calibrations", "MinFrontScale_um_per_pixel", 0.78);
                      double dFrontScaleMax = Reg.GetRegiDouble("Calibrations", "MaxFrontScale_um_per_pixel", 0.89);
                      double dScale = m_dCalibIDia_um / m_dLastBlackDI_pix;
                      if (IsInRange(dScale, dFrontScaleMin, dFrontScaleMax))
                        m_dInternalBlackScale_um_per_pixel = dScale;
                      if (!m_bLastBlackDOOK)
                        m_dExternalScale_um_per_pixel = m_dInternalBlackScale_um_per_pixel;
                      else
                      {
                        dScale = m_dCalibODia_um / (m_dLastExternalDia_pix);
                        if (IsInRange(dScale, dFrontScaleMin, dFrontScaleMax))
                          m_dExternalScale_um_per_pixel = dScale;
                      }

                      m_dScale_um_per_pix = 0.5 * (m_dInternalBlackScale_um_per_pixel + m_dExternalScale_um_per_pixel);
                      Reg.WriteRegiString("Calibrations",
                        "LastBlackCalibrationTime", GetTimeStamp());
                      Reg.WriteRegiCmplx("Calibrations", "HoleBlackCenter_pix",
                        m_cLastIntBlackCenter_pix);
                      Reg.WriteRegiCmplx("Calibrations", "ExtCenter_pix",
                        m_cLastExtCenter_pix);

                      CTextFrame * pSetScaleToImaging = CreateTextFrame(
                        "Scale", pDataFrame->GetId());
                      pSetScaleToImaging->GetString().Format("Scale(%.7f,um)", m_dScale_um_per_pix);
                      PutFrame(GetOutputConnector(MPPDO_Measurement_Control), pSetScaleToImaging);
                      CTextFrame * pSetScaleToRender = CreateTextFrame("Scale&Units", pDataFrame->GetId());
                      pSetScaleToRender->GetString().Format("%.7f,um", m_dScale_um_per_pix);
                      PutFrame(GetOutputConnector(MPPDO_VideoOut), pSetScaleToRender);
                      SaveScales();

                      FXString FinishMsg;
                      FinishMsg.Format("Finished;//Scales: IntB=%.5f IntW=%.5f ExtB=%.5f AvB=%.5f",
                        m_dInternalBlackScale_um_per_pixel, m_dInternalWhiteScale_um_per_pixel,
                        m_dExternalScale_um_per_pixel, m_dScale_um_per_pix);
                      SendMessageToEngine((LPCTSTR)FinishMsg, "CalibFinishedToEngine");
                      SetCameraTriggerParams(false);
                      Sleep(30);
                      SetFrontLight(false);
                      SetBackLight(true);
                      SetExposureAndGain(m_CurrentPart.m_iFrontForWhiteExposure_us,
                        m_CurrentPart.m_iGainForWhite_dBx10);
                      ProgramImaging("Task(4);");
                      SetIdleStage();

                      m_iFrameCntAfterDone = 0;
                      m_dLastMotionFinishedTime = GetHRTickCount();
                    }
                  }
                  break;

                case STG_FrontBlackFinalMeasurement:
                  {
                    if ((++m_iFrameCntAfterDone > 2) && m_bUseFrontLight) // Black stage of final measurement
                    {
                      bool bFinished = false;
                      if (m_iFinalAverageCntr < m_iAverageForFinalResults)
                      {
                        m_dBlackDI_um[m_iFinalAverageCntr] = m_dLastBlackDI_um;
                        m_dBlackDImin_um[m_iFinalAverageCntr] = m_dLastMinBlackDI_um;
                        m_dBlackDImax_um[m_iFinalAverageCntr] = m_dLastMaxBlackDI_um;
                        if (!m_bUseWhiteForFinalDI)
                          m_dFinalDIAveraged_um += m_dLastBlackDI_um;
                        if (!m_bUseWhiteForFinalMinMax)
                        {
                          m_dFinalDIMinAveraged_um += m_dLastMinBlackDI_um;
                          m_dFinalDIMaxAveraged_um += m_dLastMaxBlackDI_um;
                        }
                        double dTirB_pix = abs(m_cLastExtCenter_pix - m_cLastIntBlackCenter_pix);
                        m_dLastTirB_um = dTirB_pix * m_dInternalBlackScale_um_per_pixel;
                        double dLastTIRw_pix =
                          abs(m_cLastExtCenter_pix - m_cLastIntWhiteCenter_pix);
                        m_dLastTirW_um = dLastTIRw_pix * m_dInternalWhiteScale_um_per_pixel;
                        m_dTirBAveraged_um += m_dLastTirB_um;
                        m_dTirWAveraged_um += m_dLastTirW_um;
                        int iBlackCnt = m_CurrentPart.AddToBlack(m_dLastBlackDI_um,
                          m_dLastMinBlackDI_um, m_dLastMaxBlackDI_um,
                          m_dLastExternalDia_um, m_dLastTirB_um);
                        if (iBlackCnt = 1)
                          m_CurrentPart.m_dTIRw_Sum_um = 0.;
                        m_CurrentPart.m_dTIRw_Sum_um += m_dLastTirW_um;

                        if (++m_iFinalAverageCntr >= m_iAverageForFinalResults)
                        {
                          m_LastResults.m_dPartFinalBDI_um = (m_dFinalDIAveraged_um /= m_iFinalAverageCntr);
                          m_LastResults.m_dPartFinalBMinDI_um = (m_dFinalDIMinAveraged_um /= m_iFinalAverageCntr);
                          m_LastResults.m_dPartFinalBMaxDI_um = (m_dFinalDIMaxAveraged_um /= m_iFinalAverageCntr);
                          m_LastResults.m_dTirB_um = (m_dTirBAveraged_um /= m_iFinalAverageCntr);
                          m_LastResults.m_dTirW_um = (m_dTirWAveraged_um /= m_iFinalAverageCntr);
                          bFinished = true;
                        }
                      }
                      else
                        bFinished = true;

                      if (bFinished)
                      {
                        StartMotor();
                        m_WorkingStage = STG_FrontWhiteFinalWithRotation;
                        SetFrontLight(false);
                        SetBackLight(true);
                        SetExposureAndGain(m_CurrentPart.m_iFrontForWhiteExposure_us,
                          m_CurrentPart.m_iGainForWhite_dBx10);
                        m_CurrentPart.ResetWhiteRotationResults();
                        m_iFinalAverageCntr = 0;
                      }
                    }
                    else
                      m_CurrentPart.ResetBlackResults();
                  }
                  break;
                }
              }
              else if (m_bUseBackLight && m_bLastWhiteDIOK)
              {
                FXString AsText;
                double dHoleTargetSizeCorrection_um = -3 ;
                cmplx ViewDiaPt(m_cLastROICent_pix.real() * 1.25, m_cLastROICent_pix.imag());
                switch (m_WorkingStage)
                {
                case STG_FrontGrinding:
                  {
                    dHoleTargetSizeCorrection_um = Reg.GetRegiDouble(
                      "Control", "HoleTargetSizeCoarseCorrection_um", -2.);
                    double dDiffToUse_um = GetLastWhiteMeasurementResult(AsText,
                      dHoleTargetSizeCorrection_um);
                    LPCTSTR pColor = (m_dLastDiaDiffToNominal_um > 0.) ?
                      "0x00ffff" : (m_dLastDiaDiffToMax_um > 0.) ? "0x00ff00" : "0x0000ff";

                    m_InfoAboutWhiteGrindingMeasurement.Format(
                      _T("Hole Dia=%.2fum\nTo Nominal=%.2fum\nMinToMin=%.2fum\n"
                      "Grind Dist=%.2fum\nWork By %s %.2fum\n"
                      "Addition=%.2fum"),
                      m_dLastWhiteDI_um, m_dLastDiaDiffToNominal_um,
                      m_dLastDiaDiffMinToMin_um, m_dLastGrindingDist_um,
                      m_bMinToMinForFrontGrinding ? "MinToMinDia" : "AveDia",
                      m_bMinToMinForFrontGrinding ?
                      m_dLastMinWhiteDI_um : m_dLastWhiteDI_um,
                      dHoleTargetSizeCorrection_um);
                    pMarking->AddFrame(CreateTextFrame(ViewDiaPt,
                      pColor, 14, _T("DiaReport"), pDataFrame->GetId(),
                      (LPCTSTR)m_InfoAboutWhiteGrindingMeasurement));
                    if ((dTimeAfterMotion > dDelayAfterMotion) && (dDiffToUse_um < 100.))
                    {
                      double dMoveDist = 0.;
                      if (dDiffToUse_um > dBigStepForFrontGrinding )
                        dMoveDist = dBigStepForFrontGrinding;
                      else
                      {
                        dDiffToUse_um = GetLastWhiteAveragedResult(10, dHoleTargetSizeCorrection_um);
                        LPCTSTR pColor = (m_dLastDiaDiffToNominal_um > 0.) ?
                          "0x00ffff" : (m_dLastDiaDiffToMax_um > 0.) ? "0x00ff00" : "0x0000ff";
                        cmplx ViewDiaPt(m_cLastROICent_pix.real() * 1.25, m_cLastROICent_pix.imag() * 1.37);
                        m_InfoAboutLastResults = m_CurrentPart.ResultsToString() + _T("\nGrinding");

                        if (m_dLastGrindingDist_um > dSmallStepForFrontGrinding)
                        {
                          dMoveDist = dSmallStepForFrontGrinding;
                          m_InfoAboutLastResults += _T(" Small Step");
                        }
                      }
                      SaveGrindingLogMsg("%d,%.2f,%.2f,%.2f,%.2f,Grinding",
                        ++m_LastResults.m_iNGrindingCycles,
                        m_LastResults.m_dPartAfterFrontGrindingDI_um = m_dLastWhiteDI_um,
                        m_LastResults.m_dPartAfterFrontGrindingMinDI_um = m_dLastMinWhiteDI_um,
                        m_LastResults.m_dPartAfterFrontGrindingMaxDI_um = m_dLastMaxWhiteDI_um,
                        dMoveDist);
                      if (dMoveDist)
                        MoveFrontStone(-dMoveDist, Comment);
                      else
                      {
                        FXString Msg("Finished;//");
                        Msg += AsText;
                        SendMessageToEngine(Msg, "GrindingFinished");
                        SetIdleStage();
                        m_InfoAboutLastResults += _T(" finished");
                        m_dLastMinDiaAfterGrinding_um = m_dLastMinWhiteDI_um ;
                      }
                      m_bWasGrindingOrPolishing = TRUE;
                      pMarking->AddFrame(CreateTextFrame(ViewDiaPt,
                        pColor, 14, _T("DiaReport"), pDataFrame->GetId(),
                        (LPCTSTR)m_InfoAboutLastResults));
                    }
                    else
                      m_CurrentPart.ResetWhiteResults();
                  }
                  break;
                case STG_FrontPolishing:
                  {
                    if ((dTimeAfterMotion > dDelayAfterMotion) || (m_bMotorForFrontGrinding && m_bDone))
                    {
                      ViewDiaPt._Val[_IM] = m_cLastROICent_pix.imag() * 1.4;
                      FXString Addition;
                      if (!m_bMotorForFrontGrinding)
                      {
                        StartMotor();
                        m_bDone = false;
                        m_bMotorForFrontGrinding = true;
                        m_dLastMotionFinishedTime = GetHRTickCount();
                        break;
                      }

                      dHoleTargetSizeCorrection_um = Reg.GetRegiDouble(
                        "Control", "HoleTargetSizeCorrection_um" , 0.5 );
                      double dDiffToUse_um = GetLastWhiteMeasurementResult(AsText,
                        dHoleTargetSizeCorrection_um);
                      LPCTSTR pColor = (m_dLastDiaDiffToNominal_um > 0.) ?
                        "0x00ffff" : (m_dLastDiaDiffToMax_um > 0.) ? "0x00ff00" : "0x0000ff";
                      cmplx ViewDiaPt(m_cLastROICent_pix.real() * 1.25, m_cLastROICent_pix.imag());

                      m_InfoAboutWhiteGrindingMeasurement.Format(
                        _T("Hole Dia=%.2fum\nTo Nominal=%.2fum\nMinToMin=%.2fum\n"
                        "Grind Dist=%.2fum\nWork By %s %.2fum\n"
                        "Addition=%.2fum PStep=%d"),
                        m_dLastWhiteDI_um, m_dLastDiaDiffToNominal_um,
                        m_dLastDiaDiffMinToMin_um, m_dLastGrindingDist_um,
                        m_bMinToMinForFrontGrinding ? "MinToMinDia" : "AveDia",
                        m_bMinToMinForFrontGrinding ?
                        m_dLastMinWhiteDI_um : m_dLastWhiteDI_um,
                        dHoleTargetSizeCorrection_um , m_iPolishSteps );
                      FXString ForGrindingLog;
                      ForGrindingLog.Format("%d,%.2f,%.2f,%.2f,",
                        m_LastResults.m_iNPolishingCycles,
                        m_LastResults.m_dPartAfterPolishingDI_um = m_dLastWhiteDI_um,
                        m_LastResults.m_dPartAfterPolishingMinDI_um = m_dLastMinWhiteDI_um,
                        m_LastResults.m_dPartAfterPolishingMaxDI_um = m_dLastMaxWhiteDI_um);
                      pMarking->AddFrame(CreateTextFrame(ViewDiaPt,
                        pColor, 14, _T("DiaReport"), pDataFrame->GetId(),
                        (LPCTSTR)m_InfoAboutWhiteGrindingMeasurement));
                      int iNPolishSteps = Reg.GetRegiInt("Motions", "NPolishSteps", 4);
                      
                      if (m_iPolishSteps >= iNPolishSteps)
                      {
                        dDiffToUse_um = GetLastWhiteAveragedResult(10, dHoleTargetSizeCorrection_um);
                        if (m_CurrentPart.m_iWhiteResultCounter >= 10)
                        {
                          pColor = (m_dLastDiaDiffToNominal_um > 0.) ?
                            "0x00ffff" : (m_dLastDiaDiffToMax_um > 0.) ? "0x00ff00" : "0x0000ff";
                          ViewDiaPt._Val[_IM] = m_cLastROICent_pix.imag() * 1.4;
                          m_InfoAboutLastResults = m_CurrentPart.ResultsToString() + _T("\nPolishing ");
                        }
                        else
                          break; // continue accumulate info for averaging
                      }
                      else
                      {
                        m_CurrentPart.ResetWhiteResults();
                        m_InfoAboutLastResults.Empty();
                      }
                      if ((m_iPolishSteps++ < iNPolishSteps) || (dDiffToUse_um > 0.))
                      {
                        FXString Addition;
                        Addition.Format(" - Polish Step %d; dDiff=%.2f", m_iPolishSteps,
                          fabs(dDiffToUse_um) < 100. ? dDiffToUse_um : 0.);
                        m_dLastWhiteDI_um = m_CurrentPart.GetDIWhiteWithIdleAverageResults(
                          m_dLastMinWhiteDI_um, m_dLastMaxWhiteDI_um);
                        m_dLastDiaDiffMinToMin_um = m_dLastMinWhiteDI_um - m_CurrentPart.m_dIDmin_um;
                        m_dLastDiaDiffMaxToMax_um = m_dLastMaxWhiteDI_um - m_CurrentPart.m_dIDmax_um;

                        double dSpare = 0.;
                        if (m_dLastDiaDiffMinToMin_um >= 0.)
                        {
                          // following value is < 0., if there is possibility to grind without
                          // cross IDmax value by real max diameter
                          dSpare = m_dLastDiaDiffMinToMin_um + m_dLastDiaDiffMaxToMax_um;
                        }
                        double dMinDiffToLastDiaAfterCoarseGrinding_um = 
                          Reg.GetRegiDouble( "Control" , "MinimalPolishingDist_um"  , 2. );
                        
                        double dDiffToCoarseGrinding = m_dLastMinWhiteDI_um - m_dLastMinDiaAfterGrinding_um ;
                        bool bEnoughGrinded = (dDiffToCoarseGrinding > dMinDiffToLastDiaAfterCoarseGrinding_um) ;
                        if ( ((m_dLastDiaDiffMinToMin_um < 0.) && !bEnoughGrinded )
                          || ((m_dLastDiaDiffMinToMin_um < 1.) && (dSpare < -1.))) // spare is > 1 micron on both sides
                        {
                          Addition.Format("%.2f,Polish", dPolishStepForFrontGrinding);
                          ForGrindingLog += Addition;
                          SaveLogMsg((LPCTSTR)ForGrindingLog);
                          Addition.Format(" - Polish Step %d; ToMin=%.2f ToNom=%.2f ToMax=%.2f Spare=%.2f",
                            m_iPolishSteps, m_dLastDiaDiffMinToMin_um,
                            m_dLastDiaDiffToNominal_um, m_dLastDiaDiffMaxToMax_um,
                            dSpare);
                          Comment += Addition;
                          MoveFrontStone(-dPolishStepForFrontGrinding, Comment);
                          m_bMotorForFrontGrinding = false;
                          m_bWasGrindingOrPolishing = TRUE;
                          m_InfoAboutLastResults += "Moved";
                        }
                        else // may be finished
                        {
                          m_bWaitAnswerFromFRender = (bool)Reg.GetRegiInt("Control",
                            "TryToContinuePolishing", FALSE);
                          if ( m_bWaitAnswerFromFRender)
                          {
                            SetFrontLight(true, true);

                            cmplx ViewQuestion(20., 20. );

                            FXString Info ;
                            Info.Format( "Hole Dia=%.2f[%.2f,%.2f]\n" ,
                              m_dLastWhiteDI_um , m_dLastMinWhiteDI_um , m_dLastMaxWhiteDI_um ) ;
                            LPCTSTR Questions[] =
                            {
                              "Continue Polish" , "Finish Polish" , NULL
                            } ;

                            vector<CRect> Zones ;

                            CreateQuestionsOnRender( ViewQuestion , Info ,
                              Questions , Zones , pMarking ) ;

                            if ( Zones.size() == 2 )
                            {
                              m_ContinueRect = Zones[ 0 ] ;
                              m_FinishRect = Zones[ 1 ] ;
                            }
                            else
                              m_bWaitAnswerFromFRender = false ;
                            if ( !m_bWaitAnswerFromFRender )
                            {
                              SendMessageToEngine( "Finished;" , "GrindingFinished" );
                              SetIdleStage();
                              m_bMotorForFrontGrinding = false;
                              m_InfoAboutLastResults += "Finished";

                              ForGrindingLog += "0.00,Finished";
                              SaveGrindingLogMsg( ( LPCTSTR ) ForGrindingLog );
                            }
                          }
                        }
                        if ( !m_bWaitAnswerFromFRender )
                        {
                          pMarking->AddFrame(CreateTextFrame(ViewDiaPt,
                            pColor, 14, _T("DiaReport"), pDataFrame->GetId(),
                            (LPCTSTR)m_InfoAboutLastResults));
                        }
                      }
                      if (!m_bWaitAnswerFromFRender)
                      {
                        ViewDiaPt._Val[_RE] = 20.;
                        pMarking->AddFrame(CreateTextFrame(ViewDiaPt,
                          pColor, 14, _T("DiaReport"), pDataFrame->GetId(),
                          _T("%s=%.2fum\nTo Target=%.2fum %s\nMove=%.2fum\nPolish Step %i"),
                          m_bMinToMinForFrontGrinding ? "MinDia" : "Dia",
                          m_bMinToMinForFrontGrinding ? m_dLastMinWhiteDI_um : m_dLastWhiteDI_um,
                          dDiffToUse_um, dPolishStepForFrontGrinding, m_iPolishSteps));
                      }
                    }
                    break;
                case STG_FrontWhiteFinalMeasurement:
                  {
                    if (++m_iFrameCntAfterDone > 3)
                    {
                      double dTirW_pix = abs(m_cLastExtCenter_pix - m_cLastIntWhiteCenter_pix);
                      m_dLastTirW_um = dTirW_pix * m_dInternalWhiteScale_um_per_pixel;
                      m_dLastWhiteDI_pix = m_dLastDI_pix;
                      m_dLastWhiteDI_um = m_dLastDI_pix * m_dInternalWhiteScale_um_per_pixel * m_dFrontScaleCorrectionCoeff;
                      bool bFinishedWhite = false;
                      if (m_bUseWhiteForFinalDI)
                        m_dFinalDIAveraged_um += m_dLastWhiteDI_um;
                      if (m_bUseWhiteForFinalMinMax)
                      {
                        m_dFinalDIMinAveraged_um += m_dLastMinWhiteDI_um;
                        m_dFinalDIMaxAveraged_um += m_dLastMaxWhiteDI_um;
                      }
                      m_CurrentPart.AddToWhite(m_dLastWhiteDI_um,
                        m_dLastMinWhiteDI_um, m_dLastMaxWhiteDI_um, m_dLastTirW_um);
                      if (++m_iFinalAverageCntr >= m_iAverageForFinalResults)
                      {
                        m_LastResults.m_dPartFinalWDI_um = (m_dFinalDIAveraged_um /= m_iFinalAverageCntr);
                        m_LastResults.m_dPartFinalWMinDI_um = (m_dFinalDIMinAveraged_um /= m_iFinalAverageCntr);
                        m_LastResults.m_dPartFinalWMaxDI_um = (m_dFinalDIMaxAveraged_um /= m_iFinalAverageCntr);

                        m_iFrameCntAfterDone = 0;
                        m_iFinalAverageCntr = 0;
                        m_WorkingStage = STG_FrontBlackFinalMeasurement;
                        SetFrontLight(true);
                        SetBackLight(false);
                        SetExposureAndGain(m_CurrentPart.m_iFrontForBlackExposure_us,
                          m_CurrentPart.m_iGainForBlack_dBx10);
                        m_CurrentPart.ResetBlackResults();
                      }
                    }
                  }
                  break;
                case STG_FrontWhiteFinalWithRotation:
                  {
                    bool bFinished = false;
                    if (m_bMotorIsOn && (++m_iFrameCntAfterDone > 10))
                    {
                      int iWhiteRotationCntr = m_CurrentPart.AddToWhiteRotation(
                        m_dLastWhiteDI_um, m_dLastMinWhiteDI_um, m_dLastMaxWhiteDI_um,
                        m_dLastTirW_um);
                      if (iWhiteRotationCntr >= m_iAverageForFinalResults)
                      {
                        int iNWaitForViewCycles = Reg.GetRegiInt("Motions", "NWaitAfterBlackMeas", 10);
                        if (m_iFrameCntAfterDone > iNWaitForViewCycles + m_iAverageForFinalResults + 3)
                        {
                          m_iFrameCntAfterDone = 0;
                          SetIdleStage();

                          double dHoleTargetSizeCorrection_um = Reg.GetRegiDouble(
                            "Control", "HoleTargetSizeCorrection_um", 0.);
                          double dAutoCorrection_PercOfDiff = Reg.GetRegiDouble(
                            "Control", "AutoCorrection_PercOfDiff", 50.);
                          double dCorrection_um = 0.;
                          double dError_um = 0.;
                          if (dAutoCorrection_PercOfDiff && m_bWasGrindingOrPolishing)
                          {
                            if (m_bMinToMinForFrontGrinding)
                            {  // hole minimal diameter to minimal diameter in spec
                              double dMinimalDiameterShift_um = Reg.GetRegiDouble(
                                "Control", "MinimalDiameterShift_um", 1.);
                              dError_um = m_dFinalDIMinAveraged_um - m_CurrentPart.m_dIDmin_um;
                              dCorrection_um = (dError_um - dMinimalDiameterShift_um)
                                * dAutoCorrection_PercOfDiff / 100.;
                              dHoleTargetSizeCorrection_um += dCorrection_um;
                            }
                            else // hole diameter to nominal diameter
                            {
                              double dDiaNominal_um = m_CurrentPart.GetNominalHoleDia_um();
                              dError_um = m_dFinalDIAveraged_um - dDiaNominal_um;
                              dCorrection_um = dError_um * dAutoCorrection_PercOfDiff / 100.;
                              dHoleTargetSizeCorrection_um += dCorrection_um;
                            }
                            m_bWasGrindingOrPolishing = FALSE;
                          }
                          if (dHoleTargetSizeCorrection_um != 0.)
                          {
                            Reg.WriteRegiDouble("Control",
                              "HoleTargetSizeCorrection_um", dHoleTargetSizeCorrection_um);
                          }
                          m_dLastWhiteDI_um = m_CurrentPart.GetDIWhiteAverageResults(
                            m_dLastMinWhiteDI_um, m_dLastMaxWhiteDI_um,
                            m_dLastTirW_um);
                          m_dLastWhiteRotationDI_um =
                            m_CurrentPart.GetDIWhiteRotationAverageResults(
                            m_dLastMinWhiteRotationDI_um, m_dLastMaxWhiteRotationDI_um,
                            m_dLastTirWRotation_um);
                          m_dLastMinBlackDI_um = m_CurrentPart.GetBlackAverageResults(
                            m_dLastMinBlackDI_um, m_dLastMaxBlackDI_um,
                            m_dLastExternalDia_um, m_dLastTirB_um);
                          m_dLastSideExternalDia_um = m_CurrentPart.GetSideAverageResults(
                            m_LastAveragedConeAngle_deg);

                          BOOL bUseRotationResultsForWhite =
                            Reg.GetRegiInt(
                            "Measurements", "UseWhiteRotationResult", 1);
                          if (bUseRotationResultsForWhite)
                          {
                            m_LastResults.m_dPartFinalWDI_um = m_dFinalDIAveraged_um = m_dLastWhiteRotationDI_um;
                            m_LastResults.m_dPartFinalWMinDI_um = m_dFinalDIMinAveraged_um = m_dLastMinWhiteRotationDI_um;
                            m_LastResults.m_dPartFinalWMaxDI_um = m_dFinalDIMaxAveraged_um = m_dLastMaxWhiteRotationDI_um;
                            m_dTirWAveraged_um = m_dLastTirWRotation_um;
                          }
                          else
                          {
                            BOOL bUseWhiteForDI = Reg.GetRegiInt(
                              "Measurements", "UseWhiteForFinalDI", 1);
                            BOOL bUseWhiteForMaxMin = Reg.GetRegiInt(
                              "Measurements", "UseWhiteForFinalDIMinMax", 1);
                            m_dFinalDIAveraged_um = (bUseWhiteForDI) ?
                              m_dLastWhiteDI_um : m_dLastBlackDI_um;
                            if (bUseWhiteForMaxMin)
                            {
                              m_dFinalDIMinAveraged_um = m_dLastMinWhiteDI_um;
                              m_dFinalDIMaxAveraged_um = m_dLastMaxWhiteDI_um;
                            }
                          }
                          m_dTirBAveraged_um = m_dLastTirB_um;
                          FXString FinalResult, AddInfo1, AddInfo2;
                          FinalResult.Format("Finished: HoleD=%.2f; TipD=%.2f; Tir=%.2f; "
                            "HoleDmin=%.2f; HoleDmax=%.2f; Ellipt=%.2f; ConAng=%.2f;",
                            m_dFinalDIAveraged_um,
                            m_dLastExternalDia_um, m_dTirWAveraged_um,
                            m_dFinalDIMinAveraged_um, m_dFinalDIMaxAveraged_um,
                            m_dFinalDIMaxAveraged_um - m_dFinalDIMinAveraged_um,
                            m_LastAveragedConeAngle_deg);
                          AddInfo1.Format("// DIb=%.2f(%.2f,%.2f); DIw=%.2f(%.2f,%.2f); DIwr=%.2f(%.2f,%.2f); "
                            "TIRb=%.2f; ElliptBlack=%.2f; ElliptWhite=%.2f; ElliptWhiteR=%.2f; ",
                            m_dLastBlackDI_um, m_dLastMinBlackDI_um, m_dLastMaxBlackDI_um,
                            m_dLastWhiteDI_um, m_dLastMinWhiteDI_um, m_dLastMaxWhiteDI_um,
                            m_dLastWhiteRotationDI_um, m_dLastMinWhiteRotationDI_um,
                            m_dLastMaxWhiteRotationDI_um, m_dTirBAveraged_um,
                            m_dLastMaxBlackDI_um - m_dLastMinBlackDI_um,
                            m_dLastMaxWhiteDI_um - m_dLastMinWhiteDI_um,
                            m_dLastMaxWhiteRotationDI_um - m_dLastMinWhiteRotationDI_um);
                          LPCTSTR pDIby = bUseRotationResultsForWhite ? "WhiteRot"
                            : (m_bUseWhiteForFinalDI ? "White" : "Black");
                          LPCTSTR pMaxMinBy = bUseRotationResultsForWhite ? "WhiteRot"
                            : (m_bUseWhiteForFinalMinMax ? "White" : "Black");

                          AddInfo2.Format("TipToMaster=%.2um;ScaleCorr=%g;"
                            "Error=%.2fum; Correction=%.2fum; CorrForNext=%.2fum;",
                            m_dDistFromLastVertEdgeToMasterVertEdge_um, m_dFrontScaleCorrectionCoeff,
                            dError_um, dCorrection_um, dHoleTargetSizeCorrection_um, m_iAverageForFinalResults);
                          FinalResult += AddInfo1 + AddInfo2;
                          AddInfo2.Format("DI by %s;MaxMin by %s;", pDIby, pMaxMinBy);
                          FinalResult += AddInfo2;

                          SendMessageToEngine(FinalResult, "FinalResult");

                          LPCTSTR pColor = (m_dLastDiaDiffToNominal_um > 0.) ?
                            "0x00ffff" : (m_dLastDiaDiffToMax_um > 0.) ? "0x00ff00" : "0x0000ff";
                          cmplx ViewDiaPt(m_cLastROICent_pix.real() * 1.25, m_cLastROICent_pix.imag() + 10);

                          m_InfoAboutLastResults = m_CurrentPart.ResultsToString();
                          pMarking->AddFrame(CreateTextFrame(ViewDiaPt,
                            pColor, 14, _T("DiaReport"), pDataFrame->GetId(),
                            (LPCTSTR)m_InfoAboutLastResults));
                          m_LastResults.m_FinalMeasurementFinishedTimeStamp = GetTimeAsString_ms();
                          FXString ForCSV = m_LastResults.ToCSVString();
                          SaveCSVLogMsg("%s", (LPCTSTR)ForCSV);
                          m_LastResults.Reset();
                        }
                      }
                    }
                    else
                    {
                      m_CurrentPart.ResetWhiteRotationResults();
                      m_iFinalAverageCntr = 0;
                    }
                  }
                  break;
                  }
                }
              }
            }
            break;
          }
        }
        break;
      }
    case MPPD_Side:
      {
        m_cMasterCenter_pix = 0.5 *(m_cCalibSideUpperRightCorner + m_cCalibSideLowerRightCorner);
        double dDist_pix = -1000.;
        double dDist_um = dDist_pix * m_dScale_um_per_pix;

        CColorSpot * pFrontDistData = GetSpotData("front_dist");
        if (pFrontDistData)
        {
          dDist_pix = pFrontDistData->m_dBlobWidth;
          dDist_um = dDist_pix * m_dScale_um_per_pix;
          m_dConesEdgePos_pix = pFrontDistData->m_SimpleCenter.x - dDist_pix / 2.;
          double dDistFromEdgeToMasterEdge_pix = m_dConesEdgePos_pix - m_cMasterCenter_pix.real();
          double dDistFromEdgeToMasterEdge_um = dDistFromEdgeToMasterEdge_pix * m_dScale_um_per_pix;
          m_dMaxTipDeviationFromMaster_um = Reg.GetRegiDouble(
            "Measurements", "MaxTipDeviationFromMaster_um", 70.);

          if (fabs(dDistFromEdgeToMasterEdge_um) > m_dMaxTipDeviationFromMaster_um)
          {
            cmplx cViewPt(m_cLastROICent_pix.real() * 0.5, m_cLastROICent_pix.imag() * 0.5);
            if (dDistFromEdgeToMasterEdge_um > 0.)
            {
              pMarking->AddFrame(CreateTextFrame(cViewPt, "0xff00ff", 20,
                "InfoData", pDataFrame->GetId(), "TOO LONG BLANK\n%.1fum FROM MASTER",
                dDistFromEdgeToMasterEdge_um));
            }
            else
            {
              pMarking->AddFrame(CreateTextFrame(cViewPt, "0xff00ff", 20,
                "InfoData", pDataFrame->GetId(), "TOO SHORT BLANK\n%.1fum FROM MASTER",
                dDistFromEdgeToMasterEdge_um));
            }
          }
        }

        CRect LeftAverageArea(ROUND(m_cMasterCenter_pix.real() - 150.),
          ROUND(m_cMasterCenter_pix.imag() - 50.),
          ROUND(m_cMasterCenter_pix.real() - 100.),
          ROUND(m_cMasterCenter_pix.imag() + 50.));
        CRect RightAverageArea(ROUND(m_cMasterCenter_pix.real() + 50.),
          ROUND(m_cMasterCenter_pix.imag() - 50.),
          ROUND(m_cMasterCenter_pix.real() + 100.),
          ROUND(m_cMasterCenter_pix.imag() + 50.));
        pMarking->AddFrame(CreateFigureFrame(LeftAverageArea, 0x0000ff));
        pMarking->AddFrame(CreateFigureFrame(RightAverageArea, 0x0000ff));
        double dLeftAver = _calc_average(pVF, LeftAverageArea);
        double dRightAver = _calc_average(pVF, RightAverageArea);
        CRect CentRect(CPoint(ROUND(m_cMasterCenter_pix.real()), ROUND(m_cMasterCenter_pix.imag())),
          CSize(11, 11));
        CRect UpperRect = CentRect, LowerRect = CentRect;
        UpperRect.OffsetRect(-5, ROUND(-m_cLastROICent_pix.imag() * 0.4));
        LowerRect.OffsetRect(-5, ROUND(m_cLastROICent_pix.imag() * 0.4));
        pMarking->AddFrame(CreateFigureFrame(UpperRect, 0x0000ff));
        pMarking->AddFrame(CreateFigureFrame(LowerRect, 0x0000ff));
        double dUpperCheckPtAver = _calc_average(pVF, UpperRect);
        double dLowerCheckPtAver = _calc_average(pVF, LowerRect);

        m_AdditionalInfo2.Format("AVl=%.2f,AVr=%.2f,Up=%.2f,Down=%.2f ",
          dLeftAver, dRightAver, dUpperCheckPtAver, dLowerCheckPtAver);

        double dMoveFrontStoneDist = 0.;
        switch (m_WorkingStage)
        {
        case STG_FrontStoneInit:
          {
            double dFrontStepBeforeGrinding = Reg.GetRegiDouble("Motions",
              "FrontStepBeforeGrinding_um", 100.);
            if ((m_dLastMotionFinishedTime != 0.) && pFrontDistData)
            {
              if (dDist_um < 60.0)
              {
                SendMessageToEngine("ReadyToGrind;", NULL);
                SetIdleStage();
                m_bReadyToGrind = true;
                m_dLastFrontDistaceToStone_um = dDist_um;
                m_AdditionalInfo2.Format("Front is ready for Grinding\n"
                  "Last Dist=%.2f um", dDist_um);
                m_LastResults.m_iNGrindingCycles = m_LastResults.m_iNPolishingCycles = 0;
              }
              else
                MoveFrontStone(dMoveFrontStoneDist = (-dDist_um / 4.));
            }
            else if (m_dLastMotionFinishedTime != 0.)
            {
              double dDiffLR = dRightAver - dLeftAver;
              double dDiffLU = dUpperCheckPtAver - dLeftAver;
              double dDiffLL = dLowerCheckPtAver - dLeftAver;
              if ( /*( dDiffLR > 60. ) &&*/ (dDiffLU > 70.) && (dDiffLL > 70.))
                MoveFrontStone(dMoveFrontStoneDist = -dFrontStepBeforeGrinding);
              else
              {
                SendMessageToEngine("Failed;", "BadImageForFrontSToneAdjust");
                SetIdleStage();
              }
            }
            if (dMoveFrontStoneDist != 0.)
            {
              m_AdditionalInfo2.Format("Move Front Stone %.2f um", dMoveFrontStoneDist);
              m_dLastFrontStoneMoveDist = dMoveFrontStoneDist;
            }
          }
          break;
        }
      }
      break;
    }
  }
  if (m_iViewMode >= 3)
  {
    cmplx ViewPt(20., 60.);
    double dXCoeff = (m_dLastBlackDI_pix > 70.) ? -0.1 : 0.3;
    //     cmplx ViewPt( m_cLastExtCenter_pix.real() + dExtRadius_pix * dXCoeff , m_cLastExtCenter_pix.imag() - dExtRadius_pix * 0.3 ) ;
    if (m_dLastExternalDia_pix > 0.)
    {
      double dExternalCorrectedDia = (m_dLastExternalDia_pix + m_dExternalCorrection_pix);
      m_dLastExternalDia_pix = dExternalCorrectedDia;
      pMarking->AddFrame(CreateTextFrame(ViewPt, "0x00ff00", 14,
        "ExtData", pDataFrame->GetId(), "\nDO=%.2f um",
        dExternalCorrectedDia * m_dExternalScale_um_per_pixel/*m_dScale_um_per_pix*/));
    }
    if (m_dLastWhiteDI_um > 0.)
    {
      double dInternalCorrectedDia = (m_dLastWhiteDI_pix + m_dInternalCorrection_pix);
      pMarking->AddFrame(CreateTextFrame(ViewPt, "0x00ff00", 14,
        "IntData", pDataFrame->GetId(), "\n\nDIw=%.2f [%.2f,%.2f]um dMaxMinW = %.2f",
        m_dLastWhiteDI_um, m_dLastMinWhiteDI_um, m_dLastMaxWhiteDI_um,
        m_dLastMaxWhiteDI_um - m_dLastMinWhiteDI_um));
    }
    if (m_dLastBlackDI_um > 0.)
    {
      double dInternalCorrectedDia = (m_dLastBlackDI_pix + m_dInternalCorrection_pix);
      pMarking->AddFrame(CreateTextFrame(ViewPt, "0x00ff00", 14,
        "IntData", pDataFrame->GetId(), "\n\n\nDIb=%.2f [%.2f,%.2f]um dMaxMinB = %.2f",
        m_dLastBlackDI_um,
        m_dLastMinBlackDI_um, m_dLastMaxBlackDI_um,
        m_dLastMaxBlackDI_um - m_dLastMinBlackDI_um));
    }
    if (m_dLastWhiteDI_pix > 0. && m_dLastExternalDia_pix > 0.)
    {
      pMarking->AddFrame(CreateTextFrame(ViewPt, "0x00ff00", 14,
        "Eccentricity", pDataFrame->GetId(), "\n\n\n\n\nTIRb=%.2fum TIRw=%.2fum Ang=%.2fdeg ",
        abs(m_cLastExtCenter_pix - m_cLastIntBlackCenter_pix) * m_dScale_um_per_pix,
        abs(m_cLastExtCenter_pix - m_cLastIntWhiteCenter_pix) * m_dScale_um_per_pix,
        m_CurrentPart.m_dResultConuseAngle_deg));
    }
    cmplx cScaleViewPt = m_cLastROICent_pix
      + cmplx(-m_cLastROICent_pix.real() * 0.9, m_cLastROICent_pix.imag() * 0.93);
    CTextFrame * pGlobalInfo = (m_WorkingMode == MPPD_Side) ?
      CreateTextFrame(cScaleViewPt, "0x0000ff", 12, "Scale", 0,
      "Scale=%.5f (um/pix)  Part=%s Ver.=%s %s",
      m_dScale_um_per_pix, (LPCTSTR)m_PartName,
#ifdef _DEBUG
      "Debug"
#else
      "Release"
#endif
      , __TIMESTAMP__)
      : ( m_WorkingMode == MPPD_Front ) ?
      CreateTextFrame(cScaleViewPt, "0x0000ff", 11, "Scale", 0,
      "Sib=%.5f Siw=%.5f Se=%.5f(um/pix) Part=%s V=%s %s",
      m_dInternalBlackScale_um_per_pixel, m_dInternalWhiteScale_um_per_pixel,
      m_dExternalScale_um_per_pixel,
      (LPCTSTR)m_PartName,
#ifdef _DEBUG
      "Debug"
#else
      "Release"
#endif
      , __TIMESTAMP__) 
      :
      CreateTextFrame( cScaleViewPt , "0x0000ff" , 11 , "Scale" , 0 ,
        "Sib=%.5f Siw=%.5f Se=%.5f(um/pix) V=%s %s" ,
        m_dInternalBlackScale_um_per_pixel , m_dInternalWhiteScale_um_per_pixel ,
        m_dExternalScale_um_per_pixel ,
#ifdef _DEBUG
        "Debug"
#else
        "Release"
#endif
        , __TIMESTAMP__ )
      ;
    pMarking->AddFrame(pGlobalInfo);

    if (m_cLastExtCenter_pix.real())
      pMarking->AddFrame(CreatePtFrame(m_cLastExtCenter_pix, "color=0x00ffff; Sz=5;"));
    if (m_cLastIntBlackCenter_pix.real())
      pMarking->AddFrame(CreatePtFrame(m_cLastIntBlackCenter_pix, "color=0xff00ff; Sz=5;"));
    if (m_cLastIntWhiteCenter_pix.real())
      pMarking->AddFrame(CreatePtFrame(m_cLastIntWhiteCenter_pix, "color=0x00ff00; Sz=5;"));
  }

  if (!m_AdditionalMsgForManual.IsEmpty())
    pMarking->AddFrame(CreateTextFrame(cmplx(-400., -300.),
    m_AdditionalMsgForManual, 0x00ff00, 24, NULL, 0));

  pMarking->AddFrame(pDataFrame);

  m_LastFigures.clear();
  m_LastSegments.clear();
  return pMarking;
}

int MPPT_Dispens::GetCircles(const CDataFrame* pDataFrame, CContainerFrame * pMarking) // 
{
  int iRes = 0;
  m_FoundExtCircles.clear();
  m_FoundIntCircles.clear();
  m_FoundCircles.clear();
  CFramesIterator * pIter = pDataFrame->CreateFramesIterator(figure);
  if (pIter)
  {
    CFigureFrame * pFigure = NULL;
    cmplx cInternal, cExternal, cBlankInt, cBlankExt;
    double dRadInternal = 0., dRadExternal = 0.;
    double dBlankRadInt = 0., dBlankRadExt = 0.;
    while (pFigure = (CFigureFrame*)pIter->Next())
    {
      LPCTSTR pLabel = pFigure->GetLabel();
      FXString Label(pLabel);
      LPCTSTR pCircleText = NULL;
      bool bBackLight = false;
      if (pFigure->Count() > 20  // something big enough
        && _tcsstr(pLabel, _T("Contur")) == pLabel)
      {
        pCircleText = _tcsstr(pLabel, _T("blank_"));
        if (!pCircleText)
          bBackLight = (pCircleText = _tcsstr(pLabel, _T("back_"))) != NULL;
        if (pCircleText)
        {
          const cmplx * pPts = (cmplx*)pFigure->GetData();
          int iNPst = (int)pFigure->size();
          cmplx Center;
          double dRadius = 0., dDiam_pix = 0.;

          if (CircleFitting(pPts, iNPst, Center, dDiam_pix))
          {

            CircleData NewCircle(pLabel, dRadius = dDiam_pix / 2.,
              Center, pDataFrame->GetTime(), bBackLight);
            DWORD CircColor = 0xff0000;
            DWORD CentColor = 0xff0000;
            if (_tcsstr(pCircleText + 5, "_int") // this is internal circle
              || _tcsstr(pCircleText + 5, "light"))
            {
              CentColor = 0x0000ff;
              cInternal = Center;
              dRadInternal = dRadius;
              m_FoundIntCircles.push_back(NewCircle);
            }
            else if (_tcsstr(pCircleText + 5, "_ext")) // this is external circle
            {
              CentColor = 0x00ff00;
              cExternal = Center;
              dRadExternal = dRadius;
              m_FoundExtCircles.push_back(NewCircle);
            }
            pMarking->AddFrame(CreatePtFrame(Center,
              GetHRTickCount(), CentColor, "Cent", pDataFrame->GetId()));
            if (m_iViewMode > 7)
            {
              CFigureFrame * pCircle = CreateCircleView(Center, dRadius, CircColor);
              pMarking->AddFrame(pCircle);
            }
          }
        }
        else if (pCircleText = _tcsstr(pLabel, _T("circle_")))
        {
          int iNumberPos = (int)Label.ReverseFind(_T('_'));
          int iContNum = atoi((LPCTSTR(Label) + iNumberPos + 1));
          const cmplx * pPts = (cmplx*)pFigure->GetData();
          int iNPst = (int)pFigure->size();
          cmplx Center;
          double dRadius = 0., dDiam_pix = 0.;
          if (CircleFitting(pPts, iNPst, Center,
            dDiam_pix, m_LastSpots[iContNum].m_Area))
          {
            CircleData NewIntCircle(pLabel,
              dRadius = dDiam_pix / 2., Center, pDataFrame->GetTime());
            DWORD CircColor = 0xff0000;
            DWORD CentColor = 0xff0000;
            if (_tcsstr(pCircleText + 6, "_int")) // this is internal circle
            {
              CentColor = 0x0000ff;
              cBlankInt = Center;
              dBlankRadInt = dRadius;
              m_FoundIntCircles.push_back(NewIntCircle);
            }
            if (_tcsstr(pCircleText + 6, "_ext")) // this is external circle
            {
              CentColor = 0x00ff00;
              cBlankExt = Center;
              dBlankRadExt = dRadius;
              CircleData NewExtCircle(pLabel, dRadius, Center, pDataFrame->GetTime());
              m_FoundExtCircles.push_back(NewExtCircle);
            }
            pMarking->AddFrame(CreatePtFrame(Center,
              GetHRTickCount(), CentColor, "Cent", pDataFrame->GetId()));
            if (m_iViewMode > 7)
            {
              CFigureFrame * pCircle = CreateCircleView(Center, dRadius, CircColor);
              pMarking->AddFrame(pCircle);
            }
          }
        }
      }
    }
    delete pIter;
    if (m_iViewMode > 9 && dRadExternal && dRadInternal)
    {
      cmplx cResultPlace = cExternal + cmplx(-dRadExternal, dRadExternal);
      CTextFrame *pResult = CreateTextFrame(cResultPlace,
        "0xff00ff", 14, "Result", pDataFrame->GetId(),
        "Grinding Front Result:\n"
        "Eccent=%.2f\n"
        "IntDia=%.2f\n"
        "ExtDia=%.2f",
        abs(cInternal - cExternal) * m_dScale_um_per_pix,
        dRadInternal * m_dScale_um_per_pix * 2.,
        dRadExternal * m_dScale_um_per_pix * 2.);
      pMarking->AddFrame(pResult);
    }
    else if (m_iViewMode > 9 && dBlankRadExt && dBlankRadInt)
    {
      cmplx cResultPlace = cBlankExt + cmplx(-dBlankRadExt, dBlankRadExt);
      CTextFrame *pResult = CreateTextFrame(cResultPlace,
        "0xff00ff", 14, "Result", pDataFrame->GetId(),
        "Blank Qualification:\n"
        "Eccent=%.2f\n"
        "IntDia=%.2f\n"
        "ExtDia=%.2f",
        abs(cBlankInt - cBlankExt) * m_dScale_um_per_pix,
        dBlankRadInt * m_dScale_um_per_pix * 2.,
        dBlankRadExt * m_dScale_um_per_pix * 2.);
      pMarking->AddFrame(pResult);
    }

  }
  //    SENDERR( "failed to take the screenshot. err: %s\n" , (LPCTSTR)FxLastErr2Mes());
  return iRes;
}

int MPPT_Dispens::GetCircles(LPCTSTR pNamePrefix, CirclesVector& Results,
  const CDataFrame* pDataFrame, CContainerFrame * pMarking) // 
{
  int iRes = 0;
  const CVideoFrame * pVF = pDataFrame->GetVideoFrame();
  CFramesIterator * pIter = pDataFrame->CreateFramesIterator(figure);
  if (pIter)
  {
    CFigureFrame * pFigure = NULL;

    while (pFigure = (CFigureFrame*)pIter->Next())
    {
      LPCTSTR pLabel = pFigure->GetLabel();
      LPCTSTR pCircleText = NULL;
      if (pFigure->Count() > 20  // something big enough
        && _tcsstr(pLabel, _T("Contur")) == pLabel)
      {
        if (pCircleText = _tcsstr(pLabel, pNamePrefix))
        {
          iRes++;
          const cmplx * pPts = (cmplx*)pFigure->GetData();
          int iNPst = (int)pFigure->size();
          cmplx Center;
          double dRadius = 0.;
          if (CircleFitting(pPts, iNPst, Center, dRadius))
          {
            DWORD CircColor = 0xff0000;
            DWORD CentColor = 0xff0000;
            CircleData Circle(pCircleText, dRadius, Center, pVF ? pVF->GetTime() : 0.);
            Results.push_back(Circle);
            CircColor = 0xffff00;
            if (_tcsstr(pCircleText + 6, "_int")) // this is internal circle
              CentColor = 0x0000ff;
            else if (_tcsstr(pCircleText + 6, "_ext")) // this is external circle
              CentColor = 0x00ff00;
            else
            {
              CircColor = 0xff00ff;
              CentColor = 0xff00ff;
            }
            pMarking->AddFrame(CreatePtFrame(Center,
              GetHRTickCount(), CentColor, "Cent", pDataFrame->GetId()));
            CFigureFrame * pCircle = CFigureFrame::Create();
            pCircle->SetSize(360);
            for (int i = 0; i < 360; i++)
            {
              cmplx cPt = Center + dRadius * polar(1., i * M_PI / 180.);
              pCircle->SetAt(i, CmplxToCDPoint(cPt));
            }
            pCircle->Attributes()->WriteLong("color", CircColor);
            pMarking->AddFrame(pCircle);
          }
        }
      }
    }
    delete pIter;
  }
  return iRes;
}

double MPPT_Dispens::DoMoveX(double dDeltaX_pix)
{
  DoAdjustForCentering(CM_MoveX, dDeltaX_pix);

  m_dAccumulatedX += dDeltaX_pix;
  return m_dAccumulatedX;
};
double MPPT_Dispens::DoMoveY(double dDeltaY_pix)
{
  DoAdjustForCentering(CM_MoveY, dDeltaY_pix);

  m_dAccumulatedY += dDeltaY_pix;
  return m_dAccumulatedY;
};

int MPPT_Dispens::LightOn(int iMask)
{
  // Real switch on light bits


  m_iCurrentLightMask |= iMask;
  return m_iCurrentLightMask;
}

int MPPT_Dispens::LightOff(int iMask)
{
  // Real switch on light bits


  m_iCurrentLightMask &= ~iMask;
  return m_iCurrentLightMask;
}

int MPPT_Dispens::ProcessAkoTekContour( const CVideoFrame * pVF , CContainerFrame * pMarking )
{
  const CFigureFrame * pAkoTekContur = NULL;
  size_t iItCnt = 0;
  for ( ; iItCnt < m_LastFigures.size(); iItCnt++ )
  {
    const CFigureFrame * pFr = ( const CFigureFrame * ) m_LastFigures[ iItCnt ];
    LPCTSTR pLabel = pFr->GetLabel();
    if ( _tcsstr( pLabel , "Contur" ) && _tcsstr( pLabel , "ako_tek" ) )
    {
      pAkoTekContur = pFr;
      break ;
    }
  }

  FXRegistry Reg( "TheFileX\\MPP_Dispens" );

  const cmplx * pContur = (cmplx*)( pAkoTekContur->GetData() ) ;
  if ( pAkoTekContur )
  {
    int iUpLineIndex = -1 ;
    int iDownLineIndex = -1 ;
    cmplx cUpperInit , cLowerInit ;
    const cmplx * pEnd = pContur + pAkoTekContur->size() ;
    const cmplx * p = pContur ;
    while ( ++p < pEnd )
    {
      if ( fabs( p->real() - m_cLastROICent_pix.real() ) < 0.5 )
      {
        if ( p->imag() <= m_cLastROICent_pix.imag() )
        {
          iUpLineIndex = (int)(p - pContur) ;
          cUpperInit = *p ;
          pMarking->AddFrame( CreatePtFrame( cUpperInit , "color=0x0000ff; Sz=5;" ) );
        }
        else
        {
          iDownLineIndex = ( int ) ( p - pContur ) ;
          cLowerInit = *p ;
          pMarking->AddFrame( CreatePtFrame( cLowerInit , "color=0x0000ff; Sz=5;" ) );
        }
        if ( (iDownLineIndex > 0) && (iUpLineIndex > 0) )
          break ;
        p += 100 ;
      }
    }
    if ( ( iDownLineIndex < 0 ) || ( iUpLineIndex < 0 ) )
      return 0 ;

    cmplx cUpperLeft , cUpperRight , cLowerLeft , cLowerRight ; 
    // Extract upper horizontal line
    int iMinUpIndex = 0 , iMaxUpIndex = 0 ;
    if ( ExtractStraightSegment( pContur , ( int ) pAkoTekContur->size() ,
      iUpLineIndex , 100 , iMinUpIndex , iMaxUpIndex , 2. ) )
    {
      cUpperLeft = pContur[ iMinUpIndex + 5 ] ;
      cUpperRight = pContur[ iMaxUpIndex - 5 ] ;
      pMarking->AddFrame( CreatePtFrame( cUpperLeft , "color=0x0000ff; Sz=3;" ) );
      pMarking->AddFrame( CreatePtFrame( cUpperRight , "color=0x0000ff; Sz=3;" ) );
      pMarking->AddFrame( CreateLineFrame( cUpperLeft , cUpperRight , 0x00ff00 ) );
    }
    else
      return 0 ;

    // Extract lower horizontal line
    int iMinDownIndex = 0 , iMaxDownIndex = 0 ;
    if ( ExtractStraightSegment( pContur , ( int ) pAkoTekContur->size() ,
      iDownLineIndex , 100 , iMinDownIndex , iMaxDownIndex , 2. ) )
    {
      cLowerLeft = pContur[ iMaxDownIndex - 5 ];
      cLowerRight = pContur[ iMinDownIndex + 5 ];
      pMarking->AddFrame( CreatePtFrame( cLowerLeft , "color=0x0000ff; Sz=3;" ) );
      pMarking->AddFrame( CreatePtFrame( cLowerRight , "color=0x0000ff; Sz=3;" ) );
      pMarking->AddFrame( CreateLineFrame( cLowerRight , cLowerLeft , 0x00ff00 ) );
    }
    else
      return 0 ;

    cmplx cUpperCent = ( cUpperLeft + cUpperRight ) / 2. ;
    cmplx cViewPt = cUpperCent - cmplx( 50. , 50. ) ;
    cmplx cLowerCent = ( cLowerLeft + cLowerRight ) / 2. ;
    double dMeasuredDia_pix = ( cLowerCent.imag() - cUpperCent.imag() ) ;
    double dMeasuredDia_um = dMeasuredDia_pix * m_dScale_um_per_pix ;

    pMarking->AddFrame( CreateTextFrameEx( cViewPt , 0x004000 , 14 , "Dia=%.2fum" ,
      dMeasuredDia_um ) ) ;

    CLine2d UpperLine( cUpperLeft , cUpperRight ) ;
    CLine2d LowerLine( cLowerLeft , cLowerRight ) ;
//     CLine2d BiSector = LowerLine.GetBisector( UpperLine ) ;
//     double dBisectorAngle = BiSector.get_angle() ;

    double dBisectorLeftY = 0.5 * (UpperLine.GetY( 10. ) + LowerLine.GetY( 10. ));
    double dBisectorRightY = 0.5 * ( UpperLine.GetY( m_LastFOV.cx - 10. ) + LowerLine.GetY( m_LastFOV.cx - 10. ) ); ;

    cmplx cBisectorLeftPt( 10. , dBisectorLeftY ) ;
    cmplx cBisectorRightPt( m_LastFOV.cx - 10. , dBisectorRightY ) ;
    cmplx cBisectorVect = cBisectorRightPt - cBisectorLeftPt ;

    pMarking->AddFrame( CreateLineFrame( cBisectorLeftPt , cBisectorRightPt , 0x00ff00 ) );

    CLine2d BiSector( cBisectorLeftPt , cBisectorRightPt ) ;
    double dMinDistToBisector = DBL_MAX ;
    int iMinDistIndex = iMaxUpIndex ;
    for ( int i = iMaxUpIndex + 1 ; i < iMinDownIndex ; i++ )
    {
      double dDistToBisector = abs( BiSector.GetDistFromPoint( pContur[ i ] ) ) ;
      if ( dDistToBisector < dMinDistToBisector )
      {
        dMinDistToBisector = dDistToBisector ;
        iMinDistIndex = i ;
      }
    }

    cmplx dCenterPt = pContur[ iMinDistIndex ] ;

    cmplx cCent_pix ;
    double dDia_pix ;
    //int iHalfRange = ROUND(m_dCalibODia_um/2.) ;
    int iHalfRange = ROUND( dMeasuredDia_pix * 0.5 ) ;
    bool bRes = CircleFitting( pContur + iMinDistIndex - iHalfRange , 2 * iHalfRange + 1 , cCent_pix , dDia_pix );

    pMarking->AddFrame( CreateCircleView( cCent_pix , dDia_pix/2. , 0xffff00 ) ) ;
    pMarking->AddFrame( CreatePtFrame( cCent_pix , "color=0x00ffff; Sz=5;" ) ) ;
    cViewPt = ( cCent_pix += cmplx( -100. , 60. )) ;
    pMarking->AddFrame( CreateTextFrame( cCent_pix , "0xffff00" , 14 , NULL , NULL , 
      "C(%.2f,%.2f)\nR=%.2fum" , cCent_pix.real() , cCent_pix.imag() , m_dScale_um_per_pix * dDia_pix/2. ) ) ;

    cmplx cUpperToLowerInitVect = cLowerInit - cUpperInit ;
    cmplx cLeftUpperForLength = cBisectorLeftPt - cUpperToLowerInitVect * 0.55 ;
    double dBisectorLen = abs( cBisectorVect ) ;
    cmplx cStep = cBisectorVect/dBisectorLen ;

    double Signal[2000] ;

    cmplx cUpperEdge = GetThresCrossOnLine( cLeftUpperForLength , cStep , dBisectorLen ,
      Signal , 2000 , pVF , 128. ) ;
    cmplx cUpperRightEdge = cUpperEdge + cBisectorVect ;

    CLine2d UpperMeasLine( cUpperEdge , cUpperRightEdge ) ;
    cmplx cOrthoCross ;
    UpperMeasLine.GetOrthoCross( dCenterPt , cOrthoCross ) ;

    pMarking->AddFrame( CreatePtFrame( cUpperEdge , "color=0xff0000; Sz=3;" ) ) ;
    pMarking->AddFrame( CreatePtFrame( cOrthoCross , "color=0xff0000; Sz=3;" ) ) ;
    pMarking->AddFrame( CreateLineFrame( cUpperEdge , cOrthoCross , 0xff0000 ) );
    pMarking->AddFrame( CreateLineFrame( dCenterPt , cOrthoCross , 0xff0000 ) );
    cmplx cLenViewPt = cOrthoCross + cmplx( -50. , -50. ) ;
    pMarking->AddFrame( CreateTextFrame( cLenViewPt , "0xff0000" , 14 , NULL , NULL ,
      "L=%.2fum" , abs( cOrthoCross - cUpperEdge ) * m_dScale_um_per_pix ) ) ;

    cmplx cLeftLowerForLength = cBisectorLeftPt + cUpperToLowerInitVect * 0.55 ;

    cmplx cLowerEdge = GetThresCrossOnLine( cLeftLowerForLength , cStep , dBisectorLen ,
      Signal , 2000 , pVF , 128. ) ;
    cmplx cLowerRightEdge = cLowerEdge + cBisectorVect ;

    CLine2d LowerMeasLine( cLowerEdge , cLowerRightEdge ) ;
    LowerMeasLine.GetOrthoCross( dCenterPt , cOrthoCross ) ;

    pMarking->AddFrame( CreatePtFrame( cLowerEdge , "color=0xff0000; Sz=3;" ) ) ;
    pMarking->AddFrame( CreatePtFrame( cOrthoCross , "color=0xff0000; Sz=3;" ) ) ;
    pMarking->AddFrame( CreateLineFrame( cLowerEdge , cOrthoCross , 0xff0000 ) );
    pMarking->AddFrame( CreateLineFrame( dCenterPt , cOrthoCross , 0xff0000 ) );
    cLenViewPt = cOrthoCross + cmplx( -50. , 50. ) ;
    pMarking->AddFrame( CreateTextFrame( cLenViewPt , "0xff0000" , 14 , NULL , NULL ,
      "L=%.2fum" , abs( cOrthoCross - cLowerEdge ) * m_dScale_um_per_pix ) ) ;

  }

  return 0;
}

int MPPT_Dispens::ProcessSideContour(CContainerFrame * pMarking)
{
  cmplx Corner1, Corner2; // right vertical edge

  const CFigureFrame * pSideContur = NULL;
  const CFigureFrame * pStoneContur = NULL;
  size_t iItCnt = 0;
  for (; iItCnt < m_LastFigures.size(); iItCnt++)
  {
    const CFigureFrame * pFr = (const CFigureFrame *)m_LastFigures[iItCnt];
    LPCTSTR pLabel = pFr->GetLabel();
    if (_tcsstr(pLabel, "Contur") && _tcsstr(pLabel, "side_view"))
    {
      pSideContur = pFr;
    }
    if (_tcsstr(pLabel, "Contur") && _tcsstr(pLabel, "side_stone"))
      pStoneContur = pFr;
    if (pSideContur && pStoneContur)
      break;
  }

  FXRegistry Reg("TheFileX\\MPP_Dispens");

  if (pSideContur)
  {
    CColorSpot * pSideConturData = GetSpotData("side_view");
    if (pSideConturData)
    {
      cmplx cConusCenter = CDPointToCmplx(pSideConturData->m_SimpleCenter);
      if (cConusCenter.imag() < m_cMainCrossCenter.imag() - m_cLastROICent_pix.imag() / 2.)
        pSideContur = NULL;
      else if (pStoneContur)
      {
        CColorSpot * pSideStoneData = GetSpotData("side_stone");
        if (pSideStoneData)
        {
          cmplx cDistBetweenCenters =
            CDPointToCmplx(pSideConturData->m_SimpleCenter)
            - CDPointToCmplx(pSideStoneData->m_SimpleCenter);
          if (abs(cDistBetweenCenters) < 10.)
            pStoneContur = NULL;
        }
      }
      // check for side contour central point is near center
      if (pSideContur)
      {
        cmplx * pcContur = (cmplx*)(pSideContur->GetData());
        cmplx cTmp;
        CmplxArray Extrems;
        FXIntArray Indexes;
        cmplx cConturSize;
        cmplx cSideConturCenter = FindExtrems(pSideContur, Extrems, &Indexes, &cConturSize);

        double dYCenterByExtrems = 0.5 * (Extrems[EXTREME_INDEX_TOP].imag() + Extrems[EXTREME_INDEX_BOTTOM].imag());
        double dYMasterCenter = 0.5 * (m_cCalibSideUpperRightCorner.imag() + m_cCalibSideLowerRightCorner.imag());
        double dYCentersDiff = dYCenterByExtrems - dYMasterCenter;
        if ( fabs( dYCentersDiff ) < 70. ) // No processing if cone is not symmetrical to master
        {
          double dConusHalfAngle = 0.5 * (m_CurrentPart.m_dConuseAngleMin_deg + m_CurrentPart.m_dConuseAngleMax_deg) / 2.;
          //GetCenterForFigure(  pcContur , (int)pSideContur->GetCount() , cTmp , Moments ) ;

          int iCentIndex = FindNearestToYIndex(pSideContur, cSideConturCenter, true); // find right center pt on contour

          cmplx * pcIter = pcContur + Indexes[EXTREME_INDEX_TOP];
          int iStep = pcIter->real() < (pcIter + 1)->real() ? 1 : -1;
          cmplx cTopLeft = *(pcIter++);
          double dRightLimit = cSideConturCenter.real() - 10.;
          CFRegression UpperConeSideRegression;
          while (pcIter->real() < dRightLimit)
          {
            UpperConeSideRegression.Add(*pcIter);
            pcIter += iStep;
            if (pcIter - pcContur > pSideContur->GetCount())
              pcIter = pcContur;
            else if (pcIter < pcContur)
              pcIter = pcContur + pSideContur->GetCount() - 1;
          }

          if (UpperConeSideRegression.Size() > 20)
          {
            cmplx cUpperRegrEnd = *(pcIter - 1);
            UpperConeSideRegression.Calculate();
            CLine2d UpperSideLine = UpperConeSideRegression.GetCLine2d();

            cmplx cRightPtOnUpperLine =
              UpperSideLine.GetPtForX(Extrems[EXTREME_INDEX_RIGHT].real());

            if ((cRightPtOnUpperLine.imag() < m_LastFOV.cy * 0.667)
              && (cRightPtOnUpperLine.imag() > 10.)
              && (cRightPtOnUpperLine.real() > 10.)
              && (cRightPtOnUpperLine.real() < m_LastFOV.cx * 0.8))
            {
              pMarking->AddFrame(CreateLineFrame(cTopLeft, cRightPtOnUpperLine,
                0xff0000, "UpperConeEdge"));
              pMarking->AddFrame(CreatePtFrame(cTopLeft, "color=0xff0000; Sz=5;"));
              pMarking->AddFrame(CreatePtFrame(cRightPtOnUpperLine, "color=0xff0000; Sz=5;"));

              while (pcIter->imag() < cRightPtOnUpperLine.imag() + 1.)
              {
                pcIter += iStep;
                if (pcIter - pcContur > pSideContur->GetCount())
                  pcIter = pcContur;
                else if (pcIter < pcContur)
                  pcIter = pcContur + pSideContur->GetCount() - 1;
              }

              CFRegression RightConeSideRegression;
              double dBottomLimit = 2. * cSideConturCenter.imag() - cRightPtOnUpperLine.imag() - 10.;
              while (pcIter->imag() < dBottomLimit)
              {
                RightConeSideRegression.Add(*pcIter);
                pcIter += iStep;
                if (pcIter - pcContur > pSideContur->GetCount())
                  pcIter = pcContur;
                else if (pcIter < pcContur)
                  pcIter = pcContur + pSideContur->GetCount() - 1;
              }
              if (RightConeSideRegression.Size() > 20)
              {
                RightConeSideRegression.Calculate();
                CLine2d RightLine = RightConeSideRegression.GetCLine2d();
                pMarking->AddFrame(CreateLineFrame(*(pcIter - iStep), cRightPtOnUpperLine,
                  0xff0000, "UpperConeEdge"));
                pMarking->AddFrame(CreatePtFrame(*(pcIter - iStep), "color=0xff0000; Sz=5;"));

                while (pcIter->real() > dRightLimit - 20.)
                {
                  pcIter += iStep;
                  if (pcIter - pcContur > pSideContur->GetCount())
                    pcIter = pcContur;
                  else if (pcIter < pcContur)
                    pcIter = pcContur + pSideContur->GetCount() - 1;
                }

                CFRegression LowerConeSideRegression;
                double dLeftLimit = Extrems[EXTREME_INDEX_LEFT].real() + 10.;
                while (pcIter->real() > dLeftLimit)
                {
                  LowerConeSideRegression.Add(*pcIter);
                  pcIter += iStep;
                  if (pcIter - pcContur > pSideContur->GetCount())
                    pcIter = pcContur;
                  else if (pcIter < pcContur)
                    pcIter = pcContur + pSideContur->GetCount() - 1;
                }
                pMarking->AddFrame(CreatePtFrame(*(pcIter - iStep), "color=0xff0000; Sz=5;"));
                if (LowerConeSideRegression.Size() > 20)
                {
                  LowerConeSideRegression.Calculate();
                  CLine2d LowerConeLine = LowerConeSideRegression.GetCLine2d();
                  if (RightLine.intersect(UpperSideLine, Corner1)  // upper corner
                    && RightLine.intersect(LowerConeLine, Corner2))
                  {
                    if (Corner1.imag() > Corner2.imag())
                      swap(Corner1, Corner2);

                    m_cSideUpperLeftCorner = cTopLeft;
                    m_cSideUpperRightCorner = Corner1;
                    m_cSideLowerRightCorner = Corner2;
                    m_cSideLowerLeftCorner = *(pcIter - iStep);
                    m_dLastUpperConusAngle_deg = RadToDeg(UpperSideLine.get_angle()) - 180.;
                    m_dLastLowerConusAngle_deg = RadToDeg(LowerConeLine.get_angle()) + 180.;
                    cmplx cRightEdgeCenter = (Corner1 + Corner2) * 0.5;
                    cmplx ViewPt(cRightEdgeCenter + cmplx(-200., 220.));
                    m_dLastTipOnSideDia_um = abs(Corner1 - Corner2) * m_dScale_um_per_pix;
                    m_dLastTipOnSideMeasTime_ms = GetHRTickCount();
                    m_cMasterCenter_pix = 0.5 *(m_cCalibSideUpperRightCorner + m_cCalibSideLowerRightCorner);
                    double dHorizDistFromCurrentToMaster_pix = (cRightEdgeCenter.real() - m_cMasterCenter_pix.real());
                    m_dDistFromLastVertEdgeToMasterVertEdge_um = dHorizDistFromCurrentToMaster_pix * m_dScale_um_per_pix;
                    m_dFrontScaleCorrectionCoeff = 1.
                      - (m_dDistFromLastVertEdgeToMasterVertEdge_um / m_dFrontLensWorkingDistance_um);
                    double dStoneToConusAngle = pStoneContur ?
                      m_dLastStoneAngle_deg - m_dLastUpperConusAngle_deg : 360.;
                    m_CurrentPart.m_dResultConuseAngle_deg = 
                      m_dLastLowerConusAngle_deg - m_dLastUpperConusAngle_deg;
                    double dFrontAngle_deg = RadToDeg(arg(m_cSideLowerRightCorner - m_cSideUpperRightCorner));
                    if (pStoneContur)
                    {
                      pMarking->AddFrame(CreateTextFrame(ViewPt, "0x0000ff",
                        20, "Result", pMarking->GetId(),
                        "Cone Dia %.2f um\nConus Angle=%.2f[%.2f,%.2f]\n"
                        "Stone-Conus Angle=%.2f\n"
                        "FrontAngle_deg=%.2f",
                        m_dLastTipOnSideDia_um, m_CurrentPart.m_dResultConuseAngle_deg,
                        m_dLastUpperConusAngle_deg, m_dLastLowerConusAngle_deg,
                        fabs( dStoneToConusAngle) >45 ? 0. : dStoneToConusAngle , 
                        dFrontAngle_deg ));
                    }
                    else
                    {
                      pMarking->AddFrame(CreateTextFrame(ViewPt, "0x0000ff",
                        20, "Result", pMarking->GetId(),
                        "Cone Dia %.2f um\nConus Angle=%.2f[%.2f,%.2f]\n"
                        "FrontAngle_deg=%.2f",
                        m_dLastTipOnSideDia_um, m_CurrentPart.m_dResultConuseAngle_deg,
                        m_dLastUpperConusAngle_deg, m_dLastLowerConusAngle_deg , dFrontAngle_deg));
                      if (m_WorkingStage == STG_SideFinalMeasurement)
                      {
                        int iNAccumulated = m_CurrentPart.AddToSide(
                          m_dLastTipOnSideDia_um, m_CurrentPart.m_dResultConuseAngle_deg);
                        if (iNAccumulated >= 20)
                          SetIdleStage(); // measurement finished
                      }
                    }
                    cmplx PartDirection = polar(200., DegToRad(m_dLastUpperConusAngle_deg + m_dLastLowerConusAngle_deg));

                    pMarking->AddFrame(CreatePtFrame(m_cSideUpperRightCorner, "color=0x0000ff; Sz=5;"));
                    pMarking->AddFrame(CreatePtFrame(m_cSideLowerRightCorner, "color=0x0000ff; Sz=5;"));
                    pMarking->AddFrame(CreateLineFrame(m_cSideUpperRightCorner, m_cSideUpperLeftCorner,
                      0xff0000, "ConeEdge"));
                    pMarking->AddFrame(CreateLineFrame(m_cSideLowerLeftCorner, m_cSideLowerRightCorner,
                      0xff0000, "ConeEdge"));
                    cmplx cFrontVect = m_cSideLowerRightCorner - m_cSideUpperRightCorner;
                    double dFrontLength_pix = abs(cFrontVect);
                    double dFront_um = dFrontLength_pix * m_dScale_um_per_pix;
                    double dScale_um_per_pix = m_dCalibODia_um / dFrontLength_pix;

                    cmplx cOrthoToFront = GetOrthoLeft(cFrontVect);
                    cmplx cOrthoEnd1 = cRightEdgeCenter + (2. * cOrthoToFront);
                    cmplx cOrthoEnd2 = cRightEdgeCenter - (2. * cOrthoToFront);
                    pMarking->AddFrame(CreateLineFrame(cOrthoEnd1, cOrthoEnd2,
                      0xff00ff, "OrthToFront"));
                    switch (m_WorkingStage)
                    {
                    case STG_SideCalibration:
                      {
                        m_dScale_um_per_pix = dScale_um_per_pix;
                        Reg.WriteRegiDouble(
                          "Calibrations", "SideScale_um_per_pixel", m_dScale_um_per_pix);
                        Reg.WriteRegiDouble(
                          "Calibrations", "CalibExternalDia_um", m_dCalibODia_um);
                        FXString Msg;
                        Msg.Format("Finished;//Side camera Scale=%.6f Dia=%.2f",
                          m_dScale_um_per_pix, m_dCalibODia_um);
                        SendMessageToEngine(Msg, "CalibFinishedToEngine");

                        m_cCalibSideUpperRightCorner = m_cSideUpperRightCorner;
                        m_cCalibSideLowerRightCorner = m_cSideLowerRightCorner;
                        m_cCalibSideUpperLeftCorner = m_cSideUpperLeftCorner;
                        m_cCalibSideLowerLeftCorner = m_cSideLowerLeftCorner;

                        cmplx cUpperSideVector = m_cSideUpperRightCorner - m_cSideUpperLeftCorner;
                        m_LastPartUpperSideLine.Update(m_cSideUpperLeftCorner, m_cSideUpperRightCorner);
                        double dDistToStone_pix = m_dStoneToMasterDistance_um / m_dScale_um_per_pix;
                        cmplx cToStoneDist = -GetOrthoLeft(GetNormalized(cUpperSideVector))
                          * dDistToStone_pix;
                        m_cLeftTopStoneTargetMark = m_cSideUpperLeftCorner + cToStoneDist;
                        m_cRightTopStoneTargetMark = m_cSideUpperRightCorner + cToStoneDist;
                        m_LastStoneTargetLine.Update(m_cLeftTopStoneTargetMark, m_cRightTopStoneTargetMark);
                        pMarking->AddFrame(CreateLineFrame(m_cLeftTopStoneTargetMark, m_cRightTopStoneTargetMark,
                          0xff0000, "StoneTarget"));

                        Reg.WriteRegiCmplx("Calibrations",
                          "CalibSideUpperRightCorner", m_cCalibSideUpperRightCorner);
                        Reg.WriteRegiCmplx("Calibrations",
                          "CalibSideLowerRightCorner", m_cCalibSideLowerRightCorner);
                        Reg.WriteRegiCmplx("Calibrations",
                          "CalibSideUpperLeftCorner", m_cCalibSideUpperLeftCorner);
                        Reg.WriteRegiCmplx("Calibrations",
                          "CalibSideLowerLeftCorner", m_cCalibSideLowerLeftCorner);
                        Reg.WriteRegiCmplx("Calibrations",
                          "LeftTopStoneTargetMark", m_cLeftTopStoneTargetMark);
                        Reg.WriteRegiCmplx("Calibrations",
                          "RightTopStoneTargetMark", m_cRightTopStoneTargetMark);

                        SetIdleStage();
                      }
                      break;
                    case STG_SideFineGrinding:
                      {
                        double dFineSideWaitTime_ms = Reg.GetRegiDouble(
                          "Motions", "WaitAfterFineGrindCycle_ms", 2000.);
                        double dTipTargetCorrection = Reg.GetRegiDouble(
                          "Motions", "TipTargetCorrection_um", 0.);
                        double dTimeAfterMotionFinished = GetHRTickCount() - m_dLastMotionFinishedTime;
                        m_dLastTipOnSideDia_um = m_dLastTipOnSideDia_um == 0. ?
                          dFront_um : min(m_dLastTipOnSideDia_um, dFront_um);
                        if (m_dSideTipDiaBeforeFineGrinding_um == 0.)
                          m_dSideTipDiaBeforeFineGrinding_um = m_dLastTipOnSideDia_um;

                        bool bTooFigFrontAngleDeviation = (fabs(dFrontAngle_deg - 90.) > 0.5);
                        if (m_bDone && (dTimeAfterMotionFinished > dFineSideWaitTime_ms)
                          && !bTooFigFrontAngleDeviation ) // ms
                        {
                          FXString Msg;
                          double dTarget_um = m_CurrentPart.GetNominalTipDia_um();
                          double dStoneTipOverSize = m_dLastTipOnSideDia_um - dTarget_um + dTipTargetCorrection;
                          double dGrindedDepth_um = m_dSideTipDiaBeforeFineGrinding_um - m_dLastTipOnSideDia_um;
                          double dMinGrindedDepth_um = Reg.GetRegiDouble(
                            "Motions", "MinGrindedDepth_um", 8.);
                          if (dStoneTipOverSize > 10.)
                          {
                            Msg.Format("Medium;// OD_um=%.2f(%.2f) Target=%.2f Diff=%.2f FrontAng=%.2fdeg",
                              m_dLastTipOnSideDia_um, dFront_um, dTarget_um, dStoneTipOverSize , dFrontAngle_deg );
                          }
                          else if (dStoneTipOverSize > 1.)
                          {
                            Msg.Format("Small;// OD_um=%.2f(%.2f) Target=%.2f Diff=%.2fFrontAng=%.2fdeg",
                              m_dLastTipOnSideDia_um, dFront_um, dTarget_um, dStoneTipOverSize , dFrontAngle_deg);
                          }
                          else if ((dGrindedDepth_um < dMinGrindedDepth_um)
                            && (m_dLastTipOnSideDia_um > m_CurrentPart.m_dODmin_um + 3.))
                          {
                            Msg.Format("Small;// OD_um=%.2f(%.2f) Target=%.2f Diff=%.2f FrontAng=%.2fdeg Too small grinded depth",
                              m_dLastTipOnSideDia_um, dFront_um, dTarget_um, dStoneTipOverSize, dFrontAngle_deg);
                          }
                          else 
                          {
                            int iNAccumulated = m_CurrentPart.AddToSide(
                              m_dLastTipOnSideDia_um, m_CurrentPart.m_dResultConuseAngle_deg);
                            if (iNAccumulated > 10)
                            {
                              Msg.Format("Finished;// OD_um=%.2f(%.2f) Target=%.2f Diff=%.2f",
                                m_dLastTipOnSideDia_um, dFront_um, dTarget_um, dStoneTipOverSize);
                              SetIdleStage();
                              m_LastResults.m_dPartDOAfterFine_um = m_dLastTipOnSideDia_um;
                            }
                          }
                          m_dLastTipOnSideDia_um = 0.;
                          if (!Msg.IsEmpty())
                          {
                            SendMessageToEngine(Msg, NULL);
                            m_bDone = false;
                          }
                        }
                        else
                        {
                          m_CurrentPart.ResetSideResults();
                          if ( bTooFigFrontAngleDeviation )
                          {
                            pMarking->AddFrame(CreateTextFrame(ViewPt, "0x0000ff",
                              20, "AngleDeviationResult", pMarking->GetId(),
                              "\n\n\n\nFrontdX_um=%.2f", cFrontVect.real() * m_dScale_um_per_pix
                            ));
                          }
                        }
                      }
                      break;
                    }
                  }
                }
              }
            }
          }
        }
      }
    }
  }
  if ((Corner1.real() == 0.) && (Corner2.real() == 0.))
  {
    cmplx ViewPt(cmplx(m_cLastROICent_pix.real() * 0.5, m_cLastROICent_pix.imag() * 1.5));
    pMarking->AddFrame(CreateTextFrame(ViewPt, "0x0000ff",
      20, "Result", pMarking->GetId(), "Can't measure Cone"));
  }

  if (pStoneContur)
  {
    CmplxVector Extrems;
    IntVector Indexes;
    cmplx cStoneCenter = FindExtrems(pStoneContur, Extrems, &Indexes);
    cmplx * pcAsCmplx = (cmplx*)pStoneContur->GetData();


    CFRegression StoneDownRegression;
    cmplx cBottom = Extrems[EXTREME_INDEX_BOTTOM];
    cmplx cRight = Extrems[EXTREME_INDEX_RIGHT];
    cmplx cLeft = Extrems[EXTREME_INDEX_LEFT];
    cmplx cTop = Extrems[EXTREME_INDEX_TOP];


    cmplx * pcIter = pcAsCmplx + Indexes[EXTREME_INDEX_BOTTOM];
    int iStep = (pcIter->real() > (pcIter + 10)->real()) ? 1 : -1;
    double dLeftEdge = cLeft.real() + 200.;
    if (cBottom.real() < cRight.real() - 200.) // case, when stone is fully on left side
      dLeftEdge = cLeft.real() + 5.;

    while (pcIter->real() > dLeftEdge)
    {
      StoneDownRegression.Add(*pcIter);
      pcIter = pcIter + iStep;
    }
    //     cmplx EdgesViewPt(100. , 1200.);
    //     pMarking->AddFrame(CreateTextFrame(EdgesViewPt, "0x0000ff",
    //       14, "StoneResult", pMarking->GetId(),
    //       "Left[%.1f,%.1f]\nTop[%.1f,%.1f]\nRight[%.1f,%.1f]\nBottom[%.1f,%.1f]\n"
    //       "ItLeft[%.1f,%.1f] - ItRight[%.1f,%.1f]",
    //       cLeft, cTop , cRight , cBottom , 
    //       *(pcIter - iStep) , *(pcAsCmplx + Indexes[EXTREME_INDEX_BOTTOM]))) ;

    if (StoneDownRegression.Size() > 10)
    {
      StoneDownRegression.Calculate();
      cmplx * pcContur = (cmplx*)(pStoneContur->GetData());
      CLine2d Line = StoneDownRegression.GetCLine2d();
      m_cStoneRightPt = Line.GetPtForX(cRight.real() - 5.);
      m_cStoneLeftPt = Line.GetPtForX(dLeftEdge);
      pMarking->AddFrame(CreatePtFrame(m_cStoneLeftPt, "color=0xffff00; Sz=5;"));
      pMarking->AddFrame(CreatePtFrame(m_cStoneRightPt, "color=0xffff00; Sz=5;"));
      pMarking->AddFrame(CreateLineFrame(m_cStoneLeftPt, m_cStoneRightPt,
        0xffff00, "StoneEdge"));
      //       m_dLastStoneAngle_deg = -RadToDeg( arg( m_cStoneRightPt - m_cStoneLeftPt ) ) ;
      //       cmplx ViewPt( m_SideStoneROI.CenterPoint().x - 40. , m_SideStoneROI.top + 10. ) ;
      //       pMarking->AddFrame( CreateTextFrame( ViewPt , "0xffff00" ,
      //         20 , "StoneResult" , pMarking->GetId() ,
      //         "Stone Angle %.2f deg" , m_dLastStoneAngle_deg ) ) ;

      m_LastStoneEdgeLine.Update(m_cStoneLeftPt, m_cStoneRightPt);
      cmplx cStoneEdgeCent = (m_cStoneLeftPt + m_cStoneRightPt) * 0.5;
      cmplx cStoneVect = m_cStoneRightPt - m_cStoneLeftPt;
      cmplx cOrthoToStoneEdge = GetOrthoRight(cStoneVect);
      CLine2d cOrthoLineFromCenter;

      cOrthoLineFromCenter.ByPointAndDir(cStoneEdgeCent, cOrthoToStoneEdge);
      cmplx cOrthoCross;
      m_LastPartUpperSideLine.intersect(cOrthoLineFromCenter, cOrthoCross);
      m_dLastUpperConusAngle_deg = RadToDeg(m_LastPartUpperSideLine.get_angle()) - 180.;
      cmplx cOrthoVect = cStoneEdgeCent - cOrthoCross;
      double dOrthoAngle_deg = arg(cOrthoVect);
      m_dLastStoneToPartSideDist_um = abs(cOrthoVect) * m_dScale_um_per_pix;
      if (cStoneEdgeCent.imag() > cOrthoCross.imag())
        m_dLastStoneToPartSideDist_um = -m_dLastStoneToPartSideDist_um;
      if (m_dLastStoneToPartSideDist_um < m_dLastMinStoneToPartDist_um)
        m_dLastMinStoneToPartDist_um = m_dLastStoneToPartSideDist_um;
      pMarking->AddFrame(CreateLineFrame(cOrthoCross, cStoneEdgeCent,
        0xffff00, "StoneEdge"));
      cmplx cDistViewPt((cOrthoCross + cStoneEdgeCent) * 0.5 + cmplx(20., 0.));
      pMarking->AddFrame(CreateTextFrame(cDistViewPt, "0x00000ff",
        13, "StoneResult", pMarking->GetId(),
        "DistToPart=%.2f um\nMinDistToPart=%.2f",
        m_dLastStoneToPartSideDist_um, m_dLastMinStoneToPartDist_um));

      m_dLastStoneAngle_deg = -RadToDeg(arg(m_cStoneRightPt - m_cStoneLeftPt));
      cmplx ViewPt(m_SideStoneROI.CenterPoint().x - 40., m_SideStoneROI.top + 10.);
      pMarking->AddFrame(CreateTextFrame(ViewPt, "0xffff00",
        20, "StoneResult", pMarking->GetId(),
        "Stone Angle = %.2f deg\nStoneToPart = %.2fdeg\n"
        "DistToPart=%.2f um\nMinDistToPart=%.2f",
        m_dLastStoneAngle_deg,
        m_dLastStoneAngle_deg - m_dLastUpperConusAngle_deg,
        m_dLastStoneToPartSideDist_um, m_dLastMinStoneToPartDist_um));

      m_dLastStoneToTargetDist_um =
        m_dLastStoneToPartSideDist_um - m_dStoneToMasterDistance_um;

      m_bStoneIsMeasured = true;
      int iLargeMoveTime_ms = Reg.GetRegiInt(
        "Motions", "CoarseLargeMotion_ms", 1000);
      double dLargeMoveDistForCoarse400ms_um = Reg.GetRegiDouble(
        "Motions", "LargeMoveDistForCoarse400ms_um", 200.);
      switch (m_WorkingStage)
      {
      case STG_FineStoneMeas:
        {
          if (m_bDone && ((GetHRTickCount() - m_dLastMotionFinishedTime) > 600.))
          {
            if (!m_bSideStoneInitialAdjustmentIsFinished)
            {
              if (m_dLastStoneToPartSideDist_um > 350.)
              {
                iLargeMoveTime_ms = Reg.GetRegiInt(
                  "Motions", "FineLargeMotion_ms", 200);
                FXString LargeMsg;
                LargeMsg.Format("Large=%d;//Dist=%.2f", iLargeMoveTime_ms, m_dLastStoneToPartSideDist_um);
                SendMessageToEngine(LargeMsg, NULL);
                m_iLastStoneMovingTime_ms = iLargeMoveTime_ms;
              }
              else
              {
                // Until what distance to use medium button
                // default is 70 microns above target line for coarse stone
                double dRangeForMediumMoveFineStone_um = Reg.GetRegiDouble(
                  "Motions", "MediumMoveFineStoneThres_um", 70.);
                // Until what distance to use "Small" button 
                // default is 5 microns below target line for coarse stone
                double dRangeForSmallMoveFineStone_um = Reg.GetRegiDouble(
                  "Motions", "SmallMoveFineStoneThres_um", -5.);
                // NO REWIND!!! 
                // When to use "Rewind" button  (i.e.pull fine stone back)
                // default is 25 microns below target line for coarse stone
  //               double dThresForFineStoneRewind_um = Reg.GetRegiDouble(
  //                 "Motions" , "RewindFineStoneThres_um" , -25. );

                if (m_dLastStoneToPartSideDist_um >
                  m_dLastCoarseSideAdjustedPos + dRangeForMediumMoveFineStone_um)
                {
                  FXString Msg;
                  Msg.Format("Medium;//FineDist=%.2f Coarse=%.2f", m_dLastStoneToPartSideDist_um,
                    m_dLastCoarseSideAdjustedPos);
                  SendMessageToEngine(Msg, NULL);
                }
                else if (m_dLastStoneToPartSideDist_um >
                  m_dLastCoarseSideAdjustedPos + dRangeForSmallMoveFineStone_um)
                {
                  FXString Msg;
                  Msg.Format("Small;//FineDist=%.2f Coarse=%.2f", m_dLastStoneToPartSideDist_um,
                    m_dLastCoarseSideAdjustedPos);
                  SendMessageToEngine(Msg, NULL);
                }
                // NO REWIND
  //               else if ( m_dLastStoneToPartSideDist_um <
  //                 m_dCoarseStoneToPartTargetPosition_um + dThresForFineStoneRewind_um )
  //                 SendMessageToEngine( "Rewind;" , NULL );
                else
                {
                  FXString Msg;
                  Msg.Format("TuneSideWheel;//FineDist=%.2f Coarse=%.2f", m_dLastStoneToPartSideDist_um,
                    m_dLastCoarseSideAdjustedPos);
                  SendMessageToEngine(Msg, NULL);
                  m_bSideStoneInitialAdjustmentIsFinished = true;
                  m_dLastFineSideGrindingStart = m_dLastStoneToPartSideDist_um;
                }
              }
            }
            else
            {
              // Fine stone should be above coarse stone
              // for 20-40 microns : it's initial position for slow fine
              // grinding before final grinding by camera
              FXString Msg;
              Msg.Format("Finished;//Final FineDist=%.2f Coarse=%.2f", m_dLastStoneToPartSideDist_um,
                m_dLastCoarseSideAdjustedPos);
              SendMessageToEngine(Msg, NULL);
              SetIdleStage();
              m_bSideStoneInitialAdjustmentIsFinished = false;
              m_dLastFineSideGrindingStart = m_dLastStoneToPartSideDist_um;
            }
            m_bDone = false;
            m_dLastMotionFinishedTime = 0.;
          }
        }
        return true;
      case STG_CoarseStoneMeas:
        {
          if (m_bDone && ((GetHRTickCount() - m_dLastMotionFinishedTime) > 2000.))
          {
            m_bDone = false;
            FXString Msg;
            Msg.Format("//Dist=%.2f", m_dLastStoneToPartSideDist_um);
            if ( !m_bSideStoneInitialAdjustmentIsFinished )
            {
              if (m_dLastStoneToPartSideDist_um > 550.)
              {
                double dLargeStep = Reg.GetRegiDouble(
                  "Motions", "FirstLargeStepForCoarse_ms", 100.);
                FXString Command;
                Command.Format("Large=%d;", ROUND(dLargeStep));
                Msg.Insert(0, Command);
                SendMessageToEngine(Msg, NULL);
              }
              else if (m_dLastStoneToPartSideDist_um > m_dStoneToMasterDistance_um + 100.)
              {
                //               int iLargeTime_ms = ROUND(400. * m_dLastStoneToPartSideDist_um 
                //                 / dLargeMoveDistForCoarse400ms_um);
                //               FXString Command;
                //               Command.Format("Large=%d;", iLargeTime_ms);
                Msg.Insert(0, "Medium;");
                SendMessageToEngine(Msg, NULL);
              }
              else if (m_dLastStoneToPartSideDist_um > m_dStoneToMasterDistance_um + 10.)
              {
                Msg.Insert(0, "Small;");
                SendMessageToEngine(Msg, NULL);
              }
              else if (m_dLastStoneToPartSideDist_um < m_dStoneToMasterDistance_um - 5.)
              {
                Msg.Insert(0, "Rewind;");
                SendMessageToEngine(Msg, NULL);
              }
              else 
              {
                Msg.Insert(0, "TuneSideWheel;");
                SendMessageToEngine(Msg, NULL);
                m_bSideStoneInitialAdjustmentIsFinished = true;
                m_dLastCoarseSideAdjustedPos = m_dLastStoneToPartSideDist_um;
              }
            }
            else
            {
              Msg.Insert(0, "Finished;");
              SendMessageToEngine(Msg, NULL);
              SetIdleStage();
              m_bSideStoneInitialAdjustmentIsFinished = false;
              m_dLastCoarseSideAdjustedPos = m_dLastStoneToPartSideDist_um;
              return true;
            }
            m_bDone = false;
            m_dLastMotionFinishedTime = 0.;
          }
        }
        break;

      case STG_CoarseTipPosition:
        {
          if (m_dLastMotionFinishedTime != 0.)
          {
            double dTimeAfterMotionFinished = GetHRTickCount() - m_dLastMotionFinishedTime;
            if (dTimeAfterMotionFinished > 2000.) // ms
            {
              FXString Msg;
              int iViewAndStabilizationTime_frames =
                Reg.GetRegiInt("Motions", "ViewAndStabilizationFrames", 60);
              double dCoarseTipPositionToMasterDist_um =
                Reg.GetRegiDouble("Motions", "CoarseTipPositionToMasterDist_um", 10.);
              if (!m_iGoodPositionCounter)
              {
                if (m_dLastMinStoneToPartDist_um > m_dStoneToMasterDistance_um + dCoarseTipPositionToMasterDist_um)
                {
                  Msg.Format("Large;// Coarse MinDist=%.2f", m_dLastMinStoneToPartDist_um);
                }
                //                 else if (m_dLastMinStoneToPartDist_um > m_dStoneToMasterDistance_um + 10. )
                //                 {
                //                   double dDelta = m_dLastMinStoneToPartDist_um - m_dStoneToMasterDistance_um;
                //                   int iOrderTime_ms = ROUND(400. * dDelta / 45.);
                //                   Msg.Format("Large;// MinDist=%.2f", 
                //                     /*iOrderTime_ms ,*/ m_dLastMinStoneToPartDist_um);
                //                 }
                //                 else if ( m_dLastMinStoneToPartDist_um < m_dStoneToMasterDistance_um )
                //                   Msg.Format( "Rewind;// MinDist=%.2f" , m_dLastMinStoneToPartDist_um );
                else
                {
                  m_dLastCoarseSideGrindingStop = m_dLastMinStoneToPartDist_um;
                  m_iGoodPositionCounter = 1;
                }
              }
              else if (++m_iGoodPositionCounter < iViewAndStabilizationTime_frames)
              {
                pMarking->AddFrame(CreateTextFrame(ViewPt, "0x808000",
                  20, "CoarseStabilization", pMarking->GetId(),
                  "\n\n\n\nCoarse Dist: %.2fum, Cnt=%d",
                  m_dLastMinStoneToPartDist_um, m_iGoodPositionCounter));
              }
              else
              {   // switch to fine stone for tip adjustment
                Sleep(500);
                Msg.Format("Medium;// Tip Coarse MinDist=%.2f Static CoarseDist=%.2f",
                  m_dLastMinStoneToPartDist_um, m_dLastCoarseSideAdjustedPos);
                m_dLastCoarseSideGrindingStop = m_dLastMinStoneToPartDist_um;
                m_WorkingStage = STG_FineTipPosition;
              }
              if (!Msg.IsEmpty())
              {
                SendMessageToEngine(Msg, "CoarseTipPosionCheck");
                m_dLastMinStoneToPartDist_um = 10000.;
                m_iGoodPositionCounter = 0;
                m_dLastMotionFinishedTime = 0.;
                m_bDone = false;
              }
            }
          }
          return 1;
        }
      case STG_FineTipPosition:
        {
          int iViewAndStabilizationTime_frames =
            Reg.GetRegiInt("Motions", "ViewAndStabilizationFrames", 60);
          double dTimeAfterMotionFinished = GetHRTickCount() - m_dLastMotionFinishedTime;
          if (m_bDone && (dTimeAfterMotionFinished > 2000.)) // ms
          {
            FXString Msg;
            if (!m_iGoodPositionCounter)
            {
              //               if ( m_dCoarseStoneToPartTargetPosition_um > m_dStoneToMasterDistance_um
              //                 || m_dCoarseStoneToPartTargetPosition_um + 50. < m_dStoneToMasterDistance_um )
              //               {
              //                 m_dCoarseStoneToPartTargetPosition_um = m_dStoneToMasterDistance_um;
              //               }
              double dCoarseToFineDistForIp_um = Reg.GetRegiDouble(
                "Motions", "FineStoneToCoarseDistForTip_um", 30.);

              if (m_dLastMinStoneToPartDist_um >
                (m_dLastCoarseSideGrindingStop - dCoarseToFineDistForIp_um))
              {
                Msg.Format("Small;// Fine MinDist=%.2f CoarseStopDist=%.2f",
                  m_dLastMinStoneToPartDist_um, m_dLastCoarseSideGrindingStop);
              }
              //               else if ( m_dLastMinStoneToPartDist_um < m_dCoarseStoneToPartTargetPosition_um - 15. )
              //                 Msg.Format( "Rewind;// MinDist=%.2f CoarseDist=%.2f" ,
              //                   m_dLastMinStoneToPartDist_um , m_dCoarseStoneToPartTargetPosition_um );
              else
              {
                m_dLastFineSideAdjustedStop = m_dLastMinStoneToPartDist_um;
                m_iGoodPositionCounter = 1;
              }
            }
            else if (++m_iGoodPositionCounter < iViewAndStabilizationTime_frames)
            {
              pMarking->AddFrame(CreateTextFrame(ViewPt, "0x808000",
                20, "FineStabilization", pMarking->GetId(),
                "\n\n\n\nFine dist: %.2fum, Cnt=%d",
                m_dLastMinStoneToPartDist_um, m_iGoodPositionCounter));
              m_dLastFineSideAdjustedStop = m_dLastMinStoneToPartDist_um;
            }
            else
            {
              Sleep(500);
              Msg.Format("Finished;// MinDist=%.2f FineDist=%.2f",
                m_dLastMinStoneToPartDist_um, m_dFineStoneToPartTargetPosition_um);
              m_dLastFineSideAdjustedStop = m_dLastMinStoneToPartDist_um;
              SetIdleStage();
              m_AdditionalInfo4.Format("Stone Adjustment Results:\n"
                "FineBegin=%.2fum\nCoarseStop=%.2fum\n"
                "CoarseTip=%.2fum\nFineTip=%.2fum",
                m_dLastFineSideGrindingStart, m_dLastCoarseSideAdjustedPos,
                m_dLastCoarseSideGrindingStop, m_dLastFineSideAdjustedStop);
              SaveLogMsg(m_AdditionalInfo4);

            }
            if (!Msg.IsEmpty())
            {
              SendMessageToEngine(Msg, NULL);
              m_dLastMinStoneToPartDist_um = 10000.;
              m_iGoodPositionCounter = 0;
              m_dLastMotionFinishedTime = 0.;
              m_bDone = false;
            }
          }
        }
        return 1;
      }
    }
  }
  else
  {
    m_bStoneIsMeasured = false;
    switch (m_WorkingStage)
    {
    case STG_FineStoneMeas:
      if (m_bDone && ((GetHRTickCount() - m_dLastMotionFinishedTime) > 400.))
      {
        m_iLastStoneMovingTime_ms = Reg.GetRegiInt(
          "Motions", "FineLargeMotion_ms", 200);
        FXString LargeMsg;
        LargeMsg.Format("Large=%d;", m_iLastStoneMovingTime_ms);
        SendMessageToEngine(LargeMsg, NULL);
      }
      return true;
    case STG_CoarseStoneMeas:
      if (m_bDone && ((GetHRTickCount() - m_dLastMotionFinishedTime) > 1000.))
      {
        m_iLastStoneMovingTime_ms = Reg.GetRegiInt(
          "Motions", "CoarseLargeMotion_ms", 1000);
        FXString LargeMsg;
        LargeMsg.Format("Large=%d;", m_iLastStoneMovingTime_ms);
        if (!m_bStoneIsMeasured)
          SendMessageToEngine(LargeMsg, NULL);
      }
      return true;
    }
  }

  return 0;
}

WhatDispenserFigureFound MPPT_Dispens::SetDataAboutContour(
  const CFigureFrame * pFrame)
{
  LPCTSTR pLabel = pFrame->GetLabel();
  bool bIsContur = (_tcsstr(pLabel, _T("Contur")) == pLabel);
  bool bIsSegment = (_tcsstr(pLabel, _T("Data_Edge")) == pLabel);
  bool bIsStraight = (_tcsstr(pLabel, _T("Straight")) == pLabel);
  // conturs, lines and edges
  //   back_light
  //   blank_ext
  //   blank_int
  //   circle_ext
  //   circle_int
  //   side_view

  if (bIsContur)
  {
    for (int i = 0; i < ARRSZ(NamesAndIDs); i++)
    {
      if (_tcsstr(pLabel, NamesAndIDs[i].pName))
      {
        m_LastFigures.push_back(pFrame);
        TRACE("\nm_LastFigures[%d]=%s(%d pts) %s", m_LastFigures.size() - 1,
          pLabel, pFrame->size(), GetWorkingModeName() );
        return NamesAndIDs[i].ID;
      }
    }
  }
  else if (bIsStraight)
  {
    LPCTSTR pNumber = _tcschr(pLabel, _T('_'));
    if (pNumber)
    {
      int iSegmentNumber = atoi(pNumber + 1);
      if (iSegmentNumber >= (int)m_LastSegments.size())
        m_LastSegments.resize(iSegmentNumber + 1);
      cmplx Pt1 = CDPointToCmplx(pFrame->GetAt(0));
      cmplx Pt2 = CDPointToCmplx(pFrame->GetAt(1));
      CLine2d NewSegment(Pt1, Pt2);
      m_LastSegments[iSegmentNumber] = NewSegment;
      return WDFF_Segment;
    }
  }
  return WDFF_NotFound;
}

int MPPT_Dispens::SetDataAboutContours(CFramesIterator * pFiguresIterator)
{
  if (!pFiguresIterator)
    return 0;
  DWORD dwFoundMask = 0;
  CFigureFrame * pNewFigure = NULL;
  int iNFound = 0;
  while (pNewFigure = (CFigureFrame*)pFiguresIterator->Next())
  {
    LPCTSTR pLabel = pNewFigure->GetLabel();
    //     if (pNewFigure->GetCount() < 20 )
    //       continue;
    LogFigure(pLabel, Fig_Touch, __LINE__, pNewFigure->GetUserCnt());
    WhatDispenserFigureFound Res = SetDataAboutContour(pNewFigure);
    dwFoundMask |= Res;
    iNFound += ((Res & (WDFF_Conturs | WDFF_Segment)) != 0);
    LogFigure(pNewFigure->GetLabel(), Fig_Touch, __LINE__, pNewFigure->GetUserCnt());
  }
  m_dwLastFilledConturs = dwFoundMask;
  return iNFound;
}

int MPPT_Dispens::SetDataAboutROIs(const CDataFrame * pDataFrame)
{
  int iNExtracted = 0;
  CFramesIterator * Iter = pDataFrame->CreateFramesIterator(rectangle);
  if (Iter)
  {
    CRectFrame * pNewRect = NULL;
    while (pNewRect = (CRectFrame*)Iter->Next())
    {
      iNExtracted++;
      LPCTSTR pLabel = pNewRect->GetLabel();
      if (_tcsstr(pLabel, "ROI:side_view"))
        m_SideROI = *((RECT*)pNewRect);
      else if (_tcsstr(pLabel, "ROI:blank_ext"))
        m_ConeCircleROI = *((RECT*)pNewRect);
      else if (_tcsstr(pLabel, "ROI:blank_int"))
        m_HoleROI = *((RECT*)pNewRect);
      else if (_tcsstr(pLabel, "ROI:back_light"))
        m_HoleROI = *((RECT*)pNewRect);
      else if (_tcsstr(pLabel, "ROI:side_stone"))
        m_SideStoneROI = *((RECT*)pNewRect);
      else
        iNExtracted--;
    }
  }

  return iNExtracted;
}

int MPPT_Dispens::ClearInputQueue(bool bSingle, bool bContainers, bool bVFrames)
{
  Sleep(25);
  int iNFrames = 0;
  CDataFrame * p = NULL;
  vector<CDataFrame*> ForResend;
  while (m_pInput->GetNFramesInQueue())
  {
    if (m_pInput->Get(p))
    {
      if (p->IsContainer() && !bContainers)
      {
        ForResend.push_back(p);
        continue;
      }
      else
      {
        datatype Type = p->GetDataType();
        if (Type == vframe)
        {
          if (!bVFrames)
          {
            ForResend.push_back(p);
            continue;
          }
        }
        else if (!bSingle)
        {
          ForResend.push_back(p);
          continue;
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
  for (size_t i = 0; i < ForResend.size(); i++)
    m_pInput->Send(ForResend[i]);

  return iNFrames;
}

int MPPT_Dispens::SetWatchDog(int iTime_ms)
{
  DeleteWatchDog();
  if (!CreateTimerQueueTimer(&m_hWatchDogTimer, NULL, (WAITORTIMERCALLBACK)MPPT_DispensWDTimerRoutine,
    this, iTime_ms, 0, 0))
  {
    SEND_GADGET_ERR("Create Watchdog timer failed");
    return false;
  }
  return true;
}

bool MPPT_Dispens::DeleteWatchDog()
{
  m_LockWatchdog.Lock();
  if (m_hWatchDogTimer != NULL)
  {
    BOOL bDeleteTimerQueueTimer = DeleteTimerQueueTimer(NULL, m_hWatchDogTimer, NULL);
    m_hWatchDogTimer = NULL;
    if (!bDeleteTimerQueueTimer)
    {
      if (GetLastError() != ERROR_IO_PENDING)
      {
        SEND_GADGET_ERR(_T("Can't delete Watchdog Timer"));
        m_LockWatchdog.Unlock();
        return false;
      }
    }
  }
  m_LockWatchdog.Unlock();
  return true;
}

int MPPT_Dispens::SelectCurrentPart(LPCTSTR pPartName)
{
  FXAutolock al(m_MPPD_Lock);
  auto it = m_KnownParts.begin();
  for (; it != m_KnownParts.end(); it++)
  {
    string ItPartName(it->m_Name);
    if (ItPartName == pPartName)
    {
      m_CurrentPart = *it;
      m_iSelectedPart = (int)(it - m_KnownParts.begin());
      FXRegistry Reg("TheFileX\\MPP_Dispens");
      Reg.WriteRegiString("PartsData", "SelectedPart", m_CurrentPart.m_Name);
      return 1;
    }
  }
  return 0;
}

bool MPPT_Dispens::FilterCircle(CmplxVector& Pts,
  cmplx& cCent_pix, double& dDia_pix,
  double& dMaxDia_pix, double& dMinDia_pix,
  double dMaxDeviation_pix,
  CContainerFrame * pMarking)
{
  bool bRes = CircleFitting(Pts.data(), (int)Pts.size(), cCent_pix, dDia_pix);
  if (bRes)
  {
    double dRadius_pix = dDia_pix * 0.5;
    for (size_t i = 0; i < Pts.size(); i++)
    {
      double dDistToCenter = abs(cCent_pix - Pts[i]);
      double dDist = fabs(dDistToCenter - dRadius_pix);
      if (dDist > dMaxDeviation_pix)
      {
        if ((m_iViewMode >= 8) && pMarking)
          pMarking->AddFrame(CreatePtFrame(Pts[i], "color=0x0000ff;Sz=2;"));
        Pts.erase(Pts.begin() + i--);
      }
      else
      {
        if ((m_iViewMode >= 8) && pMarking)
          pMarking->AddFrame(CreatePtFrame(Pts[i], "color=0x00ff00;Sz=2;"));
      }
    }
    if (!Pts.empty())
    {
      bRes = CircleFitting(Pts.data(),
        (int)Pts.size(), cCent_pix, dDia_pix);
      if (bRes)
      {
        pMarking->AddFrame(CreateCircleView(
          cCent_pix, dDia_pix * 0.5, 0x8040ff));
        dRadius_pix = dDia_pix / 2.0;

        double dMaxRadius = 0., dMinRadius = DBL_MAX;
        for (size_t i = 0; i < Pts.size(); i++)
        {
          double dDistToCenter = abs(cCent_pix - Pts[i]);
          double dDist = fabs(dDistToCenter - dRadius_pix);
          SetMinMax(dDistToCenter, dMinRadius, dMaxRadius);
        }
        GetMinMaxDia(Pts.data(), (int)Pts.size(), cCent_pix,
          N_SEGMENTS_FOR_CIRCLE_EXTRACTION, dMinDia_pix, dMaxDia_pix,
          m_CurrentPart.m_iDiameterAverage);
        return true;
      }
    }
  }
  return false;
}

bool MPPT_Dispens::FilterByCenterAndDia(CmplxVector& Pts,
  cmplx cCent_pix, double dDia_pix, cmplx& cNewCent_pix, double& dNewDia_pix,
  double dMaxDeviation_pix, CContainerFrame * pMarking)
{
  double dRadius_pix = dDia_pix * 0.5;
  for (size_t i = 0; i < Pts.size(); i++)
  {
    double dDistToCenter = abs(cCent_pix - Pts[i]);
    double dDist = fabs(dDistToCenter - dRadius_pix);
    if (dDist > dMaxDeviation_pix)
    {
      if ((m_iViewMode >= 8) && pMarking)
        pMarking->AddFrame(CreatePtFrame(Pts[i], "color=0x0000ff;Sz=2;"));
      Pts.erase(Pts.begin() + i--);
    }
    else
    {
      if ((m_iViewMode >= 8) && pMarking)
        pMarking->AddFrame(CreatePtFrame(Pts[i], "color=0x00ffff;Sz=2;"));
    }
  }
  if (Pts.size() > 5)
  {
    bool bRes = CircleFitting(Pts.data(),
      (int)Pts.size(), cNewCent_pix, dNewDia_pix);
    if (bRes)
    {
      pMarking->AddFrame(CreateCircleView(
        cCent_pix, dDia_pix * 0.5, 0x8040ff));
      dRadius_pix = dDia_pix / 2.0;

      double dMaxRadius = 0., dMinRadius = DBL_MAX;
      for (size_t i = 0; i < Pts.size(); i++)
      {
        double dDistToCenter = abs(m_cLastIntCenter_pix - Pts[i]);
        double dDist = fabs(dDistToCenter - dRadius_pix);
        SetMinMax(dDistToCenter, dMinRadius, dMaxRadius);
      }
      return true;
    }
  }
  return false;
}


// true - some circles are extracted
bool MPPT_Dispens::PreProcessCircles(
  CContainerFrame * pMarking, const CDataFrame * pDataFrame, const CVideoFrame * pVF)
{
  m_bLastWhiteDIOK = m_bLastBlackDIOK = m_bLastBlackDOOK = false;
  CmplxVector Pts;
  // Moisey 01.11.22
  // Only white contours will be found
  // If front light is in using, contours should be extracted from original image
  if ((m_dwLastFilledConturs & WDFF_WhiteContur) && m_bUseBackLight)
  {
    m_bWhiteWasUsed = true;
    m_LastSpots.RemoveAll();
    m_dLastBlackDI_pix = m_dLastDI_pix = 0.;
    ExtractDataAboutSpots(pDataFrame, m_LastSpots);
    GetCircles(pDataFrame, pMarking);
    if (m_FoundIntCircles.size() && pVF != NULL)
    {
      cmplx cCent = m_FoundIntCircles[0].m_cCenter;
      cmplx cDistToCent = cCent - m_cLastROICent_pix;
      if (abs(cDistToCent) > abs(m_cLastROICent_pix) / 3)
        return false;

      double dRadBegin = m_FoundIntCircles[0].m_dRadius * 0.5;
      double dRadEnd = m_FoundIntCircles[0].m_dRadius * 1.2;
      if (pMarking && (m_iViewMode > 9))
      {
        pMarking->AddFrame(CreateCircleView(cCent, dRadBegin, 0xff8000));
        pMarking->AddFrame(CreateCircleView(cCent, dRadEnd, 0xff0080));
      }
      m_bLastWhiteDIOK = ExtractCirclesByCenterAndRadius(
        pVF, cCent, dRadBegin, dRadEnd, 0.5,
        CEM_FirstEdge, DTM_PositiveOnly,
        Pts, (m_iViewMode > 7) ? pMarking : NULL, N_SEGMENTS_FOR_CIRCLE_EXTRACTION);
      if (m_bLastWhiteDIOK)
      {
        m_dLastDI_pix = m_FoundIntCircles[0].m_dRadius * 2.;
        m_dLastDI_um = m_dLastDI_pix * m_dInternalWhiteScale_um_per_pixel * m_dFrontScaleCorrectionCoeff;
        double dIntRadius_um = m_dLastDI_um * 0.5;

        m_bLastWhiteDIOK = FilterCircle(Pts, m_cLastIntCenter_pix, m_dLastDI_pix,
          m_dLastMaxWhiteDI_pix, m_dLastMinWhiteDI_pix,
          m_dMaxPtDeviation_pix, pMarking);
        //       bRes = CircleFitting( Pts.data() , ( int ) Pts.size() , m_cLastIntCenter_pix , m_dLastDI_pix ) ;
        if (m_bLastWhiteDIOK)
        {
          m_cLastIntWhiteCenter_pix = m_cLastWhiteConturCenter_pix = m_cLastIntCenter_pix;
          m_dLastWhiteDI_pix = m_dLastDI_pix;
          m_dLastWhiteDI_um = m_dLastDI_pix * m_dInternalWhiteScale_um_per_pixel * m_dFrontScaleCorrectionCoeff;
          m_dLastMinWhiteDI_um = m_dLastMinWhiteDI_pix * m_dInternalWhiteScale_um_per_pixel * m_dFrontScaleCorrectionCoeff;
          m_dLastMaxWhiteDI_um = m_dLastMaxWhiteDI_pix * m_dInternalWhiteScale_um_per_pixel  * m_dFrontScaleCorrectionCoeff;
        }
      }
    }
    return (m_bLastWhiteDIOK);
  }
  if (abs(m_cLastIntWhiteCenter_pix - m_cLastROICent_pix) > 100.)
  {
    cmplx cViewPt(m_cLastROICent_pix - m_cLastROICent_pix.real() * 0.5);
    pMarking->AddFrame(CreateTextFrame(cViewPt, "Adjust white spot\n      to center", 0x0000ff, 20));
    return false;
  }

  if (m_bUseFrontLight)
  {
    m_bWhiteIsOK = false;
    double dStartSearch_pix = (m_CurrentPart.m_dBlankIDmin_um * 0.5 / m_dInternalBlackScale_um_per_pixel) * 0.8;
    double dEndSearch_pix = (m_CurrentPart.m_dIDmax_um * 0.5 / m_dInternalBlackScale_um_per_pixel) * 1.2;
    if (pMarking && (m_iViewMode > 9))
    {
      pMarking->AddFrame(CreateCircleView(
        m_cLastIntWhiteCenter_pix, dStartSearch_pix, 0xff8000));
      pMarking->AddFrame(CreateCircleView(
        m_cLastIntWhiteCenter_pix, dEndSearch_pix, 0xff0080));
    }
    m_bLastBlackDIOK = ExtractCirclesByCenterAndRadius(
      pVF, m_cLastIntWhiteCenter_pix, dStartSearch_pix, dEndSearch_pix,
      0.5, CEM_FirstDiff, DTM_PositiveOnly, // internal circle will be found by positive diff
      Pts, (m_iViewMode > 7) ? pMarking : NULL, N_SEGMENTS_FOR_CIRCLE_EXTRACTION);
    if (m_bLastBlackDIOK)
    {
      m_bLastBlackDIOK = FilterCircle(Pts, m_cLastIntBlackCenter_pix, m_dLastDI_pix,
        m_dLastMaxBlackDI_pix, m_dLastMinBlackDI_pix,
        m_dMaxPtDeviation_pix, pMarking);
      // center deviation not more than 100 pixels
      bool bCenterIsOK = abs(m_cLastIntBlackCenter_pix - m_cLastROICent_pix) < 100.;
      bool bStartSearchOK = (fabs(m_cLastIntBlackCenter_pix.real() + dStartSearch_pix) < GetWidth(pVF))
        && (fabs(m_cLastIntBlackCenter_pix.real() - dStartSearch_pix) >= 0);
      bool bEndSearchOK = (fabs(m_cLastIntBlackCenter_pix.real() + dEndSearch_pix) < GetWidth(pVF))
        && (fabs(m_cLastIntBlackCenter_pix.real() - dEndSearch_pix) >= 0);
      if (m_bLastBlackDIOK && bCenterIsOK && bStartSearchOK && bEndSearchOK)
      {
        dStartSearch_pix = m_dLastDI_pix * 0.46;
        dEndSearch_pix = m_dLastDI_pix * 0.55;
        if (pMarking && (m_iViewMode > 9))
        {
          pMarking->AddFrame(CreateCircleView(
            m_cLastIntBlackCenter_pix, dStartSearch_pix, 0x80ff00));
          pMarking->AddFrame(CreateCircleView(
            m_cLastIntBlackCenter_pix, dEndSearch_pix, 0x80ff80));
        }
        // repeat search for smaller range
        m_bLastBlackDIOK = ExtractCirclesByCenterAndRadius(
          pVF, m_cLastIntBlackCenter_pix, dStartSearch_pix, dEndSearch_pix,
          0.5, CEM_FirstDiff, DTM_PositiveOnly, // internal circle will be found by positive diff
          Pts, (m_iViewMode > 7) ? pMarking : NULL, N_SEGMENTS_FOR_CIRCLE_EXTRACTION);
        m_bLastBlackDIOK = FilterCircle(Pts, m_cLastIntBlackCenter_pix, m_dLastDI_pix,
          m_dLastMaxBlackDI_pix, m_dLastMinBlackDI_pix,
          m_dMaxPtDeviation_pix, pMarking);
        if (m_bLastBlackDIOK)
        {
          m_dLastDI_um = m_dLastDI_pix * m_dInternalBlackScale_um_per_pixel * m_dFrontScaleCorrectionCoeff;
          m_dLastBlackDI_pix = m_dLastDI_pix;
          m_dLastBlackDI_um = m_dLastDI_pix * m_dInternalBlackScale_um_per_pixel * m_dFrontScaleCorrectionCoeff;
          m_dLastMinBlackDI_um = m_dLastMinBlackDI_pix * m_dInternalBlackScale_um_per_pixel * m_dFrontScaleCorrectionCoeff;
          m_dLastMaxBlackDI_um = m_dLastMaxBlackDI_pix * m_dInternalBlackScale_um_per_pixel * m_dFrontScaleCorrectionCoeff;
        }
      }
      else
        m_dLastBlackDI_um = m_dLastBlackDI_pix = m_dLastDI_um = m_dLastDI_pix = 0.;
    }

    dStartSearch_pix = (m_CurrentPart.m_dODmax_um * 0.5 / m_dExternalScale_um_per_pixel) * 1.2;
    dEndSearch_pix = (m_CurrentPart.m_dODmin_um * 0.5 / m_dExternalScale_um_per_pixel) * 0.8;

    int iWidth = GetWidth(pVF);
    int iHeight = GetHeight(pVF);
    int iRad = min(iWidth, iHeight) / 2;
#define DIST_FROM_EDGE 6
    if (dStartSearch_pix + m_cLastIntWhiteCenter_pix.real() > (2 * iRad) - DIST_FROM_EDGE)
      dStartSearch_pix = iRad - DIST_FROM_EDGE;
    if (m_cLastIntWhiteCenter_pix.real() - dStartSearch_pix < DIST_FROM_EDGE)
      dStartSearch_pix = m_cLastIntWhiteCenter_pix.real() - DIST_FROM_EDGE;
    if (dEndSearch_pix + m_cLastIntWhiteCenter_pix.real() > (2 * iRad) - DIST_FROM_EDGE)
      dEndSearch_pix = iRad - DIST_FROM_EDGE;
    if (m_cLastIntWhiteCenter_pix.real() - dEndSearch_pix < DIST_FROM_EDGE)
      dEndSearch_pix = m_cLastIntWhiteCenter_pix.real() - DIST_FROM_EDGE;

    if (pMarking && (m_iViewMode > 9))
    {
      pMarking->AddFrame(CreateCircleView(
        m_cLastIntWhiteCenter_pix, dStartSearch_pix, 0xff8000));
      pMarking->AddFrame(CreateCircleView(
        m_cLastIntWhiteCenter_pix, dEndSearch_pix, 0xff0080));
    }
    //Search will be from outside to inside
    m_bLastBlackDOOK = ExtractCirclesByCenterAndRadius(
      pVF, m_cLastIntWhiteCenter_pix, dStartSearch_pix, dEndSearch_pix,
      m_dImagingDiffThreshold, CEM_FirstDiff, DTM_NegativeOnly,
      Pts, (m_iViewMode > 7) ? pMarking : NULL, N_SEGMENTS_FOR_CIRCLE_EXTRACTION);
    if (m_bLastBlackDOOK)
    {
      double dExpectedExtDia = 0.5 * (m_CurrentPart.m_dODmax_um + m_CurrentPart.m_dODmin_um)
        / m_dExternalScale_um_per_pixel;

      m_bLastBlackDOOK = FilterByCenterAndDia(Pts, m_cLastIntWhiteCenter_pix,
        dExpectedExtDia, m_cLastExtCenter_pix,
        m_dLastExternalDia_pix, m_dMaxPtDeviation_pix * 3., pMarking);

      if (pMarking && (m_iViewMode > 9))
      {
        pMarking->AddFrame(CreateCircleView(
          m_cLastExtCenter_pix, m_dLastExternalDia_pix * 0.5, 0xff0080));
        pMarking->AddFrame(CreatePtFrame(m_cLastExtCenter_pix, "color=0xff00ff;sz=4;"));
        if (m_iViewMode >= 11)
        {  // View radius number
          for (size_t i = 0; i < Pts.size(); i++)
          {
            cmplx cVectFromCenter = Pts[i] - m_cLastExtCenter_pix;
            cmplx ViewPt = m_cLastExtCenter_pix + cVectFromCenter * 1.05;
            pMarking->AddFrame(CreateTextFrame(ViewPt, "0x00ff00", 6, NULL, 0, "%d", (int)i));
          }
        }
      }
      if (pMarking && (m_iViewMode > 5))
      {
        double dODmin_pix = m_CurrentPart.m_dODmin_um / m_dExternalScale_um_per_pixel;
        double dSide_pix = 0.1 * dODmin_pix;
        double dPos_pix = 0.4 * dODmin_pix;
        cmplx ViewPt(m_cLastIntWhiteCenter_pix.real() - dPos_pix,
          m_cLastIntWhiteCenter_pix.imag() - dSide_pix * 0.5);

        CRect FocusArea(ROUND(ViewPt.real()),
          ROUND(ViewPt.imag()),
          ROUND(ViewPt.real() + dSide_pix),
          ROUND(ViewPt.imag() + dSide_pix * 0.5));

        double dFocusVal = _calc_laplace(pVF, FocusArea);
        pMarking->AddFrame(CreateTextFrame(ViewPt, "0x00ffff", 10, NULL,
          0, "F=%.2f", dFocusVal));
      }
      if (m_bLastBlackDOOK)
      {
        dStartSearch_pix = m_dLastExternalDia_pix * 0.52;
        dEndSearch_pix = m_dLastExternalDia_pix * 0.46;
        if (pMarking && (m_iViewMode > 10))
        {
          pMarking->AddFrame(CreateCircleView(
            m_cLastExtCenter_pix, dStartSearch_pix, 0xffffff));
          pMarking->AddFrame(CreateCircleView(
            m_cLastExtCenter_pix, dEndSearch_pix, 0xffffff));
        }
        m_bLastBlackDOOK = ExtractCirclesByCenterAndRadius(
          pVF, m_cLastExtCenter_pix, dStartSearch_pix, dEndSearch_pix,
          m_dImagingDiffThreshold, CEM_FirstDiff, DTM_NegativeOnly,
          Pts, (m_iViewMode > 7) ? pMarking : NULL, N_SEGMENTS_FOR_CIRCLE_EXTRACTION);
        if (m_bLastBlackDOOK)
        {
          m_bLastBlackDOOK = FilterCircle(Pts, m_cLastExtCenter_pix, m_dLastExternalDia_pix,
            m_dLastMaxExternalDia_pix, m_dLastMinExternalDia_pix,
            m_dMaxPtDeviation_pix, pMarking);
          if (m_bLastBlackDOOK)
          {
            GetMinMaxDia(Pts.data(), (int)Pts.size(), m_cLastExtCenter_pix,
              N_SEGMENTS_FOR_CIRCLE_EXTRACTION,
              m_dLastMinExternalDia_pix, m_dLastMaxExternalDia_pix,
              m_CurrentPart.m_iDiameterAverage);

            if (m_bLastBlackDOOK)
              m_dLastExternalDia_um = m_dLastExternalDia_pix * m_dExternalScale_um_per_pixel * m_dFrontScaleCorrectionCoeff;
            else
              m_dLastExternalDia_um = m_dLastExternalDia_pix = 0.;
          }
        }
      }
    }
    return (m_bLastBlackDIOK && m_bLastBlackDOOK);
  }
  return (m_bLastWhiteDIOK || (m_bLastBlackDIOK && m_bLastBlackDOOK));
}

// true - continue processing
bool MPPT_Dispens::PreProcessVideoFrame(CContainerFrame * pMarking,
  const CDataFrame * pDataFrame, const CVideoFrame * pVF)
{
  if (pVF)
  {
    m_cLastROICent_pix = cmplx(GetWidth(pVF) / 2., GetHeight(pVF) / 2.);
    m_LastROICent = CPoint(GetWidth(pVF) / 2, GetHeight(pVF) / 2);
    m_LastFOV.cx = m_LastFrameRect.right = GetWidth(pVF) - 1;
    m_LastFOV.cy = m_LastFrameRect.bottom = GetHeight(pVF) - 1;
    CRect rc(1, 1, m_LastFOV.cx - 1, m_LastFOV.cy - 1);

    int iMin = INT_MAX, iMax = -INT_MAX;
    // First and last rows are include info about capture conditions
    // it can distort min max calculation results
    _find_min_max(pVF, iMin, iMax, rc);
    m_iLastConstrast = iMax - iMin;
    bool bLowContrast = m_iLastConstrast < m_dMinBrightnessAmplitude;
    if (bLowContrast && (iMax < 200))
      return false;
    else
      m_iNLowContrastImages = 0;

    m_dLastCaptureTime_ms = GetFrameTSFromEmbeddedInfo(pVF);
    double dDiff_ms = 0.;
    if (m_dLastCaptureTime_ms == 0.)
      m_dLastCaptureTime_ms = pVF->GetTime();
    dDiff_ms = (m_dLastCaptureTime_ms - m_dPrevCaptureTime_ms);
    m_dPrevCaptureTime_ms = m_dLastCaptureTime_ms;

    m_AllCaptureTimes.insert(m_AllCaptureTimes.begin(), dDiff_ms);

    if (m_WorkingMode == MPPD_Front)
    {
      // permanent check for sync
      if (m_dLastTriggerDelayTime_us == 0.)
      {
        if (m_dLastCaptureTime_ms != 0.)
        {
          m_CaptureTimes.push_back(m_dLastCaptureTime_ms);
          GetCapturePeriod(); // m_dLastCalculatedRotationPeriod_ms is filled
        }
      }
      else
        m_CaptureTimes.clear();

      cmplx cViewPt(20., 60.);

      double dPeriodDiff = dDiff_ms - m_dLastCalculatedRotationPeriod_ms;
      if (m_bMotorIsOn && (bIsSyncWork(m_WorkingStage)
        //         || ( ( m_WorkingStage != STG_Idle )
        //           && ( m_TriggerMode == TM_OneFrame )
        //           && ( m_WorkingStage != STG_WaitForMotorStop ) ) )
        )
        )
      {
        if (abs(dPeriodDiff) > m_dLastCalculatedRotationPeriod_ms * 0.05)
        {
          if (++m_iNOmittedFrames > 15)
          {
            m_dLastCalculatedRotationPeriod_ms = 113.;
            m_AllCaptureTimes.clear();
          }

          pMarking->AddFrame(CreateTextFrame(cViewPt,
            "0x0000ff", 12, NULL, pVF->GetId(),
            "No Sync: Last=%.2f Period=%.2f dDiff=%.4f Stage=%s(%d) nfr=%d",
            dDiff_ms, m_dLastCalculatedRotationPeriod_ms,
            dPeriodDiff, GetWorkingStateName(),
            m_WorkingStage, (int)m_CaptureTimes.size()));

          return false;
        }
        m_iNOmittedFrames = 0;
      }
      if (m_AllCaptureTimes.size() > 40)
        m_AllCaptureTimes.erase(m_AllCaptureTimes.end() - 1);
      pMarking->AddFrame(CreateTextFrame(cViewPt,
        "0x00ffff", 12, NULL, pVF->GetId(),
        "Sync: Last=%.2f Period=%.2f dDiff=%.4f Stage=%s(%d) nfr=%d",
        dDiff_ms, m_dLastCalculatedRotationPeriod_ms,
        dPeriodDiff, GetWorkingStateName(),
        m_WorkingStage, (int)m_CaptureTimes.size()));
      PreProcessCircles(pMarking, pDataFrame, pVF);
    }
  }
  return true;
}


double MPPT_Dispens::GetLastWhiteMeasurementResult(FXString& FullResultAsText,
  double dHoleTargetSizeCorrection_um)
{
  if (m_bUseBackLight && m_bLastWhiteDIOK)
  {
    m_dLastDiaDiffToMin_um = m_CurrentPart.m_dIDmin_um - m_dLastWhiteDI_um;
    m_dLastDiaDiffToMax_um = m_CurrentPart.m_dIDmax_um - m_dLastWhiteDI_um;
    // the next: if > 0 then necessary to continue to grind
    m_dLastDiaDiffMinToMin_um = m_CurrentPart.m_dIDmin_um - m_dLastMinWhiteDI_um;

    double dDiaNominal_um = m_CurrentPart.GetNominalHoleDia_um();
    m_dLastDiaDiffToNominal_um = m_dLastWhiteDI_um - dDiaNominal_um;
    double dInternalHalfAngle_rad = DegToRad(m_CurrentPart.m_dInternalConuseAngle_deg / 2.);

    if (m_bMinToMinForFrontGrinding)
    {  // work by min-to-min
      double dCorrectedMinToMin_um = m_dLastDiaDiffMinToMin_um + dHoleTargetSizeCorrection_um;
      m_dLastGrindingDist_um = (dCorrectedMinToMin_um / 2.) / tan(dInternalHalfAngle_rad);
      m_dLastDiffToUse_um = dCorrectedMinToMin_um/*m_dLastDiaDiffMinToMin_um*/;
    }
    else // work by nominal diameter
    {
      double dCorrectedDiaDiffToNom_um = m_dLastDiaDiffToNominal_um + dHoleTargetSizeCorrection_um;
      m_dLastGrindingDist_um = (dCorrectedDiaDiffToNom_um / 2.) / tan(dInternalHalfAngle_rad);
      m_dLastDiffToUse_um = dCorrectedDiaDiffToNom_um;
    }

    FullResultAsText.Format("Hole dDia=%.2f(%.2f,%.2f)um dDiaDiff=%.3fum "
      "MinToMin=%.2fum GrindDist=%.2fum",
      m_dLastWhiteDI_um, m_dLastMinWhiteDI_um, m_dLastMaxWhiteDI_um,
      m_dLastDiaDiffToNominal_um, m_dLastDiaDiffMinToMin_um, m_dLastGrindingDist_um);

    m_dLastDiaDiffMinToMin_um += dHoleTargetSizeCorrection_um;
    return m_dLastDiffToUse_um;
  }

  return DBL_MAX;
}

double MPPT_Dispens::GetLastWhiteAveragedResult(int iNAverage, double dHoleTargetSizeCorrection_um)
{
  double dDiffToUse_um = DBL_MAX;
  if (m_bUseBackLight && m_bLastWhiteDIOK)
  {
    int iWhiteCount = m_CurrentPart.AddToWhite(m_dLastWhiteDI_um,
      m_dLastMinWhiteDI_um, m_dLastMaxWhiteDI_um, m_dLastTirW_um);
    double dTIRw = 0.;
    double dAverWhite = m_CurrentPart.GetDIWhiteAverageResults(
      m_dLastMinWhiteDI_um, m_dLastMaxWhiteDI_um, dTIRw);
    m_dLastDiaDiffMinToMin_um = m_dLastMinWhiteDI_um - m_CurrentPart.m_dIDmin_um;
    m_dLastDiaDiffToNominal_um = dAverWhite - m_CurrentPart.GetNominalHoleDia_um();
    m_dLastDiaDiffMaxToMax_um = m_dLastMaxWhiteDI_um - m_CurrentPart.m_dIDmax_um;
    if (iWhiteCount >= iNAverage)
    {
      if (m_bMinToMinForFrontGrinding)
        dDiffToUse_um = m_dLastDiaDiffMinToMin_um + dHoleTargetSizeCorrection_um;
      else
        dDiffToUse_um = m_dLastDiaDiffToNominal_um + dHoleTargetSizeCorrection_um;
      double dInternalHalfAngle_rad = DegToRad(m_CurrentPart.m_dInternalConuseAngle_deg / 2.);

      m_dLastGrindingDist_um = (dDiffToUse_um / 2.) / tan(dInternalHalfAngle_rad);
      m_dLastDiffToUse_um = dDiffToUse_um;
    }
  }

  return dDiffToUse_um;
}
