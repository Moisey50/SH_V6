// UserExampleGadget.h : Declaration of the UserExampleGadget class



#ifndef __INCLUDE__Noga_H__
#define __INCLUDE__Noga_H__


#pragma once
#include "helpers\UserBaseGadget.h"
#include "gadgets/ContainerFrame.h"
#include <helpers\FramesHelper.h>
#include <iostream>
#include <iomanip>
#include <map>
#include <random>
#include <cmath>
#include <math\FigureProcessing.h>
#include <helpers\PropertyKitEx.h>
// #include "math\random.h"

class Noga : public UserBaseGadget
{
protected:
	Noga();
  virtual ~Noga() ;
public:

  FXLockObject m_LockSettings ;
  double m_dScaleMicronPerPix ;
  FXString m_MainConturName ;
  FXString m_TripplePtsName ;
  FXString m_sMeasurementPts ;
  FXDblArray m_MeasPts ;

  CFigure m_LastMainContur ;
  CFigure m_LastUpperSegment ;
  CFigure m_LastLowerSegment ;

  
  
  cmplx m_ExtremePt ;
  cmplx m_RightTripplePt ;
  cmplx m_LeftTriplePt ;

  CVideoFrame * m_pLastVideoFrame ;

	//	Mandatory functions
	void PropertiesRegistration();
	void ConnectorsRegistration();

	CDataFrame* DoProcessing(const CDataFrame* pDataFrame);
	void AsyncTransaction(CDuplexConnector* pConnector, CDataFrame* pParamFrame);
  //virtual bool ScanProperties(LPCTSTR text, bool& Invalidate) ;
  //virtual bool PrintProperties(FXString& text) ;

	DECLARE_RUNTIME_GADGET(Noga);
};

#endif	// __INCLUDE__Noga_H__

