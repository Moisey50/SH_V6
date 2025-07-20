// ConturAsm.h : IO methods of the ConturAsm class


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

#define THIS_MODULENAME "ConturAsm"

extern LPCTSTR StageNames[];
extern LPCTSTR StageDescriptions[];

int GetNStageNames();
int GetNStageDescriptions();


size_t ConturAsm::SaveScanResults(LPCTSTR pFileName)
{
  size_t Cnt = 0;
  if (m_ConturData.GetCount())
  {
    //     FXString PathName( _T( "C:/BurrInspectorData" ) ) ;
    //     FxVerifyCreateDirectory( PathName ) ;
    FXString TS = m_MeasurementTS;
    FXString NewLogString;
    NewLogString.Format(_T("# Of '%s' Measured = %d\n"), (LPCTSTR)m_PartFile, 1);
    NewLogString.Format(_T("%s, Max Width = %.1fum Pt#=%d (Measured %d points) PO=%s\n"),
      (LPCTSTR)TS, m_dLastScanMaxWidth_um, m_iIndexWithMaxWidth, m_ConturData.Count(), (LPCTSTR)m_PO);

    FXString DataDir = CheckCreateDataDir();
    FXString GlobalLogName(DataDir + _T("GlobalLog.Log"));
    FILE * LogFile;
    errno_t err = _tfopen_s(&LogFile, GlobalLogName, _T("a+"));
    if (err == ERROR_SUCCESS)
    {
      fwrite((LPCTSTR)(m_PartFile + _T(' ')), sizeof(TCHAR), m_PartFile.GetLength() + 1, LogFile);
      fwrite((LPCTSTR)NewLogString, sizeof(TCHAR), NewLogString.GetLength(), LogFile);
      fclose(LogFile);
      LogFile = NULL;
    }

    FXString PartLogFileName(m_PartDirectory + m_PartFile + _T(".log"));
    FXString PartLogForDeleteName(m_PartDirectory + m_PartFile + _T("_for_delete.log"));
    err = _tfopen_s(&LogFile, PartLogFileName, _T("r"));
    if (err)
    {
      TCHAR ErrorString[200];
      _tcserror_s(ErrorString, err);
      SENDERR("Can't open part log file %s for reading (%s). Creating new.",
        (LPCTSTR)PartLogFileName, ErrorString);
      // First Log File Creation
      err = _tfopen_s(&LogFile, PartLogFileName, _T("w"));
      if (err)
      {
        _tcserror_s(ErrorString, err);
        SENDERR("Can't create new log file %s for writing (%s).",
          (LPCTSTR)PartLogFileName, ErrorString);
      }
      else
      {
        fwrite((LPCTSTR)NewLogString, sizeof(TCHAR), NewLogString.GetLength(), LogFile);
        fclose(LogFile);
      }
    }
    else
    {
      int iNMeasured = 0;
      FXSIZE iLogLen = FxGetFileLength(LogFile);
      FXString FileContent;
      if (iLogLen > 10)
      {
        LPTSTR pBuf = FileContent.GetBuffer(iLogLen + 1);
        FXSIZE iNRead = fread(pBuf, 1, iLogLen, LogFile);
        if (iNRead < iLogLen / 2/*(iLogLen - 5)*/)
        {
          SENDERR("Error on log file read %s (requested %d, read %d)",
            (LPCTSTR)PartLogFileName, iLogLen, iNRead);
        }
        FileContent.ReleaseBuffer(iNRead);
        fclose(LogFile);
      }
      err = _tfopen_s(&LogFile, PartLogForDeleteName, _T("w"));
      if (err)
      {
        TCHAR ErrorString[200];
        _tcserror_s(ErrorString, err);
        SENDERR("Can't open part log file %s for writing (%s)", (LPCTSTR)PartLogForDeleteName, ErrorString);
      }
      else
      {
        FXSIZE iPos = 0;
        do
        {
          FXString Token = FileContent.Tokenize(_T("\n"), iPos);
          if (!Token.IsEmpty())
          {
            FXSIZE iFoundPos;
            if ((iFoundPos = Token.Find(_T("# Of "))) >= 0)
            {
              FXSIZE iNumPos = Token.Find(_T('='), iFoundPos + 5);
              iNMeasured = atoi((LPCTSTR)Token + iNumPos + 1);
              Token.Format(_T("# Of '%s' Measured = %d"), (LPCTSTR)m_PartFile, ++iNMeasured);
            }
            Token += _T('\n');
            fwrite((LPCTSTR)Token, sizeof(TCHAR), Token.GetLength(), LogFile);
          }
        } while (iPos >= 0);
        fwrite((LPCTSTR)NewLogString, sizeof(TCHAR), NewLogString.GetLength(), LogFile);
        fclose(LogFile);
        FXString TmpName = PartLogFileName + _T(".del");
        if (!rename((LPCTSTR)PartLogFileName, (LPCTSTR)TmpName))
        {
          if (!rename(PartLogForDeleteName, PartLogFileName))
          {
            if (remove(TmpName))
              SENDERR("Can't remove %s to %s", (LPCTSTR)TmpName);
          }
          else
            SENDERR("Can't rename %s to %s", (LPCTSTR)PartLogForDeleteName,
            (LPCTSTR)PartLogFileName);
        }
        else
        {
          SENDERR("Can't rename %s to %s", (LPCTSTR)PartLogFileName, (LPCTSTR)TmpName);
        }
      }
    }

    FXString FileNameWithTS = (m_PartFile + _T("_")) + TS;
    FXString FileName = (pFileName) ? pFileName : m_ResultDirectory + FileNameWithTS;
    FileName += _T(".dat");
    FILE * fw = NULL;
    err = _tfopen_s(&fw, (LPCTSTR)FileName, _T("w"));
    if (err == NO_ERROR)
    {
      FXPropKit2 ForWrite;
      ForWrite.WriteString("TimeStamp", m_MeasurementTS);
      ForWrite.WriteString("ConturASM_TS", __DATE__ __TIME__ );
      ForWrite.WriteString("Config",
#ifdef _DEBUG
        "Debug");
#else
        "Release" );
#endif
      ForWrite.WriteDouble( "ScanTime_sec" , m_dLastScanTime_ms / 1000. ) ;
      ForWrite.Insert(0, _T("//  "));
      Cnt = fwrite((LPCTSTR)ForWrite, sizeof(TCHAR), ForWrite.GetLength(), fw);
      ForWrite.Empty();
      ForWrite.WriteString("PO", m_PO);
      ForWrite.WriteString(_T("FileName"), FileName);
      ForWrite.Insert(0, _T("\n//  "));
      Cnt += fwrite((LPCTSTR)ForWrite, sizeof(TCHAR), ForWrite.GetLength(), fw);
      FXString Descr("\n//");
      Descr += m_ConturData[0].GetCaption();
      Cnt += fwrite((LPCTSTR)Descr, sizeof(TCHAR), Descr.GetLength(), fw);
      for (int i = 0; i < (int)m_ConturData.GetCount(); i++)
      {
        FXString AsString;
        m_ConturData[i].ToString(AsString, _T("("), _T(")"));
        AsString += _T("\n");
        size_t WrLen = fwrite((LPCTSTR)AsString, sizeof(TCHAR), AsString.GetLength(), fw);
        if (WrLen != 0)
          Cnt += WrLen;
        else
          break;
      }
      fclose(fw);
    }
    else
    {
      SENDERR("Can't write result file %s(err %d)", pFileName, err);
    }

  }
  return Cnt;
}

bool ConturAsm::LoadResults(LPCTSTR pFileName, LPCTSTR pDataDir)
{
  m_dMeasurementStartTime = GetHRTickCount();
  // Clear views
  ResetViews();

  FR_RELEASE_DEL(m_VectorsToVertices);
  FR_RELEASE_DEL(m_pObservationImage);

  FXString Dir(pDataDir), FileName(pFileName);
  if (pDataDir && *pDataDir)
  {
    if (Dir[Dir.GetLength() - 1] != _T('\\')
      && Dir[Dir.GetLength() - 1] != _T('/'))
    {
      Dir += _T('\\');
    }
    FileName = Dir + FileName;
  }
  FXSIZE iBackSlashPos = FileName.ReverseFind(_T('\\'));
  FXSIZE iSlashPos = FileName.ReverseFind(_T('/'));
  if (iBackSlashPos > iSlashPos)
    iSlashPos = iBackSlashPos;

  FXString PureFileName = FileName.Mid(iSlashPos + 1);
  m_ResultDirectory = Dir = FileName.Left(iSlashPos + 1);
  m_ImagesDirectory = Dir + _T("Images\\");
  FXString PartDirectory(m_ResultDirectory);
  m_ResultDirectory.Delete(iSlashPos);
  iBackSlashPos = m_ResultDirectory.ReverseFind(_T('\\'));
  iSlashPos = m_ResultDirectory.ReverseFind(_T('/'));
  if (iBackSlashPos > iSlashPos)
    iSlashPos = iBackSlashPos;
  m_PartDirectory = m_ResultDirectory.Left(iSlashPos + 1);

  //yuras 20200506
  FXSIZE Underscore1 = PureFileName.Find(_T('_'));
  FXSIZE Underscore2 = PureFileName.Find(_T('_'), Underscore1 + 1);
  FXSIZE fileExt = PureFileName.ReverseFind(_T('.'));
  m_PureFileName = PureFileName.Left(fileExt);
  FXString TimeStamp = m_PureFileName.Mid(Underscore2 + 1);
  m_MeasurementTS = TimeStamp;
  m_InternalCatalogID = m_PureFileName.Left(Underscore1);
  m_PartCatalogName = m_PureFileName.Mid(Underscore1 + 1, Underscore2 - Underscore1 - 1);
  //yuras 20200506
  m_ResultDirectory += _T("\\");  // restore backslash

  FILE * fr = NULL;
  errno_t err = fopen_s(&fr, (LPCTSTR)FileName, _T("r"));
  if (err != ERROR_SUCCESS)
  {
    SENDERR("ConturAsm::LoadResults: ERROR %s on File %s opening for reading",
      _tcserror(err), (LPCTSTR)FileName);
    return false;
  }
  m_ConturData.RemoveAll();
  m_MeasurementExtremes.SetSize(4);

  FR_RELEASE_DEL(m_pPartConturOnObservation);

  double dXMin = DBL_MAX, dXMax = -DBL_MAX,
    dYMin = DBL_MAX, dYMax = -DBL_MAX;
  TCHAR Buf[1000];
  int iNReadStrings = 0;
  int iIndexOfLastSegment = 0;
  int iNReported = 0;

  //yuras 20200506
  m_iPercentOfMeasured = 0;
  //yuras 20200506
  m_dLastAverageWidth_um = 0;
  m_dLastScanMaxWidth_um = 0.;
  m_dLastMaxWidth_um = 0.;
  CCoordinate RobotPos;
  char * pNextBuf = NULL;
  do
  {
    ConturSample NewSample;
    pNextBuf = fgets(Buf, sizeof(Buf), fr);
    if (pNextBuf)
    {
      FXPropKit2 AsString(Buf);
      if (Buf[0] != _T('/'))
      {
        NewSample.Reset();
        if (NewSample.FromString(AsString) >= NewSample.GetNItems())
        {
          SetMinMax(NewSample.m_MiddlePt._Val[_RE], dXMin, dXMax);
          SetMinMax(NewSample.m_MiddlePt._Val[_IM], dYMin, dYMax);
          if (NewSample.m_MiddlePt.real() == dXMin)
            m_MeasurementExtremes[0] = NewSample.m_MiddlePt;
          else if (NewSample.m_MiddlePt.real() == dXMax)
            m_MeasurementExtremes[2] = NewSample.m_MiddlePt;
          if (NewSample.m_MiddlePt.imag() == dYMin)
            m_MeasurementExtremes[1] = NewSample.m_MiddlePt;
          else if (NewSample.m_MiddlePt.imag() == dYMax)
            m_MeasurementExtremes[3] = NewSample.m_MiddlePt;
          m_ConturData.Add(NewSample);
          if (NewSample.m_dAveBurrWidth_um > m_dLastMaxWidth_um)
            m_dLastMaxWidth_um = NewSample.m_dAveBurrWidth_um;
        }
        else
          SENDERR("ConturAsm::LoadResults - Error on string %d parsing %s ",
            iNReadStrings, Buf);
      }
      else
      {
        FXString SavedTS, SavedPO;
        if (AsString.GetString("TimeStamp", SavedTS))
          TimeStamp = SavedTS;
        if (AsString.GetString("PO", SavedPO))
          m_PO = SavedPO;
        double dScanTime ;
        if ( AsString.GetDouble( "ScanTime_sec" , dScanTime ) )
          m_dLastScanTime_ms = dScanTime * 1000. ;
        else
          m_dLastScanTime_ms = 0. ;
      }
    }

    if (NewSample.m_iSegmentNumInScan != iIndexOfLastSegment
      || !pNextBuf)
    {
      FXString FullReport, OnePtReport;
      int iNGood = 0;
      DWORD dwMarkColor = 0;
      m_dLastAverageWidth_um = 0.;
      int iLimit = (int)m_ConturData.Count() - ((pNextBuf) ? 1 : 0);
      RobotPos = m_ConturData[iNReported].m_RobotPos;
      int iSegmentNum = m_ConturData[iNReported].m_iSegmentNumInScan;
      for (int i = iNReported; i < iLimit; i++)
      {
        ConturSample& Sample = m_ConturData.GetAt(i);

        double dWidth = Sample.m_dAveBurrWidth_um;

        if (IsValidGradValue(dWidth))
        {
          m_dLastAverageWidth_um += dWidth;
          if (dWidth > m_dLastScanMaxWidth_um)
          {
            m_iIndexWithMaxWidth = i;
            m_dLastScanMaxWidth_um = dWidth;
          }
          ++iNGood;
        }

        switch (Sample.m_iBadEdge)
        {
        case EQ_Defects: dwMarkColor = 0x4040ff; break;
        case EQ_Invisible: dwMarkColor = 0xffffff; break;
        default:
          dwMarkColor = GetMarkColor(
            dWidth, m_dMaximumsForViewCsale[(int)m_ResultViewMode] , 0. );
          break;
        }

        OnePtReport.Format("Pt#=%d Width=%.1f Ready=%d%% "
          "Robot(%d,%d,%d) Color=0x%06X "
          "Mode=%s T=%s\n",
          i, dWidth, m_iPercentOfMeasured,
          ROUND(Sample.m_MiddlePt.real()), ROUND(Sample.m_MiddlePt.imag()),
          ROUND(Sample.m_RobotPos.m_z), dwMarkColor,
          fabs(m_dLastMaxWidth_um - dWidth) < 0.01 ? "Max" : "Val",
          (LPCTSTR)TimeStamp);
        FullReport += OnePtReport;
      }

      if (iNGood < m_iNMinGoodSamples)
        dwMarkColor = 0xffffff;
      if (iNGood)
        m_dLastAverageWidth_um /= iNGood;

      ConturSample& Sample = m_ConturData.GetAt(iNReported);
      OnePtReport.Format("Pt#=%d Width=%.1f Ready=%d%% "
        "Robot(%d,%d,%d) Color=0x%06X "
        "Mode=%s T=%s\n",
        iSegmentNum,
        m_dLastAverageWidth_um, m_iPercentOfMeasured,
        ROUND(RobotPos.m_x), ROUND(RobotPos.m_y),
        ROUND(RobotPos.m_z), dwMarkColor,
        "Ave", (LPCTSTR)TimeStamp);
      FullReport += OnePtReport;
      if (!FullReport.IsEmpty())
      {
        CTextFrame * pFullReport = CreateTextFrame(
          FullReport, "Result", 0);
        PutFrame(m_pOutput, pFullReport);
        if (m_iDelayBetweenSegmentsOnLoading > 0)
        {
          if (m_iDelayBetweenSegmentsOnLoading > 200)
            m_iDelayBetweenSegmentsOnLoading = 200;
          Sleep(m_iDelayBetweenSegmentsOnLoading);
        }
      }
      iIndexOfLastSegment = m_ConturData.Last().m_iSegmentNumInScan; // it will index of next segment
      iNReported = (int)m_ConturData.Count() - 1;
      m_dLastMaxWidth_um = 0.;
    }
    iNReadStrings++;
  } while (pNextBuf);
  m_iMaxGradPtIndex = -1;
  if (m_ConturData.Count())
  {
    SENDINFO("%d points loaded from file %s", m_ConturData.Count(),
      (LPCTSTR)PureFileName);
    SENDINFO("Data Directory %s", (LPCTSTR)m_ResultDirectory);

    CTextFrame * pEndOfScan = FormScanFinishMessage("Result", true);
    PutFrame(m_pOutput, pEndOfScan);

    ViewSavedObservationImage();
    m_ObservationExtremes.RemoveAll();

    m_iSelectedPtIndex = m_iIndexWithMaxWidth;
    m_iMaxGradPtIndex = m_iSelectedPtIndex;
    m_bFormMaxGradMarker = true;
    CDataFrame * pMapView = FormMapImage(NULL, m_ResultViewMode, m_iIndexWithMaxWidth, 100);
    if (pMapView)
      pMapView->Release(); // now there is only one way to fill coordinates on map
    CalcAveragesForRegions(m_AverData);
    m_MapMouseClickMode = MCM_ViewSaved ;
    pMapView = FormMapImage(NULL, m_ResultViewMode, m_iIndexWithMaxWidth, 100);
    if (pMapView)
    {
      PrepareDataForCombinedMap(pMapView);

      PutFrame(m_pOutput, pMapView);
    }
  }

  m_DataFrom = ADF_MeasuredInThePast;
  return true;
}

FXString ConturAsm::CheckCreateDataSubDir(LPCTSTR pszSubdir)
{
  FXRegistry Reg("TheFileX\\ConturASM");
  FXString Directory = Reg.GetRegiString(
    "Data", "MainDirectory", "c:\\BurrInspector\\");
  Directory += Reg.GetRegiString("Data", "DataSubDir", "Data\\");
  Directory += pszSubdir;
  if (!FxVerifyCreateDirectory(Directory))
  {
    SENDERR("Can't create data directory '%s' ", (LPCTSTR)Directory);
    return FXString();
  };
  Directory += _T('\\');
  return Directory;
}


FXString ConturAsm::CheckCreateDataDir()
{
  FXRegistry Reg("TheFileX\\ConturASM");
  FXString Directory = Reg.GetRegiString(
    "Data", "MainDirectory", "c:\\BurrInspector\\");
  FXString DataSubDir = Reg.GetRegiString("Data", "DataSubDir", "Data\\");
  FXString DataDir(Directory + DataSubDir);
  if (!FxVerifyCreateDirectory(DataDir))
  {
    SENDERR("Can't create data directory '%s' ", (LPCTSTR)DataDir);
    return FXString();
  };
  return DataDir;
}

FXString ConturAsm::CheckCreatePartResultDir()
{
  FXString DirName = CheckCreateDataDir();
  if (DirName.IsEmpty())
    return DirName;

  FXString PartDir = CheckCreateDataSubDir(m_PartFile);
  if (PartDir.IsEmpty())
    return PartDir;

  m_PureFileName = m_PartFile + _T('_') + m_MeasurementTS;
  m_PartDirectory = PartDir;
  FXString ResultDir = PartDir + m_PureFileName + _T('\\');
  if (!FxVerifyCreateDirectory(ResultDir))
  {
    SENDERR("Can't create result directory '%s' ", (LPCTSTR)ResultDir);
    return FXString();
  };
  m_ResultDirectory = ResultDir;
  FXString ImagesDir = ResultDir + _T("Images\\");
  if (!FxVerifyCreateDirectory(ImagesDir))
  {
    SENDERR("Can't create images directory '%s' ", (LPCTSTR)ImagesDir);
    return FXString();
  };
  m_ImagesDirectory = ImagesDir;
  return ResultDir;
}

void ConturAsm::SaveFormsFileName(LPCTSTR pFileName)
{
  FXRegistry Reg("TheFileX\\ConturASM");
  FXString DataDir = CheckCreateDataDir();
  FxVerifyCreateDirectory(DataDir);
  FXString PureFileName = FxGetFileName(pFileName);
  Reg.WriteRegiString("Data", "FormsDescrFileName", PureFileName);
}

PartInitStatus ConturAsm::DecodePartDescriptors()
{
  FXString InternalCatalogID;
  FXString PO;
  FXPropertyKit ForPars(!m_PartFile.IsEmpty() ? 
    m_PartFile : m_NewPartName );
  ForPars.Trim();
  if (!m_PartFile.IsEmpty())
  {
    ForPars.Replace(',', ';');
    ForPars.GetString("PO", PO);
    m_PO = PO;

    ForPars.GetString("ID", InternalCatalogID);
    if (InternalCatalogID.IsEmpty())
    {
      int iUnderscorePos = (int) ForPars.Find(_T('_'));
      if (iUnderscorePos > 0)
      {
        m_InternalCatalogID = ForPars.Left(iUnderscorePos);
        m_PartCatalogName = ForPars.Mid(iUnderscorePos + 1);
        m_FullPartName = ForPars;
        int m_iKnownPartsIndex = m_ObservGeomAnalyzer.FindFormByID(m_InternalCatalogID);
        if (m_iKnownPartsIndex >= 0)
        {
          m_FullPartName = m_ObservGeomAnalyzer.GetAt(m_iKnownPartsIndex).m_Name;
          return Part_Known;
        }
        return Part_New; 
      }
    }
    else
    {
      int m_iKnownPartsIndex = m_ObservGeomAnalyzer.FindFormByID(InternalCatalogID);
      if (m_iKnownPartsIndex >= 0 )
      {
        m_InternalCatalogID = InternalCatalogID;
        m_FullPartName = m_ObservGeomAnalyzer.GetAt(m_iKnownPartsIndex).m_Name;
        return Part_Known;
      }
    }
  }
  else if (!ForPars.IsEmpty()) // From m_NewPartName
  {
    int iUnderscorePos = (int) ForPars.Find(_T('_'));
    if (iUnderscorePos > 0)
    {
      m_InternalCatalogID = ForPars.Left(iUnderscorePos);
      m_PartCatalogName = ForPars.Mid(iUnderscorePos + 1);
      m_FullPartName = ForPars;
      return Part_New;
    }
  }
  m_InternalCatalogID.Empty();
  m_iKnownPartsIndex = -1;
  return Part_Unknown;
}

bool ConturAsm::LoadPartParametersFromRegistry( bool bDefault ) // registry item is in m_FullPartName
{
  FXRegistry Reg("TheFileX\\ConturASM\\Parts");

  LPCTSTR pPartName = bDefault ? "Default" : (LPCTSTR)m_FullPartName;

  m_iObservationMeasExp_us = Reg.GetRegiInt(pPartName, "ObservExp_us", m_iObservationMeasExp_us);
  m_iMeasurementExposure_us = Reg.GetRegiInt(pPartName, "MeasExp_us", m_iMeasurementExposure_us);
  m_iSegmentLength = Reg.GetRegiInt(pPartName, "SegmentLength_pix", m_iSegmentLength);
  m_dDarkEdgeThreshold = Reg.GetRegiDouble(pPartName, "DarkEdgeThres", m_dDarkEdgeThreshold);
  int ResViewMode = Reg.GetRegiInt(pPartName, "ResultViewMode", 1); // default burr width
  if (ResViewMode < m_dMaximumsForViewCsale.GetSize())
    m_ResultViewMode = (ResultViewMode)ResViewMode;
  m_dMaximumsForViewCsale[(int)m_ResultViewMode] = 
    Reg.GetRegiDouble(pPartName, "ViewRedThreshold_um", 25. ) ;
  m_dPartHeight = Reg.GetRegiDouble( pPartName , "InitialPartHeight_um" , m_dPartHeight ) ;
  if ( m_dPartHeight > 40000 )
  {
    m_dPartHeight = 6000. ;
    Reg.WriteRegiDouble( pPartName , "InitialPartHeight_um" , m_dPartHeight ) ;
  }

  m_LastObservationForm.m_dHeight_um = m_dPartHeight ;

  FXRegistry Reg2("TheFileX\\ConturASM");
  m_iAccel_units = Reg2.GetRegiInt("Parameters", "Acceleration_units", 8 );
  m_iXYMotionScanSpeed_um_sec = Reg2.GetRegiInt( "Parameters" , "XYMotionSpeed_um_per_sec" , 20000 );
  m_iZMotionSpeed_um_sec = Reg2.GetRegiInt( "Parameters" , "ZMotionSpeed_um_sec" , 20000 );
  return true;
}

bool ConturAsm::SavePartParametersToRegistry(bool bSuccess) // registry item is in m_FullPartName
{
  FXRegistry Reg("TheFileX\\ConturASM\\Parts");

  Reg.WriteRegiInt(m_FullPartName, "ObservExp_us", m_iObservationMeasExp_us);
  Reg.WriteRegiInt(m_FullPartName, "MeasExp_us", m_iMeasurementExposure_us);
  Reg.WriteRegiInt(m_FullPartName, "SegmentLength_pix", m_iSegmentLength);
  Reg.WriteRegiDouble(m_FullPartName, "DarkEdgeThres", m_dDarkEdgeThreshold);
  Reg.WriteRegiInt(m_FullPartName, "ResultViewMode", m_ResultViewMode);
  Reg.WriteRegiDouble(m_FullPartName, "ViewRedThreshold_um", m_dMaximumsForViewCsale[m_ResultViewMode]);
  double dHeight = bSuccess ? m_dBaseHeight - m_dAdapterHeight - m_CurrentPos.m_z : m_dPartHeight;
  ASSERT( dHeight < 40000 ) ;
  Reg.WriteRegiDouble( m_FullPartName , "InitialPartHeight_um" , dHeight ) ;
  double dPerimeter_um = m_pPartConturOnObservation->GetConturLength() / m_dObservScale_pix_per_um ;
  Reg.WriteRegiDouble( m_FullPartName , "PartPerimeterOnObservation_um" , dPerimeter_um ) ;
  double dArea_um2 = GetFigureArea( m_pPartConturOnObservation ) /
    (m_dObservScale_pix_per_um * m_dObservScale_pix_per_um) ;
  Reg.WriteRegiDouble( m_FullPartName , "PartArea_um2" , dArea_um2 ) ;
  
  int iNMeasured = Reg.GetRegiInt( m_FullPartName , "NMeasured" , 0 ) ;
  double dAverPerimeter_um = Reg.GetRegiDouble( m_FullPartName , "AverPerimeter_um" , 0. );
  dAverPerimeter_um = (iNMeasured) ?
    ((dAverPerimeter_um * iNMeasured / (iNMeasured + 1.)) + dPerimeter_um / (iNMeasured + 1.))
    : dPerimeter_um ;
  Reg.WriteRegiDouble( m_FullPartName , "AverPerimeter_um" , dAverPerimeter_um , "%.3f" ) ;
  double dAverArea_um2 = Reg.GetRegiDouble( m_FullPartName , "AverArea_um2" , 0. );
  dAverArea_um2 = (iNMeasured) ?
    ((dAverArea_um2 * iNMeasured / (iNMeasured + 1.)) + dArea_um2 / (iNMeasured + 1.))
    : dArea_um2 ;
  Reg.WriteRegiDouble( m_FullPartName , "AverArea_um2" , dAverArea_um2 , "%.3f" );
  Reg.WriteRegiInt( m_FullPartName , "NMeasured" , iNMeasured + 1 ) ;


  FXString Now = GetTimeAsString_ms() ;
  FXString CreationTS = Reg.GetRegiString(m_FullPartName, "RecordCreationTS", Now );
  Reg.WriteRegiString( m_FullPartName , "LastMeasurementTS" , Now );

  return true;
}

bool ConturAsm::SaveLastScanParametersToRegistry(bool bSuccess) // registry item is in m_FullPartName
{
  FXRegistry Reg("TheFileX\\ConturASM");
  Reg.WriteRegiString("LastScan", "LastPart", m_PartFile);
  Reg.WriteRegiString("LastScan", "LastOrder", m_PO);
  Reg.WriteRegiInt("LastScan", "ObservExp_us", m_iObservationMeasExp_us);
  Reg.WriteRegiInt("LastScan", "MeasExp_us", m_iMeasurementExposure_us);
  Reg.WriteRegiInt("LastScan", "SegmentLength_pix", m_iSegmentLength);
  Reg.WriteRegiInt("LastScan", "ResultViewMode", m_ResultViewMode);
  Reg.WriteRegiDouble("LastScan", "ViewRedThreshold_um", m_dMaximumsForViewCsale[m_ResultViewMode]);

  return SavePartParametersToRegistry(bSuccess) ;
}

bool ConturAsm::ResetViews()
{
  CTextFrame * pResetViews = CreateTextFrame(_T("set reset(1)"), _T("ResetViews"));
  return PutFrame(m_pOutput, pResetViews);
}

bool ConturAsm::SaveImageFragment( const CVideoFrame * pVf ,
  CRect& Fragment , FXString& FilePath )
{
  Fragment.left &= ~(3) ;
  Fragment.top &= ~(3) ;
  Fragment.right &= ~(3) ;
  Fragment.bottom &= ~(3) ;
  pTVFrame pFragment = _cut_rect( pVf , Fragment , FALSE );
  if ( !pFragment )
    return false ;

  LPBYTE pData = GetData( pFragment ) ;
  EmbeddedFrameInfo * pInfo = (EmbeddedFrameInfo*) pData ;
  pInfo->Reset() ;
  pInfo->m_ROIPos = Fragment ;
  EmbeddedFrameInfo * pInfo2 = (EmbeddedFrameInfo*) (
    pData + Fragment.Width() * (Fragment.Height() - 1));
  memcpy( pInfo2 , pInfo , sizeof( EmbeddedFrameInfo ) ) ;

  bool bRes = saveSH2BMP( FilePath , pFragment->lpBMIH ) ;
  freeTVFrame( pFragment ) ;

  return bRes ;
  //CVideoFrame * pVideoFragment = CVideoFrame::Create( pFragment ) ;
  //pVideoFragment->SetLabel( FilePath ) ;
  //return PutFrame( m_pImageSave , pVideoFragment ) ;
}

CVideoFrame * ConturAsm::GetImageFromFile(
  FXString& FilePath , CRect& Fragment )
{
  pTVFrame tvF = (pTVFrame) malloc( sizeof( TVFrame ) ) ;
  tvF->lpBMIH = loadDIB( FilePath , false ) ; // don't convert to YUV
  if ( tvF->lpBMIH )
  {
    tvF->lpData = NULL ;
    LPBYTE pData = GetData( tvF ) ;
    if ( tvF->lpBMIH->biClrUsed )
    {
      int iImageSize = tvF->lpBMIH->biHeight * tvF->lpBMIH->biWidth ;
      memcpy( pData , pData + sizeof( RGBQUAD ) * tvF->lpBMIH->biClrUsed ,
        iImageSize ) ;
    }
    EmbeddedFrameInfo * pInfo = (EmbeddedFrameInfo*) pData ;
    if ( pInfo->IsPattern() )
      Fragment = pInfo->m_ROIPos ;
    else
      Fragment.SetRectEmpty() ;

    CVideoFrame * pVF = CVideoFrame::Create( tvF ) ;
    pVF->SetLabel( FilePath ) ;
    return pVF ;
  }
  free( tvF ) ;
  return NULL ;
}

bool ConturAsm::InitDirectoriesForSimulation()
{
  FXString PartNameFragment = m_PartFile ;

  FXRegistry Reg( "TheFileX\\ConturASM" );
  FXString MainDirectory = Reg.GetRegiString(
    "Data" , "MainDirectory" , "c:\\BurrInspector\\" );
  MainDirectory += Reg.GetRegiString( "Data" , "DataSubDir" , "Data\\" );

  FXFileFind finder;

// build a string with wildcards
  FXString SearchFileWildCard( MainDirectory );
  SearchFileWildCard += _T( '*' );
  int iSemiPos = (int)m_PartFile.Find( ';' ) ;
  FXString DirWildCard , FileWildCard ;
  if ( iSemiPos > 0 )
  {
    DirWildCard = m_PartFile.Left( iSemiPos ) ;
    FileWildCard = m_PartFile.Mid( iSemiPos + 1 ) ;
    iSemiPos = (int) FileWildCard.Find( ';' ) ;
    if ( iSemiPos > 0 )
      FileWildCard = FileWildCard.Left( iSemiPos ).Trim() ;
  }
  else
    DirWildCard = m_PartFile ;
  
  SearchFileWildCard += DirWildCard + "*.*" ;

  // start working for files
  BOOL bWorking = finder.FindFile( SearchFileWildCard );
  bool bFound = false ;
  while ( bWorking )
  {
    bWorking = finder.FindNextFile();

    // skip . and .. files; otherwise, we'd
    // recur infinitely!

    if ( finder.IsDots() )
      continue;

   // if it's a directory, recursively search it

    if ( finder.IsDirectory() )
    {  // This is main directory for selected part
      FXString MainPartDirectory = finder.GetFilePath();

      FXStringArray DirsWithData ;
      FXUIntArray NumberOfFilesInDirsWithData ;

      int iNDirs = (int)FxGetListOfDirs( MainPartDirectory , DirsWithData ,
        NumberOfFilesInDirsWithData , 
        FileWildCard.IsEmpty() ? NULL : (LPCTSTR)FileWildCard ) ;

      if ( iNDirs )
      {
        for ( int i = 0 ; i < iNDirs ; i++ )
        {
          if (  NumberOfFilesInDirsWithData[i] >= 5 )
          {
            if ( !bFound )
            {
              m_SimulationDirectories.RemoveAll() ;
              m_NFilesInSimulationDirs.RemoveAll() ;
              bFound = true ;
            }
            m_SimulationDirectories.Add( DirsWithData[ i ] ) ;
            m_NFilesInSimulationDirs.Add( NumberOfFilesInDirsWithData[ i ] ) ;
            for ( int j = i + 1 ; j < iNDirs ; j++ )
            {
              if ( NumberOfFilesInDirsWithData[ j ] >= 5 )
              {
                m_SimulationDirectories.Add( DirsWithData[ j ] ) ;
                m_NFilesInSimulationDirs.Add( NumberOfFilesInDirsWithData[ j ] ) ;
              }
            }
//             return true ;
          }
        }
      }
    }
  }

  return bFound ;
}
