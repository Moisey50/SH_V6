// ConturAsm.h : Implementation of the ConturAsm class


#include "StdAfx.h"
#include "ConturAsm.h"
#include <imageproc\clusters\segmentation.h>
#include <imageproc\utilities.h>
#include <imageproc/simpleip.h>
#include <gadgets\vftempl.h>
#include <helpers\propertykitEx.h>
#include <helpers\FramesHelper.h>
#include "ConturData.h"
#include <files\imgfiles.h>
#define _CRT_RAND_S
#include <stdlib.h>

#define THIS_MODULENAME "ConturAsm"
int iDataFrameLen = sizeof(CDataFrame);
int iVideoFrameLen = sizeof(CVideoFrame);
int iFigureFrameLen = sizeof(CFigureFrame);
int iTextFrameLen = sizeof(CTextFrame);
int iContainerFrameLen = sizeof(CContainerFrame);
int iQuantityFrameLen = sizeof(CQuantityFrame);


#define Fig_Create 1
#define Fig_Release 2
#define Fig_AddRef 3
#define Fig_Touch 4

LPCTSTR StageNames[] =
{
  _T("Stage_Inactive"),
  _T("Stage_LoadPart"),
  _T("Stage_PrepareMeasurement"),
  _T("Stage_Focusing"),
  _T("Stage_EdgeToCenter"),
  _T("Stage_GetInternalBorder"),
  _T("Stage_GetExternalBorder"),
  _T("Stage_WaitForMotionEnd"),
  _T("Stage_WaitForMotionEndX"),
  _T("Stage_WaitForMotionEndY"),
  _T("Stage_ScanFinished"),
  _T("Stage_LightToCenter"),
  _T("Stage_LightFromCenter"),
  _T("Stage_FocusingPlus"),
  _T("Stage_FocusingMinus"),
  _T("Stage_GetCurrentPositions"),

  _T("Stage_ManualMotion "),
  _T("Stage_0_GetCurrPos "),
  _T("Stage_1_MoveToObserv "),
  _T("Stage_2_MeasOnObserv "),
  _T("Stage_3_AutoFocusOnObserv "),
  _T("Stage_4_MeasFocusedOnObserv "),
  _T("Stage_5_SendToEdgeMeasurement "),
  _T("Stage_6_MoveEdgeToCenter "),
  _T("Stage_7_AutoFocusEdge "),
  _T("Stage_8_GetInternalBorder "),
  _T("Stage_9_MoveEdgeToCenter "),
  _T("Stage_10_GetIntOrExtBorder"),
  _T("Stage_11_GetExternalBorder"),
  _T("Stage_12_MeasWalls"),
  _T("Stage_12a_MeasDarkEdge"),
  _T("Stage_13_ScanInlineFocusing"),
  _T("Stage_14_OrderIntContour"),

  _T("Stage_OneDarkMeasBeginToMax"),
  _T("Stage_OneDarkMeasEndToMax"),
  _T("Stage_BeginToMax"),
  _T("Stage_EndToMax"),

  _T("Stage_DistCalib1"),
  _T("Stage_DistCalib2"),
  _T("Stage_DistCalib3"),
  _T("Stage_DistCalib4"),
  _T("Stage_Adjust_Z  "),
  _T("Stage_Observation_Focus"),
  _T("Stage_GetExtImage"),
  _T("Stage_GetAllLightsImage"),
  _T("Stage_OneIntMeasurement"),
  _T("Stage_OneExtMeasurement"),
  _T("Stage_OneExtMeasurement_a"),
  _T("Stage_GoToObservation")
  _T("Stage_OneObservMeasurement"),
  _T("Stage_HomeX"),
  _T("Stage_HomeY"),
  _T("Stage_HomeZ")
  _T("Stage_CalibLight0"),
  _T("Stage_CalibLight1"),
  _T("Stage_CalibLight2"),
  _T("Stage_CalibLight3"),
  _T("Stage_CalibLight4"),
  _T("Stage_CalibLight5"),
  _T("Stage_CalibLight6"),
  _T("Stage_CalibLight7"),
  _T("Stage_CalibLightLast"),
  _T("Dummy")
};

LPCTSTR StageDescriptions[] =
{
  _T("Inactive"),
  _T("Load Part"),
  _T("Prepare Measurement"),
  _T("Focusing as process"),
  _T("Move Edge To Center"),
  _T("Measure Internal Edge"),
  _T("Measure External Edge"),
  _T("Wait For Motion End"),
  _T("Wait For Motion End X"),
  _T("Wait For Motion End Y"),
  _T("Scan Finished"),
  _T("Light To Center"),
  _T("Light From Center"),
  _T("Focusing Plus"),
  _T("Focusing Minus"),
  _T("Get Current Positions"),

  _T("Manual Motion"),
  _T("Start Measurement"),                         // 0 
  _T("Go to Observation Pos"),                     // 1
  _T("Get Image and measure on observation"),      // 2
  //_T( "Init Focusing" ) ,                            
  _T("Observation Autofocusing process"),          // 3 
  _T("Measure Observation Conturs"),               // 4 
  _T("Go to Inspection pos"),                      // 5 
  _T("Set Edge to Center"),                        // 6 
  _T("Do Focusing on Inspection pos"),             // 7 
  _T("Get Internal Border"),                       // 8 
  _T("Move edge to center"),                       // 9 
  _T("Order Internal Border Measurement"),         // 10 
  _T("Order External Border Measurement"),         // 11  
  _T("Process wall"),                              // 12 
  _T("Process Low constrast edge (Dark Edge)"),    // 12a 
  _T("Do focusing on measurement"),                // 13 
  _T("Order Internal Image after Focusing"),       // 14 

  _T("Begin to max for One dark measurement"),
  _T("End to max for One dark measurement"),
  _T("Set segment beginning to focus and measure"),
  _T("Go to segment end with focusing and measuse"),

  _T("Stage_DistCalib1"),
  _T("Stage_DistCalib2"),
  _T("Stage_DistCalib3"),
  _T("Stage_DistCalib4"),
  _T("Stage_Adjust_Z"),
  _T("Observation Focus"),
  _T("Get Ext Image with Light to Center"),
  _T("Get Image with all lights"),
  _T("One Measurement (internal edge)"),
  _T("One Measurement (external edge)"),
  _T("One Measurement By Low Contrast"),
  _T("Go To Observation"),
  _T("One Observation Measurement"),
  _T("Home X"),
  _T("Home Y"),
  _T("Home Z"),
  _T("Prepare For Light Calibration"),
  _T("Check right observation LED"),
  _T("Check left observation LED"),
  _T("Measure Platform center"),
  _T("Go to Measurement LEDs check"),
  _T("Measure Platform Center on Scan"),
  _T("Motion to white place for LED calibration"),
  _T("Measurement LEDs calibration"),
  _T("Last Light calibration")

};

int GetNStageNames()
{
  return ARRAYSIZE(StageNames);
};
int GetNStageDescriptions()
{
  return ARRAYSIZE(StageDescriptions);
};


int             ConturAsm::g_iIndexInLCTRL = 0;
SentFramesIds   ConturAsm::g_LCTRLSentContainers[1000];
double          ConturAsm::g_dLastSendFrameTime = GetHRTickCount();


Directions gOpposite[8] = { H06_00, H07_30, H09_00, H10_30, H12_00, H01_30, H03_00, H04_30 };
// int gFront[ 8 ] = { 0x40 , 0x80 , 0x01 , 0x02 , 0x04 , 0x08 , 0x10 , 0x20 };
// int gBack[ 8 ] = { 0x04 , 0x08 , 0x10 , 0x20 , 0x40 , 0x80 , 0x01 , 0x02 };
int gFront[8] = { 0x01, 0x02, 0x04, 0x08, 0x10, 0x20, 0x40, 0x80 };
int gBack[8] = { 0x10, 0x20, 0x40, 0x80, 0x01, 0x02, 0x04, 0x08 };
cmplx AngleCenters[8] = {
  cmplx(1., 0.), cmplx(1., 1.), cmplx(0., 1.), cmplx(-1., 1.),
  cmplx(-1., 0.), cmplx(-1., -1.), cmplx(0., -1.), cmplx(1., -1.)
};

LPCTSTR DirNames[] =
{
  "H12_00", "H01_30", "H03_00", "H04_30", "H06_00", "H07_30", "H09_00", "H10_30"
};

LPCTSTR GetDirName(Directions Dir)
{
  return (Dir > H_UNKNOWN && Dir <= H10_30) ? DirNames[Dir] : "Unknown";
};
Directions DecodeDirName(FXString& AsString)
{

  if (_totupper(AsString[0]) == _T('H'))
    AsString.Delete(0);
  int iVal = atoi((LPCTSTR)AsString);
  switch (iVal)
  {
  case 12: return H12_00;
  case 1:  return H01_30;
  case 3:  return H03_00;
  case 4:  return H04_30;
  case 6:  return H06_00;
  case 7:  return H07_30;
  case 9:  return H09_00;
  case 10:  return H10_30;
  }
  return H12_00;
}

Axis DecodeRobotAnswer(const FXString& Answer, Axis& From, double& dCoord)
{
  int iIdPos = (int)Answer.Find("DeviceID=");
  if (iIdPos < 0)
    return Axis_Undef;
  int iAxis = atoi(((LPCTSTR)Answer) + iIdPos + 9);
  switch (iAxis)
  {
  case 3: From = Axis_Z; break;
  case 2: From = Axis_X; break;
  case 1: From = Axis_Y; break;
  default: return Axis_Undef;
  }
  int iCoordPos = (int)Answer.Find("Position(um)=");
  if (iCoordPos < 0)
    return FromZaber;
  double d = atof(((LPCTSTR)Answer) + iCoordPos + 13);
  if (d >= 0.)
  {
    dCoord = d;
    return From;
  }
  return NotCorrectCoord;
}


double GetAvValues(const OneMeasPt * pData, int iAvFactor,
  double& dAvFocus, double& dMaxFocus)
{
  const OneMeasPt* pEnd = pData + iAvFactor;
  dAvFocus = 0.;
  dMaxFocus = 0.;
  do
  {
    dAvFocus += pData->m_dAverFocus;
    dMaxFocus += pData->m_dMaxFocus;
  } while (++pData < pEnd);
  dAvFocus /= iAvFactor;
  dMaxFocus /= iAvFactor;

  return dAvFocus;
}

IMPLEMENT_RUNTIME_GADGET_EX(ConturAsm, CFilterGadget, "FileX_Specific", TVDB400_PLUGIN_NAME);

ConturAsm::ConturAsm(void) :
  m_PartEdgeViewCenter(250., 250.)
  , m_dEdgeViewScale(50.)
  , m_dWidthViewScale(10.)
  , m_iViewLength(20)
  , m_cPartCentPos(38687., 50126.)
  , m_iStopOnFault(0)
  , m_iOut_pix(5)
  , m_iIn_pix(15)
  , m_iMinShadowAmpl(50)
  , m_dUpperThreshold(0.3)
  , m_dThicknessThreshold(0.5)
  , m_dShadowThres(0.5)
  , m_VectorsViewCent(800., 500.)
  , m_iLastTask(-1)
  , m_InitialDir(H12_00)
  , m_iMeasurementExposure_us(800)
  , m_iObservationMeasExp_us(80)
  , m_iObservationFocusExp_us(360)
  , m_iFocusFall_perc(20)
  , m_dK_DefocusTodZ(10.)
  , m_iDefocusToPlus(-1)
  , m_bFocusDirectionChanged(false)
  , m_iFocusStep_um(50)
  , m_iLastFocusingMotionSteps(0)
  , m_dScale_pix_per_um(0.2535)
  , m_dObservScale_pix_per_um(0.05)
  , m_dBaseHeight(88000.)
  , m_dPartHeight(5000.)
  , m_dAdapterHeight(2000.)
  , m_cMeasCenter(40250., 43867.)
  , m_cMeasRange(35000., 35000.)
  , m_cObservCenter(105000., 43867.)
  , m_cPartPlacementPos(200000., 43867.)
  , m_cMeasFOVCent(960., 600.)
  , m_cObservFOVCenter(600., 600.)
  , m_cFOVCenterForLightCalib(960., 600.)
  , m_FreeZone(0., 115000., 0., 101600.)
  , m_PlacementZone(115000., 203200., 52750., 53150.)
  , m_iXYMotionScanSpeed_um_sec( 26000 )
  , m_iZMotionSpeed_um_sec(20000)
  , m_MotionSequence(MS_NoMotion)
  , m_bInFocusing(false)
  , m_dLastObservCoordTime(0.)
  , m_PartFile(_T("PartUnknown_0"))
  , m_ResultDirectory(_T(".\\"))
  , m_ScaleBarPos(700, 80, 750, 580)
  , m_iLastShownIndex(-1)
  , m_dStdThreshold(0.5)
  , m_dDarkEdgeThreshold(0.3)
  , m_iFocusAreaSize_pix(20)
  , m_iFocusDistFromEdge_pix(20)
  , m_iObservFocusRange_um(2000)
  , m_iMeasInitialFocusRange_um(500)
  , m_MeasFocusRange_um(300)
  , m_iFocusWorkingMode(0)
  , m_dFocusingCorrection_um(0.)
  , m_sLastCmndForFocusingLightWithExt(_T("out 0x08ff 1400\r"))
  , m_iIndexForMapView(-1)
  , m_HomingState(HS_HomingUnknown)
  , m_ResultViewMode(RVM_MaxBurrWidth)
  , m_LightDir(LD_Ortho)
  , m_iNLeds(1)
  , m_bReportAfterMotion(false)
  , m_dFocusDecreaseNorm(0.5)
  , m_dFocusRangeK(300.)
  , m_dMinimalContrast(90.)
  , m_iFocusingLight_us(800)
  , m_cInitialVectToPartCent(0.0)
  , m_cRobotPosToInitialPt(0., 0.)
  , m_dMinimalLocalFocus(150.)
  , m_iNMotionSteps(0)
  , m_iAccel_units(10)
  , m_dSegmentTurnThres(SEGMENT_TURN_INITIAL_THRES)
  , m_dStopReceivedTime(0.0)
  , m_hWatchDogTimer(NULL)
  , m_DataFrom(ADF_NoData)
  , m_dMinGradWidth_um(3.5)
  , m_dMaxGradWidth_um(70.)
  , m_iNMinGoodSamples(3)
  , m_pFilteredObservationContur(NULL)
  , m_SectorsMarkingMode(SMM_NoMarking)
  , m_VectorsToVertices(NULL)
  , m_SaveFragmentSize(300., 300.)
  , m_LastMapPt(0, 0)
  , m_bLastShiftPressedForMap(false)
  , m_MapMouseClickMode(MCM_ViewSaved)
  , m_iAveragingRange_um(500)
  , m_dLastPrintTime(GetHRTickCount())
  , m_FocusingRect(960 - 150, 600 - 150, 960 + 150, 600 + 150)
  , m_StageAfterFocusing(Stage_Inactive)
  , m_cLastROICenter(960., 600.)
  , m_bFocusingAfterPosFinishing(false)
  , m_MotionLogger( _T("D:/BurrInspector") , _T("Motion.Log") )
{
  m_iDir = H12_00;
  m_iMeasStep = 50;
  m_iMeasZoneWidth = 100;
  m_iMinimalContinuty = 10;
  m_iSegmentLength = 12;
  m_LastROI = CRect(0, 0, 0, 0);
  m_pInput = new CInputConnector(transparent);
  m_pInput->SetQueueSize(20);
  m_pOutput = new COutputConnector(transparent);
  m_pLightControl = new COutputConnector(transparent);
  m_pXMotion = new COutputConnector(text);
  m_pYMotion = new COutputConnector(text);
  m_pMarking = new COutputConnector(transparent);
  m_pImageSave = new COutputConnector(transparent);
  m_pMainView = new COutputConnector(transparent);
  m_pDuplex = new CDuplexConnector(this, text, text);
  m_pOutput->SetName("Maps");
  m_pYMotion->SetName("YControl");
  m_pXMotion->SetName("XControl");
  m_pLightControl->SetName("LightControl");
  m_pMarking->SetName("Marking");
  m_pImageSave->SetName("ImageSave");
  m_pMainView->SetName("MainResultView");

  m_iNotMeasuredCntr = 0;
  m_iNotMeasuredLimit = 8;
  m_WorkingMode = WM_DarkEdge_NoInternal;
  m_iCameraMask = 0x0800;
  m_iLightLen = 1500;
  m_iExtLight_us = m_iIntLight_us = 1200;
  m_iLightForDark_us = 400;

  m_ProcessingStage = Stage_Inactive;
  m_pLastExternalContur = m_pLastInternalContur = NULL;
  m_pPartExternalEdge = m_pPartInternalEdge = NULL;
  m_pLastActiveExternal = m_pLastActiveInternal = NULL;
  m_pPartConturOnObservation = NULL;
  m_pHoleConturOnObservation = m_pHoleConturOnMeasPlace = NULL;
  m_pDebugging = NULL;
  m_pObservationImage = NULL;
  m_OutputMode = modeReplace;
  m_dMaximumsForViewCsale.Add(0.); // not relevant for 3d
  m_dMaximumsForViewCsale.Add(30.); // For average width
  m_dMaximumsForViewCsale.Add(100.); // For max width
  m_dMaximumsForViewCsale.Add(20.); // For average height
  m_dMaximumsForViewCsale.Add(40.); // For max height
  m_dMaximumsForViewCsale.Add(50000.); // For volume
  m_dMaximumsForViewCsale.Add(300.); // For burr length
  m_sLastCmndForExtLight = m_sLastCmndForFocusingLightWithExt;
  m_iIndexOfFoundObservForm = -1;
  // Take form data from file
  int iNKnownForms = m_ObservGeomAnalyzer.Init();
  FXRegistry Reg("TheFileX\\ConturASM");
  Reg.GetRegiCmplx("Parameters", "SaveFragmentSize", m_SaveFragmentSize, cmplx(300., 300.));
  m_PartFile = Reg.GetRegiString("LastScan", "LastPart", "");
  m_PO = Reg.GetRegiString("LastScan", "LastOrder", m_PO);
  m_iObservationMeasExp_us = Reg.GetRegiInt("LastScan", "ObservExp_us", m_iObservationMeasExp_us);
  m_iMeasurementExposure_us = Reg.GetRegiInt("LastScan", "MeasExp_us", m_iMeasurementExposure_us);
  m_iSegmentLength = Reg.GetRegiInt("LastScan", "SegmentLength_pix", m_iSegmentLength);
  m_iAveragingRange_um = Reg.GetRegiInt("Parameters", "AveragingRange_um", 1000);
  // resume gadget main thread
  Resume();
}

void ConturAsm::ShutDown()
{
  //TODO: Add all destruction code here
  CFilterGadget::ShutDown();
  m_ObservationPartConturs.RemoveAll();
  m_LastExtConturs.RemoveAll();
  m_LastScanFrames.RemoveAll();
  FR_RELEASE_DEL(m_pLastExternalContur);
  FR_RELEASE_DEL(m_pLastInternalContur);
  FR_RELEASE_DEL(m_pPartConturOnObservation);
  FR_RELEASE_DEL(m_pHoleConturOnObservation);
  FR_RELEASE_DEL(m_pHoleConturOnMeasPlace);
  FR_RELEASE_DEL(m_pPartInternalEdge);
  FR_RELEASE_DEL(m_pPartExternalEdge);
  FR_RELEASE_DEL(m_pDebugging);
  FR_RELEASE_DEL(m_VectorsToVertices);

  PTR_CHECK_DEL(m_pLastActiveExternal);
  PTR_CHECK_DEL(m_pLastActiveInternal);

  PTR_CHECK_DEL(m_pInput);
  PTR_CHECK_DEL(m_pOutput);
  PTR_CHECK_DEL(m_pLightControl);
  PTR_CHECK_DEL(m_pXMotion);
  PTR_CHECK_DEL(m_pYMotion);
  PTR_CHECK_DEL(m_pMarking);
  PTR_CHECK_DEL(m_pImageSave);
  PTR_CHECK_DEL(m_pMainView);
  PTR_CHECK_DEL(m_pDuplex);

  if (!m_SystemConfigFileName.IsEmpty())
  {
    FXPropKit2 pkSys;
    pkSys.WriteDouble("MeasScale_pix_per_um", m_dScale_pix_per_um);
    pkSys.WriteDouble("ObservScale_pix_per_um", m_dObservScale_pix_per_um);
    pkSys.WriteCmplx("MeasCenter", m_cMeasCenter);
    pkSys.WriteDouble("BaseHeight", m_dBaseHeight);
    pkSys.WriteDouble("AdapterHeight", m_dAdapterHeight);
    pkSys.WriteCmplx("MeasRange", m_cMeasRange);
    pkSys.WriteCmplx("ObservCent", m_cObservCenter);
    pkSys.WriteCmplx("PartPlacePos", m_cPartPlacementPos);
    TCHAR AsText[1000];
    m_DistFromObservationToMeasurement.ToString(AsText, 1000, "%lf,%lf,%lf");
    pkSys.WriteString("ObservToMeasDist", AsText);

  }

  m_ObservGeomAnalyzer.Save();
}

void ConturAsm::AsyncTransaction(CDuplexConnector* pConnector, CDataFrame* pParamFrame)
{
  if (!pParamFrame)
    return;
  CTextFrame* tf = pParamFrame->GetTextFrame(DEFAULT_LABEL);
  if (tf)
  {
    FXParser pk = (LPCTSTR)tf->GetString();
    FXString cmd;
    FXString param;
    FXSIZE pos = 0;
    pk.GetWord(pos, cmd);

    if ((cmd.CompareNoCase("set") == 0))
    {
      param = pk.Mid(pos + 1);
      if (!param.IsEmpty())
      {
        bool bInvalidate = false;
        if (ScanProperties(param, bInvalidate))
          pk = _T("OK");
      }
    }
    else if ((cmd.CompareNoCase("get") == 0))
    {
      param = pk.Mid(pos + 1);
      if (!param.IsEmpty())
      {
        FXPropKit2 Properties;
        if (PrintProperties(Properties))
        {
          FXSIZE iPos = 0;
          FXString Token = param.Tokenize(_T(";"), iPos);
          pk.Empty();
          while (!Token.IsEmpty())
          {
            FXString Value;
            if (Properties.GetString(Token, Value))
            {
              pk += Token + _T('=') + Value + ';';
            }
            Token = param.Tokenize(_T(";"), iPos);
          }
          if (pk.IsEmpty())
            pk.Format(_T("ERROR: Properties %s not found"), param);
          {
          }
        }
        else
          pk = _T("ERROR: Can't print properties");
      }
    }
    else
    {
      pk = _T("List of command\n"
        "set <PropName1>=<PropValue1>;<PropName2>=<...\r\n"
        "set SaveSysConfig=<FileName; - save system parameters");
    }
    CTextFrame* retV = CTextFrame::Create(pk);
    retV->ChangeId(NOSYNC_FRAME);
    if (!m_pDuplex->Put(retV))
      retV->RELEASE(retV);

  }
  pParamFrame->Release(pParamFrame);
}

// for debugging without system
static bool bObserv = false;
static bool bMeas = false;

CDataFrame* ConturAsm::DoProcessing(const CDataFrame* pDataFrame)
{
  double dTBegin = GetHRTickCount();
  if (m_GadgetInfo.IsEmpty())
  {
    GetGadgetName(m_GadgetInfo);
    SENDINFO("Make %s %s Config=%s Platf=%s", __DATE__, __TIME__, FX_CONFIG, _FX_PLATF);
    RequestCurrentPosition();
  }

  const CTextFrame * pScanControl = NULL ;
  FXString ScanCommand ;
  datatype InputDType = pDataFrame->GetDataType();

  if ( InputDType == text )
  {
    pScanControl = pDataFrame->GetTextFrame( _T( "ScanControl" ) );
    if ( pScanControl )
    {
      ScanCommand = pScanControl->GetString();
      // Stop part edge scanning
      if ( ( ScanCommand == _T( "Stop" ) ) || ( ScanCommand == _T( "Reset" ) ) )
      {
        if ( !IsInactive() &&
          ( IsEmbeddedFocus() || IsOneDarkMeasurement()
          || m_StageBeforeMotion == Stage_BeginToMax
          || m_StageBeforeMotion == Stage_EndToMax )
          )
        {
          CTextFrame * pFullReport = CreateTextFrame(
            "Measurement Stop" , "CloseFile" , 1000000 );
        }
        SetProcessingStage( Stage_Inactive , true , _T( "Stop Received" ) , true );
        while ( !m_DelayedOperations.empty() )
          m_DelayedOperations.pop();
        m_MotionMask = 0;
        m_iNWaitsForMotionAnswers = 0;
        m_bInFocusing = m_bInLineFocusing = false;
      }
    }
  }

  if ( m_iSimulationMode )
  {
    bObserv = ( m_ProcessingStage == Stage_2_MeasOnObserv ) ;
    if ( bObserv )
    {
      bMeas = false ;
    }
    else
    {
      bMeas = ( ( Stage_10_GetIntOrExtBorder <= m_ProcessingStage )
        && ( m_ProcessingStage <= Stage_OneDarkMeasEndToMax ) ) ;
    }
  }
  // Platform on observation place
  bool bIsOnObservationPlace = bObserv ||
    (abs(GetCurrentCmplxPos() - m_cObservCenter) < 20000.);

  // Platform on measurement place
  bool bIsOnMeasurementPlace = !bObserv && (bMeas ||
    (abs(GetCurrentCmplxPos() - m_cMeasCenter) < 30000.) );

  m_bShiftKeyIsDown = IsShiftKeyDown();
  m_bCTRLKeyIsDown = IsCTRLKeyDown();

  if ( InputDType == quantity)
  {                         // Separate quantity frame received
    const CQuantityFrame * pFocusData =
      pDataFrame->GetQuantityFrame(_T("DiffSum"));
    if (pFocusData) // Event - new value from focus measurement received
    {
      m_dLastFocusValue = (double)(*pFocusData);
      if (IsFocusing())
      {
        switch (m_ProcessingStage)
        {
        case Stage_Focusing: // if we did additional imaging cycle
                             // order next light cycle for focusing
          SetLightForExternal(m_dLightForExternal, m_iExtLight_us,
            m_dLastLightAngle);
          return NULL;
        }
      }
      else
      {
        m_dLastFocusTime = pFocusData->GetTime();
      }
    }

    const CQuantityFrame * pFocuZ = pDataFrame->GetQuantityFrame(_T("FocusFound"));
    if (pFocuZ) // Event - focus search process finished and Z-stage arrived
                  //         to focal point
    {
      m_dLastFocusCoordinate = (double)(*pFocuZ);
      CTextFrame * pFoundFocusPlace = CreateTextFrame("Status", (LPCTSTR)NULL);
      pFoundFocusPlace->GetString().Format("Focus Found on Z=%.1f Stage is %s",
        m_dLastFocusCoordinate, GetStageDescription(m_ProcessingStage));
      PutFrame(m_pOutput, pFoundFocusPlace);

      if (IsFocusing())
      {
        m_bInFocusing = false;
        m_bInLineFocusing = false;
        switch (m_ProcessingStage)
        {
        case Stage_Focusing: // Focus is found
          {
            m_bFocusFound = true;
            SetProcessingStage(Stage_Inactive, true);
            break;
          }
        case Stage_3_AutoFocusOnObserv:
          {
            if (!SetCamerasExposure(m_iObservationMeasExp_us,
              _T("Can't adjust exposure for observation focusing"),
              Stage_Inactive))
            {
              return NULL;
            }
            SetProcessingStage(Stage_4_MeasFocusedOnObserv, true); // will be processed after 
            Sleep(80);                                     // final observation measurement
            OrderImagingTaskAndLight(10, CAM_OBSERVATION,
              OBSERVATION_LEDS, m_iObservationMeasExp_us);
          }
          break;
        case Stage_8_GetInternalBorder:
          {
            m_bInMeasurementFocusFound = true;
            if (m_WorkingMode == WM_DarkEdge_NoInternal)
              SetLightToCenter();
            else
              SetLightFromCenter();
            SetProcessingStage(Stage_9_MoveEdgeToCenter, true); // Edge to center after 
                                                      // initial focusing 

          }
          break;
        case Stage_13_ScanInlineFocusing:
          {
            m_bInMeasurementFocusFound = true;
            if (m_WorkingMode == WM_DarkEdge_NoInternal)
              SetLightToCenter();
            else
              SetLightFromCenter();
            SetProcessingStage(Stage_10_GetIntOrExtBorder, true); // Get internal or external border 
                                                     // after in line focusing 
          }
        }
      }
      return NULL;
    }
  }
  if (InputDType == text)
  {
    const CTextFrame * pText = (const CTextFrame*)(pDataFrame->GetTextFrame());
    FXString Content = pText->GetString();
    if (Content == _T("Stop"))
    {
      if (!IsInactive() &&
        (IsEmbeddedFocus() || IsOneDarkMeasurement()
          || m_StageBeforeMotion == Stage_BeginToMax
          || m_StageBeforeMotion == Stage_EndToMax)
        )
      {
        CTextFrame * pFullReport = CreateTextFrame(
          "Measurement Stop", "CloseFile", 1000000);
        PutFrame(m_pOutput, pFullReport); // For new file creation
      }
      SetProcessingStage(Stage_Inactive, true, _T("Stop Received"), true);
      while (!m_DelayedOperations.empty())
        m_DelayedOperations.pop();
      m_MotionMask = 0;
      m_iNWaitsForMotionAnswers = 0;
      m_bInFocusing = m_bInLineFocusing = false;
      StopMotion();
      SENDINFO("Command Stop received");

      CTextFrame * pLightControl = CreateTextFrame("astbon 0\r", "LightControl");
      PutFrame(m_pLightControl, pLightControl);
      Sleep(100);

      m_dStopReceivedTime = GetHRTickCount();
      return NULL;
    }
    FXString Label = pDataFrame->GetLabel();
    if (!IsInactive())
    {
      if (Label.Find(_T("Timeout")) == 0)// Check for timeout in scan process
      {
        if (IsInScanMotion())
        {
          CTextFrame * pRequestCoord = CreateTextFrame(_T(
            "Return_Current_Position=1\r\n"), _T("GetCurrPos"));
          if (!PutFrame(m_pLightControl, pRequestCoord))
            SEND_GADGET_WARN(_T("Can't send Pos Request on timeout"));
          else
            SEND_GADGET_WARN(_T("Timeout on motion: %s") , (LPCTSTR)m_WatchDogMessage );
          if (m_bDoMotionLog)
          {
            FXString LogMsg;
            LogMsg.Format("Current((%.1f,%.1f,%.1f) Mask=%d",
              m_CurrentPos.m_x, m_CurrentPos.m_y, m_CurrentPos.m_z,
              m_MotionMask);

            m_MotionLogger.AddMsg((LPCTSTR)LogMsg, "Timeout: ");
          }


          // SetProcessingStage( Stage_10_GetIntOrExtBorder , true ) ; // Get internal or external border 
                                                    // after in line focusing 
        }
        else if (IsInScanLighting())
        {
          CTextFrame * pLightControl = CreateTextFrame(
            m_sLastLightCommand, "LightControl");
          if (!PutFrame(m_pLightControl, pLightControl))
            SEND_GADGET_ERR("Can't set working LEDs on timeout");
          else
            SEND_GADGET_WARN(_T("Timeout on light flash: %s" ) , ( LPCTSTR ) m_WatchDogMessage );
          CTextFrame * pRequestCoord = CreateTextFrame(_T(
            "Return_Current_Position=1\r\n"), _T("GetCurrPos"));
          if (!PutFrame(m_pLightControl, pRequestCoord))
            SEND_GADGET_WARN(_T("Can't send Pos Request on timeout"));
          else
            SEND_GADGET_WARN(_T("Timeout on motion: %s" ) , ( LPCTSTR ) m_WatchDogMessage );
        }
        else if (m_StageBeforeMotion == Stage_Focusing)
        {
          CTextFrame * pRequestCoord = CreateTextFrame(_T(
            "Return_Current_Position=1\r\n"), _T("GetCurrPos"));
          if (!PutFrame(m_pLightControl, pRequestCoord))
            SEND_GADGET_WARN(_T("Can't send Pos Request on timeout"));
          else
            SEND_GADGET_WARN(_T("Timeout on motion: %s" ) , ( LPCTSTR ) m_WatchDogMessage );
        }
        return NULL;
      }
    }
    else
    {
      // separated text frame received and system is in inactive state
      const CTextFrame * pSetSectors = pDataFrame->GetTextFrame(_T("SetSectors"));
      if (pSetSectors)
      {
        FXPropKit2 pk = pSetSectors->GetString();
        double dRange;
        if (pk.GetDouble("range_um", dRange))
        {
          if (dRange == 0.)
            m_SectorsMarkingMode = SMM_NoMarking;
          m_dSectorRange_um = dRange;
        }
        int iMarkMode = 0;
        if (pk.GetInt("marking_mode", iMarkMode))
        {
          if (-2 <= iMarkMode && iMarkMode <= 1)
            m_SectorsMarkingMode = (SectorMarkingMode)iMarkMode;
          if (m_SectorsMarkingMode == SMM_DeleteAll)
          {
            m_CurrentDetailedForm.m_Sectors.RemoveAll();
            m_SectorsMarkingMode = SMM_NoMarking;
          }
        }
      }

      const CTextFrame * pObservViewCoords = pDataFrame->GetTextFrame(_T("Observation"));
      if (pObservViewCoords && m_bShiftKeyIsDown)
      {    // Cursor Coordinates from observation image are received
           // If shift is pressed - send XY to correspondent point in measurement space
        if (GetHRTickCount() - m_dLastObservCoordTime > 2000.)
        {
          FXPropertyKit pk(pObservViewCoords->GetString());
          int iX, iY;
          if (pk.GetInt("x", iX) && pk.GetInt("y", iY))
          {
            cmplx PtInObservation(iX, iY);
            CCoordinate Target = GetTargetForObservationPoint(PtInObservation);
            Target.m_z = m_CurrentPos.m_z;
            OrderAbsMotion(Target);
            m_DelayedOperations.push(Stage_GetAllLightsImage);

            m_dLastObservCoordTime = GetHRTickCount();
          };
        }
        return NULL;
      }
      const CTextFrame * pFromMap = pDataFrame->GetTextFrame(_T("FromMap"));
      if (pFromMap)
      {
        bool bRes = ProcessMsgFromMap(pFromMap);

        return NULL;
      }
      const CTextFrame * pFromUI = pDataFrame->GetTextFrame(_T("FromUI"));
      if (pFromUI)
      {
        bool bRes = ProcessMsgFromUI(pFromUI);
        return NULL;
      }
      const CTextFrame * pDoHoming = pDataFrame->GetTextFrame(_T("DoHoming"));
      if (pDoHoming)
      {   // Send stage Z to home position (it's possible to do only through FindExtem gadget
        CTextFrame * pHomeZCommand = CreateTextFrame("Home=1;", "SendToMotion");
        if (PutFrame(m_pLightControl, pHomeZCommand))
        {
          m_HomeSetMask |= MotionMaskZ;
          m_MotionMask |= MotionMaskZ;
          m_ProcessingStage = Stage_HomeZ;
        }
        return NULL;
      }
      const CTextFrame * pMoveRelative = pDataFrame->GetTextFrame(_T("MoveRel"));
      if (pMoveRelative)
      {
        FXPropKit2 pk(pMoveRelative->GetString());
        double dDX = 0., dDY = 0., dDZ = 0.;
        bool bMoveX = pk.GetDouble("X", dDX);
        bool bMoveY = pk.GetDouble("Y", dDY);
        bool bMoveZ = pk.GetDouble("Z", dDZ);

        OrderRelMotion(dDX, dDY, dDZ);
        if (dDX == 0. && dDY == 0. && dDZ == 0.)
        {
          SetProcessingStage(Stage_Inactive, true, _T("No motion data or Out of Range"));
        }
        return NULL;
      }

      const CTextFrame * pDoView = pDataFrame->GetTextFrame(_T("DoView"));
      if (pDoView)
      {
        // External image capture request
        // INput text frame consists of property kit with capture parameters
        FXPropKit2 pk(pDoView->GetString());
        BOOL bStop = FALSE;
        FXString LightOut;
        bool bIsStop = pk.GetInt("Stop", bStop);

        int iCam = (m_LastLightMask & (1 << 10)) ? 0 : 1;

        int iPulseLen = m_iLastPulseTime, iDoOnce = 0, iLDelta = 0;
        UINT Mask = m_LastLightMask & 0x3ff;


        bool bPulse = pk.GetInt("PulseTime", iPulseLen);
        bool bMask = pk.GetUIntOrHex("BitMask", Mask);
        bool bCam = pk.GetInt("CameraNum", iCam);
        iCam &= 1;
        DWORD dwCam = 1 << (iCam + 10);
        bool bOtherContent = bCam || bMask || bPulse
          || pk.GetInt("DoOnce", iDoOnce);

        if (pk.GetInt("LDelta", iLDelta))
        {
          if (iLDelta <= -100)
            iLDelta = 0;
          else
          {
            iPulseLen = ROUND(iPulseLen * (1. + iLDelta * 0.01));
            bPulse = bOtherContent = true;
          }
        }
        FXString Operation;
        if (!bOtherContent && bStop)
        {
          CTextFrame * pLightControl = CreateTextFrame(LightOut, "ab\r");
          PutFrame(m_pLightControl, pLightControl);
          Operation = _T("Stopped");
        }
        else
        {
          m_LastLightMask = (1 << (iCam + 10)) | (Mask & 0x3ff);
          if (bPulse)
          {
            m_iLastPulseTime = iPulseLen;
            SetCamerasExposure(m_iLastPulseTime + 200,
              "DoView order failed", Stage_Inactive);
          }
          if (iDoOnce)
          {
            if (!bStop)
            {
              SetWorkingLeds(iCam, Mask, iPulseLen);
              Operation = _T("Once");
            }
          }
          else
          {
            LightOut.Format("astbset 0x%04X %d\r",
              m_LastLightMask, iPulseLen);
            CTextFrame * pLightControl = CreateTextFrame(LightOut, "LightControl");

            PutFrame(m_pLightControl, pLightControl);
            m_sLastLightCommand = LightOut;
            Sleep(20);
            pLightControl = CreateTextFrame(LightOut, "isdset 100000\r");
            PutFrame(m_pLightControl, pLightControl);
            if (!bStop)
            {
              Sleep(20);
              pLightControl = CreateTextFrame(LightOut, "astbon 1\r");
              Operation = _T("Continuously");
            }
            else
              Operation = "Programmed";

            TRACE("\n%s  Light out by Mask=0x%04X Tl=%d",
              GetStageName(),
              m_LastLightMask, iPulseLen);
          }
        }
        pk.Format(_T("DoView: %s - PulseTime=%d CameraNum=%d"
          "BitMask=0x%04X Stop=%d DoOnce=%d"),
          (LPCTSTR)Operation,
          iPulseLen, iCam, Mask, bStop, iDoOnce);

        CTextFrame * pAnswer = CreateTextFrame(pk, _T("DoView"));
        return NULL;
      }

      const CTextFrame * pDoFocusing = pDataFrame->GetTextFrame(_T("DoFocusing"));
      if (pDoFocusing)  // External focusing request
      {
        InitIterativeFocusing(Stage_Inactive);

        return NULL;
      }
      const CTextFrame * pLiveView = pDataFrame->GetTextFrame(_T("LiveView"));
      if (pLiveView)  // LiveView request
      {
        m_DataForPosition.RemoveAll();
        m_CurrentViewMeasStatus.Reset(true);
        m_dLastdZ = 0.;

        m_LastLightMask = bIsOnMeasurementPlace ? 0x8ff : 0x700;
        int iPulseLen = bIsOnMeasurementPlace ? m_iMeasurementExposure_us : m_iObservationMeasExp_us;
        FXString LightOut;
        LightOut.Format("astbset 0x%04X %d\r",
          m_LastLightMask, iPulseLen);
        CTextFrame * pLightControl = CreateTextFrame(LightOut, "LightControl");

        PutFrame(m_pLightControl, pLightControl);
        m_sLastLightCommand = LightOut;
        Sleep(20);
        pLightControl = CreateTextFrame("isdset 100000\r", "LightControl");
        PutFrame(m_pLightControl, pLightControl);
        Sleep(20);
        pLightControl = CreateTextFrame("astbon 1\r", "LightControl");
        PutFrame(m_pLightControl, pLightControl);

        TRACE("\n%s  Light out by Mask=0x%04X Tl=%d",
          GetStageName(),
          m_LastLightMask, iPulseLen);
        //         SetTask(0);
        //         SetLightForDarkEdge(0);

        return NULL;
      }

      const CTextFrame * pCheckLight = pDataFrame->GetTextFrame(_T("CheckLight"));
      if (pCheckLight)
      {
        bool bRes = false;
        CTextFrame * pSetMaxPulseTime = CreateTextFrame(_T("tmax 30000\r"), _T("LightControl"));
        PutFrame(m_pLightControl, pSetMaxPulseTime);

        m_iNMotionSteps = 0;
        m_ProcessingStage = Stage_CalibLight0;
        CCoordinate Target(m_ccObservCenter);
        Target.m_z -= m_dAdapterHeight;
        if (!IsAllCoordsAvailable())
        {
          CTextFrame * pRequestCoord = CreateTextFrame(
            _T("Set_Home_Status=1\r\n"), _T("GetCurrPos"));
          if (PutFrame(m_pLightControl, pRequestCoord))
          {
            m_MotionMask = MotionMaskX | MotionMaskY | MotionMaskZ;
            m_iNWaitsForMotionAnswers = 3;
            SetProcessingStage(Stage_CalibLight0);
            m_DelayedOperations.push(Stage_CalibLight0);
          }
          Sleep(200);
        }
        else
        {
          SetMotionSpeed(m_iXYMotionScanSpeed_um_sec);
          Sleep(100);
          SetZMotionSpeed(20000.);
          Sleep(100);

          if (abs(m_cObservCenter - m_cRobotPos) < 100.
            && fabs(Target.m_z - m_CurrentPos.m_z) < 500.)
          {
            ProcessLightCalibration(); // go to Stage_CalibLight1
          }
          else
          {
            OrderAbsMotion(Target);
            m_DelayedOperations.push(Stage_CalibLight0);
            SetProcessingStage(Stage_CalibLight0);
          }
        }
        return NULL;
      }
      const CTextFrame * pSavePos = pDataFrame->GetTextFrame(_T("SavePos"));
      if (pSavePos)
      {   // Save Current position for future using
        int iPos = atoi((LPCTSTR)(pSavePos->GetString()));
        if ((0 <= iPos) && (iPos < ARRSZ(m_SavedPositions)))
        {
          m_SavedPositions[iPos] = m_CurrentPos;
        }
        else
        {
          SEND_GADGET_ERR("SavePos Cmnd has incorrect argument '%s'",
            (LPCTSTR)(pSavePos->GetString()));
        }
        return NULL;
      }
      const CTextFrame * pGoToPos = pDataFrame->GetTextFrame(_T("GoToPos"));
      if (pGoToPos)
      {   // Save Current position for future using
        int iPos = atoi((LPCTSTR)(pGoToPos->GetString()));
        if ((0 <= iPos) && (iPos < ARRSZ(m_SavedPositions))
          && (m_SavedPositions[iPos].m_x != 0.))
        {
          OrderAbsMotion(m_SavedPositions[iPos]);
        }
        else
        {
          if (m_SavedPositions[iPos].m_x != 0.)
          {
            SEND_GADGET_ERR("GoToPos Cmnd has incorrect argument '%s'",
              (LPCTSTR)(pGoToPos->GetString()));
          }
          else
          {
            SEND_GADGET_ERR("GoToPos Cmnd: Pt#%d is not saved; Cmnd is '%s'",
              iPos, (LPCTSTR)(pGoToPos->GetString()));
          }
        }
        return NULL;
      }
      const CTextFrame * pLoadSavedData = pDataFrame->GetTextFrame(_T("LoadSavedData"));
      if (pLoadSavedData)
      {   // Get FilePath
        FXString FileData(pLoadSavedData->GetString());
        FXSIZE  iCommaPos = FileData.Find(_T(','));
        if (iCommaPos > 0)
        {
          int iEquPos = (int)FileData.Find(_T('='), iCommaPos);
          if (iEquPos > 0)
          {
            int iDelay = atoi(((LPCTSTR)FileData) + iEquPos + 1);
            m_iDelayBetweenSegmentsOnLoading =
              (iDelay > 0) ? iDelay : 50;
          }
          FileData = FileData.Left(iCommaPos);
        }
        else
          m_iDelayBetweenSegmentsOnLoading = 1;

        FXString Path = FxExtractPath(FileData);
        FXString FileName = FxGetNotExpandedFileName(FileData);
        LoadResults(FileName, Path);
        return NULL;
      }

    }
  }

  m_pDebugging = NULL;
  const CTextFrame * pViewControl = pDataFrame->GetTextFrame(_T("ViewControl"));
  if (pViewControl)
  {
    ResultViewMode Mode = RVM_Unknown;
    FXString ViewCommand = pViewControl->GetString();
    if (ViewCommand == _T("ViewWidth"))
      Mode = RVM_BurrWidth;
    else if (ViewCommand == _T("ViewMaxWidth"))
      Mode = RVM_MaxBurrWidth;
    else if (ViewCommand == _T("ViewHeight"))
      Mode = RVM_BurrHeight;
    else if (ViewCommand == _T("ViewMaxHeight"))
      Mode = RVM_MaxBurrHeight;
    else if (ViewCommand == _T("ViewVolume"))
      Mode = RVM_BurrVolume;
    else if (ViewCommand == _T("ViewBurrEdgeLength"))
      Mode = RVM_BurrEdgeLength;

    if (Mode != RVM_Unknown)
    {
      m_iSelectedPtIndex = -1;
      CDataFrame * pMap = FormMapImage(NULL, Mode, (int)m_ConturData.GetUpperBound());
      if (pMap)
      {
        PutFrame(m_pOutput, pMap);
      }
    }
    return NULL;
  }

  if ( pScanControl )  // message from motion or from manual controls or application
  {
    Axis Ax = DecodeInputString(pScanControl->GetString());

    if (m_bDoMotionLog)
    {
      FXString LogMsg;
      LogMsg.Format("From motion '%s' Current((%.1f,%.1f,%.1f) Mask=0x%x",
        (LPCTSTR)pScanControl->GetString() ,
        m_CurrentPos.m_x, m_CurrentPos.m_y, m_CurrentPos.m_z,
        m_MotionMask);

      m_MotionLogger.AddMsg((LPCTSTR)LogMsg, "On entry: ");
    }

    if (Ax == FromZaber)
    {
      TRACE("\n%s    Not position msg: %s",
        GetStageName(),
        (LPCTSTR)pScanControl->GetString());
      if (IsLightCalibration())
      {
        Sleep(100);
        ProcessLightCalibration(pDataFrame);
      }
      if (m_bDoMotionLog)
      {
        FXString LogMsg;
        LogMsg.Format("Current((%.1f,%.1f,%.1f) Mask=%d",
          m_CurrentPos.m_x, m_CurrentPos.m_y, m_CurrentPos.m_z,
          m_MotionMask);

        m_MotionLogger.AddMsg((LPCTSTR)LogMsg, "Info from motion : ");
      }

      return NULL; // some message from Zaber, like speed switch done
    }

    if (Ax != Axis_Undef) // robot position message
    {
      m_iNMotionSteps++;
      switch (Ax)
      {
      case Axis_X:
        {
          if (m_InitPos.m_x == 0.)
            m_cCurrentTarget._Val[ _RE ] = m_InitPos.m_x = m_CurrentPos.m_x;
          m_cRobotPos._Val[_RE] = m_CurrentPos.m_x;
          m_ReceivedCoords |= MotionMaskX;
          if (!(m_HomeSetMask & MotionMaskX))
          {
            CTextFrame * pHomeDoneStatus = CreateTextFrame(
              _T("Set_Home_Status=1\r\n"), (LPCTSTR)NULL);
            if (PutFrame(m_pXMotion, pHomeDoneStatus))
              m_HomeSetMask |= MotionMaskX;
          }
          else
          {
            if (m_ProcessingStage == Stage_HomeX)
            {
              CTextFrame * pHomeYCommand = CreateTextFrame("Home=1", (LPCTSTR)NULL);
              if (PutFrame(m_pYMotion, pHomeYCommand))
              {
                m_HomeSetMask = MotionMaskY;
                m_MotionMask = MotionMaskY;
                m_ProcessingStage = Stage_HomeY;
                m_iNWaitsForMotionAnswers = 1;
              }
            }
          }
          if (m_ProcessingStage != Stage_HomeY)
          {
            if (IsWaitForCoordinates())
            {
              if (m_iNWaitsForMotionAnswers > 0)
                --m_iNWaitsForMotionAnswers;
              m_MotionMask &= ~MotionMaskX;
            }
            if (m_MotionSequence != MS_NoMotion
              && m_MotionSequence != MS_Both)
            {
              OrderAbsMotion(m_cFinalTarget);
              m_MotionSequence = MS_Both;
              if (m_StageBeforeMotion == Stage_Load_Part)
                m_ProcessingStage = Stage_Load_Part;
            }
          }
          TRACE("\n%s---- X C(%8.1f,%8.1f,%8.1f) Mask=0x%02x NW=%d Nq=%d  ",
            GetStageName(),
            m_CurrentPos.m_x, m_CurrentPos.m_y, m_CurrentPos.m_z,
            m_MotionMask, m_iNWaitsForMotionAnswers,
            m_pInput->GetNFramesInQueue());
          if (m_ProcessingStage == Stage_Load_Part)
            SetProcessingStage(Stage_Inactive);
        }
        break;
      case Axis_Y:
        {
          if (m_InitPos.m_y == 0.)
            m_cCurrentTarget._Val[_IM] = m_InitPos.m_y = m_CurrentPos.m_y;
          m_cRobotPos._Val[_IM] = m_CurrentPos.m_y;
          m_ReceivedCoords |= MotionMaskY;
          if (!(m_HomeSetMask & MotionMaskY))
          {
            CTextFrame * pHomeDoneStatus = CreateTextFrame(
              _T("Set_Home_Status=1\r\n"), (LPCTSTR)NULL);
            if (PutFrame(m_pYMotion, pHomeDoneStatus))
              m_HomeSetMask |= MotionMaskY;
          }
          else
          {
            if (m_ProcessingStage == Stage_HomeY)
            {
              CTextFrame * pReport = CreateTextFrame(
                _T("Homing Finished"), _T("Status"));
              PutFrame(m_pOutput, pReport);
              m_HomeSetMask = 0;
              m_MotionMask = 0;
              m_ProcessingStage = Stage_Inactive;
              m_iNWaitsForMotionAnswers = 0;
              m_MotionSequence = MS_NoMotion;
              SEND_GADGET_INFO("Homing Finished");
            }
          }
          if (IsWaitForCoordinates())
          {
            if (m_iNWaitsForMotionAnswers > 0)
              --m_iNWaitsForMotionAnswers;
            m_MotionMask &= ~MotionMaskY;
          }
          if (m_MotionSequence != MS_NoMotion
            && m_MotionSequence != MS_Both)
          {
            OrderAbsMotion(m_cFinalTarget);
            m_MotionSequence = MS_Both;
            if (m_StageBeforeMotion == Stage_Load_Part)
              m_ProcessingStage = Stage_Load_Part;
          }
          TRACE("\n%s---- Y C(%8.1f,%8.1f,%8.1f) Mask=0x%02x NW=%d Nq=%d  ",
            GetStageName(),
            m_CurrentPos.m_x, m_CurrentPos.m_y, m_CurrentPos.m_z,
            m_MotionMask, m_iNWaitsForMotionAnswers,
            m_pInput->GetNFramesInQueue());
        }
        break;
      case Axis_Z:
        {
          if (m_InitPos.m_z == 0.)
            m_dZTarget = m_InitPos.m_z = m_CurrentPos.m_z;
          m_ReceivedCoords |= MotionMaskZ;
          if (!(m_HomeSetMask & MotionMaskZ))
          {
            CTextFrame * pHomeDoneStatus = CreateTextFrame(
              _T("Set_Home_Status=1\r\n"), _T("SendToMotion"));
            if (PutFrame(m_pLightControl, pHomeDoneStatus))
              m_HomeSetMask |= MotionMaskZ;
          }
          else
          {
            if (m_ProcessingStage == Stage_HomeZ)
            {
              CTextFrame * pHomeXCommand = CreateTextFrame("Home=1;", (LPCTSTR)NULL);
              if (PutFrame(m_pXMotion, pHomeXCommand))
              {
                m_HomeSetMask = MotionMaskX;
                m_MotionMask = MotionMaskX;
                m_ProcessingStage = Stage_HomeX;
                m_iNWaitsForMotionAnswers = 1;
              }
            }
          }
          if ((m_ProcessingStage != Stage_HomeX))
          {
            if ((m_MotionMask & MotionMaskZ)
              && (m_iNWaitsForMotionAnswers > 0))
            {
              --m_iNWaitsForMotionAnswers;
              m_MotionMask &= ~MotionMaskZ;
            }
          }

          TRACE("\n%s---- Z C(%8.1f,%8.1f,%8.1f) Mask=0x%02x NW=%d Nq=%d  ",
            GetStageName(),
            m_CurrentPos.m_x, m_CurrentPos.m_y, m_CurrentPos.m_z,
            m_MotionMask, m_iNWaitsForMotionAnswers,
            m_pInput->GetNFramesInQueue());
        }
        break;
      }
      double dAbsDeltaX = fabs(m_cCurrentTarget.real() - m_CurrentPos.m_x);
      if (dAbsDeltaX > 10.)
        m_MotionMask |= MotionMaskX ;
      else if (dAbsDeltaX > 2.)
        m_MotionMask &= ~MotionMaskX;
      
      double dAbsDeltaY = fabs(m_cCurrentTarget.imag() - m_CurrentPos.m_y);
      if ( dAbsDeltaY > 10. )
        m_MotionMask |= MotionMaskY ;
      else if ( dAbsDeltaY < 2. )
        m_MotionMask &= ~MotionMaskY;

      double dAbsDeltaZ = fabs(m_dZTarget - m_CurrentPos.m_z);
      if ( dAbsDeltaZ > 10. )
        m_MotionMask |= MotionMaskZ ;
      else if (dAbsDeltaZ < 2.)
        m_MotionMask &= ~MotionMaskZ;

      if (m_bDoMotionLog)
      {
        FXString LogMsg;
        LogMsg.Format("Target(%.1f,%.1f,%.1f) Current((%.1f,%.1f,%.1f) Mask=%d Nwaits=%d",
          m_cCurrentTarget.real(), m_cCurrentTarget.imag(), m_dZTarget,
          m_CurrentPos.m_x, m_CurrentPos.m_y, m_CurrentPos.m_z,
          m_MotionMask , m_iNWaitsForMotionAnswers );

        m_MotionLogger.AddMsg((LPCTSTR)LogMsg, "After decode: ");
      }


      if (m_MotionMask == 0)
      {
        DeleteWatchDog();

        double dDistFromTarget = abs( m_cFinalTarget - m_cRobotPos ) ;
        if (m_iNWaitsForMotionAnswers)
        {
          TRACE("\ns---- Error: m_iNWaitsForMotionAnswers=%d when MotionMask==0",
            GetStageName(),
            m_iNWaitsForMotionAnswers);
        }
        m_iNWaitsForMotionAnswers = 0;
        if (m_ProcessingStage == Stage_Focusing)
        {  // simple grab image and wait for this image
          SetLightForDarkEdge(-1); // nothing to measure, we need image only
          return NULL;
        }
        if (IsLightCalibration())
        {
          ProcessLightCalibration(pDataFrame);
          return NULL;
        }
        if (m_ProcessingStage == Stage_Load_Part)
        {
          m_ProcessingStage = Stage_Inactive;
          return NULL;
        }
        if (m_bReportAfterMotion
          && !m_StringAfterMotion.IsEmpty()
          && !IsInScanProcess())
        {
          SetProcessingStage(Stage_Inactive, true,
            m_StringAfterMotion);
          m_bReportAfterMotion = false;
          m_StringAfterMotion.Empty();
        }
        m_cRobotPosToInitialPt = m_cRobotPos - m_cInitialRobotPosForMeas;
        if (!IsGoToFocusPosition()  // 
          || IsEmbeddedFocus())     // Focus step
        {
          CheckOperationStack(false);
        }
      }

      m_cRobotPos = cmplx(m_CurrentPos.m_x, m_CurrentPos.m_y);
      cmplx ToCenter = m_cRobotPos - m_cPartCentPos;
      m_dLastAngleToCenter = arg(conj(ToCenter));

      if (m_MotionMask == 0)
      { // All motions are finished

        switch (m_StageBeforeMotion)
        {
        case Stage_BeginToMax:
        case Stage_EndToMax:
        case Stage_OneDarkMeasBeginToMax:
        case Stage_OneDarkMeasEndToMax:
        case Stage_Focusing:
          m_ProcessingStage = m_StageBeforeMotion;
          m_StageBeforeMotion = Stage_Inactive;
          break;
        }
        switch (m_ProcessingStage)
        {
        case Stage_GetCurrentPositions:
          CheckOperationStack();
          break;
        case Stage_ManualMotion:
          if (m_MotionSequence == MS_Both)
          {
            m_MotionSequence = MS_NoMotion;
            CheckOperationStack();
          }
          else if (m_cFinalTarget != 0.)
          {
            OrderAbsMotion(m_cFinalTarget);
            m_cFinalTarget = 0.;
            return NULL;
          }
          break;
        }
        // What to do after motion
        switch (m_ProcessingStage)
        {
        case Stage_Inactive: return NULL;  // nothing to do

        case Stage_0_GetCurrPos:
          {
            SetZMotionSpeed(20000.);
            Sleep(50);
            SetMotionSpeed(m_iXYMotionScanSpeed_um_sec);
            Sleep(50);
            SetMotionAcceleration(m_iAccel_units * 8, 0x03);
            SetCamerasExposure(m_iObservationMeasExp_us,
              "Can't set Observation Measurement Exposure", Stage_Inactive);
            CCoordinate Target(m_ccObservCenter);
            Target.m_z -= m_dPartHeight + m_dAdapterHeight;
            cmplx cCurPos = m_cRobotPos;
            if (MoveToTarget(Target))
            {
              m_DelayedOperations.push(Stage_1_MoveToObserv);
              FXString TS = GetTimeAsString_ms( ) + " Motion to Observation started (next stage 1)" ;
              CTextFrame * pEndOfScan = CreateTextFrame( TS , "Result");
              PutFrame(m_pOutput, pEndOfScan);
              double dDistX = abs(Target.m_x - cCurPos.real());
              double dDistY = abs(Target.m_y - cCurPos.imag());
              double dDist = max(dDistX, dDistY);
              double dTime = dDist * 2000. / m_iXYMotionScanSpeed_um_sec ; // timeout in ms; *2 - for insurance
              double dZ = fabs( Target.m_z - m_CurrentPos.m_z ) ;
              double dZTime = dZ * 2000. / m_iZMotionSpeed_um_sec ;
              if ( dZTime > dTime )
                dTime = dZTime ;

              SetWatchDog(ROUND(dTime + 5000.));
            }
            else // no motion is needed, order flash
            {
              if ( !m_iSimulationMode )
              {
                Sleep( 50 );
                OrderImagingTaskAndLight( 10 , CAM_OBSERVATION , OBSERVATION_LEDS , m_iObservationMeasExp_us );
                SetProcessingStage( Stage_2_MeasOnObserv , true ); // will be processed after 
                                                         // observation initial measurement
                CTextFrame * pEndOfScan = CreateTextFrame( "Part on observation (next stage 2)" ,
                  "Result" );
                PutFrame( m_pOutput , pEndOfScan );
              }
              else // simulation, order task and image
              {
                bIsOnMeasurementPlace = false ;
                bIsOnObservationPlace = true ;
                SetTask( 10 ) ;
                OrderBMPForSimulation( false ) ;
              }
            }
            return NULL;
          }
        case Stage_OneObservMeasurement:
        case Stage_1_MoveToObserv:
          {
            if ( abs( m_cRobotPos - m_cObservCenter ) > 100. )
            { // we are not on observation center (why?) 
              // necessary to correct position
              CCoordinate AdditionalTarget( m_cObservCenter ) ;
              AdditionalTarget.m_z = m_CurrentPos.m_z ;
              m_DelayedOperations.push( Stage_1_MoveToObserv );
              MoveToTarget( AdditionalTarget ) ;
              return NULL ;
            }
            CTextFrame * pEndOfScan = CreateTextFrame("Arrived to Observation",
              "Result");
            PutFrame(m_pOutput, pEndOfScan);

            if (SetCamerasExposure(m_iObservationMeasExp_us,
              "Can't set Observation Measurement Exposure", Stage_Inactive))
            {
              Sleep(40);
              OrderImagingTaskAndLight(10, CAM_OBSERVATION, OBSERVATION_LEDS, m_iObservationMeasExp_us);
              switch (m_ProcessingStage)
              {
              case Stage_1_MoveToObserv:
                SetProcessingStage(Stage_2_MeasOnObserv, true); // will be processed after 
                                                         // observation initial measurement
                break;
              case Stage_OneObservMeasurement:
                SetProcessingStage(Stage_Inactive, true); // nothing to do, onlyu view image
                break;
              }
            }
            return NULL;
          }
        case Stage_5_SendToEdgeMeasurement:
          {
            if (abs(m_cCurrentTarget - m_cRobotPos) < 10.)
            {
              //              SetLightForDarkEdge(0);

              SetLightForDarkEdge(6);

              SetProcessingStage(Stage_9_MoveEdgeToCenter, true);
              CTextFrame * pEndOfScan = CreateTextFrame(
                "Arrived to measurement, measurement ordered (next stage 9)",
                "Result");
              PutFrame(m_pOutput, pEndOfScan);
            }
            return NULL;
          }
          break;
        case Stage_6_MoveEdgeToCenter:
          {
            //             if ( !IsDarkMode() )
            //             {
            //               SetLightFromCenter() ;
            //               SetProcessingStage( Stage_7_AutoFocusEdge , true ) ;
            //             }
            //             else
            //             {
            //               SetLightForDarkEdge( 0 ) ;
            //               SetProcessingStage( )
            //             }

            return NULL;
          }
        case Stage_10_GetIntOrExtBorder: //end of motion before internal edge measurement
          {
            if (m_WorkingMode == WM_DarkEdge_NoInternal)
            {
              SetProcessingStage(Stage_12_MeasWalls, true);
              if (m_pLastExternalContur)
              {
                cmplx CentralPt = CDPointToCmplx(
                  m_pLastExternalContur->GetAt(m_iMinDistIndexForExternal));
                SetFocusingArea(CentralPt);
              }
              SetLightToCenter();
            }
            else
            {
              SetProcessingStage(Stage_11_GetExternalBorder, true); // Internal border will be measured 
              SetLightFromCenter();
            }
            return NULL;
          }
        case Stage_13_ScanInlineFocusing: // end of motion before in line focusing
          {
            if (!m_bInFocusing)
            {
              if (m_bIsFocusingNecessary)
              {
                m_sLastCmndForFocusingLightWithExt.Format(_T("out 0x8ff %d\r\n"),
                  m_iMeasurementExposure_us);
                m_dFocusingRange_um = m_dFocusRangeK;
                if (!StartFocusing( // usual in line focusing
                  (LPCTSTR)m_sLastCmndForFocusingLightWithExt,
                  500, m_iMeasurementExposure_us, 2))
                {
                  SEND_GADGET_ERR("%s Can't send focusing command",
                    GetStageName());
                  SetProcessingStage(Stage_Inactive, true);
                }
                m_dStartFocusingTime = GetHRTickCount();
              }
            }

            return NULL;
          }
        case Stage_EdgeToCenter:
          {
            if (!SetLightFromCenter())
              SEND_GADGET_WARN(_T("Can't set light for edge to center moving"));
            return NULL;
          }
        case Stage_DistCalib1:
          {
            OrderImagingTaskAndLight(11, CAM_OBSERVATION, OBSERVATION_LEDS, m_iObservationMeasExp_us);
            m_ProcessingStage = Stage_DistCalib2; // will be processed after 
                                                   // observation initial measurement
            return NULL;
          }
        case Stage_DistCalib3:
          {
            OrderImagingTaskAndLight(2, CAM_MEASUREMENT, MEASUREMENT_LEDS, 200);
            m_ProcessingStage = Stage_DistCalib4; // will be processed after 
                                                   // observation initial measurement
            return NULL;
          }
        case Stage_GetExtImage:
        case Stage_OneExtMeasurement:
        case Stage_GetAllLightsImage:
          {
            if (SetCamerasExposure(m_iMeasurementExposure_us,
              "Can't set Measurement Exposure", Stage_Inactive))
            {
              int iTask = (m_ProcessingStage == Stage_GetExtImage) ? 0 : -1;
              double dRealAngle;
              if (m_WorkingMode != WM_DarkEdge_NoInternal)
              {
                if (m_ProcessingStage == Stage_GetAllLightsImage)
                  SetTask(iTask);
                SetLightForExternal(m_dLastAngleToCenter, m_iExtLight_us, dRealAngle);
              }
              else
                SetLightForDarkEdge(iTask);
            };
            if (m_ProcessingStage != Stage_OneExtMeasurement)
              m_ProcessingStage = Stage_Inactive;
            return NULL;
          }
          //           case Stage_TryFocusing:
          //           case Stage_TryFocusingBack:
        case Stage_BeginToMax:
        case Stage_EndToMax:
        case Stage_OneDarkMeasBeginToMax:
        case Stage_OneDarkMeasEndToMax:
          {
            SetLightForDarkEdge(0);
            break;
          }
        case Stage_Focusing:
          SetLightForDarkEdge(-1); // nothing to measure, we need image only
          break;
        }
      }
      return NULL;
    }

    if ( ScanCommand == _T("Status"))
    {
      CTextFrame * pReport = CreateTextFrame(
        _T("Status"), (DWORD)0);

      FXString Pos = (abs(m_cRobotPos - m_cPartPlacementPos) < 500.) ?
        _T("Load Part") :
        (abs(m_cRobotPos - m_cObservCenter) < 500.) ? _T("Observation") :
        _T("Measurement");
      pReport->GetString().Format(
        _T("%s(%d)"), GetStageDescription(m_ProcessingStage), m_iNMotionSteps);
      if (m_ProcessingStage == Stage_ScanFinished)
      {
        FXString FinalInfo;
        FinalInfo.Format(_T("\nWmax=%.1f Pt#=%d WAveMax=%.f Pt#=%d"),
          m_ConturData[m_iIndexWithMaxOfMaxWidth].m_dMaxBurrWidth_um,
          m_iIndexWithMaxOfMaxWidth,
          m_ConturData[m_iIndexWithMaxWidth].m_dAveBurrWidth_um,
          m_iIndexWithMaxWidth);
        pReport->GetString() += FinalInfo;
        m_iMaxGradPtIndex = m_iIndexWithMaxOfMaxWidth;
        m_bFormMaxGradMarker = true;
      }
      PutFrame(m_pOutput, pReport);
    }
    else if (IsInactive())
    {
      // Move robot to load part position
      if (ScanCommand == _T("LoadPart"))
      {
        SetMotionAcceleration(m_iAccel_units * 10);
        Sleep(50);
        SetMotionSpeed(m_iXYMotionScanSpeed_um_sec);
        Sleep(50);
        SetZMotionSpeed(20000.);
        Sleep(50);
        if (abs(m_cRobotPos - m_cPartPlacementPos) > 30.) // deviation > 30 microns
        {
          m_bReportAfterMotion = true;
          m_StringAfterMotion = _T("Robot is on load position");
          OrderAbsMotion(m_cPartPlacementPos);
          m_ProcessingStage = Stage_Load_Part;
        }
        else
          SetProcessingStage(Stage_Inactive, true,
            _T("Robot is on load position"));
        m_DataFrom = ADF_NoData;
      }
      // Begin full measurement cycle
      else if (ScanCommand == _T("StartMeas"))
      {
        InitMeasurement();

        if ( !m_iSimulationMode )
          m_DelayedOperations.push(Stage_0_GetCurrPos);
        else
        {
          if ( InitDirectoriesForSimulation() )
          {
            if ( !m_bShuffle )
            {
              OrderNextDirForSimulation( m_SimulationDirectories[ 0 ] , true ) ;
              OrderNextDirForSimulation( m_SimulationDirectories[ 0 ] , false ) ;
              m_SimulationLogger.AddFormattedMsg( "New part started from dir %s" , m_SimulationDirectories[ 0 ] ) ;
              m_SimulationDirectories.RemoveAt( 0 ) ;
            }
            else
            {
              UINT uiIndex ;
              rand_s( &uiIndex ) ;
              uiIndex %= m_SimulationDirectories.GetCount() ;
              OrderNextDirForSimulation( m_SimulationDirectories[ ( int ) uiIndex ] , true ) ;
              OrderNextDirForSimulation( m_SimulationDirectories[ ( int ) uiIndex ] , false ) ;
              m_SimulationLogger.AddFormattedMsg( "New part started from dir %s" , m_SimulationDirectories[ ( int ) uiIndex ] ) ;
            }
            SetProcessingStage( Stage_2_MeasOnObserv , true );
            SetTask( 10 ) ;
            OrderBMPForSimulation( false );
          }
        }
      }
      // Begin full measurement cycle
      else if (ScanCommand == _T("ObservToMeas"))
      {
        CCoordinate Target = m_CurrentPos;
        Target = Target.ShiftXYZ(m_DistFromObservationToMeasurement);
        OrderAbsMotion(Target);
      }
      // start one measurement
      else if (ScanCommand == _T("Start")
        || ScanCommand == _T("OneMeas"))
      {
        FXRegistry Reg( "TheFileX\\ConturASM" ) ;
        FXString ObjectPars = Reg.GetRegiString( "Parameters" ,
          ( m_iSimulationMode ) ? "TVObjParsForSimulation" : "TVObjParsForNormalWork" ,
          ( m_iSimulationMode ) ? TVOBJ_PARS_FOR_SIMULATION : TVOBJ_PARS_FOR_WORK ) ;
        SetTVObjPars( ObjectPars ) ;

        if (ScanCommand == _T("Start"))
        {
          m_CurrentPartStatus = DecodePartDescriptors();
          LoadPartParametersFromRegistry(m_CurrentPartStatus != Part_Known);
          SetProcessingStage(Stage_BeginToMax, true);
        }

        // Robot is on observation position
        if (bIsOnObservationPlace && !m_iSimulationMode )
        {
          if (SetCamerasExposure(m_iObservationMeasExp_us,
            "Can't set Measurement Exposure", Stage_Inactive))
          {
            OrderImagingTaskAndLight(10, CAM_OBSERVATION, OBSERVATION_LEDS,
              m_iObservationMeasExp_us);
            SetProcessingStage(Stage_OneObservMeasurement, true);
          }
        }
        else
        {
          if (!m_pLastExternalContur)
          {
            if ( !m_iSimulationMode )
            {
              m_dLastAngleToCenter = arg( m_cRobotPos - m_cMeasCenter );
              m_iCameraMask = 0x0400;
            }
          }
          else
            m_dLastAngleToCenter = GetAngleToCenter();
          if (SetCamerasExposure(m_iMeasurementExposure_us,
            "Can't set Measurement Exposure", Stage_Inactive))
          {
            double dRealAngle;
            if (!IsDarkMode())
            {
              //SetLightForExternal( m_dLastAngleToCenter , m_iExtLight_us , dRealAngle ) ;
              SetLightForInternal(NormTo2PI(m_dLastAngleToCenter + M_PI),
                m_iIntLight_us, dRealAngle);
              SetProcessingStage(Stage_OneIntMeasurement, true);
            }
            else
            {
              if ( !m_iSimulationMode )
              {
                m_iDefocusToPlus = 1;
                m_bFocusDirectionChanged = false;
                m_dLastdZ = 0.;
                m_dMinZ = m_dMinZWithGood = m_CurrentPos.m_z;
                m_dMaxZ = m_dMaxZWithGood = m_CurrentPos.m_z;
                m_CurrentPos.m_theta = m_dMeasurementStartTime = GetHRTickCount();
                m_cInitialRobotPosForMeas = cmplx( m_CurrentPos.m_x , m_CurrentPos.m_y );
                m_bPrepareToEnd = false;
                m_iIndexWithMaxWidth = m_iIndexWithMaxOfMaxWidth = 0;
                m_iLastSentToUI_Index = 0;

                g_iIndexInLCTRL = 0;
                memset( g_LCTRLSentContainers , 0 , sizeof( g_LCTRLSentContainers ) );
                //m_ConturData.RemoveAll() ;
                m_DataForPosition.RemoveAll();
                m_CurrentViewMeasStatus.Reset();
                m_ViewsForPosition.RemoveAll();
                m_dDistToPt0.RemoveAll();
                SetMotionSpeed( m_iXYMotionScanSpeed_um_sec );
                Sleep( 50 );
                SetZMotionSpeed( 20000. );
                Sleep( 50 );
                SetMotionAcceleration( m_iAccel_units * 6 , 0x03 );
                Sleep( 50 );

                SetTask( 0 );
                SetLightForDarkEdge( 0 );
              }
              else
              {
                OrderBMPForSimulation( true ) ;
              }

              if (ScanCommand == _T("Start"))
              {
                m_CurrentPartStatus = DecodePartDescriptors();
                if (m_CurrentPartStatus == Part_Known)
                {
                }
                SetProcessingStage(Stage_BeginToMax, true);
              }
              else
              {
                SetProcessingStage(Stage_OneDarkMeasBeginToMax, true);
              }
            }
          }
        }
        //         else
        //         {
        //           SEND_GADGET_ERR( "No data for one measurement: do automatic edge scan" ) ;
        //         }
      }
      else if (ScanCommand == _T("DistCalib"))
      {
        double dTargetZ = m_dBaseHeight - m_dPartHeight - m_dAdapterHeight;
        double dZ = dTargetZ - m_CurrentPos.m_z;
        double dXY = abs(m_cRobotPos - m_cObservCenter);
        if (dZ > 10. || dXY > 10.)
        {
          SetMotionSpeed(m_iXYMotionScanSpeed_um_sec);
          Sleep(50);
          OrderAbsMotion(m_cObservCenter);
          OrderAbsZMotion(dTargetZ);
          m_DelayedOperations.push(Stage_DistCalib1);
        }
        else
        {
          SetTask(11);
          SetWorkingLeds(CAM_OBSERVATION, OBSERVATION_LEDS, 200);
          m_ProcessingStage = Stage_DistCalib2; // will be processed after 
                                                 // observation initial measurement
        }
        //        m_ProcessingStage = Stage_1_MoveToObserv ;
      }
      else if (ScanCommand.Find("SetCenter") >= 0)
      {
        m_cMeasCenter = m_cPartCentPos = m_cRobotPos;
      }
      else if (ScanCommand == _T("GoToMeasPlace"))
      {
        SetMotionSpeed(m_iXYMotionScanSpeed_um_sec);
        CCoordinate Target = m_ccMeasCenter;
        Target.m_z = m_dBaseHeight - m_dAdapterHeight - m_dPartHeight;
        OrderAbsMotion(Target);
        m_ProcessingStage = Stage_ManualMotion;
      }
      else if (ScanCommand == _T("GoToObservPlace"))
      {
        SetMotionSpeed(m_iXYMotionScanSpeed_um_sec);
        SetZMotionSpeed(20000.);

        CCoordinate Target(m_ccObservCenter);
        Target.m_z -= m_dPartHeight + m_dAdapterHeight;
        CCoordinate Diff = Target;
        Diff -= m_CurrentPos;
        if (Diff.cabs(Diff) > 30.)
        {
          m_bReportAfterMotion = true;
          m_StringAfterMotion = _T("Robot is on observation position");
          OrderAbsMotion(Target);
          m_DelayedOperations.push(Stage_OneObservMeasurement);
        }
        else
        {
          SetProcessingStage(Stage_Inactive, true,
            _T("Robot is on observation position"));
        }
      }
      return NULL;
    }
    else   if (IsLightCalibration())
    {
      int iRes = ProcessLightCalibration(pDataFrame);
      return NULL;
    }

    return NULL;
  }
  else   if (IsLightCalibration())
  {
    int iRes = ProcessLightCalibration(pDataFrame);
    return NULL;
  }
  else if (IsFocusing() ||
    (!pDataFrame->IsContainer()
      && (m_ProcessingStage == Stage_4_MeasFocusedOnObserv      // ZStage goes to focus position after observation
        || m_ProcessingStage == Stage_8_GetInternalBorder      // ZStage goes to focus position after
        )))                                              //  measurement initial focusing
  {
    return NULL;
  }
  if (m_ProcessingStage == Stage_WaitForMotionEnd)
  {
    if (IsEmbeddedFocus() || m_StageBeforeMotion == Stage_Focusing)
    {
      m_CheckedPos = m_CurrentPos;
      if (GetHRTickCount() - m_dLastMotionOrderTime > 2000.)
        RequestCurrentPosition();
    }
    return NULL; // nothing to measure
  }

  if (m_dStopReceivedTime != 0.)
  {
    if (!IsInactive())
      m_ProcessingStage = Stage_Inactive;

    return NULL;
  }
  const CVideoFrame * pImage = pDataFrame->GetVideoFrame();
  if (pImage)
  {
    m_cLastROICenter = cmplx( GetWidth( pImage ) / 2. , GetHeight( pImage ) / 2. );
    if ( m_iSimulationMode )
    {
      if ( m_ProcessingStage == Stage_2_MeasOnObserv )
      {
        m_CurrentPos = m_ccObservCenter ;
        m_cRobotPos = cmplx( m_CurrentPos.m_x , m_CurrentPos.m_y ) ;
      }
      else if ( m_ProcessingStage == Stage_12a_MeasDarkEdge )
      {
        FXString Label = pImage->GetLabel() ;
        cmplx cPos = DecodeCoordsForSimulation( Label ) ;
        if ( cPos.real() != 0. && cPos.imag() != 0. )
        {
          m_cRobotPos = cPos ;
          m_CurrentPos.m_x = cPos.real() ;
          m_CurrentPos.m_y = cPos.imag() ;
        }
      }
    }
  }

  const CFigureFrame * pContur = (const CFigureFrame*)GetFrameWithLabel(
    pDataFrame, figure, _T("Contur"), WP_Begin);
  if (!pContur)
  {
    pContur = (const CFigureFrame*)GetFrameWithLabel(
      pDataFrame, figure, _T("EdgeSegment"), WP_Begin);
  }
  LPCTSTR pConturLabel = (pContur) ? pContur->GetLabel() : NULL;

  if (pImage)
  {
    DeleteWatchDog();
    if (!pContur)
    {
      switch (m_ProcessingStage)
      {
      case Stage_BeginToMax:
      case Stage_EndToMax:
        if (++m_iNotMeasuredCntr < m_iNotMeasuredLimit)
        {
          FXString Msg;
          Msg.Format("\nNO Measurement cnt=%d, Resample ", m_iNotMeasuredCntr);
          TRACE("\n%s   %s", GetStageName(),
            (LPCTSTR)Msg);
          CTextFrame * pErrorInfo = CreateTextFrame(Msg, "Result");
          PutFrame(m_pOutput, pErrorInfo);
          SetLightForDarkEdge(0);
        }
        else  // too much not measured steps
        {
          FXString TextInfo;
          TextInfo.Format("Process stopped because no measurement (%d)\n",
            m_iNMotionSteps);
          CTextFrame * pErrorInfo = CTextFrame::Create((LPCTSTR)TextInfo);
          SetProcessingStage(Stage_Inactive, true, NULL, true);
          TRACE("\n%s   %s", GetStageName(),
            (LPCTSTR)TextInfo);
          PutFrame(m_pOutput, pErrorInfo);
        }
        return NULL;
      case Stage_Focusing: // check focusing state and move or stop
        if (!ContinueFocusing(pImage))
        {
          m_StageBeforeMotion = m_StageAfterFocusing;
          m_ProcessingStage = m_StageAfterFocusing;
          if (m_ProcessingStage != Stage_Inactive)
            break;
          else
          {
            SetMotionAcceleration(m_iAccel_units, 0x03); // XY
            SetProcessingStage(m_ProcessingStage, true);
            SetLightForDarkEdge(0);
            CTextFrame * pInfo = CreateTextFrame("Measurement begins",
              "Result");
            PutFrame(m_pOutput, pInfo);

          }
        }
        return NULL;
      }
    }
  }

  // Special case for distance between cameras measurement
  if (bIsOnMeasurementPlace)
    m_cMeasFOVCent = m_cLastROICenter;
  else
    m_cObservFOVCenter = m_cLastROICenter;
  if (m_iStopOnFault)
    Sleep(abs(m_iStopOnFault));
  bool bExternal = false;
  const CRectFrame * pROI = pDataFrame->GetRectFrame(DEFAULT_LABEL);
  LPCTSTR pLab = (pROI) ? pROI->GetLabel() : NULL;
  bool bIsROI = (pROI && pLab && _tcsstr(pLab, _T("ROI:")) == pLab);
  if (bIsROI)
  {
    m_LastROI = *(RECT*)pROI;

    CPoint Cent = (pROI) ? m_LastROI.CenterPoint() : CPoint(390, 290);
    m_cMeasFOVCent = cmplx((double)Cent.x, (double)Cent.y);
    cmplx cCurrentPartCentPos = m_cRobotPos - m_cPartCentPos;
    m_dLastAngleToCenter = arg(conj(cCurrentPartCentPos));
  }

  const CFigureFrame* pNewFigure = NULL;
  int iNearestIndex = -1;
  const CFigureFrame * pNearestFig = NULL;
  double dAngleToPartCentInFOV = GetAngleToCenter();
  bool bWasSegment = false;
  double dMinDistToCenter = DBL_MAX;
  int iIndexOfMinDist = -1; // not found; really this index could be zero.
  CSegmentInsideROI NearestSegm;

  CContainerFrame * pOut = GetNewOutputContainer(pDataFrame);

  // OK: there are image, contour and ROI info, 
  //     We are on measurement place and Measurement task
  //     was performed in EdgeMeas gadget
  // do extract informational segments
  if (pImage && bIsROI && pContur)
  {
    CFramesIterator * pFiguresIterator = pDataFrame->CreateFramesIterator(figure);
    int iNSettled = 0;
    if (pFiguresIterator)
    {
      iNSettled = SetDataAboutContours(pFiguresIterator); //m_dwLastFilledConturs
                                                             // will be filled there
      delete pFiguresIterator;
    }
    if (!iNSettled)
    {
      DWORD dwCont = SetDataAboutContour(pContur);
      if (dwCont)
      {
        m_dwLastFilledConturs = dwCont;
        iNSettled = 1;
      }
    }
    if (bIsOnMeasurementPlace)  // New segment extraction and checking
    {
      pOut->AddFrame(pImage); // user counter will be incremented automatically
      if (((0 <= m_iLastTask) && (m_iLastTask < 2)) || (m_iLastTask == 6)) //  
      {
        int iMinIndex = 0;
        double dCurrMinDistToCent = DBL_MAX;

        for (int i = 0; i < m_LastExtConturs.GetCount(); i++)
        {
          CFigureFrame * pSegmentEnds = NULL;
          pNewFigure = (const CFigureFrame*)(m_LastExtConturs.GetFrame(i));
          LPCTSTR pLabel = pNewFigure->GetLabel();

          if (pNewFigure)
          {
            if (_tcsstr(pLabel, "EdgeSegment") == pLabel)
            {
              NearestSegm.m_iIndexBegin = 0;
              NearestSegm.m_iIndexEnd = (int) pNewFigure->GetUpperBound();
              cmplx cFirst = CDPointToCmplx((*pNewFigure)[0]);
              cmplx cLast = CDPointToCmplx((*pNewFigure).Last());
              int FirstEdge = IsPtNearEdge(cFirst, *pROI, 2.);
              int SecondEdge = IsPtNearEdge(cLast, *pROI, 2.);

              NearestSegm.AccountFigure(pNewFigure, &m_cLastROICenter);
              double dDistBetweenEnds = abs(cLast - cFirst);
              int iRoiWdith = pROI->right - pROI->left;
              if ((dDistBetweenEnds < (iRoiWdith / 2))
                /*|| (NearestSegm.m_dSegmentLength > ( iRoiWdith * 2 ))*/)
              {
                cmplx cMotionVect = m_cLastMotion / 2.;
              }
              else
              {
                pNearestFig = NearestSegm.m_pFigure = pNewFigure;
                dMinDistToCenter = dCurrMinDistToCent = NearestSegm.m_dMinDistToCenter;
                bWasSegment = true;
              }
              break;
            }
            else
            {
              ActiveSegments * pNewSegments = GetCrossROISegments(
                *pNewFigure, m_LastROI, &iMinIndex, &dCurrMinDistToCent);
              if (pNewSegments && dCurrMinDistToCent < dMinDistToCenter)
              {
                NearestSegm = *pNewSegments->GetAt(iMinIndex);
                dMinDistToCenter = dCurrMinDistToCent;
                bWasSegment = true;
              }
              if (pNewSegments)
              {
                for (int i = 0; i < pNewSegments->GetCount(); i++)
                  delete pNewSegments->GetAt(i);

                delete pNewSegments;
              }

              if (NearestSegm.Count())
              {
                pNewFigure = pNearestFig = NearestSegm.m_pFigure;

                if (NearestSegm.m_iIndexBegin > NearestSegm.m_iIndexEnd)
                {
                  int iFigLen = (int)pNewFigure->GetCount();
                  cmplx * pRotationArray = new cmplx[iFigLen];
                  int iRotationShift = iFigLen - NearestSegm.m_iIndexBegin;
                  memcpy(pRotationArray,
                    pNewFigure->GetData() + NearestSegm.m_iIndexBegin,
                    iRotationShift * sizeof(cmplx));
                  memcpy(pRotationArray + iRotationShift,
                    pNewFigure->GetData(),
                    (NearestSegm.m_iIndexBegin) * sizeof(cmplx));
                  memcpy((void*)(pNewFigure->GetData()), pRotationArray,
                    iFigLen * sizeof(cmplx));
                  NearestSegm.m_iIndexEnd += iRotationShift;
                  NearestSegm.m_iIndexEnd %= iFigLen;
                  NearestSegm.m_iIndexBegin = 0;
                  NearestSegm.m_iMinDistToCenterIndex += iRotationShift;
                  NearestSegm.m_iMinDistToCenterIndex %= iFigLen;
                  delete[] pRotationArray;
                }

              }
            }
          }
        }
        if (NearestSegm.Count())
        {
          cmplx * pCmplx = (cmplx*)(pNewFigure->GetData());
          cmplx Pt1 = pCmplx[NearestSegm.m_iIndexBegin];
          cmplx Pt2 = pCmplx[NearestSegm.m_iIndexEnd];
          cmplx Pts[2] = { Pt1, Pt2 };

          if ( !m_iSimulationMode )
          {
            CFigureFrame * pSegmentEnds = CreateFigureFrame(
              Pts , 2 , GetHRTickCount() , _T( "0xff0000" ) );
#ifdef CHECK_IS_IN_CONTAINER
            ASSERT( !IsInContainer( pOut , pSegmentEnds ) );
#endif 
            pOut->AddFrame( pSegmentEnds ); // once
          }
          NearestSegm.m_pFigure = pNewFigure;
          FR_RELEASE_DEL(m_pLastExternalContur);
          ReplaceFrame((const CDataFrame**)&m_pLastExternalContur,
            NearestSegm.m_pFigure);
          if (m_pLastExternalContur)
          {
            LogFigure(m_pLastExternalContur->GetLabel(), Fig_Touch,
              __LINE__, m_pLastExternalContur->GetUserCnt());
          }

          m_iMinDistIndexForExternal = NearestSegm.m_iMinDistToCenterIndex;
          m_NearestExternal = m_pLastExternalContur->GetAt(m_iMinDistIndexForExternal);
        }
        else
        {
          if (m_pLastExternalContur)
          {
            LogFigure("Release LastContur", Fig_Touch,
              m_pLastExternalContur->Count(), m_pLastExternalContur->GetUserCnt());
          }
          if (NearestSegm.m_pFigure)
          {
            LogFigure("No Good Segment", Fig_Touch,
              NearestSegm.m_pFigure->Count(), NearestSegm.m_pFigure->GetUserCnt());
          }
          FR_RELEASE_DEL(NearestSegm.m_pFigure);
          FR_RELEASE_DEL(m_pLastExternalContur);
        }

        if ( !m_iSimulationMode )
        {
          CFigureFrame * pDirToCent = CreateFigureFrame( &m_VectorsViewCent , 1 ,
            GetHRTickCount() , "0x00c0c0" ); // Dark Yellow
          //dAngleToPartCentInFOV = NormTo2PI( dAngleToPartCentInFOV + M_PI ) ;
          cmplx cDirToCent = polar( 100. , -dAngleToPartCentInFOV );
          cmplx EndDirToCent = m_VectorsViewCent + cDirToCent;
          pDirToCent->AddPoint( CmplxToCDPoint( EndDirToCent ) );
          pOut->AddFrame( pDirToCent ); // once
        }
      }
      else if (m_iLastTask == 2)
      {
      }
      else
        ASSERT(m_iLastTask < 0); // measurement tasks should not be used
    }
    else if (bIsOnObservationPlace)
    {
      if (IsInactive())
      {
        pOut->AddFrame(pImage);
        CheckObservationContur(pOut, pDataFrame, false);
        if (m_iIndexOfFoundObservForm >= 0)
          pOut = NULL; // this container is already sent

        switch (m_iLastTask)
        {
        case 10: // part measurement on observation
          {
          }
          break;
        case 11:

          break;
        default:
          break;
        }
      }
    }
  }


  // Special cases for distance calibration procedures
  switch (m_ProcessingStage)
  { // Spot measurement on observation position
  case Stage_DistCalib2:
    {
      const CFigureFrame * pResult = pDataFrame->GetFigureFrame(_T("calib_observ"));
      if (pResult)
      {
        cmplx cSpotPos = CDPointToCmplx(pResult->GetAt(0)) - m_cLastROICenter;
        cmplx cSpotPos_um = cSpotPos / m_dObservScale_pix_per_um;
        m_cHoleCenterOnObservation = m_cRobotPos + cSpotPos_um;
        OrderAbsMotion(m_cMeasCenter);
        m_DelayedOperations.push(Stage_DistCalib3);
        return NULL;
      }
    }
    break;
  case Stage_DistCalib4:
    {
      const CFigureFrame * pResult = pDataFrame->GetFigureFrame(_T("calib"));
      if (pResult)
      {
        cmplx cSpotPos = CDPointToCmplx(pResult->GetAt(0)) - m_cLastROICenter;
        cmplx cSpotPos_um = cSpotPos / m_dScale_pix_per_um;
        m_cHoleCenterOnMeasurement = m_cRobotPos + cSpotPos_um;
        double dZ = m_DistFromObservationToMeasurement.m_z;
        m_DistFromObservationToMeasurement =
          m_cHoleCenterOnMeasurement - m_cHoleCenterOnObservation;
        m_DistFromObservationToMeasurement.m_z = dZ;
        SetProcessingStage(Stage_Inactive, true);
        return NULL;
      }
    }
    break;
  }

  bool bDarkMeasurement = IsDark() && m_pLastExternalContur;
  if (bDarkMeasurement && (m_iLastTask < 0)
    && pImage)  // No edge measurement result
  {
    if (!pOut)
      pOut = GetNewOutputContainer(pDataFrame);

    cmplx * pExtData = (cmplx*)(m_pLastExternalContur->GetData());
    int iFigLen = (int)m_pLastExternalContur->GetCount();

    int iFirstPtIndex = m_iMinDistIndexForExternal - m_iMeasZoneWidth + iFigLen;
    int iFirstIndexNorm = iFirstPtIndex % iFigLen;
    int iLastPtIndex = m_iMinDistIndexForExternal + m_iMeasZoneWidth + iFigLen;
    int iLastIndexNorm = iLastPtIndex % iFigLen;
    cmplx LPt1 = pExtData[iFirstIndexNorm];
    cmplx LPt2 = pExtData[iLastIndexNorm];
    CalcLightDirections(LPt1, LPt2,
      m_dLightForInternal, m_dLightForExternal,
      m_dRealLightDirForExternal, pOut);

    if (m_iStopOnFault != 0)
      Sleep(abs(m_iStopOnFault));
    CFigureFrame * pExt = CFigureFrame::Create();
    *(pExt->Attributes()) = _T("color=0x0000ff");
    CFigureFrame * pInt = CFigureFrame::Create();
    *(pInt->Attributes()) = _T("color=0x00ff00");
    CFigureFrame * pShadow = CFigureFrame::Create();
    *(pShadow->Attributes()) = _T("color=0x00ffff");
    //CFigureFrame * pDirMarking = CFigureFrame::Create() ;
    CFigureFrame * pDirMarking = NULL;
    //*(pDirMarking->Attributes()) = _T( "color=0xffff00" ) ;
    int iNGood;
    int iNIntervals = FindBurrByAvgAndStd(pImage, pExtData, iFigLen, iFirstIndexNorm,
      abs(iLastPtIndex - iFirstPtIndex) + 1,
      m_iSegmentLength, iNGood, pExt, pInt, pShadow, pDirMarking);

    if (pExt->GetCount() && pInt->GetCount())
    {
      cmplx cRealExtLightVect = polar(1., m_dRealLightDirForExternal);
      double dAngleFromLightToOrth = GetAbsAngleBtwVects(
        cRealExtLightVect, m_cNormToDirectionLight);
      double dCorrection = 1.; // no light direction
      cmplx * pExtCmplx = (cmplx*)pExt->GetData();
      cmplx * pIntCmplx = (cmplx*)pInt->GetData();
      cmplx cNearestPt = CDP2Cmplx(m_pLastExternalContur->GetAt(m_iMinDistIndexForExternal));
      double dK = dCorrection / m_dScale_pix_per_um;
      cmplx cLastNotZeroBurrPosition(-1., 0.);
      double dBurrLength_um = 0.;
      double dVolume_qum = 0.;
      m_dLastMaxWidth_um = 0.;
      m_dLastAverageWidth_um = 0.;
      int iNEmpty = 0;
      if (iNGood >= iNIntervals / 2)
      {
        for (int i = 0; i < pInt->GetCount(); i++)
        {
          double dWidth = abs(pExtCmplx[i] - pIntCmplx[i]);
          if (dWidth < m_iIn_pix - 3)
          {
            double dWidth_um = dWidth * dK;
            m_dLastAverageWidth_um += dWidth_um;
            if (m_dLastMaxWidth_um < dWidth_um)
              m_dLastMaxWidth_um = dWidth_um;
          }
        }
        m_dLastAverageWidth_um /= iNGood;
      }
      if (!IsOneMeasurement())
      {
        double dNow = GetHRTickCount();
        double dInterval = (m_dLastResultSaveTime != 0.) ?
          dNow - m_dLastResultSaveTime : 0;
        m_dLastResultSaveTime = dNow;
        if (!IsEmbeddedFocus())
        {
        }
        else // Embedded focusing
        {
        }
        ConturSample& NewSample = m_ConturData.GetLast();
        if (m_bInMeasurementFocusFound)
        {
          m_dLastContrastAfterFocusing = m_dLastContrast;
          m_dLastFocusValueAfterFocusing = m_dLastFocusValue;
          m_bInMeasurementFocusFound = false;
        }
        if (m_ConturData.GetCount() == 1)
        {
          m_iIndexWithMaxWidth = m_iIndexWithMaxOfMaxWidth = 0;
        }
        else if (NewSample.m_iBadEdge == EQ_Normal)
        {
          if (m_ConturData[m_iIndexWithMaxWidth].m_dAveBurrWidth_um
            < m_dLastAverageWidth_um)
          {
            m_iIndexWithMaxWidth = (int)m_ConturData.GetUpperBound();
          }
          if (m_ConturData[m_iIndexWithMaxOfMaxWidth].m_dMaxBurrWidth_um
            < m_dLastScanMaxWidth_um)
          {
            m_iIndexWithMaxOfMaxWidth = (int)m_ConturData.GetUpperBound();
          }
        }
        CVideoFrame* pImageCopy = (CVideoFrame*)pImage->Copy();

        FXString Label;
        Label.Format(_T("%sPt%d_X%d_Y%d_Z%d"),
          (LPCTSTR)m_ResultDirectory,
          NewSample.m_iIndex,
          ROUND(m_CurrentPos.m_x),
          ROUND(m_CurrentPos.m_y),
          ROUND(m_CurrentPos.m_z));
        pImageCopy->SetLabel(Label);
        //         pImageCopy->AddRef() ; // this frame is going to two places
        //         m_LastScanFrames.AddFrame( pImageCopy ) ;
        PutFrame(m_pImageSave, pImageCopy);
      }
    }
    else
    {
      ASSERT(0);
    }
    pOut->AddFrame(pExt);
    pOut->AddFrame(pInt);
    pOut->AddFrame(pShadow);

    if (m_pDebugging)
    {
      pOut->AddFrame(m_pDebugging);
      m_pDebugging = NULL;
    }
    pExt = pInt = pShadow = NULL;
    //            pOut->AddFrame( pDirMarking ) ;

  }
  else
  {
    // Check for figures presence in results
    if (!pContur)
    {
      if (pOut)
        pOut->Release();

      if ( m_iSimulationMode ) // contur is not found, switch to next part 
        OrderNextPartForSimulation( 0 ) ; // FAILED
      SENDERR( "Measurement Failed" );

      return NULL;
    }

    bool bExitWithNull = false;
    // OK, there are figures for Observation processing
    switch (m_ProcessingStage)
    {
    case Stage_2_MeasOnObserv: // found one or two contours on observation
      {
        bExitWithNull = true;
        pOut->AddFrame(pImage);
        int iPartConturFound = CheckObservationContur(
          pOut, pDataFrame, true);

        if (iPartConturFound)
        {
          //ys_20201002 What is it? FXString KnownName = CheckCreateDataSubDir( _T( "ObservationImages" ) );
          //if ( !KnownName.IsEmpty() )
          if (!m_ResultDirectory.IsEmpty() && !m_iSimulationMode )
          {
            //ys_20201002 FXString ObservationName = KnownName + m_PartFile + GetTimeAsString() + _T(".bmp") ;
            FXString ObservationName = m_ResultDirectory + m_PartFile + _T("_") + m_MeasurementTS + _T("_ObservationImage.bmp");
            bool bRes = saveSH2BMP(ObservationName, pImage->lpBMIH);
          }

          CRect ScalingFragment;
          GetScalingFragment(m_ObservationExtremes, 10, __out ScalingFragment);
          if ( !m_iSimulationMode )
          {
            //Save as image of this measurement
            FXString FileName = m_ResultDirectory
              + ( m_PartFile + _T( '_' ) )
              + m_MeasurementTS + _T( "_PartView.bmp" );
            SaveImageFragment( pImage , ScalingFragment , FileName );
          }
          //           if (m_PartCatalogName.IsEmpty())
          m_PartCatalogName = m_PartFile.Mid(m_PartFile.Find('_') + 1);

          //           if ( m_iIndexOfFoundObservForm < 0 )
          //           {
          ReplaceFrame((const CDataFrame**)&m_pObservationImage, pImage);
          //           }
          cmplx cLeftPt = m_ObservationExtremes[ EXTREME_INDEX_LEFT ]; // Xmin
          if ( m_iSimulationMode )
          {
            Sleep( 2000 ) ;
            m_iLastSimulationPt = -1 ;
            m_bScanFinished = false ;
            SetTask( 0 ) ;
            OrderBMPForSimulation( true ) ;
            SetProcessingStage( Stage_12a_MeasDarkEdge ) ;
          }
          else
          {
          // Calc target for motion into measurement zone
            CCoordinate Target = GetTargetForObservationPoint( cLeftPt );
            cmplx DistToPartCenter = -GetDistanceOnObservation( cLeftPt , m_cPartCenterOnObservation );

            cmplx cVectFromFOVCentToPartCent = m_cPartCenterOnObservation
              - m_cObservFOVCenter;
            cmplx cScaledPartCent = cVectFromFOVCentToPartCent / m_dObservScale_pix_per_um;
            m_cPartCenterOnMeas = ( cmplx ) Target + conj( DistToPartCenter );
            Target.m_x += 70.; // external edge should be nearest to FOV center
                                // Internal contours will be on bigger distance
            m_cInitialRobotPosForMeas = ( cmplx ) Target;
            m_cInitialVectToPartCent = DistToPartCenter;
            m_cRobotPosToInitialPt = cmplx( 0. , 0. );
            cmplx cCurPos = m_cRobotPos;
            OrderAbsMotion( Target );
            m_DelayedOperations.push( Stage_5_SendToEdgeMeasurement );
            SetCamerasExposure( m_iMeasurementExposure_us ,
              "Can't set measurement exposure" , Stage_Inactive );
            double dDistX = abs( Target.m_x - cCurPos.real() );
            double dDistY = abs( Target.m_y - cCurPos.imag() );
            double dDist = max( dDistX , dDistY );
            double dTime = dDist * 2000. / m_iXYMotionScanSpeed_um_sec; // 
            SetWatchDog( ROUND( dTime + 3000. ) );
          }
        }
        else if ( !m_iSimulationMode )
        {
          SEND_GADGET_ERR("Can't find observation conturs. %s",
            GetStageName());
          SetProcessingStage(Stage_Inactive, false, NULL, true);

          FXString UnknownName = CheckCreateDataDir() + _T("Unidentified\\");
          if (!m_NewPartName.IsEmpty())
          {
            UnknownName += _T("AsNew_");
            UnknownName += m_NewPartName + _T('_');
          }
          else
            UnknownName += _T("Unknown_");
          UnknownName += GetTimeAsString() + _T(".bmp");
          bool bRes = saveSH2BMP(UnknownName, pImage->lpBMIH);
        }
      }
      break;// end of processing stage 2
    case Stage_4_MeasFocusedOnObserv: // Order measurement of
                             //observation contours after focusing
      {
        OrderImagingTaskAndLight(10, CAM_OBSERVATION, OBSERVATION_LEDS,
          m_iObservationMeasExp_us);
        // will be processed after
        // observation initial measurement
        SetProcessingStage(Stage_5_SendToEdgeMeasurement, true);
      }
      bExitWithNull = true;
      break;// end of processing stage 4
             // Measure observation contours after focusing
    case Stage_5_SendToEdgeMeasurement:
      {
        if (abs(m_cCurrentTarget - m_cRobotPos) < 10.)
        {
          //          SetLightForDarkEdge(0);
          SetLightForDarkEdge(6);
          SetProcessingStage(Stage_9_MoveEdgeToCenter, true);
          bExitWithNull = true;
          CTextFrame * pEndOfScan = CreateTextFrame(
            "Arrived to measurement (next stage 9)", "Result");
          PutFrame(m_pOutput, pEndOfScan);
        }
      }
      break;
    case Stage_7_AutoFocusEdge:
      {
        m_sLastCmndForFocusingLightWithExt.Format(_T("out 0x8ff %d\r\n"),
          m_iMeasurementExposure_us);
        SetFocusingArea((int)m_cMeasFOVCent.real() + 40, (int)m_cMeasFOVCent.imag(),
          m_iFocusAreaSize_pix, m_iFocusAreaSize_pix, m_iFocusWorkingMode);
        if (!StartFocusing(
          -m_iMeasInitialFocusRange_um, m_iMeasInitialFocusRange_um,
          (LPCTSTR)m_sLastCmndForFocusingLightWithExt,
          700, m_iMeasurementExposure_us))
        {
          SEND_GADGET_ERR("Can't send focusing command");
          SetProcessingStage(Stage_Inactive, true);
        }
        else
        {
          SetProcessingStage(Stage_8_GetInternalBorder, true);
        }
        bExitWithNull = true;
        break;// end of processing stage 7
      }
      break;
    default:
      break;
    }
    if (bExitWithNull)
    {
      if (m_iStopOnFault != 0)
        Sleep(abs(m_iStopOnFault));
      return NULL;
    }
  }

  if (!pOut)
    pOut = GetNewOutputContainer(pDataFrame);

  if (m_iStopOnFault != 0)
    Sleep(abs(m_iStopOnFault));


  // Figures are:
  //  "Contur" - figure for edge inspection

  if (!NearestSegm.Count() && (m_iLastTask >= 0) && !IsDark()) // no segments for 12a
  {                                            // Old external contur will be used
    if (bWasSegment)
    {
      cmplx MsgPos(20, 100);
      CTextFrame * pMsgAboutDirty = CreateTextFrame(MsgPos, _T("Dirty"), _T("0x0000ff"));
      pOut->AddFrame(pMsgAboutDirty); // once
    }
    if (m_iStopOnFault != 0)
      Sleep(abs(m_iStopOnFault));
    ASSERT(m_iStopOnFault <= 0);


    //     if ( pOut->GetFramesCount() )
    //       PutAndRegisterFrame( m_pMarking , pOut );
    ProcessingStage OldProcessingStage = m_ProcessingStage;
    bool bFront = false;
    switch (m_ProcessingStage)
    {
    case Stage_EdgeToCenter:
      ASSERT(0);
      break;
    case Stage_11_GetExternalBorder:
    case Stage_GetInternalBorder:
      {
        m_cMeasImageCent = m_cLastROICenter;
        if (m_ProcessingStage == Stage_GetInternalBorder)
          SetProcessingStage(Stage_GetExternalBorder, true);
        else
          SetProcessingStage(Stage_12_MeasWalls, true);

        SetLightToCenter();

        if (m_pLastInternalContur)
        {
          ((CFigureFrame*)m_pLastInternalContur)->Release();
          m_pLastInternalContur = NULL;
        }
        cmplx MsgPos(20, 150);
        CTextFrame * pMsgAboutFault = CreateTextFrame(MsgPos,
          _T("Internal fault, switched to Ext"), _T("0x0000ff"));
        pOut->AddFrame(pMsgAboutFault); // once
      }
      break;
    case Stage_12_MeasWalls:
    case Stage_GetExternalBorder:
    case Stage_12a_MeasDarkEdge:
      FR_RELEASE_DEL(m_pLastExternalContur);
    case Stage_OneDarkMeasBeginToMax:
      m_cMeasImageCent = m_cLastROICenter;
      if (++m_iNotMeasuredCntr < m_iNotMeasuredLimit)
      {

        m_cLastMotion = m_cLastNotSmallMotion;
        m_cTraveling += cmplx(fabs(m_cLastMotion.real()), fabs(m_cLastMotion.imag()));
        m_dTraveling += abs(m_cLastMotion);
        m_cRobotPos += m_cLastNotSmallMotion;
        ASSERT(m_MotionMask == 0 && m_iNWaitsForMotionAnswers == 0);

        FXString Msg;
        Msg.Format("\nNOMeas: C(%d,%d) Shift(%d,%d)",
          (int)m_cMeasFOVCent.real(), (int)m_cMeasFOVCent.imag(),
          (int)m_cLastMotion.real(), (int)m_cLastMotion.imag());
        TRACE("\n%s   %s", GetStageName(),
          (LPCTSTR)Msg);

        cmplx MsgPos(20, 150);
        CTextFrame * pMsgAboutFault = CreateTextFrame(MsgPos,
          (LPCTSTR)Msg, _T("0x0000ff"));
        pOut->AddFrame(pMsgAboutFault); // once
      }
      else  // too much not measured steps
      {
        FXString TextInfo;
        TextInfo.Format("Too many errors on measurements (%d)\n"
          "Process is stopped\n after (%7.1f,%7.1f) traveling",
          m_iNMotionSteps,
          m_cTraveling.real(), m_cTraveling.imag());
        CTextFrame * pTextInfo = CTextFrame::Create((LPCTSTR)TextInfo);
        pTextInfo->Attributes()->Format("color=0xffff00; Sz=20; x=%d; y=%d;",
          20, 180);
        pOut->AddFrame(pTextInfo); // once
        SetProcessingStage(Stage_Inactive, true);
        TRACE("\n%s   %s", GetStageName(),
          (LPCTSTR)TextInfo);
      }
      break;
    }
    if (pOut && !IsInactive())
    {
      FXString ContList, Prefix;
      int iLevel = 0;
      FXPtrArray FramePtrs;
      int iNFrames = FormFrameTextView(
        pOut, ContList, Prefix, iLevel, FramePtrs);
      PutAndRegisterFrame(m_pMarking, pOut);
    }

    if (m_iStopOnFault != 0)
      Sleep(abs(m_iStopOnFault));
    ASSERT(m_iStopOnFault <= 0);
    if (OldProcessingStage == Stage_GetInternalBorder
      || OldProcessingStage == Stage_11_GetExternalBorder)
    {
      if (!IsDarkMode())
        SetLightToCenter();
      else
        SetLightForDarkEdge(0);
    }
    else if (m_ProcessingStage != Stage_Inactive
      && !IsOneMeasurement() && !IsDarkWithEmbFocus())
    {
      if (m_iNotMeasuredCntr < m_iNotMeasuredLimit)
      {
        CCoordinate Vect(m_cLastMotion.real(), m_cLastMotion.imag());
        OrderRelMotion(Vect);
      }
    }
    if (IsOneMeasurement())
    {
      SetProcessingStage(Stage_Inactive);
    }
    if (m_iStopOnFault != 0)
      Sleep(abs(m_iStopOnFault));
    return NULL;
  }

  if (pImage && (NearestSegm.Count() || (m_iLastTask < 0)))
  {
    if ( (m_iLastTask >= 0) && NearestSegm.Count() && !m_iSimulationMode )
    {
      CFigureFrame * pDirToCent = CreateFigureFrame(&m_VectorsViewCent, 1,
        GetHRTickCount(), "0x00c0c0", "DirToCent"); // Dark Yellow
      //dAngleToPartCentInFOV = NormTo2PI( dAngleToPartCentInFOV + M_PI ) ;
      cmplx cDirToCent = polar(100., -dAngleToPartCentInFOV);
      cmplx EndDirToCent = m_VectorsViewCent + cDirToCent;
      pDirToCent->AddPoint(CmplxToCDPoint(EndDirToCent));
      pOut->AddFrame(pDirToCent); // once
    }

    switch (m_ProcessingStage)
    {
    case Stage_GetExtImage:
      SetProcessingStage(Stage_Inactive, true);
      break;
    case Stage_9_MoveEdgeToCenter: // Edge to center for full process
    case Stage_EdgeToCenter:
      {
        SetMotionAcceleration(m_iAccel_units, 0x03);

        m_cMeasImageCent = m_cLastROICenter;
        cmplx * pSegmData = (cmplx*)(m_pLastExternalContur->GetData());
        cmplx cClosestToCent = GetRelElem(pSegmData,
          NearestSegm.m_iMinDistToCenterIndex, 0,
          m_pLastExternalContur->GetCount());
        m_cLastMotion = (m_cMeasFOVCent - cClosestToCent) / m_dScale_pix_per_um;
        m_MotionMask = m_iNWaitsForMotionAnswers = 0;
        CCoordinate Vect(m_cLastMotion.real(), m_cLastMotion.imag());
        m_cPartCenterOnMeas += Vect;
        m_cInitialMeasPos = (m_cInitialRobotPosForMeas += Vect);
        m_cInitialVectToPartCent = m_cPartCenterOnMeas - m_cInitialMeasPos;
        m_cRobotPosToInitialPt = cmplx(0., 0.);
        m_dScanInitialAngle = NormTo2PI(GetAngleFromCenter());
        m_bPrepareToEnd = false;
        if (m_dInitialAngle > M_2PI)
          m_dInitialAngle = GetAngleFromCenter();
        if (abs(m_cLastMotion) < 2.0)
        {
          m_MotionMask = m_iNWaitsForMotionAnswers = 0;
          if (m_ProcessingStage == Stage_EdgeToCenter)
          {
            SetProcessingStage(Stage_GetInternalBorder, true);
            SetLightFromCenter();
          }
          else
          {
            if (m_WorkingMode == WM_DarkEdge_NoInternal)
            {
              InitIterativeFocusing(Stage_BeginToMax);
              CTextFrame * pInfo = CreateTextFrame("Initial Focusing Begins",
                "Result");
              PutFrame(m_pOutput, pInfo);

              //               SetMotionAcceleration( m_iAccel_units , 0x03 ) ; // XY
              //               SetProcessingStage( Stage_BeginToMax , true ) ;
              //               SetLightForDarkEdge( 0 ) ;
              //               CTextFrame * pInfo = CreateTextFrame( "Measurement begins" ,
              //                 "Result" ) ;
              //               PutFrame( m_pOutput , pInfo ) ;
                            //               SetProcessingStage( Stage_12_MeasWalls , true ) ;
                            //               SetLightToCenter() ;
            }
            else
            {
              ASSERT(0);
              SetProcessingStage(Stage_11_GetExternalBorder, true);
              SetLightFromCenter();
            }
          }
        }
        else
        {
          OrderRelMotion(Vect);
          InitIterativeFocusing(Stage_BeginToMax);
          m_DelayedOperations.push(Stage_Focusing);

          //m_DelayedOperations.push( Stage_BeginToMax ) ;
          CTextFrame * pInfo = CreateTextFrame("Centering Ordered",
            "Result");
          PutFrame(m_pOutput, pInfo);
          // order light after motion
//           m_DelayedOperations.push( Stage_10_GetIntOrExtBorder ); 
        }
      }
      break;
    case Stage_11_GetExternalBorder:      // Image with internal border arrived
    case Stage_GetInternalBorder:
    case Stage_OneIntMeasurement:  // One point measurement series
      {
        m_cMeasImageCent = m_cLastROICenter;
        if (m_cRobotPos.real() == 0. || m_cRobotPos.imag() == 0.)
        {  // no info about coords
            // wait for obtaining
          //SetWorkingLed( m_iDir , false );
          SetLightFromCenter();
          break;
        }
        const cmplx * pCmplx = (cmplx*)(pNearestFig->GetData());
        int iFigLen = (int)pNearestFig->GetCount();
        int iMinIndex = iIndexOfMinDist;

        CFigureFrame * pMark = CFigureFrame::Create();
        pMark->Add(pNearestFig->GetAt(iMinIndex));
        pMark->Attributes()->WriteString("color", "0x0000ff");

        pOut->AddFrame(pMark); // once
        m_NearestInternal = pNearestFig->GetAt(iIndexOfMinDist);

        ReplaceFrame((const CDataFrame**)&m_pLastInternalContur,
          pNearestFig);
        m_LastInternalConturRobotPos = m_CurrentPos;
        m_iMinDistIndexForInternal = iIndexOfMinDist;
        CFigureFrame * pEdgeMark = CFigureFrame::Create();
        for (int i = iMinIndex - 50; i <= iMinIndex + 50; i++)
        {
          const CDPoint& Pt = pNearestFig->GetAt((i + iFigLen) % iFigLen);
          pEdgeMark->Add(Pt);
        }
        pEdgeMark->Attributes()->WriteString("color", "0x00ff00");
        pOut->AddFrame(pEdgeMark); // once

        cmplx LPt1 = pCmplx[NearestSegm.m_iIndexBegin];
        cmplx LPt2 = pCmplx[NearestSegm.m_iIndexEnd];
        double dPreviousLightForInternal = m_dLightForInternal;
        double dPreviousLightForExternal = m_dLightForExternal;
        Directions iDir = (Directions)CalcLightDirections(LPt1, LPt2,
          m_dLightForInternal, m_dLightForExternal,
          m_dRealLightDirForExternal, pOut);
        if (m_dInitialAngle > M_2PI)
          m_dInitialAngle = GetAngleFromCenter();

        //CFigureFrame * pDirToCent = CreateFigureFrame( &m_VectorsViewCent , 1 ,
        //  GetHRTickCount() , "0x00c0c0" ) ;
        ////dAngleToPartCentInFOV = NormTo2PI( dAngleToPartCentInFOV + M_PI ) ;
        //cmplx cDirToCent = polar( 100. , -dAngleToPartCentInFOV ) ;
        //cmplx EndDirToCent = m_VectorsViewCent + cDirToCent ;
        //pDirToCent->AddPoint( CmplxToCDPoint( EndDirToCent ) ) ;
        //pOut->AddFrame( pDirToCent ) ;

        double dAbsDelta = AbsDeltaAngle(m_dLastLightAngle, m_dLightForInternal);

        FXString TextInfo;
        // Check for proper light direction
        if (dAbsDelta > LED_ANGLE_STEP * 1.1) //Is it necessary to 
                                                // change Change Light direction?
        {
          // Yes
          SetLightForInternal(m_dLightForInternal,
            m_iIntLight_us, m_dLastLightAngle);

          TextInfo.Format("Motion(%d,%d)\nPos(%8.2f,%8.2f)\ndAng=%5.1f %s<=%s\n"
            "Pt1(%5.1f,%5.1f)\nPt2(%5.1f,%5.1f) ",
            ROUND(m_cLastMotion.real()), ROUND(m_cLastMotion.imag()),
            ROUND(m_CurrentPos.m_x), ROUND(m_CurrentPos.m_y),
            RadToDeg(dAbsDelta),
            GetDirName(m_iPrevDir), GetDirName(m_iDir),
            LPt1.real(), LPt1.imag(), LPt2.real(), LPt2.imag());
          CTextFrame * pTextInfo = CTextFrame::Create((LPCTSTR)TextInfo);
          pTextInfo->Attributes()->Format("color=0x800080; Sz=16; x=%d; y=%d;",
            ROUND(m_NearestInternal.x) + 20, ROUND(m_NearestInternal.y - 60));
          pOut->AddFrame(pTextInfo); // once
          int iDiff = abs((m_iPrevDir - iDir) % 8);
          ASSERT((iDiff <= 1) || (iDiff == 7));
          m_iPrevDir = m_iDir;
          m_iDir = iDir;
          TRACE("\n%s   IntLight dAng=%5.1f Ldir=%5.1f(%5.1f) Pt1(%.1f,%.1f)\nPt2(%.1f,%.1f) ",
            GetStageName(),
            RadToDeg(dAbsDelta), RadToDeg(m_dLightForInternal),
            RadToDeg(m_dLastLightAngle),
            LPt1.real(), LPt1.imag(), LPt2.real(), LPt2.imag());
          break;
          // m_ProcessingStage is still Stage_GetInternalBorder or Stage_11_GetExternalBorder
        }
        else
        {
          // OK, it was correct light direction
          m_iPrevDir = m_iDir;
          m_iDir = (Directions)iDir;
          // Order imaging for with external light
          //if ( m_WorkingMode != WM_DarkEdge )
          //{
          SetLightForExternal(m_dLightForExternal,
            m_iExtLight_us, m_dLastLightAngle);
          if (m_ProcessingStage == Stage_11_GetExternalBorder)
            SetProcessingStage(Stage_12_MeasWalls, true);
          else if (m_ProcessingStage == Stage_GetInternalBorder)
            SetProcessingStage(Stage_GetExternalBorder, true);
          else
            SetProcessingStage(Stage_OneExtMeasurement, true);

          //}
          //else
          //{
          //  SetLightForDarkEdge( 0 ) ;
          //}

          m_dLastEdgeAngle = arg(conj(LPt2 - LPt1));
          TextInfo.Format("Measure Ext.\nPos(%8.2f,%8.2f)\ndAng=%5.1f %s %s\n"
            "PPt(%5.1f,%5.1f)\nMPt(%5.1f,%5.1f) ",
            m_CurrentPos.m_x, m_CurrentPos.m_y,
            RadToDeg(m_dLastEdgeAngle), (m_bPointsSwapped) ? "Swap" : "No Swap",
            GetDirName(m_iDir),
            LPt1.real(), LPt1.imag(), LPt2.real(), LPt2.imag());
          CTextFrame * pTextInfo = CTextFrame::Create((LPCTSTR)TextInfo);
          pTextInfo->Attributes()->Format("color=0x00ff80; Sz=16; x=%d; y=%d;",
            ROUND(m_NearestInternal.x) + 20, ROUND(m_NearestInternal.y - 60));
          pOut->AddFrame(pTextInfo); // once
          m_iDir = iDir;
          // for next step after external contour measurement and robot move
          TRACE("\n%s   IntLight(out) Ang=%5.1f Ldir=%.1f(%.1f) Pt1(%.1f,%.1f)\nPt2(%.1f,%.1f) ",
            GetStageName(),
            RadToDeg(m_dLastEdgeAngle), RadToDeg(m_dLightForInternal),
            RadToDeg(dPreviousLightForInternal),
            LPt1.real(), LPt1.imag(), LPt2.real(), LPt2.imag());

        }
      }
      break;
    case Stage_12_MeasWalls:     // Image with external or DARK light arrived
    case Stage_12a_MeasDarkEdge:
    case Stage_GetExternalBorder:
    case Stage_OneExtMeasurement:
    case Stage_BeginToMax:
    case Stage_EndToMax:
    case Stage_OneDarkMeasBeginToMax:
    case Stage_OneDarkMeasEndToMax:
      {
        m_cMeasImageCent = m_cLastROICenter;
        double dAngleFromIntToExt = GetCurrentInternalLightAngle(m_iDir, true);
        double dMinDist = DBL_MAX;
        cmplx dIntCent(m_NearestInternal.x, m_NearestInternal.y);

        if (m_iLastTask < 0)
        {

        }
        else if (!m_pLastExternalContur) // No, there is no visible external edge
                                           // error processing
        {
          if (m_iStopOnFault < 0)
          {
            Sleep(-m_iStopOnFault);
          }
          if (!IsOneMeasurement()
            && (++m_iNotMeasuredCntr < m_iNotMeasuredLimit))
          {
            m_cLastMotion = m_cLastNotSmallMotion;
            m_cTraveling += cmplx(fabs(m_cLastMotion.real()),
              fabs(m_cLastMotion.imag()));
            m_dTraveling += abs(m_cLastMotion);
            m_cRobotPos += m_cLastNotSmallMotion;
            ASSERT(m_MotionMask == 0 && m_iNWaitsForMotionAnswers == 0);

            CCoordinate Vect(m_cLastMotion.real(), m_cLastMotion.imag());
            OrderRelMotion(Vect);

            TRACE("\nNOMeas: C(%d,%d) Shift(%d,%d)",
              (int)m_cMeasFOVCent.real(), (int)m_cMeasFOVCent.imag(),
              (int)m_cLastMotion.real(), (int)m_cLastMotion.imag());
          }
          else  // too much not measured steps
          {
            FXString TextInfo;
            TextInfo.Format("Too many errors on measurements (%d)\n"
              "Process is stopped\n after (%7.1f,%7.1f) traveling",
              m_iNMotionSteps,
              m_cTraveling.real(), m_cTraveling.imag());
            CTextFrame * pTextInfo = CTextFrame::Create((LPCTSTR)TextInfo);
            pTextInfo->Attributes()->Format("color=0xffff00; Sz=20; x=%d; y=%d;",
              20, 40);
            pOut->AddFrame(pTextInfo); // once
            m_ProcessingStage = Stage_Inactive;
            TRACE("\n%s", (LPCTSTR)TextInfo);
          }
          break; // end of error processing
        }

        cmplx * pExtData = (cmplx*)(m_pLastExternalContur->GetData());
        int iFigLen = (int)m_pLastExternalContur->GetCount();
        cmplx cNearestPt = pExtData[m_iMinDistIndexForExternal];
        FXString ForReporting;
        if (m_iLastTask >= 0) // we did measure contur
        {
          cmplx LPt1, LPt2;
          //          int iFirstPtIndex = m_iMinDistIndexForExternal ;
                    // Full segment measurement 20.06.20
          int iFirstPtIndex = 0/* m_iMinDistIndexForExternal */;
          int iLastPtIndex = iFigLen - 1;
          LPt1 = pExtData[iFirstPtIndex];
          LPt2 = pExtData[iLastPtIndex];
          CalcLightDirections(LPt1, LPt2,
            m_dLightForInternal, m_dLightForExternal,
            m_dRealLightDirForExternal, pOut);

          if (m_iStopOnFault != 0)
            Sleep(abs(m_iStopOnFault));

          CFigureFrame * pExt = NULL;
          CFigureFrame * pInt = NULL;
          CFigureFrame * pAdditional = NULL;
          CFigureFrame * pDirMarking = NULL;
          if ( IsDarkWithEmbFocus() || IsFinalMeasurement() || m_iSimulationMode )
          {
            pExt = CFigureFrame::Create();
            *(pExt->Attributes()) = _T("color=0x00ff00");
            pExt->SetLabel("EdgePoints");
            pInt = CFigureFrame::Create();
            *(pInt->Attributes()) = _T("color=0x0000ff");
            pInt->SetLabel("BurrEndPts");
            pAdditional = CFigureFrame::Create();
            *(pAdditional->Attributes()) = _T("color=0x00ffff");
            pAdditional->SetLabel("AddGraphics");
          }

          int iNIntervals = 0;
          int iNGood;
          switch (m_ProcessingStage)
          {
          case Stage_BeginToMax:
          case Stage_EndToMax:
          case Stage_OneDarkMeasBeginToMax:
          case Stage_OneDarkMeasEndToMax:
            {
              if (m_iStopOnFault != 0)
                Sleep(abs(m_iStopOnFault));

              if (m_DataForPosition.GetCount() == 0)
                m_dMinZ = m_dMaxZ = m_CurrentPos.m_z;

              // find not good points
              int iNBads = MarkPointsWithBigDeviation(*m_pLastExternalContur,
                2., m_Bads);

              if (iNBads && pOut)
              {
                for (int i = 0; i < m_pLastExternalContur->Count(); i++)
                {
                  if (m_Bads[i])
                  {
                    cmplx Pt = CDPtToCmplx(m_pLastExternalContur->GetAt(i));
                    CFigureFrame * pBadPoint = CreateFigureFrame(&Pt, 1, (DWORD)0x0000ff);
                    pBadPoint->Attributes()->WriteInt("thickness", 3);
                    pOut->AddFrame(pBadPoint); // once

                  }
                }
              }

              if (abs(cNearestPt - m_cMeasFOVCent) < 50.
                && m_DataForPosition.Count() <= 50)
              { //OK found edge passes near center
                // OK, this is main processing for process with local focusing 
                // Find points inside ROI
                int iIndexOnEdge = iLastPtIndex;
                while (!IsPointInRect(m_LastROI, pExtData[iIndexOnEdge])
                  && (iFirstPtIndex < --iIndexOnEdge));

                FXRegistry Reg("TheFileX\\ConturASM");
                m_dNeightboursThicknessRatioThres = Reg.GetRegiDouble(
                  "Data", "NeightbourThicknessRatioThres", 0.2);
                m_dMinAmplAfterEdge = Reg.GetRegiDouble(
                  "Data", "dMinAmplAfterEdge", 15);

                iNIntervals = FindFocusAndBurrByAvg(
                  pImage, pExtData, iFigLen, iFirstPtIndex,
                  iIndexOnEdge - iFirstPtIndex + 1,
                  m_iSegmentLength, iNGood, pExt, pInt, pAdditional, pOut);
                //               if ( m_pDebugging )
                //               {
                //                 pOut->AddFrame( m_pDebugging ) ;
                //                 m_pDebugging = NULL ;
                //               }


                if (iNIntervals) // This checking is placed two times
                {                  // In the middle we do current image with graphics saving
                  pOut->AddFrame(pExt);
                  pOut->AddFrame(pInt);
                  if (pAdditional)
                  {
                    if (pAdditional->GetCount())
                      pOut->AddFrame(pAdditional);
                    else
                      pAdditional->Release();
                  }
                  //pOut->AddFrame( pDirMarking ) ;
                  pExt = pInt = pAdditional = pDirMarking = NULL;
                  if (m_cResMiddlePoint != 0.) // this is for image centering in FRender
                  {
                    CTextFrame * pMiddlePoint = CreateTextFrame("SetCenter", pDataFrame->GetId());
                    pMiddlePoint->GetString().Format("Xc=%d;Yc=%d;",
                      ROUND(m_cResMiddlePoint.real()), ROUND(m_cResMiddlePoint.imag()));
                    pOut->AddFrame(pMiddlePoint); // once
                  }
                }
                if (pOut) // there is image in container now, not necessary to add
                {           // Moisey 02.03.20
                  pOut->AddRef();
                  m_ViewsForPosition.AddFrame(pOut);
                  if (!IsOneDarkMeasurement())
                    ASSERT(m_ViewsForPosition.GetCount() == m_DataForPosition.GetCount());
                }
                if (iNIntervals)
                {
                  MeasuredValues& LastResult = m_DataForPosition.GetLast();
                  LastResult.SetAvValues();

                  if (m_CurrentViewMeasStatus.m_iNWithGoodFocus = 0)
                    m_dMaxZWithGood = m_dMinZWithGood = m_CurrentPos.m_z;

                  double dRawZStep = GetFocusingStepForFarFromFocus();
                  if (dRawZStep != 0.) // we are far from focus, do move and remeasure
                  {
                    if (m_DataForPosition.GetCount() < 25)
                    {
                      SetMinMax(m_CurrentPos.m_z, m_dMinZ, m_dMaxZ);
                      m_dMaxZWithGood = m_dMinZWithGood = m_CurrentPos.m_z;
#ifdef CHECK_IS_IN_CONTAINER
                      ASSERT(!IsInContainer(pOut, pROI));
#endif 
                      pOut->AddFrame(pROI);
                      if (NearestSegm.Count() && NearestSegm.m_pFigure)
                      {
#ifdef CHECK_IS_IN_CONTAINER
                        ASSERT(!IsInContainer(pOut, NearestSegm.m_pFigure));
#endif 
                        pOut->AddFrame(NearestSegm.m_pFigure);
                      }
                      FocusLogAndMoveZ(
                        _T("Continue Raw Focusing"), dRawZStep, (CDataFrame**)&pOut);
                      return NULL;
                    }
                    m_bFocusingAfterPosFinishing = true; // too long focus process
                  }

                  // OK, we did measure somethings
                  MeasuredValues& Vals = m_DataForPosition.GetLast();

                  //ASSERT( Vals.m_Results[ 0 ].m_dAverFocus ) ;
                  double dMinFoc, dMaxFoc, dAverAverFoc;
                  int iMaxWidthPos;
                  Vals.GetAvForGoodValues(
                    m_dLastAverageWidth_um, m_dLastMaxWidth_um, iMaxWidthPos,
                    dMinFoc, dMaxFoc, dAverAverFoc, m_dMinimalLocalFocus);
                  m_iLastMinAverageDefocusing = ROUND(dMinFoc);
                  m_iLastMaxAverageDefocusing = ROUND(dMaxFoc);

                  FocusState FState = FS_ERROR;
                  int iLastGoodIndex = 0;
                  //                 if ( IsDarkWithEmbFocus() )
                  //                 {
                  int iFirstIndex = (IsBeginToMax()) ? 0 : m_iLastGoodMeasured;
                  double dNextStep = 0.;
                  m_LastFocusState = FState = CheckPtForFocusAndGetDirection(
                    dNextStep, iFirstIndex, iLastGoodIndex);
                  m_iLastGoodMeasured = iLastGoodIndex;
                  bool bOK = false;
                  if (m_DataForPosition.GetCount() > 25)
                  {  // too many focusing attempts, go to next position
                    m_bFocusingAfterPosFinishing = true;
                    m_LastFocusState = FState = FS_OK;
                  }
                  switch (FState)
                  {
                  case FS_OK:
                    {
                      bOK = FocusLogAndMoveZ(_T("Focus OK."));
                    }
                    break;
                  case FS_ContinueFocusing:
                    {
                      // add input image and graphics from TVObject
#ifdef CHECK_IS_IN_CONTAINER
                      ASSERT(!IsInContainer(pOut, pROI));
#endif 
                      pOut->AddFrame(pROI);
                      if (NearestSegm.Count() && NearestSegm.m_pFigure)
                      {
#ifdef CHECK_IS_IN_CONTAINER
                        ASSERT(!IsInContainer(pOut, NearestSegm.m_pFigure));
#endif 
                        pOut->AddFrame(NearestSegm.m_pFigure);
                      }
                      bOK = FocusLogAndMoveZ(
                        _T("Continue Focus with standard step."),
                        dNextStep, (CDataFrame**)&pOut);
                      break; // Move and grab, no changes
                    }
                  case FS_TakeValuesFromPrevious:
                    {
                      int iNData = (int)m_DataForPosition.RemoveLast();
                      int iNViews = (int)m_ViewsForPosition.RemoveLast();
                      ASSERT(iNData == iNViews);
                      CContainerFrame * pOutFromView =
                        (CContainerFrame*)m_ViewsForPosition.GetLast();
                      if (pOutFromView != pOut)
                      {
                        if (pOutFromView)
                        {
                          pOut->Release();
                          pOut = pOutFromView;
                          FXString Lab;
                          Lab.Format("%s-LastRepl_%d", pOut->GetLabel(), iNViews);
                          pOut->SetLabel(Lab);
                          pOut->AddRef();
                        }
                      }
                      // add graphics from TVObject
#ifdef CHECK_IS_IN_CONTAINER
                      ASSERT(!IsInContainer(pOut, pROI));
#endif 
                      pOut->AddFrame(pROI);
                      if (NearestSegm.Count() && NearestSegm.m_pFigure)
                      {
#ifdef CHECK_IS_IN_CONTAINER
                        ASSERT(!IsInContainer(pOut, NearestSegm.m_pFigure));
#endif 
                        pOut->AddFrame(NearestSegm.m_pFigure);
                      }
                      bOK = FocusLogAndMoveZ(
                        _T("Previous Focus is  OK."), 0.0, (CDataFrame**)&pOut);
                      break;
                    }
                  default:
                    SEND_GADGET_ERR("Bad Focusing. Process Stopped");
                    m_ProcessingStage = Stage_Inactive;
                    TRACE("***************Bad Focusing. Dark Measurement process Stopped");
                    pOut->Release();
                    pOut = NULL;
                    break;
                  }
                  if (!bOK) // New Z motion ordered for better focusing
                  {
                    FR_RELEASE_DEL(m_pLastExternalContur);
                    return NULL;
                  }
                  //                }

                  bool IsCurrentPosFinished = true;

                  if (IsCurrentPosFinished) // OK, all burrs are measured
                  {
                    FilterLastResults();
                    FXSIZE iNSavedBefore = m_ConturData.GetCount();
                    int iSegmentNumber = (m_ConturData.GetCount()) ?
                      m_ConturData.GetLast().m_iSegmentNumInScan + 1 : 0;

                    if (iNSavedBefore == 0)
                    {
                      m_dLastScanMaxWidth_um = 0.;
                      m_iIndexWithMaxWidth = 0;
                    }
                    m_dLastMaxWidth_um = 0.;
                    cmplx cAver;
                    FXString FullReport;
                    int iLeftInd = m_CurrentViewMeasStatus.m_iMinusIndex;
                    int iRightInd = m_CurrentViewMeasStatus.m_iPlusIndex;
                    if (!IsOneDarkMeasurement())
                    {
                      for (int i = iLeftInd; i < iRightInd; i++)
                      {
                        OneMeasPt& Pt = m_CurrentViewMeasStatus.m_Results[i];
                        if (m_bPrepareToEnd)
                        {
                          cmplx cInRobotCoords = GetPtPosInRobotCoords(Pt);
                          double dDistToInitialPoint =
                            abs(cInRobotCoords - m_cFirstPtRobotCoord);
                          if (m_dDistToPt0.Count() == 10)
                          {
                            memmove(&m_dDistToPt0[1], &m_dDistToPt0[0],
                              sizeof(double) * 9);
                            m_dDistToPt0.SetAt(0, dDistToInitialPoint);
                          }
                          else
                            m_dDistToPt0.InsertAt(0, dDistToInitialPoint);
                          if (dDistToInitialPoint < 2000.)
                          {
                            int iFoundIndex = -1;
                            double dMinValue = dDistToInitialPoint;
                            double Diffs[9];
                            for (int j = 1; j < 10; j++)
                            {
                              if (m_dDistToPt0[j] < dMinValue)
                              {
                                iFoundIndex = j;
                                dMinValue = m_dDistToPt0[j];
                              }
                              Diffs[j - 1] = m_dDistToPt0[j] - m_dDistToPt0[j - 1];
                            }
                            if (m_dMinimalDistToFirstPt < dMinValue)
                              m_dMinimalDistToFirstPt = dMinValue;
                            if (m_dMinimalDistToFirstPt < dMinValue - 150.)
                              m_bScanFinished = true; // Scan is finished, we near first point
                            else if (iFoundIndex > 4)
                            {
                              int iDiffNegLeft = 0, iDiffNegRight = 0;
                              for (int j = 0; j < 9; j++)
                              {
                                if (Diffs[j] < 0)
                                {
                                  if (j < iFoundIndex)
                                    iDiffNegLeft++;
                                }
                                else if (j > iFoundIndex)
                                  iDiffNegRight++;
                              }
                              if (iDiffNegLeft >= 3 && iDiffNegRight >= 3)
                                m_bScanFinished = true; // Scan is finished, we near first point
                              if (m_bScanFinished)
                              {
                                if (iFoundIndex >= ((int) m_ConturData.Count() - (int) iNSavedBefore))
                                  iFoundIndex = ((int) m_ConturData.Count() - (int) iNSavedBefore);
                                if (iFoundIndex)
                                {
                                  m_ConturData.RemoveAt(m_ConturData.Count() - iFoundIndex - 1,
                                    iFoundIndex);
                                }
                              }
                              break;
                            }
                          }
                        }
                        if (AddNewSampleToContur(Pt, i, iSegmentNumber, NULL) == 1)
                        {
                          m_cFirstPtRobotCoord = cmplx(m_ConturData[0].m_RobotPos.m_x,
                            m_ConturData[0].m_RobotPos.m_y); ;
                        }
                      }
                      int iNNewData = (int) m_ConturData.Count() - (int) iNSavedBefore;
                      if (iNNewData)
                      {
                        // Put max value for XY position into all saved now results
                        // Simultaneously calculate middle point of segment for fragment saving
                        for (FXSIZE i = iNSavedBefore; i < m_ConturData.GetCount(); i++)
                        {
                          m_ConturData[i].m_dMaxBurrWidth_um = m_dLastMaxWidth_um;
                          cAver += m_ConturData[i].m_cEdgePt;
                        }
                        cAver /= (double)(m_ConturData.Count() - iNSavedBefore);
                      }
                    }
                    int iGoodZIndex = iRightInd - 1;
                    OneMeasPt& LastPt = m_CurrentViewMeasStatus.m_Results[iGoodZIndex];
                    // This will be target Z coordinate for next XY position
                    m_dNextGoodZ = LastPt.m_RobotPos.m_z;
                    if (LastPt.m_dAverFocus < m_dMinimalLocalFocus)
                    {
                      for (--iGoodZIndex;
                        iGoodZIndex >= iLeftInd; iGoodZIndex--)
                      {
                        OneMeasPt& Pt = m_CurrentViewMeasStatus.m_Results[iGoodZIndex];
                        if (Pt.m_dAverFocus >= m_dMinimalLocalFocus)
                        {
                          m_dNextGoodZ = Pt.m_RobotPos.m_z;
                          break;
                        }
                      }
                    }
                    int iMeasCenter = (iLeftInd + iRightInd) / 2;
                    int iNPtsForAveraging = (iLeftInd + iRightInd) / 4;
                    int iOptimalViewIndex = GetIndexWithOptimalFocus(iMeasCenter, -1 /*iNPtsForAveraging*/);
                    CContainerFrame * pOutFromView =
                      (CContainerFrame*)m_ViewsForPosition.GetFrame(iOptimalViewIndex);
                    if (pOutFromView != pOut)
                    {
                      if (pOutFromView)
                      {
                        pOut->Release();
                        pOut = pOutFromView;
                        pOut->AddRef();
                        pOutFromView = NULL;
                      }
                    }

                    m_dLastOptimalZ =
                      m_CurrentViewMeasStatus.m_Results[m_CurrentViewMeasStatus.m_iRightMeasured].m_RobotPos.m_z;

                    if (!IsOneDarkMeasurement())
                    {
                      FXString FilePath;
                      FilePath.Format(_T("%sPt%d_X%d_Y%d.bmp"),
                        (LPCTSTR)m_ImagesDirectory,
                        iNSavedBefore,
                        ROUND(m_CurrentPos.m_x),
                        ROUND(m_CurrentPos.m_y));

                      CRect Fragment = (cAver.real() == 0.) ? m_LastROI
                        : CRect(CPoint(ROUND(cAver.real() - (m_SaveFragmentSize.real() / 2.)),
                          ROUND(cAver.imag() - (m_SaveFragmentSize.imag() / 2.))),
                          CSize(ROUND(m_SaveFragmentSize.real()), ROUND(m_SaveFragmentSize.imag())));

                      const CVideoFrame * pOptimalFrame = pOut->GetVideoFrame();
                      if (pOptimalFrame)
                      {
                        if (!SaveImageFragment(pOptimalFrame, Fragment,
                          FilePath))
                        {
                          SENDERR("Can't save Pt%d fragment in File %s",
                            m_ConturData.GetCount(), (LPCTSTR)FilePath);
                        };
                        // Now we will do container with optimal view 
                        // and with combined from different images edges
                        CContainerFrame * pFinalOut = CreateCombinedResultContainer(
                          (CVideoFrame*)pOptimalFrame, pDataFrame);
                        if (pFinalOut)
                        {
                          if (pOut)
                            pOut->Release();
                          pOut = pFinalOut;

                          FXString View, Prefix;
                          FXPtrArray FramePtrs;
                          int iNFrames = 0;
                          FormFrameTextView(pOut, View, Prefix, iNFrames, FramePtrs);
                          ASSERT(iNFrames);
                        }
                      }
                      else
                        SENDERR("No Video Frame for Pt #d", m_ConturData.GetCount());
                    }

                    //                   FXString EndReport ;
                    //                   EndReport.Format( "Position End: Optimal Pt=%d Z=%.1f " ,
                    //                     iOptimalViewIndex , m_dLastOptimalZ ) ;
                    //                   TRACE( "\n------%s" , (LPCTSTR) EndReport ) ;
                    //                   CTextFrame * pFocusLog = CreateTextFrame( EndReport , "FocusLog" , 0 ) ;
                    //                   Sleep( 100 ) ;
                    //                   PutFrame( m_pImageSave , pFocusLog ) ;

                    if ((iNSavedBefore == 0)
                      && !IsOneDarkMeasurement())
                    {
                      if (m_iIndexOfFoundObservForm == -1)
                      {
                        if ( !m_iSimulationMode )
                        {
                          m_LastObservationForm.m_dLastOptimalZ = m_dLastOptimalZ;
                          m_LastObservationForm.m_dHeight_um = m_dPartHeight =
                            m_dBaseHeight - m_dLastOptimalZ - m_dAdapterHeight;
                          ASSERT( m_dPartHeight < 40000. ) ;
                        }

                        if (m_ObservGeomAnalyzer.Add(m_LastObservationForm))
                        {
                          m_ObservGeomAnalyzer.Save();

                          CheckCreatePartResultDir();

                          GetCurrentDetailedForm();

                          CheckSendFoundPartInfoToYuri(
                            (LPCTSTR)m_PartFile,
                            ROUND(m_dPartHeight),
                            m_ResultDirectory, NULL, true);


                          if (m_pObservationImage)
                          {
                            CRect ScalingFragment;
                            GetScalingFragment(m_ObservationExtremes, 10, __out ScalingFragment);

                            // Save as common part image
                            FXString FileName = m_PartDirectory
                              + m_PartFile + _T("_PartView.bmp");
                            SaveImageFragment(
                              m_pObservationImage, ScalingFragment, FileName);

                            // ys_20201002 It is unnecessary. The same thing is here: 'case Stage_2_MeasOnObserv: // found one or two contours on observation'
                            // and 'm_pObservationImage' does NOT have a 'lpBMIH->biCompression'// 
                            //                           //Save as image of this measurement
                            //                           FileName = m_ResultDirectory
                            //                             + (m_PartFile + _T( '_' ))
                            //                             + m_MeasurementTS + _T( "_PartView.bmp" ) ;
                            //                           SaveImageFragment(
                            //                             m_pObservationImage , ScalingFragment , FileName ) ;
                            // ys_20201002 is unnecessary same is here: 'case Stage_2_MeasOnObserv: // found one or two contours on observation'
                          }

                        }
                        else  // ??? should be filtered out after observation
                        {
                          FXString Error;
                          Error.Format("The part with name %s is ALREADY EXISTS. "
                            "New part is not added. Check PO# or change part and try again.", (LPCTSTR)m_NewPartName);
                          CheckSendFoundPartInfoToYuri(
                            (LPCTSTR)m_IdentifiedObservForm.m_Name,
                            ROUND(m_IdentifiedObservForm.m_dHeight_um),
                            m_PartDirectory, Error);
                          if (pOut)
                          {
                            PutAndRegisterFrame(m_pMarking, pOut);
                            pOut = NULL;
                          }
                          SetProcessingStage(Stage_Inactive);
                          return NULL;
                        }
                      }
                    }
                  }
                  else
                  {
                  }
                }
                else
                {
                  //                 CTextFrame * pFocusLog = CreateTextFrame(
                  //                   "ConturASM::FindFocusAndBurrByAvg failed to measure" ,
                  //                   "FocusLog" , 0 ) ;
                  //                 PutFrame( m_pImageSave , pFocusLog ) ;
                }
              }
              else if (!IsOneMeasurement())
              {  // found edge doesn't pass near center
                 // or too much attempts
                m_DataForPosition.RemoveAll();
                m_ViewsForPosition.RemoveAll();

                m_cTraveling += cmplx(
                  fabs(m_cLastMotion.real()), fabs(m_cLastMotion.imag()));
                m_dTraveling += abs(m_cLastMotion);
                m_MotionMask = m_iNWaitsForMotionAnswers = 0;
                double dZ = m_dNextGoodZ - m_CurrentPos.m_z;
                CCoordinate Vect(m_cLastMotion.real(), m_cLastMotion.imag(), dZ);
                ProcessingStage PSBeforeMotion = m_ProcessingStage;
                OrderRelMotion(Vect);  // <<<<<<<<<<<<<<<<<<<<<<<< do step to next position
                if (pOut)
                {
                  FXString ContList, Prefix;
                  int iLevel = 0;
                  FXPtrArray FramePtrs;
                  int iNFrames = FormFrameTextView(
                    pOut, ContList, Prefix, iLevel, FramePtrs);
                  PutAndRegisterFrame(m_pMarking, pOut);
                }
                return NULL;
              }
            }
            break;
          case Stage_12_MeasWalls:
          case Stage_GetExternalBorder:
            {
              iNIntervals = FindWallEdges(
                pImage, pExtData, iFigLen, iFirstPtIndex,
                abs(iLastPtIndex - iFirstPtIndex) + 1,
                m_dRealLightDirForExternal,
                pExt, pInt, pAdditional, pDirMarking); // pAdditional is for Shadow
              if (iNIntervals < m_iMinimalContinuty * 2)
              {
                if (pOut)
                {
                  FXString ContList, Prefix;
                  int iLevel = 0;
                  FXPtrArray FramePtrs;
                  int iNFrames = FormFrameTextView(
                    pOut, ContList, Prefix, iLevel, FramePtrs);
                  PutAndRegisterFrame(m_pMarking, pOut);
                  pOut = NULL;
                }
                if (m_iStopOnFault != 0)
                  Sleep(abs(m_iStopOnFault));
                switch (m_ProcessingStage)
                {
                case Stage_12_MeasWalls:
                  SetLightForDarkEdge(!IsBeforeDark() ? 0 : -1);
                  m_ProcessingStage = Stage_12a_MeasDarkEdge;
                  break;
                default:
                  break;
                }
                return NULL;
              }
            }
            break;
          case Stage_12a_MeasDarkEdge:
            iNIntervals = FindBurrByAvgAndStd(
              pImage , pExtData , iFigLen , iFirstPtIndex ,
              abs( iLastPtIndex - iFirstPtIndex ) + 1 ,
              m_iSegmentLength , iNGood , pExt , pInt , pAdditional , pDirMarking );
            if ( m_iSimulationMode )
            {
              for ( int i = 0 ; i < pExt->GetCount() ; i++ )
                pOut->AddFrame( CreatePtFrameEx( CDPointToCmplx( pExt->GetAt( i ) ) , 0xff0000 ) ) ;
              for ( int i = 0 ; i < pInt->GetCount() ; i++ )
                pOut->AddFrame( CreatePtFrameEx( CDPointToCmplx( pInt->GetAt( i ) ) , 0xff0000 ) ) ;
              pOut->AddFrame( pExt ) ;
              pOut->AddFrame( pInt ) ;
              pOut->AddFrame( pAdditional ) ;

              if ( m_dLastGradValue_um != 0. )
              {
                pOut->AddFrame( CreateTextFrame( m_cGradViewPt , "0x0000ff" , 14 , NULL ,
                  pDataFrame->GetId() , "%.2fum" , m_dLastGradValue_um ) ) ;
              }

              if ( m_iLastSimulationPt > m_iCurrPtForSimulation ) // End of scan
              {
                m_bPrepareToEnd = true ;
                m_iPercentOfMeasured = 100;
                m_bScanFinished = true ;
                break ;
              }
              else
                m_iLastSimulationPt = m_iCurrPtForSimulation ;
             // pOut->AddFrame( pDirMarking ) ;
            
            }
            break;
          default:
            break;
          }
        }
        cmplx PPtStep;

        if (IsEmbeddedFocus() && m_ConturData.Count())
        {
          cmplx cFirstPtRobotPos(m_ConturData[0].m_RobotPos.m_x,
            m_ConturData[0].m_RobotPos.m_y);

          cFirstPtRobotPos -= m_cPartCenterOnMeas;
          cmplx cLastPtRobotPos(m_ConturData.GetLast().m_RobotPos.m_x,
            m_ConturData.GetLast().m_RobotPos.m_y);
          cLastPtRobotPos -= m_cPartCenterOnMeas;
          double dAngleDiff = GetAngleBtwVects(cFirstPtRobotPos, cLastPtRobotPos);
          if (dAngleDiff <= 0. && dAngleDiff > -0.01)
          {
            if (!m_bPrepareToEnd)
              m_iPercentOfMeasured = 1;
            else
              m_iPercentOfMeasured = 100;
          }
          else
          {
            m_iPercentOfMeasured = ROUND(100.
              * ((dAngleDiff < 0.) ? -dAngleDiff : (M_2PI - dAngleDiff)) / M_2PI);
          }

          if (dAngleDiff > M_PI / 2)
            m_bPrepareToEnd = true;
          // Analysis for contour finishing
          if (m_bPrepareToEnd && ((dAngleDiff < 0.) || m_bScanFinished))
          {
            FXString FinishMessage;
            FinishMessage.Format("Measurement Finished\n"
              "Npts=%d BeginToEndDiff="
              "(%.1f,%.1f,%.1f,%.1f,%.1f,%.1f,%.1f,%.1f,%.1f,%.1f)",
              m_ConturData.GetCount(),
              m_dDistToPt0[0], m_dDistToPt0[1],
              m_dDistToPt0[2], m_dDistToPt0[3],
              m_dDistToPt0[4], m_dDistToPt0[5],
              m_dDistToPt0[6], m_dDistToPt0[7],
              m_dDistToPt0[8], m_dDistToPt0[9]);

            m_bScanFinished = true;
            m_iPercentOfMeasured = 100;
            CTextFrame * pFullReport = CreateTextFrame(
              (LPCTSTR)FinishMessage, "CloseFile", 1000000);
            PutFrame(m_pOutput, pFullReport);
            m_ViewsForPosition.RemoveAll();
            m_LastExtConturs.RemoveAll();
            m_ObservationPartConturs.RemoveAll();
          }
          else  // send to EdgeMeas gadget (TVObjects) a prediction for edge direction
          {
            //             int iFoundGood = 0 ;
            //             cmplx cLast , cPrev ;
            //             for ( int i = m_ConturData.Count() - 1 ;
            //               i >= 0 ; i-- )
            //             {
            //               ConturSample& Pt = m_ConturData.GetAt( i ) ;
            //               if ( IsValidGradValue( Pt.m_dAveBurrWidth_um ) )
            //               {
            //                 if ( cLast.real() == 0. )
            //                   cLast = Pt.m_cEdgePt ;
            //                 else if ( ++iFoundGood > 6 )
            //                 {
            //                   cPrev = Pt.m_cEdgePt ;
            //                   break ;
            //                 }
            //               }
            //             }
            //             if ( iFoundGood > 6 )
            //             {
            SetPredictedEdgeAngle() ;
          }

          if (m_DataForPosition.Count())
          {
            MeasuredValues LastResult = m_DataForPosition.GetLast();
            OneMeasPt FirstOnSegmentPt = LastResult.m_Results[LastResult.m_iMinusIndex];
            OneMeasPt LastOnSegmentPt = LastResult.m_Results[LastResult.m_iPlusIndex - 1];
            cmplx cFirstPt = FirstOnSegmentPt.m_cEdgePt;
            //           cmplx cLastPt = LastOnSegmentPt.m_cEdgePt ;
            //           //          cmplx cVect = cFirstPt - cLastPt ;
            //           cmplx cVect = m_cMeasFOVCent - cLastPt ;

            int iLastPtOnEdge = (int) m_pLastExternalContur->Count() - 1;
            cmplx cLast = pExtData[iLastPtOnEdge];
            cmplx cVect = m_cMeasFOVCent - cLast;
            m_cLastNormalMotion = m_cLastMotion = cVect / m_dScale_pix_per_um;
            //           // Motion of last point to opposite side of measurement zone
            //           // after full segment measurement
            //           // 20.06.20
            //           m_cLastNormalMotion = m_cLastMotion = cVect * 2.0 / m_dScale_pix_per_um ;
            //          PPtStep = cLastPt ;
            double dMinFoc, dMaxFoc, dAverAverFoc;
            int iMaxWidthPos;
            LastResult.GetAvForGoodValues(
              m_dLastAverageWidth_um, m_dLastMaxWidth_um, iMaxWidthPos,
              dMinFoc, dMaxFoc, dAverAverFoc, m_dMinimalLocalFocus);
            m_iLastMinAverageDefocusing = ROUND(dMinFoc);
            m_iLastMaxAverageDefocusing = ROUND(dMaxFoc);
            //FR_RELEASE_DEL( m_pLastExternalContur );
          }
        }
        else if (!IsOneMeasurement())
        {
          PPtStep = GetRelElem(pExtData,
            m_iMinDistIndexForExternal, m_iMeasStep + m_iSegmentLength, iFigLen);
          m_cLastNormalMotion = m_cLastMotion =
            (m_cMeasFOVCent - PPtStep) / m_dScale_pix_per_um;
        }

        int iNecessaryLight = GetLightDir(m_iDir, false);

        if (!IsOneMeasurement())
        {
          if ( !IsDarkWithEmbFocus() && !m_iSimulationMode )
          {
            // form cross on the place of next measurement center
            CFigureFrame * pViewRobotTarget = CreateFigureFrame(&PPtStep, 1,
              GetHRTickCount(), "0xff0000");
            pOut->AddFrame(pViewRobotTarget); // once

            cmplx OneVectToOrtho = conj(polar(1., m_dLightForExternal));
            cmplx OneVectToSCan = GetNormalized(m_cEdgeVect);
            cmplx cContrastPt = PPtStep - 10. * OneVectToOrtho;
            double dStd2, dPrev = 0., dContrast = 0.;
            m_dLastNextPtContrast = 0.;

            // form cross on the place of next measurement center
            CFigureFrame * pViewContrastBeginPt = CreateFigureFrame(&cContrastPt, 1,
              GetHRTickCount(), "0x800000");
            pOut->AddFrame(pViewContrastBeginPt); // once

            double Signal[30];
            for (int i = 0; i < 20; i++)
            {
              double dAverage = GetAvrgAndStdOnLine(
                cContrastPt, OneVectToSCan, 3.,
                Signal, ARRSZ(Signal), dStd2, pImage);

              if (dPrev > 0.)
              {
                dContrast = fabs(dPrev - dAverage);
                if (m_dLastNextPtContrast < dContrast)
                  m_dLastNextPtContrast = dContrast;
              }
              dPrev = dAverage;
              cContrastPt += OneVectToOrtho;
            }

            // Focus status processing

            //  Old method: next point is analyzed relatively to current point
            //bool bContrastRatioCritery = 
            //  m_dLastNextPtContrast < m_dFocusDecreaseNorm * m_dLastContrast ;
            //bool bTooLowContrast = m_dLastContrast < m_dMinimalContrast ;
            // m_bIsFocusingNecessary = bContrastRatioCritery || bTooLowContrast ;

            // New method: next point is analyzed relatively to contrast after focusing
            if (m_dLastContrast > m_dLastContrastAfterFocusing)
              m_dLastContrastAfterFocusing = m_dLastContrast;
            if (GetHRTickCount() > m_dLastFocusTime)
            {
              if (m_dLastFocusValue > m_dLastFocusValueAfterFocusing)
                m_dLastFocusValueAfterFocusing = m_dLastFocusValue;
              m_bIsFocusingNecessary =
                m_dLastFocusValue < (m_dLastFocusValueAfterFocusing * m_dFocusDecreaseNorm);

            }
            else
            {
              m_bIsFocusingNecessary =
                m_dLastNextPtContrast < (m_dLastContrastAfterFocusing * m_dFocusDecreaseNorm);
            }
          }
          else
            m_bIsFocusingNecessary = false;
          
          if ( !m_iSimulationMode )
          {
            double dMotionDist = abs( m_cLastMotion );
            cmplx cClosestToCent = GetRelElem( pExtData ,
              m_iMinDistIndexForExternal , 0 , iFigLen );
            double dDistToCent = abs( cClosestToCent - m_cMeasFOVCent );
            if ( dDistToCent > 75. )
            {
              if ( pOut )
              {
                FXString ContList , Prefix;
                int iLevel = 0;
                FXPtrArray FramePtrs;
                int iNFrames = FormFrameTextView(
                  pOut , ContList , Prefix , iLevel , FramePtrs );
                PutAndRegisterFrame( m_pMarking , pOut );
                pOut = NULL;
              }
              pOut = GetNewOutputContainer( pDataFrame );

              if ( m_iStopOnFault != 0 )
                Sleep( abs( m_iStopOnFault ) );
              ASSERT( m_iStopOnFault <= 0 );
            }
            if ( dMotionDist > 5. )
            {
              m_cLastNotSmallMotion = m_cLastMotion;
              m_iNotMeasuredCntr = 0;
            }
          }
        }

        double dAngleFromCenter = NormTo2PI(GetAngleFromCenter());

        if (m_iStopOnFault != 0)
          Sleep(abs(m_iStopOnFault));
        if ( !IsOneMeasurement() && !m_iSimulationMode )
        {
          if (m_bPrepareToEnd && m_bScanFinished)
          {
            SetProcessingStage(Stage_ScanFinished, true); // scanning finished
            m_DataFrom = ADF_MeasuredNow;
          }
          else if ( !m_iSimulationMode )
          {
            if (IsEmbeddedFocus())
            {
              SetProcessingStage(Stage_BeginToMax, true);
            }
            if (!IsEmbeddedFocus() && !m_bPrepareToEnd
              && (dAngleFromCenter > 1.5 * M_PI)) // angle is over zero (cw direction)
            {
              m_bPrepareToEnd = true;
            }
            m_cTraveling += cmplx(
              fabs(m_cLastMotion.real()), fabs(m_cLastMotion.imag()));
            m_dTraveling += abs(m_cLastMotion);
            m_MotionMask = m_iNWaitsForMotionAnswers = 0;
            double dZ = m_dNextGoodZ - m_CurrentPos.m_z;
            CCoordinate Vect(m_cLastMotion.real(), m_cLastMotion.imag(), dZ);
            ProcessingStage PSBeforeMotion = m_ProcessingStage;
            OrderRelMotion(Vect);  // <<<<<<<<<<<<<<<<<<<<<<<< do step to next position
            if (m_bFocusingAfterPosFinishing)
            {
              InitIterativeFocusing(PSBeforeMotion, false);
              m_DelayedOperations.push(Stage_Focusing);
              m_StageBeforeMotion = Stage_Focusing;
              m_bFocusingAfterPosFinishing = false;
            }

            // Is necessary to do focusing?
            if (m_bIsFocusingNecessary
              /*|| ( m_iFocusingPeriod &&
              ( ++m_iNStepsAfterFocusing >= m_iFocusingPeriod) )*/)
            {
              m_bFocusFound = false;
              m_bInLineFocusing = true;
              m_iNStepsAfterFocusing = 0;
            }

          }
        }

        if (IsOneMeasurement() || m_iSimulationMode )
        {
          //SetProcessingStage( Stage_Inactive , true , ForReporting ) ;
        }
        else 
        {
          if (abs(m_cLastMotion) < 2.0
            || m_ProcessingStage == Stage_ScanFinished)
          {
            // Too small motion, i.e. go to internal edge measurement
            // without motion
            m_MotionMask = m_iNWaitsForMotionAnswers = 0;

            // Go to internal edge measurement (no motion)
            switch (m_ProcessingStage)
            {
            case Stage_12_MeasWalls:
            case Stage_12a_MeasDarkEdge:
              SetLightForInternal(m_dLightForInternal,
                m_iIntLight_us, m_dLastLightAngle);
              SetProcessingStage(Stage_11_GetExternalBorder, true, ForReporting);
              break;
            case Stage_GetExternalBorder:
              SetLightForInternal(m_dLightForInternal,
                m_iIntLight_us, m_dLastLightAngle);
              SetProcessingStage(Stage_GetInternalBorder, true, ForReporting);
              break;
            case Stage_ScanFinished:
              break;
              //             case Stage_OneDarkMeasurement:
              //               SetLightForDarkEdge( 0 ) ;
              //               break ;
            default:
              SetProcessingStage(Stage_Inactive, true);
              break;
            }
          }
          else
          {
            switch (m_StageBeforeMotion)
            {
            case Stage_12_MeasWalls:
            case Stage_12a_MeasDarkEdge:
              SendReport(ForReporting);
              if (!m_bInLineFocusing)
                // Order imaging after motion
                m_DelayedOperations.push(Stage_10_GetIntOrExtBorder);
              else
              {
                // Order focusing after motion
                m_DelayedOperations.push(Stage_13_ScanInlineFocusing);
              }
              break;
            case Stage_GetExternalBorder:
              SetProcessingStage(Stage_GetInternalBorder, true);
              break;
            case Stage_OneDarkMeasBeginToMax:
            case Stage_OneDarkMeasEndToMax:
              SendReport(ForReporting);
              SetProcessingStage(m_StageBeforeMotion, true);
              break;
            case Stage_BeginToMax:
            case Stage_EndToMax:
              SendReport(ForReporting);
              //SetProcessingStage( Stage_BeginToMax , true ) ;
              break;
            case Stage_Focusing:
              if (m_StageAfterFocusing != Stage_Inactive)
              {
                m_DataForPosition.RemoveAll();
                m_ViewsForPosition.RemoveAll();
                break;
              }
            default:
              SetProcessingStage(Stage_Inactive, true);
              break;
            }
          }
        }
        if ( IsDarkWithEmbFocus() ) 
        {
          cmplx Width = m_dLastAverageWidth_um;

          BOUND(m_iPercentOfMeasured, 0, 100);

          FXString ForReporting;

          int ResLen = FormReportStringForUI(ForReporting);

          CTextFrame * pResult = CreateTextFrame(ForReporting, _T("Result"),
            (int)m_ConturData.GetCount());

          if (m_ProcessingStage == Stage_ScanFinished)
          {
            ResLen = (int) ForReporting.GetLength();
          }

          PutFrame(m_pOutput, pResult);

          CTextFrame * pTextInfo = CreateTextFrame(_T("PtResult"), (int)m_ConturData.GetCount());

          CTextFrame * pEndOfScan = NULL;

          if (m_ProcessingStage == Stage_ScanFinished) // Really we check for Stage_ScanFinished
          {
            double dWidth = 0.;
            FXString Name, Units;
            int iIndex = 0;

            switch (m_ResultViewMode)
            {
            case   RVM_BurrWidth:
              iIndex = m_iIndexWithMaxOfMaxWidth;
              dWidth = m_ConturData[iIndex].m_dAveBurrWidth_um;
              break;
            case   RVM_MaxBurrWidth:
              iIndex = m_iIndexWithMaxOfMaxWidth;
              dWidth = m_ConturData[iIndex].m_dAveBurrWidth_um;
              break;
            case RVM_3D:
            case   RVM_BurrHeight:
            case   RVM_MaxBurrHeight:
            case   RVM_BurrVolume:
            case   RVM_BurrEdgeLength:
              iIndex = 0;
              break;
            }
            m_dLastScanTime_ms = GetHRTickCount() - m_dMeasurementStartTime;

            pTextInfo = FormScanFinishMessage("");
            pOut->AddFrame(pTextInfo); // once
            pTextInfo = NULL;

            pEndOfScan = FormScanFinishMessage("Result");
            Sleep(250);
            PutFrame(m_pOutput, pEndOfScan);
            SaveScanResults();
            SaveLastScanParametersToRegistry( true );
          }

          if (!pTextInfo)
          {
            pTextInfo = CreateTextFrame(_T("MeasResult"),
              (int)m_ConturData.GetCount());
          }
          if ( m_DataForPosition.GetCount() )
          {
            MeasuredValues& Vals = m_DataForPosition.GetLast();
            double dMinFoc, dMaxFoc, dAverAverFoc;
            int iMaxWidthIndex = 0;
            Vals.GetAvForGoodValues(
              m_dLastAverageWidth_um, m_dLastMaxWidth_um, iMaxWidthIndex,
              dMinFoc, dMaxFoc, dAverAverFoc, m_dMinimalLocalFocus);
            m_iLastMinAverageDefocusing = ROUND(dMinFoc);
            m_iLastMaxAverageDefocusing = ROUND(dMaxFoc);
            pTextInfo->GetString().Format(
              _T("Wav=%6.2fum Wmax=%6.2fum\n"
                "Ready=%d , Tproc=%.2f T=%.0f\n"
                "Cmin=%4d Cmax=%4d a=%.3f b=%.f\n "
                "Avl=%.1f Avc=%.1f Avr=%.1f AvAv=%.1f\n"
                //"MAXl=%.1f MAXc=%.1f MAXr=%.1f\n"
                //"Dmax=%.2f Rlen=%.3f Std=%.2f\n"
                "Tmax=%.2f Lmin=%.1f Lmax=%.1f"
              ),
              m_dLastAverageWidth_um, m_dLastMaxWidth_um,
              m_iPercentOfMeasured, GetHRTickCount() - dTBegin,
              GetHRTickCount() - m_dMeasurementStartTime,
              m_iLastMinAverageDefocusing, m_iLastMaxAverageDefocusing,
              m_dLastFocusRegressionA, m_dLastFocusRegressionB,
              Vals.m_FV.m_dLastAvFocusNearBegin, Vals.m_FV.m_dLastAvFocusInCenter,
              Vals.m_FV.m_dLastAvFocusNearEnd, dAverAverFoc,
              //                 Vals.m_FV.m_dLastMaxFocusNearBegin ,
              //                 Vals.m_FV.m_dLastMaxFocusInCenter , Vals.m_FV.m_dLastMaxFocusNearEnd ,
                              //m_dMaxCrossDist , m_dLengthRatio , m_dCrossDistStd ,
              RadToDeg(m_dLastMaxTurn), m_dLastMinSegmentLength,
              m_dLastMaxSegmentLength
            );
            // Following two arrays are not necessary for continuation
          }
          else
            pTextInfo->GetString() = "No Data";

          m_DataForPosition.RemoveAll();
          m_ViewsForPosition.RemoveAll();
          // true below means that all accounting 
          // will be from segment beginning
          // (not from segment center)
          m_CurrentViewMeasStatus.Reset(true);
          pTextInfo->Attributes()->Format("color=0x%06x; Sz=%d; x=%d; y=%d;",
            0x0000ff, 12,
            ROUND(m_cTextViewCent.real()), ROUND(m_cTextViewCent.imag()));
          pOut->AddFrame(pTextInfo); // once
          pTextInfo = NULL;
          if (!IsOneMeasurement() && (m_dWidthViewScale != 0))
          {
            // Form graphical presentation
            cmplx cExtViewPoint =
              conj(m_cPartCentPos - m_cRobotPos) / m_dEdgeViewScale;
            cExtViewPoint += m_cMeasFOVCent;
            if (!m_pPartExternalEdge)
            {
              m_pPartExternalEdge = CreateFigureFrame(&cExtViewPoint, 1,
                GetHRTickCount(), _T("0x00ff00"), _T("Replace:ExtContur"), pDataFrame->GetId());
            }
            else
              m_pPartExternalEdge->Add(*((CDPoint*)&cExtViewPoint));
            m_Widths.Add(m_dLastAverageWidth_um);
            PutFrame(m_pOutput, m_pPartExternalEdge->Copy());

            CDataFrame * pMapView = NULL;
            switch (m_ResultViewMode)
            {
            case RVM_3D:
              pMapView = FormImageFor3dView(pImage);
              break;
            case   RVM_BurrWidth:
            case   RVM_MaxBurrWidth:
            case   RVM_BurrHeight:
            case   RVM_MaxBurrHeight:
            case   RVM_BurrVolume:
            case   RVM_BurrEdgeLength:
              pMapView = FormMapImage(pImage, m_ResultViewMode,
                m_iIndexWithMaxWidth,
                m_iPercentOfMeasured);
              break;
            }

            if (pMapView)
            {
              if (m_ProcessingStage == Stage_ScanFinished)
              {
                CalcAveragesForRegions(m_AverData);
                PrepareDataForCombinedMap(pMapView);
              }
              PutFrame(m_pOutput, pMapView);
            }

            cmplx cIntViewPoint = cExtViewPoint
              - m_dLastAverageWidth_um * m_dWidthViewScale / m_dEdgeViewScale;
            if (!m_pPartInternalEdge)
            {
              m_pPartInternalEdge = CreateFigureFrame(&cIntViewPoint, 1,
                GetHRTickCount(), _T("0x00ffff"), _T("Replace:IntContur"),
                pDataFrame->GetId());
            }
            else
              m_pPartInternalEdge->Add(*((CDPoint*)&cIntViewPoint));
            PutFrame(m_pOutput, m_pPartInternalEdge->Copy());

            // Form image for 3d presentation
//             CVideoFrame * pResultView = NULL ;
// 
// 
//             if ( pResultView )
//               PutAndRegisterFrame( m_pMarking , pResultView ) ;
          }
        }

        if (m_cResMiddlePoint != 0.) // this is for image centering in FRender
        {
          CTextFrame * pMiddlePoint = CreateTextFrame("SetCenter", pDataFrame->GetId());
          pMiddlePoint->GetString().Format("Xc=%d;Yc=%d;",
            ROUND(m_cResMiddlePoint.real()), ROUND(m_cResMiddlePoint.imag()));
          pOut->AddFrame(pMiddlePoint); // once
        }

        FXString ContList, Prefix;
        int iLevel = 0;
        FXPtrArray FramePtrs;
        int iNFrames = FormFrameTextView(
          pOut, ContList, Prefix, iLevel, FramePtrs);

        if ( m_iSimulationMode )
        {
          if ( !m_bScanFinished )
          {
            PutAndRegisterFrame( m_pMainView , pOut );
            SetPredictedEdgeAngle() ;
            Sleep( m_iSimulationMode & ~0x7 ) ;
            OrderBMPForSimulation( true ) ; // get next image for measurement
          }
          else
          {
            if ( m_iSimulationMode )
              OrderNextPartForSimulation( 1 ) ; // success
            else
              SetProcessingStage( Stage_ScanFinished ) ;
            pOut->Release() ;
          }
        }
        else
          PutAndRegisterFrame(m_pMainView, pOut);
        pOut = NULL;
      }
      if (IsOneMeasurement() || m_ProcessingStage == Stage_ScanFinished)
      {
        SetProcessingStage(Stage_Inactive, true);
        m_LastExtConturs.RemoveAll();
      }
      break;
    default:
      break;
    }
  }
  if (pOut)
  {
    FXString ContList, Prefix;
    int iLevel = 0;
    FXPtrArray FramePtrs;
    int iNFrames = FormFrameTextView(
      pOut, ContList, Prefix, iLevel, FramePtrs);
    PutAndRegisterFrame(m_pMarking, pOut);
  }

  return NULL;
}

bool ConturAsm::GetScalingFragment(const CmplxArray& source, int scalingFactor, __out CRect& ScalingFragment) const
{
  bool res = false;

  if (source.Count() == 4 && scalingFactor > 0)
  {
    // Calculate rectangle for fragment saving
    ScalingFragment.left = ROUND(source[0].real());
    ScalingFragment.top = ROUND(source[1].imag());
    ScalingFragment.right = ROUND(source[2].real());
    ScalingFragment.bottom = ROUND(source[3].imag());

    ScalingFragment.InflateRect(
      ScalingFragment.Width() / scalingFactor, ScalingFragment.Height() / scalingFactor);

    res = ScalingFragment.Width() && ScalingFragment.Height();
  }
  return res;
}

cmplx ConturAsm::FindExtremePoint(
  const CDataFrame * pDataFrame, LPCTSTR pFigNamePart, Edge WhatExtreme)
{
  CFigureFrame * pCent = (CFigureFrame*)GetFrameWithLabel(
    pDataFrame, figure, pFigNamePart, WP_Full);
  if (pCent)
  {
    FXString ConturName(_T("Contur["));
    ConturName += pFigNamePart;
    CFigureFrame * pFigure = (CFigureFrame*)GetFrameWithLabel(
      pDataFrame, figure, ConturName, WP_Any);

    if (pFigure)
    {
      ReplaceFrame((const CDataFrame**)&m_pPartConturOnObservation,
        pFigure);

      cmplx cCent = CDPointToCmplx(pCent->GetAt(0));
      m_cPartCenterOnObservation = cCent;
      cmplx cCentByExtremes = FindExtrems(pFigure,
        m_ObservationExtremes, NULL, &m_cObservationSizes);
      switch (WhatExtreme)
      {
      case E03: return m_ObservationExtremes[ EXTREME_INDEX_RIGHT ]; // right extremum
      case E06: return m_ObservationExtremes[ EXTREME_INDEX_BOTTOM ]; // down extremum
      case E09: return m_ObservationExtremes[ EXTREME_INDEX_LEFT ]; // left extremum
      case E12: return m_ObservationExtremes[ EXTREME_INDEX_TOP ]; // upper extremum
      }
    }
  }
  return cmplx();
}
// Returns CRect near left point on contour with  y=cCenter.imag() 
CRect ConturAsm::GetFocusingArea(const CFigureFrame * pFigure, cmplx cCenter)
{
  CRect Result(0, 0, 0, 0);

  if (pFigure)
  {
    cmplx cLeftOnPart = FindNearestToY(pFigure,
      cCenter, false);

    Result.left = ROUND(cLeftOnPart.real() + 10);
    Result.top = ROUND(cLeftOnPart.imag() - 10);
    Result.right = Result.left + 20;
    Result.bottom = Result.top + 20;
  }
  return Result;
}
// Returns CRect near left extreme point inside contour 
CRect ConturAsm::GetFocusingArea(const CFigureFrame * pFigure)
{
  CRect Result(0, 0, 0, 0);

  if (pFigure)
  {
    int iIndex;
    cmplx cLeftOnPart = FindLeftPt(pFigure, iIndex);

    Result.left = ROUND(cLeftOnPart.real() + 10);
    Result.top = ROUND(cLeftOnPart.imag() - 10);
    Result.right = Result.left + 20;
    Result.bottom = Result.top + 20;
  }
  return Result;
}
// Returns CRect near selected extremum inside contour
CRect ConturAsm::GetFocusingArea(const CmplxArray& Extrems, Edge WhatExtreme)
{
  CRect Result(0, 0, 0, 0);

  if (Extrems.GetCount() >= 4)
  {
    cmplx Pt;
    switch (WhatExtreme)
    {
    case E03: // right extremum
      {
        Pt = Extrems[2];
        Result.left = ROUND(Pt.real() - 30);
        Result.top = ROUND(Pt.imag() - 10);
      }
      break;
    case E06: // down extremum
      {
        Pt = Extrems[3];
        Result.left = ROUND(Pt.real() - 10);
        Result.top = ROUND(Pt.imag() - 30);
      }
      break;
    case E09: // left extremum
      {
        Pt = Extrems[0];
        Result.left = ROUND(Pt.real() + 10);
        Result.top = ROUND(Pt.imag() - 10);
      }
      break;

    case E12: // upper extremum
      {
        Pt = Extrems[1];
        Result.left = ROUND(Pt.real() - 10);
        Result.top = ROUND(Pt.imag() + 10);
      }
      break;
    }
    Result.right = Result.left + 20;
    Result.bottom = Result.top + 20;
  }
  return Result;
}

int ConturAsm::FindBurrByAvgAndStd(const CVideoFrame * pVF,
  cmplx * pEdgeData, int iEdgeDataLen, int iPt1Index,
  int iNPtsOnEdge, int iStep, int& iNGood,
  CFigureFrame * pExternal, CFigureFrame * pInternal,
  CFigureFrame * pAdditional, CFigureFrame * pDirMarking)
{
  int iMaxSignalLen = m_iOut_pix + m_iIn_pix + 5;

#define TEMP_ALLOC_SIZE 150
  double pSignal[TEMP_ALLOC_SIZE];
  //   double pAverage[ TEMP_ALLOC_SIZE ] ;
  double pStd[TEMP_ALLOC_SIZE];
  double pFullAver[TEMP_ALLOC_SIZE];
  double pFullStd[TEMP_ALLOC_SIZE];

#define NMAX_INTERVALS 150
  PosOnInterval pPositions[NMAX_INTERVALS];  // up to 50 intervals

  double dLen = 0 ;
  for ( int i = 1 ; i < iEdgeDataLen ; i++ )
    dLen += abs(pEdgeData[ i ] - pEdgeData[ i - 1 ]) ;

  double dAvSegmLen = dLen / iEdgeDataLen ;

  m_dLastNextPtContrast = 0.; // Init for correct output in all cases
  int iFigLen = (int)m_pLastExternalContur->GetCount();

  int iLastPt = iNPtsOnEdge - 1 ;

  int iPtOnEdge = iPt1Index;
  int iNextPt = iPtOnEdge + 2 ;
  int iNMeasurements = 0;
  int iNIntervals = iNPtsOnEdge - 1 ; // for the case, when intervals are
                                      // measured by TVObjects
  int iNSteps = m_iOut_pix + m_iIn_pix;
  cmplx * pIntData = NULL , cFirstInt , cLastInt;
  bool bInverse = false;
  if ( m_iSimulationMode )
    m_LastExternalConturRobotPos = m_CurrentPos;

  cmplx cShift = m_cRobotPos - ( cmplx ) m_LastExternalConturRobotPos;
  int iIndexOnInternal = -1;
  int iIntFigLen = 0;

  cmplx cCent = m_cMeasFOVCent;
  double dMinDist = DBL_MAX;
  m_dLastContrast = 0.;
  iNGood = 0;
  m_LastAverageDefocusingValues.RemoveAll();
  m_LastMaxDefocusingValues.RemoveAll();

  cmplx cViewBase , cViewOrt ;
  int iViewIndex = iLastPt / 2 ;
  int iStepLimit = m_iOut_pix + m_iIn_pix ;
  // Full edge processing cycle (for all points on edge received)
  while ( iNextPt < iLastPt )
  {
    cmplx cFirstExt = pEdgeData[ iPtOnEdge ] ; // On external contour 
    cmplx cNextExt = pEdgeData[ iNextPt ] ;    // measured by TVObjects gadget

    cmplx cVect = m_cEdgeVect = cNextExt - cFirstExt;
    cmplx cCentPt = pEdgeData[ iPtOnEdge + 1 ] ;

    cmplx cOrthoRight = GetNormalized( GetOrthoRightOnVF( cVect ) ) ; // normalized
    cmplx cMoveDir = GetNormalized( cVect ) ;

    double dMaxAvInside = -DBL_MAX;
    double dMinAvInside = DBL_MAX;
    int iMaxAvIndexInside = 0;
    int iMinAvIndexInside = 0;
    double dMaxAverage = -DBL_MAX;
    double dMinAverage = DBL_MAX;
    int iMaxAvIndex = 0;
    int iMinAvIndex = 0;
    int iBigStepDownIndex = 0;
    int iIndexForDarkEdgeMeas = 0;
    double dMaxStd = -DBL_MAX;
    double dMinStd = DBL_MAX;
    int iMaxStdIndex = 0;
    int iMinStdIndex = 0;
    double dMaxStdUpValue = -DBL_MAX;
    int iMaxStdUpIndex = 0;
    int iStepIndex = 0; // Counter in Average power array pFullAver
    double dStdSum = 0.;
    double dAvSum = 0.;
    double dAvStd = 0.;
    double dAvOfAv = 0.;
    double dAvOfAvOnEdge = 0;
    double dContrast = 0.;
    double dPrev = 0.;
    int iFromOut = -m_iOut_pix;
  
    cmplx FirstCntrst = cCentPt + (( double ) iFromOut * cOrthoRight) ;
    cmplx cScanBeginShiftToSide = cMoveDir * dAvSegmLen * 0.5 ;
    cmplx cOrtIterator = FirstCntrst - cScanBeginShiftToSide ;
    if ( iNMeasurements < ARRSZ( pPositions ) )
    {
      pPositions[ iNMeasurements ].m_cFirst = FirstCntrst ;
      pPositions[ iNMeasurements ].m_cOrtho = cOrthoRight ;
    }
    int iNMeasured = 0 ;
    // Average signal extraction cycle (average in ortho direction)
    while ( iStepIndex <  iStepLimit )
    {
      cmplx cLastPt = cOrtIterator + dAvSegmLen * cMoveDir;
      if (IsPtInFrame(cOrtIterator, pVF) && IsPtInFrame(cLastPt, pVF))
      {
        double dAverage = pFullAver[iStepIndex] = GetAvrgAndStdOnLine(
          cOrtIterator, cMoveDir, dAvSegmLen,
          pSignal, ARRSZ(pSignal), pStd[iStepIndex], pVF);
        pFullStd[iStepIndex] = pStd[iStepIndex];
        // Analysis for inside contour
        if (iStepIndex > m_iOut_pix)
        {
          SetMinMax(dAverage, dMinAvInside, dMaxAvInside,
            iStepIndex, iMinAvIndexInside, iMaxAvIndexInside);
        }
        SetMinMax(dAverage, dMinAverage, dMaxAverage,
          iStepIndex, iMinAvIndex, iMaxAvIndex);
        iNMeasured++;
      }
      else
      {
        pFullAver[iStepIndex] = -1.;
        pFullStd[iStepIndex] = pStd[iStepIndex] = -1.;
      }
      cOrtIterator += cOrthoRight ; // go to deeper inside contour for
      iStepIndex++ ;                // scan next line parallel to the edge
    }
    // Signal extracted
    double dAmplOfAver = dMaxAvInside - dMinAvInside;
    double dThresExt = ( dMinAverage + dMaxAverage ) * 0.5;
    int i = 0;
    bool bExtFound = false , bIntFound = false ;
    cmplx cExtCent , cIntCent ;
    // Edges extraction cycle
    if ( iPtOnEdge == iViewIndex )
    {
      cViewBase = FirstCntrst ;
      cViewOrt = -cOrthoRight ;
    }

    for ( ; i < iStepIndex; i++ )
    {
      if ( !bExtFound ) // Find external edge
      {
        if ( pFullAver[ i ] < dThresExt )
        {
          if ( i == 0 )
            break;
          double dAddition = ( pFullAver[ i - 1 ] - dThresExt )
            / ( pFullAver[ i - 1 ] - pFullAver[ i ] );
          cExtCent = FirstCntrst + cOrthoRight * ( i - 1. + dAddition );
          pExternal->Add( CmplxToCDPoint( cExtCent ) );
          bExtFound = true ;
          if ( iNMeasurements < NMAX_INTERVALS )
            pPositions[ iNMeasurements ].m_cExt = cExtCent ;

          while ( (pFullAver[ i + 1 ] < pFullAver[ i ]) )
          {
            if ( ++i >= iStepIndex )
              break ;
          }
          continue ;
        }
      }
      // Find Internal edge
      else
      {
        // initial grad presence analysis: space near edge should be black
        double dDiffThres = dAmplOfAver * m_dDarkEdgeThreshold;
        double dRelToMinThres = dMinAverage + m_dDarkEdgeThreshold * dAmplOfAver * 1.5;
        // First of all - go down
        int iEdge = i;
        double dPos = i ;
        for ( ; i < iStepIndex - 1; i++ )
        {
          if ( pFullAver[ i ] >= dRelToMinThres
            && pFullAver[ i + 1 ] >= dRelToMinThres )
          {
            if ( i - iEdge < 2 ) // too thin grad
              dPos = i - iEdge ;
            else
            {
              double dAddition = ( dRelToMinThres - pFullAver[ i - 1 ] )
                / ( pFullAver[ i ] - pFullAver[ i - 1 ] );
              dPos = i - iEdge + dAddition ;
              ASSERT( dPos >= 0. );
              iNGood++;
            }
            cIntCent = FirstCntrst + cOrthoRight * ( dPos + iEdge ) ;
            pInternal->Add( CmplxToCDPoint( cIntCent ) ) ;
            if ( iNMeasurements < NMAX_INTERVALS )
            {
              pPositions[ iNMeasurements ].m_dPos = abs( cExtCent - cIntCent ) - 0.8 ;
              pPositions[ iNMeasurements ].m_dPos2 = dPos ;
            }
            bIntFound = true ;

            break;
          }
        }
        if ( bIntFound )
          break ;  // From edges extraction cycle
        if ( i >= iStepIndex ) // edge is not found
        {
          cmplx cIntCent = FirstCntrst + cOrthoRight * (double)iStepIndex ;
          pInternal->Add( CmplxToCDPoint( cIntCent ) ) ;
          if ( iNMeasurements < NMAX_INTERVALS )
            pPositions[ iNMeasurements ].m_dPos = iStepIndex;
        }
      }
    }
    // Go to next point on edge
    iPtOnEdge++ ;
    iNextPt++ ;
    iNMeasurements++ ;
  }
  m_dLastGradValue_um = 0. ;
  if ( m_iSimulationMode && (cViewBase.real() != 0.) )
  {
    int iBegin = ( iNMeasurements / 2 ) - 4 ;
    int iEnd = iBegin + 9 ;
    if ( (iBegin >= 0) && (iEnd < iNMeasurements) )
    {
      double dAverage_um = 0. ;
      int iNGood = 0 ;
      for ( int i = iBegin ; i <= iEnd ; i++ )
      {
        PosOnInterval& Sample = pPositions[ i ] ;
        if ( (Sample.m_dPos < (iStepLimit - 2))
          && ( 1.1 <= Sample.m_dPos ) )
        {
          double dBurr_um = ( Sample.m_dPos / m_dScale_pix_per_um );
          dAverage_um += dBurr_um ;
          iNGood++ ;
        }
      }
      if ( iNGood >= 3 )
      {
        m_cGradViewPt = cViewBase + cViewOrt * 40. ;
        m_dLastGradValue_um = dAverage_um / iNGood ;
      }
    }
  }
//   while ( iFirstPt < iLastPt )  // Moisey break
//   {
//     if (!m_pDebugging)
//     {
//       m_pDebugging = CContainerFrame::Create();
//       m_pDebugging->SetLabel(_T("ConturASM-Debug1"));
//     }
//     CFigureFrame * pBeginCross = CreateFigureFrame(
//       &cFirstPt, 1, GetHRTickCount(), (iNMeasurements) ? "0x0000ff" : "0xff0000"); ;
//     m_pDebugging->AddFrame(pBeginCross);
// 
//     if ((i == 0) || (i >= iStepIndex))
//     {
//       pExternal->Add(CmplxToCDPoint(FirstCntrst)); // for bad place marking
//       pInternal->Add(CmplxToCDPoint(cBegin + (cOrthoDir * (double)m_iIn_pix)));
//     }
// 
// //     if (pIntData)
// //     {
// //       cmplx cInitial = pIntData[iIndexOnInternal];
// //       cmplx cAccum(cInitial);
// //       int iIndex = iIndexOnInternal;
// //       int iIndexStep = (bInverse) ? -1 : 1;
// //       cmplx cNext = pIntData[iIndex += iIndexStep];
// //       while (abs(cNext - cInitial) < iStep)
// //       {
// //         cAccum += cNext;
// //         iIndex += iIndexStep;
// //         iIndex = GetRelIndex(iIndex, iIntFigLen);
// //         cNext = pIntData[iIndex];
// //       };
// //       cAccum /= (double)abs(iIndex - iIndexOnInternal);
// //       iIndexOnInternal = iIndex;
// // 
// //       pAdditional->Add(CmplxToCDPoint(cAccum));
// //     }
//     //     else
//     //       pAdditional->Add( CmplxToCDPoint( cAvgStepUp ) ) ;
// 
//     iNMeasurements++;
//     iFirstPt = iEndPt;
//   }
// 
//   int iThr = iNSteps - 2;
// 
//   for (int i = 0; i < iNMeasurements; i++)
//   {
//     PosOnInterval* Pt = pPositions + i;
//     cmplx cIntPos;
//     int iNOK = Pt->m_dPos < iThr;
//     if (iNOK)
//     {
//       if (i <= (iNMeasurements - 1))
//         iNOK += ((Pt + 1)->m_dPos < iThr);
//       if ((iNOK < 2) && (i > 0))
//         iNOK += ((Pt - 1)->m_dPos < iThr);
//       if (iNOK >= 2)
//         cIntPos = Pt->m_cFirst + Pt->m_cOrtho * Pt->m_dPos;
//       else
//       {
//         cIntPos = Pt->m_cFirst + Pt->m_cOrtho * (double)iNSteps;
//         ASSERT(iNSteps >= 0);
//         iNGood--;
//       }
//     }
//     else
//       cIntPos = Pt->m_cFirst + Pt->m_cOrtho * Pt->m_dPos;
// 
//     pInternal->Add(CmplxToCDPoint(cIntPos));
//   }

  return iNMeasurements;
}

static double dFocusViewK = 2.;

int ConturAsm::FindFocusAndBurrByAvg(const CVideoFrame * pVF,
  cmplx * pEdgeData, int iEdgeDataLen, int iPt1Index,
  int iNPtsOnEdge, int iStep, int& iNGood,
  CFigureFrame * pExternal, CFigureFrame * pInternal,
  CFigureFrame * pAdditional, CContainerFrame * pMainOut)
{
  DWORD dwCompr = GetCompression(pVF);
  bool bY8 = (dwCompr == BI_YUV9)
    || (dwCompr == BI_Y8)
    || (dwCompr == BI_Y800)
    || (dwCompr == BI_YUV12);
  if (!bY8 && (dwCompr != BI_Y16))
    return 0;

  int iMaxSignalLen = m_iOut_pix + m_iIn_pix + 5;

  // #define TEMP_ALLOC_SIZE 150
  double pSignal[TEMP_ALLOC_SIZE];
  double pStd[TEMP_ALLOC_SIZE];
  double pFullAver[TEMP_ALLOC_SIZE];
  double pFullStd[TEMP_ALLOC_SIZE];

  // #define NMAX_INTERVALS 150
  //   PosOnInterval pPositions[ NMAX_INTERVALS ] ;  // up to 50 intervals

  m_dLastNextPtContrast = 0.; // Init for correct output in all cases
  int iFigLen = (int)m_pLastExternalContur->GetCount();
  int iFirstPt = iPt1Index;
  cmplx cFirstExt = pEdgeData[iFirstPt];
  int iLastPt = iPt1Index + iNPtsOnEdge - 1;
  cmplx cLastExt = pEdgeData[iLastPt % iEdgeDataLen];
  cmplx cVect = m_cEdgeVect = cLastExt - cFirstExt;
  double dLengthToLast = abs(cVect);
  m_dMaxCrossDist = GetDiffFromStraight(pEdgeData, iFigLen,
    iFirstPt, iLastPt, m_dLengthRatio, &m_dCrossDistStd);
  m_dLastMaxTurn = CheckStraightness(pEdgeData, iFigLen,
    iFirstPt, iLastPt, 10,
    m_dLastMinSegmentLength, m_dLastMaxSegmentLength);
  int iNMeasurements = 0;

  cmplx cCent = m_cMeasFOVCent;
  double dMinDist = DBL_MAX;
  m_dLastContrast = 0.;
  iNGood = 0;
  //int iNIntervals = (iLastPt - iFirstPt) /*/ iStep*/ ; // comment because segment is built from long sections
  int iNSteps = m_iOut_pix + m_iIn_pix;

  m_LastAverageDefocusingValues.RemoveAll();
  m_LastMaxDefocusingValues.RemoveAll();

  MeasuredValues Vals(true);
  int iCentIndex = m_iMinDistIndexForExternal;
  int iIndexToPlus = iFirstPt;
  //   int iScanLimit = iNPtsOnEdge + iStep  ;
  bool bToPLus = true;
  m_iLastMinAverageDefocusing = INT_MAX;
  m_iLastMinMaxDefocusing = INT_MAX;
  m_iLastMaxAverageDefocusing = 0;
  m_iLastMaxMaxDefocusing = 0;
  m_dLastMaxWidth_um = 0.;
  m_cResMiddlePoint = 0.;
  int iNAccounted = 0;
  int iPlusIndex = 0;
  cmplx cEndPt = cFirstExt;
  cmplx cBegin = cFirstExt;
  cmplx cEdgeDir, cPrevEdgeDir;

  double dDistToBegin = 0.;
  int iCurrNextIndex = iIndexToPlus + 1;
  double dScanLimit = dLengthToLast - iStep;
  while ((dDistToBegin = abs(cEndPt - cFirstExt)) < dScanLimit // in pixels
    && iCurrNextIndex <= iLastPt)
  {
    cPrevEdgeDir = cEdgeDir;
    OneMeasPt * ThisResult = &Vals.m_Results[iPlusIndex];
    ThisResult->m_dSampleTime = GetHRTickCount();

    cEndPt = pEdgeData[iCurrNextIndex];
    cmplx cDistToPrev = cEndPt - cBegin;
    double dDistToPrev = abs(cDistToPrev);
    if (dDistToPrev < iStep)
    {
      do
      {
        if (++iCurrNextIndex > iLastPt)
          break;
        cEndPt = pEdgeData[iCurrNextIndex];
        if ((dDistToBegin = abs(cEndPt - cFirstExt)) > dScanLimit)
          break;
        cDistToPrev = cEndPt - cBegin;
        dDistToPrev = abs(cDistToPrev);
      } while (dDistToPrev < iStep);
      if ((dDistToBegin > dScanLimit) || (iCurrNextIndex > iLastPt)) // from main loop
        break;
    };
    if (dDistToPrev > iStep)
    {
      cEndPt = cBegin + (cDistToPrev * (double)iStep / dDistToPrev);
      cDistToPrev = cEndPt - cBegin;
      dDistToPrev = abs(cDistToPrev);
    }
    if (m_Bads[iCurrNextIndex])
    {
      ThisResult->m_dBurrValue_um = -1.;
      ThisResult->m_cEdgePt = cEndPt;
      ThisResult->m_cEdgeDir = cDistToPrev;
      ThisResult->m_dAverFocus = 300;
    }
    if (dDistToPrev == iStep) // very low probability
      iCurrNextIndex++; // we are exactly on next point: we are moving to another

    ThisResult->m_cFirst = cBegin;
    cEdgeDir = ThisResult->m_cEdgeDir = cDistToPrev;
    cmplx cMoveDir = GetNormalized(cEdgeDir);
    cmplx cOrthoDir = ThisResult->m_cOrtho = -GetOrthoRight(cMoveDir); // also normalized
    cBegin = cEndPt; // for next loop

    if (iPlusIndex < LAST_MEAS_VALUES_ARR_SIZE - 1)
      iPlusIndex++;
    else
    {
      SENDWARN("FindFocusAndBurrByAvg Too many points in segment %d", iPlusIndex);
      break;
    }
  }
  cmplx cMeasDir = cEndPt - cFirstExt;
  cmplx cMeasOrtho = -GetOrthoRight(GetNormalized(cMeasDir));
  Vals.m_iPlusIndex = iPlusIndex;
  iPlusIndex = 0;

  while (iPlusIndex < Vals.m_iPlusIndex)
  {
    OneMeasPt * ThisResult = &Vals.m_Results[iPlusIndex];
    bool bTurnIsOK = true;
    if (!iPlusIndex) // first segment in measurement
    {
      if (!IsOneDarkMeasurement() && m_ConturData.GetCount()) // necessary to check angle with last
      {                                   // segment in previous measurement
        double dTurn = GetAbsAngleBtwVects(
          m_ConturData.GetLast().m_cSegmentVect, ThisResult->m_cEdgeDir);
        if (dTurn > m_dSegmentTurnThres)
          bTurnIsOK = false;
      }
    }
    else
    {

      double dTurn = GetAbsAngleBtwVects(ThisResult->m_cEdgeDir,
        Vals.m_Results[iPlusIndex - 1].m_cEdgeDir);
      if (dTurn > m_dSegmentTurnThres)
        bTurnIsOK = false;
    }
    cmplx cFirstPt = ThisResult->m_cFirst;
    cmplx cEdgeDir = ThisResult->m_cEdgeDir;
    cmplx cMoveDir = GetNormalized(cEdgeDir);
    cmplx cOrthoDir = ThisResult->m_cOrtho; // also normalized
    if (!bTurnIsOK)
    {
      if ((iPlusIndex > 0) && ((ThisResult - 1)->m_cOrtho != cMeasOrtho))
        cOrthoDir = ThisResult->m_cOrtho = (ThisResult - 1)->m_cOrtho;
      else
        cOrthoDir = ThisResult->m_cOrtho = cMeasOrtho;
    }

    double dMaxAverage = -DBL_MAX;
    double dMinAverage = DBL_MAX;
    int iMaxAvIndex = 0;
    int iMinAvIndex = 0;
    int iBigStepDownIndex = 0;
    int iIndexForDarkEdgeMeas = 0;
    int iStepIndex = 0;
    double dAvSum = 0.;
    double dAvOfAv = 0.;
    double dAvOfAvOnEdge = 0;
    double dContrast = 0.;
    double dPrev = 0.;
    int iFromOut = -m_iOut_pix;
    // go to left for iFromOut pixels (or out of contour)
    cmplx FirstCntrst = cFirstPt + (((double)iFromOut) * cOrthoDir);
    cmplx cOrtIterator = FirstCntrst;
    if (ThisResult->m_dBurrValue_um == 0.)
    {
      while (iFromOut < 0)
      {
        double dAverage = pFullAver[iStepIndex] = GetAvrgAndStdOnLine(
          cOrtIterator, cMoveDir, abs(cEdgeDir),
          pSignal, ARRSZ(pSignal), pStd[iStepIndex], pVF);
        pFullStd[iStepIndex] = pStd[iStepIndex];
        SetMinMax(dAverage, dMinAverage, dMaxAverage,
          iStepIndex++, iMinAvIndex, iMaxAvIndex);

        iFromOut++;
        cOrtIterator += cOrthoDir; // go for one pt inside, i.e. right
      }
      // Get "signal" - values averaged by parallel to edge lines
      for (int i = 0; i < m_iIn_pix; i++)
      {
        double dAverage = pFullAver[iStepIndex] = GetAvrgAndStdOnLine(
          cFirstPt, cMoveDir, abs(cEdgeDir),
          pSignal, ARRSZ(pSignal), pStd[iStepIndex], pVF);
        pFullStd[iStepIndex] = pStd[iStepIndex];
        cFirstPt += cOrthoDir;
        SetMinMax(dAverage, dMinAverage,
          dMaxAverage, iStepIndex, iMinAvIndex, iMaxAvIndex);

        iStepIndex++;
      }

      double dAmplOfAVer = dMaxAverage - dMinAverage;
      if (dAmplOfAVer < m_dMinimalContrast)
      {
        pMainOut->AddFrame(CreateTextFrame(m_cMeasImageCent,
          "TOO LOW CONTRAST", "0x0000ff", 20));
        //         pMainOut->AddFrame( CreateRectFrame( CRect() )
      }
      double dAmplAfterEdge = dAmplOfAVer;
      double dThresExt = (dMinAverage + dMaxAverage) * 0.5;
      int i = 0;
      // Find edge (white black slope)
      for (; i < iStepIndex; i++)
      {
        if (pFullAver[i] < dThresExt)
        {                  // Black edge is found
          if (i > 0)
          {
            double dAddition = (pFullAver[i - 1] - dThresExt)
              / (pFullAver[i - 1] - pFullAver[i]);
            cmplx cExt = FirstCntrst + cOrthoDir * (i - 1. + dAddition);
            cmplx cExtCent = cExt + cEdgeDir / 2.;

            int iMaxDiffValue = 0, iMaxAbsValue = 0;
            int iAverFocus = GetMaxDiffAndMaxValForPt(
              pVF, cExtCent, iMaxDiffValue, iMaxAbsValue);

            ASSERT(iAverFocus);
            if (iMaxAbsValue)
            {
              iAverFocus = (iAverFocus * (bY8 ? 255 : 65535)) / iMaxAbsValue;
              iMaxDiffValue = (iMaxDiffValue * (bY8 ? 255 : 65535)) / iMaxAbsValue;
            }
            m_LastAverageDefocusingValues.Add(iAverFocus);
            m_LastMaxDefocusingValues.Add(iMaxDiffValue);
            ThisResult->m_cEdgePt = cExtCent;
            ThisResult->m_dAverFocus = iAverFocus;
            ThisResult->m_dMaxFocus = iMaxDiffValue;
            Vals.CheckAndSetFocusMinMax(iAverFocus, iMaxDiffValue, i);
            if (iAverFocus >= m_dMinimalLocalFocus)
              Vals.m_iNWithGoodFocus++;
            //i++ ; // Shift to the next after edge pixel.
          }
          break;
        }
      }
      m_cResMiddlePoint += ThisResult->m_cEdgePt;
      iNAccounted++;
      if ((i == 0) || (i >= iStepIndex))
      {  // edge not found: mark as very wide burr
        ThisResult->m_dPos = m_iIn_pix;
        ThisResult->m_cEdgePt = FirstCntrst;
      }
      else
      {
        // calculate amplitude after edge for depth up to 60 microns
        double dAfterEdgeMin = DBL_MAX, dAfterEdgeMax = -DBL_MAX;
        for (int j = i /*+ 1*/; j < iStepIndex; j++)
          SetMinMax(pFullAver[j], dAfterEdgeMin, dAfterEdgeMax);

        dAmplAfterEdge = dAfterEdgeMax - dAfterEdgeMin;
        // Edge is found, try to find second burr edge

        ThisResult->m_cFirst = FirstCntrst + (cEdgeDir / 2.);
        ThisResult->m_cOrtho = cOrthoDir;
        //       double dDiffThres = dAmplOfAVer * m_dDarkEdgeThreshold ;
        //       double dRelToMinThres = dMinAverage + m_dDarkEdgeThreshold * dAmplOfAVer * 1.5 ;
        double dRelToMinThres = dMinAverage + m_dDarkEdgeThreshold * dAmplAfterEdge;
        // First of all - go down, i.e. find where "signal" going up
        int iEdge = i;
        if ((iMinAvIndex - iEdge <= 6) // minimum is less than ~27 um from edge
          && (dAmplAfterEdge > m_dMinAmplAfterEdge))     // Some contrast we have 
        {
          for (; i < iStepIndex - 1; i++)
          {   // if two samples in "signal" above threshold, it's end of burr
            if (pFullAver[i] >= dRelToMinThres
              && pFullAver[i + 1] >= dRelToMinThres
              && i >= iMinAvIndex - 1)
            {  // burr has thickness more than 2 pixels
              if (i - iEdge < 1.3)
                ThisResult->m_dPos = m_iIn_pix + 10; // mark too thin burr as very wide
              else
              {  // save result
                double dAddition = (dRelToMinThres - pFullAver[i - 1])
                  / (pFullAver[i] - pFullAver[i - 1]);
                double dPos = i - 1. + dAddition;
                ThisResult->m_dPos = dPos;
                if ((dPos - iEdge) < 1.3)
                  ThisResult->m_dPos = m_iIn_pix + 10; // mark too thin burr as very wide
                else
                  iNGood++;
              }
              break;
            }
          }
        }
        else
          i = iStepIndex;
        if (i >= iStepIndex) // edge is not found
          ThisResult->m_dPos = iStepIndex;
      }
      ThisResult->m_cBurrEndPt = ThisResult->m_cFirst
        + (ThisResult->m_cOrtho * ThisResult->m_dPos);
      cmplx cBurrVect = ThisResult->m_cBurrEndPt - ThisResult->m_cEdgePt;
      double dBurrLen_pix = abs(cBurrVect);
      if (dBurrLen_pix <= m_dMaxGradWidth_pix)
      {
        double dBurr_um = ThisResult->m_dBurrValue_um = (dBurrLen_pix / m_dScale_pix_per_um);
        // #ifdef _DEBUG
        //         if (iPlusIndex > 0)
        //         {
        //           OneMeasPt * Prev = ThisResult - 1;
        //           double dPreBurr_um = Prev->m_dBurrValue_um;
        //           if ( ROUND(dPreBurr_um * 100.) - ROUND(dBurr_um * 100.) == 0 )
        //             ASSERT(0);
        //         }
        // #endif
      }
      else
      {    // Burr is not measured
        ThisResult->m_dPos = 60;
        ThisResult->m_cBurrEndPt = ThisResult->m_cFirst
          + (ThisResult->m_cOrtho * ThisResult->m_dPos);
        ThisResult->m_dBurrValue_um = 0.;
      }
    }
    else
    {
      ThisResult->m_dPos = 80;
      ThisResult->m_cBurrEndPt = ThisResult->m_cEdgePt;
      ThisResult->m_dBurrValue_um = 0.;
    }

    ThisResult->m_RobotPos = m_CurrentPos;
    ThisResult->m_RobotPos.m_time = m_CurrentPos.m_theta;

    if (iPlusIndex < LAST_MEAS_VALUES_ARR_SIZE - 1)
      iPlusIndex++;
    else
      ASSERT(0);
    iIndexToPlus += iStep;

    iNMeasurements++;
  }

  m_cResMiddlePoint /= iNAccounted;

  int iThr = iNSteps - 2;
  CFRegression Regr;
  cmplx cMaxPosCrossPos;
  double dMaxWidth = 0.;
  for (int i = Vals.m_iMinusIndex;
    i < Vals.m_iPlusIndex; i++)
  {
    OneMeasPt* Pt = &Vals.m_Results[i];
    double dThisThickness = Pt->m_dPos;
    int iNOK = (dThisThickness < iThr)  // there is something measured
      && (dThisThickness > 0.);

    if (iNOK)
    {
      if ((0 < i) && (i <= (iNMeasurements - 1)))
      {
        double dNextThickness = (Pt + 1)->m_dPos;
        double dPrevThickness = (Pt - 1)->m_dPos;
        if ((dNextThickness > 0.) && (dPrevThickness > 0.))
        {
          double dRatio1 = (dThisThickness - dNextThickness) / dThisThickness;
          double dRatio2 = (dThisThickness - dPrevThickness) / dThisThickness;
          if (dRatio1 >= m_dNeightboursThicknessRatioThres
            || dRatio2 >= m_dNeightboursThicknessRatioThres)
          {
            Pt->m_dPos = (dNextThickness + dPrevThickness) / 2.;
          }
        };
      }
      Pt->m_cBurrEndPt = Pt->m_cFirst + Pt->m_cOrtho * Pt->m_dPos;
      if ((Pt->m_dPos < m_iIn_pix) && (Pt->m_dPos > dMaxWidth))
      {
        cMaxPosCrossPos = Pt->m_cBurrEndPt;
        dMaxWidth = Pt->m_dPos;
      }
    }
    else
    {
      Pt->m_cBurrEndPt = Pt->m_cFirst + Pt->m_cOrtho * (double)iNSteps;
      ASSERT(iNSteps >= 0);
      iNGood--;
    }

    pExternal->Add(CmplxToCDPoint(Pt->m_cEdgePt));
    pInternal->Add(CmplxToCDPoint(Pt->m_cBurrEndPt));
    //     if ( !m_pDebugging )
    //     {
    //       m_pDebugging = CContainerFrame::Create() ;
    //       m_pDebugging->SetLabel( _T( "ConturASM-Debug3" ) ) ;
    //     }
    DWORD dwColor = (Pt->m_dAverFocus >= m_dMinimalLocalFocus) ? 0x000000ff : 0x00ff0000;
    double dNormCorr = (1. - (Pt->m_dAverFocus / m_iLastMaxAverageDefocusing));
    int iColorCorrection = ROUND(255. * dFocusViewK * dNormCorr);
    if (iColorCorrection > 255)
      iColorCorrection = 255;
    dwColor += (iColorCorrection << 8);
    CFigureFrame * pBeginCross = CreateFigureFrame(
      &Pt->m_cEdgePt, 1, dwColor);
    pMainOut->AddFrame(pBeginCross);
    Regr.Add(i, Pt->m_dAverFocus);
  }
  if (iNMeasurements)
  {
    Regr.Calculate();
    m_dLastFocusRegressionA = Regr.m_da;
    m_dLastFocusRegressionB = Regr.m_db;

    if (dMaxWidth > 0.)
    {
      CFigureFrame * pMaxCross = CreateFigureFrame(
        &cMaxPosCrossPos, 1, (DWORD)0x0000ff);
      pMainOut->AddFrame(pMaxCross);

    }
  }
  else
    m_dLastFocusRegressionA = m_dLastFocusRegressionB = 0.;

  if (Vals.m_iPlusIndex > Vals.m_iMinusIndex + 6)
  {
    int iStartIndex = Vals.m_iMinusIndex + 1;
    GetAvValues(&Vals.m_Results[iStartIndex],
      3, Vals.m_FV.m_dLastAvFocusNearBegin, Vals.m_FV.m_dLastMaxFocusNearBegin);
    //iStartIndex = (LAST_MEAS_VALUES_ARR_SIZE / 2) - 1 ;
    iStartIndex = (Vals.m_iPlusIndex / 2) - 1;
    GetAvValues(&Vals.m_Results[iStartIndex],
      3, Vals.m_FV.m_dLastAvFocusInCenter, Vals.m_FV.m_dLastMaxFocusInCenter);
    iStartIndex = Vals.m_iPlusIndex - 3;
    GetAvValues(&Vals.m_Results[iStartIndex],
      3, Vals.m_FV.m_dLastAvFocusNearEnd, Vals.m_FV.m_dLastMaxFocusNearEnd);
  }

  Vals.m_RobotPos = m_CurrentPos;
  Vals.m_RobotPos.m_time = m_CurrentPos.m_theta;
  Vals.m_dLastZStep = m_dLastdZ;
  //  ASSERT( m_ViewsForPosition.GetCount() == m_DataForPosition.GetCount() ) ;

  m_DataForPosition.Add(Vals);
  return iNMeasurements;
}

// Return is true, if not necessary to continue focusing (i.e. ~all 
// segments were in focus
bool ConturAsm::AnalyzeLastMeasByAvg(int& iNGood, // N good segments in this measurement
  int& iFirstGoodIndex, // index of first good segment in this measurement
  int& iLastGoodIndex) // index of last good segment in this measurement
{

  if (m_DataForPosition.GetCount() == 0)
    return false;

  // Get last obtained set of data
  MeasuredValues& Vals = m_DataForPosition.GetLast();
  int iNIntervals = Vals.m_iPlusIndex - Vals.m_iMinusIndex;
  iNGood = Vals.m_iNWithGoodFocus;
  iFirstGoodIndex = Vals.m_iLeftMeasured;
  iLastGoodIndex = Vals.m_iRightMeasured;
  if (((m_DataForPosition.GetCount() > 15)
    && (m_CurrentViewMeasStatus.m_iNWithGoodFocus > (iNIntervals / 2)))
    || (m_DataForPosition.GetCount() > 20))
  {
    iNGood = m_CurrentViewMeasStatus.m_iNWithGoodFocus;
    iFirstGoodIndex = m_CurrentViewMeasStatus.m_iLeftMeasured;
    iLastGoodIndex = m_CurrentViewMeasStatus.m_iRightMeasured;
    return true;
  }



  if (m_CurrentViewMeasStatus.m_iLeftMeasured == m_CurrentViewMeasStatus.m_iMinusIndex
    && m_CurrentViewMeasStatus.m_iRightMeasured >= m_CurrentViewMeasStatus.m_iPlusIndex - 2)
    //   {
    //   }
    //   if ( iCheckPointIndex >= m_CurrentViewMeasStatus.m_iMinusIndex
    //     && iNGood >= iPtNum - m_CurrentViewMeasStatus.m_iMinusIndex  - 1 )
  {
    return true;
  }
  double dMinMeasuredZ = DBL_MAX, dMaxMeasuredZ = -DBL_MAX;
  double dAvMeasuredZ = 0;
  int iNSamples = 0;
  if (m_CurrentViewMeasStatus.m_iNWithGoodFocus)
  {
    for (int i = m_CurrentViewMeasStatus.m_iMinusIndex;
      i < m_CurrentViewMeasStatus.m_iPlusIndex; i++)
    {
      if (m_CurrentViewMeasStatus.m_Results[i].m_dAverFocus >= m_dMinimalLocalFocus)
      {
        double dZ = m_CurrentViewMeasStatus.m_Results[i].m_RobotPos.m_z;
        dAvMeasuredZ += dZ;
        SetMinMax(dZ, dMinMeasuredZ, dMaxMeasuredZ);
        iNSamples++;
      }
    }
    ASSERT(iNSamples == m_CurrentViewMeasStatus.m_iNWithGoodFocus);
    dAvMeasuredZ /= iNSamples;
    if (dMinMeasuredZ < m_dMinZ - 200
      || dMaxMeasuredZ > m_dMaxZ + 200)
    {
      return true;
    }
  }
  else
  {
    if (m_dMaxZ - m_dMinZ > 600)
      return true; // no focus, nothing to measure
  }

  return false;
}

int ConturAsm::GetFocusValueForPt(
  const CVideoFrame * pVF, cmplx& Pt, int& iMaxFocusValue)
{
  CPoint FocusCent(ROUND(Pt.real()), ROUND(Pt.imag()));
  CRect FocusEstimationRect(FocusCent.x - 1, FocusCent.y - 1,
    FocusCent.x + 1, FocusCent.y + 1);
  iMaxFocusValue = 0;
  return _get_ave_diff_for_rectangle(pVF, FocusEstimationRect, iMaxFocusValue);
}

int ConturAsm::GetMaxDiffAndMaxValForPt(
  const CVideoFrame * pVF, cmplx& Pt, int& iMaxDiffVal, int& iMaxVal)
{
  CPoint FocusCent(ROUND(Pt.real()), ROUND(Pt.imag()));
  CRect FocusEstimationRect(FocusCent.x - 1, FocusCent.y - 1,
    FocusCent.x + 1, FocusCent.y + 1);
  iMaxDiffVal = iMaxVal = 0;
  return _get_ave_diff_and_check_max_for_rectangle(
    pVF, FocusEstimationRect, iMaxDiffVal, iMaxVal);
}

// Function looking for points in Data with bad focus and returns first with minimum 3 following points with bad focus
int ConturAsm::GetLastIndexWithGoodFocus(MeasuredValues& Data, int iFirstIndex)
{
  for (int i = iFirstIndex; i < Data.m_iPlusIndex; i++)
  {
    if (Data.m_Results[i].m_dAverFocus >= m_dMinimalLocalFocus)
      continue;
    else
    {
      if (i < Data.m_iPlusIndex - 1)
      {
        if (Data.m_Results[i + 1].m_dAverFocus >= m_dMinimalLocalFocus)
          continue;
        if (i < Data.m_iPlusIndex - 2)
        {
          if (Data.m_Results[i + 2].m_dAverFocus >= m_dMinimalLocalFocus)
            continue;
        }
      }
    }
  }
  return 0;
}

// return value is Z step for next measurement
// if result is zero, then we did measure points with good enough focus
// and it's necessary to do more deep analysis

double ConturAsm::GetFocusingStepForFarFromFocus()
{
  double dNextStep = 0.0;

  if (!m_CurrentViewMeasStatus.m_iNWithGoodFocus) // no saved good focused points
  {
    MeasuredValues& LastResult = m_DataForPosition.GetLast();
    if (LastResult.m_iNWithGoodFocus)
      return 0.;

    if (m_DataForPosition.GetCount() == 1)
      dNextStep = 2. * m_iFocusStep_um;
    else
    {
      double dCurrentFocus = LastResult.m_dAvFocus;
      double dBestFocus = m_DataForPosition[0].m_dAvFocus;
      int iIndex = 0;
      int i = 1;
      for (; i < m_DataForPosition.GetCount(); i++)
      {
        if (m_DataForPosition[i].m_dAvFocus > dBestFocus)
        {
          dBestFocus = m_DataForPosition[i].m_dAvFocus;
          iIndex = i;
        }
      }
      if (abs(dCurrentFocus - dBestFocus) > dBestFocus * 0.07)
      {
        double dDiffZToBest = m_CurrentPos.m_z - m_DataForPosition[iIndex].m_RobotPos.m_z;
        dNextStep = -dDiffZToBest
          + ((dDiffZToBest > 0) ? -m_iFocusStep_um : m_iFocusStep_um);
      }
      else
      {
        dNextStep = (m_dLastdZ > 0) ? m_iFocusStep_um : -m_iFocusStep_um;
      }
    }
  }
  return dNextStep;
}


// Function CheckPtForFocusAndGetDirection does test 
// for focus in the iCheckPointIndex point of m_DataForPosition array
// Should be used only after first image capturing
// Function does use m_DataForPosition array
//   Checks focus values in the iCheckPointIndex point and 3 points after (average), 
//   compares these values for different Zs and decides
//   about focus improvement process
// Return values: 
// FS_OK = 0 ,OK, it seems optimum
// FS_ContinueFocusing continue to move into the same direction
// FS_TakeValuesFromPrevious - previous position was the best, take values from it
//
// FS_Error - something strange

FocusState ConturAsm::CheckPtForFocusAndGetDirection(
  double& dNextStep, int iCheckPointIndex, int& iLastGoodIndex)
{
  double dMaxSavedAvValue = -DBL_MAX;
  AlignedByZPtrs Aligned;
  bool bCurrentViewFirstPass = (m_CurrentViewMeasStatus.m_iLeftMeasured == -1);
  int iLeftMeasuredBefore = m_CurrentViewMeasStatus.m_iLeftMeasured;
  int iRightMeasuredBefore = m_CurrentViewMeasStatus.m_iRightMeasured;
  SetMinMax(m_CurrentPos.m_z, m_dMinZ, m_dMaxZ);
  FXSIZE iNSaved = m_DataForPosition.GetCount();

  // Add measured points to accumulator (m_CurrentViewMeasStatus)
  // It's necessary to do for last measurement only

  MeasuredValues& MeasVals = m_DataForPosition.GetLast();
  int iNPts = MeasVals.m_iPlusIndex - MeasVals.m_iMinusIndex + 1;
  int iNNewWithGoodFocus = 0;
  for (int iPtNum = MeasVals.m_iMinusIndex; iPtNum < MeasVals.m_iPlusIndex; iPtNum++)
  {
    OneMeasPt& SavedResult = m_CurrentViewMeasStatus.m_Results[iPtNum];
    OneMeasPt& MeasResult = MeasVals.m_Results[iPtNum];
    if (MeasResult.m_dAverFocus >= m_dMinimalLocalFocus)
    {
      if (MeasVals.m_iLeftMeasured == -1)
        MeasVals.m_iLeftMeasured = iPtNum;
      MeasVals.m_iRightMeasured = iPtNum;
      iNNewWithGoodFocus++;
    }
    if (SavedResult.m_dAverFocus < MeasResult.m_dAverFocus)
    {
      if (m_CurrentViewMeasStatus.m_RobotPos.m_x == 0.)
      {
        m_CurrentViewMeasStatus.m_RobotPos = m_CurrentPos;
        m_CurrentViewMeasStatus.m_RobotPos.m_time = m_CurrentPos.m_theta;
      }
      //       if ( MeasResult.m_dAverFocus > m_dMinimalLocalFocus )
      //         MeasVals.m_iNNewWithGoodFocus++ ;
      memcpy(&SavedResult, &MeasResult, sizeof(OneMeasPt));
      if (dMaxSavedAvValue < SavedResult.m_dAverFocus)
      {
        dMaxSavedAvValue = SavedResult.m_dAverFocus;
        m_CurrentViewMeasStatus.m_iMaxAvIndex = iPtNum;
        m_CurrentViewMeasStatus.m_iMaxAvMeasNumber = (int)m_DataForPosition.GetUpperBound();
      }
      if (m_CurrentViewMeasStatus.m_iMinusIndex > iPtNum)
        m_CurrentViewMeasStatus.m_iMinusIndex = iPtNum;
      if (m_CurrentViewMeasStatus.m_iPlusIndex < iPtNum + 1)
        m_CurrentViewMeasStatus.m_iPlusIndex = iPtNum + 1;
    }
  }
  MeasVals.m_iNNewWithGoodFocus = iNNewWithGoodFocus;
  if (m_CurrentViewMeasStatus.m_iLeftMeasured == -1)
    m_CurrentViewMeasStatus.m_iLeftMeasured = MeasVals.m_iLeftMeasured;
  else if (m_CurrentViewMeasStatus.m_iLeftMeasured > MeasVals.m_iLeftMeasured)
    m_CurrentViewMeasStatus.m_iLeftMeasured = MeasVals.m_iLeftMeasured;
  if (m_CurrentViewMeasStatus.m_iRightMeasured == -1)
    m_CurrentViewMeasStatus.m_iRightMeasured = MeasVals.m_iRightMeasured;
  else if (m_CurrentViewMeasStatus.m_iRightMeasured < MeasVals.m_iRightMeasured)
    m_CurrentViewMeasStatus.m_iRightMeasured = MeasVals.m_iRightMeasured;
  m_CurrentViewMeasStatus.m_FV.UpdateBestFocusValues(MeasVals.m_FV);

  if (!m_CurrentViewMeasStatus.m_iNWithGoodFocus)
  {  // first Z where something was measured 
    m_dMinZWithGood = m_CurrentPos.m_z;
    m_dMaxZWithGood = m_CurrentPos.m_z;
  }
  int iNGood = 0;
  int iPtNum = m_CurrentViewMeasStatus.m_iMinusIndex;
  for (;
    iPtNum < m_CurrentViewMeasStatus.m_iPlusIndex; iPtNum++)
  {
    OneMeasPt& SavedResult = m_CurrentViewMeasStatus.m_Results[iPtNum];
    if (SavedResult.m_dAverFocus >= m_dMinimalLocalFocus)
      iNGood++;
  }

  MeasVals.m_iNWithGoodFocusOnXYPosition = iNGood;
  m_CurrentViewMeasStatus.GetGoodValues(
    m_dMinimalLocalFocus, iCheckPointIndex, iLastGoodIndex);


  int iMeasuredRange = m_CurrentViewMeasStatus.m_iPlusIndex
    - m_CurrentViewMeasStatus.m_iMinusIndex;
  int iFocusedRange = m_CurrentViewMeasStatus.m_iRightMeasured
    - m_CurrentViewMeasStatus.m_iLeftMeasured;
  if (m_CurrentViewMeasStatus.m_iNWithGoodFocus >= iMeasuredRange * 0.8
    /*|| iFocusedRange >= iMeasuredRange * 0.9*/)
    return FS_OK;

  if (MeasVals.m_iNNewWithGoodFocus)
  {  // If there is pts with better focus in last measurement, continue
    SetMinMax(m_CurrentPos.m_z, m_dMinZWithGood, m_dMaxZWithGood);
    dNextStep = (m_dLastdZ > 0.) ? m_iFocusStep_um : -m_iFocusStep_um;
    if (MeasVals.m_iNNewWithGoodFocus >= m_CurrentViewMeasStatus.m_iNWithGoodFocus)
      m_dLastOptimalZ = m_CurrentPos.m_z;
    return FS_ContinueFocusing;
  }


  // Finished, what is in the center not important
  if (((m_CurrentViewMeasStatus.m_iLeftMeasured == m_CurrentViewMeasStatus.m_iMinusIndex)
    && (m_CurrentViewMeasStatus.m_iRightMeasured >= MeasVals.m_iPlusIndex - 2)))
  {
    return FS_OK;
  }
  // Check and go down
  if (m_dMinZWithGood - m_dMinZ < 80.)
  {
    double dZTarget = m_dMinZ - m_iFocusStep_um;
    dNextStep = dZTarget - m_CurrentPos.m_z;
    return FS_ContinueFocusing;
  }
  // check and go up
  if (m_dMaxZ - m_dMaxZWithGood < 80.)
  {
    double dZTarget = m_dMaxZ + m_iFocusStep_um;
    dNextStep = dZTarget - m_CurrentPos.m_z;
    return FS_ContinueFocusing;
  }
  iLastGoodIndex = m_CurrentViewMeasStatus.m_iRightMeasured;

  return FS_OK;
}


// Function compare measured focuses for different Z 
// for assigned point and provide Z step value for focusing
// If result is zero - there is no possibility for focus improve
double ConturAsm::GetZStepForLocalPoint(int iPtNumber)
{
  if (m_DataForPosition.GetCount() >= 2)
  {
    int iMaxIndex = (int)m_DataForPosition.GetUpperBound();
    MeasuredValues& Last = m_DataForPosition.GetAt(iMaxIndex);
    MeasuredValues& Prev = m_DataForPosition.GetAt(iMaxIndex - 1);

    OneMeasPt& LastPtResult = Last.m_Results[iPtNumber];
    OneMeasPt& PrevPtResult = Prev.m_Results[iPtNumber];
    double dFocusDiff = LastPtResult.m_dAverFocus - PrevPtResult.m_dAverFocus;

    return (dFocusDiff >= 0.) ? Last.m_dLastZStep : -Last.m_dLastZStep;
  }

  return 0.;
}

// Function compare measured focuses for different Z 
// for assigned point and provide Z step value for focusing
// If result is zero - there is no possibility for focus improve
double ConturAsm::GetZStepFromHistory()
{
  if (m_ConturData.GetCount() == 0)  // first 
    return (double)m_iFocusStep_um;

  ConturSample& LastSample = m_ConturData.GetLast();
  double dZ = m_CurrentPos.m_z - LastSample.m_RobotPos.m_z;
  if (abs(dZ) > 10.)
    return (double)((dZ > 0.) ? m_iFocusStep_um : -m_iFocusStep_um);
  else
    return (double)m_iFocusStep_um;
}

int ConturAsm::FindWallEdges(const CVideoFrame * pVF,
  cmplx * pEdgeData, int iEdgeDataLen, int iPt1Index,
  int iNPtsOnEdge, double dRealLightDirForExternal,
  CFigureFrame * pExternal, CFigureFrame * pInternal,
  CFigureFrame * pShadow, CFigureFrame * pDirMarking)
{
  int iMaxSignalLen = m_iOut_pix + m_iIn_pix + 5;
#ifdef _DEBUG
  double pSignal[1000];
#else
  double * pSignal = new double[iMaxSignalLen];
#endif

  double dDirAngle = dRealLightDirForExternal; // get direction angle
  cmplx cStepPlus = polar(1., -dDirAngle); // one distance unit to the inside part
  cmplx cStepMinus = -cStepPlus;           // one distance unit to the outside part
  cmplx cSignalLine = (double)(m_iOut_pix + m_iIn_pix) * cStepPlus;
  cmplx cDistToMinus = (double)(m_iOut_pix)* cStepMinus;
  cmplx cDistToPlus = (double)(m_iIn_pix)* cStepPlus;
  double d3Edges[3];
  int iNShadowContinuity = 0, iNWallContinuity = 0;
  int iNVisiblePointsOnWall = 0;
  for (int i = 0; i < iNPtsOnEdge; i++)
  {
    // take next point
    cmplx Pt = GetRelElem(pEdgeData, iPt1Index, i, iEdgeDataLen);
    cmplx BeginPoint = Pt + cDistToMinus;
    cmplx EndPoint = Pt + cDistToPlus;
    if (i == 0 && pDirMarking)
    {
      pDirMarking->AddPoint(CmplxToCDPoint(BeginPoint));
      pDirMarking->AddPoint(CmplxToCDPoint(EndPoint));
    }

    GetPixelsOnLine(BeginPoint, EndPoint, pSignal, iMaxSignalLen, pVF);
    int iNEdges = Find3Edges(pSignal, m_iOut_pix + m_iIn_pix, d3Edges);
    switch (iNEdges)
    {
    case 3:
      {
        cmplx Pt3 = BeginPoint + cStepPlus * d3Edges[2];
        iNShadowContinuity++;
        iNVisiblePointsOnWall++;
        if (pShadow)
          pShadow->Add(CmplxToCDPoint(Pt3));
      }
    case 2:
      {
        cmplx Pt2 = BeginPoint + cStepPlus * d3Edges[1];
        iNWallContinuity++;
        if (pInternal)
          pInternal->Add(CmplxToCDPoint(Pt2));
      }
    case 1:
      {
        cmplx Pt1 = BeginPoint + cStepPlus * d3Edges[0];
        if (pExternal)
          pExternal->Add(CmplxToCDPoint( /*Pt1*/ Pt));
      }
      break;
    }
    if (iNEdges < 3)
    {
      if (pShadow)
        pShadow->Add(CmplxToCDPoint(Pt));
      if (iNShadowContinuity < m_iMinimalContinuty
        && pShadow && pExternal)
      {
        iNVisiblePointsOnWall -= iNShadowContinuity;
        int iIndex = (int)pShadow->GetUpperBound() - 1;
        while (iNShadowContinuity > 0)
        {
          pShadow->SetAt(iIndex, pExternal->GetAt(iIndex));
          iIndex--;
          iNShadowContinuity--;
        };
      }
      if (iNEdges < 2)
      {
        if (pInternal)
          pInternal->Add(CmplxToCDPoint(Pt));
        if (iNWallContinuity < m_iMinimalContinuty
          && pInternal && pExternal)
        {
          int iIndex = (int)pInternal->GetUpperBound() - 1;
          while (iNWallContinuity > 0)
          {
            pInternal->SetAt(iIndex, pExternal->GetAt(iIndex));
            iIndex--;
            iNWallContinuity--;
          };
        }
        if (iNEdges < 1)
        {
          if (pExternal)
            pExternal->Add(CmplxToCDPoint(Pt));
        }
      }
      iNShadowContinuity = iNWallContinuity = 0;
    }
  }
#ifndef _DEBUG
  delete pSignal;
#endif

  return iNVisiblePointsOnWall;
}

int ConturAsm::Find3Edges(double * pSignal, int iSignalLength, double d3edges[3])
{
  double * pPix = pSignal + iSignalLength - 1;
  double dMin = *pPix, dMax = *pPix;
  double dAverageFar = 0;
  while (--pPix >= pSignal + m_iOut_pix + m_iIn_pix - 6)
  {
    double dVal = *pPix;
    SetMinMax(dVal, dMin, dMax);
    dAverageFar += dVal;
  }
  dAverageFar /= 5.;
  double dFarThres = dAverageFar * 1.2;
  while ((*(--pPix) < dFarThres) && (pPix > pSignal + m_iOut_pix))
  {
    double dVal = *pPix;
    SetMinMax(dVal, dMin, dMax);
  }
  double dOutMax = dMax;
  while (--pPix >= pSignal)
  {
    double dVal = *pPix;
    SetMinMax(dVal, dMin, dOutMax);
  }

  double dAmpl = dMax - dMin;
  if (dAmpl < m_iMinShadowAmpl)
  {
    memset(d3edges, 0, sizeof(d3edges));
    return 0;
  }

  double dThicknessThres = dMin + m_dThicknessThreshold * dAmpl;
  double dShadowThres = dMin + m_dShadowThres * dAmpl;
  double dUpperThres = dMax + (dOutMax - dMax) * m_dUpperThreshold;
  // find edge
  pPix = pSignal;
  double * pEnd = pSignal + iSignalLength;
  while ((*pPix > dUpperThres) && (pPix < pEnd))
    pPix++;
  if (pPix == pSignal)
    d3edges[0] = 0.;
  else
  {
    if (pPix == pEnd) // all white, no edge
    {
      d3edges[0] = d3edges[1] = d3edges[2] = 0.; // (double) iSignalLength ;
      return 0;
    }
    // edge found, calculate precise position
    double dPos = GetThresPosition(*(pPix - 1), *(pPix), dUpperThres);
    d3edges[0] = pPix - pSignal + dPos; // save position
  }

  if (!IsDarkMode())
  {
    while ((*pPix >= dThicknessThres) && (pPix < pEnd))
      pPix++;
    if ((pPix < pEnd) && (*pPix < dThicknessThres))
    { // black zone found
      double dPos = GetThresPosition(*(pPix - 1), *(pPix), dThicknessThres);
      if ((pPix - pSignal) - 1 >= 0)
        d3edges[1] = pPix - pSignal + dPos - 1.; // save shadow begin position 
                                                    // go to find beginning of black zone (shadow)
      else
        d3edges[1] = 0.;

      while ((*pPix > dShadowThres) && (pPix < pEnd))
        pPix++;
      if ((pPix >= pEnd) && (*pPix > dShadowThres))
      {
        d3edges[2] = d3edges[1];
        return 2;
      }
      while ((*pPix <= dShadowThres) && (pPix < pEnd))
        pPix++;
      if ((pPix < pEnd) && (*pPix > dShadowThres))
      { // black zone found
        double dPos = GetThresPosition(*(pPix - 1), *(pPix), dShadowThres);
        if ((pPix - pSignal) - 1 >= 0)
          d3edges[2] = pPix - pSignal + dPos - 1.; // save shadow begin position 
                                                      // go to find beginning of black zone (shadow)
        else
          d3edges[2] = 0.;
        return 3; // all 3 points found
      }
      // third point is not found (too wide shadow)
      d3edges[2] = iSignalLength;
      return 3; // only wall width is found (from first edge to shadow begin)
    }
  }
  d3edges[2] = d3edges[1] = 0.;

  // go to find beginning of black zone (shadow)
  return 1; // only part edge is found, no wall (burr) 
}

bool ConturAsm::ScanProperties(LPCTSTR text, bool& Invalidate)
{
  CFilterGadget::ScanProperties(text, Invalidate);
  bool bSystemPar = false;
  FXPropKit2 pk(text);
  int iNChanged = (pk.GetInt(_T("WorkingMode"), (int&)m_WorkingMode) != false);
  Invalidate |= (iNChanged != 0);
  iNChanged += (pk.GetInt("Dir", (int&)m_iDir) != false);
  iNChanged += (pk.GetInt("MeasStep", m_iMeasStep) != false);
  iNChanged += (pk.GetInt("MeasZoneWidth", m_iMeasZoneWidth) != false);
  iNChanged += (pk.GetInt("MinEdgeContinuty", m_iMinimalContinuty) != false);
  iNChanged += (pk.GetInt("SegmentLen", m_iSegmentLength) != false);
  iNChanged += (pk.GetDouble("EdgeViewScale", m_dEdgeViewScale) != false);
  //FXString ThresholdsAsString ;
  //pk.GetString( "Thresholds" , ThresholdsAsString );
  iNChanged += (pk.GetInt("MinShadowAmpl", m_iMinShadowAmpl) != false);

  int iEquPos = -1;
  int iIndex = pk.GetIndexForName(_T("Thresholds"), iEquPos);
  if (iIndex >= -1)
  {
    if (iIndex >= 0)
    {
      if (iIndex < 3 && iEquPos > 0)
      {
        iNChanged++;
        double dVal = atof(((LPCTSTR)pk) + iEquPos + 1);
        switch (iIndex)
        {
        case 2: m_dShadowThres = dVal; break;
        case 1: m_dThicknessThreshold = dVal; break;
        case 0: m_dUpperThreshold = dVal; break;
        }
      }
      else
      {
        SEND_GADGET_ERR(_T("Not correct index %d in Thresholds"),
          iIndex);
      }
    }
    else
    {
      FXDblArray Data;
      int iNItems = GetArray((FXPropertyKit&)pk,
        "Thresholds", _T('f'), &Data);
      iNChanged += (iNItems != 0);
      switch (iNItems)
      {
      case 3:
        m_dShadowThres = Data[2];
      case 2:
        m_dThicknessThreshold = Data[1];
      case 1:
        m_dUpperThreshold = Data[0];
        break;
      default:
        if (iNItems > 3)
        {
          SEND_GADGET_ERR(_T("Not correct number of Thresholds: %d"),
            Data.GetCount());
        }
        break;
      }
    }

  }
  iNChanged += (pk.GetDouble("StdThreshold", m_dStdThreshold) != false);
  iNChanged += (pk.GetDouble("DarkEdgeThres", m_dDarkEdgeThreshold) != false);
  iNChanged += (pk.GetDouble("WidthViewScale", m_dWidthViewScale) != false);
  iNChanged += (pk.GetInt("ViewLength", m_iViewLength) != false);
  if (pk.GetInt(_T("ResultView"), (int&)m_ResultViewMode))
  {
    iNChanged++;
    if (IsInactive() && m_ConturData.GetCount())
    {
      CDataFrame * pMapView = FormMapImage(NULL, m_ResultViewMode);
      if (pMapView)
        PutAndRegisterFrame(m_pMarking, pMapView);
    }
  }
  pk.GetInt(_T("LightDir"), (int&)m_LightDir);
  pk.GetInt(_T("NActiveLeds"), m_iNLeds);
  int iNItems = GetArray(pk, "Maximums", _T('f'),
    (int)m_dMaximumsForViewCsale.GetCount(), m_dMaximumsForViewCsale.GetData());
  iNChanged += (iNItems != 0);
  if (pk.GetInt("ExtLight_us", m_iExtLight_us))
  {
    iNChanged++;
    for (size_t i = 0; i < 8; i++)
      m_iLightForLED_us[i] = m_iExtLight_us;
  }
  iNItems = GetArray(pk, "LightForLEDs", 'd', 8, m_iLightForLED_us);
  iNChanged += (pk.GetInt("IntLight_us", m_iIntLight_us) != false);
  iNChanged += (pk.GetInt("LightForDark_us", m_iLightForDark_us) != false);
  iNChanged += (pk.GetInt("Outside_pix", m_iOut_pix) != false);
  iNChanged += (pk.GetInt("Inside_pix", m_iIn_pix) != false);
  iNChanged += (pk.GetInt("StopOnFault", m_iStopOnFault) != false);
  iNChanged += (pk.GetInt("MeasExp_us", m_iMeasurementExposure_us) != false);
  iNChanged += (pk.GetInt("ObsMeasExp_us", m_iObservationMeasExp_us) != false);
  iNChanged += (pk.GetInt("ObsFocusExp_us", m_iObservationFocusExp_us) != false);
  iNChanged += (pk.GetDouble("MinLocalFocus", m_dMinimalLocalFocus) != false);
  iNChanged += (pk.GetInt("FocusFall_perc", m_iFocusFall_perc) != false);
  iNChanged += (pk.GetDouble("KDefocusToHeight", m_dK_DefocusTodZ) != false);
  iNChanged += (pk.GetInt("FocusStep_um", m_iFocusStep_um) != false);
  iNChanged += (pk.GetInt("ScanAccel_un", m_iAccel_units) != false);
  iNChanged += (pk.GetInt("FocusingPeriod", m_iFocusingPeriod) != false);

  iEquPos = -1;
  iIndex = pk.GetIndexForName(_T("FocusingParams"), iEquPos);
  if (iIndex >= -1)
  {
    if (iIndex >= 0)
    {
      if (iIndex < 7 && iEquPos > 0)
      {
        iNChanged++;
        int iVal = atoi(((LPCTSTR)pk) + iEquPos + 1);
        switch (iIndex)
        {
        case 0: m_iFocusAreaSize_pix = iVal; break;
        case 1: m_iFocusDistFromEdge_pix = iVal; break;
        case 2: m_iObservFocusRange_um = iVal; break;
        case 3: m_iMeasInitialFocusRange_um = iVal; break;
        case 4: m_MeasFocusRange_um = iVal; break;
        case 5: m_iFocusWorkingMode = iVal; break;
        case 6: m_dFocusingCorrection_um = iVal; break;
        }
      }
      else
      {
        SEND_GADGET_ERR(_T("Not correct index %d in Thresholds"),
          iIndex);
      }
    }
    else
    {
      FXIntArray FocusParams;
      iNItems = GetArray((FXPropertyKit&)pk,
        "FocusingParams", _T('d'), &FocusParams);
      iNChanged += (iNItems != 0);
      if (iNItems >= 6)
      {
        m_iFocusAreaSize_pix = FocusParams[0];
        m_iFocusDistFromEdge_pix = FocusParams[1];
        m_iObservFocusRange_um = FocusParams[2];
        m_iMeasInitialFocusRange_um = FocusParams[3];
        m_MeasFocusRange_um = FocusParams[4];
        m_iFocusWorkingMode = FocusParams[5];
        if (iNItems >= 7)
          m_dFocusingCorrection_um = FocusParams[6];
      }
    }
  }

  iEquPos = -1;
  iIndex = pk.GetIndexForName(_T("FocusIndication"), iEquPos);
  if (iIndex >= -1)
  {
    if (iIndex >= 0)
    {
      if (iIndex < 3 && iEquPos > 0)
      {
        iNChanged++;
        double dVal = atof(((LPCTSTR)pk) + iEquPos + 1);
        switch (iIndex)
        {
        case 2: m_dMinimalContrast = dVal; break;
        case 1: m_dFocusRangeK = dVal; break;
        case 0: m_dFocusDecreaseNorm = dVal; break;
        }
      }
      else
      {
        SEND_GADGET_ERR(_T("Not correct index %d in Thresholds"),
          iIndex);
      }
    }
    else
    {
      FXDblArray FocusIndicationParams;
      iNItems = GetArray(pk, "FocusIndication", _T('f'), &FocusIndicationParams);
      if (iNItems >= 2)
      {
        m_dFocusDecreaseNorm = FocusIndicationParams[0];
        m_dFocusRangeK = FocusIndicationParams[1];
        if (iNItems >= 3)
          m_dMinimalContrast = FocusIndicationParams[2];
      }
    }
  }
  if ( pk.GetDouble( "PartHeight" , m_dPartHeight ) )
  {
    iNChanged++ ;
    ASSERT( m_dPartHeight < 40000 ) ;
  }
  if (pk.GetString("InitialDir", m_sInitialDir))  // !!! direction, not directory
  {
    bSystemPar = true;
    m_InitialDir = DecodeDirName(m_sInitialDir);
  }
  bSystemPar |= pk.GetDouble("MeasScale_pix_per_um", m_dScale_pix_per_um);
  bSystemPar |= pk.GetDouble("ObservScale_pix_per_um", m_dObservScale_pix_per_um);
  bSystemPar |= pk.GetDouble("BaseHeight", m_dBaseHeight);
  bSystemPar |= pk.GetDouble("AdapterHeight", m_dAdapterHeight);
  bSystemPar |= pk.GetCmplx("MeasRange", m_cMeasRange);
  if ( pk.GetCmplx("PartPlacePos", m_cPartPlacementPos) )
  {
    m_PlacementZone.top = m_cPartPlacementPos.imag() - 200. ;
    m_PlacementZone.bottom = m_cPartPlacementPos.imag() + 200. ;
    bSystemPar = true;
  }
  if (pk.Get3d("ObservCent", (void*)&m_ccObservCenter, 'f'))
  {
    m_cObservCenter = cmplx(m_ccObservCenter.m_x, m_ccObservCenter.m_y);
    bSystemPar = true;
  }
  if (pk.Get3d("MeasCenter", (void*)&m_ccMeasCenter, 'f'))
  {
    m_cMeasCenter = cmplx(m_ccMeasCenter.m_x, m_ccMeasCenter.m_y);
    m_cPartCentPos = m_cMeasCenter;
    bSystemPar = true;
  }
  bSystemPar |= pk.Get3d("ObservToMeasDist",
    (void*)&m_DistFromObservationToMeasurement, 'f');
  if (pk.GetString("FocusingLight", m_sLastCmndForFocusingLightWithExt))
  {
    m_sLastCmndForExtLight = m_sLastCmndForFocusingLightWithExt;
    int iPos = (int)m_sLastCmndForExtLight.Find(_T("X "));
    if (iPos)
    {
      int iVal = atoi(((LPCTSTR)m_sLastCmndForExtLight) + iPos + 2);
      if (iVal > 0)
        m_iFocusingLight_us = iVal;
    }
  }
  if (pk.GetInt("FocusingLight_us", m_iFocusingLight_us))
  {
    m_sLastCmndForFocusingLightWithExt.Format(
      _T("out 0x8ff %d\r\n"), m_iFocusingLight_us);
    m_sLastCmndForExtLight = m_sLastCmndForFocusingLightWithExt;
  }

  if (pk.GetString("PartFile", m_PartFile))
  {
    int slashpos = (int)m_PartFile.ReverseFind('\\');
    if (slashpos == -1)
      slashpos = (int)m_PartFile.ReverseFind(_T('/'));
    if (slashpos != -1)
      m_ResultDirectory = m_PartFile.Left(slashpos + 1);
    else
      m_ResultDirectory = _T(".\\");
    //FxVerifyCreateDirectory( m_PartFile ) ;
  }
  pk.GetString("NewPartName", m_NewPartName);
  if (pk.GetString("SysConfFileName", m_SystemConfigFileName)
    && !m_SystemConfigFileName.IsEmpty())
  {
    bSystemPar = true;
    FXPropKit2 pkSys;
    if (pkSys.GetFromFile(m_SystemConfigFileName))
    {
      pkSys.GetDouble("MeasScale_pix_per_um", m_dScale_pix_per_um);
      pkSys.GetDouble("ObservScale_pix_per_um", m_dObservScale_pix_per_um);
      if (pkSys.GetCmplx("MeasCenter", m_cMeasCenter))
        m_cPartCentPos = m_cMeasCenter;
      pkSys.GetDouble("BaseHeight", m_dBaseHeight);
      pkSys.GetDouble("AdapterHeight", m_dAdapterHeight);
      pkSys.GetCmplx("MeasRange", m_cMeasRange);
      pkSys.GetCmplx("ObservCent", m_cObservCenter);
      pkSys.GetCmplx("PartPlacePos", m_cPartPlacementPos);
      FXString AsText;
      if (pkSys.GetString("ObservToMeasDist", AsText))
      {
        AsText.Trim();
        if (AsText[0] == '(' || AsText[0] == '[' || AsText[0] == '{')
          m_DistFromObservationToMeasurement.FromString((LPCTSTR)AsText + 1);
        else
          m_DistFromObservationToMeasurement.FromString((LPCTSTR)AsText);
      };
    }
  }
  FXString Tmp;
  if (pk.GetString("SaveSysConfig", Tmp))
  {
    FXPropKit2 pkSys;
    pkSys.WriteString("InitialDir", m_sInitialDir);
    pkSys.WriteDouble("MeasScale_pix_per_um", m_dScale_pix_per_um);
    pkSys.WriteDouble("ObservScale_pix_per_um", m_dObservScale_pix_per_um);
    pkSys.WriteDouble("BaseHeight", m_dBaseHeight);
    pkSys.WriteDouble("AdapterHeight", m_dAdapterHeight);
    pkSys.WriteDouble("PartHeight", m_dPartHeight);
    pkSys.WriteCmplx("MeasCenter", m_cMeasCenter);
    pkSys.WriteCmplx("MeasRange", m_cMeasRange);
    pkSys.WriteCmplx("ObservCent", m_cObservCenter);
    pkSys.WriteCmplx("PartPlacePos", m_cPartPlacementPos);
    TCHAR AsText[1000];
    m_DistFromObservationToMeasurement.ToString(AsText, 1000, "%lf,%lf,%lf");
    pkSys.WriteString("ObservToMeasDist", AsText);

    if (!pkSys.WriteToFile(Tmp))
      SENDERR("Can't save system config to file %s", (LPCTSTR)Tmp);
  }
  if (bSystemPar && (iNItems == 1))
  {
    FXPropKit2 pkSys;
    pkSys.WriteString("InitialDir", m_sInitialDir);
    pkSys.WriteDouble("MeasScale_pix_per_um", m_dScale_pix_per_um);
    pkSys.WriteDouble("ObservScale_pix_per_um", m_dObservScale_pix_per_um);
    pkSys.WriteDouble("BaseHeight", m_dBaseHeight);
    pkSys.WriteDouble("AdapterHeight", m_dAdapterHeight);
    pkSys.WriteDouble("PartHeight", m_dPartHeight);
    pkSys.WriteCmplx("MeasCenter", m_cMeasCenter);
    pkSys.WriteCmplx("MeasRange", m_cMeasRange);
    pkSys.WriteCmplx("ObservCent", m_cObservCenter);
    pkSys.WriteCmplx("PartPlacePos", m_cPartPlacementPos);
    TCHAR AsText[1000];
    m_DistFromObservationToMeasurement.ToString(AsText, 1000, "%lf,%lf,%lf");
    pkSys.WriteString("ObservToMeasDist", AsText);

    FXRegistry Reg("TheFileX\\ConturASM");
    FXString Directory = Reg.GetRegiString("Data", "MainDirectory", "c:\\BurrInspector\\");
    FXString ConfigSubDir = Reg.GetRegiString("Data", "ConfigSubDir", "Config\\");
    FXString DataSubDir = Reg.GetRegiString("Data", "DataSubDir", "Data\\");
    FXString FileName = Reg.GetRegiString("Data",
      "SysParFileName", "SystemParameters.dat");
    FXString SysParFullPath(Directory + ConfigSubDir + FileName);
    m_SimulationLogFileName = Directory + "SimulationLog.log" ;

    if (!pkSys.WriteToFile((LPCTSTR)SysParFullPath))
      SENDERR("Can't save system config to file %s", (LPCTSTR)SysParFullPath);
    Reg.GetRegiCmplx("Parameters", "SaveFragmentSize", m_SaveFragmentSize, cmplx(300., 300.));
  }
  m_bUpdateSimulationData = pk.GetInt( "Simulation" , m_iSimulationMode ) ;
  m_bShuffle = pk.GetInt( "Shuffle" , m_bShuffle ) ;
  if ( m_iSimulationMode )
  {
    FXRegistry Reg( "TheFileX\\ConturASM" );
    FXString Directory = Reg.GetRegiString( "Data" , "MainDirectory" , "c:\\BurrInspector\\" );
    m_SimulationLogger.SetFile( Directory , "Simulation.log" ) ;
  }
  return true;
}

bool ConturAsm::PrintProperties(FXString& text)
{
  FXPropKit2 pk;
  CFilterGadget::PrintProperties(text);
  pk.WriteInt("Dir", m_iDir);
  pk.WriteInt("MeasStep", m_iMeasStep);
  pk.WriteInt("MeasZoneWidth", m_iMeasZoneWidth);
  pk.WriteInt(_T("WorkingMode"), (int)m_WorkingMode);
  pk.WriteInt(_T("ResultView"), (int)m_ResultViewMode);
  pk.WriteInt(_T("LightDir"), (int)m_LightDir);
  pk.WriteInt(_T("NActiveLeds"), m_iNLeds);
  WriteArray(pk, "Maximums", _T('f'), _T("%.1f"),
    (int)m_dMaximumsForViewCsale.GetCount(), m_dMaximumsForViewCsale.GetData());
  pk.WriteInt("MinEdgeContinuty", m_iMinimalContinuty);
  pk.WriteInt("SegmentLen", m_iSegmentLength);
  pk.WriteDouble("EdgeViewScale", m_dEdgeViewScale);
  pk.WriteDouble("WidthViewScale", m_dWidthViewScale);
  pk.WriteInt("MinShadowAmpl", m_iMinShadowAmpl);
  FXString ThresholdsAsString;
  ThresholdsAsString.Format(_T("%5.3f,%5.3f,%5.3f"),
    m_dUpperThreshold, m_dThicknessThreshold, m_dShadowThres);
  pk.WriteString("Thresholds", ThresholdsAsString);
  pk.WriteDouble("StdThreshold", m_dStdThreshold);
  pk.WriteDouble("DarkEdgeThres", m_dDarkEdgeThreshold);
  pk.WriteInt("ViewLength", m_iViewLength);
  pk.WriteInt("ExtLight_us", m_iExtLight_us);
  WriteArray(pk, "LightForLEDs", 'd', 8, m_iLightForLED_us);
  pk.WriteInt("IntLight_us", m_iIntLight_us);
  pk.WriteInt("LightForDark_us", m_iLightForDark_us);
  pk.WriteInt("Outside_pix", m_iOut_pix);
  pk.WriteInt("Inside_pix", m_iIn_pix);
  pk.WriteInt("StopOnFault", m_iStopOnFault);
  pk.WriteInt("MeasExp_us", m_iMeasurementExposure_us);
  pk.WriteInt("ObsMeasExp_us", m_iObservationMeasExp_us);
  pk.WriteInt("ObsFocusExp_us", m_iObservationFocusExp_us);
  pk.WriteDouble("MinLocalFocus", m_dMinimalLocalFocus);
  pk.WriteInt("FocusFall_perc", m_iFocusFall_perc);
  pk.WriteDouble("KDefocusToHeight", m_dK_DefocusTodZ);
  pk.WriteInt("FocusStep_um", m_iFocusStep_um);
  pk.WriteInt("ScanAccel_un", m_iAccel_units);
  pk.WriteInt("FocusingPeriod", m_iFocusingPeriod);
  FXString FocusParamsAsString;
  FocusParamsAsString.Format(_T("%d,%d,%d,%d,%d,%d,%d"),
    m_iFocusAreaSize_pix, m_iFocusDistFromEdge_pix,
    m_iObservFocusRange_um, m_iMeasInitialFocusRange_um,
    m_MeasFocusRange_um, m_iFocusWorkingMode,
    (int)m_dFocusingCorrection_um);
  pk.WriteString("FocusingParams", FocusParamsAsString);
  FocusParamsAsString.Format(_T("%.3f,%.1f,%.1f"), m_dFocusDecreaseNorm,
    m_dFocusRangeK, m_dMinimalContrast);
  pk.WriteString("FocusIndication", FocusParamsAsString);
  pk.WriteString("InitialDir", m_sInitialDir);
  pk.WriteDouble("MeasScale_pix_per_um", m_dScale_pix_per_um);
  pk.WriteDouble("ObservScale_pix_per_um", m_dObservScale_pix_per_um);
  pk.WriteDouble("BaseHeight", m_dBaseHeight);
  pk.WriteDouble("AdapterHeight", m_dAdapterHeight);
  pk.WriteDouble("PartHeight", m_dPartHeight);
  pk.Write3d("ObservCent", (void*)&m_ccObservCenter, 'f', "%.1f");
  pk.Write3d("MeasCenter", (void*)&m_ccMeasCenter, 'f', "%.1f");
  pk.Write3d("ObservToMeasDist",
    (void*)&m_DistFromObservationToMeasurement, 'f', "%.1f");

  pk.WriteCmplx("MeasRange", m_cMeasRange);
  pk.WriteCmplx("PartPlacePos", m_cPartPlacementPos);
  pk.WriteString("FocusingLight", m_sLastCmndForFocusingLightWithExt);
  pk.WriteInt("FocusingLight_us", m_iFocusingLight_us);
  pk.WriteString("PartFile", m_PartFile);
  pk.WriteString("NewPartName", m_NewPartName);
  pk.WriteString("SysConfFileName", m_SystemConfigFileName);
  pk.WriteInt( "Simulation" , m_iSimulationMode ) ;
  pk.WriteInt( "Shuffle" , m_bShuffle ) ;

  text += pk;
  return true;
}

bool ConturAsm::ScanSettings(FXString& text)
{
  text = "template("
    //     "Spin(Dir,0,7)"
    "Spin(MeasStep,-200,200)"
    ",Spin(MeasZoneWidth,-1,200)" // -1 means, that zone is equal to MeasStep
    ",Spin(MinEdgeContinuty,1,100)"
    ",Spin(SegmentLen,3,30)"
    ",ComboBox(WorkingMode(ScanOnly(0),Measure(1),DarkEdge(2),DarkEdge_NoInternal(3)))"
    ",ComboBox(ResultView(View3D(0),ViewWidth(1),ViewMaxWidth(2),"
    "ViewHeight(3),ViewMaxHeight(4),ViewVolume(5),ViewBurrLen(6)))";
  //   if ( m_WorkingMode != WM_DarkEdge_NoInternal )
  //   {
  //     text += ",ComboBox(LightDir(Ortho(0),Centered(1)))"
  //       ",Spin(NActiveLeds,1,3)" ;
  //   }
  text += ",EditBox(Maximums)"
    ",EditBox(EdgeViewScale)"
    ",EditBox(PartFile)"
    ",EditBox(NewPartName)"
    ",EditBox(WidthViewScale)"
    ",EditBox(Thresholds)";
  //   if ( m_WorkingMode != WM_DarkEdge_NoInternal )
  //   {
  //     text += ",EditBox(StdThreshold)"
  //       ",Spin(MinShadowAmpl,10,10000)"
  //       ",Spin(ExtLight_us,10,30000)"
  //       ",Spin(IntLight_us,10,30000)" ;
  //   }

  text += ",Spin(LightForDark_us,10,30000)"
    ",EditBox(DarkEdgeThres)"
    ",Spin(ViewLength,10,100)"
    ",Spin(Outside_pix,0,20)"
    ",Spin(Inside_pix,5,80)"
    //     ",EditBox(InitialDir)"
    ",Spin(StopOnFault,-10000,10000)"
    ",Spin(MeasExp_us,10,30000)"
    ",Spin(ObsMeasExp_us,10,30000)"
    //    ",Spin(ObsFocusExp_us,10,30000)"
    ",EditBox(MinLocalFocus)"
    ",Spin(FocusFall_perc, 5 , 100)"
    ",EditBox(KDefocusToHeight)"
    ",Spin(FocusStep_um,10,200)";
  ",Spin(ScanAccel_un,3,500)";
  //   if ( m_WorkingMode != WM_DarkEdge_NoInternal )
  //   {
  //     text += ",Spin(FocusingPeriod,0,10000)" ;
  //   }
  text += ",EditBox(FocusingParams)"
    ",EditBox(FocusIndication)"
    ",EditBox(AdapterHeight)"
    ",EditBox(BaseHeight)"
    ",EditBox(PartHeight)"
    ",EditBox(MeasScale_pix_per_um)"
    ",EditBox(ObservScale_pix_per_um)"
    ",EditBox(MeasCenter)"
    ",EditBox(MeasRange)"
    ",EditBox(ObservCent)"
    ",EditBox(PartPlacePos)"
    ",EditBox(ObservToMeasDist)"
    ",EditBox(FocusingLight)"
    ",EditBox(SysConfFileName)"
    ",Spin(Simulation, 0 , 1024)"
    ",Spin(Shuffle,0,1)"
    ")";
  return true;
}

Directions ConturAsm::GetNewDirection(cmplx& cEdgeToM, cmplx& cEdgeToP, cmplx& cCent)
{
  double dAngle = -arg(cEdgeToP - cEdgeToM);
  dAngle = NormTo2PI(dAngle);
  Directions Res = (Directions)(ROUND((M_2PI - (dAngle - M_PI_2)) / LED_ANGLE_STEP) & 7);
  int iIntNew = (int)Res;
  int iIntOld = (int)m_iDir;
  int iDiff = (iIntNew - iIntOld);
  if (abs(iDiff) > 1)
  {
    if ((iDiff != 7) && (iDiff != -7))
    {
      //      ASSERT(0) ;
    }
  }
  return Res;
}

double ConturAsm::CorrectAngle(double dAngle)
{
  switch (m_iDir)
  {
  case H12_00:
    if (dAngle >= M_PI)
      dAngle -= M_PI;
    break;
  case H01_30:
    if (dAngle >= M_PI_2 + M_PI_4 && dAngle <= M_2PI - M_PI_4)
      dAngle -= M_PI;
    break;
  case H03_00:
    if (dAngle >= M_PI_2 && dAngle <= M_PI + M_PI_2)
      dAngle -= M_PI;
    break;
  case H04_30:
    if (dAngle >= M_PI_4 && dAngle <= M_PI + M_PI_4)
      dAngle -= M_PI;
    break;
  case H06_00:
    if (dAngle <= M_PI)
      dAngle -= M_PI;
    break;
  case H07_30:
    if (dAngle <= M_PI_4 + M_PI_2 || dAngle >= M_2PI - M_PI_4)
      dAngle -= M_PI;
    break;
  case H09_00:
    if (dAngle <= M_PI_2 || dAngle >= M_PI + M_PI_2)
      dAngle -= M_PI;
    break;
  case H10_30:
    if (dAngle <= M_PI_4 || dAngle >= M_PI + M_PI_4)
      dAngle -= M_PI;
    break;
  }
  dAngle = fmod(dAngle + 2 * M_PI, 2 * M_PI); // put to range 0..2*PI
  return dAngle;
}


// Calculates lights directions for current edge position
Directions ConturAsm::CalcLightDirections(cmplx& cPt1, cmplx& cPt2,
  double& dInternalLightDir, double& dExternalLightDir,
  double& dRealLightDirForExternal,
  CContainerFrame* pGraphics)
{
  cmplx DirVect = cPt2 - cPt1;
  cmplx cDirVectNorm = GetNormalized(DirVect);
  cmplx cOrthoLeft = GetOrthoLeft(cDirVectNorm);

  double dDirAngle = arg(conj(DirVect));

  // Now we have to calculate angle from Pt1 to Pt2 relatively to part center
  cmplx FromPartCent = -GetVectFromFOVCentToPartCent();

  cmplx cPt1ToAbs = conj(cPt1 - m_cMeasFOVCent);
  cmplx cPt2ToAbs = conj(cPt2 - m_cMeasFOVCent);
  cmplx cPt1RelToPartCent_um = (cPt1ToAbs / m_dScale_pix_per_um) + FromPartCent;
  cmplx cPt2RelToPartCent_um = (cPt2ToAbs / m_dScale_pix_per_um) + FromPartCent;
  double dAngleFromPt1ToPt2 = GetAngleBtwVects(cPt1RelToPartCent_um, cPt2RelToPartCent_um);

  cmplx cRotate = cmplx(0., (dAngleFromPt1ToPt2 > 0) ? 1. : -1.);

  double dRotation = (dAngleFromPt1ToPt2 > 0) ? M_PI2 : -M_PI2;

  // cmplx cLightDirVect = conj( DirVect ) * cRotate ;
  // cmplx cLightDirVect = DirVect * cRotate ;


  dExternalLightDir = dDirAngle - dRotation;
  dExternalLightDir = NormTo2PI(dExternalLightDir);
  dRealLightDirForExternal = GetDiscreteDir(dExternalLightDir);
  dInternalLightDir = NormTo2PI(dExternalLightDir + M_PI);
  m_cNormToDirectionLight = polar(80., dInternalLightDir); // from part

  cmplx cNormOutInFOV = polar(80., -dInternalLightDir);
  //m_VectorsViewCent = cPt1 - m_cNormToDirectionLight ;
  m_VectorsViewCent = cPt1 + cNormOutInFOV;
  cmplx cVectCent = m_VectorsViewCent;
  cmplx EndDirVect = cVectCent + DirVect * 0.3;
  cmplx cEdgeCenter = 0.5 * (cPt1 + cPt2);

  //   if ( dInternalLightDir <= M_PI_4 )
  //     m_cTextViewCent = cEdgeCenter + polar( 20. , -dInternalLightDir ) ;
  //   else if ( dInternalLightDir <= 3. * M_PI_4 )
  //     m_cTextViewCent = cPt1 + polar( 50. , -dInternalLightDir ) ;
  //   else if ( dInternalLightDir <= 5. * M_PI_4 )
  //     m_cTextViewCent = cEdgeCenter + polar( 100. , -dInternalLightDir ) ;
  //   else if ( dInternalLightDir <= 7. * M_PI_4 )
  //     m_cTextViewCent = cPt2 + polar( 50. , -dInternalLightDir ) ;
  //   else
  //     m_cTextViewCent = cEdgeCenter + polar( 20. , -dInternalLightDir ) ;

//  m_cTextViewCent = ((cPt1 + cPt2) * 0.5) - cOrthoLeft * 80. ;
  m_cFinishingMsgPt = cPt1 - cOrthoLeft * 150. - cmplx(0., 100.);
  m_cTextViewCent = m_cFinishingMsgPt + cmplx(0., 60);
  if ( pGraphics && !m_iSimulationMode )
  {
    CFigureFrame * pDirView = CreateFigureFrame(&cVectCent, 1,
      GetHRTickCount(), "0xff8000");
    pDirView->AddPoint(CmplxToCDPoint(EndDirVect));
    pGraphics->AddFrame(pDirView);

    CFigureFrame * pDirOrtho = CreateFigureFrame(&cVectCent, 1,
      GetHRTickCount(), "0x00ff00");
    cmplx EndLightVect = cVectCent - cNormOutInFOV; // +m_cNormToDirectionLight ;
    pDirOrtho->AddPoint(CmplxToCDPoint(EndLightVect));
    pGraphics->AddFrame(pDirOrtho);

    CFigureFrame * pDirReal = CreateFigureFrame(&cVectCent, 1,
      GetHRTickCount(), "0x0000ff");
    cmplx cRealLightDirVect = polar(100., dRealLightDirForExternal);
    EndLightVect = cVectCent + conj(cRealLightDirVect);
    pDirReal->AddPoint(CmplxToCDPoint(EndLightVect));
    pGraphics->AddFrame(pDirReal);
    CFigureFrame * pDirToCent = CreateFigureFrame(&m_VectorsViewCent, 1,
      GetHRTickCount(), "0x00c0c0");
    double dAngleToPartCent = GetAngleToCenter();
    cmplx cDirToCent = polar(100., -dAngleToPartCent);
    cmplx EndDirToCent = m_VectorsViewCent + cDirToCent;
    pDirToCent->AddPoint(CmplxToCDPoint(EndDirToCent));
    pGraphics->AddFrame(pDirToCent);
  }

  Directions iDir = GetDir(dDirAngle + (dAngleFromPt1ToPt2 < 0) ? M_PI : 0.);
  return iDir;
}

bool ConturAsm::InitMeasurement()
{
  m_iStage2FaultCounter = 0;
  m_bInLineFocusing = false;
  m_bFocusFound = false;
  m_bPrepareToEnd = false;
  m_iDir = m_InitialDir;
  m_iPrevDir = m_InitialDir;
  m_dInitialAngle = 5.0 * M_PI;
  m_cLastMotion = m_cLastNotSmallMotion = cmplx(0., 0.);
  m_iNWaitsForMotionAnswers = 3;
  m_iIndexForMapView = -1;
  m_iNStepsAfterFocusing = 0;
  m_bIsFocusingNecessary = false;
  m_bInMeasurementFocusFound = false;
  m_DataFrom = ADF_NoData;
  m_dLastContrast = m_dLastNextPtContrast = m_dLastContrastAfterFocusing = 0.;
  m_dLastResultSaveTime = 0.;
  m_iLastSentToUI_Index = 0;
  m_iSelectedPtIndex = -1;
  m_iMaxGradPtIndex = -1;
  m_bFormMaxGradMarker = false;
  FR_RELEASE_DEL(m_pLastInternalContur);
  FR_RELEASE_DEL(m_pLastExternalContur);
  FR_RELEASE_DEL(m_pPartConturOnObservation);
  FR_RELEASE_DEL(m_pObservationImage);
  g_iIndexInLCTRL = 0;
  memset(g_LCTRLSentContainers, 0, sizeof(g_LCTRLSentContainers));

  if (m_pPartInternalEdge)
    m_pPartInternalEdge->RemoveAll();
  if (m_pPartExternalEdge)
    m_pPartExternalEdge->RemoveAll();
  m_LastScanFrames.RemoveAll();
  m_AverData.RemoveAll();
  m_AveragedSelectedPoints.RemoveAll();
  m_iDefocusToPlus = 1;
  m_bFocusDirectionChanged = false;
  m_dLastdZ = 0.;
  m_dMinZ = m_dMinZWithGood = m_CurrentPos.m_z;
  m_dMaxZ = m_dMaxZWithGood = m_CurrentPos.m_z;
  m_CurrentPos.m_theta = m_dMeasurementStartTime = GetHRTickCount();
  m_cInitialRobotPosForMeas = cmplx(m_CurrentPos.m_x, m_CurrentPos.m_y);
  m_bPrepareToEnd = false;
  m_bScanFinished = false;
  m_SectorsMarkingMode = SMM_NoMarking;
  m_MapMouseClickMode = MCM_ViewSaved;

  m_iIndexWithMaxWidth = m_iIndexWithMaxOfMaxWidth = 0;
  m_DataForPosition.RemoveAll();
  m_CurrentViewMeasStatus.Reset();
  m_ViewsForPosition.RemoveAll();
  m_dDistToPt0.RemoveAll();
  m_dMinimalDistToFirstPt = DBL_MAX;
  m_dMaxGradWidth_pix = m_iIn_pix - 5;
  m_dMaxGradWidth_um = m_dMaxGradWidth_pix / m_dScale_pix_per_um;

  m_ObservGeomAnalyzer.GetFormFileNameAndVersion();
  int iNKnownForms =
    m_ObservGeomAnalyzer.Init(m_ObservGeomAnalyzer.m_FormattedFileName);
  m_InitPos.Reset();

  m_CurrentPartStatus = DecodePartDescriptors();
  LoadPartParametersFromRegistry(m_CurrentPartStatus != Part_Known);

  m_StageBeforeMotion = Stage_Inactive;
  m_bInFocusing = false;
  m_dLightForInternal = M_PI;
  m_dLightForExternal = 0.;
  m_cLastNotSmallMotion = cmplx(10., 10.);
  m_cTraveling = m_cRobotPos = cmplx(0., 0.);
  m_dTraveling = 0.;
  m_iLastFocusingMotionSteps = m_iNMotionSteps = 0;
  m_iNotMeasuredCntr = 0;
  m_dStopReceivedTime = 0.;
  m_Widths.RemoveAll();
  m_ConturData.RemoveAll();
  m_iLastShownIndex = -1;
  m_MeasurementTS = GetTimeAsString();
  CQuantityFrame * pResetWidthGraph = CQuantityFrame::Create(0);
  pResetWidthGraph->SetLabel(_T("Remove:Width_um"));
  PutAndRegisterFrame(m_pMarking, pResetWidthGraph);
  ResetViews();
  Sleep(20);
  FXRegistry Reg( "TheFileX\\ConturASM" ) ;
  FXString ObjectPars = Reg.GetRegiString( "Parameters" ,
    ( m_iSimulationMode ) ? "TVObjParsForSimulation" : "TVObjParsForNormalWork" ,
    ( m_iSimulationMode ) ? TVOBJ_PARS_FOR_SIMULATION : TVOBJ_PARS_FOR_WORK ) ;
  SetTVObjPars( ObjectPars ) ;
  //   CTextFrame * pCommand = CreateTextFrame("name=ext_segment;dir_degrees=90;",
//     "SetObjectProp");
//   PutFrame(m_pOutput, pCommand);
  m_dLastEdgePredictedAngle_deg = 90;


  m_MotionMask = MotionMaskX | MotionMaskY | MotionMaskZ;
  m_iNWaitsForMotionAnswers = 3;
  m_cCurrentTarget = m_CurrentPos.m_x ;
  m_dZTarget = m_CurrentPos.m_z ;

  SetProcessingStage(Stage_0_GetCurrPos, true);
  RequestCurrentPosition();
  Sleep(100);
  CTextFrame * pSetMaxPulseTime = CreateTextFrame(_T("tmax 30000\r"), _T("LightControl"));
  PutFrame(m_pLightControl, pSetMaxPulseTime);
  SetZMotionSpeed(20000.);
  Sleep(50);
  SetMotionSpeed(m_iXYMotionScanSpeed_um_sec);
  Sleep(50);
  SetMotionAcceleration(m_iAccel_units * 8, 0x03);

  //   CTextFrame * pFullReport = CreateTextFrame(
  //     "00" , "CloseFile" , 1000000 ) ;
  //   PutFrame( m_pOutput , pFullReport ) ; // For new file creation
  //   Sleep( 100 ) ;
  FXString BeginMsg(_T("Measurement Start\n"));
  BeginMsg += ConturSample::GetCaption();
  CTextFrame * pFullReport = CreateTextFrame(
    (LPCTSTR)BeginMsg, "AllDataReport", 1000000);
  PutFrame(m_pOutput, pFullReport);

  return true;
}


CCoordinate ConturAsm::GetAbsPosOnObservation(cmplx ObservPt)
{
  cmplx cDistToFOVCent = ObservPt - m_cObservFOVCenter;
  cmplx cObservPtScaled = cDistToFOVCent / m_dObservScale_pix_per_um;
  cmplx cAbs = m_cObservCenter + cObservPtScaled;
  CCoordinate AbsPos(cAbs);
  double dZPos = m_ccObservCenter.m_z - m_dAdapterHeight - m_dPartHeight;
  AbsPos.m_z = dZPos;
  return AbsPos;
}


CCoordinate ConturAsm::GetTargetForObservationPoint(cmplx ObservPt)
{
  CCoordinate ccPosOnObservation = GetAbsPosOnObservation(ObservPt);
  CCoordinate ccShiftToObservationCenter =
    m_ccObservCenter.MinusXY(ccPosOnObservation);
  CCoordinate TargetOnMeas =
    m_ccMeasCenter.ShiftXY(ccShiftToObservationCenter);
  //   double dZToFocus = m_CurrentPos.m_z - m_ccObservCenter.m_z 
  //     - m_DistFromObservationToMeasurement.m_z ;
  TargetOnMeas.m_z = m_ccMeasCenter.m_z - m_dAdapterHeight - m_dPartHeight;
  return TargetOnMeas;
}
cmplx ConturAsm::GetDistanceOnObservation(cmplx Pt1, cmplx Pt2) // vect from pt1 to pt2
{
  cmplx cDistInFOV = Pt2 - Pt1;
  cmplx cDistScaled = cDistInFOV / m_dObservScale_pix_per_um;
  return conj(cDistScaled);
}

bool ConturAsm::GetTargetForMapPoint(CPoint MapPt, CCoordinate& Target,
  int& iNearestIndex)
{
  if (!m_ConturData.GetCount())
    return false;

  iNearestIndex = 0;
  double dMinDist = Dist(MapPt, m_ConturData[0].m_PointOnMap);
  for (int i = 1; i < m_ConturData.GetCount(); i++)
  {
    double dNewDist = Dist(MapPt, m_ConturData[i].m_PointOnMap);
    if (dNewDist < dMinDist)
    {
      dMinDist = dNewDist;
      iNearestIndex = i;
    }
  }
  if (dMinDist < 12)
  {
    Target = m_ConturData[iNearestIndex].m_RobotPos;
    return true;
  }
  return false;
}

LPCTSTR ConturAsm::GetScaleCaption(ResultViewMode Mode)
{
  switch (Mode)
  {
  case RVM_BurrWidth: return _T("    Width, um");
  case RVM_MaxBurrWidth: return _T("Max Width, um");
  case RVM_BurrHeight: return _T("   Height, um");
  case RVM_MaxBurrHeight: return _T("Max Height, um");
  case RVM_BurrVolume: return _T("   Volume, qum");
  case RVM_BurrEdgeLength: return _T("Burr Length, um");
  }
  return _T("Unknown");
}

bool ConturAsm::GetDataForIndex(ResultViewMode Mode, int iIndex,
  double& dValue)
{
  bool bRes = true;
  switch (Mode)
  {
  case RVM_BurrWidth:
    dValue = m_ConturData[iIndex].m_dAveBurrWidth_um;
    break;
  case RVM_MaxBurrWidth:
    dValue = m_ConturData[iIndex].m_dMaxBurrWidth_um;
    break;
  case RVM_BurrHeight:
    dValue = m_ConturData[iIndex].m_dAveBurrHeight_um;
    break;
  case RVM_MaxBurrHeight:
    dValue = m_ConturData[iIndex].m_dMaxBurrHeight_um;
    break;
  case RVM_BurrVolume:
    dValue = m_ConturData[iIndex].m_dBurrVolume_qum;
    break;
  case RVM_BurrEdgeLength:
    dValue = m_ConturData[iIndex].m_dEdgeWithBurrLen_um;
    break;
  default: bRes = false; break;
  }
  return bRes;
}

static CPoint gs_LastPt(0, 0);
static double gs_dFromMapLastTime = 0.;

bool ConturAsm::ProcessMsgFromUI(const CTextFrame * pMsgFromUI)
{
  FXPropertyKit pk = pMsgFromUI->GetString();
  int iPt;
  FXString ViewMethod;
  if (IsInactive() && (m_DataFrom != ADF_NoData)
    && m_ConturData.GetCount()
    && pk.GetInt(_T("Pt"), iPt)
    && pk.GetString(_T("View"), ViewMethod))
  {
    if (iPt >= 0 && iPt < m_ConturData.GetCount())
    {
      m_iSelectedPtIndex = iPt;
      CDataFrame * pMapView = FormMapImage(NULL, m_ResultViewMode, m_iSelectedPtIndex, -2);
      if (pMapView)
        PutFrame(m_pOutput, pMapView);

      if (ViewMethod == _T("Saved"))
      {
        CDataFrame * pOut = FormFrameForSavedImageView(iPt);

        if (pOut)
          return PutFrame((pOut->IsContainer()) ? m_pMainView : m_pMarking, pOut);

        return false;
      }
      else if (m_DataFrom == ADF_MeasuredNow)
      {
        ConturSample& Sample = m_ConturData.GetAt(iPt);
        double dAngle = -arg(Sample.m_cSegmentVect);// Y is going down
        SendEdgeAngleToTVObject(dAngle);
        if (!Sample.m_RobotPos.Compare(m_CurrentPos, 10.))
        {
          if (ViewMethod == _T("Live"))
          {
            m_ProcessingStage = Stage_GetExtImage;
            SetLightForDarkEdge();
            return true;
          }
          if (ViewMethod == _T("DoMeas"))
          {
            m_ProcessingStage = Stage_OneDarkMeasBeginToMax;
            SetLightForDarkEdge(0);
            return true;
          }
        }
        else
        {
          if (ViewMethod == _T("Live"))
          {
            OrderAbsMotion(Sample.m_RobotPos);
            m_DelayedOperations.push(Stage_GetExtImage);
            return true;
          }
          if (ViewMethod == _T("DoMeas"))
          {
            OrderAbsMotion(Sample.m_RobotPos);
            m_DelayedOperations.push(Stage_OneDarkMeasBeginToMax);
            return true;
          }
        }
      }
      else
        SENDERR("Data from history, there is no options for LIVE or MEASUREMENT");
    }
  }

  return false;
}


bool ConturAsm::SetFocusingArea(
  int iXOffset, int iYOffset, int iWidth, int iHeight, int iWorkingMode)
{
  CTextFrame * pFocusingROI = CreateTextFrame(
    _T(""), _T("ROI"));
  pFocusingROI->GetString().Format(
    "XOff=%d;YOff=%d;Width=%d;Height=%d;WorkingMode=%d;Correction_um=%d;",
    iXOffset, iYOffset, iWidth, iHeight, iWorkingMode,
    (int)m_dFocusingCorrection_um);
  return PutFrame(m_pLightControl, pFocusingROI);
}


bool ConturAsm::SetFocusingArea(cmplx& ViewPtOnEdge)
{
  int iXOff, iYOff;
  double dAngToCent = GetAngleToCenter();
  cmplx VectToCent = polar(1.0, dAngToCent);
  cmplx FocusCentPt = ViewPtOnEdge + (double)m_iFocusDistFromEdge_pix * conj(VectToCent);
  iXOff = ROUND(FocusCentPt.real()) - m_iFocusAreaSize_pix / 2;
  iYOff = ROUND(FocusCentPt.imag()) - m_iFocusAreaSize_pix / 2;
  return SetFocusingArea(iXOff, iYOff,
    m_iFocusAreaSize_pix, m_iFocusAreaSize_pix, m_iFocusWorkingMode);
}

bool ConturAsm::StartFocusing(LPCTSTR pLightCommand,
  int iScanSpeed, int iExposure_us, int iWorkingMode)
{
  int iXOff = 0, iYOff = 0;
  int iW = 0, iH = 0;
  if (m_pLastExternalContur)
  {
    cmplx CentralPt = CDPointToCmplx(
      m_pLastExternalContur->GetAt(m_iMinDistIndexForExternal));
    SetFocusingArea(CentralPt);
  }
  else
    SetFocusingArea(iXOff = 920, iYOff = 560, iW = 80, iH = 80, m_iFocusWorkingMode);

  SetCamerasExposure(iExposure_us, "Can't set exposure for focusing", Stage_Inactive);
  SetTask(-1);
  Sleep(20);
  m_iLastFocusingMotionSteps = m_iNMotionSteps;
  CTextFrame * pBeginFocusing = CreateTextFrame(
    _T(""), _T("FindFocus"));
  bool bEnfoced = (m_bIsFocusingNecessary &&
    m_dLastContrast > 5. && m_dLastNextPtContrast > 5.);
  double dRange = m_MeasFocusRange_um;
  if (bEnfoced)
  {
    m_bIsFocusingNecessary = false;
    dRange = m_dFocusingRange_um;
  }
  pBeginFocusing->GetString().Format(
    "RangeBegin=%d;RangeEnd=%d;LightCommand=%s;"
    "ScanSpeed_um=%d;WorkingMode=%d;Correction_um=%d;",
    (int)(-dRange), (int)dRange,
    pLightCommand, iScanSpeed, iWorkingMode, (int)m_dFocusingCorrection_um);
  m_bInFocusing = PutFrame(m_pLightControl, pBeginFocusing);
  TRACE("\nFocus Start (%s): %s\n   Exp=%d; R(%d,%d,%d,%d);",
    (m_bInFocusing) ? "OK" : "FAULT",
    (LPCTSTR)(pBeginFocusing->GetString()),
    iExposure_us, iXOff, iYOff, iW, iH);
  return m_bInFocusing;
}

int ConturAsm::ProcessLightCalibration(const CDataFrame * pDataFrame)
{
  bool bInactive = false;
  if (m_ProcessingStage == Stage_CalibLight0) // arrived to observation point
  {
    if (!IsAllCoordsAvailable())
    {
      CTextFrame * pRequestCoord = CreateTextFrame(
        _T("Return_Current_Position=1\r\n"), _T("GetCurrPos"));
      if (PutFrame(m_pLightControl, pRequestCoord))
      {
        m_MotionMask = MotionMaskX | MotionMaskY | MotionMaskZ;
        m_iNWaitsForMotionAnswers = 3;
        m_DelayedOperations.push(Stage_CalibLight0);
        SetProcessingStage(Stage_CalibLight0);
      }
      Sleep(100);
      return  0;
    }
    else
    {
      CCoordinate Target(m_ccObservCenter);
      Target.m_z -= m_dAdapterHeight;
      if (abs(m_cObservCenter - m_cRobotPos) > 100.
        || fabs(Target.m_z - m_CurrentPos.m_z) > 1000.)
      {
        OrderAbsMotion(Target);
        m_DelayedOperations.push(Stage_CalibLight0);
        SetProcessingStage(Stage_CalibLight0);
        return 0;
      }
    }
    if (!SetCamerasExposure(m_iObservationMeasExp_us * 2 + 200,
      "Can't set exposure for light calibration", Stage_Inactive))
    {
      return -1;
    }
    SetTask(-1); // no measurement
    SetWorkingLeds(CAM_OBSERVATION, OBSERV_RIGHT_LED, m_iObservationMeasExp_us * 2);
    m_ProcessingStage = Stage_CalibLight1;
    return 0; // OK, wait for image from right LED
  }
  else // not first step in light calibration
  {
    double dLightMin = 255. * 0.66; // 66% of range
    CTextFrame * pReport = CreateTextFrame(_T(""), _T("Status"));
    const CVideoFrame * pImage = pDataFrame->GetVideoFrame();
    CRect ImRect(0, 0, 0, 0);
    bool bIsRect = GetRect(pImage, ImRect);
    m_cFOVCenterForLightCalib = cmplx(ImRect.CenterPoint().x, ImRect.CenterPoint().y);
    int iRes = 0;
    switch (m_ProcessingStage)
    {
    case Stage_CalibLight1:
      {
        if (bIsRect)
        {
          CRect CalcRect(CPoint((ImRect.right * 5) / 8, (ImRect.bottom * 3) / 8),
            CSize(ImRect.right / 8, ImRect.bottom / 4));
          double dAver = GetAverageForRect(pImage, CalcRect);
          if ((Is8BitsImage(pImage) && (dAver >= dLightMin))
            || (Is16BitsImage(pImage) && (dAver >= 255. * dLightMin))) //  
          {   //Right observation LED is OK, go to check left LED
            SetWorkingLeds(CAM_OBSERVATION, OBSERV_LEFT_LED, m_iObservationMeasExp_us * 2);
            SetProcessingStage(Stage_CalibLight2);
            pReport->GetString().Format(_T("Observation Right LED OK (%.1f)"), dAver);
          }
          else
          {
            pReport->GetString().Format(_T("Fault: Observation Right LED test (%.1f)"), dAver);
            iRes = -2;
          }
        }
        else
        {
          SetWorkingLeds(CAM_OBSERVATION, OBSERV_RIGHT_LED, m_iObservationMeasExp_us * 2);
          pReport->GetString().Format(_T("Fault: No image for right LED test"));
          iRes = -3;
        }
      }
      PutFrame(m_pOutput, pReport);
      return iRes; // OK, wait for image from right LED
    case Stage_CalibLight2:
      {
        if (bIsRect)
        {
          CRect CalcRect(CPoint((ImRect.right * 2) / 8, (ImRect.bottom * 3) / 8),
            CSize(ImRect.right / 8, ImRect.bottom / 4));
          double dAver = GetAverageForRect(pImage, CalcRect);
          if ((Is8BitsImage(pImage) && (dAver >= dLightMin))
            || (Is16BitsImage(pImage) && (dAver >= 255. * dLightMin))) //  
          {   //Left observation LED is OK, go to check central spot position
            SetTask(11); // measure central point
            SetWorkingLeds(CAM_OBSERVATION, OBSERVATION_LEDS, m_iObservationMeasExp_us);
            SetProcessingStage(Stage_CalibLight3);
            pReport->GetString().Format(_T("Observation Left LED OK (%.1f)"), dAver);
          }
          else
          {
            SetWorkingLeds(CAM_OBSERVATION, OBSERV_LEFT_LED, m_iObservationMeasExp_us * 2);
            pReport->GetString().Format(_T("Fault: Observation Left LED test (%.1f)"), dAver);
            iRes = -4;
          }
        }
        else
        {
          SetWorkingLeds(CAM_OBSERVATION, OBSERV_LEFT_LED, m_iObservationMeasExp_us * 2);
          pReport->GetString().Format(_T("Fault: No image for Left LED test"));
          iRes = -5;
        }
      }
      PutFrame(m_pOutput, pReport);
      return iRes; // OK, wait for image from both LEDs and measure central spot
    case Stage_CalibLight3:
      {
        const CFigureFrame * pCenterCoords = pDataFrame->GetFigureFrame(_T("calib_observ"));
        if (pCenterCoords)
        {
          cmplx cCenter = CDP2Cmplx(pCenterCoords->GetAt(0));
          cmplx cToCenter = cCenter - m_cFOVCenterForLightCalib;
          double dError = abs(cToCenter);
          double dDistToCent = abs(m_cFOVCenterForLightCalib);
          if (dError > dDistToCent * 0.01)
          {
            cmplx cShift = cToCenter / m_dObservScale_pix_per_um;
            CCoordinate Target = m_cRobotPos - cShift;
            Target.m_z = m_CurrentPos.m_z;
            OrderAbsMotion(Target);
            m_DelayedOperations.push(Stage_CalibLight3);
            SetProcessingStage(Stage_CalibLight3);
            return NULL;
          }
          CCoordinate diff(m_ccObservCenter);
          diff.m_z -= m_dAdapterHeight;
          diff -= m_CurrentPos;
          double dErr = diff.cabs();
          if (dErr > 10) // microns
          {
            pReport->GetString().Format(_T("New Observation Center is (X%d,Y%d,Z%d)\n"
              "Old is (X%d,Y%d,Z%d)"),
              ROUND(m_CurrentPos.m_x), ROUND(m_CurrentPos.m_y),
              ROUND(m_CurrentPos.m_z + m_dAdapterHeight),
              ROUND(m_ccObservCenter.m_x), ROUND(m_ccObservCenter.m_y),
              ROUND(m_ccObservCenter.m_z));
          }
          else
          {
            pReport->GetString().Format(_T("Observation Center is (X%d,Y%d,Z%d)\n"),
              ROUND(m_CurrentPos.m_x), ROUND(m_CurrentPos.m_y),
              ROUND(m_CurrentPos.m_z + m_dAdapterHeight));
          }
          m_ccObservCenter = m_CurrentPos;
          m_ccObservCenter.m_z += m_dAdapterHeight;
          m_cObservCenter = cmplx(m_CurrentPos.m_x, m_CurrentPos.m_y);


          m_DistFromObservationToMeasurement = m_ccMeasCenter;
          m_DistFromObservationToMeasurement -= m_ccObservCenter;

          //          CCoordinate TargetForCenterViewOnMeasPos = GetTargetForObservationPoint( cCenter ) ;
          CCoordinate TargetForCenterViewOnMeasPos = m_ccMeasCenter;
          TargetForCenterViewOnMeasPos.m_z -= m_dAdapterHeight;
          OrderAbsMotion(TargetForCenterViewOnMeasPos);
          SetProcessingStage(Stage_CalibLight4);
          m_DelayedOperations.push(Stage_CalibLight4);
        }
        else
        {
          SetWorkingLeds(CAM_OBSERVATION, OBSERVATION_LEDS, m_iObservationMeasExp_us);
          pReport->GetString().Format(_T("Fault: No observation center measurement results"));
          iRes = -6;
        }
      }
      PutFrame(m_pOutput, pReport);
      return iRes; // OK, for robot arrive to measurement position
    case Stage_CalibLight4:
      { // OK robot arrived to measurement pos, order imaging for center measurement
        SetTask(2); // measure central point
        if (SetCamerasExposure(m_iMeasurementExposure_us + 200,
          "Can't set exposure", Stage_Inactive))
        {
          SetWorkingLeds(CAM_MEASUREMENT, MEASUREMENT_LEDS, m_iMeasurementExposure_us);
          SetProcessingStage(Stage_CalibLight5);
        }
      }
      return iRes;
    case Stage_CalibLight5:
      { // TAke data about platform center
        const CFigureFrame * pCenterCoords = pDataFrame->GetFigureFrame(_T("calib"));
        if (pCenterCoords)
        {
          cmplx cCenter = CDP2Cmplx(pCenterCoords->GetAt(0));
          cmplx cToCenter = cCenter - m_cFOVCenterForLightCalib;
          double dError = abs(cToCenter);
          double dDistToCent = abs(m_cFOVCenterForLightCalib);
          if (dError > dDistToCent * 0.01)
          {
            cmplx cShift = cToCenter / m_dScale_pix_per_um;
            CCoordinate Target = m_cRobotPos - cShift;
            Target.m_z = m_CurrentPos.m_z;
            OrderAbsMotion(Target);
            m_DelayedOperations.push(Stage_CalibLight5);
            SetProcessingStage(Stage_CalibLight5);
            return NULL;
          }
          CCoordinate diff(m_ccMeasCenter);
          diff.m_z -= m_dAdapterHeight;
          diff -= m_CurrentPos;
          double dErr = diff.cabs();

          if (dErr > 10)
          {
            pReport->GetString().Format(_T("New Measurement Center is (X%d,Y%d,Z%d)\n"
              "Old is (X%d,Y%d,Z%d)"),
              ROUND(m_CurrentPos.m_x), ROUND(m_CurrentPos.m_y),
              ROUND(m_CurrentPos.m_z + m_dAdapterHeight),
              ROUND(m_ccMeasCenter.m_x), ROUND(m_ccMeasCenter.m_y),
              ROUND(m_ccMeasCenter.m_z));
          }
          else
          {
            pReport->GetString().Format(_T("Measurement Center is (X%d,Y%d,Z%d)\n"),
              ROUND(m_CurrentPos.m_x), ROUND(m_CurrentPos.m_y),
              ROUND(m_CurrentPos.m_z + m_dAdapterHeight));
          }
          m_ccMeasCenter = m_CurrentPos;
          m_ccMeasCenter.m_z += m_dAdapterHeight;
          m_cMeasCenter = cmplx(m_CurrentPos.m_x, m_CurrentPos.m_y);

          m_DistFromObservationToMeasurement = m_ccMeasCenter;
          m_DistFromObservationToMeasurement -= m_ccObservCenter;

          CCoordinate Target(m_CurrentPos);
          Target.m_x += 5000.;
          OrderAbsMotion(Target);
          m_DelayedOperations.push(Stage_CalibLight6);
          SetProcessingStage(Stage_CalibLight6);
          m_iLightCalibLEDCntr = 0;
        }
        else
        {
          SetWorkingLeds(CAM_MEASUREMENT, MEASUREMENT_LEDS, m_iMeasurementExposure_us);
          pReport->GetString().Format(_T("Fault: No observation center measurement results"));
          iRes = -6;
        }
      }
      PutFrame(m_pOutput, pReport);
      return iRes;
    case Stage_CalibLight6:
      {
        m_iLightCalibLEDCntr = 0;
        if (!SetCamerasExposure(m_iLightForLED_us[m_iLightCalibLEDCntr] + 200,
          "Can't set exposure", Stage_Inactive))
          ASSERT(0);
        Sleep(50);
        SetWorkingLeds(CAM_MEASUREMENT, 1 << m_iLightCalibLEDCntr,
          m_iLightForLED_us[m_iLightCalibLEDCntr]);
        SetProcessingStage(Stage_CalibLight7);

      }
      return iRes;
    case Stage_CalibLight7:
      {
        if (bIsRect)
        {
          CRect CalcRect(CPoint((ImRect.right * 3) / 8, (ImRect.bottom * 3) / 8),
            CSize(ImRect.right / 4, ImRect.bottom / 4));
          double dAver = GetAverageForRect(pImage, CalcRect);
          double dTolerance = 15.;
          double dTarget = 190.;
          bool bFault = false;
          bool bNotInRange = false;
          if (Is8BitsImage(pImage))
          {
            if (bNotInRange = !IsInRange(dAver, dTarget - dTolerance, dTarget + dTolerance))
            {
            }
            else if (bFault = (dAver < (dTarget - dTolerance) / 5.))
            {
              pReport->GetString().Format(
                _T("Fault: LED %d doesn't work (%.1f)"), m_iLightCalibLEDCntr, dAver);
              iRes = -7;
            }
          }
          else if (Is16BitsImage(pImage))
          {
            dTarget *= 256.;
            if (bNotInRange = !IsInRange(dAver,
              (dTarget - dTolerance) * 256., (dTarget + dTolerance) * 256.))
            {
            }
            else if (bFault = (dAver < (dTarget - dTolerance) * 256. / 5.))
            {
              pReport->GetString().Format(
                _T("Fault: LED %d doesn't work (%.1f)"), m_iLightCalibLEDCntr, dAver);
              iRes = -7;
            }
          }
          else
            ASSERT(0);
          if (bNotInRange && !bFault)
          {
            double dNewTime = (double)m_iLightForLED_us[m_iLightCalibLEDCntr];
            if (dNewTime > 3000.)
              dNewTime = 3000.;
            dNewTime *= dTarget / dAver;
            if (dNewTime > 3000.)
            {
              dNewTime = 3000.;
            }
            FXString Add;
            Add.Format(_T("LED %d change light from %d to %d (%.1f)"),
              m_iLightCalibLEDCntr,
              m_iLightForLED_us[m_iLightCalibLEDCntr], ROUND(dNewTime), dAver);
            pReport->GetString() += Add;
            m_iLightForLED_us[m_iLightCalibLEDCntr] = ROUND(dNewTime);
            if (!SetCamerasExposure(m_iLightForLED_us[m_iLightCalibLEDCntr] + 200,
              "Can't set exposure", Stage_Inactive))
              ASSERT(0);
            Sleep(50);
            SetWorkingLeds(CAM_MEASUREMENT, 1 << m_iLightCalibLEDCntr,
              m_iLightForLED_us[m_iLightCalibLEDCntr]);
            SetProcessingStage(Stage_CalibLight7);
          }
          else // OK or LED doesn't work
          {
            if (!bFault)
            {
              FXString Add;
              Add.Format(_T("LED %d is OK (%.1f)"), m_iLightCalibLEDCntr, dAver);
              pReport->GetString() += Add;
            }
            if (++m_iLightCalibLEDCntr < 8)
            {
              if (!SetCamerasExposure(m_iLightForLED_us[m_iLightCalibLEDCntr] + 200,
                "Can't set exposure", Stage_Inactive))
                ASSERT(0);
              Sleep(50);
              SetWorkingLeds(CAM_MEASUREMENT, 1 << m_iLightCalibLEDCntr,
                m_iLightForLED_us[m_iLightCalibLEDCntr]);
              SetProcessingStage(Stage_CalibLight7);
            }
            else
            {
              pReport->GetString() += _T("\nLED test and adjustment is finished");
              pReport->SetLabel("Status");
              bInactive = true;
            }
          }
        }
        else
        {
          pReport->GetString().Format(_T("Fault: No image for LED %d"), m_iLightCalibLEDCntr);
          iRes = -8;
        }
      }
      PutFrame(m_pOutput, pReport);
      if (bInactive)
        SetProcessingStage(Stage_Inactive);
      return iRes;
    }
  }

  return -1000;
}


// Function logs pMsg
// if dZ != 0. then function does Z motion ordering
// If pFrameForMarkingOutput is not zero, function sends this frame to
//   m_pMarking output and put zero to ptr value
//!!!!!!! *pFrameForMarkingOutput will be set to zero
bool ConturAsm::FocusLogAndMoveZ(
  LPCTSTR pMsg, double dZ, CDataFrame ** pFrameForMarkingOutput)
{
  //   FXString Format( pMsg ) ;
  //   Format += " X = %.1f , Y = %.1f , Z = %.1f \n" ;
  //   FXString FocusAsText , ForFormat ;
  //   FocusAsText = m_CurrentPos.ToString( ForFormat , Format ) ;
  //   FXString Addition ;
  //   if ( dZ != 0. )
  //     Addition.Format( "Last dZ=%.1f , Ordered dZ=%.1f" , m_dLastdZ , dZ ) ;
  //   else
  //     Addition.Format( "Last dZ=%.1f" , m_dLastdZ ) ;
  // 
  //   FocusAsText += Addition ;
  // 
  //   CTextFrame * pFocusLog = CreateTextFrame( FocusAsText , "FocusLog" , 0 ) ;
  //   PutFrame( m_pImageSave , pFocusLog ) ;
  if (dZ != 0.)
    OrderRelZMotion(dZ, true, 120.);

  if (pFrameForMarkingOutput)
  {
    if (*pFrameForMarkingOutput)
    {
      //       FXString ContList , Prefix ;
      //       int iLevel = 0 ;
      //       FXPtrArray FramePtrs ;
      //       int iNFrames = FormFrameTextView(
      //         *pFrameForMarkingOutput , ContList , Prefix , iLevel , FramePtrs ) ;
      PutAndRegisterFrame(m_pMarking, *pFrameForMarkingOutput);
      *pFrameForMarkingOutput = NULL;
    }
    if (m_iStopOnFault != 0)
      Sleep(abs(m_iStopOnFault));
  }
  return dZ == 0.;
}

double ConturAsm::GetOptimalZForIndex(int iIndexForPosition, int iAvFactor)
{
  int iIndexInData = GetIndexWithOptimalFocus(iIndexForPosition, iAvFactor);
  if (iIndexInData == -1) // no data
    return 0.0;

  return m_DataForPosition.GetAt(iIndexInData).m_RobotPos.m_z;
}

int ConturAsm::GetIndexWithOptimalFocus(int iIndexForPosition, int iAvFactor)
{
  double dMaxAv = -DBL_MAX;
  int iIndexInData = -1;
  int iMaxNPoints = 0;
  for (int i = 0; i < m_DataForPosition.GetCount(); i++)
  {
    MeasuredValues& Vals = m_DataForPosition.GetAt(i);
    if (iAvFactor > 0)
    {
      double dAvAv, dAvMax;
      if (iIndexForPosition < Vals.m_iMinusIndex + iAvFactor)
        iIndexForPosition = Vals.m_iMinusIndex;
      else if (iIndexForPosition > Vals.m_iPlusIndex - iAvFactor - 1)
        iIndexForPosition = Vals.m_iPlusIndex - iAvFactor - 1;
      GetAvValues(&Vals.m_Results[iIndexForPosition],
        iAvFactor, dAvAv, dAvMax);
      if (dAvAv > dMaxAv)
      {
        dMaxAv = dAvAv;
        iIndexInData = i;
      }
    }
    else
    {
      if (Vals.m_iNWithGoodFocus > iMaxNPoints)
      {
        iMaxNPoints = Vals.m_iNWithGoodFocus;
        iIndexInData = i;
      }
    }
  }
  return iIndexInData;
}

WhatFigureFound ConturAsm::SetDataAboutContour(const CFigureFrame * pFrame)
{
  LPCTSTR pLabel = pFrame->GetLabel();
  bool bIsContur = (_tcsstr(pLabel, _T("Contur")) == pLabel);
  bool bIsSegment = (_tcsstr(pLabel, _T("EdgeSegment")) == pLabel);
  if (bIsContur)
  {
    if (_tcsstr(pLabel, _T("part")))
    {
      m_ObservationPartConturs.AddFrame(pFrame);
      //ReplaceFrame( (const CDataFrame **) &m_pPartConturOnObservation , pFrame ) ;
      return WFF_PartContur;
    }
    else if (_tcsstr(pLabel, _T("hole")))
    {
      ReplaceFrame((const CDataFrame **)&m_pHoleConturOnObservation, pFrame);
      return WFF_HoleContur;
    }
    // order of calib and calib_observ is important
    else if (_tcsstr(pLabel, _T("calib_observ")))
    {
      ReplaceFrame((const CDataFrame **)&m_pHoleConturOnObservation, pFrame);
      return WFF_CalibObservContur;
    }
    else if (_tcsstr(pLabel, _T("calib")))
    {
      ReplaceFrame((const CDataFrame **)&m_pHoleConturOnMeasPlace, pFrame);
      return WFF_CalibContur;
    }
    else if (_tcsstr(pLabel, _T("external")))
    {
      m_LastExtConturs.AddFrame(pFrame);
      return WFF_ExternalContur;
    }
    else if (_tcsstr(pLabel, _T("internal")))
    {
      ReplaceFrame((const CDataFrame **)&m_pLastInternalContur, pFrame);
      return WFF_InternalContur;
    }
  }
  else if (bIsSegment)
  {
    if (_tcsstr(pLabel, _T("ext_segment"))
      || _tcsstr(pLabel, _T("ext_big_segm")))
    {
      m_LastExtConturs.AddFrame(pFrame);
      return WFF_ExternalContur;
    }
  }
  else
  {
    if (_tcsstr(pLabel, _T("part")))
    {
      m_cPartCenterOnObservation = CDPointToCmplx(pFrame->GetAt(0));
      return WFF_PartCenter;
    }
    else if (_tcsstr(pLabel, _T("hole")))
    {
      m_cHoleCenterOnObservation = CDPointToCmplx(pFrame->GetAt(0));
      return WFF_HoleCenter;
    }
    else if (_tcsstr(pLabel, _T("calib_observ")))
    {
      //m_cHoleCenterOnMeasurement = CDPointToCmplx( pFrame->GetAt( 0 ) ) ;
      return WFF_CalibObservCenter;
    }
    else if (_tcsstr(pLabel, _T("calib")))
    {
      m_cHoleCenterOnMeasurement = CDPointToCmplx(pFrame->GetAt(0));
      return WFF_CalibCenter;
    }
  }
  return WFF_NotFound;
}

int ConturAsm::SetDataAboutContours(CFramesIterator * pFiguresIterator)
{
  if (!pFiguresIterator)
    return 0;
  DWORD dwFoundMask = 0;
  CFigureFrame * pNewFigure = NULL;
  int iNFound = 0;
  m_ObservationPartConturs.RemoveAll();
  m_LastExtConturs.RemoveAll();
  while (pNewFigure = (CFigureFrame*)pFiguresIterator->Next())
  {
    WhatFigureFound Res = SetDataAboutContour(pNewFigure);
    dwFoundMask |= Res;
    iNFound += ((Res & WFF_Conturs) != 0);
    LogFigure(pNewFigure->GetLabel(), Fig_Touch, __LINE__, pNewFigure->GetUserCnt());
  }
  m_dwLastFilledConturs = dwFoundMask;
  return iNFound;
}

CContainerFrame * ConturAsm::GetNewOutputContainer(const CDataFrame * pInput)
{
  CContainerFrame *pCreatedContainer = CContainerFrame::Create();
  pCreatedContainer->CopyAttributes(pInput);
  FXString ContainerLabel;
  ContainerLabel.Format(_T("C%d-%d-X%d-Y%d"),
    m_ConturData.GetCount(), m_LastScanFrames.GetCount(),
    ROUND(m_CurrentPos.m_x), ROUND(m_CurrentPos.m_y));
  pCreatedContainer->SetLabel(ContainerLabel);
  return pCreatedContainer;
}

bool ConturAsm::PutAndRegisterFrame(COutputConnector * pConnector,
  const CDataFrame * pFrame)
{
  //   bool bOur = ( ((pConnector == m_pLightControl) || (pConnector == m_pMarking))
  //     && pFrame->IsContainer()) ;
  //   SentFramesIds& Next = g_LCTRLSentContainers[ g_iIndexInLCTRL ] ;
  //   if ( bOur )
  //   {
  //     strcpy_s( Next.m_Label , pFrame->GetLabel() + 20 ) ;
  //     Next.m_Id = pFrame->GetId() ;
  //     Next.m_pFrame = pFrame ;
  //     Next.m_iNUsers = pFrame->GetUserCnt() ;
  //     Next.m_dwBegin = *((DWORD*)pFrame) ;
  //     g_dLastSendFrameTime = Next.m_dTime = GetHRTickCount() ;
  //     ((CDataFrame*)pFrame)->AddRef() ;
  //   }
  //   
  bool bRes = PutFrame(pConnector, (CDataFrame*)pFrame);
  //   if ( bOur )
  //   {
  //     Next.m_iNRUs = pFrame->GetUserCnt() ;
  //     if ( ++g_iIndexInLCTRL >= ARRSZ( g_LCTRLSentContainers ) )
  //       g_iIndexInLCTRL = 0 ;
  // 
  //     for ( int i = 0 ; i < 100 ; i++ )
  //     {
  //       int iIndex = (g_iIndexInLCTRL - 1 - i + ARRSZ( g_LCTRLSentContainers ))
  //         % ARRSZ( g_LCTRLSentContainers ) ;
  //       SentFramesIds& Passed = g_LCTRLSentContainers[ iIndex ] ;
  //       if ( Passed.m_dwBegin &&
  //         Passed.m_dwBegin == *((DWORD*)(Passed.m_pFrame)))
  //       {
  //         if ( g_dLastSendFrameTime > Passed.m_dTime + 20000. )
  //         {
  //           ((CDataFrame*)(Passed.m_pFrame))->Release() ;
  //           Passed.Reset() ;
  //         }
  //       }
  //     }
  // 
  //   }
  return bRes;
}


int ConturAsm::FormReportStringForUI(FXString& Result)
{
  ConturSample& LastSample = m_ConturData.GetCount() ?
    m_ConturData.GetAt(m_ConturData.GetUpperBound()) : m_OneSample;

  FXString Tmp;
  DWORD dwMarkColor;
  double dWidth;
  FXString TimeStamp = GetTimeAsString_ms();

  int iNGood = 0;
  for (int i = m_iLastSentToUI_Index; i < m_ConturData.GetCount(); i++)
  {
    ConturSample& LastSample = m_ConturData.GetAt(i);

    dWidth = LastSample.m_dAveBurrWidth_um;

    if (IsValidGradValue(dWidth))
      iNGood++;
    else
    {
      if (LastSample.m_iBadEdge != EQ_Normal)
        LastSample.m_iBadEdge = EQ_Invisible;
      dWidth = LastSample.m_dAveBurrWidth_um = 0.;
    }
    switch (LastSample.m_iBadEdge)
    {
    case EQ_Defects: dwMarkColor = 0x4040ff; break;
    case EQ_Invisible: dwMarkColor = 0xffffff; break;
    default:
      dwMarkColor = GetMarkColor(
        dWidth,
        m_dMaximumsForViewCsale[(int)m_ResultViewMode] , 0. );
      break;
    }

    Tmp.Format("Pt#=%d Width=%.1f Ready=%d%% "
      "Robot(%d,%d,%d) Color=0x%06X "
      "Mode=%s T=%s\n",
      i, dWidth, m_iPercentOfMeasured,
      ROUND(LastSample.m_MiddlePt.real()), ROUND(LastSample.m_MiddlePt.imag()),
      ROUND(LastSample.m_RobotPos.m_z), dwMarkColor,
      fabs(m_dLastMaxWidth_um - dWidth) < 0.05 ? "Max" : "Val", (LPCTSTR)TimeStamp);
    Result += Tmp;
  }
  m_iLastSentToUI_Index = (int)m_ConturData.GetCount();

  if (iNGood < m_iNMinGoodSamples)
    dwMarkColor = 0xffffff;
  else
  {
    dwMarkColor = GetMarkColor(
      dWidth,
      m_dMaximumsForViewCsale[(int)m_ResultViewMode] , 0. );
  }
  Tmp.Format("Pt#=%d Width=%.1f Ready=%d%% "
    "Robot(%d,%d,%d) Color=0x%06X "
    "Mode=%s T=%s\n",
    LastSample.m_iSegmentNumInScan,
    m_dLastAverageWidth_um, m_iPercentOfMeasured,
    ROUND(m_CurrentPos.m_x), ROUND(m_CurrentPos.m_y),
    ROUND(m_CurrentPos.m_z), dwMarkColor,
    "Ave", (LPCTSTR)TimeStamp);
  Result += Tmp;

  return (int)Result.GetLength();
}


int ConturAsm::SetWorkingLed(Directions Dir, bool bFront, int iLedNum)
{
  int iLight = 0;
  if (Dir != H_UNKNOWN)
    iLight = GetLightDir(Dir, bFront);
  else if (iLedNum >= 0 && iLedNum < 8)
    iLight = 1 << iLedNum;
  else
    return 0;

  DWORD OutMask = m_iCameraMask | iLight;
  int iLightTime = (bFront) ? m_iExtLight_us : m_iIntLight_us;
  FXString LightOut;
  LightOut.Format("out 0x%04X %d\r\n", OutMask, iLightTime);
  CTextFrame * pLightControl = CTextFrame::Create(LightOut);
  pLightControl->SetLabel("LightControl");

  if (!PutFrame(m_pLightControl, pLightControl))
  {
    SEND_GADGET_ERR("Can't set working LEDs on motion end");
  }
  else
  {
    m_LastLightMask = iLight;
    m_sLastLightCommand = LightOut;
    m_iLastPulseTime = iLightTime;
    SetWatchDog((iLightTime / 1000) + 200);
  }
  TRACE("\nLIght out by dir=%d F=%d Mask=%0x04X Tl=%d",
    Dir, (int)bFront, OutMask, iLightTime);
  return iLight;
}

int ConturAsm::SetWorkingLed(DWORD dwMask, int iLightTime)
{
  DWORD OutMask = m_iCameraMask | (dwMask & 0x3ff); // 8 leds in ring and 2 leds for observation
  FXString LightOut;
  LightOut.Format("out 0x%04X %d\r\n", OutMask, iLightTime);
  CTextFrame * pLightControl = CreateTextFrame(LightOut, "LightControl");

  if (!PutFrame(m_pLightControl, pLightControl))
  {
    SEND_GADGET_ERR("Can't set working LEDs on motion end");
    return 0;
  }
  else
  {
    m_LastLightMask = OutMask;
    m_sLastLightCommand = LightOut;
    m_iLastPulseTime = iLightTime;
    SetWatchDog((iLightTime / 1000) + 200);
  }

  TRACE("\nLight out by Mask=0x%04X Tl=%d", OutMask, iLightTime);
  return OutMask;
}

int ConturAsm::SetWorkingLeds(DWORD dwCamera, DWORD dwLEDMask, int iLightTime)
{
  // Cameras are 0x0400 (observation) and 0x0800 (measurement)
  // Observation LEDs are 0x0300
  // LED ring is in bits 0x00ff
  DWORD OutMask = (1 << (dwCamera + 10)) | (dwLEDMask & 0x3ff); // 8 leds in ring and 2 leds for observation
  FXString LightOut;
  LightOut.Format("out 0x%04X %d\r\n", OutMask, iLightTime);
  CTextFrame * pLightControl = CreateTextFrame(LightOut, "LightControl");

  if (!PutFrame(m_pLightControl, pLightControl))
  {
    SEND_GADGET_ERR("Can't set working LEDs on motion end");
    return 0;
  }
  else
  {
    m_LastLightMask = OutMask;
    m_sLastLightCommand = LightOut;
    m_iLastPulseTime = iLightTime;
    SetWatchDog((iLightTime / 1000) + 200);
  }

  TRACE("\n%s:  Light out by Mask=0x%04X Tl=%d",
    GetStageName(),
    OutMask, iLightTime);
  return OutMask;
}

bool ConturAsm::DeleteWatchDog()
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
  m_WatchDogMessage.Empty() ;
  m_LockWatchdog.Unlock();
  return true;
}

VOID CALLBACK TimerRoutine(LPVOID lpParam, BOOLEAN TimerOrWaitFired)
{
  ConturAsm * pGadget = (ConturAsm*)lpParam;
  //   if ( pGadget->m_pStatus->GetStatus() == CExecutionStatus::RUN )
  //   {
  
  CTextFrame * pTimeoutNotification = CreateTextFrame("Timeout", "Timeout");
  pGadget->GetInputConnector(0)->Send(pTimeoutNotification);
  //   }
  pGadget->DeleteWatchDog();
}


int ConturAsm::SetWatchDog(int iTime_ms , LPCTSTR pMsg )
{
  DeleteWatchDog();
  if (!CreateTimerQueueTimer(&m_hWatchDogTimer, NULL, (WAITORTIMERCALLBACK)TimerRoutine,
    this, iTime_ms, 0, 0))
  {
    SEND_GADGET_ERR("Create Watchdog timer failed");
    return false;
  }
  if ( pMsg )
    m_WatchDogMessage = pMsg ;

  return true;
}



int ConturAsm::GetFirstSampleInSegment(int iIndexInSegment)
{
  if (iIndexInSegment < m_ConturData.Count()
    && iIndexInSegment >= 0)
  {
    for (int i = iIndexInSegment; i >= 0; i--)
    {
      if (m_ConturData[i].m_iSubindex == 0)
        return i;
    }
  }
  return -1;
}

int ConturAsm::AddNewSampleToContur(OneMeasPt& Pt,
  int iIndex, int iSegmentNumber, FXString * pForReporting)
{
  ConturSample* PrevSample = (m_ConturData.GetCount()) ?
    &m_ConturData.GetLast() : NULL;

  cmplx cPtOnScreenInRobotCoords = GetPtPosInRobotCoords(Pt);

  if (m_ConturData.GetCount() == 0)
    m_cInitialRobotPosForMeas = (cmplx)m_CurrentPos;
  ConturSample NewSample((int)m_ConturData.GetCount(),
    Pt.m_RobotPos, cPtOnScreenInRobotCoords,
    Pt.m_dBurrValue_um,
    Pt.m_dAverFocus, Pt.m_dMaxFocus);
  NewSample.m_iSubindex = iIndex;
  NewSample.m_iSegmentNumInScan = iSegmentNumber;
  NewSample.m_cEdgePt = Pt.m_cEdgePt;
  // for segment turn calculation on next measurement
  // and for off line viewing
  NewSample.m_cSegmentVect = Pt.m_cEdgeDir;

  //                       NewSample.m_cPartCenter = -GetVectToPartCent() ;
  NewSample.m_cPartCenter =
    cmplx(m_ccMeasCenter.m_x, m_ccMeasCenter.m_y);
  if (PrevSample)
  {
    double dDist = abs(NewSample.m_MiddlePt - PrevSample->m_MiddlePt);
    NewSample.m_dEdgeWithBurrLen_um = dDist;
  }
  else
    NewSample.m_dEdgeWithBurrLen_um = 0.;

  NewSample.m_iBadEdge = Pt.m_dBurrValue_um == 0. ?
    EQ_Defects : EQ_Normal;
  NewSample.m_dLastTime = ((Pt.m_dSampleTime != 0.) ?
    Pt.m_dSampleTime : GetHRTickCount())
    - m_dMeasurementStartTime;
  NewSample.m_RobotPos.m_theta -= m_dMeasurementStartTime;
  ASSERT(NewSample.m_dLastTime > 0.);
  m_ConturData.Add(NewSample);
  if (pForReporting)
  {
    FXString OnePtReport;
    NewSample.ToString(OnePtReport, NULL, "\n");
    *pForReporting += OnePtReport;
  }
  if (NewSample.m_iBadEdge == EQ_Normal)
  {
    if (m_dLastMaxWidth_um < Pt.m_dBurrValue_um)
    {
      m_dLastMaxWidth_um = Pt.m_dBurrValue_um;
      m_iIndexWithMaxWidth = NewSample.m_iIndex;
      if (m_dLastScanMaxWidth_um < Pt.m_dBurrValue_um)
      {
        m_dLastScanMaxWidth_um = Pt.m_dBurrValue_um;
        m_iIndexWithMaxWidth = NewSample.m_iIndex;
      }
    }
  }
  //                     }
  return (int)m_ConturData.Count();
}


// Check Last results (m_CurrentViewMeasStatus) for spikes and remove them
int ConturAsm::FilterLastResults()
{
  int iLeftInd = m_CurrentViewMeasStatus.m_iMinusIndex;
  int iRightInd = m_CurrentViewMeasStatus.m_iPlusIndex;
  if (iLeftInd == iRightInd)
    return 0;

  OneMeasPt * pPrevPrev = NULL, *pPrev = NULL,
    *pCurrent = m_CurrentViewMeasStatus.m_Results,
    *pNext = (iRightInd > 1) ? pCurrent + 1 : NULL,
    *pNextNext = (iRightInd > 2) ? pNext + 1 : NULL;
  OneMeasPt * pLast = pCurrent + m_CurrentViewMeasStatus.m_iPlusIndex;
  double dCurrVal = pCurrent->m_dBurrValue_um;
  double dNextVal = pNext->m_dBurrValue_um;
  double dPrevVal = 0.;
  bool bCurrOK = IsValidGradValue(dCurrVal) && (pCurrent->m_dAverFocus >= 200);
  bool bNextOK = IsValidGradValue(dNextVal) && (pNext->m_dAverFocus >= 200);
  bool bPrevOK = false;

  while (pNext)
  {
    dNextVal = pNext->m_dBurrValue_um;
    bNextOK = IsValidGradValue(dNextVal) && (pNext->m_dAverFocus >= 200);

    if (!bCurrOK )
      pCurrent->m_dBurrValue_um = 0.;
    else if (!bPrevOK && !bNextOK)
    {
      dCurrVal = pCurrent->m_dBurrValue_um = 0.;
      bCurrOK = false;
    }
    else // current is OK, Prev or Next are OK
    {
      // check for edge direction turn
      if ( abs(pCurrent->m_cEdgeDir) > 0.
        && abs(pNext->m_cEdgeDir) > 0.)
      {
        cmplx div = pCurrent->m_cEdgeDir / pNext->m_cEdgeDir;
        double dInclination = arg(div);
        if (abs(dInclination) > M_PI_4)
          pCurrent->m_dBurrValue_um = 0.;
        else if (bPrevOK && bNextOK)// turn is not very big, check for values
        {
          double dDiffToPrev = fabs(dCurrVal - dPrevVal) / dCurrVal;
          double dDiffToNext = fabs(dCurrVal - dNextVal) / dCurrVal;

          bool bDiffPrevOK = IsInRange(dDiffToNext, -0.3, 0.3);
          bool bDiffNextOK = IsInRange(dDiffToNext, -0.3, 0.3);
          if (bDiffNextOK)
          {
            if (bDiffPrevOK)
            {
            }
            else
            {
              if (pLast - pNext > 1)
              {
                pNextNext = pNext + 1;
                double dNextNextVal = pNextNext->m_dBurrValue_um;
                bool bNextNextOK = IsValidGradValue(dNextNextVal) && (pNextNext->m_dAverFocus >= 200);
                if (bNextNextOK)
                {
                  double dDiffNextToNext = fabs(dNextVal - dNextNextVal) / dNextVal;
                  bool bDiffNextToNextOK = IsInRange(dDiffNextToNext, 0.75, 1.25);
                  if (!bDiffNextToNextOK)
                    dCurrVal = pCurrent->m_dBurrValue_um = 0.;
                }
              }
            }
          }
          else //bDiffNextOK == false
          {
            if (!bDiffPrevOK)
              dCurrVal = pCurrent->m_dBurrValue_um = 0.;
          }
        }
      }
    }
    pPrevPrev = pPrev;
    pPrev = pCurrent;
    dPrevVal = dCurrVal;
    bPrevOK = bCurrOK;

    pCurrent = pNext;
    dCurrVal = dNextVal;
    bCurrOK = bNextOK;

    pNext = (pNext < pLast) ? ++pNext : NULL;
  }
  return 0;
}


CContainerFrame * ConturAsm::CreateCombinedResultContainer(
  const CVideoFrame * pOptimalImage, const CDataFrame * pInputDataFrame)
{
  CContainerFrame * pResult = GetNewOutputContainer(pInputDataFrame);
  if (pResult)
  {
    pResult->AddFrame(pOptimalImage);

    CFigureFrame * pExt = CFigureFrame::Create();
    *(pExt->Attributes()) = _T("color=0x0000ff");
    pExt->SetLabel("EdgePoints");
    CFigureFrame * pInt = CFigureFrame::Create();
    *(pInt->Attributes()) = _T("color=0x00ff00");
    pInt->SetLabel("BurrEndPts");

    int iLeftInd = m_CurrentViewMeasStatus.m_iMinusIndex;
    int iRightInd = m_CurrentViewMeasStatus.m_iPlusIndex;
    OneMeasPt * pCurrent = m_CurrentViewMeasStatus.m_Results;
    cmplx cEdgeDir = pCurrent->m_cEdgeDir;
    cmplx cPrevEdgeDir = cEdgeDir;
    for (int i = iLeftInd; i < iRightInd; i++)
    {
      double dBurrWidth_pix = pCurrent->m_dBurrValue_um * m_dScale_pix_per_um;
      cmplx cEdgePt = pCurrent->m_cEdgePt;
      cmplx cNextEdgeDir = (i < iRightInd - 1) ?
        (pCurrent + 1)->m_cEdgeDir : cEdgeDir;
      double dEdgeLength_pix = abs(cEdgeDir);
      bool bNotSmallAndGood = (dEdgeLength_pix >= 0.5);
      if (bNotSmallAndGood)
      {
        double dAngleToPrev = arg(cPrevEdgeDir / cEdgeDir);
        double dAngleToNext = arg(cNextEdgeDir / cEdgeDir);
        bNotSmallAndGood = !((fabs(dAngleToNext) > M_PI_8)
          || (fabs(dAngleToPrev) > M_PI_8));
      }
      pExt->AddPoint(CmplxToCDPoint(cEdgePt));
      if (bNotSmallAndGood)
      {
        cmplx cBurrDir = -GetOrthoRight(cEdgeDir) / dEdgeLength_pix; // with normalize
        cmplx cBurrWidth = cBurrDir * dBurrWidth_pix;
        cmplx cBurrPt = (dBurrWidth_pix != 0.) ? cEdgePt + cBurrWidth : cEdgePt;
        pInt->AddPoint(CmplxToCDPoint(cBurrPt));
      }
      else // not smooth edge, reset burr width and put burr pt on edge
      {
        pCurrent->m_dBurrValue_um = 0.;
        pInt->AddPoint(CmplxToCDPoint(cEdgePt));
      }
      cPrevEdgeDir = cEdgeDir;
      pCurrent++;
      cEdgeDir = pCurrent->m_cEdgeDir;
    }
    *(pExt->Attributes()) = _T("color=0x0000ff;");
    pExt->SetLabel("EdgePoints");
    *(pInt->Attributes()) = _T("color=0x00ff00;");
    pInt->SetLabel("BurrEndPts");
    pResult->AddFrame(pExt);
    pResult->AddFrame(pInt);
  }
  else
    SENDERR("ConturAsm::CreateCombinedResultContainer: Can't make container");


  return pResult;
}

double ConturAsm::GetAverageOfSegmentForPt(int iPtIndex)
{
  double dAve = 0.0;
  int iNValid = 0;
  if (iPtIndex >= 0 && iPtIndex < m_ConturData.Count())
  {
    int iBegin = iPtIndex;
    int iSegmentNumber = m_ConturData[iBegin].m_iSegmentNumInScan;
    while (m_ConturData[iPtIndex].m_iSegmentNumInScan
      == iSegmentNumber)
    {
      double dBurrVal = m_ConturData[iPtIndex].m_dAveBurrWidth_um;
      if (IsValidGradValue(dBurrVal))
      {
        dAve += dBurrVal;
        iNValid++;
      }
      if (!(iPtIndex = (++iPtIndex % m_ConturData.Count())))
        break;
    }
    if (iPtIndex)
    {
      iPtIndex = (int)(iBegin - 1 + m_ConturData.Count()) % m_ConturData.Count();
      while (m_ConturData[iPtIndex].m_iSegmentNumInScan
        == iSegmentNumber)
      {
        double dBurrVal = m_ConturData[iPtIndex].m_dAveBurrWidth_um;
        if (IsValidGradValue(dBurrVal))
        {
          dAve += dBurrVal;
          iNValid++;
        }
        iPtIndex = (int)(--iPtIndex + (int)m_ConturData.Count()) % (int)m_ConturData.Count();
      }
    }
    if (iNValid)
      dAve /= iNValid;
  }

  return dAve;
}


int ConturAsm::MarkSector(int iPtIndex, cmplx cPartCEnter, double SectorLength_um)
{

  return 0;
}


FXString ConturAsm::CheckSendFoundPartInfoToYuri(
  LPCTSTR psPartId, int iHeight_um, LPCTSTR psPartDir,
  LPCTSTR psErr, bool bIsNewPart)
{
  FXString Msg;
  bool bFinishMsgIsNecessary = false;
  if (psPartId)
  {
    if (psErr) // Some error happens
    {
      LPCTSTR pAskOperator = (bIsNewPart) ?
        "Ask Operator=\"Create New?\"" : "";

      Msg.Format("Found Part=%s, Height=%d microns T=%s "
        "Dir=\"%s\" %s Err=\"%s\"",
        psPartId, iHeight_um, GetTimeAsString(),
        psPartDir ? psPartDir : "Not Assigned", pAskOperator, psErr);
      bFinishMsgIsNecessary = true;
    }
    else if (!bIsNewPart) // Normal expected part
    {
      Msg.Format("Found Part=%s, Height=%d microns T=%s "
        "Dir=\"%s\" ",
        psPartId, iHeight_um, m_MeasurementTS, psPartDir);
    }
    else // New part
    {
      Msg.Format("Found Part=%s, Height=%d microns T=%s "
        "Dir=\"%s\" Data Base updated",
        psPartId, iHeight_um, m_MeasurementTS,
        psPartDir ? psPartDir : "Not Assigned", psErr);
    }
  }
  else
  {
    Msg.Format("Found Part=UNKNOWN, Operation stopped T=%s ",
      GetTimeAsString());
    bFinishMsgIsNecessary = true;
  }

  CTextFrame * pMsg = CreateTextFrame(
    (LPCTSTR)Msg,
    _T("Result"));
  PutFrame(m_pOutput, pMsg);
  if (bFinishMsgIsNecessary)
  {
    CTextFrame * pEndOfScan = CreateTextFrame(
      _T("Scan Finished\n Worst Burr is UNKNOWN"),
      _T("Result"));

    PutFrame(m_pOutput, pEndOfScan);
  }


  return Msg;
}


CTextFrame * ConturAsm::FormScanFinishMessage(LPCTSTR pLabel, bool bFromFile)
{
  double dWorstResult = 0.;
  int iWorstIndex = 0;
  for (int i = 0; i < m_ConturData.Count(); i++)
  {
    if (m_ConturData[i].m_dAveBurrWidth_um > dWorstResult)
    {
      double dAver = 0.;
      for (int j = i - 2; j <= i + 2; j++)
      {
        if (j == i)
          continue;
        dAver += m_ConturData[(j + m_ConturData.Count()) % m_ConturData.Count()].m_dAveBurrWidth_um;
      }
      dAver /= 4.;
      if (dAver >= m_ConturData[i].m_dAveBurrWidth_um * 0.75)
      {
        dWorstResult = m_ConturData[i].m_dAveBurrWidth_um;
        iWorstIndex = i;
      }
    }
  }
  m_dLastScanMaxWidth_um = dWorstResult;
  m_iIndexWithMaxWidth = iWorstIndex;
  FXString FinishMsg;
  if (bFromFile)
  {
    FinishMsg.Format(
      _T("Scan Finished\n Worst Burr is %.1f um Pt#=%d\n"
        "Scan Time=0.0 seconds \n PO=%s"),
      m_dLastScanMaxWidth_um, m_iIndexWithMaxWidth, (LPCTSTR)m_PO);
  }
  else
  {
    FinishMsg.Format(
      _T("Scan Finished\n Worst Burr is %.1f um Pt#=%d\n"
        "Scan Time=%.1f seconds \n PO=%s"),
      m_dLastScanMaxWidth_um, m_iIndexWithMaxWidth,
      m_dLastScanTime_ms * 0.001, (LPCTSTR)m_PO);
  }
  CTextFrame * pFinishMsg = CreateTextFrame(FinishMsg,
    pLabel, (int)m_ConturData.GetCount());
  pFinishMsg->Attributes()->Format("color=0x%06x; Sz=%d; x=%d; y=%d;",
    0xc000c0, 16,
    ROUND(m_cFinishingMsgPt.real()), ROUND(m_cFinishingMsgPt.imag()));
  return pFinishMsg;
}


int ConturAsm::CheckObservationContur(CContainerFrame * pOutData,
  const CDataFrame * pOrigin, bool bScanOperation)
{
  if ((m_dwLastFilledConturs & WFF_PartContur)
    //           && (m_dwLastFilledConturs & WFF_PartCenter)
    && m_ObservationPartConturs.GetCount())
  {
    FXString ForReporting;
    const CFigureFrame * pObservationContur = (const CFigureFrame *)m_ObservationPartConturs.GetFrame(0);
    FR_RELEASE_DEL(m_pFilteredObservationContur);
    m_pFilteredObservationContur = (CFigureFrame*)pObservationContur->Copy();

    //int iNRemoved = RemoveSmallOutGrowths( *m_pFilteredObservationContur , 5 , M_PI / 8 , 150 ) ;
    ReplaceFrame((const CDataFrame**)&m_pPartConturOnObservation,
      m_pFilteredObservationContur);
    LogFigure("Observation contur replaced", Fig_Touch,
      __LINE__, m_pPartConturOnObservation->GetUserCnt());
    m_dPartConturLength =
      GetFigureLength(m_pPartConturOnObservation) / m_dObservScale_pix_per_um;
    m_cPartCenterOnObservation = FindExtrems(
      (CFigure*)m_pPartConturOnObservation, m_ObservationExtremes);
    cmplx cLeftPt = m_ObservationExtremes[0]; // Xmin
    if (abs(cLeftPt) == 0.) // not found
    {
      SEND_GADGET_ERR("Can't find observation conturs");
      SetProcessingStage(Stage_Inactive, true);
      return 0;
    };

    pOutData->SetLabel("MarkedObservation");
    //pOutData->AddFrame( pImage ) ;
    pOutData->AddFrame(m_pPartConturOnObservation);

    FXString InternalCatalogID;
    FXString PO;
    FXPropertyKit ForPars(!m_PartFile.IsEmpty() ? m_PartFile
      : !m_NewPartName.IsEmpty() ? m_NewPartName : _T(""));
    if (!ForPars.IsEmpty())
    {
      ForPars.Replace(',', ';');
      if ( ForPars.GetString( "PO" , PO ) )
        m_PO = PO;
      else if ( m_iSimulationMode )
        m_PO = "Simulation" ;


      ForPars.GetString("ID", InternalCatalogID);
      if (InternalCatalogID.IsEmpty())
      {
        int iUnderscorePos = (int) ForPars.Find(_T('_'));
        if (iUnderscorePos > 0)
          InternalCatalogID = ForPars.Left(iUnderscorePos);
      }
      m_InternalCatalogID = InternalCatalogID;
    }


    int iNVertices = m_LastObservationForm.SetData(m_NewPartName, m_dPartHeight,
      (const CFigure*)m_pPartConturOnObservation, m_dObservScale_pix_per_um, true, InternalCatalogID, m_PO, m_MeasurementTS);
    m_LastObservationForm.m_dLastOptimalZ = m_CurrentPos.m_z; //m_CurrentViewMeasStatus.m_Results[ m_CurrentViewMeasStatus.m_iRightMeasured ].m_RobotPos.m_z ;

    if (iNVertices >= 0)
    {
      FR_RELEASE_DEL(m_VectorsToVertices);
      m_VectorsToVertices = CContainerFrame::Create();
      m_VectorsToVertices->SetLabel("VectorsToVertices");
      if (iNVertices > 1) // not round form
      {
        for (int i = 0; i < m_LastObservationForm.m_Vertices.GetCount(); i++)
        {
          cmplx VertexInPixels = m_LastObservationForm.m_CalculatedCenter
            + (m_LastObservationForm.m_Vertices[i] * m_dObservScale_pix_per_um);
          COLORREF Color = (m_LastObservationForm.m_Vertices[i] == m_LastObservationForm.GetMainDirection()) ?
            0xff00ff : 0x00ffff;
          CFigureFrame * pToVertice = CreateLineFrame(
            m_LastObservationForm.m_CalculatedCenter, VertexInPixels,
            Color, "Vertice");
          m_VectorsToVertices->AddFrame(pToVertice);
        }
      }
      else // round form, radius in Vertices[0] as real
      {
        cmplx OnEdge = m_LastObservationForm.m_CalculatedCenter +
          m_LastObservationForm.m_Vertices[0] * m_dObservScale_pix_per_um;
        CFigureFrame * pToVertice = CreateLineFrame(
          m_LastObservationForm.m_CalculatedCenter, OnEdge, (COLORREF)0x00ffff, "Radius");
        pOutData->AddFrame(pToVertice);
        FXString Description;
        Description.Format("R=%.1fum", m_LastObservationForm.m_dAverDist);
        CTextFrame * DescrFrame = CreateTextFrame(m_LastObservationForm.m_CalculatedCenter,
          (LPCTSTR)Description, "0x00ffff", 14, _T("Description"));
        m_VectorsToVertices->AddFrame(DescrFrame);
      }
      m_VectorsToVertices->AddRef();
      pOutData->AddFrame(m_VectorsToVertices);
    }
    m_iIndexOfFoundObservForm = -1;
    FXString FoundAndIdView;
    FXString PartIdInFound;



    //**************************************************************************************** 
    // Part identification and processing logic
    // Contour exists  m_PartFile   m_NewPartName     Found       FoundOther   Reaction
    //1.    Yes        Not Empty    Not important      Yes          No        Normal processing +
    //2.    Yes         Empty         Empty            No           Yes       Measurement (for result only) +
    //3.    Yes         Empty         Empty            No           No        Measurement (for result only) +
    //4.    Yes         Empty       Not Empty          Yes          No        Normal Processing (no new part) +
    //5.    Yes        Not Empty    Not important      No           Yes       Stop (order error) +
    //6.    Yes        Not Empty    Not important      No           No        Normal Processing (Part id on operator responsibility)+
    //7.    Yes         Empty       Not Empty          No           Yes       Normal Processing (create new part)+
    //8.    Yes         Empty       Not Empty          No           No        Normal Processing (create new part)+
    //9.     No       Not important  Not important       Not important         Stop (No part)+

    //10.   Yes  m_PartFile and/or m_NewPartName are not in the list of known forms and not identified - new part will be created

    // Case 9 (no contour) is processed before

    if (m_ObservGeomAnalyzer.GetNKnownForms() || m_iSimulationMode )
    {
      FXString FoundParts;
      double dFoundHeight = 0.;
      FXIntArray FoundIndexes;
      m_iIndexOfFoundObservForm = m_ObservGeomAnalyzer.FindFormByID(m_InternalCatalogID);

      if ((m_iIndexOfFoundObservForm < 0) && m_NewPartName.IsEmpty() && !m_iSimulationMode ) // no such part
      {
        if (m_CurrentPartStatus == Part_Unknown)
        {
          FoundAndIdView.Format("Part is not found\n"
            "and is not correspondent expected %s\n"
            "Check PO# and try again.  STOP OPERATION",
            (LPCTSTR)InternalCatalogID);
          FXString Error;
          Error.Format("Part is not found"
            "the expected Item Code (%s). Check PO# or change part or create new and try again",
            (LPCTSTR)InternalCatalogID);
          CheckSendFoundPartInfoToYuri(
            NULL,
            4000,
            NULL, Error, true);

          cmplx TextPt = m_ObservationExtremes[1] + cmplx(-100., -100.);
          CTextFrame * pMatchResult = CreateTextFrame(TextPt, (LPCTSTR)FoundAndIdView,
            "0x0000ff", 10, "MatchResult");
          pOutData->AddFrame(pMatchResult);
          pOutData->SetLabel("MarkedObservation");
          PutAndRegisterFrame(m_pOutput, pOutData);
          return 0;
          //             }
          //             else  // Case 10 : New part will be created
          //             {
          //             }
          //          }
        }
      }
      if ( (m_iIndexOfFoundObservForm >= 0) || m_iSimulationMode ) // form is identified
      {
        //ys_20201003 What is this?
        //m_IdentifiedObservForm = m_ObservGeomAnalyzer[ m_iIndexOfFoundObservForm ] ;
        //FXSIZE iPosInFound = m_IdentifiedObservForm.m_Name.Find( _T( '_' ) ) ;
        //PartIdInFound = m_IdentifiedObservForm.m_Name.Mid( 0 , iPosInFound ) ;
        //ys_20201003 What is this?

        if ( m_iSimulationMode )
        {
          m_dPartHeight = 6000. ;
          if ( m_iIndexOfFoundObservForm < 0 )
          {
            m_IdentifiedObservForm.m_Name = m_PartCatalogName ;
            m_IdentifiedObservForm.m_dHeight_um = m_dPartHeight ;
            m_IdentifiedObservForm.m_bIsActive = true ;
            m_IdentifiedObservForm.m_dLastOptimalZ = 75000. ;
            m_IdentifiedObservForm.m_dPerimeter_um = m_dPartConturLength ;
            m_IdentifiedObservForm.m_sPartID = m_PO ;
            m_IdentifiedObservForm.m_sPONumber = m_PO ;
          }
        }
        else
        {
          m_IdentifiedObservForm = m_ObservGeomAnalyzer[ m_iIndexOfFoundObservForm ];
          ASSERT( !m_IdentifiedObservForm.m_Name.IsEmpty() );
          if ( m_IdentifiedObservForm.m_sPONumber.IsEmpty() && !m_LastObservationForm.m_sPONumber.IsEmpty() )
            m_IdentifiedObservForm.m_sPONumber = m_LastObservationForm.m_sPONumber;
          if ( m_IdentifiedObservForm.m_sPartID.IsEmpty() && !m_LastObservationForm.m_sPartID.IsEmpty() )
            m_IdentifiedObservForm.m_sPartID = m_LastObservationForm.m_sPartID;
        }

        cmplx TextPt = m_ObservationExtremes[1] + cmplx(-60., -80.);
        CTextFrame * pMatchResult = CreateTextFrame(TextPt, (LPCTSTR)FoundAndIdView,
          "0x0000ff", 10, "MatchResult");
        pOutData->AddFrame(pMatchResult);
        pOutData->SetLabel("MarkedObservation");
        PutAndRegisterFrame(m_pOutput, pOutData);

        if ( !m_iSimulationMode )
          m_LastObservationForm.m_Name = m_PartFile = m_IdentifiedObservForm.m_Name;
        m_LastObservationForm.m_dHeight_um = m_dPartHeight /*= m_IdentifiedObservForm.m_dHeight_um*/;
        if ( !m_iSimulationMode )
          CheckCreatePartResultDir();

        GetCurrentDetailedForm();

        if (bScanOperation)
        {
          CheckSendFoundPartInfoToYuri(
            (LPCTSTR)m_IdentifiedObservForm.m_Name,
            ROUND(m_IdentifiedObservForm.m_dHeight_um),
            (LPCTSTR)m_ResultDirectory);
        }
      }
      else
      {
        FoundAndIdView.Format("Found %s, Expected assigned %s ",
          (LPCTSTR)PartIdInFound, (LPCTSTR)PartIdInFound);
      }
    }
    if ( m_iSimulationMode )
      return 1 ;

    if (m_iIndexOfFoundObservForm < 0)
    {
      if (m_NewPartName.IsEmpty())
      {
        if (bScanOperation)
        {
          CheckSendFoundPartInfoToYuri(NULL);
          SetProcessingStage(Stage_Inactive, true, " Part Not Found ", true);
          m_UnknownObservationForm = m_LastObservationForm;
        }
      }
      else
      {
        if (m_ObservGeomAnalyzer.ContainsFormName(m_NewPartName))
        {
          if (bScanOperation)
          {
            FXString Error;
            Error.Format("The part with name %s is ALREADY EXISTS. "
              "New part is not added. Check PO# or change part and try again.", (LPCTSTR)m_NewPartName);
            CheckSendFoundPartInfoToYuri(
              (LPCTSTR)m_IdentifiedObservForm.m_Name,
              ROUND(m_IdentifiedObservForm.m_dHeight_um),
              m_PartDirectory, Error);

            SetProcessingStage(Stage_Inactive, true, ForReporting, true);
          }
        }
        else
        {
          ASSERT( m_dPartHeight < 40000 ) ;
          ForReporting.Format("New part name %s will be used Height=%d microns"
            " T=%s",
            (LPCTSTR)(m_LastObservationForm.m_Name = m_NewPartName),
            ROUND(m_LastObservationForm.m_dHeight_um = m_dPartHeight), (LPCTSTR)GetTimeAsString());
          m_NewPartName.Empty();
          m_PartFile = m_LastObservationForm.m_Name;
          ASSERT( m_dPartHeight < 40000 ) ;
          CheckCreatePartResultDir();

          if (m_SetupObject && m_SetupObject->IsOn())
          {
          }
        }
      }
    }
    if (!ForReporting.IsEmpty() && bScanOperation)
    {
      CTextFrame * pResult = CreateTextFrame(ForReporting, _T("Result"),
        pOrigin->GetId());
      PutFrame(m_pOutput, pResult);
    }
    if (!IsInactive()) // part is not identified and new part field is empty
      return 1;
  }
  return 0;
}

bool ConturAsm::GetCurrentDetailedForm()
{
  bool res = false;
  m_PartDetailsFileName = m_PartDirectory + m_PartFile + _T("_Details.dat");
  if ( !m_iSimulationMode )
  {
    bool bRes = m_CurrentDetailedForm.GetFromFile( ( LPCTSTR ) m_PartDetailsFileName );

    if ( bRes )
      res = bRes;
    else if ( !m_iSimulationMode )// No detailed description, create file in current part directory
    {
      FXPropKit2 DetailedFormDescription = m_LastObservationForm.ToString();
      if ( !DetailedFormDescription.WriteToFile( m_PartDetailsFileName ) )
      {
        SENDERR( "Can't write file %s for detailed form save: %s " ,
          ( LPCTSTR ) m_PartDetailsFileName , _tcserror( GetLastError() ) );
      }

      res = m_CurrentDetailedForm.FromString( DetailedFormDescription ) > 0;
    }

  }

  return res;
}

cmplx ConturAsm::GetPtPosInRobotCoords(OneMeasPt& Pt)
{
  cmplx cRelToFOVCenter = Pt.m_cEdgePt - m_cMeasFOVCent;
  cRelToFOVCenter /= m_dScale_pix_per_um;
  cmplx cPtOnScreenInRobotCoords = cmplx(Pt.m_RobotPos.m_x, Pt.m_RobotPos.m_y)
    - cRelToFOVCenter;
  return cPtOnScreenInRobotCoords;
}


int ConturAsm::ChangeAverageRange(bool bPlus)
{
  int iNKnownForms = m_ObservGeomAnalyzer.Init();
  FXRegistry Reg("TheFileX\\ConturASM");
  int iRangeMax = Reg.GetRegiInt("Parameters", "AveragingRangeMax_um", 1000);
  int iRangeStep = Reg.GetRegiInt("Parameters", "AveragingRangeStep_um", 25);
  if (bPlus)
  {
    if ((m_iAveragingRange_um += iRangeStep) > iRangeMax)
      m_iAveragingRange_um = iRangeMax;
  }
  else if ((m_iAveragingRange_um -= iRangeStep) < iRangeStep)
    m_iAveragingRange_um = iRangeStep;
  return m_iAveragingRange_um;
}


double ConturAsm::GetAverageAroundPt(int iPtNum, double dRange_um,
  CPoint& PtOnMapBackward, CPoint& PtOnMapForward)
{
  double dSum = 0.;
  int iNPoints = 0;
  if (!m_ConturData.IsEmpty() && iPtNum < m_ConturData.Count())
  {
    ConturSample * Pt = m_ConturData.GetData() + iPtNum;
    ConturSample * pInitialPt = Pt;
    if (Pt->m_dAveBurrWidth_um > 0.)
    {
      dSum += Pt->m_dAveBurrWidth_um;
      iNPoints++;
    }
    do
    {
      Pt++;
      if (Pt - m_ConturData.GetData() >= m_ConturData.Count())
        Pt = m_ConturData.GetData();
      if (Pt->m_dAveBurrWidth_um > 0.)
      {
        dSum += Pt->m_dAveBurrWidth_um;
        iNPoints++;
        PtOnMapForward = Pt->m_PointOnMap;
      }
      if (abs(Pt->m_MiddlePt - pInitialPt->m_MiddlePt) > m_iAveragingRange_um / 2.)
        break;
    } while (iNPoints < 50);
    if (iNPoints >= 50)
    {
      SENDERR("GetAverageAroundPt: Too many points to plus (%d)", iNPoints);
      return 0.;
    }
    Pt = pInitialPt;
    int iNMinusPts = 0;
    do
    {
      Pt--;
      if (Pt < m_ConturData.GetData())
        Pt = m_ConturData.GetData() + m_ConturData.Count() - 1;
      if (Pt->m_dAveBurrWidth_um > 0.)
      {
        dSum += Pt->m_dAveBurrWidth_um;
        iNPoints++;
        PtOnMapBackward = Pt->m_PointOnMap;
      }
      if (abs(Pt->m_MiddlePt - pInitialPt->m_MiddlePt) > m_iAveragingRange_um / 2.)
        break;
    } while (++iNMinusPts < 50);
    if (iNMinusPts >= 50)
    {
      SENDERR("GetAverageAroundPt: Too many points to minus (%d)", iNMinusPts);
      return 0.;
    }
    if (dSum > 0.)
      return dSum / iNPoints;
  }

  return 0.0;
}

bool ConturAsm::SendEdgeAngleToTVObject(double dAngle_rad)
{
  double dAngle_deg = RadToDeg(dAngle_rad);
  FXString AngleCommand;
  AngleCommand.Format("name=ext_segment;dir_degrees=%d;", ROUND(dAngle_deg));
  CTextFrame * pCommand = CreateTextFrame(AngleCommand,
    "SetObjectProp");
  return PutFrame(m_pOutput, pCommand);
}

double ConturAsm::SetPredictedEdgeAngle()
{
  cmplx * pExtData = ( cmplx* ) ( m_pLastExternalContur->GetData() );
  int iLastPtOnEdge = ( int ) m_pLastExternalContur->Count() - 1;
  cmplx cLast = pExtData[ iLastPtOnEdge ];
  cmplx cPtMinus7 = pExtData[ iLastPtOnEdge - 7 ];
  cmplx cEdgeVectorNearEnd = cLast - cPtMinus7;
  double dAngle = -arg( cEdgeVectorNearEnd );
  SendEdgeAngleToTVObject( dAngle );
  TRACE( "\n  Angle is %d" , ROUND( RadToDeg( dAngle ) ) );
  m_dLastEdgePredictedAngle_deg = dAngle; // dAngle now in degrees
  return dAngle ;
}


int ConturAsm::OrderNextPartForSimulation( int iResult )
{
  if ( iResult >= 0)
    m_SimulationLogger.AddFormattedMsg( "Result: %s" , iResult ? "Success" : "FAILED" ) ;
  m_uiSimuFileIndex = 0 ;
  if ( m_SimulationDirectories.GetCount() )
  {
    FXString NextWorkingDir ;
    if ( m_bShuffle )
    {
      rand_s( &m_uiSimuFileIndex ) ;
      m_uiSimuFileIndex %= m_SimulationDirectories.GetCount() ;
    }
    OrderNextDirForSimulation( m_SimulationDirectories[ ( int ) m_uiSimuFileIndex ] , true ) ;
    OrderNextDirForSimulation( m_SimulationDirectories[ ( int ) m_uiSimuFileIndex ] , false ) ;
    NextWorkingDir = m_SimulationDirectories[ ( int ) m_uiSimuFileIndex ] ;
    if ( !m_bShuffle )
      m_SimulationDirectories.RemoveAt( 0 ) ;

    Sleep( 1000 ) ;
    m_cRobotPos = m_cObservCenter ;
    m_CurrentPos.m_x = m_cObservCenter.real() ;
    m_CurrentPos.m_y = m_cObservCenter.imag() ;
    SetProcessingStage( Stage_2_MeasOnObserv , true );
    SetTask( 10 ) ;
    OrderBMPForSimulation( false );
    m_SimulationLogger.AddFormattedMsg( "Next part index %u(%u) ordered from dir %s" , 
      m_uiSimuFileIndex , m_SimulationDirectories.GetCount() , (LPCTSTR) NextWorkingDir ) ;
    return 1 ;
  }
  else
    SetProcessingStage( Stage_Inactive ) ;
  return 0;
}
