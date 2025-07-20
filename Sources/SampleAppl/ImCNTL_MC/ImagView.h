// ImagView.h : interface of the CImageView class
//
/////////////////////////////////////////////////////////////////////////////

#if !defined(AFX_IMAGVIEW_H__5C595B64_DA04_11D2_84CB_00A0C9616FBC__INCLUDED_)
#define AFX_IMAGVIEW_H__5C595B64_DA04_11D2_84CB_00A0C9616FBC__INCLUDED_

#include <habitat.h>
#include "math\intf_sup.h"
#include <fxfc/fxfc.h>
#include "resource.h"
#include "helpers\SharedMemLib.h"
#include "ImagingExch.h"
#include "MainFrm.h"
#include "FImParamDialog.h"	// Added by ClassView
#include "FFTFilter.h"	// Added by ClassView
#include "BigViewWnd.h"
#include "RButton.h"	// Added by ClassView		 
// #include "ProcessThread.h"
#include <imageproc\clusters\Segmentation.h>
#include <imageproc\LineData.h>
#include "cpp_util\DynArray.h"
#include <PlugInLoader.h>
#include <Gadgets\gadbase.h>
// #include <gadgets/BaseGadgets.h>
#include <gadgets/TextFrame.h>
#include "GraphCameraControl.h"
#include "RenderWnd.h"
#include "DebugWnd.h"
#include "HTMLDlg.h"

class ProtectedSpotsData: public SpotArray
{
private:
  CRITICAL_SECTION m_Lock ;
public:
  ProtectedSpotsData() { InitializeCriticalSection(&m_Lock) ; } ;
  virtual ~ProtectedSpotsData() { DeleteCriticalSection(&m_Lock) ;} ;
  void Lock() { EnterCriticalSection(&m_Lock) ; } ;
  void Unlock() { LeaveCriticalSection(&m_Lock) ; } ;
  void RemoveAll() {   Lock() ; SpotArray::RemoveAll() ; Unlock() ;  }
};

class ProtectedProfileData : public CDPointArray
{
private:
  CRITICAL_SECTION m_Lock ;
public:
  ProtectedProfileData() { InitializeCriticalSection(&m_Lock) ; } ;
  virtual ~ProtectedProfileData() { DeleteCriticalSection(&m_Lock) ;} ;
  void Lock() { EnterCriticalSection(&m_Lock) ; } ;
  void Unlock() { LeaveCriticalSection(&m_Lock) ; } ;
  void RemoveAll() {   Lock() ; CDPointArray::RemoveAll() ; Unlock() ;  }
};

class ProtectedLinesData: public CLineResults
{
private:
  CRITICAL_SECTION m_Lock ;
public:
  ProtectedLinesData() { InitializeCriticalSection(&m_Lock) ; } ;
  virtual ~ProtectedLinesData() { DeleteCriticalSection(&m_Lock) ;} ;
  void Lock() { EnterCriticalSection(&m_Lock) ; } ;
  void Unlock() { LeaveCriticalSection(&m_Lock) ; } ;
  void RemoveAll() {   Lock() ; CLineResults::RemoveAll() ; Unlock() ;  }
};

#include "MeasurementSet.h"
#include <habitat.h>


#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#define PROFILE_WIDTH  1024
#define PROFILE_HEIGHT 1024


#define Y_BEGIN_MEAS    ( m_ImParam.m_iFFTYc - 192 )
#define Y_END_MEAS      ( m_ImParam.m_iFFTYc + 192 )
#define X_STRIP_1       ( m_ImParam.m_iFFTXc - 192 )
#define X_STRIP_2       ( m_ImParam.m_iFFTXc + 192 )


#define IMAGE_WIDTH 1024
#define IMAGE_HEIGHT 1024

#define FIND_BAD_PIXELS 10
#define SET_GAIN 11
#define SET_ANALOG_OFFSET 12
#define SET_NORM_THRESH   13
#define SET_FIXED_THRESHOLD     14
#define GET_NEAREST_SPOT_NUMBER 15
#define RESET_ALL_BACKGROUNDS   16
#define IS_GRAPH_BASED_PROCESSING 17
#define SET_DELAY_IN_SCANS      18
#define SET_SCAN_TIME           19
#define SET_NORM_ROTATION_THRESH    20



#define DIGITIZER_MODE_SYNC    1
#define DIGITIZER_MODE_ASYNC   0

#define EVT_MEASUREMENT_CALLBACK (WM_APP)
#define EVT_IMAGE_CALLBACK       (WM_APP + 1)



#define E_VALUE 2.7182818284590452353602874713527
#define PI_VALUE (atan(1.0)*4.0)
#ifndef ROUND
#define ROUND(x) ((int)(x+0.5))
#endif

class CImageView ;


typedef struct tagBadPix
{
  int iX ; 
  int iY  ;
  int iB;
}	BadPix;                    

// typedef CArray<BackImage, BackImage&> Backgrounds ;



class CImageCNTLDoc ;

class CImageView : public CMainFrame
{
protected: // create from serialization only
	DECLARE_DYNCREATE(CImageView)

  CDebugWnd*		m_pDebugWnd;
// Attributes
public:
	CImageView();
  IGraphbuilder* m_pBuilder;
  IPluginLoader* m_pPluginLoader;
  ConnectGraph  m_Graph ;
  MeasSets      m_MeasSets ;
  int           m_iGraphMode ; // 0 - two AVT cameras
                               // 1 - LAN communication with second imaging
                               // 2 - one Dalsa camera with Solios Frame Grabber
                               // 3 - two Dalsa cameras with Solios Frame Grabber

 //bool    m_bFlippedImg;
  BadPix  m_BadPix[50] ;

	int      m_imode ;
	int      m_iOnBlobProc;
	
  int     m_DefGrabBuf ;
	int     m_GrabHeight;
	int     m_GrabWidth;
	int     m_DispPitch;
	int     m_GrabPitch;
  int     m_iImgMode; // = 0 when 512*512, = 1 when 1024*1024
  int     m_iWidthWin; 
  int     m_iHeightWin;
	int     m_iThreshold;
  double  m_dNormThreshold ; // between 0 and 1
  double  m_dRotThreshold;
	WORD *  m_GrabBase;

	double  m_dPrevTime;
	double  m_dTimeInterval;
	double  m_dResizeTime;

    // Exposure Param
  int     m_iMaxExposure_us ;
  int     m_iMinExposure_us ;
  int     m_iMinGain ;
  int     m_iMaxGain ;
  double  m_dGainStep_dB ;

  int     m_BackGrabBuf ;
	WORD *  m_BackGrabBase;

	// Measurement Results
  int m_iMaxBlobs ;
  int m_NBlobs ;
  double * m_dArea ;
  double * m_dCenterX ;
  double * m_dCenterY ;
  double * m_dFeretElong ;
  double * m_dMeanPixel ;
  double * m_dSumPixel ;
  double * m_dMaxPixel ;
  double * m_dAngle ; 
  int * m_iMinX ;
  int * m_iMinY ;
  int * m_iMaxX ;
  int * m_iMaxY ;

  double * m_dBlobWidth ;
  double * m_dBlobHeigth ;

  double * m_dRDiffraction ;
  double * m_dLDiffraction ;
  double * m_dUDiffraction ;
  double * m_dDDiffraction ;
  double * m_dCentralIntegral ;
  //char   * m_cMaxDiffrDirection ; // U,D,L,R; N - undefined

  int m_iMaxLines ;
  int m_NLines ;
  int m_NLeftLines ;
  int m_NRightLines ;
  //double * m_dLinePos ;
  double  m_dLinePos[2] ;
  double * m_dLineWidth ;
  double * m_dLineAngle ;
  double * m_dLineLeftPos ;
  double * m_dLineRightPos ;
  double * m_dLineLeftWidth ;
  double * m_dLineRightWidth ;
	double * m_dSumPower;

  double m_dLastCx , m_dLastCy ;

	int m_nCurBoard;
	BOOL m_bOpenBoard;
	BOOL m_bOnGrab;
	BOOL m_bOnDMA;
  BOOL m_bPlayBackMode ;
	WORD * m_pUserBuff, * m_pShortUsBuff;
	WORD * m_pSaveBuff;
	ULONG m_BuffSize, m_ShortBufSize;
	int m_nPixelSize;
	int m_nDibColorDepth;
	BOOL m_bDMACtrlCreated;
  SIZE m_SurfaceSize;			
	SIZE m_CaptureSize;
	int  m_iShortWidth;
  BOOL m_bPaintOK,m_bDisp;
  CSharedMemLib *   m_pSharedMemory ;

	BOOL m_bBigGrab;
	BOOL m_bEndLine;

  CColorSpot m_LastSpot;
  CLineResults m_MeasuredLines;
  SpotArray    m_MeasuredBlobs;
  
  double  dAngles[2];
  

	ULONG   m_nEntries;
	BYTE  m_nBankAvail,  m_nTapAvail,  m_nByte;


	PowerImagingExch  *   m_pCommon ;  // common variable of shared memory

  SpotArray 	m_ColSpots;

 SegmentArray   * m_pCurr,* m_pPrev,m_Segm1,m_Segm2;

 int m_byThres;																						
 int m_iVertThres ;
 int m_iHorThres ;

 bool FirstFileReady;
 bool SecondfileReady;
	

	
 int m_iCam;
 int m_WindisAlive;
 double cSpotX,cSpotY;
	
 int m_iSavedImg;
	
// Operations
public:

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CImageView)
	public:
	virtual void OnDraw(CDC* pDC);  // overridden to draw this view
	virtual BOOL PreCreateWindow(CREATESTRUCT& cs);
	virtual BOOL Create(LPCTSTR lpszClassName, LPCTSTR lpszWindowName, DWORD dwStyle, const RECT& rect, CWnd* pParentWnd, UINT nID, CCreateContext* pContext = NULL);
	protected:
// 	virtual BOOL OnPreparePrinting(CPrintInfo* pInfo);
// 	virtual void OnBeginPrinting(CDC* pDC, CPrintInfo* pInfo);
// 	virtual void OnEndPrinting(CDC* pDC, CPrintInfo* pInfo);
	//}}AFX_VIRTUAL

// Implementation
public:
  double m_dScanTime ;
	double m_dResTime;
	int m_iLastFrame;
	int m_iGrabFromProxy;
	int m_iProxyGrab;
	void MakeBinary(); // content commented out for Eilat ATJ
	int m_iDraw;
	int m_iFindBadPix;
	int m_iLock;
	int m_i8Bit;
	int m_iCheckExp;
	void Measure();
	int m_iBigWhiteBlobsArrInd[100];
	void RButDlgReaction();
	int m_iNotBlob;
	int m_iExist;
	CString m_strMsg;
	CLock m_lock;
	int m_iYOffset;
	double m_dMaxAmpl;

	int m_iBigSpots;
	BOOL m_bOnGrayBin;
	double m_dFindSpotETime;
	double m_dFindSpotBTime;
	BOOL m_bOnBinaryDisp;
	double m_dGrabTime;
	double m_dLastTime;
  double m_dLastDataArrivingTime ;
	double m_dStartGrabTime;
  char   m_LogMessageData[1024] ;
	void StopGrab(); // edited for Eilat ATJ
	double * m_dOldAngle;
	double m_dMaxDirBySectors;
	int FindBadPixels(); // content commented out for Eilat ATJ
	BOOL m_bRButtonDlgModal;
	CRButton m_RButtonDlg;
	CBigViewWnd m_BigViewWnd;
	int SaveCurrentImage( 
    int iXc = 0 , int iYc = 0 , int iRadiusX = 0 , int iRadiusY = 0 );
    // edited fot Eilat ATJ
  // Meas line power content commented out for Eilat ATJ
	double m_dAverX , m_dAverY , m_dAverSzX , m_dAverSzY ;
	int m_iNAver;
	double m_dAverSum2;
	double m_dAverSum;
	void CheckAndStopGrab( int bStop = 0 );
	void Clean();
	int SingleGrab( bool bSync = false ); // content edited for Eilat ATJ dev
	int GrabBack(); // inner contents commented out for Eilat ATJ
	afx_msg void OnAsynchroneMode(); // edited for Eilat ATJ
	afx_msg void OnFiltration(); // empty function
	afx_msg void OnMeasureBlob();// content was commented out for Eilat ATJ
	afx_msg void OnMeasureLine();// content was commented out for Eilat ATJ
	afx_msg void OnMeasurePower();// content was commented out for Eilat ATJ
	afx_msg void OnMeasureYDist();// content was commented out for Eilat ATJ
	afx_msg void OnLiveVideo();
	afx_msg void OnViewRadialDistr();// content was commented out for Eilat ATJ
  afx_msg void ProccedClick();

	int SaveRestoreImage( bool bSave );
	void SelectView(); // empty function
  int SetGrabTrigger( int Setting , int iCamNum = -1 ); 
  int SetGrabTriggerDelay( int iDelay_us , int iCamNum = -1 ); 
  int SetTriggerSource( int Setting , int iCamNum = -1 ); 
  int SetTriggerPolarity( int Setting , int iCamNum = -1 ); // 0 - fall front, 1 - rise front
  int SetExposure( int iExposure_scans , bool bInMicroSeconds = false , int iCamNum = -1 ); 
  int SetGain( int iGain , int iCamNum = -1 ); 
	int PreSetting(); // empty function
	static UINT CommandFunc(LPVOID P); // function is currently unused - but may be of service in shared memory interfacing in the future Eilat ATJ development.

  bool m_bDrawLine;
  int m_bAsyncGrab ;
  CString m_TriggerModePropertyName ;
  int m_iTriggerSrc ;
  int m_iTriggerPolarity ;
	DWORD m_MarkerColor;
  DWORD m_dwSHViewMode ;
	int m_MarkerType;
	CPoint m_MarkerPos;
  int m_bSubBackground;
	int m_SaveBuffer;
	double m_dLastY;
	int m_iMaxBlobNumber;
	int m_iLastMaxIntens;
	int m_iLastMinIntens;
	CImCNTLDlg * m_pDialog;
	double m_dScaleYmicronPerPixel;
	double m_dScaleXmicronPerPixel;
  int m_FeatureList;
// 	ProcessThread * m_hPrThread;
	CFImParamDialog m_ImParam;
	double m_BinThres;
	CFFTFilter m_FilterDial;
	int m_FFTShow ;
	COLORREF m_ProfPixels[ PROFILE_WIDTH ];
	int m_NProfilePoints;
	CPoint m_MarkCoord[PROFILE_WIDTH];
	COLORREF m_VProfPixels[ PROFILE_WIDTH ];
	int m_NVProfilePoints;
	CPoint m_VMarkCoord[PROFILE_WIDTH];
  BOOL	m_bContrasting;
	int m_LiveVideo,m_SingleGrab,m_SingleCont;
	int m_GrabDispLUT;
	int m_Timer;
	bool m_bShow;

  BOOL m_bsetAutoSave;
  BOOL m_bsetManSave ;
  BOOL m_bSaveOnePic;


  BOOL m_bVideoSavingEnabled;

  CString m_sSaveRectCam0,m_sSaveRectCam1;
  int m_iMaxIntensity ;
  bool m_bIsSpotData ;
  bool m_bIsLineData ;
  int m_iNSpotPackets ;
  int m_iNLinePackets ;
  int m_iNFrames ;
  
  int m_iSearchArea;
  

	//CFImParamDialog    m_DialogPar;

	virtual ~CImageView();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

protected:
  bool m_bRunBeforeInspect ;

// Generated message map functions
protected:
	//{{AFX_MSG(CImageView)
	afx_msg void OnTimer(UINT nIDEvent); //???
	afx_msg void OnGrabButton();
	
	afx_msg void OnDestroy();
	afx_msg void OnFileSave(); // func edited for Eilat ATJ
	afx_msg void OnFileOpen(); // func edited for Eilat ETj
	afx_msg void OnEditGrabber();
  afx_msg int  OnExposureMinus(); 
  afx_msg int  OnExposurePlus();
  afx_msg int  OnGainMinus(); 
  afx_msg int  OnGainPlus();
	afx_msg void OnFft(); // empty function
	afx_msg void OnEditFftFilter();
	afx_msg void OnSaveBinary(); // empty function
	afx_msg void OnMinusBackground();
	afx_msg void OnGrayBinaryDisp();
	afx_msg void OnEditParameters();
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnUpdateMinusBackground(CCmdUI* pCmdUI);
	afx_msg void OnMoving(UINT fwSide, LPRECT pRect);
	//afx_msg void OnPaint();
	afx_msg void OnUpdateAsynchMode(CCmdUI* pCmdUI); // empty func
	afx_msg void OnRButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnContrasting();
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct); // edited for Eilat ATJ
	afx_msg void OnUpdateLiveVideo(CCmdUI* pCmdUI);
	afx_msg void OnUpdateGrabButton(CCmdUI* pCmdUI);
	afx_msg void OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags);
	afx_msg void OnFILELoadCamCfg();
	afx_msg void OnBUTTON1024x1024();
	afx_msg void OnBUTTON512x512();
	afx_msg void OnAppExit();
  afx_msg void RButton();
  afx_msg void IniExpoandGain();
  afx_msg void OnCancel();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
public:
  void MakeContrast(void); // content commented out for Eilat ATJ
  int * m_piBlobIndexArr;
  int m_iLastMissedFrame;
  int m_iFirstChangeBuf;
  int m_iBackToLiveVideo;
  double m_dUpRelation;
  double m_dDownRelation;

  afx_msg void SavePictures(BOOL bSave);
  afx_msg void EnableLPFilter(bool bEnable);
  afx_msg void OnClose(); // edited for Eilat ATJ
  LRESULT OnCaptureGadgetMsg(WPARAM w_dummy, LPARAM l_param); 
  LRESULT OnImProcResultMsg(WPARAM w_dummy, LPARAM l_param); 
  LRESULT OnSharedMemoryMsg(WPARAM iOper, LPARAM LParam); 
  LRESULT OnLogMsg(WPARAM iPanIndex, LPARAM pStringPointer); 
  CString m_GrabberConfigFileName;
  afx_msg void OnLButtonDblClk(UINT nFlags, CPoint point);
  double m_dMaxBackExposureRatio;
  CRITICAL_SECTION m_BackProtect ;
  afx_msg void OnSize(UINT nType, int cx, int cy);
  int ProcessReceivedDataFrame(CDataFrame * pData , CMeasurementSet * pSet );
  int ProcessReceivedDataFrame(CString Data , CString Label , CMeasurementSet * pSet , bool bFinish = false );
  afx_msg void OnGraphSetup();
  int SwitchCamera( int iNFrames , int iCamNumber = -1 ) ;
  int LoadGraph(CString& GraphName);
  int SetThreshold( double dThres );
  bool WaitEndOfGrabAndProcess( int iTimeOut_ms = 1000 ) ;
  int SetGammaCorrection(int iValue, int iCamNum);
  afx_msg void OnViewGraph();
  afx_msg void OnParentNotify(UINT message, LPARAM lParam);
  afx_msg int AddspotToFile (CColorSpot cSpot , CString sCamNum );
  afx_msg void OnDontUseExternalImaging();
  afx_msg void OnSendImagesToLAN();
  afx_msg void OnTakeImagesFromLAN();
  afx_msg void UseSoliosCamera();
  afx_msg BOOL CheckIfStopAutoSaving();
  afx_msg void SetBMPPath(CString sBMPPath);
  afx_msg void SaveLastPicture(int iCam);
  afx_msg void EnableVideoWriting(bool  bEnable, int iCam);
  afx_msg void CloseVideoFile(int iCam);
  void LogMsg3(LPCTSTR Msg);

  void OnViewHelp();

};
/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_IMAGVIEW_H__5C595B64_DA04_11D2_84CB_00A0C9616FBC__INCLUDED_)


