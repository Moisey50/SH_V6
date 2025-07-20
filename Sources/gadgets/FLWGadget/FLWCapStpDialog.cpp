// FLWCapStpDialog.cpp : implementation file
//

#include "stdafx.h"
#include "flwgadget.h"
#include "FLWCapStpDialog.h"
#include "FLWCaptureGadget.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

#define THIS_MODULENAME "FLWCaptureSetupDialog"

/////////////////////////////////////////////////////////////////////////////
// CFLWCapStpDialog dialog

CFLWCapStpDialog::CFLWCapStpDialog(CGadget* pGadget, CWnd* pParent):
  CGadgetSetupDialog(pGadget, CFLWCapStpDialog::IDD, pParent),
  m_bAutoRewindToStart(FALSE) ,
  m_bDoLog( FALSE )
  , m_dTimeZoom( 0 )
  , m_bByFRames( FALSE )
{
	//{{AFX_DATA_INIT(CFLWCapStpDialog)
	m_FileName = _T("");
	m_bLoop = FALSE;
	m_bFrameRate = FALSE;
	m_FrameRate = 40;
	m_bExtTrigger = FALSE;
	//}}AFX_DATA_INIT
}


void CFLWCapStpDialog::DoDataExchange(CDataExchange* pDX)
{
  CDialog::DoDataExchange( pDX );
  //{{AFX_DATA_MAP(CFLWCapStpDialog)
  DDX_Text( pDX , IDC_FILENAME , m_FileName );
  DDX_Check( pDX , IDC_LOOP , m_bLoop );
  DDX_Check( pDX , IDC_CONST_FRAME_RATE , m_bFrameRate );
  DDX_Text( pDX , IDC_FRAMERATE , m_FrameRate );
  DDX_Check( pDX , IDC_SOFTWARE_TRIGGER , m_bExtTrigger );
  DDX_Check( pDX , IDC_AUTOREWIND , m_bAutoRewindToStart );
  DDX_Check( pDX , IDC_DO_LOG , m_bDoLog );
  //}}AFX_DATA_MAP
  DDX_Text( pDX , IDC_FRAME_NUMBER_MIN , m_FrameNumberMin );
  DDV_MinMaxInt( pDX , m_FrameNumberMin , 0 , 100000000 );
  DDX_Text( pDX , IDC_FRAME_NUMBER_MAX , m_FrameNumberMax );
  DDV_MinMaxInt( pDX , m_FrameNumberMax , 1 , 100000000 );
  DDX_Text( pDX , IDC_TIME_MIN , m_dFrameTimeMin_ms );
  DDX_Text( pDX , IDC_TIME_MAX , m_dFrameTimeMax_ms );
  DDX_Text( pDX , IDC_PLAY_NUM_MIN , m_PlayNumberMin );
  DDV_MinMaxInt( pDX , m_PlayNumberMin , 0 , m_FrameNumberMax - 1 );
  DDX_Text( pDX , IDC_PLAY_NUM_MAX , m_PlayNumberMax );
  DDV_MinMaxInt( pDX , m_PlayNumberMax , m_PlayNumberMin , m_FrameNumberMax );
  DDX_Text( pDX , IDC_PLAY_TIME_MIN , m_dPlayTimeMin_ms );
  DDX_Text( pDX , IDC_PLAY_TIME_MAX , m_dPlayTimeMax_ms );

  DDX_Text( pDX , IDC_PLAY_TIME_ZOOM , m_dTimeZoom );
  DDV_MinMaxDouble( pDX , m_dTimeZoom , 0.001 , 50. );
  DDX_Radio( pDX , IDC_ITERATE_BY_FRAMES , m_bByFRames );
}


BEGIN_MESSAGE_MAP(CFLWCapStpDialog, CDialog)
	//{{AFX_MSG_MAP(CFLWCapStpDialog)
	ON_BN_CLICKED(IDC_BROWSE_FILENAME, OnBrowseFilename)
	ON_BN_CLICKED(IDC_CONST_FRAME_RATE, OnConstFrameRate)
	ON_BN_CLICKED(IDC_SOFTWARE_TRIGGER, OnSoftwareTrigger)
	//}}AFX_MSG_MAP
  ON_BN_CLICKED( IDC_APPLY , &CFLWCapStpDialog::OnBnClickedApply )
  ON_BN_CLICKED( IDC_ITERATE_BY_FRAMES , &CFLWCapStpDialog::OnBnClickedIterateByFrames )
  ON_BN_CLICKED( IDC_ITERATE_BY_TIME , &CFLWCapStpDialog::OnBnClickedIterateByTime )
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CFLWCapStpDialog message handlers

bool CFLWCapStpDialog::Show(CPoint point, LPCTSTR uid)
{
    FXString DlgHead;
    DlgHead.Format("%s Setup Dialog", uid);
    if (!m_hWnd)
    {
        FX_UPDATERESOURCE fur(pThisDll->m_hResource);
        if (!Create(IDD_FLW_CAPTURE_DLG, NULL))
        {
            SENDERR_0("Failed to create Setup Dialog");
            return false;
        }
    }
    SetWindowText(DlgHead);
    SetWindowPos(NULL, point.x, point.y, 0, 0, SWP_NOSIZE | SWP_NOZORDER);
    ShowWindow(SW_SHOWNORMAL);
    return true;
}

BOOL CFLWCapStpDialog::OnInitDialog() 
{
	CDialog::OnInitDialog();
	FXString text;

	m_pGadget->PrintProperties(text);
	FXPropertyKit pk(text);
    FXString tmpS;
	if (pk.GetString("FileName", tmpS))
        m_FileName=tmpS;
	int bTmp = 1;
	pk.GetInt("Loop", bTmp);
	m_bLoop = (bTmp == 1);
    
  bTmp = 1;
	pk.GetInt("AutoRewind", bTmp);
	m_bAutoRewindToStart = (bTmp == 1);

	pk.GetInt("ConstRate", bTmp);
	m_bFrameRate = (bTmp == 1);
	pk.GetInt("FrameRate", bTmp);
	m_FrameRate = (UINT)bTmp;
	pk.GetInt("ExtTrig", bTmp);
	m_bExtTrigger = (bTmp == 1);
  pk.GetInt( "DoLog" , m_bDoLog ) ;

  pk.GetInt( "FrameNumMin" , m_FrameNumberMin ) ;
  pk.GetInt( "FrameNumMax" , m_FrameNumberMax ) ;
  pk.GetDouble( "FrameTimeMin" , m_dFrameTimeMin_ms ) ;
  pk.GetDouble( "FrameTimeMax" , m_dFrameTimeMax_ms ) ;
  pk.GetInt(    "PlayNumMin" , m_PlayNumberMin ) ;
  pk.GetInt(    "PlayNumMax" , m_PlayNumberMax ) ;
  pk.GetDouble( "PlayTimeMin" , m_dPlayTimeMin_ms ) ;
  pk.GetDouble( "PlayTimeMax" , m_dPlayTimeMax_ms ) ;
  pk.GetDouble( "TimeZoom" , m_dTimeZoom ) ;
  if ( pk.GetInt( "IterateByFrames" , m_bIterateByFrames ) )
  {
    if ( m_bIterateByFrames )
    {
      m_bIterateByTime = FALSE ;
      m_bByFRames = 0 ;
    }
    else
    {
      m_bIterateByTime = TRUE ;
      m_bByFRames = 1 ;
    }
  };

  UpdateData( FALSE );
	UpdateCtrls();

    HWND hWndCtrl;
	
    GetDlgItem(IDC_LOOP, &hWndCtrl);
    
    ASSERT(hWndCtrl!=NULL);

	return TRUE;  
}

void CFLWCapStpDialog::OnBrowseFilename() 
{
	CFileDialog fd(TRUE, "flw", m_FileName, OFN_FILEMUSTEXIST, "FLW files (*.flw)|*.flw|all files (*.*)|*.*||", this);
	if (fd.DoModal() == IDOK)
	{
		m_FileName = fd.GetPathName();
		GetDlgItem(IDC_FILENAME)->SetWindowText(m_FileName);
	}
}

void CFLWCapStpDialog::UploadParams()
{
  UpdateData(TRUE);
	FXPropertyKit pk;
  bool Invalidate=false;
	pk.WriteString("FileName", m_FileName);
	pk.WriteInt("Loop", (m_bLoop ? 1 : 0));
  pk.WriteInt("AutoRewind",(m_bAutoRewindToStart? 1:0));
	pk.WriteInt("ConstRate", (m_bFrameRate ? 1 : 0));
	pk.WriteInt("FrameRate", (int)m_FrameRate);
	pk.WriteInt("ExtTrig", (m_bExtTrigger ? 1 : 0));
  pk.WriteInt( "DoLog" , m_bDoLog );
  pk.WriteInt( "PlayNumMin" , m_PlayNumberMin ) ;
  pk.WriteInt( "PlayNumMax" , m_PlayNumberMax ) ;
  pk.WriteDouble( "PlayTimeMin" , m_dPlayTimeMin_ms , "%.3f" ) ;
  pk.WriteDouble( "PlayTimeMax" , m_dPlayTimeMax_ms , "%.3f" ) ;
  pk.WriteDouble( "TimeZoom" , m_dTimeZoom , "%.4f" ) ;
  pk.WriteInt( "IterateByFrames" , m_bIterateByFrames );
  m_pGadget->ScanProperties( pk , Invalidate );
  CGadgetSetupDialog::UploadParams();
}

void CFLWCapStpDialog::OnConstFrameRate() 
{
	m_bFrameRate = !m_bFrameRate;
	if (m_bFrameRate)
		m_bExtTrigger = FALSE;
	UpdateCtrls();
}

void CFLWCapStpDialog::OnSoftwareTrigger() 
{
	m_bExtTrigger = !m_bExtTrigger;
	if (m_bFrameRate)
		m_bFrameRate = FALSE;
	UpdateCtrls();
}

void CFLWCapStpDialog::UpdateCtrls()
{
	((CButton*)GetDlgItem(IDC_SOFTWARE_TRIGGER))->SetCheck((m_bExtTrigger ? 1 : 0));
	((CButton*)GetDlgItem(IDC_CONST_FRAME_RATE))->SetCheck((m_bFrameRate ? 1 : 0));
	GetDlgItem(IDC_FRAMERATE)->EnableWindow(m_bFrameRate);
}



void CFLWCapStpDialog::OnBnClickedApply()
{
  UploadParams() ;
}


void CFLWCapStpDialog::OnBnClickedIterateByFrames()
{
  m_bIterateByFrames = TRUE ;
  m_bIterateByTime = FALSE ;
  UploadParams() ;
}


void CFLWCapStpDialog::OnBnClickedIterateByTime()
{
  m_bIterateByFrames = FALSE ;
  m_bIterateByTime = TRUE ;
  UploadParams() ;
}
