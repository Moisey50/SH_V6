// ScriptRunnerDlg.cpp : implementation file

#include "stdafx.h"
#include "GenericScriptRunnerDlg.h"
#include "GenericScriptRunner.h"
#include <fstream> // for is exist file 

using namespace std;

#define THIS_MODULENAME "GenericScriptRunnerDlg"
// ScriptRunnerDlg dialog

IMPLEMENT_DYNAMIC(GenericScriptRunnerDlg, CDialog)
BEGIN_MESSAGE_MAP(GenericScriptRunnerDlg, CGadgetSetupDialog)
  ON_BN_CLICKED(IDC_BROWSE, &GenericScriptRunnerDlg::OnBnClickedBrowse)
	ON_BN_CLICKED(IDC_RELOAD, &GenericScriptRunnerDlg::OnBnClickedReload)
END_MESSAGE_MAP()

GenericScriptRunnerDlg::GenericScriptRunnerDlg(GenericScriptRunner* Gadget, CWnd* pParent /*=NULL*/)
: CGadgetSetupDialog(Gadget, GenericScriptRunnerDlg::IDD, pParent) 
{
  m_ScriptPath = "C:\\Graph\\Script.ch" ;
  m_GadgetName = Gadget->m_GadgetName ;
  m_iNumOfOutPins = Gadget->m_iNumOfOutPins ;
  m_ScriptPath = Gadget->m_ScriptPath ;
}
GenericScriptRunnerDlg::~GenericScriptRunnerDlg()
{

}
BOOL GenericScriptRunnerDlg::Create(UINT nIDTemplate, CWnd* pParentWnd /* = NULL */)
{
  FXPropertyKit pk;
  m_pGadget->PrintProperties(pk);
  //   pk.GetString("Path", m_ScriptPath);
  //   pk.GetInt("NumOfOutPins", m_iNumOfOutPins);
  return CGadgetSetupDialog::Create(nIDTemplate, pParentWnd);
}
void GenericScriptRunnerDlg::OnBnClickedBrowse()
{
  CFileDialog fd(TRUE, "ch, c", m_ScriptPath, OFN_FILEMUSTEXIST, "scripts (*.cpp)|*.cpp|scripts (*.c)|*.c|scripts (*.ch)|*.ch|all files (*.*)|*.*||", this);
  if (fd.DoModal() == IDOK)
  {
    m_ScriptPath = (LPCTSTR) fd.GetPathName();
    GetDlgItem(IDC_ENTER_SCRIPT_PATH)->SetWindowText(m_ScriptPath);
  }
}
void GenericScriptRunnerDlg::DoDataExchange(CDataExchange* pDX)
{
  FXString DlgHead ("");
  DlgHead.Append(m_GadgetName);
  DlgHead.Append(" Setup Dialog ");  
  SetWindowText(DlgHead);
  CDialog::DoDataExchange(pDX);
  CString Temp = (LPCTSTR)m_ScriptPath ;
  DDX_Text(pDX, IDC_ENTER_SCRIPT_PATH, Temp );
  m_ScriptPath = (LPCTSTR)Temp ;
  DDX_Text(pDX, NumOfOutPins, m_iNumOfOutPins); 
}
void GenericScriptRunnerDlg::OnOK()
{
  UpdateData(TRUE);
  DestroyWindow();
  UploadParams();
}
void GenericScriptRunnerDlg::UploadParams()
{
  bool Invalidate=false;
  FXPropertyKit pk;
  pk.WriteString("Path", m_ScriptPath);
  pk.WriteInt("NumOfOutPins", m_iNumOfOutPins);
  m_pGadget->ScanProperties(pk,Invalidate);
  fstream fin;
  fin.open(m_ScriptPath);
  if (fin.is_open()) // does script file exists?
  {
    fin.close();
    ((GenericScriptRunner*)m_pGadget)->SetCallOnExitFunc( true );    
    ((GenericScriptRunner*)m_pGadget)->SetNotInitialized( true );      
  }
  else   // script file doesn't exist
  {
    FXString NoFile ("There is no script in the path you entered:  ")  ;      // create a proper error massage 
    NoFile.Append(m_ScriptPath);
    NoFile.Append("          ");
    FXString NoFileHead = "Parse Error ";
    MessageBox(NoFile, NoFileHead, MB_ICONWARNING | MB_OK ); 
  }
}


void GenericScriptRunnerDlg::OnBnClickedReload()
{
	// TODO: Add your control notification handler code here
	UpdateData(TRUE);
	//DestroyWindow();
	UploadParams();
}

bool GenericScriptRunnerDlg::Show(CPoint point, LPCTSTR uid)
{
  HINSTANCE hOldResource=AfxGetResourceHandle();
  AfxSetResourceHandle(pThisDll->m_hResource);

  FXString DlgHead;
  DlgHead.Format("%s Setup Dialog", uid);
  if (!m_hWnd)
  {
    if (!Create(IDD_SCRIPT_PATH_DLG, NULL))
    {
      SENDERR_0("Failed to create Setup Dialog");
      AfxSetResourceHandle(hOldResource);
      return false;
    }
  }
  SetWindowText(DlgHead);
  SetWindowPos(NULL, point.x, point.y, 0, 0, SWP_NOSIZE | SWP_NOZORDER);
  ShowWindow(SW_SHOWNORMAL);
  AfxSetResourceHandle(hOldResource);
  // place initialization here if required
  // UpdateData(FALSE);
  return true;
}

