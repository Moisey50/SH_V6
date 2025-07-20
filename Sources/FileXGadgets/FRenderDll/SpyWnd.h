#pragma once

#define MSG_SWITCH_HANDLE (10)


typedef void SpyEvent(int Event, WPARAM wParam, void * lParam);

// SpyWnd

class SpyWnd : public CWnd
{
	DECLARE_DYNAMIC(SpyWnd)

public:
	SpyWnd();
	virtual ~SpyWnd();

protected:
	DECLARE_MESSAGE_MAP()

  SpyEvent *  m_CallBack ;
  void *      m_pParam   ;

  HCURSOR		m_hCursorSearchWindow ;
  HCURSOR		m_hCursorPrevious ;
  CBitmap		m_BitmapFinderToolFilled;
  CBitmap		m_BitmapFinderToolEmpty;
  HWND		  m_hwndFoundWindow ;
  HWND      m_hHost ;
  HGDIOBJ   m_hRectanglePen ;

public:
  afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
  BOOL Create(const RECT& rect, void * pParam , HWND hHost );
  afx_msg void OnLButtonUp(UINT nFlags, CPoint point);
  afx_msg void OnMouseMove(UINT nFlags, CPoint point);
  void SetCallBack( SpyEvent * pCallBackFunction , void * pParam ) 
  { m_CallBack = pCallBackFunction ; m_pParam = pParam ; }
  long HighlightFoundWindow ( HWND hwndFoundWindow) ;
  BOOL CheckWindowValidity ( HWND hwndToCheck ) ;
  long RefreshWindow (HWND hwndWindowToBeRefreshed) ;
};


