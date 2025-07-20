#include "stdafx.h"
#include <helpers\BrowseFolder.h>

INT CALLBACK BrowseCallbackProc(HWND hwnd, UINT uMsg, LPARAM lp, LPARAM pData) 
{
    CBrowseFolder* pbf=(CBrowseFolder*)pData;
    switch(uMsg) 
    {
    case BFFM_INITIALIZED: 
        {
            FXString apppath;
            if (pbf)
                apppath=pbf->m_StartFolder;
            else
                apppath=FxGetAppPath();
            SendMessage(hwnd, BFFM_SETSELECTION, TRUE, (LPARAM)(LPCTSTR)apppath);
            break;
        }
    case BFFM_SELCHANGED: 
        {
            TCHAR szDir[MAX_PATH];
            if (SHGetPathFromIDList((LPITEMIDLIST) lp ,szDir))
                SendMessage(hwnd,BFFM_SETSTATUSTEXT,0,(LPARAM)szDir);
            break;
        }
    }
    return 0;
}

CBrowseFolder::CBrowseFolder(CWnd* pParentWnd):
    m_pParentWnd(pParentWnd),
    m_Title(_T("Select directory"))
{
}

CBrowseFolder::~CBrowseFolder(void)
{
}

int CBrowseFolder::DoModal( )
{
    int retV=IDOK;
    BROWSEINFO bi;
    LPITEMIDLIST pidl;

    CoInitialize(NULL);
    ZeroMemory(&bi,sizeof(bi));

    bi.hwndOwner=m_pParentWnd->m_hWnd;
    bi.pidlRoot=NULL;
    bi.lpszTitle=m_Title;
    bi.ulFlags = BIF_RETURNONLYFSDIRS | BIF_STATUSTEXT | BIF_USENEWUI;
    bi.lpfn = BrowseCallbackProc;
    bi.lParam=(LPARAM)this;
    pidl = SHBrowseForFolder(&bi);
    if (pidl)
    {
        if (!SHGetPathFromIDList(pidl,m_Result.GetBuffer(MAX_PATH)))
            retV=IDABORT;
    }
    else
        retV=IDABORT;
    CoTaskMemFree(pidl);
    CoUninitialize();

    return retV;
}