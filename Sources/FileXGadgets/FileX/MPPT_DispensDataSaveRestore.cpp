#include "stdafx.h"
#include "MPPT.h"
#include <files/imgfiles.h>
#include <fxfc/FXRegistry.h>
#include <imageproc/statistics.h>

#include "MPPT_Dispens.h"

#define THIS_MODULENAME "MPPT"

extern CRect g_ROI;


void MPPT_Dispens::LoadAndUpdatePartParameters()
{
  RestoreKnownParts();
}

bool MPPT_Dispens::ProcessTextCommand(const CTextFrame * pCommand)
{
  if (pCommand)
  {
    FXString Label = pCommand->GetLabel();
    FXPropertyKit FullCommand(pCommand->GetString());

    // don't send to log if Done and Working Stage is Idle
    if (!((FullCommand == "Done") && (m_WorkingStage == STG_Idle)))
    {
      FXString ForLog;
      ForLog.Format("Proc.TextCom. %s entry: Lab=%s Cmd=%s State=%s",
        (m_WorkingMode == MPPD_Front) ? "Front" : "Side",
        (LPCTSTR)Label, (LPCTSTR)FullCommand, GetWorkingStateName());
      SaveLogMsg(ForLog);
    }

    TRACE("\nCommand '%s' received", (LPCTSTR)FullCommand);

    FXPropertyKit Command;
    if (!FullCommand.GetString("Command", Command))
      Command = FullCommand;
    m_dLastCommandTime = GetHRTickCount();
    Command.MakeLower();
    Command.Trim(" \t\n\r");

    if (Label.Find(_T("Timeout")) == 0)// Check for timeout in scan process
    {
      SEND_GADGET_ERR("Timeout on image grab, New grab initiated");
      //       GrabImage();
      // ASSERT( 0 ) ;
      return true;
    }
    if (Label.Find("Trigger_mode") == 0)
    {
      FXString TriggerMode;
      if (Command.GetString("trigger_mode", TriggerMode))
        m_TriggerMode = (TriggerMode == "on") ? TM_OneFrame : TM_NoTrigger;
      return true;
    }
    if ( Label.Find( "FromFRender" ) == 0 )
    {
      if (!m_bWaitAnswerFromFRender)
        return false; // did not wait for operator answer
      FXPropertyKit pk = pCommand->GetString();
      CPoint CursorPt;
      if (pk.GetInt(_T("x"), (int&) CursorPt.x)
        && pk.GetInt(_T("y"), (int&) CursorPt.y))
      {
        if ( m_bWaitAnswerFromFRender )
        {
          if ( m_FinishRect.PtInRect(CursorPt) )
          {
            SendMessageToEngine( "Finished;// By Operator command" , "GrindingFinished" );
            SetIdleStage();
            m_bMotorForFrontGrinding = false;
            m_InfoAboutLastResults += "Finished";

            SaveGrindingLogMsg( "Polishing finished by operator command" );
            m_bWaitAnswerFromFRender = false ;
          }
          else if ( m_ContinueRect.PtInRect(CursorPt) )
          {
            m_bWaitAnswerFromFRender = false ;
            SetFrontLight( false , false ) ;
            SetBackLight( true ) ;

            StartMotor();
            m_bDone = false;
            m_bMotorForFrontGrinding = true;
            m_dLastMotionFinishedTime = GetHRTickCount();
          }
        }
        return true;
      }
      return false;
    }
    if (Command.Find("startmeasurements") == 0)
    {
      m_bDoMeasurement = true;
      if (m_WorkingMode == MPPD_Front)
        SendMessageToEngine("OK;//MeasurementsEnabled", "MeasurementsEnabled");
      return true;
    }
    if (Command.Find("stopmeasurements") == 0)
    {
      m_bDoMeasurement = false;
      if (m_WorkingMode == MPPD_Front)
      {
        SendMessageToEngine("OK;//MeasurementsDisabled", "MeasurementsDIsabled");
        SetBackLight(true);
        SetFrontLight(false);
      }
      return true;
    }
    int iSeparatorIndex = (int)Command.Find(':');
    if (iSeparatorIndex > 0)
    {
      Label = Command.Left(iSeparatorIndex);
      Command.Delete(0, iSeparatorIndex + 1);
    }
    MPPD_Stage NewState = STG_Idle;

    FXRegistry Reg("TheFileX\\MPP_Dispens");
    if (Command == _T("restart"))
    {
      m_iNProcessedParts = 0;
      SaveLogMsg("Restart");
      //       switch (m_WorkingMode)
      //       {
      //         case MPPTWM_Down:
      //           SaveCSVLogMsg( "Restart\nTime Stamp              ,   Cav#, dX um,  dY um, dZ um, dZAver , dZleft , dZRight , Stat" );
      //           break;
      //         case MPPTWM_UpFront:
      //           SaveCSVLogMsg( "Restart\nTime Stamp              , Blank#, dX um, dY um , dZ um, Stat" );
      //           break;
      //       }
      SendMessageToEngine("OK", "RestartReaction");
      SetIdleStage();
      m_bDoMeasurement = false;
      NewState = STG_Stopped;
      return true;
    }
    else if (Command.Find(_T("stop")) == 0
      || Command.Find(_T("abort")) == 0)
    {
      if (m_WorkingStage != STG_Idle)
        SEND_GADGET_INFO(_T("Stop received, Working Stage was %s set to IDLE"),
        GetWorkingStateName());
      SetIdleStage();
      NewState = STG_Idle;
      m_bDoMeasurement = false;
      m_Centering = CNTST_Idle;
      SetCameraTriggerParams(false);

      TRACE(", operation aborted");
      DeleteWatchDog();

      //SwitchOffConstantLight(true, true);
      return true;
    }
    else if (Command == _T("savepart"))
    {
      m_MPPD_Lock.Lock();
      CopyDataFromDialog(m_CurrentPart);
      m_CurrentPart.SaveDispenserData();
      m_MPPD_Lock.Unlock();
      RestoreKnownParts(m_CurrentPart.m_Name);
      return true;
    }
    else if (Command == _T("reset_counters"))
    {
      m_iNProcessedParts = 0;
      SaveLogMsg("Reset Counters");
      switch (m_WorkingMode)
      {
      case MPPD_Side:
        SaveCSVLogMsg("Restart\n");
        SaveCSVLogMsg(m_LastResults.GetCaption());
        m_LastResults.m_iDispenserNumber = 0;
        break;
      case MPPD_Front:
        SaveCSVLogMsg("Reset Counters\n");
        SaveCSVLogMsg(m_LastResults.GetCaption());
        m_LastResults.m_iDispenserNumber = 0;
        break;
      }
      SendMessageToEngine("OK", "ResetCountersReaction");
      return true;
    }
    else if (Command == "grab" || Command == "takeshot")
    {
      GrabImage();
      NewState = STG_ShortCommand;
      return true;
    }
    else if (Command.Find("part") == 0)
    {
      Dispenser Part;
      FXString Tmp;
      DWORD dwPartParametersMask = (DWORD)Command.GetString("part", Tmp);
      if (dwPartParametersMask)
      {
        RestoreKnownParts((LPCTSTR)Tmp.MakeUpper());
        SelectCurrentPart((LPCTSTR)Tmp.MakeUpper());
        Part = m_CurrentPart;
        strcpy_s(Part.m_Name, (LPCTSTR)Tmp.MakeUpper());
        ScanPartParameters(Command, Part);
        m_CurrentPart = Part;
        SendMessageToEngine("OK", "PartSettings");
        m_PartName = Part.m_Name;
        Part.SaveDispenserData();
        SelectCurrentPart(m_PartName);
        PropertiesReregistration();
        m_iIDmin_pix = ROUND(m_CurrentPart.m_dIDmin_um * 0.8 / m_dScale_um_per_pix);
        m_iIDmax_pix = ROUND(m_CurrentPart.m_dIDmax_um * 1.2 / m_dScale_um_per_pix);
        m_iBlankIDmin_pix = ROUND(m_CurrentPart.m_dBlankIDmin_um * 0.8 / m_dScale_um_per_pix);
        m_iBlankIDmax_pix = ROUND(m_CurrentPart.m_dBlankIDmax_um * 1.2 / m_dScale_um_per_pix);

        if (m_WorkingMode == MPPD_Front)
          SetImagingParameters(false);
        if (m_SetupObject)
          m_SetupObject->Update();

        m_bMinToMinForFrontGrinding = Reg.GetRegiInt(
          "Measurements", "MinToMinFrontGrindingEnable", 1);

        m_CurrentCSVLogFilePath = (m_CurrentLogDir + GetWorkingModeName()) + GetTimeStamp("Log", NULL)
          + Reg.GetRegiString("Data", "CsvFileSuffixAndExt", ".csv");
        SaveCSVLogMsg(m_LastResults.GetCaption());
        m_LastResults.m_iDispenserNumber = 0;
        return true;
      }
      SendMessageToEngine("Error; Bad format (minimum is 'part=<part type>')",
        (LPCTSTR)(m_GadgetName + _T("_PartSettings")));
      SEND_GADGET_ERR("Bad format in string %s  (expected 'part=<part type>')", (LPCTSTR)Command);
      return false;
    }
    else
    {
      m_bMeasureFocus = false;
      switch (m_WorkingMode)
      {
      case MPPD_Side:
        {
          if (Command == _T("done"))
          {
            FXString sOldState = GetWorkingStateName();
            m_bDone = true;
            FXRegistry Reg("TheFileX\\MPP_Dispens");
            int iLargeMoveTime_ms = Reg.GetRegiInt(
              "Motions", "CoarseLargeMotion_ms", 1000);
            m_dStoneToMasterDistance_um = Reg.GetRegiDouble(
              "Calibrations", "StoneToEdgeDist_um", 100.);
            m_dLastMotionFinishedTime = GetHRTickCount();
            m_dLastMinStoneToPartDist_um = 10000.;
            switch (m_WorkingStage)
            {
              //           case STG_Idle:
              //           {
              //             SendMessageToEngine("Failed;// imaging is in IDLE state", "FailedInIdle");
              //           }
              //           return true;
            case STG_CoarseTipPosition:
              {
                LPCTSTR pStoneMotionMsg = NULL;
                bool bMsgSent = false;
                if (!m_bStoneIsMeasured)
                {
                  Sleep(300);
                  pStoneMotionMsg = "Large;";
                  SendMessageToEngine("Large;", NULL);
                  m_dLastMinStoneToPartDist_um = 10000.;
                  m_dLastMotionFinishedTime = 0.;
                  m_iGoodPositionCounter = 0;
                }
                else
                  m_dLastMotionFinishedTime = GetHRTickCount();
                return true;
              }
              break;
            case STG_FineTipPosition:
              {
                m_dLastMotionFinishedTime = GetHRTickCount();
                m_iGoodPositionCounter = 0;
                return true;
              }
              break;
            case STG_SidePolishing:
            case STG_FrontStoneInit:
              {
                m_dLastMotionFinishedTime = GetHRTickCount();
              }
              break;
            }
          }
          else if (Command.Find(_T("domeasure")) == 0)
          {
            ProgramImaging("Task(3);");
            SetCameraTriggerParams(false, 0.);
            m_PreviousStage = STG_Idle;

            FXString PartName;
            if (Command.GetString("partname", PartName))
            {
              if (!SelectCurrentPart(PartName))
              {
                SendMessageToEngine("Error; No such part", "BadPartInDoMeasure");
                SEND_GADGET_ERR("Unknown part name %", (LPCTSTR)PartName);
              }
              else
                m_PartName = PartName;
              m_InfoAboutWhiteGrindingMeasurement.Empty();
            }
            m_CurrentPart.RestoreDispenserData(m_PartName);
            LoadAndUpdatePartParameters();

            NewState = m_WorkingStage = STG_SideMeasureAndView;
            m_bDoMeasurement = true;
            m_CurrentPart.ResetSideResults();
            return true;
          }
          else if ((Command.Find("docalib") == 0)
            || (Command.Find("startcalibration") == 0))
          {
            m_CurrentPart.RestoreDispenserData(m_PartName);
            if (!DecodeCalibrationPars(Command))
              return NULL;
            //SendMessageToEngine("SideLightOn", NULL);
            LoadAndUpdatePartParameters();
            ProgramImaging("Task(3);");
            SetCameraTriggerParams(false, 0.);
            m_PreviousStage = STG_Idle;
            m_WorkingStage = STG_SideCalibration;
            m_bDoMeasurement = true;
            m_ForWhatSync = SS_NoSync;
            m_CurrentPart.ResetSideResults();
            return true;
          }
          else if ((Command.Find(_T("startcoarssidepositioning")) >= 0)
            || (Command.Find(_T("startfinesidepositioning")) >= 0))
          {
            m_CurrentPart.RestoreDispenserData(m_PartName);
            LoadAndUpdatePartParameters();
            ProgramImaging("Task(3);");
            SetCameraTriggerParams(false, 0.);
            m_PreviousStage = STG_Idle;
            m_WorkingStage = (Command.Find("fine") > 0) ?
              STG_FineStoneMeas : STG_CoarseStoneMeas;
            m_bDoMeasurement = true;
            m_bSideStoneInitialAdjustmentIsFinished = false;

            m_ForWhatSync = SS_NoSync;
            m_bStoneIsMeasured = false;
            int iInitialMove_ms = Reg.GetRegiInt(
              "Motions", "InitialLargeMotion_ms", 7000);
            FXString Msg;
            Msg.Format("Large=%d;", iInitialMove_ms);
            SendMessageToEngine(Msg, NULL);
            m_bDone = false;
            return true;
          }
          else if (Command.Find(_T("starttippositioning")) >= 0)
          {
            m_CurrentPart.RestoreDispenserData(m_PartName);
            LoadAndUpdatePartParameters();
            ProgramImaging("Task(3);");
            SetCameraTriggerParams(false, 0.);
            SendMessageToEngine("Large;", NULL);
            m_PreviousStage = STG_Idle;
            m_WorkingStage = STG_CoarseTipPosition;
            m_bDoMeasurement = true;
            m_ForWhatSync = SS_NoSync;
            m_bStoneIsMeasured = false;
            m_bDone = false;
            return true;
          }
          else if (Command.Find("startfrontgrinding") == 0)
          {
            m_CurrentPart.RestoreDispenserData(m_PartName);
            m_WorkingStage = STG_FrontStoneInit;
            m_bDoMeasurement = true;
            m_dLastMotionFinishedTime = GetHRTickCount();
            ProgramImaging("Task(3);");
            SetCameraTriggerParams(false, 0.);
            m_AdditionalInfo4.Empty();
            m_bMinToMinForFrontGrinding = Reg.GetRegiInt(
              "Measurements", "MinToMinFrontGrindingEnable", 1);
            return true;
          }
          else if (Command.Find("startfrontpolishgrinding") == 0)
          {
            m_WorkingStage = STG_SidePolishing;
            m_bDoMeasurement = true;
            m_dLastMotionFinishedTime = GetHRTickCount();
            return true;
          }
          else if (Command.Find("startfinesidegrinding") == 0)
          {
            m_CurrentPart.RestoreDispenserData(m_PartName);
            m_WorkingStage = STG_SideFineGrinding;
            m_bDoMeasurement = true;
            m_dSideTipDiaBeforeFineGrinding_um = m_dLastTipOnSideDia_um = 0.;
            m_dLastMotionFinishedTime = GetHRTickCount();
            return true;
          }
          else if (Command.Find("startfinalmeasurement") == 0)
          {
            m_bMinToMinForFrontGrinding = Reg.GetRegiInt(
              "Measurements", "MinToMinFrontGrindingEnable", 1);
            m_CurrentPart.RestoreDispenserData(m_PartName);
            SetCameraTriggerParams(false);
            m_iFrameCntAfterDone = 0;
            SetExposureAndGain(m_iSideExposure_us, m_CurrentPart.m_iGainForWhite_dBx10);
            ProgramImaging("Task(3);");
            m_WorkingStage = STG_SideFinalMeasurement;
            m_bDoMeasurement = true;
            m_CurrentPart.ResetSideResults();

            return true;
          }
          else if (Command.Find("viewhandle") == 0)
          {
            FXPropertyKit pk(Command);
            FXString TargetAsString;
            if (pk.GetString("hs", TargetAsString))
            {
              CTextFrame * pText = CreateTextFrame((LPCTSTR)TargetAsString, "SetWndHandle");
              PutFrame(m_pOutput, pText);
              Sleep(100);
              SendMessageToEngine("OK; Handle for Side image is received", "Side_ViewHandle");
            }
            return NULL;
          }
          else if (Command.Find("liveview") == 0)
          {
            Command.Delete(0, 9);
            Command.Trim();
            FXString ViewMode;
            int iExpChange_perc = 0;
            FXPropertyKit pk(Command);
            if (pk.GetString("viewmode", ViewMode))
            {
              if (ViewMode == "focusmeasure"
                || ViewMode == "focus")
              {
                LoadAndUpdatePartParameters();
                NewState = m_WorkingStage = STG_FrontFocusing;
                m_bMeasureFocus = true;
                ResetIterations();
                SendMessageToEngine("OK", "Focus Measure View");
                CBooleanFrame * pLPFSwitch = CBooleanFrame::Create(true);
                PutFrame(GetOutputConnector(4), pLPFSwitch);
                Sleep(30);
                GrabImage();
                return true;
              }
              else
              {
                SendMessageToEngine("Error: unknown live view mode",
                  "ErrorOnView");
              }
            }
          }
          else if (Command.Find("sidelighton") == 0)
          {
            return true;
          }
          else if (Command.Find("sidelightoff") == 0)
          {
            return true;
          }
          else if (Command.Find("issidelighton") == 0)
          {
            m_iCheckSideLight = 3; // if camera is working, answer will be sent automatically
            return true;
          }
          else if (Command.Find("isblankok") == 0)
          {
            m_iBlankLengthCheck = 3; // if camera is working, answer will be sent automatically
            return true;
          }
          else if (Command.Find("switchlight") == 0)
          {
            // return for not show "unprocessed" command, which is for front mode
            return true;
          }
          break; // not found command
        } // end of MPPD_Side case

      case MPPD_Front:
        {
          if (Command == _T("done"))
          {
            FXString sOldState = GetWorkingStateName();
            m_iFrameCntAfterDone = 0;
            m_bDone = true;
            NewState = STG_Unknown;
            if (m_bReadyToGrind)
            {
              m_bReadyToGrind = false;
              if (m_WorkingStage == STG_FrontPolishing)
              {
                double dPolishStepForFrontGrinding = Reg.GetRegiDouble("Motions",
                  "PolishStepForFrontGrinding_um", 0.5);
                MoveFrontStone(-dPolishStepForFrontGrinding, "Polishing");
                SetFrontLight(false);
                SetBackLight(true);
                SetExposureAndGain(m_CurrentPart.m_iFrontForWhiteExposure_us,
                  m_CurrentPart.m_iGainForWhite_dBx10);
                m_bDoMeasurement = true;
              }
              else
              {
                NewState = m_WorkingStage = STG_FrontStoneTouchWait;
                m_bDoMeasurement = true;
                double dInitialCorrection = Reg.GetRegiDouble("Motions",
                  "FrontStoneInitialStepCorrection_um", 20.);
                MoveFrontStone(-m_dLastFrontDistaceToStone_um + dInitialCorrection, "Stone Touch Wait");
              }
              return 0;
            }
            switch (m_WorkingStage)
            {
              //           case STG_Idle:
              //           {
              //             m_Centering = CNTST_Idle;
              //             SetCameraTriggerParams(false);
              //             SendMessageToEngine("Failed;// imaging is in IDLE state",
              //               "FailedInIdle");
              //           }
              //           return true;
            case STG_FrontBlackCalibration:
              {
                m_bDoMeasurement = true;
                GrabImage();
              }
              break;
            case STG_FrontSynchronization:
              m_bMotorIsOn = true;
              NewState = STG_FrontSynchronization;
              m_bDoMeasurement = true;
              break;
            case STG_MoveXPlusCalibration:
            case STG_MoveXMinusCalibration:
            case STG_MoveYPlusCalibration:
            case STG_MoveYMinusCalibration:
              {
                m_iTriggerDelay0 = 0;
                m_iTriggerDelay1 = ROUND(m_dLastCalculatedRotationPeriod_ms / 2.);
                SetCameraTriggerParams(true, 0.);
                m_AdditionalMsgForManual.Empty();
                NewState = m_WorkingStage;
                m_bDoMeasurement = true;
              }
              break;
            case STG_CorrectX:
              {
                if (fabs(m_dYMotionDist_pix) > 0.5)
                {
                  DoAdjustForCentering(CM_MoveY, m_dYMotionDist_pix);
                  NewState = m_WorkingStage = STG_CorrectY;
                  break;
                }
              }  // intensionally no break!!!!!!!!
            case STG_CorrectY:
              SetGrabForCentering(STG_Get0DegImage);
              break;
            case STG_WaitForMotorStop:
              {
                FXString MsgForEngine;
                SetIdleStage();
                MsgForEngine.Format("Finished; // Ecc=%.2f(%.2f,%.2f) um;",
                  abs(cmplx(m_dLastXEccentricitet_um, m_dLastYEccentricitet_um)),
                  m_dLastXEccentricitet_um, m_dLastYEccentricitet_um);
                SendMessageToEngine(MsgForEngine, "Centering Finished");
                m_bDoMeasurement = false;
              }
              break;
            case STG_FrontStoneTouchWait:
              {
                SendMessageToEngine("Large;//STG_FrontStoneTouchWait", "FirstFrontGrinding");
                m_WorkingStage = STG_FrontGrinding;
                m_bDoMeasurement = true;
                SetFrontLight(false);
                SetBackLight(true);
                m_dLastMotionFinishedTime = GetHRTickCount();
                m_iFrameCntAfterDone = 0;
              }
              break;
            case STG_FrontGrinding:
            case STG_FrontPolishing:
              {
                m_dLastMotionFinishedTime = GetHRTickCount();
                FXString MsgToEngine = (m_WorkingStage == STG_FrontPolishing) ?
                  "Large;//Cycle STG_FrontPolishing" : "Large;//Cycle STG_FrontGrinding";
                if (m_bFrontMotionOrdered)
                {
                  m_bFrontMotionOrdered = false;
                  m_bMotorForFrontGrinding = false;
                  SendMessageToEngine(MsgToEngine, "OrderFrontGrinding");
                  m_bDone = false;
                  return true;
                }
                m_bDone = true;
                ProgramImaging("Task(4);");
                SetCameraTriggerParams(false);

                m_iFrameCntAfterDone = 0;
                m_bDoMeasurement = true;
                m_CurrentPart.ResetWhiteResults();

                break;
              }
            case STG_FrontWhiteFinalWithRotation:
              m_bMotorIsOn = true;
              break;
            default:
              break;
            }
            SaveOperativeLogMsg("%s: DONE Msg, OldState=%s NewState=%s",
              GetWorkingModeName(), (LPCTSTR)sOldState,
              GetWorkingStateName());
          }
          else if (Command.Find("switchlight") == 0)
          {
            if (Command.Find("back") > 0)
            {
              SetBackLight(true);
              SetFrontLight(false);
            }
            else if (Command.Find("front") > 0)
            {
              SetBackLight(false);
              SetFrontLight(true);
            }
            else if (Command.Find("off") > 0)
            {
              SetBackLight(false);
              SetFrontLight(false);
            }
            else if (Command.Find("frbright") > 0)
            {
              SetBackLight(false);
              SetFrontLight(true, true);
            }
            else
            {
              switch (m_iCurrentLightMask & (LightMask_Front | LightMask_Back))
              {
              default:
              case LightMask_Unknown: SetBackLight(true); break;
              case LightMask_Back:
                SetFrontLight(true);
                SetBackLight(false);
                break;
              case LightMask_Front:  SetFrontLight(false); break;
                break;
              }
            }

            if (Label != "Internal")
              SendMessageToEngine("OK;", "Light Switched");
            return true;
          }
          else if (Command.Find("trigger=") == 0)
          {
            bool bOn = Command.Find("on") > 0;
            SetCameraTriggerParams(bOn);
            if (Label.Find("Trig.") == -1)
              SendMessageToEngine("OK;", "Trigger Switched");
          }
          else if (Command.Find("dosync") == 0)
          {
            ProgramImaging("Task(4);");
            SetCameraTriggerParams(true, 0.);
            m_WorkingStage = STG_FrontSynchronization;
            m_bDoMeasurement = true;
            m_ForWhatSync = SS_SimpleSync;
            m_CaptureTimes.clear();
            return true;
          }
          else if (Command.Find("centering") >= 0)
          {
            m_InfoAboutWhiteGrindingMeasurement.Empty();
            m_AdditionalInfo3 = "Centering Process";
            bool bCalib = (Command.Find("startcalib") >= 0);
            bool bGrinding = (Command.Find("startgrinding") >= 0);
            bool bWhiteSpotStart = (Command.Find("start") >= 0) && !bCalib && !bGrinding;
            RestoreKnownParts(m_CurrentPart.m_Name);
            LPCTSTR pNoteForEngine = NULL;
            //           m_bCenteringWithBlack = ( bool ) Reg.GetRegiInt(
            //             "Centering" , "CenteringWithBlack" , 1 );
            //           FXString ImagingCommandForCentering =
            //             ( m_bCenteringWithBlack ) ? "Task(0);" : "Task(4)";
            FXString ImagingCommandForCentering = "Task(4)";
            //           bool bTrigger_mode = ( bool ) Reg.GetRegiInt(
            //             "Centering" , "TriggerModeForCentering" , 1 );
            bool bTrigger_mode = true;
            int iFrontExp = m_CurrentPart.m_iFrontForWhiteExposure_us;
            InitCentering();

            m_iIDmin_pix = ROUND(m_CurrentPart.m_dIDmin_um * 0.8 / m_dScale_um_per_pix);
            m_iIDmax_pix = ROUND(m_CurrentPart.m_dIDmax_um * 1.2 / m_dScale_um_per_pix);
            m_iBlankIDmin_pix = ROUND(m_CurrentPart.m_dBlankIDmin_um * 0.8 / m_dScale_um_per_pix);
            m_iBlankIDmax_pix = ROUND(m_CurrentPart.m_dBlankIDmax_um * 1.2 / m_dScale_um_per_pix);
            SetImagingParameters(bCalib || bGrinding);
            int iIDmin_pix = m_iBlankIDmin_pix;
            int iIDmax_pix = m_iIDmax_pix;
            m_ForWhatSync = SS_SyncForProduction;
            SetObjectPlacementAndSize(_T("back_light"),
              CSize(m_iBlankIDmin_pix, m_iBlankIDmin_pix),
              CSize(m_iIDmax_pix, m_iIDmax_pix));
            SetObjectPlacementAndSize(_T("circle_int"),
              CSize(m_iBlankIDmin_pix, m_iBlankIDmin_pix),
              CSize(m_iIDmax_pix, m_iIDmax_pix));

            ImagingCommandForCentering = "Task(4);";
            SetFrontLight(false);
            SetBackLight(true);
            pNoteForEngine = "Centering started;";
            m_bCenteringWithBlack = false;
            if (m_LastResults.m_dBlankInitialDI_um == 0.)
            {
              m_LastResults.m_ProcessingStartTimeStamp = GetTimeAsString_ms();
              m_LastResults.m_iDispenserNumber++;
              TCHAR Name[50];
              _stprintf_s(Name, _T("GrindingLog_%d_"), m_LastResults.m_iDispenserNumber);
              m_GrindingLogPath = m_CurrentLogDir + GetTimeStamp(Name, "") + ".log";
            }
            else if (m_LastResults.m_dBlankDOAfterCoarse_um = 0.)
              m_LastResults.m_dBlankDOAfterCoarse_um = m_dLastTipOnSideDia_um;
            StartMotor();
            m_CurrentPart.RestoreDispenserData(m_PartName);
            m_iNFramesForThrowing =
              Reg.GetRegiInt("Measurements", "NotUsedFirstFrames", 4);
            ProgramImaging(ImagingCommandForCentering);
            SetCameraTriggerParams(bTrigger_mode, 0.);
            SetExposureAndGain(m_CurrentPart.m_iFrontForWhiteExposure_us,
              m_CurrentPart.m_iGainForWhite_dBx10);
            m_WorkingStage = STG_FrontSynchronization;
            m_bDoMeasurement = true;
            m_CaptureTimes.clear();
            m_iFrameCntAfterDone = 0;
            //SendOKToEngine(/*pNoteForEngine*/);
            return true;
          }
          else if (Command.Find("domeasure") == 0)
          {
            m_CurrentPart.RestoreDispenserData(m_PartName);
            //           FXString ImagingCommandForMeasurement = Reg.GetRegiString(
            //             "Calibrations" , 
            //             (m_bUseFrontLight) ? "FrontMeasurementImagingCommand" 
            //             : "FrontMeasurementWhiteImagingCommand" ,
            //             (m_bUseFrontLight) ? "Task(0);" : "Task(2);" ) ;
            //           bool bTrigger_mode = ( bool ) Reg.GetRegiInt(
            //             "Calibrations" , "TriggerModeForCalibration" , 0 );
             //         ProgramImaging( ImagingCommandForMeasurement );
            ProgramImaging("Task(4);");
            SetCameraTriggerParams(false);
            if (m_bUseFrontLight)
            {
              SetExposureAndGain(m_CurrentPart.m_iFrontForBlackExposure_us,
                m_CurrentPart.m_iGainForBlack_dBx10);
              m_CurrentPart.ResetBlackResults();

            }
            else
            {
              SetExposureAndGain(m_CurrentPart.m_iFrontForWhiteExposure_us,
                m_CurrentPart.m_iGainForWhite_dBx10);
              m_CurrentPart.ResetWhiteResults();
            }

            m_WorkingStage = (m_bUseFrontLight) ? STG_FrontBlackMeasAndView : STG_FrontWhiteMeasAndView;
            m_bDoMeasurement = true;
            m_ForWhatSync = SS_SimpleSync;
            m_CaptureTimes.clear();
            return true;
          }
          else if (Command.Find("startfrontpolishgrinding") == 0) // Front polish 
          {
            m_CurrentPart.RestoreDispenserData(m_PartName);
            SetFrontLight(false);
            SetBackLight(true);
            SetExposureAndGain(m_CurrentPart.m_iFrontForWhiteExposure_us,
              m_CurrentPart.m_iGainForWhite_dBx10);
            Sleep(100);
            m_bReadyToGrind = true;
            m_iPolishSteps = 0;
            m_iFrameCntAfterDone = 0;
            m_bDone = false;
            m_WorkingStage = STG_FrontPolishing;
            m_bDoMeasurement = true;
            SendMessageToEngine("ReadyToGrind;//STG_FrontPolishing settled", "PolishReady");
            return true;
          }
          else if (Command.Find("startfinalmeasurement") == 0)
          {
            m_bUseWhiteForFinalDI = Reg.GetRegiInt("Measurements",
              "UseWhiteForFinalDI", 1);
            m_bUseWhiteForFinalMinMax = Reg.GetRegiInt("Measurements",
              "UseWhiteForFinalDIMinMax", 1);
            m_iAverageForFinalResults = Reg.GetRegiInt("Measurements",
              "AverageForFinalResults", 1);
            m_iFinalAverageCntr = 0;
            m_dFinalDIAveraged_um = m_dFinalDIMinAveraged_um =
              m_dFinalDIMaxAveraged_um = m_dTirWAveraged_um =
              m_dTirBAveraged_um = 0.;

            m_CurrentPart.RestoreDispenserData(m_PartName);
            SetCameraTriggerParams(false);
            m_iFrameCntAfterDone = 0;

            // We do white measurement first

            SetExposureAndGain(m_CurrentPart.m_iFrontForWhiteExposure_us,
              m_CurrentPart.m_iGainForWhite_dBx10);
            SetFrontLight(false);
            SetBackLight(true);
            m_CurrentPart.ResetWhiteResults();

            ProgramImaging("Task(4);");
            m_WorkingStage = STG_FrontWhiteFinalMeasurement;
            m_bDoMeasurement = true;
            m_ForWhatSync = SS_NoSync;
            m_CaptureTimes.clear();
            return true;
          }
          else if ((Command.Find("docalib") == 0)
            || (Command.Find("startcalibration") == 0))
          {
            m_CurrentPart.RestoreDispenserData(m_PartName);
            if (!DecodeCalibrationPars(Command))
              return NULL;
            SetBackLight(true);
            SetFrontLight(false);
            m_CurrentPart.ResetWhiteResults();
            ProgramImaging("Task(4)");
            SetCameraTriggerParams(false);
            SetExposureAndGain(m_CurrentPart.m_iFrontForWhiteExposure_us,
              m_CurrentPart.m_iGainForWhite_dBx10);
            m_WorkingStage = STG_FrontWhiteCalibration;
            m_bDoMeasurement = true;
            m_ForWhatSync = SS_SimpleSync;
            m_CaptureTimes.clear();
            m_dLastMotionFinishedTime = GetHRTickCount();
            m_iFrameCntAfterDone = 0;
            return true;
          }
          else if ((Command.Find(_T("startcoarssidepositioning")) >= 0)
            || (Command.Find(_T("startfinesidepositioning")) >= 0)
            || (Command.Find(_T("starttippositioning")) >= 0))
          {
            SetFrontLight(false);
            return true;
          }
          else if (Command.Find("liveview") == 0)
          {
            Command.Delete(0, 9);
            Command.Trim();
            FXString ViewMode;
            FXPropertyKit pk(Command);
            int iExpChange_perc = 0;
            if (pk.GetString("viewmode", ViewMode))
            {
              if (ViewMode == "focus")
              {
                LoadAndUpdatePartParameters();
                NewState = m_WorkingStage = STG_FrontFocusing;
                m_bDoMeasurement = true;
                SendMessageToEngine("OK", "Front View for focus");
                FXString CameraCommand;
                CameraCommand.Format("set properties"
                  "(shutter_us=%d;);", m_CurrentPart.m_iFrontForBlackExposure_us);
                ProgramCamera(CameraCommand);
                GrabImage();
              }
              if (ViewMode == "measure")
              {
                LoadAndUpdatePartParameters();
                NewState = m_WorkingStage = STG_FrontFocusing;
                SendMessageToEngine("OK", "Front View");
                //                 ProgramExposureAndLightParameters( m_iFrontExposure_us ,
                //                   m_iFrontExposure_us , m_iFrontExposure_us );
                GrabImage();
              }

            }
            else if (pk.GetInt("exposurechange", iExpChange_perc))
            {
              if (iExpChange_perc != 0)
              {
                FXString PartFolder("TheFileX\\MPP_Dispens\\PartsData");
                PartFolder = (PartFolder + "\\") + m_PartName;
                m_CurrentPart.RestorePartDataFromRegistry(
                  "TheFileX\\MPP_Dispens\\PartsData", m_PartName);
              }
              //                 ExpAsString.Format( "Exposure=%d us" , m_CurrentPart.m_Blank.m_iBlankExp_us ) ;
              //                SendMessageToEngine( "OK" , ExpAsString );
              GrabImage();
              return true;
            }
          }
          else if (Command.Find("viewhandle") == 0)
          {
            FXPropertyKit pk(Command);
            FXString TargetAsString;
            if (pk.GetString("hf", TargetAsString))
            {
              CTextFrame * pText = CreateTextFrame((LPCTSTR)TargetAsString, "SetWndHandle");
              PutFrame(m_pOutput, pText);
              Sleep(100);
              SendMessageToEngine("OK; Handle for Front image is received", "Front_ViewHandle");
            }
            if (pk.GetString("hs", TargetAsString))
            {
              CTextFrame * pText = CreateTextFrame((LPCTSTR)Command, "SetWndHandle");
              PutFrame(GetOutputConnector(2), pText);
            }

            return NULL;
          }
          else
            break;
        }
        return true;
      }
    }
    SENDWARN("Unprocessed command %s for working mode %s gadget %s",
      (LPCTSTR)Command, GetWorkingModeName(), (LPCTSTR)m_GadgetName);
  }
  return false;
}


Dispenser::Dispenser(LPCTSTR pPartName)
{
  RestoreDispenserData(pPartName);
}

void Dispenser::RestoreDispenserData(LPCTSTR pPartName)
{
  LPCTSTR pRegFolderName(_T("TheFileX\\MPP_Dispens\\PartData"));
  RestorePartDataFromRegistry(pRegFolderName, pPartName);
}

void Dispenser::SaveDispenserData(LPCTSTR pPartName)
{
  FXRegistry Reg("TheFileX\\MPP_Dispens\\PartData");
  if (!pPartName)
    pPartName = m_Name;

  Reg.WriteRegiString(pPartName, "Name", pPartName);
  Reg.WriteRegiDouble(pPartName, "IDmin_um", m_dIDmin_um);
  Reg.WriteRegiDouble(pPartName, "IDmax_um", m_dIDmax_um);
  Reg.WriteRegiDouble(pPartName, "BlankIDmin_um", m_dBlankIDmin_um);
  Reg.WriteRegiDouble(pPartName, "BlankIDmax_um", m_dBlankIDmax_um);
  Reg.WriteRegiDouble(pPartName, "IDMaxDiff_um", m_dIDMax_diff_um);
  Reg.WriteRegiDouble(pPartName, "ODmin_um", m_dODmin_um);
  Reg.WriteRegiDouble(pPartName, "ODmax_um", m_dODmax_um);
  Reg.WriteRegiDouble(pPartName, "ConuseAngleMin_deg", m_dConuseAngleMin_deg);
  Reg.WriteRegiDouble(pPartName, "ConuseAngleMax_deg", m_dConuseAngleMax_deg);
  Reg.WriteRegiDouble(pPartName, "InternalConuseAngle_deg", m_dInternalConuseAngle_deg);
  Reg.WriteRegiDouble(pPartName, "MaxEccentricity_um", m_dMaxEccentricity_um);
  Reg.WriteRegiInt(pPartName, "FrontForBlackExposure_us", m_iFrontForBlackExposure_us);
  Reg.WriteRegiInt(pPartName, "FrontForWhiteExposure_us", m_iFrontForWhiteExposure_us);
  Reg.WriteRegiInt(pPartName, "GainForWhite_dBx10", m_iGainForWhite_dBx10);
  Reg.WriteRegiInt(pPartName, "GainForBlack_dBx10", m_iGainForBlack_dBx10);
  Reg.WriteRegiInt(pPartName, "SideExposure_us", m_iSideExposure_us);
  Reg.WriteRegiInt(pPartName, "DiameterAverage", m_iDiameterAverage);
}

int Dispenser::RestorePartDataFromRegistry(
  LPCTSTR pFolderForParts, LPCTSTR pPartName, bool bSetDefault)
{
  if (!pFolderForParts)
    pFolderForParts = "TheFileX\\MPP_Dispens\\PartsData";
  string PartFolder(pFolderForParts);
  //PartFolder += '\\';
  FXRegistry Reg(PartFolder.c_str());

  if (bSetDefault)
  {
    m_dIDmin_um = 39.; // Target values
    m_dIDmax_um = 43.;
    m_dBlankIDmin_um = 18.; // min hole diameter before grinding
    m_dBlankIDmax_um = 33.; // max hole diameter before grinding
    m_dODmin_um = 210.;
    m_dODmax_um = 230.;
    m_dIDMax_diff_um = 3.;
    m_dMaxEccentricity_um = 2.5;
    m_dConuseAngleMin_deg = 29.;
    m_dConuseAngleMax_deg = 31.;
    m_dInternalConuseAngle_deg = 20.;
    m_iDiameterAverage = 3;
    m_iFrontForBlackExposure_us = 400;
    m_iFrontForWhiteExposure_us = 5000;
    m_iSideExposure_us = 400;
    m_iGainForWhite_dBx10 = 370; // may be some other units known by camera
    m_iGainForBlack_dBx10 = 450; // may be some other units known by camera
  }
  //  m_Name = Reg.GetRegiString( pPartName , "Name" , pPartName ) ;
  FXString Name = Reg.GetRegiString(pPartName, "Name", pPartName);
  strcpy_s(m_Name, (LPCTSTR)Name);

  m_dIDmin_um = Reg.GetRegiDouble(pPartName, "IDmin_um", m_dIDmin_um);
  m_dIDmax_um = Reg.GetRegiDouble(pPartName, "IDmax_um", m_dIDmax_um);
  m_dBlankIDmin_um = Reg.GetRegiDouble(pPartName, "BlankIDmin_um", m_dBlankIDmin_um);
  m_dBlankIDmax_um = Reg.GetRegiDouble(pPartName, "BlankIDmax_um", m_dBlankIDmax_um);
  m_dIDMax_diff_um = Reg.GetRegiDouble(pPartName, "IDMaxDiff_um", m_dIDMax_diff_um);
  m_dODmin_um = Reg.GetRegiDouble(pPartName, "ODmin_um", m_dODmin_um);
  m_dODmax_um = Reg.GetRegiDouble(pPartName, "ODmax_um", m_dODmax_um);
  m_dConuseAngleMin_deg = Reg.GetRegiDouble(
    pPartName, "ConuseAngleMin_deg", m_dConuseAngleMin_deg);
  m_dConuseAngleMax_deg = Reg.GetRegiDouble(
    pPartName, "ConuseAngleMax_deg", m_dConuseAngleMax_deg);
  m_dInternalConuseAngle_deg = Reg.GetRegiDouble(
    pPartName, "InternalConuseAngle_deg", m_dInternalConuseAngle_deg);
  m_dMaxEccentricity_um = Reg.GetRegiDouble(
    pPartName, "MaxEccentricity_um", m_dMaxEccentricity_um);
  m_iFrontForBlackExposure_us =
    Reg.GetRegiInt(pPartName, "FrontForBlackExposure_us", m_iFrontForBlackExposure_us);
  m_iFrontForWhiteExposure_us =
    Reg.GetRegiInt(pPartName, "FrontForWhiteExposure_us", m_iFrontForWhiteExposure_us);
  m_iSideExposure_us =
    Reg.GetRegiInt(pPartName, "SideExposure_us", m_iSideExposure_us);
  m_iGainForWhite_dBx10 = Reg.GetRegiInt(pPartName,
    "GainForWhite_dBx10", m_iGainForWhite_dBx10);
  m_iGainForBlack_dBx10 = Reg.GetRegiInt(pPartName,
    "GainForBlack_dBx10", m_iGainForBlack_dBx10);
  m_iDiameterAverage =
    Reg.GetRegiInt(pPartName, "DiameterAverage", m_iDiameterAverage);

  ResetAllResults();
  return 1;
}

FXString Dispenser::ResultsToString()
{
  FXString Result;
  double dIDWhiteResultMin_um;
  double dIDWhiteResultMax_um;
  double dTIRw_um;
  double dIDWhiteResult_um = GetDIWhiteAverageResults(    // White Processing result
    dIDWhiteResultMin_um, dIDWhiteResultMax_um, dTIRw_um);

  double dIDWhiteRotationResultMin_um;
  double dIDWhiteRotationResultMax_um;
  double dTIRwRotation_um;
  double dIDWhiteRotationResult_um = GetDIWhiteRotationAverageResults(    // White with 
    dIDWhiteRotationResultMin_um, dIDWhiteRotationResultMax_um, // rotation Processing result
    dTIRwRotation_um);

  double dIDBlackResultMin_um;
  double dIDBlackResultMax_um;
  double dTIRb_um;
  double dODBlackResult_um;
  double dIDBlackResult_um = GetBlackAverageResults(dIDBlackResultMin_um,
    dIDBlackResultMax_um, dODBlackResult_um, dTIRb_um);  // Black Processing result

  double dResultConuseAngle_deg;
  double dODSideResult_um = GetSideAverageResults(dResultConuseAngle_deg);    // Side processing results

  Result.Format("DIw=%.2f(%.2f,%.2f)um\n"
    "DIb=%.2f(%.2f,%.2f)um\n"
    "TIRw=%.2fum TIRb=%.2f\n"
    "DOb=%.2fum DOs=%.2f\n"
    "Angle=%.2fdeg\n"
    "Nw=%d Nb=%d Ns=%d Nwr=%d\n\n"
    "DIwr=%.2f(%.2f,%.2f)um\n"
    "TIRwr=%.2fum",
    dIDWhiteResult_um,
    dIDWhiteResult_um == 0. ? 0. : dIDWhiteResultMin_um,
    dIDWhiteResult_um == 0. ? 0. : dIDWhiteResultMax_um,
    dIDBlackResult_um,
    dIDBlackResult_um == 0. ? 0. : dIDBlackResultMin_um,
    dIDBlackResult_um == 0. ? 0. : dIDBlackResultMax_um,
    dTIRw_um, dTIRb_um,
    dODBlackResult_um, dODSideResult_um,
    dODSideResult_um == 0. ? 0. : dResultConuseAngle_deg,
    m_iWhiteResultCounter, m_iBlackResultCounter, m_iSideResultCounter,
    m_iWhiteRotationResultCounter,
    dIDWhiteRotationResult_um, dIDWhiteRotationResultMin_um, dIDWhiteRotationResultMax_um,
    dTIRwRotation_um);
  return Result;
}

FXString DispenserProcessingResults::ToCSVString()
{
  FXString Result;

  Result.Format("%s,%s,"      // Time stamps (begin and end)
    "%.2f,%.2f,%.2f,%.2f,"     // Blank Initial DIw and height diff 
    "%.2f,%.2f,%.2f,%.2f,"     // DOs after coarse and DI after centering
    "%.2f,%.2f,%.2f,%d,"       // DI after front grinding and # cycles
    "%.2f,"                    // DO after fine grinding 
    "%.2f,%.2f,%.2f,"          // DI after polishing and # cycles
    "%.2f,%.2f,%.2f,%.2f,%.2f" // Final DOs, DOb and DIw(with min max)
    "%.2f,%.2f,%.2f,%.2f,%.2f\n",  // Final DIb(with min max), TirB and TirW
    (LPCTSTR)m_ProcessingStartTimeStamp,
    (LPCTSTR)m_FinalMeasurementFinishedTimeStamp,
    m_dBlankInitialDI_um, m_dBlankInitialMinDI_um, m_dBlankInitialMaxDI_um,
    m_dBlankHeightDiffFromMaster_um,

    m_dBlankDOAfterCoarse_um, m_dBlankAfterSecondCenteringDI_um,
    m_dBlankAfterSecondCenteringMinDI_um, m_dBlankAfterSecondCenteringMaxDI_um,

    m_dPartAfterFrontGrindingDI_um, m_dPartAfterFrontGrindingMinDI_um,
    m_dPartAfterFrontGrindingMaxDI_um, m_iNGrindingCycles,

    m_dPartDOAfterFine_um,

    m_dPartAfterPolishingDI_um, m_dPartAfterPolishingMinDI_um,
    m_dPartAfterPolishingMaxDI_um, m_iNPolishingCycles,

    m_dPartFinalDOSide_um, m_dPartFinalDOFront_um, m_dPartFinalWDI_um,
    m_dPartFinalWMinDI_um, m_dPartFinalWMaxDI_um,

    m_dPartFinalBDI_um, m_dPartFinalBMinDI_um, m_dPartFinalBMaxDI_um,
    m_dTirB_um, m_dTirW_um
  );
  return Result;
}

FXString DispenserProcessingResults::GetCaption()
{
  FXString Result(_T("//Save Time, Begin time , End Time ,"
    "BlankDi, BMinDI , BMaxDI , Hdiff , DOcoarse , DIcent1 , DIc1min , DIc1max ,"
    "DIgrind, DIgrindMin , DIgrindMax , NFgrinds , DOfine , DIpolish , DIpolMin , DIpolMax , NFpolishes ,"
    "DOfinalS , DOfinalF , DIfinalW , DIfinalWmin , DOfinalWmax,"
    "DIfinalB, DIfinalBmin , DIfinalBmax , TirB , TirW\n"));
  return Result;
}

// returns number of parts
int MPPT_Dispens::SaveKnownParts()
{
  if (m_KnownParts.size() == 0)
    return 0;

  FXRegistry Reg("TheFileX\\MPP_Dispens\\PartsData");
  int iNSavedToRegistry = 0;
  for (auto it = m_KnownParts.begin(); it != m_KnownParts.end(); it++)
  {
    it->SaveDispenserData();
    //     if (it->m_Name == m_CurrentPart.m_Name)
    if (strcmp(it->m_Name, m_CurrentPart.m_Name) == 0)
    {
      Reg.WriteRegiDouble(it->m_Name, "LastIDCalib_um", m_dCalibIDia_um);
      Reg.WriteRegiDouble(it->m_Name, "LastODCalib_um", m_dCalibODia_um);
    }
  }

  FxSendLogMsg(MSG_INFO_LEVEL, _T("MPPT_Dispens::SaveKnownParts"), 0,
    _T("Parts Data saved to Registry' (%d records)"), iNSavedToRegistry);
  return iNSavedToRegistry;
}

int MPPT_Dispens::RestoreKnownParts(LPCTSTR pPartName, bool bReportToEngine)
{
  FXRegistry Reg("TheFileX\\MPP_Dispens");

  m_iAveragingC = Reg.GetRegiInt("Measurements", "PointsAveraging", 10);
  m_iNFramesForThrowing = Reg.GetRegiInt("Measurements", "NotUsedFirstFFrames", 4);
  m_dAllowedPtDeviation = Reg.GetRegiDouble("Measurements", "AllowedPtDeviation_pix", 3.);
  //   Reg.GetRegiCmplx( "Measurements" ,
  //     ( m_WorkingMode == MPPD_Front ) ? "NormFrontMainCrossCenter" : "NormSideMainCrossCenter" ,
  //     m_cNormMainCrossCenter , cmplx( 0.5 , 0.5 ) ) ;
  //   m_cMainCrossCenter = cmplx( m_cLastROICent_pix.real() * m_cNormMainCrossCenter.real() ,
  //     m_cLastROICent_pix.imag() * m_cNormMainCrossCenter.imag() ) ;
  m_cNormMainCrossCenter = cmplx();

  m_MPPD_Lock.Lock();
  m_iSelectedPart = -1;
  LPCTSTR pNewRegistryName = "TheFileX\\MPP_Dispens\\PartData";
  FXRegistry PartsNewReg(pNewRegistryName);
  FXStringArray PartsNames;
  PartsNewReg.EnumerateFolderForSubfolders("", PartsNames);
  if (PartsNames.GetCount())
  { // there we will be all times after new registry created
    FXString SelectedPart;
    if (pPartName)
    {
      PartsNewReg.WriteRegiString(
        "LastSettings", "SelectedPart", pPartName);
      SelectedPart = pPartName;
    }
    else
      SelectedPart = PartsNewReg.GetRegiString(
      "LastSettings", "SelectedPart", "");
    SelectedPart.MakeUpper();

    Dispensers Knowns;
    string KnownNames;

    for (int i = 0; i < PartsNames.Count(); i++)
    {
      if (PartsNames[i] != "LastSettings")
      {
        Dispenser NewPart(PartsNames[i]);
        Knowns.push_back(NewPart);
        KnownNames += PartsNames[i] + ';';
        if (NewPart.m_Name == SelectedPart)
        {
          m_iSelectedPart = (long)Knowns.size() - 1;
          m_SelectedPartName = SelectedPart;
          m_CurrentPart = Knowns[m_iSelectedPart];
          m_PartName = m_CurrentPart.m_Name;
          if (bReportToEngine)
            SendMessageToEngine("OK", (LPCTSTR)(m_GadgetName + _T("_AnswerForPart")));

          //           m_iIDmin_pix = ROUND(m_CurrentPart.m_dBlankIDmin_um * 0.8 / m_dScale_um_per_pix);
          //           m_iIDmax_pix = ROUND(m_CurrentPart.m_dIDmax_um * 1.2 / m_dScale_um_per_pix);
          // 
          //           int iRadius = (m_iIDmax_pix * 2) / 3;
          // 
          //           if (m_cAverageCenterFor0Deg.real() != 0.)
          //           {
          //             CRect ForKnownPos(ROUND(m_cAverageCenterFor0Deg.real()) - iRadius ,
          //               ROUND(m_cAverageCenterFor0Deg.imag()) - iRadius ,
          //               ROUND(m_cAverageCenterFor0Deg.real()) + iRadius ,
          //               ROUND(m_cAverageCenterFor0Deg.imag()) + iRadius);
          //             SetObjectPlacement(_T("circle_int") , ForKnownPos);
          //             SetObjectPlacement(_T("back_light") , ForKnownPos);
          //           }
          // 
          //           SetObjectPlacementAndSize(_T("circle_int") ,
          //             CSize(m_iIDmin_pix , m_iIDmin_pix) ,
          //             CSize(m_iIDmax_pix , m_iIDmax_pix));
          //           SetObjectPlacementAndSize(_T("back_light") ,
          //             CSize(m_iIDmin_pix , m_iIDmin_pix) ,
          //             CSize(m_iIDmax_pix , m_iIDmax_pix));
          // 
        }
      }
    }

    if (m_pKnownNames)
      delete m_pKnownNames;
    m_pKnownNames = new TCHAR[KnownNames.size() + 1];
    strcpy_s(m_pKnownNames, KnownNames.size() + 1, KnownNames.c_str());

    m_KnownParts.clear();
    m_KnownParts = Knowns;

    if (pPartName && (m_iSelectedPart == -1) && bReportToEngine)
      SendMessageToEngine("Error; No such part", (LPCTSTR)(m_GadgetName + _T("_AnswerForPart")));
    else if (m_iSelectedPart == -1)
    {
      m_iSelectedPart = 0;
    }
  }
  m_MPPD_Lock.Unlock();
  return 0;
}

FXString MPPT_Dispens::GetMainDir()
{
  FXRegistry Reg("TheFileX\\MPP_Dispens");
  FXString Directory = Reg.GetRegiString(
    "Data", "MainDirectory", "d:/DispensersData/");
  return Directory;
}

void MPPT_Dispens::SaveLogMsg(LPCTSTR pFormat, ...)
{
  FXString Out;
  Out.Format("\n%24s: ", GetTimeAsString_ms());
  va_list argList;
  va_start(argList, pFormat);
  FXString AsString;
  AsString.FormatV(pFormat, argList);
  va_end(argList);

  Out += AsString;
  ofstream myfile((LPCTSTR)m_CurrentLogFilePath, ios_base::app);
  if (myfile.is_open())
  {
    myfile << (LPCTSTR)Out;
    myfile.close();
  }
  else
  {
    SENDERR("MPPT_Dispens::SaveLogMsg ERROR: %s for file %s",
      strerror(GetLastError()), (LPCTSTR)m_CurrentLogFilePath);
  }
}
void MPPT_Dispens::SaveOperativeLogMsg(LPCTSTR pFormat, ...)
{
  FXString Out;
  Out.Format("\n%24s: ", GetTimeAsString_ms());
  va_list argList;
  va_start(argList, pFormat);
  FXString AsString;
  AsString.FormatV(pFormat, argList);
  va_end(argList);

  Out += AsString;
  ofstream myfile((LPCTSTR)m_CurrentOperativeLogFilePath, ios_base::app);
  if (myfile.is_open())
  {
    myfile << (LPCTSTR)Out;
    myfile.close();
  }
  else
  {
    SENDERR("MPPT_Dispens::SaveOperativeLogMsg ERROR: %s for file %s",
      strerror(GetLastError()), (LPCTSTR)m_CurrentOperativeLogFilePath);
  }
}

void MPPT_Dispens::SaveCSVLogMsg(LPCTSTR pFormat, ...)
{
  FXString Out;
  Out.Format("\n%24s, ", GetTimeAsString_ms());
  va_list argList;
  va_start(argList, pFormat);
  FXString AsString;
  AsString.FormatV(pFormat, argList);
  va_end(argList);

  Out += AsString;
  ofstream myfile((LPCTSTR)m_CurrentCSVLogFilePath, ios_base::app);
  if (myfile.is_open())
  {
    myfile << (LPCTSTR)Out;
    myfile.close();
  }
  else
  {
    SENDERR("MPPT_Dispens::SaveCSVLogMsg ERROR: %s for file %s",
      strerror(GetLastError()), (LPCTSTR)m_CurrentCSVLogFilePath);
  }
}

void MPPT_Dispens::SaveGrindingLogMsg(LPCTSTR pFormat, ...)
{
  FXString Out;
  Out.Format("\n%24s, ", GetTimeAsString_ms());
  va_list argList;
  va_start(argList, pFormat);
  FXString AsString;
  AsString.FormatV(pFormat, argList);
  va_end(argList);

  Out += AsString;
  ofstream myfile((LPCTSTR)m_GrindingLogPath, ios_base::app);
  if (myfile.is_open())
  {
    myfile << (LPCTSTR)Out;
    myfile.close();
  }
  else
  {
    SENDERR("MPPT_Dispens::SaveGrindingLogMsg ERROR: %s for file %s",
      strerror(GetLastError()), (LPCTSTR)m_CurrentCSVLogFilePath);
  }
}


// Check save  mode and save if necessary
int MPPT_Dispens::CheckAndSaveImage(const pTVFrame pImage, bool bFinal)
{
  if (!bFinal)
  {
    switch (m_SaveMode)
    {
    default:
    case DispensSaveMode_No: return 0;
    case DispensSaveMode_All: break;
    case DispensSaveMode_OnePerSerie:
      if (m_iAfterCommandSaved == 0)
        break;
      m_iFrameCount++;
      return 0;
    case DispensSaveMode_Decimate:
      if (m_iFrameCount % m_iSaveDecimator == 0)
        break;
      m_iFrameCount++;
      return 0;
    case DispensSave_Final:
      {
        switch (m_WorkingStage)
        {
          //        case ULF_MeasureAndCorrect:
        case ULS_ZCorrection2:
        case ULF_AfterSideCorrection:
        case DL_CaptureZWithCorrectHeigth:
        case DL_FinalImageOverPin: break;
        case DL_CaptureCavityFinalImage: // final image will be saved in
          // CheckAndSaveFinalImages together with graphical image
        default: return 0;
        }
      }
      break;
    case DispensSave_Bad:
      {
        return 0;
      }
      break;
    }
  }

  FXString FileName;
  FileName.Format("%s_%s%s%d.bmp", (LPCTSTR)GetTimeStamp(),
    GetWorkingModeName(), GetWorkingStateName(), m_iFrameCount++);

  return SaveImage(pImage, FileName);
}

// Check save  mode and save if necessary
int MPPT_Dispens::SaveImage(const pTVFrame pImage, LPCTSTR pFileName)
{

  m_CurrentDataDir = CheckCreatePartResultDir();
  if (m_CurrentDataDir.IsEmpty())
  {
    SENDERR("Can't create Main Data Directory for image saving");
    return 0;
  }
  FXString FileName = m_CurrentImagesDir + pFileName;

  return (0 != saveSH2BMP(FileName, pImage->lpBMIH, pImage->lpData));
}

FXString MPPT_Dispens::CheckCreatePartResultDir()
{
  FXString DirName = CheckCreateDataDir();
  if (DirName.IsEmpty())
    return DirName;

  FXString SubDir(GetWorkingModeName());
  SubDir += GetDateAsString() + _T("/");

  m_CurrentDataDir = DirName + SubDir;
  if (!FxVerifyCreateDirectory(m_CurrentDataDir))
  {
    SENDERR("Can't create result directory '%s' ", (LPCTSTR)m_CurrentDataDir);
    return FXString();
  };
  FXString ImagesDir = m_CurrentDataDir/* + _T( "Images/" )*/;
  if (!FxVerifyCreateDirectory(ImagesDir))
  {
    SENDERR("Can't create images directory '%s' ", (LPCTSTR)ImagesDir);
    return FXString();
  };
  m_CurrentImagesDir = ImagesDir;
  return m_CurrentDataDir;
}

FXString MPPT_Dispens::CheckCreateCurrentLogs()
{
  FXRegistry Reg("TheFileX\\MPP_Dispens");
  FXString Directory = GetMainDir();
  FXString LogSubDir = Reg.GetRegiString("Data", "LogDir", "Logs/");

  FXString LogPath(Directory + LogSubDir);
  if (!FxVerifyCreateDirectory(LogPath))
  {
    SENDERR("Can't create data directory '%s' ", (LPCTSTR)LogPath);
    return FXString();
  };
  m_CurrentLogDir = LogPath;
  LogPath += (((GetDateAsString() + "_") + GetWorkingModeName()));
  m_CurrentCSVLogFilePath = LogPath
    + Reg.GetRegiString("Data", "CsvFileSuffixAndExt", ".csv");
  m_CurrentOperativeLogFilePath = LogPath + _T("OperativeLog.log");
  LogPath +=
    +Reg.GetRegiString("Data", "LogFileSuffixAndExt", ".log");

  return LogPath;
}

LPCTSTR MPPT_Dispens::GetWorkingModeName()
{
  switch (m_WorkingMode)
  {
  case MPPD_NotSet: return _T("NotSet");
  case MPPD_Side: return _T("Side");
  case MPPD_Front: return _T( "Front" );
  case Ako_Tek: return _T( "AkoTek" );
  }
  return "Unknown";
}

LPCTSTR MPPT_Dispens::GetWorkingStateName()
{
  switch (m_WorkingStage)
  {
  case STG_Unknown: return _T("STUnk_");
  case STG_Idle: return _T("STIdle_");
  case STG_Stopped: return _T("STStop_");
  case STG_ShortCommand: return _T("Short_");
  case STG_SideMeasureAndView: return _T("SideMeas");
  case STG_SideCalibration: return _T("SideCalib");
    // Front view and measurement
  case STG_FrontBlackMeasAndView: return _T("FronBlackMeas"); // For focusing
  case STG_FrontWhiteMeasAndView: return _T("FronWhiteMeas"); // For focusing
      // Master Centering
  case STG_MasterRawSync: return _T("MasterRawSync"); // Initial sync with rotation, one frame per sync
  case STG_MasterXMeasure: return _T("MasterXMeasure");       // Measure X shift
  case STG_MasterXAdjust: return _T("MasterXAdjust");        // X Command to motors
  case STG_MasterYMeasure: return _T("MasterYMeasure");       // Measure Y shift
  case STG_MasterYAdjust: return _T("STG_MasterYAdjust");        // Y command to motors
    // Master measurement
  case STG_MasterMeas: return _T("MasterMeas"); // Switch on side measurement and view
  case STG_MasterCalibration: return _T("MasterCalibration"); // Take and save calibration data
    // Side Stones measurement
  case STG_CoarseStoneMeas: return _T("CoarseStoneMeas");// Switch  on Coarse stone measurement and view
  case STG_FineStoneMeas: return _T("FineStoneMeas");         // Switch  on Fine stone measurement and view
    // Blank Centering by white
  case STG_BlankRawSync: return _T("BlankRawSync"); // Initial sync with rotation, one frame per sync
  case STG_BlankXMeasure: return _T("BlankXMeasure");       // Measure X shift
  case STG_BlankXAdjust: return _T("BlankXAdjust");        // X Command to motors
  case STG_BlankYMeasure: return _T("BlankYMeasure");       // Measure Y shift
  case STG_BlankYAdjust: return _T("BlankYAdjust");        // Y command to motors
    // Coarse side grinding
  case STG_SideCoarseGrinding: return _T("SideCoarseGrinding");
    // There will be centering by white
  case STG_SideFineGrinding: return _T("SideFineGrinding");
    // Front stone initial adjustment until grinding process first signs
  case STG_FrontStoneInit: return _T("FrontStoneInit");
    // Front grinding until last "5 microns"
  case STG_FrontGrinding: return _T("FrontGrinding");
    // Front polishing
  case STG_FrontPolishing: return _T("FrontPolishing");
  case STG_FrontSynchronization: return _T("FrontPSync");
  case STG_MoveXPlusCalibration: return _T("X+1000_");
  case STG_MoveXMinusCalibration: return _T("X-1000");
  case STG_MoveYPlusCalibration: return _T("Y+1000_");
  case STG_MoveYMinusCalibration: return _T("Y-1000_");
  case STG_Get0DegImage: return _T("Get0Deg_");
  case STG_Get90DegImage: return _T("Get90Deg_");
  case STG_Get180DegImage: return _T("Get180Deg_");
  case STG_Get270DegImage: return _T("Get270Deg_");
  case STG_CorrectX: return _T("CorrX_");
  case STG_CorrectY: return _T("CorrY_");
  case STG_WaitForMotorStop: return _T("WaitForMotorOff_");
  case STG_FrontWhiteFinalMeasurement: return _T("WhiteFinalMeas_");
  case STG_FrontBlackFinalMeasurement: return _T("BlackFinalMeas_");
  case STG_SideFinalMeasurement: return _T("SideFinalMeas_");
  case STG_FrontBlackCalibration: return _T("BlackCalibMeas_");
  case STG_FrontWhiteCalibration: return _T("WhiteCalibMeas_");

  }
  m_Info2.Format("Unknown_Id=%d_", m_WorkingStage);
  return m_Info2;
}

FXString MPPT_Dispens::CheckCreateDataDir()
{
  FXRegistry Reg("TheFileX\\Micropoint");
  FXString Directory = GetMainDir();
  FXString DataSubDir = Reg.GetRegiString("Data", "DataSubDir", "Data/");
  FXString DataDir(Directory + DataSubDir);
  if (!FxVerifyCreateDirectory(DataDir))
  {
    SENDERR("Can't create data directory '%s' ", (LPCTSTR)DataDir);
    return FXString();
  };
  FXString ReportsDir = Directory + _T("Reports\\");
  if (!FxVerifyCreateDirectory(ReportsDir))
  {
    SENDERR("Can't create reports directory '%s' ", (LPCTSTR)ReportsDir);
    return FXString();
  };
  m_CurrentReportsDir = ReportsDir;
  return DataDir;
}

static const char * pWorkingMode = "MPPD_Side;MPPD_Front;Ako-Tek;";
static const char * pImagingMode = "SimpleCOntours;ExternalByDiff";
bool MPPT_Dispens::ScanPropertiesBase(LPCTSTR text, bool& Invalidate)
{
  FXPropertyKit pk(text);
  // get scale for calculations
  pk.GetDouble("Scale_um_per_pix", m_dScale_um_per_pix);
  if (!pk.GetPtr("MsgOrigin", (VOID *&)m_pMsgOrigin))
    return true;
  m_pMsgOrigin = NULL;
  return false;
}
void MPPT_Dispens::PropertiesRegistration()
{
  if (m_KnownParts.empty())
    RestoreKnownParts();

  addProperty(SProperty::COMBO, "WorkingMode", &m_WorkingMode, SProperty::Long, pWorkingMode);
  SetChangeNotification(_T("WorkingMode"), ConfigParamChange, this);
  addProperty(SProperty::EDITBOX, "PartType", &m_PartName, SProperty::String);

  string KnownNames;
  for (auto it = m_KnownParts.begin(); it != m_KnownParts.end(); it++)
  {
    KnownNames += it->m_Name;
    KnownNames += ';';
  }

  if (m_pKnownNames)
    delete m_pKnownNames;
  m_pKnownNames = new TCHAR[KnownNames.size() + 1];
  strcpy_s(m_pKnownNames, KnownNames.size() + 1, KnownNames.c_str());
  addProperty(SProperty::COMBO, "SelectedPart", &m_iSelectedPart, SProperty::Long, m_pKnownNames);
  SetChangeNotification(_T("SelectedPart"), ConfigParamChange, this);
  SetChangeNotification(_T("PartType"), ConfigParamChange, this);
  addProperty(SProperty::EDITBOX, "MinExternalDia_um", &m_CurrentPart.m_dODmin_um, SProperty::Double);
  addProperty(SProperty::EDITBOX, "MaxExternalDia_um", &m_CurrentPart.m_dODmax_um, SProperty::Double);
  addProperty(SProperty::EDITBOX, "CalibExtDia_um", &m_dCalibODia_um, SProperty::Double);

  addProperty(SProperty::SPIN, "ViewMode", &m_iViewMode, SProperty::Int, 0, 100);
  addProperty(SProperty::EDITBOX, "Scale_um_per_pix", &m_dScale_um_per_pix, SProperty::Double);
  if (m_WorkingMode == MPPD_Front)
  {
    addProperty(SProperty::EDITBOX, "MinInternalDia_um", &m_CurrentPart.m_dIDmin_um, SProperty::Double);
    addProperty(SProperty::EDITBOX, "MaxInternalDia_um", &m_CurrentPart.m_dIDmax_um, SProperty::Double);
    addProperty(SProperty::EDITBOX, "BlankMinHoleDia_um",
      &m_CurrentPart.m_dBlankIDmin_um, SProperty::Double);
    addProperty(SProperty::EDITBOX, "BlankMaxHoleDia_um",
      &m_CurrentPart.m_dBlankIDmax_um, SProperty::Double);
    addProperty(SProperty::EDITBOX, "InternalConusAngle_deg",
      &m_CurrentPart.m_dInternalConuseAngle_deg, SProperty::Double);

    addProperty(SProperty::EDITBOX, "MaxInternalDiaDiff_um",
      &m_CurrentPart.m_dIDMax_diff_um, SProperty::Double);
    addProperty(SProperty::EDITBOX, "CalibIntDia_um", &m_dCalibIDia_um, SProperty::Double);
    addProperty(SProperty::EDITBOX, "MaxEccentricity_um",
      &m_CurrentPart.m_dMaxEccentricity_um, SProperty::Double);
    addProperty(SProperty::EDITBOX, "MaxPtDeviat_pix", &m_dMaxPtDeviation_pix, SProperty::Double);
    addProperty(SProperty::EDITBOX, "InternCorr_pix", &m_dInternalCorrection_pix, SProperty::Double);
    addProperty(SProperty::EDITBOX, "ExternCorr_pix", &m_dExternalCorrection_pix, SProperty::Double);
    addProperty(SProperty::EDITBOX, "DiffThres", &m_dImagingDiffThreshold, SProperty::Double);
    addProperty(SProperty::COMBO, "ImagingMode", &m_ImagingMode, SProperty::Long, pImagingMode);
  }
  else
  {
    addProperty(SProperty::EDITBOX, "MinConusAngle_deg",
      &m_CurrentPart.m_dConuseAngleMin_deg, SProperty::Double);
    addProperty(SProperty::EDITBOX, "MaxConusAngle_deg",
      &m_CurrentPart.m_dConuseAngleMax_deg, SProperty::Double);
  }
}

void MPPT_Dispens::ConfigParamChange(LPCTSTR pName, void* pObject, bool& bInvalidate, bool& bInitRescan)
{
  MPPT_Dispens * pGadget = (MPPT_Dispens*)pObject;
  if (pGadget)
  {
    if (!_tcsicmp(pName, _T("WorkingMode")))
    {
      if (pGadget->m_OldWorkingMode != pGadget->m_WorkingMode)
      {
        pGadget->m_OldWorkingMode = pGadget->m_WorkingMode;
        pGadget->m_bRestoreScales = true;
        bInitRescan = true;
        bInvalidate = true;
        pGadget->m_CurrentLogFilePath = pGadget->CheckCreateCurrentLogs();
        pGadget->SaveLogMsg("\nLog Initialized at %s\n",
          (LPCTSTR)GetTimeAsString_ms());
        pGadget->SaveCSVLogMsg("\nLog Initialized at %s\n",
          (LPCTSTR)GetTimeAsString_ms());
      }
    }
    else if (!_tcsicmp(pName, _T("SelectedPart")))
    {
      if ((pGadget->m_iSelectedPart >= 0) && (pGadget->m_iSelectedPart < (int)pGadget->m_KnownParts.size()))
      {
        FXString SelectedPart = pGadget->m_KnownParts[pGadget->m_iSelectedPart].m_Name;
        pGadget->m_MPPD_Lock.Lock();
        pGadget->m_CurrentPart.RestoreDispenserData(SelectedPart);
        pGadget->m_PartName = SelectedPart;
        pGadget->CopyDataToDialog(m_CurrentPart);
        pGadget->m_MPPD_Lock.Unlock();

        FXRegistry PartsNewReg("TheFileX\\MPP_Dispens\\PartData");
        PartsNewReg.WriteRegiString(
          "LastSettings", "SelectedPart", SelectedPart);
        pGadget->SetImagingParameters(false);
        if (pGadget->m_SetupObject)
          pGadget->m_SetupObject->Update();
        // if properties arrived from another gadget
        pGadget->NotifyOtherGadgetsAboutPartChange();
      }
    }
    else if (!_tcsicmp(pName, _T("PartType")))
    {
      if (!pGadget->m_PartName.IsEmpty())
      {
        strcpy(pGadget->m_CurrentPart.m_Name, (LPCTSTR)pGadget->m_PartName);
        pGadget->m_CurrentPart.SaveDispenserData();

        FXString SelectedPart(pGadget->m_CurrentPart.m_Name);
        pGadget->m_MPPD_Lock.Lock();
        pGadget->m_CurrentPart.RestoreDispenserData(SelectedPart);
        pGadget->m_PartName = SelectedPart;
        pGadget->CopyDataToDialog(m_CurrentPart);
        pGadget->m_MPPD_Lock.Unlock();

        pGadget->SetImagingParameters(false);
        FXRegistry PartsNewReg("TheFileX\\MPP_Dispens\\PartData");
        PartsNewReg.WriteRegiString(
          "LastSettings", "SelectedPart", SelectedPart);
        if (pGadget->m_SetupObject)
          pGadget->m_SetupObject->Update();
      }
    }
  }
}

bool MPPT_Dispens::NotifyOtherGadgetsAboutPartChange()
{
  // if properties arrived from another gadget
//   if ( !m_pMsgOrigin )
//   {
//     FXString PropertiesForOther;
//     PropertiesForOther.Format( "PartType=%s;MsgOrigin=%p" ,
//       ( LPCTSTR ) m_PartName , this ) ;
//     //Set property "SelectedPart" to another MPPT_Dispens gadgets about selected part change 
//     for ( size_t i = 0 ; i < m_ExistentGadgets.size() ; i++ )
//     {
//       MPPT_Dispens * pOtherGadget = m_ExistentGadgets[ i ] ;
//       if ( ( pOtherGadget != this ) && ( pOtherGadget != m_pMsgOrigin ) )
//       {
//         bool bDummyInvalidate = false ;
//         pOtherGadget->ScanProperties( PropertiesForOther , bDummyInvalidate ) ;
//       }
//     }
//     return true ;
//   }
  return false;
}

void MPPT_Dispens::CopyDataToDialog(Dispenser& Src)
{
  strcpy_s(m_CurrentPart.m_Name, Src.m_Name);
  m_CurrentPart.m_dIDmin_um = Src.m_dIDmin_um; // Targ
  m_CurrentPart.m_dIDmax_um = Src.m_dIDmax_um;
  m_CurrentPart.m_dIDMax_diff_um = Src.m_dIDMax_diff_um;
  m_CurrentPart.m_dODmin_um = Src.m_dODmin_um;
  m_CurrentPart.m_dODmax_um = Src.m_dODmax_um;
  m_CurrentPart.m_dMaxEccentricity_um = Src.m_dMaxEccentricity_um;
  m_CurrentPart.m_dConuseAngleMin_deg = Src.m_dConuseAngleMin_deg;
  m_CurrentPart.m_dConuseAngleMax_deg = Src.m_dConuseAngleMax_deg;
}

void MPPT_Dispens::CopyDataFromDialog(Dispenser& Dest)
{
  strcpy_s(Dest.m_Name, m_CurrentPart.m_Name);
  Dest.m_dIDmin_um = m_CurrentPart.m_dIDmin_um; // Targ
  Dest.m_dIDmax_um = m_CurrentPart.m_dIDmax_um;
  Dest.m_dIDMax_diff_um = m_CurrentPart.m_dIDMax_diff_um;
  Dest.m_dBlankIDmin_um = m_CurrentPart.m_dBlankIDmin_um;
  Dest.m_dBlankIDmax_um = m_CurrentPart.m_dBlankIDmax_um;
  Dest.m_dODmin_um = m_CurrentPart.m_dODmin_um;
  Dest.m_dODmax_um = m_CurrentPart.m_dODmax_um;
  Dest.m_dMaxEccentricity_um = m_CurrentPart.m_dMaxEccentricity_um;
  Dest.m_dConuseAngleMin_deg = m_CurrentPart.m_dConuseAngleMin_deg;
  Dest.m_dConuseAngleMax_deg = m_CurrentPart.m_dConuseAngleMax_deg;
}


int MPPT_Dispens::ScanPartParameters(LPCTSTR pAsString, Dispenser& Part)
{
  FXPropKit2 ParsAsString(pAsString);
  double dDI = 0.5 * (Part.m_dIDmin_um + Part.m_dIDmax_um);
  double dDITol = 0.5 * (Part.m_dIDmax_um - Part.m_dIDmin_um);
  int iParCnt = (int)ParsAsString.GetDouble("holed", dDI);
  iParCnt += (int)ParsAsString.GetDouble("holedtol", dDITol);
  if (iParCnt)
  {
    Part.m_dIDmin_um = dDI - dDITol;
    Part.m_dIDmax_um = dDI + dDITol;
  }
  int iAllParCnt = iParCnt;

  double dDO = 0.5 * (Part.m_dODmin_um + Part.m_dODmax_um);
  double dDOTol = 0.5 * (Part.m_dODmax_um - Part.m_dODmin_um);
  iParCnt = (int)ParsAsString.GetDouble("tipd", dDO);
  iParCnt += (int)ParsAsString.GetDouble("tipdtol", dDOTol);
  if (iParCnt)
  {
    Part.m_dODmin_um = dDO - dDOTol;
    Part.m_dODmax_um = dDO + dDOTol;
    iAllParCnt += iParCnt;
  }
  double dConuseAngle = 0.5 * (Part.m_dConuseAngleMax_deg + Part.m_dConuseAngleMin_deg);
  double dConuseAngleTol = 0.5 * (Part.m_dConuseAngleMax_deg - Part.m_dConuseAngleMin_deg);
  iParCnt = ParsAsString.GetDouble("conang", dConuseAngle);
  iParCnt += ParsAsString.GetDouble("conangtol", dConuseAngleTol);
  if (iParCnt)
  {
    Part.m_dConuseAngleMin_deg = dConuseAngle - dConuseAngleTol;
    Part.m_dConuseAngleMax_deg = dConuseAngle + dConuseAngleTol;
    Part.m_dInternalConuseAngle_deg = dConuseAngle - 10.;
    iAllParCnt += iParCnt;
  }
  iAllParCnt += (int)ParsAsString.GetDouble("blankholediamin", Part.m_dBlankIDmin_um);
  iAllParCnt += (int)ParsAsString.GetDouble("blankholediamax", Part.m_dBlankIDmax_um);
  iAllParCnt += (int)ParsAsString.GetDouble("tirtol", Part.m_dMaxEccentricity_um);
  iAllParCnt += (int)ParsAsString.GetDouble("internconang", Part.m_dInternalConuseAngle_deg);
  iAllParCnt += (int)ParsAsString.GetDouble("ellipttol", Part.m_dIDMax_diff_um);
  return iAllParCnt;
}

bool MPPT_Dispens::DecodeCalibrationPars(LPCTSTR AsString)
{
  FXPropertyKit pk(AsString);
  double dHoleDia = 0., dTipDia = 0.;
  if (pk.GetDouble("holed", dHoleDia))
  {
    if (dHoleDia >= m_CurrentPart.m_dIDmin_um
      && dHoleDia <= m_CurrentPart.m_dIDmax_um)
    {
      m_dCalibIDia_um = dHoleDia;
    }
    else
    {
      FXString Msg;
      Msg.Format("Failed; Hole Dia should be in range [%.1f,%.1f]um",
        m_CurrentPart.m_dIDmin_um, m_CurrentPart.m_dIDmax_um);
      SendMessageToEngine(Msg, NULL);
      return false;
    }
  }
  if (pk.GetDouble("tipd", dTipDia))
  {
    if (dTipDia >= m_CurrentPart.m_dODmin_um
      && dTipDia <= m_CurrentPart.m_dODmax_um)
    {
      m_dCalibODia_um = dTipDia;
    }
    else
    {
      FXString Msg;
      Msg.Format("Failed; Tip Dia should be in range [%.1f,%.1f]um",
        m_CurrentPart.m_dODmin_um, m_CurrentPart.m_dODmax_um);
      SendMessageToEngine(Msg, NULL);
      return false;
    }
  }
  switch (m_WorkingMode)
  {
  case MPPD_Side:
    if (dTipDia != 0. && dHoleDia != 0.)
      return false; // command is for front calibration
    return true;
  case MPPD_Front:
    if (dTipDia != 0. && dHoleDia == 0.)
      return false;// side calibration
    if (dTipDia != 0. && dHoleDia != 0.)
      return true; // front calibration
  }
  SendMessageToEngine("Failed; No data about calibration sizes", NULL);
  return false;
}

int MPPT_Dispens::DrawStandardGraphics(CContainerFrame * pMarking)
{
  if (m_iViewMode > 0)
  {
    // Draw center cross over full screen
    cmplx cCentUp(m_cMainCrossCenter.real(), 0.);
    cmplx cCentDown(m_cMainCrossCenter.real(), (double)m_LastFrameRect.bottom);
    cmplx cCentLeft(0., m_cMainCrossCenter.imag());
    cmplx cCentRight((double)m_LastFrameRect.right, m_cMainCrossCenter.imag());
    CFigureFrame * pVertical = CreateLineFrame(cCentUp, cCentDown, 0xc0c000);
    CFigureFrame * pHorizontal = CreateLineFrame(cCentLeft, cCentRight, 0xc0c000);
    pMarking->AddFrame(pVertical);
    pMarking->AddFrame(pHorizontal);

    // Draw Info Texts
    if (m_iViewMode > 3)
    {
      pMarking->AddFrame(CreateTextFrame(
        cmplx(10., m_cLastROICent_pix.imag() * 1.1), "0x00ff00", 12,
        "TSView", 0, "%s", (LPCTSTR)GetTimeAsString_ms()));
      if (m_cAverageCenterFor0Deg.real() != 0.)
        pMarking->AddFrame(CreatePtFrame(m_cAverageCenterFor0Deg, "color=0xff8080;Sz=5;", "LastCenter"));
      if (!m_AdditionalInfo2.IsEmpty())
      {
        cmplx cResultView = cmplx(10., m_cLastROICent_pix.imag() * 1.5);
        pMarking->AddFrame(CreateTextFrame(cResultView, m_AdditionalInfo2, 0x40ff00, 14));
      }
      if (!m_AdditionalInfo3.IsEmpty())
      {
        cmplx cResultView = cmplx(10., m_cLastROICent_pix.imag() * 1.65);
        pMarking->AddFrame(CreateTextFrame(cResultView, m_AdditionalInfo3, 0x40ff00, 14));
      }
      if (!m_AdditionalInfo4.IsEmpty())
      {
        cmplx cResultView = cmplx(m_cLastROICent_pix.real() * 1.4, m_cLastROICent_pix.imag());
        pMarking->AddFrame(CreateTextFrame(cResultView, m_AdditionalInfo4, 0x40ff00, 14));
      }
      if ((m_iViewMode > 5))
      {
        if (!m_InfoAboutWhiteGrindingMeasurement.IsEmpty())
        {
          LPCTSTR pColor = (m_dLastDiaDiffToNominal_um > 0.) ?
            "0x00ffff" : (m_dLastDiaDiffToMax_um > 0.) ? "0x00ff00" : "0x0000ff";
          cmplx ViewDiaPt(m_cLastROICent_pix.real() * 1.25, 100.);

          pMarking->AddFrame(CreateTextFrame(ViewDiaPt,
            pColor, 14, _T("FinalWhiteInfo"), 0,
            (LPCTSTR)m_InfoAboutWhiteGrindingMeasurement));
        }
        if (!m_InfoAboutLastResults.IsEmpty())
        {
          LPCTSTR pColor = (m_dLastDiaDiffToNominal_um > 0.) ?
            "0x00ffff" : (m_dLastDiaDiffToMax_um > 0.) ? "0x00ff00" : "0x0000ff";
          cmplx ViewDiaPt(m_cLastROICent_pix.real() * 1.25, m_cLastROICent_pix.imag() * 1.3);

          pMarking->AddFrame(CreateTextFrame(ViewDiaPt,
            pColor, 14, _T("FinalWhiteInfo"), 0,
            (LPCTSTR)m_InfoAboutLastResults));
        }
      }
    }

    // Draw patterns
    if (m_iViewMode > 2)
    {
      switch (m_WorkingMode)
      {
      case MPPD_Front:
        break;
      case MPPD_Side:
        if (m_cCalibSideLowerLeftCorner.imag() != 0.) // Check that calibration coordinates are initiated
        {
          // Draw calibrated position
          pMarking->AddFrame(CreateLineFrame(m_cCalibSideUpperRightCorner, m_cCalibSideUpperLeftCorner,
            0x008000, "ConeEdge"));
          pMarking->AddFrame(CreateLineFrame(m_cCalibSideLowerRightCorner, m_cCalibSideLowerLeftCorner,
            0x008000, "ConeEdge"));
          pMarking->AddFrame(CreateLineFrame(m_cCalibSideUpperRightCorner, m_cCalibSideLowerRightCorner,
            0x008000, "ConeEdge"));

          cmplx cUpperSideVector = m_cCalibSideUpperRightCorner - m_cCalibSideUpperLeftCorner;
          double dDistToStone_pix = m_dStoneToMasterDistance_um / m_dScale_um_per_pix;
          cmplx cToStoneDist = -GetOrthoLeft(GetNormalized(cUpperSideVector))
            * dDistToStone_pix;
          m_cLeftTopStoneTargetMark = m_cCalibSideUpperLeftCorner + cToStoneDist;
          m_cRightTopStoneTargetMark = m_cCalibSideUpperRightCorner + cToStoneDist;
          m_LastStoneTargetLine.Update(m_cLeftTopStoneTargetMark, m_cRightTopStoneTargetMark);
          pMarking->AddFrame(CreateLineFrame(m_cLeftTopStoneTargetMark, m_cRightTopStoneTargetMark,
            0xff0000, "StoneTarget"));
        }
        break;
      }
    }
  }
  return 0;
}

CColorSpot * MPPT_Dispens::GetSpotData(LPCTSTR pName)
{
  return ::GetSpotData(m_LastSpots, pName);
}


void MPPT_Dispens::StartMotor()
{
  m_bMotorIsOn = false;
  SendMessageToEngine("StartMotor;", NULL);
}

void MPPT_Dispens::StopMotor()
{
  m_bMotorIsOn = true;
  SendMessageToEngine("StopMotor;", NULL);
}
void MPPT_Dispens::MoveFrontStone(double dDist, LPCTSTR pComments)
{
  FXString MoveCommand;
  MoveCommand.Format("MoveFw=%.2f;", dDist);
  if (pComments)
  {
    MoveCommand += "//";
    MoveCommand += pComments;
  }
  SendMessageToEngine(MoveCommand, "MoveFrontStone");
  m_dLastMotionFinishedTime = 0.;
  m_bDone = false;
  m_bFrontMotionOrdered = true;
}

void MPPT_Dispens::GetCenteringParameters()
{
  FXRegistry Reg("TheFileX\\MPP_Dispens");
  // Scales
  Reg.GetRegiCmplx("Calibrations", "cXPlusScale_pix_per_second",
    m_cXScalePlus_pix_per_sec, m_cXScalePlus_pix_per_sec);
  Reg.GetRegiCmplx("Calibrations", "cXMinusScale_pix_per_second",
    m_cXScaleMinus_pix_per_sec, m_cXScaleMinus_pix_per_sec);
  Reg.GetRegiCmplx("Calibrations", "cYPlusScale_pix_per_second",
    m_cYScalePlus_pix_per_sec, m_cYScalePlus_pix_per_sec);
  Reg.GetRegiCmplx("Calibrations", "cYMinusScale_pix_per_second",
    m_cYScaleMinus_pix_per_sec, m_cYScaleMinus_pix_per_sec);
  // Measured points
  Reg.GetRegiCmplx("Calibrations", "cLastInitialPos_pix",
    m_cLastInitialPosition, m_cLastInitialPosition);
  Reg.GetRegiCmplx("Calibrations", "cX+1000_Pos_pix",
    m_cLastXMovedPlusPosition, m_cLastXMovedPlusPosition);
  Reg.GetRegiCmplx("Calibrations", "cX-1000_Pos_pix",
    m_cLastXReturnedPosition, m_cLastXReturnedPosition);
  Reg.GetRegiCmplx("Calibrations", "cY+1000_Pos_pix",
    m_cLastYMovedPlusPosition, m_cLastYMovedPlusPosition);
  Reg.GetRegiCmplx("Calibrations", "cY-1000_Pos_pix",
    m_cLastYReturnedPosition, m_cLastYReturnedPosition);

}

bool MPPT_Dispens::CalculateAndSaveCenteringScales()
{
  m_cXScalePlus_pix_per_sec = (m_cLastXMovedPlusPosition - m_cLastInitialPosition);
  m_cXScaleMinus_pix_per_sec = (m_cLastXReturnedPosition - m_cLastXMovedPlusPosition);
  m_cYScalePlus_pix_per_sec = (m_cLastYMovedPlusPosition - m_cLastXReturnedPosition);
  m_cYScaleMinus_pix_per_sec = (m_cLastYReturnedPosition - m_cLastYMovedPlusPosition);
  double dXScalePlus = abs(m_cXScalePlus_pix_per_sec);
  double dXScaleMinus = abs(m_cXScaleMinus_pix_per_sec);
  double dYScalePlus = abs(m_cYScalePlus_pix_per_sec);
  double dYScaleMinus = abs(m_cYScaleMinus_pix_per_sec);
  double dXAnglePlus = RadToDeg(arg(m_cXScalePlus_pix_per_sec));
  double dXAngleMinus = RadToDeg(arg(m_cXScaleMinus_pix_per_sec));
  double dYAnglePlus = RadToDeg(arg(m_cYScalePlus_pix_per_sec));
  double dYAngleMinus = RadToDeg(arg(m_cYScaleMinus_pix_per_sec));
  FXRegistry Reg("TheFileX\\MPP_Dispens");
  // Scales
  Reg.WriteRegiCmplx("Calibrations",
    "cXPlusScale_pix_per_second", m_cXScalePlus_pix_per_sec);
  Reg.WriteRegiCmplx("Calibrations",
    "cXMinusScale_pix_per_second", m_cXScaleMinus_pix_per_sec);
  Reg.WriteRegiCmplx("Calibrations",
    "cYPlusScale_pix_per_second", m_cYScalePlus_pix_per_sec);
  Reg.WriteRegiCmplx("Calibrations",
    "cYMinusScale_pix_per_second", m_cYScaleMinus_pix_per_sec);
  // Measured points
  Reg.WriteRegiCmplx("Calibrations",
    "cLastInitialPos_pix", m_cLastInitialPosition);
  Reg.WriteRegiCmplx("Calibrations",
    "cX+1000_Pos_pix", m_cLastXMovedPlusPosition);
  Reg.WriteRegiCmplx("Calibrations",
    "cX-1000_Pos_pix", m_cLastXReturnedPosition);
  Reg.WriteRegiCmplx("Calibrations",
    "cY+1000_Pos_pix", m_cLastYMovedPlusPosition);
  Reg.WriteRegiCmplx("Calibrations",
    "cY-1000_Pos_pix", m_cLastYReturnedPosition);

  Reg.WriteRegiString("Calibrations", "LastCenteringScalesMeasTime", GetTimeStamp());
  return true;
}

void MPPT_Dispens::SaveScales()
{
  m_dScale_um_per_pix = (m_dExternalScale_um_per_pixel
    + m_dInternalBlackScale_um_per_pixel) * 0.5;
  FXRegistry Reg("TheFileX\\MPP_Dispens");
  Reg.WriteRegiDouble("Calibrations", "FrontInternalScale_um_per_pixel", m_dInternalBlackScale_um_per_pixel);
  Reg.WriteRegiDouble("Calibrations", "FrontExternalScale_um_per_pixel", m_dExternalScale_um_per_pixel);
  Reg.WriteRegiDouble("Calibrations", "FrontAverageScale_um_per_pixel", m_dScale_um_per_pix);
  Reg.WriteRegiDouble("Calibrations", "CalibExternalDia_um", m_dCalibODia_um);
  Reg.WriteRegiDouble("Calibrations", "CalibInternalDia_um", m_dCalibIDia_um);
  Reg.WriteRegiDouble("Calibrations", "FrontInternalWhiteScale_um_per_pixel", m_dInternalWhiteScale_um_per_pixel);

  CTextFrame * pSetScaleToImaging = CreateTextFrame("Scale", (DWORD)0);
  pSetScaleToImaging->GetString().Format("Scale(%.7f,um)", m_dScale_um_per_pix);
  PutFrame(GetOutputConnector(MPPDO_Measurement_Control), pSetScaleToImaging);
  CTextFrame * pSetScaleToRender = CreateTextFrame("Scale&Units", (DWORD)0);
  pSetScaleToRender->GetString().Format("%.7f,um", m_dScale_um_per_pix);
  PutFrame(GetOutputConnector(MPPDO_VideoOut), pSetScaleToRender);

}

bool MPPT_Dispens::IsProcessingStage()
{
  switch (m_WorkingStage)
  {
  case STG_SideCalibration:
  case STG_FrontWhiteFinalMeasurement:
  case STG_FrontBlackFinalMeasurement:
  case STG_SideFinalMeasurement:
  case STG_FrontBlackCalibration: // calibration by front light
  case   STG_FrontWhiteCalibration: // calibration by back light
  case   STG_FrontSynchronization:
  case   STG_MoveXPlusCalibration:
  case   STG_MoveXMinusCalibration:
  case   STG_MoveYPlusCalibration:
  case   STG_MoveYMinusCalibration:
  case   STG_Get0DegImage:
  case   STG_Get90DegImage:
  case   STG_Get180DegImage:
  case   STG_Get270DegImage:
  case   STG_CorrectX:
  case   STG_CorrectY:
  case STG_Get0DegImageForAverage:
  case STG_WaitForMotorStop:
  case STG_MasterRawSync: // Initial sync with rotation, one frame per sync
  case STG_MasterXMeasure:       // Measure X shift
  case STG_MasterXAdjust:       // X Command to motors
  case STG_MasterYMeasure:       // Measure Y shift
  case STG_MasterYAdjust:        // Y command to motors
  case STG_MasterCalibration: // Take and save calibration data
  case STG_CoarseStoneMeas: // Switch  on Coarse stone measurement and view
  case STG_FineStoneMeas:          // Switch  on Fine stone measurement and view
  case STG_CoarseTipPosition:
  case STG_FineTipPosition:
  case STG_SideCoarseGrinding:
  case STG_SideFineGrinding:
  case STG_FrontStoneInit:
  case STG_FrontStoneTouchWait:
  case STG_FrontGrinding:
  case STG_FrontPolishing:
    return true;
  }
  return false;
}

bool MPPT_Dispens::IsStoneSideMeasurement()
{
  switch (m_WorkingStage)
  {
  case STG_CoarseStoneMeas: // Switch  on Coarse stone measurement and view
  case STG_FineStoneMeas:          // Switch  on Fine stone measurement and view
  case STG_CoarseTipPosition:
  case STG_FineTipPosition:
    return true;
  }
  return false;
}

int MPPT_Dispens::CreateQuestionsOnRender(cmplx& ViewPt,
  LPCTSTR pInfo, LPCTSTR * pQuestions, vector<CRect>& Zones ,
  CContainerFrame * pMarking)
{
  bool bMinOK = ( m_CurrentPart.m_dODmin_um <= m_dLastMinWhiteDI_um ) ;
  bool bMaxOKM1 = ( m_dLastMaxWhiteDI_um <= m_CurrentPart.m_dODmax_um - 1 ) ;
  bool bMaxOK = ( m_dLastMaxWhiteDI_um <= m_CurrentPart.m_dODmax_um ) ;
  DWORD dwColor = ( bMinOK && bMaxOKM1 ) ? 0x00ff00 : ( bMaxOK ? 0x00ffff : 0x0000ff ) ;

  return FormQuestionsOnRender( ViewPt , pInfo , pQuestions , 
    24 , dwColor , Zones , pMarking ) ;
}
