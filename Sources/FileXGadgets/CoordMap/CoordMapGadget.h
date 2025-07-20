// CoordMapGadget.h : Declaration of the CoordMapGadget class



#ifndef __INCLUDE__CoordMapGadget_H__
#define __INCLUDE__CoordMapGadget_H__


#pragma once
#include "helpers\UserBaseGadget.h"
#include "Calibration/Grid.h"
#include "gadgets\ContainerFrame.h"
#include <helpers/FramesHelper.h>
#include <helpers\MonitorEnum.h>
#include "helpers/StatisticsForSmallDotsAndLines.h"
#include "Calibration\GridDetector.h"
#include "Math\R2.h"

// #include "math\Quaternion.h"
enum EMode
{
  FOVtoWORLD = 0 ,
  CALIBRATION ,
  WORLDtoFOV ,
  SimpleCopy ,
  ImageOnly , 
  Throw ,
  SpotStatistics 
};

enum CalibStage
{
  Inactive = 0 ,
  DrawPattern ,
  CalibStarted ,
  AnswerReceived ,
  CalibFinishedOK ,
  CalibFailed ,
};

class CoordMapGadget : public UserBaseGadget
{
private:


  CDataFrame * DoCalibrationProcess(const CDataFrame* pDataFrame , bool bUnconditional = false );
  void DoTextProcess(const CDataFrame* pDataFrame, CContainerFrame* &container);
  void DoFigureProcess(const CDataFrame* pDataFrame, CContainerFrame* &container);

protected:

  static const char* m_pModeList;

  // Properties
  EMode m_ConvertMode;
  int m_iNBigDotsX;
  int m_iNBigDotsY;
  double m_dWCalibStep;
  double m_dPosTolerance_perc;
  int    m_iInsertMissed ;
  FXString m_CalibDataFileName ;
  FXString m_CalibDataLabel ;
  FXString m_CalibDataName ;
  FXString m_CenterDataName ;
  FXString m_MeasureDataNames ;
  FXStringArray m_MeasNamesAsArray ;
  FXString m_OffsetAsString ;
  cmplx    m_cOffset ; // for point (0,0)
  double   m_dScale_um_per_pixel ;
  FXArray <const CDataFrame*, const CDataFrame*> m_FramesForConversion ;
  vector< CFigureFrame* > m_Conturs ;
  SmallSpotsStatistics m_SpotsStatistics ;
  int     m_iMeasurentCount ;
  BOOL    m_bGenerateCaption ;

  FXString m_LastSavedCalibData ;
  bool m_bIsCalibrated;
  Grid m_CalibGrid;
  Spot m_CenterData ;
  cGridDetector m_GridDetector ;
  //   Quaternion  a ;

  CoordMapGadget();

public:

  void PropertiesRegistration();
  void ConnectorsRegistration();
  void ShutDown() ;

  virtual bool ScanProperties(LPCTSTR text, bool& Invalidate) ;

  void AsyncTransaction(CDuplexConnector* pConnector, CDataFrame* pParamFrame);
  CDataFrame* DoProcessing(const CDataFrame* pDataFrame);
  size_t ExtractConturs( const CDataFrame * pDataFrame ) ;
  bool FormStatistics( const CDataFrame * pDataFrame ) ;
  const CDataFrame * GetCalibData( const CDataFrame * InputData );
  const CDataFrame * GetCenterData( const CDataFrame * InputData );
  int GetMeasData(const CDataFrame * InputData );
  bool SaveCalibData() ;
  bool SaveCalibData( FXString& Result ) ;
  bool LoadCalibData() ;
  DECLARE_RUNTIME_GADGET(CoordMapGadget);
};

class MapControl : public UserBaseGadget
{
  friend BOOL CALLBACK MonitorEnumProc( HMONITOR hMonitor , HDC hDC , LPRECT pRect , LPARAM Par ) ;
  friend int WINAPI wWinMain( HINSTANCE hInstance , HINSTANCE , PWSTR pCmdLine , int nCmdShow ) ;

private:

protected:

  static const char* m_pModeList;

  // Properties
  int    m_ConvertMode;
  CSize  m_MatrixStep ;
  CSize  m_SpotSize ;
  int    m_iNBigDotsX;
  int    m_iNBigDotsY;
  double m_dWCalibStep; // In pixels
  double m_dPosTolerance_perc;
  int    m_iDisplay ;
  int    m_iInsertMissed ;
  HWND   m_hTargetWnd ;
  HWND   m_hScreenOverlapWindow ;
  DWORD  m_dwDrawThreadId ;
  HANDLE m_hDrawThreadHandle ;
  FXString m_CalibDataFileName ;
  FXString m_CalibDataLabel ;
  FXString m_CalibDataName ;
  FXString m_CenterDataName ;
  FXString m_MeasureDataNames ;
  FXArray< CRect > m_MonitorRects ;
  bool m_bIsCalibrated;
  double m_dLastCalibStartTime ;

  CalibStage m_CalibStage ;

  bool IsInactive()
  {
    return (m_CalibStage == Inactive
      || m_CalibStage == CalibFinishedOK
      || m_CalibStage == CalibFailed) ;
  }
public:
  bool   m_bDrawn ;

  MapControl();
  void PropertiesRegistration();
  void ConnectorsRegistration();

  void AsyncTransaction( CDuplexConnector* pConnector , CDataFrame* pParamFrame );
  CDataFrame* DoProcessing( const CDataFrame* pDataFrame );
  size_t DrawOnScreen() ;
  static DWORD WINAPI DrawThread( LPVOID Param ) ;

  DECLARE_RUNTIME_GADGET( MapControl );
};

#endif	// __INCLUDE__CoordMapGadget_H__

