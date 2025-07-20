// UserExampleGadget.h : Declaration of the UserExampleGadget class



#ifndef __INCLUDE__Tracker_H__
#define __INCLUDE__Tracker_H__


#pragma once
#include <fxfc/fxfc.h>
#include <fxfc/FXRegistry.h>
#include "helpers\UserBaseGadget.h"
#include "gadgets/ContainerFrame.h"
#include "helpers/FramesHelper.h"
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

#define RESULTS_PIN 0
#define CORRELATION_OUT_PIN 1

class Template
{
public:
  Template()
  {
    m_iIndex = -1 ;
  }
  Template( cv::Mat Matr , FXString& NotIndexedName , int iIndex )
  {
    m_Mat = Matr ;
    m_Name.Format( "%d_%s" , iIndex , ( LPCTSTR ) NotIndexedName ) ;
    m_iIndex = iIndex ;
  } ;
  ~Template()
  {
    if ( !m_Mat.empty() )
    {
      free( m_Mat.data ) ;
      m_Mat.release() ;
    }
  }
  Template& operator =(Template& Orig)
  {
    m_Mat = Orig.m_Mat ;
    m_Name = Orig.m_Name ;
    m_iIndex = Orig.m_iIndex ;
  }

  bool Save( LPCTSTR Dir )
  {
    FXString FileName( Dir + m_Name + _T( ".bmp" ) ) ;
    return cv::imwrite( ( LPCTSTR ) FileName , m_Mat ) ;
  }

  bool Load( LPCTSTR FilePath )
  {
    m_Mat = cv::imread( FilePath , cv::IMREAD_GRAYSCALE ) ;
    if ( m_Mat.empty() )
    {
      FxSendLogMsg( MSG_ERROR_LEVEL , "Load cvImage" , 0 ,
        "Can't load image from file %s" , FilePath ) ;
      return false ;
    }
    else
    {
      m_Name = FxGetFileTitle( FilePath );
      m_iIndex = atoi( ( LPCTSTR ) m_Name ) ;
    }
    return true ;
  }
  cv::Mat m_Mat ;
  FXString m_Name ;
  int      m_iIndex ;
};

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
  double m_dThreshold = 0.8 ;

	cv::Mat m_template;
  vector<Template> m_Templates ;
  vector<int> m_BusyIndexes ;
  vector<int> m_ActiveIndexes ;
  vector<int> m_NewActiveIndexes ;
  FXLockObject m_DataUpdateLock ;
  int         m_iMaxIndex = -1 ;
	cv::Rect m_TemplateRect;
  CRect m_FutureROI_Rect;
  FXString m_sActivePatternsAsText ;
  FXString m_sPartID ;
  FXString m_sPO ;
  FXString m_sDataDir ;
  FXString m_sPatternsDir ;
  int      m_iMeasurementExposure_us ;
  double  m_dSizeRangePercent = 15. ;
  double m_dAngleRange_deg = 7.0 ;
  int    m_iCurrentPattern = 0 ;
  int    m_iLastPattern = 0;
  FXString m_sPatternName ;


	CDPoint m_ROI_Origin;

	bool m_bselect_ROI;

	double m_dFirstMatchCoeff;

	//CDPoint cdCenterPoint;

	FXLockObject m_Lock;
	void PropertiesRegistration();
	void ConnectorsRegistration();
  static void ConfigParamChange(
    LPCTSTR pName , void* pObject , bool& bInvalidate , bool& bInitRescan );

	CDataFrame* DoProcessing(const CDataFrame* pDataFrame);	


  CContainerFrame* m_pContainer;
  void AsyncTransaction(CDuplexConnector* pConnector, CDataFrame* pParamFrame);
  cv::Mat ConvertVideoFrameToCvMat(const CVideoFrame* pVideoFrame);
  CVideoFrame* ConvertCvMatToVideoFrame( const cv::Mat cvImage) ;
  CDPoint  TrackKeyPoints(cv::Mat &img, const cv::Mat &templ, double &dMatch);
  CDPoint  GetNextCenter(CDPoint pt, double time);
  bool UpdateTemplateRect( cv::Mat Img , CRect& rect );
  bool CreateAndSaveTemplate( cv::Mat Img , CRect& rect ) ;
  void UpdateTemplateROI(cv::Mat Img, CDPoint center);

  size_t LoadTemplates() ;
  cv::Rect GetRect(CDPoint center, double rad);
  cv::Mat CropROI( const cv::Mat& img , CDPoint center , double rad , double sizeCoeff );
  cv::Mat CropROI( const cv::Mat& img , CRect rect );
  cv::Mat GetDefaultRect(const cv::Mat &img, CDPoint center, double rad, double sizeCoeff);
  CDPoint TrackTemplateMatching(cv::Mat &img, const cv::Mat &templ, double &dMatch, bool bSearchAllImage = false);
  cv::Mat GetEdges(const cv::Mat &img);
  DECLARE_RUNTIME_GADGET(TrackerGadget);
};

#endif	// __INCLUDE__UserExampleGadget_H__

