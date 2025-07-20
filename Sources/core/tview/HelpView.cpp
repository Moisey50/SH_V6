// HelpView.cpp : implementation file
//

#include "stdafx.h"
#include <gadgets\tview.h>
#include "helpview.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

#define THIS_MODULENAME "CHELPVIEW"

__forceinline bool IsLocalFile(LPCTSTR url)
{
    CString tmpS(url);
    if (tmpS.Find(_T("http://"))==0) return false;
    if (tmpS.Find(_T("res://"))==0) return false;
    return true;
}

__forceinline bool getTempName(CString& name)
{
    char temppath[_MAX_PATH];
    char tempname[_MAX_PATH];
    GetTempPath(_MAX_PATH,temppath);
    GetTempFileName(temppath,_T("shshlp"),0,tempname);
    int pos=(int)strlen(tempname)-4;
    ASSERT(_tcscmp(&tempname[pos],_T(".tmp"))==0);
    strcpy(&tempname[pos],_T(".htm"));
    name=tempname;
    return true;
}

/////////////////////////////////////////////////////////////////////////////
// CHelpView

IMPLEMENT_DYNCREATE(CHelpView, CHtmlView)

CHelpView::CHelpView()
{
	//{{AFX_DATA_INIT(CHelpView)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
}

CHelpView::~CHelpView()
{
    POSITION pos;
    CString key, fname;
    for( pos = m_AliasedFiles.GetStartPosition(); pos != NULL; )
    {
        m_AliasedFiles.GetNextAssoc( pos, key, fname );
        DeleteFile(fname);
    }
    m_AliasedFiles.RemoveAll();
}

void CHelpView::DoDataExchange(CDataExchange* pDX)
{
	CHtmlView::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CHelpView)
		// NOTE: the ClassWizard will add DDX and DDV calls here
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CHelpView, CHtmlView)
	//{{AFX_MSG_MAP(CHelpView)
		// NOTE - the ClassWizard will add and remove mapping macros here.
	//}}AFX_MSG_MAP
    ON_WM_SIZE()
    ON_WM_DESTROY()
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CHelpView diagnostics

#ifdef _DEBUG
void CHelpView::AssertValid() const
{
	CHtmlView::AssertValid();
}

void CHelpView::Dump(CDumpContext& dc) const
{
	CHtmlView::Dump(dc);
}
#endif //_DEBUG

/////////////////////////////////////////////////////////////////////////////
// CHelpView message handlers

BOOL CHelpView::Create(CWnd* pParentWnd, DWORD dwAddStyle) 
{
    BOOL RESULT=TRUE;
    RECT rect;

    pParentWnd->GetClientRect(&rect);
	VERIFY(CHtmlView::Create(NULL, "Help", WS_CHILD|WS_VISIBLE, rect, pParentWnd, AFX_IDW_PANE_FIRST));
	UpdateWindow();
	return (RESULT);
}

void CHelpView::OnDestroy()
{
    CHtmlView::OnDestroy();
}

void CHelpView::SetDefaultBrowseDirectory(LPCTSTR path)
{
    m_DefaultDir=path;
}

void CHelpView::Navigate(LPCTSTR URL, DWORD dwFlags,LPCTSTR lpszTargetFrameName,LPCTSTR lpszHeaders, LPVOID lpvPostData,DWORD dwPostDataLen)
{
    CString newURL=URL;
    PreProcessURL(newURL);
    CHtmlView::Navigate(newURL, dwFlags,lpszTargetFrameName,lpszHeaders, lpvPostData, dwPostDataLen);
}

void CHelpView::Navigate2(LPCTSTR lpszURL, DWORD dwFlags,LPCTSTR lpszTargetFrameName, LPCTSTR lpszHeaders,LPVOID lpvPostData, DWORD dwPostDataLen)
{
    CString newURL=lpszURL;
    PreProcessURL(newURL);
    CHtmlView::Navigate2(newURL, dwFlags,lpszTargetFrameName, lpszHeaders,lpvPostData, dwPostDataLen);
}

bool CHelpView::MemoryNavigate(LPCTSTR httpdoc)
{
    CString tempname;
    getTempName(tempname);
    
    CFile fl;
    CFileException e;

    int len= (int) strlen(httpdoc);
    if (!fl.Open(tempname,CFile::modeCreate | CFile::modeWrite, &e)) 
    {
        return false;
    }
    fl.Write(httpdoc,len);
    fl.Close();
    Navigate(tempname);
    m_tmpFileName=tempname;
    return true;
}

void CHelpView::OnDocumentComplete(LPCTSTR lpszURL) 
{
	CHtmlView::OnDocumentComplete(lpszURL);
    if (m_tmpFileName.GetLength())
    {
        DeleteFile(m_tmpFileName);
        m_tmpFileName.Empty();
    }
}

__forceinline bool istvgurl(LPCTSTR lpszURL)
{
    const char tvgpreffix[]="tvg://";
    bool res=true;
    for (unsigned i=0; i<strlen(tvgpreffix); i++)
    {
        res&=(toupper(tvgpreffix[i])==toupper(lpszURL[i]));
    }
    return res;
}

__forceinline CString getdlldir(CString& str)
{
    const char prefix[]="res://";
    CString retV("");
    if (str.Find(prefix)==0)
    {
        retV=str.Mid( (int) strlen(prefix));
    }
    int pos=-1;
    pos=retV.Find("dll/");
    if (pos>=0)
    {
        retV=retV.Left(pos+3);
    }
    else
        retV="";
    return retV;
}

void CHelpView::OnBeforeNavigate2(LPCTSTR lpszURL, DWORD nFlags, LPCTSTR lpszTargetFrameName, CByteArray& baPostedData, LPCTSTR lpszHeaders, BOOL* pbCancel) 
{
    CString URL(lpszURL);
    CString fName= (LPCTSTR) FxGetFileName(URL);
    if (istvgurl(lpszURL))
    {
        CWnd* wnd=AfxGetApp()->GetMainWnd(); 
        if (wnd)
        {
            CString tmpS=GetLocationURL();
            CString dlldir=getdlldir(tmpS);
            HMODULE hModule=LoadLibrary(dlldir);
            if (hModule)
            {
                int resID=atoi(fName);
                HRSRC hSrc=FindResource(hModule, MAKEINTRESOURCE(resID),"TVG");
                if (hSrc)
                {
                HGLOBAL hGlob=LoadResource(hModule,hSrc);
                char * res=(char*)LockResource(hGlob);
                    ::SendMessage(wnd->m_hWnd,UM_SHOWSAMPLE,(WPARAM)(LPCTSTR)fName,(LPARAM)res);
                DeleteObject(hSrc);
                }
                else
                    SENDERR("Can't load %d.tvg file from resource, can't find resource #%d in \"%s\"",resID,resID,dlldir);
                FreeLibrary(hModule);
            }
        }
        *pbCancel=TRUE;
        return;
    }
    if (fName.GetLength()==0)
    {
        //*pbCancel=TRUE;
    }
    else if (fName[0]=='@')
    {
        URL=fName;
        *pbCancel=TRUE;
    }
    else if (IsLocalFile(lpszURL))
    {
        CFileStatus fs;
        if (!CFile::GetStatus(lpszURL,fs)) // if file doesn't exist - go to default dir
        {
            
            URL=m_DefaultDir+fName;
            *pbCancel=TRUE;
        }
    }
	//CHtmlView::OnBeforeNavigate2(URL, nFlags,	lpszTargetFrameName, baPostedData, lpszHeaders, pbCancel);
    if (*pbCancel)
    {
        Navigate(URL);
    }
}


bool CHelpView::CreateAliasedURL(LPCTSTR alias, LPCTSTR doc_htm)
{
    CFile fl;
    CFileException e;
    CString tempname;
    getTempName(tempname);

    int len= (int) strlen(doc_htm);
    if (!fl.Open(tempname,CFile::modeCreate | CFile::modeWrite, &e)) 
    {
        return false;
    }
    fl.Write(doc_htm,len);
    fl.Close();
    m_AliasedFiles.SetAt(alias,tempname);
    return true; 
}

bool CHelpView::PreProcessURL(CString &URL)
{
    if (URL[0]=='@')
    {
        CString newURL;
        if (m_AliasedFiles.Lookup(URL,newURL))
            URL=newURL;
        else
            return false;
    }
    return true;
}


void CHelpView::OnSize(UINT nType, int cx, int cy)
{
    CHtmlView::OnSize(nType, cx, cy);
    Invalidate();
    UpdateWindow();
    if (m_pBrowserApp)
        SetVisible(TRUE);
}

