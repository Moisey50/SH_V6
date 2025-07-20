// Code128Reader.cpp : implementation file
//

#include "stdafx.h"
#include "tuner.h"
#include "Code128Reader.h"
#include <files\futils.h>
#include <files\imgfiles.h>
#include <messages.h>
#include <imageproc\recognition\htriggerbin.h>
#include <imageproc\recognition\decode128.h>

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

void CCode128Event(int Event, void *Data, void *pParam, CDIBViewBase* wParam)
{
    CCode128Reader* pv=(CCode128Reader*)pParam;
    if (Event==DIBVE_LINESELECTEVENT)
    {
        RECT rc;
        int style=((CDIBView*)wParam)->GetSelStyle();
        if (style==SEL_LINE)
        {
            ((CDIBView*)wParam)->GetSelLine(rc);
            CPoint lu=CPoint(rc.left,rc.top);
            CPoint rb=CPoint(rc.right,rc.bottom);
            pv->OnLineSelection(lu,rb);
        }
    }
    return;
}


/////////////////////////////////////////////////////////////////////////////
// CCode128Reader dialog


CCode128Reader::CCode128Reader(CWnd* pParent /*=NULL*/)
	: CDispDlg(CCode128Reader::IDD, pParent)
{
    m_Frame.lpBMIH=NULL;	
    m_Frame.lpData=NULL;
	//{{AFX_DATA_INIT(CCode128Reader)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
}


void CCode128Reader::DoDataExchange(CDataExchange* pDX)
{
	CDispDlg::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CCode128Reader)
		// NOTE: the ClassWizard will add DDX and DDV calls here
		DDX_Text(pDX, IDC_RESULTSTR, m_ResultStr);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CCode128Reader, CDispDlg)
	//{{AFX_MSG_MAP(CCode128Reader)
	ON_WM_DESTROY()
	ON_BN_CLICKED(IDC_RUN_TVDB, OnRunTvdb)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CCode128Reader message handlers

void CCode128Reader::OnDestroy() 
{
	CDispDlg::OnDestroy();
	
}

BOOL CCode128Reader::OnInitDialog() 
{
	CDispDlg::OnInitDialog();
    m_DataView.Create(GetDlgItem(IDC_DATAVIEW));
	m_Display.Create(GetDlgItem(IDC_UPCREADERDISPLAY));
    m_Display.SetScale(-1);
    m_Display.InitSelBlock(true,false);
    m_Display.SetCallback(CCode128Event,this);

	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void CCode128Reader::OnRunTvdb() 
{
    STARTUPINFO si;
    PROCESS_INFORMATION pi;
    char* tmpname=_tempnam("C:\\", "aviframe");
    CString TempFileName=tmpname; TempFileName+=".bmp";
    free(tmpname);

    saveDIB(TempFileName, m_Display.GetFramePntr());

    ZeroMemory( &si, sizeof(si) );
    si.cb = sizeof(si);

    CString execpath=GetStartDir();

    execpath+="tvdb300.exe";
    execpath+=" \"";
    execpath+=TempFileName;
    execpath+="\"";
    char a[_MAX_PATH];
    if (execpath.GetLength()>_MAX_PATH-1)
    {
        AfxMessageBox(ERROR_UNKNOWN);
        return;
    }
    strcpy(a,execpath);
    if( !CreateProcess( NULL,a,NULL,NULL,FALSE,0,NULL,NULL,&si,&pi)) 
    {
        AfxMessageBox(ERROR_CANTSTART_EXE);
        return;
    }
    WaitForSingleObject( pi.hProcess, INFINITE );

    // Close process and thread handles. 
    CloseHandle( pi.hProcess );
    CloseHandle( pi.hThread );	
}

bool CCode128Reader::LoadDIB(BITMAPINFOHEADER *bmih)
{
    if (!bmih) return false;

    int dSize=bmih->biSizeImage+bmih->biSize;
    if (m_Frame.lpBMIH) free(m_Frame.lpBMIH); m_Frame.lpBMIH=NULL;    
	m_Frame.lpBMIH=bmih;
	m_Frame.lpData=NULL;
	
	double s=GetHRTickCount();
	//_normalize(&m_Frame);
	pTVFrame bin=htriggerbin(&m_Frame);

	m_Frame.lpBMIH=bin->lpBMIH;
	m_Frame.lpData=bin->lpData;
	free(bin);
	FXString res;
	m_ResultStr="";
	if (parse128(&m_Frame,res))
	{
		m_ResultStr=res;
		MessageBeep(-1);
	} 
    double ts=GetHRTickCount()-s;
    CString tmp;
    tmp.Format("%.2f ms",ts);
    GetDlgItem(IDC_TIMESPENT)->SetWindowText(tmp); 

	bool result=m_Display.LoadFrame(&m_Frame);
    if (m_Frame.lpBMIH) free(m_Frame.lpBMIH); m_Frame.lpBMIH=NULL;    
    UpdateData(FALSE);
    return result;
}

void CCode128Reader::OnLineSelection(CPoint& lu, CPoint& rb)
{
    TRACE("+++ (%d:%d) - (%d:%d)\n",lu.x,lu.y, rb.x, rb.y);
    GetSection(m_Display.GetFramePntr(), lu, rb, m_DataView.GetData());
    m_DataView.Invalidate();
    double s=GetHRTickCount();
/*    CBarcodeRd bcRd;
    if (!bcRd.Parse(m_DataView.GetData(),m_ResultStr))
    {
        m_ResultStr="";
    } */
    double ts=GetHRTickCount()-s;
    CString tmp;
    tmp.Format("%.2f ms",ts);
    GetDlgItem(IDC_TIMESPENT)->SetWindowText(tmp); 
    UpdateData(FALSE);
}
