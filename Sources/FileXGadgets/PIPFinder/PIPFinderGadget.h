// UserExampleGadget.h : Declaration of the UserExampleGadget class



#ifndef __INCLUDE__PIPFinderGadget_H__
#define __INCLUDE__PIPFinderGadget_H__

#pragma once
#include "opencv2/core/core.hpp"
#include "opencv2/features2d/features2d.hpp"
#include "opencv2/highgui/highgui.hpp"
#include "opencv2/imgproc/imgproc.hpp"
//#include "opencv2/nonfree/features2d.hpp"



#include <vector>
#include <string>
#include <algorithm>
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#pragma once
#include "helpers\UserBaseGadget.h"
#include "gadgets/ContainerFrame.h"
class PIPFinderGadget : public UserBaseGadget
{
protected:

	PIPFinderGadget();
  virtual ~PIPFinderGadget()
  {} ;

	bool bFullShow;
private:

public:
	CContainerFrame* m_pContainer;
	//	Mandatory functions


	void PropertiesRegistration();
	void ConnectorsRegistration();
	CDataFrame* DoProcessing(const CDataFrame* pDataFrame);
	void AsyncTransaction(CDuplexConnector* pConnector, CDataFrame* pParamFrame);
    void FindCenterOfMass(cv::Mat &img_1, cv::Mat &img_2,CDPoint &cdLeft, CDPoint &cdRight);

	DECLARE_RUNTIME_GADGET(PIPFinderGadget);
};

#endif	// __INCLUDE__UserExampleGadget_H__

