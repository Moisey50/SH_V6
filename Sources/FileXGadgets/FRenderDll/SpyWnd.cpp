// SpyWnd.cpp : implementation file
//

#include "stdafx.h"
#include "FRenderDll.h"
#include "SpyWnd.h"


// SpyWnd

IMPLEMENT_DYNAMIC(SpyWnd, CWnd)

SpyWnd::SpyWnd()
{
  m_CallBack = NULL ;
  m_hRectanglePen = CreatePen (PS_SOLID, 3, RGB(256, 0, 0)) ;
}

SpyWnd::~SpyWnd()
{
  DeleteObject (m_hRectanglePen) ;
}


BEGIN_MESSAGE_MAP(SpyWnd, CWnd)
  ON_WM_CREATE()
  ON_WM_LBUTTONUP()
  ON_WM_MOUSEMOVE()
END_MESSAGE_MAP()



// SpyWnd message handlers




int SpyWnd::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
  if (CWnd::OnCreate(lpCreateStruct) == -1)
    return -1;

  // TODO:  Add your specialized creation code here

  return 0;
}


BOOL SpyWnd::Create( const RECT& rect, void* pParam , HWND hHost )
{
  m_hHost = hHost ;
  LPCTSTR lpszDIBVClass =  AfxRegisterWndClass(CS_HREDRAW | CS_VREDRAW , 
    LoadCursor(NULL, IDC_ARROW) , (HBRUSH) ::GetStockObject(WHITE_BRUSH));
  BOOL RESULT =  CWnd::CreateEx( WS_EX_TOPMOST |WS_EX_NOPARENTNOTIFY ,
    lpszDIBVClass,"Select",
    WS_VISIBLE|WS_POPUP,rect,NULL,0);
  if (RESULT) 
  {
    UpdateWindow();
    m_hCursorPrevious = SetCursor( LoadCursor(NULL, IDC_UPARROW) ) ; // set real cursor
    SetCapture() ;
  }
  return RESULT ;
}


void SpyWnd::OnLButtonUp(UINT nFlags, CPoint point)
{
  if ( GetAsyncKeyState( '1' ) & 0x8000 )
  {
    if (m_hCursorPrevious)
    {
      SetCursor (m_hCursorPrevious);
      m_hCursorPrevious = NULL ;
    }
    if (m_hwndFoundWindow)
      RefreshWindow (m_hwndFoundWindow);
    // Very important : must release the mouse capture.
    if ( m_CallBack )
      m_CallBack( MSG_SWITCH_HANDLE , (WPARAM)m_hwndFoundWindow , m_pParam ) ;
  }
  ReleaseCapture ();
  CWnd::OnLButtonUp(nFlags, point);
}


void SpyWnd::OnMouseMove(UINT nFlags, CPoint point)
{
  if ( (GetAsyncKeyState( VK_LBUTTON ) & 0x8000)
    && (GetAsyncKeyState( _T('1') ) & 0x8000) )
  {
    CPoint screenpoint ;
    ::GetCursorPos (&screenpoint);  

    // Determine the window that lies underneath the mouse cursor.
    HWND hwndFoundWindow = ::WindowFromPoint ( screenpoint );

    // Check first for validity.
    if (CheckWindowValidity ( hwndFoundWindow ))
    {
      // If there was a previously found window, we must instruct it to refresh itself. 
      // This is done to remove any highlighting effects drawn by us.
      if (m_hwndFoundWindow)
        RefreshWindow (m_hwndFoundWindow);
 
      // Indicate that this found window is now the current global found window.
      m_hwndFoundWindow = hwndFoundWindow;

      // We now highlight the found window.
      HighlightFoundWindow ( m_hwndFoundWindow );
    }
//     CRect cr ;
//     GetClientRect( &cr ) ;
//     CPoint Center = cr.CenterPoint() ;
//     CSize Shift = point - Center ;
//     if ( Shift.cx != 0  ||  Shift.cy != 0 )
//     {
//       CRect wr ;
//       GetWindowRect( &wr ) ;
//       wr += Shift ;
//       MoveWindow( &wr , TRUE ) ;
//     }
  }

  CWnd::OnMouseMove(nFlags, point);
}

// Synopsis :
// 1. This function checks a hwnd to see if it is actually the "Search Window" Dialog's or Main Window's
// own window or one of their children. If so a FALSE will be returned so that these windows will not
// be selected. 
//
// 2. Also, this routine checks to see if the hwnd to be checked is already a currently found window.
// If so, a FALSE will also be returned to avoid repetitions.
BOOL SpyWnd::CheckWindowValidity ( HWND hwndToCheck )
{
  // The window must not be NULL.
  if ( !hwndToCheck )
    return FALSE ;

  // It must also be a valid window as far as the OS is concerned.
  if ( !IsWindow(hwndToCheck) )
    return FALSE ;

  // Ensure that the window is not the current one which has already been found.
  if ( hwndToCheck == m_hwndFoundWindow )
    return FALSE ;

  // It must also not be the main window itself.
  if ( hwndToCheck == m_hHost )
    return FALSE ;

//   // It also must not be the "Search Window" dialog box itself.
//   if (hwndToCheck == hwndDialog)
//     return FALSE ;

  // It also must not be one of the dialog box's children...
  HWND hwndTemp = ::GetParent (hwndToCheck);
  if (( hwndTemp == m_hHost ) /*|| (hwndTemp == g_hwndMainWnd)*/)
    return FALSE ;

  return TRUE ;
}

long SpyWnd::RefreshWindow (HWND hwndWindowToBeRefreshed)
{
  ::InvalidateRect (hwndWindowToBeRefreshed, NULL, TRUE);
  ::UpdateWindow (hwndWindowToBeRefreshed);
  ::RedrawWindow (hwndWindowToBeRefreshed, NULL, NULL, 
    RDW_FRAME | RDW_INVALIDATE | RDW_UPDATENOW | RDW_ALLCHILDREN);
  return 0;
}















// Performs a highlighting of a found window.
// Comments below will demonstrate how this is done.
long SpyWnd::HighlightFoundWindow ( HWND hwndFoundWindow )
{
  RECT		rect;              // Rectangle area of the found window.
  // Get the screen coordinates of the rectangle of the found window.
  ::GetWindowRect ( hwndFoundWindow , &rect);

  // Get the window DC of the found window.
  HDC		hWindowDC = ::GetWindowDC ( hwndFoundWindow );


  if (hWindowDC)
  {
    // Select our created pen into the DC and backup the previous pen.
    HGDIOBJ	hPrevPen = ::SelectObject ( hWindowDC , m_hRectanglePen);

    // Select a transparent brush into the DC and backup the previous brush.
    HGDIOBJ	hPrevBrush = ::SelectObject (hWindowDC, GetStockObject(HOLLOW_BRUSH));

    // Draw a rectangle in the DC covering the entire window area of the found window.
    ::Rectangle (hWindowDC, 0, 0, rect.right - rect.left, rect.bottom - rect.top);

    // Reinsert the previous pen and brush into the found window's DC.
    ::SelectObject ( hWindowDC , hPrevPen );

    ::SelectObject ( hWindowDC , hPrevBrush );

    // Finally release the DC.
    ::ReleaseDC ( hwndFoundWindow , hWindowDC );
  }
  return 0;
}


