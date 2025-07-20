// UserExampleGadget.h : Declaration of the UserExampleGadget class



#ifndef __INCLUDE__StereoCameraGadget_H__
#define __INCLUDE__StereoCameraGadget_H__

#pragma once
 //#include "opencv/cv.h"
 //#include "opencv/cxmisc.h"
 #include "opencv/cvaux.h"
 #include "opencv2/core/core.hpp"
 #include "opencv2/highgui/highgui.hpp"
 #include "opencv2/imgproc/imgproc.hpp"


#include <vector>
#include <string>
#include <algorithm>
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#pragma once
#include "helpers\UserBaseGadget.h"
#include "gadgets/ContainerFrame.h"
class StereoCameraGadget : public UserBaseGadget
{
protected:

	StereoCameraGadget();
	 CInputConnector* m_pInputs[2];

	 typedef struct
	 {
		 FXString sFileName;
		 cv::Mat TempImg;
	 } SingleImg ;

private:
	std::vector<CvPoint2D32f> points[2];
	std::vector<CvPoint3D32f> objectPoints;
	std::vector<uchar> active[2];
	std::vector<SingleImg> MatArray;
	CvSize imageSize;
public:

	int			nx,ny;
	double		dStepSize;
	FXString    sSQLPath;
	FXString    sCalibPathLeftCam;
	FXString    sCalibPathRightCam;

	static const char* pList;
	 CContainerFrame* m_pContainer;
	//	Mandatory functions

	void PropertiesRegistration();
	void ConnectorsRegistration();

	CDataFrame* DoProcessing(const CDataFrame* pDataFrame);
	CDataFrame* Calibrate(const CDataFrame* pDataFrame);
	CDataFrame* CalibrateChess(const CDataFrame* pDataFrame);
	

	void AsyncTransaction(CDuplexConnector* pConnector, CDataFrame* pParamFrame);
    void LoadCorrectionXML();
	void StereoCalib( /*std::vector<CvPoint2D32f> (&points)[2], int nx, int ny*/ );
	int LoadPointsFromFile();

	CvMat *Q ;  
	CvMat *mx1;
	CvMat *my1; 
	CvMat *mx2; 
	CvMat *my2; 

	DECLARE_RUNTIME_GADGET(StereoCameraGadget);
};

#endif	// __INCLUDE__UserExampleGadget_H__

