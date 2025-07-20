// UserExampleGadget.h : Declaration of the UserExampleGadget class



#ifndef __INCLUDE__PlockGadget_H__
#define __INCLUDE__PlockGadget_H__


#pragma once
#include "helpers\UserBaseGadget.h"
#include "helpers\FramesHelper.h"
#include <math\intf_sup.h>
#include "PID_Control.h"

int
CircleFit( CmplxArray& Points , cmplx& Center , double& dRadius ) ;

typedef struct
{
	cmplx _point;
	double _time;
} PtOnCircle;


class PlockGadget : public UserBaseGadget
{
protected:
  PID_Control m_PID ;
  cmplx m_PrevPt ;
  double m_dAccumulatedAngle ;
  double m_dLastAngle ;
	bool m_circleDetected;
	double m_radius;
	cmplx  m_center;
	bool GetThreePoints(cmplx & pt1, cmplx & pt2, cmplx &pt3);
	bool GetEquation(cmplx pt1, cmplx pt2, cmplx pt3);
	double GetCentralAngle();
	void GetSidesLength(double & a, double & b, double & c);
	bool GetCircle();
  int AccumCirclePoint( cmplx pt , FXString time );
  int AccumCirclePoint( cmplx pt , double dTime );
  double GetSpeedCorrection();
	double angles[3];
  double ang[ 256 ];
  CmplxArray m_Points ;
  DWORD      m_dwMarkedDirections ;
  FXString m_OutputFormat ;
  double   m_dTargetAngle_Deg ;
  double   m_dSpeedAccelThreshold ;
  double m_dFirstPointTime ;
  double m_dOriginalFPS ;
  double   m_dMaxSpeedAccel ;
  double   m_dAngleMult ;
  double   m_dMaxAngleAccel ;
  bool     m_bAngleControl ;
  int      m_iAngleControlCnt ;
  double m_dLastCorrectionTime ;
  DWORD  m_dwLastCorrectionID ;
  DWORD  m_dwAdjustStep ;
  int    m_counter;

  PlockGadget();
  inline double GetAngle( cmplx coord )
  {
    double angRad = -std::arg( coord - m_center ) ;
    return angRad;
  }

//   inline double GetPtAngle( int iIndex )
//   {
//     cmplx Pt = m_CircleArr.GetAt( iIndex )._point ;
//     return GetAngle( Pt ) ;
//   }
//   inline double GetPtTime( int iIndex )
//   {
//     return m_CircleArr.GetAt( iIndex )._time ;
//   }


  inline double GetLastTurn( cmplx& NewPt )
  {
    double dAng1 = GetAngle( m_PrevPt ) ;
    double dAng2 = GetAngle( NewPt ) ;
    double dQ = GetDeltaAngle( dAng1 , dAng2 );
    return dQ ;
  }

public:

	//	Mandatory functions

	void PropertiesRegistration();
	void ConnectorsRegistration();

	CDataFrame* DoProcessing(const CDataFrame* pDataFrame);
	void AsyncTransaction(CDuplexConnector* pConnector, CDataFrame* pParamFrame);
  void Reset( bool bFullReset = false ) 
  { 
    m_Points.RemoveAll() ; 
    m_dwMarkedDirections = 0x0000ffff ; 
    m_circleDetected = false ;
    if ( bFullReset )
    {
      m_PID.Reset() ;
      m_dOriginalFPS = 0. ;
      m_dAccumulatedAngle = 0. ;
      m_dLastAngle = 0. ;
    }
  }

	//bool AccumCirclePoint();
	DECLARE_RUNTIME_GADGET(PlockGadget);
};

#endif	

