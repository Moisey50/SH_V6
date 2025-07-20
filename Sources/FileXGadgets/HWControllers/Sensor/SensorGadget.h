// UserExampleGadget.h : Declaration of the UserExampleGadget class



#ifndef __INCLUDE__SensorGadget_H__
#define __INCLUDE__SensorGadget_H__

#pragma once

#include "helpers\UserBaseGadget.h"
#include "helpers\FramesHelper.h"
#include <math\intf_sup.h>
#include "SensTypes.h"


class SensorGadget : public UserBaseGadget
{
protected:
  SensorGadget();

  Sensors m_Sensors;
  int   m_iViewDebug;
public:

	//	Mandatory functions

	void PropertiesRegistration();
	void ConnectorsRegistration();

	CDataFrame* DoProcessing(const CDataFrame* pDataFrame);
	void AsyncTransaction(CDuplexConnector* pConnector, CDataFrame* pParamFrame);
	//bool AccumCirclePoint();
	DECLARE_RUNTIME_GADGET(SensorGadget);
};

#endif	

