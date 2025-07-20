#pragma once
#include "StdAfx.h"
#include "resource.h"		// main symbols
#include "helpers\shmemcontrol.h"
#include "ImagView.h"
#include <gadgets\QuantityFrame.h>

enum CAM_CONTROL_MSG_ID
{
  MSG_ID_OK = 0 ,
  GRABBACK = 100000000,
  GRAB,
  MEASUREBLOBS,
  GETBLOBPARAM,
  SETFFTFLAG,
  SETCAMERASCALE,
  GETCAMERASCALE,
  INITOPERATION,
  GETEXPOSURE, 
  SETEXPOSURE,
  MEASURELINES,
  GETLINEPARAM,
  GETLASTMININTENSITY,
  GETLASTMAXINTENSITY,
  SETFFTZONE,
  DOFFTFILTRATION,
  SAVERESTOREIMAGE,
  SETGETMEASAREAEXPANSION,
  GETMAXINTENSITY,
  SETWINPOS,
  SETEXPOSURESTARTPOSITION,
  GETEXPOSURESTARTPOSITION,
  MEASUREPOWER,
  GETPOWERDATA,
  SETINTEGRATIONRADIUS,
  GETINTEGRATIONRADIUS,
  MEASLINEPOWER,
  SETMINAREA,
  ABSTRACTCALL,
  SETSYNCMODE, 
  SETMARKERPOSISITON,
  GETDIFFRACTIONMEASPAR,
  SETDIFFRACTIONMEASPAR,
  SET_THRESHOLD_FOR_BINARIZATION ,
  SET_THRESHOLD_FOR_ROTATION ,
  SETGAIN ,
  GETGAIN ,
  SETTRIGGERDELAY ,
  GETTRIGGERDELAY ,
  SAVEPICTURE,
  ENABLELPFILTER,
  SETPROFILEROI,
  GETPROFILE,
  SETDEBOUNCING ,
  GETDEBOUNCING ,
  SETGAMMA ,
  GETGAMMA ,
  CLEARBACK ,
  SETBMPPATH,

  UNKNOWN_REQUEST = -1 ,
  UNKNOWN_ANSWER = -2 ,
  NOT_FULL_INITIALIZATION = -3 ,
  OPERATION_ERROR = -4 ,
  GRAB_AND_MEASURE_IS_NOT_FINISHED = -5 ,
  UNAVAILABLE_INFO_REQUESTED = -6 ,
  UNKNOWN_MEASUREMENT_MODE = -7 ,
  NO_APPROPRIATE_OBJECTS = -8 ,
  UNAVAILABLE_FUNCTION_REQUESTED = -9
} ;



class CameraControlMessage
{
public:
  CameraControlMessage()
  {
    m_iStatus = 0 ;
    m_szStringData[0] = 0 ;
    m_iMsgLen = sizeof(*this) ;
  } ;
  ~CameraControlMessage()
  {

  }

  CAM_CONTROL_MSG_ID m_iId ;
  int                m_iMsgLen ;
  int                m_iStatus ; // 0 - OK
  long               m_longIntParams[5] ;
  double             m_dDoubleParams[5] ;
  TCHAR              m_szStringData[400] ;
};
    
class CMeasurementSet ;


class CSMCamControl :
  public CShMemControl
{
public:
  CSMCamControl( LPCTSTR pShMemName , 
    LPCTSTR pInEventName , LPCTSTR pOutEventName , 
    CImageView * pView = NULL , CMeasurementSet * pSet = NULL ) ;
  ~CSMCamControl(void);
  static DWORD WINAPI  ControlFunction( LPVOID pParam ) ;
 virtual int Process( BYTE * pMsg , int iMsgLen ) ;
  void SetHostView( CImageView * pView) { m_pView = pView ; } ;
  void SetHostSet( CMeasurementSet * pSet ) { m_pMyMeasSet = pSet ;} ;
  virtual const char * GetName() { return "CSMCamControl" ; }

  CImageView * m_pView;
  CMeasurementSet * m_pMyMeasSet ;
  CString m_ShMemName ;
  CString m_InEventName ;
  CString m_OutEventName ;
  HANDLE m_hProcessingThread ;
  DWORD    m_dwThreadId ;
  bool m_bExit ;
};
