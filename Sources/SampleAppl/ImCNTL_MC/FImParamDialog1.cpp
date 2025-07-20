// FImParamDialog1.cpp : implementation file
//

#include "stdafx.h"
#include "ImCNTL.h"
#include "FImParamDialog.h"
#include "helpers/Registry.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif


#ifndef E_VALUE
  #define E_VALUE 2.7182818284590452353602874713527
#endif
/////////////////////////////////////////////////////////////////////////////
// CFImParamDialog dialog
            
CFImParamDialog::CFImParamDialog(CWnd* pParent /*=NULL*/)
	: CDialog(CFImParamDialog::IDD, pParent)
  , m_iThresholdValue(2048)
  , m_bFixThreshold(FALSE)
  , m_bBackSubstract(TRUE)
  , m_bUseExposureForBackground(false)
  , m_bLowPassOn(FALSE)
{
	//{{AFX_DATA_INIT(CFImParamDialog)
	m_iFFTXc = 511;
	m_iFFTYc = 511;
	m_BinThreshold = 0.5;//1. / ( E_VALUE * E_VALUE );
  m_iMeasExpansion = 75 ;
	m_UseFFTFilter = FALSE;
  m_PowerSaveName = "c:\\ATJ" ;
  m_iPowerRadius = 100 ;
  m_bClear = 0;
  m_iMinAmplitude = 500 ;
	m_iExposureBegin = 0;
	m_iViewCX = 512;
	m_iViewCY = 512;
	m_SaveExtendedInformation = FALSE;
	m_iMinArea = 200;
	m_iDiffractionRadius = 150 ;
	m_iMaxSpotDia = 75 ;
	m_iDiffractionMeasurementMethod = 1;
  m_iDiffractionRadius_Y = 40 ;
	m_iBackgroundDist = 200;
	m_iImageSaveMode = 0 ;
	m_bImageZip = FALSE;
	m_bSave16Bits = FALSE;
	m_ImageSaveDir = "C://ATJ//ImageData";//// _T("");

	m_bAutoSave = FALSE;
	m_iDiffrMaxSearchDist = 0;
	m_MeasBeamRotBySectors = FALSE;
	m_bViewDir = FALSE;
  m_bSaveImage = FALSE;
 
  m_bCam0 = TRUE;
  m_bCam1 = TRUE;
  m_bSaveVideo = FALSE;

  m_bShowRadDisable = FALSE;

  
	//}}AFX_DATA_INIT
}


void CFImParamDialog::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CFImParamDialog)
	DDX_Text(pDX, IDC_FFT_XC, m_iFFTXc);
	DDX_Text(pDX, IDC_FFT_YC, m_iFFTYc);
	DDX_Text(pDX, IDC_BIN_THRES, m_BinThreshold);
	DDV_MinMaxDouble(pDX, m_BinThreshold, 0., 1.);
	DDX_Text(pDX, IDC_MEAS_EXPANSION, m_iMeasExpansion);
	DDV_MinMaxDouble(pDX, m_iMeasExpansion, 1, 200 );
	DDX_Check(pDX, IDC_USE_FFT_FILTER, m_UseFFTFilter);
	DDX_Text(pDX, IDC_POWER_SAVE_NAME, m_PowerSaveName);
	DDX_Text(pDX, IDC_POWER_RADIUS, m_iPowerRadius);
  //DDX_Check(pDX, IDC_SAVE_POWER_GRADES, m_bSavePowerData);
  DDX_Check(pDX, IDC_AUTO_SAVE, m_bSaveImage);

  DDX_Check(pDX, IDC_SAVE_VIDEO , m_bSaveVideo);
  DDX_Check(pDX, IDC_VIDEO_CAM1 , m_bCam0);
  DDX_Check(pDX, IDC_VIDEO_CAM2 , m_bCam1);

	DDX_Text(pDX, IDC_MIN_AMPLITUDE, m_iMinAmplitude);
	DDX_Text(pDX, IDC_EXPOSURE_BEGIN, m_iExposureBegin);
	DDX_Text(pDX, IDC_VIEW_CX, m_iViewCX);
	DDX_Text(pDX, IDC_VIEW_CY, m_iViewCY);
	DDX_Check(pDX, IDC_SAVE_EXTENDED_INFO, m_SaveExtendedInformation);
	DDX_Text(pDX, IDC_MIN_AREA, m_iMinArea);
	DDX_Text(pDX, IDC_DIFFRACTION_RADIUS, m_iDiffractionRadius);
	DDX_Text(pDX, IDC_MAX_SPOT_DIA, m_iMaxSpotDia);
	DDX_Radio(pDX, IDC_DIFFRACTION_MAXIMUM_METOD, m_iDiffractionMeasurementMethod);
	DDX_Text(pDX, IDC_DIFFRACTION_RADIUS_Y, m_iDiffractionRadius_Y);
	DDX_Text(pDX, IDC_BACKGROUND_DIST, m_iBackgroundDist);
	DDX_Radio(pDX, IDC_SAVE_MODE, m_iImageSaveMode);
	DDX_Check(pDX, IDC_SAVE_DO_ZIP, m_bImageZip);
	DDX_Check(pDX, IDC_SAVE_16_BITS, m_bSave16Bits);
	DDX_Text(pDX, IDC_IMAGE_SAVE_DIR, m_ImageSaveDir);
  //DDX_Check(pDX, IDC_AUTO_SAVE, m_bAutoSave);
	DDX_Text(pDX, IDC_DIFFR_MAX_SEARCH_DIST, m_iDiffrMaxSearchDist);
	DDX_Check(pDX, IDC_BEAM_ROT_BY_SECTORS, m_MeasBeamRotBySectors);
	DDX_Check(pDX, IDC_VIEW_DIR, m_bViewDir);

	//}}AFX_DATA_MAP

  DDX_Text(pDX, IDC_THRESHOLD_VALUE, m_iThresholdValue);
  DDV_MinMaxInt(pDX, m_iThresholdValue, 0, 4095);
  DDX_Check(pDX, IDC_FIXED_THRESHOLD, m_bFixThreshold);
  DDX_Check(pDX, IDC_MINUS_BACK, m_bBackSubstract);
  DDX_Check(pDX, IDC_LOW_PASS, m_bLowPassOn);

  if (m_bSaveVideo)
  {
    GetDlgItem(IDC_VIDEO_CAM1)->EnableWindow(FALSE);
    GetDlgItem(IDC_VIDEO_CAM2)->EnableWindow(FALSE);
  }

  if (m_bShowRadDisable)
  {
    GetDlgItem(IDC_DIFFRACTION_RADIUS)->EnableWindow(FALSE);
    GetDlgItem(IDC_DIFFRACTION_RADIUS_Y)->EnableWindow(FALSE);
  }

  CRegistry RegIm("File Company\\ImCNTL");
  int iGraphMode = RegIm.GetRegiInt("CameraComm", "GraphMode", 1);

  //m_bFlippedImg = Reg.GetRegiInt( "Positions" , "UnifiedWH" , 0 ) ;

  if (iGraphMode == 0)
  {
    RegIm.WriteRegiInt("ImProcParam", "DiffractionRadius_pix_AVT", m_iDiffractionRadius);
    RegIm.WriteRegiInt("ImProcParam", "DiffractionRadius_Y_pix_AVT", m_iDiffractionRadius_Y );
  }
  else
  {
    RegIm.WriteRegiInt("ImProcParam", "DiffractionRadius_pix", m_iDiffractionRadius);
    RegIm.WriteRegiInt("ImProcParam", "DiffractionRadius_Y_pix", m_iDiffractionRadius_Y);
  }

}


BEGIN_MESSAGE_MAP(CFImParamDialog, CDialog)
	//{{AFX_MSG_MAP(CFImParamDialog)
	ON_BN_CLICKED(IDC_VIEW_DIR, OnViewDir)
	//}}AFX_MSG_MAP
 // ON_BN_CLICKED(IDC_AUTO_SAVE, &CFImParamDialog::OnBnClickedAutoSave)
  ON_BN_CLICKED(IDC_USE_EXPOSURE_FOR_BACKGROUND, &CFImParamDialog::OnBnClickedUseExposureForBackground)
  ON_BN_CLICKED(IDC_SAVE_POWER_GRADES, &CFImParamDialog::OnBnClickedSavePowerGrades)
  ON_BN_CLICKED(IDOK, &CFImParamDialog::OnBnClickedOk)

END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CFImParamDialog message handlers

void CFImParamDialog::OnViewDir() 
{
	// TODO: Add your control notification handler code here
	
}

// void CFImParamDialog::OnBnClickedAutoSave()
// {
//   UpdateData() ;
// }

void CFImParamDialog::OnBnClickedUseExposureForBackground()
{
  UpdateData() ;
}

void CFImParamDialog::OnBnClickedSavePowerGrades()
{

}

void CFImParamDialog::OnBnClickedOk()
{
  // TODO: Add your control notification handler code here
  OnOK();
}
LRESULT CFImParamDialog::OnEnableDisable(WPARAM wpar, LPARAM lpar)
{
  ( GetDlgItem( wpar) )->EnableWindow( lpar ) ;
  return 0 ;
}