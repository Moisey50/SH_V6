
#include "StdAfx.h"
#include "ImagView.h"
#include "MeasurementSet.h"
#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif


extern int iMsgFromSMThreads;


CMeasurementSet::CMeasurementSet(CImageView * pView )
{
  m_pView = pView; m_pCaptureGadget = NULL;
  m_pRenderWnd = NULL; m_pRenderGadget = NULL;
  m_pProcessGadget = NULL;
  m_iActiveInput = -1;
  m_dCaptureFinishedAt = get_current_time();
  m_dScanTime_us = 262.25;
  m_iLastMaxIntensity = 0;
  m_pShMemControl = NULL;
  m_iTrigDelay_uS = 100;
  m_iDebouncing_units = 60;
  m_iAttemps = 0;

  m_hEVDataImg = CreateEvent(FALSE, TRUE, 0, NULL);
};


bool CMeasurementSet::CheckAndStopLiveVideo(
  DWORD dwTimeOut_ms)
{
  if (m_pView->m_LiveVideo)
  {
    m_pView->PostMessage(iMsgFromSMThreads, GRAB, 0);
    Sleep(250);
  }
  Dword dwStartTime = GetTickCount();
  while (!m_pView->m_bPlayBackMode && m_dCaptureFinishedAt == 0.0)
  {
    Sleep(20);
    if (GetTickCount() - dwStartTime > dwTimeOut_ms)
    {
      if (m_pShMemControl)
      {
        TRACE("\nGrab Timeout: %d, %s",
          (GetTickCount() - dwStartTime),
          m_pShMemControl->m_ShMemName);
        break;
      }
    }
  }
  if (m_dCaptureFinishedAt != 0)
  {
    m_iAttemps = 0;
    if (m_pShMemControl)
    {
      TRACE("\nGrab_1 Time = %10.6f, %s", (m_dCaptureFinishedAt - m_dCaptureStartTime), m_pShMemControl->m_ShMemName);
    }
    return true;
  }
  return false;
}

CAM_CONTROL_MSG_ID CMeasurementSet::Grab(int iNGrabs)
{
  CheckAndStopLiveVideo(100);
  CQuantityFrame * pQuan = CQuantityFrame::Create(0.);
  pQuan->ChangeId(0);
  if (!m_pView->m_pBuilder->SendDataFrame(pQuan, m_ImageOrBackName))
    pQuan->Release(pQuan);
  m_iActiveInput = 0;

  m_dCaptureStartTime = get_current_time();
  if (m_pView->m_bPlayBackMode)
  {
    CBooleanFrame * pCommand = CBooleanFrame::Create(iNGrabs != 0);
    pCommand->ChangeId(0);
    if (!m_pView->m_pBuilder->SendDataFrame(pCommand,
      m_OpenReadFileSwitchName))
    {
      m_Status.Format("Can't send command to %s",
        (LPCTSTR)(m_SetName));
      pCommand->Release(pCommand);
      return OPERATION_ERROR;
    }
  }
  else
  {
    CTextFrame * pCommand = CTextFrame::Create();
    pCommand->GetString().Format("set grab(%d)", iNGrabs);
    ResetEvent(m_hEVDataImg);
    if (!m_pView->m_pBuilder->SendDataFrame(pCommand,
      m_CaptureControlName))
    {
      m_Status.Format("Can't send command to %s",
        (LPCTSTR)(m_SetName));
      pCommand->Release(pCommand);
      SetEvent(m_hEVDataImg);
      return OPERATION_ERROR;
    }
  }
  //ASSERT( m_dCaptureFinishedAt != 0.0 ) ;
  m_SpotResults.Lock();
  m_Status = (iNGrabs) ? "Grab Started" : "Grab Stopped";
  m_dCaptureFinishedAt = 0.0;
  m_iMaxBlobNumber = -1;
  m_SpotResults.RemoveAll();
  m_LineResults.RemoveAll();
  m_ProcLResults.RemoveAll();
  m_SpotResults.Unlock();
  if ( !m_bTriggerMode )
  {
    int iNAttempts = 0;
    while ( iNAttempts++ < 5 )
    {
      if (WaitForSingleObject(m_hEVDataImg, 150) == WAIT_OBJECT_0)
        return GRAB ;
      CTextFrame * pCommand = CTextFrame::Create();
      pCommand->GetString().Format("set grab(%d)", iNGrabs);
      ResetEvent(m_hEVDataImg);
      if (!m_pView->m_pBuilder->SendDataFrame(pCommand,
        m_CaptureControlName))
      {
        m_Status.Format("Can't send command to %s",
          (LPCTSTR)(m_SetName));
        pCommand->Release(pCommand);
        SetEvent(m_hEVDataImg);
        return OPERATION_ERROR;
      }
    }
  }
  m_Status.Format("Can't GRAB for %s",
    (LPCTSTR)(m_SetName));
  SetEvent(m_hEVDataImg);

  return GRAB;
}

CAM_CONTROL_MSG_ID CMeasurementSet::GrabBack()
{
  CheckAndStopLiveVideo();
  if (m_iActiveInput != 1)
  {
    CQuantityFrame * pQuan = CQuantityFrame::Create(1.);
    pQuan->ChangeId(0);
    if (!m_pView->m_pBuilder->SendDataFrame(pQuan, m_ImageOrBackName))
      pQuan->Release(pQuan);
    m_iActiveInput = 1;
  }
  CTextFrame * pCommand = CTextFrame::Create();
  pCommand->ChangeId(0);
  pCommand->GetString().Format("set grab(%d)", 1);
  if (!m_pView->m_pBuilder->SendDataFrame(pCommand, m_CaptureControlName))
  {
    m_Status.Format("Can't send command to %s",
      (LPCTSTR)(m_SetName));
    pCommand->Release(pCommand);
    return OPERATION_ERROR;
  }
  else
  {
    m_Status = "Grab back Started";
    return GRABBACK;
  }
}
CAM_CONTROL_MSG_ID CMeasurementSet::ClearBack()
{
  //   CDataFrame * pCommand = CDataFrame::Create() ;
  //   pCommand->ChangeId(0) ;
  //   if ( !m_pView->m_pBuilder->SendDataFrame( pCommand , m_ClearBackName ) )
  //   {
  //     m_Status.Format( "Can't send ClearBack command to %s" , 
  //       (LPCTSTR) (m_SetName) ) ;
  //     pCommand->Release( pCommand ) ;
  //     return OPERATION_ERROR ;
  //   }

  CDataFrame * ResetBlackFrame = CDataFrame::Create(transparent);
  ResetBlackFrame->ChangeId(0);
  if (!m_pView->m_pBuilder->SendDataFrame(ResetBlackFrame, m_ClearBackName))
  {
    ResetBlackFrame->Release(ResetBlackFrame);
    return OPERATION_ERROR;
  }
  else
  {
    m_Status = "Clear Back Done";
    return CLEARBACK;
  }



  CTextFrame * pCommand = CTextFrame::Create();
  pCommand->ChangeId(0);
  if (!m_pView->m_pBuilder->SendDataFrame(pCommand, m_ClearBackName))
  {
    m_Status.Format("Can't send ClearBack command to %s",
      (LPCTSTR)(m_SetName));
    pCommand->Release(pCommand);
    return OPERATION_ERROR;
  }
  else
  {
    m_Status = "Clear Back Done";
    return CLEARBACK;
  }
}

CAM_CONTROL_MSG_ID CMeasurementSet::MeasureBlobs(
  double dNormThreshold)
{
  if (m_GraphMode > 0)
  {
    DWORD dwTimeOut = 3000;
    if (WaitForSingleObject(m_hEVDataImg, dwTimeOut) != WAIT_OBJECT_0)
      return GRAB_AND_MEASURE_IS_NOT_FINISHED;
  }

  CheckAndStopLiveVideo();
  if (dNormThreshold == 0.0)
  {
    double dStart = get_current_time();
    while ((m_dCaptureFinishedAt == 0.0)
      && (get_current_time() - dStart < 2000))
    {
      Sleep(10);
    }
    if (m_dCaptureFinishedAt == 0.0)
    {
      m_Status = "Grab is not finished";
      return GRAB_AND_MEASURE_IS_NOT_FINISHED;
    }
    m_Status.Format("Found %d blobs", m_SpotResults.GetSize());
    return (CAM_CONTROL_MSG_ID)m_SpotResults.GetSize();
  }
  AbstractCall(SET_NORM_THRESH, NULL, 0, dNormThreshold);
  CString Control("Task(0)");
  CTextFrame * p = CTextFrame::Create(Control);
  if (!m_pView->m_pBuilder->SendDataFrame(p, m_ProcessControlName))
    p->Release(p);
  m_Status = "Blob measurement mode is on";
  return MEASUREBLOBS;
}

CAM_CONTROL_MSG_ID CMeasurementSet::GetBlobParam(
  int iBlobNumber)
{
  if (m_GraphMode > 0)
  {
    DWORD dwTimeOut = 3000;
    if (WaitForSingleObject(m_hEVDataImg, dwTimeOut) != WAIT_OBJECT_0)
      return GRAB_AND_MEASURE_IS_NOT_FINISHED;
  }

  if (!WaitEndOfGrabAndProcess(2000))
  {
    m_Status.Format("ERROR: Timeout on grab and measure");
    return GRAB_AND_MEASURE_IS_NOT_FINISHED;
  }

  m_SpotResults.Lock();
  if (iBlobNumber == -1)
    iBlobNumber = m_iMaxBlobNumber;
  if (iBlobNumber < 0
    || iBlobNumber >= m_SpotResults.GetSize())
  {
    m_SpotResults.Unlock();
    m_Status.Format("ERROR: Blob %d doesn't exists", iBlobNumber);
    return UNAVAILABLE_INFO_REQUESTED;
  }

  CColorSpot mySpot = m_SpotResults.GetAt(iBlobNumber);
  m_SpotResults.Unlock();
  double dRat;
  if (mySpot.m_dLongDiametr && mySpot.m_dShortDiametr)
    dRat = mySpot.m_dLongDiametr / mySpot.m_dShortDiametr;
  else
    dRat = 0;//prevent dividing by zero
  if ((mySpot.m_dBlobHeigth <= 0)
    && (mySpot.m_OuterFrame.Height() > 0))
    mySpot.m_dBlobHeigth = mySpot.m_OuterFrame.Height();
  if ((mySpot.m_dBlobWidth <= 0)
    && (mySpot.m_OuterFrame.Width() > 0))
    mySpot.m_dBlobWidth = mySpot.m_OuterFrame.Width();

  if (mySpot.m_Area > 100. && mySpot.m_dBlobHeigth > 0 && mySpot.m_dBlobWidth > 0)
  {
    if (m_pView->m_ImParam.m_iDiffractionMeasurementMethod == 0)
    {
      m_Status.Format("%d %d %6.2f %6.2f %6.2f %5.2f %d %6.2f %6.2f %6.2f %6.2f %6.2f %8.1f %8.1f",
        ROUND(mySpot.m_SimpleCenter.x),
        ROUND(mySpot.m_SimpleCenter.y),
        mySpot.m_dAngle,
        mySpot.m_dBlobWidth,
        mySpot.m_dBlobHeigth,
        dRat,//mySpot.m_dLongDiametr / (mySpot.m_dShortDiametr > 0) ? mySpot.m_dShortDiametr : 1, 
        (int)mySpot.m_iMaxPixel,
        ((int)mySpot.m_iMaxPixel > 0) ?
        (100. *mySpot.m_dRDiffraction / mySpot.m_iMaxPixel)
        :
        0.,
        ((int)mySpot.m_iMaxPixel > 0) ?
        (100. *mySpot.m_dLDiffraction / mySpot.m_iMaxPixel)
        :
        0.,
        ((int)mySpot.m_iMaxPixel > 0) ?
        (100. *mySpot.m_dDDiffraction / mySpot.m_iMaxPixel)
        :
        0.,
        ((int)mySpot.m_iMaxPixel > 0) ?
        (100. *mySpot.m_dUDiffraction / mySpot.m_iMaxPixel)
        :
        0.,
        mySpot.m_dAngle,     //for two angle measure methods results presentation 
        mySpot.m_dCentral5x5Sum,
        mySpot.m_dSumOverThreshold
      );
      return GETBLOBPARAM;
    }
    else if (m_pView->m_ImParam.m_iDiffractionMeasurementMethod == 1)
    {
      if (mySpot.m_bDetailed) // Is full measurement done?
      {
        m_Status.Format("%d %d %6.2f %6.2f %6.2f %5.2f %d %6.2f %6.2f %6.2f %6.2f %6.2f %6.2f %8.1f %d %8.1f %6.2f %6.2f %g %g %g %g %g %g %g %g %g %8.2f %8.2f %8.2f %8.2f %8.2f %8.2f",
          ROUND(mySpot.m_SimpleCenter.x),
          ROUND(mySpot.m_SimpleCenter.y),
          mySpot.m_dAngle,
          mySpot.m_dBlobWidth,
          mySpot.m_dBlobHeigth,
          dRat,
          mySpot.m_iMaxPixel,
          100. *mySpot.m_dRDiffraction,
          100. *mySpot.m_dLDiffraction,
          100. *mySpot.m_dDDiffraction,
          100. *mySpot.m_dUDiffraction,
          100. *mySpot.m_dCentralIntegral,
          mySpot.m_dAngle,
          mySpot.m_dCentral5x5Sum,
          mySpot.m_Area,
          mySpot.m_dSumOverThreshold,
          mySpot.m_iGain,
          mySpot.m_iExposure,
          mySpot.m_dIntegrals[0],
          mySpot.m_dIntegrals[1],
          mySpot.m_dIntegrals[2],
          mySpot.m_dIntegrals[3],
          mySpot.m_dIntegrals[4],
          mySpot.m_dIntegrals[5],
          mySpot.m_dIntegrals[6],
          mySpot.m_dIntegrals[7],
          mySpot.m_dIntegrals[8],
          mySpot.m_ImgMoments.m_dM00,
          mySpot.m_ImgMoments.m_dM01,
          mySpot.m_ImgMoments.m_dM10,
          mySpot.m_ImgMoments.m_dM11,
          mySpot.m_ImgMoments.m_dM02,
          mySpot.m_ImgMoments.m_dM20);
      }
      else   // No, some data will be replaced for constants
      {
        m_Status.Format("%d %d 1000. %6.2f %6.2f -1000. %d 0 0 0 0 0 1000. %8.1f %8.1f",
          ROUND(mySpot.m_SimpleCenter.x),
          ROUND(mySpot.m_SimpleCenter.y),
          mySpot.m_dBlobWidth,
          mySpot.m_dBlobHeigth,
          mySpot.m_iMaxPixel,
          mySpot.m_dAngle,
          mySpot.m_dCentral5x5Sum,
          mySpot.m_dSumOverThreshold
        );
      }
      return GETBLOBPARAM;
    }
    m_Status.Format("ERROR: Unknown Measurement Blob method %d",
      m_pView->m_ImParam.m_iDiffractionMeasurementMethod);
    return UNKNOWN_MEASUREMENT_MODE;
  }
  else
    m_Status.Format("ERROR: Small blob area %4.0f or bad focus", mySpot.m_Area);
  return NO_APPROPRIATE_OBJECTS;
}

CAM_CONTROL_MSG_ID CMeasurementSet::MeasureLines(
  double dNormThreshold)
{
  if (m_GraphMode > 0)
  {
    DWORD dwTimeOut = 3000;
    if (WaitForSingleObject(m_hEVDataImg, dwTimeOut) != WAIT_OBJECT_0)
      return GRAB_AND_MEASURE_IS_NOT_FINISHED;
  }
  if (dNormThreshold == 0.0)
  {
    if (!WaitEndOfGrabAndProcess(2000))
    {
      m_Status.Format("ERROR: Timeout on grab and measure");
      return GRAB_AND_MEASURE_IS_NOT_FINISHED;
    }
    m_Status.Format("Found %d lines", m_ProcLResults.GetSize());
    return (CAM_CONTROL_MSG_ID)m_ProcLResults.GetSize();
  }
  AbstractCall(SET_NORM_THRESH, NULL, 0, dNormThreshold);
  CString Control("Task(1)");
  CTextFrame * p = CTextFrame::Create(Control);
  if (!m_pView->m_pBuilder->SendDataFrame(p, m_ProcessControlName))
    p->Release(p);
  m_Status = "Lines measurement mode is on";
  return MEASURELINES;
}

CAM_CONTROL_MSG_ID CMeasurementSet::GetLineParam(int iLineNumber)
{
  if (m_GraphMode > 0)
  {
    DWORD dwTimeOut = 3000;
    if (WaitForSingleObject(m_hEVDataImg, dwTimeOut) != WAIT_OBJECT_0)
      return GRAB_AND_MEASURE_IS_NOT_FINISHED;
  }

  if (!WaitEndOfGrabAndProcess(2000))
  {
    m_Status.Format("ERROR: Timeout on grab and measure");
    return GRAB_AND_MEASURE_IS_NOT_FINISHED;
  }

  if (iLineNumber < 0)
    iLineNumber = 0;

  m_ProcLResults.Lock();
  if (iLineNumber >= 0
    && iLineNumber < m_ProcLResults.GetCount())
  {
    CLineResult Line = m_ProcLResults[iLineNumber];
    m_ProcLResults.Unlock();
    double dLineThikness = Line.m_DRect.bottom - Line.m_DRect.top;
    if (dLineThikness > 5.)
    {
      m_Status.Format("%d %6.2f %6.2f %2.8f %8.1f %8.1f %d %d",
        iLineNumber,
        Line.m_Center.y, dLineThikness, Line.m_dAngle,
        Line.m_dExtremalAmpl, Line.m_dAverCent5x5,
        Line.m_ImageMinBrightness, Line.m_ImageMaxBrightness);
      return GETLINEPARAM;
    }
    m_Status.Format("ERROR: Small line %d thickness %4.0f",
      iLineNumber, dLineThikness);
    return NO_APPROPRIATE_OBJECTS;
  }
  m_ProcLResults.Unlock();
  m_Status.Format("ERROR: Line %d doesn't exists",
    iLineNumber);
  return NO_APPROPRIATE_OBJECTS;
}
CAM_CONTROL_MSG_ID CMeasurementSet::AbstractCall(
  long iCallID, LPCTSTR pszParameters, long iPar, double dPar)
{
  switch (iCallID)
  {
  case SET_GAIN:
  {
    //       m_View->m_fGain = (float)dPar;
    //       m_View->SetGain();
    break;
  }

  case FIND_BAD_PIXELS:
  {
    //       m_View->OnFindBadPixels();
    break;
  }
  case SET_ANALOG_OFFSET:
  {
    //       m_View->m_iAnalogOffSet = iPar;
    //       m_View->SetAnalogOffset(1);
    break;
  }
  case SET_NORM_THRESH:
  {
    if (abs(dPar - m_dLastBinThreshold) > 0.001 || iPar == 1)
    {
      SetThresholdForBinarization(dPar);
      m_pView->m_dNormThreshold = dPar;
      m_dLastBinThreshold = dPar;
      m_Status.Format("OK Threshold settled to %5.3f", dPar);
    }

    return MSG_ID_OK;
  }
  case SET_NORM_ROTATION_THRESH:
  {
    if (abs(dPar - m_dLastRotThreshold) > 0.001)
    {
      SetThresholdForRotation(dPar);
      m_pView->m_dRotThreshold = dPar;
      m_dLastRotThreshold = dPar;
      m_Status.Format("OK Rotation Threshold settled to %5.3f", dPar);
    }
    return MSG_ID_OK;
  }
  case SET_FIXED_THRESHOLD:
  {
    m_pView->m_ImParam.m_bFixThreshold = (iPar != 0);
    if (dPar > 0.  &&  dPar < 4095.)
      m_pView->m_ImParam.m_iThresholdValue = ROUND(dPar);
    break;
  }
  m_Status = "OK";
  return MSG_ID_OK;

  case GET_NEAREST_SPOT_NUMBER:
  {
    cmplx Center((double)iPar, dPar);
    int iNearest = -1;
    double dMinDist = 10e6;
    for (int i = 0; i < m_SpotResults.GetCount(); i++)
    {
      cmplx BlobC((double)m_SpotResults[i].m_SimpleCenter.x,
        (double)m_SpotResults[i].m_SimpleCenter.y);
      double dDist = abs(Center - BlobC);
      if (dDist < dMinDist)
      {
        dMinDist = dDist;
        iNearest = i;
      }
    }
    m_Status.Format("Nearest Spot is %d", iNearest);
    return (CAM_CONTROL_MSG_ID)iNearest;
  }
  break;
  case RESET_ALL_BACKGROUNDS:
    // m_View->m_Backgrounds.RemoveAll() ;
    break;
  case IS_GRAPH_BASED_PROCESSING:
    m_Status.Format("%d cameras connected", m_pView->m_MeasSets.GetCount());
    return (CAM_CONTROL_MSG_ID)m_pView->m_MeasSets.GetCount();
  case SET_DELAY_IN_SCANS:
  {
    //       m_View->m_iAnalogOffSet = iPar;
    //       m_View->SetAnalogOffset(1);
    break;
  }
  case SET_SCAN_TIME:
  {
    m_dScanTime_us = dPar;
    m_Status.Format("Scan time is %7.2f", m_dScanTime_us);
    return ABSTRACTCALL;
    break;
  }

  }
  m_Status.Format("Abstract call can't execute function %d", iCallID);
  return UNAVAILABLE_FUNCTION_REQUESTED;
}

CAM_CONTROL_MSG_ID CMeasurementSet::GetExposure(int& iExposure)
{
  if (m_iExposure > m_dScanTime_us / 4)
    iExposure = ROUND(m_iExposure / m_dScanTime_us);
  else
    iExposure = m_iExposure;
  m_Status = "OK";
  return GETEXPOSURE;
}
CAM_CONTROL_MSG_ID CMeasurementSet::SetExposure(int iExposure, BOOL bIn_us)
{
  if (!bIn_us)
  {
    double dNewExposure = iExposure * m_dScanTime_us;
    iExposure = ROUND(dNewExposure);
  }
  CString Control;
  Control.Format("set Shutt_uS(%d)", iExposure);
  CTextFrame * pCommand = CTextFrame::Create(Control);
  if (!m_pView->m_pBuilder->SendDataFrame(pCommand, m_CaptureControlName))
  {
    m_Status.Format("Can't send command to %s",
      (LPCTSTR)(m_SetName));
    pCommand->Release(pCommand);
    return OPERATION_ERROR;
  }
  else
  {
    m_iExposure = iExposure;


    CString Msg;
    for (int i = 0; i < m_pView->m_MeasSets.GetCount(); i++)
    {
      int iGain = 0;
      m_pView->m_MeasSets[i].GetGain(iGain);
      double dGain_dB = m_pView->m_dGainStep_dB * iGain;
      double dGain = pow(10.0, dGain_dB / 20.);
      int iExposure = m_pView->m_MeasSets[i].m_iExposure;
      CString CamInfo;
      CamInfo.Format("C%d E=%duS(%dscans) - G%d(x%4.1f ;%4.1f dB)  ",
        i, iExposure, ROUND(iExposure / m_dScanTime_us),
        iGain, dGain, dGain_dB);
      Msg += CamInfo;
    }
    char * pMsg = new char[Msg.GetLength() + 1];
    strcpy_s(pMsg, Msg.GetLength() + 1, (LPCTSTR)Msg);
    ::PostMessage(m_pView->m_hWnd, iMsgForLog, 0, (LPARAM)pMsg);
    return SETEXPOSURE;
  }

}
CAM_CONTROL_MSG_ID CMeasurementSet::GetGain(int& iGain)
{
  iGain = m_iGain;
  m_Status = "OK";
  return GETGAIN;
}
CAM_CONTROL_MSG_ID CMeasurementSet::SetGain(int iGain)
{
  if (m_GraphMode > 0 && m_GraphMode < 4) // we don't change Gain in Dalsa camera
    return SETGAIN;

  CString Control;
  Control.Format("set Gain_dBx10(%d)", iGain);
  CTextFrame * pCommand = CTextFrame::Create(Control);
  if (!m_pView->m_pBuilder->SendDataFrame(pCommand, m_CaptureControlName))
  {
    m_Status.Format("Can't send command to %s",
      (LPCTSTR)(m_SetName));
    pCommand->Release(pCommand);
    return OPERATION_ERROR;
  }
  else
  {
    m_iGain = iGain;
    CString Msg;
    for (int i = 0; i < m_pView->m_MeasSets.GetCount(); i++)
    {
      if (m_pView->m_MeasSets[i].m_GraphMode > 0)
      {
        return SETGAIN;
      }

      int iGain = 0;
      m_pView->m_MeasSets[i].GetGain(iGain);
      double dGain_dB = m_pView->m_dGainStep_dB * iGain;
      double dGain = pow(10.0, dGain_dB / 20.);
      int iExposure = m_pView->m_MeasSets[i].m_iExposure;
      CString CamInfo;
      CamInfo.Format("C%d E=%duS(%dscans) - G%d(x%4.1f ;%4.1f dB)  ",
        i, iExposure, ROUND(iExposure / m_dScanTime_us),
        iGain, dGain, dGain_dB);
      Msg += CamInfo;
    }
    char * pMsg = new char[Msg.GetLength() + 1];
    strcpy_s(pMsg, Msg.GetLength() + 1, (LPCTSTR)Msg);
    ::PostMessage(m_pView->m_hWnd, iMsgForLog, 0, (LPARAM)pMsg);
    return SETGAIN;
  }
}
CAM_CONTROL_MSG_ID CMeasurementSet::SetTriggerDelay(int iTriggerDelay_uS, int iSet)
{
  //return SETTRIGGERDELAY ;//unable to set during the work
  CString Control;
  Control.Format("set TriggerDelay(%d)", iTriggerDelay_uS);
  CTextFrame * pCommand = CTextFrame::Create(Control);
  if (!m_pView->m_pBuilder->SendDataFrame(pCommand, m_CaptureControlName))
  {
    m_Status.Format("Can't send command to %s",
      (LPCTSTR)(m_SetName));
    pCommand->Release(pCommand);
    return OPERATION_ERROR;
  }
  else
  {
    if (iSet > 0)
      m_pView->m_MeasSets[iSet].m_iTrigDelay_uS = iTriggerDelay_uS;
    m_iTrigDelay_uS = iTriggerDelay_uS;
    CString Msg;
    Msg.Format("%s TriggerDelay=%d uS", m_SetName, iTriggerDelay_uS);
    char * pMsg = new char[Msg.GetLength() + 1];
    strcpy_s(pMsg, Msg.GetLength() + 1, (LPCTSTR)Msg);
    ::PostMessage(m_pView->m_hWnd, iMsgForLog, 0, (LPARAM)pMsg);
    return SETTRIGGERDELAY;
  }
}
CAM_CONTROL_MSG_ID CMeasurementSet::GetTriggerDelay(int& iTriggerDelay_uS)
{
  iTriggerDelay_uS = m_iTrigDelay_uS;
  m_Status = "OK";
  return GETTRIGGERDELAY;
}
CAM_CONTROL_MSG_ID CMeasurementSet::SaveLastPicture(int iCamNum)
{
  //   bool bSave;
  //   if (iSave == 1)
  //     bSave = true;
  //   else 
  //     bSave = false;
  m_pView->SetBMPPath("");
  m_pView->SaveLastPicture(iCamNum);

  return SAVEPICTURE;
}

CAM_CONTROL_MSG_ID CMeasurementSet::SetBmpPath(CString sPath)
{
  m_pView->SetBMPPath(sPath);

  return SETBMPPATH;
}
CAM_CONTROL_MSG_ID CMeasurementSet::EnableLPFilter(int iEnable)
{
  CQuantityFrame * pQuan = CQuantityFrame::Create(iEnable/*0.*/);
  pQuan->ChangeId(0);
  if (!m_pView->m_pBuilder->SendDataFrame(pQuan, m_LowPassOnName))
    pQuan->Release(pQuan);
  m_iActiveInput = 0;
  return ENABLELPFILTER;
}
CAM_CONTROL_MSG_ID CMeasurementSet::SetDebouncing(int iDebouncing_units)
{
  CString Control;
  Control.Format("set Debouncing(%d)", iDebouncing_units);
  CTextFrame * pCommand = CTextFrame::Create(Control);
  if (!m_pView->m_pBuilder->SendDataFrame(pCommand, m_CaptureControlName))
  {
    m_Status.Format("Can't send command to %s",
      (LPCTSTR)(m_SetName));
    pCommand->Release(pCommand);
    return OPERATION_ERROR;
  }
  else
  {
    m_iDebouncing_units = iDebouncing_units;
    CString Msg;
    Msg.Format("%s Debouncing=%d uS", m_SetName, iDebouncing_units);
    char * pMsg = new char[Msg.GetLength() + 1];
    strcpy_s(pMsg, Msg.GetLength() + 1, (LPCTSTR)Msg);
    ::PostMessage(m_pView->m_hWnd, iMsgForLog, 0, (LPARAM)pMsg);
    return SETDEBOUNCING;
  }
}
CAM_CONTROL_MSG_ID CMeasurementSet::GetDebouncing(int& iDebouncing_units)
{
  iDebouncing_units = m_iDebouncing_units;
  m_Status = "OK";
  return GETDEBOUNCING;
}

CAM_CONTROL_MSG_ID CMeasurementSet::GetLastMaxIntensity(int& iMaxIntensity)
{
  if (m_GraphMode > 0)
  {
    DWORD dwTimeOut = 3000;
    if (WaitForSingleObject(m_hEVDataImg, dwTimeOut) != WAIT_OBJECT_0)
      return GRAB_AND_MEASURE_IS_NOT_FINISHED;
  }

  double dBegin = get_current_time();
  //   if ( !WaitEndOfGrabAndProcess( 2000 ) )
  //   {
  //     m_Status.Format( "ERROR: Timeout on grab and measure" ) ;
  //     return GRAB_AND_MEASURE_IS_NOT_FINISHED; 
  //   }

  iMaxIntensity = m_iLastMaxIntensity;
  m_Status.Format("MaxIntensity = %d ", m_iLastMaxIntensity);
  TRACE("\n%s MaxIntens=%d Wait=%10.6f", (LPCTSTR)m_pShMemControl->m_ShMemName, m_iLastMaxIntensity, get_current_time() - dBegin);
  return GETLASTMAXINTENSITY;
}
CAM_CONTROL_MSG_ID CMeasurementSet::GetLastMinIntensity(int& iMinIntensity)
{
  if (m_GraphMode > 0)
  {
    DWORD dwTimeOut = 3000;
    if (WaitForSingleObject(m_hEVDataImg, dwTimeOut) != WAIT_OBJECT_0)
      return GRAB_AND_MEASURE_IS_NOT_FINISHED;
  }

  double dBegin = get_current_time();
  //   if ( !WaitEndOfGrabAndProcess( 2000 ) )
  //   {
  //     m_Status.Format( "ERROR: Timeout on grab and measure" ) ;
  //     return GRAB_AND_MEASURE_IS_NOT_FINISHED; 
  //   }

  iMinIntensity = m_iLastMinIntensity;
  m_Status.Format("MinIntensity = %d ", m_iLastMinIntensity);
  TRACE("\n%s MinIntens=%d Wait=%10.6f", (LPCTSTR)m_pShMemControl->m_ShMemName, m_iLastMinIntensity, get_current_time() - dBegin);
  return GETLASTMAXINTENSITY;
}

CAM_CONTROL_MSG_ID CMeasurementSet::SetWinPos(int iX, int iY)
{
  ::SetWindowPos(m_pView->m_hWnd, HWND_TOP, iX, iY, 0, 0, SWP_NOSIZE);
  ::BringWindowToTop(m_pView->m_hWnd);
  m_Status.Format("Imaging Window pos [%d,%d] ", iX, iY);
  return SETWINPOS;
}

CAM_CONTROL_MSG_ID CMeasurementSet::SetMinArea(long iMinArea)
{
  m_iMinArea = iMinArea;
  m_Status.Format("Min Area is %d", iMinArea);
  return SETMINAREA;
};

CAM_CONTROL_MSG_ID CMeasurementSet::SetSyncMode(long bAsync)
{
  int iTriggerValue = 0;
  //if ( bAsync == 0 )
  //{
  iTriggerValue = (bAsync == 0);
  //iTriggerValue |= (m_pView->m_iTriggerSrc==0)? 1 : 3 ;
  //iTriggerValue += (m_pView->m_iTriggerPolarity != 0) ;
//}

  CTextFrame * pCommand = CTextFrame::Create();
  //pCommand->GetString().Format( "set Trigger(%d)" , iTriggerValue ); 
  pCommand->GetString().Format("set %s(%d)", (LPCTSTR)m_pView->m_TriggerModePropertyName, iTriggerValue);

  if (!m_pView->m_pBuilder->SendDataFrame(pCommand, m_CaptureControlName))
  {
    pCommand->Release(pCommand);
    m_Status.Format("Can't set #%s to %s mode", m_SetName,
      (bAsync == 0) ? "Sync" : "Async");
    return OPERATION_ERROR;
  }
  if (bAsync == 0)
    SetTriggerDelay(m_iTrigDelay_uS);

  m_pView->PostMessage(iMsgFromSMThreads, SETSYNCMODE, bAsync);
  m_bTriggerMode = !bAsync;
  return SETSYNCMODE;
};

CAM_CONTROL_MSG_ID CMeasurementSet::GetDiffractionMeasPar(LPCTSTR pResult)
{
  FXPropertyKit pk;
  m_pProcessGadget->PrintProperties(pk);
  FXString Radiuses;
  if (pk.GetString("diffr", Radiuses))
  {
    int iCommaPos = Radiuses.Find(',');
    if (iCommaPos > 0)
    {
      CString RadX = Radiuses.Mid(0, iCommaPos);
      if (iCommaPos < Radiuses.GetLength() - 1)
      {
        CString RadY = Radiuses.Right(Radiuses.GetLength() - iCommaPos - 1);
        m_Status.Format("%d %d %d %d",
          1, (LPCTSTR)(RadX), (LPCTSTR)(RadY), 0);
        memcpy((void*)pResult, (LPCTSTR)m_Status, m_Status.GetLength());
        return GETDIFFRACTIONMEASPAR;
      }
    }
    m_Status.Format("Bad radius description: %s", (LPCTSTR)Radiuses);
  }
  else
  {
    m_Status.Format("%d %d %d %d", 1, m_pView->m_ImParam.m_iDiffractionRadius, m_pView->m_ImParam.m_iDiffractionRadius_Y, 0);
    memcpy((void*)pResult, (LPCTSTR)m_Status, m_Status.GetLength());
    return GETDIFFRACTIONMEASPAR;
  }

  m_Status.Format("No radius information");
  return OPERATION_ERROR;
}

CAM_CONTROL_MSG_ID
CMeasurementSet::SetDiffractionMeasPar(
  int iMeasMethod, int iRadX, int iRadY, int iBackDist)

{
  FXPropertyKit pk;
  m_pProcessGadget->PrintProperties(pk);
  CString Before, After;
  CString DiffrVal;
  DiffrVal.Format("%d,%d", iRadX, iRadY);
  int start = 0;
  while ((start = pk.Find("diffr", start)) >= 0)
  {
    while (start < pk.GetLength() && pk[start] != 'o')//'('
      start++;
    if (start >= pk.GetLength())
    {
      m_Status.Format("Can't process radius information");
      return OPERATION_ERROR;
    }
    start++;
    Before = pk.Left(start);
    int iIndex = start;
    bool found = false;
    do
    {
      switch (pk[iIndex])
      {
      case '\\'://case ')':
        After = pk.Mid(iIndex);
        pk = Before + DiffrVal + After;
        start = Before.GetLength() + DiffrVal.GetLength();
        found = true;
        break;
        //case '\\':
      case 0:
        m_Status.Format("Can't process radius information");
        return OPERATION_ERROR;

      default: iIndex++; break;
      }
    } while (!found);
  }
  bool Invalidate = true;
  m_pProcessGadget->ScanProperties(pk, Invalidate);
  m_Status.Format("Diffr pars are settled ");
  return SETDIFFRACTIONMEASPAR;
}
/*
CAM_CONTROL_MSG_ID
CMeasurementSet::SetThresholdForBinarization( double dThres )
{
  FXPropertyKit pk ;
  m_pProcessGadget->PrintProperties( pk ) ;
  CString ThresValue ;
  CString Before , After ;
  ThresValue.Format( "%g" , dThres ) ;
  int start = 0 ;
  while ( (start = pk.Find( "thres" , start )) >= 0 )
  {

    start += strlen( "thres" ) ;
    int iIndex = start ;
    int iEuqPos = pk.Find( "\\e" , iIndex ) ;
    if ( iEuqPos < 0  &&  (iEuqPos - iIndex) < 6 )
    {
      m_Status.Format( "Can't find threshold value" ) ;
      return OPERATION_ERROR ;
    }
    iIndex = iEuqPos + 2 ;
    Before = pk.Left(iIndex) ;
    while ( isdigit(pk[iIndex])
      ||  pk[iIndex] == '.'
      ||  pk[iIndex] == '+'
      ||  pk[iIndex] == '-'
      ||  pk[iIndex] == ' ')
    {
      iIndex++ ;
    }
    After = pk.Mid( iIndex ) ;
    pk = Before + ThresValue + After ;
    start = Before.GetLength() + ThresValue.GetLength() ;
  }
  bool Invalidate = true ;
  m_pProcessGadget->ScanProperties( pk , Invalidate ) ;
  m_Status.Format( "Threshold is settled" ) ;
  return SET_THRESHOLD_FOR_BINARIZATION ;
}

CAM_CONTROL_MSG_ID
CMeasurementSet::SetThresholdForRotation( double dThres )
{
  FXPropertyKit pk ;
  m_pProcessGadget->PrintProperties( pk ) ;
  CString DiffrVal ;
  DiffrVal.Format( "%g" , dThres ) ;
  if (pk.WriteString( "rotation_thres" , DiffrVal ) )
  {
    m_Status.Format( "Rotation threshold is set" ) ;
    return SETDIFFRACTIONMEASPAR ;
  }
  m_Status.Format( "Can't write rotation threshold information" ) ;
  return OPERATION_ERROR ;
}
*/

CAM_CONTROL_MSG_ID
CMeasurementSet::SetThresholdForBinarization(double dThres)
{
  FXPropertyKit pk;
  m_pProcessGadget->PrintProperties(pk);
  CString Before, After;
  CString DiffrVal;
  DiffrVal.Format("%g", dThres);
  int start = 0;
  while ((start = pk.Find("bthres", start)) >= 0)
  {
    while (start < pk.GetLength() && pk[start - 2] != 'e' || pk[start - 1] != 92 || pk[start] != 'b')
      start++;
    //start+=1;
    if (start >= pk.GetLength())
    {
      m_Status.Format("Can't process Bin Threshold information");
      return OPERATION_ERROR;
    }
    start++;
    Before = pk.Left(start);
    int iIndex = start;
    bool found = false;
    do
    {
      switch (pk[iIndex])
      {
      case '\\':
        After = pk.Mid(iIndex);
        pk = Before + DiffrVal + After;
        start = Before.GetLength() + DiffrVal.GetLength();
        found = true;
        break;
        //case '\\':
      case 0:
        m_Status.Format("Can't process Bin Threshold information");
        return OPERATION_ERROR;

      default: iIndex++; break;
      }
    } while (!found);
  }
  bool Invalidate = true;
  m_pProcessGadget->ScanProperties(pk, Invalidate);
  m_Status.Format("Threshold is settled");


  //FXPropertyKit pk11 ;
  //m_pProcessGadget->PrintProperties( pk11 ) ;

  return  SET_THRESHOLD_FOR_BINARIZATION;
}

CAM_CONTROL_MSG_ID
CMeasurementSet::SetThresholdForRotation(double dThres)
{
  FXPropertyKit pk;
  m_pProcessGadget->PrintProperties(pk);
  FXPropertyKit savedpk;
  savedpk = pk;

  CString Before, After;
  CString DiffrVal;
  DiffrVal.Format("%g", dThres);
  int start = 0;
  while ((start = pk.Find("srotation_thres", start)) >= 0)
  {
    while (start < pk.GetLength() && pk[start - 2] != 'e')
      start++;
    start += 1;
    if (start >= pk.GetLength())
    {
      m_Status.Format("Can't process Rot Threshold information");
      return OPERATION_ERROR;
    }
    start++;
    Before = pk.Left(start);
    int iIndex = start;
    bool found = false;
    do
    {
      switch (pk[iIndex])
      {
      case '\\':
        After = pk.Mid(iIndex);
        pk = Before + DiffrVal + After;
        start = Before.GetLength() + DiffrVal.GetLength();
        found = true;
        break;
        //case '\\':
      case 0:
        m_Status.Format("Can't process  Rot Threshold information");
        return OPERATION_ERROR;

      default: iIndex++; break;
      }
    } while (!found);
  }
  bool Invalidate = true;
  m_pProcessGadget->ScanProperties(pk, Invalidate);
  m_Status.Format("Rotation Threshold is settled");
  return  SET_THRESHOLD_FOR_ROTATION;
}


void
CMeasurementSet::SetScale(double dScale, TCHAR * pUnitName)
{
  CTextFrame * p = CTextFrame::Create();
  if (pUnitName == NULL)
    pUnitName = _T("pix");
  p->GetString().Format("Scale(%15.6f,%s)", dScale, pUnitName);
  if (!m_pView->m_pBuilder->SendDataFrame(p, m_ProcessControlName))
    p->Release(p);
};

CAM_CONTROL_MSG_ID CMeasurementSet::SetGamma(double dGamma)
{
  CString Control;
  Control.Format("set Gamma(%d)", (int)dGamma);
  CTextFrame * pCommand = CTextFrame::Create(Control);
  if (!m_pView->m_pBuilder->SendDataFrame(pCommand, m_CaptureControlName))
  {
    m_Status.Format("Can't send command to %s",
      (LPCTSTR)(m_SetName));
    pCommand->Release(pCommand);
    return OPERATION_ERROR;
  }
  else
  {
    m_dGamma = dGamma;
    CString Msg;
    Msg.Format("%s Gamma=%d", m_SetName, (int)m_dGamma);
    char * pMsg = new char[Msg.GetLength() + 1];
    strcpy_s(pMsg, Msg.GetLength() + 1, (LPCTSTR)Msg);
    ::PostMessage(m_pView->m_hWnd, iMsgForLog, 0, (LPARAM)pMsg);
    return SETGAMMA;
  }
}
CAM_CONTROL_MSG_ID CMeasurementSet::GetGamma(double& dGamma)
{
  dGamma = m_dGamma;
  m_Status = "OK";
  return GETGAMMA;
}

bool CMeasurementSet::EnableGetImagesFromLAN(bool bEnable)
{
  return m_pView->m_Graph.SendQuantity(m_CamOrLANSwitchPinName, bEnable ? GWM_PASS_THROW : GWM_REJECT);
}

bool CMeasurementSet::EnableSendResultsToLAN(bool bEnable)
{
  return m_pView->m_Graph.SetWorkingMode(m_ResultsExtractorGadgetName, bEnable ? GWM_PROCESS : GWM_REJECT);
}

bool CMeasurementSet::EnableSendImagesToLAN(bool bEnable)
{
  return m_pView->m_Graph.SetWorkingMode(m_ImageToLANExtractorGadgetName, bEnable ? GWM_PROCESS : GWM_REJECT);
}

bool CMeasurementSet::EnableRadialDistributionShow(bool bEnable)
{
  return m_pView->m_Graph.SetWorkingMode(m_RadialDistrCalcGadgetName, bEnable ? GWM_PROCESS : GWM_PASS_THROW);
}

bool CMeasurementSet::GetRadialDistributionShow(bool& bEnabled)
{
  int iWorkingMode = 0;
  bool bRes = m_pView->m_Graph.GetWorkingMode(m_RadialDistrCalcGadgetName, iWorkingMode);
  if (bRes)
    bEnabled = (iWorkingMode == GWM_PROCESS);
  return bRes;
}
CAM_CONTROL_MSG_ID CMeasurementSet::SetTextThroughLAN(CString sText)
{
  CTextFrame * pCommand = CTextFrame::Create();
  pCommand->GetString().Format("%s", sText);
  if (!m_pView->m_pBuilder->SendDataFrame(pCommand,
    m_LANInputPinName))
  {
    m_Status.Format("Can't send command to %s",
      (LPCTSTR)(m_SetName));
    pCommand->Release(pCommand);
    return OPERATION_ERROR;
  }
  return MSG_ID_OK;
}

CAM_CONTROL_MSG_ID CMeasurementSet::SetROIforProfile(int x, int y, int iwidth, int iheight)
{
  CString sCrect;// = _T("Rect=%d,%d,&d,&d;");

  //if( m_GraphMode > 0 )
  //  sCrect = _T("Rect=4,4,984,984;");
  sCrect.Format("Rect=%d,%d,%d,%d;", x, y, iwidth, iheight);

  CTextFrame *pText = CTextFrame::Create(sCrect);
  if (!m_pView->m_pBuilder->SendDataFrame(pText, m_ProfileGadgetControlName))
    pText->Release(pText);


  //   CQuantityFrame * pQuan ;// = CQuantityFrame::Create( iEnable/*0.*/ ) ;
  //   pQuan->ChangeId( 0 ) ;
  //   if ( !m_pView->m_pBuilder->SendDataFrame( pQuan , m_ProfileGadgetControlName ) )
  //     pQuan->Release( pQuan ) ;                                               
  //   m_iActiveInput = 0 ;

  return SETPROFILEROI;
}

CAM_CONTROL_MSG_ID CMeasurementSet::GetProfile(int iProfN)
{
  if (m_GraphMode > 0)
  {
    DWORD dwTimeOut = 3000;
    if (WaitForSingleObject(m_hEVDataImg, dwTimeOut) != WAIT_OBJECT_0)
      return GRAB_AND_MEASURE_IS_NOT_FINISHED;
  }

  if (iProfN != -100) // get background profile
  {
    if (!WaitEndOfGrabAndProcess(2000))
    {
      m_Status.Format("ERROR: Timeout on grab and measure");
      return GRAB_AND_MEASURE_IS_NOT_FINISHED;
    }
  }
  else
  {
    m_dCaptureFinishedAt = get_current_time();
    iProfN = -1;
    Sleep(2000); // ensure that grab is finished
  }

  if (iProfN == -1)//request
  {
    m_Status.Format("%d", m_pProfilePtArray.GetSize());
    return GETPROFILE;
  }

  m_pProfilePtArray.Lock();

  if (m_pProfilePtArray.GetSize() == 0)
  {
    m_pProfilePtArray.Unlock();
    m_Status.Format("ERROR - Profile is Empty ");
    return UNAVAILABLE_INFO_REQUESTED;
  }

  CDPoint myProfileN = m_pProfilePtArray.GetAt(iProfN);
  m_pProfilePtArray.Unlock();

  m_Status.Format("%d %6.2f %6.2f",
    iProfN,
    myProfileN.x,
    myProfileN.y);
  return GETPROFILE;
}


