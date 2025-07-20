// StaticEx.cpp : implementation file
//


#include "stdafx.h"
#include "StaticEx.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CStaticEx
static HHOOK lpMouseHook = NULL;

CStaticEx* CStaticEx::m_pThis = NULL;

CStaticEx::CStaticEx()
{
  CStaticEx::m_pThis = this;
}

CStaticEx::CStaticEx(CRelationalCheckList* pBuddy)
{
  CStaticEx::m_pThis = this;
  m_pBuddy = pBuddy;
}

CStaticEx::~CStaticEx()
{
   
}


BEGIN_MESSAGE_MAP(CStaticEx, CWnd)
	//{{AFX_MSG_MAP(CStaticEx)
	ON_WM_PAINT()
	ON_WM_SHOWWINDOW()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()
//	

/////////////////////////////////////////////////////////////////////////////
// CStaticEx message handlers


void CStaticEx::OnPaint() 
{
  CPaintDC dc(this); // device context for painting
  CDC mdc;
  CRect rc;

  GetClientRect(&rc);

  mdc.CreateCompatibleDC(&dc);
  CBitmap bm;
  bm.CreateCompatibleBitmap(&dc, rc.Width(), rc.Height());
  CBitmap* pOldBm = mdc.SelectObject(&bm);


  mdc.FillSolidRect(rc,GetSysColor(COLOR_3DFACE));
  dc.BitBlt(0, 0, rc.Width(), rc.Height(), &mdc, 0, 0, SRCCOPY);

  mdc.SelectObject(pOldBm);
  ReleaseDC(&dc);
  bm.DeleteObject();
  mdc.DeleteDC();
}

void CStaticEx::HideItems()
{
   if(m_pBuddy->m_pCurArray != NULL)
   {
     for(int i=0; i<m_pBuddy->m_pCurArray->GetSize(); i++)
	 {
       m_pBuddy->m_pCurArray->GetAt(i)->m_pwnd->ShowWindow(SW_HIDE);
	   m_pBuddy->m_pCurArray->GetAt(i)->m_pwnd->SetParent(m_pBuddy->m_pCurArray->GetAt(i)->m_pwndOrgParent);
	 }
     ShowWindow(SW_HIDE);
   }
}

void CStaticEx::OnShowWindow(BOOL bShow, UINT nStatus) 
{
  if(bShow)
  {
	lpMouseHook = SetWindowsHookEx(WH_MOUSE,CStaticEx::HookProc,NULL,GetCurrentThreadId());
    m_pBuddy->GetParent()->SendMessage(PN_MESSAGE_LOADDATA,(WPARAM)m_pBuddy->m_CurDropDownItem);
  }
  else
  {
	ASSERT(UnhookWindowsHookEx(lpMouseHook));
    m_pBuddy->GetParent()->SendMessage(PN_MESSAGE_UPDATEDATA,(WPARAM)m_pBuddy->m_CurDropDownItem);
  }
  CWnd::OnShowWindow(bShow, nStatus);
}

LRESULT CALLBACK CStaticEx::HookProc(int nCode,WPARAM wParam,LPARAM lParam)
{

  if(nCode == HC_ACTION||HC_NOREMOVE)
  {
    if(wParam == WM_LBUTTONDOWN)
	{
		MOUSEHOOKSTRUCT* pM = (MOUSEHOOKSTRUCT*)lParam;
		CRect rc;
		m_pThis->GetWindowRect(&rc);
		CPoint pt(pM->pt);

		if(!rc.PtInRect(pt))
          m_pThis->HideItems();
	}
  }
  return CallNextHookEx(lpMouseHook,nCode,wParam,lParam);
}



BOOL CStaticEx::PreCreateWindow(CREATESTRUCT& cs)
{
  static CString sClassName;
   if (sClassName.IsEmpty())
      sClassName = AfxRegisterWndClass(0);
   cs.lpszClass = sClassName;
   cs.style = WS_POPUP|WS_BORDER;
   return CWnd::PreCreateWindow(cs);
}

BOOL CStaticEx::Create(CWnd* pParentWnd, UINT nStyle/*=0*/, UINT nID/*=0*/)
{
return CreateEx(0,
      NULL,
      NULL,
      WS_POPUP,
      CRect(),
      pParentWnd,
      nID);
}

