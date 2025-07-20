#pragma once
#include "helpers\shmemcontrol.h"

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
  SAVEPICTURE ,
  ENABLELPFILTER ,
  SETDEBOUNCING ,
  GETDEBOUNCING ,
  SETGAMMA ,
  GETGAMMA ,
  CLEARBACK ,


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
  CameraControlMessage( CAM_CONTROL_MSG_ID iD , int iMsgLen = 8 )
  {
    m_iStatus = 0 ;
    m_iId = iD ;
    m_szStringData[0] = 0 ;
    m_iMessageLen = iMsgLen ;
  } ;
  ~CameraControlMessage()
  {

  }

  CAM_CONTROL_MSG_ID m_iId ;
  int                m_iMessageLen ; // length after m_iMessageLen
  int                m_iStatus ; // 0 - OK
  long               m_longIntParams[5] ;
  double             m_dDoubleParams[5] ;
  TCHAR              m_szStringData[400] ;
};
    

class CSMSoliosRequest :
  public CShMemControl
{
public:
  CSMSoliosRequest( LPCTSTR pShMemName , 
    LPCTSTR pInEventName , LPCTSTR pOutEventName , DWORD dwImageSize ) :
  CShMemControl( 1024 , 2048 , 8192 + dwImageSize , 
    pShMemName , pInEventName , pOutEventName ) 
  { 
    m_ShMemName = pShMemName ; 
    m_InEventName = pInEventName ;
    m_OutEventName = pOutEventName ;
    m_pImage = m_pArea + 8192 ;
    m_hOutEvent = NULL ;
    m_hImageOutEvt = NULL ;
  } ;
  ~CSMSoliosRequest(void);
  virtual int ProcessRequest( CameraControlMessage& Msg , 
    int iLen , int iTimeout_ms = 1000 , 
    int iBufLen = sizeof(CameraControlMessage) ) ;
  const LPBYTE GetImage() { return m_pImage ; } ;
  void SetOutImageEventName( LPCTSTR ImEventName ) ;
  BOOL WaitForImage( DWORD dwTimeout_ms )
  {
    return WaitForEvent(m_hImageOutEvt , dwTimeout_ms) ;
  }
  CString m_ShMemName ;
  CString m_InEventName ;
  CString m_OutEventName ;
  CString m_OutImageEventName ;
  HANDLE m_hImageOutEvt ;
  BYTE  * m_pImage ;
};
