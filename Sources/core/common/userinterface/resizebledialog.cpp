// ResizebleDialog.cpp : implementation file
//

#include "stdafx.h"
#include <shbase\shbase.h>

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CResizebleDialog dialog


CResizebleDialog::CResizebleDialog(LPCTSTR lpszTemplateName, CWnd* pParentWnd): 
        CDialog(lpszTemplateName, pParentWnd),
        m_pDlgItems(NULL),
        m_ItemsCnt(0),
        m_DlgOrgRC(0,0,0,0), 
        m_Updated(false)
{
	//{{AFX_DATA_INIT(CResizebleDialog)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
}

CResizebleDialog::CResizebleDialog(UINT nIDTemplate, CWnd* pParentWnd):
        CDialog(MAKEINTRESOURCE(nIDTemplate),pParentWnd),
        m_pDlgItems(NULL),
        m_ItemsCnt(0),
        m_DlgOrgRC(0,0,0,0), 
        m_Updated(false)
{

}

void CResizebleDialog::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CResizebleDialog)
		// NOTE: the ClassWizard will add DDX and DDV calls here
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CResizebleDialog, CDialog)
	//{{AFX_MSG_MAP(CResizebleDialog)
	ON_WM_DESTROY()
	ON_WM_SIZE()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CResizebleDialog message handlers

BOOL CResizebleDialog::Create(LPCTSTR lpszTemplateName, CWnd* pParentWnd)
{
	return CDialog::Create(lpszTemplateName, pParentWnd);
}

BOOL CResizebleDialog::Create( UINT nIDTemplate, CWnd* pParentWnd ) 
{
    return CDialog::Create(nIDTemplate,pParentWnd);
}

BOOL CResizebleDialog::OnInitDialog(pDlgItem pDlgItems, unsigned ItemsCnt) 
{
	CDialog::OnInitDialog();
    m_pDlgItems	= pDlgItems;
    m_ItemsCnt=ItemsCnt;
    GetClientRect(m_DlgOrgRC);
    for (unsigned i=0; i<ItemsCnt; i++)
    {
        CRect oRect;
        GetDlgItem(pDlgItems[i].iD)->GetWindowRect(oRect);
        ScreenToClient(oRect);
        pDlgItems[i].orgpos=oRect;
    }
	return TRUE;  
}

void CResizebleDialog::OnDestroy() 
{
	CDialog::OnDestroy();
}

void CResizebleDialog::OnSize(UINT nType, int cx, int cy) 
{
	CDialog::OnSize(nType, cx, cy);
	FitDialog();
}

void CResizebleDialog::FitDialog()
{
    if (!m_Updated) return;

    CRect rc;
    GetClientRect(rc);
    int dy=rc.bottom-m_DlgOrgRC.bottom;
    int dx=rc.right-m_DlgOrgRC.right;
    for (unsigned i=0; i<m_ItemsCnt; i++)
    {
        CRect oRc;
        CRect iRc=m_pDlgItems[i].orgpos;
        //if (dy>0)
        {
            if (m_pDlgItems[i].flags&BOTTOM_MOVE)
            {
                iRc.bottom+=dy;
                iRc.top+=dy;
            }
            if (m_pDlgItems[i].flags&BOTTOM_ALIGN)
            {
                iRc.bottom+=dy;
            }
        }
        //if (dx>0)
        {
            if (m_pDlgItems[i].flags&RIGHT_ALIGN)
            {
                iRc.right+=dx;
            }
            else if (m_pDlgItems[i].flags&RIGHT_MOVE)
            {
                iRc.left+=dx;
                iRc.right+=dx;
            }
        }
        //TRACE("++++++++ %d %d %d %d\n",iRc.left,iRc.top, iRc.right,iRc.bottom);
        CWnd * pWnd = GetDlgItem( m_pDlgItems[ i ].iD ) ;
        if ( pWnd )
          pWnd->MoveWindow( iRc );
        else
          TRACE( "\n******** CResizebleDialog::FitDialog -  No Window for element %d (ID=%u)" , i , m_pDlgItems[ i ].iD ) ;
    }
	Invalidate();
}
