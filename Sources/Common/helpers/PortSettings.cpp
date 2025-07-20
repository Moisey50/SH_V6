// CComPortSettings.cpp : implementation file
//

#include "stdafx.h"
#include "COMCapture.h"
#include "PortSettings.h"
#include "TxtGadgets.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

#define THIS_MODULENAME "CComPortSettings"

const char PortNames[][6]={"COM1:","COM2:","COM3:","COM4","COM5","COM6","COM7","COM8","COM9","COM10"};
const char WrongName[]="*";

const char * GetPortName(int No)
{
  if ((No>=0) && (No<10))
    return(PortNames[No]);
  return(WrongName);
}

const DWORD Bauds[]={ CBR_110, CBR_300, CBR_600, CBR_1200, CBR_2400,
  CBR_4800, CBR_9600, CBR_14400, CBR_19200, CBR_38400,
  CBR_56000, CBR_115200, CBR_128000, CBR_256000 };
DWORD GetBaudRate(int No)
{
  if ((No>=0) && (No<(sizeof(Bauds)/sizeof(DWORD))))
    return(Bauds[No]);
  return(-1);
}
DWORD GetIndex( DWORD iBaud )
{
  for ( int i = 0 ; i < ARR_SIZE(Bauds) ; i++ )
  {
    if ( Bauds[i] == iBaud )
      return i ;
  }
  return 11 ;
}

const BYTE Parities[]={NOPARITY,EVENPARITY,ODDPARITY,MARKPARITY,SPACEPARITY};
const char * ParityAsText[5] = 
{
  "No Parity", "Even Parity" , "Odd Parity" , "Mark Parity" , "Space Parity"
} ;

BYTE GetParity(int No)
{
  if ((No>=0) && (No<(sizeof(Parities)/sizeof(BYTE))))
    return(Parities[No]);
  return(-1);
}


/////////////////////////////////////////////////////////////////////////////
// CComPortSettings dialog

CComPortSettings::CComPortSettings(CGadget* pGadget, CWnd* pParent)
: CGadgetSetupDialog(pGadget, CComPortSettings::IDD, pParent)
{
  //{{AFX_DATA_INIT(CComPortSettings)
  m_PORTCB = -1;
  m_Parity = 0;
  m_DatBits = 3;
  m_BaudRate = 115200;
  m_StopBits = 0;
  m_DTRDSR = FALSE;
  m_XonXoff = FALSE;
  m_RtsCts = FALSE;
	m_WaitForReturn = FALSE;
	//}}AFX_DATA_INIT
}


void CComPortSettings::DoDataExchange(CDataExchange* pDX)
{
  CGadgetSetupDialog::DoDataExchange(pDX);
  //{{AFX_DATA_MAP(CComPortSettings)
  DDX_CBIndex(pDX, IDD_PORTCB, m_PORTCB);
  DDX_CBIndex(pDX, IDD_PARITY, m_Parity);
  DDX_CBIndex(pDX, IDD_DATABITS, m_DatBits);
  DDX_CBIndex(pDX, IDD_BAUDRATE, m_BaudRate);
  DDX_CBIndex(pDX, IDD_STOPBITS, m_StopBits);
  DDX_Check(pDX, IDD_DTRDSR, m_DTRDSR);
  DDX_Check(pDX, IDD_XONXOFF, m_XonXoff);
  DDX_Check(pDX, IDD_RTSCTS, m_RtsCts);
	DDX_Check(pDX, IDC_WAIT_FOR_CR, m_WaitForReturn);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CComPortSettings, CGadgetSetupDialog)
  //{{AFX_MSG_MAP(CComPortSettings)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CComPortSettings message handlers
bool CComPortSettings::Show(CPoint point, LPCTSTR uid)
{
    FXString DlgHead;
    DlgHead.Format("%s Setup Dialog", uid);
    if (!m_hWnd)
    {
        FX_UPDATERESOURCE fur(pThisDll->m_hResource);
        if (!Create(IDD_COMMCNFIG, NULL))
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
BOOL CComPortSettings::Create(UINT nIDTemplate, CWnd* pParentWnd)
{
    FXPropertyKit pk;
    COMCapture* Gadget = (COMCapture*)m_pGadget;
    Gadget->PrintProperties(pk);
    pk.GetInt("Port", m_PORTCB);
    pk.GetInt("BaudRate", m_BaudRate); m_BaudRate=GetIndex(m_BaudRate);
    pk.GetInt("DataBits", m_DatBits);
    FXString ParityText;
    pk.GetString("Parity", ParityText ) ;
    m_Parity = 0 ;
    for ( int i = 0 ; i < sizeof(ParityAsText)/sizeof(LPCTSTR) ; i++ )
    {
    if ( ParityText == ParityAsText[i] )
    {
      m_Parity = i ;
      break ;
    }
    }
    pk.GetInt("StopBits", m_StopBits);
    pk.GetInt("XonXoff",  m_XonXoff);
    pk.GetInt("DtrDsr", m_DTRDSR );
    pk.GetInt("RtsCts", m_RtsCts);
    pk.GetInt("WantsReturn", m_WaitForReturn);

    return CGadgetSetupDialog::Create(nIDTemplate, pParentWnd);
}

void CComPortSettings::UploadParams()
{
    UpdateData(TRUE);
    bool Invalidate=false;
    FXPropertyKit pk;
    pk.WriteInt("Port", m_PORTCB);
    pk.WriteInt("BaudRate", Bauds[m_BaudRate]);
    pk.WriteInt("DataBits", m_DatBits);
    pk.WriteString("Parity", ParityAsText[m_Parity]);
    pk.WriteInt("StopBits", m_StopBits );
    pk.WriteInt("XonXoff", m_XonXoff );
    pk.WriteInt("DtrDsr", m_DTRDSR);
    pk.WriteInt("RtsCts", m_RtsCts);
    pk.WriteInt("WantsReturn", m_WaitForReturn);
    COMCapture* Gadget = (COMCapture*)m_pGadget;
    Gadget->ScanProperties(pk,Invalidate);
    Gadget->OpenConnection();
    CGadgetSetupDialog::UploadParams();
}
