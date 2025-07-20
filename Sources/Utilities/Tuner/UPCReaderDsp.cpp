//  $File : FeatureDetectorDsp.cpp : implementation file
//  (C) Copyright HomeBuilt Group and File X Ltd 2002. 
//
//
//
//  Revision History (excluding minor changes for specific compilers)
//   13 May 02 Firts release version, all followed changes must be listed below  (Andrey Chernushich)


#include "stdafx.h"
#include "tuner.h"
#include "UPCReaderDsp.h"
#include <recognition\BarcodeRd.h>
#include <messages.h>
#include <files\imgfiles.h>
#include <files\futils.h>
//#include <helpers\ctimer.h>
#include <fxfc\fxtimer.h>



#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

bool parse(pTVFrame tvf, CString& res);

void CCDPPEvent(int Event, void *Data, void *pParam, CDIBViewBase* wParam)
{
    CUPCReaderDsp* pv=(CUPCReaderDsp*)pParam;
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
// CUPCReaderDsp dialog


CUPCReaderDsp::CUPCReaderDsp(CWnd* pParent /*=NULL*/)
	: CDispDlg(CUPCReaderDsp::IDD, pParent)
    , m_ResultStr(_T(""))
{
    m_Frame.lpBMIH=NULL;	
    m_Frame.lpData=NULL;
    //{{AFX_DATA_INIT(CUPCReaderDsp)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
}


void CUPCReaderDsp::DoDataExchange(CDataExchange* pDX)
{
    CDispDlg::DoDataExchange(pDX);
    //{{AFX_DATA_MAP(CUPCReaderDsp)
    // NOTE: the ClassWizard will add DDX and DDV calls here
    //}}AFX_DATA_MAP
    DDX_Text(pDX, IDC_RESULTSTR, m_ResultStr);
}


BEGIN_MESSAGE_MAP(CUPCReaderDsp, CDispDlg)
	//{{AFX_MSG_MAP(CUPCReaderDsp)
	//}}AFX_MSG_MAP
    ON_WM_DESTROY()
    ON_BN_CLICKED(IDC_RUN_TVDB, CUPCReaderDsp::OnBnClickedRunTvdb)
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CUPCReaderDsp message handlers

BOOL CUPCReaderDsp::OnInitDialog() 
{
	CDispDlg::OnInitDialog();
    m_DataView.Create(GetDlgItem(IDC_DATAVIEW));
	m_Display.Create(GetDlgItem(IDC_UPCREADERDISPLAY));
    m_Display.SetScale(-1);
    m_Display.InitSelBlock(true,false);
    m_Display.SetCallback(CCDPPEvent,this);
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void CUPCReaderDsp::OnDestroy()
{
    if (m_Frame.lpBMIH) free(m_Frame.lpBMIH); m_Frame.lpBMIH=NULL;
    CDispDlg::OnDestroy();
}


bool CUPCReaderDsp::LoadDIB(BITMAPINFOHEADER *bmih)
{
    if (!bmih) return false;

    int dSize=bmih->biSizeImage+bmih->biSize;
    if (m_Frame.lpBMIH) free(m_Frame.lpBMIH); m_Frame.lpBMIH=NULL;    
    m_Frame.lpBMIH=(LPBITMAPINFOHEADER)malloc(dSize);
    memcpy(m_Frame.lpBMIH,bmih,dSize);

    CBarcodeRd bcRd;
    m_ResultStr="";
/*    if (m_Frame.lpBMIH->biWidth>640)
        _resample(&m_Frame,640,480); */
    if (bcRd.Parse(&m_Frame,false))
    {
        m_ResultStr=bcRd.GetResult();
        MessageBeep(-1);
    }
    CString tmp;
    tmp.Format("%.2f ms",bcRd.GetTimeSpent());
    GetDlgItem(IDC_TIMESPENT)->SetWindowText(tmp); 
    bool result=m_Display.LoadFrame(&m_Frame);
    if (m_Frame.lpBMIH) free(m_Frame.lpBMIH); m_Frame.lpBMIH=NULL;    
    UpdateData(FALSE);
    return result;
}

void CUPCReaderDsp::OnLineSelection(CPoint& lu, CPoint& rb)
{
    TRACE("+++ (%d:%d) - (%d:%d)\n",lu.x,lu.y, rb.x, rb.y);
    GetSection(m_Display.GetFramePntr(), lu, rb, m_DataView.GetData());
    m_DataView.Invalidate();
    double s=GetHRTickCount();
    FXString result;
    CBarcodeRd bcRd;
    if (!bcRd.Parse(m_DataView.GetData(),result))
    {
        m_ResultStr="";
    }
    else
        m_ResultStr=result;
    double ts=GetHRTickCount()-s;
    CString tmp;
    tmp.Format("%.2f ms",ts);
    GetDlgItem(IDC_TIMESPENT)->SetWindowText(tmp); 
    UpdateData(FALSE);
}

void CUPCReaderDsp::OnBnClickedRunTvdb()
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


