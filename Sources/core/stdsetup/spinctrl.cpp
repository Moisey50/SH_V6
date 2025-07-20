// SpinCtrl.cpp : implementation file
//

#include "stdafx.h"
#include <gadgets\stdsetup.h>

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

extern int StdSetupShowDebugInfo ;

/////////////////////////////////////////////////////////////////////////////
// CSpinCtrl

CSpinCtrl::CSpinCtrl()
{
}

CSpinCtrl::~CSpinCtrl()
{
}


BEGIN_MESSAGE_MAP(CSpinCtrl, CStatic)
	//{{AFX_MSG_MAP(CSpinCtrl)
	ON_WM_SIZE()
	ON_WM_VSCROLL()
	ON_WM_SETFOCUS()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CSpinCtrl message handlers

BOOL CSpinCtrl::Create(DWORD dwStyle, const RECT& rect, CWnd* pParentWnd, UINT nID) 
{
	if (CStatic::Create(_T("Static"), dwStyle, rect, pParentWnd, nID))
    {
        if (m_Buddy.Create(ES_LEFT|ES_WANTRETURN|WS_CHILD|WS_VISIBLE,CRect(0,0,0,0), this, 0))
        {
            m_Buddy.ModifyStyleEx(0,WS_EX_CLIENTEDGE);
            m_Ctrl.Create(UDS_AUTOBUDDY|UDS_SETBUDDYINT|UDS_ARROWKEYS|UDS_ALIGNRIGHT|WS_VISIBLE|WS_CHILD,CRect(0,0,0,0),this,1);
        }
        return TRUE;
    }
    return FALSE;
}

void CSpinCtrl::OnSize(UINT nType, int cx, int cy) 
{
	CStatic::OnSize(nType, cx, cy);
    if (m_Buddy.GetSafeHwnd())
    {
        CRect rc;
        GetClientRect(rc);
        m_Buddy.MoveWindow(rc);
        m_Ctrl.SetBuddy(&m_Buddy);
    }
}

int CSpinCtrl::SetPos( int nPos )
{
    #if (_MSC_VER<1300)
        return m_Ctrl.SetPos(nPos);
    #else
  return /*m_Ctrl.SetPos32( nPos ); */m_Ctrl.PostMessage( UDM_SETPOS32 , 0 , nPos ) ;
#endif
}

int CSpinCtrl::GetPos( )
{
    int l,u;
    #if (_MSC_VER<1300)
        int retV=(short)m_Ctrl.GetPos();
        m_Ctrl.GetRange(l,u);
    #else
        int retV=m_Ctrl.GetPos32();
        m_Ctrl.GetRange32(l,u);
    #endif
    if (retV<l)
        retV=l;
    if (retV>u)
        retV=u;
    return retV;
}


void CSpinCtrl::SetRange( int nLower, int nUpper )
{
    #if (_MSC_VER<1300)
        m_Ctrl.SetRange( m_iMin = nLower , m_iMax = nUpper );
    #else
        m_Ctrl.SetRange32( m_iMin = nLower, m_iMax = nUpper );
    #endif
}

void CSpinCtrl::GetRange( int& nLower , int& nUpper )
{
  nLower = m_iMin ;
  nUpper = m_iMax ;
}

BOOL CSpinCtrl::OnCommand(WPARAM wParam, LPARAM lParam) 
{
    int nID=LOWORD(wParam);
    int msg=HIWORD(wParam);
    int nnID=GetDlgCtrlID( );    
    //TRACE("+++ CSpinCtrl::OnCommand ID=0x%x msg=0x%x ctrl=0x%x\n",nID,msg, lParam); 
    if (msg==EN_CHANGE)
        GetParent()->SendMessage(WM_COMMAND,MAKEWPARAM(nnID,msg),(LPARAM)(this->m_hWnd));
    if ( IsWindow( (HWND)lParam ) )
    	return CStatic::OnCommand(wParam, lParam);
//    return FALSE ;
    return TRUE ;
}

void CSpinCtrl::OnVScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar) 
{
	CStatic::OnVScroll(nSBCode, nPos, pScrollBar); //WM_SETFOCUS

}

void CSpinCtrl::OnSetFocus(CWnd* pOldWnd) 
{
	CStatic::OnSetFocus(pOldWnd);
    m_Buddy.SetFocus();
}

/////////////////////////////////////////////////////////////////////////////
// CSpinChkCtrl

CSpinChkCtrl::CSpinChkCtrl()
{
}

CSpinChkCtrl::~CSpinChkCtrl()
{
}


BEGIN_MESSAGE_MAP(CSpinChkCtrl, CStatic)
	//{{AFX_MSG_MAP(CSpinChkCtrl)
	ON_WM_SIZE()
	ON_WM_VSCROLL()
	ON_WM_SETFOCUS()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CSpinChkCtrl message handlers

BOOL CSpinChkCtrl::Create(DWORD dwStyle, const RECT& rect, CWnd* pParentWnd, UINT nID) 
{
	if (CStatic::Create(_T("Static"), dwStyle, rect, pParentWnd, nID))
    {
        if (m_Buddy.Create(ES_LEFT|ES_WANTRETURN|WS_CHILD|WS_VISIBLE,CRect(0,0,0,0), this, 0))
        {
            m_Buddy.ModifyStyleEx(0,WS_EX_CLIENTEDGE);
            m_Ctrl.Create(UDS_AUTOBUDDY|UDS_SETBUDDYINT|UDS_ARROWKEYS|UDS_ALIGNRIGHT|WS_VISIBLE|WS_CHILD,CRect(0,0,0,0),this,1);
        }
        m_Button.Create(_T(""),BS_AUTOCHECKBOX|WS_CHILD|WS_VISIBLE,CRect(0,0,0,0),this,2);
        return TRUE;
    }
    return FALSE;
}

void CSpinChkCtrl::OnSize(UINT nType, int cx, int cy) 
{
	CStatic::OnSize(nType, cx, cy);
    if (m_Buddy.GetSafeHwnd())
    {
        CRect rc;
        GetClientRect(rc);
        CRect brc=rc;
        brc.right-=rc.Height();
        CRect cbrc(rc); 
        cbrc.left=brc.right+1;
        m_Buddy.MoveWindow(brc);
        m_Ctrl.SetBuddy(&m_Buddy);
        m_Button.MoveWindow(cbrc);
    }
}

int CSpinChkCtrl::SetPos( int nPos )
{
#if (_MSC_VER<1300)
    return m_Ctrl.SetPos(nPos);
#else
  TCHAR Buf[ 20 ] ;
  _stprintf_s( Buf , "%d" , nPos ) ;
  ::PostMessage( m_Buddy.m_hWnd , WM_SETTEXT , 0 , (LPARAM)Buf ) ;
  //m_Buddy.SetWindowTextA( Buf ) ;
  return m_Ctrl.PostMessage( UDM_SETPOS32 , 0 , nPos ) ;
#endif
}

int CSpinChkCtrl::GetPos( )
{
    int l,u;
    #if (_MSC_VER<1300)
        int retV=(short)m_Ctrl.GetPos();
    #else
        int retV=m_Ctrl.GetPos32();
    #endif
    m_Ctrl.GetRange32(l,u);
    if (retV<l)
        retV=l;
    if (retV>u)
        retV=u;
    return retV;
}


void CSpinChkCtrl::SetRange( int nLower, int nUpper )
{
    #if (_MSC_VER<1300)
  m_Ctrl.SetRange( m_iMin = nLower , m_iMax = nUpper );
    #else
  m_Ctrl.SetRange32( m_iMin = nLower , m_iMax = nUpper );
    #endif
}

void CSpinChkCtrl::GetRange( int& nLower , int& nUpper )
{
  nLower = m_iMin ;
  nUpper = m_iMax ;
}

BOOL CSpinChkCtrl::OnCommand(WPARAM wParam, LPARAM lParam) 
{
    int nID=LOWORD(wParam);
    int msg=HIWORD(wParam);
    int nnID=GetDlgCtrlID( );    
    if ( StdSetupShowDebugInfo )
    TRACE("+++ CSpinChkCtrl::OnCommand ID=0x%x msg=0x%x ctrl=0x%x\n",nID,msg, lParam); 
    if (msg==EN_CHANGE)
        GetParent()->SendMessage(WM_COMMAND,MAKEWPARAM(nnID,msg),(LPARAM)(this->m_hWnd));
    else if (msg==BN_CLICKED)
    {
        GetParent()->SendMessage(WM_COMMAND,MAKEWPARAM(nnID,msg),(LPARAM)(this->m_hWnd));
        BOOL bChecked = (GetCheck() == TRUE) ;
        m_Buddy.EnableWindow( !bChecked ) ;
        m_Ctrl.EnableWindow( !bChecked ) ;
        if ( StdSetupShowDebugInfo )
        TRACE("++++ %s\n", GetCheck()?"Checked":"Unchecked");
    }
	return CStatic::OnCommand(wParam, lParam);
}

void CSpinChkCtrl::OnVScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar) 
{
	CStatic::OnVScroll(nSBCode, nPos, pScrollBar); //WM_SETFOCUS

}

void CSpinChkCtrl::OnSetFocus(CWnd* pOldWnd) 
{
	CStatic::OnSetFocus(pOldWnd);
  m_Buddy.SetFocus();
}

bool CSpinChkCtrl::GetCheck()
{
  bool bRes = ( m_Button.GetCheck() == 1 ) ;
//   m_Ctrl.EnableWindow( !bRes ) ;
  return bRes ;
}

void CSpinChkCtrl::SetCheck(bool set)
{
  m_Button.SendMessage( BM_SETCHECK , (set? BST_CHECKED : BST_UNCHECKED ) ) ;
  m_Buddy.PostMessage( WM_ENABLE , ( !set /*GetCheck()!=TRUE*/ ) );
  m_Ctrl.PostMessage( WM_ENABLE , ( !set /*GetCheck()!=TRUE*/ ) );
}

/////////////////////
//////   CChkCtrl

CChkCtrl::CChkCtrl()
{}

CChkCtrl::~CChkCtrl()
{}


BEGIN_MESSAGE_MAP( CChkCtrl , CStatic )
  //{{AFX_MSG_MAP(CChkCtrl)
  ON_WM_SIZE()
  ON_WM_SETFOCUS()
  //}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CChkCtrl message handlers

BOOL CChkCtrl::Create( DWORD dwStyle , 
  const RECT& rect , CWnd* pParentWnd , UINT nID , LPCTSTR name )
{
  if ( CStatic::Create( _T( "Static" ) , dwStyle , rect , pParentWnd , nID ) )
  {
    m_Button.Create( name , BS_AUTOCHECKBOX | WS_CHILD | WS_VISIBLE | BS_LEFT ,
      CRect( 0 , 0 , 0 , 0 ) , this , 2 );
    return TRUE;
  }
  return FALSE;
}

void CChkCtrl::OnSize( UINT nType , int cx , int cy )
{
  CStatic::OnSize( nType , cx , cy );
  if ( m_Button.GetSafeHwnd() )
  {
    CRect rc;
    GetClientRect( rc );
//     CRect brc = rc;
//     brc.right -= rc.Height();
//     CRect cbrc( rc );
//     cbrc.left = brc.right + 1;
//    m_Button.MoveWindow( cbrc );
    m_Button.MoveWindow( rc );
  }
}

BOOL CChkCtrl::OnCommand( WPARAM wParam , LPARAM lParam )
{
  int nID = LOWORD( wParam );
  int msg = HIWORD( wParam );
  int nnID = GetDlgCtrlID();
  if ( StdSetupShowDebugInfo )
  TRACE( "+++ CChkCtrl::OnCommand ID=0x%x msg=0x%x ctrl=0x%x\n" , nID , msg , lParam );
  if ( msg == BN_CLICKED )
  {
    GetParent()->SendMessage( WM_COMMAND , MAKEWPARAM( nnID , msg ) , (LPARAM) (this->m_hWnd) );
    //TRACE( "++++ %s\n" , GetCheck() ? "Checked" : "Unchecked" );
  }
  return TRUE ; // CStatic::OnCommand( wParam , lParam );
}

void CChkCtrl::OnSetFocus( CWnd* pOldWnd )
{
  CStatic::OnSetFocus( pOldWnd );
}

bool CChkCtrl::GetCheck()
{
  return (m_Button.GetCheck() == 1);
}

void CChkCtrl::SetCheck( bool set )
{
  m_Button.SetCheck( set ? 1 : 0 );
}

