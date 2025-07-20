// ExposureControl.h: interface for the ExposureControl class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_EXPOSURECONTROL_H__FB505A21_2D48_11D3_8501_00A0C9616FBC__INCLUDED_)
#define AFX_EXPOSURECONTROL_H__FB505A21_2D48_11D3_8501_00A0C9616FBC__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "serio.h"
  // for external timers

#define EXPOSURE_START   560
#define DEFAULT_EXPOSURE 340
#define MAX_EXPOSURE ( 1590 - EXPOSURE_START )

/*    
#define EXPOSURE_START   100
#define DEFAULT_EXPOSURE 200
#define MAX_EXPOSURE ( 900 )
*/
class ExposureControl  
{
public:
	int SetExposure( int NScans );
	int GetExposure();
	CCSerIO * m_Channel;
	ExposureControl();
	virtual ~ExposureControl();

  char m_ExpControlBuffer[ 7 ] ;
  short * m_PrIn ;
  short * m_Ext ;

};

#endif // !defined(AFX_EXPOSURECONTROL_H__FB505A21_2D48_11D3_8501_00A0C9616FBC__INCLUDED_)
