// UserExampleGadget.h.h : Implementation of the UserExampleGadget class


#include "StdAfx.h"
#include "StereoCameraGadget.h"


USER_FILTER_RUNTIME_GADGET(StereoCameraGadget,"OpenCV");	//	Mandatory

void CALLBACK OnCalibration(CDataFrame* lpData, void* lpParam, CConnector* lpInput)
{
	((StereoCameraGadget*)lpParam)->Calibrate(lpData);
}
void CALLBACK OnImageCalibration(CDataFrame* lpData, void* lpParam, CConnector* lpInput)
{
	((StereoCameraGadget*)lpParam)->CalibrateChess(lpData);
}

StereoCameraGadget::StereoCameraGadget()
{
	nx = 5;
	ny = 4;
	dStepSize = 3;
	m_pContainer = NULL;
	imageSize.width  = 640;
	imageSize.height = 480;
	m_pOutput = new COutputConnector( text );	

	Q  = NULL;
	mx1 = NULL;
	my1 = NULL;
	mx2 = NULL;
	my2 = NULL;
	init();
}

CDataFrame* StereoCameraGadget::CalibrateChess(const CDataFrame* pDataFrame)
{
	const CVideoFrame* VideoFrame = pDataFrame->GetVideoFrame(DEFAULT_LABEL);

	if (!VideoFrame  || !VideoFrame->lpBMIH || VideoFrame->GetTime()<0)
		return NULL;

	CContainerFrame* resVal;
	resVal = CContainerFrame::Create();
	CFramesIterator* Iterator = pDataFrame->CreateFramesIterator(vframe);
	const CVideoFrame *pFf = NULL;

    points[0].clear();
	points[1].clear();
	objectPoints.clear();
	active[0].clear();
	active[1].clear();
	MatArray.clear();

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
			SingleImg tmp;
			tmp.sFileName = pFf->GetLabel();
			Tempimage.copyTo(tmp.TempImg);
			MatArray.push_back(tmp);
			pFf = (CVideoFrame*)Iterator->Next(DEFAULT_LABEL);	
		}while(pFf != NULL);
	}

	if( MatArray.size() > 0 )
	{
		const int maxScale = 1;
		int i, j, lr, /*nframes,*/ n = nx*ny, N = 0;
		std::vector<FXString> imageNames[2];
		//std::vector<CvPoint3D32f> objectPoints;
		//std::vector<CvPoint2D32f> points[2];
		std::vector<int> npoints;
		//std::vector<uchar> active[2];
		std::vector<CvPoint2D32f> temp(n);
		CvSize imageSize = {0,0};

		double M1[3][3], M2[3][3], D1[5], D2[5];
		double R[3][3], T[3], E[3][3], F[3][3];
		double Q[4][4];
		CvMat _M1 = cvMat(3, 3, CV_64F, M1 );
		CvMat _M2 = cvMat(3, 3, CV_64F, M2 );
		CvMat _D1 = cvMat(1, 5, CV_64F, D1 );
		CvMat _D2 = cvMat(1, 5, CV_64F, D2 );
		CvMat _R = cvMat(3, 3, CV_64F, R );
		CvMat _T = cvMat(3, 1, CV_64F, T );
		CvMat _E = cvMat(3, 3, CV_64F, E );
		CvMat _F = cvMat(3, 3, CV_64F, F );
		CvMat _Q = cvMat(4,4, CV_64F, Q);

		std::vector<SingleImg>::iterator it;
	    i=0;
		for( it = MatArray.begin(); it!= MatArray.end();it++)
		{
			int count = 0, result=0;
			if (it->sFileName.Find("Left")>-1)
				lr = 0;
			else if(it->sFileName.Find("Right")>-1)
				lr = 1;
				
			//lr = i % 2;
			std::vector<CvPoint2D32f>& pts = points[lr];
			i++;			
			IplImage p;
			p = it->TempImg.operator IplImage();
			IplImage * img = cvCloneImage(&p);

			imageSize = cvGetSize(&p);
			imageNames[lr].push_back(it->sFileName);
			for( int s = 1; s <= maxScale; s++ )
			{
				IplImage* timg = img;
				if( s > 1 )
				{
					timg = cvCreateImage(cvSize(img->width*s,img->height*s),
						img->depth, img->nChannels );
					cvResize( img, timg, CV_INTER_CUBIC );
				}
				result = cvFindChessboardCorners( timg, cvSize(nx, ny),
					&temp[0], &count,
					CV_CALIB_CB_ADAPTIVE_THRESH |
					CV_CALIB_CB_NORMALIZE_IMAGE);
				if( timg != img )
					cvReleaseImage( &timg );
				if( result || s == maxScale )
					for( j = 0; j < count; j++ )
					{
						temp[j].x /= s;
						temp[j].y /= s;
					}
					if( result )
						break;
			}
			N = (int) pts.size();
			pts.resize(N + n, cvPoint2D32f(0,0));
			active[lr].push_back((uchar)result);
			if( result )
			{
				//Calibration will suffer without subpixel interpolation
				cvFindCornerSubPix( img, &temp[0], count,
					cvSize(11, 11), cvSize(-1,-1),
					cvTermCriteria(CV_TERMCRIT_ITER+CV_TERMCRIT_EPS,
					30, 0.01) );
				copy( temp.begin(), temp.end(), pts.begin() + N );
			}
			cvReleaseImage( &img );

		}
		StereoCalib();
	}
    return (CDataFrame*) pDataFrame;
}
CDataFrame* StereoCameraGadget::Calibrate(const CDataFrame* pDataFrame)
{
	int lr=-1;
	FXString fxLabel = pDataFrame->GetLabel();
	if(fxLabel.Find("Right")>-1)
	 lr = 1;
	else if(fxLabel.Find("Left")>-1)
	 lr = 0;
   
	const CTextFrame* TextFrame = pDataFrame->GetTextFrame(DEFAULT_LABEL);
	FXString fxString = TextFrame->GetString();
	int n = nx*ny;  
	std::vector<CvPoint2D32f> temp(n);
	FXSIZE iTok = 0;
	while (iTok != -1)
	{
		FXString fxcoord = fxString.Tokenize(_T(";"),iTok);
		CvPoint2D32f pt;
    double dX, dY;
    sscanf(fxcoord, "%lf%lf", &dX, &dY);
    pt.x = (float)dX;
    pt.y = (float)dY;
		temp.push_back(pt);
	}

	std::vector<CvPoint2D32f>& pts = points[lr];
	FXSIZE N = pts.size();
	pts.resize(N + n, cvPoint2D32f(0,0));
	active[lr].push_back((uchar)1); 
	std::copy( temp.begin(), temp.end(), pts.begin() + N );

    return (CDataFrame*) pDataFrame;
}

CDataFrame* StereoCameraGadget::DoProcessing(const CDataFrame* pDataFrame) 
{
	if (!pDataFrame)
		return NULL;

	if (!Q)
	{
	 LoadCorrectionXML();
	 if (!Q)
	 {
		 CVideoFrame* retV = NULL;
		 return retV;
	 }
	}
	CContainerFrame* resVal;
    CFigureFrame *pFf = NULL;
	resVal = CContainerFrame::Create();
	CDPoint cdLeft,cdRight;
	CFramesIterator* Iterator = pDataFrame->CreateFramesIterator(figure);
	if (Iterator!=NULL)
	{
		do 
		{ 
			pFf = (CFigureFrame*)Iterator->Next(DEFAULT_LABEL);
		    cdLeft = pFf->GetAt(0);
			if(pFf==NULL)
				break;
			pFf = (CFigureFrame*)Iterator->Next(DEFAULT_LABEL);
		    cdRight = pFf->GetAt(0);	
			break;

		}while(pFf != NULL);


		float d = float(cdRight.x - cdLeft.x);
		float X = float(cdLeft.x * cvmGet(Q, 0, 0) + cvmGet(Q, 0, 3));		
		float Y = float(cdLeft.y * cvmGet(Q, 1, 1) + cvmGet(Q, 1, 3));
		float Z = float(cvmGet(Q, 2, 3));
		float W = float(d * cvmGet(Q, 3, 2) + cvmGet(Q, 3, 3));
		
		X = X / W;// 	
		Y = Y / W;// 	
		Z = Z / W;// 	
		 	
		FXString fxOut;
		fxOut.Format("x=%lf,y=%lf,z=%lf,Scaling=%lf",X,Y,Z,W);
		CTextFrame * ViewText = CTextFrame::Create(fxOut); 
		
		resVal->ChangeId(pDataFrame->GetId());
		resVal->SetTime(pDataFrame->GetTime());
		resVal->SetLabel("Results");
		resVal->AddFrame(ViewText);	

		return resVal;
	}
	CVideoFrame* retV = NULL;
	return retV;
}


void StereoCameraGadget::AsyncTransaction(CDuplexConnector* pConnector, CDataFrame* pParamFrame) 
{
	if (!pParamFrame)
		return;

	CTextFrame * ParamText = pParamFrame->GetTextFrame(DEFAULT_LABEL);
	if (ParamText)
  {
    if ( ParamText->GetString().Find( "LoadXML" ) > -1 )
    {
      FXSIZE iTok = 0 ;
      FXString sPath = ParamText->GetString().Tokenize( _T( "-" ) , iTok );
      if ( iTok != -1 )
        sPath = ParamText->GetString().Tokenize( _T( "-" ) , iTok );
      sPath += ("\\");
      sSQLPath = sPath;
      LoadCorrectionXML();
    }
    else if ( ParamText->GetString().Find( "DoCalibration" ) > -1 )
    {
      StereoCalib();
    }
    else if ( ParamText->GetString().Find( "LoadPoints" ) > -1 )
    {
      LoadPointsFromFile();
      //StereoCalib();
    }
  }
  pParamFrame->Release( pParamFrame );
};


void StereoCameraGadget::PropertiesRegistration() 
{
	addProperty(SProperty::SPIN			,	_T("nx")		,	&nx		,	SProperty::Long	,	3		,	15	);
	addProperty(SProperty::SPIN			,	_T("ny")		,	&ny		,	SProperty::Long	,	3		,	15	);
	addProperty(SProperty::SPIN			,	_T("ImageWidth")		,	&imageSize.width 		,	SProperty::Long	,	240		,	2048	);
	addProperty(SProperty::SPIN			,	_T("ImageHeight")		,	&imageSize.height		,	SProperty::Long	,	240		,	2048	);
 	addProperty(SProperty::EDITBOX		,	_T("SquareSize")	,		&dStepSize,	SProperty::Double		);
    addProperty(SProperty::EDITBOX		,	_T("SQLPath")	,			&sSQLPath		,	SProperty::String);
	addProperty(SProperty::EDITBOX		,	_T("CalibPathLeftCam")	,			&sCalibPathLeftCam		,	SProperty::String);
	addProperty(SProperty::EDITBOX		,	_T("CalibPathRightCam")	,			&sCalibPathRightCam		,	SProperty::String);
};



void StereoCameraGadget::ConnectorsRegistration() 
{
	addInputConnector( transparent, "Input");
	//addInputConnector( transparent, "CalibDataInput",OnCalibration,this);
	//addInputConnector( transparent, "CalibImageInput",OnImageCalibration,this);
	addOutputConnector( text , "RealWorldCoordinate");
	addDuplexConnector( transparent, transparent, "Control");
};

void StereoCameraGadget::StereoCalib()
{
	std::vector<int> npoints;
	int n = nx*ny;
	double M1[3][3], M2[3][3], D1[5], D2[5];
	double R[3][3], T[3], E[3][3], F[3][3];
	double dQ[4][4];
	CvMat _M1 = cvMat(3, 3, CV_64F, M1 );
	CvMat _M2 = cvMat(3, 3, CV_64F, M2 );
	CvMat _D1 = cvMat(1, 5, CV_64F, D1 );
	CvMat _D2 = cvMat(1, 5, CV_64F, D2 );
	CvMat _R = cvMat(3, 3, CV_64F, R );
	CvMat _T = cvMat(3, 1, CV_64F, T );
	CvMat _E = cvMat(3, 3, CV_64F, E );
	CvMat _F = cvMat(3, 3, CV_64F, F );
	CvMat _Q = cvMat(4,4, CV_64F, dQ);

	 int i, j;
	// HARVEST CHESSBOARD 3D OBJECT POINT LIST:
  int nframes = (int) active[0].size();//Number of good chessboads found
	objectPoints.resize(nframes*n);
	for(  i = 0; i < ny; i++ )
		for( j = 0; j < nx; j++ )
			objectPoints[i*nx + j] = cvPoint3D32f(i*dStepSize, j*dStepSize, 0);

	for(  i = 1; i < nframes; i++ )
		std::copy( objectPoints.begin(), objectPoints.begin() + n,objectPoints.begin() + i*n );

    npoints.resize(nframes,n);
    int N = nframes*n;
	CvMat _objectPoints = cvMat(1, N, CV_32FC3, &objectPoints[0] );
	CvMat _imagePoints1 = cvMat(1, N, CV_32FC2, &points[0][0] );
	CvMat _imagePoints2 = cvMat(1, N, CV_32FC2, &points[1][0] );
	CvMat _npoints = cvMat(1, (int) npoints.size(), CV_32S, &npoints[0] );
	cvSetIdentity(&_M1);
	cvSetIdentity(&_M2);
	cvZero(&_D1);
	cvZero(&_D2);

	cvStereoCalibrate( &_objectPoints, &_imagePoints1,
		&_imagePoints2, &_npoints,
		&_M1, &_D1, &_M2, &_D2,
		imageSize, &_R, &_T, &_E, &_F,
		cvTermCriteria(CV_TERMCRIT_ITER+
		CV_TERMCRIT_EPS, 100, 1e-5),
		CV_CALIB_FIX_ASPECT_RATIO +
		CV_CALIB_ZERO_TANGENT_DIST +
		CV_CALIB_SAME_FOCAL_LENGTH );


	std::vector<CvPoint3D32f> lines[2];
	points[0].resize(N);
	points[1].resize(N);
	_imagePoints1 = cvMat(1, N, CV_32FC2, &points[0][0] );
	_imagePoints2 = cvMat(1, N, CV_32FC2, &points[1][0] );
	lines[0].resize(N);
	lines[1].resize(N);
	CvMat _L1 = cvMat(1, N, CV_32FC3, &lines[0][0]);
	CvMat _L2 = cvMat(1, N, CV_32FC3, &lines[1][0]);
	//Always work in undistorted space
	cvUndistortPoints( &_imagePoints1, &_imagePoints1,
		&_M1, &_D1, 0, &_M1 );
	cvUndistortPoints( &_imagePoints2, &_imagePoints2,
		&_M2, &_D2, 0, &_M2 );
	cvComputeCorrespondEpilines( &_imagePoints1, 1, &_F, &_L1 );
	cvComputeCorrespondEpilines( &_imagePoints2, 2, &_F, &_L2 );
	double avgErr = 0;
	for(  i = 0; i < N; i++ )
	{
		double err = fabs(points[0][i].x*lines[1][i].x +
			points[0][i].y*lines[1][i].y + lines[1][i].z)
			+ fabs(points[1][i].x*lines[0][i].x +
			points[1][i].y*lines[0][i].y + lines[0][i].z);
		avgErr += err;
	}

	CvMat* mx1 = cvCreateMat( imageSize.height,
		imageSize.width, CV_32F );
	CvMat* my1 = cvCreateMat( imageSize.height,
		imageSize.width, CV_32F );
	CvMat* mx2 = cvCreateMat( imageSize.height,

		imageSize.width, CV_32F );
	CvMat* my2 = cvCreateMat( imageSize.height,
		imageSize.width, CV_32F );
// 	CvMat* img1r = cvCreateMat( imageSize.height,
// 		imageSize.width, CV_8U );
// 	CvMat* img2r = cvCreateMat( imageSize.height,
// 		imageSize.width, CV_8U );
// 	CvMat* disp = cvCreateMat( imageSize.height,
// 		imageSize.width, CV_16S );
// 	CvMat* vdisp = cvCreateMat( imageSize.height,
// 		imageSize.width, CV_8U );
	
	//CvMat* pair;

	double R1[3][3], R2[3][3], P1[3][4], P2[3][4];
	CvMat _R1 = cvMat(3, 3, CV_64F, R1);
	CvMat _R2 = cvMat(3, 3, CV_64F, R2);

	CvMat _P1 = cvMat(3, 4, CV_64F, P1);
	CvMat _P2 = cvMat(3, 4, CV_64F, P2);
	cvStereoRectify( &_M1, &_M2, &_D1, &_D2, imageSize,
		&_R, &_T,
		&_R1, &_R2, &_P1, &_P2, &_Q,
		0/*CV_CALIB_ZERO_DISPARITY*/ );

	//isVerticalStereo = fabs(P2[1][3]) > fabs(P2[0][3]);
	//Precompute maps for cvRemap()
	
	cvInitUndistortRectifyMap(&_M1,&_D1,&_R1,&_P1,mx1,my1);
	cvInitUndistortRectifyMap(&_M2,&_D2,&_R2,&_P2,mx2,my2);

	cvSave(sSQLPath+"\\M1.xml",&_M1);
	cvSave(sSQLPath+"\\D1.xml",&_D1);
	cvSave(sSQLPath+"\\R1.xml",&_R1);
	cvSave(sSQLPath+"\\P1.xml",&_P1);
	cvSave(sSQLPath+"\\M2.xml",&_M2);
	cvSave(sSQLPath+"\\D2.xml",&_D2);
	cvSave(sSQLPath+"\\R2.xml",&_R2);
	cvSave(sSQLPath+"\\P2.xml",&_P2);
	cvSave(sSQLPath+"\\Q.xml",&_Q);
	cvSave(sSQLPath+"\\mx1.xml",mx1);
	cvSave(sSQLPath+"\\my1.xml",my1);
	cvSave(sSQLPath+"\\mx2.xml",mx2);
	cvSave(sSQLPath+"\\my2.xml",my2);


	//LoadCorrectionXML();
     Q = cvCreateMat( 4, 4, CV_64F);
	 Q = (CvMat* )cvClone(&_Q);
}


void StereoCameraGadget::LoadCorrectionXML()
{
	Q   = (CvMat* )cvLoad(sSQLPath+"\\Q.xml",NULL,NULL,NULL);	
//  	mx1 = (CvMat *)cvLoad(sSQLPath+"\\mx1.xml",NULL,NULL,NULL);
//  	my1 = (CvMat *)cvLoad(sSQLPath+"\\my1.xml",NULL,NULL,NULL);
//  	mx2 = (CvMat *)cvLoad(sSQLPath+"\\mx2.xml",NULL,NULL,NULL);
//  	my2 = (CvMat *)cvLoad(sSQLPath+"\\my2.xml",NULL,NULL,NULL);
}


int StereoCameraGadget::LoadPointsFromFile()
{
	points[0].clear();
	points[1].clear();
	FILE * fw = NULL ;
    errno_t Err = fopen_s( &fw , (LPCTSTR)sCalibPathLeftCam , _T("r") ) ;
	if ( fw )
	{
		bool bSpotDataFound = false;
		bool bStart = false;
		char buf [ 300 ] ;
		while ( fgets ( buf, 300, fw ) )
		{
			FXString fx(buf);		
			if(fx.Find("ImageSize")>-1)
			{
				fx.Trim("ImageSize(");
				sscanf( fx.GetBuffer() , "%d,%d" , &imageSize.width , &imageSize.height ) ;
			}
			else if(fx.Find("MatrixSize")>-1)
			{
				fx.Trim("MatrixSize(");
				sscanf( fx.GetBuffer() , "%d,%d" , &nx , &ny ) ;
			}
			else if (fx.Find("SpotData") > -1)
				bSpotDataFound = true;
			else if(bSpotDataFound && fx.Find("<") > -1)
				bStart = true;
			else if(bStart)
			{
			 FXSIZE iTok = 0;
			 FXString fxTemp = fx.Tokenize("World",iTok);
			 fxTemp.Trim("Img");
			 fxTemp.Trim("()");
			 double d1,d2;
			 sscanf( fxTemp.GetBuffer() , "%lf,%lf" , &d1 , &d2 ) ;
			 CvPoint2D32f d;
			 d.x = float(d1);
			 d.y = float(d2);
			 points[0].push_back(d);
			}
		}
		fclose(fw);
	}

    Err = fopen_s( &fw , (LPCTSTR)sCalibPathRightCam , _T("r") ) ;
	if ( fw )
	{
		bool bSpotDataFound = false;
		bool bStart = false;
		char buf [ 300 ] ;
		while ( fgets ( buf, 300, fw ) )
		{
			FXString fx(buf);		
			if (fx.Find("SpotData") > -1)
				bSpotDataFound = true;
			else if(bSpotDataFound && fx.Find("<") > -1)
				bStart = true;
			else if(bStart)
			{
				FXSIZE iTok = 0;
				FXString fxTemp = fx.Tokenize("World",iTok);
				fxTemp.Trim("Img");
				fxTemp.Trim("()");
				double d1,d2;
				sscanf( fxTemp.GetBuffer() , "%lf,%lf" , &d1 , &d2 ) ;
				CvPoint2D32f d;
				d.x = float(d1);
				d.y = float(d2);
				points[1].push_back(d);
			}
		}
		fclose(fw);
	}

	active[0].push_back((uchar)1);
	return 0;
}
