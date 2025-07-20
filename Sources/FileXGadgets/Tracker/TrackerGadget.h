// UserExampleGadget.h : Declaration of the UserExampleGadget class



#ifndef __INCLUDE__Tracker_H__
#define __INCLUDE__Tracker_H__


#pragma once
#include "helpers\UserBaseGadget.h"
#include "gadgets/ContainerFrame.h"
#include <opencv2/core/types_c.h>
#include "opencv2/highgui/highgui.hpp"
#include "opencv2/imgproc/imgproc.hpp"
#include "opencv2/features2d/features2d.hpp"
#include "Math\FRegression.h"


typedef struct tagPointTime
{
	CDPoint _pt;
	double _time;
	tagPointTime(CDPoint pt = CDPoint(-1, -1), double time = 0)
	{
		_pt = pt;
		_time = time;
	}
}PointTime;


class TrackerGadget : public UserBaseGadget
{
protected:

	TrackerGadget();

public:

	double		m_dMaxAllowedView;
	double		m_dMaxAllowedPattern;
	static const char* pList;
	FXString	m_sMatchMethod;

	FXArray<PointTime> m_centersArr;

	CDPoint       m_prevCenter;

	CFRegression m_FRegrPt;

	CDPoint m_cdTrackPt;
	double  m_dRadius;
	double m_dTemplateToROICoeff;

	cv::Mat m_template;
	//cv::Rect m_Rect;

	CDPoint m_ROI_Origin;

	bool m_bselect_ROI;

	double m_dFirstMatchCoeff;

	//CDPoint cdCenterPoint;

	FXLockObject m_Lock;
	void PropertiesRegistration();
	void ConnectorsRegistration();

	CDataFrame* DoProcessing(const CDataFrame* pDataFrame);	


  CContainerFrame* m_pContainer;
  void AsyncTransaction(CDuplexConnector* pConnector, CDataFrame* pParamFrame);
  //cv::Mat TrackerGadget::CropROI(const cv::Mat &img);
  //cv::Mat TrackerGadget::CropROI(const cv::Mat &img, double sizeCoeff);
  cv::Mat TrackerGadget::ConvertVideoFrameToCvMat(const CVideoFrame* pVideoFrame);
  //CDPoint  TrackerGadget::TrackTemplateMatching(cv::Mat &img, const cv::Mat &templ, double &dMatch);
  CDPoint  TrackerGadget::TrackKeyPoints(cv::Mat &img, const cv::Mat &templ, double &dMatch);
  CDPoint  TrackerGadget::GetNextCenter(CDPoint pt, double time);
  void TrackerGadget::UpdateTemplateRect(CDPoint center);
  void TrackerGadget::UpdateTemplateROI(cv::Mat Img, CDPoint center);


  cv::Rect TrackerGadget::GetRect(CDPoint center, double rad);
  cv::Mat TrackerGadget::CropROI(const cv::Mat &img, CDPoint center, double rad, double sizeCoeff);
  cv::Mat TrackerGadget::GetDefaultRect(const cv::Mat &img, CDPoint center, double rad, double sizeCoeff);
  CDPoint TrackTemplateMatching(cv::Mat &img, const cv::Mat &templ, double &dMatch, bool bSearchAllImage = false);
  cv::Mat TrackerGadget::GetEdges(const cv::Mat &img);
  DECLARE_RUNTIME_GADGET(TrackerGadget);
};

#endif	// __INCLUDE__UserExampleGadget_H__

