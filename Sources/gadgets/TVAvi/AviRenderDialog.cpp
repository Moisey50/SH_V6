// AviRenderDialog.cpp : implementation file
//

#include "stdafx.h"
#include "AviRenderDialog.h"
#include "AviRenderGadget.h"
#include "TVAvi.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

#define THIS_MODULENAME "AviRenderSetupDialog"

/////////////////////////////////////////////////////////////////////////////
// AviRenderDialog dialog


AviRenderDialog::AviRenderDialog(CGadget* pGadget, CWnd* pParent):
  CGadgetSetupDialog(pGadget, AviRenderDialog::IDD, pParent),
  m_VIDC_FOURCC(0),
  m_CycleWritting(FALSE)
  , m_MaxFileNumber(0)
  , m_MaxFileLength(0)
  , m_iRenderId( -1 )
  , m_CalcFrameRate(FALSE)
  , m_OverwriteFrameRate(FALSE)
  , m_FrameRate(25)
{
  //{{AFX_DATA_INIT(AviRenderDialog)
  //}}AFX_DATA_INIT
}

void AviRenderDialog::DoDataExchange(CDataExchange* pDX)
{
  CGadgetSetupDialog::DoDataExchange(pDX);
  CString tmpFN=m_Filename;
  //{{AFX_DATA_MAP(AviRenderDialog)
  DDX_Control(pDX, IDC_CODEC_LIST, m_CodecList);
  DDX_Text(pDX, IDC_FILENAME, tmpFN);
  //}}AFX_DATA_MAP
  DDX_Text(pDX, IDC_MAXFILES, m_MaxFileNumber);
  DDV_MinMaxInt(pDX, m_MaxFileNumber, -1, 268435455);
  DDX_Text(pDX, IDC_MAXFILELEN, m_MaxFileLength);
  m_Filename=tmpFN;
  DDX_Check(pDX, IDC_CALC_FRAMERATE, m_CalcFrameRate);
  DDX_Check(pDX, IDC_OVERWITE_FRAMERATE, m_OverwriteFrameRate);
  DDX_Text(pDX, IDC_EDIT_FRAMERATE, m_FrameRate);
  DDX_Text(pDX, IDC_RENDER_ID, m_iRenderId);
  DDV_MinMaxInt(pDX, m_iRenderId, -1, 1000000000);
  DDV_MinMaxDouble(pDX, m_FrameRate, 1, 100);
}


BEGIN_MESSAGE_MAP(AviRenderDialog, CGadgetSetupDialog)
  //{{AFX_MSG_MAP(AviRenderDialog)
  ON_BN_CLICKED(IDC_BROWSE_FILENAME, OnBrowseFilename)
  ON_BN_CLICKED(IDC_OVERWRITE, OnOverwrite)
  ON_BN_CLICKED(IDC_CLOSE_FILE, OnCloseFile)
  ON_BN_CLICKED(IDC_ADD_INPUT, OnAddInput)
  ON_CBN_SELCHANGE(IDC_CODEC_LIST, OnSelchangeCodecList)
  //}}AFX_MSG_MAP
  ON_BN_CLICKED(IDC_CYCLEWRITTING, &AviRenderDialog::OnBnClickedCyclewritting)
  ON_BN_CLICKED(IDC_OVERWITE_FRAMERATE, &AviRenderDialog::OnBnClickedOverwiteFramerate)
  ON_BN_CLICKED(IDC_CALC_FRAMERATE, &AviRenderDialog::OnBnClickedCalcFramerate)
END_MESSAGE_MAP()

void AviRenderDialog::UploadParams()
{
  UpdateData(TRUE);
  FXPropertyKit pk;
  bool Invalidate=false;
  pk.WriteBool("Overwrite", m_bOverwrite);
  pk.WriteString("File", m_Filename);
  pk.WriteInt("videofourcc",m_VIDC_FOURCC);
  pk.WriteBool("cyclewritting",m_CycleWritting);
  pk.WriteInt("cyclewritting",m_CycleWritting);
  pk.WriteInt("maxfilenumber",m_MaxFileNumber);
  pk.WriteInt("maxfilelength",m_MaxFileLength);
  pk.WriteBool("overwriteframerate", m_OverwriteFrameRate);
  pk.WriteBool("calcframerate", m_CalcFrameRate);
  pk.WriteDouble("framerate", m_FrameRate);
  pk.WriteInt("RenderId" , m_iRenderId ) ;
  m_pGadget->ScanProperties(pk, Invalidate);
  CGadgetSetupDialog::UploadParams();
}


/////////////////////////////////////////////////////////////////////////////
// AviRenderDialog message handlers

bool AviRenderDialog::Show(CPoint point, LPCTSTR uid)
{
  FXString DlgHead;
  DlgHead.Format("%s Setup Dialog", uid);
  if (!m_hWnd)
  {
    FX_UPDATERESOURCE fur(pThisDll->m_hResource);
    if (!Create(IDD_AVI_RENDER_DLG, NULL))
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


BOOL AviRenderDialog::OnInitDialog() 
{
  TRACE("+++ AviRenderDialog::OnInitDialog() \n");
  CGadgetSetupDialog::OnInitDialog();

  ASSERT(GetDlgItem(IDC_OVERWRITE)!=NULL);
  ASSERT(GetDlgItem(IDC_FILENAME)!=NULL);
  ASSERT(GetDlgItem(IDC_CODEC_LIST)!=NULL);

  FXString text;
  m_pGadget->PrintProperties(text);
  FXPropertyKit pk(text);
  FXString tmpS;
  if (!pk.GetString("File", tmpS))
    m_Filename = "";
  else
    m_Filename=tmpS;
  if (!pk.GetBool("Overwrite", m_bOverwrite))
    m_bOverwrite = TRUE;
  pk.GetInt("videofourcc",(int&)m_VIDC_FOURCC);

  pk.GetInt("cyclewritting",m_CycleWritting);
  GetDlgItem(IDC_CYCLEWRITTING)->SendMessage(BM_SETCHECK,m_CycleWritting?BST_CHECKED:BST_UNCHECKED,0);
  pk.GetInt("maxfilenumber",m_MaxFileNumber);
  pk.GetInt("maxfilelength",m_MaxFileLength);
  pk.GetInt( "RenderId" , m_iRenderId ) ;
  for (int i=0; i<m_Codecs.GetCodecCount(); i++)
  {
    if (m_CodecList.GetSafeHwnd()!=NULL)
    {
      int w=m_CodecList.AddString(m_Codecs.GetCodecItem(i).cName);
      if (w!=CB_ERR)
      {
        m_CodecList.SetItemData(w,m_Codecs.GetCodecItem(i).fccHandler);
        if (m_VIDC_FOURCC==m_Codecs.GetCodecItem(i).fccHandler)
          m_CodecList.SetCurSel(w);
      }
    }
  }
  ((CButton*)GetDlgItem(IDC_OVERWRITE))->SetCheck((m_bOverwrite ? MF_CHECKED : MF_UNCHECKED));
  GetDlgItem(IDC_CLOSE_FILE)->EnableWindow(((AviRender*)m_pGadget)->IsAviOpen());
  if (!pk.GetBool("overwriteframerate", m_OverwriteFrameRate))
    m_OverwriteFrameRate = FALSE;
  if (!pk.GetBool("calcframerate", m_CalcFrameRate))
    m_CalcFrameRate = FALSE;
  if (!pk.GetDouble("framerate", m_FrameRate))
    m_FrameRate = 25.0;

  UpdateData(FALSE);
  UpdateInterface();
  return TRUE;  
}

void AviRenderDialog::OnBrowseFilename() 
{
  CFileDialog fd(FALSE, "avi", m_Filename, OFN_HIDEREADONLY, "videos (*.avi)|*.avi|all files (*.*)|*.*||", this);
  if (fd.DoModal() == IDOK)
  {
    m_Filename = fd.GetPathName() ;
    FXSIZE iDotPos = m_Filename.ReverseFind( _T('.') ) ;
    if ( iDotPos > 0 )
       m_Filename.Delete( iDotPos , m_Filename.GetLength() - iDotPos ) ;
    GetDlgItem(IDC_FILENAME)->SetWindowText(m_Filename);
  }
}

void AviRenderDialog::OnOverwrite() 
{
  m_bOverwrite = !m_bOverwrite;
  ((CButton*)GetDlgItem(IDC_OVERWRITE))->SetCheck((m_bOverwrite ? MF_CHECKED : MF_UNCHECKED));
}

void AviRenderDialog::OnCloseFile() 
{
  CWaitCursor wc;
  ((AviRender*)m_pGadget)->CloseAvi();
  Sleep(1000);
  GetDlgItem(IDC_CLOSE_FILE)->EnableWindow(((AviRender*)m_pGadget)->IsAviOpen());
}

void AviRenderDialog::OnAddInput() 
{
  ((AviRender*)m_pGadget)->AddInput();
}

void AviRenderDialog::OnSelchangeCodecList() 
{
  int w=m_CodecList.GetCurSel( );
  if (w!=CB_ERR)
  {
    DWORD fourcc = (DWORD)m_CodecList.GetItemData(w);
    if (fourcc!=CB_ERR) 
      m_VIDC_FOURCC=fourcc;
    TRACE("+++ Choosen codec '%c%c%c%c'\n", ((char*)(&fourcc))[0],
      ((char*)(&fourcc))[1],
      ((char*)(&fourcc))[2],
      ((char*)(&fourcc))[3]);
  }
}

void AviRenderDialog::UpdateInterface()
{
  UpdateData(TRUE);
  GetDlgItem(IDC_OVERWITE_FRAMERATE)->EnableWindow(!m_CalcFrameRate);
  if (m_CalcFrameRate) 
    m_OverwriteFrameRate=FALSE;
  UpdateData(FALSE);
  GetDlgItem(IDC_EDIT_FRAMERATE)->EnableWindow(m_OverwriteFrameRate);
  GetDlgItem(IDC_MAXFILES)->EnableWindow(m_CycleWritting);
  GetDlgItem(IDC_MAXFILELEN)->EnableWindow(m_CycleWritting);
  FXString path = m_Filename ;
  FXSIZE iDotPos = path.ReverseFind( _T('.') ) ;
  if ( iDotPos > 0 )
    path.Delete( iDotPos , path.GetLength() - iDotPos ) ;
  GetDlgItem(IDC_FILENAME)->SetWindowText( path );
 
  if (m_CycleWritting)
  {
    ((CButton*)GetDlgItem(IDC_OVERWRITE))->SetCheck(MF_CHECKED);
    GetDlgItem(IDC_OVERWRITE)->EnableWindow(FALSE);
//     FXString path = FxExtractPath(m_Filename) ;
//     if ( path != m_Filename )
//       GetDlgItem(IDC_FILENAME)->SetWindowText( path );
  }
  else
  {
    ((CButton*)GetDlgItem(IDC_OVERWRITE))->SetCheck((m_bOverwrite ? MF_CHECKED : MF_UNCHECKED));
    GetDlgItem(IDC_OVERWRITE)->EnableWindow(TRUE);
//     FXString path=FxExtractPath(m_Filename);
//     if (path==m_Filename)
//       GetDlgItem(IDC_FILENAME)->SetWindowText(path +"%X %x.avi");
  }
}

void AviRenderDialog::OnBnClickedCyclewritting()
{
  m_CycleWritting=(GetDlgItem(IDC_CYCLEWRITTING)->SendMessage(BM_GETCHECK,0,0)==BST_CHECKED);
  UpdateInterface();
}

void AviRenderDialog::OnBnClickedOverwiteFramerate()
{
  UpdateInterface();
}

void AviRenderDialog::OnBnClickedCalcFramerate()
{
  UpdateInterface();
}
