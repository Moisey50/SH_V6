// UserExampleGadget.h : Declaration of the UserExampleGadget class



#ifndef __INCLUDE__RadiusConnecorGadget_H__
#define __INCLUDE__RadiusConnecorGadget_H__


#pragma once

#include <fxfc\fxfc.h>
#include "math\intf_sup.h"
#include "helpers\UserBaseGadget.h"
#include "gadgets/ContainerFrame.h"
#include <helpers\FramesHelper.h>

typedef struct tagContour
{
  FXArray<CDPoint> CountourVector;
  CDPoint    Up_Rel,Up_Abs;
  CDPoint    Down_Rel,Down_Abs;
  CDPoint    Right_Rel,Right_Abs;
  CDPoint    Left_Rel,Left_Abs;
  double     dWidthRel,dHeightRel;
  double     dWidthAbs,dHeightAbs;
  double     dAngle;
  CDPoint    dCenter;
}Contour;


class RadiusExtracterGadget : public UserBaseGadget
{
protected:

	RadiusExtracterGadget();

public:

	//	Example variables

	static const char* pList;
  
   
	//	Mandatory functions
	void PropertiesRegistration();
	void ConnectorsRegistration();

	CDataFrame* DoProcessing(const CDataFrame* pDataFrame);
	void AsyncTransaction(CDuplexConnector* pConnector, CDataFrame* pParamFrame);
  void GetFigureCenterandAngle(const CDataFrame* pDataFrame,CFigureFrame* pFigureFrame, double &dAngle, CDPoint &cdCenter);
  void FindFigureHeightandWidth(Contour &SourceCountour);
  void GetFarestPoints(FXArray <CDPoint> &ExPtArr, CDPoint &pt1, CDPoint &pt2);
  double GetDistance(CDPoint &pt1, CDPoint &pt2);
	DECLARE_RUNTIME_GADGET(RadiusExtracterGadget);
};

#endif	// __INCLUDE__UserExampleGadget_H__

