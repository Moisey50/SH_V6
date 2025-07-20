
#if !defined(AFX_EXPOSURECONTROL_H__FB505A21_2D48_11D3_8501_00A0C9616FBC__INCLUDED_)
#define AFX_EXPOSURECONTROL_H__FB505A21_2D48_11D3_8501_00A0C9616FBC__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "serio.h"
  // for external timers

#define EXPOSURE_START   23
#define DEFAULT_EXPOSURE 140
#define DEFAULT_ASYNC_EXP 100 // meteor
#define MAX_EXPOSURE ( 1200 )
#define MAX_ASYNC_EXP 360

class CMainFrame;
/*    
#define EXPOSURE_START   100
#define DEFAULT_EXPOSURE 200
#define MAX_EXPOSURE ( 900 )
*/
class ExposureControl  
{
public:
	int SetVSyncFront( int bFall );
	int m_iAsyncExposure_ms ; // Asynchronous exposure
	int m_iAsyncPeriod_ms ;
	int m_iAsynchronousMode;
	CString GetStatus();
	
  int ActualProgram(); //wrapper function!
  int ActualProgram_Meteor() ;
  int ActualProgram_Solios() ;
	
  int SetExposure( int NScans_or_ms ); // wrapper function!
  int SetExposure_Meteor( int iNScans_or_ms ) ;
  int SetExposure_Solios( int iNScans_or_ms ) ;

  int SetExposureMode( int iSync ) ; // moved here from CMainFrm, Alex 13.10.09

  int WaitCameraAnswer( void ) ; // moved here from CMainFrm, Alex 13.10.09

	int GetExposure(); // returns exposure in scans when synced, and in msecs when not synced.
	CCSerIO * m_Channel;
	ExposureControl();
	virtual ~ExposureControl();
  CMainFrame* m_pOwner ;

  int m_iExpStart ;
  int m_iExpEnd ;
  int m_iExposure ; // stores current exposure data, in units of scans

  // the following variables are grabber dependent
  int m_iMaxExposure ; // in scans
  int m_iMinExposure ; // in scans
  int m_iDefaultExposure ;
  int m_iMaxAsyncExposure ;
//   int m_iMinAsyncExposure ;
  int m_iDefaultAsyncExposure ;
  double m_dScanTime ;
  void InitExposureData() ;
  
  int ReleaseChannel(void);
  int CheckAndTakeChannel(void);
  void CleanCamera_RS_InputBuffer( void ) ;
};

#endif // !defined(AFX_EXPOSURECONTROL_H__FB505A21_2D48_11D3_8501_00A0C9616FBC__INCLUDED_)
