// UserExampleGadget.h.h : Implementation of the UserExampleGadget class


#include "StdAfx.h"
#include "DistortionCorrectorGadget.h"

USER_FILTER_RUNTIME_GADGET(DistortionCorrectorGadget, LINEAGE_FILEX );	//	Mandatory

DistortionCorrectorGadget::DistortionCorrectorGadget()
  : UserBaseGadget()
  , m_sCorrTablePath_Red()
  , m_sCorrTablePath_Blue()
  , m_sCorrTablePath_Green()
  , m_dCamScale(DEF_CAM_SCALE)
  //, m_iRows()
  //, m_iColumns()
  , m_iRoiLimit(10)
  , m_iWidth(DEF_ROI_WIDTH_PIX)
  , m_iHeight(DEF_ROI_HEIGHT_PIX)
  , m_intrinsic(3, 3, CV_64F)
  , m_iFramesCounter(-1)
  , m_corTbl_Red()
  , m_corTbl_Blue()
  , m_corTbl_Green()
  , m_iToCrop(0)
  , m_ViewState(VIEW_STATE::LIVE)
{
  init();

  m_nonCorrTblsPropNames =
  {
    PARAM_NAME_CAMERA_PIX_PER_UM
    //, PARAM_NAME_ROWS
   // , PARAM_NAME_COLUMNS
    ,PARAM_NAME_CROP_IMAGE
    ,PARAM_NAME_ROWS_COLUMNS_LIMIT
    , PARAM_NAME_ROI_WIDTH_PIX
    , PARAM_NAME_ROI_HEIGHT_PIX
  };

  m_corrTblsPropNames =
  {
    PARAM_NAME_CORR_PATH_RED
    , PARAM_NAME_CORR_PATH_GREEN
    , PARAM_NAME_CORR_PATH_BLUE
  };
}
CVideoFrame* DistortionCorrectorGadget::ConvertCVMatToVideoFrame(const cv::Mat & imageMat)
{
	DWORD dwImageSize = m_iWidth * m_iHeight;
	CVideoFrame * pvf = CVideoFrame::Create();
	if (pvf && pvf->lpBMIH==NULL)
	{
		pvf->lpBMIH = (BITMAPINFOHEADER*)malloc(sizeof(BITMAPINFOHEADER) + dwImageSize);
		pvf->lpData = NULL;
		pvf->lpBMIH->biSize = sizeof(BITMAPINFOHEADER);
		pvf->lpBMIH->biWidth = m_iWidth;
		pvf->lpBMIH->biHeight = m_iHeight;
		pvf->lpBMIH->biSizeImage = dwImageSize;
		pvf->lpBMIH->biPlanes = 1;
		pvf->lpBMIH->biBitCount = 8;
		pvf->lpBMIH->biCompression = BI_Y8;
		pvf->lpBMIH->biXPelsPerMeter = 0;
		pvf->lpBMIH->biYPelsPerMeter = 0;
		pvf->lpBMIH->biClrUsed = 0;
		pvf->lpBMIH->biClrImportant = 0;
	}

	memcpy(pvf->lpBMIH + 1, imageMat.data, dwImageSize);
	
  //free(imageMat.data);
  ((cv::Mat &)imageMat).release();


	return pvf;
}
cv::Mat DistortionCorrectorGadget::ConvertVideoFrameToCvMat(const CVideoFrame* pVideoFrame)
{
	size_t memsize  = pVideoFrame->lpBMIH->biWidth * pVideoFrame->lpBMIH->biHeight;
	//if (Img.data == NULL)
//	{
		//m_SourceImgMat = cv::Mat(pVideoFrame->lpBMIH->biHeight, pVideoFrame->lpBMIH->biWidth, CV_16S);
		cv::Mat Img(pVideoFrame->lpBMIH->biHeight, pVideoFrame->lpBMIH->biWidth, CV_16S);//(480, 640, CV_16S);
    Img.flags = 1124024320;////124024336;
    Img.dims = 2;
    Img.step = pVideoFrame->lpBMIH->biWidth;// 640/*1920*/;
    Img.step.buf[1] = 1;
    Img.data = (uchar*)malloc(memsize);
	//}
	memset(Img.data, 0, memsize);
	memcpy(Img.data, pVideoFrame->lpBMIH + 1, memsize);

	//cv::imwrite("e:\\RAWImage.jpg", Img);
	return Img;
}
DistortionCorrectorGadget::~DistortionCorrectorGadget()
{
	m_intrinsic.release();

  m_corTbl_Red.Destroy();
  m_corTbl_Blue.Destroy();
  m_corTbl_Green.Destroy();

  SetChangeNotification(PARAM_NAME_CAMERA_PIX_PER_UM, NULL, NULL);
  //SetChangeNotification(PARAM_NAME_COLUMNS, NULL, NULL);
  SetChangeNotification(PARAM_NAME_ROI_HEIGHT_PIX, NULL, NULL);
  SetChangeNotification(PARAM_NAME_ROI_WIDTH_PIX, NULL, NULL);
  //SetChangeNotification(PARAM_NAME_ROWS, NULL, NULL);

  SetChangeNotification(PARAM_NAME_ROWS_COLUMNS_LIMIT, NULL, NULL);
  SetChangeNotification(PARAM_NAME_CROP_IMAGE, NULL, NULL);

  SetChangeNotification(PARAM_NAME_CORR_PATH_RED, NULL, NULL);
  SetChangeNotification(PARAM_NAME_CORR_PATH_GREEN, NULL, NULL);
  SetChangeNotification(PARAM_NAME_CORR_PATH_BLUE, NULL, NULL);

  m_nonCorrTblsPropNames.clear();
  m_corrTblsPropNames.clear();
}
CDataFrame* DistortionCorrectorGadget::DoProcessing(const CDataFrame* pDataFrame)
{
  if ((pDataFrame) && (!Tvdb400_IsEOS(pDataFrame)))
  {
    FXString Label = pDataFrame->GetLabel();
    if (Label.Find(_T("StatusSnapshot")) == 0)
    {
      m_ViewState = SNAPSHOT;
      m_iFramesCounter = 0;
      return NULL;
    }
    else if (Label.Find("StatusVideo") == 0)
    {
      m_ViewState = LIVE;
      return NULL;
    }

    if (m_ViewState == SNAPSHOT && m_iFramesCounter == 3)
      return NULL;

    if (m_ViewState == LIVE && ((CDataFrame*)pDataFrame)->AddRef() > 0)
    {
      return (CDataFrame*)pDataFrame;
    }

    const CVideoFrame* VideoFrame = pDataFrame->GetVideoFrame(DEFAULT_LABEL);
    if (!VideoFrame)
      return NULL;

    m_iFramesCounter++;

    PVFEI pEmbedInfo = (PVFEI)GetData(VideoFrame);
    cv::Mat imageUndistorted;
    cv::Mat sourceImage = ConvertVideoFrameToCvMat(VideoFrame);

    CorrTableSingleWave *pCorTbl = NULL;
    switch (m_iFramesCounter)
    {
    case 1:
      pCorTbl = &m_corTbl_Red;
      pEmbedInfo->m_ColorSign = 'R';
      break;
    case 2:
      pCorTbl = &m_corTbl_Blue;
      pEmbedInfo->m_ColorSign = 'B';
      break;
    case 3:
      pCorTbl = &m_corTbl_Green;
      pEmbedInfo->m_ColorSign = 'G';
      break;
    default:
      break;
    }

    if(pCorTbl)
      undistort(sourceImage, imageUndistorted, m_intrinsic, pCorTbl->DistCoffs);

    //20190407_YuriS_Origanal version RGB
    //if (m_iFramesCounter == 1)
    //{
    //  undistort(sourceImage, imageUndistorted, m_intrinsic, m_corTbl_Red.DistCoffs);
    //  pEmbedInfo->m_ColorSign = 'R';
    //}
    //else if (m_iFramesCounter == 2)
    //{
    //  undistort(sourceImage, imageUndistorted, m_intrinsic, m_corTbl_Green.DistCoffs);
    //  pEmbedInfo->m_ColorSign = 'G';
    //}
    //else if (m_iFramesCounter == 3)
    //{
    //  undistort(sourceImage, imageUndistorted, m_intrinsic, m_corTbl_Blue.DistCoffs);
    //  pEmbedInfo->m_ColorSign = 'B';
    //}
    //20190407_YuriS_Origanal version RGB


    //CString path1;
    //path1.Format("d:\\before%d_.jpg",m_iFramesCounter);
    //cv::imwrite(path1.GetBuffer(), imageUndistorted);
    //free(sourceImage.data);
    //sourceImage.release();			

    //Crop Circle
    if (m_iToCrop == 1)
    {
      cv::Mat im2(imageUndistorted.rows, imageUndistorted.cols, CV_8UC1, cv::Scalar(0, 0, 0));
      cv::circle(im2, cv::Point(int(m_iWidth / 2), 
        int(m_iHeight / 2)), int(m_iHeight / 2), cv::Scalar(255, 255, 255), cv::FILLED);
      //cv::Mat res;
      bitwise_and(imageUndistorted, im2, imageUndistorted);
    }
  
    CVideoFrame * pvf = ConvertCVMatToVideoFrame(imageUndistorted);
    memcpy(GetData(pvf), pEmbedInfo, sizeof(VideoFrameEmbeddedInfo));
    
    free(sourceImage.data);
    sourceImage.release();

    free(imageUndistorted.data);
    imageUndistorted.release();

    return pvf;
    //m_pOutput->Put(pvf);
    //return NULL;
  }
  return NULL;
}
void DistortionCorrectorGadget::UploadAllCorrectionTable()
{
  for each (FXString paramName in m_corrTblsPropNames)
  {
    UploadCorrectionTable(paramName);
  }
}
void DistortionCorrectorGadget::UploadCorrectionTable(LPCTSTR pPropertyName)
{
  FXString msg("Correction table for the '%s' color is%s loaded from the '%s' file.");
  CorrTableSingleWave* pTbl = NULL;
  bool isLoaded=false;

  if (strcmp(PARAM_NAME_CORR_PATH_RED, pPropertyName)==0 && ReadData(m_sCorrTablePath_Red, m_corTbl_Red.Table) > 0)
  {
    m_corTbl_Red.Wave = RGB_STATE::R;
    isLoaded = BuiltCorrectionMatrix(m_corTbl_Red.Table, m_corTbl_Red.DistCoffs) == TRUE;
    
    m_corTbl_Red.Name = "Red";
    m_corTbl_Red.RawDataPath = m_sCorrTablePath_Red;
    pTbl = &m_corTbl_Red;
  }
  else if (strcmp(PARAM_NAME_CORR_PATH_GREEN, pPropertyName) == 0 && ReadData(m_sCorrTablePath_Green, m_corTbl_Green.Table) > 0)
  {
    m_corTbl_Green.Wave = RGB_STATE::G;
    isLoaded = BuiltCorrectionMatrix(m_corTbl_Green.Table, m_corTbl_Green.DistCoffs) == TRUE;

    m_corTbl_Green.Name = "Green";
    m_corTbl_Green.RawDataPath = m_sCorrTablePath_Green;
    pTbl = &m_corTbl_Green;
  }
  else if (strcmp(PARAM_NAME_CORR_PATH_BLUE, pPropertyName) == 0 && ReadData(m_sCorrTablePath_Blue, m_corTbl_Blue.Table) > 0)
  {
    m_corTbl_Blue.Wave = RGB_STATE::B;
    isLoaded = BuiltCorrectionMatrix(m_corTbl_Blue.Table, m_corTbl_Blue.DistCoffs) == TRUE;

    m_corTbl_Blue.Name = "Blue";
    m_corTbl_Blue.RawDataPath = m_sCorrTablePath_Blue;
    pTbl = &m_corTbl_Blue;
  }
  else
  {
    SENDERR_1("Can't load correction table for the '%s' property.", pPropertyName);
  }

  if (pTbl) //!colorName.IsEmpty() && !filePath.IsEmpty())
  {
    if (isLoaded)
      SENDINFO(msg, pTbl->Name, "", pTbl->RawDataPath);
    else
      SENDERR(msg, pTbl->Name, " NOT", pTbl->RawDataPath);
  }
}

void DistortionCorrectorGadget::PropertyParamChanged(LPCTSTR pName, void* pSource, 
  bool& bInvalidate , bool& RescanParameters )
{
  bool isNonPathProp = false;

  if ((DistortionCorrectorGadget*)pSource)
  {
    DistortionCorrectorGadget& gdgt = *(DistortionCorrectorGadget*)pSource;
    
    for each (const FXString nonPathPropName in gdgt.m_nonCorrTblsPropNames)
    {
      isNonPathProp = (nonPathPropName.Compare(pName) == 0);
      if (isNonPathProp)
      {
        gdgt.UploadAllCorrectionTable();
        break;
      }
    }

    if (!isNonPathProp)
      gdgt.UploadCorrectionTable(pName);
  }
}
static LPCTSTR pList = _T("False;True;");
void DistortionCorrectorGadget::PropertiesRegistration()
{
  addProperty(SProperty::EDITBOX, PARAM_NAME_CAMERA_PIX_PER_UM, &m_dCamScale, SProperty::Double, 0, 50);
  SetChangeNotification(PARAM_NAME_CAMERA_PIX_PER_UM, PropertyParamChanged, this);
  //addProperty(SProperty::EDITBOX, PARAM_NAME_ROWS, &m_iRows, SProperty::Int);
  //SetChangeNotification(PARAM_NAME_ROWS, PropertyParamChanged, this);
  //addProperty(SProperty::EDITBOX, PARAM_NAME_COLUMNS, &m_iColumns, SProperty::Int);
  //SetChangeNotification(PARAM_NAME_COLUMNS, PropertyParamChanged, this);
  
  addProperty(SProperty::EDITBOX, PARAM_NAME_ROWS_COLUMNS_LIMIT, &m_iRoiLimit, SProperty::Int);
  SetChangeNotification(PARAM_NAME_ROWS_COLUMNS_LIMIT, PropertyParamChanged, this);
  

  addProperty(SProperty::EDITBOX, PARAM_NAME_ROI_WIDTH_PIX, &m_iWidth, SProperty::Int);
  SetChangeNotification(PARAM_NAME_ROI_WIDTH_PIX, PropertyParamChanged, this);
  addProperty(SProperty::EDITBOX, PARAM_NAME_ROI_HEIGHT_PIX, &m_iHeight, SProperty::Int);
  SetChangeNotification(PARAM_NAME_ROI_HEIGHT_PIX, PropertyParamChanged, this);

  addProperty(SProperty::EDITBOX, PARAM_NAME_CORR_PATH_RED, &m_sCorrTablePath_Red, SProperty::String);
  SetChangeNotification(PARAM_NAME_CORR_PATH_RED, PropertyParamChanged, this);
  addProperty(SProperty::EDITBOX, PARAM_NAME_CORR_PATH_GREEN, &m_sCorrTablePath_Green, SProperty::String);
  SetChangeNotification(PARAM_NAME_CORR_PATH_GREEN, PropertyParamChanged, this);
  addProperty(SProperty::EDITBOX, PARAM_NAME_CORR_PATH_BLUE, &m_sCorrTablePath_Blue, SProperty::String);
  SetChangeNotification(PARAM_NAME_CORR_PATH_BLUE, PropertyParamChanged, this);

  addProperty(SProperty::COMBO, PARAM_NAME_CROP_IMAGE, &m_iToCrop, SProperty::Int , pList);
  SetChangeNotification(PARAM_NAME_CROP_IMAGE, PropertyParamChanged, this);
};

void DistortionCorrectorGadget::ConnectorsRegistration()
{
  addInputConnector(transparent, "ImageIn");
  addOutputConnector(transparent, "ImageOut");
  //addDuplexConnector(transparent, transparent, "Control");
};

int DistortionCorrectorGadget::ReadData(const FXString& sPath, __out FXArray <TablePt>& waveTbl)
{
	FILE * pFile = NULL;
	errno_t Err = _tfopen_s(&pFile, (LPCTSTR)sPath, _T("r"));
  FXArray <TablePt>& Table = waveTbl;
	if (Err == 0)
	{
    Table.RemoveAll();

		TCHAR buf[1000];
		FXString fxstr;

		bool bHeaderFound = false;
		TablePt pt;
		while (_fgetts(buf, 1000, pFile))
		{
			fxstr = (buf);
			if (fxstr.Find(_T("i	   j	   X-Field  	   Y-Field  	   R-Field  	  Predicted X	  Predicted Y	   Real X	        Real Y	        Distortion")) > -1)
			{
				bHeaderFound = true;
			}
			else if (bHeaderFound)
			{
				if (fxstr.GetLength() < 40)
					break;
				FXSIZE iTok = 0;
				pt.i = atoi((fxstr.Tokenize(_T("\t"), iTok)));           //Reads field 'i'
				pt.j = atoi((fxstr.Tokenize(_T("\t"), iTok)));           //Reads field 'j'
				(fxstr.Tokenize(_T("\t"), iTok));                        //Skips field 'X-Field'
				(fxstr.Tokenize(_T("\t"), iTok));                        //Skips field 'Y-Field'
				(fxstr.Tokenize(_T("\t"), iTok));                        //Skips field 'Z-Field'
				pt.dPredictedX = atof((fxstr.Tokenize(_T("\t"), iTok))); //Reads field 'Predicted X'
				pt.dPredictedY = atof((fxstr.Tokenize(_T("\t"), iTok))); //Reads field 'Predicted Y'
				pt.dRealX = atof((fxstr.Tokenize(_T("\t"), iTok)));      //Reads field 'Real X'
				pt.dRealY = atof((fxstr.Tokenize(_T("\t"), iTok)));      //Reads field 'Real Y'
				if (abs(pt.i) <= m_iRoiLimit && abs(pt.j) <= m_iRoiLimit) //filter 
				 Table.Add(pt);
			}
		}
		fclose(pFile);
	}
	return (int) Table.GetSize();
}
int DistortionCorrectorGadget::BuiltCorrectionMatrix(const FXArray <TablePt>& Table, cv::Mat& DistCoeffs)
{
  int res = NULL;

  if (m_iHeight > 0 && m_iWidth > 0)
  {
    cv::Size imageSize(m_iWidth/*1920*/, m_iHeight/*1200*/);

    //int numCornersHor = m_iColumns;
    //int numCornersVer = m_iRows;
    //int numSquares = numCornersHor * numCornersVer;
    //if (numSquares > 0)
    //{
      //cv::Size board_sz = cv::Size(numCornersHor, numCornersVer);

      vector<cv::Point2f> corners;
      vector<vector<cv::Point2f>> image_points;

      vector<cv::Point3f> obj;
      vector<vector<cv::Point3f>> object_points;

      for (int i = 0; i < (int) Table.GetSize(); i++)
      {
        const TablePt& tpt = Table[i];

        float fX = (float)(m_iWidth / 2 + /*m_redCorTable.Table[i]*/tpt.dRealX * 1000 / m_dCamScale);
        float fY = (float)(m_iHeight / 2 + /*m_redCorTable.Table[i]*/tpt.dRealY * 1000 / m_dCamScale);
        cv::Point2f ptReal = cv::Point2f(fX, fY);
        corners.push_back(ptReal);

        fX = (float)(m_iWidth / 2 + /*m_redCorTable.Table[i]*/tpt.dPredictedX * 1000 / m_dCamScale);
        fY = (float)(m_iHeight / 2 + /*m_redCorTable.Table[i]*/ tpt.dPredictedY * 1000 / m_dCamScale);
        cv::Point3f ptPredicted = cv::Point3f(fX, fY, 0.0f);
        obj.push_back(ptPredicted);
      }

      image_points.push_back(corners);
      object_points.push_back(obj);

      vector<cv::Mat> rvecs;
      vector<cv::Mat> tvecs;
      
      DistCoeffs.release();

      double d = cv::calibrateCamera(object_points, image_points, imageSize, m_intrinsic, DistCoeffs, rvecs, tvecs);
      res = TRUE;
    //}
  }
	return res;
}