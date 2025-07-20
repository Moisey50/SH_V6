// UserExampleGadget.h : Declaration of the UserExampleGadget class



#ifndef __INCLUDE__ObjectMatcher_H__
#define __INCLUDE__ObjectMatcher_H__


#pragma once
#include "helpers\UserBaseGadget.h"
#include "gadgets/ContainerFrame.h"
#include "helpers/FramesHelper.h"
#include "opencv2/imgproc/types_c.h"
#include "opencv2/highgui/highgui.hpp"
#include "opencv2/imgproc/imgproc.hpp"


typedef struct tagContour
{
	std::vector <cv::Point> CountourVector;
	FXString  Label;
  double    dWidth;
  double    dHeight;
  double    dAngle;
  CDPoint   dCenter;
}Contour;


class ObjectMatcherGadget : public UserBaseGadget
{
protected:

	ObjectMatcherGadget();

public:

	//	Example variables

	double		dMaxAllowedDev;
	FXString	sMatchMethod;
  int			  iMaxSizeDeviation;
  bool		  bIgnoreSizeCompare;
  int		    iMinimalFigLength;


  //CDPoint cdRight,cdLeft,cdUp,cdDown;
	static const char* pList;

	//	Mandatory functions
  FXArray <Contour> m_TemplateCountourArr;
	FXLockObject m_Lock;
	void PropertiesRegistration();
	void ConnectorsRegistration();

	CDataFrame* DoProcessing(const CDataFrame* pDataFrame);	
	//CDataFrame* m_pPatternDataFrame;

  CContainerFrame* m_pContainer;
	void AsyncTransaction(CDuplexConnector* pConnector, CDataFrame* pParamFrame);
	int FigureParser(CDataFrame* pParamFrame);
	void AddTemplateFigure( CFigureFrame* pFigureFrame , double dAngle, CDPoint cdCenter );
  void LoadTemplatesFromFile(FXString sPath);
  void SaveTemplatesToFile(FXString sPath);
  void GetParamFromString(FXString fxstr, FXString &fxLabel, double &dAngle, CDPoint &cdCenter);
  double GetDistance(CDPoint &pt1, CDPoint &pt2);

  //bool ScanProperties(LPCTSTR text, bool& Invalidate);
  //bool PrintProperties(FXString& text);
  //bool ScanSettings(FXString& text);
  void FindFigureHeightandWidth(Contour &SourceCountour);
  void GetFigureCenterandAngle(const CDataFrame* pDataFrame,CFigureFrame* pFigureFrame, double &dAngle, CDPoint &cdCenter);

	DECLARE_RUNTIME_GADGET(ObjectMatcherGadget);
};

#endif	// __INCLUDE__UserExampleGadget_H__

