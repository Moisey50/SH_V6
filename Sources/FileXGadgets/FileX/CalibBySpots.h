// CalibBySpots.h : Declaration of the CalibBySpots class

#ifndef __INCLUDE__CalibBySpots_H__
#define __INCLUDE__CalibBySpots_H__

#pragma once


#include "helpers\UserBaseGadget.h"
#include "math\intf_sup.h"

#define MODE_CONVERT   0
#define MODE_CALIBRATE 1

class CalibBySpots : public UserBaseGadget
{
protected:

	CalibBySpots(void);

public:

  void ConnectorsRegistration();

	CDataFrame* DoProcessing(const CDataFrame* pDataFrame);

	void PropertiesRegistration();

	void AsyncTransaction(CDuplexConnector* pConnector, CDataFrame* pParamFrame);

  CDataFrame * ConvertToWorld(const CDataFrame * pFOVData);
  LPCTSTR Calibrate(const CDataFrame * pData);


  int m_iWorkingMode ;
  cmplx m_GridSpaceInWorld ;
  cmplx  m_WorldCenter ;


	DECLARE_RUNTIME_GADGET(CalibBySpots);
};

#endif //CalibBySpots


