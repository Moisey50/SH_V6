// UserExampleGadget.h.h : Implementation of the UserExampleGadget class


#include "StdAfx.h"
#include <opencv2/imgproc/types_c.h>
#include "math\intf_sup.h"
#include "TrackerGadget.h"

//
//using namespace cv;
IMPLEMENT_RUNTIME_GADGET_EX(TrackerGadget, CFilterGadget, "Matchers", TVDB400_PLUGIN_NAME);
//USER_FILTER_RUNTIME_GADGET(ObjectMatcherGadget,"ObjectMatcher");	//	Mandatory
const char* TrackerGadget::pList = "CV_TM_SQDIFF;CV_TM_SQDIFF_NORMED;CV_TM_CCORR;CV_TM_CCORR_NORMED;CV_TM_CCOEFF;CV_TM_CCOEFF_NORMED;";

TrackerGadget::TrackerGadget()
{
	m_pContainer = NULL;
	m_pOutput = new COutputConnector(vframe * text);

	FXPropertyKit pk;
	FXString text;
	bool Invalidate = false;
	m_bselect_ROI = false;
	m_dRadius = 12;
	m_dTemplateToROICoeff = 1.5;

	m_dFirstMatchCoeff = -1;

	m_prevCenter.x = m_prevCenter.y = -1;

	m_cdTrackPt.x = m_cdTrackPt.y = -1;
	m_dMaxAllowedPattern = 0.99;
	m_dMaxAllowedView = 0.5;
	m_sMatchMethod = "CV_TM_CCORR";

	init();
}

cv::Mat TrackerGadget::ConvertVideoFrameToCvMat(const CVideoFrame* pVideoFrame)
{
	//pVideoFrame->lpBMIH.biWidth;
	//biHeight;


	cv::Mat Img(pVideoFrame->lpBMIH->biHeight,pVideoFrame->lpBMIH->biWidth,  CV_16S);//(480, 640, CV_16S);
	size_t memsize = pVideoFrame->lpBMIH->biWidth * pVideoFrame->lpBMIH->biHeight/*3 **/;// 480 * 640;
	Img.flags = 1124024320;////124024336;
	Img.dims = 2;
	Img.step = pVideoFrame->lpBMIH->biWidth;// 640/*1920*/;
	Img.step.buf[1] = 1;
	Img.data = (uchar*)malloc(memsize);
	memset(Img.data, 0, memsize);
	memcpy(Img.data, pVideoFrame->lpBMIH + 1, memsize);

	//cv::imwrite("e:\\RAWImage.jpg", Img);
	return Img;
}
CDataFrame* TrackerGadget::DoProcessing(const CDataFrame* pDataFrame)
{

	using namespace std;
	const CVideoFrame* VideoFrame = pDataFrame->GetVideoFrame(DEFAULT_LABEL);

	if (!VideoFrame || !VideoFrame->lpBMIH)
		return NULL;

	cv::Mat cvImg = ConvertVideoFrameToCvMat(VideoFrame);
	if (m_bselect_ROI)
	{
		m_bselect_ROI = false;
		cv::Mat tmp = CropROI(cvImg, m_cdTrackPt, m_dRadius, 1);
		tmp.copyTo(m_template);
		tmp.release();
	}

	double dMatchFactor = 0;	
	CDPoint cdCenterPoint;

	 cdCenterPoint = TrackTemplateMatching(cvImg, m_template, dMatchFactor);

	 //double dAnotherMatchactor;
	 //CDPoint pt = TrackKeyPoints(cvImg, m_template, dAnotherMatchactor);

	 m_cdTrackPt = cdCenterPoint;

	//if (m_dFirstMatchCoeff == -1 && dMatchFactor != 0)
	//{
	//	m_dFirstMatchCoeff = dMatchFactor;
	//	dMatchFactor = dMatchFactor / m_dFirstMatchCoeff;
	//}
		
	
	if (dMatchFactor > m_dMaxAllowedPattern)
	{
         cdCenterPoint = GetNextCenter(cdCenterPoint, pDataFrame->GetTime());
		 UpdateTemplateROI(cvImg, cdCenterPoint);
	}



	CContainerFrame* resVal;
	resVal = CContainerFrame::Create();

	resVal->ChangeId(pDataFrame->GetId());
	resVal->SetTime(pDataFrame->GetTime());
	resVal->SetLabel("Results");

	if (dMatchFactor > m_dMaxAllowedView)
	{
		m_prevCenter = cdCenterPoint;//save for image lost case
		
		//Template
		CFigureFrame* ff = CFigureFrame::Create();
		ff->Attributes()->WriteString("color", "0xff0000");
		ff->AddPoint(CDPoint(cdCenterPoint.x - m_dRadius, cdCenterPoint.y - m_dRadius));
		ff->AddPoint(CDPoint(cdCenterPoint.x + m_dRadius, cdCenterPoint.y - m_dRadius));
		ff->AddPoint(CDPoint(cdCenterPoint.x + m_dRadius, cdCenterPoint.y + m_dRadius));
		ff->AddPoint(CDPoint(cdCenterPoint.x - m_dRadius, cdCenterPoint.y + m_dRadius));
		ff->AddPoint(CDPoint(cdCenterPoint.x - m_dRadius, cdCenterPoint.y - m_dRadius));
		ff->ChangeId(pDataFrame->GetId());
		resVal->AddFrame(ff);

		//ROI
		

		ff = CFigureFrame::Create();
		ff->Attributes()->WriteString("color", "0x0000FF");
		ff->AddPoint(CDPoint(cdCenterPoint.x - m_dRadius * m_dTemplateToROICoeff, cdCenterPoint.y - m_dRadius * m_dTemplateToROICoeff));
		ff->AddPoint(CDPoint(cdCenterPoint.x + m_dRadius * m_dTemplateToROICoeff, cdCenterPoint.y - m_dRadius * m_dTemplateToROICoeff));
		ff->AddPoint(CDPoint(cdCenterPoint.x + m_dRadius * m_dTemplateToROICoeff, cdCenterPoint.y + m_dRadius * m_dTemplateToROICoeff));
		ff->AddPoint(CDPoint(cdCenterPoint.x - m_dRadius * m_dTemplateToROICoeff, cdCenterPoint.y + m_dRadius * m_dTemplateToROICoeff));
		ff->AddPoint(CDPoint(cdCenterPoint.x - m_dRadius * m_dTemplateToROICoeff, cdCenterPoint.y - m_dRadius * m_dTemplateToROICoeff));
		ff->ChangeId(pDataFrame->GetId());
		resVal->AddFrame(ff);



		//ff = CFigureFrame::Create();
		//ff->Attributes()->WriteString("color", "0xff00FF");
		//ff->AddPoint(pt);
		//ff->ChangeId(pDataFrame->GetId());
		//resVal->AddFrame(ff);
	}
	else
	{
		//cdCenterPoint = m_prevCenter;		


		cdCenterPoint = TrackTemplateMatching(cvImg, m_template, dMatchFactor,true);
		m_cdTrackPt = cdCenterPoint;
		if (m_dFirstMatchCoeff == -1 && dMatchFactor != 0)
		{
			m_dFirstMatchCoeff = dMatchFactor;
			dMatchFactor = dMatchFactor / m_dFirstMatchCoeff;
		}
		if (dMatchFactor > m_dMaxAllowedPattern)
		{
			cdCenterPoint = GetNextCenter(cdCenterPoint, pDataFrame->GetTime());
			UpdateTemplateROI(cvImg, cdCenterPoint);
		}
	}
		 
	m_cdTrackPt = cdCenterPoint;
	FXString CoordView;
	CoordView.Format("Match Factor : %6.5f", dMatchFactor);
	CTextFrame * ViewText = CTextFrame::Create(CoordView);
	ViewText->ChangeId(pDataFrame->GetId());
	ViewText->SetTime(pDataFrame->GetTime());
	resVal->AddFrame(ViewText);

	//FXString CoordView;
	CoordView.Format("New Coordinate : x=%6.2f,y=%6.2f", cdCenterPoint.x, cdCenterPoint.y);
	ViewText = CTextFrame::Create(CoordView);
	ViewText->ChangeId(pDataFrame->GetId());
	ViewText->SetTime(pDataFrame->GetTime());
	resVal->AddFrame(ViewText);

	free(cvImg.data);
	cvImg.release();

	resVal->AddFrame(VideoFrame);
	m_pOutput->Put(resVal);



	CVideoFrame* retV = NULL;
	return retV;
}
cv::Rect TrackerGadget::GetRect(CDPoint center, double rad)
{
	cv::Rect tmpRect;
	tmpRect.x = (int)(center.x - rad);
	tmpRect.y = (int)(center.y - rad);
	tmpRect.height = (int)(2 * rad);
	tmpRect.width = (int)(2 * rad);

	if (tmpRect.x < 0) tmpRect.x = 0;
	if (tmpRect.y < 0) tmpRect.y = 0;

	return tmpRect;
}
void TrackerGadget::AsyncTransaction(CDuplexConnector* pConnector, CDataFrame* pParamFrame)
{
  if (!pParamFrame)
    return;

  CTextFrame * ParamText = pParamFrame->GetTextFrame(DEFAULT_LABEL);
  if (ParamText)
  {

    TRACE( "%s" , _T( ParamText->GetString() ) );

    if ( ParamText->GetString().Find( "selected" ) == -1 )
      return;

    if ( ParamText->GetString().Find( "x=" ) > -1 && ParamText->GetString().Find( "y=" ) > -1 )
    {
      double x = -1 , y = -1;
      FXSIZE iTok = 0;
      FXString fxData = ParamText->GetString().Tokenize( _T( ";" ) , iTok );
      if ( iTok == -1 )
        return;
      //iTok = 0;
      FXString fxY;
      FXString fxX = ParamText->GetString().Tokenize( _T( ";" ) , iTok );
      if ( iTok != -1 )
        fxY = ParamText->GetString().Tokenize( _T( ";" ) , iTok );

      FXString pt;
      if ( iTok != -1 )
      {
        iTok = 0;
        pt = fxX.Tokenize( _T( "=" ) , iTok );
        if ( iTok != -1 )
          x = atof( fxX.Tokenize( _T( "=" ) , iTok ) );

        iTok = 0;
        pt = fxY.Tokenize( _T( "=" ) , iTok );
        if ( iTok != -1 )
          y = atof( fxY.Tokenize( _T( "=" ) , iTok ) );


        m_cdTrackPt.x = x;
        m_cdTrackPt.y = y;

        m_bselect_ROI = true;
        m_dFirstMatchCoeff = -1;
        m_centersArr.RemoveAll();
        m_prevCenter.x = m_prevCenter.y = -1;
      }
    }
  }
  pParamFrame->Release( pParamFrame );
};

void TrackerGadget::UpdateTemplateROI(cv::Mat Img, CDPoint center)
{
	try
	{
		cv::Mat newTemplate = CropROI(Img, m_cdTrackPt, m_dRadius, 1);
		newTemplate.copyTo(m_template);
	}
  
  catch ( const char * str )
	{
    TRACE( "\n exception in TrackerGadget::UpdateTemplateROI:%s" , str ) ;
		int y = *str ;
	}
	//cv::imwrite("e:\\NewTemplate.jpg", m_template);	
}

void TrackerGadget::PropertiesRegistration()
{	
	addProperty(SProperty::EDITBOX, _T("MaxAllowedForView"), &m_dMaxAllowedView, SProperty::Double, 0.001, 1000000000);
	addProperty(SProperty::EDITBOX, _T("MaxAllowedForPattern"), &m_dMaxAllowedPattern, SProperty::Double, 0.001, 100000000);
	addProperty(SProperty::EDITBOX, _T("Template_pix"), &m_dRadius, SProperty::Double, 5, 400);
	addProperty(SProperty::EDITBOX, _T("ROI_xToTempl"), &m_dTemplateToROICoeff, SProperty::Double, 1, 100);
	//addProperty(SProperty::COMBO, _T("MatchMethod"), &m_sMatchMethod, SProperty::String, pList);
};

void TrackerGadget::ConnectorsRegistration()
{
	addInputConnector( transparent, "Image");
	addOutputConnector( createComplexDataType(3, rectangle, text, vframe) , "Results");
	addDuplexConnector( transparent, transparent, "Control");
};

cv::Mat TrackerGadget::CropROI(const cv::Mat &img ,CDPoint center, double rad ,double sizeCoeff)
{
	cv::Rect newRect = GetRect(center, rad * sizeCoeff);

	if (newRect.x < 4) newRect.x = 4;
	if (newRect.y < 4) newRect.y = 4;
	if (newRect.y + newRect.height > img.rows - 4) newRect.height = img.rows - newRect.y - 4;
	if (newRect.x + newRect.width > img.cols - 4) newRect.width = img.cols - newRect.x - 4;
    cv::Mat newImg;
	if (newRect.height > 4 && newRect.width > 4)
	{
		newImg = img(newRect);
		m_ROI_Origin.x = center.x;
		m_ROI_Origin.y = center.y;
	}

	return newImg;
}
CDPoint  TrackerGadget::TrackTemplateMatching(cv::Mat &img, const cv::Mat &templ, double &dMatch, bool bSearchAllImage)
{
	if (m_template.data == NULL)
		return CDPoint(-1, -1);

	cv::Mat cropedImg;
	if ((m_cdTrackPt.x != -1 && m_cdTrackPt.y != -1) && !bSearchAllImage)
	{
		cropedImg = CropROI(img, m_cdTrackPt, m_dRadius, m_dTemplateToROICoeff);	
	}
	else
	{
		img.copyTo(cropedImg);
	}

	//cv::imwrite("e:\\templ.jpg", templ);
	//cv::imwrite("e:\\cropedImg.jpg", cropedImg);

	//cv::imwrite("e:\\templEdges.jpg", GetEdges(templ));
	//cv::imwrite("e:\\cropedImgEdges.jpg", GetEdges(cropedImg));

	int match_method = CV_TM_CCOEFF_NORMED;

	//switch (atoi(m_sMatchMethod))
	//{
	//	case 0:
	//	{
	//		match_method = CV_TM_SQDIFF;
	//	}
	//	break;
	//	case 1:
	//	{
	//		match_method = CV_TM_SQDIFF_NORMED;
	//	}
	//	break;
	//	case 2:
	//	{
	//		match_method = CV_TM_CCORR;
	//	}
	//	break;
	//	case 3:
	//	{
	//		match_method = CV_TM_CCORR_NORMED;
	//	}
	//	break;
	//	case 4:
	//	{
	//		match_method = CV_TM_CCOEFF;
	//	}
	//	break;
	//	case 5:
	//	{
	//		match_method = CV_TM_CCOEFF_NORMED;
	//	}
	//	break;
	//default:
	//	break;
	//}

	cv::Mat result;	
	matchTemplate(cropedImg, templ, result, match_method);
	//matchTemplate(GetEdges(cropedImg), GetEdges(templ), result, match_method);


	//normalize(result, result, 0, 1, cv::NORM_MINMAX, -1, cv::Mat());
	//normalize(result, result, 0, 1, cv::NORM_HAMMING, -1, cv::Mat());

	double minVal; double maxVal; cv::Point minLoc; cv::Point maxLoc;
	cv::Point matchLoc0;
	minMaxLoc(result, &minVal, &maxVal, &minLoc, &maxLoc, cv::Mat());

	if (match_method == CV_TM_SQDIFF || match_method == CV_TM_SQDIFF_NORMED)
	{
		matchLoc0 = minLoc;
		dMatch = minVal;
	}
	else
	{
		matchLoc0 = maxLoc;
		dMatch = maxVal;
	}

	if (bSearchAllImage)
	{
		double newX = matchLoc0.x + templ.cols / 2;
		double newY = matchLoc0.y + templ.rows / 2;

		m_cdTrackPt.x = newX;
		m_cdTrackPt.y = newY;
	}
	else
	{
		double newX = m_ROI_Origin.x - cropedImg.cols / 2 + (matchLoc0.x + templ.cols / 2);
		double newY = m_ROI_Origin.y - cropedImg.rows / 2 + (matchLoc0.y + templ.rows / 2);

		if (abs(newX - m_cdTrackPt.x) > 2 || abs(newY - m_cdTrackPt.y) > 2)
		{
			m_cdTrackPt.x = newX;
			m_cdTrackPt.y = newY;
		}
	}


    cropedImg.release();
	return m_cdTrackPt;
}
CDPoint  TrackerGadget::GetNextCenter(CDPoint pt, double time)
{
	return pt;
	if (m_prevCenter.x == -1 && m_prevCenter.y == -1)
	{
		m_prevCenter = pt;
		return pt;
	}
	else
	{
      CDPoint newCenter(pt.x + pt.x - m_prevCenter.x, pt.y + pt.y - m_prevCenter.y);
	  m_prevCenter = pt;
	  return newCenter;
	}


}
cv::Mat TrackerGadget::GetEdges(const cv::Mat &img)
{
	/// Canny detector
	cv::Mat detected_edges;
	int edgeThresh = 1;
	int lowThreshold = 5;
	int ratio = 3;
	int kernel_size = 3;
	cv::Canny(img, detected_edges, lowThreshold, lowThreshold*ratio, kernel_size);
	return detected_edges;
}
/*
CDPoint  TrackerGadget::TrackKeyPoints(cv::Mat &img, const cv::Mat &templ, double &dMatch)
{
	if (m_template.data == NULL)
		return CDPoint(-1, -1);

	//int minHessian = 400;
	cv::SiftFeatureDetector   detector;

	cv::Mat cropedImg;
	if (m_cdTrackPt.x != -1 && m_cdTrackPt.y != -1)
	{
		cropedImg = CropROI(img, m_cdTrackPt, m_dRadius, m_dTemplateToROICoeff);
		//cropedImg = CropROI(img, m_dTemplateToROICoeff);
		//cv::imwrite("e:\\templ.jpg", templ);
		//cv::imwrite("e:\\cropedImg.jpg", cropedImg);		
	}
	else
	{
		img.copyTo(cropedImg);
	}

	std::vector<cv::KeyPoint> keypoints_1, keypoints_2;
	detector.detect(cropedImg, keypoints_1);
	detector.detect(templ, keypoints_2);

	if (keypoints_1.size() == 0 || keypoints_2.size() == 0)
		return CDPoint(-1,-1);

	cv::SurfDescriptorExtractor extractor;

	cv::Mat descriptors_1, descriptors_2;
	extractor.compute(cropedImg, keypoints_1, descriptors_1);
	extractor.compute(templ, keypoints_2, descriptors_2);

	cv::FlannBasedMatcher matcher;
	std::vector< cv::DMatch > matches;
	matcher.match(descriptors_1, descriptors_2, matches);

	double max_dist = 0; double min_dist = 100;
	//-- Quick calculation of max and min distances between keypoints
	for (int i = 0; i < descriptors_1.rows; i++)
	{
		double dist = matches[i].distance;
		if (dist < min_dist) min_dist = dist;
		if (dist > max_dist) max_dist = dist;
	}

	std::vector< cv::DMatch > good_matches;
	for (int i = 0; i < descriptors_1.rows; i++)
	{
		if (matches[i].distance <= max(2 * min_dist, 0.02))
			good_matches.push_back(matches[i]);
	}


	vector<cv::Point> contours01;
	vector<cv::Point> contours02;
	std::vector<cv::KeyPoint>::iterator it;
	for (int i = 0; i < (int)good_matches.size(); i++)
	{
		int j = good_matches[i].queryIdx;
		int k = good_matches[i].trainIdx;
		contours01.push_back(keypoints_1[j].pt);
		contours02.push_back(keypoints_2[k].pt);
	}

	vector<vector<cv::Point> > contours1;
	vector<vector<cv::Point> > contours2;
	contours1.push_back(contours01);
	contours2.push_back(contours02);

	vector<cv::Moments> mu1(contours1.size());
	for (int j = 0; j < (int)contours1.size(); j++)
		mu1[j] = moments(contours1[j], false);

	std::vector<cv::Point>::iterator it1;
	float dsum1x = 0, dsum1Y = 0;
	for (it1 = contours01.begin(); it1 != contours01.end(); it1++)
	{
		dsum1x += it1->x;
		dsum1Y += it1->y;
	}
	vector<cv::Point2f> mc(2);
	mc[0] = cv::Point2f(dsum1x / (int)good_matches.size(), dsum1Y / (int)good_matches.size());

	int lx = int(mc[0].x);
	int ly = int(mc[0].y);


	cv::Point matchLoc(-1,-1);
	matchLoc.x = lx;
	matchLoc.y = ly;

	//if (min_dist < m_dMaxAllowedPattern)
	//{
	//	cv::Rect newRect = cv::Rect((int)matchLoc.x - (int)m_dRadius, (int)matchLoc.y - (int)m_dRadius, 2 * (int)m_dRadius, 2 * (int)m_dRadius);
	//	if (newRect.x >= 0 && newRect.y >= 0)
	//	{
	//		cv::Mat newTemplate = img(newRect);
	//		newTemplate.copyTo(m_template);
	//	}
	//}

	keypoints_1.clear();
	keypoints_2.clear();
	good_matches.clear();
	matches.clear();
	contours1.clear();
	contours1.clear();
	//mu1.clear();
	//mu2.clear();

	dMatch = min_dist;

	CDPoint matchPt(-1,-1);
	//if (min_dist < m_dMaxAllowedView)
	{	
		matchPt.x = m_ROI_Origin.x +  matchLoc.x;
		matchPt.y = m_ROI_Origin.y + matchLoc.y;
	}
	return matchPt;
}
*/