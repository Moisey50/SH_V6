#pragma once
#include <winuser.h>

typedef void (*MouseHookProc)(int iMouseX , int iMouseY , HWND hSelected , void * pGadget ) ;

// LRESULT CALLBACK LowLevelMouseProc(
//   _In_ int    nCode , _In_ WPARAM wParam , _In_ LPARAM lParam )
// {
//   if (nCode < 0)
//     return CallNextHookEx( NULL , nCode , wParam , lParam ) ;
//   switch (wParam)
//   {
//     case WM_LBUTTONUP:
//       MSLLHOOKSTRUCT * pData = ( MSLLHOOKSTRUCT * ) lParam ;
//       
// 
//       break ;
//   }
//   return 1 ;
// }


class CMouseHook
{
public:
  CMouseHook( MouseHookProc pCallBack , void * pHost )
  {
    m_CallBack = pCallBack ;
    m_pHost = pHost ;
    m_ResultBuffer[ 0 ] = 0 ;
    m_hHook = SetWindowsHookEx( WH_MOUSE_LL , LowLevelMouseProc , NULL , 0 ) ;
  }

  static LRESULT CALLBACK LowLevelMouseProc(
    _In_ int    nCode , _In_ WPARAM wParam , _In_ LPARAM lParam )
  {
    if (nCode < 0)
      return CallNextHookEx( NULL , nCode , wParam , lParam ) ;
    switch (wParam)
    {
      case WM_LBUTTONUP:
        UnhookWindowsHookEx( m_hHook ) ;
        m_hHook = NULL ;
        MSLLHOOKSTRUCT * pData = ( MSLLHOOKSTRUCT * ) lParam ;
        HWND hSelected = WindowFromPoint( pData->pt ) ;
        sprintf_s( m_ResultBuffer , "Pt=(%d,%d); hWnd=0x%p;" ,
          pData->pt.x , pData->pt.y , hSelected ) ;
        m_CallBack( pData->pt.x , pData->pt.y , hSelected , m_pHost ) ;
        break ;
    }
    return CallNextHookEx( NULL , nCode , wParam , lParam ) ;
  }
  static HHOOK m_hHook ;
  static MouseHookProc m_CallBack ;
  static void *        m_pHost ;
  static char   m_ResultBuffer[ 100 ] ;
};

