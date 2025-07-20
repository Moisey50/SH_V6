#include "stdafx.h"
#include "MPPT.h"
#include <files/imgfiles.h>
#include <fxfc/FXRegistry.h>
#include <imageproc/statistics.h>


#define THIS_MODULENAME "MPPT"

extern CRect g_ROI;


void MPPT::LoadAndUpdatePartParameters()
{
  RestoreKnownParts();
//   RestorePartDataFromRegistry(
//     m_SelectedPartName, m_CurrentPart );
  // If part doesn't exists, it will be created with default values
  m_UsedBlankEdge = m_CurrentPart.m_Blank.m_UsedBlankEdge;
  FXRegistry Reg( "TheFileX\\Micropoint" );

  m_dXY_PlotPeriod = Reg.GetRegiDouble( "Parameters" , "XY_PlotPeriod" , 1000. );
  m_iFocusingRadius = Reg.GetRegiInt( "Parameters" , "FocusingRadius_pix" , 50 );
  m_iFocusingY = Reg.GetRegiInt( "Parameters" , "FocusingY_pix" , 760 );
  m_dGoodStdThres_pix = Reg.GetRegiDouble( "Parameters" , "dGoodStdThres_pix" , 0.5 ) ;
  FXString Name( GetWorkingModeName() );
  Name += _T( "XYCalibStep_um" );
  m_dCalibStep = Reg.GetRegiDouble( "Data" , Name , 100. );
  m_iPinDefocusExposure = Reg.GetRegiInt("Parameters", "PinDefocusingExposure_us", 420);
//   m_dNormBirghtnessForCavity = Reg.GetRegiDouble(
//     "Parameters" , "NormBirghtnessForCavity" , 0.96 );
  m_iDistToFocusingArea = Reg.GetRegiInt( "Parameters" , "DistForPinFocusing" , 750 );
  m_bDoPinXYCalibration = Reg.GetRegiInt( "Parameters" , "DoPinXYCalibration" , 0 );
  m_iNAllowedErrors = Reg.GetRegiInt( "Data" , "NAllowedErrors" , 10 );
  m_iNCavityZMeasurements = Reg.GetRegiInt( "Data" , "NCavityZMeasurements" , 10 );
  m_dWidth_Tolerance = Reg.GetRegiDouble(
    "Parameters" , "dWidthTolerance_perc" , 7. );
  m_dHeight_Tolerance = Reg.GetRegiDouble(
    "Parameters" , "dHeightTolerance_perc" , 7. );
  m_dArea_Tolerance = Reg.GetRegiDouble(
    "Parameters" , "dAreaTolerance_perc" , 13. );
  Reg.GetRegiCmplx( "Parameters" , "NormZMeasArea" ,
    m_cNormZMeasArea , cmplx( 0.5 , 0.7 ) );
  m_dWhiteThreshold = Reg.GetRegiDouble(
    "Parameters" , "dWhiteThreshold" , 0.95 );
  m_iNAttempts = 0;
  m_iNMaxAttempts = Reg.GetRegiInt( "Data" , "NExpAdjustAttempts" , 10 );
  m_dTargetZ_pix = Reg.GetRegiDouble( "Data" , "TargetZ_pix" , 717. );
  m_WhatSideToUseForCavity = (DL_WhatSideToUseForZ)Reg.GetRegiInt("Parameters",
    "WhatCavZUse(1-L,2-R,3-Both)", (int)DLWSZ_Right);
  m_WhatSideToUseForPin = (DL_WhatSideToUseForZ)Reg.GetRegiInt("Parameters",
    "WhatPinZUse(1-L,2-R,3-Both)", (int)DLWSZ_Right);
  m_iFocusLogPeriod_samples = Reg.GetRegiInt("Parameters", "FocusLogPeriod_samples", 20);
  m_bXYandFocusLog = Reg.GetRegiInt("Parameters", "XYandFocusLog", 20);
  m_iSaveCavityPreliminaryData = Reg.GetRegiInt("Parameters", "SaveCavityPreliminaryData", 3);
  m_iSaveBlankPreliminaryData = Reg.GetRegiInt("Parameters", "SaveBlankPreliminaryData", 3);
  m_iSamplesCnt = 0;
  m_bDisableLightSwitchOff = Reg.GetRegiInt("Parameters", "DisableLightSwitchOff", 1);

  m_CurrentPart.RestorePartDataFromRegistry( NULL , m_SelectedPartName , true ) ;
  m_dAllowedAngleErrorBetweenVertLines = m_CurrentPart.m_Cavity.m_dMaxAngleDiffBetweenInternalEdges_deg;
  m_CavityEdge = m_CurrentPart.m_Cavity.m_CavityEdge ;
  m_dDLCavityHorizBaseLine = m_CurrentPart.m_Cavity.m_dPlaneHeight_um / 2. ;
  if ( m_CurrentPart.m_Cavity.m_CavityEdge == CavEdge_Lower_Xc
    || m_CurrentPart.m_Cavity.m_CavityEdge == CavEdge_Lower )
  {
    m_dDLCavityHorizBaseLine = -m_dDLCavityHorizBaseLine ;
  }
  m_iNewCavityExposure = m_iCavityExposure = m_CurrentPart.m_Cavity.m_iCavityExp_us;
  m_iNewFocusExposure = m_iCavityFocusExposure = m_CurrentPart.m_Cavity.m_iDefocusExp_us;
  m_iCentralZoneWidthForX = m_CurrentPart.m_Cavity.m_iCentralZoneWidth_pix;
  m_dTargetForFocusExpAdjust =m_CurrentPart.m_Cavity.m_dTargetForFocusExpAdjust ;
  m_dNormBirghtnessForCavity = m_CurrentPart.m_Cavity.m_dNormBrightnessForCavity;
  m_ShiftsDL.Reset();
  SetMeasureConturParameters();
  int iMinWidth_pix = ROUND(m_CurrentPart.m_Blank.m_dBlankWidth_um * 0.7 / m_dScale_um_per_pix);
  int iMinHeight_pix = ROUND(m_CurrentPart.m_Blank.m_dBlankHeight_um * 0.7 / m_dScale_um_per_pix);
  m_iMinSideSize_pix = min(iMinHeight_pix , iMinWidth_pix);
}


// returns number of parts
int MPPT::SaveKnownParts()
{
  if ( m_KnownParts.size() == 0 )
    return 0;

  FXRegistry Reg( "TheFileX\\Micropoint" );
  string FileName = (LPCTSTR) Reg.GetRegiString(
    "Parameters" , "CavitiesData" , "Cavities" );
  string Path( string( GetMainDir() ) + FileName + ".dat" );
  if ( PathFileExists( Path.c_str() ) )
  {
    string PathForOld( Path );
    struct stat result;
    if ( stat( Path.c_str() , &result ) == 0 )
    {
      time_t mod_time = result.st_mtime;
      tm *ltm = localtime( &mod_time );
      std::stringstream date( ios_base::out );
      date << 1900 + ltm->tm_year << 1 + ltm->tm_mon << ltm->tm_mday
        << "_" << ltm->tm_hour << ltm->tm_min << ltm->tm_sec;
      PathForOld += date.str() + ".dat";
    }
    else
      PathForOld += (LPCTSTR) (GetTimeAsString_ms() + ".dat");
    rename( Path.c_str() , PathForOld.c_str() );
  }
  ofstream myfile( Path.c_str() );
  if ( myfile.is_open() )
  {
    for ( auto it = m_KnownParts.begin(); it != m_KnownParts.end(); it++ )
    {
      string Out = (LPCTSTR) (it->m_Cavity.ToString() + '\n');
      myfile << Out;
    }
    myfile.close();
    FxSendLogMsg( MSG_INFO_LEVEL , _T( "MPPT::SaveKnownParts" ) , 0 ,
      _T( "Cavities Data saved to File '%s' (%d records)" ) ,
      Path.c_str() , m_KnownParts.size() );
  }
  else
  {
    FxSendLogMsg( MSG_ERROR_LEVEL , _T( "MPPT::SaveKnownParts" ) , 0 ,
      _T( "Can't open File '%s' for Cavities Data" ) , Path.c_str() );
  }
  int iNSavedToRegistry = 0 ;
  for ( auto it = m_KnownParts.begin(); it != m_KnownParts.end(); it++ )
  {
    iNSavedToRegistry += SavePartDataToRegistry( *it ) ;
  }
  FxSendLogMsg( MSG_INFO_LEVEL , _T( "MPPT::SaveKnownParts" ) , 0 ,
    _T( "Parts Data saved to Registry' (%d records)" ) , iNSavedToRegistry );
  return iNSavedToRegistry ;
}

int MPPT::RestoreKnownParts(LPCTSTR pPartName , bool bReportToEngine)
{
  FXRegistry Reg( "TheFileX\\Micropoint" );
  BOOL bIsRegistryDBInitialized = Reg.GetRegiInt( "Parts" , "IsInitialized" , 0 ) ;
  if ( !bIsRegistryDBInitialized ) //Take from file
  {
    string FileName = (LPCTSTR) Reg.GetRegiString(
      "Parameters" , "CavitiesData" , "Cavities" );
    string Path( string( GetMainDir() ) + FileName + ".dat" );

    ifstream myfile( Path );
    if ( myfile.is_open() )
    {
      m_KnownParts.clear();
      string line;
      while ( getline( myfile , line ) )
      {
        if ( line[ 0 ] == '/' )
          continue;

        PartParams NewCavity;
        if ( NewCavity.FromString( line.c_str() ) )
          m_KnownParts.push_back( NewCavity );
        else
        {
          FxSendLogMsg( MSG_ERROR_LEVEL , _T( "MPPT::RestoreKnownParts" ) , 0 ,
            _T( "Bad format in File '%s' with Cavities Data - %s" ) ,
            Path.c_str() , line.c_str() );
        }
      }
      myfile.close();
      if ( m_WorkingState != DL_MeasCavityXY )
      {
        FxSendLogMsg( MSG_INFO_LEVEL , _T( "MPPT::RestoreKnownParts" ) , 0 ,
          _T( "%d Cavities data restored from file %s" ) ,
          m_KnownParts.size() , Path.c_str() );
      }
      if ( !m_PartName.IsEmpty() )
        SelectCurrentPart( m_PartName );

      return (int)m_KnownParts.size();
    }
    else
    {
      FxSendLogMsg( MSG_ERROR_LEVEL , _T( "MPPT::RestoreKnownParts" ) , 0 ,
        _T( "Can't open File '%s' with Cavities Data" ) , Path.c_str() );
    }
  }
  else // restore from registry
  {
    m_KnownParts.clear() ;
    m_iSelectedPart = -1;
    LPCTSTR pNewRegistryName = "TheFileX\\Micropoint\\PartsData" ;
    FXRegistry PartsNewReg( pNewRegistryName ) ;
    FXStringArray PartsNames ;
    PartsNewReg.EnumerateFolderForSubfolders( "" , PartsNames ) ;
    if ( PartsNames.GetCount() > 1 )
    { // there we will be all times after new registry created
      FXString SelectedPart;
      if ( pPartName )
      {
        PartsNewReg.WriteRegiString(
          "LastSettings" , "SelectedPart" , pPartName );
        SelectedPart = pPartName;
      }
      else
        SelectedPart = PartsNewReg.GetRegiString(
        "LastSettings", "SelectedPart", "");
      SelectedPart.MakeUpper();
      for ( int i = 0 ; i < PartsNames.Count() ; i++ )
      {
        if ( PartsNames[i] != "LastSettings" )
        {
          PartParams NewPart( PartsNames[ i ] ) ;
          m_KnownParts.push_back( NewPart ) ;
          if ( PartsNames[ i ] == SelectedPart )
          {
            m_iSelectedPart = i;
            m_SelectedPartName = SelectedPart ;
            m_CurrentPart = m_KnownParts[ m_iSelectedPart ];
            m_PartName = m_CurrentPart.m_Name.c_str();
            if (bReportToEngine)
              SendMessageToEngine("OK" , ( LPCTSTR )( m_GadgetName + _T("_AnswerForWedge") ));
          }
        }
      }
      if ( pPartName && (m_iSelectedPart == -1) && bReportToEngine )
        SendMessageToEngine("Error; No such part" , ( LPCTSTR )( m_GadgetName + _T("_AnswerForWedge") ));
    }
    else // there we will be only once, when new registry created
    {
      FXRegistry PartsReg( "TheFileX\\Micropoint\\Parts" ) ;
      FXString SelectedPart = Reg.GetRegiString(
        "Parts" , "SelectedPart" , "" );
      FXRegistry Reg( "TheFileX\\Micropoint" ) ;
      Reg.WriteRegiString( "PartsData" , "SelectedPart" , SelectedPart ) ;
      FXStringArray PartNames ;
      PartsReg.EnumerateFolderForSubfolders( "" , PartNames ) ;
      for ( int i = 0 ; i < PartNames.Count() ; i++ )
      {
        PartParams NewPart( NULL , NULL ) ;
        RestorePartDataFromRegistry( PartNames[ i ] , NewPart ) ;
        // following really creates record in registry for current part (copy old registry to new one)
        NewPart.RestorePartDataFromRegistry( pNewRegistryName , PartNames[ i ] ) ;
        m_KnownParts.push_back( NewPart ) ;
        if ( PartNames[ i ] == SelectedPart )
        {
          m_iSelectedPart = i;
          m_SelectedPartName = SelectedPart ;
          m_CurrentPart = m_KnownParts[ m_iSelectedPart ];
          m_PartName = m_CurrentPart.m_Name.c_str();
        }
      }
    }
  }
  return 0;
}


int MPPT::CheckAndAddCavity()
{
  if ( abs( m_cLastLeftCavitySize_um ) != 0.
    && abs( m_cLastRightCavitySize_um ) != 0.
    && m_dLastCavityArea_um2 != 0. ) // we did measure something
  {
    auto it = m_KnownParts.begin();
    if (!m_PartName.IsEmpty())
    {
      for (; it != m_KnownParts.end(); it++)
      {
        if (m_PartName == it->m_Name.c_str())
        {
          FillCurrentPartParameters(*it);
          break;
        }
      }
    }
    else if ( m_iSelectedPart < (int)m_KnownParts.size() )
      FillCurrentPartParameters( *(it += m_iSelectedPart) );
    if ( it == m_KnownParts.end() )
    {
      PartParams NewCavity;
      FillCurrentPartParameters( NewCavity ) ;
      m_KnownParts.push_back( NewCavity ) ;
      it = m_KnownParts.end() - 1 ;
    }
    return SavePartDataToRegistry(*it);
  }

  return 0;
}

void MPPT::FillCurrentPartParameters( PartParams& Part )
{
  if (!m_PartName.IsEmpty())
    Part.m_Name = (LPCTSTR)m_PartName;
  else if ( Part.m_Name.empty() )
    Part.m_Name = _T("Default");
  Part.m_Cavity.m_dPlaneArea_um2 = m_dLastCavityArea_um2 ;
  Part.m_Cavity.m_iCentralZoneWidth_pix = m_iCentralZoneWidthForX;
  Part.m_Cavity.m_iCavityExp_us = m_iCavityExposure;
  if ( abs( m_cLastLeftCavitySize_um ) != 0.
    && abs( m_cLastRightCavitySize_um ) != 0. )
  {
    cmplx cSize = 0.5 * (m_cLastLeftCavitySize_um + m_cLastRightCavitySize_um);
    Part.m_Cavity.m_dPlaneHeight_um = cSize.imag();
    Part.m_Cavity.m_dPlaneWidth_um = cSize.real();
  }
}

bool MPPT::SaveXYCalibData( FXString * pStatistics )
{
  FXString Dir = GetMainDir();
  if ( m_CalibMatrixSize.cx && m_CalibMatrixSize.cy
    && (m_NewCalibData.size() == m_CalibMatrixSize.cx * m_CalibMatrixSize.cy) )
  {
    FXRegistry Reg( "TheFileX\\Micropoint" );
    FXString ItemName( GetWorkingModeName() );
    ItemName += _T( "CalibFileName" );
    FXString DefaultFileName( GetShortWorkingModeName() );
    DefaultFileName += "XYCalib.dat";
    FXString FileName = Reg.GetRegiString(
      "Parameters" , ItemName , DefaultFileName );
    FXString Path = Dir + FileName;
    Named_ofstream myfile;
    myfile.open( ((LPCTSTR) Path) );
    if ( myfile.is_open() )
    {
      FXString Out;
      Out.Format( "MatrixSize(%d,%d)\n" , m_CalibMatrixSize.cx , m_CalibMatrixSize.cy );
      myfile << (LPCTSTR) Out;
      for ( size_t i = 0; i < m_NewCalibData.size(); i++ )
      {
        CoordsCorresp Pt = m_NewCalibData[ i ];
        Out.Format( "%3d,%.2f,%.2f,%.3f,%.3f\n" , i ,
          Pt.World.real() , Pt.World.imag() , Pt.FOV.real() , Pt.FOV.imag() );
        myfile << (LPCTSTR) Out;
      }
      if ( pStatistics )
        myfile << (LPCTSTR) *pStatistics;

      myfile.close();
      return true;
    }
    else
    {
      FxSendLogMsg( MSG_ERROR_LEVEL , _T( "MPPT::SaveXYCalibData" ) , 0 ,
        _T( "Can't open file '%s' for writing " ) , (LPCTSTR) Path );
    };
    SendScalesToRender();
  }
  return false;
}


bool MPPT::RestoreXYCalibData()
{
  FXString Dir = GetMainDir();
  FXRegistry Reg( "TheFileX\\Micropoint" );
  FXString ItemName( GetWorkingModeName() );
  ItemName += _T( "CalibFileName" );
  FXString DefaultFileName( GetShortWorkingModeName() );
  DefaultFileName += "XYCalib.dat";
  FXString FileName = Reg.GetRegiString(
    "Parameters" , ItemName , DefaultFileName );
  FXString Path = Dir + FileName;
  Named_ifstream myfile;
  myfile.open( ((LPCTSTR) Path) );
  bool bOK = false;
  if ( myfile.is_open() )
  {
    string line;
    if ( getline( myfile , line ) )
    {
      if ( line.find( "MatrixSize(" ) >= 0 )
      {
        int iPos = (int) line.find( '(' );
        LPCTSTR pScan = line.c_str() + iPos + 1;
        int Szx , Szy;
        int iNNumbers = sscanf( pScan , "%d,%d" , &Szx , &Szy );
        if ( (iNNumbers == 2) && (Szx > 0) && (Szy > 0) )
        {
          CoordPairs NewCalibData;
          CSize NewSize( Szx , Szy );
          int iNPts = Szx * Szy;
          CoordsCorresp NewPt;
          int iPtNumber = 0 , iNewPtNumber;
          while ( (iPtNumber < iNPts) && getline( myfile , line ) )
          {
            int iNScanned = sscanf( line.c_str() , "%d,%lf,%lf,%lf,%lf" , &iNewPtNumber ,
              &NewPt.World._Val[ _RE ] , &NewPt.World._Val[ _IM ] ,
              &NewPt.FOV._Val[ _RE ] , &NewPt.FOV._Val[ _IM ] );
            if ( iNScanned == 5 && (iPtNumber == iNewPtNumber) )
            {
              NewCalibData.push_back( NewPt );
              iPtNumber++;
            }
            else
            {
              FxSendLogMsg( MSG_ERROR_LEVEL , _T( "MPPT::RestoreXYCalibData" ) , 0 ,
                _T( "Error in File '%s' on Pt %d reading '%s'" ) , (LPCTSTR) Path ,
                iPtNumber , line.c_str() );
              break;
            }
          }
          if ( iPtNumber == iNPts )
          {
            m_CalibMatrixSize = m_NewCalibMatrixSize = NewSize;
            m_CalibData = m_NewCalibData = NewCalibData;
            FxSendLogMsg( MSG_INFO_LEVEL , _T( "MPPT::RestoreXYCalibData" ) , 0 ,
              _T( "Data from File '%s' restored (%d pts)" ) , (LPCTSTR) Path , iNPts );
            bOK = true;
            SendScalesToRender();
          }
        }
      }
    }
    myfile.close();
  }
  else
  {
    FxSendLogMsg( MSG_ERROR_LEVEL , _T( "MPPT::RestoreXYCalibData" ) , 0 ,
      _T( "Can't open file '%s' for writing " ) , (LPCTSTR) Path );
  };

  return bOK;
}


FXString MPPT::GetMainDir()
{
  FXRegistry Reg( "TheFileX\\Micropoint" );
  FXString Directory = Reg.GetRegiString(
    "Data" , "MainDirectory" , "d:/ErrosionImaging/" );
  return Directory;
}

bool MPPT::SaveZCalibData()
{
  FXRegistry Reg( "TheFileX\\Micropoint" );
  FXString ModeName( GetWorkingModeName() );
  FXString Name = ModeName + _T( "ZCalibFileName" );
  FXString DefaultFileName( GetShortWorkingModeName() );
  DefaultFileName += "ZCalib.dat";
  string FileName = (LPCTSTR) Reg.GetRegiString(
    "Parameters" , Name , DefaultFileName );
  string Path( string( GetMainDir() ) + FileName );
  if ( PathFileExists( Path.c_str() ) )
  {
    string PathForOld( Path );
    struct stat result;
    if ( stat( Path.c_str() , &result ) == 0 )
    {
      time_t mod_time = result.st_mtime;
      tm *ltm = localtime( &mod_time );
      std::stringstream date( ios_base::out );
      date << 1900 + ltm->tm_year << 1 + ltm->tm_mon << ltm->tm_mday
        << "_" << ltm->tm_hour << ltm->tm_min << ltm->tm_sec;
      PathForOld += date.str() + ".dat";
    }
    else
      PathForOld += (LPCTSTR) (GetTimeAsString_ms() + ".dat");
    rename( Path.c_str() , PathForOld.c_str() );
  }
  bool bOK = false;
  Named_ofstream myfile;
  myfile.open( Path.c_str() , std::ofstream::out | std::ofstream::binary );
  if ( myfile.is_open() )
  {
    switch ( m_WorkingMode )
    {
    case MPPTWM_UpSide:
    case MPPTWM_Down:
      bOK = SaveZCalibData( myfile );
      break;
    case MPPTWM_UpFront:
      break;
    case MPPTWM_FinalFront:
      break;
    case MPPTWM_FinalSide:
      break;
    }
    myfile.close();
    return bOK;
  }
  SENDERR( "Can't open file %s for save Z calib data %s" ,
    Path.c_str() , (LPCTSTR) FxLastErr2Mes() );
  return false;
}


bool MPPT::RestoreZCalibData()
{
  FXRegistry Reg( "TheFileX\\Micropoint" );
  FXString ModeName( GetWorkingModeName() );
  FXString Name = ModeName + _T( "ZCalibFileName" );
  FXString DefaultFileName( GetShortWorkingModeName() );
  DefaultFileName += "ZCalib.dat";
  string FileName = (LPCTSTR) Reg.GetRegiString(
    "Parameters" , Name , DefaultFileName );
  string Path( string( GetMainDir() ) + FileName );
  Named_ifstream myfile;
  myfile.open( (Path.c_str()) , std::ifstream::in | std::ofstream::binary );
  bool bOK = false;
  if ( myfile.is_open() )
  {
    myfile.m_Filename = Path;
    switch ( m_WorkingMode )
    {
    case MPPTWM_UpSide:
    case MPPTWM_Down:
      bOK = RestoreZCalibData( myfile );
      break;
    case MPPTWM_UpFront:
      break;
    case MPPTWM_FinalFront:
      break;
    case MPPTWM_FinalSide:
      break;
    }
    myfile.close();
    return bOK;
  }
  SENDERR( "Can't open file %s for save Z calib data %s" ,
    Path.c_str() , (LPCTSTR) FxLastErr2Mes() );
  return false;
}

bool MPPT::SaveZCalibData( Named_ofstream& myfile )
{
  FXString Out;
  Out.Format( "NPoints=%d Range=(%.1f,%.1f)\n" , m_ZNewCalibData.size() ,
    m_ZNewCalibData.front().m_dHeight , m_ZNewCalibData.back().m_dHeight );
  myfile << (LPCTSTR) Out;
  for ( auto it = m_ZCalibData.begin(); it != m_ZCalibData.end(); it++ )
  {
    Out.Format( "%2d Z=%6.2f FOV(%8.2f,%8.2f)\n" ,
      it - m_ZCalibData.begin() , it->m_dHeight , it->m_cFOV.real() , it->m_cFOV.imag() );
    myfile << (LPCTSTR) Out;
  }
  Out.Format( "PtZero(%7.2f,%7.2f) Scale=%.6f pix/um ZeroIndex=%d Scale_um_per_pix=%.5f\n " ,
    m_ZNewCalibData[ m_iZeroIndex ].m_cFOV.real() , m_ZNewCalibData[ m_iZeroIndex ].m_cFOV.imag() ,
    m_dZSensitivity_pix_per_um , m_iZeroIndex , m_dZScale_um_per_pix );
  myfile << (LPCTSTR) Out;

  myfile.close();
  SendScalesToRender();
  return true;
}


bool MPPT::RestoreZCalibData( Named_ifstream& myfile )
{
  bool bOK = false;
  string line;
  if ( getline( myfile , line ) )
  {
    int iEqPos = (int) line.find( '=' );
    size_t iNPoints = atoi( line.c_str() + iEqPos + 1 );
    if ( iNPoints > 0 )
    {
      m_ZNewCalibData.clear();
      for ( size_t i = 0; i < iNPoints; i++ )
      {
        if ( getline( myfile , line ) )
        {
          size_t iPtNum = atoi( line.c_str() );
          if ( iPtNum == i )
          {
            iEqPos = (int) line.find( '=' );
            if ( iEqPos > 0 )
            {
              double dZ = atof( line.c_str() + iEqPos + 1 );
              iEqPos = (int) line.find( '(' , iEqPos + 1 );
              if ( iEqPos > 0 )
              {
                double dXFOV = atof( line.c_str() + iEqPos + 1 );
                iEqPos = (int) line.find( ',' , iEqPos + 1 );
                if ( iEqPos > 0 )
                {
                  double dYFOV = atof( line.c_str() + iEqPos + 1 );
                  cmplx cFOV( dXFOV , dYFOV );
                  HeightMeasResult NewZPt( dZ , cFOV );
                  m_ZNewCalibData.push_back( NewZPt );
                }
                else
                  break;
              }
              else
                break;
            }
            else
              break;
          }
        }
        else
        {
          break;
        }
      }
      if ( iNPoints != m_ZNewCalibData.size() )
      {
        SENDERR( "Not enough or non consistent info in File %s (%d/%d)" ,
          myfile.m_Filename.c_str() , iNPoints , m_ZNewCalibData.size() );
      }
      else // All right
      {
        while ( getline( myfile , line ) )
        {
          if ( line.size() > 10 )
          {
            iEqPos = 0;
            iEqPos = (int) line.find( '(' , iEqPos + 1 );
            if ( iEqPos > 0 )
            {
              double dXFOV = atof( line.c_str() + iEqPos + 1 );
              iEqPos = (int) line.find( ',' , iEqPos + 1 );
              if ( iEqPos > 0 )
              {
                double dYFOV = atof( line.c_str() + iEqPos + 1 );
                cmplx cFOV( dXFOV , dYFOV );
                iEqPos = (int) line.find( '=' , iEqPos + 1 );
                if ( iEqPos > 0 )
                {
                  double dScale = atof( line.c_str() + iEqPos + 1 );
                  iEqPos = (int) line.find( '=' , iEqPos + 1 );
                  if ( iEqPos > 0 )
                  {
                    int iZeroSavedIndex = atoi( line.c_str() + iEqPos + 1 );
                    int iZeroIndex = ((int) m_ZNewCalibData.size() / 2);
                    if ( iZeroIndex == iZeroSavedIndex )
                    {
                      m_iZeroIndex = iZeroIndex;
                      m_cNominalZZero = cFOV;
                      m_dZSensitivity_pix_per_um = dScale;
                      m_dZScale_um_per_pix = 1. / dScale;
                      m_ZCalibData = m_ZNewCalibData;
                      bOK = true;
                      SendScalesToRender();
                    }
                  }
                }
              }
            }
            break;
          }
        }
        if ( !bOK )
        {
          SENDERR( "Bad statistics in File %s (inconsistent data)" ,
            myfile.m_Filename.c_str() );
        }
        else
        {
          FxSendLogMsg( MSG_INFO_LEVEL , _T( "MPPT::RestoreZCalibData" ) , 0 ,
            _T( "Data from File '%s' restored (%d pts)" ) ,
            myfile.m_Filename.c_str() , m_ZNewCalibData.size() );
        }
      }
    }
  }
  return bOK;
}

void MPPT::SaveLogMsg(LPCTSTR pFormat, ...)
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
    SENDERR("MPPT::SaveLogMsg ERROR: %s for file %s",
      strerror(GetLastError()), (LPCTSTR)m_CurrentLogFilePath);
  }
}
void MPPT::SaveOperativeLogMsg(LPCTSTR pFormat, ...)
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
    SENDERR("MPPT::SaveOperativeLogMsg ERROR: %s for file %s",
      strerror(GetLastError()), (LPCTSTR)m_CurrentOperativeLogFilePath);
  }
}

void MPPT::SaveFocusLog(LPCTSTR pFormat, ...)
{
  FXString Out;
  Out.Format("\n%24s: ", GetTimeAsString_ms());
  va_list argList;
  va_start(argList, pFormat);
  FXString AsString;
  AsString.FormatV(pFormat, argList);
  va_end(argList);

  Out += AsString;
  ofstream myfile((LPCTSTR)m_CurrentFocusLogFilePath, ios_base::app);
  if (myfile.is_open())
  {
    myfile << (LPCTSTR)Out;
    myfile.close();
  }
  else
  {
    SENDERR("MPPT::SaveLogMsg ERROR: %s for file %s",
      strerror(GetLastError()), (LPCTSTR)m_CurrentLogFilePath);
  }
}

void MPPT::SaveCSVLogMsg( LPCTSTR pFormat , ... )
{
  FXString Out;
  Out.Format( "\n%24s, " , GetTimeAsString_ms() );
  va_list argList;
  va_start( argList , pFormat );
  FXString AsString;
  AsString.FormatV( pFormat , argList );
  va_end( argList );

  Out += AsString;
  ofstream myfile( (LPCTSTR) m_CurrentCSVLogFilePath , ios_base::app );
  if ( myfile.is_open() )
  {
    myfile << (LPCTSTR) Out;
    myfile.close();
  }
  else
  {
    SENDERR( "MPPT::SaveCSVLogMsg ERROR: %s for file %s" ,
      strerror( GetLastError() ) , (LPCTSTR) m_CurrentLogFilePath );
  }
}

void MPPT::SaveCavityResultLogMsg( LPCTSTR pFormat , ... )
{
  FXString Out;
  Out.Format( "\n%s, Cavity=%3d :" , GetTimeAsString_ms() , m_iNProcessedCavities );
  va_list argList;
  va_start( argList , pFormat );
  FXString AsString;
  AsString.FormatV( pFormat , argList );
  va_end( argList );

  Out += AsString;
  ofstream myfile( (LPCTSTR) (m_CurrentImagesDir + _T("/Cavity.log")), ios_base::app );
  if ( myfile.is_open() )
  {
    myfile << (LPCTSTR) Out;
    myfile.close();
  }
  else
  {
    SENDERR( "MPPT::SaveCSVLogMsg ERROR: %s for file %s" ,
      strerror( GetLastError() ) , (LPCTSTR) m_CurrentLogFilePath );
  }
}

// Check save  mode and save if necessary
int MPPT::CheckAndSaveImage( const pTVFrame pImage , bool bFinal )
{
  if ( !bFinal )
  {
    switch ( m_SaveMode )
    {
    default:
    case SaveMode_No: return 0;
    case SaveMode_All: break;
    case SaveMode_OnePerSerie:
      if ( m_iAfterCommandSaved == 0 )
        break;
      m_iFrameCount++;
      return 0;
    case SaveMode_Decimate:
      if ( m_iFrameCount % m_iSaveDecimator == 0 )
        break;
      m_iFrameCount++;
      return 0;
    case Save_Final:
      {
        switch ( m_WorkingState )
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
    case Save_Bad:
      {
        return 0;
      }
      break;
    }
  }

  FXString FileName;
  if ( m_WorkingState == DL_ScaleCalib )
  {
    FileName.Format( "%s_%s[%d,%d,%d]_%d.bmp" , (LPCTSTR) GetTimeStamp() ,
      GetShortWorkingModeName() , (int) m_ShiftsDL.m_x ,
      (int) m_ShiftsDL.m_y , (int) m_ShiftsDL.m_z , m_iFrameCount++ );
  }
  else if ( m_WorkingState == DL_LaserCalib )
  {
    FileName.Format( "%s_%s%s%d.bmp" , (LPCTSTR) GetTimeStamp() ,
      GetShortWorkingModeName() , GetWorkingStateName() , m_iFrameCount++ );
  }
  else if ( (m_WorkingState == DL_CaptureCavityFinalImage)
    /*|| (m_WorkingState ==  DL_MeasCavity)
    || (m_WorkingState == DL_MeasCavityXY)*/ )
  {
    FileName.Format( "%s_%sCavXY_%d.bmp" , (LPCTSTR) GetTimeStamp() ,
      GetShortWorkingModeName() , m_iNProcessedCavities );
  }
  else if ( m_WorkingState == DL_CaptureZWithCorrectHeigth )
  {
    FileName.Format( "%s_%sCavZ_%d.bmp" , (LPCTSTR) GetTimeStamp() ,
      GetShortWorkingModeName() , m_iNProcessedCavities );
  }

  //   else if ( (m_WorkingState <= (DL_CaptureCavityFinalImage + 10))
  //           && m_WorkingState >= DL_CavityJumpForZ )
  //   {
  //     FileName.Format( "%s_%s_CavZ%d_%d.bmp" , (LPCTSTR) GetTimeStamp() ,
  //       GetShortWorkingModeName() , m_iNProcessedCavities , m_iFrameCount++ ) ;
  //   }
  else
  {
    FileName.Format( "%s_%s%s%d.bmp" , (LPCTSTR) GetTimeStamp() ,
      GetShortWorkingModeName() , GetWorkingStateName() , m_iFrameCount++ );
  }
  if (m_bWaitForContinueCommand == 1) // avoid stabilization images saving
    return 0;

  return SaveImage( pImage , FileName );
}

// Check save  mode and save if necessary
int MPPT::SaveImage( const pTVFrame pImage , LPCTSTR pFileName )
{

  m_CurrentDataDir = CheckCreatePartResultDir();
  if ( m_CurrentDataDir.IsEmpty() )
  {
    SENDERR( "Can't create Main Data Directory for image saving" );
    return 0;
  }
  FXString FileName = m_CurrentImagesDir + pFileName;

  return (0 != saveSH2BMP( FileName , pImage->lpBMIH , pImage->lpData ));
}

FXString MPPT::CheckCreatePartResultDir()
{
  FXString DirName = CheckCreateDataDir();
  if ( DirName.IsEmpty() )
    return DirName;

  FXString SubDir( GetShortWorkingModeName() );
  SubDir += GetDateAsString() + _T( "/" );

  m_CurrentDataDir = DirName + SubDir;
  if ( !FxVerifyCreateDirectory( m_CurrentDataDir ) )
  {
    SENDERR( "Can't create result directory '%s' " , (LPCTSTR) m_CurrentDataDir );
    return FXString();
  };
  FXString ImagesDir = m_CurrentDataDir/* + _T( "Images/" )*/;
  if ( !FxVerifyCreateDirectory( ImagesDir ) )
  {
    SENDERR( "Can't create images directory '%s' " , (LPCTSTR) ImagesDir );
    return FXString();
  };
  m_CurrentImagesDir = ImagesDir;
  return m_CurrentDataDir;
}

FXString MPPT::CheckCreateDataDir()
{
  FXRegistry Reg( "TheFileX\\Micropoint" );
  FXString Directory = GetMainDir();
  FXString DataSubDir = Reg.GetRegiString( "Data" , "DataSubDir" , "Data/" );
  FXString DataDir( Directory + DataSubDir );
  if ( !FxVerifyCreateDirectory( DataDir ) )
  {
    SENDERR( "Can't create data directory '%s' " , (LPCTSTR) DataDir );
    return FXString();
  };
  FXString ReportsDir = Directory + _T( "Reports\\" );
  if ( !FxVerifyCreateDirectory( ReportsDir ) )
  {
    SENDERR( "Can't create reports directory '%s' " , (LPCTSTR) ReportsDir );
    return FXString();
  };
  m_CurrentReportsDir = ReportsDir ;
  return DataDir;
}

FXString MPPT::CheckCreateCurrentLogs()
{
  FXRegistry Reg( "TheFileX\\Micropoint" );
  FXString Directory = GetMainDir();
  FXString LogSubDir = Reg.GetRegiString( "Data" , "LogDir" , "Logs/" );

  FXString LogPath( Directory + LogSubDir );
  if ( !FxVerifyCreateDirectory( LogPath ) )
  {
    SENDERR( "Can't create data directory '%s' " , (LPCTSTR) LogPath );
    return FXString();
  };
  LogPath += (((GetDateAsString() + "_") + GetShortWorkingModeName()));
  m_CurrentCSVLogFilePath = LogPath  
    + Reg.GetRegiString( "Data" , "CsvFileSuffixAndExt" , ".csv" );
  m_CurrentFocusLogFilePath = LogPath + _T("_FocusLog.log");
  m_CurrentOperativeLogFilePath = LogPath+ _T("OperativeLog.log") ;
  LogPath += 
    + Reg.GetRegiString( "Data" , "LogFileSuffixAndExt" , ".log" );

  return LogPath;
}

int MPPT::SavePartDataToRegistry( LPCTSTR pPartname , bool bTakeFromCurrent )
{
  FXRegistry Reg( "TheFileX\\Micropoint" );

  for ( size_t i = 0 ; i < m_KnownParts.size() ; i++ )
  {
    if ( m_KnownParts[ i ].m_Name.c_str() == pPartname )
    {
      if (bTakeFromCurrent)
        m_KnownParts[i] = m_CurrentPart;
      SavePartDataToRegistry( m_KnownParts[ i ] ) ;
      return (int) i ;
    }
  }
  return -1;
}

int MPPT::SavePartDataToRegistry( PartParams& Part )
{
  FXRegistry Reg( "TheFileX\\Micropoint" );

  string PartFolder( "Parts\\" ) ;
  PartFolder += Part.m_Name ;

  LPCTSTR pFormat = _T( "%.1f" ) ;
  Reg.WriteRegiString( PartFolder.c_str() , "PartName" ,
    Part.m_Name.c_str() ) ;
  Reg.WriteRegiString( PartFolder.c_str() , "Description" ,
    Part.m_Description.c_str() ) ;
  Reg.WriteRegiInt( PartFolder.c_str() , "CavityExposure_us" , Part.m_Cavity.m_iCavityExp_us ) ;
  Reg.WriteRegiInt( PartFolder.c_str() , "FocusExposure_us" , Part.m_Cavity.m_iDefocusExp_us ) ;
  Reg.WriteRegiInt( PartFolder.c_str() , "FocusLight(1-Ring,2-Straight,3-Both)" ,
    (int) Part.m_Cavity.m_bCavFocusRingLightOn | (((int) Part.m_Cavity.m_bCavFocusStraightLightOn) << 1) ) ;
  Reg.WriteRegiInt( PartFolder.c_str() , "XYLight(1-Ring,2-Straight,3-Both)" ,
    (int) Part.m_Cavity.m_bCavXYRingLightOn | (((int) Part.m_Cavity.m_bCavXYStraightLightOn) << 1) ) ;
  Reg.WriteRegiInt(PartFolder.c_str(), "BlankExposure_us", Part.m_Blank.m_iBlankExp_us);
  Reg.WriteRegiInt(PartFolder.c_str(), "GaugeExposure_us", Part.m_Gauge.m_iBlankExp_us);
  Reg.WriteRegiDouble( PartFolder.c_str() , "TargetForFocusExpAdjust" ,
    Part.m_Cavity.m_dTargetForFocusExpAdjust , "%.4f" ) ;
  Reg.WriteRegiDouble( PartFolder.c_str() , "TargetForCavityExpAdjust" ,
    Part.m_Cavity.m_dNormBrightnessForCavity , "%.4f" ) ;

  Reg.WriteRegiInt( PartFolder.c_str() , "CentralZoneWidth_pix" , Part.m_Cavity.m_iCentralZoneWidth_pix ) ;
  Reg.WriteRegiDouble( PartFolder.c_str() , "CavityPlaneWidth_um" ,
    Part.m_Cavity.m_dPlaneWidth_um , pFormat ) ;
  Reg.WriteRegiDouble( PartFolder.c_str() , "CavityPlaneHeight_um" ,
    Part.m_Cavity.m_dPlaneHeight_um , pFormat ) ;
  Reg.WriteRegiDouble( PartFolder.c_str() , "CavityPlaneArea_um2" ,
    Part.m_Cavity.m_dPlaneArea_um2 , pFormat ) ;
  Reg.WriteRegiDouble( PartFolder.c_str() , "CavityDistBetweenAreas_um" ,
    Part.m_Cavity.m_dDistBetweenAreas_um , pFormat ) ;
  Reg.WriteRegiDouble( PartFolder.c_str() , "BlankWidth_um" ,
    Part.m_Blank.m_dBlankWidth_um , pFormat ) ;
  Reg.WriteRegiDouble( PartFolder.c_str() , "BlankHeight_um" ,
    Part.m_Blank.m_dBlankHeight_um , pFormat ) ;
  Reg.WriteRegiDouble( PartFolder.c_str() , "SizeTolerance_perc" ,
    Part.m_Blank.m_dSizeTolerance_perc , pFormat ) ;
  Reg.WriteRegiDouble(PartFolder.c_str(), "YCorrectionWidth_um",
    Part.m_Cavity.m_dYCorrectionWidth_um, pFormat);
  Reg.WriteRegiInt( PartFolder.c_str() , "FindBlackCorners" , Part.m_Cavity.m_bFindBlackCorners ) ;
  Reg.WriteRegiInt( PartFolder.c_str() , "CavityEdge(1-low,2-up)" , Part.m_Cavity.m_CavityEdge ) ;
  Reg.WriteRegiInt( PartFolder.c_str() , "BlankEdge(LURL-1234)" , Part.m_Blank.m_UsedBlankEdge) ;
  Reg.WriteRegiDouble( PartFolder.c_str() , "YShiftForBlank_um" ,
    Part.m_Blank.m_dYShiftForBlank_um, pFormat ) ;
  return 1 ;
}

int MPPT::SavePartDataToRegistryEx( PartParams& Part )
{
  FXRegistry Reg( "TheFileX\\Micropoint" );

  string PartFolder( "PartsData\\" ) ;
  PartFolder += Part.m_Name + "\\";
  string CavityFolder = PartFolder + _T( "Cavity" ) ;
  string BlankFolder = PartFolder + _T( "Blank" ) ;
  string GaugeFolder = PartFolder + _T( "Gauge" ) ;

  LPCTSTR pFormat = _T( "%.1f" ) ;
  Reg.WriteRegiString( PartFolder.c_str() , "PartName" ,
    Part.m_Name.c_str() ) ;
  Reg.WriteRegiString( PartFolder.c_str() , "Description" ,
    Part.m_Description.c_str() ) ;

  Reg.WriteRegiInt( CavityFolder.c_str() , "CavityExposure_us" , Part.m_Cavity.m_iCavityExp_us ) ;
  Reg.WriteRegiInt( CavityFolder.c_str() , "FocusExposure_us" , Part.m_Cavity.m_iDefocusExp_us ) ;
  Reg.WriteRegiInt( CavityFolder.c_str() , "FocusLight(1-Ring,2-Straight,3-Both)" ,
    (int) Part.m_Cavity.m_bCavFocusRingLightOn | (((int) Part.m_Cavity.m_bCavFocusStraightLightOn) << 1) ) ;
  Reg.WriteRegiInt( CavityFolder.c_str() , "XYLight(1-Ring,2-Straight,3-Both)" ,
    (int) Part.m_Cavity.m_bCavXYRingLightOn | (((int) Part.m_Cavity.m_bCavXYStraightLightOn) << 1) ) ;
  Reg.WriteRegiDouble( CavityFolder.c_str() , "TargetForFocusExpAdjust" ,
    Part.m_Cavity.m_dTargetForFocusExpAdjust , "%.4f" ) ;
  Reg.WriteRegiDouble( CavityFolder.c_str() , "TargetForCavityExpAdjust" ,
    Part.m_Cavity.m_dNormBrightnessForCavity , "%.4f" ) ;
  Reg.WriteRegiInt( CavityFolder.c_str() , "FindBlackCorners" , Part.m_Cavity.m_bFindBlackCorners ) ;
  Reg.WriteRegiInt( CavityFolder.c_str() , "CavityEdge(1-low,2-up)" , Part.m_Cavity.m_CavityEdge ) ;
  Reg.WriteRegiInt( CavityFolder.c_str() , "CentralZoneWidth_pix" , Part.m_Cavity.m_iCentralZoneWidth_pix ) ;
  Reg.WriteRegiDouble( CavityFolder.c_str() , "CavityPlaneWidth_um" ,
    Part.m_Cavity.m_dPlaneWidth_um , pFormat ) ;
  Reg.WriteRegiDouble( CavityFolder.c_str() , "CavityPlaneHeight_um" ,
    Part.m_Cavity.m_dPlaneHeight_um , pFormat ) ;
  Reg.WriteRegiDouble( CavityFolder.c_str() , "CavityPlaneArea_um2" ,
    Part.m_Cavity.m_dPlaneArea_um2 , pFormat ) ;
  Reg.WriteRegiDouble( CavityFolder.c_str() , "CavityDistBetweenAreas_um" ,
    Part.m_Cavity.m_dDistBetweenAreas_um , pFormat ) ;
  Reg.WriteRegiDouble( CavityFolder.c_str() , "YCorrectionWidth_um" ,
    Part.m_Cavity.m_dYCorrectionWidth_um , pFormat );
  
  Reg.WriteRegiInt( BlankFolder.c_str() , "BlankExposure_us" , Part.m_Blank.m_iBlankExp_us );
  Reg.WriteRegiDouble( BlankFolder.c_str() , "BlankWidth_um" ,
    Part.m_Blank.m_dBlankWidth_um , pFormat ) ;
  Reg.WriteRegiDouble( BlankFolder.c_str() , "BlankHeight_um" ,
    Part.m_Blank.m_dBlankHeight_um , pFormat ) ;
  Reg.WriteRegiDouble( BlankFolder.c_str() , "SizeTolerance_perc" ,
    Part.m_Blank.m_dSizeTolerance_perc , pFormat ) ;
  Reg.WriteRegiInt( BlankFolder.c_str() , "BlankEdge(LURL-1234)" , Part.m_Blank.m_UsedBlankEdge ) ;
  Reg.WriteRegiDouble( BlankFolder.c_str() , "YShiftForBlank_um" ,
    Part.m_Blank.m_dYShiftForBlank_um , pFormat ) ;
 
  Reg.WriteRegiInt( GaugeFolder.c_str() , "BlankExposure_us" , Part.m_Gauge.m_iBlankExp_us );
  Reg.WriteRegiDouble( GaugeFolder.c_str() , "BlankWidth_um" ,
    Part.m_Gauge.m_dBlankWidth_um , pFormat ) ;
  Reg.WriteRegiDouble( GaugeFolder.c_str() , "BlankHeight_um" ,
    Part.m_Gauge.m_dBlankHeight_um , pFormat ) ;
  Reg.WriteRegiDouble( GaugeFolder.c_str() , "SizeTolerance_perc" ,
    Part.m_Gauge.m_dSizeTolerance_perc , pFormat ) ;
  Reg.WriteRegiInt( GaugeFolder.c_str() , "BlankEdge(LURL-1234)" , Part.m_Gauge.m_UsedBlankEdge ) ;
  Reg.WriteRegiDouble( GaugeFolder.c_str() , "YShiftForBlank_um" ,
    Part.m_Gauge.m_dYShiftForBlank_um , pFormat ) ;
  return 1 ;
}

int MPPT::RestorePartDataFromRegistry(
  PartParams& Part , PartParams* pPrototype , string& PartFolder ,
  LPCTSTR pPartName )
{
  bool bProto = ( pPrototype != NULL );
  FXRegistry Reg("TheFileX\\Micropoint");

  string Descr = Reg.GetRegiString(PartFolder.c_str() , "Description" , "No Description");
  int  iResult = ( Descr != "No Description" ); // No Description (should be filled in registry after first saving)
  Part.m_Description = Descr;
  Part.m_Name = Reg.GetRegiString(PartFolder.c_str() , "PartName" , pPartName);
  Part.m_Cavity.m_iCavityExp_us = Reg.GetRegiInt(PartFolder.c_str() , "CavityExposure_us" ,
    bProto ? pPrototype->m_Cavity.m_iCavityExp_us : m_iCavityExposure);
  Part.m_Cavity.m_iDefocusExp_us = Reg.GetRegiInt(PartFolder.c_str() , "FocusExposure_us" ,
    bProto ? pPrototype->m_Cavity.m_iDefocusExp_us : m_iCavityFocusExposure);
  int iLightMask = Reg.GetRegiInt(PartFolder.c_str() , "FocusLight(1-Ring,2-Straight,3-Both)" , 
    bProto ? (pPrototype->m_Cavity.m_bCavFocusRingLightOn | ( pPrototype->m_Cavity.m_bCavFocusStraightLightOn << 1 )) : 2);
  Part.m_Cavity.m_bCavFocusRingLightOn = ( iLightMask & 1 ) != 0;
  Part.m_Cavity.m_bCavFocusStraightLightOn = ( iLightMask & 2 ) != 0;
  iLightMask = Reg.GetRegiInt(PartFolder.c_str() , "XYLight(1-Ring,2-Straight,3-Both)" , 
    bProto ? ( pPrototype->m_Cavity.m_bCavXYRingLightOn | ( pPrototype->m_Cavity.m_bCavXYStraightLightOn << 1 ) ) : 2);
  Part.m_Cavity.m_bCavXYRingLightOn = ( iLightMask & 1 ) != 0;
  Part.m_Cavity.m_bCavXYStraightLightOn = ( iLightMask & 2 ) != 0;
  Part.m_Cavity.m_dTargetForFocusExpAdjust = Reg.GetRegiDouble(PartFolder.c_str() ,
    "TargetForFocusExpAdjust" , bProto ? pPrototype->m_Cavity.m_dTargetForFocusExpAdjust : 0.88);
  Part.m_Cavity.m_dNormBrightnessForCavity = Reg.GetRegiDouble(PartFolder.c_str() ,
    "TargetForCavityExpAdjust" , bProto ? pPrototype->m_Cavity.m_dNormBrightnessForCavity : 0.95);
  Part.m_Blank.m_iBlankExp_us = Reg.GetRegiInt(PartFolder.c_str() , "BlankExposure_us" , 
    bProto ? pPrototype->m_Blank.m_iBlankExp_us : m_iFrontExposure_us);
  iLightMask = Reg.GetRegiInt( PartFolder.c_str() , "BlankLight(1-Ring,2-Straight,3-Both)" ,
    bProto ? (pPrototype->m_Blank.m_bBlankXYRingLightOn | (pPrototype->m_Blank.m_bBlankXYStraightLightOn << 1)) : 2 );
  Part.m_Gauge.m_bBlankXYRingLightOn = Part.m_Blank.m_bBlankXYRingLightOn = (iLightMask & 1) != 0;
  Part.m_Gauge.m_bBlankXYStraightLightOn = Part.m_Blank.m_bBlankXYStraightLightOn = (iLightMask & 2) != 0;
  Part.m_Gauge.m_iBlankExp_us = Reg.GetRegiInt(PartFolder.c_str() , "GaugeExposure_us" ,
    bProto ? pPrototype->m_Gauge.m_iBlankExp_us : m_iFrontExposure_us);
  Part.m_Cavity.m_iCentralZoneWidth_pix = Reg.GetRegiInt(PartFolder.c_str() , "CentralZoneWidth_pix" ,
    bProto ? pPrototype->m_Cavity.m_iCentralZoneWidth_pix : m_iCentralZoneWidthForX);
  Part.m_Cavity.m_dPlaneWidth_um = Reg.GetRegiDouble(PartFolder.c_str() , "CavityPlaneWidth_um" ,
    bProto ? pPrototype->m_Cavity.m_dPlaneWidth_um : Part.m_Cavity.m_dPlaneWidth_um);
  Part.m_Cavity.m_dPlaneHeight_um = Reg.GetRegiDouble(PartFolder.c_str() , "CavityPlaneHeight_um" ,
    bProto ? pPrototype->m_Cavity.m_dPlaneHeight_um : Part.m_Cavity.m_dPlaneHeight_um);
  Part.m_Cavity.m_dPlaneArea_um2 = Reg.GetRegiDouble(PartFolder.c_str() , "CavityPlaneArea_um2" ,
    bProto ? pPrototype->m_Cavity.m_dPlaneArea_um2 : Part.m_Cavity.m_dPlaneArea_um2);
  Part.m_Cavity.m_dDistBetweenAreas_um = Reg.GetRegiDouble(PartFolder.c_str() , "CavityDistBetweenAreas_um" ,
    bProto ? pPrototype->m_Cavity.m_dDistBetweenAreas_um : 400.);
  Part.m_Blank.m_dBlankWidth_um = Reg.GetRegiDouble(PartFolder.c_str() , "BlankWidth_um" ,
    bProto ? pPrototype->m_Blank.m_dBlankWidth_um : m_dBlankWidth_um);
  Part.m_Blank.m_dBlankHeight_um = Reg.GetRegiDouble(PartFolder.c_str() , "BlankHeight_um" ,
    bProto ? pPrototype->m_Blank.m_dBlankHeight_um : m_dBlankHeight_um);
  Part.m_Blank.m_dSizeTolerance_perc = Reg.GetRegiDouble(PartFolder.c_str() , "SizeTolerance_perc" ,
    bProto ? pPrototype->m_Blank.m_dSizeTolerance_perc : Part.m_Blank.m_dSizeTolerance_perc);
  Part.m_Cavity.m_dYCorrectionWidth_um = Reg.GetRegiDouble(PartFolder.c_str() , "YCorrectionWidth_um" ,
    bProto ? pPrototype->m_Cavity.m_dYCorrectionWidth_um : 0.);
  Part.m_Cavity.m_bFindBlackCorners = Reg.GetRegiInt(PartFolder.c_str() , "FindBlackCorners" ,
    bProto ? pPrototype->m_Cavity.m_bFindBlackCorners : 0);
  Part.m_Cavity.m_CavityEdge = ( CAVITY_EDGE )Reg.GetRegiInt(PartFolder.c_str() , "CavityEdge(1-low,2-up)" ,
    bProto ? pPrototype->m_Cavity.m_CavityEdge : ( int )Part.m_Cavity.m_CavityEdge);
  Part.m_Blank.m_UsedBlankEdge = ( SQ_EDGE_AND_CORNERS )Reg.GetRegiInt(PartFolder.c_str() , "BlankEdge(LURL-1234)" ,
    bProto ? pPrototype->m_Blank.m_UsedBlankEdge : ( int )m_UsedBlankEdge);
  Part.m_Blank.m_dYShiftForBlank_um = Reg.GetRegiDouble(PartFolder.c_str() , "YShiftForBlank_um" ,
    bProto ? pPrototype->m_Blank.m_dYShiftForBlank_um : 95.);
  Part.m_Cavity.m_dDefocusingThreshold = Reg.GetRegiDouble( "Cavity" , "DefocusingThreshold" ,
    bProto ? pPrototype->m_Cavity.m_dDefocusingThreshold :  0.7 );
  Part.m_Cavity.m_dDefocusingLongStep = Reg.GetRegiDouble( "Cavity" , "DefocusingLongStep_um" ,
    bProto ? pPrototype->m_Cavity.m_dDefocusingLongStep : 50. );
  Part.m_Cavity.m_dDefocusingShortStep = Reg.GetRegiDouble( "Cavity" , "DefocusingShortStep_um" ,
    bProto ? pPrototype->m_Cavity.m_dDefocusingShortStep : 10. );

  return iResult;

}
int MPPT::RestorePartDataFromRegistry( 
  LPCTSTR pPartName , PartParams& Part , PartParams* pGaugeParams )
{
  FXRegistry Reg( "TheFileX\\Micropoint" );

  string PartFolder( "Parts\\" ) ;
  PartFolder += pPartName ;

  int iRes = RestorePartDataFromRegistry( Part , NULL , PartFolder , pPartName );
  string GaugeFolder = PartFolder + "\\Gauge";
  PartParams m_GaugeForCurrentPart ;
  iRes += RestorePartDataFromRegistry( m_GaugeForCurrentPart , &Part ,
    GaugeFolder , (string( "GaugeFor" ) + pPartName).c_str() );
  Part.m_Gauge = m_GaugeForCurrentPart.m_Gauge ;
  //int iRes = Part.RestorePartDataFromRegistry( NULL , pPartName , false ); // set default if doesn't exists in registry
  return iRes;
}

int MPPT::SelectCurrentPart( LPCTSTR pPartName )
{
  auto it = m_KnownParts.begin();
  for ( ; it != m_KnownParts.end(); it++ )
  {
    if ( it->m_Name == pPartName )
    {
      m_CurrentPart = *it;
      m_iSelectedPart = (int) (it - m_KnownParts.begin()) ;
      FXRegistry Reg( "TheFileX\\Micropoint" );
      Reg.WriteRegiString( "PartsData" , "SelectedPart" , m_CurrentPart.m_Name.c_str() );
      return 1;
    }
  }
  return 0;
}

int MPPT::CheckAndAddPart( LPCTSTR pPartName , PartParams& PartData )
{
  auto it = m_KnownParts.begin();
  for ( ; it != m_KnownParts.end(); it++ )
  {
    if ( it->m_Name == pPartName )
    {
      *it = m_CurrentPart ;
      return 1;
    }
  }
  m_KnownParts.push_back( PartData ) ;
  return 0;
}

int MPPT::CheckAndSaveFinalImages( CContainerFrame * pOutInfo , bool bBad , LPCTSTR pSuffix )
{
  int iCounter = (m_WorkingMode == MPPTWM_Down) ?
    m_iNProcessedCavities : m_iNProcessedBlanks ;
  LPCTSTR pObjName = (m_WorkingMode == MPPTWM_Down) ? _T("Cav") : _T("Blank");
  cmplx NumViewPt = m_cLastROICenter + cmplx(m_cLastROICenter.real() * 0.6, 0.);
  pOutInfo->AddFrame(CreateTextFrame(NumViewPt, "0x00ff00", 14, "ViewNum",
    0, _T("%#s=%d"), pObjName , iCounter));
  FXString Suffix ;
  Suffix.Format( "_%s%s_%d" , GetShortWorkingModeName() ,
    pSuffix ? pSuffix : (bBad ? _T( "BadFinal" ) : _T( "Final" )) , iCounter ) ;
  if ( m_SaveFinalImageWithGraphics )
  {
    FXString SaveParam ;
    SaveParam.Format( "1 %s" , m_CurrentImagesDir + _T( "WithGraphics\\" ) ) ;
    pOutInfo->AddFrame( CreateTextFrame( SaveParam , "SaveImage" ) ) ;
    pOutInfo->AddFrame( CreateTextFrame( (LPCTSTR) Suffix , _T( "ImageSaveSuffix" ) ) ) ;
  }
  if ( m_SaveMode == Save_Final || pSuffix )
  {
    FXString FileName ;
    FileName.Format( "%s_%s_%s.bmp" , (LPCTSTR) GetTimeStamp() , (LPCTSTR) Suffix , GetWorkingStateName() ) ;
    SaveImage( m_pCurrentImage , FileName ) ;
  }

  return 0;
}
