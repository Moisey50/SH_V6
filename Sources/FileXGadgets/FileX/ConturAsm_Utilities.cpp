// Utilities and service functions for ConturASM gadget


#include "Stdafx.h"

#include "ConturAsm.h"
#include <imageproc/statistics.h>


#define THIS_MODULENAME "ConturAsm"

LPCTSTR GetStageName(ProcessingStage PrSt)
{
  int iIndex = (int)PrSt;
  if (0 <= iIndex && iIndex < GetNStageNames())
    return StageNames[iIndex];
  return _T("Unknown Stage");
}

bool IsShiftKeyDown()
{
  return ((GetAsyncKeyState(VK_SHIFT) & 0x8000) != 0);
}
bool IsCTRLKeyDown()
{
  return ((GetAsyncKeyState(VK_CONTROL) & 0x8000) != 0);
}
LPCTSTR GetStageDescription(ProcessingStage PrSt)
{
  int iIndex = (int)PrSt;
  if (0 <= iIndex && iIndex < GetNStageDescriptions())
    return StageDescriptions[iIndex];
  return _T("Unknown Stage");
}
int GetLightDir(Directions Dir, bool bFront)
{
  ASSERT(Dir != H_UNKNOWN);
  int iRes = (bFront) ? gFront[(int)Dir] : gBack[(int)Dir];
  return iRes;
}

double GetCurrentInternalLightAngle(Directions Dir, bool bFront)
{
  double dAngle = M_PI_2 - ((int)Dir * LED_ANGLE_STEP);
  dAngle = NormTo2PI(dAngle);
  dAngle += (bFront) ? M_PI_2 : -M_PI_2;
  dAngle = NormTo2PI(dAngle);
  //   dAngle = fabs(dAngle) <= M_PI_2 ? 
//     NormRad( dAngle ) : fmod( dAngle + M_2PI , M_2PI ) ;
  return dAngle;
}

cmplx GetAverageAroundPt(cmplx * pcData, int iLen, int iPtIndex, int iRange)
{
  cmplx cResult;
  for (int i = iPtIndex - iRange; i <= iPtIndex + iRange; i++)
  {
    cResult += pcData[(i + iLen) % iLen];
  }
  cResult /= (double)(2 * iRange + 1);
  return cResult;
}





bool ConturAsm::PutFrameAndCheck(COutputConnector * pPin, CDataFrame * pData,
  LPCTSTR ErrorMsg, ProcessingStage OnError, ProcessingStage OnOK )
{
  if (!PutFrame(pPin, pData))
  {
    SEND_GADGET_ERR(ErrorMsg);
    m_ProcessingStage = OnError;
    return false;
  }
  if (OnOK != Stage_Unchanged)
    m_ProcessingStage = OnOK;
  return true;
}

bool ConturAsm::SetCamerasExposure(int iExp_us, LPCTSTR pErrorMsg, ProcessingStage OnError)
{
  CTextFrame * pCmd = CreateTextFrame(_T(""), _T("SetExposure"));
  pCmd->GetString().Format(_T("set Shutter_us(%d)"), iExp_us);
  TRACE("\nExposure=%d", iExp_us);
  return PutFrameAndCheck(m_pLightControl, pCmd, pErrorMsg, OnError);
}

bool ConturAsm::StartFocusing(int iRangeMinus, int iRangePlus,
  LPCTSTR pLightCommand, int iScanSpeed, int iExposure_us, int iWorkingMode )
{
  SetCamerasExposure(iExposure_us, "Can't set exposure for focusing", Stage_Inactive);
  SetTask(-1);
  Sleep(40);
  m_iLastFocusingMotionSteps = m_iNMotionSteps;
  CTextFrame * pBeginFocusing = CreateTextFrame(
    _T(""), _T("FindFocus"));
  pBeginFocusing->GetString().Format(
    "RangeBegin=%d;RangeEnd=%d;LightCommand=%s;"
    "ScanSpeed_um=%d;WorkingMode=%d;Correction_um=%d;",
    iRangeMinus, iRangePlus, pLightCommand, iScanSpeed,
    iWorkingMode, (int)m_dFocusingCorrection_um);
  return (m_bInFocusing = PutFrame(m_pLightControl, pBeginFocusing));
}

ProcessingStage  ConturAsm::SetProcessingStage(
  ProcessingStage NewStage, bool bReport,
  LPCTSTR pAddition, bool bDontAllert)
{
  ProcessingStage OldStage = m_ProcessingStage;
  if (IsInScanProcess() && (m_dStopReceivedTime != 0.))
  {
    m_ProcessingStage = Stage_Inactive;
  }
  else
  {
    if (IsInScanProcess()
      && (NewStage == Stage_Inactive)
      && !bDontAllert)
    {
      ASSERT(0);
    }
    m_ProcessingStage = NewStage;
    //     if ( OldStage == NewStage
    //       && m_ProcessingStage == Stage_WaitForMotionEnd )
    //     {
    //       ASSERT( 0 ) ;
    //     }
    if (bReport || IsInactive())
    {
      FXString Report;

      if (IsMotionStep())
      {
        Report.Format(_T("%s (%d). Mask=0x%X Before Motion is %s "),
          GetStageDescription(m_ProcessingStage),
          m_iNMotionSteps, m_MotionMask,
          GetStageDescription(m_StageBeforeMotion));
      }
      else
      {
        Report.Format(_T("%s (%d). Old is %s "),
          GetStageDescription(m_ProcessingStage),
          m_iNMotionSteps,
          GetStageDescription(OldStage));
      }
      if (pAddition)
      {
        Report += pAddition;
      }
      SendReport(Report);
    }
  }
  return OldStage;
}

bool  ConturAsm::CheckAndAddSegment(BorderSegment& Seg,
  const CDPoint * pPt, Edge CurrEdge)
{
  if (Seg.m_pBegin)
  {
    Seg.m_pEnd = pPt;
    Seg.m_EdgeEnd = CurrEdge;
    m_NewSegments.Add(Seg);
    Seg.Reset();
    return true;
  }
  return false;
}
ProcessingStage  ConturAsm::CheckOperationStack(bool bStopIfEmpty)
{
  if (!IsFocusing())
  {
    if (m_DelayedOperations.empty())
    {
      if (bStopIfEmpty)
        SetProcessingStage(Stage_Inactive, true);
    }
    else
    {
      SetProcessingStage(m_DelayedOperations.top(), true);
      m_DelayedOperations.pop();
    }
  }
  return m_ProcessingStage;
}

double ConturAsm::GetDiscreteDir(double dAngle)
{
  //dAngle += M_PI * 4. ; ;
  double dShifted = M_2PI - (dAngle + LEDS_SHIFT_ANGLE);
  double dNorm = NormTo2PI(dShifted);
  int iIndex = ROUND(dNorm * 4. / M_PI);
  iIndex %= 8;
  double dRealLightAngle = M_2PI - LEDS_SHIFT_ANGLE
    - iIndex * M_PI / 4.0;
  return NormTo2PI(dRealLightAngle);
}

DWORD ConturAsm::GetLightMask(double dAngle,
  double& dRealLightAngle, int iNLeds )
{
  if (!iNLeds)
    iNLeds = m_iNLeds;
  double dNormAngle = NormTo2PI(dAngle);
  dRealLightAngle = GetDiscreteDir(dNormAngle);
  switch (iNLeds)
  {
  case 1:
    {
      if (dNormAngle < M_PI_4) return 0x80;
      if (dNormAngle < M_PI_2) return 0x40;
      if (dNormAngle < 3.* M_PI_4) return 0x20;
      if (dNormAngle < M_PI)   return 0x10;
      if (dNormAngle < 5.*M_PI_4)  return 0x08;
      if (dNormAngle < 3. * M_PI_2)return 0x04;
      if (dNormAngle < 7. *M_PI_4) return 0x02;
      return 0x01;
    }
  case 2:
    {
      if (dNormAngle < M_PI_8) return 0x81;
      if (dNormAngle < M_PI_4 + M_PI_8) return 0xc0;
      if (dNormAngle < M_PI_2 + M_PI_8) return 0x60;
      if (dNormAngle < M_PI - M_PI_8)   return 0x30;
      if (dNormAngle < M_PI + M_PI_8)  return 0x018;
      if (dNormAngle < 3. * M_PI_2 - M_PI_8)return 0x0c;
      if (dNormAngle < 3. * M_PI_2 + M_PI_8)return 0x06;
      if (dNormAngle < M_2PI - M_PI_8) return 0x03;
      return 0x81;
    }
  case 3:
    {
      if (dNormAngle < M_PI_4) return 0xc1;
      if (dNormAngle < M_PI_2) return 0xe0;
      if (dNormAngle < 3 * M_PI_4) return 0x70;
      if (dNormAngle < M_PI)   return 0x38;
      if (dNormAngle < 5.* M_PI_4)  return 0x01c;
      if (dNormAngle < 3. * M_PI_2)return 0x0e;
      if (dNormAngle < 7. * M_PI_4)return 0x07;
      return 0x83;
    }
  default:
    break;
  }
  return 0x80;
}
int ConturAsm::SetLightFromAngle(double dAngle,
  int iLightningTime_us, double& dRealLightAngle, int iNLeds )
{
  if (!iNLeds)
    iNLeds = m_iNLeds;
  DWORD dwMask = GetLightMask(dAngle, dRealLightAngle, iNLeds);
  return SetWorkingLed(dwMask, iLightningTime_us);
}
int ConturAsm::SetLightForExternal(double dAngle,
  int iLightningTime_us, double& dRealLightAngle)
{
  SetTask(0);
  int iRes = SetLightFromAngle(dAngle,
    iLightningTime_us, dRealLightAngle);
  m_sLastCmndForExtLight = m_sLastLightCommand;
  return iRes;
}
int ConturAsm::SetLightForInternal(double dAngle,
  int iLightningTime_us, double& dRealLightAngle, int iNLeds )
{
  SetTask(1);
  if (!iNLeds)
    iNLeds = m_iNLeds;
  int iRes = SetLightFromAngle(dAngle,
    iLightningTime_us, dRealLightAngle, iNLeds);
  m_sLastCmndForIntLight = m_sLastLightCommand;
  return iRes;
}
cmplx ConturAsm::GetMeasAbsPos(cmplx cPosInFOV)
{
  cmplx cVectToFOVCent = cPosInFOV - m_cMeasImageCent;
  cmplx cAbsVectToFOVCent = cVectToFOVCent / m_dScale_pix_per_um;
  cmplx cAbsPos = cAbsVectToFOVCent + m_cRobotPos;
  return cAbsPos;
}
cmplx ConturAsm::GetPartCenterPositionOnMeas()
{
  // intitial part center minus robot motion from initial pos
  cmplx cPartCentPos = m_cPartCenterOnMeas +
    (m_cRobotPos - m_cInitialRobotPosForMeas);
  return cPartCentPos;
}
cmplx ConturAsm::GetVectFromFOVCentToPartCent()
{
  cmplx cVect = m_cInitialVectToPartCent + conj(m_cRobotPosToInitialPt);
  return cVect;
}
cmplx ConturAsm::GetVectToPartCent(const cmplx& cPos)
{
  //    cmplx cVect = cPos - m_cPartCentPos ;
  cmplx cVect = GetVectFromFOVCentToPartCent() - cPos;
  return cVect;
}
cmplx ConturAsm::GetVectToPartCent()
{
  //    cmplx cVect = cPos - m_cPartCentPos ;
  cmplx cVect = GetVectFromFOVCentToPartCent();
  return cVect;
}
cmplx ConturAsm::GetVectToPartCent(cmplx& cPtInFOV)
{
  cmplx PartCent = GetPartCenterPositionOnMeas();
  cmplx PtPos = GetMeasAbsPos(cPtInFOV);
  cmplx cVect = PartCent - PtPos;
  return cVect;
}

double ConturAsm::GetAngleToCenter()
{
  cmplx cVect = GetVectToPartCent();
  //cVect = conj( cVect ) ;
  double dAngle = arg(cVect);
  return dAngle;
}

double ConturAsm::GetAngleFromCenter()
{
  double dAngle = GetAngleToCenter();
  dAngle += M_PI;
  dAngle = NormTo2PI(dAngle);
  return dAngle;
}

void ConturAsm::SetTask(int iTask)
{
  CTextFrame * pTask = CreateTextFrame("", "TaskControl");
  pTask->GetString().Format("Task(%d);", iTask);
  TRACE("\n%s:  %s", GetStageName(),
    (LPCTSTR)(pTask->GetString()));
  PutFrame(m_pLightControl, pTask);
  m_iLastTask = iTask;
}

void ConturAsm::SetTVObjPars( LPCTSTR pObjNameAndPars )
{
  CTextFrame * pTask = CreateTextFrame( pObjNameAndPars , "SetObjectProp" );
  TRACE( "\n%s:  %s" , GetStageName() ,
    ( LPCTSTR ) ( pTask->GetString() ) );
  PutFrame( m_pLightControl , pTask );
}

void ConturAsm::OrderBMPForSimulation( bool bMeasurement )
{     
  CTextFrame * pOrder = CreateTextFrame( "set Grab=1;" , 
    bMeasurement ? "MeasSimuCtrl" : "ObservSimuCtrl" ) ;
  PutFrame( m_pLightControl , pOrder );
}

void ConturAsm::OrderNextDirForSimulation( LPCTSTR pDirName , bool bMeasurement )
{     
  FXString Command , FileName ;
  Command.Format( "set Directory=%s%s;" , pDirName , bMeasurement ? "\\Images" : "" ) ;
  CTextFrame * pCommand = CreateTextFrame( Command ,
    bMeasurement ? "MeasSimuCtrl" : "ObservSimuCtrl" ) ;
  PutFrame( m_pLightControl , pCommand );

  Command.Format( "set FileName=%s.bmp;" , bMeasurement ? "Pt*" : "*ObservationImage" ) ;
  pCommand = CreateTextFrame( Command ,
    bMeasurement ? "MeasSimuCtrl" : "ObservSimuCtrl" ) ;
  PutFrame( m_pLightControl , pCommand ) ;
  if ( !bMeasurement )
    SEND_GADGET_INFO( "Simulation index %u(%u) dir is %s" , 
      m_uiSimuFileIndex , m_SimulationDirectories.GetCount() , pDirName ) ;
}


cmplx ConturAsm::DecodeCoordsForSimulation( FXString Label )
{
  int iPosSlashBack = (int) Label.ReverseFind( '\\' ) ;
  int iPosSlashForv = ( int ) Label.ReverseFind( '/' ) ;
  if ( iPosSlashForv >= 0 )
  {
    if ( iPosSlashBack < iPosSlashForv )
      iPosSlashBack = iPosSlashForv + 1 ;
  }
  else if ( iPosSlashBack >= 0 )
    iPosSlashBack++ ;

    // extract file name from full path if necessary
  if ( iPosSlashBack >= 0 )
    Label = Label.Mid( iPosSlashBack ) ;
  int iPtPos = ( int ) Label.Find( "Pt" ) ;
  if ( iPtPos >= 0 )
    m_iCurrPtForSimulation = atoi( ( LPCTSTR ) Label + iPtPos + 2 ) ;

  int iPosX = ( int ) Label.Find( "_X" ) ;
  if ( iPosX >= 0 )
  {
    int iPosY = ( int ) Label.Find( "_Y" , iPosX + 2 ) ;
    if ( iPosY >= 0 )
    {
      cmplx cRes( atof( ( LPCTSTR ) Label + iPosX + 2 ) , atof( ( LPCTSTR ) Label + iPosY + 2 ) ) ;
      return cRes ;
    }
  }
  return cmplx() ;
}

int ConturAsm::SetLightFromCenter(int iNLeds )
{
  SetTask(1);
  double dAngle = GetAngleFromCenter();
  if (!iNLeds)
    iNLeds = m_iNLeds;
  int iRes = SetLightFromAngle(dAngle,
    m_iIntLight_us, m_dLastLightAngle, iNLeds);
  m_sLastCmndForIntLight = m_sLastLightCommand;
  return iRes;
}
int ConturAsm::SetLightToCenter(int iNLeds )
{
  SetTask(0);
  double dAngle = GetAngleToCenter();
  if (!iNLeds)
    iNLeds = m_iNLeds;
  int iRes = SetLightFromAngle(dAngle,
    m_iExtLight_us, m_dLastLightAngle, iNLeds);
  m_sLastCmndForExtLight = m_sLastLightCommand;
  return iRes;
}
int ConturAsm::SetLightForDarkEdge(int iTaskNumber )
{
  SetTask(iTaskNumber);
  return SetWorkingLeds(CAM_MEASUREMENT,
    MEASUREMENT_LEDS, m_iMeasurementExposure_us);
  ;
}

// Calculate laplacian, check previous images, 
//do Z step, if necessary and return true, 
// or stop process, if image is in focus
bool ConturAsm::ContinueFocusing(const CVideoFrame * pImage)
{
  double dLaplace = _find_max_diff(pImage, m_FocusingRect);// _calc_laplace(pImage, m_FocusingRect);

  CContainerFrame * pOut = CContainerFrame::Create();
  m_dCurrentZCoord_um = m_CurrentPos.m_z;
  pOut->AddFrame(pImage);
  cmplx TextPt = m_cLastROICenter * 1.5;
  CTextFrame * pLapl = CreateTextFrame(TextPt, "0x0000ff", 16, NULL, pImage->GetId(),
    "H=%.2f Ns=%d\nLapl=%.3f", m_dCurrentZCoord_um , m_FocusData.size() , dLaplace);
  pOut->AddFrame(pLapl);
  CRectFrame * pRect = CreateRectFrame(m_FocusingRect, "0x0000ff");
  pOut->AddFrame(pRect);
  PutFrame(GetOutputConnector(4), pOut);
  FocusSample NewPt(m_dCurrentZCoord_um, dLaplace);
  m_FocusData.push_back(NewPt);
  int iSz = (int) m_FocusData.size() - 1; // index with last value
  FocusSample * pData = m_FocusData.data();
  switch ( iSz )
  {
  case 0: 
    {
      // keep in the mind, that bigger value means lower position (Z directed to down)
      double dTargetHeight_um = m_CurrentPos.m_z + m_dCurrentFocusStep_um * 3;
      double dDefocusingMaxLimit_um = (m_dBaseHeight - m_dAdapterHeight - m_dPartHeight / 2.);
      if (dTargetHeight_um > dDefocusingMaxLimit_um ) // for parts with small height we should limit
        // defocusing range (otherwise system could catch some spots on basement glass
      {
        OrderAbsZMotion(dDefocusingMaxLimit_um); // lower position will be on defocusing limit
                                                 // upper position will be on defocusing limit plus 6 steps
      }
      else
        OrderRelZMotion(m_dCurrentFocusStep_um * 3); // move down for 3 steps (if step is positive)
    }
    return true;
  case 1:
  case 2:
  case 3:
  case 4:
  case 5:
  case 6:
    {
      OrderRelZMotion( -m_dCurrentFocusStep_um ); // go up and measure
    }
    return true;
  case 7:  // OK, we did raw scan
    {
      FocusSample Max = m_FocusData[0];
      for ( auto it = m_FocusData.begin() + 1 ; it < m_FocusData.end() ; it++ )
      {
        if (Max.m_dValue < it->m_dValue)
          Max = *it;
      }
      m_FocusData.push_back(Max);
      m_dCurrentFocusStep_um /= 3;
      double dNewTarget = Max.m_dCoord + m_dCurrentFocusStep_um;
      OrderAbsZMotion(dNewTarget);
    }
    return true;
  case 9:
    {
      double dNewTarget = m_CurrentPos.m_z - 2. * m_dCurrentFocusStep_um;
      OrderAbsZMotion(dNewTarget);
    }
    return true;
  default:  // raw estimation finished, 3 points are taken (PrevPrev, Prev and current)
    {
      //double dStep_um = pData[iSz].m_dCoord - pData[iSz - 1].m_dCoord ;

      if ( m_iNSmallSteps < 0 ) // raw focus searching
      {
        // 1. Order 3 last samples by Z coordinate
        FocusSample * pMinZ = NULL , *pMiddleZ = NULL , *pMaxZ = NULL ;
        bool bLastZLessPrev = pData[ iSz ].IsOtherZHigher( pData[ iSz - 1 ] ) ;
        bool bLastZLessPrevPrev = pData[ iSz ].IsOtherZHigher( pData[ iSz - 2 ] ) ;
        bool bPrevZLessPrevPrev = pData[ iSz - 1 ].IsOtherZHigher( pData[ iSz - 2 ] ) ;
        if ( bLastZLessPrev && bLastZLessPrevPrev )
      {
          pMinZ = pData + iSz ;
          if ( bPrevZLessPrevPrev )
        {
            pMaxZ = pData + iSz - 2 ;
            pMiddleZ = pData + iSz - 1 ;
          }
          else
          {
            pMaxZ = pData + iSz - 1 ;
            pMiddleZ = pData + iSz - 2 ;
          }
        }
        else if ( bPrevZLessPrevPrev )
        {
          pMinZ = pData + iSz - 1 ;
          if ( bLastZLessPrevPrev )
          {
            pMaxZ = pData + iSz - 2 ;
            pMiddleZ = pData + iSz ;
          }
          else
          {
            pMaxZ = pData + iSz ;
            pMiddleZ = pData + iSz - 2 ;
          }
        }
        else
        {
          pMinZ = pData + iSz - 2 ;
          if ( bLastZLessPrev )
          {
            pMaxZ = pData + iSz - 1 ;
            pMiddleZ = pData + iSz ;
      }
      else
      {
            pMaxZ = pData + iSz ;
            pMiddleZ = pData + iSz - 1 ;
          }
        }

        // 2. Compare laplacian values
        bool bMiddleZValIsMoreThanMinZVal = !pMiddleZ->IsOtherValHigher( *pMinZ ) ;
        bool bMiddleZValIsMoreThanMaxZVal = !pMiddleZ->IsOtherValHigher( *pMaxZ ) ;
        bool bMinZValIsMoreThanMaxZVal = !pMinZ->IsOtherValHigher( *pMaxZ ) ;
        if ( bMiddleZValIsMoreThanMinZVal && bMiddleZValIsMoreThanMaxZVal ) // max is in center
        { // go to center between MinZ and MiddleZ and scan with step/10
          // to the center between MiddleZ and MaxZ until max laplacian value
          double dScanStartZ = (pMinZ->m_dCoord + pMiddleZ->m_dCoord) / 2. ;
          if (m_dCurrentFocusStep_um < 200.)
            m_iNSmallSteps = 0;
          else
            m_dCurrentFocusStep_um /= 3.;
          OrderAbsZMotion( dScanStartZ ) ;
        }
        else if ( bMinZValIsMoreThanMaxZVal ) // continue raw search to Z-
        {
          OrderRelZMotion( -m_dCurrentFocusStep_um ) ;
      }
        else // continue raw search to Z+
        {
          OrderRelZMotion( m_dCurrentFocusStep_um ) ;
    }
  }
      else // fine focus searching
      {
        if ( (m_iNSmallSteps++ != 0) && (pData[iSz].m_dValue < pData[iSz-1].m_dValue) )
        { // if not first small step and laplacian decreased
          OrderRelZMotion( -m_dCurrentFocusStep_um / 4. ) ; // come back to optimal value
          m_bFocusingFinished = true ; // process is finished
          return false;
        }
        OrderRelZMotion( m_dCurrentFocusStep_um / 4. ) ; // continue scanning with small step
      }
    }
  }
  return true ; // continue focus search
}

int ConturAsm::InitIterativeFocusing( ProcessingStage StageAfterFocusing , bool bGrabAndSetStage )
{
  m_DataForPosition.RemoveAll();
  m_CurrentViewMeasStatus.Reset(true);
  m_FocusData.clear();
  m_dFoundFocus_um = 0.;// focus unknown
  FXRegistry Reg("TheFileX\\ConturASM");
  m_dCurrentFocusStep_um = Reg.GetRegiDouble(
    "Parameters", "InitFocusStep_um", 300.);
  m_iNSmallSteps = -1;

  if (Reg.GetRegiIntSerie("Parameters", "FocusingRect_pix", (int*)&m_FocusingRect, 4) != 4)
  {
    FXString AsString;
    AsString.Format("%d,%d,%d,%d", m_FocusingRect.left, m_FocusingRect.top,
      m_FocusingRect.right, m_FocusingRect.bottom);
    Reg.WriteRegiString("Parameters", "FocusingRect_pix", AsString);
  }
  m_FocusingRect.OffsetRect(CPoint(ROUND(m_cLastROICenter.real()), ROUND(m_cLastROICenter.imag())));
  m_dCurrentZCoord_um = m_CurrentPos.m_z;
  m_iDefocusToPlus = 1;
  m_bFocusDirectionChanged = false;
  m_dLastdZ = 0.;
  if ( bGrabAndSetStage )
  {
    SetLightForDarkEdge(-1); // nothing to measure, we need image only
    SetProcessingStage(Stage_Focusing, true);
  }
  m_StageAfterFocusing = StageAfterFocusing;
  return 0;
}

double ConturAsm::GetBestZForEndAndSlope(double& dAvOnEnd, double& dOtherZ, double& dAvForOtherZ)
{
  double dBestZ = 0.;
  double dBestAvOnEnd = -DBL_MAX ;
  int iNAccountedValues = 0;
  int iNSamplesOnEnd = 6;
  FXDblArray Averages;
  for ( int i = 0 ; i < m_DataForPosition.GetCount() ; i++ )
  {
    MeasuredValues& Current = m_DataForPosition.GetAt(i);
    double dCurrentAv = 0;
    int iNCurrentAccumulated = 0;
    for ( int j = Current.m_iPlusIndex ; j >= Current.m_iPlusIndex - iNSamplesOnEnd ; j-- )
    {
      double dCurrVal = Current.m_Results[j].m_dAverFocus;
      if ( (dCurrVal != 300.) && (dCurrVal != 0.) )
      {
        dCurrentAv += dCurrVal;
        iNCurrentAccumulated++;
      }
    }
    if ( iNCurrentAccumulated )
    {
      dCurrentAv /= iNCurrentAccumulated;
      if ( dCurrentAv > dBestAvOnEnd )
      {
        dBestAvOnEnd = dCurrentAv;
        dBestZ = Current.m_RobotPos.m_z;
      }
    }
    Averages.Add(dCurrentAv);
  }

  if ( dBestZ != 0. )
  {
    dAvOnEnd = dBestAvOnEnd;

  }
  return 0.0;
}
