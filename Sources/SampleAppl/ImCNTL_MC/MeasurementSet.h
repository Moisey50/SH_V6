#ifndef __CMeasurementSet_h_
#define __CMeasurementSet_h_

#include "math\intf_sup.h"
#include <imageproc\clusters\Segmentation.h>
#include <imageproc\LineData.h>
#include "cpp_util\DynArray.h"
#include <Gadgets\gadbase.h>
// #include <gadgets/BaseGadgets.h>
#include <gadgets/TextFrame.h>
#include "GraphCameraControl.h"
#include "RenderWnd.h"
#include "SMCamControl.h"
#include <habitat.h>
#include "helpers\get_time.h"
#include "helpers\shwrapper.h"

extern int iCaptureGadgetMessage ;
extern int iImProcResultMessage ;
extern int iMsgFromSMThreads ;
extern int iMsgForLog ;


#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

class CSMCamControl ;

class CMeasurementSet
{
public:
  CMeasurementSet( CImageView * pView = NULL ) ;
  ~CMeasurementSet()
  {
    m_pView = NULL ;
//     if ( m_pShMemControl )
//     {
//       delete m_pShMemControl ;
//       m_pShMemControl = NULL ;
//     }
  }
  void ReleaseShMem()
  {
    if ( m_pShMemControl )
    {
      delete m_pShMemControl ;
      m_pShMemControl = NULL ;
    }
  }

  CImageView        * m_pView ;
  CString             m_CaptureGadgetName ;
  CString             m_CaptureControlName ; // Input name for capture control (as string)
  CString             m_ImageOrBackName ; // Input name for Image/Backround switching (quantity)
  CString             m_ClearBackName ; // Input name for Image/Backround switching (quantity)
  CString             m_LowPassOnName ; // Input name for Image/Backround switching (quantity)
  CString             m_ProcessControlName ; // Input name for task switching (as string)
  CString             m_ProcessScriptName ; // Input name for object descriptions as string sending
  CString             m_DataOutName ;  // pin name for data callback receiving
  CString             m_CaptureProperties ;
  CString             m_ReceivedFromCap ;
  CString             m_RenderGadgetName ;
  CString             m_RenderProperties ;
  CString             m_OpenReadFileSwitchName ;
  CString             m_OpenWriteFileSwitchName ;
  CString             m_SetName ;
  CString             m_Status ;
  CString             m_BMPSwitchName;
  CString             m_BMPRenderName;
  CString             m_CamOrLANSwitchPinName ;
  CString             m_ResultsExtractorGadgetName ;
  CString             m_ImageToLANExtractorGadgetName ;
  CString             m_LANOutputPinName ;
  CString             m_LANInputPinName;
  CString             m_BMPBufferControlPinName;
  CString             m_BMPCutRectPinName;
  CString             m_RadialDistrCalcGadgetName ;
  CString             m_ProfileGadgetOutputPinName;
  CString             m_ProfileGadgetControlName;
  CString             m_VideoSwitchName;
  CString             m_AviRenderInputName;
  int                 m_GraphMode ;  // 0 - not using
                                     // 1 - get images send results
                                     // 2 - send images and get results
  BOOL                m_bTriggerMode = FALSE;

  CCaptureGadget    * m_pCaptureGadget ;
  int                 m_iExposure ;
  int                 m_iGain ;
  int                 m_iTrigDelay_uS ;
  int                 m_iDebouncing_units ;
  double              m_dGamma ;

  CRenderGadget     * m_pRenderGadget ;
  CWnd              * m_pRenderWnd ;
  CWnd              * m_pChildWindow ;

  CFilterGadget     * m_pProcessGadget ;

  int                 m_iActiveInput ;

  CString             m_LastResultAsString ;

  CSMCamControl     * m_pShMemControl ;

  ProtectedProfileData m_pProfilePtArray;
  ProtectedSpotsData  m_SpotResults ;
  int                 m_iMaxBlobNumber ;
  CRect               m_FOV ;
  ProtectedLinesData  m_LineResults ;
  ProtectedLinesData  m_ProcLResults ;
  double              m_dCaptureFinishedAt ;
  double              m_dLastFrameInterval = 0.;
  double              m_dCaptureStartTime;
  double              m_dScanTime_us ;
  int                 m_iLastNBlobs ;
  int                 m_iLastNLines ;
  int                 m_iLastMaxIntensity ;
  int                 m_iLastMinIntensity ;
  int                 m_iMinArea ;
  int                 m_iAttemps;
  double              m_dLastBinThreshold,m_dLastRotThreshold;

  HANDLE m_hEVDataImg;

  CMeasurementSet& operator = (CMeasurementSet& Orig) 
  {
    m_pView = Orig.m_pView ;
    m_CaptureGadgetName = Orig.m_CaptureGadgetName ;
    m_CaptureControlName = Orig.m_CaptureControlName ;
    m_ImageOrBackName = Orig.m_ImageOrBackName ;
    m_LowPassOnName = Orig.m_LowPassOnName ;
    m_ClearBackName = Orig.m_ClearBackName ;
    m_ProcessControlName = Orig.m_ProcessControlName ;
    m_ProcessScriptName = Orig.m_ProcessScriptName ;
    m_DataOutName = Orig.m_DataOutName ;
    m_pCaptureGadget = Orig.m_pCaptureGadget ;
    m_RenderGadgetName = Orig.m_RenderGadgetName ;
    m_pRenderWnd = Orig.m_pRenderWnd ;
    m_pChildWindow = Orig.m_pChildWindow ;
    m_pProcessGadget = Orig.m_pProcessGadget ;
    m_pRenderGadget = Orig.m_pRenderGadget ;
    m_BMPSwitchName = Orig.m_BMPSwitchName;
    m_BMPRenderName = Orig.m_BMPRenderName;
    m_BMPBufferControlPinName = Orig.m_BMPBufferControlPinName;
    m_OpenReadFileSwitchName = Orig.m_OpenReadFileSwitchName ;
    m_OpenWriteFileSwitchName = Orig.m_OpenWriteFileSwitchName ;
    m_CamOrLANSwitchPinName = Orig.m_CamOrLANSwitchPinName ;
    m_ResultsExtractorGadgetName = Orig.m_ResultsExtractorGadgetName ;
    m_ImageToLANExtractorGadgetName = Orig.m_ImageToLANExtractorGadgetName ;
    m_LANOutputPinName = Orig.m_LANOutputPinName ;
    m_BMPCutRectPinName = Orig.m_BMPCutRectPinName;
     m_LANInputPinName = Orig.m_LANInputPinName;
    m_RadialDistrCalcGadgetName = Orig.m_RadialDistrCalcGadgetName ;
    m_ProfileGadgetOutputPinName = Orig.m_ProfileGadgetOutputPinName;
    m_ProfileGadgetControlName =  Orig.m_ProfileGadgetControlName;
    m_VideoSwitchName = Orig.m_VideoSwitchName;
    m_AviRenderInputName = Orig.m_AviRenderInputName;

    m_GraphMode = Orig.m_GraphMode ;
    m_SetName = Orig.m_SetName ;
    m_CaptureProperties = Orig.m_CaptureProperties ;
    m_RenderProperties = Orig.m_RenderProperties ;
    m_iMaxBlobNumber = Orig.m_iMaxBlobNumber ;
    m_pShMemControl = Orig.m_pShMemControl ;
    m_dScanTime_us = Orig.m_dScanTime_us ;
    m_iLastMaxIntensity = Orig.m_iLastMaxIntensity ;
    m_iMinArea = Orig.m_iMinArea ;
    m_iTrigDelay_uS = Orig.m_iTrigDelay_uS ;
    m_iDebouncing_units = Orig.m_iDebouncing_units ;
    m_hEVDataImg = Orig.m_hEVDataImg;
    return *this ;
  }
  BOOL WaitEndOfGrabAndProcess( int iTimeout_ms )
  {
    double dBeginTime = get_current_time() ;
    do
    {
      if ( m_dCaptureFinishedAt )
        return TRUE ;
      Sleep(5) ;
    } while ( get_current_time() - dBeginTime < iTimeout_ms ) ;
    return FALSE ;
  }
  CAM_CONTROL_MSG_ID AbstractCall( 
    long iCallID, LPCTSTR pszParameters, long iPar, double dPar ) ;
  CAM_CONTROL_MSG_ID Grab( int iNGrabs = 1 ) ;
  CAM_CONTROL_MSG_ID GrabBack() ;
  CAM_CONTROL_MSG_ID ClearBack() ;

  CAM_CONTROL_MSG_ID MeasureBlobs( double dNormThreshold ) ;
  CAM_CONTROL_MSG_ID MeasureLines( double dNormThreshold ) ;
  CAM_CONTROL_MSG_ID GetBlobParam( int iBlobNumber ) ;
  CAM_CONTROL_MSG_ID GetLineParam( int iBlobNumber ) ;
  CAM_CONTROL_MSG_ID GetExposure( int& iExposure ) ;
  CAM_CONTROL_MSG_ID SetExposure( int iExposure , BOOL bIn_us = FALSE ) ;
  CAM_CONTROL_MSG_ID GetGain( int& iGain ) ;
  CAM_CONTROL_MSG_ID SetGain( int iGain ) ;
  CAM_CONTROL_MSG_ID SetTriggerDelay( int iTriggerDelay_uS , int iSet = -1) ;
  CAM_CONTROL_MSG_ID GetTriggerDelay( int& iTriggerDelay_uS ) ;
  CAM_CONTROL_MSG_ID SetDebouncing( int iDebouncing_units ) ;
  CAM_CONTROL_MSG_ID GetDebouncing( int& iDebouncing_units ) ;
  CAM_CONTROL_MSG_ID GetLastMaxIntensity( int& iMaxIntensity ) ;
  CAM_CONTROL_MSG_ID GetLastMinIntensity( int& iMinIntensity ) ;
  CAM_CONTROL_MSG_ID SetWinPos( int iX , int iY ) ;
  CAM_CONTROL_MSG_ID SetMinArea(long iMinArea)  ;
  CAM_CONTROL_MSG_ID SetSyncMode(long bAsync)  ;
  CAM_CONTROL_MSG_ID CMeasurementSet::GetDiffractionMeasPar( LPCTSTR pResult ) ;
  CAM_CONTROL_MSG_ID CMeasurementSet::SetDiffractionMeasPar( 
    int iMeasMethod , int iRadX , int iRadY , int iBackDist ) ;
  CAM_CONTROL_MSG_ID SetThresholdForBinarization( double dThres ) ;
  CAM_CONTROL_MSG_ID SetThresholdForRotation( double dThres ) ;
  void SetScale( double dScale , TCHAR * pUnitName = NULL ) ;
  CAM_CONTROL_MSG_ID SetGamma( double dGamma ) ;
  CAM_CONTROL_MSG_ID GetGamma( double& dGamma ) ;

  CAM_CONTROL_MSG_ID SaveLastPicture( int iCamNum );
  CAM_CONTROL_MSG_ID SetBmpPath (CString sBMPPath);
  CAM_CONTROL_MSG_ID EnableLPFilter (int iEnable );
  CAM_CONTROL_MSG_ID SetROIforProfile(int x , int y, int iwidth , int iheight );
  CAM_CONTROL_MSG_ID GetProfile(int iProfN);



  bool EnableGetImagesFromLAN( bool bEnable ) ; // otherwise get from camera
  bool EnableSendResultsToLAN( bool bEnable ) ; // otherwise not send
  bool EnableSendImagesToLAN( bool bEnable ) ; // otherwise not send

  bool EnableRadialDistributionShow( bool bEnable ) ;
  bool GetRadialDistributionShow( bool& bEnabled ) ;

  CAM_CONTROL_MSG_ID SetTextThroughLAN(CString sText);

  bool CheckAndStopLiveVideo( DWORD dwTimeOut_ms = 1000 ) ;

  CAM_CONTROL_MSG_ID CloseDalsa(); 

};

typedef CArray <CMeasurementSet,CMeasurementSet& > MeasSets ;

#endif // __CMeasurementSet_h_