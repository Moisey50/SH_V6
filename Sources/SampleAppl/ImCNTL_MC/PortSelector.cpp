// PortSelector.cpp : implementation file
//

#include "stdafx.h"

#include "PortSelector.h"
#include "CLSerIPX.h"
#include "CLSerLynx.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CPortSelector dialog


CPortSelector::CPortSelector(CWnd* pParent /*=NULL*/)
	: CDialog(CPortSelector::IDD, pParent)
{
	m_nPort = -100;
	m_dllFileName = "";
}


void CPortSelector::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_PORT_LIST, m_PortList);
}


BEGIN_MESSAGE_MAP(CPortSelector, CDialog)
	ON_NOTIFY(NM_DBLCLK, IDC_PORT_LIST, OnNMDblclkPortList)
	ON_BN_CLICKED(IDC_BUTTON1, OnBnClickedButton1)
	ON_WM_CREATE()
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CPortSelector message handlers

BOOL CPortSelector::OnInitDialog() 
{
	const int maxNumPort = 10;

// 	USES_CONVERSION;
	CDialog::OnInitDialog();
	SetWindowPos(NULL,300,300,0,0,SWP_NOSIZE|SWP_NOZORDER);

	m_PortList.DeleteAllItems();
	m_PortList.SetExtendedStyle(m_PortList.GetExtendedStyle()|LVS_EX_FULLROWSELECT);
	{
		CCLSer* pclCameraLink = NULL;

		WIN32_FIND_DATA stFindData;
		TCHAR szSys[MAX_PATH];
		GetSystemDirectory(szSys, MAX_PATH);
		CString strDLLMask = szSys;
		strDLLMask += _T("\\clser*.dll");
		HANDLE hf;
		CString strDLLName;
		int nInit;
		CString dbgStr;
		LVCOLUMN clm;
		ZeroMemory(&clm,sizeof(clm));
		
		m_PortList.InsertColumn(0,_T("Port"),LVCFMT_CENTER,78,0);
		m_PortList.InsertColumn(1,_T("Interface DLL"),LVCFMT_CENTER,100,1);
		m_PortList.InsertColumn(2,_T("Camera"),LVCFMT_CENTER,100,2);
		CWinApp *pApp = AfxGetApp();
		bool bMarked = false;
		int mark = -1;
		ShowWindow(SW_SHOW);

		bool cond = true;
		CString clserial = _T("clserial.dll");
		for (hf = FindFirstFile(strDLLMask, &stFindData); (cond && (hf!=INVALID_HANDLE_VALUE)); cond = (FindNextFile(hf,&stFindData) == TRUE) )
		{
			if(clserial.CompareNoCase(stFindData.cFileName) == 0)
			{
				continue;
			}

			UINT port= 0;
			strDLLName.Format(_T("%s\\%s"), szSys, stFindData.cFileName);
			OutputDebugString(strDLLName);OutputDebugString(_T("\n"));

			if(pclCameraLink != NULL)
			{
				delete pclCameraLink;
				pclCameraLink = NULL;
			}
			
			dbgStr.Format(_T("ReInit - %s\n"),(LPCTSTR)strDLLName);
			OutputDebugString(dbgStr);
			
			pclCameraLink = new CCLSerLynx;
			nInit = pclCameraLink->Init(strDLLName);
			
			dbgStr.Format(_T("Initialized <%s> with error = %d 0x%x\n"),(LPCTSTR)strDLLName,nInit,nInit);
			OutputDebugString(dbgStr);
			if(nInit < 0)
			{
				OutputDebugString(_T("If return code < 0:\n -51 = dll not found\n -52 = clSerialInit func not defined\n -53 = clSerialRead func not defined\n -54 = clSerialWrite func not defined\n -55 = clSerialClose func not defined\n"));
				continue;
			}
			OutputDebugString(_T("Collect available Ports\n"));

			nInit = pclCameraLink->CheckPort(port);
			if(nInit < 0)
			{
				dbgStr.Format(_T("clCameraLink.CheckPort(%d) = %d\n"),port,nInit);
				OutputDebugString(dbgStr);
				continue;
			}
	
			while (nInit >= 0 && (int)port < maxNumPort)
			{
				CString str,camName;
				str.Format(_T("%02d"),port);
				int num = m_PortList.GetItemCount();
				int num1;
				int camID = pclCameraLink->CheckConnectedCamera();
				if(camID != LYNX_ID)
				{
					delete pclCameraLink;
					pclCameraLink = NULL;
					pclCameraLink = new CCLSerIPX;
					nInit = pclCameraLink->Init(strDLLName);
					dbgStr.Format(_T("Initialized <%s> with error = %d 0x%x\n"),(LPCTSTR)strDLLName,nInit,nInit);
					OutputDebugString(dbgStr);
					if(nInit >= 0)
					{
						nInit = pclCameraLink->CheckPort(port);
						if(nInit >= 0)
						{
							camID = pclCameraLink->CheckConnectedCamera();
						}
					}
				}

				camName = pclCameraLink->GetCameraName();

				num1=m_PortList.InsertItem(LVIF_TEXT|LVIF_PARAM,num,str,0,0,0,port);
				m_PortList.SetItem(num1,1,LVIF_TEXT,stFindData.cFileName,0,0,0,port);
				m_PortList.SetItem(num1,2,LVIF_TEXT,camName,0,0,0,port);
				
				str.Format(_T("%02d:%s %s\n"),port,stFindData.cFileName,camName);
				OutputDebugString(str);

				if(camID != LYNX_ID)
				{
					delete pclCameraLink;
					pclCameraLink = NULL;
					pclCameraLink = new CCLSerLynx;
					nInit = pclCameraLink->Init(strDLLName);
				}
				
				port++;
				nInit = pclCameraLink->CheckPort(port);
				if(nInit < 0)
				{
					dbgStr.Format(_T("clCameraLink.CheckPort(%d) = %d\n"),port,nInit);
					OutputDebugString(dbgStr);
				}

				m_PortList.RedrawWindow();
			}

			dbgStr.Format(_T("total Ports count = %d\n"),port);
			OutputDebugString(dbgStr);
		}
		if(pclCameraLink != NULL)
		{
			delete pclCameraLink;
			pclCameraLink = NULL;
		}
		FindClose(hf);

		int nComPort;
		for(nComPort=0;nComPort<10;nComPort++)
		{
			if(pclCameraLink != NULL)
			{
				delete pclCameraLink;
				pclCameraLink = NULL;
			}
			CString strComPort;
			strComPort.Format(_T("COM%d"),nComPort);
			pclCameraLink = new CCLSerIPX;
			nInit = pclCameraLink->Init(strComPort);
			if(nInit < 0)
			{
				dbgStr.Format(_T("%s can't be opened\n"),strComPort);
				OutputDebugString(dbgStr);
				continue;
			}
			CString camName,str;
			int camID = pclCameraLink->CheckConnectedCamera();
			camName = pclCameraLink->GetCameraName();

			int num = m_PortList.GetItemCount();
			int num1;
			num1=m_PortList.InsertItem(LVIF_TEXT|LVIF_PARAM,num,(LPCTSTR)strComPort,0,0,0,-1);
			m_PortList.SetItem(num1,1,LVIF_TEXT,(LPCTSTR)_T(""),0,0,0,-1);
			m_PortList.SetItem(num1,2,LVIF_TEXT,(LPCTSTR)camName,0,0,0,-1);
			str.Format(_T("%s --- %s\n"),strComPort,camName);
			OutputDebugString(str);
			m_PortList.RedrawWindow();
		}

		if(pclCameraLink != NULL)
		{
			delete pclCameraLink;
			pclCameraLink = NULL;
		}
	}
	
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void CPortSelector::OnOK() 
{
	int num = m_PortList.GetSelectionMark();
	if(num >= 0)
	{
		UINT port;
		CString str,str0,str2;
		port = (UINT)m_PortList.GetItemData(num);
		str = m_PortList.GetItemText(num,1);
		CString dbgStr;
		str0 = m_PortList.GetItemText(num,0);
		str2 = m_PortList.GetItemText(num,2);
		dbgStr.Format(_T("selected string-%d : %s:%s - %s\n"),num,str0,str,str2);
		OutputDebugString(dbgStr);
		dbgStr.Format(_T("parameter in this string - port = %d\n"),port);
		OutputDebugString(dbgStr);

		CCLSer* pclCameraLink;
		if(port != -1)
		{
			WIN32_FIND_DATA stFindData;
			TCHAR szSys[MAX_PATH];
			GetSystemDirectory(szSys, MAX_PATH);
			CString strDLLFile = szSys;
			strDLLFile+="\\";
			strDLLFile+=str;
			HANDLE hf = FindFirstFile(strDLLFile, &stFindData);

			int nInit;
			if(hf != INVALID_HANDLE_VALUE)
			{
				pclCameraLink = new CCLSerIPX;
				nInit = pclCameraLink->Init(strDLLFile);
				nInit = pclCameraLink->CheckPort(port);
				if(nInit>=0)
				{
					m_dllFileName = strDLLFile;
					m_nPort = port;
				}
				else
				{
					m_dllFileName.Empty();
					m_nPort = -1;
				}
				delete pclCameraLink;
				if(m_dllFileName.IsEmpty())
				{
					dbgStr.Format("Can't open port %s:%d",str,port);
					FindClose(hf);
					return;
				}
			}
			else
			{
				dbgStr.Format("File %s disappeared",strDLLFile);
				AfxMessageBox(dbgStr);
				return;
			}
			FindClose(hf);
		}
		else
		{
			m_dllFileName = str0;
			m_nPort = port;
		}

		CDialog::OnOK();
	}
	else
	{
		AfxMessageBox(_T("Camera not selected"));
	}
}

void CPortSelector::OnNMDblclkPortList(NMHDR *pNMHDR, LRESULT *pResult)
{
	*pResult = 0;
	OnOK();
}

void CPortSelector::OnBnClickedButton1()
{
	OnInitDialog();
}

int CPortSelector::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if (CDialog::OnCreate(lpCreateStruct) == -1)
		return -1;

	return 0;
}
