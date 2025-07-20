#pragma once
#include "helpers/UserBaseGadget.h"
#include <imageproc/statistics.h>
#include <imageproc/VFrameEmbedInfo.h>
#include <..\..\gadgets\common\imageproc\clusters\Segmentation.h>
#include <imageproc\ExtractVObjectResult.h>
#include <vector>
#include <iostream>
#include <fstream>
#include <sstream>      // std::stringstream

enum SS_ProcessingMode
{
  SSPM_Unknown = -1 ,
  SSPM_Idle ,
  SSPM_Process ,
  SSPM_ViewSOSSensor
};

enum SS_Outputs
{
  SSO_MainOutput = 0 ,
  SSO_OriginalsOut ,
  SSO_LPF_Control ,
  SSO_CameraControl ,
};

class SpotSymmetry :
  public UserBaseGadget 
{
public:
  SS_ProcessingMode  m_ProcessingMode = SSPM_Idle ;
  SS_ProcessingMode m_OldProcessingMode = SSPM_Unknown ;
  FXString m_GadgetName ;

  SpotArray    m_LastSpots ;
  int          m_iProfileWidth = 6 ;
  double       m_dAveragedHeight_pix = 0 ;
  double       m_dAveragedIntensity = 0 ;
  FXString     m_ImageSaveDirectory ;
  FXString     m_ResultFileNamePrefix ;
  FXString     m_LowPassArea ;
  FXString     m_LowPassAreaUsed ;
  FXString     m_SOS_Zone ;
  FXString     m_SOS_ZoneUsed ;
  FXString     m_sThresholdForLog ;
  double       m_dSkewThresForLog_um = 0. ;
  double       m_dSizeThresForLog_um = 0. ;
  double       m_dHLineDistThresForLog_um = 0. ;
  double       m_dTargetSpotAmplitude ;
  double       m_dMaxHorDistortion_perc ;
  double       m_dMaxVertDistortion_perc ;
  CRect        m_rcLowPassArea ;
  CRect        m_rcSOS_Zone ;
  int          m_iLastExposure_us ;
  int          m_iLastGain_dBx10 ;
  cmplx        m_cLastFrameSize ;
  int          m_iNExposureOK = 0 ;
  int          m_iNframesNoSpots = 0 ;
  int          m_iViewMode = 5 ;
  int          m_iDiffAverager = 5 ;
  double       m_dHLastAveragedDiff = 0 ;
  double       m_dVLastAveragedDiff = 0 ;

  vector<const CVideoFrame*>  m_Originals ;
  VideoFrameEmbeddedInfo m_VFEmbedInfo ;
  uint64_t     m_ui64LastFrameTimeStamp_ns = 0 ;
  double       m_dLastMax = 0. ;
  double       m_dMaxDeration_per_ms ;

  bool         m_bSaveOrigin = false ;
  bool         m_bSaveResult = false ;
  BOOL         m_bAutoExposeEnable = TRUE ;
  BOOL         m_bLicensed = FALSE ;
 public:
  SpotSymmetry() ;
  void ShutDown();

  void PropertiesRegistration();
  void ConnectorsRegistration();
  CDataFrame* DoProcessing( const CDataFrame* pDataFrame );
  void AsyncTransaction( CDuplexConnector* pConnector , CDataFrame* pParamFrame );
  int ProgramCamera( int iExposure_us , int iGain_dBx10 = -1 ) ;
  bool SetParametersToTVObject( LPCTSTR pParams ) ;
  bool SetTaskTVObject( int iTask ) ;
  DECLARE_RUNTIME_GADGET( SpotSymmetry );
 
  int CalcCamControlValues( const CVideoFrame * pVF , 
    int iMaxValInSpot , int& iNewExposure_us , int& iNewGain_dBx10 );
  int SetViewZone();
};

