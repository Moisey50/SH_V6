// UserExampleGadget.h : Declaration of the UserExampleGadget class



#ifndef __INCLUDE__RandomNumber_H__
#define __INCLUDE__RandomNumber_H__


#pragma once
#include "helpers\UserBaseGadget.h"
#include "gadgets/ContainerFrame.h"
#include <helpers\FramesHelper.h>
#include <iostream>
#include <iomanip>
#include <map>
#include <random>
#include <cmath>
#include <math\intf_sup.h>
#include <helpers\PropertyKitEx.h>
// #include "math\random.h"
typedef std::random_device Myeng; 
typedef std::normal_distribution<double> Mydist; 


class GaussGen : public UserBaseGadget
{
protected:

//   std::default_random_engine generator; 
//   std::normal_distribution<double> distribution/*(/ *mean=* /0.0, / *stddev=* /1.0)*/; // automatically mean=0.0, std=1.0 
//   std::normal_distribution<double> distribution2 ;

	GaussGen();
  virtual ~GaussGen() ;

public:

	//	Example variables

	double		m_dMean;
  double		m_dSTD;
	BOOL        m_bHeart;
	int         iHRCounter;
  FXString    m_sMean ;
  FXString    m_sStdDev ;
  cmplx       m_cmplxMean ;
  cmplx       m_cmplxStd ;
  bool        m_bComplex ;
//   Normaldev   m_NormGen ;
//   Normaldev   m_ImagNormGen ;
  std::normal_distribution<double>::param_type m_CurrentReal ;
  std::normal_distribution<double>::param_type m_CurrentImag ;
  bool        m_bReset ;


	FXArray <double> m_SingleArr;
	FXArray <FXString> m_StockArr;
  FXLockObject     m_Protect ;
	//	Mandatory functions

	void PropertiesRegistration();
	void ConnectorsRegistration();

	CDataFrame* DoProcessing(const CDataFrame* pDataFrame);
	void AsyncTransaction(CDuplexConnector* pConnector, CDataFrame* pParamFrame);
  virtual bool ScanProperties(LPCTSTR text, bool& Invalidate) ;
  virtual bool PrintProperties(FXString& text) ;

	DECLARE_RUNTIME_GADGET(GaussGen);
};

#endif	// __INCLUDE__UserExampleGadget_H__

