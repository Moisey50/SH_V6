// DistortionCorrectorGadget.h : Declaration of the DistortionCorrectorGadget class
#ifndef __INCLUDE__DistortionCorrectorGadgetGadget_H__
#define __INCLUDE__DistortionCorrectorGadgetGadget_H__


#pragma once
#include <vector>
#include <list>
#include <opencv2/opencv.hpp>
#include <opencv2/imgproc.hpp>
#include "helpers\UserBaseGadget.h"
#include <imageproc\VFrameEmbedInfo.h>

using namespace std;

enum RGB_STATE
{
	None = -1,
	R = 1,
	G = R<<1,
	B = G<<1
};
enum VIEW_STATE
{
  NONE=0,
  SNAPSHOT = 1,
  LIVE = 2
};
typedef struct tagTablePt
{
	int i;
	int j;
	double    dPredictedY;
	double    dPredictedX;
	double    dRealY;
	double    dRealX;
	double    shiftX;
	double    shiftY;
}TablePt;
typedef struct tagCorrTableSingleWave
{
	RGB_STATE Wave;
  FXString Name;
  FXString RawDataPath;
	double Column[13];
	double Row[13];
	FXArray <TablePt> Table;
  cv::Mat DistCoffs;
  
  //vector<double> EquationsVert[13];
	//vector<double> EquationsHoriz[13];

  void Destroy()
  {
    Table.RemoveAll();
    DistCoffs.release();
    Wave = RGB_STATE::None;
    Name = "";
    RawDataPath = "";
    //for each (vector<double> equats in EquationsVert)
    //  equats.clear();
    //for each (vector<double> equats in EquationsHoriz)
    //  equats.clear();
  }
}CorrTableSingleWave;

#define THIS_MODULENAME              ("DistortionCorrectorGadget")

//#define FULL_ROI_LIMIT               (10)

#define PARAM_NAME_CAMERA_PIX_PER_UM ("CameraPixToUm")
//#define PARAM_NAME_ROWS              ("Rows")
//#define PARAM_NAME_COLUMNS           ("Columns")
#define PARAM_NAME_ROWS_COLUMNS_LIMIT          ("RowsColumnsLimit")
#define PARAM_NAME_ROI_WIDTH_PIX     ("Width_pix")
#define PARAM_NAME_ROI_HEIGHT_PIX    ("Height_pix")

#define PARAM_NAME_CORR_PATH_RED     ("CorrectionPathRed")
#define PARAM_NAME_CORR_PATH_GREEN   ("CorrectionPathGreen")
#define PARAM_NAME_CORR_PATH_BLUE    ("CorrectionPathBlue")

#define PARAM_NAME_CROP_IMAGE    ("ToCrop")
                                     
#define DEF_ROI_WIDTH_PIX            (1920)
#define DEF_ROI_HEIGHT_PIX           (1200)
#define DEF_CAM_SCALE                (1)

class DistortionCorrectorGadget : public UserBaseGadget
{
  list<FXString> m_nonCorrTblsPropNames;
  list<FXString> m_corrTblsPropNames;

  static void PropertyParamChanged(LPCTSTR pName, void* pSource, 
    bool& bInvalidate , bool& bRescanParameters );

  void UploadAllCorrectionTable();
  void UploadCorrectionTable(LPCTSTR pPropertyName);
protected:

	DistortionCorrectorGadget();
	~DistortionCorrectorGadget();

	FXString m_sCorrTablePath_Red; 
	FXString m_sCorrTablePath_Blue;
	FXString m_sCorrTablePath_Green;
	double m_dCamScale;
	//int m_iRows;
	//int m_iColumns;
  int m_iRoiLimit;
	int m_iWidth;
	int m_iHeight;

	cv::Mat m_intrinsic;
  int m_iFramesCounter;


	CorrTableSingleWave m_corTbl_Red;
	CorrTableSingleWave m_corTbl_Blue;
	CorrTableSingleWave m_corTbl_Green;

  int m_iToCrop;

  VIEW_STATE m_ViewState;

	int ReadData(const FXString& sPath, __out FXArray <TablePt>& waveTbl);
	int BuiltCorrectionMatrix(const FXArray <TablePt> &Table, cv::Mat &DistCoeffs);
	cv::Mat ConvertVideoFrameToCvMat(const CVideoFrame* pVideoFrame);
	CVideoFrame* ConvertCVMatToVideoFrame(const cv::Mat & imageMat);
public:

	//	Mandatory functions

	void PropertiesRegistration();
	void ConnectorsRegistration();

	CDataFrame* DoProcessing(const CDataFrame* pDataFrame);


	DECLARE_RUNTIME_GADGET(DistortionCorrectorGadget);
};

#endif	// __INCLUDE__UserExampleGadget_H__

