// FindExtrem gadget implementation
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




//************************************************************************************

IMPLEMENT_RUNTIME_GADGET_EX(FindExtrem, CFilterGadget, _T("Helpers"), TVDB400_PLUGIN_NAME);

FindExtrem::FindExtrem() :
  m_Count(0),
  m_dBegin(0.0),
  m_dStep(1.0),
  m_dEnd(10.0),
  m_iIntMask(0),
  m_bIntFormat(0),
  m_bLoop(FALSE),
  m_bWasReset(false),
  m_bWasInRange(false),
  m_State(PS_Inactive),
  m_dScanBeginTime(0.),
  m_dLastValueTime(0.),
  m_IDPrefix(_T("ID=")),
  m_CoordPrefix(_T("um)=")),
  m_dScanSpeed_umPerSec(500.),
  m_dCorrection_um(0.)
{
  m_OutputMode = modeReplace;
  m_pInput = new CInputConnector(transparent);
  m_pInput->SetName(_T("NextValue"));
  m_pFromMotionInput = new CInputConnector(transparent, _fn_MotionStatus, this);
  m_pFromMotionInput->SetName(_T("FromMotion"));
  m_pOutput = new COutputConnector(transparent);
  m_pOutput->SetName(_T("ProcessResult"));
  m_pToMotion = new COutputConnector(text);
  m_pToMotion->SetName(_T("ToMotion"));
  m_pToLightControl = new COutputConnector(text);
  m_pToLightControl->SetName(_T("ToLightControl"));
  Resume();
}

void FindExtrem::ShutDown()
{
  CFilterGadget::ShutDown();
  delete m_pInput;
  m_pInput = NULL;
  delete m_pFromMotionInput;
  m_pFromMotionInput = NULL;
  delete m_pOutput;
  m_pOutput = NULL;
  delete m_pToMotion;
  m_pToMotion = NULL;
  delete m_pToLightControl;
  m_pToLightControl = NULL;
}

bool FindExtrem::PrintProperties(FXString& text)
{
  FXPropertyKit pk;
  pk.WriteDouble(_T("RangeBegin"), m_dBegin);
  //   pk.WriteDouble( _T( "Step" ) , m_dStep ) ;
  pk.WriteDouble(_T("RangeEnd"), m_dEnd);
  pk.WriteString(_T("LightCommand"), m_LightCommand);
  pk.WriteInt(_T("ControlAxis"), m_iControlAxis);
  pk.WriteDouble(_T("ScanSpeed_um"), m_dScanSpeed_umPerSec);
  pk.WriteDouble(_T("Correction_um"), m_dCorrection_um);
  pk.WriteString(_T("CoordPrefix"), m_CoordPrefix);
  pk.WriteString(_T("IdPrefix"), m_IDPrefix);
  text += pk;
  return true;
}

bool FindExtrem::ScanProperties(LPCTSTR text, bool& Invalidate)
{
  FXPropertyKit pk(text);
  bool bInt = true;
  FXString AsString;
  if (pk.GetString(_T("RangeBegin"), AsString))
  {
    m_dBegin = atof((LPCTSTR)AsString);
    m_Count = 0;
  }
  //   if ( pk.GetString( _T( "Step" ) , AsString ) )
  //   {
  //     m_dStep = atof( ( LPCTSTR ) AsString ) ;
  //     m_Count = 0 ;
  //   }
  if (pk.GetString(_T("RangeEnd"), AsString))
  {
    m_dEnd = atof((LPCTSTR)AsString);
    m_Count = 0;
  }
  pk.GetString(_T("LightCommand"), m_LightCommand);
  pk.GetInt(_T("ControlAxis"), m_iControlAxis);
  pk.GetString(_T("CoordPrefix"), m_CoordPrefix);
  pk.GetString(_T("IdPrefix"), m_IDPrefix);
  pk.GetDouble(_T("ScanSpeed_um"), m_dScanSpeed_umPerSec);
  pk.GetDouble(_T("Correction_um"), m_dCorrection_um);
  m_dLastValue = m_dBegin;
  m_bWasReset = true;
  return true;
}

bool FindExtrem::ScanSettings(FXString& text)
{
  text = "template("
    "EditBox(RangeBegin)"
    //     ",EditBox(Step)"
    ",EditBox(RangeEnd)"
    ",EditBox(LightCommand)"
    ",EditBox(CoordPrefix)"
    ",EditBox(IdPrefix)"
    ",Spin(ControlAxis,1,3)"
    ",EditBox(ScanSpeed_um)"
    ",EditBox(Correction_um)"
    ")";
  return true;
}
static double dMaxDelayFromStartTime_ms = 100000.;
static double dMaxDelayAfterLastValue_ms = 100000.;

CDataFrame* FindExtrem::DoProcessing(const CDataFrame* pDataFrame)
{
  if (m_GadgetInfo.IsEmpty())
  {
    GetGadgetName(m_GadgetInfo);
  }
  double dCurrent = 0.;
  switch (m_iControlAxis)
  {
  case 1: dCurrent = m_CurrentPos.m_x; break;
  case 2: dCurrent = m_CurrentPos.m_y; break;
  case 3: dCurrent = m_CurrentPos.m_z; break;
  default:
    SEND_GADGET_ERR("Unknown control axis %d for focusing", m_iControlAxis);
    return NULL;
  }
  double dEntryTime = GetHRTickCount();
  if (dEntryTime - m_dLastStartTime > dMaxDelayFromStartTime_ms)
  {
    if (dEntryTime - m_dLastValueTime > dMaxDelayAfterLastValue_ms)
    {
      m_State = PS_Inactive;
    }
  }

  FXString Label = pDataFrame->GetLabel();
  const CTextFrame * pContent = pDataFrame->GetTextFrame();
  if (Label == _T("Stop")
    || (pContent && (pContent->GetString() == _T("Stop"))))
  {
    m_State = PS_Inactive;
    return NULL;
  }

  if ((_tcscmp(pDataFrame->GetLabel(), _T("SetMotionSpeed")) == 0)
    && ((m_State == PS_Inactive) || (m_State == PS_Finished)))
  {
    CDataFrame * pCopy = pDataFrame->Copy();
    PutFrame(m_pToMotion, pCopy);
    return NULL;
  }

  if ((_tcscmp(pDataFrame->GetLabel(), _T("SendToMotion")) == 0)
    && ((m_State == PS_Inactive) || (m_State == PS_Finished)))
  {
    CDataFrame * pCopy = pDataFrame->Copy();
    PutFrame(m_pToMotion, pCopy);
    return NULL;
  }

  if ((_tcscmp(pDataFrame->GetLabel(), _T("FindFocus")) == 0)
    && ((m_State == PS_Inactive) || (m_State == PS_Finished))
    && (dCurrent > 1.))
  {
    const CTextFrame * tf = pDataFrame->GetTextFrame();
    if (tf)
    {
      if (!tf->GetString().IsEmpty())
      {
        bool bDummy = false;
        ScanProperties(tf->GetString(), bDummy);
      }

    }
    //FXString 
    m_Samples.RemoveAll();
    FXString Cmd = FormSpeedInUnitsFrom_um_persecond(m_dScanSpeed_umPerSec * 5.);
    CTextFrame * pCommand = CTextFrame::Create(Cmd);
    pCommand->CopyAttributes(pDataFrame);
    if (PutFrame(m_pToMotion, pCommand))
    {
      // switch off periodical light and capture
      pCommand = CreateTextFrame(_T("astbon 0\r\n"), (LPCTSTR)NULL);
      PutFrame(m_pToLightControl, pCommand);

      Sleep(30);
      double dBeginPos = dCurrent + m_dBegin;
      m_dTargetForScan = dCurrent + m_dEnd;
      ASSERT(dCurrent > 1.);

      Cmd.Format(_T("Move_Absolute=%.2f\r\n"), dBeginPos - m_dCorrection_um);
      pCommand = CTextFrame::Create(Cmd);
      pCommand->CopyAttributes(pDataFrame);
      if (PutFrame(m_pToMotion, pCommand))
        m_State = PS_SendToBegin;
      else
      {
        SEND_GADGET_ERR(
          "Can't send initial coordinate for focusing", m_iControlAxis);
      }
      m_dLastStartTime = dEntryTime;
    }
    else
    {
      SEND_GADGET_ERR(
        "Can't send initial speed for focusing", m_iControlAxis);
    }
    return NULL;
  }
  const CQuantityFrame * pQuan = pDataFrame->GetQuantityFrame();
  if (pQuan)
  {
    double dVal = (double)(*pQuan);
    if (m_State == PS_InScan)
    {
      m_dLastValueTime = dEntryTime;
      double dDistToTarget = m_dTargetForScan - dCurrent;
      double dMax = 0.;
      int iMaxIndex = -1;
      if (dDistToTarget < 3.0)
      { // scan is finished, do result calculations

        m_State = PS_ScanFinished;
        for (int i = 0; i < m_Samples.GetCount(); i++)
        {
          if (dMax < m_Samples[i].m_dValue)
          {
            dMax = m_Samples[i].m_dValue;
            iMaxIndex = i;
          }
        }
        if (iMaxIndex >= 0)
        {
          FXString Cmd = FormSpeedInUnitsFrom_um_persecond(m_dScanSpeed_umPerSec * 5.);
          CTextFrame * pCommand = CTextFrame::Create(Cmd);
          pCommand->CopyAttributes(pDataFrame);
          PutFrame(m_pToMotion, pCommand);
          Sleep(20);
          m_dFoundFocusCoord = m_Samples[iMaxIndex].m_dCoord + m_dCorrection_um;
          m_dOptimalValue = m_Samples[iMaxIndex].m_dValue;
          CTextFrame * pToFocalDist = CreateTextFrame(NULL, (DWORD)0);
          pToFocalDist->GetString().Format(_T("Move_Absolute=%.2f\r\n"),
            m_dFoundFocusCoord);
          PutFrame(m_pToMotion, pToFocalDist);

          return NULL;
        }
        SEND_GADGET_ERR("No data for focus calculation");
        return NULL;
      }
      else
      {
#ifdef _DEBUG
        m_DebugSamples[m_Samples.GetCount()] = dVal;
#endif
        ExtremSample NewSample(dVal, dCurrent, GetHRTickCount(), m_dLastCoordTime);
        m_Samples.Add(NewSample);
        CTextFrame * pRequestCoord = CreateTextFrame(_T("Return_Current_Position=1\r\n"), (LPCTSTR)NULL);
        if (!PutFrame(m_pToMotion, pRequestCoord))
        {
          SEND_GADGET_WARN(_T("Can't send Request for position"));
          m_State = PS_Inactive;
        }
        else
        {
          CTextFrame * pLightCommand = CreateTextFrame(m_LightCommand, (LPCTSTR)NULL);
          if (!PutFrame(m_pToLightControl, pLightCommand))
          {
            SEND_GADGET_WARN(
              "Can't send light command for focusing");
            m_State = PS_Inactive;
          }
        }
      }
    }
    else // pass DiffSum
    {
      CQuantityFrame * pFocusData = CreateQuantityFrame(
        dVal, _T("DiffSum"), pDataFrame->GetId());
      //PutFrame( m_pToLightControl , pFocusData ) ;
      PutFrame(m_pOutput, pFocusData);
    }
  }
  return NULL;
}

int FindExtrem::DecodeMotionStatus(FXString& Msg)
{
  int iID = DecodeInputString(Msg);
  if (iID == m_iControlAxis)
  {
    if (m_bTimeout)
      m_State = PS_Inactive;
    switch (m_State)
    {
    case PS_SendToBegin:
      {
        FXString Cmd = FormSpeedInUnitsFrom_um_persecond(m_dScanSpeed_umPerSec);
        CTextFrame * pCommand = CTextFrame::Create(Cmd);
        pCommand->ChangeId(0);
        if (PutFrame(m_pToMotion, pCommand))
        {
          Sleep(30);

          Cmd.Format(_T("Move_Absolute=%.2f\r\n"), m_dTargetForScan);
          pCommand = CTextFrame::Create(Cmd);
          pCommand->ChangeId(0);
          if (!PutFrame(m_pToMotion, pCommand))
          {
            SEND_GADGET_ERR(
              "Can't send initial coordinate for focusing", m_iControlAxis);
            m_State = PS_Inactive;
            return NULL;
          }
        }
        else
        {
          SEND_GADGET_ERR(
            "Can't send initial speed for focusing", m_iControlAxis);
          return NULL;
        }
        pCommand = CreateTextFrame(m_LightCommand, (LPCTSTR)NULL);
        if (!PutFrame(m_pToLightControl, pCommand))
        {
          SEND_GADGET_ERR(
            "Can't send initial coordinate for focusing", m_iControlAxis);
          m_State = PS_Inactive;
          return NULL;
        }
        m_State = PS_InScan;
      }
      return 0;
    case PS_ScanFinished:
      {
        CTextFrame * pLightCommand = CreateTextFrame(m_LightCommand, (LPCTSTR)NULL);
        PutFrame(m_pToLightControl, pLightCommand);
        m_State = PS_Finished;
        return 1;
      }
    }
  }
  return 0;
}
