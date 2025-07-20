// BrightnessAverageGadget.h : Declaration of the BrightnessAverageGadget class



#ifndef __INCLUDE__BrightnessAverageGadget_H__
#define __INCLUDE__BrightnessAverageGadget_H__


#pragma once
#include "helpers\UserBaseGadget.h"

class BrightnessAverageGadget : public UserBaseGadget
{
protected:

	BrightnessAverageGadget();

public:

	void PropertiesRegistration();
	void ConnectorsRegistration();

	CDataFrame* DoProcessing(const CDataFrame* pDataFrame);
	void AsyncTransaction(CDuplexConnector* pConnector, CDataFrame* pParamFrame);

	DECLARE_RUNTIME_GADGET(BrightnessAverageGadget);
};

typedef enum
{
  DS_AverageDiff = 0 ,
  DS_AbsMAxDiff = 1 ,
  DS_Laplace
} DiffSummWorkingMode ;

class CalcDiffSum : public UserBaseGadget
{
protected:

  CalcDiffSum();

public:
  CRect m_Area ; 
  FXString m_Format ;
  DWORD    m_FrameCntr ;
  DiffSummWorkingMode m_WorkingMode ;

  void PropertiesRegistration();
  void ConnectorsRegistration();

  CDataFrame* DoProcessing( const CDataFrame* pDataFrame );
  void AsyncTransaction( CDuplexConnector* pConnector , CDataFrame* pParamFrame );

  DECLARE_RUNTIME_GADGET( CalcDiffSum );
};


#endif	// __INCLUDE__BrightnessAverageGadget_H__

