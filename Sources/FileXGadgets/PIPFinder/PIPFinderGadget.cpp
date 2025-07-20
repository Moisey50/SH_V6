// UserExampleGadget.h.h : Implementation of the UserExampleGadget class


#include "StdAfx.h"
#include "PIPFinderGadget.h"
#include "opencv2/xfeatures2d/nonfree.hpp"


USER_FILTER_RUNTIME_GADGET(PIPFinderGadget,"OpenCV");	//	Mandatory
PIPFinderGadget::PIPFinderGadget()
{
	bFullShow = false;
	init();
}


CDataFrame* PIPFinderGadget::DoProcessing(const CDataFrame* pDataFrame) 
{
	const CVideoFrame* VideoFrame = pDataFrame->GetVideoFrame(DEFAULT_LABEL);

	if (!VideoFrame  || !VideoFrame->lpBMIH )
		return NULL;
	
	CContainerFrame* resVal;
	resVal = CContainerFrame::Create();
	CFramesIterator* Iterator = pDataFrame->CreateFramesIterator(vframe);
    const CVideoFrame *pFf = NULL;
	cv::Mat image1;
	cv::Mat image2;
	int iCounter = 0;
	if (Iterator!=NULL)
	{
		pFf = (CVideoFrame*)Iterator->Next(DEFAULT_LABEL);	
		do
		{
			if(pFf==NULL)
				break;
		LPBITMAPINFOHEADER pInputInfoHeader = VideoFrame->lpBMIH;
		int width =  pInputInfoHeader->biWidth;
		int height = pInputInfoHeader->biHeight;
		cv::Mat Tempimage(cv::Size(width, height), CV_8UC1, GetData(pFf), cv::Mat::AUTO_STEP);

		if (iCounter == 0)
			Tempimage.copyTo(image1);
		if (iCounter == 1)
			Tempimage.copyTo(image2);

		iCounter++;
		if (iCounter >=2)
			break;;
		pFf = (CVideoFrame*)Iterator->Next(DEFAULT_LABEL);	
	  }while(pFf != NULL);
	}


	if( image1.data && image2.data )
	{
		CDPoint cdLeft,cdRight;
		
		FindCenterOfMass(image1,image2,cdLeft,cdRight);	
		if (!bFullShow)
		{
			resVal->ChangeId(pDataFrame->GetId());
			resVal->SetTime(pDataFrame->GetTime());
			resVal->SetLabel("Results");
			CFigureFrame* f1=CFigureFrame::Create();
			f1->AddPoint( cdLeft ) ;
			f1->SetLabel("Left");
			CFigureFrame* f2=CFigureFrame::Create();
			f2->AddPoint( cdRight ) ;
			f2->SetLabel("Right");
			resVal->AddFrame(f1);
			resVal->AddFrame(f2);
			return resVal;
		}
		CContainerFrame* firstCont;
		firstCont = CContainerFrame::Create();
	    CContainerFrame* secCont;
		secCont = CContainerFrame::Create();
		
		Iterator = pDataFrame->CreateFramesIterator(vframe);
		if (Iterator!=NULL)
		{	
			do
			{
				pFf = (CVideoFrame*)Iterator->Next(DEFAULT_LABEL);	
				if(pFf==NULL)
					break;

				firstCont->ChangeId(pDataFrame->GetId()+1);
				firstCont->SetTime(pDataFrame->GetTime());
				firstCont->SetLabel("Right camera");

				CFigureFrame* f1=CFigureFrame::Create();
				f1->Attributes()->WriteString( "color" , "0xff0000" ) ;
				f1->AddPoint( cdLeft ) ;

				FXString CoordView ;
				CoordView.Format( "(%6.2f,%6.2f) - %s" ,  cdLeft.x , cdLeft.y , (LPCTSTR)"Center" ) ;
				CTextFrame * ViewText = CTextFrame::Create( CoordView ) ;
				ViewText->Attributes()->WriteInt("x", (int)(cdLeft.x));
				ViewText->Attributes()->WriteInt("y", (int)(cdLeft.y ));
				ViewText->Attributes()->WriteString("color", "0xff0000" );
				ViewText->Attributes()->WriteInt("Sz" , 12 ) ;
				ViewText->SetLabel("center right");
				ViewText->ChangeId(pDataFrame->GetId()+1) ;
				ViewText->SetTime(pDataFrame->GetTime()) ;
				firstCont->AddFrame( ViewText ); 

				f1->ChangeId(pDataFrame->GetId()+1);
				firstCont->AddFrame(f1); 
				firstCont->AddFrame(pFf);

				pFf = (CVideoFrame*)Iterator->Next(DEFAULT_LABEL);	
				if(pFf==NULL)
					break;

				secCont->ChangeId(pDataFrame->GetId()+2);
				secCont->SetTime(pDataFrame->GetTime()+2);
				secCont->SetLabel("LeftCamera");

				CFigureFrame* f2=CFigureFrame::Create();
				f2->Attributes()->WriteString( "color" , "0xff0000" ) ;
				f2->AddPoint( cdRight ) ;

				CoordView.Format( "(%6.2f,%6.2f) - %s" ,  cdRight.x , cdRight.y , (LPCTSTR)"Center" ) ;
			    ViewText = CTextFrame::Create( CoordView ) ;
				ViewText->Attributes()->WriteInt("x", (int)(cdRight.x));
				ViewText->Attributes()->WriteInt("y", (int)(cdRight.y ));
				ViewText->Attributes()->WriteString("color", "0xff0000" );
				ViewText->Attributes()->WriteInt("Sz" , 12 ) ;
				ViewText->SetLabel("center left");
				ViewText->ChangeId(pDataFrame->GetId()+2) ;
				ViewText->SetTime(pDataFrame->GetTime()) ;
				secCont->AddFrame( ViewText ); 

				f2->ChangeId(pDataFrame->GetId()+2);
				secCont->AddFrame(f2); 
				secCont->AddFrame(pFf);
        		break;

			}while(pFf != NULL);
		}

		resVal->AddFrame(firstCont);
		resVal->AddFrame(secCont);
		return resVal;
	}

	CVideoFrame* retV = NULL;
	return retV;
}


void PIPFinderGadget::AsyncTransaction(CDuplexConnector* pConnector, CDataFrame* pParamFrame) 
{
	if (!pParamFrame)
		return;
	CTextFrame * ParamText = pParamFrame->GetTextFrame(DEFAULT_LABEL);
  if ( ParamText )
  {

  }
  pParamFrame->Release( pParamFrame );
};


void PIPFinderGadget::PropertiesRegistration() 
{
 	addProperty(SProperty::EDITBOX		,	_T("FullShow")		,	&bFullShow		,	SProperty::Bool		);
};



void PIPFinderGadget::ConnectorsRegistration() 
{
 	addInputConnector( transparent, "Input");
 	addOutputConnector( transparent , "Coordinates");
};

void PIPFinderGadget::FindCenterOfMass(cv::Mat &img_1, cv::Mat &img_2,CDPoint &cdLeft, CDPoint &cdRight)
{
	using namespace cv;
	//-- Step 1: Detect the keypoints using SURF Detector
	int minHessian = 400;

  xfeatures2d::SurfFeatureDetector detector( minHessian );
	std::vector<KeyPoint> keypoints_1, keypoints_2;
	detector.detect( img_1, keypoints_1 );
	detector.detect( img_2, keypoints_2 );

	if (keypoints_1.size() == 0 || keypoints_2.size()==0)
		return;

	//-- Step 2: Calculate descriptors (feature vectors)
  xfeatures2d::SurfDescriptorExtractor extractor;

	Mat descriptors_1, descriptors_2;
	extractor.compute( img_1, keypoints_1, descriptors_1 );
	extractor.compute( img_2, keypoints_2, descriptors_2 );

	//-- Step 3: Matching descriptor vectors using FLANN matcher
	FlannBasedMatcher matcher;
	std::vector< DMatch > matches;
	matcher.match( descriptors_1, descriptors_2, matches );

	double max_dist = 0; double min_dist = 100;
	//-- Quick calculation of max and min distances between keypoints
	for( int i = 0; i < descriptors_1.rows; i++ )
	{
		double dist = matches[i].distance;
	   if( dist < min_dist ) min_dist = dist;
	   if( dist > max_dist ) max_dist = dist;
	}

	std::vector< DMatch > good_matches;
	for( int i = 0; i < descriptors_1.rows; i++ )
	{ 
		if( matches[i].distance <= max(2*min_dist, 0.02) )
			good_matches.push_back( matches[i]); 
	}

	vector<Point> contours01;
	vector<Point> contours02;
	std::vector<cv::KeyPoint>::iterator it;
	for( int i = 0; i < (int)good_matches.size(); i++ )
	{
		int j =  good_matches[i].queryIdx;
		int k =  good_matches[i].trainIdx;
		contours01.push_back(keypoints_1[j].pt);
		contours02.push_back(keypoints_2[k].pt);
	}

	vector<vector<Point> > contours1;
	vector<vector<Point> > contours2;
	contours1.push_back(contours01);
	contours2.push_back(contours02);

	vector<Moments> mu1(contours1.size() );
	for( int j = 0; j < (int)contours1.size(); j++ )
		mu1[j] = moments( contours1[j], false );

	vector<Moments> mu2(contours2.size() );
	for( int j = 0; j < (int)contours2.size(); j++ )
		mu2[j] = moments( contours2[j], false );

	std::vector<Point>::iterator it1;
	float dsum1x = 0,dsum1Y = 0;
	for( it1= contours01.begin(); it1!= contours01.end();it1++)
	{
		dsum1x += it1->x;
		dsum1Y += it1->y;
	}
	vector<Point2f> mc( 2 );
	mc[0] = Point2f(dsum1x/(int)good_matches.size(),dsum1Y/(int)good_matches.size());

	float dsum2x = 0,dsum2Y = 0;
	for( it1= contours02.begin(); it1!= contours02.end();it1++)
	{
		dsum2x += it1->x;
		dsum2Y += it1->y;
	}
	//vector<Point2f> mc2( contours1.size() );
	mc[1] = Point2f(dsum2x/(int)good_matches.size(),dsum2Y/(int)good_matches.size());

	cdLeft.x = mc[0].x; cdLeft.y = mc[0].y;
	cdRight.x = mc[1].x; cdRight.y = mc[1].y;

	
    keypoints_1.clear();
	keypoints_2.clear();
	good_matches.clear();
	matches.clear();
	contours1.clear();
	contours1.clear();
	mu1.clear();
	mu2.clear();
  
	img_1.release();
	img_2.release();
}
